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
#include "interp.h"
#include "classes.h"
#include "act_info.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "update.h"
#include "lookup.h"
#include "olc_value.h"
#include "act_comm.h"
#include "ability.h"
#include "group.h"
#include "utils.h"


// default casting rule is everything to level 1
casting_rule_type default_casting_rule = { 1, -1 };
// max casting rule is everything to level 5
casting_rule_type max_casting_rule = { 5, -1 };
// SinaC 2003: wild mage has a special casting rule: one to level 4 and other to level 3
casting_rule_type wild_magic_casting_rule = { 4, 3 };

// Added by SinaC 2001
// Count the number of spell/skill of specified casting level
int count_level( CHAR_DATA *ch, int level ) {
  int count = 0;

  for ( int i = 0; i < MAX_ABILITY; i++ ){
    if (ability_table[i].name == NULL)
      break;
    
    // Added by SinaC 2001 to avoid problem with raceskill with casting level > 1
    if ( get_raceability( ch, i ) )
      continue;

    if ( ch->pcdata->ability_info[i].casting_level == level ) count++;
  }

  return count;
}

// Count number of abilities with a casting level highest than param
int count_higher_level( CHAR_DATA *ch, int level ) {
  int count = 0;

  for ( int i = 0; i < MAX_ABILITY; i++ ){
    if (ability_table[i].name == NULL)
      break;
    // Added by SinaC 2001 to avoid problem with raceskill with casting level > 1
    if ( get_raceability( ch, i ) )
      continue;

    if ( ch->pcdata->ability_info[i].casting_level >= level ) count++;
  }

  return count;
}

// Check if we are already a master in a skill/spell
bool check_master( CHAR_DATA *ch, int sn ) {
  // we already have the maximum level in that skill/spell
  if ( ch->pcdata->ability_info[sn].casting_level >= ability_table[sn].nb_casting_level )
    return TRUE;

  return FALSE;
}

// Check if a player doesn't get too many high casting level spell
bool check_max_spell_level( CHAR_DATA *ch, int sn ) {
  // Modified by SinaC 2001
  /*
  // if we already have more than one spell level MAX-1, we can't have spell level MAX
  if ( count_level( ch, MAX_CASTING_LEVEL-1 ) > 1
       && ch->pcdata->ability_info[sn].casting_level == MAX_CASTING_LEVEL-1 ) 
    return TRUE;
  // if we already have a spell level MAX, other spells can only be level MAX-2
  if ( count_level( ch, MAX_CASTING_LEVEL ) > 0 
       && ch->pcdata->ability_info[sn].casting_level == MAX_CASTING_LEVEL-2 ) 
    return TRUE;

  return FALSE;
  */
/*
  switch ( class_max_casting_rule( ch ) ) {
  default:
    bug("check_max_spell_level: invalid max_casting_rule: %d",
	class_max_casting_rule( ch ) );
    return TRUE;
    break;
    // case -1
  case CASTING_RULE_SKILL_ALL_LVL5:
    // only skills can be learned at higher level
    if ( ability_table[sn].type != TYPE_SKILL
	 || ch->pcdata->ability_info[sn].casting_level >= 5 )
      return TRUE;
    break;
    // case 0
  case CASTING_RULE_ALL_LVL1:
    // no casting level > 1
    if ( ch->pcdata->ability_info[sn].casting_level >= 1 )
      return TRUE;
    break;
    //case 1:
  case CASTING_RULE_ALL_LVL3:
    // no casting level > 3
    if ( ch->pcdata->ability_info[sn].casting_level >= 3 )
      return TRUE;
    break;
    //case 2:
  case CASTING_RULE_ALL_LVL4:
    // no casting level > 4
    if ( ch->pcdata->ability_info[sn].casting_level >= 4 )
      return TRUE;
    break;
    //case 3:
  case CASTING_RULE_LVL5_OTHER_LVL3:
    // one to level 5 and other to level 3
    if ( count_level( ch, 4 ) > 0 
	 && ch->pcdata->ability_info[sn].casting_level == 3 )
      return TRUE;
    // if we already have a casting level 5, other spells can only be level 3
    if ( count_level( ch, 5 ) > 0
	 && ch->pcdata->ability_info[sn].casting_level >= 3 ) 
      return TRUE;
    break;
    //case 4:
  case CASTING_RULE_LVL5_OTHER_LVL4:
    // one to level 5 and other to level 4
    // if we already have a casting level 5, other spells can only be level 4
    if ( count_level( ch, 5 ) > 0 
	 && ch->pcdata->ability_info[sn].casting_level >= 4 ) 
    return TRUE;
    break;
    // Added by SinaC 2003
  case CASTING_RULE_LVL4_OTHER_LVL3:
    // one to level 4 and other to level 3
    if ( count_level( ch, 3 ) > 0 
	 && ch->pcdata->ability_info[sn].casting_level == 3 )
      return TRUE;
    // if we already have a casting level 4, other spells can only be level 3
    if ( count_level( ch, 4 ) > 0
	 && ch->pcdata->ability_info[sn].casting_level >= 3 ) 
      return TRUE;
    break;
    // special case for immortals, they can get everything at casting level 5
  case CASTING_RULE_ALL_LVL5:
  if ( ch->pcdata->ability_info[sn].casting_level >= 5 )
      return TRUE;
    break;
  }
  return FALSE;
*/
  casting_rule_type rule = classes_casting_rule( ch, sn );
  int c = ch->pcdata->ability_info[sn].casting_level;
  // other == -1: every abilities at highest level
  if ( rule.other == -1 ) {
    if ( c >= rule.highest )
      return TRUE;
  }
  // other != 1: one ability at highest level, others abilities to other level
  else {
    // if we already have abilities higher level than .other+1
    //   and ability to test is at .other  then  we cannot train ability to test to a higher level
    if ( c == rule.other 
	 && count_higher_level( ch, rule.other+1 ) > 0 )
      return TRUE;
    // if we already have an ability to .higher
    //  and ability to test is >= .other  then  we cannot train ability to test to a higher level
    if ( c >= rule.other
	 && count_level( ch, rule.highest ) > 0 )
      return FALSE;
  }
  return FALSE;
}

int check_gain_prereq( CHAR_DATA *ch, int sn ) {
  ability_type *sk = &( ability_table[sn] );
  if ( sk->prereqs == NULL )
    return ERR_OK;
  int wanted_lvl = sk->nb_casting_level == 0 ? 0 : ch->pcdata->ability_info[sn].casting_level+1;
  if ( wanted_lvl > sk->nb_casting_level ) // should never happen
    return ERR_ALREADY_MASTER;
  PREREQ_DATA *p = &(sk->prereqs[wanted_lvl]);

  if ( p->plr_level > ch->level ) // SinaC 2003
    return ERR_LEVEL_TOO_LOW;

  if ( !( p->classes & ch->bstat(classes) ) ) // SinaC 2003
    return ERR_CANT_LEARN;

  for ( int i = 0; i < p->nb_prereq; i++ ) {
    PREREQ_LIST *l = &(p->prereq[i]);
    int psn = l->sn;
    int slevel = ch->pcdata->ability_info[psn].casting_level;
    if ( ch->pcdata->ability_info[psn].learned <= 0
	 || slevel < l->casting_level )
      return ERR_NOT_FIT_PREREQ;
  }
  return ERR_OK;
//  ability_type *sk = &( ability_table[sn] );
//  int wanted_lvl = ch->pcdata->ability_info[sn].casting_level+1;
//  
//  // skill/spell has level but no prereqs
//  if ( ability_table[sn].prereqs == NULL )
//    return TRUE;
//  
//  // now we check if the player has the prereqs
//  for ( int i = 0; i < sk->nb_casting_level+1; i++ ) {
//    PREREQ_DATA *p = &( sk->prereqs[i] );
//    // we check only the level we want to gain, actual+1
//    //if ( wanted_lvl != p->casting_level ) 
//    if ( wanted_lvl != i )  // SinaC 2003
//      continue;
//
//    // check each prereq for that level
//    for ( int j = 0; j < p->nb_prereq; j++ ) {
//      PREREQ_LIST *l = &( p->prereq[j] );
//
//      //int psn = skill_lookup( l->name );
//      int psn = l->sn;
//      int slevel = ch->pcdata->ability_info[psn].casting_level==0?1:ch->pcdata->ability_info[psn].casting_level;
//
//      if ( ch->pcdata->ability_info[psn].learned <= 0
//	   || slevel < l->casting_level )
//	return FALSE;
//    }
//  }
//  return TRUE;
}

// Added by SinaC 2001
bool can_gain_creation( CHAR_DATA *ch, int sn ) {
  if ( ch->pcdata->ability_info[sn].learned != 0 ) {
    return FALSE;
  }

  // Modified by SinaC 2003
  if ( class_gainabilityrating( ch, sn, 1 ) > 0 )
    // player fits prereq ?
    if ( check_gain_prereq( ch, sn ) == ERR_OK )
      return TRUE;
    else
      return FALSE;

  return FALSE;
}

int can_gain( CHAR_DATA *ch, int sn ){
  // god, race or clan skill
  if ( get_clanability( ch, sn )
       /*|| get_godskill( ch, sn )   removed by SinaC 2003*/
       || get_raceability( ch, sn ) )
    return ERR_GOD_CLAN_RACE;

  // skill/spell not known
  if ( ch->pcdata->ability_info[sn].learned == 0 ) {
    // player is allowed to learn that spell/skill
    // Modified by SinaC 2003
    if ( class_gainabilityrating( ch, sn, 1 ) > 0 )
      // player higher level enough ?, Modified by SinaC 2001
      // REMOVED by SinaC 2003, player can gain first casting level of an ability even if they are
      //  too low level but they can't gain higher ability casting level
      //if ( ch->level < class_abilitylevel( /*ch->cstat(classes)*/ch, sn ) )
      //return ERR_LEVEL_TOO_LOW;
      //else
	return check_gain_prereq( ch, sn );
    else
      return ERR_CANT_LEARN;
  }
  // skill/spell already known
  else {
    // so people who have learn the 1st level with a item giving the right class
    //  can't learn the next level of that spell
    // or class which got it from a sphere
    // or for immortals who have 'set skill <self> all perc 100'
    // Modified by SinaC 2003
    if ( class_gainabilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) <= 0 )
      //return ERR_CANT_LEARN;
      return ERR_STUDIED_SPELL;

// WAS BEFORE THE PREVIOUS TEST, SinaC 2001

    // was after the next test
    // no more than 1 level for that spell
    if ( ability_table[sn].nb_casting_level <= 1 )
      return ERR_SPELL_NO_LEVEL;
    // player higher level enough ?, Modified by SinaC 2001
    if ( ch->level < class_abilitylevel( /*ch->cstat(classes)*/ch, sn ) )
      return ERR_LEVEL_TOO_LOW;

    // already master at that skill/spell
    if ( check_master( ch, sn ) )
      return ERR_ALREADY_MASTER;
    // already have enough more than level 1 spell
    if ( check_max_spell_level( ch, sn ) )
      return ERR_ALREADY_MAX_LEVEL;
    // does player has 90% in that skill/spell
    if ( ch->pcdata->ability_info[sn].learned < 90 )
      return ERR_NOT_ENOUGH_PERC;
    return check_gain_prereq( ch, sn );
  }
}

// New do_gain version by SinaC 2001
void do_gain( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *trainer;
  int gn = 0, sn = 0;
  int col;
  bool found;

  if (IS_NPC(ch))
    return;

  //log_stringf("%s ==> %d",NAME(ch),class_max_casting_rule( ch ));

  // find a trainer
  for ( trainer = ch->in_room->people; trainer != NULL; trainer = trainer->next_in_room)
    if (IS_NPC(trainer) && IS_SET(trainer->act,ACT_GAIN))
      break;
  
  if (trainer == NULL || !can_see(ch,trainer)) {
    send_to_char("You can't do that here.\n\r",ch);
    return;
  }

  one_argument(argument,arg);

  if (arg[0] == '\0') {
    do_say(trainer,"Pardon me?");
    return;
  }

  if (!str_prefix(arg,"list")) {

    send_to_char("This command has been modified.(check help about gain)\n\r"
		 "Use  gain skills\n\r"
		 "     gain spells\n\r"
		 "     gain powers\n\r"
		 "     gain songs\n\r"
		 "     gain groups  instead.\n\r", ch );
    return;
  }

  if (!str_prefix(arg,"groups")){
    
    col = 0;
    
    sprintf(buf, 
	    "{W%-20s %-2s %-20s %-2s %-20s %-2s{x\n\r"
	    "----------------------- ----------------------- -----------------------\n\r",
	    "group","tp","group","tp","group","tp");
    send_to_char(buf,ch);

    found = FALSE;
    for (gn = 0; gn < MAX_GROUP; gn++) {
      if (!ch->pcdata->group_known[gn]
	  && class_grouprating(ch->bstat(classes),gn) > 0 
	  // Added by SinaC 2003
	  && god_grouprating(ch,gn) > 0 ) {
	found = TRUE;
	sprintf(buf,"{y%-20s {c%-2d{x ",
		group_table[gn].name,class_grouprating(ch->bstat(classes),gn));
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	  send_to_char("\n\r",ch);
      }
    }
    if (col % 3 != 0)
      send_to_char("\n\r",ch);
    if (!found)
      send_to_char("You can't learn any groups.\n\r",ch);
    return;
  }

  if (!str_prefix(arg,"skills")){

    col = 0;
    
    sprintf(buf, 
	    "{W%-20s %-2s %-3s  %-20s %-2s %-3s{x\n\r"
	    "---------------------------  ---------------------------\n\r",
	    "skill","tp","lvl","skill","tp","lvl");
    send_to_char(buf,ch);

    found = FALSE;
    for (sn = 0; sn < MAX_ABILITY; sn++) {
      if (ability_table[sn].name == NULL)
	break;
   
      /* Modified by SinaC 2001
      // Modified by SinaC 2000
      if ( ( !ch->pcdata->ability_info[sn].learned || check_prereq(ch,sn) )
	  &&  class_abilityrating(ch,sn) > 0
	  &&  ability_table[sn].spell_fun == spell_null)
      */
      if ( can_gain( ch, sn ) == ERR_OK 
	   // Modified by SinaC 2001
	   //&& ability_table[sn].spell_fun == spell_null ) {
	   && ability_table[sn].type == TYPE_SKILL ) {
	found = TRUE;
	if ( ability_table[sn].nb_casting_level > 0 )
	  sprintf(buf,"{y%-20s {c%-2d {g%3d{x  ",
		  ability_table[sn].name,
		  // Modified by SinaC 2003
		  class_abilityrating(ch,sn,ch->pcdata->ability_info[sn].casting_level+1),
		  ch->pcdata->ability_info[sn].casting_level+1);
	else
	  sprintf(buf,"{y%-20s {c%-2d{x      ",
		  ability_table[sn].name,
		  // Modified by SinaC 2003
		  class_abilityrating(ch,sn, 1) );
	send_to_char(buf,ch);
	if (++col % 2 == 0)
	  send_to_char("\n\r",ch);
      }
    }
    if (col % 2 != 0)
      send_to_char("\n\r",ch);
    if (!found)
      send_to_char("You can't learn any skills.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if (!str_prefix(arg,"spells")){

    col = 0;

    sprintf(buf, 
	    "{W%-20s %-2s %-3s  %-20s %-2s %-3s{x\n\r"
	    "---------------------------  ---------------------------\n\r",
	    "spell","tp","lvl","spell","tp","lvl");    
    send_to_char(buf,ch);
    
    found = FALSE;
    for (sn = 0; sn < MAX_ABILITY; sn++) {
      if (ability_table[sn].name == NULL)
	break;

      /* Modified by SinaC 2001
      // Modified by SinaC 2000
      if ( ( !ch->pcdata->ability_info[sn].learned || check_prereq(ch,sn) )
	  &&  class_abilityrating(ch,sn) > 0
	  &&  ability_table[sn].spell_fun != spell_null) 
      */
      if ( can_gain( ch, sn ) == ERR_OK 
	   //&& ability_table[sn].spell_fun != spell_null 
	   // Modified by SinaC 2001 for mental user
	   && ability_table[sn].type == TYPE_SPELL ) {
	found = TRUE;
	if ( ability_table[sn].nb_casting_level > 0 )
	  sprintf(buf,"{y%-20s {c%-2d {g%3d{x  ",
		  ability_table[sn].name,
		  // Modified by SinaC 2003
		  class_abilityrating(ch,sn,ch->pcdata->ability_info[sn].casting_level+1),
		  ch->pcdata->ability_info[sn].casting_level+1);
	else
	  sprintf(buf,"{y%-20s {c%-2d{x      ",
		  ability_table[sn].name,
		  // Modified by SinaC 2003
		  class_abilityrating(ch,sn,1) );
	send_to_char(buf,ch);
	if (++col % 2 == 0)
	  send_to_char("\n\r",ch);
      }
    }
    if (col % 2 != 0)
      send_to_char("\n\r",ch);
    if (!found)
      send_to_char("You can't learn any spells.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if (!str_prefix(arg,"powers")){

    col = 0;

    sprintf(buf, 
	    "{W%-20s %-2s %-3s  %-20s %-2s %-3s{x\n\r"
	    "---------------------------  ---------------------------\n\r",
	    "power","tp","lvl","power","tp","lvl");    
    send_to_char(buf,ch);
    
    found = FALSE;
    for (sn = 0; sn < MAX_ABILITY; sn++) {
      if (ability_table[sn].name == NULL)
	break;

      /* Modified by SinaC 2001
      // Modified by SinaC 2000
      if ( ( !ch->pcdata->ability_info[sn].learned || check_prereq(ch,sn) )
	  &&  class_abilityrating(ch,sn) > 0
	  &&  ability_table[sn].spell_fun != spell_null) 
      */
      if ( can_gain( ch, sn ) == ERR_OK 
	   //&& ability_table[sn].spell_fun != spell_null 
	   // Modified by SinaC 2001 for mental user
	   && ability_table[sn].type == TYPE_POWER ) {
	found = TRUE;
	if ( ability_table[sn].nb_casting_level > 0 )
	  sprintf(buf,"{y%-20s {c%-2d {g%3d{x  ",
		  ability_table[sn].name,
		  // Modified by SinaC 2003
		  class_abilityrating(ch,sn,ch->pcdata->ability_info[sn].casting_level+1),
		  ch->pcdata->ability_info[sn].casting_level+1);
	else
	  sprintf(buf,"{y%-20s {c%-2d{x      ",
		  ability_table[sn].name,
		  // Modified by SinaC 2003
		  class_abilityrating(ch,sn,1) );
	send_to_char(buf,ch);
	if (++col % 2 == 0)
	  send_to_char("\n\r",ch);
      }
    }
    if (col % 2 != 0)
      send_to_char("\n\r",ch);
    if (!found)
      send_to_char("You can't learn any powers.\n\r",ch);
    return;
  }

  // Added by SinaC 2003 for bard (songs)
  if (!str_prefix(arg,"songs")){

    col = 0;

    sprintf(buf, 
	    "{W%-20s %-2s %-3s  %-20s %-2s %-3s{x\n\r"
	    "---------------------------  ---------------------------\n\r",
	    "song","tp","lvl","song","tp","lvl");    
    send_to_char(buf,ch);
    
    found = FALSE;
    for (sn = 0; sn < MAX_ABILITY; sn++) {
      if (ability_table[sn].name == NULL)
	break;
      if ( can_gain( ch, sn ) == ERR_OK 
	   && ability_table[sn].type == TYPE_SONG ) {
	found = TRUE;
	if ( ability_table[sn].nb_casting_level > 0 )
	  sprintf(buf,"{y%-20s {c%-2d {g%3d{x  ",
		  ability_table[sn].name,
		  class_abilityrating(ch,sn,ch->pcdata->ability_info[sn].casting_level+1),
		  ch->pcdata->ability_info[sn].casting_level+1);
	else
	  sprintf(buf,"{y%-20s {c%-2d{x      ",
		  ability_table[sn].name,
		  class_abilityrating(ch,sn,1) );
	send_to_char(buf,ch);
	if (++col % 2 == 0)
	  send_to_char("\n\r",ch);
      }
    }
    if (col % 2 != 0)
      send_to_char("\n\r",ch);
    if (!found)
      send_to_char("You can't learn any songs.\n\r",ch);
    return;
  }

  
  if (!str_prefix(arg,"convert")) {
    if (ch->practice < 10) {
      act("$N tells you 'You are not yet ready.'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }
    
    act("$N helps you apply your practice to training.",
	ch,NULL,trainer,TO_CHAR);
    ch->practice -= 10;
    ch->train +=1 ;
    return;
  }
  // Added by SinaC 2000
  if (!str_prefix(arg,"revert")) {
    if (ch->train < 1 )	{
      act("$N tells you 'You are not yet ready.'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }
    
    act("$N helps you apply your train to practicing.",
	ch,NULL,trainer,TO_CHAR);
    ch->practice += 10;
    ch->train -=1 ;
    return;
  }
  
  if (!str_prefix(arg,"points")) {

    //
    //send_to_char("This option has been removed for the moment!\n\r",ch);
   // return;
    //

    if (ch->train < 2) {
      act("$N tells you 'You are not yet ready.'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }
    
    if (ch->pcdata->points <= 40) {
      act("$N tells you 'There would be no point in that.'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }
    
    act("$N trains you, and you feel more at ease with your skills.",
	ch,NULL,trainer,TO_CHAR);
    
    ch->train -= 2;
    ch->pcdata->points -= 1;
    // modified by SinaC 2000, Modified again by SinaC 2000
    //   xp = ch->exp - (ch-level * exp_per_level(ch,ch->pcdata->points);
    //   ^^^  before the points are changed..
    //   and after the new ch->exp is calculated.. add this
    //   ch->exp += xp;
    
    ch->exp = exp_per_level(ch,ch->pcdata->points) * ch->level;
    //
    //int xp;
    //
    //xp = ch->exp - (ch->level*exp_per_level(ch,ch->pcdata->points));
    //ch->exp += xp;
    //
    return;
  }

  // else add a group/skill

  gn = group_lookup(argument);
  if (gn > 0) {
    if (ch->pcdata->group_known[gn]) {
      act("$N tells you 'You already know that group!'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }
    
    if (class_grouprating( ch->bstat(classes), gn) <= 0) {
      act("$N tells you 'That group is beyond your powers.'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }
    
    // Added by SinaC 2003
    if ( god_grouprating(ch,gn) <= 0 ) {
      act("$N tells you 'That group can only be learned by follower of a certain god.'",
	  ch, NULL, trainer, TO_CHAR );
      return;
    }

    if (ch->train < class_grouprating(ch->bstat(classes),gn) ) {
      act("$N tells you 'You are not yet ready for that group.'",
	  ch,NULL,trainer,TO_CHAR);
      return;
    }

    // add the group 
    gn_add(ch,gn);
    act("$N trains you in the art of '$t'",
	ch,group_table[gn].name,trainer,TO_CHAR);
    ch->train -= class_grouprating(ch->bstat(classes),gn);
    return;
  }
  
  sn = ability_lookup(argument);
  if (sn > -1) {
    // Modified by SinaC 2000, spells can be gained without learning the full group
    //if (ability_table[sn].spell_fun != spell_null)
    //  {
    //    act("$N tells you 'You must learn the full group.'",
    //        ch,NULL,trainer,TO_CHAR);
    //    return;
    //  }
    //
    char sname[MAX_INPUT_LENGTH];
    char sname_level_current[MAX_INPUT_LENGTH];
    char sname_level_next[MAX_INPUT_LENGTH];

    sprintf( sname, 
	     "'%s'", 
	     ability_table[sn].name );
    if ( ch->pcdata->ability_info[sn].casting_level == 0 ) {
      sprintf( sname_level_current,
	       "'%s'",
	       ability_table[sn].name );
      sprintf( sname_level_next,
	       "'%s'",
	       ability_table[sn].name );
    }
    else {
      sprintf( sname_level_current,
	       "'%s (level %d)'",
	       ability_table[sn].name,
	       ch->pcdata->ability_info[sn].casting_level);
      sprintf( sname_level_next,
	       "'%s (level %d)'",
	       ability_table[sn].name,
	       ch->pcdata->ability_info[sn].casting_level+1);
    }

    int can_gain_code = can_gain( ch, sn );

    if ( can_gain_code != ERR_OK ) {
      switch( can_gain_code ) {
	// Added by SinaC 2003
      case ERR_STUDIED_SPELL:
	sprintf( buf, "$N tells you 'You can't get a higher level in %s'",
		 sname );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_LEVEL_TOO_LOW:
	sprintf( buf, "$N tells you 'You aren't level high enough to learn %s.'",
		sname );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_GOD_CLAN_RACE:
	sprintf( buf, "$N tells you '%s is already a special ability for you.'",
		sname );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_NOT_FIT_PREREQ:
	sprintf( buf, "$N tells you 'You don't fit prereqs to learn %s.'",
		sname_level_next );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_CANT_LEARN:
	sprintf( buf, "$N tells you 'You can't learn %s.'",
		sname );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_SPELL_NO_LEVEL:
	sprintf( buf, "$N tells you 'You already known %s.'",
		sname );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_ALREADY_MASTER:
	sprintf( buf, "$N tells you 'You already are a master of %s.'",
		sname_level_current );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_ALREADY_MAX_LEVEL:
	sprintf( buf, "$N tells you 'You can't learn that many high level skills/spells." );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      case ERR_NOT_ENOUGH_PERC:
	sprintf( buf, "$N tells you 'You haven't practiced enough %s.",
		sname_level_current );
	act( buf, ch, NULL, trainer, TO_CHAR );
	break;
      default:
	act( "DO_GAIN BUG: Warn an immortal", ch, NULL, trainer, TO_CHAR );
	break;
      }
      return;
    }


    // Modified by SinaC 2003
    if ( ch->train < class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level+1 ) ) {
      if ( ability_table[sn].nb_casting_level <= 1 )
	sprintf( buf, "$N tells you 'You don't have enough train sessions to gain %s.",
		 sname );
      else
	sprintf( buf, "$N tells you 'You don't have enough train sessions to gain %s.",
		 sname_level_next );
      act( buf, ch, NULL, trainer, TO_CHAR );
      return;
    }
   
    // Modified by SinaC 2003
    // add the skill
    ch->train -= class_abilityrating(ch,sn,ch->pcdata->ability_info[sn].casting_level+1);

    ch->pcdata->ability_info[sn].learned = 1;
    if ( ability_table[sn].nb_casting_level != 0 )
      ch->pcdata->ability_info[sn].casting_level++;
    if ( ability_table[sn].nb_casting_level <= 1 )
      act("$N trains you in the art of $t",
	  ch,sname,trainer,TO_CHAR);
    else
      act("$N trains you in the art of $t",
	  ch,sname_level_next,trainer,TO_CHAR);
    return;
  }
  
  act("$N tells you 'I do not understand...'",ch,NULL,trainer,TO_CHAR);
}

/* shows skills, groups and costs (only if not bought) */
void list_group_costs(CHAR_DATA *ch) {
  char buf[100];
  int gn,sn,col;

  if (IS_NPC(ch))
    return;

  col = 0;
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x\n\r",
	  "group","cp","group","cp","group","cp");
  send_to_char(buf,ch);

  for (gn = 0; gn < MAX_GROUP; gn++) {
    // Removed by SinaC 2000
    //if (group_table[gn].name == NULL)
    //  break;

    if (!ch->gen_data->group_chosen[gn]
	&&  !ch->pcdata->group_known[gn]
	&&  class_grouprating( ch->bstat(classes), gn ) > 0 
	// Added by SinaC 2003
	&& god_grouprating( ch, gn ) > 0 ) {
      sprintf(buf,"{y%-20s {c%-2d{x ",group_table[gn].name,
	      class_grouprating( ch->bstat(classes), gn ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  col = 0;
  
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x\n\r",
	  "skill","cp","skill","cp","skill","cp");
  send_to_char(buf,ch);
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL)
      break;
    
    // Modified by SinaC 2000, again by SinaC 2001 for god skill
    if (!ch->gen_data->ability_chosen[sn]
	&& ch->pcdata->ability_info[sn].learned == 0
	&& !get_raceability( ch, sn )
	&& !get_clanability( ch, sn )
	/*&& !get_godskill( ch, sn )  removed by SinaC 2003*/
	// Modified by SinaC 2001
	//&&  ability_table[sn].spell_fun == spell_null
	&&  ability_table[sn].type == TYPE_SKILL
	//&&  class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) > 0
	// Added by SinaC 2001
	&& can_gain_creation( ch, sn ) ) {
      sprintf(buf,"{y%-20s {c%-2d{x ",ability_table[sn].name,
	      // Modified by SinaC 2003
	      class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->pcdata->points);
  send_to_char(buf,ch);
  sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	  exp_per_level(ch,ch->gen_data->points_chosen));
  send_to_char(buf,ch);
  return;
}


void list_group_chosen(CHAR_DATA *ch) {
  char buf[100];
  int gn,sn,col;
 
  if (IS_NPC(ch))
    return;
 
  col = 0;
 
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x",
	  "group","cp","group","cp","group","cp\n\r");
  send_to_char(buf,ch);
 
  for (gn = 0; gn < MAX_GROUP; gn++) {
    // Removed by SinaC 2000
    //if (group_table[gn].name == NULL)
    //  break;
    
    // Modified by SinaC 2000 || ch->pcdata
    if ( (ch->gen_data->group_chosen[gn] || ch->pcdata->group_known[gn] )
	 &&  class_grouprating( ch->bstat(classes), gn ) > 0
	 // Added by SinaC 2003
	 && god_grouprating( ch, gn ) > 0 ) {
      sprintf(buf,"{y%-20s {c%-2d{x ",group_table[gn].name,
	      class_grouprating( ch->bstat(classes), gn ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  col = 0;
  
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x",
	  "skill","cp","skill","cp","skill","cp\n\r");
  send_to_char(buf,ch);
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL)
      break;
    
    // Modified by SinaC 2000 || get_race, ...
    if ((ch->gen_data->ability_chosen[sn] /*|| get_skill(ch,sn) > 0*/ )
	// Modified by SinaC 2003
	&&  class_gainabilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) > 0
	// Added by SinaC 2000, modified by SinaC 2001 for god skill
	&& !(get_raceability( ch, sn ) 
	     || get_clanability( ch, sn )
	     /*|| get_godskill( ch, sn )   removed by SinaC 2003*/ ) ) {
      sprintf(buf,"{y%-20s {c%-2d{x ",ability_table[sn].name,
	      // Modified by SinaC 2003
	      class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->gen_data->points_chosen);
  send_to_char(buf,ch);
  sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	  exp_per_level(ch,ch->gen_data->points_chosen));
  send_to_char(buf,ch);
  return;
}

// Added by SinaC 2000
void list_group_known(CHAR_DATA *ch ) {
  char buf[100];
  int gn,sn,col;
 
  if (IS_NPC(ch))
    return;
 
  col = 0;
 
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x",
	  "group","cp","group","cp","group","cp\n\r");
  send_to_char(buf,ch);

  for (gn = 0; gn < MAX_GROUP; gn++) {
/*
    log_stringf("name: %s  chosen: %s  known:%s  rating:%d",
		group_table[gn].name,
		ch->gen_data->group_chosen[gn]?"TRUE":"FALSE",
		ch->pcdata->group_known[gn]?"TRUE":"FALSE",
		class_grouprating( ch, gn ) );
*/
    if ( (ch->gen_data->group_chosen[gn] || ch->pcdata->group_known[gn] ) ) {

      if ( class_grouprating( ch->bstat(classes), gn ) > 0  )
	sprintf(buf,"{y%-20s {c%-2d{x ",group_table[gn].name,
	      class_grouprating( ch->bstat(classes), gn ));
      else
	sprintf(buf,"{y%-20s {c%-2s{x ",group_table[gn].name,
	      "*");
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);

  col = 0;
  
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x",
	  "ability","cp","ability","cp","ability","cp\n\r");
  send_to_char(buf,ch);
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL)
      break;
    
    // Modified by SinaC 2000 || ch->pcdata
    // Modified by SinaC 2001 for god skill
    if ((ch->gen_data->ability_chosen[sn] || ch->pcdata->ability_info[sn].learned > 0 
	 || get_raceability( ch, sn ) 
	 || get_clanability( ch, sn )
	 /*|| get_godskill( ch, sn )   removed by SinaC 2003*/  ) ) {
      if ( get_raceability( ch, sn ) 
	   || get_clanability( ch, sn ) 
	   /*|| get_godskill( ch, sn )  removed by SinaC 2003*/ )
	sprintf(buf,"{y%-20s {c%-2s{x ",ability_table[sn].name,
		"*");
      else
	sprintf(buf,"{y%-20s {c%-2d{x ",ability_table[sn].name,
		class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);

  send_to_char("(*) means no creation points: race ability or basic group\n\r",ch);
  
  sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->gen_data->points_chosen);
  send_to_char(buf,ch);
  sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	  exp_per_level(ch,ch->gen_data->points_chosen));
  send_to_char(buf,ch);
  return;
}

// Added by SinaC 2001 so spells can be gained one by one and not only by group
void list_spell_costs(CHAR_DATA *ch) {
  char buf[100];
  int sn,col;

  if (IS_NPC(ch))
    return;
 
  col = 0;
  
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x\n\r",
	  "spell","cp","spell","cp","spell","cp");
  send_to_char(buf,ch);
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL)
      break;
    
    // Modified by SinaC 2000, again by SinaC 2001 for god skill
    if (!ch->gen_data->ability_chosen[sn]
	&& ch->pcdata->ability_info[sn].learned == 0
	&& !get_raceability( ch, sn )
	&& !get_clanability( ch, sn )
	/*&& !get_godskill( ch, sn )  removed by SinaC 2003*/
	//&& ability_table[sn].spell_fun != spell_null
	// Added by SinaC 2001 for mental power
	&& ability_table[sn].type == TYPE_SPELL
	//&& class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) > 0
	// Added by SinaC 2001
	&& can_gain_creation( ch, sn ) ) {
	sprintf(buf,"{y%-20s {c%-2d{x ",
		ability_table[sn].name,
		class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->pcdata->points);
  send_to_char(buf,ch);
  sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	  exp_per_level(ch,ch->gen_data->points_chosen));
  send_to_char(buf,ch);
  return;
}

// Added by SinaC 2001 so powers can be gained one by one and not only by group
void list_power_costs(CHAR_DATA *ch) {
  char buf[100];
  int sn,col;

  if (IS_NPC(ch))
    return;
 
  col = 0;
  
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x\n\r",
	  "power","cp","power","cp","power","cp");
  send_to_char(buf,ch);
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL)
      break;
    
    // Modified by SinaC 2000, again by SinaC 2001 for god skill
    if (!ch->gen_data->ability_chosen[sn]
	&& ch->pcdata->ability_info[sn].learned == 0
	&& !get_raceability( ch, sn )
	&& !get_clanability( ch, sn )
	/*&& !get_godskill( ch, sn )  removed by SinaC 2003*/
	//&& ability_table[sn].spell_fun != spell_null
	// Added by SinaC 2001 for mental power
	&& ability_table[sn].type == TYPE_POWER
	//&& class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) > 0
	// Added by SinaC 2001
	&& can_gain_creation( ch, sn ) ) {
	sprintf(buf,"{y%-20s {c%-2d{x ",
		ability_table[sn].name,
		class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->pcdata->points);
  send_to_char(buf,ch);
  sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	  exp_per_level(ch,ch->gen_data->points_chosen));
  send_to_char(buf,ch);
  return;
}

// Added by SinaC 2003 for bard (songs)
void list_song_costs(CHAR_DATA *ch) {
  char buf[100];
  int sn,col;

  if (IS_NPC(ch))
    return;
 
  col = 0;
  
  sprintf(buf,"{W%-20s %-2s %-20s %-2s %-20s %-2s{x\n\r",
	  "song","cp","song","cp","song","cp");
  send_to_char(buf,ch);
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL)
      break;
    
    if (!ch->gen_data->ability_chosen[sn]
	&& ch->pcdata->ability_info[sn].learned == 0
	&& !get_raceability( ch, sn )
	&& !get_clanability( ch, sn )
	/*&& !get_godskill( ch, sn )  removed by SinaC 2003*/
	&& ability_table[sn].type == TYPE_SONG
	&& can_gain_creation( ch, sn ) ) {
	sprintf(buf,"{y%-20s {c%-2d{x ",
		ability_table[sn].name,
		class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ));
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  send_to_char("\n\r",ch);
  
  sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->pcdata->points);
  send_to_char(buf,ch);
  sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	  exp_per_level(ch,ch->gen_data->points_chosen));
  send_to_char(buf,ch);
  return;
}

int exp_per_level(CHAR_DATA *ch, int points) {
  int expl,inc;

  if (IS_NPC(ch))
    return 1000; 

  // Modified by SinaC 2001,  bstat  is a better idea than  cstat  I think
  expl = pc_race_table[ch->bstat(race)].expl;
  //expl = 1000;
  inc = 500;

  if (points < 40)
    // Modified by SinaC 2001
    return expl;
    //return (expl * class_mult( ch )/100 );
    //return (1000 * class_mult( ch )/100 );

  // processing
  points -= 40;

  while (points > 9) {
    expl += inc;
    points -= 10;
    if (points > 9) {
      expl += inc;
      inc *= 2;
      points -= 10;
    }
  }
  
  expl += points * inc / 10;  
  
  // Modified by SinaC 2001
  expl *= class_count(ch->bstat(classes));

  // Modified by SinaC 2001
  //return (expl * class_mult( ch )/100);
  return expl;
}

// Modified by SinaC 2000, 2001 and 2003 for bard (songs)
/* this procedure handles the input parsing for the skill generator */
bool parse_gen_groups(CHAR_DATA *ch,const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  int gn,sn,i;

  if (argument[0] == '\0')
    return FALSE;

  argument = one_argument(argument,arg);

  if (!str_prefix(arg,"help")) {
    if (argument[0] == '\0') {
      do_help(ch,"group help");
      return TRUE;
    }
    
    do_help(ch,argument);
    return TRUE;
  }

  // Removed by SinaC 2001, replaced with help
  /*
  // Added by Sinac 1997
  if (!str_prefix(arg,"level")) {
    do_level(ch,argument);
    return TRUE;
  }
  */

  // Added by SinaC 2001, so spells can be gained one by one not only by group
  if (!str_prefix(arg,"spell")) {
    list_spell_costs(ch);
    return TRUE;
  }

  // Added by SinaC 2001, so powers can be gained one by one not only by group
  if (!str_prefix(arg,"power")) {
    list_power_costs(ch);
    return TRUE;
  }

  // Added by SinaC 2003
  if (!str_prefix(arg,"song")) {
    list_song_costs(ch);
    return TRUE;
  }

  if (!str_prefix(arg,"add")) {
    if (argument[0] == '\0') {
      send_to_char("You must provide a skill/spell or group name.\n\r",ch);
      return TRUE;
    }
    
    gn = group_lookup(argument);
    if (gn != -1) {
      if (ch->gen_data->group_chosen[gn]
	  ||  ch->pcdata->group_known[gn]) {
	send_to_char("You already know that group!\n\r",ch);
	return TRUE;
      }
      
      if (class_grouprating( ch->bstat(classes), gn ) <= 0
	  || god_grouprating( ch, gn ) <= 0 ) {
	send_to_char("That group is not available.\n\r",ch);
	return TRUE;
      }
    
      sprintf(buf,"{W%s group added.{x\n\r",group_table[gn].name);
      send_to_char(buf,ch);
      ch->gen_data->group_chosen[gn] = TRUE;
      ch->gen_data->points_chosen += class_grouprating( ch->bstat(classes), gn );
      gn_add(ch,gn);
      ch->pcdata->points += class_grouprating( ch->bstat(classes), gn );
      return TRUE;
    }
    
    sn = ability_lookup(argument);
    if (sn != -1) {
      // Modified by SinaC 2001 for god skill
      if (ch->gen_data->ability_chosen[sn]
	  || get_raceability( ch, sn )
	  || get_clanability( ch, sn )
	  /*|| get_godskill( ch, sn )  removed by SinaC 2003*/
	  ||  ch->pcdata->ability_info[sn].learned > 0) {
	send_to_char("You already know that skill/spell/power/song!\n\r",ch);
	return TRUE;
      }
      
      // Modified by SinaC 2003
      if (class_gainabilityrating(ch, sn, ch->pcdata->ability_info[sn].casting_level ) <= 0
	  // Removed by SinaC 2001 so spells can be gained one by one
	  //  not only by group
	  /*||  ability_table[sn].spell_fun != spell_null*/) {
	send_to_char("That skill/spell/power/song is not available.\n\r",ch);
	return TRUE;
      }

      // Added by SinaC 2001
      if ( !can_gain_creation( ch, sn ) ) {
 	send_to_charf( ch, "You don't fit the prereqs for %s.\n\r",
		       ability_table[sn].name);
	return TRUE;
      }


      sprintf(buf, "{W%s skill/spell/power/song added.{x\n\r",ability_table[sn].name);
      send_to_char(buf,ch);
      ch->gen_data->ability_chosen[sn] = TRUE;
      ch->gen_data->points_chosen += class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level );
      //ch->pcdata->ability_info[sn].learned = 1;  SinaC 2003, casting level need to be set
      group_add( ch, ability_table[sn].name, FALSE );
      ch->pcdata->points += class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level );
      return TRUE;
    }
    
    send_to_char("No skills/spells/powers/songs or groups by that name...\n\r",ch);
    return TRUE;
  }
  
  if (!str_cmp(arg,"drop")) {
    if (argument[0] == '\0') {
      send_to_char("You must provide a skill/spell/power/song or group to drop.\n\r",ch);
      return TRUE;
    }

    gn = group_lookup(argument);
    if (gn != -1 && ch->gen_data->group_chosen[gn]) {
      send_to_char("{WGroup dropped.{x\n\r",ch);
      ch->gen_data->group_chosen[gn] = FALSE;
      ch->gen_data->points_chosen -= class_grouprating( ch->bstat(classes), gn );
      gn_remove(ch,gn);
      for (i = 0; i < MAX_GROUP; i++) {
	if (ch->gen_data->group_chosen[gn])
	  gn_add(ch,gn);
      }
      ch->pcdata->points -= class_grouprating( ch->bstat(classes), gn );
      // To avoid losing basic abilities which would be included in the group we just removed
      //  we re-add basics
      group_add( ch, class_table[ class_firstclass( ch->bstat(classes) ) ].base_group, FALSE );
      return TRUE;
    }
    
    sn = ability_lookup(argument);
    if (sn != -1 && ch->gen_data->ability_chosen[sn]) {
      send_to_char("{WSkill/spell/power/song dropped.{x\n\r",ch);
      ch->gen_data->ability_chosen[sn] = FALSE;
      ch->gen_data->points_chosen -= class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level );
      ch->pcdata->ability_info[sn].learned = 0;
      ch->pcdata->points -= class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level );
      return TRUE;
    }
    
    send_to_char("You haven't bought any such skill/spell/power/song or group.\n\r",ch);
    return TRUE;
  }
  
  if (!str_prefix(arg,"premise")) {
    do_help(ch,"premise");
    return TRUE;
  }
  
  if (!str_prefix(arg,"list")) {
    list_group_costs(ch);
    return TRUE;
  }
  
  if (!str_prefix(arg,"learned")) {
    list_group_chosen(ch);
    return TRUE;
  }

  // Added by SinaC 2000
  if (!str_prefix(arg,"known")){
    list_group_known(ch);
  }
  
  if (!str_prefix(arg,"info")) {
    do_groups(ch,argument);
    return TRUE;
  }

  return FALSE;
}


// Added by SinaC 2003
int find_ability( CHAR_DATA *ch, const char *name, const int type ) {
  /* finds an ability the character can cast/psi/use/song if possible */

  //if (IS_NPC(ch))
  //  return ability_lookup(name);

  int found = -1;
  for ( int sn = 0; sn < MAX_ABILITY; sn++ ) {
    if (ability_table[sn].name == NULL)
      break;
	  
    if ( ability_table[sn].type != type )
      continue;

    if (LOWER(name[0]) == LOWER(ability_table[sn].name[0])
	&&  !str_prefix(name,ability_table[sn].name)) {
      if ( found == -1)
	found = sn;
      /* Modified by Sinac 1997 */
      if (get_ability( ch, sn )>0)
	return sn;
    }
  }
  return found;
}

// Added by SinaC 2001 so, people can teach skills/spells to other
void do_teach( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  int sn;

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );

  if ( IS_NPC(ch)
       && ( IS_AFFECTED( ch, AFF_CHARM ) 
	    || ch->master != NULL
	    || ch->leader != NULL ) ) {
    send_to_char("You can't teach anything.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC( victim ) ) {
    send_to_char("NPC's are dumb, they can't learn anything.\n\r", ch );
    return;
  }

  if ( ( sn = ability_lookup( arg ) ) <= 0 ) {
    send_to_char("That skill/spell doesn't exist!\n\r", ch );
    return;
  }

  if ( get_ability( ch, sn ) < 95 ) {
    send_to_charf( ch,
		   "You haven't practiced '%s' enough to teach it.\n\r", 
		   ability_table[sn].name );
    return;
  }

  // get_ability_simple cos' we don't want to take care of victim's level
  if ( get_ability_simple( victim, sn ) > 0 ) {
    act("$N already knows '$t'.", ch, ability_table[sn].name, victim, TO_CHAR );
    return;
  }

  victim->pcdata->ability_info[sn].learned = 1;
  victim->pcdata->ability_info[sn].casting_level = ability_table[sn].nb_casting_level > 0 ? 1 : 0;
  victim->pcdata->ability_info[sn].level = URANGE( 1, victim->level, 100 );

  act( "$n teachs '$t' to $N.",
       ch, ability_table[sn].name, victim, TO_NOTVICT );
  act( "You teach '$t' to $N.",
       ch, ability_table[sn].name, victim, TO_CHAR );
  act( "$N teachs you '$t'.",
       victim, ability_table[sn].name, ch, TO_CHAR );
}



//******************************************************************************************
//********************************** NEW ABILITY.C *****************************************
//******************************************************************************************

int can_gain_ability( CHAR_DATA *ch, const int sn, const bool creation ) {
  int learned = get_ability( ch, sn );
  int casting = get_casting_level( ch, sn );
  //int max_casting_rule = class_max_casting_rule( ch );


  // to be continued
  return 0;
}

int can_gain_group( CHAR_DATA *ch, const int sn, const bool creation ) {
  // to be done
  return 0;  
}

/* checks for skill improvement */
void check_improve( CHAR_DATA *ch, int sn, bool success, int multiplier ) {
  int chance;
  char buf[100];

  if (IS_NPC(ch))
    return;

  // Modified by SinaC 2001
  if (ch->level < class_abilitylevel( /*ch->cstat(classes)*/ch, sn )
      // Modified by SinaC 2003
      ||  class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) == 0
      ||  ch->pcdata->ability_info[sn].learned == 0
      ||  ch->pcdata->ability_info[sn].learned == 100)
    return;  /* skill is not known */

  /* check to see if the character has a chance to learn */
  chance = 10 * int_app[ch->cstat(INT)].learn;
  chance /= ( multiplier
	      *	class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level )
	      *	4 );
  chance += ch->level;

  if (number_range(1,1000) > chance)
    return;

  /* now that the character has a CHANCE to learn, see if they really have */

  bool done = FALSE; // Added by SinaC 2003
  if (success) {
    chance = URANGE(5,100 - ch->pcdata->ability_info[sn].learned, 95);
    if (number_percent() < chance) {
      sprintf(buf,"You have become better at %s!\n\r",
	      ability_table[sn].name);
      send_to_char(buf,ch);
      ch->pcdata->ability_info[sn].learned++;
      gain_exp(ch,2 * class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level), TRUE);
      done = TRUE;
    }
  }
  else {
    chance = URANGE(5,ch->pcdata->ability_info[sn].learned/2,30);
    if (number_percent() < chance) {
      sprintf(buf,
	      "You learn from your mistakes, and your %s skill improves.\n\r",
	      ability_table[sn].name);
      send_to_char(buf,ch);
      ch->pcdata->ability_info[sn].learned += number_range(1,3);
      ch->pcdata->ability_info[sn].learned = UMIN(ch->pcdata->ability_info[sn].learned,100);
      gain_exp(ch,2 * class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level), TRUE);
      done = TRUE;
    }
  }
  if ( done && ch->pcdata->ability_info[sn].learned == 100 ) // Added by SinaC 2003
    send_to_charf(ch,"You are now master in '%s'.\n\r", ability_table[sn].name );
}
