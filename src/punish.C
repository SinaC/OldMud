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
#include "tables.h"
#include "lookup.h"
#include "act_comm.h"
#include "handler.h"
#include "save.h"
#include "fight.h"
#include "comm.h"
#include "db.h"
#include "olc_value.h"
#include "wiznet.h"
#include "interp.h"


/* RT nochannels command, for those spammers */
void do_nochannels( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
 
  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Nochannel whom?\n\r", ch );
    return;
  }
 
  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
 
  if ( get_trust( victim ) >= get_trust( ch ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }
 
  if ( IS_SET(victim->comm, COMM_NOCHANNELS) ) {
    REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
    send_to_char( "The gods have restored your channel priviliges.\n\r", 
		  victim );
    send_to_char( "NOCHANNELS removed.\n\r", ch );
    sprintf(buf,"$N restores channels to %s",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else {
    SET_BIT(victim->comm, COMM_NOCHANNELS);
    send_to_char( "The gods have revoked your channel priviliges.\n\r", 
		  victim );
    send_to_char( "NOCHANNELS set.\n\r", ch );
    sprintf(buf,"$N revokes %s's channels.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
 
  return;
}

void do_deny( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Deny whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  SET_BIT(victim->act, PLR_DENY);
  send_to_char( "You are denied access!\n\r", victim );
  sprintf(buf,"$N denies access to %s",victim->name);
  wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  send_to_char( "OK.\n\r", ch );
  //save_char_obj(victim);
  new_save_pFile(victim,TRUE);
  stop_fighting(victim,TRUE);
  do_quit( victim, "" );

  return;
}

void do_disconnect( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Disconnect whom?\n\r", ch );
    return;
  }

  if (is_number(arg)) {
    int desc;

    desc = atoi(arg);
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      if ( d->descriptor == desc ) {
	close_socket( d );
	send_to_char( "Ok.\n\r", ch );
	return;
      }
    }
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->desc == NULL ) {
    act( "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
    return;
  }

  for ( d = descriptor_list; d != NULL; d = d->next ) {
    if ( d == victim->desc ) {
      close_socket( d );
      send_to_char( "Ok.\n\r", ch );
      return;
    }
  }

  bug( "Do_disconnect: desc not found." );
  send_to_char( "Descriptor not found!\n\r", ch );
  return;
}

void do_protect( CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;

  if (argument[0] == '\0') {
    send_to_char("Protect whom from snooping?\n\r",ch);
    return;
  }

  if ((victim = get_char_world(ch,argument)) == NULL) {
    send_to_char("You can't find them.\n\r",ch);
    return;
  }

  if (IS_SET(victim->comm,COMM_SNOOP_PROOF)) {
    act_new("$N is no longer snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
    send_to_char("Your snoop-proofing was just removed.\n\r",victim);
    REMOVE_BIT(victim->comm,COMM_SNOOP_PROOF);
  }
  else {
    act_new("$N is now snoop-proof.",ch,NULL,victim,TO_CHAR,POS_DEAD);
    send_to_char("You are now immune to snooping.\n\r",victim);
    SET_BIT(victim->comm,COMM_SNOOP_PROOF);
  }
}

void do_snoop( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Snoop whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->desc == NULL ) {
    send_to_char( "No descriptor to snoop.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( "Cancelling all snoops.\n\r", ch );
    wiznet("$N stops being such a snoop.",
	   ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      if ( d->snoop_by == ch->desc )
	d->snoop_by = NULL;
    }
    return;
  }

  if ( victim->desc->snoop_by != NULL ) {
    send_to_char( "Busy already.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
      &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
    send_to_char("That character is in a private room.\n\r",ch);
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) 
       ||   IS_SET(victim->comm,COMM_SNOOP_PROOF)) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( ch->desc != NULL ) {
    for ( d = ch->desc->snoop_by; d != NULL; d = d->snoop_by ) {
      if ( d->character == victim || d->original == victim ) {
	send_to_char( "No snoop loops.\n\r", ch );
	return;
      }
    }
  }

  victim->desc->snoop_by = ch->desc;
  sprintf(buf,"$N starts snooping on %s",
	  (IS_NPC(ch) ? victim->short_descr : victim->name));
  wiznet(buf,ch,NULL,WIZ_SNOOPS,WIZ_SECURE,get_trust(ch));
  send_to_char( "Ok.\n\r", ch );
  return;
}
 	
void do_freeze( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Freeze whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->act, PLR_FREEZE) ) {
    REMOVE_BIT(victim->act, PLR_FREEZE);
    send_to_char( "You can play again.\n\r", victim );
    send_to_char( "FREEZE removed.\n\r", ch );
    sprintf(buf,"$N thaws %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else {
    SET_BIT(victim->act, PLR_FREEZE);
    send_to_char( "You can't do ANYthing!\n\r", victim );
    send_to_char( "FREEZE set.\n\r", ch );
    sprintf(buf,"$N puts %s in the deep freeze.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  //save_char_obj( victim );
  new_save_pFile(victim,TRUE);

  return;
}

void do_log( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Log whom?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) ) {
    if ( fLogAll ) {
      fLogAll = FALSE;
      send_to_char( "Log ALL off.\n\r", ch );
    }
    else {
      fLogAll = TRUE;
      send_to_char( "Log ALL on.\n\r", ch );
    }
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  /*
   * No level check, gods can log anyone.
   */
  if ( IS_SET(victim->act, PLR_LOG) ) {
    REMOVE_BIT(victim->act, PLR_LOG);
    send_to_char( "LOG removed.\n\r", ch );
  }
  else {
    SET_BIT(victim->act, PLR_LOG);
    send_to_char( "LOG set.\n\r", ch );
  }

  return;
}

void do_noemote( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Noemote whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->comm, COMM_NOEMOTE) ) {
    REMOVE_BIT(victim->comm, COMM_NOEMOTE);
    send_to_char( "You can emote again.\n\r", victim );
    send_to_char( "NOEMOTE removed.\n\r", ch );
    sprintf(buf,"$N restores emotes to %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else {
    SET_BIT(victim->comm, COMM_NOEMOTE);
    send_to_char( "You can't emote!\n\r", victim );
    send_to_char( "NOEMOTE set.\n\r", ch );
    sprintf(buf,"$N revokes %s's emotes.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}

void do_noshout( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Noshout whom?\n\r",ch);
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->comm, COMM_NOSHOUT) ) {
    REMOVE_BIT(victim->comm, COMM_NOSHOUT);
    send_to_char( "You can shout again.\n\r", victim );
    send_to_char( "NOSHOUT removed.\n\r", ch );
    sprintf(buf,"$N restores shouts to %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else {
    SET_BIT(victim->comm, COMM_NOSHOUT);
    send_to_char( "You can't shout!\n\r", victim );
    send_to_char( "NOSHOUT set.\n\r", ch );
    sprintf(buf,"$N revokes %s's shouts.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}

void do_notell( CHAR_DATA *ch,const  char *argument )
{
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( IS_NPC( ch ) ) {
    send_to_char("Mobiles can't use this command!\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' ) {
    send_to_char( "Notell whom?", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( get_trust( victim ) >= get_trust( ch ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  if ( IS_SET(victim->comm, COMM_NOTELL) ) {
    REMOVE_BIT(victim->comm, COMM_NOTELL);
    send_to_char( "You can tell again.\n\r", victim );
    send_to_char( "NOTELL removed.\n\r", ch );
    sprintf(buf,"$N restores tells to %s.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }
  else {
    SET_BIT(victim->comm, COMM_NOTELL);
    send_to_char( "You can't tell!\n\r", victim );
    send_to_char( "NOTELL set.\n\r", ch );
    sprintf(buf,"$N revokes %s's tells.",victim->name);
    wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
  }

  return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' ) {
    send_to_char( "Force whom to do what?\n\r", ch );
    return;
  }

  one_argument(argument,arg2);
  
  if (!str_cmp(arg2,"delete")) {
    send_to_char("That will NOT be done.\n\r",ch);
    return;
  }

  sprintf( buf, "$n forces you to '%s'.", argument );

  if ( !str_cmp( arg, "all" ) ) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if (get_trust(ch) < MAX_LEVEL - 3) {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    for ( vch = char_list; vch != NULL; vch = vch_next ) {
      vch_next = vch->next;

      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) ) {
	act( buf, ch, NULL, vch, TO_VICT );
	interpret( vch, argument );
      }
    }
  }
  else if (!str_cmp(arg,"players")) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
 
    if (get_trust(ch) < MAX_LEVEL - 2) {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }
 
    for ( vch = char_list; vch != NULL; vch = vch_next ) {
      vch_next = vch->next;
 
      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch ) 
	   &&	 vch->level < LEVEL_HERO) {
	act( buf, ch, NULL, vch, TO_VICT );
	interpret( vch, argument );
      }
    }
  }
  else if (!str_cmp(arg,"gods")) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
 
    if (get_trust(ch) < MAX_LEVEL - 2) {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }
 
    for ( vch = char_list; vch != NULL; vch = vch_next ) {
      vch_next = vch->next;
 
      if ( !IS_NPC(vch) && get_trust( vch ) < get_trust( ch )
	   &&   vch->level >= LEVEL_HERO) {
	act( buf, ch, NULL, vch, TO_VICT );
	interpret( vch, argument );
      }
    }
  }
  else {
    CHAR_DATA *victim;

    if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch ) {
      send_to_char( "Aye aye, right away!\n\r", ch );
      return;
    }

    if (!is_room_owner(ch,victim->in_room) 
	&&  ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
      send_to_char("That character is in a private room.\n\r",ch);
      return;
    }

    if ( get_trust( victim ) >= get_trust( ch ) ) {
      send_to_char( "Do it yourself!\n\r", ch );
      return;
    }

    if ( !IS_NPC(victim) && get_trust(ch) < MAX_LEVEL -3) {
      send_to_char("Not at your level!\n\r",ch);
      return;
    }

    act( buf, ch, NULL, victim, TO_VICT );
    interpret( victim, argument );
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}
