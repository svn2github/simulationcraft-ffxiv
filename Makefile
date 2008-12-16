# ==========================================================================
# Dedmonwakeen's Raid DPS/TPS Simulator.
# Send questions to natehieter@gmail.com
# ==========================================================================

# To build on Unix: make 
# To build on Windows with MinGW: c:\mingw\bin\mingw32-make OS=WINDOWS
# To build on Mac: make OS=OSX

# To build debuggable executable, add OPTS=-g to cmd-line invocation

OS = UNIX

# Any UNIX platform
ifeq (UNIX,${OS})
CXX        = g++
CPP_FLAGS  = -Wall
LINK_FLAGS = 
LINK_LIBS  = -lpthread
OPTS       = -O3
BITS       = 32
ifeq (64,${BITS})
OPTS += -m64
else
OPTS += -malign-double 
endif
endif

# Windows platform with MinGW32
ifeq (WINDOWS,${OS})
CXX        = g++
CPP_FLAGS  = -Wall
LINK_FLAGS = 
LINK_LIBS  =
OPTS       = -O3 -malign-double 
endif

# Mac platform
ifeq (OSX,${OS})
CXX        = g++
CPP_FLAGS  = -arch ppc -arch i386 -Wall
LINK_FLAGS = -arch ppc -arch i386
LINK_LIBS  = -lpthread
OPTS       = -O
endif


SRC =\
	simcraft.h		\
	sc_action.cpp		\
	sc_attack.cpp		\
	sc_consumable.cpp	\
	sc_druid.cpp		\
	sc_enchant.cpp		\
	sc_event.cpp		\
	sc_mage.cpp		\
	sc_option.cpp		\
	sc_pet.cpp		\
	sc_player.cpp		\
	sc_priest.cpp		\
	sc_rating.cpp		\
	sc_report.cpp		\
	sc_rng.cpp		\
	sc_rogue.cpp		\
	sc_shaman.cpp		\
	sc_scaling.cpp		\
	sc_sim.cpp		\
	sc_spell.cpp		\
	sc_stats.cpp		\
	sc_target.cpp		\
	sc_thread.cpp		\
	sc_unique_gear.cpp	\
	sc_util.cpp		\
	sc_warlock.cpp		\
	sc_weapon.cpp

SRC_H   := $(filter %.h,   $(SRC))
SRC_CPP := $(filter %.cpp, $(SRC))
SRC_OBJ := $(SRC_CPP:.cpp=.o)

simcraft: $(SRC_OBJ)
	$(CXX) $(LINK_FLAGS) $(SRC_OBJ) $(LINK_LIBS) -o simcraft

$(SRC_OBJ): %.o: %.cpp $(SRC_H) Makefile
	$(CXX) $(CPP_FLAGS) $(OPTS) -c $< -o $@

# Release targets (may use platform-specific cmds)

REV=0
tarball:
	tar -cvf simcraft-r$(REV).tar $(SRC) Makefile raid_70.txt raid_80.txt
	gzip simcraft-r$(REV).tar

clean:
	rm -f simcraft *.o *~

# Deprecated targets

unix windows mac:
	$(error unix/windows/mac are no longer a valid targets, please read the docs at the top of Makefile)
