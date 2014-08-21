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
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"

#include "classes.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "update.h"
#include "act_wiz.h"
#include "act_info.h"
#include "save.h"
#include "fight.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "wiznet.h"
#include "interp.h"
#include "ability.h"
#include "config.h"
#include "copyover.h"
#include "utils.h"


/* equips a character */
void do_outfit ( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  int i,sn,vnum;
  // Added by SinaC 2000
  int pra;

  if (ch->level > 5 || IS_NPC(ch)) {
    send_to_char("Find it yourself!\n\r",ch);
    return;
  }
    
  // added by SinaC 2000
  if (ch->carry_number+4 > can_carry_n(ch)) {
    send_to_char("You are carrying too much, try dropping some items.\n\r",ch);
    return;
  }
    
  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) == NULL ) {
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0 );
    obj->cost = 0;
    obj_to_char( obj, ch );
    equip_char( ch, obj, WEAR_LIGHT );
  }

  if ( ( obj = get_eq_char( ch, WEAR_BODY ) ) == NULL ) {
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0 );
    obj->cost = 0;
    obj_to_char( obj, ch );
    equip_char( ch, obj, WEAR_BODY );
  }

  /* do the weapon thing */

  // Removed by SinaC 2000
  /*  if ( ch->cstat(classes) !=( 1<<CLASS_MONK) )*/
  if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL) {

    sn = 0;
    vnum = OBJ_VNUM_SCHOOL_SWORD; /* just in case! */
    pra = 0;

    for (i = 0; weapon_table[i].name != NULL; i++){
      if ( weapon_table[i].gsn == NULL )
	continue;
      // Added by SinaC 2000
      pra = get_ability( ch, sn );

      if ( pra < get_ability(ch,*weapon_table[i].gsn ) )
	/* Modified by SinaC 2000
	   if (ch->pcdata->learned[sn] <
	   ch->pcdata->learned[*weapon_table[i].gsn])
	*/
	{
	  sn = *weapon_table[i].gsn;
	  vnum = weapon_table[i].vnum;
	}
    }

    if ( pra >= 1 + get_ability(ch,gsn_hand_to_hand) ){
      // Modified by SinaC 2000
      //if (ch->pcdata->learned[sn]>=1+ch->pcdata->learned[gsn_hand_to_hand]){
      obj = create_object(get_obj_index(vnum),0);
      obj_to_char(obj,ch);
      equip_char(ch,obj,WEAR_WIELD);
    }
  }

  if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL
       ||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS))
      &&  (obj = get_eq_char( ch, WEAR_SHIELD ) ) == NULL ){
    obj = create_object( get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0 );
    obj->cost = 0;
    obj_to_char( obj, ch );
    equip_char( ch, obj, WEAR_SHIELD );
  }
  // Modified by SinaC 2001 for god
  //send_to_char("You have been equipped by Mota.\n\r",ch);
  send_to_charf( ch,
		 "You have been equipped by %s.\n\r",
		 char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
}

/* Bonus Xp Added by Seytan */
void do_xpbonus( CHAR_DATA *ch, const char *argument) {
  CHAR_DATA *victim;
  char       buf  [ MAX_STRING_LENGTH ];
  char       arg1 [ MAX_INPUT_LENGTH ];
  char       arg2 [ MAX_INPUT_LENGTH ];
  int      value;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) ){
    send_to_char( "Syntax: xpbonus <char> <Exp>.\n\r", ch );
    return;
  }

  if (( victim = get_char_world ( ch, arg1 ) ) == NULL ){
    send_to_char("That player is not here.\n\r", ch);
    return;
  }

  if ( IS_NPC( victim ) ){
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }
  if ( ch == victim ){
    send_to_char( "You may not bonus yourself.\n\r", ch );
    return;
  }
  if (IS_IMMORTAL(victim)){
    send_to_char("You can't bonus immortals silly!\n\r", ch);
    return;
  }
  value = atoi( arg2 );
  if ( value < -5000 || value > 5000 ){
    send_to_char( "Value range is -5000 to 5000.\n\r", ch );
    return;
  }
  if ( value == 0 ){
    send_to_char( "The value cannot be equal to 0.\n\r", ch );
    return;
  }
  gain_exp(victim, value, TRUE );

  sprintf( buf,"You have bonused %s a whopping %d experience points.\n\r",
	   victim->name, value);
  send_to_char(buf, ch);
  if ( value > 0 ){
    sprintf( buf,"You have been bonused %d experience points.\n\r", value );
    send_to_char( buf, victim );
  }
  else{
    sprintf( buf,"You have been penalized %d experience points.\n\r", value );
    send_to_char( buf, victim );
  }

  return;
}

// Modified by SinaC 2000, now mortals can use this command
void do_smote(CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *vch;
  const char *letter,*name;
  char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
  // Modified by SinaC 2001  int  before
  unsigned int matches = 0;
 
  if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) ){
    send_to_char( "You can't show your emotions.\n\r", ch );
    return;
  }
 
  if ( argument[0] == '\0' ){
    send_to_char( "Emote what?\n\r", ch );
    return;
  }
  // Modified by SinaC 2000
  if ( (IS_NPC(ch) && !is_name(argument,ch->name))
       || (strstr(argument,ch->name) == NULL)) {
    send_to_char("You must include your name in an smote.\n\r",ch);
    return;
  }
    
  send_to_char(argument,ch);
  send_to_char("\n\r",ch);
 
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    if (vch->desc == NULL || vch == ch)
      continue;
    if ((letter = strstr(argument,vch->name)) == NULL) {
      send_to_char(argument,vch);
      send_to_char("\n\r",vch);
      continue;
    }
 
    strcpy(temp,argument);
    temp[strlen(argument) - strlen(letter)] = '\0';
    last[0] = '\0';
    name = vch->name;
 
    for (; *letter != '\0'; letter++) {
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
 
    send_to_char(temp,vch);
    send_to_char("\n\r",vch);
  }
 
  return;
}

void do_bamfin( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  if ( !IS_NPC(ch) ) {
    //    smash_tilde( argument );

    if (argument[0] == '\0'){
      if ( ch->pcdata->bamfin == NULL || ch->pcdata->bamfin[0]=='\0' )
	sprintf(buf,"You don't have any poofin.\n\r");
      else
	sprintf(buf,"Your poofin is %s\n\r",ch->pcdata->bamfin);
      send_to_char(buf,ch);
      return;
    }

    if ( strstr(argument,ch->name) == NULL){
      send_to_char("You must include your name.\n\r",ch);
      return;
    }
	     
    ch->pcdata->bamfin = str_dup( argument );

    sprintf(buf,"Your poofin is now %s\n\r",ch->pcdata->bamfin);
    send_to_char(buf,ch);
  }
  return;
}

void do_bamfout( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
 
  if ( !IS_NPC(ch) ){
    //    smash_tilde( argument );
 
    if (argument[0] == '\0'){
      if ( ch->pcdata->bamfout == NULL  || ch->pcdata->bamfout[0]=='\0')
	sprintf(buf,"You don't have any poofout.\n\r");
      else
	sprintf(buf,"Your poofout is %s\n\r",ch->pcdata->bamfout);
      send_to_char(buf,ch);
      return;
    }
 
    if ( strstr(argument,ch->name) == NULL) {
      send_to_char("You must include your name.\n\r",ch);
      return;
    }
 
    ch->pcdata->bamfout = str_dup( argument );
 
    sprintf(buf,"Your poofout is now %s\n\r",ch->pcdata->bamfout);
    send_to_char(buf,ch);
  }
  return;
}

void do_pardon( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' ){
    send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ){
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ){
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "killer" ) ) {
    if ( IS_SET(victim->act, PLR_KILLER) ){
      REMOVE_BIT( victim->act, PLR_KILLER );
      send_to_char( "Killer flag removed.\n\r", ch );
      send_to_char( "You are no longer a KILLER.\n\r", victim );
    }
    return;
  }

  if ( !str_cmp( arg2, "thief" ) ) {
    if ( IS_SET(victim->act, PLR_THIEF) ){
      REMOVE_BIT( victim->act, PLR_THIEF );
      send_to_char( "Thief flag removed.\n\r", ch );
      send_to_char( "You are no longer a THIEF.\n\r", victim );
    }
    return;
  }

  send_to_char( "Syntax: pardon <character> <killer|thief>.\n\r", ch );
  return;
}

void do_echo( CHAR_DATA *ch, const char *argument ) {
  DESCRIPTOR_DATA *d;
    
  if ( argument[0] == '\0' ) {
    send_to_char( "Global echo what?\n\r", ch );
    return;
  }
    
  for ( d = descriptor_list; d; d = d->next ) {
    if ( d->connected == CON_PLAYING ){
      if (get_trust(d->character) >= get_trust(ch))
	send_to_char( "global> ",d->character);
      send_to_char( argument, d->character );
      send_to_char( "\n\r",   d->character );
    }
  }

  return;
}


void do_recho( CHAR_DATA *ch, const char *argument ) {
  DESCRIPTOR_DATA *d;
    
  if ( argument[0] == '\0' ) {
    send_to_char( "Local echo what?\n\r", ch );

    return;
  }

  for ( d = descriptor_list; d; d = d->next ) {
    if ( d->connected == CON_PLAYING
	 &&   d->character->in_room == ch->in_room ){
      if (get_trust(d->character) >= get_trust(ch))
	send_to_char( "local> ",d->character);
      send_to_char( argument, d->character );
      send_to_char( "\n\r",   d->character );
    }
  }

  return;
}

void do_zecho(CHAR_DATA *ch, const char *argument) {
  DESCRIPTOR_DATA *d;

  if (argument[0] == '\0') {
    send_to_char("Zone echo what?\n\r",ch);
    return;
  }

  for (d = descriptor_list; d; d = d->next) {
    if (d->connected == CON_PLAYING
	&&  d->character->in_room != NULL && ch->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area){
      if (get_trust(d->character) >= get_trust(ch))
	send_to_char("zone> ",d->character);
      send_to_char(argument,d->character);
      send_to_char("\n\r",d->character);
    }
  }
}

void do_pecho( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  argument = one_argument(argument, arg);
 
  if ( argument[0] == '\0' || arg[0] == '\0' ){
    send_to_char("Personal echo what?\n\r", ch); 
    return;
  }
   
  if  ( (victim = get_char_world(ch, arg) ) == NULL ){
    send_to_char("Target not found.\n\r",ch);
    return;
  }

  if (get_trust(victim) >= get_trust(ch) && get_trust(ch) != MAX_LEVEL)
    send_to_char( "personal> ",victim);

  send_to_char(argument,victim);
  send_to_char("\n\r",victim);
  send_to_char( "personal> ",ch);
  send_to_char(argument,ch);
  send_to_char("\n\r",ch);
}

void do_transfer( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  DESCRIPTOR_DATA *d;
  CHAR_DATA *victim;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' ) {
    send_to_char( "Transfer whom (and where)?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg1, "all" ) ) {
    for ( d = descriptor_list; d != NULL; d = d->next ){
      if ( d->connected == CON_PLAYING
	   &&   d->character != ch
	   &&   d->character->in_room != NULL
	   &&   can_see( ch, d->character ) ) {
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "%s %s", d->character->name, arg2 );
	do_transfer( ch, buf );
      }
    }
    return;
  }

  /*
   * Thanks to Grodyn for the optional location parameter.
   */
  if ( arg2[0] == '\0' ) {
    location = ch->in_room;
  }
  else {
    if ( ( location = find_location( ch, arg2 ) ) == NULL ){
      send_to_char( "No such location.\n\r", ch );
      return;
    }

    if ( !is_room_owner(ch,location) && room_is_private( location ) 
	 &&  get_trust(ch) < MAX_LEVEL)	{
      send_to_char( "That room is private right now.\n\r", ch );
      return;
    }
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim->in_room == NULL ) {
    send_to_char( "They are in limbo.\n\r", ch );
    return;
  }

  if ( victim->fighting != NULL )
    stop_fighting( victim, TRUE );
  act( "$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, location );
  act( "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );
  if ( ch != victim )
    act( "$n has transferred you.", ch, NULL, victim, TO_VICT );
  do_look( victim, "auto" );
  send_to_char( "Ok.\n\r", ch );
}

void do_at( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  ROOM_INDEX_DATA *original;
  OBJ_DATA *on;
  CHAR_DATA *wch;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' || argument[0] == '\0' ) {
    send_to_char( "At where what?\n\r", ch );
    return;
  }

  if ( ( location = find_location( ch, arg ) ) == NULL ) {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,location) && room_is_private( location ) 
      &&  get_trust(ch) < MAX_LEVEL) {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  original = ch->in_room;
  on = ch->on;
  char_from_room( ch );
  char_to_room( ch, location );
  interpret( ch, argument );

  /*
   * See if 'ch' still exists before continuing!
   * Handles 'at XXXX quit' case.
   */
  for ( wch = char_list; wch != NULL; wch = wch->next ) {
    if ( wch == ch ){
      OBJ_DATA *obj;

      char_from_room( ch );
      char_to_room( ch, original );
      // Modified by SinaC 2000
      //	    ch->on = on;
      /* See if on still exists before setting ch->on back to it. */
      for ( obj = original->contents; obj; obj = obj->next_content ) {
	if ( obj == on ){
	  ch->on = on;
	  break;
	}
      }
      break;
    }
  }

  return;
}

void do_goto( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *location;
  CHAR_DATA *rch;
  int count = 0;

  // Added by SinaC 2001 for goto_default
  if ( argument[0] == '\0' ) {
    if ( IS_NPC(ch) ) {
      send_to_char( "Goto where?\n\r", ch );
      return;
    }
    location = get_room_index( ch->pcdata->goto_default );
    if ( location == NULL ) {
      send_to_char("You default goto location is incorrect.\n\r"
		   "Setting now to 1.\n\r", ch );
      ch->pcdata->goto_default = 1;
      return;
    }
  }
  else
    if ( ( location = find_location( ch, argument ) ) == NULL ) {
      send_to_char( "No such location.\n\r", ch );
      return;
    }

  count = 0;
  for ( rch = location->people; rch != NULL; rch = rch->next_in_room )
    count++;

  if (!is_room_owner(ch,location) && room_is_private(location) 
      &&  (count > 1 || get_trust(ch) < MAX_LEVEL)) {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  if ( ch->fighting != NULL )
    stop_fighting( ch, TRUE );

  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
    // Modified by SinaC 2001
    if ( can_see( rch, ch ) ) {
    //if (get_trust(rch) >= ch->invis_level){
      // Added by SinaC 2000  && ch->pcdata->bamfout != NULL
      if (ch->pcdata != NULL && ch->pcdata->bamfout != NULL && ch->pcdata->bamfout[0] != '\0' )
	act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
      else
	act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
    }
  }

  char_from_room( ch );
  char_to_room( ch, location );


  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
    // Modified by SinaC 2001
    if ( can_see( rch, ch ) ) {
    //if (get_trust(rch) >= ch->invis_level) {
      // Added by SinaC 2000  && ch->pcdata->bamfin != NULL
      if (ch->pcdata != NULL && ch->pcdata->bamfin != NULL && ch->pcdata->bamfin[0] != '\0')
	act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
      else
	act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
    }
  }

  do_look( ch, "auto" );
  return;
}

void do_violate( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *location;
  CHAR_DATA *rch;
 
  if ( argument[0] == '\0' ) {
    send_to_char( "Goto where?\n\r", ch );
    return;
  }
 
  if ( ( location = find_location( ch, argument ) ) == NULL ) {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!room_is_private( location )) {
    send_to_char( "That room isn't private, use goto.\n\r", ch );
    return;
  }
 
  if ( ch->fighting != NULL )
    stop_fighting( ch, TRUE );
 
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
    // Modified by SinaC 2001
    if ( can_see( rch, ch ) ) {
    //if (get_trust(rch) >= ch->invis_level) {
      if (ch->pcdata != NULL && ch->pcdata->bamfout != NULL && ch->pcdata->bamfout[0] != '\0')
	act("$t",ch,ch->pcdata->bamfout,rch,TO_VICT);
      else
	act("$n leaves in a swirling mist.",ch,NULL,rch,TO_VICT);
    }
  }
 
  char_from_room( ch );
  char_to_room( ch, location );
 
 
  for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room) {
    // Modified by SinaC 2001
    if ( can_see( rch, ch ) ) {
    //if (get_trust(rch) >= ch->invis_level) {
      if (ch->pcdata != NULL && ch->pcdata->bamfin != NULL && ch->pcdata->bamfin[0] != '\0')
	act("$t",ch,ch->pcdata->bamfin,rch,TO_VICT);
      else
	act("$n appears in a swirling mist.",ch,NULL,rch,TO_VICT);
    }
  }
 
  do_look( ch, "auto" );
  return;
}

void do_reboo( CHAR_DATA *ch, const char *argument ) {
  send_to_char( "If you want to REBOOT, spell it out.\n\r", ch );
  return;
}

void do_reboot( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d,*d_next;
  CHAR_DATA *vch;

  if ( IS_NPC( ch ) ) {
    send_to_char( "only PC's\n\r", ch );
    return;
  }

  if (ch->invis_level < LEVEL_HERO) {
    sprintf( buf, "Reboot by %s.", ch->name );
    do_echo( ch, buf );
  }

  // Added by SinaC 2003
  save_on_shutdown();

  merc_down = TRUE;
  for ( d = descriptor_list; d != NULL; d = d_next ) {
    d_next = d->next;
    vch = d->original ? d->original : d->character;

    send_to_charf(ch,"Saving %s.\n\r",vch?vch->name:"NULL");

    if (vch != NULL) {
      //save_char_obj(vch);
      new_save_pFile(vch, TRUE );
    }

    close_socket(d);
  }

  return;
}

void do_shutdow( CHAR_DATA *ch, const char *argument ) {
  send_to_char( "If you want to SHUTDOWN, spell it out.\n\r", ch );
  return;
}

void do_shutdown( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d,*d_next;
  CHAR_DATA *vch;

  if ( IS_NPC( ch ) ) {
    send_to_char( "only PC's\n\r", ch );
    return;
  }

  if (ch->invis_level < LEVEL_HERO)
    sprintf( buf, "Shutdown by %s.", ch->name );
  append_file( ch, SHUTDOWN_FILE, buf );
  strcat( buf, "\n\r" );

  if (ch->invis_level < LEVEL_HERO)
    do_echo( ch, buf );

  // Added by SinaC 2003
  save_on_shutdown();

  merc_down = TRUE;
  for ( d = descriptor_list; d != NULL; d = d_next) {
    d_next = d->next;
    vch = d->original ? d->original : d->character;

    log_stringf("Saving %s on shutdown.", vch?vch->name:"NULL");

    send_to_charf(ch,"Saving %s.\n\r",vch?vch->name:"NULL");
    if (vch != NULL) {
      //save_char_obj(vch);
      new_save_pFile(vch, TRUE );
    }

    close_socket(d);
  }
  return;
}

void do_switch( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );
    
  if ( arg[0] == '\0' ) {
    send_to_char( "Switch into whom?\n\r", ch );
    return;
  }

  if ( ch->desc == NULL )
    return;
    
  if ( ch->desc->original != NULL ) {
    send_to_char( "You are already switched.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if (!IS_NPC(victim)) {
    send_to_char("You can only switch into mobiles.\n\r",ch);
    return;
  }

  if (!is_room_owner(ch,victim->in_room) && ch->in_room != victim->in_room 
      &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
    send_to_char("That character is in a private room.\n\r",ch);
    return;
  }

  if ( victim->desc != NULL ) {
    send_to_char( "Character in use.\n\r", ch );
    return;
  }

  sprintf(buf,"$N switches into %s",victim->short_descr);
  wiznet(buf,ch,NULL,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));

  ch->desc->character = victim;
  ch->desc->original  = ch;
  victim->desc        = ch->desc;
  ch->desc            = NULL;
  /* change communications to match */
  if (ch->prompt != NULL)
    victim->prompt = str_dup(ch->prompt);
  victim->comm = ch->comm;
  victim->lines = ch->lines;
  send_to_char( "Ok.\n\r", victim );
  return;
}

void do_return( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  if ( ch->desc == NULL )
    return;

  if ( ch->desc->original == NULL ) {
    send_to_char( "You aren't switched.\n\r", ch );
    return;
  }

  send_to_char( "You return to your original body. Type replay to see any missed tells.\n\r", ch );
  if (ch->prompt != NULL) {
    ch->prompt = NULL;
  }

  sprintf(buf,"$N returns from %s.",ch->short_descr);
  wiznet(buf,ch->desc->original,0,WIZ_SWITCHES,WIZ_SECURE,get_trust(ch));
  ch->desc->character       = ch->desc->original;
  ch->desc->original        = NULL;
  ch->desc->character->desc = ch->desc; 
  ch->desc                  = NULL;
  return;
}

/* trust levels for load and clone */
bool obj_check (CHAR_DATA *ch, OBJ_DATA *obj)
{
  if (IS_TRUSTED(ch,GOD)
      || (IS_TRUSTED(ch,IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
      || (IS_TRUSTED(ch,DEMI)	    && obj->level <= 10 && obj->cost <= 500)
      || (IS_TRUSTED(ch,ANGEL)    && obj->level <=  5 && obj->cost <= 250)
      || (IS_TRUSTED(ch,AVATAR)   && obj->level ==  0 && obj->cost <= 100))
    return TRUE;
  else
    return FALSE;
}

// only clone an item and what it contains
void recursive_clone2(OBJ_DATA *obj, OBJ_DATA *clone) {
  OBJ_DATA *c_obj, *t_obj;

  for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content) {
    t_obj = create_object(c_obj->pIndexData,0);
    clone_object(c_obj,t_obj);
    SET_BIT( t_obj->base_extra, ITEM_NODROP|ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_ROT_DEATH ); 
    SET_BIT( t_obj->extra_flags, ITEM_NODROP|ITEM_NOREMOVE|ITEM_NOUNCURSE|ITEM_ROT_DEATH ); 
    obj_to_obj(t_obj,clone);
    recursive_clone2(c_obj,t_obj);
  }
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone) {
  OBJ_DATA *c_obj, *t_obj;


  for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content) {
    if (obj_check(ch,c_obj)){
      t_obj = create_object(c_obj->pIndexData,0);
      clone_object(c_obj,t_obj);
      obj_to_obj(t_obj,clone);
      recursive_clone(ch,c_obj,t_obj);
    }
  }
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  const char *rest;
  CHAR_DATA *mob;
  OBJ_DATA  *obj;

  rest = one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Clone what?\n\r",ch);
    return;
  }

  if (!str_prefix(arg,"object")){
    mob = NULL;
    obj = get_obj_here(ch,rest);
    if (obj == NULL){
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")) {
    obj = NULL;
    mob = get_char_room(ch,rest);
    if (mob == NULL){
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else{ /* find both */
    mob = get_char_room(ch,argument);
    obj = get_obj_here(ch,argument);
    if (mob == NULL && obj == NULL){
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }

  /* clone an object */
  if (obj != NULL){
    OBJ_DATA *clone;

    if (!obj_check(ch,obj)){
      send_to_char(
		   "Your powers are not great enough for such a task.\n\r",ch);
      return;
    }

    clone = create_object(obj->pIndexData,0); 
    clone_object(obj,clone);
    if (obj->carried_by != NULL)
      obj_to_char(clone,ch);
    else
      obj_to_room(clone,ch->in_room);
    recursive_clone(ch,obj,clone);

    act("$n has created $p.",ch,clone,NULL,TO_ROOM);
    act("You clone $p.",ch,clone,NULL,TO_CHAR);
    wiznet("$N clones $p.",ch,clone,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    return;
  }
  else if (mob != NULL){
    CHAR_DATA *clone;
    OBJ_DATA *new_obj;
    char buf[MAX_STRING_LENGTH];

    if (!IS_NPC(mob)){
      send_to_char("You can only clone mobiles.\n\r",ch);
      return;
    }

    if ((mob->level > 20 && !IS_TRUSTED(ch,GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch,IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch,DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch,ANGEL))
	||  !IS_TRUSTED(ch,AVATAR)){
      send_to_char(
		   "Your powers are not great enough for such a task.\n\r",ch);
      return;
    }

    clone = create_mobile(mob->pIndexData);
    clone_mobile(mob,clone); 
	
    for (obj = mob->carrying; obj != NULL; obj = obj->next_content)	{
      if (obj_check(ch,obj)) {
	new_obj = create_object(obj->pIndexData,0);
	clone_object(obj,new_obj);
	recursive_clone(ch,obj,new_obj);
	obj_to_char(new_obj,clone);
	new_obj->wear_loc = obj->wear_loc;
      }
    }
    char_to_room(clone,ch->in_room);
    act("$n has created $N.",ch,NULL,clone,TO_ROOM);
    act("You clone $N.",ch,NULL,clone,TO_CHAR);
    sprintf(buf,"$N clones %s.",clone->short_descr);
    wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
    return;
  }
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  load mob <vnum>\n\r",ch);
    send_to_char("  load obj <vnum> <level>\n\r",ch);
    return;
  }

  if (!str_cmp(arg,"mob") || !str_cmp(arg,"char")) {
    do_mload(ch,argument);
    return;
  }

  if (!str_cmp(arg,"obj")) {
    do_oload(ch,argument);
    return;
  }
  /* echo syntax */
  do_load(ch,"");
}

void do_mload( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];
    
  one_argument( argument, arg );

  if ( arg[0] == '\0' || !is_number(arg) ) {
    send_to_char( "Syntax: load mob <vnum>.\n\r", ch );
    return;
  }

  if ( ( pMobIndex = get_mob_index( atoi( arg ) ) ) == NULL ) {
    send_to_char( "No mob has that vnum.\n\r", ch );
    return;
  }

  victim = create_mobile( pMobIndex );
  char_to_room( victim, ch->in_room );
  act( "$n has created $N!", ch, NULL, victim, TO_ROOM );
  sprintf(buf,"$N loads %s.",victim->short_descr);
  wiznet(buf,ch,NULL,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
  act( "You have created $N!", ch, NULL, victim, TO_CHAR );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_oload( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  OBJ_DATA *obj;
  int level;
    
  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || !is_number(arg1)) {
    send_to_char( "Syntax: load obj <vnum> <level>.\n\r", ch );
    return;
  }
    
  level = get_trust(ch); /* default */
  
  if ( arg2[0] != '\0'){  /* load with a level */
    if (!is_number(arg2)) {
      send_to_char( "Syntax: oload <vnum> <level>.\n\r", ch );
      return;
    }
    level = atoi(arg2);
    if (level < 0 || level > get_trust(ch)) {
      send_to_char( "Level must be be between 0 and your level.\n\r",ch);
      return;
    }
  }

  if ( ( pObjIndex = get_obj_index( atoi( arg1 ) ) ) == NULL ) {
    send_to_char( "No object has that vnum.\n\r", ch );
    return;
  }

  obj = create_object( pObjIndex, level );
  if ( CAN_WEAR(obj, ITEM_TAKE) ) {
    obj_to_char( obj, ch );
    // Added by SinaC 2001
    recompute(ch);
  }
  else {
    obj_to_room( obj, ch->in_room );
    // Added by SinaC 2001
    recomproom(ch->in_room);
  }
  act( "$n has created $p!", ch, obj, NULL, TO_ROOM );
  wiznet("$N loads $p.",ch,obj,WIZ_LOAD,WIZ_SECURE,get_trust(ch));
  act( "You have created $p!", ch, obj, NULL, TO_CHAR );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_purge( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  DESCRIPTOR_DATA *d;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    /* 'purge' */
    CHAR_DATA *vnext;
    OBJ_DATA  *obj_next;

    for ( victim = ch->in_room->people; victim != NULL; victim = vnext ) {
      vnext = victim->next_in_room;
      if ( IS_NPC(victim) && !IS_SET(victim->act,ACT_NOPURGE) 
	   &&   victim != ch /* safety precaution */ )
	extract_char( victim, TRUE );
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;
      if (!IS_OBJ_STAT(obj,ITEM_NOPURGE))
	extract_obj( obj );
    }

    act( "$n purges the room!", ch, NULL, NULL, TO_ROOM);
    send_to_char( "Ok.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim) ) {
    if (ch == victim) {
      send_to_char("Ho ho ho.\n\r",ch);
      return;
    }

    if (get_trust(ch) <= get_trust(victim))	{
      send_to_char("Maybe that wasn't a good idea...\n\r",ch);
      sprintf(buf,"%s tried to purge you!\n\r",ch->name);
      send_to_char(buf,victim);
      return;
    }

    act("$n disintegrates $N.",ch,0,victim,TO_NOTVICT);

    if (victim->level > 1)
      //save_char_obj( victim );
      new_save_pFile(victim, TRUE );
    d = victim->desc;
    extract_char( victim, TRUE );
    if ( d != NULL )
      close_socket( d );

    return;
  }

  act( "$n purges $N.", ch, NULL, victim, TO_NOTVICT );
  extract_char( victim, TRUE );
  return;
}

void do_advance( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;
  int iLevel;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) ) {
    send_to_char( "Syntax: advance <char> <level>.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
    send_to_char( "That player is not here.\n\r", ch);
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }

  if ( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL-1 ) {
    send_to_charf( ch, "Level must be 1 to %d.\n\r", MAX_LEVEL-1 );
    return;
  }

  if ( level > get_trust( ch ) ){
    send_to_char( "Limited to your trust level.\n\r", ch );
    return;
  }

  /*
   * Lower level:
   *   Reset to level 1.
   *   Then raise again.
   *   Currently, an imp can lower another imp.
   *   -- Swiftest
   */
  if ( level <= victim->level ) {
    int temp_prac;

    send_to_char( "Lowering a player's level!\n\r", ch );
    send_to_char( "**** OOOOHHHHHHHHHH  NNNNOOOO ****\n\r", victim );
    temp_prac = victim->practice;
    victim->level    = 1;
    victim->exp      = exp_per_level(victim,victim->pcdata->points);
    victim->bstat(max_hit)  = 20;
    victim->bstat(max_mana) = 100;
    // Added by SinaC 2001 for mental user
    victim->bstat(max_psp) = 100;
    victim->bstat(max_move) = 100;
    victim->practice = 0;
    victim->hit      = victim->bstat(max_hit);
    victim->mana     = victim->bstat(max_mana);
    // Added by SinaC 2001 for mental user
    victim->psp     = victim->bstat(max_psp);
    victim->move     = victim->bstat(max_move);
    advance_level( victim, TRUE );
    victim->practice = temp_prac;
  }
  else {
    send_to_char( "Raising a player's level!\n\r", ch );
    send_to_char( "**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n\r", victim );
  }

  for ( iLevel = victim->level ; iLevel < level; iLevel++ ) {
    victim->level += 1;
    advance_level( victim,TRUE);
  }
  sprintf(buf,"You are now level %d.\n\r",victim->level);
  send_to_char(buf,victim);
  victim->exp   = exp_per_level(victim,victim->pcdata->points)
    * UMAX( 1, victim->level );
  victim->trust = 0;
  //save_char_obj(victim);
  new_save_pFile(victim, FALSE );

  //recompute(victim); NO NEED: done in advance_level

  return;
}

void do_trust( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int level;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) ) {
    send_to_char( "Syntax: trust <char> <level>.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ){
    send_to_char( "That player is not here.\n\r", ch);
    return;
  }

  if ( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL-1 ) {
    send_to_charf( ch, "Level must be 0 (reset) or 1 to %d.\n\r", MAX_LEVEL-1 );
    return;
  }

  if ( level > get_trust( ch ) ) {
    send_to_char( "Limited to your trust.\n\r", ch );
    return;
  }

  send_to_charf(ch,"%s has now a trust level of %d.\n\r",NAME(victim),level);

  victim->trust = level;
  return;
}

void do_restore( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *vch;
  DESCRIPTOR_DATA *d;

  one_argument( argument, arg );
  if (arg[0] == '\0' || !str_cmp(arg,"room")) {
    /* cure room */
    	
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room){
      affect_strip(vch,gsn_plague);
      affect_strip(vch,gsn_poison);
      affect_strip(vch,gsn_blindness);
      affect_strip(vch,gsn_sleep);
      affect_strip(vch,gsn_curse);

      vch->hit 	= vch->cstat(max_hit);
      vch->mana	= vch->cstat(max_mana);
      // Added by SinaC 2001 for mental user
      vch->psp	= vch->cstat(max_psp);
      vch->move	= vch->cstat(max_move);
      update_pos( vch);
      act("$n has restored you.",ch,NULL,vch,TO_VICT);
    }

    sprintf(buf,"$N restored room %d.",ch->in_room->vnum);
    wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));

    send_to_char("Room restored.\n\r",ch);
    return;

  }
    
  if (!str_cmp(arg,"all")) 
    if ( get_trust(ch) >=  MAX_LEVEL - 1) {
      /* cure all */
    	
      for (d = descriptor_list; d != NULL; d = d->next) {
	victim = d->character;

	if (victim == NULL || IS_NPC(victim))
	  continue;
                
	affect_strip(victim,gsn_plague);
	affect_strip(victim,gsn_poison);
	affect_strip(victim,gsn_blindness);
	affect_strip(victim,gsn_sleep);
	affect_strip(victim,gsn_curse);
            
	victim->hit 	= victim->cstat(max_hit);
	victim->mana	= victim->cstat(max_mana);
	// Added by SinaC 2001 for mental user
	victim->psp	= victim->cstat(max_psp);
	victim->move	= victim->cstat(max_move);
	update_pos( victim);
	if (victim->in_room != NULL)
	  act("$n has restored you.",ch,NULL,victim,TO_VICT);
      }
      send_to_char("All active players restored.\n\r",ch);
      return;
    }
    else {
      send_to_char("Only higher gods are able to restore the world.\n\r",ch);
      return;
    }
      

  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  affect_strip(victim,gsn_plague);
  affect_strip(victim,gsn_poison);
  affect_strip(victim,gsn_blindness);
  affect_strip(victim,gsn_sleep);
  affect_strip(victim,gsn_curse);

  victim->hit  = victim->cstat(max_hit);
  victim->mana = victim->cstat(max_mana);
  // Added by SinaC 2001 for mental user
  victim->psp = victim->cstat(max_psp);
  victim->move = victim->cstat(max_move);
  update_pos( victim );
  act( "$n has restored you.", ch, NULL, victim, TO_VICT );
  sprintf(buf,"$N restored %s",
	  IS_NPC(victim) ? victim->short_descr : victim->name);
  wiznet(buf,ch,NULL,WIZ_RESTORE,WIZ_SECURE,get_trust(ch));
  send_to_char( "Ok.\n\r", ch );
  return;
}

// Modified by SinaC 2000
void do_peace( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *rch;
  if ( argument[0] == '\0' ) {
    for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room ){
      if ( rch->fighting != NULL )
	stop_fighting( rch, TRUE );
      if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	REMOVE_BIT(rch->act,ACT_AGGRESSIVE);

      if ( ch != rch ) send_to_char("{WPeace.{x\n\r",rch);
    }
    send_to_char( "Ok.\n\r", ch );
  }
  else if (!str_cmp(argument,"all")) {
    rch = get_char( ch );
	
    for ( rch = char_list; rch; rch = rch->next ) {
      if ( rch->desc == NULL || rch->desc->connected != CON_PLAYING )
	continue;
	    
      if ( rch->fighting ) {
	act("{W$n has declared World Peace.{x\n\r", ch, NULL, rch, TO_VICT);
	stop_fighting( rch, TRUE );
	if (IS_NPC(rch) && IS_SET(rch->act,ACT_AGGRESSIVE))
	  REMOVE_BIT(rch->act,ACT_AGGRESSIVE);
      }
    }

    send_to_char( "You have declared World Peace.\n\r", ch );
    return;
  }
  else {
    send_to_char("Syntax :\n\r", ch );
    send_to_char("  peace\n\r", ch );
    send_to_char("  peace all\n\r", ch );
    return;
  }
  return;
}

void do_wizlock( CHAR_DATA *ch, const char *argument ) {
  wizlock = !wizlock;

  if ( wizlock ){
    wiznet("$N has wizlocked the game.",ch,NULL,0,0,0);
    send_to_char( "Game wizlocked.\n\r", ch );
  }
  else {
    wiznet("$N removes wizlock.",ch,NULL,0,0,0);
    send_to_char( "Game un-wizlocked.\n\r", ch );
  }

  return;
}

/* RT anti-newbie code */
void do_newlock( CHAR_DATA *ch, const char *argument ) {
  newlock = !newlock;
 
  if ( newlock ){
    wiznet("$N locks out new characters.",ch,NULL,0,0,0);
    send_to_char( "New characters have been locked out.\n\r", ch );
  }
  else{
    wiznet("$N allows new characters back in.",ch,NULL,0,0,0);
    send_to_char( "Newlock removed.\n\r", ch );
  }
 
  return;
}

void do_slookup( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int sn;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Lookup which skill or spell or power or song or all?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) ) {
    for ( sn = 0; sn < MAX_ABILITY; sn++ ){
      if ( ability_table[sn].name == NULL )
	break;
      sprintf( buf, "Sn: %3d  Slot: %3d  %10s: '%s'\n\r",
	       sn, ability_table[sn].slot, 
	       abilitytype_name(ability_table[sn].type), ability_table[sn].name );
      send_to_char( buf, ch );
    }
  }
  else {
    if ( ( sn = ability_lookup( arg ) ) < 0 ){
      send_to_char( "No such skill or spell or power or song.\n\r", ch );
      return;
    }

    sprintf( buf, "Sn: %3d  Slot: %3d  %10s: '%s'\n\r",
	     sn, ability_table[sn].slot, 
	     abilitytype_name(ability_table[sn].type), ability_table[sn].name );
    send_to_char( buf, ch );
  }

  return;
}

/*
 * Revised Socket command, written for Envy by Stimpy, ported
 * to Rom2.4 by Silverhand, make sure you follow his agreements
 * basically just email him that you are using the code. :)
 *
 */
void do_sockets( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA       *vch;
  DESCRIPTOR_DATA *d;
  char            buf  [ MAX_STRING_LENGTH ];
  char            buf2 [ MAX_STRING_LENGTH ];
  int             count;
  const char *          st;
  char            s[100];
  char            idle[10];
  count       = 0;
  buf[0]      = '\0';
  buf2[0]     = '\0';

  strcat( buf2, "\n\r[Num Connected_State Login@ Idl] Player name  Host\n\r" );
  strcat( buf2, "--------------------------------------------------------------------------\n\r");
  for ( d = descriptor_list; d; d = d->next ) {
    if ( d->character && can_see( ch, d->character ) ) {
      // Added by SinaC 2001
      bool cheater = FALSE;
      for ( DESCRIPTOR_DATA *d1 = descriptor_list; d1; d1 = d1->next )
	if ( d1->character && can_see( ch, d1->character ) )
	  if ( d != d1 && !str_cmp( d->host, d1->host ) ) {
	    cheater = TRUE;
	    break;
	  }
	  

      /* NB: You may need to edit the CON_ values */
      switch( d->connected ) {
      case CON_PLAYING:              st = "    PLAYING    ";    break;
      case CON_GET_NAME:             st = "   Get Name    ";    break;
      case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
      case CON_CONFIRM_NEW_NAME:     st = " Confirm Name  ";    break;
      case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
      case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
      case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
      case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
      case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
      case CON_GET_ALIGNMENT:        st = " Get Alignment ";    break;
      case CON_DEFAULT_CHOICE:       st = "   Customize?  ";    break;
      case CON_GEN_GROUPS:           st = "  Customizing  ";    break;
      case CON_PICK_WEAPON:          st = "Picking weapon ";    break;
      case CON_READ_MOTD:            st = "  Reading MOTD ";    break;
      case CON_READ_IMOTD:           st = " Reading IMOTD ";    break;
      case CON_BREAK_CONNECT:        st = " Reconnecting? ";    break;
      case CON_COPYOVER_RECOVER:     st = "CopyOvr Recover";    break;
      case CON_NOTE_TO:              st = "NOTE: To       ";    break;
      case CON_NOTE_SUBJECT:         st = "NOTE: Subject  ";    break;
      case CON_NOTE_EXPIRE:          st = "NOTE: Expire   ";    break;
      case CON_NOTE_TEXT:            st = "NOTE: Text     ";    break;
      case CON_NOTE_FINISH:          st = "NOTE: Finishing";    break;
	// Added by SinaC 2000
      case CON_GET_GOD:              st = "      Get God  ";    break;
	// Added by SinaC 2001 for rebirth
      case CON_REBIRTH:              st = "      Rebirth  ";    break;
      case CON_REMORT:               st = "      Remort   ";    break;
	// SinaC 2003
      case CON_ANSI:                 st = "    Get Ansi   ";    break;
      case CON_GET_HOMETOWN:         st = "  Get Hometown ";    break;
      default:                       st = "   !UNKNOWN!   ";    break;
      }
      count++;

      /* Format "login" value... */
      vch = d->original ? d->original : d->character;
      strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );

      if ( vch->level <= LEVEL_HERO && vch->timer > 0 )
	sprintf( idle, "%-2d", vch->timer );
      else
	sprintf( idle, "  " );

      // Added by SinaC 2001
      char bufhost[128];
      if ( cheater )
	sprintf(bufhost,"{R%s", d->host);
      else	
	sprintf(bufhost,"%s", d->host);

      sprintf( buf, "[%3d %s %7s %2s] %-12s %-32.32s{x\n\r",
	       d->descriptor,
	       st,
	       s,
	       idle,
	       ( d->original ) ? d->original->name
	       : ( d->character )  ? d->character->name
	       : "(None!)",
	       //d->host );
	       bufhost );

      strcat( buf2, buf );
    }
  }
  sprintf( buf, "\n\r%d user%s\n\r", count, count == 1 ? "" : "s" );
  strcat( buf2, buf );
  send_to_char( buf2, ch );
  return;
}

/*
 * New routines by Dionysos.
 */
void do_invis( CHAR_DATA *ch, const char *argument ) {
  int level;
  char arg[MAX_STRING_LENGTH];

  /* RT code for taking a level argument */
  one_argument( argument, arg );

  if ( IS_NPC( ch ) ) {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' ) 
    /* take the default path */
    if ( ch->invis_level) {
      ch->invis_level = 0;
      act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\n\r", ch );
    }
    else {
      ch->invis_level = get_trust(ch);
      act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }
  else{
    /* do the level thing */
    level = atoi(arg);
    if (level < 2 || level > get_trust(ch)) {
      send_to_char("Invis level must be between 2 and your level.\n\r",ch);
      return;
    }
    else {
      ch->reply = NULL;
      ch->invis_level = level;
      act( "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\n\r", ch );
    }
  }

  return;
}

void do_incognito( CHAR_DATA *ch, const char *argument ) {
  int level;
  char arg[MAX_STRING_LENGTH];
 
  /* RT code for taking a level argument */
  one_argument( argument, arg );
 
  if ( arg[0] == '\0' )
    /* take the default path */
    if ( ch->incog_level) {
      ch->incog_level = 0;
      act( "$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You are no longer cloaked.\n\r", ch );
    }
    else {
      ch->incog_level = get_trust(ch);
      act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You cloak your presence.\n\r", ch );
    }
  else{
    /* do the level thing */
    level = atoi(arg);
    if (level < 2 || level > get_trust(ch)) {
      send_to_char("Incog level must be between 2 and your level.\n\r",ch);
      return;
    }
    else {
      ch->reply = NULL;
      ch->incog_level = level;
      act( "$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You cloak your presence.\n\r", ch );
    }
  }
 
  return;
}

void do_holylight( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) )
    return;

  if ( IS_SET(ch->act, PLR_HOLYLIGHT) ) {
    REMOVE_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char( "Holy light mode off.\n\r", ch );
  }
  else {
    SET_BIT(ch->act, PLR_HOLYLIGHT);
    send_to_char( "Holy light mode on.\n\r", ch );
  }

  return;
}

/* prefix command: it will put the string typed on each line typed */

void do_prefi (CHAR_DATA *ch, const char *argument) {
  send_to_char("You cannot abbreviate the prefix command.\r\n",ch);
  return;
}

void do_prefix (CHAR_DATA *ch, const char *argument) {
  char buf[MAX_INPUT_LENGTH];

  if (argument[0] == '\0') {
    if (ch->prefix[0] == '\0'){
      send_to_char("You have no prefix to clear.\r\n",ch);
      return;
    }

    send_to_char("Prefix removed.\r\n",ch);
    ch->prefix = str_dup("");
    return;
  }

  if (ch->prefix[0] != '\0') {
    sprintf(buf,"Prefix changed to %s.\r\n",argument);
  }
  else {
    sprintf(buf,"Prefix set to %s.\r\n",argument);
  }

  ch->prefix = str_dup(argument);
}

void do_spy( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  BUFFER *output;
  DESCRIPTOR_DATA *d;
  int immmatch;
  int mortmatch;
  int hptemp;

  /*
   * Initalize Variables.
   */
  immmatch = 0;
  mortmatch = 0;
  buf[0] = '\0';
  output = new_buf();
  /*
   * Count and output the IMMs.
   */
  sprintf( buf, " ----Immortals:----\n\r");
  add_buf(output,buf);
  sprintf( buf, "Name           Level  Wiz     Incog   [ Vnum]\n\r");
  add_buf(output,buf);
  for ( d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *wch;
    if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
      continue;

    wch   = CH(d);
    if (!can_see(ch,wch)  || !IS_IMMORTAL(wch))
      continue;

    immmatch++;
    sprintf( buf, "%-14s %3d    %-3d     %-3d     [%5d]\n\r",
	     wch->name,
	     wch->level,
	     wch->invis_level,
	     wch->incog_level,
	     wch->in_room->vnum);
    add_buf(output,buf);
  }


  /*
   * Count and output the Morts.
   */
  sprintf( buf, " \n\r ----Mortals:----\n\r");
  add_buf(output,buf);
  sprintf( buf, "Name           Race/Class      Position        Lev  %%hps  [ Vnum]\n\r");
  add_buf(output,buf);
  hptemp = 0;

  for ( d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *wch;
    char const *cla;

    if ( d->connected != CON_PLAYING || !can_see( ch, d->character ) )
      continue;

    wch   = ( d->original != NULL ) ? d->original : d->character;
    if (!can_see(ch,wch) || wch->level > ch->level || IS_IMMORTAL(wch))
      continue;

    mortmatch++;
    if ((wch->cstat(max_hit) != wch->hit) && (wch->hit > 0))
      hptemp = (wch->hit*100)/wch->cstat(max_hit);
    else if (wch->cstat(max_hit) == wch->hit)
      hptemp = 100;
    else if (wch->hit < 0)
      hptemp = 0;

    cla = class_whoname(wch->cstat(classes));
    char race[10];
    if ( race_table[wch->cstat(race)].pc_race )
      strcpy( race, pc_race_table[wch->cstat(race)].who_name ); // PC race
    else {
      strncpy( race, race_table[wch->cstat(race)].name, 5 ); // NPC race
      race[5] = '\0';
    }
    sprintf( buf, "%-14s\t%5s/%3s%s\t%-15s\t%-2d\t%3d%%\t[%5d]\n\r",
	     wch->name,
	     //wch->cstat(race)<MAX_PC_RACE?pc_race_table[wch->cstat(race)].who_name:"     ",
	     race,
	     cla,
	     wch->isWildMagic?" W":"  ", // SinaC 2003
	     capitalize( position_table[wch->position].name) ,
	     wch->level,
	     hptemp,
	     wch->in_room->vnum);
    add_buf(output,buf);
  }
  /*
   * Tally the counts and send the whole list out.
   */
  sprintf( buf2, "\n\rIMMs found: %d\n\r", immmatch );
  add_buf(output,buf2);
  sprintf( buf2, "Morts found: %d\n\r", mortmatch );
  add_buf(output,buf2);
  page_to_char( buf_string(output), ch );
  return;
}



