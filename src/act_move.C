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
#include "merc.h"
#include <stdlib.h>
#include <ctype.h>

#include "classes.h"
#include "db.h"
#include "handler.h"
#include "interp.h"
#include "comm.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "act_info.h"
#include "act_move.h"
#include "ability.h"
#include "config.h"
#include "clan.h"
#include "utils.h"


//#define VERBOSE


char *	const	dir_name	[]		=
{
  //  "north", "east", "south", "west", "up", "down"   Modified by SinaC 2003
  "north", "east", "south", "west", "up", "down", "northeast", "northwest", "southeast", "southwest", "special"
};

// Modified by SinaC 2003
//const char * short_dir_name[6]     = {"n","e","s","w","u","d"};
//const char * short_rev_dir_name[6] = {"s","w","n","e","d","u"};
const char * short_dir_name[]     = {"n","e","s","w","u","d","ne","nw","se","sw", "special"};
const char * short_rev_dir_name[] = {"s","w","n","e","d","u","sw","se","nw","ne", "special"};



const	int	rev_dir		[]		=
{
  //  2, 3, 0, 1, 5, 4           Modified by SinaC 2003
  2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};

const	int	movement_loss	[SECT_MAX]	=
{
  // Has deleted SECT_UNUSED/*6*/ and has replaced it by SECT_BURNING
  // Have switched the 2 last one, was 10, 6 before
  1, 2, 2, 3, 4, 6, 4, 1, /*6*/10, 6, 10, 15
};



/*
 * Local functions.
 */
int	find_door	args( ( CHAR_DATA *ch, const char *arg ) );
OBJ_DATA*	has_key		args( ( CHAR_DATA *ch, int key ) ); // Modified by SinaC 2003

void move_char( CHAR_DATA *ch, int door, const bool follow, const bool climbing,
		const bool passDoor ) { // SinaC 2003, passDoor = TRUE: can pass thru closed door
  CHAR_DATA *fch;
  CHAR_DATA *fch_next;
  ROOM_INDEX_DATA *in_room;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;

  //if ( door < 0 || door > 5 ) { Modified by SinaC 2003
  if ( door < 0 || door >= MAX_DIR ) {
    bug( "Do_move: bad door %d.", door );
    return;
  }

#ifdef VERBOSE
  log_stringf("MOVE: start: [%s] [%s] [%s] [%s]",
	      NAME(ch), dir_name[door], follow?"true":"false", climbing?"true":"false");
#endif

  // Get's char mount, added by SinaC 2003 for mount code
  CHAR_DATA *mount = get_mount( ch );
  // if mount is not in the same room, we do as player doesn't have a mount
  if ( mount && mount->in_room != ch->in_room )
    mount = NULL;

  // Added by SinaC 2003
  if ( mount ) {
    if ( mount->position < POS_STANDING ) {
      act("$N is not standing.", ch, NULL, mount, TO_CHAR );
      return;
    }
  }

#ifdef VERBOSE
  log_stringf("MOVE: mount: [%s]", mount?NAME(mount):"<NO MOUNT>");
#endif

  // Added by SinaC 2003, when mounted: tests are only done on mount
  if ( mount ) {
    if ( IS_AFFECTED( mount, AFF_ROOTED ) ) {
      act("$N's legs are immobilised by {groots{x.", ch, NULL, mount, TO_CHAR );
      return;
    }
  }
  else
    // Added by SinaC 2001
    if ( IS_AFFECTED( ch, AFF_ROOTED ) ) {
      send_to_char("You are immobilised because of the {groots{x around your legs.\n\r", ch );
      return;
    }

  // Added by SinaC 2001 for walking drunk
  // Uh oh, another drunk Frenchman on the loose! :)
  if ( !IS_NPC(ch) 
       && !IS_IMMORTAL(ch)
       // Added by SinaC 2003 (drunk doesn't matter when mounting)
       && !mount
       && !passDoor // SinaC 2003
       && door != DIR_SPECIAL // SinaC 2003, we suppose never missing a dive
       && ch->pcdata->condition[COND_DRUNK] > 10 )
    if (ch->pcdata->condition[COND_DRUNK] > number_percent()) {
      act("You feel a little drunk.. not to mention kind of lost..",
	  ch,NULL,NULL,TO_CHAR);
      act("$n looks a little drunk.. not to mention kind of lost..",
	  ch,NULL,NULL,TO_ROOM);
      //door = number_range(0,5);
      door = number_range(0,MAX_DIR); // Modified by SinaC 2003
    }
    else {
      act("You feel a little.. drunk..",ch,NULL,NULL,TO_CHAR);
      act("$n looks a little.. drunk..",ch,NULL,NULL,TO_ROOM);
    }
  // end
  
  in_room = ch->in_room;
  // Added by SinaC 2003, when mounting, tests are only done on the mount
  if ( mount ) {
    if ( ( pexit   = in_room->exit[door] ) == NULL
	 || ( to_room = pexit->u1.to_room  ) == NULL 
	 || !can_see_room(mount,pexit->u1.to_room) )
      if ( door == DIR_SPECIAL ) { // special dir: dive in an hole
	act("$N dives and slam $S head on the ground.",ch,NULL,mount,TO_CHAR);
	act("$N dives and slam $S head on the ground.",ch,NULL,mount,TO_NOTVICT);
	return;
      }
      else {
	// Modified by SinaC 2001
	//send_to_char( "Alas, you cannot go that way.\n\r", ch );
	act("$N almost goes $t, but suddenly realize that there's no exit there.",ch,dir_name[door],mount,TO_CHAR);
	act("$N looks like $e's about to go $t, but suddenly stops short and looks confused.",ch,dir_name[door],mount,TO_NOTVICT);
	return;
      }
  }
  else
    if ( ( pexit   = in_room->exit[door] ) == NULL
	 || ( to_room = pexit->u1.to_room  ) == NULL 
	 || !can_see_room(ch,pexit->u1.to_room) )
      if ( door == DIR_SPECIAL ) { // special dir: dive in an hole
	act("You dive and slam your head on the ground.",ch,NULL,NULL,TO_CHAR);
	act("$n dives and slam $s head on the ground.",ch,NULL,NULL,TO_ROOM);
	return;
      }
      else {
	// Modified by SinaC 2001
	//send_to_char( "Alas, you cannot go that way.\n\r", ch );
	act("You almost go $T, but suddenly realize that there's no exit there.",ch,NULL,dir_name[door],TO_CHAR);
	act("$n looks like $e's about to go $T, but suddenly stops short and looks confused.",ch,NULL,dir_name[door],TO_ROOM);
	return;
      }

  // Added by SinaC 2003, climb exits.
  if ( mount ) {
    if ( IS_SET(pexit->exit_info, EX_CLIMB )
	 && !IS_AFFECTED(mount,AFF_FLYING)
	 && !climbing ) {
      act("Your mount must be flying or climbing.", ch, NULL, NULL, TO_CHAR );
      return;
    }
  }
  else {
    if ( IS_SET(pexit->exit_info, EX_CLIMB )
	 && !IS_AFFECTED(ch,AFF_FLYING)
	 && !climbing ) {
      act("You must flying or climbing to go $T.", ch, NULL, dir_name[door], TO_CHAR );
      return;
    }
  }

  // --Added by SinaC 2003, when mounting, tests are only done on the mount
  if ( mount ) {
    if (IS_SET(pexit->exit_info, EX_CLOSED)
	&& (!IS_AFFECTED(mount, AFF_PASS_DOOR) 
	    || IS_SET(pexit->exit_info,EX_NOPASS))
	&& !IS_TRUSTED(mount,ANGEL)) {
      act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
      return;
    }
  }
  //else SinaC 2003, mount and mounter must be affected by pass_door
  if (
      IS_SET(pexit->exit_info, EX_CLOSED)
      && (!IS_AFFECTED(ch, AFF_PASS_DOOR) 
	  || IS_SET(pexit->exit_info,EX_NOPASS))
      && !IS_TRUSTED(ch,ANGEL)
      && !passDoor // SinaC 2003
      ) {
    act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
    return;
  }

  if ( IS_AFFECTED(ch, AFF_CHARM)
       && ch->master != NULL
       && in_room == ch->master->in_room ) {
    send_to_char( "What?  And leave your beloved master?\n\r", ch );
    return;
  }

  if ( !is_room_owner(ch,to_room)
       && room_is_private( to_room ) ) {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }


  // --Added by SinaC 2003, when mounting, tests are only done on the mount
  if ( mount ) {
    if ( to_room->cstat(maxsize) != SIZE_NOSIZE
	 && to_room->cstat(maxsize) < mount->cstat(size) ) {
      act("$N is too huge to go that direction.", ch, NULL, mount, TO_CHAR );
      return;
    }
  }
  //else SinaC 2003, mount and mounter must have the right size
    // Added by SinaC 2001
    // if the player is too huge he/she can't enter the room
  if ( !IS_IMMORTAL(ch)
       // Modified by SinaC 2001
       && to_room->cstat(maxsize) != SIZE_NOSIZE
       && ( to_room->cstat(maxsize) < ch->cstat(size)
	    /*|| pexit->max_size < ch->cstat(size)*/) ) {
    send_to_char( "You're too huge to go to that direction.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  // These tests were inside the if !IS_NPC(ch)
  // Modified by SinaC 2001
  if ( in_room->cstat(sector) == SECT_AIR
       || to_room->cstat(sector) == SECT_AIR ) {
    // Added by SinaC 2003, when mounting, each test are only done on the mount
    if ( mount ) {
      if ( !IS_AFFECTED(mount, AFF_FLYING) ) {
	act("$N can't fly.", ch, NULL, mount, TO_CHAR );
	return;
      }
    }
    else
      if ( !IS_AFFECTED(ch, AFF_FLYING) 
	   && !IS_IMMORTAL(ch) ) {
	send_to_char( "You can't fly.\n\r", ch );
	return;
      }
  }

  // Modified by SinaC 2001
  // if we are in a swim room or go in a swim room
  if ( in_room->cstat(sector) == SECT_WATER_SWIM 
       || to_room->cstat(sector) == SECT_WATER_SWIM ) {
    // Added by SinaC 2003, when mounting, each test are only done on the mount
    if ( mount ) {
      if ( !IS_AFFECTED( mount, AFF_FLYING )
	   && !IS_AFFECTED( mount, AFF_SWIM )
	   && !IS_AFFECTED2( mount, AFF2_WALK_ON_WATER ) ) {
	act("$N needs to be simming, flying or walking on water.\n\r", ch, NULL, mount, TO_CHAR );
	return;
      }
    }
    else
      // if we're flying or walking on water: it's okay
      if ( !IS_IMMORTAL(ch)
	   && !IS_AFFECTED( ch, AFF_FLYING )
	   && !IS_AFFECTED( ch, AFF_SWIM )
	   && !IS_AFFECTED2( ch, AFF2_WALK_ON_WATER ) ) {
	// else do we have a boat ?
	OBJ_DATA *obj;
	bool found = FALSE;
	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
	  if ( obj->item_type == ITEM_BOAT ) {
	    found = TRUE;
	    break;
	  }
	}
	if (!found ) {
	  send_to_char( "You need a boat to go there, or be swimming, flying or walking on water.\n\r", ch );
	  return;
	}
      }
  }

  // Modified by SinaC 2001
  // if we are in a noswim room or go in a noswim room  or underwater
  if ( in_room->cstat(sector) == SECT_WATER_NOSWIM 
       || to_room->cstat(sector) == SECT_WATER_NOSWIM 
       // Added by SinaC 2003
       || in_room->cstat(sector) == SECT_UNDERWATER
       || to_room->cstat(sector) == SECT_UNDERWATER ) {
    // Added by SinaC 2003, when mounting, each test are only done on the mount
    if ( mount ) {
      if ( !IS_AFFECTED( mount, AFF_FLYING )
	   && !IS_AFFECTED2( mount, AFF2_WALK_ON_WATER ) ) {
	act("$N needs to be flying or walking on water.\n\r", ch, NULL, mount, TO_CHAR );
	return;
      }
    }
    else
      // if we're flying, swimming or walking in water: it's okay
      if ( !IS_IMMORTAL(ch) 
	   && !IS_AFFECTED( ch, AFF_FLYING )
	   //&& !IS_AFFECTED( ch, AFF_SWIM )  Can't be swimmming (NOSWIM sector)
	   && !IS_AFFECTED2( ch, AFF2_WALK_ON_WATER ) ) {
	
	//send_to_char( "You need to be swimming, flying or walking on water.\n\r", ch );
	send_to_char( "You need to be flying or walking on water.\n\r", ch );
	return;
      }
  }


#ifdef VERBOSE
  log_stringf("MOVE: found an available pass-able exit");
#endif

  // ONEXITING: test if there is people in the room who prevent from leaving room
  // Added by SinaC 2003 for script
  bool notLeaving = FALSE;
  for ( fch = in_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;
    if ( ch == fch
	 || fch->leader == ch // SinaC 2003, people following group leader doesn't trigger
	 || fch->master == ch
	 || ch->leader == fch
	 || ch->master == fch )
      continue;
    if (can_see(fch,ch)) {

#ifdef VERBOSE
  log_stringf("MOVE: onExiting [%s]  [%s]", NAME(fch), NAME(ch) );
#endif

      Value args[] = {ch, short_dir_name[door]};
      bool value = mobprog(fch,ch,"onExiting",args);
      if ( value )
	notLeaving = TRUE;
      if ( !follow ) { // Leader
#ifdef VERBOSE
  log_stringf("MOVE: onExitingLeader [%s]  [%s]", NAME(fch), NAME(ch) );
#endif
	Value args[] = {ch, short_dir_name[door]};
	bool value = mobprog(fch,ch,"onExitingLeader",args);
	if ( value )
	  notLeaving = TRUE;
      }
    }
    // Added by SinaC 2003, is mount unable to leave?
    if ( mount 
	 && can_see( fch, mount ) ) {
#ifdef VERBOSE
  log_stringf("MOVE: onExiting MOUNT [%s]  [%s]", NAME(fch), NAME(ch) );
#endif
      Value args[] = {mount, short_dir_name[door]};
      bool value = mobprog(fch,mount,"onExiting",args);
      if ( value )
	notLeaving = TRUE;
      // no need to test !follow, a mount will never be the leader of a group
    }
  }

#ifdef VERBOSE
  log_stringf("MOVE: onExiting ROOM [%d]  [%s]", in_room->vnum, NAME(ch) );
#endif
  // ONEXITING: test if the room prevents from leaving
  // Added by SinaC 2003 for room program
  Value args[] = {ch, short_dir_name[door]};
  bool value = roomprog(in_room,ch,"onExiting",args);
  if ( value )
    notLeaving = TRUE;

  // Leader
  if ( !follow ) {
#ifdef VERBOSE
  log_stringf("MOVE: onExitingLeader ROOM [%d]  [%s]", in_room->vnum, NAME(ch) );
#endif
    Value args[] = {ch, short_dir_name[door]};
    bool value = roomprog(in_room,ch,"onExitingLeader",args);
    if ( value )
      notLeaving = TRUE;
  }
  // Added by SinaC 2003, is mount unable to leave ?
  if ( mount ) {
#ifdef VERBOSE
  log_stringf("MOVE: onExiting ROOM MOUNT [%d]   [%s]", in_room->vnum, NAME(ch) );
#endif
    Value argsm[] = {mount, short_dir_name[door]};
    bool value = roomprog(in_room,mount,"onExiting",argsm);
    if ( value )
      notLeaving = TRUE;
  }
  // no need to test !follow on mount, a mount will never be the leader of a group

#ifdef VERBOSE
  log_stringf("MOVE: onEntering ROOM [%d]  [%s]", to_room->vnum, NAME(ch) );
#endif
  // ONENTERING: test if destination room prevents from entering
  // Added by SinaC 2003 for room program
  Value args2[] = {ch, short_rev_dir_name[door]};
  value = roomprog(to_room,ch,"onEntering",args2);
  if ( value )
    notLeaving = TRUE;
  // Leader
  if ( !follow ) {
#ifdef VERBOSE
  log_stringf("MOVE: onEnteringLeader ROOM [%d]   [%s]", to_room->vnum, NAME(ch) );
#endif
    Value args2[] = {ch, short_rev_dir_name[door]};
    value = roomprog(to_room,ch,"onEnteringLeader",args2);
    if ( value )
      notLeaving = TRUE;
  }
  // Added by SinaC 2003, is mount unable to leave ?
  if ( mount ) {
#ifdef VERBOSE
  log_stringf("MOVE: onEntering ROOM MOUNT [%d]   [%s]", to_room->vnum, NAME(ch) );
#endif
    Value argsm2[] = {mount, short_rev_dir_name[door]};
    value = roomprog(to_room,mount,"onEntering",argsm2);
    if ( value )
      notLeaving = TRUE;
  }
  // no need to test !follow on mount, a mount will never be the leader of a group

  // Added by SinaC 2003 to be able to prevent a char from leaving/entering a room
  if ( notLeaving )
    return;

#ifdef VERBOSE
  log_stringf("MOVE: Allowed to go from [%d] to [%d]   [%s]", in_room->vnum, to_room->vnum, NAME(ch) );
#endif

  if ( !IS_NPC(ch) ) {
    int move;
    
    /* Modified by Oxtal and SinaC 2000 */
    if (to_room->guild 
	&& !IS_IMMORTAL(ch)
	//&& !(to_room->guild & ch->cstat(classes)) 
	&& !check_guild_room( to_room->guild, ch->cstat(classes) ) ) {
      send_to_char( "You aren't allowed in there.\n\r", ch );
      return;
    }

    // Modified by SinaC 2001
    // Some tests has been moved above the if !IS_NPC(ch)

    // Added by SinaC 2003, when mounting a player doesn't lose any move point
    if ( !mount ) {
      // Modified by SinaC 2001
      move = movement_loss[UMIN(SECT_MAX-1, in_room->cstat(sector))]
	+ movement_loss[UMIN(SECT_MAX-1, to_room->cstat(sector))];
      
      move /= 2;  // i.e. the average
      
      /* conditional effects */
      if (IS_AFFECTED(ch,AFF_FLYING) 
	  || IS_AFFECTED(ch,AFF_HASTE))
	move /= 2;
      
      if (IS_AFFECTED(ch,AFF_SLOW))
	move *= 2;
      
      // Added by SinaC 2003
      if ( IS_AFFECTED2( ch, AFF2_FREE_MOVEMENT ) ) {
	move = 1;
      }
      int fleetfooted_chance = get_ability( ch, gsn_fleetfooted );
      if ( fleetfooted_chance > 0 )
	if ( number_percent() > fleetfooted_chance )
	  check_improve( ch, gsn_fleetfooted, FALSE, 8 );
	else {
	  check_improve( ch, gsn_fleetfooted, TRUE, 5 );
	  move = 1;
	}
      
      if ( ch->move < move ) {
	send_to_char( "You are too exhausted.\n\r", ch );
	return;
      }
      ch->move -= move;
    }
    WAIT_STATE( ch, 1 );
  }

  // Modified by SinaC 2001 for walking drunk
  if (!IS_NPC(ch) 
      && !IS_IMMORTAL(ch)
      // Added by SinaC 2003 (drunk doesn't matter when mounting)
      && !mount
      && !passDoor // SinaC 2003
      && ch->pcdata->condition[COND_DRUNK] > 10)
    act("$n stumbles off drunkenly on $s way $T.",
	ch,NULL,dir_name[door],TO_ROOM);
  else
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
	 && ch->invis_level < LEVEL_HERO)
      if ( door == DIR_SPECIAL ) // special dir: dive in an hole, SinaC 2003
	act( "$n dives.", ch, NULL, NULL, TO_ROOM );
      else
	act( "$n leaves $T.", ch, NULL, dir_name[door], TO_ROOM );

#ifdef VERBOSE
  log_stringf("MOVE: MOVING [%s]", NAME(ch) );
#endif

  char_from_room( ch );
  char_to_room( ch, to_room );
  
  // Modified by SinaC 2001 for walking drunk
  if (!IS_NPC(ch) 
      && !IS_IMMORTAL(ch) 
      // Added by SinaC 2003 (drunk doesn't matter when mounting)
      && !mount
      && !passDoor // SinaC 2003
      && ch->pcdata->condition[COND_DRUNK] > 10)
    act("$n stumbles in drunkenly, looking all nice and French.",
	ch,NULL,NULL,TO_ROOM);
  else
    if ( !IS_AFFECTED(ch, AFF_SNEAK)
	 && ch->invis_level < LEVEL_HERO)
      act( "$n has arrived.", ch, NULL, NULL, TO_ROOM );

  do_look( ch, "auto" );

  if (in_room == to_room) /* no circular follows */
    return;

  // Followers
  for ( fch = in_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;

    if ( fch->master == ch 
	 && IS_AFFECTED(fch,AFF_CHARM)
	 && fch->position < POS_STANDING)
      do_stand(fch,"");

    if ( fch->master == ch  // standing followers
	 && fch->position == POS_STANDING 
	 && can_see_room(fch,to_room))	{
      // Modified by SinaC 2001
      if (IS_SET(ch->in_room->cstat(flags),ROOM_LAW)
	  && (IS_NPC(fch) 
	      && IS_SET(fch->act,ACT_AGGRESSIVE))) {
	act("You can't bring $N into the city.",
	    ch,NULL,fch,TO_CHAR);
	act("You aren't allowed in the city.",
	    fch,NULL,NULL,TO_CHAR);
	continue;
      }

#ifdef VERBOSE
  log_stringf("MOVE: [%s] follows [%s] -> RECURSIVE CALL", NAME(fch), NAME(ch) );
#endif

      act( "You follow $N.", fch, NULL, ch, TO_CHAR );
      // Added by SinaC 2003 for climbing, pets and charmies climb if master can climb
      if ( ch->pet == fch || ( fch->master == ch && IS_NPC(fch) ) )
	move_char( fch, door, TRUE, climbing, FALSE ); // SinaC 2003
      else
	move_char( fch, door, TRUE, FALSE, FALSE ); // SinaC 2003
    }
    else // non followers
      if ( ch != fch  // SinaC 2003, people following group leader doesn't trigger
	   && fch->leader != ch
	   && fch->master != ch
	   && ch->leader != fch
	   && ch->master != fch ) {
#ifdef VERBOSE
  log_stringf("MOVE: onExited [%s] [%s]", NAME(fch), NAME(ch) );
#endif
	MOBPROG(fch,ch,"onExited", ch, short_dir_name[door] );
      }
  }

  // Added by SinaC 2003, for leader: trigger only when every followers have left
  for ( fch = in_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;
    if ( ch == fch 
	 || fch->leader == ch // SinaC 2003, people following group leader doesn't trigger
	 || fch->master == ch
	 || ch->leader == fch
	 || ch->master == fch )
      continue;
    if ( can_see(fch,ch) && !follow ) { // Leader
#ifdef VERBOSE
  log_stringf("MOVE: onExitedLeader [%s] [%s]", NAME(fch), NAME(ch) );
#endif
      MOBPROG(fch,ch,"onExitedLeader", ch, short_dir_name[door] );
    }
  }

  // Added by SinaC 2003 for room program
  //Value args3[] = {ch, short_dir_name[door]};
  //roomprog(in_room,ch,"onExited",args3);
#ifdef VERBOSE
  log_stringf("MOVE: onExited ROOM [%d] [%s]", in_room->vnum , NAME(ch) );
#endif
  ROOMPROG(in_room,ch,"onExited", ch, short_dir_name[door] );
  if ( !follow ) { // Leader
#ifdef VERBOSE
    log_stringf("MOVE: onExitedLeader ROOM [%d] [%s]", in_room->vnum , NAME(ch) );
#endif
    ROOMPROG(in_room,ch,"onExitedLeader", ch, short_dir_name[door] );
  }
  
  /* Greet trigger*/
  for ( fch = to_room->people; fch != NULL; fch = fch_next ) {
    /* This is quick hack... To be changed */
    fch_next = fch->next_in_room;

    if ( ch == fch
	 || fch->leader == ch
	 || fch->master == ch
	 || ch->leader == fch
	 || ch->master == fch ) // Added by SinaC 2003, doesn't need to test ch neither ch's followers
      continue;

    //if (can_see(fch,ch) && IS_NPC(fch)) {
    if (can_see(fch,ch)) { // onGreet is also for player
      //Value args[] = {ch, short_rev_dir_name[door]};
      //mobprog(fch,ch,"onGreet",args);
#ifdef VERBOSE
  log_stringf("MOVE: onGreet [%s] [%s]", NAME(fch), NAME(ch) );
#endif
      MOBPROG(fch,ch,"onGreet", ch, short_rev_dir_name[door] );
      if ( !follow ) { // Leader
#ifdef VERBOSE
	log_stringf("MOVE: onGreetLeader [%s] [%s]", NAME(fch), NAME(ch) );
#endif
	MOBPROG(fch,ch,"onGreetLeader", ch, short_rev_dir_name[door] );
      }
    }

    OBJ_DATA *obj, *obj_next;
    for ( obj = fch->carrying;
	  obj != NULL; 
	  obj = obj_next) {
      obj_next = obj->next_content;
#ifdef VERBOSE
      log_stringf("MOVE: onGreet OBJ [%d] [%s]", obj->pIndexData->vnum, NAME(ch) );
#endif
      OBJPROG(obj,fch,"onGreet", ch, short_rev_dir_name[door] );
      if ( !follow ) { // Leader
#ifdef VERBOSE
      log_stringf("MOVE: onGreetLeader OBJ [%d] [%s]", obj->pIndexData->vnum, NAME(ch) );
#endif
	OBJPROG(obj,fch,"onGreetLeader", ch, short_rev_dir_name[door] );
      }
    }
  }


  OBJ_DATA *obj, *obj_next;
  for ( obj = ch->carrying;
	obj != NULL; 
	obj = obj_next) {
    obj_next = obj->next_content;
    //OBJPROG(obj,ch,"onCarrierMoves", ch, short_rev_dir_name[door] );
#ifdef VERBOSE
      log_stringf("MOVE: onMoved OBJ [%d] [%s]", obj->pIndexData->vnum, NAME(ch) );
#endif
    OBJPROG(obj,ch,"onMoved", ch, short_rev_dir_name[door] );
  }

  // Added by SinaC 2003 for room program
  //Value args4[] = {ch, short_rev_dir_name[door]};
  //roomprog(to_room,ch,"onEntered",args4);
#ifdef VERBOSE
      log_stringf("MOVE: onEntered ROOM [%d] [%s]", to_room->vnum, NAME(ch) );
#endif
  ROOMPROG(to_room,ch,"onEntered", ch, short_rev_dir_name[door]);
  if ( !follow ) {
#ifdef VERBOSE
      log_stringf("MOVE: onEnteredLeader ROOM [%d] [%s]", to_room->vnum, NAME(ch) );
#endif
    ROOMPROG(to_room,ch,"onEnteredLeader", ch, short_rev_dir_name[door]);
  }

#ifdef VERBOSE
      log_stringf("MOVE: END OF MOVE: onMoved [%s]", NAME(ch) );
#endif
  // Added by SinaC 2003
  MOBPROG(ch,NULL,"onMoved");

  // Added by SinaC 2003 for blend skill
  if (is_affected(ch,gsn_blend) &&
      ( (ch->in_room->cstat(sector) == SECT_CITY)
	|| (ch->in_room->cstat(sector) == SECT_CITY)
	|| (ch->in_room->cstat(sector) == SECT_WATER_SWIM)
	|| (ch->in_room->cstat(sector) == SECT_WATER_NOSWIM)
	|| (ch->in_room->cstat(sector) == SECT_UNDERWATER)
	|| (ch->in_room->cstat(sector) == SECT_BURNING)
	|| (ch->in_room->cstat(sector) == SECT_AIR)) ) {
    send_to_char("You stop blending, the nature is less present here.\n\r",ch);
    affect_strip( ch, gsn_blend );
    //recompute( ch ); NO NEED: done in affect_strip
  }

  // Done in aggr_update now, SinaC 2003
//  // New aggressive code based on faction, SinaC 2003
//  // -> problem: the first person from the group matching will be attacked
//  //              this first person is the first from a group, leader excluded
//  for ( fch = to_room->people; fch != NULL; fch = fch_next ) {
//    fch_next = fch->next_in_room;
//    if ( ch == fch
//	 || fch->leader == ch
//	 || fch->master == ch
//	 || ch->leader == fch
//	 || ch->master == fch ) // Added by SinaC 2003, doesn't need to test ch neither ch's followers
//      continue;
//    if ( check_aggro_faction( fch, ch, FALSE ) ) // coming person is attacked by roomates?
//      multi_hit( fch, ch, TYPE_UNDEFINED );
//    else if ( check_aggro_faction( ch, fch, FALSE ) ) // coming person attack roomates?
//      multi_hit( ch, fch, TYPE_UNDEFINED );
//  }
  return;
}

// Added by SinaC 2003, for special exits: DIR_SPECIAL: dive
void do_dive( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_SPECIAL, FALSE, FALSE, FALSE ); // SinaC 2003
  if ( was_room == ch->in_room )
    free_runbuf(ch->desc);
  return;
}

// do_north, do_east, ... has been modified for 'jog' by SinaC 2000
void do_north( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_NORTH, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
  /*
    move_char( ch, DIR_NORTH, FALSE );
    return;
  */
}

void do_east( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_EAST, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
  /*
    move_char( ch, DIR_EAST, FALSE );
    return;
  */
}

void do_south( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_SOUTH, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
  /*
    move_char( ch, DIR_SOUTH, FALSE );
    return;
  */
}

void do_west( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_WEST, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
  /*
    move_char( ch, DIR_WEST, FALSE );
    return;
  */
}

void do_up( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_UP, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
  /*
    move_char( ch, DIR_UP, FALSE );
    return;
  */
}

void do_down( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_DOWN, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
  /*
    move_char( ch, DIR_DOWN, FALSE );
    return;
  */
}

void do_northeast( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_NORTHEAST, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
}

void do_northwest( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_NORTHWEST, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
}

void do_southeast( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_SOUTHEAST, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
}

void do_southwest( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_room;

  was_room = ch->in_room;
  move_char( ch, DIR_SOUTHWEST, FALSE, FALSE, FALSE ); // SinaC 2003 );
  if (was_room == ch->in_room)
    free_runbuf(ch->desc);
  return;
}

int find_door( CHAR_DATA *ch, const char *arg ) {
  EXIT_DATA *pexit;
  int door;

  if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) door = DIR_NORTH;
  else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) door = DIR_EAST;
  else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) door = DIR_SOUTH;
  else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) door = DIR_WEST;
  else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) door = DIR_UP;
  else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) door = DIR_DOWN;
  // Added by SinaC 2003
  else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast"  ) ) door = DIR_NORTHEAST;
  else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest"  ) ) door = DIR_NORTHWEST;
  else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast"  ) ) door = DIR_SOUTHEAST;
  else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest"  ) ) door = DIR_SOUTHWEST;

  else {
    for ( door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
      if ( ( pexit = ch->in_room->exit[door] ) != NULL
	   && IS_SET(pexit->exit_info, EX_ISDOOR)
	   && pexit->keyword != NULL
	   && is_name( arg, pexit->keyword ) )
	return door;
    }
    act( "I see no $T here.", ch, NULL, arg, TO_CHAR );
    return -1;
  }

  if ( ( pexit = ch->in_room->exit[door] ) == NULL ) {
    act( "I see no door $T here.", ch, NULL, arg, TO_CHAR );
    return -1;
  }

  if ( !IS_SET(pexit->exit_info, EX_ISDOOR) ) {
    send_to_char( "You can't do that.\n\r", ch );
    return -1;
  }

  return door;
}

void do_open( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Open what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) != NULL ) {
    /* open portal */
    if (obj->item_type == ITEM_PORTAL) {
      if (!IS_SET(obj->value[1], EX_ISDOOR)) {
	send_to_char("You can't do that.\n\r",ch);
	return;
      }

      if (!IS_SET(obj->value[1], EX_CLOSED)) {
	send_to_char("It's already open.\n\r",ch);
	return;
      }

      if (IS_SET(obj->value[1], EX_LOCKED)) {
	send_to_char("It's locked.\n\r",ch);
	return;
      }

      REMOVE_BIT(obj->value[1], EX_CLOSED);
      act("You open $p.",ch,obj,NULL,TO_CHAR);
      act("$n opens $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'open object' */
    if ( obj->item_type != ITEM_CONTAINER) { 
      send_to_char( "That's not a container.\n\r", ch ); 
      return; 
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) ) { 
      send_to_char( "It's already open.\n\r",      ch ); 
      return; 
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) ) { 
      send_to_char( "You can't do that.\n\r",      ch ); 
      return; 
    }
    if ( IS_SET(obj->value[1], CONT_LOCKED) ) { 
      send_to_char( "It's locked.\n\r",            ch ); 
      return; 
    }

    REMOVE_BIT(obj->value[1], CONT_CLOSED);
    act("You open $p.",ch,obj,NULL,TO_CHAR);
    act( "$n opens $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 ) {
    /* 'open door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) ) { 
      send_to_char( "It's already open.\n\r",      ch ); 
      return; 
    }
    if ( IS_SET(pexit->exit_info, EX_LOCKED) ) { 
      send_to_char( "It's locked.\n\r",            ch ); 
      return; 
    }
    
    // Added by SinaC 2001 for EX_INFURIATING
    if (IS_SET(pexit->exit_info, EX_INFURIATING ) ) {
      send_to_char( "The door is magically locked.\n\r", ch  );
      return;
    }

    REMOVE_BIT(pexit->exit_info, EX_CLOSED);
    act( "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );

    /* open the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
	 && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	 && pexit_rev->u1.to_room == ch->in_room ) {
      CHAR_DATA *rch;

      REMOVE_BIT( pexit_rev->exit_info, EX_CLOSED );
      for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	act( "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR );
    }
  }

  return;
}

void do_close( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Close what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) != NULL ) {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL) {

      if (!IS_SET(obj->value[1],EX_ISDOOR)
	  || IS_SET(obj->value[1],EX_NOCLOSE)) {
	send_to_char("You can't do that.\n\r",ch);
	return;
      }

      if (IS_SET(obj->value[1],EX_CLOSED)) {
	send_to_char("It's already closed.\n\r",ch);
	return;
      }

      SET_BIT(obj->value[1],EX_CLOSED);
      act("You close $p.",ch,obj,NULL,TO_CHAR);
      act("$n closes $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'close object' */
    if ( obj->item_type != ITEM_CONTAINER ) { 
      send_to_char( "That's not a container.\n\r", ch ); 
      return; 
    }
    if ( IS_SET(obj->value[1], CONT_CLOSED) ) { 
      send_to_char( "It's already closed.\n\r",    ch ); 
      return; 
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSEABLE) ) { 
      send_to_char( "You can't do that.\n\r",      ch ); 
      return; 
    }

    SET_BIT(obj->value[1], CONT_CLOSED);
    act("You close $p.",ch,obj,NULL,TO_CHAR);
    act( "$n closes $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 ) {
    /* 'close door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit	= ch->in_room->exit[door];
    if ( IS_SET(pexit->exit_info, EX_CLOSED) ) { 
      send_to_char( "It's already closed.\n\r",    ch ); 
      return; 
    }

    // Added by SinaC 2001 for EX_INFURIATING
    if (IS_SET(pexit->exit_info, EX_INFURIATING ) ) {
      send_to_char("It can't be closed.\n\r",ch);
      return;
    }

    // SinaC 2003 for bashed door
    if ( IS_SET(pexit->exit_info, EX_BASHED ) ) { 
      send_to_char("It has been bashed.\n\r", ch );
      return;
    }
    
    SET_BIT(pexit->exit_info, EX_CLOSED);
    act( "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM );
    send_to_char( "Ok.\n\r", ch );

    /* close the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
	 && ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	 && pexit_rev->u1.to_room == ch->in_room ) {
      CHAR_DATA *rch;

      SET_BIT( pexit_rev->exit_info, EX_CLOSED );
      for ( rch = to_room->people; rch != NULL; rch = rch->next_in_room )
	act( "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR );
    }
  }

  return;
}

// Modified by SinaC 2003
OBJ_DATA * has_key( CHAR_DATA *ch, int key ) {
  OBJ_DATA *obj;

  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj->pIndexData->vnum == key )
      return obj;
  }

  return NULL;
}

void do_lock( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Lock what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) != NULL ) {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL) {
      if (!IS_SET(obj->value[1],EX_ISDOOR)
	  || IS_SET(obj->value[1],EX_NOCLOSE)) {
	send_to_char("You can't do that.\n\r",ch);
	return;
      }
      if (!IS_SET(obj->value[1],EX_CLOSED)) {
	send_to_char("It's not closed.\n\r",ch);
	return;
      }

      if (obj->value[4] < 0 || IS_SET(obj->value[1],EX_NOLOCK)) {
	send_to_char("It can't be locked.\n\r",ch);
	return;
      }

      if ( has_key(ch,obj->value[4]) == NULL ) {
	send_to_char("You lack the key.\n\r",ch);
	return;
      }

      if (IS_SET(obj->value[1],EX_LOCKED)) {
	send_to_char("It's already locked.\n\r",ch);
	return;
      }

      SET_BIT(obj->value[1],EX_LOCKED);
      act("You lock $p.",ch,obj,NULL,TO_CHAR);
      act("$n locks $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'lock object' */
    if ( obj->item_type != ITEM_CONTAINER ) { 
      send_to_char( "That's not a container.\n\r", ch ); 
      return; 
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) ) { 
      send_to_char( "It's not closed.\n\r",        ch ); 
      return; 
    }
    if ( obj->value[2] < 0 ) { 
      send_to_char( "It can't be locked.\n\r",     ch ); 
      return; 
    }
    if ( has_key( ch, obj->value[2] ) == NULL ) { 
      send_to_char( "You lack the key.\n\r",       ch ); 
      return; 
    }
    if ( IS_SET(obj->value[1], CONT_LOCKED) ) { 
      send_to_char( "It's already locked.\n\r",    ch ); 
      return; 
    }

    SET_BIT(obj->value[1], CONT_LOCKED);
    act("You lock $p.",ch,obj,NULL,TO_CHAR);
    act( "$n locks $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 ) {
    /* 'lock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit	= ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) ) { 
      send_to_char( "It's not closed.\n\r",        ch ); 
      return; 
    }
    if ( pexit->key < 0 ) { 
      send_to_char( "It can't be locked.\n\r",     ch ); 
      return; 
    }

    // Added by SinaC 2001 for EX_INFURIATING
    if (IS_SET(pexit->exit_info, EX_INFURIATING ) ) {
      send_to_char("It can't be locked.\n\r",ch);
      return;
    }

    if ( has_key( ch, pexit->key) == NULL ) { 
      send_to_char( "You lack the key.\n\r",       ch ); 
      return; 
    }
    if ( IS_SET(pexit->exit_info, EX_LOCKED) ) { 
      send_to_char( "It's already locked.\n\r",    ch ); 
      return; 
    }

    SET_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    act( "$n locks the $d.", ch, NULL, pexit->keyword, TO_ROOM );

    /* lock the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
	 && ( pexit_rev = to_room->exit[rev_dir[door]] ) != 0
	 && pexit_rev->u1.to_room == ch->in_room ) {
      SET_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
  }

  return;
}



void do_unlock( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Unlock what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_here( ch, arg ) ) != NULL ) {
    /* portal stuff */
    if (obj->item_type == ITEM_PORTAL) {
      if (!IS_SET(obj->value[1],EX_ISDOOR)) {
	send_to_char("You can't do that.\n\r",ch);
	return;
      }

      if (!IS_SET(obj->value[1],EX_CLOSED)) {
	send_to_char("It's not closed.\n\r",ch);
	return;
      }

      if (obj->value[4] < 0) {
	send_to_char("It can't be unlocked.\n\r",ch);
	return;
      }

      if ( has_key(ch,obj->value[4]) == NULL ) {
	send_to_char("You lack the key.\n\r",ch);
	return;
      }

      if (!IS_SET(obj->value[1],EX_LOCKED)) {
	send_to_char("It's already unlocked.\n\r",ch);
	return;
      }

      REMOVE_BIT(obj->value[1],EX_LOCKED);
      act("You unlock $p.",ch,obj,NULL,TO_CHAR);
      act("$n unlocks $p.",ch,obj,NULL,TO_ROOM);
      return;
    }

    /* 'unlock object' */
    if ( obj->item_type != ITEM_CONTAINER ) { 
      send_to_char( "That's not a container.\n\r", ch ); 
      return; 
    }
    if ( !IS_SET(obj->value[1], CONT_CLOSED) ) { 
      send_to_char( "It's not closed.\n\r",        ch ); 
      return; 
    }
    if ( obj->value[2] < 0 ) { 
      send_to_char( "It can't be unlocked.\n\r",   ch ); 
      return; 
    }
    if ( has_key( ch, obj->value[2] ) == NULL ) { 
      send_to_char( "You lack the key.\n\r",       ch ); 
      return; 
    }
    if ( !IS_SET(obj->value[1], CONT_LOCKED) ) { 
      send_to_char( "It's already unlocked.\n\r",  ch ); 
      return; 
    }

    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    act("You unlock $p.",ch,obj,NULL,TO_CHAR);
    act( "$n unlocks $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 ) {
    /* 'unlock door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) ) { 
      send_to_char( "It's not closed.\n\r",        ch ); 
      return; 
    }
    if ( pexit->key < 0 ) { 
      send_to_char( "It can't be unlocked.\n\r",   ch ); 
      return; 
    }

    // Added by SinaC 2001 for EX_INFURIATING
    if (IS_SET(pexit->exit_info, EX_INFURIATING ) ) {
      send_to_char("It can't be unlocked.\n\r",ch);
      return;
    }

    OBJ_DATA *key;
    if ( ( key = has_key( ch, pexit->key) ) == NULL ) { 
      send_to_char( "You lack the key.\n\r",       ch ); 
      return; 
    }
    if ( !IS_SET(pexit->exit_info, EX_LOCKED) ) { 
      send_to_char( "It's already unlocked.\n\r",  ch ); 
      return; 
    }

    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    act( "$n unlocks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
    // Added by SinaC 2003
    if ( IS_SET(pexit->exit_info, EX_EATKEY ) ) {
      act("$p vanishes.", ch, obj, NULL, TO_CHAR );
      act("$p vanishes.", ch, obj, NULL, TO_ROOM );
    }

    /* unlock the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
	 && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	 && pexit_rev->u1.to_room == ch->in_room ) {
      REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
  }
  return;
}

void do_stand( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj = NULL;

  if (argument[0] != '\0') {
    if (ch->position == POS_FIGHTING) {
      send_to_char("Maybe you should finish fighting first?\n\r",ch);
      return;
    }
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL) {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
    if (obj->item_type != ITEM_FURNITURE
	|| (!IS_SET(obj->value[2],STAND_AT)
	     && !IS_SET(obj->value[2],STAND_ON)
	     && !IS_SET(obj->value[2],STAND_IN))) {
      send_to_char("You can't seem to find a place to stand.\n\r",ch);
      return;
    }
    if ( ch->on != obj 
	 && count_users(obj) >= obj->value[0])	{
      act_new("There's no room to stand on $p.",
	      ch,obj,NULL,TO_CHAR,POS_DEAD);
      return;
    }
    ch->on = obj;
  }
    
  switch ( ch->position ) {
  case POS_SLEEPING:
    if ( IS_AFFECTED(ch, AFF_SLEEP) ) { 
      send_to_char( "You can't wake up!\n\r", ch ); 
      return; 
    }
	
    if (obj == NULL) {
      send_to_char( "You wake and stand up.\n\r", ch );
      act( "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM );
      ch->on = NULL;
    }
    else if (IS_SET(obj->value[2],STAND_AT)) {
      act_new("You wake and stand at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      act("$n wakes and stands at $p.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],STAND_ON)) {
      act_new("You wake and stand on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      act("$n wakes and stands on $p.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act_new("You wake and stand in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      act("$n wakes and stands in $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_STANDING;
    do_look(ch,"auto");
    break;

  case POS_RESTING: case POS_SITTING:
    if (obj == NULL) {
      send_to_char( "You stand up.\n\r", ch );
      act( "$n stands up.", ch, NULL, NULL, TO_ROOM );
      ch->on = NULL;
    }
    else if (IS_SET(obj->value[2],STAND_AT)) {
      act("You stand at $p.",ch,obj,NULL,TO_CHAR);
      act("$n stands at $p.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],STAND_ON)) {
      act("You stand on $p.",ch,obj,NULL,TO_CHAR);
      act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act("You stand in $p.",ch,obj,NULL,TO_CHAR);
      act("$n stands on $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_STANDING;
    break;

  case POS_STANDING:
    send_to_char( "You are already standing.\n\r", ch );
    break;

  case POS_FIGHTING:
    send_to_char( "You are already fighting!\n\r", ch );
    break;
  }

  // Added by SinaC 2003
  CHAR_DATA *vch, *vch_next = NULL;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    MOBPROG( vch, ch, "onStand", ch );
  }

  return;
}



void do_rest( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj = NULL;

  if (ch->position == POS_FIGHTING) {
    send_to_char("You are already fighting!\n\r",ch);
    return;
  }

  /* okay, now that we know we can rest, find an object to rest on */
  if (argument[0] != '\0') {
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL) {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else obj = ch->on;

  if (obj != NULL) {
    if (!IS_SET(obj->item_type,ITEM_FURNITURE) 
    	|| (!IS_SET(obj->value[2],REST_ON)
	    && !IS_SET(obj->value[2],REST_IN)
	    && !IS_SET(obj->value[2],REST_AT))) {
      send_to_char("You can't rest on that.\n\r",ch);
      return;
    }

    if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0]) {
      act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      return;
    }
	
    ch->on = obj;
  }

  switch ( ch->position ) {
  case POS_SLEEPING:
    if (IS_AFFECTED(ch,AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r",ch);
      return;
    }

    if (obj == NULL) {
      send_to_char( "You wake up and start resting.\n\r", ch );
      act ("$n wakes up and starts resting.",ch,NULL,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],REST_AT))	{
      act_new("You wake up and rest at $p.",
	      ch,obj,NULL,TO_CHAR,POS_SLEEPING);
      act("$n wakes up and rests at $p.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],REST_ON)) {
      act_new("You wake up and rest on $p.",
	      ch,obj,NULL,TO_CHAR,POS_SLEEPING);
      act("$n wakes up and rests on $p.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act_new("You wake up and rest in $p.",
	      ch,obj,NULL,TO_CHAR,POS_SLEEPING);
      act("$n wakes up and rests in $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_RESTING;
    break;

  case POS_RESTING:
    send_to_char( "You are already resting.\n\r", ch );
    break;

  case POS_STANDING:
    if (obj == NULL) {
      send_to_char( "You rest.\n\r", ch );
      act( "$n sits down and rests.", ch, NULL, NULL, TO_ROOM );
    }
    else if (IS_SET(obj->value[2],REST_AT)) {
      act("You sit down at $p and rest.",ch,obj,NULL,TO_CHAR);
      act("$n sits down at $p and rests.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],REST_ON)) {
      act("You sit on $p and rest.",ch,obj,NULL,TO_CHAR);
      act("$n sits on $p and rests.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act("You rest in $p.",ch,obj,NULL,TO_CHAR);
      act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_RESTING;
    break;

  case POS_SITTING:
    if (obj == NULL) {
      send_to_char("You rest.\n\r",ch);
      act("$n rests.",ch,NULL,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],REST_AT)) {
      act("You rest at $p.",ch,obj,NULL,TO_CHAR);
      act("$n rests at $p.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],REST_ON)) {
      act("You rest on $p.",ch,obj,NULL,TO_CHAR);
      act("$n rests on $p.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act("You rest in $p.",ch,obj,NULL,TO_CHAR);
      act("$n rests in $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_RESTING;
    break;
  }

  // Added by SinaC 2003
  CHAR_DATA *vch, *vch_next = NULL;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    MOBPROG( vch, ch, "onRest", ch );
  }

  return;
}


void do_sit (CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj = NULL;

  if (ch->position == POS_FIGHTING) {
    send_to_char("Maybe you should finish this fight first?\n\r",ch);
    return;
  }

  /* okay, now that we know we can sit, find an object to sit on */
  if (argument[0] != '\0') {
    obj = get_obj_list(ch,argument,ch->in_room->contents);
    if (obj == NULL) {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
  }
  else obj = ch->on;

  if (obj != NULL)  {
    if (!IS_SET(obj->item_type,ITEM_FURNITURE)
	|| (!IS_SET(obj->value[2],SIT_ON)
	    && !IS_SET(obj->value[2],SIT_IN)
	    && !IS_SET(obj->value[2],SIT_AT))) {
      send_to_char("You can't sit on that.\n\r",ch);
      return;
    }

    if (obj != NULL && ch->on != obj && count_users(obj) >= obj->value[0]) {
      act_new("There's no more room on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      return;
    }

    ch->on = obj;
  }
  switch (ch->position) {
  case POS_SLEEPING:
    if (IS_AFFECTED(ch,AFF_SLEEP)) {
      send_to_char("You can't wake up!\n\r",ch);
      return;
    }

    if (obj == NULL) {
      send_to_char( "You wake and sit up.\n\r", ch );
      act( "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM );
    }
    else if (IS_SET(obj->value[2],SIT_AT)) {
      act_new("You wake and sit at $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],SIT_ON)) {
      act_new("You wake and sit on $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      act("$n wakes and sits at $p.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act_new("You wake and sit in $p.",ch,obj,NULL,TO_CHAR,POS_DEAD);
      act("$n wakes and sits in $p.",ch,obj,NULL,TO_ROOM);
    }

    ch->position = POS_SITTING;
    break;
  case POS_RESTING:
    if (obj == NULL)
      send_to_char("You stop resting.\n\r",ch);
    else if (IS_SET(obj->value[2],SIT_AT)) {
      act("You sit at $p.",ch,obj,NULL,TO_CHAR);
      act("$n sits at $p.",ch,obj,NULL,TO_ROOM);
    }

    else if (IS_SET(obj->value[2],SIT_ON)) {
      act("You sit on $p.",ch,obj,NULL,TO_CHAR);
      act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_SITTING;
    break;
  case POS_SITTING:
    send_to_char("You are already sitting down.\n\r",ch);
    break;
  case POS_STANDING:
    if (obj == NULL) {
      send_to_char("You sit down.\n\r",ch);
      act("$n sits down on the ground.",ch,NULL,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],SIT_AT)) {
      act("You sit down at $p.",ch,obj,NULL,TO_CHAR);
      act("$n sits down at $p.",ch,obj,NULL,TO_ROOM);
    }
    else if (IS_SET(obj->value[2],SIT_ON)) {
      act("You sit on $p.",ch,obj,NULL,TO_CHAR);
      act("$n sits on $p.",ch,obj,NULL,TO_ROOM);
    }
    else {
      act("You sit down in $p.",ch,obj,NULL,TO_CHAR);
      act("$n sits down in $p.",ch,obj,NULL,TO_ROOM);
    }
    ch->position = POS_SITTING;
    break;
  }

  // Added by SinaC 2003
  CHAR_DATA *vch, *vch_next = NULL;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    MOBPROG( vch, ch, "onSit", ch );
  }
  return;
}


void do_sleep( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj = NULL;

  switch ( ch->position ) {
  case POS_SLEEPING:
    send_to_char( "You are already sleeping.\n\r", ch );
    break;

  case POS_RESTING:
  case POS_SITTING:
  case POS_STANDING: 
    if (argument[0] == '\0' && ch->on == NULL) {
      send_to_char( "You go to sleep.\n\r", ch );
      act( "$n goes to sleep.", ch, NULL, NULL, TO_ROOM );
      ch->position = POS_SLEEPING;
    }
    else  {/* find an object and sleep on it */
      if (argument[0] == '\0')
	obj = ch->on;
      else
	obj = get_obj_list( ch, argument,  ch->in_room->contents );

      if (obj == NULL) {
	send_to_char("You don't see that here.\n\r",ch);
	return;
      }
      if (obj->item_type != ITEM_FURNITURE
	  || (!IS_SET(obj->value[2],SLEEP_ON) 
	      && !IS_SET(obj->value[2],SLEEP_IN)
	      && !IS_SET(obj->value[2],SLEEP_AT))) {
	send_to_char("You can't sleep on that!\n\r",ch);
	return;
      }

      if (ch->on != obj && count_users(obj) >= obj->value[0]) {
	act_new("There is no room on $p for you.",
		ch,obj,NULL,TO_CHAR,POS_DEAD);
	return;
      }

      ch->on = obj;
      if (IS_SET(obj->value[2],SLEEP_AT)) {
	act("You go to sleep at $p.",ch,obj,NULL,TO_CHAR);
	act("$n goes to sleep at $p.",ch,obj,NULL,TO_ROOM);
      }
      else if (IS_SET(obj->value[2],SLEEP_ON)) {
	act("You go to sleep on $p.",ch,obj,NULL,TO_CHAR);
	act("$n goes to sleep on $p.",ch,obj,NULL,TO_ROOM);
      }
      else {
	act("You go to sleep in $p.",ch,obj,NULL,TO_CHAR);
	act("$n goes to sleep in $p.",ch,obj,NULL,TO_ROOM);
      }
      ch->position = POS_SLEEPING;
    }
    break;

  case POS_FIGHTING:
    send_to_char( "You are already fighting!\n\r", ch );
    break;
  }

  // Added by SinaC 2003
  CHAR_DATA *vch, *vch_next = NULL;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    MOBPROG( vch, ch, "onSleep", ch );
  }

  return;
}



void do_wake( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) { 
    do_stand( ch, argument ); 
    return; 
  }

  if ( !IS_AWAKE(ch) ) { 
    send_to_char( "You are asleep yourself!\n\r",       ch ); 
    return; 
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) { 
    send_to_char( "They aren't here.\n\r",              ch ); 
    return; 
  }

  if ( IS_AWAKE(victim) ) { 
    act( "$N is already awake.", ch, NULL, victim, TO_CHAR ); 
    return; 
  }

  if ( IS_AFFECTED(victim, AFF_SLEEP) ) { 
    act( "You can't wake $M!",   ch, NULL, victim, TO_CHAR );  
    return; 
  }

  act_new( "$n wakes you.", ch, NULL, victim, TO_VICT,POS_SLEEPING );
  do_stand(victim,"");
  return;
}


/*
 * Contributed by Alander.
 */
void do_visible( CHAR_DATA *ch, const char *argument ) {
  // Added by SinaC 2001
  affect_strip ( ch, gsn_invisible		);

  affect_strip ( ch, gsn_invis			);
  affect_strip ( ch, gsn_mass_invis		);
  affect_strip ( ch, gsn_sneak			);
  REMOVE_BIT   ( ch->cstat(affected_by), AFF_HIDE		);
  REMOVE_BIT   ( ch->cstat(affected_by), AFF_INVISIBLE	);
  REMOVE_BIT   ( ch->cstat(affected_by), AFF_SNEAK		);
  send_to_char( "You are now visible.\n\r", ch );
  recompute(ch);
  return;
}

// SinaC 2003
ROOM_INDEX_DATA *get_recall_room( CHAR_DATA *ch ) {
  if ( IS_NPC(ch) // if mob
       || ch->pcdata->hometown < 0 ) // or no hometown available
    return get_room_index( DEFAULT_RECALL );

  if ( ch->clan )
    return get_room_index( get_clan_table(ch->clan)->hall ); 
  else
    return get_room_index( hometown_table[ch->pcdata->hometown].recall );
}

ROOM_INDEX_DATA *get_hall_room( CHAR_DATA *ch ) {
  if ( IS_NPC(ch) // if mob
       || ch->pcdata->hometown < 0 ) // or no hometown available
    return get_room_index( DEFAULT_HALL );

  if ( ch->clan )
    return get_room_index( get_clan_table(ch->clan)->hall ); 
  else
    return get_room_index( hometown_table[ch->pcdata->hometown].hall );
}

ROOM_INDEX_DATA *get_morgue_room( CHAR_DATA *ch ) {
  if ( IS_NPC(ch) )
    return get_room_index( DEFAULT_MORGUE );
  if ( ch->pcdata->hometown < 0 || ch->pcdata->hometown >= MAX_HOMETOWN )
    return get_room_index( DEFAULT_MORGUE );
  if ( ch->clan )
    return get_room_index( get_clan_table(ch->clan)->morgue ); 
  else
    return get_room_index( hometown_table[ch->pcdata->hometown].morgue );
}

ROOM_INDEX_DATA *get_donation_room( CHAR_DATA *ch ) {
  if ( IS_NPC(ch) )
    return get_room_index( DEFAULT_DONATION );
  if ( ch->pcdata->hometown < 0 || ch->pcdata->hometown >= MAX_HOMETOWN )
    return get_room_index( DEFAULT_DONATION );
  if ( ch->clan )
    return get_room_index( get_clan_table(ch->clan)->donation ); 
  else
    return get_room_index( hometown_table[ch->pcdata->hometown].donation );
}

ROOM_INDEX_DATA *get_school_room( CHAR_DATA *ch ) {
  if ( IS_NPC(ch) )
    return get_room_index( DEFAULT_SCHOOL );
  if ( ch->pcdata->hometown < 0 || ch->pcdata->hometown >= MAX_HOMETOWN )
    return get_room_index( DEFAULT_SCHOOL );
  return get_room_index( hometown_table[ch->pcdata->hometown].school );
}

void do_hometown(CHAR_DATA *ch, const char *argument) {
  int town;

  if (IS_NPC(ch)) 
    return;
  
  for ( town = 0; hometown_table[town].recall; town++)
    if ( hometown_table[town].recall == ch->in_room->vnum)
      break;
  if ( town < MAX_HOMETOWN && hometown_table[town].recall ) {
    ch->pcdata->hometown = town;
    send_to_charf(ch,"Your hometown is now %s.\n\r", hometown_table[town].name);
  } 
  else {
    // Modified by SinaC 2001 for god
    //send_to_char("Mota won't transport you back here.\n\r",ch);    
    send_to_charf( ch, 
		   "%s won't transport you back here.\n\r",
		   god_name(ch->pcdata->god));
  }
}


void do_train( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *mob;
  int stat = - 1;
  const char *pOutput = NULL;
  int cost;

  if ( IS_NPC(ch) )
    return;

  /*
   * Check for trainer.
   */
  for ( mob = ch->in_room->people; mob; mob = mob->next_in_room ) {
    if ( IS_NPC(mob) && IS_SET(mob->act, ACT_TRAIN) )
      break;
  }

  if ( mob == NULL ) {
    send_to_char( "You can't do that here.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    sprintf( buf, "You have %d training sessions.\n\r", ch->train );
    send_to_char( buf, ch );
    argument = "foo";
  }

  cost = 1;

  if ( !str_cmp( argument, "str" ) ) {
    /* Oxtal> Tsk! The following code is totally useless... since anyway cost = 1*/
    if ( class_hasattr(ch,STAT_STR) )
      cost    = 1;
    stat        = STAT_STR;
    pOutput     = "strength";
  }

  else if ( !str_cmp( argument, "int" ) ) {
    if ( class_hasattr(ch,STAT_INT) )
      cost    = 1;
    stat	    = STAT_INT;
    pOutput     = "intelligence";
  }

  else if ( !str_cmp( argument, "wis" ) ) {
    if ( class_hasattr(ch,STAT_WIS) )
      cost    = 1;
    stat	    = STAT_WIS;
    pOutput     = "wisdom";
  }

  else if ( !str_cmp( argument, "dex" ) ) {
    if ( class_hasattr(ch,STAT_DEX) )
      cost    = 1;
    stat  	    = STAT_DEX;
    pOutput     = "dexterity";
  }

  else if ( !str_cmp( argument, "con" ) ) {
    if ( class_hasattr(ch,STAT_CON) )
      cost    = 1;
    stat	    = STAT_CON;
    pOutput     = "constitution";
  }

  else if ( !str_cmp(argument, "hp" ) )
    cost = 1;

  else if ( !str_cmp(argument, "mana" ) )
    cost = 1;
  // Added by SinaC 2001 for mental user
  else if ( !str_cmp(argument, "psp" ) )
    cost = 1;

  else  {
    strcpy( buf, "You can train:" );
    if ( ch->bstat(STR) < get_max_train(ch,STAT_STR))
      strcat( buf, " str" );
    if ( ch->bstat(INT) < get_max_train(ch,STAT_INT))
      strcat( buf, " int" );
    if ( ch->bstat(WIS) < get_max_train(ch,STAT_WIS))
      strcat( buf, " wis" );
    if ( ch->bstat(DEX) < get_max_train(ch,STAT_DEX))
      strcat( buf, " dex" );
    if ( ch->bstat(CON) < get_max_train(ch,STAT_CON))  
      strcat( buf, " con" );
    // Modified by SinaC 2001 for mental user
    //strcat( buf, " hp mana");
    strcat( buf, " hp mana psp");

    if ( buf[strlen(buf)-1] != ':' ) {
      strcat( buf, ".\n\r" );
      send_to_char( buf, ch );
    }
    else {
      /*
       * This message dedicated to Jordan ... you big stud!
       */
      act( "You have nothing left to train, you $T!",
	   ch, NULL,
	   ch->cstat(sex) == SEX_MALE   ? "big stud" :
	   ch->cstat(sex) == SEX_FEMALE ? "hot babe" :
	   "wild thing",
	   TO_CHAR );
    }

    return;
  }

  if (!str_cmp("hp",argument)) {
    if ( cost > ch->train )	{
      send_to_char( "You don't have enough training sessions.\n\r", ch );
      return;
    }

    ch->train -= cost;
    ch->bstat(max_hit) += 10;
    ch->hit +=10;
    recompute(ch);
    act( "Your durability increases!",ch,NULL,NULL,TO_CHAR);
    act( "$n's durability increases!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  if (!str_cmp("mana",argument)) {
    if ( cost > ch->train ) {
      send_to_char( "You don't have enough training sessions.\n\r", ch );
      return;
    }

    ch->train -= cost;
    ch->bstat(max_mana) += 10;
    ch->mana += 10;
    recompute(ch);
    // Modified by SinaC 2001 for mental user
    act( "Your mana power increases!",ch,NULL,NULL,TO_CHAR);
    act( "$n's mana power increases!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  // Added by SinaC 2001 for mental user
  if (!str_cmp("psp",argument)) {
    if ( cost > ch->train ) {
      send_to_char( "You don't have enough training sessions.\n\r", ch );
      return;
    }

    ch->train -= cost;
    ch->bstat(max_psp) += 10;
    ch->psp += 10;
    recompute(ch);
    act( "Your mental power increases!",ch,NULL,NULL,TO_CHAR);
    act( "$n's mental power increases!",ch,NULL,NULL,TO_ROOM);
    return;
  }

  if ( ch->bstat(stat0+stat)  >= get_max_train(ch,stat) ) {
    act( "Your $T is already at maximum.", ch, NULL, pOutput, TO_CHAR );
    return;
  }

  if ( cost > ch->train ) {
    send_to_char( "You don't have enough training sessions.\n\r", ch );
    return;
  }

  ch->train -= cost;

  ch->bstat(stat0+stat) += 1;
  recompute(ch);
  act( "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
  act( "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
  return;
}

// Added by SinaC 2000
/**************************************************************************/

/*
 *  This function allows a character to knock on a door.
*/

void do_knock(CHAR_DATA *ch, const char *argument) {
  /* Constructs taken from do_open().  */
  int door;
  char arg[MAX_INPUT_LENGTH];

  one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Knock on what?\n\r",ch);
    return;
  }

  if ( ( door = find_door( ch, arg ) ) >= 0 ) {
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;

    pexit = ch->in_room->exit[door];

    act( "$n knocks on the $d.", ch, NULL, pexit->keyword, TO_ROOM );
    act( "You knock on the $d.", ch, NULL, pexit->keyword, TO_CHAR );

    /* Notify the other side.  */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
	 && ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	 && pexit_rev->u1.to_room == ch->in_room ) {
      CHAR_DATA *rch, *rch_next;
      for ( rch = to_room->people; rch != NULL; rch = rch_next ) {
	rch_next = rch->next_in_room;
	if ( rch == ch )
	  continue;
	act( "You hear someone knocking on the door.", rch, NULL, pexit_rev->keyword, TO_CHAR );
	Value args[] = {ch, short_rev_dir_name[door]};

	mobprog(rch,ch,"onKnock",args);
      }
    }
  }
  
  return;
}

/* 
   This code was written by Jaey of Crimson Souls MUD.  Any
   credit given, while appreciated, is not necessary.  Please
   contact me with any problems.

   Contact info:

       E-mail:  scarrico@mail.transy.edu
          Web:  http://www.transy.edu/~steven
Crimson Souls:  ns2.ka.net 6969
*/
void do_jog( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_INPUT_LENGTH],arg[MAX_INPUT_LENGTH];
  char *p;
  bool dFound = FALSE;
  
  if (!ch->desc || *argument == '\0') {
    send_to_char("You run in place!\n\r",ch);
    return;
  }
  
  buf[0] = '\0';
  
  while (*argument != '\0') {
    argument = no_lower_one_argument(argument,arg);
    strcat(buf,arg);
  }
  
  for( p = buf + strlen(buf)-1; p >= buf; p--) {
    if (!isdigit(*p)) {
      switch( *p ) {
      case 'n':
      case 's':
      case 'e':
      case 'w':
      case 'u':
      case 'd': dFound = TRUE;
	break;
	
	// Added by SinaC 2003, 4 new directions: NE, NW, SE, SW
      case 'E':
      case 'W':
	p--;
	if ( *p == 'S' || *p == 'N' ) dFound = TRUE;
	else {
	  send_to_char("Invalid direction!\n\r",ch);
	  return;
	}
	break;
	      
      case 'o': break;
	      
      default: send_to_char("Invalid direction!\n\r",ch);
	return;
      }
    }
    else if (!dFound) *p = '\0';
  }

  if (!dFound) {
    send_to_char("No directions specified!\n\r",ch); 
    return;
  }
  
  ch->desc->run_buf = str_dup_unsafe( buf ); // FIXME
  ch->desc->run_head = ch->desc->run_buf;
  send_to_char("You start running...\n\r",ch);
  return;
}
