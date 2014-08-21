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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

// Added by SinaC 2001
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "olc_value.h"
#include "interp.h"


char *const distance[4]=
{
  "right here.", "nearby to the %s.", "not far %s.", "off in the distance %s."
};

void scan_list           args((ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch,
                               int depth, int door));
void scan_char           args((CHAR_DATA *victim, CHAR_DATA *ch,
                               int depth, int door));
void do_scan_farsight(CHAR_DATA *ch, const char *argument) {
  char arg1[MAX_INPUT_LENGTH], buf[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *scan_room;
  EXIT_DATA *pExit;
  int door, depth;

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0') {
    act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
    send_to_char("Looking around you see:\n\r", ch);
    scan_list(ch->in_room, ch, 0, -1);

    for (door=0;door<MAX_DIR;door++) { // Modified by SinaC 2003
      if ((pExit = ch ->in_room->exit[door]) != NULL)
	scan_list(pExit->u1.to_room, ch, 1, door);
    }
    return;
  }
  // Modified by SinaC 2003
  else if (!str_cmp(arg1, "n") || !str_cmp(arg1, "north")) door = DIR_NORTH;
  else if (!str_cmp(arg1, "e") || !str_cmp(arg1, "east"))  door = DIR_EAST;
  else if (!str_cmp(arg1, "s") || !str_cmp(arg1, "south")) door = DIR_SOUTH;
  else if (!str_cmp(arg1, "w") || !str_cmp(arg1, "west"))  door = DIR_WEST;
  else if (!str_cmp(arg1, "u") || !str_cmp(arg1, "up" ))   door = DIR_UP;
  else if (!str_cmp(arg1, "d") || !str_cmp(arg1, "down"))  door = DIR_DOWN;
  // Added by SinaC 2003
  else if ( !str_cmp( arg1, "ne" ) || !str_cmp( arg1, "northeast"  ) ) door = DIR_NORTHEAST;
  else if ( !str_cmp( arg1, "nw" ) || !str_cmp( arg1, "northwest"  ) ) door = DIR_NORTHWEST;
  else if ( !str_cmp( arg1, "se" ) || !str_cmp( arg1, "southeast"  ) ) door = DIR_SOUTHEAST;
  else if ( !str_cmp( arg1, "sw" ) || !str_cmp( arg1, "southwest"  ) ) door = DIR_SOUTHWEST;
  else { send_to_char("Which way do you want to scan?\n\r", ch); return; }

  act("You peer intently $T.", ch, NULL, dir_name[door], TO_CHAR);
  act("$n peers intently $T.", ch, NULL, dir_name[door], TO_ROOM);
  sprintf(buf, "Looking %s you see:\n\r", dir_name[door]);
                                                                                  
  scan_room = ch->in_room;

  for (depth = 1; depth < 4; depth++) {
    if ((pExit = scan_room->exit[door]) != NULL) {
      scan_room = pExit->u1.to_room;
      scan_list(pExit->u1.to_room, ch, depth, door);
    }
  }
  return;
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, int depth,
               int door)
{
  CHAR_DATA *rch;

  /*    SinaC 2000
   * this used to cause a mysterious crash here, finally realized it was
   * 'door' being -1, and rev_dir seems to have a problem with that...
   * only acted up when it was done in a room with "extra" exits - Mull
   */
  if ( door != -1 && scan_room->exit[rev_dir[door]] != NULL
       && IS_SET(scan_room->exit[rev_dir[door]]->exit_info,EX_CLOSED) )
    return;

  if (scan_room == NULL) return;
  for (rch=scan_room->people; rch != NULL; rch=rch->next_in_room) {
    if (rch == ch) continue;
    if (!IS_NPC(rch) && rch->invis_level > get_trust(ch)) continue;
    if (can_see(ch, rch)) scan_char(rch, ch, depth, door);
  }
  return;
}

void scan_char(CHAR_DATA *victim, CHAR_DATA *ch, int depth, int door) {
  char buf[MAX_INPUT_LENGTH], buf2[MAX_INPUT_LENGTH];

  buf[0] = '\0';

  strcat(buf, PERS(victim, ch));
  strcat(buf, ", ");
  sprintf(buf2, distance[depth], dir_name[door]);
  strcat(buf, buf2);
  strcat(buf, "\n\r");
 
  send_to_char(buf, ch);
  return;
}

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

Note that function prototypes are not included in this code - remember to add
them to merc.h yourself.

The classic SCAN command, shows the mobs surrounding the character.

Last update: Sep 18, 1995

Should work on : MERC2.2 ROM2.3

Fixed since last update:
 ch/target were reversed in the call to can_see

Know bugs and limitations yet to be fixed:
 Blind character should get another message
 Dark rooms can be scanned into?

Comments:
 For ROM2.3, replace to_room with u1.to_room
 It's primitive, but also some of my first work! :)

*/

/* 
 * returns everything the character can see at that location in buf
 * returns number of creatures seen 
 */

int scan_room (CHAR_DATA *ch, const ROOM_INDEX_DATA *room,char *buf)
{
  CHAR_DATA *target = room->people;
  int number_found = 0;

  while (target != NULL) {/* repeat as long more peple in the room */
    if (can_see(ch,target)) {/* show only if the character can see the target */
      strcat (buf, " - ");
      strcat (buf, IS_NPC(target) ? target->short_descr : target->name);
      strcat (buf, "\n\r");
      number_found++;
    }
    target = target->next_in_room;
  }

  return number_found;
}

void do_scan (CHAR_DATA *ch, const char *argument)
{
  EXIT_DATA * pexit;
  ROOM_INDEX_DATA * room;
  char buf[MAX_STRING_LENGTH];
  int dir;
  int distance;

  // Modified by SinaC 2001
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSCAN )
       && !IS_IMMORTAL( ch )  ) {
    send_to_char( "Your vision is clouded by a mysterious force.\n\r", ch );
    return;
  }

  sprintf (buf, "Right here you see:\n\r");
  if (scan_room(ch,ch->in_room,buf) == 0)
    strcat (buf, "Noone\n\r");
  send_to_char (buf,ch);

  for (dir = 0; dir < MAX_DIR; dir++) {/* look in every direction  Modified by SinaC 2003*/
    room = ch->in_room; /* starting point */

    for (distance = 1 ; distance < 4; distance++) {
      pexit = room->exit[dir]; /* find the door to the next room */
      if ((pexit == NULL) || (pexit->u1.to_room == NULL) || (IS_SET(pexit->exit_info, EX_CLOSED)))
	break; /* exit not there OR points to nothing OR is closed */

      /* char can see the room */
      sprintf (buf, "{c%d {r%s{x from here you see:\n\r", distance, dir_name[dir]);
      if (scan_room(ch,pexit->u1.to_room,buf)) /* if there is something there */
	send_to_char (buf,ch);

      room = pexit->u1.to_room; /* go to the next room */
    } /* for distance */
  } /* for dir */
}
