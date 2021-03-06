/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvements copyright (C) 1992, 1993 by Michael         *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefiting.  We hope that you share your changes too.  What goes       *
 *  around, comes around.                                                  *
 ***************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include "merc.h"
#include "recycle.h"

// Added by SinaC 2001
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "save.h"
#include "olc_value.h"
#include "interp.h"
#include "wizutil.h"
#include "act_info.h"
#include "config.h"
#include "utils.h"


const char wizutil_id [] = "$Id: wizutil.c,v 1.6 1996/01/04 21:30:45 root Exp root $";

/*

===========================================================================
This snippet was written by Erwin S. Andreasen, 4u2@aabc.dk. You may use
this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://login.dknet.dk/~ea/

If you do not have WWW access, try ftp'ing to login.dknet.dk and examine
the /pub/ea directory.
===========================================================================

  Various administrative utility commands.
  Version: 3 - Last update: January 1996.

  To use these 2 commands you will have to add a filename field to AREA_DATA.
  This value can be found easily in load_area while booting - the filename
  of the current area boot_db is reading from is in the strArea global.

  Since version 2 following was added:

  A rename command which renames a player. Search for do_rename to see
  more info on it.

  A FOR command which executes a command at/on every player/mob/location.  

  Fixes since last release: None.
*/


/* To have VLIST show more than vnum 0 - 9900, change the number below: */ 

#define MAX_SHOW_VNUM   370 /* show only 1 - 100*100 */


/* get the 'short' name of an area (e.g. MIDGAARD, MIRROR etc. */
/* assumes that the filename saved in the AREA_DATA struct is something like midgaard.are */
const char * area_name (AREA_DATA *pArea) {
  static char buffer[65]; /* short filename */
  char  *period;

  assert (pArea != NULL);
	
  //strncpy (buffer, pArea->file_name, 64); /* copy the filename */	
  strncpy (buffer, pArea->name, 64); /* copy the filename */	
  period = strchr (buffer, '.'); /* find the period (midgaard.are) */
  if (period) /* if there was one */
    *period = '\0'; /* terminate the string there (midgaard) */
		
  return buffer;	
}

typedef enum {exit_from, exit_to, exit_both} exit_status;

/* depending on status print > or < or <> between the 2 rooms */
void room_pair (ROOM_INDEX_DATA* left, ROOM_INDEX_DATA* right, exit_status ex, char *buffer) {
  const char *sExit;
	
  switch (ex){
  default:        sExit = "??"; break; /* invalid usage */
  case exit_from: sExit = "< "; break;
  case exit_to:   sExit = " >"; break;
  case exit_both: sExit = "<>"; break;
  }
  sprintf (buffer, 
	   "%5d %-26.26s %s%5d %-26.26s(%-8.8s)\n\r",
	   left->vnum, left->name, 
	   sExit,
	   right->vnum, right->name,
	   area_name(right->area));
}

/* for every exit in 'room' which leads to or from pArea but NOT both, print it */
bool checkexits (ROOM_INDEX_DATA *room, AREA_DATA *pArea, char* buffer) {
  char buf[MAX_STRING_LENGTH];
  int i;
  EXIT_DATA *exit;
  ROOM_INDEX_DATA *to_room;
  bool found = FALSE;
	
  strcpy (buffer, "");
  for (i = 0; i < MAX_DIR; i++){ // SinaC 2003
    exit = room->exit[i];
    if (!exit)
      continue;
    else
      to_room = exit->u1.to_room;
    
    if (to_room)  /* there is something on the other side */
      if ( (room->area == pArea) && (to_room->area != pArea) ){ 
	/* an exit from our area to another area */
	/* check first if it is a two-way exit */
	
	if ( to_room->exit[rev_dir[i]] &&
	     to_room->exit[rev_dir[i]]->u1.to_room == room )
	  room_pair (room,to_room,exit_both,buf); /* <> */
	else
	  room_pair (room,to_room,exit_to,buf); /* > */
	
	found = TRUE;
	strcat (buffer, buf);				
      }			
      else			
	if ( (room->area != pArea) && (exit->u1.to_room->area == pArea) ){ 
	  /* an exit from another area to our area */
	  
	  if  (!(to_room->exit[rev_dir[i]] &&
		 to_room->exit[rev_dir[i]]->u1.to_room == room )
	       ){
	    /* two-way exits are handled in the other if */
	    room_pair (to_room,room,exit_from,buf);
	    found = TRUE;
	    strcat (buffer, buf);
	  }
	} /* if room->area */
  } /* for */
  return found;
}

/* for now, no arguments, just list the current area */
void do_exlist (CHAR_DATA *ch, const char * argument) {
  AREA_DATA* pArea;
  ROOM_INDEX_DATA* room;
  int i;
  char buffer[MAX_STRING_LENGTH];
  bool found = FALSE;

  pArea = ch->in_room->area; /* this is the area we want info on */
  for (i = 0; i < MAX_KEY_HASH; i++) /* room index hash table */
    for (room = room_index_hash[i]; room != NULL; room = room->next){
      /* run through all the rooms on the MUD */
      if ( checkexits(room, pArea, buffer) )
	found = TRUE;
      send_to_char (buffer, ch);
    }
  if ( !found )
    send_to_char("No exits found.\n\r", ch );
} 

/* show a list of all used VNUMS */

#define COLUMNS 		5   /* number of columns */
#define MAX_ROW 		((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

void do_vlist (CHAR_DATA *ch, const char *argument) {
  int i,j,vnum;
  ROOM_INDEX_DATA *room;
  char buffer[MAX_ROW*100]; /* should be plenty */
  char buf2 [100];
  BUFFER *output;
	
  output = new_buf();
	
  for (i = 0; i < MAX_ROW; i++){
    strcpy (buffer, ""); /* clear the buffer for this row */
    
    for (j = 0; j < COLUMNS; j++){ /* for each column */
      vnum = ((j*MAX_ROW) + i); /* find a vnum whih should be there */
      if (vnum < MAX_SHOW_VNUM){
	/* each zone has to have a XXX01 room */
	room = get_room_index (vnum * 100 + 1);
	sprintf (buf2, "%3d %-8.8s  ", vnum, 
		 room ? area_name(room->area) : "-" ); 
	/* something there or unused ? */
	strcat (buffer,buf2);				
      } 
    } /* for columns */
    /*
      send_to_char (buffer,ch);
      send_to_char ("\n\r",ch);
    */
    add_buf( output, buffer );
    add_buf( output, "\n\r" );
  } /* for rows */
  
  page_to_char( buf_string( output ), ch );
}

/*
 * do_rename renames a player to another name.
 * PCs only. Previous file is deleted, if it exists.
 * Char is then saved to new file.
 * New name is checked against std. checks, existing offline players and
 * online players. 
 * .gz files are checked for too, just in case.
 */

bool check_parse_name (const char* name);  /* comm.c */
const char *initial( const char *str );    /* comm.c */

void do_rename (CHAR_DATA* ch, const char* argument) {
  char old_name[MAX_INPUT_LENGTH], 
    new_name[MAX_INPUT_LENGTH],
    strsave [MAX_INPUT_LENGTH];

  CHAR_DATA* victim;
  FILE* file;
	
  argument = one_argument(argument, old_name); /* find new/old name */
  one_argument (argument, new_name);
	
  /* Trivial checks */
  if (!old_name[0]){
    send_to_char ("Rename who?\n\r",ch);
    return;
  }
  
  victim = get_char_world (ch, old_name);
	
  if (!victim) {
    send_to_char ("There is no such a person online.\n\r",ch);
    return;
  }
  
  if (IS_NPC(victim)) {   
    send_to_char ("You cannot use Rename on NPCs.\n\r",ch);
    return;
  }

  /* allow rename self new_name,but otherwise only lower level */	
  if ( (victim != ch) && (get_trust (victim) >= get_trust (ch)) ) {
    send_to_char ("You failed.\n\r",ch);
    return;
  }
	
  if (!victim->desc || (victim->desc->connected != CON_PLAYING) ) {
    send_to_char ("This player has lost his link or is inside a pager or the like.\n\r",ch);
    return;
  }
  
  if (!new_name[0]) {
    send_to_char ("Rename to what new name?\n\r",ch);
    return;
  }
	
  /* Insert check for clan here!! */
	
  if (is_clan(victim)) {
    send_to_char ("This player is member of a clan, remove him from there first.\n\r",ch);
    return;
  }
  
  if (!check_parse_name(new_name)){
    send_to_char ("The new name is illegal.\n\r",ch);
    return;
  }
  
  /* First, check if there is a player named that off-line */	
  /*#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( new_name ),
    "/", capitalize( new_name ) );
    #else*/
  //sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( new_name ) );
  sprintf( strsave, "%s%s.new", PLAYER_DIR, capitalize( new_name ) );
  /*#endif*/

  fclose (fpReserve); /* close the reserve file */
  file = fopen (strsave, "r"); /* attempt to to open pfile */
  if (file){
    send_to_char ("A player with that name already exists!\n\r",ch);
    fclose (file);
    fpReserve = fopen( NULL_FILE, "r" ); /* is this really necessary these days? */
    return;		
  }
  fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */
  
  /* Check .gz file ! */
  /*#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s.gz", PLAYER_DIR, initial( new_name ),
    "/", capitalize( new_name ) );
    #else*/
  //sprintf( strsave, "%s%s.gz", PLAYER_DIR, capitalize( new_name ) );
  sprintf( strsave, "%s%s.new.gz", PLAYER_DIR, capitalize( new_name ) );
  /*#endif*/
  
  fclose (fpReserve); /* close the reserve file */
  file = fopen (strsave, "r"); /* attempt to to open pfile */
  if (file) {
    send_to_char ("A player with that name already exists in a compressed file!\n\r",ch);
    fclose (file);
    fpReserve = fopen( NULL_FILE, "r" ); 
    return;		
  }
  fpReserve = fopen( NULL_FILE, "r" );  /* reopen the extra file */
  
  if (get_char_world(ch,new_name)){ /* check for playing level-1 non-saved */
    send_to_char ("A player with the name you specified already exists!\n\r",ch);
    return;
  }

  /* Save the filename of the old name */

  /*#if !defined(machintosh) && !defined(MSDOS)
    sprintf( strsave, "%s%s%s%s", PLAYER_DIR, initial( victim->name ),
    "/", capitalize( victim->name ) );
    #else*/
  sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( victim->name ) );
  /*#endif*/


  /* Rename the character and save him to a new file */
  /* NOTE: Players who are level 1 do NOT get saved under a new name */

  victim->name = str_dup (capitalize(new_name));
  // Added by SinaC 2001
  victim->pcdata->pwd = str_dup("");
	

  //save_char_obj (victim);
  new_save_pFile(victim,FALSE);
	
  /* unlink the old file */
  unlink (strsave); /* unlink does return a value.. but we do not care */
  strcat( strsave, ".new" );
  unlink( strsave );

  /* That's it! */
	
  send_to_char ("Character renamed.\n\r",ch);

  victim->position = POS_STANDING; /* I am laaazy */
  act("$n has renamed you to $N!",ch,NULL,victim,TO_VICT);
  send_to_char("Your password has been removed, don't forget to set a new one.\n\r"
	       "Use password null <new password>\n\r", victim );
			
} /* do_rename */


/* Super-AT command:

FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>


Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/   
const char * name_expand (CHAR_DATA *ch) {
  int count = 1;
  CHAR_DATA *rch;
  /*  HOPEFULLY no mob has a name longer than THAT */
  char name[MAX_INPUT_LENGTH];

  static char outbuf[MAX_INPUT_LENGTH];	
	
  if (!IS_NPC(ch))
    return ch->name;
		
  one_argument (ch->name, name); /* copy the first word into name */
	
  if (!name[0]){ /* weird mob .. no keywords */
    strcpy (outbuf, ""); /* Do not return NULL, just an empty buffer */
    return outbuf;
  }
		
  for (rch = ch->in_room->people; rch && (rch != ch);rch = rch->next_in_room)
    if (is_name (name, rch->name))
      count++;
			

  sprintf (outbuf, "%d.%s", count, name);
  return outbuf;
}

void do_for( CHAR_DATA *ch, const char *argument ) {
  char range[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
  ROOM_INDEX_DATA *room, *old_room;
  CHAR_DATA *p, *p_next;
  int i;
	
  argument = one_argument (argument, range);
	
  if (!range[0] || !argument[0]){ /* invalid usage? */
    do_help (ch, "for");
    return;
  }
  
  if (!str_prefix("quit", argument)){
    send_to_char ("Are you trying to crash the MUD or something?\n\r",ch);
    return;
  }
	
	
  if (!str_cmp (range, "all")){
    fMortals = TRUE;
    fGods = TRUE;
  }
  else if (!str_cmp (range, "gods"))
    fGods = TRUE;
  else if (!str_cmp (range, "mortals"))
    fMortals = TRUE;
  else if (!str_cmp (range, "mobs"))
    fMobs = TRUE;
  else if (!str_cmp (range, "everywhere"))
    fEverywhere = TRUE;
  else
    do_help (ch, "for"); /* show syntax */

  /* do not allow # to make it easier */		
  if (fEverywhere && strchr (argument, '#')){
    send_to_char ("Cannot use FOR EVERYWHERE with the # thingie.\n\r",ch);
    return;
  }
		
  if (strchr (argument, '#')){ /* replace # ? */
    for (p = char_list; p ; p = p_next){
      /* In case someone DOES try to AT MOBS SLAY # */
      p_next = p->next;
      found = FALSE;
      
      if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
	continue;
      
      if (IS_NPC(p) && fMobs)
	found = TRUE;
      else if (!IS_NPC(p) && IS_IMMORTAL(p) && fGods)
	found = TRUE;
      else if (!IS_NPC(p) && !IS_IMMORTAL(p) && fMortals)
	found = TRUE;
      
      /* It looks ugly to me.. but it works :) */				
      if (found){ /* p is 'appropriate' */
	const char *pSource = argument; /* head of buffer to be parsed */
	char *pDest = buf; /* parse into this */
	
	while (*pSource){
	  if (*pSource == '#'){ /* Replace # with name of target */
	    const char *namebuf = name_expand (p);
	    
	    if (namebuf) /* in case there is no mob name ?? */
	      while (*namebuf) /* copy name over */
		*(pDest++) = *(namebuf++);
	    
	    pSource++;
	  }
	  else
	    *(pDest++) = *(pSource++);
	} /* while */
	*pDest = '\0'; /* Terminate */
	
				/* Execute */
	old_room = ch->in_room;
	char_from_room (ch);
	char_to_room (ch,p->in_room);
	interpret (ch, buf);
	char_from_room (ch);
	char_to_room (ch,old_room);
	
      } /* if found */
    } /* for every char */
  }
  else{ /* just for every room with the appropriate people in it */
    for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
      for (room = room_index_hash[i] ; room ; room = room->next){
	found = FALSE;
	
				/* Anyone in here at all? */
	if (fEverywhere) /* Everywhere executes always */
	  found = TRUE;
	else if (!room->people) /* Skip it if room is empty */
	  continue;
	
	
	/* Check if there is anyone here of the requried type */
	/* Stop as soon as a match is found or there are no more ppl in room */
	for (p = room->people; p && !found; p = p->next_in_room){
	  
	  if (p == ch) /* do not execute on oneself */
	    continue;
	  
	  if (IS_NPC(p) && fMobs)
	    found = TRUE;
	  else if (!IS_NPC(p) && IS_IMMORTAL(p) && fGods)
	    found = TRUE;
	  else if (!IS_NPC(p) && !IS_IMMORTAL(p) && fMortals)
	    found = TRUE;
	} /* for everyone inside the room */
	/* Any of the required type here AND room not private? */
	if (found && !room_is_private(room)){
	  /* This may be ineffective. Consider moving character out of old_room
	     once at beginning of command then moving back at the end.
	     This however, is more safe?
	  */
	  
	  old_room = ch->in_room;
	  char_from_room (ch);
	  char_to_room (ch, room);
	  interpret (ch, argument);
	  char_from_room (ch);
	  char_to_room (ch, old_room);
	} /* if found */
      } /* for every room in a bucket */
  } /* if strchr */
} /* do_for */
