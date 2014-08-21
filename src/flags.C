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
#include "tables.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "olc_value.h"
#include "interp.h"
#include "bit.h"
#include "lookup.h"

void do_flag(CHAR_DATA *ch, const char *argument) {
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH],arg3[MAX_INPUT_LENGTH];
  char word[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  long *flag = NULL, old = 0, newflag = 0, marked = 0, pos;
  char type;
  struct flag_type *flag_table;

  argument = one_argument(argument,arg1);
  argument = one_argument(argument,arg2);
  argument = one_argument(argument,arg3);

  type = argument[0];

  if (type == '=' || type == '-' || type == '+')
    argument = one_argument(argument,word);

  if (arg1[0] == '\0') {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  flag mob  <name> <field> [operator] <flags>\n\r",ch);
    send_to_char("  flag char <name> <field> [operator] <flags>\n\r",ch);
    send_to_char("  mob  flags: act,aff,aff2,off,imm,res,vuln,form,part\n\r",ch);
    send_to_char("  char flags: plr,comm,aff,aff2,imm,res,vuln\n\r",ch);
    send_to_char("  operator  +: add flag, -: remove flag, = set equal to\n\r",ch);
    send_to_char("  otherwise flag toggles the flags listed.\n\r",ch);
    return;
  }
  if (arg2[0] == '\0') {
    send_to_char("What do you wish to set flags on?\n\r",ch);
    return;
  }
  if (arg3[0] == '\0') {
    send_to_char("You need to specify a flag to set.\n\r",ch);
    return;
  }
  if (argument[0] == '\0') {
    send_to_char("Which flags do you wish to change?\n\r",ch);
    return;
  }

  if (!str_prefix(arg1,"mob") || !str_prefix(arg1,"char")) {
    victim = get_char_world(ch,arg2);
    if (victim == NULL) {
      send_to_char("You can't find them.\n\r",ch);
      return;
    }

    // select a flag to set
    if (!str_prefix(arg3,"act")) {
      if (!IS_NPC(victim)) {
	send_to_char("Use plr for PCs.\n\r",ch);
	return;
      }

      flag = &victim->act;
      flag_table = act_flags;
    }
    else if (!str_prefix(arg3,"plr")) {
      if (IS_NPC(victim)) {
	send_to_char("Use act for NPCs.\n\r",ch);
	return;
      }

      flag = &victim->act;
      flag_table = plr_flags;
    }
    else if (!str_prefix(arg3,"aff")) {
      flag = &victim->bstat(affected_by);
      flag_table = affect_flags;
    }
    else if (!str_prefix(arg3,"off")) { // SinaC 2003
      flag = &victim->off_flags;
      flag_table = off_flags;
    }
    // Added by SinaC 2001
    else if (!str_prefix(arg3,"aff2")) {
      flag = &victim->bstat(affected2_by);
      flag_table = affect2_flags;
    }

    else if (!str_prefix(arg3,"imm")) {
      flag = &victim->bstat(imm_flags);
      flag_table = irv_flags;
    }
    else if (!str_prefix(arg3,"resist")) {
      flag = &victim->bstat(res_flags);
      flag_table = irv_flags;
    }
    else if (!str_prefix(arg3,"vuln")) {
      flag = &victim->bstat(vuln_flags);
      flag_table = irv_flags;
    }
    else if (!str_prefix(arg3,"form")) {
      //      if (!IS_NPC(victim)) {
      //	send_to_char("Form can't be set on PCs.\n\r",ch);
      //	return;
      //      }

      flag = &victim->bstat(form);
      flag_table = form_flags;
    }
    else if (!str_prefix(arg3,"parts")) {
      //      if (!IS_NPC(victim)) {
      //	send_to_char("Parts can't be set on PCs.\n\r",ch);
      //	return;
      //      }

      flag = &victim->bstat(parts);
      flag_table = part_flags;
    }
    else if (!str_prefix(arg3,"comm")) {
      if (IS_NPC(victim)) {
	send_to_char("Comm can't be set on NPCs.\n\r",ch);
	return;
      }

      flag = &victim->comm;
      flag_table = comm_flags;
    }
    else {
      send_to_char("That's not an acceptable flag.\n\r",ch);
      return;
    }

    if (flag)
      old = *flag;
    victim->zone = NULL;

    if (type != '=')
      newflag = old;

    // mark the words
    for (; ;) {
      argument = one_argument(argument,word);

      if (word[0] == '\0')
	break;

      pos = flag_lookup(word,flag_table);
      if (pos == 0) {
	send_to_charf(ch, "That flag doesn't exist: %s!\n\r",word);
	return;
      }
      else
	SET_BIT(marked,pos);
    }

    for (pos = 0; flag_table[pos].name != NULL; pos++) {
      if (!flag_table[pos].settable && IS_SET(old,flag_table[pos].bit)) {
	SET_BIT(newflag,flag_table[pos].bit);
	continue;
      }

      if (IS_SET(marked,flag_table[pos].bit)) {
	switch(type) {
	case '=':
	case '+':
	  SET_BIT(newflag,flag_table[pos].bit);
	  break;
	case '-':
	  REMOVE_BIT(newflag,flag_table[pos].bit);
	  break;
	default:
	  TOGGLE_BIT(newflag,flag_table[pos].bit);
	  //	  if (IS_SET(newflag,flag_table[pos].bit))
	  //	    REMOVE_BIT(newflag,flag_table[pos].bit);
	  //	  else
	  //	    SET_BIT(newflag,flag_table[pos].bit);
	  break;
	}
      }
    }
    *flag = newflag;
    return;
  }
}
