// Added by SinaC 2001 for skills only certain races can use
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
#include "restriction.h"

#include "handler.h"
#include "comm.h"
#include "act_comm.h"
#include "db.h"
#include "fight.h"
#include "lookup.h"
#include "gsn.h"
#include "olc_value.h"
//#include "disease.h"  removed by SinaC 2003
#include "language.h"
#include "interp.h"
#include "ability.h"
#include "utils.h"
#include "damage.h"


/*************************************************************************************\
 ****************************** NON COMBAT SKILLS ************************************
\*************************************************************************************/

// Added by SinaC 2001 for goblin
void do_war_drums(CHAR_DATA *ch, const char *argument )
{
  AFFECT_DATA af;
  CHAR_DATA *wch;
  int level;
  int count;
  //  int gob;

  //  gob = race_lookup( "goblin" );

  if ( get_ability( ch, gsn_war_drums ) == 0
       && !IS_IMMORTAL(ch)) {
    send_to_char("Huh?\n\r", ch );
    return;
  }
  /*
  if ( ch->cstat(race) != gob
       && !IS_IMMORTAL( ch ) ) {
    send_to_char("You don't know the rythm of the goblin war drums properly.\n\r",ch);
    return;
  }
  */

  level = count = 0;
  // for every people in the room
  for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room ) {
    // in the same group ?
    if ( !is_same_group( ch, wch ) )
      continue;
    // for every goblin
    //if ( wch->cstat(race) == gob ) {
    if ( get_ability( ch, gsn_war_drums ) > 0 ) {
      count++;
      level += wch->level;
    }
  }

  // at least 5 goblins in the group
  if ( count < 5 ) {
    send_to_char( "You need to be more goblins in the group.\n\r", ch );
    return;
  }

  level /= count;

  act("$n starts to play out the blood frenzing goblins war drums song!",ch,0,0,TO_ROOM);
  send_to_char("You let out a fierce goblins war drums song!\n\r",ch);

  for ( wch = ch->in_room->people; wch != NULL; wch = wch->next_in_room ) {
    // if not in same group or not goblin, no effects
    if ( !is_same_group( ch, wch ) 
	 //|| wch->cstat(race) != gob )
	 || get_ability( ch, gsn_war_drums ) == 0 )
      continue;

    if ( is_affected( wch, gsn_war_drums ) ) {
      if ( wch == ch )
	act( "$N is already affected by the power of the goblins war drums.",ch,NULL,wch,TO_CHAR );
      else
	send_to_char("You are already affected by the power of the goblins war drums.\n\r",ch);
      return;
    }

    // Add hit/dam/saves
    //afsetup( af, CHAR, hitroll, ADD, level, level, level, gsn_war_drums );
    //affect_to_char( wch, &af );
    //afsetup( af, CHAR, damroll, ADD, level, level, level, gsn_war_drums );
    //affect_to_char( wch, &af );
    //afsetup( af, CHAR, saving_throw, ADD, -UMIN(level/10,10), level, level, gsn_war_drums );
    //affect_to_char( wch, &af );
    createaff(af,level,level,gsn_war_drums,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,saving_throw,ADD,-UMIN(level/10,10));
    addaff(af,CHAR,hitroll,ADD,level);
    addaff(af,CHAR,damroll,ADD,level);
    affect_to_char( wch, &af );
  
    WAIT_STATE( wch, BEATS( gsn_war_drums ) );
  }

  return;
}

// Resize skill for dwarf, added by SinaC 2001
void do_resize( CHAR_DATA *ch, const char *argument )
{
  OBJ_DATA *obj;
  RESTR_DATA *restr;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int chance, size;

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( ( chance = get_ability(ch,gsn_resize) ) == 0){
    send_to_char("You don't know how to resize!\n\r",ch);
    return;
  }

  if ( arg1[0] == '\0' ) {
    send_to_char( "Which item do you want to resize?\n\r", ch );
    return;
  }

  if ( arg2[0] == '\0' ) {
    send_to_char( "To what size do you want to resize?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if ( ( size = size_lookup( arg2 ) )== -1 ) {
    send_to_char( "It's not a valid size.\n\r", ch );
    return;
  }

  /* Modified by SinaC 2003
  if ( obj->item_type != ITEM_ARMOR 
       && obj->item_type != ITEM_JEWELRY ) {
    send_to_char("You can only resize armors and jewelry.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  //restr = find_restriction( obj, ATTR_size );
  restr = find_restriction( obj->restriction, ATTR_size );
  if ( restr == NULL && !obj->enchanted )
    restr = find_restriction( obj->pIndexData->restriction, ATTR_size );

  if ( restr == NULL ) {
    send_to_char( "This item doesn't need to be resized.\n\r"
		  "It is available for every size!\n\r", ch );
    return;
  }

  chance += ( ch->level - obj->level )*2;
  chance = URANGE( 5, chance, 95 );

  WAIT_STATE( ch, BEATS(gsn_resize) );
  if ( chance < number_percent() ) {
    send_to_char( "You failed to resize.\n\r", ch );
    check_improve(ch,gsn_resize,FALSE,3);
    return;
  }

  // Modified by SinaC 2001
  affect_enchant(obj);
  restr = find_restriction( obj->restriction, ATTR_size );
  if (restr == NULL ) {
    send_to_char("Please warn an IMP!!\n\r", ch );
    bug("do_resize: restriction not existing for %s (%d)",
	obj->short_descr, obj->pIndexData->vnum );
    return;
  }
  restr->value = size;
  check_improve(ch,gsn_resize,TRUE,3);
  recompobj( obj );
  */

  if ( obj->size == SIZE_NOSIZE ) {
    act("$p has no size, it doesn't need to be resized.", ch, obj, NULL, TO_CHAR );
    return;
  }

  chance += ( ch->level - obj->level )*2;
  chance = URANGE( 5, chance, 95 );

  WAIT_STATE( ch, BEATS(gsn_resize) );
  if ( chance < number_percent() ) {
    send_to_char( "You failed to resize.\n\r", ch );
    check_improve(ch,gsn_resize,FALSE,3);
    return;
  }

  obj->size = size;
  check_improve(ch,gsn_resize,TRUE,3);

  sprintf( buf, "$p has been resized to %s.", arg2 );
  act( buf, ch, obj, NULL, TO_CHAR );
  sprintf( buf, "$p has been resized to %s by $n.", arg2 );
  act( buf, ch, obj, NULL, TO_ROOM );
}

// Forge skill for dwarf, added by SinaC 2001
void do_forge(CHAR_DATA *ch,const  char *argument)
{
  OBJ_DATA *obj;
  int chance;

  if (argument[0] == '\0') {
    send_to_char("Forge what item?\n\r",ch);
    return;
  }

  if ( ( obj =  get_obj_carry(ch,argument,ch) ) == NULL ) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ((chance = get_ability(ch,gsn_forge)) <= 0 ) {
    send_to_char("You don't know how to forge!\n\r",ch);
    return;
  }

  if (obj->item_type != ITEM_WEAPON) {
    send_to_char("That isn't a weapon.\n\r",ch);
    return;
  }

  if ( obj->value[0] != WEAPON_MACE
       || attack_table[GET_WEAPON_DAMTYPE(obj)].damage != DAM_BASH ) {
    send_to_char("You can only forge blunt weapons.\n\r",ch);
    return;
  }

  if ( chance < number_percent() ) {
    act("You fail to forge $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_forge,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_forge));
    return;
  }

  chance += ( ch->level - obj->level )*2;
  chance = URANGE( 5, chance, 95 );

  act("You forge $p.",ch,obj,NULL,TO_CHAR);
  act("$n forges $p.",ch,obj,NULL,TO_ROOM);

  obj->baseval[4] |= WEAPON_WEIGHTED;

  check_improve(ch,gsn_forge,TRUE,3);
  WAIT_STATE(ch,BEATS(gsn_forge));
  recompobj( obj );
}

// Vorpalize skill for dwarf, added by SinaC 2001
void do_vorpalize(CHAR_DATA *ch,const  char *argument)
{
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int chance;

  if (argument[0] == '\0') {
    send_to_char("Vorpalize what item?\n\r",ch);
    return;
  }

  if ( ( obj =  get_obj_carry(ch,argument,ch) ) == NULL ) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ((chance = get_ability(ch,gsn_vorpalize)) <= 0 ) {
    send_to_char("You don't know how to vorpalize!\n\r",ch);
    return;
  }

  if (obj->item_type != ITEM_WEAPON) {
    send_to_char("That isn't a weapon.\n\r",ch);
    return;
  }

  if ( ( obj->value[0] != WEAPON_SWORD 
	 && obj->value[0] != WEAPON_DAGGER
	 && obj->value[0] != WEAPON_AXE 
	 && obj->value[0] != WEAPON_POLEARM )
       || attack_table[GET_WEAPON_DAMTYPE(obj)].damage == DAM_BASH ) {
    send_to_char("You can vorpalize only edge weapons.\n\r",ch);
    return;
  }

  chance += ( ch->level - obj->level )*2;
  chance = URANGE( 5, chance, 95 );

  if ( chance < number_percent() ) {
    act("You fail to forge $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_vorpalize,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_vorpalize));
    return;
  }

  act("You vorpalize $p.",ch,obj,NULL,TO_CHAR);
  act("$n vorpalizes $p.",ch,obj,NULL,TO_ROOM);

  //afsetup(af,WEAPON,NA,OR,WEAPON_VORPAL,UMAX(ch->level/5,3),ch->level,gsn_vorpalize);
  //affect_to_obj(obj, &af);
  createaff(af,UMAX(ch->level/5,3),ch->level,gsn_vorpalize,0,AFFECT_ABILITY);
  addaff(af,WEAPON,NA,OR,WEAPON_VORPAL);
  affect_to_obj( obj, &af );
  
  check_improve(ch,gsn_vorpalize,TRUE,3);
  WAIT_STATE(ch,BEATS(gsn_vorpalize));
  recompobj( obj );
}

// Added by SinaC 2001 for quickling
void do_speedup( CHAR_DATA *ch,const  char *argument ) {
  AFFECT_DATA af;

  if ( get_ability( ch, gsn_speedup ) == 0
       && !IS_IMMORTAL(ch)) {
    send_to_char("Huh?\n\r", ch );
    return;
  }

  // if already affect, remove the affect
  AFFECT_DATA *oldaf = affect_find( ch->affected, gsn_speedup );
  if ( oldaf != NULL ) {
    affect_remove( ch, oldaf );
    send_to_char( "You are moving less quickly.\n\r", ch );
    act("$n is moving less quickly.", ch,NULL,NULL,TO_ROOM);
    return;
  }

  int lvl = 1;
  if ( ch->level >= 26 )
    lvl = 2;
  if ( ch->level >= 51 )
    lvl = 3;
  if ( ch->level >= 76 )
    lvl = 4;
  if ( ch->level >= 100 )
    lvl = 5;
  //newafsetup(af, CHAR, affected_by, OR, AFF_HASTE, -1, ch->level, gsn_speedup, lvl );
  // Modified by SinaC 2001 for permanent non-dispellable affects
  //newafsetup(af, CHAR, affected_by, OR, AFF_HASTE, DURATION_PERMANENT, ch->level, 
  //     gsn_speedup, lvl );
  //  affect_to_char( ch, &af );
  createaff(af,-1,ch->level,gsn_speedup,lvl,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_STAY_DEATH);
  addaff(af,CHAR,affected_by,OR,AFF_HASTE);
  affect_to_char( ch, &af );

  send_to_char( "You feel yourself moving more quickly.\n\r", ch );
  act("$n is moving more quickly.", ch, NULL, NULL, TO_ROOM );
}

// Added by SinaC 2001 for sprite
void do_invisible( CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA af;
  // invisible is different from invisibility, invisible is a race ability for sprite
  if ( get_ability( ch, ability_lookup("invisible") ) == 0
       && !IS_IMMORTAL(ch)) {
    send_to_char("Huh?\n\r", ch );
    return;
  }
  /*
  if ( ch->cstat(race) != race_lookup( "sprite" ) 
       && !IS_IMMORTAL(ch)) {
    send_to_char("Huh?\n\r", ch );
    return;
  }
  */
  if ( IS_AFFECTED( ch, AFF_INVISIBLE ) ) {
    send_to_char("You already are invisible.\n\r", ch );
    return;
  }

  // Added by SinaC 2001
  if ( IS_AFFECTED2( ch, AFF2_FAERIE_FOG ) ) {
    send_to_char("Something prevents you from becoming invisible.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001 for permanent non-dispellable affects
  //afsetup( af, CHAR, affected_by, OR, AFF_INVISIBLE, -1, ch->level, gsn_invisible );
  //afsetup( af, CHAR, affected_by, OR, AFF_INVISIBLE, DURATION_PERMANENT, ch->level, 
  //   gsn_invisible );
  //affect_to_char( ch, &af );
  createaff(af,-1,ch->level,gsn_invisible,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_STAY_DEATH);
  addaff(af,CHAR,affected_by,OR,AFF_INVISIBLE);
  affect_to_char( ch, &af );

  send_to_char("You are now invisible.\n\r", ch );
}

void do_repair( CHAR_DATA *ch, const char *argument )
{
  OBJ_DATA *obj;
  char arg[MAX_INPUT_LENGTH];
  int chance;

  if ((chance = get_ability(ch,gsn_repair)) <= 0 ) {
    send_to_char("You don't know how to repair!\n\r",ch);
    return;
  }

  one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Repair what item?\n\r",ch);
    return;
  }

  if ( ( obj = get_obj_carry(ch,arg,ch) ) == NULL ) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ( obj->condition == 100 ) {
    act("$p doesn't need to be repaired.", ch, obj, NULL, TO_CHAR );
    return;
  }

  if ( obj->condition == 0 ) {
    act("$p is totally broken, it can be repaired.", ch, obj, NULL, TO_CHAR );
    return;
  }

  // Modified by SinaC 2001
  if ( IS_NPC(ch) 
       && !IS_AFFECTED(ch,AFF_CHARM)
       && ch->master==NULL
       && ch->leader==NULL )
    chance = 100;
  else {
    chance += ( ch->level - obj->level )*2;
    chance = URANGE( 5, chance, 95 );
  }

  if ( chance < number_percent() ) {
    act("You fail to repair $p.",ch,obj,NULL,TO_CHAR);
    act("$n fails to repair $p.",ch,obj,NULL,TO_ROOM);
    obj->condition -= 25;
    obj->condition = URANGE( 0, obj->condition, 100 );
    if ( obj->condition == 0 ) {
      act("$p is totally broken.", ch, obj, NULL, TO_CHAR );
      act("$p is totally broken.", ch, obj, NULL, TO_ROOM );
    }
    check_improve(ch,gsn_repair,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_repair));
    return;
  }

  obj->condition = 100;
  // Added by SinaC 2001
  affect_strip_obj( obj, gsn_acid_breath );

  recompobj( obj );
  act("You have successfully repaired $p.",ch,obj,NULL,TO_CHAR);
  act("$n has successfully repaired $p.",ch,obj,NULL,TO_ROOM);

  check_improve(ch,gsn_repair,TRUE,3);
  WAIT_STATE(ch,BEATS(gsn_repair));
  return;
}

/*************************************************************************************\
 ******************************** COMBAT SKILLS **************************************
\*************************************************************************************/


// Added by SinaC 2000 for some giant races and ogre
void do_crush(CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;
  int chance, dam; 

  if ( (chance = get_ability(ch,gsn_crush)) <= 0 ) {
    send_to_char("Crushing, how do you do that again?\n\r",ch); 
    return; 
  }

  if ((victim = ch->fighting) == NULL) {
    send_to_char("You aren't fighting anyone.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }


  chance -= chance/4; 
  chance += (ch->level - victim->level) * 2; 
  chance += ch->cstat(STR); 
  chance -= victim->cstat(DEX)/3; 
  chance -= victim->cstat(STR)/2; 

  if (ch->cstat(size) < victim->cstat(size))
    chance += (ch->cstat(size) - victim->cstat(size))*25; 
  else
    chance += (ch->cstat(size) - victim->cstat(size))*10; 

  if (number_percent() < chance) {
    act("$n grabs you and slams you to the ground with bone crushing force!",ch,0,victim,TO_VICT);
    act("You grab $N and slam $M to the ground with bone crushing force!",ch,0,victim,TO_CHAR); 
    act("$n slams $N to the ground with bone crushing force!",ch,0,victim,TO_NOTVICT); 
    check_improve(ch,gsn_crush,TRUE,4);
    if (ch->level < 20) dam = 20;
    else if (ch->level < 25) dam = 30;
    else if (ch->level < 30) dam = 40;
    else if (ch->level < 35) dam = 50;
    else if (ch->level < 40) dam = 60;
    else if (ch->level < 52) dam = 70;
    else dam = 70;

    dam += str_app[ch->cstat(STR)].todam;

    WAIT_STATE(victim,PULSE_VIOLENCE);
    WAIT_STATE(ch,2*PULSE_VIOLENCE);

    ability_damage(ch,victim,dam,gsn_crush,DAM_BASH,TRUE,FALSE);

    victim->position = POS_RESTING;

    return;

  }
  act("Your crush attempt misses $N.",ch,0,victim,TO_CHAR);
  act("$n lashes out wildly with $s arms but misses.",ch,0,0,TO_ROOM);

  WAIT_STATE(ch,2*PULSE_VIOLENCE);

  check_improve(ch,gsn_crush,FALSE,3);

  return;
}

// Added by SinaC 2000 for centaur race
void do_rear_kick(CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;
  int dam, chance;

  if ( ( chance = get_ability( ch, gsn_rear_kick ) ) <= 0 ) {
    send_to_char("You don't have a centaur's rear legs to kick like that.\n\r",ch);
    return;
  }
  
  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You are not fighting anyone.\n\r", ch );
    return;
  }
  
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  WAIT_STATE( ch, BEATS(gsn_rear_kick) );
  act("$n whips about and kicks out with $s hind legs!",ch,0,0,TO_ROOM);
  act("You whip about and kick out with your hind legs!",ch,0,0,TO_CHAR);

  if ( number_percent( ) < chance ) {
    dam = number_range( ch->level/2, ch->level );
    dam += ch->cstat(STR);
    dam += number_range( 1, ch->level );
    dam += number_range( ch->level/4, ch->level/2 );
    
    dam = URANGE( 1, dam, 140 );
    
    ability_damage( ch, victim, dam, gsn_rear_kick,DAM_BASH, TRUE, FALSE );
    check_improve(ch,gsn_rear_kick,TRUE,1);
  }
  else {
    ability_damage( ch, victim, 0, gsn_rear_kick,DAM_BASH,TRUE, FALSE );
    check_improve(ch,gsn_rear_kick,FALSE,1);
  }

  return;
}

// Added by SinaC 2000 for lizardman and dragonish
void do_tail( CHAR_DATA *ch, const char *argument )
{
  CHAR_DATA *victim;

  if ( get_ability(ch,gsn_tail) == 0 ) {
    send_to_char( "You don't have any tail...\n\r", ch );
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
  
  WAIT_STATE( ch, BEATS(gsn_tail) );
  if ( get_ability(ch,gsn_tail) > number_percent()) {
    ability_damage(ch,victim,number_range( 2*ch->level, 3*ch->level ), gsn_tail,DAM_BASH,TRUE, FALSE);
    DAZE_STATE(victim,2*PULSE_VIOLENCE);
    check_improve(ch,gsn_tail,TRUE,1);
  }
  else {
    ability_damage( ch, victim, 0, gsn_tail,DAM_BASH,TRUE, FALSE);
    check_improve(ch,gsn_tail,FALSE,1);
  }
  check_killer(ch,victim);
  return;
}

// Added by SinaC 2001 for banshee
void do_wail( CHAR_DATA *ch, const char *argument )
{
  CHAR_DATA *pChar, *pChar_next;
  int dam;
  pChar_next = NULL;

  if ( get_ability(ch,gsn_wail) == 0 ) {
    send_to_char( "You don't have that ability...\n\r", ch );
    return;
  }

  for ( pChar = ch->in_room->people; pChar; pChar = pChar_next ) {
    pChar_next = pChar->next_in_room;
    if ( pChar == ch )
      continue;
    if ( !is_safe( ch, pChar ) ) {
      act( "{c$n screams a horrible sound! Your ears pop{x!", ch, NULL, pChar, TO_VICT );
      dam = dice(UMAX(ch->level/5,7),UMAX(ch->level/2,7));
      if ( saves_spell( ch->level, pChar, DAM_SOUND ) )	
	dam /= 2;
      ability_damage( ch, pChar, dam, gsn_wail, DAM_SOUND,TRUE,FALSE);
      check_killer( ch, pChar );
    } 
    else
      act( "{c$n screams a horrible sound!{x", ch, NULL, pChar, TO_VICT );
  }
  WAIT_STATE(ch,BEATS(gsn_wail));
  return;
}

// Added by SinaC 2001 for vampire
void do_feed( CHAR_DATA *ch, const char *argument )
{
  CHAR_DATA *victim;
  int dam, chance;

  if ( ( chance = get_ability(ch,gsn_feed) ) == 0 ) {	
    send_to_char("Feed? What's that?\n\r",ch);
    return;
  }

  if ( ( victim = ch->fighting ) == NULL ) {
    send_to_char( "You aren't fighting anyone.\n\r", ch );
    return;
  }

  if ( IS_SET( victim->cstat(form), FORM_UNDEAD ) ) {
    send_to_char( "You can't do that on undead!\n\r", ch );
    return;
  }

  if ( IS_SET( victim->cstat(form), FORM_CONSTRUCT )
       || IS_SET( victim->cstat(form), FORM_MIST ) 
       || IS_SET( victim->cstat(form), FORM_INTANGIBLE ) ) {
    send_to_char( "You can't do that on intangible, mist or construct things!\n\r", ch );
    return;
  }

  if ( victim->hit < victim->cstat(max_hit) / 6) {
    act( "$N is hurt and suspicious ... you can't get close enough.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }

  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  WAIT_STATE( ch, BEATS(gsn_feed) );
  if ( number_percent( ) < chance/2
       || !IS_AWAKE(victim) ) {
    check_improve(ch,gsn_feed,TRUE,1);
    act( "{i$n bites you.{x", ch, NULL, victim, TO_VICT);
    act( "{hYou bite $N.{x", ch, NULL, victim, TO_CHAR);
    act( "{k$n bites $N.{x", ch, NULL, victim, TO_NOTVICT);
    dam=number_range( (((ch->level/2)+(victim->level/2))/*/3*/),
		      (((ch->level/2)+(victim->level/2))/*/3*/)*3 );
    ability_damage( ch, victim, dam, gsn_feed, DAM_NEGATIVE, TRUE, FALSE );
    ch->hit += dam/2;
  }
  else  {
    check_improve(ch,gsn_feed,FALSE,1);
    act( "{i$n tries to bite you, but hits only air.{x", ch, NULL, victim, TO_VICT);
    act( "{hYou chomp a mouthfull of air.{x", ch, NULL, victim, TO_CHAR);
    act( "{k$n tries to bite $N.{x", ch, NULL, victim, TO_NOTVICT);
    ability_damage( ch, victim, 0, gsn_feed,DAM_NEGATIVE,TRUE, FALSE );
  }
  
  return;
}


// Added by SinaC 2001 for vampire
void do_flight( CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA af;

  if ( get_ability( ch, gsn_flight ) == 0
       && !IS_IMMORTAL(ch)) {
    send_to_char("Huh?\n\r", ch );
    return;
  }

  // if already affect, remove the affect
  AFFECT_DATA *oldaf = affect_find( ch->affected, gsn_flight );
  if ( oldaf != NULL ) {
    affect_remove( ch, oldaf );
    send_to_char( "You fly to the ground.\n\r", ch );
    act("$n flies to the ground.", ch,NULL,NULL,TO_ROOM);
    return;
  }

  if ( !IS_AFFECTED( ch, AFF_FLYING ) ) {
    send_to_char( "You fly away from the ground.\n\r", ch );
    act("$n fly away from the ground.", ch, NULL, NULL, TO_ROOM );
  }

  //afsetup(af, CHAR, affected_by, OR, AFF_FLYING, DURATION_PERMANENT, ch->level, gsn_flight );
  //affect_to_char( ch, &af );
  createaff(af,-1,ch->level,gsn_flight,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_STAY_DEATH);
  addaff(af,CHAR,affected_by,OR,AFF_FLYING);
  affect_to_char( ch, &af );
}

void do_mistform( CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA af;

  if ( get_ability( ch, gsn_mistform ) == 0
       && !IS_IMMORTAL(ch)) {
    send_to_char("Huh?\n\r", ch );
    return;
  }

  // if already affect, remove the affect
  if ( is_affected( ch, gsn_mistform ) ) {
    affect_strip( ch, gsn_mistform);
    send_to_char( "You feel solid again.\n\r", ch );
    act("$n takes a solid form.", ch,NULL,NULL,TO_ROOM);
    return;
  }

  // Unequip every items
  bool found = FALSE;
  for ( int iWear = 0; iWear < MAX_WEAR; iWear++ ) {
    OBJ_DATA *obj;
    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL 
	 || iWear == WEAR_FLOAT )
      continue;
    
    unequip_char( ch, obj );
    found = TRUE;
  }

  //afsetup(af, CHAR, affected_by, OR, AFF_PASS_DOOR, DURATION_PERMANENT, ch->level,
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );
  //  afsetup(af, CHAR, allAC, ADD, -3*(ch->level+number_range(2,10)), DURATION_PERMANENT,ch->level, 
  //  gsn_mistform );
  //  affect_to_char( ch, &af );
  // remove vuln_day_light
  //  afsetup(af, CHAR, vuln_flags, NOR, IRV_DAYLIGHT, DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );
  //  afsetup(af, CHAR, res_flags, OR, IRV_WEAPON, DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );
  //  afsetup(af, CHAR, form, NOR, FORM_BIPED, DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );
  //  afsetup(af, CHAR, form, OR, FORM_MIST, DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );
  //  afsetup(af, CHAR, parts, NOR, 
  //	  PART_HEAD|PART_ARMS|PART_LEGS|PART_HANDS|PART_FEET
  //	  |PART_FINGERS|PART_EAR|PART_EYE|PART_FANGS,
  //	  DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );

  // Added by SinaC 2001
  //afsetup(af, CHAR, affected2_by, OR, AFF2_NOEQUIPMENT, DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  //  affect_to_char( ch, &af );

  createaff(af,-1,ch->level,gsn_mistform,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_STAY_DEATH);
  addaff(af,CHAR,affected_by,OR,AFF_PASS_DOOR);
  addaff(af,CHAR,allAC,ADD,-3*(ch->level+number_range(2,10)));
  addaff(af,CHAR,vuln_flags,NOR,IRV_DAYLIGHT);
  addaff(af,CHAR,res_flags,NOR,IRV_WEAPON);
  addaff(af,CHAR,form,NOR,FORM_BIPED);
  addaff(af,CHAR,form,OR,FORM_MIST);
  addaff(af,CHAR,form,NOR,PART_HEAD|PART_ARMS|PART_LEGS|PART_HANDS|PART_FEET
	 |PART_FINGERS|PART_EAR|PART_EYE|PART_FANGS);
  addaff(af,CHAR,affected2_by,OR,AFF2_NOEQUIPMENT);
  affect_to_char( ch, &af );

  send_to_char( "You transform yourself in a mist.\n\r", ch );
  if (found)
    send_to_char("All your equipement have been removed.\n\r", ch);
  act("$n transforms $mself in a mist.", ch, NULL, NULL, TO_ROOM );
}

void do_bite( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  int chance;

  if ( (chance=get_ability(ch,gsn_bite)) == 0 ) {
    send_to_char("Hmmm....\n\r", ch );
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
  
  WAIT_STATE( ch, BEATS(gsn_bite) );
  if ( chance > number_percent()) {
    ability_damage(ch,victim,number_range( 1, 2*ch->level ), gsn_bite, DAM_PIERCE,TRUE, FALSE);
    check_improve(ch,gsn_bite,TRUE,1);
  }
  else {
    ability_damage( ch, victim, 0, gsn_bite,DAM_PIERCE,TRUE, FALSE);
    check_improve(ch,gsn_bite,FALSE,1);
  }
  check_killer(ch,victim);
  return;
}


/*************************************************************************************\
 ********************************* MORPHING SKILLS ***********************************
\*************************************************************************************/
// Race skill for doppleganger, they can morph themself in any selectable race
void do_morph( CHAR_DATA *ch, const char *argument ) {
  char *art;
  char race[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int iRace, timer;
  /*
  send_to_charf(ch,"lvl: %d   learned: %d   sk: %d\n", 
		ch->pcdata->ability_info[gsn_morph].level,
		ch->pcdata->ability_info[gsn_morph].learned,
		get_ability(ch,gsn_morph));
  */

  if ( is_affected( ch, gsn_morph ) // polymorph test added by SinaC 2003
       || is_affected( ch, gsn_polymorph_self ) ) {
    send_to_char( "You are already under the effect of the morph!\n\r", ch );
    return;
  }

  one_argument( argument, race );

  if ( get_ability(ch,gsn_morph) <= 0 ) {	
    send_to_char("Huh???\n\r",ch);
    return;
  }

  if ( race[0] == '\0' ) {
    send_to_char("You need to specify a race.\n\r", ch );
    return;
  }

  iRace = race_lookup( race, TRUE );
  if ( iRace < 0 
       || !race_table[iRace].pc_race
       //|| ( !pc_race_table[iRace].pickable && !IS_IMMORTAL(ch))) { modified by SinaC 2003
       || ( pc_race_table[iRace].type != RACE_CREATION && !IS_IMMORTAL(ch))) {
    send_to_char("That's not an acceptable race.\n\r", ch );
    return;
  }

  timer = UMAX( 10, ch->level/5 );
  // change race, form, parts, imm, res, vuln, aff
  change_race( ch, iRace, timer, ch->level, gsn_morph, 0 );

/* Removed by SinaC 2003, automatically done with new affect wearoff/update
  // Modified by SinaC 2003  A doppleganger keeps its morph ability while morphed and 
  //  lose every affects while morphing
  // we remove every affects to avoid a doppleganger keeping race ability affect 
  //  while morphing into another race
  while ( ch->affected )
    affect_remove( ch, ch->affected );
  // change race, form, parts, imm, res, vuln, aff
  change_race( ch, iRace, -1, ch->level, gsn_morph, 0 ); // -1 means no duration
  // artificially set skill level to 1 and percentage to 100%  so even if morphed
  //  the doppleganger keeps his morph ability
  ch->pcdata->ability_info[gsn_morph].level = 1; // should be 0 but class_skilllevel force us to set to 1
  ch->pcdata->ability_info[gsn_morph].learned = 100;
*/

  strcpy( race, pc_race_table[iRace].name );
  if ( race[0] == 'a' 
       || race[0] == 'e' 
       || race[0] == 'i' 
       || race[0] == 'o' 
       || race[0] == 'u'
       || race[0] == 'y' )
    art = "an";
  else
    art = "a";
  send_to_charf(ch, "You look like %s %s now.\n\r",
		art, race);
  sprintf( buf, "$n starts to morph ... and $e now looks like %s %s.", art, race );
  act( buf, ch, NULL, NULL, TO_ROOM );
  return;
}

// SHOULD NOT EXIST

// Transform a player to a werewolf
// Add STR, DEX, CON
// WIS and INT set to 8
// Add hit/dam  (+level)
// Add max hp   (+level*10)
// Set race, parts, form, imm, res, vuln, aff and size to werewolf one
// Set class to warrior
//-- Set dam_type to claw      useless now
// Unequip every item and drop them to the ground
// Unable to wear any equipement: act_obj.C
void do_lycanthropy( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  AFFECT_DATA af;
  OBJ_DATA *obj;
  int race;
  int level;
  int sn;
  int classes;
  int timer;
  bool found;

  if ( get_ability(ch,gsn_lycanthropy) == 0 ) {
    send_to_char( "You can't use that...\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  if ( arg[0] != '\0' ) {
    send_to_char( "Lycanthropy doesn't need any arguments.\n\r", ch );
    return;
  } 

  sn = gsn_lycanthropy;

  if ( is_affected( ch, sn ) ) {
    send_to_char( "You are already under the effect of the lycanthropy!\n\r", ch );
    return;
  }

  level = ch->level;
  timer = number_fuzzy( level );
  race = race_lookup( "werewolf" );
  classes = 1<<class_lookup( "warrior" );
  if ( race < 0 ) {
    bug("LYCANTHROPY: Invalid race");
    return;
  }

  // Unequip every items and drop them on the floor
  found = FALSE;
  for ( int iWear = 0; iWear < MAX_WEAR; iWear++ ) {
    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL 
	 || iWear == WEAR_FLOAT /*|| iWear == WEAR_BRAND  removed by SinaC 2003*/ )
      continue;
    
    unequip_char( ch, obj );
    //obj_from_char( obj );
    //obj_to_room( obj, ch->in_room );

    found = TRUE;
  }

  // change race, form, parts, imm, res, vuln, aff, size
  //change_race( ch, race, timer, level, sn, 0 );
  createaff(af,timer,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,race,ASSIGN,race);
  addaff(af,CHAR,parts,ASSIGN,race_table[race].parts);
  addaff(af,CHAR,form,ASSIGN,race_table[race].form);
  addaff(af,CHAR,imm_flags,OR,race_table[race].imm);
  addaff(af,CHAR,res_flags,OR,race_table[race].res);
  addaff(af,CHAR,vuln_flags,OR,race_table[race].vuln);
  addaff(af,CHAR,affected_by,OR,race_table[race].aff);
  //if ( race_table[race].pc_race )
  //addaff(af,CHAR,size,ASSIGN,pc_race_table[race].size);
  addaff(af,CHAR,size,ASSIGN,race_table[race].size);

  // classes
  //afsetup(af,CHAR,classes,ASSIGN,classes,timer,level,sn);
  //affect_to_char(ch,&af);
  addaff(af,CHAR,classes,ASSIGN,classes);

  // Removed by SinaC 2001, it's check in fight.C now, search PART_CLAWS
  // player get claws
  //afsetup(af,CHAR,dam_type,ASSIGN,5,timer,level,sn);
  //affect_to_char(ch,&af);

  // hitroll
  //afsetup(af,CHAR,hitroll,ADD,2*level,timer,level,sn);
  //affect_to_char(ch,&af);
  addaff(af,CHAR,hitroll,ADD,2*level);

  // damroll
  //afsetup(af,CHAR,damroll,ADD,2*level,timer,level,sn);
  //affect_to_char(ch,&af);
  addaff(af,CHAR,damroll,ADD,2*level);

  // raise max hp
  //afsetup(af,CHAR,max_hit,ADD,10*level,timer,level,sn);
  //affect_to_char(ch,&af);
  addaff(af,CHAR,max_hit,ADD,10*level);
  
  // Add STR, DEX, CON
  //afsetup( af, CHAR, STR, ADD, 10, timer, level, sn );
  //affect_to_char(ch,&af);
  //afsetup( af, CHAR, CON, ADD,  5, timer, level, sn );
  //affect_to_char(ch,&af);
  //afsetup( af, CHAR, DEX, ADD,  5, timer, level, sn );
  //affect_to_char(ch,&af);
  addaff(af,CHAR,STR,ADD,10);
  addaff(af,CHAR,CON,ADD,5);
  addaff(af,CHAR,DEX,ADD,5);

  // Set INT and WIS to 8
  //afsetup( af, CHAR, INT, ASSIGN, 8, timer, level, sn );
  //affect_to_char(ch,&af);
  //afsetup( af, CHAR, WIS, ASSIGN, 8, timer, level, sn );
  //affect_to_char(ch,&af);
  addaff(af,CHAR,INT,ASSIGN,8);
  addaff(af,CHAR,WIS,ASSIGN,8);

  // No equipment
  //afsetup(af, CHAR, affected2_by, OR, AFF2_NOEQUIPMENT, DURATION_PERMANENT, ch->level, 
  //	  gsn_mistform );
  addaff(af,CHAR,affected2_by,OR,AFF2_NOEQUIPMENT);
  affect_to_char( ch, &af );

  //recompute( ch ); NOT NEEDED: done in affect_to_char

  send_to_charf( ch, "You look like a %s now.\n\r", pc_race_table[race].name );
  if (found) {
    send_to_char("All your equipement have been removed.\n\r", ch);
    recomproom( ch->in_room );
  }
}
