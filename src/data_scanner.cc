#include <time.h>  // added by SinaC 2003
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include "merc.h"
#include "script2.h"
#include "cxx/parser_skel.hh"
#include "data_parser.hh"
#include "db.h"
#include "scanner.hh"
#include "utils.h"

//#define VERBOSE


#define NOTHING (-666)
#define COMMENT (-999)

// add \r for each \n found
void add_carriage_return( char *s ) {
  if ( !strstr( s, "\n" ) )
    return;
  char buf[MAX_STRING_LENGTH];
  char *q = s;
  char *p = buf;

  while ( *q != '\0' ) {
    *p = *q;
    if ( *q == '\n' )
      *(++p) = '\r';
    p++; q++;
  }
  *p = '\0';

  strcpy( s, buf );
}

int data_scan() {
  int res=NOTHING; 

  const char* start;
  token_image[0] = '\0';

  do {
    while (isspace(*cur_pos)) {
      if ( *cur_pos == '\n' )
	numline++;
      cur_pos++;
    }

    switch (*cur_pos) {   

    case '\0' :
      res = E_O_F;
      break;

    case '/':
      res = NOTHING;
      if (*(cur_pos+1) == *cur_pos) {
	// the two chars are equal.
	cur_pos++;
	if ( *cur_pos == '/' ) {
	  while ((*cur_pos) != '\n')
	    cur_pos++;
	  numline++;
    	  res = COMMENT;
	  break;
	}
	cur_pos++;
      }
      
      if (res != NOTHING)
	break;
      // ELSE FALL THROUGH

    case '=':
    case '(':
    case ')':
    case '{':
    case '}':
    case ';':
    case ',':
    case '+':
      res = *cur_pos;
      cur_pos++;
      break;

    case '-':
    case '0' ... '9':
      start = cur_pos;

      if ( *cur_pos == '-' )
	cur_pos++;

      while (!isspace(*cur_pos) && !ispunct(*cur_pos)) 
	cur_pos++;

      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';
      
      res = DATA_INTEGER;
      break;

    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '_':

      start = cur_pos;
      while ((!isspace(*cur_pos) && !ispunct(*cur_pos))||*cur_pos=='_')
	cur_pos++;
    
      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';
      
      if ( !str_cmp( token_image, "true" ) || !str_cmp( token_image, "false") )
	res = DATA_BOOLEAN;
      else 
	res = DATA_IDENT;
      break;

    case '\'': {
      cur_pos++;
      start = cur_pos;
      bool found = FALSE;
      while (*cur_pos != '\'') {
	if ( *cur_pos == '\n' )
	  numline++;
	if ( *cur_pos == '\\' ) { // skip backslash
	  found = TRUE;
	  cur_pos++;
	}
	cur_pos++;
      }
      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';
      // add \r for each \n found
      //add_carriage_return( token_image );

      //remove backslash
      if ( found ) strip_char( token_image, '\\' );

      cur_pos++;
      res = DATA_STRING;
      break;
    }

    case '"': {
      cur_pos++;
      start = cur_pos;
      bool found = FALSE;
      while (*cur_pos != '"') {
	if ( *cur_pos == '\n' )
	  numline++;
	if ( *cur_pos == '\\' ) {// skip backslash
	  found = TRUE;
	  cur_pos++;
	}
	cur_pos++;
      }
      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';

     // add \r for each \n found
      //add_carriage_return( token_image );

      //remove backslash
      if ( found ) strip_char( token_image, '\\' );

      cur_pos++;
      res = DATA_STRING;
      break;
    }
    }
  
    token=res;
    //    log_stringf("Found token: %d '%c' [%s]\n", res, res, token_image);
  } while (res == COMMENT);  

#ifdef VERBOSE
  printf("=== scan()  token_image [%s]\n", token_image); fflush(stdout);
#endif

  if ( res == NOTHING )
    p_error("Unknown token: [%s] [line %d]", token_image, numline );

  return res;
}
