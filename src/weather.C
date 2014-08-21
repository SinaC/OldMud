#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "weather.h"
#include "moons.h"
#include "olc_value.h"
#include "db.h"
#include "utils.h"
#include "comm.h"


const char * sky_look[] = {
  "cloudless",
  "cloudy",
  "rainy",
  "lit by flashes of lightning"
};

const char *	const	day_name	[NBR_DAYS_IN_WEEK] = {
  "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
  "the Great Gods", "the Sun"
};

const char *	const	month_name	[NBR_MONTHS_IN_YEAR] = {
  "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
  "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
  "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
  "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

/*
 * Update the weather.
 */
void weather_update( void ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  int diff;

  buf[0] = '\0';

  switch ( ++time_info.hour ) {
  case  5:
    weather_info.sunlight = SUN_LIGHT;
    strcat( buf, "The day has begun.\n\r" );
    break;
    
  case  6:
    weather_info.sunlight = SUN_RISE;
    strcat( buf, "The sun rises in the east.\n\r" );
    break;
    
  case 19:
    weather_info.sunlight = SUN_SET;
    strcat( buf, "The sun slowly disappears in the west.\n\r" );
    break;
    
  case 20:
    weather_info.sunlight = SUN_DARK;
    strcat( buf, "The night has begun.\n\r" );
    break;
    
  case 24:
    time_info.hour = 0;
    time_info.day++;
    break;
  }
  
  //  if ( time_info.day   >= 35 ) {
  if ( time_info.day >= NBR_DAYS_IN_MONTH ) {
    time_info.day = 0;
    time_info.month++;
  }
  
  //if ( time_info.month >= 17 ) {
  if ( time_info.month >= NBR_MONTHS_IN_YEAR ) {
    time_info.month = 0;
    time_info.year++;
  }
  
  // Oxtal    
  update_moons(buf);    
  
  /*
   * Weather change.
   */
  if ( time_info.month >= 9 && time_info.month <= 16 ) // summer
    diff = weather_info.mmhg >  985 ? -2 : 2;
  else
    diff = weather_info.mmhg > 1015 ? -2 : 2; // winrer

  weather_info.change   += diff * dice(1, 4) + dice(2, 6) - dice(2, 6);
  weather_info.change    = UMAX(weather_info.change, -12);
  weather_info.change    = UMIN(weather_info.change,  12);

  weather_info.mmhg += weather_info.change;
  weather_info.mmhg  = UMAX(weather_info.mmhg,  960);
  weather_info.mmhg  = UMIN(weather_info.mmhg, 1040);

  switch ( weather_info.sky ){
  default: 
    bug( "Weather_update: bad sky %d.", weather_info.sky );
    weather_info.sky = SKY_CLOUDLESS;
    break;
    
  case SKY_CLOUDLESS:
    if ( weather_info.mmhg <  990
	 || ( weather_info.mmhg < 1010 && number_bits( 2 ) == 0 ) ){
      strcat( buf, "The sky is getting cloudy.\n\r" );
      weather_info.sky = SKY_CLOUDY;
    }
    break;
    
  case SKY_CLOUDY:
    if ( weather_info.mmhg <  970
	 || ( weather_info.mmhg <  990 && number_bits( 2 ) == 0 ) ){
      strcat( buf, "It starts to rain.\n\r" );
      weather_info.sky = SKY_RAINING;
    }
    
    if ( weather_info.mmhg > 1030 && number_bits( 2 ) == 0 ){
      strcat( buf, "The clouds disappear.\n\r" );
      weather_info.sky = SKY_CLOUDLESS;
    }
    break;
    
  case SKY_RAINING:
    if ( weather_info.mmhg <  970 && number_bits( 2 ) == 0 ){
      strcat( buf, "Lightning flashes in the sky.\n\r" );
      weather_info.sky = SKY_LIGHTNING;
    }
    
    if ( weather_info.mmhg > 1030
	 || ( weather_info.mmhg > 1010 && number_bits( 2 ) == 0 ) ){
      strcat( buf, "The rain stopped.\n\r" );
      weather_info.sky = SKY_CLOUDY;
    }
    break;
    
  case SKY_LIGHTNING:
    if ( weather_info.mmhg > 1010
	 || ( weather_info.mmhg >  990 && number_bits( 2 ) == 0 ) ){
      strcat( buf, "The lightning has stopped.\n\r" );
      weather_info.sky = SKY_RAINING;
      break;
    }
    break;
  }
  
  if ( buf[0] != '\0' ){
    for ( d = descriptor_list; d != NULL; d = d->next ){
      if ( d->connected == CON_PLAYING
	   &&   IS_OUTSIDE(d->character)
	   && d->character->in_room->cstat(sector) != SECT_INSIDE // SinaC 2003
	   && d->character->in_room->cstat(sector) != SECT_UNDERWATER // SinaC 2003
	   &&   IS_AWAKE(d->character) )
	send_to_char( buf, d->character );
    }
  }
  return;
}
