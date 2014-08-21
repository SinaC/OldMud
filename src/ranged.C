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
#include "olc_value.h"
#include "ranged.h"
#include "magic.h"
#include "recycle.h"
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "interp.h"
#include "fight.h"
#include "act_comm.h"
#include "gsn.h"
#include "ability.h"
#include "condition.h"
#include "const.h"
#include "names.h"
#include "utils.h"
#include "damage.h"


// *******************************************************************************************************
// ****************************************** Ranged attack code *****************************************
// *******************************************************************************************************

//#define VERBOSE


// RANGED DAMAGE
int ranged_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, const int dt, const int dam_type ) {
  if ( ch == NULL || !ch->valid || victim == NULL || !victim->valid ) {
    bug("damage with ch = NULL (vict:%s) or vict = NULL", victim?victim->name:"[none]" );
    return DAMAGE_NOTDONE;
  }

  if ( victim->position == POS_DEAD )
    return DAMAGE_NOTDONE;

  // Stop up any residual loopholes.
  if ( dam > max_damage && dt >= TYPE_HIT) {
    bug( "Damage: %s did %d with an arrow", 
	 ch->name, dam );
    dam = max_damage;
  }

  // damage reduction
  if ( IS_NPC(ch) ) { // damage reduction only for mob
    if ( dam > 35)
      dam = (dam - 35)/2 + 35;
    if ( dam > 80)
      dam = (dam - 80)/2 + 80; 
  }

  if ( victim != ch ) {
    if ( is_safe( ch, victim ) )
      return DAMAGE_NOTDONE;
    check_killer( ch, victim );
    if ( victim->position > POS_PARALYZED ) {
      if ( ch->in_room == victim->in_room // SinaC 2003
	   && victim->fighting == NULL )
	set_fighting( victim, ch );
      if ( ch->in_room == victim->in_room // SinaC 2003
	   && victim->timer <= 4 )
	victim->position = POS_FIGHTING;
    }

    if ( victim->position > POS_PARALYZED )
      if (  ch->in_room == victim->in_room // SinaC 2003
	    && ch->fighting == NULL )
	set_fighting( ch, victim );

    if ( victim->master == ch )
      stop_follower( victim );
  }

  // Inviso attacks ... not.
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) ) {
    affect_strip( ch, gsn_invis );
    affect_strip( ch, gsn_mass_invis );
    REMOVE_BIT( ch->bstat(affected_by), AFF_INVISIBLE );
    recompute(ch);
    if ( !IS_AFFECTED(ch,AFF_INVISIBLE))
      act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
  }

  // Damage modifiers.
  if ( dam > 1 && !IS_NPC(victim)
       && victim->pcdata->condition[COND_DRUNK]  > 10 )
    dam = 9 * dam / 10;
  
  if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
    dam /= 2;

  // No protect good/evil with ranged weapon
  //if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
  //		   || (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
  //  dam -= dam / 4;
  
  // Added by SinaC 2003 for symbiote
  if ( dam > 1 && is_affected( victim, gsn_symbiote ) )
    dam -= (2*dam)/5;

  bool immune = FALSE;

  // Added by SinaC 2001
  if ( check_armor_absorb( victim, ch ) )
    return DAMAGE_NOTDONE;

  switch(check_immune(victim,dam_type)) {
  case(IS_IMMUNE):
    immune = TRUE;
    dam = 0;
    break;
  case(IS_RESISTANT):
    dam -= dam/3;
    break;
  case(IS_VULNERABLE):
    // Modified by SinaC 2001
    //dam += dam/2;
    dam += dam;
    break;
  }

  // Added by SinaC 2001  for mistform disadvantage, can't do damage
  if ( is_affected( ch, gsn_mistform ) )
    return DAMAGE_NOTDONE;

  // Added by SinaC 2001
  //if ( check_shroud( victim, ch, dam ) )  shroud should absorb damage but can't target ch
  //  return DAMAGE_NOTDONE;

  // send message to other people in the room, only if point blank shot
  if ( ch->in_room == victim->in_room )
    dam_message( ch, victim, dam, dt, immune, TRUE );
  else
    dam_message( ch, victim, dam, dt, immune, FALSE );
  
  if (dam == 0)    
    return DAMAGE_NOTDONE;

  // Hurt the victim.
  // Inform the victim of his new state.
  victim->hit -= dam;

  // Added by SinaC 2003 for phylactery
  // When affected by phylactery, victim has a chance to cheat death and get some hp back
  if ( victim->hit < 1 ) {
    bool check = check_phylactery( victim );
    if ( !check )
      if ( check_strategic_retreat( victim ) )
	return DAMAGE_NOTDONE; // damage are done but victim is transfered so we considered dam as not done
  }

  if ( !IS_NPC(victim)
       && IS_IMMORTAL(victim)
       && victim->hit < 1 )
    victim->hit = 1;
  update_pos( victim );

  position_msg( victim, dam );

  // Sleep spells and extremely wounded folks.
  if ( !IS_AWAKE(victim) )
    stop_fighting( victim, FALSE );

  if ( victim->position == POS_DEAD) {
    killing_payoff( ch, victim );
    return DAMAGE_DEADLY;
  }

  if ( victim == ch )
    return DAMAGE_DONE;

  // Take care of link dead people.
  if ( link_dead( victim ) )
    return DAMAGE_DONE;

  // Wimp out?
  wimp_out( ch, victim, dam );

  return DAMAGE_DONE;
}

// LEGAL RANGED TARGET
bool is_legal_ranged_target( CHAR_DATA *ch, CHAR_DATA *victim ) {
  if ( is_safe(ch, victim ) )
    return FALSE;
  if ( IS_SET(victim->in_room->cstat(flags), ROOM_SAFE) 
       || IS_SET(victim->in_room->cstat(flags), ROOM_BANK)
       || IS_SET(ch->in_room->cstat(flags), ROOM_SAFE)
       || IS_SET(ch->in_room->cstat(flags), ROOM_BANK) ) {
    send_to_char("Not to that room.\n\r", ch );
    return FALSE;
  }
  if ( IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim ) {
    act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
    return FALSE;
  }
  if ( IS_NPC(victim)
       && victim->in_room != ch->in_room
       && ( IS_SET(victim->act, ACT_SENTINEL)
	    || IS_SET(ch->in_room->cstat(flags), ROOM_NO_MOB)
	    || ( IS_SET(victim->act, ACT_OUTDOORS)
		 && IS_SET(ch->in_room->cstat(flags),ROOM_INDOORS))
	    || ( IS_SET(victim->act, ACT_INDOORS)
		 && !IS_SET(ch->in_room->cstat(flags),ROOM_INDOORS)) ) ) {
    send_to_char("Not to that victim.\n\r", ch );
    return FALSE;
  }
  return TRUE;
}

// return first ITEM_WEAPON/WEAPON_ARROW from inventory
OBJ_DATA *get_arrow( CHAR_DATA *ch ) {
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
    if ( obj->wear_loc == WEAR_NONE // only in inventory
	 && obj->item_type == ITEM_WEAPON // weapon
	 && obj->value[0] == WEAPON_ARROW // arrow
	 && obj->condition > 0 // in good condition
	 && can_see_obj( ch, obj ) )
      return obj;
  return NULL;
}

// Ch shoots with Bow, if MAX_ARROWS = -1: no maximal arrows
//  return value: 0: ok
//               -1: problem: no valid target, exit not found, ...
//               -2: missed skill
//               -3: deadly damage
#define SHOOT_OK      (0)
#define SHOOT_INVALID (1)
#define SHOOT_MISSED  (2)
#define SHOOT_DEADLY  (3)
static int shoot( CHAR_DATA *ch, OBJ_DATA *bow, int percent,
		  const char *arg1, const char *arg2,
		  const int MAX_ARROWS,
		  CHAR_DATA *&victim ) {
  int chance_point_blank_shot = get_ability( ch, gsn_point_blank_shot );
  // Search victim
  victim = NULL;
  int dir = DIR_NORTH; // starting dir
  bool point_blank_shot = FALSE; // in same room ?
  int dist = 1; // how far

  ROOM_INDEX_DATA *rooms[MAX_RANGED_DISTANCE];

#ifdef VERBOSE
  log_stringf("archery: %d", percent );
#endif

  // No direction argument means no more than one room away
  if ( arg2[0] == '\0' ) { // no additional direction argument
    // first: search in room
    victim = get_char_room( ch, arg1 );
    if ( victim != NULL ) { // found in room: too 
      if ( chance_point_blank_shot > 0 ) { // victim in room only if skilled in point blank shot
	point_blank_shot = TRUE;
	percent = ( percent + chance_point_blank_shot ) / 2; // adapt percent with point blank %
#ifdef VERBOSE
	log_stringf("point blank: %d  -> percent: %d", chance_point_blank_shot, percent );
#endif
      }
      else {
	send_to_char("{WDon't you think that standing a bit further away would be wise?{x\n\r", ch);
	return SHOOT_INVALID;
      }
    }
    // second: search in rooms around starting from DIR_NORTH (0) ending with DIR_SOUTHWEST (9)
    //          skipping DIR_SPECIAL
    if ( !point_blank_shot ) {
      ROOM_INDEX_DATA *was_in_room = ch->in_room;
      for ( int door = 0 ; door < MAX_DIR; door++ ) {
	if ( door == DIR_SPECIAL ) // skip special dir
	  continue;
	EXIT_DATA *pExit;
	if ( (  pExit = was_in_room->exit[door] ) != NULL
	     && pExit->u1.to_room != NULL                    // skip non-existing exit
	     && pExit->u1.to_room != was_in_room             // avoid self-exit room
	     && pExit->u1.to_room->area == was_in_room->area // can't fire in another area
	     && !IS_SET(pExit->exit_info,EX_CLOSED) ) {      // skip closed door room
	  ch->in_room = pExit->u1.to_room;
	  victim = get_char_room ( ch, arg1 );   // search victim in room
	  dir = door;
	  if ( victim != NULL ) // victim found
	    break;
	}
      }
      ch->in_room = was_in_room;
    }
  }
  else { // an additional direction argument has been specified
    dir = find_exit( arg2 );
    if ( dir == -1 ) { // specified direction doesn't exist
      send_to_char("This exit doesn't exist.\n\r", ch );
      return SHOOT_INVALID;
    }
    ROOM_INDEX_DATA *was_in_room = ch->in_room;
    int max_dist = 1;
    int chance_long_range_shot =  get_ability( ch, gsn_long_range_shot );
    if ( chance_long_range_shot > 0 ) // if long range shot is known: max dist is bow max distance
      max_dist = UMIN( bow->cRangedDistance, MAX_RANGED_DISTANCE );
    bool problem = FALSE;
#ifdef VERBOSE
    log_stringf("max dist: %d", max_dist );
#endif
    while ( dist <= max_dist ) { // while max distance not reached
#ifdef VERBOSE
      log_stringf("  dist: %d", dist );
#endif
      EXIT_DATA *pExit = ch->in_room->exit[dir];
      if ( pExit == NULL ) { // exit doesn't exist
	if ( dist == 1 ) // show non-existing exit only if dist is 1
	  send_to_char("This exit doesn't exist.\n\r", ch );
	problem = TRUE;
	break;
      }
      if ( IS_SET( pExit->exit_info, EX_CLOSED ) ) {  // closed door
	if ( dist == 1 ) // show door exit only if dist is 1
	  send_to_char("{WYou can't fire through a door.{x",ch);
	problem = TRUE;
	break;
      }
#ifdef VERBOSE
      log_stringf("  exit found and not a door");
#endif
      ch->in_room = pExit->u1.to_room;
      victim = get_char_room ( ch, arg1 );   // search victim in room direction
      rooms[dist-1] = ch->in_room; // store room(s) where the arrow(s) will go(es)
      if ( victim != NULL ) // if victim found: stop loop
	break;
      dist++;
    }
    ch->in_room = was_in_room;
    if ( problem && dist == 1 )
      return SHOOT_INVALID;
    if ( dist > 1 && victim != NULL ) {
      percent = ( percent + chance_long_range_shot ) / ( dist + 1 ); // harder to shot far away
#ifdef VERBOSE
      log_stringf("long range [%d]: %d  -> percent: %d", dist, chance_long_range_shot, percent );
      for ( int i = 0; i < dist; i++ )
	log_stringf("room: %d", rooms[i]?rooms[i]->vnum:-1);
#endif
    }
  }

  // Victim not found
  if ( victim == NULL ) {
    send_to_char( "You can't find it.\n\r", ch );
    return SHOOT_INVALID;
  }

  // Victim has been found.
  // Safe check
  if ( !is_legal_ranged_target( ch, victim ) )
    return SHOOT_INVALID;

  int chance_critical = get_ability( ch, gsn_critical_shot ); // critical shot
  int chance_strength = get_ability( ch, gsn_strength_shot ); // strength shot

  int num_arrows = 1;
  if ( point_blank_shot
       || dist > 1 ) // if point blank shot OR long range shot -> only one arrow
    num_arrows = 1;
  else // if not point blank shot: number of arrows depends on casting level
    num_arrows = UMAX( 1, get_casting_level( ch, gsn_bowfire ) );

  if ( MAX_ARROWS != -1 ) // SinaC 2003
    num_arrows = UMAX( num_arrows, MAX_ARROWS );

#ifdef VERBOSE
  log_stringf("#arrows: %d  (casting: %d)", num_arrows, get_casting_level( ch, gsn_bowfire ) );
#endif

  // Fire an arrow or multiple arrows at the same time
  for ( int i = 0; i < num_arrows; i++ ) {
    // Find an arrow in inventory
    OBJ_DATA *arrow = get_arrow( ch );
    if ( arrow == NULL ) { // no arrow found
      if ( i == 0 ) { // first one: send a msg saying we need at least an arrow to shoot
	send_to_char("You need an arrow to fire.\n\r", ch );
	return SHOOT_INVALID;
      }
      break; // not the first arrow: we just stop to fire arrows
    }
    
    if ( i == 0 ) { // checks are only done for the first arrow
      WAIT_STATE( ch, BEATS( gsn_bowfire ) );
      // Ability check
      if ( number_percent() > percent ) {
	act("You failed to fire $p.", ch, arrow, NULL, TO_CHAR );
	check_improve( ch, gsn_bowfire, FALSE, 1 );
	return SHOOT_MISSED;
      }
      check_improve( ch, gsn_bowfire, TRUE, 1 );
    }

    // Fire an arrow
    int dam = dice(GET_WEAPON_DNUMBER(arrow),GET_WEAPON_DTYPE(arrow)); // arrow damage

#ifdef VERBOSE
    log_stringf("arrow dam: %d", dam );
#endif

    // Critical shot
    if ( chance_critical > 0 ) {
      if ( number_percent()*2 > chance_critical ) // hard to get a critical shot
	check_improve( ch, gsn_critical_shot, FALSE, 8 );
      else {
	check_improve( ch, gsn_critical_shot, TRUE, 8 );
	dam += (4*dam)/5;

#ifdef VERBOSE
	log_stringf("critical: %d  -> dam: %d", chance_critical, dam );
#endif
      }
    }

    // Point blank shot does more damage
    if ( point_blank_shot ) {
      if ( number_percent() <= chance_point_blank_shot ) {
	check_improve( ch, gsn_point_blank_shot, TRUE, 8 );
	dam += (dam*chance_point_blank_shot)/150;

#ifdef VERBOSE
	log_stringf("point blank: %d  -> dam: %d", chance_point_blank_shot, dam );
#endif
      }
    }

    // Strength shot
    if ( chance_strength > 0 ) {
      if ( number_percent()*3 > chance_strength ) // really hard to get a strong shot
	check_improve( ch, gsn_strength_shot, FALSE, 8 );
      else {
	check_improve( ch, gsn_strength_shot, TRUE, 8 );
	dam += str_app[ch->cstat(STR)].todam * bow->cRangedStrength;
#ifdef VERBOSE
	log_stringf("strength shot: %d  -> dam: %d", chance_strength, dam );
#endif
      }
    }

    // Arrow sharpened
    if ( IS_WEAPON_STAT( arrow, WEAPON_SHARP ) ) {
      if ( ( percent = number_percent()) <= 25 ) {
	  dam = 2 * dam + ( dam * 2 * percent / 100 );
#ifdef VERBOSE
	  log_stringf("sharp arrow  -> dam: %d", dam );
#endif
      }
    }

    // Msg
    if ( point_blank_shot ) {
      act("{WYou shoot with $p at $N.{x", ch, bow, victim, TO_CHAR );
      act("{W$n shoots with $p at you.{x", ch, bow, victim, TO_VICT );
      act("{W$n shoots with $p at $N.{x", ch, bow, victim, TO_NOTVICT );
    }
    else {
      if ( i == 0 ) {
	act("{WYou shoot with $p.{x", ch, bow, victim, TO_CHAR );
	act("{W$n shoots with $p.{x", ch, bow, victim, TO_NOTVICT );
      }

      char buf[MAX_STRING_LENGTH];
      act( "{W$p flies in from $T and hits $n!{x", victim, arrow,
	   dir_name[rev_dir[dir]], TO_ROOM );
      act( "{W$p flies in from $T and hits you!{x", victim, arrow,
	   dir_name[rev_dir[dir]], TO_CHAR );
      sprintf( buf, "{W$p flew %d room%s %s and hits $N!{x", dist,
	       dist>1?"s":"",dir_name[dir] );
      act( buf, ch, arrow, victim, TO_CHAR );
      // send a message to rooms, between ch and victim, crossed by arrows
      for ( int j = 0; j < dist-1; j++ ) {
	// send the message to everyone in the room
#ifdef VERBOSE
	log_stringf("  ->rooms[%d]: %d", j, rooms[j]?rooms[j]->vnum:-1);
#endif
	if ( rooms[j] != NULL )
	  for ( CHAR_DATA *rch = rooms[j]->people; rch != NULL; rch = rch->next_in_room ) {
	    sprintf( buf, "{W%s flies from %s to %s.{x\n\r", 
		     can_see_obj( rch, arrow ) ? arrow->short_descr : "something",
		     dir_name[rev_dir[dir]], dir_name[dir] );
	    send_to_char( buf, rch );
	  }
      }
    }

    // Arrow in victim's inventory: FIXME: lodge
    obj_from_char( arrow );
    obj_to_char( arrow, victim );

    // Does the damage
    int done = ranged_damage( ch, victim, dam, gsn_bowfire, DAM_PIERCE );
    if ( done == DAMAGE_DEADLY ) // if victim is dead: no need to continue
      return SHOOT_DEADLY;
    if ( done == DAMAGE_DONE ) { // dmg done: arrow lodges? arrow takes damage? funky arrow?
      // TO DO: lodge
      if ( chance(20) ) // 20% chance to damage the arrow
	damage_obj( NULL, arrow, 10, DAM_BASH );
#ifdef VERBOSE
      if ( arrow->value[4] != 0 && !IS_WEAPON_STAT( arrow, WEAPON_SHARP ) )
	log_stringf("Funky arrow: %s", weapon_bit_name( arrow->value[4] ) );
#endif
      additional_hit_affect( ch, victim, arrow );
    }
  }
  check_killer( ch, victim );
  
  // does bow take damage?
  if ( bow->cRangedStringConditionModifier > 0 
       && chance(bow->cRangedStringConditionModifier) )
    // more arrows -> more damage       high bow strength -> bigger damage
    bow->bRangedStringCondition -= num_arrows * 
      URANGE( MIN_STRING_DAMAGE, number_range(1,2) + bow->cRangedStrength, MAX_STRING_DAMAGE );
  bow->bRangedStringCondition = URANGE( 0, bow->bRangedStringCondition, MAX_STRING_CONDITION );
  recompobj(bow);
  if ( bow->cRangedStringCondition <= 0 ) // string broken
    act("{W$p's string is broken.{x", ch, bow, NULL, TO_CHAR );

  return SHOOT_OK;
}

// *****************
// ********** SKILLS

// Basic ranged skill using a ranged weapon and arrow(s)
// fire <victim> [direction]
void do_bowfire( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( ch->in_room == NULL ) {
    send_to_char("How did you do that?!?\n\r", ch );
    bug("do_bowfire: %s is not in a room.", NAME(ch) );
    return;
  }

  // No argument
  if ( arg1[0] == '\0' ) {
    send_to_char( "Shoot at whom?\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if ( ch->stunned ) {
    send_to_char( "You're still a little woozy.\n\r", ch );
    return;
  }

  // Know bow ability ?
  int percent = get_ability( ch, gsn_bowfire );
  if ( percent == 0 ) {
    send_to_char("You don't even know how to use a bow.\n\r", ch );
    return;
  }

  // Find bow
  OBJ_DATA *bow1, *bow2; // bows are ITEM_WEAPON class WEAPON_RANGED
  if ( ( ( bow1 = get_eq_char(ch,WEAR_WIELD) ) == NULL )
       || bow1->item_type != ITEM_WEAPON // SinaC 2003
       || bow1->value[0] != WEAPON_RANGED )
    bow1 = NULL;
  if ( ( ( bow2 = get_eq_char(ch,WEAR_THIRDLY) ) == NULL )  // check second pair of arms
       || bow2->item_type != ITEM_WEAPON // SinaC 2003
       || bow2->value[0] != WEAPON_RANGED )
    bow2 = NULL;
  if ( bow1 == NULL && bow2 == NULL ) {
    send_to_char("You aren't wielding any ranged weapon to fire with.\n\r",ch);
    return;
  }
  OBJ_DATA *bow = bow1;
  if ( bow == NULL )
    bow = bow2;
  if ( bow->cRangedStringCondition <= 0 ) {
    if ( bow == bow2 || bow2 == NULL ) { // no second bow
      act("{W$p doesn't have any string.{x", ch, bow, NULL, TO_CHAR );
      return;
    }
    bow = bow2; // try the second bow
    if ( bow->cRangedStringCondition <= 0 ) { // second bow's string is also broken
      act("{W$p doesn't have any string.{x", ch, bow, NULL, TO_CHAR );
      return;
    }
  }

  CHAR_DATA *victim;
  int result = shoot( ch, bow, percent, arg1, arg2, -1, victim );
  if ( ( result == SHOOT_OK || result == SHOOT_MISSED ) // successfull or skill missed
       && bow2 != NULL && bow != bow2 ) { // try the second bow if not already done
    int result2 = shoot( ch, bow2, percent, arg1, arg2, -1, victim );
    if ( result2 == SHOOT_DEADLY )
      result = result2;
  }

  // Successfull non-deadly shoot, Victim comes to ch ? FIXME TO DO
  if ( result == SHOOT_OK
       && victim != NULL && victim->valid // should always be true if result == 0
       && IS_NPC(victim)
       && number_percent() > 50 ) { // better test
  }

  return;
}


// TO DO
void do_disarming_shot( CHAR_DATA *ch, const char *argument ) {
//'DISARMING SHOT'
//Syntax: fire disarm <victim> [direction]
//DISARMING SHOT is a specialized shot used by an archer to attempt to disarm
//his or her target.  A disarming shot will not inflict any damage on the
//target, as it is intended solely to dislodge the target's weapon from his or
//her hand.  Firing a disarming shot is considered a hostile act and may
//result in the target coming after the shooter.  Only one disarming shot may
//be fired at a time, and it cannot be fired over long range or at point blank
//range.
//See also: help archery
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  // Room check
  if ( ch->in_room == NULL ) {
    send_to_char("How did you do that?!?\n\r", ch );
    bug("do_bowfire: %s is not in a room.", NAME(ch) );
    return;
  }

  // Ability first check
  int archery_percent = get_ability( ch, gsn_bowfire );
  int disarming_percent = get_ability( ch, gsn_disarming_shot );
  if ( archery_percent == 0 ) {
    send_to_charf(ch,"You don't know how to use a bow.\n\r");
    return;
  }
  if ( disarming_percent == 0 ) {
    send_to_charf(ch,"You don't know how to perform a disarming shot.\n\r");
    return;
  }

  // No argument
  if ( arg1[0] == '\0' ) {
    send_to_char( "Shoot at whom?\n\r", ch );
    return;
  }

  // Stunned?
  if ( ch->stunned ) {
    send_to_char( "You're still a little woozy.\n\r", ch );
    return;
  }

  // Find bow
  OBJ_DATA *bow1, *bow2; // bows are ITEM_WEAPON class WEAPON_RANGED
  if ( ( ( bow1 = get_eq_char(ch,WEAR_WIELD) ) == NULL )
       || bow1->item_type != ITEM_WEAPON // SinaC 2003
       || bow1->value[0] != WEAPON_RANGED )
    bow1 = NULL;
  if ( ( ( bow2 = get_eq_char(ch,WEAR_THIRDLY) ) == NULL )  // check second pair of arms
       || bow2->item_type != ITEM_WEAPON // SinaC 2003
       || bow2->value[0] != WEAPON_RANGED )
    bow2 = NULL;
  if ( bow1 == NULL && bow2 == NULL ) {
    send_to_char("You aren't wielding any ranged weapon to fire with.\n\r",ch);
    return;
  }
  OBJ_DATA *bow = bow1;
  if ( bow == NULL )
    bow = bow2;
  if ( bow->cRangedStringCondition <= 0 ) {
    if ( bow == bow2 || bow2 == NULL ) { // no second bow
      act("{W$p doesn't have any string.{x", ch, bow, NULL, TO_CHAR );
      return;
    }
    bow = bow2; // try the second bow
    if ( bow->cRangedStringCondition <= 0 ) { // second bow's string is also broken
      act("{W$p doesn't have any string.{x", ch, bow, NULL, TO_CHAR );
      return;
    }
  }

  // Search victim
  CHAR_DATA *victim = NULL;
  int dir = DIR_NORTH; // starting dir
  int dist = 1; // how far

  // No direction argument means no more than one room away
  if ( arg2[0] == '\0' ) { // no additional direction argument
    // first: search in room
    victim = get_char_room( ch, arg1 );
    if ( victim != NULL ) { // found in room
	send_to_char("{WDon't you think that standing a bit further away would be wise?{x\n\r", ch);
	return;
    }
    // second: search in rooms around starting from DIR_NORTH (0) ending with DIR_SOUTHWEST (9)
    //          skipping DIR_SPECIAL
    ROOM_INDEX_DATA *was_in_room = ch->in_room;
    for ( int door = 0 ; door < MAX_DIR; door++ ) {
      if ( door == DIR_SPECIAL ) // skip special dir
	continue;
      EXIT_DATA *pExit;
      if ( (  pExit = was_in_room->exit[door] ) != NULL
	   && pExit->u1.to_room != NULL                    // skip non-existing exit
	   && pExit->u1.to_room != was_in_room             // avoid self-exit room
	   && pExit->u1.to_room->area == was_in_room->area // can't fire in another area
	   && !IS_SET(pExit->exit_info,EX_CLOSED) ) {      // skip closed door room
	ch->in_room = pExit->u1.to_room;
	victim = get_char_room ( ch, arg1 );   // search victim in room
	dir = door;
	if ( victim != NULL ) // victim found
	  break;
      }
    }
    ch->in_room = was_in_room;
  }
  else { // an additional direction argument has been specified
    dir = find_exit( arg2 );
    if ( dir == -1 ) { // specified direction doesn't exist
      send_to_char("This exit doesn't exist.\n\r", ch );
      return;
    }
    ROOM_INDEX_DATA *was_in_room = ch->in_room;
    bool problem = FALSE;
    EXIT_DATA *pExit = ch->in_room->exit[dir];
    if ( pExit == NULL ) { // exit doesn't exist
      send_to_char("This exit doesn't exist.\n\r", ch );
      return;
    }
    if ( IS_SET( pExit->exit_info, EX_CLOSED ) ) {  // closed door
      send_to_char("{WYou can't fire through a door.{x",ch);
      return;
    }
    
    ch->in_room = pExit->u1.to_room;
    victim = get_char_room ( ch, arg1 );   // search victim in room direction
    ch->in_room = was_in_room;
  }

  // Victim not found
  if ( victim == NULL ) {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  // Victim has been found.
  // Safe check
  if ( !is_legal_ranged_target( ch, victim ) )
    return;

  // We have a valid target: victim

  // Find an arrow in inventory
  OBJ_DATA *arrow = get_arrow( ch );
  if ( arrow == NULL ) { // no arrow found
    send_to_char("You need an arrow to fire.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, BEATS( gsn_bowfire ) );
  WAIT_STATE( ch, BEATS( gsn_disarming_shot ) );
  // Ability check
  if ( number_percent() > archery_percent ) {
    act("You failed to fire $p.", ch, arrow, NULL, TO_CHAR );
    check_improve( ch, gsn_bowfire, FALSE, 1 );
    return;
  }

  // Shoot
  act("{WYou shoot with $p.{x", ch, bow, victim, TO_CHAR );
  act("{W$n shoots with $p.{x", ch, bow, victim, TO_NOTVICT );

  obj_from_char( arrow );
  obj_to_room( arrow, victim->in_room );
  int chance = disarming_percent;

  bool failed = FALSE;
  // Find victim's weapon
  OBJ_DATA *victim_wield;
  if ( ( ( victim_wield = get_eq_char( victim, WEAR_WIELD ) ) == NULL )
       && ( ( victim_wield = get_eq_char( victim, WEAR_SECONDARY ) ) == NULL )
       && ( ( victim_wield = get_eq_char( victim, WEAR_THIRDLY ) ) == NULL )
       && ( ( victim_wield = get_eq_char( victim, WEAR_FOURTHLY ) ) == NULL ) )
    failed = TRUE;

  if ( !failed ) { // victim has a weapon
    // Find victim weapon skills
    int victim_sn = get_weapon_sn(victim,victim_wield); // first wield but obj already got in previous test
    int vict_weapon = get_weapon_ability(victim,victim_sn);
    int ch_vict_weapon = get_weapon_ability(ch,victim_sn);
    
    chance = chance * archery_percent/100;
    chance += (ch_vict_weapon/2 - vict_weapon) / 2; 

    // dex vs. strength
    chance += ch->cstat(DEX);
    chance -= 2 * victim->cstat(STR);
    
    // level
    chance += (ch->level - victim->level) * 2;
    
    // if victim has deathgrip
    if ( is_affected( victim, gsn_deathgrip) )
      chance -= victim->level/10;
    
    if ( number_percent() > chance ) { // basic failed
      check_improve( ch, gsn_disarming_shot, FALSE, 1 );
      failed = TRUE;
    }
    else { // success but is affected by grip ?
      int grip_perc = get_ability( victim, gsn_grip );
      if ( grip_perc >= 0 )
	if ( number_percent() <= (4*grip_perc)/5 ) { // grip ability -> failed
	  check_improve( victim, gsn_grip, TRUE, 1 );
	  failed = TRUE;
	}
	else {
	  check_improve( victim, gsn_grip, FALSE, 1 );
	  failed = FALSE;
	}
      else
	failed = FALSE;
      check_improve( ch, gsn_disarming_shot, TRUE, 1 );
    }
  }

  char buf[MAX_STRING_LENGTH];
  // Really failed
  if ( failed ) {
    act( "{W$p flies in from $T and misses $n!{x", victim, arrow, dir_name[rev_dir[dir]], TO_ROOM );
    act( "{W$p flies in from $T and misses you!{x", victim, arrow, dir_name[rev_dir[dir]], TO_CHAR );
    sprintf( buf, "{W$p flew %d room%s %s and misses $N!{x", dist, dist>1?"s":"",dir_name[dir] );
    act( buf, ch, arrow, victim, TO_CHAR );
  }
  // Really succeed
  else {
    // Will weapon be removed/dropped/...

    // No remove
    if ( IS_OBJ_STAT(victim_wield,ITEM_NOREMOVE)) {
      sprintf( buf, "{W$p flies in from %s and hits $P!{x", dir_name[rev_dir[dir]] );
      act( buf, victim, arrow, victim_wield, TO_ROOM );
      sprintf( buf, "{W$p flies in from %s and hits your weapon which won't budge!{x", dir_name[rev_dir[dir]] );
      act( buf, victim, arrow, NULL, TO_CHAR );
      sprintf( buf, "{W$p flew %d room%s %s and hits $P!{x", dist, dist>1?"s":"", dir_name[dir] );
      act( buf, ch, arrow, victim_wield, TO_CHAR );
      failed = TRUE;
    }

    // Onremoving trigger
    if ( !failed ) {
      Value args[] = {ch};
      bool result = objprog( victim_wield, ch, "onRemoving", args);
      if ( result ) // if onRemoving returns 1,  ch doesn't disarm victim
	failed = TRUE;
    }

    // Finally, we manage to disarm
    if ( !failed ) {
      sprintf( buf, "{W$p flies in from %s and hits $P sending it flying!{x", dir_name[rev_dir[dir]] );
      act( buf, victim, arrow, victim_wield, TO_ROOM );
      sprintf( buf, "{W$p flies in from %s and hits your weapon sending it flying!{x", dir_name[rev_dir[dir]] );
      act( buf, victim, arrow, NULL, TO_CHAR );
      sprintf( buf, "{W$p flew %d room %s and hits $P sending it flying!{x", dist, dir_name[dir] );
      act( buf, ch, arrow, victim_wield, TO_CHAR );

    
      obj_from_char( victim_wield );
      
      // Added by SinaC 2003, if result == true, item doesn't want to be dropped
      Value args[] = {ch};
      bool result = objprog( victim_wield, ch, "onDropping", args );
      
      // Modified by SinaC 2003
      if ( IS_OBJ_STAT( victim_wield, ITEM_NODROP ) || IS_OBJ_STAT( victim_wield, ITEM_INVENTORY )
	   || result )
	obj_to_char( victim_wield, victim );
      else {
	obj_to_room( victim_wield, victim->in_room );
	OBJPROG( victim_wield, victim, "onDropped", victim); // Added by SinaC 2003
	recomproom( victim->in_room );
      }
      
      MOBPROG( victim, ch, "onDisarmed", ch, victim_wield );
    }
  }

  check_killer( ch, victim );

  // Successfull non-deadly shoot, Victim comes to ch ? FIXME TO DO
  if ( victim != NULL && victim->valid // should always be true if result == 0
       && IS_NPC(victim)
       && number_percent() > 50 ) { // better test
  }

  return;
}



























void do_throw( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  long int accur,power,learn,dt,dam;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);

  if (arg1[0] == '\0') {
    send_to_char("Throw what?\n\r",ch);
    return;
  }
  if (arg2[0] == '\0') {
    send_to_char("Throw to whom?\n\r",ch);
    return;
  }

  if ( (learn = get_ability(ch,gsn_throw))==0 ) {
    send_to_char("You're so poor at throwing that the result would be to hurt you.\n\r",ch);
    return;
  }

  if ((obj = get_obj_carry(ch,arg1,ch)) == NULL) {
    send_to_char("You don't carry that item.\n\r",ch);
    return;
  }
  if ((victim = get_char_room(ch,arg2)) == NULL) {
    send_to_char("They aren't here.\n\r",ch);
    return;
  }

  if (victim == ch) {
    send_to_char("This would be bad for your health.\n\r",ch);
    return;
  }

  if ( !can_drop_obj(ch,obj) ) {
    send_to_char("You cannot let go this object.\n\r",ch);
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
  
  if (IS_AFFECTED(ch,AFF_CHARM) && ch->master == victim) {
    act("But $N is your friend!",ch,NULL,victim,TO_CHAR);
    return;
  }

  /* Perform throwing */
  act("You throw $p at $N",ch,obj,victim,TO_CHAR);
  act("$n throws a $p at $N.",ch,obj,victim,TO_NOTVICT);
  act("$n throws a $p at you.",ch,obj,victim,TO_VICT);

  obj_from_char(obj);

  accur = ch->cstat(DEX)*number_percent()*learn/20000;
  power = ch->cstat(STR)*number_percent()*learn/20000;

  power += (ch->level - victim->level);
  accur += (ch->level - victim->level);

  accur += victim->cstat(size);

  if (IS_SET(victim->off_flags,OFF_FAST) && victim->position > POS_RESTING )
    accur -= 20;

  if (obj->item_type == ITEM_WEAPON && obj->value[0] == WEAPON_SPEAR) {
    accur += GET_WEAPON_DNUMBER(obj)+10;
    power += GET_WEAPON_DTYPE(obj)+5;
    power *= 3;
  }

  send_to_charf(ch," Power : %ld\r\n Accuracy : %ld\n\r",power,accur);

  if (power < 10)  {
    act("$p falls down at half the distance to $N.",ch,obj,victim,TO_ROOM);
    act("$p falls down at half the distance to $N.",ch,obj,victim,TO_CHAR);
    obj_to_room(obj,ch->in_room);
    OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
    check_improve(ch,gsn_throw,FALSE,3);
  } 
  else if (accur < 50) {
    act("You miss $N completely.",ch,obj,victim,TO_CHAR);
    act("$n misses $N completely.",ch,obj,victim,TO_ROOM);
    obj_to_room(obj,ch->in_room);
    OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
  } 
  else if (accur < 70) {
    act("You miss $N.",ch,obj,victim,TO_ROOM);
    act("$n misses $N.",ch,obj,victim,TO_CHAR);
    obj_to_room(obj,ch->in_room);
    OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
  } 
  else if ( victim->position >= POS_STANDING
	      && accur < 95
	      && victim->cstat(DEX)*get_obj_weight(obj)*get_ability(victim,gsn_dodge) > 150*power ) {
    act("$N dodges $n's throw.",ch,obj,victim,TO_NOTVICT);
    act("You dodge $n's throw.",ch,obj,victim,TO_VICT);
    obj_to_room(obj,ch->in_room);
    OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
  } 
  else if (IS_SET(victim->cstat(parts),PART_HANDS)     /* You reach target */
	     && power*power/100 < victim->cstat(STR)*victim->cstat(DEX)/10) {
    act("$N catches $p!",ch,obj,victim,TO_NOTVICT);
    act("You catch $p!",ch,obj,victim,TO_VICT);
    act("$N catches $p!",ch,obj,victim,TO_CHAR);
    obj_to_char(obj,victim);
  } 
  else {
    check_improve(ch,gsn_throw,TRUE,3);
    
    if (accur>80)
      obj_to_char(obj,victim);
    else {
      obj_to_room(obj,victim->in_room);
      OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
    }
    
    dam = power+accur-150;
    dt = DAM_BASH;
    
    if (obj->item_type == ITEM_WEAPON) {
      if (obj->value[0] == WEAPON_SPEAR) {
	dt = DAM_PIERCE;
	dam *= 3;
	act("$p impales $N!",ch,obj,victim,TO_NOTVICT);
	act("$p impales you!",ch,obj,victim,TO_VICT);
	act("$p impales $N!",ch,obj,victim,TO_CHAR);
      } 
      else if (number_percent()*learn/100 > 70) {
	/* Little chance that the weapon hit with the "egde" */
	dt = obj->value[5];
	dam *= 2;
	act("$p smashes $N!",ch,obj,victim,TO_NOTVICT);
	act("$p smashes you!",ch,obj,victim,TO_VICT);
	act("$p smashes $N!",ch,obj,victim,TO_CHAR);
      } 
      else {
	act("$p hurts $N!",ch,obj,victim,TO_NOTVICT);
	act("$p hurts you!",ch,obj,victim,TO_VICT);
	act("$p hurts $N!",ch,obj,victim,TO_CHAR);
      }
      
    } 
    else {                                 
      act("$p hits $N!",ch,obj,victim,TO_NOTVICT);
      act("$p hits you!",ch,obj,victim,TO_VICT);
      act("$p hits $N!",ch,obj,victim,TO_CHAR);
    }
    
    log_stringf("do_throw: Avg Damage : %ld ",dam);
    
    DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
    WAIT_STATE(ch,BEATS(gsn_throw));
    ability_damage(ch,victim,number_range(dam*4/5,dam*6/5),gsn_throw,
		   dt, TRUE, FALSE);
    
    check_killer(ch,victim);
    return;
  }
  
  check_improve(ch,gsn_throw,FALSE,3);
  ability_damage(ch,victim,0,gsn_throw,DAM_BASH,FALSE, FALSE);
  check_killer(ch,victim);

  WAIT_STATE(ch,BEATS(gsn_throw));
}



// Removed by SinaC 2003
// Added by SinaC 2000
/*
#10002
dart~
a strange dart~
You see a little dart here.~
wood~
throwing 0 AO
5 6 pierce 15 'fireball'
0 10 210 P


v0: is the number of dice dammage
v1: is the number of side of each dice
v2: the type of damage ( same as weapon's ones )
v3: usualy filled with a 0 but hightly powered dart can be filled with
    a spell effect if so this hold the level of the spell.
v4: hold the spell, if you wanna make a clumsy dart fill it with
    a benefit spell.. would you throw a spear that make a heal on
    the victim... dart and flask are't the same on our mud
    flask are more powerful, but dart is more a warrior skill to make
    distant attack however both use throw command... the code make
    the difference between classes using flask and throwing weapon.
void do_throwing( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj,*obj_next;
  ROOM_INDEX_DATA *was_in_room;
  EXIT_DATA *pexit;
  char buf[256];
  int damm,damm_type,door,outside;

  outside=0;  // used to make mob move

  one_argument( argument, arg );

  if ( arg[0] == '\0' && ch->fighting == NULL ){
    send_to_char( "Throw on whom or what?\n\r", ch );
    return;
  }
  if ( ( obj = get_eq_char( ch, WEAR_HOLD ) ) == NULL ){
    send_to_char( "You hold nothing in your hand.\n\r", ch );
    return;
  }
  if ( obj->item_type != ITEM_THROWING ){
    send_to_char( "You can throw only a throwing weapons.\n\r", ch );
    return;
  }
  if ( arg[0] == '\0' ){
    if ( ch->fighting != NULL )
      victim = ch->fighting;
    else{
      send_to_char( "Throw on whom or what?\n\r", ch );
      return;
    }
  }
  else{
    // try to use 1 range victim allowance
    // look if victim is in the room if not look the surroundings rooms
    if ( ( victim = get_char_room ( ch, arg ) ) == NULL){
      was_in_room=ch->in_room;
      
      for( door=0 ; door<=5 && victim==NULL ; door++ ){ 
	if ( (  pexit = was_in_room->exit[door] ) != NULL
	     &&   pexit->u1.to_room != NULL
	     &&   pexit->u1.to_room != was_in_room 
	     &&   !str_cmp( pexit->u1.to_room->area->name ,
			   was_in_room->area->name ) ){ 
	  ch->in_room = pexit->u1.to_room;
	  victim = get_char_room ( ch, arg ); 
	}
      }
      
      ch->in_room=was_in_room;
      if(victim==NULL || IS_SET(pexit->exit_info,EX_CLOSED)){
	send_to_char( "You can't find it.\n\r", ch );
	return;
      }
      
      outside=1; // target was outside of the room
      
      // forbid target that cannot move back to revenge on
      // the thrower
      
      if(IS_NPC(victim)){
	if ( IS_SET(victim->act, ACT_SENTINEL)
	     // Modified by SinaC 2001
	     || IS_SET(ch->in_room->cstat(flags), ROOM_NO_MOB)
	     || ( IS_SET(victim->act, ACT_OUTDOORS)
		  && IS_SET(ch->in_room->cstat(flags),ROOM_INDOORS))
	     || ( IS_SET(victim->act, ACT_INDOORS)
		  && !IS_SET(ch->in_room->cstat(flags),ROOM_INDOORS))){ 
	  act("$N avoid your deadly throw !!!",ch,obj,victim,TO_CHAR);
	  was_in_room=ch->in_room;
	  ch->in_room=victim->in_room;
	  act("$N avoid $p thrown by $n.",ch,obj,victim,TO_NOTVICT);
	  act("You avoid $p thrown by $n.",ch,obj,victim,TO_VICT);
	  ch->in_room=was_in_room;
	  extract_obj(obj);
	  return;
	}
      }
    }
  }
  
  // this is a strong attack i recommand this wait state
  
  WAIT_STATE( ch, 2 * PULSE_VIOLENCE ); 
  
  sprintf(buf,"You have no more of %s.\n\r",obj->short_descr);
  
  // ITEM THROWING
  
  if(obj->item_type== ITEM_THROWING ){
    // check if there is a victim
    if ( victim != NULL ){
      act( "$n throw $p on $N.", ch,   obj, victim, TO_NOTVICT );
      act( "You throw $p on $N.", ch,   obj, victim, TO_CHAR );
      act( "$n throw $p on you.",ch,   obj, victim, TO_VICT );
      if(outside){ 
	was_in_room=ch->in_room;
	ch->in_room=victim->in_room;
	act( "$n throw $p on $N.", ch,   obj, victim, TO_NOTVICT );
	ch->in_room=was_in_room;
      }
    }
    
    // throw the dices :)
    if (ch->level <   obj->level
	||  number_percent() >= 20 + get_ability(ch,gsn_throwing) * 4/5 - (10 * outside) ){       
      // it is a normal miss
      act( "You throw $p aimlessly on the ground and it broke up.",
	   ch,  obj,NULL,TO_CHAR);
      act( "$n throw $p aimlessly on the ground and it broke up.",
	   ch,  obj,NULL,TO_ROOM);
      
      check_improve(ch,gsn_throwing,FALSE,2);
    }
    else{      
      // it is a success hit the target
      
      damm      =  dice(obj->value[0],obj->value[1]);
      //damm_type =  TYPE_HIT + obj->value[2];
      //damage( ch, victim, damm, damm_type, attack_table[obj->value[2]].damage, TRUE, FALSE);
      
      damm_type =  TYPE_HIT;
      damm_type += attack_table[obj->value[2]].damage;
      damage( ch, victim, damm, gsn_throwing, damm_type, TRUE, FALSE); 
      
      if( obj->value[3] != 0 && 
	  obj->value[4] != 0 && 
	  victim->position > POS_DEAD )  
	obj_cast_spell( obj->value[4], obj->value[3], ch, victim, obj);
      
      check_improve(ch,gsn_throwing,TRUE,2);
    }
    
    // look in the inventory for an objet of the same key_word and that is a throwing
    
    for(obj_next=ch->carrying; obj_next != NULL 
	  && (    (obj_next->item_type != ITEM_THROWING  ) 
		  || ( obj == obj_next )
		  || (strcmp(obj->name, obj_next->name) ) ); 
	obj_next=obj_next->next_content );
    extract_obj(obj);
    
    // if found equip the ch with
    if (  obj_next == NULL )
      send_to_char( buf, ch );
    else
      wear_obj( ch, obj_next, TRUE ); 
  } 
  // Routine that make a ranged mob moving 50% 
  //   to attack the aggressor ..... i m looking for a way
  //   to make the mob wait a little before doing it
  
  if( outside 
      && (victim != NULL )
      && (victim->position > POS_STUNNED) ){  
    pexit = victim->in_room->exit[0];
    
    for( door=0 ; door<=5 ; door++ ){
      pexit = victim->in_room->exit[door];
      if(  pexit != NULL
	   && (pexit->u1.to_room == ch->in_room) )
	break;
    }
    
    if(door>=6)	{ 
      bug("no back way in throw....");
      return;
    }
    
    if(IS_NPC(victim) && (number_percent() > 50 ) ){
      if ( !IS_SET(ch->act, ACT_SENTINEL)
	   && ( pexit = victim->in_room->exit[door] ) != NULL
	   &&   pexit->u1.to_room != NULL
	   &&   !IS_SET(pexit->exit_info, EX_CLOSED)
	   // Modified by SinaC 2001
	   &&   !IS_SET(pexit->u1.to_room->cstat(flags), ROOM_NO_MOB)
	   && ( !IS_SET(victim->act, ACT_OUTDOORS)
		||   !IS_SET(pexit->u1.to_room->cstat(flags),ROOM_INDOORS))
	   && ( !IS_SET(victim->act, ACT_INDOORS)
		||   IS_SET(pexit->u1.to_room->cstat(flags),ROOM_INDOORS))
	   && ( IS_NPC(victim) )
	   && ( (victim->position != POS_FIGHTING) 
		|| ( victim->fighting == ch )  ) ) {
	if( (victim->position == POS_FIGHTING)
	    && ( victim->fighting == ch ) ){  
	  stop_fighting( victim, TRUE );
	}
	
	move_char( victim, door, FALSE, FALSE );
	(victim)->wait = UMAX((victim)->wait, (8));
	act("$N scream and attack $n !!!",ch,NULL,victim,TO_NOTVICT);
	act("$N scream and attack You !!!",ch,NULL,victim,TO_CHAR);
	multi_hit( victim, ch, TYPE_UNDEFINED );
      }
    }
    else { 
      switch(door) { 
      case 0 : sprintf(buf,"The throw came from NORTH !!!\n\r");
	break;
      case 1 : sprintf(buf,"The throw came from EAST !!!\n\r");
	break;
      case 2 : sprintf(buf,"The throw came from SOUTH !!!\n\r");
	break;
      case 3 : sprintf(buf,"The throw came from WEST !!!\n\r");
	break;
      case 4 : sprintf(buf,"The throw came from UP !!!\n\r");
	break;
      case 5 : sprintf(buf,"The throw came from DOWN !!!\n\r");
	break;
      default : sprintf(buf,"Throw ERROR award an IMM\n\r");
	break;
      }
      send_to_char(buf,victim);
    }
  }
  return;
}
*/
















//Bowfire Code v1.0 (c)1997-99 Feudal Realms 
//
//This is my bow and arrow code that I wrote based off of a thrown weapon code
//that I had from long ago (if you wrote it let me know so I can give you
//credit for that part, I do not have it anymore), it's a little more complex 
//than I had originally wanted, but well, it works.  There are a couple things 
//that are involved which if you don't want to use, remove them, that simple.  
//One of them are the use of the "lodged" wearbits.  The code is designed to 
//lodge an arrow in a victim, not just do damage to them once, and there are three
//places it can lodge, etc.  Included are all of the pieces of code for quivers,
//arrows, drawing arrows, dislodging, etc.  Use whatever of this code that you
//want, if you have a credits page, add me on there, and please drop me an email
//at mustang@roscoe.mudservices.com so I know its out there somewhere being used.
//
//Any bugs that people find, if you email me, I will fix, unless it's something
//from a modification that you made, and if that's the case, I will probably
//help you figure out what's up with it if I can.  My code is not stock, and I
//tried to add in everything that people might need to add in this feature.
//
//Thanks,
//Tch
//
//===============================================================================
//Features in v1.0
//
//- Bowfire from adjacent rooms at targets
//- Arrows lodge in various body parts (leg, arm, and chest)
//- Quiver and arrow new item types
//- Shoulder wearbit used for quivers (if you want it)
//- Dislodging arrows does damage
//- OLC support for arrows and quivers
//
//===============================================================================
//
//Add in a bow weapon type with the other ones on the list.(if you don't know 
//how to do this, grep/search for sword and add in bow in the respective places)
//		
//===============================================================================
//
//In act_info.c in "void do_look" add this
//
//case ITEM_QUIVER:
//		if ( obj->value[0] <= 0 )
//		{
//		send_to_char( "{WThe quiver is out of arrows.{x\n\r", ch );
//		break;
//		}
//		
//		if (obj->value[0] == 1 )
//		{
//		send_to_char( "{WThe quiver has 1 arrow remaining in it.{x\n\r", ch );
//		break;
//		}
//		
//		if (obj->value[0] > 1 )
//		{
//		sprintf( buf, "{WThe quiver has %d arrows in it.{x\n\r", obj->value[0]);
//		}
//		send_to_char( buf, ch);
//		break;
//
//And add these to the end of "char *	const	where_name	[] ="
//
//	"{W({Rlodged in a leg{W){x  ",
//	"{W({Rlodged in an arm{W){x ",
//	"{W({Rlodged in a rib{W){x  ",
//
//===============================================================================
//
//In act_obj.c add these (or whatever file you want, that's just where mine is)
//
//
///* Bowfire code -- used to draw an arrow from a quiver */
//void do_draw( CHAR_DATA *ch, char *argument )
//{  
//    OBJ_DATA *quiver;
//	OBJ_DATA *arrow;
//    int hand_count= 0;
//	
//    if ( ( quiver = get_eq_char( ch, WEAR_SHOULDER ) ) == NULL )
//    {
//	send_to_char( "{WYou aren't wearing a quiver where you can get to it.{x\n\r", ch );
//	return;
//    }
//
//    if ( quiver->item_type != ITEM_QUIVER )
//    {
//	send_to_char( "{WYou can only draw arrows from a quiver.{x\n\r", ch );
//	return;
//    }
//
//	if (get_eq_char(ch,WEAR_LIGHT) != NULL) hand_count++;
//   	if (get_eq_char(ch,WEAR_SHIELD)!= NULL) hand_count++; 
//   	if (get_eq_char(ch,WEAR_HOLD)  != NULL) hand_count++;  
//   	if (get_eq_char(ch,WEAR_WIELD) != NULL) hand_count++;
//	if ( hand_count > 1)
//	{
//		send_to_char (	"{WYou need a free hand to draw an arrow.{x\n\r", ch );
//		return;
//	}
//	
//	if ( get_eq_char(ch, WEAR_HOLD) != NULL)
//	{
//	       send_to_char ( "{WYour hand is not empty!{x\n\r", ch );
//	       return;
//	}
//
//    if ( quiver->value[0] > 0 )
//    {
//	WAIT_STATE( ch, PULSE_VIOLENCE );	
//	act( "{W$n draws an arrow from $p{W.{x", ch, quiver, NULL, TO_ROOM );
//	act( "{WYou draw an arrow from $p{W.{x", ch, quiver, NULL, TO_CHAR );
//	
//	arrow = create_object(get_obj_index(OBJ_VNUM_ARROW), 0);
//	arrow->value[1] = quiver->value[1];
//	arrow->value[2] = quiver->value[2];
//	arrow->level    = quiver->level;
//	obj_to_char(arrow,ch);
//	wear_obj( ch,arrow,TRUE );
//	quiver->value[0] -= quiver->value[1];
//	
//	
//    if ( quiver->value[0] <= 0 )
//    {
//	act( "{WYour $p {Wis now out of arrows, you need to find another one.{x", ch, quiver, NULL, TO_CHAR );
//	extract_obj(quiver);
//    }
//
//    return;
//	}
//}
//
//
///* Bowfire code -- Used to dislodge an arrow already lodged */
//void do_dislodge( CHAR_DATA *ch, char *argument )
//{
//	OBJ_DATA * arrow = NULL;
//	int dam = 0;
//	
//    if (argument[0] == '\0') /* empty */
//    {
//        send_to_char ("{WDislodge what?{x\n\r",ch);
//        return;
//    }	
//	
//	if ( get_eq_char(ch, WEAR_LODGE_RIB) != NULL)
//	{
//	arrow = get_eq_char( ch, WEAR_LODGE_RIB );
//	act( "{WWith a wrenching pull, you dislodge $p {Wfrom your chest.{x", ch, arrow, NULL, TO_CHAR );
//	unequip_char( ch, arrow );
//	arrow->extra_flags = arrow->extra_flags - 134217728;
//	dam      =  dice((3 * arrow->value[1]), (3 * arrow->value[2]));
//	damage( ch, ch, dam, gsn_bow, DAM_SLASH, TRUE );
//	return;
//	}	
//	
//	else
//	if (get_eq_char(ch,WEAR_LODGE_ARM) != NULL)
//	{
//	arrow = get_eq_char( ch, WEAR_LODGE_ARM );
//	act( "{WWith a tug you dislodge $p {Wfrom your arm.{x", ch, arrow, NULL, TO_CHAR );
//	unequip_char( ch, arrow );
//	arrow->extra_flags = arrow->extra_flags - 134217728;
//	dam      =  dice((3 * arrow->value[1]), (2 * arrow->value[2]));
//	damage( ch, ch, dam, gsn_bow, DAM_SLASH, TRUE );
//	return;
//	}	
//	
//	else
//	if (get_eq_char(ch,WEAR_LODGE_LEG) != NULL)  
//	{
//	arrow = get_eq_char( ch, WEAR_LODGE_LEG );
//	act( "{WWith a tug you dislodge $p {Wfrom your leg.{x", ch, arrow, NULL, TO_CHAR );
//	unequip_char( ch, arrow );
//	arrow->extra_flags = arrow->extra_flags - 134217728;
//	dam      =  dice((2 * arrow->value[1]), (2 * arrow->value[2]));
//	damage( ch, ch, dam, gsn_bow, DAM_SLASH, TRUE );
//	return;
//	}
//	else
//	{	
//	send_to_char("{WYou have nothing lodged in your body.{x\n\r", ch);
//	return;
//	}
//}	
//
//===============================================================================
//
//In act_wiz.c add this to do_ostat
//
//    case ITEM_QUIVER:
//		sprintf(buf,"{WIt holds {R%d %d{Wd{R%d {Warrows.{x\n\r",
//		obj->value[0], obj->value[1], obj->value[2]);
//		send_to_char(buf,ch);
//		break;
//	case ITEM_ARROW:
//		sprintf(buf,"{WThis arrow will do {R%d{Wd{R%d {Wdamage for an average of {R%d{W.{x\n\r",
//		obj->value[1], obj->value[2], ( obj->value[1] + obj->value[2] ) / 2 );
//		send_to_char(buf,ch);
//		break;
//
//
//===============================================================================
//
//In const.c add this (modify it to your # of classes of course)
//
//	{
//    "bow",               
//	{ IM, IM, IM, IM, IM, 25, IM, 10, IM },     
//	{ 0,  0,  0,  0, 0,  5, 0, 4, 0},
//    spell_null,             TAR_IGNORE,             POS_FIGHTING,
//    &gsn_bow,     			SLOT( 0),       0,      0,
//    "arrow",                     "!Bow!",		""
//    },
//	
//and this to the "const struct item_type		item_table	[]	="
//
//	{	ITEM_QUIVER,	"quiver"	},
//	{	ITEM_ARROW,		"arrow",	},
//	
//===============================================================================
//
//In db.c add this to "void load_old_obj" add in what's between the arrows
//
//->	case ITEM_STAFF:
//	case ITEM_QUIVER:
//	case ITEM_ARROW:
//->	case ITEM_WAND:
//	    pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
//	    break;
//
//And in "void reset_room" add in what's between the arrows
//
//->             case ITEM_WAND:         olevel = number_range( 10, 20 ); break;
//			   case ITEM_QUIVER:         olevel = number_range( 10, 20 ); break;
//			   case ITEM_ARROW:         olevel = number_range( 10, 20 ); break;
//->             case ITEM_STAFF:        olevel = number_range( 15, 25 ); break;
//
//And in "OBJ_DATA *create_object" add in what's between the arrows
//
//->	break;
//
//    case ITEM_QUIVER:
//	case ITEM_ARROW:
//->	case ITEM_WEAPON:
//
//===============================================================================
//
//In db2.c in "void load_objects" add this anywhere
//
//	case ITEM_QUIVER:
//	case ITEM_ARROW:
//			pObjIndex->value[0]		= fread_number(fp);
//		    pObjIndex->value[1]		= fread_number(fp);
//		    pObjIndex->value[2]		= fread_number(fp);
//		    pObjIndex->value[3]		= fread_number(fp);
//		    pObjIndex->value[4]		= fread_number(fp);
//		    break;
//
//And in "void convert_object" add in what's between the arrows
//
//->      case ITEM_BOAT:
//		case ITEM_QUIVER:
//		case ITEM_ARROW:
//->      case ITEM_CORPSE_NPC:
//
//===============================================================================
//
//In fight.c add this
//
///* Bowfire code -- actual firing function */
//void do_fire( CHAR_DATA *ch, char *argument )
//{
//    char arg[MAX_INPUT_LENGTH];
//    CHAR_DATA *victim = NULL;
//    OBJ_DATA *arrow;
//	OBJ_DATA *bow;
//    ROOM_INDEX_DATA *was_in_room;
//    EXIT_DATA *pexit;
//    int dam ,door ,chance;
//
//
//    bow = get_eq_char(ch, WEAR_WIELD);
//    if (bow == NULL)
//	{
//	send_to_char("{WWhat are you going to do, throw the arrow at them?{x\n\r", ch);
//	return;
//	}
//
//    if (bow->value[0] != WEAPON_BOW)
//	{
//	send_to_char("{WYou might want to use a bow to fire that arrow with{x\n\r", ch);
//	return;
//	}    
//
//    one_argument( argument, arg );
//    if ( arg[0] == '\0' && ch->fighting == NULL )
//    {
//        send_to_char( "{WFire an arrow at who?{x\n\r", ch );
//        return;
//    }
//
//	if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == ch)
//	{
//		send_to_char("{WHow exactly did you plan on firing an arrow at yourself?{x\n\r", ch );
//		return;
//	}
//
//    if ( ( arrow = get_eq_char( ch, WEAR_HOLD ) ) == NULL )
//    {
//        send_to_char( "{WYou hold nothing in your hand.{x\n\r", ch );
//        return;
//    }
//
//
//    if ( arrow->item_type != ITEM_ARROW )
//    {
//        send_to_char( "{WYou can only a fire arrows or quarrels.{x\n\r", ch );
//        return;
//    }
//	
//    if ( arg[0] == '\0' )
//    {
//        if ( ch->fighting != NULL )
//        {
//            victim = ch->fighting;
//        }
//        else
//        {
//            send_to_char( "{WFire at whom or what?{x\n\r", ch );
//            return;
//        }
//    }
//    else
//    {
//        
//	/* See if who you are trying to shoot at is nearby... */
//
//        if ( ( victim = get_char_room ( ch, arg ) ) == NULL)
//        {
//            was_in_room=ch->in_room;
//
//
//            for( door=0 ; door<=5 && victim==NULL ; door++ )
//             { 
//                if ( (  pexit = was_in_room->exit[door] ) != NULL
//                   &&   pexit->u1.to_room != NULL
//                   &&   pexit->u1.to_room != was_in_room 
//                   &&   !strcmp( pexit->u1.to_room->area->name ,
//                                was_in_room->area->name ) )
//                   { 
//                     ch->in_room = pexit->u1.to_room;
//                     victim = get_char_room ( ch, arg ); 
//                    }
//               
//              }
//
//
//            ch->in_room=was_in_room;
//            if(victim==NULL)
//              {
//               send_to_char( "{WYou can't find it.{x\n\r", ch );
//               return;
//              }
//            else
//              {  if(IS_SET(pexit->exit_info,EX_CLOSED))
//                    { send_to_char("{WYou can't fire through a door.{x",ch);
//                      return;
//                     } 
//             }
//        }
//    }
//
//
//
//
//if((ch->in_room) == (victim->in_room))
//{
//send_to_char("{WDon't you think that standing a bit further away would be wise?{x\n\r", ch);
//return;
//}
//
///* Lag the bowman... */
//WAIT_STATE( ch, 2 * PULSE_VIOLENCE ); 
//
///* Fire the damn thing finally! */
//
//
//if(arrow->item_type== ITEM_ARROW )
//   {
//        
//      /* Valid target? */
//        
//        if ( victim != NULL )
//        {
//            act( "{W$r {Wfires $p {Wat $R{W.{x", ch,  arrow, victim, TO_NOTVICT );
//            act( "{WYou fire $p {Wat $R{W.{x", ch,   arrow, victim, TO_CHAR );
//            act( "{W$r {Wfires $p {Wat you.{x",ch,   arrow, victim, TO_VICT );
//        }
//
//
//      /* Did it hit? */
//      
//        if (ch->level <   arrow->level
//        ||  number_percent() >= 20 + get_skill(ch,gsn_bow) * 4/5 )
//        {       
//             /* denied... */
//                     
//              act( "{WYou fire $p {Wmissing, and it lands harmlessly on the ground.{x",
//                 ch,  arrow,NULL,TO_CHAR);
//              act( "{W$r fires $p {Wmissing, and it lands harmlessly on the ground.{x",
//                 ch,  arrow,NULL,TO_ROOM);
//              obj_from_char(arrow);
//			  obj_to_room(arrow, victim->in_room);
//          check_improve(ch,gsn_bow,FALSE,2);
//        }
//        else
//        {      
//               /* Shawing battah!  Now, where did it thud into? */
//
//
//	chance=dice(1,10);
//    switch (chance)
//    {
//    case 1 :
//	case 2 :
//	case 3 :
//	case 4 :
//	case 5 :
//	case 6 :
//            obj_from_char(arrow);
//			obj_to_char(arrow, victim);
//			arrow->wear_flags = 8388609;
//			wear_obj(victim, arrow,TRUE);
//			arrow->wear_flags = 65537;
//			arrow->extra_flags = arrow->extra_flags + 134217728;
//            dam      =  dice(arrow->value[1],arrow->value[2]);
//			damage( ch, victim, dam, gsn_bow, DAM_PIERCE, TRUE );						
//			check_improve(ch,gsn_bow,TRUE,2);
//			break;
//	case 7 :
//	case 8 :
//	case 9 :
//            obj_from_char(arrow);
//			obj_to_char(arrow, victim);
//			arrow->wear_flags = 16777217;
//			wear_obj(victim, arrow,TRUE);
//			arrow->wear_flags = 65537;
//			arrow->extra_flags = arrow->extra_flags + 134217728;
//            dam      = 3*( dice(arrow->value[1],arrow->value[2]))/2;
//			damage( ch, victim, dam, gsn_bow, DAM_PIERCE, TRUE );						
//			check_improve(ch,gsn_bow,TRUE,2);
//			break;
//	case 10 :
//            obj_from_char(arrow);
//			obj_to_char(arrow, victim);
//			arrow->wear_flags = 33554433;
//			wear_obj(victim, arrow,TRUE);
//			arrow->wear_flags = 65537;
//			arrow->extra_flags = arrow->extra_flags + 134217728;
//            dam      = 2*( dice(arrow->value[1],arrow->value[2]));
//			damage( ch, victim, dam, gsn_bow, DAM_PIERCE, TRUE );						
//			check_improve(ch,gsn_bow,TRUE,2);
//			break;
//	}		
//   }
//
//  } 
//
//    return;
//}
//
//===============================================================================
//
//In handler.c add this to "char *extra_bit_name( int extra_flags )"
//
//	if ( extra_flags & ITEM_LODGED		 ) strcat( buf, " lodged"		);
//
//===============================================================================
//
//In merc.h add these with the other wearbit defines
//
//#define WEAR_LODGE_LEG				27
//#define WEAR_LODGE_ARM				28
//#define WEAR_LODGE_RIB				29
//
//*** If you change the numbers, change the stuff on dislodge and such, they match
//    up to the numbers in there to make it work right...
//	
//And add this to the extra flags for items
//
//#define ITEM_LODGED			(bb)
//
//And add this to the item declarations
//
//#define ITEM_QUIVER			 	35
//#define ITEM_ARROW				36
//
//And add this to the static eq vnums (you can change the 1208, that's just what
//I used, it can go in any area, it really doesn't matter, I put it in immortal)
//
//#define OBJ_VNUM_ARROW			   1208 
//
//===============================================================================	
//
//In olc_act.c (if you use OLC) add this to "void show_obj_values"
//
//	case ITEM_QUIVER:
//		sprintf( buf, "[v0] Number of Arrows:     %d\n\r", obj->value[0] );
//		send_to_char( buf, ch);
//		sprintf( buf, "[v1] Number of Dice:       %d\n\r", obj->value[1] );
//		send_to_char( buf, ch);
//		sprintf( buf, "[v2] Type of Dice:         %d\n\r", obj->value[2] );
//		send_to_char( buf, ch);		
//		sprintf( buf, "[v3] Spell:                %s\n\r",
//		obj->value[3] != -1 ? skill_table[obj->value[3]].name
//				                    : "none" );
//		send_to_char( buf, ch);
//		break;
//
//	case ITEM_ARROW:
//		sprintf( buf, "[v1] Number of Dice:       %d\n\r", obj->value[1] );
//		send_to_char( buf, ch);
//		sprintf( buf, "[v2] Type of Dice:         %d\n\r", obj->value[2] );
//		send_to_char( buf, ch);		
//		sprintf( buf, "[v3] Spell:                %s\n\r",
//		obj->value[3] != -1 ? skill_table[obj->value[3]].name
//				                    : "none" );
//		send_to_char( buf, ch);
//		break;
//		
//And in "bool set_obj_values" add this in
//
//	case ITEM_QUIVER:
//	    switch ( value_num )
//	    {
//	        default:
//		    do_help( ch, "ITEM_QUIVER" );
//	            return FALSE;
//	        case 0:
//	            send_to_char( "NUMBER OF ARROWS SET.\n\r\n\r", ch );
//	            pObj->value[0] = atoi( argument );
//	            break;
//	        case 1:
//	            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
//	            pObj->value[1] = atoi( argument );
//	            break;
//	        case 2:
//	            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
//	            pObj->value[2] = atoi( argument );
//	            break;
//	    }
//            break;
//
//case ITEM_ARROW:
//	    switch ( value_num )
//	    {
//	        default:
//		    do_help( ch, "ITEM_ARROW" );
//	            return FALSE;
//				
//	        case 1:
//	            send_to_char( "NUMBER OF DICE SET.\n\r\n\r", ch );
//	            pObj->value[1] = atoi( argument );
//	            break;
//	        case 2:
//	            send_to_char( "TYPE OF DICE SET.\n\r\n\r", ch );
//	            pObj->value[2] = atoi( argument );
//	            break;
//	    }
//            break;
//
//===============================================================================	
//	
//In olc_save.c (if you use OLC) add this to "void save_object"
//
//	  	case ITEM_QUIVER:
//			fprintf( fp, "%d ", pObjIndex->value[0] );
//			fprintf( fp, "%d ", pObjIndex->value[1] );
//			fprintf( fp, "%d 0 0\n", pObjIndex->value[2]);
//			break;
//
//	  	case ITEM_ARROW:
//			fprintf( fp, "0 %d ", pObjIndex->value[1] );
//			fprintf( fp, "%d 0 0\n", pObjIndex->value[2]);
//			break;
//
//===============================================================================
//
//In tables.c add these to the end of "const struct flag_type wear_loc_flags[] ="
//
//	{	"lodge_leg",	WEAR_LODGE_LEG,	TRUE	},
//	{	"lodge_arm",	WEAR_LODGE_ARM,	TRUE	},
//	{	"lodge_rib",	WEAR_LODGE_RIB,	TRUE	},
//	
//And add this to "const struct flag_type extra_flags[] ="
//	
//	{	"lodged",			ITEM_LODGED,		TRUE	},
//
//And add this to "const struct flag_type type_flags[] ="
//
//	{	"quiver",		ITEM_QUIVER,		TRUE	},
//	{	"arrow",		ITEM_ARROW,			TRUE	},
//	
//===============================================================================	
//
//In immortal.are add this to the object list (or any other area/vnum, just change
//the declaration for it in merc.h if you do)
//
//#1208
//an arrow~
//an arrow~
//A wooden arrow is here.~
//unknown~
//arrow 0 AQ
//0 0 0 0 0
//9 0 0 P
//
//
//==========================================================================























///********************************************************
//
//  i based my automated ranged fighting code on this
//  nice file, not as good as my engage which i will release
//  in FuBaR 1.3.4X  you can make throw leave corpses and give
//  exp if you use ranged_hit or ranged_damage functions here.
//
//       Kjodo
//	   Kjodo@hotmail.com
//
//*********************************************************/
//
//
//
//
//#include <stdio.h>
//#include <string.h>
//#include <time.h>
//#include "merc.h"
//
//
//
//void    death_xp_loss        args( ( CHAR_DATA *victim ) );
//void    group_gain           args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//bool    check_dodge          args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//void    check_killer         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//void    dam_message          args( ( CHAR_DATA *ch, CHAR_DATA *victim, int
//dam,
//                                    int dt ) );
//void    set_fighting         args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//void    item_damage             args( ( CHAR_DATA *ch, int dam ) );
//void    ranged_damage	args(( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int
//dt ));
//
//int per_type args((CHAR_DATA *ch, OBJ_DATA *Obj));
//
//void ranged_hit args((CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *arrow,
//int dam));
//
//void do_shoot( CHAR_DATA *ch, char *argument )
//{
//  char arg1[MAX_INPUT_LENGTH];
//  char arg2[MAX_INPUT_LENGTH];
//  char arg3[MAX_INPUT_LENGTH];
//  char buf[MAX_STRING_LENGTH];
//  CHAR_DATA *victim;
//  ROOM_INDEX_DATA *to_room;
//  ROOM_INDEX_DATA *in_room;
//  OBJ_DATA *Obj;
//  EXIT_DATA *pexit;
//  int dir = 0;
//  int dist = 0;
//  int MAX_DIST = 5;
//  extern char *dir_noun [];
//  OBJ_DATA *bow;
//
//  argument = one_argument( argument, arg1 );
//  argument = one_argument( argument, arg2 );
//  argument = one_argument( argument, arg3 );
//
//  if (!can_use_skpell( ch, gsn_archery ) )
//	{ send_to_char(C_DEFAULT, "Maybe you should learn to use a bow.\n\r", ch );
//  	return;
//	}
//  if ( arg1[0] == '\0' )
//  {
//    send_to_char( C_DEFAULT, "What do you intend on shooting?\n\r", ch);
//    return;
//  }
//
//  if ( ( Obj = get_obj_carry( ch, arg1 ) ) == NULL )
//  {
//    send_to_char( C_DEFAULT,
//		 "You are not carrying that item.\n\r", ch );
//    return;
//  }
//
//  if (!(IS_SET(Obj->extra_flags, ITEM_AMMUNITION)) )
//  {
//    send_to_char( C_DEFAULT,
//		 "You cannot shoot that item.\n\r", ch );
//    return;
//  }
//
//  if (IS_SET( Obj->extra_flags, ITEM_NODROP ) )
//  {
//    send_to_char( C_DEFAULT, "You can't let go of it!\n\r", ch );
//    return;
//  }
//
//  if ( ( bow = get_eq_char( ch, WEAR_HOLD ) ) == NULL
//	|| bow->item_type != ITEM_RANGED_WEAPON )
//    {
//	send_to_char(AT_BLUE,"You aren't holding a ranged weapon.\n\r", ch
//);
//	return;
//    }
//
//
//
//  in_room = ch->in_room;
//  to_room = ch->in_room;
//
//  if ( ( victim = ch->fighting ) == NULL )
//  {
//    if ( arg2[0] == '\0' )
//    {
//      send_to_char( C_DEFAULT, "Shoot at whom?\n\r", ch );
//      return;
//    }
//
//    if ( arg3[0] == '\0' )
//    {
//      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
//      {
//	send_to_char( C_DEFAULT, "They aren't here.\n\r", ch );
//	return;
//      }
//    }
//    else
//    {
//      if ( get_curr_dex( ch ) >= 20 )
//      {
//        MAX_DIST = 7;
//        if ( get_curr_dex( ch ) == 25 )
//          MAX_DIST = 8;
//      }
//
//      for ( dir = 0; dir < 6; dir++ )
//	if ( arg2[0] == dir_name[dir][0] && !str_prefix( arg2,
//							 dir_name[dir] ) )
//	  break;
//
//      if ( dir == 6 )
//      {
//	send_to_char( C_DEFAULT, "Shoot in which direction?\n\r", ch );
//	return;
//      }
//
//      if ( ( pexit = to_room->exit[dir] ) == NULL ||
//	   ( to_room = pexit->to_room ) == NULL )
//
//      {
//	send_to_char( C_DEFAULT, "You cannot shoot in that direction.\n\r",
//		     ch );
//	return;
//      }
//
//      if ( IS_SET( pexit->exit_info, EX_CLOSED ) )
//      {
//	send_to_char( C_DEFAULT, "You cannot shoot through a door.\n\r",
//ch );
//	return;
//      }
//
//      if ( IS_SET( pexit->exit_info, EX_ISWALL ) )
//      {
//        send_to_char( C_DEFAULT, "You cannot shoot through a wall.\n\r",
//ch );
//        return;
//      }
//
//    if ( number_percent() > ch->pcdata->learned[gsn_archery] +
//(get_curr_dex( ch ) / 2)) {
//        send_to_char(AT_RED,"You fumble your bow!\n\r", ch );
//        return;
//        }
//
//      for ( dist = 1; dist <= MAX_DIST; dist++ )
//      {
//	char_from_room( ch );
//	char_to_room( ch, to_room );
//	if ( ( victim = get_char_room( ch, arg3 ) ) != NULL )
//	  break;
//
//	if ( ( pexit = to_room->exit[dir] ) == NULL ||
//	     ( to_room = pexit->to_room ) == NULL ||
//	       IS_SET( pexit->exit_info, EX_CLOSED ) ||
//               IS_SET( pexit->exit_info, EX_ISWALL) )
//	{
//	  sprintf( buf, "A $p flys in from $T and hits the %s wall.",
//		   dir_name[dir] );
//	  act( AT_WHITE, buf, ch, Obj, dir_noun[rev_dir[dir]], TO_ROOM );
//	  sprintf( buf, "You shoot your $p %d room%s $T, where it hits a
//wall.",
//		   dist, dist > 1 ? "s" : "" );
//	  act( AT_WHITE, buf, ch, Obj, dir_name[dir], TO_CHAR );
//	  char_from_room( ch );
//	  char_to_room( ch, in_room );
//	  oprog_throw_trigger( Obj, ch );
//	  obj_from_char( Obj );
//	  obj_to_room( Obj, to_room );
//	  return;
//	}
//      }
//
//      if ( victim == NULL )
//      {
//	act( AT_WHITE,
//	    "A $p flies in from $T and falls harmlessly to the ground.",
//	    ch, Obj, dir_noun[rev_dir[dir]], TO_ROOM );
//	sprintf( buf,
//		"Your $p falls harmlessly to the ground %d room%s $T of
//here.",
//		dist, dist > 1 ? "s" : "" );
//	act( AT_WHITE, buf, ch, Obj, dir_name[dir], TO_CHAR );
//	char_from_room( ch );
//	char_to_room( ch, in_room );
//	oprog_throw_trigger( Obj, ch );
//	obj_from_char( Obj );
//	obj_to_room( Obj, to_room );
//	return;
//      }
//    }
//    if ( dist > 0 )
//    {
//      char_from_room( ch );
//      char_to_room( ch, in_room );
//      act( AT_WHITE, "A $p flys in from $T and hits $n!", victim, Obj,
//	  dir_noun[rev_dir[dir]], TO_NOTVICT );
//      act( AT_WHITE, "A $p flys in from $T and hits you!", victim, Obj,
//	  dir_noun[rev_dir[dir]], TO_CHAR );
//      sprintf( buf, "Your $p flew %d rooms %s and hit $N!", dist,
//	      dir_name[dir] );
//      act( AT_WHITE, buf, ch, Obj, victim, TO_CHAR );
//      oprog_throw_trigger( Obj, ch );
//      obj_from_char( Obj );
//      obj_to_room( Obj, to_room );
//      ranged_damage( ch, victim, per_type( ch, Obj ), gsn_archery );
//
//
//      update_skpell( ch, gsn_archery );
//
//      if ( IS_NPC( victim ) )
//      {
//         if ( victim->level > 3 )
//             victim->hunting = ch;
//      }
//      return;
//    }
//  }
//  obj_from_char( Obj );
//  obj_to_room( Obj, to_room );
//  act( AT_WHITE, "$n shoots a $p at $N!", ch, Obj, victim, TO_ROOM );
//  act( AT_WHITE, "You shoot your $p at $N.", ch, Obj, victim, TO_CHAR );
//  oprog_throw_trigger( Obj, ch );
//  ranged_damage( ch, victim, per_type( ch, Obj ), gsn_archery );
//  ranged_hit( ch, victim, Obj, TYPE_UNDEFINED );
//  /* put ranged_hit statement here */
//  return;
//}
//
//void ranged_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt )
//{
//
//    if ( victim->position == POS_DEAD )
//        return;
//
//    /*
//     * Stop up any residual loopholes.
//     */
//    if ( dam > 3500 )
//    {
//        char buf [ MAX_STRING_LENGTH ];
//
//      if ( dt != 91 && ch->level <= LEVEL_HERO
//      && dt != 40 )
//      {
//        if ( IS_NPC( ch ) && ch->desc && ch->desc->original )
//            sprintf( buf,
//                    "Damage: %d from %s by %s: > 3500 points with %d dt!",
//                    dam, ch->name, ch->desc->original->name, dt );
//        else
//            sprintf( buf,
//                    "Damage: %d from %s: > 3500 points with %d dt!",
//                    dam, ch->name, dt );
//
//        bug( buf, 0 );
//      }
//    }
//
//    if ( victim != ch )
//    {
//        /*
//         * Certain attacks are forbidden.
//         * Most other attacks are returned.
//         */
//        if ( is_safe( ch, victim ) )
//            return;
//        check_killer( ch, victim );
//
//
//        /*
//         * More charm stuff.
//         */
//        if ( victim->master == ch )
//            stop_follower( victim );
//
//        /*
//         * Inviso attacks ... not.
//        */
//        if ( IS_AFFECTED( ch, AFF_INVISIBLE ) )
//        {
//            affect_strip( ch, gsn_invis      );
//            affect_strip( ch, gsn_mass_invis );
//            REMOVE_BIT( ch->affected_by, AFF_INVISIBLE );
//            act(AT_GREY, "$n fades into existence.", ch, NULL, NULL, TO_ROOM
//);
//        }
//        if (IS_AFFECTED2( ch, AFF_PHASED ) )
//        {
//            affect_strip ( ch, skill_lookup("phase shift") );
//            affect_strip ( ch, skill_lookup("mist form") );
//            REMOVE_BIT( ch->affected_by2, AFF_PHASED );
//            act(AT_GREY, "$n returns from an alternate plane.", ch, NULL,
//NULL, TO_ROOM );
//        }
//        /*
//         * Damage modifiers.
//         */
//        if ( ch->race == RACE_OGRE )
//            dam -= dam / 20;
///*
//    if ( ch->level < L_APP && ch->class == CLASS_VAMPIRE )
//     if ( !IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
//     {
//      if ( time_info.hour > 6 && time_info.hour < 18 )
//      {
//       send_to_char( AT_RED,
//       "The sunlight has weakened your attack.\n\r", ch );
//       dam -= dam / 2;
//      }
//    }
//
//    if ( victim->level < L_APP && victim->class == CLASS_VAMPIRE )
//    if ( !IS_SET( victim->in_room->room_flags, ROOM_INDOORS ) )
//     {
//      if ( time_info.hour > 6 && time_info.hour < 18 )
//      {
//       send_to_char( AT_RED,
//       "The sunlight has weakened your defense.\n\r", victim );
//       dam += dam / 2;
//      }
//    }
//
//    if ( ch->class == CLASS_VAMPIRE )
//     if ( !IS_SET( ch->in_room->room_flags, ROOM_INDOORS ) )
//     {
//      if ( time_info.hour > 23 || time_info.hour < 1 )
//      {
//        dam *= 2;
//      }
//    }
//
//    if ( victim->class == CLASS_VAMPIRE )
//     if ( !IS_SET( victim->in_room->room_flags, ROOM_INDOORS ) )
//     {
//      if ( time_info.hour > 23 || time_info.hour < 1 )
//      {
//        dam /= 2;
//      }
//    }
//*/
//        /*
//         * Check for disarm, trip, parry, and dodge.
//         */
//        if  (  dt >= TYPE_HIT )
//        {
//
//            if ( check_dodge( ch, victim ) && dam > 0 )
//                return;
//         }
//
//
//
//        if ( IS_AFFECTED( victim, AFF_SANCTUARY ) )
//            dam /= 2;
//        if ( IS_AFFECTED2( victim, AFF_GOLDEN ) )
//            dam /= 4;
//        if ( IS_AFFECTED2( victim, AFF_DANCING ) )
//            dam += dam/2;
//        if ( IS_AFFECTED( victim, AFF_FIRESHIELD )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 8;
//        if ( IS_AFFECTED( victim, AFF_ICESHIELD )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 8;
//        if ( IS_AFFECTED( victim, AFF_CHAOS )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 4;
//        if ( IS_AFFECTED( victim, AFF_SHOCKSHIELD )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 4;
//        if ( IS_AFFECTED( victim, AFF_VIBRATING )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 4;
//        if ( IS_AFFECTED2( victim, AFF_INERTIAL ) )
//            dam -= dam / 8;
//        if ( IS_SET( victim->act, UNDEAD_TYPE( victim ) ) )
//            dam -= dam / 8;
//        if ( IS_AFFECTED2( victim, AFF_BLADE )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 4;
//        if ( IS_AFFECTED2( victim, AFF_FIELD )
//        && !( dt == gsn_backstab && chance( number_range( 5, 10 ) ) ) )
//            dam -= dam / 4;
//        if ( IS_AFFECTED( victim, AFF_PROTECT   )
//            && IS_EVIL( ch ) )
//            dam -= dam / 4;
//        if ( IS_AFFECTED2( victim, AFF_PROTECTION_GOOD )
//            && IS_EVIL( ch ) )
//            dam -= dam / 4;
//        if ( dam < 0 )
//            dam = 0;
//
//    }
//
//    /* We moved dam_message out of the victim != ch if above
//     * so self damage would show.  Other valid type_undefined
//     * damage is ok to avoid like mortally wounded damage - Kahn
//     */
//    if ( ( !IS_NPC(ch) ) && ( !IS_NPC(victim) ) )
//       dam -= dam/4;
//
//    /*
//     * Hurt the victim.
//     * Inform the victim of his new state.
//     */
//    if ( !IS_NPC(ch) && prime_class( ch ) == CLASS_WARRIOR )
//       dam += dam/2;
//    if ( ( !IS_NPC(ch) ) && (ch->race == RACE_OGRE ) )
//       dam += dam/10;
//    if (!IS_NPC(ch) && !IS_NPC(victim))
//      dam /= number_range(2, 4);
//    if ( dt != TYPE_UNDEFINED )
//        dam_message( ch, victim, dam, dt );
//
//    if ( dam > 25 && number_range( 0, 100 ) <= 15 )
//      item_damage(victim, dam);
//
//    victim->hit -= dam;
//    if ( ( ( !IS_NPC( victim )                  /* so imms only die by */
//           && IS_NPC( ch )                      /* the hands of a PC   */
//           && victim->level >= LEVEL_IMMORTAL )
//         ||
//           ( !IS_NPC( victim )                   /* so imms don,t die  */
//           && victim->level >= LEVEL_IMMORTAL    /* by poison type dmg */
//           && ch == victim ) )                   /* since an imm == pc */
//         && victim->hit < 1 )
//            victim->hit = 1;
//  if ( victim->position == POS_DEAD  )
//        return;
//
//    update_pos( victim );
//
//    switch( victim->position )
//    {
//    case POS_MORTAL:
//        send_to_char(AT_RED,
//            "You are mortally wounded, and will die soon, if not
//aided.\n\r",
//            victim );
//        act(AT_RED, "$n is mortally wounded, and will die soon, if not
//aided.",
//            victim, NULL, NULL, TO_ROOM );
//        break;
//    case POS_INCAP:
//        send_to_char(AT_RED,
//            "You are incapacitated and will slowly die, if not aided.\n\r",
//            victim );
//        act(AT_RED, "$n is incapacitated and will slowly die, if not
//aided.",
//            victim, NULL, NULL, TO_ROOM );
//        break;
//
//    case POS_STUNNED:
//        send_to_char(AT_WHITE,"You are stunned, but will probably
//recover.\n\r",
//            victim );
//        act(AT_WHITE, "$n is stunned, but will probably recover.",
//            victim, NULL, NULL, TO_ROOM );
//        break;
//
//    case POS_DEAD:
//        send_to_char(AT_BLOOD, "You have been KILLED!!\n\r\n\r", victim );
//        act(AT_BLOOD, "$n is DEAD!!", victim, NULL, NULL, TO_ROOM );
//        break;
//
//    default:
//        if ( dam > MAX_HIT(victim) / 4 )
//            send_to_char(AT_RED, "That really did HURT!\n\r", victim );
//        if ( victim->hit < MAX_HIT(victim) / 4 )
//            send_to_char(AT_RED, "You sure are BLEEDING!\n\r", victim );
//        break;
//    }
//
//    /*
//     * Sleep spells and extremely wounded folks.
//     */
//    if ( !IS_AWAKE( victim ) )
//        stop_fighting( victim, FALSE );
//    /*
//     * Payoff for killing things.
//     */
//
//    if ( victim->position == POS_DEAD )
//    {
//        if ( !IS_ARENA(ch) )
//        {
//          group_gain( ch, victim );
//
//          if(((ch->guild != NULL) ? ch->guild->type & GUILD_CHAOS : 0)
//           && ch->guild == victim->guild
//           && victim->guild_rank > ch->guild_rank)
//          {
//            int temp;
//            temp = ch->guild_rank;
//            ch->guild_rank = victim->guild_rank;
//            victim->guild_rank = temp;
//          }
//          if ( ( !IS_NPC(ch) ) && ( !IS_NPC(victim) ) )
//          {
//            CLAN_DATA  *pClan;
//            CLAN_DATA  *Cland;
//            if ( ch->clan != victim->clan )
//            {
//          if ( !(abs(ch->level - victim->level) > 10 ))
//          {
//              if ( (pClan = get_clan_index(ch->clan)) != NULL )
//                pClan->pkills++;
//              if ( (Cland = get_clan_index(victim->clan)) != NULL )
//                Cland->pdeaths++;
//           }            }
///*            REMOVE_BIT(victim->act, PLR_THIEF);*/
//          }
//          if ( ( !IS_NPC(ch) ) && ( !IS_NPC(victim) ) )
//          {
//          if ( !(abs(ch->level - victim->level) > 10 ) )
//          {
//          ch->pkills++;
//          victim->pkilled++;
//          }
//          }
//          if ( ( !IS_NPC(ch) ) && ( IS_NPC(victim) ) )
//          {
//           CLAN_DATA    *pClan;
//           if ( (pClan=get_clan_index(ch->clan)) != NULL )
//             pClan->mkills++;
//          }
//          if ( ( IS_NPC(ch) ) && (!IS_NPC(victim)) )
//          {
//           CLAN_DATA   *pClan;
//           if ( (pClan=get_clan_index(victim->clan)) != NULL )
//             pClan->mdeaths++;
//          }
//
//          if ( !IS_NPC( victim ) )
//          {
//            /*
//             * Dying penalty:
//             * 1/2 way back to previous level.
//             */
//            if ( victim->level < LEVEL_HERO
//            || ( victim->level >= LEVEL_HERO && IS_NPC( ch ) ) )
//                death_xp_loss( victim );
//            sprintf( log_buf, "%s killed by %s at %d.", victim->name,
//                ch->name, victim->in_room->vnum );
//            log_string( log_buf, CHANNEL_LOG, -1 );
//            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
//            if ( !IS_NPC( ch )
//            && IS_SET( victim->act, PLR_THIEF )
//            && ch->guild
//            && !strcmp( ch->guild->name, "MERCENARY" ) )
//                {
//                REMOVE_BIT( victim->act, PLR_THIEF );
//                info( "%s the puny thief gets destroyed by the &rMERCENARY&C
//%s!",
//                      (int)victim->name, (int)ch->name );
//                }
//            else if ( !IS_NPC( ch )
//            && IS_SET( ch->act, PLR_THIEF )
//            && victim->guild
//            && !strcmp( victim->guild->name, "MERCENARY" ) )
//                info( "%s, the sly thief, has killed the &rMERCENARY&C %s.",
//                      (int)ch->name, (int)victim->name );
//            else
//                info( "%s gets slaughtered by %s!", (int)victim->name,
//                  (int)(IS_NPC(ch) ? ch->short_descr : ch->name) );
//            save_clans();
//          }
//        }
//        else
//        {
//          sprintf(log_buf, "&C%s &chas defeated &C%s &cin the arena!",
//                  ch->name, victim->name);
//          wiznet(log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);
//          log_string(log_buf, CHANNEL_LOG, -1);
//          challenge(log_buf, 0, 0);
//          ch->arenawon++;
//          victim->arenalost++;
//        }
//
//        raw_kill( ch, victim );
//
//        /* Ok, now we want to remove the deleted flag from the
//         * PC victim.
//         */
//        if ( !IS_NPC( victim ) )
//            victim->deleted = FALSE;
//
//        if ( !IS_NPC( ch ) && IS_NPC( victim ) )
//        {
//            if ( IS_SET( ch->act, PLR_AUTOLOOT ) )
//                do_get( ch, "all corpse" );
//            else
//                do_look( ch, "in corpse" );
//
//            if ( IS_SET( ch->act, PLR_AUTOCOINS ) )
//                do_get( ch, "all.coin corpse" );
//            if ( IS_SET( ch->act, PLR_AUTOSAC  ) )
//                do_sacrifice( ch, "corpse" );
//        }
//
//        return;
//    }
//
//    if ( victim == ch )
//        return;
//
//    /*
//     * Take care of link dead people.
//     */
//    if ( !IS_NPC( victim ) && !victim->desc )
//    {
//        if ( number_range( 0, victim->wait ) == 0 )
//        {
//            do_recall( victim, "" );
//            return;
//        }
//    }
//
//    /*
//     * Wimp out?
//     */
//    if ( IS_NPC( victim ) && dam > 0 )
//    {
//        if ( ( IS_SET( victim->act, ACT_WIMPY ) && number_bits( 1 ) == 0
//              && victim->hit < MAX_HIT(victim) / 2 )
//            || ( IS_AFFECTED( victim, AFF_CHARM ) && victim->master
//                && victim->master->in_room != victim->in_room ) )
//            do_flee( victim, "" );
//    }
//
//    if ( !IS_NPC( victim )
//        && victim->hit   > 0
//        && victim->hit  <= victim->wimpy
//        && victim->wait == 0 )
//        do_flee( victim, "" );
//
//    tail_chain( );
//    return;
//}
//
//void ranged_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *arrow, int dt
//)
//{
//    char      buf [ MAX_STRING_LENGTH ];
//    int       victim_ac;
//    int       thac0;
//    int       thac0_00;
//    int       thac0_97;
//    int       dam;
//    int       diceroll;
//
//    /*
//     * Can't beat a dead char!
//     * Guard against weird room-leavings.
//     */
//	/* if this causes crash  remove below kjodo */
//    if ( victim->position == POS_DEAD )
//    {
//        if ( !IS_ARENA(ch) )
//        {
//	  group_gain( ch, victim );
//
//          if(((ch->guild != NULL) ? ch->guild->type & GUILD_CHAOS : 0)
//           && ch->guild == victim->guild
//           && victim->guild_rank > ch->guild_rank)
//          {
//            int temp;
//            temp = ch->guild_rank;
//            ch->guild_rank = victim->guild_rank;
//            victim->guild_rank = temp;
//          }
//          if ( ( !IS_NPC(ch) ) && ( !IS_NPC(victim) ) )
//          {
//            CLAN_DATA  *pClan;
//            CLAN_DATA  *Cland;
//            if ( ch->clan != victim->clan )
//            {
//              if ( (pClan = get_clan_index(ch->clan)) != NULL )
//                pClan->pkills++;
//              if ( (Cland = get_clan_index(victim->clan)) != NULL )
//                Cland->pdeaths++;
//            }
///*            REMOVE_BIT(victim->act, PLR_THIEF);*/
//          }
//          if ( ( !IS_NPC(ch) ) && ( IS_NPC(victim) ) )
//          {
//           CLAN_DATA    *pClan;
//           if ( (pClan=get_clan_index(ch->clan)) != NULL )
//             pClan->mkills++;
//          }
//          if ( ( IS_NPC(ch) ) && (!IS_NPC(victim)) )
//          {
//           CLAN_DATA   *pClan;
//           if ( (pClan=get_clan_index(victim->clan)) != NULL )
//             pClan->mdeaths++;
//          }
//
//	  if ( !IS_NPC( victim ) )
//	  {
//	    /*
//	     * Dying penalty:
//	     * 1/2 way back to previous level.
//	     */
//	    if ( victim->level < LEVEL_HERO
//	    || ( victim->level >= LEVEL_HERO && IS_NPC( ch ) ) )
//		death_xp_loss( victim );
//	    sprintf( log_buf, "%s killed by %s at %d.", victim->name,
//	        ch->name, victim->in_room->vnum );
//	    log_string( log_buf, CHANNEL_LOG, -1 );
//            wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
//	    if ( !IS_NPC( ch )
//	    && IS_SET( victim->act, PLR_THIEF )
//	    && ch->guild
//	    && !strcmp( ch->guild->name, "MERCENARY" ) )
//		{
//		REMOVE_BIT( victim->act, PLR_THIEF );
//		info( "%s the puny thief gets destroyed by the
//&rMERCENARY&C %s!",
//		      (int)victim->name, (int)ch->name );
//		}
//	    else if ( !IS_NPC( ch )
//	    && IS_SET( ch->act, PLR_THIEF )
//	    && victim->guild
//	    && !strcmp( victim->guild->name, "MERCENARY" ) )
//		info( "%s, the sly thief, has killed the &rMERCENARY&C
//%s.",
//		      (int)ch->name, (int)victim->name );
//	    else
//	        info( "%s gets slaughtered by %s!", (int)victim->name,
//                  (int)(IS_NPC(ch) ? ch->short_descr : ch->name) );
//	    save_clans();
//	  }
//	}
//	else
//	{
//	  sprintf(log_buf, "&C%s &chas defeated &C%s &cin the arena!",
//	          ch->name, victim->name);
//	  wiznet(log_buf, NULL, NULL, WIZ_DEATHS, 0, 0);
//	  log_string(log_buf, CHANNEL_LOG, -1);
//	  challenge(log_buf, 0, 0);
//	}
//
//	raw_kill( ch, victim );
//
//	/* Ok, now we want to remove the deleted flag from the
//	 * PC victim.
//	 */
//	if ( !IS_NPC( victim ) )
//	    victim->deleted = FALSE;
//      }
//
//	/* if crash remove above */
//
//    if ( victim->position == POS_DEAD )
//        return;
//
//    if ( IS_STUNNED( ch, STUN_NON_MAGIC ) ||
//         IS_STUNNED( ch, STUN_TOTAL ) )
//      return;
//
//    /*
//     * Figure out the type of damage message.
//     */
//
//    if ( dt == TYPE_UNDEFINED )
//    {
//        dt = TYPE_HIT;
//        if ( arrow && arrow->item_type == ITEM_WEAPON )
//            dt += arrow->value[3];
//    }
//
//    /*
//     * Calculate to-hit-armor-class-0 versus armor.
//     */
//    if ( IS_NPC( ch ) )
//    {
//        thac0_00 =  18;
//        thac0_97 = -24;
//    }
//    else
//    {
//        thac0_00 = class_table[prime_class(ch)].thac0_00;
//        thac0_97 = class_table[prime_class(ch)].thac0_97;
//    }
//
//    if (!IS_NPC(ch))
//        thac0     = interpolate( ch->level, thac0_00, thac0_97 )
//                  - GET_HITROLL( ch );
//    else
//        thac0     = interpolate(ch->level, thac0_00, thac0_97 )
//                  - (ch->level + ch->level/2);
//
//    if ( ( !IS_NPC( ch ) ) && ( ch->pcdata->learned[gsn_enhanced_hit] > 0
//) )
//    {
//       thac0 -= ch->pcdata->learned[gsn_enhanced_hit] / 5;
//       update_skpell( ch, gsn_enhanced_hit );
//    }
//
//    victim_ac = UMAX( -15, GET_AC( victim ) / 10 );
///*
//    if ( victim->level < L_APP && victim->class == CLASS_VAMPIRE )
//     if ( !IS_SET( victim->in_room->room_flags, ROOM_INDOORS ) )
//     {
//      if ( time_info.hour > 6 && time_info.hour < 18 )
//      {
//        victim_ac += victim->level * 2;
//      }
//    }
//*/
//    if ( !can_see( ch, victim ) )
//        victim_ac -= 4;
//
//    /*
//     * The moment of excitement!
//     */
//    dam = 0;
//
//    while ( ( diceroll = number_bits( 5 ) ) >= 20 )
//        ;
//
//    if (     diceroll == 0
//        || ( diceroll != 19 && diceroll < thac0 - victim_ac ) )
//    {
//        /* Miss. */
//        ranged_damage( ch, victim, 0, dt );
//        tail_chain( );
//        return;
//    }
//
//    /*
//     * Hit.
//     * Calc damage.
//     */
//
//  if ( !victim || victim->position == POS_DEAD)
//        return;
//    if ( IS_NPC( ch ) )
//    {
//        dam = number_range( ch->level / 3, ch->level * 3 / 2 );
//        if ( arrow )
//            dam += dam / 3;
//    }
//    else
//    {
//        if ( arrow )
//            dam = number_range( arrow->value[1], arrow->value[2] );
//
//        if ( arrow && dam > 1000 && !IS_IMMORTAL(ch) )
//        {
//            sprintf( buf, "One_hit dam range > 1000 from %d to %d",
//                    arrow->value[1], arrow->value[2] );
//            bug( buf, 0 );
//            if ( arrow->name )
//              bug( arrow->name, 0 );
//        }
//    }
//
//    /*
//     * Bonuses.
//     */
//    dam += GET_DAMROLL( ch );
//    if ( arrow && IS_SET( arrow->extra_flags, ITEM_POISONED ) )
//        dam += dam / 8;
//    if (arrow && IS_SET( arrow->extra_flags, ITEM_FLAME ) )
//        dam += dam / 8;
//     if (arrow && IS_SET( arrow->extra_flags, ITEM_CHAOS ) )
//        dam += dam / 4;
//    if (arrow && IS_SET( arrow->extra_flags, ITEM_ICY   ) )
//        dam += dam / 8;
//    if ( !IS_NPC( ch ) && ch->pcdata->learned[gsn_enhanced_damage] > 0 )
//    {
//        dam += dam * ch->pcdata->learned[gsn_enhanced_damage] / 150;
//        update_skpell( ch, gsn_enhanced_damage );
//    }
//    if ( !IS_NPC( ch ) && ch->pcdata->learned[gsn_enhanced_two] > 0 )
//    {
//        dam += dam / 4 * ch->pcdata->learned[gsn_enhanced_two] / 150;
//        update_skpell( ch, gsn_enhanced_two );
//    }
//    if ( !IS_NPC( ch ) && ch->pcdata->learned[gsn_enhanced_three] > 0 )
//    {
//        dam += dam / 4 * ch->pcdata->learned[gsn_enhanced_three] / 150;
//        update_skpell( ch, gsn_enhanced_three );
//    }
//    if ( !IS_AWAKE( victim ) )
//        dam *= 2;
//    if ( dam <= 0 )
//        dam = 1;
//    ranged_damage( ch, victim, dam, dt );
//    tail_chain( );
//    return;
//}
