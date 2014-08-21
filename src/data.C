/*
 *
 * Added by SinaC 2000 to store tables from const.C in files
 * skill_table
 * liquid_table, race_table, pc_race_table, class_table,
 * group_table, material_table, unique items, god_table, command_table,
 * sphere_table, prereqs
 */


#if defined(macintosh)
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
#include "merc.h"
//#include "olc.h"
#include "tables.h"
#include "lookup.h"
#include "bit.h"
#include "classes.h"
#include "language.h"
#include "db.h"
#include "handler.h"
#include "interp.h"
#include "olc_value.h"
#include "names.h"
#include "restriction.h"
#include "affects.h"
#include "dbdata.h"
#include "group.h"
#include "config.h"
#include "data.h"
#include "moons.h"
#include "olc.h"
#include "olc_act.h"
#include "wearoff_affect.h"
#include "update_affect.h"
#include "brew.h"
#include "update.h"
#include "utils.h"
//#include "ability.h"
#include "magic_schools.h"
#include "ability.h"
#include "weather.h"


// Locally used structure to store sphere informations
//  these informations will be used in load_gods (which is called after load_spheres)
// sphere_table is allocated in load_spheres and freed in load_gods
struct ability_info_sphere {
  int sn; // ability number
  int rate; // rate, how hard for classes accepted by god
  int level; // which level player get the skill for classes accepted by god
};
struct sphere_type {
  int gn; // group number associated to sphere
  const char *sphere_name; // sphere name
  int rate; // how many trains to get the sphere
  int nb_abilities; //number of abilities
  ability_info_sphere *ability_list; // list of abilities in sphere
};
sphere_type *sphere_table;
int MAX_SPHERE;


int MAX_RACE;
int MAX_PC_RACE;
int MAX_CLASS;
int MAX_GROUP;
int MAX_POSE;
int MAX_LIQUID;
int MAX_GODS;
int MAX_MATERIAL;
int MAX_COMMANDS;
int MAX_ABILITY;
int MAX_ABILITY_INIT;
int MAX_HOMETOWN;
int MAX_SCHOOL;
int MAX_SUPERRACE;


// Added by SinaC 2000, used in act_wiz2.C, act_wiz3.C and class.C
bool                abilitynotsaved;
bool                clannotsaved;
bool                racenotsaved;
bool                cmdnotsaved;
bool                pcclassnotsaved;
bool                godnotsaved;
bool                liqnotsaved;
bool                materialnotsaved;
bool                groupnotsaved;
bool                spherenotsaved;
bool                schoolnotsaved;
bool                superracenotsaved;

// Update info in ability_table with info from sphere_table and god_table
//    for a group
//      for each skill in sphere
//        unavailable for each class
//        for each class in god.priest
//          get rate from sphere_table and assign it to class
static void update_ability_sphere( int gn, god_data *god ) {
  group_type *gr = &(group_table[gn]);
  // find sphere in sphere_table, we could calculate index but easier to search it in list
  sphere_type *sp = NULL;
  for ( int k = 0; k < MAX_SPHERE; k++ )
    if ( sphere_table[k].gn == gn ) { // found it
      sp = &(sphere_table[k]);
      break;
    }
  if ( sp == NULL ) {
    bug("Can't find group [%s] in sphere_table", gr->name );
    exit(-1);
  }

  // each skill in group
  for ( int k = 0; k < gr->spellnb; k++ ) {
    int sn = ability_lookup(gr->spells[k]);
    ability_type *sk = &(ability_table[sn]);
    ability_info_sphere *spsk = NULL;

    // find skill in sphere, we could calculate index but easier to search it in list
    for ( int l = 0; l < sp->nb_abilities; l++ )
      if ( sp->ability_list[l].sn == sn ) { // found it
	spsk = &(sp->ability_list[l]);
	break;
      }
    if ( spsk == NULL ) {
      bug("Can't find skill [%s] in sphere [%s]", sk->name, gr->name );
      exit(-1);
    }
    // Removed by SinaC 2003, some spheres can be used by a class without getting the right god
    // unavailable for each class
    //    for ( int l = 0; l < MAX_CLASS; l++ ) {
    //      sk->rating[l] = 0;
    //      sk->ability_level[l] = LEVEL_IMMORTAL;
    //    }
    // each class in priest
    //for ( int l = 0; l < god->nb_priest; l++ ) {
    //  int n = class_firstclass( god->priest[l] );
    //  if ( n < 0 || n >= MAX_CLASS ) {
    //	bug("Invalid class %d", god->priest[l] );
    //	exit(-1);
    //}
    int priest = god->priest;
    for ( int n = 0; n < MAX_CLASS; n++ ) {
      if ( !( ( 1 << n ) & priest ) ) // not an available priest
	continue;
      // we must set this rate to a negative value
      if ( spsk->rate > 0 )
	sk->rating[n] = -spsk->rate;
      else
	sk->rating[n] = spsk->rate;
      sk->ability_level[n] = spsk->level;
    }
  }
}

// Update god_rate and class rating in group_table for a sphere of a god
static void update_group_sphere( int gn, int godId ) {
  group_type *gr = &(group_table[gn]);

  // god_rate
  /* Removed by SinaC 2003, spheres can be shared by many gods
     for ( int i = 0; i < MAX_GODS; i++ )
     if ( godId != i )
     gr->god_rate[i] = 0; // follower of other gods can't get the group
  */
  gr->god_rate[godId] = 1;

  // class rating
  god_data *god = &(gods_table[godId]);
  // find sphere in sphere_table, we could calculate index but easier to search it in list
  sphere_type *sp = NULL;
  for ( int k = 0; k < MAX_SPHERE; k++ )
    if ( sphere_table[k].gn == gn ) { // found it
      sp = &(sphere_table[k]);
      break;
    }
  if ( sp == NULL ) {
    bug("Can't find group [%s] in sphere_table", gr->name );
    exit(-1);
  }
  
  // each class in priest
  //  for ( int l = 0; l < god->nb_priest; l++ ) {
  //    int n = class_firstclass( god->priest[l] );
  //    
  //    if ( n < 0 || n >= MAX_CLASS ) {
  //      bug("Invalid class %d", god->priest[l] );
  //      exit(-1);
  //    }
  int priest = god->priest;
  for ( int n = 0; n < MAX_CLASS; n++ ) {
    if ( !( ( 1 << n ) & priest ) ) // not an available priest
      continue;
    gr->rating[n] = sp->rate;
  }
}
void update_group_abilities_with_spheres() {
  // we now update group_table and ability_table with infos read in load_spheres
  log_stringf("Updating groups and abilities with spheres informations.");
    
  // first we allocate god_rate and accept every group for each god
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    group_type *gr = &(group_table[i]);
    if ( ( gr->god_rate = (int*)GC_MALLOC( MAX_GODS * sizeof(int))) == NULL ) {
      bug("Can't allocate memory god_rate (1)");
      exit(-1);
    }
    for ( int j = 0; j < MAX_GODS; j++ )
      gr->god_rate[j] = 1; // each group is available for each god
  }
  // Spheres are unavailable for each gods
  for ( int i = 0; i < MAX_SPHERE; i++ )
    for ( int j = 0; j < MAX_GODS; j++ )
      group_table[sphere_table[i].gn].god_rate[j] = 0;

  // update ability_table and group_table
  //  for each god
  //    for each sphere in god.sphere + minor sphere
  // each god
  for ( int i = 0; i < MAX_GODS; i++ ) {
    god_data *god = &(gods_table[i]);
    // each major sphere
    for ( int j = 0; j < god->nb_major_sphere; j++ ) {
      int gn = god->major_sphere[j];
      update_ability_sphere( gn, god );
      update_group_sphere( gn, i );
    }
    // minor sphere
    if ( god->minor_sphere > 0 ) {
      update_ability_sphere( god->minor_sphere, god );
      update_group_sphere( god->minor_sphere, i );
    }
  }
}

// Added by SinaC 2003, was done in load_group but now we have different files with groups
//  So we must read every group files before checking them
void check_groups() {
  log_stringf("Checking groups...");
  // now, we check every groups to check if the skill/group within are right
  for ( int i = 0; i < MAX_GROUP; i++ )
    for ( int j = 0; j < group_table[i].spellnb; j++ )
      if ( ability_lookup( group_table[i].spells[j] ) < 0
	   && group_lookup( group_table[i].spells[j] ) < 0 )
	bug("Unknown ability/group (%s) in group (%s)",
	    group_table[i].spells[j],group_table[i].name);

  // we also take care about skill/group which are more than once in the group
  for ( int i = 0; i < MAX_GROUP; i++ )
    for ( int j = 0; j < group_table[i].spellnb-1; j++ ) {
      int sn = ability_lookup(group_table[i].spells[j]);
      int gn = group_lookup(group_table[i].spells[j]);
      for ( int k = j+1; k < group_table[i].spellnb; k++ ) {
	int sn2 = ability_lookup(group_table[i].spells[k]);
	int gn2 = group_lookup(group_table[i].spells[k]);
	if ( sn == sn2 
	     && gn == gn2 )
	  bug("Ability/Group (%s) is more than once in the group (%s)",
	      group_table[i].spells[j],group_table[i].name);
      }
    }

  // we also check if we have loaded the base and default group for each classes
  for ( int i = 0; i < MAX_CLASS; i++ ){
    if ( group_lookup( class_table[i].base_group ) < 0 )
      bug("Unknow base group (%s) for class (%s)",
	  class_table[i].base_group, class_table[i].name );
    if ( group_lookup( class_table[i].default_group ) < 0 )
      bug("Unknow default group (%s) for class (%s)",
	  class_table[i].default_group, class_table[i].name );
  }

  // we also check if the groups contains only abilities that class can get
  for ( int num = 0; num < MAX_CLASS; num++ ) // each class
    for ( int i = 0; i < MAX_GROUP; i++ ) // each group
      if ( group_table[i].rating[num] > 0 )  // class can take this group
	for ( int j = 0; j < group_table[i].spellnb; j++ ) {
	  int sn = ability_lookup(group_table[i].spells[j]);
	  int gn = group_lookup(group_table[i].spells[j]);
	  if ( sn <= 0 && gn < 0 )
	    bug("group '%s' contains an unknown ability/group '%s'.",
		group_table[i].name, group_table[i].spells[j] );
	  else if ( sn > 0 && gn < 0 && ability_table[sn].rating[num] <= 0 )
	    //{
	    //for ( int toto = 0; toto < MAX_CLASS; toto++ )
	    //  printf("gr: %s cl: %s rtg: %d   cl: %s  ab: %s [%d]\n\r",
	    //          group_table[i].name, class_table[num].name, group_table[i].rating[num],
	    //	     class_table[toto].name, 
	    //	     ability_table[sn].name, ability_table[sn].rating[toto]);

	    bug("group '%s' shouldn't have ability '%s', because of class [%s].",
		group_table[i].name, ability_table[sn].name, class_table[num].name );
	  //}
	}
  log_stringf("Done.");
}


//*****************************************************************
//****************************** Command **************************

//Command <STRING Command Name> {
//  Position = <STRING Position Name>;
//  Level = <INTEGER Level>;
//  Log = <STRING Log Value (Always,Never,Normal)>
//  Show = <BOOLEAN Show (True/False)>
//}

void new_save_commands() {
  FILE *fp;
  log_stringf("Saving commands");
  fclose( fpReserve );
  if ( ( fp = fopen( COMMANDS_FILE, "w" ) ) == NULL ) {
    bug( "Can't access commands file: [%s]!", COMMANDS_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( int i = 0; i < MAX_COMMANDS; i++ ) {
    cmd_type *cmd = &(cmd_table[i]);
    fprintf(fp,
	    "Command %s {\n"
	    "  Position = %s;\n",
	    quotify(cmd->name,"'"),
	    quotify(flag_string(position_flags, cmd->position),"'") );
    if ( cmd->level != 0 ) fprintf(fp,"  Level = %d;\n", cmd->level );
    if ( cmd->log != LOG_NORMAL ) fprintf( fp, "  Log = %s;\n", quotify(flag_string(log_flags, cmd->log ),"'"));
    if ( !cmd->show ) fprintf( fp, "  Show = %s;\n", cmd_table[i].show?"true":"false");
    fprintf( fp, "}\n\n" );
  }
  cmdnotsaved = FALSE;
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_load_commands() {
  FILE *fp;

  log_stringf("Reading commands");

  fclose(fpReserve);
  if (!(fp = fopen ( COMMANDS_FILE, "r"))){
    bug("Could not open file [%s] in order to load commands.", COMMANDS_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );
  log_stringf(" %d commands found.", MAX_COMMANDS );

  for ( int j = 0; cmd_table_init[j].name[0] != '\0'; j++ ) {
    bool found = FALSE;
    for ( int i = 0; i < MAX_COMMANDS; i++ )
      if ( !str_cmp(cmd_table[i].name, cmd_table_init[j].name ) ) {
	found = TRUE;
	break;
      }
    if ( !found )
      bug("Error in command loading: command missing in file: [%s]", 
	  cmd_table_init[j].name );
  }
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  cmdnotsaved = FALSE;
}

void create_command_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_COMMANDS = count;
  if ( ( cmd_table = (cmd_type*) GC_MALLOC( sizeof(cmd_type)*MAX_COMMANDS ) ) == NULL )
    p_error("Can't allocate memory cmd_table");
  done = TRUE;
}

void parse_command( DATAData *comm ) {
  static int index = 0;

  cmd_type *command = &(cmd_table[index]);
  command->name = str_dup( comm->value->eval().asStr() );
  command->level = 0; // default values
  command->log = LOG_NORMAL;
  command->show = TRUE;

  if ( DATA_VERBOSE > 0 ) {
    printf("Command name: %s\n\r", command->name );
  }

  int k = -1;
  for ( int j = 0; cmd_table_init[j].name[0] != '\0'; j++ )
    if ( !str_cmp(command->name, cmd_table_init[j].name ) )
      k = j;
  if ( k == -1 )
    p_error("Unknow command %s, no found in cmd_table_init", command->name );

  // fields
  for ( int fieldCount = 0; fieldCount < comm->fields_count; fieldCount++ ) {
    DATAData *field = comm->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {

      // Minimum level
    case TAG_Level:
      command->level = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Level: %d\n\r", command->level );
      }
      break;

      // Show command in command list ?
    case TAG_Show:
      command->show = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Show: %s\n\r", command->show?"true":"false" );
      }
      break;

      // Minimum position
    case TAG_Position: {
      const char *s = field->value->eval().asStr();
      if ( ( command->position = flag_value_complete_init( position_flags, s ) ) == NO_FLAG ) {
	bug("Invalid position (%s) for cmd [%s]", s, command->name );
	command->position = POS_DEAD;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  Position: %s\n\r", flag_string( position_flags, command->position ) );
      }
      break;
    }
      // Log: Always, Normal, Never
    case TAG_Log: {
      const char *s = field->value->eval().asStr();
      if ( ( command->log = flag_value_init( log_flags, s ) ) == NO_FLAG ) {
	bug("Invalid log (%s) for cmd [%s]", s, command->name );
	command->log = LOG_NORMAL;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  Log: %s\n\r", flag_string( log_flags, command->log ) );
      }
      break;
    }
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  command->do_fun = cmd_table_init[k].do_fun;
  index++;
}


//****************************************************************
//****************************** Unique **************************

//Unique <INTEGER Object Vnum> {
//  Room = <INTEGER Room Vnum>;
//}

void new_save_unique_items() {
  FILE *fp;

  log_string("Saving unique items");

  fclose( fpReserve );
  if ( ( fp = fopen( UNIQUE_ITEM_FILE, "w" ) ) == NULL ){
    bug( "Can't access unique item file: [%s]!", UNIQUE_ITEM_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj->next ) {
    if ( IS_SET( obj->extra_flags, ITEM_UNIQUE ) ) {
      if ( obj->carried_by != NULL ) {
	ROOM_INDEX_DATA *room;
	CHAR_DATA *ch = obj->carried_by;
	int vnum;
	obj_from_char( obj );
	// select the new location
	vnum = ch->in_room?ch->in_room->vnum:0;
	if( vnum == 0 || (room = get_room_index(vnum))==NULL) {
	  room = get_room_index(ROOM_VNUM_LIMBO);
	  bug("save_unique_items: %s carrying [%s] (vnum %d) is in a invalid room (vnum %d)",
	      NAME(ch), obj->name, obj->pIndexData->vnum, vnum );
	}
	obj_to_room( obj, room );
      }
      log_stringf("Item '%s' (vnum %d) in room %d",
		  obj->name, obj->pIndexData->vnum,
		  obj->in_room->vnum );
      fprintf( fp, 
	       "Unique %d {\n"
	       "  Room = %d;\n"
	       "}\n\n",
	       obj->pIndexData->vnum, obj->in_room->vnum );
    }
  }
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_load_unique_items() {
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  log_string("Reading unique items");
  
  fclose( fpReserve );
  if ( ( fp = fopen( UNIQUE_ITEM_FILE, "r" ) ) == NULL ){
    bug( "Can't access unique item file: [%s]!", UNIQUE_ITEM_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  int count = parse_datas( fp );
  log_stringf(" %d unique items found.", count );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void parse_unique( DATAData *unique ) {
  // tag:    Unique
  // value:  <Obj Vnum>
  // fields: Room = <Room vnum>;
  OBJ_INDEX_DATA *pObj;
  ROOM_INDEX_DATA *pRoom = NULL;
  OBJ_DATA *obj;
  int roomVnum = -1;

  // value
  const int objVnum = unique->value->eval().asInt();
  if ((pObj = get_obj_index( objVnum )) == NULL ) {
    bug("Invalid object vnum: %d", objVnum );
    return;
  }

  if ( DATA_VERBOSE > 0 ) {
    printf("objVnum: %d\n\r", objVnum );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < unique->fields_count; fieldCount++ ) {
    DATAData *field = unique->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {

      // Room Vnum
    case TAG_Room:
      roomVnum = field->value->eval().asInt();
      if ((pRoom = get_room_index( roomVnum )) == NULL ) {
	bug("Invalid room vnum: %d", roomVnum );
	return;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("roomVnum: %d\n\r", roomVnum );
      }
      break;

    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  if ( pObj == NULL || pRoom == NULL ) {
    bug("Error while loading unique item: %d, missing room.", objVnum );
    return;
  }

  obj = create_object( pObj, 0 );
  obj_to_room( obj, pRoom );
  log_stringf("Item '%s' (vnum %d) in room %d",
	      obj->name, objVnum, roomVnum );
  if ( DATA_VERBOSE > 1 ) {
    printf("Unique (%s) (vnum: %d) (room: %d)\n\r", obj->name, objVnum, roomVnum );
  }
}

//*************************************************************************
//******************************* Material ********************************

//Material <STRING Material Name> {
//  Color = <STRING Color>;
//  Immunities = <LIST OF STRING> ( <STRING Immunities1>, <STRING Immunities2>, ... );
//  Resistances = <LIST OF STRING> ( <STRING Resistances1>, <STRING Resistances2>, ... );
//  Vulnerabilities = <LIST OF STRING> ( <STRING Vulnerabilities1>, <STRING Vulnerabilities2>, ... );
//  Metal = <BOOLEAN IsMetal (True/False)>;
//}
void new_save_material() {
  log_string("Saving material table");
  
  fclose( fpReserve );
  FILE *fp;
  if ( ( fp = fopen( MATERIAL_FILE, "w" ) ) == NULL ) {
    bug( "Can't access material file: [%s]!", MATERIAL_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( int i = 0; i < MAX_MATERIAL; i++ ) {
    material_type *mat = &(material_table[i]);
    char immBuf[MAX_STRING_LENGTH];
    char resBuf[MAX_STRING_LENGTH];
    char vulnBuf[MAX_STRING_LENGTH];
    fprintf(fp,
	    "Material %s {\n"
	    "  Color = %s;\n",
	    quotify(mat->name,"'"), quotify(mat->color,"'") );
    if ( mat->imm != 0 )
      fprintf(fp, "  Immunities = (%s);\n",list_flag_string( mat->imm, irv_flags, "'", ", " ));
    if ( mat->res != 0 )
      fprintf(fp, "  Resistances = (%s);\n",list_flag_string( mat->res, irv_flags, "'", ", " ));
    if ( mat->vuln != 0 )
      fprintf(fp, "  Vulnerabilities = (%s);\n",list_flag_string( mat->vuln, irv_flags, "'", ", " ));
    fprintf( fp, 
	     "  Metal = %s;\n"
	     "}\n\n",
	     material_table[i].metallic?"true":"false");
  }
}

void new_load_material() {
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  log_string("Reading material table");
  
  fclose( fpReserve );
  if ( ( fp = fopen( MATERIAL_FILE, "r" ) ) == NULL ) {
    bug( "Can't access material file: [%s]!", MATERIAL_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  parse_datas( fp );
  log_stringf(" %d materials found.", MAX_MATERIAL );

  // create flag table
  if ((material_flags = (flag_type *)GC_MALLOC(sizeof(flag_type)*(MAX_MATERIAL+1))) == NULL ) {
    bug("Can't allocate memory for material_flags");
    exit(-1);
  }
  material_flags[MAX_MATERIAL].name = NULL;
  for ( int i = 0; i < MAX_MATERIAL; i++ ) {
    material_flags[i].name = str_dup( material_table[i].name );
    material_flags[i].bit = i;
    material_flags[i].settable = TRUE;
  }
  // Builders can't set material to value  0  ~  <Not Entered>
  material_flags[0].settable = FALSE;
  
  //  update_flag_stat_table( "material", material_flags );
  //  update_flag_stat_table_init( "material", material_flags );
  //  update_help_table( "material", material_flags );

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

void create_material_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_MATERIAL = count;
  if (( material_table = (material_type*)GC_MALLOC(sizeof(material_type)*MAX_MATERIAL))==NULL)
    p_error("Can't allocate memory material_table");
  done = TRUE;
}

void parse_material( DATAData *mat ) {
  static int index = 0;
  // tag:     Material
  // value:   <Material Name>
  // fields:  Color = <Color Name>;
  // fields:  Immunities = <List of string (immunities)>;
  // fields:  Resistances = <List of string (resistances)>;
  // fields:  Vulnerabilities = <List of string (vulnerabilities)>;
  // fields:  Metal = <Boolean (Is this material is metallic?)>;
  material_type *material = &(material_table[index]);
  // value
  material->name = str_dup( mat->value->eval().asStr() );
  material->imm = material->res = material->vuln = 0; // non mandatory information

  // fields
  for ( int fieldCount = 0; fieldCount < mat->fields_count; fieldCount++ ) {
    DATAData *field = mat->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {

      // Color
    case TAG_Color:
      material->color = str_dup( field->value->eval().asStr() );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Color: %s\n\r", material->color );
      }
      break;
      // Immunities
    case TAG_Immunities:
      material->imm = list_flag_value( field->value, irv_flags );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Imm: %ld\n\r", material->imm );
      }
      break;
      // Resistances
    case TAG_Resistances:
      material->res = list_flag_value( field->value, irv_flags );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Res: %ld\n\r", material->res );
      }
      break;
      // Vulnerabilities
    case TAG_Vulnerabilities:
      material->vuln = list_flag_value( field->value, irv_flags );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Vuln: %ld\n\r", material->vuln );
      }
      break;

    case TAG_Metal:
      material->metallic = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Metallic: %s\n\r", material->metallic?"true":"false");
      }
      break;

    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  if ( DATA_VERBOSE > 1 ) {
    printf("Material: %s  (color: %s) (metallic: %s) Flag: (%ld/%ld/%ld)\n",
	   material->name, material->color, material->metallic?"true":"false",
	   material->imm, material->res, material->vuln );
  }

  index++;
}

//*************************************************************************
//******************************** Liquid *********************************
//Liquid <STRING Liquid Name> {
//  Color = <STRING Color>;
//  Drunk = <INTEGER Drunk>;
//  Full = <INTEGER Full>;
//  Thirst = <INTEGER Thirst>;
//  Food = <INTEGER Food>;
//  Ssize = <INTEGER Ssize>;
//}

void new_save_liquid() {
  FILE *fp;
  log_string("Saving liquid table");
  fclose( fpReserve );
  if ( ( fp = fopen( LIQ_FILE, "w" ) ) == NULL ){
    bug( "Can't access liquid file: [%s]!", LIQ_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( int i = 0; i < MAX_LIQUID; i++ )
    fprintf(fp,
	    "Liquid %s {\n"
	    "  Color = %s;\n"
	    "  Drunk = %d;\n"
	    "  Full = %d;\n"
	    "  Thirst = %d;\n"
	    "  Food = %d;\n"
	    "  Ssize = %d;\n"
	    "}\n\n",
	    quotify(liq_table[i].liq_name,"'"),
	    quotify(liq_table[i].liq_color,"'"),
	    liq_table[i].liq_affect[0],
	    liq_table[i].liq_affect[1],
	    liq_table[i].liq_affect[2],
	    liq_table[i].liq_affect[3],
	    liq_table[i].liq_affect[4]);
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_load_liquid() {
  FILE *fp;
  int i;
  
  log_string("Reading liquid table");
  
  fclose( fpReserve );
  
  if ( ( fp = fopen( LIQ_FILE, "r" ) ) == NULL ){
    bug( "Can't access liquid file: [%s]!", LIQ_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );
  log_stringf(" %d liquids found.", MAX_LIQUID );

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

void create_liquid_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_LIQUID = count;
  if ( ( liq_table = (liq_type *)GC_MALLOC(sizeof(liq_type)*MAX_LIQUID) ) == NULL )
    p_error("Can't allocate memory for liquid_table");
  done = TRUE;
}

//Liquid <STRING Liquid Name> {
//  Color = <STRING Color>;
//  Drunk = <INTEGER Drunk>;
//  Full = <INTEGER Full>;
//  Thirst = <INTEGER Thirst>;
//  Food = <INTEGER Food>;
//  Ssize = <INTEGER Ssize>;
//}

void parse_liquid( DATAData *liq ) {
  static int index = 0;
  // tag:    Liquid
  // value:  <Liquid Name>
  // fields: Color = <Color name>;
  // fields: Drunk = <Integer Drunk value>;
  // fields: Full = <Integer Full value>;
  // fields: Thirst = <Integer Thirst value>;
  // fields: Food = <Integer Food value>;
  // fields: Ssize = <Integer ????>;

  liq_type *liquid = &(liq_table[index]);
  liquid->liq_name = str_dup( liq->value->eval().asStr() );
  if ( DATA_VERBOSE > 0 ) {
    printf(" Liquid: %s\n\r", liquid->liq_name );
  }
  
  // fields
  for ( int fieldCount = 0; fieldCount < liq->fields_count; fieldCount++ ) {
    DATAData *field = liq->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
      // Color
    case TAG_Color:
      liquid->liq_color = str_dup( field->value->eval().asStr() );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Color: %s\n\r", liquid->liq_color );
      }
      break;
      // Drunk
    case TAG_Drunk:
      liquid->liq_affect[0] = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Drunk: %d\n\r", liquid->liq_affect[0] );
      }
      break;
      // Full
    case TAG_Full:
      liquid->liq_affect[1] = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Full: %d\n\r", liquid->liq_affect[1] );
      }
      break;
      // Thirst
    case TAG_Thirst:
      liquid->liq_affect[2] = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Thirst: %d\n\r", liquid->liq_affect[2] );
      }
      break;
      // Food
    case TAG_Food:
      liquid->liq_affect[3] = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Food: %d\n\r", liquid->liq_affect[3] );
      }
      break;
      // Ssize
    case TAG_Ssize:
      liquid->liq_affect[4] = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  Ssize: %d\n\r", liquid->liq_affect[4] );
      }
      break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  if ( DATA_VERBOSE > 1 ) {
    printf(" Liquid: %s (color: %s) (value: %4d %4d %4d %4d %4d)\n\r",
	   liquid->liq_name, liquid->liq_color, 
	   liquid->liq_affect[0],liquid->liq_affect[1],liquid->liq_affect[2],
	   liquid->liq_affect[3],liquid->liq_affect[4]);
  }

  index++;
}

//****************************************************************************
//********************************** Gods ************************************
//God <STRING God Name> {
//  Title = <STRING Title>;
//  Story = <STRING Story>;
//  Minor = <STRING Minor Sphere>;
//  Major = <LIST OF STRING> ( <STRING Sphere1>, <STRING Sphere2>, ... );
//  Priest = <LIST OF STRING> ( <STRING Class1>, <STRING Class2>, ... );
//  Alignment = <LIST OF STRING> ( <STRING Align1>, <STRING Align2>, ... );
//  Races = <LIST OF STRING> ( <STRING Race1>, <STRING Race2>, ... );
//  Classes = <LIST OF STRING> ( <STRING Class1>, <STRING Class2>, ... );
//}
void new_save_gods() {
  FILE *fp;
  log_string("Saving gods table");

  fclose( fpReserve );
  if ( ( fp = fopen( GODS_FILE, "w" ) ) == NULL ) {
    bug( "Can't access gods file: [%s]!", GODS_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_GODS; i++ ) {
    god_data *god = &(gods_table[i]);
    fprintf( fp,
	     "God %s {\n"
	     "  Title = %s;\n"
	     "  Story = %s;\n"
	     "  Minor = %s;\n"
	     "  Major = (%s);\n"
	     "  Priest = (%s);\n"
	     "  Alignment = (%s);\n"
	     "  Races = (%s);\n"
	     "  Classes = (%s);\n"
	     "  AcceptWildMagic = %s;\n"
	     "}\n\n",
	     quotify(god->name,"'"), quotify(god->title,"'"), quotify(god->story,"\""),
	     god->minor_sphere == -1?"'none'":quotify(group_table[god->minor_sphere].name,"'"),
	     create_groups_list( god->nb_major_sphere, god->major_sphere ),
	     //create_classes_list( god->nb_priest, god->priest ),
	     list_flag_string( god->priest, classes_flags ),
	     create_alignment_list( god->nb_allowed_align, god->allowed_align ),
	     create_races_list( god->nb_allowed_race, god->allowed_race), 
	     //create_classes_list( god->nb_allowed_class, god->allowed_class) );
	     list_flag_string( god->allowed_class, classes_flags ),
	     god->acceptWildMagic?"true":"false");
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_load_gods() {
  FILE *fp;

  log_string("Reading gods table");
  
  fclose( fpReserve );
  if ( ( fp = fopen( GODS_FILE, "r" ) ) == NULL ) {
    bug( "Can't access gods file: [%s]!", GODS_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );
  log_stringf(" %d gods found.", MAX_GODS );

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void create_god_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_GODS = count;
  if ( ( gods_table = (god_data*) GC_MALLOC( MAX_GODS * sizeof(god_data) ) ) == NULL )
    p_error("Can't allocate memory gods_table (1)");
  done = TRUE;
}

void parse_god_major( DATAData *field, god_data *god ) {
  ValueList *list = field->value->eval().asList();

  god->nb_major_sphere = list->size;
  if ( god->nb_major_sphere == 0 ) {
    log_stringf("No major sphere for god %s", god->name );
    god->major_sphere = NULL;
    return;
  }
  if ( ( god->major_sphere = (int*) GC_MALLOC( god->nb_major_sphere* sizeof(int) ) ) == NULL )
    p_error("Can't allocate memory major_sphere gods_table (1)");

  for ( int i = 0; i < list->size; i++ ) {
    const char *buf = list->elems[i].asStr();
    if ( ( god->major_sphere[i] = group_lookup(buf) ) <= 0 )
      p_error("Invalid god major sphere (%s) for god (%s)", buf, god->name );
  }

  list->explicit_free();
}
//void parse_god_priest( DATAData *field, god_data *god ) {
//  ValueList *list = field->value->eval().asList();
//
//  god->nb_priest = list->size;
//  if ( god->nb_priest == 0 )
//    p_error("No priest for god %s", god->name );
//  if ( ( god->priest = (int*) GC_MALLOC( god->nb_priest * sizeof(int) ) ) == NULL )
//    p_error("Can't allocate memory priest gods_table (1)");
//
//  for ( int i = 0; i < list->size; i++ ) {
//    const char *buf = list->elems[i].asStr();
//    if ( ( god->priest[i] = 1<<class_lookup(buf) ) <= 0 )
//      p_error("Invalid %d° priest (%s) for god (%s)", i+1, buf, god->name );
//  }
//}
void parse_god_align( DATAData *field, god_data *god ) {
  ValueList *list = field->value->eval().asList();

  god->nb_allowed_align = list->size;
  if ( god->nb_allowed_align == 0 )
    p_error("No alignment for god %s", god->name );
  if ( ( god->allowed_align = (align_info*) GC_MALLOC( god->nb_allowed_align * sizeof(align_info) ) ) == NULL )
    p_error("Can't allocate memory allowed_align gods_table (1)");
  
  for ( int i = 0; i < list->size; i++ ) {
    const char *buf = list->elems[i].asStr();
    if ( convert_etho_align_string_to_align_info( buf, god->allowed_align[i] ) == -1 )
      p_error("Invalid alignment (%s) for god (%s)", buf, god->name );
  }

  list->explicit_free();
}
void parse_god_races( DATAData *field, god_data *god ) {
  ValueList *list = field->value->eval().asList();

  god->nb_allowed_race = list->size;
  if ( ( god->allowed_race = (int*) GC_MALLOC( god->nb_allowed_race * sizeof(int) ) ) == NULL )
    p_error("Can't allocate memory allowed_race gods_table (1)");
  if ( god->nb_allowed_race == 0 )
    p_error("No races for god %s", god->name );
  
  for ( int i = 0; i < list->size; i++ ) {
    const char *buf = list->elems[i].asStr();
    if ( ( god->allowed_race[i] = race_lookup(buf,TRUE) ) < 0 )
      p_error("Invalid %d° race (%s) for god (%s)", i+1, buf, god->name );
  }

  list->explicit_free();
}
//void parse_god_classes( DATAData *field, god_data *god ) {
//  ValueList *list = field->value->eval().asList();
//
//  god->nb_allowed_class = list->size;
//  if ( god->nb_allowed_class == 0 )
//    p_error("No classes for god %s", god->name );
//  if ( ( god->allowed_class = (int*) GC_MALLOC( god->nb_allowed_class * sizeof(int) ) ) == NULL )
//    p_error("Can't allocate memory allowed_class gods_table (1)");
//
//  for ( int i = 0; i < list->size; i++ ) {
//    const char *buf = list->elems[i].asStr();
//    if ( ( god->allowed_class[i] = 1<<class_lookup(buf) ) <= 0 )
//      p_error("Invalid %d° class (%s) for god (%s)", i+1, buf, god->name );
//  }
//}
void parse_god( DATAData *godD ) {
  static int index = 0;
  // tag: God
  // value: <God Name>
  // fields: Title = <STRING Title>;
  // fields: Story = <STRING Story>;
  // fields: Minor = <STRING Minor Sphere>;
  // fields: Major = <LIST OF STRING> ( <STRING Sphere1>, <STRING Sphere2>, ... );
  // fields: Priest = <LIST OF STRING> ( <STRING Class1>, <STRING Class2>, ... );
  // fields: Alignment = <LIST OF STRING> ( <STRING Align1>, <STRING Align2>, ... );
  // fields: Races = <LIST OF STRING> ( <STRING Race1>, <STRING Race2>, ... );
  // fields: Classes = <LIST OF STRING> ( <STRING Class1>, <STRING Class2>, ... );
  god_data *god = &(gods_table[index]);
  god->name = str_dup( godD->value->eval().asStr() );
  god->nb_major_sphere = god->nb_allowed_align = god->nb_allowed_race = 0;
  god->minor_sphere = -1;
  god->acceptWildMagic = TRUE; // true is default value

  if ( DATA_VERBOSE > 0 ) {
    printf("God %s\n\r", god->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < godD->fields_count; fieldCount++ ) {
    DATAData *field = godD->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
      // Title
    case TAG_Title:
      god->title = str_dup( field->value->eval().asStr() );
      if ( DATA_VERBOSE > 2 ) {
	printf("  title: %s\n\r", god->title );
      }
      break;
      // Story
    case TAG_Story:
      god->story = str_dup( field->value->eval().asStr() );
      if ( DATA_VERBOSE > 2 ) {
	printf("  story: %s\n\r", god->story );
      }
      break;
      // Minor
    case TAG_Minor: {
      const char *s = field->value->eval().asStr();
      if ( !str_cmp( s, "none" ) ) {
	log_stringf("No minor sphere for god %s", god->name );
	god->minor_sphere = -1;
      }
      else
	if ( ( god->minor_sphere = group_lookup( s ) ) < 0 )
	  p_error("Unknown Minor sphere: %s for god %s", s, god->name );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Minor sphere: %s", god->minor_sphere == -1 ? "none":group_table[god->minor_sphere].name );
      }
      break;
    }
      // Major
    case TAG_Major: parse_god_major( field, god ); break;
      // Priest
      //case TAG_Priest: parse_god_priest( field, god ); break;
    case TAG_Priest:
      if ( ( god->priest = list_flag_value( field->value, classes_flags ) ) == NO_FLAG )
	p_error("Invalid priest for god %s", god->name );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Priest: %s\n\r", flag_string( classes_flags, god->priest ) );
      }
      break;
      // Alignment
    case TAG_Alignment: parse_god_align( field, god ); break;
      // Races
    case TAG_Races: parse_god_races( field, god ); break;
      // Classes
      //case TAG_Classes: parse_god_classes( field, god ); break;
    case TAG_Classes:
      if ( ( god->allowed_class = list_flag_value( field->value, classes_flags ) ) == NO_FLAG )
	p_error("Invalid class for god %s", god->name );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Classes: %s\n\r", flag_string( classes_flags, god->allowed_class ) );
      }
      break;
    case TAG_AcceptWildMagic: god->acceptWildMagic = field->value->eval().asInt(); break; // SinaC 2003
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
  
  if ( god->nb_major_sphere == 0 )
    p_error("Missing major sphere information for god %s", god->name );
  if ( god->priest == 0 )
    p_error("Missing priest information for god %s", god->name );
  if ( god->nb_allowed_align == 0 )
    p_error("Missing alignment information for god %s", god->name );
  if ( god->nb_allowed_race == 0 )
    p_error("Missing races information for god %s", god->name );
  if ( god->allowed_class == 0 )
    p_error("Missing classes information for god %s", god->name );

  // Added by SinaC 2003: Check if priests are god available classes
//  for ( int j = 0; j < god->nb_priest; j++ ) {
//    bool found = FALSE;
//    for ( int k = 0; k < god->nb_allowed_class; k++ )
//      if ( god->priest[j] == god->allowed_class[k] ) {
//	found = TRUE;
//	break;
//      }
//    if ( !found )
//      p_error("Class getting sphere: %s is not an available class for god %s",
//	      class_name(god->priest[j]), god->name);
//  }
  int priest = god->priest;
  int allowed_class = god->allowed_class;
  for ( int j = 0; j < MAX_CLASS; j++ )
    if ( ( 1 << j ) & priest )  // for each priest
      if ( !( ( 1 << j ) & allowed_class ) ) // priest not found in classes
	p_error("Priest %s is not an available class for god %s.",
		class_table[j].name, god->name);
  index++;
}

//**********************************************************************************************
//************************************ PC/NPC RACES ********************************************
//Race <STRING Race name> {
//  Act = <LIST OF STRING> ( <STRING Act>, ...);
//  Affect = <LIST OF STRING> ( <STRING Affect>, ...);
//  Affect2 = <LIST OF STRING> ( <STRING Affect2>, ...);
//  Offensive = <LIST OF STRING> ( <STRING Offensive>, ...);
//  Immunities = <LIST OF STRING> ( <STRING Immunities1>, <STRING Immunities2>, ... );
//  Resistances = <LIST OF STRING> ( <STRING Resistances1>, <STRING Resistances2>, ... );
//  Vulnerabilities = <LIST OF STRING> ( <STRING Vulnerabilities1>, <STRING Vulnerabilities2>, ... );
//  Forms = <LIST OF STRING> ( <STRING Form1>, <STRING Form2>, ...);
//  Parts = <LIST OF STRING> ( <STRING Part1>, <STRING Part2>, ...);
//}
void new_save_races() {
  FILE *fp;
  
  log_string("Saving race table");

  fclose( fpReserve );
  if ( ( fp = fopen( RACE_FILE, "w" ) ) == NULL ){
    bug( "Can't access race file: [%s]!", RACE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_RACE; i++ ) {
    race_type *race = &(race_table[i]);
    fprintf(fp,
	    "Race %s {\n"
	    "  Act = (%s);\n"
	    "  Affect = (%s);\n"
	    "  Affect2 = (%s);\n"
	    "  Offensive = (%s);\n"
	    "  Immunities = (%s);\n"
	    "  Resistances = (%s);\n"
	    "  Vulnerabilities = (%s);\n"
	    "  Forms = (%s);\n"
	    "  Parts = (%s);\n"
	    "  Size = %s;\n"
	    "}\n\n",
	    quotify(race->name,"'"),
	    list_flag_string( race->act, act_flags, "'", ", " ),
	    list_flag_string( race->aff, affect_flags, "'", ", " ),
	    list_flag_string( race->aff2, affect2_flags, "'", ", " ),
	    list_flag_string( race->off, off_flags, "'", ", " ),
	    list_flag_string( race->imm, irv_flags, "'", ", " ),
	    list_flag_string( race->res, irv_flags, "'", ", " ),
	    list_flag_string( race->vuln, irv_flags, "'", ", " ),
	    list_flag_string( race->form, form_flags, "'", ", " ),
	    list_flag_string( race->parts, part_flags, "'", ", " ),
	    quotify(flag_string(size_flags, race->size))
	    );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  racenotsaved = FALSE;
}

void new_load_races() {
  FILE *fp;

  log_string("Reading race table");

  fclose( fpReserve );
  if ( ( fp = fopen( RACE_FILE, "r" ) ) == NULL ){
    bug( "Can't access race file: [%s]!", RACE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  parse_datas(fp);
  log_stringf(" %d races found.", MAX_RACE );

  for ( int i = 0; i < MAX_RACE; i++ ) {
    race_type *race = &(race_table[i]);
    if ( !IS_SET( race->form, FORM_ANIMAL ) ) {
      if ( IS_SET( race->parts, PART_ARMS ) && !IS_SET( race->parts, PART_HANDS ) )
	log_stringf("race [%s] is not animal, have arms but no hands", race->name );
      if ( IS_SET( race->parts, PART_LEGS ) && !IS_SET( race->parts, PART_FEET ) )
	log_stringf("race [%s] is not animal, have legs but no feet", race->name );
      if ( IS_SET( race->parts, PART_HANDS ) && !IS_SET( race->parts, PART_FINGERS ) )
	log_stringf("race [%s] is not animal, have hands but no fingers", race->name );
      if ( IS_SET( race->parts, PART_HEAD ) && !IS_SET( race->parts, PART_EAR ) )
	log_stringf("race [%s] is not animal, have head but no ear", race->name );
      if ( IS_SET( race->parts, PART_HEAD ) && !IS_SET( race->parts, PART_EYE ) )
	log_stringf("race [%s] is not animal, have head but no eye", race->name );
      if ( ( IS_SET( race->parts, PART_ARMS )
	     || IS_SET( race->parts, PART_LEGS )
	     || IS_SET( race->parts, PART_HEAD) )
	   && !IS_SET( race->parts, PART_BODY ) )
	log_stringf("race [%s] is not animal, have arms/legs/head but no body", race->name );
    }
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  racenotsaved = FALSE;
}

void create_races_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_RACE = count;
  if (( race_table = (race_type *) GC_MALLOC( sizeof( race_type )*MAX_RACE) ) == NULL ){
    bug("Can't allocate memory for race_table");
    exit(-1);
  }

  // +1 for the dummy race in races_flags
  if (( races_flags = (flag_type *)GC_MALLOC( sizeof(flag_type)*(MAX_RACE+1))) == NULL ){
    bug( "Can't allocate memory for races_flags" );
    exit(-1);
  }
  for ( int i = 0; i < MAX_RACE+1; i++ )
    races_flags[i].name = NULL;
  done = TRUE;
}

void parse_race( DATAData *raceD ) {
  static int index = 0;
  race_type *race = &(race_table[index]);
  race->name = str_dup(raceD->value->eval().asStr());
  race->pc_race = FALSE;
  
  races_flags[index].name = str_dup( race->name );
  races_flags[index].bit = index;
  races_flags[index].settable = TRUE;
  
  if ( DATA_VERBOSE > 0 ) {
    printf(" Race %s\n\r", race->name );
  }
  // fields
  for ( int fieldCount = 0; fieldCount < raceD->fields_count; fieldCount++ ) {
    DATAData *field = raceD->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Act:
      race->act = list_flag_value( field->value, act_flags );
      if ( race->act == NO_FLAG ) {
	bug("Invalid act flags for race [%s], assuming none", race->name );
	race->act = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  act: %s\n\r", flag_string_init( act_flags, race->act ) );
      }
      break;
    case TAG_Affect:
      race->aff = list_flag_value( field->value, affect_flags );
      if ( race->aff == NO_FLAG ) {
	bug("Invalid aff flags for race [%s], assuming none", race->name );
	race->aff = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  aff: %s\n\r", flag_string_init( affect_flags, race->aff ) );
      }
      break;
    case TAG_Affect2:
      race->aff2 = list_flag_value( field->value, affect2_flags );
      if ( race->aff2 == NO_FLAG ) {
	bug("Invalid aff2 flags for race [%s], assuming none", race->name );
	race->aff2 = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  aff2: %s\n\r", flag_string_init( affect2_flags, race->aff2 ) );
      }
      break;
    case TAG_Offensive:
      race->off = list_flag_value( field->value, off_flags );
      if ( race->off == NO_FLAG ) {
	bug("Invalid off flags for race [%s], assuming none", race->name );
	race->off = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  off: %s\n\r", flag_string_init( off_flags, race->off ) );
      }
      break;
    case TAG_Immunities:
      race->imm = list_flag_value( field->value, irv_flags );
      if ( race->imm == NO_FLAG ) {
	bug("Invalid imm flags for race [%s], assuming none", race->name );
	race->imm = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  imm: %s\n\r", flag_string_init( irv_flags, race->imm ) );
      }
      break;
    case TAG_Resistances:
      race->res = list_flag_value( field->value, irv_flags );
      if ( race->res == NO_FLAG ) {
	bug("Invalid res flags for race [%s], assuming none", race->name );
	race->res = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  res: %s\n\r", flag_string_init( irv_flags, race->res ) );
      }
      break;
    case TAG_Vulnerabilities:
      race->vuln = list_flag_value( field->value, irv_flags );
      if ( race->vuln == NO_FLAG ) {
	bug("Invalid vuln flags for race [%s], assuming none", race->name );
	race->vuln = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  vuln: %s\n\r", flag_string_init( irv_flags, race->vuln ) );
      }
      break;
    case TAG_Forms:
      race->form = list_flag_value( field->value, form_flags );
      if ( race->form == NO_FLAG ) {
	bug("Invalid forms flags for race [%s], assuming none", race->name );
	race->form = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  forms: %s\n\r", flag_string_init( form_flags, race->form ) );
      }
      break;
    case TAG_Parts:
      race->parts = list_flag_value( field->value, part_flags );
      if ( race->parts == NO_FLAG ) {
	bug("Invalid parts flags for race [%s], assuming none", race->name );
	race->parts = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  parts: %s\n\r", flag_string_init( part_flags, race->parts ) );
      }
      break;
    case TAG_Size: {
      const char *s = field->value->eval().asStr();
      //race->size = size_lookup( s );
      race->size = flag_value_complete_init( size_flags, s );
      if ( race->size == NO_FLAG ) {
	bug("Invalid size (%s) for race: [%s], assuming [%s]", s, race->name, DEFAULT_SIZE_NAME );
	//race->size = size_lookup( "medium" );
	race->size = DEFAULT_SIZE;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  size: %s\n\r", size_table[race->size].name );
      }
      break;
    }
      // Other
    default: p_error("Invalid Tag: [%s]", tagName ); break;
    }
  }
  
  index++;
}


/*
 *
 *  pc race table
 *
 */
/*
  const	struct	pc_race_type	pc_race_table	[]	=
  {
  "race name", 	"short name", 	points,	{ class multipliers },
  { "bonus skills" },
  { base stats },		{ max stats },		size 
  },
*/
//PCRace <STRING PC Race Name> {
//  WhoName = <STRING Who/Whois name>;
//  Experience = <INTEGER Base experience per level>;
//  Choosable = <BOOLEAN Can race be picked at create (True/False)>;
//  Size = <STRING Race size (small, medium, ...)>;
//  Attributes = <LIST OF LIST> ( ( <STRING Attribute Name>, <INTEGER Start value>, <INTEGER Max value> ) );
//  Language = <STRING Race language>;
//  Abilities = <LIST OF STRING> ( <STRING Ability name>, ...);
//  Alignment = <LIST OF STRING> ( <STRING Align1>, <STRING Align2>, ...);
//  Classes = <LIST OF STRING> ( <STRING Class1>, <STRING Class2>, ...);
//}
void new_save_pcraces() {
  FILE *fp;

  log_string("Saving pc race table");

  fclose( fpReserve );
  if ( ( fp = fopen( PCRACE_FILE, "w" ) ) == NULL ){
    bug( "Can't access race file: [%s]!", PCRACE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  for ( int i = 0; i < MAX_PC_RACE; i++ ){
    char raceAttributes[MAX_STRING_LENGTH];
    pc_race_type *race = &(pc_race_table[i]);
    
    raceAttributes[0] = '\0';
    for ( int j = 0; j < MAX_STATS; j++ ) {
      char buf2[MAX_STRING_LENGTH];
      sprintf( buf2, "( %s, %d, %d )",
	       quotify(attr_flags[ATTR_STR+j].name,"'"), race->stats[j], race->max_stats[j] );
      strcat( raceAttributes, buf2 );
      if ( j < MAX_STATS-1 )
	strcat(raceAttributes, ", ");
      strcat( raceAttributes, "\n                ");
    }
    
    fprintf( fp,
	     "PCRace %s {\n"
	     "  WhoName = %s;\n"
	     "  Experience = %d;\n"
	     //	     "  Type = %d;\n"
	     "  Type = %s;\n"
	     //	     "  Size = %s;\n"
	     "  Attributes = (%s);\n"
	     "  Language = %s;\n"
	     "  Abilities = (%s);\n"
	     "  Alignment = (%s);\n"
	     "  Classes = (%s);\n"
	     "  Remort = (%s);\n"
	     "}\n\n",
	     quotify(race->name,"'"), quotify(race->who_name,"'"), race->expl,
	     quotify(flag_string( race_type_flags, race->type ) ),
	     //	     quotify(size_table[race->size].name,"'"),
	     raceAttributes, 
	     quotify(language_name(race->language),"'"),
	     create_abilities_list(race->nb_abilities, race->abilities),
	     create_alignment_list(race->nb_allowed_align, race->allowed_align), 
	     //create_classes_list(race->nb_allowed_class, race->allowed_class) );
	     list_flag_string( race->allowed_class, classes_flags ),
	     create_races_list(race->nb_remorts, race->remorts) );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  racenotsaved = FALSE;
}

// Store temporary remort/rebirth list for each race
ValueList **race_remort;
ValueList **race_rebirth;
void new_load_pcraces() {
  FILE *fp;
  char buf[MAX_STRING_LENGTH];
  const char *buf2, *buf3;

  log_string("Reading pc race table");

  fclose( fpReserve );
  if ( ( fp = fopen( PCRACE_FILE, "r" ) ) == NULL ){
    bug( "Can't access race file: [%s]!", PCRACE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );
  log_stringf(" %d pc races found.", MAX_PC_RACE );

  // now set the race_table.pc_race flag
  for ( int i = 0; i < MAX_PC_RACE; i++ ){
    int j;
    
    j = race_lookup( pc_race_table[i].name, TRUE );
    if ( j < 0 || j > MAX_RACE ){
      bug( "[%s] is an invalid pc race, doesn't exist in global races file", pc_race_table[i].name);
      exit(-1);
    }
    races_flags[j].settable = TRUE;
    race_table[j].pc_race = TRUE;

    if ( IS_SET( race_table[j].form, FORM_ANIMAL ) )
      bug("pc race [%s] has form ANIMAL set.", race_table[j].name );
    if ( !IS_SET( race_table[j].form, FORM_SENTIENT ) )
      bug("pc race [%s] has NOT form SENTIENT", race_table[j].name );
  }

  // Create remort vector
  for ( int i = 0; i < MAX_PC_RACE; i++ ) {
    //    log_stringf("Race: %s", pc_race_table[i].name );
    if ( race_remort[i] != NULL ) { // pc race has remort races
      pc_race_table[i].nb_remorts = race_remort[i]->size;
      if ( ( pc_race_table[i].remorts = (int*)GC_MALLOC(sizeof(int)*pc_race_table[i].nb_remorts)) == NULL ) 
	p_error("Can't allocate memory for remorts");
      int index = 0;
      for ( int j = 0; j < race_remort[i]->size; j++ ) {
	const char *s = race_remort[i]->elems[j].asStr();
	int iRace = race_lookup( s, TRUE );
	if ( iRace < 0 ) {
	  bug("Invalid remort race [%s] for race [%s]", s, pc_race_table[i].name );
	  pc_race_table[i].nb_remorts--;
	  continue;
	}
	pc_race_table[i].remorts[index] = iRace;
	index++;
	//	log_stringf("  [%s]", pc_race_table[iRace].name );
      }
    }
    race_remort[i]->explicit_free();
  }

  // Create rebirth vector
  // List of couple ( rebirth, probability ) -> Sum probability must be equal 100
  for ( int i = 0; i < MAX_PC_RACE; i++ ) {
    //    log_stringf("Race: %s", pc_race_table[i].name );
    if ( race_rebirth[i] != NULL ) { // pc race has rebirth races
      int total = 0;
      pc_race_table[i].nb_rebirth = race_rebirth[i]->size;
      if ( ( pc_race_table[i].rebirth_list = (int*)GC_MALLOC(sizeof(int)*pc_race_table[i].nb_rebirth)) == NULL ) 
	p_error("Can't allocate memory for rebirth list");
      if ( ( pc_race_table[i].rebirth_probability = (int*)GC_MALLOC(sizeof(int)*pc_race_table[i].nb_rebirth)) == NULL ) 
	p_error("Can't allocate memory for rebirth probability");
      int index = 0;
      for ( int j = 0; j < race_rebirth[i]->size; j++ ) {
	ValueList *couple = race_rebirth[i]->elems[j].asList();
	if ( couple->size != 2 ) {
	  bug("Invalid number of elements in couple (Race Name, Probability)");
	  pc_race_table[i].nb_rebirth--;
	  continue;
	}
	const char *s = couple->elems[0].asStr();
	int iRace = race_lookup( s, TRUE );
	if ( iRace < 0 ) {
	  bug("Invalid rebirth race [%s] for race [%s]", s, pc_race_table[i].name );
	  pc_race_table[i].nb_rebirth--;
	  continue;
	}
	if ( pc_race_table[iRace].type != RACE_REBIRTH ) {
	  bug("Rebirth race [%s] for race [%s] is NOT SET as type rebirth", s, pc_race_table[i].name );
	  pc_race_table[i].nb_rebirth--;
	  continue;
	}
	pc_race_table[i].rebirth_list[index] = iRace;
	pc_race_table[i].rebirth_probability[index] = couple->elems[1].asInt();
	total += pc_race_table[i].rebirth_probability[index];
	index++;
      }
      if ( total != 100 )
	bug("Race [%s] has rebirth probability [%d] DIFFERENT from 100",
	    pc_race_table[i].name, total );
    }
    race_rebirth[i]->explicit_free();
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  racenotsaved = FALSE;
}
void create_pcraces_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_PC_RACE = count;
  if (( pc_race_table = (pc_race_type *)GC_MALLOC( sizeof(pc_race_type)*MAX_PC_RACE)) == NULL )
    p_error( "Can't allocate memory for pc_race_table" );

  // temporary structure storing remort races
  if (( race_remort = (ValueList **)GC_MALLOC( sizeof(ValueList *)*MAX_PC_RACE)) == NULL )
    p_error( "Can't allocate memory for remort race" );
  // temporary structure storing rebirth races
  if (( race_rebirth = (ValueList **)GC_MALLOC( sizeof(ValueList *)*MAX_PC_RACE)) == NULL )
    p_error( "Can't allocate memory for rebirth race" );

  done = TRUE;
}
void parse_race_abilities( DATAData *field, pc_race_type *race ) {
  ValueList *list = field->value->eval().asList();

  race->nb_abilities = list->size;
  if ( race->nb_abilities == 0 ) {
    race->abilities = NULL;
    if ( DATA_VERBOSE > 2 ) {
      printf("  No ability\n\r");
    }
    return;
  }
  if ( ( race->abilities = (int*) GC_MALLOC( race->nb_abilities * sizeof(int) ) ) == NULL )
    p_error("Can't allocate memory abilities pc_race_table (1)");

  for ( int i = 0; i < list->size; i++ ) {
    const char *buf = list->elems[i].asStr();
    if ( ( race->abilities[i] = ability_lookup(buf) ) <= 0 )
      p_error("Invalid race ability (%s) for race (%s)", buf, race->name );
    if ( DATA_VERBOSE > 2 ) {
      printf("  abilities: %s\n\r", buf );
    }
  }

  list->explicit_free();
}
void parse_race_align( DATAData *field, pc_race_type *race ) {
  ValueList *list = field->value->eval().asList();

  race->nb_allowed_align = list->size;
  if ( race->nb_allowed_align == 0 )
    p_error("No alignment for race %s", race->name );
  if ( ( race->allowed_align = (align_info*) GC_MALLOC( race->nb_allowed_align * sizeof(align_info) ) ) == NULL )
    p_error("Can't allocate memory allowed_align pc_race_table (1)");
  
  for ( int i = 0; i < list->size; i++ ) {
    const char *buf = list->elems[i].asStr();
    if ( convert_etho_align_string_to_align_info( buf, race->allowed_align[i] ) == -1 )
      p_error("Invalid alignment (%s) for race (%s)", buf, race->name );
    if ( DATA_VERBOSE > 2 ) {
      printf("  align: %s\n\r", buf );
    }
  }

  list->explicit_free();
}
//void parse_race_classes( DATAData *field, pc_race_type *race ) {
//  ValueList *list = field->value->eval().asList();
//
//  race->nb_allowed_class = list->size;
//  if ( race->nb_allowed_class == 0 )
//    p_error("No classes for race %s", race->name );
//  if ( ( race->allowed_class = (int*) GC_MALLOC( race->nb_allowed_class * sizeof(int) ) ) == NULL )
//    p_error("Can't allocate memory allowed_class pc_race_table (1)");
//
//  for ( int i = 0; i < list->size; i++ ) {
//    const char *buf = list->elems[i].asStr();
//    if ( ( race->allowed_class[i] = 1<<class_lookup(buf) ) <= 0 )
//      p_error("Invalid %d° class (%s) for race (%s)", i+1, buf, race->name );
//    if ( DATA_VERBOSE > 2 ) {
//      printf("  class: %s\n\r", buf );
//    }
//  }
//}
void parse_race_attributes( DATAData *field, pc_race_type *race ) {
  ValueList *list = field->value->eval().asList();
  
  if ( list->size != MAX_STATS )
    p_error("Wrong number of attributes (%d) for race %s", list->size, race->name );
  for ( int i = 0; i < list->size; i++ ) {
    ValueList *triplet = list->elems[i].asList();
    if ( triplet->size != 3 )
      p_error("Wrong number of elements in triplet <Attr Name> <Base Value> <Max Value>");
    const char *s = triplet->elems[0].asStr();
    int attrId = flag_value_init( attr_flags, s ) - ATTR_STR;
    if ( attrId < STAT_STR || attrId > MAX_STATS )
      p_error("Invalid attributes %s for race %s", s, race->name );
    race->stats[attrId] = triplet->elems[1].asInt();
    race->max_stats[attrId] = triplet->elems[2].asInt();
    if ( DATA_VERBOSE > 2 ) {
      printf("  Attributes  %5d  %3d/%3d\n\r", attrId, race->stats[attrId], race->max_stats[attrId] );
    }
  }

  list->explicit_free();
}
void parse_pcrace( DATAData *pcrace ) {
  static int index = 0;
  //pc_race_type *race = &(pc_race_table[index]);
  //race->name = str_dup(pcrace->value->eval().asStr());
  const char *raceName = pcrace->value->eval().asStr();
  int raceId = race_lookup( raceName, TRUE );
  if ( raceId < 0 )
    p_error("Invalid pc race %s, not found in global race file.", raceName );
  pc_race_type *race = &(pc_race_table[raceId]);
  race->name = str_dup(raceName);
  race->min_remort_number = 0;
  race->super_race = -1;

  if ( DATA_VERBOSE > 0 ) {
    printf(" PCRace %s\n\r", race->name );
  }
  // fields
  for ( int fieldCount = 0; fieldCount < pcrace->fields_count; fieldCount++ ) {
    DATAData *field = pcrace->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_WhoName:
      strcpy(race->who_name,field->value->eval().asStr());
      if ( DATA_VERBOSE > 2 ) {
	printf("  whoname: %s\n\r", race->who_name);
      }
      break;
    case TAG_Experience:
      race->expl = field->value->eval().asInt();
      if ( race->expl < 1000 || race->expl > 3500 )
	log_stringf("WARNING: pcrace %s requires %d exp/level",
		    race->name, race->expl );
      if ( DATA_VERBOSE > 2 ) {
	printf("  experience: %d\n\r", race->expl );
      }
      break;
    case TAG_Type: {
      const char *s = field->value->eval().asStr();
      if ( ( race->type = flag_value( race_type_flags, s ) ) == NO_FLAG ) {
	bug("Invalid pc race type [%s] for race [%s], assuming cursed race", s, race->name );
	race->type = RACE_CURSED;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  type: %s\n\r", flag_string( race_type_flags, race->type ) );
      }
      break;
    }
    case TAG_Attributes: parse_race_attributes( field, race ); break;
    case TAG_Language: {
      const char *s = field->value->eval().asStr();
      race->language = language_lookup( s );
      if ( race->language < 0 ) {
	bug("Invalid language (%s) for pcrace [%s], assuming [%s]", s, race->name, DEFAULT_LANGUAGE_NAME );
	//race->language = language_lookup( "common" );
	race->language = DEFAULT_LANGUAGE;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  language: %s\n\r", language_name(race->language));
      }
      break;
    }
    case TAG_Abilities: parse_race_abilities( field, race ); break;
    case TAG_Alignment: parse_race_align( field, race ); break;
      //case TAG_Classes: parse_race_classes( field, race ); break;
    case TAG_Classes:
      if ( ( race->allowed_class = list_flag_value( field->value, classes_flags ) ) == NO_FLAG )
	p_error("Invalid class for pcrace %s", race->name );
      if ( DATA_VERBOSE > 2 ) {
	printf("  Classes: %s\n\r", flag_string( classes_flags, race->allowed_class ) );
      }
      break;
      // Put remort list in a temporary vector, real pc_race_type->remort will be
      //  initialized at the end of new_load_pcraces
    case TAG_Remort: race_remort[raceId] = field->value->eval().asList(); break;
    case TAG_Min_Num_Remort: race->min_remort_number = field->value->eval().asInt(); break;
      // Put rebirth list in a temporary vector, real pc_race_type->rebirth will be
      //  initialized at the end of new_load_pcraces
    case TAG_Rebirth: race_rebirth[raceId] = field->value->eval().asList(); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  index++;
}

void new_reload_races() {
  int old_max_race = MAX_RACE,
    old_max_pc_race = MAX_PC_RACE;

  log_stringf("Reloading race/pcrace table");

  // reload tables
  new_load_races();
  new_load_pcraces();

  if ( old_max_race > MAX_RACE 
       || old_max_pc_race > MAX_PC_RACE ) {
    bug("reload_race: less max_race or max_pc_race after reload");
    exit(-1);
  }

  update_flag_stat_table( "race", races_flags );
  update_flag_stat_table_init( "race", races_flags );
  update_attr_table( "race", races_flags );
  update_restr_table( "race", races_flags );
  update_help_table( "race", races_flags );
}

//******************************************************************************************
//****************************************** Spheres ***************************************
// Added by SinaC 2003 for god sphere
//  same as group with more informations
// group_table is updated by this function
// sphere_table is created and deleted at the end of load_gods
//Sphere <STRING Sphere Name> {
//  Cost = <INTEGER Train cost>;
//  Contains = <LIST OF LIST> ( ( <STRING Ability Name>, <INTEGER Level>, <INTEGER Cost>), ... );
//}
void new_save_spheres() {
  FILE *fp;

  log_string("Saving spheres");
  
  fclose( fpReserve );
  if ( ( fp = fopen( SPHERE_FILE, "w" ) ) == NULL ){
    bug( "Can't access sphere file: [%s]!", SPHERE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_SPHERE; i++ ) {
    char sphereAbilities[MAX_STRING_LENGTH];
    sphere_type *sp = &(sphere_table[i]);

    sphereAbilities[0] = '\0';
    for ( int j = 0; j < sp->nb_abilities; j++ ) {
      char buf2[MAX_STRING_LENGTH];
      ability_info_sphere *sk = &( sp->ability_list[j] );
      sprintf( buf2, "( %s, %d, %d )", 
	       quotify(ability_table[sk->sn].name,"'"), sk->level, sk->rate );
      strcat( sphereAbilities, buf2 );
      if ( j < sp->nb_abilities-1 )
	strcat( sphereAbilities, ", ");
      strcat( sphereAbilities, "\n             ");
    }

    fprintf( fp,
	     "Sphere %s {\n"
	     "  Cost = %d;\n"
	     "  Contains = (%s);\n"
	     "}\n\n",
	     quotify(sp->sphere_name,"'"), sp->rate,
	     sphereAbilities );
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  groupnotsaved = FALSE;
}

void new_load_spheres() {
  FILE *fp;

  log_string("Reading spheres");
  
  fclose( fpReserve );
  if ( ( fp = fopen( SPHERE_FILE, "r" ) ) == NULL ){
    bug( "Can't access sphere file: [%s]!", SPHERE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );
  log_stringf(" %d spheres found.", MAX_SPHERE );
  group_type *tmp;
  if ( ( tmp = (group_type*) GC_MALLOC( (MAX_GROUP+MAX_SPHERE)*sizeof(group_type) ) ) == NULL )
    p_error("Can't allocate memory for group_table (2)");
  bcopy(group_table,tmp, MAX_GROUP*sizeof(group_type));
  group_table = tmp;

  for ( int i = 0; i < MAX_SPHERE; i++ ) {
    sphere_type *sp = &(sphere_table[i]);
    group_type *gr = &(group_table[MAX_GROUP]);

    gr->name = str_dup( sp->sphere_name );
    if ( ( gr->rating = (int*)GC_MALLOC_ATOMIC(MAX_CLASS * sizeof(int))) == NULL ) {
      bug("Can't allocate memory group_table.rating (2)");
      exit(-1);
    }
    memset( gr->rating, 0, MAX_CLASS*sizeof(int));
    gr->spellnb = sp->nb_abilities;
    if ( ( gr->spells = (const char**)GC_MALLOC(gr->spellnb * sizeof(char*)) ) == NULL ) {
      bug("Can't allocate memory group_table.spells (2)");
      exit(-1);
    }
    for ( int j = 0; j < gr->spellnb; j++ )
      gr->spells[j] = str_dup( ability_table[sp->ability_list[j].sn].name );
    //group_table[MAX_GROUP].god_rate; updated in load_gods

    gr->isSphere = TRUE;

    MAX_GROUP++;
  }
  
  log_stringf("MAX_GROUP updated %d", MAX_GROUP );

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  groupnotsaved = FALSE;
}

void create_spheres_table( const int count ) {
  static bool done = FALSE;
  if ( done )
    return;
  MAX_SPHERE = count;
  if ( ( sphere_table = (sphere_type*) GC_MALLOC( MAX_SPHERE * sizeof(sphere_type) ) ) == NULL )
    p_error("Can't allocate memory: sphere_table (1)");
  done = TRUE;
}

void parse_sphere_contains( DATAData *contains, sphere_type *sp ) {
  ValueList *list = contains->value->eval().asList();

  sp->nb_abilities = list->size;
  if ( sp->nb_abilities <= 0 )
    p_error("Invalid number of skills (%d) for sphere %s", sp->nb_abilities, sp->sphere_name);

  if ( ( sp->ability_list = (ability_info_sphere*)GC_MALLOC(sp->nb_abilities * sizeof(ability_info_sphere) ) ) == NULL )
    p_error("Can't allocate memory for ability_list in sphere_table (1)" );

  for ( int i = 0; i < sp->nb_abilities; i++ ) {
    ValueList *triplet = list->elems[i].asList();
    ability_info_sphere *sk = &(sp->ability_list[i]);
    if ( triplet->size != 3 )
      p_error("Wrong number of elements in triplet <Ability Name> <Level> <Cost> for sphere %s",
	      sp->sphere_name );
    const char *s = triplet->elems[0].asStr();
    if ( ( sk->sn = ability_lookup(s) ) < 0 )
      p_error("Invalid ability %s in sphere %s", s, sp->sphere_name );
    if ( ( sk->level = triplet->elems[1].asInt() ) <= 0 )
      p_error("Invalid level %d for ability %s in sphere %s", sk->level, s, sp->sphere_name );
    if ( ( sk->rate = triplet->elems[2].asInt() ) <= 0 )
      p_error("Invalid rate %d for ability %s in sphere %s", sk->rate, s, sp->sphere_name );
    if ( DATA_VERBOSE > 2 ) {
      printf("  ability  %15s %3d %3d\n\r", s, sk->level, sk->rate );
    }
    triplet->explicit_free();
  }

  list->explicit_free();
}

void parse_sphere( DATAData *sphere ) {
  static int index = 0;

  sphere_type *sp = &( sphere_table[index] );
  sp->gn = MAX_GROUP + index;
  sp->sphere_name = str_dup( sphere->value->eval().asStr() );
  if ( DATA_VERBOSE > 0 ) {
    printf("Sphere %s\n\r", sp->sphere_name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < sphere->fields_count; fieldCount++ ) {
    DATAData *field = sphere->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Cost:
      sp->rate = field->value->eval().asInt();
      if ( sp->rate < 0 ) {
	bug("Invalid rate (%d) for sphere [%s], assuming rate = 0 (minor sphere)", 
	    sp->rate, sp->sphere_name );
	sp->rate = 0;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  rate/cost: %d\n\r", sp->rate );
      }
      break;
    case TAG_Contains: parse_sphere_contains( field, sp ); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  index++;
}

//**************************************************************************************************
//********************************************* Groups *********************************************
// FIXME: gr->spells will not be string anymore
//         but a struct with 2 elements: a boolean giving if spells[i] is an ability or a group
//                                       and an integer giving group number or ability number
//Group <STRING Group Name> {
//  Contains = <LIST OF LIST> ( ( <STRING Type (Ability/Group)>, <STRING Ability/Group Name>), ... );
//}
void new_save_groups() {
  FILE *fp;

  log_string("Saving group table");

  fclose( fpReserve );
  if ( ( fp = fopen( GROUP_FILE, "w" ) ) == NULL ){
    bug( "Can't access group file: [%s]!", GROUP_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    char groupAbilities[MAX_STRING_LENGTH];
    group_type *gr = &(group_table[i]);

    if ( gr->isSphere ) // we don't save spheres, only general groups
      continue;

    groupAbilities[0] = '\0';
    for ( int j = 0; j < gr->spellnb; j++ ) {
      char buf2[MAX_STRING_LENGTH];
      sprintf(buf2,"( '%s', %s )",
	      group_lookup(gr->spells[j])<0?"Ability":"Group",
	      quotify(gr->spells[j],"'"));
      strcat( groupAbilities, buf2 );
      if ( j < gr->spellnb-1 )
	strcat( groupAbilities, ", ");
      strcat( groupAbilities, "\n              ");
    }
    fprintf( fp,
	     "Group %s {\n"
	     "  Contains = (%s);\n"
	     "}\n\n",
	     quotify(gr->name,"'"),
	     groupAbilities );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  groupnotsaved = FALSE;
}

void new_load_groups() {
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  log_string("Reading group table");
  
  fclose( fpReserve );
  if ( ( fp = fopen( GROUP_FILE, "r" ) ) == NULL ){
    bug( "Can't access group file: [%s]!", GROUP_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas(fp);
  log_stringf(" %d groups found.", MAX_GROUP );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  groupnotsaved = FALSE;
}

void create_groups_table( const int count ) {
  static bool done = FALSE;
  if ( done )
    return;

  MAX_GROUP = count;
  if ( ( group_table = (group_type*) GC_MALLOC( MAX_GROUP * sizeof(group_type) ) ) == NULL ){
    bug("Can't allocate memory group_table (1)");
    exit(-1);
  }
  done = TRUE;
}

void parse_groups_contains( DATAData *contains, group_type *gr ) {
  ValueList *list = contains->value->eval().asList();
  
  gr->spellnb = list->size;
  if ( gr->spellnb <= 0 )
    p_error("Invalid number of ability/group for group %s", gr->name );

  if ( ( gr->spells = (const char**)GC_MALLOC(sizeof(char*)*gr->spellnb) ) == NULL )
    p_error("Can't allocate memory group_table (3)");
  
  for ( int i = 0; i < list->size; i++ ) {
    ValueList *couple = list->elems[i].asList();
    if ( couple->size != 2 )
      p_error("Wrong number of elements in couple <Ability/Group> <Ability/Group name> for group %s",
	      gr->name );
    const char *s = couple->elems[0].asStr();
    if ( !str_cmp( s, "group" ) ) {
      gr->spells[i] = str_dup(couple->elems[1].asStr());
      //Can't do a group_lookup, we are constructing group_table
      //if ( group_lookup( gr->spells[i] ) < 0 )
      //	p_error("Invalid group name %s  in group %s", gr->spells[i], s );
    }
    else if ( !str_cmp( s, "ability") ) {
      gr->spells[i] = str_dup(couple->elems[1].asStr());
      if ( ability_lookup( gr->spells[i] ) < 0 )
	p_error("Invalid ability name %s  in group %s", gr->spells[i], s );
    }
    else
      p_error("Invalid info for group %s: Ability or Group was expected", gr->name );
    if ( DATA_VERBOSE > 2 ) {
      printf("  %s  %s\n\r", s, gr->spells[i] );
    }

    couple->explicit_free();
  }

  list->explicit_free();
}

void parse_groups( DATAData *group ) {
  static int index = 0;
  group_type *gr = &(group_table[index]);
  gr->name = str_dup(group->value->eval().asStr());
  if ( DATA_VERBOSE > 0 ) {
    printf("  Group %s\n\r", gr->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < group->fields_count; fieldCount++ ) {
    DATAData *field = group->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Contains: parse_groups_contains( field, gr ); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  gr->isSphere = FALSE;
  index++;
}

//******************************************************************************************************
//*********************************************** Abilities ********************************************
//Ability <STRING Ability Name> {
//  Type = <STRING Ability type (Spell/Skill/Power/Song)>;
//  Target = <STRING Target>;
//  Position = <STRING Position>;
//  Slot = <INTEGER Slot>;
//  Cost = <INTEGER Ressource Cost (in Mana or Psp)>;
//  Beats = <INTEGER Beats>;
//  NounDamage = <STRING Damage Noun>;
//  MsgOff = <STRING Affect wear off message on char>;
//  MsgObj = <STRING Affect wear off message on obj>;
//  MsgRoom = <STRING Affect wear off message on room>;
//  MobUse = <INTEGER Mob Use>;
//  Dispel = <BOOLEAN Is affect dispellable (True/False)>;
//  MsgDispel = <STRING Affect dispelled message>;
//  Wait = <INTEGER Wait time between 2 ability use>;
//  Craftable = <BOOLEAN Can ability be crafted in item <True/False>;
//}
int count_init_ability() {
  int i;
  for ( i = 0; ability_table_init[i].name != NULL && ability_table_init[i].name[0] != '\0'; i++ )
    ;
  return i;
}
int ability_lookup_init( const char *name, const int from = 0 ) {
  for ( int i = from; i < MAX_ABILITY_INIT; i++ )
    if (!str_cmp( name, ability_table_init[i].name ))
      return i;
  return -1;
}
void fwrite_nonnull_string_quotify( FILE *fp, const char *tocheck, const char *s ) {
  if ( tocheck && tocheck[0] != '\0' && str_cmp(tocheck, "none") )
    fprintf( fp, s, quotify( tocheck, "'" ) );
}
// Utility functions used to load abilities
void nullify_ability( const int i ) {
  ability_type *sk = &(ability_table[i]);
  sk->name = NULL;
  sk->target = TAR_IGNORE;
  sk->minimum_position = POS_STANDING;
  sk->slot = 0;
  sk->min_cost = 0;
  sk->beats = 0;
  sk->noun_damage = NULL;
  sk->msg_off = NULL;
  sk->msg_obj = NULL;
  sk->msg_room = NULL;
  //sk->mob_use = MOB_USE_NONE;
  sk->mob_use = 0; // SinaC 2003
  sk->dispellable = FALSE;
  sk->msg_dispel = NULL;
  sk->to_wait = 0;
  sk->craftable = FALSE;
  sk->prereqs = NULL;
  sk->scriptAbility = FALSE;
  sk->action_fun = NULL;
  sk->nb_casting_level = 0;
  sk->wearoff_fun = wearoff_null;
  sk->update_fun = update_null;
  sk->pgsn = NULL;
  sk->school = -1;
}
void noneize( const int i ) {
  ability_type *sk = &(ability_table[i]);
  if ( sk->noun_damage == NULL ) sk->noun_damage = str_dup("none");
  if ( sk->msg_off == NULL ) sk->msg_off = str_dup("none");
  if ( sk->msg_obj == NULL ) sk->msg_obj = str_dup("none");
  if ( sk->msg_room == NULL ) sk->msg_room = str_dup("none");
  if ( sk->msg_dispel == NULL ) sk->msg_dispel = str_dup("none");
}
void copy_ability( ability_type *from, ability_type *to ) {
  to->name = str_dup(from->name);
  to->target = from->target;
  to->minimum_position = from->minimum_position;
  to->slot = from->slot;
  to->min_cost = from->min_cost;
  to->beats = from->beats;
  to->noun_damage = str_dup(from->noun_damage);
  to->msg_off = str_dup(from->msg_off);
  to->msg_obj = str_dup(from->msg_obj);
  to->msg_room = str_dup(from->msg_room);
  to->mob_use = from->mob_use;
  to->dispellable = from->dispellable;
  to->msg_dispel = str_dup(from->msg_dispel);
  to->to_wait = from->to_wait;
  to->craftable = from->craftable;
  to->type = from->type;
  to->scriptAbility = from->scriptAbility;
  //to->prereqs = NULL; no need to copy this, this are read later
  to->nb_casting_level = from->nb_casting_level;
  //to->school = -1; no need to copy this, this is read later
}
void free_ability( ability_type *a ) {
  a->target = 0;
  a->minimum_position = 0;
  a->slot = 0;
  a->min_cost = 0;
  a->beats = 0;
  a->mob_use = 0;
  a->dispellable = 0;
  a->to_wait = 0;
  a->craftable = 0;
  a->type = 0;
  a->scriptAbility = FALSE;
}
int find_ability( const char *name, const int to ) {
  for ( int i = 0; i < to; i++ ) {
    if ( ability_table[i].name == NULL )
      break;
    if ( LOWER(name[0]) == LOWER(ability_table[i].name[0])
	 && !str_prefix( name, ability_table[i].name ) )
      return i;
  }
  return -1;
}

void new_save_abilities() {
  FILE *skill_fp, *spell_fp, *power_fp, *song_fp, *affect_fp, *fp;
  log_string("Saving abilities...");

  if (!(skill_fp = fopen ( SKILLS_FILE, "w"))) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Could not open file [%s] in order to load skills.", SKILLS_FILE );
    return;
  }
  if (!(spell_fp = fopen ( SPELLS_FILE, "w"))) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Could not open file [%s] in order to load spells.", SPELLS_FILE );
    return;
  }
  if (!(power_fp = fopen ( POWERS_FILE, "w"))) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Could not open file [%s] in order to load powers.", POWERS_FILE );
    return;
  }
  if (!(song_fp = fopen ( SONGS_FILE, "w"))) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Could not open file [%s] in order to load songs.", SONGS_FILE );
    return;
  }
  if (!(affect_fp = fopen ( AFFECTS_FILE, "w"))) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Could not open file [%s] in order to load special affects.", AFFECTS_FILE );
    return;
  }
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ability_type *sk = &(ability_table[i]);
    switch( sk->type ) {
    case TYPE_SKILL: fp = skill_fp; break;
    case TYPE_SPELL: fp = spell_fp; break;
    case TYPE_POWER: fp = power_fp; break;
    case TYPE_SONG: fp = song_fp; break;
    case TYPE_AFFECT: fp = affect_fp; break;
    default: 
      bug("Invalid ability type %d for ability [%s], assuming skill", sk->type, sk->name );
      fp = skill_fp;
      sk->type = TYPE_SKILL;
      break;
    }
    fprintf( fp, 
	     "Ability %s {\n"
	     "  Type = %s;\n", 
	     quotify(sk->name,"'"),
	     quotify(flag_string( ability_type_flags, sk->type ),"'") );
    fprintf( fp, "  Target = %s;\n", quotify(target_table[sk->target].short_name,"'") );
    fprintf( fp, "  Position = %s;\n", quotify( flag_string( position_flags, sk->minimum_position),"'") );
    if ( sk->slot != 0 ) fprintf( fp, "  Slot = %d;\n", sk->slot );
    if ( sk->min_cost != 0 ) fprintf( fp, "  Cost = %d;\n", sk->min_cost );
    if ( sk->beats != 0 ) fprintf( fp, "  Beats = %d;\n", sk->beats );
    fwrite_nonnull_string_quotify( fp, sk->noun_damage, "  NounDamage = %s;\n" );
    fwrite_nonnull_string_quotify( fp, sk->msg_off, "  MsgOff = %s;\n" );
    fwrite_nonnull_string_quotify( fp, sk->msg_obj, "  MsgObj = %s;\n" );
    fwrite_nonnull_string_quotify( fp, sk->msg_room, "  MsgRoom = %s;\n" );
    //fprintf( fp, "  MobUse = %s;\n", quotify(flag_string(mob_use_flags,sk->mob_use)) ); SinaC 2003
    fprintf( fp, "  MobUse = (%s);\n", list_flag_string(sk->mob_use,mob_use_flags) );
    if ( sk->dispellable ) fprintf( fp, "  Dispel = %s;\n", sk->dispellable?"true":"false" );
    fwrite_nonnull_string_quotify( fp, sk->msg_dispel, "  MsgDispel = %s;\n" );
    if ( sk->to_wait != 0 ) fprintf( fp, "  Wait = %d;\n", sk->to_wait );
    if ( sk->craftable != 0 ) fprintf( fp, "  Craftable = %s;\n", sk->craftable?"true":"false" );
    if ( sk->scriptAbility && sk->nb_casting_level > 0 ) fprintf( fp, "  MaxCasting = %d;\n", sk->nb_casting_level );
    if ( sk->scriptAbility ) fprintf( fp, "  ScriptAbility = %s;\n", sk->scriptAbility?"true":"false");
    fprintf(fp,"}\n\n");
  }
  fclose(skill_fp); fclose(spell_fp), fclose(power_fp); fclose(song_fp); fclose(affect_fp);

  abilitynotsaved = FALSE;
}
//int new_load_ability_type( const int type ) {
//  FILE *fp;
//  //const char *filename;
//  char filename[MAX_STRING_LENGTH];
//  int count = 0;
//
//  switch ( type ) {
//  case TYPE_SKILL: strcpy( filename, SKILLS_FILE ); break;
//  case TYPE_SPELL: strcpy( filename, SPELLS_FILE ); break;
//  case TYPE_POWER: strcpy( filename, POWERS_FILE ); break;  
//  case TYPE_SONG: strcpy( filename, SONGS_FILE ); break;
//  default : p_error("Invalid ability type (%d)", type );
//  }
//  fclose(fpReserve);
//  if (!(fp = fopen ( filename, "r"))) {
//    fpReserve = fopen( NULL_FILE, "r" );
//    bug("Could not open file %s in order to load %ss.", abilitytype_name(type), filename );
//  }
//  else {
//    count = parse_datas( fp );
//    if ( count == 0 )
//      log_stringf("no %ss found.", abilitytype_name(type) );
//    else
//      log_stringf("%d %ss found.", count, abilitytype_name(type) );
//    fclose(fp);
//  }
//  fpReserve = fopen( NULL_FILE, "r" );
//  return count;
//}
DATAModule *new_load_ability_type( const int type ) {
  FILE *fp;
  char filename[MAX_STRING_LENGTH];
  int count = 0;

  switch ( type ) {
  case TYPE_SKILL: strcpy( filename, SKILLS_FILE ); break;
  case TYPE_SPELL: strcpy( filename, SPELLS_FILE ); break;
  case TYPE_POWER: strcpy( filename, POWERS_FILE ); break;  
  case TYPE_SONG: strcpy( filename, SONGS_FILE ); break;
  case TYPE_AFFECT: strcpy( filename, AFFECTS_FILE ); break;
  default : p_error("Invalid ability type (%d)", type );
  }
  fclose(fpReserve);
  if (!(fp = fopen ( filename, "r"))) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Could not open file %s in order to load [%s].", abilitytype_name(type), filename );
    return NULL;
  }
  else
    return get_data_module( fp );
  return NULL;
}
void new_load_abilities() {
  log_string("Reading abilities...");

  // Count number of abilities in ability_table_init
  MAX_ABILITY_INIT = count_init_ability();

  // Check duplicates in ability_table_init
  for ( int i = 0; i < MAX_ABILITY_INIT; i++ )
    if ( ability_lookup_init( ability_table_init[i].name, i+1 ) != -1 )
      bug("Ability [%s] is duplicated in ability_table_init", ability_table_init[i].name );
  
  int count = 0;
  // We read every ability module
  DATAModule *skill_module = new_load_ability_type( TYPE_SKILL );
  if ( skill_module != NULL ) count += skill_module->datas_count;
  DATAModule *spell_module = new_load_ability_type( TYPE_SPELL );
  if ( spell_module != NULL ) count += spell_module->datas_count;
  DATAModule *power_module = new_load_ability_type( TYPE_POWER );
  if ( power_module != NULL ) count += power_module->datas_count;
  DATAModule *song_module = new_load_ability_type( TYPE_SONG );
  if ( song_module != NULL  ) count += song_module->datas_count;
  DATAModule *affect_module = new_load_ability_type( TYPE_AFFECT );
  if ( affect_module != NULL  ) count += affect_module->datas_count;

  MAX_ABILITY = count;

  // Create ability table
  if ( ( ability_table = (ability_type *) GC_MALLOC(sizeof(ability_type)*(MAX_ABILITY)) ) == NULL )
    p_error("Can't allocate memory ability_table");
  for ( int i = 0; i < MAX_ABILITY; i++ ) // nullify ability_table
    ability_table[i].name = NULL;

  fclose(fpReserve);

  // After creating the table, we extract informations from module
  if ( skill_module != NULL ) parse_data_module( skill_module );
  if ( spell_module != NULL ) parse_data_module( spell_module );
  if ( power_module != NULL ) parse_data_module( power_module );
  if ( song_module != NULL  ) parse_data_module( song_module );
  if ( affect_module != NULL  ) parse_data_module( affect_module );

  // Check number of ability
  if ( MAX_ABILITY < MAX_ABILITY_INIT )
    log_stringf("Some abilities are missing in abilities files (%d)",
		MAX_ABILITY_INIT-MAX_ABILITY );

  // ScriptAbility found
  if ( MAX_ABILITY - MAX_ABILITY_INIT > 0 )
    log_stringf("Should have %d script abilities.", MAX_ABILITY - MAX_ABILITY_INIT );

  // Check if every read ability is in ability_table_init
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    if ( ability_table[i].name == NULL )
      continue;
    if ( ability_lookup_init( ability_table[i].name ) == -1 
	 && !ability_table[i].scriptAbility )
      log_stringf("Ability not found in tables: %s", ability_table[i].name );
  }

  // Check if every ability has been found in files
  for ( int i = 0; i < MAX_ABILITY_INIT; i++ ) {
    int m = -1;
    if ( ability_table_init[i].name == NULL )
      continue;
    if ( ability_lookup( ability_table_init[i].name ) == -1 )
      log_stringf("Ability not found in files: %s", ability_table_init[i].name );
  }


  abilitynotsaved = FALSE;

  fpReserve = fopen( NULL_FILE, "r" );
}

void parse_abilities( DATAData *ability ) {
  static int index = 0;

  nullify_ability(index);
  const char *s = ability->value->eval().asStr();

  // Find duplicate or similar
  int dup = find_ability( s, index );
  int realIndex = index;
  if ( dup != -1 ) {
    unsigned int dupLen = strlen( ability_table[dup].name );
    if ( dupLen == strlen(s) )
      log_stringf(" duplicated ability: %s", ability_table[dup].name );
    // found an ability starting with the same name, copy the longer in new location
    else if ( dupLen > strlen(s) ) {
      log_stringf(" switching %s with %s", s, ability_table[dup].name );
      copy_ability( &ability_table[dup], &ability_table[index] ); 
      free_ability( &ability_table[dup] );
      realIndex = dup; // work with longer old location
    }
  }
  ability_type *sk = &(ability_table[realIndex]);
  sk->name = str_dup( s );
  if ( DATA_VERBOSE > 0 ) {
    printf("  Ability %s\n\r", sk->name );
  }
  // fields
  for ( int fieldCount = 0; fieldCount < ability->fields_count; fieldCount++ ) {
    DATAData *field = ability->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Type: {
      const char *s = field->value->eval().asStr();
      if ( ( sk->type = flag_value( ability_type_flags, s ) ) == NO_FLAG ) {
	bug("Invalid type (%s) for ability (%s) assuming SKILL", s, sk->name );
	sk->type = TYPE_SKILL;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  type: %s\n\r", flag_string( ability_type_flags, sk->type ) );
      }
      break;
    }
    case TAG_Target: {
      const char *s = field->value->eval().asStr();
      if ( ( sk->target = check_target( s ) ) == -1 ) {
	bug("Invalid target (%s) for ability (%s) assuming ignore", s, sk->name );
	sk->target = TAR_IGNORE;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  target: %s\n\r", target_table[sk->target].name );
      }
      break;
    }
    case TAG_Position: {
      const char *s = field->value->eval().asStr();
      if ( ( sk->minimum_position = flag_value_complete_init( position_flags, s ) ) == NO_FLAG ) {
	bug("Invalid position (%s) for ability (%s) assuming standing", s, sk->name );
	sk->minimum_position = POS_STANDING;
      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  position: %s\n\r", flag_string_init( position_flags, sk->minimum_position ) );
      }
      break;
    }
    case TAG_Slot:
      sk->slot = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  slot: %d\n\r", sk->slot );
      }
      for ( int i = 0; i < index; i++ )
	if ( ability_table[i].slot == sk->slot
	     && str_cmp( ability_table[i].name, sk->name ) ) // avoid moved ability
	  bug("Duplicate ability slot [%d] for ability [%s] and ability [%s]",
	      sk->slot, sk->name, ability_table[i].name );
      break;
    case TAG_Cost:
      sk->min_cost = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  cost: %d\n\r", sk->min_cost );
      }
      break;
    case TAG_Beats:
      sk->beats = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  beats: %d\n\r", sk->beats );
      }
      break;
    case TAG_NounDamage:
      sk->noun_damage = str_dup(field->value->eval().asStr());
      if ( DATA_VERBOSE > 2 ) {
	printf("  noun damage: %s\n\r", sk->noun_damage );
      }
      break;
    case TAG_MsgOff:
      sk->msg_off = str_dup(field->value->eval().asStr());
      if ( DATA_VERBOSE > 2 ) {
	printf("  msg off: %s\n\r", sk->msg_off );
      }
      break;
    case TAG_MsgObj:
      sk->msg_obj = str_dup(field->value->eval().asStr());
      if ( DATA_VERBOSE > 2 ) {
	printf("  msg obj: %s\n\r", sk->msg_obj );
      }
      break;
    case TAG_MsgRoom:
      sk->msg_room = str_dup(field->value->eval().asStr());
      if ( DATA_VERBOSE > 2 ) {
	printf("  msg room: %s\n\r", sk->msg_room );
      }
      break;
    case TAG_MsgDispel:
      sk->msg_dispel = str_dup(field->value->eval().asStr());
      if ( DATA_VERBOSE > 2 ) {
	printf("  msg dispel: %s\n\r", sk->msg_dispel );
      }
      break;
    case TAG_MobUse: {
      sk->mob_use = list_flag_value( field->value, mob_use_flags );
      if ( sk->mob_use == NO_FLAG ) {
	bug("Invalid mob use value [%s] for ability [%s], assuming nothing", s, sk->name );
	sk->mob_use = 0;
      }
//      const char *s = field->value->eval().asStr(); // SinaC 2003, bit vector now
//      if ( !str_cmp( s, "none" ) ) sk->mob_use = 0;
//      else if ( !str_cmp( s, "combat" ) ) SET_BIT( sk->mob_use, MOB_USE_COMBAT );
//      else if ( !str_cmp( s, "automatic" ) ) SET_BIT( sk->mob_use, MOB_USE_COMBAT );
//      else if ( !str_cmp( s, "normal" ) ) SET_BIT( sk->mob_use, MOB_USE_CHARMED );
//      else {
//	bug("Invalid mob use value [%s] for ability [%s], assuming normal", s, sk->name );
//	sk->mob_use = 0;
//      }
//      //sk->mob_use = flag_value( mob_use_flags, s );
//      if ( sk->mob_use == NO_FLAG ) {
//	bug("Invalid mob use value [%s] for ability [%s], assuming normal", s, sk->name );
//	//sk->mob_use = MOB_USE_NORMAL;
//	sk->mob_use = 0;
//      }
      if ( DATA_VERBOSE > 2 ) {
	printf("  mob use: %d\n\r", sk->mob_use );
      }
      break;
    }
    case TAG_Wait:
      sk->to_wait = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  wait: %d\n\r", sk->to_wait );
      }
      break;
    case TAG_Dispel:
      sk->dispellable = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  dispellable: %s\n\r", sk->dispellable?"true":"false" );
      }
      break;
    case TAG_Craftable:
      sk->craftable = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  craftable: %s\n\r", sk->craftable?"true":"false" );
      }
      break;
    case TAG_ScriptAbility:
      sk->scriptAbility = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  scriptAbility: %s\n\r", sk->scriptAbility?"true":"false" );
      }
      if ( ability_lookup_init( sk->name ) >= 0 && sk->scriptAbility )
	bug("'%s' is a script ability but has been found in ability_table_init!", sk->name );
      break;
    case TAG_MaxCasting:
      sk->nb_casting_level = field->value->eval().asInt();
      if ( DATA_VERBOSE > 2 ) {
	printf("  nb casting level: %d\n\r", sk->nb_casting_level );
      }
      break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

//  // Check if we have the same casting level in ability_table and in ability_table_init
//  //  only if not a scriptAbility
//  if ( !sk->scriptAbility ) {
//    int isn = ability_lookup_init( sk->name );
//    if ( isn < 0 ) {
//      bug("Ability %s found in file but not in table, set scriptAbility to TRUE if it's a script ability.",
//	  sk->name );
//      return;
//    }
//    if ( sk->nb_casting_level != ability_table_init[isn].nb_casting_level )
//      bug("Ability %s  has file casting level [%d] but table casting level [%d], keeping file casting level",
//	  sk->name, sk->nb_casting_level, ability_table_init[isn].nb_casting_level );
//  }

  noneize(realIndex);
  index++;
}

//*****************************************************************************************************
//************************************************* Class *********************************************
//Classes/Pc/Class.txt + className
//
//Class <STRING Class Name> {
//  WhoName = <STRING Who/Whois name>;
//  Attribute = <STRING Primary attribute (Strength, Intelligence, ...)>;
//  Weapon = <INTEGER Outfit weapon vnum>;
//  Adept = <INTEGER Practice percentage (75 or 90)>;
//  Thac0_00 = <INTEGER Thac0_00 value>;
//  Thac0_32 = <INTEGER Thac0_32 value>;
//  HpMin = <INTEGER Min hp bonus while gaining a level>;
//  HpMax = <INTEGER Max hp bonus while gaining a level>;
//  Type = <STRING Class type (Magic, Mental, Combat)>;
//  CastingRule = <INTEGER Casting rule>;
//  BaseGroup = <STRING Basic group name>;
//  DefaultGroup = <STRING Default group name>;
//  Parent = <STRING Parent class name>;
//  Choosable = <INTEGER Can class be picked at creation/specialization/... (0-->2)>;
//
//  Abilities = <LIST OF LIST> ( ( <INTEGER Level>, <INTEGER Cost>, <STRING Ability name>), ...);
//  Groups = <LIST OF LIST> ( ( <INTEGER Cost>, <STRING Group name>), ...);
//  Title = <LIST OF LIST> ( ( <INTEGER Level>, <STRING Male title>, <STRING Female title>), ...);
//  Pose = <LIST OF LIST> ( ( <INTEGER Id>, <STRING User msg> <STRING Other msg>), ...);
//  Alignment = <LIST OF STRING> ( <STRING Align1>, <STRING Align2>, ...);
//}
void new_save_pc_classes() {
  FILE *fp;
  
  log_string("Saving pc classes");

  fclose( fpReserve );
  if ( ( fp = fopen( PC_CLASS_FILE, "w" ) ) == NULL ){
    bug( "Can't access pc class file: [%s]!", PC_CLASS_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( int i = 0; i < MAX_CLASS; i++ ) {
    class_type *cl = &(class_table[i]);

    fprintf( fp,
	     "Class %s {\n"
	     "  WhoName = %s;\n"
	     "  Attribute = %s;\n"
	     "  Weapon = %d;\n"
	     "  Adept = %d;\n"
	     "  Thac0_00 = %d;\n"
	     "  Thac0_32 = %d;\n"
	     "  HpMin = %d;\n"
	     "  HpMax = %d;\n"
	     "  Type = (%s);\n"
	     //	     "  CastingRule = %d;\n"
	     "  BaseGroup = %s;\n"
	     "  DefaultGroup = %s;\n",
	     quotify(cl->name,"'"), quotify(cl->who_name,"'"),
	     quotify(flag_string(attr_flags,cl->attr_prime+ATTR_stat0),"'"),
	     cl->weapon, cl->ability_adept, cl->thac0_00, cl->thac0_32,
	     cl->hp_min, cl->hp_max, quotify(list_flag_string(cl->type,class_type_flags)),
	     //	     cl->max_casting_rule,
	     quotify(cl->base_group,"'"), quotify(cl->default_group,"'") );
    if ( cl->parent_class != i && cl->parent_class > 0 )
      fprintf( fp, "  Parent = %s;\n", quotify(class_table[cl->parent_class].name) );
    // Casting rules are a bit different now: one rule for each ability type
    //  we save rules only if they are different from default_casting_rule
    char buf[MAX_STRING_LENGTH];
    int count_rule = 0; // count number of rules to save
    for ( int j = 0; j < ABILITY_TYPE_COUNT; j++ )
      if ( cl->casting_rule[j] != default_casting_rule )
	count_rule++;
    if ( count_rule > 0 ) { // if there is rules to save
      int k = 0;
      buf[0] = '\0'; // save the rules
      for ( int j = 0; j < ABILITY_TYPE_COUNT; j++ ) {
	if ( cl->casting_rule[j] != default_casting_rule ) {
	  char buf2[MAX_STRING_LENGTH];
	  sprintf( buf2, "( %s, %d, %d )%s",
		   quotify(flag_string( ability_type_flags, j )), 
		   cl->casting_rule[i].highest, cl->casting_rule[i].other,
		   ( k < count_rule-1 ) ? ", ":"" );
	  strcat( buf, buf2 );
	  k++;
	}
      }
      fprintf( fp, "  CastingRule = (%s);\n", buf );
    }
    if ( cl->num_obj_list != 0 ) {
      fprintf( fp, "Obj = (");
      for ( int j = 0; j < cl->num_obj_list; j++ ) {
	fprintf( fp, "%d", cl->obj_list[j] );
	if ( j < cl->num_obj_list-1 )
	  fprintf( fp, ", ");
      }
      fprintf( fp, ");\n");
    }
    fprintf( fp, "  Choosable = %s;\n", quotify(flag_string( class_choosable_flags, cl->choosable ) ) );

    // abilities
    fprintf( fp,
	     "  Abilities = (");
    int countMax = 0;
    for (int j = 0; j < MAX_ABILITY; j++) {
      ability_type *sk = &(ability_table[j]);
      if (!sk->name || !sk->name[0])
	continue;
      if ( sk->rating[i] > 0 )
	countMax++;
    }
    int count = 0;
    for (int lev = 0; lev < LEVEL_IMMORTAL; lev++)
      for (int j = 0; j < MAX_ABILITY; j++) {
	ability_type *sk = &(ability_table[j]);
	if (!sk->name || !sk->name[0])
	  continue;
	if ( sk->ability_level[i] == lev
	     && sk->rating[i] > 0 ) {
	  fprintf( fp, "( %d, %d, %s )", lev, sk->rating[i], quotify(sk->name,"'") );
	  if ( count < countMax-1 )
	    fprintf( fp, ", ");
	  fprintf( fp, "\n               ");
	  count++;
	}
      }
    // groups
    fprintf( fp,
	     ");\n"
	     "  Groups = (");
    countMax = count = 0;
    for ( int j = 0; j < MAX_GROUP; j++ )
      if ( group_table[j].rating[i] > 0
	   && !group_table[j].isSphere )
	countMax++;
    for ( int j = 0; j < MAX_GROUP; j++ ) {
      group_type *gr = &(group_table[j]);
      if ( gr->rating[i] > 0
	   && !gr->isSphere ) {
	fprintf( fp, "( %d, %s )", gr->rating[i], quotify(gr->name,"'") );
	if ( count < countMax-1 )
	  fprintf( fp, ", ");
	fprintf( fp, "\n            ");
	count++;
      }
    }
    // titles
    fprintf( fp,
	     ");\n"
	     "  Title = (");
    count = 0;
    for ( int j = 0; j < MAX_LEVEL+1; j++ ) {
      fprintf( fp, "( %d, %s, %s )", j, quotify(title_table[i][j][0],"'"), quotify(title_table[i][j][1],"'"));
      if ( count < MAX_LEVEL+1-1 )
	fprintf( fp, ", ");
      fprintf( fp, "\n           ");
      count++;
    }
    // poses
    fprintf( fp,
	     ");\n"
	     "  Pose = (");
    count = 0;
    for ( int j = 0; j < MAX_POSE; j++ ) {
      fprintf( fp, "( %s, %s )", quotify(pose_table[i][j][0],"\""), quotify(pose_table[i][j][1],"\""));
      if ( count < MAX_POSE-1 )
	fprintf( fp, ", ");
      fprintf( fp, "\n          ");
      count++;
    }
    fprintf( fp,
	     ");\n"
	     "  Alignment = (%s);\n"
	     "}\n\n\n", 
	     create_alignment_list( cl->nb_allowed_align, cl->allowed_align ) );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_load_pc_classes() {
  FILE *fp;
  log_string("Reading pc classes");
  
  fclose( fpReserve );
  if ( ( fp = fopen( PC_CLASS_FILE, "r" ) ) == NULL ){
    bug( "Can't access pc class file: [%s]!", PC_CLASS_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas(fp);
  log_stringf(" %d classes found.", MAX_CLASS );

  // Create subclasses vectors
  int count_parent[MAX_CLASS];
  memset(count_parent,0,MAX_CLASS*sizeof(int));
  for ( int i = 0; i < MAX_CLASS; i++ )
    if ( class_table[i].parent_class != i )
      count_parent[class_table[i].parent_class]++;
  for ( int i = 0; i < MAX_CLASS; i++ ) {
    class_type *cl = &(class_table[i]);
    cl->nb_sub_classes = count_parent[i];
    cl->sub_classes = NULL;
    if ( cl->nb_sub_classes > 0 ) {
      if ( (cl->sub_classes = (int*)GC_MALLOC(sizeof(int)*cl->nb_sub_classes)) == NULL ) {
	bug("Can't allocate memory for subclass (%d)", i );
	exit(-1);
      }
      int index = 0;
      for ( int j = 0; j < MAX_CLASS; j++ ) {
	if (class_table[j].parent_class == i && i != j ) {
	  cl->sub_classes[index] = j;
	  index++;
	}
      }
    }
  }

  pcclassnotsaved = FALSE;

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}
void create_classes_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_CLASS = count;

  if ( ( class_table = (class_type*) GC_MALLOC( MAX_CLASS * sizeof(class_type)) ) == NULL )
    p_error("Can't allocate memory for class_table");

  // +1 for the dummy class in race_flags
  if (( classes_flags = (flag_type *)GC_MALLOC( sizeof(flag_type) * (MAX_CLASS+1) )) == NULL )
    p_error( "Can't allocate memory for classes_flags" );

  for ( int i = 0; i < MAX_CLASS+1; i++ )
    classes_flags[i].name = NULL;

  // Abilities
  for (int j = 0; j < MAX_ABILITY; j++ ){
    if ( ( ability_table[j].ability_level = (int *)GC_MALLOC(MAX_CLASS*sizeof(int)) ) == NULL ){
      bug("Can't allocate memory for ability_table (1)");
      exit(-1);
    }
    if ( ( ability_table[j].rating = (int *)GC_MALLOC(MAX_CLASS*sizeof(int)) ) == NULL ){
      bug("Can't allocate memory for ability_table (2)");
      exit(-1);
    }
  }

  // Groups
  for ( int j = 0; j < MAX_GROUP; j++ ){
    if ( ( group_table[j].rating = (int*)GC_MALLOC(sizeof(int)*MAX_CLASS) ) == NULL ){
      bug("Can't allocate memory group_table (2)");
      exit(-1);
    }
    for ( int i = 0; i < MAX_CLASS; i++ )
      group_table[j].rating[i] = -1;
  }

  // Title
  if ( ( title_table = (const char****)GC_MALLOC(sizeof(char***)*MAX_CLASS) ) == NULL ){
    bug("Can't allocate memory for title_table (1)");
    exit(-1);
  }
  for ( int j = 0; j < MAX_CLASS; j++ ){
    if ( ( title_table[j] = (const char***)GC_MALLOC(sizeof(char**)*(MAX_LEVEL+1)) ) == NULL ){
      bug("Can't allocate memory for title_table (2)");
      exit(-1);
    }
    for ( int i = 0; i < MAX_LEVEL+1; i++ ){
      if ( ( title_table[j][i] = (const char**)GC_MALLOC(sizeof(char*)*2) ) == NULL ){
	bug("Can't allocate memory for title_table (3)");
	exit(-1);
      }
      title_table[j][i][0] = NULL;
      title_table[j][i][1] = NULL;
    }
  }
  
  // Pose
  if ( ( pose_table = (const char****)GC_MALLOC(sizeof(char***)*MAX_CLASS ) ) == NULL ){
    bug("Can't allocate memory for pose_table(1)");
    exit(-1);
  }
  MAX_POSE = 17; // artificially set to 17  
  for ( int j = 0; j < MAX_CLASS; j++ ){
    if ( ( pose_table[j] = (const char***)GC_MALLOC(sizeof(char**)*MAX_POSE ) ) == NULL ){
      bug("Can't allocate memory for pose_table(2)");
      exit(-1);
    }
    for ( int i = 0; i < MAX_POSE; i++ ){
      if ( ( pose_table[j][i] = (const char**)GC_MALLOC(sizeof(char*)*2 ) ) == NULL ){
	bug("Can't allocate memory for pose_table(3)");
	exit(-1);
      }
      pose_table[j][i][0] = NULL;
      pose_table[j][i][1] = NULL;
    }
  }

  for (int i = 0; i < MAX_CLASS; i++)
    for (int j = 0; j < MAX_ABILITY; j++){
      ability_table[j].ability_level[i] = LEVEL_IMMORTAL;
      ability_table[j].rating[i] = 0;
    }

  done = TRUE;
}
void parse_classes_alignment( DATAData *align, class_type *cl ) {
  ValueList *list = align->value->eval().asList();

  cl->nb_allowed_align = list->size;
  if ( cl->nb_allowed_align == 0 )
    p_error("No alignment for class %s", cl->name );
  if ( ( cl->allowed_align = (align_info*) GC_MALLOC( cl->nb_allowed_align * sizeof(align_info) ) ) == NULL )
    p_error("Can't allocate memory allowed_align class_table (1)");
  
  for ( int i = 0; i < list->size; i++ ) {
    const char *buf = list->elems[i].asStr();
    if ( convert_etho_align_string_to_align_info( buf, cl->allowed_align[i] ) == -1 )
      p_error("Invalid alignment (%s) for class (%s)", buf, cl->name );
  }

  list->explicit_free();
}
void parse_classes_abilities( DATAData *ability, class_type *cl, const int clIndex ) {
  ValueList *list = ability->value->eval().asList();
  for ( int i = 0; i < list->size; i++ ) {
    ValueList *triplet = list->elems[i].asList();
    if ( triplet->size != 3 )
      p_error("Wrong number of elements in triplet <Level> <Rate> <Ability Name> for class %s", cl->name );
    const int level = triplet->elems[0].asInt();
    const int rate = triplet->elems[1].asInt();
    const char *s = triplet->elems[2].asStr();
    const int sn = ability_lookup( s );
    if ( sn < 0 )
      p_error("Invalid ability %s for class %s", s, cl->name );
    else {
      ability_type *sk = &(ability_table[sn]);
      if ( sk->ability_level[clIndex] != LEVEL_IMMORTAL || sk->rating[clIndex] != 0 )
	bug( "Ability: [%s] already set at level %3d for %2d trains (now: %d for %d)",
	     sk->name, sk->ability_level[clIndex], sk->rating[clIndex], level, rate );
      else {
	sk->ability_level[clIndex] = level;
	sk->rating[clIndex] = rate;
      }
    }

    triplet->explicit_free();
  }

  list->explicit_free();
}
void parse_classes_groups( DATAData *group, class_type *cl, const int clIndex ) {
  ValueList *list = group->value->eval().asList();
  for ( int i = 0; i < list->size; i++ ) {
    ValueList *couple = list->elems[i].asList();
    if ( couple->size != 2 )
      p_error("Wrong number of elements in couple <Rate> <Group Name> for class %s", cl->name );
    const int rate = couple->elems[0].asInt();
    const char *s = couple->elems[1].asStr();
    int gsn = group_lookup( s );
    if ( gsn <= 0 )
      bug("Invalid group [%s] for class [%s]", s, cl->name );
    else
      group_table[gsn].rating[clIndex] = rate;

    couple->explicit_free();
  }

  list->explicit_free();
}
void parse_classes_title( DATAData *title, class_type *cl, const int clIndex ) {
  ValueList *list = title->value->eval().asList();
  for ( int i = 0; i < list->size; i++ ) {
    ValueList *triplet = list->elems[i].asList();
    if ( triplet->size != 3 )
      p_error("Wrong number of elements in triplet <Level> <Male title> <Female title> for class %s", cl->name );
    int level = triplet->elems[0].asInt();
    if ( level < 0 || level > MAX_LEVEL )
      p_error("Wrong title level %d for class %s", level, cl->name );
    title_table[clIndex][level][0] = triplet->elems[1].asStr();
    title_table[clIndex][level][1] = triplet->elems[2].asStr();
    
    triplet->explicit_free();
  }

  list->explicit_free();
}
void parse_classes_pose( DATAData *pose, class_type *cl, const int clIndex ) {
  ValueList *list = pose->value->eval().asList();
  if ( list->size != MAX_POSE )
    p_error("Invalid number of pose for class %s, expecting %d", cl->name, MAX_POSE );
  for ( int i = 0; i < list->size; i++ ) {
    ValueList *couple = list->elems[i].asList();
    if ( couple->size != 2 )
      p_error("Wrong number of elements in couple <Male pose> <Female pose> for class %s", cl->name );
    pose_table[clIndex][i][0] = couple->elems[0].asStr();
    pose_table[clIndex][i][1] = couple->elems[1].asStr();

    couple->explicit_free();
  }

  list->explicit_free();
}
void nullify_class( const int i ) {
  class_type *cl = &(class_table[i]);
  cl->attr_prime = 0; // strength
  cl->weapon = OBJ_VNUM_SCHOOL_DAGGER; // dagger
  cl->ability_adept = 75;
  cl->thac0_00 = 30;
  cl->thac0_32 = 10;
  cl->hp_min = 1;
  cl->hp_max = 2;
  cl->type = CLASS_COMBAT;
  //cl->base_group = 0; // rom basics
  //cl->default_group = 0; // rom basics
  cl->base_group = str_dup(DEFAULT_GROUP_NAME); // rom basics
  cl->default_group = str_dup(DEFAULT_GROUP_NAME); // rom basics

  cl->parent_class = i; // parent is self
  cl->choosable = CLASS_CHOOSABLE_NEVER;
  cl->nb_allowed_align = 0;
  cl->allowed_align = NULL;
  cl->num_obj_list = 0;
  cl->obj_list = 0;
  for ( int i = 0; i < ABILITY_TYPE_COUNT; i++ )
    cl->casting_rule[i] = default_casting_rule;
}
void parse_classes( DATAData *cla ) {
  static int index = 0;
  class_type *cl = &(class_table[index]);
  cl->name = cla->value->eval().asStr();
  char buf[MAX_STRING_LENGTH];
  classes_flags[index].name = str_dup( cl->name );
  classes_flags[index].bit = 1 << index;
  classes_flags[index].settable = TRUE;
  log_stringf("Reading class %s", cl->name );

  nullify_class( index );

  // fields
  for ( int fieldCount = 0; fieldCount < cla->fields_count; fieldCount++ ) {
    DATAData *field = cla->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_WhoName: strcpy( cl->who_name, field->value->eval().asStr() ); break;
    case TAG_Attribute: {
      const char *s = field->value->eval().asStr();
      cl->attr_prime = flag_value_init( attr_flags, s ) - ATTR_STR;
      if ( cl->attr_prime < STAT_STR || cl->attr_prime > MAX_STATS )
	p_error("Invalid attributes %s for class %s", s, cl->name );
      break;
    }
    case TAG_Weapon: { 
      cl->weapon = field->value->eval().asInt(); 
      //if ( get_obj_index( cl->weapon ) == NULL )
      //bug("Unknown obj vnum:[%d] as weapon for class %s", cl->weapon, cl->name );
      break;
    }
    case TAG_Adept: cl->ability_adept = field->value->eval().asInt(); break;
    case TAG_Thac0_00: cl->thac0_00 = field->value->eval().asInt(); break;
    case TAG_Thac0_32: cl->thac0_32 = field->value->eval().asInt(); break;
    case TAG_HpMin: cl->hp_min = field->value->eval().asInt(); break;
    case TAG_HpMax: cl->hp_max = field->value->eval().asInt(); break;
    case TAG_Type: {
      //const char *s = field->value->eval().asStr();
      if ( ( cl->type = list_flag_value( field->value, class_type_flags ) ) == NO_FLAG )
	//p_error("Invalid class type (%s) for class (%s)", s, cl->name );
	p_error("Invalid class type for class (%s)", cl->name );
      break;
    }
    case TAG_CastingRule: {
      ValueList *list = field->value->eval().asList();
      for ( int i = 0; i < list->size; i++ ) {
	ValueList *triplet = list->elems[i].asList();
	if ( triplet->size != 3 ) {
	  bug("Wrong number of elements in casting rule #%d for class [%s]", i, cl->name );
	  continue;
	}
	const char *s = triplet->elems[0].asStr();
	int type = flag_value( ability_type_flags, s );
	if ( type == NO_FLAG ) {
	  bug("Invalid ability type %s in casting rule #%d for class [%s]", s, i, cl->name );
	  continue;
	}
	cl->casting_rule[type].highest = triplet->elems[1].asInt();
	cl->casting_rule[type].other = triplet->elems[2].asInt();
	if ( cl->casting_rule[type].highest <= cl->casting_rule[type].other ) {
	  bug("Invalid casting rule (%2d,%2d) -> creating (%2d,-1)",
	      cl->casting_rule[type].highest, cl->casting_rule[type].other,
	      cl->casting_rule[type].other );
	  cl->casting_rule[type].highest = cl->casting_rule[type].other;
	  cl->casting_rule[type].other = -1;
	}
	if ( DATA_VERBOSE > 2 ) {
	  printf("  Casting rule: %s  -> (%2d,%2d)\n\r", 
		 flag_string( ability_type_flags, type ), 
		 cl->casting_rule[type].highest, cl->casting_rule[type].other );
	}
      }
      break;
    }
    case TAG_BaseGroup:
      cl->base_group = field->value->eval().asStr();
      if ( group_lookup(cl->base_group) < 0 )
	p_error("Invalid base group %s for class %s", cl->base_group, cl->name );
      break;
    case TAG_DefaultGroup:
      cl->default_group = field->value->eval().asStr();
      if ( group_lookup(cl->default_group) < 0 )
	p_error("Invalid default group %s for class %s", cl->default_group, cl->name );
      break;
    case TAG_Parent: {
      const char *s = field->value->eval().asStr();
      if ( !str_cmp( s, "none" ) )
	cl->parent_class = index;
      else {
	cl->parent_class = class_lookup(s, TRUE, index);
	if ( cl->parent_class < 0 ) {
	  bug("Invalid parent class [%s] for class [%s], assuming no parent class.", s, cl->name );
	  cl->parent_class = index;
	}
      }
      break;
    }
    case TAG_Obj: {
      ValueList *list = field->value->eval().asList();
      cl->num_obj_list = list->size;
      if ( ( cl->obj_list = (int*)GC_MALLOC( cl->num_obj_list*sizeof(int) ) ) == NULL )
	p_error("Can't allocate memory for class obj list.");
      for ( int i = 0; i < list->size; i++ ) {
	cl->obj_list[i] = list->elems[i].asInt();
      }

      list->explicit_free();
    }
    case TAG_Choosable: {
      const char *s = field->value->eval().asStr();
      if ( ( cl->choosable = flag_value( class_choosable_flags, s ) ) == NO_FLAG ) {
	bug("Invalid choosable (%s) for class [%s], assuming NEVER", cl->name, s );
	cl->choosable = CLASS_CHOOSABLE_NEVER;
      }
      break;
    }
      // Other
    case TAG_Abilities: parse_classes_abilities( field, cl, index ); break;
    case TAG_Groups: parse_classes_groups( field, cl, index ); break;
    case TAG_Title: parse_classes_title( field, cl, index ); break;
    case TAG_Pose: parse_classes_pose( field, cl, index ); break;
    case TAG_Alignment: parse_classes_alignment( field, cl ); break;
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
 
  index++;
}


//*****************************************************************************************************
//************************************************* Time **********************************************
void save_time() {
  log_stringf("Saving time, weather informations and pulses");

  FILE *fp;
  fclose(fpReserve);
  if ( ( fp = fopen( TIME_FILE, "w" ) ) == NULL ) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Can't find time file: [%s]", TIME_FILE );
    return;
  }

  char moonsStr[MAX_STRING_LENGTH];
  moonsStr[0] = '\0';
  for ( int i = 0; i < NBR_MOON; i++ ) {
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "( %d, %d )%s", moon_info[i].ph_t, moon_info[i].po_t,(i<NBR_MOON-1)?", ":"");
    strcat( moonsStr, buf );
  }
  fprintf( fp,
	   "Time 0 {\n"
	   "  Hour = %d;\n"
	   "  Date = ( %d, %d, %d );\n"
	   "  Change = %d;\n"
	   "  mmHg = %d;\n"
	   "  Moons = (%s);\n"
	   "  Area = %d;\n"
	   "  Mobile = %d;\n"
	   "  Violence = %d;\n"
	   "  Point = %d;\n"
	   "  Auction = %d;\n"
	   "  Hint = %d;\n"
	   "  ArenaStart = %d;\n"
	   "  Arena = %d;\n"
	   "  Room = %d;\n"
	   "  Quake = %d;\n"
	   "  DoubleXp = %d;\n"
	   "  DoubleXpDuration = %d;\n"
	   "}\n",
	   time_info.hour, time_info.day, time_info.month, time_info.year,
	   weather_info.change, weather_info.mmhg, moonsStr,
	   pulse_area, pulse_mobile, pulse_violence, pulse_point, pulse_auction,
	   pulse_hints, pulse_start_arena, pulse_arena, pulse_room, pulse_quake,
	   pulse_double_xp, pulse_double_xp_duration
	   );
  fpReserve = fopen( NULL_FILE, "r" );
  fclose(fp);
}

void load_time() {
  init_time_weather(); // initialize value

  log_stringf("Reading time, weather informations and pulses");

  FILE *fp;
  if ( ( fp = fopen( TIME_FILE, "r" ) ) == NULL ) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Can't find time file: [%s]", TIME_FILE );
    return;
  }

  parse_datas( fp );

  fpReserve = fopen( NULL_FILE, "r" );
  fclose(fp);
}

void init_time_weather() {
  long lhour, lday, lmonth;
  
  lhour		= (current_time - 650336715)
    / (PULSE_TICK / PULSE_PER_SECOND);
  time_info.hour	= lhour  % 24;
  lday		= lhour  / 24;
  time_info.day	= lday   % NBR_DAYS_IN_MONTH;
  lmonth		= lday   / NBR_DAYS_IN_MONTH;
  time_info.month	= lmonth % NBR_MONTHS_IN_YEAR;
  time_info.year	= lmonth / NBR_MONTHS_IN_YEAR;
  
  if ( time_info.hour <  5 ) weather_info.sunlight = SUN_DARK;
  else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
  else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
  else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
  else                            weather_info.sunlight = SUN_DARK;
  
  weather_info.change	= 0;
  weather_info.mmhg	= 960;
  if ( time_info.month >= 7 && time_info.month <= 12 )
    weather_info.mmhg += number_range( 1, 50 );
  else
    weather_info.mmhg += number_range( 1, 80 );
  
  if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
  else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
  else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
  else                                  weather_info.sky = SKY_CLOUDLESS;
}

void parse_time( DATAData *time ) {
  // time->value is not revelant unless == -1
  if ( time->value->eval().asInt() == -1 ) {
    log_stringf("Using default time and weather value.");
    return;
  }
  // fields
  for ( int fieldCount = 0; fieldCount < time->fields_count; fieldCount++ ) {
    DATAData *field = time->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Hour: time_info.hour = field->value->eval().asInt(); break;
    case TAG_Date: {
      ValueList *list = field->value->eval().asList();
      bool error = FALSE;
      int      lhour		= (current_time - 650336715)
	/ (PULSE_TICK / PULSE_PER_SECOND);
      int lday, lmonth, lyear;
      if ( list->size != 3 ) {
	bug("Wrong number of elements for date, assuming current_time %ld", current_time );
	break; // init values are already set in init_time_weather
      }
      if ( !error ) {
	lday = list->elems[0].asInt();
	lmonth = list->elems[1].asInt();
	lyear = list->elems[2].asInt();
	if ( lday < 0 || lday > 35 
	     || lmonth < 0 || lmonth > 17
	     || lyear < 0 ) {
	  bug("Invalid day, month or year, assuming current_time %ld", current_time );
	  break;
	}
	time_info.day	= lday;
	time_info.month	= lmonth;
	time_info.year	= lmonth;
      }
      list->explicit_free();
      break;
    }
    case TAG_Change: weather_info.change = field->value->eval().asInt(); break;
    case TAG_mmHg: weather_info.mmhg = field->value->eval().asInt(); break;
    case TAG_Moons: {
      ValueList *list = field->value->eval().asList();
      int size = list->size;
      if ( size != NBR_MOON ) {
	bug("Wrong number of moons, number of moons must be %d", NBR_MOON );
	size = UMIN( size, NBR_MOON );
      }
      for ( int i = 0; i < size; i++ ) {
	ValueList *couple = list->elems[i].asList();
	if ( couple->size != 2 ) {
	  bug("Wrong number of elements in moon couple <Phase Num> <Position Num>");
	  break;
	}
	moon_info[i].ph_t = couple->elems[0].asInt();
	moon_info[i].po_t = couple->elems[1].asInt();
	if ( DATA_VERBOSE > 1 ) {
	  printf("Moon %d: phase %d  position %d\n\r",
		 i, moon_info[i].ph_t, moon_info[i].po_t );
	}

	couple->explicit_free();
      }

      list->explicit_free();
      break;
    }
    case TAG_Area: pulse_area = field->value->eval().asInt(); break;
    case TAG_Mobile: pulse_mobile = field->value->eval().asInt(); break;
    case TAG_Violence: pulse_violence = field->value->eval().asInt(); break;
    case TAG_Point: pulse_point = field->value->eval().asInt(); break;
    case TAG_Auction: pulse_auction = field->value->eval().asInt(); break;
    case TAG_Hint: pulse_hints = field->value->eval().asInt(); break;
    case TAG_ArenaStart: pulse_start_arena = field->value->eval().asInt(); break;
    case TAG_Arena: pulse_arena = field->value->eval().asInt(); break;
    case TAG_Room: pulse_room = field->value->eval().asInt(); break;
    case TAG_Quake: pulse_quake = field->value->eval().asInt(); break;
    case TAG_DoubleXp: pulse_double_xp = field->value->eval().asInt(); break;
    case TAG_DoubleXpDuration:
      pulse_double_xp_duration = field->value->eval().asInt();
      if ( pulse_double_xp_duration > 0 )
	double_xp = TRUE;
      break;
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
  if ( DATA_VERBOSE > 0 ) {
    printf("Time: %d ( %d, %d, %d )  change: %d  mmHg: %d\n\r",
	   time_info.hour, time_info.day, time_info.month, time_info.year,
	   weather_info.change, weather_info.mmhg );
  }

  // use values to initialize weather information
  if ( time_info.hour <  5 )      weather_info.sunlight = SUN_DARK;
  else if ( time_info.hour <  6 ) weather_info.sunlight = SUN_RISE;
  else if ( time_info.hour < 19 ) weather_info.sunlight = SUN_LIGHT;
  else if ( time_info.hour < 20 ) weather_info.sunlight = SUN_SET;
  else                            weather_info.sunlight = SUN_DARK;
  
  if ( weather_info.mmhg <=  980 )      weather_info.sky = SKY_LIGHTNING;
  else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
  else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
  else                                  weather_info.sky = SKY_CLOUDLESS;
}

//*****************************************************************************************************
//**********************************************  Hometown  *******************************************
void save_hometown() {
  log_stringf("Saving hometown");
  FILE *fp;
  fclose(fpReserve);
  if ( ( fp = fopen( HOMETOWN_FILE, "w" ) ) == NULL ) {
    bug("Can't open hometown file [%s].", HOMETOWN_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_HOMETOWN; i++ ) {
    fprintf( fp, 
	     "Hometown %s {\n"
	     "  Recall = %d;\n"
	     "  Hall = %d;\n"
	     "  Donation = %d;\n"
	     "  Morgue = %d;\n"
	     "  School = %d;\n"
	     "}\n\n", 
	     quotify(hometown_table[i].name),
	     hometown_table[i].recall,
	     hometown_table[i].hall,
	     hometown_table[i].donation,
	     hometown_table[i].morgue,
	     hometown_table[i].school );
  }

  fpReserve = fopen( NULL_FILE, "r" );
  fclose(fp);
}
void load_hometown() {
  log_stringf("Reading hometown");
  FILE *fp;
  fclose(fpReserve);
  if ( ( fp = fopen( HOMETOWN_FILE, "r" ) ) == NULL ) {
    bug("Can't open hometown file [%s].", HOMETOWN_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas(fp);
  if ( MAX_HOMETOWN == 0 ) {
    bug("No hometown found!!");
    exit(-1);
  }
  log_stringf(" %d hometown found.", MAX_HOMETOWN );

  fpReserve = fopen( NULL_FILE, "r" );
  fclose(fp);
}
void create_hometown_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_HOMETOWN = count;
  if ( ( hometown_table = (hometown_type*)GC_MALLOC(sizeof(hometown_type)*MAX_HOMETOWN) ) == NULL )
    p_error("Can't allocate memory for hometown table." );
  done = TRUE;
}
void parse_hometown( DATAData *homeD) {
  static int index = 0;

  hometown_type *home = &(hometown_table[index]);
  home->name = str_dup( homeD->value->eval().asStr() );
  home->recall = DEFAULT_RECALL;
  home->hall = DEFAULT_HALL;
  home->morgue = DEFAULT_MORGUE;
  home->school = DEFAULT_SCHOOL;
  home->donation = DEFAULT_DONATION;
  home->choosable = FALSE;

  if ( DATA_VERBOSE > 0 ) {
    printf("Hometown name: %s\n\r", home->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < homeD->fields_count; fieldCount++ ) {
    DATAData *field = homeD->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Recall: {
      int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *pRoom = get_room_index( vnum );
      if ( pRoom == NULL ) {
	bug("Recall room vnum %d for hometown [%s] doesn't exist, setting to 1", vnum, home->name );
	break;
      }
      home->recall = vnum;
      break;
    }
    case TAG_Hall: {
      int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *pRoom = get_room_index( vnum );
      if ( pRoom == NULL ) {
	bug("Hall room vnum %d for hometown [%s] doesn't exist, setting to 1", vnum, home->name );
	break;
      }
      home->hall = vnum;
      break;
    }
    case TAG_Morgue: {
      int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *pRoom = get_room_index( vnum );
      if ( pRoom == NULL ) {
	bug("Morgue room vnum %d for hometown [%s] doesn't exist, setting to 1", vnum, home->name );
	break;
      }
      home->morgue = vnum;
      break;
    }
    case TAG_Donation: {
      int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *pRoom = get_room_index( vnum );
      if ( pRoom == NULL ) {
	bug("Donation room vnum %d for hometown [%s] doesn't exist, setting to 1", vnum, home->name );
	break;
      }
      home->donation = vnum;
      break;
    }
    case TAG_School: {
      int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *pRoom = get_room_index( vnum );
      if ( pRoom == NULL ) {
	bug("Mud school room vnum %d for hometown [%s] doesn't exist, setting to 1", vnum, home->name );
	break;
      }
      home->school = vnum;
      break;
    }
    case TAG_Choosable: home->choosable = field->value->eval().asInt(); break;
      // Other
    default: p_error("Invalid Tag: [%s]", tagName ); break;
    }
  }

  index++;
}

//*****************************************************************************************************
//**********************************************  School  *********************************************

void save_schools() {
  FILE *fp;

  log_string("Saving magic school table");

  fclose( fpReserve );
  if ( ( fp = fopen( SCHOOL_FILE, "w" ) ) == NULL ){
    bug( "Can't access school file: [%s]!", SCHOOL_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  for ( int i = 0; i < MAX_SCHOOL; i++ ) {
    char schoolList[MAX_STRING_LENGTH];
    magic_school_type *sc = &(magic_school_table[i]);

    schoolList[0] = '\0';
    for ( int j = 0; j < sc->nb_spells; j++ ) {
      char buf2[MAX_STRING_LENGTH];
      int sn = sc->spells_list[j];
      if ( sn > 0 )
	sprintf(buf2,"%s", quotify(ability_table[sn].name,"'"));
      else
	sprintf(buf2,"'none'");
      strcat( schoolList, buf2 );
      if ( j < sc->nb_spells-1 )
	strcat( schoolList, ", ");
      strcat( schoolList, "\n           ");
    }
    fprintf( fp,
	     "School %s {\n"
	     "  List = (%s);\n"
	     "}\n\n",
	     quotify(sc->name,"'"),
	     schoolList );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  schoolnotsaved = FALSE;
}

void load_schools() {
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  log_string("Reading magic school table");
  
  fclose( fpReserve );
  if ( ( fp = fopen( SCHOOL_FILE, "r" ) ) == NULL ){
    bug( "Can't access school file: [%s]!", SCHOOL_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas(fp);
  log_stringf(" %d schools found.", MAX_SCHOOL );

  // Assign ability school field to the right value
  for ( int i = 0; i < MAX_SCHOOL; i++ ) {
    magic_school_type *sc = &(magic_school_table[i]);
    for ( int j = 0; j < sc->nb_spells; j++ ) {
      int sn = sc->spells_list[j];
      if ( sn > 0 ) {
	for ( int k = 0; k < i; k++ )
	  if ( ability_in_school( k, sn ) >= 0 ) {
	    bug("Ability [%s] in school [%s] and [%s]",
		ability_table[sn].name, 
		sc->name,
		magic_school_table[k].name );
	    sc->spells_list[j] = -1;
	  }
      }
      if ( sn > 0 )
	ability_table[sn].school = i; // assign school
    }
  }
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  schoolnotsaved = FALSE;
}

void create_school_table( const int count ) {
  static bool done = FALSE;
  if ( done )
    return;

  MAX_SCHOOL = count;
  if ( ( magic_school_table = (magic_school_type*) GC_MALLOC( MAX_SCHOOL * sizeof(magic_school_type) ) ) == NULL ){
    bug("Can't allocate memory school_table (1)");
    exit(-1);
  }
  done = TRUE;
}

void parse_school_list( DATAData *contains, magic_school_type *sc ) {
  ValueList *list = contains->value->eval().asList();
  
  sc->nb_spells = list->size;
  if ( sc->nb_spells <= 0 )
    p_error("Invalid number of ability for school %s", sc->name );

  if ( ( sc->spells_list = (int*)GC_MALLOC(sizeof(int)*sc->nb_spells) ) == NULL )
    p_error("Can't allocate memory school_table (2)");
  
  for ( int i = 0; i < list->size; i++ ) {
    const char *s = list->elems[i].asStr();
    int sn = ability_lookup( s );
    sc->spells_list[i] = sn;
    if ( sn < 0 ) {
      bug("Invalid ability [%s] in school [%s]", s, sc->name );
      sc->spells_list[i] = -1;
    }
    for ( int j = 0; j < i; j++ )
      if ( sn == sc->spells_list[j] ) {
	bug("Find duplicate ability [%s] in school [%s]", s, sc->name );
	sc->spells_list[i] = -1;
      }
  }

  list->explicit_free();
}

void parse_school( DATAData *school ) {
  static int index = 0;
  magic_school_type *sc = &(magic_school_table[index]);
  sc->name = str_dup(school->value->eval().asStr());
  if ( DATA_VERBOSE > 0 ) {
    printf("  School %s\n\r", sc->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < school->fields_count; fieldCount++ ) {
    DATAData *field = school->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
      // Spell List
    case TAG_List: parse_school_list( field, sc ); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  index++;
}


//*****************************************************************************************************
//*******************************************  Super Race  ********************************************

void save_superraces() {
  FILE *fp;

  log_string("Saving super race table");

  fclose( fpReserve );
  if ( ( fp = fopen( SUPER_RACE_FILE, "w" ) ) == NULL ){
    bug( "Can't access super race file: [%s]!", SUPER_RACE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  for ( int i = 0; i < MAX_SUPERRACE; i++ ) {
    char list[MAX_STRING_LENGTH];
    super_pc_race_type *race = &(super_pc_race_table[i]);

    list[0] = '\0';
    for ( int j = 0; j < race->nb_pc_race; j++ ) {
      strcat( list, race_table[race->pc_race_list[j]].name );
      if ( j < race->nb_pc_race-1 ) {
	strcat( list, ", ");
	if ( (j+1) % 4 == 0 )
	  strcat( list, "\n          ");
      }
    }
    fprintf( fp,
	     "SuperRace %s {\n"
	     "  List = (%s);\n"
	     "}\n\n",
	     quotify(race->name),
	     list );
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  superracenotsaved = FALSE;
}

void load_superraces() {
  FILE *fp;
  char buf[MAX_STRING_LENGTH];

  log_string("Reading super race table");
  
  fclose( fpReserve );
  if ( ( fp = fopen( SUPER_RACE_FILE, "r" ) ) == NULL ){
    bug( "Can't access super race file: [%s]!", SUPER_RACE_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas(fp);
  log_stringf(" %d super races found.", MAX_SUPERRACE );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  // Assign pc_race_type->super_race
  for ( int i = 0; i < MAX_SUPERRACE; i++ )
    for ( int j = 0; j < super_pc_race_table[i].nb_pc_race; j++ )
      pc_race_table[super_pc_race_table[i].pc_race_list[j]].super_race = i;
  for ( int i = 0; i < MAX_PC_RACE; i++ )
    if ( pc_race_table[i].super_race == -1 )
      bug("Race [%s] doesn't have a super race", race_table[i].name );

  superracenotsaved = FALSE;
}

void create_super_race_table( const int count ) {
  static bool done = FALSE;
  if ( done )
    return;

  MAX_SUPERRACE = count;
  if ( ( super_pc_race_table = (super_pc_race_type*) GC_MALLOC( MAX_SUPERRACE * sizeof(super_pc_race_type) ) ) == NULL ) {
    bug("Can't allocate memory super_pc_race_table (1)");
    exit(-1);
  }
  done = TRUE;
}

void parse_super_race_list( DATAData *contains, super_pc_race_type *race ) {
  ValueList *list = contains->value->eval().asList();

  race->nb_pc_race = list->size;
  if ( race->nb_pc_race <= 0 )
    p_error("Invalid number of race for super race %s", race->name );
  if ( ( race->pc_race_list = (int*)GC_MALLOC(sizeof(int)*race->nb_pc_race) ) == NULL )
    p_error("Can't allocate memory super_pc_race_table (2)");
  
  for ( int i = 0; i < list->size; i++ ) {
    const char *s = list->elems[i].asStr();
    int raceId = race_lookup( s );
    race->pc_race_list[i] = raceId;
    if ( raceId < 0 ) {
      bug("Invalid race [%s] in super race [%s]", s, race->name );
      race->pc_race_list[i] = -1;
    }
    else if ( !race_table[raceId].pc_race ) {
      bug("Race [%s] is not a pc race for super race [%s]", s, race->name );
      race->pc_race_list[i] = -1;
    }
    for ( int j = 0; j < i; j++ )
      if ( raceId == race->pc_race_list[j] ) {
	bug("Find duplicate race [%s] in super race [%s]", s, race->name );
	race->pc_race_list[i] = -1;
      }
  }

  list->explicit_free();
}

void parse_super_race( DATAData *raceD ) {
  static int index = 0;
  super_pc_race_type *race = &(super_pc_race_table[index]);
  race->name = str_dup(raceD->value->eval().asStr());
  if ( DATA_VERBOSE > 0 ) {
    printf("  Super race %s\n\r", race->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < raceD->fields_count; fieldCount++ ) {
    DATAData *field = raceD->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
      // Race List
    case TAG_List: parse_super_race_list( field, race ); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  index++;
}


//***********************************************************************************************
//************************************** INTEGRITY **********************************************

// SinaC 2003, check integrity of structures
void check_struct_integrity() {
  log_stringf("Checking datas integrity...");
  // Check classes weapon
  for ( int i = 0; i < MAX_CLASS; i++ ) {
    class_type *cl = &(class_table[i]);
    if ( get_obj_index( cl->weapon ) == NULL )
      bug("Unknown obj vnum:[%d] as weapon for class [%s]", cl->weapon, cl->name );
  }

  // Each race available to at least one god
  bool race_checked[MAX_PC_RACE];
  memset( race_checked, 0, sizeof(bool)*MAX_PC_RACE );
  for ( int i = 0; i < MAX_PC_RACE; i++ ) { // for each race
    pc_race_type *race = &(pc_race_table[i]);
    bool found = FALSE;
    for ( int j = 0; j < MAX_GODS; j++ ) {
      god_data *god = &(gods_table[j]);
      for ( int k = 0; k < god->nb_allowed_race; k++ )
	if ( god->allowed_race[k] == i ) {
	  found = TRUE;
	  break;
	}
    }
    if ( !found ) {
      bug("Race [%s] is not available to any god", race->name );
      race_checked[i] = TRUE;
      if ( race->type != RACE_NOTAVAILABLE )
	exit(-1);
    }
  }

  // Each class available to at least one god
  bool class_checked[MAX_PC_RACE];
  memset( class_checked, 0, sizeof(bool)*MAX_CLASS );
  for ( int i = 0; i < MAX_CLASS; i++ ) { // for each race
    class_type *cl = &(class_table[i]);
    bool found = FALSE;
    for ( int j = 0; j < MAX_GODS; j++ ) {
      god_data *god = &(gods_table[j]);
      if ( god->allowed_class & ( 1 << i ) ) {
	found = TRUE;
      }
    }
    if ( !found ) {
      bug("Class [%s] is not available to any god", cl->name );
      class_checked[i] = TRUE;
      if ( cl->choosable != CLASS_CHOOSABLE_NEVER )
	exit(-1);
    }
  }

  // Check for every possible combinaison of race/class/align if a god can be picked
  //  for each race
  //    for each class available to this race
  //      for each align valid to class/race
  //        for each god valid to align/class/race
  for ( int i = 0; i < MAX_PC_RACE; i++ ) { // for each race
    pc_race_type *race = &(pc_race_table[i]);
    if ( race_checked[i] )
      continue;
    if ( race->allowed_class == 0 ) {
      bug("Race [%s] doesn't have any available class.", race->name );
      continue;
    }
    for ( int j = 0; j < MAX_CLASS; j++ ) // for each class available to race
      if ( ( ( 1 << j ) & race->allowed_class ) && !class_checked[j] ) {
	class_type *cl = &(class_table[j]);
	bool alignFound = FALSE;
	for ( int k = 0; k < race->nb_allowed_align; k++ ) // for each align available to race/class
	  for ( int l = 0; l < cl->nb_allowed_align; l++ )
	    if ( race->allowed_align[k].alignment == cl->allowed_align[l].alignment
		 && race->allowed_align[k].etho == cl->allowed_align[l].etho ) {
	      alignFound = TRUE;
	      bool godFound = FALSE;
	      for ( int m = 0; m < MAX_GODS; m++ ) {
		god_data *god = &(gods_table[m]);
		for ( int n = 0; n < god->nb_allowed_race; n++ )
		  if ( god->allowed_race[n] == i
		       && ( god->allowed_class & ( 1 << j ) ) )
		    for ( int o = 0; o < god->nb_allowed_align; o++ )
		      if ( race->allowed_align[k].alignment == god->allowed_align[o].alignment
			   && race->allowed_align[k].etho == god->allowed_align[o].etho ) {
			godFound = TRUE;
			//			log_stringf("Race: %s  Class: %s  Align: %s  God: %s",
			//				    race->name,
			//				    cl->name,
			//				    etho_align_name(race->allowed_align[k].etho, race->allowed_align[k].alignment),
			//				    god->name );
		      }
	      }
	      if ( !godFound ) {
		bug("Race [%s] | Class [%s] | Align [%s] doesn't have a valid god.",
		    race->name, cl->name,
		    etho_align_name(race->allowed_align[k].etho, race->allowed_align[k].alignment) );
		continue;
	      }
	    }
	if ( !alignFound ) {
	  bug("Race [%s] | Class [%s]  doesn't have common alignment.", race->name, cl->name );
	  continue;
	}
      }
  }

  // Check if each components can be found at least once in the mud
  bool check_component[MAX_BREW_COMPONENT];
  memset(check_component,FALSE,sizeof(bool)*MAX_BREW_COMPONENT);

  int nMatch=0;
  OBJ_INDEX_DATA *pObjIndex;
  for(int vnum=0;nMatch<top_obj_index;vnum++)
    if ( (pObjIndex=get_obj_index(vnum)) != NULL) {
      nMatch++;
      if ( pObjIndex->item_type == ITEM_COMPONENT )
	for ( int i = 0; i < 5; i++ )
	  if ( pObjIndex->value[i] != NO_FLAG
	       && pObjIndex->value[i] >= 0
	       && pObjIndex->value[i] < MAX_BREW_COMPONENT )
	    check_component[pObjIndex->value[i]] = TRUE;
    }
  for ( int i = 0; i < MAX_BREW_COMPONENT; i++ )
    if ( !check_component[i] )
      bug("Component [%s] cannot be found in the world", flag_string( brew_component_flags, i ) );
  log_stringf("Done.");
}
