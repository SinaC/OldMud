#include <time.h>  // added by SinaC 2003
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include "merc.h"
#include "script2.h"
#include "cxx/parser_skel.hh"
#include "grammar.hh"
#include "db.h"
#include "config.h"
#include "error.hh"

//#define VERBOSE

int token;
const char* cur_pos;
const char* previous_cur_pos;

#define NOTHING (-666)
#define COMMENT (-999)

int grammar_scan() {
  int res=NOTHING; 

  previous_cur_pos = cur_pos;

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

    case '`':
      cur_pos++;
      start = cur_pos;
      while (*cur_pos != '\n' && *cur_pos != '`' )
	cur_pos++;

      if ( *cur_pos == '\n' )
	numline++;

      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';
      
      if ( *cur_pos == '\n' )
	res = COMMAND_LAST;
      else
	res = COMMAND;
      cur_pos++;
      break;

    case '<':
      cur_pos++;
      if (*cur_pos == '=') {
	cur_pos++;
	res = LE;	
      } else
      if (*cur_pos == '-') {
	cur_pos++;
	res = LEFTARROW;	
      } else
	res = '<';  
      break;

    case '>':
      cur_pos++;
      if (*cur_pos == '=') {
	cur_pos++;
	res = GE;
      } else
	res = '>';  
      break;

    case '%':
      cur_pos++;
      if (*cur_pos == '=') {
	cur_pos++;
	res = EQUIV;
      }
      else
	res = NOTHING;
      break;

    case '/':
    case '&':
    case '|':
    case '=':
      res = NOTHING;
      if (*(cur_pos+1) == *cur_pos) {
	// the two chars are equal.
	cur_pos++;
	switch (*cur_pos) {
	case '&':
	  res = AND;
	  break;
	case '|':
	  res = OR;
	  break;
	case '=':
	  res = EQ;
	  break;
	case '/':
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

    case '*':
    case ';':
    case ',':
    case '+':
    case '-':
    case '(':
    case ')':
    case '[':
    case ']':
    case '{':
    case '}':
      res = *cur_pos;
      cur_pos++;
      break;

      // Modified by SinaC 2003 .. added
    case '.':
      if (*(cur_pos+1) == '.') {
	cur_pos++;
	res = PTPT;
      }
      else
	res = *cur_pos;
      cur_pos++;
      break;

      // Modified by SinaC 2003  ::  added
    case ':':
      if (*(cur_pos+1) == ':') {
	cur_pos++;
	res = DBLPT;
      } else
	res = *cur_pos;
      cur_pos++;
      break;

    case '!':      
      if (*(cur_pos+1) == '=') {
	cur_pos++;
	res = NE;
      } else
	res = '!';
      cur_pos++;
      break;

    case '0' ... '9':
      start = cur_pos;
      while (!isspace(*cur_pos) && !ispunct(*cur_pos)) 
	cur_pos++;

      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';
      
      res = INTEGER;
      break;

    case 'A' ... 'Z':
    case 'a' ... 'z':
    case '_':

      start = cur_pos;
      while ((!isspace(*cur_pos) && !ispunct(*cur_pos))||*cur_pos=='_')
	cur_pos++;
    
      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';

      if (!strcmp("if", token_image))
	res = IF;
      else if (!strcmp("in", token_image))
	res = IN;
      else if (!strcmp("else", token_image))
	res = ELSE;
      else if (!strcmp("objvar", token_image))
	res = OBJVAR;
      else if (!strcmp("var", token_image))
	res = VAR;
      else if (!strcmp("class", token_image))
	res = CLASS;
      else if (!strcmp("while", token_image))
	res = WHILE;
      else if (!strcmp("extends", token_image))
	res = EXTENDS;
      else if (!strcmp("force", token_image))
	res = FORCE;
      else if (!strcmp("NULL", token_image))
	res = NULL_TOK;
      else if (!strcmp("abstract", token_image))
	res = ABSTRACT;
      else if (!strcmp("is", token_image)) // SinaC 2003
	res = IS;
      else if (!strcmp("has", token_image)) // SinaC 2003
	res = HAS;
      else if (!strcmp("first", token_image))  // SinaC 2003
	res = FIRST;
      else if (!strcmp("any", token_image))  // SinaC 2003
	res = ANY;
      else if (!strcmp("delvar", token_image))  // SinaC 2003
	res = DELVAR;
      else if (!strcmp("lifetimevar", token_image))  // SinaC 2003
	res = LIFETIMEVAR;
      else res = IDENT;
      break;
    case '"':
      cur_pos++;
      start = cur_pos;
      while (*cur_pos != '"')
	cur_pos++;
      strncpy(token_image, start, cur_pos-start);
      token_image[cur_pos-start] = '\0';
      cur_pos++;
      res = STRING;
      break;
    }
  
    token=res;
    //    log_stringf("Found token: %d '%c' [%s]\n", res, res, token_image);
  } while (res == COMMENT);  

  if ( SCRIPT_VERBOSE > 5 ) {
    printf("=== scan()  token [%d]  token_image [%s]\n", token, token_image); fflush(stdout);
  }

  if ( res == NOTHING )
    p_error("Unknown token: [%s] line: %d", token_image, numline );

  return res;
}

void expect(int tok) {
  if (tok != token) {
    p_error("Parser error: Expected token %d '%c' at %s", tok, tok, cur_pos);
  }
  grammar_scan();
}
