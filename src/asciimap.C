/********************************************************************\
*   History: Developed by: mlkesl@stthomas.edu			     *
*                     and: mlk                                       *
*   MapArea: when given a room, ch, x, and y                         *
*            this function will fill in values of map as it should   *
*   ShowMap: will simply spit out the contents of map array          *
*	    Would look much nicer if you built your own areas        *
*	    without all of the overlapping stock Rom has             *
*   do_map: core function, takes map size as argument                *
*   update: map is now a 2 dimensional array of integers             *
*	    uses SECT_MAX for null                                   *
*	    uses SECT_MAX+1 for mazes or one ways to SECT_ENTER	     *
*	    use the SECTOR numbers to represent sector types :)	     *
*                                                                    *
\********************************************************************/
 
/**********************************************************\
* WELCOME TO THE WORLD OF CIRCLEMUD ASCII MAP!!!           *
*  This was originall for Rom(who would use that?) and is  *
*  now available for CircleMUD. This was done on bpl17 but *
*  the only major conversions were room structures so it   *
*  should work on any Circle with fairly up to date rooms. *
*                                                          *
*  If you use this drop me a line maybe, and if your       *
*  generous put me in a line in your credits. Doesn't      *
*  matter though, ENJOY! Feel free to fix any bugs here    *
*  make sure you mail the CircleMUD discussion list to let *
*  everyone know!                                          *
*                                                          *
*  Edward Felch, efelch@hotmail.com  4/25/2001             *
\**********************************************************/

/************************************************************************************\ 
  Notes:
  - You will need more cool sector types!
  - In act.informative.c put in checks for if a room is flagged
    wilderness, if it is: do_map(ch, " "); instead of the normal
    room name and description sending.
  - #define SECT_MAX 22 should be the max number of sector types + 1
    when I tried to include oasis.h for this it gave me errors so I
    just used the number 22.
  - In utils.h I added this in:
    #define URANGE(a, b, c)          ((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
  - Edit XXX.zon so its top room is real high or something and use
     that as a masssively large world map type file
  - We have a ROOM_NOVIEW flag as well as a ROOM_NOENTER flag (used in act.movement.c)
\************************************************************************************/

//#include "conf.h"
//#include "sysdep.h"
//#include "structs.h"
//#include "utils.h"
//#include "comm.h"
//#include "interpreter.h"
//#include "handler.h"
//#include "db.h"
//#include "spells.h"
//#include "constants.h"
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
#include "olc_value.h"
#include "interp.h"
#include "handler.h"
#include "comm.h"

//#define MAX_MAP 80 
#define MAX_MAP (24)
#define MIN_MAP (12)
#define MAX_MAP_DIR 4

int map[MAX_MAP][MAX_MAP];
int offsets[4][2] ={ {-1, 0},{ 0, 1},{ 1, 0},{ 0, -1} };

// Heavily modified - Edward
void MapArea(ROOM_INDEX_DATA *room, CHAR_DATA *ch, const int x, const int y, const int min, const int max) {
  // marks the room as visited
  map[x][y] = room->cstat(sector);

  for ( int door = 0; door < MAX_MAP_DIR; door++ ) {
    EXIT_DATA *pexit;
    if ( ( pexit = room->exit[door]) != NULL  &&   
	 ( pexit->u1.to_room != NULL ) &&
	 ( !IS_SET(pexit->exit_info, EX_CLOSED)) ) {
      if ( (x < min) || ( y < min) || ( x > max ) || ( y >max) )
	return;
      ROOM_INDEX_DATA *prospect_room = pexit->u1.to_room;
      
      // one way into area OR maze
      // if not two way
      if ( prospect_room->exit[rev_dir[door]] &&
	   prospect_room->exit[rev_dir[door]]->u1.to_room != room ) { 
	map[x][y] = SECT_MAX + 1;
	return;
      }
      // end two way
      // players cant see past these
      if ( !IS_IMMORTAL(ch) )
	map[x+offsets[door][0]][y+offsets[door][1]] = prospect_room->bstat(sector);
      // ^--two way into area
      
      if ( map[x+offsets[door][0]][y+offsets[door][1]] == SECT_MAX )
	MapArea(pexit->u1.to_room,ch,x + offsets[door][0], y + offsets[door][1], min, max);
    } // end if exit there
  }
  return;
}

void ShowOneRoom( CHAR_DATA *ch, const int x, const int y, const int min, const int max ) {
  //if ( ( y== min ) || ( map[x][y-1] != map[x][y] ) )
    switch( map[x][y] ) {
    case SECT_MAX:  	    send_to_char(" ",ch); break;
    case SECT_INSIDE:	    send_to_char("{W%",ch); break;
    case SECT_CITY:	    send_to_char("{W#",ch); break;
    case SECT_FIELD:        send_to_char("{G\"",ch); break;
    case SECT_FOREST:       send_to_char("{g@",ch); break;
    case SECT_HILLS:	    send_to_char("{G^",ch); break;
    case SECT_MOUNTAIN:	    send_to_char("{y^",ch); break;
    case SECT_WATER_SWIM:   send_to_char("{B~",ch); break;
    case SECT_WATER_NOSWIM: send_to_char("{b~",ch); break;
    case SECT_BURNING:	    send_to_char("{R\"",ch); break;
    case SECT_AIR:          send_to_char("{c.", ch); break;
    case SECT_DESERT:       send_to_char("{Y.",ch); break;
    case SECT_UNDERWATER:   send_to_char("{c:",ch); break;
    case (SECT_MAX+1):	    send_to_char("{r?",ch); break;
    default: 		    send_to_char("{R*",ch); break; // gh->in_room
    }
//--//  else
//--//    switch( map[x][y] ) {
//--//    case SECT_MAX:	    send_to_char(" ",ch); break;
//--//    case SECT_INSIDE:	    send_to_char("%",ch); break;
//--//    case SECT_CITY:	    send_to_char("#",ch); break;
//--//    case SECT_FIELD:        send_to_char("\"",ch); break;
//--//    case SECT_FOREST:       send_to_char("@",ch); break;
//--//    case SECT_HILLS:	    send_to_char("^",ch); break;
//--//    case SECT_MOUNTAIN:	    send_to_char("^",ch); break;
//--//    case SECT_WATER_SWIM:   send_to_char("~",ch); break;
//--//    case SECT_WATER_NOSWIM: send_to_char("~",ch); break;
//--//    case SECT_BURNING:	    send_to_char("\"",ch); break;
//--//    case SECT_AIR:          send_to_char(".", ch); break;
//--//    case SECT_DESERT:       send_to_char(".",ch); break;
//--//    case SECT_UNDERWATER:   send_to_char(":",ch); break;
//--//    case (SECT_MAX+1):	    send_to_char("?",ch); break;
//--//    default: 		    send_to_char("*",ch); break; // gh->in_room
//--//    }
}

// mlk :: shows a map, specified by size
void ShowMap( CHAR_DATA* ch, const int min, const int max) {
  // every row
  for ( int x = min; x < max; ++x ) {
    // every column
    for ( int y = min; y < max; ++y )
      ShowOneRoom( ch, x, y, min, max );
    send_to_char("\n\r",ch); 
  }
  return;
}

// This is the main map function, do_map(ch, " ") is good to use
// do_map(ch "number") is for immortals to see the world map

// Edward: If you play with some of the values here you can make the normal
// map people see larger or smaller. size = URANGE(9, size, MAX_MAP), the 9
// is the map size shown by default. Also look for: ShowMap (ch, min, max+1);
// and change the size of min and max and see what you like.
void make_big_map( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  argument = one_argument(argument, arg1);
  int size = atoi(arg1);
  size = URANGE( MIN_MAP, size, MAX_MAP );

  int center = MAX_MAP/2;

  int min = MAX_MAP/2 - size/2;
  int max = MAX_MAP/2 + size/2;
  // MAX_MAP
  for ( int x = 0; x < MAX_MAP; ++x)
    for (int y = 0; y < MAX_MAP; ++y)
      map[x][y]=SECT_MAX;

  // starts the mapping with the center room
  MapArea(ch->in_room, ch, center, center, min-1, max-1); 
  // marks the center, where ch is
  map[center][center] = SECT_MAX+2;  

  //ShowMap(ch, min, max+1);
  ShowMap(ch, min, max);
  return;
}

void do_bigmap( CHAR_DATA *ch, const char *argument ) {
  make_big_map(ch, "100" );
}

