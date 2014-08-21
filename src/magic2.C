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
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"

// Added by SinaC 2001
#include "comm.h"
#include "handler.h"
#include "clan.h"
#include "db.h"
#include "effects.h"
#include "fight.h"
#include "act_comm.h"
#include "scan.h"
#include "olc_value.h"
#include "spells_def.h"
#include "utils.h"
#include "damage.h"


void spell_portal( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim;
  OBJ_DATA *portal, *stone;

  if ( ( victim = get_char_world( ch, target_name ) ) == NULL
       ||   victim == ch
       ||   victim->in_room == NULL
       ||   !can_see_room(ch,victim->in_room)
       // Modified by SinaC 2001
       ||   IS_SET(victim->in_room->cstat(flags), ROOM_SAFE)
       ||   IS_SET(victim->in_room->cstat(flags), ROOM_PRIVATE)
       ||   IS_SET(victim->in_room->cstat(flags), ROOM_SOLITARY)
       ||   IS_SET(victim->in_room->cstat(flags), ROOM_NO_RECALL)
       ||   IS_SET(ch->in_room->cstat(flags), ROOM_NO_RECALL)
       // Added by SinaC 2001
       ||   IS_SET(victim->act, ACT_IS_SAFE )
       ||   victim->level >= level + 3
       ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
       ||   (IS_NPC(victim) && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
       ||	(is_clan(victim) && !is_same_clan(ch,victim))) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }   

  stone = get_eq_char(ch,WEAR_HOLD);
  if (!IS_IMMORTAL(ch) 
      &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE)) {
    send_to_char("You lack the proper component for this spell.\n\r",ch);
    return;
  }

  if (stone != NULL && stone->item_type == ITEM_WARP_STONE) {
    act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
    act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
    extract_obj(stone);
  }

  portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
  portal->timer = 2 + level / 25; 
  portal->baseval[3] = victim->in_room->vnum;
  recompobj(portal);

  obj_to_room(portal,ch->in_room);
  // Added by SinaC 2001
  recomproom(ch->in_room);

  act("{c$p rises up from the ground.{x",ch,portal,NULL,TO_ROOM);
  act("{c$p rises up before you.{x",ch,portal,NULL,TO_CHAR);
}

void spell_nexus( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *victim;
  OBJ_DATA *portal, *stone;
  ROOM_INDEX_DATA *to_room, *from_room;
  
  from_room = ch->in_room;
  
  if ( ( victim = get_char_world( ch, target_name ) ) == NULL
       ||   victim == ch
       ||   (to_room = victim->in_room) == NULL
       ||   !can_see_room(ch,to_room) || !can_see_room(ch,from_room)
       // Modified by SinaC 2001
       ||   IS_SET(to_room->cstat(flags), ROOM_SAFE)
       ||	 IS_SET(from_room->cstat(flags),ROOM_SAFE)
       ||   IS_SET(to_room->cstat(flags), ROOM_PRIVATE)
       ||   IS_SET(to_room->cstat(flags), ROOM_SOLITARY)
       ||   IS_SET(to_room->cstat(flags), ROOM_NO_RECALL)
       ||   IS_SET(from_room->cstat(flags),ROOM_NO_RECALL)
       // Added by SinaC 2001
       ||   IS_SET(victim->act, ACT_IS_SAFE )
       ||   victim->level >= level + 3
       ||   (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */
       ||   (IS_NPC(victim) && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       ||   (IS_NPC(victim) && saves_spell( level, victim,DAM_NONE) ) 
       ||	 (is_clan(victim) && !is_same_clan(ch,victim))) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }   
  
  stone = get_eq_char(ch,WEAR_HOLD);
  if (!IS_IMMORTAL(ch)
      &&  (stone == NULL || stone->item_type != ITEM_WARP_STONE)) {
    send_to_char("You lack the proper component for this spell.\n\r",ch);
    return;
  }
  
  if (stone != NULL && stone->item_type == ITEM_WARP_STONE) {
    act("You draw upon the power of $p.",ch,stone,NULL,TO_CHAR);
    act("It flares brightly and vanishes!",ch,stone,NULL,TO_CHAR);
    extract_obj(stone);
  }
  
  /* portal one */ 
  portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
  portal->timer = 1 + level / 10;
  portal->baseval[3] = to_room->vnum;
  recompobj(portal);
  
  obj_to_room(portal,from_room);
  // Added by SinaC 2001
  recomproom(from_room);
  
  act("$p rises up from the ground.",ch,portal,NULL,TO_ROOM);
  act("$p rises up before you.",ch,portal,NULL,TO_CHAR);
  
  /* no second portal if rooms are the same */
  if (to_room == from_room)
    return;
  
  /* portal two */
  portal = create_object(get_obj_index(OBJ_VNUM_PORTAL),0);
  portal->timer = 1 + level/10;
  portal->baseval[3] = from_room->vnum;
  recompobj(portal);
  
  obj_to_room(portal,to_room);
  // Added by SinaC 2001
  recomproom(to_room);
  
  if (to_room->people != NULL) {
    act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
    act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
  }
}


/* Added by Sinac 1997 for spell Black Lotus */

void spell_black_lotus( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
 
  victim->mana = UMIN( victim->mana+victim->mana/3, victim->cstat(max_mana) );
  
  send_to_char( "Wow ... What a rush !\n\r", victim );
  act( "$n glows with energy for a second.", victim, NULL, NULL, TO_ROOM );
  
  return;
   
}

/*
 *
 * That's it!  Feel free to use it as you please... a little credit 
 * somewhere would be nice, but not necessary.  Oh, and any improvements 
 * (*beam*) or bugs (*cringe*), please write to aprocter@mail.coin.missouri.edu.
 *
 * --Dribble
 */
void spell_resurrect( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  OBJ_DATA *obj;
  CHAR_DATA *mob;
  int i;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  obj = get_obj_here( ch, target_name );

  if ( obj == NULL ) {
    send_to_char( "Resurrect what?\n\r", ch );
    return;
  }

  /* Nothing but NPC corpses. */

  if( obj->item_type != ITEM_CORPSE_NPC ) {
    if( obj->item_type == ITEM_CORPSE_PC )
      send_to_char( "You can't resurrect players.\n\r", ch );
    else
      send_to_char( "It would serve no purpose...\n\r", ch );
    return;
  }

  if( obj->level > (ch->level + 2) ) {
    send_to_char( "You couldn't call forth such a great spirit.\n\r", ch );
    return;
  }

  if( ch->pet != NULL ) {
    send_to_char( "You already have a pet.\n\r", ch );
    return;
  }

  /* Chew on the zombie a little bit, recalculate level-dependant stats */

  mob = create_mobile( get_mob_index( MOB_VNUM_ZOMBIE ) );
  mob->level                 = obj->level;
  mob->bstat(DICE_NUMBER)    = ( mob->level + 9 ) / 10;
  mob->bstat(DICE_TYPE)      = 6;
  mob->bstat(max_hit)        = mob->level * 8 + 
    number_range( mob->level * mob->level/4, mob->level * mob->level);

  mob->bstat(max_hit)        = mob->bstat(max_hit) * 9 / 10;
  mob->hit                   = mob->bstat(max_hit);
  mob->bstat(max_mana)       = 100 + dice(mob->level,10);
  mob->mana                  = mob->bstat(max_mana);
  // Added by SinaC 2001 for mental user
  mob->bstat(max_psp)       = 100 + dice(mob->level,10);
  mob->psp                  = mob->bstat(max_psp);
  mob->bstat(max_move)       = mob->bstat(max_mana);
  mob->move                  = mob->bstat(max_mana);
  for (i = 0; i < 3; i++)
    mob->bstat(ac0+i)        = interpolate(mob->level,100,-100);
  mob->bstat(ac0+3)          = interpolate(mob->level,100,0);

  for (i = 0; i < MAX_STATS; i++)
    mob->bstat(stat0+i) = 11 + mob->level/4;

  /* You rang? */
  extract_obj(obj);

  /* Yessssss, massssssster... */
  SET_BIT(mob->bstat(affected_by), AFF_CHARM);
  SET_BIT(mob->act, ACT_PET|ACT_CREATED); // SinaC 2003
  mob->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
  add_follower( mob, ch );
  mob->leader = ch;
  ch->pet = mob;

  char_to_room( mob, ch->in_room );
  act( "$p springs to life as an hideous zombie!", ch, obj, NULL, TO_ROOM );
  act( "$p springs to life as an hideous zombie!", ch, obj, NULL, TO_CHAR );
 
 /* For a little flavor... */
  do_say( mob, "How may I serve you, master?" );
  //  recompute(mob); NO NEED: done in char_to_room
  return;
}

void spell_acid_arrow( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(2 + level/2, 4);

  // Modified by SinaC 2001
  /*
  if ( saves_spell( level, victim,DAM_ACID) )
    dam /= 2;
  damage( ch, victim, dam, sn, DAM_ACID ,TRUE, TRUE);
  */

  if (saves_spell(level,victim,DAM_ACID)) {
    acid_effect( victim, level/2, dam/4, TARGET_CHAR);
    ability_damage( ch, victim, dam/2, sn, DAM_ACID, TRUE, TRUE);
  }
  else {
    acid_effect( victim, level, dam, TARGET_CHAR);
    int done = ability_damage( ch, victim, dam, sn, DAM_ACID, TRUE, TRUE);
    if (done == DAMAGE_DONE )
      // Modified by SinaC 2001
      spell_poison( sn, level/2, ch, victim, TARGET_CHAR, 1 );
  }

  return; 
}

void spell_flame_arrow( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(4 + level/2, 6);
  if ( saves_spell( level, victim,DAM_FIRE) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);
  return; 
}

void spell_create_sword( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  OBJ_DATA *sword;
  AFFECT_DATA af;

  sword = create_object( get_obj_index( OBJ_VNUM_BLADE ), 0 );
  sword->timer = level;
  //sword->baseval[1] = UMAX(ch->level/3,3);
  //sword->baseval[2] = UMAX(ch->level/7,2);
  sword->baseval[1] = UMAX(level/5,5);
  sword->baseval[2] = number_fuzzy(10);
  recompobj(sword);
  sword->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(sword,&af);
  //afsetup(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(sword,&af);
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)));
  addaff(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)));
  affect_to_obj( sword, &af );

  obj_to_char( sword, ch );
  act( "created $p.", ch, sword, NULL, TO_ROOM );
  act( "You have created $p.", ch, sword, NULL, TO_CHAR );
  return;
}

void spell_create_dagger( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  OBJ_DATA *dagger;
  AFFECT_DATA af;

  dagger = create_object( get_obj_index( OBJ_VNUM_KNIFE ), 0 );
  dagger->timer = level;
  //dagger->baseval[1] = UMAX(ch->level/3,3);
  //dagger->baseval[2] = UMAX(ch->level/8,2);
  dagger->baseval[1] = UMAX(level/5,5);
  dagger->baseval[2] = number_fuzzy(8);

  recompobj(dagger);
  dagger->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(dagger,&af);
  //afsetup(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(dagger,&af);
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)));
  addaff(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)));
  affect_to_obj( dagger, &af );

  obj_to_char( dagger, ch );
  act( "$n has created $p.", ch, dagger, NULL, TO_ROOM );
  act( "You have created $p.", ch, dagger, NULL, TO_CHAR );
  return;
}


void spell_aid( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = ( CHAR_DATA *) vo;
  AFFECT_DATA af;

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
  //afsetup(af,CHAR,saving_throw,ADD,0-level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level+6,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,saving_throw,ADD,0-level/8);
  addaff(af,CHAR,damroll,ADD,level/8);
  affect_to_char( victim, &af );


  send_to_char( "You feel aided.\n\r", victim );
  if ( ch != victim )
    act("You aid $N.",ch,NULL,victim,TO_CHAR);
  return;
}


void spell_blur ( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already distorted.\n\r",ch);
    else
      act("$N is already distorted.",ch,NULL,victim,TO_CHAR);
    return;
  }  

  //afsetup(af,CHAR,allAC,ADD,4-level,level-4,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,UMAX(level-4,1),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,4-level);
  affect_to_char( victim, &af );

  send_to_char( "You shift, waver and appear distorded.\n\r",
		victim );
  act( "$N shifts, wavers, and appears distorded.",
       ch, NULL, victim, TO_NOTVICT );
  return;
}
