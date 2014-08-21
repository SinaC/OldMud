/***************************************************************************
 *  File: string.c                                                         *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/


#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

// Added by SinaC 2001
#include "db.h"
#include "comm.h"
#include "string.h"
#include "interp.h"
// shift(int)
#include "cxx/parser_skel.hh"
#include "utils.h"


typedef void SCRIPT_EDIT_FUN  args( ( CHAR_DATA *ch, FCT_DATA *f ) );

// Show color code if stringColorMode if OFF
void string_send_to_char( const char *s, CHAR_DATA *ch ) {
  if ( ch->desc->stringColorMode )
    send_to_char( s, ch );
  else
    write_to_buffer( ch->desc, s, -1 );
}


/*****************************************************************************
 Name:		string_edit
 Purpose:	Clears string and puts player into editing mode.
 Called by:	none
 ****************************************************************************/
void string_edit( CHAR_DATA *ch, const char **pString ) {
  send_to_char( "-========- Entering EDIT Mode -=========-\n\r", ch );
  send_to_char( "    Type .h on a new line for help\n\r", ch );
  send_to_char( " Terminate with a @ (not ~) on a blank line.\n\r", ch );
  send_to_char( "-=======================================-\n\r", ch );

  *pString = str_dup( "" );

  ch->desc->pString = pString;

  return;
}



/*****************************************************************************
 Name:		string_append
 Purpose:	Puts player into append mode for given string.
 Called by:	(many)olc_act.c
 ****************************************************************************/
void string_append( CHAR_DATA *ch, const char **pString ) {
  send_to_char( "-=======- Entering APPEND Mode -========-\n\r", ch );
  send_to_char( "    Type .h on a new line for help\n\r", ch );
  send_to_char( " Terminate with a @ (not ~) on a blank line.\n\r", ch );
  send_to_char( "-=======================================-\n\r", ch );

  if ( *pString == NULL ) {
    *pString = str_dup( "" );
  }

  if ( ch->desc->stringColorMode )
    string_show( *pString, ch );
  else
    string_show_without_color( *pString, ch );
    
  if ( *(*pString + strlen( *pString ) - 1) != '\r' )
    send_to_char( "\n\r", ch );

  ch->desc->pString = pString;

  return;
}



/*****************************************************************************
 Name:		string_replace
 Purpose:	Substitutes one string for another.
 Called by:	string_add(string.c) (aedit_builder)olc_act.c.
 ****************************************************************************/
const char * string_replace( const char * orig, const char * old, const char * newstr ) {
  char xbuf[MAX_STRING_LENGTH];
  int i;

  xbuf[0] = '\0';
  strcpy( xbuf, orig );
  if ( strstr( orig, old ) != NULL ) {
    i = strlen( orig ) - strlen( strstr( orig, old ) );
    xbuf[i] = '\0';
    strcat( xbuf, newstr );
    strcat( xbuf, &orig[i+strlen( old )] );
  }

  return str_dup( xbuf );
}



/*****************************************************************************
 Name:		string_add
 Purpose:	Interpreter for string editing.
 Called by:	game_loop_xxxx(comm.c).
 ****************************************************************************/
void string_add( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  /*
   * Thanks to James Seng
   */
  //  smash_tilde( argument );

  if ( *argument == '.' ) {
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];
    //char arg3 [MAX_INPUT_LENGTH];

    argument = one_argument( argument, arg1 );
    argument = first_arg( argument, arg2, FALSE );
    //argument = first_arg( argument, arg3, FALSE );

    if ( !str_cmp( arg1, ".c" ) ) {
      send_to_char( "String cleared.\n\r", ch );
      *ch->desc->pString = str_dup("");
      return;
    }

    if ( !str_cmp( arg1, ".s" ) ) {
      send_to_char( "String so far:\n\r", ch );
      string_show( *ch->desc->pString, ch );
      return;
    }

    if ( !str_cmp( arg1, ".r" ) ) {
      if ( arg2[0] == '\0' ) {
	send_to_char(
		     "usage:  .r \"old string\" \"new string\"\n\r", ch );
	return;
      }
      //      smash_tilde( arg3 );   /* Just to be sure -- Hugin */
      *ch->desc->pString = string_replace( *ch->desc->pString, arg2, argument );//, arg3 );
      sprintf( buf, "'%s' replaced with '%s'.\n\r", arg2, argument );//, arg3 );
      string_send_to_char( buf, ch );
      return;
    }

    if ( !str_cmp( arg1, ".f" ) ) {
      *ch->desc->pString = format_string( *ch->desc->pString );
      send_to_char( "String formatted.\n\r", ch );
      return;
    }

    if ( !str_cmp( arg1, ".n") ) {
      ch->desc->lineMode = !ch->desc->lineMode;
      send_to_charf(ch,"Line mode %s.\n\r",ch->desc->lineMode?"Activated":"Desactivated");
      return;
    }

    if ( !str_cmp( arg1, ".rl" ) ) {
      if ( arg2[0] == '\0' || !is_number(arg2 ) ) {
	send_to_char("usage:   .rl #line \"new string\"\n\r", ch );
	return;
      }
      int line = atoi(arg2);
      *ch->desc->pString = string_replace_line( *ch->desc->pString, line, argument );//, arg3 );
      sprintf( buf, "Line %d replaced with '%s'.\n\r", line, argument ); //arg3 );
      string_send_to_char( buf, ch );
      return;
    }

    if ( !str_cmp( arg1, ".cl" ) ) {
      if ( arg2[0] == '\0' || !is_number(arg2 ) ) {
	send_to_char("usage:   .d #line\n\r", ch );
	return;
      }
      int line = atoi(arg2);
      *ch->desc->pString = string_delete_line( *ch->desc->pString, line );
      send_to_charf(ch,"Line %d deleted.\n\r", line );
      return;
    }

    if ( !str_cmp( arg1, ".i" ) ) {
      if ( arg2[0] == '\0' || !is_number(arg2 ) ) {
	send_to_char("usage:   .i #line \"string\"\n\r", ch );
	return;
      }
      int line = atoi(arg2);
      *ch->desc->pString = string_insert_line( *ch->desc->pString, line, argument );//, arg3 );
      //sprintf( buf, "'%s' inserted before line %d.\n\r", arg3, line );
      sprintf( buf, "'%s' inserted before line %d.\n\r", argument, line );
      string_send_to_char( buf, ch );
      return;
    }

    if ( !str_cmp( arg1, ".sc" ) ) {
      send_to_char( "String so far (without color code):\n\r", ch );
      string_show_without_color( *ch->desc->pString, ch );
      return;
    }

    if ( !str_cmp( arg1, ".ra" ) ) {
      if ( arg2[0] == '\0' ) {
	send_to_char(
		     "usage:  .ra \"old string\" \"new string\"\n\r", ch );
	return;
      }
      *ch->desc->pString = string_replace_all( *ch->desc->pString, arg2, argument );//, arg3 );
      sprintf( buf, "'%s' replaced everywhere with '%s'.\n\r", arg2, argument );//, arg3 );
      string_send_to_char( buf, ch );
      return;
    }

    if ( !str_cmp( arg1, ".color" ) ) {
      ch->desc->stringColorMode = !ch->desc->stringColorMode;
      send_to_charf(ch,"String color mode %s.\n\r",ch->desc->stringColorMode?"Activated":"Desactivated");
      return;
    }

    if ( !str_cmp( arg1, ".indent" ) ) {
      *ch->desc->pString = string_indent( *ch->desc->pString  );
      send_to_charf(ch,"String indentation completed.\n\r");
      return;
    }

    if ( !str_cmp( arg1, ".h" ) ) {
      send_to_char( "Sedit help (commands on blank line): \n\r", ch );
      send_to_char( ".r 'old' new        - replace a substring (old requires '', \"\")\n\r", ch );
      send_to_char( ".h                  - get help (this info)\n\r", ch );
      send_to_char( ".s                  - show string so far\n\r", ch );
      send_to_char( ".f                  - (word wrap) string\n\r", ch );
      send_to_char( ".c                  - clear string so far\n\r", ch );
      send_to_char( ".rl #line new       - replace a line\n\r", ch );
      send_to_char( ".cl #line           - clear a line\n\r", ch );
      send_to_char( ".i #line str        - insert a str before a line \n\r", ch );
      send_to_char( ".ra 'old' new       - replace every substring (old requires '', \"\")\n\r", ch );
      send_to_char( ".n                  - toggle line number mode\n\r", ch );
      send_to_char( ".color              - toggle string editing color mode (off= show color code {{ )\n\r", ch );
      send_to_char( ".indent             - indent string (kind of word wrap for program)\n\r", ch );
      send_to_char( "@                   - end string         \n\r", ch );
      return;
    }
            

    send_to_char( "SEdit:  Invalid dot command.\n\r", ch );
    return;
  }

  if ( *argument == '~' || *argument == '@' ) {
    ch->desc->pString = NULL;
    send_to_charf(ch,"\n\r");
    // Call a special procedure initalized before setting pString, SinaC 2003
    //  may only call a script_edit_function for the moment (update_after_edit_method)
    if ( ch->desc->pStringFun != NULL )
      (*(SCRIPT_EDIT_FUN*)ch->desc->pStringFun)( ch, (FCT_DATA *)ch->desc->pStringArg );
    return;
  }

  strcpy( buf, *ch->desc->pString );

  /*
   * Truncate strings to MAX_STRING_LENGTH.
   * --------------------------------------
   */
  if ( strlen( buf ) + strlen( argument ) >= ( MAX_STRING_LENGTH - 4 ) ) {
    send_to_char( "String too long, last line skipped.\n\r", ch );

    /* Force character out of editing mode. */
    ch->desc->pString = NULL;
    return;
  }

  /*
   * Ensure no tilde's inside string.
   * --------------------------------
   */
  //  smash_tilde( argument );

  strcat( buf, argument );
  strcat( buf, "\n\r" );
  *ch->desc->pString = str_dup( buf );
  return;
}



/*
 * Thanks to Kalgen for the new procedure (no more bug!)
 * Original wordwrap() written by Surreality.
 */
/*****************************************************************************
 Name:		format_string
 Purpose:	Special string formating and word-wrapping.
 Called by:	string_add(string.c) (many)olc_act.c
 ****************************************************************************/
const char *format_string( const char *oldstring0 /*, bool fSpace */) {
  char xbuf[MAX_STRING_LENGTH];
  char xbuf2[MAX_STRING_LENGTH];
  char *rdesc;
  int i=0;
  bool cap=TRUE;
  
  xbuf[0]=xbuf2[0]=0;
  
  i=0;
  
  char oldstring[MAX_STRING_LENGTH];
  strcpy(oldstring,oldstring0);

  for (rdesc = oldstring; *rdesc; rdesc++) {
    if (*rdesc=='\n') {
      if (xbuf[i-1] != ' ') {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc=='\r') ;
    else if (*rdesc==' ') {
      if (xbuf[i-1] != ' ') {
        xbuf[i]=' ';
        i++;
      }
    }
    else if (*rdesc==')') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        xbuf[i-1]=' ';
        xbuf[i]=' ';
        i++;
      }
      else {
        xbuf[i]=*rdesc;
        i++;
      }
    }
    else if (*rdesc=='.' || *rdesc=='?' || *rdesc=='!') {
      if (xbuf[i-1]==' ' && xbuf[i-2]==' ' && 
          (xbuf[i-3]=='.' || xbuf[i-3]=='?' || xbuf[i-3]=='!')) {
        xbuf[i-2]=*rdesc;
        if (*(rdesc+1) != '\"') {
          xbuf[i-1]=' ';
          xbuf[i]=' ';
          i++;
        }
        else {
          xbuf[i-1]='\"';
          xbuf[i]=' ';
          xbuf[i+1]=' ';
          i+=2;
          rdesc++;
        }
      }
      else {
        xbuf[i]=*rdesc;
        if (*(rdesc+1) != '\"') {
          xbuf[i+1]=' ';
          xbuf[i+2]=' ';
          i += 3;
        }
        else {
          xbuf[i+1]='\"';
          xbuf[i+2]=' ';
          xbuf[i+3]=' ';
          i += 4;
          rdesc++;
        }
      }
      cap = TRUE;
    }
    else {
      xbuf[i]=*rdesc;
      if ( cap ) {
	cap = FALSE;
	xbuf[i] = UPPER( xbuf[i] );
      }
      i++;
    }
  }
  xbuf[i]=0;
  strcpy(xbuf2,xbuf);
  
  rdesc=xbuf2;
  
  xbuf[0]=0;
  
  for ( ; ; ) {
    for (i=0; i<77; i++) {
      if (!*(rdesc+i)) break;
    }
    if (i<77) {
      break;
    }
    for (i=(xbuf[0]?76:73) ; i ; i--) {
      if (*(rdesc+i)==' ') break;
    }
    if (i) {
      *(rdesc+i)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"\n\r");
      rdesc += i+1;
      while (*rdesc == ' ') rdesc++;
    }
    else {
      bug ("No spaces");
      *(rdesc+75)=0;
      strcat(xbuf,rdesc);
      strcat(xbuf,"-\n\r");
      rdesc += 76;
    }
  }
  while (*(rdesc+i) && (*(rdesc+i)==' '||
                        *(rdesc+i)=='\n'||
                        *(rdesc+i)=='\r'))
    i--;
  *(rdesc+i+1)=0;
  strcat(xbuf,rdesc);
  if (xbuf[strlen(xbuf)-2] != '\n')
    strcat(xbuf,"\n\r");

  return(str_dup(xbuf));
}



/*
 * Used above in string_add.  Because this function does not
 * modify case if fCase is FALSE and because it understands
 * parenthesis, it would probably make a nice replacement
 * for one_argument.
 */
/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quotes, parenthesis (barring ) ('s) and
 		percentages.
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *first_arg( const char *argument, char *arg_first, bool fCase ) {
  char cEnd;

  while ( *argument == ' ' )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"'
       || *argument == '%'  || *argument == '(' ) {
    if ( *argument == '(' ) {

      cEnd = ')';
      argument++;
    }
    else cEnd = *argument++;
  }

  while ( *argument != '\0' ) {
    if ( *argument == cEnd ) {
      argument++;
      break;
    }
    if ( fCase ) *arg_first = LOWER(*argument);
    else *arg_first = *argument;
    arg_first++;
    argument++;
  }
  *arg_first = '\0';

  while ( *argument == ' ' )
    argument++;

  return argument;
}




/*
 * Used in olc_act.c for aedit_builders.
 */
const char * string_unpad( const char * argument0 ) {
  char buf[MAX_STRING_LENGTH];
  char *s;

  char argument[MAX_STRING_LENGTH];
  strcpy(argument,argument0);

  s = argument;

  while ( *s == ' ' )
    s++;

  strcpy( buf, s );
  s = buf;

  if ( *s != '\0' ) {
    while ( *s != '\0' )
      s++;
    s--;

    while( *s == ' ' )
      s--;
    s++;
    *s = '\0';
  }

  return str_dup( buf );
}



/*
 * Same as capitalize but changes the pointer's data.
 * Used in olc_act.c in aedit_builder.
 */
const char * string_proper( const char * argument0 ) {
  char *s;
  char argument[MAX_STRING_LENGTH];
  strcpy(argument,argument0);

  s = argument;

  while ( *s != '\0' )
    if ( *s != ' ' ) {
      *s = UPPER(*s);
      while ( *s != ' ' && *s != '\0' )
	s++;
    }
    else
      s++;

  return str_dup(argument);
}

//-------------------------- online text editor: smarter edition functions
void string_show( const char *buffer, CHAR_DATA *ch ) {
  const char *p = buffer;
  char buf[MAX_STRING_LENGTH];
  char *b = buf;
  int lines = 0;

  if ( !ch->desc->stringColorMode ) {
    string_show_without_color( buffer, ch );
    return;
  }

  if ( !ch->desc->lineMode ) {
    send_to_char( buffer, ch );
    return;
  }

  while (*p) {
    if ( *p == '\n' || *p == '\r' || *p == '\0' ) {
      lines++; p++;
      *b = '\0';
      //if ( b-buf <= 0 ) do we really need this test?
      //break;
      send_to_charf(ch,"%2d> %s\n\r",lines,buf);
      if ( *p == '\r' ) // skip '\r'
	p++;
      b = buf;
    }
    else
      *b++ = *p++;
  }
}

void get_string_line( const char *buffer, const int line, 
		      int &lStart, int &lEnd ) {
  const char *p = buffer;
  int lines = 0;
  int i = 0;
  lStart = 0;
  lEnd = 0;

  while (*p) {
    if ( *p == '\n' || *p == '\r' || *p == '\0' ) {
      lines++; p++; i++;
      if ( *p == '\r' ) { // skip '\r'
	p++;
	i++;
      }
      lStart = lEnd;
      //lEnd = i; 
      lEnd = p - buffer;
      if ( lines == line )
	return;
    }
    else {
      p++;
      i++;
    }
  }
  lStart = lEnd; // simulate a misuse
  return;
}

const char *string_replace_line( const char *buffer, const int line, const char *newstring ) {
  char xbuf[MAX_STRING_LENGTH];
  int lStart, lEnd;
  get_string_line( buffer, line, lStart, lEnd );

  if ( lStart == lEnd )
    return buffer;

  strncpy(xbuf,buffer,lStart); // before #line
  xbuf[lStart] = '\0';
  strcat(xbuf,newstring);      // insert new string
  strcat(xbuf,"\n\r");
  strcat(xbuf,buffer+lEnd);    // after #line

  return str_dup(xbuf);
}

const char *string_delete_line( const char *buffer, const int line ) {
  char xbuf[MAX_STRING_LENGTH];
  int lStart, lEnd;
  get_string_line( buffer, line, lStart, lEnd );

  if ( lStart == lEnd )
    return buffer;

  strncpy(xbuf,buffer,lStart); // before #line
  xbuf[lStart] = '\0';
  strcat(xbuf,buffer+lEnd);    // after #line

  return str_dup(xbuf);
}

const char *string_insert_line( const char *buffer, const int line, const char *str ) {
  char xbuf[MAX_STRING_LENGTH];
  int lStart, lEnd;
  get_string_line( buffer, line, lStart, lEnd );

  if ( lStart == lEnd )
    return buffer;

  strncpy(xbuf,buffer,lStart); // before #line
  xbuf[lStart] = '\0';
  strcat(xbuf,str);            // insert string
  strcat(xbuf,"\n\r");
  strcat(xbuf,buffer+lStart);  // after #line

  return str_dup(xbuf);
}

const char * string_replace_all( const char * orig, const char * old, const char * newstr ) {
  char xbuf[MAX_STRING_LENGTH];

  int ol = strlen( old );
  int nl = strlen( newstr );

  strcpy( xbuf, orig ); // copy buffer
  char *s = &xbuf[0]; // start at the beginning of the buffer
  while ( 1 ) {
    char *t = strstr( s, old ); // search old in the rest of buffer
    if ( t == NULL ) // loop end condition
      break;
    char buf[MAX_STRING_LENGTH];
    strcpy( buf, t+ol ); // save end of string
    *t = '\0';
    strcat( t, newstr ); // replace old with newstr
    s = t+nl; // we'll find old after newstr
    strcat( t, buf ); // copy end of string after newstr
  }

  return str_dup( xbuf );
}

void string_show_without_color( const char *buffer, CHAR_DATA *ch ) {
  const char *p = buffer;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char *b = buf;
  int lines = 0;

  if ( ch->desc->stringColorMode ) {
    string_show( buffer, ch );
    return;
  }

  if ( !ch->desc->lineMode ) {
    write_to_buffer( ch->desc, buffer, -1 );
    return;
  }

  while (*p) {
    if ( *p == '\n' || *p == '\r' || *p == '\0' ) {
      lines++; p++;
      *b = '\0';
      //if ( b-buf <= 0 ) do we really need this test.
      //break;
      sprintf( buf2, "%2d> %s\n\r",lines,buf );
      write_to_buffer( ch->desc, buf2, -1 );
      if ( *p == '\r' ) // skip '\r'
	p++;
      b = buf;
    }
    else
      *b++ = *p++;
  }
  return;
}

bool is_opening_parenthesis( const char c ) {
  if ( c == '(' || c == '[' || c == '{' )
    return TRUE;
  return FALSE;
}
bool is_closing_parenthesis( const char c ) {
  if ( c == ')' || c == ']' || c == '}' )
    return TRUE;
  return FALSE;
}

const char * string_indent( const char *buffer ) {
  char buf[MAX_STRING_LENGTH];
  char oneLine[MAX_STRING_LENGTH];
  char *b = oneLine;
  const char *p = buffer;
  int parentCount; // current number of non-closed parenthesis
  int previousParent; // number of non-closed parenthesis the line before
  const int additionalParent = 1; // starting number of parenthesis
  const int spaceByParent = 2; // number of space to add for each non-closed parenthesis

  bool inString = FALSE; // are we in a string
  bool inComment = FALSE; // are we in a comment
  buf[0] = '\0';
  previousParent = parentCount = 0;

  while(1) {
    if ( *p == '\n' || *p == '\r' || *p == '\0' ) { // CR or end of string
      *b = '\0'; // end of the line
      if ( !inString ) // we add spaces only if we are not in a string
	if ( previousParent >= parentCount )
	  strcat( buf, shift( (parentCount+additionalParent)*spaceByParent ) ); // add spaces
	else // we don't add a space for the line continaing the opening parenthesis
	  strcat( buf, shift( (parentCount+additionalParent-1)*spaceByParent ) ); // add spaces
      strcat( buf, trim( oneLine ) ); // add the line and remove leading and ending spaces

      if ( *p == '\0' ) // end of loop when not more char to read
	break;

      // we add a \n\r only if we are not at the last line because we are sure to have
      //  an empty string if it ends with a \0
      strcat( buf, "\n\r" ); // add a CR

      //      printf("%d  %s  %s\n\r"
      //	     "string:[%s]\n\r", 
      //	     parentCount, inString?"true":"false", inComment?"true":"false",
      //	     oneLine ); fflush(stdout);

      p++;
      if ( *p == '\r' ) // skip '\r'
	p++;
      b = oneLine; *b = '\0'; // we start a new line
      inComment = FALSE;
      previousParent = parentCount;
    }
    else {
      if ( !inString && !inComment && is_opening_parenthesis( *p ) )
	parentCount++;
      else if ( !inString && !inComment && is_closing_parenthesis( *p ) )
	parentCount--;
      else if ( !inComment && *p == '"' ) // toggle inString
	inString = !inString;
      else if ( *p == '/' && *p && *p == '/' ) // set comment ON
	inComment = TRUE;
      *b++ = *p++; // fill the line
    }
  }
  return str_dup(buf);
}
