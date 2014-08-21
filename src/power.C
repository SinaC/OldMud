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
#include "olc_value.h"
#include "comm.h"
#include "magic.h"
#include "classes.h"
#include "db.h"
#include "fight.h"
#include "handler.h"
#include "skills.h"
#include "gsn.h"
#include "act_comm.h"
#include "act_info.h"
// Added by SinaC 2003
#include "condition.h"
#include "interp.h"
#include "ability.h"
#include "const.h"
#include "recycle.h"
#include "config.h"
#include "utils.h"
#include "act_move.h"
#include "lookup.h"
#include "damage.h"
#include "act_enter.h"
#include "effects.h"


// Added by SinaC 2001 for mental user
void do_psipowers(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char arg[MAX_INPUT_LENGTH];
  /*char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];*/
  char spell_list[MAX_LEVEL+1][MAX_STRING_LENGTH];
  /*char spell_columns[LEVEL_HERO + 1];*/
  char spell_columns[MAX_LEVEL+1];
  int sn, level, 
      // Modified by SinaC 2000
    min_lev = 0, 
    /*max_lev = LEVEL_HERO, */
    max_lev = MAX_LEVEL,
    psp,
    // Added by SinaC 2000
    pra;
  bool fAll = FALSE, found = FALSE;
  char buf[MAX_STRING_LENGTH];
 
  if (IS_NPC(ch))
    return;

  if (argument[0] != '\0') {
    fAll = TRUE;
    
    if (str_prefix(argument,"all")) {
      argument = one_argument(argument,arg);
      if (!is_number(arg)) {
	send_to_char("Arguments must be numerical or all.\n\r",ch);
	return;
      }
      max_lev = atoi(arg);
      
      // Modified by SinaC 2000
      if (max_lev < 0 || max_lev > LEVEL_HERO) {
	sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
	send_to_char(buf,ch);
	return;
      }
      
      if (argument[0] != '\0') {
	argument = one_argument(argument,arg);
	if (!is_number(arg)) {
	  send_to_char("Arguments must be numerical or all.\n\r",ch);
	  return;
	}
	min_lev = max_lev;
	max_lev = atoi(arg);
      // Modified by SinaC 2000	
	if (max_lev < 0 || max_lev > LEVEL_HERO) {
	  sprintf(buf,
		  "Levels must be between 1 and %d.\n\r",LEVEL_HERO);
	  send_to_char(buf,ch);
	  return;
	}
	
	if (min_lev > max_lev) {
	  send_to_char("That would be silly.\n\r",ch);
	  return;
	}
      }
    }
  } 

  /* initialize data */
  /*    for (level = 0; level < LEVEL_HERO + 1; level++)*/
  for (level = 0; level < MAX_LEVEL + 1; level++) {
    spell_columns[level] = 0;
    spell_list[level][0] = '\0';
  }
  /***************************/ 

  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;
    
    // Modified by SinaC 2001
    level = class_abilitylevel( /*ch->cstat(classes)*/ch, sn);
    // Modified by SinaC 2001
    //pra = ch->pcdata->skill_info[sn].learned;
    pra = get_ability_simple( ch, sn );

    // Modified by SinaC 2000
    //pra = get_skill( ch, sn );
    // Modified by SinaC 2000, again by SinaC 2001 for god skill
    if ( get_raceability( ch, sn ) 
	 || get_clanability(ch, sn )
	 /*|| get_godskill( ch, sn )     removed by SinaC 2003*/) { 
      level = 0;
      pra = 100;
    }
    
    if ( ( level < LEVEL_HERO + 1 
	   /* Modified by Sinac 1997 */
	   || ch->level >= IM  )
	 &&  (fAll || level <= ch->level)
	 &&  level >= min_lev && level <= max_lev
	 //&&  ability_table[sn].spell_fun != spell_null
	 && ability_table[sn].type == TYPE_POWER
	 // Modified by SinaC 2000
	 &&  /*ch->pcdata->learned[sn] > 0*/ pra > 0 ) {
      found = TRUE;
      // Modified by SinaC 2000
      /*
	level = class_skilllevel( ch->cstat(classes),sn);
	// Modified by Sinac 1997
	if ( raceskill ) level = 1;
      */
      if (ch->level < level)
	sprintf(buf,"%-20s  n/a       ", ability_table[sn].name);
      else {
	// Modified by SinaC 2001, Modified again by SinaC 2001
	/*
	  if (ch->level + 2 == class_skilllevel(ch,sn))
	  mana = 50;
	  else
	  mana = UMAX(
	  ability_table[sn].min_mana,
	  // Modified by SinaC 2001
	  100 / ( 2 + ch->level -  class_skilllevel(ch,sn) ) );
	*/
	if (ch->level + 2 == level )
	  psp = 50;
	else
	  psp = UMAX( //ability_table[sn].min_mana,
		     ability_table[sn].min_cost,
		       // Modified by SinaC 2001
		       100 / ( 2 + ch->level -  level ) );

	//mana = UMAX(ability_table[sn].min_mana,
	//	    100/(2 + ch->level - level));

	// Modified by SinaC 2000, again by SinaC 2001
	int casting_level = get_casting_level( ch, sn );
	if ( /*ch->pcdata->skill_info[sn].learned_lvl*/
	    casting_level != 0 )
	  sprintf(buf,
		  "%-20s  %3d  (%2d) ",
		  ability_table[sn].name,
		  psp,
		  /*ch->pcdata->skill_info[sn].learned_lvl*/
		  casting_level);
	else
	  sprintf(buf,
		  "%-20s  %3d       ",
		  ability_table[sn].name,
		  psp);
      }
      
      if (spell_list[level][0] == '\0')
	sprintf(spell_list[level],"\n\rLvl %3d: %s",level,buf);
      else { /* append */
	if ( ++spell_columns[level] % 2 == 0)
	  strcat(spell_list[level],"\n\r         ");
	strcat(spell_list[level],buf);
      }
    }
  }
  
  /* return results */
 
  // Modified by SinaC 2000
  if (!found) {
    send_to_char("No powers found.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  send_to_charf( ch,
		 "Level    Power name           Psp    lvl"
		 " Power name           Psp   lvl\n\r"
		 "----------------------------------------"
		 " ------------------------------");

  buffer = new_buf();
  /*    for (level = 0; level < LEVEL_HERO + 1; level++)*/
  for (level = 0; level < MAX_LEVEL+1; level++)
    if (spell_list[level][0] != '\0')
      add_buf(buffer,spell_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}


// Added by SinaC 2001 for mental user
/* for finding psp costs -- temporary version */
int psp_cost (CHAR_DATA *ch, int min_psp, int level) {
  if (ch->level + 2 == level)
    return 1000;
  return UMAX(min_psp,(100/(2 + ch->level - level)));
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 * Another global for level of the spell added by SinaC 2000
 */
void do_power( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int psp;
  int sn;
  int target;
  // Added by SinaC 2000 to store the true caster if a switched mob
  //  use to determine if there is enough psp, so they use psp
  CHAR_DATA *caster;

  // Added by SinaC 2001
  if ( ch->in_room == NULL )
    return;

  /*
   * Switched NPC's can cast spells, but others can't.
   */

  // Modified by SinaC 2000
  /*
  if ( IS_NPC(ch) && ch->desc == NULL)
    return;
  */
  // Modified by SinaC 2000 for mob class, so they use psp
  if ( IS_NPC(ch) && ch->desc != NULL )
    caster = ch->desc->original;
  else
    caster = ch;
            
  //target_name = one_argument( argument, arg1 );
  //one_argument( target_name, arg2 );
  // Modified by SinaC 2001
  argument = one_argument( argument, arg1 );
  strcpy(target_name,argument);
  argument = one_argument( argument, arg2 );
  one_argument(argument, arg3 );

  // without that test, player are forced to specify the target if they want to
  //  to use a specific level of a spell
  // forced to use: cast armor self 4
  // better if they can use: cast armor 4
  if ( is_number(arg2) && arg3[0] == '\0' ) {
    strcpy(arg3,arg2);  // copy arg2 in arg3
    strcpy(arg2,"");    // arg2 is empty
  }

  if ( arg1[0] == '\0' ) {
    send_to_char( "Power which what where?\n\r", ch );
    return;
  }

  if( //(sn = find_spell(ch,arg1)) < 1
     (sn = find_ability(ch,arg1,TYPE_POWER)) < 1
      //|| ability_table[sn].spell_fun == spell_null
      // Added by SinaC 2001 for mental power
      || ability_table[sn].type != TYPE_POWER
      || get_ability(ch,sn)==0 ) {
    send_to_char( "You don't know any powers of that name.\n\r", ch );
    if ( SCRIPT_VERBOSE > 0 ) {
      if ( IS_NPC(ch) )
	log_stringf("%s (%d) tries to use power: %s %s %s", 
		    NAME(ch), ch->pIndexData->vnum,
		    arg1, arg2, arg3 );
    }
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( ch->position < ability_table[sn].minimum_position ) {
    send_to_char( "You can't concentrate enough.\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  // Modified by SinaC 2001
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOPOWER ) 
       && !IS_IMMORTAL( ch ) ) {
    send_to_char( "A force prevents you from using powers.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  if (ch->level + 2 == class_abilitylevel(/*ch->cstat(classes)*/ch,sn))
    psp = 50;
  else
    psp = UMAX(
	       //ability_table[sn].min_mana,
	       ability_table[sn].min_cost,
	       100 / ( 2 + ch->level -  class_abilitylevel(/*ch->cstat(classes)*/ch,sn) ) );

  /*
   * Locate targets.
   */
  victim	= NULL;
  obj		= NULL;
  vo		= NULL;
  target	= TARGET_NONE;

  switch ( ability_table[sn].target ) {
  default:
    bug( "Do_power: bad target for sn %d.", sn );
    return;

  case TAR_IGNORE:
    break;

  case TAR_CHAR_OFFENSIVE:
    if ( arg2[0] == '\0' ) {
      if ( ( victim = ch->fighting ) == NULL ) {
	send_to_char( "Use the power on whom?\n\r", ch );
	return;
      }
    }
    else {
      // Modified by SinaC 2001
      //if ( ( victim = get_char_room( ch, target_name ) ) == NULL ) {
      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }
    /*
     *        if ( ch == victim )
     *       {
     *           send_to_char( "You can't do that to yourself.\n\r", ch );
     *           return;
     *       }
     */

// Removed by SinaC 2001
//    if ( !IS_NPC(ch) ) {

      if (is_safe(ch,victim) && victim != ch) {
	send_to_char("Not on that target.\n\r",ch);
	return;
      }
      check_killer(ch,victim);
//    }

    if ( IS_AFFECTED(ch, AFF_CHARM) 
	 && ch->master == victim ) {
      send_to_char( "You can't do that on your own follower.\n\r",
		    ch );
      return;
    }

    vo = (void *) victim;
    target = TARGET_CHAR;
    break;

  case TAR_CHAR_DEFENSIVE:
    if ( arg2[0] == '\0' ) {
      victim = ch;
    }
    else {
      // Modified by SinaC 2001
      //if ( ( victim = get_char_room( ch, target_name ) ) == NULL ) {
      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    }

    vo = (void *) victim;
    target = TARGET_CHAR;
    break;

  case TAR_CHAR_SELF:
    // Modified by SinaC 2001
    //if ( arg2[0] != '\0' && !is_name( target_name, ch->name ) ) {
    if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) ) {
      send_to_char( "You cannot use this power on another.\n\r", ch );
      return;
    }

    vo = (void *) ch;
    target = TARGET_CHAR;
    break;

  case TAR_OBJ_INV:
    if ( arg2[0] == '\0' ) {
      send_to_char( "What should the power be use upon?\n\r", ch );
      return;
    }

    // Modified by SinaC 2001
    //if ( ( obj = get_obj_carry( ch, target_name, ch ) ) == NULL ) {
    if ( ( obj = get_obj_carry( ch, arg2, ch ) ) == NULL ) {
      send_to_char( "You are not carrying that.\n\r", ch );
      return;
    }

    vo = (void *) obj;
    target = TARGET_OBJ;
    break;

  case TAR_OBJ_CHAR_OFF:
    if (arg2[0] == '\0') {
      if ((victim = ch->fighting) == NULL) {
	send_to_char("Use the power on whom or what?\n\r",ch);
	return;
      }

      target = TARGET_CHAR;
    }
    // Added by SinaC 2001
    //else if ((victim = get_char_room(ch,target_name)) != NULL) {
    else if ((victim = get_char_room(ch,arg2)) != NULL) {
      target = TARGET_CHAR;
    }

    if (target == TARGET_CHAR) {/* check the sanity of the attack */
      if(is_safe_spell(ch,victim,FALSE) && victim != ch) {
	send_to_char("Not on that target.\n\r",ch);
	return;
      }

      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
	send_to_char( "You can't do that on your own follower.\n\r",
		      ch );
	return;
      }

      if (!IS_NPC(ch))
	check_killer(ch,victim);

      vo = (void *) victim;
    }
    // Modified by SinaC 2001
    //else if ((obj = get_obj_here(ch,target_name)) != NULL) {
    else if ((obj = get_obj_here(ch,arg2)) != NULL) {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
    break;

  case TAR_OBJ_CHAR_DEF:
    if (arg2[0] == '\0') {
      vo = (void *) ch;
      target = TARGET_CHAR;                                                 
    }
    // Modified by SinaC 2001
    //else if ((victim = get_char_room(ch,target_name)) != NULL) {
    else if ((victim = get_char_room(ch,arg2)) != NULL) {
      vo = (void *) victim;
      target = TARGET_CHAR;
    }
    // Modified by SinaC 2001
    //else if ((obj = get_obj_carry(ch,target_name,ch)) != NULL) {
    else if ((obj = get_obj_carry(ch,arg2,ch)) != NULL) {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
    break;
  }

  // Modified by SinaC 2000 for mob class, so they use mana
  //if ( !IS_NPC(ch) && ch->mana < mana ) {
  if ( caster->psp < psp ) {
    send_to_char( "You don't have enough psp.\n\r", ch );
    return;
  }
      
  // no say_spell, SinaC 2001

  //WAIT_STATE( ch, ability_table[sn].beats );
  WAIT_STATE( ch, BEATS(sn) );

  // Added by SinaC 2000
  // we can lose our concentration if we're not enough wise
  int perc;
  // WIS     perc
  //   0 ==>   3 + 25 =  28
  //   5 ==>  10 + 25 =  35
  //  10 ==>  27 + 25 =  52
  //  15 ==>  31 + 25 =  56
  //  20 ==>  46 + 25 =  71
  //  25 ==>  65 + 25 =  95
  //  30 ==>  90 + 25 = 115
  if ( IS_IMMORTAL( ch ) ) 
    perc = 100;
  else
    // major modification with do_cast: WIS is used instead of INT, SinaC 2001
    perc = UMIN( int_app[ch->cstat(WIS)].learn + 25, 100 );

  //  if we can't see the target, we have less chance to cast successfull ( -50% )
  bool notsee;
  bool missed;
  if ( vo != NULL )
    if ( target == TARGET_CHAR )
      notsee = !can_see( ch, (CHAR_DATA*)vo );
    else
      notsee = !can_see_obj( ch, (OBJ_DATA*)vo );
  else
    notsee = FALSE;
  if ( notsee )
    perc -= 50;

  missed = FALSE;
  // Modified by SinaC 2000
  //if ( number_percent() > get_ability(ch,sn) )
  if ( ( ( number_percent() > get_ability(ch,sn)
	|| number_percent() > perc ) )
       && !IS_IMMORTAL(ch) ) {

    // if we missed the target because we didn't see it ( !can_see )
    if ( notsee ){
      send_to_char( "You missed your target.\n\r", ch );
      check_improve(ch,sn,FALSE,1);
      caster->psp -= psp;
      missed = TRUE;
    }
    // if we missed to cast the spell
    // Added by SinaC 2001 race spell can't be missed
    else if ( !get_raceability(ch,sn) ) {
      send_to_char( "You lost your concentration.\n\r", ch );
      check_improve(ch,sn,FALSE,1);
      // Modified by SinaC 2000 for mob class, so they use mana
      //ch->mana -= mana / 2;
      caster->psp -= psp/2;
      missed = TRUE;
    }
  }

  // Modified by SinaC 2001
  if ( !missed ) {
    // Modified by SinaC 2000 for mob class, so they use mana
    //ch->mana -= mana;
    caster->psp -= psp;

    // Modified by SinaC 2000
    int sp_lvl;
    // class has spells
    if (IS_NPC(ch) || class_fPsp(ch->cstat(classes)))
      sp_lvl = ch->level;
    else
      sp_lvl = UMAX(3 * ch->level/4,1);
   
    // Modified by SinaC 2001
    (*(POWER_FUN*)ability_table[sn].action_fun) ( sn, sp_lvl, ch, vo, target );
    check_improve(ch,sn,TRUE,1);
  }

  if ( ch->in_room == NULL )
    return;

  if ( (ability_table[sn].target == TAR_CHAR_OFFENSIVE
	||   (ability_table[sn].target == TAR_OBJ_CHAR_OFF 
	      && target == TARGET_CHAR))
       &&   victim != ch
       &&   victim->master != ch) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    for ( vch = ch->in_room->people; vch; vch = vch_next ) {
      vch_next = vch->next_in_room;
      if ( victim == vch && victim->fighting == NULL ) {	
	check_killer(victim,ch);
	multi_hit( victim, ch, TYPE_UNDEFINED );
	break;
      }
    }
  }

  return;
}


/*
 * Use powers at targets using a magical object.
 */
void obj_use_power( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) {
  void *vo;
  int target = TARGET_NONE;

  if ( sn <= 0 )
    return;

  if ( sn >= MAX_ABILITY
       && ( ability_table[sn].type != TYPE_SPELL || ability_table[sn].type != TYPE_POWER ) ) {
    bug( "Obj_cast_power: bad sn %d.", sn );
    return;
  }

  if ( ability_table[sn].type == TYPE_SPELL ) {
    obj_cast_spell( sn, level, ch, victim, obj );
    return;
  }
  
  switch ( ability_table[sn].target ) {
  default:
    bug( "Obj_use_power: bad target for sn %d.", sn );
    return;

  case TAR_IGNORE:
    vo = NULL;
    break;

  case TAR_CHAR_OFFENSIVE:
    if ( victim == NULL )
      victim = ch->fighting;
    if ( victim == NULL ) {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }
    if (is_safe(ch,victim) && ch != victim) {
      send_to_char("Something isn't right...\n\r",ch);
      return;
    }
    vo = (void *) victim;
    target = TARGET_CHAR;
    break;

  case TAR_CHAR_DEFENSIVE:
  case TAR_CHAR_SELF:
    if ( victim == NULL )
      victim = ch;
    vo = (void *) victim;
    target = TARGET_CHAR;
    break;

  case TAR_OBJ_INV:
    if ( obj == NULL || !obj->valid ) {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }
    vo = (void *) obj;
    target = TARGET_OBJ;
    break;

  case TAR_OBJ_CHAR_OFF:
    if ( victim == NULL && obj == NULL || !victim->valid || !obj->valid )
      if (ch->fighting != NULL)
	victim = ch->fighting;
      else {
	send_to_char("You can't do that.\n\r",ch);
	return;
      }

    if (victim != NULL) {
      if (is_safe_spell(ch,victim,FALSE) && ch != victim) {
	send_to_char("Somehting isn't right...\n\r",ch);
	return;
      }

      vo = (void *) victim;
      target = TARGET_CHAR;
    }
    else {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    break;

  case TAR_OBJ_CHAR_DEF:
    if (victim == NULL && obj == NULL || !victim->valid || !obj->valid ) {
      vo = (void *) ch;
      target = TARGET_CHAR;
    }
    else if (victim != NULL) {
      vo = (void *) victim;
      target = TARGET_CHAR;
    }
    else {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
	
    break;
  }

  // Modified by SinaC 2001
  //target_name = "";
  strcpy(target_name,"");

  (*(POWER_FUN*)ability_table[sn].action_fun) ( sn, level, ch, vo, target );
  
  if ( (ability_table[sn].target == TAR_CHAR_OFFENSIVE
	|| (ability_table[sn].target == TAR_OBJ_CHAR_OFF && target == TARGET_CHAR))
       &&   victim != ch
       &&   victim->master != ch ) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    for ( vch = ch->in_room->people; vch; vch = vch_next ) {
      vch_next = vch->next_in_room;
      if ( victim == vch && victim->fighting == NULL ) {
	check_killer(victim,ch);
	multi_hit( victim, ch, TYPE_UNDEFINED );
	break;
      }
    }
  }

  return;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void power_death_field ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int        dam;
  int        hpch;
  AFFECT_DATA af;

  if ( !IS_EVIL( ch ) ) {
    send_to_char( "You are not evil enough to do that!\n\r", ch);
    return;
  }

  send_to_char( "A black haze emanates from you!\n\r", ch );
  act ( "A black haze emanates from $n!", ch, NULL, ch, TO_ROOM );

  for ( vch = ch->in_room->people; vch; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( is_safe_spell( ch, vch, TRUE ) 
	 || (vch == ch) 
	 || is_same_group(ch,vch)) 
      continue;

    dam = hpch = URANGE( 10, ch->hit, 999 );
    if ( !saves_spell( level, vch , DAM_NEGATIVE )
	 && level <= vch->level + 5
	    /*  && level >= vch->level - 5 )*/ ) {
      send_to_char( "The haze envelops and kills you!\n\r", vch );
      act( "The haze envelops $N and kills $M!", ch, NULL, vch, TO_ROOM );
      act( "The haze envelops $N and kills $M!", ch, NULL, vch, TO_CHAR );
      sudden_death( ch, vch );
    }
    else {
      // Added by SinaC 2000
      createaff(af,level/4,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
      bool found = FALSE;
      if ( number_percent() > 50 ){
	//afsetup( af, CHAR, STR, ADD, -level/20, level/4, level, sn );
	//affect_to_char( vch, &af );
	found = TRUE;
	addaff(af,CHAR,STR,ADD,-level/20);
      }
      if ( number_percent() > 50 ){
	//afsetup( af, CHAR, DEX, ADD, -level/30, level/4, level, sn );
	//affect_to_char( vch, &af );
	found = TRUE;
	addaff(af,CHAR,DEX,ADD,-level/30);
      }
      if ( number_percent() > 50 ){
	//afsetup( af, CHAR, hitroll, ADD, -level/15, level/4, level, sn );
	//affect_to_char( vch, &af );
	found = TRUE;
	addaff(af,CHAR,hitroll,ADD,-level/15);
      }
      if ( number_percent() > 50 ){
	//afsetup( af, CHAR, damroll, ADD, -level/15, level/4, level, sn );
	//affect_to_char( vch, &af );
	found = TRUE;
	addaff(af,CHAR,damroll,ADD,-level/15);
      }
      if ( found )
	affect_to_char(vch,&af);

      ability_damage( ch, vch, dam, sn, DAM_NEGATIVE , TRUE, TRUE);
    }
  }
  return;
}
////////////////////////////////////////////////////////////////////////////////////////////////////




void power_complete_healing ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{                  
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  victim->hit = victim->cstat(max_hit);
  update_pos( victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  send_to_char( "Ahhhhhh...You are completely healed!\n\r", victim );
  return;        
}

void power_lend_health ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int        hpch;

  if ( ch == victim ) {
    send_to_char( "Lend health to yourself ?\n\r", ch );
    return;
  }
  hpch = UMIN( 50, victim->cstat(max_hit) - victim->hit );
  if ( hpch == 0 ) {
    act( "Nice thought, but $N doesn't need healing.", ch, NULL,
	 victim, TO_CHAR );
    return;
  }
  if ( ch->hit-hpch < 50 ) {
    send_to_char( "You aren't healthy enough yourself!\n\r", ch );
    return;
  }
  victim->hit += hpch;
  ch->hit     -= hpch;
  update_pos( victim );
  update_pos( ch );

  act( "You lend some of your health to $N.", ch, NULL, victim, TO_CHAR );
  act( "$n lends you some of $s health.",     ch, NULL, victim, TO_VICT );

  return;
}

void power_levitation ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_FLYING ) ) {
    if (victim == ch)
      send_to_char("You are already airborne.\n\r",ch);
    else
      act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_FLYING,3+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,3+level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_FLYING);
  affect_to_char( victim, &af );

  send_to_char( "Your feet rise off the ground.\n\r", victim );
  act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_dizziness( int sn, int level, CHAR_DATA *ch, void *vo, int target) {
  CHAR_DATA *victim = ( CHAR_DATA * ) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || saves_spell(level,victim,DAM_MENTAL)) {
    send_to_char("Power failed.\n\r",ch);
    return;
  }
  
  //afsetup(af,CHAR,hitroll,ADD,-1,1+level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,allAC,ADD,level/2,1+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,1+level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,-1);
  addaff(af,CHAR,allAC,ADD,level/2);
  affect_to_char( victim, &af );

  send_to_char( "You feel dizzy.\n\r", victim );
  act( "$n feels dizzy.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_hesitation( int sn, int level, CHAR_DATA *ch, void *vo, int target) {
  CHAR_DATA *victim = ( CHAR_DATA * ) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || saves_spell(level,victim,DAM_MENTAL)) {
    send_to_char("Power failed.\n\r",ch);
    return;
  }
  
  //afsetup(af,CHAR,hitroll,ADD,0-level/8,1+level,level,sn);
  //affect_to_char( victim, &af );
  //af.location  = ATTR_damroll;
  //affect_to_char( victim, &af );
  //af.location  = ATTR_DEX;
  //af.modifier  = -2;
  //affect_to_char( victim, &af );
  createaff(af,1+level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,0-level/8);
  addaff(af,CHAR,damroll,ADD,0-level/8);
  addaff(af,CHAR,DEX,ADD,-2);
  affect_to_char( victim, &af );

  send_to_char( "You feel hesitating.\n\r", victim );
  act( "$n feels hesitating.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_trouble( int sn, int level, CHAR_DATA *ch, void *vo, int target) {
  CHAR_DATA *victim = ( CHAR_DATA * ) vo;
  AFFECT_DATA af;
  
  if ( is_affected( victim, sn ) 
       || saves_spell(level,victim,DAM_MENTAL)) {
    send_to_char("Power failed.\n\r",ch);
    return;
  }
  
  //afsetup(af,CHAR,DEX,ADD,-2,level/2,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/2,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,DEX,ADD,-2);
  affect_to_char( victim, &af );
  
  send_to_char( "You feel troubled.\n\r", victim );
  act( "$n feels troubled.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_minor_pain( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(2 + level/2, 3);
  if ( saves_spell( level, victim, DAM_MENTAL) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_MENTAL ,TRUE, TRUE);
  return; 
}

void power_major_pain( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(5 + level/2, 6);
  if ( saves_spell( level, victim, DAM_MENTAL) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_MENTAL ,TRUE, TRUE);
  return; 
}

void power_psychic_impact( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(4 + level/2, 5);
  if ( saves_spell( level, victim, DAM_MENTAL) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_MENTAL ,TRUE, TRUE );
  return; 
}

void power_combat_mind( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = ( CHAR_DATA * ) vo;
  AFFECT_DATA af;
  
  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You are already affected by combat mind.\n\r", ch );
    else
      act("$N is already affected by combat mind.",ch,NULL,victim,TO_CHAR);
    return;
  }
   
  if ( IS_AFFECTED( victim, AFF_CALM ) ) {
    if ( victim == ch )    
      send_to_char( "Why don't you just relax for a while ?\n\r", ch );
    else
      act( "$N doesn't look like $e wants to fight anymore.",
	   ch, NULL, victim, TO_CHAR );
    return;
  }    
  
  //afsetup(af,CHAR,hitroll,ADD,level/8,level,level,sn);
  //affect_to_char( victim, &af );
  //af.location  = ATTR_damroll;
  //affect_to_char( victim, &af );
  //af.location  = ATTR_allAC;
  //af.modifier  = (-10*level)/12;
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,level/8);
  addaff(af,CHAR,damroll,ADD,level/8);
  addaff(af,CHAR,allAC,ADD,(-10*level)/12);
  affect_to_char( victim, &af );

  send_to_char( "Your sense of battle improve.\n\r", victim );
  act( "The sense of battle of $n improves.", victim, NULL, NULL, TO_ROOM );
  
  return;
}

/*
 * Code for Psionicist spells/skills by Thelonius
 */
void power_adrenaline_control ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You already control your adrenaline flow.\n\r", ch );
    else
      act("$N already controls $S adrenaline flow.\n\r", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,DEX,ADD,2,level-5,level,sn);
  //affect_to_char( victim, &af );
  //af.location = ATTR_CON;
  //affect_to_char( victim, &af );
  createaff(af,level-5,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,DEX,ADD,2);
  addaff(af,CHAR,CON,ADD,2);
  affect_to_char( victim, &af );

  send_to_char( "You have given yourself an adrenaline rush!\n\r", ch );
  act( "$n has given $mself an adrenaline rush!", ch, NULL, NULL, TO_ROOM );

  return;
}

void power_agitation ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim       = (CHAR_DATA *) vo;
  int        dam;

  static const int        dam_each [ ] =
  {
    0,
      4,  5,  6,  8,  10,      12, 15, 18, 21, 24,
      24, 24, 25, 25, 26,      26, 26, 27, 27, 27,
      28, 28, 28, 29, 29,      29, 30, 30, 30, 31,
      31, 31, 32, 32, 32,      33, 33, 33, 34, 34,
      34, 35, 35, 35, 36,      36, 36, 37, 37, 37,

      // added by SinaC 2000

      38, 38, 38, 39, 39,      39, 39, 40, 40, 40,
      40, 41, 41, 41, 41,      42, 42, 42, 42, 43,
      43, 43, 43, 44, 44,      44, 44, 45, 45, 45,
      45, 46, 46, 46, 46,      47, 47, 47, 47, 48,
      48, 48, 48, 48, 48,      49, 49, 49, 49, 49
      };

  level    = UMIN( level, ( int)(sizeof( dam_each )/sizeof( dam_each[0] ) - 1) );
  level    = UMAX( 0, level );
  // Modified by SinaC 2001:  /2,  *2  before
  dam      = number_range( dam_each[level], dam_each[level] * 4 );

  if ( saves_spell( level, victim, DAM_MENTAL ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_MENTAL,TRUE, TRUE );
  return;
}

void power_aura_sight ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  const char *msg;
  const char *msg_etho;
  int ap;

  // Modified by SinaC 2001 etho/alignment are attributes now
  // Modified by SinaC 2000 for alignment/etho
  //ap = victim->align.alignment;
  ap = victim->cstat(alignment);

  // Added by SinaC 2001 for etho
  //  switch( victim->align.etho ) {
  switch( victim->cstat(etho) ) {
    case  1: msg_etho = "$N is lawful"; break;
    case  0: msg_etho = "$N is neutral"; break;
    case -1: msg_etho = "$N is chaotic"; break;
  default: msg_etho = "Etho bug: Warn an immortal"; break;
  }

  if ( ap >  700 ) msg = "$N has a pure and good aura.";
  else if ( ap >  350 ) msg = "$N is of excellent moral character.";
  else if ( ap >  100 ) msg = "$N is often kind and thoughtful.";
  else if ( ap > -100 ) msg = "$N doesn't have a firm moral commitment.";
  else if ( ap > -350 ) msg = "$N lies to $S friends.";
  else if ( ap > -700 ) msg = "$N is a black-hearted murderer.";
  else msg = "$N is the embodiment of pure evil!.";

  // Added by SinaC 2001 for etho
  act( msg_etho, ch, NULL, victim, TO_CHAR );
  act( msg, ch, NULL, victim, TO_CHAR );
  return;
}

void power_awe ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( victim->fighting == ch && !saves_spell( level, victim, DAM_OTHER ) ) {
    stop_fighting ( victim, TRUE);
    act( "$N is in AWE of you!", ch, NULL, victim, TO_CHAR    );
    act( "You are in AWE of $n!",ch, NULL, victim, TO_VICT    );
    act( "$N is in AWE of $n!",  ch, NULL, victim, TO_NOTVICT );
  }
  else
    send_to_char("Power failed.\n\r",ch);
  return;
}

void power_ballistic_attack ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim       = (CHAR_DATA *) vo;
  int        dam;

  static const int        dam_each [ ] =
  {
    0,
      3,  4,  4,  5,  6,       6,  6,  7,  7,  7,
      7,  7,  8,  8,  8,       9,  9,  9, 10, 10,
      10, 11, 11, 11, 12,      12, 12, 13, 13, 13,
      14, 14, 14, 15, 15,      15, 16, 16, 16, 17,
      17, 17, 18, 18, 18,      19, 19, 19, 20, 20,

      // added by SinaC 2000

      20, 21, 21, 21, 22,      22, 22, 23, 23, 23,
      24, 24, 24, 25, 25,      25, 26, 26, 26, 27,
      27, 27, 28, 28, 28,      29, 29, 29, 29, 30,
      30, 30, 30, 31, 31,      31, 31, 32, 32, 32,
      32, 33, 33, 33, 33,      34, 34, 34, 34, 34
      };

  level    = UMIN( level, ( int)(sizeof( dam_each )/sizeof( dam_each[0] ) - 1) );
  level    = UMAX( 0, level );
  dam      = number_range( dam_each[level] / 2, dam_each[level] * 2 );

  if ( saves_spell( level, victim, DAM_BASH ) )
    dam /= 2;
  act( "You chuckle as a stone strikes $N.", ch, NULL, victim,
       TO_CHAR );
  ability_damage( ch, victim, dam, sn, DAM_BASH,TRUE, TRUE);
  return;
}

void power_biofeedback ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_SANCTUARY ) ) {
    if (victim == ch)
      send_to_char("You are already affected by a sanctuary like spell.\n\r",ch);
    else
      act("$N is already affected by a sanctuary like spell.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_SANCTUARY,number_fuzzy( level / 8 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 8 ),level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  affect_to_char( victim, &af );

  send_to_char( "You are surrounded by a white aura.\n\r", victim );
  act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_cell_adjustment ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( is_affected( victim, gsn_poison ) ) {
    affect_strip( victim, gsn_poison );
    send_to_char( "A warm feeling runs through your body.\n\r", victim );
    act( "$N looks better.", ch, NULL, victim, TO_NOTVICT );
  }
  if ( is_affected( victim, gsn_curse  ) ) {
    affect_strip( victim, gsn_curse  );
    send_to_char( "You feel better.\n\r", victim );
  }
  send_to_char( "Ok.\n\r", ch );
  return;
}

void power_control_flames ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim       = (CHAR_DATA *) vo;
  static const int        dam_each [ ] =
  {
    0,
      4,  5,  6,  8,  9,       11,  13, 16, 20, 24,
      28, 32, 35, 38, 40,      42, 44, 45, 45, 45,
      46, 46, 46, 47, 47,      47, 48, 48, 48, 49,
      49, 49, 50, 50, 50,      51, 51, 51, 52, 52,
      52, 53, 53, 53, 54,      54, 54, 55, 55, 55,

      // added by SinaC 2000
	 
      56, 56, 56, 57, 57,      57, 58, 58, 58, 59,
      59, 59, 60, 60, 60,      61, 61, 61, 62, 62,
      62, 63, 63, 63, 64,      64, 64, 65, 65, 65,
      66, 66, 66, 67, 67,      67, 68, 68, 68, 69,
      69, 69, 69, 70, 70,      70, 70, 71, 71, 71
      };
  int        dam;

  if ( !get_eq_char( ch, WEAR_LIGHT ) ) {
    send_to_char( "You must be carrying a light source.\n\r", ch );
    return;
  }

  level    = UMIN( level, ( int)(sizeof( dam_each )/sizeof( dam_each[0] ) - 1) );
  level    = UMAX( 0, level );
  //   /2   *2  before
  dam      = number_range( dam_each[level], dam_each[level] * 3 );
  if ( saves_spell( level, victim, DAM_FIRE ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_FIRE,TRUE, TRUE );
  return;
}

void power_create_sound ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  char       buf1    [ MAX_STRING_LENGTH ];
  char       buf2    [ MAX_STRING_LENGTH ];
  char       speaker [ MAX_INPUT_LENGTH  ];
  // Added by SinaC 2001
  const char      *tname;

  // Modified by SinaC 2001
  //target_name = one_argument( target_name, speaker );
  tname = one_argument( target_name, speaker );

  sprintf( buf1, "%s says '%s'.\n\r", speaker, /*target_name*/tname );
  sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, /*target_name*/tname );
  buf1[0] = UPPER( buf1[0] );

  for ( vch = ch->in_room->people; vch; vch = vch->next_in_room ) {
    if ( !is_name( speaker, vch->name ) )
      send_to_char( saves_spell( level, vch, DAM_OTHER ) ? buf2 : buf1, vch );
  }
  return;
}

void power_detonate ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim       = (CHAR_DATA *) vo;
  static const int        dam_each [ ] =
  {
    0,
      4,   5,   8,   10,  13,        17,  21,  24,  27,  30,
      33,  37,  41,  45,  50,       55,  60,  65,  70,  75,
      80,  85,  90,  95, 100,       102, 104, 106, 108, 110,
      112, 114, 116, 118, 120,      122, 124, 126, 128, 130,
      132, 134, 136, 138, 140,      142, 144, 146, 148, 150,

      // Added by SinaC 2000

      152, 154, 156, 158, 160,      162, 164, 166, 168, 170,
      172, 174, 176, 178, 180,      182, 184, 186, 188, 190,
      192, 194, 196, 198, 200,      202, 204, 206, 208, 210,
      212, 214, 216, 218, 220,      222, 224, 226, 228, 230,
      232, 234, 236, 238, 240,      242, 244, 246, 248, 250

      };
  int        dam;

  level    = UMIN( level, ( int)(sizeof( dam_each )/sizeof( dam_each[0] ) - 1));
  level    = UMAX( 0, level );
  dam      = number_range( dam_each[level], dam_each[level] * 4 );
  if ( saves_spell( level, victim, DAM_ENERGY ) )
    dam /= 2;
  // Added by SinaC 2001
  else if ( number_percent() < level ) {
    OBJ_DATA *obj_next;
    for ( OBJ_DATA *obj_lose = victim->carrying; obj_lose; obj_lose = obj_next ) {
      obj_next = obj_lose->next_content;

      if ( number_bits( 3 ) != 0 
	   || IS_SET(obj_lose->extra_flags, ITEM_NOCOND) )
	continue;

      act( "$p explodes!",      victim, obj_lose, NULL, TO_CHAR );
      act( "$n's $p explodes!", victim, obj_lose, NULL, TO_ROOM );
      extract_obj( obj_lose ) ;
    }
  }
  ability_damage( ch, victim, dam, sn, DAM_ENERGY,TRUE, TRUE );
  return;
}

void power_disintegrate ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA  *obj_lose;
  OBJ_DATA  *obj_next;

  /* can't disintegrate a player but can disintegrate their equipement
  if (!IS_NPC(victim)) {
    send_to_char("Disintegrate a player, hmm.. nah, forget it!\n\r",ch);
    return;
  }
  */

  if ( number_percent() < level 
       && !saves_spell( level, victim, DAM_OTHER ) )
    for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next ) {
      obj_next = obj_lose->next_content;

      if ( number_bits( 2 ) != 0  // check_immune_obj added by SinaC 2003
	   || IS_SET(obj_lose->extra_flags, ITEM_NOCOND)
	   || check_immune_obj(obj_lose, DAM_OTHER) )
	continue;

      act( "$p disintegrates!",      victim, obj_lose, NULL, TO_CHAR );
      act( "$n's $p disintegrates!", victim, obj_lose, NULL, TO_ROOM );
      extract_obj( obj_lose ) ;
    }

  if ( !saves_spell( level, victim, DAM_OTHER ) 
       && IS_NPC(ch) ) {
    /*
     * Disintegrate char, do not generate a corpse, do not
     * give experience for kill.  Extract_char will take care
     * of items carried/wielded by victim.  Needless to say,
     * it would be bad to be a target of this spell!
     * --- Thelonius (Monk)
     */
    act( "You have DISINTEGRATED $N!",         ch, NULL, victim, TO_CHAR );
    act( "You have been DISINTEGRATED by $n!", ch, NULL, victim, TO_VICT );
    act( "$n's spell DISINTEGRATES $N!",       ch, NULL, victim, TO_ROOM );

    if ( IS_NPC( victim ) )
      extract_char( victim, TRUE );
    else
      extract_char( victim, FALSE );
  }
  return;
}

void power_displacement ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("Your form already shiver.\n\r", ch );
    else
      act("$N's form already shivers.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,allAC,ADD,4-level,level-4,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level-4,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,allAC,ADD,4-level);
  affect_to_char( victim, &af );

  send_to_char( "Your form shimmers, and you appear displaced.\n\r",
		victim );
  act( "$N shimmers and appears in a different location.",
       ch, NULL, victim, TO_NOTVICT );
  return;
}

void power_domination ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_safe(ch,victim)) return;

  if ( victim == ch ) {
    send_to_char( "Dominate yourself?  You're weird.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED( victim, AFF_CHARM )
       || IS_AFFECTED( ch,  AFF_CHARM )
       || level < victim->level
       || IS_SET(victim->cstat(imm_flags),IRV_CHARM)
       || saves_spell( level, victim, DAM_CHARM ) ) {
    send_to_char("Power failed.\n\r", ch );
    return;
  }

  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );
  // Added by SinaC 2000
  victim->leader = ch;

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 4 ),level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( victim, &af );

  act( "Your will dominates $N!", ch, NULL, victim, TO_CHAR );
  act( "Your will is dominated by $n!", ch, NULL, victim, TO_VICT );
  return;
}

void power_ectoplasmic_form ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_PASS_DOOR ) ) {
    send_to_char( "You are already as ectoplasmic as you could be.\n\r" ,ch);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_PASS_DOOR,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 4 ),level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_PASS_DOOR);
  affect_to_char( victim, &af );

  send_to_char( "You turn translucent.\n\r", victim );
  act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_ego_whip ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || saves_spell( level, victim, DAM_MENTAL ) ) {
    send_to_char("Power failed.\n\r", ch );
    return;
  }

  //afsetup(af,CHAR,hitroll,ADD,-2,level,level,sn);
  //affect_to_char( victim, &af );
  //af.location  = ATTR_saving_throw;
  //af.modifier  = 2;
  //affect_to_char( victim, &af );
  //af.location  = ATTR_allAC;
  //af.modifier  = level / 2;
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,-2);
  addaff(af,CHAR,allAC,ADD,level/2);
  addaff(af,CHAR,saving_throw,ADD,2);
  affect_to_char( victim, &af );

  act( "You ridicule $N about $S childhood.", ch, NULL, victim, TO_CHAR    );
  send_to_char( "Your ego takes a beating.\n\r", victim );
  act( "$N's ego is crushed by $n!",          ch, NULL, victim, TO_NOTVICT );

  return;
}

void power_energy_containment ( int sn, int level, CHAR_DATA *ch, void *vo, int target) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You already absorb some forms of energy.\n\r",ch);
    else
      act("$N already absorbs some forms of energy.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,saving_throw,ADD,0-level/5,level/2+7,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,7+level/2,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,saving_throw,ADD,0-level/5);
  affect_to_char( victim, &af );

  send_to_char( "You can now absorb some forms of energy.\n\r", ch );
  return;
}

void power_enhanced_strength ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You can't get any stronger.\n\r", ch );
    else
      act("$N can't get any stronger.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  //afsetup(af,CHAR,STR,ADD,1 + ( level >= 15 ) + ( level >= 25 ) + (level >= 32), level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,STR,ADD,1 + ( level >= 15 ) + ( level >= 25 ) + (level >= 32));
  affect_to_char( victim, &af );
  
  send_to_char( "You are HUGE!\n\r", victim );
  return;
}

void power_flesh_armor ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("Your flesh is already turned to steel.\n\r", ch );
    else
      act("$N's flesh is already turned to steel.\n\r", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,allAC,ADD,-40,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,allAC,ADD,-40);
  affect_to_char( victim, &af );

  send_to_char( "Your flesh turns to steel.\n\r", victim );
  act( "$N's flesh turns to steel.", ch, NULL, victim, TO_NOTVICT);
  return;
}

void power_intellect_fortress ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *gch;
  AFFECT_DATA af;

  // Modified by SinaC 2000
  //afsetup(af,CHAR,allAC,ADD,-40,24,level,sn);
  //afsetup(af,CHAR,allAC,ADD,-40,UMAX(1,level-10),level,sn);
  createaff(af,UMAX(1,level-10),level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,allAC,ADD,-40);

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    if ( !is_same_group( gch, ch ) 
	 || is_affected( gch, sn ) )
      continue;

    send_to_char( "A virtual fortress forms around you.\n\r", gch );
    act( "A virtual fortress forms around $N.", gch, NULL, gch, TO_ROOM );

    affect_to_char( gch, &af );
  }
  return;
}

void power_mental_barrier ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("You have already a mental barrier around yourself.\n\r", ch );
    else
      act("$N has already a mental barrier around $Mself.", ch, NULL, victim, TO_CHAR );
    return;
  }

  // Modified by SinaC 2000
  //afsetup(af,CHAR,allAC,ADD,-20, 24,level,sn);
  //afsetup(af,CHAR,allAC,ADD,-20, level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,allAC,ADD,-20);
  affect_to_char( victim, &af );

  send_to_char( "You erect a mental barrier around yourself.\n\r",
		victim );
  return;
}

void power_mind_thrust ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  ability_damage( ch, (CHAR_DATA *) vo, dice( 1, 10 ) + level / 2, sn, DAM_MENTAL,TRUE, TRUE );
  return;
}

void power_project_force ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  ability_damage( ch, (CHAR_DATA *) vo, dice( 4, 6 ) + level, sn, DAM_BASH,TRUE, TRUE );
  return;
}

void power_neuro_blast ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *victim       = (CHAR_DATA *) vo;
  static const int        dam_each [ ] =
  {
    0,
      4,   5,   7,   8,   9,        10,  12,  15,  18,  21,
      24,  27,  30,  33,  37,       41,  45,  50,  55,  60,
      64,  68,  72,  76,  80,       82,  84,  86,  88,  90,
      92,  94,  96,  98, 100,       102, 104, 106, 108, 110,
      112, 114, 116, 118, 120,      122, 124, 126, 128, 130,

      132, 134, 136, 138, 140,      142, 144, 146, 148, 150,
      152, 154, 156, 158, 160,      162, 164, 166, 168, 170,
      172, 174, 176, 178, 180,      182, 184, 186, 188, 190,
      192, 194, 196, 198, 200,      202, 204, 206, 208, 210,
      212, 214, 216, 218, 220,      222, 224, 226, 228, 230
      };

  int        dam;

  level    = UMIN( level, ( int)(sizeof( dam_each )/sizeof( dam_each[0] ) - 1) );
  level    = UMAX( 0, level );
  // Modified by SinaC 2001  was /2,  *2 before
  dam      = number_range( dam_each[level], dam_each[level] * 3);
  if ( saves_spell( level, victim, DAM_ENERGY ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_ENERGY,TRUE, TRUE );
  return;
}

void power_neuro_crush ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  ability_damage( ch, (CHAR_DATA *) vo, dice( 3, 5 ) + level, sn, DAM_ENERGY,TRUE, TRUE );
  return;
}

void power_neuro_drain ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || saves_spell( level, victim, DAM_OTHER ) ) {
    send_to_char("Power failed.\n\r",ch);
    return;
  }

  //afsetup(af,CHAR,STR,ADD,-1 - ( level >= 10 ) - ( level >= 20 ) - ( level >= 30 ),level/2,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/2,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,STR,ADD,-1 - ( level >= 10 ) - ( level >= 20 ) - ( level >= 30 ));
  affect_to_char( victim, &af );

  send_to_char( "You feel drained.\n\r", victim );
  act( "$n appears drained of strength.", victim, NULL, NULL, TO_ROOM );
  return;
}

void power_neuro_healing ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice( 3, 6 ) + level*2;
  victim->hit = UMIN( victim->hit + heal, victim->cstat(max_hit) );
  update_pos( victim );

  send_to_char( "You feel better!\n\r", victim );
  return;
}

void power_share_strength ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( victim == ch ) {
    send_to_char( "You can't share strength with yourself.\n\r", ch );
    return;
  }
  if ( is_affected( victim, sn ) ) {
    act( "$N already shares someone's strength.", ch, NULL, victim,
	 TO_CHAR );
    return;
  }
  if ( ch->cstat(STR) <= 5 ) {
    send_to_char( "You are too weak to share your strength.\n\r", ch );
    return;
  }

  //afsetup(af,CHAR,STR,ADD,1 + ( level >= 20 ) + ( level >= 30 ),level,level,sn);
  //affect_to_char( victim, &af );
  //af.modifier  = -af.modifier;
  //affect_to_char( ch,     &af );
  int mod = 1 + ( level >= 10 ) + ( level >= 20 ) + ( level >= 30 );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,STR,ADD,mod);
  affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,STR,ADD,-mod);
  affect_to_char( ch, &af );

  act( "You share your strength with $N.", ch, NULL, victim, TO_CHAR );
  act( "$n shares $s strength with you.",  ch, NULL, victim, TO_VICT );
  return;
}

void power_thought_shield ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("You have already a shield around yourself.\n\r", ch );
    else
      act("$N has already a shield around $Mself.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,allAC,ADD,-20,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,allAC,ADD,-20);
  affect_to_char( victim, &af );

  send_to_char( "You have created a shield around yourself.\n\r", ch );
  return;
}

void power_ultrablast ( int sn, int level, CHAR_DATA *ch, void *vo, int target )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int        dam;
  int        hpch;

  for ( vch = ch->in_room->people; vch; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( vch != ch && !is_safe_spell(ch,vch,TRUE) && !is_same_group(ch,vch)) {
//      if ( IS_NPC( ch ) ? !IS_NPC( vch ) : IS_NPC( vch ) ) {
	hpch = UMAX( 10, ch->hit );
	// was hpch/8+1, hpch/4 before
	dam  = number_range( hpch / 6+1, hpch / 4 );
	if ( saves_spell( level, vch, DAM_MENTAL ) )
	  dam /= 2;
	ability_damage( ch, vch, dam, sn, DAM_MENTAL,TRUE, TRUE );
//      }
    }
  }
  return;
}

void power_raw_flesh( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (!is_made_flesh(victim)) {
    act("$N is not made of flesh.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if ( is_affected( victim, sn ) ) {
    act("$N has no flesh to rip off anymore.",ch,NULL,victim,TO_CHAR);
    return;
  }

  // Modified by SinaC 2001, was  level/2, 4  before
  int done = ability_damage( ch, victim, dice( level, 10), sn, DAM_MENTAL,TRUE, TRUE );

  if ( done == DAMAGE_DONE ) {
    //afsetup(af,CHAR,allAC,ADD,100,level,level,sn);
    //affect_to_char( victim, &af );
    createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,allAC,ADD,100);
    affect_to_char( victim, &af );

    send_to_char( "Your flesh is ripped from your body.\n\r", victim );
    act( "$n's flesh is ripped from $s body.", victim, NULL, NULL, TO_ROOM );
  }
  return;
}

void power_inflict_pain ( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  ability_damage( ch, (CHAR_DATA *) vo, dice( 2, 10 ) + level / 2, sn, DAM_MENTAL,TRUE, TRUE );
  return;
}


/* Original Code by Jason Huang (god@sure.net).                       */
/* Permission to use this code is granted provided this header is     */
/* retained and unaltered.                                            */ 
void power_mind_fear( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = ( CHAR_DATA *) vo;

  if (victim == ch
      || !victim->in_room
      // Modified by SinaC 2001
      || IS_SET( victim->in_room->cstat(flags), ROOM_SAFE      )
      || IS_SET( victim->in_room->cstat(flags), ROOM_PRIVATE   )
      || IS_SET( victim->in_room->cstat(flags), ROOM_SOLITARY  )
      || IS_SET( victim->in_room->cstat(flags), ROOM_NO_RECALL )
       // Added by SinaC 2001
      ||   IS_SET(victim->act, ACT_IS_SAFE )
      || victim->level >= level 
      || victim->in_room->area != ch->in_room->area
      || ( IS_NPC( victim ) && saves_spell( level, victim, DAM_BASH ) )
      || number_percent() < 50 ) {
    send_to_char( "Power failed.\n\r", ch );
    return;
  }

  do_flee( victim, "" );   
  return;
}

void power_telepathic_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  CHAR_DATA *victim;
  bool gate_pet;

  if ( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || victim->in_room == NULL
       || !can_see_room(ch,victim->in_room) 
       // Modified by SinaC 2001
       || IS_SET(victim->in_room->cstat(flags), ROOM_SAFE)
       || IS_SET(victim->in_room->cstat(flags), ROOM_PRIVATE)
       || IS_SET(victim->in_room->cstat(flags), ROOM_SOLITARY)
       || IS_SET(victim->in_room->cstat(flags), ROOM_NO_RECALL)
       || IS_SET(ch->in_room->cstat(flags), ROOM_NO_RECALL)
       // Added by SinaC 2001
       ||   IS_SET(victim->act, ACT_IS_SAFE )
       || victim->level >= level + 3
       || (is_clan(victim) && !is_same_clan(ch,victim))
       || (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
       || (IS_NPC(victim) && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       || (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) ) ) {
      send_to_char( "You failed.\n\r", ch );
      return;
    }	

  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;

  act("$n suddenly disappears.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You concentrate yourself on your target and disappear.\n\r",ch);
  char_from_room(ch);
  char_to_room(ch,victim->in_room);

  act("$n suddenly appears.",ch,NULL,NULL,TO_ROOM);
  do_look(ch,"auto");

  send_to_charf(ch,"You feel drained.\n\r"); // SinaC 2003
  ch->move = UMAX( 1, ch->move-250 );

  if (gate_pet) {
    act("$n suddenly disappears.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You suddenly disappear.\n\r",ch);
    char_from_room(ch->pet);
    char_to_room(ch->pet,victim->in_room);
    act("$n suddenly appears.",ch,NULL,NULL,TO_ROOM);
    do_look(ch->pet,"auto");
  }
}

// Added by SinaC 2001 for mental user
void power_endure( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  AFFECT_DATA af;

  if (is_affected(ch,sn)) {
    send_to_char("You already have the mental resolve to resist magic.\n\r",ch);
    return;
  }

  //afsetup( af, CHAR, saving_throw, ADD, -UMAX(3,level/10)+number_range(0,3), level, level, gsn_endure );
  //affect_to_char( ch, &af );
  //afsetup( af, CHAR, allAC, ADD, -UMAX(3,level/5)+number_range(0,level/5), level, level, gsn_endure );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,saving_throw,ADD,-UMAX(3,level/10)+number_range(0,3));
  addaff(af,CHAR,allAC,ADD,-UMAX(3,level/5)+number_range(0,level/5));
  affect_to_char( ch, &af );

  send_to_char("You build up the mental resolve to better resist magic.\n\r",ch);
  return;
}

// Modified by SinaC 2001 for mental user
void power_iron_hand( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  AFFECT_DATA af;
  int chance;
  
  if ( is_affected( ch, sn ) ) {
    send_to_char( "You already have your hands as hard as iron.\n\r", ch );
    return;
  }

  //afsetup( af, CHAR, NA, ADD, 0, level, level, gsn_iron_hand );
  //affect_to_char( ch, &af ),
  createaff(af,level,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( ch, &af );

  send_to_char("You concentrate yourself and your hands become as hard as iron.\n\r", ch );
  return;
}

void power_kihai( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  CHAR_DATA *victim = (CHAR_DATA*)vo;
  int success;

  // success
  act("You concentrate on $N's and screams 'Ki Hai'.", ch, NULL, victim, TO_CHAR );
  act("$n screams 'Ki Hai' focusing $N.", ch, NULL, victim, TO_NOTVICT );
  act("$n screams 'Ki Hai' focusing on you.", ch, NULL, victim, TO_VICT );

  if ( !IS_SET(victim->cstat(parts), PART_BRAINS) ) {
    send_to_char("Nothing seemed to happen.\n\r", ch );
    return;
  }

  success = get_ability( ch, sn );
  success += (ch->cstat(WIS)+ch->cstat(INT))-(victim->cstat(WIS)+victim->cstat(INT));
  success += (level - victim->level);
  success /= 4;

  switch( check_immune(victim, DAM_MENTAL) ) {
  case IS_IMMUNE:
    act("$N is unaffected by your mental attack.", ch, NULL, victim, TO_CHAR );
    return;
    break;
  case IS_RESISTANT:
    success /= 2;
    break;
  case IS_VULNERABLE:
    success *= 2;
    break;
  }

  if ( number_percent() < success ) {
    act("$N's brain has exploded.", ch, NULL, victim, TO_CHAR );
    send_to_char("Your brain seems to boil and suddenly explodes.\n\r", victim );

    SET_BIT( victim->cstat(parts), PART_BRAINS );

    sudden_death( ch, victim );
    check_improve(ch,sn,TRUE,2);
  }
  // fail
  else {
    // damage
    if ( chance(50) ) {
      send_to_char("You feel intense pain in your head.\n\r", victim );
      ability_damage(ch,victim,level*ch->cstat(WIS),sn,DAM_MENTAL,TRUE,FALSE);

      if ( !is_affected( ch, sn ) ) {
	AFFECT_DATA af;
	int lvl = level;

	//afsetup( af, CHAR, INT, ADD, -number_fuzzy(5), lvl/4, lvl, sn );
	//affect_to_char( victim, &af );
	//afsetup( af, CHAR, STR, ADD, -number_fuzzy(5), lvl/4, lvl, sn );
	//affect_to_char( victim, &af );
	//afsetup( af, CHAR, DEX, ADD, -number_fuzzy(5), lvl/4, lvl, sn );
	//affect_to_char( victim, &af );
	createaff(af,lvl/4,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
	addaff(af,CHAR,INT,ADD,0-number_fuzzy(5));
	addaff(af,CHAR,STR,ADD,0-number_fuzzy(5));
	addaff(af,CHAR,DEX,ADD,0-number_fuzzy(5));
	affect_to_char( victim, &af );

	DAZE_STATE(victim, PULSE_VIOLENCE);
	check_improve(ch,sn,TRUE,1);
      }
    }
    // no damage
    else {
      ability_damage(ch,victim,0,sn,DAM_MENTAL,TRUE,FALSE);
      check_improve(ch,sn,FALSE,1);
    }
    check_killer( ch, victim );
  }
}

// Count how many power_XXX_fist already enchant ch
int count_fist( CHAR_DATA *ch, const int whichOne ) {
  int count = 0;
  if ( is_affected( ch, whichOne ) )
    return -1;
  if ( is_affected( ch, gsn_energy_fist ) ) count++;
  if ( is_affected( ch, gsn_burning_fist ) ) count++;
  if ( is_affected( ch, gsn_icy_fist ) ) count++;
  if ( is_affected( ch, gsn_acid_fist ) ) count++;
  if ( is_affected( ch, gsn_draining_fist ) ) count++;
  return count;
}
// Remove all fists
void remove_fists( CHAR_DATA *ch ) {
  affect_strip( ch, gsn_energy_fist );
  affect_strip( ch, gsn_burning_fist );
  affect_strip( ch, gsn_icy_fist );
  affect_strip( ch, gsn_acid_fist );
  affect_strip( ch, gsn_draining_fist );
}
// Replace every power_XXX_fist
void enchant_fists( CHAR_DATA *ch, const int whichOne, const char *msg, int level ) {
  // Can use it only if unarmed and unarmored
  //  if ( !is_unarmed_and_unarmored(ch) ) {
  if ( !IS_UNARMED_UNARMORED(ch) ) { // SinaC 2003
    send_to_char("You must be unarmed and unarmored to use this power.\n\r", ch);
    return;
  }
  // If already enchanted by 'whichOne': failed
  int count = count_fist( ch, whichOne );
  if ( count == -1 ) {
    send_to_char("Your hands are already enchanted.\n\r",ch);
    return;
  }
  // If already affected by power_XXX_fist
  if ( count > 0 ) {
    // Hands of multitude allow multiple power_XXX_fist at the same time
    int percent = get_ability( ch, gsn_hands_of_multitude );
    // miss
    if ( percent <= 0
	 || number_percent()*count > percent ) { // really hard if already many power_XXX_fist
      send_to_char("You failed.\n\r", ch );
      remove_fists( ch );
      check_improve( ch, gsn_hands_of_multitude, FALSE, 8 );
      return;
    }
    check_improve( ch, gsn_hands_of_multitude, TRUE, 6 );
  }
  AFFECT_DATA af;
  int dur = UMAX( number_fuzzy( level/(3+count) ), 4 ); // duration lower with the number of power_XXX_fist

  //afsetup( af, CHAR, NA, ADD, 0, dur, level, whichOne );
  //affect_to_char( ch, &af );
  createaff(af,dur,level,whichOne,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( ch, &af );

  send_to_char( msg, ch );
}

void power_energy_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  enchant_fists( ch, gsn_energy_fist, "Your hands are engulfed with energy sparks.\n\r", level );
  return;
}

void power_burning_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  enchant_fists( ch, gsn_burning_fist, "Your hands are engulfed with flames.\n\r", level );
  return;
}

void power_icy_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  enchant_fists( ch, gsn_icy_fist, "Your hands are engulfed with freezing ice.\n\r", level );
  return;
}

void power_acid_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  enchant_fists( ch, gsn_acid_fist, "Your hands are engulfed with acid.\n\r", level );
  return;
}

void power_draining_fist( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  enchant_fists( ch, gsn_draining_fist, "Your hands are engulfed with a dark aura.\n\r", level );
  return;
}

void power_inertial_barrier( int sn, int level, CHAR_DATA *ch, void *vo,int target ) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA*)vo;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_NEGATIVE)
       || IS_SET(victim->cstat(imm_flags),IRV_NEGATIVE)) {
    if (victim == ch)
      send_to_char("You are already resisting to negative energy.\n\r",ch);
    else
      act("$N is already resisting to negative energy.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_NEGATIVE, 24, level, sn );
  //affect_to_char( victim, &af );
  //afsetup(af, CHAR, affected_by, OR, AFF_PROTECT_EVIL, 24, level, sn );
  //affect_to_char( victim, &af );
  createaff(af,24,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,res_flags,OR,IRV_NEGATIVE);
  addaff(af,CHAR,affected_by,OR,AFF_PROTECT_EVIL);
  affect_to_char( victim, &af );

  send_to_char("You are not yet afraid by negative energy.\n\r",victim);
  if ( ch != victim )
    act("$N is not yet afraid by negative energy.",ch,NULL,victim,TO_CHAR);
}

/* Added by Sinac 2003 for regain power */
void power_regain_power( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
 
  victim->psp = UMIN( victim->psp+victim->psp/3, victim->cstat(max_psp) );
  
  send_to_char( "Wow ... What a rush !\n\r", victim );
  act( "$n glows with energy for a second.", victim, NULL, NULL, TO_ROOM );
  
  return;
}


//Syntax: psi 'ki pass' direction
//
//This skill allows a character to focus his or her ki, or internal life force
//energy, in order to allow the character to pass through solid doors.
//Ki-pass will work on most, but not all, closed doors.
void power_ki_pass( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  if ( target_name == NULL || target_name[0] == '\0' ) {
    send_to_charf(ch,"Which direction?\n\r");
    return;
  }

  // Find the exit
  ROOM_INDEX_DATA *pRoom = ch->in_room;
  if ( pRoom == NULL ) {
    send_to_charf(ch,"You are completly lost.\n\r");
    bug("power_ki_pass: [%s] [vnum: %d] room is NULL", NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
    return;
  }
  int door;
  if ( ( door = find_door( ch, target_name ) ) < 0 )
    return;
  EXIT_DATA *pExit = pRoom->exit[door];
  EXIT_DATA *pExitRev = NULL;
  ROOM_INDEX_DATA *toRoom;
  if (
      ( toRoom = pExit->u1.to_room ) == NULL
      || ( pExitRev = toRoom->exit[rev_dir[door]] ) == NULL
      || pExitRev->u1.to_room != pRoom
      || IS_SET( pExit->exit_info, EX_NOPASS ) // can't pass thru NOPASS exit
      ) {
    send_to_charf(ch,"You can't ki pass this door.\n\r");
    return;
  }
  if ( IS_SET( pExit->exit_info, EX_INFURIATING ) && chance(50) ) { // 50% chance to pass thru infuriating
    send_to_charf(ch,"You failed.\n\r");
    return;
  }
  // Okay, we have found a door, even not closed
  act("You focus you ki on $t.", ch, dir_name[door], NULL, TO_CHAR );
  move_char( ch, door, FALSE, FALSE, TRUE );
  return;
}

//Syntax: psi lifetouch
//Syntax: psi lifetouch <target>
//
//Lifetouch is a skill exclusive to the monk class.  It allows the monk to
//focus his or her life force energy, or ki, and convert same into healing
//power.  The monk may heal his or her own wounds, or the wounds of another
//character.  However, the healing lifetouch comes at a price -- the monk
//temporarily loses one point from his or her constitution stat every time
//this skill is used.  The more times the skill is used, the lower the monk's
//constitution will drop and the longer it will take for the affect to wear
//off.  Monks should be careful in their use of this skill -- letting one's
//constitution score drop too low can be deadly.
//~
void power_lifetouch( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  // check constitution
  if ( ch->cstat(CON) <= 3 ) {
    send_to_charf(ch,"Your constitution is too low.\n\r");
    return;
  }

  // find affect
  AFFECT_DATA *af = affect_find( ch->affected, sn );
  if ( af == NULL ) { // no affect found: create one
    af = new_affect();
    createaff(*af,level/10,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(*af,CHAR,CON,ADD,-1);
    affect_to_char( victim, af );
  }
  else { // affect found
    int dur = af->duration;
    AFFECT_LIST *laf = af->list;
    while ( laf->location != ATTR_CON ) // find constitution affect
      laf = laf->next;
    if ( laf == NULL || laf->location != ATTR_CON ) { // not found, create one
      addaff(*af,CHAR,CON,ADD,-1);
      dur = 10;
    }
    else { // found
      laf->modifier -= 1; // constitution modifier increases
      dur = -laf->modifier * 10;
    }
    af->duration = dur; // increase duration
    recompute(ch); // need to recompute because we have modify an affect without calling affect_to_char
  }

  // heal victim
  victim->hit = URANGE( victim->hit, victim->hit+level*5, victim->cstat(max_hit) );

  act("$n feels better.", victim, NULL, NULL, TO_ROOM );
  send_to_charf(victim,"You feel better.\n\r");
  send_to_charf(ch,"You feel weaker.\n\r");
  return;
}


//DISRUPT               PSI 130
//Help Keywords : DISRUPT.
//Help Category : Miscellaneous.
//Related Helps : Dispel Magic, Cancellation.
//
//Syntax: Cast disrupt <Spellname>
//
//A more focused version of the somewhat chaotic Cancellation
//spell, Disrupt attempts to remove one particular spell affect that is
//present upon the target. If no target is specified, the caster is
//assumed to be trying to remove the spell from themselves.
void power_disrupt( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  if ( target_name == NULL || target_name[0] == '\0' ) {
    send_to_charf(ch,"You have to specify an affect.\n\r");
    return;
  }
  int sn2find = ability_lookup( target_name );
  if ( sn2find < 0 ) {
    send_to_charf(ch,"Affect not found.\n\r");
    return;
  }
  const char *abilityName = ability_table[sn2find].name;
  bool done = FALSE;
  bool found = FALSE;
  AFFECT_DATA *fromAffect = ch->affected;
  while (1) {
    AFFECT_DATA *paf = affect_find( fromAffect, sn2find );
    if ( paf == NULL ) // no more affect: leave loop
      break;
    if ( IS_SET( paf->flags, AFFECT_NON_DISPELLABLE ) ) { // not dispellable, check next affect
      fromAffect = paf->next;
      found = TRUE;
    }
    else { // dispellable: dispel and leave loop
      affect_remove( ch, paf );
      done = TRUE;
      break;
    }
  }
  if ( done )
    send_to_charf(ch,"You disrupt %s.\n\r", abilityName );
  else if ( found )
    send_to_charf(ch,"You failed to disrupt %s.\n\r", abilityName );
  else
    send_to_charf(ch,"Affect not found.\n\r");
  return;
}


//WARMTH                PSI 133
//Help Keywords : 'WARMTH'.
//Help Category : Protection.
//
//This spell gives its target resistance to cold based attacks such as
//chill touch, frost breath, ice storm and any cold based weapons.
void power_warmth( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_COLD)
       || IS_SET(victim->cstat(imm_flags),IRV_COLD)) {
    if ( ch == victim )
      send_to_char("You are already resisting to cold.\n\r",ch);
    else
      act("$N is already resisting to cold.", ch, NULL, victim, TO_CHAR );
    return;
  }

  createaff(af,level/10,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_COLD);
  affect_to_char( victim, &af );

  send_to_char("A warm feeling passes through you.\n\r", victim );
  return;
}

//NEUROMANCER
//
//TELEKINESIS           PSI 25
//Help Keywords : 'TELEKINESIS'.
//Help Category : Attack Spell.
//
//Syntax: cast 'telekinesis' <object>
//
//This spell allows the caster to mentally hurl an object at the being
//the caster is fighting.  The damage done depends on the weight of the
//object hurled.
void power_telekinesis( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = ch->fighting;
  if ( victim == NULL ) {
    send_to_charf(ch,"You must be fighting to use this psi power.\n\r");
    return;
  }
  OBJ_DATA *obj = get_obj_carry( ch, target_name, ch );
  if ( obj == NULL ) {
    send_to_charf(ch,"You can't find that item.\n\r");
    return;
  }
  
  act("$p is hurled at you.", ch, obj, victim, TO_VICT );
  act("You mentally hurl $p at $N.", ch, obj, victim, TO_CHAR );
  act("$p is hurled at $N.", ch, obj, victim, TO_NOTVICT );

  int dam = (obj->weight*obj->weight)+1;
  ability_damage(ch,victim,dam,sn,DAM_BASH,TRUE,TRUE);

  return;
}

//MIND OVER BODY         PSI 19
//Help Keywords : 'MIND OVER BODY'.
//Help Category : Survival.
//Related Helps : Sustenance.
//
//Syntax: cast 'mind over body'
//
//This allows the caster to fend off the need for food and drink
//with but a thought.
void power_mind_over_body( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  send_to_charf(ch,"Not Yet Implemented.\n\r");

  return;
}

//SOFTEN                 PSI 29
//Help Keywords : SOFTEN.
//Help Category : Malediction.
//
//This spell makes the targets armor soft and easy to penetrate.
void power_soften( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    send_to_charf(ch,"Power failed.\n\r");
    return;
  }

  if ( saves_spell( level, victim, DAM_MENTAL ) ) {
    send_to_charf(ch,"You failed.\n\r");
    return;
  }

  AFFECT_DATA af;
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,level*2);
  affect_to_char( victim, &af );

  act("$n makes your armor soft and easy to penetrate.", ch, NULL, victim, TO_VICT );
  act("You make $n's armor soft and easy to penetrate.", ch, NULL, victim, TO_CHAR );
  return;
}

//MIND FREEZE            PSI 29
//Help Keywords : 'MIND FREEZE'.
//Help Category : Attack Spell.
//Related Helps : Mind Thrust, Psychic Crush, Psionic Blast.
//
//As the psionicist gains more skill, they become adept at causing
//significant harm to their foe. This discipline enables them to capture
//the mind of their enemy, in a freezing, vice like mental grip, and
//simply squeeze, very, very hard.. 
void power_mind_freeze( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( !IS_SET( victim->cstat(parts), PART_BRAINS ) ) {
    send_to_charf(ch,"You failed.\n\r");
    return;
  }

  int dam = dice( level, 7 );
  if ( saves_spell( level, victim, DAM_MENTAL ) )
    dam /= 3;

  send_to_charf(victim,"You feel like your brain is squeezed by a mental grip.\n\r");
  act("You mentally squeeze $N's brains", ch, NULL, victim, TO_CHAR );

  ability_damage(ch,victim,dam,sn,DAM_MENTAL,TRUE,TRUE);
  return;
}

//ACCELERATE                 PSI 31
//Help Keywords : ACCELERATE.
//Help Category : Enhancement.
//Related Helps : Haste, Slow, Lightspeed.
//
//Syntax: cast Accelerate <Target>
//
//Premise:
//This Discipline stimulates the adrenal glands of the
//target, allowing them to move at great speed. As well as
//accelerating them, this will also increase the prowess
//of those it is used upon.
void power_accelerate( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  int casting_level = 1; // from 1 to 20: level 1   from 21 to 80: level 2  from 81 to 100: level 3
  if ( level > 20 )
    casting_level += 1;
  if ( level > 80 )
    casting_level += 1;

  if ( is_affected( victim, sn ) 
       || IS_AFFECTED(victim,AFF_HASTE)
       || IS_SET(victim->off_flags,OFF_FAST)) {
    if (victim == ch)
      send_to_char("You can't move any faster!\n\r",ch);
    else
      act("$N is already moving as fast as $E can.",
	  ch,NULL,victim,TO_CHAR);
    return;
  }

  if (IS_AFFECTED(victim,AFF_SLOW)) {
    if (!check_dispel(level,victim,gsn_slow)) {
      if (victim != ch)
	send_to_char("Spell failed.\n\r",ch);
      send_to_char("You feel momentarily faster.\n\r",victim);
      return;
    }
    act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
    return;
  }

  AFFECT_DATA af;
  createaff(af,victim == ch ? level/2 : level/4,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_HASTE);
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself accelerated.\n\r", victim );
  if ( ch != victim )
    act( "You stimulate $N's adrenaline glands.\n\r", ch, NULL, victim, TO_CHAR );
  act("$n is accelerated.",victim,NULL,NULL,TO_ROOM);
  return;
}

//NERVE SHOCK     PSI 39
//Help Keywords : 'NERVE SHOCK'.
//Help Category : Attack Spell.
//Related Helps : Acidic Touch.
//
//By focussing on the neural structure of those they face, the Psionicist
//who wields this powerful discipline can cause their nerves to fire
//neurons up to a crescendo which quite literally sends thousands of
//volts surging through their enemy. A nasty death, and a fine
//light show :)
void power_nerve_shock( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  int dam = dice( level, 12 );
  if ( saves_spell( level, victim, DAM_LIGHTNING ) )
    dam /= 3;

  act( "You focus on $N's neural structure.", ch, NULL, victim, TO_CHAR );
  send_to_charf(victim,"You feel eletric lightning passing through you.\n\r");
  ability_damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE,TRUE);
}

//PROBABILITY TRAVEL     PSI 47
//Help Keywords : 'PROBABILITY TRAVEL'.
//Help Category : Transport.
//Related Helps : Teleport.
//
//This spell allows the caster to use the minds power of random thought
//to transport the caster to a point of random origin.
void power_probability_travel( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  ROOM_INDEX_DATA *pRoomIndex;

  if ( IS_SET(ch->in_room->cstat(flags), ROOM_NO_RECALL)
       || ( !IS_NPC(ch) && ch->fighting != NULL ) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  pRoomIndex = get_random_room(ch);

  if ( ch->fighting != NULL )
    stop_fighting( ch, FALSE );

  act( "$n vanishes!", ch, NULL, NULL, TO_ROOM );
  char_from_room( ch );
  char_to_room( ch, pRoomIndex );
  act( "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
  do_look( ch, "auto" );

  MOBPROG(ch,NULL,"onMoved");
  return;
}

//CAUSE DECAY         PSI 49
//Help Keywords : 'CAUSE DECAY'.
//Help Category : Attack Spell.
//
//This spell has the ability to severely damage the target by decaying
//their flesh.
void power_cause_decay( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (!is_made_flesh(victim)) {
    act("$N is not made of flesh.",ch,NULL,victim,TO_CHAR);
    return;
  }

  int dam = dice( level, 13 );
  if ( saves_spell( level, victim, DAM_NEGATIVE ) )
    dam /= 3;

  ability_damage(ch,victim,dam,sn,DAM_NEGATIVE,TRUE,TRUE);
}
//
//ACIDIC TOUCH       PSI 55
//Help Keywords : 'ACIDIC TOUCH'.
//Help Category : Attack Spell.
//
//Syntax: cast 'acidic touch' <target>
//
//Premise:
//With this spell, the caster learns to exude acid from their very
//pores. Potent enchantment stops it hurting the caster, but any
//target unfortunate enough to get in its way is treated to a
//burning, corrosive attack upon their person.
void power_acidic_touch( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  int dam = dice( level, 13 );
  if ( saves_spell( level, victim, DAM_POISON ) ) {
    dam /= 3;
    level /= 2;
  }

  act("$n exude acid from $s pores.", ch, NULL, NULL, TO_ROOM );
  act("$n exude acid from $s pores and hits you.", ch, NULL, victim, TO_VICT );
  act("You exude acid from your pores and hits $N.", ch, NULL, victim, TO_CHAR );
  int done = ability_damage(ch,victim,dam,sn,DAM_POISON,TRUE,TRUE);
  if ( done == DAMAGE_DONE )
    acid_effect(victim,level,dam,TARGET_CHAR);
}

//
//NEURAL BURN    PSI 60
//Help Keywords : 'NEURAL BURN'.
//Help Category : Attack Spell.
//Related Helps : Control Flames, Pyromania, Agitation.
//
//Bringing the full force of their minds to bear, the Psionicist
//excites the molecules within the enemies head. This causes friction
//and a great deal of pain, just before flames erupt inside their
//skull. Guaranteed to give anyone a bad day.
void power_neural_burn( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  int dam = dice( level, 13 );
  if ( saves_spell( level, victim, DAM_FIRE ) )
    dam /= 3;

  int done = ability_damage(ch,victim,dam,sn,DAM_FIRE,TRUE,TRUE);
}

//ENHANCE ARMOR    PSI 68
//Help Keywords : 'ENHANCE ARMOR'.
//Help Category : Equipment.
//
//Syntax: cast 'enhance armor' <object>
//
//With this devotion, a psionicist can change the molecular makeup of
//objects, making them stronger and more durable.  This has obvious
//applications to armor, but if the armor is already magically enchanted,
//a psionicist cannot rearrange its molecules.
//
//Note: Due to their Divine Source, Blessed objects and artifacts
//may not be affected by this discipline.
void power_enhance_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  OBJ_DATA *obj = (OBJ_DATA *)vo;

  if ( obj->item_type != ITEM_ARMOR ) {
    send_to_charf(ch,"You can't use enhance armor on that type of item.");
    return;
  }

  if ( obj->enchanted ) {
    act("$p is already enchanted.", ch, obj, NULL, TO_CHAR );
    return;
  }

  if ( IS_OBJ_STAT( obj, ITEM_BLESS ) || IS_OBJ_STAT( obj, ITEM_UNIQUE ) ) {
    act("You can't use enhance armor on $p", ch, obj, NULL, TO_CHAR );
    return;
  }

  AFFECT_DATA *paf; 
  int result, fail;
  int hit_bonus, dam_bonus, added;
  bool hit_found = FALSE, dam_found = FALSE;

  int base_fail = 25;	// base 25% chance of failure
  int fact_fail = 2;    // faction factor
  int add_fail = 25;    // additional fail if not saves/hitroll/damroll
  int max_fail = 95;
  // this means they have no bonus
  hit_bonus = 0;
  dam_bonus = 0;
  fail = base_fail;

  // find the bonuses
  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
	if ( laf->location == ATTR_hitroll ) {
	  hit_bonus = laf->modifier;
	  hit_found = TRUE;
	  fail += fact_fail * (hit_bonus * hit_bonus);
	}
	else if (laf->location == ATTR_damroll ) {
	  dam_bonus = laf->modifier;
	  dam_found = TRUE;
	  fail += fact_fail * (dam_bonus * dam_bonus);
	}
	else  // things get a little harder
	  fail += add_fail;
 
  for ( paf = obj->affected; paf != NULL; paf = paf->next )
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
      if ( laf->location == ATTR_hitroll ) {
	hit_bonus = laf->modifier;
	hit_found = TRUE;
	fail += fact_fail * (hit_bonus * hit_bonus);
      }
      else if (laf->location == ATTR_damroll ) {
	dam_bonus = laf->modifier;
	dam_found = TRUE;
	fail += fact_fail * (dam_bonus * dam_bonus);
      }
      else // things get a little harder
	fail += add_fail;

  // apply other modifiers
  fail -= 3 * level/2;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,max_fail);

  result = number_percent();

  // the moment of truth
  if (result < (fail / 5)) { // item destroyed
    act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
    act("$p shivers violently and explodeds!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    return;
  }

  if (result < (fail / 2)) {// item disenchanted
    act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
    act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
    obj->enchanted = TRUE;

    //remove all affects
    obj->affected = NULL;

    //clear all flags
    REM_OBJ_STAT(obj,ITEM_GLOW|ITEM_HUM|ITEM_MAGIC|ITEM_BLESS);
    return;
  }

  if ( result <= fail ) { // failed, no bad result
    send_to_char("Nothing seemed to happen.\n\r",ch);
    return;
  }

  affect_enchant(obj);

  if (result <= (100 - level/5)) { // success!
    act("$p glows blue.",ch,obj,NULL,TO_CHAR);
    act("$p glows blue.",ch,obj,NULL,TO_ROOM);
    SET_OBJ_STAT(obj, ITEM_MAGIC);
    added = 1;
  }
    
  else { // exceptional enchant
    act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
    act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
    SET_OBJ_STAT(obj,ITEM_MAGIC|ITEM_GLOW);
    added = 2;
  }
		
  // now add the enchantments

  if (obj->level < LEVEL_HERO - 1)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

  AFFECT_DATA af;
  createaff(af,-1,level,sn,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,added);
  addaff(af,CHAR,damroll,ADD,added);
  affect_join_obj( obj, &af );
}

//TRAUMA                PSI 94
//Help Keywords : TRAUMA.
//Help Category : Attack Spell.
//Related Helps : Psychic Crush, Mind Thrust.
//
//By the use of the Trauma Discipline, the Psionicist is able to create
//temporary disruptions to their opponents muscle, nervous and circulatory
//system, mainly centered around the heart and brain. Such disruption
//inevitably causes great and lasting pain, and even death..
void power_trauma( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( !IS_SET( victim->cstat(parts), PART_HEART )
       || !IS_SET( victim->cstat(parts), PART_BRAINS ) ) {
    send_to_charf(ch,"Spell failed.\n\r");
    return;
  }

  int dam = dice( level, 15 );
  if ( saves_spell( level, victim, DAM_MENTAL ) )
    dam /= 3;

  // FIXME: sudden_death sometimes

  act("You create temporary disruptions to $N's muscles, nervous and circulatory system.", ch, NULL, victim, TO_CHAR );
  send_to_charf(victim,"You feel an intense and lasting pain in your brain.\n\r");
  int done = ability_damage(ch,victim,dam,sn,DAM_MENTAL,TRUE,TRUE);
  if ( done == DAMAGE_DONE ) {
    victim->stunned = 2;
    victim->daze = 2;
  }
}

//WILLPOWER            PSI 101
//Help Keywords : 'WILLPOWER'.
//Help Category : Protection.
//
//This spell gives its target additional resistance to mental attacks
//such as those used by psionic/undead enemies. 
void power_willpower( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  
  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_MENTAL)
       || IS_SET(victim->cstat(imm_flags),IRV_MENTAL)) {
    if ( ch == victim )
      send_to_char("You are already protected from mental.\n\r",ch);
    else
      act("$N is already protected from mental.", ch, NULL, victim, TO_CHAR );
    return;
  }

  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_MENTAL);
  affect_to_char( victim, &af );
  
  send_to_char( "You feel protected from mental.\n\r", victim );
  if ( victim != ch )
    send_to_char("Ok.\n\r", ch);
  return;
}

//WEB                  PSI 107
//Help Keywords : 'WEB'.
//Help Category : Malediction.
//Related Helps : Dissolve.
//
//Syntax: Cast web <target>
//
//Spellcasters and thieves can entangle an opponent in a magical web
//which completely prevents them from moving. A person or mob that
//is snared in a web cannot be summoned, they cannot gate, and they
//cannot recall. Magical portals may still be entered, but, the
//web will move with the victim.
//
//Notes:
//The success of a Web spell varies depending upon, among other things,
//the level of the caster and the target. 
void power_web( int sn, int level, CHAR_DATA *ch, void *vo, int target ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r"); // fixme: spell_web already exists
  return;
}

//PSYCHOSIS            PSI 108
//Help Keywords : PSYCHOSIS.
//Help Category : Attack Spell.
//Related Helps : Nightmare Touch, Mind Freeze.
//
//A Psionicist must undergo significant training before they are able
//to use this discipline. With it, they cause the deep seated hateful
//emotions in their opponent to surface, and attack the ego, causing great
//internal pain and conflict in its hapless victim.
//
//SPASM                PSI 117
//Help Keywords : SPASM.
//Help Category : Attack Spell.
//Related Helps : Ballistic Attack, Raw Flesh.
//
//Seizing control of the opponent's body for a few seconds, the Psionicist
//is able to use his power to force the enemies body to contort against
//itself. As the muscles are filled with rictus, they violently pull
//against flesh and break bone.
//
//
//ENERGY SHIELD         PSI 120
//Help Keywords : 'ENERGY SHIELD'.
//Help Category : Protection.
//
//This spell gives its target resistance to the effects of the
//destructive energy created by many explosives and other attacks.
//
//
//NIGHTMARE TOUCH       PSI 124
//Help Keywords : 'NIGHTMARE TOUCH'.
//Help Category : Attack Spell.
//Related Helps : Mind Thrust, Trauma, Psychosis.
//
//The focused use of this discipline is two-fold. Firstly, the Psionicist
//gains an understanding of his victim's worst nightmares, and then plunges
//the hapless foe directly into them. Screaming as their horror is
//manifests before them, they suffer great torment.
//
//
//KNOCK                 PSI 127
//Help Keywords : KNOCK.
//Help Category : Miscellaneous.
//Related Helps : Open, close, pass door, pick lock.
//
//Syntax: cast knock <dir>
//
//Premise:
//Some high level spellcasters can gain the ability to unlock doors by
//the command of their mind. It has been rumoured that even doors
//which are physically protected from picking can be unlocked by the
//power of this magic. The drawback to this spell is that it is
//completely ineffective when used on doors that are magically warded. 
//
//
//SOLIDIFY              PSI 136
//Help Keywords : SOLIDIFY.
//Help Category : Equipment.
//Related Helps : Detect invis, Illuminate.
//
//Syntax: Cast Solidify <item>
//
//High level mages and psionicists are able to manipulate the
//molecular structure of items they are carrying to make them
//more compact. The result of this spell is that items previously
//invisible can now be seen by the average eye.
//
//
//PANIC                 PSI 139
//Help Keywords : PANIC.
//Help Category : Attack Spell.
//Related Helps : Flee..
//
//Premise:
//A nasty trick to play on a room full of crowded mobs! Ever stand
//up in a busy mall and start shouting RUN! RUN! You can guarantee
//some people will flee the area even if they have no idea why :)
//
//
//HOLY MIRROR             PSI 142
//Help Keywords : 'HOLY MIRROR'.
//Help Category : Protection.
//
//This spell gives its target resistance to holy and light based attacks
//which includes colour spray, moonbeam, sunray, prismatic spray and any
//weapons based on light/holy damage.
//
//
//RESONATE                PSI 145
//Help Keywords : RESONATE.
//Help Category : Enhancement.
//
//Through the use of this Discipline, the psionicist is able to cause
//the molecules within an object of his choice to vibrate. The resultant
//resonant field makes the object easily detectable, even by touch alone.  
//
//
//
//LIGHTSPEED              PSI 149
//Help Keywords : 'LIGHTSPEED'.
//Help Category : Enhancement.
//Related Helps : Haste..
//
//This spell will give its target the ability to move at an extremely
//accelerated speed and allow them to gain an additional attack per round
//on top of that already given by standard haste. This spell has no
//effect on classes other than mage, cleric and psi even if taken as a
//potion or pill.
//
//
//NEURAL OVERLOAD         PSI 151
//Help Keywords : 'NEURAL OVERLOAD'.
//Help Category : Attack Spell.
//Related Helps : Mind Thrust, Neural Burn, Psychic Crush.
//
//The most extreme use of this discipline can quite literally cause the
//victim's brain to explode out of their ears. It works by building up
//pressure inside the skull, which must be expelled, somewhere. Invariably
//violent, and sometimes, very messy...
//
//
//SCRY                    PSI 154
//Help Keywords : 'SCRY'.
//Help Category : Detection.
//
//Using this ancient art, a high powered spellcaster can transport
//themselves to another, view their surroundings, and, return to their
//original location all within a single heartbeat. The target will
//sense a presence but that caster of this spell is so quick that
//they will never know who exactly was there.
//
//
//PYROMANIA               PSI 165
//Help Keywords : 'PYROMANIA'.
//Help Category : Attack Spell.
//
//High noble psionicists have evolved in dangerous times. Those wiser
//among them have acknowledged that there are times when the strong mental
//energy of their class is simply not enough. After successful
//experimentation with explosive magics in the form of detonation, the
//psionicist high council has prepared a spell which allows those preparing
//for its ranks to generate multiple explosions.
//
//
//
//DESOLATION              PSI 173
//Help Keywords : DESOLATION.
//Help Category : Attack Spell.
//Related Helps : Nightmare Touch, Psychosis.
//
//This Discipline is one of the most truly terrifying the Psionicist
//will learn. With it, he forces a link between a being's spirit, and
//the Realm of Entropy. The forces of decay and dissolution, flooding
//into a living body force it to twist and wither, before your eyes..
//
//
//CHAOS PORTAL            PSI 185
//Help Keywords : 'CHAOS PORTAL'.
//Help Category : Transport.
//Related Helps : Gate, Doorway.
//
//In ancient times when the world was far less stable and the Demonlord
//would often descend upon the cities of Aardwolf, an emergency escape
//plan was developed. This involved the creation of portals
//that could be handed out to the fleeing citizens of Aardwolf. Each portal
//would take its owner to a completely random location. While
//many had considerable misfortune in their destinations and were
//slain anyway, many others survived. The spreading of victims across
//the realm made it impossible for the Demonlord to slaughter everyone
//so at least some survived....
//
//In recent years, a council of high noble spellcasters have regained
//the knowledge of how to create such portals. They have had less
//success however in discovering the location of key ingredients needed
//to make these portals. The only clue is that certain key figures in
//the defense of Midgaard were said to be the keepers of the ingredients.
