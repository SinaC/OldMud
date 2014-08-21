
#include <string.h>
#include "merc.h"
#include "config.h"
#include "dbdata.h"
#include "lookup.h"
#include "language.h"
#include "comm.h"
#include "olc.h"
#include "data_edit.h"
#include "script_edit.h"
#include "copyover.h"
#include "group.h"
#include "faction.h"

//#define DEFAULT_CONFIG_FILE "/home/fch/Rom/config.ini"
#define DEFAULT_CONFIG_FILE "config.ini"
char CONFIG_FILE[MAX_FILE_NAME_LENGTH];
char wantedConfig[MAX_STRING_LENGTH];


//--// Default data values
//--
//--#define DEFAULT_PORT          (4000)
//--
//--#if defined(macintosh)
//--#define DEFAULT_PLAYER_DIR	       ""			
//--#define DEFAULT_TEMP_FILE	       "romtmp"
//--#endif
//--
//--#if defined(MSDOS)
//--#define DEFAULT_PLAYER_DIR	       ""			
//--#define DEFAULT_TEMP_FILE	       "romtmp"
//--#endif
//--
//--#if defined(unix)
//--#define DEFAULT_PLAYER_DIR             "../player/"
//--#define DEFAULT_GOD_DIR                "../gods/"
//--#define DEFAULT_TEMP_FILE	       "../player/romtmp"
//--#endif
//--
//--#define DEFAULT_HELP_LIST              "help.lst"
//--#define DEFAULT_AREA_LIST              "area.lst"
//--#define DEFAULT_BUG_FILE               "../data/bugs/bugs.txt"
//--#define DEFAULT_TYPO_FILE              "../data/bugs/typos.txt"
//--#define DEFAULT_SHUTDOWN_FILE          "shutdown.txt"
//--#define DEFAULT_AREA_DIR               ""
//--#define DEFAULT_HELP_DIR               ""
//--
//--#define DEFAULT_SKILLS_FILE            "../data/abilities/skills.ini"
//--#define DEFAULT_SPELLS_FILE            "../data/abilities/spells.ini"
//--#define DEFAULT_POWERS_FILE            "../data/abilities/powers.ini"
//--#define DEFAULT_SONGS_FILE             "../data/abilities/songs.ini"
//--#define DEFAULT_AFFECTS_FILE           "../data/abilities/affects.ini"
//--#define DEFAULT_LIQ_FILE               "../data/liquid.ini"
//--#define DEFAULT_RACE_FILE              "../data/races/race.ini"
//--#define DEFAULT_PCRACE_FILE            "../data/races/pcrace.ini"
//--#define DEFAULT_PC_CLASS_FILE          "../data/classes/pc/class.ini"
//--#define DEFAULT_GROUP_FILE             "../data/abilities/group.ini"
//--#define DEFAULT_GODS_FILE              "../data/gods.ini"
//--#define DEFAULT_MATERIAL_FILE          "../data/material.ini"
//--#define DEFAULT_UNIQUE_ITEM_FILE       "../data/unique.ini"
//--#define DEFAULT_COMMANDS_FILE          "../data/commands.ini"
//--#define DEFAULT_SPHERE_FILE            "../data/abilities/spheres.ini"
//--#define DEFAULT_BAN_FILE               "../data/ban.ini"
//--#define DEFAULT_BREWFORMULA_FILE       "../data/brewformula.ini"
//--#define DEFAULT_BREWCOMPONENT_FILE     "../data/brewcomponent.lst"
//--#define DEFAULT_PREREQ_FILE            "../data/abilities/prereqs.ini"
//--#define DEFAULT_DISABLED_FILE          "../data/disabled.ini"
//--#define DEFAULT_HINT_FILE              "../data/hints.txt"
//--#define DEFAULT_CLAN_FILENAME          "../data/clans/clans.ini"
//--#define DEFAULT_HALL_FAME_FILE         "../data/halloffame.lst"
//--#define DEFAULT_FACTION_FILE           "../data/factions.ini"
//--#define DEFAULT_TIME_FILE              "../data/time.ini"
//--#define DEFAULT_HOMETOWN_FILE          "../data/hometown.ini"
//--
//--#define DEFAULT_COPYOVER_FILE	       "../data/output/copyover.data"
//--#define DEFAULT_LAST_COMMAND_FILE      "../data/output/last_command.txt" 
//--
//--#define DEFAULT_SCRIPT_DUMP_FILE       "../data/output/scripts.dump"
//--#define DEFAULT_SCRIPT_LIST            "../data/scripts/list"
//--#define DEFAULT_SCRIPT_DIR	       "../data/scripts/"
//--#define DEFAULT_SCRIPT_FILENAME_DEF    "other"
//--
//--#define DEFAULT_NOTE_DIR               "../data/boards/"
//--#define DEFAULT_DUMP_DIR               "../data/output/"
//--
//--#define DEFAULT_RACE_NAME_DEF          "unique"
//--#define DEFAULT_PC_RACE_NAME_DEF       "human"
//--#define DEFAULT_GOD_NAME_DEF           "mota"
//--#define DEFAULT_HOMETOWN_NAME_DEF      "midgaard"
//--#define DEFAULT_SIZE_NAME_DEF          "medium"
//--#define DEFAULT_LANGUAGE_NAME_DEF      "common"
//--
//--#define DEFAULT_RECALL_DEF             (3001)
//--#define DEFAULT_HALL_DEF               (3054)
//--#define DEFAULT_MORGUE_DEF             (3390)
//--#define DEFAULT_DONATION_DEF           (3360)
//--#define DEFAULT_SCHOOL_DEF             (3700)
//--
//--#define DEFAULT_FORBIDDEN_LIST         "../data/forbidden.lst"
//--
//--#define DEFAULT_MONITORING_FILE        "../data/output/monitoring.out"
//--
//--#define DEFAULT_SCHOOL_FILE            "../data/abilities/schools.ini"
//--#define DEFAULT_STAT_DIR               "../data/stat/"
//--
//--#define DEFAULT_SOCIAL_FILE            "../social/social.are"
//--#define DEFAULT_HTML_DIR               "../html/"
//--
//--#define DEFAULT_GROUP_NAME_DEF         "rom basics"
//--// 32 Megas
//--#define DEFAULT_MAX_HEAP_SIZE          (32*1024*1024)


// Default data values
#define DEFAULT_PORT          (4000)

#if defined(macintosh)
#define DEFAULT_PLAYER_DIR	       ""			
#define DEFAULT_TEMP_FILE	       "romtmp"
#endif

#if defined(MSDOS)
#define DEFAULT_PLAYER_DIR	       ""			
#define DEFAULT_TEMP_FILE	       "romtmp"
#endif

#if defined(unix)
#define DEFAULT_PLAYER_DIR             "player/"
#define DEFAULT_GOD_DIR                "gods/"
#define DEFAULT_TEMP_FILE	       "player/romtmp"
#endif

#define DEFAULT_HELP_LIST              "help.lst"
#define DEFAULT_AREA_LIST              "area.lst"
#define DEFAULT_BUG_FILE               "data/bugs/bugs.txt"
#define DEFAULT_TYPO_FILE              "data/bugs/typos.txt"
#define DEFAULT_SHUTDOWN_FILE          "shutdown.txt"
#define DEFAULT_AREA_DIR               "area/"
#define DEFAULT_HELP_DIR               "help/"

#define DEFAULT_SKILLS_FILE            "data/abilities/skills.ini"
#define DEFAULT_SPELLS_FILE            "data/abilities/spells.ini"
#define DEFAULT_POWERS_FILE            "data/abilities/powers.ini"
#define DEFAULT_SONGS_FILE             "data/abilities/songs.ini"
#define DEFAULT_AFFECTS_FILE           "data/abilities/affects.ini"
#define DEFAULT_LIQ_FILE               "data/liquid.ini"
#define DEFAULT_RACE_FILE              "data/races/race.ini"
#define DEFAULT_PCRACE_FILE            "data/races/pcrace.ini"
#define DEFAULT_SUPER_RACE_FILE        "data/races/superraces.ini"
#define DEFAULT_PC_CLASS_FILE          "data/classes/pc/class.ini"
#define DEFAULT_GROUP_FILE             "data/abilities/group.ini"
#define DEFAULT_GODS_FILE              "data/gods.ini"
#define DEFAULT_MATERIAL_FILE          "data/material.ini"
#define DEFAULT_UNIQUE_ITEM_FILE       "data/unique.ini"
#define DEFAULT_COMMANDS_FILE          "data/commands.ini"
#define DEFAULT_SPHERE_FILE            "data/abilities/spheres.ini"
#define DEFAULT_BAN_FILE               "data/ban.ini"
#define DEFAULT_BREWFORMULA_FILE       "data/brewformula.ini"
#define DEFAULT_BREWCOMPONENT_FILE     "data/brewcomponent.lst"
#define DEFAULT_PREREQ_FILE            "data/abilities/prereqs.ini"
#define DEFAULT_DISABLED_FILE          "data/disabled.ini"
#define DEFAULT_HINT_FILE              "data/hints.txt"
#define DEFAULT_CLAN_FILENAME          "data/clans/clans.ini"
#define DEFAULT_HALL_FAME_FILE         "data/halloffame.lst"
#define DEFAULT_FACTION_FILE           "data/factions.ini"
#define DEFAULT_TIME_FILE              "data/time.ini"
#define DEFAULT_HOMETOWN_FILE          "data/hometown.ini"

#define DEFAULT_COPYOVER_FILE	       "data/output/copyover.data"
#define DEFAULT_LAST_COMMAND_FILE      "data/output/last_command.txt" 

#define DEFAULT_SCRIPT_DUMP_FILE       "data/output/scripts.dump"
#define DEFAULT_SCRIPT_LIST            "data/scripts/list"
#define DEFAULT_SCRIPT_DIR	       "data/scripts/"
#define DEFAULT_SCRIPT_FILENAME_DEF    "other"

#define DEFAULT_NOTE_DIR               "data/boards/"
#define DEFAULT_DUMP_DIR               "data/output/"

#define DEFAULT_RACE_NAME_DEF          "unique"
#define DEFAULT_PC_RACE_NAME_DEF       "human"
#define DEFAULT_GOD_NAME_DEF           "mota"
#define DEFAULT_HOMETOWN_NAME_DEF      "midgaard"
#define DEFAULT_SIZE_NAME_DEF          "medium"
#define DEFAULT_LANGUAGE_NAME_DEF      "common"

#define DEFAULT_RECALL_DEF             (3001)
#define DEFAULT_HALL_DEF               (3054)
#define DEFAULT_MORGUE_DEF             (3390)
#define DEFAULT_DONATION_DEF           (3360)
#define DEFAULT_SCHOOL_DEF             (3700)

#define DEFAULT_FORBIDDEN_LIST         "data/forbidden.lst"

#define DEFAULT_MONITORING_FILE        "data/output/monitoring.out"

#define DEFAULT_SCHOOL_FILE            "data/abilities/schools.ini"
#define DEFAULT_STAT_DIR               "data/stat/"

#define DEFAULT_SOCIAL_FILE            "social/social.are"
#define DEFAULT_HTML_DIR               "html/"

#define DEFAULT_GROUP_NAME_DEF         "rom basics"
// 32 Megas
#define DEFAULT_MAX_HEAP_SIZE          (32*1024*1024)


char PLAYER_DIR[MAX_FILE_NAME_LENGTH];
char TEMP_FILE[MAX_FILE_NAME_LENGTH];
char GOD_DIR[MAX_FILE_NAME_LENGTH];

char HELP_LIST[MAX_FILE_NAME_LENGTH];
char AREA_LIST[MAX_FILE_NAME_LENGTH];
char BUG_FILE[MAX_FILE_NAME_LENGTH];
char TYPO_FILE[MAX_FILE_NAME_LENGTH];
char SHUTDOWN_FILE[MAX_FILE_NAME_LENGTH];
char AREA_DIR[MAX_FILE_NAME_LENGTH];
char HELP_DIR[MAX_FILE_NAME_LENGTH];

char SKILLS_FILE[MAX_FILE_NAME_LENGTH];
char SPELLS_FILE[MAX_FILE_NAME_LENGTH];
char POWERS_FILE[MAX_FILE_NAME_LENGTH];
char SONGS_FILE[MAX_FILE_NAME_LENGTH];
char AFFECTS_FILE[MAX_FILE_NAME_LENGTH];
char LIQ_FILE[MAX_FILE_NAME_LENGTH];
char RACE_FILE[MAX_FILE_NAME_LENGTH];
char PCRACE_FILE[MAX_FILE_NAME_LENGTH];
char PC_CLASS_FILE[MAX_FILE_NAME_LENGTH];
char GROUP_FILE[MAX_FILE_NAME_LENGTH];
char GODS_FILE[MAX_FILE_NAME_LENGTH];
char MATERIAL_FILE[MAX_FILE_NAME_LENGTH];
char UNIQUE_ITEM_FILE[MAX_FILE_NAME_LENGTH];
char COMMANDS_FILE[MAX_FILE_NAME_LENGTH];
char SPHERE_FILE[MAX_FILE_NAME_LENGTH];
char BAN_FILE[MAX_FILE_NAME_LENGTH];
char BREWFORMULA_FILE[MAX_FILE_NAME_LENGTH];
char BREWCOMPONENT_FILE[MAX_FILE_NAME_LENGTH];
char PREREQ_FILE[MAX_FILE_NAME_LENGTH];
char DISABLED_FILE[MAX_FILE_NAME_LENGTH];
char HINT_FILE[MAX_FILE_NAME_LENGTH];
char CLAN_FILENAME[MAX_FILE_NAME_LENGTH];
char HALL_FAME_FILE[MAX_FILE_NAME_LENGTH];
char FACTION_FILE[MAX_FILE_NAME_LENGTH];
char TIME_FILE[MAX_FILE_NAME_LENGTH];
char HOMETOWN_FILE[MAX_FILE_NAME_LENGTH];

char COPYOVER_FILE[MAX_FILE_NAME_LENGTH];
char LAST_COMMAND_FILE[MAX_FILE_NAME_LENGTH];

char SCRIPT_DUMP_FILE[MAX_FILE_NAME_LENGTH];
char SCRIPT_LIST[MAX_FILE_NAME_LENGTH];
char SCRIPT_DIR[MAX_FILE_NAME_LENGTH];
char DEFAULT_SCRIPT_FILENAME[MAX_FILE_NAME_LENGTH];

char NOTE_DIR[MAX_FILE_NAME_LENGTH];
char DUMP_DIR[MAX_FILE_NAME_LENGTH];

char SOCIAL_FILE[MAX_FILE_NAME_LENGTH];
char HTML_DIR[MAX_FILE_NAME_LENGTH];

char DEFAULT_PC_RACE_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_PC_RACE;
char DEFAULT_RACE_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_RACE;
char DEFAULT_GOD_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_GOD;
char DEFAULT_HOMETOWN_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_HOMETOWN;
int DEFAULT_RECALL;
int DEFAULT_HALL;
int DEFAULT_MORGUE;
int DEFAULT_DONATION;
int DEFAULT_SCHOOL;
char DEFAULT_SIZE_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_SIZE;
char DEFAULT_LANGUAGE_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_LANGUAGE;
char DEFAULT_GROUP_NAME[MAX_FILE_NAME_LENGTH];
int DEFAULT_GROUP;

char FORBIDDEN_LIST[MAX_FILE_NAME_LENGTH];
char MONITORING_FILE[MAX_FILE_NAME_LENGTH];

char STAT_DIR[MAX_FILE_NAME_LENGTH];

char SCHOOL_FILE[MAX_FILE_NAME_LENGTH];
char SUPER_RACE_FILE[MAX_FILE_NAME_LENGTH];

int SCRIPT_VERBOSE;
int DATA_VERBOSE;
int AGGR_VERBOSE;

int MAX_HEAP_SIZE;

ValueList *NEEDED_SCRIPT = NULL;
ValueList *NEEDED_OBJ = NULL;
ValueList *NEEDED_MOB = NULL;


void assign_default_values() {
  wantedConfig[0] = '\0';

  port = DEFAULT_PORT;

  strcpy( CONFIG_FILE, DEFAULT_CONFIG_FILE );

  strcpy( PLAYER_DIR, DEFAULT_PLAYER_DIR );
  strcpy( TEMP_FILE, DEFAULT_TEMP_FILE );
  strcpy( GOD_DIR, DEFAULT_GOD_DIR );
  strcpy( HELP_LIST, DEFAULT_HELP_LIST );
  strcpy( AREA_LIST, DEFAULT_AREA_LIST );
  strcpy( BUG_FILE, DEFAULT_BUG_FILE );
  strcpy( TYPO_FILE, DEFAULT_TYPO_FILE );
  strcpy( SHUTDOWN_FILE, DEFAULT_SHUTDOWN_FILE );
  strcpy( AREA_DIR, DEFAULT_AREA_DIR );
  strcpy( SKILLS_FILE, DEFAULT_SKILLS_FILE );
  strcpy( SPELLS_FILE, DEFAULT_SPELLS_FILE );
  strcpy( POWERS_FILE, DEFAULT_POWERS_FILE );
  strcpy( SONGS_FILE, DEFAULT_SONGS_FILE );
  strcpy( AFFECTS_FILE, DEFAULT_AFFECTS_FILE );
  strcpy( LIQ_FILE, DEFAULT_LIQ_FILE );
  strcpy( RACE_FILE, DEFAULT_RACE_FILE );
  strcpy( PCRACE_FILE, DEFAULT_PCRACE_FILE );
  strcpy( PC_CLASS_FILE, DEFAULT_PC_CLASS_FILE );
  strcpy( GROUP_FILE, DEFAULT_GROUP_FILE );
  strcpy( GODS_FILE, DEFAULT_GODS_FILE );
  strcpy( MATERIAL_FILE, DEFAULT_MATERIAL_FILE );
  strcpy( UNIQUE_ITEM_FILE, DEFAULT_UNIQUE_ITEM_FILE );
  strcpy( COMMANDS_FILE, DEFAULT_COMMANDS_FILE );
  strcpy( SPHERE_FILE, DEFAULT_SPHERE_FILE );
  strcpy( BAN_FILE, DEFAULT_BAN_FILE );
  strcpy( BREWFORMULA_FILE, DEFAULT_BREWFORMULA_FILE );
  strcpy( BREWCOMPONENT_FILE, DEFAULT_BREWCOMPONENT_FILE );
  strcpy( PREREQ_FILE, DEFAULT_PREREQ_FILE );
  strcpy( DISABLED_FILE, DEFAULT_DISABLED_FILE );
  strcpy( HINT_FILE, DEFAULT_HINT_FILE );
  strcpy( CLAN_FILENAME, DEFAULT_CLAN_FILENAME );
  strcpy( HALL_FAME_FILE, DEFAULT_HALL_FAME_FILE );
  strcpy( FACTION_FILE, DEFAULT_FACTION_FILE );
  strcpy( COPYOVER_FILE, DEFAULT_COPYOVER_FILE );
  strcpy( LAST_COMMAND_FILE, DEFAULT_LAST_COMMAND_FILE );
  strcpy( SCRIPT_DUMP_FILE, DEFAULT_SCRIPT_DUMP_FILE );
  strcpy( SCRIPT_LIST, DEFAULT_SCRIPT_LIST );
  strcpy( SCRIPT_DIR, DEFAULT_SCRIPT_DIR );
  strcpy( NOTE_DIR, DEFAULT_NOTE_DIR );
  strcpy( HELP_DIR, DEFAULT_HELP_DIR );
  strcpy( DUMP_DIR, DEFAULT_DUMP_DIR );
  strcpy( TIME_FILE, DEFAULT_TIME_FILE );
  strcpy( DEFAULT_RACE_NAME, DEFAULT_RACE_NAME_DEF );
  strcpy( DEFAULT_PC_RACE_NAME, DEFAULT_PC_RACE_NAME_DEF );
  strcpy( DEFAULT_GOD_NAME, DEFAULT_GOD_NAME_DEF );
  strcpy( DEFAULT_HOMETOWN_NAME, DEFAULT_HOMETOWN_NAME_DEF );
  strcpy( DEFAULT_SIZE_NAME, DEFAULT_SIZE_NAME_DEF );
  strcpy( DEFAULT_LANGUAGE_NAME, DEFAULT_LANGUAGE_NAME_DEF );
  strcpy( HOMETOWN_FILE, DEFAULT_HOMETOWN_FILE );
  strcpy( DEFAULT_SCRIPT_FILENAME, DEFAULT_SCRIPT_FILENAME_DEF );
  strcpy( FORBIDDEN_LIST, DEFAULT_FORBIDDEN_LIST );
  strcpy( DEFAULT_GROUP_NAME, DEFAULT_GROUP_NAME_DEF );
  strcpy( MONITORING_FILE, DEFAULT_MONITORING_FILE );
  strcpy( SCHOOL_FILE, DEFAULT_SCHOOL_FILE );
  strcpy( STAT_DIR, DEFAULT_STAT_DIR );
  strcpy( SOCIAL_FILE, DEFAULT_SOCIAL_FILE );
  strcpy( HTML_DIR, DEFAULT_HTML_DIR );
  strcpy( SUPER_RACE_FILE, DEFAULT_SUPER_RACE_FILE );

  SCRIPT_VERBOSE = 0;
  DATA_VERBOSE = 0;
  AGGR_VERBOSE = 0;
  wizlock = FALSE;
  newlock = FALSE;
  GC_VERBOSE = 0;
  TESTING_CHARMIES = FALSE;
  CAPTURE_SIGSEGV = FALSE;
  DUMP_MOB_MSG = FALSE;
  SAVE_STATE_ON_COPYOVER = TRUE;
  fState = FALSE;

  DATA_MINIMUM_SECURITY = 9;
  SCRIPT_MINIMUM_SECURITY = 9;

  MAX_HEAP_SIZE = DEFAULT_MAX_HEAP_SIZE;

  DEFAULT_RECALL = DEFAULT_RECALL_DEF;
  DEFAULT_HALL = DEFAULT_HALL_DEF;
  DEFAULT_MORGUE = DEFAULT_MORGUE_DEF;
  DEFAULT_DONATION = DEFAULT_DONATION_DEF;
  DEFAULT_SCHOOL = DEFAULT_SCHOOL_DEF;
}

void assign_default_struct() {
  if ( ( DEFAULT_RACE = race_lookup( DEFAULT_RACE_NAME ) ) < 0 ) {
    bug("Unknown default race [%s].", DEFAULT_RACE_NAME );
    exit(-1);
  }
  if ( ( DEFAULT_PC_RACE = race_lookup( DEFAULT_PC_RACE_NAME ) ) < 0 ) {
    bug("Unknown default pc race [%s].", DEFAULT_PC_RACE_NAME );
    exit(-1);
  }
  if ( ( DEFAULT_GOD = god_lookup( DEFAULT_GOD_NAME ) ) < 0 ) {
    bug("Unknown default god [%s].", DEFAULT_GOD_NAME );
    exit(-1);
  }
  if ( ( DEFAULT_HOMETOWN = hometown_lookup( DEFAULT_HOMETOWN_NAME ) ) < 0 ) {
    bug("Default hometown [%s] not found, using default recall [%d] and default hall [%d]",
	DEFAULT_HOMETOWN_NAME, DEFAULT_RECALL, DEFAULT_HALL );
    //bug("Unknown default hometown %s.", DEFAULT_HOMETOWN_NAME );
    //exit(-1);
  }
  if ( ( DEFAULT_SIZE = size_lookup( DEFAULT_SIZE_NAME ) ) < 0 ) {
    bug("Unknown default size [%s].", DEFAULT_SIZE_NAME );
    exit(-1);
  }
  if ( ( DEFAULT_LANGUAGE = language_lookup( DEFAULT_LANGUAGE_NAME ) ) < 0 ) {
    bug("Unknown default language [%s].", DEFAULT_LANGUAGE_NAME );
    exit(-1);
  }
  if ( ( DEFAULT_GROUP = group_lookup( DEFAULT_GROUP_NAME ) ) < 0 ) {
    bug("Unknown default group [%s].", DEFAULT_GROUP_NAME );
    exit(-1);
  }
  if ( get_room_index(DEFAULT_RECALL) == NULL ) {
    bug("Default recall (%d) doesn't exist", DEFAULT_RECALL );
    exit(-1);
  }
  if ( get_room_index(DEFAULT_HALL) == NULL ) {
    bug("Default hall (%d) doesn't exist", DEFAULT_HALL );
    exit(-1);
  }
  if ( get_room_index(DEFAULT_DONATION) == NULL ) {
    bug("Default donation (%d) doesn't exist", DEFAULT_DONATION );
    exit(-1);
  }
  if ( get_room_index(DEFAULT_MORGUE) == NULL ) {
    bug("Default morgue (%d) doesn't exist", DEFAULT_MORGUE );
    exit(-1);
  }
  if ( get_room_index(DEFAULT_SCHOOL) == NULL ) {
    bug("Default school (%d) doesn't exist", DEFAULT_SCHOOL );
    exit(-1);
  }
}

bool assign_config_variable( const int tagId, const Value &v ) {
  switch( tagId ) {
  case TAG_Port: port = v.asInt(); break;
  case TAG_Bug: strcpy( BUG_FILE, v.asStr() ); break;
  case TAG_Typo: strcpy( TYPO_FILE, v.asStr() ); break;
  case TAG_Hints: strcpy( HINT_FILE, v.asStr() ); break;
  case TAG_Ban: strcpy( BAN_FILE, v.asStr() ); break;
  case TAG_Brew: strcpy( BREWFORMULA_FILE, v.asStr() ); break;
  case TAG_Component: strcpy( BREWCOMPONENT_FILE, v.asStr() ); break;
  case TAG_Clans: strcpy( CLAN_FILENAME, v.asStr() ); break;
  case TAG_HallFame: strcpy( HALL_FAME_FILE, v.asStr() ); break;
  case TAG_Liquid: strcpy( LIQ_FILE, v.asStr() ); break;
  case TAG_Races: strcpy( RACE_FILE, v.asStr() ); break;
  case TAG_PCRaces: strcpy( PCRACE_FILE, v.asStr() ); break;
  case TAG_PCClasses: strcpy( PC_CLASS_FILE, v.asStr() ); break;
  case TAG_Gods: strcpy( GODS_FILE, v.asStr() ); break;
  case TAG_Material: strcpy( MATERIAL_FILE, v.asStr() ); break;
  case TAG_Commands: strcpy( COMMANDS_FILE, v.asStr() ); break;
  case TAG_Disabled: strcpy( DISABLED_FILE, v.asStr() ); break;
  case TAG_Factions: strcpy( FACTION_FILE, v.asStr() ); break;
  case TAG_Groups: strcpy( GROUP_FILE, v.asStr() ); break;
  case TAG_Spheres: strcpy( SPHERE_FILE, v.asStr() ); break;
  case TAG_Prerequisites: strcpy( PREREQ_FILE, v.asStr() ); break;
  case TAG_Skills: strcpy( SKILLS_FILE, v.asStr() ); break;
  case TAG_Spells: strcpy( SPELLS_FILE, v.asStr() ); break;
  case TAG_Powers: strcpy( POWERS_FILE, v.asStr() ); break;
  case TAG_Songs: strcpy( SONGS_FILE, v.asStr() ); break;
  case TAG_Unique: strcpy( UNIQUE_ITEM_FILE, v.asStr() ); break;
  case TAG_Script_List: strcpy( SCRIPT_LIST, v.asStr() ); break;
  case TAG_Area_List: strcpy( AREA_LIST, v.asStr() ); break;
  case TAG_Help_List: strcpy( HELP_LIST, v.asStr() ); break;
  case TAG_Shutdown: strcpy( SHUTDOWN_FILE, v.asStr() ); break;
  case TAG_Copyover: strcpy( COPYOVER_FILE, v.asStr() ); break;
  case TAG_Last_Command: strcpy( LAST_COMMAND_FILE, v.asStr() ); break;
  case TAG_Script_Dump: strcpy( SCRIPT_DUMP_FILE, v.asStr() ); break;
  case TAG_Temp: strcpy( TEMP_FILE, v.asStr() ); break;
  case TAG_Player_Dir: strcpy( PLAYER_DIR, v.asStr() ); break;
  case TAG_God_Dir: strcpy( GOD_DIR, v.asStr() ); break;
  case TAG_Script_Dir: strcpy( SCRIPT_DIR, v.asStr() ); break;
  case TAG_Note_Dir: strcpy( NOTE_DIR, v.asStr() ); break;
  case TAG_Area_Dir: strcpy( AREA_DIR, v.asStr() ); break;
  case TAG_Help_Dir: strcpy( HELP_DIR, v.asStr() ); break;
  case TAG_Dump_Dir: strcpy( DUMP_DIR, v.asStr() ); break;
  case TAG_Data_Verbose: DATA_VERBOSE = v.asInt(); break;
  case TAG_Script_Verbose: SCRIPT_VERBOSE = v.asInt(); break;
  case TAG_Aggr_Verbose: AGGR_VERBOSE = v.asInt(); break;
  case TAG_Wizlock: wizlock = v.asInt(); break;
  case TAG_Newlock: newlock = v.asInt(); break;
  case TAG_Time: strcpy( TIME_FILE, v.asStr() ); break;
  case TAG_GC_Verbose: GC_VERBOSE = v.asInt(); break;
  case TAG_Test_Charmies: TESTING_CHARMIES = v.asInt(); break;
  case TAG_Capture_SIGSEGV: CAPTURE_SIGSEGV = v.asInt(); break;
  case TAG_GC_Use_Entire_Heap: GC_use_entire_heap = v.asInt(); break;
  case TAG_GC_Dont_Expand: GC_dont_expand = v.asInt(); break;
  case TAG_GC_Dont_GC: GC_dont_gc = v.asInt(); break;
  case TAG_Default_Race: strcpy( DEFAULT_RACE_NAME, v.asStr() ); break;
  case TAG_Default_PC_Race: strcpy( DEFAULT_PC_RACE_NAME, v.asStr() ); break;
  case TAG_Default_God: strcpy( DEFAULT_GOD_NAME, v.asStr() ); break;
  case TAG_Default_Hometown: strcpy( DEFAULT_HOMETOWN_NAME, v.asStr() ); break;
  case TAG_Default_Size: strcpy( DEFAULT_SIZE_NAME, v.asStr() ); break;
  case TAG_Default_Language: strcpy( DEFAULT_LANGUAGE_NAME, v.asStr() ); break;
  case TAG_Hometown: strcpy( HOMETOWN_FILE, v.asStr() ); break;
  case TAG_Data_Min_Security: DATA_MINIMUM_SECURITY = v.asInt(); break;
  case TAG_Script_Min_Security: SCRIPT_MINIMUM_SECURITY = v.asInt(); break;
  case TAG_Default_Recall: DEFAULT_RECALL = v.asInt(); break;
  case TAG_Default_Hall: DEFAULT_HALL = v.asInt(); break;
  case TAG_Default_Morgue: DEFAULT_MORGUE = v.asInt(); break;
  case TAG_Default_Donation: DEFAULT_DONATION = v.asInt(); break;
  case TAG_Default_School: DEFAULT_SCHOOL = v.asInt(); break;
  case TAG_Default_Script_Filename: strcpy( DEFAULT_SCRIPT_FILENAME, v.asStr() ); break;
  case TAG_Dump_Mob_Msg: DUMP_MOB_MSG = v.asInt(); break;
  case TAG_Save_State_On_Copyover: SAVE_STATE_ON_COPYOVER = v.asInt(); break;
  case TAG_Start_World_State: fState = v.asInt(); break;
  case TAG_Forbidden_List: strcpy( FORBIDDEN_LIST, v.asStr() ); break;
  case TAG_Default_Group: strcpy( DEFAULT_GROUP_NAME, v.asStr() ); break;
  case TAG_Max_Heap_Size: MAX_HEAP_SIZE = v.asInt(); break;
  case TAG_Monitoring_File: strcpy( MONITORING_FILE, v.asStr() ); break;
  case TAG_Affects: strcpy( AFFECTS_FILE, v.asStr() ); break;  
  case TAG_School: strcpy( SCHOOL_FILE, v.asStr() ); break;
  case TAG_Stat_Dir: strcpy( STAT_DIR, v.asStr() ); break;
  case TAG_Start_With_Saved_Time: START_WITH_SAVED_TIME = v.asInt(); break;
  case TAG_Needed_Script: NEEDED_SCRIPT = v.asList(); break;
  case TAG_Needed_Obj: NEEDED_OBJ = v.asList(); break;
  case TAG_Needed_Mob: NEEDED_MOB = v.asList(); break;
  case TAG_Social: strcpy( SOCIAL_FILE, v.asStr() ); break;
  case TAG_Use_Faction_On_Aggro: USE_FACTION_ON_AGGRO = v.asInt(); break;
  case TAG_Html_Dir: strcpy( HTML_DIR, v.asStr() ); break;
  case TAG_Super_Races: strcpy( SUPER_RACE_FILE, v.asStr() ); break;
  default: return FALSE; break; // unknown tag
  }
  return TRUE;
}

void show_config_variable() {
  if ( DATA_VERBOSE > 0 ) {
    printf( "port:           %d\n\r", port );
    printf( "bug:            %s\n\r", BUG_FILE );
    printf( "type:           %s\n\r", TYPO_FILE );
    printf( "hints:          %s\n\r", HINT_FILE );
    printf( "ban:            %s\n\r", BAN_FILE );
    printf( "brew:           %s\n\r", BREWFORMULA_FILE );
    printf( "component       %s\n\r", BREWCOMPONENT_FILE );
    printf( "clan:           %s\n\r", CLAN_FILENAME ); 
    printf( "hall fame:      %s\n\r", HALL_FAME_FILE ); 
    printf( "liquid:         %s\n\r", LIQ_FILE ); 
    printf( "race:           %s\n\r", RACE_FILE ); 
    printf( "pcrace:         %s\n\r", PCRACE_FILE ); 
    printf( "pcclass:        %s\n\r", PC_CLASS_FILE ); 
    printf( "gods:           %s\n\r", GODS_FILE ); 
    printf( "material:       %s\n\r", MATERIAL_FILE ); 
    printf( "commands:       %s\n\r", COMMANDS_FILE ); 
    printf( "disabled:       %s\n\r", DISABLED_FILE ); 
    printf( "faction:        %s\n\r", FACTION_FILE ); 
    printf( "group:          %s\n\r", GROUP_FILE ); 
    printf( "sphere:         %s\n\r", SPHERE_FILE ); 
    printf( "prereq:         %s\n\r", PREREQ_FILE ); 
    printf( "skills:         %s\n\r", SKILLS_FILE ); 
    printf( "spells:         %s\n\r", SPELLS_FILE ); 
    printf( "powers:         %s\n\r", POWERS_FILE ); 
    printf( "songs:          %s\n\r", SONGS_FILE ); 
    printf( "unique:         %s\n\r", UNIQUE_ITEM_FILE ); 
    printf( "social:         %s\n\r", SOCIAL_FILE );
    printf( "html dir:       %s\n\r", HTML_DIR );
    printf( "script list:    %s\n\r", SCRIPT_LIST );
    printf( "area list:      %s\n\r", AREA_LIST );
    printf( "help list:      %s\n\r", HELP_LIST );
    printf( "shutdown:       %s\n\r", SHUTDOWN_FILE ); 
    printf( "copyover:       %s\n\r", COPYOVER_FILE ); 
    printf( "last command:   %s\n\r", LAST_COMMAND_FILE ); 
    printf( "script dump:    %s\n\r", SCRIPT_DUMP_FILE ); 
    printf( "temp:           %s\n\r", TEMP_FILE ); 
    printf( "hometown:       %s\n\r", HOMETOWN_FILE );
    printf( "player dir:     %s\n\r", PLAYER_DIR );
    printf( "god dir:        %s\n\r", GOD_DIR );
    printf( "script dir:     %s\n\r", SCRIPT_DIR );
    printf( "note dir:       %s\n\r", NOTE_DIR );
    printf( "area dir:       %s\n\r", AREA_DIR );
    printf( "help dir:       %s\n\r", HELP_DIR );
    printf( "dump dir:       %s\n\r", DUMP_DIR );
    printf( "data verbose:   %d\n\r", DATA_VERBOSE );
    printf( "script verbose: %d\n\r", SCRIPT_VERBOSE );
    printf( "aggr verbose:   %s\n\r", AGGR_VERBOSE?"true":"false" );
    printf( "Wizlock:        %s\n\r", wizlock?"true":"false");
    printf( "Newlock:        %s\n\r", newlock?"true":"false");
    printf( "Time:           %s\n\r", TIME_FILE );
    printf( "GC verbose:     %d\n\r", GC_VERBOSE );
    printf( "Test_Charmies:  %s\n\r", TESTING_CHARMIES?"true":"false" );
    printf( "Capture SIGSEGV: %s\n\r", CAPTURE_SIGSEGV?"true":"false");
    printf( "GC use entire heap: %s\n\r", GC_use_entire_heap?"true":"false");
    printf( "GC don't expand: %s\n\r", GC_dont_expand?"true":"false");
    printf( "GC don't gc: %s\n\r", GC_dont_gc?"true":"false");
    printf( "Default race:   %s\n\r", DEFAULT_RACE_NAME );
    printf( "Default pc race:   %s\n\r", DEFAULT_PC_RACE_NAME );
    printf( "Default god:    %s\n\r", DEFAULT_GOD_NAME );
    printf( "Default hometown: %s\n\r", DEFAULT_HOMETOWN_NAME );
    printf( "Default size:    %s\n\r", DEFAULT_SIZE_NAME );
    printf( "Default language: %s\n\r", DEFAULT_LANGUAGE_NAME );
    printf( "Data min security: %d\n\r", DATA_MINIMUM_SECURITY );
    printf( "Script min security: %d\n\r", SCRIPT_MINIMUM_SECURITY );
    printf( "Default recall: %d\n\r", DEFAULT_RECALL );
    printf( "Default hall: %d\n\r", DEFAULT_HALL );
    printf( "Default donation: %d\n\r", DEFAULT_DONATION );
    printf( "Default morgue: %d\n\r", DEFAULT_MORGUE );
    printf( "Default school: %d\n\r", DEFAULT_SCHOOL );
    printf( "Default script filename: %s\n\r", DEFAULT_SCRIPT_FILENAME );
    printf( "Dump mob msg: %s\n\r", DUMP_MOB_MSG?"true":"false");
    printf( "Save state on copyover: %s\n\r", SAVE_STATE_ON_COPYOVER?"true":"false");
    printf( "Starting with world state: %s\n\r", fState?"true":"false");
    printf( "Forbidden list: %s\n\r", FORBIDDEN_LIST );
    printf( "Default group: %s\n\r", DEFAULT_GROUP_NAME );
    printf( "Max Heap: %d\n\r", MAX_HEAP_SIZE );
    printf( "Monitoring: %s\n\r", MONITORING_FILE );
    printf( "SpecialAffects: %s\n\r", AFFECTS_FILE ); 
    printf( "Schools: %s\n\r", SCHOOL_FILE );
    printf( "Stat dir: %s\n\r", STAT_DIR );
    printf( "Super race: %s\n\r", SUPER_RACE_FILE );
    if ( NEEDED_SCRIPT != NULL )
      printf( "Needed scripts: %s\n\r", ((Value)NEEDED_SCRIPT).asStr() );
    printf( "Start with saved time: %s\n\r", START_WITH_SAVED_TIME?"true":"false" );
    printf( "Use faction on aggro: %s\n\r", USE_FACTION_ON_AGGRO?"true":"false" );
    if ( NEEDED_MOB != NULL )
      printf( "Needed mobs: %s\n\r", ((Value)NEEDED_MOB).asStr() );
    if ( NEEDED_OBJ != NULL )
      printf( "Needed objs: %s\n\r", ((Value)NEEDED_OBJ).asStr() );
  }
}

bool configFound;
bool parse_config( DATAData *config ) {
  const char *configName = config->value->eval().asStr();

  if ( str_cmp( configName, wantedConfig ) ) // if not the right config: skip
    return FALSE;

  // fields
  for ( int fieldCount = 0; fieldCount < config->fields_count; fieldCount++ ) {
    DATAData *field = config->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    addContext( tagName, field->value );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    if ( !assign_config_variable( tagId, field->value->eval() ) ) {
      // Already added in the beginning of the loop
      //addContext( tagName, field->value ); // unknown tag are considered as local variable
      if ( DATA_VERBOSE > 3 ) {
	printf("Local variable: %s\n\r", tagName );
      }
      //p_error("Invalid Tag: %s", tagName ); break;
    }
  }
  show_config_variable();
  configFound = TRUE;
  return TRUE;
}

bool load_config( const char *s ) {
  strcpy( wantedConfig, s );

  FILE *fp;
  fclose(fpReserve);
  if ( ( fp = fopen( CONFIG_FILE, "r" ) ) == NULL ) {
    fprintf(stderr,"Can't open config file %s.", CONFIG_FILE );
    exit(-1);
  }

  printf("Searching config [%s] in config file: %s\n\r", wantedConfig, CONFIG_FILE ); fflush(stdout);

  configFound = FALSE;
  parse_datas( fp );
  if ( configFound ) {
    printf("Found.\n\r"); fflush(stdout);
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  return configFound;
}
