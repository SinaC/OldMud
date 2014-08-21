#ifndef __WEATHER_H__
#define __WEATHER_H__

// Time and weather stuff.
#define SUN_DARK		    0
#define SUN_RISE		    1
#define SUN_LIGHT		    2
#define SUN_SET			    3

#define SKY_CLOUDLESS		    0
#define SKY_CLOUDY		    1
#define SKY_RAINING		    2
#define SKY_LIGHTNING		    3

// number of month in a year
#define NBR_MONTHS_IN_YEAR        (17)
#define NBR_DAYS_IN_WEEK           (7)
#define NBR_DAYS_IN_MONTH         (35)

// Name of weekdays
extern const char *	const	day_name[NBR_DAYS_IN_WEEK];
// Name of months
extern const char *	const	month_name[NBR_MONTHS_IN_YEAR];
// Sky name
extern const char * sky_look[];


void weather_update( void );

#endif
