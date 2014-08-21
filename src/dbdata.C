#if defined( macintosh )
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>		
#include <sys/time.h>
#endif
#include <ctype.h>		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "merc.h"
#include "db.h"
//#include "olc.h"
#include "cxx/parser_skel.hh"
#include "data_parser.hh"
#include "dbdata.h"
#include "bit.h"
#include "names.h"
#include "classes.h"
#include "tables.h"
#include "lookup.h"
#include "config.h"
#include "data.h"
#include "ban.h"
#include "prereqs.h"
#include "brew.h"
#include "scanner.hh"
#include "olc_value.h"
#include "utils.h"
#include "affects.h"


// data parsing method from other module: prereqs.C, brew.C, ban.C, data.C, interp.C, clan.C
extern void parse_prerequisite( DATAData* prereq );
extern void parse_brew_formula( DATAData *formula );
extern void create_faction_table( const int count, DATAModule *module );
extern void parse_faction( DATAData *faction );
extern void parse_ban( DATAData *ban );
extern void parse_disabled( DATAData *disabled );
extern void parse_clan( DATAData *clan );
extern void create_spheres_table( const int count );
extern void parse_sphere( DATAData *race );
extern void create_command_table( const int count );
extern void parse_command( DATAData *command );
extern void parse_unique( DATAData *unique );
extern void create_material_table( const int count );
extern void parse_material( DATAData *material );
extern void create_god_table( const int count );
extern void parse_god( DATAData *god );
extern void create_groups_table( const int count );
extern void parse_groups( DATAData *race );
extern void create_races_table( const int count );
extern void parse_race( DATAData *race );
extern void create_pcraces_table( const int count );
extern void parse_pcrace( DATAData *race );
extern void create_liquid_table( const int count );
extern void parse_liquid( DATAData *liquid );
extern void create_hometown_table( const int count );
extern void parse_hometown( DATAData *cla );
extern void parse_time( DATAData *time );
extern void create_classes_table( const int count );
extern void parse_classes( DATAData *cla );
extern void parse_abilities( DATAData *ability );
extern void parse_player( DATAData *player );
extern void parse_area( DATAData *area );
extern bool parse_config( DATAData *config );
extern void parse_area_state( DATAData *area );
extern void create_school_table( const int count );
extern void parse_school( DATAData *school );
extern void create_super_race_table( const int count );
extern void parse_super_race( DATAData *race );

// Private structure
struct tag_list_type {
  const char *tagName;
  const int tagId;
  const int whichData; // bit vector
};
static const struct tag_list_type tagList [] = {
  { "Ban", TAG_Ban, DATA_HEADER },
  { "Level", TAG_Level, DATA_BAN|DATA_BREW|DATA_MOB|DATA_OBJ|DATA_COMMAND|DATA_PREREQ|DATA_DISABLED },
  { "Flags", TAG_Flags, DATA_BAN|DATA_ROOM|DATA_AREA },
  { "BrewFormula", TAG_BrewFormula, DATA_HEADER|DATA_CONFIG },
  { "Component", TAG_Component, DATA_BREW },
  { "Cost", TAG_Cost, DATA_BREW|DATA_OBJ|DATA_PREREQ|DATA_SPHERE|DATA_ABILITY },
  { "Effects", TAG_Effects, DATA_BREW },
  { "Command", TAG_Command, DATA_CONFIG|DATA_HEADER },
  { "Position", TAG_Position, DATA_COMMAND|DATA_ABILITY|DATA_MOB },
  { "Log", TAG_Log, DATA_COMMAND },
  { "Show", TAG_Show, DATA_COMMAND },
  { "Disable", TAG_Disable, DATA_HEADER },
  { "Player", TAG_Player, DATA_HEADER|DATA_DISABLED },
  { "Faction", TAG_Faction, DATA_HEADER|DATA_MOB },
  { "Friendliness", TAG_Friendliness, DATA_FACTION },
  { "God", TAG_God, DATA_HEADER|DATA_MOB },
  { "Title", TAG_Title, DATA_GOD|DATA_MOB|DATA_PC_CLASSES },
  { "Story", TAG_Story, DATA_GOD },
  { "Minor", TAG_Minor, DATA_GOD },
  { "Major", TAG_Major, DATA_GOD },
  { "Priest", TAG_Priest, DATA_GOD },
  { "Alignment", TAG_Alignment, DATA_GOD|DATA_PC_RACE|DATA_PC_CLASSES|DATA_MOB },
  { "Races", TAG_Races, DATA_CLAN|DATA_CONFIG|DATA_GOD|DATA_FACTION },
  { "Classes", TAG_Classes, DATA_CLAN|DATA_GOD|DATA_PC_RACE|DATA_MOB|DATA_PREREQ },
  { "Liquid", TAG_Liquid, DATA_HEADER|DATA_CONFIG },
  { "Color", TAG_Color, DATA_LIQUID|DATA_MATERIAL },
  { "Drunk", TAG_Drunk, DATA_LIQUID },
  { "Full", TAG_Full, DATA_LIQUID },
  { "Thirst", TAG_Thirst, DATA_LIQUID },
  { "Food", TAG_Food, DATA_LIQUID },
  { "Ssize", TAG_Ssize, DATA_LIQUID },
  { "Material", TAG_Material, DATA_HEADER },
  { "Immunities", TAG_Immunities, DATA_MATERIAL|DATA_RACE|DATA_MOB },
  { "Resistances", TAG_Resistances, DATA_MATERIAL|DATA_RACE|DATA_MOB },
  { "Vulnerabilities", TAG_Vulnerabilities,DATA_MATERIAL|DATA_RACE|DATA_MOB },
  { "Metal", TAG_Metal, DATA_MATERIAL },
  { "Unique", TAG_Unique, DATA_UNIQUE|DATA_CONFIG },
  { "Room", TAG_Room, DATA_UNIQUE|DATA_ROOM|DATA_MOB },
  { "Group", TAG_Group, DATA_MOB|DATA_HEADER|DATA_MOB },
  { "Contains", TAG_Contains, DATA_GROUP|DATA_SPHERE },
  { "Ability", TAG_Ability, DATA_CLAN|DATA_HEADER|DATA_MOB },
  { "Type", TAG_Type, DATA_PC_RACE|DATA_ABILITY|DATA_PC_CLASSES },
  { "Target", TAG_Target, DATA_ABILITY },
  { "Slot", TAG_Slot, DATA_ABILITY },
  { "Beats", TAG_Beats, DATA_ABILITY },
  { "NounDamage", TAG_NounDamage, DATA_ABILITY },
  { "MsgOff", TAG_MsgOff, DATA_ABILITY },
  { "MsgObj", TAG_MsgObj, DATA_ABILITY },
  { "MsgRoom", TAG_MsgRoom, DATA_ABILITY },
  { "MobUse", TAG_MobUse, DATA_ABILITY },
  { "Dispel", TAG_Dispel, DATA_ABILITY },
  { "MsgDispel", TAG_MsgDispel, DATA_ABILITY },
  { "Wait", TAG_Wait, DATA_ABILITY },
  { "Craftable", TAG_Craftable, DATA_ABILITY },
  { "Prereq", TAG_Prereq, DATA_HEADER },
  { "NumberCasting", TAG_NumberCasting, DATA_PREREQ },
  { "Casting", TAG_Casting, DATA_PREREQ },
  { "List", TAG_List, DATA_PREREQ },
  { "Sphere", TAG_Sphere, DATA_HEADER },
  { "Clan", TAG_Clan, DATA_HEADER|DATA_ROOM|DATA_MOB },
  { "Id", TAG_Id, DATA_CLAN|DATA_MOB },
  { "WhoName", TAG_WhoName, DATA_CLAN|DATA_PC_RACE|DATA_PC_CLASSES  },
  { "Hall", TAG_Hall, DATA_CLAN|DATA_HOMETOWN },
  { "Recall", TAG_Recall, DATA_CLAN|DATA_HOMETOWN },
  { "Independent", TAG_Independent, DATA_CLAN },
  { "Member", TAG_Member, DATA_CLAN },
  { "Status", TAG_Status, DATA_CLAN },
  { "Class", TAG_Class, DATA_HEADER },
  { "Attributes", TAG_Attributes, DATA_PC_RACE },
  { "Weapon", TAG_Weapon, DATA_PC_CLASSES },
  { "Adept", TAG_Adept, DATA_PC_CLASSES },
  { "Thac0_00", TAG_Thac0_00, DATA_PC_CLASSES },
  { "Thac0_32", TAG_Thac0_32, DATA_PC_CLASSES },
  { "HpMin", TAG_HpMin, DATA_PC_CLASSES },
  { "HpMax", TAG_HpMax, DATA_PC_CLASSES },
  { "CastingRule", TAG_CastingRule, DATA_PC_CLASSES },
  { "BaseGroup", TAG_BaseGroup, DATA_PC_CLASSES },
  { "DefaultGroup", TAG_DefaultGroup, DATA_PC_CLASSES },
  { "Parent", TAG_Parent, DATA_PC_CLASSES },
  { "Choosable", TAG_Choosable, DATA_PC_CLASSES },
  { "ScriptAbility", TAG_ScriptAbility, DATA_ABILITY },
  { "MaxCasting", TAG_MaxCasting, DATA_PC_CLASSES },
  { "Abilities", TAG_Abilities, DATA_PC_CLASSES | DATA_PC_RACE },
  { "Groups", TAG_Groups, DATA_CONFIG|DATA_PC_CLASSES },
  { "Pose", TAG_Pose, DATA_PC_CLASSES },
  { "PCRace", TAG_PCRace, DATA_CONFIG|DATA_HEADER },
  { "Experience", TAG_Experience, DATA_PC_CLASSES|DATA_MOB },
  { "Size", TAG_Size, DATA_PC_RACE|DATA_MOB|DATA_OBJ },
  { "Attribute", TAG_Attribute, DATA_PC_CLASSES },
  { "Language", TAG_Language, DATA_PC_RACE },
  { "Race", TAG_Race, DATA_MOB|DATA_HEADER },
  { "Act", TAG_Act, DATA_RACE|DATA_MOB },
  { "Affect", TAG_Affect, DATA_RACE|DATA_OBJ|DATA_MOB|DATA_ROOM },
  { "Affect2", TAG_Affect2, DATA_RACE },
  { "Offensive", TAG_Offensive, DATA_RACE|DATA_MOB },
  { "Forms", TAG_Forms, DATA_RACE|DATA_MOB },
  { "Parts", TAG_Parts, DATA_RACE|DATA_MOB },
  { "Petition", TAG_Petition, DATA_CLAN|DATA_MOB },
  { "Pet",  TAG_Pet, DATA_MOB },
  { "AfBy",  TAG_AfBy, DATA_MOB },
  { "AfBy2",  TAG_AfBy2, DATA_MOB },
  { "Address",  TAG_Address, DATA_MOB },
  { "Alias",  TAG_Alias, DATA_MOB },
  { "ACs",  TAG_ACs, DATA_MOB },
  { "Attr",  TAG_Attr, DATA_MOB },
  { "Bin",  TAG_Bin, DATA_MOB },
  { "Bout",  TAG_Bout, DATA_MOB },
  { "Beacons",  TAG_Beacons, DATA_MOB },
  { "Boards",  TAG_Boards, DATA_MOB },
  { "ClanStatus",  TAG_ClanStatus, DATA_MOB },
  { "Condition",  TAG_Condition, DATA_MOB },
  { "Comm",  TAG_Comm, DATA_MOB },
  { "Damroll",  TAG_Damroll, DATA_MOB },
  { "Dnumber",  TAG_Dnumber, DATA_MOB },
  { "Dtype",  TAG_Dtype, DATA_MOB },
  { "DamType",  TAG_DamType, DATA_MOB },
  { "Desc",  TAG_Desc, DATA_MOB|DATA_OBJ|DATA_ROOM },
  { "DisabledCmd",  TAG_DisabledCmd, DATA_MOB },
  { "ExField",  TAG_ExField, DATA_MOB|DATA_OBJ|DATA_ROOM },
  { "Etho",  TAG_Etho, DATA_MOB },
  { "Gold",  TAG_Gold, DATA_MOB },
  { "Gset",  TAG_Gset, DATA_MOB },
  { "Hitroll",  TAG_Hitroll, DATA_MOB },
  { "Hometown",  TAG_Hometown, DATA_MOB|DATA_CONFIG|DATA_HEADER },
  { "HMVP",  TAG_HMVP, DATA_MOB },
  { "Invis",  TAG_Invis, DATA_MOB },
  { "Inco",  TAG_Inco, DATA_MOB },
  { "Immtitle",  TAG_Immtitle, DATA_MOB },
  { "LastLevel",  TAG_LastLevel, DATA_MOB },
  { "LogO",  TAG_LogO, DATA_MOB },
  { "LnD",  TAG_LnD, DATA_MOB },
  { "LineMode",  TAG_LineMode, DATA_MOB },
  { "NameAccepted",  TAG_NameAccepted, DATA_MOB },
  { "Password",  TAG_Password, DATA_MOB },
  { "Played",  TAG_Played, DATA_MOB },
  { "Pnts",  TAG_Pnts, DATA_MOB },
  { "Prac",  TAG_Prac, DATA_MOB },
  { "Prompt",  TAG_Prompt, DATA_MOB },
  //  { "QuestNext",  TAG_QuestNext, DATA_MOB },
  //  { "QuestCount",  TAG_QuestCount, DATA_MOB },
  //  { "QuestObj",  TAG_QuestObj, DATA_MOB },
  //  { "QuestObjLoc",  TAG_QuestObjLoc, DATA_MOB },
  //  { "QuestMob",  TAG_QuestMob, DATA_MOB },
  //  { "QuestGiver",  TAG_QuestGiver, DATA_MOB },
  { "SavingThrow",  TAG_SavingThrow, DATA_MOB },
  { "Scroll",  TAG_Scroll, DATA_MOB },
  { "Sex",  TAG_Sex, DATA_MOB },
  { "ShD",  TAG_ShD, DATA_MOB|DATA_OBJ },
  { "Security",  TAG_Security, DATA_MOB },
  { "Silver",  TAG_Silver, DATA_MOB },
  { "Stance",  TAG_Stance, DATA_MOB },
  { "Train",  TAG_Train, DATA_MOB },
  { "Trust",  TAG_Trust, DATA_MOB },
  { "Trivia",  TAG_Trivia, DATA_MOB },
  { "Version",  TAG_Version, DATA_MOB },
  { "Wimpy",  TAG_Wimpy, DATA_MOB },
  { "Wiznet",  TAG_Wiznet, DATA_MOB },
  { "Enchanted",  TAG_Enchanted, DATA_OBJ|DATA_MOB},
  { "ExtraFlags",  TAG_ExtraFlags, DATA_OBJ },
  { "ExtraDescr",  TAG_ExtraDescr, DATA_OBJ|DATA_ROOM },
  { "ItemType",  TAG_ItemType, DATA_OBJ },
  { "Owner",  TAG_Owner, DATA_OBJ|DATA_ROOM },
  { "Oldstyle",  TAG_Oldstyle, DATA_OBJ },
  { "Restriction",  TAG_Restriction, DATA_OBJ },
  { "Spell",  TAG_Spell, DATA_OBJ },
  { "Timer",  TAG_Timer, DATA_OBJ },
  { "Values",  TAG_Values, DATA_OBJ },
  { "WearFlags",  TAG_WearFlags, DATA_OBJ },
  { "WearLoc",  TAG_WearLoc, DATA_OBJ },
  { "Weight",  TAG_Weight, DATA_OBJ },
  { "Obj", TAG_Obj, DATA_PC_CLASSES|DATA_AREA|DATA_OBJ|DATA_ROOM|DATA_MOB},
  { "Name", TAG_Name, DATA_OBJ|DATA_MOB|DATA_ROOM },
  //  { "QuestPnts", TAG_QuestPnts, DATA_MOB },
  { "Mobile", TAG_Mobile, DATA_AREA|DATA_MOB|DATA_ROOM },
  { "Hit", TAG_Hit, DATA_MOB },
  { "Mana", TAG_Mana, DATA_MOB },
  { "Psp", TAG_Psp, DATA_MOB },
  { "Damage", TAG_Damage, DATA_MOB },
  { "Wealth", TAG_Wealth, DATA_MOB },
  { "Program", TAG_Program, DATA_MOB|DATA_OBJ|DATA_ROOM },
  { "Special", TAG_Special, DATA_MOB },
  { "Shop", TAG_Shop, DATA_MOB },
  { "Upgrade", TAG_Upgrade, DATA_OBJ },
  { "Sector", TAG_Sector, DATA_ROOM },
  { "MaxSize", TAG_MaxSize, DATA_ROOM },
  { "Rate", TAG_Rate, DATA_ROOM },
  { "Guilds", TAG_Guilds, DATA_ROOM },
  { "Repop", TAG_Repop, DATA_ROOM },
  { "Exit", TAG_Exit, DATA_ROOM },
  { "Reset", TAG_Reset, DATA_ROOM },
  { "Area", TAG_Area, DATA_HEADER },
  { "Builders", TAG_Builders, DATA_AREA },
  { "Vnums", TAG_Vnums, DATA_AREA },
  { "Credits", TAG_Credits, DATA_AREA },
  { "Object", TAG_Object, DATA_AREA },

  { "Config", TAG_Config, DATA_HEADER },
  { "Port", TAG_Port, DATA_CONFIG },
  { "Bug", TAG_Bug, DATA_CONFIG },
  { "Typo", TAG_Typo, DATA_CONFIG },
  { "Hints", TAG_Hints, DATA_CONFIG },
  { "Brew", TAG_Brew, DATA_CONFIG },
  { "Clans", TAG_Clans, DATA_CONFIG },
  { "HallFame", TAG_HallFame, DATA_CONFIG },
  { "PCRaces", TAG_PCRaces, DATA_CONFIG },
  { "PCClasses", TAG_PCClasses, DATA_CONFIG },
  { "Gods", TAG_Gods, DATA_CONFIG },
  { "Commands", TAG_Commands, DATA_CONFIG },
  { "Disabled", TAG_Disabled, DATA_CONFIG },
  { "Factions", TAG_Factions, DATA_CONFIG },
  { "Spheres", TAG_Spheres, DATA_CONFIG },
  { "Prerequisites", TAG_Prerequisites, DATA_CONFIG },
  { "Skills", TAG_Skills, DATA_CONFIG },
  { "Spells", TAG_Spells, DATA_CONFIG },
  { "Powers", TAG_Powers, DATA_CONFIG },
  { "Songs", TAG_Songs, DATA_CONFIG },
  { "Script_List", TAG_Script_List, DATA_CONFIG },
  { "Help_List", TAG_Help_List, DATA_CONFIG },
  { "Area_List", TAG_Area_List, DATA_CONFIG },
  { "Shutdown", TAG_Shutdown, DATA_CONFIG },
  { "Copyover", TAG_Copyover, DATA_CONFIG },
  { "Last_Command", TAG_Last_Command, DATA_CONFIG },
  { "Script_Dump", TAG_Script_Dump, DATA_CONFIG },
  { "Player_Dir", TAG_Player_Dir, DATA_CONFIG },
  { "God_Dir", TAG_God_Dir, DATA_CONFIG },
  { "Script_Dir", TAG_Script_Dir, DATA_CONFIG },
  { "Note_Dir", TAG_Note_Dir, DATA_CONFIG },
  { "Area_Dir", TAG_Area_Dir, DATA_CONFIG },
  { "Temp", TAG_Temp, DATA_CONFIG },
  { "Help_Dir", TAG_Help_Dir, DATA_CONFIG },
  { "Dump_Dir", TAG_Dump_Dir, DATA_CONFIG },
  { "Data_Verbose", TAG_Data_Verbose, DATA_CONFIG },
  { "Script_Verbose", TAG_Script_Verbose, DATA_CONFIG },
  { "Wizlock", TAG_Wizlock, DATA_CONFIG },
  { "Newlock", TAG_Newlock, DATA_CONFIG },
  { "Time", TAG_Time, DATA_CONFIG|DATA_HEADER},
  { "Hour", TAG_Hour, DATA_TIME },
  { "Date", TAG_Date, DATA_TIME },
  { "Change", TAG_Change, DATA_TIME },
  { "mmHg", TAG_mmHg, DATA_TIME },
  { "Moons", TAG_Moons, DATA_TIME },
  { "GC_Verbose", TAG_GC_Verbose, DATA_CONFIG },
  { "Test_Charmies", TAG_Test_Charmies, DATA_CONFIG },
  { "Capture_SIGSEGV", TAG_Capture_SIGSEGV, DATA_CONFIG },
  { "GC_Use_Entire_Heap", TAG_GC_Use_Entire_Heap, DATA_CONFIG },
  { "GC_Dont_Expand", TAG_GC_Dont_Expand, DATA_CONFIG },
  { "GC_Dont_GC", TAG_GC_Dont_GC, DATA_CONFIG },
  { "Default_Race", TAG_Default_Race, DATA_CONFIG },
  { "Default_PC_Race", TAG_Default_PC_Race, DATA_CONFIG },
  { "Default_God", TAG_Default_God, DATA_CONFIG },
  { "Default_Hometown", TAG_Default_Hometown, DATA_CONFIG },
  { "Default_Size", TAG_Default_Size, DATA_CONFIG },
  { "Default_Language", TAG_Default_Language, DATA_CONFIG },
  { "Data_Min_Security", TAG_Data_Min_Security, DATA_CONFIG },
  { "Script_Min_Security", TAG_Script_Min_Security, DATA_CONFIG },
  { "Default_Recall", TAG_Default_Recall, DATA_CONFIG },
  { "Default_Hall", TAG_Default_Hall, DATA_CONFIG },
  { "Default_Script_Filename", TAG_Default_Script_Filename, DATA_CONFIG },
  { "Dump_Mob_Msg", TAG_Dump_Mob_Msg, DATA_CONFIG },
  { "Save_State_On_Copyover", TAG_Save_State_On_Copyover, DATA_CONFIG },
  { "Start_With_World_State", TAG_Start_World_State, DATA_CONFIG },
  { "Forbidden_List", TAG_Forbidden_List, DATA_CONFIG },
  { "Default_Group", TAG_Default_Group, DATA_CONFIG },
  { "Max_Heap_Size", TAG_Max_Heap_Size, DATA_CONFIG },
  { "Daze", TAG_Daze, DATA_MOB },
  { "Stunned", TAG_Stunned, DATA_MOB },
  { "Hunting", TAG_Hunting, DATA_MOB },
  { "BettedOn", TAG_BettedOn, DATA_MOB },
  { "BetAmt", TAG_BetAmt, DATA_MOB },
  { "Challenged", TAG_Challenged, DATA_MOB },
  { "Remort", TAG_Remort, DATA_PC_RACE },
  { "MinNumRemort", TAG_Min_Num_Remort, DATA_PC_RACE },
  { "RemortCount", TAG_Remort_Count, DATA_MOB },
  { "Monitoring", TAG_Monitoring_File, DATA_CONFIG },
  { "Affects", TAG_Affects, DATA_CONFIG },
  { "School", TAG_School, DATA_SCHOOL },
  { "Schools", TAG_School_File, DATA_CONFIG },
  { "Rebirth", TAG_Rebirth, DATA_PC_RACE },
  { "Morgue", TAG_Morgue, DATA_HOMETOWN },
  { "Donation", TAG_Donation, DATA_HOMETOWN },
  { "Default_Morgue", TAG_Default_Morgue, DATA_CONFIG },
  { "Default_Donation", TAG_Default_Donation, DATA_CONFIG },
  { "Default_School", TAG_Default_School, DATA_CONFIG },
  { "Stat_Dir", TAG_Stat_Dir, DATA_CONFIG },
  { "Violence", TAG_Violence, DATA_TIME },
  { "Point", TAG_Point, DATA_TIME },
  { "Auction", TAG_Auction, DATA_TIME },
  { "Hint", TAG_Hint, DATA_TIME },
  { "ArenaStart", TAG_ArenaStart, DATA_TIME },
  { "Arena", TAG_Arena, DATA_TIME },
  { "Quake", TAG_Quake, DATA_TIME },
  { "DoubleXp", TAG_DoubleXp, DATA_TIME },
  { "DoubleXpDuration", TAG_DoubleXpDuration, DATA_TIME },
  { "Start_With_Saved_Time", TAG_Start_With_Saved_Time, DATA_CONFIG },
  { "Clazz", TAG_Clazz, DATA_MOB },
  { "Needed_Script", TAG_Needed_Script, DATA_CONFIG },
  { "Social", TAG_Social, DATA_CONFIG },
  { "Use_Faction_On_Aggro", TAG_Use_Faction_On_Aggro, DATA_CONFIG },
  { "Html_Dir", TAG_Html_Dir, DATA_CONFIG },
  { "Aggr_Verbose", TAG_Aggr_Verbose, DATA_CONFIG },
  { "isWildMagic", TAG_IsWildMagic, DATA_MOB },
  { "AcceptWildMagic", TAG_AcceptWildMagic, DATA_GOD },
  { "Needed_Obj", TAG_Needed_Obj, DATA_CONFIG },
  { "Needed_Mob", TAG_Needed_Mob, DATA_CONFIG },
  { "SuperRace", TAG_Super_Race, DATA_SUPER_RACE },
  { "SuperRaces", TAG_Super_Races, DATA_CONFIG },

  { "AreaState", TAG_AreaState, DATA_HEADER },
  { "Master", TAG_Master, DATA_MOB },
  { "Fighting", TAG_Fighting, DATA_MOB },
  { "Ptr", TAG_Ptr, DATA_MOB },
  { "Leader", TAG_Leader, DATA_MOB },

  { NULL, -1, -1 }
};

//void assign_tags() {
//  int tagValue = 0;
//  for ( int i = 0; tagList[i].tagName != NULL; i++ )
//    *tagList[i].tagId = tagValue++;
//}


int data_depth = 0;
void sh_fprintf( FILE *fp, const char *str, ... ) {
  char buf[MAX_STRING_LENGTH*4];
  va_list argptr;

  buf[0] = '\0';
  if ( data_depth > 0 ) {
    memset(buf, ' ', 2*(data_depth-1) ); // add spaces
    buf[2*(data_depth-1)] = '\0';
  }

  va_start(argptr, str); // get format information
  vsprintf(buf + strlen(buf) , str, argptr);
  va_end(argptr);

  fprintf( fp, buf );
}








// Convert a flag into a string each word inside '' and seperated by ,
const char *list_flag_string_init( const long flag, struct flag_type *flag_table, const char *quote, const char *separator ) {
  char buf2[MAX_STRING_LENGTH];
  static char buf[MAX_STRING_LENGTH];
  strcpy( buf2, flag_string_init( flag_table, flag ) );

  if ( !str_cmp( buf2, "none") )
    return str_dup( quotify("none",quote) );

  int nb_space = 1;                  // if not bitvector -> only 1 value
  if ( !is_stat_init( flag_table ) ) // if bitvector -> count bit
    nb_space = count_bit( flag ); // number of words

  buf[0] = '\0';
  char *s = buf2;
  char* tok = strsep(&s, " " );
  int i = 0;
  while ( tok != NULL ) {
    if ( tok[0] != '\0' ) {
      //strcat( buf, bracket);
      //strcat( buf, tok );
      //strcat( buf, bracket);
      strcat( buf, quotify( tok, quote ) );
      if ( i < nb_space-1 )
	strcat( buf, separator );
      i++;
    }
    
    tok = strsep(&s, " " );
  }
  if ( i != nb_space && !is_stat_init(flag_table) ) {
    bug("List_Flag_String_Init: invalid number of word: %d, number of bits %d [flag = %ld] [table: %s]",
	i, nb_space, flag, get_flag_table_name_init(flag_table) );
    buf[strlen(buf)-strlen(separator)] = '\0';
  }

  return str_dup(buf);
}
// Convert a flag into a string each word inside '' and separated by ,
const char *list_flag_string( const long flag, struct flag_type *flag_table, const char *quote, const char *separator ) {
  char buf2[MAX_STRING_LENGTH];
  static char buf[MAX_STRING_LENGTH];
  strcpy( buf2, flag_string( flag_table, flag ) );

  if ( !str_cmp( buf2, "none") )
    return str_dup( quotify("none",quote) );

  int nb_space = 1;             // if not bitvector -> only 1 value
  if ( !is_stat( flag_table ) ) // if bitvector -> count bit
    nb_space = count_bit( flag ); // number of words

  //  log_stringf("table: %s  nb_space: %d   buf2: %s   flag: %ld", get_flag_table_name(flag_table), nb_space, buf2, flag );

  buf[0] = '\0';
  char *s = buf2;
  char* tok = strsep(&s, " " );
  int i = 0;
  while ( tok != NULL ) {
    if ( tok[0] != '\0' ) {
      //strcat( buf, bracket);
      //strcat( buf, tok );
      //strcat( buf, bracket);
      strcat( buf, quotify( tok, quote ) );
      if ( i < nb_space-1 )
	   //	   && tok != NULL && tok[0] != NULL )
	strcat( buf, separator );
      i++;
    }

    tok = strsep(&s, " " );
  }
  if ( i != nb_space && !is_stat(flag_table) ) {
    bug("List_Flag_String: invalid number of word: %d, number of bits %d [flag = %ld] [table: %s]",
	i, nb_space, flag, get_flag_table_name(flag_table) );
    buf[strlen(buf)-strlen(separator)] = '\0';
  }

  return str_dup(buf);
}


// Convert a DATAList flag into a long integer value
long list_flag_value( DATAExpression *flag, struct flag_type *flag_table ) {
  ValueList *list = flag->eval().asList();
  char flagString[MAX_STRING_LENGTH];
  flagString[0] = '\0';

  if ( list->size == 0 )
    return 0;

  if ( list->size == 1 )
    if ( !str_cmp( "none", list->elems[0].asStr() ) )
      return 0;
  // Any  is accepted as element in a list only if flag_table is bit based
    else if ( !str_cmp( "any", list->elems[0].asStr() )
	      && !is_stat_init( flag_table ) ) {
      int bit = 0;
      for (int flag = 0; flag_table[flag].name != NULL; flag++)
	bit |= flag_table[flag].bit;
      return bit;
    }
  
  // Convert the list into a string
  for ( int i = 0; i < list->size; i++ ) {
    strcat( flagString, list->elems[i].asStr() );
    if ( i < list->size-1 )
      strcat( flagString, " " );
  }

  list->explicit_free();

  // Send the string to flag_value
  return flag_value( flag_table, flagString );
}

// Convert a DATAList flag into a long integer value
long list_flag_value_complete( DATAExpression *flag, struct flag_type *flag_table ) {
  ValueList *list = flag->eval().asList();
  char flagString[MAX_STRING_LENGTH];
  flagString[0] = '\0';

  if ( list->size == 0 || ( list->size == 1 && !str_cmp( "none", list->elems[0].asStr() ) ) )
    return 0;
  
  // Convert the list into a string
  for ( int i = 0; i < list->size; i++ ) {
    strcat( flagString, list->elems[i].asStr() );
    if ( i < list->size-1 )
      strcat( flagString, " " );
  }

  list->explicit_free();

  // Send the string to flag_value
  return flag_value_complete( flag_table, flagString );
}

// Convert a ValueList flag into a long integer value
long list_flag_value( ValueList *list, struct flag_type *flag_table ) {
  char flagString[MAX_STRING_LENGTH];
  flagString[0] = '\0';

  if ( list->size == 0 || ( list->size == 1 && !str_cmp( "none", list->elems[0].asStr() ) ) )
    return 0;
  
  // Convert the list into a string
  for ( int i = 0; i < list->size; i++ ) {
    strcat( flagString, list->elems[i].asStr() );
    if ( i < list->size-1 )
      strcat( flagString, " " );
  }

  // Send the string to flag_value
  return flag_value( flag_table, flagString );
}

// Convert a ValueList flag into a long integer value
long list_flag_value_complete( ValueList *list, struct flag_type *flag_table ) {
  char flagString[MAX_STRING_LENGTH];
  flagString[0] = '\0';

  if ( list->size == 0 || ( list->size == 1 && !str_cmp( "none", list->elems[0].asStr() ) ) )
    return 0;
  
  // Convert the list into a string
  for ( int i = 0; i < list->size; i++ ) {
    strcat( flagString, list->elems[i].asStr() );
    if ( i < list->size-1 )
      strcat( flagString, " " );
  }

  // Send the string to flag_value
  return flag_value_complete( flag_table, flagString );
}


const char *duplicate( const char *input, const char c ) {
  char output[MAX_STRING_LENGTH];
  char *p = &output[0];
  const char *q = input;
  
  while ( *q != '\0' ) {
    if ( *q == c )
      *p++ = c;
    *p++ = *q++;
  }
  *p = '\0';
  return str_dup(output);
}

const char *quotify( const char *input, const char *quote ) {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  static char output[MAX_STRING_LENGTH];

  //strcpy( buf, input );

  //strcpy( buf2, duplicate( input, '\\' ));
  //strcpy( buf, duplicate( buf2, '%' ) );

  strcpy( buf, duplicate( input, '\\' ));

  char *s = buf;
  char *tok = strsep(&s, quote );
  if ( tok == NULL ) {
    strcpy( output, input );
    return output;
  }
  buf2[0] = '\0';
  int i = 0;
  while ( tok != NULL ) {
    strcat( buf2, tok );
    tok = strsep(&s, quote );
    if ( tok != NULL ) {
      strcat( buf2, "\\");
      strcat( buf2, quote );
    }
  }
  strcpy( output, quote );
  strcat( output, buf2 );
  strcat( output, quote );
  return str_dup(output);
}

const char * create_classes_list( const int count, int *classes ) {
  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  // construct classes list
  for ( int i = 0; i < count; i++ ) {
    strcat( buf, quotify( class_name( classes[i] ) ) );
    if ( i < count-1 )
      strcat( buf, ", " );
  }
  return str_dup(buf);
}
const char * create_races_list( const int count, int *races ) {
  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  // construct races list
  for ( int i = 0; i < count; i++ ) {
    strcat( buf, quotify( race_table[ races[i] ].name ) );
    if ( i < count-1 )
      strcat( buf, ", " );
  }
  return str_dup(buf);
}
const char * create_abilities_list( const int count, int *abilities ) {
  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  // construct abilities list
  for ( int i = 0; i < count; i++ ) {
    strcat( buf, quotify( ability_table[ abilities[i] ].name ) );
    if ( i < count-1 )
      strcat( buf, ", " );
  }
  return str_dup(buf);
}
const char * create_alignment_list( const int count, ALIGN_INFO *align ) {
  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  // construct alignment list
  for ( int i = 0; i < count; i++ ) {
    strcat( buf, quotify( etho_align_name(align[i].etho, 
					  align[i].alignment) ) );
    if ( i < count-1 )
      strcat( buf, ", " );
  }
  return str_dup(buf);
}
const char * create_groups_list( const int count, int *groups ) {
  char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  // construct groups list
  for ( int i = 0; i < count; i++ ) {
    strcat( buf, quotify( group_table[ groups[i] ].name ) );
    if ( i < count-1 )
      strcat( buf, ", " );
  }
  return str_dup(buf);
}

// Check duplicates
void check_tags() {
  for ( int i = 0; tagList[i].tagName != NULL; i++ )
    for ( int j = i+1; tagList[j].tagName != NULL; j++ )
      if ( !str_cmp( tagList[i].tagName, tagList[j].tagName )
	   || tagList[i].tagId == tagList[j].tagId ) {
	bug("tags #%d and #%d have the same name (%s/%s) or id (%d/%d)",
	    i, j, 
	    tagList[i].tagName, tagList[j].tagName, 
	    tagList[i].tagId, tagList[j].tagId );
	exit(-1);
      }
}

// Return tagId related to tagName
// -1 if not found
int find_tag( const char *tagName, int whichData ) {
  for ( int i = 0; tagList[i].tagName != NULL; i++ )
    if ( !str_cmp( tagName, tagList[i].tagName )
	 && ( whichData == -1
	      || IS_SET( tagList[i].whichData, whichData ) ) )
      return tagList[i].tagId;
  return -1;
}

// General data parsing method
bool old_parse_affect( DATAData *pafD, AFFECT_DATA *paf ) {
  bug("NOT ANYMORE AVAILABLE");
  return FALSE;
}
void old_save_affects( AFFECT_DATA *paf, FILE *fp ) {
  bug("NOT ANYMORE AVAILABLE");
}

// General data parsing method
bool parse_affect( DATAData *pafD, AFFECT_DATA *paf ) {
//    Affect = ( 'enchant armor', -1, 110, 0, ('permanent'),
//		( 'char', 'damroll', 0, 50 ),
//              ( 'char', 'hitroll', 0, 50 ) );
  ValueList *list = pafD->value->eval().asList();
  // Minimum 5 elements but revelant affects have at least 6 elements
  if ( list->size < 5 ) {
    bug("Wrong number of elements in affect [<name>]<where><location><op><modifier><duraction><level><casting level>");
    return FALSE;
  }
  // ability, duration, level, casting, (flags),
  //  ( where, loc, op, modifier ),
  // ..
  const char *s = list->elems[0].asStr();
  int sn;
  if ( !str_cmp( s, "none" ) ) // not an ability affect
    sn = -1;
  else {
    sn = ability_lookup( s );
    if ( sn < 0 ) {
      bug("Invalid ability name %s", s );
      return FALSE;
   }
  }
  int dur = list->elems[1].asInt();
  int level = list->elems[2].asInt();
  int casting = list->elems[3].asInt();
  long flags = list_flag_value_complete(list->elems[4].asList(),affect_data_flags);
  if ( flags == NO_FLAG ) {
    flags = ( sn == -1 ) ? AFFECT_INHERENT : AFFECT_ABILITY;
    bug("Invalid affect flag, assuming %s", flag_string( affect_data_flags, flags ) );
  }

  createaff( *paf, dur, level, sn, casting, flags );
  //if ( list->size == 5 )
  //bug("Irrevelant affect: %d %d %s %d %ld", dur, level, s, casting, flags );


  for ( int i = 5; i < list->size; i++ ) { // now, extract affect_list
    ValueList *alist = list->elems[i].asList();
    // where, loc, op, modifier
    s = alist->elems[0].asStr(); int where = flag_value( afto_type, s );
    if ( where == NO_FLAG ) {
      bug("Invalid where name %s", s );
      return FALSE;
    }
    int loc = ATTR_NA;
    int mod = 0;
    switch (where) {
    case AFTO_CHAR: 
      s = alist->elems[1].asStr(); loc = flag_value( attr_flags, s );
      if ( loc == NO_FLAG ) {
	bug("Invalid location name %s", s );
	return FALSE;
      }
      if (attr_table[loc].bits == NULL)
	mod = alist->elems[3].asInt();
      else
	mod = list_flag_value_complete( alist->elems[3].asList(), attr_table[loc].bits );
      break;
    case AFTO_ROOM:
      s = alist->elems[1].asStr(); loc = flag_value( room_attr_flags, s );
      if ( loc == NO_FLAG ) {
	bug("Invalid location name %s", s );
	return FALSE;
      }
      if (room_attr_table[loc].bits == NULL)
	mod = alist->elems[3].asInt();
      else
	mod = list_flag_value_complete( alist->elems[3].asList(), room_attr_table[loc].bits );
      break;
    case AFTO_OBJVAL:
      loc = alist->elems[1].asInt();
      mod = alist->elems[3].asInt();
      break;
    case AFTO_WEAPON:
      // loc  not revelant
      mod = list_flag_value_complete( alist->elems[3].asList(), weapon_type2 );
      break;
    case AFTO_OBJECT:
      // loc  not revelant
      mod = list_flag_value_complete( alist->elems[3].asList(), extra_flags );
      break;
    default:
      bug("Invalid where: %s", s );
      break;
    }
    int op = alist->elems[2].asInt();
    //int mod = alist->elems[3].asInt();
    addaff2(*paf,where,loc,op,mod);
  }
  list->explicit_free();
  return TRUE;
}
void new_save_affect( AFFECT_DATA *paf, FILE *fp ) {
  //    Affect = ( 'enchant armor', -1, 110, 0, ('permanent'),
  //	 	 ( 'char', 'damroll', 0, 50 ),
  //             ( 'char', 'hitroll', 0, 50 ) );
  char buf[MAX_STRING_LENGTH*2];
  sprintf( buf, "  Affect = ( %s, %d, %d, %d, (%s)", 
	   paf->type >= 0 ? quotify(ability_table[paf->type].name) : "'none'",
	   paf->duration, paf->level, paf->casting_level,
	   list_flag_string( paf->flags, affect_data_flags, "'", ", " ) );
  if ( paf->list != NULL ) {
    strcat( buf, ",\n" );
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
      char buf2[MAX_STRING_LENGTH];
      char loc[MAX_STRING_LENGTH];
      char mod[MAX_STRING_LENGTH];
      switch (laf->where) {
      case AFTO_CHAR: 
	strcpy( loc, quotify(flag_string(attr_flags,laf->location)) );
	if (attr_table[laf->location].bits == NULL)
	  sprintf( mod, "%ld", laf->modifier);
	else
	  sprintf( mod, "(%s)", list_flag_string(laf->modifier,attr_table[laf->location].bits));
	break;
      case AFTO_ROOM:
	strcpy( loc, quotify(flag_string(room_attr_flags,laf->location)) );
	if (room_attr_table[laf->location].bits == NULL)
	  sprintf( mod, "%ld", laf->modifier);
	else
	  sprintf( mod, "(%s)", list_flag_string(laf->modifier,room_attr_table[laf->location].bits));
	break;
      case AFTO_OBJVAL:
	sprintf( loc, "%d", laf->location );
	sprintf( mod, "%ld", laf->modifier);
	break;
      case AFTO_WEAPON:
	sprintf( loc, "'weapon'" );
	sprintf( mod,"(%s)", list_flag_string( laf->modifier, weapon_type2 ) );
	break;
      case AFTO_OBJECT:
	strcpy( loc, "'extra'" );
	sprintf( mod,"(%s)", list_flag_string( laf->modifier, extra_flags) );
	break;
      }
      //sprintf( buf2, "            ( %s, %s, %d, %ld )",
      sprintf( buf2, "            ( %s, %s, %d, %s )",
	       quotify(flag_string(afto_type,laf->where)),
	       loc,
	       laf->op,
	       //laf->modifier);
	       mod );
      strcat( buf, buf2 );
      if ( laf->next != NULL )
	strcat( buf, ",\n");
    }
  }
  strcat( buf, " );\n");
  sh_fprintf( fp, buf );
}


// only one scope
#define MAX_DATA_CONTEXT_VARIABLE  (256)
struct DataContext {
  int numvar;

  Value* val[256]; // max context variable
  const char* name[256]; // max context variable name

 public:

  int getVarIndex( const char *n ) {
    for ( int i = 0; i < numvar; i++ )
      if ( !str_cmp( n, name[i] ) )
	return i;
    return -1;
  }
  Value* newVar(const char* n) {
    ASSERT(numvar < MAX_DATA_CONTEXT_VARIABLE - 1, "data context has too much variables");
    
    int varIndex = getVarIndex(n);
    Value* res;
    if ( varIndex == -1 ) { // not found
      res = new Value((long int)0);
      val[numvar] = res;
      name[numvar] = n;
      numvar++;
    }
    else { // already exists: overwrite
      res = val[varIndex];
      name[varIndex] = n;
    }
    return res;
  }
  Value* getVar(const char* n) {
    //for (int i = 0; i<numvar; i++)
    //  if (!str_cmp(n, name[i]))
    //return val[i];
    int i = getVarIndex(n);
    if ( i == -1 )
      return NULL;
    else
      return val[i];
  }

  void clean() {
    numvar = 0;
  };

  DataContext() {
    numvar = 0;
  };
  ~DataContext() {
    clean();
  }
};

static DataContext *data_context;
void addContext( const char *varName, DATAExpression *varValue ) {
  Value var = data_context->newVar(varName);
  var.setValue(varValue->eval());
}
Value* getContext( const char *varName ) {
  return data_context->getVar(varName);
}

DATAModule *get_data_module( FILE *fp ) {
  int actual_size = 0;
  large_buffer[0] = '\0';
  actual_size = fread(large_buffer, 1, BIG_SIZE, fp);
  large_buffer[actual_size] = '\0';

  if ( DATA_VERBOSE > 5 ) {
    log_stringf("dump:\n\r"
		"%s\n\r"
		"actual size: %d bytes\n\r",
		large_buffer,actual_size);
  }

  cur_pos = large_buffer;

  init_data_parsing();

  bool fatalError = TRUE; // if true, catch leads to exit(-1)

  DATAModule* module;

  try {
     module = (DATAModule*)parse();

    if ( DATA_VERBOSE > 6 ) {
      module->dump(1);
    }

    if ( DATA_VERBOSE > 1 ) {
      printf("Data found: %d\n\r", module->datas_count);
    }

    return module;

  } catch (ScriptException e) {
    bug("Error while parsing datas. %s", e.msg );
    if ( fatalError ) {
      bug("FATAL error. Bye!");
      exit(-1);
    }
    else
      return NULL;
  }

  dump_GC_info();

  return NULL;
}

int parse_data_module( DATAModule *module ) {
  int data_count;
  bool fatalError = TRUE; // if true, catch leads to exit(-1)
  try {
    data_count = module->datas_count;
    for ( int i = 0; i < module->datas_count; i++ ) {
      DataContext ctx; data_context = &ctx;

      const char *tagName = module->datas[i]->tag->image;
      const int tagId = find_tag( tagName );
      if ( DATA_VERBOSE > 4 ) {
	printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
      }
      switch ( tagId ) {
      case TAG_Prereq: parse_prerequisite( module->datas[i] ); break;
      case TAG_BrewFormula: 
	create_brew_formula_table( module->datas_count ); 
	parse_brew_formula( module->datas[i] ); 
	break;
      case TAG_Ban: parse_ban( module->datas[i] ); break;
      case TAG_Command:
	create_command_table( module->datas_count );
	parse_command( module->datas[i] );
	break;
      case TAG_Unique: parse_unique( module->datas[i] ); break;
      case TAG_Disable: parse_disabled( module->datas[i] ); break;
      case TAG_Material: 
	create_material_table( module->datas_count );
	parse_material( module->datas[i] );
	break;
      case TAG_Liquid:
	create_liquid_table( module->datas_count );
	parse_liquid( module->datas[i] );
	break;
      case TAG_God:
	create_god_table( module->datas_count );
	parse_god( module->datas[i] );
	break;
      case TAG_Clan: parse_clan( module->datas[i] ); break;
      case TAG_Faction:
	create_faction_table( module->datas_count, module );
	parse_faction( module->datas[i] );
	break;
      case TAG_Race:
	create_races_table( module->datas_count );
	parse_race( module->datas[i] );
	break;
      case TAG_PCRace:
	create_pcraces_table( module->datas_count );
	parse_pcrace( module->datas[i] );
	break;
      case TAG_Sphere:
	create_spheres_table( module->datas_count );
	parse_sphere( module->datas[i] );
	break;
      case TAG_Group:
	create_groups_table( module->datas_count );
	parse_groups( module->datas[i] );
	break;
      case TAG_Ability: parse_abilities( module->datas[i] ); break;
      case TAG_Class: 
	create_classes_table( module->datas_count );
	parse_classes( module->datas[i] );
	break;
      case TAG_Player: 
	fatalError = FALSE;
	parse_player( module->datas[i] );
	fatalError = TRUE;
	break;
      case TAG_Area: parse_area( module->datas[i] ); break;
      case TAG_Config:
	if ( parse_config( module->datas[i] ) )  // once we have found the right config: skip others
	  return data_count;
	break;
      case TAG_AreaState: parse_area_state( module->datas[i] ); break;
      case TAG_Time: parse_time( module->datas[i] ); break;
      case TAG_Hometown:
	create_hometown_table( module->datas_count );
	parse_hometown( module->datas[i] );
	break;
      case TAG_School:
	create_school_table( module->datas_count );
	parse_school( module->datas[i] );
	break;
      case TAG_Super_Race:
	create_super_race_table( module->datas_count );
	parse_super_race( module->datas[i] );
	break;

      default: p_error("Invalid Tag: %s", tagName ); break;
      }

    }
  } catch (ScriptException e) {
    bug("Error while parsing datas. %s", e.msg );
    if ( fatalError ) {
      bug("FATAL error. Bye!");
      exit(-1);
    }
    else
      return -1;
  }

  dump_GC_info();

  return data_count;
}

int parse_datas( FILE *fp ) {
  DATAModule *module = get_data_module( fp );
  return parse_data_module( module );
}
