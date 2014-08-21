#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "merc.h"
#include "olc_value.h"
#include "comm.h"
#include "db.h"
#include "config.h"
#include "bit.h"
#include "utils.h"


// Dump killed/killed_by for every mob_index_data
static void mfindkill() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "mob_kill.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("mfindkill: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  int nMatch = 0;
  for ( int vnum = 0; nMatch < top_mob_index; vnum++ ) {
    MOB_INDEX_DATA *pMobIndex;  
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
      continue;
    nMatch++;
    if ( pMobIndex->killed == 0
	 && pMobIndex->killed_by == 0 )
      continue;
    fprintf( fp,
	     "vnum: %6d    killed: %-10d    killed_by: %-10d\n",
	     pMobIndex->vnum, pMobIndex->killed, pMobIndex->killed_by );
  }
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Dump every material and how many item has this material
static void ofindmat() {
  int lastone[MAX_MATERIAL];
  int howmany[MAX_MATERIAL];
  memset(lastone,0,sizeof(int)*MAX_MATERIAL);
  memset(howmany,0,sizeof(int)*MAX_MATERIAL);

  // Count material
  int nMatch = 0;
  for(int vnum=0;nMatch<top_obj_index;vnum++){
    OBJ_INDEX_DATA *pObjIndex;  
    if ( (pObjIndex=get_obj_index(vnum)) !=NULL){
      nMatch++;
      
      howmany[pObjIndex->material]++;
      lastone[pObjIndex->material] = pObjIndex->vnum;
    }
  }
  
  // Write material
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "obj_material.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("ofindmat: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_MATERIAL; i++ )
    fprintf(fp,"Material [%20s]  number: %5d  last: %5d\n",
	    material_table[i].name,howmany[i],lastone[i]);
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Dump every mob material and how many mob has this material
static void mfindmat() {
  const int MAX = 1024;
  int lastone[MAX];
  int howmany[MAX];
  const char *matname[MAX];
  memset(lastone,0,sizeof(int)*MAX);
  memset(howmany,0,sizeof(int)*MAX);

  // Count material
  int count = 0;
  matname[count] = str_dup("<Not Entered>");
  lastone[count] = 0;
  howmany[count] = 0;
  count++;

  int nMatch = 0;
  for( int vnum=0; nMatch < top_mob_index; vnum++ ) {
    MOB_INDEX_DATA *pMobIndex;  
    if ( ( pMobIndex = get_mob_index(vnum) ) != NULL ) {
      nMatch++;
      if ( pMobIndex->material == NULL || pMobIndex->material[0] == '\0' ) {
	howmany[0]++;
	lastone[0] = pMobIndex->vnum;
	continue;
      }
      int found = -1;
      for ( int i = 1; i < count; i++ ) {
	if ( !str_cmp( pMobIndex->material, matname[i] ) ) {
	  found = i;
	  break;
	}
      }
      if ( found == -1 ) {
	matname[count] = str_dup( pMobIndex->material );
	howmany[count] = 1;
	lastone[count] = pMobIndex->vnum;
	count++;
      }
      else {
	howmany[found]++;
	lastone[found] = pMobIndex->vnum;
      }
    }
  }

  // Write material
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "mob_material.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("mfindmat: Can't open file %s", filename);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  for ( int i = 0; i < count; i++ )
    fprintf(fp,"Material [%20s]  number: %5d  last: %5d\n",
	    matname[i], howmany[i], lastone[i]);
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Dump how many item have condition from 0 to 100
static void ofindcond() {
  const int MAX = 101;
  int lastone[MAX];
  int howmany[MAX];
  memset(lastone,0,sizeof(int)*MAX);
  memset(howmany,0,sizeof(int)*MAX);

  // Find condition
  int count = 0;
  int nMatch = 0;
  for( int vnum = 0; nMatch < top_obj_index ; vnum++ ) {
    OBJ_INDEX_DATA *pObjIndex;  
    if ( ( pObjIndex = get_obj_index(vnum)) != NULL ) {
      nMatch++;
      int cond = URANGE( 0, pObjIndex->condition, 100 );
      howmany[cond]++;
      lastone[cond] = pObjIndex->vnum;
    }
  }
  
  // Write condition
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "obj_condition.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("ofindcond: Can't open file %s", filename);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  for ( int i = 0; i < MAX; i++ )
    fprintf(fp,"condition [%3d]  number: %5d  last: %5d\n",
	    i, howmany[i], lastone[i] );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Find every closed door needing a key, and check if this key is loaded
static void rfindkey() {
  fclose( fpReserve );
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "room_missing_key.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("rfindkey: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  int nMatch = 0;
  for( int vnum = 0; nMatch < top_room; vnum++ ) {
    // if the room exists
    ROOM_INDEX_DATA *pRoomIndex;
    if ( ( pRoomIndex = get_room_index(vnum) ) != NULL ) {
      nMatch++; 
      // every doors of the room
      for ( int door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
	EXIT_DATA *pexit;
	
	int keyval = -1;
	if ( ( pexit = pRoomIndex->exit[door] ) ) {
	  keyval = pexit->key;
	  
	  // doesn't need a key
	  if ( keyval == 0 || keyval == -1 )
	    continue;
	  
	  /* Used to fix some key, SinaC 2000
	     if ( keyval == 1 ) 
	     pRoomIndex->exit[door]->key = -1;
	  */
	  
	  // okay a key which doesn't exist
	  OBJ_INDEX_DATA *pObjIndex;
	  if ( !( pObjIndex = get_obj_index( keyval ) ) ) {
	    fprintf(fp,"Not Exist: Room: %d  exit: %d  key: %d\n",
		    pRoomIndex->vnum, door, keyval);
	    continue;
	  }
	  // the key is not a ITEM_KEY
	  if ( pObjIndex->item_type != ITEM_KEY ) {
	    fprintf(fp,"Not a key: Room: %d  exit: %d  key: %d\n",
		    pRoomIndex->vnum, door, keyval);
	    continue;
	  }
	  
	  // now we will check if the key is on the floor, on a mob,...
	  //  so we check every obj on the mud
	  bool found = FALSE;
	  for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj->next ) {
	    // not the key we're searching
	    if ( obj->pIndexData != pObjIndex )
	      continue;
	    
	    // get out of container
	    OBJ_DATA *in_obj;
	    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	      ;
	    
	    if ( ( in_obj->carried_by != NULL && in_obj->carried_by->in_room != NULL) 
		 || ( (in_obj->in_room != NULL ) ) ){
	      found = TRUE;
	      break;
	    }
	  }
	  if ( !found ) {
	    fprintf(fp,"Missing key: Room: %d  exit: %d  key: %d\n",
		    pRoomIndex->vnum, door, keyval );
	    continue;
	  }
	  fprintf(fp,"Found: Room: %d  exit: %d  key: %d\n",
		  pRoomIndex->vnum, door, keyval );
	}
      }
    }
  }
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Search if every items are loaded somewhere in the mud
static int rst_table[34000];
static void ofindreset() {
  for ( int i = 0; i < 34000; i++ ) {
    OBJ_INDEX_DATA *pObj;
    if ( ( pObj = get_obj_index( i ) ) == NULL ) rst_table[i] = -1;
    else                                         rst_table[i] = 0;
  }

  // Search obj in resets
  for ( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    for ( ROOM_INDEX_DATA *pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
      for ( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
	switch( pReset->command ) {
	case 'O':
	case 'E':
	case 'G': 
	case 'P': rst_table[ pReset->arg1 ]++; break;
	}
      }

  // Write obj
  fclose( fpReserve );
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "oreset.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ){
    bug("Can't open file %s", filename);
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < 34000; i++ ) {
    switch ( rst_table[i] ) {
    case -1: break;
    case 0 : fprintf(fp,"%6d never appears.\n", i ); break;
    default: fprintf(fp,"%6d appears %5d times.\n", i, rst_table[i] ); break;
    }
  }  

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Search if every mobiles are loaded somewhere in the mud
static void mfindreset() {
  for ( int i = 0; i < 34000; i++ ) {
    MOB_INDEX_DATA *pMob;
    if ( ( pMob = get_mob_index( i ) ) == NULL ) rst_table[ i ] = -1;
    else rst_table[ i ] = 0;
  }

  // Search mob in resets
  for ( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    for( ROOM_INDEX_DATA *pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
      for( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
	switch( pReset->command ){
	case 'M': rst_table[ pReset->arg1 ]++; break;
	}
      }

  fclose( fpReserve );
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "mreset.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ){
    bug("Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < 34000; i++ ) {
    switch ( rst_table[i] ) {
    case -1: break;
    case 0 : fprintf(fp,"%6d never appears.\n", i ); break;
    default: fprintf(fp,"%6d appears %5d times.\n", i, rst_table[i] ); break;
    }
  }

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

static void mfindrace() {
  int count[MAX_RACE];
  int last[MAX_RACE];
  memset( count, 0, sizeof(int)*MAX_RACE);
  memset( last, 0, sizeof(int)*MAX_RACE);
  int invalid = 0;
  int lastInvalid = -1;

  // Count races
  int nMatch = 0;
  for ( int vnum = 0; nMatch < top_mob_index; vnum++ ) {
    MOB_INDEX_DATA *pMobIndex;  
    if ( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
      continue;
    nMatch++;
    int race = pMobIndex->race;
    if ( race >= 0 && race < MAX_RACE ) {
      count[race]++;
      last[race] = pMobIndex->vnum;
    }
    else {
      invalid++;
      lastInvalid = pMobIndex->vnum;
    }
  }

  // Write races
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "mob_races.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("ofindmat: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_RACE; i++ )
    fprintf(fp,"Race [%20s]  number: %5d  last: %5d\n",
	    race_table[i].name, count[i], last[i] );
  fprintf(fp, "Invalid     number: %5d  last: %5d\n",
	  invalid, lastInvalid );
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

static void ofindkey() {
  fclose( fpReserve );
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "obj_missing_key.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("rfindkey: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  int nMatch = 0;
  for( int vnum = 0; nMatch < top_obj_index ; vnum++ ) {
    OBJ_INDEX_DATA *pObjIndex;  
    if ( ( pObjIndex = get_obj_index(vnum)) != NULL ) {
      nMatch++;
      int key = pObjIndex->value[2];
      if ( pObjIndex->item_type != ITEM_CONTAINER
	   || key == 0 || key == -1 )
	  continue;
      OBJ_INDEX_DATA *pKey = get_obj_index( key );
      if ( pKey == NULL ) {
	fprintf( fp, "Not exist: Container: %d  key: %d\n",
		 pObjIndex->vnum, key );
	continue;
      }
      if ( pKey->item_type != ITEM_KEY ) {
	fprintf( fp, "Not a key: Container: %d  key: %d\n",
		 pObjIndex->vnum, key );
	continue;
      }
      // now we will check if the key is on the floor, on a mob,...
      //  so we check every obj on the mud
      bool found = FALSE;
      for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj->next ) {
	// not the key we're searching
	if ( obj->pIndexData != pKey )
	  continue;
	
	// get out of container
	OBJ_DATA *in_obj;
	for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
	  ;
	
	if ( ( in_obj->carried_by != NULL && in_obj->carried_by->in_room != NULL) 
	     || ( (in_obj->in_room != NULL ) ) ){
	  found = TRUE;
	  break;
	}
      }
      if ( !found ) {
	fprintf(fp,"Missing key:  Container: %d  key: %d\n",
		pObjIndex->vnum, key );
	continue;
      }
      fprintf(fp,"Found:   Container: %d  key: %d\n",
	      pObjIndex->vnum, key );
    }
  }
	  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Find mob loaded out of their area
static void mfindoutarea() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "mob_outarea.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("ofindmat: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  // Search mob in resets
  for ( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    for( ROOM_INDEX_DATA *pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
      for( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
	switch( pReset->command ) {
	case 'M': {
	  MOB_INDEX_DATA *pMob = get_mob_index(pReset->arg1);
	  if ( pMob->area != pRoom->area )
	    fprintf( fp, "Room: %d   Mob: %d\n", pRoom->vnum, pMob->vnum );
	  break;
	}
	}
      }
 
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

// Find objects loaded out of their area
static void ofindoutarea() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "obj_outarea.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("ofindmat: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  // Search obj in resets
  for ( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    for( ROOM_INDEX_DATA *pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next )
      for( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
	switch( pReset->command ) {
	case 'O':
	case 'E':
	case 'G': 
	case 'P': {
	  OBJ_INDEX_DATA *pObj = get_obj_index(pReset->arg1);
	  if ( pObj->area != pRoom->area )
	    fprintf( fp, "Room: %d   Obj: %d\n", pRoom->vnum, pObj->vnum );
	  break;
	}
	}
      }
 
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

static void rfinddoors() {
  fclose( fpReserve );
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "room_doors.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("rfindkey: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  
  int nMatch = 0;
  for( int vnum = 0; nMatch < top_room; vnum++ ) {
    // if the room exists
    ROOM_INDEX_DATA *pRoomIndex;
    if ( ( pRoomIndex = get_room_index(vnum) ) != NULL ) {
      nMatch++; 
      // every doors of the room
      for ( int door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
	EXIT_DATA *pexit;
	
	if ( ( pexit = pRoomIndex->exit[door] ) && pexit->rs_flags != 0 ) {
	  fprintf( fp, "Room [%5d] dir [%10s] key [%d] flags [%s]",
		   pRoomIndex->vnum, dir_name[door], pexit->key, flag_string( exit_flags, pexit->rs_flags ) );
	  ROOM_INDEX_DATA *to_room = pexit->u1.to_room;
	  int rev = rev_dir[door];
	  EXIT_DATA *rev_exit = to_room?to_room->exit[rev]:NULL;
	  if ( to_room == NULL || rev_exit == NULL )
	    fprintf( fp, "  NO REVERSE!!");
	  else {
	    fprintf( fp, "  reverse [%5d] dir [%9s] key [%d] flags [%s]",
		     to_room->vnum, dir_name[rev], rev_exit->key, flag_string( exit_flags, rev_exit->rs_flags ) );
	    if ( rev_exit->rs_flags != pexit->rs_flags || pexit->key != rev_exit->key )
	      fprintf( fp, " DIFFERENT");
	  }
	  fprintf( fp, "\n");
	}
      }
    }
  }
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

int get_item_points( OBJ_INDEX_DATA *pObjIndex, int &problem ) {
  int points = 0;
  problem = 0;
  
  // Count points for specific item type
  switch ( pObjIndex->item_type ) {
  case ITEM_WEAPON: {
    if ( pObjIndex->value[0] == WEAPON_RANGED ) {
      problem = 12;
    }
    else {
      // average damage
      int v1 = pObjIndex->value[1];
      int v2 = pObjIndex->value[2];
      int avg;
      if (pObjIndex->new_format)
	avg = ((1 + v2) * v1) / 2;
      else
	avg = ( v1 + v2 ) / 2;
      points += avg;
      //   Average damage :  Weapons level 1-20 receive a bonus of 5 extra points of
      //    average damage.  Weapons level 21-40 receive 10 points.  41-60 receive 15
      //    points.  61-80 receive 20 points.  81-100 receive 25 points.  101+ receive
      //    30 points.
      if ( pObjIndex->level >= 1 && pObjIndex->level <= 20 )        points -= 5;
      else if ( pObjIndex->level >= 21 && pObjIndex->level <= 40 )  points -= 10;
      else if ( pObjIndex->level >= 41 && pObjIndex->level <= 60 )  points -= 15;
      else if ( pObjIndex->level >= 61 && pObjIndex->level <= 80 )  points -= 20;
      else if ( pObjIndex->level >= 81 && pObjIndex->level <= 100 ) points -= 25;
      else if ( pObjIndex->level >= 101 )                           points -= 30;
      // weapon flags
      int wflags = count_bit( pObjIndex->value[4] );
      points += 8 * wflags;
      if ( IS_SET( pObjIndex->value[4], WEAPON_SHARP )
	   || IS_SET( pObjIndex->value[4], WEAPON_SHARP ) )
	points += 2;
      if ( IS_SET( pObjIndex->value[4], WEAPON_VORPAL ) )
	points += 4;
      if ( IS_SET( pObjIndex->value[4], WEAPON_TWO_HANDS ) )
	points -= 8;
    }
    if ( IS_SET( pObjIndex->extra_flags, ITEM_ANTI_EVIL ) )
      points -= 1;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_ANTI_GOOD ) )
      points -= 1;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_ANTI_NEUTRAL ) )
      points -= 1;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_BURN_PROOF ) )
      points += 3;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_HUM ) )
      points += 5;
    break;
  }
  case ITEM_ARMOR: {
    // average armor
    int avg = ( pObjIndex->value[0] + pObjIndex->value[1] + pObjIndex->value[2] + pObjIndex->value[3] ) / 4; 
    points += avg;
    //   Average armor : This is determined by averaging the four armor values
    //    together.  Armor levels 21-40 recieves an extra 5 points of average armor.
    //    Levels 41-60 receives 10 points.  61-80 = 15 points.  81-100 = 20 points.
    //	Level 101+ receives 25 extra points.
    if ( pObjIndex->level >= 21 && pObjIndex->level <= 40 )       points -= 5;
    else if ( pObjIndex->level >= 41 && pObjIndex->level <= 60 )  points -= 10;
    else if ( pObjIndex->level >= 61 && pObjIndex->level <= 80 )  points -= 15;
    else if ( pObjIndex->level >= 81 && pObjIndex->level <= 100 ) points -= 20;
    else if ( pObjIndex->level >= 101 )                           points -= 25;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_BURN_PROOF ) )
      points += 3;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_HUM ) )
      points += 5;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_MAGIC ) )
      points += 5;
    //armor size giant without extra flag "magic"                + 3
    //armor size tiny without extra flag "magic"                 + 3
    break;
  }
  case ITEM_LIGHT: {
    if ( pObjIndex->value[2] == -1 || pObjIndex->value[2] == 999 )
      points += 10;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_BURN_PROOF ) )
      points += 3;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_HUM ) )
      points += 5;
    break;
  }
  case ITEM_JEWELRY: case ITEM_CLOTHING: {
    if ( IS_SET( pObjIndex->extra_flags, ITEM_BURN_PROOF ) )
      points += 3;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_HUM ) )
      points += 5;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_MAGIC ) )
      points += 5;
    break;
  }
  case ITEM_CONTAINER: case ITEM_DRINK_CON: case ITEM_FOUNTAIN: {
    if ( pObjIndex->affected != NULL )
      problem = 1;
    // such burnproof item cannot be sold and may not have special affect
    break;
  }
  case ITEM_STAFF: case ITEM_WAND: {
    if ( IS_SET( pObjIndex->extra_flags, ITEM_BURN_PROOF ) )
      points += 3;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_HUM ) )
      points += 5;
    if ( IS_SET( pObjIndex->extra_flags, ITEM_MAGIC ) )
      points += 5;
    if ( pObjIndex->value[0] > 5 + pObjIndex->level
	 || pObjIndex->level < 15 )
      problem = 2;
    int charge = pObjIndex->value[2];
    if ( pObjIndex->level <= 50 ) {
      charge -= 3;
      if ( charge >= 3 )
	problem = 3;
    }
    else {
      charge -= 5;
      if ( charge >= 5 )
	problem = 3;
    }
    points += 5*charge; // 5 points for each extra charges
    // cannot be sold
    break;
  }
  case ITEM_PILL: case ITEM_POTION: case ITEM_SCROLL: {
    int count_spell = 0;
    for ( int i = 1; i < 5; i++ )
      if ( pObjIndex->value[i] > 0 )
	count_spell++;
    if ( count_spell == 1 && pObjIndex->level < 20 ) problem = 4;
    else if ( count_spell == 2 && pObjIndex->level < 50 ) problem = 4;
    else if ( count_spell == 3 && pObjIndex->level < 75 ) problem = 4;
    else if ( count_spell == 4 && pObjIndex->level < 95 ) problem = 4;
    // negative spell, that hurts player  doesn't count in spell counting
    // cannot be sold
    break;
  }
  case ITEM_COMPONENT: {
    // cannot be sold
    break;
  }
  case ITEM_TEMPLATE: {
    int count_spell = 0;
    for ( int i = 1; i < 5; i++ )
      if ( pObjIndex->value[i] > 0 )
	count_spell++;
    if ( count_spell > 0 )
      problem = 13;
    break;
  }
  }

  // Count points with Affects
  bool vuln = FALSE;
  for ( AFFECT_DATA *paf = pObjIndex->affected; paf != NULL; paf = paf->next )
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
      if ( laf->location == ATTR_vuln_flags )
	vuln = TRUE;
      if ( laf->location == ATTR_imm_flags || laf->location == ATTR_res_flags )
	problem = 5;
      if ( laf->location == ATTR_max_hit
	   || laf->location == ATTR_max_mana
	   || laf->location == ATTR_max_psp )
	points += laf->modifier/2;
      if ( laf->location == ATTR_max_move ) {
	points += laf->modifier/4;
	if ( 10 * laf->modifier > 12 * pObjIndex->level )
	  problem = 6;
      }
      if ( laf->location >= ATTR_STR && laf->location <= ATTR_CON ) {
	points += laf->modifier*4;
	if ( 20 * laf->modifier > pObjIndex->level )
	  problem = 7;
      }
      if ( laf->location == ATTR_saving_throw )
	points -= laf->modifier*3;
      if ( laf->location == ATTR_hitroll ) {
	points += laf->modifier*3;
	if ( 10 * laf->modifier > pObjIndex->level )
	  problem = 8;
      }
      if ( laf->location == ATTR_damroll ) {
	points += laf->modifier;
	if ( 10 * laf->modifier > pObjIndex->level )
	  problem = 9;
      }
      if ( ( laf->location >= ATTR_ac0 && laf->location <= ATTR_ac3 )
	   || ( laf->location == ATTR_allAC ) )
	points -= laf->modifier;
      if ( laf->location == ATTR_affected_by ) {
	if ( IS_SET( laf->modifier, AFF_SLEEP )
	     || IS_SET( laf->modifier, AFF_SILENCE )
	     || IS_SET( laf->modifier, AFF_ROOTED ) ) points -= 15;
	if ( IS_SET( laf->modifier, AFF_BLIND )
	     || IS_SET( laf->modifier, AFF_CHARM )
	     || IS_SET( laf->modifier, AFF_PLAGUE )
	     || IS_SET( laf->modifier, AFF_SLOW ) ) points -= 10;
	if ( IS_SET( laf->modifier, AFF_CURSE ) ) points -= 5;
	if ( IS_SET( laf->modifier, AFF_CALM ) ) points += 0;
	if ( IS_SET( laf->modifier, AFF_DETECT_EVIL )
	     || IS_SET( laf->modifier, AFF_DETECT_GOOD )
	     || IS_SET( laf->modifier, AFF_DETECT_MAGIC )
	     || IS_SET( laf->modifier, AFF_INFRARED ) ) points += 10;
	if ( IS_SET( laf->modifier, AFF_DETECT_INVIS )
	     || IS_SET( laf->modifier, AFF_DETECT_HIDDEN )
	     || IS_SET( laf->modifier, AFF_DARK_VISION ) ) points += 15;
	if ( IS_SET( laf->modifier, AFF_PROTECT_EVIL )
	     || IS_SET( laf->modifier, AFF_PROTECT_GOOD ) ) points += 20;
	if ( IS_SET( laf->modifier, AFF_FLYING )
	     || IS_SET( laf->modifier, AFF_PASS_DOOR )
	     || IS_SET( laf->modifier, AFF_INVISIBLE )
	     || IS_SET( laf->modifier, AFF_SNEAK )
	     || IS_SET( laf->modifier, AFF_HIDE )
	     || IS_SET( laf->modifier, AFF_SWIM ) ) points += 25;
	if ( IS_SET( laf->modifier, AFF_HASTE ) ) points += 30;
	if ( IS_SET( laf->modifier, AFF_REGENERATION ) ) points += 40;

	if ( IS_SET( laf->modifier, AFF_SANCTUARY )
	     || IS_SET( laf->modifier, AFF_FAERIE_FIRE )
	     || IS_SET( laf->modifier, AFF_POISON )
	     || IS_SET( laf->modifier, AFF_WEAKEN )
	     || IS_SET( laf->modifier, AFF_BERSERK ) ) problem = 10;
      }
      if ( laf->location == ATTR_affected2_by ) {
	if ( IS_SET( laf->modifier, AFF2_NOSPELL ) ) points -= 15;
	if ( IS_SET( laf->modifier, AFF2_CONFUSION )
	     || IS_SET( laf->modifier, AFF2_FAERIE_FOG ) ) points -= 10;
	if ( IS_SET( laf->modifier, AFF2_DETECT_EXITS ) ) points += 20;
	if ( IS_SET( laf->modifier, AFF2_WATER_BREATH )
	     || IS_SET( laf->modifier, AFF2_WALK_ON_WATER ) ) points += 25;	    
	    
	if ( IS_SET( laf->modifier, AFF2_MAGIC_MIRROR )
	     || IS_SET( laf->modifier, AFF2_NOEQUIPMENT )
	     || IS_SET( laf->modifier, AFF2_INCREASED_CASTING )
	     || IS_SET( laf->modifier, AFF2_HIGHER_MAGIC_ATTRIBUTES )
	     || IS_SET( laf->modifier, AFF2_FREE_MOVEMENT ) 
	     || IS_SET( laf->modifier, AFF2_NECROTISM ) ) problem = 11;
      }
    }
  // Count points with Immunities/Resistances, Vulnerabilities
  if ( vuln )
    points -= 10;

  // Count points with Restrictions
  int count_race_restr = 0, count_class_restr = 0, count_sex_restr = 0,
    count_ability_restr = 0, count_other_restr = 0, count_attr_restr = 0;
  for ( RESTR_DATA *restr = pObjIndex->restriction; restr != NULL; restr = restr->next ) {
    //if ( restr->ability_r || restr->type == RESTR_ABILITY ) count_ability_restr++;
    if ( restr->type == RESTR_ABILITY ) count_ability_restr++;
    else if ( restr->type >= RESTR_STR && restr->type <= RESTR_CON ) count_attr_restr++;
    else if ( restr->type == RESTR_CLASSES ) count_class_restr++;
    else if ( restr->type == RESTR_RACE ) count_race_restr++;
    else if ( restr->type == RESTR_SEX ) count_sex_restr++;
    else count_other_restr++; // part/form/etho/align
  }
  if ( count_race_restr >= 3 && count_race_restr <= 5 ) points -= 1;
  else if ( count_race_restr >= 6 && count_race_restr <= 8 ) points -= 2;
  else if ( count_race_restr >= 9 && count_race_restr <= 11 ) points -= 3;
  else if ( count_race_restr >= 12 && count_race_restr <= 14 ) points -= 4;
  else if ( count_race_restr >= 15 && count_race_restr <= 39 ) points -= 5;
  else if ( count_race_restr >= 40 ) points -= 8;
  if ( count_class_restr == 1 ) points -= 1;
  else if ( count_class_restr == 2 ) points -= 2;
  else if ( count_class_restr == 3 ) points -= 3;
  else if ( count_class_restr == 4 ) points -= 4;
  else if ( count_class_restr >= 5 && count_class_restr <= 10 ) points -= 5;
  else if ( count_class_restr >= 11 ) points -= 8;
  points -= count_sex_restr * 3;
  points -= count_ability_restr * 1; // FIXME, should find better values
  points -= count_other_restr * 1;
  points -= count_attr_restr * 1;

  return points;
}

const char *problem_list[] = {
  "<no problem>", // 0
  "<container with affects>", // 1
  "<wands with too high spell level>", // 2
  "<wands with too many charges>", // 3
  "<staves with too many spells>", // 4
  "<immunities or resistances>", // 5
  "<too many moves>", // 6
  "<too many stats>", // 7
  "<too many hitroll>", // 8
  "<too many damroll>", // 9
  "<invalid affects>", // 10
  "<invalid affects2>", // 11
  "<RANGED weapon>", // 12
  "<template with spells>",//13
  "<UNKNOWN>"
};

static void ofindbalanced() {
  fclose( fpReserve );
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", STAT_DIR, "balanced_items.txt" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("ofindbalanced: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  int nMatch = 0;
  for( int vnum = 0; nMatch < top_obj_index ; vnum++ ) {
    OBJ_INDEX_DATA *pObjIndex;  
    if ( ( pObjIndex = get_obj_index(vnum)) != NULL ) {
      nMatch++;
      int problem;
      int points = get_item_points( pObjIndex, problem );

      fprintf( fp, "Item [%5d] level: (%3d)  points: <%3d>%s%s%s\n",
	       pObjIndex->vnum, pObjIndex->level, points,
	       problem>0?" PROBLEM":"", problem?problem_list[problem]:"",
	       pObjIndex->level != points?" DIFFERENCE":"");
    }
  }
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void do_writestat( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  send_to_char("Writing mud statistics...\n\r", ch );

  mfindkill();     send_to_char("Mobiles killed/killed_by done.\n\r", ch );
  ofindmat();      send_to_char("Objects material done.\n\r", ch );
  mfindmat();      send_to_char("Mobiles material done.\n\r", ch );
  ofindcond();     send_to_char("Objects condition done.\n\r", ch );
  rfindkey();      send_to_char("Room wrong keys done.\n\r", ch );
  ofindreset();    send_to_char("Objects resets done.\n\r", ch );
  mfindreset();    send_to_char("Mobiles resets done.\n\r", ch );
  mfindrace();     send_to_char("Mobiles races done.\n\r", ch );
  ofindkey();      send_to_char("Objects wrong keys done.\n\r", ch );
  mfindoutarea();  send_to_char("Mobiles out of area done.\n\r", ch );
  ofindoutarea();  send_to_char("Objects out of area done.\n\r", ch );
  rfinddoors();    send_to_char("Doors closed/locked/... done.\n\r", ch );
  ofindbalanced(); send_to_char("Balanced items done.\n\r", ch );
}
