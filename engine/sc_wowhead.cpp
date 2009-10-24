// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

#include "simulationcraft.h"

namespace   // ANONYMOUS NAMESPACE ==========================================
{

// format_server =========================================================

std::string& format_server( std::string& name )
{
  if ( name.empty() ) return name;

  std::string buffer="";

  int size = ( int ) name.size();
  for ( int i=0; i < size; i++ )
  {
    unsigned char c = name[ i ];

    if ( c >= 0x80 )
    {
      continue;
    }
    else if ( ( c == '\'' ) || ( c == '.' ) )
    {
      continue;
    }
    else if ( isalpha( ( int ) c ) )
    {
      c = tolower( ( unsigned ) c );
    }
    buffer += c;
  }
  name = buffer;

  return name;
}

// download_profile =========================================================

static js_node_t* download_profile( sim_t* sim,
                                    const std::string& id )
{
  std::string url = "http://www.wowhead.com/?profile=load&id=" + id;
  std::string result;

  if ( http_t::download( result, url ) )
  {
    std::string::size_type start = result.find( "WowheadProfiler.registerProfile(" );
    if ( start != std::string::npos ) start = result.find( "{", start );
    if ( start != std::string::npos )
    {
      std::string::size_type finish = result.find( ";", start );
      if ( finish != std::string::npos ) finish = result.rfind( "}", finish );
      if ( finish != std::string::npos )
      {
        std::string buffer = result.substr( start, ( finish - start ) + 1 );
        if ( sim -> debug ) util_t::fprintf( sim -> output_file, "%s\n", buffer.c_str() );
        return js_t::create( buffer );
      }
    }
  }

  return 0;
}

// download_id ==============================================================

static xml_node_t* download_id( sim_t* sim,
                                const std::string& id_str,
                                int cache_only=0 )
{
  if ( id_str.empty() || id_str == "" || id_str == "0" ) return 0;
  std::string url_www = "http://www.wowhead.com/?item=" + id_str + "&xml";
  std::string url_ptr = "http://ptr.wowhead.com/?item=" + id_str + "&xml";

  xml_node_t *node;

  if ( cache_only )
  {
    node = xml_t::download_cache( url_www );
    if ( ! node )
      node = xml_t::download_cache( url_ptr );
  }
  else
  {
    node = xml_t::download( url_www, "</json>", 0 );
    if ( ! node )
      node = xml_t::download( url_ptr, "</json>", 0 );
  }

  if ( sim -> debug ) xml_t::print( node );
  return node;
}

// parse_stats ==============================================================

static void parse_stats( std::string& encoding,
                         const std::string& stats_str )
{
  std::vector<std::string> splits;
  int num_splits = util_t::string_split( splits, stats_str, "," );

  for ( int i=0; i < num_splits; i++ )
  {
    std::string type_str, value_str;

    if ( 2 == util_t::string_split( splits[ i ], ":", "S S", &type_str, &value_str ) )
    {
      int type = util_t::parse_stat_type( type_str );
      if ( type != STAT_NONE )
      {
        encoding += "_";
        encoding += value_str;
        encoding += util_t::stat_type_abbrev( type );
      }
    }
  }
}

// parse_gems ===============================================================

static bool parse_gems( item_t&           item,
                        xml_node_t*       node,
                        const std::string gem_ids[ 3 ] )
{
  item.armory_gems_str.clear();

  std::string stats_str;
  if ( ! xml_t::get_value( stats_str, node, "jsonEquip/cdata" ) )
    return true;

  int sockets[ 3 ] = { 0, 0, 0 };
  std::vector<std::string> splits;
  int num_splits = util_t::string_split( splits, stats_str, "," );

  for ( int i=0; i < num_splits; i++ )
  {
    std::string type_str;
    int value;

    if ( 2 == util_t::string_split( splits[ i ], ":", "S i", &type_str, &value ) )
    {
      if ( type_str == "socket1" ) sockets[ 0 ] = value;
      if ( type_str == "socket2" ) sockets[ 1 ] = value;
      if ( type_str == "socket3" ) sockets[ 2 ] = value;
    }
  }

  bool match = true;
  for ( int i=0; i < 3; i++ )
  {
    int gem = item_t::parse_gem( item, gem_ids[ i ] );

    switch ( sockets[ i ] )
    {
    case 0: sockets[ i ] = GEM_NONE;   break;
    case 1: sockets[ i ] = GEM_META;   break;
    case 2: sockets[ i ] = GEM_RED;    break;
    case 4: sockets[ i ] = GEM_YELLOW; break;
    case 8: sockets[ i ] = GEM_BLUE;   break;
    }

    if ( ! util_t::socket_gem_match( sockets[ i ], gem ) )
      match = false;
  }

  if ( match )
  {
    std::string tooltip_str;
    if ( xml_t::get_value( tooltip_str, node, "htmlTooltip/cdata" ) )
    {
      const char* search = "Socket Bonus: ";
      std::string::size_type pos = tooltip_str.find( search );
      if ( pos != std::string::npos )
      {
        std::string::size_type start = pos + strlen( search );
        std::string::size_type pos = tooltip_str.find( "<", start );
        if ( pos != std::string::npos )
        {
          armory_t::fuzzy_stats( item.armory_gems_str, tooltip_str.substr( start, pos-start ) );
        }
      }
    }
  }

  armory_t::format( item.armory_gems_str );

  return true;
}

// parse_weapon =============================================================

static bool parse_weapon( item_t&     item,
                          xml_node_t* node )
{
  std::string stats_str;
  if ( ! xml_t::get_value( stats_str, node, "jsonEquip/cdata" ) )
    return true;

  std::string speed, dps, dmgmin, dmgmax;
  std::vector<std::string> splits;
  int num_splits = util_t::string_split( splits, stats_str, "," );

  for ( int i=0; i < num_splits; i++ )
  {
    std::string type_str, value_str;

    if ( 2 == util_t::string_split( splits[ i ], ":", "S S", &type_str, &value_str ) )
    {
      if ( type_str == "speed"   ) speed  = value_str;
      if ( type_str == "dps"     ) dps    = value_str;
      if ( type_str == "dmgmin1" ) dmgmin = value_str;
      if ( type_str == "dmgmax1" ) dmgmax = value_str;
    }
  }

  if ( speed.empty() || dps.empty() || dmgmin.empty() || dmgmax.empty() ) return true;

  std::string subclass_str;
  if ( ! xml_t::get_value( subclass_str, node, "subclass/cdata" ) )
    return true;

  int weapon_type = WEAPON_NONE;
  if      ( subclass_str == "One-Handed Axes"         ) weapon_type = WEAPON_AXE;
  else if ( subclass_str == "Two-Handed Axes"         ) weapon_type = WEAPON_AXE_2H;
  else if ( subclass_str == "Daggers"                 ) weapon_type = WEAPON_DAGGER;
  else if ( subclass_str == "Fist Weapons"            ) weapon_type = WEAPON_FIST;
  else if ( subclass_str == "One-Handed Maces"        ) weapon_type = WEAPON_MACE;
  else if ( subclass_str == "Two-Handed Maces"        ) weapon_type = WEAPON_MACE_2H;
  else if ( subclass_str == "Polearms"                ) weapon_type = WEAPON_POLEARM;
  else if ( subclass_str == "Staves"                  ) weapon_type = WEAPON_STAFF;
  else if ( subclass_str == "One-Handed Swords"       ) weapon_type = WEAPON_SWORD;
  else if ( subclass_str == "Two-Handed Swords"       ) weapon_type = WEAPON_SWORD_2H;
  else if ( subclass_str == "Bows"                    ) weapon_type = WEAPON_BOW;
  else if ( subclass_str == "Crossbows"               ) weapon_type = WEAPON_CROSSBOW;
  else if ( subclass_str == "Guns"                    ) weapon_type = WEAPON_GUN;
  else if ( subclass_str == "Thrown"                  ) weapon_type = WEAPON_THROWN;
  else if ( subclass_str == "Wands"                   ) weapon_type = WEAPON_WAND;
  else if ( subclass_str == "Fishing Poles"           ) weapon_type = WEAPON_POLEARM;
  else if ( subclass_str == "Miscellaneous (Weapons)" ) weapon_type = WEAPON_POLEARM;

  if ( weapon_type == WEAPON_NONE ) return false;
  if ( weapon_type == WEAPON_WAND ) return true;

  item.armory_weapon_str = util_t::weapon_type_string( weapon_type );
  item.armory_weapon_str += "_" + speed + "speed" + "_" + dmgmin + "min" + "_" + dmgmax + "max";

  return true;
}

// parse_item_stats =========================================================

static bool parse_item_stats( item_t&     item,
                              xml_node_t* node )
{
  item.armory_stats_str.clear();

  std::string stats_str;
  if ( ! xml_t::get_value( stats_str, node, "jsonEquip/cdata" ) )
    return true;

  parse_stats( item.armory_stats_str, stats_str );
  armory_t::format( item.armory_stats_str );

  return true;
}

// parse_item_name ==========================================================

static bool parse_item_name( item_t&     item,
                             xml_node_t* node )
{
  if ( ! xml_t::get_value( item.armory_name_str, node, "name/cdata" ) )
    return false;

  if ( ! xml_t::get_value( item.armory_id_str, node, "item/id" ) )
    return false;

  armory_t::format( item.armory_name_str );

  return true;
}

// translate_profession_id ==================================================

static const char* translate_profession_id( int type )
{
  switch ( type )
  {
  case PROF_ALCHEMY:        return "skills/171";
  case PROF_BLACKSMITHING:  return "skills/164";
  case PROF_ENCHANTING:     return "skills/333";
  case PROF_ENGINEERING:    return "skills/202";
  case PROF_HERBALISM:      return "skills/182";
  case PROF_INSCRIPTION:    return "skills/773";
  case PROF_JEWELCRAFTING:  return "skills/755";
  case PROF_LEATHERWORKING: return "skills//165";
  case PROF_MINING:         return "skills/186";
  case PROF_SKINNING:       return "skills/393";
  case PROF_TAILORING:      return "skills/197";
  }
  return "unknown";
}

// translate_inventory_id ===================================================

static const char* translate_inventory_id( int slot )
{
  switch ( slot )
  {
  case SLOT_HEAD:      return "inventory/1";
  case SLOT_NECK:      return "inventory/2";
  case SLOT_SHOULDERS: return "inventory/3";
  case SLOT_SHIRT:     return "inventory/4";
  case SLOT_CHEST:     return "inventory/5";
  case SLOT_WAIST:     return "inventory/6";
  case SLOT_LEGS:      return "inventory/7";
  case SLOT_FEET:      return "inventory/8";
  case SLOT_WRISTS:    return "inventory/9";
  case SLOT_HANDS:     return "inventory/10";
  case SLOT_FINGER_1:  return "inventory/11";
  case SLOT_FINGER_2:  return "inventory/12";
  case SLOT_TRINKET_1: return "inventory/13";
  case SLOT_TRINKET_2: return "inventory/14";
  case SLOT_BACK:      return "inventory/15";
  case SLOT_MAIN_HAND: return "inventory/16";
  case SLOT_OFF_HAND:  return "inventory/17";
  case SLOT_RANGED:    return "inventory/18";
  }

  return "unknown";
}

} // ANONYMOUS NAMESPACE ====================================================

// wowhead_t::parse_gem ================================================================

int wowhead_t::parse_gem( item_t&            item,
                          const std::string& gem_id,
                          int cache_only )
{
  if ( gem_id.empty() || gem_id == "" || gem_id == "0" )
    return GEM_NONE;

  xml_node_t* node = download_id( item.sim, gem_id, cache_only );
  if ( ! node )
  {
    if ( ! cache_only )
      util_t::fprintf( item.sim -> output_file,
		       "\nsimulationcraft: Player %s unable to download gem id %s from wowhead\n", 
		       item.player -> name(), gem_id.c_str() );
    return GEM_NONE;
  }

  int gem_type = GEM_NONE;

  std::string color_str;
  if ( xml_t::get_value( color_str, node, "subclass/cdata" ) )
  {
    std::string::size_type pos = color_str.find( ' ' );
    if ( pos != std::string::npos ) color_str.erase( pos );
    armory_t::format( color_str );
    gem_type = util_t::parse_gem_type( color_str );

    if ( gem_type == GEM_META )
    {
      std::string name_str;
      if ( xml_t::get_value( name_str, node, "name/cdata" ) )
      {
        std::string::size_type pos = name_str.find( " Diamond" );
        if ( pos != std::string::npos ) name_str.erase( pos );
        armory_t::format( name_str );
        item.armory_gems_str += "_";
        item.armory_gems_str += name_str;
      }
    }
    else
    {
      std::string stats_str;
      if ( xml_t::get_value( stats_str, node, "jsonEquip/cdata" ) )
      {
        parse_stats( item.armory_gems_str, stats_str );
      }
    }
  }

  return gem_type;
}

// wowhead_t::download_glyph ================================================

bool wowhead_t::download_glyph( sim_t*             sim,
                                std::string&       glyph_name,
                                const std::string& glyph_id,
                                int                cache_only )
{
  xml_node_t* node = download_id( sim, glyph_id, cache_only );
  if ( ! node || ! xml_t::get_value( glyph_name, node, "name/cdata" ) )
  {
    if ( ! cache_only )
      util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to download glyph id %s from wowhead\n", glyph_id.c_str() );
    return false;
  }

  glyph_name.erase( 0, 9 ); // remove "Glyph of "
  armory_t::format( glyph_name );
  return true;
}

// wowhead_t::download_item =================================================

bool wowhead_t::download_item( item_t&            item,
                               const std::string& item_id,
                               int cache_only )
{
  player_t* p = item.player;

  xml_node_t* node = download_id( item.sim, item_id, cache_only );
  if ( ! node )
  {
    if ( ! cache_only )
      util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to download item id %s from wowhead at slot %s.\n", p -> name(), item_id.c_str(), item.slot_name() );
    return false;
  }

  if ( ! parse_item_name( item, node ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine item name for id %s at slot %s.\n", p -> name(), item_id.c_str(), item.slot_name() );
    return false;
  }

  if ( ! parse_item_stats( item, node ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine stats for item '%s' at slot %s.\n", p -> name(), item.name(), item.slot_name() );
    return false;
  }

  if ( ! parse_weapon( item, node ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine weapon info for item '%s' at slot %s.\n", p -> name(), item.name(), item.slot_name() );
    return false;
  }

  return true;
}

// wowhead_t::download_slot =================================================

bool wowhead_t::download_slot( item_t&            item,
                               const std::string& item_id,
                               const std::string& enchant_id,
                               const std::string  gem_ids[ 3 ],
                               int cache_only )
{
  player_t* p = item.player;

  xml_node_t* node = download_id( item.sim, item_id, cache_only );
  if ( ! node )
  {
    if ( ! cache_only )
      util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to download item id '%s' from wowhead at slot %s.\n", p -> name(), item_id.c_str(), item.slot_name() );
    return false;
  }

  if ( ! parse_item_name( item, node ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine item name for id '%s' at slot %s.\n", p -> name(), item_id.c_str(), item.slot_name() );
    return false;
  }

  if ( ! parse_item_stats( item, node ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine stats for item '%s' at slot %s.\n", p -> name(), item.name(), item.slot_name() );
    return false;
  }

  if ( ! parse_weapon( item, node ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine weapon info for item '%s' at slot %s.\n", p -> name(), item.name(), item.slot_name() );
    return false;
  }

  if ( ! parse_gems( item, node, gem_ids ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to determine gems for item '%s' at slot %s.\n", p -> name(), item.name(), item.slot_name() );
    return false;
  }

  if ( ! enchant_t::download( item, enchant_id ) )
  {
    util_t::fprintf( item.sim -> output_file, "\nsimulationcraft: Player %s unable to parse enchant id %s for item \"%s\" at slot %s.\n", p -> name(), enchant_id.c_str(), item.name(), item.slot_name() );
    //return false;
  }

  return true;
}

// wowhead_t::download_player ===============================================

player_t* wowhead_t::download_player( sim_t* sim,
                                      const std::string& region,
                                      const std::string& server,
                                      const std::string& name,
                                      int use_active_talents )
{
  std::string character_name = name;
  armory_t::format( character_name, FORMAT_CHAR_NAME_MASK | FORMAT_ASCII_MASK );

  std::string server_name = server;
  format_server( server_name );

  std::string url = "http://www.wowhead.com/?profile=" + region + "." + server_name + "." + character_name;
  std::string result;

  if ( http_t::download( result, url ) )
  {
    if ( sim -> debug ) util_t::fprintf( sim -> output_file, "%s\n%s\n", url.c_str(), result.c_str() );

    std::string::size_type start = result.find( "profilah.initialize(" );
    if ( start != std::string::npos )
    {
      std::string::size_type finish = result.find( ";", start );
      if ( finish != std::string::npos )
      {
        std::vector<std::string> splits;
        int num_splits = util_t::string_split( splits, result.substr( start, finish-start ), "(){}.;,: \t\n" );
        for ( int i=0; i < num_splits-1; i++ )
        {
          if ( splits[ i ] == "id" )
          {
            return download_player( sim, splits[ i+1 ], use_active_talents );
          }
        }
      }
    }
  }

  return 0;
}

// wowhead_t::download_player ===============================================

player_t* wowhead_t::download_player( sim_t* sim,
                                      const std::string& id,
                                      int use_active_talents )
{
  js_node_t* profile_js = download_profile( sim, id );

  if ( ! profile_js )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to download character profile %s from wowhead.\n", id.c_str() );
    return 0;
  }

  if ( sim -> debug ) js_t::print( profile_js, sim -> output_file );

  std::string name_str;
  if ( ! js_t::get_value( name_str, profile_js, "name"  ) )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to extract player name from wowhead id '%s'.\n", id.c_str() );
    name_str = "wowhead" + id;
  }

  armory_t::format( name_str, FORMAT_CHAR_NAME_MASK | FORMAT_UTF8_MASK );
  util_t::format_name ( name_str );

  std::string level_str;
  if ( ! js_t::get_value( level_str, profile_js, "level"  ) )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to extract player level from wowhead id '%s'.\n", id.c_str() );
    return 0;
  }
  int level = atoi( level_str.c_str() );
  if ( level < 60 )
  {
    level = 60;
  }
  else if ( level > 80 )
  {
    level = 80;
  }

  std::string cid_str;
  if ( ! js_t::get_value( cid_str, profile_js, "classs" ) )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to extract player class from wowhead id '%s'.\n", id.c_str() );
    return 0;
  }
  int player_type = util_t::translate_class_id( atoi( cid_str.c_str() ) );
  std::string type_str = util_t::player_type_string( player_type );

  if ( type_str == "death_knight" )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: The Death Knight module are still in development, so wowhead downloads are disabled.\n" );
    return 0;
  }

  std::string rid_str;
  if ( ! js_t::get_value( rid_str, profile_js, "race" ) )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to extract player race from wowhead id '%s'.\n", id.c_str() );
    return 0;
  }
  int race_type = util_t::translate_race_id( atoi( rid_str.c_str() ) );

  player_t* p = player_t::create( sim, type_str, name_str, race_type );
  if ( ! p )
  {
    util_t::fprintf( sim -> output_file, "\nsimulationcraft: Unable to build player with class '%s' and name '%s' from wowhead id '%s'.\n",
                    type_str.c_str(), name_str.c_str(), id.c_str() );
    return 0;
  }

  p -> level = level;

  std::vector<std::string> region_data;
  int num_region = js_t::get_value( region_data, profile_js, "region" );
  if ( num_region > 0 ) p -> region_str = region_data[ 0 ];

  std::vector<std::string> realm_data;
  int num_realm = js_t::get_value( realm_data, profile_js, "realm" );
  if ( num_realm > 0 ) p -> server_str = realm_data[ 0 ];

  int user_id=0;
  if ( js_t::get_value( user_id, profile_js, "user" ) && ( user_id != 0 ) )
  {
    p -> origin_str = "http://profiler.wowhead.com/?profile=" + id;
  }
  else
  {
    std::string server_name = p -> server_str;
    std::string character_name = name_str;
    format_server( server_name );
    armory_t::format( character_name, FORMAT_CHAR_NAME_MASK | FORMAT_ASCII_MASK );
    p -> origin_str = "http://profiler.wowhead.com/?profile=" + p -> region_str + "." + server_name + "." + character_name;
  }

  p -> professions_str = "";
  for ( int i=0; i < PROFESSION_MAX; i++ )
  {
    std::vector<std::string> skill_levels;
    if ( 2 == js_t::get_value( skill_levels, profile_js, translate_profession_id( i ) ) )
    {
      if ( p -> professions_str.size() > 0 ) p -> professions_str += "/";
      p -> professions_str += util_t::profession_type_string( i );
      p -> professions_str += "=";
      p -> professions_str += skill_levels[ 0 ];
    }
  }

  int active_talents = 0;
  js_t::get_value( active_talents, profile_js, "talents/active" );
  if ( ! use_active_talents ) active_talents = ( active_talents ? 0 : 1 );

  std::vector<std::string> talent_encodings;
  int num_builds = js_t::get_value( talent_encodings, profile_js, "talents/build" );
  if ( num_builds == 2 )
  {
    std::string& encoding = talent_encodings[ active_talents ];
    if ( ! p -> parse_talents_armory( encoding ) )
    {
      util_t::fprintf( sim -> output_file, "\nsimulationcraft: Player %s unable to parse talent encoding '%s'.\n", p -> name(), encoding.c_str() );
      return 0;
    }
    p -> talents_str = "http://www.wowarmory.com/talent-calc.xml?cid=" + cid_str + "&tal=" + encoding;
  }

  std::vector<std::string> glyph_encodings;
  num_builds = js_t::get_value( glyph_encodings, profile_js, "glyphs" );
  if ( num_builds == 2 )
  {
    p -> glyphs_str = "";
    std::vector<std::string> glyph_ids;
    int num_glyphs = util_t::string_split( glyph_ids, glyph_encodings[ active_talents ], ":" );
    for ( int i=0; i < num_glyphs; i++ )
    {
      std::string& glyph_id = glyph_ids[ i ];
      if ( glyph_id == "0" ) continue;
      std::string glyph_name;

      if ( ! item_t::download_glyph( sim, glyph_name, glyph_id ) )
      {
        return 0;
      }
      if ( i ) p -> glyphs_str += "/";
      p -> glyphs_str += glyph_name;
    }
  }

  for ( int i=0; i < SLOT_MAX; i++ )
  {
    sim -> current_slot = i;
    if( sim -> canceled ) return 0;

    std::vector<std::string> inventory_data;
    if ( js_t::get_value( inventory_data, profile_js, translate_inventory_id( i ) ) )
    {
      std::string    item_id = inventory_data[ 0 ];
      std::string enchant_id = inventory_data[ 2 ];

      std::string gem_ids[ 3 ];
      gem_ids[ 0 ] = inventory_data[ 4 ];
      gem_ids[ 1 ] = inventory_data[ 5 ];
      gem_ids[ 2 ] = inventory_data[ 6 ];

      if ( ! item_t::download_slot( p -> items[ i ], item_id, enchant_id, gem_ids ) )
      {
        return 0;
      }
    }
  }

  return p;
}
