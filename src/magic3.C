#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
//#include "olc.h"

// Added by SinaC 2001
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "fight.h"
#include "act_info.h"
#include "act_enter.h"
#include "act_comm.h"
#include "magic.h"
#include "effects.h"
#include "gsn.h"
#include "olc_value.h"
#include "lookup.h"
#include "moons.h"
#include "spells_def.h"
#include "interp.h"
#include "update.h"
#include "utils.h"
#include "damage.h"
#include "weather.h"


// Extra spell by Oxtal
void spell_bird_form( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  /* Just for the fun, example of what can be done */

  AFFECT_DATA af;
  CHAR_DATA * victim = (CHAR_DATA *) vo;
  OBJ_DATA * obj;

  int i,bird = race_lookup("song bird");

  if (IS_PC(victim)) {
    act("Doing that on $M? No, $N is a player!",ch,NULL,victim,TO_CHAR);
    return;
  }

  if ( victim->cstat(race) == bird ) {
    act("But $N is already a bird!",ch,NULL,victim,TO_CHAR);
    return;
  }

  while ( victim->carrying != NULL ) {
    obj = victim->carrying;

    /* Remove the obj if it is worn */
    if (obj->wear_loc != WEAR_NONE)
      unequip_char( victim, obj );

    obj_from_char( obj );
    obj_to_room( obj, victim->in_room );
    OBJPROG(obj,victim,"onDropped",victim); // Added by SinaC 2003
  }

//  afsetup(af,CHAR,race,ASSIGN,bird,level,level,sn);
//  affect_to_char(victim,&af);
//  af.location = ATTR_parts;
//  af.modifier = race_table[bird].parts;
//  affect_to_char(victim,&af);
//  af.location = ATTR_form;
//  af.modifier = race_table[bird].form;
//  affect_to_char(victim,&af);
//  af.location = ATTR_DICE_TYPE;
//  af.op = AFOP_ADD;
//  af.modifier = 0-victim->cstat(DICE_TYPE) *4/5;
//  affect_to_char(victim,&af);
//  for (i = ATTR_STR; i<=ATTR_CON; i++) {
//    af.location = i;
//    af.modifier = 0-victim->curattr[i] *2/3;
//    affect_to_char(victim,&af);
//  }
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,race,ASSIGN,bird);
  addaff(af,CHAR,parts,ASSIGN,race_table[bird].parts);
  addaff(af,CHAR,form,ASSIGN,race_table[bird].form);
  addaff(af,CHAR,DICE_TYPE,ADD,0-victim->cstat(DICE_TYPE) *4/5);
  for ( i = ATTR_STR; i <= ATTR_CON; i++)
    addaff2(af,AFTO_CHAR,i,AFOP_ADD,-victim->curattr[i] *2/3);
  affect_to_char( victim, &af );

  act("You changed $N into a song bird.",ch,NULL,victim,TO_CHAR);
  act("$n changed $N into a song bird!",ch,NULL,victim,TO_NOTVICT);
  act("$n has changed you to a song bird!",ch,NULL,victim,TO_VICT);
}

// Added by SinaC 2000

/* A simple beacon spell, written by Quzah (quzah@geocities.com) Enjoy. */
void spell_beacon( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int  a;
  ROOM_INDEX_DATA *pRoomIndex = NULL;
  
  if( ch->in_room == NULL ) 
    return;
  
  // Modified by SinaC 2001
  const char *tname;
  //target_name = one_argument( target_name, arg );
  tname = one_argument( target_name, arg );
  
  /* List beacons. */
  if( arg[0] == '\0' ) {
    send_to_char( " ## | Location name\n\r" , ch );
    send_to_char( "----+---------------\n\r" , ch );
    
    for( a = 0; a < MAX_BEACONS; a++ ) {
      pRoomIndex = get_room_index( ch->pcdata->beacons[a] );
      
      sprintf( buf, " %2d | %s\n\r", a,
	       ( pRoomIndex == NULL ? "(unused)" : pRoomIndex->name ) );
      send_to_char( buf, ch );
      pRoomIndex = NULL;
    }
    return;
  }
  
  /* Set a beacon. */
  if( !str_cmp( "set", arg ) ) {
    // Modified by SinaC 2001
    //target_name = one_argument( target_name, arg );
    tname = one_argument( tname, arg );

    if( arg[0] == '\0' ) {
      send_to_char("Set a beacon like this: 'cast beacon set #'\n\r", ch );
      return;
    }
    a = atoi(arg);
    if ( a < 0 || a >= MAX_BEACONS ) {
      send_to_char("Beacon number out of range.\n\r", ch );
      return;
    }
    else {
      // Modified by SinaC 2001
      if( IS_SET( ch->in_room->cstat(flags), ROOM_NO_RECALL ) ) {
	send_to_char("You can't memorize this location.\n\r", ch);
	return;
      }
      ch->pcdata->beacons[a] = ch->in_room->vnum;
      send_to_char("You memorize your current location.\n\r", ch );
      return;
    }
  }
  
  /* Go to a beacon location. */
  if( !str_cmp( "recall", arg ) ) {
    // Modified by SinaC 2001
    //target_name = one_argument( target_name, arg );
    tname = one_argument( tname, arg );
    if( arg[0] == '\0' ) {
      send_to_char("To transfer to a beacon: 'cast beacon recall #'\n\r",ch);
      return;
    }
    a = atoi( arg );
    if( a < 0 || a >= MAX_BEACONS ) {
      send_to_char("Beacon number out of range.\n\r", ch );
      return;
    }
    else {
      pRoomIndex = get_room_index( ch->pcdata->beacons[a] );

      // Modified by SinaC 2001      
      if( IS_SET( ch->in_room->cstat(flags), ROOM_NO_RECALL ) ) {
	send_to_char("The room glows for a moment, then fades.\n\r", ch);
	return;
      }
      if( pRoomIndex == NULL ) {
	send_to_char("You remember no such location.\n\r", ch );
	ch->pcdata->beacons[a] = 0;
	return;
      }
      act("$n disapears in a quick flash of light.",ch,NULL,NULL,TO_ROOM);
      char_from_room( ch );
      char_to_room( ch, pRoomIndex );
      act("$n appears in a quick burst of light.",ch,NULL,NULL,TO_ROOM);
      do_look( ch, "auto" );
      return;
    }
  }
  return;
}

/*
Hello folks,

The list has been a wee bit quiet lately so I thought I would post a
contribution. Following is the code for a "fumble" spell we added to our own
MUD. Hopefully I've managed to pull out all the stuff that makes it
non-generic. Share and enjoy :-)

Andy South
Elwyn of Dragon Realms (realms.envy.com 4444)
*/

// Modified by SinaC 2000
void spell_fumble( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  OBJ_DATA   *obj;
  OBJ_DATA   *obj_drop;
  int         carry;
  int         check;
  int         count;
  int         drop;
  bool        fumbled = FALSE;
  
  if ( !IS_AWAKE( victim ) )
    return;
  
  carry = 0;
  for ( obj = victim->carrying; obj; obj = obj->next_content )
    carry++;
  
  drop = carry - can_carry_n( victim ) + 5;
  
  for ( check = 0; check < drop; check++ ){
    obj_drop = NULL;
    count = 0;
    
    for ( obj = victim->carrying; obj; obj = obj->next_content ){
      if ( obj->wear_loc == WEAR_NONE
	   && number_range( 0, count++ ) == 0 )
	obj_drop = obj;
    }
    
    if ( !obj_drop )
      break;
    
    //fumbled = fumble_obj( victim, obj_drop, level, TRUE ) || fumbled;
    fumbled |= fumble_obj( ch, victim, obj_drop, level, TRUE );
  }

  if ( ( obj_drop = get_eq_char( victim, WEAR_HOLD ) ) )
    //fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;
    fumbled |= fumble_obj( ch, victim, obj_drop, level, FALSE );
  
  if ( ( obj_drop = get_eq_char( victim, WEAR_LIGHT ) ) )
    //fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;
    fumbled |= fumble_obj( ch, victim, obj_drop, level, FALSE );
  
  if ( ( obj_drop = get_eq_char( victim, WEAR_WIELD ) ) )
    //fumbled = fumble_obj( victim, obj_drop, level, FALSE ) || fumbled;
    fumbled |= fumble_obj( ch, victim, obj_drop, level, FALSE );
  
  if ( !fumbled ){
    send_to_char( "You stumble momentarily, but quickly recover.\n\r",
		  victim );
    act( "$n stumbles momentarily, but quickly recovers.",
	 victim, NULL, NULL, TO_ROOM );
  }

  // Added by SinaC 2001
  recompute(victim);
  recomproom(victim->in_room);
 
  return;
}

/*
  MAGIC SPELLS from the Mage's Lair MUD.  (c) The Mage 1998 (c)
  Issue Version 1.00

  The file is released under the GNU license. Feel free to use it.  Give
  me credit if you want.

  The headers for each spell should be added to your codebase if you use
  these spells or the ideas from them.

  The Mage IMP of The Mage's Lair (lordmage.erols.com port 4000)

  NOTE: This assumes you can put the stuff in magic.h and const.c
  It also uses Lopes Colour code.


  A note on magic_dam().  This function is used to centralize all the
  damage done via the spells.  It uses the DICE command so you could
  switch out all your other spells to use this function and then you
  would have 1 central place for all the spells in your system to be
  balanced and weighted against each other.
                                              The Mage.


*/

/*
 * Current Spells included:
 * Banshee Scream. 	v 1.0
 * Ionwave. 		v 1.0
 * Vaccine.		v 1.0
 * Sunbeam.		v 1.0
 */

void spell_ionwave( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;
  
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next ) {
    pChar_next = pChar->next_in_room;
    if ( !is_safe_spell( ch, pChar, TRUE ) && (pChar != ch) && !is_same_group(ch,pChar)) {
      act( "$n sends a huge wave of energy out! The energy burns you{x!", ch, NULL, pChar, TO_VICT);
      dam = dice(9,10);
      if ( saves_spell( level, pChar, DAM_ENERGY ) )	
	dam /= 2;
      ability_damage( ch, pChar, dam, sn, DAM_ENERGY,TRUE,TRUE);
    }
  }
  return;
}
/*
  SPELL: Vaccine
  DESC: Adds resistance to DISEASE.
  EFFECT: Target.
  AUTHOR: The Mage.
*/
void spell_vaccine( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_DISEASE)
       || IS_SET(victim->cstat(imm_flags),IRV_DISEASE)) {
    if (victim == ch)
      send_to_char("You are already vaccinated.\n\r",ch);
    else
      act("$N is already vaccinated.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  //afsetup(af, CHAR, res_flags, OR, IRV_DISEASE, level, level, sn );
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_DISEASE);
  affect_to_char( victim, &af );
  
  send_to_char( "{BYou feel someone vaccinating you.{x\n\r", victim );
  if ( ch != victim )
    act("{B$N{x is vaccinated by your magic.",ch,NULL,victim,TO_CHAR);
  return;
  
}
/*
  SPELL: Banshee Scream
  DESC: Sends a blast of SOUND out in all directions from the caster.
  EFFECT: SOUND damage to everyone not the CASTER.
  AUTHOR: The Mage.
*/
void spell_banshee_scream( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  /* This spell will when used.. wipe out your mana */
  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next ) {
    pChar_next = pChar->next_in_room;
    if ( pChar == ch )
      continue;
    if ( !is_safe_spell( ch, pChar, TRUE ) ) {
      act( "{c$n screams a horrible sound! Your ears pop{x!", ch, NULL, pChar, TO_VICT );
      dam = dice(UMAX(level/5,7),UMAX(level/2,7));
      if ( saves_spell( level, pChar, DAM_SOUND ) )	
	dam /= 2;
      ability_damage( ch, pChar, dam, sn, DAM_SOUND,TRUE,TRUE);
    } 
    else
      act( "{c$n screams a horrible sound!{x", ch, NULL, pChar, TO_VICT );
  }
  return;
}

/*
 * SinaC 2000
 */
void spell_locate_person( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *room;
  char room_name[MAX_STRING_LENGTH];
  int chance;

  if ( ( victim = get_char_world( ch, target_name ) ) == NULL || 
       !can_see( ch, victim) ) {
    send_to_char("You failed.\n\r", ch );
    return;
  }

  chance = number_percent();

  if ( IS_SET( victim->cstat(imm_flags),IRV_SUMMON ) ) chance = 0;
  if ( IS_SET( victim->cstat(res_flags),IRV_SUMMON  ) ) chance -= 50;
  if ( !can_see_room( ch, victim->in_room ) ) chance = 0;
  if ( IS_SET( victim->cstat(vuln_flags),IRV_SUMMON ) ) chance = 100;
  
  chance = UMAX( 0, chance );

  if ( chance == 0 ) {
    send_to_char("You failed.\n\r", ch );
    return;
  }
  
  // 70% finding the target
  if ( chance >= 30 )
    room = victim->in_room; 
  // 30% not finding the target
  else
    room = get_random_room( ch );

  sprintf(room_name,"%s",room==NULL?"somewhere":room->name);
  send_to_charf(ch,"%s is in %s.\n\r",PERS(victim, ch ),room_name);
}

void spell_silence( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
    
  /*
  if ( victim == ch ) {
    send_to_char("You're really silly.\n\r", ch );
    return;
  }
  */

  if ( is_affected( victim, sn ) // Added by SinaC 2003
       || saves_spell( level, victim, DAM_OTHER ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  //afsetup( af, CHAR, affected_by, OR, AFF_SILENCE, 2, level, sn ); 
  //affect_to_char( victim, &af );
  //afsetup( af, CHAR, affected2_by, OR, AFF2_NOSPELL, 2, level, sn ); 
  //affect_to_char( victim, &af );
  createaff(af,2,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SILENCE);
  addaff(af,CHAR,affected2_by,OR,AFF2_NOSPELL);
  affect_to_char( victim, &af );

  act("{W$N is silenced!{x", ch, NULL, victim, TO_CHAR );
  send_to_char("{WYou are silenced!{x\n\r", victim );
  act("{W$N is silenced!{x", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_acid_rain( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int        dam,i;

  for ( i = 0; i < 8; i++ ) {
    dam = dice( level, 8 );
    if ( saves_spell( level, victim, DAM_ACID ) )
      dam /= 2;
    if ( victim->in_room == ch->in_room ) {
      act("{RWaves of {GA{gc{Gi{gd{Gi{gc {BRain {gcalled by $n shower down upon $N!{x",
	  ch,NULL,victim,TO_NOTVICT);
      act("{RWaves of {GA{gc{Gi{gd{Gi{gc {BRain {gshower down upon you!{x",
	  ch,NULL,victim,TO_VICT); 
      act("{RWaves of {GA{gc{Gi{gd{Gi{gc {BRain {gshower down upon $N!{x",
	  ch,NULL,victim,TO_CHAR); 
      ability_damage( ch, victim, dam, sn, DAM_ACID,TRUE, TRUE ); 
    }
  }
  return;
}

void spell_layhands( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  
  victim->hit = UMIN( victim->hit + 250, victim->cstat(max_hit) );
  update_pos( victim );
  send_to_char( "Your God heals your wounds.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Your God heals their wounds.\n\r", ch );
  return;
}

/* RT clerical berserking spell */
void spell_jades_lust(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_affected(victim,sn) 
      || IS_AFFECTED(victim,AFF_BERSERK)) {
    if (victim == ch)
      send_to_char("You are already freaky lustful.\n\r",ch);
    else
      act("$N is already in a freaky lustful.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if (is_affected(victim,gsn_calm)) {
    if (victim == ch)
      send_to_char("Why don't you just relax for a while?\n\r",ch);
    else
      act("$N doesn't look like $e wants to fight anymore.",
	  ch,NULL,victim,TO_CHAR);
    return;
  }
  
  //afsetup(af,CHAR,hitroll,ADD,level/9,level/5,level,sn);
  //affect_to_char(victim,&af);
  //afsetup(af,CHAR,damroll,ADD,level/9,level/5,level,sn);
  //affect_to_char(victim,&af);
  //afsetup(af,CHAR,allAC,ADD,10*(level/15),level/5,level,sn);
  //affect_to_char(victim,&af);
  createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,level/9);
  addaff(af,CHAR,damroll,ADD,level/9);
  addaff(af,CHAR,allAC,ADD,10*(level/15));
  affect_to_char( victim, &af );

  send_to_char("You're overtaken with lust and must fight to release it!\n\r",
	       victim);
  act("$n gets a lustful look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

void spell_summon_lgolem( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i=0;
  int nb_max;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if (is_affected(ch,sn)) {
    send_to_char("You lack the power to summon another golem right now.\n\r",
		 ch);
    return;
  }
  
  send_to_char("You attempt to summon a lesser golem.\n\r",ch);
  act("$n attempts to summon a lesser golem.",ch,NULL,NULL,TO_ROOM);

  // 4 max.
  nb_max = 1 + (level>46) + (level>71) + (level>90);
  for (gch = char_list; gch != NULL; gch = gch->next) {
    if (IS_NPC(gch) 
	&& IS_AFFECTED(gch,AFF_CHARM) 
	&& gch->master == ch 
	&& ( gch->pIndexData->vnum == MOB_VNUM_LGOLEM ) ) {
      i++;
      if (i >= nb_max) {
	send_to_char("More golems are more than you can control!\n\r",ch);
	return;
      }
    }
  }

  golem = create_mobile( get_mob_index(MOB_VNUM_LGOLEM) );
  
  for (i = 0; i < MAX_STATS; i ++)
    golem->bstat(stat0+i) = UMIN(25,15 + level/10);
            
  golem->bstat(STR) += 3;
  golem->bstat(INT) -= 1;
  golem->bstat(CON) += 2;

  golem->bstat(max_hit) = IS_NPC(ch)? URANGE(ch->cstat(max_hit),1 * ch->cstat(max_hit),30000) : UMIN( (2 * ch->hit) + 400,30000);
  golem->hit = golem->bstat(max_hit);
  golem->bstat(max_mana) = IS_NPC(ch)? ch->cstat(max_mana) : ch->mana;
  golem->mana = golem->bstat(max_mana);
  // Added by SinaC 2001 for mental user
  golem->bstat(max_psp) = IS_NPC(ch)? ch->cstat(max_psp) : ch->psp;
  golem->psp = golem->bstat(max_psp);

  golem->level = ch->level;
  for (i=0; i < 4; i++)
    golem->bstat(ac0+i) = interpolate(golem->level,100,-100);
  golem->gold = 0;
  golem->timer = 0;
  golem->bstat(DICE_NUMBER) = 3;   
  golem->bstat(DICE_TYPE) = 10;
  golem->bstat(DICE_BONUS) = level / 2;

  
  //afsetup( af, CHAR, NA, ADD, 0, 15, level, sn ); 
  //affect_to_char(ch, &af);
  createaff(af,15,level,sn,0,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( ch, &af );

  SET_BIT(golem->bstat(affected_by), AFF_CHARM);
  SET_BIT(golem->act, ACT_CREATED ); // SinaC 2003
  
  char_to_room(golem,ch->in_room);
  send_to_char("You summoned a lesser golem!\n\r",ch);
  act("$n summons a lesser golem!",ch,NULL,NULL,TO_ROOM);

  add_follower( golem, ch );
  golem->leader = ch;

  //  recompute( golem ); NO NEED: done in char_to_room
}

void spell_summon_ggolem( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) 
{
  CHAR_DATA *gch;
  CHAR_DATA *golem;
  AFFECT_DATA af;
  int i = 0;
  int nb_max;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if (is_affected(ch,sn)) {
    send_to_char("You lack the power to summon another golem right now.\n\r", 
		 ch);
    return;
  }

  send_to_char("You attempt to summon a greater golem.\n\r",ch);
  act("$n attempts to summon a greater golem.",ch,NULL,NULL,TO_ROOM);

  // 2 max.
  nb_max = 1 + (level>90);
  for (gch = char_list; gch != NULL; gch = gch->next) {
    if (IS_NPC(gch) 
	&& IS_AFFECTED(gch,AFF_CHARM) 
	&& gch->master == ch 
	&& ( gch->pIndexData->vnum == MOB_VNUM_GGOLEM ) ) {
      i++;
      if (i >= nb_max ) {
	send_to_char("More golems are more than you can control!\n\r",ch);
	return;
      }
    }
  }
  
  golem = create_mobile( get_mob_index(MOB_VNUM_GGOLEM) );

  for (i = 0; i < MAX_STATS; i ++)
    golem->bstat(stat0+i) = UMIN(25,15 + level/10);
            
  golem->bstat(STR) += 3;
  golem->bstat(INT) -= 1;
  golem->bstat(CON) += 2;

  golem->bstat(max_hit) = IS_NPC(ch)? URANGE(ch->cstat(max_hit),1 * ch->cstat(max_hit),30000) : UMIN( (10 * ch->hit) + 4000, 30000);
  golem->hit = golem->bstat(max_hit);
  golem->bstat(max_mana) = IS_NPC(ch)? ch->cstat(max_mana) : ch->mana;
  golem->mana = golem->bstat(max_mana);
  // Added by SinaC 2001 for mental user
  golem->bstat(max_psp) = IS_NPC(ch)? ch->cstat(max_psp) : ch->psp;
  golem->psp = golem->bstat(max_psp);
  golem->level = ch->level;
  for (i=0; i < 4; i++)
    golem->bstat(ac0+i) = interpolate(golem->level,100,-100);

  golem->gold = 0;
  golem->timer = 0;
  golem->bstat(DICE_NUMBER) = 13;   
  golem->bstat(DICE_TYPE) = 9;
  golem->bstat(DICE_BONUS) = level / 2 + 10;

  //afsetup( af, CHAR, NA, ADD, 0, 20, level, sn ); 
  //affect_to_char(ch, &af);  
  createaff(af,20,level,sn,0,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( ch, &af );

  SET_BIT(golem->bstat(affected_by), AFF_CHARM);
  SET_BIT(golem->act, ACT_CREATED ); // SinaC 2003

  char_to_room(golem,ch->in_room);
  send_to_char("You summoned a greater golem!\n\r",ch);
  act("$n summons a greater golem!",ch,NULL,NULL,TO_ROOM);

  add_follower( golem, ch );
  golem->leader = ch;

  // recompute( golem ); NO NEED: done in char_to_room
}

void spell_soul_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  OBJ_DATA *obj;
  OBJ_DATA *sblade;
  int number,type;
  
  obj = get_obj_here( ch, target_name );
  
  if ( obj == NULL ) {
    send_to_char( "Cast Soul Blade on What?\n\r", ch );
    return;
  }
  
  /* Nothing but NPC corpses. */
  
  if( obj->item_type != ITEM_CORPSE_NPC ) {
    if( obj->item_type == ITEM_CORPSE_PC )
      send_to_char( "The player wishes to keep their soul.\n\r", ch );
    else
      send_to_char( "It would serve no purpose...\n\r", ch );
    return;
  }
  
  if( obj->level > (level + 2) 
      && !IS_IMMORTAL(ch) ) {
    send_to_char( "You cannot forge such a soul into a blade.\n\r", ch );
    return;
  }
  
  /* Create the soulblade */
  sblade = create_object ( get_obj_index (OBJ_VNUM_SOULBLADE), 0);
  
  sblade->level                  = UMIN( obj->level, 100 );

  //number = UMAX(sblade->level/4 + 1, 2);
  //type   = UMAX(sblade->level/5,3);
  number = UMAX(sblade->level/5, 3);
  type   = number_fuzzy(UMAX(sblade->level/10,2));

  sblade->baseval[1] = number;
  sblade->baseval[2] = type;
  
  //sblade->timer = UMAX((sblade->level +10)/2,15);
  sblade->timer = UMAX(sblade->level+10,15);

  AFFECT_DATA af;
  //int roll = number_fuzzy(UMAX(level/5,3));
  int roll = number_fuzzy(UMAX(level/20,3));
  //afsetup(af,CHAR,damroll,ADD,roll,sblade->timer,level,sn);
  //affect_to_obj(sblade,&af);
  //afsetup(af,CHAR,hitroll,ADD,roll,sblade->timer,level,sn);
  //affect_to_obj(sblade,&af);
  //afsetup(af,CHAR,max_hit,ADD,100+number_fuzzy(level),sblade->timer,level,sn);
  //affect_to_obj(sblade,&af);
  createaff(af,sblade->timer,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,damroll,ADD,roll);
  addaff(af,CHAR,hitroll,ADD,roll);
  addaff(af,CHAR,max_hit,ADD,100+number_fuzzy(level));
  affect_to_obj( sblade, &af );

  //recompobj(sblade); DONE in affect_to_obj

  /* Action! */
  obj_to_char(sblade,ch);
  act( "$n waves dramatically and $p appears.", ch, sblade, NULL, TO_ROOM );
  act( "You wave dramatically and $p appears.", ch, sblade, NULL, TO_CHAR );
  
  extract_obj(obj);
  
  return;
}

void spell_mind_meld ( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam;

  if ( is_affected( victim, sn ) 
       || saves_spell( level, victim, DAM_MENTAL ) ) {
    send_to_char( "Your efforts fail to produce melding.\n\r", ch );
    return;
  }

  dam = dice( 6, level );
  int done = ability_damage( ch, victim, dam, sn, DAM_MENTAL, TRUE, TRUE );
  if ( done == DAMAGE_DONE ) {
    //afsetup( af, CHAR, INT, ADD, -5, 2+level, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,2+level,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,INT,ADD,-5);
    affect_to_char( victim, &af );
  }

  act( "$N has been mind melded.", ch, NULL, victim, TO_CHAR    );
  send_to_char( "You feel an immense pain in your head!\n\r", victim );
  act( "$N grimaces in pain!", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_psi_twister( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  send_to_char( "You release your mental powers to do its will in the room!\n\r", 
		ch );
  act( "$n levitates amid a swirl of psychic energy.", ch, NULL, NULL, TO_ROOM );

  for ( vch = ch->in_room->people; vch; vch = vch_next ) {
    vch_next = vch->next_in_room;
    
    if ( vch != ch && !is_safe_spell(ch,vch,TRUE)){
      if (saves_spell(level,vch, DAM_MENTAL))
	ability_damage( ch, vch, 1, sn, DAM_MENTAL, TRUE, TRUE );
      else
	ability_damage( ch, vch, level + dice(3, 8), sn, DAM_MENTAL, TRUE, TRUE );
    }
  }
  return;
}

void spell_regeneration( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
	
  if (IS_AFFECTED(victim, AFF_REGENERATION)) {
    if ( ch == victim )
      send_to_char("You are already regenerating.\n\r", ch );
    else
      send_to_char("That target is already regenerating.\n\r",ch);
    return;
  }
  
  //afsetup(af,CHAR,affected_by,OR,AFF_REGENERATION,24,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,24,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_REGENERATION);
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself healing your wounds more quickly.\n\r", 
		victim );
  act("$n is healing much more quickly.",victim,NULL,NULL,TO_ROOM);
  return;
}

/* elfren Exorcism, good version of Demonfire */
void spell_exorcism(int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_NPC(ch) && !IS_GOOD(ch) ) {
    victim = ch;
    send_to_char("The Angels turn to attack you!\n\r",ch);
  }

  // Modified by SinaC 2001 etho/alignment are attributes now 
  // Modified by SinaC 2000 for alignment/etho 
  //ch->align.alignment = UMAX(1000, ch->align.alignment + 50);
  ch->bstat(alignment) = UMAX(1000, ch->bstat(alignment) + 50);

  if (victim != ch) {
    act("$n invokes the wrath of his God on $N!",
	ch,NULL,victim,TO_ROOM);
    act("$n has called forth the Angels of heaven to exorcise you!",
	ch,NULL,victim,TO_VICT);
    send_to_char("You call forth Angels of God!\n\r",ch);
  }
  dam = dice( level, 10 );
  if ( saves_spell( level, victim, DAM_HOLY ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_HOLY, TRUE, TRUE );
}

/*
 * Some cool spells from Tartarus, Added by SinaC 2000
 */

/*
  Heal spell, but also for only slightly more cost can cure poison and disease.
  Can stop wasting, but no heal benefit is gained if used this way. Won't
  restore undead_drains or wither etc.
*/
void spell_utter_heal(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if (IS_EVIL(ch)) {
    send_to_char("You're too evil to have such beneficial spells.\n\r",ch);
    return;
  }
  
  if ( is_affected(victim, gsn_poison) 
       && check_dispel(level,victim,gsn_poison) ) {
    send_to_char("A warm feeling runs through your body.\n\r",victim);
    act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
  }
  
  if (is_affected(ch,gsn_plague) 
      && check_dispel(level,victim,gsn_plague)) {
    act("The sores on $n's body disappear.",victim,0,0,TO_ROOM);
    send_to_char("The sores on your body disappear.\n\r",victim);
  }
  
  send_to_char("You feel better!\n\r",victim);
  victim->hit = UMIN(victim->hit + 100, victim->cstat(max_hit));
  if (victim != ch)
    send_to_char("Ok.\n\r",ch);
  
  return;
}

void spell_fire_and_ice(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, sn_frost, sn_fire, d_type, sn_type;
  
  sn_frost = ability_lookup("frost breath");
  sn_fire = gsn_fire_breath;

  if (sn_fire == -1 || sn_frost == -1) {
    send_to_char("The elements fail to combine.\n\r",ch);
    return;
  }
  
  if (number_percent() > 50) {
    sn_type = sn_frost;
    d_type = DAM_COLD;
  }
  else {
    sn_type = sn_fire;
    d_type = DAM_FIRE;
  }
  dam = dice(level,8);
  act("$n unleashes a blast of fire and ice!",ch,0,0,TO_ROOM);
  send_to_char("You unleash a blast of fire and ice!\n\r",ch);
  
  if (saves_spell(level,victim,d_type))
    dam /= 2;
  ability_damage(ch,victim,dam,sn_type,d_type,TRUE,TRUE);
  
  dam = dice(level, 6);
  if (d_type == DAM_COLD) {
    d_type = DAM_FIRE;
    sn_type = sn_fire;
  }
  else {
    d_type = DAM_COLD;
    sn_type = sn_frost;
  }
  ability_damage(ch,victim,dam,sn_type,d_type,TRUE,TRUE);
  return;
}

void spell_shock_sphere(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int dam;
  
  act("A sphere of crackling energy detonates around $n with explosive sound!", victim,0,0,TO_ROOM);
  send_to_char("A sphere of fierce energy detonates around you with deafening sound!\n\r",victim);

  if (is_affected(victim,sn)) {
    dam = dice(level, 3);
    ability_damage(ch,victim,saves_spell(level,victim,DAM_SOUND) ? dam/2 : dam,sn,DAM_SOUND,TRUE,TRUE);
    return;
  }
  
  if (saves_spell(level,victim,DAM_SOUND)) {
    dam = dice(level,5);
    ability_damage(ch,victim,saves_spell(level,victim,DAM_SOUND) ? dam/2 : dam,sn,DAM_SOUND,TRUE,TRUE);
    return;
  }
  
  dam = dice(level,7);
  int done = ability_damage(ch,victim,saves_spell(level,victim,DAM_SOUND) ? dam/2 : dam,sn,DAM_SOUND,TRUE,TRUE);

  if ( done == DAMAGE_DONE ) {
    act("$n appears deafened.",victim,0,0,TO_ROOM);
    send_to_char("You can't hear a thing!\n\r",victim);
    
    //afsetup( af, CHAR, hitroll, ADD, -2, level/8, level, sn );
    //affect_to_char(victim,&af);
    createaff(af,level/8,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,hitroll,ADD,-2);
    affect_to_char( victim, &af );
  }
  return;
}

void spell_cremate(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  dam = dice(level,7);
  
  act("$n is engulfed in raging fire!",victim,0,0,TO_ROOM);
  send_to_char("You are engulfed in raging fire!\n\r",victim);
  
  if ( saves_spell( level, victim, DAM_FIRE ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_FIRE, TRUE, TRUE);
  return;
}

void spell_flesh_golem(int sn,int level, CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *golem;
  AFFECT_DATA af;
  CHAR_DATA *check;
  OBJ_DATA *part;
  OBJ_DATA *part_next;
  int parts, lvl, i, nb_max;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }
  
  parts = 0;
  if (is_affected(ch,sn)) {
    send_to_char("You aren't up to fashioning another flesh golem yet.\n\r",ch);
    return;
  }

  // 3 max.
  nb_max = 1 + (level>57) + (level>90);
  i = 0;
  for (check = char_list; check != NULL; check = check->next) {
    if ( IS_NPC(check) 
	 && IS_AFFECTED(check,AFF_CHARM) 
	 && check->master == ch
	 && ( check->pIndexData->vnum == MOB_VNUM_FLESHGOLEM ) ) {
      i++;
      if (i >= nb_max) {
	send_to_char("More golems are more than you can control!\n\r",ch);
	return;
      }
    }
  }  
  
  for (part = ch->carrying; part != NULL; part = part_next) {
    part_next = part->next_content;
    if (part->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD
	&& part->pIndexData->vnum != OBJ_VNUM_TORN_HEART
	&& part->pIndexData->vnum != OBJ_VNUM_SLICED_ARM
	&& part->pIndexData->vnum != OBJ_VNUM_SLICED_LEG
	&& part->pIndexData->vnum != OBJ_VNUM_GUTS
	&& part->pIndexData->vnum != OBJ_VNUM_BRAINS)
      continue;
    parts++;
  }
  
  if (parts == 0) {
    send_to_char("You don't have any body parts to animate.\n\r",ch);
    return;
  }
  if (parts <= 1) {
    send_to_char("You don't have enough parts to build a flesh golem.\n\r",ch);
    return;
  }
  
  for ( part = ch->carrying; part != NULL; part = part_next ) {
    part_next = part->next_content;
    if (part->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD
	&& part->pIndexData->vnum != OBJ_VNUM_TORN_HEART
	&& part->pIndexData->vnum != OBJ_VNUM_SLICED_ARM
	&& part->pIndexData->vnum != OBJ_VNUM_SLICED_LEG
	&& part->pIndexData->vnum != OBJ_VNUM_GUTS
	&& part->pIndexData->vnum != OBJ_VNUM_BRAINS)
      continue;
    extract_obj(part);
  }
  if (parts >= level/5)
    parts = level/5;
  
  lvl = UMAX((level - 3 + parts),1);

  if ( ch->level < 70 ) {
    //afsetup( af, CHAR, NA, ADD, 0, 15, level, sn ); 
    //affect_to_char(ch,&af);
    createaff(af,15,level,sn,0,AFFECT_ABILITY);
    //addaff(af,CHAR,NA,ADD,0);
    affect_to_char( ch, &af );
  }
  
  act("$n fashions a flesh golem!",ch,0,0,TO_ROOM);
  send_to_char("You fashion a flesh golem to serve you!\n\r",ch);
  
  golem = create_mobile(get_mob_index(MOB_VNUM_FLESHGOLEM) );
  golem->level = lvl;
  for (i = 0; i < MAX_STATS; i++)
    golem->bstat(stat0+i) = UMIN(25,15 + level/10);
  
  golem->bstat(STR) += 3;
  golem->bstat(INT) -= 1;
  golem->bstat(CON) += 2;
  
  for (i=0; i < 4; i++)
    golem->bstat(ac0+i) = interpolate(golem->level,100,-100);

  golem->bstat(DICE_NUMBER) = parts;
  golem->bstat(DICE_TYPE) = parts * 3;
  golem->bstat(DICE_BONUS) = level / 2;

  golem->cstat(damroll) = level/2 + parts;

  if (parts <= 3) golem->bstat(max_hit) = ((lvl*lvl/2) + dice(lvl,10));
  else if (parts == 4) golem->bstat(max_hit) = ((lvl*lvl/2) + dice(lvl,12));
  else if (parts == 5) golem->bstat(max_hit) = ((lvl*lvl/2) + dice(lvl,13));
  else if (parts == 6) golem->bstat(max_hit) = ((lvl*lvl/2) + dice(lvl,15));
  else if (parts == 7) golem->bstat(max_hit) = ((lvl*lvl/2) + dice(lvl,18));
  else golem->bstat(max_hit) = ((lvl*lvl/2) + dice(lvl,20));

  golem->hit = golem->bstat(max_hit);
  golem->bstat(max_move) = ch->bstat(max_move);
  golem->move = golem->bstat(max_move);

  SET_BIT(golem->bstat(affected_by), AFF_CHARM);
  SET_BIT(golem->act, ACT_CREATED ); // SinaC 2003

  char_to_room(golem,ch->in_room);
  
  add_follower(golem, ch);
  golem->leader = ch;

  //  recompute(golem); NO NEED: done in char_to_room

  return;
}

void spell_sunbolt(int sn,int level, CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  act("$n prays to the Gods and a beam of sunlight erupts from $s hands!",
      ch,0,0,TO_ROOM);
  send_to_char("You pray to the Gods and a beam of sunlight erupts from your hands!\n\r",ch);

  if (IS_GOOD(victim)) {
    act("$n is unaffected by the blast of light.",victim,0,0,TO_ROOM);
    act("You are unaffected by the blast of light.",victim,0,0,TO_CHAR);
    return;
  }
  dam = dice(level,7);

  if (IS_EVIL(victim))
    dam += dice(level,3);

  if (IS_NPC(victim) 
      && ( IS_SET(victim->act,ACT_UNDEAD) 
	   || IS_SET(victim->cstat(form), FORM_UNDEAD) )
      && number_percent() > 60) {
    act("The positive energy begins to vaporise $n!",victim,0,0,TO_ROOM);
    act("The positive energy begins to vaporise you!",victim,0,0,TO_VICT);
    dam += dice(level,7);
  }

  if ( !(saves_spell(level,victim,DAM_LIGHT)) ) {
    // Modified by SinaC 2001
    spell_blindness(gsn_blindness, level, ch, (void *) victim, TARGET_CHAR, 1);
  }
  if (saves_spell(level,victim,DAM_LIGHT))
    dam /= 2;

  ability_damage(ch,victim,dam,sn,DAM_LIGHT,TRUE,TRUE);

  return;
}

void spell_frostbolt(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (victim == ch) {
    act("$n is blasted with a bolt of ice!",ch,0,0,TO_ROOM);
    send_to_char("You are blasted by a bolt of ice!\n\r",ch);
  }
  else {
    act("$n points at $N and a bolt of ice flies forth!",ch,0,victim,TO_NOTVICT);
    act("$n points at you and a bolt of ice flies forth!",ch,0,victim,TO_VICT);
    act("You point at $N and a bolt of ice flies forth!",ch,0,victim,TO_CHAR);
  }
  dam = dice(level,10);
  if (saves_spell(level,victim,DAM_COLD))
    dam /= 2;
  ability_damage(ch,victim,dam,sn,DAM_COLD,TRUE,TRUE);
  return;
}

void spell_icelance(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (victim == ch) {
    act("$n throws out a shard of sharp ice!",ch,0,0,TO_ROOM);
    send_to_char("You are struck by your shard of ice!\n\r",ch);
  }
  else {
    act("$n throws forth a shard of sharp ice at $N!",ch,0,victim,TO_NOTVICT);
    act("$n throws forth a shard of sharp ice at you !",ch,0,victim,TO_VICT);
    act("You point at $N and throw forth a shard of ice!",ch,0,victim,TO_CHAR);
  }
  dam = dice(level,6);
  if (saves_spell(level,victim,DAM_COLD))
    dam /= 2;
  ability_damage(ch,victim,dam,sn,DAM_COLD,TRUE,TRUE);
  return;
}

void spell_lightshield( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if (!IS_GOOD(victim)) {
    if (victim == ch)
      send_to_char("You are not pure enough to receive the gift of light.\n\r",ch);
    else
      act("$N is not pure enough to receive the gift of light.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already protected.\n\r",ch);
    else
      act("$N is already protected.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,saving_throw,ADD,0-level/10,level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,allAC,ADD,-level,level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_INFRARED,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,saving_throw,ADD,0-level/10);
  addaff(af,CHAR,allAC,ADD,-level);
  addaff(af,CHAR,affected_by,OR,AFF_INFRARED);
  affect_to_char( victim, &af );

  send_to_char( "You are surrounded by a glowing afflatus of purity.\n\r", 
		victim );
  if ( ch != victim )
    act("$N is surrounded by a glowing afflatus of purity.",
	ch,NULL,victim,TO_CHAR);
  return;
}

void spell_lifebane(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  
  int sn_poison, sn_curse, sn_weaken;
  int dam;
  
  sn_poison = gsn_poison;
  sn_curse = gsn_curse;
  sn_weaken = gsn_weaken;

  dam = dice(level,4);
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( ch != vch && !is_safe_spell(ch,vch,TRUE) && !is_same_group( ch, vch ) ){
      
    // Modified by SinaC 2001
      spell_poison(sn_poison,level - 5,ch,vch,target, 1);
      spell_weaken(sn_weaken,level - 5,ch,vch,target, 1);
      spell_curse(sn_curse,level - 8, ch,vch,target, 1);
      
      if (saves_spell(level,vch,DAM_NEGATIVE))
	ability_damage(ch,vch,dam/2,sn,DAM_NEGATIVE,TRUE,TRUE);
      else
	ability_damage(ch,vch,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
    }
  }
  return;
}

void spell_old_deathspell(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int klevel, dam;
  klevel = level - 7;
  
  act("$n utters a word of power and the negative energy explodes in the room!",ch,0,0,TO_ROOM);
  send_to_char("You utter a word of power and negative energy explodes into the room!\n\r",ch);

  dam = dice(klevel,4);
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next=  vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
      
    if ( IS_SET(vch->act,ACT_UNDEAD) 
	 || IS_SET( vch->cstat(form), FORM_UNDEAD )
	 || IS_SET(vch->cstat(imm_flags),IRV_NEGATIVE)) {
      act("$n is unaffected by the negative energy field.",vch,0,0,TO_ROOM);
      act("You are unaffected by the negative energy field.",vch,0,0,TO_CHAR);
      continue;
    }

    if (saves_spell(klevel, vch, DAM_NEGATIVE)
	|| vch->level > klevel) {
      if (saves_spell(level, vch, DAM_NEGATIVE)
	  || vch->level > klevel)
	ability_damage(ch,vch,dam/2,sn,DAM_NEGATIVE,TRUE,TRUE);
      else
	ability_damage(ch,vch,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
    }
    else {
      act("$n gets a horrible look in $s eye's then keels over dead!",vch,0,0,TO_ROOM);
      send_to_char("You feel an intense pain in your head as the energy ruptures your skull!\n\r",vch);
      sudden_death( ch, vch );
    }
  }
  return;
}

void spell_darkshield(int sn,int level,CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if (is_affected(victim,sn)){
    if (victim == ch)
      send_to_char("You are already surrounded by a dark shield.\n\r",ch);
    else
      send_to_char("They are already surrounded by a dark shield.\n\r",ch);
    return;
  }
  
  if (IS_GOOD(victim)) {
    if (victim == ch)
      send_to_char("You are too pure for such a spell.\n\r",ch);
    else
      act("$N is too pure for such a spell.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,allAC,ADD,(IS_EVIL(victim) ? -level : -(level/2)),(IS_EVIL(victim) ? level : level/2),level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,saving_throw,ADD,(IS_EVIL(victim) ? -level/10: -2),level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,(IS_EVIL(victim) ? level : level/2),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,saving_throw,ADD,(IS_EVIL(victim) ? -level : -(level/2)));
  addaff(af,CHAR,allAC,ADD,(IS_EVIL(victim) ? -level/10 : -2));
  affect_to_char( victim, &af );
  
  act("$n is surrounded by a darkened shield.",victim,0,0,TO_ROOM);
  send_to_char("You are surrounded by a darkened shield.\n\r",victim);
  
  return;
}

void spell_blade_barrier(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, spins;
  int chance, i;

  chance = 100;

  dam = dice(level, 3);
  spins = number_range(2,UMAX(level/20,3));
  /*
    dam *= 9;
    dam /= 10;
  */

  if (number_percent() > chance 
      || victim == ch) {
    /*
      act("$n creates a whirlwind of spinning blades which turn and strike $m down!",ch,0,0,TO_ROOM);
      act("Your blade barrier turns and strikes you down!",ch,0,0,TO_CHAR);
    */
    dam /= 2;
    noaggr_damage( ch, dam, DAM_SLASH,
		   "Your blade barrier turns and strikes you down!",
		   "$n creates a whirlwind of spinning blades which turn and strike $m down!",
		   "is dead due to spinning blades!",
		   TRUE );
    for ( i = 0; i < spins; i++ ) {
      if (saves_spell(level,ch,DAM_SLASH))
	dam /= 2;

      dam *= 3;
      dam /= 4;
      /*
	act("The blades spin and slice away at $n.",ch,0,0,TO_ROOM);
	act("The blades spin and slice away at you.",ch,0,0,TO_CHAR);
      */ 
      noaggr_damage( ch, dam, DAM_SLASH,
		     "The blades spin and slice away at you.",
		     "The blades spin and slice away at $n.",
		     "is dead due to spinning blades!",
		     TRUE );
    }
  }
  else {
    act("You create a whirlwind of spinning blades to strike down $N!",ch,0,victim,TO_CHAR);
    act("$n creates a deadly blade barrier that tears into $N!",ch,0,victim,TO_NOTVICT);
    act("$n creates a deadly blade barrier that tears into you!",ch,0,victim,TO_VICT);
    ability_damage(ch,victim,dam,sn,DAM_SLASH,TRUE,TRUE);

    for (i = 0; i < spins; i++) {
      if (victim->in_room != ch->in_room) return;

      if (saves_spell(level,victim,DAM_SLASH))
	dam /= 2;

      dam *= 3;
      dam /= 4;
      act("The blades spin and slice away at $n.",victim,0,0,TO_ROOM);
      act("The blades spin and slice away at you.",victim,0,0,TO_CHAR);

      ability_damage(ch,victim,dam,sn,DAM_SLASH,TRUE,TRUE);
    }
  }

  act("The blade barrier fades away.",ch,0,0,TO_ROOM);
  act("Your blade barrier fades away.",ch,0,0,TO_CHAR);
  return;
}

void spell_holy_fire(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  int chance, dam_mod;

  chance = 100;
  dam_mod = 10;

  if (IS_GOOD(ch) && IS_GOOD(victim)) {
    dam_mod = 5;
    chance = 0;
  }
  else if (IS_EVIL(ch)) {
    dam_mod = 7;
    chance = 0;
  }
  else if (IS_GOOD(ch) && IS_NEUTRAL(victim))
    dam_mod = 8;
  else if (IS_NEUTRAL(ch)) {
    dam_mod = 6;
    chance = 70;
  }

  dam = dice(level, 7);
  if (number_percent() > chance) {
    /*
      act("$n's heavenly fire turns on $m!",ch,0,0,TO_ROOM);
      act("Your heavenly fire turns on you for your sins!",ch,0,0,TO_CHAR);
    */
    dam *= dam_mod;
    dam /= 10;
    if (saves_spell(level,ch,DAM_HOLY))
      dam /= 2;
    //      ability_damage(ch,ch,dam,sn,DAM_HOLY,TRUE,TRUE);
    noaggr_damage( ch, dam, DAM_HOLY,
		   "Your heavenly fire turns on you for your sins!",
		   "$n's heavenly fire turns on $m!",
		   "has been killed by heavenly fire!",
		   TRUE );
    return;
  }
  act("$n calls down fire from the heavens!",ch,0,0,TO_ROOM);
  act("You call down fire from the heavens!",ch,0,0,TO_CHAR);
  dam *= dam_mod;
  dam /= 10;
  ability_damage(ch,victim,dam,sn,DAM_HOLY,TRUE,TRUE);
  return;
}


void spell_revolt(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo; 
  CHAR_DATA *charmie; 
  CHAR_DATA *charmie_next; 
  char buf[MAX_STRING_LENGTH]; 
  
  for (charmie = ch->in_room->people; charmie != NULL; charmie = charmie_next) {
    charmie_next = charmie->next_in_room; 
    if (!IS_AFFECTED(charmie,AFF_CHARM) 
	|| charmie->leader != victim
	|| saves_spell(level,victim,DAM_OTHER) )
      continue; 

    act("$n suddenly looks very angry.",charmie,0,0,TO_ROOM); 
    act("You suddenly feel incited by $n's words and turn on your master!",charmie,0,ch,TO_CHAR); 
    sprintf(buf,"I refuse to follow a tyrannt like you, %s!",victim->name); 
    do_gtell(charmie,buf); 

    REMOVE_BIT(charmie->bstat(affected_by),AFF_CHARM);
    REMOVE_BIT(charmie->cstat(affected_by),AFF_CHARM);

    //do_follow(charmie,"self"); 
    stop_follower( charmie );
    //add_follower( charmie, charmie );

    sprintf(buf,"Help! %s is revolting!", 
	    IS_NPC(charmie) ? charmie->short_descr : charmie->name); 
    do_yell(victim, buf); 

    //charmie->fighting = victim;
    multi_hit(charmie, victim, TYPE_UNDEFINED); 
  }
  
  return;
}

void spell_firestream(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo; 
  OBJ_DATA *obj_lose; 
  OBJ_DATA *obj_next; 
  int dam;
  int dice_sz;

  dice_sz = 4;
  if (level < 40)       dice_sz = 4; 
  else if (level < 50)  dice_sz = 5; 
  else if (level < 60)  dice_sz = 5; 
  else if (level < 80)  dice_sz = 6; 
  else dice_sz = 6; 

  if (number_percent() < level) {
    for (obj_lose = victim->carrying; obj_lose != NULL; obj_lose = obj_next) {
      char *msg; 
      obj_next = obj_lose->next_content; 
      if (number_bits(2) != 0
	  || IS_SET(obj_lose->extra_flags, ITEM_NOCOND))
	continue; 
      switch(obj_lose->item_type) {
      default: continue; 
      case ITEM_SCROLL: 
	// Added by SinaC 2003
      case ITEM_TEMPLATE: msg = "$p bursts into flames and turns to ash."; break; 
      case ITEM_POTION: msg = "$p bubbles and vaporises."; break; 
      }
      act(msg,victim,obj_lose,NULL,TO_CHAR); 
      extract_obj(obj_lose); 
    }
  }
  dam = dice(level/2, dice_sz - 1) + dice(level/2, dice_sz); 

  if (saves_spell(level,victim,DAM_FIRE))
    dam /= 2; 
  act("$n clenches a fist and releases a stream of searing flames!",
      ch,0,0,TO_ROOM); 
  act("You gesture at $N and release a stream of searing fire!",
      ch,0,victim,TO_CHAR); 
  ability_damage(ch,victim,dam,sn,DAM_FIRE,TRUE,TRUE); 
  return; 
}

void spell_wither(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, chance;
  AFFECT_DATA af;

  // Modified by SinaC 2000, was ch->pcdata->learned[sn] before
  chance = get_ability( ch, sn ) + (level - victim->level)*3;
  dam = dice(level,5);
  if (is_affected(victim,sn))
    chance = 0;
  chance = URANGE(5,chance,90);

  if ((number_percent() > chance)
      || saves_spell(level,victim,DAM_NEGATIVE) ) {
    send_to_char("You feel an intense pain in your body.\n\r",victim);
    act("$n jerks in sudden pain.",victim,0,0,TO_ROOM);
    if (saves_spell(level,victim,DAM_HARM) )
      dam /= 2;
    ability_damage(ch,victim,dam,sn,DAM_HARM,TRUE,TRUE);
    return;
  }

  switch(number_range(0,9)) {
  case (0):
  case (1):
  case (2):
  case (3): /* arms */
    send_to_char("You feel a sudden intense pain as your arms wither!\n\r",victim);
    act("$n screams in agony as $s arms seem to shrivel up!",victim,0,0,TO_ROOM);

    //afsetup( af, CHAR, STR, ADD, -(level/5), level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, hitroll, ADD, -8, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, damroll, ADD, -10, level/4, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,level/4,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,STR,ADD,-(level/5));
    addaff(af,CHAR,hitroll,ADD,-8);
    addaff(af,CHAR,damroll,ADD,-10);
    affect_to_char( victim, &af );
    break;
  case (4):
  case (5):
  case (6):
  case (7): /* legs */
    send_to_char("You feel a sudden intense pain as your legs wither!\n\r",victim);
    act("$n screams in agonry as $s legs crumple beneath $s!",victim,0,0,TO_ROOM);

    //afsetup( af, CHAR, STR, ADD, -(level/10), level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, DEX, ADD, -(level/5), level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, hitroll, ADD, -5, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, damroll, ADD, -5, level/4, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,level/4,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,STR,ADD,-(level/10));
    addaff(af,CHAR,DEX,ADD,-(level/5));
    addaff(af,CHAR,hitroll,ADD,-5);
    addaff(af,CHAR,damroll,ADD,-5);
    affect_to_char( victim, &af );

    ch->move = UMAX( ch->move-(3*level), 0 );

    break;
  case(8): /* body */
    act("$n's body suddenly seems to crumple up and wither!",victim,0,0,TO_ROOM);
    send_to_char("You feel a sudden intense pain as your body gives out and withers up!\n\r",victim);

    //afsetup( af, CHAR, STR, ADD, -8, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, DEX, ADD, -5, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, hitroll, ADD, -6, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, damroll, ADD, -10, level/4, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,level/4,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,STR,ADD,-8);
    addaff(af,CHAR,DEX,ADD,-5);
    addaff(af,CHAR,hitroll,ADD,-6);
    addaff(af,CHAR,damroll,ADD,-10);
    affect_to_char( victim, &af );

    dam *= 2;

    break;
  case (9): /* head */
    send_to_char("Your head ruptures and then shrivels as it undergoes a sudden withering!\n\r",victim);
    act("$n's skull seems to just wither and shrivel up!",victim,0,0,TO_ROOM);

    //afsetup( af, CHAR, STR, ADD, -8, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, DEX, ADD, -5, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, hitroll, ADD, -6, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, damroll, ADD, -10, level/4, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, affected_by, OR, AFF_BLIND, level/4, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,level/4,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,STR,ADD,-8);
    addaff(af,CHAR,DEX,ADD,-5);
    addaff(af,CHAR,hitroll,ADD,-6);
    addaff(af,CHAR,damroll,ADD,-10);
    addaff(af,CHAR,affected_by,OR,AFF_BLIND);
    affect_to_char( victim, &af );
      
    send_to_char("Your eyes are desicated...you are blinded!\n\r",victim);
    dam *= 4;
      
    break;
  }

  ability_damage(ch,victim,dam,sn,DAM_HARM,TRUE,TRUE);
  return;
}

void spell_hand_of_vengeance(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  if (!IS_NPC(victim) && !IS_SET(victim->act,PLR_KILLER) ) {
    act("A huge clenched fist appears above $n but fades without striking.",victim,0,0,TO_ROOM);
    send_to_char("A huge clenched fist appears above you but fades without striking.\n\r",victim);
    send_to_char("The Immortal of Enforcer frowns upon the abuse of this power.\n\r",ch);
    return;
  }
  
  dam = dice(level,6) + dice(level,5) + dice(level,4);
  act("A huge clenched fist appears above $n and strikes down.",victim,0,0,TO_ROOM);
  send_to_char("A huge clenched fist appears above you and strikes down.\n\r",victim);
  
  if (saves_spell(level,victim,DAM_BASH) )
    dam /= 2;
  
  if (number_range(0,3) == 0) {
    act("The blow hammers $n to the ground with savage force!",victim,0,0,TO_ROOM);
    send_to_char("The blow hammers you to the ground with savage force!\n\r",victim);
    dam += dice(level,4);
    WAIT_STATE(victim, 24);
  }
  
  ability_damage(ch,victim,dam,sn,DAM_BASH,TRUE,TRUE);
  
  return;
}

void spell_iceball( int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  
  int dam;
  
  dam = dice(level,6);
  
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    ability_damage(ch,vch, (saves_spell(level,vch,DAM_COLD) ? dam/2 : dam), sn,DAM_COLD,TRUE,TRUE);
  }
  return;
}

void spell_cone_of_cold2(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  
  static const int dam_each[] =
  {
    0,
      4,   5,   6,   7,   8,     10,  15,  20,  30,  40,
      48,  50,  52,  54,  56,    58,  60,  62,  64,  66,
      68,  70,  72,  74,  76,    78,  80,  82,  84,  86,
      88,  90,  92,  94,  96,    98, 100, 102, 105, 130,
      132, 134, 136, 138, 150,   152, 154, 156, 158, 170,

      172, 174, 176, 178, 190,   192, 194, 196, 198, 200,
      202, 204, 206, 208, 210,   232, 234, 236, 238, 240,
      262, 264, 266, 268, 270,   292, 294, 296, 298, 300,
      302, 304, 306, 308, 310,   332, 334, 336, 338, 340,
      345, 350, 355, 365, 370,   375, 380, 385, 390, 395
      
      };
  int dam, tmp_dam;
  
  act("$n creates a freezing blast of air!",ch,0,0,TO_ROOM);
  send_to_char("You draw heat from the room to create a blast of freezing air!\n\r",ch);
  
  level = UMIN(level,( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level = UMAX(0, level);
  dam = number_range(dam_each[level]/2, dam_each[level]*2);

  // Modified by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;

  for (vch = ch->in_room->people;vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

        if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
     
    if ( saves_spell( level, vch, DAM_COLD ) ) 
      tmp_dam = dam/2;
    else {
      tmp_dam = dam;
      // Modified by SinaC 2001 for spell level
      if ( casting_level == 5 )
	cold_effect( (void *) vch, level/2, dam, TARGET_CHAR );
    }
      
    ability_damage(ch,vch,tmp_dam,sn,DAM_COLD,TRUE,TRUE);
  }
  return;
}


void spell_spiritblade(int sn,int level, CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  act("A shadowy blade appears above $n and strikes down!",victim,0,0,TO_ROOM);
  send_to_char("A shadowy blade manifests above you and suddenly descends!\n\r",victim);
  
  dam = dice(level, 5);
  if (saves_spell(level,victim,DAM_ENERGY) )
    dam /= 2;
  
  if (number_percent() < (level/2 + (get_ability(ch,sn)/10) )) {
    act("$n's spiritblade brutally cleaves $N!",ch,0,victim,TO_NOTVICT);
    act("Your spiritblade brutally cleaves $N!",ch,0,victim,TO_CHAR);
    act("$n's spiritblade brutally cleaves you!",ch,0,victim,TO_VICT);
    dam += dice(level,2);
    dam += dice(level/2, 2);
  }
  
  ability_damage(ch,victim,dam,sn,DAM_ENERGY,TRUE,TRUE);
  return;
}

void spell_drain(int sn, int level, CHAR_DATA *ch,void *vo, int target, int casting_level)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf;
  int drain,fail;
  
  if (obj->wear_loc != WEAR_NONE) {
    send_to_char("You can't do that on a worn object.\n\r",ch);
    return;
  }
  if (!IS_SET(obj->extra_flags,ITEM_MAGIC)) {
    send_to_char("That item is not magical.\n\r",ch);
    return;
  }
  switch (obj->item_type) {
  default: 
    //bug("Invalid item drain type: %d.",obj->item_type);
    drain = 1;
    break;
      
  case ITEM_LIGHT :        
    if (obj->value[2] == -1)     drain = 9;
    else drain = 4;
    break;
    
  case ITEM_SCROLL :
    // Added by SinaC 2003
  case ITEM_TEMPLATE:
  case ITEM_WAND :
  case ITEM_STAFF :        drain = obj->value[0]/10;                   break;
  case ITEM_WEAPON :       
    //drain = (obj->value[1] + obj->value[2])/3;  break;
    drain = (GET_WEAPON_DNUMBER(obj)+GET_WEAPON_DTYPE(obj))/3; break;
  case ITEM_TREASURE :     drain = 4;                                  break;
  case ITEM_ARMOR :        drain = 12;                                 break;
  case ITEM_POTION :       drain = 5;                                  break;
  }

  for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
      if (laf->location > ATTR_sex && laf->location < ATTR_dam_type)
	drain += laf->modifier;
  }

  drain *= dice(3,3);
  drain *= obj->level/2;

  drain = UMIN(drain,200);
  
  fail = 95 * get_ability( ch, sn );
  act("$p vaporises in a flash of light!",ch,obj,NULL,TO_ROOM);
  if (number_percent() > fail) {
    act("$p vaporises in a flash of light but you fail to channel the energy.",ch,obj,NULL,TO_CHAR);
    extract_obj(obj);
  }
  
  act("$p vaporises in a flash of light and you feel the energy surge through you.",ch,obj,NULL,TO_CHAR);
  extract_obj(obj);
  
  ch->mana = UMIN(ch->mana + drain, ch->cstat(max_mana));
  return;
}

void spell_earthmaw(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, save_num;
  int count;

  // Modified by SinaC 2001
  if ( ( ch->in_room->cstat(sector) == SECT_AIR )
       || ( ch->in_room->cstat(sector) == SECT_WATER_SWIM )
       || ( ch->in_room->cstat(sector) == SECT_WATER_NOSWIM ) 
       || ( ch->in_room->cstat(sector) == SECT_UNDERWATER ) ) {
    send_to_char("You can't do that in this environment.\n\r",ch);
    return;
  }

  act("$n sunders the ground beneath $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n sunders the ground beneath you.",ch,NULL,victim,TO_VICT);
  act("You sunder the ground beneath $N.",ch,NULL,victim,TO_CHAR);

  save_num = 0;

  dam = dice(level,10) + dice(level,4);

  for (count = 0; count < 2; count++)
    if (saves_spell(level,victim,DAM_BASH) )
      save_num++;
  
  if (save_num == 0) {
    act("$n cries out as $s is crushed savagely within the rift!",victim,NULL,NULL,TO_ROOM);
    send_to_char("You fall into the rift and scream in agony as it crushes you!\n\r",victim);
    ability_damage(ch,victim,dam,sn,DAM_BASH,TRUE,TRUE);
    return;
  }
  dam /= save_num * 2;
  ability_damage(ch,victim,dam,sn,DAM_BASH,TRUE,TRUE);

  return;
}

void spell_windwall(int sn, int level, CHAR_DATA *ch,void *vo, int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  AFFECT_DATA af;

  // Modified by SinaC 2001
  if ( ( ch->in_room->cstat(sector) == SECT_WATER_SWIM )
       || ( ch->in_room->cstat(sector) == SECT_WATER_NOSWIM ) 
       || ( ch->in_room->cstat(sector) == SECT_UNDERWATER ) ) {
    send_to_char("You can't do that in this environment.\n\r",ch);
    return;
  }

  //afsetup(af,CHAR,hitroll,ADD,-3,number_fuzzy(2),level,sn);
  createaff(af,number_fuzzy(2),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,-3);

  send_to_char("You raise a violent wall of wind to strike your foes.\n\r",ch);
  act("$n raises a violent wall of wind, sending debri flying!",ch,NULL,NULL,TO_ROOM);
  dam = dice(level, 5);
  if ( dam > 20 )
    dam += dice(level, 2);
  if ( dam > 40 )
    dam += dice(level, 2);
  if ( dam > 60 )
    dam += dice(level, 2);
  if ( dam > 80 )
    dam += dice(level, 2);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    if ((number_range(0,1) == 0) && !saves_spell(level,vch,DAM_BASH)) {
      act("$n appears blinded by the debris.",vch,NULL,NULL,TO_ROOM);
      affect_to_char(vch,&af);
    }
    ability_damage(ch,vch,dam,sn,DAM_BASH,TRUE,TRUE);

    if ( is_affected( vch, gsn_levitation )
	 || is_affected( vch, gsn_fly) 
	 || is_affected( vch, gsn_flight)
	 || is_affected( vch, gsn_flight_bird) ) {
      if (!saves_spell(level,vch,DAM_BASH) ) {
	act("$n is thrown wildly to the ground by the air blast!",vch,NULL,NULL,TO_ROOM);
	send_to_char("You are thrown down by the air blast!\n\r",vch);
	affect_strip(vch,gsn_fly);
	affect_strip(vch,gsn_levitation);
	affect_strip(vch,gsn_flight);
	affect_strip(vch,gsn_flight_bird);
      }
    }
  }
  return;
}

void spell_web(int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int succ=1;
  
  if (is_affected(victim,sn) ) {
    if (victim == ch)
      send_to_char("You are already covered in sticky webs.\n\r",victim);
    else 
      send_to_char("They are already covered in sticky webs.\n\r",ch);
      
    return;
  }
  
  act("$n points a finger at $N and strands of sticky web spew forth.",
      ch,NULL,victim,TO_NOTVICT);
  act("$n points at you and strands of sticky webs spew forth.",
      ch,NULL,victim,TO_VICT);
  act("You point at $N and send a stream of sticky webs spewing forth.",
      ch,NULL,victim,TO_CHAR);
  
  if ( victim->cstat(DEX) > 23 || saves_spell( level, victim, DAM_OTHER ) )
    succ = 0;

  if ( succ == 0 ) {
    //afsetup( af, CHAR, DEX, ADD, -2, level/5, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,DEX,ADD,-2);
    affect_to_char( victim, &af );
  }
  else {
    //afsetup( af, CHAR, DEX, ADD, -(level/10), level/5, level, sn );
    //affect_to_char( victim, &af );
    //afsetup( af, CHAR, hitroll, ADD, -(level/8), level/5, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,DEX,ADD,-(level/10));
    addaff(af,CHAR,hitroll,ADD,-(level/5));
    affect_to_char( victim, &af );
  }

  victim->move = UMAX(0, victim->move - 60);
  
  act("$n is covered in sticky webs.",victim,NULL,NULL,TO_ROOM);
  send_to_char("You are covered in sticky webs.\n\r",victim);
 
  return;
}

/* Modified turn undead spell. Good and neutral clerics attempt to destroy
   undead, with good aligns getting a bonus. Evil try to either subdue aggro
   undead or control them, depending on the difference in their level.
*/
void spell_old_turn_undead( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int dam = 0;
  int value, count, num;
  CHAR_DATA *follower;
  AFFECT_DATA af;

  follower = NULL; /* follower, count and num used for evil clerics */
  count = 0;
  num = 0;

  if (!IS_EVIL(ch)) {
    act("$n raises $s hands and calls upon the gods to destroy the unholy.",
	ch,NULL,NULL,TO_ROOM);
    act("You raise your hands and call upon the gods to destroy the unholy.", 
	ch,NULL,NULL,TO_CHAR);
    if (!IS_GOOD(ch))
      level -= 3;
  }
  else {
    act("$n turns $s unholy influence upon the room.",ch,0,0,TO_ROOM);
    send_to_char("You turn your unholy influence upon those in the room.\n\r",ch);
  }

  if (!IS_EVIL(ch)) {
    for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
      v_next = victim->next_in_room;
      if (is_same_group(victim,ch) 
	  || !IS_SET(victim->act,ACT_UNDEAD) 
	  || is_safe_spell( ch, victim, TRUE ) 
	  || !IS_SET( victim->cstat(form), FORM_UNDEAD ) ) 
	continue;

      value = UMAX(1, level - victim->level + 10);
      value = UMAX(13, value - 4);
      if (IS_GOOD(ch))
	dam = dice(level, value);

      if ( ( (level > (victim->level + 15) )
	     && IS_NPC(victim) 
	     && IS_GOOD(ch) )
	   || ( (level > (victim->level + 20) ) 
		&& IS_NPC(victim) ) ) {
	act("$N crumbles to dust from the power of $n's turning.",ch,NULL,victim,TO_ROOM);
	act("$N's body crumbles to dust from the power of your turning.",ch,NULL,victim,TO_CHAR);
	sudden_death( ch, victim );
      }
      else {
	ability_damage( ch, victim, saves_spell( level, victim, DAM_HOLY) ? dam / 2 :dam, sn,DAM_HOLY, TRUE, TRUE);
      }
    }
    return;
  }
  else {
    for (follower = char_list; follower != NULL; follower = follower->next)
      if ( (follower->master == ch) 
	   && ( IS_SET(follower->act,ACT_UNDEAD)
		|| IS_SET(follower->cstat(form),FORM_UNDEAD) )
	   && follower != ch) {
	num++;
	count += follower->level;
      }

    for (victim = ch->in_room->people; victim != NULL; victim = v_next) {
      v_next = victim->next_in_room;
      if (is_same_group(victim,ch) 
	  || !IS_SET(victim->act,ACT_UNDEAD) 
	  || is_safe_spell( ch, victim, TRUE ) 
	  || !IS_SET( victim->cstat(form), FORM_UNDEAD ) )
	continue;
      if (IS_AFFECTED(victim,AFF_CHARM))
	continue;

      if ( (ch->level*3 < count*2 ) 
	   || (ch->level < 20 && num > 0)
	   || (ch->level < 40 && num > 1)
	   || (ch->level < 51 && num > 2) 
	   || (ch->level < (victim->level + 10)) 
	   || saves_spell(ch->level,victim,DAM_OTHER)
	   || !IS_NPC(victim)) {
	act("You attempt to control $N but do not have the influence.",
	    ch,0,victim,TO_CHAR);
	continue;
      }
      else {
	act("$n stares in hatred for a moment then suddenly becomes very subdued.",
	    victim,0,0,TO_NOTVICT);

	stop_fighting(victim,TRUE);

	//afsetup(af,CHAR,affected_by,OR,AFF_CHARM,level/4,level,sn);
	//affect_to_char( victim, &af );
	createaff(af,level/4,level,sn,0,AFFECT_ABILITY);
	addaff(af,CHAR,affected_by,OR,AFF_CHARM);
	affect_to_char( victim, &af );

	REMOVE_BIT(victim->act,ACT_AGGRESSIVE);

	add_follower(victim,ch);
	victim->leader = ch;

	act("$N now follows you.",ch,0,victim,TO_CHAR);

	num++;
	count += victim->level;
      }
    }
  }
  return;
}


void spell_channel(int sn,int level, CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  
  if (is_affected(victim,sn) ) {
    if ( victim == ch )
      send_to_char("You are already as healthy as you can get.\n\r",ch);
    else
      act("$N is already as healthy as $e can get.", ch, NULL, victim, TO_CHAR );
    return;
  }
  if (victim->hit > victim->cstat(max_hit)) {
    if ( victim == ch )
      send_to_char("Your mind is already overflowing with health.\n\r",ch);
    else
      act("$N's mind is already overflowing with health.\n\r", ch, NULL, victim, TO_CHAR );
    return;
  }

  victim->hit += victim->cstat(max_hit)/2;

  //afsetup( af, CHAR, max_hit, ADD, ch->cstat(max_hit)/2, 24, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,number_fuzzy(24),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,max_hit,ADD,ch->cstat(max_hit)/2);
  affect_to_char( victim, &af );
  
  send_to_char("You feel your health improve as you control your body with your mind.\n\r",victim);
  if ( victim != ch )
    send_to_char("Ok.\n\r", ch );
  return;
}


void spell_true_sight( int sn, int level, CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (is_affected(victim,sn)) {
    if ( victim == ch )
      send_to_char("You already see truly.\n\r",ch);
    else
      act("$N already sees truly.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_INVIS,level,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN,level,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC,level,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_INFRARED,level,level,sn);
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_INVIS);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC);
  addaff(af,CHAR,affected_by,OR,AFF_INFRARED);
  affect_to_char( victim, &af );

  send_to_char("Your vision sharpens!\n\r",victim);
  if ( ch != victim )
    send_to_char("Ok.\n\r", ch );
  return;
}


void spell_dark_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  if (!IS_GOOD(victim) ) {
    act("$N is unaffected by $n's dark wrath.", ch, NULL,victim,TO_NOTVICT);
    act("$N is unaffected by your dark wrath.",ch,NULL,victim,TO_CHAR);
    act("You are unaffected by $n's dark wrath.",ch,NULL,victim,TO_VICT);
    return;
  }
  
  dam = dice(level,10);
  if (saves_spell(level,victim,DAM_NEGATIVE) )
    dam /= 2;
  ability_damage(ch,victim,dam,sn, DAM_NEGATIVE, TRUE,TRUE);
  
  if (number_range(0,3) != 0)
    return;

    // Modified by SinaC 2001
  spell_curse(gsn_curse, level, ch, (void *) victim, TARGET_CHAR, 1);
  return;
}


void spell_wrath(int sn,int level,CHAR_DATA *ch,void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  if (!IS_EVIL(victim)) {
    act("$N is unaffected by $n's heavenly wrath.",ch,NULL,victim,TO_NOTVICT);
    act("You are unaffected by $n's heavenly wrath.\n\r",ch,NULL,victim,TO_VICT);
    send_to_char("The Gods do not enhance your wrath and frown on your actions\n\r",ch);
    return;
  }

  if (IS_SET(victim->act,ACT_UNDEAD)
      || IS_SET(victim->cstat(form),FORM_UNDEAD))
    level += number_range(2,level/3);
  
  dam = dice(level, 8);
  
  if (level <= 50) dam += dice(level,2);
  else if (level <= 60) dam += dice(level/2,2) + dice(level/2,3);
  else if (level < 70) dam += dice(level,3);
  else if (level < 80) dam += dice(level/2,3) + dice(level/2, 4);
  else if (level < 92) dam += dice(level,4);
  else dam += dice(level,5);
  
  if (saves_spell(level,victim,DAM_HOLY) 
      || saves_spell(level + 5, victim,DAM_HOLY))
    dam /= 2;
  
  act("You call down the wrath of god upon $N.",ch,0,victim,TO_CHAR);
  act("$n calls down the wrath of god upon $N.",ch,0,victim,TO_NOTVICT);
  act("$n calls down the wrath of god upon you.",ch,0,victim,TO_VICT);
  ability_damage(ch,victim,dam,sn,DAM_HOLY,TRUE,TRUE);
  
  if (number_range(0,3) != 0)
    return;

    // Modified by SinaC 2001
  spell_curse(gsn_curse,level,ch,(void *) victim, TARGET_CHAR, 1);
  return;
}

/* RT really nasty high-level attack spell */
void spell_new_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num,sanc_num;
  int wrath_num;

  if (IS_GOOD(ch))  
    wrath_num = ability_lookup("wrath");
  else
    wrath_num = ability_lookup("dark wrath");

  sanc_num = gsn_sanctuary;
  bless_num = gsn_bless;
  curse_num = gsn_curse;
  frenzy_num = gsn_frenzy;

  act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
  send_to_char("You utter a word of divine power.\n\r",ch);
 
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ((IS_GOOD(ch) && IS_GOOD(vch)) ||
	(IS_EVIL(ch) && IS_EVIL(vch)) ||
	(IS_NEUTRAL(ch) && IS_NEUTRAL(vch)) ) {
      send_to_char("You feel full more powerful.\n\r",vch);

    // Modified by SinaC 2001
      spell_frenzy(frenzy_num,level,ch,(void *) vch,TARGET_CHAR, 1); 
      spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR, 1);
      spell_sanctuary(sanc_num,level,ch,(void *) vch,TARGET_CHAR, 1);
    }

    else 
      if ((IS_GOOD(ch) && IS_EVIL(vch)) ||
	  (IS_EVIL(ch) && IS_GOOD(vch)) ) {
	if (!is_safe_spell(ch,vch,TRUE) && !is_same_group(ch,vch)) {
	  // Modified by SinaC 2001
	  spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR, 1);
	  send_to_char("You are struck down!\n\r",vch);
	  if (IS_GOOD(ch))
	    // Modified by SinaC 2001
	    spell_wrath(wrath_num,level,ch,(void *) vch,TARGET_CHAR, 1);
	  else 
	    // Modified by SinaC 2001
	    spell_dark_wrath(wrath_num,level,ch,(void *) vch,TARGET_CHAR, 1);
	}
      }

      else if (IS_NEUTRAL(ch)) {
	if (!is_safe_spell(ch,vch,TRUE) && !is_same_group(ch,vch)) {
    // Modified by SinaC 2001
	  spell_curse(curse_num,level/2,ch,(void *) vch,TARGET_CHAR, 1);
	  send_to_char("You are struck down!\n\r",vch);
	  dam = dice(level,10);
	  ability_damage(ch,vch,dam,sn,DAM_ENERGY,TRUE,TRUE);
	}
      }
  }  
    
  send_to_char("You feel drained.\n\r",ch);
  ch->move /= 2;
  ch->hit /= 2;
}


void spell_new_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, dam_mod, chance;
  
  chance = 100;
  dam_mod = 10;
  if (IS_GOOD(ch)) {
    dam_mod = 5;
    chance = 0;
  }
  else if (IS_NEUTRAL(ch)) {
    dam_mod = 7;
    chance = 50;
  }
  
  dam = dice(level, 10);
  if (number_percent() > chance) {
    /* 
       act("$n's demonfire turns on $m!",ch,0,0,TO_ROOM);
       act("Your demonfire turns on you!",ch,0,0,TO_CHAR);
    */
    dam *= dam_mod;
    dam /= 10;
    if (saves_spell(level,ch,DAM_NEGATIVE))
      dam /= 2;
    //      ability_damage(ch,ch,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
    noaggr_damage( ch, dam, DAM_NEGATIVE,
		   "Your demonfire turns on you!",
		   "$n's demonfire turns on $m!",
		   "has been killed by demonfire.",
		   TRUE );
    return;
  }
  if (victim != ch)  {
    act("$n calls forth the demons of Hell upon $N!",
	ch,NULL,victim,TO_ROOM);
    act("$n has assailed you with the demons of Hell!",
	ch,NULL,victim,TO_VICT);
    send_to_char("You conjure forth the demons of hell!\n\r",ch);
  }
  
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);

    // Modified by SinaC 2001
  spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR, 1);
}

void spell_concatenate(int sn,int level,CHAR_DATA *ch,void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  act("$n points an arm at $N and releases a blast of pure energy.",
      ch,NULL,victim,TO_NOTVICT);
  act("$n gestures at you and a blast of white light engulfs you!",
      ch,NULL,victim,TO_VICT);
  act("You concatenate the magical energies around you and channel them at $N.",
      ch,NULL,victim,TO_CHAR);


  if (level <= 70) dam = dice(level,11);
  else if (level <= 80) dam = dice(level,12);
  else if (level <= 90) dam = dice(level,13);
  else {
    dam = dice(level,13);
    dam += number_range(0,level);
  }

  if (IS_NPC(victim)) {
    if (level <= 70) dam += level;
    else if (level <= 80) dam += (3*level);
    else dam += 4*level;
  }

  if (saves_spell(level,victim, DAM_ENERGY) )
    dam /= 2;

  ability_damage( ch, victim, dam, sn, DAM_ENERGY, TRUE, TRUE);
  if (number_bits(2) == 0)
    // Modified by SinaC 2001
    spell_blindness(gsn_blindness, level*3/4, ch, (void *) victim, TARGET_CHAR, 1);

  send_to_char("You stop channeling the energy and the beam of light dissipates.\n\r", ch);
  act("$n's beam of light dissipates.",ch,NULL,NULL,TO_ROOM);

  return;
   
} 

void spell_power_word_kill(int sn,int level,CHAR_DATA *ch,void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,saves,modify;
  
  saves = 0;
  
  act("$n points a finger at $N and utters the word, 'Die'.",
      ch,NULL,victim,TO_NOTVICT);
  act("$n points a finger at you and utters the word, 'Die'.",
      ch,NULL,victim,TO_VICT);
  send_to_char("You intone a word of unholy power.\n\r",ch);
  
  for ( modify = 0; modify < 4; modify++) {
    if (saves_spell(level,victim,DAM_NEGATIVE) )
      saves++;
  }
  if (saves == 0) {
    act("$N shudders in shock as $S heart explodes!", ch, NULL,victim,TO_NOTVICT);
    send_to_char("You feel your heart rupture in a violent explosion of pain!\n\r",victim);
    act("Your word of power vaporises $N's heart, killing $M instantly!",ch,NULL,victim,TO_CHAR);
    sudden_death( ch, victim );
    return;
  }

  dam = dice(level,14);
  dam /= saves;
  ability_damage(ch,victim,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
  
  return;
}

void spell_evil_eye(int sn,int level, CHAR_DATA *ch,void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, modify, saves;
  
  if (victim == ch) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }
  
  act("$n's eyes turn pitch-black and a surging darkness erupts at $N.",ch,NULL,victim,TO_NOTVICT);
  act("Your eyes turn pitch-black and a stream of pure negative energy streams out at $N.",ch,NULL,victim,TO_CHAR);
  send_to_char("You feel a sudden intense agony burning into your skull.\n\r",victim);
  
  if (IS_AFFECTED(victim,AFF_BLIND) ) {
    dam = dice(level,8);
    if (saves_spell(level,victim, DAM_NEGATIVE) )
      dam /= 2;
    ability_damage(ch,victim,dam,sn,DAM_NEGATIVE, TRUE,TRUE);
    return;
  }
  
  saves = 0;
  for (modify = 0; modify < 4; modify ++) {
    if (saves_spell(level, victim, DAM_NEGATIVE))
      saves++;
  }
  
  if (saves == 0) {
    act("$n's surging darkness devastates $N's skull, killing $M instantly!",ch,NULL,victim,TO_NOTVICT);
    act("Your darkness slays $N!",ch,NULL,victim,TO_CHAR);
    send_to_char("With a violent burning sensation your mind vaporises.\n\r",victim);
    raw_kill(ch,victim);
    return;
  }
  
  dam = dice(level, 16);
  dam /= (saves + 1);
  
  ability_damage(ch,victim,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
  return;
}

/* This is a great spell for having fun with, especially if you come
across a player's corpse :)
-Ceran
*/
void spell_zombify( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *zombie;
  OBJ_DATA *corpse;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj;
  CHAR_DATA *search;
  AFFECT_DATA af;
  const char *name;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int chance;
  int z_level;
  int control;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }
   
  if ((is_affected(ch,sn)
       || is_affected(ch,gsn_mummify)
       || is_affected(ch,gsn_animate_skeleton))
      && level < 70) {
    send_to_char("You have not yet regained your powers over the dead.\n\r",ch);
    return;
  }

  control = 6;

  for (search = char_list; search != NULL; search = search->next) {
    if (IS_NPC(search) && (search->master == ch) 
	&& search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
      control += 6;
    else if (IS_NPC(search) && (search->master == ch)
	     && search->pIndexData->vnum == MOB_VNUM_SKELETON)
      control += 4;
    else if (IS_NPC(search) && (search->master == ch)
	     && search->pIndexData->vnum == MOB_VNUM_MUMMY)
      control += 12;
  }

  if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
      || (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
      || control > 36) {
    send_to_char("You already control as many undead as you can.\n\r",ch);
    return;
  }

  if (target_name[0] == '\0') {
    send_to_char("Animate which corpse?\n\r",ch);
    return;
  }

  corpse = get_obj_here(ch,target_name);

  if (corpse == NULL) {
    send_to_char("You can't animate that.\n\r",ch);
    return;
  }

  if ((corpse->item_type != ITEM_CORPSE_NPC) 
      && (corpse->item_type != ITEM_CORPSE_PC) ) {
    send_to_char("You can't animate that.\n\r",ch);
    return;
  }
  /*
    if (IS_SET(corpse->extra_flags,CORPSE_NO_ANIMATE))
    {
    send_to_char("That corpse can not sustain further life beyond the grave.\n\r",ch);
    return;
    }
  */

  for (obj = corpse->contains; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    obj_from_obj(obj);
    obj_to_room(obj,ch->in_room);
  }
  // Added by SinaC 2001
  recomproom(ch->in_room);


  if ( ch->level < 70 ) {
    //afsetup( af, CHAR, NA, ADD, 0, 12, level, sn ); 
    //affect_to_char(ch, &af);
    createaff(af,12,level,sn,0,AFFECT_ABILITY);
    //addaff(af,CHAR,NA,ADD,0);
    affect_to_char( ch, &af );
  }

  chance = get_ability(ch,sn);

  if (level < corpse->level) {
    chance += (3*level);
    chance -= (3*corpse->level);
  }

  chance = URANGE(20,chance,90);

  if (number_percent() > chance) {
    act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
    act("$n tries to animate a corpse but destroys it.",ch,NULL,NULL,TO_ROOM);
    extract_obj(corpse);
    return;
  }

  act("$n utters an incantation and a burning red glow flares into the eyes of $p.",ch,corpse,NULL,TO_ROOM);
  act("$p shudders and comes to life!",ch,corpse,NULL,TO_ROOM);
  act("You call upon the powers of the dark to give life to $p.",ch,corpse,NULL,TO_CHAR);
 
  zombie = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE));

  // Set form
  SET_BIT( zombie->bstat(form), corpse->value[1] ); SET_BIT( zombie->cstat(form), corpse->value[1] );
  // Copy parts
  zombie->bstat(parts) = zombie->cstat(parts) = corpse->value[2];
  // Remove unwanted form/parts
  REMOVE_BIT( zombie->bstat(parts), PART_HEART|PART_GUTS|PART_BRAINS );
  REMOVE_BIT( zombie->bstat(form), FORM_EDIBLE|FORM_MIST|FORM_INTANGIBLE|FORM_CONSTRUCT);

  z_level = UMAX(1,corpse->level - number_range(1,4));
  zombie->level = z_level;
  zombie->bstat(max_hit) = UMAX((dice(z_level, 15))+(z_level*20),1);
  zombie->hit = zombie->bstat(max_hit);
  zombie->bstat(DICE_TYPE) = UMAX(1, z_level/3);
  if ( z_level >= 1  && z_level <= 10 )
    zombie->bstat(DICE_NUMBER) = 4;
  if ( z_level >= 11 && z_level <= 20 )
    zombie->bstat(DICE_NUMBER) = 5;
  if ( z_level >= 21 && z_level <= 30 )
    zombie->bstat(DICE_NUMBER) = 6;
  if ( z_level > 30 )
    zombie->bstat(DICE_NUMBER) = 7;

  // Modified by SinaC 2001 etho/alignment are attributes now 
  // Modified by SinaC 2000 for alignment/etho
  //zombie->align.alignment = -1000;
  zombie->bstat(alignment) = -1000;
  zombie->bstat(etho) = ETHO_LAWFUL;
  
  // So name starts just after 'The corpse of'
  name = corpse->short_descr+14;

  sprintf( buf1, "the zombie of %s", name);
  sprintf( buf2, "A zombie of %s is standing here.\n\r", name);

  zombie->short_descr = str_dup(buf1);
  zombie->long_descr = str_dup(buf2);

  extract_obj(corpse);

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,-1,level,sn);
  //affect_to_char( zombie, &af );
  createaff(af,-1,level,sn,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( zombie, &af );

  SET_BIT(zombie->act, ACT_CREATED ); // SinaC 2003

  char_to_room(zombie,ch->in_room);
  add_follower(zombie,ch);
  zombie->leader = ch;
  //recompute(zombie); NO NEED: done in char_to_room

  return;
}

/* You'll need a skeleton to use this. Make skeletons with the decay
corpse spell that turns corpses into skeletons. - Ceran
*/
void spell_animate_skeleton( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *skeleton;
  OBJ_DATA *corpse;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj;
  CHAR_DATA *search;
  AFFECT_DATA af;
  const char *name;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int chance;
  int z_level;
  int control;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }
   

  if (is_affected(ch,sn)) {
    send_to_char("You have not yet regained your powers to animate bones.\n\r",ch);
    return;
  }

  control = 4;

  for (search = char_list; search != NULL; search = search->next) {
    if (IS_NPC(search) && (search->master == ch) 
	&& search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
      control += 6;
    else if (IS_NPC(search) && (search->master == ch)
	     && search->pIndexData->vnum == MOB_VNUM_SKELETON)
      control += 4;
    else if (IS_NPC(search) && (search->master == ch)
	     && search->pIndexData->vnum == MOB_VNUM_MUMMY)
      control += 12;
  }

  if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
      || (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
      || control > 36) {
    send_to_char("You already control as many undead as you can.\n\r",ch);
    return;
  }

  if (target_name[0] == '\0') {
    send_to_char("Animate which skeleton?\n\r",ch);
    return;
  }

  corpse = get_obj_here(ch,target_name);

  if (corpse == NULL) {
    send_to_char("You can't find that skeleton.\n\r",ch);
    return;
  }

  //  if (corpse->item_type != ITEM_SKELETON) {
  if ( corpse->item_type != ITEM_CORPSE_NPC
       || !IS_SET( corpse->value[3], CORPSE_SKELETON ) ) {
    send_to_char("You can't animate that.\n\r",ch);
    return;
  }
  /*
    if (IS_SET(corpse->extra_flags,CORPSE_NO_ANIMATE))
    {
    send_to_char("That skeleton does not have the stability to be animated anymore.\n\r",ch);
    return;
    }
  */

  for (obj = corpse->contains; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    obj_from_obj(obj);
    obj_to_room(obj,ch->in_room);
  }
  // Added by SinaC 2001
  recomproom(ch->in_room);

  if ( ch->level < 70 ) {
    //afsetup( af, CHAR, NA, ADD, 0, 8, level, sn ); 
    //affect_to_char(ch, &af);
    createaff(af,8,level,sn,0,AFFECT_ABILITY);
    //addaff(af,CHAR,NA,ADD,0);
    affect_to_char( ch, &af );
  }

  chance = get_ability(ch,sn);

  if (level < corpse->level) {
    chance += (4*level);
    chance -= (3*corpse->level);
  }

  chance = URANGE(10,chance,95);

  if (number_percent() > chance) {
    act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
    act("$n tries to animate a skeleton but destroys it.",ch,NULL,NULL,TO_ROOM);
    extract_obj(corpse);
    return;
  }

  act("$n utters an incantation and $p slowly stumbles to it's feet.",ch,corpse,NULL,TO_ROOM);
  act("$p shudders and slowly stumbles to it's feet!",ch,corpse,NULL,TO_ROOM);
  act("You invoke the powers of death and $p slowly rises to it's feet.",ch,corpse,NULL,TO_CHAR);

  skeleton = create_mobile(get_mob_index(MOB_VNUM_SKELETON));
  // Set form
  SET_BIT( skeleton->bstat(form), corpse->value[1] ); SET_BIT( skeleton->cstat(form), corpse->value[1] );
  // Copy parts
  skeleton->bstat(parts) = skeleton->cstat(parts) = corpse->value[2];

  z_level = UMAX(1,corpse->level - number_range(2,6));
  skeleton->level = z_level;
  skeleton->bstat(max_hit) = UMAX((dice(z_level, 12)) + (z_level*15),1);
  skeleton->hit = skeleton->bstat(max_hit);
  skeleton->bstat(DICE_TYPE) = UMAX(1,(2*z_level)/5);
  skeleton->bstat(DICE_NUMBER) = 5;

  // Modified by SinaC 2001 etho/alignment are attributes now 
  // Modified by SinaC 2000 for alignment/etho
  //skeleton->align.alignment = -1000;
  skeleton->bstat(alignment) = -1000;
  skeleton->bstat(etho) = ETHO_LAWFUL;

  // So name starts just after 'The skeleton of'
  name = corpse->short_descr+16;

  extract_obj(corpse);
    
  sprintf( buf1, "the skeleton of %s", name);
  sprintf( buf2, "A skeleton of %s is standing here.\n\r", name);
  skeleton->short_descr = str_dup(buf1);
  skeleton->long_descr = str_dup(buf2);

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,-1,level,sn);
  //affect_to_char( skeleton, &af );
  createaff(af,-1,level,sn,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( skeleton, &af );

  SET_BIT(skeleton->act, ACT_CREATED ); // SinaC 2003

  char_to_room(skeleton,ch->in_room);
  add_follower(skeleton,ch);
  skeleton->leader = ch;
  // recompute( skeleton ); NO NEED: done in char_to_room

  return;
}

/* hacked off the animate dead spell, mummies are more powerful though,
 *  and you can't control as many. (Ceran)
 */
void spell_mummify( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *mummy;
  OBJ_DATA *corpse;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj;
  CHAR_DATA *search;
  AFFECT_DATA af;
  const char *name;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int chance;
  int z_level;
  int control;
   

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ( (is_affected(ch,gsn_animate_skeleton)
	|| is_affected(ch,sn)
	|| is_affected(ch,gsn_zombify) )
       && level < 70) {
    send_to_char("You have not yet regained your powers to over the dead.\n\r",ch);
    return;
  }
  
  control = 12;
  
  for (search = char_list; search != NULL; search = search->next) {
    if (IS_NPC(search) && (search->master == ch) 
	&& search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
      control += 6;
    else if (IS_NPC(search) && (search->master == ch)
	     && search->pIndexData->vnum == MOB_VNUM_SKELETON)
      control += 4;
    else if (IS_NPC(search) && (search->master == ch)
	     && search->pIndexData->vnum == MOB_VNUM_MUMMY)
      control += 12;
  }
  
  if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
      || (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
      || control > 36) {
    send_to_char("You already control as many undead as you can.\n\r",ch);
    return;
  }
  
  if (target_name[0] == '\0') {
    send_to_char("Attempt to mummify which corpse?\n\r",ch);
    return;
  }
  
  corpse = get_obj_here(ch,target_name);
  
  if (corpse == NULL) {
    send_to_char("You can't find that corpse.\n\r",ch);
    return;
  }
  
  if ((corpse->item_type != ITEM_CORPSE_NPC) 
      && (corpse->item_type != ITEM_CORPSE_PC) ) {
    send_to_char("You can't mummify and animate that.\n\r",ch);
    return;
  }
  /*
    if (IS_SET(corpse->extra_flags,CORPSE_NO_ANIMATE))
    {
    send_to_char("That corpse does not have the stability to be animated anymore.\n\r",ch);
    return;
    }
  */
  /* Had this in but players said it was too hard to mummify with needing to
     successfully emblam and then successfully mummify. But back in if you like.
     -Ceran
     if (!is_affected_obj(corpse,gsn_embalm))
     {
     send_to_char("The corpse must be embalmed in order to mummify it.\n\r",ch);
     return;
     }
  */
  
  for (obj = corpse->contains; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    obj_from_obj(obj);
    obj_to_room(obj,ch->in_room);
  }
  // Added by SinaC 2001
  recomproom(ch->in_room);
  
  if (level<70) {
    //afsetup( af, CHAR, NA, ADD, 0, 20, level, sn ); 
    //affect_to_char(ch, &af);
    createaff(af,20,level,sn,0,AFFECT_ABILITY);
    //addaff(af,CHAR,NA,ADD,0);
    affect_to_char( ch, &af );
  }

  chance = get_ability(ch,sn);

  if (level < corpse->level) {
    chance += (2*level);
    chance -= (3*corpse->level);
  }

  chance = URANGE(10,chance,90);

  if (number_percent() > chance) {
    act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
    act("$n tries to mummify $p but destroys it.",ch,corpse,NULL,TO_ROOM);
    extract_obj(corpse);
    return;
  }

  act("$n utters an incantation and $p slowly stumbles to it's feet.",ch,corpse,NULL,TO_ROOM);
  act("$p shudders and slowly stumbles to it's feet!",ch,corpse,NULL,TO_ROOM);
  act("You invoke the powers of death and $p slowly rises to it's feet.",ch,corpse,NULL,TO_CHAR);

  mummy = create_mobile(get_mob_index(MOB_VNUM_MUMMY));
  // Set form
  SET_BIT( mummy->bstat(form), corpse->value[1] ); SET_BIT( mummy->cstat(form), corpse->value[1] );
  // Copy parts
  mummy->bstat(parts) = mummy->cstat(parts) = corpse->value[2];
  // Remove unwanted form/parts
  REMOVE_BIT( mummy->bstat(parts), PART_HEART|PART_GUTS|PART_BRAINS );
  REMOVE_BIT( mummy->bstat(form), FORM_EDIBLE|FORM_MIST|FORM_INTANGIBLE|FORM_CONSTRUCT);

  z_level = UMAX(1,corpse->level - number_range(0,2));
  mummy->level = z_level;
  mummy->bstat(max_hit) = UMAX((dice(z_level, 25)) + (z_level*25),1);
  mummy->hit = mummy->bstat(max_hit);
  mummy->bstat(DICE_TYPE) = UMAX(1,(z_level/2));
  mummy->bstat(DICE_NUMBER) = 10;


  // Modified by SinaC 2001 etho/alignment are attributes now 
  // Modified by SinaC 2000 for alignment/etho
  //mummy->align.alignment = -1000;
  mummy->bstat(alignment) = -1000;
  mummy->bstat(etho) = ETHO_LAWFUL;

  // So name starts just after 'The corpse of'
  name = corpse->short_descr+14;

  extract_obj(corpse);
    
  sprintf( buf1, "the mummy of %s", name);
  sprintf( buf2, "A torn and shredded mummy of %s is standing here.\n\r", name);
  mummy->short_descr = str_dup(buf1);
  mummy->long_descr = str_dup(buf2);

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,-1,level,ability_lookup("animate dead"));
  //affect_to_char( mummy, &af ); 
  createaff(af,-1,level,gsn_zombify,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( mummy, &af );

  SET_BIT(mummy->act, ACT_CREATED ); // SinaC 2003
  char_to_room(mummy,ch->in_room);
  add_follower(mummy,ch);
  mummy->leader = ch;

  return;
}

/* You need to strip the flesh of corpses using this spell if your necros
want to animate skeleton on the remains. (Ceran)
*/
void spell_decay_corpse(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  OBJ_DATA *corpse;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *skeleton;
  const char *name;
  int chance;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if (target_name[0] == '\0') {
    send_to_char("Decay which corpse?\n\r",ch);
    return;
  }
  
  corpse = get_obj_here(ch,target_name);
  
  if (corpse == NULL) {
    send_to_char("You can't find that object.\n\r",ch);
    return;
  }
  
  if ((corpse->item_type != ITEM_CORPSE_NPC) 
      && (corpse->item_type != ITEM_CORPSE_PC) ) {
    send_to_char("You can't decay that.\n\r",ch);
    return;
  }

  // SinaC 2003
  if ( IS_SET( corpse->value[1], FORM_INSTANT_DECAY )
       || IS_SET( corpse->value[1], FORM_UNDEAD )
       || IS_SET( corpse->value[1], FORM_CONSTRUCT )
       || IS_SET( corpse->value[1], FORM_MIST )
       || IS_SET( corpse->value[1], FORM_INTANGIBLE )
       || IS_SET( corpse->value[1], FORM_INSECT )
       || IS_SET( corpse->value[1], FORM_SPIDER )
       || IS_SET( corpse->value[1], FORM_CRUSTACEAN )
       || IS_SET( corpse->value[1], FORM_WORM )
       || IS_SET( corpse->value[1], FORM_BLOB ) ) {
    send_to_char("You can't decay this kind of corpse.\n\r", ch );
    return;
  }

  for (obj = corpse->contains; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    obj_from_obj(obj);
    obj_to_room(obj,ch->in_room);
  }
  // Added by SinaC 2001
  recomproom(ch->in_room);

  chance = get_ability(ch,sn);
  
  if (number_percent() > chance) {
    act("Your decaying becomes uncontrolled and you destroy $p.",ch,corpse,NULL,TO_CHAR);
    act("$n tries to decay $p but reduces it to a puddle of slime.",ch,corpse,NULL,TO_ROOM);
    extract_obj(corpse);
    return;
  }
  
  act("$n decays the flesh off $p.",ch,corpse,NULL,TO_ROOM);
  act("You decay the flesh off $p and reduce it to a skeleton.",ch,corpse,NULL,TO_CHAR);
 
  // So name starts just after 'The corpse of'
  name = corpse->short_descr+14;

  skeleton = create_object(get_obj_index(OBJ_VNUM_SKELETON), 1);

  //skeleton value0 is corpse's mob vnum
  //skeleton value1 is corpse's form
  //skeleton value2 is corpse's parts
  skeleton->baseval[0] = skeleton->value[0] = corpse->baseval[0];
  //remove unwanted form such has fur, edible, ...
  REMOVE_BIT(corpse->baseval[1], FORM_EDIBLE|FORM_FUR|FORM_COLD_BLOOD);
  skeleton->baseval[1] = skeleton->value[1] = corpse->baseval[1];
  //remove unwanted parts such has heart, guts, brains
  REMOVE_BIT(corpse->baseval[2], PART_HEART|PART_GUTS|PART_BRAINS );
  skeleton->baseval[2] = skeleton->value[2] = corpse->baseval[2];

  skeleton->level = corpse->level;

  obj_to_room(skeleton,ch->in_room);

  extract_obj(corpse);
  
  sprintf( buf1, "the skeleton of %s", name);
  sprintf( buf2, "A skeleton of %s is lying here in a puddle of decayed flesh.", name);
  skeleton->short_descr = str_dup(buf1);
  skeleton->description = str_dup(buf2);
//  skeleton->item_type = ITEM_SKELETON;
  skeleton->item_type = ITEM_CORPSE_NPC;
  SET_BIT( skeleton->baseval[3], CORPSE_SKELETON );
  SET_BIT( skeleton->value[3], CORPSE_SKELETON );

  return;
}

/* Necros use this to keep body parts longer...for flesh golems */
void spell_preserve(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int vnum, chance;

  vnum = obj->pIndexData->vnum;
  if (vnum != OBJ_VNUM_SEVERED_HEAD
      && vnum != OBJ_VNUM_TORN_HEART
      && vnum != OBJ_VNUM_SLICED_ARM
      && vnum != OBJ_VNUM_SLICED_LEG
      && vnum != OBJ_VNUM_GUTS
      && vnum != OBJ_VNUM_BRAINS) {
    send_to_char("You can't preserve that.\n\r",ch);
    return;
  }

  if (obj->timer > 10) {
    send_to_char("It's already in very well preserved condition.\n\r",ch);
    return;
  }
  chance = ( get_ability(ch,sn) * 9 ) / 10;

  if (number_percent() > chance) {
    act("$n destroys $p.",ch,obj,NULL,TO_ROOM);
    act("You fail and destroy $p.",ch,obj,NULL,TO_CHAR);
    extract_obj(obj);
    return;
  }

  act("You surround $p with necromantic magic to slow it's decay.",ch,obj,NULL,TO_CHAR);
  obj->timer += number_range(level/2,level);
  return;
}

void spell_restoration(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int sn_word_fear, sn_wither, sn_drain;
  bool success = FALSE;
  
  sn_word_fear = ability_lookup("word of fear");
  sn_wither = ability_lookup("wither");
  sn_drain = ability_lookup("energy drain");

  if (is_affected(victim,sn_word_fear)) {
    send_to_char("You feel less afraid.\n\r",victim);
    act("$n looks less afraid.",victim,0,0,TO_ROOM);
    affect_strip(victim,sn_word_fear);
    success = TRUE;
  }
  if (is_affected(victim,sn_wither)) {
    send_to_char("Your emaciated body is restored.\n\r",victim);
    act("$n's emaciated body looks healthier.",victim,0,0,TO_ROOM);
    affect_strip(victim,sn_wither);
    success = TRUE;
  }
  if (is_affected(victim,sn_drain)
      && check_dispel(level + 15,victim,sn_drain)) {
    act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
    affect_strip(victim,sn_drain);
    success = TRUE;
  }
    
  if (!success)
    send_to_char("Spell had no effect.\n\r",ch);

  return;
}

// Added by SinaC 2001

// Modified by SinaC 2003
void spell_drain_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {

  if ( target == TARGET_OBJ ) {
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
    
    if(obj->item_type != ITEM_WEAPON) {
      send_to_char("You can only cast this spell on weapons.\n\r",ch);
      return;
    }

    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
      return;
    }
    
    // Added by SinaC 2001
    if ( IS_WEAPON_STAT( obj, WEAPON_HOLY ) ) {
      send_to_char("This weapon is too holy to be vampiric!\n\r", ch );
    }
    
    if(IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)) {
      send_to_char("This weapon is already vampiric.\n\r", ch);
      return;
    }
    
    if(IS_OBJ_STAT(obj,ITEM_BLESS)) {
      send_to_char("This weapon is too blessed.\n\r", ch);
      return;
    }
    
    // af              : affect_data
    // WEAPON          : say affect change weapon flags
    // NA              : not revelant
    // OR              : means we add that flag
    // WEAPON_VAMPIRIC : add vampiric flag
    // level/5         : duration
    // level           : level of spell
    // sn              : means that's drain_blade who is the origin of the affect
    //afsetup(af,WEAPON,NA,OR,WEAPON_VAMPIRIC,level/5,level,sn);
    //affect_to_obj(obj, &af);
    createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
    addaff(af,WEAPON,NA,OR,WEAPON_VAMPIRIC);
    affect_to_obj( obj, &af );
    
    act("$p carried by $n turns {Ddark and vampiric{x.", ch, obj, NULL, TO_ROOM);
    act("$p turns {Ddark and vampiric{x.", ch, obj, NULL, TO_CHAR);
    return;
  }
  
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  if ( saves_spell( level, victim, DAM_NEGATIVE )
       || is_affected( victim, sn ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }
  AFFECT_DATA af;

  //afsetup(af,CHAR,CON,ADD,2,-5,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,2,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,CON,ADD,-5);
  affect_to_char( victim, &af );

  act("$n lose $s will to live.", ch, NULL, victim, TO_CHAR );
  if ( victim != ch )
    send_to_char("You lose your will to live.\n\r", victim );
  return;
}
// Modified by SinaC 2003
void spell_shocking_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  if ( target == TARGET_OBJ ) {
    OBJ_DATA * obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
    
    if(obj->item_type != ITEM_WEAPON) {
      send_to_char("You can only cast this spell on weapons.\n\r",ch);
      return;
    }
    
    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
      return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_SHOCKING)) {
      send_to_char("This weapon is already electrical.\n\r", ch);
      return;
    }
    
    //afsetup(af,WEAPON,NA,OR,WEAPON_SHOCKING,level/5,level,sn);
    //affect_to_obj(obj, &af);
    createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
    addaff(af,WEAPON,NA,OR,WEAPON_SHOCKING);
    affect_to_obj( obj, &af );
    
    act("$p carried by $n {Ysparks with electricity{x.", ch, obj, NULL, TO_ROOM);
    act("$p {Ysparks with electricity{x.", ch, obj, NULL, TO_CHAR);
    return;
  }
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  if ( saves_spell( level, victim, DAM_LIGHTNING )
       || is_affected( victim, sn ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }
  AFFECT_DATA af;

  //afsetup(af,CHAR,DEX,ADD,2,-5,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,2,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,DEX,ADD,-5);
  affect_to_char( victim, &af );

  if (!saves_spell(level,victim,DAM_LIGHTNING)) {
    send_to_char("Your muscles stop responding.\n\r",victim);
    DAZE_STATE(victim,UMAX(12,level));
    WAIT_STATE(victim,PULSE_VIOLENCE);
  }
  
  act("$n is struck weak by the power of lightning.", ch, NULL, victim, TO_CHAR );
  if ( victim != ch )
    send_to_char("You are struck weak by the power of lightning.\n\r", victim );
  return;

}
// Modified by SinaC 2003
void spell_flame_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  if ( target == TARGET_OBJ ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
    
    if(obj->item_type != ITEM_WEAPON) {
      send_to_char("You can only cast this spell on weapons.\n\r",ch);
      return;
    }
    
    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
      return;
    }

    // Added by SinaC 2001
    if ( IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) {
      send_to_char("This weapon is protected against fire!\n\r", ch );
    }
    
    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
      send_to_char("This weapon is already flaming.\n\r", ch);
      return;
    }
    
    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) {
      send_to_char("This weapon is too frost to handle this magic.\n\r", ch);
      return;
    }
    
    //afsetup(af,WEAPON,NA,OR,WEAPON_FLAMING,level/5,level,sn);
    //affect_to_obj(obj, &af);
    createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
    addaff(af,WEAPON,NA,OR,WEAPON_FLAMING);
    affect_to_obj( obj, &af );
    
    act("$p carried by $n gets a {Rfiery aura{x.", ch, obj, NULL, TO_ROOM);
    act("$p gets a {Rfiery aura{x.", ch, obj, NULL, TO_CHAR);
    return;
  }
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  if ( saves_spell( level, victim, DAM_FIRE ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  int dam = dice( level/2, 5 );
  int done = ability_damage( ch, victim, dam, sn, DAM_FIRE, TRUE, TRUE );
  if ( done == DAMAGE_DONE )
    spell_blindness(gsn_blindness,
		    level/2,ch,(void*)victim,TARGET_CHAR,1);
  return;
}
// Modified by SinaC 2003
void spell_frost_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ){
  if ( target == TARGET_OBJ ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;
    
    if(obj->item_type != ITEM_WEAPON) {
      send_to_char("You can only cast this spell on weapons.\n\r",ch);
      return;
    }
    
    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
      return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_FROST)) {
      send_to_char("This weapon is already frost.\n\r", ch);
      return;
    }
    
    if(IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
      send_to_char("This weapon is too warm to handle this magic.\n\r", ch);
      return;
    }
    
    //afsetup(af,WEAPON,NA,OR,WEAPON_FROST,level/5,level,sn);
    //affect_to_obj(obj, &af);
    createaff(af,level/5,level,sn,0,AFFECT_ABILITY);
    addaff(af,WEAPON,NA,OR,WEAPON_FROST);
    affect_to_obj( obj, &af );
    
    act("$p carried by $n grows {Cwickedly cold{x.", ch, obj, NULL, TO_ROOM);
    act("$p grows {Cwickedly cold{x.", ch, obj, NULL, TO_CHAR);
    return;
  }
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  if ( saves_spell( level, victim, DAM_COLD )
       || is_affected( victim, sn ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }
  AFFECT_DATA af;

  //afsetup(af,CHAR,STR,ADD,2,-5,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,2,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,STR,ADD,-5);
  affect_to_char( victim, &af );

  DAZE_STATE(victim,number_fuzzy(5));
  WAIT_STATE(victim,PULSE_VIOLENCE);

  act("$n feels weaken and freezed.", ch, NULL, victim, TO_CHAR );
  if ( victim != ch )
    send_to_char("You fee weaken and freezed.\n\r", victim );
  return;
}

// Added by SinaC 2001, ideas from Legend of Dracon 1
void spell_daemonic_aid(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( victim->fighting != NULL ) {
    if ( victim == ch )
      send_to_char("You can't cast that when fighting.\n\r",ch);
    else
      send_to_char("You can't cast that on person in combat.\n\r",ch);
    return;
  }

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already aided.\n\r",ch);
    else
      act("$N is already aided.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,damroll,ADD,level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,hitroll,ADD,level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,6+level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,damroll,ADD,level/8);
  addaff(af,CHAR,hitroll,ADD,level/8);
  affect_to_char( victim, &af );

  send_to_char( "You feel aided.\n\r", victim );
  if ( ch != victim )
    act("You aid $N.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_daemonic_carapace( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_EVIL(ch ) ) {
    send_to_char("You're not evil enough to cast that spell.\n\r", ch );
    return;
  }
  if ( IS_GOOD(victim) ) {
    act("$N is too pure to cast that spell on.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already surrounded by a spiked carapace.\n\r",ch);
    else
      act("$N is already surrounded by a spiked carapace.",ch,NULL,victim,TO_CHAR);
    return;
  }  

  //afsetup(af,CHAR,allAC,ADD,4-level,level-4,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level-4,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,4-level);
  affect_to_char( victim, &af );

  send_to_char( "You are surrounded by a spiked carapace.\n\r",
		victim );
  act( "$N is surrounded by a spiked carapace.",
       ch, NULL, victim, TO_NOTVICT );
  if ( victim != ch )
    act( "$N is surrounded by a spiked carapace.",
	 ch, NULL, victim, TO_CHAR );
  return;
}

void spell_daemonic_potency(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already as strong as you can get!\n\r",ch);
    else
      act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,STR,ADD, 1 + (level >= 18) + (level >= 25) + (level >= 32),
  //	  level,level,sn);
  //  affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,STR,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));
  affect_to_char( victim, &af );

  send_to_char( "You feel stronger!\n\r", victim );
  act("$n feels stronger.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_repulsion(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if ( IS_AFFECTED(victim, AFF_SANCTUARY) ) {
    if (victim == ch)
      send_to_char("You are already affected by a sanctuary like spell.\n\r",ch);
    else
      act("$N is already affected by a sanctuary like spell.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_SANCTUARY,level/6,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,damroll,ADD,level/8,level/6,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/6,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,damroll,ADD,level/5);
  affect_to_char( victim, &af );

  act( "$n is surrounded by a black aura.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a black aura.\n\r", victim );
  return;
}

void spell_divine_intervention(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( victim->fighting != NULL ) {
    if ( victim == ch )
      send_to_char("You can't cast that when fighting.\n\r",ch);
    else
      send_to_char("You can't cast that on person in combat.\n\r",ch);
    return;
  }

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You already have the favor of your god.\n\r",ch);
    else
      act("$N already has the favor of your god.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,damroll,ADD,level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,hitroll,ADD,level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,6+level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,level/5);
  addaff(af,CHAR,damroll,ADD,level/8);
  affect_to_char( victim, &af );

  send_to_char( "You feel the favor of your god.\n\r", victim );
  if ( ch != victim )
    act("$N feels more powerful.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_holy_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_GOOD(ch ) ) {
    send_to_char("You're not pure enough to cast that spell.\n\r", ch );
    return;
  }
  if ( IS_EVIL(victim) ) {
    act("$N is too evil to cast that spell on.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already surrounded by an holy armor.\n\r",ch);
    else
      act("$N is already surrounded by an holy armor.",ch,NULL,victim,TO_CHAR);
    return;
  }  

  //afsetup(af,CHAR,allAC,ADD,4-level,level-4,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level-4,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,4-level);
  affect_to_char( victim, &af );

  send_to_char( "You are surrounded by an holy armor.\n\r", victim );
  act( "$N is surrounded by an holy armor.", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_holy_sword( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *blade;
  AFFECT_DATA af;

  blade = create_object( get_obj_index( OBJ_VNUM_HOLY_SWORD ), 0 );
  blade->timer = level;
  //blade->baseval[1] = ch->level/3;
  //blade->baseval[2] = ch->level/7;
  blade->baseval[1] = level/5;
  blade->baseval[2] = number_fuzzy(10);
  recompobj(blade);
  blade->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(blade,&af);
  //afsetup(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(blade,&af);
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)));
  addaff(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)));
  affect_to_obj( blade, &af );

  obj_to_char( blade, ch );
  act( "created $p.", ch, blade, NULL, TO_ROOM );
  act( "You have created $p.", ch, blade, NULL, TO_CHAR );
  return;
}

void spell_unholy_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *blade;
  AFFECT_DATA af;

  blade = create_object( get_obj_index( OBJ_VNUM_UNHOLY_BLADE ), 0 );
  blade->timer = level;
  //blade->baseval[1] = ch->level/3;
  //blade->baseval[2] = ch->level/7;
  blade->baseval[1] = level/5;
  blade->baseval[2] = number_fuzzy(10);
  recompobj(blade);
  blade->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(blade,&af);
  //afsetup(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(blade,&af);
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)));
  addaff(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)));
  affect_to_obj( blade, &af );

  obj_to_char( blade, ch );
  act( "created $p.", ch, blade, NULL, TO_ROOM );
  act( "You have created $p.", ch, blade, NULL, TO_CHAR );
  return;
}

void spell_resist_fire( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_FIRE)
       || IS_SET(victim->cstat(imm_flags),IRV_FIRE)) {
    if ( victim == ch )
      send_to_char("You are already resisting to fire.\n\r",ch);
    else
      act("$N is already resisting to fire.", ch, NULL, victim, TO_CHAR );
    return;
  }
  
  //afsetup(af, CHAR, res_flags, OR, IRV_FIRE, level, level, sn );
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_FIRE);
  affect_to_char( victim, &af );
  
  send_to_char( "{RYou are now resisting to fire.{x\n\r", victim );
  if ( ch != victim )
    act("$N is now resisting to {Rfire{x.",ch,NULL,victim,TO_CHAR);
  return;
}

// Spell which need more than one spellcaster in the group, the sum of the casters' level
//  has to be >= 120
// This spell transforms every mobiles who aren't spell safe into centipede :))
void spell_arcane_concordance( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *wch;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  char buf[MAX_STRING_LENGTH];
  int count = 0;
  int race;

  // for every people in the room
  for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room ) {
    // in the same group ?
    if ( IS_NPC(wch) || !is_same_group( ch, wch ) )
      continue;
    // player has the same classes ==> add the level
    if ( ch->cstat(classes) & wch->cstat(classes) == ch->cstat(classes) )
      count += wch->level;
  }
  // high level enough spellcasters?
  if ( count < 120 ) {
    send_to_char("You need a group with more higher level spellcasters.\n\r", ch );
    return;
  }
  // okay, we have a group with high level spellcasters
  // we transform every one in the room into centipede

  race = race_lookup( "centipede" );
  if ( race < 0 ) {
    bug("ARCANE CONCORDANCE: Invalid race");
    return;
  }
  for ( victim = ch->in_room->people; victim != NULL; victim = victim->next_in_room ) {
    if ( !IS_NPC( victim ) || is_safe_spell( ch, victim, TRUE ) )
      continue;

    while ( victim->carrying != NULL ) {
      obj = victim->carrying;
      
      /* Remove the obj if it is worn */
      if (obj->wear_loc != WEAR_NONE)
	unequip_char( victim, obj );
      
      obj_from_char( obj );
      obj_to_room( obj, victim->in_room );
      OBJPROG(obj,victim,"onDropped",victim); // Added by SinaC 2003
    }

//    afsetup(af,CHAR,race,ASSIGN,race,level,level,sn);
//    affect_to_char(victim,&af);
//    af.location = ATTR_parts;
//    af.modifier = race_table[race].parts;
//    affect_to_char(victim,&af);
//    af.location = ATTR_form;
//    af.modifier = race_table[race].form;
//    affect_to_char(victim,&af);
//    af.location = ATTR_DICE_TYPE;
//    af.op = AFOP_ADD;
//    af.modifier = 0-victim->cstat(DICE_TYPE) *4/5;
//    affect_to_char(victim,&af);
//    for (int i = ATTR_STR; i<=ATTR_CON; i++) {
//      af.location = i;
//      af.modifier = 0-victim->curattr[i] *2/3;
//      affect_to_char(victim,&af);
//    }
    createaff(af,level,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,race,ASSIGN,race);
    addaff(af,CHAR,parts,ASSIGN,race_table[race].parts);
    addaff(af,CHAR,form,ASSIGN,race_table[race].form);
    addaff(af,CHAR,DICE_TYPE,ADD,0-victim->cstat(DICE_TYPE) *4/5);
    for ( int i = ATTR_STR; i <= ATTR_CON; i++)
      addaff2(af,AFTO_CHAR,i,AFOP_ADD,-victim->curattr[i] *2/3);
    affect_to_char( victim, &af );

    send_to_charf( victim, "%s's group has changed you to a centipede!\n\r", NAME(ch) );
    sprintf( buf, "%s's group has changed $N into a centipede!", NAME(ch) );
    for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room ) {
      if ( IS_NPC(wch) )
	continue;
      // in the same group ?
      if ( !is_same_group( ch, wch ) )
	act( buf, wch, NULL, victim, TO_CHAR );
      else
	act("You and your group have changed $N into a centipede.",wch,NULL,victim,TO_CHAR);
    }
  }
}

void spell_wraithform( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if ( IS_AFFECTED(victim, AFF_SANCTUARY) ) {
    if (victim == ch)
      send_to_char("You are already affected by a sanctuary like spell.\n\r",ch);
    else
      act("$N is already affected by a sanctuary like spell.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_SANCTUARY,level/6,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,allAC,ADD,-(level-7),level/6,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/6,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,allAC,ADD,-(level/7));
  affect_to_char( victim, &af );

  act( "$n takes a shadowy form.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You take a shadowy form.\n\r", victim );
  return;
}

void spell_death_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int klevel, dam;
  // UMAX added by SinaC 2003
  klevel = UMAX( level - 7, 1 );

  act("$n breathes a {Dcloud of noxious gas{x.",ch,NULL,NULL,TO_ROOM);
  act("You breath a {Dcloud of noxious gas{x over the room.",ch,NULL,NULL,TO_CHAR);

  dam = dice(klevel,5);
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next=  vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
      
    if ( IS_SET(vch->act,ACT_UNDEAD) 
	 || IS_SET( vch->cstat(form), FORM_UNDEAD )
	 || IS_SET(vch->cstat(imm_flags),IRV_NEGATIVE)) {
      act("$n is unaffected by the gas.",vch,0,0,TO_ROOM);
      act("You are unaffected by the gas.",vch,0,0,TO_CHAR);
      continue;
    }
     
    if (saves_spell(klevel, vch, DAM_NEGATIVE) 
	|| vch->level > klevel ) {
      if (saves_spell(level, vch, DAM_NEGATIVE)
	  || vch->level > klevel ) {
	ability_damage(ch,vch,dam/2,sn,DAM_NEGATIVE,TRUE,TRUE);
      }
      else {
	ability_damage(ch,vch,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
      }
    }
    else {
      act("$n gets a horrible look in $s eye's then keels over dead!",
	  vch,0,0,TO_ROOM);
      send_to_char("You feel an intense pain over all your body!\n\r",vch);
      sudden_death( ch, vch );
    }
  }
  return;
}

void spell_water_breath( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED2( ch, AFF2_WATER_BREATH ) ) {
    if ( ch == victim )
      send_to_char("You are already able to breath under water.\n\r", ch );
    else
      act("$N is already able to breath under water.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, affected2_by, OR, AFF2_WATER_BREATH, level, level, sn ); 
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_WATER_BREATH);
  affect_to_char( victim, &af );

  if ( ch == victim )
    send_to_char("You are now able to breath under water.\n\r", ch );
  else {
    act("$N is now able to breath under water.", ch, NULL, victim, TO_CHAR );
    send_to_char("You are now able to breath under water", victim );
  }
  act("$N is now able to breath under water.", ch, NULL, victim, TO_NOTVICT );
  
  return;
}

void spell_walk_on_water( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  AFFECT_DATA af;
  
  if ( IS_AFFECTED2( victim, AFF2_WALK_ON_WATER ) ) {
    if ( ch == victim ) 
      send_to_char("you are already able to walk on water.\n\r", ch );
    else
      act("$N is already able to walk on water.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, affected2_by, OR, AFF2_WALK_ON_WATER, level, level, sn ); 
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_WALK_ON_WATER);
  affect_to_char( victim, &af );

  if ( ch == victim )
    send_to_char("You are now able to walk on water.\n\r", ch );
  else {
    act("$N is now able to walk on water.", ch, NULL, victim, TO_CHAR );
    send_to_char("You are now able to walk on water.", victim );
  }
  act("$N is now able to walk on water.", ch, NULL, victim, TO_NOTVICT );
  
  return;
}

void spell_revitalize( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  
  if (ch->fighting != NULL) {
    send_to_char("You are too occupied to revitalize anyone.{x\n\r",ch);
    return;
  }
  
  if ( victim->fighting != NULL) {
    send_to_char("You cannot revitalize someone in combat!{x\n\r",ch);
    return;
  }

/* Removed by SinaC 2001 
  if ( victim->hit >= victim->cstat(max_hit) )  {
    send_to_char("That would be pointless..\n\r",ch);
    return;
  }
*/  

  send_to_char( "Godly essence flows through your body knitting your wounds!\n\r", victim );
  send_to_char( "You call divine power to heal!\n\r",ch);
  
  affect_strip(victim,gsn_plague);
  affect_strip(victim,gsn_poison);
  affect_strip(victim,gsn_weaken);
  affect_strip(victim,gsn_blindness);
  affect_strip(victim,gsn_sleep);
  affect_strip(victim,gsn_curse);
  
  victim->hit        = victim->cstat(max_hit);
  victim->move       = victim->cstat(max_move);
  update_pos(victim);
  
  return;
}

void spell_magic_mirror( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char( "You have already a magic mirror around you.\n\r", ch );
    else
      act("$N has already a magic mirror around you.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, affected2_by, OR, AFF2_MAGIC_MIRROR, UMAX(level/10,3), level, sn ); 
  //affect_to_char( ch, &af );
  createaff(af,UMAX(level/10,3),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_MAGIC_MIRROR);
  affect_to_char( victim, &af );

  send_to_char("You have created a magic mirror.\n\r", victim );
  if ( victim != ch )
    send_to_char("Ok.\n\r", ch );

  return;
}

void spell_manashield( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if ( IS_AFFECTED(victim, AFF_SANCTUARY) ) {
    if (victim == ch)
      send_to_char("You are already affected by a sanctuary like spell.\n\r",ch);
    else
      act("$N is already affected by a sanctuary like spell.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_SANCTUARY,level/6,level,sn);
  //affect_to_char( victim, &af );
  //afsetup( af, CHAR, max_mana, ADD, level*4, level/6,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/6,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,max_mana,ADD,level*4);
  affect_to_char( victim, &af );

  act( "$n is surrounded by a yellow shield.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a yellow aura.\n\r", victim );
  return;
}

/* 
 * send me any comments at bp49512@hotmail.com 
 */
void spell_prismatic_spray(int sn, int level, CHAR_DATA *ch, void *vo, 
			   int target, int casting_level ) 
{ 
  CHAR_DATA *victim = (CHAR_DATA *) vo; 
  int dam; 

  send_to_char( "You raise your hand and out leaps a {Wc{Do{Gl{Bo{rr{ge{yd{x spray!\n\r", ch ); 
  act( "$n sprays color from their fingers!", ch, NULL, NULL, TO_ROOM ); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {Wwhite ray{x!\n\r", ch ); 
  act( "$n raise your hand and out leaps a {Wwhite ray{x!\n\r", 
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_LIGHT ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_LIGHT,TRUE, TRUE ); 
  spell_blindness(gsn_blindness, level,ch,(void *) victim,TARGET_CHAR, 1 ); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {Dblack ray{x!\n\r", ch ); 
  act( "$n raise your hand and out leaps a {Dblack ray{x!\n\r", 
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_NEGATIVE ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE,TRUE, TRUE ); 
  spell_weaken(gsn_weaken, level,ch,(void *) victim,TARGET_CHAR, 1 ); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {Ggreen ray{x!\n\r", ch ); 
  act( "$n raise your hand and out leaps a {Ggreen ray{x!\n\r",
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_ACID ) )
    dam /= 2;  
  ability_damage( ch, victim, dam, sn, DAM_ACID,TRUE, TRUE ); 
  acid_effect(victim,level,dam,TARGET_CHAR); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {cblue ray{x!\n\r", ch ); 
  act( "$n raise your hand and out leaps a {cblue ray{x!\n\r",
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_COLD ) )
    dam /= 2;  
  ability_damage( ch, victim, dam, sn, DAM_COLD,TRUE, TRUE ); 
  cold_effect(victim,level,dam,TARGET_CHAR); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {rred ray{x!\n\r", ch ); 
  act( "$n raise your hand and out leaps a {rred ray{x!\n\r",
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_FIRE ) )
    dam /= 2;  
  ability_damage( ch, victim, dam, sn, DAM_FIRE,TRUE, TRUE ); 
  fire_effect(victim,level,dam,TARGET_CHAR); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {ggreen ray{x!\n\r", ch ); 
  act( "$n raise your hand and out leaps a {ggreen ray{x!\n\r", 
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_POISON ) )
    dam /= 2;  
  ability_damage( ch, victim, dam, sn, DAM_POISON,TRUE, TRUE ); 
  spell_poison(gsn_poison, level,ch,(void *) victim,TARGET_CHAR, 1 ); 

  dam = dice(level, number_fuzzy(10)); 
  send_to_char( "You raise your hand and out leaps a {Yyellow ray{x!\n\r", ch );
  act( "$n raise your hand and out leaps a {Yyellow ray{x!\n\r", 
       ch, NULL, NULL, TO_ROOM ); 
  if ( saves_spell( level, victim, DAM_LIGHTNING ) )
    dam /= 2;  
  ability_damage( ch, victim, dam, sn, DAM_LIGHTNING,TRUE, TRUE ); 
  shock_effect(victim,level,dam,TARGET_CHAR); 

  return; 
}

void spell_creeping_doom( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  send_to_char( "You summon a horde of insects!\n\r", ch );
  act( "$n summons a horde of insects!", ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL || vch == ch )
      continue;
    if ( vch->in_room == ch->in_room ) {
      if ( !is_safe_spell(ch,vch,TRUE)) {

	if (is_same_group(vch,ch) )
	  continue;

	// Plague Modifier to Creeping Doom, Lloth - 9/4/00

	dam = dice(level+20, 15);
	int tmp_dam = dam,
	  tmp_lvl = level;
	if ( saves_spell( level, vch, DAM_ENERGY ) ) {
	  tmp_dam = dam/2;
	  tmp_lvl = level/2;
	}
	ability_damage( ch, vch, tmp_dam, sn, DAM_ENERGY, TRUE, TRUE );
	spell_plague( gsn_plague, tmp_lvl, ch, (void *) vch, TARGET_CHAR, 1 );
      }
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area )
      send_to_char( "From somewhere nearby you hear a buzzing sound.\n\r", vch );
  }

  return;
}

void spell_moonbeam( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_OUTSIDE(ch) ) {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }
  
  if (moon_night() && weather_info.sky <= SKY_CLOUDLESS ) {
    int i;
    for (i=0;i<NBR_MOON;i++)
      if (moon_visible(i)) {
	dam = dice( 9, 11 );
	if ( saves_spell( level, victim, DAM_LIGHT ) )
	  dam /= 2;
	ability_damage( ch, victim, dam, sn, DAM_LIGHT, TRUE, TRUE);
      }
  }
  else {
    send_to_char("You need to see a moon.\n\r", ch );
    return;
  }
}

void spell_icestorm( int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  
  int dam;
  
  dam = dice(level+2,10);
  
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    ability_damage(ch,vch, (saves_spell(level,vch,DAM_COLD) ? dam/2 : dam), sn,DAM_COLD,TRUE,TRUE);
  }
  return;
}


// Added by SinaC 2003
void spell_shrink( int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected(victim,sn) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  if ( victim->cstat(size) == SIZE_TINY ) {
    if ( victim == ch )
      send_to_char("You are already as small as possible.\n\r", ch );
    else
      act("$N is already as small as possible.", ch, NULL, victim, TO_CHAR );
    return;
  }
  
  //newafsetup( af, CHAR, size, ASSIGN, SIZE_TINY, level*3/2, level, sn , 0);
  //affect_to_char(victim,&af);
  createaff(af,3*level/2,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,size,ASSIGN,SIZE_TINY);
  affect_to_char( victim, &af );
  
  send_to_char( "You feel small, very small.\n\r", victim );
  act( "$n has been shrinked.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_soulbind(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if (!IS_NPC(victim)) {
    send_to_char("You can't bind them to your soul.\n\r",ch);
    return;
  }

  if ( ch->pet != NULL ) {
    if ( IS_NPC(ch->pet)
	 && ( ch->pet->pIndexData->vnum == MOB_VNUM_ZOMBIE
	      || ch->pet->pIndexData->vnum == MOB_VNUM_MUMMY
	      || ch->pet->pIndexData->vnum == MOB_VNUM_SKELETON 
	      || ch->pet->pIndexData->vnum == MOB_VNUM_FLESHGOLEM ) )
      send_to_char("You already have an undead soulbound.\n\r",ch);
    else
      send_to_char("You already have a familiar.\n\r", ch );
    return;
  }

  if ((victim->pIndexData->vnum != MOB_VNUM_ZOMBIE
       && victim->pIndexData->vnum != MOB_VNUM_MUMMY
       && victim->pIndexData->vnum != MOB_VNUM_SKELETON
       && victim->pIndexData->vnum != MOB_VNUM_FLESHGOLEM )
      || victim->master != ch) {
    send_to_char("You can't bind them to your soul.\n\r",ch);
    return;
  }

  if (is_affected(victim,sn)) {
    send_to_char("That undead is already soulbound.\n\r",ch);
    return;
  }

  //afsetup(af,CHAR,hitroll,ADD,3,DURATION_PERMANENT,level,sn);
  //affect_to_char(victim,&af);
  createaff(af,-1,level,sn,casting_level,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,3);
  affect_to_char( victim, &af );
  
  SET_BIT(victim->act,ACT_PET);
  victim->leader = ch;
  ch->pet = victim;
  
  act("$N's eyes burn bright red as it's energy is bound to your soul.",ch,0,victim,TO_CHAR);
  act("$N's eyes burn bright red for a moment.",ch,0,victim,TO_NOTVICT);
  return;
}

// Necromancers use this spell to preserve corpse
void spell_embalm(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int vnum, chance;

  if( obj->item_type != ITEM_CORPSE_NPC ) {
    if( obj->item_type == ITEM_CORPSE_PC )
      send_to_char( "The player wishes to keep their soul.\n\r", ch );
    else
      send_to_char( "It would serve no purpose...\n\r", ch );
    return;
  }

  if (obj->timer > 10) {
    send_to_char("It's already in very well preserved condition.\n\r",ch);
    return;
  }
  chance = ( get_ability(ch,sn) * 9 ) / 10;

  if (number_percent() > chance) {
    act("$n destroys $p.",ch,obj,NULL,TO_ROOM);
    act("You fail and destroy $p.",ch,obj,NULL,TO_CHAR);
    extract_obj(obj);
    return;
  }

  act("You surround $p with necromantic magic to slow it's decay.",ch,obj,NULL,TO_CHAR);
  obj->timer += number_range(level/2,level);
  return;
}
