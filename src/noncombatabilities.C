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
#include "spells_def.h"
#include "recycle.h"
#include "interp.h"

// Added by SinaC 2001
#include "classes.h"
//#include "restriction.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "update.h"
#include "fight.h"
#include "act_obj.h"
#include "act_wiz.h"
#include "save.h"
#include "act_move.h"
#include "act_comm.h"
#include "lookup.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "clan.h"
#include "wiznet.h"
#include "condition.h"
// Added by SinaC 2003
#include "act_info.h"
#include "ability.h"
#include "bit.h"
#include "const.h"
#include "utils.h"


/*************************************************************************************\
 ****************************** NON COMBAT SKILLS ************************************
\*************************************************************************************/

void do_recall( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  ROOM_INDEX_DATA *location = NULL;
  int dest_vnum = 0;

  if (IS_NPC(ch)) {
    if (IS_SET(ch->act,ACT_PET))
      if (ch->leader)
	dest_vnum = ch->leader->in_room->vnum;
      else {
	bug("do_recall: Pet<->leader relation broken! Pet is vnum %d",ch->pIndexData->vnum);
	return;
      }
    else {
      // Modified by SinaC 2001, so charmies can recall
      if ( IS_AFFECTED( ch, AFF_CHARM ) && 
	   ch->master ) {
	dest_vnum = ch->master->in_room->vnum;
      }
      else {
	send_to_char("Only players/pet/charmies can recall.\n\r",ch);
	return;
      }
    }
  }
  else
    if (!*argument)
      //dest_vnum = hometown_table[ch->pcdata->hometown].recall;
      location = get_recall_room(ch); // SinaC 2003
    else
      // Added/Modified by SinaC 2001
      if ( !str_cmp(argument, "clan" ) )
	if ( ch->clan )
	  //dest_vnum = get_clan_table(ch->clan)->recall;
	  location = get_room_index(get_clan_table(ch->clan)->recall);
	else {
	  send_to_char("You're not in a clan.\n\r", ch );
	  return;
	}
      else if ( !str_cmp(argument, "house" ) ) {
	send_to_char("Not Yet Implemented.\n\r", ch );
	return;
      }
      else {
	send_to_char("I don't understand that.\n\r",ch);
	return;
      }

  act( "$n prays for transportation!", ch, 0, 0, TO_ROOM );

  //if ( ( location = get_room_index( dest_vnum ) ) == NULL ) {
  //  send_to_char( "You are completely lost.\n\r", ch );
  //  return;
  //}
  if ( location == NULL ) { // SinaC 2003
    send_to_char( "You are completely lost.\n\r", ch );
    return;
  }
    
  if ( ch->in_room == location )
    return;

  // Modified by SinaC 2001    
  if ( IS_SET(ch->in_room->cstat(flags), ROOM_NO_RECALL)
       ||   IS_AFFECTED(ch, AFF_CURSE)) {
    // Modified by SinaC 2001 for god
    //send_to_char( "Mota has forsaken you.\n\r", ch );
    send_to_charf( ch, 
		   "%s has forsaken you.\n\r",
		   char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
    return;
  }
    
  if ( ( victim = ch->fighting ) != NULL ) {
    int lose,skill;
      
    skill = get_ability(ch,gsn_recall);
      
    if ( number_percent() >= 80 * skill / 100 ) {
      check_improve(ch,gsn_recall,FALSE,6);
      WAIT_STATE( ch, 4 );
      sprintf( buf, "You failed!.\n\r");
      send_to_char( buf, ch );
      return;
    }
      
    lose = (ch->desc != NULL) ? 25 : 50;
    gain_exp( ch, 0 - lose, TRUE );
    check_improve(ch,gsn_recall,TRUE,4);
    sprintf( buf, "You recall from combat!  You lose %d exps.\n\r", lose );
    send_to_char( buf, ch );
    stop_fighting( ch, TRUE );
      
  }
  
  // Added by SinaC 2001
  ROOM_INDEX_DATA * old_room = ch->in_room;

  ch->move /= 2;
  act( "$n disappears.", ch, NULL, NULL, TO_ROOM );
  char_from_room( ch );
  char_to_room( ch, location );
  act( "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );
    
  if (ch->pet != NULL) {
    sprintf(buf,"%d",dest_vnum);
    do_recall(ch->pet,buf);	
  }


  // Added by SinaC 2001 for charmies
  if ( !old_room )
    return;

  CHAR_DATA *fch, *fch_next;
  for ( fch = old_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;
    
    if ( fch->master == ch 
	 && IS_AFFECTED(fch,AFF_CHARM)
	 && fch->position < POS_STANDING)
      do_stand(fch,"");
    
    if ( fch->master == ch 
	 && fch->position == POS_STANDING
	 && IS_AFFECTED( fch, AFF_CHARM ) ) {
      sprintf(buf,"%d",dest_vnum);
      do_recall(fch,buf);
    }
  }
  
  return;
}

void do_sneak( CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA af;

  // Added by SinaC 2001
  if ( IS_AFFECTED2( ch, AFF2_FAERIE_FOG ) ) {
    send_to_char("Something prevents you from sneaking.\n\r", ch );
    return;
  }

  send_to_char( "You attempt to move silently.\n\r", ch );
  affect_strip( ch, gsn_sneak );

  // Modified by SinaC 2000
  //  to allow to sneak even if we are already sneaking due to an item
  if (IS_AFFECTED(ch,AFF_SNEAK))
    return;

  if ( number_percent() < get_ability(ch,gsn_sneak)) {
    check_improve(ch,gsn_sneak,TRUE,3);
    //afsetup(af,CHAR,affected_by,OR,AFF_SNEAK,ch->level,ch->level,gsn_sneak);
    //affect_to_char(ch,&af);
    createaff(af,ch->level,ch->level,gsn_sneak,0,AFFECT_ABILITY);
    addaff(af,CHAR,affected_by,OR,AFF_SNEAK);
    affect_to_char(ch, &af);
  }
  else
    check_improve(ch,gsn_sneak,FALSE,3);
  
  return;
}

void do_hide( CHAR_DATA *ch, const char *argument ) {
  // Added by SinaC 2001
  if ( IS_AFFECTED2( ch, AFF2_FAERIE_FOG ) ) {
    send_to_char("Something prevents you from hiding.\n\r", ch );
    return;
  }

  send_to_char( "You attempt to hide.\n\r", ch );

  if ( IS_AFFECTED(ch, AFF_HIDE) )
    REMOVE_BIT(ch->bstat(affected_by), AFF_HIDE);

  if ( number_percent( ) < get_ability(ch,gsn_hide)) {
    SET_BIT(ch->bstat(affected_by), AFF_HIDE);
    check_improve(ch,gsn_hide,TRUE,3);
  }
  else
    check_improve(ch,gsn_hide,FALSE,3);

  recompute(ch);

  return;
}

void do_pick( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *gch;
  OBJ_DATA *obj;
  int door;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Pick what?\n\r", ch );
    return;
  }
  
  WAIT_STATE( ch, BEATS(gsn_pick_lock) );

  /* look for guards */
  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    if ( IS_NPC(gch) && IS_AWAKE(gch) && ch->level + 5 < gch->level ) {
      act( "$N is standing too close to the lock.",
	   ch, NULL, gch, TO_CHAR );
      return;
    }
  }

  // Moved by SinaC 2001
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
      
      if (IS_SET(obj->value[1],EX_PICKPROOF)) {
	send_to_char("You failed.\n\r",ch);
	return;
      }

      // Modified by SinaC 2001
      int chance = get_ability(ch,gsn_pick_lock);
      if ( IS_SET(obj->value[1],EX_EASY ) )
	chance *= 2;
      if ( IS_SET(obj->value[1],EX_HARD ) )
	chance /= 2;
      // !IS_NPC removed by SinaC 2000
      if (/* !IS_NPC(ch) &&*/ number_percent( ) > chance ) {
	send_to_char( "You failed.\n\r", ch);
	check_improve(ch,gsn_pick_lock,FALSE,2);
	return;
      }
      
      REMOVE_BIT(obj->value[1],EX_LOCKED);
      act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
      act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
      check_improve(ch,gsn_pick_lock,TRUE,2);
      return;
    }
    
    /* 'pick object' */
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
    if ( !IS_SET(obj->value[1], CONT_LOCKED) ) { 
      send_to_char( "It's already unlocked.\n\r",  ch ); 
      return; 
    }
    if ( IS_SET(obj->value[1], CONT_PICKPROOF) ) { 
      send_to_char( "You failed.\n\r",             ch ); 
      return; 
    }
    
    REMOVE_BIT(obj->value[1], CONT_LOCKED);
    act("You pick the lock on $p.",ch,obj,NULL,TO_CHAR);
    act("$n picks the lock on $p.",ch,obj,NULL,TO_ROOM);
    check_improve(ch,gsn_pick_lock,TRUE,2);
    return;
  }
  
  if ( ( door = find_door( ch, arg ) ) >= 0 ) {
    /* 'pick door' */
    ROOM_INDEX_DATA *to_room;
    EXIT_DATA *pexit;
    EXIT_DATA *pexit_rev;
    
    pexit = ch->in_room->exit[door];
    if ( !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_IMMORTAL(ch)){ 
      send_to_char( "It's not closed.\n\r",        ch ); 
      return; 
    }
    if ( pexit->key < 0 && !IS_IMMORTAL(ch)){ 
      send_to_char( "It can't be picked.\n\r",     ch ); 
      return; 
    }
    // Added by SinaC 2001 for EX_INFURIATING
    if (IS_SET(pexit->exit_info, EX_INFURIATING ) ) {
      send_to_char("It can't be picked.\n\r",ch);
      return;
    }

    if ( !IS_SET(pexit->exit_info, EX_LOCKED) ){ 
      send_to_char( "It's already unlocked.\n\r",  ch ); 
      return; 
    }
    if ( IS_SET(pexit->exit_info, EX_PICKPROOF) && !IS_IMMORTAL(ch)){ 
      send_to_char( "You failed.\n\r",             ch ); 
      return; 
    }

    // Modified by SinaC 2001
    int chance = get_ability(ch,gsn_pick_lock);
    if ( IS_SET(pexit->exit_info,EX_EASY ) )
      chance *= 2;
    if ( IS_SET(pexit->exit_info,EX_HARD ) )
      chance /= 2;
    // !IS_NPC removed by SinaC 2000
    if (/* !IS_NPC(ch) &&*/ number_percent( ) > chance ) {
      send_to_char( "You failed.\n\r", ch);
      check_improve(ch,gsn_pick_lock,FALSE,2);
      return;
    }
    
    REMOVE_BIT(pexit->exit_info, EX_LOCKED);
    send_to_char( "*Click*\n\r", ch );
    act( "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM );
    check_improve(ch,gsn_pick_lock,TRUE,2);
    
    /* pick the other side */
    if ( ( to_room   = pexit->u1.to_room            ) != NULL
	 &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	 &&   pexit_rev->u1.to_room == ch->in_room ) {
      REMOVE_BIT( pexit_rev->exit_info, EX_LOCKED );
    }
  }
  
  return;
}

void do_forage(CHAR_DATA *ch,const char *argument) {
  OBJ_DATA *berry_1;
  OBJ_DATA *berry_2;
  OBJ_DATA *berry_3;
  OBJ_DATA *berry_4;

  int chance, found;

  if ( (chance = get_ability(ch,gsn_forage)) == 0 ) {
    send_to_char("You aren't able to decide on which plants are edible.\n\r",ch);
    return;
  }

  // Modified by SinaC 2001  
  if (ch->in_room->cstat(sector) != SECT_FOREST) {
    send_to_char("You aren't in a suitable forest region where you can apply your plant lore.\n\r",ch);
    return;
  }
  
  chance = UMIN( chance, 90 );
  
  if (number_percent() > chance) {
    act("$n messes about in the undergrowth but comes up looking perplexed.",ch,0,0,TO_ROOM);
    send_to_char("You search around but find nothing you can recognise as edible.\n\r",ch);
    check_improve(ch,gsn_forage,FALSE,2);
    WAIT_STATE(ch,12);
    return;
  }
  
  act("$n messes about in the nearby bushes and comes out with some berries.",ch,0,0,TO_ROOM);
  send_to_char("You search around and find some edible berries in the bushes.\n\r",ch);
  check_improve(ch,gsn_forage,TRUE,2);

  found = number_range(1,4);
  WAIT_STATE(ch, BEATS(gsn_forage) );
  check_improve(ch,gsn_forage,TRUE,2);

  berry_1 = create_object(get_obj_index(OBJ_VNUM_BERRY),1);
  if (berry_1 == NULL) {
    bug("do_forage: OBJ_VNUM_BERRY (%d) not found.", OBJ_VNUM_BERRY );
    return;
  }
  obj_to_char(berry_1,ch);
  
  if (found >= 2) {
    berry_2 = create_object(get_obj_index(OBJ_VNUM_BERRY),1);
    obj_to_char(berry_2,ch);
  }
  
  if (found >= 3) {
    berry_3 = create_object(get_obj_index(OBJ_VNUM_BERRY),1);
    obj_to_char(berry_3,ch);
  }
  if (found >= 4) {
    berry_4 = create_object(get_obj_index(OBJ_VNUM_BERRY),1);
    obj_to_char(berry_4,ch);
  }
  
  return;
}

void do_find_water(CHAR_DATA *ch,const char *argument) {
  OBJ_DATA *spring;
  int chance;

  chance = UMIN( get_ability(ch,gsn_find_water), 95 );

  if (chance <= 0 ) {
    send_to_char("You poke the ground with a stick but find no water that way.\n\r",ch);
    return;
  }
  
  if (ch->move < 15) {
    send_to_char("You don't have the move.\n\r",ch);
    return;
  }
  
  if (ch->in_room->cstat(sector) == SECT_WATER_SWIM
      || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
      || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char("Water water all around but not a drop to drink..\n\r",ch);
    return;
  }

  if (number_percent() > chance) {
    act("$n pokes the ground with a stick then scratches $s head.",ch,0,0,TO_ROOM);
    send_to_char("You poke about on the ground but fail to find any water.\n\r",ch);
    check_improve(ch,gsn_find_water,FALSE,1);
    ch->move -= 7;
    WAIT_STATE(ch,18);
    return;
  }
  act("$n pokes at the ground and digs up a spring of natural water!",ch,0,0,TO_ROOM);
  send_to_char("You poke about for a bit and eventually dig up a spring of water.\n\r",ch);
  
  WAIT_STATE(ch,BEATS(gsn_find_water));
  ch->move -= 15;
  check_improve(ch,gsn_find_water,TRUE,1);

  spring = create_object(get_obj_index(OBJ_VNUM_SPRING),0);
  obj_to_room(spring,ch->in_room);

  // Added by SinaC 2001
  recomproom(ch->in_room);

  return;
}

/* to sharpen weapons             added by Sinac 1997 */
void do_sharpen(CHAR_DATA *ch, const char *argument) {
  OBJ_DATA *obj;
  AFFECT_DATA *paf; 
  int result, fail;
  int hit_bonus, dam_bonus, added;
  bool hit_found = FALSE, dam_found = FALSE;
  int level = ch->level;
  int skill;

  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  if (arg[0] == '\0') {
    send_to_char("Sharpen what item?\n\r",ch);
    return;
  }
  
  obj =  get_obj_list(ch,arg,ch->carrying);

  if (obj== NULL) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }
  
  if ((skill = get_ability(ch,gsn_sharpen)) <= 0 ) {
    send_to_char("You don't know how to sharpen!\n\r",ch);
    return;
  }
  
  if (obj->item_type != ITEM_WEAPON) {
    send_to_char("That isn't a weapon.\n\r",ch);
    return;
  }

  if (obj->wear_loc != -1) {
    send_to_char("The item must be carried to be sharpen.\n\r",ch);
    return;
  }

  int v3 = GET_WEAPON_DAMTYPE(obj); // SinaC 2003
  if ( v3 < 0 
       || obj->value[0] == WEAPON_RANGED
       || attack_table[v3].damage == DAM_BASH ) {
    send_to_char("You can only sharpen edged weapons.\n\r",ch);
    return;
  }
  
  if ( number_percent() >= skill ) {
    act("You fail to sharpen $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_sharpen,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_sharpen));
    return;
  }
  
  /* this means they have no bonus */
  hit_bonus = 0;
  dam_bonus = 0;
  fail = 25;	/* base 25% chance of failure */

  /* find the bonuses */
  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
	if ( laf->location == ATTR_hitroll ) {
	  hit_bonus = laf->modifier;
	  hit_found = TRUE;
	  fail += 2 * (hit_bonus * hit_bonus);
	}
	else if (laf->location == ATTR_damroll ) {
	  dam_bonus = laf->modifier;
	  dam_found = TRUE;
	  fail += 2 * (dam_bonus * dam_bonus);
	}
	else  /* things get a little harder */
	  fail += 25;

  for ( paf = obj->affected; paf != NULL; paf = paf->next )
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
      if ( laf->location == ATTR_hitroll ) {
	hit_bonus = laf->modifier;
	hit_found = TRUE;
	fail += 2 * (hit_bonus * hit_bonus);
      }
      else if (laf->location == ATTR_damroll ) {
	dam_bonus = laf->modifier;
	dam_found = TRUE;
	fail += 2 * (dam_bonus * dam_bonus);
      }
      else /* things get a little harder */
	fail += 25;
  
  /* apply other modifiers */
  fail -= 3 * level/2;
  /*
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
    if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;
  */
  fail = URANGE(5,fail,95);

  result = number_percent();

  /* the moment of truth */
  if (result < (fail / 3)) { /* item destroyed */
    act("$p shatters in thousand pieces!",ch,obj,NULL,TO_CHAR);
    act("$p shatters in thousand pieces!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    check_improve(ch,gsn_sharpen,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_sharpen));
    return;
  }
  if ( result <= fail ) { /* failed, no bad result */
    act("You fail to sharpen $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_sharpen,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_sharpen));
    return;
  }
  
  affect_enchant(obj);
  
  act("You sharpen $p.",ch,obj,NULL,TO_CHAR);
  act("$n sharpens $p.",ch,obj,NULL,TO_ROOM);
  
  if (result <= (100 - level/5)) { /* success! */
    SET_OBJ_STAT(obj, ITEM_MAGIC);
    added = 1;
  }    
  else { /* exceptional enchant */
    SET_OBJ_STAT(obj,ITEM_MAGIC);
    obj->baseval[4] |= WEAPON_SHARP;
    recompobj(obj);
    added = 2;
  }
		
  /* now add the enchantments */ 

  if (obj->level < LEVEL_HERO - 1)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

//  if (dam_found) {
//    for ( paf = obj->affected; paf != NULL; paf = paf->next) {
//      if ( paf->location == ATTR_damroll) {
//	paf->type = gsn_sharpen;
//	paf->modifier += added;
//	paf->level = UMAX(paf->level,level);
//	//if (paf->modifier > 4)  SET_OBJ_STAT(obj,ITEM_HUM);
//      }
//    }
//  }
//  else {// add a new affect
//    paf = new_affect();
//    
//    afsetup(*paf,CHAR,damroll,ADD,added,-1,level,gsn_sharpen);
//    
//    paf->next	= obj->affected;
//    obj->affected	= paf;
//  }
//  
//  if (hit_found) {
//    for ( paf = obj->affected; paf != NULL; paf = paf->next) {
//      if ( paf->location == ATTR_hitroll) {
//	paf->type = gsn_sharpen;
//	paf->modifier += added;
//	paf->level = UMAX(paf->level,level);
//	//if (paf->modifier > 4) SET_OBJ_STAT(obj,ITEM_HUM);
//      }
//    }
//  }
//  else {// add a new affect
//    paf = new_affect();
//    
//    afsetup(*paf,CHAR,hitroll,ADD,added,-1,level,gsn_sharpen);
//    
//    paf->next       = obj->affected;
//    obj->affected   = paf;
//  }

  AFFECT_DATA af;
  createaff(af,-1,level,gsn_sharpen,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,added);
  addaff(af,CHAR,damroll,ADD,added);
  affect_join_obj( obj, &af );

  check_improve(ch,gsn_sharpen,TRUE,3);
  WAIT_STATE(ch,BEATS(gsn_sharpen));
  recompobj( obj );
}

void do_pillify( CHAR_DATA * ch,const char *argument ) {
  OBJ_DATA  *pill;
  char       arg [ MAX_INPUT_LENGTH ];
  int        sn;

  if(IS_NPC(ch)) {
    send_to_char("You don't have any need for pills.\n\r",ch);
    return; 
  }
  
  if(get_ability(ch,gsn_pillify) < 1) {
    send_to_char("Huh?\n\r",ch);
    return; 
  }
  
  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Make a pill out of what spell?\n\r", ch );
    return;
  }

  if ( ( sn = ability_lookup( arg ) ) < 0 ) {
    send_to_char( "You don't know any spells by that name.\n\r", ch );
    return;
  }

  if ( ability_table[sn].type != TYPE_SPELL ) {
    send_to_charf(ch,"%s is not a spell.\n\r",ability_table[sn].name);
    return;
  }
  
  if( number_percent() > get_ability(ch,sn) ) {
    send_to_char("You don't know that spell well enough to make a pill of it!\n\r",ch);
    return; 
  }

  if ( ability_table[sn].target != TAR_CHAR_DEFENSIVE
       && ability_table[sn].target != TAR_CHAR_SELF ) {
    send_to_char( "You cannot make a pill of that spell.\n\r", ch );
    return;
  }
  pill = create_object( get_obj_index( OBJ_VNUM_PILL ), 0 );
  if(!pill) {
    send_to_char("Could not find the pill object, please notify an IMP!\n\r",ch);
    return; 
  }
  if ( !IS_NPC( ch )
       && ( number_percent( ) > get_ability( ch, gsn_pillify ) ||
	    number_percent( ) > ( ( ch->cstat(INT) - 13 ) * 5 +
				  ( ch->cstat(WIS) - 13 ) * 3 ) ) ) {
    /* Modified by SinaC 2000
       act( "$p {Yexplodes {Rviolently{x!", ch, pill, NULL, TO_CHAR );
       act( "$p {Yexplodes {Rviolently{x!", ch, pill, NULL, TO_ROOM );
    */
    send_to_char("You begin focusing your magical energy.\n\r",ch);
    act( "$n begins focusing their magical energy.", ch, NULL, NULL, TO_ROOM );
    WAIT_STATE( ch, BEATS(gsn_pillify) );
    act( "$p suddenly pops into existence.",ch, pill, NULL, TO_CHAR);
    act( "$p suddenly pops into existence.",ch, pill, NULL, TO_ROOM);
    
    extract_obj( pill );
    //      damage( ch, ch, ch->cstat(max_hit) / 10, gsn_pillify, DAM_ENERGY, TRUE, FALSE );
    noaggr_damage( ch, ch->cstat(max_hit)/10, DAM_ENERGY,
		   "A pill {Yexplodes {Rviolently{x and hurt you!",
		   "A pill {Yexplodes {Rviolently{x and hurts $n!",
		   "is dead due to a missed pillify.",
		   FALSE );
    check_improve(ch,gsn_pillify,FALSE,1);
    return;
  }

  send_to_char("You begin focusing your magical energy.\n\r",ch);
  act( "$n begins focusing their magical energy.", ch, NULL, NULL, TO_ROOM );

  pill->level       = ch->level / 2;
  pill->baseval[0]  = pill->value[0]    = ch->level / 2;
  if ( !imprint_spell( sn, ch->level, ch, pill ) ) {
    return;
  }

  obj_to_char(pill,ch);
  WAIT_STATE( ch, BEATS(gsn_pillify) );
  act( "$p suddenly pops into existence.",ch, pill, NULL, TO_CHAR);
  act( "$p suddenly pops into existence.",ch, pill, NULL, TO_ROOM);


  check_improve(ch,gsn_pillify,TRUE,1);
  return;
}

void do_recite( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *scroll;
  OBJ_DATA *obj;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You do not have that scroll.\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }
  
  if ( scroll->item_type != ITEM_SCROLL
       // Added by SinaC 2003
       && scroll->item_type != ITEM_TEMPLATE ) {
    send_to_char( "You can recite only scrolls.\n\r", ch );
    return;
  }

  if ( ch->level < scroll->level) {
    send_to_char( "This scroll is too complex for you to comprehend.\n\r",ch);
    return;
  }

  obj = NULL;
  if ( arg2[0] == '\0' ) {
    victim = ch;
  }
  else {
    if ( ( victim = get_char_room ( ch, arg2 ) ) == NULL
	 &&   ( obj    = get_obj_here  ( ch, arg2 ) ) == NULL ) {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
  }
  
  act( "$n recites $p.", ch, scroll, NULL, TO_ROOM );
  act( "You recite $p.", ch, scroll, NULL, TO_CHAR );
  
  if (number_percent() >= 20 + get_ability(ch,gsn_scrolls) * 4/5) {
    send_to_char("You mispronounce a syllable.\n\r",ch);
    check_improve(ch,gsn_scrolls,FALSE,2);
  }
  else {
    obj_cast_spell( scroll->value[1], scroll->value[0], ch, victim, obj );
    obj_cast_spell( scroll->value[2], scroll->value[0], ch, victim, obj );
    obj_cast_spell( scroll->value[3], scroll->value[0], ch, victim, obj );
    check_improve(ch,gsn_scrolls,TRUE,2);
  }

  extract_obj( scroll );
  return;
}

void do_brandish( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  OBJ_DATA *staff;
  int sn;

  if ( ( staff = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
    send_to_char( "You hold nothing in your hand.\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( staff->item_type != ITEM_STAFF ) {
    send_to_char( "You can brandish only with a staff.\n\r", ch );
    return;
  }

  if ( ( sn = staff->value[3] ) < 0
       ||   sn >= MAX_ABILITY
       // Modified by SinaC 2001
       //||   ability_table[sn].spell_fun == 0 ) 
       || ability_table[sn].type != TYPE_SPELL ) {
    bug( "Do_brandish: bad sn %d.", sn );
    return;
  }
  
  WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

  if ( staff->value[2] > 0 ) {
    act( "$n brandishes $p.", ch, staff, NULL, TO_ROOM );
    act( "You brandish $p.",  ch, staff, NULL, TO_CHAR );
    if ( ch->level < staff->level 
	 ||   number_percent() >= 20 + get_ability(ch,gsn_staves) * 4/5) {
      act ("You fail to invoke $p.",ch,staff,NULL,TO_CHAR);
      act ("...and nothing happens.",ch,NULL,NULL,TO_ROOM);
      check_improve(ch,gsn_staves,FALSE,2);
    }
    else for ( vch = ch->in_room->people; vch; vch = vch_next ) {
      vch_next	= vch->next_in_room;
      
      switch ( ability_table[sn].target ) {
      default:
	bug( "Do_brandish: bad target for sn %d.", sn );
	return;
	
      case TAR_IGNORE:
	if ( vch != ch )
	  continue;
	break;
	
      case TAR_CHAR_OFFENSIVE:
	if ( IS_NPC(ch) ? IS_NPC(vch) : !IS_NPC(vch) )
	  continue;
	break;
	
      case TAR_CHAR_DEFENSIVE:
	if ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) )
	  continue;
	break;
	
      case TAR_CHAR_SELF:
	if ( vch != ch )
	  continue;
	break;
      }
      
      obj_cast_spell( staff->value[3], staff->value[0], ch, vch, NULL );
      check_improve(ch,gsn_staves,TRUE,2);
    }
  }
  
  if ( --staff->value[2] <= 0 ) {
    act( "$n's $p blazes bright and is gone.", ch, staff, NULL, TO_ROOM );
    act( "Your $p blazes bright and is gone.", ch, staff, NULL, TO_CHAR );
    extract_obj( staff );
  }
  
  return;
}

void do_zap( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *wand;
  OBJ_DATA *obj;

  one_argument( argument, arg );
  if ( arg[0] == '\0' && ch->fighting == NULL ) {
    send_to_char( "Zap whom or what?\n\r", ch );
    return;
  }
  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( ( wand = get_eq_char( ch, WEAR_HOLD ) ) == NULL ) {
    send_to_char( "You hold nothing in your hand.\n\r", ch );
    return;
  }

  if ( wand->item_type != ITEM_WAND ) {
    send_to_char( "You can zap only with a wand.\n\r", ch );
    return;
  }

  obj = NULL;
  if ( arg[0] == '\0' )  {
    if ( ch->fighting != NULL ) {
      victim = ch->fighting;
    }
    else {
      send_to_char( "Zap whom or what?\n\r", ch );
      return;
    }
  }
  else {
    if ( ( victim = get_char_room ( ch, arg ) ) == NULL
	 &&   ( obj    = get_obj_here  ( ch, arg ) ) == NULL ) {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
  }
  
  WAIT_STATE( ch, 2 * PULSE_VIOLENCE );

  if ( wand->value[2] > 0 ) {
    if ( victim != NULL ) {
      act( "$n zaps $N with $p.", ch, wand, victim, TO_NOTVICT );
      act( "You zap $N with $p.", ch, wand, victim, TO_CHAR );
      act( "$n zaps you with $p.",ch, wand, victim, TO_VICT );
    }
    else {
      act( "$n zaps $P with $p.", ch, wand, obj, TO_ROOM );
      act( "You zap $P with $p.", ch, wand, obj, TO_CHAR );
    }
    
    if (ch->level < wand->level 
	||  number_percent() >= 20 + get_ability(ch,gsn_wands) * 4/5) {
      act( "Your efforts with $p produce only smoke and sparks.",
	   ch,wand,NULL,TO_CHAR);
      act( "$n's efforts with $p produce only smoke and sparks.",
	   ch,wand,NULL,TO_ROOM);
      check_improve(ch,gsn_wands,FALSE,2);
    }
    else {
      obj_cast_spell( wand->value[3], wand->value[0], ch, victim, obj );
      check_improve(ch,gsn_wands,TRUE,2);
    }
  }
  
  if ( --wand->value[2] <= 0 ) {
    act( "$n's $p explodes into fragments.", ch, wand, NULL, TO_ROOM );
    act( "Your $p explodes into fragments.", ch, wand, NULL, TO_CHAR );
    extract_obj( wand );
  }
  
  return;
}

void do_steal( CHAR_DATA *ch, const char *argument ) {
  char buf  [MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  int percent;
  char sex[3];
  int skill;
 
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  // Added by SinaC 2000
  if ( ( skill = get_ability( ch, gsn_steal ) ) <= 0 ){
    send_to_char("You don't even know how to steal!\n\r",ch);
    return;
  }
 
  if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
    send_to_char( "Steal what from whom?\n\r", ch );
    return;
  }
  
  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  
  if ( victim == ch ) {
    send_to_char( "That's pointless.\n\r", ch );
    return;
  }
 
  /*
   *if (IS_NPC(victim))
   * {
   *	// the ShopKeepers can't be stealed
   *	//
   *	//      if ( victim->pIndexData->pShop == NULL )
   *	//        if (is_safe(ch,victim))
   *	//          return;
	
   *	if (is_safe(ch,victim))
   *  return;
   *}
   *else 
   * if (is_safe(ch,victim))
   *	return;	
   */
  // modified by SinaC 2000
  if (is_safe(ch,victim))
    return;
 
  if ( IS_NPC(victim) 
       && victim->position == POS_FIGHTING) {
    send_to_char(  "Kill stealing is not permitted.\n\r"
		   "You'd better not -- you might get hit.\n\r",ch);
    return;
  }
 
  WAIT_STATE( ch, BEATS(gsn_steal) );
  percent  = number_percent()/* - ch->level + victim->level*/;
 
  if (!IS_AWAKE(victim))
    //percent -= 10;
    percent -= 50; // I think a sleeping person can be easily stolen, SinaC 2000
  else if (!can_see(victim,ch))
    percent += 25;
  else 
    percent += 50;

  // Added by SinaC 2003 for improved steal, if steal% >= 90, we try to improve it with improved steal
  if ( skill >= 90 ) {
    if ( number_percent() <= get_ability( ch, gsn_improved_steal )/2 ) {
      check_improve(ch,gsn_improved_steal,TRUE,2);
      percent -= 20; // get 20% more to success a steal test, successed improved steal
    }
    else
      check_improve(ch,gsn_improved_steal,FALSE,2); // failed improved steal
  }

  if ( ((ch->level + 7 < victim->level || ch->level -7 > victim->level)
	&& !IS_NPC(victim) && !IS_NPC(ch) )
       // neither a PC or a NPC should be able to steal if they
       // don't know how to steal, SinaC 2000
       || ( /*!IS_NPC(ch) &&*/ percent > skill )
       //That test is useless cos' we aren't allowed to steal a PC who's not in a clan
       // SinaC 2000: is_safe  test above
       /*    || ( !IS_NPC(ch) && ch->clan && IS_NPC(victim))*/ 
       ) {
    // Failure
    send_to_char( "Oops.\n\r", ch );
    
    affect_strip(ch,gsn_sneak);
    REMOVE_BIT(ch->cstat(affected_by),AFF_SNEAK);
    recompute(ch);
    
    /* Used to test SinaC 2000
     * 	send_to_charf( ch, "percent :%d\n\r", percent );
     * 	send_to_charf( ch, "1)test : %s\n\r",
     *        ((ch->level + 7 < victim->level || ch->level -7 > victim->level) 
     *    && !IS_NPC(victim) && !IS_NPC(ch) ) ? "TRUE" : "FALSE" );
     * 	send_to_charf( ch, "2)test : %s\n\r", 
     * 		       !IS_NPC(ch) && percent > get_ability(ch,gsn_steal) ? "TRUE" : "FALSE" );
     *	send_to_charf( ch, "3)test : %s\n\r", 
     *( !IS_NPC(ch) && ch->clan && !IS_NPC(victim)) ? "TRUE": "FALSE" );
     */
    
    // Modified by SinaC 2000 if the victim can't see the robber, he can't 
    // yell the name
    if ( can_see(victim,ch)) {
      act( "$n tried to steal from you.", ch, NULL, victim, TO_VICT    );
      act( "$n tried to steal from $N.",  ch, NULL, victim, TO_NOTVICT );
      strcpy( sex, (ch->cstat(sex) == 2) ? "her" : "his");
    }
    else {
      act( "$n tried to steal from you.", ch, NULL, victim, TO_VICT    );
      act( "$n tried to steal from $N.",  ch, NULL, victim, TO_NOTVICT );
      strcpy( sex, "his");
    }
    switch(number_range(0,3)) {
    case 0 :
      // 	   sprintf( buf, "%s is a lousy thief!", ch->name );
      sprintf( buf, "%s is a lousy thief!", PERS(ch,victim) );
      break;
    case 1 :
      //   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
      //    ch->name,(ch->cstat(sex) == 2) ? "her" : "his");
      sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
	       PERS(ch,victim),sex);
      break;
    case 2 :
      // 	    sprintf( buf,"%s tried to rob me!",ch->name );
      sprintf( buf,"%s tried to rob me!",PERS(ch,victim) );
      break;
    case 3 :
      if (can_see(victim,ch))
	sprintf(buf,"Keep your hands out of there, %s!",ch->name);
      else
	sprintf(buf,"Keep your hands out of there!");
      break;
    }
    // end of modif SinaC 2000
    
    if (!IS_AWAKE(victim))
      do_wake(victim,"");
    if (IS_AWAKE(victim))
      do_yell( victim, buf );
    if ( !IS_NPC(ch) ) {
      if ( IS_NPC(victim) ) {
	check_improve(ch,gsn_steal,FALSE,2);
	multi_hit( victim, ch, TYPE_UNDEFINED );
      }
      else {
	sprintf(buf,"$N tried to steal from %s.",victim->name);
	wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
	if ( !IS_SET(ch->act, PLR_THIEF) ) {
	  SET_BIT(ch->act, PLR_THIEF);
	  send_to_char( "*** You are now a THIEF!! ***\n\r", ch );
	  //save_char_obj( ch );
	  new_save_pFile(ch,FALSE);
	}
      }
    }
    
    return;
  }
  
  if ( !str_cmp( arg1, "coin"  )
       ||   !str_cmp( arg1, "coins" )
       ||   !str_cmp( arg1, "gold"  ) 
       ||	!str_cmp( arg1, "silver")) {
    int gold, silver;
    
    gold = victim->gold * number_range(1, ch->level) / 60;
    silver = victim->silver * number_range(1,ch->level) / 60;
    if ( gold <= 0 && silver <= 0 ) {
      send_to_char( "You couldn't get any coins.\n\r", ch );
      return;
    }
    
    /*
     * Added by SinaC
     */
    gold = UMIN( gold, victim->gold );
    silver = UMIN( silver, victim->silver );
    
    ch->gold     	+= gold;
    ch->silver   	+= silver;
    victim->silver 	-= silver;
    victim->gold 	-= gold;
    
    if (silver <= 0)
      sprintf( buf, "Bingo!  You got %d gold coins.\n\r", gold );
    else if (gold <= 0)
      sprintf( buf, "Bingo!  You got %d silver coins.\n\r",silver);
    else
      sprintf(buf, "Bingo!  You got %d silver and %d gold coins.\n\r",
	      silver,gold);
    
    send_to_char( buf, ch );
    check_improve(ch,gsn_steal,TRUE,2);
    return;
  }
  
  if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL ) {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }
  /* we could steal a worn equipement only if the target is sleeping
   *  SinaC 2000
   *if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
   * {
   *   if ( ( obj = get_obj_wear( victim, arg1 ) ) == NULL )
   *	 {
   *	   send_to_char( "You can't find it.\n\r", ch );
   *	   return;
   *	 }
   *   else
   *	 {
   *	   if ( number_percent() > 50 )
   *	     {
   *	       send_to_char( "You failed to steal it.\n\r", ch );
   *	       return;
   *	     }  
   *	 } 	
   *  }
   */   
  if ( !can_drop_obj( ch, obj )
       ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
       ||   obj->level > ch->level ) {
    send_to_char( "You can't pry it away.\n\r", ch );
    return;
  }
  
  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) ) {
    send_to_char( "You have your hands full.\n\r", ch );
    return;
  }
  
  if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) ) {
    send_to_char( "You can't carry that much weight.\n\r", ch );
    return;
  }
   
  obj_from_char( obj );
  obj_to_char( obj, ch );
  obj->timer = 0;
  act("You pocket $p.",ch,obj,NULL,TO_CHAR);
  check_improve(ch,gsn_steal,TRUE,2);
  send_to_char( "Got it!\n\r", ch );
  return;
}


/* REMOVED by SinaC 2000
*void do_steal( CHAR_DATA *ch, const char *argument )
*{
*    char buf  [MAX_STRING_LENGTH];
*    char arg1 [MAX_INPUT_LENGTH];
*    char arg2 [MAX_INPUT_LENGTH];
*    CHAR_DATA *victim;
*    OBJ_DATA *obj;
*    int percent;
*
*    argument = one_argument( argument, arg1 );
*    argument = one_argument( argument, arg2 );
*
*    if ( arg1[0] == '\0' || arg2[0] == '\0' )
*    {
*	send_to_char( "Steal what from whom?\n\r", ch );
*	return;
*    }
*
*    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
*    {
*	send_to_char( "They aren't here.\n\r", ch );
*	return;
*    }
*
*    if ( victim == ch )
*    {
*	send_to_char( "That's pointless.\n\r", ch );
*	return;
*    }
*
*    if (is_safe(ch,victim))
*	return;
*
*    if ( IS_NPC(victim) 
*	  && victim->position == POS_FIGHTING)
*    {
*	send_to_char(  "Kill stealing is not permitted.\n\r"
*		       "You'd better not -- you might get hit.\n\r",ch);
*	return;
*    }
*
*    WAIT_STATE( ch, ability_table[gsn_steal].beats );
*    percent  = number_percent();
*    if (get_ability(ch,gsn_steal) >= 1)
*    percent  += ( IS_AWAKE(victim) ? 10 : -50 );
*
*    if ( ((ch->level + 7 < victim->level || ch->level -7 > victim->level) 
*    && !IS_NPC(victim) && !IS_NPC(ch) )
*    || ( !IS_NPC(ch) && percent > get_ability(ch,gsn_steal))
*    || ( !IS_NPC(ch) && !is_clan(ch) 
*	             && IS_NPC(victim))     // added by SinaC 2000
*       )
*    {
*    // Failure.
*
* 	affect_strip(ch,gsn_sneak);
* 	REMOVE_BIT(ch->bstat(affected_by),AFF_SNEAK);
* 	recompute(ch);
*
*	send_to_char( "Oops.\n\r", ch );
*
*
*	send_to_charf( ch, "percent :%d\n\r", percent );
*	send_to_charf( ch, "1)test : %s\n\r",
*        ((ch->level + 7 < victim->level || ch->level -7 > victim->level) 
*    && !IS_NPC(victim) && !IS_NPC(ch) ) ? "TRUE" : "FALSE" );
*	send_to_charf( ch, "2)test : %s\n\r", 
*		       !IS_NPC(ch) && percent > get_ability(ch,gsn_steal) ? "TRUE" : "FALSE" );
*	send_to_charf( ch, "3)test : %s\n\r", 
*( !IS_NPC(ch) && !is_clan(ch)) ? "TRUE": "FALSE" );
*
*
*	act( "$n tried to steal from you.\n\r", ch, NULL, victim, TO_VICT    );
*	act( "$n tried to steal from $N.\n\r",  ch, NULL, victim, TO_NOTVICT );
*	switch(number_range(0,3))
*	{
*	case 0 :
*	   sprintf( buf, "{z{R%s{x{R is a lousy thief!{x", ch->name );
*	   break;
*        case 1 :
*	   sprintf( buf, "%s couldn't rob %s way out of a paper bag!",
*		    ch->name,(ch->cstat(sex) == 2) ? "her" : "his");
*	   break;
*	case 2 :
*	    sprintf( buf,"{z{R%s{x{R tried to rob me!{x",ch->name );
*	    break;
*	case 3 :
*	    sprintf(buf,"{RKeep your hands out of there, {z%s{x{R!{x",ch->name);
*	    break;
*        }
*	do_yell( victim, buf );
*	if ( !IS_NPC(ch) )
*	{
*	    if ( IS_NPC(victim) )
*	    {
*	        check_improve(ch,gsn_steal,FALSE,2);
*		multi_hit( victim, ch, TYPE_UNDEFINED );
*	    }
*	    else
*	    {
*		sprintf(buf,"{R$N{x tried to steal from {B%s{x.",victim->name);
*		wiznet(buf,ch,NULL,WIZ_FLAGS,0,0);
*	    }
*	}
*
*	return;
*    }
*
*    if ( !str_cmp( arg1, "coin"  )
*    ||   !str_cmp( arg1, "coins" )
*    ||   !str_cmp( arg1, "gold"  ) 
*    ||	 !str_cmp( arg1, "silver"))
*    {
*	int gold, silver;
*
*	gold = victim->gold * number_range(1, ch->level) / 60;
*	silver = victim->silver * number_range(1,ch->level) / 60;
*	if ( gold <= 0 && silver <= 0 )
*	{
*	    send_to_char( "You couldn't get any coins.\n\r", ch );
*	    return;
*	}
*
*	ch->gold     	+= gold;
*	ch->silver   	+= silver;
*	victim->silver 	-= silver;
*	victim->gold 	-= gold;
*	if (silver <= 0)
*	    sprintf( buf, "Bingo!  You got {g%d{x gold coins.\n\r", gold );
*	else if (gold <= 0)
*	    sprintf( buf, "Bingo!  You got {g%d{x silver coins.\n\r",silver);
*	else
*	    sprintf(buf, "Bingo!  You got {g%d{x silver and {g%d{x gold coins.\n\r",
*		    silver,gold);
*
*	send_to_char( buf, ch );
*	check_improve(ch,gsn_steal,TRUE,2);
*	return;
*    }
*
*    if ( ( obj = get_obj_carry( victim, arg1, ch ) ) == NULL )
*    {
*	send_to_char( "You can't find it.\n\r", ch );
*	return;
*    }
*	
*    if ( !can_drop_obj( ch, obj )
*    ||   IS_SET(obj->extra_flags, ITEM_INVENTORY)
*    ||   obj->level > ch->level )
*    {
*	send_to_char( "You can't pry it away.\n\r", ch );
*	return;
*    }
*
*    if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) )
*    {
*	send_to_char( "You have your hands full.\n\r", ch );
*	return;
*    }
*
*    if ( ch->carry_weight + get_obj_weight( obj ) > can_carry_w( ch ) )
*    {
*	send_to_char( "You can't carry that much weight.\n\r", ch );
*	return;
*    }
*
*    obj_from_char( obj );
*    obj_to_char( obj, ch );
*    check_improve(ch,gsn_steal,TRUE,2);
*    send_to_char( "Got it!\n\r", ch );
*    return;
*}
*/

/* for poisoning weapons and food/drink */
void do_envenom(CHAR_DATA *ch, const char *argument) {
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int percent,skill;

  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  /* find out what */
  if (arg[0] == '\0') {
    send_to_char("Envenom what item?\n\r",ch);
    return;
  }

  obj =  get_obj_list(ch,arg,ch->carrying);

  if (obj== NULL) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ((skill = get_ability(ch,gsn_envenom)) < 1) {
    send_to_char("Are you crazy? You'd poison yourself!\n\r",ch);
    return;
  }

  if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) {
      act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
      return;
    }
    // modified by SinaC 2000
    if (obj->value[3]) {
      act("$p is already poisoned.",ch,obj,NULL,TO_CHAR);
      return;
    }
    if (number_percent() < skill) { /* success! */
      act("$n treats $p with deadly poison.",ch,obj,NULL,TO_ROOM);
      act("You treat $p with deadly poison.",ch,obj,NULL,TO_CHAR);
      
      obj->baseval[3] = 1;
      recompobj(obj);
      check_improve(ch,gsn_envenom,TRUE,4);
      
      WAIT_STATE(ch,ability_table[gsn_envenom].beats);
      return;
    }
    
    act("You fail to poison $p.",ch,obj,NULL,TO_CHAR);
    if (!obj->value[3])
      check_improve(ch,gsn_envenom,FALSE,4);
    WAIT_STATE(ch,BEATS(gsn_envenom));
    return;
  }
  
  if (obj->item_type == ITEM_WEAPON) {
    int v3 = GET_WEAPON_DAMTYPE(obj);
    if ( v3 < 0
	 || obj->value[0] == WEAPON_RANGED
	 || attack_table[v3].damage == DAM_BASH ) { // SinaC 2003
      send_to_char("You can only envenom edged weapons.\n\r",ch);
      return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) {
      act("You can't seem to envenom $p.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if (IS_WEAPON_STAT(obj,WEAPON_POISON)) {
      act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
      return;
    }
    
    percent = number_percent();
    if (percent < skill) {
      
      //afsetup(af,WEAPON,NA,OR,WEAPON_POISON,
      //	      ch->level/2 * percent / 100,
      //      ch->level * percent / 100,
      //      gsn_poison);
      //affect_to_obj(obj,&af);
      createaff(af,ch->level/2 * percent / 100,ch->level * percent / 100,gsn_envenom,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
      addaff(af,WEAPON,NA,OR,WEAPON_POISON);
      affect_to_obj(obj, &af);
      
      act("$n coats $p with deadly venom.",ch,obj,NULL,TO_ROOM);
      act("You coat $p with venom.",ch,obj,NULL,TO_CHAR);
      check_improve(ch,gsn_envenom,TRUE,3);
      WAIT_STATE(ch,BEATS(gsn_envenom));
      return;
    }
    else {
      act("You fail to envenom $p.",ch,obj,NULL,TO_CHAR);
      check_improve(ch,gsn_envenom,FALSE,3);
      WAIT_STATE(ch,BEATS(gsn_envenom));
      return;
    }
  }
  
  act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
  return;
}

void do_fire( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA * fire;
  int chance;

  /*    if ( (chance = get_ability(ch,gsn_fire)) == 0
	&&    ch->level < class_skilllevel(ch->classes,gsn_fire))

	Redundant test - Removed by Oxtal - in all skill procs
  */
  if ( (chance = get_ability(ch,gsn_fire)) == 0 ) {
    send_to_char("You don't know how.\n\r",ch);
    return;
  }
  
  if(get_obj_list( ch, "campfire", ch->in_room->contents )) {
    send_to_char("There is a fire here already.\n\r",ch);
    return;
  }

  if ( is_affected_room( ch->in_room, gsn_fire ) ) {
    send_to_char("There is a fire here already.\n\r",ch);
    return;
  }
  
  if(ch->move<100) {
    send_to_char("You don't have enough moves left to collect wood.\n\r",ch);
    return;
  }
  
  ch->move-=100;
  
  // Modified by SinaC 2001
  switch(ch->in_room->cstat(sector)) {
  case(SECT_INSIDE):              chance  =  0;   break;
  case(SECT_CITY):                chance  =  0;   break;
  case(SECT_FIELD):               chance +=  5;   break;
  case(SECT_FOREST):              chance += 15;   break;
  case(SECT_HILLS):                               break;
  case(SECT_MOUNTAIN):            chance -= 10;   break;
  case(SECT_WATER_SWIM):          chance  =  0;   break;
  case(SECT_WATER_NOSWIM):        chance  =  0;   break;
  case(SECT_AIR):                 chance  =  0;   break;
    // Modified by SinaC 2001, chance = 0
  case(SECT_DESERT):              chance -= 50;   break;
  case(SECT_UNDERWATER):          chance  =  0;   break;
  }
  
  if (chance == 0) {
    send_to_char("You run around searching, but can't find any wood.\n\r",ch);
    return;
  }

  if (number_percent() < chance) {
    fire = create_object( get_obj_index( OBJ_VNUM_FIRE ), 0 );
    fire->timer = 3; /* will last 3 ticks */
    obj_to_room( fire, ch->in_room );
    
    act("$n has made a campfire here!",ch,NULL,NULL,TO_ROOM);
    send_to_char("You made a campfire!\n\r",ch);
    check_improve(ch,gsn_fire,TRUE,2);
    WAIT_STATE(ch,BEATS(gsn_fire));

    // Added by SinaC 2001 so campfire is a room affect
    //  in fact, an object campfire is created with room affects on it
    AFFECT_DATA af;
    // +100% in heal/mana/psp rate
    //afsetup(af,ROOM,healrate,ADD,100,3,ch->level,gsn_fire);
    //affect_to_obj(fire,&af);
    //afsetup(af,ROOM,manarate,ADD,100,3,ch->level,gsn_fire);
    //affect_to_obj(fire,&af);
    //afsetup(af,ROOM,psprate,ADD,100,3,ch->level,gsn_fire);
    //affect_to_obj(fire,&af);
    // +1 in light
    //afsetup(af,ROOM,light,ADD,1,3,ch->level,gsn_fire);
    //affect_to_obj(fire,&af);
    createaff(af,3,ch->level,gsn_fire,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,ROOM,healrate,ADD,100);
    addaff(af,ROOM,manarate,ADD,100);
    addaff(af,ROOM,psprate,ADD,100);
    addaff(af,ROOM,light,ADD,1);
    affect_to_obj( fire, &af);
  }
  else {
    send_to_char("You run around searching, but can't find any wood.\n\r",ch);
    check_improve(ch,gsn_fire,FALSE,2);
    WAIT_STATE(ch,BEATS(gsn_fire));
  }
}

void do_lore( CHAR_DATA *ch, const char * argument ) {
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  
  if (!get_ability(ch,gsn_lore)) {
    send_to_char("Lore? You don't know how!\n\r",ch);
    return;
  }
  
  one_argument( argument, arg1 );
  if ( arg1[0] == '\0' ) {
    send_to_char( "Lore what ?\n\r", ch );
    return;
  }
  if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You aren't carrying that!\n\r", ch );
    return;
  }

  if ( IS_OBJ_STAT(obj, ITEM_NOIDENT) ) {
    send_to_char("You dont understand this objects concept.", ch );
    return;
  }
  
  WAIT_STATE(ch,BEATS(gsn_lore));
    
  if ( number_percent() < get_ability(ch,gsn_lore) / 2)
    // Modified by SinaC 2001
    spell_identify( 1, ch->level, ch, obj, 0, 1 );
  else
    send_to_char( "You failed to know something about that item.\n\r", ch );
}

// Added by SinaC 2000
void do_familiar( CHAR_DATA *ch, const char *argument) {
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *companion;
  int i, chance, clevel;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ((chance=get_ability(ch,gsn_familiar)) < 1 ){
    send_to_char("You hum a little tune and smile.\n\r",ch);
    return;
  }
  
  if ( ch->pet != NULL ){
    send_to_char("One familiar at a time is enough!\n\r",ch);
    return;
  }
  
  // Modified by SinaC 2001
  if ( number_percent() > (chance*8)/10
       // Modified by SinaC 2001
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM 
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ){
    
    act("You chant slowly, but nothing answers your call.",ch,NULL,NULL,TO_CHAR);
    act("$n chants slowly, but nature ignores the call.",ch,NULL,NULL,TO_ROOM);
    
    ch->move = (ch->move*2)/3;
    ch->mana = UMAX( ch->mana, ch->mana-30);
    
    check_improve(ch,gsn_familiar,FALSE,6);
    
    WAIT_STATE(ch,BEATS(gsn_familiar));
    
    return;
  }
  
  pMobIndex = get_mob_index( MOB_VNUM_FAMILIAR );
  
  companion = create_mobile(pMobIndex);
    
  clevel = companion->level = number_fuzzy( ch->level+1 );
  companion->hit = companion->bstat(max_hit) = number_fuzzy( clevel * 40 + 10 );
  companion->mana = companion->bstat(max_mana) = 0;
  // Added by SinaC 2001 for mental user
  companion->psp = companion->bstat(max_psp) = 0;
  companion->move = companion->bstat(max_move) = number_fuzzy( clevel * 10 + 100 );
  
  for ( i=0; i < MAX_STATS; i++)
    companion->bstat(stat0+i)=16+clevel/12;

  for(i = 0; i < 4; i++)
    companion->bstat(ac0+i) = number_fuzzy(clevel * -3 + 80 );
  
  companion->bstat(hitroll) = 
    companion->bstat(damroll) = UMAX( 1, number_fuzzy(clevel / 2) );

  companion->bstat(DICE_NUMBER)=UMAX(clevel/20,2);
  companion->bstat(DICE_TYPE)=UMAX(clevel/2,2);
  companion->bstat(DICE_BONUS)=UMAX(clevel/5,2);


  /* Choose a companion based on room sector */
  // Modified by SinaC 2001
  switch ( ch->in_room->cstat(sector) ){
  case ( SECT_INSIDE ) :
    companion->name = str_dup("giant cockroach familiar");
    companion->short_descr = str_dup("a giant cockroach");
    companion->long_descr = str_dup("A giant cockroach scuttles about here.\n\r");
    companion->description =
      str_dup("This grisly cockroach is larger than any you have ever\n\r"
	      "seen, perhaps even big enough to tear you up and digest\n\r"
	      "you in his sick little way.\n\r");
    companion->bstat(dam_type) = 22;          /* scratch */
    break;
    
  case ( SECT_CITY ) :
    companion->name = str_dup("vicious sewer rat familiar");
    companion->short_descr = str_dup("a vicious sewer rat");
    companion->long_descr = str_dup("A vicious sewer rat peers around with beady eyes.\n\r");
    companion->description =
      str_dup("This vile animal looks (and smells) like it has crawled\n\r"
	      "out of a sewer to where he stands now. Absolutely gross.\n\r");
    companion->bstat(dam_type) = 10;          /* bite */
    break;
    
  case ( SECT_FIELD ) :
    companion->name = str_dup("crafty fox familiar");
    companion->short_descr = str_dup("a crafty fox");
    companion->long_descr = str_dup("A crafty fox is sniffing about curiously.\n\r");
    companion->description =
      str_dup("This cute little fox probably isn't so cute underneath.\n\r"
	      "He has a perky red tail and slanted eyes, indicating that\n\r"
	      "he is always on the alert for prey.\n\r");
    companion->bstat(dam_type) = 7;          /* pound */
    break;
    
  case ( SECT_FOREST ) :
    companion->name = str_dup("black bear familiar");
    companion->short_descr = str_dup("a black bear");
    companion->long_descr = str_dup("A black bear lumbers about restlessly here.\n\r");
    companion->description =
      str_dup("The bear before you is a testament to nature's raw power.\n\r"
	      "It has massive limbs and a gargantuan body, and its massive\n\r"
	      "claws invoke fear in the depth of your heart.\n\r");
    companion->bstat(dam_type) = 15;          /* charge */
    break;
    
  case ( SECT_DESERT ) :
    companion->name = str_dup("giant cobra familiar");
    companion->short_descr = str_dup("a giant cobra");
    companion->long_descr = str_dup("A giant cobra coils its body, anticipating your move.\n\r");
    companion->description =
      str_dup("This scaled monstrosity reaches well over 12 feet in length,\n\r"
	      "and its muscular body ripples and shimmers in the light. It\n\r"
	      "bares its teeth at you and spits venomously.\n\r");
    companion->bstat(dam_type) = 31;          /* acidic bite */
    break;
    
    /* Removed by SinaC 2001
  case ( SECT_WATER_SWIM ) :
  case ( SECT_WATER_NOSWIM ) :
    
    companion->name = str_dup("sleek shark familiar");
    companion->short_descr = str_dup("a sleek shark");
    companion->long_descr = str_dup("A sleek shark is here, searching for a quick meal.\n\r");
    companion->description =
      str_dup("This shark swims about effortlessly, swingings its long tail\n\r"
	      "back and forth in smooth motions. It looks slender and quite\n\r"
	      "beautiful, but you know its bite is deadly.\n\r");
    companion->bstat(dam_type) = 32;          // chomp
    SET_BIT( companion->bstat(affected2_by), AFF2_WALK_ON_WATER );
    SET_BIT( companion->bstat(affected2_by), AFF2_WATER_BREATH );
    break;
    */
  case ( SECT_HILLS ) :
    companion->name = str_dup("hawk familiar");
    companion->short_descr = str_dup("a hawk");
    companion->long_descr = str_dup("A dark-eyed hawk keeps a watchful eye over the room.\n\r");
    companion->description =
      str_dup("This graceful bird is both glamorous and deadly. It has very long\n\r"
	      "wings, and short legs with deadly claws. Its sharp beak is stained\n\r"
	      "red from the flesh of recent prey.\n\r");
    companion->bstat(dam_type) = 23;          /* peck */
    // Logic isn't it ?
    SET_BIT(companion->bstat(affected_by),AFF_FLYING);
    break;
    
  case ( SECT_MOUNTAIN ) :
    companion->name = str_dup("mountain lion familiar");
    companion->short_descr = str_dup("a mountain lion");
    companion->long_descr = str_dup("A mountain lion paces slowly in circles, smelling the area.\n\r");
    companion->description =
      str_dup("This deadly cat is the epitome of speed, grace, and power. It is made\n\r"
	      "made by nature to be a killing machine, and its sharp eyes and sharper\n\r"
	      "claws serve it well to this end.\n\r");
    companion->bstat(dam_type) = 5;          /* claw */
    break;
    
  case ( SECT_AIR ) :
  default :
    companion->name = str_dup("chimera familiar");
    companion->short_descr = str_dup("a chimera");
    companion->long_descr = str_dup("A black chimera is here, silently looking around.\n\r");
    companion->description =
      str_dup("This dreadful creature looks like it could rip you limb from\n\r"
	      "limb in a heartbeat. Stand clear for your own good!\n\r");
    companion->bstat(dam_type) = 29;          /* flaming bite */
    // logic isn't it ?
    SET_BIT(companion->bstat(affected_by),AFF_FLYING);
    break;
  }
  
  act("You chant slowly, and $N answers your call!",ch,NULL,companion,TO_CHAR);
  act("$n chants slowly, and $N answers the call!",ch,NULL,companion,TO_ROOM);
  
  ch->move = (ch->move*2)/3;
  ch->mana = UMAX( ch->mana, ch->mana-30 );

  SET_BIT(companion->act, ACT_PET);
  SET_BIT(companion->act, ACT_WARRIOR);
  SET_BIT(companion->bstat(affected_by), AFF_CHARM);
  SET_BIT(companion->bstat(affected_by), AFF_HASTE);
  SET_BIT(companion->act, ACT_CREATED ); // SinaC 2003
  
  char_to_room( companion, ch->in_room );
  add_follower( companion,ch );
  companion->leader = ch;
  ch->pet = companion;
  
  //  recompute( companion ); NO NEED: done in char_to_room

  check_improve(ch,gsn_familiar,TRUE,6);
  WAIT_STATE(ch,BEATS(gsn_familiar));

  return;
}

/*
Here is the meditate command, as coded by Plasma
(morrisal@pirates.armstrong.edu or Kaneda69@hotmail.com)
for AAEKILLA (mud.usacomputers.com 4000).

All I ask is that if you use this code, keep my name and
email in the comments, and email me any bugfixes or patches
for other code bases, as I will be glad to add them.

                Thanks,
                           -Plasma

For the meditate command, which gives clerics (And other
relegion based classes (such as monk)) the ability to
regain their hit points.  At the cost of their movement
*/

#define		hpcost		2
void	do_meditate( CHAR_DATA *ch, const char *argument) {
  int	diff;/*Difference In Characters Max hp to hp*/
  int	price;/*How Much will it cost*/
  int	afford;/*If they can't do it all, how much can they afford */
  int chance;

  chance = get_ability(ch,gsn_meditate);
  if ( !IS_NPC( ch ) 
       && chance == 0 ){
    send_to_char("Go join the Church!!!\n\r", ch);
    return;
  }
  
  if (ch->position != POS_SITTING ) {
    send_to_char("You MUST sit first!\n\r", ch);
    return;
  }

  if ( chance < number_percent() ) {
    send_to_char("You lost your concentration.\n\r", ch );
    check_improve(ch,gsn_meditate,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_meditate));
    return;
  }
  
  if( ch->hit >= ch->cstat(max_hit) ) {
    send_to_char("Why bother, you're already at full health\n\r", ch);
    ch->move -= ch->move / 4;
    return;
  }
  
  diff = ch->cstat(max_hit) - ch->hit;
  price = hpcost * diff;
  
  if (ch->move >= price) {
    send_to_char("You levitate and feel fine.\n\r", ch);
    ch->hit = ch->cstat(max_hit);
    ch->move -= price;
    return;
  }
  
  afford = ( (ch->move - (ch->move % hpcost) ) / hpcost );
  price = afford * hpcost;
  
  ch->hit += afford;
  ch->move -= price;
  
  send_to_char("You levitate from the ground and feel better.\n\r", ch);
  check_improve(ch,gsn_meditate,FALSE,3);
  WAIT_STATE(ch,BEATS(gsn_meditate));
  return;

}	

void do_deathgrip( CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA af;
  int level;
  
  if ( is_affected(ch,gsn_deathgrip) ) {
    send_to_char("You already have a grip of death.\n\r",ch);
    return;
  }
  
  if ( get_ability(ch,gsn_deathgrip) < 1 ) {
    send_to_char("What's that?\n\r",ch);
    return;
  }
  
  if ( get_ability(ch,gsn_deathgrip)  <  number_percent() ) {
    send_to_char("You failed to create a grip of death.\n",ch);
    check_improve(ch,gsn_deathgrip,FALSE,1);
    WAIT_STATE(ch,BEATS(gsn_deathgrip));
    return;
  }
  
  /* Now for adding the affect to the player */
  
  level = ch->level;
  //afsetup( af, CHAR, damroll, ADD, level/7+1, 6+level, level, gsn_deathgrip);
  //affect_to_char(ch, &af);
  createaff(af,6+level,level,gsn_deathgrip,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,damroll,ADD,1+level/7);
  affect_to_char(ch, &af);
  
  act("$n's hands are shrouded with a black mist.",ch,NULL,NULL,TO_ROOM);
  send_to_char("Your hands are shrouded with a black mist.\n\r",ch);
  
  WAIT_STATE(ch,BEATS(gsn_deathgrip));
  check_improve(ch,gsn_deathgrip,TRUE,1);
}

void do_butcher( CHAR_DATA *ch, const char *argument ) {
  /* Butcher skill, created by Argawal                   */
  /* Original Idea taken fom Carrion Fields Mud          */
  /* If you have an interest in this skill, feel free    */
  /* to use it in your mud if you so desire.             */
  /* All I ask is that Argawal is credited with creating */
  /* this skill, as I wrote it from scratch.             */
  
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  OBJ_DATA *steak;
  OBJ_DATA *obj;
  int chance;
  
  one_argument(argument, arg);
  if((chance=get_ability(ch,gsn_butcher))==0) {
    send_to_char("{wButchering is beyond your skills.{x\n\r",ch);
    return;
  }

  if(arg[0]=='\0') {
    send_to_char("{wButcher what?{x\n\r",ch);
    return;
  }
  
  obj = get_obj_list( ch, arg, ch->in_room->contents );
  if ( obj == NULL ) {
    send_to_char( "{wIt's not here.{x\n\r", ch );
    return;
  }
  
  if( (obj->item_type != ITEM_CORPSE_NPC)
      && (obj->item_type!=ITEM_CORPSE_PC) ) {
    send_to_char( "{wYou can only butcher corpses.{x\n\r", ch );
    return;
  }

  /* create and rename the steak */

  steak = create_object( get_obj_index( OBJ_VNUM_STEAK ), 0 );

  steak->baseval[0]=steak->value[0] = ch->level / 2;
  steak->baseval[1]=steak->value[1] = ch->level;

  buf[0]='\0';
  strcat(buf,"A steak of ");
  strcat(buf, obj->short_descr );
  strcat(buf," is here.");
  steak->description=str_dup(buf);

  buf[0]='\0';
  strcat(buf,"A steak of ");
  strcat( buf, obj->short_descr );
  steak->short_descr=str_dup(buf);

  WAIT_STATE( ch, BEATS(gsn_butcher));

  /* Check the skill roll, and put a random amount of steaks here. */
  if(number_percent( ) < get_ability(ch,gsn_butcher)){
    obj_to_room( steak, ch->in_room );
    act( "$n butchers a corpse and creates a steak.", ch, NULL, NULL, TO_ROOM );
    act( "You butcher a corpse and create a steak.", ch, NULL, NULL, TO_CHAR );
    check_improve(ch,gsn_butcher,TRUE,1);
  }
  else {
    act( "$n fails to butcher a corpse, and destroys it.", ch, NULL, NULL, TO_ROOM );
    act( "You fail to butcher a corpse, and destroy it.", ch, NULL, NULL, TO_CHAR );
    check_improve(ch,gsn_butcher,FALSE,1);
  }
  
  /* dump items caried */
  /* Taken from the original ROM code and added into here. */
  if ( obj->item_type == ITEM_CORPSE_PC ) {   /* save the contents */
    OBJ_DATA *t_obj, *next_obj;
    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
      next_obj = t_obj->next_content;
      obj_from_obj(t_obj);
      if (obj->in_obj) /* in another object */
	obj_to_obj(t_obj,obj->in_obj);
      else if (obj->carried_by) /* carried */
	if (obj->wear_loc == WEAR_FLOAT)
	  if (obj->carried_by->in_room == NULL)
	    extract_obj(t_obj);
	  else
	    obj_to_room(t_obj,obj->carried_by->in_room);
	else
	  obj_to_char(t_obj,obj->carried_by);
      else if (obj->in_room == NULL) /* destroy it */
	extract_obj(t_obj);
      else /* to a room */
	obj_to_room(t_obj,obj->in_room);
    }
  }

  if ( obj->item_type == ITEM_CORPSE_NPC ){
    OBJ_DATA *t_obj, *next_obj;
    for (t_obj = obj->contains; t_obj != NULL; t_obj = next_obj) {
      next_obj = t_obj->next_content;
      obj_from_obj(t_obj);
      if (obj->in_obj) /* in another object */
	obj_to_obj(t_obj,obj->in_obj);
      else if (obj->carried_by) /* carried */
	if (obj->wear_loc == WEAR_FLOAT)
	  if (obj->carried_by->in_room == NULL)
	    extract_obj(t_obj);
	  else
	    obj_to_room(t_obj,obj->carried_by->in_room);
	else
	  obj_to_char(t_obj,obj->carried_by);
      else if (obj->in_room == NULL) /* destroy it */
	extract_obj(t_obj);
      else /* to a room */
	obj_to_room(t_obj,obj->in_room);
    }
  }
  
  /* Now remove the corpse */
  extract_obj(obj);

  // Added by SinaC 2001
  recomproom(ch->in_room);
  return;
}

/***************************************************************************
 *  This skill was designed by the Maniac of Mythran Mud.                  *
 *  It is copyright (C) 1995 1996 by Mark Janssen (a.k.a The Maniac).      *
 *  Some portions of this code are copied from Thelonius' poison weapon.   *
 *  please comply with the envy, diku and merc licenses.                   *
 ***************************************************************************/
/* Poison weapon by Thelonius for EnvyMud */
/* Blade thirst code is a changed version of poison weapon */
/* Written by The Maniac. This skill came from the internet book */
/* The Tome of Mighty Magic */
void do_bladethirst( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  OBJ_DATA *pobj;
  OBJ_DATA *wobj;
  AFFECT_DATA af;
  char      arg [ MAX_INPUT_LENGTH ];

  if ( get_ability( ch, gsn_bladethirst) <= 0 ) {
    send_to_char( "What do you think you are, a necromancer?\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) { 
    send_to_char( "What are you trying to do...?\n\r",    ch ); 
    return; 
  }
  if ( ch->fighting ) { 
    send_to_char( "While you're fighting?  Nice try.\n\r", ch ); 
    return; 
  }
  if ( !( obj = get_obj_carry( ch, arg, ch ) ) ) { 
    send_to_char( "You do not have that weapon.\n\r",      ch ); 
    return; 
  }
  if ( obj->item_type != ITEM_WEAPON ) { 
    send_to_char( "That item is not a weapon.\n\r",        ch ); 
    return; 
  }
  if ( obj->value[0] == WEAPON_RANGED ) {
    send_to_char("You can't do that on that kind of weapon.\n\r", ch );
    return;
  }

  if ( IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC) ) { 
    send_to_char( "That weapon is already thirsty.\n\r",  ch ); 
    return; 
  }

  /* Now we have a valid weapon...check to see if we have the bar of mithril. */
  for ( pobj = ch->carrying; pobj; pobj = pobj->next_content ){
    if ( pobj->pIndexData->vnum == OBJ_VNUM_MITHRIL )
      break;
  }
  if ( !pobj ) {
    send_to_char( "You do not have the mithril.\n\r", ch );
    return;
  }
  
  /* Okay, we have the mithril...do we have blood? */
  for ( wobj = ch->carrying; wobj; wobj = wobj->next_content ) {
    if ( wobj->item_type == ITEM_DRINK_CON
	 && wobj->value[1]  >  0
	 && wobj->value[2]  == 13 )
      break;
  }
  if ( !wobj ) {
    send_to_char( "You need some blood for this skill.\n\r", ch );
    return;
  }
  
  /* Great, we have the ingredients...but is the ch smart enough? */
  if ( !IS_NPC( ch ) 
       && ch->cstat(INT) < 17 ) {
    send_to_char( "You can't quite remember what to do...\n\r", ch );
    return;
  }
  /* And does he have steady enough hands? */
  if ( !IS_NPC( ch ) 
       && ( ch->cstat(DEX) < 17
	    || ch->pcdata->condition[COND_DRUNK] > 0 ) ) {
    send_to_char( "Your hands aren't steady enough to properly mix the ingredients.\n\r", ch );
    return;
  }

  WAIT_STATE( ch, BEATS(gsn_bladethirst));
  
  /* Check the skill percentage */
  if ( !IS_NPC( ch )
       && number_percent( ) > get_ability(ch,gsn_bladethirst) ) {
    /*
     *send_to_char( "You failed and spill some on yourself.  Ouch!\n\r",
     *	    ch );
     *damage( ch, ch, ch->level * 2, gsn_bladethirst, DAM_NEGATIVE, TRUE, FALSE );
     *act( "$n spills the blade thirst liquid all over!",ch,NULL, NULL, TO_ROOM );
     */
    noaggr_damage( ch, ch->level*2, DAM_NEGATIVE,
		   "You failed and spill some on yourself.  Ouch!",
		   "$n spills the blade thirst liquid all over!",
		   "is dead due to a missed enchantment.",
		   FALSE );
    extract_obj( pobj );
    extract_obj( wobj );
    check_improve(ch,gsn_bladethirst,FALSE,3);
    return;
  }
  
  /* Well, I'm tired of waiting.  Are you? */
  act( "You mix $p in $P, creating an evil looking potion!",
       ch, pobj, wobj, TO_CHAR );
  act( "$n mixes $p in $P, creating an evil looking potion!",
       ch, pobj, wobj, TO_ROOM );
  act( "You pour the potion over $p, which glistens wickedly!",
       ch, obj, NULL, TO_CHAR  );
  act( "$n pours the potion over $p, which glistens wickedly!",
       ch, obj, NULL, TO_ROOM  );
  //  SET_BIT( obj->extra_flags, ITEM_BLADE_THIRST );
  obj->baseval[4] |= WEAPON_VAMPIRIC; // add vampiric flag
  
  obj->cost *= ch->level;

  /* Set an object timer.  Dont want proliferation of vampiric weapons */
  obj->timer = 10 + ch->level;

  if ( IS_OBJ_STAT( obj, ITEM_BLESS ) )
    obj->timer *= 2;

  if ( IS_OBJ_STAT( obj, ITEM_MAGIC ) )
    obj->timer *= 2;

  /* WHAT?  All of that, just for that one bit?  How lame. ;) */
  act( "The remainder of the potion eats through $p.",
       ch, wobj, NULL, TO_CHAR );
  act( "The remainder of the potion eats through $p.",
       ch, wobj, NULL, TO_ROOM );
  extract_obj( pobj );
  extract_obj( wobj );

  //afsetup(af,CHAR,hitroll,ADD,3,-1,ch->level,gsn_bladethirst);
  //affect_to_obj(obj,&af);
  createaff(af,-1,ch->level,gsn_bladethirst,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,3);
  affect_to_obj(obj, &af);

  check_improve(ch,gsn_bladethirst,TRUE,3);

  return;
}

/* Replaced with a spell
*void do_barkskin( CHAR_DATA *ch,const char *argument) {
*  AFFECT_DATA af;
*  int chance;
*
*  if ( ( chance = get_ability(ch,gsn_barkskin) ) <= 0) {
*    send_to_char("You do not know how to turn your skin to bark.\n\r",ch);
*    return;
*  }
*  
*  if (is_affected(ch,gsn_barkskin) ) {
*    send_to_char("Your skin is already covered in bark.\n\r",ch);
*    return;
*  }
*
*  if (ch->mana < 40) {
*    send_to_char("You do not have the mana.\n\r",ch);
*    return;
*  }
*
*  if ( chance < number_percent() ) {
*    send_to_char("You failed to bark your skin.\n\r", ch );
*    check_improve(ch,gsn_barkskin,FALSE,3);
*    WAIT_STATE(ch,BEATS(gsn_barkskin));
*    return;
*  }
*
*
*  afsetup(af,CHAR,allAC,ADD,-20-(ch->level*2)/3,ch->level,ch->level,gsn_barkskin);
*  affect_to_char(ch,&af);
*
*  ch->mana -= 40;
*  
*  act("$n's skin slowly becomes covered in bark.",ch,NULL,NULL,TO_ROOM);
*  send_to_char("Your skin slowly becomes covered in hardened bark.\n\r",ch);
*
*  check_improve(ch,gsn_barkskin,TRUE,3);
*  WAIT_STATE(ch,BEATS(gsn_barkskin));
*  
*  return;
*}
*/

void do_warcry(CHAR_DATA *ch,const char *argument) {
  AFFECT_DATA af;
  int chance;
  if ( (chance = get_ability(ch,gsn_warcry)) <= 0 ) {
    send_to_char("You don't know how to warcry properly.\n\r",ch);
    return;
  }
  
  if (is_affected(ch,gsn_warcry)) {
    send_to_char("You are already affected by a warcry.\n\r",ch);
    return;
  }

  if (ch->mana < 20) {
    send_to_char("You don't have the mana.\n\r",ch);
    return;
  }

  if (number_percent() > chance){
    act("$n makes some soft grunting noises.",ch,0,0,TO_ROOM);
    send_to_char("You make soft grunting sounds but nothing happens.\n\r",ch);
    check_improve(ch,gsn_warcry,FALSE,2);
    ch->mana -= 10;
    WAIT_STATE( ch, BEATS(gsn_warcry) );
    return;
  }
  
  act("$n lets out a blood freezing warcry!",ch,0,0,TO_ROOM);
  send_to_char("You let out a fierce warcry!\n\r",ch);
  check_improve(ch,gsn_warcry,TRUE,2);

  //afsetup( af, CHAR, hitroll, ADD, ch->level/10, ch->level, ch->level, gsn_warcry );
  //affect_to_char( ch, &af );
  //afsetup( af, CHAR, saving_throw, ADD, -3, ch->level, ch->level, gsn_warcry );
  //affect_to_char( ch, &af );
  createaff(af,ch->level,ch->level,gsn_warcry,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,ch->level/10);
  addaff(af,CHAR,saving_throw,ADD,-3);
  affect_to_char(ch, &af);


  ch->mana -= 20;
  WAIT_STATE( ch, BEATS(gsn_warcry) );
  return;
}

void do_bandage(CHAR_DATA *ch,const char *argument) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;

  one_argument(argument,arg);

  if ( get_ability(ch,gsn_bandage) == 0) {
    send_to_char("Huh?\n\r",ch);
    return;
  }
  
  if (is_affected(ch,gsn_bandage)) {
    send_to_char("You can't apply more aid yet.\n\r",ch);
    return;
  }

  if (ch->mana < 15) {
    send_to_char("You don't have the mana.\n\r",ch);
    return;
  }

  if (arg[0] == '\0') 
    victim = ch;
  else 
    if ((victim = get_char_room(ch,arg)) == NULL) {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }
  
  if (number_percent() > get_ability( ch, gsn_bandage) ) {
    act("You fail to apply battle dressing to $N's wounds.",ch,0,victim,TO_CHAR);
    act("$n fumbles with $s bandages but fails to use them effectively.",ch,0,0,TO_ROOM);
    ch->mana -= 7;
    check_improve(ch,gsn_bandage,FALSE,3);
    return;
  }
  ch->mana -= 15;
  
  if (victim != ch) {
    act("$n applies bandages to $N's battle wounds.",ch,0,victim,TO_NOTVICT);
    act("You apply bandages to $N's battle wounds.",ch,0,victim,TO_CHAR);
    act("$n applies bandages to your battle wounds.",ch,0,victim,TO_VICT);
  }
  else {
    act("$n applies bandages to $mself.",ch,0,0,TO_ROOM);
    send_to_char("You apply battle dressing to yourself.\n\r",ch);
  }
  send_to_char("You feel better.\n\r",victim);
  
  victim->hit = UMIN(victim->hit + (3*ch->level), victim->cstat(max_hit));
  if (number_percent() < 25) {
    if (IS_AFFECTED(victim,AFF_PLAGUE)) {
      affect_strip(victim,gsn_plague);
      act("The sores on $n's body vanish.\n\r",victim,0,0,TO_ROOM);
      send_to_char("The sores on your body vanish.\n\r",victim);
    }
  }
  if (number_percent() < 25) {
    if (is_affected(victim, gsn_blindness)){
      affect_strip(victim, gsn_blindness);
      send_to_char("Your vision returns!\n\r",victim);
    }
  }
  if (number_percent() < 25) {
    if (is_affected(victim,gsn_poison)){
      affect_strip(victim,gsn_poison);
      send_to_char("A warm feeling goes through your body.\n\r",victim);
      act("$n looks better.",victim,0,0,TO_ROOM);
    }
  }
  
  check_improve(ch,gsn_bandage,TRUE,3);
  
  //afsetup( af, CHAR, NA, ADD, 0, 7, ch->level, gsn_bandage ); 
  //affect_to_char( ch, &af );
  createaff(af,7,ch->level,gsn_bandage,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char(ch, &af);
  
  return;
}

void do_herb(CHAR_DATA *ch,const char *argument) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;

  one_argument(argument,arg);

  if ( get_ability(ch,gsn_herb) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }
  if (is_affected(ch,gsn_herb)) {
    send_to_char("You can't find any more herbs.\n\r",ch);
    return;
  }

  // Modified by SinaC 2001
  if ((ch->in_room->cstat(sector) != SECT_FOREST)
      && (ch->in_room->cstat(sector) != SECT_HILLS)
      && (ch->in_room->cstat(sector) != SECT_MOUNTAIN) ) {
    send_to_char("You can't find any herbs here.\n\r",ch);
    return;
  }
  
  if (arg[0] == '\0') 
    victim = ch;
  else 
    if ((victim = get_char_room(ch,arg)) == NULL) {
      send_to_char("They aren't here.\n\r",ch);
      return;
    }

  WAIT_STATE(ch,BEATS(gsn_herb));
 
  if (number_percent() > get_ability( ch, gsn_herb ) ){
    send_to_char("You search for herbs but fail to find any.\n\r",ch);
    act("$n looks about in the bushes but finds nothing.",ch,0,0,TO_ROOM);
    check_improve(ch,gsn_herb,FALSE,4);
    return;
  }
  
  if (victim != ch) {
    act("$n applies herbs to $N.",ch,0,victim,TO_NOTVICT);
    act("You apply herbs to $N.",ch,0,victim,TO_CHAR);
    act("$n applies herbs to you.",ch,0,victim,TO_VICT);
  }
  
  if (victim == ch) {
    act("$n applies herbs to $mself.",ch,0,0,TO_ROOM);
    send_to_char("You find herbs and apply them to yourself.\n\r",ch);
  }

  send_to_char("You feel better.\n\r",victim);
    
  if (IS_AFFECTED(victim,AFF_PLAGUE) && number_percent() > 30) {
    affect_strip(victim,gsn_plague);
    act("The sores on $n's body vanish.\n\r",victim,0,0,TO_ROOM);
    send_to_char("The sores on your body vanish.\n\r",victim);
  }
  
  check_improve(ch,gsn_herb,TRUE,4);

  victim->hit = UMIN(victim->hit + 4*ch->level, victim->cstat(max_hit));

  //afsetup( af, CHAR, NA, ADD, 0, 4, ch->level, gsn_herb );
  //affect_to_char(ch,&af);
  createaff(af,4,ch->level,gsn_herb,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char(ch, &af);

  return;
}

void do_berserk( CHAR_DATA *ch, const char *argument) {
  int chance, hp_percent;

  if ( (chance = get_ability(ch,gsn_berserk)) == 0 ) {
    send_to_char("You turn red in the face, but nothing happens.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(ch,AFF_BERSERK) || is_affected(ch,gsn_berserk)
      ||  is_affected(ch,gsn_frenzy)) {
    send_to_char("You get a little madder.\n\r",ch);
    return;
  }
  
  if (IS_AFFECTED(ch,AFF_CALM)) {
    send_to_char("You're feeling to mellow to berserk.\n\r",ch);
    return;
  }
  
  if (ch->mana < 50) {
    send_to_char("You can't get up enough energy.\n\r",ch);
    return;
  }
  
  /* modifiers */

  /* fighting */
  if (ch->position == POS_FIGHTING)
    chance += 10;

  /* damage -- below 50% of hp helps, above hurts */
  hp_percent = 100 * ch->hit/ch->cstat(max_hit);
  chance += 25 - hp_percent/2;

  if (number_percent() < chance) {
    AFFECT_DATA af;
    
    WAIT_STATE(ch,PULSE_VIOLENCE);
    ch->mana -= 50;
    ch->move /= 2;
    
    /* heal a little damage */
    ch->hit += ch->level * 2;
    ch->hit = UMIN(ch->hit,ch->cstat(max_hit));
    
    send_to_char("Your pulse races as you are consumed by rage!\n\r",ch);
    act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
    check_improve(ch,gsn_berserk,TRUE,2);
    
    //afsetup(af,CHAR,affected_by,OR,AFF_BERSERK,number_fuzzy(ch->level / 8),ch->level,gsn_berserk);
    //affect_to_char(ch,&af);
    //af.op = AFOP_ADD;
    //af.modifier	= UMAX(1,ch->level/5);
    //af.location     = ATTR_hitroll;
    //affect_to_char(ch,&af);
    //af.location     = ATTR_damroll;
    //affect_to_char(ch,&af);
    //af.modifier	= UMAX(10,10 * (ch->level/5));
    //af.location     = ATTR_allAC;
    //affect_to_char(ch,&af);
    createaff(af,number_fuzzy(ch->level/8),ch->level,gsn_berserk,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,affected_by,OR,AFF_BERSERK);
    addaff(af,CHAR,hitroll,ADD,UMAX(1,ch->level/5));
    addaff(af,CHAR,damroll,ADD,UMAX(1,ch->level/5));
    addaff(af,CHAR,allAC,ADD,UMAX(10,2*ch->level));
    affect_to_char(ch, &af);
  }
  else {
    WAIT_STATE(ch,3 * PULSE_VIOLENCE);
    ch->mana -= 25;
    ch->move /= 2;
    
    send_to_char("Your pulse speeds up, but nothing happens.\n\r",ch);
    check_improve(ch,gsn_berserk,FALSE,2);
  }
}

// Added by SinaC 2001 for thief and assassin
void do_align_detect( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int chance, level;

  level = ch->level;

  one_argument( argument, arg );

  if ( arg[0] != '\0' ) {
    send_to_char("You can use that only on you!\n\r", ch );
    return;
  }

  if ( ( chance = get_ability(ch,gsn_align_detect) ) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if ( is_affected( ch, gsn_align_detect ) ) {
    send_to_char("You already are able to detect align.\n\r", ch );
    return;
  }
  
  if (number_percent() >= chance) {
    send_to_char("You failed!\n\r", ch );
    
    check_improve(ch,gsn_align_detect,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_align_detect));

    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_EVIL,level,level,gsn_align_detect);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_GOOD,level,level,gsn_align_detect);
  //affect_to_char( ch, &af );
  createaff(af,level,level,gsn_align_detect,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_GOOD);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_EVIL);
  affect_to_char(ch, &af);

  send_to_char( "Your eyes tingle.\n\r", ch );

  check_improve(ch,gsn_align_detect,TRUE,3);
  WAIT_STATE(ch,BEATS(gsn_align_detect));

  return;
}

// Added by SinaC 2001 for thief and assassin
void do_poison_detect( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  int chance;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char("Which item do you want to poison detect ?\n\r", ch );
    return;
  }

  if ( ( chance = get_ability(ch,gsn_poison_detect) ) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }

  OBJ_DATA *obj = get_obj_carry( ch, arg, ch );

  if ( obj == NULL ) {
    send_to_char("You don't have that item!\n\r", ch );
    return;
  }

  if (number_percent() >= chance) {
    send_to_char("You failed!\n\r", ch );

    check_improve(ch,gsn_poison_detect,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_poison_detect));

    return;
  }

  check_improve(ch,gsn_poison_detect,FALSE,3);
  WAIT_STATE(ch,BEATS(gsn_poison_detect));

  if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD ) {
    if ( obj->value[3] != 0 )
      send_to_char( "You smell poisonous fumes.\n\r", ch );
    else
      send_to_char( "It looks delicious.\n\r", ch );
  }
  else
    send_to_char( "It doesn't look poisoned.\n\r", ch );

  return;
}

// Added by SinaC 2001 for thief and assassin
void do_magic_detect( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int chance, level;

  level = ch->level;

  one_argument( argument, arg );

  if ( ( chance = get_ability(ch,gsn_magic_detect) ) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if ( arg[0] != '\0' ) {
    send_to_char("You can use that only on you!\n\r", ch );
    return;
  }

  if ( is_affected( ch, gsn_magic_detect ) ) {
    send_to_char("You already are able to detect magic.\n\r", ch );
    return;
  }

  if (number_percent() >= chance) {
    send_to_char("You failed!\n\r", ch );

    check_improve(ch,gsn_magic_detect,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_magic_detect));

    return;
  }

  check_improve(ch,gsn_magic_detect,FALSE,3);
  WAIT_STATE(ch,BEATS(gsn_magic_detect));

  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC,level,level,gsn_magic_detect);
  //affect_to_char( ch, &af );
  createaff(af,level,level,gsn_magic_detect,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC);
  affect_to_char(ch, &af);

  send_to_char( "Your eyes tingle.\n\r", ch );
  return;
}

// Added by SinaC 2001 for thief and assassin
void do_exits_detect( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA af;
  int chance, level;

  level = ch->level;

  one_argument( argument, arg );

  if ( ( chance = get_ability(ch,gsn_exits_detect) ) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if ( arg[0] != '\0' ) {
    send_to_char("You can use that only on you!\n\r", ch );
    return;
  }

  if ( is_affected( ch, gsn_exits_detect ) ) {
    send_to_char("You already are able to detect exits.\n\r", ch );
    return;
  }

  if (number_percent() >= chance) {
    send_to_char("You failed!\n\r", ch );

    check_improve(ch,gsn_exits_detect,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_exits_detect));

    return;
  }

  check_improve(ch,gsn_exits_detect,FALSE,3);
  WAIT_STATE(ch,BEATS(gsn_exits_detect));

  //afsetup(af,CHAR,affected2_by,OR,AFF2_DETECT_EXITS,level,level,gsn_exits_detect);
  //affect_to_char( ch, &af );
  createaff(af,level,level,gsn_exits_detect,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected2_by,OR,AFF2_DETECT_EXITS);
  affect_to_char(ch, &af);

  send_to_char( "Your eyes tingle.\n\r", ch );
  return;
}

// Added by SinaC 2003
void do_blend( CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA af;

  // Added by SinaC 2001
  if ( IS_AFFECTED2( ch, AFF2_FAERIE_FOG ) ) {
    send_to_char("Something prevents you from being blended.\n\r", ch );
    return;
  }

  if ((ch->in_room->cstat(sector) == SECT_CITY)
      || (ch->in_room->cstat(sector) == SECT_CITY)
      || (ch->in_room->cstat(sector) == SECT_WATER_SWIM)
      || (ch->in_room->cstat(sector) == SECT_WATER_NOSWIM)
      || (ch->in_room->cstat(sector) == SECT_BURNING)
      || (ch->in_room->cstat(sector) == SECT_AIR)
      || (ch->in_room->cstat(sector) == SECT_UNDERWATER)) {
    send_to_char("You can't find anyway to blend into the nature.\n\r",ch);
    return;
  }

  send_to_char( "You attempt to blend into the nature.\n\r", ch );
  affect_strip( ch, gsn_blend );

  if ( number_percent() < get_ability(ch,gsn_blend)) {
    check_improve(ch,gsn_blend,TRUE,3);
    //afsetup(af,CHAR,affected_by,OR,AFF_SNEAK,ch->level,ch->level,gsn_blend);
    //affect_to_char(ch,&af);
    createaff(af,ch->level,ch->level,gsn_blend,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,affected_by,OR,AFF_SNEAK);
    affect_to_char(ch, &af);
  }
  else
    check_improve(ch,gsn_blend,FALSE,3);
  
  return;
}

void do_appraisal( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int chance;

  one_argument( argument, arg );

  if (IS_NPC(ch))  {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  if ( ( chance = get_ability(ch,gsn_appraisal) ) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }

  if ( arg[0] == '\0' ) {
    send_to_char( "Appraise what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  int cost;
  // miss test
  if ( number_percent() < get_ability( ch, gsn_appraisal ) ) {
    cost = UMAX( 0, number_range( obj->cost/2, obj->cost*2 ) );
    check_improve(ch,gsn_appraisal,FALSE,3);
  }
  // success test
  else {
    cost = obj->cost;
    check_improve(ch,gsn_appraisal,TRUE,3);
  }

  char buf[MAX_STRING_LENGTH];
  if ( cost == 0 )
    sprintf(buf,"$p has no trade value.");
  else
    sprintf(buf,"You estimate the value of $p to %d coins.", cost);
  act( buf, ch, obj, NULL, TO_CHAR );
}

void do_purify( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  int skill;
  
  one_argument( argument, arg );
  
  if ( arg[0] == '\0' ) {
    send_to_char("Which item do you want to poison detect ?\n\r", ch );
    return;
  }

  if ( ( skill = get_ability(ch,gsn_purify) ) == 0){
    send_to_char("Huh?\n\r",ch);
    return;
  }
  
  OBJ_DATA *obj = get_obj_carry( ch, arg, ch );
  
  if ( obj == NULL ) {
    send_to_char("You don't have that item!\n\r", ch );
    return;
  }
  
  if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
    if (!obj->value[3]) {
      act("$p is not poisoned.",ch,obj,NULL,TO_CHAR);
      return;
    }
    if (number_percent() < skill) { /* success! */
      act("$n removes poison from $p.",ch,obj,NULL,TO_ROOM);
      act("You remove poison from $p.",ch,obj,NULL,TO_CHAR);
      
      obj->baseval[3] = 0;
      recompobj(obj);
      check_improve(ch,gsn_purify,TRUE,4);
      WAIT_STATE(ch,BEATS(gsn_purify));
      return;
    }
    
    act("You fail to remove poison from $p.",ch,obj,NULL,TO_CHAR);
    if (obj->value[3])
      check_improve(ch,gsn_purify,FALSE,4);
    WAIT_STATE(ch,BEATS(gsn_purify));
    return;
  }
  
  if (obj->item_type == ITEM_WEAPON) {
    if ( !IS_WEAPON_STAT(obj,WEAPON_POISON ) ) {
      act("$p is not envenomed.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if ( obj->value[0] == WEAPON_RANGED )
      return;
    
    if (number_percent() < skill) {

      REMOVE_BIT(obj->value[4], WEAPON_POISON );
      if ( IS_SET(obj->baseval[4], WEAPON_POISON ) ) // Remove definitively poison from weapon
	REMOVE_BIT(obj->baseval[4], WEAPON_POISON );

      affect_strip_obj( obj, gsn_poison ); // remove affect from poison spell
      affect_strip_obj( obj, gsn_envenom ); // remove affect from envenom skill
      
      act("$n successfully removes poison from $p.",ch,obj,NULL,TO_ROOM);
      act("You successfully remove poison from $p.",ch,obj,NULL,TO_CHAR);
      check_improve(ch,gsn_purify,TRUE,3);
      WAIT_STATE(ch,BEATS(gsn_purify));
      return;
    }

    act("You fail to purify $p.",ch,obj,NULL,TO_CHAR);
    check_improve(ch,gsn_purify,FALSE,3);
    WAIT_STATE(ch,BEATS(gsn_purify));
    return;
  }
}

void do_signal( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA * fch, * fch_next;
  char buf[MAX_STRING_LENGTH];

  if ( argument[0] == '\0' ) {
    send_to_char( "Signal what?\n\r", ch );
    return;
  }

  // Added by SinaC 2001
  if ( !IS_SET( ch->cstat(form), FORM_SENTIENT ) ) {
    log_stringf("do_signal: '%s' (%d) is not sentient", 
		NAME(ch), ch->in_room?ch->in_room->vnum:0 );
    return;
  }

  if ( number_percent() >= get_ability( ch, gsn_signal ) ) { // fail
    send_to_char("You try to signal but just manage to do nodes with fingers.\n\r",ch);
    return;
  }

  act( "{gYou signal '{x$t{g'{x", ch, argument, NULL, TO_CHAR );
  
  for ( fch = ch->in_room->people; fch != NULL; fch = fch_next ) {
    fch_next = fch->next_in_room;
    // Added by SinaC 2001 for racial language
    if ( ch != fch )
      if ( number_percent() < get_ability( fch, gsn_signal ) )
	act( "{g$n signals '{x$t{g'{x", ch, argument, fch, TO_VICT );
  }
  return;
}


void do_armslore( CHAR_DATA *ch, const char * argument ) {
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  
  if (!get_ability(ch,gsn_armslore)) {
    send_to_char("Armslore? You don't know how!\n\r",ch);
    return;
  }
  
  one_argument( argument, arg1 );
  if ( arg1[0] == '\0' ) {
    send_to_char( "Armslore what ?\n\r", ch );
    return;
  }
  if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You aren't carrying that!\n\r", ch );
    return;
  }

  if ( IS_OBJ_STAT(obj, ITEM_NOIDENT) ) {
    send_to_char("You dont understand this objects concept.", ch );
    return;
  }
  
  WAIT_STATE(ch,BEATS(gsn_armslore));
    
  if ( number_percent() < get_ability(ch,gsn_armslore) / 2 ) {
    check_improve( ch, gsn_armslore, TRUE, 4 );
    sprintf( buf, // size added by SinaC 2003
	     "Object '%s' is type %s, extra flags %s.\n\r"
	     "Weight is %d, value is %d, level is %d, condition is %s.\n\r"
	     "Material is %s",
	     obj->name,
	     item_name(obj->item_type),
	     extra_bit_name( obj->extra_flags ),
	     obj->weight / 10,
	     obj->cost,
	     obj->level,
	     show_obj_cond(obj),
	     material_table[obj->material].name );
    send_to_char( buf, ch );
    if ( obj->size != SIZE_NOSIZE ) // size is not anymore a restriction but an obj stat
      send_to_charf(ch,", size is %s", flag_string(size_flags, obj->size) );
    send_to_charf(ch,"\n\r");
    
    switch ( obj->item_type ) {
    case ITEM_SCROLL: 
    case ITEM_POTION:
    case ITEM_PILL:
    // Added by SinaC 2003
    case ITEM_TEMPLATE:
      sprintf( buf, "Level %d spells of:", obj->value[0] );
      send_to_char( buf, ch );
      
      if ( obj->value[1] > 0 && obj->value[1] < MAX_ABILITY ) {
	send_to_char( " '", ch );
	send_to_char( ability_table[obj->value[1]].name, ch );
	send_to_char( "'", ch );
      }
      
      if ( obj->value[2] > 0 && obj->value[2] < MAX_ABILITY ) {
	send_to_char( " '", ch );
	send_to_char( ability_table[obj->value[2]].name, ch );
	send_to_char( "'", ch );
      }
      
      if ( obj->value[3] > 0 && obj->value[3] < MAX_ABILITY ) {
	send_to_char( " '", ch );
	send_to_char( ability_table[obj->value[3]].name, ch );
	send_to_char( "'", ch );
      }
      
      if (obj->value[4] > 0 && obj->value[4] < MAX_ABILITY) {
	send_to_char(" '",ch);
	send_to_char(ability_table[obj->value[4]].name,ch);
	send_to_char("'",ch);
      }
      
      send_to_char( ".\n\r", ch );
      break;
      
    case ITEM_WAND: 
    case ITEM_STAFF: 
      sprintf( buf, "Has %d charges of level %d",
	       obj->value[2], obj->value[0] );
      send_to_char( buf, ch );
      
      if ( obj->value[3] >= 0 && obj->value[3] < MAX_ABILITY ) {
	send_to_char( " '", ch );
	send_to_char( ability_table[obj->value[3]].name, ch );
	send_to_char( "'", ch );
      }
      
      send_to_char( ".\n\r", ch );
      break;
      
    case ITEM_DRINK_CON:
      sprintf(buf,"It holds %s-colored %s.\n\r",
	      liq_table[obj->value[2]].liq_color,
	      liq_table[obj->value[2]].liq_name);
      send_to_char(buf,ch);
      break;
      
    case ITEM_CONTAINER:
      sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	      obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
      send_to_char(buf,ch);
      if (obj->value[4] != 100) {
	sprintf(buf,"Weight multiplier: %d%%\n\r",
		obj->value[4]);
	send_to_char(buf,ch);
      }
      break;
      
    case ITEM_WEAPON: {
      send_to_charf(ch,"Weapon type is %s.\n\r",flag_string(weapon_class,obj->value[0]));
      int v1 = GET_WEAPON_DNUMBER(obj);
      int v2 = GET_WEAPON_DTYPE(obj);
	if (obj->pIndexData->new_format)
	  sprintf(buf,"Damage is %dd%d (average %d)\n\r",
		  v1,v2,
		  ((1 + v2) * v1) / 2);
	else
	  sprintf( buf, "Damage is %d to %d (average %d)\n\r",
		   v1, v2,
		   ( v1 + v2 ) / 2 );
      send_to_char( buf, ch );
      
      // SinaC 2003
      int v4 = GET_WEAPON_FLAGS(obj);
      if ( v4 ){  /* weapon flags */
	sprintf(buf,"Weapons flags: %s\n\r",
		weapon_bit_name(v4));
	send_to_char(buf,ch);
      }
      
      if ( obj->value[0] == WEAPON_RANGED ) {
	send_to_charf(ch, "String condition: %d.\n\r", obj->value[1] );
	send_to_charf(ch, "String condition modifier probability: %d.\n\r", obj->value[2] );
	send_to_charf(ch, "Strength: %d.\n\r", obj->value[3] );
	send_to_charf(ch, "Max distance: %d.\n\r", obj->value[4] );
      }
      break;
    }
    case ITEM_ARMOR:
      sprintf( buf, 
	       "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic.\n\r",
	       obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
      send_to_char( buf, ch );
      break;
    }
  }
  else {
    send_to_char( "You failed to know something about that item.\n\r", ch );
    check_improve( ch, gsn_armslore, FALSE, 4 );
  }
}

void do_scribe( CHAR_DATA *ch, const char * argument ) {
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int percent;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players can use this skill.\n\r", ch );
    return;
  }

  if (( percent = get_ability(ch,gsn_scribe) )==0) {
    send_to_char("Scribe? You don't know how!\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  // arg1: spell name
  // arg2: parchment

  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // First, find the spell
  if ( arg1[0] == '\0' ) {
    send_to_char("Scribe which spell?\n\r", ch );
    return;
  }
  int sn = ability_lookup( arg1 );
  if ( sn <= 0 ) {
    send_to_char("This spell doesn't exist!\n\r", ch );
    return;
  }
  ability_type *sp = &(ability_table[sn]);
  if ( sp->type != TYPE_SPELL ) {
    send_to_charf(ch, "%s is not a spell.\n\r", sp->name );
    return;
  }
  int percent_spell = get_ability( ch, sn );
  if ( percent_spell == 0 ) {
    send_to_charf(ch,"You don't know '%s'!\n\r", sp->name );
    return;
  }

  // Second, find the scroll
  if ( arg2[0] == '\0' ) {
    send_to_char("Scribe on what?\n\r", ch );
    return;
  }
  OBJ_DATA *scroll;
  if ( ( scroll = get_obj_carry( ch, arg2, ch ) ) == NULL ) {
    send_to_char("You do not have that parchment.\n\r", ch );
    return;
  }
  if ( scroll->item_type != ITEM_TEMPLATE ) {
    send_to_char("You can scribe only on special scrolls.\n\r", ch );
    return;
  }
  bool used = FALSE;
  for ( int i = 0; i < 5; i++ )
    if ( scroll->baseval[i] > 0
	 || scroll->value[i] > 0 ) {
      used = TRUE;
      break;
    }
  if ( used ) {
    act("Something is already written on $p.", ch, scroll, NULL, TO_CHAR );
    return;
  }

  WAIT_STATE( ch, BEATS(gsn_scribe));
  
  // Okay, we got the parchment and the spell
  // is the spell craftable?
  if ( !sp->craftable ) {
    act("You cannot scribe that spell on enchant $p.", ch, scroll, NULL, TO_CHAR );
    return;
  }
  // scribe check failed
  if ( number_percent() >= percent ) {
    sprintf(buf,"You failed to scribe '%s' on $p and destroy it.", sp->name );
    act(buf,ch,scroll,NULL,TO_CHAR);
    act("$n tries to scribe something on $p but fail and destroy it.", ch, scroll, NULL, TO_ROOM );
    check_improve(ch,gsn_scribe,FALSE,3);
    extract_obj(scroll);
    return;
  }
  // spell check failed
  if ( number_percent() >= percent_spell ) {
    if ( percent_spell < 75 ) // if spell is below 75%, the scroll explodes
      noaggr_damage( ch, ch->level*2, DAM_FIRE,
		     "The scroll bursts into flames!",
		     "$n's scroll bursts into flames!",
		     "is dead because of a missed enchantment.",
		     FALSE );
    else {                    // else, it's just destroyed
      sprintf(buf,"You failed to scribe '%s' on $p and destroy it.", sp->name );
      act(buf,ch,scroll,NULL,TO_CHAR);
      act("$n tries to scribe something on $p but fail and destroy it.", ch, scroll, NULL, TO_ROOM );
    }
    check_improve(ch,sn,FALSE,5);
    check_improve(ch,gsn_scribe,FALSE,4);
    extract_obj(scroll);
    return;
  }
  // ch manage to scribe spell on the scroll
  sprintf(buf,"You scribe '%s' on $p.", sp->name );
  act(buf,ch,scroll,NULL,TO_CHAR);
  act("$n scribes something on $p.", ch, scroll, NULL, TO_ROOM );
  
  sprintf(buf,"A template of '%s'", sp->name );
  scroll->short_descr = str_dup( buf );

  scroll->baseval[0] = scroll->value[0] = ch->level;
  scroll->baseval[1] = scroll->value[1] = sn;
  check_improve(ch,gsn_scribe,TRUE,4);
  check_improve(ch,sn,TRUE,5);
  recompobj(scroll);
}

OBJ_DATA *find_saddle( CHAR_DATA *ch, bool equip ) {
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( ( equip || obj->wear_loc == WEAR_NONE )
	 && obj->item_type == ITEM_SADDLE )
      return obj;
  }
  return NULL;
}
void do_mount( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Only players can ride a mount.\n\r",ch);
    return;
  }
  if ( get_mount(ch) ) {
    send_to_char("You already have a mount.\n\r", ch );
    return;
  }
  CHAR_DATA *pet = ch->pet;
  int skill = get_ability( ch, gsn_riding );

  if ( skill == 0 ) {
    send_to_char("You don't know how to ride a mount.\n\r", ch );
    return;
  }

  // higher casting level gives bonus and allow to mount without saddle
  int bonus = 0;
  int casting_level = get_casting_level( ch, gsn_riding );
  if ( casting_level >= 2 )
    bonus = 25;

  // it's easier when the mob is already a pet
  if ( pet ) {
    if ( pet->in_room != ch->in_room ) {
      act("You are not in the same room as $N", ch, NULL, pet, TO_CHAR );
      return;
    }

    // find a saddle on the pet or in inventory
    bool onMount = TRUE;
    OBJ_DATA *saddle = NULL;
    if ( casting_level < 2 ) { // if casting == 1: must have a saddle, so mount is TRUE if casting >= 2
      saddle = find_saddle( pet, TRUE );
      if ( !saddle ) {
	saddle = find_saddle( ch, FALSE );
	onMount = FALSE;
      }
      // no saddle, neither on inventory neither on mount
      if ( !saddle ) {
	send_to_char("You need a saddle in order to ride.\n\r", ch );
	return;
      }
    }

    if ( !IS_SET( pet->act, ACT_MOUNTABLE ) ) {
      act("You can't ride $N.", ch, NULL, pet, TO_CHAR );
      return;
    }
    WAIT_STATE(ch,BEATS(gsn_riding));
    int chance = skill + (ch->level - pet->level)*3;
    chance -= pet->cstat(DEX)/3;
    chance += ch->cstat(DEX)/2;
    chance += bonus;
    // if mount is to high level or skill test missed or too high level diff, mount attacks player
    if ( ch->level < pet->level
	 || number_percent() > skill
	 || number_percent() > chance ) {
      check_improve(ch,gsn_riding,FALSE,3);
      act("You failed to mount $N.", ch, NULL, pet, TO_CHAR );
      act("$n tries to mount $N but failed.", ch, NULL, pet, TO_NOTVICT );
      return;
    }
    check_improve(ch,gsn_riding,TRUE,2);
    if ( onMount ) {
      act("You ride $N.", ch, NULL, pet, TO_CHAR );
      act("$n rides $N.", ch, NULL, pet, TO_NOTVICT );
      act("$n rides you.", ch, NULL, pet, TO_VICT );
    }
    else {
      act("You place your saddle on $N's back and ride it.", ch, NULL, pet, TO_CHAR );
      act("$n places $s saddle on $N's back and rides it.", ch, NULL, pet, TO_NOTVICT );
      act("$n places $s saddle on your back and rides you.", ch, NULL, pet, TO_VICT );
      obj_from_char(saddle);
      obj_to_char(saddle,pet);
    }
    SET_BIT(pet->bstat(affected_by), AFF_CHARM); SET_BIT(pet->cstat(affected_by), AFF_CHARM);
    SET_BIT(pet->act, ACT_IS_MOUNTED);
    check_mount_class( pet );
    return;
  }
  // when trying to mount a wild mob (not already a pet), we can fail resulting by being attack
  //  by the mob
  else {
    CHAR_DATA *mount;
    char arg[MAX_INPUT_LENGTH];
    one_argument( argument, arg );
    if ( arg[0] == '\0' ) {
      send_to_char("Mount whom?\n\r", ch );
      return;
    }
    mount = get_char_room( ch, arg );
    if ( !mount ) {
      send_to_char("They aren't here.\n\r", ch );
      return;
    }
    // find a saddle on the mount or in inventory
    bool onMount = TRUE;
    OBJ_DATA *saddle = NULL;
    if ( casting_level < 2 ) { // if casting == 1: must have a saddle, so mount is TRUE if casting >= 2
      saddle = find_saddle( mount, TRUE );
      if ( !saddle ) {
	saddle = find_saddle( ch, FALSE );
	onMount = FALSE;
      }
      // no saddle, neither on inventory neither on mount
      if ( !saddle ) {
	send_to_char("You need a saddle in order to ride.\n\r", ch );
	return;
      }
    }
    if ( !IS_SET( mount->act, ACT_MOUNTABLE )
	 || IS_AFFECTED( mount, AFF_CHARM )
	 || mount->master != NULL ) {
      act("You can't ride $N.", ch, NULL, mount, TO_CHAR );
      return;
    }
    WAIT_STATE(ch,BEATS(gsn_riding));
    int chance = skill + (ch->level - mount->level)*3;
    chance -= mount->cstat(DEX)/3;
    chance += ch->cstat(DEX)/2;
    chance += bonus;
    // if mount is to high level or skill test missed or too high level diff, mount attacks player
    if ( ch->level < mount->level
	 || number_percent() > skill
	 || number_percent() > chance ) {
      check_improve(ch,gsn_riding,FALSE,3);
      act("You failed to mount $N.", ch, NULL, mount, TO_CHAR );
      act("$n tries to mount $N but failed.", ch, NULL, mount, TO_NOTVICT );
      multi_hit(mount,ch,TYPE_UNDEFINED);
      return;
    }
    else {
      check_improve(ch,gsn_riding,TRUE,2);
      SET_BIT(mount->act, ACT_PET);
      SET_BIT(mount->bstat(affected_by), AFF_CHARM); SET_BIT(mount->cstat(affected_by), AFF_CHARM);
      SET_BIT(mount->act, ACT_IS_MOUNTED);
      if ( onMount ) {
	act("You ride $N.", ch, NULL, mount, TO_CHAR );
	act("$n rides $N.", ch, NULL, mount, TO_NOTVICT );
	act("$n rides you.", ch, NULL, mount, TO_VICT );
      }
      else {
	act("You place your saddle on $N's back and ride it.", ch, NULL, mount, TO_CHAR );
	act("$n places $s saddle on $N's back and ride it.", ch, NULL, mount, TO_NOTVICT );
	act("$n places $s saddle on your back and ride you.", ch, NULL, mount, TO_VICT );
	obj_from_char(saddle);
	obj_to_char(saddle,mount);
      }
      add_follower( mount, ch );
      mount->leader = ch;
      ch->pet = mount;
      check_mount_class( mount );
      return;
    }
  }
}

void do_dismount( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Only players can ride a mount and so dismount.\n\r",ch);
    return;
  }
  CHAR_DATA *mount = get_mount( ch );
  if ( !mount ) {
    send_to_char("You are not riding any mount.\n\r", ch );
    return;
  }

  if ( mount->in_room != ch->in_room ) {
    send_to_char("You need to be in the same room as your mount in order to dismount it.\n\r", ch );
    return;
  }

  int skill = get_ability( ch, gsn_riding );
  WAIT_STATE(ch,BEATS(gsn_riding));
  if ( number_percent() > skill ) {
    check_improve(ch, gsn_riding, FALSE, 2 );
    act("You failed to dismount $N.", ch, NULL, mount, TO_CHAR );
    return;
  }
  check_improve(ch, gsn_riding, TRUE, 2 );
  REMOVE_BIT(mount->act,ACT_IS_MOUNTED);
  act("You successfully dismount $N.", ch, NULL, mount, TO_CHAR );

  // find a saddle
  OBJ_DATA *saddle = NULL;
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj->wear_loc == WEAR_NONE
	 && obj->item_type == ITEM_SADDLE )
      saddle = obj;
  }
  if ( saddle ) {
    send_to_char("You get your saddle back.\n\r", ch );
    obj_from_char(saddle);
    obj_to_char(saddle,ch);
  }
  //  remove_mount_class( mount );
}

void do_bond( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Only players can bond to their mount.\n\r",ch);
    return;
  }

  int skill = get_ability( ch, gsn_bond );
  if ( skill == 0 ) {
    send_to_char("You don't know how to be bond with a mount.\n\r", ch );
    return;
  }

  // Find mount
  CHAR_DATA *mount = get_mount( ch );
  if ( !mount ) {
    send_to_char("You are not riding any mount.\n\r", ch );
    return;
  }
  if ( mount->in_room != ch->in_room ) {
    send_to_char("You need to be in the same room as your mount in order to bond it.\n\r", ch );
    return;
  }

  skill = URANGE( 5, skill, 95 );
  if ( number_percent() < skill ) { // success
    act("You create a powerful bond between you and $N.", ch, NULL, mount, TO_CHAR );
    act("$n creates a powerful bond between you and $m.", ch, NULL, mount, TO_VICT );
    act("$n creates a bond between $m and $N.", ch, NULL, mount, TO_NOTVICT );
    check_improve(ch,gsn_bond,TRUE,3);

    AFFECT_DATA af;
    // Increase stats
    int midLevel = (ch->level+mount->level)/2;
    int mod = UMAX( midLevel/12, 2 );
    //afsetup( af, CHAR, STR, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond);
    //affect_to_char(ch, &af);
    //affect_to_char(mount, &af);
    //afsetup( af, CHAR, INT, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond);
    //affect_to_char(ch, &af);
    //affect_to_char(mount, &af);
    //afsetup( af, CHAR, WIS, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond);
    //affect_to_char(ch, &af);
    //affect_to_char(mount, &af);
    //afsetup( af, CHAR, DEX, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond);
    //affect_to_char(ch, &af);
    //affect_to_char(mount, &af);
    //afsetup( af, CHAR, CON, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond);
    //affect_to_char(ch, &af);
    //affect_to_char(mount, &af);
    createaff(af,-1,midLevel,gsn_bond,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT|AFFECT_STAY_DEATH);
    addaff(af,CHAR,STR,ADD,mod);
    addaff(af,CHAR,INT,ADD,mod);
    addaff(af,CHAR,WIS,ADD,mod);
    addaff(af,CHAR,DEX,ADD,mod);
    addaff(af,CHAR,CON,ADD,mod);
    
    // Increase saving throw
    mod = -UMAX( midLevel/5, 5 );
    //afsetup( af, CHAR, saving_throw, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond );
    //affect_to_char(ch, &af );
    //affect_to_char(mount, &af );
    addaff(af,CHAR,saving_throw,ADD,mod);

    // Increase Hit, Mana, Move
    mod = midLevel*15;
    //afsetup( af, CHAR, max_hit, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond );
    //affect_to_char(ch, &af );
    //affect_to_char(mount, &af );
    addaff(af,CHAR,max_hit,ADD,mod);
    mod = midLevel*8;
    //afsetup( af, CHAR, max_mana, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond );
    //affect_to_char(ch, &af );
    //affect_to_char(mount, &af );
    addaff(af,CHAR,max_mana,ADD,mod);
    addaff(af,CHAR,max_psp,ADD,mod);
    mod = midLevel*5;
    //afsetup( af, CHAR, max_move, ADD, mod, DURATION_PERMANENT, midLevel, gsn_bond );
    //affect_to_char(ch, &af );
    //affect_to_char(mount, &af );
    addaff(af,CHAR,max_move,ADD,mod);

    affect_to_char(ch, &af);
    affect_to_char(mount, &af);

    return;
  }
  else {
    act("You fail binding with $N, your feel drained.", ch, NULL, mount, TO_CHAR );
    act("$n fails binding with $N and feels drained.", ch, NULL, mount, TO_CHAR );
    ch->psp = ch->move = 0; ch->mana = 0; // really nasty draining effect
    check_improve(ch,gsn_bond,FALSE,3);
    return;
  }
}

void do_study( CHAR_DATA *ch, const char * argument ) {
  char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int percent;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players can use this skill.\n\r", ch );
    return;
  }

  if (( percent = get_ability(ch,gsn_study) )==0) {
    send_to_char("Study? You don't know how!\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  // arg1: parchment
  // arg2: #trains

  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  // First, find the scroll and the spell
  if ( arg1[0] == '\0' ) {
    send_to_char("Study which parchment?\n\r", ch );
    return;
  }
  OBJ_DATA *scroll;
  if ( ( scroll = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char("You do not have that parchment.\n\r", ch );
    return;
  }
  if ( scroll->item_type != ITEM_TEMPLATE ) {
    send_to_char("You can study only from special scrolls.\n\r", ch );
    return;
  }
  int sn = scroll->value[1];
  if ( sn <= 0 || sn >= MAX_ABILITY ) {
    send_to_char("Nothing is written on that parchment.\n\r", ch );
    return;
  }
  ability_type *sk = &(ability_table[sn]);
  if ( sk->type != TYPE_SPELL ) {
    send_to_char("It crumbles into dust.\n\r", ch );
    bug("do_study: parchment %s [%d] with %s", 
	scroll->short_descr, scroll->pIndexData->vnum, sk->name );
    extract_obj(scroll);
    return;
  }
  if ( get_ability( ch, sn ) ) {
    send_to_charf(ch,"You already know '%s'.\n\r", sk->name);
    return;
  }
  if ( check_gain_prereq( ch, sn ) != 0 ) {
    //send_to_charf( ch, "You don't fit the prereq to study '%s'.\n\r", sk->name );
    send_to_charf(ch, "You don't fit prerequisites or are not high level enough or can't study '%s'.\n\r", sk->name );
    return;
  }

  // Second, get the trains
  if ( arg2[0] == '\0' || !is_number(arg2) ) {
    send_to_char("You must specify an amount of trains.\n\r", ch );
    return;
  }
  int train = atoi(arg2);
  if ( train <= 0 || train > 10 ) {
    send_to_char("The amount of trains must be between 1 and 10.\n\r", ch );
    return;
  }
  if ( ch->train < train ) {
    send_to_char("You don't have that many trains.\n\r", ch );
    return;
  }

  // Now we have the spell and the amount of trains
  WAIT_STATE( ch, BEATS(gsn_study));
  // Skill test failed
  if ( number_percent() >= percent ) {
    act("You failed to study '$T', $p is destroyed.",ch,scroll,sk->name,TO_CHAR);
    extract_obj(scroll);
    return;
  }
  // Skill test succeed
  // Now we check if the player successfully learn the spell
  //  5% for each train dedicated, a bonus if player is a specialized class
  int bonus = 0;
  // Failed to learn the spell ... ouch: lose the trains
  if ( number_percent() >= train*5+bonus ) {
    check_improve(ch,gsn_study,FALSE,4);
    act("You failed to study '$T', $p is destroyed.",ch,scroll,sk->name,TO_CHAR);
    extract_obj(scroll);
    ch->train -= train;
    return;
  }
  // Okay, ch has successfully learned the spell
  check_improve(ch,gsn_study,TRUE,4);
  act("You successfully learn '$T' from $p.",ch,scroll,sk->name,TO_CHAR);
  act("$n successfully learns '$T' from $p.",ch,scroll,sk->name,TO_ROOM);
  extract_obj(scroll);
  ch->train -= train;
  ch->pcdata->ability_info[sn].learned = 1;
  ch->pcdata->ability_info[sn].casting_level = ability_table[sn].nb_casting_level > 0 ? 1 : 0;
  ch->pcdata->ability_info[sn].level = URANGE( 1, ch->level, 100 );
}

/*
Syntax: disguise <good/neutral/evil>
Syntax: disguise <lawful/neutral2/chaotic>
Syntax: disguise <race>
Syntax: disguise none

DISGUISE is a skill by which a character can hide their true race and/or
alignment from others.  One can disguise their good/evil alignment as well
as their lawful/chaotic alignment.  Note that neutral2 is used for
neutral as to law/chaos.  Disguise none will cancel all disguises.

When disguising one's race, one may only change into a race that players
are allowed to choose, including races only available through remort.
However, undead (rebirth) races are only available as a disguise for other
undead, and Ancients are never available as a disguise.  In addition, one
may only disguise themselves as a race of equivalent size to their own.

Disguise is useful in fooling creatures into believing you are someone
that you are not.

Disguising oneself takes a lot of energy, and therefore uses half of
one's movement per attempt.
*/
void do_disguise( CHAR_DATA *ch, const char *argument ) {
  send_to_char("Not yet implemented.\n\r", ch );
  return;
}

/*
Syntax: counterfeit <object>

COUNTERFEIT allows a character to change the appearance of an object making
it appear to be more valuable than it actually is.  In addition, counterfeit
allows a character to attempt to pass off this increased value to a
shopkeeper.  Most shopkeepers are astute at looking for counterfeit goods;
therefore, characters unskilled in counterfeit that attempt to sell
counterfeit goods to a shopkeeper are often caught.

The counterfeit skill will increase the apparent value of an object up to
three times its original value.  An object can only be counterfeited once.
Objects without any value cannot have their value increased.

A character that is caught attempting to sell counterfeit goods will
generally be banned from a shop in the same manner that a character
insulting a shopkeeper with a haggle offer will (see 'help haggle').  Note
that selling counterfeit goods requires a skill check when the goods are
sold as well as when the counterfeiting is done, so even a learned
counterfeiter may on occasion be caught.
*/
void do_counterfeit( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  int percent, skill;

  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  // find out what
  if (arg[0] == '\0') {
    send_to_char("Counterfeit what item?\n\r",ch);
    return;
  }

  obj =  get_obj_list(ch,arg,ch->carrying);

  if (obj== NULL) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if ((skill = get_ability(ch,gsn_counterfeit)) <= 0) {
    send_to_char("You don't know how to counterfeit!\n\r",ch);
    return;
  }

  // Check if item has already been counterfeit
  if ( get_extra_field( obj, "counterfeit_value" ) != NULL ) { // extra field found
    act("$p has already been counterfeit.", ch, obj, NULL, TO_CHAR );
    return;
  }

  // Can't counterfeit a worthless item
  if ( obj->cost == 0 ) {
    act("$p is worthless, you can't counterfeit it.", ch, obj, NULL, TO_CHAR );
    return;
  }

  // Store old cost in an extra field
  Value v = add_extra_field( obj, "counterfeit_value" );
  v.setValue( obj->cost );
  // New cost: from cost+1 to cost*3
  obj->cost = number_range( obj->cost+1, UMAX( obj->cost+5, (obj->cost*skill)/33) );
  act("You counterfeit $p.", ch, obj, NULL, TO_CHAR );
  return;
}

void do_favored_enemy( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  int race;

  argument = one_argument(argument,arg);

  if ( get_ability( ch, gsn_favored_enemy ) == 0 ) {
    send_to_char("You don't know how to choose a favored enemy.\n\r", ch );
    return;
  }
  Value *sp = get_extra_field( ch, "favored_enemy" );
  if ( sp != NULL ) {
    char w[MAX_INPUT_LENGTH];
    strcpy( w, sp->asStr() );
    if ( w != NULL && w[0] != '\0' )
      send_to_charf(ch,"%s is already your favored enemy.\n\r", w );
    return;
  }
  if ( ( race = race_lookup( arg, TRUE ) ) < 0 ) {
    send_to_charf(ch,"%s is not an available race.\n\r", arg );
    return;
  }
  if ( race == ch->bstat(race) ) {
    send_to_char("You cannot choose your own race.\n\r", ch );
    return;
  }

  // Add an extra field storing in which weapon the player is specialized
  Value v = add_extra_field( ch, "favored_enemy" );
  v.setValue( race_table[race].name );
  send_to_charf(ch,"%s is now your favored enemy.\n\r", race_table[race].name );
  return;
}

void do_climbing( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );
  if ( arg == NULL || arg[0] == '\0' ) {
    send_to_char("Climb in which direction?\n\r", ch );
    return;
  }

  int door = find_exit( arg );
  if ( door == -1 ) {
    send_to_char("This exit doesn't exist.\n\r", ch );
    return;
  }

  if ( ch->in_room == NULL ) {
    send_to_char("How did you do that?!?\n\r", ch );
    bug("do_climbing: %s is not in a room.", NAME(ch) );
    return;
  }

  EXIT_DATA *pExit = ch->in_room->exit[door];
  if ( pExit == NULL ) {
    send_to_char("This exit doesn't exist.\n\r", ch );
    return;
  }

  if ( !IS_SET( pExit->exit_info, EX_CLIMB ) ) {
    act("You start to climb to the $t but realize there is not need to climb.",
	ch, dir_name[door], NULL, TO_CHAR );
    act("$n starts to climb to the $t but realizes there is not need to climb.",
	ch, dir_name[door], NULL, TO_ROOM );
    move_char( ch, door, FALSE, FALSE, FALSE ); // SinaC 2003 ); // move the player even if climbing was not needed
    return;
  }
  CHAR_DATA *mount = get_mount( ch );
  // if mount is not in the same room, we do as player doesn't have a mount
  if ( mount && mount->in_room != ch->in_room )
    mount = NULL;
  if ( mount ) {
    send_to_char("You can't climb when mounting.\n\r", ch );
    return;
  }

  // Climbing skill or a rope is needed to enter CLIMB exits
  int chance = get_ability( ch, gsn_climbing );
  OBJ_DATA *rope = get_item_here( ch, ITEM_ROPE );
  if ( rope )
    chance += 50; // bonus if there is a rope
  else if ( ch->cstat(DEX) < 20 ) // if no high dex, 
    chance -= ( 20 - ch->cstat(DEX)) * 3;
  chance = URANGE( 2, chance+ch->cstat(DEX)-20, 97 );
  WAIT_STATE(ch,BEATS(gsn_climbing));
  if ( number_percent() > chance ) {
    check_improve(ch,gsn_climbing,FALSE,4);
    if ( rope ) {
      act("You start to climb on $p, lose a grip and fall!", ch, rope, NULL, TO_CHAR );
      act("$n starts to climb on $p, loses a grip and falls!", ch, rope, NULL, TO_ROOM );
      noaggr_damage( ch, 150, DAM_BASH,
		     "", "", "has done a deadly fall.",
		     FALSE );
    }
    else
      noaggr_damage( ch, 150, DAM_BASH,
		     "You start to climb, lose a grip and fall!",
		     "$n starts to climb, loses a grip and falls!",
		     "has done a deadly fall.",
		     FALSE );
    return;
  }

  if ( rope ) {
    act("You start to climb on $p.", ch, rope, NULL, TO_CHAR );
    act("$n starts to climb on $p.", ch, rope, NULL, TO_ROOM );
  }
  else {
    act("You start to climb.", ch, NULL, NULL, TO_CHAR );
    act("$n starts to climb.", ch, NULL, NULL, TO_ROOM );
  }
  check_improve(ch,gsn_climbing,TRUE,4);
  move_char( ch, door, FALSE, TRUE, FALSE ); // SinaC 2003 );

  return;
}

//'SHAPECHANGE' 'SHAPECHANGE 1' 'SHAPECHANGE 2' 'SHAPECHANGE 3'
//Syntax: shapechange wolfs
//Syntax: shapechange bear
//Syntax: shapechange <race>
//Syntax: shapechange none
//
//The ability to shapechange into various forms is one of the most unique
//skills available to the druid.  There are three levels of this skill.
//By purchasing any one of the three levels, the druid gains the ability to
//shapechange into a wolf.  Upon purchase of two levels of this skill, the
//druid can choose between wolf form and bear form.  Upon purchase of all
//three levels, the druid can shapechange into any race listed in 'help
//shapechange races'.  In addition, upon purchase of all three levels of
//this skill, the druid's shapechange no longer has a duration.  Instead, it
//will last until canceled via the 'shapechange none' command or upon
//death.
//
//'Shapechange none' will revert the druid to his or her natural form.
void do_shapechange( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_charf(ch,"You must specify a race. Check help about shapechange.\n\r" );
    return;
  }
  int casting_level = get_casting_level( ch, gsn_shapechange );
  int chance = get_ability( ch, gsn_shapechange );
  if ( chance == 0 ) {
    send_to_charf(ch,"You don't know how to shapechange.\n\r");
    return;
  }
  int race;
  int which = -1;  // wolf: 1      bear: 2     other: 3      none: -1
  if ( !str_cmp( arg, "wolf" ) ) {
    race = race_lookup("wolf", TRUE );
    which = 1; // at least casting lvl 1
  }
  else if ( !str_cmp( arg, "bear" ) ) {
    race = race_lookup("bear", TRUE);
    which = 2; // at least casting lvl 2
  }
  else if ( !str_cmp( arg, "none" ) ) {
    race = -1;
    which = -1;
  }
  else if ( !str_prefix( arg, "unique" ) ) { // Unique is not a valid race
    race = -1;
    which = 3;
  }
  else {
    race = race_lookup(arg, TRUE);
    which = 3; // at least casting lvl 3
  }

  if ( is_affected( ch, gsn_shapechange ) ) { // already affected
    if ( which == -1 ) { // none  specified: remove affect
      send_to_charf(ch,"You regain your original form.\n\r");
      affect_strip( ch, gsn_shapechange );
      return;
    }
    else { // must use  none  before shapechanging into a new race
      send_to_charf(ch,"You are already shapechanged.\n\r");
      return;
    }
  }
  if ( which == -1 ) { // none  specified  but not shapechanged
    send_to_charf(ch,"You are not shapechanged.\n\r");
    return;
  }
  if ( race < 0 ) { // Unknown race
    send_to_charf(ch,"This race is not available.\n\r");
    return;
  }
  if ( race_table[race].pc_race ) { // Only in NPC race
    send_to_charf(ch,"This race is not available.\n\r");
    return;
  }
  if ( casting_level < which ) { // Casting level not high enough
    send_to_charf(ch,"You are not master enough in shapechanging for this race.\n\r");
    return;
  }
  // Okay, wa can now shapechange
  change_race( ch, race, -1, ch->level, gsn_shapechange, casting_level );

  const char *s = race_table[race].name;
  char *art;
  if ( s[0] == 'a' 
       || s[0] == 'e' 
       || s[0] == 'i' 
       || s[0] == 'o' 
       || s[0] == 'u'
       || s[0] == 'y' )
    art = "an";
  else
    art = "a";
  send_to_charf(ch, "You shapechange yourself into %s %s.\n\r",
		art, s);
  char buf[MAX_STRING_LENGTH];
  sprintf( buf, "$n starts to shapechange $mself into %s %s.", art, s );
  act( buf, ch, NULL, NULL, TO_ROOM );
  return;
}


void do_fletcher( CHAR_DATA *ch, const char *argument ) {
//FLETCH FLETCHER
//A character skilled as a FLETCHER has the ability to make arrows.  A
//fletcher will always have or be able to quickly find the raw materials
//necessary to make an arrow: a stick of wood, a piece of flint, some twine,
//and a feather or leaf.  Due to the common nature of these items, fletchers
//are presumed to always have them in their possession.
//See also: help archery
  OBJ_INDEX_DATA *objIndex = get_obj_index(OBJ_VNUM_ARROW);
  if ( objIndex == NULL ) {
    bug("OBJ_VNUM_ARROW (%d) not found.", OBJ_VNUM_ARROW );
    send_to_char("Problem...warn an immortal.\n\r", ch );
    return;
  }

  int percent = get_ability( ch, gsn_fletcher );
  if ( percent == 0 ) {
    send_to_char("You don't know how to make arrows.\n\r", ch );
    return;
  }
  WAIT_STATE( ch, BEATS(gsn_fletcher) );
  if ( number_percent() > percent ) {
    send_to_char("You failed to make an arrow.\n\r", ch );
    return;
  }
  OBJ_DATA *obj = create_object( objIndex, ch->level );
  obj->level = URANGE( 1, number_fuzzy( ch->level-2 ), LEVEL_HERO );
  obj->baseval[1] = UMAX( number_range( obj->level/25, obj->level/15 ), 1 );
  obj->baseval[2] = UMAX( number_range( obj->level/20, obj->level/10 ), 1 );
  send_to_char("You have made an arrow.\n\r", ch );
  act("$n has made an arrow.", ch, NULL, NULL, TO_ROOM );
  obj_to_char( obj, ch );
  return;
}

void do_restring( CHAR_DATA *ch, const char *argument ) {
//BOWYER RESTRING STRENGTHEN
//A character skilled as a bowyer is a master of the craft of improving and
//fixing bows.  Proficiency in this skill gives a character access to two
//separate commands: RESTRING and STRENGTHEN.
//RESTRING allows a character to repair frayed or broken bow strings.  A
//character skilled as a bowyer is presumed to always have the materials and
//tools necessary to replace the bow's string on him or her at all times.
//Restring will not work on a bow with a level higher than the character's,
//and may cause an already frayed bowstring to snap.
//See also: help archery
  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char("Restring what?\n\r", ch );
    return;
  }

  OBJ_DATA *bow = get_obj_carry( ch, arg, ch );
  if ( bow == NULL ) { // no obj 'argument' found
    send_to_char("You can't find it.\n\r", ch );
    return;
  }
  if ( bow->item_type != ITEM_WEAPON
       || bow->value[0] != WEAPON_RANGED ) {
    act("$p is not a ranged weapon.", ch, bow, NULL, TO_CHAR );
    return;
  }

  if ( bow->condition <= 0  ) {
    act("$p is in too bad condition to be restring.", ch, bow, NULL, TO_CHAR );
    return;
  }

  if ( bow->cRangedStringCondition >= MAX_REPAIRABLE_STRING_CONDITION ) { // too good condition
    act("The string of $p is in too good condition to be repaired.", ch, bow, NULL, TO_CHAR );
    return;
  }

  int percent = get_ability( ch, gsn_bowyer );
  if ( percent == 0 ) {
    act("You don't know how to repair the string of $p.", ch, bow, NULL, TO_CHAR );
    return;
  }

  if ( bow->level > ch->level ) { // bow too high level
    if ( number_percent() > ch->level 
	 && bow->cRangedStringCondition > 0 ) { // unlucky: frayed string has been broken while trying to repair
      act("You break the string of $p while trying to repair it.", ch, bow, NULL, TO_CHAR );
      act("$n breaks the string of $p while trying to repair it.", ch, bow, NULL, TO_ROOM );
      bow->bRangedStringCondition = 0;
      check_improve( ch, gsn_bowyer, FALSE, 10 );
    }
  }
  else { // bow's level <= ch's level
    if ( number_percent() > percent ) { // failed
      act("You failed to repair the string of $p.", ch, bow, NULL, TO_CHAR );
      if ( chance(10) 
	   && bow->cRangedStringCondition > 0 ) { // unlucky: failed and string broken
	act("You break the string of $p while repairing it.", ch, bow, NULL, TO_CHAR );
	act("$n breaks the string of $p while repairing it.", ch, bow, NULL, TO_ROOM );
	bow->bRangedStringCondition = 0;
      }
      check_improve( ch, gsn_bowyer, FALSE, 5 );
    }
    else { // succeed
      act("You manage to repair the string of $p.", ch, bow, NULL, TO_CHAR );
      act("$n repairs the string of $p.", ch, bow, NULL, TO_ROOM );
      bow->bRangedStringCondition = MAX_STRING_CONDITION;
      check_improve( ch, gsn_bowyer, TRUE, 4 );
    }
  }

  recompobj(bow);
  return;
}

void do_strengthen( CHAR_DATA *ch, const char *argument ) {
//BOWYER RESTRING STRENGTHEN
//A character skilled as a bowyer is a master of the craft of improving and
//fixing bows.  Proficiency in this skill gives a character access to two
//separate commands: RESTRING and STRENGTHEN.
//STRENGTHEN allows a character to improve his or her bow, thus causing it to
//inflict more damage.  Strengthen can only be used on bows at least 2 levels
//below the character's level.  Attempting to strengthen a higher level bow
//could destroy it.  Strengthen will increase the bow's level in addition to
//the damage it inflicts, and can only be used a limited amount of times
//before a bow reaches its maximum strength.
//See also: help archery
  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char("Strengthen what?\n\r", ch );
    return;
  }

  OBJ_DATA *bow = get_obj_carry( ch, arg, ch );
  if ( bow == NULL ) { // no obj 'argument' found
    send_to_char("You can't find it.\n\r", ch );
    return;
  }
  if ( bow->item_type != ITEM_WEAPON
       || bow->value[0] != WEAPON_RANGED ) {
    act("$p is not a ranged weapon.", ch, bow, NULL, TO_CHAR );
    return;
  }

  if ( bow->condition <= 0  ) {
    act("$p is in too bad condition to be strengthen.", ch, bow, NULL, TO_CHAR );
    return;
  }

  if ( bow->cRangedStrength >= MAX_RANGED_STRENGTH ) { // already at maximum strength
    act("$p is already at maximum strength.", ch, bow, NULL, TO_CHAR );
    return;
  }

  int percent = get_ability( ch, gsn_bowyer );
  if ( percent == 0 ) {
    act("You don't know how to strengthen $p.", ch, bow, NULL, TO_CHAR );
    return;
  }

  if ( bow->level > ch->level-2 ) { // bow too high level
    act("$p is too high level for you to strengthen.", ch, bow, NULL, TO_CHAR );
    return;
  }
  
  // harder to strengthen if strength is too high
  percent -= bow->cRangedStrength*5;
  if ( number_percent() > percent ) { // failed
    act("You failed to strengthen $p.", ch, bow, NULL, TO_CHAR );
    if ( chance(10) ) { // really unlucky -> bow is broken and string too
      act("You break $p while trying to strengthen it.", ch, bow, NULL, TO_CHAR );
      bow->bRangedStringCondition = 0;
      bow->condition = 0;
      bow->bRangedStrength = 0;
    }
    check_improve( ch, gsn_bowyer, FALSE, 5 );
  }
  else { // succeed
    act("You strengthen $p.", ch, bow, NULL, TO_CHAR );
    act("$n strengthens $p.", ch, bow, NULL, TO_ROOM );
    bow->bRangedStrength++;
    if (bow->level < LEVEL_HERO - 1)
    bow->level = UMIN( LEVEL_HERO - 1, bow->level + 1 );
    check_improve( ch, gsn_bowyer, TRUE, 4 );
  }

  recompobj(bow);
  return;
}

void do_ignite_arrow( CHAR_DATA *ch, const char *argument ) {
//'IGNITE ARROW'
//Characters with skill in IGNITE ARROW are able to light arrows on fire, thus
//making them inflict flaming damage on their target in addition to the
//arrow's regular damage.  Attempting to ignite an arrow that is too powerful
//for a character may result in the arrow being destroyed by the flames.
//An arrow will only remain lit of fire for a short time.  Once that time,
//which does increase with a character's level, has expired, the flames will
//consume the arrow and it will be destroyed.
//See also: help archery
  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  if (arg[0] == '\0') {
    send_to_char("Ignite what arrow?\n\r",ch);
    return;
  }

  OBJ_DATA *arrow = get_obj_carry( ch, arg, ch );
  if ( arrow == NULL ) { // no obj 'argument' found
    send_to_char("You can't find it.\n\r", ch );
    return;
  }
  if ( arrow->item_type != ITEM_WEAPON
       && arrow->value[0] != WEAPON_ARROW ) {
    act("$p is not an arrow.", ch, arrow, NULL, TO_CHAR );
    return;
  }

  int percent = get_ability( ch, gsn_ignite_arrow );
  if ( percent == 0 ) {
    act("You don't know how to ignite $p.", ch, arrow, NULL, TO_CHAR );
    return;
  }

  if ( IS_WEAPON_STAT( arrow, WEAPON_FLAMING ) ) {
    act("$p is already ignited.", ch, arrow, NULL, TO_CHAR );
    return;
  }

  WAIT_STATE(ch,BEATS( gsn_ignite_arrow ));
  if ( number_percent() >= percent ) {
    act( "You fail to sharpen $p.", ch, arrow, NULL, TO_CHAR );
    check_improve( ch, gsn_ignite_arrow, FALSE, 3);
    return;
  }

  // this means they have no bonus
  int hit_bonus = 0;
  int dam_bonus = 0;
  int fail = 25;	// base 25% chance of failure

  // find the bonuses
  if ( !arrow->enchanted )
    for ( AFFECT_DATA *paf = arrow->pIndexData->affected; paf != NULL; paf = paf->next )
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
	if ( laf->location == ATTR_hitroll ) {
	  hit_bonus = laf->modifier;
	  fail += 2 * (hit_bonus * hit_bonus);
	}
	else if (laf->location == ATTR_damroll ) {
	  dam_bonus = laf->modifier;
	  fail += 2 * (dam_bonus * dam_bonus);
	}
	else  // things get a little harder
	  fail += 25;
  
  for ( AFFECT_DATA *paf = arrow->affected; paf != NULL; paf = paf->next )
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
      if ( laf->location == ATTR_hitroll ) {
	hit_bonus = laf->modifier;
	fail += 2 * (hit_bonus * hit_bonus);
      }
      else if (laf->location == ATTR_damroll ) {
	dam_bonus = laf->modifier;
	fail += 2 * (dam_bonus * dam_bonus);
      }
      else // things get a little harder
	fail += 25;

  // apply other modifiers
  fail -= 3 * ch->level/2;
  fail = URANGE(5,fail,95);

  int result = number_percent();

  // the moment of truth
  if (result < (fail / 3)) { // item destroyed
    act("$p shatters in thousand pieces!", ch, arrow, NULL, TO_CHAR );
    act("$p shatters in thousand pieces!", ch, arrow, NULL, TO_ROOM );
    extract_obj( arrow );
    check_improve( ch, gsn_ignite_arrow, FALSE, 3 );
    return;
  }
  if ( result <= fail ) { // failed, no bad result
    act("You fail to sharpen $p.", ch, arrow, NULL, TO_CHAR );
    check_improve( ch, gsn_ignite_arrow, FALSE, 3 );
    return;
  }

  act("You ignite $p.", ch, arrow, NULL, TO_CHAR );
  act("$n ignites $p.", ch, arrow, NULL, TO_ROOM );

  check_improve( ch, gsn_ignite_arrow, TRUE, 3 );

  AFFECT_DATA af;
  int dur = number_range(2+ch->level/20,4+ch->level/10);
  //afsetup( af, WEAPON, NA, OR, WEAPON_FLAMING, dur, ch->level/2, gsn_ignite_arrow );
  //affect_to_obj( arrow, &af );
  createaff(af,dur,ch->level/2,gsn_ignite_arrow,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,WEAPON,NA,OR,WEAPON_FLAMING);
  affect_to_obj( arrow, &af );

  return;
}


void do_sharpen_arrow( CHAR_DATA *ch, const char *argument ) {
// 'SHARPEN ARROW'
//Sharpen arrow is a skill used to improve the condition of an arrow.  It adds
//the "sharp" flag to the arrow, causing it to do more damage.  The arrow will
//remain sharp only for a limited duration, which increases with your level.
//Trying to use this skill on an arrow too powerful for you could result in
//its destruction.
  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );

  if (arg[0] == '\0') {
    send_to_char("Sharpen what arrow?\n\r",ch);
    return;
  }

  OBJ_DATA *arrow = get_obj_carry( ch, arg, ch );
  if ( arrow == NULL ) { // no obj 'argument' found
    send_to_char("You can't find it.\n\r", ch );
    return;
  }
  if ( arrow->item_type != ITEM_WEAPON
       && arrow->value[0] != WEAPON_ARROW ) {
    act("$p is not an arrow.", ch, arrow, NULL, TO_CHAR );
    return;
  }

  int percent = get_ability( ch, gsn_sharpen_arrow );
  if ( percent == 0 ) {
    act("You don't know how to sharpen $p.", ch, arrow, NULL, TO_CHAR );
    return;
  }

  if ( IS_WEAPON_STAT( arrow, WEAPON_SHARP ) ) {
    act("$p is already sharpen.", ch, arrow, NULL, TO_CHAR );
    return;
  }

  WAIT_STATE(ch,BEATS(gsn_sharpen_arrow));
  if ( number_percent() >= percent ) {
    act("You fail to sharpen $p.",ch,arrow,NULL,TO_CHAR);
    check_improve(ch,gsn_sharpen_arrow,FALSE,3);
    return;
  }
  
  // this means they have no bonus
  int hit_bonus = 0;
  int dam_bonus = 0;
  int fail = 25;	// base 25% chance of failure

  // find the bonuses
  if (!arrow->enchanted)
    for ( AFFECT_DATA *paf = arrow->pIndexData->affected; paf != NULL; paf = paf->next )
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
	if ( laf->location == ATTR_hitroll ) {
	  hit_bonus = laf->modifier;
	  fail += 2 * (hit_bonus * hit_bonus);
	}
	else if (laf->location == ATTR_damroll ) {
	  dam_bonus = laf->modifier;
	  fail += 2 * (dam_bonus * dam_bonus);
	}
 	else  // things get a little harder
	  fail += 25;
  
  for ( AFFECT_DATA *paf = arrow->affected; paf != NULL; paf = paf->next )
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
      if ( laf->location == ATTR_hitroll ) {
	hit_bonus = laf->modifier;
	fail += 2 * (hit_bonus * hit_bonus);
      }
      else if (laf->location == ATTR_damroll ) {
	dam_bonus = laf->modifier;
	fail += 2 * (dam_bonus * dam_bonus);
      }
      else // things get a little harder
	fail += 25;
  
  // apply other modifiers
  fail -= 3 * ch->level/2;
  fail = URANGE(5,fail,95);

  int result = number_percent();

  // the moment of truth
  if (result < (fail / 3)) { // item destroyed
    act("$p shatters in thousand pieces!",ch,arrow,NULL,TO_CHAR);
    act("$p shatters in thousand pieces!",ch,arrow,NULL,TO_ROOM);
    extract_obj(arrow);
    check_improve(ch,gsn_sharpen_arrow,FALSE,3);
    return;
  }
  if ( result <= fail ) { // failed, no bad result
    act("You fail to sharpen $p.",ch,arrow,NULL,TO_CHAR);
    check_improve(ch,gsn_sharpen_arrow,FALSE,3);
    return;
  }

  act("You sharpen $p.",ch,arrow,NULL,TO_CHAR);
  act("$n sharpens $p.",ch,arrow,NULL,TO_ROOM);

  arrow->baseval[4] |= WEAPON_SHARP;
  recompobj(arrow);

  if (arrow->level < LEVEL_HERO - 1)
    arrow->level = UMIN( LEVEL_HERO - 1, arrow->level + 1 );

  check_improve(ch,gsn_sharpen_arrow,TRUE,3);
}
