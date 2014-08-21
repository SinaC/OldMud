#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "db.h"

int race_in_superrace( const int superrace, const int race ) {
  if ( race < 0 ) {
    bug("race_in_superrace: invalid race [%d]", race );
    return -1;
  }
  if ( superrace < 0 ) {
    bug("race_in_superrace: invalid super race [%d]", superrace );
    return -1;
  }
  for ( int i = 0; i < super_pc_race_table[superrace].nb_pc_race; i++ )
    if ( race == super_pc_race_table[superrace].pc_race_list[i] )
      return i;
  return -1;
}

void remove_from_superrace( const int race ) {
  if ( race < 0 ) {
    bug("remove_from_superrace: invalid race [%d]", race );
    return;
  }
  pc_race_type *r = &(pc_race_table[race]);
  if ( r->super_race < 0 )
    return;
  super_pc_race_type *sr = &(super_pc_race_table[r->super_race]);
  int pos = race_in_superrace( r->super_race, race );
  if ( pos < 0 ) {
    bug("remove_from_superrace: race [%s] has superrace set but can't be found in super pc race table [%s]",
	r->name, sr->name );
    return;
  }
  // Remove from super race
  // Allocate new vector
  int *tmp;
  if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(sr->nb_pc_race-1)) ) == NULL ) {
    bug("Can't allocate memory for super_pc_race_table");
    return;
  }
  // Copy informations
  int idx = 0;
  for ( int i = 0; i < sr->nb_pc_race; i++ ) {
    //    log_stringf("remove_from_superrace: [%d]  [%d]  [%d]", i, idx, pos );
    if ( i != pos )
      tmp[idx++] = sr->pc_race_list[i];
  }
  // Re-Set pointer
  sr->pc_race_list = tmp;
  sr->nb_pc_race--;
  r->super_race = -1;
}

void add_to_superrace( const int superrace, const int race ) {
  if ( race < 0 ) {
    bug("add_to_superrace: invalid race [%d]", race );
    return;
  }
  if ( superrace < 0 ) {
    bug("add_to_superrace: invalid superrace [%d]", superrace );
    return;
  }
  super_pc_race_type *sr = &(super_pc_race_table[superrace]);
  pc_race_type *r = &(pc_race_table[race]);
  if ( r->super_race >= 0 ) {
    bug("add_to_superrace: race [%s] is already in super race [%s]",
	r->name, super_pc_race_table[r->super_race].name );
    return;
  }
  // Add in super race
  // Allocate new vector
  int *tmp;
  if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(sr->nb_pc_race+1)) ) == NULL ) {
    bug("Can't allocate memory for super_pc_race_table");
    return;
  }
  // Copy informations
  for ( int i = 0; i < sr->nb_pc_race; i++ )
    tmp[i] = sr->pc_race_list[i];
  tmp[sr->nb_pc_race] = race; // add new information
  // Re-Set pointer
  sr->pc_race_list = tmp;
  sr->nb_pc_race++;
  r->super_race = superrace;
}
