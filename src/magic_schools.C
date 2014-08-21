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

// MAGIC SCHOOLS
int ability_in_school( const int school, const int sn ) {
  if ( sn < 0 ) {
    bug("ability_in_school: invalid sn [%d]", sn );
    return -1;
  }
  if ( school < 0 ) {
    bug("ability_in_school: invalid school [%d]", school );
    return -1;
  }
  for ( int i = 0; i < magic_school_table[school].nb_spells; i++ )
    if ( sn == magic_school_table[school].spells_list[i] )
      return i;
  return -1;
}

void remove_from_school( const int sn ) {
  if ( sn < 0 ) {
    bug("Remove_from_school: invalid sn [%d]", sn );
    return;
  }
  ability_type *sk = &(ability_table[sn]);
  if ( sk->school < 0 )
    return;
  magic_school_type *sc = &(magic_school_table[sk->school]);
  int pos = ability_in_school( sk->school, sn );
  if ( pos < 0 ) {
    bug("Remove_from_school: ability [%s] has school set but can't be found in school table [%s]",
	sk->name, sc->name );
    return;
  }
  // Remove from school
  // Allocate new vector
  int *tmp;
  if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(sc->nb_spells-1)) ) == NULL ) {
    bug("Can't allocate memory for school_table");
    return;
  }
  // Copy informations
  int idx = 0;
  for ( int i = 0; i < sc->nb_spells; i++ ) {
    //    log_stringf("remove_from_school: [%d]  [%d]  [%d]", i, idx, pos );
    if ( i != pos ) {
      tmp[idx++] = sc->spells_list[i];
    }
  }
  // Re-Set pointer
  sc->spells_list = tmp;
  sc->nb_spells--;
  sk->school = -1;
}

void add_to_school( const int school, const int sn ) {
  if ( sn < 0 ) {
    bug("add_to_school: invalid sn [%d]", sn );
    return;
  }
  if ( school < 0 ) {
    bug("add_to_school: invalid school [%d]", school );
    return;
  }
  magic_school_type *sc = &(magic_school_table[school]);
  ability_type *sk = &(ability_table[sn]);
  if ( sk->school >= 0 ) {
    bug("add_to_school: ability [%s] is already in a school [%s]",
	sk->name, magic_school_table[sk->school].name );
    return;
  }
  // Add in school
  // Allocate new vector
  int *tmp;
  if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(sc->nb_spells+1)) ) == NULL ) {
    bug("Can't allocate memory for school_table");
    return;
  }
  // Copy informations
  for ( int i = 0; i < sc->nb_spells; i++ )
      tmp[i] = sc->spells_list[i];
  tmp[sc->nb_spells] = sn; // add new information
  // Re-Set pointer
  sc->spells_list = tmp;
  sc->nb_spells++;
  sk->school = school;
}
