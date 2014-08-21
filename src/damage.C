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
#include "db.h"
#include "damage.h"
#include "arena.h"
#include "ability.h"
#include "hunt.h"
#include "olc_value.h"
#include "act_comm.h"
#include "fight.h"
#include "update.h"
#include "wiznet.h"
#include "special.h"
#include "act_obj.h"
#include "condition.h"
#include "gsn.h"
#include "handler.h"
#include "comm.h"
#include "utils.h"
#include "noncombatabilities.h"


void killing_payoff( CHAR_DATA *ch, CHAR_DATA *victim ) {
  // Payoff for killing things.    FROM HERE, we are sure VICTIM is DEAD, SinaC 2003
  int explost;
  OBJ_DATA *corpse;
  char buf[MAX_STRING_LENGTH];
  if ( ch != victim && !IN_BATTLE(victim) ) // Modified by SinaC 2001
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
    
    // Dying penalty:
    // 2/3 way back to previous level.
    if (!IN_BATTLE(victim)) {
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
  // dump the flags
  if ( IS_PC(ch) ?
       ( ch != victim
	 && !is_same_clan(ch,victim)
	 && !IN_BATTLE(ch) )
       :                   // Added by Oxtal
       ch->spec_fun == spec_executioner ) {
    if (IS_SET(victim->act,PLR_KILLER))
      REMOVE_BIT(victim->act,PLR_KILLER);
    else
      REMOVE_BIT(victim->act,PLR_THIEF);
  }
  
  // RT new auto commands
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
// end of VICTIM's DEAD code, SinaC 2003
}

void wimp_out( CHAR_DATA *ch, CHAR_DATA *victim, const int dam ) {
  // Wimp out?
  if ( IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
    if ( ( IS_SET(victim->act, ACT_WIMPY )
	   && number_bits( 2 ) == 0 
	   && victim->hit < victim->cstat(max_hit) / 5)
	 || ( IS_AFFECTED(victim, AFF_CHARM) 
	      && victim->master != NULL
	      && victim->master->in_room != victim->in_room ) )
      do_flee( victim, "" );
  }

  if ( !IS_NPC(victim)
       &&   victim->hit > 0
       &&   victim->hit <= victim->wimpy
       &&   victim->wait < PULSE_VIOLENCE / 2 )
    do_flee( victim, "" );
}

void position_msg( CHAR_DATA *victim, const int dam ) {
  switch( victim->position ) {
  case POS_MORTAL:
    act( "$n is mortally wounded, and will die soon, if not aided.",
	 victim, NULL, NULL, TO_ROOM );
    send_to_char("You are mortally wounded, and will die soon, if not aided.\n\r",
		 victim );
    break;

  case POS_INCAP:
    act( "$n is incapacitated and will slowly die, if not aided.",
	 victim, NULL, NULL, TO_ROOM );
    send_to_char("You are incapacitated and will slowly die, if not aided.\n\r",
		 victim );
    break;

    // Added by SinaC 2003 for paralyzing
  case POS_PARALYZED:
    act( "$n is paralyzed.",
	 victim, NULL, NULL, TO_ROOM );
    send_to_char("You are paralyzed.\n\r",
		 victim );
    break;

  case POS_STUNNED:
    act( "$n is stunned, but will probably recover.",
	 victim, NULL, NULL, TO_ROOM );
    send_to_char("You are stunned, but will probably recover.\n\r",
		 victim );
    break;

  case POS_DEAD:
    act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
    send_to_char( "You have been KILLED!!\n\r\n\r", victim );
    break;

  default:
    if ( dam > victim->cstat(max_hit) / 4 )
      send_to_char( "That really did HURT!\n\r", victim );
    if ( victim->hit < victim->cstat(max_hit) / 4 )
      send_to_char( "You sure are BLEEDING!\n\r", victim );
    break;
  }
}

bool link_dead( CHAR_DATA *victim ) {
  if ( !IS_NPC(victim) && victim->desc == NULL ) {
    if ( number_range( 0, victim->wait ) == 0 ) {
      do_recall( victim, "" );
      return TRUE;
    }
  }
  return FALSE;
}

// Inflict damage from an ability (skill/spell/...), heavily modified by SinaC 1997 -> 2003.  
//  return value if an integer instead of bool, check fight.h for available value
const int max_damage = 2000;
int ability_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type,
		    bool show, bool old_dam ) {
  if ( ch == NULL || !ch->valid ) {
    bug("damage with ch=NULL (vict:%s)",victim->name );
    return DAMAGE_NOTDONE;
  }

  if ( victim->position == POS_DEAD )
    return DAMAGE_NOTDONE;

  if ( dt >= TYPE_HIT ) {// if combat damage: call combat_damage
    bug("ability_damage: combat_damage should be called [%s] [%s] [%d] [%d] [%d] [%s] [%s]",
	NAME(ch), NAME(victim), dam, dt, dam_type, show?"true":"false", old_dam?"true":"false");
    combat_damage( ch, victim, dam, dt, dam_type, show, NULL );
  }

  // damage reduction
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

      // IF old_dam is TRUE, just initiate the fight but don't do any damage
      // If victim is charmed, ch might attack victim's master.
      // taken out by Russ!
      if ( old_dam == TRUE             // old damage Sinac 1997
	   &&   IS_NPC(ch)
	   &&   IS_NPC(victim)
	   &&   IS_AFFECTED(victim, AFF_CHARM)
	   &&   victim->master != NULL
	   &&   victim->master->in_room == ch->in_room
	   &&   number_bits( 3 ) == 0 ) {
	stop_fighting( ch, FALSE );
	multi_hit( ch, victim->master, TYPE_UNDEFINED );
	return DAMAGE_NOTDONE;
      }
    }

    // More charm stuff.
    if ( victim->master == ch )
      stop_follower( victim );
  }

  // Inviso attacks ... not.
  if ( IS_AFFECTED(ch, AFF_INVISIBLE) ) {
    affect_strip( ch, gsn_invis );
    affect_strip( ch, gsn_mass_invis );
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

  // Added by SinaC 2003, skills/affects allowing to avoid magic damage
  //  if ( dt < TYPE_HIT     not needed dt is always < TYPE_HIT
  //       && ch != victim  ) {
  if ( ch != victim ) {
    // check if affected by Justice and taking Negative damage, counter the attack
    if ( dam_type == DAM_NEGATIVE 
	 && is_affected( victim, gsn_justice ) ) {
      ability_damage( victim, ch, dam, gsn_justice, DAM_HOLY,
		      TRUE, TRUE );
      return DAMAGE_NOTDONE;
    }

    // check if affected by Corruption and taking Holy damage, counter the attack
    if ( dam_type == DAM_HOLY
	 && is_affected( victim, gsn_justice ) ) {
      ability_damage( victim, ch, dam, gsn_corruption, DAM_NEGATIVE,
		      TRUE, TRUE );
      return DAMAGE_NOTDONE;
    }
  }  

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

  // Moved by SinaC 2003, was before check_immune before
  // Added by SinaC 2001
  //if ( check_shroud( victim, ch ) ) // victim's shroud affect  absorbs  ch's attack
  if ( check_shroud( victim, ch, dam ) ) // victim's shroud affect  absorbs  ch's attack
    return DAMAGE_NOTDONE;

  // Added by SinaC 2001 for object condition
  check_damage_obj( victim, NULL, 2, dam_type );  // damage a random victim's item

  // Modified by SinaC 2001, not needed: dt is always < TYPE_HIT
  //  if ( wield != NULL  // damage ch's weapon
  //       && dt >= TYPE_HIT ) { // shooting arrows won't lower bow condition
  //    check_damage_obj( ch, wield, 2, DAM_NONE );
  
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
  //}
  // end of moved code

  // Added by SinaC 2001  for mistform disadvantage, can't do damage
  if ( is_affected( ch, gsn_mistform ) )
    return DAMAGE_NOTDONE;

  if (show)
    dam_message( ch, victim, dam, dt, immune, TRUE );

  if (dam == 0)
    return DAMAGE_NOTDONE;

  // Claws damage and bite skill cause lycanthropy to propagate
  check_lycanthropy( ch, victim, dt, NULL );

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


// Functions used to compute spell damage
//  low damage
int compute_spell_damage_easy( const int level, const int casting_level ) {
  return dice( level*UMAX(1,casting_level), 7 );
}
// normal damage
int compute_spell_damage_normal( const int level, const int casting_level ) {
  return dice( level*UMAX(1,casting_level), 11 );
}
// high damage
int compute_spell_damage_hard( const int level, const int casting_level ) {
  return dice( level*UMAX(1,casting_level), 15 );
}
