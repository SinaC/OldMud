/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "tables.h"

// Added by SinaC 2001
#include "db.h"
#include "interp.h"

int flag_lookup (const char *name, struct flag_type *flag_table) {
  int flag;

  for (flag = 0; flag_table[flag].name != NULL; flag++) {
    if (LOWER(name[0]) == LOWER(flag_table[flag].name[0])
	&&  !str_prefix(name,flag_table[flag].name))
      return flag_table[flag].bit;
  }

  return 0;
}

int position_lookup (const char *name) {
  int pos;

  for (pos = 0; position_table[pos].name != NULL; pos++) {
    if (LOWER(name[0]) == LOWER(position_table[pos].name[0])
	&&  !str_prefix(name,position_table[pos].name))
      return pos;
  }
   
  return -1;
}

int sex_lookup (const char *name) {
  int sex;
   
  for (sex = 0; sex_table[sex].name != NULL; sex++) {
    if (LOWER(name[0]) == LOWER(sex_table[sex].name[0])
	&&  !str_prefix(name,sex_table[sex].name))
      return sex;
  }

  return -1;
}

int size_lookup (const char *name) {
  int size;
 
  for ( size = 0; size_table[size].name != NULL; size++) {
    if (LOWER(name[0]) == LOWER(size_table[size].name[0])
        &&  !str_prefix( name,size_table[size].name))
      return size;
  }
 
  return -1;
}

int hometown_lookup(const char * name) {
//  int town;
//
//  if (!*name) return -1;
//  for (town = 0; hometown_table[town].recall;town++)
//    if (!str_prefix(name,hometown_table[town].name))
//      break;
//  if (!hometown_table[town].recall) return -1;
//  return town;
  if (!*name) return -1;
  for ( int town = 0; town < MAX_HOMETOWN; town ++ )
    if ( !str_prefix(name,hometown_table[town].name) )
      return town;
  return -1;
}
     
/* returns material number, Modified by SinaC 2001 */
int material_lookup (const char *name) {

  if ( !str_cmp(name, "none" ) ) // Added by SinaC 2003
    return 0;

  for ( int i = 0; i < MAX_MATERIAL; i++ ) {
    if ( !str_cmp( name, material_table[i].name ) )
      return i;
  }
  
  return -1;

// return 0;
}

/* returns race number */
int race_lookup (const char *name, const bool complete = FALSE) {
  int race;
  if ( complete ) { // use str_cmp instead of str_prefix
    for ( race = 0; race < MAX_RACE; race++) {
      if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	  &&  !str_cmp( name,race_table[race].name))
	return race;
    }
  }
  else {
    // Modified by SinaC 2000
    //for ( race = 0; race_table[race].name != NULL; race++)
    for ( race = 0; race < MAX_RACE; race++) {
      if (LOWER(name[0]) == LOWER(race_table[race].name[0])
	  &&  !str_prefix( name,race_table[race].name))
	return race;
    }
  }
    // Modified by SinaC 2000
    //return 0;
    return -1;
} 

int liq_lookup (const char *name) {
  int liq;

  // Modified by SinaC 2000
  //for ( liq = 0; liq_table[liq].liq_name != NULL; liq++)
  for ( liq = 0; liq < MAX_LIQUID; liq++) {
    if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
	&& !str_prefix(name,liq_table[liq].liq_name))
      return liq;
  }
  
  return -1;
}

int weapon_lookup (const char *name) {
  int type;

  for (type = 0; weapon_table[type].name != NULL; type++) {
    if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
      return type;
  }
 
  return -1;
}

int item_lookup(const char *name) {
  int type;

  for (type = 0; item_table[type].name != NULL; type++) {
    if (LOWER(name[0]) == LOWER(item_table[type].name[0])
	&&  !str_prefix(name,item_table[type].name))
      return item_table[type].type;
  }
 
  return -1;
}

int attack_lookup  (const char *name) {
  int att;

  for ( att = 0; attack_table[att].name != NULL; att++) {
    if (LOWER(name[0]) == LOWER(attack_table[att].name[0])
	&&  !str_prefix(name,attack_table[att].name))
      return att;
  }

  return 0;
}

/* returns a flag for wiznet */
long wiznet_lookup (const char *name) {
  int flag;

  for (flag = 0; wiznet_table[flag].name != NULL; flag++) {
    if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
	&& !str_prefix(name,wiznet_table[flag].name))
      return flag;
  }

  return -1;
}

/* returns class number */
int class_lookup (const char *name, const bool fComplete, int n = -1 ) {
  if ( n == -1 )
    n = MAX_CLASS;

  if ( fComplete ) {
    for ( int iclass = 0; iclass < n; iclass++) {
      if (LOWER(name[0]) == LOWER(class_table[iclass].name[0])
	  &&  !str_cmp( name,class_table[iclass].name))
	return iclass;
    }
  }
  else {
    for ( int iclass = 0; iclass < n; iclass++) {
      if (LOWER(name[0]) == LOWER(class_table[iclass].name[0])
	  &&  !str_prefix( name,class_table[iclass].name))
	return iclass;
    }
  }

  return -1;
}

// Added by SinaC 2001
// returns god number
int god_lookup (const char *name) {
  int god;

  for ( god = 0; god < MAX_GODS; god++) {
    if (LOWER(name[0]) == LOWER(gods_table[god].name[0])
	&&  !str_prefix( name,gods_table[god].name))
      return god;
  }

  return -1;
} 

/*
 * Lookup an ability by name.
 */
int ability_lookup( const char *name, const int from = 0 ) {
  for ( int sn = from; sn < MAX_ABILITY; sn++ ) {
    if ( ability_table[sn].name == NULL )
      break;
    if ( LOWER(name[0]) == LOWER(ability_table[sn].name[0])
	 &&   !str_prefix( name, ability_table[sn].name ) )
      return sn;
  }

  return -1;
}

/*
 * Lookup an ability by slot number.
 * Used for object loading.
 */
int slot_lookup( const int slot ) {
  int sn;

  if ( slot <= 0 )
    return -1;

  for ( sn = 0; sn < MAX_ABILITY; sn++ ) {
    if ( slot == ability_table[sn].slot )
      return sn;
  }

  if ( fBootDb ) {
    bug( "Slot_lookup: bad slot %d.", slot );
    //abort( );
    return -1;
  }

  return -1;
}

// Added by SinaC 2001
int command_lookup( const char *name ) {
  for ( int i = 0; i < MAX_COMMANDS; i++ )
    if ( !str_prefix( name, cmd_table[i].name ) )
      return i;
  return -1;
}

// Added by SinaC 2001, removed by SinaC 2003: must use flag_string/flag_value
//int classtype_lookup( const char *name ) {
//  if ( !str_cmp( name, "magic" ) )
//    return CLASS_MAGIC;
//  else if ( !str_cmp( name, "mental" ) )
//    return CLASS_MENTAL;
//  else if ( !str_cmp( name, "combat" ) )
//    return CLASS_COMBAT;
//  return -1;
//}

// Modified by SinaC 2003 for bard (songs)
int abilitytype_lookup ( const char *name ) {
  if ( !str_cmp(name,"skill") )
    return TYPE_SKILL;
  else if ( !str_cmp(name,"spell") )
    return TYPE_SPELL;
  else if ( !str_cmp(name,"power") )
    return TYPE_POWER;
  else if ( !str_cmp(name,"song") )
    return TYPE_SONG;
  else if ( !str_cmp(name,"specialaffect") )
    return TYPE_AFFECT;
  
  return -1;
}

int script_type_lookup( const char *name ) {
  if ( !str_cmp( name, "int") ) return SVT_INT;
  if ( !str_cmp( name, "string" ) ) return SVT_STR;
  if ( !str_cmp( name, "entity" ) ) return SVT_ENTITY;
  if ( !str_cmp( name, "function" ) ) return SVT_FCT;
  if ( !str_cmp( name, "var" ) ) return SVT_VAR;
  if ( !str_cmp( name, "list" ) ) return SVT_LIST;
  if ( !str_cmp( name, "void" ) ) return SVT_VOID;
  return -1;
}

int magic_school_lookup( const char *name ) {
  for ( int i = 0; i < MAX_SCHOOL; i++ )
    if ( !str_cmp( name, magic_school_table[i].name ) )
      return i;
  return -1;
}

int super_race_lookup( const char *name ) {
  for ( int i = 0; i < MAX_SUPERRACE; i++ )
    if ( !str_cmp( super_pc_race_table[i].name, name ) )
      return i;
  return -1;
}
