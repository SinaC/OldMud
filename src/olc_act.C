/**************************************************************************
 *  File: olc_act.c                                                        *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 **************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"
#include "recycle.h"
#include "lookup.h"

// Added by SinaC 2001
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "clan.h"
#include "special.h"
#include "fight.h"
#include "act_move.h"
#include "string.h"
#include "olc_value.h"
#include "names.h"
#include "scrhash.h"
#include "dbscript.h"
#include "interp.h"
#include "act_scri.h"
#include "act_info.h"
#include "const.h"
#include "olc_act.h"
#include "bit.h"
#include "mem.h"
#include "data_edit.h"
#include "faction.h"
#include "utils.h"
#include "affects.h"
#include "language.h"
#include "statistics.h"


// SinaC 2003
bool general_add_affect( CHAR_DATA *ch,
			 AFFECT_DATA *&starting_list,
			 const int entity_type, // same value as entity: CHAR_ENTITY, OBJ_ENTITY, ROOM_ENTITY
			 OBJ_INDEX_DATA *pObjIndex, // NULL if not an obj
			 const int level,
			 const char *argument );
bool general_del_affect( CHAR_DATA *ch, AFFECT_DATA *&start_list, const char *argument );
bool general_set_affect( CHAR_DATA *ch, AFFECT_DATA *&start_list, const char *argument );
void general_show_affects( CHAR_DATA *ch, AFFECT_DATA *afList, const char *argument );


char * mprog_type_to_name ( int type );

/* Return TRUE if area changed, FALSE if not. */
#define REDIT( fun )		bool fun( CHAR_DATA *ch, const char *argument )
#define OEDIT( fun )		bool fun( CHAR_DATA *ch, const char *argument )
#define MEDIT( fun )		bool fun( CHAR_DATA *ch, const char *argument )
#define AEDIT( fun )		bool fun( CHAR_DATA *ch, const char *argument )


/*
 * Prototypes for deletion
 */
void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset );
void unlink_obj_index( OBJ_INDEX_DATA *pObj );
void unlink_room_index( ROOM_INDEX_DATA *pRoom );
void unlink_mob_index( MOB_INDEX_DATA *pMob );

struct olc_help_type {
  char *command;
  const void *structure;
  char *desc;
};



bool show_version( CHAR_DATA *ch, const char *argument )
{
  send_to_char( VERSION, ch );
  send_to_char( "\n\r", ch );
  send_to_char( AUTHOR, ch );
  send_to_char( "\n\r", ch );
  send_to_char( DATE, ch );
  send_to_char( "\n\r", ch );
  send_to_char( CREDITS, ch );
  send_to_char( "\n\r", ch );

  return FALSE;
}    

// Added by SinaC 2000
void init_help_table() {
  /*
   * This table contains help commands and a brief description of each.
 * ------------------------------------------------------------------
 */
  extern const struct olc_help_type *help_table;
  static const struct olc_help_type help_table_source[] =
  {
    {	"area",		area_flags,	 "Area attributes."		 },
    {	"room",		room_flags,	 "Room attributes."		 },
    {	"sector",	sector_flags,	 "Sector types, terrain."	 },
    {	"exit",		exit_flags,	 "Exit types."			 },
    {	"type",		type_flags,	 "Types of objects."		 },
    {	"extra",	extra_flags,	 "Object attributes."		 },
    {	"wear",		wear_flags,	 "Where to wear object."	 },
    {	"spec",		spec_table,	 "Available special programs." 	 },
    {	"program",	NULL,		 "Available programs."		},
    {	"sex",		sex_flags,	 "Sexes."			 },
    {	"act",		act_flags,	 "Mobile attributes."		 },
    {	"affect",	affect_flags,	 "Mobile affects."		 },
    // Added by SinaC 2001
    {	"affect2",	affect2_flags,	 "Mobile affects second part."		 },
    {	"wear-loc",	wear_loc_flags,	 "Where mobile wears object."	 },
    {	"ability",	ability_table,	 "Names of current abilities." 	 },
    {	"container",	container_flags, "Container status."		 },

    /* ROM specific bits: */
    /* Oxtal */
    {   "where",        afto_type,       "Where an affect will apply."  },
    {   "attribs",      attr_flags,      "Which attribute an affect will touch." },
    {   "ops",          ops_flags,       "What an affect will do." },

    {	"armor",	ac_type,	 "Ac for different attacks."	 },
    {	"form",		form_flags,	 "Mobile body form."	         },
    {	"part",		part_flags,	 "Mobile body parts."		 },

    // SinaC 2003, same tables
    {	"imm",		irv_flags,	 "Mobile immunity."		 },
    {	"res",		irv_flags,	 "Mobile resistance."	         },
    {	"vuln",		irv_flags,	 "Mobile vulnerability."	 },
    //    {	"imm",		imm_flags,	 "Mobile immunity."		 },
    //    {	"res",		res_flags,	 "Mobile resistance."	         },
    //    {	"vuln",		vuln_flags,	 "Mobile vulnerability."	 },

    {	"off",		off_flags,	 "Mobile offensive behaviour."	 },
    // Modified by SinaC 2000
    {	"size",		size_flags,	 "Mobile."			 },
    {   "position",     position_flags,  "Mobile positions."             },
    {   "wclass",       weapon_class,    "Weapon class."                 }, 
    {   "wtype",        weapon_type2,    "Special weapon type."          },
    {	"portal",	portal_flags,	 "Portal types."		 },
    {	"furniture",	furniture_flags, "Furniture types."		 },
    {   "liquid",	liq_table,	 "Liquid types."		 },
    {	"weapon",	attack_table,	 "Weapon types."		 },

    // Added by SinaC 2000 for object restrictions
    {   "restriction",  restr_flags,     "Object restrictions."          },
    // Added by SinaC 2000 for mobile classes
    {   "classes",      classes_flags,   "Mobile classes."               },
    // Added by SinaC 2001 for mobiles races
    {   "race",         races_flags,     "Mobile race"                   },
    // Added by SinaC 2001 for material
    {   "material",     material_table,  "Object Material"               },
    // SinaC 2003
    {   "target",       target_flags,    "Ability Target"                },
    {   "mobuse",       mob_use_flags,   "Mob Use"                       },
    {   "racetype",     race_type_flags, "Race Type"                     },
    {   "log",          log_flags,       "Command log"                   },
    {   "classtype",    class_type_flags, "Class type"                   },
    {   "classchoosable", class_choosable_flags, "Class choosable"       },
    {   "abilitytype",  ability_type_flags, "Ability type"               },
    {   "affectflag",   affect_data_flags, "Affect flags"                },
    {   "component",    brew_component_flags, "Component list"           },
    {   "afto",         afto_type,       "Affect target"                 },
    {   "roomattr",     room_attr_flags, "Room affect attributes"        },
    {   "language",     NULL,            "Racial languages"              },

    // Added by SinaC 2001 for disease
    //{   "disease",      disease_flags,   "Disease"                       },  removed by SinaC 2003
    {	NULL,		NULL,		 NULL				 }
  };

  help_table = help_table_source;
}

//void update_help_table( const char *name, flag_type *flag ) {
void update_help_table( const char *name, void *flag ) {
  int i;

  extern const struct olc_help_type *help_table;
  struct olc_help_type *l_help_table = (struct olc_help_type *) help_table;
  // it's the only place in the code where we modify it :)
  
  for ( i = 0; l_help_table[i].command != NULL; i++ )
    if ( !str_cmp( l_help_table[i].command, name ) )
      break;
  if ( l_help_table[i].command == NULL ) {
    bug("update_help_table: Invalid help table entry: %s", name );
    exit(-1);
  }
  l_help_table[i].structure = flag;
}

const struct olc_help_type *help_table;

/*****************************************************************************
 Name:		show_flag_cmds
 Purpose:	Displays settable flags and stats.
 Called by:     show_help(olc_act.c) addaffect(olc_act.c)
 ****************************************************************************/
void show_flag_cmds( CHAR_DATA *ch, struct flag_type *flag_table )
{
  char buf  [ MAX_STRING_LENGTH ];
  char buf1 [ MAX_STRING_LENGTH ];
  int  flag;
  int  col;
 
  buf1[0] = '\0';
  col = 0;

  if ( flag_table == NULL ){
    send_to_char("Doesn't exist!!!\n\r",ch);
    return;
  }

  // SinaC 2003.
  bool SHOW_COMPLETE = FALSE;
  if ( ch->desc != NULL
       && ch->desc->editor >= ED_ABILITY ) // if we are editing datas: show complete flag
    SHOW_COMPLETE = TRUE;

  for (flag = 0; flag_table[flag].name != NULL; flag++) {

//    log_stringf("%d) %s", flag, flag_table[flag].name);

    if ( flag_table[flag].settable || SHOW_COMPLETE ) { // SinaC 2003
      sprintf( buf, "%-19.18s", flag_table[flag].name );
      strcat( buf1, buf );
      if ( ++col % 4 == 0 )
	strcat( buf1, "\n\r" );
    }
  }
 
  if ( col % 4 != 0 )
    strcat( buf1, "\n\r" );

  send_to_char( buf1, ch );
  return;
}


/*****************************************************************************
 Name:		show_skill_cmds
 Purpose:	Displays all skill functions.
 		Does remove those damn immortal commands from the list.
 		Could be improved by:
 		(1) Adding a check for a particular class.
 		(2) Adding a check for a level range.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_skill_cmds( CHAR_DATA *ch, int type, int tar )
{
  char buf  [ MAX_STRING_LENGTH ];
  char buf1 [ MAX_STRING_LENGTH*2 ];
  int  sn;
  int  col;
 
  buf1[0] = '\0';
  col = 0;
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if ( !ability_table[sn].name )
      break;

    if ( !str_cmp( ability_table[sn].name, "reserved" )
	 // Modified by SinaC 2001
	 //|| ability_table[sn].spell_fun == spell_null )
	 || ability_table[sn].type != type )
      continue;

    if ( tar < 0 || ability_table[sn].target == tar ) {
      sprintf( buf, "%-19.18s", ability_table[sn].name );
      strcat( buf1, buf );
      if ( ++col % 4 == 0 )
	strcat( buf1, "\n\r" );
    }
  }
 
  if ( col % 4 != 0 )
    strcat( buf1, "\n\r" );

  send_to_char( buf1, ch );
  return;
}



/*****************************************************************************
 Name:		show_spec_cmds
 Purpose:	Displays settable special functions.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
void show_spec_cmds( CHAR_DATA *ch )
{
  char buf  [ MAX_STRING_LENGTH ];
  char buf1 [ MAX_STRING_LENGTH ];
  int  spec;
  int  col;
 
  buf1[0] = '\0';
  col = 0;
  send_to_char( "Preceed special functions with 'spec_'\n\r\n\r", ch );
  for (spec = 0; spec_table[spec].function != NULL; spec++) {
    sprintf( buf, "%-19.18s", &spec_table[spec].name[5] );
    strcat( buf1, buf );
    if ( ++col % 4 == 0 )
      strcat( buf1, "\n\r" );
  }
 
  if ( col % 4 != 0 )
    strcat( buf1, "\n\r" );

  send_to_char( buf1, ch );
  return;
}

/*****************************************************************************
 Name:		show_progs
 Purpose:	Displays "mob programs" available.
 Called by:	show_help(olc_act.c).
 ****************************************************************************/
// Modified by SinaC 2002 to keep trace of class's incoming file
void show_progs( CHAR_DATA *ch, char *argument ) {
  do_programs( ch, argument );
}



/*****************************************************************************
 Name:		show_help
 Purpose:	Displays help for many tables used in OLC.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_help( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int cnt;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  /*
   * Display syntax.
   */
  if ( arg[0] == '\0' ) {
    send_to_char( "Syntax:  ? [command]\n\r\n\r", ch );
    send_to_char( "[command]  [description]\n\r", ch );
    for (cnt = 0; help_table[cnt].command != NULL; cnt++) {
      sprintf( buf, "%-20.19s -%s\n\r",
	       capitalize( help_table[cnt].command ),
	       help_table[cnt].desc );
      send_to_char( buf, ch );
    }
    return FALSE;
  }

  /*
   * Find the command, show changeable data.
   * ---------------------------------------
   */
  for (cnt = 0; help_table[cnt].command != NULL; cnt++) {

    if (  arg[0] == help_table[cnt].command[0]
	  && !str_prefix( arg, help_table[cnt].command ) ) {

      if ( !str_cmp(help_table[cnt].command,"program")) {/* Should find another test -- Oxtal*/
	// Modified by SinaC 2002
	char args[MAX_STRING_LENGTH];
	sprintf( args, "%s %s", arg2, arg3 );
	show_progs( ch, args );
	return FALSE;
      }
      else if ( !str_cmp(help_table[cnt].command,"languages")) { // SinaC 2003
	int col = 0;
	for ( int i = 0; i < MAX_ABILITY; i++ )
	  if ( is_language(i) ) {
	    send_to_charf(ch,"%s", ability_table[i].name );
	    if ( ++col % 4 == 0 )
	      send_to_charf(ch,"\n\r");
	  }
	if ( col % 4 != 0 )
	  send_to_charf(ch,"\n\r");
      }
      else if ( help_table[cnt].structure == spec_table )	{
	show_spec_cmds( ch );
	return FALSE;
      } 
      else if ( help_table[cnt].structure == liq_table ) {
	show_liqlist( ch );
	return FALSE;
      }
      else if ( help_table[cnt].structure == attack_table ) {
	show_damlist( ch );
	return FALSE;
      }
      // Added by SinaC 2001
      else if ( help_table[cnt].structure == material_table ) {
	show_material( ch );
	return FALSE;
      }
      else if ( help_table[cnt].structure == ability_table ) {
	    
	int type = flag_value( ability_type_flags, arg2 );
	int target = flag_value( target_flags, arg3 );
	if ( arg2[0] == '\0' || arg3[0] == '\0'
	     || type == NO_FLAG ) {
	  send_to_char( "Syntax:  ? ability [ability type] [target]\n\r"
			"Type '? target' for a list of target\n\r"
			" and '? abilitytype' for a list of ability type\n\r", ch );
	  return FALSE;
	}
	show_skill_cmds( ch, type, target );
	return FALSE;
      } 
      else {
	show_flag_cmds( ch, (flag_type*) help_table[cnt].structure );
	return FALSE;
      }
    }
  }

  show_help( ch, "" );
  return FALSE;
}

REDIT( redit_rlist )
{
  ROOM_INDEX_DATA	*pRoomIndex;
  AREA_DATA		*pArea;
  char		buf  [ MAX_STRING_LENGTH   ];
  BUFFER		*buf1;
  char		arg  [ MAX_INPUT_LENGTH    ];
  bool found;
  int vnum;
  int  col = 0;

  one_argument( argument, arg );

  pArea = ch->in_room->area;
  buf1=new_buf();
  /*    buf1[0] = '\0'; */
  found   = FALSE;

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ ) {
    if ( ( pRoomIndex = get_room_index( vnum ) ) ) {
      found = TRUE;
      sprintf( buf, "[%5d] %-17.16s",
	       vnum, capitalize( pRoomIndex->name ) );
      add_buf( buf1, buf );
      if ( ++col % 3 == 0 )
	add_buf( buf1, "\n\r" );
    }
  }

  if ( !found ) {
    send_to_char( "Room(s) not found in this area.\n\r", ch);
    return FALSE;
  }

  if ( col % 3 != 0 )
    add_buf( buf1, "\n\r" );

  page_to_char( buf_string(buf1), ch );
  return FALSE;
}

REDIT( redit_mlist )
{
  MOB_INDEX_DATA	*pMobIndex;
  AREA_DATA		*pArea;
  char		buf  [ MAX_STRING_LENGTH   ];
  BUFFER		*buf1;
  char		arg  [ MAX_INPUT_LENGTH    ];
  bool fAll, found;
  int vnum;
  int  col = 0;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Syntax:  mlist <all/name>\n\r", ch );
    return FALSE;
  }

  buf1=new_buf();
  pArea = ch->in_room->area;
  /*    buf1[0] = '\0'; */
  fAll    = !str_cmp( arg, "all" );
  found   = FALSE;

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ ) {
    if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL ) {
      if ( fAll || is_name( arg, pMobIndex->player_name ) ) {
	found = TRUE;
	sprintf( buf, "[%5d] %-17.16s",
		 pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
	add_buf( buf1, buf );
	if ( ++col % 3 == 0 )
	  add_buf( buf1, "\n\r" );
      }
    }
  }

  if ( !found ) {
    send_to_char( "Mobile(s) not found in this area.\n\r", ch);
    return FALSE;
  }

  if ( col % 3 != 0 )
    add_buf( buf1, "\n\r" );

  page_to_char( buf_string(buf1), ch );
  return FALSE;
}



REDIT( redit_olist )
{
  OBJ_INDEX_DATA	*pObjIndex;
  AREA_DATA		*pArea;
  char		buf  [ MAX_STRING_LENGTH   ];
  BUFFER		*buf1;
  char		arg  [ MAX_INPUT_LENGTH    ];
  bool fAll, found;
  int vnum;
  int  col = 0;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Syntax:  olist <all/name/item_type>\n\r", ch );
    return FALSE;
  }

  pArea = ch->in_room->area;
  buf1=new_buf();
  /*    buf1[0] = '\0'; */
  fAll    = !str_cmp( arg, "all" );
  found   = FALSE;

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ ) {
    if ( ( pObjIndex = get_obj_index( vnum ) ) ) {
      if ( fAll || is_name( arg, pObjIndex->name )
	   || flag_value( type_flags, arg ) == pObjIndex->item_type ) {
	found = TRUE;
	sprintf( buf, "[%5d] %-17.16s",
		 pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
	add_buf( buf1, buf );
	if ( ++col % 3 == 0 )
	  add_buf( buf1, "\n\r" );
      }
    }
  }

  if ( !found ) {
    send_to_char( "Object(s) not found in this area.\n\r", ch);
    return FALSE;
  }

  if ( col % 3 != 0 )
    add_buf( buf1, "\n\r" );

  page_to_char( buf_string(buf1), ch );
  return FALSE;
}



REDIT( redit_mshow )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  mshow <vnum>\n\r", ch );
    return FALSE;
  }

  if ( !is_number( argument ) ) {
    send_to_char( "REdit: Ingresa un numero.\n\r", ch);
    return FALSE;
  }

  if ( is_number( argument ) ) {
    value = atoi( argument );
    if ( !( pMob = get_mob_index( value ) )) {
      send_to_char( "REdit:  That mobile does not exist.\n\r", ch );
      return FALSE;
    }

    ch->desc->pEdit = (void *)pMob;
  }
 
  medit_show( ch, argument );
  ch->desc->pEdit = (void *)ch->in_room;
  return FALSE; 
}



REDIT( redit_oshow )
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  oshow <vnum>\n\r", ch );
    return FALSE;
  }

  if ( !is_number( argument ) ) {
    send_to_char( "REdit: Ingresa un numero.\n\r", ch);
    return FALSE;
  }

  if ( is_number( argument ) ) {
    value = atoi( argument );
    if ( !( pObj = get_obj_index( value ) )) {
      send_to_char( "REdit:  That object does not exist.\n\r", ch );
      return FALSE;
    }

    ch->desc->pEdit = (void *)pObj;
  }
 
  oedit_show( ch, argument );
  ch->desc->pEdit = (void *)ch->in_room;
  return FALSE; 
}


/*****************************************************************************
 Name:		check_range( lower vnum, upper vnum )
 Purpose:	Ensures the range spans only one area.
 Called by:	aedit_vnum(olc_act.c).
 ****************************************************************************/
bool check_range( int lower, int upper )
{
  AREA_DATA *pArea;
  int cnt = 0;

  for ( pArea = area_first; pArea; pArea = pArea->next ) {
    /*
     * lower < area < upper
     */
    if ( ( lower <= pArea->min_vnum && pArea->min_vnum <= upper )
	 ||   ( lower <= pArea->max_vnum && pArea->max_vnum <= upper ) )
      ++cnt;

    if ( cnt > 1 )
      return FALSE;
  }
  return TRUE;
}



AREA_DATA *get_vnum_area( int vnum )
{
  AREA_DATA *pArea;

  for ( pArea = area_first; pArea; pArea = pArea->next ) {
    if ( vnum >= pArea->min_vnum
	 && vnum <= pArea->max_vnum )
      return pArea;
  }

  return 0;
}



/*
 * Area Editor Functions.
 */
AEDIT( aedit_show )
{
  AREA_DATA *pArea;
  char buf  [MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);

  sprintf( buf, "Name:     [%5d] %s\n\r", pArea->vnum, pArea->name );
  send_to_char( buf, ch );

#if 0  /* ROM OLC */
  sprintf( buf, "Recall:   [%5d] %s\n\r", pArea->recall,
	   get_room_index( pArea->recall )
	   ? get_room_index( pArea->recall )->name : "none" );
  send_to_char( buf, ch );
#endif /* ROM */

  sprintf( buf, "File:     %s\n\r", pArea->file_name );
  send_to_char( buf, ch );

  sprintf( buf, "Vnums:    [%d-%d]\n\r", pArea->min_vnum, pArea->max_vnum );
  send_to_char( buf, ch );

  sprintf( buf, "Age:      [%d]\n\r",	pArea->age );
  send_to_char( buf, ch );

  sprintf( buf, "Players:  [%d]\n\r", pArea->nplayer );
  send_to_char( buf, ch );

  sprintf( buf, "Security: [%d]\n\r", pArea->security );
  send_to_char( buf, ch );

  sprintf( buf, "Builders: [%s]\n\r", pArea->builders );
  send_to_char( buf, ch );

  sprintf( buf, "Credits : [%s]\n\r", pArea->credits );
  send_to_char( buf, ch );

  sprintf( buf, "Flags:    [%s]\n\r", flag_string( area_flags, pArea->area_flags ) );
  send_to_char( buf, ch );
  // Removed by SinaC 2003, scripts can do that
  // Added by SinaC 2000 for teleport room
  //sprintf( buf, "Teleport: [%d]\n\r", pArea->teleport_room );
  //send_to_char( buf, ch );

  return FALSE;
}

/* Removed by SinaC 2003, scripts can do that
AEDIT( aedit_teleport )
{
  AREA_DATA *pArea;
  char teleport[MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);
  
  one_argument( argument, teleport );
  
  if ( !is_number( teleport ) || teleport[0] == '\0' ) {
    send_to_char( "Syntax:  teleport <vnum>\n\r", ch );
    return FALSE;
  }
  
  pArea->teleport_room = atoi( teleport );
  
  send_to_char( "Teleport Room set.\n\r", ch );
  return TRUE;
}
*/

AEDIT( aedit_reset )
{
  AREA_DATA *pArea;

  EDIT_AREA(ch, pArea);

  reset_area( pArea );
  send_to_char( "Area reset.\n\r", ch );

  return FALSE;
}



AEDIT( aedit_create )
{
  AREA_DATA *pArea;

  pArea               =   new_area();
  area_last->next     =   pArea;
  area_last		=   pArea;	/* Thanks, Walker. */
  ch->desc->pEdit     =   (void *)pArea;

  SET_BIT( pArea->area_flags, AREA_ADDED );
  send_to_char( "Area Created.\n\r", ch );
  return FALSE;
}



AEDIT( aedit_name )
{
  AREA_DATA *pArea;

  EDIT_AREA(ch, pArea);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:   name [$name]\n\r", ch );
    return FALSE;
  }

  pArea->name = str_dup( argument, TRUE );

  send_to_char( "Name set.\n\r", ch );
  return TRUE;
}

AEDIT( aedit_credits )
{
  AREA_DATA *pArea;

  EDIT_AREA(ch, pArea);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:   credits [$credits]\n\r", ch );
    return FALSE;
  }

  pArea->credits = str_dup( argument, TRUE );

  send_to_char( "Credits set.\n\r", ch );
  return TRUE;
}


AEDIT( aedit_file )
{
  AREA_DATA *pArea;
  char file[MAX_STRING_LENGTH];
  int i, length;

  EDIT_AREA(ch, pArea);

  one_argument( argument, file );	/* Forces Lowercase */

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  filename [$file]\n\r", ch );
    return FALSE;
  }

  /*
   * Simple Syntax Check.
   */
  length = strlen( argument );
  if ( length > 8 ) {
    send_to_char( "No more than eight characters allowed.\n\r", ch );
    return FALSE;
  }
    
  /*
   * Allow only letters and numbers.
   */
  for ( i = 0; i < length; i++ ) {
    if ( !isalnum( file[i] ) ) {
      send_to_char( "Only letters and numbers are valid.\n\r", ch );
      return FALSE;
    }
  }    

  strcat( file, ".are" );
  pArea->file_name = str_dup( file, TRUE );

  send_to_char( "Filename set.\n\r", ch );
  return TRUE;
}



AEDIT( aedit_age )
{
  AREA_DATA *pArea;
  char age[MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);

  one_argument( argument, age );

  if ( !is_number( age ) || age[0] == '\0' ) {
    send_to_char( "Syntax:  age [#xage]\n\r", ch );
    return FALSE;
  }

  pArea->age = atoi( age );

  send_to_char( "Age set.\n\r", ch );
  return TRUE;
}


#if 0 /* ROM OLC */
AEDIT( aedit_recall )
{
  AREA_DATA *pArea;
  char room[MAX_STRING_LENGTH];
  int  value;

  EDIT_AREA(ch, pArea);

  one_argument( argument, room );

  if ( !is_number( argument ) || argument[0] == '\0' ) {
    send_to_char( "Syntax:  recall [#xrvnum]\n\r", ch );
    return FALSE;
  }

  value = atoi( room );

  if ( !get_room_index( value ) ) {
    send_to_char( "AEdit:  Room vnum does not exist.\n\r", ch );
    return FALSE;
  }

  pArea->recall = value;

  send_to_char( "Recall set.\n\r", ch );
  return TRUE;
}
#endif /* ROM OLC */


AEDIT( aedit_security )
{
  AREA_DATA *pArea;
  char sec[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int  value;

  EDIT_AREA(ch, pArea);

  one_argument( argument, sec );

  if ( !is_number( sec ) || sec[0] == '\0' ) {
    send_to_char( "Syntax:  security [#xlevel]\n\r", ch );
    return FALSE;
  }

  value = atoi( sec );

  if ( value > ch->pcdata->security || value < 0 ) {
    if ( ch->pcdata->security != 0 ) {
      sprintf( buf, "Security is 0-%d.\n\r", ch->pcdata->security );
      send_to_char( buf, ch );
    }
    else
      send_to_char( "Security is 0 only.\n\r", ch );
    return FALSE;
  }

  pArea->security = value;

  send_to_char( "Security set.\n\r", ch );
  return TRUE;
}



AEDIT( aedit_builder )
{
  AREA_DATA *pArea;
  char name[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  EDIT_AREA(ch, pArea);

  one_argument( argument, name );

  if ( name[0] == '\0' ) {
    send_to_char( "Syntax:  builder [$name]  -toggles builder\n\r", ch );
    send_to_char( "Syntax:  builder All      -allows everyone\n\r", ch );
    return FALSE;
  }

  name[0] = UPPER( name[0] );

  if ( strstr( pArea->builders, name ) != '\0' ) {
    pArea->builders = string_replace( pArea->builders, name, "\0" );
    pArea->builders = string_unpad( pArea->builders );

    if ( pArea->builders[0] == '\0' ) {
      pArea->builders = str_dup( "None", TRUE );
    }
    send_to_char( "Builder removed.\n\r", ch );
    return TRUE;
  }
  else {
    buf[0] = '\0';
    if ( strstr( pArea->builders, "None" ) != '\0' ) {
      pArea->builders = string_replace( pArea->builders, "None", "\0" );
      pArea->builders = string_unpad( pArea->builders );
    }

    if (pArea->builders[0] != '\0' ) {
      strcat( buf, pArea->builders );
      strcat( buf, " " );
    }
    strcat( buf, name );
    pArea->builders = string_proper( str_dup( buf, TRUE ) );

    send_to_char( "Builder added.\n\r", ch );
    send_to_char( pArea->builders,ch);
    return TRUE;
  }

  return FALSE;
}



AEDIT( aedit_vnum )
{
  AREA_DATA *pArea;
  char lower[MAX_STRING_LENGTH];
  char upper[MAX_STRING_LENGTH];
  int  ilower;
  int  iupper;

  EDIT_AREA(ch, pArea);

  argument = one_argument( argument, lower );
  one_argument( argument, upper );

  if ( !is_number( lower ) || lower[0] == '\0'
       || !is_number( upper ) || upper[0] == '\0' ) {
    send_to_char( "Syntax:  vnum [#xlower] [#xupper]\n\r", ch );
    return FALSE;
  }

  if ( ( ilower = atoi( lower ) ) > ( iupper = atoi( upper ) ) ) {
    send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
    return FALSE;
  }
    
  if ( !check_range( atoi( lower ), atoi( upper ) ) ) {
    send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
    return FALSE;
  }

  if ( get_vnum_area( ilower )
       && get_vnum_area( ilower ) != pArea ) {
    send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
    return FALSE;
  }

  pArea->min_vnum = ilower;
  send_to_char( "Lower vnum set.\n\r", ch );

  if ( get_vnum_area( iupper )
       && get_vnum_area( iupper ) != pArea ) {
    send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
    return TRUE;	/* The lower value has been set. */
  }

  pArea->max_vnum = iupper;
  send_to_char( "Upper vnum set.\n\r", ch );

  return TRUE;
}



AEDIT( aedit_lvnum )
{
  AREA_DATA *pArea;
  char lower[MAX_STRING_LENGTH];
  int  ilower;
  int  iupper;

  EDIT_AREA(ch, pArea);

  one_argument( argument, lower );

  if ( !is_number( lower ) || lower[0] == '\0' ) {
    send_to_char( "Syntax:  min_vnum [#xlower]\n\r", ch );
    return FALSE;
  }

  if ( ( ilower = atoi( lower ) ) > ( iupper = pArea->max_vnum ) ) {
    send_to_char( "AEdit:  Value must be less than the max_vnum.\n\r", ch );
    return FALSE;
  }
    
  if ( !check_range( ilower, iupper ) ) {
    send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
    return FALSE;
  }

  if ( get_vnum_area( ilower )
       && get_vnum_area( ilower ) != pArea ) {
    send_to_char( "AEdit:  Lower vnum already assigned.\n\r", ch );
    return FALSE;
  }

  pArea->min_vnum = ilower;
  send_to_char( "Lower vnum set.\n\r", ch );
  return TRUE;
}



AEDIT( aedit_uvnum )
{
  AREA_DATA *pArea;
  char upper[MAX_STRING_LENGTH];
  int  ilower;
  int  iupper;

  EDIT_AREA(ch, pArea);

  one_argument( argument, upper );

  if ( !is_number( upper ) || upper[0] == '\0' ) {
    send_to_char( "Syntax:  max_vnum [#xupper]\n\r", ch );
    return FALSE;
  }

  if ( ( ilower = pArea->min_vnum ) > ( iupper = atoi( upper ) ) ) {
    send_to_char( "AEdit:  Upper must be larger then lower.\n\r", ch );
    return FALSE;
  }
    
  if ( !check_range( ilower, iupper ) ) {
    send_to_char( "AEdit:  Range must include only this area.\n\r", ch );
    return FALSE;
  }

  if ( get_vnum_area( iupper )
       && get_vnum_area( iupper ) != pArea ) {
    send_to_char( "AEdit:  Upper vnum already assigned.\n\r", ch );
    return FALSE;
  }

  pArea->max_vnum = iupper;
  send_to_char( "Upper vnum set.\n\r", ch );

  return TRUE;
}



/*
 * Room Editor Functions.
 */
REDIT( redit_show )
{
  ROOM_INDEX_DATA	*pRoom;
  char		buf  [MAX_STRING_LENGTH];
  char		buf1 [2*MAX_STRING_LENGTH];
  OBJ_DATA		*obj;
  CHAR_DATA		*rch;
  int			door;
  bool		fcnt;
    
  EDIT_ROOM(ch, pRoom);

  buf1[0] = '\0';
    
  sprintf( buf, "Description:\n\r%s", pRoom->description );
  strcat( buf1, buf );

  sprintf( buf, "Name:       [%s]\n\rArea:       [%5d] %s\n\r",
	   pRoom->name, pRoom->area->vnum, pRoom->area->name );
  strcat( buf1, buf );

  // Modified by SinaC 2001
  sprintf( buf, "Vnum:       [%5d]\n\rSector:     [%s]\n\r",
	   pRoom->vnum, flag_string( sector_flags, pRoom->bstat(sector) ) );
  strcat( buf1, buf );

  // Modified by SinaC 2001
  sprintf( buf, "Room flags: [%s]\n\r",
	   flag_string( room_flags, pRoom->bstat(flags) ) );
  strcat( buf1, buf );

  // Modified by SinaC 2001 for mental user
  /*
    sprintf( buf, "Health recovery:[%d]\n\rMana recovery  :[%d]\n\r",
    pRoom->heal_rate , pRoom->mana_rate );
  */
  sprintf( buf, 
	   "Health recovery:[%d]\n\r"
	   "Mana recovery  :[%d]\n\r"
	   "Psp recovery   :[%d]\n\r",
	   // Modified by SinaC 2001
	   pRoom->bstat(healrate), 
	   pRoom->bstat(manarate),
	   pRoom->bstat(psprate) );

  strcat( buf1, buf );
        
  sprintf( buf, "Clan:       [%d] %s\n\r" , pRoom->clan ,
	   ((pRoom->clan > 0) ? get_clan_table(pRoom->clan)->name : "none" ));
  strcat( buf1, buf );
    
  /* Oxtal */
  sprintf( buf, "Guild:      [%s]\n\r", flag_string( classes_flags, pRoom->guild ) );
  strcat( buf1, buf );
    
  sprintf( buf, "Owner:      [%s]\n\r", pRoom->owner );
  strcat( buf1, buf );

  // Added by SinaC 2001
  // Modified by SinaC 2001
  sprintf( buf, "Max size:   [%s]\n\r", flag_string( size_flags, pRoom->bstat(maxsize) ) );
  strcat( buf1, buf );

  if (pRoom->program) {
    sprintf( buf, "Program :    [%s]\n\r", pRoom->program->name );
    strcat( buf1, buf );
  }

  if ( pRoom->time_between_repop != BASE_REPOP_TIME 
       || pRoom->time_between_repop_people != BASE_REPOP_TIME_PEOPLE ) {
    sprintf( buf, "Repop time:  [%d] [%d]\n",
	     pRoom->time_between_repop, 
	     pRoom->time_between_repop_people );
    strcat( buf1, buf);
  }

  /* Modified by SinaC 2001
  if ( pRoom->extra_descr ) {
    EXTRA_DESCR_DATA *ed;

    strcat( buf1, "Desc Kwds:  [" );
    for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
      // Modified by SinaC 2001
      char buf2[512];
      sprintf(buf2, "(%s)", ed->keyword );
      //strcat( buf1, ed->keyword );
      strcat( buf1, buf2 );
      if ( ed->next )
	strcat( buf1, " " );
    }
    strcat( buf1, "]\n\r" );
  }
  */

  if ( pRoom->extra_descr ) {
    EXTRA_DESCR_DATA *ed;

    strcat( buf1, "Ex desc kwd: " );

    for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
      strcat( buf1, "[" );
      strcat( buf1,  ed->keyword );
      strcat( buf1, "]" );
    }

    strcat( buf1, "\n\r" );
  }

  strcat( buf1, "Characters: [" );
  fcnt = FALSE;
  for ( rch = pRoom->people; rch; rch = rch->next_in_room ) {
    one_argument( rch->name, buf );
    strcat( buf1, buf );
    strcat( buf1, " " );
    fcnt = TRUE;
  }

  if ( fcnt ) {
    int end;

    end = strlen(buf1) - 1;
    buf1[end] = ']';
    strcat( buf1, "\n\r" );
  }
  else
    strcat( buf1, "none]\n\r" );

  strcat( buf1, "Objects:    [" );
  fcnt = FALSE;
  for ( obj = pRoom->contents; obj; obj = obj->next_content ) {
    one_argument( obj->name, buf );
    strcat( buf1, buf );
    strcat( buf1, " " );
    fcnt = TRUE;
  }

  if ( fcnt ) {
    int end;

    end = strlen(buf1) - 1;
    buf1[end] = ']';
    strcat( buf1, "\n\r" );
  }
  else
    strcat( buf1, "none]\n\r" );

  for ( door = 0; door < MAX_DIR; door++ ) {
    EXIT_DATA *pexit;

    if ( ( pexit = pRoom->exit[door] ) ) {
      char word[MAX_INPUT_LENGTH];
      char reset_state[MAX_STRING_LENGTH];
      const char *state;
      int i, length;

      sprintf( buf, "-%-5s to [%5d] Key: [%5d] ",
	       capitalize(dir_name[door]),
	       pexit->u1.to_room ? pexit->u1.to_room->vnum : 0,      /* ROM OLC */
	       pexit->key );
      strcat( buf1, buf );

      /*
       * Format up the exit info.
       * Capitalize all flags that are not part of the reset info.
       */
//      strcpy( reset_state, flag_string( exit_flags, pexit->rs_flags ) );
//      state = flag_string( exit_flags, pexit->exit_info );
//      strcat( buf1, " Exit flags: [" );
//      for (; ;) {
//	state = one_argument( state, word );
//
//	if ( word[0] == '\0' ) {
//	  int end;
//
//	  end = strlen(buf1) - 1;
//	  buf1[end] = ']';
//	  strcat( buf1, "\n\r" );
//	  break;
//	}
//
//	if ( str_infix( word, reset_state ) ) {
//	  length = strlen(word);
//	  for (i = 0; i < length; i++)
//	    word[i] = UPPER(word[i]);
//	}
//	strcat( buf1, word );
//	strcat( buf1, " " );
//      }

      strcat( buf1, " Exit flags: [" );
      bool found = FALSE;
      long total = pexit->rs_flags | pexit->exit_info;
      for ( int i = 0; i < 32; i++ ) { // check every bits
	long bit = 1 << i;
	if ( !IS_SET( total, bit ) ) // bit not found
	  continue;
	found = TRUE;
	// rs_flags OR exit_info OR both contains this bit
	if ( IS_SET( pexit->rs_flags, bit ) )    // rs_flags
	  if ( IS_SET( pexit->exit_info, bit ) ) // and exit_info -> normal
	    strcat( buf1, flag_string( exit_flags, bit ) );
	  else {                                 // but not exit_info -> upper case (closed i.e.)
	    //	    char tmp[MAX_INPUT_LENGTH];
	    //	    strcpy( tmp, flag_string( exit_flags, bit ) );
	    //	    int length = strlen( tmp );
	    //	    for ( int i = 0; i < length; i++ )
	    //	      tmp[i] = UPPER(tmp[i]);
	    //	    strcat( buf1, tmp );
	    strcat( buf1, str_to_upper( flag_string( exit_flags, bit ) ) );
	  }
	else                                     // exit_info
	  if ( IS_SET( pexit->rs_flags, bit ) )  // and rs_flags -> normal
	    strcat( buf1, flag_string( exit_flags, bit ) );
	  else {                                 // but not rs_flags -> between parenthesis (bashed i.e.)
	    strcat( buf1, "(");
	    strcat( buf1, flag_string( exit_flags, bit ) );
	    strcat( buf1, ")");
	  }
	strcat( buf1, " " );
      }
      if ( !found )
	strcat( buf1, "none]\n\r" );
      else {
	buf1[strlen(buf1)-1] = '\0'; // remove unwanted space
	strcat( buf1, "]\n\r");
      }

      if ( pexit->keyword && pexit->keyword[0] != '\0' ) {
	sprintf( buf, "Kwds: [%s]\n\r", pexit->keyword );
	strcat( buf1, buf );
      }
      if ( pexit->description && pexit->description[0] != '\0' ) {
	sprintf( buf, "%s", pexit->description );
	strcat( buf1, buf );
      }
    }
  }
  send_to_char( buf1, ch );

  general_show_affects( ch, pRoom->base_affected, "auto" );

  return FALSE;
}




/* Local function. */
bool change_exit( CHAR_DATA *ch, const char *argument, int door ) {
  ROOM_INDEX_DATA *pRoom;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int  value;

  EDIT_ROOM(ch, pRoom);

  /*
   * Set the exit flags, needs full argument.
   * ----------------------------------------
   */
  bool unlinked = FALSE;
  // if unlinked is specified, flag are not applied to reverse dir
  if ( !str_prefix( "unlinked", argument ) ) {
    argument = one_argument( argument, command );
    unlinked = TRUE;
  }
  if ( ( value = flag_value( exit_flags, argument ) ) != NO_FLAG ) {
    ROOM_INDEX_DATA *pToRoom;
    int rev;                                    /* ROM OLC */

    if ( !pRoom->exit[door] ) {
      send_to_char("Exit doesn't exist.\n\r",ch);
      return FALSE;
    }
    /*   pRoom->exit[door] = new_exit(); */

    /*
     * This room.
     */
    TOGGLE_BIT(pRoom->exit[door]->rs_flags,  value);
    /* Don't toggle exit_info because it can be changed by players. */
    pRoom->exit[door]->exit_info = pRoom->exit[door]->rs_flags;

    /*
     * Connected room.
     */
    pToRoom = pRoom->exit[door]->u1.to_room;     /* ROM OLC */
    rev = rev_dir[door];

    if ( !unlinked && pToRoom->exit[rev] != NULL) { // unlinked, SinaC 2003
      TOGGLE_BIT(pToRoom->exit[rev]->rs_flags,  value);
      TOGGLE_BIT(pToRoom->exit[rev]->exit_info, value);
      SET_BIT(pToRoom->area->area_flags, AREA_CHANGED);
    }

    send_to_char( "Exit flag toggled.\n\r", ch );
    return TRUE;
  }

  /*
   * Now parse the arguments.
   */
  argument = one_argument( argument, command );
  one_argument( argument, arg );

  if ( command[0] == '\0' && argument[0] == '\0' ) {	/* Move command. */
    move_char( ch, door, TRUE, TRUE, FALSE ); // SinaC 2003 );                    /* ROM OLC */
    return FALSE;
  }

  if ( command[0] == '?' ) {
    do_help( ch, "EXIT" );
    return FALSE;
  }

  if ( !str_cmp( command, "delete" ) ) {
    ROOM_INDEX_DATA *pToRoom;
    int rev;                                     /* ROM OLC */
	
    if ( !pRoom->exit[door] ) {
      send_to_char( "REdit:  Cannot delete a null exit.\n\r", ch );
      return FALSE;
    }

    /*
     * Remove ToRoom Exit.
     */
    rev = rev_dir[door];
    pToRoom = pRoom->exit[door]->u1.to_room;       /* ROM OLC */
	
    if ( pToRoom->exit[rev] ) {
      pToRoom->exit[rev] = NULL;
    }

    /*
     * Remove this exit.
     */
    pRoom->exit[door] = NULL;

    SET_BIT(pToRoom->area->area_flags, AREA_CHANGED);


    send_to_char( "Exit unlinked.\n\r", ch );
    return TRUE;
  }

  if ( !str_cmp( command, "link" ) ) {
    EXIT_DATA *pExit;

    if ( arg[0] == '\0' || !is_number( arg ) ) {
      send_to_char( "Syntax:  [direction] link [vnum]\n\r", ch );
      return FALSE;
    }

    value = atoi( arg );
    ROOM_INDEX_DATA *pToRoom = get_room_index( value );

    if ( !pToRoom ) {
      send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
      return FALSE;
    }

    if ( !IS_BUILDER( ch, pToRoom->area ) ) {
      send_to_char( "REdit:  Cannot link to that area.\n\r", ch );
      return FALSE;
    }

    if ( pToRoom->exit[rev_dir[door]] ) {
      send_to_char( "REdit:  Remote side's exit already exists.\n\r", ch );
      return FALSE;
    }

    if ( !pRoom->exit[door] )	{
      pRoom->exit[door] = new_exit();
    }

    pRoom->exit[door]->u1.to_room = pToRoom;   /* ROM OLC */
    pRoom->exit[door]->orig_door = door;
	
    /*	pRoom->exit[door]->vnum = value;                Can't set vnum in ROM */

    door                    = rev_dir[door];
    pExit                   = new_exit();
    pExit->u1.to_room       = ch->in_room;
    /*	pExit->vnum             = ch->in_room->vnum;    Can't set vnum in ROM */
    pExit->orig_door	= door;
    pToRoom->exit[door]       = pExit;
    SET_BIT(pToRoom->area->area_flags, AREA_CHANGED);

    send_to_char( "Two-way link established.\n\r", ch );
    return TRUE;
  }
        
  if ( !str_cmp( command, "dig" ) ) {
    char buf[MAX_STRING_LENGTH];
	
    if ( arg[0] == '\0' || !is_number( arg ) ) {
      send_to_char( "Syntax: [direction] dig <vnum>\n\r", ch );
      return FALSE;
    }
	
    redit_create( ch, arg );
    sprintf( buf, "link %s", arg );
    change_exit( ch, buf, door);
    return TRUE;
  }

  if ( !str_cmp( command, "room" ) ) {
    if ( arg[0] == '\0' || !is_number( arg ) ) {
      send_to_char( "Syntax:  [direction] room [vnum]\n\r", ch );
      return FALSE;
    }

    value = atoi( arg );
    ROOM_INDEX_DATA *pToRoom = get_room_index( value );

    if ( !pToRoom ) {
      send_to_char( "REdit:  Cannot link to non-existant room.\n\r", ch );
      return FALSE;
    }
	
    if ( !pRoom->exit[door] ) {
      pRoom->exit[door] = new_exit();
    }

    pRoom->exit[door]->u1.to_room = pToRoom;    /* ROM OLC */
    pRoom->exit[door]->orig_door = door;
    /*	pRoom->exit[door]->vnum = value;                 Can't set vnum in ROM */
    send_to_char( "One-way link established.\n\r", ch );
    return TRUE;
  }

  if ( !str_cmp( command, "key" ) ) {
    if ( arg[0] == '\0' || !is_number( arg ) ) {
      send_to_char( "Syntax:  [direction] key [vnum]\n\r", ch );
      return FALSE;
    }

    if ( !pRoom->exit[door] )	{
      send_to_char("That exit doesn't exits.\n\r",ch);
      return FALSE;
    }

    /*	
	if ( !pRoom->exit[door] )
	{
	pRoom->exit[door] = new_exit();
	} 
    */

    value = atoi( arg );

    // Modified by SinaC 2000, have added if ( value != -1 ...
    if ( value != -1 && value != 0 ) {
      if ( !get_obj_index( value ) ) {
	send_to_char( "REdit:  Item doesn't exist.\n\r", ch );
	return FALSE;
      }
	    
      if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY ) {
	send_to_char( "REdit:  Key doesn't exist.\n\r", ch );
	return FALSE;
      }
    }

    pRoom->exit[door]->key = value;

    send_to_char( "Exit key set.\n\r", ch );
    return TRUE;
  }

  if ( !str_cmp( command, "name" ) ) {
    if ( arg[0] == '\0' ) {
      send_to_char( "Syntax:  [direction] name [string]\n\r", ch );
      send_to_char( "         [direction] name none\n\r", ch );
      return FALSE;
    }

    if ( !pRoom->exit[door] ) {
      send_to_char("Exit doesn't exist.\n\r",ch);
      return FALSE;
    }

    /*	if ( !pRoom->exit[door] )
	{
	pRoom->exit[door] = new_exit();
	} */

    if (str_cmp(arg,"none"))
      pRoom->exit[door]->keyword = str_dup( arg, TRUE );
    else
      pRoom->exit[door]->keyword = &str_empty[0];//str_dup("");

    send_to_char( "Exit name set.\n\r", ch );
    return TRUE;
  }

  if ( !str_prefix( command, "description" ) ) {
    if ( arg[0] == '\0' ) {
      if ( !pRoom->exit[door] ) {
	send_to_char("Exit doesn't exist.\n\r",ch);
	return FALSE;
      }

      /*	    if ( !pRoom->exit[door] )
		    {
		    pRoom->exit[door] = new_exit();
		    } */

      string_append( ch, &pRoom->exit[door]->description );
      return TRUE;
    }

    send_to_char( "Syntax:  [direction] desc\n\r", ch );
    return FALSE;
  }

  return FALSE;
}



REDIT( redit_north )
{
  if ( change_exit( ch, argument, DIR_NORTH ) )
    return TRUE;

  return FALSE;
}



REDIT( redit_south )
{
  if ( change_exit( ch, argument, DIR_SOUTH ) )
    return TRUE;

  return FALSE;
}



REDIT( redit_east )
{
  if ( change_exit( ch, argument, DIR_EAST ) )
    return TRUE;

  return FALSE;
}



REDIT( redit_west )
{
  if ( change_exit( ch, argument, DIR_WEST ) )
    return TRUE;

  return FALSE;
}



REDIT( redit_up )
{
  if ( change_exit( ch, argument, DIR_UP ) )
    return TRUE;

  return FALSE;
}



REDIT( redit_down )
{
  if ( change_exit( ch, argument, DIR_DOWN ) )
    return TRUE;

  return FALSE;
}

// Added by SinaC 2003
REDIT( redit_northeast )
{
  if ( change_exit( ch, argument, DIR_NORTHEAST ) )
    return TRUE;

  return FALSE;
}
REDIT( redit_northwest )
{
  if ( change_exit( ch, argument, DIR_NORTHWEST ) )
    return TRUE;

  return FALSE;
}
REDIT( redit_southeast )
{
  if ( change_exit( ch, argument, DIR_SOUTHEAST ) )
    return TRUE;

  return FALSE;
}
REDIT( redit_southwest )
{
  if ( change_exit( ch, argument, DIR_SOUTHWEST ) )
    return TRUE;

  return FALSE;
}


void ed_syntax( CHAR_DATA *ch ) {
  send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
  send_to_char( "         ed edit [keyword]\n\r", ch );
  send_to_char( "         ed delete [keyword]\n\r", ch );
  send_to_char( "         ed format [keyword]\n\r", ch );
  // Added by SinaC 2001
  send_to_char( "         ed change [old keyword] [new keyword]\n\r", ch);
}

REDIT( redit_ed )
{
  ROOM_INDEX_DATA *pRoom;
  EXTRA_DESCR_DATA *ed;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];

  EDIT_ROOM(ch, pRoom);

  // Modified by SinaC 2001
  argument = one_argument( argument, command );
  //one_argument( argument, keyword ); moved down by SinaC 2001

  // Modified by SinaC 2001
  if ( command[0] == '\0' ) { //|| keyword[0] == '\0' ) {
    ed_syntax(ch);
    return FALSE;
  }

  // Added by SinaC 2001
  if ( !str_cmp( command, "change" ) ) {
    argument = one_argument( argument, keyword );
    
    if ( keyword[0] == '\0' || argument[0] == '\0' )	{
      send_to_char( "Syntax:  ed change [old keyword] [new keyword]\n\r", ch );
      return FALSE;
    }
    for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
    }

    if ( !ed ) {
      send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    ed->keyword = str_dup( argument, TRUE );
    return TRUE;
  }

  // Added by SinaC 2001
  strcpy( keyword, argument );
  if ( keyword[0] == '\0' ) {
    ed_syntax(ch);
    return FALSE;
  }

  if ( !str_cmp( command, "add" ) ) {
    if ( keyword[0] == '\0' )	{
      send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
      return FALSE;
    }

    ed			=   new_extra_descr();
    ed->keyword		=   str_dup( keyword, TRUE );
    ed->description		=   &str_empty[0];//str_dup( "" );
    ed->next		=   pRoom->extra_descr;
    pRoom->extra_descr	=   ed;

    string_append( ch, &ed->description );

    return TRUE;
  }


  if ( !str_cmp( command, "edit" ) ) {
    if ( keyword[0] == '\0' )	{
      send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
      return FALSE;
    }

    for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
    }

    if ( !ed ) {
      send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    string_append( ch, &ed->description );

    return TRUE;
  }


  if ( !str_cmp( command, "delete" ) ) {
    EXTRA_DESCR_DATA *ped = NULL;

    if ( keyword[0] == '\0' ) {
      send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
      return FALSE;
    }

    for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
      ped = ed;
    }

    if ( !ed ) {
      send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    if ( !ped )
      pRoom->extra_descr = ed->next;
    else
      ped->next = ed->next;

    send_to_char( "Extra description deleted.\n\r", ch );
    return TRUE;
  }


  if ( !str_cmp( command, "format" ) ) {
    if ( keyword[0] == '\0' )	{
      send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
      return FALSE;
    }

    for ( ed = pRoom->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
    }

    if ( !ed ) {
      send_to_char( "REdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    ed->description = format_string( ed->description );

    send_to_char( "Extra description formatted.\n\r", ch );
    return TRUE;
  }

  redit_ed( ch, "" );
  return FALSE;
}



REDIT( redit_create )
{
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  int value;
  int iHash;
    
  EDIT_ROOM(ch, pRoom);

  value = atoi( argument );

  if ( argument[0] == '\0' || value <= 0 ) {
    send_to_char( "Syntax:  create [vnum > 0]\n\r", ch );
    return FALSE;
  }

  pArea = get_vnum_area( value );
  if ( !pArea ) {
    send_to_char( "REdit:  That vnum is not assigned an area.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "REdit:  Vnum in an area you cannot build in.\n\r", ch );
    return FALSE;
  }

  if ( get_room_index( value ) ) {
    send_to_char( "REdit:  Room vnum already exists.\n\r", ch );
    return FALSE;
  }

  pRoom			= new_room_index();
  pRoom->area			= pArea;
  pRoom->vnum			= value;

  if ( value > top_vnum_room )
    top_vnum_room = value;

  iHash			= value % MAX_KEY_HASH;
  pRoom->next			= room_index_hash[iHash];
  room_index_hash[iHash]	= pRoom;
  ch->desc->pEdit		= (void *)pRoom;

  send_to_char( "Room created.\n\r", ch );
  return TRUE;
}



REDIT( redit_name )
{
  ROOM_INDEX_DATA *pRoom;

  EDIT_ROOM(ch, pRoom);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  name [name]\n\r", ch );
    return FALSE;
  }

  pRoom->name = str_dup( argument, TRUE );

  send_to_char( "Name set.\n\r", ch );
  return TRUE;
}



REDIT( redit_desc )
{
  ROOM_INDEX_DATA *pRoom;

  EDIT_ROOM(ch, pRoom);

  if ( argument[0] == '\0' ) {
    string_append( ch, &pRoom->description );
    return TRUE;
  }

  send_to_char( "Syntax:  desc\n\r", ch );
  return FALSE;
}

REDIT( redit_heal )
{
  ROOM_INDEX_DATA *pRoom;
    
  EDIT_ROOM(ch, pRoom);
    
  if (is_number(argument)) {
    // Modified by SinaC 2001
    pRoom->bstat(healrate) = atoi ( argument );
    send_to_char ( "Heal rate set.\n\r", ch);
    recomproom(pRoom);
    return TRUE;
  }

  send_to_char ( "Syntax : heal <#xnumber>\n\r", ch);
  return FALSE;
}       

REDIT( redit_mana )
{
  ROOM_INDEX_DATA *pRoom;
    
  EDIT_ROOM(ch, pRoom);
    
  if (is_number(argument)) {
    // Modified by SinaC 2001
    pRoom->bstat(manarate) = atoi ( argument );
    send_to_char ( "Mana rate set.\n\r", ch);
    recomproom(pRoom);
    return TRUE;
  }

  send_to_char ( "Syntax : mana <#xnumber>\n\r", ch);
  return FALSE;
}       

REDIT( redit_psp )
{
  ROOM_INDEX_DATA *pRoom;
    
  EDIT_ROOM(ch, pRoom);
    
  if (is_number(argument)) {
    // Modified by SinaC 2001
    pRoom->bstat(psprate) = atoi ( argument );
    send_to_char ( "Psp rate set.\n\r", ch);
    recomproom(pRoom);
    return TRUE;
  }

  send_to_char ( "Syntax : psp <#xnumber>\n\r", ch);
  return FALSE;
}       


REDIT( redit_clan )
{
  ROOM_INDEX_DATA *pRoom;
  int clan;

  EDIT_ROOM(ch, pRoom);
    
  if ( ( clan = clan_lookup(argument) ) == 0 ) {
    send_to_char( "Invalid clan!\n\r", ch );
    return FALSE;
  }
  pRoom->clan = clan;

  /* Modified by SinaC 2001
  pRoom->clan = clan_lookup(argument);
  */
    
  send_to_char ( "Clan set.\n\r", ch);
  return TRUE;
}

REDIT( redit_guild )
{
  ROOM_INDEX_DATA *pRoom;
  long res;

  EDIT_ROOM(ch, pRoom);
    
  if (!str_cmp(argument,"none")) {
    pRoom->guild = 0;
    send_to_char("Room is no longer guild",ch);
    return TRUE;
  }
    
  if ( (res = flag_value(classes_flags,argument)) != NO_FLAG) {    
    pRoom->guild = res;
    send_to_charf (ch, "Room is now guild for %s.\n\r", flag_string( classes_flags, res)) ;
    return TRUE;
  } 
  else {
    send_to_char ("Syntax : guild { <classes> | none }\n\r",ch);
    return FALSE;
  }
}

REDIT( redit_maxsize )
{
  ROOM_INDEX_DATA *pRoom;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_ROOM( ch, pRoom );
    
    if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG ) {
      // Modified by SinaC 2001
      pRoom->bstat(maxsize) = value;
      send_to_char( "Maximal size set.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: maxsize [size]\n\r"
		"Type '? maxsize' for a list of sizes.\n\r", ch );
  return FALSE;
}

      
REDIT( redit_format )
{
  ROOM_INDEX_DATA *pRoom;

  EDIT_ROOM(ch, pRoom);

  pRoom->description = format_string( pRoom->description );

  send_to_char( "String formatted.\n\r", ch );
  return TRUE;
}



REDIT( redit_mreset )
{
  ROOM_INDEX_DATA	*pRoom;
  MOB_INDEX_DATA	*pMobIndex;
  CHAR_DATA		*newmob;
  char		arg [ MAX_INPUT_LENGTH ];
  char		arg2 [ MAX_INPUT_LENGTH ];

  RESET_DATA		*pReset;
  char		output [ MAX_STRING_LENGTH ];

  EDIT_ROOM(ch, pRoom);

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );

  if ( arg[0] == '\0' || !is_number( arg ) ) {
    send_to_char ( "Syntax:  mreset <vnum> <max #x> <mix #x>\n\r", ch );
    return FALSE;
  }

  if ( !( pMobIndex = get_mob_index( atoi( arg ) ) ) ) {
    send_to_char( "REdit: No mobile has that vnum.\n\r", ch );
    return FALSE;
  }

  if ( pMobIndex->area != pRoom->area ) {
    send_to_char( "REdit: No such mobile in this area.\n\r", ch );
    return FALSE;
  }

  /*
   * Create the mobile reset.
   */
  pReset              = new_reset_data();
  pReset->command	= 'M';
  pReset->arg1	= pMobIndex->vnum;
  pReset->arg2	= is_number( arg2 ) ? atoi( arg2 ) : MAX_MOB;
  pReset->arg3	= pRoom->vnum;
  pReset->arg4	= is_number( argument ) ? atoi (argument) : 1;
  add_reset( pRoom, pReset, 0/* Last slot*/ );

  /*
   * Create the mobile.
   */
  newmob = create_mobile( pMobIndex );
  char_to_room( newmob, pRoom );

  sprintf( output, "%s (%d) has been loaded and added to resets.\n\r"
	   "There will be a maximum of %d loaded to this room.\n\r",
	   capitalize( pMobIndex->short_descr ),
	   pMobIndex->vnum,
	   pReset->arg2 );
  send_to_char( output, ch );
  act( "$n has created $N!", ch, NULL, newmob, TO_ROOM );
  return TRUE;
}

// Added by SinaC 2001 for room flags and sector
REDIT( redit_flag )
{
  ROOM_INDEX_DATA *pRoom;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_ROOM(ch, pRoom);

    if ( ( value = flag_value( room_flags, argument ) ) != NO_FLAG ) {
      // Modified by SinaC 2001
      TOGGLE_BIT(pRoom->bstat(flags), value);
      send_to_char( "Room flag toggled.\n\r", ch );
      recomproom(pRoom);
      return TRUE;
    }
  }

  send_to_char( "Syntax: flag [room flag]\n\r"
		"Type '? room' for a list of room flags.\n\r", ch );
  return FALSE;
}

REDIT( redit_sector )
{
  ROOM_INDEX_DATA *pRoom;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_ROOM(ch, pRoom);

    if ( ( value = flag_value( sector_flags, argument ) ) != NO_FLAG ) {
      // Modified by SinaC 2001
      pRoom->bstat(sector) =  value;
      send_to_char( "Sector type set.\n\r", ch );
      recomproom(pRoom);
      return TRUE;
    }
  }

  send_to_char( "Syntax: sector [sector type]\n\r"
		"Type '? sector' for a list of sector type.\n\r", ch );
  return FALSE;
}

// Added by SinaC 2003 for room program
REDIT( redit_program )
{
  ROOM_INDEX_DATA *pRoom;
  CLASS_DATA *cla;

  EDIT_ROOM(ch, pRoom);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  program [program name/none]\n\r", ch );
    return FALSE;
  }


  if ( !str_cmp( argument, "none" ) ) {
    pRoom->program = NULL;
    pRoom->clazz = NULL;

    send_to_char( "program removed.\n\r", ch);
    return TRUE;
  }

  if ( ( cla = silent_hash_get_prog( argument ) ) ) {
    if ( get_root_class(cla) == default_room_class ) {
      if ( cla->isAbstract ) {
	send_to_char("This is an abstract program, it must be derivated before being used.\n\r",ch);
	return FALSE;
      }
      pRoom->program = cla;
      pRoom->clazz = cla;
      send_to_char( "Program set.\n\r", ch);
      return TRUE;
    } 
    else {
      send_to_char("Not a room program!!\n\r", ch);
      return FALSE;
    }
  }

  send_to_char( "OEdit: No such program.\n\r", ch );
  return FALSE;
}

REDIT( redit_addaffect ) {
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM(ch, pRoom);

  return general_add_affect( ch, pRoom->base_affected, ROOM_ENTITY, NULL, 100, argument );
}
REDIT( redit_delaffect ) {
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM(ch, pRoom);

  return general_del_affect( ch, pRoom->base_affected, argument );
}
REDIT( redit_showaffect ) {
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM(ch, pRoom);

  general_show_affects( ch, pRoom->base_affected, argument );

  return FALSE;
}
REDIT( redit_setaffect ) {
  ROOM_INDEX_DATA *pRoom;
  EDIT_ROOM(ch, pRoom);

  return general_set_affect( ch, pRoom->base_affected, argument );
}

struct wear_type
{
  int	wear_loc;
  int	wear_bit;
};



const struct wear_type wear_table[] =
{
  {	WEAR_NONE,	ITEM_TAKE		},
  {	WEAR_LIGHT,	ITEM_LIGHT		},
  {	WEAR_FINGER_L,	ITEM_WEAR_FINGER	},
  {     WEAR_FINGER_R,	ITEM_WEAR_FINGER	},
  {	WEAR_NECK_1,	ITEM_WEAR_NECK		},
  {	WEAR_NECK_2,	ITEM_WEAR_NECK		},
  {	WEAR_BODY,	ITEM_WEAR_BODY		},
  {	WEAR_HEAD,	ITEM_WEAR_HEAD		},
  {	WEAR_LEGS,	ITEM_WEAR_LEGS		},
  {	WEAR_FEET,	ITEM_WEAR_FEET		},
  {	WEAR_HANDS,	ITEM_WEAR_HANDS		},
  {	WEAR_ARMS,	ITEM_WEAR_ARMS		},
  {	WEAR_SHIELD,	ITEM_WEAR_SHIELD	},
  {	WEAR_ABOUT,	ITEM_WEAR_ABOUT		},
  {	WEAR_WAIST,	ITEM_WEAR_WAIST		},
  {	WEAR_WRIST_L,	ITEM_WEAR_WRIST		},
  {	WEAR_WRIST_R,	ITEM_WEAR_WRIST		},
  {	WEAR_WIELD,	ITEM_WIELD		},
  {	WEAR_HOLD,	ITEM_HOLD		},
    // Added by SinaC 2001, was missing
  {	WEAR_FLOAT,	ITEM_WEAR_FLOAT		},
    // Added by SinaC 2001 for brand mark
  //{	WEAR_BRAND,	ITEM_BRAND		},  removed by SinaC 2003
  {	NO_FLAG,	NO_FLAG			}
};



/*****************************************************************************
 Name:		wear_loc
 Purpose:	Returns the location of the bit that matches the count.
 		1 = first match, 2 = second match etc.
 Called by:	oedit_reset(olc_act.c).
 ****************************************************************************/
int wear_loc(int bits, int count)
{
  int flag;
 
  for (flag = 0; wear_table[flag].wear_bit != NO_FLAG; flag++) {
    if ( IS_SET(bits, wear_table[flag].wear_bit) && --count < 1)
      return wear_table[flag].wear_loc;
  }
 
  return NO_FLAG;
}



/*****************************************************************************
 Name:		wear_bit
 Purpose:	Converts a wear_loc into a bit.
 Called by:	redit_oreset(olc_act.c).
 ****************************************************************************/
int wear_bit(int loc)
{
  int flag;
 
  for (flag = 0; wear_table[flag].wear_loc != NO_FLAG; flag++) {
    if ( loc == wear_table[flag].wear_loc )
      return wear_table[flag].wear_bit;
  }
 
  return 0;
}



REDIT( redit_oreset )
{
  ROOM_INDEX_DATA	*pRoom;
  OBJ_INDEX_DATA	*pObjIndex;
  OBJ_DATA		*newobj;
  OBJ_DATA		*to_obj;
  CHAR_DATA		*to_mob;
  char		arg1 [ MAX_INPUT_LENGTH ];
  char		arg2 [ MAX_INPUT_LENGTH ];
  int			olevel = 0;

  RESET_DATA		*pReset;
  char		output [ MAX_STRING_LENGTH ];

  EDIT_ROOM(ch, pRoom);

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || !is_number( arg1 ) ) {
    send_to_char ( "Syntax:  oreset <vnum> <args>\n\r", ch );
    send_to_char ( "        -no_args               = into room\n\r", ch );
    send_to_char ( "        -<obj_name>            = into obj\n\r", ch );
    send_to_char ( "        -<mob_name> <wear_loc> = into mob\n\r", ch );
    return FALSE;
  }

  if ( !( pObjIndex = get_obj_index( atoi( arg1 ) ) ) ) {
    send_to_char( "REdit: No object has that vnum.\n\r", ch );
    return FALSE;
  }

  if ( pObjIndex->area != pRoom->area ) {
    send_to_char( "REdit: No such object in this area.\n\r", ch );
    return FALSE;
  }

  /*
   * Load into room.
   */
  if ( arg2[0] == '\0' ) {
    pReset		= new_reset_data();
    pReset->command	= 'O';
    pReset->arg1	= pObjIndex->vnum;
    pReset->arg2	= 0;
    pReset->arg3	= pRoom->vnum;
    pReset->arg4	= 0;
    add_reset( pRoom, pReset, 0/* Last slot*/ );

    newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
    obj_to_room( newobj, pRoom );

    // Added by SinaC 2001
    recomproom(pRoom);

    sprintf( output, "%s (%d) has been loaded and added to resets.\n\r",
	     capitalize( pObjIndex->short_descr ),
	     pObjIndex->vnum );
    send_to_char( output, ch );
  }
  else
    /*
     * Load into object's inventory.
     */
    if ( argument[0] == '\0'
	 && ( ( to_obj = get_obj_list( ch, arg2, pRoom->contents ) ) != NULL ) ) {
      pReset		= new_reset_data();
      pReset->command	= 'P';
      pReset->arg1	= pObjIndex->vnum;
      pReset->arg2	= 0;
      pReset->arg3	= to_obj->pIndexData->vnum;
      pReset->arg4	= 1;
      add_reset( pRoom, pReset, 0/* Last slot*/ );

      newobj = create_object( pObjIndex, number_fuzzy( olevel ) );
      newobj->cost = 0;
      obj_to_obj( newobj, to_obj );

      sprintf( output, "%s (%d) has been loaded into "
	       "%s (%d) and added to resets.\n\r",
	       capitalize( newobj->short_descr ),
	       newobj->pIndexData->vnum,
	       to_obj->short_descr,
	       to_obj->pIndexData->vnum );
      send_to_char( output, ch );
    }
    else
      /*
       * Load into mobile's inventory.
       */
      if ( ( to_mob = get_char_room( ch, arg2 ) ) != NULL ) {
	int	wear_loc;

	/*
	 * Make sure the location on mobile is valid.
	 */
	if ( (wear_loc = flag_value( wear_loc_flags, argument )) == NO_FLAG ) {
	  send_to_char( "REdit: Invalid wear_loc.  '? wear-loc'\n\r", ch );
	  return FALSE;
	}

	/*
	 * Disallow loading a sword(WEAR_WIELD) into WEAR_HEAD.
	 */
	if ( !IS_SET( pObjIndex->wear_flags, wear_bit(wear_loc) ) ) {
	  sprintf( output,
		   "%s (%d) has wear flags: [%s]\n\r",
		   capitalize( pObjIndex->short_descr ),
		   pObjIndex->vnum,
		   flag_string( wear_flags, pObjIndex->wear_flags ) );
	  send_to_char( output, ch );
	  return FALSE;
	}

	/*
	 * Can't load into same position.
	 */
	if ( wear_loc != -1 ) 
	  if ( get_eq_char( to_mob, wear_loc ) ) {
	    send_to_char( "REdit:  Object already equipped.\n\r", ch );
	    return FALSE;
	  }

	pReset		= new_reset_data();
	pReset->arg1	= pObjIndex->vnum;
	pReset->arg2	= wear_loc;
	if ( pReset->arg2 == WEAR_NONE )
	  pReset->command = 'G';
	else
	  pReset->command = 'E';
	pReset->arg3	= wear_loc;

	add_reset( pRoom, pReset, 0/* Last slot*/ );

	olevel  = URANGE( 0, to_mob->level - 2, LEVEL_HERO );
	newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	if ( to_mob->pIndexData->pShop ) {	/* Shop-keeper? */
	  switch ( pObjIndex->item_type ) {
	  default:		olevel = 0;				break;
	  case ITEM_PILL:	olevel = number_range(  0, 10 );	break;
	  case ITEM_POTION:	olevel = number_range(  0, 10 );	break;
	  case ITEM_SCROLL:
	    // Added by SinaC 2003
	  case ITEM_TEMPLATE:   olevel = number_range(  5, 15 );	break;
	  case ITEM_WAND:	olevel = number_range( 10, 20 );	break;
	  case ITEM_STAFF:	olevel = number_range( 15, 25 );	break;
	  case ITEM_ARMOR:	olevel = number_range(  5, 15 );	break;
	  case ITEM_WEAPON:	if ( pReset->command == 'G' )
	    olevel = number_range( 5, 15 );
	  else
	    olevel = number_fuzzy( olevel );
	  break;
	  }

	  newobj = create_object( pObjIndex, olevel );
	  if ( pReset->arg2 == WEAR_NONE ) { // Modified by SinaC 2001
	    SET_BIT( newobj->extra_flags, ITEM_INVENTORY );
	    SET_BIT( newobj->base_extra, ITEM_INVENTORY );
	  }
	}
	else
	  newobj = create_object( pObjIndex, number_fuzzy( olevel ) );

	obj_to_char( newobj, to_mob );
	if ( pReset->command == 'E' )
	  equip_char( to_mob, newobj, pReset->arg3 );

	sprintf( output, "%s (%d) has been loaded "
		 "%s of %s (%d) and added to resets.\n\r",
		 capitalize( pObjIndex->short_descr ),
		 pObjIndex->vnum,
		 flag_string( wear_loc_strings, pReset->arg3 ),
		 to_mob->short_descr,
		 to_mob->pIndexData->vnum );
	send_to_char( output, ch );
      }
      else	/* Display Syntax */
	{
	  send_to_char( "REdit:  That mobile isn't here.\n\r", ch );
	  return FALSE;
	}

  act( "$n has created $p!", ch, newobj, NULL, TO_ROOM );
  return TRUE;
}



/*
 * Object Editor Functions.
 */
void show_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *obj )
{
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];

  switch( obj->item_type ) {
  default:	/* No values. */
    break;

  case ITEM_COMPONENT: // SinaC 2003
    if ( obj->value[0] == NO_FLAG ) sprintf( buf, "No component" );
    else sprintf( buf, "%s", flag_string(brew_component_flags, obj->value[0]) );
    if ( obj->value[1] == NO_FLAG ) sprintf( buf1, "No component" );
    else sprintf( buf1, "%s", flag_string(brew_component_flags, obj->value[1]) );
    if ( obj->value[2] == NO_FLAG ) sprintf( buf2, "No component" );
    else sprintf( buf2, "%s", flag_string(brew_component_flags, obj->value[2]) );
    if ( obj->value[3] == NO_FLAG ) sprintf( buf3, "No component" );
    else sprintf( buf3, "%s", flag_string(brew_component_flags, obj->value[3]) );
    if ( obj->value[4] == NO_FLAG ) sprintf( buf4, "No component" );
    else sprintf( buf4, "%s", flag_string(brew_component_flags, obj->value[4]) );
    send_to_charf( ch,
		   "[v0] Component:      [%s]\n\r"
		   "[v1] Component:      [%s]\n\r"
		   "[v2] Component:      [%s]\n\r"
		   "[v3] Component:      [%s]\n\r"
		   "[v4] Component:      [%s]\n\r",
		   buf, buf1, buf2, buf3, buf4 );
    break;

    //case ITEM_SKELETON: // SinaC 2003
  case ITEM_CORPSE_NPC:
  case ITEM_CORPSE_PC:
    sprintf( buf2, flag_string( form_flags, obj->value[1]) );
    sprintf( buf3, flag_string( part_flags, obj->value[2]) );
    sprintf( buf4, flag_string( part_flags, obj->value[4]) );
    sprintf( buf,
	     "[v0] Vnum:           [%d]\n\r"
	     "[v1] Form:           %s\n\r"
	     "[v2] Parts:          %s\n\r"
	     "[v3] Skeleton:       %s\n\r"
	     "[v4] Missing parts:  %s\n\r",
	     obj->value[0], buf2, buf3, IS_SET( obj->value[3], CORPSE_SKELETON )?"Yes":"No", buf4 );
    send_to_charf( ch, buf );
    break;

    // Added by SinaC 2000:
  case ITEM_MAP:
    sprintf( buf, "[v0] Player keeps it when leaving: [%s]\n\r",
	     obj->value[0] == 0 ?"NO":"YES" );
    send_to_char(buf,ch);
    break;
      
  case ITEM_LIGHT:
    if ( obj->value[2] == -1 || obj->value[2] == 999 ) /* ROM OLC */
      sprintf( buf, "[v2] Light:  Infinite[-1]\n\r" );
    else
      sprintf( buf, "[v2] Light:  [%d]\n\r", obj->value[2] );
    send_to_char( buf, ch );
    break;

    // SinaC 2000 for window
  case ITEM_WINDOW:
    sprintf( buf, "[v0] To Room    [%d]\n\r",obj->value[0]);
    send_to_char( buf, ch );
    break;
    /* Removed by SinaC 2003, can be emulate with script
    // grenade SinaC 2000
  case ITEM_GRENADE:
    sprintf( buf,
	     "[v0] Timer           [%d]\n\r"
	     "[v1] Unused          [?]\n\r"
	     "[v2] Damage          [%d]\n\r"
	     "[v3] Unused          [?]\n\r"
	     "[v4] Unused          [?]\n\r",
	     obj->value[0],
	     obj->value[2]);
    send_to_char( buf, ch );
    break;
    */
    /* Removed by SinaC 2003
    // throwing item SinaC 2000
  case ITEM_THROWING:
    sprintf(buf,
	    "[v0] Number of dice: [%d]\n\r"
	    "[v1] Type of dice:   [%d]\n\r"
	    "[v2] Damage type:    [%s]\n\r"
	    "[v3] Spell level:    [%d]\n\r"
	    "[v4] Spell:          [%s]\n\r",
	    obj->value[0], 
	    obj->value[1],
	    attack_table[obj->value[2]].name,
	    obj->value[3],
	    obj->value[4] != -1 ? ability_table[obj->value[4]].name:"none"
	    );
    send_to_char(buf,ch);
    break;
    */
  case ITEM_WAND:
  case ITEM_STAFF:
    sprintf( buf,
	     "[v0] Level:          [%d]\n\r"
	     "[v1] Charges Total:  [%d]\n\r"
	     "[v2] Charges Left:   [%d]\n\r"
	     "[v3] Spell:          %s\n\r",
	     obj->value[0],
	     obj->value[1],
	     obj->value[2],
	     obj->value[3] != -1 ? ability_table[obj->value[3]].name
	     : "none" );
    send_to_char( buf, ch );
    break;
      
  case ITEM_PORTAL:
    sprintf( buf2, flag_string( exit_flags, obj->value[1]));
    sprintf( buf3, flag_string( portal_flags , obj->value[2]));
    sprintf( buf,
	     "[v0] Charges:        [%d]\n\r"
	     "[v1] Exit Flags:     %s\n\r"
	     "[v2] Portal Flags:   %s\n\r"
	     "[v3] Goes to (vnum): [%d]\n\r",
	     obj->value[0],
	     buf2,
	     buf3,
	     obj->value[3] );
    send_to_char( buf, ch);
    break;
	    
  case ITEM_FURNITURE:          
    sprintf( buf,
	     "[v0] Max people:      [%d]\n\r"
	     "[v1] Max weight:      [%d]\n\r"
	     "[v2] Furniture Flags: %s\n\r"
	     "[v3] Heal bonus:      [%d]\n\r"
	     "[v4] Mana bonus:      [%d]\n\r",
	     obj->value[0],
	     obj->value[1],
	     flag_string( furniture_flags, obj->value[2]),
	     obj->value[3],
	     obj->value[4] );
    send_to_char( buf, ch );
    break;

  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_PILL:
    // Added by SinaC 2003
  case ITEM_TEMPLATE:  
    sprintf( buf,
	     "[v0] Level:  [%d]\n\r"
	     "[v1] Spell:  %s\n\r"
	     "[v2] Spell:  %s\n\r"
	     "[v3] Spell:  %s\n\r"
	     "[v4] Spell:  %s\n\r",
	     obj->value[0],
	     obj->value[1] != -1 ? ability_table[obj->value[1]].name
	     : "none",
	     obj->value[2] != -1 ? ability_table[obj->value[2]].name
	     : "none",
	     obj->value[3] != -1 ? ability_table[obj->value[3]].name
	     : "none",
	     obj->value[4] != -1 ? ability_table[obj->value[4]].name
	     : "none" );
    send_to_char( buf, ch );
    break;

    /* ARMOR for ROM */

  case ITEM_ARMOR:
    sprintf( buf,
	     "[v0] Ac pierce       [%d]\n\r"
	     "[v1] Ac bash         [%d]\n\r"
	     "[v2] Ac slash        [%d]\n\r"
	     "[v3] Ac exotic       [%d]\n\r",
	     obj->value[0],
	     obj->value[1],
	     obj->value[2],
	     obj->value[3] );
    send_to_char( buf, ch );
    break;

    /* WEAPON changed in ROM: */
    /* I had to split the output here, I have no idea why, but it helped -- Hugin */
    /* It somehow fixed a bug in showing scroll/pill/potions too ?! */
    // SinaC 2003: because of flag_string which work in a static buffer, so calling it
    //  a second time in the same sprintf will cause problem (buffer will be overwritten)
  case ITEM_WEAPON:
    sprintf( buf, "[v0] Weapon class:      %s\n\r",
	     flag_string( weapon_class, obj->value[0] ) );
    send_to_char( buf, ch );
    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_charf( ch, "[v1] String condition:  [%d]\n\r", obj->value[1] );
      send_to_charf( ch, "[v2] String cond. mod.: [%d]\n\r", obj->value[2] );
      send_to_charf( ch, "[v3] Strength:          [%d]\n\r", obj->value[3] );
      send_to_charf( ch, "[v4] Max distance:      [%d]\n\r", obj->value[4] );
      send_to_charf( ch, "Number of dice:         [%d]\n\r", GET_WEAPON_DNUMBER(obj));
      send_to_charf( ch, "Type of dice:           [%d]\n\r", GET_WEAPON_DTYPE(obj));
    }
    else {
      sprintf( buf, "[v1] Number of dice:    [%d]\n\r", obj->value[1] );
      send_to_char( buf, ch );
      sprintf( buf, "[v2] Type of dice:      [%d]\n\r", obj->value[2] );
      send_to_char( buf, ch );
      sprintf( buf, "[v3] Type:              %s\n\r",
	       attack_table[obj->value[3]].name );
      send_to_char( buf, ch );
      sprintf( buf, "[v4] Special type:      %s\n\r",
	       flag_string( weapon_type2,  obj->value[4] ) );
      send_to_char( buf, ch );
    }
    break;

  case ITEM_CONTAINER:
    sprintf( buf,
	     "[v0] Weight:     [%d kg]\n\r"
	     "[v1] Flags:      [%s]\n\r"
	     "[v2] Key:     %s [%d]\n\r"
	     "[v3] Capacity    [%d]\n\r"
	     "[v4] Weight Mult [%d]\n\r",
	     obj->value[0],
	     flag_string( container_flags, obj->value[1] ),
	     get_obj_index(obj->value[2]) ? get_obj_index(obj->value[2])->short_descr : "none",
	     obj->value[2],
	     obj->value[3],
	     obj->value[4] );
    send_to_char( buf, ch );
    break;

  case ITEM_DRINK_CON:
    sprintf( buf,
	     "[v0] Liquid Total: [%d]\n\r"
	     "[v1] Liquid Left:  [%d]\n\r"
	     "[v2] Liquid:       %s\n\r"
	     "[v3] Poisoned:     %s\n\r",
	     obj->value[0],
	     obj->value[1],
	     liq_table[obj->value[2]].liq_name,
	     obj->value[3] != 0 ? "Yes" : "No" );
    send_to_char( buf, ch );
    break;

  case ITEM_FOUNTAIN:
    sprintf( buf,
	     "[v0] Liquid Total: [%d]\n\r"
	     "[v1] Liquid Left:  [%d]\n\r"
	     "[v2] Liquid:	    %s\n\r",
	     obj->value[0],
	     obj->value[1],
	     liq_table[obj->value[2]].liq_name );
    send_to_char( buf,ch );
    break;
	        
  case ITEM_FOOD:
    sprintf( buf,
	     "[v0] Food hours: [%d]\n\r"
	     "[v1] Full hours: [%d]\n\r"
	     "[v3] Poisoned:   %s\n\r",
	     obj->value[0],
	     obj->value[1],
	     obj->value[3] != 0 ? "Yes" : "No" );
    send_to_char( buf, ch );
    break;

  case ITEM_MONEY:
    sprintf( buf, "[v0] Silver:   [%d]\n\r", obj->value[0] );
    send_to_char( buf, ch );
    break;

  /*  Removed by SinaC 2003
  // Added by SinaC 2001 for levers
  case ITEM_LEVER:
    sprintf( buf, "[v0] Pulled:    [%s]\n\r",
	     obj->value[0]==LEVER_PULLEDDOWN?"down":"up");
    send_to_char( buf, ch );
    break;
  */
  }

  return;
}


// Added by SinaC 2003
void set_obj_spell( int &v, const char *argument, CHAR_DATA *ch, const char *okay ) {
  int sn;
  sn = ability_lookup( argument );
  if ( sn <= 0 || !ability_table[sn].craftable )
    send_to_char("SPELL DOESN'T EXIST OR IS NOT CRAFTABLE.\n\r", ch );
  else {
    send_to_char( okay, ch );
    v = sn;
  }
}

bool set_obj_values( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, int value_num, const char *argument)
{
  // Added by SinaC 2001 to avoid too high average weapon damage
  int old_v1, old_v2, old_v;

  switch( pObj->item_type ) {
  default:
    break;

    /*Removed by SinaC 2003
// Added by SinaC 2001 for levers
  case ITEM_LEVER:
    if ( value_num == 0 )
      if ( !str_prefix( argument, "down" ) ) {
	send_to_char("LEVER PULLED DOWN.\n\r\n\r", ch );
	pObj->value[0] = LEVER_PULLEDDOWN;
      }
      else if ( !str_prefix( argument, "up" ) ) {
	send_to_char("LEVER PULLED UP.\n\r\n\r", ch );
	pObj->value[0] = LEVER_PULLEDUP;
      }
      else {
	send_to_char("Valid v0 value: up / down.\n\r", ch );
	return FALSE;
      }
    else {
      send_to_char("Only value0 can be set.\n\r", ch );
      return FALSE;
    }
    break;
    */
  case ITEM_COMPONENT:
    old_v = pObj->value[value_num];
    pObj->value[value_num] = flag_value(brew_component_flags, argument);
    if ( pObj->value[value_num] == NO_FLAG ) {
      send_to_charf(ch,"Unknown component [%s]\n\r"
		    "Type '? component' for a list of available components\n\r", argument );
      pObj->value[value_num] = old_v;
    }
    else
      send_to_charf(ch,"COMPONENT %d SET.\n\r", value_num );
    break;

    //  case ITEM_SKELETON: // SinaC 2003
  case ITEM_CORPSE_NPC:
  case ITEM_CORPSE_PC:
    switch( value_num ) {
    case 0:
      send_to_charf(ch,"MOB VNUM SET.\n\r\n\r");
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_charf(ch,"FORM TOGGLED.\n\r\n\r");
      pObj->value[1] ^= flag_value( form_flags, argument );
      break;
    case 2:
      send_to_charf(ch,"PARTS TOGGLED.\n\r\n\r");
      pObj->value[2] ^= flag_value( part_flags, argument );
      break;
    case 3:
      if ( !str_cmp( argument, "skeleton" ) ) {
	SET_BIT( pObj->value[3], CORPSE_SKELETON );
	send_to_charf(ch,"SKELETON SET.\n\r");
      }
      else if ( !str_cmp( argument, "corpse" ) ) {
	REMOVE_BIT( pObj->value[3], CORPSE_SKELETON );
	send_to_charf(ch,"SKELETON REMOVED.\n\r");
      }
      else
	send_to_charf(ch,"Please specify: skeleton or corpse as argument.\n\r");
      break;
    case 4:
      send_to_charf(ch,"MISSING PARTS TOGGLED.\n\r\n\r");
      pObj->value[2] ^= flag_value( part_flags, argument );
      break;
    }
    break;

    // Added by SinaC 2000
  case ITEM_MAP:
    if ( value_num == 0 ) {
      send_to_char("MAP KEEP STATUS SET.\n\r\n\r",ch);
      pObj->value[0] = atoi( argument );
      break;
    }
    else{
      send_to_char("Only value0 can be set.\n\r"
		   "0: player doesn't keep the map when leaving\n\r"
		   "1: player keep the map when leaving",ch );
      return FALSE;
    }
    break;

  case ITEM_LIGHT:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_LIGHT" );
      return FALSE;
    case 2:
      send_to_char( "HOURS OF LIGHT SET.\n\r\n\r", ch );
      pObj->value[2] = atoi( argument );
      break;
    }
    break;
    // SinaC 2000 for window
  case ITEM_WINDOW:
    switch ( value_num ) {
    default:
      send_to_char( "set v0 to the destination window, or to zero if you wish to use the windows description \n\r",ch);
      return FALSE;
    case 0:
      send_to_char( "VIEWING ROOM SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;       
    }
    break;
        /* Removed by SinaC 2003, can be emulate with script
    // grenade by SinaC 2000
  case ITEM_GRENADE:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_GRENADE" );
      return FALSE;
    case 0:
      send_to_char( "TIMER SET.\n\r\n\r", ch);
      pObj->value[0] = atoi( argument );
      break;
    case 2:
      send_to_char( "DAMAGE SET.\n\r\n\r", ch );
      pObj->value[2] = atoi( argument );
      break;
    }
    break;
	*/
    /* Removed by SinaC 2003
    // throwing item by SinaC 2000
  case ITEM_THROWING:
    switch ( value_num ) {
    case 0 : 
      send_to_char("NUMBER OF DICE SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument ); 
      break;
    case 1 : 
      send_to_char("TYPE OF DICE SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument ); 
      break;
    case 2 : 
      send_to_char("DAMAGE TYPE SET.\n\r\n\r", ch );
      pObj->value[2] = attack_lookup( argument ); 
      break;
    case 3 : 
      send_to_char("SPELL LEVEL SET.\n\r\n\r", ch );
      pObj->value[3] = atoi( argument ); 
      break;
    case 4 : 
      send_to_char("SPELL SET.\n\r\n\r", ch );
      pObj->value[4] = ability_lookup( argument ); 
      break;
    }
    break;
    */
  case ITEM_WAND:
  case ITEM_STAFF:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_STAFF_WAND" );
      return FALSE;
    case 0:
      send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_char( "TOTAL NUMBER OF CHARGES SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument );
      break;
    case 2:
      send_to_char( "CURRENT NUMBER OF CHARGES SET.\n\r\n\r", ch );
      pObj->value[2] = atoi( argument );
      break;
    case 3:
      set_obj_spell( pObj->value[3], argument, ch, "SPELL TYPE SET.\n\r\n\r" );
      break;
    }
    break;

  case ITEM_SCROLL:
  case ITEM_POTION:
  case ITEM_PILL:
    // Added by SinaC 2003
  case ITEM_TEMPLATE:  
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_SCROLL_POTION_PILL" );
      return FALSE;
    case 0:
      send_to_char( "SPELL LEVEL SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      //send_to_char( "SPELL TYPE 1 SET.\n\r\n\r", ch );
      //pObj->value[1] = ability_lookup( argument );
      set_obj_spell( pObj->value[1], argument, ch, "SPELL TYPE 1 SET.\n\r\n\r" );
      break;
    case 2:
      //send_to_char( "SPELL TYPE 2 SET.\n\r\n\r", ch );
      //pObj->value[2] = ability_lookup( argument );
      set_obj_spell( pObj->value[2], argument, ch, "SPELL TYPE 2 SET.\n\r\n\r" );
      break;
    case 3:
      //send_to_char( "SPELL TYPE 3 SET.\n\r\n\r", ch );
      //pObj->value[3] = ability_lookup( argument );
      set_obj_spell( pObj->value[3], argument, ch, "SPELL TYPE 3 SET.\n\r\n\r" );
      break;
    case 4:
      //send_to_char( "SPELL TYPE 4 SET.\n\r\n\r", ch );
      //pObj->value[4] = ability_lookup( argument );
      set_obj_spell( pObj->value[4], argument, ch, "SPELL TYPE 4 SET.\n\r\n\r" );
      break;
    }
    break;

    /* ARMOR for ROM: */

  case ITEM_ARMOR:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_ARMOR" );
      return FALSE;
    case 0:
      send_to_char( "AC PIERCE SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_char( "AC BASH SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument );
      break;
    case 2:
      send_to_char( "AC SLASH SET.\n\r\n\r", ch );
      pObj->value[2] = atoi( argument );
      break;
    case 3:
      send_to_char( "AC EXOTIC SET.\n\r\n\r", ch );
      pObj->value[3] = atoi( argument );
      break;
    }
    break;

    /* WEAPONS changed in ROM */

  case ITEM_WEAPON:
    // Added by SinaC 2001
    old_v1 = pObj->value[1];
    old_v2 = pObj->value[2];
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_WEAPON" );
      return FALSE;
    case 0:
      send_to_char( "WEAPON CLASS SET.\n\r\n\r", ch );
      pObj->value[0] = flag_value( weapon_class, argument );
      if ( pObj->value[0] == WEAPON_RANGED ) {
	pObj->value[1] = MAX_STRING_CONDITION;
	pObj->value[2] = 10;
	pObj->value[3] = 0;
	pObj->value[4] = 1;
      }
      break;
    case 1:
      if ( pObj->value[0] == WEAPON_RANGED ) {
	send_to_char("STRING CONDITION SET.\n\r\n\r", ch );
	pObj->value[1] = URANGE(0,atoi(argument),MAX_STRING_CONDITION);
      }
      else {
	send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
	pObj->value[1] = atoi( argument );
      }
      break;
    case 2:
      if ( pObj->value[0] == WEAPON_RANGED ) {
	send_to_char( "STRING CONDITION MODIFIER PROBABILITY SET.\n\r\n\r", ch );
	pObj->value[2] = URANGE(0,atoi(argument),100);
      }
      else {
	send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
	pObj->value[2] = atoi( argument );
      }
      break;
    case 3:
      if ( pObj->value[0] == WEAPON_RANGED ) {
	send_to_char("BOW STRENGTH SET.\n\r\n\r", ch );
	pObj->value[3] = URANGE(0,atoi(argument),MAX_RANGED_STRENGTH);
      }
      else {
	send_to_char( "WEAPON TYPE SET.\n\r\n\r", ch );
	pObj->value[3] = attack_lookup( argument );
      }
      break;
    case 4:
      if ( pObj->value[0] == WEAPON_RANGED ) {
	send_to_char("MAX DISTANCE SET.\n\r\n\r", ch );
	pObj->value[4] = URANGE(1,atoi(argument),MAX_RANGED_DISTANCE);
      }
      else {
	send_to_char( "SPECIAL WEAPON TYPE TOGGLED.\n\r\n\r", ch );
	pObj->value[4] ^= (flag_value( weapon_type2, argument ) != NO_FLAG
			   ? flag_value( weapon_type2, argument ) : 0 );
      }
      break;
    }
    // Added by SinaC 2001
    if ( pObj->value[0] != WEAPON_RANGED 
	 && ( (1 + pObj->value[2]) * pObj->value[1] / 2) > 500 ) {
      send_to_charf(ch,
		    ">>>Too high average damage\n\r"
		    "Setting back to v1:%d  and v2:%d<<<\n\r\n\r", 
		    old_v1, old_v2);
      pObj->value[1] = old_v1;
      pObj->value[2] = old_v2;
    }
    break;

  case ITEM_PORTAL:
    switch ( value_num ) {
    default:
      do_help(ch, "ITEM_PORTAL" );
      return FALSE;
	            
    case 0:
      send_to_char( "CHARGES SET.\n\r\n\r", ch);
      pObj->value[0] = atoi ( argument );
      break;
    case 1:
      send_to_char( "EXIT FLAGS SET.\n\r\n\r", ch);
      pObj->value[1] = flag_value( exit_flags, argument );
      break;
    case 2:
      send_to_char( "PORTAL FLAGS SET.\n\r\n\r", ch);
      pObj->value[2] = flag_value( portal_flags, argument );
      break;
    case 3:
      send_to_char( "EXIT VNUM SET.\n\r\n\r", ch);
      pObj->value[3] = atoi ( argument );
      break;
    }
    break;

  case ITEM_FURNITURE:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_FURNITURE" );
      return FALSE;
	            
    case 0:
      send_to_char( "NUMBER OF PEOPLE SET.\n\r\n\r", ch);
      pObj->value[0] = atoi ( argument );
      break;
    case 1:
      send_to_char( "MAX WEIGHT SET.\n\r\n\r", ch);
      pObj->value[1] = atoi ( argument );
      break;
    case 2:
      send_to_char( "FURNITURE FLAGS TOGGLED.\n\r\n\r", ch);
      pObj->value[2] ^= (flag_value( furniture_flags, argument ) != NO_FLAG
			 ? flag_value( furniture_flags, argument ) : 0);
      break;
    case 3:
      send_to_char( "HEAL BONUS SET.\n\r\n\r", ch);
      pObj->value[3] = atoi ( argument );
      break;
    case 4:
      send_to_char( "MANA BONUS SET.\n\r\n\r", ch);
      pObj->value[4] = atoi ( argument );
      break;
    }
    break;
	   
  case ITEM_CONTAINER:
    switch ( value_num ) {
      int value;
		
    default:
      do_help( ch, "ITEM_CONTAINER" );
      return FALSE;
    case 0:
      send_to_char( "WEIGHT CAPACITY SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      if ( ( value = flag_value( container_flags, argument ) )
	   != NO_FLAG )
	TOGGLE_BIT(pObj->value[1], value);
      else {
	do_help ( ch, "ITEM_CONTAINER" );
	return FALSE;
      }
      send_to_char( "CONTAINER TYPE SET.\n\r\n\r", ch );
      break;
    case 2:
      if ( atoi(argument) != 0 ) {
	if ( !get_obj_index( atoi( argument ) ) )	{
	  send_to_char( "THERE IS NO SUCH ITEM.\n\r\n\r", ch );
	  return FALSE;
	}

	if ( get_obj_index( atoi( argument ) )->item_type != ITEM_KEY ) {
	  send_to_char( "THAT ITEM IS NOT A KEY.\n\r\n\r", ch );
	  return FALSE;
	}
      }
      send_to_char( "CONTAINER KEY SET.\n\r\n\r", ch );
      pObj->value[2] = atoi( argument );
      break;
    case 3:
      send_to_char( "CONTAINER MAX WEIGHT SET.\n\r", ch);
      pObj->value[3] = atoi( argument );
      break;
    case 4:
      send_to_char( "WEIGHT MULTIPLIER SET.\n\r\n\r", ch );
      pObj->value[4] = atoi ( argument );
      break;
    }
    break;

  case ITEM_DRINK_CON:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_DRINK" );
      /* OLC		    do_help( ch, "liquids" );    */
      return FALSE;
    case 0:
      send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument );
      break;
    case 2:
      send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
      pObj->value[2] = ( liq_lookup(argument) != -1 ?
			 liq_lookup(argument) : 0 );
      break;
    case 3:
      send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
      pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
      break;
    }
    break;

  case ITEM_FOUNTAIN:
    switch (value_num) {
    default:
      do_help( ch, "ITEM_FOUNTAIN" );
      /* OLC		    do_help( ch, "liquids" );    */
      return FALSE;
    case 0:
      send_to_char( "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_char( "CURRENT AMOUNT OF LIQUID HOURS SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument );
      break;
    case 2:
      send_to_char( "LIQUID TYPE SET.\n\r\n\r", ch );
      pObj->value[2] = ( liq_lookup( argument ) != -1 ?
			 liq_lookup( argument ) : 0 );
      break;
    }
    break;
		    	
  case ITEM_FOOD:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_FOOD" );
      return FALSE;
    case 0:
      send_to_char( "HOURS OF FOOD SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_char( "HOURS OF FULL SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument );
      break;
    case 3:
      send_to_char( "POISON VALUE TOGGLED.\n\r\n\r", ch );
      pObj->value[3] = ( pObj->value[3] == 0 ) ? 1 : 0;
      break;
    }
    break;

  case ITEM_MONEY:
    switch ( value_num ) {
    default:
      do_help( ch, "ITEM_MONEY" );
      return FALSE;
    case 0:
      send_to_char( "GOLD AMOUNT SET.\n\r\n\r", ch );
      pObj->value[0] = atoi( argument );
      break;
    case 1:
      send_to_char( "SILVER AMOUNT SET.\n\r\n\r", ch );
      pObj->value[1] = atoi( argument );
      break;
    }
    break;
  }

  show_obj_values( ch, pObj );

  return TRUE;
}



OEDIT( oedit_show ) {
  OBJ_INDEX_DATA *pObj;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  int cnt;
  // Added by SinaC 2000
  RESTR_DATA *restr;
  ABILITY_UPGRADE *upgr;

  EDIT_OBJ(ch, pObj);

  sprintf( buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
	   pObj->name,
	   !pObj->area ? -1        : pObj->area->vnum,
	   !pObj->area ? "No Area" : pObj->area->name );
  send_to_char( buf, ch );


  sprintf( buf, "Vnum:        [%5d]\n\rType:        [%s]\n\r",
	   pObj->vnum,
	   flag_string( type_flags, pObj->item_type ) );
  send_to_char( buf, ch );

  sprintf( buf, "Level:       [%5d]\n\r", pObj->level );
  send_to_char( buf, ch );

  sprintf( buf, "Wear flags:  [%s]\n\r",
	   flag_string( wear_flags, pObj->wear_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Extra flags: [%s]\n\r",
	   flag_string( extra_flags, pObj->extra_flags ) );
  send_to_char( buf, ch );

  // Modified by SinaC 2001
  //sprintf( buf, "Material:    [%s]\n\r",                /* ROM */
  //   pObj->material );
  sprintf( buf, "Material:    [%s]\n\r",
	   flag_string( material_flags, pObj->material ) );
  send_to_char( buf, ch );

  sprintf( buf, "Condition:   [%5d]\n\r",               /* ROM */
	   pObj->condition );
  send_to_char( buf, ch );

  sprintf( buf, "Weight:      [%5d]\n\rCost:        [%5d]\n\r",
	   pObj->weight, pObj->cost );
  send_to_char( buf, ch );

  // Added by SinaC 2003
  sprintf( buf, "Size:        [%s]\n\r",
	   flag_string( size_flags, pObj->size ) );
  send_to_char( buf, ch );

  if (pObj->program)
    send_to_charf(ch,"Program :    [%s]\n\r", pObj->program->name );


  if ( pObj->extra_descr ) {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "Ex desc kwd: ", ch );

    for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
      send_to_char( "[", ch );
      send_to_char( ed->keyword, ch );
      send_to_char( "]", ch );
    }

    send_to_char( "\n\r", ch );
  }

  sprintf( buf, "Short desc:  %s\n\rLong desc:\n\r     %s\n\r",
	   pObj->short_descr, pObj->description );
  send_to_char( buf, ch );

  // Added by SinaC 2000 for skill/spell upgrade
  for ( cnt = 0, upgr = pObj->upgrade; upgr; upgr = upgr->next ){
    // Modified by SinaC 2001
    if ( cnt == 0 ) 
      send_to_charf(ch,
		    "Number   %s          name            Value\n\r"
		    "------ --------------------          -----\n\r",
		    abilitytype_name(ability_table[upgr->sn].type));
    if (upgr->value>0)
      sprintf(buf2,"+%d",upgr->value);
    else
      sprintf(buf2,"-%d",-upgr->value);
    send_to_charf(ch,"[%4d] %20s    %10s%%\n\r",
		  cnt,
		  ability_table[upgr->sn].name,
		  buf2);
    cnt++;
  }
  // End of skill/spell upgrade

  // Added by SinaC 2000 for object restriction
  for ( cnt = 0, restr = pObj->restriction; restr; restr = restr->next ) {
    if ( cnt == 0 ) send_to_char("Number      Restriction       Op        Value\n\r"
			         "------ --------------------   ---       -----\n\r",ch);
    // Added by SinaC 2000 for skill/spell restriction
    send_to_charf(ch,"[%4d] %20s   ",
		  cnt,
		  //restr->ability_r?
		  restr->type == RESTR_ABILITY?
		  ability_table[restr->sn].name:
		  flag_string( restr_flags, restr->type ));
    // Modified by SinaC 2000 for skill/spell restriction
    if ( restr->type == RESTR_ABILITY//restr->ability_r
	 || restr_table[restr->type].bits == NULL ) {
      send_to_charf(ch,"%3s %11d%s\n\r",
		    // Modified by SinaC 2000 for NOT restriction
		    restr->not_r?"<":">=",
		    restr->value,
		    restr->type == RESTR_ABILITY//restr->ability_r
		    ?"%":"");
    }
    else {
      send_to_charf(ch,"%3s %11s\n\r",
		    // Modified by SinaC 2000 for NOT restriction
		    restr->not_r?
		    (is_stat(restr_table[restr->type].bits)?"!=":"nor"):
		    (is_stat(restr_table[restr->type].bits)?"=":"or"),
		    flag_string( restr_table[restr->type].bits, restr->value ));
    }
    cnt++;
  }
  // End of object restriction

  general_show_affects( ch, pObj->affected, "auto" );
 
  show_obj_values( ch, pObj );

  return FALSE;
}


OEDIT( oedit_points ) {
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ(ch, pObj);
  
  int problem; // not used
  int points = get_item_points( pObj, problem );
  if ( problem > 0 || pObj->level != points ) { // non-balanced item
    if ( problem > 0 )
      send_to_charf( ch, "Item points: %d (should be equal to level: %d)  => problem %s\n\r",
		     points, pObj->level, problem_list[problem]);
    else
      send_to_charf( ch, "Item points: %d (should be equal to level: %d)\n\r",
		     points, pObj->level);
    send_to_charf( ch, "Check help about POINTS to see how to balance items.\n\r");
  }
  else // balanced item
    send_to_charf( ch, "Item points: %d (level: %d)  => item is balanced\n\r",
		   points, pObj->level);

  return FALSE;
}

OEDIT( oedit_program )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  program [program name/none]\n\r", ch );
    return FALSE;
  }

  if ( !str_cmp( argument, "none" ) ) {
    pObj->program = NULL;

    send_to_char( "program removed.\n\r", ch);
    return TRUE;
  }

  CLASS_DATA *cla;
  if ( ( cla = silent_hash_get_prog( argument ) ) ) {
    if ( get_root_class(cla) == default_obj_class ) {
      if ( cla->isAbstract ) {
	send_to_char("This is an abstract program, it must be derivated before being used.\n\r",ch);
	return FALSE;
      }
      pObj->program = cla;
      send_to_char( "Program set.\n\r", ch);
      return TRUE;
    } 
    else {
      send_to_char("Not an obj program!!\n\r", ch);
      return FALSE;
    }
  }

  send_to_char( "OEdit: No such program.\n\r", ch );
  return FALSE;
}


OEDIT( oedit_addaffect ) {
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ(ch, pObj);

  return general_add_affect( ch, pObj->affected, OBJ_ENTITY, pObj, pObj->level, argument );
}

//OEDIT( oedit_addaffect ) {
//  int vloc, vop;
//  long vmod;
//  OBJ_INDEX_DATA *pObj;
//  AFFECT_DATA *pAf;
//  char loc[MAX_STRING_LENGTH];
//  const char * mod;
//
//  EDIT_OBJ(ch, pObj);
//
//  mod = one_argument( argument, loc );
//
//  if ( loc[0] == '\0' || mod[0] == '\0') {
//    send_to_char( "Syntax:  addaffect [attrib] [modifier]\n\r", ch );
//    return FALSE;
//  }
//
//  if ( ( vloc = flag_value( attr_flags, loc ) ) == NO_FLAG ) {
//    send_to_char( "Valid affects are:\n\r", ch );
//    show_help( ch, "attribs" );
//    return FALSE;
//  }
//
//  if ( attr_table[vloc].bits == NULL) {
//    if (!is_number(mod)) {
//      send_to_char( "Modifier for this location is numeric.\n\r", ch );
//      return FALSE;
//    }
//    vmod = atoi(mod);
//    vop = AFOP_ADD;
//  } 
//  else {
//    if ( ( vmod = flag_value( attr_table[vloc].bits, mod ) ) == NO_FLAG ) {
//      send_to_char( "Valid modifiers are:\n\r", ch );
//      show_flag_cmds(ch,attr_table[vloc].bits);
//      return FALSE;
//    }
//    vop = is_stat(attr_table[vloc].bits) ? AFOP_ASSIGN : AFOP_OR;        
//  }
//
//  pAf             =   new_affect();
//  createaff( *pAf, -1, pObj->level, -1, 0, AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT );
//  addaff2( *pAf, AFTO_CHAR, vloc, vop, vmod );
//  //pAf->where      =   AFTO_CHAR;
//  //pAf->location   =   vloc;
//  //pAf->op         =   vop;
//  //pAf->modifier   =   vmod;
//  //pAf->duration   =   -1;
//  //pAf->level      =   pObj->level;
//  //pAf->type       =   -1;
//  pAf->next       =   pObj->affected;
//  pObj->affected  =   pAf;
//
//  send_to_char( "Affect added.\n\r", ch);
//  return TRUE;  
//}

OEDIT( oedit_addapply ) {
  send_to_char( "This command is temporarily not working. Try addaffect instead.\n\r", ch );
  return FALSE;

  /*
    int value,bv,typ;
    OBJ_INDEX_DATA *pObj;
    AFFECT_DATA *pAf;
    char loc[MAX_STRING_LENGTH];
    char mod[MAX_STRING_LENGTH];
    char type[MAX_STRING_LENGTH];
    char bvector[MAX_STRING_LENGTH];

    EDIT_OBJ(ch, pObj);

    argument = one_argument( argument, type );
    argument = one_argument( argument, loc );
    argument = one_argument( argument, mod );
    one_argument( argument, bvector );


    if ( type[0] == '\0' || ( typ = flag_value( apply_types, type ) ) == NO_FLAG )
    {
    send_to_char( "Invalid apply type. Valid apply types are:\n\r", ch);
    show_help( ch, "apptype" );
    return FALSE;
    }

    if ( loc[0] == '\0' || ( value = flag_value( apply_flags, loc ) ) == NO_FLAG )
    {
    send_to_char( "Valid applys are:\n\r", ch );
    show_help( ch, "apply" );
    return FALSE;
    }


    bv = flag_value( bitvector_type[typ].table, bvector );
 
    if ( bv == NO_FLAG )
    {
    send_to_char( "Invalid bitvector type.\n\r", ch );
    send_to_char( "Valid bitvector types are:\n\r", ch );
    show_help( ch, bitvector_type[typ].help );
    return FALSE;
    }

    if ( mod[0] == '\0' || !is_number( mod ) )
    {
    send_to_char( "Syntax:  addapply [type] [location] [#xmod] [bitvector]\n\r", ch );
    return FALSE;
    }

    pAf             =   new_affect();
    pAf->location   =   value;
    pAf->modifier   =   atoi( mod );
    pAf->where	    =   apply_types[typ].bit;
    pAf->type	    =	-1;
    pAf->duration   =   -1;
    pAf->bitvector  =   bv;
    pAf->level      =	pObj->level;
    pAf->next       =   pObj->affected;
    pObj->affected  =   pAf;

    send_to_char( "Apply added.\n\r", ch);
    return TRUE;

  */
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OEDIT( oedit_delaffect ) {
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ(ch, pObj);

  return general_del_affect( ch, pObj->affected, argument );
}
OEDIT( oedit_showaffect ) {
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ(ch, pObj);

  general_show_affects( ch, pObj->affected, argument );

  return FALSE;
}
OEDIT( oedit_setaffect ) {
  OBJ_INDEX_DATA *pObj;
  EDIT_OBJ(ch, pObj);

  return general_set_affect( ch, pObj->affected, argument );
}



OEDIT( oedit_name )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  name [string]\n\r", ch );
    return FALSE;
  }

  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  pObj->name = str_dup( buf, TRUE );
  //pObj->name = str_dup( argument );

  send_to_char( "Name set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_short )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  short [string]\n\r", ch );
    return FALSE;
  }

  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  pObj->short_descr = str_dup( buf, TRUE );
  //pObj->short_descr = str_dup( argument );
  // Removed by SinaC 2001
  //pObj->short_descr[0] = LOWER( pObj->short_descr[0] );

  send_to_char( "Short description set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_long )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  long [string]\n\r", ch );
    return FALSE;
  }
        
  //pObj->description = str_dup_capitalize( argument );
  char buf[MAX_STRING_LENGTH];
  strcpy(buf,argument);
  buf[0] = UPPER(buf[0]);
  pObj->description = str_dup(buf, TRUE);

  send_to_char( "Long description set.\n\r", ch);
  return TRUE;
}



bool set_value( CHAR_DATA *ch, OBJ_INDEX_DATA *pObj, const char *argument, int value )
{
  if ( argument[0] == '\0' ) {
    set_obj_values( ch, pObj, -1, "" );     /* '\0' changed to "" -- Hugin */
    return FALSE;
  }

  if ( set_obj_values( ch, pObj, value, argument ) )
    return TRUE;

  return FALSE;
}



/*****************************************************************************
 Name:		oedit_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin )
 ****************************************************************************/
bool oedit_values( CHAR_DATA *ch, const char *argument, int value )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( set_value( ch, pObj, argument, value ) )
    return TRUE;

  return FALSE;
}


OEDIT( oedit_value0 )
{
  if ( oedit_values( ch, argument, 0 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value1 )
{
  if ( oedit_values( ch, argument, 1 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value2 )
{
  if ( oedit_values( ch, argument, 2 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value3 )
{
  if ( oedit_values( ch, argument, 3 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_value4 )
{
  if ( oedit_values( ch, argument, 4 ) )
    return TRUE;

  return FALSE;
}



OEDIT( oedit_weight )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  weight [number]\n\r", ch );
    return FALSE;
  }

  pObj->weight = atoi( argument );

  send_to_char( "Weight set.\n\r", ch);
  return TRUE;
}

OEDIT( oedit_cost )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  cost [number]\n\r", ch );
    return FALSE;
  }

  pObj->cost = atoi( argument );

  send_to_char( "Cost set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_create )
{
  OBJ_INDEX_DATA *pObj;
  AREA_DATA *pArea;
  int  value;
  int  iHash;

  value = atoi( argument );
  if ( argument[0] == '\0' || value == 0 ) {
    send_to_char( "Syntax:  oedit create [vnum]\n\r", ch );
    return FALSE;
  }

  pArea = get_vnum_area( value );
  if ( !pArea ) {
    send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "OEdit:  Vnum in an area you cannot build in.\n\r", ch );
    return FALSE;
  }

  if ( get_obj_index( value ) ) {
    send_to_char( "OEdit:  Object vnum already exists.\n\r", ch );
    return FALSE;
  }
        
  pObj			= new_obj_index();
  pObj->vnum			= value;
  pObj->area			= pArea;
        
  if ( value > top_vnum_obj )
    top_vnum_obj = value;

  iHash			= value % MAX_KEY_HASH;
  pObj->next			= obj_index_hash[iHash];
  obj_index_hash[iHash]	= pObj;
  ch->desc->pEdit		= (void *)pObj;

  // Added by SinaC 2001
  pObj->wear_flags = ITEM_TAKE;

  send_to_char( "Object Created.\n\r", ch );
  return TRUE;
}



OEDIT( oedit_ed )
{
  OBJ_INDEX_DATA *pObj;
  EXTRA_DESCR_DATA *ed;
  char command[MAX_INPUT_LENGTH];
  char keyword[MAX_INPUT_LENGTH];

  EDIT_OBJ(ch, pObj);

  /* Modified by SinaC 2001
  argument = one_argument( argument, command );
  one_argument( argument, keyword );

  if ( command[0] == '\0' ) {
    send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
    send_to_char( "         ed delete [keyword]\n\r", ch );
    send_to_char( "         ed edit [keyword]\n\r", ch );
    send_to_char( "         ed format [keyword]\n\r", ch );
    return FALSE;
  }
  */
  // Modified by SinaC 2001
  argument = one_argument( argument, command );
  //one_argument( argument, keyword ); moved down by SinaC 2001

  // Modified by SinaC 2001
  if ( command[0] == '\0' ) { //|| keyword[0] == '\0' ) {
    ed_syntax(ch);
    return FALSE;
  }

  // Added by SinaC 2001
  if ( !str_cmp( command, "change" ) ) {
    argument = one_argument( argument, keyword );
    
    if ( keyword[0] == '\0' || argument[0] == '\0' )	{
      send_to_char( "Syntax:  ed change [old keyword] [new keyword]\n\r", ch );
      return FALSE;
    }
    for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
    }

    if ( !ed ) {
      send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    ed->keyword = str_dup( argument, TRUE );
    return TRUE;
  }

  // Added by SinaC 2001
  strcpy( keyword, argument );
  if ( keyword[0] == '\0' ) {
    ed_syntax(ch);
    return FALSE;
  }

  if ( !str_cmp( command, "add" ) ) {
    if ( keyword[0] == '\0' ) {
      send_to_char( "Syntax:  ed add [keyword]\n\r", ch );
      return FALSE;
    }

    ed                  =   new_extra_descr();
    ed->keyword         =   str_dup( keyword, TRUE );
    ed->next            =   pObj->extra_descr;
    pObj->extra_descr   =   ed;

    string_append( ch, &ed->description );

    return TRUE;
  }

  if ( !str_cmp( command, "edit" ) ) {
    if ( keyword[0] == '\0' ) {
      send_to_char( "Syntax:  ed edit [keyword]\n\r", ch );
      return FALSE;
    }

    for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
    }

    if ( !ed ) {
      send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    string_append( ch, &ed->description );

    return TRUE;
  }

  if ( !str_cmp( command, "delete" ) ) {
    EXTRA_DESCR_DATA *ped = NULL;

    if ( keyword[0] == '\0' ) {
      send_to_char( "Syntax:  ed delete [keyword]\n\r", ch );
      return FALSE;
    }

    for ( ed = pObj->extra_descr; ed; ed = ed->next )	{
      if ( is_name( keyword, ed->keyword ) )
	break;
      ped = ed;
    }

    if ( !ed ) {
      send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    if ( !ped )
      pObj->extra_descr = ed->next;
    else
      ped->next = ed->next;

    send_to_char( "Extra description deleted.\n\r", ch );
    return TRUE;
  }


  if ( !str_cmp( command, "format" ) ) {
    EXTRA_DESCR_DATA *ped = NULL;

    if ( keyword[0] == '\0' )	{
      send_to_char( "Syntax:  ed format [keyword]\n\r", ch );
      return FALSE;
    }

    for ( ed = pObj->extra_descr; ed; ed = ed->next ) {
      if ( is_name( keyword, ed->keyword ) )
	break;
      ped = ed;
    }

    if ( !ed ) {
      send_to_char( "OEdit:  Extra description keyword not found.\n\r", ch );
      return FALSE;
    }

    ed->description = format_string( ed->description );

    send_to_char( "Extra description formatted.\n\r", ch );
    return TRUE;
  }

  oedit_ed( ch, "" );
  return FALSE;
}





/* ROM object functions : */

OEDIT( oedit_extra )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_OBJ(ch, pObj);

    if ( ( value = flag_value( extra_flags, argument ) ) != NO_FLAG ) {
      TOGGLE_BIT(pObj->extra_flags, value);

      send_to_char( "Extra flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax:  extra [flag]\n\r"
		"Type '? extra' for a list of flags.\n\r", ch );
  return FALSE;
}


OEDIT( oedit_wear )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_OBJ(ch, pObj);

    if ( ( value = flag_value( wear_flags, argument ) ) != NO_FLAG ) {
      TOGGLE_BIT(pObj->wear_flags, value);

      send_to_char( "Wear flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax:  wear [flag]\n\r"
		"Type '? wear' for a list of flags.\n\r", ch );
  return FALSE;
}


OEDIT( oedit_type )      /* Moved out of oedit() due to naming conflicts -- Hugin */
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_OBJ(ch, pObj);

    if ( ( value = flag_value( type_flags, argument ) ) != NO_FLAG ) {
      pObj->item_type = value;

      send_to_char( "Type set.\n\r", ch);

      /*
       * Clear the values.
       */
      pObj->value[0] = 0;
      pObj->value[1] = 0;
      pObj->value[2] = 0;
      pObj->value[3] = 0;
      pObj->value[4] = 0;     /* ROM */

      return TRUE;
    }
  }

  send_to_char( "Syntax:  type [flag]\n\r"
		"Type '? type' for a list of flags.\n\r", ch );
  return FALSE;
}

OEDIT( oedit_material )
{
  OBJ_INDEX_DATA *pObj;
  int value;

  EDIT_OBJ(ch, pObj);

  /* Modified by SinaC 2001
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  material [string]\n\r", ch );
    return FALSE;
  }

  pObj->material = str_dup( argument );
  */
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  material [type]\n\r"
		  "Type '? material' for a list of material type.\n\r", ch );
    return FALSE;
  }
  if ( ( value = flag_value( material_flags, argument ) ) == NO_FLAG ) {
    send_to_char( "Invalid material!!\n\r"
		  "Type '? material' for a list of available material type.\n\r", ch );
    return FALSE;
  }

  pObj->material = value;
  
  send_to_char( "Material set.\n\r", ch);
  return TRUE;
}

OEDIT( oedit_level )
{
  OBJ_INDEX_DATA *pObj;

  EDIT_OBJ(ch, pObj);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  level [number]\n\r", ch );
    return FALSE;
  }

  int level = atoi( argument );
  if ( level < 0 || level > 120 ) {
    send_to_char( "Invalid level range: from 0 to 120\n\r", ch );
    return FALSE;
  }

  pObj->level = level;

  send_to_char( "Level set.\n\r", ch);
  return TRUE;
}



OEDIT( oedit_condition )
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( argument[0] != '\0'
       && ( value = atoi (argument ) ) >= 0
       && ( value <= 100 ) ) {
    EDIT_OBJ( ch, pObj );

    pObj->condition = value;
    send_to_char( "Condition set.\n\r", ch );

    return TRUE;
  }

  send_to_char( "Syntax:  condition [number]\n\r"
		"Where number can range from 0 (ruined) to 100 (perfect).\n\r",
		ch );
  return FALSE;
}

// Added by SinaC 2000 for object restriction
// Syntax: addrestriction <type> <value> [not]
// Modified by SinaC 2000
OEDIT( oedit_addrestriction )
{
  OBJ_INDEX_DATA *pObj;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  int type, value;
  int sn;
  RESTR_DATA *restr;

  EDIT_OBJ( ch, pObj );

  // now we check the arguments
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  // Modified by SinaC 2000 for NOT restriction
  argument = one_argument( argument, arg3 );
  if ( arg1[0] == '\0' || arg2[0] == '\0' 
       || ( arg3[0] != '\0' && str_cmp( arg3, "not" ) ) ){
    send_to_char("Syntax: addrestriction <type> <value> [not]\n\r"
		 "Type '? restriction' for a list of available restrictions.\n\r",ch);
    return FALSE;
  }

  sn = ability_lookup( arg1 );
  // ability restriction
  if ( sn > 0 ) {
    type = RESTR_ABILITY;
    value = atoi( arg2 );
    if ( value <= 0 || value > 100 ){
      send_to_char("Skill/Spell/Power/Song percentage must be between 1 and 100.\n\r",ch);
      return FALSE;
    }
  }
  // not ability restriction
  else {
    // what kind of restriction type ?
    type = flag_value( restr_flags, arg1 );
    if ( type == NO_FLAG ){
      send_to_char("Invalid restriction type\n\r"
		   "Valid restriction types are:\n\r",ch);
      show_help( ch, "restriction" );
      return FALSE;
    }
    // what's the value ?
    // restriction needs a numeric value
    if ( restr_table[ type ].bits == NULL ){
      if ( !is_number( arg2 ) ){
	send_to_char("That restriction needs a numeric value\n\r",ch);
	return FALSE;
      }
      // Maybe we could check if the value is in a acceptable range [5..25]
      value = atoi( arg2 );
      // restriction needs something else
    }
    else {
      value = flag_value( restr_table[type].bits, arg2 ); 
      if ( value == NO_FLAG ) {
	send_to_char( "Valid value are:\n\r", ch );
	show_flag_cmds(ch,restr_table[type].bits);
	return FALSE;
      }
    }
  }

  // okay
  restr = new_restriction();
  restr->type = type;
  restr->value = value;
  // Added by SinaC 2000 for NOT restriction
  if ( arg3[0] != '\0' && !str_cmp( arg3, "not" ) )
    restr->not_r = TRUE;
  else
    restr->not_r = FALSE;

  if ( sn > 0 ) {
    //    restr->ability_r = TRUE;
    restr->sn = sn;
  }
  else {
    //    restr->ability_r = FALSE;
    restr->sn = -1;
  }
  
  restr->next = pObj->restriction;
  pObj->restriction = restr;

  send_to_char("Restriction added.\n\r",ch);

  return TRUE;
}

OEDIT( oedit_delrestriction )
{
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_INPUT_LENGTH];
  int value, cnt = 0;
  RESTR_DATA *restr, *restr_next;

  EDIT_OBJ(ch, pObj);

  // right arg ?
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' || !is_number(arg) ){
    send_to_char("Syntax: delrestriction <#xrestriction>\n\r",ch);
    return FALSE;
  }

  value = atoi( arg );
  
  if ( value < 0 ){
    send_to_char( "Only non-negative number allowed.\n\r", ch );
    return FALSE;
  }

  if ( !( restr = pObj->restriction ) ){
    send_to_char( "OEdit:  Non-existant restriction.\n\r", ch );
    return FALSE;
  }

  if( value == 0 ){	// First case: Remove first affect
    restr = pObj->restriction;
    pObj->restriction = restr->next;
  }
  else{
    // Affect to remove is not the first
    while ( ( restr_next = restr->next ) && ( ++cnt < value ) )
      restr = restr_next;

    if( restr_next ){		// See if it's the next affect
      restr->next = restr_next->next;
    }
    else{                                 // Doesn't exist
      send_to_char( "No such restriction.\n\r", ch );
      return FALSE;
    }
  }  

  send_to_char( "Restriction removed.\n\r", ch);
  return TRUE;
}

// Added a skill/spell upgrade by SinaC 2000
// Syntax: addskillupgrade <skill/spell name> <value>
OEDIT( oedit_addskillupgrade )
{
  OBJ_INDEX_DATA *pObj;
  ABILITY_UPGRADE *upgr;
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int sn, value;

  EDIT_OBJ( ch, pObj );

  // now we check the arguments
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' ){
    send_to_char("Missing argument(s)\n\r", ch );
    send_to_char("Syntax: addskillupgrade <skill/spell name> <value>\n\r", ch );
    return FALSE;
  }
  
  sn = ability_lookup( arg1 );
  if ( sn < 0 ){
    send_to_char("Invalid skill/spell name.\n\r", ch );
    return FALSE;
  }

  value = atoi( arg2 );
  if ( value < -99  || value > 99 || value == 0){
    send_to_char("Invalid percentage value, must be between -99 and 99 and different from 0\n\r", ch );
    return FALSE;
  }
  
  // okay
  upgr = new_ability_upgrade();
  upgr->sn = sn;
  upgr->value = value;
  
  upgr->next = pObj->upgrade;
  pObj->upgrade = upgr;

  send_to_char("Ability upgrade added.\n\r",ch);

  return TRUE;  
}

OEDIT( oedit_delskillupgrade )
{
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_INPUT_LENGTH];
  int value, cnt = 0;
  ABILITY_UPGRADE *upgr, *upgr_next;

  EDIT_OBJ(ch, pObj);

  // right arg ?
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' || !is_number(arg) ){
    send_to_char("Syntax: delskillupgrade <#xskillupgrade>\n\r",ch);
    return FALSE;
  }

  value = atoi( arg );
  
  if ( value < 0 ){
    send_to_char( "Only non-negative number allowed.\n\r", ch );
    return FALSE;
  }

  if ( !( upgr = pObj->upgrade ) ){
    send_to_char( "OEdit:  Non-existant upgrade.\n\r", ch );
    return FALSE;
  }

  if( value == 0 ){	// First case: Remove first affect
    upgr = pObj->upgrade;
    pObj->upgrade = upgr->next;
  }
  else{
    // Affect to remove is not the first
    while ( ( upgr_next = upgr->next ) && ( ++cnt < value ) )
      upgr = upgr_next;

    if( upgr_next ){		// See if it's the next affect
      upgr->next = upgr_next->next;
    }
    else{                                 // Doesn't exist
      send_to_char( "No such skill/spell/power upgrade.\n\r", ch );
      return FALSE;
    }
  }  

  send_to_char( "Skill/Spell/Power upgrade removed.\n\r", ch);
  return TRUE;
}

// Added by SinaC 2003, size is not anymore a restriction but an obj stat
OEDIT( oedit_size )
{
  OBJ_INDEX_DATA *pObj;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_OBJ(ch, pObj);

    if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG ) {
      pObj->size = value;

      send_to_char( "Size set.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax:  size [flag]\n\r"
		"Type '? size' for a list of flags.\n\r", ch );
  return FALSE;
}



/*
 * Mobile Editor Functions.
 */
MEDIT( medit_show )
{
  MOB_INDEX_DATA *pMob;
  char buf[MAX_STRING_LENGTH];

  EDIT_MOB(ch, pMob);

  sprintf( buf, "Name:        [%s]\n\rArea:        [%5d] %s\n\r",
	   pMob->player_name,
	   !pMob->area ? -1        : pMob->area->vnum,
	   !pMob->area ? "No Area" : pMob->area->name );
  send_to_char( buf, ch );

  sprintf( buf, "Act:         [%s]\n\r",
	   flag_string( act_flags, pMob->act ) );
  send_to_char( buf, ch );

  sprintf( buf, "Vnum:        [%5d]\n\rSex:         [%s]\n\r",
	   pMob->vnum,
	   pMob->sex == SEX_MALE    ? "male"   :
	   pMob->sex == SEX_FEMALE  ? "female" : 
	   pMob->sex == 3           ? "random" : "neutral" );  /* ROM magic number */
  send_to_char( buf, ch );

  sprintf( buf, "Group:       [%5d]\n\r", pMob->group );
  send_to_char( buf, ch );

  // SinaC 2003
  send_to_charf( ch, "Faction      [%s]\n\r", faction_table[pMob->faction].name );

  sprintf( buf, "Race:        [%s]\n\r",                   /* ROM OLC */
	   race_table[pMob->race].name );
  send_to_char( buf, ch );

  // Added by SinaC 2000 for mobile classes
  sprintf( buf, "Classes:     [%s]\n\r",
	   flag_string( classes_flags, pMob->classes ) );
  send_to_char( buf, ch );

  sprintf( buf,
	   "Level:       [%2d]\n\r"
	   "Etho:        [%s]\n\r"
	   "Align:       [%4d]\n\r",
  // Modified by SinaC 2000 for alignment/etho
	   pMob->level,       
	   etho_name(pMob->align.etho),
	   pMob->align.alignment );
  send_to_char( buf, ch );

  /* ROM values: */

  sprintf( buf, "Hitroll:     [%d]\n\r",
	   pMob->hitroll );
  send_to_char( buf, ch );

  sprintf( buf, "Dam Type:    [%s]\n\r",
	   attack_table[pMob->dam_type].name );
  send_to_char( buf, ch );

  sprintf( buf, "Hit dice:    [%2dd%-3d+%4d]\n\r",
	   pMob->hit[DICE_NUMBER],
	   pMob->hit[DICE_TYPE],
	   pMob->hit[DICE_BONUS] );
  send_to_char( buf, ch );

  sprintf( buf, "Damage dice: [%2dd%-3d+%4d]\n\r",
	   pMob->damage[DICE_NUMBER],
	   pMob->damage[DICE_TYPE],
	   pMob->damage[DICE_BONUS] );
  send_to_char( buf, ch );

  sprintf( buf, "Mana dice:   [%2dd%-3d+%4d]\n\r",
	   pMob->mana[DICE_NUMBER],
	   pMob->mana[DICE_TYPE],
	   pMob->mana[DICE_BONUS] );
  send_to_char( buf, ch );

  // Added by SinaC 2001 for mental user
  sprintf( buf, "Psp dice:    [%2dd%-3d+%4d]\n\r",
	   pMob->psp[DICE_NUMBER],
	   pMob->psp[DICE_TYPE],
	   pMob->psp[DICE_BONUS] );
  send_to_char( buf, ch );


  /* ROM values end */

  sprintf( buf, "Affected by: [%s]\n\r",
	   flag_string( affect_flags, pMob->affected_by ) );
  send_to_char( buf, ch );
  // Added by SinaC 2001
  sprintf( buf, "Also affected by: [%s]\n\r",
	   flag_string( affect2_flags, pMob->affected2_by ) );
  send_to_char( buf, ch );


  /* ROM values: */

  sprintf( buf, "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n\r",
	   pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
	   pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC] );
  send_to_char( buf, ch );

  sprintf( buf, "Form:        [%s]\n\r",
	   flag_string( form_flags, pMob->form ) );
  send_to_char( buf, ch );

  sprintf( buf, "Parts:       [%s]\n\r",
	   flag_string( part_flags, pMob->parts ) );
  send_to_char( buf, ch );

  sprintf( buf, "Imm:         [%s]\n\r",
	   flag_string( irv_flags, pMob->imm_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Res:         [%s]\n\r",
	   flag_string( irv_flags, pMob->res_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Vuln:        [%s]\n\r",
	   flag_string( irv_flags, pMob->vuln_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Off:         [%s]\n\r",
	   flag_string( off_flags,  pMob->off_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Size:        [%s]\n\r",
	   flag_string( size_flags, pMob->size ) );
  send_to_char( buf, ch );

  sprintf( buf, "Material:    [%s]\n\r",
	   pMob->material );
  send_to_char( buf, ch );

  sprintf( buf, "Start pos.   [%s]\n\r",
	   flag_string( position_flags, pMob->start_pos ) );
  send_to_char( buf, ch );

  sprintf( buf, "Default pos  [%s]\n\r",
	   flag_string( position_flags, pMob->default_pos ) );
  send_to_char( buf, ch );

  sprintf( buf, "Wealth:      [%5ld]\n\r",
	   pMob->wealth );
  send_to_char( buf, ch );

  /* ROM values end */

  // Added by SinaC 2001 for disease         Removed by SinaC 2003
  //sprintf( buf, "Disease:     [%s]\n\r",
  //	   flag_string( disease_flags, pMob->disease ) );
  //send_to_char( buf, ch );

  if ( pMob->spec_fun ) {
    sprintf( buf, "Spec fun:    [%s]\n\r",  spec_name( pMob->spec_fun ) );
    send_to_char( buf, ch );
  }
    
  if (pMob->program)
    send_to_charf(ch,"Program :    [%s]\n\r", pMob->program->name );

  sprintf( buf, "Short descr: %s\n\rLong descr:\n\r%s",
	   pMob->short_descr,
	   pMob->long_descr );
  send_to_char( buf, ch );

  sprintf( buf, "Description:\n\r%s", pMob->description );
  send_to_char( buf, ch );

  if ( pMob->pShop ) {
    SHOP_DATA *pShop;
    int iTrade;

    pShop = pMob->pShop;

    sprintf( buf,
	     "Shop data for [%5d]:\n\r"
	     "  Markup for purchaser: %d%%\n\r"
	     "  Markdown for seller:  %d%%\n\r",
	     pShop->keeper, pShop->profit_buy, pShop->profit_sell );
    send_to_char( buf, ch );
    sprintf( buf, "  Hours: %d to %d.\n\r",
	     pShop->open_hour, pShop->close_hour );
    send_to_char( buf, ch );

    for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
      if ( pShop->buy_type[iTrade] != 0 ) {
	if ( iTrade == 0 ) {
	  send_to_char( "  Number Trades Type\n\r", ch );
	  send_to_char( "  ------ -----------\n\r", ch );
	}
	sprintf( buf, "  [%4d] %s\n\r", iTrade,
		 flag_string( type_flags, pShop->buy_type[iTrade] ) );
	send_to_char( buf, ch );
      }
    }
  }

  general_show_affects( ch, pMob->affected, "auto" );

  return FALSE;
}



MEDIT( medit_create )
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int  value;
  int  iHash;

  value = atoi( argument );
  if ( argument[0] == '\0' || value == 0 ) {
    send_to_char( "Syntax:  medit create [vnum]\n\r", ch );
    return FALSE;
  }

  pArea = get_vnum_area( value );

  if ( !pArea ) {
    send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "MEdit:  Vnum in an area you cannot build in.\n\r", ch );
    return FALSE;
  }

  if ( get_mob_index( value ) ) {
    send_to_char( "MEdit:  Mobile vnum already exists.\n\r", ch );
    return FALSE;
  }

  pMob			= new_mob_index();
  pMob->vnum			= value;
  pMob->area			= pArea;
        
  if ( value > top_vnum_mob )
    top_vnum_mob = value;        

  pMob->act			= ACT_IS_NPC;
  iHash			= value % MAX_KEY_HASH;
  pMob->next			= mob_index_hash[iHash];
  mob_index_hash[iHash]	= pMob;
  ch->desc->pEdit		= (void *)pMob;

  send_to_char( "Mobile Created.\n\r", ch );
  return TRUE;
}


MEDIT( medit_faction ) { // SinaC 2003
  MOB_INDEX_DATA *pMob;
  EDIT_MOB( ch, pMob );
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  faction [faction name]\n\r"
		  "Type 'dlist faction' for a list of available factions.\n\r");
    return FALSE;
  }
  if ( !str_cmp( argument, "none" ) ) {
    pMob->faction = get_neutral_faction();
    send_to_charf(ch,"Faction removed.\n\r");
    return TRUE;
  }
  int id = get_faction_id( argument );
  if ( id <= -1 ) {
    send_to_charf(ch,"MEdit: No such faction.\n\r"
		  "Type 'dlist faction' for a list of available factions.\n\r");
    return FALSE;
  }
  pMob->faction = id;
  send_to_charf(ch,"Faction set.\n\r");
  return TRUE;
}

MEDIT( medit_spec )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  spec [special function]\n\r", ch );
    return FALSE;
  }


  if ( !str_cmp( argument, "none" ) ) {
    pMob->spec_fun = NULL;

    send_to_char( "Spec removed.\n\r", ch);
    return TRUE;
  }

  if ( spec_lookup( argument ) ) {
    pMob->spec_fun = spec_lookup( argument );
    send_to_char( "Spec set.\n\r", ch);
    return TRUE;
  }

  send_to_char( "MEdit: No such special function.\n\r", ch );
  send_to_char( "Use  program list special   to check if there is a mobprogram\n\r"
		" replacing this special function.\n\r", ch );
  return FALSE;
}

MEDIT( medit_program ) {
  MOB_INDEX_DATA *pMob;
  CLASS_DATA *cla;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  program [program name/none]\n\r", ch );
    return FALSE;
  }


  if ( !str_cmp( argument, "none" ) ) {
    pMob->program = NULL;

    send_to_char( "program removed.\n\r", ch);
    return TRUE;
  }

  if ( ( cla = silent_hash_get_prog( argument ) ) ) {
    if ( get_root_class(cla) == default_mob_class ) {
      if ( cla->isAbstract ) {
	send_to_char("This is an abstract program, it must be derivated before being used.\n\r",ch);
	return FALSE;
      }
      pMob->program = cla;
      send_to_char( "Program set.\n\r", ch);
      return TRUE;
    } 
    else {
      send_to_char("Not a mob program!!\n\r", ch);
      return FALSE;
    }
  }

  send_to_char( "MEdit: No such program.\n\r", ch );
  return FALSE;
}

MEDIT( medit_damtype )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  damtype [damage message]\n\r", ch );
    send_to_char( "Para ver una lista de tipos de mensajes, pon '? weapon'.\n\r", ch );
    return FALSE;
  }

  pMob->dam_type = attack_lookup(argument);
  send_to_char( "Damage type set.\n\r", ch);
  return TRUE;
}


MEDIT( medit_align )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  alignment [number]\n\r", ch );
    return FALSE;
  }
  // Modified by SinaC 2000 for alignment/etho
  pMob->align.alignment = atoi( argument );

  send_to_char( "Alignment set.\n\r", ch);
  return TRUE;
}

// Added by SinaC 2000 for etho
MEDIT( medit_etho )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ){
    send_to_char( "Syntax:  etho chaotic|neutral|good\n\r", ch );
    return FALSE;
  }
  
  if ( !str_cmp( argument, "chaotic" ) )
    pMob->align.etho = -1;
  else if ( !str_cmp( argument, "neutral" ) )
    pMob->align.etho = 0;
  else if ( !str_cmp( argument, "lawful" ) )
    pMob->align.etho = 1;
  else {
    send_to_char( "Syntax:  etho chaotic|neutral|good\n\r", ch );
    return FALSE;
  }
  
  send_to_char( "Etho set.\n\r", ch);
  return TRUE;
}


MEDIT( medit_level )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  level [number]\n\r", ch );
    return FALSE;
  }

  int level = atoi( argument );
  if ( level < 0 || level > 120 ) {
    send_to_char( "Invalid level range: from 0 to 120\n\r", ch );
  }

  pMob->level = level;

  // Added by SinaC 2001
  if ( pMob->level < pMob->area->low_range )
    pMob->area->low_range = pMob->level;
  if ( pMob->level > pMob->area->high_range )
    pMob->area->high_range = pMob->level;

  send_to_char( "Level set.\n\r", ch);
  return TRUE;
}



MEDIT( medit_desc )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    string_append( ch, &pMob->description );
    return TRUE;
  }

  send_to_char( "Syntax:  desc    - line edit\n\r", ch );
  return FALSE;
}




MEDIT( medit_long )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  long [string]\n\r", ch );
    return FALSE;
  }

  char buf[MAX_INPUT_LENGTH];
  sprintf(buf,"%s\n\r",argument);
  pMob->long_descr = str_dup_capitalize( buf, TRUE );
  //char buf[MAX_INPUT_LENGTH];
  //sprintf(buf,"%s\n\r",argument);
  //buf[0] = UPPER(buf[0]);
  //pMob->long_descr = str_dup( buf );

  send_to_char( "Long description set.\n\r", ch);
  return TRUE;
}



MEDIT( medit_short )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  short [string]\n\r", ch );
    return FALSE;
  }

  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  pMob->short_descr = str_dup( buf, TRUE );
  //pMob->short_descr = str_dup( argument );

  send_to_char( "Short description set.\n\r", ch);
  return TRUE;
}



MEDIT( medit_name )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  name [string]\n\r", ch );
    return FALSE;
  }

  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  pMob->player_name = str_dup( buf, TRUE );
  //pMob->player_name = str_dup( argument );

  send_to_char( "Name set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_shop )
{
  MOB_INDEX_DATA *pMob;
  char command[MAX_INPUT_LENGTH];
  char arg1[MAX_INPUT_LENGTH];

  argument = one_argument( argument, command );
  argument = one_argument( argument, arg1 );

  EDIT_MOB(ch, pMob);

  if ( command[0] == '\0' ) {
    send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
    send_to_char( "         shop profit [#xbuying%] [#xselling%]\n\r", ch );
    send_to_char( "         shop type [#x0-4] [item type]\n\r", ch );
    send_to_char( "         shop assign\n\r", ch );
    send_to_char( "         shop remove\n\r", ch );
    return FALSE;
  }


  if ( !str_cmp( command, "hours" ) ) {
    if ( arg1[0] == '\0' || !is_number( arg1 )
	 || argument[0] == '\0' || !is_number( argument ) ) {
      send_to_char( "Syntax:  shop hours [#xopening] [#xclosing]\n\r", ch );
      return FALSE;
    }

    if ( !pMob->pShop ) {
      send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
      return FALSE;
    }

    pMob->pShop->open_hour = atoi( arg1 );
    pMob->pShop->close_hour = atoi( argument );

    send_to_char( "Shop hours set.\n\r", ch);
    return TRUE;
  }


  if ( !str_cmp( command, "profit" ) ) {
    if ( arg1[0] == '\0' || !is_number( arg1 )
	 || argument[0] == '\0' || !is_number( argument ) ) {
      send_to_char( "Syntax:  shop profit [#xbuying%] [#xselling%]\n\r", ch );
      return FALSE;
    }

    if ( !pMob->pShop ) {
      send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
      return FALSE;
    }

    pMob->pShop->profit_buy     = atoi( arg1 );
    pMob->pShop->profit_sell    = atoi( argument );

    send_to_char( "Shop profit set.\n\r", ch);
    return TRUE;
  }


  if ( !str_cmp( command, "type" ) ) {
    char buf[MAX_INPUT_LENGTH];
    int value;

    if ( arg1[0] == '\0' || !is_number( arg1 )
	 || argument[0] == '\0' ) {
      send_to_char( "Syntax:  shop type [#x0-4] [item type]\n\r", ch );
      return FALSE;
    }

    if ( atoi( arg1 ) >= MAX_TRADE ) {
      sprintf( buf, "MEdit:  May sell %d items max.\n\r", MAX_TRADE );
      send_to_char( buf, ch );
      return FALSE;
    }

    if ( !pMob->pShop ) {
      send_to_char( "MEdit:  Debes crear un shop primero (shop assign).\n\r", ch );
      return FALSE;
    }

    if ( ( value = flag_value( type_flags, argument ) ) == NO_FLAG ) {
      send_to_char( "MEdit:  That type of item is not known.\n\r", ch );
      return FALSE;
    }

    pMob->pShop->buy_type[atoi( arg1 )] = value;

    send_to_char( "Shop type set.\n\r", ch);
    return TRUE;
  }

  /* shop assign && shop delete by Phoenix */

  if ( !str_prefix(command, "assign") ) {
    if ( pMob->pShop ) {
      send_to_char("Mob already has a shop assigned to it.\n\r", ch);
      return FALSE;
    }

    pMob->pShop		= new_shop();
    if ( !shop_first )
      shop_first	= pMob->pShop;
    if ( shop_last )
      shop_last->next	= pMob->pShop;
    shop_last		= pMob->pShop;

    pMob->pShop->keeper	= pMob->vnum;

    send_to_char("New shop assigned to mobile.\n\r", ch);
    return TRUE;
  }

  if ( !str_prefix(command, "remove") ) {
    SHOP_DATA *pShop;

    pShop		= pMob->pShop;
    pMob->pShop	= NULL;

    if ( pShop == shop_first ) {
      if ( !pShop->next ) {
	shop_first = NULL;
	shop_last = NULL;
      }
      else
	shop_first = pShop->next;
    }
    else {
      SHOP_DATA *ipShop;

      for ( ipShop = shop_first; ipShop; ipShop = ipShop->next ) {
	if ( ipShop->next == pShop ) {
	  if ( !pShop->next ) {
	    shop_last = ipShop;
	    shop_last->next = NULL;
	  }
	  else
	    ipShop->next = pShop->next;
	}
      }
    }

    send_to_char("Mobile is no longer a shopkeeper.\n\r", ch);
    return TRUE;
  }

  medit_shop( ch, "" );
  return FALSE;
}


/* ROM medit functions: */


MEDIT( medit_sex )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( sex_flags, argument ) ) != NO_FLAG ) {
      pMob->sex = value;

      send_to_char( "Sex set.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: sex [sex]\n\r"
		"Type '? sex' for a list of flags.\n\r", ch );
  return FALSE;
}


MEDIT( medit_act )          /* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( act_flags, argument ) ) != NO_FLAG ) {
      pMob->act ^= value;
      SET_BIT( pMob->act, ACT_IS_NPC );

      send_to_char( "Act flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: act [flag]\n\r"
		"Type '? act' for a list of flags.\n\r", ch );
  return FALSE;
}


MEDIT( medit_affect )      /* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  long value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( affect_flags, argument ) ) != NO_FLAG ) {

      pMob->affected_by ^= value;

      send_to_char( "Affect flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: affect [flag]\n\r"
		"Type '? affect' for a list of flags.\n\r", ch );
  return FALSE;
}

// Added by SinaC 2001
MEDIT( medit_affect2 )      /* Moved out of medit() due to naming conflicts -- Hugin */
{
  MOB_INDEX_DATA *pMob;
  long value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( affect2_flags, argument ) ) != NO_FLAG ) {

      pMob->affected2_by ^= value;

      send_to_char( "Affect second part flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: affect2 [flag]\n\r"
		"Type '? affect2' for a list of flags.\n\r", ch );
  return FALSE;
}

// Added by SinaC 2001 for disease
/*      Removed by SinaC 2003
MEDIT( medit_disease )
{
  MOB_INDEX_DATA *pMob;
  long value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( disease_flags, argument ) ) != NO_FLAG ) {

      pMob->disease ^= value;

      send_to_char( "Disease toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: disease [flag]\n\r"
		"Type '? disease' for a list of flags.\n\r", ch );
  return FALSE;
}
*/

MEDIT( medit_ac )
{
  MOB_INDEX_DATA *pMob;
  char arg[MAX_INPUT_LENGTH];
  int pierce, bash, slash, exotic;

  do {  /* So that I can use break and send the syntax in one place */
    if ( argument[0] == '\0' )  break;

    EDIT_MOB(ch, pMob);
    argument = one_argument( argument, arg );

    if ( !is_number( arg ) )  break;
    pierce = atoi( arg );
    argument = one_argument( argument, arg );

    if ( arg[0] != '\0' ) {
      if ( !is_number( arg ) )  break;
      bash = atoi( arg );
      argument = one_argument( argument, arg );
    }
    else
      bash = pMob->ac[AC_BASH];

    if ( arg[0] != '\0' ) {
      if ( !is_number( arg ) )  break;
      slash = atoi( arg );
      argument = one_argument( argument, arg );
    }
    else
      slash = pMob->ac[AC_SLASH];

    if ( arg[0] != '\0' )
      {
	if ( !is_number( arg ) )  break;
	exotic = atoi( arg );
      }
    else
      exotic = pMob->ac[AC_EXOTIC];

    pMob->ac[AC_PIERCE] = pierce;
    pMob->ac[AC_BASH]   = bash;
    pMob->ac[AC_SLASH]  = slash;
    pMob->ac[AC_EXOTIC] = exotic;
	
    send_to_char( "Ac set.\n\r", ch );
    return TRUE;
  } while ( FALSE );    /* Just do it once.. */

  send_to_char( "Syntax:  armor [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n\r"
		"help MOB_AC  gives a list of reasonable ac-values.\n\r", ch );
  return FALSE;
}

// Added by SinaC 2000 for mobile class
MEDIT( medit_classes )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( classes_flags, argument ) ) != NO_FLAG ) {
      pMob->classes ^= value;
      send_to_char( "Class toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: classes [flags]\n\r"
		"Type '? classes' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_form )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( form_flags, argument ) ) != NO_FLAG ) {
      pMob->form ^= value;
      send_to_char( "Form toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: form [flags]\n\r"
		"Type '? form' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_part )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( part_flags, argument ) ) != NO_FLAG ) {
      pMob->parts ^= value;
      send_to_char( "Parts toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: part [flags]\n\r"
		"Type '? part' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_imm )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( irv_flags, argument ) ) != NO_FLAG ) {
      pMob->imm_flags ^= value;
      send_to_char( "Immunity toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: imm [flags]\n\r"
		"Type '? imm' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_res )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( irv_flags, argument ) ) != NO_FLAG ) {
      pMob->res_flags ^= value;
      send_to_char( "Resistance toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: res [flags]\n\r"
		"Type '? res' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_vuln )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( irv_flags, argument ) ) != NO_FLAG ) {
      pMob->vuln_flags ^= value;
      send_to_char( "Vulnerability toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: vuln [flags]\n\r"
		"Type '? vuln' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_material )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  material [string]\n\r", ch );
    return FALSE;
  }

  pMob->material = str_dup( argument, TRUE );

  send_to_char( "Material set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_off )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( off_flags, argument ) ) != NO_FLAG ) {
      pMob->off_flags ^= value;
      send_to_char( "Offensive behaviour toggled.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: off [flags]\n\r"
		"Type '? off' for a list of flags.\n\r", ch );
  return FALSE;
}

MEDIT( medit_size )
{
  MOB_INDEX_DATA *pMob;
  int value;

  if ( argument[0] != '\0' ) {
    EDIT_MOB( ch, pMob );

    if ( ( value = flag_value( size_flags, argument ) ) != NO_FLAG ) {
      /* Removed by SinaC 2000 cos' of general restriction
	 // Added by SinaC 2000
	 if ( value == SIZE_NOSIZE )
	 {
	 send_to_char("This size can only be used for objects!\n\r",ch);
	 return FALSE;
	 }
      */
      pMob->size = value;
      send_to_char( "Size set.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax: size [size]\n\r"
		"Type '? size' for a list of sizes.\n\r", ch );
  return FALSE;
}

bool extract_dice( const char *argument, char *&num, char *&type, char *&bonus ) {
  char buf[MAX_INPUT_LENGTH];
  char *cp;
  strcpy(buf,argument);

  num = cp = buf;

  while ( isdigit( *cp ) ) ++cp;
  while ( *cp != '\0' && !isdigit( *cp ) )  *(cp++) = '\0';

  type = cp;

  while ( isdigit( *cp ) ) ++cp;
  while ( *cp != '\0' && !isdigit( *cp ) ) *(cp++) = '\0';

  bonus = cp;

  while ( isdigit( *cp ) ) ++cp;
  if ( *cp != '\0' ) *cp = '\0';

  if ( ( !is_number( num   ) || atoi( num   ) < 1 )
       ||   ( !is_number( type  ) || atoi( type  ) < 1 ) 
       ||   ( !is_number( bonus ) || atoi( bonus ) < 0 ) ) {
    return FALSE;
  } 
  return TRUE;
}

MEDIT( medit_hitdice )
{
  static char syntax[] = "Syntax:  hitdice <number> d <type> + <bonus>\n\r";
  char *num, *dice, *bonus;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' ) {
    send_to_char( syntax, ch );
    return FALSE;
  }

  if (!extract_dice( argument, num, dice, bonus )) {
    send_to_char( syntax, ch );
    return FALSE;
  }
    
  pMob->hit[DICE_NUMBER] = atoi( num   );
  pMob->hit[DICE_TYPE]   = atoi( dice  );
  pMob->hit[DICE_BONUS]  = atoi( bonus );
  
  send_to_char( "Hitdice set.\n\r", ch );
  return TRUE;
}

MEDIT( medit_manadice )
{
  static char syntax[] = "Syntax:  manadice <number> d <type> + <bonus>\n\r";
  char *num, *bonus, *dice;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' ) {
    send_to_char( syntax, ch );
    return FALSE;
  }

  if (!extract_dice( argument, num, dice, bonus )) {
    send_to_char( syntax, ch );
    return FALSE;
  }

  pMob->mana[DICE_NUMBER] = atoi( num   );
  pMob->mana[DICE_TYPE]   = atoi( dice  );
  pMob->mana[DICE_BONUS]  = atoi( bonus );

  send_to_char( "Manadice set.\n\r", ch );
  return TRUE;
}

// Added by SinaC 2001 for mental user
MEDIT( medit_pspdice )
{
  static char syntax[] = "Syntax:  pspdice <number> d <type> + <bonus>\n\r";
  char *num, *bonus, *dice;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' ) {
    send_to_char( syntax, ch );
    return FALSE;
  }

  if (!extract_dice( argument, num, dice, bonus )) {
    send_to_char( syntax, ch );
    return FALSE;
  }

  pMob->psp[DICE_NUMBER] = atoi( num   );
  pMob->psp[DICE_TYPE]   = atoi( dice  );
  pMob->psp[DICE_BONUS]  = atoi( bonus );

  send_to_char( "Pspdice set.\n\r", ch );
  return TRUE;
}


MEDIT( medit_damdice )
{
  static char syntax[] = "Syntax:  damdice <number> d <type> + <bonus>\n\r";
  char *num, *bonus, *dice;
  MOB_INDEX_DATA *pMob;

  EDIT_MOB( ch, pMob );

  if ( argument[0] == '\0' ) {
    send_to_char( syntax, ch );
    return FALSE;
  }

  extract_dice( argument, num, dice, bonus );

  pMob->damage[DICE_NUMBER] = atoi( num   );
  pMob->damage[DICE_TYPE]   = atoi( dice  );
  pMob->damage[DICE_BONUS]  = atoi( bonus );

  send_to_char( "Damdice set.\n\r", ch );
  return TRUE;
}


MEDIT( medit_race )
{
  MOB_INDEX_DATA *pMob;
  int race;

  if ( argument[0] != '\0'
       // Modified by SinaC 2000
       //&& ( race = race_lookup( argument ) ) != 0 )
       && ( race = race_lookup( argument, TRUE ) ) >= 0 ) {
    EDIT_MOB( ch, pMob );

    pMob->race = race;

    //pMob->act	      |= race_table[race].act | ACT_IS_NPC;
    //pMob->affected_by |= race_table[race].aff;
    // Added by SinaC 2001
    //pMob->affected2_by |= race_table[race].aff2;
    //pMob->off_flags   |= race_table[race].off;
    //pMob->imm_flags   |= race_table[race].imm;
    //pMob->res_flags   |= race_table[race].res;
    //pMob->vuln_flags  |= race_table[race].vuln;
    //pMob->form        |= race_table[race].form;
    //pMob->parts       |= race_table[race].parts;

    pMob->act	      = race_table[race].act | ACT_IS_NPC;
    pMob->affected_by = race_table[race].aff;
    // Added by SinaC 2001
    pMob->affected2_by = race_table[race].aff2;
    pMob->off_flags   = race_table[race].off;
    pMob->imm_flags   = race_table[race].imm;
    pMob->res_flags   = race_table[race].res;
    pMob->vuln_flags  = race_table[race].vuln;
    pMob->form        = race_table[race].form;
    pMob->parts       = race_table[race].parts;
    pMob->size        = race_table[race].size;

    send_to_char( "Race set.\n\r", ch );
    return TRUE;
  }

  if ( argument[0] == '?' ) {
    char buf[MAX_STRING_LENGTH];

    send_to_char( "Available races are:", ch );

    // Modified by SinaC 2000
    //for ( race = 0; race_table[race].name != NULL; race++ )
    for ( race = 0; race < MAX_RACE; race++ ) {
      if ( ( race % 3 ) == 0 )
	send_to_char( "\n\r", ch );
      sprintf( buf, " %-15s", race_table[race].name );
      send_to_char( buf, ch );
    }

    send_to_char( "\n\r", ch );
    return FALSE;
  }

  send_to_char( "Syntax:  race [race]\n\r"
		"Type 'race ?' for a list of races.\n\r", ch );
  return FALSE;
}

MEDIT( medit_modify_race )
{
  MOB_INDEX_DATA *pMob;
  int race;

  if ( argument[0] != '\0'
       // Modified by SinaC 2000
       //&& ( race = race_lookup( argument ) ) != 0 )
       && ( race = race_lookup( argument, TRUE ) ) >= 0 ) {
    EDIT_MOB( ch, pMob );

    pMob->race = race;

    pMob->act	      |= race_table[race].act | ACT_IS_NPC;
    pMob->affected_by |= race_table[race].aff;
    // Added by SinaC 2001
    pMob->affected2_by |= race_table[race].aff2;
    pMob->off_flags   |= race_table[race].off;
    pMob->imm_flags   |= race_table[race].imm;
    pMob->res_flags   |= race_table[race].res;
    pMob->vuln_flags  |= race_table[race].vuln;
    pMob->form        = race_table[race].form; // form and flags are erased
    pMob->parts       = race_table[race].parts;
    pMob->size        = race_table[race].size;

    send_to_char( "Race set.\n\r", ch );
    return TRUE;
  }

  if ( argument[0] == '?' ) {
    char buf[MAX_STRING_LENGTH];

    send_to_char( "Available races are:", ch );

    // Modified by SinaC 2000
    //for ( race = 0; race_table[race].name != NULL; race++ )
    for ( race = 0; race < MAX_RACE; race++ ) {
      if ( ( race % 3 ) == 0 )
	send_to_char( "\n\r", ch );
      sprintf( buf, " %-15s", race_table[race].name );
      send_to_char( buf, ch );
    }

    send_to_char( "\n\r", ch );
    return FALSE;
  }

  send_to_char( "Syntax:  race [race]\n\r"
		"Type 'race ?' for a list of races.\n\r", ch );
  return FALSE;
}


MEDIT( medit_position )
{
  MOB_INDEX_DATA *pMob;
  char arg[MAX_INPUT_LENGTH];
  int value;

  argument = one_argument( argument, arg );

  switch ( arg[0] ) {
  default:
    break;

  case 'S':
  case 's':
    if ( str_prefix( arg, "start" ) )
      break;

    if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
      break;

    EDIT_MOB( ch, pMob );

    pMob->start_pos = value;
    send_to_char( "Start position set.\n\r", ch );
    return TRUE;

  case 'D':
  case 'd':
    if ( str_prefix( arg, "default" ) )
      break;

    if ( ( value = flag_value( position_flags, argument ) ) == NO_FLAG )
      break;

    EDIT_MOB( ch, pMob );

    pMob->default_pos = value;
    send_to_char( "Default position set.\n\r", ch );
    return TRUE;
  }

  send_to_char( "Syntax:  position [start/default] [position]\n\r"
		"Type '? position' for a list of positions.\n\r", ch );
  return FALSE;
}


MEDIT( medit_gold )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  wealth [number]\n\r", ch );
    return FALSE;
  }

  pMob->wealth = atoi( argument );

  send_to_char( "Wealth set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_hitroll )
{
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  hitroll [number]\n\r", ch );
    return FALSE;
  }

  pMob->hitroll = atoi( argument );

  send_to_char( "Hitroll set.\n\r", ch);
  return TRUE;
}

MEDIT( medit_addaffect ) {
  MOB_INDEX_DATA *pMob;
  EDIT_MOB(ch, pMob);

  return general_add_affect( ch, pMob->affected, CHAR_ENTITY, NULL, pMob->level, argument );
}
MEDIT( medit_delaffect ) {
  MOB_INDEX_DATA *pMob;
  EDIT_MOB(ch, pMob);

  return general_del_affect( ch, pMob->affected, argument );
}
MEDIT( medit_showaffect ) {
  MOB_INDEX_DATA *pMob;
  EDIT_MOB(ch, pMob);

  general_show_affects( ch, pMob->affected, argument );

  return FALSE;
}
MEDIT( medit_setaffect ) {
  MOB_INDEX_DATA *pMob;
  EDIT_MOB(ch, pMob);

  return general_set_affect( ch, pMob->affected, argument );
}


void show_liqlist(CHAR_DATA *ch)
{
  int liq;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
    
  buffer = new_buf();
  
  // Modified by SinaC 2000
  add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");
  //for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
  for ( liq = 0; liq < MAX_LIQUID; liq++) {
    /*
      if ( (liq % 21) == 0 )
      add_buf(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n\r");
    */
    
    sprintf(buf, "%-20s %-14s %5d %4d %6d %4d %5d\n\r",
	    liq_table[liq].liq_name,liq_table[liq].liq_color,
	    liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
	    liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
	    liq_table[liq].liq_affect[4] );
    add_buf(buffer,buf);
  }
  
  page_to_char(buf_string(buffer),ch);
  return;
}

// Added by SinaC 2001 for material
void show_material(CHAR_DATA *ch)
{
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  struct material_type* mat;
    
  buffer = new_buf();
  
  add_buf(buffer,"Name           Color          Imm          Res          Vuln        Metal?\n\r");
  for ( int i = 0; i < MAX_MATERIAL; i++) {
    mat = &(material_table[i]);
    const char *imm = flag_string(irv_flags, mat->imm);
    const char *res = flag_string(irv_flags, mat->res);
    const char *vuln = flag_string(irv_flags, mat->vuln);

    sprintf(buf, "%-14s %-14s %-12s %-12s %-12s %d\n\r",
	    mat->name, mat->color,
	    imm, res, vuln,
	    mat->metallic );
    add_buf(buffer,buf);
  }
  
  page_to_char(buf_string(buffer),ch);
  return;
}


void show_damlist(CHAR_DATA *ch)
{
  int att;
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
    
  buffer = new_buf();
    
  for ( att = 0; attack_table[att].name != NULL; att++) {
    if ( (att % 21) == 0 )
      add_buf(buffer,"Name                 Noun\n\r");

    sprintf(buf, "%-20s %-20s\n\r",
	    attack_table[att].name,attack_table[att].noun );
    add_buf(buffer,buf);
  }

  page_to_char(buf_string(buffer),ch);

  return;
}

MEDIT( medit_group )
{
  MOB_INDEX_DATA *pMob;
  MOB_INDEX_DATA *pMTemp;
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int temp;
  BUFFER *buffer;
  bool found = FALSE;
    
  EDIT_MOB(ch, pMob);
    
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax: group [number]\n\r", ch);
    send_to_char( "        group show [number]\n\r", ch);
    return FALSE;
  }
    
  if (is_number(argument)) {
    pMob->group = atoi(argument);
    send_to_char( "Group set.\n\r", ch );
    return TRUE;
  }
    
  argument = one_argument( argument, arg );
    
  if ( !strcmp( arg, "show" ) && is_number( argument ) ) {
    if (atoi(argument) == 0) {
      send_to_char( "Are you crazy?\n\r", ch);
      return FALSE;
    }

    buffer = new_buf ();

    for (temp = 0; temp < 65536; temp++) {
      pMTemp = get_mob_index(temp);
      if ( pMTemp && ( pMTemp->group == atoi(argument) ) ) {
	found = TRUE;
	sprintf( buf, "[%5d] %s\n\r", pMTemp->vnum, pMTemp->player_name );
	add_buf( buffer, buf );
      }
    }

    if (found)
      page_to_char( buf_string(buffer), ch );
    else
      send_to_char( "No mobs in that group.\n\r", ch );

    return FALSE;
  }
    
  return FALSE;
}


MEDIT( medit_easy ) {
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);

  if ( pMob->level <= 0 || pMob->level > 120 ) {
    send_to_char("You can only use these commands on mobs in level range: 1..110\n\r",ch);
    return FALSE;
  }

  pMob->hit[DICE_NUMBER] = hit_nb_easy( pMob->level );
  pMob->hit[DICE_TYPE]   = hit_val_easy( pMob->level );
  pMob->hit[DICE_BONUS]  = hit_add_easy( pMob->level );

  pMob->mana[DICE_NUMBER] = mana_nb_easy( pMob->level );
  pMob->mana[DICE_TYPE]   = mana_val_easy( pMob->level );
  pMob->mana[DICE_BONUS]  = mana_add_easy( pMob->level );

  pMob->psp[DICE_NUMBER] = pMob->mana[DICE_NUMBER];
  pMob->psp[DICE_TYPE]   = pMob->mana[DICE_TYPE];
  pMob->psp[DICE_BONUS]  = pMob->mana[DICE_BONUS];

  pMob->ac[AC_PIERCE] = pMob->ac[AC_BASH] = pMob->ac[AC_SLASH] = armor_easy( pMob->level );
  if ( pMob->ac[AC_PIERCE] <= 0 )
    pMob->ac[AC_EXOTIC] = (2*pMob->ac[AC_PIERCE])/3;
  else
    pMob->ac[AC_EXOTIC] = (4*pMob->ac[AC_PIERCE])/3;

  pMob->damage[DICE_NUMBER] = dam_nb_easy( pMob->level );
  pMob->damage[DICE_TYPE]   = dam_val_easy( pMob->level );
  pMob->damage[DICE_BONUS]  = dam_add_easy( pMob->level );

  pMob->hitroll = hitroll_easy( pMob->level );

  send_to_char("Hitdice, manadice, armor, damdice, hitroll set to easy.\n\r", ch);

  return TRUE;
}

MEDIT( medit_normal ) {
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);
  if ( pMob->level <= 0 || pMob->level > 110 ) {
    send_to_char("You can only use these commands on mobs in level range: 1..110\n\r",ch);
    return FALSE;
  }

  pMob->hit[DICE_NUMBER] = hit_nb_norm( pMob->level );
  pMob->hit[DICE_TYPE]   = hit_val_norm( pMob->level );
  pMob->hit[DICE_BONUS]  = hit_add_norm( pMob->level );

  pMob->mana[DICE_NUMBER] = mana_nb_norm( pMob->level );
  pMob->mana[DICE_TYPE]   = mana_val_norm( pMob->level );
  pMob->mana[DICE_BONUS]  = mana_add_norm( pMob->level );

  pMob->psp[DICE_NUMBER] = pMob->mana[DICE_NUMBER];
  pMob->psp[DICE_TYPE]   = pMob->mana[DICE_TYPE];
  pMob->psp[DICE_BONUS]  = pMob->mana[DICE_BONUS];

  pMob->ac[AC_PIERCE] = pMob->ac[AC_BASH] = pMob->ac[AC_SLASH] = armor_norm( pMob->level );
  if ( pMob->ac[AC_PIERCE] <= 0 )
    pMob->ac[AC_EXOTIC] = (2*pMob->ac[AC_PIERCE])/3;
  else
    pMob->ac[AC_EXOTIC] = (4*pMob->ac[AC_PIERCE])/3;

  pMob->damage[DICE_NUMBER] = dam_nb_norm( pMob->level );
  pMob->damage[DICE_TYPE]   = dam_val_norm( pMob->level );
  pMob->damage[DICE_BONUS]  = dam_add_norm( pMob->level );

  pMob->hitroll = hitroll_norm( pMob->level );

  send_to_char("Hitdice, manadice, armor, damdice, hitroll set to normal.\n\r", ch);

  return TRUE;
}
MEDIT( medit_hard ) {
  MOB_INDEX_DATA *pMob;

  EDIT_MOB(ch, pMob);
  if ( pMob->level <= 0 || pMob->level > 110 ) {
    send_to_char("You can only use these commands on mobs in level range: 1..110\n\r",ch);
    return FALSE;
  }

  pMob->hit[DICE_NUMBER] = hit_nb_hard( pMob->level );
  pMob->hit[DICE_TYPE]   = hit_val_hard( pMob->level );
  pMob->hit[DICE_BONUS]  = hit_add_hard( pMob->level );

  pMob->mana[DICE_NUMBER] = mana_nb_hard( pMob->level );
  pMob->mana[DICE_TYPE]   = mana_val_hard( pMob->level );
  pMob->mana[DICE_BONUS]  = mana_add_hard( pMob->level );

  pMob->psp[DICE_NUMBER] = pMob->mana[DICE_NUMBER];
  pMob->psp[DICE_TYPE]   = pMob->mana[DICE_TYPE];
  pMob->psp[DICE_BONUS]  = pMob->mana[DICE_BONUS];

  pMob->ac[AC_PIERCE] = pMob->ac[AC_BASH] = pMob->ac[AC_SLASH] = armor_hard( pMob->level );
  if ( pMob->ac[AC_PIERCE] <= 0 )
    pMob->ac[AC_EXOTIC] = (2*pMob->ac[AC_PIERCE])/3;
  else
    pMob->ac[AC_EXOTIC] = (4*pMob->ac[AC_PIERCE])/3;

  pMob->damage[DICE_NUMBER] = dam_nb_hard( pMob->level );
  pMob->damage[DICE_TYPE]   = dam_val_hard( pMob->level );
  pMob->damage[DICE_BONUS]  = dam_add_hard( pMob->level );

  pMob->hitroll = hitroll_hard( pMob->level );

  send_to_char("Hitdice, manadice, armor, damdice, hitroll set to hard.\n\r", ch);

  return TRUE;
}


REDIT( redit_owner )
{
  ROOM_INDEX_DATA *pRoom;

  EDIT_ROOM(ch, pRoom);

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  owner [owner]\n\r", ch );
    send_to_char( "         owner none\n\r", ch );
    return FALSE;
  }
  if (!str_cmp(argument, "none"))
    pRoom->owner = &str_empty[0];//str_dup("");
  else {
    //pRoom->owner = str_dup_capitalize( argument );
    char buf[MAX_INPUT_LENGTH];
    strcpy(buf,argument);
    buf[0] = UPPER(buf[0]);
    pRoom->owner = str_dup( buf, TRUE );
  }

  send_to_char( "Owner set.\n\r", ch );
  return TRUE;
}

REDIT( redit_repop_time ) {
  ROOM_INDEX_DATA *pRoom;
  char arg1[MAX_STRING_LENGTH],
    arg2[MAX_STRING_LENGTH];

  EDIT_ROOM(ch, pRoom);
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0'
       || !is_number(arg1) || !is_number(arg2) ) {
    send_to_char( "Syntax: repop [time without people in area] [time with people in area]\n\r", ch);
    return FALSE;
  }

  pRoom->time_between_repop = URANGE( MIN_REPOP_TIME, atoi(arg1), MAX_REPOP_TIME );
  pRoom->time_between_repop_people = URANGE( MIN_REPOP_TIME, atoi(arg2), MAX_REPOP_TIME );

  send_to_char( "Repop time set.\n\r", ch );
  return TRUE;
}


/*
 * Added by SinaC 2000 to allow deletion of rooms, mobiles and objects with OLC
 */
OEDIT( oedit_delete )
{
  OBJ_DATA *obj, *obj_next;
  OBJ_INDEX_DATA *pObj;
  RESET_DATA *pReset, *wReset;
  ROOM_INDEX_DATA *pRoom;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int index, rcount, ocount, i, iHash;

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  oedit delete [vnum]\n\r", ch );
    return FALSE;
  }

  one_argument( argument, arg );

  if( is_number( arg ) ) {
    index = atoi( arg );
    pObj = get_obj_index( index );
  }
  else {
    send_to_char( "That is not a number.\n\r", ch );
    return FALSE;
  }

  if( !pObj ) {
    send_to_char( "No such object.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pObj->area ) ) {
    send_to_char( "OEdit:  Insufficient security to delete obj.\n\r", ch );
    edit_done( ch );
    return FALSE;
  }

  if ( !CAN_EDIT( ch, pObj ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name( pObj ) );
    return FALSE;
  }

  SET_BIT( pObj->area->area_flags, AREA_CHANGED );

  if( top_vnum_obj == index )
    for( i = 1; i < index; i++ )
      if( get_obj_index( i ) )
	top_vnum_obj = i;


  top_obj_index--;

  /* remove objects */
  ocount = 0;
  for( obj = object_list; obj; obj = obj_next ) {
    obj_next = obj->next;

    if( obj->pIndexData == pObj ) {
      extract_obj( obj );
      ocount++;
    }
  }

  /* crush resets */
  rcount = 0;
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next ) {
      for( pReset = pRoom->reset_first; pReset; pReset = wReset ) {
	wReset = pReset->next;
	switch( pReset->command )	{
	case 'O':
	case 'E':
	case 'P':
	case 'G':
	  if ( ( pReset->arg1 == index ) ||
	       ( ( pReset->command == 'P' ) && (pReset->arg3 == index ) ) ) {
	    unlink_reset( pRoom, pReset );
	    rcount++;
	    SET_BIT( pRoom->area->area_flags,
		     AREA_CHANGED );

	  }
	}
      }
    }
  }

  unlink_obj_index( pObj );

  pObj->area = NULL;
  pObj->vnum = 0;

  sprintf( buf, "Removed object vnum {C%d{x and"
	   " {C%d{x resets.\n\r", index,rcount );

  send_to_char( buf, ch );

  sprintf( buf, "{C%d{x occurences of the object"
	   " were extracted from the mud.\n\r", ocount );

  send_to_char( buf, ch );

  // Added by SinaC 2001 to avoid a segfault 
  //  if the Imm deletes the object he was editing
  if ( pObj == ch->desc->pEdit ) {
    edit_done(ch);
  }

  return TRUE;
}


MEDIT( medit_delete )
{
  CHAR_DATA *wch, *wnext;
  MOB_INDEX_DATA *pMob;
  RESET_DATA *pReset, *wReset;
  ROOM_INDEX_DATA *pRoom;
  char arg[MAX_INPUT_LENGTH];
  int index, mcount, rcount, iHash, i;
  bool foundmob = FALSE;
  bool foundobj = FALSE;

  if( argument[0] == '\0' ) {
    send_to_char( "Syntax:  medit delete [vnum]\n\r", ch );
    return FALSE;
  }

  one_argument( argument, arg );

  if( is_number( arg ) ) {
    index = atoi( arg );
    pMob = get_mob_index( index );
  }
  else {
    send_to_char( "That is not a number.\n\r", ch );
    return FALSE;
  }

  if( !pMob ) {
    send_to_char( "No such mobile.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pMob->area ) ) {
    send_to_char( "MEdit:  Insufficient security to delete mobile.\n\r", ch );
    edit_done( ch );
    return FALSE;
  }

  if ( !CAN_EDIT( ch, pMob ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name( pMob ) );
    return FALSE;
  }


  SET_BIT( pMob->area->area_flags, AREA_CHANGED );

  if( top_vnum_mob == index )
    for( i = 1; i < index; i++ )
      if( get_mob_index( i ) )
	top_vnum_mob = i;

  top_mob_index--;

  /* Now crush all resets and take out mobs while were at it */
  rcount = 0;
  mcount = 0;
	

  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next ) {

      for( wch = pRoom->people; wch; wch = wnext ) {
	wnext = wch->next_in_room;
	if( wch->pIndexData == pMob ) {
	  // Added by SinaC 2000
	  if ( wch->fighting != NULL )
	    stop_fighting( wch, TRUE );

	  extract_char( wch, TRUE );
	  mcount++;
	}
      }

      for( pReset = pRoom->reset_first; pReset; pReset = wReset ) {
	wReset = pReset->next;
	switch( pReset->command ) {
	case 'M':
	  if( pReset->arg1 == index ) {
	    foundmob = TRUE;

	    unlink_reset( pRoom, pReset );

	    rcount++;
	    SET_BIT( pRoom->area->area_flags,
		     AREA_CHANGED );

	  }
	  else
	    foundmob = FALSE;

	  break;
	case 'E':
	case 'G':
	  if( foundmob ) {
	    foundobj = TRUE;

	    unlink_reset( pRoom, pReset );

	    rcount++;
	    SET_BIT( pRoom->area->area_flags,
		     AREA_CHANGED );

	  }
	  else
	    foundobj = FALSE;

	  break;
	case '0':
	  foundobj = FALSE;
	  break;
	case 'P':
	  if( foundobj && foundmob ) {
	    unlink_reset( pRoom, pReset );

	    rcount++;
	    SET_BIT( pRoom->area->area_flags,
		     AREA_CHANGED );
	  }
	}
      }
    }
  }


  unlink_mob_index( pMob );


  pMob->area = NULL;
  pMob->vnum = 0;

  send_to_charf( ch, "Removed mobile vnum {C%d{x and"
		 " {C%d{x resets.\n\r", index, rcount );
  send_to_charf( ch, "{C%d{x mobiles were extracted"
		 " from the mud.\n\r",mcount );

  // Added by SinaC 2001 to avoid a segfault 
  //  if the Imm deletes the mobile he was editing
  if ( pMob == ch->desc->pEdit ) {
    edit_done(ch);
  }
  
  return TRUE;
}

REDIT( redit_delete )
{
  ROOM_INDEX_DATA *pRoom, *pRoom2;
  RESET_DATA *pReset;
  EXIT_DATA *ex;
  OBJ_DATA *Obj, *obj_next;
  CHAR_DATA *wch, *wnext;
  EXTRA_DESCR_DATA *pExtra;
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int index, i, iHash, rcount, ecount, mcount, ocount, edcount;

  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  redit delete [vnum]\n\r", ch );
    return FALSE;
  }

  one_argument( argument, arg );

  if( is_number( arg ) ) {
    index = atoi( arg );
    pRoom = get_room_index( index );
  }
  else {
    send_to_char( "That is not a number.\n\r", ch );
    return FALSE;
  }

  if( !pRoom ) {
    send_to_char( "No such room.\n\r", ch );
    return FALSE;
  }

  if ( !IS_BUILDER( ch, pRoom->area ) ) {
    send_to_char( "REdit:  Insufficient security to delete room.\n\r", ch );
    edit_done( ch );
    return FALSE;
  }

  if ( !CAN_EDIT( ch, pRoom ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name( pRoom ) );
    return FALSE;
  }

  /* Move the player out of the room. */
  if( ch->in_room->vnum == index ) {
    send_to_char( "Moving you out of the room"
		  " you are deleting.\n\r", ch);
    if( ch->fighting != NULL )
      stop_fighting( ch, TRUE );

    // Modified by SinaC 2000
    /*
     *char_from_room( ch );
     *char_to_room( ch, get_room_index( 3 ) ); // Limbo
     *ch->was_in_room = ch->in_room;
     *ch->from_room = ch->in_room;
     */
    // Removed by SinaC 2000
    //      ch->was_in_room = ch->in_room;
    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
  }

  SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

  /* Count resets. They are freed by free_room_index. */
  rcount = 0;

  for( pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
    rcount++;
  }

  /* Now contents */
  ocount = 0;
  for( Obj = pRoom->contents; Obj; Obj = obj_next ) {
    obj_next = Obj->next_content;

    extract_obj( Obj );
    ocount++;
  }

  /* Now PCs and Mobs */
  mcount = 0;
  for( wch = pRoom->people; wch; wch = wnext ) {
    wnext = wch->next_in_room;
    if( IS_NPC( wch ) ) {
      extract_char( wch, TRUE );
      mcount++;
    }
    else {
      send_to_char( "This room is being deleted. Moving you somewhere safe.\n\r", wch );
      if( wch->fighting != NULL )
	stop_fighting( wch, TRUE );

      // Modified by SinaC 2000
      /*
       *char_from_room( wch );
       *char_to_room( wch, get_room_index( 3054 ) ); //Midgaard Temple
       *wch->was_in_room = wch->in_room;
       *wch->from_room = wch->in_room;
       */
      // Removed by SinaC 2000
      //	  wch->was_in_room = wch->in_room;
      char_from_room( wch );
      //char_to_room( wch, get_room_index( ROOM_VNUM_ALTAR ) ); 
      char_to_room( wch, get_hall_room(wch) );
    }
  }

  /* unlink all exits to the room. */
  ecount = 0;
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pRoom2 = room_index_hash[iHash]; pRoom2; pRoom2 = pRoom2->next ) {
      for( i = 0; i < MAX_DIR; i++ ) { // <=  Modified by SinaC 2003
	if( !( ex = pRoom2->exit[i] ) )
	  continue;

	if( pRoom2 == pRoom ) {
	  /* these are freed by free_room_index */
	  ecount++;
	  continue;
	}

	if( ex->u1.to_room == pRoom ) {
	  pRoom2->exit[i] = NULL;
	  SET_BIT( pRoom2->area->area_flags, AREA_CHANGED );
	  ecount++;
	}
      }
    }
  }

  /* count extra descs. they are freed by free_room_index */
  edcount = 0;
  for ( pExtra = pRoom->extra_descr; pExtra; pExtra = pExtra->next ) {
    edcount++;
  }

  if( top_vnum_room == index )
    for( i = 1; i < index; i++ )
      if( get_room_index( i ) )
	top_vnum_room = i;

  top_room--;

  unlink_room_index( pRoom );

  pRoom->area = NULL;
  pRoom->vnum = 0;

  /* Na na na na! Hey Hey Hey, Good Bye! */

  sprintf( buf, "Removed room vnum {C%d{x, %d resets, %d extra "
	   "descriptions and %d exits.\n\r", index, rcount, edcount, ecount );
  send_to_char( buf, ch );
  sprintf( buf, "{C%d{x objects and {C%d{x mobiles were extracted "
	   "from the room.\n\r", ocount, mcount );
  send_to_char( buf, ch );

  // Added by SinaC 2001 to avoid a segfault 
  //  if the Imm deletes the room he was editing
  if ( pRoom == ch->desc->pEdit ) {
    edit_done(ch);
  }

  return TRUE;
}


/* unlink a given reset from a given room */
void unlink_reset( ROOM_INDEX_DATA *pRoom, RESET_DATA *pReset )
{
  RESET_DATA *prev, *wReset;

  prev = pRoom->reset_first;
  for( wReset = pRoom->reset_first; wReset; wReset = wReset->next ) {
    if( wReset == pReset ) {
      if( pRoom->reset_first == pReset ) {
	pRoom->reset_first = pReset->next;
	if( !pRoom->reset_first )
	  pRoom->reset_last = NULL;
      }
      else if( pRoom->reset_last == pReset ) {
	pRoom->reset_last = prev;
	prev->next = NULL;
      }
      else
	prev->next = prev->next->next;

      if( pRoom->area->reset_first == pReset )
	pRoom->area->reset_first = pReset->next;

      if( !pRoom->area->reset_first )
	pRoom->area->reset_last = NULL;
    }

    prev = wReset;
  }
}


void unlink_obj_index( OBJ_INDEX_DATA *pObj )
{
  int iHash;
  OBJ_INDEX_DATA *iObj, *sObj;

  iHash = pObj->vnum % MAX_KEY_HASH;

  sObj = obj_index_hash[iHash];

  if( sObj->next == NULL ) /* only entry */
    obj_index_hash[iHash] = NULL;
  else if( sObj == pObj ) /* first entry */
    obj_index_hash[iHash] = pObj->next;
  else {/* everything else */
    for( iObj = sObj; iObj != NULL; iObj = iObj->next ) {
      if( iObj == pObj ) {
	sObj->next = pObj->next;
	break;
      }
      sObj = iObj;
    }
  }
}


void unlink_room_index( ROOM_INDEX_DATA *pRoom )
{
  int iHash;
  ROOM_INDEX_DATA *iRoom, *sRoom;

  iHash = pRoom->vnum % MAX_KEY_HASH;

  sRoom = room_index_hash[iHash];

  if( sRoom->next == NULL ) /* only entry */
    room_index_hash[iHash] = NULL;
  else if( sRoom == pRoom ) /* first entry */
    room_index_hash[iHash] = pRoom->next;
  else {/* everything else */
    for( iRoom = sRoom; iRoom != NULL; iRoom = iRoom->next ){
      if( iRoom == pRoom ) {
	sRoom->next = pRoom->next;
	break;
      }
      sRoom = iRoom;
    }
  }
}


void unlink_mob_index( MOB_INDEX_DATA *pMob )
{
  int iHash;
  MOB_INDEX_DATA *iMob, *sMob;

  iHash = pMob->vnum % MAX_KEY_HASH;

  sMob = mob_index_hash[iHash];

  if( sMob->next == NULL ) /* only entry */
    mob_index_hash[iHash] = NULL;
  else if( sMob == pMob ) /* first entry */
    mob_index_hash[iHash] = pMob->next;
  else {/* everything else */
    for( iMob = sMob; iMob != NULL; iMob = iMob->next ) {
      if( iMob == pMob ) {
	sMob->next = pMob->next;
	break;
      }
      sMob = iMob;
    }
  }
}


/*
 * redit_copy function thanks to Zanthras of Mystical Realities MUD.
 */
REDIT( redit_copy )
{
  ROOM_INDEX_DATA	*pRoom;
  ROOM_INDEX_DATA	*pRoom2; /* Room to copy */
  int vnum;

  if ( argument[0] == '\0' ) {
    send_to_char("Syntax: copy <vnum> \n\r",ch);
    return FALSE;
  }

  if ( !is_number(argument) ) {
    send_to_char("REdit: You must enter a number (vnum).\n\r",ch);
    return FALSE;
  }
  else {/* argument is a number */
    vnum = atoi(argument);
    if( !( pRoom2 = get_room_index(vnum) ) ) {
      send_to_char("REdit: That room does not exist.\n\r",ch);
      return FALSE;
    }
  }

  EDIT_ROOM(ch, pRoom);

  pRoom->description = str_dup( pRoom2->description, TRUE );
    
  pRoom->name = str_dup( pRoom2->name, TRUE );

  /* sector flags */
  // Modified by SinaC 2001
  pRoom->bstat(sector) = pRoom2->bstat(sector);

  /* room flags */
  // Modified by SinaC 2001
  pRoom->bstat(flags) = pRoom2->bstat(flags);

  // Modified by SinaC 2001
  pRoom->bstat(healrate) = pRoom2->bstat(healrate);
  pRoom->bstat(manarate) = pRoom2->bstat(manarate);
// Added by SinaC 2001 for mental user
  pRoom->bstat(psprate) = pRoom2->bstat(psprate);


  pRoom->clan = pRoom2->clan;

  pRoom->owner = str_dup( pRoom2->owner, TRUE );

  pRoom->guild = pRoom2->guild;

  pRoom->bstat(maxsize) = pRoom2->bstat(maxsize);
  pRoom->program = pRoom2->program;
  pRoom->clazz = pRoom2->clazz;
  pRoom->time_between_repop = pRoom2->time_between_repop;
  pRoom->time_between_repop_people = pRoom2->time_between_repop_people;

  pRoom2->base_affected = NULL; // SinaC 2003, copy room affect
  for ( AFFECT_DATA *paf = pRoom->base_affected; paf != NULL; paf = paf->next ) {
    AFFECT_DATA *paf_new = new_affect();
    affect_copy( paf_new, paf ); // copy affect

    paf_new->next = pRoom2->base_affected; // insert in affect list
    pRoom2->base_affected = paf_new;
  }

  //pRoom->extra_descr = pRoom2->extra_descr;

  send_to_char( "Room info copied.\n\r", ch );
  return TRUE;
}


/*
 * oedit_copy function thanks to Zanthras of Mystical Realities MUD.
 */
OEDIT( oedit_copy )
{
  OBJ_INDEX_DATA *pObj;
  OBJ_INDEX_DATA *pObj2; /* The object to copy */
  int vnum, i;

  if ( argument[0] == '\0' ) {
    send_to_char("Syntax: copy <vnum> \n\r",ch);
    return FALSE;
  }

  if ( !is_number(argument) ) {
    send_to_char("OEdit: You must enter a number (vnum).\n\r",ch);
    return FALSE;
  }
  else {/* argument is a number */
    vnum = atoi(argument);
    if( !( pObj2 = get_obj_index(vnum) ) ) {
      send_to_char("OEdit: That object does not exist.\n\r",ch);
      return FALSE;
    }
  }

  EDIT_OBJ(ch, pObj);

  pObj->name = str_dup( pObj2->name, TRUE );

  pObj->item_type = pObj2->item_type;

  pObj->level = pObj2->level;

  pObj->wear_flags  = pObj2->wear_flags;
  pObj->extra_flags = pObj2->extra_flags;

  /* Modified by SinaC 2001
  pObj->material = str_dup( pObj2->material );
  */
  pObj->material = pObj2->material;
    
  pObj->condition = pObj2->condition;

  pObj->weight = pObj2->weight;
  pObj->cost   = pObj2->cost;
  pObj->size   = pObj2->size;

  //pObj->extra_descr = pObj2->extra_descr;

  pObj->short_descr = str_dup( pObj2->short_descr, TRUE );

  pObj->description = str_dup( pObj2->description, TRUE );

  //pObj->affected = pObj2->affected;
  pObj2->affected = NULL; // SinaC 2003, copy object affect
  for ( AFFECT_DATA *paf = pObj->affected; paf != NULL; paf = paf->next ) {
    AFFECT_DATA *paf_new = new_affect();
    affect_copy( paf_new, paf ); // copy affect

    paf_new->next = pObj2->affected; // insert in affect list
    pObj2->affected = paf_new;
  }

  for (i = 0; i < 5; i++) {
    pObj->value[i] = pObj2->value[i];
  }

  pObj->program = pObj2->program;
  pObj2->upgrade = NULL;
  for ( ABILITY_UPGRADE *upgr = pObj->upgrade; upgr; upgr = upgr->next ) {
    ABILITY_UPGRADE *upgr_new = new_ability_upgrade();
    upgr_new->sn = upgr->sn;
    upgr_new->value = upgr->value;

    upgr_new->next = pObj2->upgrade;
    pObj2->upgrade = upgr_new;
  }
  pObj2->restriction = NULL;
  for ( RESTR_DATA *restr = pObj->restriction; restr; restr = restr->next ) {
    RESTR_DATA *restr_new = new_restriction();
    restr_new->type = restr->type;
    restr_new->value = restr->value;
    restr_new->not_r = restr->not_r;
    //    restr_new->ability_r = restr->ability_r;
    restr_new->sn = restr->sn;

    restr_new->next = pObj2->restriction;
    pObj2->restriction = restr_new;
  }

  send_to_char( "Object info copied.\n\r", ch );
  return TRUE;
}


/*
 * medit_copy function thanks to Zanthras of Mystical Realities MUD.
 * Thanks to Ivan for what there is of the incomplete mobprog part.
 * Hopefully it will be finished in a later release of this snippet.
 */
MEDIT( medit_copy )
{
  MOB_INDEX_DATA *pMob;
  MOB_INDEX_DATA *pMob2; /* The mob to copy */
  int vnum;

  /* MPROG_LIST *list; */ /* Used for the mob prog for loop */

  if ( argument[0] == '\0' ) {
    send_to_char("Syntax: copy <vnum> \n\r",ch);
    return FALSE;
  }

  if ( !is_number(argument) ) {
    send_to_char("MEdit: You must enter a number (vnum).\n\r",ch);
    return FALSE;
  }
  else {/* argument is a number */
    vnum = atoi(argument);
    if( !( pMob2 = get_mob_index(vnum) ) ) {
      send_to_char("MEdit: That mob does not exist.\n\r",ch);
      return FALSE;
    }
  }

  EDIT_MOB(ch, pMob);

  pMob->player_name = str_dup( pMob2->player_name, TRUE );

  pMob->new_format = pMob2->new_format;
  pMob->act = pMob2->act;
  pMob->sex = pMob2->sex;
 
  pMob->race = pMob2->race;

  pMob->level = pMob2->level;
  // Modified by SinaC 2000 for alignment/etho    
  pMob->align.alignment = pMob2->align.alignment;
  pMob->align.etho = pMob2->align.etho;

  // Added by SinaC 2001 for class
  pMob->classes = pMob2->classes;
    
  pMob->hitroll = pMob2->hitroll;
    
  pMob->dam_type = pMob2->dam_type;

  pMob->group = pMob2->group;

  pMob->hit[DICE_NUMBER] = pMob2->hit[DICE_NUMBER];
  pMob->hit[DICE_TYPE]   = pMob2->hit[DICE_TYPE];
  pMob->hit[DICE_BONUS]  = pMob2->hit[DICE_BONUS];

  pMob->damage[DICE_NUMBER] = pMob2->damage[DICE_NUMBER];
  pMob->damage[DICE_TYPE]   = pMob2->damage[DICE_TYPE];
  pMob->damage[DICE_BONUS]  = pMob2->damage[DICE_BONUS];
    
  pMob->mana[DICE_NUMBER] = pMob2->mana[DICE_NUMBER];
  pMob->mana[DICE_TYPE]   = pMob2->mana[DICE_TYPE];
  pMob->mana[DICE_BONUS]  = pMob2->mana[DICE_BONUS];
  // Added by SinaC 2001 for mental user
  pMob->psp[DICE_NUMBER] = pMob2->psp[DICE_NUMBER];
  pMob->psp[DICE_TYPE]   = pMob2->psp[DICE_TYPE];
  pMob->psp[DICE_BONUS]  = pMob2->psp[DICE_BONUS];

  pMob->affected_by = pMob2->affected_by;
  // Added by SinaC 2001
  pMob->affected2_by = pMob2->affected2_by;
    
  pMob->ac[AC_PIERCE] = pMob2->ac[AC_PIERCE];
  pMob->ac[AC_BASH]   = pMob2->ac[AC_BASH];
  pMob->ac[AC_SLASH]  = pMob2->ac[AC_SLASH];
  pMob->ac[AC_EXOTIC] = pMob2->ac[AC_EXOTIC];
    

  pMob->form  = pMob2->form;
  pMob->parts = pMob2->parts;

  pMob->imm_flags  = pMob2->imm_flags;
  pMob->res_flags  = pMob2->res_flags;
  pMob->vuln_flags = pMob2->vuln_flags;
  pMob->off_flags  = pMob2->off_flags;

  pMob->size     = pMob2->size;

  pMob->material = str_dup( pMob2->material, TRUE ); 

  pMob->start_pos   = pMob2->start_pos;
  pMob->default_pos = pMob2->default_pos;

  pMob->wealth = pMob2->wealth;

  pMob->spec_fun = pMob2->spec_fun;

  pMob->short_descr = str_dup( pMob2->short_descr, TRUE );

  pMob->long_descr = str_dup( pMob2->long_descr, TRUE );

  pMob->description = str_dup( pMob2->description, TRUE );


  pMob->faction = pMob2->faction;
  pMob->program = pMob2->program;

  pMob2->affected = NULL; // SinaC 2003, copy mobile affect
  for ( AFFECT_DATA *paf = pMob->affected; paf != NULL; paf = paf->next ) {
    AFFECT_DATA *paf_new = new_affect();
    affect_copy( paf_new, paf ); // copy affect

    paf_new->next = pMob2->affected; // insert in affect list
    pMob2->affected = paf_new;
  }

  /* Hopefully get the shop data to copy later
   * This is the fields here if you get it copying send me
   * a copy and I'll add it to the snippet with credit to
   * you of course :) same with the mobprogs for loop :)
   */

  /*
    if ( pMob->pShop )
    {
    SHOP_DATA *pShop, *pShop2;

    pShop =  pMob->pShop;
    pShop2 = pMob2->pShop;
 
    pShop->profit_buy = pShop2->profit_buy;
    pShop->profit_sell = pShop2->profit_sell;
	
    pShop->open_hour = pShop2->open_hour;
    pShop->close_hour = pShop2->close_hour;
	
    pShop->buy_type[iTrade] = pShop2->buy_type[iTrade];
    }
  */
  /*  for loop thanks to Ivan, still needs work though

      for (list = pMob->mprogs; list; list = list->next )
      {
      MPROG_LIST *newp = new_mprog();
      newp->trig_type = list->trig_type;

      newp->trig_phrase = str_dup( list->trig_phrase );

      newp->vnum = list->vnum;

      newp->code = str_dup( list->code );

      pMob->mprogs = newp;
      }
  */

  send_to_char( "Mob info copied.\n\r", ch );
  return FALSE;
}




// General affect edition
// addaffect [<target>] <attrib> <modifier> [<ability/none>] [<flags>] [<duration>] [<level>] [<casting_level>]
//  FIXME optional: flags, duration, ... not yet done
// affect: where, type, location, modifier, [flags], [duration], [level], [casting level]  operator(automatic)
bool general_add_affect( CHAR_DATA *ch, AFFECT_DATA *&starting_list,
			 const int entity_type, // same value as entity: CHAR_ENTITY, OBJ_ENTITY, ROOM_ENTITY
			 OBJ_INDEX_DATA *pObjIndex, // NULL if not an obj
			 const int level,
			 const char *argument ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  char arg4[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 ); // if arg1 not specified: CHAR
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg4 ); // arg4 is optional if arg1 is OBJECT
  if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  addaffect [<target>] <attrib> <modifier> [<ability/none>]\n\r"
		  "See 'help addaffect'\n\r");
    return FALSE;
  }
  //  log_stringf("BEFORE: argument: arg1: %s  arg2: %s  arg3: %s  arg4: %s",
  //	      arg1, arg2, arg3, arg4 );
  long target = flag_value( afto_type, arg1 );
  if ( arg3[0] == '\0' && arg4[0] == '\0' ) { // only 2 arguments: target is char or object or weapon
    if ( target != AFTO_OBJECT && target != AFTO_WEAPON ) // if target was not specified -> char
      target = AFTO_CHAR;
    // we copy arg1 in arg2, and arg2 in arg3
    strcpy( arg3, arg2 ); // arg3 is modifier
    strcpy( arg2, arg1 ); // arg2 is attrib
  }
  else
    if ( target == NO_FLAG ) {
      send_to_charf(ch,"Invalid affect target.\n\r"
		    "Type '? afto' for a list of available target.\n\r");
      return FALSE;
    }
  int sn = 0;
  if ( arg4[0] == '\0' || !str_cmp( arg4, "none" ) )
    sn = -1;
  else if ( ( sn = ability_lookup( arg4 ) ) <= 0 ) {
    send_to_charf(ch,"Invalid ability.\n\r"
		  "Type '? ability <ability type> <target>' for a list of available abilities.\n\r");
    return FALSE;
  }
//  log_stringf("AFTER: argument: arg1: %s  arg2: %s  arg3: %s  arg4: %s",
//	      arg1, arg2, arg3, arg4 );

  int dur = -1;
  int lvl = level;
  int clvl = 0;
  long flags = AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT;
  int vloc = NO_FLAG;
  int vmod = NO_FLAG;
  int vop = NO_FLAG;
  switch( target ) { // attribs depends on affect's target
  case AFTO_CHAR: // modify attr_table
    vloc = flag_value( attr_flags, arg2 );
    if ( vloc == NO_FLAG ) {
      send_to_charf(ch,"Invalid char attributes.\n\r"
		    "Type '? attr' for a list of available char atributes.\n\r");
      return FALSE;
    }
    if ( attr_table[vloc].bits == NULL) {
      if (!is_number(arg3)) {
	send_to_char( "Modifier for this location is numeric.\n\r", ch );
	return FALSE;
      }
      vmod = atoi(arg3);
      vop = AFOP_ADD;
    } 
    else {
      if ( ( vmod = flag_value( attr_table[vloc].bits, arg3 ) ) == NO_FLAG ) {
	send_to_char( "Valid modifiers are:\n\r", ch );
	show_flag_cmds(ch,attr_table[vloc].bits);
	return FALSE;
      }
      vop = is_stat(attr_table[vloc].bits) ? AFOP_ASSIGN : AFOP_OR;        
    }
    break;
  case AFTO_OBJECT: // modify extra fields
    vloc = ATTR_NA;
    vop = AFOP_OR;
    if ( ( vmod = flag_value( extra_flags, arg3 ) ) == NO_FLAG ) {
      send_to_charf(ch,"Invalid object extra flags.\n\r"
		    "Type '? extra' for a list of available obj extra flags.\n\r");
      return FALSE;
    }
    break;
  case AFTO_OBJVAL: // modify value from 0 to 4, only if type is item
    if ( entity_type != OBJ_ENTITY ) {
      send_to_charf(ch,"This target [objval] can only be added on item.\n\r");
      return FALSE;
    }
    send_to_charf(ch,"Not yet implemented.\n\r");
    break;
  case AFTO_WEAPON: // modify value 4 on weapon, only if type is item
    if ( entity_type != OBJ_ENTITY ) {
      send_to_charf(ch,"This kind of affect's target [weapon] can only be added on item.\n\r");
      return FALSE;
    }
    if ( pObjIndex->item_type != ITEM_WEAPON || pObjIndex->value[0] != WEAPON_RANGED ) {
      send_to_charf(ch,"This kind of affect's target [weapon] can only be added on non-ranged weapon.\n\r");
      return FALSE;
    }
    vloc = ATTR_NA;
    vop = AFOP_OR;
    if ( ( vmod = flag_value( weapon_type2, arg3 ) ) == NO_FLAG ) {
      send_to_charf(ch,"Invalid special weapon type.\n\r"
		    "Type '? wtype' for a list of available special weapon type.\n\r");
      return FALSE;
    }
    break;
  case AFTO_ROOM: // modify room_attr_table
    vloc = flag_value( room_attr_flags, arg2 );
    if ( vloc == NO_FLAG ) {
      send_to_charf(ch,"Invalid room attributes.\n\r"
		    "Type '? roomattr' for a list of available room attributes.\n\r");
      return FALSE;
    }
    if ( room_attr_table[vloc].bits == NULL) {
      if (!is_number(arg3)) {
	send_to_char( "Modifier for this location is numeric.\n\r", ch );
	return FALSE;
      }
      vmod = atoi(arg3);
      vop = AFOP_ADD;
    } 
    else {
      if ( ( vmod = flag_value( room_attr_table[vloc].bits, arg3 ) ) == NO_FLAG ) {
	send_to_char( "Valid modifiers are:\n\r", ch );
	show_flag_cmds(ch,room_attr_table[vloc].bits);
	return FALSE;
      }
      vop = is_stat(room_attr_table[vloc].bits) ? AFOP_ASSIGN : AFOP_OR;        
    }
    break;
  default:
    send_to_charf(ch,"Invalid target.\n\r"
		  "Type '? afto' for a list of available target.\n\r");
    bug("general_add_affect: unknown target: %ld", target);
    return FALSE;
  }

  AFFECT_DATA *pAf             =   new_affect();
  if ( sn > 0 ) {
    SET_BIT( flags, AFFECT_ABILITY );
    REMOVE_BIT( flags, AFFECT_INHERENT );
  }
  createaff( *pAf, dur, lvl, sn, clvl, flags );
  addaff2( *pAf, target, vloc, vop, vmod );
  pAf->next       =   starting_list;
  starting_list  =   pAf;

  send_to_charf(ch,"Affect added.\n\r");
  return TRUE;
}

// delaffect <#affectId>
bool general_del_affect( CHAR_DATA *ch, AFFECT_DATA *&start_list, const char *argument ) {
  char affect[MAX_STRING_LENGTH];
  one_argument( argument, affect );

  if ( !is_number( affect ) || affect[0] == '\0' ) {
    send_to_char( "Syntax:  delaffect [#xaffect]\n\r", ch );
    return FALSE;
  }

  int value = atoi( affect );

  if ( value < 0 ) {
    send_to_char( "Only non-negative affect-numbers allowed.\n\r", ch );
    return FALSE;
  }

  if ( start_list == NULL ) {
    send_to_char( "Edit:  Non-existant affect.\n\r", ch );
    return FALSE;
  }

  AFFECT_DATA *pAf =  start_list;
  AFFECT_DATA *pAf_next = NULL;
  if( value == 0 ) // First case: Remove first affect
    start_list = pAf->next;
  else {   	   // Affect to remove is not the first
    int cnt = 0;
    while ( ( pAf_next = pAf->next ) && ( ++cnt < value ) )
      pAf = pAf_next;

    if( pAf_next )		// See if it's the next affect
      pAf->next = pAf_next->next;
    else {                                 // Doesn't exist
      send_to_char( "No such affect.\n\r", ch );
      return FALSE;
    }
  }
  send_to_char( "Affect removed.\n\r", ch);
  return TRUE;
}

// setaffect <#affectId> <field> <value>
//  field: sn         location     modifier   level
//         casting    flag         duration   list
bool general_set_affect( CHAR_DATA *ch, AFFECT_DATA *&start_list, const char *argument ) {
  char affectArg[MAX_STRING_LENGTH];
  char fieldArg[MAX_STRING_LENGTH];
  char valueArg[MAX_STRING_LENGTH];
  argument = one_argument( argument, affectArg );
  argument = one_argument( argument, fieldArg );
  argument = one_argument( argument, valueArg );

  // Find affect
  int affectId = atoi( affectArg );
  if ( affectId < 0 ) {
    send_to_charf(ch,"Only non-negative affect-numbers allowed.\n\r");
    return FALSE;
  }
  if ( start_list == NULL ) {
    send_to_char( "Edit:  Non-existant affect.\n\r", ch );
    return FALSE;
  }
  AFFECT_DATA *pAf; int cnt;
  for ( cnt = 0, pAf = start_list; pAf; pAf = pAf->next )
    if ( cnt++ == affectId )
      break;
  if ( pAf == NULL ) {
    send_to_charf(ch,"No such affect.\n\r");
    return FALSE;
  }
  // Affect has been found: pAf
  // Which field must be modified ?
  if ( !str_cmp( fieldArg, "sn" )
       || !str_cmp( fieldArg, "type" ) ) { // sn, type: just check if valueArg is NONE or an ability
    int value = -1;
    if ( str_cmp( valueArg, "none" ) )
      if ( ( value = ability_lookup( valueArg ) ) <= 0 ) {
	send_to_charf(ch,"%s is not a valid ability.\n\r"
		      "none  may be used if you want a inherent affect (not given by an ability)\n\r", valueArg );
	return FALSE;
      }
    pAf->type = value;
    send_to_charf(ch,"Type of affect [%d] set.\n\r", affectId );
    return TRUE;
  }
  if ( !str_cmp( fieldArg, "list" ) ) {
    pAf->list = NULL;
    send_to_charf(ch,"List of affect [%d] removed.\n\r", affectId );
    return TRUE;
  }
  if ( !str_cmp( fieldArg, "duration" ) ) {
    if ( !str_prefix( valueArg, "unlimited" ) ) {
      pAf->duration = -1;
      SET_BIT( pAf->flags, AFFECT_PERMANENT );
    }
    else {
      int value = atoi( valueArg );
      pAf->duration = value;
    }
    send_to_charf(ch,"Duration of affect [%d] set.\n\r", affectId );
    return TRUE;
  }
  if ( !str_cmp( fieldArg, "level" ) ) {
    int value = atoi( valueArg );
    pAf->level = value;
    send_to_charf(ch,"Level of affect [%d] set.\n\r", affectId );
    return TRUE;
  }
  if ( !str_cmp( fieldArg, "casting" ) ) {
    int value = atoi( valueArg );
    pAf->casting_level = value;
    send_to_charf(ch,"Casting level of affect [%d] set.\n\r", affectId );
    return TRUE;
  }
  if ( !str_cmp( fieldArg, "flags" ) ) {
    int value = flag_value( affect_data_flags, valueArg );
    if ( value == NO_FLAG ) {
      send_to_charf(ch,"Invalid affect flag.\n\r"
		    "Type '? affectflag' for a complete list of affect flags.\n\r");
      return FALSE;
    }
    TOGGLE_BIT( pAf->flags, value );
    send_to_charf(ch,"Flag of affect [%d] toggled.\n\r", affectId );
    return TRUE;
  }
  if ( !str_prefix( fieldArg, "location" ) ) { // location: depends of where
    AFFECT_LIST *lAf = pAf->list;
    if ( lAf == NULL ) {
      send_to_charf(ch,"Problem while setting affect.\n\r");
      bug("general_set_affect: NULL paf->list  paf(sn:%d)", pAf->type);
      return FALSE;
    }
    switch( lAf->where ) {
    case AFTO_OBJECT: case AFTO_WEAPON:
      send_to_charf(ch,"This affect has a static location.\n\r");
      return FALSE;
      break;
    case AFTO_OBJVAL:
      send_to_charf(ch,"Not Yet Implemented.\n\r");
      return FALSE;
      break;
    case AFTO_CHAR: {
      int vloc = flag_value( attr_flags, valueArg );
      if ( vloc == NO_FLAG ) {
	send_to_charf(ch,"Invalid char attributes (location).\n\r"
		      "Type '? attr' for a list of available char atributes (location).\n\r");
	return FALSE;
      }
      lAf->location = vloc;
      lAf->modifier = 0; // Modifier must be resetted because it depends of location
      send_to_charf(ch,"Modifier resetted.\n\r");
      break;
    }
    case AFTO_ROOM: {
      int vloc = flag_value( room_attr_flags, valueArg );
      if ( vloc == NO_FLAG ) {
	send_to_charf(ch,"Invalid room attributes (location).\n\r"
		      "Type '? roomattr' for a list of available room attributes (location).\n\r");
	return FALSE;
      }
      lAf->location = vloc;
      lAf->modifier = 0; // Modifier must be resetted because it depends of location
      send_to_charf(ch,"Modifier resetted.\n\r");
    }
    }
    send_to_charf(ch,"Location of affect %d set.\n\r", affectId );
    return TRUE;
  }
  if ( !str_prefix( fieldArg, "modifier" ) ) {
    AFFECT_LIST *lAf = pAf->list;
    if ( lAf == NULL ) {
      send_to_charf(ch,"Problem while setting affect.\n\r");
      bug("general_set_affect: NULL paf->list  paf(sn:%d)", pAf->type);
      return FALSE;
    }
    switch ( lAf->where ) {
    case AFTO_CHAR: {
      int vloc = lAf->location;
      int vmod = NO_FLAG;
      int vop = NO_FLAG;
      if ( attr_table[vloc].bits == NULL) {
	if (!is_number(valueArg)) {
	  send_to_char( "Modifier for this location is numeric.\n\r", ch );
	  return FALSE;
	}
	vmod = atoi(valueArg);
	vop = AFOP_ADD;
      } 
      else {
	if ( ( vmod = flag_value( attr_table[vloc].bits, valueArg ) ) == NO_FLAG ) {
	  send_to_char( "Valid modifiers are:\n\r", ch );
	  show_flag_cmds(ch,attr_table[vloc].bits);
	  return FALSE;
	}
	vop = is_stat(attr_table[vloc].bits) ? AFOP_ASSIGN : AFOP_OR;        
      }
      lAf->modifier = vmod;
      lAf->op = vop;
      break;
    }
    case AFTO_ROOM: {
      int vloc = lAf->location;
      int vmod = NO_FLAG;
      int vop = NO_FLAG;
      if ( room_attr_table[vloc].bits == NULL) {
	if (!is_number(valueArg)) {
	  send_to_char( "Modifier for this location is numeric.\n\r", ch );
	  return FALSE;
	}
	vmod = atoi(valueArg);
	vop = AFOP_ADD;
      } 
      else {
	if ( ( vmod = flag_value( room_attr_table[vloc].bits, valueArg ) ) == NO_FLAG ) {
	  send_to_char( "Valid modifiers are:\n\r", ch );
	  show_flag_cmds(ch,room_attr_table[vloc].bits);
	  return FALSE;
	}
	vop = is_stat(room_attr_table[vloc].bits) ? AFOP_ASSIGN : AFOP_OR;        
      }
      lAf->modifier = vmod;
      lAf->op = vop;
      break;
    }
    }
    send_to_charf(ch,"Modifier of affect %d set.\n\r", affectId );
    return TRUE;
  }
  send_to_charf(ch,"Syntax:  setaffect <#affectId> <field> <value>\n\r"
		"Field: type, location, modifier, duration, casting, level, flags.\n\r");
  return FALSE;
}

// showaffects [all/#affect]
void general_show_affects( CHAR_DATA *ch, AFFECT_DATA *afList, const char *argument ) {
  // show all affects
  if ( !str_cmp( argument, "all" ) || !str_cmp( argument, "auto" ) ) {
    // Affects
    AFFECT_DATA *paf = afList;
    for ( int cnt = 0; paf != NULL; paf = paf->next ) {
      if ( cnt == 0 ) {
	send_to_char( "Number To        Location    Op  Modifier\n\r", ch );
	send_to_char( "------ ------ -------------- --- --------\n\r", ch );
      }                         
      // SinaC 2003: new affect system
      send_to_charf(ch,"[%4d] ", cnt );
      if ( paf->list == NULL ) { // no list -> only where information
	send_to_charf( ch, "****** Use showaffect %-3d ******\n\r", cnt );
      }
      else {
	bool first = TRUE;
	for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
	  if ( !first )
	    send_to_charf( ch,"       ");
	  send_to_charf( ch, "%6s ", flag_string( afto_type , laf->where ));
	  switch (laf->where) {
	  case AFTO_CHAR :
	    send_to_charf(ch,"%14s ", flag_string( attr_flags, laf->location));
	    send_to_charf(ch,"%3s ", flag_string( ops_flags, laf->op));
	    if (attr_table[laf->location].bits == NULL)
	      send_to_charf(ch,"%6ld",laf->modifier);
	    else
	      send_to_charf(ch,"%s",
			    flag_string(attr_table[laf->location].bits, laf->modifier));
	    break;
	  case AFTO_OBJECT :
	    send_to_charf(ch,"%14s %3s ",
			  "N/A",
			  flag_string( ops_flags, laf->op));
	    send_to_charf(ch,"%s",flag_string( extra_flags, laf->modifier) );
	    break;
	  case AFTO_WEAPON :
	    send_to_charf(ch,"%14s %3s ",
			  "N/A",
			  flag_string( ops_flags, laf->op));
	    send_to_charf(ch,"%s",flag_string( weapon_type2, laf->modifier ) );
	    break;
	  case AFTO_OBJVAL :
	    send_to_charf(ch,"%14d %3s %ld",
			  laf->location,
			  flag_string( ops_flags, laf->op),
			  laf->modifier);
	    break;
	  case AFTO_ROOM:
	    send_to_charf(ch,"%14s ", flag_string( room_attr_flags, laf->location));
	    send_to_charf(ch,"%3s ", flag_string( ops_flags, laf->op));
	    if (room_attr_table[laf->location].bits == NULL)
	      send_to_charf(ch,"%6ld",laf->modifier);
	    else
	      send_to_charf(ch,"%s",
			    flag_string(room_attr_table[laf->location].bits, laf->modifier));
	    break;
	  }
	  send_to_char("\n\r",ch);
	  first = FALSE;
	}
      }
      cnt++;
    }
  }
  else {
    // Find affect
    int affectId = atoi( argument );
    if ( affectId < 0 ) {
      send_to_charf(ch,"Only non-negative affect-numbers allowed.\n\r");
      return;
    }
    if ( afList == NULL ) {
      send_to_char( "Edit:  Non-existant affect.\n\r", ch );
      return;
    }
    AFFECT_DATA *pAf; int cnt;
    for ( cnt = 0, pAf = afList; pAf; pAf = pAf->next )
      if ( cnt++ == affectId )
	break;
    if ( pAf == NULL ) {
      send_to_charf(ch,"No such affect.\n\r");
      return;
    }
    // Affect found, display
    send_to_charf( ch, "Type: [%s]\n\r", pAf->type > 0 ? ability_table[pAf->type].name:"none" );
    if ( pAf->duration >= 0 )
      send_to_charf( ch, "Duration: [%d]\n\r", pAf->duration );
    else
      send_to_charf( ch, "Duration: [Unlimited]\n\r" );
    send_to_charf( ch, "Level: [%d]\n\r", pAf->level );
    if ( pAf->type > 0 )
      send_to_charf( ch, "Casting Level: [%d]\n\r", pAf->casting_level );
    else
      send_to_charf( ch, "Casting Level: [N/A]\n\r" );
    send_to_charf( ch, "Flags: %s\n\r", flag_string( affect_data_flags, pAf->flags ) );
    bool first = TRUE;
    for ( AFFECT_LIST *laf = pAf->list; laf != NULL; laf = laf->next ) {
      if ( !first )
	send_to_charf(ch,"------------\n\r");
      send_to_charf( ch, "  To: [%s]\n\r", flag_string( afto_type , laf->where ) );
      switch (laf->where) {
      case AFTO_CHAR :
	send_to_charf( ch, "  Location: [%s]\n\r", flag_string( attr_flags, laf->location) );
	send_to_charf( ch, "  Operator: [%s]\n\r", flag_string( ops_flags, laf->op) );
	if (attr_table[laf->location].bits == NULL)
	  send_to_charf( ch, "  Modifier: %6ld", laf->modifier );
	else
	  send_to_charf( ch, "  Modifier: %s",
			 flag_string(attr_table[laf->location].bits, laf->modifier) );
	break;
      case AFTO_OBJECT :
	send_to_charf( ch, "  Location: [N/A]\n\r" );
	send_to_charf( ch, "  Operator: [%s]\n\r", flag_string( ops_flags, laf->op) );
	send_to_charf( ch, "  Modifier: %s\n\r", flag_string( extra_flags, laf->modifier) );
	break;
      case AFTO_WEAPON :
	send_to_charf( ch, "  Location: [N/A]\n\r" );
	send_to_charf( ch, "  Operator: [%s]\n\r", flag_string( ops_flags, laf->op) );
	send_to_charf( ch, "  Modifier: %s\n\r",flag_string( weapon_type2, laf->modifier ) );
	break;
      case AFTO_OBJVAL :
	send_to_charf( ch, "  Location: [N/A]\n\r" );
	send_to_charf( ch, "  Operator: [%s]\n\r", flag_string( ops_flags, laf->op) );
	send_to_charf( ch, "  Modifier: %6ld\n\r", laf->modifier );
	break;
      case AFTO_ROOM:
	send_to_charf( ch, "  Location: [%s]\n\r", flag_string( room_attr_flags, laf->location) );
	send_to_charf( ch, "  Operator: [%s]\n\r", flag_string( ops_flags, laf->op) );
	if (room_attr_table[laf->location].bits == NULL)
	  send_to_charf( ch, "  Modifier: %6ld", laf->modifier );
	else
	  send_to_charf( ch, "  Modifier: %s",
			 flag_string(room_attr_table[laf->location].bits, laf->modifier) );
	break;
      }
      first = FALSE;
    }
  }
}
