#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "merc.h" 
#include "db.h"
#include "moons.h"

/*
 * This is a pseudo moon simulator. It's totally 
 * false astronomically. But so is the rest of the mud...
 * Who cares?
 * Anyway, this implements moon phases and pseudo-rotation.
 *
 * Below the table indicates the data of the moons.
 * The two leftmost numbers indicates the phase data.
 * The second is the duration, in hours, of a full cycle.
 * The first is the start hour in this cycle. This is important
 * for prediction of moon conjunctions.
 *
 * The three next are the same but for position in the sky.
 * The middle number is the hour of moon rise, in the moon rotation
 * cycle.
 *
 * Example : 12*29, 24*29,	9, 10, 23,	"white"
 * 
 * That means that a cycle is 29 days. The mud starts with moon full.
 * The moon day is 23 hours, so a little shift per mud day.
 * The moon is visible 12/23th of the time, and will be rising one hour
 * after the mud has started.
 */


struct moon_data moon_info[NBR_MOON] =
/* phase : start(ph_t), period(ph_p),
   position : start(po_t), visible from(po_v), period(po_p)  */

{
  { 12*29/*0*/, 24*29,	        9, 10,	23,	"blue"},
  { 12*13/*1*/, 24*13/*13*/,    0,  7,	14,	"red"}

};


// Full moon is 5th phase
#define FULL_MOON (4)

const char * moon_phase_msg[NBR_PHASE] = {
  "",
  "You see a crescent shaped growing %s moon.\n\r",
  "You see the first quarter the %s moon.\n\r",
  "The %s moon is waning.\n\r",
  "The %s moon is full and lightens the whole place.\n\r",
  "The %s moon is waning.\n\r",
  "You see the last quarter of the %s moon.\n\r",
  "The last crescent of the %s moon appears in the sky.\n\r"
};


int moon_lookup( const char *name ) {
  for ( int i = 0; i < NBR_MOON; i++ )
    if ( !str_cmp( moon_info[i].name, name ) )
      return i;
  return -1;
}

int moon_phase( const int i ) {
  return ( moon_info[i].ph_t * NBR_PHASE ) / moon_info[i].ph_p;
}

bool moon_night() {
  return time_info.hour < 5 || time_info.hour >= 20;
}

bool moon_insky( const int i ) {
  return moon_info[i].po_t >= moon_info[i].po_v;
}

bool moon_visible( const int n ) {
  return 
    moon_night()
    && moon_phase(n)
    && moon_insky(n);
}

bool moon_full( const int n ) {
  return moon_phase( n ) == FULL_MOON;
}

void update_moons(char * buf) {
  /*
   * Moons changes.
   */
     
  for ( int i = 0; i < NBR_MOON; i++ ) {
    bool wasvis = moon_visible(i);
    // update position and phase
    if ( ++moon_info[i].ph_t >= moon_info[i].ph_p )
      moon_info[i].ph_t = 0;    
    if ( ++moon_info[i].po_t >= moon_info[i].po_p )
      moon_info[i].po_t = 0;
    
    // if night and moon visibility has changed
    if ( moon_visible(i) != wasvis && moon_night() ) {
      if ( time_info.hour == 20 )
        sprintf( buf+strlen(buf), "The %s moon is fading through the night.\n\r", moon_info[i].name );
      else if ( moon_info[i].po_t == moon_info[i].po_v )
        sprintf( buf+strlen(buf), "The %s moon rises.\n\r", moon_info[i].name );
      else if ( moon_info[i].po_t == 0 )
        sprintf( buf+strlen(buf), "The %s moon sets.\n\r", moon_info[i].name );
      else if ( moon_phase(i) == 1 )
        sprintf( buf+strlen(buf), "The %s moon shows up a thin crescent.\n\r", moon_info[i].name );
      else if ( moon_phase(i) == 0 )
        sprintf( buf+strlen(buf), "The remaining crescent of the %s moon has disappeared.\n\r", moon_info[i].name);
     
    }
  }

  // DEBUG
  //  for ( int i = 0; i < NBR_MOON; i++ )
  //    if ( moon_full(i)
  //	 && moon_visible(i) )
  //      printf("MOON: full %s %2d %2d %2d %2d\n\r",
  //	     moon_info[i].name, moon_info[i].po_t, moon_info[i].ph_t, moon_insky(i), moon_phase(i) );
  //  for ( int i = 0; i < NBR_MOON; i++ )
  //    for ( int j = i+1; j < NBR_MOON; j++ )
  //      if ( moon_full(i) && moon_full(j)
  //	   && moon_visible(i) && moon_visible(j) )
  //	printf("MOON: Conjunction on full moon!!\n\r");
}

