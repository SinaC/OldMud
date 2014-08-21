/*
 *
 * Added by SinaC 2000 for object restrictions
 *  restriction table structure are based on the same principle as OLC
 *
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
//#include "olc.h"
#include "restriction.h"
#include "db.h"
#include "comm.h"
// Added by SinaC 2001
#include "handler.h"
// Added by SinaC 2003
#include "olc_value.h"
#include "bit.h"


// Added by SinaC 2000 for object restriction
//  called in db.C after classes and races have been read from files
//  so classes_flags and races_flags are correctly initialized
void init_restr_table() {
  extern const struct attr_type *restr_table;
  static const struct attr_type restr_table_source[]= {
    { "strength",       NULL },
    { "intelligence",   NULL },
    { "wisdom",         NULL },
    { "dexterity",      NULL },
    { "constitution",   NULL },
    { "sex",            sex_flags },
    { "classes",        classes_flags },
    { "race",           races_flags },
    { "parts",          part_flags },
    { "form",           form_flags },
    // Added by SinaC 2001 to replace ANTI_NEUTRAL, ANTI_GOOD, ANTI_EVIL
    { "etho",           etho_flags },
    { "alignment",      NULL }
  };

  restr_table = restr_table_source;  
}

// Added by SinaC 2001 for object restriction
// This is used to update the restr_table if pointer to a table has been modified
//  ie: when we reload the race table from file
void update_restr_table( const char *name, flag_type *flag ) {
  int i;
  extern const struct attr_type *restr_table;
  struct attr_type *l_restr_table = (struct attr_type *)restr_table;
  // it's the only place in the code where we modify it :)
  for ( i = 0; l_restr_table[i].name != NULL; i++ )
    if ( !str_cmp( l_restr_table[i].name, name ) )
      break;
  if ( l_restr_table[i].name == NULL ) {
    bug("update_restr_table: Invalid restr table entry: %s", name );
    exit(-1);
  }
  l_restr_table[i].bits = flag;
}

// Initialized in db.C: call init_restr_table()
const struct attr_type *restr_table;

// I have used flag_type because I didn't want to create a new struct
// ==> the last field is useless, the first is useless too :))))
//  bit (2nd value) must be the same as in tables.C:attr_table
struct flag_type restr_attr[]= {
  { "strength",       ATTR_STR,       TRUE },
  { "intelligence",   ATTR_INT,       TRUE },
  { "wisdom",         ATTR_WIS,       TRUE },
  { "dexterity",      ATTR_DEX,       TRUE },
  { "constitution",   ATTR_CON,       TRUE },
  { "sex",            ATTR_sex,       TRUE },
  { "classes",        ATTR_classes,   TRUE },
  { "race",           ATTR_race,      TRUE },
  { "parts",          ATTR_parts,     TRUE },
  { "form",           ATTR_form,      TRUE },
  // Added by SinaC 2001 to replace ANTI_NEUTRAL, ANTI_GOOD, ANTI_EVIL
  { "etho",           ATTR_etho,      TRUE },
  { "alignment",      ATTR_alignment, TRUE },
  { NULL,             0,              FALSE }
};


// Modified by SinaC 2001
bool check_one_restriction( CHAR_DATA *ch, RESTR_DATA *restr ) {
  // Added by SinaC 2000 for restriction on ability
  //if ( restr->ability_r ) {
  if ( restr->type == RESTR_ABILITY ) {
    if ( restr->not_r == FALSE ) {
      if ( get_ability( ch, restr->sn ) < restr->value )
	return FALSE;
    }
    else
      if ( get_ability( ch, restr->sn ) >= restr->value )
	return FALSE;
  }
  // If not an restriction on ability than it's a restriction on a stat
  else {
    // Modified by SinaC 2000 for NOT restriction

    // >=         str, int, wis, dex, con, alig   if not_r not set
    // <                                          if not_r set
    if ( restr_table[restr->type].bits == NULL ) {
      if ( restr->not_r == FALSE ) {
	if ( ch->curattr[restr_attr[restr->type].bit] < restr->value )
	  return FALSE;
      }
      else
	if ( ch->curattr[restr_attr[restr->type].bit] >= restr->value )
	  return FALSE;
    }
    // =          race, sex, etho              if not_r not set
    // !=                                      if not_r set
    else if ( is_stat( restr_table[restr->type].bits ) ) {
      if ( restr->not_r == FALSE ) {
	if ( ch->curattr[restr_attr[restr->type].bit] != restr->value )
	  return FALSE;
      }
      else
	if ( ch->curattr[restr_attr[restr->type].bit] == restr->value )
	  return FALSE;
    }
    // or         parts, classes, form    if not_r not set
    // nor                                if not_r set
    else {
      if ( restr->not_r == FALSE ) {
	if (( ch->curattr[restr_attr[restr->type].bit] & restr->value ) != restr->value)
	  return FALSE;
      }
      else
	if (( ch->curattr[restr_attr[restr->type].bit] & restr->value ) == restr->value)
	  return FALSE;
    }
  }
  return TRUE;
}

// Added by SinaC 2003, size is not anymore a restriction but an obj stat
//  But we still check here if the item can be worn
//  return TRUE if the item can be worn, FALSE otherwise
bool check_size( CHAR_DATA *ch, OBJ_DATA *obj, bool silent ) {
  if ( obj->size != SIZE_NOSIZE ) { // if item has a size
    if ( obj->size < ch->cstat(size) ) { // item's size smaller than char's size
      if ( !silent ) {
	act("$p is too small for you to use.", ch, obj, NULL, TO_CHAR );
	act("$n is too tall to use $p.", ch, obj, NULL, TO_ROOM);
      }
      return FALSE;
    } 
    else if ( obj->size > ch->cstat(size) ) { // item's size greater than char's size
      if ( !silent ) {
	act("$p is too big for you to use.", ch, obj, NULL, TO_CHAR );
	act("$n is too small to use $p.", ch, obj, NULL, TO_ROOM);
      }
      return FALSE;
    }
  }
  return TRUE;
}

// Return TRUE if the item can be worn, FALSE otherwise
// if many restriction are done on an attributes requiring strict equality
//  we consider char must have one of this attributes
// examples: restriction  race == high-elf
//                        race == grey-elf
//                        race == sylvan-elf
// if char's race is high-elf or grey-elf or sylvan-elf the test will be passed
//           restriction  etho == lawful
//                        etho == neutral
// only chaotic char will not be able to wear this, equivalent to restriction  etho != chaotic
bool check_restriction( CHAR_DATA *ch, OBJ_DATA *obj ) {
  const int MAX_RESTRICTION = 16;
  // max 16 restrictions, first column gives nuber of restrictions
  long restriction_or[ATTR_NUMBER][1+MAX_RESTRICTION];
  memset( restriction_or, 0, ATTR_NUMBER * (1+MAX_RESTRICTION) * sizeof(long) );

  bool found = FALSE;
  // obj restriction: check non-stat restriction and store stat restriction
//  for ( RESTR_DATA *restr = obj->restriction; restr != NULL; restr = restr->next ) {
//    int type = restr->type;
//    // stat restriction: race, sex, etho
//    if ( restr_table[type].bits != NULL && is_stat( restr_table[type].bits ) ) {
//      int loc = restr_attr[type].bit;
//      if ( restr->not_r == FALSE ) {// if non not-restriction: store value
//	found = TRUE;
//	restriction_or[loc][++restriction_or[loc][0]] = restr->value;
//      }
//      else if ( ch->curattr[restr_attr[restr->type].bit] == restr->value ) // if not-restriction: just test
//	return FALSE;
//    }
//    else if (!check_one_restriction( ch, restr ))
//      return FALSE;
//  }

  // objIndex restriction: check non-stat restriction and store stat restriction
  //  if (!obj->enchanted)
    for ( RESTR_DATA *restr = obj->pIndexData->restriction; restr != NULL; restr = restr->next ) {
      int type = restr->type;
      // stat restriction: race, sex, etho, ...
      if ( restr_table[type].bits != NULL && is_stat( restr_table[type].bits ) ) {
	int loc = restr_attr[type].bit;
	if ( restr->not_r == FALSE ) { // if non not-restriction: store value
	  found = TRUE;
	  restriction_or[loc][++restriction_or[loc][0]] = restr->value;
	}
	else if ( ch->curattr[restr_attr[restr->type].bit] == restr->value ) // if not-restriction: just test
	  return FALSE;
      }
      // non-stat restriction
      else if (!check_one_restriction( ch, restr ))
	return FALSE;
    }

  if ( !found )
    return TRUE;

  // Check stored stat-restriction
  for ( int i = 0; i < ATTR_NUMBER; i++ )
    if ( restriction_or[i][0] > 0 ) {
      bool ok = FALSE;
      // stat-restriction
      for ( int j = 0; j < restriction_or[i][0]; j++ )
	if ( ch->curattr[i] == restriction_or[i][1+j] ) { // value equal, 1+ because first column gives #
	  ok = TRUE;
	  break;
	}
      if ( !ok ) // if current char value is not in stored stat-restriction, restriction test failed
	return FALSE;
    }

  return TRUE;
}

// Modified by SinaC 2001
//  return TRUE if the item can be worn, FALSE otherwise
bool old_check_restriction( CHAR_DATA *ch, OBJ_DATA *obj ) {
  RESTR_DATA *restr;

  //  for ( restr = obj->restriction; restr != NULL; restr = restr->next )
  //    if (!check_one_restriction( ch, restr ))
  //      return FALSE;

  //  if (!obj->enchanted)
    for ( restr = obj->pIndexData->restriction; restr != NULL; restr = restr->next )
      if (!check_one_restriction( ch, restr ))
	return FALSE;

  return TRUE;
}

// format restrictions into a buffer
void restrstring( char *buf, RESTR_DATA *restr ) {
  // Added by SinaC 2000 for skill/spell restriction
  //if ( restr->ability_r ) {
  if ( restr->type == RESTR_ABILITY ) {
    if ( restr->not_r == FALSE )
      sprintf( buf, "requires                  at least %4d%%  in      %22s\n\r",
	       restr->value, 
	       ability_table[restr->sn].name );
    else
      sprintf( buf, "requires                 less than %4d%%  in      %22s\n\r",
	       restr->value, 
	       ability_table[restr->sn].name );
  }
  else {
    // Modified by SinaC 2000
    // >=         str, int, wis, dex, con         if not_r not set
    // <                                          if not_r set
    if ( restr_table[restr->type].bits == NULL ) {
      if ( restr->not_r == FALSE )
	sprintf( buf, "requires                  at least %5d  in      %22s\n\r",
		 restr->value, 
		 restr_table[restr->type].name );
      else
	sprintf( buf, "requires                 less than %5d  in      %22s\n\r",
		 restr->value, 
		 restr_table[restr->type].name );
    }
    // =          race, sex, [--size]                if not_r not set
    // !=                                        if not_r set
    else if ( is_stat( restr_table[restr->type].bits ) ) {
      if ( restr->not_r == FALSE )
	sprintf( buf, "requires                  %14s  equals to    %17s\n\r",
		 restr_table[restr->type].name,
		 flag_string( restr_table[restr->type].bits, restr->value ) );
      else
	sprintf( buf, "requires                  %14s  not equals to %16s\n\r",
		 restr_table[restr->type].name,
		 flag_string( restr_table[restr->type].bits, restr->value ) );
    }
    // or         parts, classes, form           if not_r not set
    // nor                                       if not_r set
    else {
      if ( restr->not_r == FALSE )
	sprintf( buf, "requires                  %14s  has      %21s\n\r",
		 restr_table[restr->type].name,
		 flag_string( restr_table[restr->type].bits, restr->value ));
      else
	sprintf( buf, "requires                  %14s  has not  %21s\n\r",
		 restr_table[restr->type].name,
		 flag_string( restr_table[restr->type].bits, restr->value ));
    }
  }
}

// find a restriction
//RESTR_DATA* find_restriction( OBJ_DATA *obj, int restriction )
// Modified by SinaC 2001
RESTR_DATA* find_restriction( RESTR_DATA *restr_start, int restriction ) {
  RESTR_DATA *restr;

  // restr = obj->restriction
  for ( restr = restr_start; restr != NULL; restr = restr->next )
    if ( restr_attr[restr->type].bit == restriction )
      return restr;

  return NULL;
}
