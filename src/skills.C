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
#include "interp.h"

// Added by SinaC 2001
#include "classes.h"
#include "act_info.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "update.h"
#include "skills.h"
#include "lookup.h"
#include "olc_value.h"
#include "const.h"
#include "act_comm.h"
#include "act_obj.h"
#include "ability.h"
#include "config.h"


void do_skills(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char arg[MAX_INPUT_LENGTH];
  /* char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
     char skill_columns[LEVEL_HERO + 1];*/
  char skill_list[MAX_LEVEL+1][MAX_STRING_LENGTH];
  char skill_columns[MAX_LEVEL+1];
  int sn, level,
  // Modified by SinaC 2000
    min_lev = 0, 
    /*max_lev = LEVEL_HERO;*/
    max_lev = MAX_LEVEL,
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
	sprintf(buf,"Levels must be between 0 and %d.\n\r",LEVEL_HERO);
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
    skill_columns[level] = 0;
    skill_list[level][0] = '\0';
  }
  
  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;
    
    // Modified by SinaC 2001
    level = class_abilitylevel( /*ch->cstat(classes)*/ch,sn);
    // Modified by SinaC 2001
    //pra = ch->pcdata->skill_info[sn].learned;
    pra = get_ability_simple( ch, sn );

    // Modified by SinaC 2000
    //pra = get_skill( ch, sn );
    // Modified by SinaC 2000, again by SinaC 2001 for god skill
    if ( get_raceability( ch, sn ) 
	 || get_clanability(ch, sn ) 
	 /*|| get_godskill( ch, sn )     removed by SinaC 2003*/) {
      // Modified by SinaC 2000
      level = 0;
      pra = 100;
    }
    
    if ( ( level < LEVEL_HERO + 1 
	   // Modified by Sinac 1997, and SinaC 2000
	   || ch->level >= IM )
	 &&  (fAll || level <= ch->level)
	 &&  level >= min_lev && level <= max_lev
	 // Modified by SinaC 2001
	 //&&  ability_table[sn].spell_fun == spell_null
	 && ability_table[sn].type == TYPE_SKILL
	 // Modified by SinaC 2000
	 &&  /*ch->pcdata->learned[sn] > 0*/ pra > 0 ) {
      found = TRUE;
      /*
	level = class_skilllevel( ch->cstat(classes), sn );
	// Modified by Sinac 1997
	if ( raceskill ) level = 1;
      */
      
      if (ch->level < level)
	sprintf(buf,"%-20s  n/a       ", ability_table[sn].name);
      else {
	// Modified by SinaC 2000, again by SinaC 2001
	int casting_level = get_casting_level( ch, sn );
	if ( /*ch->pcdata->skill_info[sn].learned_lvl*/
	    casting_level != 0 )
	  sprintf(buf,
		  "%-20s  %3d%% (%2d) ",
		  ability_table[sn].name,
		  pra,
		  /*ch->pcdata->skill_info[sn].learned_lvl*/casting_level);
	else
	  sprintf(buf,
		  "%-20s  %3d%%      ",
		  ability_table[sn].name,
		  pra);
      }
      
      if (skill_list[level][0] == '\0')
	sprintf(skill_list[level],"\n\rLvl %3d: %s",level,buf);
      else { /* append */
	if ( ++skill_columns[level] % 2 == 0)
	  strcat(skill_list[level],"\n\r         ");
	strcat(skill_list[level],buf);
      }
    }
  }
  
  /* return results */

  // Modified by SinaC 2000
  if (!found) {
    send_to_char("No skills found.\n\r",ch);
    return;
  }

  // Added by SinaC 2000

  send_to_charf( ch,
		 "Level    Skill name            %%age  lvl"
		 " Skill name            %%age  lvl\n\r"
		 "----------------------------------------"
		 " -------------------------------");  

  buffer = new_buf();
  /*    for (level = 0; level < LEVEL_HERO + 1; level++)*/
  for (level = 0; level < MAX_LEVEL + 1; level++)
    if (skill_list[level][0] != '\0')
      add_buf(buffer,skill_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}

// Removed by SinaC 2001
/*
// Added by Sinac 1997
void do_level( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int sn, i, level;
  bool found = FALSE;
  
  one_argument( argument, arg );
  
  if ( IS_NPC( ch ) ) {
    send_to_char( "Mobs cannot see these informations.\n\r", ch );
    return;
  }
  
  if ( arg[0] == '\0' ) {
    send_to_char( "You must specify a skill or a spell.\n\r", ch );
    return;
  }
  
  if ( ( sn = skill_lookup( arg ) ) < 1 ) {
    send_to_char( "It is not a available skill or spell !\n\r", ch );
    return;
  }
    
  if ( ability_table[sn].spell_fun == spell_null )
    sprintf( buf, "      Skill : %s", ability_table[sn].name );
  else  
    sprintf( buf, "      Spell : %s", ability_table[sn].name );
  if ( ability_table[sn].nb_casting_level != 0 ) {
    char buf2[128];
    if ( ability_table[sn].nb_casting_level > 1 )
      sprintf( buf2, "   %2d levels\n\r", ability_table[sn].nb_casting_level );
    else
      sprintf( buf2, "   %2d level\n\r", ability_table[sn].nb_casting_level );
    strcat( buf, buf2 );
  }
  else
    strcat( buf, "    no level\n\r" );
  send_to_char( buf, ch );

  if ( ability_table[sn].prereqs != NULL ) {
    sprintf( buf, 
	     "Prerequisites:\n\r" );
    for ( i = 0; i < ability_table[sn].nb_casting_level; i++ ){
      sprintf( buf2,
	       " level %d: ", ability_table[sn].prereqs[i].casting_level );
      if ( ability_table[sn].prereqs[i].nb_prereq == 0 )
	strcat( buf2, "none\n\r");
      else{
	for ( int j = 0; j < ability_table[sn].prereqs[i].nb_prereq; j++ ){
	  sprintf( buf3,
		   "%s%s level %d\n\r",
		   j == 0 ? "" : "          ",
		   //ability_table[sn].prereqs[i].prereq[j].name,
		   ability_table[ability_table[sn].prereqs[i].prereq[j].sn].name,
		   ability_table[sn].prereqs[i].prereq[j].casting_level );
	  strcat( buf2, buf3 );
	}
      }
      strcat( buf, buf2 );
    }
    send_to_char( buf, ch );
  }
  
  send_to_char( "Classes :\n\r", ch );
    
  int col = 0;
  for ( i = 0; i < MAX_CLASS; i++ ) {
    if ( ( level = ability_table[sn].skill_level[i] ) == IM )
      sprintf( buf, "  %-20s  level : N/A", class_table[i].name );
    else  
      sprintf( buf, "  %-20s  level : %3d", class_table[i].name, level );
    if ( col % 2 != 0 )
      strcat( buf, "\n\r" );
    else
      strcat( buf, "     " );

    send_to_char( buf, ch );
    col++;
  }
  if ( col % 2 != 0 )
    send_to_char( "\n\r", ch );
  
  sprintf( buf, "Natural born abilities for : " );
  for ( i = 0; i < MAX_PC_RACE; i++ ) {
    for ( int j = 0; j < pc_race_table[i].nb_skills; j++ ) {
      if ( pc_race_table[i].skills[j] <= 0 )
	continue;
      if ( pc_race_table[i].skills[j] == sn ) {
	sprintf( buf2, "%s ", pc_race_table[ i ].name );
	strcat( buf, buf2 );
	found = TRUE;
	break;
      }
    }
  }
  if ( !found ) strcat( buf, "/" );
  strcat( buf, "\n\r" );
  send_to_char( buf, ch );
    
  return;  
}
*/


// Added by SinaC 2001
void do_use( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg1 );

  // Added by SinaC 2001
  if ( ch->in_room == NULL )
    return;

  if ( arg1[0] == '\0' ) {
    send_to_char("Syntax:\n\r"
		 "  use <skill name> [<additional skill parameter>]...\n\r", ch );
    send_to_char("   if the skill name is in more than one word, put in into ''\n\r",ch);
    return;
  }

  //  int sn = skill_lookup( arg1 );
  //  if ( sn <= 0 ) {
  //    send_to_char("This skill doesn't exist.\n\r", ch );
  int sn = find_ability(ch,arg1,TYPE_SKILL);
  if ( sn < 1 ) {
    send_to_char( "You don't know any skills of that name.\n\r", ch );
    if ( SCRIPT_VERBOSE > 0 ) {
      if ( IS_NPC(ch) )
	log_stringf("%s (%d) tries to use skill: %s %s", 
		    NAME(ch), ch->pIndexData->vnum,
		    arg1, argument );
    }
    return;
  }
  //  if ( ability_table[sn].type != TYPE_SKILL ) {
  //    send_to_char("'USE' can only be used with skills.\n\r", ch );
  //    return;
  //  }
  
  // Added by SinaC 2001 to check minimal position
  if ( ch->position < ability_table[sn].minimum_position ) {
    switch( ch->position ) {
    case POS_DEAD:
      send_to_char( "Lie still; you are DEAD.\n\r", ch );
      break;
      
    case POS_MORTAL:
    case POS_INCAP:
      send_to_char( "You are hurt far too bad for that.\n\r", ch );
      break;

      // Added by SinaC 2003
    case POS_PARALYZED:
    send_to_char( "You are paralyzed, you can't move.\n\r", ch);
    //send_to_char( "You can't move.\n\r", ch);
      break;
      
    case POS_STUNNED:
      send_to_char( "You are too stunned to do that.\n\r", ch );
      break;
      
    case POS_SLEEPING:
      send_to_char( "In your dreams, or what?\n\r", ch );
      break;
      
    case POS_RESTING:
      send_to_char( "Nah... You feel too relaxed...\n\r", ch);
      break;
      
    case POS_SITTING:
      send_to_char( "Better stand up first.\n\r",ch);
      break;
      
    case POS_FIGHTING:
      send_to_char( "No way!  You are still fighting!\n\r", ch);
      break;
      
    }
    return;
  }
  

/* This must be uncommented but msgs like 'You'd better leave martial arts'
    will not be yet shown
  if ( get_skill( ch, sn ) <= 0 ) {
    send_to_charf(ch,
		  "You don't even know how to use that skill (%s)\n\r",
		  ability_table[sn].name);
    return;
  }
*/
  // Modified by SinaC 2001
  //if ( ability_table[sn].do_fun == NULL ) {
  if ( ability_table[sn].action_fun == NULL ) {
    send_to_charf(ch,"This is an automatic skill (%s).\n\r", ability_table[sn].name );
    return;
  }

  //(*ability_table[sn].do_fun) ( ch, argument );
  (*(DO_FUN*)ability_table[sn].action_fun) ( ch, argument );
}


