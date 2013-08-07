# This is a little script to synchronize our various build files.
#
# Currently supported: 
# - parsing Qt files
# - output: Qt, Visual Studio, POSIX Makefile 

import re
import datetime

def parse_qt( filename ):
    f = open( filename, "r" )
    out = []
    if f:
        for line in f:
            match = re.search ( r"(\s*)(SOURCES|HEADERS|PRECOMPILED_HEADER)(\s*\+?\=\s*)([\w\/]*)(\.\w*)", line )
            if match:
                out.append( ( match.group(2), match.group(4) + match.group(5), match.group(5) ) )
        f.close()
    return out


def header( system ):
    if system == "VS":
        h = "<!--\n"
    else:
        h = "# "
    h += "This file is automatically generated by update.py\n"
    if system != "VS":
        h += "# "
    h += "Last updated: " + str(datetime.datetime.now() ) + "\n\n"
    if system != "VS":
        h += "# "
    h += "To change the list of source files, update the QT_ files and run update.py"
    if system == "VS":
        h += "\n-->"
    h += "\n\n"
    return h

def write_to_file( filename, out ):
    f = open( filename, "w" )
    if f:
        f.write( out )
        f.close()

def create_qt_str( input ):
    prepare = header( "QT" )
    for entry in input:
        if entry[0] == "SOURCES" or entry[0] == "HEADERS" or entry[0] == "PRECOMPILED_HEADER":
            prepare += "\n " + entry[0] + " += " + entry[1]
    return prepare


def create_make_str( input ):
    modified_input = replace( input, r"engine/", r"" )
    modified_input = replace( modified_input , r"/" , r"$(PATHSEP)" )
    prepare = header( "Makefile" )
    prepare += "SRC += \\"
    for entry in modified_input:
        if entry[0] == "SOURCES" or entry[0] == "HEADERS":
            prepare += "\n    " + entry[1] + " \\"
    return prepare

def create_vs_str( input, gui = False ):
    modified_input = replace( input, r"^", r"..\\" )
    modified_input = replace( modified_input , r"/" , r"\\" )
    prepare = header( "VS" )
    prepare += """<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
<ItemGroup>"""
    for entry in modified_input:
        if re.search( r"sc_io.cpp", entry[1] ):
            prepare += "\n<ClCompile Include=\"" + entry[1] + "\">\n      <PrecompiledHeader>Create</PrecompiledHeader>\n    </ClCompile>"
        elif not gui and entry[0] == "HEADERS":
            prepare += "\n    <ClInclude Include=\"" + entry[1] + "\" />"
        elif entry[0] == "SOURCES":
            prepare += "\n    <ClCompile Include=\"" + entry[1] + "\" />"
    prepare += """\n  </ItemGroup>  
</Project>"""
    return prepare
        
def replace( input, separator, repl ):
    r = []
    for entry in input:
        r.append( ( entry[0], re.sub( separator, repl, entry[1] ) ) )
    return r

def sort_by_name( input ):
    input.sort( key=lambda entry : entry[2], reverse=True )

def create_file( file_type, build_systems ):
    result = parse_qt( "QT_" + file_type + ".pri" )
    # print(result)
    sort_by_name( result )
    if "make" in build_systems:
        write_to_file( file_type + "_make", create_make_str( result ) )
    if "VS" in build_systems:
        write_to_file( "VS_" + file_type + ".props", create_vs_str( result ) )
    if "VS_GUI" in build_systems:
        write_to_file( "VS_" + file_type + ".props", create_vs_str( result, True ) )
    if "QT" in build_systems:
        write_to_file( "QT_" + file_type + ".pri", create_qt_str( result ) )
    
def main():
    create_file( "engine", ["make","VS", "QT"] )
    create_file( "engine_main", ["make","VS", "QT"] )
    create_file( "gui", ["QT"] ) # TODO: finish mocing part of VS_GUI
    
if __name__ == "__main__":
    main()
            
