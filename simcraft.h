// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================
#ifndef __SIMCRAFT_H
#define __SIMCRAFT_H

// Platform Initialization ==================================================

#if defined( _MSC_VER ) || defined( __MINGW__ ) || defined( _WINDOWS ) || defined( WIN32 )
#  define WIN32_LEAN_AND_MEAN
#  define VC_EXTRALEAN
#  define _CRT_SECURE_NO_WARNINGS
#endif

// Switching of using 'const'-flag on methods possible ======================
// Results of the sim should always be the same, no matter if this is set to:
// #define SC_CONST const    -OR-
// #define SC_CONST
#define SC_CONST const

#if defined( _MSC_VER )
#  include "./vs/stdint.h"
#  define snprintf _snprintf
#else
#  include <stdint.h>
#endif

#if defined(__GNUC__)
#  define likely(x)       __builtin_expect((x),1)
#  define unlikely(x)     __builtin_expect((x),0)
#else
#  define likely(x) (x)
#  define unlikely(x) (x)
#endif

#include <typeinfo>
#include <stdarg.h>
#include <float.h>
#include <time.h>
#include <string>
#include <queue>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#if __BSD_VISIBLE
#  include <netinet/in.h>
#  if !defined(CLOCKS_PER_SEC)
#    define CLOCKS_PER_SEC 1000000
#  endif
#endif

// Patch Specific Modeling ==================================================

struct patch_t
{
  uint64_t mask;
  uint64_t encode( int arch, int version, int revision ) { return arch*10000 + version*100 + revision; }
  patch_t        ( int arch, int version, int revision ) {         mask = encode( arch, version, revision ); }
  void set       ( int arch, int version, int revision ) {         mask = encode( arch, version, revision ); }
  int  before    ( int arch, int version, int revision ) { return ( mask <  encode( arch, version, revision ) ) ? 1 : 0; }
  int  after     ( int arch, int version, int revision ) { return ( mask >= encode( arch, version, revision ) ) ? 1 : 0; }
  void decode( int* arch, int* version, int* revision )
  {
    uint64_t m = mask;
    *revision = ( int ) m % 100; m /= 100;
    *version  = ( int ) m % 100; m /= 100;
    *arch     = ( int ) m % 100;
  }
  patch_t() { mask = encode( 3, 2, 0 ); }
};

// Forward Declarations ======================================================

struct action_t;
struct action_callback_t;
struct alias_t;
struct attack_t;
struct spell_tick_t;
struct base_stats_t;
struct buff_t;
struct callback_t;
struct death_knight_t;
struct druid_t;
struct enchant_t;
struct event_t;
struct gain_t;
struct hunter_t;
struct item_t;
struct js_node_t;
struct mage_t;
struct option_t;
struct paladin_t;
struct pet_t;
struct player_t;
struct priest_t;
struct proc_t;
struct raid_event_t;
struct rank_t;
struct rating_t;
struct report_t;
struct rng_t;
struct rogue_t;
struct scaling_t;
struct shaman_t;
struct sim_t;
struct spell_t;
struct stats_t;
struct talent_translation_t;
struct target_t;
struct unique_gear_t;
struct uptime_t;
struct warlock_t;
struct warrior_t;
struct weapon_t;
struct xml_node_t;

// Enumerations ==============================================================

enum race_type
{
  RACE_NONE=0,
  // Target Races
  RACE_BEAST, RACE_DRAGONKIN, RACE_GIANT, RACE_HUMANOID, RACE_DEMON, RACE_ELEMENTAL,
  // Player Races
  RACE_NIGHT_ELF, RACE_HUMAN, RACE_GNOME, RACE_DWARF, RACE_DRAENEI, RACE_ORC, RACE_TROLL, RACE_UNDEAD, RACE_BLOOD_ELF, RACE_TAUREN,
  RACE_MAX
};

enum player_type
{
  PLAYER_NONE=0,
  DEATH_KNIGHT, DRUID, HUNTER, MAGE, PALADIN, PRIEST, ROGUE, SHAMAN, WARLOCK, WARRIOR,
  PLAYER_PET, PLAYER_GUARDIAN,
  PLAYER_MAX
};

enum dmg_type { DMG_DIRECT=0, DMG_OVER_TIME=1 };

enum attribute_type { ATTRIBUTE_NONE=0, ATTR_STRENGTH, ATTR_AGILITY, ATTR_STAMINA, ATTR_INTELLECT, ATTR_SPIRIT, ATTRIBUTE_MAX };

enum base_stat_type { BASE_STAT_STRENGTH=0, BASE_STAT_AGILITY, BASE_STAT_STAMINA, BASE_STAT_INTELLECT, BASE_STAT_SPIRIT, 
                      BASE_STAT_HEALTH, BASE_STAT_MANA,
                      BASE_STAT_MELEE_CRIT_PER_AGI, BASE_STAT_SPELL_CRIT_PER_INT, 
                      BASE_STAT_DODGE_PER_AGI,
                      BASE_STAT_MELEE_CRIT, BASE_STAT_SPELL_CRIT, BASE_STAT_MAX };

enum resource_type
{
  RESOURCE_NONE=0,
  RESOURCE_HEALTH, RESOURCE_MANA,  RESOURCE_RAGE, RESOURCE_ENERGY, RESOURCE_FOCUS, RESOURCE_RUNIC,
  RESOURCE_MAX
};

enum result_type
{
  RESULT_NONE=0,
  RESULT_MISS,  RESULT_RESIST, RESULT_DODGE, RESULT_PARRY,
  RESULT_BLOCK, RESULT_GLANCE, RESULT_CRIT,  RESULT_HIT,
  RESULT_MAX
};

#define RESULT_HIT_MASK  ( (1<<RESULT_GLANCE) | (1<<RESULT_BLOCK) | (1<<RESULT_CRIT) | (1<<RESULT_HIT) )
#define RESULT_CRIT_MASK ( (1<<RESULT_CRIT) )
#define RESULT_MISS_MASK ( (1<<RESULT_MISS) )
#define RESULT_ALL_MASK  -1

enum proc_type
{
  PROC_NONE=0,
  PROC_DAMAGE,
  PROC_ATTACK,
  PROC_SPELL,
  PROC_TICK,
  PROC_ATTACK_DIRECT,
  PROC_SPELL_DIRECT,
  PROC_MAX
};

enum action_type { ACTION_USE=0, ACTION_SPELL, ACTION_ATTACK, ACTION_SEQUENCE, ACTION_OTHER, ACTION_MAX };

enum school_type
{
  SCHOOL_NONE=0,
  SCHOOL_ARCANE,    SCHOOL_BLEED,  SCHOOL_CHAOS,  SCHOOL_FIRE,     SCHOOL_FROST,
  SCHOOL_FROSTFIRE, SCHOOL_HOLY,   SCHOOL_NATURE, SCHOOL_PHYSICAL, SCHOOL_SHADOW,
  SCHOOL_MAX
};

enum talent_tree_type
{
  TREE_NONE=0,
  TREE_BLOOD,         TREE_UNHOLY,                         // DEATH KNIGHT
  TREE_BALANCE,       TREE_FERAL,        TREE_RESTORATION, // DRUID
  TREE_BEAST_MASTERY, TREE_MARKSMANSHIP, TREE_SURVIVAL,    // HUNTER
  TREE_ARCANE,        TREE_FIRE,         TREE_FROST,       // MAGE
                                         TREE_RETRIBUTION, // PALADIN
  TREE_DISCIPLINE,    TREE_HOLY,         TREE_SHADOW,      // PRIEST
  TREE_ASSASSINATION, TREE_COMBAT,       TREE_SUBTLETY,    // ROGUE
  TREE_ELEMENTAL,     TREE_ENHANCEMENT,                    // SHAMAN
  TREE_AFFLICTION,    TREE_DEMONOLOGY,   TREE_DESTRUCTION, // WARLOCK
  TREE_ARMS,          TREE_PROTECTION,   TREE_FURY,        // WARRIOR
  TALENT_TREE_MAX
};

enum weapon_type
{
  WEAPON_NONE=0,
  WEAPON_DAGGER,                                                                                   WEAPON_SMALL,
  WEAPON_BEAST,    WEAPON_SWORD,    WEAPON_MACE,     WEAPON_AXE,    WEAPON_FIST,                   WEAPON_1H,
  WEAPON_BEAST_2H, WEAPON_SWORD_2H, WEAPON_MACE_2H,  WEAPON_AXE_2H, WEAPON_STAFF,  WEAPON_POLEARM, WEAPON_2H,
  WEAPON_BOW,      WEAPON_CROSSBOW, WEAPON_GUN,      WEAPON_WAND,   WEAPON_THROWN,                 WEAPON_RANGED,
  WEAPON_MAX
};

enum slot_type   // these enum values match armory settings
{
  SLOT_NONE      = -1,
  SLOT_HEAD      = 0,
  SLOT_NECK      = 1,
  SLOT_SHOULDERS = 2,
  SLOT_SHIRT     = 3,
  SLOT_CHEST     = 4,
  SLOT_WAIST     = 5,
  SLOT_LEGS      = 6,
  SLOT_FEET      = 7,
  SLOT_WRISTS    = 8,
  SLOT_HANDS     = 9,
  SLOT_FINGER_1  = 10,
  SLOT_FINGER_2  = 11,
  SLOT_TRINKET_1 = 12,
  SLOT_TRINKET_2 = 13,
  SLOT_BACK      = 14,
  SLOT_MAIN_HAND = 15,
  SLOT_OFF_HAND  = 16,
  SLOT_RANGED    = 17,
  SLOT_TABARD    = 18,
  SLOT_MAX       = 19
};

enum set_type
{
  SET_NONE = 0,
  SET_T6_CASTER, SET_T6_2PC_CASTER, SET_T6_4PC_CASTER,
  SET_T6_MELEE,  SET_T6_2PC_MELEE,  SET_T6_4PC_MELEE,
  SET_T6_TANK,   SET_T6_2PC_TANK,   SET_T6_4PC_TANK,
  SET_T7_CASTER, SET_T7_2PC_CASTER, SET_T7_4PC_CASTER,
  SET_T7_MELEE,  SET_T7_2PC_MELEE,  SET_T7_4PC_MELEE,
  SET_T7_TANK,   SET_T7_2PC_TANK,   SET_T7_4PC_TANK,
  SET_T8_CASTER, SET_T8_2PC_CASTER, SET_T8_4PC_CASTER,
  SET_T8_MELEE,  SET_T8_2PC_MELEE,  SET_T8_4PC_MELEE,
  SET_T8_TANK,   SET_T8_2PC_TANK,   SET_T8_4PC_TANK,
  SET_T9_CASTER, SET_T9_2PC_CASTER, SET_T9_4PC_CASTER,
  SET_T9_MELEE,  SET_T9_2PC_MELEE,  SET_T9_4PC_MELEE,
  SET_T9_TANK,   SET_T9_2PC_TANK,   SET_T9_4PC_TANK,
  SET_T10_CASTER, SET_T10_2PC_CASTER, SET_T10_4PC_CASTER,
  SET_T10_MELEE,  SET_T10_2PC_MELEE,  SET_T10_4PC_MELEE,
  SET_T10_TANK,   SET_T10_2PC_TANK,   SET_T10_4PC_TANK,
  SET_SPELLSTRIKE,
  SET_MAX
};

enum gem_type
{
  GEM_NONE=0,
  GEM_META, GEM_PRISMATIC,
  GEM_RED, GEM_YELLOW, GEM_BLUE,
  GEM_ORANGE, GEM_GREEN, GEM_PURPLE,
  GEM_MAX
};

enum meta_gem_type
{
  META_GEM_NONE=0,
  META_AUSTERE_EARTHSIEGE,
  META_BEAMING_EARTHSIEGE,
  META_BRACING_EARTHSIEGE,
  META_BRACING_EARTHSTORM,
  META_CHAOTIC_SKYFIRE,
  META_CHAOTIC_SKYFLARE,
  META_EMBER_SKYFLARE,
  META_ENIGMATIC_SKYFLARE,
  META_ENIGMATIC_STARFLARE,
  META_ENIGMATIC_SKYFIRE,
  META_ETERNAL_EARTHSIEGE,
  META_ETERNAL_EARTHSTORM,
  META_FORLORN_SKYFLARE,
  META_FORLORN_STARFLARE,
  META_IMPASSIVE_SKYFLARE,
  META_IMPASSIVE_STARFLARE,
  META_INSIGHTFUL_EARTHSIEGE,
  META_INSIGHTFUL_EARTHSTORM,
  META_MYSTICAL_SKYFIRE,
  META_PERSISTENT_EARTHSIEGE,
  META_PERSISTENT_EARTHSHATTER,
  META_POWERFUL_EARTHSIEGE,
  META_POWERFUL_EARTHSHATTER,
  META_POWERFUL_EARTHSTORM,
  META_RELENTLESS_EARTHSIEGE,
  META_RELENTLESS_EARTHSTORM,
  META_REVITALIZING_SKYFLARE,
  META_SWIFT_SKYFIRE,
  META_SWIFT_SKYFLARE,
  META_SWIFT_STARFIRE,
  META_SWIFT_STARFLARE,
  META_TIRELESS_STARFLARE,
  META_GEM_MAX
};

enum stat_type
{
  STAT_NONE=0,
  STAT_STRENGTH, STAT_AGILITY, STAT_STAMINA, STAT_INTELLECT, STAT_SPIRIT,
  STAT_HEALTH, STAT_MANA, STAT_RAGE, STAT_ENERGY, STAT_FOCUS, STAT_RUNIC,
  STAT_SPELL_POWER, STAT_SPELL_PENETRATION, STAT_MP5,
  STAT_ATTACK_POWER, STAT_EXPERTISE_RATING, STAT_ARMOR_PENETRATION_RATING,
  STAT_HIT_RATING, STAT_CRIT_RATING, STAT_HASTE_RATING,
  STAT_WEAPON_DPS, STAT_WEAPON_SPEED,
  STAT_ARMOR, STAT_DEFENSE_RATING, STAT_DODGE_RATING, STAT_PARRY_RATING, 
  STAT_BLOCK_RATING, STAT_BLOCK_VALUE,
  STAT_MAX
};

enum elixir_type
{
  ELIXIR_NONE=0,
  ELIXIR_DRAENIC_WISDOM,
  ELIXIR_MAJOR_MAGEBLOOD,
  ELIXIR_GUARDIAN,
  ELIXIR_ADEPT,
  ELIXIR_FEL_STRENGTH,
  ELIXIR_GREATER_ARCANE,
  ELIXIR_MAJOR_AGILITY,
  ELIXIR_MAJOR_FIRE_POWER,
  ELIXIR_MAJOR_FROST_POWER,
  ELIXIR_MAJOR_SHADOW_POWER,
  ELIXIR_MAJOR_STRENGTH,
  ELIXIR_MASTERY,
  ELIXIR_MONGOOSE,
  ELIXIR_ONSLAUGHT,
  ELIXIR_SAGES,
  ELIXIR_BATTLE,
  ELIXIR_MAX
};

enum flask_type
{
  FLASK_NONE=0,
  FLASK_BLINDING_LIGHT,
  FLASK_DISTILLED_WISDOM,
  FLASK_ENDLESS_RAGE,
  FLASK_FROST_WYRM,
  FLASK_MIGHTY_RESTORATION,
  FLASK_PURE_DEATH,
  FLASK_RELENTLESS_ASSAULT,
  FLASK_SUPREME_POWER,
  FLASK_MAX
};

enum food_type
{
  FOOD_NONE=0,
  FOOD_TENDER_SHOVELTUSK_STEAK,
  FOOD_SMOKED_SALMON,
  FOOD_SNAPPER_EXTREME,
  FOOD_POACHED_BLUEFISH,
  FOOD_BLACKENED_BASILISK,
  FOOD_GOLDEN_FISHSTICKS,
  FOOD_CRUNCHY_SERPENT,
  FOOD_BLACKENED_DRAGONFIN,
  FOOD_DRAGONFIN_FILET,
  FOOD_HEARTY_RHINO,
  FOOD_GREAT_FEAST,
  FOOD_FISH_FEAST,
  FOOD_MAX
};

enum position_type { POSITION_NONE=0, POSITION_FRONT, POSITION_BACK, POSITION_RANGED, POSITION_MAX };

enum profession_type
{
  PROFESSION_NONE=0,
  PROF_ALCHEMY,
  PROF_MINING,
  PROF_HERBALISM,
  PROF_LEATHERWORKING,
  PROF_ENGINEERING,
  PROF_BLACKSMITHING,
  PROF_INSCRIPTION,
  PROF_SKINNING,
  PROF_TAILORING,
  PROF_JEWELCRAFTING,
  PROF_ENCHANTING,
  PROFESSION_MAX
};

enum role_type { ROLE_NONE=0, ROLE_ATTACK, ROLE_SPELL, ROLE_TANK, ROLE_HYBRID, ROLE_MAX };

enum rng_type
{
  // Specifies the general class of RNG desired
  RNG_DEFAULT=0,     // Do not care/know where it will be used
  RNG_GLOBAL,        // Returns reference to global RNG on sim_t
  RNG_DETERMINISTIC, // Returns reference to global deterministic RNG on sim_t
  RNG_CYCLIC,        // Normalized even/periodical results are acceptable
  RNG_DISTRIBUTED,   // Normalized variable/distributed values should be returned

  // Specifies a particular RNG desired
  RNG_STANDARD,          // Creates RNG using srand() and rand()
  RNG_MERSENNE_TWISTER,  // Creates RNG using SIMD oriented Fast Mersenne Twister
  RNG_PHASE_SHIFT,       // Simplistic cycle-based RNG, unsuitable for overlapping procs
  RNG_DISTANCE_SIMPLE,   // Simple normalized proc-separation RNG, suitable for fixed proc chance
  RNG_DISTANCE_BANDS,    // Complex normalized proc-separation RNG, suitable for varying proc chance
  RNG_PRE_FILL,          // Deterministic number of procs with random distribution
  RNG_MAX
};

enum save_type
{
  // Specifies the type of profile data to be saved
  SAVE_ALL=0,
  SAVE_GEAR,
  SAVE_TALENTS,
  SAVE_ACTIONS,
  SAVE_MAX
};

enum format_type
{
  FORMAT_NONE=0,
  FORMAT_NAME,
  FORMAT_CHAR_NAME,
  FORMAT_CONVERT_HEX,
  FORMAT_CONVERT_UTF8,
  FORMAT_MAX
};

#define FORMAT_CHAR_NAME_MASK  ( (1<<FORMAT_NAME) | (1<<FORMAT_CHAR_NAME) )
#define FORMAT_GUILD_NAME_MASK ( (1<<FORMAT_NAME) )
#define FORMAT_ALL_NAME_MASK   ( (1<<FORMAT_NAME) | (1<<FORMAT_CHAR_NAME) )
#define FORMAT_UTF8_MASK       ( (1<<FORMAT_CONVERT_HEX) | (1<<FORMAT_CONVERT_UTF8) )
#define FORMAT_ASCII_MASK      ( (1<<FORMAT_CONVERT_UTF8) )
#define FORMAT_CONVERT_MASK    ( (1<<FORMAT_CONVERT_HEX) | (1<<FORMAT_CONVERT_UTF8) )
#define FORMAT_DEFAULT         ( FORMAT_ASCII_MASK )
#define FORMAT_ALL_MASK        -1

// Options ====================================================================

enum option_type_t
{
  OPT_NONE=0,
  OPT_STRING,     // std::string*
  OPT_APPEND,     // std::string* (append)
  OPT_CHARP,      // char*
  OPT_BOOL,       // int (only valid values are 1 and 0)
  OPT_INT,        // int
  OPT_FLT,        // double
  OPT_LIST,       // std::vector<std::string>*
  OPT_FUNC,       // function pointer
  OPT_DEPRECATED,
  OPT_UNKNOWN
};

// Expression Operation types ==================================================
enum exp_res_t    { ETP_NONE, ETP_BOOL, ETP_NUM };

enum exp_type { AEXP_NONE=0,
                AEXP_AND, AEXP_OR, AEXP_NOT, AEXP_EQ, AEXP_NEQ, AEXP_GREATER, AEXP_LESS, AEXP_GE, AEXP_LE, // these operations result in boolean
                AEXP_PLUS, AEXP_MINUS, AEXP_MUL, AEXP_DIV, // these operations result in double
                AEXP_VALUE, AEXP_INT_PTR, AEXP_DOUBLE_PTR, // these are "direct" value return (no operations)
                AEXP_BUFF, AEXP_FUNC, // these presume overriden class
                AEXP_MAX
              };


typedef bool ( *option_function_t )( sim_t* sim, const std::string& name, const std::string& value );

struct option_t
{
  const char* name;
  int type;
  void* address;

  void print( FILE* );
  void save ( FILE* );
  bool parse( sim_t*, const std::string& name, const std::string& value );

  static void copy( std::vector<option_t>& opt_vector, option_t* opt_array );
  static bool parse( sim_t*, std::vector<option_t>&, const std::string& name, const std::string& value );
  static bool parse( sim_t*, const char* context, std::vector<option_t>&, const std::string& options_str );
  static bool parse( sim_t*, const char* context, option_t*,              const std::string& options_str );
  static bool parse_file( sim_t*, FILE* file );
  static bool parse_line( sim_t*, char* line );
  static bool parse_token( sim_t*, std::string& token );
};

// Utilities =================================================================

struct util_t
{
  static double talent_rank( int num, int max, double increment );
  static double talent_rank( int num, int max, double value1, double value2, ... );

  static int talent_rank( int num, int max, int increment );
  static int talent_rank( int num, int max, int value1, int value2, ... );

  static double ability_rank( int player_level, double ability_value, int ability_level, ... );
  static int    ability_rank( int player_level, int    ability_value, int ability_level, ... );

  static char* dup( const char* );

  static const char* attribute_type_string     ( int type );
  static const char* dmg_type_string           ( int type );
  static const char* elixir_type_string        ( int type );
  static const char* flask_type_string         ( int type );
  static const char* food_type_string          ( int type );
  static const char* gem_type_string           ( int type );
  static const char* meta_gem_type_string      ( int type );
  static const char* player_type_string        ( int type );
  static const char* profession_type_string    ( int type );
  static const char* race_type_string          ( int type );
  static const char* resource_type_string      ( int type );
  static const char* result_type_string        ( int type );
  static const char* school_type_string        ( int type );
  static const char* slot_type_string          ( int type );
  static const char* stat_type_string          ( int type );
  static const char* stat_type_abbrev          ( int type );
  static const char* stat_type_wowhead         ( int type );
  static const char* talent_tree_string        ( int tree );
  static const char* weapon_type_string        ( int type );

  static int parse_attribute_type     ( const std::string& name );
  static int parse_dmg_type           ( const std::string& name );
  static int parse_elixir_type        ( const std::string& name );
  static int parse_flask_type         ( const std::string& name );
  static int parse_food_type          ( const std::string& name );
  static int parse_gem_type           ( const std::string& name );
  static int parse_meta_gem_type      ( const std::string& name );
  static int parse_player_type        ( const std::string& name );
  static int parse_profession_type    ( const std::string& name );
  static int parse_race_type          ( const std::string& name );
  static int parse_resource_type      ( const std::string& name );
  static int parse_result_type        ( const std::string& name );
  static int parse_school_type        ( const std::string& name );
  static int parse_slot_type          ( const std::string& name );
  static int parse_stat_type          ( const std::string& name );
  static int parse_talent_tree        ( const std::string& name );
  static int parse_weapon_type        ( const std::string& name );

  static const char* class_id_string( int type );
  static int translate_class_id( int cid );
  static int translate_race_id( int rid );
  static bool socket_gem_match( int socket, int gem );

  static int string_split( std::vector<std::string>& results, const std::string& str, const char* delim, bool allow_quotes = false );
  static int string_split( const std::string& str, const char* delim, const char* format, ... );

  static int64_t milliseconds();
  static int64_t parse_date( const std::string& month_day_year );

  static int printf( const char *format,  ... );
  static int fprintf( FILE *stream, const char *format,  ... );

  static std::string& utf8_binary_to_hex( std::string& name );
  static std::string& ascii_binary_to_utf8_hex( std::string& name );
  static std::string& utf8_hex_to_ascii( std::string& name );
  static std::string& format_name( std::string& name );

  static void add_base_stats( base_stats_t& result, base_stats_t& a, base_stats_t b );
};

// Event =====================================================================

struct event_t
{
  event_t*  next;
  sim_t*    sim;
  player_t* player;
  uint32_t  id;
  double    time;
  double    reschedule_time;
  int       canceled;
  const char* name;
  event_t( sim_t* s, player_t* p=0, const char* n=0 ) :
      next( 0 ), sim( s ), player( p ), reschedule_time( 0 ), canceled( 0 ), name( n )
  {
    if ( ! name ) name = "unknown";
  }
  double occurs() SC_CONST { return reschedule_time != 0 ? reschedule_time : time; }
  virtual void reschedule( double new_time );
  virtual void execute() { util_t::printf( "%s\n", name ? name : "(no name)" ); assert( 0 ); }
  virtual ~event_t() {}
  static void cancel( event_t*& e ) { if ( e ) { e -> canceled = 1;                 e=0; } }
  static void  early( event_t*& e ) { if ( e ) { e -> canceled = 1; e -> execute(); e=0; } }
  // Simple free-list memory manager.
  static void* operator new( size_t, sim_t* );
  static void* operator new( size_t ) throw();  // DO NOT USE!
  static void  operator delete( void* );
  static void  operator delete( void*, sim_t* ) {}
  static void deallocate( event_t* e );
};

struct event_compare_t
{
  bool operator () ( event_t* lhs, event_t* rhs ) SC_CONST
  {
    return( lhs -> time == rhs -> time ) ? ( lhs -> id > rhs -> id ) : ( lhs -> time > rhs -> time );
  }
};

// Raid Event

struct raid_event_t
{
  sim_t* sim;
  std::string name_str;
  int num_starts;
  double first, last;
  double cooldown;
  double cooldown_stddev;
  double cooldown_min;
  double cooldown_max;
  double duration;
  double duration_stddev;
  double duration_min;
  double duration_max;
  double distance_min;
  double distance_max;
  rng_t* rng;
  std::vector<player_t*> affected_players;

  raid_event_t( sim_t*, const char* name );
  virtual ~raid_event_t() {}

  virtual double cooldown_time() SC_CONST;
  virtual double duration_time() SC_CONST;
  virtual void schedule();
  virtual void reset();
  virtual void start();
  virtual void finish() {}
  virtual void parse_options( option_t*, const std::string& options_str );
  virtual const char* name() SC_CONST { return name_str.c_str(); }
  static raid_event_t* create( sim_t* sim, const std::string& name, const std::string& options_str );
  static void init( sim_t* );
  static void reset( sim_t* );
  static void combat_begin( sim_t* );
  static void combat_end( sim_t* ) {}
};

// Gear Stats ================================================================

struct gear_stats_t
{
  double attribute[ ATTRIBUTE_MAX ];
  double resource[ RESOURCE_MAX ];
  double spell_power;
  double spell_penetration;
  double mp5;
  double attack_power;
  double expertise_rating;
  double armor_penetration_rating;
  double hit_rating;
  double crit_rating;
  double haste_rating;
  double weapon_dps;
  double weapon_speed;
  double armor;
  double defense;
  double dodge;
  double parry;
  double block;
  double block_value;

  gear_stats_t() { memset( ( void* ) this, 0x00, sizeof( gear_stats_t ) ); }

  void   add_stat( int stat, double value );
  void   set_stat( int stat, double value );
  double get_stat( int stat ) SC_CONST;
  void   print( FILE* );
  static double stat_mod( int stat );
};

// Buffs ======================================================================

struct buff_t
{
  sim_t* sim;
  player_t* player;
  std::string name_str;
  std::vector<std::string> aura_str;
  std::vector<double> stack_occurrence;
  int current_stack, max_stack;
  double current_value, react, duration, cooldown, cooldown_ready, default_chance;
  double last_start, last_trigger, start_intervals_sum, trigger_intervals_sum, uptime_sum;
  int64_t up_count, down_count, start_intervals, trigger_intervals, start_count, refresh_count;
  int64_t trigger_attempts, trigger_successes;
  double uptime_pct, benefit_pct, trigger_pct, avg_start_interval, avg_trigger_interval, avg_start, avg_refresh;
  bool constant, quiet;
  int aura_id;
  event_t* expiration;
  rng_t* rng;
  buff_t* next;

  buff_t() : sim( 0 ) {}
  virtual ~buff_t() { };

  // Raid Aura
  buff_t( sim_t*, const std::string& name,
          int max_stack=1, double duration=0, double cooldown=0,
          double chance=1.0, bool quiet=false, int rng_type=RNG_CYCLIC, int aura_id=0 );

  // Player Buff
  buff_t( player_t*, const std::string& name,
          int max_stack=1, double duration=0, double cooldown=0,
          double chance=1.0, bool quiet=false, int rng_type=RNG_CYCLIC, int aura_id=0 );

  // Use check() inside of ready() methods to prevent skewing of "benefit" calculations.
  // Use up() where the presence of the buff affects the action mechanics.

  virtual int    check() { return current_stack; }
  virtual bool   up()    { if( current_stack > 0 ) { up_count++; } else { down_count++; } return current_stack > 0; }
  virtual int    stack() { if( current_stack > 0 ) { up_count++; } else { down_count++; } return current_stack; }
  virtual double value() { if( current_stack > 0 ) { up_count++; } else { down_count++; } return current_value; }
  virtual double remains();
  virtual bool   remains_gt( double time );
  virtual bool   remains_lt( double time );
  virtual bool   may_react( int stacks=0 );
  virtual bool   trigger  ( int stacks=1, double value=-1.0, double chance=-1.0 );
  virtual void   increment( int stacks=1, double value=-1.0 );
  virtual void   decrement( int stacks=1, double value=-1.0 );
  virtual void   start    ( int stacks=1, double value=-1.0 );
  virtual void   refresh  ( int stacks=0, double value=-1.0 );
  virtual void   override ( int stacks=1, double value=-1.0 );
  virtual void   expire();
  virtual void   reset();
  virtual void   aura_gain();
  virtual void   aura_loss();
  virtual void   merge( buff_t* other_buff );
  virtual void   analyze();
  virtual void   init();
  virtual const char* name() { return name_str.c_str(); }

  static buff_t* find(    sim_t*, const std::string& name );
  static buff_t* find( player_t*, const std::string& name );
};

struct stat_buff_t : public buff_t
{
  int stat;
  double amount;
  stat_buff_t( player_t*, const std::string& name,
	       int stat, double amount,
	       int max_stack=1, double duration=0, double cooldown=0,
	       double chance=1.0, bool quiet=false, int rng_type=RNG_CYCLIC, int aura_id=0 );
  virtual ~stat_buff_t() { };
  virtual void start    ( int stacks=1, double value=-1.0 );
  virtual void refresh  ( int stacks=0, double value=-1.0 );
  virtual void decrement( int stacks=1, double value=-1.0 );
  virtual void expire();
};

struct debuff_t : public buff_t
{
  debuff_t( sim_t*, const std::string& name,
	    int max_stack=1, double duration=0, double cooldown=0,
	    double chance=1.0, bool quiet=false, int rng_type=RNG_CYCLIC, int aura_id=0 );
  virtual void aura_gain();
  virtual void aura_loss();
};

typedef struct buff_t aura_t;

// Expressions =================================================================

// definition of operators
struct operator_def_t
{
  std::string name;
  int type;
  bool binary;
  exp_res_t result_type;
  exp_res_t operand_type;
};

struct act_expression_t
{
  int    type;
  double value;
  void   *p_value;
  std::string exp_str;
  act_expression_t* operand_1;
  act_expression_t* operand_2;
  act_expression_t( int e_type, std::string expression_str="", double e_value=0 );
  std::string to_string( int level=0 );
  virtual ~act_expression_t() {}
  virtual double evaluate();
  virtual bool   ok();
  static act_expression_t* create( action_t* action, std::string& expression, std::string alias_protect, exp_res_t expected_type );
  static void warn( int severity, action_t* action, const char* format, ... );
  static act_expression_t* find_operator( action_t* action, std::string& unmasked, std::string& expression, std::string& alias_protect,
                                          operator_def_t& op );
  static std::string op_name( int op_type );

};

struct oldbuff_expression_t: public act_expression_t
{
  event_t** expiration_ptr;
  int* int_ptr;
  double* double_ptr;
  oldbuff_expression_t( std::string expression_str, void* value_ptr, event_t** e_expiration=0, int ptr_type=1 );
  virtual ~oldbuff_expression_t() {};
  virtual double evaluate();
};

// Alias support =============================================================
struct alias_t
{
  std::string alias_str;
  std::vector<std::string> alias_name;
  std::vector<std::string> alias_value;
  alias_t() {}
  virtual ~alias_t() {}
  virtual void add( std::string& name, std::string& value );
  virtual void init_parse();
  virtual std::string find( std::string& name );
};

// Simulation Engine =========================================================

struct sim_t
{
  int         argc;
  char**      argv;
  sim_t*      parent;
  patch_t     patch;
  int         P322;
  event_t*    free_list;
  target_t*   target;
  player_t*   player_list;
  player_t*   active_player;
  int         num_players;
  double      queue_lag, queue_lag_stddev;
  double      gcd_lag, gcd_lag_stddev;
  double      channel_lag, channel_lag_stddev;
  double      queue_gcd_reduction;
  double      travel_variance, default_skill, reaction_time, regen_periodicity;
  double      current_time, max_time;
  int         events_remaining, max_events_remaining;
  int         events_processed, total_events_processed;
  int         seed, id, iterations, current_iteration;
  int         infinite_resource[ RESOURCE_MAX ];
  int         armor_update_interval;
  int         optimal_raid, spell_crit_suppression, log, debug;
  int         report_buffed_stats, save_profiles;
  std::string default_region_str, default_server_str;
  alias_t     alias;

  // Default stat enchants
  gear_stats_t enchant;

  std::vector<option_t> options;
  std::vector<std::string> party_encoding;

  // Random Number Generation
  rng_t* rng;
  rng_t* deterministic_rng;
  rng_t* rng_list;
  int smooth_rng, deterministic_roll, average_range, average_gauss;

  // Timing Wheel Event Management
  event_t** timing_wheel;
  int    wheel_seconds, wheel_size, wheel_mask, timing_slice;
  double wheel_granularity;

  // Raid Events
  std::vector<raid_event_t*> raid_events;
  std::string raid_events_str;

  // Buffs and Debuffs Overrides
  struct overrides_t
  {
    int abominations_might;
    int arcane_brilliance;
    int battle_shout;
    int bleeding;
    int blessing_of_kings;
    int blessing_of_might;
    int blessing_of_wisdom;
    int blood_frenzy;
    int bloodlust;
    int bloodlust_early;
    int celerity;
    int crypt_fever;
    int curse_of_elements;
    int divine_spirit;
    int earth_and_moon;
    int elemental_oath;
    int expose_armor;
    int faerie_fire;
    int ferocious_inspiration;
    int flametongue_totem;
    int focus_magic;
    int fortitude;
    int heart_of_the_crusader;
    int heroic_presence;
    int hunters_mark;
    int improved_faerie_fire;
    int improved_moonkin_aura;
    int improved_scorch;
    int improved_shadow_bolt;
    int insect_swarm;
    int judgement_of_wisdom;
    int judgements_of_the_just;
    int leader_of_the_pack;
    int mana_spring_totem;
    int mangle;
    int mark_of_the_wild;
    int master_poisoner;
    int misery;
    int moonkin_aura;
    int poisoned;
    int rampage;
    int replenishment;
    int sanctified_retribution;
    int savage_combat;
    int scorpid_sting;
    int strength_of_earth;
    int sunder_armor;
    int swift_retribution;
    int thunder_clap;
    int totem_of_wrath;
    int trauma;
    int trueshot_aura;
    int unleashed_rage;
    int windfury_totem;
    int winters_chill;
    int wrath_of_air;
    overrides_t() { memset( ( void* ) this, 0x0, sizeof( overrides_t ) ); }
  };
  overrides_t overrides;

  // Auras
  struct auras_t
  {
    aura_t* abominations_might;
    aura_t* battle_shout;
    aura_t* celerity;
    aura_t* elemental_oath;
    aura_t* ferocious_inspiration;
    aura_t* flametongue_totem;
    aura_t* improved_moonkin;
    aura_t* leader_of_the_pack;
    aura_t* mana_spring_totem;
    aura_t* moonkin;
    aura_t* rampage;
    aura_t* sanctified_retribution;
    aura_t* strength_of_earth;
    aura_t* swift_retribution;
    aura_t* totem_of_wrath;
    aura_t* trueshot;
    aura_t* unleashed_rage;
    aura_t* windfury_totem;
    aura_t* wrath_of_air;
    auras_t() { memset( (void*) this, 0x0, sizeof( auras_t ) ); }
  };
  auras_t auras;

  // Auras and De-Buffs
  buff_t* buff_list;

  // Replenishment
  int replenishment_targets;
  std::vector<player_t*> replenishment_candidates;

  // Reporting
  report_t*  report;
  scaling_t* scaling;
  double     raid_dps, total_dmg, total_seconds, elapsed_cpu_seconds;
  int        merge_ignite;
  int        report_progress;
  std::string reference_player_str;
  std::vector<player_t*> players_by_rank;
  std::vector<player_t*> players_by_name;
  std::vector<std::string> id_dictionary;
  std::vector<std::string> dps_charts, gear_charts, dpet_charts;
  std::string downtime_chart;
  std::string output_file_str, log_file_str, html_file_str, wiki_file_str, xml_file_str;
  std::deque<std::string> active_files;
  FILE* output_file;
  FILE* log_file;
  int armory_throttle;
  int current_throttle;
  int duration_uptimes;
  int debug_exp;

  // Multi-Threading
  int threads;
  std::vector<sim_t*> children;
  void* thread_handle;
  int  thread_index;

  sim_t( sim_t* parent=0, int thrdID=0 );
  virtual ~sim_t();

  int       main( int argc, char** argv );
  void      combat( int iteration );
  void      combat_begin();
  void      combat_end();
  void      add_event( event_t*, double delta_time );
  void      reschedule_event( event_t* );
  void      flush_events();
  void      cancel_events( player_t* );
  event_t*  next_event();
  void      reset();
  bool      init();
  void      analyze();
  void      merge( sim_t& other_sim );
  void      merge();
  void      iterate();
  void      partition();
  void      execute();
  void      print_options();
  std::vector<option_t>& get_options();
  bool      parse_option( const std::string& name, const std::string& value );
  bool      parse_options( int argc, char** argv );
  bool      time_to_think( double proc_time ) { if ( proc_time == 0 ) return false; return current_time - proc_time > reaction_time; }
  bool      cooldown_ready( double cooldown_time ) { return cooldown_time <= current_time; }
  int       roll( double chance );
  double    range( double min, double max );
  double    gauss( double mean, double stddev );
  rng_t*    get_rng( const std::string& name, int type=RNG_DEFAULT );
  player_t* find_player( const std::string& name );
  void      use_optimal_buffs_and_debuffs( int value );
  void      aura_gain( const char* name, int aura_id=0 );
  void      aura_loss( const char* name, int aura_id=0 );
};

// Scaling ===================================================================

struct scaling_t
{
  sim_t* sim;
  int    scale_stat;
  double scale_value;
  int    calculate_scale_factors;
  int    center_scale_delta;
  int    scale_lag;
  double scale_factor_noise;
  int    normalize_scale_factors;
  int    smooth_scale_factors;
  int    debug_scale_factors;
  std::string scale_only_str;

  // Gear delta for determining scale factors
  gear_stats_t stats;

  scaling_t( sim_t* s );

  void init_deltas();
  void analyze();
  void analyze_stats();
  void analyze_lag();
  void analyze_gear_weights();
  void normalize();
  void derive();
  int  get_options( std::vector<option_t>& );
};

// Gear Rating Conversions ===================================================

struct rating_t
{
  double  spell_haste,  spell_hit,  spell_crit;
  double attack_haste, attack_hit, attack_crit;
  double expertise, armor_penetration;
  double defense, dodge, parry, block;
  rating_t() { memset( this, 0x00, sizeof( rating_t ) ); }
  void init( sim_t*, int level );
  static double interpolate( int level, double val_60, double val_70, double val_80 );
  static double get_attribute_base( int level, int class_type, int race, int stat_type );
};

// Weapon ====================================================================

struct weapon_t
{
  int    type, school;
  double damage, dps;
  double swing_time;
  int    slot;
  int    buff_type;
  double buff_value;

  int    group() SC_CONST;
  double normalized_weapon_speed() SC_CONST;
  double proc_chance_on_swing( double PPM, double adjusted_swing_time=0 ) SC_CONST;

  weapon_t( int t=WEAPON_NONE, double d=0, double st=2.0, int s=SCHOOL_PHYSICAL ) :
      type(t), school(s), damage(d), swing_time(st), slot(SLOT_NONE), buff_type(0), buff_value(0) {}
};

// Item ======================================================================

struct item_t
{
  sim_t* sim;
  player_t* player;
  int slot;
  bool unique, unique_enchant;

  // Option Data
  std::string option_name_str;
  std::string option_id_str;
  std::string option_stats_str;
  std::string option_gems_str;
  std::string option_enchant_str;
  std::string option_equip_str;
  std::string option_use_str;
  std::string option_weapon_str;
  std::string options_str;

  // Armory Data
  std::string armory_name_str;
  std::string armory_id_str;
  std::string armory_stats_str;
  std::string armory_gems_str;
  std::string armory_enchant_str;
  std::string armory_weapon_str;

  // Encoded Data
  std::string id_str;
  std::string encoded_name_str;
  std::string encoded_stats_str;
  std::string encoded_gems_str;
  std::string encoded_enchant_str;
  std::string encoded_equip_str;
  std::string encoded_use_str;
  std::string encoded_weapon_str;

  // Extracted data
  gear_stats_t stats;
  struct special_effect_t
  {
    std::string name_str, trigger_str;
    int trigger_type, trigger_mask;
    int stat, school, max_stacks;
    double amount, proc_chance, duration, cooldown;
    special_effect_t() :
        trigger_type( 0 ), trigger_mask( 0 ), stat( 0 ), school( 0 ),
        max_stacks( 0 ), amount( 0 ), proc_chance( 0 ), duration( 0 ), cooldown( 0 ) {}
    bool active() { return stat || school; }
  } use, equip, enchant;

  item_t() : sim( 0 ), player( 0 ), slot( SLOT_NONE ), unique( false ), unique_enchant( false ) {}
  item_t( player_t*, const std::string& options_str );
  bool active() SC_CONST;
  const char* name() SC_CONST;
  const char* slot_name() SC_CONST;
  weapon_t* weapon() SC_CONST;
  bool init();
  bool parse_options();
  void encode_options();
  bool decode_stats();
  bool decode_gems();
  bool decode_enchant();
  bool decode_special( special_effect_t&, const std::string& encoding );
  bool decode_weapon();

  static bool download_slot( item_t&, const std::string& item_id, const std::string& enchant_id, const std::string gem_ids[ 3 ] );
  static bool download_item( item_t&, const std::string& item_id );
  static bool download_glyph( sim_t* sim, std::string& glyph_name, const std::string& glyph_id );
  static int  parse_gem( item_t&            item,
                         const std::string& gem_id );
};

// Set Bonus =================================================================

struct set_bonus_t
{
  int count[ SET_MAX ];
  int tier6_2pc_caster(); int tier6_2pc_melee(); int tier6_2pc_tank();
  int tier6_4pc_caster(); int tier6_4pc_melee(); int tier6_4pc_tank();
  int tier7_2pc_caster(); int tier7_2pc_melee(); int tier7_2pc_tank();
  int tier7_4pc_caster(); int tier7_4pc_melee(); int tier7_4pc_tank();
  int tier8_2pc_caster(); int tier8_2pc_melee(); int tier8_2pc_tank();
  int tier8_4pc_caster(); int tier8_4pc_melee(); int tier8_4pc_tank();
  int tier9_2pc_caster(); int tier9_2pc_melee(); int tier9_2pc_tank();
  int tier9_4pc_caster(); int tier9_4pc_melee(); int tier9_4pc_tank();
  int tier10_2pc_caster(); int tier10_2pc_melee(); int tier10_2pc_tank();
  int tier10_4pc_caster(); int tier10_4pc_melee(); int tier10_4pc_tank();
  int spellstrike();
  int decode( player_t*, item_t& item ) SC_CONST;
  bool init( player_t* );
  set_bonus_t();
};

// Player ====================================================================

struct player_t
{
  sim_t*      sim;
  std::string name_str, talents_str, glyphs_str, id_str;
  std::string region_str, server_str, origin_str;
  player_t*   next;
  int         index, type, level, tank, party, member;
  double      skill, distance, gcd_ready, base_gcd;
  int         potion_used, sleeping, initialized;
  rating_t    rating;
  pet_t*      pet_list;
  int64_t     last_modified;

  // Option Parsing
  std::vector<option_t> options;

  // Profs
  std::string professions_str;
  int profession[ PROFESSION_MAX ];

  // Race
  std::string race_str;
  int race;

  // Haste
  double base_haste_rating, initial_haste_rating, haste_rating;
  double spell_haste, attack_haste;

  // Attributes
  double attribute                   [ ATTRIBUTE_MAX ];
  double attribute_base              [ ATTRIBUTE_MAX ];
  double attribute_initial           [ ATTRIBUTE_MAX ];
  double attribute_multiplier        [ ATTRIBUTE_MAX ];
  double attribute_multiplier_initial[ ATTRIBUTE_MAX ];

  // Spell Mechanics
  double base_spell_power,       initial_spell_power[ SCHOOL_MAX+1 ], spell_power[ SCHOOL_MAX+1 ];
  double base_spell_hit,         initial_spell_hit,                   spell_hit;
  double base_spell_crit,        initial_spell_crit,                  spell_crit;
  double base_spell_penetration, initial_spell_penetration,           spell_penetration;
  double base_mp5,               initial_mp5,                         mp5;
  double spell_power_multiplier,    initial_spell_power_multiplier;
  double spell_power_per_intellect, initial_spell_power_per_intellect;
  double spell_power_per_spirit,    initial_spell_power_per_spirit;
  double spell_crit_per_intellect,  initial_spell_crit_per_intellect;
  double mp5_per_intellect;
  double mana_regen_base;
  double mana_regen_while_casting;
  double energy_regen_per_second;
  double focus_regen_per_second;
  double last_cast;

  // Attack Mechanics
  double base_attack_power,       initial_attack_power,        attack_power;
  double base_attack_hit,         initial_attack_hit,          attack_hit;
  double base_attack_expertise,   initial_attack_expertise,    attack_expertise;
  double base_attack_crit,        initial_attack_crit,         attack_crit;
  double base_attack_penetration, initial_attack_penetration,  attack_penetration;
  double attack_power_multiplier,   initial_attack_power_multiplier;
  double attack_power_per_strength, initial_attack_power_per_strength;
  double attack_power_per_agility,  initial_attack_power_per_agility;
  double attack_crit_per_agility,   initial_attack_crit_per_agility;
  int    position;

  // Defense Mechanics
  event_t* target_auto_attack;
  double base_armor,       initial_armor,       armor, armor_snapshot;
  double base_defense,     initial_defense,     defense;
  double base_dodge,       initial_dodge,       dodge;
  double base_parry,       initial_parry,       parry;
  double base_block,       initial_block,       block;
  double base_block_value, initial_block_value, block_value;
  double armor_per_agility, initial_armor_per_agility;
  double dodge_per_agility, initial_dodge_per_agility;
  bool   use_armor_snapshot;

  // Weapons
  weapon_t main_hand_weapon;
  weapon_t off_hand_weapon;
  weapon_t ranged_weapon;

  // Resources
  double  resource_base   [ RESOURCE_MAX ];
  double  resource_initial[ RESOURCE_MAX ];
  double  resource_max    [ RESOURCE_MAX ];
  double  resource_current[ RESOURCE_MAX ];
  double  mana_per_intellect;
  double  health_per_stamina;

  // Replenishment
  std::vector<player_t*> replenishment_targets;

  // Consumables
  std::string flask_str, elixirs_str, food_str;
  int elixir_guardian;
  int elixir_battle;
  int flask;
  int food;

  // Events
  action_t* executing;
  action_t* channeling;
  event_t*  readying;
  bool      in_combat;
  bool      action_queued;

  // Callbacks
  std::vector<action_callback_t*> resource_gain_callbacks       [ RESOURCE_MAX ];
  std::vector<action_callback_t*> resource_loss_callbacks       [ RESOURCE_MAX ];
  std::vector<action_callback_t*> attack_result_callbacks       [ RESULT_MAX ];
  std::vector<action_callback_t*> spell_result_callbacks        [ RESULT_MAX ];
  std::vector<action_callback_t*> attack_direct_result_callbacks[ RESULT_MAX ];
  std::vector<action_callback_t*> spell_direct_result_callbacks [ RESULT_MAX ];
  std::vector<action_callback_t*> tick_callbacks;
  std::vector<action_callback_t*> tick_damage_callbacks;
  std::vector<action_callback_t*> direct_damage_callbacks;

  // Action Priority List
  action_t*   action_list;
  std::string action_list_str;
  std::string action_list_skip;
  int         action_list_default;

  // Reporting
  int       quiet;
  action_t* last_foreground_action;
  double    current_time, total_seconds;
  double    total_waiting;
  double    iteration_dmg, total_dmg;
  double    resource_lost  [ RESOURCE_MAX ];
  double    resource_gained[ RESOURCE_MAX ];
  double    dps, dps_min, dps_max, dps_std_dev, dps_error;
  double    dpr, rps_gain, rps_loss;
  buff_t*   buff_list;
  proc_t*   proc_list;
  gain_t*   gain_list;
  stats_t*  stats_list;
  uptime_t* uptime_list;
  std::vector<double> timeline_resource;
  std::vector<double> timeline_dmg;
  std::vector<double> timeline_dps;
  std::vector<double> iteration_dps;
  std::vector<int> distribution_dps;
  std::string action_dpet_chart, action_dmg_chart, gains_chart;
  std::string timeline_resource_chart, timeline_dps_chart, distribution_dps_chart;
  std::string gear_weights_lootrank_link, gear_weights_wowhead_link, gear_weights_pawn_string;
  std::string save_str;
  std::string save_gear_str;
  std::string save_talents_str;
  std::string save_actions_str;
  std::string comment_str;

  // Gear
  std::string items_str, meta_gem_str;
  std::vector<item_t> items;
  gear_stats_t stats, gear, enchant;
  set_bonus_t set_bonus;
  int meta_gem;

  // Scale Factors
  gear_stats_t scaling;
  gear_stats_t normalized_scaling;
  int          normalized_to;
  double       scaling_lag;
  int          scales_with[ STAT_MAX ];

  struct buffs_t
  {
    buff_t* arcane_brilliance;
    buff_t* berserking; 
    buff_t* blessing_of_kings;
    buff_t* blessing_of_might;
    buff_t* blessing_of_wisdom;
    buff_t* blood_fury_ap;
    buff_t* blood_fury_sp;
    buff_t* bloodlust;
    buff_t* demonic_pact;
    buff_t* divine_spirit;
    buff_t* focus_magic;
    buff_t* fortitude;
    buff_t* heroic_presence;
    buff_t* innervate;
    buff_t* mark_of_the_wild;
    buff_t* mongoose_mh;
    buff_t* mongoose_oh;
    buff_t* moving;
    buff_t* power_infusion;
    buff_t* hysteria;
    buff_t* replenishment;
    buff_t* stoneform;
    buff_t* stunned;
    buff_t* tricks_of_the_trade;
    buffs_t() { memset( (void*) this, 0x0, sizeof( buffs_t ) ); }
  };
  buffs_t buffs;

  struct gains_t
  {
    gain_t* arcane_torrent;
    gain_t* blessing_of_wisdom;
    gain_t* dark_rune;
    gain_t* energy_regen;
    gain_t* focus_regen;
    gain_t* innervate;
    gain_t* glyph_of_innervate;
    gain_t* judgement_of_wisdom;
    gain_t* mana_potion;
    gain_t* mana_spring_totem;
    gain_t* mana_tide;
    gain_t* mp5_regen;
    gain_t* replenishment;
    gain_t* restore_mana;
    gain_t* spellsurge;
    gain_t* spirit_intellect_regen;
    gain_t* vampiric_embrace;
    gain_t* vampiric_touch;
    gain_t* water_elemental;
    void reset() { memset( ( void* ) this, 0x00, sizeof( gains_t ) ); }
    gains_t() { reset(); }
  };
  gains_t gains;

  struct procs_t
  {
    proc_t* hat_donor;
    void reset() { memset( ( void* ) this, 0x00, sizeof( procs_t ) ); }
    procs_t() { reset(); }
  };
  procs_t procs;

  rng_t* rng_list;

  struct rngs_t
  {
    rng_t* lag_channel;
    rng_t* lag_gcd;
    rng_t* lag_queue;
    void reset() { memset( ( void* ) this, 0x00, sizeof( rngs_t ) ); }
    rngs_t() { reset(); }
  };
  rngs_t rngs;


  player_t( sim_t* sim, int type, const std::string& name, int race_type = RACE_NONE );

  virtual ~player_t();

  virtual const char* name() SC_CONST { return name_str.c_str(); }
  virtual const char* id();

  virtual void init();
  virtual void init_glyphs() {}
  virtual void init_base() = 0;
  virtual void init_items();
  virtual void init_meta_gem( gear_stats_t& );
  virtual void init_core();
  virtual void init_race();
  virtual void init_spell();
  virtual void init_attack();
  virtual void init_defense();
  virtual void init_weapon( weapon_t* );
  virtual void init_unique_gear();
  virtual void init_enchant();
  virtual void init_resources( bool force = false );
  virtual void init_consumables();
  virtual void init_professions();
  virtual void init_actions();
  virtual void init_rating();
  virtual void init_scaling();
  virtual void init_buffs();
  virtual void init_gains();
  virtual void init_procs();
  virtual void init_uptimes();
  virtual void init_rng();
  virtual void init_stats();
  virtual void init_expressions();

  virtual void reset();
  virtual void combat_begin();
  virtual void combat_end();

  virtual double composite_attack_power() SC_CONST;
  virtual double composite_attack_crit() SC_CONST;
  virtual double composite_attack_expertise() SC_CONST { return attack_expertise; }
  virtual double composite_attack_hit() SC_CONST;
  virtual double composite_attack_penetration() SC_CONST { return attack_penetration; }

  virtual double composite_armor()          SC_CONST;
  virtual double composite_armor_snapshot() SC_CONST { return armor_snapshot; }
  virtual double composite_defense()        SC_CONST { return defense; }
  virtual double composite_miss()           SC_CONST;
  virtual double composite_dodge()          SC_CONST;
  virtual double composite_parry()          SC_CONST;
  virtual double composite_block()          SC_CONST;
  virtual double composite_block_value()    SC_CONST;

  virtual double composite_spell_power( int school ) SC_CONST;
  virtual double composite_spell_crit() SC_CONST;
  virtual double composite_spell_hit() SC_CONST;
  virtual double composite_spell_penetration() SC_CONST { return spell_penetration; }

  virtual double composite_attack_power_multiplier() SC_CONST;
  virtual double composite_spell_power_multiplier() SC_CONST { return spell_power_multiplier; }
  virtual double composite_attribute_multiplier( int attr ) SC_CONST;

  virtual double strength() SC_CONST;
  virtual double agility() SC_CONST;
  virtual double stamina() SC_CONST;
  virtual double intellect() SC_CONST;
  virtual double spirit() SC_CONST;

  virtual void      interrupt();
  virtual void      clear_debuffs();
  virtual void      schedule_ready( double delta_time=0, bool waiting=false );
  virtual double    available() SC_CONST { return 0.1; }
  virtual action_t* execute_action();

  virtual void   regen( double periodicity=2.0 );
  virtual double resource_gain( int resource, double amount, gain_t* g=0, action_t* a=0 );
  virtual double resource_loss( int resource, double amount, action_t* a=0 );
  virtual bool   resource_available( int resource, double cost ) SC_CONST;
  virtual int    primary_resource() SC_CONST { return RESOURCE_NONE; }
  virtual int    primary_role() SC_CONST     { return ROLE_HYBRID; }
  virtual int    primary_tree() SC_CONST     { return TALENT_TREE_MAX; }

  virtual void stat_gain( int stat, double amount );
  virtual void stat_loss( int stat, double amount );

  virtual void  summon_pet( const char* name, double duration=0 );
  virtual void dismiss_pet( const char* name );

  virtual bool ooc_buffs() { return true; }
  virtual void target_swing() {}

  virtual void register_callbacks();
  virtual void register_resource_gain_callback       ( int resource, action_callback_t* );
  virtual void register_resource_loss_callback       ( int resource, action_callback_t* );
  virtual void register_attack_result_callback       ( int result_mask, action_callback_t* );
  virtual void register_spell_result_callback        ( int result_mask, action_callback_t* );
  virtual void register_attack_direct_result_callback( int result_mask, action_callback_t* );
  virtual void register_spell_direct_result_callback ( int result_mask, action_callback_t* );
  virtual void register_tick_callback                ( action_callback_t* );
  virtual void register_tick_damage_callback         ( action_callback_t* );
  virtual void register_direct_damage_callback       ( action_callback_t* );

  virtual bool get_talent_translation( std::vector<int*>& tree, talent_translation_t translation[] );
  virtual bool get_talent_translation( std::vector<int*>& tree1, std::vector<int*>& tree2, std::vector<int*>& tree3, talent_translation_t translation[][3] );
  virtual bool get_talent_trees( std::vector<int*>& tree1, std::vector<int*>& tree2, std::vector<int*>& tree3 );
  virtual bool parse_talent_tree( std::vector<int*>& talent_tree, const std::string& talent_string );
  virtual bool parse_talents_armory ( const std::string& talent_string );
  virtual bool parse_talents_mmo    ( const std::string& talent_string );
  virtual bool parse_talents_wowhead( const std::string& talent_string );

  virtual std::vector<option_t>& get_options();
  virtual bool save( FILE*, int save_type=SAVE_ALL );

  virtual action_t* create_action( const std::string& name, const std::string& options );
  virtual pet_t*    create_pet   ( const std::string& name ) { return 0; }
  virtual pet_t*    find_pet     ( const std::string& name );

  virtual void trigger_replenishment();
  virtual act_expression_t* create_expression( std::string& name, std::string& prefix, std::string& suffix, exp_res_t expected_type );

  virtual void armory( xml_node_t* sheet_xml, xml_node_t* talents_xml ) {}
  virtual int  decode_set( item_t& item ) { return SET_NONE; }

  virtual void recalculate_haste();

  // Class-Specific Methods

  static player_t* create( sim_t* sim, const std::string& type, const std::string& name, int race_type = RACE_NONE );

  static player_t * create_death_knight( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_druid       ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_hunter      ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_mage        ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_paladin     ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_priest      ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_rogue       ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_shaman      ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_warlock     ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );
  static player_t * create_warrior     ( sim_t* sim, const std::string& name, int race_type = RACE_NONE );

  // Raid-wide aura/buff/debuff maintenance
  static void init        ( sim_t* sim );
  static void combat_begin( sim_t* sim );
  static void combat_end  ( sim_t* sim );

  // Raid-wide Death Knight buff maintenance
  static void death_knight_init        ( sim_t* sim );
  static void death_knight_combat_begin( sim_t* sim );
  static void death_knight_combat_end  ( sim_t* sim ) {}

  // Raid-wide Druid buff maintenance
  static void druid_init        ( sim_t* sim );
  static void druid_combat_begin( sim_t* sim );
  static void druid_combat_end  ( sim_t* sim ) {}

  // Raid-wide Hunter buff maintenance
  static void hunter_init        ( sim_t* sim );
  static void hunter_combat_begin( sim_t* sim );
  static void hunter_combat_end  ( sim_t* sim ) {}

  // Raid-wide Mage buff maintenance
  static void mage_init        ( sim_t* sim );
  static void mage_combat_begin( sim_t* sim );
  static void mage_combat_end  ( sim_t* sim ) {}

  // Raid-wide Paladin buff maintenance
  static void paladin_init        ( sim_t* sim );
  static void paladin_combat_begin( sim_t* sim );
  static void paladin_combat_end  ( sim_t* sim ) {}

  // Raid-wide Priest buff maintenance
  static void priest_init        ( sim_t* sim );
  static void priest_combat_begin( sim_t* sim );
  static void priest_combat_end  ( sim_t* sim ) {}

  // Raid-wide Rogue buff maintenance
  static void rogue_init        ( sim_t* sim );
  static void rogue_combat_begin( sim_t* sim );
  static void rogue_combat_end  ( sim_t* sim ) {}

  // Raid-wide Shaman buff maintenance
  static void shaman_init        ( sim_t* sim );
  static void shaman_combat_begin( sim_t* sim );
  static void shaman_combat_end  ( sim_t* sim ) {}

  // Raid-wide Warlock buff maintenance
  static void warlock_init        ( sim_t* sim );
  static void warlock_combat_begin( sim_t* sim );
  static void warlock_combat_end  ( sim_t* sim ) {}

  // Raid-wide Warrior buff maintenance
  static void warrior_init        ( sim_t* sim );
  static void warrior_combat_begin( sim_t* sim );
  static void warrior_combat_end  ( sim_t* sim ) {}

  bool is_pet() { return type == PLAYER_PET || type == PLAYER_GUARDIAN; }

  death_knight_t* cast_death_knight() { assert( type == DEATH_KNIGHT ); return ( death_knight_t* ) this; }
  druid_t       * cast_druid       () { assert( type == DRUID        ); return ( druid_t       * ) this; }
  hunter_t      * cast_hunter      () { assert( type == HUNTER       ); return ( hunter_t      * ) this; }
  mage_t        * cast_mage        () { assert( type == MAGE         ); return ( mage_t        * ) this; }
  paladin_t     * cast_paladin     () { assert( type == PALADIN      ); return ( paladin_t     * ) this; }
  priest_t      * cast_priest      () { assert( type == PRIEST       ); return ( priest_t      * ) this; }
  rogue_t       * cast_rogue       () { assert( type == ROGUE        ); return ( rogue_t       * ) this; }
  shaman_t      * cast_shaman      () { assert( type == SHAMAN       ); return ( shaman_t      * ) this; }
  warlock_t     * cast_warlock     () { assert( type == WARLOCK      ); return ( warlock_t     * ) this; }
  warrior_t     * cast_warrior     () { assert( type == WARRIOR      ); return ( warrior_t     * ) this; }
  pet_t         * cast_pet         () { assert( is_pet()             ); return ( pet_t         * ) this; }

  bool      in_gcd() SC_CONST { return gcd_ready > sim -> current_time; }
  bool      recent_cast();
  item_t*   find_item( const std::string& );
  action_t* find_action( const std::string& );
  void      share_cooldown( const std::string& name, double ready );
  void      share_duration( const std::string& name, double ready );
  double    mana_regen_per_second();
  bool      dual_wield() SC_CONST { return main_hand_weapon.type != WEAPON_NONE && off_hand_weapon.type != WEAPON_NONE; }
  void      aura_gain( const char* name, int aura_id=0 );
  void      aura_loss( const char* name, int aura_id=0 );
  gain_t*   get_gain  ( const std::string& name );
  proc_t*   get_proc  ( const std::string& name, sim_t* );
  stats_t*  get_stats ( const std::string& name );
  uptime_t* get_uptime( const std::string& name );
  rng_t*    get_rng   ( const std::string& name, int type=RNG_DEFAULT );
};

// Pet =======================================================================

struct pet_t : public player_t
{
  std::string full_name_str;
  player_t* owner;
  pet_t* next_pet;
  double stamina_per_owner;
  double intellect_per_owner;
  double summon_time;

  pet_t( sim_t* sim, player_t* owner, const std::string& name, bool guardian=false );

  virtual double composite_attack_expertise() SC_CONST { return owner -> composite_attack_hit() * 6.5 / 8.0; }
  virtual double composite_attack_hit()       SC_CONST { return owner -> composite_attack_hit(); }
  virtual double composite_spell_hit()        SC_CONST { return owner -> composite_spell_hit();  }

  virtual double stamina() SC_CONST;
  virtual double intellect() SC_CONST;

  virtual void init();
  virtual void init_base();
  virtual void reset();
  virtual void summon( double duration=0 );
  virtual void dismiss();
  virtual bool ooc_buffs() { return false; }

  virtual action_t* create_action( const std::string& name, const std::string& options );
  virtual const char* name() SC_CONST { return full_name_str.c_str(); }
  virtual const char* id();
};

// Target ====================================================================

struct target_t
{
  sim_t* sim;
  std::string name_str, race_str, id_str;
  int race;
  int level;
  int spell_resistance[ SCHOOL_MAX ];
  int initial_armor, armor;
  int block_value;
  int shield;
  double attack_speed, attack_damage;
  double initial_health, current_health;
  double total_dmg;

  struct debuffs_t
  {
    debuff_t* bleeding;
    debuff_t* blood_frenzy;
    debuff_t* casting;
    debuff_t* crypt_fever;
    debuff_t* curse_of_elements;
    debuff_t* earth_and_moon;
    debuff_t* faerie_fire;
    debuff_t* frostbite;
    debuff_t* heart_of_the_crusader;
    debuff_t* hunters_mark;
    debuff_t* improved_faerie_fire;
    debuff_t* improved_scorch;
    debuff_t* improved_shadow_bolt;
    debuff_t* insect_swarm;
    debuff_t* invulnerable;
    debuff_t* judgement_of_wisdom;
    debuff_t* judgements_of_the_just;
    debuff_t* mangle;
    debuff_t* misery;
    debuff_t* scorpid_sting;
    debuff_t* slow;
    debuff_t* sunder_armor;
    debuff_t* thunder_clap;
    debuff_t* totem_of_wrath;
    debuff_t* trauma;
    debuff_t* vulnerable;
    debuff_t* winters_chill;
    debuff_t* winters_grasp;
    debuff_t* master_poisoner;
    debuff_t* poisoned;
    debuff_t* savage_combat;
    debuff_t* expose_armor;
    debuff_t* hemorrhage;
    debuffs_t() { memset( (void*) this, 0x0, sizeof( debuffs_t ) ); }
    bool frozen() { return frostbite -> check() || winters_grasp -> check(); }
    bool snared() { return frozen() || judgements_of_the_just -> check() || slow -> check() || thunder_clap -> check(); }
  };
  debuffs_t debuffs;

  target_t( sim_t* s );
  ~target_t();

  void init();
  void reset();
  void combat_begin();
  void combat_end();
  void assess_damage( double amount, int school, int type );
  void recalculate_health();
  double time_to_die() SC_CONST;
  double health_percentage() SC_CONST;
  double base_armor() SC_CONST;
  void aura_gain( const char* name, int aura_id=0 );
  void aura_loss( const char* name, int aura_id=0 );
  int get_options( std::vector<option_t>& );
  const char* name() SC_CONST { return name_str.c_str(); }
  const char* id();
};

// Stats =====================================================================

struct stats_t
{
  std::string name_str;
  sim_t* sim;
  player_t* player;
  stats_t* next;
  int school;
  bool channeled;
  bool analyzed;
  bool initialized;

  double resource_consumed;
  double frequency, num_executes, num_ticks;
  double total_execute_time, total_tick_time;
  double total_dmg, portion_dmg;
  double dps, portion_dps, dpe, dpet, dpr;
  double total_intervals, num_intervals;
  double last_execute;

  struct stats_results_t
  {
    double count, min_dmg, max_dmg, avg_dmg, total_dmg;
  };
  stats_results_t execute_results[ RESULT_MAX ];
  stats_results_t    tick_results[ RESULT_MAX ];

  int num_buckets;
  std::vector<double> timeline_dmg;
  std::vector<double> timeline_dps;

  void consume_resource( double r ) { resource_consumed += r; }
  void add( double amount, int dmg_type, int result, double time );
  void init();
  void reset( action_t* );
  void analyze();
  void merge( stats_t* other );
  stats_t( const std::string& name, player_t* );
};

// Rank ======================================================================

struct rank_t
{
  int level, index;
  double dd_min, dd_max, tick, cost;
};

// Base Stats ================================================================

struct base_stats_t
{
  int level;
  double health, mana;
  double strength, agility, stamina, intellect, spirit;
  double spell_crit, melee_crit;
};

// Action ====================================================================

struct action_t
{
  sim_t* sim;
  int type;
  std::string name_str;
  player_t* player;
  int id, school, resource, tree, result;
  bool dual, special, binary, channeled, background, repeating, aoe, harmful, proc, pseudo_pet;
  bool may_miss, may_resist, may_dodge, may_parry, may_glance, may_block, may_crush, may_crit;
  bool tick_may_crit, tick_zero, clip_dot;
  double min_gcd, trigger_gcd, range;
  double weapon_power_mod, direct_power_mod, tick_power_mod;
  double base_execute_time, base_tick_time, base_cost;
  double base_dd_min, base_dd_max, base_td, base_td_init;
  double base_dd_multiplier, base_td_multiplier;
  double   base_multiplier,   base_hit,   base_crit,   base_penetration;
  double player_multiplier, player_hit, player_crit, player_penetration;
  double target_multiplier, target_hit, target_crit, target_penetration;
  double   base_spell_power,   base_attack_power;
  double player_spell_power, player_attack_power;
  double target_spell_power, target_attack_power;
  double   base_spell_power_multiplier,   base_attack_power_multiplier;
  double player_spell_power_multiplier, player_attack_power_multiplier;
  double   base_crit_multiplier,   base_crit_bonus_multiplier, base_crit_bonus;
  double player_crit_multiplier, player_crit_bonus_multiplier;
  double target_crit_multiplier, target_crit_bonus_multiplier;
  double base_dd_adder, player_dd_adder, target_dd_adder;
  double resource_consumed;
  double direct_dmg, tick_dmg;
  double resisted_dmg, blocked_dmg;
  int num_ticks, current_tick, added_ticks;
  int ticking;
  std::string cooldown_group, duration_group;
  double cooldown, cooldown_ready, duration_ready;
  weapon_t* weapon;
  double weapon_multiplier;
  bool normalize_weapon_damage;
  bool normalize_weapon_speed;
  rng_t* rng[ RESULT_MAX ];
  rng_t* rng_travel;
  stats_t* stats;
  event_t* execute_event;
  event_t* tick_event;
  double time_to_execute, time_to_tick, time_to_travel, travel_speed;
  int rank_index, bloodlust_active;
  double max_haste;
  double min_current_time, max_current_time;
  double min_time_to_die, max_time_to_die;
  double min_health_percentage, max_health_percentage;
  int P322, moving, vulnerable, invulnerable, wait_on_ready;
  int has_if_exp, is_ifall;
  act_expression_t* if_exp;
  std::string if_expression;
  std::string sync_str;
  action_t*   sync_action;
  action_t** observer;
  action_t* next;

  action_t( int type, const char* name, player_t* p=0, int r=RESOURCE_NONE, int s=SCHOOL_NONE, int t=TREE_NONE, bool special=false );
  virtual ~action_t() {}

  virtual void      parse_options( option_t*, const std::string& options_str );
  virtual option_t* merge_options( std::vector<option_t>&, option_t*, option_t* );
  virtual rank_t*   init_rank( rank_t* rank_list, int id=0 );

  virtual double cost() SC_CONST;
  virtual double haste() SC_CONST        { return 1.0;               }
  virtual double gcd() SC_CONST          { return trigger_gcd;       }
  virtual double execute_time() SC_CONST { return base_execute_time; }
  virtual double tick_time() SC_CONST    { return base_tick_time;    }
  virtual double travel_time();
  virtual void   player_buff();
  virtual void   target_debuff( int dmg_type );
  virtual void   calculate_result() { assert( 0 ); }
  virtual bool   result_is_hit() SC_CONST;
  virtual bool   result_is_miss() SC_CONST;
  virtual double calculate_direct_damage();
  virtual double calculate_tick_damage();
  virtual double calculate_weapon_damage();
  virtual double armor() SC_CONST;
  virtual double resistance() SC_CONST;
  virtual void   consume_resource();
  virtual void   execute();
  virtual void   tick();
  virtual void   last_tick();
  virtual void   travel( int result, double dmg );
  virtual void   assess_damage( double amount, int dmg_type );
  virtual void   schedule_execute();
  virtual void   schedule_tick();
  virtual void   schedule_travel();
  virtual void   refresh_duration();
  virtual void   extend_duration( int extra_ticks );
  virtual void   update_ready();
  virtual void   update_stats( int type );
  virtual void   update_time( int type );
  virtual bool   ready();
  virtual void   reset();
  virtual void   cancel();
  virtual void   check_talent( int talent_rank );
  virtual const char* name() SC_CONST { return name_str.c_str(); }

  virtual double total_multiplier() SC_CONST { return   base_multiplier * player_multiplier * target_multiplier; }
  virtual double total_hit() SC_CONST        { return   base_hit        + player_hit        + target_hit;        }
  virtual double total_crit() SC_CONST       { return   base_crit       + player_crit       + target_crit;       }
  virtual double total_crit_bonus() SC_CONST;

  virtual double total_spell_power() SC_CONST  { return ( base_spell_power  + player_spell_power  + target_spell_power  ) * base_spell_power_multiplier  * player_spell_power_multiplier;  }
  virtual double total_attack_power() SC_CONST { return ( base_attack_power + player_attack_power + target_attack_power ) * base_attack_power_multiplier * player_attack_power_multiplier; }
  virtual double total_power() SC_CONST;

  // Some actions require different multipliers for the "direct" and "tick" portions.

  virtual double total_dd_multiplier() SC_CONST { return total_multiplier() * base_dd_multiplier; }
  virtual double total_td_multiplier() SC_CONST { return total_multiplier() * base_td_multiplier; }
  virtual act_expression_t* create_expression( std::string& name, std::string& prefix, std::string& suffix, exp_res_t expected_type );

};

// Attack ====================================================================

struct attack_t : public action_t
{
  double base_expertise, player_expertise, target_expertise;

  attack_t( const char* n=0, player_t* p=0, int r=RESOURCE_NONE, int s=SCHOOL_PHYSICAL, int t=TREE_NONE, bool special=false );
  virtual ~attack_t() {}

  // Attack Overrides
  virtual double haste() SC_CONST;
  virtual double execute_time() SC_CONST;
  virtual void   player_buff();
  virtual void   target_debuff( int dmg_type );
  virtual int    build_table( double* chances, int* results );
  virtual void   calculate_result();
  virtual void   execute();

  // Attack Specific
  virtual double   miss_chance( int delta_level ) SC_CONST;
  virtual double  dodge_chance( int delta_level ) SC_CONST;
  virtual double  parry_chance( int delta_level ) SC_CONST;
  virtual double glance_chance( int delta_level ) SC_CONST;
  virtual double  block_chance( int delta_level ) SC_CONST;
  virtual double   crit_chance( int delta_level ) SC_CONST;
  virtual double total_expertise() SC_CONST { return base_expertise + player_expertise + target_expertise; }
};

// Spell =====================================================================

struct spell_t : public action_t
{
  spell_t( const char* n=0, player_t* p=0, int r=RESOURCE_NONE, int s=SCHOOL_PHYSICAL, int t=TREE_NONE );
  virtual ~spell_t() {}

  // Spell Overrides
  virtual double haste() SC_CONST;
  virtual double gcd() SC_CONST;
  virtual double execute_time() SC_CONST;
  virtual double tick_time() SC_CONST;
  virtual void   player_buff();
  virtual void   target_debuff( int dmg_type );
  virtual double level_based_miss_chance( int player, int target ) SC_CONST;
  virtual double crit_chance( int player, int target ) SC_CONST;
  virtual void   calculate_result();
  virtual void   execute();
};

// Sequence ==================================================================

struct sequence_t : public action_t
{
  std::vector<action_t*> sub_actions;
  int current_action;

  sequence_t( const char* name, player_t*, const std::string& sub_action_str );
  virtual ~sequence_t();
  virtual void schedule_execute();
  virtual void reset();
  virtual bool ready();
  virtual void restart() { current_action=0; }
};

// Action Callback ===========================================================

struct action_callback_t
{
  sim_t* sim;
  player_t* listener;
  bool active;
  action_callback_t( sim_t* s, player_t* l ) : sim( s ), listener( l ), active( true ) {}
  virtual ~action_callback_t() {}
  virtual void trigger( action_t* ) = 0;
  virtual void reset() {}
  virtual void activate() { active=true; }
  virtual void deactivate() { active=false; }
  static void trigger( std::vector<action_callback_t*>& v, action_t* a )
  {
    int size = v.size();
    for ( int i=0; i < size; i++ )
    {
      action_callback_t* cb = v[ i ];
      if ( cb -> active ) cb -> trigger( a );
    }
  }
  static void   reset( std::vector<action_callback_t*>& v )
  {
    int size = v.size();
    for ( int i=0; i < size; i++ )
    {
      v[ i ] -> reset();
    }
  }
};

// Player Ready Event ========================================================

struct player_ready_event_t : public event_t
{
  player_ready_event_t( sim_t* sim, player_t* p, double delta_time );
  virtual void execute();
};

// Action Execute Event ======================================================

struct action_execute_event_t : public event_t
{
  action_t* action;
  action_execute_event_t( sim_t* sim, action_t* a, double time_to_execute );
  virtual void execute();
};

// Action Tick Event =========================================================

struct action_tick_event_t : public event_t
{
  action_t* action;
  action_tick_event_t( sim_t* sim, action_t* a, double time_to_tick );
  virtual void execute();
};

// Action Travel Event =======================================================

struct action_travel_event_t : public event_t
{
  action_t* action;
  int result;
  double damage;
  action_travel_event_t( sim_t* sim, action_t* a, double time_to_travel );
  virtual void execute();
};

// Regen Event ===============================================================

struct regen_event_t : public event_t
{
  regen_event_t( sim_t* sim );
  virtual void execute();
};

// Unique Gear ===============================================================

struct unique_gear_t
{
  static void init( player_t* );

  static action_callback_t* register_stat_proc( int type, int mask, const std::string& name, player_t*,
                                                int stat, int max_stacks, double amount,
                                                double proc_chance, double duration, double cooldown,
                                                int rng_type=RNG_DEFAULT );

  static action_callback_t* register_discharge_proc( int type, int mask, const std::string& name, player_t*,
                                                     int max_stacks, int school, double min_dmg, double max_dmg,
                                                     double proc_chance, double cooldown,
                                                     int rng_type=RNG_DEFAULT );

  static action_callback_t* register_stat_proc     ( item_t&, item_t::special_effect_t& );
  static action_callback_t* register_discharge_proc( item_t&, item_t::special_effect_t& );

  static bool get_equip_encoding( std::string& encoding,
                                  const std::string& item_name,
                                  const std::string& item_id=std::string() );

  static bool get_hidden_encoding( std::string&       encoding,
                                   const std::string& item_name,
                                   const std::string& item_id=std::string() );

  static bool get_use_encoding  ( std::string& encoding,
                                  const std::string& item_name,
                                  const std::string& item_id=std::string() );
};

// Enchants ===================================================================

struct enchant_t
{
  static void init( player_t* );
  static bool get_encoding( std::string& name, std::string& encoding, const std::string& enchant_id );
  static bool download( item_t&, const std::string& enchant_id );
};

// Consumable ================================================================

struct consumable_t
{
  static void init_flask  ( player_t* );
  static void init_elixirs( player_t* );
  static void init_food   ( player_t* );

  static action_t* create_action( player_t*, const std::string& name, const std::string& options );
};

// Up-Time =====================================================================

struct uptime_t
{
  std::string name_str;
  uint64_t up, down;
  uptime_t* next;
  uptime_t( const std::string& n ) : name_str( n ), up( 0 ), down( 0 ) {}
  virtual ~uptime_t() {}
  void   update( bool is_up ) { if ( is_up ) up++; else down++; }
  double percentage() SC_CONST { return ( up==0 ) ? 0 : ( 100.0*up/( up+down ) ); }
  virtual void   merge( uptime_t* other ) { up += other -> up; down += other -> down; }
  const char* name() SC_CONST { return name_str.c_str(); }
};

// Gain ======================================================================

struct gain_t
{
  std::string name_str;
  double actual, overflow;
  int id;
  gain_t* next;
  gain_t( const std::string& n, int _id=0 ) : name_str( n ), actual( 0 ), overflow( 0 ), id( _id ) {}
  void add( double a, double o=0 ) { actual += a; overflow += o; }
  void merge( gain_t* other ) { actual += other -> actual; overflow += other -> overflow; }
  void analyze( sim_t* sim ) { actual /= sim -> iterations; overflow /= sim -> iterations; }
  const char* name() SC_CONST { return name_str.c_str(); }
};

// Proc ======================================================================

struct proc_t
{
  sim_t* sim;
  player_t* player;
  std::string name_str;
  double count;
  double frequency;
  double interval_sum;
  double interval_count;
  double last_proc;
  proc_t* next;
  proc_t( sim_t* s, const std::string& n ) :
    sim(s), name_str(n), count(0), frequency(0), interval_sum(0), interval_count(0), last_proc(0) {}
  void occur()
  {
    count++;
    if ( last_proc > 0 && last_proc < sim -> current_time )
    {
      interval_sum += sim -> current_time - last_proc;
      interval_count++;
    }
    last_proc = sim -> current_time;
  }
  void merge( proc_t* other )
  {
    count          += other -> count;
    interval_sum   += other -> interval_sum;
    interval_count += other -> interval_count;
  }
  void analyze( sim_t* sim )
  {
    count /= sim -> iterations;
    if ( interval_count > 0 ) frequency = interval_sum / interval_count;
  }
  const char* name() SC_CONST { return name_str.c_str(); }
};


// Report =====================================================================

struct report_t
{
  static void print_profiles( sim_t* );
  static void print_text( FILE*, sim_t*, bool detail=true );
  static void print_html( sim_t* );
  static void print_wiki( sim_t* );
  static void print_xml( sim_t* );
  static void print_suite( sim_t* );
};

// Chart ======================================================================

struct chart_t
{
  static int raid_dps ( std::vector<std::string>& images, sim_t* );
  static int raid_dpet( std::vector<std::string>& images, sim_t* );
  static int raid_gear( std::vector<std::string>& images, sim_t* );

  static const char* raid_downtime    ( std::string& s, sim_t* );
  static const char* action_dpet      ( std::string& s, player_t* );
  static const char* action_dmg       ( std::string& s, player_t* );
  static const char* gains            ( std::string& s, player_t* );
  static const char* timeline_resource( std::string& s, player_t* );
  static const char* timeline_dps     ( std::string& s, player_t* );
  static const char* distribution_dps ( std::string& s, player_t* );

  static const char* gear_weights_lootrank( std::string& s, player_t* );
  static const char* gear_weights_wowhead ( std::string& s, player_t* );
  static const char* gear_weights_pawn ( std::string& s, player_t* );
};

// Talent Translation =========================================================

struct talent_translation_t
{
  int  index;
  int* address;
};

// Log =======================================================================

struct log_t
{
  // Generic Output
  static void output( sim_t*, const char* format, ... );
  // Combat Log
  static void start_event( action_t* );
  static void damage_event( action_t*, double dmg, int dmg_type );
  static void miss_event( action_t* );
  static void resource_gain_event( player_t*, int resource, double amount, double actual_amount, gain_t* source );
  static void aura_gain_event( player_t*, const char* name, int id );
  static void aura_loss_event( player_t*, const char* name, int id );
  static void summon_event( pet_t* );
};

// Pseudo Random Number Generation ===========================================

struct rng_t
{
  std::string name_str;
  bool   gauss_pair_use;
  double gauss_pair_value;
  double expected_roll,  actual_roll,  num_roll;
  double expected_range, actual_range, num_range;
  double expected_gauss, actual_gauss, num_gauss;
  bool   average_range, average_gauss;
  rng_t* next;

  rng_t( const std::string& n, bool avg_range=false, bool avg_gauss=false );
  virtual ~rng_t() {}

  virtual int    type() SC_CONST { return RNG_STANDARD; }
  virtual double real();
  virtual int    roll( double chance );
  virtual double range( double min, double max );
  virtual double gauss( double mean, double stddev );
  virtual void   seed( uint32_t start );
  virtual void   report( FILE* );

  static rng_t* create( sim_t*, const std::string& name, int type=RNG_STANDARD );
};

// String utils =================================================================

std::string tolower( std::string src );
std::string trim( std::string src );
void replace_char( std::string& src, char old_c, char new_c  );
void replace_str( std::string& src, std::string old_str, std::string new_str  );
bool str_to_float( std::string src, double& dest );
std::string proper_option_name( const std::string& full_name );

// Thread Wrappers ===========================================================

struct thread_t
{
  static void init();
  static void launch( sim_t* );
  static void wait( sim_t* );
  static void mutex_init( void*& mutex );
  static void mutex_lock( void*& mutex );
  static void mutex_unlock( void*& mutex );
};

// Armory ====================================================================

struct armory_t
{
  static bool download_guild( sim_t* sim,
                              const std::string& region,
                              const std::string& server,
                              const std::string& name,
                              int player_type = PLAYER_NONE,
                              int max_rank=0,
                              int cache=0 );
  static player_t* download_player( sim_t* sim,
                                    const std::string& region,
                                    const std::string& server,
                                    const std::string& name,
                                    const std::string& talents,
                                    int cache=0 );
  static bool download_slot( item_t&, const std::string& item_id, int cache_only=0 );
  static bool download_item( item_t&, const std::string& item_id, int cache_only=0 );
  static void fuzzy_stats( std::string& encoding, const std::string& description );
  static int  parse_meta_gem( const std::string& description );
  static std::string& format( std::string& name, int format_type = FORMAT_DEFAULT );
};

// Wowhead  ==================================================================

struct wowhead_t
{
  static player_t* download_player( sim_t* sim,
                                    const std::string& region,
                                    const std::string& server,
                                    const std::string& name,
                                    int active=1 );
  static player_t* download_player( sim_t* sim, const std::string& id, int active=1 );
  static bool download_slot( item_t&, const std::string& item_id, const std::string& enchant_id, const std::string gem_ids[ 3 ], int cache_only=0 );
  static bool download_item( item_t&, const std::string& item_id, int cache_only=0 );
  static bool download_glyph( sim_t* sim, std::string& glyph_name, const std::string& glyph_id, int cache_only=0 );
  static int  parse_gem( item_t& item, const std::string& gem_id, int cache_only=0 );
};

// MMO Champion ==============================================================

struct mmo_champion_t
{
  static bool download_slot( item_t&, const std::string& item_id, const std::string& enchant_id, const std::string gem_ids[ 3 ], int cache_only=0 );
  static bool download_item( item_t&, const std::string& item_id, int cache_only=0 );
  static bool download_glyph( sim_t* sim, std::string& glyph_name, const std::string& glyph_id, int cache_only=0 );
  static int  parse_gem( item_t& item, const std::string& gem_id, int cache_only=0 );
};

// Rawr ======================================================================

struct rawr_t
{
  static player_t* load_player( sim_t*, const std::string& character_filename );
  static player_t* load_player( sim_t*, const std::string& character_filename, const std::string& character_xml );
};

// HTTP Download  ============================================================

struct http_t
{
  static bool cache_load();
  static bool cache_save();
  static void cache_clear();
  static void cache_set( const std::string& url, const std::string& result, int64_t timestamp=0 );
  static bool cache_get( std::string& result, const std::string& url, int64_t timestamp=0 );
  static bool download( std::string& result, const std::string& url );
  static bool get( std::string& result, const std::string& url, const std::string& confirmation=std::string(), int64_t timestamp=0, int throttle_seconds=0 );
  static bool clear_cache( sim_t*, const std::string& name, const std::string& value );
  static std::string& format( std::string& encoded_url, const std::string& url );
};

// XML =======================================================================

struct xml_t
{
  static const char* get_name( xml_node_t* node );
  static xml_node_t* get_child( xml_node_t* root, const std::string& name );
  static xml_node_t* get_node ( xml_node_t* root, const std::string& path );
  static int  get_children( std::vector<xml_node_t*>&, xml_node_t* root, const std::string& name = std::string() );
  static int  get_nodes   ( std::vector<xml_node_t*>&, xml_node_t* root, const std::string& path );
  static bool get_value( std::string& value, xml_node_t* root, const std::string& path = std::string() );
  static bool get_value( int&         value, xml_node_t* root, const std::string& path = std::string() );
  static bool get_value( double&      value, xml_node_t* root, const std::string& path = std::string() );
  static xml_node_t* download( const std::string& url, const std::string& confirmation=std::string(), int64_t timestamp=0, int throttle_seconds=0 );
  static xml_node_t* download_cache( const std::string& url, int64_t timestamp=0 );
  static xml_node_t* create( const std::string& input );
  static xml_node_t* create( FILE* input );
  static void print( xml_node_t* root, FILE* f=0, int spacing=0 );
};


// Java Script ===============================================================

struct js_t
{
  static js_node_t* get_child( js_node_t* root, const std::string& name );
  static js_node_t* get_node ( js_node_t* root, const std::string& path );
  static int  get_children( std::vector<js_node_t*>&, js_node_t* root, const std::string& name );
  static int  get_nodes   ( std::vector<js_node_t*>&, js_node_t* root, const std::string& path );
  static int  get_value( std::vector<std::string>& value, js_node_t* root, const std::string& path = std::string() );
  static bool get_value( std::string& value, js_node_t* root, const std::string& path = std::string() );
  static bool get_value( int&         value, js_node_t* root, const std::string& path = std::string() );
  static bool get_value( double&      value, js_node_t* root, const std::string& path = std::string() );
  static js_node_t* create( const std::string& input );
  static js_node_t* create( FILE* input );
  static void print( js_node_t* root, FILE* f=0, int spacing=0 );
};


#endif // __SIMCRAFT_H

