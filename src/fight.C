/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik Strfeldt, Tom Madsen, and Katja Nyboe.    *
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
****************************************************************************/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "classes.h"
#include "tables.h"
#include "magic.h"
#include "spells_def.h"
#include "interp.h"

#include "condition.h" // Added by SinaC 2001
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "hunt.h"
#include "fight.h"
#include "update.h"
#include "special.h"
#include "act_move.h"
#include "effects.h"
#include "act_comm.h"
#include "save.h"
#include "lookup.h"
#include "gsn.h"
#include "olc_value.h"
#include "combatabilities.h"
#include "noncombatabilities.h"
#include "power.h"
#include "const.h"
#include "clan.h"
#include "wiznet.h"
#include "raceabilities.h"
#include "act_obj.h"
#include "faction.h"
#include "ability.h"
#include "skills.h"
#include "song.h"
#include "bit.h"
#include "utils.h"
#include "arena.h"
#include "damage.h"
#include "recycle.h"


//#define VERBOSE
//#define VERBOSE2


/*
 * Local functions.
 */
void	check_assist	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_parry	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_block     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//void	death_cry	args( ( CHAR_DATA *ch ) );
 // SinaC 2003, return which part has been cut off
long	death_cry	args( ( CHAR_DATA *ch ) );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int	xp_compute	args( ( CHAR_DATA *gch, CHAR_DATA *victim, 
				int total_levels ) );
bool	is_safe		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
// Modified by SinaC 2000 to return the corpse
//void	make_corpse	args( ( CHAR_DATA *ch ) );
//void	make_corpse	args( ( CHAR_DATA *ch, OBJ_DATA *&corpse ) );
// Modified by SinaC 2003
//void	make_corpse	args( ( CHAR_DATA *ch, OBJ_DATA *&corpse, CHAR_DATA *killer ) );
void	make_corpse	args( ( CHAR_DATA *ch, OBJ_DATA *&corpse, CHAR_DATA *killer, const long parts ) );

void	one_hit	        args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int whichWield ) );
void    mob_hit		args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
// Modified by SinaC 2000
void	raw_kill	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//void	disarm		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

// Added by SinaC 2000 to allow mobiles to switch between different targets
void    switch_update   args( (CHAR_DATA *ch) );
// Other addition by SinaC 2000, Modified by SinaC 2003  (bool secondary)
bool    check_fade      args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
//bool    check_counter   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, bool secondary ) );
// Modified by SinaC 2003
bool    check_counter   args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int whichWield ) );
//bool    check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_critical  args( ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield ) );
void	check_spirit	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
int     check_blindfight args( ( CHAR_DATA *ch, CHAR_DATA *victim, 
				 int notblind, int blind, int fail ) );

// Added by SinaC 2003
bool	check_dual_parry args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_evasion    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_shield_break args( ( CHAR_DATA *victim, CHAR_DATA *ch, OBJ_DATA *weapon ) );
bool    check_block      args( ( CHAR_DATA *ch, CHAR_DATA *victim, const int dam, int &dam_modifier ) );
bool    check_tumble     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_deflect    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

bool    check_lycanthropy args( ( CHAR_DATA *ch, CHAR_DATA *victim, const int dt, OBJ_DATA *wield ) );

// Added by SinaC 2001 for mob skills/spells
void mob_use_skill_spell args( (CHAR_DATA *ch, CHAR_DATA *victim, int sn ) );


//int glob_whichWield; not needed anymore


/*
 *  CH kills VICTIM in one hit, used by vorpal weapon and maybe some spells
 *   can't kill an immortal in one hit ;)
 */
void sudden_death( CHAR_DATA *ch, CHAR_DATA *victim )
{
  int explost;
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *corpse;

  if ( IS_IMMORTAL( victim ) ) 
    return;

  // Added by SinaC 2003 for phylactery
  // When affected by phylactery, victim has a chance to cheat death and get some hp back
  if ( check_phylactery( victim ) )
    return;

  // Added by SinaC 2003, when a player sudden_death him/herself we call silent_kill
  if ( victim == ch ) {
    static char * const him_her_self[] = { "itself", "himself", "herself" };
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "kills %s!", 
	     him_her_self[URANGE(0,ch->cstat(sex),2)] );
    silent_kill( victim, buf );
    return;
  }

  victim->hit = 1;

  // Modified by SinaC 2001, battle check
  if ( ch != victim && !IN_BATTLE(victim) && !IN_BATTLE(ch))
    group_gain( ch, victim );
  
  // Added by SinaC 2000, so the hunter stops hunting his prey when his prey is
  //  dead
  remove_hunter( victim );
  
  if ( !IS_NPC(victim) ) {
    sprintf( log_buf, "%s killed by %s at %d",
	     victim->name,
	     (IS_NPC(ch) ? ch->short_descr : ch->name),
	     ch->in_room->vnum );
    log_string( log_buf );

    /*
     * Dying penalty:
     * 2/3 way back to previous level.
     */
    if ( !IN_BATTLE(victim)) {
      if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
	   * victim->level ) {	        
	explost = UMIN( 0, (2 * (exp_per_level(victim,victim->pcdata->points)
				 * victim->level - victim->exp)/3) + 50 );
	sprintf( buf, "You lose %d xp.\n\r", -explost );
	send_to_char( buf, victim );	
	gain_exp( victim, explost, TRUE );
      }
      // added by SinaC 2000
      else
	send_to_char( "You don't lose any xp.\n\r", victim );
    }
      
    sprintf( buf, "{rINFO: %s has been killed by %s{x\n\r", victim->name, 
	     ( IS_NPC( ch ) ? ch->short_descr : ch->name ) );
    info( buf );         
  }
  
  sprintf( log_buf, "%s got toasted by %s at %s [room %d]",
	   (IS_NPC(victim) ? victim->short_descr : victim->name),
	   (IS_NPC(ch) ? ch->short_descr : ch->name),
	   ch->in_room->name, ch->in_room->vnum);
  
  if (IS_NPC(victim))
    wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
  else
    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
  
  raw_kill( ch, victim );
  /* dump the flags */
  if ( IS_PC(ch) ?
       ch != victim
       && !is_same_clan(ch,victim)
       && !IN_BATTLE(ch)
       :
       ch->spec_fun == spec_executioner /* Added by Oxtal */ ) {
    if (IS_SET(victim->act,PLR_KILLER))
      REMOVE_BIT(victim->act,PLR_KILLER);
    else
      REMOVE_BIT(victim->act,PLR_THIEF);
  }
  
  /* RT new auto commands */
  
  if (!IS_NPC(ch)
      &&  (corpse = get_obj_list(ch,"corpse",ch->in_room->contents)) != NULL
      &&  corpse->item_type == ITEM_CORPSE_NPC && can_see_obj(ch,corpse)) {
    OBJ_DATA *coins;
    
    corpse = get_obj_list( ch, "corpse", ch->in_room->contents ); 
    
    if ( IS_SET(ch->act, PLR_AUTOLOOT) &&
	 corpse && corpse->contains) /* exists and not empty */
      do_get( ch, "all corpse" );
    
    if (IS_SET(ch->act,PLR_AUTOGOLD) &&
	corpse && corpse->contains  && /* exists and not empty */
	!IS_SET(ch->act,PLR_AUTOLOOT))
      if ((coins = get_obj_list(ch,"gcash",corpse->contains))
	  != NULL)
	do_get(ch, "all.gcash corpse");
    
    if ( IS_SET(ch->act, PLR_AUTOSAC) )
      if ( IS_SET(ch->act,PLR_AUTOLOOT) && corpse && corpse->contains)
	return;  /* leave if corpse has treasure */
      else
	do_sacrifice( ch, "corpse" );
  }
  return;
}

// SinaC 2003: mob affected by AFF2_CONFUSION attack random target even non-fighting target
void confusion_switch( CHAR_DATA *ch ) {
  // First we count the number of valid target
  int count = 0;
  CHAR_DATA *vch_next;
  for ( CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( vch != ch->fighting // not the person ch is fighting
	 && vch != ch // not himself
	 && !( IS_NPC(vch) && IS_SET(vch->act, ACT_IS_SAFE ) ) // only non-safe
	 )
      count++;
  }
  if ( count == 0 )
    return;
  // Then we choose a 'true' random target
  int whichOne = 1+number_range( 0, count-1 );
  for ( CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( vch != ch->fighting // not the person ch is fighting
	 && vch != ch // not himself
	 && !( IS_NPC(vch) && IS_SET(vch->act, ACT_IS_SAFE ) ) // only non-safe
	 && ( --whichOne == 0 ) ) { // found the right one
#ifdef VERBOSE
      log_stringf("CONFUSION: %s has switched to %s (before %s)",
		  NAME(ch), NAME(vch), ch->fighting?NAME(ch->fighting):"[none]");
#endif
      stop_fighting(ch,FALSE);
      set_fighting(ch,vch);
      return;
    }
  }
}

/*
 * Added by SinaC 2000 to allow mob to switch between different targets who are fighting ch
 */
void switch_update(CHAR_DATA *ch) {
  CHAR_DATA *vch, *vch_next;
  /* switch for group */
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ((vch->fighting == ch) && (vch != ch->fighting) && (vch != ch) ) {
#ifdef VERBOSE
      log_stringf("%s has switched to %s (before %s)",
		  NAME(ch), NAME(vch), ch->fighting?NAME(ch->fighting):"[none]");
#endif
      stop_fighting(ch,FALSE);
      set_fighting(ch,vch);
      return;
    }
  }
  return;
}

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update( void )
{
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  CHAR_DATA *victim;

  for ( ch = char_list; ch != NULL; ch = ch->next ) {
    ch_next	= ch->next;
 
    
    // decrement wait & daze, added by SinaC 2000
    if ( IS_NPC(ch) && ( ch->wait > 0 || ch->daze > 0 ) ){
      ch->wait = UMAX( 0, ch->wait - PULSE_VIOLENCE );
      ch->daze = UMAX( 0, ch->daze - PULSE_VIOLENCE );
    }


    /*
     * Hunting mobs. seytan 1997
     */
    if (hunt_victim(ch))
      continue;
    
    if ( ( victim = ch->fighting ) == NULL || ch->in_room == NULL )
      continue;
    
    if ( IS_AWAKE(ch) && ch->in_room == victim->in_room ) {
      // Added by SinaC 2000 to allow mob's targets switching
      if ((number_percent() >= 85) 
	  && IS_NPC(ch) 
	  && !IS_AFFECTED(ch,AFF_CHARM))
	switch_update(ch);
      else { // SinaC 2003: NPC affected by AFF2_CONFUSION attack random target in the room
	if ( IS_NPC(ch)
	     && !IS_AFFECTED(ch,AFF_CHARM)
	     && IS_AFFECTED2(ch,AFF2_CONFUSION) )
	  confusion_switch(ch);
      }
      multi_hit( ch, victim, TYPE_UNDEFINED );
    }
    else
      stop_fighting( ch, FALSE );
    
    if ( ( victim = ch->fighting ) == NULL )
      continue;
    
    /*
     * Fun for the whole family!
     */
    check_assist(ch,victim);

    // Added by SinaC 2003
    if ( ( victim = ch->fighting ) == NULL )
      continue;

    // Added by SinaC 2001
    MOBPROG( ch, NULL, "onFight", victim );
  }
  
  return;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
  CHAR_DATA *rch, *rch_next;

  for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
    rch_next = rch->next_in_room;
    
    // modified by SinaC 2000      && can_see....
    if (IS_AWAKE(rch) && rch->fighting == NULL && can_see(rch,ch)) {

      /* quick check for ASSIST_PLAYER */
      if (!IS_NPC(ch) && IS_NPC(rch) 
	  && IS_SET(rch->off_flags,ASSIST_PLAYERS)
	  &&  rch->level + 6 > victim->level) {
	do_emote(rch,"screams and attacks!");
	multi_hit(rch,victim,TYPE_UNDEFINED);
	continue;
      }

      /* PCs next */
      if (!IS_NPC(ch) || IS_AFFECTED(ch,AFF_CHARM)) {
	if ( ( (!IS_NPC(rch) && IS_SET(rch->act,PLR_AUTOASSIST))
	       ||     IS_AFFECTED(rch,AFF_CHARM)) 
	     &&   is_same_group(ch,rch) 
	     &&   !is_safe(rch, victim))
	  multi_hit (rch,victim,TYPE_UNDEFINED);
	continue;
      }

      /* now check the NPC cases */
	    
      if (IS_NPC(ch) && !IS_AFFECTED(ch,AFF_CHARM)) {
	if ( (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALL))

	     ||   (IS_NPC(rch) && rch->group && rch->group == ch->group)

	     ||   (IS_NPC(rch) && rch->cstat(race) == ch->cstat(race)
		   && IS_SET(rch->off_flags,ASSIST_RACE))

	     ||   (IS_NPC(rch) && IS_SET(rch->off_flags,ASSIST_ALIGN)
		   &&   ((IS_GOOD(rch)    && IS_GOOD(ch))
			 ||  (IS_EVIL(rch)    && IS_EVIL(ch))
			 ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch)))) 

	     ||   (rch->pIndexData == ch->pIndexData 
		   && IS_SET(rch->off_flags,ASSIST_VNUM))) {
	  CHAR_DATA *vch;
	  CHAR_DATA *target;
	  int number;

	  if (number_bits(1) == 0)
	    continue;
		
	  target = NULL;
	  number = 0;
	  for (vch = ch->in_room->people; vch; vch = vch->next) {
	    if (can_see(rch,vch)
		&&  is_same_group(vch,victim)
		&&  number_range(0,number) == 0) {
	      target = vch;
	      number++;
	    }
	  }

	  if (target != NULL) {
	    do_emote(rch,"screams and attacks!");
	    multi_hit(rch,target,TYPE_UNDEFINED);
	  }
	}	
      }
    }
  }
}



/*
 * Returns the chance of success of an attack if ch can't see victim
 *
 * if can't see and not blindfighting ==> return fail
 * if can't see and blindfighting ==> return success
 * if can see ==> return notblind
 */
int check_blindfight( CHAR_DATA *ch, CHAR_DATA *victim,
		      const int notblind, const int success, int const fail ) {
  //  int chance = 0;

  if ( !can_see( ch, victim ) )
    if ( number_percent() < get_ability( ch, gsn_blindfight ) ) {
      check_improve( ch, gsn_blindfight, TRUE, 5 );
      //      chance = success; // can't see and blindfight
      return success;
    }
  //else chance = fail; // can't see and !blindfight
    else return fail; // can't see and !blindfight
  //else chance = notblind; // can see
  else return notblind; // can see
#ifdef VERBOSE
  log_stringf("ch: %s  victim: %s  can_see: %s  chance: %d",
	      NAME(ch), NAME(victim),
	      can_see(ch,victim)?"true":"false",
	      chance );
#endif
//  return chance;
}

/*
 * Do one group of attacks.
 */
// Optimized by SinaC 2003:
//  wield only test once
//  blindfight computed once
void multi_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) {
  int     chance;
  
  // decrement the wait
  // Beurk  removed by SinaC 2000 and moved in violence_update
  //     if (ch->desc == NULL)
  //     ch->wait = UMAX(0,ch->wait - PULSE_VIOLENCE);
  //  
  //     if (ch->desc == NULL)
  //     ch->daze = UMAX(0,ch->daze - PULSE_VIOLENCE); 
  if ( IS_NPC(ch) ) {
    ch->wait = UMAX( 0, ch->wait - PULSE_VIOLENCE );
    ch->daze = UMAX( 0, ch->daze - PULSE_VIOLENCE );
  }

  
  /* no attacks for stunnies -- just a check */
  if (ch->position < POS_RESTING)
    return;
  
  // Added by SinaC 2000
  if (ch->stunned){
    ch->stunned--;
    if ( ch->stunned == 0 ){
      send_to_char("{WYou regain your equilibrium.{x\n\r", ch );
      act( "{W$n regains $s equilibrium.{x", ch, NULL, NULL, TO_ROOM);
    }
    return;
  }

  if (IS_NPC(ch)) { // NPCs have their own multi_hit
    mob_hit(ch,victim,dt);
    return;
  }

  int bonus = 0;

  // Added by SinaC 2003, mounted_combat gives hit bonus
  // if mount is in the same room
  CHAR_DATA *mount = get_mount( ch );
  if ( mount != NULL
       && mount->in_room == ch->in_room ) {
    if ( number_percent() < get_ability( ch, gsn_mounted_combat ) ) {
      bonus = 5;
      check_improve(ch,gsn_mounted_combat,TRUE,5);
    }
  }

  // Added by SinaC 2003 for weapon specialization
  Value *v = get_extra_field( ch, "specialized_weapon" );
  if ( v != NULL ) { // extra field found
    int specialized = v->asInt();
    OBJ_DATA *wield = get_eq_char( ch, WEAR_WIELD );
    int wtype;
    if ( wield == NULL || wield->item_type != ITEM_WEAPON )
      wtype = -1;
    else
      wtype = wield->value[0];
    if ( wtype == specialized && number_percent() <= get_ability(ch,gsn_specialization)) {
      bonus += 10; // 10% chance to hit
      check_improve(ch,gsn_specialization,TRUE,5);
    }
  }

  // Basic first attack ==> MAX 1
  // Added by SinaC 2000
  chance = check_blindfight( ch, victim, 100, 100, 50 ) + bonus;
  if ( number_percent() < chance )
    one_hit( ch, victim, dt, 1 );

  if (ch->fighting != victim)
    return;

  // Extra attack if secondary weapon and dual wield ==> MAX 2
  int dual_wield_perc = get_ability(ch,gsn_dual_wield );
  if ( get_eq_char (ch, WEAR_SECONDARY) 
       && number_percent() < dual_wield_perc ) {
    // Added by SinaC 2000
    chance = check_blindfight( ch, victim, 100, 85, 25 )+bonus;
    if ( number_percent() < chance )
      one_hit( ch, victim, dt, 2 );
    // Added by SinaC 2003
    check_improve(ch,gsn_dual_wield,TRUE,5);
  }

  if ( ch->fighting != victim )
    return;

  // Added by SinaC 2003  dual wield 2
  // Extra attack if secondary weapon and dual wield 2 ==> MAX 3
  if ( get_eq_char (ch, WEAR_SECONDARY) 
       //&& number_percent() < get_ability(ch,gsn_dual_wield2 ) ) {
       && get_casting_level(ch,gsn_dual_wield) >= 2 // second level in dual wield
       && number_percent() < dual_wield_perc ) {
    // Added by SinaC 2000
    chance = check_blindfight( ch, victim, 80+bonus, 0, 0 );
    if ( number_percent() < chance )
      one_hit( ch, victim, dt, 2 );
    // Added by SinaC 2003
    //check_improve(ch,gsn_dual_wield2,TRUE,5);
    check_improve(ch,gsn_dual_wield,TRUE,5);
  }

  if (ch->fighting != victim)
    return;

  // Extra attack if thirdly weapon and thirdl wield ==> MAX 4
  if ( get_eq_char(ch, WEAR_THIRDLY) 
       && number_percent() < get_ability(ch,gsn_third_wield ) ) {
    // Added by SinaC 2000
    chance = check_blindfight( ch, victim, 100, 85, 25 )+bonus;
    if ( number_percent() < chance )
      one_hit( ch, victim, dt, 3 );
    // Added by SinaC 2003
    check_improve(ch,gsn_third_wield,TRUE,5);
  }

  if ( ch->fighting != victim )
    return;

  // Extra attack if fourthly weapon and fourth wield ==> MAX 5
  if ( get_eq_char (ch, WEAR_FOURTHLY) 
       && number_percent() < get_ability(ch,gsn_fourth_wield ) ) {
    // Added by SinaC 2000
    chance = check_blindfight( ch, victim, 100, 85, 25 )+bonus;
    if ( number_percent() < chance )
      one_hit( ch, victim, dt, 4 );
    // Added by SinaC 2003
    check_improve(ch,gsn_fourth_wield,TRUE,5);
  }

  if ( ch->fighting != victim )
    return;

  // Added by SinaC 2003, assassination skill
  if ( dt == gsn_circle || dt == gsn_backstab ) {
    if ( number_percent() <= get_ability( ch, gsn_assassination )/3 ) {
      chance = check_blindfight( ch, victim, 100, 85, 25 ) + bonus;
      if ( number_percent() < chance ) 
	one_hit(ch,victim,dt, 1);
      check_improve( ch, gsn_assassination, TRUE, 8 );
    }
  }

  if ( ch->fighting != victim )
    return;

  // Added by SinaC 2003
  bool slowed = IS_AFFECTED( ch, AFF_SLOW );
  AFFECT_DATA *af_slow = NULL;
  int slow_level = 0;
  if ( slowed ) {
    af_slow = affect_find( ch->affected, gsn_slow );
    slow_level = af_slow->casting_level;
  }

  // Extra attack if affected by HASTE ==> MAX 6
  if (IS_AFFECTED(ch,AFF_HASTE)) { // SinaC 2003: no need to test if affected by slow
                                   //  cos' we can't affected by both haste and slow

    int add_attack;
    // Added by SinaC 2001 ==> more attack with higher haste level
    // no more attack with backstab or circle
    AFFECT_DATA *af = affect_find( ch->affected, gsn_haste );
    AFFECT_DATA *af2 = affect_find( ch->affected, gsn_speedup );
    AFFECT_DATA *af3 = affect_find( ch->affected, gsn_speedofcheetah );
    AFFECT_DATA *af4 = affect_find( ch->affected, gsn_accelerate );

    add_attack = 1;
    if ( af != NULL )
      add_attack = UMAX( af->casting_level, add_attack );
    if ( af2 != NULL )
      add_attack = UMAX( af2->casting_level, add_attack );
    if ( af3 != NULL )
      add_attack = UMAX( af3->casting_level, add_attack );
    if ( af4 != NULL )
      add_attack = UMAX( af4->casting_level, add_attack );

    for ( int i = 0; i < add_attack; i++ ) {
      // Added by SinaC 2000
      chance = check_blindfight( ch, victim, 100, 85, 25 ) + bonus;
      if ( number_percent() < chance )
	one_hit(ch,victim,dt, 1);
      if (ch->fighting != victim )
	return;
      if ( dt == gsn_circle || dt == gsn_backstab )
	break;
    }
  }

  // So, no more than 3 backstabs
  if ( ch->fighting != victim || dt == gsn_backstab )
    return;

  // One additional attack for flurry of fists + full set of additional attacks
  int flurry_of_fists_perc = get_ability( ch, gsn_flurry_of_fists );
  if ( number_percent() < flurry_of_fists_perc
       //&& is_unarmed_and_unarmored(ch) ) {
       && IS_UNARMED_UNARMORED(ch) ) { // SinaC 2003
    chance = get_ability(ch, gsn_hand_to_hand ) + bonus;
    // Added by SinaC 2000, if blind and !blindfight ==> 0%  if blindfight -30%
    chance = check_blindfight( ch, victim, chance, chance-30, 0 );
    if (IS_AFFECTED(ch,AFF_SLOW))
      chance /= (2*slow_level); // Modified by SinaC 2003
    if ( number_percent( ) < chance ) {
      one_hit( ch, victim, dt, 1 );
      check_improve(ch,gsn_hand_to_hand,TRUE,5);
      check_improve( ch, gsn_flurry_of_fists, TRUE, 4 );
    }

    // Flurry of fists has a little chance to do a full set of additional attacks
    if ( number_percent() < flurry_of_fists_perc / 20 // 5% chance when 100% in flurry of mists
    	 && ch->cstat(DEX) - victim->cstat(DEX)-5 >= 0 ) { // ch's dexterity must be higher than victim's dex-5
      act("{gYou speed up your attacks up really fast.{x", ch, NULL, NULL, TO_CHAR );
      act("{r$n speeds up $s attacks up really fast.{x", ch, NULL, victim, TO_VICT );
      act("{y$n speeds up $s attacks up really fast.{x", ch, NULL, victim, TO_NOTVICT );
      check_improve( ch, gsn_flurry_of_fists, TRUE, 4 );
      multi_hit( ch, victim, dt );
    }
  }


  // No second, third, ... attack if no weapon and hand to hand lvl <= 1
  int hit_gsn;
  OBJ_DATA *wield = get_eq_char( ch, WEAR_WIELD );
  int hand_to_hand_casting = get_casting_level( ch, gsn_hand_to_hand );
  if ( ( wield == NULL || wield->item_type != ITEM_WEAPON )
       && hand_to_hand_casting <= 1 )
    return;
  // we should check weapon between every hit because ch could be disarmed by an auto-disarm
  //  but it's not done

  // second attack ==> MAX 7
  //  if no weapon: hand to hand lvl 2
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
    hit_gsn = gsn_hand_to_hand;
  else
    hit_gsn = gsn_second_attack;
  chance = get_ability(ch, hit_gsn ) + bonus;
  // Added by SinaC 2000, if blind and !blindfight ==> 0%  if blindfight -30%
  chance = check_blindfight( ch, victim, chance, chance-30, 0 );
  if (IS_AFFECTED(ch,AFF_SLOW))
    chance /= (2*slow_level); // Modified by SinaC 2003
  if ( number_percent( ) < chance ) {
#ifdef VERBOSE2
    log_stringf("second attack: gsn: %d   ch: %s", hit_gsn, NAME(ch) );
#endif

    one_hit( ch, victim, dt, 1 );
    check_improve(ch,hit_gsn,TRUE,5);
  }

  if ( ch->fighting != victim )
    return;

  // No third, fourth ... attack if no weapon and hand to hand lvl <= 2
  if ( ( wield == NULL || wield->item_type != ITEM_WEAPON )
         && hand_to_hand_casting <= 2 )
    return;

  // third attack ==> MAX 8
  //  if no weapon: hand to hand lvl 3
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
    hit_gsn = gsn_hand_to_hand;
  else
    hit_gsn = gsn_third_attack;
  // Added by SinaC, if blind and !blindfight ==> 0%  if blindfight -50%
  chance = check_blindfight( ch, victim, chance, chance-50, 0 );
  if (IS_AFFECTED(ch,AFF_SLOW) ) 
    chance /= (4*slow_level);
  if ( number_percent( ) < chance ) {
#ifdef VERBOSE2
    log_stringf("third attack: gsn: %d   ch: %s", hit_gsn, NAME(ch) );
#endif

    one_hit( ch, victim, dt, 1 );
    check_improve(ch,hit_gsn,TRUE,6);
  }

  // Added by SinaC 2000 if ch can't see the victim, no more attacks ( 8 max )
  if ( ch->fighting != victim || !can_see( ch, victim ) )
    return;

  // now we're sure ch can see the victim, unless the victim counters ch and
  //  blind ch with this counter ;)))
  // ==> no blindfight test

  // No fourth, fifth ... attack if no weapon and hand to hand lvl <= 3
  if ( ( wield == NULL || wield->item_type != ITEM_WEAPON )
         && hand_to_hand_casting <= 3 )
    return;

  // Added by SinaC 2000, from 4th to 7th attack
  // fourth attack ==> MAX 9
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
    hit_gsn = gsn_hand_to_hand;
  else
    hit_gsn = gsn_fourth_attack;
  chance = get_ability(ch,hit_gsn)/2 + bonus;
  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;
  // Modified by SinaC 2000
  if ( number_percent() < chance && can_see(ch,victim) ) {
#ifdef VERBOSE2
    log_stringf("fourth attack: gsn: %d   ch: %s", hit_gsn, NAME(ch) );
#endif

    one_hit( ch, victim, dt, 1 );
    check_improve(ch,hit_gsn,TRUE,6);
  }
    
  // So, no more than 9 circles 
  if ( ch->fighting != victim || dt == gsn_circle )
    return;

  // No fifth, sixth ... attack if no weapon and hand to hand lvl <= 4
  if ( ( wield == NULL || wield->item_type != ITEM_WEAPON )
         && hand_to_hand_casting <= 4 )
    return;

  // fifth attack ==> MAX 10
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
    hit_gsn = gsn_hand_to_hand;
  else
    hit_gsn = gsn_fifth_attack;
  chance = get_ability(ch,hit_gsn)/2 + bonus;
  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;
  // Modified by SinaC 2000
  if ( number_percent() < chance && can_see(ch,victim) ) {
#ifdef VERBOSE2
    log_stringf("fifth attack: gsn: %d   ch: %s", hit_gsn, NAME(ch) );
#endif

    one_hit( ch, victim, dt, 1 );
    check_improve(ch,hit_gsn,TRUE,6);
  }

  if ( ch->fighting != victim )
    return;

  // No sixth, seventh ... attack if no weapon and hand to hand lvl <= 5
  if ( ( wield == NULL || wield->item_type != ITEM_WEAPON )
         && hand_to_hand_casting <= 5 )
    return;
    
  // sixth attack ==> MAX 11
  if ( wield == NULL || wield->item_type != ITEM_WEAPON )
    hit_gsn = gsn_hand_to_hand;
  else
    hit_gsn = gsn_sixth_attack;
  chance = get_ability(ch,hit_gsn)/2 + bonus;
  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;
  // Modified by SinaC 2000
  if ( number_percent() < chance && can_see(ch,victim) ) {
#ifdef VERBOSE2
    log_stringf("sixth attack: gsn: %d   ch: %s", hit_gsn, NAME(ch) );
#endif

    one_hit( ch, victim, dt, 1 );
    check_improve(ch,hit_gsn,TRUE,6);
  }
  if ( ch->fighting != victim )
    return;

  // 7th attack: only for armed fighters
  // seventh attack ==> MAX 12
  chance = get_ability(ch,gsn_seventh_attack)/2 + bonus;
  if (IS_AFFECTED(ch,AFF_SLOW))
    chance = 0;
  // Modified by SinaC 2000
  if ( number_percent() < chance && can_see(ch,victim) ) {
#ifdef VERBOSE2
    log_stringf("seventh attack: gsn: %d  ch: %s", hit_gsn, NAME(ch) );
#endif

    one_hit( ch, victim, dt, 1 );
    check_improve(ch,gsn_seventh_attack,TRUE,6);
  }

  return;
}

/* procedure for all mobile attacks */
void mob_hit (CHAR_DATA *ch, CHAR_DATA *victim, int dt) {
  int chance,number;
  CHAR_DATA *vch, *vch_next;
  
  // Added by SinaC 2003
  if ( ch->position <= POS_STUNNED )
    return;

  // Added by SinaC 2000
  if (ch->stunned)
    return;

  // At least a basic attack ==> MAX 1
  // Added by SinaC 2000
  chance = check_blindfight( ch, victim, 100, 100, 50 );
  if ( number_percent() < chance )
    one_hit(ch,victim,dt, 1);
  
  if (ch->fighting != victim)
    return;
  
  /* Area attack -- BALLS nasty! */
  if (IS_SET(ch->off_flags,OFF_AREA_ATTACK)) {
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
      vch_next = vch->next;
      if ((vch != victim && vch->fighting == ch)) {
	// I think 0% success if blind is logic
	chance = check_blindfight( ch, victim, 100, 0, 0 );
	if ( number_percent() < chance )
	  one_hit(ch,vch,dt, 1);
      }
    }
  }

  // Modified by SinaC 2001
  // Extra attack if affected by HASTE ==> MAX 3
  if (IS_AFFECTED(ch,AFF_HASTE)
      ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW))) {
    AFFECT_DATA *af;
    AFFECT_DATA *af2;
    AFFECT_DATA *af3;
    int add_attack;
    // Added by SinaC 2001 ==> more attack with higher haste level
    // no more attack with backstab or circle
    af = affect_find( ch->affected, gsn_haste );
    af2 = affect_find( ch->affected, gsn_speedup );
    af3 = affect_find( ch->affected, gsn_speedofcheetah );

    add_attack = 1;
    if ( af != NULL )
      add_attack = UMAX( af->casting_level, add_attack );
    if ( af2 != NULL )
      add_attack = UMAX( af2->casting_level, add_attack );

    for ( int i = 0; i < add_attack; i++ ) {
      // Added by SinaC 2000
      chance = check_blindfight( ch, victim, 100, 85, 35 );
      if ( number_percent() < chance )
	one_hit(ch,victim,dt, 1);
      if (ch->fighting != victim )
	return;
      if ( dt == gsn_circle || dt == gsn_backstab )
	break;
    }
  }
  /*  
  // Additionnal attack if affected by HASTE ==> MAX 2
  if (IS_AFFECTED(ch,AFF_HASTE) 
      ||  (IS_SET(ch->off_flags,OFF_FAST) && !IS_AFFECTED(ch,AFF_SLOW))) {
    // Added by SinaC 2000
    chance = check_blindfight( ch, victim, 100, 85, 35 );
    if ( number_percent() < chance )
      one_hit(ch,victim,dt, FALSE);
  }
  */

  // Extra attack if secondary weapon and dual wield ==> MAX 3
  if ( get_eq_char (ch, WEAR_SECONDARY) 
       && ( number_percent() < get_ability(ch,gsn_dual_wield ) ) ) {
    // Added by SinaC 2000
    chance = check_blindfight( ch, victim, 100, 85, 35 );
    if ( number_percent() < chance )
      one_hit( ch, victim, dt, 2 );
  }

  // So, no more than 3 backstabs for a mob
  if (ch->fighting != victim || dt == gsn_backstab )
    return;

  // second attack ==> MAX 4
  chance = get_ability(ch,gsn_second_attack); //   /2 removed
  // Added by SinaC 2000, modified by SinaC 2001
  //chance = check_blindfight( ch, victim, chance, chance-20, 0 );
  chance = check_blindfight( ch, victim, chance, chance-20, chance-40 );
  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST)) 
    chance /= 2;
  // Added by SinaC 2000
  if (number_percent() < chance) 
    one_hit(ch,victim,dt, 1);
  
  // So, no more than 4 circles for a mob
  // Added by SinaC 2000
  if (ch->fighting != victim || dt == gsn_circle )
    return;

  // third attack ==> MAX 5
  chance = get_ability(ch,gsn_third_attack); //   /4 removed
  // Added by SinaC 2000, modified by SinaC 2001
  //chance = check_blindfight( ch, victim, chance, chance-50, 0 );
  chance = check_blindfight( ch, victim, chance, chance-30, chance-50 );
  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST)) 
    chance /= 4;
  if (number_percent() < chance) 
    one_hit(ch,victim,dt, 1);

  /* Modified again by SinaC 2000
     // Added by SinaC 2000  || !can_see(...) ==> no special attack if can't see target
     if (ch->fighting != victim || !can_see( ch, victim ) )
     return;
  */
  if ( ch->fighting != victim )
    return;

  // Added by SinaC 2001, for mob's 4th, 5th, 6th and 7th attack
  // fourth attack ==> MAX 6
  chance = get_ability(ch,gsn_fourth_attack); //  /2 removed
  // Added by SinaC 2001
  chance = check_blindfight( ch, victim, chance, chance-50, chance-60 );
  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance = 0;
  if ( number_percent() < chance)
    one_hit( ch, victim, dt, 1 );

  if ( ch->fighting != victim )
    return;
    
  // fifth attack ==> MAX 7
  chance = get_ability(ch,gsn_fifth_attack); //  /2 removed
  // Added by SinaC 2001
  chance = check_blindfight( ch, victim, chance, chance-50, chance-60 );
  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance = 0;
  if ( number_percent() < chance )
    one_hit( ch, victim, dt, 1 );

  if ( ch->fighting != victim )
    return;

  // sixth attack ==> MAX 8
  chance = get_ability(ch,gsn_sixth_attack)/2;
  // Added by SinaC 2001
  chance = check_blindfight( ch, victim, chance, chance-50, chance-60 );
  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance = 0;
  if ( number_percent() < chance )
    one_hit( ch, victim, dt, 1 );

  if ( ch->fighting != victim )
    return;
    
  // seventh attack ==> MAX 9
  chance = get_ability(ch,gsn_seventh_attack)/2;
  // Added by SinaC 2001
  chance = check_blindfight( ch, victim, chance, chance-50, chance-60 );
  if (IS_AFFECTED(ch,AFF_SLOW) && !IS_SET(ch->off_flags,OFF_FAST))
    chance = 0;
  if ( number_percent() < chance )
    one_hit( ch, victim, dt, 1 );
  
  /* oh boy!  Fun stuff! */

  if ( ch->fighting != victim )
    return;

  if (ch->wait > 0)
    return;
  /*
   *   number = number_range(0,2);
   *
   *   if (number == 1 && IS_SET(ch->act,ACT_MAGE))
   *   {
   * //	{ mob_cast_mage(ch,victim); return; }
   *   }
   *
   *   if (number == 2 && IS_SET(ch->act,ACT_CLERIC))
   *   {	
   * //	{ mob_cast_cleric(ch,victim); return; }
   *   }
   */

  // Added by SinaC 2000 for mobiles classes
  if (
      !IS_AFFECTED(ch,AFF_CHARM ) // not charmed
      && ch->cstat(classes) != 0    // has at least a class
      && number_bits(2)==0){     // 1/4 chance

    int sn;
    char buf1[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    CHAR_DATA *victim2;

    // Modified by SinaC 2001 for mob classes
    sn = get_random_mob_usable_ability( ch );
    if ( sn <= 0 ) {
#ifdef VERBOSE
      log_stringf("No valid skills/spells found for %s(%d)",
		  NAME(ch), ch->pIndexData->vnum );
#endif
      return;
    }
    if ( !can_see( ch, victim ) ) {
#ifdef VERBOSE
      log_stringf("Can't see target: %s", victim->name );
#endif
      return;
    }

    mob_use_skill_spell(ch, victim, sn );
    return;
  }
  
  // no class

  /* now for the skills */
  //number = number_range(0,9);
  number = number_range(0,10);

  switch(number) {
  case (0) :
    if (IS_SET(ch->off_flags,OFF_BASH))
      do_bash(ch,"");
    break;
      
  case (1) :
    if ( IS_SET(ch->off_flags,OFF_BERSERK) 
	 && !IS_AFFECTED(ch,AFF_BERSERK))
      do_berserk(ch,"");
    break;
      
  case (2) :
    if (IS_SET(ch->off_flags,OFF_DISARM) 
	//|| (get_weapon_sn(ch, FALSE ) != gsn_hand_to_hand 
	|| (get_weapon_sn(ch, 1) != gsn_hand_to_hand 
	    && (IS_SET(ch->act,ACT_WARRIOR)
		||  IS_SET(ch->act,ACT_THIEF))))
      do_disarm(ch,"");
    break;
      
  case (3) :
    if (IS_SET(ch->off_flags,OFF_KICK))
      do_kick(ch,"");
    break;
      
  case (4) :
    if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
      do_dirt(ch,"");
    break;
      
  case (5) :
    if (IS_SET(ch->off_flags,OFF_TAIL))
      do_tail(ch,"");
    break; 
      
  case (6) :
    if (IS_SET(ch->off_flags,OFF_TRIP))
      do_trip(ch,"");
    break;
      
  case (7) :
    if (IS_SET(ch->off_flags,OFF_CRUSH))
      do_crush(ch,"");
    break;
  case (8) :
    if (IS_SET(ch->off_flags,OFF_BACKSTAB)) {     
      //do_backstab(ch,"");
      // modified by SinaC 2000
      do_circle(ch,"");
    }
    break;
    // Added by SinaC 2001
  case (9):
    if (IS_SET(ch->off_flags, OFF_BITE)) {
      do_bite(ch,"");
    }
    break;
    // Added by SinaC 2001 for raceskills like feed for vampire
  case (10):
    int sn = get_random_race_ability( ch );
    if ( sn <= 0 ) {
#ifdef VERBOSE
      log_stringf("No valid RACE skills/spells found for %s(%d)",
		  NAME(ch), ch->pIndexData->vnum );
#endif
      return;
    }
    if ( !can_see( ch, victim ) ) {
#ifdef VERBOSE
      log_stringf("Can't see target: %s", victim->name );
#endif
      return;
    }

    mob_use_skill_spell(ch, victim, sn );
    break;
  }
}

void mob_use_skill_spell(CHAR_DATA *ch, CHAR_DATA *victim, int sn )
{
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  CHAR_DATA *victim2;

  chardata_to_str( ch, victim, buf1, victim2 );
  
  if ( victim2 != victim ) {
#ifdef VERBOSE
    log_stringf("Invalid target: %s [%s]",
		victim2?victim2->name:"(no target)", buf1 );
#endif
    return;
  }
  int sklvl = get_casting_level( ch, sn );
  switch(ability_table[sn].type) {
  case TYPE_SKILL:
    sprintf( buf2, "%s %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf( "%s uses %s (lvl %d) on %s",
		 NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    do_use( ch, buf2 );
    break;
  case TYPE_SPELL:
    sprintf( buf2, "'%s' %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf("%s casts %s (lvl %d) on %s",
		NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    do_cast( ch, buf2 );
    break;
  case TYPE_POWER:
    sprintf( buf2, "'%s' %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf("%s powers %s (lvl %d) on %s",
		NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    do_power( ch, buf2 );
    break;
    // Added by SinaC 2003 for bard (songs)
  case TYPE_SONG:
    sprintf( buf2, "'%s' %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf("%s songs %s (lvl %d) on %s",
		NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    do_songs( ch, buf2 );
    break;
  default:
    bug("Invalid ability type [%d] in fight.C:mob_use_skill_spell", ability_table[sn].type );
    break;
  }
  /* Modified by SinaC 2001
  if ( ability_table[sn].spell_fun == spell_null ){ // skill
    sprintf( buf2, "%s %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf( "%s uses %s (lvl %d) on %s",
		 NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    // Modified by SinaC 2001
    //interpret( ch, buf2 );
    do_use( ch, buf2 );
  }
  // Modified by SinaC 2001 for mental power
  else if ( ability_table[sn].type == TYPE_SPELL ) { // spell
    sprintf( buf2, "'%s' %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf("%s casts %s (lvl %d) on %s",
		NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    do_cast( ch, buf2 );
  }
  else { // power
    sprintf( buf2, "'%s' %s", ability_table[sn].name, buf1 );
#ifdef VERBOSE
    log_stringf("%s powers %s (lvl %d) on %s",
		NAME(ch), ability_table[sn].name, sklvl, buf1 );
#endif
    do_power( ch, buf2 );
  }
  */

}




// Additional affects from funky weapon
void additional_hit_affect( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield ) {
  int dam;
  if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_POISON)) {
    int level;
    int casting_level; // Modified by SinaC 2003 for casting_level
    AFFECT_DATA *poison;
    
    if ((poison = affect_find(wield->affected,gsn_poison)) == NULL) {
      level = wield->level;
      casting_level = 1;
    }
    else {
      level = poison->level;
      casting_level = UMAX(poison->casting_level/2,1);
    }
    
    if (!saves_spell(level/2,victim,DAM_POISON)) {
      send_to_char("You feel {Gpoison coursing through your veins{x.\n\r",
		   victim);
      act("$n is {Gpoisoned by the venom{x on $p.",
	  victim,wield,NULL,TO_ROOM);

      AFFECT_DATA af;
      createaff(af,level/2,(level*3)/4,gsn_poison,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,affected_by,OR,AFF_POISON);
      addaff(af,CHAR,STR,ADD,-1*casting_level);
      affect_join( victim, &af );
      //newafsetup(af,CHAR,affected_by,OR,AFF_POISON,level/2,level*3/4,gsn_poison,casting_level);
      //affect_join( victim, &af );
      //newafsetup(af,CHAR,STR,ADD,-1*casting_level,level/2,level*3/4,gsn_poison,casting_level);
      //affect_join( victim, &af );
    }
    
    // weaken the poison if it's temporary
    if (poison != NULL) {
      poison->level = UMAX(0,poison->level - 2);
      poison->duration = UMAX(0,poison->duration - 1);
      
      if (poison->level == 0 || poison->duration == 0)
	act("The poison on $p has worn off.",ch,wield,NULL,TO_CHAR);
    }
  }
  
  // Added by SinaC 2001 for holy weapon
  if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_HOLY)) {
    dam = number_range(1, wield->level / 5 + 1);
    act("$p {Wpurifies{x $n.",victim,wield,NULL,TO_ROOM);
    act("You feel $p {Wpurifying{x you.",
	victim,wield,NULL,TO_CHAR);
    int done = ability_damage( ch, victim, dam, 0, DAM_HOLY, FALSE, TRUE );
    if ( done == DAMAGE_DONE ) { // Added by SinaC 2003
      ch->bstat(alignment) = URANGE( -1000, ch->bstat(alignment) + 1, +1000 );
      ch->cstat(alignment) = URANGE( -1000, ch->cstat(alignment) + 1, +1000 );
    }
  }
  
  if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_VAMPIRIC)) {
    dam = number_range(1, wield->level / 5 + 1);
    act("$p {Ddraws life{x from $n.",victim,wield,NULL,TO_ROOM);
    act("You feel $p {Ddrawing your life away{x.",
	victim,wield,NULL,TO_CHAR);
    int done = ability_damage( ch, victim, dam, 0, DAM_NEGATIVE, FALSE, TRUE );
    if ( done == DAMAGE_DONE ) { // Added by SinaC 2003
      // Modified by SinaC 2001 etho/alignment are attributes now
      // Modified by SinaC 2000 for alignment/etho
      ch->bstat(alignment) = URANGE( -1000, ch->bstat(alignment) - 1, +1000 );
      ch->cstat(alignment) = URANGE( -1000, ch->cstat(alignment) - 1, +1000 );
      ch->hit += dam/2;
    }
  }
  
  if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FLAMING)) {
    dam = number_range(1,wield->level / 4 + 1);
    act("$n is {Rburned{x by $p.",victim,wield,NULL,TO_ROOM);
    act("$p {Rsears your flesh{x.",victim,wield,NULL,TO_CHAR);
    int done = ability_damage( ch, victim, dam, 0, DAM_FIRE, FALSE, TRUE );
    if ( done == DAMAGE_DONE )
      fire_effect( (void *) victim,wield->level/2,dam,TARGET_CHAR);
  }
  
  if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_FROST))	{
    dam = number_range(1,wield->level / 6 + 2);
    act("$p {Cfreezes{x $n.",victim,wield,NULL,TO_ROOM);
    act("The {Ccold touch{x of $p {Csurrounds you with ice{x.",
	victim,wield,NULL,TO_CHAR);
    int done = ability_damage( ch, victim, dam, 0, DAM_COLD, FALSE, TRUE );
    if ( done == DAMAGE_DONE )
      cold_effect(victim,wield->level/2,dam,TARGET_CHAR);
  }
  
  if (ch->fighting == victim && IS_WEAPON_STAT(wield,WEAPON_SHOCKING)) {
    dam = number_range(1,wield->level/5 + 2);
    act("$n is {Ystruck by lightning{x from $p.",victim,wield,NULL,TO_ROOM);
    act("You are {Yshocked{x by $p.",victim,wield,NULL,TO_CHAR);
    int done = ability_damage( ch, victim, dam, 0, DAM_LIGHTNING, FALSE, TRUE );
    if ( done == DAMAGE_DONE )
      shock_effect(victim,wield->level/2,dam,TARGET_CHAR);
  }
  // Added by SinaC 2000 for deathgrip
  if (ch->fighting == victim && is_affected(ch,gsn_deathgrip ) ) {
    dam = number_range(1,wield->level / 5 + 4);
    act("The {Devil power{x of $p torments $n.",victim,wield,NULL,TO_ROOM);
    act("The {Devil power{x of $p torments you.",victim,wield,NULL,TO_CHAR);
    int done = ability_damage( ch, victim, dam, 0, DAM_NEGATIVE, FALSE, TRUE );
    // Modified by SinaC 2001, 0 instead of 350
    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000 for alignment/etho
    //if ( !IS_NPC(victim) && ( victim->align.alignment > 0 ) )
    if ( done == DAMAGE_DONE && !IS_NPC(victim) && ( victim->cstat(alignment) > 350 ) )
      victim->mana = UMAX(victim->mana-(ch->level)/8,0);
  }
}


/*
 * Hit one guy once.
 */
void one_hit( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int whichWield ) {
  OBJ_DATA *wield;
  int victim_ac;
  int thac0;
  int thac0_00;
  int thac0_32;
  int dam;
  int diceroll;
  int sn,skill;
  int dam_type;
  int result; // bool before, SinaC 2003

  sn = -1;

  /* just in case */
  if (victim == ch || ch == NULL || victim == NULL || !ch->valid || !victim->valid )
    return;

  /*
   * Can't beat a dead char!
   * Guard against weird room-leavings.
   */
  if ( victim->position == POS_DEAD || ch->in_room != victim->in_room )
    return;

  // Added by SinaC 2003
  if ( ch->position <= POS_STUNNED )
    return;

  /*
   * Figure out the type of damage message.
   * if secondary == true, use the second weapon.
   */
  //  if (!secondary)
  //    wield = get_eq_char( ch, WEAR_WIELD );
  //  else
  //    wield = get_eq_char( ch, WEAR_SECONDARY );
  switch( whichWield ) {
  case 0: wield = NULL; break; // hand to hand
  case 1: wield = get_eq_char( ch, WEAR_WIELD ); break;
  case 2: wield = get_eq_char( ch, WEAR_SECONDARY ); break;
  case 3: wield = get_eq_char( ch, WEAR_THIRDLY ); break;
  case 4: wield = get_eq_char( ch, WEAR_FOURTHLY ); break;
  default: 
    wield = get_eq_char( ch, WEAR_WIELD ); 
    bug("Invalid whichWield in one_hit [%d]", whichWield);
    break;
  }

  /* Oxtal> I wonder why to test on dt if it has never been assigned ? */
  // It can be assigned when calling multi_hit from backstab or circle, SinaC 2003
  if ( dt == TYPE_UNDEFINED ) {
    dt = TYPE_HIT;
    if ( wield != NULL && wield->item_type == ITEM_WEAPON ) {
      dt += GET_WEAPON_DAMTYPE(wield);
      //dt += wield->value[3];
    }
    else
      dt += ch->cstat(dam_type); // bare hand damage: punch or claws
  }

  // Added by SinaC 2001
  if ( IS_SET(PART_CLAWS,ch->cstat(parts)) && wield == NULL )
    dt = TYPE_HIT + 5; // so ==> your claw wounds ...

  if (dt < TYPE_HIT)
    if (wield != NULL) {
      dam_type = attack_table[GET_WEAPON_DAMTYPE(wield)].damage;
      //dam_type = attack_table[wield->value[3]].damage;
    }
    else
      dam_type = attack_table[ch->cstat(dam_type)].damage;
  else
    dam_type = attack_table[dt - TYPE_HIT].damage; // get dam type from attack_table

  if (dam_type == -1)
    dam_type = DAM_BASH;

  /* get the weapon skill */
  //sn = get_weapon_sn( ch, secondary );
  //skill = 20 + get_weapon_ability(ch,sn);
  sn = get_weapon_sn( ch, wield );
  skill = 20 + get_weapon_ability(ch,sn);

  /*
   * Calculate to-hit-armor-class-0 versus armor.
   */
  if ( IS_NPC(ch) ) {
    thac0_00 = 20;
    thac0_32 = -4;   /* as good as a thief */
    if (IS_SET(ch->act,ACT_WARRIOR))
      thac0_32 = -10;
    else if (IS_SET(ch->act,ACT_THIEF))
      thac0_32 = -4;
    else if (IS_SET(ch->act,ACT_CLERIC))
      thac0_32 = 2;
    else if (IS_SET(ch->act,ACT_MAGE))
      thac0_32 = 6;
  }
  else {
    thac0_00 = class_thac0_00(ch->cstat(classes));
    thac0_32 = class_thac0_32(ch->cstat(classes));
  }
  thac0  = interpolate( ch->level, thac0_00, thac0_32 );

  if (thac0 < 0)
    thac0 = thac0/2;

  if (thac0 < -5)
    thac0 = -5 + (thac0 + 5) / 2;

  thac0 -= GET_HITROLL(ch) * skill/100;
  thac0 += 5 * (100 - skill) / 100;

  if (dt == gsn_backstab)
    thac0 -= 10 * (100 - get_ability(ch,gsn_backstab));

  // Added by SinaC 2000
  if (dt == gsn_circle)
    thac0 -= 10 * (100 - get_ability(ch,gsn_circle));

  switch(dam_type) {
  case(DAM_PIERCE):      victim_ac = GET_AC(victim,AC_PIERCE)/10;	break;
  case(DAM_BASH):	 victim_ac = GET_AC(victim,AC_BASH)/10;		break;
  case(DAM_SLASH):       victim_ac = GET_AC(victim,AC_SLASH)/10;	break;
  default:	         victim_ac = GET_AC(victim,AC_EXOTIC)/10;	break;
  };

  if (victim_ac < -15)
    victim_ac = (victim_ac + 15) / 5 - 15;

  if ( !can_see( ch, victim ) )
    victim_ac -= 4;

  if ( victim->position < POS_FIGHTING)
    victim_ac += 4;

  if (victim->position < POS_RESTING)
    victim_ac += 6;

  // Added by SinaC 2003
  // if victim is mounted or mounting, have skill mounted_combat and are in the same room
  //  victim gets an ac bonus
  CHAR_DATA *mount = get_mount( victim );
  CHAR_DATA *leader;
  if ( IS_NPC(victim) )
    leader = get_mount_master( victim );
  else
    leader = victim;
  if ( ( !IS_NPC(victim)
	 && mount
	 && mount->in_room == ch->in_room )
       || 
       ( IS_NPC(victim)
	 && leader
	 && victim->in_room == leader->in_room ) ) {
    int skill = get_ability( leader, gsn_mounted_combat );
    if ( number_percent() < skill ) {
      victim_ac -= 5;
      check_improve( leader, gsn_mounted_combat, TRUE, 5 );
    }
  }

  /*
   * The moment of excitement!
   */
  while ( ( diceroll = number_bits( 5 ) ) >= 20 )
    ;

  /* That test was quite weird, let's try something else  SinaC 2000 */
  if ( diceroll == 0
       || ( diceroll != 19 && diceroll < thac0 - victim_ac ) ) {
    /**/
  //if ( diceroll == 0 || diceroll < thac0 - victim_ac ) {
    /* Miss. */
    //int saved_whichWield = glob_whichWield;
    //glob_whichWield = whichWield;
    //damage( ch, victim, 0, dt, dam_type, TRUE, FALSE );
    //glob_whichWield = saved_whichWield;
    combat_damage( ch, victim, 0, dt, dam_type, TRUE, wield );
    return;
  }

  /*
   * Hit.
   * Calc damage.
   */

  if ( IS_NPC(ch) && (!ch->pIndexData->new_format || wield == NULL))
    if (!ch->pIndexData->new_format) {
      dam = number_range( ch->level / 2, ch->level * 3 / 2 );
      if ( wield != NULL )
	dam += dam / 2;
    }
    else {
      // Modified by SinaC 2000
      dam = dice(ch->cstat(DICE_NUMBER),ch->cstat(DICE_TYPE))/* + ch->cstat(DICE_BONUS)*/;
    }
  else {
    if (sn != -1)
      check_improve(ch,sn,TRUE,5);
    if ( wield != NULL ) {
      if (wield->pIndexData->new_format)
	//dam = dice(wield->value[1],wield->value[2]) * skill/100;
	dam = (dice(GET_WEAPON_DNUMBER(wield),GET_WEAPON_DTYPE(wield))*skill)/100;
      else
	//dam = (number_range( wield->value[1],
	//		    wield->value[2])*skill)/100;
	dam = (number_range(GET_WEAPON_DNUMBER(wield),GET_WEAPON_DTYPE(wield))*skill)/100;

      if (get_eq_char(ch,WEAR_SHIELD) == NULL)  /* no shield = more */
	dam = dam * 11/10;
      
      /* sharpness! and weighted by SinaC 2001 */
      if (IS_WEAPON_STAT(wield,WEAPON_SHARP)
	  || IS_WEAPON_STAT(wield,WEAPON_WEIGHTED) ) {
	int percent;
	
	if ((percent = number_percent()) <= (skill / 8))
	  dam = 2 * dam + ( dam * 2 * percent / 100 );
      }
      /* Added by SinaC 2000 
       *  vorpal --> kill in one hit !
       *  no chance if immune
       *  5/1000    if vulnerable
       *  1/1000    if resistant
       *  2/1000    otherwise
       */
      if (IS_WEAPON_STAT(wield,WEAPON_VORPAL) && !IS_IMMORTAL(victim))  {
	int percent = number_range(0, 999);
	int chance = 998;

	switch( check_immune( victim, dam_type ) ) {
	case IS_IMMUNE : chance = 1000; break;
	case IS_RESISTANT : chance = 999; break;
	case IS_VULNERABLE : chance = 995; break;
	}
		
	if ( percent >= chance ) {
	  act( "The {Cvorpal{x of $p strikes $N.", ch, wield, victim, TO_NOTVICT );
	  act( "The {Cvorpal{x of your weapon strikes $N.", ch, wield, victim, TO_CHAR );
	  act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	  act( "The {Cvorpal{x of $p strikes and KILLS you.", ch, wield, victim, TO_VICT );
	  sudden_death( ch, victim );
		    
	  return;
	}
      }
      /* Added by SinaC 2003 for necrotism
       *  necrotism --> kill in one hit !
       *  no chance if immune
       *  5/1000    if vulnerable
       *  1/1000    if resistant
       *  2/1000    otherwise
       */
      if (IS_WEAPON_STAT(wield,WEAPON_NECROTISM) && !IS_IMMORTAL(victim))  {
	int percent = number_range(0, 999);
	int chance = 998;

	switch( check_immune( victim, dam_type ) ) {
	case IS_IMMUNE : chance = 1000; break;
	case IS_RESISTANT : chance = 999; break;
	case IS_VULNERABLE : chance = 995; break;
	}
		
	if ( percent >= chance ) {
	  act( "The {DNecrotism{x of $p strikes $N.", ch, wield, victim, TO_NOTVICT );
	  act( "The {DNecrotism{x of your weapon strikes $N.", ch, wield, victim, TO_CHAR );
	  act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
	  act( "The {DNecrotism{x of $p strikes and KILLS you.", ch, wield, victim, TO_VICT );
	  sudden_death( ch, victim );
		    
	  return;
	}
      }

      /* not a good idea, SinaC 2001
      // Added by SinaC 2001 for weapon condition
      if ( wield->condition != 100 )
	if ( wield->condition <= 1 )
	  dam = 1;
	else
	  dam = ( dam * wield->condition ) / 100;
      */
    }
    else
      dam = number_range( 1 + 4 * skill/100, 2 * ch->level/3 * skill/100);
  }

  /*
   * Bonuses.
   */
  if ( get_ability(ch,gsn_enhanced_damage) > 0 ) {
    diceroll = number_percent();
    if (diceroll <= get_ability(ch,gsn_enhanced_damage)) {
      check_improve(ch,gsn_enhanced_damage,TRUE,6);
      dam += 2 * ( dam * diceroll/300);
    }
    // Added by SinaC 2003 for enhanced damage lvl 2
    if ( get_casting_level( ch, gsn_enhanced_damage ) == 2 ) {
      check_improve(ch,gsn_enhanced_damage,TRUE,10);
      dam += 2 * ( dam * diceroll/200);
    }
  }

  // Modified by SinaC 2001 etho/alignment are attributes now
  // Modified by SinaC 2000 for alignment/etho
  // Added by SinaC 2000 for deathgrip
  if ( wield != NULL && is_affected(ch,gsn_deathgrip ) ) {
    //if (victim->align.alignment > 700)
    if (victim->cstat(alignment) > 700)
      dam = (110 * dam) / 100;
	
    //else if (victim->align.alignment > 350)
    else if (victim->cstat(alignment) > 350)
      dam = (105*dam) / 100;
	
    else dam = (102*dam) / 100;
  }

  // Added by SinaC 2003 for weapon specialization
  if ( wield != NULL ) {
    Value *v = get_extra_field( ch, "specialized_weapon" );
    if ( v != NULL ) { // extra field found
      int specialized = v->asInt();
      int wtype = wield->value[0];
      if ( wtype == specialized && number_percent() <= get_ability(ch,gsn_specialization)) {
	dam = (dam * 3)/2; // 3/2 * damage
	check_improve(ch,gsn_specialization,TRUE,5);
      }
    }
  }

  // Added by SinaC 2003 for favored enemy
  Value *v = get_extra_field( ch, "favored_enemy" );
  if ( v != NULL ) {
    char w[MAX_INPUT_LENGTH];
    int race;
    strcpy( w, v->asStr() );
    if ( w != NULL && w[0] != '\0'                                // found non empty string
	 && number_percent() <= get_ability(ch,gsn_favored_enemy) // skill check
	 && ( race = race_lookup( w, TRUE ) ) >= 0                // available race
	 && race == victim->cstat(race) )                         // favored = enemy
      dam = ( dam * 3 ) / 2;
  }

  // Added by SinaC 2003 for improved exotic
  if ( sn == -1 ) { // EXOTIC
    if ( number_percent() <= get_ability( ch, gsn_improved_exotic ) )
      dam = ( dam * 3 ) / 2; // 3/2 * dam if improved exotic
  }
    
  if ( !IS_AWAKE(victim) )
    dam *= 2;
  else 
    if (victim->position < POS_FIGHTING)
      dam = dam * 3 / 2;

  if ( dt == gsn_backstab && wield != NULL) 
    if ( wield->value[0] != WEAPON_DAGGER )
      dam *= 2 + (ch->level / 20); 
    else
      dam *= 2 + (ch->level / 8);

  // Added by SinaC 2000  
  if ( dt == gsn_circle && wield != NULL) 
    if ( wield->value[0] != WEAPON_DAGGER )
      dam *= 2 + (ch->level / 15); 
    else
      dam *= 2 + (ch->level / 12);

  dam += GET_DAMROLL(ch) * UMIN(100,skill) /100;

  // SinaC 2003: ranged weapon does really less damage
  if ( wield != NULL && wield->value[0] == WEAPON_RANGED )
    dam /= 3;

  if ( dam <= 0 )
    dam = 1;

  /* If player and no weapon */
  if ( wield == NULL ) {
    int sk;
    // SinaC 2000, player gets better damage if they know hand_to_hand
    if (!IS_NPC( ch ) && ( sk = get_ability( ch, gsn_hand_to_hand ) ) > 0 ) {
      check_improve( ch, gsn_hand_to_hand, TRUE, 4 );
      dam += (dam * sk) / 150;

      // more damage if higher level in hand_to_hand
      int hand_to_hand_level = get_casting_level( ch, gsn_hand_to_hand );
      if ( hand_to_hand_level > 1 )
	dam += ( dam * 5 ) / hand_to_hand_level;

      // they also get an additional bonus from flurry_of_fists
      int flurry_of_fists_perc = get_ability( ch, gsn_flurry_of_fists );
      if ( number_percent() < flurry_of_fists_perc
	   //&& is_unarmed_and_unarmored(ch) ) {
	   && IS_UNARMED_UNARMORED(ch) ) { // SinaC 2003
	check_improve( ch, gsn_flurry_of_fists, TRUE, 4 );
	dam += ( dam * flurry_of_fists_perc ) / 150;
      }
    }
    
    // Removed by SinaC 2001
    /*
    // If enhanced punch        dam = dam + dam * skill / 150
    if (!IS_NPC( ch ) && ( sk = get_ability( ch, gsn_enhanced_hand ) ) > 0 ) {
      check_improve( ch, gsn_enhanced_hand, TRUE, 4 );
      dam += (dam * sk) / 150;
    }
    // Added by SinaC 2001
    */
    // If affected by iron hand ==> dam += dam*sk/150;
    if (!IS_NPC(ch)
	&& ( is_affected( ch, gsn_iron_hand ) 
	     || is_affected( ch, gsn_stone_fist ) ) ) {
      dam += (dam * get_ability( ch, gsn_iron_hand ) ) / 150;
    }
    
    // If claws ==> nekojin       dam = dam + level/2
    if ( IS_SET(PART_CLAWS,ch->cstat(parts))) {
      // Modified by SinaC 2001
      dam += UMAX(2,( ch->level / 4 )); // was  dam += lvl/2  before
      // Removed by SinaC 2001, done sooner in the code
      //dt = TYPE_HIT + 5; // so ==> your claw wounds ...
    }
  }
  
  // Modified by SinaC 2000 for counter attack
  //  can't counter backstab or circles
  if ( wield != NULL 
       && dt != gsn_backstab && dt != gsn_circle ) {
    //if ( check_counter( ch, victim, dam, dt, secondary ) ) SinaC 2003
    if ( check_counter( ch, victim, dam, dt, whichWield ) )
      return;
  }
  
  //int saved_whichWield = glob_whichWield;
  //glob_whichWield = whichWield;
  //result = damage( ch, victim, dam, dt, dam_type, TRUE, FALSE );
  //glob_whichWield = saved_whichWield;
  result = combat_damage( ch, victim, dam, dt, dam_type, TRUE, wield );

  // Added by SinaC 2001 for funky punches :))
  //  only if unarmed and unarmored, SinaC 2003
  if (result == DAMAGE_DONE && wield == NULL
      //&& is_unarmed_and_unarmored(ch) ) {
      && IS_UNARMED_UNARMORED(ch) ) { // SinaC 2003
    // Modified by SinaC 2001
    if ( ch->fighting == victim && is_affected( ch, gsn_energy_fist ) ) {
      int sk = get_ability( ch, gsn_energy_fist );
      dam = number_range( 1, ch->level/5 + sk );
      act("$N's {yenergy fist{x hurts $n.",victim,NULL,ch,TO_NOTVICT);
      act("You feel $N's {yenergy fist{x shocking you.",
	  victim,NULL,ch,TO_CHAR);
      act("Your {yenergy fist{x hurts $n.", victim, NULL, ch, TO_VICT );
      ability_damage( ch, victim, dam, gsn_energy_fist, DAM_ENERGY, FALSE, TRUE );
      shock_effect(victim,ch->level/2,dam,TARGET_CHAR);
    }
    
    if ( ch->fighting == victim && is_affected( ch, gsn_burning_fist ) ) {
      int sk = get_ability( ch, gsn_burning_fist );
      dam = number_range( ch->level/4, ch->level/3 + sk );
      act("$N's {rburning fist{x hurts $n.",victim,NULL,ch,TO_NOTVICT);
      act("You feel $N's {rburning fist{x burning you.",
	  victim,NULL,ch,TO_CHAR);
      act("Your {rburning fist{x hurts $n.", victim, NULL, ch, TO_VICT );
      ability_damage( ch, victim, dam, gsn_burning_fist, DAM_FIRE, FALSE, TRUE );
      fire_effect(victim,ch->level/2,dam,TARGET_CHAR);
    }

    if ( ch->fighting == victim && is_affected( ch, gsn_icy_fist ) ) {
      int sk = get_ability( ch, gsn_icy_fist );
      dam = number_range( ch->level/4, ch->level/3 + sk );
      act("$N's {Cicy fist{x hurts $n.",victim,NULL,ch,TO_NOTVICT);
      act("You feel $N's {Cicy fist{x freezing you.",
	  victim,NULL,ch,TO_CHAR);
      act("Your {Cicy fist{x hurts $n.", victim, NULL, ch, TO_VICT );
      ability_damage( ch, victim, dam, gsn_icy_fist, DAM_COLD, FALSE, TRUE );
      cold_effect(victim,ch->level/2,dam,TARGET_CHAR);
    }

    if ( ch->fighting == victim && is_affected( ch, gsn_acid_fist ) ) {
      int sk = get_ability( ch, gsn_acid_fist );
      dam = number_range( ch->level/4, ch->level/3 + sk );
      act("$N's {Gacid fist{x hurts $n.",victim,NULL,ch,TO_NOTVICT);
      act("You feel $N's {Gacid fist{x corroding you.",
	  victim,NULL,ch,TO_CHAR);
      act("Your {Gacid fist{x hurts $n.", victim, NULL, ch, TO_VICT );
      ability_damage( ch, victim, dam, gsn_acid_fist, DAM_ACID, FALSE, TRUE );
      acid_effect(victim,ch->level/2,dam,TARGET_CHAR);
    }

    if ( ch->fighting == victim && is_affected( ch, gsn_draining_fist ) ) {
      int sk = get_ability( ch, gsn_draining_fist );
      dam = number_range( ch->level/4, ch->level/3 + sk );
      act("$N's {Ddraining fist{x hurts $n.",victim,NULL,ch,TO_NOTVICT);
      act("You feel $N's {Ddraining fist{x drawing your life away.",
	  victim,NULL,ch,TO_CHAR);
      act("Your {Ddraining fist{x draws $n's life away.", victim, NULL, ch, TO_VICT );
      int done = ability_damage( ch, victim, dam, gsn_draining_fist, DAM_NEGATIVE, FALSE, TRUE );
      if ( done == DAMAGE_DONE ) { // Added by SinaC 2003
	ch->bstat(alignment) = UMAX(-1000,ch->bstat(alignment) - 1);
	ch->cstat(alignment) = UMAX(-1000,ch->cstat(alignment) - 1);
	ch->hit += dam/2;
      }
    }
  }
    
  /* but do we have a funky weapon? */
  if (result == DAMAGE_DONE && wield != NULL) { 
    additional_hit_affect( ch, victim, wield );
  }

  // Added by SinaC 2003 for fire/ice/stone/air shield
  //  for lightning field too
  if (ch->fighting == victim) {
    if (result == DAMAGE_DONE)	{
      if ( is_affected( victim, gsn_fireshield ) ) {
	dam = number_range(20, 50);
	ability_damage(victim, ch, dam, gsn_fireshield, DAM_FIRE, TRUE, TRUE );
      }
      if ( is_affected( victim, gsn_iceshield ) ) {
	dam = number_range(20, 50);
	ability_damage(victim, ch, dam, gsn_iceshield, DAM_COLD, TRUE, TRUE );
      }
      if ( is_affected( victim, gsn_stoneshield ) ) {
	dam = number_range(20, 50);
	ability_damage(victim, ch, dam, gsn_stoneshield, DAM_BASH, TRUE, TRUE );
      }
      if ( is_affected( victim, gsn_airshield ) ) {
	dam = number_range(20, 50);
	ability_damage(victim, ch, dam, gsn_airshield, DAM_LIGHTNING, TRUE, TRUE );
      }
      if ( is_affected( victim, gsn_lightning_field ) ) {
	dam = number_range(20, 50);
	ability_damage(victim, ch, dam, gsn_lightning_field, DAM_LIGHTNING, TRUE, TRUE );
      }
    }
  }
  return;
}

 
// Inflict damage from a hit, heavily modified by SinaC 1997 -> 2003.  
//  return value if an integer instead of bool, check fight.h for available value
int combat_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type,
		   bool show, OBJ_DATA *wield ) {
  if ( ch == NULL || !ch->valid ) {
    bug("damage with ch=NULL (vict:%s)",victim->name );
    return DAMAGE_NOTDONE;
  }

  if ( victim->position == POS_DEAD )
    return DAMAGE_NOTDONE;

  if ( dt < TYPE_HIT ) {// if ability damage: call ability_damage
    bug("combat_damage: ability_damage should be called [%s] [%s] [%d] [%d] [%d] [%s] [%s (vnum:%d)]",
	NAME(ch), NAME(victim), dam, dt, dam_type, show?"true":"false", 
	wield?wield->short_descr:"NONE", wield?wield->pIndexData->vnum:-1);
    ability_damage( ch, victim, dam, dt, dam_type, show, FALSE );
  }

//  OBJ_DATA *wield = NULL; // SinaC 2003
//  if ( dt >= TYPE_HIT ) { // get weapon only if damage are done by a weapon
//    switch( glob_whichWield ) {
//    case 0: wield = NULL;
//    case 1: wield = get_eq_char( ch, WEAR_WIELD ); break;
//    case 2: wield = get_eq_char( ch, WEAR_SECONDARY ); break;
//    case 3: wield = get_eq_char( ch, WEAR_THIRDLY ); break;
//    case 4: wield = get_eq_char( ch, WEAR_FOURTHLY ); break;
//    default:
//      wield = get_eq_char( ch, WEAR_WIELD ); 
//      bug("Invalid whichWield in one_hit [%d]", glob_whichWield );
//      break;
//    }
//  }

  // Stop up any residual loopholes.
  if ( dam > max_damage && dt >= TYPE_HIT) {
    // Modified by SinaC 2000
    bug( "Damage: %s did %d with %s (vnum %d)", 
	 ch->name, dam,
	 wield==NULL?"[Nothing!!]":wield->short_descr,
	 wield==NULL?-1:wield->pIndexData->vnum);
    dam = max_damage;
  }

  // damage reduction */
  if ( IS_NPC(ch) ) { // damage reduction only for mob
    if ( dam > 35)
      dam = (dam - 35)/2 + 35;
    if ( dam > 80)
      dam = (dam - 80)/2 + 80; 
  }

  if ( victim != ch ) {
    // Certain attacks are forbidden.
    // Most other attacks are returned.
    if ( is_safe( ch, victim ) )
      return DAMAGE_NOTDONE;
    check_killer( ch, victim );

    if ( victim->position > POS_PARALYZED ) {
      if ( victim->fighting == NULL )
	set_fighting( victim, ch );
      if (victim->timer <= 4)
	victim->position = POS_FIGHTING;
    }

    if ( victim->position > POS_PARALYZED ) {
      if ( ch->fighting == NULL )
	set_fighting( ch, victim );

//      // IF old_dam is TRUE, just initiate the fight but don't do any damage
//       * If victim is charmed, ch might attack victim's master.
//       taken out by Russ! */
//      if ( old_dam == TRUE             // old damage Sinac 1997
//	   &&   IS_NPC(ch)
//	   &&   IS_NPC(victim)
//	   &&   IS_AFFECTED(victim, AFF_CHARM)
//	   &&   victim->master != NULL
//	   &&   victim->master->in_room == ch->in_room
//	   &&   number_bits( 3 ) == 0 ) {
//	stop_fighting( ch, FALSE );
//	multi_hit( ch, victim->master, TYPE_UNDEFINED );
//	return DAMAGE_NOTDONE;
//      }
    }

    // More charm stuff.
    if ( victim->master == ch )
      stop_follower( victim );
  }

  // Inviso attacks ... not.
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) ) {
    affect_strip( ch, gsn_invis );
    affect_strip( ch, gsn_mass_invis );
   //  Only if invisible spell and not if item giving invisible flag
   //  SinaC 2000
    REMOVE_BIT( ch->bstat(affected_by), AFF_INVISIBLE );
    recompute(ch); // we force a recompute to check if ch is has an item giving invisible flag
    if ( !IS_AFFECTED(ch,AFF_INVISIBLE))
      act( "$n fades into existence.", ch, NULL, NULL, TO_ROOM );
  }

  // Damage modifiers.
  if ( dam > 1 && !IS_NPC(victim)
       &&   victim->pcdata->condition[COND_DRUNK]  > 10 )
    dam = 9 * dam / 10;

  if ( dam > 1 && IS_AFFECTED(victim, AFF_SANCTUARY) )
    dam /= 2;

  if ( dam > 1 && ((IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch) )
		   || (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch) )))
    dam -= dam / 4;

  // Added by SinaC 2003 for symbiote
  if ( dam > 1 && is_affected( victim, gsn_symbiote ) )
    dam -= (2*dam)/5;

  bool immune = FALSE;

  // Check for parry, and dodge.
  // can_see added by SinaC 2000,  blind test for parry, dodge and shield block
  // maybe we could set a small chance of dodging if blind
  // Modified by SinaC 2000 !ch->stunned
  // victim tries to avoid ch's hit
  //if ( dt >= TYPE_HIT &&     not needed, dt is always >= TYPE_HIT
  if ( ch != victim 
       // if can't see and npc and 33%, modified by SinaC 2001  was 25%
       && ( can_see( victim, ch ) 
	    //|| ( IS_NPC(victim) && number_percent() < 33 ) )
	    // Modified by SinaC 2001
	    || IS_NPC(victim) )
       && victim->stunned <= 0 ) {

    // Added by SinaC 2003
    //if ( check_shield_break( victim, ch, glob_secondary?dual_wield:wield ) )
    if ( check_shield_break( victim, ch, wield ) ) // SinaC 2003
      return DAMAGE_NOTDONE;

    // Modified by SinaC 2001 for object condition
    if ( check_parry( ch, victim ) ) { // victim's weapon  parries  ch's weapon
      OBJ_DATA *tmp = get_eq_char( victim, WEAR_WIELD );
      if ( tmp != NULL ) {
	check_damage_obj( victim, tmp, 2, dam_type );
	if ( wield != NULL )
	  check_damage_obj( ch, wield, 2, dam_type );
      }
      return DAMAGE_NOTDONE;
    }

    if ( check_dodge( ch, victim ) ) // victim  dodges
      return DAMAGE_NOTDONE;

    // Modified by SinaC 2001 for object condition
    if ( check_shield_block( ch, victim ) ) { // victim's shield  block  ch's weapon
      OBJ_DATA *shield = get_eq_char( victim, WEAR_SHIELD );
      if ( shield != NULL )
	check_damage_obj( victim, shield, 10, dam_type );
      if ( wield != NULL )
	check_damage_obj( ch, wield, 10, DAM_NONE );
      return DAMAGE_NOTDONE;
    }

    // Added by SinaC 2000
    if ( check_fade( ch, victim ) ) // victim  fades
      return DAMAGE_NOTDONE;

    // Added by SinaC 2003
    if ( check_dual_parry( ch, victim ) ) { // victim's 2nd weapon  parries  ch's weapon
      OBJ_DATA *tmp = get_eq_char( victim, WEAR_SECONDARY );
      if ( tmp != NULL ) {
	check_damage_obj( victim, tmp, 2, dam_type );
	if ( wield != NULL )
	  //check_damage_obj( ch, dual_wield, 2, dam_type );  Oops
	  check_damage_obj( ch, wield, 2, dam_type );
      }
      return DAMAGE_NOTDONE;
    }

    // Added by SinaC 2003
    if ( check_evasion( ch, victim ) ) // victim  evades
      return DAMAGE_NOTDONE;

    // SinaC 2003
    if ( check_tumble( ch, victim ) ) // victim  tumbles
      return DAMAGE_NOTDONE;

    // SinaC 2003
    if ( check_deflect( ch, victim ) ) // victim  deflects
      return DAMAGE_NOTDONE;

    // SinaC 2003, block will prevent serious damage to be done
    int dam_modifier;
    if ( check_block( ch, victim, dam, dam_modifier ) ) // victim  blocks
      dam -= dam_modifier;
  }

//  // Added by SinaC 2003, skills/affects allowing to avoid magic damage
//  if ( dt < TYPE_HIT 
//       && ch != victim  ) {
//
//    // check if affected by Justice and taking Negative damage, counter the attack
//    if ( dam_type == DAM_NEGATIVE 
//	 && is_affected( victim, gsn_justice ) ) {
//      damage( victim, ch, dam, gsn_justice, DAM_HOLY,
//	      TRUE, TRUE );
//      return DAMAGE_NOTDONE;
//    }
//
//    // check if affected by Corruption and taking Holy damage, counter the attack
//    if ( dam_type == DAM_HOLY
//	 && is_affected( victim, gsn_justice ) ) {
//      damage( victim, ch, dam, gsn_corruption, DAM_NEGATIVE,
//	      TRUE, TRUE );
//      return DAMAGE_NOTDONE;
//    }
//  }  

  // Added by SinaC 2001
  if ( check_armor_absorb( victim, ch ) ) // victim's armor affect  absorbs  ch's attack
    return DAMAGE_NOTDONE;

  // check_absorb, weapon damage and random victim item damage moved after check_immune & check_critical

  switch(check_immune(victim,dam_type)) {
  case(IS_IMMUNE):
    immune = TRUE;
    dam = 0;
    break;
  case(IS_RESISTANT):
    dam -= dam/3; // remove 1/3 dam if resistant
    break;
  case(IS_VULNERABLE):
    // Modified by SinaC 2001
    //dam += dam/2;
    dam += dam; // double damage if vulnerable
    break;
  }

  // Added by SinaC 2000
  if ( dam != 0 
       && dt >= TYPE_HIT && ch != victim )
    //if ( check_critical(ch,victim) )
    if ( check_critical(ch,victim, wield ) )
      dam += ( dam * 12 ) / 10;

  // Moved by SinaC 2003, was before check_immune before
  // Added by SinaC 2001
  //if ( check_shroud( victim, ch ) ) // victim's shroud affect  absorbs  ch's attack
  if ( check_shroud( victim, ch, dam ) ) // victim's shroud affect  absorbs  ch's attack
    return DAMAGE_NOTDONE;

  // Added by SinaC 2001 for object condition
  check_damage_obj( victim, NULL, 2, dam_type );  // damage a random victim's item

  // Modified by SinaC 2001
  if ( wield != NULL  // damage ch's weapon
       //&& dt >= TYPE_HIT ) { // shooting arrows won't lower bow condition  dt always >= TYPE_HIT
       ) {
    check_damage_obj( ch, wield, 2, DAM_NONE );
  
    // Added by SinaC 2001, so VULN_SILVER, VULN_WOOD, ... is used.
    // not coded for the moment, should be better to add a list
    //  of forbidden materials
    //    switch(check_immune_material(victim,wield)) {
    //    case(IS_IMMUNE):
    //      immune = TRUE;
    //      dam = 0;
    //      break;
    //    case(IS_RESISTANT):
    //      dam -= dam/3;
    //      break;
    //    case(IS_VULNERABLE):
    //      // Modified by SinaC 2001
    //      //dam += dam/2;
    //      dam += dam;
    //      break;
    //    }
  }
  // end of moved code

  // Added by SinaC 2001  for mistform disadvantage, can't do damage
  if ( is_affected( ch, gsn_mistform ) )
    return DAMAGE_NOTDONE;

  if (show)
    dam_message( ch, victim, dam, dt, immune, TRUE );

  if (dam == 0)
    return DAMAGE_NOTDONE;

  // Claws damage and bite skill cause lycanthropy to propagate
  check_lycanthropy( ch, victim, dt, wield );

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

  // Sleep spells and extremely wounded folks, and paralyzed folks
  if ( !IS_AWAKE(victim) )
    stop_fighting( victim, FALSE );

  // Payoff for killing things.    FROM HERE, we are sure VICTIM is DEAD, SinaC 2003
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



///* Modified by Sinac 1997 for Battle */
bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim) {
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (IS_IMMORTAL(ch))
    return FALSE;

  // Added by SinaC 2000
  if (!IS_NPC(ch) && IS_SET(ch->comm,COMM_AFK)) {
    send_to_char( "You are {GAFK!{x\n\r", ch );
    return TRUE;
  }
  if (!IS_NPC(ch) && IS_SET(ch->comm,COMM_BUILDING)) {
    send_to_char( "You are {bBUILDING{x!\n\r", ch );
    return TRUE;
  }
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (!IS_NPC(ch) && IS_SET(ch->comm,COMM_EDITING)) {
    send_to_char( "You are {BEDITING{x!\n\r", ch );
    return TRUE;
  }

  /* safe or bank room? */
  // Modified by SinaC 2001
  if ( IS_SET(victim->in_room->cstat(flags),ROOM_SAFE) 
       || IS_SET(victim->in_room->cstat(flags),ROOM_BANK) ) {
    send_to_char("Not in this room.\n\r",ch);
    return TRUE;
  }

  // SinaC 2003, killer npc can't be safe
  if ( IS_NPC(ch) &&
       IS_SET(ch->act, ACT_IS_SAFE) ) {
    send_to_charf(ch,"You can't attack anyone.\n\r");
    return TRUE;
  }

  /* killing mobiles */
  if (IS_NPC(victim)) {
    if ( IS_SET(victim->act, ACT_IS_SAFE) ) {
      send_to_charf( ch, 
		     "I don't think %s would approve.\n\r",
		     char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
      return TRUE;
    }

    if (victim->pIndexData->pShop != NULL) {
      send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
      return TRUE;
    }

    /* no killing healers, trainers, etc */
    if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	// Removed by SinaC 2003
	//||  IS_SET(victim->act,ACT_IS_CHANGER)
	// neither questor  SinaC 2000
	//	||  victim->spec_fun == spec_lookup( "spec_questmaster" ) ) {
	) {
      // Modified by SinaC 2001 for god
      send_to_charf( ch, 
		     "I don't think %s would approve.\n\r",
		     char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
      return TRUE;
    }

    // Added by SinaC 2001
    if ( IS_NPC(ch) 
	 && ch->pIndexData->group == victim->pIndexData->group 
	 && ch->pIndexData->group != 0 )
      return TRUE;

// Removed by SinaC 2001
//    if (!IS_NPC(ch)) {
      /* no pets */
      if (IS_SET(victim->act,ACT_PET)) {
	send_to_charf(ch,"But %s looks so cute and cuddly...\n\r",
		      NAME(victim));
	return TRUE;
      }

      /* Removed by SinaC 2000
	// no charmed creatures unless owner
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)  {
	send_to_char("You don't own that monster.\n\r",ch);
	return TRUE;
	}
      */
      if (IS_AFFECTED(victim,AFF_CHARM) ) {
	send_to_charf(ch,"But %s is so loyal...\n\r", 
		      NAME(victim));
	return TRUE;
      }
//    }
  }
  /* killing players */
  else {
    /* NPC doing the killing */
    if (IS_NPC(ch)) {
      // Modified by SinaC 2001
      // Now pets and charmies are allowed to start a fight with a player
      /*
      // charmed mobs and pets cannot attack players while owned
      if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	  &&  ch->master->fighting != victim) {
	send_to_char("Players are your friends!\n\r",ch);
	return TRUE;
      }
      */
      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL ) {
	if (IS_NPC(ch->master))
	  return TRUE;

	if (ch->master == victim)
	  return TRUE;

	if (IN_BATTLE(ch->master))
	  return FALSE;

	// Stupid code !!!! why a mob could not attack a player in a clan ? FIXME
	if (!ch->master->clan || !victim->clan) {
	  if (!ch->master->clan)
	    send_to_char("Join a clan, get loner, or go to the battle field, if you want to kill/steal players.\n\r",ch->master);
	  else
	    send_to_char("Your victim is not in a clan!\n\r",ch->master);
	  return TRUE;
	} 
	// Added by SinaC 2000
	else {
	  // Added by SinaC 2001
	  if ( ch->master->clan == victim->clan ) {
	    send_to_charf(ch->master,"You are in the same clan as %s.\n\r", NAME(victim) );
	    return TRUE;
	  }
	  int diff = ch->master->level - victim->level;
	  if ( diff < -8 || diff > 8 ) {
	    send_to_char("Take someone in your range.\n\r", ch->master );
	    return TRUE;
	  }
	}
      }
    }
    /* player doing the killing */
    else {
      // SinaC 2000 for Arena
      if (IN_BATTLE(ch))
	return FALSE;
      if (!ch->clan || !victim->clan) {
	if (!ch->clan)
	  send_to_char("Join a clan, get loner, or go to the battle field, if you want to kill/steal players.\n\r",ch);
	else
	  send_to_char("Your victim is not in a clan!\n\r",ch);
	return TRUE;
      }
      // Added by SinaC 2000
      else {
	// Added by SinaC 2001
	if ( ch->clan == victim->clan ) {
	  send_to_charf(ch,"You are in the same clan as %s.\n\r", NAME(victim) );
	  return TRUE;
	}
	int diff = ch->level - victim->level;
	if ( diff < -8 || diff > 8 ) {
	  send_to_char("Take someone in your range.\n\r", ch );
	  return TRUE;
	}
      }
    }
  }
  return FALSE;
}

// Same function as is_safe but without any output
bool silent_is_safe( CHAR_DATA *ch, CHAR_DATA *victim ) { 
  // copy/paste of is_safe function but removed send_to_char
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (IS_IMMORTAL(ch))
    return FALSE;

  // Added by SinaC 2000
  if (!IS_NPC(ch) && IS_SET(ch->comm,COMM_AFK))
    return TRUE;
  if (!IS_NPC(ch) && IS_SET(ch->comm,COMM_BUILDING))
    return TRUE;
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (!IS_NPC(ch) && IS_SET(ch->comm,COMM_EDITING))
    return TRUE;

  /* safe or bank room? */
  // Modified by SinaC 2001
  if ( IS_SET(victim->in_room->cstat(flags),ROOM_SAFE) 
       || IS_SET(victim->in_room->cstat(flags),ROOM_BANK) )
    return TRUE;

  // SinaC 2003, killer npc can't be safe
  if ( IS_NPC(ch) &&
       IS_SET(ch->act, ACT_IS_SAFE) )
    return TRUE;

  /* killing mobiles */
  if (IS_NPC(victim)) {
    if ( IS_SET(victim->act, ACT_IS_SAFE) )
      return TRUE;

    if (victim->pIndexData->pShop != NULL)
      return TRUE;

    /* no killing healers, trainers, etc */
    if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	// Removed by SinaC 2003
	//||  IS_SET(victim->act,ACT_IS_CHANGER)
	// neither questor  SinaC 2000
	//	||  victim->spec_fun == spec_lookup( "spec_questmaster" ) )
	)
      // Modified by SinaC 2001 for god
      return TRUE;

    // Added by SinaC 2001
    if ( IS_NPC(ch) 
	 && ch->pIndexData->group == victim->pIndexData->group 
	 && ch->pIndexData->group != 0 )
      return TRUE;

// Removed by SinaC 2001
//    if (!IS_NPC(ch)) {
      /* no pets */
      if (IS_SET(victim->act,ACT_PET))
	return TRUE;

      /* Removed by SinaC 2000
	// no charmed creatures unless owner
	if (IS_AFFECTED(victim,AFF_CHARM) && ch != victim->master)  {
	send_to_char("You don't own that monster.\n\r",ch);
	return TRUE;
	}
      */
      if (IS_AFFECTED(victim,AFF_CHARM) )
	return TRUE;
//    }
  }
  /* killing players */
  else {
    /* NPC doing the killing */
    if (IS_NPC(ch)) {
      // Modified by SinaC 2001
      // Now pets and charmies are allowed to start a fight with a player
      /*
      // charmed mobs and pets cannot attack players while owned
      if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	  &&  ch->master->fighting != victim) {
	send_to_char("Players are your friends!\n\r",ch);
	return TRUE;
      }
      */
      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL ) {
	if (IS_NPC(ch->master))
	  return TRUE;

	if (ch->master == victim)
	  return TRUE;

	if (IN_BATTLE(ch->master))
	  return FALSE;

	// Stupid code !!!! why a mob could not attack a player in a clan ? FIXME
	if (!ch->master->clan || !victim->clan)
	  return TRUE;
	// Added by SinaC 2000
	else {
	  // Added by SinaC 2001
	  if ( ch->master->clan == victim->clan )
	    return TRUE;
	  int diff = ch->master->level - victim->level;
	  if ( diff < -8 || diff > 8 )
	    return TRUE;
	}
      }
    }
    /* player doing the killing */
    else {
      // SinaC 2000 for Arena
      if (IN_BATTLE(ch))
	return FALSE;
      if (!ch->clan || !victim->clan)
	return TRUE;
      // Added by SinaC 2000
      else {
	// Added by SinaC 2001
	if ( ch->clan == victim->clan )
	  return TRUE;
	int diff = ch->level - victim->level;
	if ( diff < -8 || diff > 8 )
	  return TRUE;
      }
    }
  }
  return FALSE;
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area ) {
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim == ch && area)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (IS_IMMORTAL(ch) && !area)
    return FALSE;
      
  /* safe or bank room? */
  // Modified by SinaC 2001
  if ( IS_SET(victim->in_room->cstat(flags),ROOM_SAFE) 
       || IS_SET(victim->in_room->cstat(flags),ROOM_BANK) ){
    send_to_char("Not in this room.\n\r",ch);
    return TRUE;
  }
  
  /* killing mobiles */
  if (IS_NPC(victim)) {

    if ( IS_SET(victim->act, ACT_IS_SAFE) ) {
      send_to_charf( ch, 
		     "I don't think %s would approve.\n\r",
		     char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
      return TRUE;
    }

    if (victim->pIndexData->pShop != NULL){
      send_to_char("The shopkeeper wouldn't like that.\n\r",ch);
      return TRUE;
    }

    /* no killing healers, trainers, etc */
    if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	// Removed by SinaC 2003
	//||  IS_SET(victim->act,ACT_IS_CHANGER)
	// Added by SinaC 2000
	//	||  victim->spec_fun == spec_lookup( "spec_questmaster" )){
	){
      // Modified by SinaC 2001 for god
      send_to_charf( ch, 
		     "I don't think %s would approve.\n\r",
		     char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
      return TRUE;
    }

    // Added by SinaC 2001
    if ( IS_NPC(ch)
	 && ch->pIndexData->group != 0 // SinaC 2003
	 && ch->pIndexData->group == victim->pIndexData->group )
      return TRUE;

    //    if (!IS_NPC(ch)) {
    /* no pets */
    if (IS_SET(victim->act,ACT_PET)){
	act("But $N looks so cute and cuddly...",
	    ch,NULL,victim,TO_CHAR);
	return TRUE;
    }
    
    /*
      // no charmed creatures unless owner
      if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master)){
      send_to_char("You don't own that monster.\n\r",ch);
      return TRUE;
      }
    */
    if (IS_AFFECTED(victim,AFF_CHARM) ) {
      act("But $N is so loyal...", ch, NULL, victim, TO_CHAR );
      return TRUE;
    }
    
    /* legal kill? -- cannot hit mob fighting non-group member */
    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
      return TRUE;

    if ( IS_NPC(ch) )
      /* area effect spells do not hit other mobs */
      if (area && !is_same_group(victim,ch->fighting))
	return TRUE;
  }
  /* killing players */
  else {
    if (area && IS_IMMORTAL(victim))
      return TRUE;

    /* NPC doing the killing */
    if (IS_NPC(ch)) {
      // Modified by SinaC 2001
      /*
      // charmed mobs and pets cannot attack players while owned
      if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	  &&  ch->master->fighting != victim)
	return TRUE;
      */
      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL ) {
	if (IS_NPC(ch->master))
	  return TRUE;

	if (ch->master == victim)
	  return TRUE;
	
	if (IN_BATTLE(ch->master))
	  return FALSE;
	
	if (!ch->master->clan || !victim->clan)
	  return TRUE;
	// Added by SinaC 2000
	else {
	  // Added by SinaC 2001
	  if ( ch->clan == victim->clan )
	    return TRUE;
	  int diff = ch->master->level - victim->level;
	  if ( diff < -8 || diff > 8 ) {
	    return TRUE;
	  }
	}
      }
      // legal kill? -- mobs only hit players grouped with opponent*/
      if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
	return TRUE;
    }

    /* player doing the killing */
    else {    
      if (IN_BATTLE(ch))
	return FALSE;

      if (!ch->clan || !victim->clan) 
	return TRUE;
      // Added by SinaC 2000
      else {
	// Added by SinaC 2001
	if ( ch->clan == victim->clan )
	  return TRUE;
	int diff = ch->level - victim->level;
	if ( diff < -5 || diff > 10 )
	  return TRUE;
      }
    }

  }
  return FALSE;
}

bool silent_is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area ) {
  if (victim->in_room == NULL || ch->in_room == NULL)
    return TRUE;

  if (victim == ch && area)
    return TRUE;

  if (victim->fighting == ch || victim == ch)
    return FALSE;

  if (IS_IMMORTAL(ch) && !area)
    return FALSE;
      
  /* safe or bank room? */
  // Modified by SinaC 2001
  if ( IS_SET(victim->in_room->cstat(flags),ROOM_SAFE) 
       || IS_SET(victim->in_room->cstat(flags),ROOM_BANK) )
    return TRUE;
  
  /* killing mobiles */
  if (IS_NPC(victim)) {

    if ( IS_SET(victim->act, ACT_IS_SAFE) )
      return TRUE;

    if (victim->pIndexData->pShop != NULL)
      return TRUE;

    /* no killing healers, trainers, etc */
    if (IS_SET(victim->act,ACT_TRAIN)
	||  IS_SET(victim->act,ACT_PRACTICE)
	||  IS_SET(victim->act,ACT_IS_HEALER)
	// Removed by SinaC 2003
	//||  IS_SET(victim->act,ACT_IS_CHANGER)
	// Added by SinaC 2000
	//	||  victim->spec_fun == spec_lookup( "spec_questmaster" ))
	)
      // Modified by SinaC 2001 for god
      return TRUE;

    // Added by SinaC 2001
    if ( IS_NPC(ch) 
	 && ch->pIndexData->group == victim->pIndexData->group )
      return TRUE;

    //    if (!IS_NPC(ch)) {
    /* no pets */
    if (IS_SET(victim->act,ACT_PET))
	return TRUE;
    
    /*
      // no charmed creatures unless owner
      if (IS_AFFECTED(victim,AFF_CHARM) && (area || ch != victim->master)){
      send_to_char("You don't own that monster.\n\r",ch);
      return TRUE;
      }
    */
    if (IS_AFFECTED(victim,AFF_CHARM) )
      return TRUE;
    /* legal kill? -- cannot hit mob fighting non-group member */
    if (victim->fighting != NULL && !is_same_group(ch,victim->fighting))
      return TRUE;

    if ( IS_NPC(ch) )
      /* area effect spells do not hit other mobs */
      if (area && !is_same_group(victim,ch->fighting))
	return TRUE;
  }
  /* killing players */
  else {
    if (area && IS_IMMORTAL(victim))
      return TRUE;

    /* NPC doing the killing */
    if (IS_NPC(ch)) {
      // Modified by SinaC 2001
      /*
      // charmed mobs and pets cannot attack players while owned
      if (IS_AFFECTED(ch,AFF_CHARM) && ch->master != NULL
	  &&  ch->master->fighting != victim)
	return TRUE;
      */
      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master != NULL ) {
	if (IS_NPC(ch->master))
	  return TRUE;

	if (ch->master == victim)
	  return TRUE;
	
	if (IN_BATTLE(ch->master))
	  return FALSE;
	
	if (!ch->master->clan || !victim->clan)
	  return TRUE;
	// Added by SinaC 2000
	else {
	  // Added by SinaC 2001
	  if ( ch->clan == victim->clan )
	    return TRUE;
	  int diff = ch->master->level - victim->level;
	  if ( diff < -8 || diff > 8 ) {
	    return TRUE;
	  }
	}
      }
      // legal kill? -- mobs only hit players grouped with opponent*/
      if (ch->fighting != NULL && !is_same_group(ch->fighting,victim))
	return TRUE;
    }

    /* player doing the killing */
    else {    
      if (IN_BATTLE(ch))
	return FALSE;

      if (!ch->clan || !victim->clan) 
	return TRUE;
      // Added by SinaC 2000
      else {
	// Added by SinaC 2001
	if ( ch->clan == victim->clan )
	  return TRUE;
	int diff = ch->level - victim->level;
	if ( diff < -5 || diff > 10 )
	  return TRUE;
      }
    }

  }
  return FALSE;
}


/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];

  /*
   * Follow charm thread to responsible character.
   * Attacking someone's charmed char is hostile!
   */
  while ( IS_AFFECTED(victim, AFF_CHARM) && victim->master != NULL )
    victim = victim->master;

  /*
   * NPC's are fair game.
   * So are killers and thieves.
   */
  if ( IS_NPC(victim)
       ||   IS_SET(victim->act, PLR_KILLER)
       ||   IS_SET(victim->act, PLR_THIEF))
    return;

  /*
   * Charm-o-rama.
   */

  if ( IS_SET(ch->cstat(affected_by), AFF_CHARM) ) {
    if ( ch->master == NULL )	{
      char buf[MAX_STRING_LENGTH];

      sprintf( buf, "Check_killer: %s bad AFF_CHARM",
	       IS_NPC(ch) ? ch->short_descr : ch->name );
      bug( buf, 0 );
      affect_strip( ch, gsn_charm_person );
      REMOVE_BIT( ch->cstat(affected_by), AFF_CHARM );
      return;
    }
    // Modified by SinaC 2001
    if ( !IS_SET(ch->master->act, PLR_KILLER ) ) {
      send_to_char( "*** You are now a KILLER!! ***\n\r", ch->master );
      SET_BIT(ch->master->act, PLR_KILLER);
    }
    
    // Comment by SinaC 2000
    /*
      stop_follower( ch );
      return;
    */
  }

  /*
   * NPC's are cool of course (as long as not charmed).
   * Hitting yourself is cool too (bleeding).
   * So is being immortal (Alander's idea).
   * Oxtal -- Cool also 2 opposite clans with level range
   *       -- An when you're in battle
   * And current killers stay as they are.
   */
  if ( IS_NPC(ch)
       || IN_BATTLE(ch)
       || ch == victim
       || IS_IMMORTAL(ch) 
       /* it was !is_same_clan before         SinaC 2000 */
       || (ch->clan && victim->clan && is_same_clan(ch,victim) && ch->level <= victim->level + 8)
       || IS_SET(ch->act, PLR_KILLER) 
       || ch->fighting == victim)
    return;

  send_to_char( "*** You are now a KILLER!! ***\n\r", ch );
  SET_BIT(ch->act, PLR_KILLER);
  sprintf(buf,"$N is attempting to murder %s",victim->name);
  wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
  //save_char_obj( ch );
  new_save_pFile(ch,FALSE);
  return;
}

// Added by SinaC 2000
/*
 * Check for restless spirits of dead NPC
 */
void check_spirit( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *spirit;

  if ( ch == NULL || !ch->valid )
    return;

  /* only happens 1 in 100 times and only to NPCs */
  if ( number_range(0,100) != 0 || !IS_NPC(victim) || ch->level <= 10 )
    return;
  
  spirit = create_mobile( victim->pIndexData );
  SET_BIT ( spirit->bstat(form),
	    FORM_INSTANT_DECAY|FORM_UNDEAD|FORM_INTANGIBLE );
  SET_BIT ( spirit->act, ACT_AGGRESSIVE );
  SET_BIT ( spirit->bstat(affected_by), AFF_PASS_DOOR );

  sprintf(buf,"the spirit of %s",victim->short_descr);
  spirit->short_descr = str_dup(buf);

  sprintf(buf,"The spirit of %s",victim->long_descr);
  spirit->long_descr = str_dup(buf);

  sprintf(buf,"spirit %s",victim->name);
  spirit->name = str_dup(buf);

  // Removed by SinaC 2001
  // Added by SinaC 2000, the spirit will now haunt his/her killer
  //spirit->hunting = ch;

  //recompute( spirit ); NO NEED: done in char_to_room
    
  char_to_room( spirit, ch->in_room );

  act("\n\rYou cower in fear as {y$N{x appears before you!",ch,NULL,spirit,TO_CHAR);
  act("{y$N{x suddenly appears and attacks $n!",ch,NULL,spirit,TO_ROOM);

  // Maybe we could wait 1*PULSE_VIOLENCE before multi_hit ?? SinaC 2000

  multi_hit( spirit, ch, TYPE_UNDEFINED );

  return;
}
/*
 * Check for critical strike. 
 */
bool check_critical(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield ) {
  //OBJ_DATA *obj;
  //obj = get_eq_char(ch,WEAR_WIELD);
  
  int chance = get_ability(ch,gsn_critical_strike);
  if ( wield == NULL //get_eq_char(ch,WEAR_WIELD) == NULL 
       || chance <= 0 
       //|| get_weapon_ability(ch,get_weapon_sn(ch,FALSE) )  !=  100 
       || get_weapon_ability(ch,get_weapon_sn(ch,wield) )  !=  100 
       || number_range(0,100) > chance//get_ability(ch,gsn_critical_strike)
       // Added by SinaC 2000
       || !can_see( ch, victim ) )
    return FALSE;

  if ( number_range(0,100) > 25 ) // 25% chance sucess
    return FALSE;
  
  /* Now, if it passed all the tests... */
  
  //act("$p critically strikes $n!",victim,obj,NULL,TO_NOTVICT);
  //act("$p critically strikes you!",ch,obj,victim,TO_VICT);
  act("$p critically strikes $n!",victim,wield,NULL,TO_NOTVICT);
  act("$p critically strikes you!",ch,wield,victim,TO_VICT);
  check_improve(ch,gsn_critical_strike,TRUE,6);
  return TRUE;
}

/*
 * Check for counter.
 */
//bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, bool secondary ) {
bool check_counter( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int whichWield ) {
  int chance;
  int dam_type;
  OBJ_DATA *wield;

  wield = get_eq_char(victim,WEAR_WIELD);
  chance = get_ability(victim,gsn_counter);
  if ( wield == NULL
       || !IS_AWAKE(victim) 
       || chance <= 0
       || !can_see(victim,ch) )
    return FALSE;

  chance /= 6;
  chance += ( victim->level - ch->level ) / 2;

  chance += 2 * (victim->cstat(DEX)-ch->cstat(DEX));

  //chance += get_weapon_ability(victim,get_weapon_sn(victim,secondary)) -
  //get_weapon_ability(ch,get_weapon_sn(ch,secondary) );
  chance += get_weapon_ability(victim,get_weapon_sn(victim,1)) - // primary wield will counter
    get_weapon_ability(ch,get_weapon_sn(ch,whichWield));

  chance += victim->cstat(STR) - ch->cstat(STR);

  // Added by SinaC 2000, modified by SinaC 2001 before: 10+UMAX( 0, chance*2 )
  if ( IS_NPC(ch) )
    chance = UMAX( 0, 20+chance );

  if ( number_percent() >= chance )
    return FALSE;

  dt = gsn_counter;

  if ( dt == TYPE_UNDEFINED ){
    dt = TYPE_HIT;
    if ( wield != NULL && wield->item_type == ITEM_WEAPON ) {
      dt += GET_WEAPON_DAMTYPE(wield);
      //dt += wield->value[3];
    }
    else 
      //dt += ch->dam_type;
      dt += ch->cstat(dam_type);
  }

  if (dt < TYPE_HIT)
    if (wield != NULL) {
      dam_type = attack_table[GET_WEAPON_DAMTYPE(wield)].damage;
      //dam_type = attack_table[wield->value[3]].damage;
    }
    else
      //dam_type = attack_table[ch->dam_type].damage;
      dam_type = attack_table[ch->cstat(dam_type)].damage;
  else
    dam_type = attack_table[dt - TYPE_HIT].damage;
  
  if (dam_type == -1)
    dam_type = DAM_BASH;
  
  act( "You reverse $n's attack and counter with your own!", ch, NULL, victim, TO_VICT    );
  act( "$N reverses your attack!", ch, NULL, victim, TO_CHAR    );
  
  // Modified by SinaC 2000
  ability_damage( victim, ch, dam/2, gsn_counter, dam_type, TRUE, FALSE );
  
  check_improve(victim,gsn_counter,TRUE,6);
  
  return TRUE;
}

/*
 * Check for fade.
 */
bool check_fade( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance = 0;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  // Added by SinaC 2003, mount code
  if ( IS_NPC(victim)
       && !IS_SET( victim->off_flags, OFF_FADE )
       && get_mount_master(victim)
       && is_affected( victim, gsn_invincible_mount ) )
    chance = 33;
  else
    chance = get_ability(victim,gsn_fade) / 2;

  if ( chance == 0 )
    return FALSE;

  if (!can_see(victim,ch))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level; SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  act( "Your body fades to avoid $n's attack.", ch, NULL, victim, TO_VICT    );
  act( "$N's body fades to avoid your attack.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_fade,TRUE,6);
  return TRUE;
}


/*
 * Check for parry.
 */
bool check_parry( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_parry) / 2;

  if ( get_eq_char( victim, WEAR_WIELD ) == NULL ){
    if (IS_NPC(victim))
      chance /= 2;
    else
      return FALSE;
  }

  if (!can_see(ch,victim))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level;
    chance = URANGE( 5, chance + victim->level - ch->level, 95 ); // SinaC 2003

  if ( number_percent( ) >= chance )
    return FALSE;

  act( "You parry $n's attack.",  ch, NULL, victim, TO_VICT    );
  act( "$N parries your attack.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_parry,TRUE,6);
  return TRUE;
}

/*
 * Check for shield block.
 */
bool check_shield_block( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  if ( get_eq_char( victim, WEAR_SHIELD ) == NULL )
    return FALSE;

  //chance = get_ability(victim,gsn_shield_block) / 5 + 3;
  chance = get_ability(victim,gsn_shield_block) / 5;
  if ( chance == 0 ) 
    return FALSE;
  chance += 3; 

  // Added by SinaC 2001
  if (!can_see(ch,victim))
    chance /= 2;

  // Modified by SinaC 2000
  if ( chance > 0 ) 
    //chance += victim->level - ch->level; SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  act( "You block $n's attack with your shield.",  ch, NULL, victim, TO_VICT    );
  act( "$N blocks your attack with a shield.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_shield_block,TRUE,6);
  return TRUE;
}


/*
 * Check for dodge.
 */
bool check_dodge( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_dodge) / 2;

  if ( chance == 0 )
    return FALSE;

  if (!can_see(victim,ch))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level;  SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  act( "You dodge $n's attack.", ch, NULL, victim, TO_VICT    );
  act( "$N dodges your attack.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_dodge,TRUE,6);
  return TRUE;
}


/* Added by SinaC 2003
 * Check for dual parry.
 */
bool check_dual_parry( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_dual_parry) / 4;

  if ( chance == 0 )
    return FALSE;

  if ( get_eq_char( victim, WEAR_SECONDARY ) == NULL ){
    if (IS_NPC(victim))
      chance /= 2;
    else
      return FALSE;
  }

  if (!can_see(ch,victim))
    chance /= 2;

  if ( chance > 0 )
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  //if ( number_percent( ) >= chance + victim->level - ch->level ) SinaC 2003
  if ( number_percent( ) >= chance )
    return FALSE;

  act( "You parry $n's attack with your off-hand.",  ch, NULL, victim, TO_VICT    );
  act( "$N parries your attack with $s off-hand.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_dual_parry,TRUE,6);
  return TRUE;
}
// check for evasion
bool check_evasion( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_evasion) / 2;

  if ( chance == 0 )
    return FALSE;

  if (!can_see(victim,ch))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level; SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  act( "You evade $n's attack.", ch, NULL, victim, TO_VICT    );
  act( "$N evades your attack.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_evasion,TRUE,6);
  return TRUE;
}

bool check_tumble( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_tumble) / 2;

  if ( chance == 0 )
    return FALSE;

  if (!can_see(victim,ch))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level; SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  //  if ( !is_unarmed_and_unarmored(ch) )
  if ( !IS_UNARMED_UNARMORED(victim) ) // SinaC 2003
    return FALSE;

  act( "You drop and roll to evade $n's attack.", ch, NULL, victim, TO_VICT    );
  act( "$N drops and roll to evade your attack.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_tumble,TRUE,6);
  return TRUE;
}

bool check_block( CHAR_DATA *ch, CHAR_DATA *victim, const int dam, int &dam_modifier ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_block) / 2;

  if ( chance == 0 )
    return FALSE;

  if (!can_see(victim,ch))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level; SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  //  if ( !is_unarmed_and_unarmored(ch) )
  if ( !IS_UNARMED_UNARMORED(victim) ) // SinaC 2003
    return FALSE;

  if ( number_percent() >= 50 ) {
    act( "You block $n's attack with your leg.", ch, NULL, victim, TO_VICT    );
    act( "$N blocks your attack with $S leg.", ch, NULL, victim, TO_CHAR    );
  }
  else {
    act( "You block $n's attack with your arm.", ch, NULL, victim, TO_VICT    );
    act( "$N blocks your attack with $S arm.", ch, NULL, victim, TO_CHAR    );
  }
  dam_modifier = (3*dam)/4; // prevent 75% of damage
  check_improve(victim,gsn_block,TRUE,6);
  return TRUE;
}

bool check_deflect( CHAR_DATA *ch, CHAR_DATA *victim ) {
  int chance;

  if ( !IS_AWAKE(victim) )
    return FALSE;

  chance = get_ability(victim,gsn_deflect) / 2;

  if ( chance == 0 )
    return FALSE;

  if (!can_see(victim,ch))
    chance /= 2;

  if ( chance > 0 )
    //chance += victim->level - ch->level; SinaC 2003
    chance = URANGE( 5, chance + victim->level - ch->level, 95 );

  if ( number_percent( ) >= chance )
    return FALSE;

  //  if ( !is_unarmed_and_unarmored(ch) )
  if ( !IS_UNARMED_UNARMORED(victim) ) // SinaC 2003
    return FALSE;

  act( "You deflect $n's attack with your hand.", ch, NULL, victim, TO_VICT    );
  act( "$N deflect your attack with $S hand.", ch, NULL, victim, TO_CHAR    );
  check_improve(victim,gsn_deflect,TRUE,6);
  return TRUE;
}



/*
 * Set position of a victim.
 */
void update_pos( CHAR_DATA *victim ) {
  if ( victim->hit > 0 ) {
    //if ( victim->position <= POS_STUNNED )  //Modified by SinaC 2003
    if ( victim->position <= POS_STUNNED && victim->position != POS_PARALYZED )
      victim->position = POS_STANDING;
    return;
  }

  if ( IS_NPC(victim) && victim->hit < 1 ) {
    victim->position = POS_DEAD;
    return;
  }

  if ( victim->hit <= -11 ) {
    victim->position = POS_DEAD;
    return;
  }

  if ( victim->hit <= -6 ) victim->position = POS_MORTAL;
  else if ( victim->hit <= -3 ) victim->position = POS_INCAP;
  else                          victim->position = POS_STUNNED;

  return;
}

/*
 * Start fights.
 */
void set_fighting( CHAR_DATA *ch, CHAR_DATA *victim )
{
  if ( ch->fighting != NULL ) {
    bug( "Set_fighting: already fighting");
    return;
  }

  // Added by SinaC 2001 because we don't want people fighting in NULL room
  if ( ch->in_room == NULL
       || victim->in_room == NULL )
    return;

  if ( IS_AFFECTED(ch, AFF_SLEEP) )
    affect_strip( ch, gsn_sleep );

  ch->fighting = victim;
  ch->position = POS_FIGHTING;
  // Added by SinaC 2000
  ch->stunned = 0;

  return;
}

/*
 * Stop fights.
 */
void stop_fighting( CHAR_DATA *ch, bool fBoth )
{
  CHAR_DATA *fch;

  for ( fch = char_list; fch != NULL; fch = fch->next )  {
    if ( fch == ch || ( fBoth && fch->fighting == ch ) ) {
      fch->fighting	= NULL;
      if ( fch->position != POS_PARALYZED )
	fch->position = IS_NPC(fch) ? fch->default_pos : POS_STANDING;
      //fch->position	= IS_NPC(fch) ? fch->default_pos : POS_STANDING; // SinaC 2003
      // Added by SinaC 2000
      fch->stunned = 0;
      update_pos( fch );
    }
  }

  return;
}

/*
 * Make a corpse out of a character.
 */
// Modified by SinaC 2000 to return the corpse,  killer added by SinaC 2003
//void make_corpse( CHAR_DATA *ch, OBJ_DATA *&corpse, CHAR_DATA *killer )
void make_corpse( CHAR_DATA *ch, OBJ_DATA *&corpse, CHAR_DATA *killer, const long parts ) {
  char buf[MAX_STRING_LENGTH];
  //  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  const char *name;
  ROOM_INDEX_DATA *morgue_location;
  //morgue_location = get_room_index ( ROOM_VNUM_MORGUE );
  morgue_location = get_morgue_room( ch ); // SinaC 2003
  bool instant_decay;

  // Added by SinaC 2000 for FORM_INSTANT_DECAY
  if ( IS_NPC(ch) && IS_SET( ch->cstat(form), FORM_INSTANT_DECAY ) )  {
    // Removed by SinaC 2003, replaced with a script, called in onKilled
    // if a skeleton dies, an ITEM_SKELETON is left on the ground.
    //if ( ch->pIndexData->vnum == MOB_VNUM_SKELETON ){
    //
    //  sprintf(buf, "The skeleton falls on the ground.");
    //  act( buf, ch, NULL, NULL, TO_ROOM );
    //      act( buf, ch, NULL, NULL, TO_CHAR );
    //
    //      obj = create_object( get_obj_index( OBJ_VNUM_SKELETON ), ch->level );
    //
    //      obj->short_descr = str_dup( ch->short_descr );
    //      sprintf( buf, 
    //	       "A %s is lying here in a puddle of decayed flesh.", 
    //	       ch->short_descr+4);
    //      obj->description = str_dup( buf );
    //
    //      obj->level = ch->level;
    //
    //      obj_to_room( obj, ch->in_room );
    //    }
    //    else{
    // no corpse, the obj from inventory goes in the room
    //sprintf(buf, "The corpse of %s falls and decays instantly to dust.",ch->short_descr);
    //act( buf, ch, NULL, NULL, TO_ROOM );
    //act( buf, ch, NULL, NULL, TO_CHAR );
    //}

    instant_decay = TRUE;
  }
  else
    instant_decay = FALSE;

  if ( instant_decay ) {
    corpse = NULL;
  }
  else {
    if ( IS_NPC(ch) ) {
      name	= ch->short_descr;
      corpse	= create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
      corpse->timer	= number_range( 3, 6 );
      if ( ch->gold > 0 || ch->silver > 0 ) {
	obj_to_obj( create_money( ch->gold, ch->silver ), corpse );
	ch->gold = 0;
	ch->silver = 0;
      }
      corpse->cost = 0;
    }
    else {
      name		= ch->name;
      corpse	= create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
      corpse->timer	= number_range( 25, 40 );
      REMOVE_BIT(ch->act,PLR_CANLOOT);
      // Added by SinaC 2000
      if (!ch->clan /*|| IN_BATTLE(ch)*/)
	corpse->owner = str_dup(ch->name);
      else {
	corpse->owner = NULL;
	if (ch->gold > 1 || ch->silver > 1) {
	  obj_to_obj(create_money(ch->gold/2, ch->silver/2), corpse);
	  ch->gold -= ch->gold/2; 
	  ch->silver -= ch->silver/2;
	}
      }

      corpse->cost = 0;
    }

    corpse->level = ch->level;

    sprintf( buf, corpse->short_descr, name );
    corpse->short_descr = str_dup( buf );

    sprintf( buf, corpse->description, name );
    corpse->description = str_dup( buf );
  }

  for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;

    bool floating = FALSE;

    // if stay death: stays in the right wear loc           SinaC 2001
    //  but add an extra test in extract_char to avoid losing it definitively
    if ( !IS_NPC(ch) && IS_SET( obj->extra_flags, ITEM_STAY_DEATH ) )
      continue;

    if (obj->wear_loc == WEAR_FLOAT)
      floating = TRUE;
    obj_from_char( obj );

    if (obj->item_type == ITEM_POTION)
      obj->timer = number_range(500,1000);
    if (obj->item_type == ITEM_SCROLL)
      obj->timer = number_range(1000,2500);
    if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH) && !floating) {
      obj->timer = number_range(5,10);
      REM_OBJ_STAT(obj,ITEM_ROT_DEATH);
    }
    REM_OBJ_STAT(obj,ITEM_VIS_DEATH);

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
      extract_obj( obj );
    else if (floating) {
      if (IS_OBJ_STAT(obj,ITEM_ROT_DEATH)) {/* get rid of it! */
	if (obj->contains != NULL) {
	  OBJ_DATA *in, *in_next;

	  act("$p evaporates,scattering its contents.", ch,obj,NULL,TO_ROOM);
	  for (in = obj->contains; in != NULL; in = in_next) {
	    in_next = in->next_content;
	    obj_from_obj(in);
	    obj_to_room(in,ch->in_room);
	  }
	}
	else
	  act("$p evaporates.", ch,obj,NULL,TO_ROOM);
	extract_obj(obj);
      }
      else {
	act("$p falls to the floor.",ch,obj,NULL,TO_ROOM);
	obj_to_room(obj,ch->in_room);
	OBJPROG(obj,ch,"onDropped",ch);
      }
    }
    else {
      if ( instant_decay == TRUE ) 
	obj_to_room( obj, ch->in_room );
      else 
	obj_to_obj( obj, corpse );
    }
  }

  /*obj_to_room( corpse, ch->in_room );
    return;
    }*/
  if ( corpse != NULL ) {
    // SinaC 2003
    // corpse->value0 represents ch's vnum if ch is a mob
    // corpse->value1 represents ch's form
    // corpse->value2 represents ch's parts
    // corpse->value4 represents ch's cut parts
    corpse->baseval[1] = corpse->value[1] = ch->cstat(form);
    corpse->baseval[2] = corpse->value[2] = ch->cstat(parts);
    corpse->baseval[4] = corpse->value[4] = parts;
    if ( parts ) { // SinaC 2003, parts cut off by death_cry
      REMOVE_BIT( corpse->baseval[2], parts );
      REMOVE_BIT( corpse->value[2], parts );
    }

    if ( IS_NPC(ch) ) {
      obj_to_room( corpse, ch->in_room );
      // Added by SinaC 2003, value 0 represents mob's vnum
      corpse->baseval[0] = corpse->value[0] = ch->pIndexData->vnum;
      // Added by SinaC 2003, corpse gets an extra field giving who kills him/her
      Value v = add_extra_field( corpse, "killer_name" );
      if ( killer == NULL )
	v.setValue( "" );
      else if ( killer == ch )
	v.setValue( "myself" );
      else
	v.setValue( NAME(killer) );
    }
    else
      // Added by SinaC 2000
      if (!ch->clan /*|| IN_BATTLE(ch)*/)
	obj_to_room( corpse, morgue_location );
      else
	obj_to_room( corpse, ch->in_room );
  }

  // Added by SinaC 2001
  recomproom( ch->in_room );

  return;
}



/*
 * Improved Death_cry contributed by Diavolo., Modified by SinaC 2003
 */
//void death_cry( CHAR_DATA *ch ) {
long death_cry( CHAR_DATA *ch ) {
  ROOM_INDEX_DATA *was_in_room;
  char *msg;
  int door;
  int vnum;

  long parts = 0;

  vnum = 0;
  msg = "You hear $n's death cry.";

  // Added by SinaC 2001
  if ( !IS_SET(ch->cstat(form),FORM_UNDEAD)
       && !IS_SET( ch->cstat(form), FORM_INSTANT_DECAY )
       && !IS_SET(ch->cstat(form),FORM_MIST) 
       && !IS_SET(ch->cstat(form),FORM_INTANGIBLE) )
    switch ( number_bits(4)) { // from 0 to 15
    case  0: 
      msg  = "$n hits the ground ... DEAD.";			
      break;
    case  1:
      if (ch->material == 0) {
	msg  = "$n splatters blood on your armor.";
	break;
      }
    case  2: 							
      if (IS_SET(ch->cstat(parts),PART_GUTS)) {
	msg = "$n spills $s guts all over the floor.";
	vnum = OBJ_VNUM_GUTS;
	parts = PART_GUTS; // SinaC 2003
      }
      break;
    case  3: 
      if (IS_SET(ch->cstat(parts),PART_HEAD)) {
	msg  = "$n's severed head plops on the ground.";
	vnum = OBJ_VNUM_SEVERED_HEAD;
	parts = PART_HEAD | PART_EYE | PART_EAR | PART_LONG_TONGUE
	  | PART_EYESTALKS | PART_FANGS | PART_HORNS | PART_TUSKS; // SinaC 2003
      }
      break;
    case  4:
      if (IS_SET(ch->cstat(parts),PART_HEART)) {
	msg  = "$n's heart is torn from $s chest.";
	vnum = OBJ_VNUM_TORN_HEART;
	parts = PART_HEART; // SinaC 2003
      }
      break;
    case  5:
      if (IS_SET(ch->cstat(parts),PART_ARMS)) {
	msg  = "$n's arm is sliced from $s dead body.";
	vnum = OBJ_VNUM_SLICED_ARM;
	parts = PART_ARMS | PART_HANDS | PART_FINGERS | PART_CLAWS; // SinaC 2003
      }
      break;
    case  6:
      if (IS_SET(ch->cstat(parts),PART_LEGS)) {
	msg  = "$n's leg is sliced from $s dead body.";
	vnum = OBJ_VNUM_SLICED_LEG;
	parts = PART_LEGS | PART_FEET; // SinaC 2003
      }
      break;
    case 7:
      if (IS_SET(ch->cstat(parts),PART_BRAINS))	{
	msg = "$n's head is shattered, and $s brains splash all over you.";
	vnum = OBJ_VNUM_BRAINS;
	parts = PART_BRAINS | PART_HEAD | PART_EYE | PART_EAR | PART_LONG_TONGUE
	  | PART_EYESTALKS | PART_FANGS | PART_HORNS | PART_TUSKS; // SinaC 2003
      }
    }
  
  act( msg, ch, NULL, NULL, TO_ROOM );

  if ( vnum != 0 ) {
    char buf[MAX_STRING_LENGTH];
    OBJ_DATA *obj;
    const char *name;

    name		= IS_NPC(ch) ? ch->short_descr : ch->name;
    OBJ_INDEX_DATA *objIndexData = get_obj_index( vnum );
    if ( objIndexData == NULL )
      bug("death_cry: Missing parts vnum %d (removing parts: %s)",
	  vnum, flag_string( part_flags, parts ) );
    else {
      obj		= create_object( objIndexData , 0 );
      obj->timer	= number_range( 4, 7 );
      
      sprintf( buf, obj->short_descr, name );
      obj->short_descr = str_dup( buf );
      
      sprintf( buf, obj->description, name );
      obj->description = str_dup( buf );
      
      if (obj->item_type == ITEM_FOOD) {
	if (IS_SET(ch->cstat(form),FORM_POISON)) {
	  obj->baseval[3] = 1;
	  recompobj(obj);
	} 
	else if (!IS_SET(ch->cstat(form),FORM_EDIBLE))
	  obj->item_type = ITEM_TRASH;
      }
      
      obj_to_room( obj, ch->in_room );
    }
  }

  // Sentient test added by SinaC 2001
  if ( IS_NPC(ch) && IS_SET(ch->cstat(form), FORM_SENTIENT ))
    msg = "You hear something's death cry.";
  else
    msg = "You hear someone's death cry.";

  was_in_room = ch->in_room;
  for ( door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
    EXIT_DATA *pexit;

    if ( ( pexit = was_in_room->exit[door] ) != NULL
	 &&   pexit->u1.to_room != NULL
	 &&   pexit->u1.to_room != was_in_room ) {
      ch->in_room = pexit->u1.to_room;
      act( msg, ch, NULL, NULL, TO_ROOM );
    }
  }
  ch->in_room = was_in_room;

  // Added by SinaC 2001, 15% chance of getting a puddle of blood
  if ( IS_SET( ch->cstat(form), FORM_UNDEAD )
       || IS_SET( ch->cstat(form), FORM_INTANGIBLE )
       || IS_SET( ch->cstat(form), FORM_INSTANT_DECAY )
       || IS_SET( ch->cstat(form), FORM_MIST ) )
    return parts; // SinaC 2003
  OBJ_DATA *puddle;
  if ( number_percent() < 15 ) {
    puddle = create_object( get_obj_index( OBJ_VNUM_BLOOD_PUDDLE ), 0 );
    puddle->timer = 3;
    obj_to_room( puddle, ch->in_room );
    act("$n spills blood everywhere on the ground.", ch, NULL, NULL, TO_ROOM );
  }

  return parts; // SinaC 2003
}


///* Modified by Sinac 1997 for Battle */
// Modified by SinaC 2000 ;)
void raw_kill( CHAR_DATA *ch, CHAR_DATA *victim ) {
  ROOM_INDEX_DATA *location;
  //  bool pet_charmies;
  OBJ_DATA *corpse;

  //  // Added by SinaC 2000 for Restless Spirit to avoid charmies or pet to raise from
  //  //  dead
  //  if ( IS_NPC(victim) && (IS_AFFECTED(victim, AFF_CHARM) || IS_SET(victim->act,ACT_PET) ))
  //    pet_charmies = TRUE;
  //  else
  //    pet_charmies = FALSE;

  stop_fighting( victim, TRUE );

  // Added by SinaC 2001 for onKilled script trigger
  victim->position = POS_RESTING; // to avoid problem with act
  Value args[] = { ch };
  int result = mobprog( victim, ch, "onKilled", args );
  victim->position = POS_DEAD;
  // result: 0     :   everything is done as before, nothing special
  //      bit 0 set:  don't call death_cry
  //      bit 1 set:  don't call make_corpse

  // Added by SinaC 2003 for onKill script trigger
  MOBPROG( ch, victim, "onKill", victim );

  // Added by SinaC 2003 for faction
  update_faction_on_aggressive_move( ch, victim );

  // If onKilled result has not bit NODEATHCRY (bit 0), death_cry is called
  long parts = 0;
  if ( !IS_SET_BIT( result, 0 ) ) {
    parts = death_cry( victim );
    if ( IS_NPC(victim) && IS_SET( victim->cstat(form), FORM_INSTANT_DECAY ) )  { // moved out of make_corpse
      char buf[MAX_STRING_LENGTH];
      sprintf(buf, "The corpse of %s falls and decays instantly to dust.", victim->short_descr);
      act( buf, ch, NULL, NULL, TO_ROOM );
      act( buf, ch, NULL, NULL, TO_CHAR );
    }
  }

  // Added by SinaC 2003 for MOUNT code, when a bonded mount dies
  //  its master really really suffers
  mount_dying_drawback( victim );

  // Added by SinaC 2001 to check what are the most dangerous mob
  if ( IS_NPC(ch) && !IS_NPC(victim))
    ch->pIndexData->killed_by++;

  // NPC stops here
  if ( IS_NPC(victim) ) {
    // If onKilled result has not bit NOCORPSE (bit 1), make_corpse is called
    if ( !IS_SET_BIT( result, 1 ) ) {
      make_corpse( victim, corpse, ch, parts ); // Modified by SinaC 2003
    }
    victim->pIndexData->killed++;
    kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
    extract_char( victim, TRUE );

    // Removed by SinaC 2003, could be done bu script
    //    // added by SinaC 2000 for Restless Spirit
    //    if (!pet_charmies && ch != NULL )
    //      if ( !IS_SET(victim->cstat(form),FORM_UNDEAD))
    //	check_spirit(ch,victim);

    return;
  }

  // PC continues
  // SinaC 2000 for Arena
  if ( !IN_BATTLE(victim)) {
    make_corpse( victim, corpse, ch, parts ); // SinaC 2003
    extract_char( victim, FALSE );

    // Added by SinaC 2000 for stay_death item, done in extract_char now, SinaC 2001
    //transfer_obj_stay_death( victim, corpse );
  }

  // Modified by SinaC 2001
  /*
  while ( victim->affected )
    affect_remove( victim, victim->affected );
  */
  AFFECT_DATA *paf, *paf_next;
  for ( paf = victim->affected; paf; paf = paf_next ) {
    paf_next = paf->next;
    //if ( paf->duration == DURATION_PERMANENT ) // PERMANENT affect stays when affected died
    if ( IS_SET( paf->flags, AFFECT_STAY_DEATH ) )
      continue;
    affect_remove( victim, paf ); // victim->affected
  }
    
  if ( !IN_BATTLE(victim)) {
    victim->position	= POS_RESTING;
    victim->hit	= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
  // Added by SinaC 2001 for mental user
    victim->psp	        = UMAX( 1, victim->psp );
    victim->move	= UMAX( 1, victim->move );
  }
  else {
    victim->position	= POS_STANDING;
    victim->hit	= victim->cstat( max_hit  );
    victim->mana	= victim->cstat( max_mana );
    // Added by SinaC 2001 for mental user
    victim->psp	        = victim->cstat( max_psp );
    victim->move	= victim->cstat( max_move );
  }

  victim->pcdata->condition[COND_HUNGER] = 40;
  victim->pcdata->condition[COND_THIRST] = 40;
  victim->pcdata->condition[COND_DRUNK]  = 0;
  victim->pcdata->condition[COND_FULL]   = 40;

  //recompute(victim); NO NEED: done in char_to_room
  /*  save_char_obj( victim ); we're stable enough to not need this :) */

  // Added by SinaC 2001
  //if ( victim->clan )
  //  location = get_room_index( get_clan_table(victim->clan)->hall ); 
  //else
    //location = get_room_index( hometown_table[victim->pcdata->hometown].hall ); 
  location = get_hall_room(victim); // SinaC 2003

  char_from_room( victim );
  char_to_room( victim, location );

  check_rebirth(victim);

  return;
}


void group_gain( CHAR_DATA *ch, CHAR_DATA *victim )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *gch;
  //  CHAR_DATA *lch;
  int xp;
  // Removed by SinaC 2000, don't divide XP among the members of the group
  //    int members;
  //    int group_levels;

  /*
   * Monsters don't get kill xp's or alignment changes.
   * P-killing doesn't help either.
   * Dying of mortal wounds or poison doesn't give xp to anyone!
   */
  if ( victim == ch )
    return;
 
  // Removed by SinaC 2000, XP is not yet divided among the members of the group
  /*    
   *members = 0;
   *group_levels = 0;
   *for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
   *{
   *	if ( is_same_group( gch, ch ) )
   *    {
   *	    members++;
   *	    group_levels += IS_NPC(gch) ? gch->level / 2 : gch->level;
   *	}
   *}
   *
   *if ( members == 0 )
   *{
   *bug( "Group_gain: members.", members );
   *members = 1;
   *group_levels = ch->level ;
   *}
   */

  //    lch = (ch->leader != NULL) ? ch->leader : ch;

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
    OBJ_DATA *obj;
    OBJ_DATA *obj_next;

    if ( !is_same_group( gch, ch ) || IS_NPC(gch))
      continue;

    // Removed by SinaC 2000, XP is not yet divided among the members
    //  of the group
    //	xp = xp_compute( gch, victim, group_levels );  
    xp = xp_compute( gch, victim, gch->level );

    sprintf( buf, "{yYou receive %d experience points.{x\n\r", xp );
    send_to_char( buf, gch );
    gain_exp( gch, xp, FALSE );

    //    if (IS_SET(ch->act, PLR_QUESTOR) 
    //	&& IS_NPC(victim)
    //	&& !IS_NPC(ch)) {
    //      if (ch->pcdata->questmob == victim->pIndexData->vnum) {
    //	send_to_char("{rQUEST: You have almost completed your QUEST!\n\r",ch);
    //	send_to_char("{rReturn to the questmaster before your time runs out!{x\n\r",ch);
    //	ch->pcdata->questmob = -1;
    //      }
    //    }

    // Added by SinaC 2001
    bool drop = FALSE;
    // Replaced with prereqs test in recompute, SinaC 2001
    // Modified by SinaC 2000, the gch's were previously ch's
    for ( obj = gch->carrying; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;
      if ( obj->wear_loc == WEAR_NONE )
	continue;

      if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(gch)    )
	   ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(gch)    )
	   ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(gch) ) ) {
	act( "You are zapped by $p.", gch, obj, NULL, TO_CHAR );
	act( "$n is zapped by $p.",   gch, obj, NULL, TO_ROOM );
	obj_from_char( obj );

	// before we could drop a NODROP item with this SinaC 2000
	if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
	  obj_to_char( obj, gch );
	else { // Modified by SinaC 2001
	  obj_to_room( obj, gch->in_room );
	  OBJPROG(obj,gch,"onDropped",gch); // Added by SinaC 2003
	  drop = TRUE;
	}
      }
    }
    if ( drop )
      recomproom(gch->in_room);
  }

  return;
}


// SinaC 2000
int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
{
  /* 
     By Blade of "- E -"
     since this is based on freeware, this snippet is freeware, please use it at
     your descretion - you may place it in any and all snippet/code archives you
     desire.  to install, simply replace your current xp_compute with this version 
  */
  
  int xp,base_exp;
  int align,level_range;
  int change;
  int time_per_level;

  level_range = victim->level - gch->level;
 
  // if there is a too big difference ==> 0 exp
  if ( level_range < -15 ) return 0;

  /* compute the base exp */
  /* old switch with table for base_exp removed by blade, 19 lines saved */
         
  /* non-tabular xp calculation for faster xp calc  by blade */
  if (level_range > -1)
    base_exp = 65 + 24 * (level_range);
  else 
    base_exp = ( 5 + (65/((level_range -1)* -1)));
      
  /* do alignment computations */
   
  // Modified by SinaC 2001 etho/alignment are attributes now
  // Modified by SinaC 2000 for alignment/etho
  //align = victim->align.alignment - gch->align.alignment;
  align = victim->bstat(alignment) - gch->bstat(alignment);

  if (IS_SET(victim->act,ACT_NOALIGN)) {
    /* no change */
  }
  else if (align > 500) {/* monster is more good than slayer */
    change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
    change = UMAX(1,change);
    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000 for alignment/etho
    //gch->align.alignment = UMAX(-1000,gch->align.alignment - change);
    gch->bstat(alignment) = UMAX(-1000,gch->bstat(alignment) - change);
  }

  else if (align < -500) {/* monster is more evil than slayer */
    change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
    change = UMAX(1,change);
    // Modified by SinaC 2001 etho/alignment are attributes now
    //gch->align.alignment = UMIN(1000,gch->align.alignment + change);
    gch->bstat(alignment) = UMIN(1000,gch->bstat(alignment) + change);
  }

  else {/* improve this someday */
    // Modified by SinaC 2001 etho/alignment are attributes now
    //    change =  gch->align.alignment * base_exp/500 * gch->level/total_levels;  
    //    gch->align.alignment -= change;
    change =  gch->bstat(alignment) * base_exp/500 * gch->level/total_levels;  
    gch->bstat(alignment) -= change;

  }
  if (align < 0)
    align = -1 * align;
        
  /* calculate exp multiplier */
  if (IS_SET(victim->act,ACT_NOALIGN))
    xp = base_exp;
  else {  
    /* new xp by align produces flat line xp gradient from 0 align
       dif to 2000 align diff - by blade */
      
    if (align < 0)
      align = -1 * align;
      
    if (align > 1900) /* small bonus for big align diffrences, by blade */
      base_exp = base_exp + 1;
      
    xp = (int)(base_exp * ((float)(align/1800.0) + .45));
  }
  /* old xp calc by align removed by blade, 85 lines saved */
  
  /* more exp at the low levels */
  if (gch->level < 6)
    xp = 10 * xp / (gch->level + 4);
  /* less at high - removed by blade since we have max mort of 390, it
     would suck to reduce xp for all level 35+ people */


    /* reduce for playing time */
  
  {
    /* compute quarter-hours per level */
    time_per_level = 4 *
      (gch->played + (int) (current_time - gch->logon))/3600
      / gch->level;
    
    time_per_level = URANGE(2,time_per_level,12);
    if (gch->level < 15)  /* make it a curve */
      time_per_level = UMAX(time_per_level,(15 - gch->level));
    xp = xp * time_per_level / 10; /* changed from 12 to 10 by blade to allow
				      for slightly faster leveling */
  }
   
  // Added by SinaC 2001 to allow a faster levelling
  if ( gch->level < 10 )        // < 10   100% more
    xp = xp * 2;
  else if ( gch->level < 20 )   // < 20    50% more
    xp = ( 3 * xp ) / 2;
  else if ( gch->level < 30 )   // < 30    33% more
    xp = ( 4 * xp ) / 3;
  else if ( gch->level < 40 )   // < 40    25% more
    xp = ( 5 * xp ) / 4;


  /* randomize the rewards - modified by blade from 3/4 - 5/4 to 4/5 -
6/5 */
  xp = number_range (xp * 4/5, xp * 6/5);

  /* adjust for grouping */
  xp = xp * gch->level/( UMAX(1,total_levels -1) );

  return xp;
}

//  /*
//   * Compute xp for a kill.
//   * Also adjust alignment of killer.
//   * Edit this function to change xp computations.
//   */
//  int xp_compute( CHAR_DATA *gch, CHAR_DATA *victim, int total_levels )
//  {
//      int xp,base_exp;
//      int align,level_range;
//      int change;

//      level_range = victim->level - gch->level;

//      /* compute the base exp */
//      switch (level_range)
//      {
//   	default : 	base_exp =   0;		break;
//  	case -9 :	base_exp =   1;		break;
//  	case -8 :	base_exp =   2;		break;
//  	case -7 :	base_exp =   5;		break;
//  	case -6 : 	base_exp =   9;		break;
//  	case -5 :	base_exp =  11;		break;
//  	case -4 :	base_exp =  22;		break;
//  	case -3 :	base_exp =  33;		break;
//  	case -2 :	base_exp =  50;		break;
//  	case -1 :	base_exp =  66;		break;
//  	case  0 :	base_exp =  83;		break;
//  	case  1 :	base_exp =  99;		break;
//  	case  2 :	base_exp = 121;		break;
//  	case  3 :	base_exp = 143;		break;
//  	case  4 :	base_exp = 165;		break;
//      } 
    
//      if (level_range > 4)
//  	base_exp = 160 + 20 * (level_range - 4);

//      /* do alignment computations */
   
//      align = victim->alignment - gch->alignment;

//      if (IS_SET(victim->act,ACT_NOALIGN))
//      {
//  	/* no change */
//      }
//      else if (align > 500) /* monster is more good than slayer */
//      {
//  	change = (align - 500) * base_exp / 500 * gch->level/total_levels; 
//  	change = UMAX(1,change);
//          gch->alignment = UMAX(-1000,gch->alignment - change);
//      }
//      else if (align < -500) /* monster is more evil than slayer */
//      {
//  	change =  ( -1 * align - 500) * base_exp/500 * gch->level/total_levels;
//  	change = UMAX(1,change);
//  	gch->alignment = UMIN(1000,gch->alignment + change);
//      }
//      else /* improve this someday */
//      {
//  	change =  gch->alignment * base_exp/500 * gch->level/total_levels;  
//  	gch->alignment -= change;
//      }
    
//      /* calculate exp multiplier */
//      if (IS_SET(victim->act,ACT_NOALIGN))
//  	xp = base_exp;
//      else if (gch->alignment > 500)  /* for goodie two shoes */
//      {
//  	if (victim->alignment < -750)
//  	    xp = (base_exp *4)/3;
   
//   	else if (victim->alignment < -500)
//  	    xp = (base_exp * 5)/4;

//          else if (victim->alignment > 750)
//  	    xp = base_exp / 4;

//     	else if (victim->alignment > 500)
//  	    xp = base_exp / 2;

//          else if (victim->alignment > 250)
//  	    xp = (base_exp * 3)/4; 

//  	else
//  	    xp = base_exp;
//      }
//      else if (gch->alignment < -500) /* for baddies */
//      {
//  	if (victim->alignment > 750)
//  	    xp = (base_exp * 5)/4;
	
//    	else if (victim->alignment > 500)
//  	    xp = (base_exp * 11)/10; 

//     	else if (victim->alignment < -750)
//  	    xp = base_exp/2;

//  	else if (victim->alignment < -500)
//  	    xp = (base_exp * 3)/4;

//  	else if (victim->alignment < -250)
//  	    xp = (base_exp * 9)/10;

//  	else
//  	    xp = base_exp;
//      }
//      else if (gch->alignment > 200)  /* a little good */
//      {

//  	if (victim->alignment < -500)
//  	    xp = (base_exp * 6)/5;

//   	else if (victim->alignment > 750)
//  	    xp = base_exp/2;

//  	else if (victim->alignment > 0)
//  	    xp = (base_exp * 3)/4; 
	
//  	else
//  	    xp = base_exp;
//      }
//      else if (gch->alignment < -200) /* a little bad */
//      {
//  	if (victim->alignment > 500)
//  	    xp = (base_exp * 6)/5;
 
//  	else if (victim->alignment < -750)
//  	    xp = base_exp/2;

//  	else if (victim->alignment < 0)
//  	    xp = (base_exp * 3)/4;

//  	else
//  	    xp = base_exp;
//      }
//      else /* neutral */
//      {

//  	if (victim->alignment > 500 || victim->alignment < -500)
//  	    xp = (base_exp * 4)/3;

//  	else if (victim->alignment < 200 && victim->alignment > -200)
//  	    xp = base_exp/2;

//   	else
//  	    xp = base_exp;
//      }

//      /* more exp at the low levels */
//      if (gch->level < 6)
//      	xp = 10 * xp / (gch->level + 4);

//      /* less at high */
//      if (gch->level > 35 )
//  	xp =  15 * xp / (gch->level - 25 );

//      /* reduce for playing time */

//      /* This code has been rewritten using constants. Defaults are 6, 3 -- Oxtal*/
//      #define MAX_DIV	6
//      /* This is maximal divider a player gets if he levels too quickly */
//      #define STD_TIME	2
//      /* This is the time a player should use to complete a level.
//         Some malus if he is slower (unit = hour)

//         Thus, if a player takes less than STD_TIME/MAX_DIV hours to 
//         finish a level, he gets MAX_DIV divider */
    
    
//      {
//  	/* compute quarter-hours per level */
//  	long time_per_level = (gch->played + (int) (current_time - gch->logon))
//  			 / gch->level;

//  	time_per_level = URANGE(3600*STD_TIME/MAX_DIV,time_per_level,3600*STD_TIME);
//  	if (gch->level < 15)  /* make it a curve */
//  	    time_per_level = UMAX(time_per_level,(15 - gch->level)*3600/4);
//  	xp = xp * time_per_level / STD_TIME / 3600;
//      }
   
//      /* randomize the rewards */
//      xp = number_range (xp * 3/4, xp * 5/4);

//      /* adjust for grouping */
//      xp = xp * gch->level/( UMAX(1,total_levels -1) );


//      return xp;
//  }


void dam_message( CHAR_DATA *ch, CHAR_DATA *victim,int dam,int dt,bool immune, bool toRoom ) {
  char buf1[256], buf2[256], buf3[256];
  const char *vs;
  const char *vp;
  const char *attack;
  char punct;

  if ( ch == NULL || victim == NULL || !ch->valid || !victim->valid )
    return;

/* Replaced by SinaC 2001
  if ( dam ==   0 ) { vs = "miss";	vp = "misses";		}
  else if ( dam <=   4 ) { vs = "scratch";	vp = "scratches";	}
  else if ( dam <=   8 ) { vs = "graze";	vp = "grazes";		}
  else if ( dam <=  12 ) { vs = "hit";	vp = "hits";		}
  else if ( dam <=  16 ) { vs = "injure";	vp = "injures";		}
  else if ( dam <=  20 ) { vs = "wound";	vp = "wounds";		}
  else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";		}
  else if ( dam <=  28 ) { vs = "decimate";	vp = "decimates";	}
  else if ( dam <=  32 ) { vs = "devastate";	vp = "devastates";	}
  else if ( dam <=  36 ) { vs = "maim";	vp = "maims";		}
  else if ( dam <=  40 ) { vs = "{bMUTILATE{x";	
  vp = "{bMUTILATES{x";	}
  else if ( dam <=  44 ) { vs = "{wDISEMBOWEL{x";	
  vp = "{wDISEMBOWELS{x";	}
  else if ( dam <=  48 ) { vs = "{rDISMEMBER{x";	
  vp = "{rDISMEMBERS{x";	}
  else if ( dam <=  52 ) { vs = "{mMASSACRE{x";	
  vp = "{mMASSACRES{x";	}
  else if ( dam <=  56 ) { vs = "{gMANGLE{x";	
  vp = "{gMANGLES{x";		}
  else if ( dam <=  60 ) { vs = "{c*** DEMOLISH ***{x";
  vp = "{c*** DEMOLISHES ***{x";			}
  else if ( dam <=  75 ) { vs = "{w**** {mDEVASTATE {w****{x";
  vp = "{w**** {mDEVASTATES {w****{x";			}
  else if ( dam <= 100)  { vs = "{b=== {cOBLITERATE {b==={x";
  vp = "{b=== {cOBLITERATES {b==={x";		}
  else if ( dam <= 150)  { vs = "{g>>> {yANNIHILATE{g <<<{x";
  vp = "{g>>> {yANNIHILATES{g <<<{x";		}
  else if ( dam <= 200)  { vs = "{r-=<*>=- {bERADICATE {r-=<*>=-{x";
  vp = "{r-=<*>=-{b ERADICATES {r-=<*>=-{x";			}
  else if ( dam <= 300)  { vs = "{r<*>{y<*>{g<*>{b<*>{y NUKE {b<*>{g<*>{y<*>{r<*>{x";
  vp = "{r<*>{y<*>{g<*>{b<*>{y NUKES {b<*>{g<*>{y<*>{r<*>{x";}
  else if ( dam <= 400)  { vs = "{b--*-- --*-- RUPTURE --*-- --*--{x";
  vp = "{b--*-- --*-- RUPTURES --*-- --*--{x";}
  else                   { vs = "do {bU{rN{gS{yP{cE{mA{rK{bA{gB{yL{cE {xthings to";
  vp = "does {bU{rN{gS{yP{cE{mA{rK{bA{gB{yL{cE {xthings to";		}
*/
  if ( dam ==   0 ) { vs = "miss";	vp = "misses";		}
  else if ( dam <=   4 ) { vs = "scratch";	vp = "scratches";	}
  else if ( dam <=   8 ) { vs = "graze";	vp = "grazes";		}
  else if ( dam <=  12 ) { vs = "hit";	vp = "hits";		}
  else if ( dam <=  16 ) { vs = "injure";	vp = "injures";		}
  else if ( dam <=  20 ) { vs = "wound";	vp = "wounds";		}
  else if ( dam <=  24 ) { vs = "maul";       vp = "mauls";		}
  else if ( dam <=  28 ) { vs = "decimate";	vp = "decimates";	}
  else if ( dam <=  32 ) { vs = "devastate";	vp = "devastates";	}
  else if ( dam <=  36 ) { vs = "maim";	vp = "maims";		}
  else if ( dam <=  40 ) { vs = "MUTILATE";
  vp = "MUTILATES";	}
  else if ( dam <=  44 ) { vs = "DISEMBOWEL";
  vp = "DISEMBOWELS";	}
  else if ( dam <=  48 ) { vs = "DISMEMBER";
  vp = "DISMEMBERS";	}
  else if ( dam <=  52 ) { vs = "MASSACRE";
  vp = "MASSACRES";	}
  else if ( dam <=  56 ) { vs = "MANGLE";
  vp = "MANGLES";		}
  else if ( dam <=  60 ) { vs = "*** DEMOLISH ***";
  vp = "*** DEMOLISHES ***";			}
  else if ( dam <=  75 ) { vs = "**** DEVASTATE ****";
  vp = "**** DEVASTATES ****";			}
  else if ( dam <= 100)  { vs = "=== OBLITERATE ===";
  vp = "=== OBLITERATES ===";		}
  else if ( dam <= 150)  { vs = ">>> ANNIHILATE <<<";
  vp = ">>> ANNIHILATES <<<";		}
  else if ( dam <= 200)  { vs = "-=<*>=- ERADICATE -=<*>=-";
  vp = "-=<*>=- ERADICATES -=<*>=-";			}
  else if ( dam <= 300)  { vs = "<*><*><*><*> NUKE <*><*><*><*>";
  vp = "<*><*><*><*> NUKES <*><*><*><*>";}
  else if ( dam <= 400)  { vs = "--*-- --*-- RUPTURE --*-- --*--";
  vp = "--*-- --*-- RUPTURES --*-- --*--";}
  else                   { vs = "do UNSPEAKABLE things to";
  vp = "does UNSPEAKABLE things to";		}

  punct   = (dam <= 24) ? '.' : '!';

  if ( dt == TYPE_HIT ) {
    // modified by SinaC 2000
    if (ch  == victim) {
      sprintf( buf1, "$n %s $melf%c",vp,punct);
      sprintf( buf2, "You %s yourself%c [%d]",vs,punct,dam);
    }
    else {
      sprintf( buf1, "$n %s $N%c",  vp, punct);
      // modified by SinaC 2000
      sprintf( buf2, "You %s $N%c [%d]", vs, punct, dam );
      // end
      sprintf( buf3, "$n %s you%c", vp, punct);
    }
  }
  else {
    if ( dt >= 0 && dt < MAX_ABILITY )
      attack	= ability_table[dt].noun_damage;
    else if ( dt >= TYPE_HIT
	      && dt < TYPE_HIT + MAX_DAMAGE_MESSAGE) 
      attack	= attack_table[dt - TYPE_HIT].noun;
    else {
      bug( "Dam_message: bad dt %d.", dt );
      dt  = TYPE_HIT;
      attack  = attack_table[0].name;
    }

    if (immune) {
      if (ch == victim) {
	sprintf(buf1,"$n is unaffected by $s own %s.",attack);
	sprintf(buf2,"Luckily, you are immune to that.");
      }
      else {
	sprintf(buf1,"$N is unaffected by $n's %s!",attack);
	sprintf(buf2,"$N is unaffected by your %s!",attack);
	sprintf(buf3,"$n's %s is powerless against you.",attack);
      }
    }
    else {
      if (ch == victim) {
	sprintf( buf1, "$n's %s %s $m%c",attack,vp,punct);
	sprintf( buf2, "Your %s %s you%c[%d]",attack,vp,punct, dam);
      }
      else {
	sprintf( buf1, "$n's %s %s $N%c",  attack, vp, punct );
	sprintf( buf2, "Your %s %s $N%c [%d]",  attack, vp, punct, dam );
	sprintf( buf3, "$n's %s %s you%c", attack, vp, punct );
      }
    }
  }

  // Modified by SinaC 2001
  char buf4[256], buf5[256], buf6[256];
  sprintf(buf4,"{y%s{x",buf1);
  sprintf(buf5,"{g%s{x",buf2);
  sprintf(buf6,"{r%s{x",buf3);
  if (ch == victim) {
    act(buf4,ch,NULL,NULL,TO_ROOM);
    act(buf5,ch,NULL,NULL,TO_CHAR);
  }
  else { // Modified by SinaC 2003, damage are shown even to sleeping/stunned/paralyzed people
    if ( toRoom )
      act_new( buf4, ch, NULL, victim, TO_NOTVICT, POS_PARALYZED );
    act_new( buf5, ch, NULL, victim, TO_CHAR, POS_PARALYZED );
    act_new( buf6, ch, NULL, victim, TO_VICT, POS_PARALYZED );
  }
  /*
  if (ch == victim) {
    act(buf1,ch,NULL,NULL,TO_ROOM);
    act(buf2,ch,NULL,NULL,TO_CHAR);
  }
  else {
    act( buf1, ch, NULL, victim, TO_NOTVICT );
    act( buf2, ch, NULL, victim, TO_CHAR );
    act( buf3, ch, NULL, victim, TO_VICT );
  }
  */
  return;
}

void do_kill( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Kill whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  /*  Allow player killing*/
  if ( IS_PC(victim) ) {
    send_to_char( "You must MURDER a player.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( "You hit yourself.  Ouch!\n\r", ch );
    multi_hit( ch, ch, TYPE_UNDEFINED );
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if ( victim->fighting != NULL && 
       !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
    act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( ch->position == POS_FIGHTING ) {
    send_to_char( "You do the best you can!\n\r", ch );
    return;
  }

  WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
  check_killer( ch, victim );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void do_murde( CHAR_DATA *ch, const char *argument )
{
  send_to_char( "If you want to MURDER, spell it out.\n\r", ch );
  return;
}

void do_murder( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Murder whom?\n\r", ch );
    return;
  }
/* Removed by SinaC 2001
  if (IS_AFFECTED(ch,AFF_CHARM) 
      || (IS_NPC(ch) && IS_SET(ch->act,ACT_PET)))
    return;
*/

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( victim == ch ) {
    send_to_char( "Suicide is a mortal sin.\n\r", ch );
    return;
  }

  if ( is_safe( ch, victim ) )
    return;

  if (IS_NPC(victim) &&
      victim->fighting != NULL && 
      !is_same_group(ch,victim->fighting)) {
    send_to_char("Kill stealing is not permitted.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
    act( "$N is your beloved master.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( ch->position == POS_FIGHTING ) {
    send_to_char( "You do the best you can!\n\r", ch );
    return;
  }

  WAIT_STATE( ch, 1 * PULSE_VIOLENCE );
  // Added by SinaC 2001
  if ( IS_SET( victim->cstat(form), FORM_SENTIENT ) ) {
    if ( can_see( victim, ch ) ) 
      sprintf(buf, "Help! I am being attacked by %s!",NAME(ch));
    else
      sprintf(buf, "Help! I am being attacked!");
    do_yell( victim, buf );
  }
  check_killer( ch, victim );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return;
}

void do_flee( CHAR_DATA *ch, const char *argument ) {
  ROOM_INDEX_DATA *was_in;
  ROOM_INDEX_DATA *now_in;
  CHAR_DATA *victim;
  int attempt;

  if ( ( victim = ch->fighting ) == NULL ) {
    if ( ch->position == POS_FIGHTING )
      ch->position = POS_STANDING;
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  // Added by SinaC 2003, onFleeing trigger
  Value args[] = {ch};
  bool value = mobprog(victim,ch,"onFleeing",args);
  if ( value )
    return;


  was_in = ch->in_room;
  //for ( attempt = 0; attempt < 6; attempt++ ) { // Modified by SinaC 2003, do we really need to change that?
  for ( attempt = 0; attempt < MAX_DIR; attempt++ ) {
    EXIT_DATA *pexit;
    int door;

    door = number_door( );
    if ( ( pexit = was_in->exit[door] ) == 0
	 ||   pexit->u1.to_room == NULL
	 ||   IS_SET(pexit->exit_info, EX_CLOSED)
	 ||   number_range(0,ch->daze) != 0
	 || ( IS_NPC(ch)
	      // Modified by SinaC 2001
	      &&   IS_SET(pexit->u1.to_room->cstat(flags), ROOM_NO_MOB) ) )
      continue;

    move_char( ch, door, FALSE, FALSE, FALSE ); // SinaC 2003 );
    if ( ( now_in = ch->in_room ) == was_in )
      continue;

    ch->in_room = was_in;
    act( "$n has fled!", ch, NULL, NULL, TO_ROOM );
    ch->in_room = now_in;

    if ( !IS_NPC(ch) ) {
      send_to_char( "You flee from combat!\n\r", ch );
      /* Removed by SinaC 2000, will become a basic thief skill freeflee
	 if( (ch->cstat(classes) & (1<<CLASS_THIEF))
	 && (number_percent() < 3*(ch->level/2) ) )
	 send_to_char( "You snuck away safely.\n\r", ch);
	 else
      */
      if ( number_percent() < get_ability( ch, gsn_freeflee ) ) {
	send_to_char( "You snuck away safely.\n\r", ch);
	check_improve( ch, gsn_freeflee, TRUE, 1 );
      }
      else{
	send_to_char( "You lose 10 exp.\n\r", ch);
	gain_exp( ch, -10, TRUE );
      }
    }

    stop_fighting( ch, TRUE );

    // Added by SinaC 2003
    Value args2[] = {ch, short_dir_name[door] };
    mobprog(victim,ch,"onFlee",args);
    
    return;
  }

  send_to_char( "PANIC! You couldn't escape!\n\r", ch );
  return;
}

void do_sla( CHAR_DATA *ch, const char *argument ) {
  send_to_char( "If you want to SLAY, spell it out.\n\r", ch );
  return;
}

void do_slay( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Slay whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( ch == victim ) {
    send_to_char( "Suicide is a mortal sin.\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim) && victim->level >= get_trust(ch) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  act( "You slay $M in cold blood!",  ch, NULL, victim, TO_CHAR    );
  act( "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
  act( "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
  raw_kill( ch, victim );
  return;
}

// Added by SinaC 2001 for cool spell effect additions
// ARMOR spell absorbs attack, the casting_level of the affect raise down by one
//  so a level 4 armor spell could absorb 3 attacks
bool check_armor_absorb ( CHAR_DATA *victim, CHAR_DATA *ch ) {
  AFFECT_DATA *af;
  //int sn;

  //sn = ability_lookup( "armor" );
  af = affect_find( victim->affected, gsn_armor );

  if ( af == NULL )
    return FALSE;

  if ( af->casting_level > 1 
       && number_range(0,3) == 0 ) {

    send_to_char("Your {Wmagical armor{x absorbs the attack.\n\r",victim );
    if ( ch != victim )
      act( "$N's {Wmagical armor{x absorbs your attack!", ch, NULL, victim, TO_CHAR );
    act( "$N's {Wmagical armor{x absorbs the attack!", ch, NULL, victim, TO_NOTVICT );
    af->casting_level--;
    return TRUE;
  }
  
  return FALSE;
}

// SHROUD spell absorbs maximum affect's level (between 3 and 6) attacks
//  + does damage and eventually poison and weaken
//bool check_shroud ( CHAR_DATA *victim, CHAR_DATA *ch ) {
bool check_shroud ( CHAR_DATA *victim, CHAR_DATA *ch, const int dam ) {
  AFFECT_DATA *af;
  // find a 'shroud' affect
  if ( ( af = affect_find( victim->affected, gsn_shroud ) ) == NULL )
    return FALSE;

  // 33% doing damage
  if ( number_range( 0, 2 ) == 0 ) {
    act("The {Dshroud{x around you hits $n", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
      act("The {Dshroud{x around $N hits you", ch, NULL, victim, TO_CHAR );
    act("The {Dshroud{x around $N hits $n", ch, NULL, victim, TO_NOTVICT );
    int dam = number_range( 1, af->casting_level*10 );
    int damdone = ability_damage( victim, ch, dam, gsn_shroud, DAM_NEGATIVE, FALSE, TRUE );
    if ( damdone == DAMAGE_DONE ) { // damage done but didn't kill opponent
      // 50% doing weaken/poison/slow
      if ( number_range( 0, 1 ) == 0 )
	switch(number_range(0,2)) {
	case 0:
	  spell_weaken( gsn_weaken, victim->level, victim, ch, TARGET_CHAR, 1 );
	  break;
	case 1:
	  spell_poison( gsn_poison, victim->level, victim, ch, TARGET_CHAR, 1 );
	  break;
	case 2:
	  spell_slow( gsn_slow, victim->level, victim, ch, TARGET_CHAR, 1 );
	  break;
	}
    }
  }
  
  // 25% absorbing an attack
  if ( number_range( 0, 3 ) == 0 && dam > 0 ) {
    act("The {Dshroud{x around you absorbs the attack.", ch, NULL, victim, TO_VICT );
    if ( ch != victim )
      act("The {Dshroud{x around $N absorbs the attack.", ch, NULL, victim, TO_CHAR );
    act("The {Dshroud{x around $N absorbs the attack.", ch, NULL, victim, TO_NOTVICT );
    //    af->casting_level--;
    af->level -= dam;
    //if ( af->casting_level <= 0 ) {
    if ( af->level <= 0 ) {
      // Added by SinaC 2003
      if ( ability_table[af->type].msg_off != NULL ) {
	send_to_char( ability_table[af->type].msg_off, victim );
	send_to_char( "\n\r", victim );
      }
      affect_strip( victim, gsn_shroud );
      // FIXME: return new dam value ?  -af->level  is the rest of damage not absorbed
    }
    return TRUE;
  }

  return FALSE;
}

bool check_shield_break( CHAR_DATA *victim, CHAR_DATA *ch, OBJ_DATA *weapon ) {
  AFFECT_DATA *af;

  if ( weapon == NULL ) // can't break weapon if doesn't have any
    return FALSE;

  af = affect_find( victim->affected, gsn_shield );

  if ( af == NULL )
    return FALSE;

  // if casting_level 4 and item not magic: 25% chance break weapon
  if ( af->casting_level == 4 
       && number_range(0,3) == 0 
       && !IS_SET(weapon->extra_flags, ITEM_MAGIC ) ) {

    act("$p hits your {Wmagical shield{x and shatters in thousand pieces.",
	victim, weapon, NULL, TO_CHAR );
    send_to_char("Your {Wmagical shield{x fades away.\n\r", victim );
    if ( ch != victim )
      act( "$p hits $n's {Wmagical shield{x and shatters in thousand pieces!", 
	   victim, weapon, ch, TO_VICT );
    act( "$n's {Wmagical shield{x shatters $p!", victim, weapon, ch, TO_NOTVICT );
    affect_strip( victim, gsn_shield );
    extract_obj(weapon);
    return TRUE;
  }

  if ( af->casting_level == 5
       && chance(33)
       && ( ( IS_SET(weapon->extra_flags, ITEM_MAGIC ) // 50% chance destroy magical weapon
	      && chance(50) )
	    || !IS_SET(weapon->extra_flags, ITEM_MAGIC ) ) ) {
    act("$p hits your {Wmagical shield{x and shatters in thousand pieces.",
	victim, weapon, NULL, TO_CHAR );
    send_to_char("Your {Wmagical shield{x fades away.\n\r", victim );
    if ( ch != victim )
      act( "$p hits $n's {Wmagical shield{x and shatters in thousand pieces!", 
	   victim, weapon, ch, TO_VICT );
    act( "$n's {Wmagical shield{x shatters $p!", victim, weapon, ch, TO_NOTVICT );
    affect_strip( victim, gsn_shield );
    extract_obj( weapon );
    return TRUE;
  }

  return FALSE;
}

// Claws damage and bite skill cause lycanthropy to propagate from ch to victim
// wield == NULL added, should never be used because when someone is affected by lycanthropy, it can use weapons
bool check_lycanthropy( CHAR_DATA *ch, CHAR_DATA *victim, const int dt, OBJ_DATA *wield ) {
  if ( ( ( dt == TYPE_HIT+5 && wield == NULL ) // claws
	 || dt == gsn_bite ) // bite skill
       && ch != victim
       && is_made_flesh( victim ) // only flesh people can be affected
       && !( IS_NPC(victim) && ( IS_SET(victim->act,ACT_UNDEAD) ) // neither undead
	     || IS_SET( victim->cstat(form),FORM_UNDEAD) )
       && !saves_spell( ch->level, victim, DAM_DISEASE ) // disease check
       && is_affected( ch, gsn_lycanthropy ) // ch  affected by lycanthropy
       && !is_affected( victim, gsn_protection_lycanthropy ) // victim  not affected by protection lycanthropy
       && !is_affected( victim, gsn_lycanthropy) ) { // victim not already affected by lycanthropy
    if ( chance(1) ) { // FIXME find a more complex formula based on level
      log_stringf("LYCANTHROPY TRANSMISSION FROM [%s] TO [%s]", NAME(ch), NAME(victim) );
      AFFECT_DATA af;
      createaff(af,-1,ch->level,gsn_lycanthropy,0,AFFECT_STAY_DEATH|AFFECT_PERMANENT|AFFECT_INVISIBLE|AFFECT_NON_DISPELLABLE);
      // we don't use affect_to_char because we want lycanthropy as last affect so we
      //  insert in the end of list, then even if another affect modify race,
      //  this one will be the last one
      // affect_to_char(victim,&af);
      AFFECT_DATA *paf_new = new_affect();
      affect_copy( paf_new, &af ); // copy affect

      // go the end of affect list
      AFFECT_DATA *last = victim->affected;
      for ( AFFECT_DATA *paf = victim->affected; paf; paf = paf->next )
	last = paf;
      // insert at the end
      if ( last != NULL ) // found last affect
	last->next = paf_new;
      else // no affect found: start of list
	victim->affected = paf_new;

      SET_BIT( victim->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );
      recompute(victim);
      return TRUE; // affect transmitted
    }
  }
  return FALSE; // not transmitted
}
