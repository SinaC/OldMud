/***************************************************************************   
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "interp.h"
#include "dbdata.h"
#include "ban.h"
#include "config.h"
#include "utils.h"


BAN_DATA *ban_list;


bool check_ban(const char *site,int type) {
  BAN_DATA *pban;
  char host[MAX_STRING_LENGTH];

  strcpy(host,capitalize(site));
  host[0] = LOWER(host[0]);

  for ( pban = ban_list; pban != NULL; pban = pban->next ) {
    if(!IS_SET(pban->ban_flags,type))
      continue;

    if (IS_SET(pban->ban_flags,BAN_PREFIX) 
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)  
	&&  strstr(pban->name,host) != NULL)
      return TRUE;

    if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->name,host))
      return TRUE;

    if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->name,host))
      return TRUE;
  }

  return FALSE;
}


void ban_site(CHAR_DATA *ch, const char *argument, bool fPerm) {
  char buf[MAX_STRING_LENGTH],buf2[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char *name;
  BUFFER *buffer;
  BAN_DATA *pban, *prev;
  bool prefix = FALSE,suffix = FALSE;
  int type;

  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);

  if ( arg1[0] == '\0' ) {
    if (ban_list == NULL) {
      send_to_char("No sites banned at this time.\n\r",ch);
      return;
    }
    buffer = new_buf();

    add_buf(buffer,"Banned sites  level  type     status\n\r");
    for (pban = ban_list;pban != NULL;pban = pban->next) {
      sprintf(buf2,"%s%s%s",
	      IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : "",
	      pban->name,
	      IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : "");
      sprintf(buf,"%-12s    %-3d  %-7s  %s\n\r",
	      buf2, pban->level,
	      IS_SET(pban->ban_flags,BAN_NEWBIES) ? "newbies" :
	      IS_SET(pban->ban_flags,BAN_PERMIT)  ? "permit"  :
	      IS_SET(pban->ban_flags,BAN_ALL)     ? "all"	: "",
	      IS_SET(pban->ban_flags,BAN_PERMANENT) ? "perm" : "temp");
      add_buf(buffer,buf);
    }

    page_to_char( buf_string(buffer), ch );
    return;
  }

  /* find out what type of ban */
  if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
    type = BAN_ALL;
  else if (!str_prefix(arg2,"newbies"))
    type = BAN_NEWBIES;
  else if (!str_prefix(arg2,"permit"))
    type = BAN_PERMIT;
  else {
    send_to_char("Acceptable ban types are all, newbies, and permit.\n\r",
		 ch); 
    return;
  }

  name = arg1;

  if (name[0] == '*') {
    prefix = TRUE;
    name++;
  }

  if (name[strlen(name) - 1] == '*') {
    suffix = TRUE;
    name[strlen(name) - 1] = '\0';
  }

  if (strlen(name) == 0) {
    send_to_char("You have to ban SOMETHING.\n\r",ch);
    return;
  }

  prev = NULL;
  for ( pban = ban_list; pban != NULL; prev = pban, pban = pban->next ) {
    if (!str_cmp(name,pban->name)) {
      if (pban->level > get_trust(ch)) {
	send_to_char( "That ban was set by a higher power.\n\r", ch );
	return;
      }
      else {
	if (prev == NULL)
	  ban_list = pban->next;
	else
	  prev->next = pban->next;
      }
    }
  }

  pban = new_ban();
  pban->name = str_dup(name);
  pban->level = get_trust(ch);

  /* set ban type */
  pban->ban_flags = type;

  if (prefix)
    SET_BIT(pban->ban_flags,BAN_PREFIX);
  if (suffix)
    SET_BIT(pban->ban_flags,BAN_SUFFIX);
  if (fPerm)
    SET_BIT(pban->ban_flags,BAN_PERMANENT);

  pban->next  = ban_list;
  ban_list    = pban;
  new_save_bans();
  sprintf(buf,"%s has been banned.\n\r",pban->name);
  send_to_char( buf, ch );
  return;
}

void do_ban(CHAR_DATA *ch, const char *argument) {
  ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, const char *argument) {
  ban_site(ch,argument,TRUE);
}

void do_allow( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  BAN_DATA *prev;
  BAN_DATA *curr;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Remove which site from the ban list?\n\r", ch );
    return;
  }

  prev = NULL;
  for ( curr = ban_list; curr != NULL; prev = curr, curr = curr->next ) {
    if ( !str_cmp( arg, curr->name ) ) {
      if (curr->level > get_trust(ch)) {
	send_to_char(
		     "You are not powerful enough to lift that ban.\n\r",ch);
	return;
      }
      if ( prev == NULL )
	ban_list   = ban_list->next;
      else
	prev->next = curr->next;

      sprintf(buf,"Ban on %s lifted.\n\r",arg);
      send_to_char( buf, ch );
      new_save_bans();
      return;
    }
  }

  send_to_char( "Site is not banned.\n\r", ch );
  return;
}


//Ban.txt
//
//Ban <STRING Banned Host> {
//  Level = <INTEGER Level>;
//  Flags = <STRING flags (area flag format)>;
//}
//
//

void new_save_bans(void) {
  BAN_DATA *pban;
  FILE *fp;
  bool found = FALSE;

  fclose( fpReserve ); 
  if ( ( fp = fopen( BAN_FILE, "w" ) ) == NULL ) {
    bug("Could not open file [%s] in order to save ban data.", BAN_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
  }

  for (pban = ban_list; pban != NULL; pban = pban->next)
    if (IS_SET(pban->ban_flags,BAN_PERMANENT)) {
      found = TRUE;
      fprintf(fp,
	      "Ban '%s' {\n"
	      "  Level = %d;\n"
	      "  Flags = %s;\n"
	      "}\n\n",
	      pban->name, pban->level,quotify(print_flags(pban->ban_flags)));
    }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
  //  if (!found)    We don't remove the file if it's empty
  //    unlink(BAN_FILE);
}

BAN_DATA *ban_last;
void new_load_bans() {
  FILE *fp;
 
  log_string("Reading banned sites");

  fclose(fpReserve);
  if ( ( fp = fopen( BAN_FILE, "r" ) ) == NULL ) {
    bug("Could not open file [%s] in order to load ban data.", BAN_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
  }

  ban_list = NULL;
  ban_last = NULL;
  int count = parse_datas( fp );
  log_stringf(" %d banned sites found.", count );

  fpReserve = fopen( NULL_FILE, "r" );
}

//#define VERBOSE

void parse_ban( DATAData *ban ) {
  BAN_DATA *pban;

  pban = new_ban();

  // value
  pban->name = str_dup( ban->value->eval().asStr() );

  if ( DATA_VERBOSE > 0 ) {
    printf("Ban Name: %s\n\r", pban->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < ban->fields_count; fieldCount++ ) {
    DATAData *field = ban->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Level: pban->level = field->value->eval().asInt(); break;
    case TAG_Flags: pban->ban_flags = flag_to_long( field->value->eval().asStr() ); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
  
  if ( DATA_VERBOSE > 1 ) {
    printf("  level: %d   flags: %d\n\r", pban->level, pban->ban_flags );
  }

  if (ban_list == NULL)
    ban_list = pban;
  else
    ban_last->next = pban;
  ban_last = pban;
}
