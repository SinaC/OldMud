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
 **************************************************************************/

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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "classes.h"
#include "act_comm.h"
#include "comm.h"
#include "db.h"
#include "spells_def.h"
#include "clan.h"
#include "fight.h"
#include "handler.h"
#include "save.h"
#include "gsn.h"
#include "olc_value.h"
#include "language.h"
#include "names.h"
#include "wiznet.h"
#include "interp.h"
#include "config.h"
#include "update.h"
#include "utils.h"
#include "arena.h"


// Added by SinaC 2000 for talking drunk
const char * makedrunk (const char *string, CHAR_DATA * ch);

/* RT code to delete yourself */
void do_delet( CHAR_DATA *ch, const char *argument) {
  send_to_char("You must type the full command to delete yourself.\n\r",ch);
}

void do_delete( CHAR_DATA *ch, const char *argument) {
  char strsave[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;
  
  if (ch->pcdata->confirm_delete) {
    if (argument[0] != '\0') {
      send_to_char("Delete status removed.\n\r",ch);
      ch->pcdata->confirm_delete = FALSE;
      return;
    }
    else {
      sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( ch->name ) );
      wiznet("$N turns $Mself into line noise.",ch,NULL,0,0,0);
      stop_fighting(ch,TRUE);
      // remove the player from the clan list, if he's in a clan
      if ( is_clan(ch) ) {
	remove_clan_member(&get_clan_table(ch->clan)->members,ch->name);
	ch->clan = 0;
	ch->clan_status = 0;
      }

      do_quit(ch,"");
      unlink(strsave);
      sprintf( strsave, "%s%s.new", PLAYER_DIR, capitalize( ch->name ) ); // SinaC 2003
      unlink(strsave);
      return;
    }
  }

  if (argument[0] != '\0') {
    send_to_char("Just type delete. No argument.\n\r",ch);
    return;
  }

  send_to_char("Type delete again to confirm this command.\n\r",ch);
  send_to_char("WARNING: this command is irreversible.\n\r",ch);
  send_to_char("Typing delete with an argument will undo delete status.\n\r",
	       ch);
  ch->pcdata->confirm_delete = TRUE;
  wiznet("$N is contemplating deletion.",ch,NULL,0,0,get_trust(ch));
}
	    

/* RT code to display channel status */
void do_channels( CHAR_DATA *ch, const char *argument) {
  char buf[MAX_STRING_LENGTH];

  /* lists all channels and their status */
  send_to_char("   channel     status\n\r",ch);
  send_to_char("---------------------\n\r",ch);

  send_to_char("ooc            ",ch);
  if (!IS_SET(ch->comm,COMM_NOOOC))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);
  // IC added by SinaC 2001
  send_to_char("ic             ",ch);
  if (!IS_SET(ch->comm,COMM_NOIC))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("auction        ",ch);
  if (!IS_SET(ch->comm,COMM_NOAUCTION))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("Q/A            ",ch);
  if (!IS_SET(ch->comm,COMM_NOQUESTION))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("trivia         ",ch);
  if (!IS_SET(ch->comm,COMM_NOTRIVIA))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);
    
  send_to_char("Quote          ",ch);
  if (!IS_SET(ch->comm,COMM_NOQUOTE))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("grats          ",ch);
  if (!IS_SET(ch->comm,COMM_NOGRATS))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);
  if (IS_IMMORTAL(ch)) {
    send_to_char("immtalk        ",ch);
    if(!IS_SET(ch->comm,COMM_NOWIZ))
      send_to_char("ON\n\r",ch);
    else
      send_to_char("OFF\n\r",ch);
  }

  send_to_char("shouts         ",ch);
  if (!IS_SET(ch->comm,COMM_SHOUTSOFF))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("tells          ",ch);
  if (!IS_SET(ch->comm,COMM_DEAF))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("quiet mode     ",ch);
  if (IS_SET(ch->comm,COMM_QUIET))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  if (IS_SET(ch->comm,COMM_AFK))
    send_to_char("You are AFK.\n\r",ch);
  // Added by SinaC 2000
  if (IS_SET(ch->comm,COMM_BUILDING))
    send_to_char("You are BUILDING.\n\r",ch);
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are EDITING.\n\r",ch);

  if (IS_SET(ch->comm,COMM_SNOOP_PROOF))
    send_to_char("You are immune to snooping.\n\r",ch);
   
  if (ch->lines != PAGELEN) {
    if (ch->lines) {
      sprintf(buf,"You display %d lines of scroll.\n\r",ch->lines+2);
      send_to_char(buf,ch);
    }
    else
      send_to_char("Scroll buffering is off.\n\r",ch);
  }

  if (ch->prompt != NULL) {
    sprintf(buf,"Your current prompt is: %s\n\r",ch->prompt);
    send_to_char(buf,ch);
  }

  if (IS_SET(ch->comm,COMM_NOSHOUT))
    send_to_char("You cannot shout.\n\r",ch);
  
  if (IS_SET(ch->comm,COMM_NOTELL))
    send_to_char("You cannot use tell.\n\r",ch);
 
  if (IS_SET(ch->comm,COMM_NOCHANNELS))
    send_to_char("You cannot use channels.\n\r",ch);

  if (IS_SET(ch->comm,COMM_NOEMOTE))
    send_to_char("You cannot show emotions.\n\r",ch);

}

/* RT deaf blocks out all shouts */
void do_deaf( CHAR_DATA *ch, const char *argument) {
    
  if (IS_SET(ch->comm,COMM_DEAF)) {
    send_to_char("You can now hear tells again.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_DEAF);
  }
  else {
    send_to_char("From now on, you won't hear tells.\n\r",ch);
    SET_BIT(ch->comm,COMM_DEAF);
  }
}

/* RT quiet blocks out all communication */
void do_quiet ( CHAR_DATA *ch, const char * argument) {
  if (IS_SET(ch->comm,COMM_QUIET)) {
    send_to_char("Quiet mode removed.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_QUIET);
  }
  else {
    send_to_char("From now on, you will only hear says and emotes.\n\r",ch);
    SET_BIT(ch->comm,COMM_QUIET);
  }
}


/* RT auction BET function */
/* RY 1995 AUC ROM */
void do_bet( CHAR_DATA *ch, const char *argument ) {
  DESCRIPTOR_DATA *d;
  int bet=atoi(argument);
  char buf[MAX_STRING_LENGTH]; 

  if (IS_NPC(ch)) {
    send_to_char("Mobiles can't bet/auction!\n\r",ch);
    return;
  }

  if(!aucstat) {
    send_to_char("No auction running.\n\r",ch);
    return;
  } 

  if(!bet) {
    /* id the article */
    // Modified by SinaC 2001
    spell_identify(1,1,ch,aucobj,0, 1 );  /* cast an id spell */
    return;
  }

  if(IS_IMMORTAL(ch)) {
    send_to_char("immortals shouldn't use auction.\n\r", ch);
    return;
  }

  if(ch==aucfrom) {
    send_to_char("It's YOUR auction..\n\r",ch);
    return;
  }

  if (lastbetter==ch) {
    /* By Oxtal; that ensures that the test
       ch->silver + 100 * ch->gold) < bet
       is correct
    */
    send_to_char("Calm down, for now the auction is virtually yours!\n\r",ch);
    return;
  }

  if(bet<(lastbet+100)) {
    sprintf(buf,"You have to beat the current bet by at least 100 silver coins\n\r" \
	    "Current bet on %s is now %d\n\r",aucobj->short_descr,lastbet);

    send_to_char(buf,ch);
    return;
  }

  /*   if(ch->silver<bet) */ /* Removed by Seytan 1997 */
  if ( (ch->silver + 100 * ch->gold) < bet )  {
    send_to_char("You don't have that many silver pieces.\n\r",ch);
    return;
  }

  /* Debug by Oxtal */
  if (lastbetter!=NULL) {
    /* Give back his money to the last better */
    lastbetter->silver+=betsilver;
    lastbetter->gold+=betgold;
  }
  /* Get the money NOW! Or he can deposit it, give it etc...*/
  betsilver = ch->silver;
  betgold = ch->gold;
  deduct_cost(ch,bet);
  betsilver -= ch->silver;
  betgold -= ch->gold;

  sprintf(buf,"{mA bet of %d has been received on %s{x\n\r",
	  bet,aucobj->short_descr);

  for ( d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *victim;

    victim = d->original ? d->original : d->character;

    if ( d->connected == CON_PLAYING 
	 && !IS_SET(victim->comm,COMM_NOAUCTION) 
	 && !IS_SET(victim->comm,COMM_QUIET) ) {
      /*
	act_new( "{m$n bets '$t'{x",
	ch,argument, d->character, TO_VICT,POS_SLEEPING );
      */
      send_to_char( buf, victim );
    }
  }


  /* Adjust values for this bet */
  lastbet    = bet;
  lastbetter = ch;
  aucstat    = 1;  /* back to first phase */
}

void do_pray( CHAR_DATA *ch, const char *argument) {
  // Modified by SinaC 2001, pray use racial language
  // if a mortal use it
  //  every followers of the player's god show the praying
  //  every immortals show the praying
  // if an immortal use it
  //  every followers of the player's god show the reply
  //  every immortals show the reply

  if ( IS_NPC(ch)) {
    send_to_char("Mobiles can't use that command.\n\r",ch);
    return;
  }

  if ( !ch->pcdata->name_accepted ) {
    send_to_char("Your name has to accepted before able to use that channel.\n\r", ch );
    return;
  }

  if ( ch->pcdata->god >= MAX_GODS || ch->pcdata->god < 0 ) {
    send_to_char("You don't follow a god.\n\r",ch);
    log_stringf("do_pray: %s is not following a god.",
		NAME(ch) );
    return;
  }

  if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }
 
  if ( IS_SET(ch->comm, COMM_QUIET) ) {
    send_to_char( "You must turn off quiet mode first.\n\r", ch);
    return;
  }

  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_pray: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    return;
  }

  if (  argument[0] == '\0' ) {
    send_to_char( "Pray what ?\n\r", ch );
    return;
  }

  char buf[MAX_STRING_LENGTH];
  char god_nam[128];
  strcpy(god_nam,god_name(ch->pcdata->god));
  char arg[MAX_STRING_LENGTH];
  strcpy( arg, makedrunk(argument,ch));

  // sentence player says with current language
  char said[MAX_INPUT_LENGTH];
  int lang = get_current_language( ch );
  strcpy( said, str_to_language( ch, ch, lang, arg ) );

  if ( !IS_IMMORTAL(ch) )
    sprintf(buf,"{gYou pray %s'{x%s{g' to %s.{x\n\r",
	    language_name_known(ch,lang), said, god_nam );
  else
    sprintf(buf,"{gYou reply to prayers %s'{x%s{g'.{x\n\r",
	    language_name_known(ch,lang), said );
    
  send_to_char(buf,ch);

  for ( DESCRIPTOR_DATA *d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *vch = d->character;
    if ( d->connected == CON_PLAYING && vch != ch )
      if (!str_cmp(god_nam, vch->name))
	send_to_charf(vch,"{g%s has prayed %s'{x%s{g' to you.{x\n\r",
		NAME(ch), language_name_known(ch,lang), said );
      else if ( vch->pcdata->god == ch->pcdata->god || IS_IMMORTAL(vch) )
	if ( IS_IMMORTAL(ch) )
	  send_to_charf(vch,"{g%s replies to prayers %s'{x%s{g'.{x\n\r",
		  NAME(ch),language_name_known(ch,lang), said );
	else
	  send_to_charf(vch,"{g%s has prayed %s'{x%s{g' to %s.{x\n\r",
		  NAME(ch), language_name_known(ch,lang), said, god_nam );
  }

}

/* afk command */
void do_afk ( CHAR_DATA *ch, const char * argument) {
  // Added by SinaC 2000
  if (IS_NPC(ch)) {
    send_to_char("You can't do that!\n\r", ch );
    return;
  }

  if (IS_SET(ch->comm,COMM_AFK)) {
    send_to_char("{GAFK{x mode removed.\n\r",ch);
    if (buf_string(ch->pcdata->buffer)[0] != '\0' )
      send_to_char("{rYou have received tells: Type {y'replay'{r to see them.{x\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_AFK);
  }
  else {
    send_to_char("You are now in {GAFK{x mode.\n\r",ch);
    SET_BIT(ch->comm,COMM_AFK);
  }
}

void do_replay (CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch)) {
    send_to_char("You can't replay.\n\r",ch);
    return;
  }

  if (buf_string(ch->pcdata->buffer)[0] == '\0') {
    send_to_char("You have no tells to replay.\n\r",ch);
    return;
  }

  page_to_char(buf_string(ch->pcdata->buffer),ch);
  clear_buf(ch->pcdata->buffer);
}


// Auction rewritten in ROM2.4 style by Seytan
// Now adapted for automatic auctioning of items
// 1997 AUC ROM
void do_auction( CHAR_DATA *ch, const char *argument ) {
  char             buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  OBJ_DATA        *obj;
  int              minbet; 
  char             arg [MAX_INPUT_LENGTH];
  char             arg1[MAX_INPUT_LENGTH];

  argument=one_argument( argument, arg );
  one_argument( argument, arg1);

  if (IS_NPC(ch)) {
    send_to_char("Mobiles can't bet/auction!\n\r",ch);
    return;
  }

  if (arg[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOAUCTION)) {
      send_to_char("Auction channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOAUCTION);
    }
    else {
      send_to_char("Auction channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOAUCTION);
    }
  }
  else { /* auction message sent, turn auction on if it is off */
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
    }

    REMOVE_BIT(ch->comm,COMM_NOAUCTION);

    if(aucstat) {
      send_to_char("Wait until the current auction is over.\r\n",ch);
      return;
    }

    if(IS_IMMORTAL(ch)) {
      send_to_char("immortals shouldn't use auction.\n\r", ch);
      return;
    }

    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

    if ( !can_drop_obj( ch, obj ) ) {
      send_to_char( "You can't let go of it.\n\r", ch );
      return;
    }

    if( (obj->item_type>13 && obj->item_type<=25)
	|| obj->item_type>28 ) {/* Illegal item to auction */
      send_to_char("You can't auction that.\n\r", ch );
      return;
    } 

    if(IS_OBJ_STAT(obj, ITEM_DONATED)) {
      send_to_char("You can't auction donated equipment.\n\r", ch );
      return;
    }
	
    if(obj->timer > 0) {	/* Non-permanent item */
      send_to_char("Only permanent items can be auctioned.\n\r",ch);
      return;
    }

    minbet=atoi(arg1);
    if(minbet<1) {
      send_to_char(" You have to set a minimum bet.\r\n",ch);
      return;
    }

    obj_from_char( obj ); /* Remove from inventory */

    aucobj =obj;	/* initialize the auction mechanism */
    aucfrom=ch;	/* in update.c */
    aucstat=1;
    lastbet=minbet;
    lastbetter=NULL;

    sprintf( buf, "{m%s is auctioning %s{m. Minimum bet is %d{x\n\r", 
	     aucfrom->name, aucobj->short_descr, minbet );

    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;

      victim = d->original ? d->original : d->character;

      if ( d->connected == CON_PLAYING 
	   && !IS_SET(victim->comm,COMM_NOAUCTION) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*	
		act_new( "{m$n is auctioning '$t'{x",
		ch,argument, d->character, TO_VICT,POS_SLEEPING );
	*/
	send_to_char(buf,victim);
      }
    }
  }
}

/* Trivia chat made by Seytan 1997 */
void do_trivia ( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOTRIVIA)) {
      send_to_char("Trivia channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOTRIVIA);
    }
    else {
      send_to_char("trivia channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOTRIVIA);
    }
  }
  else { /* trivia message sent, turn trivia on if it isn't already */
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);          return;
    }

    REMOVE_BIT(ch->comm,COMM_NOTRIVIA);

    sprintf( buf, "{yTRIVIA: {x%s\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;

      victim = d->original ? d->original : d->character;

      if ( d->connected == CON_PLAYING 
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOTRIVIA) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*
	act_new( "{m$n {yTRIVIA:  {x$t",
		 ch,argument, d->character, TO_VICT,POS_SLEEPING );
	*/
	act( "{m$n {yTRIVIA:  {x$t",
		 ch,argument, d->character, TO_VICT);
      }
    }
  }
}

void do_ooc( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOOOC)) {
      send_to_char("OOC channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOOOC);
    }
    else {
      send_to_char("OOC channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOOOC);
    }
  }
  else { // gossip message sent, turn gossip on if it isn't already
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
 
    }

    REMOVE_BIT(ch->comm,COMM_NOOOC);
 
    sprintf( buf, "{mYou OOC '{M%s{m'{x\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;
 
      victim = d->original ? d->original : d->character;

      if ( d->connected == CON_PLAYING 
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOOOC) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*
	act_new( "{m$n OOC '{M$t{m'{x", 
		 ch,argument, d->character, TO_VICT);
	*/
	act( "{m$n OOC '{M$t{m'{x", 
		 ch,argument, d->character, TO_VICT);
      }
    }
  }
}

// IC channel added by SinaC 2001
void do_ic( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_ic: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    send_to_charf(ch,"You can't talk.\n\r");
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_charf(ch,"You can't talk, you're muted.\n\r");
    return;
  }

  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOIC)) {
      send_to_char("IC channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOIC);
    }
    else {
      send_to_char("IC channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOIC);
    }
  }
  else { // gossip message sent, turn gossip on if it isn't already
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
 
    }

    REMOVE_BIT(ch->comm,COMM_NOIC);

    // Added by SinaC 2000 for talking drunk
    char arg[MAX_INPUT_LENGTH];
    strcpy( arg, makedrunk(argument,ch) );
    
    // sentence player says with current language
    char said[MAX_INPUT_LENGTH];
    int lang = get_current_language( ch );
    strcpy( said, str_to_language( ch, ch, lang, arg ) );
    sprintf(buf,"{rYou IC %s'{R$t{r'{x", language_name_known(ch,lang));
    act( buf, ch, said, NULL, TO_CHAR );
    
    /*
    sprintf( buf, "{rYou IC '{R%s{r'{x\n\r", argument );
    send_to_char( buf, ch );
    */
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;
 
      victim = d->original ? d->original : d->character;

      if ( d->connected == CON_PLAYING 
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOIC) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	// sentence other player hear
	char heard[MAX_INPUT_LENGTH];
	strcpy( heard, str_to_language( ch, d->character, lang, said ) );
	sprintf( buf, "{r$n IC %s'{R$t{r'{x", language_name_known(d->character,lang));
	act_new( buf, ch, heard, d->character, TO_VICT, POS_SLEEPING );
	
	/*
	act_new( "{r$n IC '{R$t{r'{x", 
		 ch,argument, d->character, TO_VICT,POS_SLEEPING );
	*/
      }
    }
  }
}

void do_grats( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
 
  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOGRATS)) {
      send_to_char("Grats channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOGRATS);
    }
    else {
      send_to_char("Grats channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOGRATS);
    }
  }
  else { /* grats message sent, turn grats on if it isn't already */
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }

    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
 
    }
 
    REMOVE_BIT(ch->comm,COMM_NOGRATS);
 
    sprintf( buf, "You grats '%s'\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;
 
      victim = d->original ? d->original : d->character;
 
      if ( d->connected == CON_PLAYING 
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOGRATS) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*
	act_new( "$n grats '$t'",
		 ch,argument, d->character, TO_VICT,POS_SLEEPING );
		 */
	act( "$n grats '$t'",
		 ch,argument, d->character, TO_VICT );
      }
    }
  }
}

void do_quote( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
 
  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOQUOTE)) {
      send_to_char("Quote channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOQUOTE);
    }
    else {
      send_to_char("Quote channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOQUOTE);
    }
  }
  else { /* quote message sent, turn quote on if it isn't already */
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOQUOTE);
 
    sprintf( buf, "{yYou quote '%s'{x\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;
 
      victim = d->original ? d->original : d->character;
 
      if ( d->connected == CON_PLAYING 
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOQUOTE) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*
	act_new( "$n quotes '$t'",
		 ch,argument, d->character, TO_VICT,POS_SLEEPING );
		 */
	act( "{y$n quotes '$t'{x",
	     ch,argument, d->character, TO_VICT );
      }
    }
  }
}

/* RT question channel */
void do_question( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
 
  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOQUESTION)) {
      send_to_char("Q/A channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOQUESTION);
    }
    else {
      send_to_char("Q/A channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOQUESTION);
    }
  }
  else { /* question sent, turn Q/A on if it isn't already */
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
    sprintf( buf, "{yYou question '%s'{x\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;
 
      victim = d->original ? d->original : d->character;
 
      if ( d->connected == CON_PLAYING 
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOQUESTION) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*
	act_new("{y$n questions '$t'{x",
		ch,argument,d->character,TO_VICT,POS_SLEEPING);
		*/
	act("{y$n questions '$t'{x",
		ch,argument,d->character,TO_VICT);
      }
    }
  }
}

/* RT answer channel - uses same line as questions */
void do_answer( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
 
  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOQUESTION)) {
      send_to_char("Q/A channel is now ON.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOQUESTION);
    }
    else {
      send_to_char("Q/A channel is now OFF.\n\r",ch);
      SET_BIT(ch->comm,COMM_NOQUESTION);
    }
  }
  else { /* answer sent, turn Q/A on if it isn't already */
    if (IS_SET(ch->comm,COMM_QUIET)) {
      send_to_char("You must turn off quiet mode first.\n\r",ch);
      return;
    }
 
    if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
      send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
      return;
    }
 
    REMOVE_BIT(ch->comm,COMM_NOQUESTION);
 
    sprintf( buf, "{yYou answer '%s'{x\n\r", argument );
    send_to_char( buf, ch );
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *victim;
 
      victim = d->original ? d->original : d->character;
 
      if ( d->connected == CON_PLAYING
	   && d->character != ch 
	   && !IS_SET(victim->comm,COMM_NOQUESTION) 
	   && !IS_SET(victim->comm,COMM_QUIET) ) {
	/*
	act_new("{y$n answers '$t'{x",
		ch,argument,d->character,TO_VICT,POS_SLEEPING);
		*/
	act("{y$n answers '$t'{x",
		ch,argument,d->character,TO_VICT);
      }
    }
  }
}


/* clan channels */
void do_clantalk( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  if (!is_clan(ch)) {
    send_to_char("You aren't in a clan.\n\r",ch);
    return;
  }
  if ( argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOCLAN)) {
      send_to_char("Clan channel is now ON\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOCLAN);
    }
    else {
      send_to_char("Clan channel is now OFF\n\r",ch);
      SET_BIT(ch->comm,COMM_NOCLAN);
    }
    return;
  }

  if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }

  REMOVE_BIT(ch->comm,COMM_NOCLAN);

  sprintf( buf, "{YYou tell the clan '{x%s{Y'{x\n\r", argument );
  send_to_char( buf, ch );
  sprintf( buf, "{YCLANTALK: {c$n, %s : '{x%s{c'{x", lookup_clan_status(ch->clan_status), argument );
  for ( d = descriptor_list; d != NULL; d = d->next ) {
    if ( d->connected == CON_PLAYING 
	 && d->character != ch 
	 && is_same_clan(ch,d->character) 
	 && !IS_SET(d->character->comm,COMM_NOCLAN) 
	 && !IS_SET(d->character->comm,COMM_QUIET) ) {
      //act_new("{Y$n clans '{x$t{Y'",ch,argument,d->character,TO_VICT,POS_DEAD);
      act_new(buf,ch,argument,d->character,TO_VICT,POS_DEAD);
    }
  }

  return;
}

void do_immtalk( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;

  if ( argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_NOWIZ)) {
      send_to_char("Immortal channel is now ON\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_NOWIZ);
    }
    else {
      send_to_char("Immortal channel is now OFF\n\r",ch);
      SET_BIT(ch->comm,COMM_NOWIZ);
    } 
    return;
  }

  REMOVE_BIT(ch->comm,COMM_NOWIZ);

  sprintf( buf, "{c[{y$n{c]: %s{x", argument );
  act_new("{c[{y$n{c]: $t{x",ch,argument,NULL,TO_CHAR,POS_DEAD);
  for ( d = descriptor_list; d != NULL; d = d->next ) {
    if ( d->connected == CON_PLAYING 
	 && IS_IMMORTAL(d->character) 
	 && !IS_SET(d->character->comm,COMM_NOWIZ) ) {
      act_new("{c[{y$n{c]: $t{x",ch,argument,d->character,TO_VICT,POS_DEAD);
    }
  }

  return;
}

void do_say( CHAR_DATA *ch, const char *argument ) { 
  CHAR_DATA * fch, * fch_next;
  char buf[MAX_STRING_LENGTH];
   
  if ( argument[0] == '\0' ) {
    send_to_char( "Say what?\n\r", ch );
    return;
  }

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_say: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    send_to_charf(ch,"You can't talk.\n\r");
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_charf(ch,"You can't talk, you're muted.\n\r");
    return;
  }

  // Added by SinaC 2000 for talking drunk
  char arg[MAX_INPUT_LENGTH];
  strcpy( arg, makedrunk(argument,ch) );

  // sentence player says with current language
  char said[MAX_INPUT_LENGTH];
  int lang = get_current_language( ch );
  strcpy( said, str_to_language( ch, ch, lang, arg ) );
  sprintf(buf,"{gYou say %s'{x$t{g'{x", language_name_known(ch,lang));
  act( buf, ch, said, NULL, TO_CHAR );
  
  for ( fch = ch->in_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;
    // Added by SinaC 2001 for racial language
    if ( ch != fch ) {
      // sentence roomates hear
      char heard[MAX_INPUT_LENGTH];
      strcpy( heard, str_to_language( ch, fch, lang, said ) );
      sprintf( buf, "{g$n says %s'{x$t{g'{x", language_name_known(fch,lang));
      act( buf, ch, heard, fch, TO_VICT );
      
      //Value args[] = {ch, argument};
      Value args[] = {ch, heard};
      mobprog(fch,ch,"onSpeech", args);
    }
  }
  ROOMPROG(ch->in_room,ch,"onSpeech", ch, arg ); // Room understands every language
  //People inventory?
//  for ( fch = ch->in_room->people; fch != NULL; fch = fch_next ) {
//    fch_next = fch->next_in_room;
//    OBJ_DATA *obj, *obj_next;
//    for ( obj = fch->carrying;
//	  obj != NULL; 
//	  obj = obj_next) {
//      obj_next = obj->next_content;
//      //OBJPROG(obj,ch,"onSpeech", ch, argument );
//      OBJPROG(obj,ch,"onSpeech", ch, said );
//    }
//  }
  OBJ_DATA *obj, *obj_next;
  for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;
    OBJPROG(obj,ch,"onSpeech", ch, said );
  }
 
  return;
}

//OSAY channel added by SinaC 2001
void do_osay( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA * fch, * fch_next;
    
  if ( argument[0] == '\0' ) {
    send_to_char( "Osay what?\n\r", ch );
    return;
  }

  act( "{g$n says (OOC) '{x$T{g'{x", ch, NULL, argument, TO_ROOM );
  act( "{gYou say (OOC) '{x$T{g'{x", ch, NULL, argument, TO_CHAR );

  return;
}

void do_shout( CHAR_DATA *ch, const char *argument ) {
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_SHOUTSOFF)) {
      send_to_char("You can hear shouts again.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);
    }
    else {
      send_to_char("You will no longer hear shouts.\n\r",ch);
      SET_BIT(ch->comm,COMM_SHOUTSOFF);
    }
    return;
  }

  if ( IS_SET(ch->comm, COMM_NOSHOUT) ) {
    send_to_char( "You can't shout.\n\r", ch );
    return;
  }
 
  REMOVE_BIT(ch->comm,COMM_SHOUTSOFF);

  WAIT_STATE( ch, 12 );

  act( "You shout '$T'", ch, NULL, argument, TO_CHAR );
  for ( d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *victim;

    victim = d->original ? d->original : d->character;

    if ( d->connected == CON_PLAYING
	 && d->character != ch 
	 && !IS_SET(victim->comm, COMM_SHOUTSOFF) 
	 && !IS_SET(victim->comm, COMM_QUIET) ) {
      act("$n shouts '$t'",ch,argument,d->character,TO_VICT);
    }
  }

  return;
}

void do_otell( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH],
    arg3[MAX_INPUT_LENGTH],
    buf[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  int number = number_argument( arg, arg3 );
  int count = 0;
  bool found = FALSE;
  OBJ_DATA *obj, *obj_next;
  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( (can_see_obj( ch, obj ) ) 
	 && is_name( arg3, obj->name ) ) {
      if ( ++count == number ) {
	found = TRUE;
	break;
      }
    }
  }
  if ( !found ) {
    send_to_char("You can't find it.\n\r", ch );
    return;
  }
  act( "{GYou tell $P '{g$t{G'{x", ch, arg2, obj, TO_CHAR );
  OBJPROG(obj,ch,"onTell", ch, arg2 ); 
}

void do_tell( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  if ( IS_SET(ch->comm, COMM_NOTELL) || IS_SET(ch->comm,COMM_DEAF)) {
    send_to_char( "Your message didn't get through.\n\r", ch );
    return;
  }

  if ( IS_SET(ch->comm, COMM_QUIET) ) {
    send_to_char( "You must turn off quiet mode first.\n\r", ch);
    return;
  }

  if (IS_SET(ch->comm,COMM_DEAF)) {
    send_to_char("You must turn off deaf mode first.\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' ) {
    send_to_char( "Tell whom what?\n\r", ch );
    return;
  }

  /*
   * Can tell to PC's anywhere, but NPC's only in same room.
   * -- Furey
   */
  if ( ( victim = get_char_world( ch, arg ) ) == NULL
       || ( IS_NPC(victim) && victim->in_room != ch->in_room ) ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  // Modified by SinaC 2001
  //sprintf(buf,"{g%s tells you '{G%s{g'{x\n\r",PERS(ch,victim),argument);
  sprintf(buf,"{g%s tells you '{G%s{g'{x\n\r",NAME(ch),argument);
  buf[0] = UPPER(buf[0]);
  if ( victim->desc == NULL && !IS_NPC(victim)) {
    act("$N seems to have misplaced $S link...try again later.",
	ch,NULL,victim,TO_CHAR);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }
  if ( ( IS_SET(victim->comm,COMM_QUIET) 
	 || IS_SET(victim->comm,COMM_DEAF) )
       && !IS_IMMORTAL(ch) ) {
    act( "$E is not receiving tells.", ch, 0, victim, TO_CHAR );
    return;
  }

  if (IS_SET(victim->comm,COMM_AFK)) {
    if (IS_NPC(victim)) {
      act("$E is AFK, and not receiving tells.",ch,NULL,victim,TO_CHAR);
      return;
    }

    act("$E is AFK, but your tell will go through when $E returns.",
	ch,NULL,victim,TO_CHAR);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  // Added by SinaC 2000
  if (IS_SET(victim->comm,COMM_BUILDING)) {
    if (IS_NPC(victim)) {
      act("$E is BUILDING, and not receiving tells.",ch,NULL,victim,TO_CHAR);
      return;
    }

    act("$E is BUILDING, but your tell will go through when $E returns.",
	ch,NULL,victim,TO_CHAR);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (IS_SET(victim->comm,COMM_EDITING)) {
    if (IS_NPC(victim)) {
      act("$E is EDITING, and not receiving tells.",ch,NULL,victim,TO_CHAR);
      return;
    }

    act("$E is EDITING, but your tell will go through when $E returns.",
	ch,NULL,victim,TO_CHAR);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  act( "{gYou tell $N '{G$t{g'{x", ch, argument, victim, TO_CHAR );
  // Modified by SinaC 2001
  //act_new("{g$n tells you '{G$t{g'{x",ch,argument,victim,TO_VICT,POS_DEAD);
  send_to_char(buf,victim);
  victim->reply	= ch;

  MOBPROG(victim,ch,"onTell", ch, argument ); 
  return;
}

void do_reply( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  if ( IS_SET(ch->comm, COMM_NOTELL) ) {
    send_to_char( "Your message didn't get through.\n\r", ch );
    return;
  }

  if ( ( victim = ch->reply ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  //sprintf(buf,"{g%s tells you '{G%s{g'{x\n\r",PERS(ch,victim),argument);
  sprintf(buf,"{g%s tells you '{G%s{g'{x\n\r",NAME(ch),argument);
  buf[0] = UPPER(buf[0]);
  if ( victim->desc == NULL && !IS_NPC(victim)) {
    act("$N seems to have misplaced $S link...try again later.",
	ch,NULL,victim,TO_CHAR);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  if ( !IS_IMMORTAL(ch) && !IS_AWAKE(victim) ) {
    act( "$E can't hear you.", ch, 0, victim, TO_CHAR );
    return;
  }

  if (( IS_SET(victim->comm,COMM_QUIET) 
	|| IS_SET(victim->comm,COMM_DEAF) )
      && !IS_IMMORTAL(ch) 
      && !IS_IMMORTAL(victim)) {
    act_new( "$E is not receiving tells.", ch, 0, victim, TO_CHAR,POS_DEAD);
    return;
  }

  if ( !IS_IMMORTAL(victim) 
       && !IS_AWAKE(ch) ) {
    send_to_char( "In your dreams, or what?\n\r", ch );
    return;
  }

  if (IS_SET(victim->comm,COMM_AFK)) {
    if (IS_NPC(victim)) {
      act_new("$E is AFK, and not receiving tells.",
	      ch,NULL,victim,TO_CHAR,POS_DEAD);
      return;
    }

    act_new("$E is AFK, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR,POS_DEAD);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }
  // Added by SinaC 2000
  if (IS_SET(victim->comm,COMM_BUILDING)) {
    if (IS_NPC(victim)) {
      act_new("$E is BUILDING, and not receiving tells.",
	      ch,NULL,victim,TO_CHAR,POS_DEAD);
      return;
    }

    act_new("$E is BUILDING, but your tell will go through when $E returns.",
	    ch,NULL,victim,TO_CHAR,POS_DEAD);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (IS_SET(victim->comm,COMM_EDITING)) {
    if (IS_NPC(victim)) {
      act("$E is EDITING, and not receiving tells.",ch,NULL,victim,TO_CHAR);
      return;
    }

    act("$E is EDITING, but your tell will go through when $E returns.",
	ch,NULL,victim,TO_CHAR);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  act_new("{gYou tell $N '{G$t{g'{x",ch,argument,victim,TO_CHAR,POS_DEAD);
  //act_new("{g$n tells you '{G$t{g'{x",ch,argument,victim,TO_VICT,POS_DEAD);
  send_to_char(buf,victim);
  victim->reply	= ch;

  return;
}

void do_yell( CHAR_DATA *ch, const char *argument ) {
  DESCRIPTOR_DATA *d;

  if ( IS_SET(ch->comm, COMM_NOSHOUT) ) {
    send_to_char( "You can't yell.\n\r", ch );
    return;
  }

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_yell: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    send_to_charf(ch,"You can't talk.\n\r");
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_charf(ch,"You can't talk, you're muted.\n\r");
    return;
  }
 
  if ( argument[0] == '\0' ) {
    send_to_char( "Yell what?\n\r", ch );
    return;
  }

  // Modified by SinaC 2003, add colors

  // Added by SinaC 2000 for talking drunk
  char arg[MAX_INPUT_LENGTH];
  strcpy( arg, makedrunk(argument,ch) );

  // sentence player says with current language
  char buf[MAX_STRING_LENGTH];
  char said[MAX_INPUT_LENGTH];
  int lang = get_current_language( ch );
  strcpy( said, str_to_language( ch, ch, lang, arg ) );
  sprintf(buf,"{GYou yell %s'$t'{x", language_name_known(ch,lang));
  act( buf, ch, said, NULL, TO_CHAR );

  //act("You yell '$t'",ch,argument,NULL,TO_CHAR);
  for ( d = descriptor_list; d != NULL; d = d->next ) {
    if ( d->connected == CON_PLAYING
	 && d->character != ch
	 && d->character->in_room != NULL
	 && d->character->in_room->area == ch->in_room->area 
	 && !IS_SET(d->character->comm,COMM_QUIET) ) {
      char heard[MAX_INPUT_LENGTH];
      strcpy( heard, str_to_language( ch, d->character, lang, said ) );
      sprintf( buf, "{G$n yells %s'$t'{x", language_name_known(d->character,lang));
      act( buf, ch, heard, d->character, TO_VICT );
      //act("$n yells '$t'",ch,argument,d->character,TO_VICT);
    }
  }

  return;
}

void do_emote( CHAR_DATA *ch, const char *argument ) {
  if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) ) {
    send_to_char( "You can't show your emotions.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_char( "Emote what?\n\r", ch );
    return;
  }
 
  act( "$n $T", ch, NULL, argument, TO_ROOM );
  act( "$n $T", ch, NULL, argument, TO_CHAR );
  return;
}

void do_pmote( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *vch;
  const char *letter,*name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  // modified by SinaC 2001  int  before
  unsigned int matches = 0;

  if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) ) {
    send_to_char( "You can't show your emotions.\n\r", ch );
    return;
  }
 
  if ( argument[0] == '\0' ) {
    send_to_char( "Emote what?\n\r", ch );
    return;
  }
 
  act( "$n $t", ch, argument, NULL, TO_CHAR );

  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    if (vch->desc == NULL || vch == ch)
      continue;

    if ((letter = strstr(argument,vch->name)) == NULL) {
      act("$N $t",vch,argument,ch,TO_CHAR);
      continue;
    }

    strcpy(temp,argument);
    temp[strlen(argument) - strlen(letter)] = '\0';
    last[0] = '\0';
    name = vch->name;
	
    for (; *letter != '\0'; letter++)	{ 
      if (*letter == '\'' && matches == strlen(vch->name)) {
	strcat(temp,"r");
	continue;
      }

      if (*letter == 's' && matches == strlen(vch->name)) {
	matches = 0;
	continue;
      }
	    
      if (matches == strlen(vch->name)) {
	matches = 0;
      }

      if (*letter == *name) {
	matches++;
	name++;
	if (matches == strlen(vch->name)) {
	  strcat(temp,"you");
	  last[0] = '\0';
	  name = vch->name;
	  continue;
	}
	strncat(last,letter,1);
	continue;
      }

      matches = 0;
      strcat(temp,last);
      strncat(temp,letter,1);
      last[0] = '\0';
      name = vch->name;
    }

    act("$N $t",vch,temp,ch,TO_CHAR);
  }

  return;
}

// Modified by SinaC 2000
const char ****pose_table;

void do_pose( CHAR_DATA *ch, const char *argument ) {
  int level;
  int pose;
  int cla;
  
  if ( IS_NPC(ch) )
    return;

  level = UMIN( ch->level, MAX_POSE-1 );
  pose = number_range( 0, level );
  
  cla = class_firstclass(ch->cstat(classes));

  act( pose_table[cla][pose][0], ch, NULL, NULL, TO_CHAR );
  act( pose_table[cla][pose][1], ch, NULL, NULL, TO_ROOM );
}


void do_bug( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't log bugs!\n\r",ch);
    return;
  }

  append_file( ch, BUG_FILE, argument );
  send_to_char( "Bug logged.\n\r", ch );
  return;
}

void do_typo( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't log typo!\n\r",ch);
    return;
  }

  append_file( ch, TYPO_FILE, argument );
  send_to_char( "Typo logged.\n\r", ch );
  return;
}

void do_rent( CHAR_DATA *ch, const char *argument ) {
  send_to_char( "There is no rent here.  Just save and quit.\n\r", ch );
  return;
}

// Added by SinaC 2000
bool check_pet_charmies_fighting( CHAR_DATA *ch ) {
  CHAR_DATA *gch;

  for ( gch = char_list; gch != NULL; gch = gch->next ){
    if ( is_same_group( gch, ch ) 
	 && IS_NPC( gch )
	 && gch->fighting != NULL )
      return TRUE;
  }
  return FALSE;
}

void do_qui( CHAR_DATA *ch, const char *argument ) {
  send_to_char( "If you want to QUIT, you have to spell it out.\n\r", ch );
  return;
}

void do_quit( CHAR_DATA *ch, const char *argument ) {
  DESCRIPTOR_DATA *d,*d_next;
  int id;

  if ( IS_NPC(ch) )
    return;

  // Added by SinaC 2000 for Arena
  // Maybe we could avoid people leaving Arena and Challenge
  if ( IN_BATTLE(ch) 
       || IN_WAITING(ch) ) {
    /*
      sprintf(qbuf,"%s has quit! Challenge is void. WHAT A WUSS!",ch->name);
      ch->challenged=NULL;
      sportschan(qbuf);
    */
    send_to_char("No way! You're in the battle field!\n\r",ch);
    return;
  }
    
  if ( ch->position == POS_FIGHTING ) {
    send_to_char( "No way! You are fighting.\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if ( check_pet_charmies_fighting( ch ) ) {
    send_to_char( "No way! Your pet or charmies are still fighting.\n\r",ch );
    return;
  }

  if ( ch->position  < POS_INCAP  ) {
    send_to_char( "You're not DEAD yet.\n\r", ch );
    return;
  }
  // Added by SinaC 2003
  if ( ch->position == POS_PARALYZED  ) {
    send_to_char( "You can't leave when paralyzed.\n\r", ch );
    return;
  }
  if ( ch->position == POS_STUNNED  ) {
    send_to_char( "You can't leave when stunned.\n\r", ch );
    return;
  }
    
  if ( ch == aucfrom ) {
    send_to_char( "Your auction is still running.\n\r", ch);
    return;
  }

  if ( ch == lastbetter ) {
    send_to_char( "You have the highest bet on an auction.\n\r", ch);
    return;
  }

  // SinaC 2003, pet is saved even if pet & master are in different rooms
  //  CHAR_DATA *mount = get_mount(ch);
  //  if ( mount )
  //    mount_to_master( mount, ch, TRUE );

  send_to_char( "Alas, all good things must come to an end.\n\r",ch);
  act( "$n has left the game.", ch, NULL, NULL, TO_ROOM );
  sprintf( log_buf, "%s has quit.", ch->name );
  log_string( log_buf );
  wiznet("$N rejoins the real world.",ch,NULL,WIZ_LOGINS,0,get_trust(ch));

  // Added by SinaC 2001, UNIQUE item aren't saved they are dropped anywhere 
  // in the mud
  // !!!! dropped on the ground for the moment !!!!
  OBJ_DATA *obj_next;
  ROOM_INDEX_DATA *room = ch->in_room;
  int vnum = room?room->vnum:-1;
  if ( room == NULL )
    room = get_room_index(ROOM_VNUM_LIMBO);
  bool found = FALSE;
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;
    if ( IS_SET( obj->extra_flags, ITEM_UNIQUE ) ) {
      log_stringf("%s leaving with a unique item %s (vnum %d)",
		  NAME(ch), obj->name, obj->pIndexData->vnum );
      obj_from_char( obj );
      obj_to_room( obj, room );
      found = TRUE;
    }
  }
  if ( found && vnum == -1 )
    bug("unique item: [%s] left while being in an invalid room (%d)",
	NAME(ch), vnum );

  /*
   * After extract_char the ch is no longer valid!
   */
  //save_char_obj( ch );
  new_save_pFile(ch, TRUE );
  id = ch->id;

  /* Free note that might be there somehow */        
  ch->pcdata->in_progress = NULL;
    
  d = ch->desc;
  extract_char( ch, TRUE );
  if ( d != NULL )
    close_socket( d );

  /* toast evil cheating bastards */
  for (d = descriptor_list; d != NULL; d = d_next) {
    CHAR_DATA *tch;

    d_next = d->next;
    tch = d->original ? d->original : d->character;
    if (tch && tch->id == id) {
      extract_char(tch,TRUE);
      close_socket(d);
    } 
  }

  return;
}

void do_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) )
    return;

  //save_char_obj( ch );
  new_save_pFile(ch, FALSE );
  send_to_char("Saving. Remember that ROM has automatic saving now.\n\r", ch);
  WAIT_STATE(ch,4 * PULSE_VIOLENCE);
  return;
}

void do_follow( CHAR_DATA *ch, const char *argument ) {
  /* RT changed to allow unlimited following and follow the NOFOLLOW rules */
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Follow whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL ) {
    act( "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
    return;
  }

  if ( victim == ch ) {
    if ( ch->master == NULL ) {
      send_to_char( "You already follow yourself.\n\r", ch );
      return;
    }
    stop_follower(ch);
    return;
  }

  if ( !IS_NPC(victim) 
       && IS_SET(victim->act,PLR_NOFOLLOW) 
       && !IS_IMMORTAL(ch) ) {
    act("$N doesn't seem to want any followers.\n\r",
	ch,NULL,victim, TO_CHAR);
    return;
  }

  REMOVE_BIT(ch->act,PLR_NOFOLLOW);
    
  if ( ch->master != NULL )
    stop_follower( ch );

  add_follower( ch, victim );
  return;
}

void add_follower( CHAR_DATA *ch, CHAR_DATA *master ){
  if ( ch->master != NULL ) {
    bug( "Add_follower: non-null master." );
    /*return; mud went into infinite loop after that */
  }

  ch->master        = master;

  /* if (ch->master!=ch->leader) */ // suggestion by Oxtal

  /* Added, by oxtal, this ensures that pet-master double link is not
     half broken */
  if (ch->leader)
    ch->leader->pet = NULL;
  ch->leader        = NULL;

  if ( can_see( master, ch ) )
    act( "$n now follows you.", ch, NULL, master, TO_VICT );

  act( "You now follow $N.",  ch, NULL, master, TO_CHAR );

  return;
}

void stop_follower( CHAR_DATA *ch ) {
  if ( ch->master == NULL ) {
    bug( "Stop_follower: null master." );
    return;
  }

  if ( IS_AFFECTED(ch, AFF_CHARM) ) {
    REMOVE_BIT( ch->bstat(affected_by), AFF_CHARM );
    //recompute(ch); doesn't need that, done in affect_stip
    affect_strip( ch, gsn_charm_person );
  }

  if ( can_see( ch->master, ch ) 
       && ch->in_room != NULL) {
    act( "$n stops following you.",     ch, NULL, ch->master, TO_VICT    );
    act( "You stop following $N.",      ch, NULL, ch->master, TO_CHAR    );
  }

  if (ch->master->pet == ch)
    ch->master->pet = NULL;

  ch->master = NULL;
  ch->leader = NULL;
  return;
}

/* nukes charmed monsters and pets */
void nuke_pets( CHAR_DATA *ch ) {
  CHAR_DATA *pet;

  if ((pet = ch->pet) != NULL) {
    stop_follower(pet);
    //if ( pet->in_room != NULL ) SinaC 2003
    //  act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
    if ( pet->in_room != NULL ) {
      CHAR_DATA *prev; // SinaC 2003: is pet really in a room
      for ( prev = ch->in_room->people; prev; prev = prev->next_in_room )
	if ( prev->next_in_room == ch )
	  break;
      if ( prev != NULL )
	act("$n slowly fades away.", pet,NULL,NULL,TO_ROOM);
    }
    extract_char(pet,TRUE);
  }
  ch->pet = NULL;

  return;
}

void die_follower( CHAR_DATA *ch ) {
  CHAR_DATA *fch;
  CHAR_DATA *pet;

  /* Added by Sinac 1997 */    
  if ( ( pet = ch->pet ) != NULL ) {
    // Added SinaC 2000
    act("$N stops following $n and disappears.",ch, NULL, ch->pet, TO_ROOM );
    // Added by SinaC 2003, mount code
    mount_dying_drawback( pet );
    extract_char( pet, TRUE );
    pet = NULL;
  }

  if ( ch->master != NULL ) {
    if (ch->master->pet == ch)
      ch->master->pet = NULL;
    stop_follower( ch );
  }

  ch->leader = NULL;

  // Modified by SinaC 2001, fch_next added
  CHAR_DATA *fch_next;
  for ( fch = char_list; fch != NULL; fch = fch_next ) {
    fch_next = fch->next;
    if ( IS_NPC(fch)
	 && ( fch->master == ch
	      || fch->leader == ch )
	 && IS_SET( fch->act, ACT_CREATED ) ) {
      act("$N stops following $n and disappears.",ch, NULL, fch, TO_ROOM );
      extract_char( fch, TRUE );
    }
    else {
      if ( fch->master == ch )
	stop_follower( fch );
      if ( fch->leader == ch )
	fch->leader = fch;
    }
  }

  return;
}

void do_order( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *och;
  CHAR_DATA *och_next;
  bool found;
  bool fAll;

  argument = one_argument( argument, arg );
  one_argument(argument,arg2);

  if (!str_cmp(arg2,"delete")) {
    send_to_char("That will NOT be done.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' || argument[0] == '\0' ) {
    send_to_char( "Order whom to do what?\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( ch, AFF_CHARM ) ) {
    send_to_char( "You feel like taking, not giving, orders.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) ) {
    fAll   = TRUE;
    victim = NULL;
  }
  else {
    fAll   = FALSE;

    if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( victim == ch ) {
      send_to_char( "Aye aye, right away!\n\r", ch );
      return;
    }

    if ( !IS_AFFECTED(victim, AFF_CHARM) 
	 || victim->master != ch
	 || ( IS_IMMORTAL(victim) 
	      && victim->trust >= ch->trust) ) {
      send_to_char( "Do it yourself!\n\r", ch );
      return;
    }
  }

  found = FALSE;
  for ( och = ch->in_room->people; och != NULL; och = och_next ) {
    och_next = och->next_in_room;

    if ( ( IS_AFFECTED(och, AFF_CHARM)
	   && och->master == ch )
	 && ( fAll 
	      || och == victim ) ) {
      found = TRUE;
      sprintf( buf, "$n orders you to '%s'.", argument );
      act( buf, ch, NULL, och, TO_VICT );
      interpret( och, argument );
    }
  }

  if ( found ) {
    WAIT_STATE(ch,PULSE_VIOLENCE);
    send_to_char( "Ok.\n\r", ch );
  }
  else
    send_to_char( "You have no followers here.\n\r", ch );
  return;
}

void do_group( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use this command!\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' ) {
    CHAR_DATA *gch;
    CHAR_DATA *leader;

    leader = (ch->leader != NULL) ? ch->leader : ch;
    sprintf( buf, "%s's group:\n\r", PERS(leader, ch) );
    send_to_char( buf, ch );

    for ( gch = char_list; gch != NULL; gch = gch->next ) {
      if ( is_same_group( gch, ch ) ) {
	sprintf( buf,
		 "[%3d %s%s] %-16s %4d/%4ld hp %4d/%4ld mana %4d/%4ld psp %4d/%4ld mv %5d xp\n\r",
		 gch->level,
		 IS_NPC(gch) ? "Mob" : class_whoname(gch->cstat(classes)),
		 IS_NPC(gch) && gch->isWildMagic ? "Wild ":"", // SinaC 2003
		 capitalize( PERS(gch, ch) ),
		 gch->hit,   gch->cstat(max_hit),
		 gch->mana,  gch->cstat(max_mana),
		 gch->psp,  gch->cstat(max_psp),
		 gch->move,  gch->cstat(max_move),
		 gch->exp    );
	send_to_char( buf, ch );
      }
    }
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( ch->master != NULL || ( ch->leader != NULL && ch->leader != ch ) ) {
    send_to_char( "But you are following someone else!\n\r", ch );
    return;
  }

  if ( victim->master != ch && ch != victim ) {
    act_new("$N isn't following you.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
  }
    
  if (IS_AFFECTED(victim,AFF_CHARM)) {
    send_to_char("You can't remove charmed mobs from your group.\n\r",ch);
    return;
  }
    
  if (IS_AFFECTED(ch,AFF_CHARM)) {
    act_new("You like your master too much to leave $m!",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
    return;
  }

  if ( is_same_group( victim, ch ) 
       && ch != victim ) {
    victim->leader = NULL;
    act_new("$n removes $N from $s group.",
	    ch,NULL,victim,TO_NOTVICT,POS_RESTING);
    act_new("$n removes you from $s group.",
	    ch,NULL,victim,TO_VICT,POS_SLEEPING);
    act_new("You remove $N from your group.",
	    ch,NULL,victim,TO_CHAR,POS_SLEEPING);
    return;
  }

  int diff_level = victim->level - ch->level;
  if ( ( diff_level > 10 || diff_level < -10 ) 
       && !IS_IMMORTAL(ch) 
       && !IS_IMMORTAL( victim ) ) {
    send_to_char( "You can only group person in an acceptable range.\n\r", ch );
    return;
  }

  victim->leader = ch;
  act_new("$N joins $n's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
  act_new("You join $n's group.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
  act_new("$N joins your group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
  return;
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  int members;
  int amount_gold = 0, amount_silver = 0;
  int share_gold, share_silver;
  int extra_gold, extra_silver;

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( arg1[0] == '\0' ) {
    send_to_char( "Split how much?\n\r", ch );
    return;
  }
    
  amount_silver = atoi( arg1 );

  if (arg2[0] != '\0')
    amount_gold = atoi(arg2);

  if ( amount_gold < 0 || amount_silver < 0) {
    send_to_char( "Your group wouldn't like that.\n\r", ch );
    return;
  }

  if ( amount_gold == 0 && amount_silver == 0 ) {
    send_to_char( "You hand out zero coins, but no one notices.\n\r", ch );
    return;
  }

  if ( ch->gold <  amount_gold || ch->silver < amount_silver) {
    send_to_char( "You don't have that much to split.\n\r", ch );
    return;
  }
  
  members = 0;
  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
    if ( is_same_group( gch, ch ) 
	 && !IS_AFFECTED(gch,AFF_CHARM))
      members++;
  }

  if ( members < 2 ) {
    send_to_char( "Just keep it all.\n\r", ch );
    return;
  }
	    
  share_silver = amount_silver / members;
  extra_silver = amount_silver % members;

  share_gold   = amount_gold / members;
  extra_gold   = amount_gold % members;

  if ( share_gold == 0 && share_silver == 0 ) {
    send_to_char( "Don't even bother, cheapskate.\n\r", ch );
    return;
  }

  ch->silver	-= amount_silver;
  ch->silver	+= share_silver + extra_silver;
  ch->gold 	-= amount_gold;
  ch->gold 	+= share_gold + extra_gold;

  if (share_silver > 0) {
    sprintf(buf,
	    "You split %d silver coins. Your share is %d silver.\n\r",
	    amount_silver,share_silver + extra_silver);
    send_to_char(buf,ch);
  }

  if (share_gold > 0) {
    sprintf(buf,
	    "You split %d gold coins. Your share is %d gold.\n\r",
	    amount_gold,share_gold + extra_gold);
    send_to_char(buf,ch);
  }

  if (share_gold == 0) {
    sprintf(buf,"$n splits %d silver coins. Your share is %d silver.",
	    amount_silver,share_silver);
  }
  else if (share_silver == 0) {
    sprintf(buf,"$n splits %d gold coins. Your share is %d gold.",
	    amount_gold,share_gold);
  }
  else {
    sprintf(buf,
	    "$n splits %d silver and %d gold coins, giving you %d silver and %d gold.\n\r",
	    amount_silver,amount_gold,share_silver,share_gold);
  }

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
    if ( gch != ch 
	 && is_same_group(gch,ch) 
	 && !IS_AFFECTED(gch,AFF_CHARM)) {
      act( buf, ch, NULL, gch, TO_VICT );
      gch->gold += share_gold;
      gch->silver += share_silver;
    }
  }

  return;
}

void do_gtell( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *gch;

  if ( argument[0] == '\0' ) {
    send_to_char( "Tell your group what?\n\r", ch );
    return;
  }

  if ( IS_SET( ch->comm, COMM_NOTELL ) ) {
    send_to_char( "Your message didn't get through!\n\r", ch );
    return;
  }

  for ( gch = char_list; gch != NULL; gch = gch->next ) {
    if ( is_same_group( gch, ch ) )
      act_new("{g$n tells the group '{x$t{g'{x",
	      ch,argument,gch,TO_VICT,POS_SLEEPING);
  }
  // Added by SinaC 2001
  send_to_charf( ch,
		 "{gYou tell to the group '{x%s{g'{x\n\r",
		 argument );

  return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group( CHAR_DATA *ach, CHAR_DATA *bch )
{
  if ( ach == NULL || bch == NULL || !ach->valid || !bch->valid )
    return FALSE;

  if ( ach->leader != NULL ) ach = ach->leader;
  if ( bch->leader != NULL ) bch = bch->leader;
  return ach == bch;
}


// Added by SinaC 2000  for drunk talk
struct struckdrunk {
  int min_drunk_level;
  int number_of_rep;
  const char *replacement[11];
};

const char * makedrunk ( const char *string, CHAR_DATA * ch ) {
  /* This structure defines all changes for a character */
  struct struckdrunk drunk[] =
  {
    {3, 10, {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,  {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,  {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 3,  {"d", "d", "D", "dD" }},
    {3, 3,  {"e", "e", "eh", "E" }},
    {4, 5,  {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 3,  {"g", "g", "G", "ggG" }},
    {9, 6,  {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,  {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,  {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 3,  {"k", "k", "K", "kk" }},
    {3, 3,  {"l", "l", "L", "ll" }},
    {5, 8,  {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,  {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,  {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 3,  {"p", "p", "P", "pP" }},
    {5, 5,  {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 3,  {"r", "r", "R", "Rr"}},
    {2, 5,  {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,  {"t", "t", "T", "tt" }},
    {3, 6,  {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 3,  {"v", "v", "V", "vv" }},
    {4, 3,  {"w", "w", "W", "Ww" }},
    {5, 6,  {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 3,  {"y", "y", "Y", "yY" }},
    {2, 9,  {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };

  char buf[MAX_STRING_LENGTH];
  char temp;
  int pos = 0;
  int drunklevel;
  int randomnum;

  /* Check how drunk a person is... */
  if (IS_NPC(ch))
    drunklevel = 0;
  else
    drunklevel = ch->pcdata->condition[COND_DRUNK];

  if (drunklevel > 0){
    do{
      temp = toupper (*string);
      if ((temp >= 'A') && (temp <= 'Z')){
	if (drunklevel > drunk[temp - 'A'].min_drunk_level){
	  randomnum = number_range (0, drunk[temp - 'A'].number_of_rep);
	  strcpy (&buf[pos], drunk[temp - 'A'].replacement[randomnum]);
	  pos += strlen (drunk[temp - 'A'].replacement[randomnum]);
	}
	else
	  buf[pos++] = *string;
      }
      else{
	if ((temp >= '0') && (temp <= '9')){
	  temp = '0' + number_range (0, 9);
	  buf[pos++] = temp;
	}
	else
	  buf[pos++] = *string;
      }
    }
    while (*string++);
    buf[pos] = '\0';          /* Mark end of the string... */
    return str_dup(buf);
  }
  return (string);
}

// Added by SinaC 2001
void do_whisper( CHAR_DATA *ch, const char *argument ) {
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg2 );

  if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }
 
  if ( IS_SET(ch->comm, COMM_QUIET) ) {
    send_to_char( "You must turn off quiet mode first.\n\r", ch);
    return;
  }

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_whisper: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    send_to_charf(ch,"You can't talk.\n\r");
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_charf(ch,"You can't talk, you're muted.\n\r");
    return;
  }

  if ( arg2[0] == '\0' 
       || argument[0] == '\0' ) {
    send_to_char( "Whisper whom what ?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  // Added by SinaC 2000 for talking drunk
  char arg[MAX_STRING_LENGTH];
  strcpy( arg, makedrunk(argument,ch ) );

  // sentence player says with current language
  char said[MAX_INPUT_LENGTH];
  int lang = get_current_language( ch );
  strcpy( said, str_to_language( ch, ch, lang, arg ) );

  if ( victim->desc == NULL && !IS_NPC(victim)) {
    act("$N seems to have misplaced $S link...try again later.",
	ch,NULL,victim,TO_CHAR);
    // sentence other player hear
    char heard[MAX_INPUT_LENGTH];
    strcpy( heard, str_to_language( ch, victim, lang, said ) );

    sprintf(buf,"%s whispers you %s'%s'\n\r",
	    PERS(ch,victim), language_name_known(victim,lang), heard );
    //sprintf(buf,"%s whispers you '%s'\n\r",PERS(ch,victim),argument);
    buf[0] = UPPER(buf[0]);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  sprintf(buf,"You whisper %s'$t' to $N.", language_name_known(ch,lang));
  act( buf, ch, said, victim, TO_CHAR );
  
  // sentence other player hear
  char heard[MAX_INPUT_LENGTH];
  strcpy( heard, str_to_language( ch, victim, lang, said ) );
  sprintf( buf, "$n whispers you %s'$t'.", language_name_known(victim,lang));
  act( buf, ch, heard, victim, TO_VICT );

  //act( "You whisper '$t' to $N.", ch, argument, victim, TO_CHAR );
  //act("$n whispers you '$t'.",ch,argument,victim,TO_VICT);
  act( "$n whispers something to $N.", ch, NULL, victim, TO_NOTVICT );
  
  MOBPROG(victim, ch, "onWhisper", ch, heard );

  return;
}

void do_dirsay( CHAR_DATA *ch, const char *argument ) {
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg2 );

  if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }
 
  if ( IS_SET(ch->comm, COMM_QUIET) ) {
    send_to_char( "You must turn off quiet mode first.\n\r", ch);
    return;
  }

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_dirsay: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    send_to_charf(ch,"You can't talk.\n\r");
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_charf(ch,"You can't talk, you're muted.\n\r");
    return;
  }

  if ( arg2[0] == '\0' 
       || argument[0] == '\0' ) {
    send_to_char( "Dirsay whom what ?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  // Added by SinaC 2000 for talking drunk
  char arg[MAX_STRING_LENGTH];
  strcpy( arg, makedrunk(argument,ch));

  // sentence player says with current language
  char said[MAX_INPUT_LENGTH];
  int lang = get_current_language( ch );
  strcpy( said, str_to_language( ch, ch, lang, arg ) );

  if ( victim->desc == NULL && !IS_NPC(victim)) {
    act("$N seems to have misplaced $S link...try again later.",
	ch,NULL,victim,TO_CHAR);
    // sentence other player hear
    char heard[MAX_INPUT_LENGTH];
    strcpy( heard, str_to_language( ch, victim, lang, said ) );

    sprintf(buf,"{g%s says you %s'{x%s{g'.{x\n\r",
	    PERS(ch,victim), language_name_known(victim,lang), heard);
    //sprintf(buf,"{g%s says you '{x%s{g'.{x\n\r",PERS(ch,victim),argument);
    buf[0] = UPPER(buf[0]);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  sprintf(buf,"{gYou say %s'{x$t{g' to $N.{x", language_name_known(ch,lang));
  act( buf, ch, said, victim, TO_CHAR );
  
  CHAR_DATA *fch_next, *fch;
  for ( fch = ch->in_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;
    // Added by SinaC 2001 for racial language
    if ( fch != ch && fch != victim ) {
      // sentence roomates hear
      char heard[MAX_INPUT_LENGTH];
      strcpy( heard, str_to_language( ch, fch, lang, said ) );
      sprintf( buf, "{g$n says %s'{x$t{g' to %s.{x", 
	       language_name_known(fch,lang),
	       PERS(victim,fch));
      act( buf, ch, heard, fch, TO_VICT );
    }
  }

  // sentence other player hear
  char heard[MAX_INPUT_LENGTH];
  strcpy( heard, str_to_language( ch, victim, lang, said ) );
  sprintf( buf, "{g$n says you %s'{x$t{g'.{x", language_name_known(victim,lang));
  act( buf, ch, heard, victim, TO_VICT );

  MOBPROG(victim, ch, "onSpeech", ch, heard );

  return;
}

void do_qdirsay( CHAR_DATA *ch, const char *argument ) {
  char arg2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg2 );

  if (IS_SET(ch->comm,COMM_NOCHANNELS)) {
    send_to_char("The gods have revoked your channel priviliges.\n\r",ch);
    return;
  }
 
  if ( IS_SET(ch->comm, COMM_QUIET) ) {
    send_to_char( "You must turn off quiet mode first.\n\r", ch);
    return;
  }

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_qdirsay: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    send_to_charf(ch,"You can't talk.\n\r");
    return;
  }

  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_charf(ch,"You can't talk, you're muted.\n\r");
    return;
  }

  if ( arg2[0] == '\0' 
       || argument[0] == '\0' ) {
    send_to_char( "Dirsay whom what ?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  // Added by SinaC 2000 for talking drunk
  char arg[MAX_STRING_LENGTH];
  strcpy( arg, makedrunk(argument,ch));

  // sentence player says with current language
  char said[MAX_INPUT_LENGTH];
  int lang = get_current_language( ch );
  strcpy( said, str_to_language( ch, ch, lang, arg ) );

  if ( victim->desc == NULL && !IS_NPC(victim)) {
    act("$N seems to have misplaced $S link...try again later.",
	ch,NULL,victim,TO_CHAR);
    // sentence other player hear
    char heard[MAX_INPUT_LENGTH];
    strcpy( heard, str_to_language( ch, victim, lang, said ) );

    sprintf(buf,"{g%s says you %s'{x%s{g'.{x\n\r",
	    PERS(ch,victim), language_name_known(victim,lang), heard);
    //sprintf(buf,"{g%s says you '{x%s{g'.{x\n\r",PERS(ch,victim),argument);
    buf[0] = UPPER(buf[0]);
    add_buf(victim->pcdata->buffer,buf);
    return;
  }

  sprintf(buf,"{gYou say %s'{x$t{g' to $N.{x", language_name_known(ch,lang));
  act( buf, ch, said, victim, TO_CHAR );
  
  // sentence other player hear
  char heard[MAX_INPUT_LENGTH];
  strcpy( heard, str_to_language( ch, victim, lang, said ) );
  sprintf( buf, "{g$n says you %s'{x$t{g'.{x", language_name_known(victim,lang));
  act( buf, ch, heard, victim, TO_VICT );

  MOBPROG(victim, ch, "onSpeech", ch, heard );
  return;
}
