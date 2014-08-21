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
#include "utils.h"
#include "db.h"

void strip_char( char *s, const char c ) {
  char *p = s;
  char *q = s;
  
  while ( *q != '\0' ) {
    if ( *q == c )
      *q++;
    *p++ = *q++;
  }
  *p = '\0';
}

void strip_char_leading( char *s, const char c ) {
  char *p = s;
  char *q = s;
  
  while ( *q == c )
    q++;
  while ( *q != '\0' )
    *p++ = *q++;
  *p = '\0';
}

void strip_char_ending( char *s, const char c ) {
  char *p = &(s[strlen(s)-1]);
  
  while ( *p == c )
    p--;
  *p = '\0';
}





/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy( const int number ) {
  switch ( number_bits( 2 ) ) {
  case 0:  return UMAX( 1, number - 1 ); break;
  case 3:  return UMAX( 1, number + 1 ); break;
  }

  return UMAX( 1, number );
}



/*
 * Generate a random number.
 */
int number_range( const int from, const int toIn ) {
  int power;
  int number;
  int to = toIn;

  if (from == 0 && to == 0)
    return 0;

  if ( ( to = to - from + 1 ) <= 1 )
    return from;

  for ( power = 2; power < to; power <<= 1 )
    ;

  while ( ( number = number_mm() & (power -1 ) ) >= to )
    ;

  return from + number;
}


/* CHANCE function. I use this everywhere in my code, very handy :> */
// Added by SinaC 2000 comes from quest.C
bool chance( const int num ) {
  if (number_range(1,100) <= num) return TRUE;
  else return FALSE;
}


/*
 * Generate a percentile roll.
 */
int number_percent() {
  int percent;

  while ( (percent = number_mm() & (128-1) ) > 99 )
    ;

  return 1 + percent;
}

// Added by SinaC 2001
int count_bit( const long flag ){
  int res = 0;
  int l = flag;
  for ( int i = 0; i < 31; i++ ) {
    if ( l & 1 )
      res++;
    l = l >> 1;
  }
  return res;
}



/*
 * Generate a random door.
 */
int number_door() {
  int door;

  //while ( ( door = number_mm() & (8-1) ) > 5)
  //  ;
  while ( ( door = number_mm() & (8-1) ) >= MAX_DIR)   // Modified by SinaC 2003
    ;

  return door;
}

long number_bits( const long width ) {
  return number_mm( ) & ( ( 1 << width ) - 1 );
}

/*
 * Roll some dice.
 */
int dice( const int number, const int size ) {
  int idice;
  int sum;

  switch ( size ) {
  case 0: return 0;
  case 1: return number;
  }

  for ( idice = 0, sum = 0; idice < number; idice++ )
    sum += number_range( 1, size );

  return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate( const int level, const int value_00, const int value_32 ) {
  return value_00 + ( level * (value_32 - value_00) )/ 32;
}

/*
 * Remove spaces in a string, added by SinaC 2001
 */
char* trim( const char *str ) {
  static char buf[MAX_STRING_LENGTH];
  const char *s, *t;

  buf[0] = '\0';

  s=str;
  while (*s && isspace(*s)) s++;
  
  t=str+strlen(str)-1;

  while (t>=s && isspace(*t)) t--;
  t++;
  memcpy(buf, s, t-s);
  buf[t-s] = '\0';

  return buf;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
//void smash_tilde( char *str ) {
//  for ( ; *str != '\0'; str++ ) {
//    if ( *str == '~' )
//      *str = '-';
//  }

//  return;
//}

/*
 * Returns an initial-capped string.
 */
char *capitalize( const char *str ) {
  static char strcap[MAX_STRING_LENGTH];
  int i;

  for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = LOWER(str[i]);
  strcap[i] = '\0';
  strcap[0] = UPPER(strcap[0]);
  return strcap;
}

char *str_to_upper( const char *str ) {
  static char strcap[MAX_STRING_LENGTH];
  int i;

  strcap[0] = '\0';

  for ( i = 0; str[i] != '\0'; i++ )
    strcap[i] = UPPER( str[i] );

  strcap[i] = '\0';

  return strcap;
}

// Modified by SinaC 2001
char *print_flags( const long flag ) {
  long count, pos = 0;
  static char buf[512];

  for (count = 0; count < 32;  count++)
    if (IS_SET(flag,1<<count)){
      if (count < 26)
	buf[pos] = 'A' + count;
      else
	buf[pos] = 'a' + (count - 26);
      pos++;
    }
    
  if (pos == 0){
    buf[pos] = '0';
    pos++;
  }
    
  buf[pos] = '\0';
    
  return buf;
}

// SinaC 2003, doesn't work but it doesn't matter, it's not used =))
const char *tokenize( const char *argument ) {
  static char buf[MAX_STRING_LENGTH];
  char tok[MAX_INPUT_LENGTH];
  buf[0] = '\0';
  const char *s = argument;
  char *q = tok;
  bool close = TRUE;
  while ( *s != '\0' ) {
    if ( ( *s == ' ' || *s == '-' ) && close) {
      while ( *s == ' ' || *s == '-' )
	s++;
      *q = '\0';
      strcat( buf, " " );
      strcat( buf, q );
      q = tok;
    }
    else {
      *q = *s;
      if ( *s == '\'' || *s == '\"' )
	close = !close;
      q++; s++;
    }
  }
  return buf+1;
}
