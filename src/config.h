#ifndef __CONFIG_H__
#define __CONFIG_H__

#define MAX_FILE_NAME_LENGTH (128)

extern char CONFIG_FILE[MAX_FILE_NAME_LENGTH];
extern char wantedConfig[MAX_STRING_LENGTH];


void assign_default_values();
bool load_config( const char * );
void assign_default_struct();
bool assign_config_variable( const int tagId, const Value &v );
void show_config_variable();


// Data files used by the server.
//
// AREA_LIST contains a list of areas to boot.
// All files are read in completely at bootup.
// Most output files (bug, idea, typo, shutdown) are append-only.
//
// The NULL_FILE is held open so that we have a stream handle in reserve,
//   so players can go ahead and telnet to all the other descriptors.
// Then we close it whenever we need to open a file (e.g. a save file).

#if defined(macintosh)
#define NULL_FILE	               "proto.are"		
#endif

#if defined(MSDOS)
#define NULL_FILE	               "nul"			
#endif

#if defined(unix)
#define NULL_FILE	               "/dev/null"
#endif

extern char PLAYER_DIR[MAX_FILE_NAME_LENGTH];
extern char TEMP_FILE[MAX_FILE_NAME_LENGTH];
extern char GOD_DIR[MAX_FILE_NAME_LENGTH];
extern char HELP_LIST[MAX_FILE_NAME_LENGTH];
extern char AREA_LIST[MAX_FILE_NAME_LENGTH];
extern char BUG_FILE[MAX_FILE_NAME_LENGTH];
extern char TYPO_FILE[MAX_FILE_NAME_LENGTH];
extern char SHUTDOWN_FILE[MAX_FILE_NAME_LENGTH];
extern char AREA_DIR[MAX_FILE_NAME_LENGTH];
extern char SKILLS_FILE[MAX_FILE_NAME_LENGTH];
extern char SPELLS_FILE[MAX_FILE_NAME_LENGTH];
extern char POWERS_FILE[MAX_FILE_NAME_LENGTH];
extern char SONGS_FILE[MAX_FILE_NAME_LENGTH];
extern char AFFECTS_FILE[MAX_FILE_NAME_LENGTH];
extern char LIQ_FILE[MAX_FILE_NAME_LENGTH];
extern char RACE_FILE[MAX_FILE_NAME_LENGTH];
extern char PCRACE_FILE[MAX_FILE_NAME_LENGTH];
extern char PC_CLASS_FILE[MAX_FILE_NAME_LENGTH];
extern char GROUP_FILE[MAX_FILE_NAME_LENGTH];
extern char GODS_FILE[MAX_FILE_NAME_LENGTH];
extern char MATERIAL_FILE[MAX_FILE_NAME_LENGTH];
extern char UNIQUE_ITEM_FILE[MAX_FILE_NAME_LENGTH];
extern char COMMANDS_FILE[MAX_FILE_NAME_LENGTH];
extern char SPHERE_FILE[MAX_FILE_NAME_LENGTH];
extern char BAN_FILE[MAX_FILE_NAME_LENGTH];
extern char BREWFORMULA_FILE[MAX_FILE_NAME_LENGTH];
extern char BREWCOMPONENT_FILE[MAX_FILE_NAME_LENGTH];
extern char PREREQ_FILE[MAX_FILE_NAME_LENGTH];
extern char DISABLED_FILE[MAX_FILE_NAME_LENGTH];
extern char HINT_FILE[MAX_FILE_NAME_LENGTH];
extern char CLAN_FILENAME[MAX_FILE_NAME_LENGTH];
extern char HALL_FAME_FILE[MAX_FILE_NAME_LENGTH];
extern char FACTION_FILE[MAX_FILE_NAME_LENGTH];
extern char COPYOVER_FILE[MAX_FILE_NAME_LENGTH];
extern char LAST_COMMAND_FILE[MAX_FILE_NAME_LENGTH];
extern char SCRIPT_DUMP_FILE[MAX_FILE_NAME_LENGTH];
extern char SCRIPT_LIST[MAX_FILE_NAME_LENGTH];
extern char SCRIPT_DIR[MAX_FILE_NAME_LENGTH];
extern char NOTE_DIR[MAX_FILE_NAME_LENGTH];
extern char HELP_DIR[MAX_FILE_NAME_LENGTH];
extern char DUMP_DIR[MAX_FILE_NAME_LENGTH];
extern char DATA_DIR[MAX_FILE_NAME_LENGTH];
extern char TIME_FILE[MAX_FILE_NAME_LENGTH];
extern char DEFAULT_RACE_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_RACE;
extern char DEFAULT_PC_RACE_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_PC_RACE;
extern char DEFAULT_GOD_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_GOD;
extern char DEFAULT_HOMETOWN_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_HOMETOWN;
extern int DEFAULT_RECALL;
extern int DEFAULT_HALL;
extern int DEFAULT_SCHOOL;
extern int DEFAULT_DONATION;
extern int DEFAULT_MORGUE;
extern char DEFAULT_SIZE_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_SIZE;
extern char DEFAULT_LANGUAGE_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_LANGUAGE;
extern char HOMETOWN_FILE[MAX_FILE_NAME_LENGTH];
extern char DEFAULT_SCRIPT_FILENAME[MAX_FILE_NAME_LENGTH];
extern char FORBIDDEN_LIST[MAX_FILE_NAME_LENGTH];
extern char DEFAULT_GROUP_NAME[MAX_FILE_NAME_LENGTH];
extern int DEFAULT_GROUP;
extern char MONITORING_FILE[MAX_FILE_NAME_LENGTH];
extern char SCHOOL_FILE[MAX_FILE_NAME_LENGTH];
extern char STAT_DIR[MAX_FILE_NAME_LENGTH];
extern char SOCIAL_FILE[MAX_FILE_NAME_LENGTH];
extern char HTML_DIR[MAX_FILE_NAME_LENGTH];
extern char SUPER_RACE_FILE[MAX_FILE_NAME_LENGTH];

extern int SCRIPT_VERBOSE;
extern int DATA_VERBOSE;
extern int AGGR_VERBOSE;

extern CLASS_DATA *default_player_class;

extern int MAX_HEAP_SIZE;

//extern int GC_VERBOSE; // from gc/malloc.c

#endif
