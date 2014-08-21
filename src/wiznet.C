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
#include "wiznet.h"
#include "comm.h"
#include "olc_value.h"
#include "lookup.h"
#include "handler.h"
#include "db.h"

/*
 * Local functions.
 */
void do_wiznet( CHAR_DATA *ch, const char *argument ) {
  int flag;
  char buf[MAX_STRING_LENGTH];

  if ( argument[0] == '\0' ){
    if (IS_SET(ch->wiznet,WIZ_ON)){
      send_to_char("Signing off of Wiznet.\n\r",ch);
      REMOVE_BIT(ch->wiznet,WIZ_ON);
    }
    else{
      send_to_char("Welcome to Wiznet!\n\r",ch);
      SET_BIT(ch->wiznet,WIZ_ON);
    }
    return;
  }

  if (!str_prefix(argument,"on")) {
    send_to_char("Welcome to Wiznet!\n\r",ch);
    SET_BIT(ch->wiznet,WIZ_ON);
    return;
  }

  if (!str_prefix(argument,"off")){
    send_to_char("Signing off of Wiznet.\n\r",ch);
    REMOVE_BIT(ch->wiznet,WIZ_ON);
    return;
  }

  /* show wiznet status */
  if (!str_prefix(argument,"status")){
    buf[0] = '\0';

    if (!IS_SET(ch->wiznet,WIZ_ON))
      strcat(buf,"off ");

    for (flag = 0; wiznet_table[flag].name != NULL; flag++)
      if (IS_SET(ch->wiznet,wiznet_table[flag].flag)){
	strcat(buf,wiznet_table[flag].name);
	strcat(buf," ");
      }
    
    strcat(buf,"\n\r");

    send_to_char("Wiznet status:\n\r",ch);
    send_to_char(buf,ch);
    return;
  }

  if (!str_prefix(argument,"show")){
    /* list of all wiznet options */
    buf[0] = '\0';

    for (flag = 0; wiznet_table[flag].name != NULL; flag++){
      if (wiznet_table[flag].level <= get_trust(ch)){
	strcat(buf,wiznet_table[flag].name);
	strcat(buf," ");
      }
    }

    strcat(buf,"\n\r");

    send_to_char("Wiznet options available to you are:\n\r",ch);
    send_to_char(buf,ch);
    return;
  }
   
  flag = wiznet_lookup(argument);

  if (flag == -1 || get_trust(ch) < wiznet_table[flag].level){
    send_to_char("No such option.\n\r",ch);
    return;
  }
   
  if (IS_SET(ch->wiznet,wiznet_table[flag].flag)){
    sprintf(buf,"You will no longer see %s on wiznet.\n\r",
	    wiznet_table[flag].name);
    send_to_char(buf,ch);
    REMOVE_BIT(ch->wiznet,wiznet_table[flag].flag);
    return;
  }
  else {
    sprintf(buf,"You will now see %s on wiznet.\n\r",
	    wiznet_table[flag].name);
    send_to_char(buf,ch);
    SET_BIT(ch->wiznet,wiznet_table[flag].flag);
    return;
  }

}

void wiznet(const char *string, CHAR_DATA *ch, OBJ_DATA *obj,
	    long flag, long flag_skip, int min_level) {
  DESCRIPTOR_DATA *d;

  for ( d = descriptor_list; d != NULL; d = d->next ) {
    if (d->connected == CON_PLAYING
	&&  IS_IMMORTAL(d->character) 
	&&  IS_SET(d->character->wiznet,WIZ_ON) 
	&&  (!flag || IS_SET(d->character->wiznet,flag))
	&&  (!flag_skip || !IS_SET(d->character->wiznet,flag_skip))
	&&  get_trust(d->character) >= min_level
	&&  d->character != ch
	// Added by SinaC 2003, avoid receiving message when using string editor
	&&  d->pString == NULL ) {
      if (IS_SET(d->character->wiznet,WIZ_PREFIX))
	send_to_char("--> ",d->character);
      act_new(string,d->character,obj,ch,TO_CHAR,POS_DEAD);
    }
  }
 
  return;
}
