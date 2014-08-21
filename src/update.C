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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
//#include "music.h"  removed by SinaC 2003

// Added by SinaC 2001
#include "classes.h"
#include "special.h"
#include "arena.h"
#include "handler.h"
#include "comm.h"
#include "fight.h"
#include "hunt.h"
#include "db.h"
#include "act_wiz.h"
#include "act_info.h"
#include "update.h"
#include "act_obj.h"
#include "save.h"
#include "act_enter.h"
#include "act_move.h"
#include "act_comm.h"
#include "moons.h"
#include "gsn.h"
#include "olc_value.h"
#include "clan.h"
#include "wiznet.h"
#include "magic.h"
#include "effects.h"
//#include "disease.h" removed by SinaC 2003
// Added by SinaC 2003
#include "update_affect.h"
#include "wearoff_affect.h"
#include "fight.h"
#include "ability.h"
#include "copyover.h"
#include "recycle.h"
#include "config.h"
#include "interp.h"
#include "faction.h"
#include "utils.h"
#include "html.h"
#include "weather.h"



/*
 * Local functions.
 */
int	hit_gain	  args( ( CHAR_DATA *ch ) );
int	mana_gain	  args( ( CHAR_DATA *ch ) );
int	move_gain	  args( ( CHAR_DATA *ch ) );
// Added by SinaC 2001 for mental user
int	psp_gain	  args( ( CHAR_DATA *ch ) );
void	mobile_update	  args( ( void ) );
void	weather_update	  args( ( void ) );
void	char_update	  args( ( void ) );
void	obj_update	  args( ( void ) );
// Added by SinaC 2001 for room affects
void	room_update	  args( ( void ) );

void	aggr_update  	  args( ( void ) );
//void    quest_update      args( ( void ) ); /* Vassago - quest.c */
void    auction_update    args( ( void ) ); /* Seytan 1997 */
//removed by SinaC 2000 really stupid idea
//void    zombie_update   args( ( void ) ); /* Sinac 1997 */
void    hints_update	  args( ( void ) ); /* Oxtal 1997 - hints.c */
// Added by SinaC 2000
void    burning_update    args( ( void ) );
void    underwater_update args( ( void ) ); 
//void    grenade_update    args( ( void ) );    Removed by SinaC 2003, can be emulate with script
void    lightning_update  args( ( void ) );
void	teleport_update   args( ( void ) );
void    quake_update      args( ( void ) );
// Added by SinaC 2001
void    daylight_update args( ( void ) );

// Added by SinaC 2000 to avoid a segfault with a player trying to connect who has
//  a pet
bool    valid_update_target args( ( CHAR_DATA *ch, bool touch_imm ) );

// Added by SinaC 2000 for noaggr_damage
int  silent_damage( CHAR_DATA *victim, int dam, int dam_type );
void silent_update( CHAR_DATA *victim );
void silent_kill( CHAR_DATA *victim, const char *death_msg );
int noaggr_damage( CHAR_DATA *victim, int dam, int dam_type,
		    const char *dam_msg_vict, const char *dam_msg_other,
		    const char *death_msg, 
		    bool showdam );

// Used for global earthquake, SinaC 2000
bool    earthquake_on = FALSE;
int     earthquake_duration;

/* used for saving */

int     save_number = 0;

/* for information */
long    ticks_elapsed = 0;

/* Auction variables */

int        aucstat=0;
CHAR_DATA *aucfrom;
OBJ_DATA  *aucobj;
int        lastbet=0;
int        betgold,betsilver;
/* By oxtal : gold and silver are not changed by auctioning*/
CHAR_DATA *lastbetter;

// Added by SinaC 2000 to avoid a segfault with a player trying to connect who has
//  a pet
/*
 * Check if the argument ch is a valid target for an update
 */
bool valid_update_target( CHAR_DATA *ch, bool touch_imm ) {
  // if the player doesn't not exist or is not in a valid room ==> FALSE
  if ( ch == NULL || ch->in_room == NULL || !ch->valid ) 
    return FALSE;

  // Modified by SinaC 2001
  // Some additionnal tests
  if ( IS_NPC( ch )
       && ( IS_SET(ch->in_room->cstat(flags),ROOM_PET_SHOP)
	    || IS_SET(ch->in_room->cstat(flags),ROOM_SAFE)
	    || IS_SET(ch->in_room->cstat(flags),ROOM_BANK)
	    || ch->pIndexData->pShop != NULL
	    // Added by SinaC 2001
	    ||   IS_SET(ch->act, ACT_IS_SAFE )
	    
	    || IS_SET(ch->act,ACT_TRAIN)
	    || IS_SET(ch->act,ACT_PRACTICE)
	    || IS_SET(ch->act,ACT_IS_HEALER) ) )
	    //|| IS_SET(ch->act,ACT_IS_CHANGER)
	    //|| ch->spec_fun == spec_lookup( "spec_questmaster" ) ) )
    return FALSE;

  // a mob who is not the pet of a not connected player with a descriptor   
  //  or a player with a descriptor, connected and not immortal
  
  // a mob
  if ( IS_NPC( ch ) ){
    // mob has a master or leader
    if ( ch->master != NULL ){
      // has to be a pet
      if ( ch->master->pet != NULL && ch->master->pet == ch ) {
	// the master is a PC, has a descriptor and is connected
	if ( !IS_NPC(ch) && ch->master->desc!= NULL 
	     && ch->master->desc->connected == CON_PLAYING )
	  return TRUE;
      }
      // not a pet
      else
	return TRUE;
    }
    else if ( ch->leader != NULL ){
      // has to be a pet
      if ( ch->leader->pet != NULL && ch->leader->pet == ch ) {
	// the master is a PC, has a descriptor and is connected
	if ( !IS_NPC(ch) && ch->leader->desc != NULL 
	     && ch->leader->desc->connected == CON_PLAYING )
	  return TRUE;
      }
      // not a pet
      else
	return TRUE;
    }
    // no master, no leader
    else
      return TRUE;
  }
  // a player
  else{
    // descriptor exist and connected
    if ( ch->desc != NULL && ch->desc->connected == CON_PLAYING ){
      // if !( immortal && touch_imm==FALSE )
      if( !(IS_IMMORTAL(ch) && touch_imm == FALSE ) )
	return TRUE;
    }
  }

  return FALSE;
}

/*
 * Some stuff for noaggr_damage
 *  return value has been changed to int, to see available value: check fight.h
 */
int noaggr_damage( CHAR_DATA *victim, int dam, int dam_type,
		    const char *dam_msg_vict, const char *dam_msg_other,
		    const char *death_msg, 
		    bool showdam ) {
  int dam_done;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  dam_done = silent_damage( victim, dam, dam_type );
  if ( dam_done != 0 ){
    if ( dam_msg_vict != NULL && dam_msg_vict[0] != '\0' ){
      //	  sprintf( buf, "%s\n\r", dam_msg_vict );
      sprintf( buf, "%s", dam_msg_vict );
      if ( showdam == TRUE ){
	sprintf( buf2, " [%d]", dam_done );
	strcat( buf, buf2 );
      }
      strcat( buf, "\n\r" );
      send_to_char( buf, victim );
    }
    if ( dam_msg_other != NULL && dam_msg_other[0] != '\0' ){
      sprintf( buf, "%s", dam_msg_other );
      act( buf , victim, NULL, NULL, TO_ROOM );
    }
  }
  silent_update( victim );
  if ( victim->position == POS_DEAD ) { // Added by SinaC 2003
    silent_kill( victim, death_msg );
    return DAMAGE_DEADLY;
  }
  if ( dam_done == 0 ) 
    return DAMAGE_NOTDONE;
  return DAMAGE_DONE;
}

// do the damage
int silent_damage( CHAR_DATA *victim, int dam, int dam_type ) {
  if ( victim->position == POS_DEAD )
    return 0;

  switch(check_immune(victim,dam_type)){
  case(IS_IMMUNE):
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

  if (dam == 0) 
    return 0;
  
  /*
   * Hurt the victim.
   * Inform the victim of his new state.
   */
  victim->hit -= dam;

  // Added by SinaC 2003 for phylactery
  // When affected by phylactery, victim has a chance to cheat death and get some hp back
  if ( victim->hit < 1 ) {
    bool check = check_phylactery( victim );
    if ( !check )
      if ( check_strategic_retreat( victim ) )
        return 0; // damage are done but victim is transfered so we considered dam as not done
  }

  if ( !IS_NPC(victim)
       && IS_IMMORTAL( victim )
       && victim->hit < 1 )
    victim->hit = 1;
  update_pos( victim );
  
  return dam;
}

// show the position message
void silent_update( CHAR_DATA *victim ) {
  switch( victim->position ){
  case POS_MORTAL:
    act( "$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are mortally wounded, and will die soon, if not aided.\n\r", victim );
    break;
    
  case POS_INCAP:
    act( "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You are incapacitated and will slowly die, if not aided.\n\r", victim );
    break;
    
    // Added by SinaC 2003 for paralyzing
  case POS_PARALYZED:
    act( "$n is paralyzed.", victim, NULL, NULL, TO_ROOM );
    send_to_char("You are paralyzed.\n\r", victim );
    break;

  case POS_STUNNED:
    act( "$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM );
    send_to_char("You are stunned, but will probably recover.\n\r", victim );
    break;
    
  case POS_DEAD:
    act( "$n is DEAD!!", victim, 0, 0, TO_ROOM );
    send_to_char( "You are DEAD!!\n\r\n\r", victim );
    break;
    
  default:
    if ( victim->hit < victim->cstat(max_hit) / 4 )
      send_to_char( "You sure are BLEEDING!\n\r", victim );
    break;
  }
}

// kill the victim without any aggressor
void silent_kill( CHAR_DATA *victim, const char *death_msg ) {
  char *msg;
  int explost;
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *was_in_room;
  ROOM_INDEX_DATA *location;
  int door;
  OBJ_DATA *corpse;

  if ( IS_IMMORTAL(victim) )
    return;

  // Added by SinaC 2003 for phylactery
  // When affected by phylactery, victim has a chance to cheat death and get some hp back
  if ( check_phylactery( victim ) )
    return;

  /*
   * Payoff for killing things.  Removed by SinaC 2003, checked before calling
   */
  //if ( victim->position != POS_DEAD ) return;

  // Added by SinaC 2000, so the hunter stops hunting his prey when his prey is
  //  dead
  remove_hunter( victim );

  if ( !IS_NPC(victim) ){
    sprintf( log_buf, "%s %s at %d",
	     victim->name,
	     death_msg,
	     victim->in_room->vnum );
    log_string( log_buf );

    /*
     * Dying penalty:
     * 2/3 way back to previous level.
     */
    if (!IN_BATTLE(victim)){
      if ( victim->exp > exp_per_level(victim,victim->pcdata->points) 
	   * victim->level ){	        
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
    
    sprintf( buf, "{rINFO: %s %s{x\n\r", victim->name, 
	     death_msg );
    info( buf );         
  }

  sprintf( log_buf, "%s %s at %s [room %d]",
	   (IS_NPC(victim) ? victim->short_descr : victim->name),
	   death_msg,
	   victim->in_room->name, victim->in_room->vnum);
  
  if (IS_NPC(victim))
    wiznet(log_buf,NULL,NULL,WIZ_MOBDEATHS,0,0);
  else
    wiznet(log_buf,NULL,NULL,WIZ_DEATHS,0,0);
  
  stop_fighting( victim, TRUE );

  // Added by SinaC 2003
  Value args[] = { victim };
  victim->position = POS_RESTING; // to avoid problem with act
  int result = mobprog( victim, NULL, "onKilled", args );
  victim->position = POS_DEAD;
  // result: 0     :   everything is done as before, nothing special
  //      bit 0 set:  don't call death_cry
  //      bit 1 set:  don't call make_corpse

  // If onKilled result has not bit NODEATHCRY (bit 0), death_cry is called
  if ( !IS_SET_BIT( result, 0 ) ) {
    if ( IS_NPC(victim) ) msg = "You hear something's death cry.";
    else msg = "You hear someone's death cry.";
    
    ROOM_INDEX_DATA *was_in_room = victim->in_room;
    for ( door = 0; door < MAX_DIR; door++ ){  // Modified by SinaC 2003
      EXIT_DATA *pexit;
      
      if ( ( pexit = was_in_room->exit[door] ) != NULL
	   &&   pexit->u1.to_room != NULL
	   &&   pexit->u1.to_room != was_in_room ){
	victim->in_room = pexit->u1.to_room;
	act( msg, victim, NULL, NULL, TO_ROOM );
      }
    }
    victim->in_room = was_in_room;
  }

  // Added by SinaC 2003 for MOUNT code, when a bonded mount dies
  //  its master really really suffers
  mount_dying_drawback( victim );

  if ( IS_NPC(victim) ){
    // If onKilled result has not bit NOCORPSE (bit 1), make_corpse is called, added by SinaC 2003
    if ( !IS_SET_BIT( result, 1 ) )
      //make_corpse( victim, corpse, victim );
      make_corpse( victim, corpse, victim, 0 ); // SinaC 2003
    victim->pIndexData->killed++;
    kill_table[URANGE(0, victim->level, MAX_LEVEL-1)].killed++;
    extract_char( victim, TRUE );
    
    return;
  }

  // SinaC 2000 for Arena
  if ( !IN_BATTLE(victim)){
    //make_corpse( victim, corpse, victim );
    make_corpse( victim, corpse, victim, 0 ); // SinaC 2003
    extract_char( victim, FALSE );
    
    // Added by SinaC 2000 for stay_death item, done in extract_char now, SinaC 2001
    //transfer_obj_stay_death( victim, corpse );
  }
  /* Modified by SinaC 2001
  while ( victim->affected )
    affect_remove( victim, victim->affected );
  */
  AFFECT_DATA *paf, *paf_next;
  for ( paf = victim->affected; paf; paf = paf_next ) {
    paf_next = paf->next;
    //if ( paf->duration == DURATION_PERMANENT ) // PERMANENT affect stays when affected died
    if ( IS_SET( paf->flags, AFFECT_STAY_DEATH ) ) // SinaC 2003: new affect system
      continue;
    affect_remove( victim, victim->affected );
  }

    
  if ( !IN_BATTLE(victim)){
    victim->position	= POS_RESTING;
    victim->hit	= UMAX( 1, victim->hit  );
    victim->mana	= UMAX( 1, victim->mana );
    victim->move	= UMAX( 1, victim->move );
    // Added by SinaC 2001 for mental user
    victim->psp	= UMAX( 1, victim->psp );
  }
  else{
    victim->position	= POS_STANDING;
    victim->hit	= victim->cstat( max_hit  );
    victim->mana	= victim->cstat( max_mana );
    victim->move	= victim->cstat( max_move );
    // Added by SinaC 2001 for mental user
    victim->psp	= victim->cstat( max_psp );
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
  //  //location = get_room_index( hometown_table[victim->pcdata->hometown].hall ); 
  location = get_hall_room( victim ); // SinaC 2003
  char_from_room( victim );
  char_to_room( victim, location );

  check_rebirth(victim);

  return;
}


/*
 * Advancement stuff.               modified by Sinac 1997
 */
void advance_level( CHAR_DATA *ch, bool hide ) {
  char buf[MAX_STRING_LENGTH];
  int add_hp;
  int add_mana;
  int add_move;
  int add_prac;
  int min_mana;
  int max_mana;
  // Added by SinaC 2001 for mental user
  int add_psp;
  int min_psp;
  int max_psp;

  ch->pcdata->last_level = 
    ( ch->played + (int) (current_time - ch->logon) ) / 3600;

  if (IS_SET( ch->act, PLR_AUTOTITLE )){
    sprintf( buf, "the %s",
	     title_table[class_firstclass(ch->cstat(classes))][ch->level][ch->cstat(sex) == SEX_FEMALE ? 1 : 0] );
    set_title( ch, buf );
  }
  add_hp	= con_app[ch->cstat(CON)].hitp 
    + number_range( class_hpmin(ch->cstat(classes)), class_hpmax(ch->cstat(classes)));
    

  /*
   * add_mana 	= number_range(2,(2*get_curr_stat(ch,STAT_INT)
   *				  + get_curr_stat(ch,STAT_WIS))/5);
   */

  min_mana = UMIN( ch->cstat(WIS)/2, ch->cstat(INT) );
  max_mana = UMAX( ch->cstat(WIS)/2, ch->cstat(INT) );
  add_mana = number_range( min_mana, max_mana );

  // Added by SinaC 2001 for mental user
  min_psp = UMIN( ch->cstat(INT)/2, ch->cstat(WIS) );
  max_psp = UMAX( ch->cstat(INT)/2, ch->cstat(WIS) );
  add_psp = number_range( min_psp, max_psp );

  // was /= 2 before
  if (!class_fMana(ch->cstat(classes)))
    add_mana /= 4;
  else
    add_mana = (add_mana*4)/3;

  // Added by SinaC 2001 for mental user, was /= 2 before
  if (!class_fPsp(ch->cstat(classes)))
    add_psp /= 4;
  else
    add_psp = (add_psp*4)/3;


  add_move	= number_range( 1, (ch->cstat(CON)
				    + ch->cstat(DEX))/6 );
  add_prac	= wis_app[ch->cstat(WIS)].practice;

  // Modified by SinaC 2001
  //add_hp = add_hp * 9/10;
  //add_mana = add_mana * 9/10;
  //add_move = add_move * 9/10;

  add_hp	= UMAX(  2, add_hp   );
  add_mana	= UMAX(  2, add_mana );
  add_move	= UMAX(  6, add_move );
  //Added by SinaC 2001 for mental user
  add_psp	= UMAX(  2, add_psp );

  ch->bstat(max_hit) 	+= add_hp;
  ch->bstat(max_mana)	+= add_mana;
  ch->bstat(max_move)	+= add_move;
  //Added by SinaC 2001 for mental user
  ch->bstat(max_psp)	+= add_psp;
  ch->practice	        += add_prac;
  ch->train		+= 1;

  recompute(ch);

  if (!hide){
    sprintf( buf, "{rINFO: %s raises a level. Now up to level %d.{x\n\r", ch->name, ch->level );
    info( buf );
    //Modified by SinaC 2001 for mental user
    /*
    sprintf(buf,
	    "You gain %d hit point%s, %d mana, %d move, and %d practice%s.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_move,
	    add_prac, add_prac == 1 ? "" : "s");
    */
    sprintf(buf,
	    "You gain %d hit point%s, %d mana, %d psp, %d move, and %d practice%s.\n\r",
	    add_hp, add_hp == 1 ? "" : "s", add_mana, add_psp, add_move,
	    add_prac, add_prac == 1 ? "" : "s");

    send_to_char( buf, ch );
  }
  return;
}


// Modified by SinaC 2001
void gain_exp( CHAR_DATA *ch, int gain, bool silent ) {
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) || ch->level >= LEVEL_HERO ) {
    if ( double_xp == TRUE && gain > 0 )
      if ( !silent )
	send_to_charf( ch, "{RDOUBLE XP: you received %d experience points.{x\n\r", gain );
    return;
  }
  
  // Added by SinaC 2001
  if ( double_xp == TRUE && gain > 0 ) {
    if ( !silent )
      send_to_charf( ch, "{RDOUBLE XP: you received %d experience points.{x\n\r", gain );
    gain *= 2;
  }

  ch->exp = UMAX( exp_per_level(ch,ch->pcdata->points), ch->exp + gain );
  while ( ch->level < LEVEL_HERO && ch->exp >= 
	  exp_per_level(ch,ch->pcdata->points) * (ch->level+1) ){
    send_to_char( "You raise a level!!  ", ch );
    ch->level += 1;
    sprintf(buf,"%s gained level %d",ch->name,ch->level);
    log_string(buf);
    sprintf(buf,"$N has attained level %d!",ch->level);
    wiznet(buf,ch,NULL,WIZ_LEVELS,0,0);
    advance_level(ch,FALSE);
    //save_char_obj(ch);
    new_save_pFile(ch,FALSE);
  }

  //recompute(ch); NO NEED: done in advance_level
  return;
}



/*
 * Regeneration stuff.
 */
int hit_gain( CHAR_DATA *ch ) {
  int gain;
  int number;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) ){

    gain =  5 + ch->level;
    if (IS_AFFECTED(ch,AFF_REGENERATION))
      gain *= 2;
    
    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
    
    switch(ch->position){
    default : 		gain /= 2;			break;
    case POS_SLEEPING: 	gain = 3 * gain/2;		break;
    case POS_RESTING:  	               		        break;
    case POS_FIGHTING:	gain /= 3;		 	break;
    }
  }
  else{
    gain = UMAX(3,ch->cstat(CON) - 3 + ch->level/2);
    gain += class_hpmax(ch->cstat(classes)) - 10;
    number = number_percent();
    if (number < get_ability(ch,gsn_fast_healing)){
      gain += number * gain / 100;
      if (ch->hit < ch->cstat(max_hit))
	check_improve(ch,gsn_fast_healing,TRUE,8);
    }
    if ( ch->cstat(CON) <= 3 ) // SinaC 2003, low constitution means no regeneration
      gain = 0;
    else if ( ch->cstat(CON) <= 10 )
      gain /= 2;
    
    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
    
    /****** Added by Sinac 1997, for troll PC race ****/
    
    if (IS_AFFECTED(ch,AFF_REGENERATION))
      gain *= 2;
    
    /*************************************************/    
    
    switch ( ch->position ){
    default:	   	gain /= 4;			break;
    case POS_SLEEPING: 					break;
    case POS_RESTING:  	gain /= 2;			break;
    case POS_FIGHTING: 	gain /= 6;			break;
    }
    
    if ( ch->pcdata->condition[COND_HUNGER] == 0 )
      gain /= 2;
    
    if ( ch->pcdata->condition[COND_THIRST] == 0 )
      gain /= 2;
  }

  // Modified by SinaC 2001
  gain *= ch->in_room->cstat(healrate) / 100;
    
  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[3] / 100;

  if ( IS_AFFECTED(ch, AFF_POISON) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /= 2;

  return UMIN(gain, ch->cstat(max_hit) - ch->hit);
}

int mana_gain( CHAR_DATA *ch ) {
  int gain;
  int number;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) ){
    // Modified by SinaC 2000
    //gain = 5 + ch->level;
    gain = 5 + 3*ch->level;

    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
    
    switch (ch->position){
    default:		gain /= 2;		break;
    case POS_SLEEPING:	gain = 3 * gain/2;	break;
    case POS_RESTING:				break;
    case POS_FIGHTING:	gain /= 3;		break;
    	}
  }
  else{

    // Modified by SinaC 2001, no *2 before
    gain = (ch->cstat(WIS)
	    + ch->cstat(INT)*2 + ch->level) / 2;
    number = number_percent();
    if (number < get_ability(ch,gsn_meditation)){
      gain += number * gain / 100;
      if (ch->mana < ch->cstat(max_mana))
	check_improve(ch,gsn_meditation,TRUE,8);
      // SinaC 2003, meditation level 2
      if ( get_casting_level( ch, gsn_meditation ) == 2 ) {  // level 2?
	gain += number * gain / 150;
	if (ch->mana < ch->cstat(max_mana))
	  check_improve(ch,gsn_meditation,TRUE,12);
      }
    }
    if (!class_fMana(ch->cstat(classes)))
      gain /= 2;
    
    /*
    // Campfire doubles gain 
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */

    switch ( ch->position ){
    default:		gain /= 4;			break;
    case POS_SLEEPING: 					break;
    case POS_RESTING:	gain /= 2;			break;
    case POS_FIGHTING:	gain /= 6;			break;
    }
    
    if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
      gain /= 2;
    
    if ( ch->pcdata->condition[COND_THIRST] == 0 )
      gain /= 2;
  }

  // Modified by SinaC 2001
  gain = gain * ch->in_room->cstat(manarate) / 100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[4] / 100;
  
  if ( IS_AFFECTED( ch, AFF_POISON ) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /= 2;

  return UMIN(gain, ch->cstat(max_mana) - ch->mana);
}

//Added by SinaC 2001 for mental user
int psp_gain( CHAR_DATA *ch ) {
  int gain;
  int number;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) ){
    // Modified by SinaC 2000
    //gain = 5 + ch->level;
    gain = 5 + 3*ch->level;
    
    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
    
    switch (ch->position){
    default:		gain /= 2;		break;
    case POS_SLEEPING:	gain = 3 * gain/2;	break;
    case POS_RESTING:				break;
    case POS_FIGHTING:	gain /= 3;		break;
    	}
  }
  else{
      
    // Modified by SinaC 2001,  no *2 before
    gain = (ch->cstat(WIS)*2
	    + ch->cstat(INT) + ch->level) / 2;
    number = number_percent();
    if (number < get_ability(ch,gsn_concentration)){
      gain += number * gain / 100;
      if (ch->psp < ch->cstat(max_psp))
	check_improve(ch,gsn_concentration,TRUE,8);
    }
    if (!class_fPsp(ch->cstat(classes)))
      gain /= 2;
    
    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
    
    switch ( ch->position ){
    default:		gain /= 4;			break;
    case POS_SLEEPING: 					break;
    case POS_RESTING:	gain /= 2;			break;
    case POS_FIGHTING:	gain /= 6;			break;
    }
    
    if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
      gain /= 2;
    
    if ( ch->pcdata->condition[COND_THIRST] == 0 )
      gain /= 2;
  }

  // Modified by SinaC 2001
  gain = gain * ch->in_room->cstat(psprate) / 100;

  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[4] / 100;
  
  if ( IS_AFFECTED( ch, AFF_POISON ) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /= 2;

  return UMIN(gain, ch->cstat(max_psp) - ch->psp);
}


int move_gain( CHAR_DATA *ch ) {
  int gain;

  if (ch->in_room == NULL)
    return 0;

  if ( IS_NPC(ch) ){
    
    gain = ch->level;

    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
  }
  else{
    gain = UMAX( 15, ch->level );
    
    /*
    // Campfire doubles gain
    if(ch->in_room)
      if(get_obj_list( ch, "campfire", ch->in_room->contents ))
	gain *= 2;
    */
    
    switch ( ch->position ){
    case POS_SLEEPING: gain += ch->cstat(DEX);	break;
    case POS_RESTING:  gain += ch->cstat(DEX) / 2;	break;
    }

    if ( ch->pcdata->condition[COND_HUNGER]   == 0 )
      gain /= 2;
    
    if ( ch->pcdata->condition[COND_THIRST] == 0 )
      gain /= 2;
  }
  
  // Modified by SinaC 2001
  gain = gain * ch->in_room->cstat(healrate)/100;
  
  if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE)
    gain = gain * ch->on->value[3] / 100;

  if ( IS_AFFECTED(ch, AFF_POISON) )
    gain /= 4;

  if (IS_AFFECTED(ch, AFF_PLAGUE))
    gain /= 8;

  if (IS_AFFECTED(ch,AFF_HASTE) || IS_AFFECTED(ch,AFF_SLOW))
    gain /= 2;

  return UMIN(gain, ch->cstat(max_move) - ch->move);
}

void gain_condition( CHAR_DATA *ch, int iCond, int value ) {
  int condition;
  int dam;

  if ( value == 0 || IS_NPC(ch) || IS_IMMORTAL(ch))
    return;

  // Added by SinaC 2001
  if ( IS_SET(ch->comm, COMM_AFK ) )
    return;

  // Added by SinaC 2001
  if ( ch->in_room == NULL 
       || ch->in_room->vnum == ROOM_VNUM_LIMBO )
    return;
  
  // Added by SinaC 2001, undead and ghost-like doesn't need to eat,drink
  if ( IS_SET( ch->cstat(form), FORM_UNDEAD ) 
       || IS_SET( ch->cstat(form), FORM_INTANGIBLE ) )
    return;

  condition				= ch->pcdata->condition[iCond];
  if (condition == -1)
    return;
  ch->pcdata->condition[iCond]	= URANGE( 0, condition + value, 48 );

  if ( ch->pcdata->condition[iCond] == 0 ){
    switch ( iCond ){
    case COND_HUNGER:
      send_to_char( "You are hungry.\n\r",  ch );
      // Added by SinaC 2001, so player takes dam from being hungry
      if ( ch->level > 5 ) {
	if ( ch->level < 30 )
	  dam = URANGE( 2, ch->cstat(max_hit)/30, 25 );
	else
	  dam = URANGE( 2, ch->cstat(max_hit)/15, 50 );
        ch->move -= dam/2;
	noaggr_damage( ch, dam, DAM_NONE,
		       "You suffer from starvation.", NULL,
		       "is dead from starvation.",
		       FALSE );
      }
      break;
      
    case COND_THIRST:
      send_to_char( "You are thirsty.\n\r", ch );
      // Added by SinaC 2001, so player takes dam from being thirsty
      // minimum 1, maximum 25
      if ( ch->level > 5 ) {
	if ( ch->level < 30 )
	  dam = URANGE( 1, ch->cstat(max_hit)/20, 25 );
	else
	  dam = URANGE( 1, ch->cstat(max_hit)/10, 50 );
        ch->move -= dam/2;
	noaggr_damage( ch, dam, DAM_NONE,
		       "You suffer from starvation.", NULL,
		       "is dead from starvation.",
		       FALSE );
      }
      break;
	  
    case COND_DRUNK:
      if ( condition != 0 )
	send_to_char( "You are sober.\n\r", ch );
      break;
    }
  }
  return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Merc cpu time.
 * -- Furey
 */
void mobile_update( void ) {
  CHAR_DATA *ch;
  CHAR_DATA *ch_next;
  EXIT_DATA *pexit;
  int door;

  /* Examine all mobs. */
  for ( ch = char_list; ch != NULL; ch = ch_next ){
    ch_next = ch->next;

    // Added by SinaC 2001, we call the script even if there is none in the area, and also on player
    if ( ch->in_room != NULL )
      MOBPROG( ch, NULL, "onPulseMobile" );

    // Modified by SinaC 2003 for MOUNT code
    // When a mount is separated from its partner
    //  It found it back, easier if the mount is bonded
    //if ( !IS_NPC(ch) || ch->in_room == NULL || IS_AFFECTED(ch,AFF_CHARM))
    //  continue;
    if ( !IS_NPC(ch) || ch->in_room == NULL )
      continue;
    //    CHAR_DATA *master = get_mount_master(ch);  SinaC 2003, moved in a script
    //    if ( master != NULL
    //	 && master->in_room != NULL
    //	 && ch->in_room != master->in_room )
    //      if ( is_affected( ch, gsn_bond )
    //	   || chance(50) ) 
    //	mount_to_master( ch, master, FALSE );
    if( IS_AFFECTED(ch,AFF_CHARM))
      continue;


    if (ch->in_room->area->empty && !IS_SET(ch->act,ACT_UPDATE_ALWAYS))
      continue;

    /* Examine call for special procedure */
    if ( ch->spec_fun != 0 ){
      if ( (*ch->spec_fun) ( ch ) )
	continue;
    }

    if (ch->pIndexData->pShop != NULL) /* give him some gold */
      if ((ch->gold * 100 + ch->silver) < ch->pIndexData->wealth){
	ch->gold += ch->pIndexData->wealth * number_range(1,20)/5000000;
	ch->silver += ch->pIndexData->wealth * number_range(1,20)/50000;
      }

    /* That's all for sleeping / busy monster, and empty zones */
    if ( ch->position != POS_STANDING )
      continue;

    /* Scavenge */
    if ( IS_SET(ch->act, ACT_SCAVENGER)
	 &&   ch->in_room->contents != NULL
	 &&   number_bits( 6 ) == 0 ){
      OBJ_DATA *obj;
      OBJ_DATA *obj_best;
      int max;

      max         = 1;
      obj_best    = 0;
      for ( obj = ch->in_room->contents; obj; obj = obj->next_content ){
	if ( CAN_WEAR(obj, ITEM_TAKE) && can_loot(ch, obj)
	     && obj->cost > max  && obj->cost > 0){
	  obj_best    = obj;
	  max         = obj->cost;
	}
      }

      if ( obj_best ){
	obj_from_room( obj_best );
	obj_to_char( obj_best, ch );
	act( "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
      }
    }

    /* Wander */
    if ( !IS_SET(ch->act, ACT_SENTINEL)
	 && number_bits(3) == 0
	 && ( door = number_bits( 5 ) ) < MAX_DIR // Modified by SinaC 2003
	 && ( pexit = ch->in_room->exit[door] ) != NULL
	 && pexit->u1.to_room != NULL
	 && !IS_SET(pexit->exit_info, EX_CLOSED)
	 // Modified by SinaC 2001
	 && !IS_SET(pexit->u1.to_room->cstat(flags), ROOM_NO_MOB)
	 && (   !IS_SET(ch->act, ACT_STAY_AREA)
		|| pexit->u1.to_room->area == ch->in_room->area )
	 && (   !IS_SET(ch->act, ACT_OUTDOORS)
		|| !IS_SET(pexit->u1.to_room->cstat(flags),ROOM_INDOORS) )
	 && (   !IS_SET(ch->act, ACT_INDOORS)
		|| IS_SET(pexit->u1.to_room->cstat(flags),ROOM_INDOORS) ) )
      move_char( ch, door, FALSE, TRUE, FALSE ); // SinaC 2003 );
  }

  return;
}


/*
 * Update all chars, including mobs.
 */

#define SAVE_TICK 20

// Redone by SinaC 2003
void char_update() {
  save_number++;
  ticks_elapsed++;

  if ( save_number >= SAVE_TICK )
    save_number = 0;

  CHAR_DATA *ch_quit = NULL;
  CHAR_DATA *ch_next;
  for ( CHAR_DATA *ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;

    if ( !ch->valid )
      continue;

    // Added by SinaC 2001
    if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_TICK ) )
      send_to_char("TICK\n\r", ch );

    // idle for too long
    if ( ch->timer > 30 )
      ch_quit = ch;

    // if character is conscient: check need home  and  regenerate hit/mana/move/psp
    if ( ch->position >= POS_STUNNED ) {
      // check to see if we need to go home
      // ch->zone is set in reset_room, before char_to_room, SinaC 2000
      if (IS_NPC(ch)
	  && ch->in_room != NULL
	  && ch->zone != NULL && ch->zone != ch->in_room->area
	  && ch->desc == NULL &&  ch->fighting == NULL
	  && !IS_AFFECTED(ch,AFF_CHARM)
	  // Can leave an area without being extract, SinaC 2001
	  && !IS_SET( ch->act, ACT_FREE_WANDER )
	  && number_percent() < 5 ) {
	act("$n wanders on home.",ch,NULL,NULL,TO_ROOM);
	log_stringf("char_update: %s (vnum: %d) in room %d wanders on home.",
		    ch->short_descr, ch->pIndexData->vnum,
		    ch->in_room->vnum);
	extract_char(ch,TRUE);
	continue;
      }

      // SinaC 2003, charmies disappear after a while
      if ( IS_NPC(ch)
	   && ch->in_room != NULL
	   && ch->fighting == NULL
	   && ch->desc == NULL
	   && !IS_SET( ch->in_room->cstat(flags), ROOM_PET_SHOP )
	   && IS_AFFECTED( ch, AFF_CHARM )
	   && ch->master == NULL
	   && ch->leader == NULL ) {
	//act("$n disappears.", ch, NULL, NULL, TO_ROOM );
	//log_stringf("char_update: charmies %s (vnum: %d) in room %d disappears.",
	//ch->short_descr, ch->pIndexData->vnum,
	//    ch->in_room->vnum );
	//extract_char(ch, TRUE );
	log_stringf("char_update: charmies %s (vnum: %d) in room %d has no master/leader",
		    ch->short_descr, ch->pIndexData->vnum,
		    ch->in_room->vnum );
	continue;
      }

      // Regeneration stuff
      if ( ch->hit  < ch->cstat(max_hit) )  ch->hit  += hit_gain(ch);
      else                                  ch->hit = ch->cstat(max_hit);
      if ( ch->mana < ch->cstat(max_mana) ) ch->mana += mana_gain(ch);
      else                	            ch->mana = ch->cstat(max_mana);
      if ( ch->move < ch->cstat(max_move) ) ch->move += move_gain(ch);
      else     	                            ch->move = ch->cstat(max_move);
      if ( ch->psp < ch->cstat(max_psp) )   ch->psp += psp_gain(ch);
      else                                  ch->psp = ch->cstat(max_psp);
    }

    // when STUNNED or PARALYZED: ??? WHY? FIXME
    if ( ch->position == POS_STUNNED
	 || ch->position == POS_PARALYZED ) // Modified by SinaC 2003
      update_pos( ch );

    // Some stupid code
    if (IS_IMMORTAL(ch)) // immortals don't go in the void
      ch->timer = 0;

    // Non-immortal player have their light's timer decreasing
    if ( !IS_NPC(ch) && !IS_IMMORTAL(ch) ) {
      OBJ_DATA *obj;
      // Check for lit off
      if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL // wear an item in light loc
	   && obj->item_type == ITEM_LIGHT // it's a light
	   && obj->value[2] > 0 ) { // with a limited duration
	if ( --obj->value[2] == 0 && ch->in_room != NULL ) { //decrease duration
	  // Modified by SinaC 2001
	  --ch->in_room->bstat(light);
	  act( "$p goes out.", ch, obj, NULL, TO_ROOM );
	  act( "$p flickers and goes out.", ch, obj, NULL, TO_CHAR );
	  extract_obj( obj );
	  recomproom( ch->in_room );
	}
	else if ( obj->value[2] <= 5 && ch->in_room != NULL) // warn player about his/her light low duration
	  act("$p flickers.",ch,obj,NULL,TO_CHAR);
      }
      // Check void disappearing
      if ( ++ch->timer >= 12 ) { // update player timer and if timer >= 12 go to void
	if ( ch->was_in_room == NULL && ch->in_room != NULL ) {
	  ch->was_in_room = ch->in_room;
	  if ( ch->fighting != NULL )
	    stop_fighting( ch, TRUE );
	  act( "$n disappears into the void.",
	       ch, NULL, NULL, TO_ROOM );
	  send_to_char( "You disappear into the void.\n\r", ch );
	  if (ch->level > 1)
	    //save_char_obj( ch );
	    new_save_pFile(ch,FALSE);
	  char_from_room( ch );
	  char_to_room( ch, get_room_index( ROOM_VNUM_LIMBO ) );
	}
      }

      // Update conditions
      gain_condition( ch, COND_DRUNK,  -1 ); // decrease drunk state
      // Added by SinaC 2001 so undead doesn't need to eat, drink
      if ( !IS_SET(ch->cstat(form), FORM_UNDEAD) ) {
	gain_condition( ch, COND_FULL, ch->cstat(size) > SIZE_MEDIUM ? -4 : -2 );
	gain_condition( ch, COND_THIRST, -1 );
	gain_condition( ch, COND_HUNGER, ch->cstat(size) > SIZE_MEDIUM ? -2 : -1);
      }
    } // end of non-immortal player code

    // Added by SinaC 2001
    bool afrem = FALSE;
    AFFECT_DATA *paf_next;
    for ( AFFECT_DATA *paf = ch->affected; paf != NULL; paf = paf_next ) {
      paf_next	= paf->next;
      if ( !IS_SET( paf->flags, AFFECT_IS_VALID ) )
	continue;
      if ( paf->duration == 0 ) { // wearoff
	if ( paf->type > 0 ) { // if affect created by an ability
	  if ( ability_table[paf->type].wearoff_fun != NULL  // wearoff function ?
	       && ability_table[paf->type].wearoff_fun != wearoff_null )
	    (*ability_table[paf->type].wearoff_fun) ( paf, (void*)ch, TARGET_CHAR );
	  else if ( ability_table[paf->type].msg_off  // wearoff message ?
		    && str_cmp(ability_table[paf->type].msg_off, "none") ){
	    send_to_char( ability_table[paf->type].msg_off, ch );
	    send_to_char( "\n\r", ch );
	  }
	  else // no wearoff function neither message: error
	    bug("ability %s needs a msg_off", ability_table[paf->type].name);
	}
	affect_remove_no_recompute( ch, paf ); // wearoff -> remove affect
	afrem = TRUE;
      }
      else { // duration is infinite or finite :)) but not equal to 0
	if ( paf->type > 0 //&& IS_SET( paf->flags, AFFECT_ABILITY ) // update function ?
	     && ability_table[paf->type].update_fun != NULL 
	     && ability_table[paf->type].update_fun != update_null )
	  (*ability_table[paf->type].update_fun) ( paf, (void*)ch, TARGET_CHAR );
	if ( paf->duration > 0 ) { // finite duration: decrease it
	  paf->duration--;
	  if (number_range(0,4) == 0 && paf->level > 0 )
	    paf->level--;  // ability strength fades with time
	}
      }
      if ( !ch->valid ) // no need to continue if ch has been destroyed by a wearoff/update function
	break;
    }
    if ( !ch->valid ) // no need to continue if ch doesn't exist anymore
      continue;

    if (afrem) // if some affects have been removed: recompute
      recompute(ch);

    if ( ch == NULL || ch->in_room == NULL || !ch->valid ) // check ch's validity
      continue;

    // Incapacitated player slowly died
    if ( ( ch->position == POS_INCAP && number_range(0,1) == 0)
	 || ch->position == POS_MORTAL )
      noaggr_damage( ch, 1, DAM_NONE,
		     "You suffer from your wounds.",
		     "$n suffers from $s wounds.",
		     "is dead from lethal wounds.",
		     FALSE );

    // Call mobprogram
    if ( ch != NULL && ch->in_room != NULL && ch->valid )
      MOBPROG( ch, NULL, "onPulseTick" );
  }
  
  // Autosave and autoquit.
  // Check that these chars still exist.
  for ( CHAR_DATA *ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;
    if ( ch->desc && ch->desc->connected == CON_PLAYING 
	 && ( ch->desc->descriptor % SAVE_TICK == save_number ) ) {
      log_stringf("Autosaving %s",ch->name);
      new_save_pFile(ch,FALSE);
    }    
    
    if ( ch == ch_quit )
      do_quit( ch, "" );
  }

  return;
}

// Update all objs.
// This function is performance sensitive.
// Redone by SinaC 2003
void obj_update( void ) {   
  OBJ_DATA *obj_next;
  for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj_next ){
    obj_next = obj->next;

    // Objects in auctions aren't modify. Seytan 1997
    if ( obj == aucobj )
      continue;
    
    if ( !obj->valid )
      continue;

    // go through affects and decrement
    bool afrem = FALSE;
    AFFECT_DATA *paf_next;
    for ( AFFECT_DATA *paf = obj->affected; paf != NULL; paf = paf_next ) {
      paf_next    = paf->next;
      if ( !IS_SET( paf->flags, AFFECT_IS_VALID ) )
	continue;
      if ( paf->duration == 0 ) { // wearoff
	if ( paf->type > 0 ) { // affect created by an ability
	  if ( ability_table[paf->type].wearoff_fun != NULL // wearoff function ?
	       && ability_table[paf->type].wearoff_fun != wearoff_null )
	    (*ability_table[paf->type].wearoff_fun) ( paf, (void*)obj, TARGET_OBJ );
	  else if ( ability_table[paf->type].msg_obj  // wearoff msg ?
		    && str_cmp( ability_table[paf->type].msg_obj , "none") ) {
	    if ( obj->carried_by != NULL ) { // carried item -> message to carrier
	      CHAR_DATA *rch = obj->carried_by;
	      act(ability_table[paf->type].msg_obj,
		  rch,obj,NULL,TO_CHAR);
	    }
	    if (obj->in_room != NULL  // item on the ground -> message to everyone in the room
		&& obj->in_room->people != NULL){
	      CHAR_DATA *rch = obj->in_room->people;
	      act(ability_table[paf->type].msg_obj,
		  rch,obj,NULL,TO_ALL);
	    }
	  }
	  else
	    bug("ability %s needs a msg_obj", ability_table[paf->type].name);
	}
	affect_remove_obj_no_recompute( obj, paf ); // wearoff: remove affect
	afrem = TRUE;
      }
      else { // infinite or finite duration: not a wearoff
	if ( paf->type > 0 //&& IS_SET( paf->flags, AFFECT_ABILITY ) // affect created by an ability
	     && ability_table[paf->type].update_fun != NULL  // update function ?
	     && ability_table[paf->type].update_fun != update_null )
	  (*ability_table[paf->type].update_fun) ( paf, (void*)obj, TARGET_OBJ );

	if ( paf->duration > 0 ) { // finite duration: decrease
	  paf->duration--;
	  if (number_range(0,4) == 0 && paf->level > 0)
	    paf->level--;  // ability strength fades with time
	}
      }
      if ( !obj->valid ) // no need to continue if obj has been destroyed by a wearoff/update function
	break;
    }
    if ( !obj->valid ) // no need to continue if obj is not valid anymore
      continue;

    if ( afrem ) // if affects have been removed: recompute
      recompobj(obj);

    OBJPROG( obj, NULL, "onPulseTick" ); // call objprogram

    if ( obj == NULL || !obj->valid ) // if item is not anymore valid after objprogram
      continue;

    // if timer wears off ... goodbye
    if ( obj->timer <= 0 || --obj->timer > 0 ) // test if obj->timer didn't wear off
      continue;
    
    char *message; // item_type specific wear off message
    switch ( obj->item_type ) {
    default:              message = "$p crumbles into dust.";         break;
    case ITEM_FOUNTAIN:   message = "$p dries up.";                   break;
    case ITEM_CORPSE_NPC: message = "$p decays into dust.";           break;
    case ITEM_CORPSE_PC:  message = "$p decays into dust.";           break;
    case ITEM_FOOD:       message = "$p decomposes.";                 break;
    case ITEM_POTION:     message = "$p has evaporated from disuse."; break;
    case ITEM_PORTAL:     message = "$p fades out of existence.";     break;
    case ITEM_CONTAINER: 
      if (CAN_WEAR(obj,ITEM_WEAR_FLOAT))
	if (obj->contains)message = "$p flickers and vanishes, spilling its contents on the floor.";
	else      	  message = "$p flickers and vanishes.";
      else                message = "$p crumbles into dust.";
      break;
    case ITEM_FURNITURE: message = "$p vanishes.";                    break;
    }
    
    CHAR_DATA *rch;
    if ( obj->carried_by != NULL ){
      act( message, obj->carried_by, obj, NULL, TO_CHAR );
      if ( obj->wear_loc == WEAR_FLOAT)
	act(message,obj->carried_by,obj,NULL,TO_ROOM);
    }
    else if ( obj->in_room != NULL
	      &&      ( rch = obj->in_room->people ) != NULL ) {
      if (!(obj->in_obj && obj->in_obj->pIndexData->vnum == OBJ_VNUM_PIT
	     && !CAN_WEAR(obj->in_obj,ITEM_TAKE))){
	act( message, rch, obj, NULL, TO_ROOM );
	act( message, rch, obj, NULL, TO_CHAR );
      }
    }
    
    if ((obj->item_type == ITEM_CORPSE_PC || obj->wear_loc == WEAR_FLOAT )
	&&  obj->contains){   // save the contents
      OBJ_DATA *next_obj;
      
      for (OBJ_DATA *t_obj = obj->contains; t_obj != NULL; t_obj = next_obj){
	next_obj = t_obj->next_content;
	obj_from_obj(t_obj);
	
	if (obj->in_obj) // in another object
	  obj_to_obj(t_obj,obj->in_obj);
	else if (obj->carried_by)  // carried
	  if (obj->wear_loc == WEAR_FLOAT)
	    if (obj->carried_by->in_room == NULL)
	      extract_obj(t_obj);
	    else
	      obj_to_room(t_obj,obj->carried_by->in_room);
	  else
	    obj_to_char(t_obj,obj->carried_by);
	
	else if (obj->in_room == NULL)  // destroy it
	  extract_obj(t_obj);
	
	else // to a room
	  obj_to_room(t_obj,obj->in_room);
      }
    }
    
    extract_obj( obj ); // extract item cos' timer wears off
  }
  return;
}

/*
 * Update all rooms.
 * This function is performance sensitive.
 */

// Redone by SinaC 2003
void room_update( void ) {   
  ROOM_INDEX_DATA *room;
  AFFECT_DATA *paf, *paf_next;

  int nMatch = 0;
  for(int vnum=0;nMatch<top_room;vnum++)
    // if the room exists
    if ( ( room = get_room_index(vnum) ) != NULL ) {
      nMatch++; 
      
      if ( !room->valid )
	continue;

      // go through affects and decrement
      bool afrem = FALSE;
      for ( paf = room->current_affected; paf != NULL; paf = paf_next ) {
	paf_next    = paf->next;
	if ( !IS_SET( paf->flags, AFFECT_IS_VALID ) )
	  continue;
	if ( paf->duration == 0 ) { // wearoff
	  if ( paf->type > 0 ){ // affect created by an ability
	    if ( ability_table[paf->type].wearoff_fun != NULL  // wearoff function?
		 && ability_table[paf->type].wearoff_fun != wearoff_null )
	      (*ability_table[paf->type].wearoff_fun) ( paf, (void*)room, TARGET_ROOM );
	    else if ( ability_table[paf->type].msg_room // wearoff msg?
		      && str_cmp( ability_table[paf->type].msg_room , "none") ) {
	      CHAR_DATA *rch = room->people;
	      if ( rch != NULL )
		act(ability_table[paf->type].msg_room,rch,NULL,NULL,TO_ALL);
	    }
	    else // no function neither msg -> bug
	      bug("ability %s needs a msg_room", ability_table[paf->type].name);
	    affect_remove_room_no_recompute( room, paf ); //wearoff -> remove affect
	    afrem = TRUE;
	  }
	}
	else { // finite or infinite duration: no wearoff
	  if ( paf->type > 0 //&& IS_SET( paf->flags, AFFECT_ABILITY )
	       && ability_table[paf->type].update_fun != NULL // update function ?
	       && ability_table[paf->type].update_fun != update_null )
	    (*ability_table[paf->type].update_fun) ( paf, (void*)room, TARGET_ROOM );

	  if ( paf->duration > 0 ) { // if positive duration: decrease
	    paf->duration--;
	    if (number_range(0,4) == 0 && paf->level > 0)
	      paf->level--;  // ability strength fades with time
	  }
	}
      }
      if ( afrem ) // if some affects have been removed: recompute
	recomproom(room);
      
      ROOMPROG( room, NULL, "onPulseTick" ); // call roomprogram
    }
  return;
}

/*
 * Aggress.
 *
 * for each mortal PC
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function takes 25% to 35% of ALL Merc cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't want the mob to just attack the first PC
 *   who leads the party into the room.
 * -- Furey
 */
void aggr_update( void ) {
  int countLoop = 0;

  if ( !USE_FACTION_ON_AGGRO ) { // ONLY OLD AGGRO CODE
    // OLD Complexity: O( #char * #PC_in_room * #NPC_in_room )   -> cubic
    // The way to get a pseudo-random target is false:
    //  first valid vch: 1 chance out 1
    //  2nd              1 chance out 2
    //  3rd              1 chance out 3
    //  ...
    //  nth              1 chance out n
    // So the first persons in the room have stochastically more chance
    //  to be aggro'ed than the last ones
    CHAR_DATA *wch;
    CHAR_DATA *wch_next;
    CHAR_DATA *ch;
    CHAR_DATA *ch_next;
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    CHAR_DATA *victim;

    // For each PC
    for ( wch = char_list; wch != NULL; wch = wch_next ) {
      wch_next = wch->next;

      if ( IS_NPC(wch) // NPC
	   ||   !wch->valid
	   ||   IS_IMMORTAL(wch) // IMMORTAL
	   ||   wch->in_room == NULL  // not in a room
	   ||   wch->in_room->area->empty // no PC in the area
	   ||   IS_SET( wch->in_room->cstat(flags), ROOM_SAFE ) ) // safe room
	continue;
    
      // For each aggressive NPC in the PC's room
      for ( ch = wch->in_room->people; ch != NULL; ch = ch_next ) {
	ch_next	= ch->next_in_room;
      
	if ( !IS_NPC(ch)
	     ||   !ch->valid
	     ||   !IS_SET(ch->act, ACT_AGGRESSIVE)
	     ||   IS_AFFECTED(ch,AFF_CALM)
	     ||   ch->fighting != NULL
	     ||   IS_AFFECTED(ch, AFF_CHARM)
	     ||   !IS_AWAKE(ch)
	     ||   ( IS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch) )
	     ||   number_bits(1) == 0
	     // modified by SinaC 2000
	     // Modified by SinaC 2001
	     //	   ||   IS_SET(ch->in_room->cstat(flags),ROOM_SAFE)
	     // Added by SinaC 2001
	     ||   IS_AFFECTED( ch, AFF_ROOTED)
	     // this test ( safe one ) was previously in the inner loop
	     ||   !can_see( ch, wch ) )
	  continue;
      
	//
	// Ok we have a 'wch' player character and a 'ch' npc aggressor.
	// Now make the aggressor fight a RANDOM pc victim in the room,
	//   giving each 'vch' an equal chance of selection.  !! not really an equal chance SinaC 2003 !!
	//
	int count	= 0;
	victim	= NULL;
	//for ( vch = wch->in_room->people; vch != NULL; vch = vch_next ) {
	// ch->in_room must be used: imagine the following situation:
	// a room with a PC and 2 aggro NPC
	// the first NPC aggro PC and PC died in one hit -> only one multi_hit called -> PC is dead before
	//  the whole loop on NPC is finished -> PC is transfered to healer
	// the second NPC will search PC to aggro in PC room which is not the same as before
	//  -> NPC will try to aggro in the healer room instead of previous room
	// Loop on ch is not affected because we store ch_next before performing any action
	for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
	  vch_next = vch->next_in_room;

	  if ( !IS_NPC(vch)
	       && vch->valid
	       && !IS_IMMORTAL(vch)
	       && ch->level >= vch->level - 5 
	       && ( !IS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch) )
	       && can_see( ch, vch ) ) {
	    countLoop++;
	    if ( number_range( 0, count ) == 0 )
	      victim = vch;
	    count++;
	  }
	}
      
	if ( victim == NULL )
	  continue;
      
	act( "$n {rsuddenly{x jumps and starts to hit you.", ch, NULL, victim, TO_VICT );
	if ( AGGR_VERBOSE > 0 ) {
	  printf("[%s] has aggro'ed [%s]\n\r", NAME(ch), NAME(victim) );
	}
	multi_hit( ch, victim, TYPE_UNDEFINED );
      }
    }
  }
  else { // OLD AGGRO AND FACTION AGGRO
    // NEW Complexity: 0( #room * #NPC_in_room * #char_in_room ) -> cubic
    // SinaC 2003
    // New aggro base on faction or act's aggressive, significantly slower because we check NPC target
    // For each room
    // |  For each NPC in room
    // |  |  For each char in room
    // |  |  |  If  check_aggro_faction  OR  act AGGRESSIVE  Then
    // |  |  |  |  Store target
    // |  |  |  Endif
    // |  |  EndFor
    // |  |  Get a "true" random target from stored target and aggro it
    // |  EndFor
    // EndFor

    const int MAX_TARGET = 128;
    CHAR_DATA *tmp_target[MAX_TARGET]; // maximum 128 available target
    int tmp_target_count;

    // For each ROOM
    for ( int i = 0; i < MAX_KEY_HASH; i++ )
      for ( ROOM_INDEX_DATA *pRoom = room_index_hash[i]; pRoom != NULL; pRoom = pRoom->next ) {
      
	if ( IS_SET( pRoom->cstat(flags), ROOM_SAFE ) // Safe room
	     || pRoom->area->empty )                  // Nothing happens in area without any PC
	  continue;
      
	// For each NPC, create target vector
	CHAR_DATA *npc, *npc_next;
	for ( npc = pRoom->people; npc != NULL; npc = npc_next ) {
	  npc_next = npc->next_in_room;
	
	  if ( !IS_NPC(npc)                // avoid PC
	       || !npc->valid                    // not valid
	       || IS_AFFECTED( npc, AFF_CALM )   // calmed
	       || npc->fighting != NULL          // fighting
	       || IS_AFFECTED( npc, AFF_CHARM )  // charmed
	       || !IS_AWAKE( npc )               // sleeping
	       || IS_AFFECTED( npc, AFF_ROOTED ) // rooted
	       || number_bits(1) == 0 )          // chance
	    continue;
	
	  bool aggressive = IS_SET( npc->act, ACT_AGGRESSIVE ); // Old aggro

	  if ( !aggressive                                // if mob is non-aggressive
	       && npc->faction == get_neutral_faction() ) // and doesn't have any faction
	    continue;                                     // no need to continue

	  tmp_target_count = 0;

	  // For each Char, check if valid target
	  CHAR_DATA *target, *target_next;
	  for ( target = pRoom->people; target != NULL; target = target_next ) {
	    target_next = target->next_in_room;

	    if ( target == npc                       // Can't attack itself
		 || !target->valid                   // only valid
		 || IS_IMMORTAL( target )            // Don't attack immortal
		 || ( aggressive && IS_NPC(target) ) // With old aggro, only PC
		 || !can_see( npc, target )          // Don't attack target we can't see
		 || silent_is_safe( npc, target ) )  // Check if target is safe
	      continue;
	  
	    countLoop++;

	    if ( ( aggressive                                  // npc  ACT_AGGRESSIVE
		   && ( !IS_SET( npc->act, ACT_WIMPY ) || !IS_AWAKE( target ) )
		   && npc->level >= target->level - 5 )
		 || check_aggro_faction( npc, target, TRUE ) ) // npc  can aggro this target on faction
	      if ( tmp_target_count < MAX_TARGET ) // target count not reached
		tmp_target[tmp_target_count++] = target;
	      else
		break; // stop when maximum target is reached
	  }

	  // Get a random target and aggro it
	  if ( tmp_target_count == 0 ) // no target found
	    continue;
	  int choose = number_range( 0, tmp_target_count-1 );
	  CHAR_DATA *victim = tmp_target[ choose ];
	  act( "$n {rsuddenly{x jumps and starts to hit you.", npc, NULL, victim, TO_VICT );
	  if ( AGGR_VERBOSE > 0 ) {
	    printf("[%s] has aggro'ed [%s]\n\r", NAME(npc), NAME(victim) );
	  }
	  multi_hit( npc, victim, TYPE_UNDEFINED );
	}
      }
  }
#ifdef VERBOSE
  printf("==>%d\n\r", countLoop );
#endif

  return;
}

/* 
 * Seytan 1997 ROM2.4 AUC
 * The new auction update system 
 *
 */
void auction_update( void ) {
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  
  if(aucstat){
    switch(aucstat){
    case 1: /* first phase..  do nothing */
      aucstat=2;
      break;
      
    case 2: /* going once */
      
      sprintf(buf,"{mA bet of %d on %s{m going ONCE{x\r\n",lastbet,
	      aucobj->short_descr);
      
      for ( d = descriptor_list; d != NULL; d = d->next ){
	CHAR_DATA *victim;
	
	victim = d->original ? d->original : d->character;
	
	if ( d->connected == CON_PLAYING &&
	     !IS_SET(victim->comm,COMM_NOAUCTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) ){
	  send_to_char(buf,victim);
	}
      }
      aucstat=3;
      break;
      
    case 3: /* going twice */
      
      sprintf(buf,"{mA bet of %d on %s{m going TWICE{x\r\n",lastbet,
	      aucobj->short_descr);
      
      for ( d = descriptor_list; d != NULL; d = d->next ){
	CHAR_DATA *victim;
	
	victim = d->original ? d->original : d->character;
	
	if ( d->connected == CON_PLAYING &&
	     !IS_SET(victim->comm,COMM_NOAUCTION) &&
	     !IS_SET(victim->comm,COMM_QUIET) ){
	  send_to_char(buf,victim);
	}
      }
      aucstat=4;
      break;
      
    case 4: /* sold / no bet received */
      if(lastbetter!=NULL){
	sprintf(buf,"{m%s{m SOLD to %s{m for %d{x\r\n",aucobj->short_descr,
		lastbetter->name,lastbet);
	obj_to_char(aucobj,lastbetter); /* Give it to him */
	
	/* By oxtal : gold and silver are not changed by auctioning*/
	aucfrom->silver+= (betsilver*80/100); /* I keep 20% */
	aucfrom->gold += (betgold*80/100);
      }
      else{
	sprintf(buf,"{mNo bet received on %s{m. Object retained by Auction Inc.{x\r\n",
		aucobj->short_descr);
	extract_obj(aucobj);  /* Kill object */
      }
      
      for ( d = descriptor_list; d != NULL; d = d->next ){
	CHAR_DATA *victim;
	
	      victim = d->original ? d->original : d->character;
	      
	      if ( d->connected == CON_PLAYING &&
		   !IS_SET(victim->comm,COMM_NOAUCTION) &&
		   !IS_SET(victim->comm,COMM_QUIET) ){
		send_to_char(buf,victim);
	      }
      }
      aucstat =0;
      lastbet =0;
      lastbetter=NULL;
      aucfrom=NULL;
      aucobj=NULL;
      break;
    } /* switch */
    
  } /* if */
}

void burning_update( void ) {
  CHAR_DATA *ch, *ch_next;
  bool okay;

  for ( ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;
    okay = valid_update_target( ch, FALSE );
    // we passed every test and are in a burning room
    // Modified by SinaC 2001
    if ( okay && ch->in_room->cstat(sector) == SECT_BURNING  ) {
      log_stringf("%s is burning in %d",
		  NAME(ch),ch->in_room->vnum );

      int done = noaggr_damage( ch, ch->level*2, DAM_FIRE, 
				 "You are {Rburning.{x", 
				 "$n is {Rburned.{x",
				 "has been consummed by fire.",
				 FALSE );
      // Added by SinaC 2001
      if ( done == DAMAGE_DONE )
	fire_effect( (void*) ch, ch->level, ch->level*2, TARGET_CHAR );
    }
  }
}

// In prevision for underwater rooms
void underwater_update( void ) {
  CHAR_DATA *ch, *ch_next;
  bool okay;

  for ( ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;

    okay = valid_update_target( ch, FALSE );

    // we passed every test and are in an underwater room
    if ( okay 
	 // Modified by SinaC 2001, modified by SinaC 2003
	 //&& ch->in_room->cstat(sector) == SECT_WATER_NOSWIM 
	 && ch->in_room->cstat(sector) == SECT_UNDERWATER 
	 && !IS_AFFECTED2( ch, AFF2_WATER_BREATH ) 
	 // Added by SinaC 2001
	 && !IS_SET( ch->cstat(form), FORM_UNDEAD ) 
	 && !IS_SET( ch->cstat(form), FORM_INTANGIBLE ) 
	 && !IS_SET( ch->cstat(form), FORM_MIST ) ) {
      log_stringf("%s is drowning in %d",
		  NAME(ch),ch->in_room->vnum );
      noaggr_damage( ch, ch->level*5, DAM_DROWNING, 
		     "You are {Bdrowning.{x", 
		     "$n is {Bdrowning.{x",
		     "is dead due to drowning.",
		     FALSE );
    }
  }
}

/* Removed by SinaC 2003, can be emulate with script
// Added by SinaC 2000 for grenade
void grenade_update( void ) {
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  CHAR_DATA *vch, *vch_next;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *begin_list;
  bool carry;
  
  for ( obj = object_list; obj != NULL; obj = obj_next ){
    obj_next = obj->next;
    
    if ( obj->item_type != ITEM_GRENADE )
      continue;
    
    if ( obj->value[1] == GRENADE_PULLED ){
      if ( obj->in_room == NULL ){
	begin_list = obj->carried_by->in_room->people;
	carry = TRUE;
      }
      else{
	begin_list = obj->in_room->people;
	carry = FALSE;
      }
      
      if ( obj->value[0] > 0 ){
	for (vch = begin_list; vch != NULL; vch = vch->next_in_room){
	  sprintf( buf, "{Y%s{Y ticks quietly here.{x\n\r", 
		   capitalize(obj->short_descr));
	  send_to_char(buf,vch);
	}
	obj->value[0]--;
	return;
      }
      for (vch = begin_list; vch != NULL; vch = vch_next){
	vch_next = vch->next_in_room;
	
	sprintf( buf, "{Y%s is consumed in flames as it {REXPLODES!{x\n\r",capitalize(obj->short_descr));
	send_to_char(buf,vch);
	// Modified by SinaC 2000
	//
	//sprintf( buf, "{rYou are struck by the shrapnel!{x\n\r");
	//send_to_char(buf,vch);
	//damage(vch,vch,obj->value[2],0,DAM_NEGATIVE,FALSE);
	//
	if ( carry == TRUE && obj->carried_by == vch )
	  noaggr_damage( vch, obj->value[2]*3, DAM_NEGATIVE,
			 "{rYou are struck by the shrapnel!{x",
			 NULL,
			 "is dead because of a grenade.",
			 FALSE );
	else
	  noaggr_damage( vch, obj->value[2], DAM_NEGATIVE,
			 "{rYou are struck by the shrapnel!{x",
			 NULL,
			 "is dead because of a grenade.",
			 FALSE );
      }
      extract_obj( obj );
    }
  }
  return;
}
*/

void lightning_update( void ) {
  DESCRIPTOR_DATA *d, *d_nxt;

  for ( d = descriptor_list; d != NULL; d = d_nxt ){
    d_nxt = d->next;
    
    if ( d->connected == CON_PLAYING
	 && IS_OUTSIDE( d->character )
	 && IS_AWAKE  ( d->character )
	   && chance(1)
	 && !IS_IMMORTAL(d->character)
	 && d->character->level > 17
	 && weather_info.sky == SKY_LIGHTNING ) {
      if ( check_immune(d->character,DAM_LIGHTNING) != IS_IMMUNE ){
	if ( d->character->fighting ){
	  stop_fighting(d->character,TRUE); 
	}
	noaggr_damage( d->character, d->character->hit/4+100, DAM_LIGHTNING,
		       "{RYou see a brillant flash come down from the sky and then black out!{x",
		       "$n has been struck by lightning!",
		       "has been killed by a lightning coming down from the sky",
		       FALSE );
      }
    } 
  }
}

/* Removed by SinaC 2003, scripts can do that
void teleport_update ( void ) {
  CHAR_DATA 	*ch;
  CHAR_DATA	*ch_next;
  ROOM_INDEX_DATA *pRoomIndex;
  bool okay;

  for (ch = char_list ; ch != NULL; ch = ch_next ){
    ch_next = ch->next;
    okay = valid_update_target( ch, FALSE );
    // Modified by SinaC 2001
    if ( okay && IS_SET(ch->in_room->cstat(flags), ROOM_TELEPORT ) ){
      //do_look ( ch, "tele" );
      if ( ch->in_room->area->teleport_room == 0 )
	pRoomIndex = get_random_room (ch);
      else
	pRoomIndex = get_room_index(ch->in_room->area->teleport_room);
      
      send_to_char ("You have been teleported!!!\n\r", ch);
      act("$n vanishes!!!", ch, NULL, NULL, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, pRoomIndex);
      act("$n slowly fades into existence.", ch, NULL, NULL,
	  TO_ROOM);
      do_look(ch, "auto");
    }
  }
}
*/

void quake_update( void ) {       
  CHAR_DATA 	*ch;
  CHAR_DATA	*ch_next;
  char buf[MAX_STRING_LENGTH];
  bool okay;
  
  // for every player, check if there is an earthquake in the are he's in
  for (ch = char_list ; ch != NULL; ch = ch_next ){
    ch_next = ch->next;

    okay = valid_update_target( ch, TRUE );

    // if valid target and earthquake in the area
    if ( okay && ch->in_room->area->earthquake_on == TRUE ){

      // no earthquake inside
      // Modified by SinaC 2001
      if ( ch->in_room->cstat(sector) == SECT_INSIDE ){
	send_to_char("{mFrom outside, faint rumbles of thunder can be heard.{x\n\r",ch);
	continue;
      }

      // no earthquake in underwater or in air area
      // Modified by SinaC 2001
      if ( ch->in_room->cstat(sector) == SECT_AIR
	   || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
	   || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
	   // Added by SinaC 2003
	   || ch->in_room->cstat(sector) == SECT_UNDERWATER )
	continue;
	  
      // earthquake still active
      if ( ch->in_room->area->earthquake_duration > 0 ){
	
	sprintf(buf,"{yViolent tremors split the land, inflicting wounds and injuries!{x\n\r");
	switch ( ch->position ){
	default:
	  strcat(buf,"{YYou fall and are stricken painfully!{x");
	  // only PC will sit unless I add a stand at the end of an earthquake
	  if ( IS_PC(ch) && !IS_IMMORTAL(ch)) 
	    ch->position = POS_SITTING;
	  break;
	case POS_STANDING: 
	  strcat(buf,"{YYou fall and are stricken painfully!{x");
	  // only PC will sit unless I add a stand at the end of an earthquake
	  if ( IS_PC(ch) && !IS_IMMORTAL(ch)) 
	    ch->position = POS_SITTING;
	  break;
	case POS_SLEEPING:
	  strcat(buf,"{YAn awful tremor awakes you violently!{x");
	  do_stand(ch,"");
	  break;
	case POS_SITTING:
	  strcat(buf,"{YAlready being on the ground, you are thrown about more!{x");
	  break; 
	case POS_FIGHTING:
	  stop_fighting( ch, TRUE );
	  strcat(buf,"{YThe tremors bewilder and halt your fighting, mid-combat.{x");
	  break;
	}
	
	if ( ch->level > 30 )
	  noaggr_damage( ch,50,DAM_BASH,
			 buf,NULL,
			 "$n is dead due to earthquake.",
			 FALSE);
	else
	  noaggr_damage( ch,5,DAM_BASH,
			 buf,NULL,
			 "$n is dead due to earthquake.",
			 FALSE);
      }
      // earthquake stops
      else
	send_to_char("{yThe ground beneath you stabilizes as the earthquake passes.{x\n\r",ch);
    }
  } 
  // now we have to update earthquake data in area_data
  AREA_DATA *pArea;
  
  // for every area
  for ( pArea = area_first; pArea != NULL; pArea = pArea->next ){
    // if there is an earthquake
    if ( pArea->earthquake_on == TRUE ){
      // if the duration is not over
      if ( pArea->earthquake_duration > 0 )
	// decrement the duration
	pArea->earthquake_duration--;
      // if the duration is 0
      else {
	// stop the earthquake
	pArea->earthquake_on = FALSE;
	sprintf(log_buf,"Earthquake has stopped in %s.",pArea->name);
	log_string(log_buf);
      }
    }
  }
}

void check_earthquake() {
  AREA_DATA *pArea;

  for ( pArea = area_first; pArea != NULL; pArea = pArea->next ){
    if ( IS_SET( pArea->area_flags, AREA_EARTHQUAKE ) ){
      if ( number_range( 0, 250 ) == 0
	   && pArea->earthquake_on == FALSE ){
	sprintf(log_buf,"Automatic earthquake has been detected in %s.",pArea->name);
	log_string(log_buf);
	
	pArea->earthquake_on = TRUE; 
	pArea->earthquake_duration = number_range( 5, 25 );      
      }
    }
  }
}


// Added by SinaC 2001
void daylight_update() {
  CHAR_DATA *ch, *ch_next;

  for ( ch = char_list; ch != NULL; ch = ch_next ) {
    ch_next = ch->next;

    if ( !valid_update_target( ch, FALSE ) )
      continue;

    ROOM_INDEX_DATA *pRoomIndex = ch->in_room;

    bool sun_shining;
    if ( pRoomIndex->cstat(sector) == SECT_INSIDE
	 || IS_SET(pRoomIndex->cstat(flags), ROOM_INDOORS) )
      sun_shining = FALSE;
    else
      if ( weather_info.sunlight == SUN_SET
	   || weather_info.sunlight == SUN_DARK )
	sun_shining = FALSE;
      else
	sun_shining = TRUE;
    if ( IS_SET( ch->cstat(vuln_flags), IRV_DAYLIGHT )
	 && sun_shining ) {
      log_stringf("%s is burned by the sun in %d",
		  NAME(ch),ch->in_room->vnum );
      int dam = 5;
      if ( ch->level > 10 )
	dam += ch->level*2;
      if ( ch->level > 30 )
	dam += ch->level*2;
      if ( ch->level > 70 )
	dam += ch->level*2;

      noaggr_damage( ch, dam, DAM_DAYLIGHT, 
		     "You are {Yburned by the sun.{x", 
		     "$n is {Yburned by the sun.{x",
		     "is dead due to sun.",
		     FALSE );
    }
  }
}

void start_double_xp() {
  // Warns everyone that double xp has started
  double_xp = TRUE;
  info("{R ==DOUBLE XP HAS STARTED=={x\n\r");
  log_stringf("==DOUBLE XP HAS STARTED==");
}

void end_double_xp() {
  // Warns everyone that double xp is over
  double_xp = FALSE;
  info("{R  ==DOUBLE XP IS OVER=={x\n\r");
  log_stringf("==DOUBLE XP IS OVER==");
}

//Added by SinaC 2003 to monitor the mud with MRTG
// we dump informations to a file and a perl script will extract those informations
// to send them to MRTG
void dump_monitoring_informations() {
  FILE *fp;
  if ( ( fp = fopen(MONITORING_FILE,"w") ) == NULL ) {
    bug("Cannot open %s to dump monitoring informations.", MONITORING_FILE );
    return;
  }
  fprintf( fp, "#UpTime %s\n", str_boot_time );

  int playing = 0, creating = 0, connecting = 0, noting = 0, other = 0;
  int mortal = 0;

  for ( DESCRIPTOR_DATA *d = descriptor_list; d; d = d->next ) {
    if ( d->character ) {
      switch( d->connected ) {
      case CON_PLAYING: playing++; break;
      case CON_GET_NAME:
      case CON_GET_OLD_PASSWORD:
      case CON_CONFIRM_NEW_NAME:
      case CON_GET_NEW_PASSWORD:
      case CON_CONFIRM_NEW_PASSWORD:
      case CON_GET_NEW_RACE:
      case CON_GET_NEW_SEX:
      case CON_GET_NEW_CLASS:
      case CON_GET_ALIGNMENT:
      case CON_DEFAULT_CHOICE:
      case CON_GEN_GROUPS:
      case CON_ANSI:
      case CON_GET_HOMETOWN:
      case CON_PICK_WEAPON: creating++; break;
      case CON_READ_MOTD:
      case CON_READ_IMOTD:
      case CON_BREAK_CONNECT:
      case CON_COPYOVER_RECOVER: connecting++; break;
      case CON_NOTE_TO:
      case CON_NOTE_SUBJECT:
      case CON_NOTE_EXPIRE:
      case CON_NOTE_TEXT:
      case CON_NOTE_FINISH: noting++; break;
      case CON_GET_GOD:
      case CON_REBIRTH:
      case CON_REMORT:
      default: other++; break;
      }
      if ( !IS_IMMORTAL(d->character) )
	mortal++;
    }
  }
  long total = playing+creating+connecting+noting+other;
  fprintf( fp, "#Total %ld\n"
	   "#Playing %d\n"
	   "#Creating %d\n"
	   "#Connecting %d\n"
	   "#Noting %d\n"
	   "#Other %d\n"
	   "#Mortal %d\n"
	   "#Immortal %ld\n",
	   total,
	   playing, creating, connecting, noting, other,
	   mortal,
	   total-mortal );
  
  fprintf( fp, "#Mobile %d\n"
	   "#Object %d\n",
	   mobile_count, object_count );

  total = 0;
  total += top_affect * sizeof( AFFECT_DATA );
  total += top_area * sizeof( AREA_DATA );
  total += top_ed * sizeof( EXTRA_DESCR_DATA );
  total += top_exit * sizeof( EXIT_DATA );
  total += top_help * sizeof( HELP_DATA );
  total += social_count * sizeof(struct social_type );
  total += MAX_ABILITY * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  total += top_mob_index * sizeof( MOB_INDEX_DATA );
  total += mobile_count * sizeof( CHAR_DATA );
  total += top_obj_index * sizeof( OBJ_INDEX_DATA );
  total += object_count * sizeof( OBJ_DATA );
  total += top_reset * sizeof( RESET_DATA );
  total += top_room * sizeof( ROOM_INDEX_DATA );
  total += top_shop * sizeof( SHOP_DATA );
  total += MAX_COMMANDS * sizeof( cmd_type );
  total += MAX_CLASS * sizeof(class_type);
  total += MAX_GROUP * (sizeof(group_type)+MAX_CLASS*sizeof(int)+MAX_GODS*sizeof(int));
  total += MAX_GODS * sizeof(god_data); // wrong approx
  total += MAX_RACE * sizeof(race_type) + MAX_PC_RACE * sizeof(pc_race_type); // wrong approx

  fprintf( fp, "#Mem %ld\n", total );

  fprintf( fp, "#Heap %d\n", GC_get_heap_size());
  fprintf( fp, "#Free %d\n", GC_get_free_bytes());
  fprintf( fp, "#Allocated %d\n", GC_get_bytes_since_gc());
  
  fprintf( fp, "#StringUsed %d\n", get_str_space_size() );
  fprintf( fp, "#StringEntries %d\n", get_str_free_entries());

  fclose(fp);
}

void area_freq_update( ) {
  AREA_DATA * a;

  for (a=area_first; a != NULL; a = a->next)
    a->totalnplayer += a->nplayer;
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */

int     pulse_area = 0;
int     pulse_mobile = 0;
int     pulse_violence = 0;
int     pulse_point = 0;
int     pulse_auction = 0;
int     pulse_hints = 0;
int     pulse_start_arena = PULSE_ARENA;
int     pulse_arena = PULSE_ARENA;
int     pulse_room = 0;
int     pulse_quake = 0;
int     pulse_double_xp = 3*PULSE_HOUR; // 3 hours after boot, double xp starts
int     pulse_double_xp_duration = 0;
void update_handler( void ) {
  if ( pulse_double_xp_duration > 0 ) {
    if ( --pulse_double_xp_duration <= 0 )
      end_double_xp();
  }
  if ( --pulse_double_xp <= 0 ) {
    pulse_double_xp = 9*PULSE_HOUR; // every 9 hours
    pulse_double_xp_duration = 30*60*PULSE_PER_SECOND; // lasts 30 minutes
    start_double_xp();
  }


  // Added by SinaC 2000 for area earthquake, every 5 seconds
  if ( --pulse_quake   <= 0 ){
    pulse_quake 	= PULSE_EARTHQUAKE;
    quake_update	();
  }

  /* Removed by SinaC 2003, scripts can do that
  // Added by SinaC 2000 for teleporting room, every 20 seconds
  if ( --pulse_teleport <= 0 ){
    pulse_teleport	= (20 * PULSE_PER_SECOND);
    teleport_update();
  }
  */
  
  // Added by SinaC 2000 for burning and underwater area, every 10 seconds
  // WILL BE REMOVED, replaced with scripts
  if ( --pulse_room <= 0 ){
    pulse_room = PULSE_PER_SECOND * 10;
    
    burning_update();
    underwater_update();
  }

  /* Removed by SinaC 2003, can be emulate with script    
  // Added by SinaC 2000 for grenade, every seconds
  if ( --pulse_grenade <= 0 ){
    pulse_grenade     = PULSE_PER_SECOND * 1;
    grenade_update();
  }
  */

  // Every 30 seconds
  if ( --pulse_hints <= 0 ){
    pulse_hints = PULSE_HINTS;
    hints_update();
  }

  // Every 12 seconds
  if ( --pulse_auction  <= 0 ){
    pulse_auction   = PULSE_MOBILE * 3;
    auction_update();

    // Added by SinaC 2001
    daylight_update();
  }
   
  // Every minute (60 seconds)
  //  if ( --pulse_quest <= 0 ){
  //    pulse_quest = PULSE_TICK;
  //    quest_update();
  //  }

  // Added by SinaC 2000 for arena, every 30 seconds
  if(in_start_arena || ppl_challenged)
    if( --pulse_start_arena <= 0){
      pulse_start_arena = PULSE_ARENA;
      start_arena();
    }
  if(ppl_in_arena)
    if(( --pulse_arena <= 0) || (num_in_arena()==1)){
      pulse_arena = PULSE_ARENA;
      do_game();
    }

  // Every 2 minutes (120 seconds)
  if ( --pulse_area     <= 0 ){
    pulse_area	= PULSE_AREA;
    area_update	();
    
    // Every 2 minutes, we check if an earthquake starts to shake us
    check_earthquake();
  }

  /* Removed by SinaC 2003
  // Every 6 seconds
  if ( --pulse_music	  <= 0 ){
    pulse_music	= PULSE_MUSIC;
    song_update();
  }
  */

  // Every 4 seconds
  if ( --pulse_mobile   <= 0 ){
    pulse_mobile	= PULSE_MOBILE;
    mobile_update	( ); // -->onPulseMobile
  }
  
  // Every 3 seconds
  if ( --pulse_violence <= 0 ){
    pulse_violence	= PULSE_VIOLENCE;
    violence_update	( );
  }

  // Every minute (60 seconds)
  if ( --pulse_point    <= 0 ){
    char buf[20];
    sprintf(buf,"TICK! %d",save_number);
    wiznet(buf,NULL,NULL,WIZ_TICKS,0,0);

    pulse_point     = PULSE_TICK; /* number_range( PULSE_TICK / 2, 3 * PULSE_TICK / 2 ); */

    generate_html_who(); // generate who

    auto_copyover_update(); // do an autocopyover ?

    weather_update	( );
    // Added by SinaC 2000
    lightning_update( );
    
    char_update	( ); // -->onPulseTick
    obj_update	( ); // -->onPulseTick
    room_update ( ); // -->onPulseTick
    
    area_freq_update( );

    dump_monitoring_informations(); // SinaC 2003
  }

  // Every second
  aggr_update( );
  return;
}
