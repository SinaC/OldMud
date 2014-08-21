/* 
 *
 * File added by SinaC 2000
 *  for object ability upgrade
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

// Added by SinaC 2001
#include "abilityupgrade.h"
#include "handler.h"
#include "olc_value.h"

// SinaC 2003
// returns the %age added by items to ability[sn]
// complexity O(n²)
int get_ability_upgrade( CHAR_DATA *ch, const int sn ) {
  if ( sn < 0 || sn > MAX_ABILITY )
    return 0;

  int value = 0;
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj == NULL || !obj->valid // item not valid
	 || obj->wear_loc == -1  ) // item not worn
      continue;
    for ( ABILITY_UPGRADE *upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next )
      if ( upgr->sn == sn )
	value += upgr->value;
  }
  
  return value;
}

// Added by SinaC 2000
// returns the %age added by items to ability[sn]
// complexity O(n³)
int old_get_ability_upgrade( CHAR_DATA *ch, int sn ) {
  int value;

  if ( sn < 0 || sn > MAX_ABILITY )
    return 0;

  value = 0;
  for (int loc = 0; loc < MAX_WEAR; loc++) {
    OBJ_DATA *obj;
    ABILITY_UPGRADE *upgr;

    obj = get_eq_char(ch,loc);
    if (obj == NULL || !obj->valid )
      continue;

    //    for ( upgr = obj->upgrade; upgr != NULL; upgr = upgr->next )
    //      if ( upgr->sn == sn )
    //	value += upgr->value;

    // Added by SinaC 2003
    //    if (!obj->enchanted)
      for ( upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next )
	if ( upgr->sn == sn )
	  value += upgr->value;
  }
  
  return value;
}

void abilityupgradestring( char *buf, ABILITY_UPGRADE *upgr ) {
  sprintf( buf, "adds                               %4d%%  in      %22s\n\r",
	   upgr->value, 
	   ability_table[upgr->sn].name );
}
