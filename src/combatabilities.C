// Added by SinaC 2000
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include "magic.h"
#include "recycle.h"

// Added by SinaC 2001
#include "classes.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "fight.h"
#include "act_comm.h"
#include "act_move.h"
#include "act_obj.h"
#include "gsn.h"
#include "olc_value.h"
#include "const.h"
#include "interp.h"
#include "ability.h"
#include "utils.h"
#include "damage.h"


void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );

/*************************************************************************************\
 ******************************** COMBAT SKILLS **************************************
\*************************************************************************************/

void do_rescue( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *fch;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Rescue whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( "What about fleeing instead?\n\r", ch );
    return;
  }

  // SinaC 2000  I don't see why a player couldn't help his pet
  /*
    if ( !IS_NPC(ch) && IS_NPC(victim) )
    {
    send_to_char( "Doesn't need your help!\n\r", ch );
    return;
    }
  */

  if ( ch->fighting == victim ) {
    send_to_char( "Too late.\n\r", ch );
    return;
  }

  if ( ( fch = victim->fighting ) == NULL ) {
    send_to_char( "That person is not fighting right now.\n\r", ch );
    return;
  }

  if ( IS_NPC(fch) && !is_same_group(ch,victim)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }

  WAIT_STATE( ch, BEATS(gsn_rescue) );
  if ( number_percent( ) > get_ability(ch,gsn_rescue)) {
    send_to_char( "You fail the rescue.\n\r", ch );
    check_improve(ch,gsn_rescue,FALSE,1);
    return;
  }

  act( "You rescue $N!",  ch, NULL, victim, TO_CHAR    );
  act( "$n rescues you!", ch, NULL, victim, TO_VICT    );
  act( "$n rescues $N!",  ch, NULL, victim, TO_NOTVICT );
  check_improve(ch,gsn_rescue,TRUE,1);

  stop_fighting( fch, FALSE );
  stop_fighting( victim, FALSE );
  // Added by SinaC 2000 to avoid a bug msg
  stop_fighting( ch, FALSE );

  check_killer( ch, fch );
  set_fighting( ch, fch );
  set_fighting( fch, ch );
  return;
}

void do_kick( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;

  if ( get_ability(ch,gsn_kick) == 0 ) {
    send_to_char("You'd better leave the martial arts to fighters.\n\r", ch );
    return;
  }
  
  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }
  
  WAIT_STATE( ch, BEATS(gsn_kick) );
  if ( get_ability(ch,gsn_kick) > number_percent()) {
    ability_damage(ch,victim,number_range( 1, 2*ch->level ), gsn_kick,DAM_BASH,TRUE, FALSE);
    check_improve(ch,gsn_kick,TRUE,1);
  }
  else {
    ability_damage( ch, victim, 0, gsn_kick,DAM_BASH,TRUE, FALSE);
    check_improve(ch,gsn_kick,FALSE,1);
  }
  check_killer(ch,victim);
  return;
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 */
void disarm( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) { // Modified by SinaC 2003: param obj added
  //  OBJ_DATA *obj;

  //  if ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
  //    return;
  if ( obj == NULL )
    return;

  if ( IS_OBJ_STAT(obj,ITEM_NOREMOVE)) {
    act("$S weapon won't budge!",ch,NULL,victim,TO_CHAR);
    act("$n tries to disarm you, but your weapon won't budge!",
	ch,NULL,victim,TO_VICT);
    act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
    return;
  }
  
 // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj, ch, "onRemoving", args);
  if ( result ) // if onRemoving returns 1,  ch doesn't disarm victim
    return;

  //act( "$n DISARMS you and sends your weapon flying!", 
  //ch, NULL, victim, TO_VICT    );
  act( "{W$n DISARMS you and sends your weapon flying!{x", 
       ch, NULL, victim, TO_VICT    );
  act( "You disarm $N!",  ch, NULL, victim, TO_CHAR    );
  act( "$n disarms $N!",  ch, NULL, victim, TO_NOTVICT );
  
  obj_from_char( obj );

  // Added by SinaC 2003, if result == true, item doesn't want to be dropped
  Value args2[] = {ch};
  result = objprog( obj,ch, "onDropping", args2);
  
    // Modified by SinaC 2003
  if ( IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_INVENTORY) 
       || result )
    obj_to_char( obj, victim );
  else {
    obj_to_room( obj, victim->in_room );
    OBJPROG(obj,victim,"onDropped",victim); // Added by SinaC 2003
    if (IS_NPC(victim) && victim->wait == 0 && can_see_obj(victim,obj))
      get_obj(victim,obj,NULL);
    // Added by SinaC 2001
    else
      recomproom(victim->in_room);
  }

  // Added by SinaC 2003
  MOBPROG( victim, ch, "onDisarmed", ch, obj );

  // Added by SinaC 2001
  //recompute(ch); NO NEED: done by obj_from_char -> unequip_char (unequipped items doesn't give affect)
  
  return;
}

// Bash has 3 levels
//  level 2 allows to bash door or object
//  level 3 increases damage
void do_bash( CHAR_DATA *ch, const char *argument ){
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int door;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit = NULL;
  EXIT_DATA *pexit_rev = NULL;
  OBJ_DATA *obj = NULL;
  int target;
  int chance;

  one_argument(argument,arg);
  if ((chance = get_ability(ch,gsn_bash)) == 0
      || (IS_NPC(ch) 
	  && !IS_SET(ch->off_flags,OFF_BASH)) ) {
    send_to_char("Bashing? What's that?\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    target = 2; // char
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't fighting anyone!\n\r",ch);
      return;
    }
  }
  // Modified by SinaC 2003, allow bashing doors or object
  else {
    if ((victim = get_char_room(ch,arg)) != NULL)
      target = 2; // char
    else if ( ( obj = get_obj_here( ch, arg ) ) != NULL )
      target = 1; // obj
    else if ( ( door = find_door( ch, arg ) ) >= 0 ) {
      pexit = ch->in_room->exit[door];
      if ( ( to_room   = pexit->u1.to_room            ) != NULL
	   && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	   && pexit_rev->u1.to_room == ch->in_room )
	target = 0; // door
      else {
	send_to_char("You cannot bash that.\n\r", ch );
	bug("do_bash: Invalid room %d (door %d)", to_room?to_room->vnum:-1, door );
	return;
      }
    } 
    else {
      send_to_char("You can't find anything to bash.\n\r", ch);
      return;
    }
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }

  int skill_level = get_casting_level( ch, gsn_bash );
  
  switch ( target ) {
  case 2:{ // char
    if (victim->position < POS_FIGHTING) {
      act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
      return;
    }
    if (victim == ch) {
      send_to_char("You try to bash your brains out, but fail.\n\r",ch);
      return;
    }
    if (is_safe(ch,victim))
      return;
    if ( IS_NPC(victim) 
	 && victim->fighting != NULL 
	 && !is_same_group(ch,victim->fighting)) {
      send_to_char("Kill stealing is not permitted.\n\r",ch);
      return;
    }
    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
      act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
      return;
    }
    // Added by SinaC 2000
    if ( !can_see( ch, victim ) ) {
      send_to_char("You get a running start, and slam right into a wall.\n\r",ch);
      return;
    }
    /* modifiers */
    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;
    if (ch->cstat(size) < victim->cstat(size))
      chance += (ch->cstat(size) - victim->cstat(size)) * 15;
    else
      chance += (ch->cstat(size) - victim->cstat(size)) * 10; 
    /* stats */
    chance += ch->cstat(STR);
    chance -= (victim->cstat(DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
      chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
      chance -= 30;
    /* level */
    chance += (ch->level - victim->level);
    if (!IS_NPC(victim) 
	&& chance < get_ability(victim,gsn_dodge) ) {
      /*
	act("$n tries to bash you, but you dodge it.",ch,NULL,victim,TO_VICT);
	act("$N dodges your bash, you fall flat on your face.",ch,NULL,victim,TO_CHAR);
	WAIT_STATE(ch,skill_table[gsn_bash].beats);
	return;*/
      chance -= 3 * (get_ability(victim,gsn_dodge) - chance);
    }
    /* now the attack */
    if (number_percent() < chance ) {
      act("$n sends you sprawling with a powerful bash!",
	  ch,NULL,victim,TO_VICT);
      act("You slam into $N, and send $M flying!",ch,NULL,victim,TO_CHAR);
      act("$n sends $N sprawling with a powerful bash.",
	  ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_bash,TRUE,1);
      int dam = number_range(2*skill_level,2 + 2 * ch->cstat(size) + chance/20);
      // Modified by SinaC 2001
      int done = ability_damage(ch,victim,dam,gsn_bash,
				DAM_BASH, TRUE, FALSE);
      if ( done == DAMAGE_DONE ) {
	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	WAIT_STATE(ch,BEATS(gsn_bash));
	victim->position = POS_RESTING;
	switch(	skill_level ) {
	default: case 0: case 1: if ( number_percent()<=25 ) knock_off_item( ch, victim, FALSE ); break;
	case 2: if ( number_percent()<=33 ) knock_off_item( ch, victim, FALSE ); break;
	case 3: if ( number_percent()<=45 ) knock_off_item( ch, victim, TRUE ); break;
	}
	// Added by SinaC 2000 and modified by SinaC 2001
      }
      else {
	ability_damage(ch,victim,0,gsn_bash,DAM_BASH,FALSE, FALSE);
	act("You fall flat on your face!",ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.", ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.", ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_bash,FALSE,1);
	ch->position = POS_RESTING;
	WAIT_STATE(ch,BEATS(gsn_bash) * 3/2); 
      }
      check_killer(ch,victim);
    }
    break;
  }
  case 1: {// object, only container and portal
    if ( obj->item_type != ITEM_CONTAINER && obj->item_type != ITEM_PORTAL ) {
      send_to_char("You can't bash that.\n\r", ch );
      return;
    }
    if ( skill_level < 2 ) { // in order to bash a door or an item, we must have bash 2
      send_to_char("You have to practice at least in bash level 2 in order to bash an item.\n\r", ch );
      return;
    }
    if ( obj->item_type == ITEM_CONTAINER ) { // contained, only if closeable and closed
      if ( !IS_SET(obj->value[1], CONT_CLOSED) ) { 
	send_to_char( "It's already open.\n\r",      ch ); 
	return; 
      }
      if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) ) { 
	send_to_char( "You can't do that.\n\r",      ch ); 
	return; 
      }
    } 
    else if ( obj->item_type == ITEM_PORTAL ) { // portal, only if is door and closed
      if (!IS_SET(obj->value[1], EX_ISDOOR)) {
	send_to_char("You can't do that.\n\r",ch);
	return;
      }
      if (!IS_SET(obj->value[1], EX_CLOSED)) {
	send_to_char("It's already open.\n\r",ch);
	return;
      }
    }
    if ( ch->move < 50 ) {
      send_to_char("You are too exhausted.\n\r", ch );
      return;
    }
    if ( number_percent() < chance ) {
      if ( obj->item_type == ITEM_CONTAINER ) {
	REMOVE_BIT(obj->value[1], CONT_CLOSED);
	REMOVE_BIT(obj->value[1], CONT_LOCKED);
      }
      else if ( obj->item_type == ITEM_PORTAL ) {
	REMOVE_BIT(obj->value[1], EX_CLOSED);
	REMOVE_BIT(obj->value[1], EX_LOCKED);
	SET_BIT(obj->value[1], EX_BASHED);
      }
      act("You slam into $p with a powerful bash it!",ch,obj,NULL,TO_CHAR);
      act("$n slams $p with a powerful bash.",ch,obj,NULL,TO_ROOM);
      check_improve( ch, gsn_bash,TRUE,3);
    }
    else {
      act("You try to bash it but failed.\n\r", ch,NULL,NULL,TO_CHAR );
      act("$n tries to bash $p but failed.\n\r", ch,obj,NULL,TO_CHAR );
      check_improve( ch, gsn_bash,FALSE,3);
    }
    WAIT_STATE(ch,BEATS(gsn_bash) * 3/2); 
    ch->move = UMIN( ch->move, ch->move-20);
    break;
  }
  case 0: { // door
    if ( skill_level < 2 ) { // in order to bash a door or an item, we must have bash 2
      send_to_char("You have to practice at least in bash level 2 in order to bash a door.\n\r", ch );
      return;
    }
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) ) { 
      send_to_char( "It's already open.\n\r",      ch ); 
      return; 
    }
    if ( ch->move < 50 ) {
      send_to_char("You are too exhausted.\n\r", ch );
      return;
    }
    if ( number_percent() < chance
	 && !IS_SET(pexit->exit_info, EX_BASHPROOF ) // SinaC 2003
	 // infuriating door are harder to bash
	 && !(IS_SET(pexit->exit_info, EX_INFURIATING ) 
	      && number_percent() > get_ability( ch, gsn_bash ) / 2 ) ) {
      REMOVE_BIT(pexit->exit_info, EX_CLOSED);
      REMOVE_BIT(pexit->exit_info, EX_LOCKED);
      SET_BIT(pexit->exit_info, EX_BASHED);
      // reverse exit
      REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
      REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
      SET_BIT(pexit_rev->exit_info, EX_BASHED);
      act("You slam into $d with a powerful bash it!",ch,NULL,pexit->keyword,TO_CHAR);
      act("$n slams $d with a powerful bash.",ch,NULL,pexit->keyword,TO_ROOM);
      check_improve( ch, gsn_bash,TRUE,3);
    }
    else {
      act("You try to bash it but failed.\n\r", ch,NULL,NULL,TO_CHAR );
      act("$n tries to bash $d but failed.\n\r", ch,NULL,pexit->keyword,TO_CHAR );
      check_improve( ch, gsn_bash,FALSE,3);
    }
    WAIT_STATE(ch,BEATS(gsn_bash) * 3/2); 
    ch->move = UMIN( ch->move, ch->move-20);
    break;
  }
  }
}

void do_dirt( CHAR_DATA *ch, const char *argument ){
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);

  if ( (chance = get_ability(ch,gsn_dirt)) == 0 ) {
    send_to_char("You get your feet dirty.\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't in combat!\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(victim,AFF_BLIND)) {
    act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Very funny.\n\r",ch);
    return;
  }
  
  if (is_safe(ch,victim))
    return;

  if (IS_NPC(victim)
      && victim->fighting != NULL 
      && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
    act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  /* modifiers */

  /* dexterity */
  chance += ch->cstat(DEX);
  chance -= 2 * victim->cstat(DEX);

  /* Oxtal > Who wrote this stupid code? */

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level) * 2;

  /* sloppy hack to prevent false zeroes */
  if (chance % 5 == 0)
    chance += 1;

  /* terrain */

  // Modified by SinaC 2001
  switch(ch->in_room->cstat(sector)) {
  case(SECT_INSIDE):		chance -= 20;	break;
  case(SECT_CITY):		chance -= 10;	break;
  case(SECT_FIELD):		chance +=  5;	break;
  case(SECT_FOREST):				break;
  case(SECT_HILLS):				break;
  case(SECT_MOUNTAIN):		chance -= 10;	break;
  case(SECT_WATER_SWIM):	chance  =  0;	break;
  case(SECT_WATER_NOSWIM):	chance  =  0;	break;
  case(SECT_AIR):		chance  =  0;  	break;
  case(SECT_DESERT):		chance += 10;   break;
  case(SECT_UNDERWATER):        chance  = 0;    break;
  default:                      chance  = 0;    break;
  }
  
  if (chance == 0) {
    send_to_char("There isn't any dirt to kick.\n\r",ch);
    return;
  }

  /* now the attack */
  if (number_percent() < chance) {
    AFFECT_DATA af;

    // Color added by SinaC 2003
    act("$n is blinded by the dirt in $s eyes!",victim,NULL,NULL,TO_ROOM);
    act("{W$n kicks dirt in your eyes!{x",ch,NULL,victim,TO_VICT);
    int done = ability_damage(ch,victim,number_range(2,5),gsn_dirt,DAM_NONE,TRUE, FALSE);
    if ( done == DAMAGE_DONE ) {
      send_to_char("You can't see a thing!\n\r",victim);
      check_improve(ch,gsn_dirt,TRUE,2);
      WAIT_STATE(ch,BEATS(gsn_dirt));
      
      //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,0,ch->level,gsn_dirt);
      //affect_to_char(victim,&af);
      //afsetup(af,CHAR,hitroll,ADD,-4,0,ch->level,gsn_dirt);
      //affect_to_char(victim,&af);
      createaff(af,0,ch->level,gsn_dirt,0,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,-4);
      addaff(af,CHAR,affected_by,OR,AFF_BLIND);
      affect_to_char( victim, &af );
    }
  }
  else {
    ability_damage(ch,victim,0,gsn_dirt,DAM_NONE,TRUE, FALSE);
    check_improve(ch,gsn_dirt,FALSE,2);
    WAIT_STATE(ch,BEATS(gsn_dirt));
  }
  check_killer(ch,victim);
}

void do_trip( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);

  if ( (chance = get_ability(ch,gsn_trip)) == 0 ) {
    send_to_char("Tripping?  What's that?\n\r",ch);
    return;
  }

  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't fighting anyone!\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
    
  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }

  if (is_safe(ch,victim))
    return;

  if (IS_NPC(victim) 
      && victim->fighting != NULL 
      && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(victim,AFF_FLYING)) {
    act("$S feet aren't on the ground.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (victim->position < POS_FIGHTING) {
    act("$N is already down.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (victim == ch) {
    send_to_char("You fall flat on your face!\n\r",ch);
    WAIT_STATE(ch,2*BEATS(gsn_trip));
    act("$n trips over $s own feet!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  if (IS_AFFECTED(ch,AFF_CHARM) 
      && ch->master == victim) {
    act("$N is your beloved master.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  /* modifiers */

  /* size */
  if (ch->cstat(size) < victim->cstat(size))
    chance += (ch->cstat(size) - victim->cstat(size)) * 10;  /* bigger = harder to trip */

  /* dex */
  chance += ch->cstat(DEX);
  chance -= victim->cstat(DEX) * 3 / 2;

  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) 
      || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) 
      || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 20;

  /* level */
  chance += (ch->level - victim->level) * 2;
  
  /* now the attack */
  if (number_percent() < chance) {
    act("$n trips you and you go down!",ch,NULL,victim,TO_VICT);
    act("You trip $N and $N goes down!",ch,NULL,victim,TO_CHAR);
    act("$n trips $N, sending $M to the ground.",ch,NULL,victim,TO_NOTVICT);
    check_improve(ch,gsn_trip,TRUE,1);
    
    WAIT_STATE(ch,BEATS(gsn_trip));
    victim->position = POS_RESTING;
    int dam = number_range(2, 2 + 2 * victim->cstat(size));
    int done = ability_damage(ch,victim, dam, gsn_trip, DAM_BASH,TRUE, FALSE);
    if ( done == DAMAGE_DONE )
      DAZE_STATE(victim,2 * PULSE_VIOLENCE);
  }
  else {
    ability_damage(ch,victim,0,gsn_trip,DAM_BASH,TRUE, FALSE);
    WAIT_STATE(ch,BEATS(gsn_trip)*2/3);
    check_improve(ch,gsn_trip,FALSE,1);
  } 
  check_killer(ch,victim);
}

void do_disarm( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance,hth,ch_weapon,vict_weapon,ch_vict_weapon;

  hth = 0;

  if ((chance = get_ability(ch,gsn_disarm)) == 0) {
    send_to_char( "You don't know how to disarm opponents.\n\r", ch );
    return;
  }
  
  if ( get_eq_char( ch, WEAR_WIELD ) == NULL
       && ((hth = get_ability(ch,gsn_hand_to_hand)) == 0) ) {
    send_to_char( "You must wield a weapon, or master hand to hand to disarm.\n\r", ch );
    return;
  }
  
  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Modified by SinaC 2003: secondary/thirdly and fourthly weapon can be disarmed
  if ( ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
       && ( ( obj = get_eq_char( victim, WEAR_SECONDARY ) ) == NULL )
       && ( ( obj = get_eq_char( victim, WEAR_THIRDLY ) ) == NULL )
       && ( ( obj = get_eq_char( victim, WEAR_FOURTHLY ) ) == NULL ) ) {
    send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
    return;
  }
  
  /* find weapon skills */
  int ch_sn = get_weapon_sn(ch,1); // first wield
  int victim_sn = get_weapon_sn(victim,obj); // first wield but obj already got in previous test
  ch_weapon = get_weapon_ability(ch,ch_sn);
  vict_weapon = get_weapon_ability(victim,victim_sn);
  ch_vict_weapon = get_weapon_ability(ch,victim_sn);

  /* modifiers */

  /* skill */
  if ( get_eq_char(ch,WEAR_WIELD) == NULL)
    chance = chance * hth/150;
  else
    chance = chance * ch_weapon/100;

  chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

  /* dex vs. strength */
  chance += ch->cstat(DEX);
  chance -= 2 * victim->cstat(STR);

  /* level */
  chance += (ch->level - victim->level) * 2;
 
  /* if victim has deathgrip */
  if ( is_affected( victim, gsn_deathgrip) )
    chance -= victim->level/10;

  /* and now the attack */
  // Modified by SinaC 2000 for grip skill
  if (number_percent() < chance) {
    if ((chance = get_ability(victim,gsn_grip)) == 0) {
      WAIT_STATE( ch, BEATS(gsn_disarm));
      disarm( ch, victim, obj );
      check_improve(ch,gsn_disarm,TRUE,1);
      return;
    }
    if (number_percent() > (4*chance)/5) {
      WAIT_STATE( ch, BEATS(gsn_disarm) );
      disarm( ch, victim, obj );
      check_improve(ch,gsn_disarm,TRUE,1);
      check_improve(victim,gsn_grip,FALSE,1);
      return;
    }
    check_improve(victim,gsn_grip,TRUE,1);
    /*
      WAIT_STATE( ch, skill_table[gsn_disarm].beats );
      disarm( ch, victim );
      check_improve(ch,gsn_disarm,TRUE,1);
    */
  }
  /*
    else
    {
  */
  WAIT_STATE(ch,BEATS(gsn_disarm));
  act("You fail to disarm $N.",ch,NULL,victim,TO_CHAR);
  act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
  act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
  check_improve(ch,gsn_disarm,FALSE,1);
  //    }
  //check_killer(ch,victim);
  return;
}

void do_circle( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  if ( victim == ch ) {
    send_to_char( "How can you sneak up on yourself?\n\r", ch );
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }
  
  if ( is_safe( ch, victim ) )
    return;

  if (IS_NPC(victim) 
      && victim->fighting != NULL 
      && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if ( ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
       || obj->item_type != ITEM_WEAPON ) {
    send_to_char( "You need to wield a weapon to circle.\n\r", ch );
    return;
  }
  
  // SinaC 2000, can't circle a person we can't see
  if ( !can_see( ch, victim ) ) {
    send_to_char("You stumble blindly into a wall.\n\r",ch);
    return;
  }
  
  if ( IS_NPC(victim) && IS_SET(victim->act, ACT_AWARE ) ) {
    act( "$N is suspicious ... you can't sneak up.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( victim->hit < victim->cstat(max_hit) / 4 ) {
    act( "$N is hurt and suspicious ... you can't sneak up.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }
  
  if ( ch->move < 100 && !IS_NPC(ch) ) {
    send_to_char("You don't have enough moves left to use circle.\n\r", ch );
    return;
  }
  
  check_killer( ch, victim );
  WAIT_STATE( ch, BEATS(gsn_circle) );
  
  if ( !IS_NPC( ch ) ) 
    ch->move -= 100;
  
  // Added by SinaC 2003
  int chance = get_ability(ch,gsn_circle);
  if ( number_percent() < get_ability( ch, gsn_assassination )/2 ) {
    chance += 20; // 20% more chance to succeed a circle if assassination test
    check_improve(ch,gsn_assassination,TRUE,5);
  }
  else
    check_improve(ch,gsn_assassination,FALSE,8);

  if ( number_percent( ) < chance // Modified by SinaC 2003
       || ( get_ability(ch,gsn_circle) >= 2 && !IS_AWAKE(victim) ) ) {
    check_improve(ch,gsn_circle,TRUE,1);
    multi_hit( ch, victim, gsn_circle );
  }
  else {
    check_improve(ch,gsn_circle,FALSE,1);
    ability_damage( ch, victim, 0, gsn_circle,DAM_NONE,TRUE, FALSE);
  }

  return;
}

void do_backstab( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if (arg[0] == '\0') {
    send_to_char("Backstab whom?\n\r",ch);
    return;
  }
  
  if (ch->fighting != NULL) {
    send_to_char("You're facing the wrong end.\n\r",ch);
    return;
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  if ( victim == ch ) {
    send_to_char( "How can you sneak up on yourself?\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if (IS_NPC(victim) 
      && victim->fighting != NULL 
      && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if ( ( ( obj = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
       || obj->item_type != ITEM_WEAPON ) {
    send_to_char( "You need to wield a weapon to backstab.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) && IS_SET(victim->act, ACT_AWARE ) ) {
    act( "$N is suspicious ... you can't sneak up.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }
  
  if ( victim->hit < victim->cstat(max_hit) / 3 ) {
    act( "$N is hurt and suspicious ... you can't sneak up.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }

  // Added by SinaC 2000, the character must be hidden, sneaking or invisible to
  //  backstab a victim, only for PC's
  if ( can_see( victim, ch ) 
       && !IS_NPC( ch )
       // Added by SinaC 2003,  33% chance to succeed a backstab if victim sees ch
       && number_percent() >= get_ability( ch, gsn_assassination )/3 ) {
    if ( number_percent() < 40 ) {
      check_killer( ch, victim );
      
      check_improve(ch,gsn_backstab,FALSE,1);
      ability_damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE, FALSE);
    } 
    else 
      act( "You can't sneak up on $N...", ch, NULL, victim, TO_CHAR );
    return;
  }

  // Added by SinaC 2003 for assassination
  int chance = get_ability(ch,gsn_backstab);
  if ( number_percent() < get_ability( ch, gsn_assassination )/2 ) {
    chance += 20; // 20% more chance to succeed a circle if assassination test
    check_improve(ch,gsn_assassination,TRUE,5);
  }
  else
    check_improve(ch,gsn_assassination,FALSE,8);
  
  check_killer( ch, victim );
  WAIT_STATE( ch, BEATS(gsn_backstab) );
  if ( number_percent( ) < chance//get_ability(ch,gsn_backstab)    Modified by SinaC 2003
       || ( get_ability(ch,gsn_backstab) >= 2 && !IS_AWAKE(victim) ) ) {
    check_improve(ch,gsn_backstab,TRUE,1);
    multi_hit( ch, victim, gsn_backstab );
  }
  else {
    check_improve(ch,gsn_backstab,FALSE,1);
    ability_damage( ch, victim, 0, gsn_backstab,DAM_NONE,TRUE, FALSE);
  }
  
  return;
}


void do_roundhousekick( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  bool missed;
  CHAR_DATA *pChar, *pChar_next;

  if ( get_ability(ch,gsn_roundhousekick) == 0 ) {
    send_to_char( "You'd better leave the martial arts to fighters.\n\r", ch );
    return;
  }
  
  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }
  
  WAIT_STATE( ch, BEATS(gsn_roundhousekick) );
  /*
  if ( get_ability(ch,gsn_roundhousekick) > number_percent())
    {
      damage(ch,victim,number_range( ch->level/2, 3*ch->level/2 ), gsn_roundhousekick,DAM_BASH,TRUE, FALSE);
      check_improve(ch,gsn_roundhousekick,TRUE,1);
    }
  else
    {
      damage( ch, victim, 0, gsn_roundhousekick,DAM_BASH,TRUE, FALSE);
      check_improve(ch,gsn_roundhousekick,FALSE,1);
    }
  check_killer(ch,victim);
  */
  missed =  get_ability(ch,gsn_roundhousekick ) < number_percent();
  pChar_next = NULL;   
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next ) {
    pChar_next = pChar->next_in_room;
    if ( pChar == ch || is_safe( ch, pChar ) ) continue;
    if (missed) {
      ability_damage( ch, pChar, 0, gsn_roundhousekick,DAM_BASH,TRUE, FALSE);
      check_improve(ch,gsn_roundhousekick,FALSE,1);
    }
    else {
      // Bug fixed :))
      ability_damage(ch,pChar,number_range( 2*ch->level, 4*ch->level ), gsn_roundhousekick,DAM_BASH,TRUE, FALSE);
      check_improve(ch,gsn_roundhousekick,TRUE,1);
    }
  }
  return;
}

// Added by SinaC 2000
void do_whirlwind( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *pChar;
  CHAR_DATA *pChar_next;
  OBJ_DATA *wield;
  bool found = FALSE;

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }
   
  if ( get_ability( ch, gsn_whirlwind ) == 0 ) {
    send_to_char( "You don't know how to do that...\n\r", ch );
    return;
  }
  
  if ( ( ( wield = get_eq_char( ch, WEAR_WIELD ) ) == NULL )
       || wield->item_type != ITEM_WEAPON ) {
    send_to_char( "You need to wield a weapon first...\n\r", ch );
    return;
  }
 
  act( "$n holds $p firmly, and starts spinning round...", ch, wield, NULL, TO_ROOM );
  act( "You hold $p firmly, and start spinning round...",  ch, wield, NULL, TO_CHAR );
   
  pChar_next = NULL;   
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next ) {
    pChar_next = pChar->next_in_room;
    if ( pChar == ch || is_safe( ch, pChar ) ) 
      continue;
    found = TRUE;
    act( "$n turns towards YOU!", ch, NULL, pChar, TO_VICT    );
    one_hit( ch, pChar, gsn_whirlwind, 1 );
  }
  
  if ( !found ) {
    act( "$n looks dizzy, and a tiny bit embarassed.", ch, NULL, NULL, TO_ROOM );
    act( "You feel dizzy, and a tiny bit embarassed.", ch, NULL, NULL, TO_CHAR );
  }
  
  WAIT_STATE( ch, BEATS(gsn_whirlwind) );
   
  if ( !found && number_percent() < 25 ) {
    act( "$n loses $s balance and falls into a heap.",  ch, NULL, NULL, TO_ROOM );
    act( "You lose your balance and fall into a heap.", ch, NULL, NULL, TO_CHAR );
    ch->stunned = 2;
  }
  
  return;
}      

void do_lure( CHAR_DATA *ch, const char *argument ) {
  char  dt, chance;
  CHAR_DATA *victim;
  char  arg [ MAX_INPUT_LENGTH ];

  if ( !IS_NPC( ch )
       && get_ability( ch, gsn_lure ) <= 0 ) {
    send_to_char("You don't know how to lure.\n\r", ch );
    return;
  }
  
  if ( !ch->fighting ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  one_argument( argument, arg );
  victim = ch->fighting;

  if ( arg[0] != '\0' )
    if ( !( victim = get_char_room( ch, arg ) ) ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }
  
  if ( !can_see( ch, victim ) ) {
    send_to_char("You can't see that target.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }

  chance = get_ability( ch, gsn_lure ) / 3;

  if ( !can_see( victim, ch ) ) {// if the victim can't see us
    if ( get_ability(victim,gsn_blindfight) >= number_percent() ) // if blindfight
      chance *= 2;                                              // chance * 2
    else                                                        // else chance = 100
      chance = 100;
  }
  
  dt = TYPE_UNDEFINED;
  
  WAIT_STATE( ch, BEATS(gsn_lure) );
  if ( number_percent( ) < chance ) {
    act("$n lures $N into a vulnerable position.", ch, NULL, victim, TO_ROOM );
    send_to_char( "Your opponent has lured you into a vulnerable position.\n\r", victim );
    send_to_char( "You lure your opponent into a vulnerable position.\n\r", ch);
    one_hit( ch, victim, dt, 1 );
    
    if(IS_AFFECTED(ch, AFF_HASTE))            one_hit( ch, victim, dt, 1 );
    
    if(get_ability( ch, gsn_second_attack)>0 )	one_hit( ch, victim, dt, 1 ); 
    if(get_ability( ch, gsn_third_attack)>0 ) 	one_hit( ch, victim, dt, 1 );
    if(get_ability( ch, gsn_fourth_attack)>0 )	one_hit( ch, victim, dt, 1 );
    if(get_ability( ch, gsn_fifth_attack)>0 )	one_hit( ch, victim, dt, 1 );
    if(get_ability( ch, gsn_sixth_attack)>0 )  	one_hit( ch, victim, dt, 1 );
    if(get_ability( ch, gsn_seventh_attack)>0 )	one_hit( ch, victim, dt, 1 );
    if ( IS_AFFECTED(ch, AFF_BERSERK) )	one_hit( ch, victim, dt, 1 );
    if ( get_eq_char (ch, WEAR_SECONDARY)  
	 && ( number_percent() < get_ability(ch,gsn_dual_wield ) ) )
      one_hit( ch, victim, dt, 2 );
    if ( get_eq_char (ch, WEAR_THIRDLY)  
	 && ( number_percent() < get_ability(ch,gsn_third_wield ) ) )
      one_hit( ch, victim, dt, 3 );
    if ( get_eq_char (ch, WEAR_FOURTHLY)  
	 && ( number_percent() < get_ability(ch,gsn_fourth_wield ) ) )
      one_hit( ch, victim, dt, 4 );
    
    check_improve( ch, gsn_lure, TRUE, 1 );
  }
  check_improve( ch, gsn_lure, FALSE, 1 );
  
  return;
}

void do_spike(CHAR_DATA *ch,const char *argument) {
  CHAR_DATA *victim;
  int chance, dam;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);
  if (IS_NPC(ch))
    return;

  if ( (chance=get_ability(ch,gsn_spike)) <= 0 ) {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    send_to_char("Attempt to spike which undead?\n\r",ch);
    return;
  }
  
  if ( (victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  if (victim->fighting != NULL || victim->position == POS_FIGHTING) {
    send_to_char("They are moving around too much to get in close for the kill.\n\r",ch);
    return;
  }
  
  /*
   *if (!IS_SET(victim->act,ACT_UNDEAD)) {
   *  send_to_char("You can only spike your weapon against undead.\n\r",ch);
   *  return;
   *}
   */
  if ( !IS_SET(victim->act,ACT_UNDEAD)
       || !IS_SET(victim->cstat(form),FORM_UNDEAD)) {
    send_to_char("You can only spike your weapon against undead.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim))
    return;

  chance /= 2;
  chance += ch->level;
  chance -= victim->level * 3/2;
  chance -= number_range(0,15);
  
  if (!can_see(victim,ch)) chance += 10;

  if (victim->position == POS_FIGHTING) chance -= 25;
  else if (victim->position == POS_SLEEPING) chance += 10;
  else chance -= 10;

  chance /= 2;
  chance = URANGE(2,chance,90);

  act("$n strikes out at $N with deadly intensity.",ch,0,victim,TO_NOTVICT);
  act("You strike out at $N with deadly intensity.",ch,0,victim,TO_CHAR);
  act("$n strikes at you with deadly intensity.",ch,0,victim,TO_VICT);

  WAIT_STATE(ch,BEATS(gsn_spike));

  if (number_percent() < chance) {
    send_to_char("With agonising pain your skull is smashed by the blow!\n\r",victim);
    act("Your blow shatters $N's skull into bloody fragments!",ch,0,victim,TO_CHAR);
    act("$N's skull is shattered into bits of mangled flesh and bone by $n's strike!",ch,0,victim,TO_NOTVICT);
    
    //raw_kill(ch,victim);
    sudden_death( ch, victim );
    
    check_improve(ch,gsn_spike,TRUE,2);
    return;
  }
  else {
    send_to_char("You feel a sharp pain searing your skull!\n\r",victim);
    act("Your deathstrike smashes $N's skull but fails to kill.",ch,0,victim,TO_CHAR);
    dam = ch->level * 2;
    dam += dice(ch->level, 4);
    ability_damage(ch,victim,dam,gsn_spike,DAM_PIERCE,TRUE,FALSE);
    check_improve(ch,gsn_spike,FALSE,1);
  }
  
  return;
}

void do_pugil(CHAR_DATA *ch,const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance;
  int dam;

  one_argument(argument,arg);

  if ( (chance = get_ability(ch,gsn_pugil)) == 0
       ||  IS_NPC(ch) ) {
    send_to_char("You're not trained in the art of pugiling.\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't fighting anyone.\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  if (ch->fighting == NULL) {
    send_to_char("You can't pugil someone like that, you must be fighting.\n\r",ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("That would be a bit stupid.\n\r", ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  obj = get_eq_char(ch,WEAR_WIELD);
  if (obj == NULL 
      || obj->item_type != ITEM_WEAPON // SinaC 2003
      || ( obj->value[0] != WEAPON_SPEAR 
	   && obj->value[0] != WEAPON_STAFF ) ) { // Added by SinaC 2003
    send_to_char("You must be wielding a staff or a spear to pugil.\n\r",ch);
    return;
  }
  
  chance += (ch->level - victim->level);
  chance = URANGE(5, chance, 90);

  WAIT_STATE(ch,BEATS(gsn_pugil));

  if (number_percent() < chance) {
    act("You smash $N with a bone crushing pugil!",ch,NULL,victim,TO_CHAR);
    act("$n smashes you with a bone crushing pugil!",ch,NULL,victim,TO_VICT);
    act("$n pugils $N with a bone crushing pugil!",ch,NULL,victim,TO_NOTVICT);
    
    check_improve(ch,gsn_pugil,TRUE,1);
    dam = dice(GET_WEAPON_DNUMBER(obj),GET_WEAPON_DTYPE(obj));
    dam += (get_ability(ch,gsn_enhanced_damage) * dam/100);
    
    if (ch->level <= 40)      dam *= number_range(10,13);
    else if (ch->level <= 50) dam *= number_range(11,14);
    else if (ch->level <= 60) dam *= number_range(12,15);
    else if (ch->level <= 70) dam *= number_range(12,17);
    else if (ch->level <= 80) dam *= number_range(13,18);
    else                      dam *= number_range(14,20);
    
    dam /= 10;
    
    // Modified by SinaC 2003
    //damage(ch,victim,dam,gsn_pugil, attack_table[obj->value[3]].damage, TRUE,FALSE);
    ability_damage(ch,victim,dam,gsn_pugil,DAM_PIERCE,TRUE,FALSE);
  }	
  else {
    check_improve(ch,gsn_pugil,FALSE,1);
    
    ability_damage(ch,victim,0,gsn_pugil,DAM_NONE,TRUE,FALSE);
  }
  
  return;
}

void do_lash(CHAR_DATA *ch,const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *weapon;

  one_argument(argument,arg);
 
  if ( (chance = get_ability(ch,gsn_lash)) == 0 ) {   
    send_to_char("You don't have the skill to lash people's legs.\n\r",ch);
    return;
  }
  
  weapon = get_eq_char(ch,WEAR_WIELD);
  if (weapon == NULL 
      || weapon->item_type != ITEM_WEAPON // SinaC 2003
      || (weapon->value[0] != WEAPON_WHIP
	  && weapon->value[0] != WEAPON_FLAIL) ) {
    chance -= 15;
    //      weapon = get_eq_char(ch,WEAR_DUAL_WIELD);
    weapon = get_eq_char (ch, WEAR_SECONDARY) ;
  }
  if (weapon == NULL
      || weapon->item_type != ITEM_WEAPON // SinaC 2003
      ) {
    send_to_char("You aren't wielding any weapon to lash with.\n\r",ch);
    return;
  }
  if (weapon->value[0] != WEAPON_WHIP && weapon->value[0] != WEAPON_FLAIL) {
    send_to_char("You need to be wielding a whip or flail to lash.\n\r",ch);
    return;
  }

  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't fighting anyone!\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if (victim->position == POS_SLEEPING
      || victim->position == POS_RESTING) {
    act("$N isn't on $S feet.",ch,NULL,victim,TO_CHAR);
    return;
  } 
  
  if (victim == ch) {
    send_to_char("You try to lash your feet and look clumsy doing it.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  if (is_safe(ch,victim))
    return;
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
    act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE)) chance += 5;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE)) chance -= 15;

  chance += ch->cstat(DEX)/2;
  chance -= victim->cstat(DEX)/2;

  if (IS_AFFECTED(victim,AFF_FLYING))
    chance -= dice(2,5);

    /* level */
  chance += (ch->level - victim->level)*3;

  if (!IS_NPC(ch) && !IS_NPC(victim)
      && (victim->fighting == NULL || ch->fighting == NULL)) {
    sprintf(buf,"Help! %s is lashing me!",PERS(ch,victim));
    do_yell(victim,buf);
  }
  if (number_percent() > chance) {
    act("$n lashes at $N's legs but misses.",ch,0,victim,TO_NOTVICT);
    act("$n lashes at your legs but misses.",ch,0,victim,TO_VICT);
    act("You lash at $N's legs but miss.",ch,0,victim,TO_CHAR);
    check_improve(ch,gsn_lash,FALSE,1);
    WAIT_STATE(ch,BEATS(gsn_lash));
    return;
  }
  act("$n lashes $N's legs, sending $M crashing to the ground.",ch,0,victim,TO_NOTVICT);
  act("$n lashes your legs, sending you crashing to the ground.",ch,0,victim,TO_VICT);
  act("You lash $N's legs, sending $M crashing to the ground.",ch,0,victim,TO_CHAR);

  check_improve(ch,gsn_lash,TRUE,1);

  int done = ability_damage(ch,victim,dice(2,7),gsn_lash,DAM_BASH,TRUE,FALSE);
  
  if ( done == DAMAGE_DONE ) {
    WAIT_STATE(victim,2 * PULSE_VIOLENCE);
    WAIT_STATE(ch,BEATS(gsn_lash));
    
    victim->position = POS_RESTING;
    victim->stunned = 1;
  }
  return;
}

void do_shield_cleave(CHAR_DATA *ch,const char *argument) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int chance;
  OBJ_DATA *weapon;
  OBJ_DATA *shield;
  bool using_primary = TRUE;
  CHAR_DATA *victim;

  one_argument(argument,arg);
  if (arg[0] == '\0')
    victim = ch->fighting;
  else
    victim = get_char_room(ch,arg);

  chance = get_ability(ch,gsn_shield_cleave);
  if (chance == 0 ) {
    send_to_char("You don't know the methods to cleave a shield in two.\n\r",ch);
    return;
  }
  weapon = get_eq_char(ch,WEAR_WIELD);
  if (weapon == NULL
      || weapon->item_type != ITEM_WEAPON // SinaC 2003
      ) {
    weapon = get_eq_char(ch,WEAR_SECONDARY);
    using_primary = FALSE;
  }
  if (weapon == NULL
      || weapon->item_type != ITEM_WEAPON // SinaC 2003
      || (weapon->value[0] != WEAPON_SWORD
	  && weapon->value[0] != WEAPON_AXE) ) {
    send_to_char("You must be wielding an axe or a sword to cleave a shield.\n\r",ch);
    return;
  }
  if (victim == NULL) {
    send_to_char("But they aren't here.\n\r",ch);
    return;
  }
  if (victim == ch) {
    send_to_char("That isn't possible.\n\r",ch);
    return;
  }
  if ((shield = get_eq_char(victim,WEAR_SHIELD)) == NULL) {
    send_to_char("But they aren't using a shield.\n\r",ch);
    return;
  }
  // Added by SinaC 2001
  if ( IS_SET(shield->extra_flags, ITEM_NOCOND) ) {
    act("$N's shield is unbreakable.", ch, NULL, victim, TO_CHAR );
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  chance *= 9;
  chance /= 10;
  chance += (ch->level - victim->level)*3;
  chance -= shield->level;
  if (!using_primary)
    chance -= 15;
  if (!IS_NPC(victim) && ch->fighting != victim) {
    sprintf(buf,"Help! %s just cleaved my shield!",ch->name);
    do_yell(victim,buf);
  }
  if (number_percent() > chance) {
    act("$n makes a might blow at $N's shield but fails to cleave it.",ch,0,victim,TO_NOTVICT);
    act("$n lands a mighty blow to your shield but fails to cleave it.",ch,0,victim,TO_VICT);
    act("You strike a mighty blow to $N's shield but fail to cleave it.",ch,0,victim,TO_CHAR);
    check_improve(ch,gsn_shield_cleave,FALSE,1);
    WAIT_STATE(ch,BEATS(gsn_shield_cleave));
    multi_hit(victim,ch,TYPE_UNDEFINED);
    return;
  }
  act("$n's mighty blow cleaves $N's shield in half!",ch,0,victim,TO_NOTVICT);
  act("Your might blow cleaves $N's shield in half!",ch,0,victim,TO_CHAR);
  act("$n strikes your shield with powerful force, cleaving it in two!",ch,0,victim,TO_VICT);
  extract_obj(shield);
  WAIT_STATE(ch,BEATS(gsn_shield_cleave));
  check_improve(ch,gsn_shield_cleave,TRUE,1);
  multi_hit(victim,ch,TYPE_UNDEFINED);
  
  return;
}

void do_nerve(CHAR_DATA *ch,const char *argument) {
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int chance;

  if ( (chance = get_ability(ch,gsn_nerve)) == 0 ) {
    send_to_char("You don't know how to use nerve pressure tactics.\n\r",ch);
    return;
  }
  
  one_argument(argument,arg);
  if (arg[0] == '\0')
    victim = ch->fighting;
  else
    victim = get_char_room(ch,arg);

  if (victim == NULL) {
    send_to_char("Attempt to put pressure on who's nerves?\n\r",ch);
    return;
  }
  if (victim == ch) {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim))
    return;

  if (is_affected(victim,gsn_nerve)) {
    send_to_char("They have already been weakened using nerve pressure.\n\r",ch); 
    return;
  }
       
  chance += (ch->level - victim->level)*3;
  chance -= victim->cstat(DEX)/3;
  chance += ch->cstat(DEX)/2;
  chance -= victim->cstat(CON)/3;

  if (number_percent() > chance) {
    act("$n grasps $N's arm but fails to apply the right pressure point.",ch,0,victim,TO_NOTVICT);
    act("You grasp $N's arm but fail to apply the right pressure point.",ch,0,victim,TO_CHAR);
    act("$n grasps your arm but fails to apply the right pressure point.",ch,0,victim,TO_VICT);
    check_improve(ch,gsn_nerve,FALSE,3);
    WAIT_STATE(ch,PULSE_VIOLENCE);
    return;
  }
  else {
    act("$n grasps $N's arm and weakens $m with pressure points.",ch,0,victim,TO_NOTVICT);
    act("You grasp $N's arm and weaken $m with pressure points.",ch,0,victim,TO_CHAR);
    act("$n grasps your arm and weakens you with pressure point.",ch,0,victim,TO_VICT);
    check_improve(ch,gsn_nerve,TRUE,3);
    //afsetup( af, CHAR, STR, ADD, -3, ch->level/5, ch->level, gsn_nerve );
    //affect_to_char( victim, &af );
    createaff(af,ch->level/5,ch->level,gsn_nerve,0,AFFECT_ABILITY);
    addaff(af,CHAR,STR,ADD,-3);
    affect_to_char( victim, &af );
    
    WAIT_STATE(ch,PULSE_VIOLENCE);
  }
  
  if (!IS_NPC(ch) && !IS_NPC(victim)
      && (ch->fighting == NULL
	  || victim->fighting == NULL) ) {
    sprintf(buf,"Help, %s is attacking me!",ch->name);
    do_yell(victim,buf);
  }
  if (victim->fighting == NULL) {
    multi_hit(victim,ch,TYPE_UNDEFINED);
  }
  
  return;
}

void do_cleave(CHAR_DATA *ch,const char *argument) {
  OBJ_DATA *weapon;
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int dam, chance, dam_type;
  char buf[MAX_STRING_LENGTH];
  int sn;
  int skill;

  if ( ( chance = get_ability(ch,gsn_cleave)) == 0 ) {
    send_to_char("You don't know how to cleave.\n\r",ch);
    return;
  }
  
  one_argument(argument,arg);
  weapon = get_eq_char(ch,WEAR_WIELD);
  
  if ( arg[0] == '\0' ) {
    send_to_char("You need to specify a target.\n\r", ch );
    return;
  }

  if (weapon == NULL
      || weapon->item_type != ITEM_WEAPON // SinaC 2003
      || ( weapon->value[0] != WEAPON_SWORD
	   && weapon->value[0] != WEAPON_AXE ) ) {
    send_to_char("You need to wield a sword or axe to cleave.\n\r",ch);
    return;
  }
  
  if ( (victim = get_char_room(ch,arg)) == NULL ) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  if (victim == ch) {
    send_to_char("You can't do such a thing.\n\r",ch);
    return;
  }
  
  if (victim->fighting != NULL) {
    send_to_char("They are moving too much to cleave.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  if (victim->hit < victim->cstat(max_hit)*9/10) {
    send_to_char("They are too hurt and watchful to cleave right now...\n\r",ch);
    return;
  }
  
  if (is_safe(ch,victim))
    return;
  
  chance /= 5;
  
  if ((ch->level - victim->level) < 0)
    chance -= (ch->level - victim->level)*3;
  else
    chance += ( ch->level - victim->level );

  chance += ch->cstat(STR)/2;
  chance -= victim->cstat(DEX)/3;  /* Improve evasion */
  chance -= victim->cstat(STR)/4;  /* Improve repelling */
  chance -= victim->cstat(CON)/4;  /* Shock survival */
  dam_type = attack_table[GET_WEAPON_DAMTYPE(weapon)].damage;

  if (!can_see(victim,ch)) chance += 5;

  chance -= dice(2,6);

  chance = URANGE(2,chance,10);

  if (number_percent() < 50)
    sprintf(buf,"Die, %s you butchering fool!",PERS(ch,victim));
  else
    sprintf(buf,"Help! %s just tried to cleave me in half!",PERS(ch,victim));

  //sn = get_weapon_sn(ch, FALSE );
  sn = get_weapon_sn(ch, weapon);
  skill = get_weapon_ability(ch,sn) + get_ability(ch, gsn_cleave ) + 10;
  skill = URANGE(0,skill/2,100);

  act("You make a brutal swing at $N in an attempt to cleave them in half.",ch,0,victim,TO_CHAR);
  act("$n attempts to cleave you in half with a brutal slice.",ch,0,victim,TO_VICT);
  act("$n makes an attempt to cleave $N in half.",ch,0,victim,TO_NOTVICT);

  /*
  if (IS_NPC(victim))
    victim->last_fought = ch;
  */

  WAIT_STATE( ch, BEATS(gsn_cleave));

  if (number_percent() > chance) {
    check_improve(ch,gsn_cleave,FALSE,5);
    if (weapon->pIndexData->new_format)
      //dam = dice(weapon->value[1],weapon->value[2])*skill/100;
      dam = (dice(GET_WEAPON_DNUMBER(weapon),GET_WEAPON_DTYPE(weapon))*skill)/100;
    else
      //dam = (number_range(weapon->value[1],weapon->value[2])*skill)/100;
      dam = (number_range(GET_WEAPON_DNUMBER(weapon),GET_WEAPON_DTYPE(weapon))*skill)/100;
    dam *= number_range(ch->level/10,ch->level/7);
    ability_damage(ch,victim,dam,gsn_cleave,dam_type,TRUE,FALSE);
    
    if (!IS_NPC(victim) && !IS_NPC(ch) && victim->hit > 1)
      do_yell(victim,buf);
    return;
  }
  
  act("Your cleave slices $S body in half with a clean cut!",ch,0,victim,TO_CHAR);
  act("$n cleaves you in half, tearing your body into two bloody bits!",ch,0,victim,TO_VICT);
  act("$n cleaves $N into to bits of bloody flesh!",ch,0,victim,TO_NOTVICT);
  check_improve(ch,gsn_cleave,TRUE,5);

//  raw_kill(ch,victim);
  sudden_death( ch, victim );

  return;
}

/* Stake skill for ROM 2.4b6 v 1.0*/


/* I put vampires on my mud.. way stupid for they are powerful.  They
   almost overran the mud.. til I gave clerics this skill.  It is used
   against The Vampire race and certain undead classes.  If you wish
   you can modify this to work with everyone.  It is used not
   once.. but many times in a battle.  Very powerful skill for
   CLERICS.

   put all the normal stuff for skills gsn's, const.c, interp.c and h.
   (I assume you know this)

   Put this in fight.c or file of your choice and watch people get
   staked!!
   (actually really fun to watch vampires get staked by clerics.)
   A note:  clerics are always hard to play and so very few are ever
   made.  This equalizes them some.. not enough but some in the battle
   department.

   Okay..  This is copywrited by me.. The Mage.  (Yes it is my handle
   but My real name is linked to it) Use it.. enjoy.. give me credit
   if you wish.  send me mail saying you are using it please.
   lordmage@erols.com

   The Mage IMP of The Mage's Lair.
*/

/* STAKE skill by The Mage */
void do_stake( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  
  one_argument(argument,arg);
  
  if ( (chance = get_ability(ch,gsn_stake)) == 0 ) { 
    send_to_char("Stake? What's that?\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("Stake What undead??\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  /*
  if (!IS_NPC(victim) 
      && victim->race != race_lookup("vampire")) {
    send_to_char("You cannot stake a non-vampire player.\n\r",ch);
    return;
  }
  if (IS_NPC(victim) 
      && (!is_name("vampire",victim->name) 
	  && !is_name("undead", victim->name) 
	  && !is_name("zombie", victim->name) 
	  && !is_name("corpse", victim->name))) {

	send_to_char("You cannot stake this mob.\n\r",ch);
	return;
      }
  if (IS_NPC(victim) 
      && (!IS_SET(victim->act,ACT_UNDEAD))) {
    send_to_char("You cannot stake the living.\n\r",ch);
    return;
  }
  */
  if ( !IS_SET(victim->cstat(form),FORM_UNDEAD) 
       || !IS_SET( victim->act, ACT_UNDEAD )) {
    send_to_char("You cannot stake the living.\n\r",ch);
    return;
  }
   
  if (victim == ch)  {
    send_to_char("You aren't undead.. you cannot stake yourself.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if (is_safe(ch,victim))
    return;

  if ( IS_NPC(victim) 
       && victim->fighting != NULL 
       && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if ( IS_AFFECTED(ch,AFF_CHARM) 
       && ch->master == victim) {
    act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
    return;
  }

  /* modifiers */

  if (ch->cstat(size) < victim->cstat(size))
    chance += (ch->cstat(size) - victim->cstat(size)) * 15;
  else
    chance += (ch->cstat(size) - victim->cstat(size)) * 10; 


    /* stats */
  chance -= GET_AC(victim,AC_PIERCE) /25;
  /* speed */
  if ( IS_SET(ch->off_flags,OFF_FAST) 
       || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if ( IS_SET(victim->off_flags,OFF_FAST) 
       || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 10;

    /* level */
  chance += (ch->level - victim->level);

  /* now the attack */
  if (number_percent() < chance ) {
    
    act("$n has stuck a stake in your heart!",
	ch,NULL,victim,TO_VICT);
    act("You slam a stake into $N!",ch,NULL,victim,TO_CHAR);
    act("$n shoves a stake into $N .",
	ch,NULL,victim,TO_NOTVICT);
    check_improve(ch,gsn_stake,TRUE,1);

    int done = ability_damage( ch,victim,((ch->level*(dice(ch->level/4,6))) + ch->level),gsn_stake,
			DAM_PIERCE,TRUE, FALSE );
    if ( done == DAMAGE_DONE ) {
      DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
      WAIT_STATE(ch,BEATS(gsn_stake));
      victim->position = POS_RESTING;
    }
  }
  else {
    ability_damage(ch,victim,0,gsn_stake,DAM_PIERCE,TRUE,FALSE);
    act("You fall flat on your face!",
	ch,NULL,victim,TO_CHAR);
    act("$n falls flat on $s face.",
	ch,NULL,victim,TO_NOTVICT);
    act("You evade $n's stake, causing $m to fall flat on $s face.",
	ch,NULL,victim,TO_VICT);
    check_improve(ch,gsn_stake,FALSE,1);
    ch->position = POS_RESTING;
    WAIT_STATE(ch,BEATS(gsn_stake) * 3/2); 
  }
  check_killer(ch,victim);
}

void do_buddha_palm( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);
  
  if ( (chance = get_ability(ch,gsn_buddha_palm)) == 0 ) { 
    send_to_char("You'd better leave the unarmed combats to monks.\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("Buddha Palm who ?\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
   
  if (victim == ch)  {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( get_eq_char( ch, WEAR_WIELD ) != NULL ) {
    send_to_char("You need to be bare hands to use that skill.\n\r", ch );
    return;
  }

  if (is_safe(ch,victim))
    return;

  if (victim->position < POS_FIGHTING) {
    act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( IS_NPC(victim) 
       && victim->fighting != NULL 
       && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if ( !can_see( ch, victim ) ) {
    send_to_char("Where did you that person?\n\r",ch);
    return;
  }
  
  chance += 3*(ch->cstat(DEX) - victim->cstat(DEX));

  WAIT_STATE(ch,BEATS(gsn_buddha_palm));
  if ( number_percent() < chance ) {
    act("You hit $N with your palm.", ch, NULL, victim, TO_CHAR );
    act("$n hits you with $s palm.", ch, NULL, victim, TO_VICT );
    act("$n hits $N with $s palm.", ch, NULL, victim, TO_NOTVICT );
    //int dam = (ch->level * ch->cstat(STR) * (50+number_range(0,50)))/100;
    int dam = 2*(ch->level * ch->cstat(STR) * (50+number_range(0,50)))/(3*100);
    int done = ability_damage( ch, victim, dam, gsn_buddha_palm, DAM_BASH, TRUE, FALSE );
    check_improve(ch,gsn_buddha_palm,TRUE,1);
    if ( done == DAMAGE_DONE )
      DAZE_STATE(victim, 2 * PULSE_VIOLENCE);
  } 
  else {
    act("You try to hit $N with your palm but miss.", ch, NULL, victim, TO_CHAR );
    ability_damage( ch, victim, 0, gsn_buddha_palm, DAM_BASH, TRUE, FALSE );
    check_improve(ch,gsn_buddha_palm,FALSE,1);
  }
  check_killer(ch,victim);
}

// Added by SinaC 2000
void do_fist_of_fury( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  int chance;

  one_argument(argument,arg);
  
  if ( (chance = get_ability(ch,gsn_fist_of_fury)) == 0 ) { 
    send_to_char("You'd better leave the unarmed combats to monks.\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("Fist of fury who ?\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
   
  if (victim == ch)  {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( get_eq_char( ch, WEAR_WIELD ) != NULL ) {
    send_to_char("You need to be bare hands to use that skill.\n\r", ch );
    return;
  }

  if (is_safe(ch,victim))
    return;

  if (victim->position < POS_FIGHTING) {
    act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( IS_NPC(victim) 
       && victim->fighting != NULL 
       && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if ( !can_see( ch, victim ) ) {
    send_to_char("Where did you that person?\n\r",ch);
    return;
  }

  if ( ch->move < 100 && !IS_NPC(ch) ) {
    send_to_char("You don't have enough moves left to use fist of fury.\n\r", ch );
    return;
  }

  if ( !IS_NPC( ch ) ) 
    ch->move -= 100;
  
  WAIT_STATE( ch, BEATS(gsn_fist_of_fury) );
  
  if ( number_percent() < chance ) {
    check_improve(ch,gsn_fist_of_fury,TRUE,1);
    
    //int n = UMAX( ch->level / 20, 2 );
    int n = number_range(1,3)+2;
    int dam = ch->level/2;

    act("You hit $N with your punch.", ch, NULL, victim, TO_CHAR );
    act("$n hits you with $s punch.", ch, NULL, victim, TO_VICT );
    act("$n hits $N with $s punch.", ch, NULL, victim, TO_NOTVICT );
    
    for ( int i = 0; i < n; i++ ) {
      ability_damage( ch, victim, dam, gsn_fist_of_fury, DAM_BASH, TRUE, FALSE );
      dam *= 2;
    }
  }
  else {
    act("You try to hit $N with your punch but miss.", ch, NULL, victim, TO_CHAR );

    ability_damage( ch, victim, 0, gsn_fist_of_fury, DAM_BASH, TRUE, FALSE );
    check_improve(ch,gsn_fist_of_fury,FALSE,1);
  }
  check_killer( ch, victim );
}

void do_dual_disarm( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance,ch_weapon,vict_weapon,ch_vict_weapon;

  if ((chance = get_ability(ch,gsn_dual_disarm)) == 0) {
    send_to_char( "You don't know how to disarm opponents with your off-and.\n\r", ch );
    return;
  }
  
  OBJ_DATA *wield2 = get_eq_char( ch, WEAR_SECONDARY );
  if ( wield2 == NULL ) {
    send_to_char( "You must wield a weapon in your off-hand in order to dual disarm.\n\r", ch );
    return;
  }
  
  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  // Modified by SinaC 2003: secondary/thirdly and fourthly weapon can be disarmed
  if ( ( ( obj = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
       && ( ( obj = get_eq_char( victim, WEAR_SECONDARY ) ) == NULL )
       && ( ( obj = get_eq_char( victim, WEAR_THIRDLY ) ) == NULL )
       && ( ( obj = get_eq_char( victim, WEAR_FOURTHLY ) ) == NULL ) ) {
    send_to_char( "Your opponent is not wielding a weapon.\n\r", ch );
    return;
  }
  
  /* find weapon skills */
  //ch_weapon = get_weapon_ability(ch,get_weapon_sn(ch,TRUE) );
  //vict_weapon = get_weapon_ability(victim,get_weapon_sn(victim,FALSE) );
  //ch_vict_weapon = get_weapon_ability(ch,get_weapon_sn(victim,FALSE) );
  int ch_sn = get_weapon_sn(ch,wield2);
  int vict_sn = get_weapon_sn(victim,obj);
  ch_weapon = get_weapon_ability(ch, ch_sn);
  vict_weapon = get_weapon_ability(victim, vict_sn);
  ch_vict_weapon = get_weapon_ability(ch,vict_sn);

  /* modifiers */

  /* skill */
  chance = chance * ch_weapon/100;

  chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

  /* dex vs. strength */
  chance += ch->cstat(DEX);
  chance -= 2 * victim->cstat(STR);

  /* level */
  chance += (ch->level - victim->level) * 2;
 
  /* if victim has deathgrip */
  if ( is_affected( victim, gsn_deathgrip) )
  chance -= victim->level/10;

  // If has disarm, chance increased
  if ( get_eq_char( ch, WEAR_WIELD )
       && chance < get_ability( ch, gsn_disarm ) )
    chance += 10;

  /* and now the attack */
  // Modified by SinaC 2000 for grip skill
  if (number_percent() < chance) {
    if ((chance = get_ability(victim,gsn_grip)) == 0) {
      WAIT_STATE( ch, BEATS(gsn_dual_disarm));
      disarm( ch, victim, obj );
      check_improve(ch,gsn_dual_disarm,TRUE,1);
      return;
    }
    if (number_percent() > (chance/5)*4) {
      WAIT_STATE( ch, BEATS(gsn_dual_disarm) );
      disarm( ch, victim, obj );
      check_improve(ch,gsn_dual_disarm,TRUE,1);
      check_improve(victim,gsn_grip,FALSE,1);
      return;
    }
    check_improve(victim,gsn_grip,TRUE,1);
    /*
      WAIT_STATE( ch, skill_table[gsn_disarm].beats );
      disarm( ch, victim );
      check_improve(ch,gsn_disarm,TRUE,1);
    */
  }
  /*
    else
    {
  */
  WAIT_STATE(ch,BEATS(gsn_dual_disarm));
  act("You fail to disarm $N with your off-hand.",ch,NULL,victim,TO_CHAR);
  act("$n tries to disarm you, but fails.",ch,NULL,victim,TO_VICT);
  act("$n tries to disarm $N, but fails.",ch,NULL,victim,TO_NOTVICT);
  check_improve(ch,gsn_dual_disarm,FALSE,1);
  //    }
  //check_killer(ch,victim);
  return;
}

void do_gouge( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);

  if ( (chance = get_ability(ch,gsn_gouge)) == 0 ) {
    send_to_char("You don't know how to blind your opponent.\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't in combat!\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(victim,AFF_BLIND)) {
    act("$E's already been blinded.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (victim == ch) {
    send_to_char("Very funny.\n\r",ch);
    return;
  }
  
  if (is_safe(ch,victim))
    return;

  if (IS_NPC(victim)
      && victim->fighting != NULL 
      && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
    act("But $N is such a good friend!",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  /* modifiers */

  /* dexterity */
  chance += ch->cstat(DEX);
  chance -= 2 * victim->cstat(DEX);

  /* speed  */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 25;

  /* level */
  chance += (ch->level - victim->level) * 2;

  /* now the attack */
  if (number_percent() < chance) {
    AFFECT_DATA af;

    // Color added by SinaC 2003
    act("$n is blinded by $N's gouging!",victim,NULL,ch,TO_ROOM);
    act("{W$n gouge your eyes!{x",ch,NULL,victim,TO_VICT);
    int done = ability_damage(ch,victim,number_range(10,5),gsn_gouge,DAM_NONE,TRUE, FALSE);
    if ( done == DAMAGE_DONE ) {
      send_to_char("You can't see a thing!\n\r",victim);
      check_improve(ch,gsn_gouge,TRUE,2);
      WAIT_STATE(ch,BEATS(gsn_gouge));
      
      //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,0,ch->level,gsn_gouge);
      //affect_to_char(victim,&af);
      //afsetup(af,CHAR,hitroll,ADD,-4,0,ch->level,gsn_gouge);
      //affect_to_char(victim,&af);
      createaff(af,0,ch->level,gsn_gouge,0,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,-4);
      addaff(af,CHAR,affected_by,OR,AFF_BLIND);
      affect_to_char( victim, &af );
    }
  }
  else {
    ability_damage(ch,victim,0,gsn_gouge,DAM_NONE,TRUE, FALSE);
    check_improve(ch,gsn_gouge,FALSE,2);
    WAIT_STATE(ch,BEATS(gsn_gouge));
  }
  check_killer(ch,victim);
}

void do_headbutt( CHAR_DATA *ch, const char *argument ){
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;

  one_argument(argument,arg);

  if ((chance = get_ability(ch,gsn_headbutt)) == 0) {
    send_to_char("You don't even know how to Headbutt!\n\r",ch);
    return;
  }

  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't fighting anyone!\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't there.\n\r", ch);
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }

    if (victim->position < POS_FIGHTING) {
      act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
      return;
    }
    
    if (victim == ch) {
      send_to_char("You can't do that.\n\r",ch);
      return;
    }
    
    if (is_safe(ch,victim))
      return;

    if ( IS_NPC(victim) 
	 && victim->fighting != NULL 
	 && !is_same_group(ch,victim->fighting)) {
      send_to_char("Kill stealing is not permitted.\n\r",ch);
      return;
    }
    
    if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
      act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
      return;
    }
    
    // Added by SinaC 2000
    if ( !can_see( ch, victim ) ) {
      send_to_char("You headbutt somewhere ... but don't hit anything.\n\r",ch);
      return;
    }
    
    /* modifiers */
    
    /* size  and weight */
    chance += ch->carry_weight / 250;
    chance -= victim->carry_weight / 200;
    
    if (ch->cstat(size) < victim->cstat(size))
      chance += (ch->cstat(size) - victim->cstat(size)) * 15;
    else
      chance += (ch->cstat(size) - victim->cstat(size)) * 10; 
    
    
    /* stats */
    chance += ch->cstat(STR);
    chance -= (victim->cstat(DEX) * 4)/3;
    chance -= GET_AC(victim,AC_BASH) /25;
    /* speed */
    if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
      chance += 10;
    if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
      chance -= 30;
    
    /* level */
    chance += (ch->level - victim->level);
    
    /* now the attack */
    if (number_percent() < chance ) {
      
      act("$n sends you sprawling with a powerful headbutt!",
	  ch,NULL,victim,TO_VICT);
      act("You slam into $N, and knock $M flying!",ch,NULL,victim,TO_CHAR);
      act("$n sends $N sprawling with a powerful headbutt.",
	  ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_headbutt,TRUE,1);
  
      int dam = number_range(20,2 + 2 * ch->cstat(size) + chance/20);
      // Modified by SinaC 2001
      int done = ability_damage(ch,victim,dam,gsn_headbutt, DAM_BASH, TRUE, FALSE);
      WAIT_STATE(ch,BEATS(gsn_headbutt));
      if ( done == DAMAGE_DONE ) {
	DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
	victim->position = POS_RESTING;
	
	// Added by SinaC 2000 and modified by SinaC 2001
	if ( victim->stunned > 0 )
	  return;
	chance = get_ability( ch, gsn_headbutt )/3;
	if (number_percent() < chance )
	  victim->stunned = 2;
	act("{gYou are stunned, and have trouble getting back up!{x", ch,NULL,victim,TO_VICT);
	act("{g$N is stunned by your headbutt!{x",ch,NULL,victim,TO_CHAR);
	act("{g$N is having trouble getting back up.{x", ch,NULL,victim,TO_NOTVICT);
      }
      else {
	ability_damage(ch,victim,0,gsn_headbutt,DAM_BASH,FALSE, FALSE);
	act("You fall flat on your face!",ch,NULL,victim,TO_CHAR);
	act("$n falls flat on $s face.", ch,NULL,victim,TO_NOTVICT);
	act("You evade $n's bash, causing $m to fall flat on $s face.", ch,NULL,victim,TO_VICT);
	check_improve(ch,gsn_headbutt,FALSE,1);

	if (ch->cstat(size) < victim->cstat(size)
	    && number_percent() < 40 ) { // if victim greater than ch, disarms ch
	  OBJ_DATA *obj = get_eq_char( ch, WEAR_WIELD );
	  if ( obj == NULL )
	    return;
	  obj_from_char( obj );
	  obj_to_room( obj, ch->in_room );
	  OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
	  //recompute( ch ); NO NEED: done by obj_from_char -> unequip_char (unequipped items doesn't give affect)
	  recomproom( ch->in_room );
	}
	ch->position = POS_RESTING;
	WAIT_STATE(ch,BEATS(gsn_headbutt) * 3/2); 
      }
      check_killer(ch,victim);
    }
}

void do_shield_bash( CHAR_DATA *ch, const char *argument ){
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  
  one_argument(argument,arg);
  
  if ((chance = get_ability(ch,gsn_shield_bash)) == 0) {
    send_to_char("You don't even know how to Shield bash!\n\r",ch);
  return;
}

  if (arg[0] == '\0') {
    victim = ch->fighting;
    if (victim == NULL) {
      send_to_char("But you aren't fighting anyone!\n\r",ch);
      return;
    }
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't there.\n\r", ch);
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  // Added by SinaC 2001
  if ( IS_AFFECTED( ch, AFF_ROOTED) ) {
    send_to_char("You can't move your leg because of the {groots{x around your legs.\n\r", ch );
    return;
  }
  
  if (victim->position < POS_FIGHTING) {
    act("You'll have to let $M get back up first.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (victim == ch) {
    send_to_char("You can't do that.\n\r",ch);
    return;
  }
  
  if ( get_eq_char( ch, WEAR_SHIELD ) == NULL ) {
    send_to_char("You need to hold a shield in order to use shield bash.\n\r", ch );
    return;
  }

    
  if (is_safe(ch,victim))
    return;
      
  if ( IS_NPC(victim) 
       && victim->fighting != NULL 
       && !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
    act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  // Added by SinaC 2000
  if ( !can_see( ch, victim ) ) {
    send_to_char("You bash with your shield somewhere ... but don't hit anything.\n\r",ch);
    return;
  }
  
  /* modifiers */
  
  /* size  and weight */
  chance += ch->carry_weight / 250;
  chance -= victim->carry_weight / 200;
  
  if (ch->cstat(size) < victim->cstat(size))
    chance += (ch->cstat(size) - victim->cstat(size)) * 15;
  else
    chance += (ch->cstat(size) - victim->cstat(size)) * 10; 
    
    
  /* stats */
  chance += ch->cstat(STR);
  chance -= (victim->cstat(DEX) * 4)/3;
  chance -= GET_AC(victim,AC_BASH) /25;
  /* speed */
  if (IS_SET(ch->off_flags,OFF_FAST) || IS_AFFECTED(ch,AFF_HASTE))
    chance += 10;
  if (IS_SET(victim->off_flags,OFF_FAST) || IS_AFFECTED(victim,AFF_HASTE))
    chance -= 30;
    
  /* level */
  chance += (ch->level - victim->level);
    
  /* now the attack */
  if (number_percent() < chance ) {
      
    act("$n sends you sprawling with a powerful shield bash!",
	ch,NULL,victim,TO_VICT);
    act("You slam into $N with your shield, and knock $M flying!",ch,NULL,victim,TO_CHAR);
    act("$n sends $N sprawling with a powerful shield bash.",
	ch,NULL,victim,TO_NOTVICT);
    check_improve(ch,gsn_shield_bash,TRUE,1);
  
    int dam = number_range(10,2 + 2 * ch->cstat(size) + chance/20);
    // Modified by SinaC 2001
    int done = ability_damage(ch,victim,dam,gsn_shield_bash, DAM_BASH, TRUE, FALSE);
    if ( done == DAMAGE_DONE ) {
      DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
      WAIT_STATE(ch,BEATS(gsn_shield_bash));
      victim->position = POS_RESTING;
	
      // FIXME knock off items from victim's hand ?

      // Added by SinaC 2000 and modified by SinaC 2001
      if ( victim->stunned > 0 )
	return;
      chance = get_ability( ch, gsn_shield_bash )/3;
      if (number_percent() < chance )
	victim->stunned = 2;
      act("{gYou are stunned, and have trouble getting back up!{x", ch,NULL,victim,TO_VICT);
      act("{g$N is stunned by your shield bash!{x",ch,NULL,victim,TO_CHAR);
      act("{g$N is having trouble getting back up.{x", ch,NULL,victim,TO_NOTVICT);
    }
    else {
      ability_damage(ch,victim,0,gsn_shield_bash,DAM_BASH,FALSE, FALSE);
      act("You fall flat on your face!",ch,NULL,victim,TO_CHAR);
      act("$n falls flat on $s face.", ch,NULL,victim,TO_NOTVICT);
      act("You evade $n's bash, causing $m to fall flat on $s face.", ch,NULL,victim,TO_VICT);
      check_improve(ch,gsn_shield_bash,FALSE,1);

      ch->position = POS_RESTING;
      WAIT_STATE(ch,BEATS(gsn_shield_bash) * 3/2); 
    }
    check_killer(ch,victim);
  }
}

void do_soothe( CHAR_DATA *ch, const char *argument ){
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int chance;
  
  one_argument(argument,arg);
  
  if ((chance = get_ability(ch,gsn_soothe)) == 0) {
    send_to_char("You don't even know how to soothe!\n\r",ch);
    return;
  }

  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( !IS_SET( victim->cstat(form), FORM_ANIMAL) ) {
    act("You failed, $N is not an animal.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (IS_AFFECTED(victim,AFF_CALM) 
      || IS_AFFECTED(victim,AFF_BERSERK)
      || is_affected(victim,gsn_frenzy)) {
    send_to_char("You failed.\n\r", ch );
    return;
  }

  if ( number_percent() < chance ) {
    send_to_char("You failed.\n\r", ch );
    return;
  }

  int count = 0,
    mlevel = 0,
    high_level = 0;
  /* get sum of all animal mobile levels in the room */
  for (CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    if (vch->position == POS_FIGHTING) {
      count++;
      if (IS_NPC(vch)
	  && IS_SET(vch->cstat(form),FORM_ANIMAL)
	  && !IS_SET(vch->cstat(imm_flags),IRV_MAGIC) ) {
	mlevel += vch->level;
	high_level = UMAX(high_level,vch->level);
      }
    }
  }

  /* compute chance of stopping combat */
  chance = 4 * ch->level - high_level + 2 * count;

  if (IS_IMMORTAL(ch)) /* always works */
    mlevel = 0;

  if (number_range(0, chance) < mlevel)
    return;

  send_to_char("A wave of calm passes over you.\n\r",victim);
  
  if (victim->fighting || victim->position == POS_FIGHTING)
    stop_fighting(victim,FALSE);
  
  AFFECT_DATA af;
  //afsetup(af,CHAR,affected_by,OR,AFF_CALM,ch->level/4,ch->level,gsn_soothe);
  //affect_to_char(victim,&af);
  createaff(af,ch->level/4,ch->level,gsn_soothe,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CALM);
  affect_to_char( victim, &af );
}

void do_charge(CHAR_DATA *ch,const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int chance, chance2;
  int dam;

  // Know how to charge and mounted combat
  if ( (chance = get_ability(ch,gsn_charge)) == 0 ) {
    send_to_char("You're not trained in the art of charging.\n\r",ch);
    return;
  }
  // Following test is not required, because charge has mounted_combat as prereq 
  if ( ( chance2 = get_ability(ch,gsn_mounted_combat)) == 0 ) {
    send_to_char("You must be trained in the art of mounted combat in order to charge.\n\r", ch );
    return;
  }

  // Stunned?
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  // Find victim
  one_argument(argument,arg);
  if (arg[0] == '\0') {
    send_to_char("Charge whom?\n\r", ch );
    return;
  }
  else if ((victim = get_char_room(ch,arg)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }
  if (victim == ch) {
    send_to_char("That would be a bit stupid.\n\r", ch);
    return;
  }

  // Find mount
  CHAR_DATA *mount = get_mount( ch );
  if ( mount ) {
    send_to_char("You must ride a mount in order to charge.\n\r", ch );
    return;
  }
  if ( mount->in_room != ch->in_room ) {
    send_to_char("Your mount must be in the same room as you.\n\r", ch );
    return;
  }

  // Find weapon
  obj = get_eq_char(ch,WEAR_WIELD);
  if ( obj == NULL
       || obj->item_type != ITEM_WEAPON // SinaC 2003
       ) {
    send_to_char("You can't charge without wielding a weapon.\n\r",ch);
    return;
  }
  if ( obj->value[0] == WEAPON_DAGGER
       || obj->value[0] == WEAPON_EXOTIC
       || obj->value[0] == WEAPON_RANGED ) {
    send_to_char("You can't charge with a dagger, an exotic or a ranged weapon.\n\r",ch);
    return;
  }

  chance += (ch->level - victim->level);
  chance = URANGE(5, chance, 90);

  WAIT_STATE(ch,BEATS(gsn_charge));

  // Success
  if (number_percent() < chance) {
    act("You hold $p firmly, and charge $N.", ch,obj,victim,TO_CHAR );
    act("$n holds $p firmly, and charges you!",ch,obj,victim,TO_VICT);
    act("$n holds $p firmly, and charges $N!",ch,obj,victim,TO_NOTVICT);

    check_improve(ch,gsn_charge,TRUE,1);

    //dam = dice(obj->value[1],obj->value[2]);
    dam = dice(GET_WEAPON_DNUMBER(obj),GET_WEAPON_DTYPE(obj));
    dam += (get_ability(ch,gsn_enhanced_damage) * dam/100);
    if ( obj->value[0] == WEAPON_SPEAR
	 || obj->value[0] == WEAPON_STAFF )
      dam = (dam*7)/5;

    if (ch->level <= 40)      dam *= number_range(10,13);
    else if (ch->level <= 50) dam *= number_range(11,14);
    else if (ch->level <= 60) dam *= number_range(12,15);
    else if (ch->level <= 70) dam *= number_range(12,17);
    else if (ch->level <= 80) dam *= number_range(13,18);
    else                      dam *= number_range(14,20);

    dam /= 10;
    
    ability_damage(ch,victim,dam,gsn_charge,DAM_PIERCE,TRUE,FALSE);
  }
  else { // Missed
    check_improve(ch,gsn_charge,FALSE,1);
    ability_damage(ch,victim,0,gsn_charge,DAM_NONE,TRUE,FALSE);
  }

  return;
}

// Monks combat skills
void do_stun( CHAR_DATA *ch, char *argument ) {
  if ( !IS_NPC(ch)
       //&& !is_unarmed_and_unarmored(ch) ) {
       && !IS_UNARMED_UNARMORED(ch) ) { // SinaC 2003
    send_to_char("You must be unarmed and unarmored to use this skill.\n\r", ch );
    return;
  }
  CHAR_DATA *victim = ch->fighting;
  if ( get_ability(ch,gsn_stun) == 0 ) {
    send_to_char("You don't know how to stun.\n\r", ch );
    return;
  }
  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  WAIT_STATE( ch, BEATS(gsn_stun) );
  if ( get_ability(ch,gsn_stun) > number_percent()) {
    ability_damage( ch, victim, number_range( 1, 2*ch->level ), gsn_stun, DAM_BASH, TRUE, FALSE);
    check_improve( ch, gsn_stun, TRUE, 1 );
    if ( victim->stunned > 0 )
      return;
    int chance = (get_ability(ch,gsn_stun)/5);
    if (number_percent() < chance ) {
      chance = (get_ability(ch,gsn_stun)/5);
      if (number_percent() < chance )
	victim->stunned = 2;
      else
	victim->stunned = 1;
      act("{gYou are stunned, and have trouble getting back up!{x", ch,NULL,victim,TO_VICT);
      act("{g$N is stunned by your bash!{x",ch,NULL,victim,TO_CHAR);
      act("{g$N is having trouble getting back up.{x", ch,NULL,victim,TO_NOTVICT);
      check_improve(ch,gsn_stun,TRUE,1);
    }
  }
  else {
    ability_damage( ch, victim, 0, gsn_stun, DAM_BASH, TRUE, FALSE );
    check_improve( ch, gsn_stun, FALSE, 1 );
  }
  check_killer( ch, victim );
}

