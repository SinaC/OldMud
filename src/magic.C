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
#include "classes.h"
//#include "olc.h"
#include "abilityupgrade.h"
#include "lookup.h"

// Added by SinaC 2001
#include "condition.h"
#include "db.h"
#include "handler.h"
#include "act_info.h"
#include "comm.h"
#include "fight.h"
#include "effects.h"
#include "update.h"
#include "act_comm.h"
#include "affects.h"
#include "act_obj.h"
#include "act_enter.h"
#include "restriction.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "power.h"
#include "wearoff_affect.h"
#include "spells_def.h"
#include "interp.h"
#include "ability.h"
#include "const.h"
#include "config.h"
#include "bit.h"
#include "utils.h"
#include "damage.h"
#include "weather.h"


//#define VERBOSE


/*
 * Local functions.
 */
void	say_spell	args( ( CHAR_DATA *ch, int sn ) );

/* imported functions */
bool    remove_obj      args( ( CHAR_DATA *ch, int iWear, bool fReplace ) );
void 	wear_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );




/* Removed by SinaC 2003
int find_spell( CHAR_DATA *ch, const char *name ) {
  // finds a spell the character can cast if possible
  int sn, found = -1;

  if (IS_NPC(ch))
    return skill_lookup(name);

  for ( sn = 0; sn < MAX_ABILITY; sn++ ) {
    if (ability_table[sn].name == NULL)
      break;
	  
    if (LOWER(name[0]) == LOWER(ability_table[sn].name[0])
	&&  !str_prefix(name,ability_table[sn].name)) {
      if ( found == -1)
	found = sn;
      // Modified by Sinac 1997
      if (get_skill( ch, sn )>0)
	return sn;
    }
  }
  return found;
}
*/


// RT spells and skills show the players spells
void do_spells(CHAR_DATA *ch, const char *argument) {
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
    mana,
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
    //pra = ch->pcdata->ability_info[sn].learned;
    pra = get_ability_simple( ch, sn );

    // Modified by SinaC 2000
    //pra = get_skill( ch, sn );
    // Modified by SinaC 2000, again by SinaC 2001 for god skill
    if ( get_raceability( ch, sn ) 
	 || get_clanability(ch, sn )
	 /*|| get_godskill( ch, sn )     removed by SinaC 2003*/ ) {
      level = 0;
      pra = 100;
    }
    
    if ( ( level < LEVEL_HERO + 1 
	   /* Modified by Sinac 1997 */
	   || ch->level >= IM  )
	 &&  (fAll || level <= ch->level)
	 &&  level >= min_lev && level <= max_lev
	 //&&  ability_table[sn].spell_fun != spell_null
	 // Added by SinaC 2001 for mental user
	 && ability_table[sn].type == TYPE_SPELL
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
	  mana = 50;
	else
	  mana = UMAX( //ability_table[sn].min_mana,
		      ability_table[sn].min_cost,
		       // Modified by SinaC 2001
		       100 / ( 2 + ch->level -  level ) );

	//mana = UMAX(ability_table[sn].min_mana,
	//	    100/(2 + ch->level - level));

	// Modified by SinaC 2000, again by SinaC 2001
	int casting_level = get_casting_level( ch, sn );
	if ( /*ch->pcdata->ability_info[sn].learned_lvl*/
	    casting_level != 0 )
	  sprintf(buf,
		  "%-20s  %3d  (%2d) ",
		  ability_table[sn].name,
		  mana,
		  /*ch->pcdata->ability_info[sn].learned_lvl*/
		  casting_level);
	else
	  sprintf(buf,
		  "%-20s  %3d       ",
		  ability_table[sn].name,
		  mana);
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
    send_to_char("No spells found.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  send_to_charf( ch,
		 "Level    Spell name           Mana   lvl"
		 " Spell name           Mana  lvl\n\r"
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



/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA *ch, int sn )
{
  char buf  [MAX_STRING_LENGTH];
  char buf2 [MAX_STRING_LENGTH];
  CHAR_DATA *rch;
  const char *pName;
  int iSyl;
  int length;

  struct syl_type
  {
    char *	old;
    char *	newsyl;
  };

  static const struct syl_type syl_table[] =
  {
    { " ",		" "		},
    { "ar",		"abra"		},
    { "au",		"kada"		},
    { "bless",	"fido"		},
    { "blind",	"nose"		},
    { "bur",	"mosa"		},
    { "cu",		"judi"		},
    { "de",		"oculo"		},
    { "en",		"unso"		},
    { "light",	"dies"		},
    { "lo",		"hi"		},
    { "mor",	"zak"		},
    { "move",	"sido"		},
    { "ness",	"lacri"		},
    { "ning",	"illa"		},
    { "per",	"duda"		},
    { "ra",		"gru"		},
    { "fresh",	"ima"		},
    { "re",		"candus"	},
    { "son",	"sabru"		},
    { "tect",	"infra"		},
    { "tri",	"cula"		},
    { "ven",	"nofo"		},
    { "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
    { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
    { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
    { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
    { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
    { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
    { "y", "l" }, { "z", "k" },
    { "", "" }
  };

  buf[0]	= '\0';
  for ( pName = ability_table[sn].name; *pName != '\0'; pName += length ) {
    for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ ) {
      if ( !str_prefix( syl_table[iSyl].old, pName ) ) {
	strcat( buf, syl_table[iSyl].newsyl );
	break;
      }
    }

    if ( length == 0 )
      length = 1;
  }

  sprintf( buf2, "$n utters the words, '%s'.", buf );
  sprintf( buf,  "$n casts the '%s' spell.", ability_table[sn].name );

  for ( rch = ch->in_room->people; rch; rch = rch->next_in_room ) {
    if ( rch != ch )
      // Modified by SinaC 2000, was rch->pcdata->learned[sn] before
      act( ( !IS_NPC(rch) && (get_ability(rch,sn) > 0) ) ? buf : buf2,
	   ch, NULL, rch, TO_VICT);
  }

  return;
}



/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_spell( int level, CHAR_DATA *victim, int dam_type )
{
  int save;

  // 50 before        replaced by 35      but 40 seems to be better
  save = 40 + ( victim->level - level) * 5 - victim->cstat(saving_throw) * 2;

  if (IS_AFFECTED(victim,AFF_BERSERK))
    save += victim->level/2;

  switch(check_immune(victim,dam_type)) {
  case IS_IMMUNE:		return TRUE;
  case IS_RESISTANT:	save += 2;	break;
  case IS_VULNERABLE:	save -= 2;	break;
  }

  // SinaC 2003: pure body gives resistances to disease and weaken
  if ( ( dam_type == DAM_DISEASE
	 || dam_type == DAM_WEAKEN )
       && is_pure_body_skilled( level, victim ) )
    return TRUE;
  // SinaC 2003: diamond body gives resistances to poison and little resistance to cold
  if ( ( dam_type == DAM_POISON
	 || ( dam_type == DAM_COLD && chance(25) ) )
       && is_diamond_body_skilled( level, victim ) )
    return TRUE;

  // Modified by SinaC 2001 for mental user
  //if (!IS_NPC(victim) && class_fMana(victim->cstat(classes)))
  if (!IS_NPC(victim) 
      && ( class_fMana(victim->cstat(classes))
	   || class_fPsp(victim->cstat(classes))))
    save = 9 * save / 10;
  save = URANGE( 5, save, 95 );

  return number_percent() < save;
}

/* RT save for dispels */
//bool saves_dispel( const int dis_level, const int spell_level, const int duration) {
//  //int save;
//  int save = spell_level;
//
//  // FIXME: can't test on duration because non-dispellable information is in affect_data
//  //if (duration == DURATION_PERMANENT) // PERMANENT affect are not dispellable
//  //  return FALSE;
//
//  // very hard to dispel permanent effects
//  if (duration == -1)
//    //spell_level += 5;
//    save += 5;
//
//  //save = 50 + (spell_level - dis_level) * 4; // was *5 before
//  save = 50 + (save - dis_level) * 4; // was *5 before
//  save = URANGE( 5, save, 95 );
//  return number_percent( ) < save;
//}
// SinaC 2003
bool saves_dispel( const int dis_level, AFFECT_DATA *paf, const int paf_spell_level, const int paf_duration ) {
  if ( paf && IS_SET( paf->flags, AFFECT_NON_DISPELLABLE ) )
    return TRUE;
  int duration = paf ? paf->duration: paf_duration;
  int spell_level = paf ? paf->level: paf_spell_level;
  if ( paf &&
       ( IS_SET( paf->flags, AFFECT_INHERENT )         // harder to dispel inherent affect
	 || IS_SET( paf->flags, AFFECT_PERMANENT ) ) ) // same for permanent affect (haste in create_mobile)
    spell_level += 5;
  
  int save = 50 + (spell_level - dis_level) * 4;
  save = URANGE( 5, save, 95 );
  return number_percent() < save;
}

/* co-routine for dispel magic and cancellation, modified by SinaC 2003 */
bool check_dispel( int dis_level, CHAR_DATA *victim, int sn ) {
  AFFECT_DATA *af;
  
  //if (is_affected(victim, sn)){  NOT NEEDED, it's checked in the loop, SinaC 2003
    for ( af = victim->affected; af != NULL; af = af->next ) {
      if ( af->type == sn ){
	/*
	// Added by SinaC 2001 so, player with permanent affect can't be dispelled
	//  as sprite's invisibility and quickling's haste
	if ( af->duration == -1 && !IS_NPC( victim ) )
	  continue;
	*/
	// Modified by SinaC 2001, not dispellable only if DURATION_PERMANENT
	//if ( af->duration == DURATION_PERMANENT )  // PERMANENT affect are not dispellable
	//  continue;
	//	if ( IS_SET( af->flags, AFFECT_NON_DISPELLABLE ) ) SinaC 2003: not needed, test in saves_dispel
	//	  continue;
	//if (!saves_dispel(dis_level,af->level,af->duration)){
	if (!saves_dispel(dis_level,af)){
	  affect_strip(victim,sn);
	  if ( ability_table[sn].msg_off ){
	    send_to_char( ability_table[sn].msg_off, victim );
	    send_to_char( "\n\r", victim );
	  }
	  // Added by SinaC 2003
	  if ( ability_table[sn].wearoff_fun != NULL 
	       && ability_table[sn].wearoff_fun != wearoff_null )
	    (*ability_table[sn].wearoff_fun) ( af, (void*)victim, TARGET_CHAR );
	  
	  return TRUE;
	}
	else
	  af->level--;
      }
    }
    //}
  return FALSE;
}

/* for finding mana costs -- temporary version */
int mana_cost (CHAR_DATA *ch, int min_mana, int level) {
  if (ch->level + 2 == level)
    return 1000;
  return UMAX(min_mana,(100/(2 + ch->level - level)));
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 * Another global for level of the spell added by SinaC 2000
 */
// Modified by SinaC 2001
//char *target_name;
char target_name[MAX_STRING_LENGTH];
char add_target_name[MAX_STRING_LENGTH]; // Added by SinaC 2003
// SinaC 2003, used for certain spell such as temporal stasis
//  spell which need special position in order to be cast
bool spell_failed;

// Added by SinaC 2000, removed by SinaC 2001
//int spell_level;

void do_cast( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  void *vo;
  int mana;
  int sn;
  int target;
  // Added by SinaC 2000 to store the true caster if a switched mob
  //  use to determine if there is enough mana, so they use mana
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
  // Modified by SinaC 2000 for mob class, so they use mana
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
  strcpy(add_target_name,arg3); // Added by SinaC 2003

  // without that test, player are forced to specify the target if they want to
  //  to use a specific level of a spell
  // forced to use: cast armor self 4
  // better if they can use: cast armor 4
  if ( is_number(arg2) && arg3[0] == '\0' ) {
    strcpy(arg3,arg2);  // copy arg2 in arg3
    strcpy(arg2,"");    // arg2 is empty
  }

  if ( arg1[0] == '\0' ) {
    send_to_char( "Cast which what where?\n\r", ch );
    return;
  }

  if( //(sn = find_spell(ch,arg1)) < 1 // Modified by SinaC 2003
     (sn = find_ability(ch,arg1,TYPE_SPELL)) < 1
      // Removed by SinaC 2001
      //|| ability_table[sn].spell_fun == spell_null
      // Added by SinaC 2001 for mental power
      || ability_table[sn].type != TYPE_SPELL
      || get_ability(ch,sn)==0 ) {
    send_to_char( "You don't know any spells of that name.\n\r", ch );
    if ( SCRIPT_VERBOSE > 0 ) {
      if ( IS_NPC(ch) )
	log_stringf("%s (%d) tries to cast spell: %s %s %s", 
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

  // Modified by SinaC 2001
  // Added by SinaC 2000
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL )
       && !IS_IMMORTAL( ch ) ) {
    send_to_char( "A force prevents you from casting spells.\n\r", ch );
    return;
  }

  /* Removed by SinaC 2003, AFF2_NOSPELL replace AFF_SILENCE
  if ( IS_AFFECTED( ch, AFF_SILENCE ) ) {
    send_to_char("{WYou can't cast any spell because you are silenced.{x\n\r",
		 ch);
    return;
  }
  */
  if ( IS_AFFECTED2( ch, AFF2_NOSPELL ) ) {
    send_to_char("{WYou can't cast any spell.{x\n\r",ch);
    return;
  }

  // Modified by SinaC 2001
  if (ch->level + 2 == class_abilitylevel(/*ch->cstat(classes)*/ch,sn))
    mana = 50;
  else
    mana = UMAX(
		//ability_table[sn].min_mana,
		ability_table[sn].min_cost,
		100 / ( 2 + ch->level -  class_abilitylevel(/*ch->cstat(classes)*/ch,sn) ) );

  // Modified by SinaC 2000 for mob class, so they use mana
  // Moved by SinaC 2001, was after the switch
  //if ( !IS_NPC(ch) && ch->mana < mana ) {
  if ( caster->mana < mana ) {
    send_to_char( "You don't have enough mana.\n\r", ch );
    return;
  }

  // Added by SinaC 2001 for branding
  /* Removed by SinaC 2003
  if ( !IS_NPC(ch) )
    if ( get_godskill( ch, sn ) 
	 && !ch->pcdata->branded
	 && !IS_IMMORTAL(ch) ) {
      send_to_charf(ch,
		    "You are not yet branded by %s.\n\r",
		    god_name(ch->pcdata->god));
      caster->mana -= mana;
      return;
    }
  */
  /*
   * Locate targets.
   */
  victim	= NULL;
  obj		= NULL;
  vo		= NULL;
  target	= TARGET_NONE;

  switch ( ability_table[sn].target ) {
  default:
    bug( "Do_cast: bad target for sn %d.", sn );
    return;

  case TAR_IGNORE:
    break;

  case TAR_CHAR_OFFENSIVE:
    if ( arg2[0] == '\0' ) {
      if ( ( victim = ch->fighting ) == NULL ) {
	send_to_char( "Cast the spell on whom?\n\r", ch );
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
      if (!IS_NPC(ch))
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
      send_to_char( "You cannot cast this spell on another.\n\r", ch );
      return;
    }

    victim = ch; // bug fix in necrotism
    vo = (void *) ch;
    target = TARGET_CHAR;
    break;

  case TAR_OBJ_INV:
    if ( arg2[0] == '\0' ) {
      send_to_char( "What should the spell be cast upon?\n\r", ch );
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
	send_to_char("Cast the spell on whom or what?\n\r",ch);
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

  if ( str_cmp( ability_table[sn].name, "ventriloquate" ) )
    say_spell( ch, sn );
      
  //WAIT_STATE( ch, ability_table[sn].beats );
  WAIT_STATE( ch, BEATS(sn) );

  // Added by SinaC 2000
  // we can lost our concentration if we're not enough intelligent
  int perc;
  // INT     perc
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
    perc = UMIN( int_app[ch->cstat(INT)].learn + 25, 100 );

  //  if we can't see the target, we have less chance to cast successfully ( -50% )
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
  //if ( number_percent() > get_skill(ch,sn) )
  if ( ( ( number_percent() > get_ability(ch,sn)
	|| number_percent() > perc ) )
       && !IS_IMMORTAL(ch) ) {

    // if we missed the target because we didn't see it ( !can_see )
    if ( notsee ){
      send_to_char( "You missed your target.\n\r", ch );
      check_improve(ch,sn,FALSE,1);
      caster->mana -= mana;
      missed = TRUE;
    }
    // if we missed to cast the spell
    // Added by SinaC 2001 race spell can't be missed
    else if ( !get_raceability(ch,sn) ) {
      send_to_char( "You lost your concentration.\n\r", ch );
      check_improve(ch,sn,FALSE,1);
      // Modified by SinaC 2000 for mob class, so they use mana
      //ch->mana -= mana / 2;
      caster->mana -= mana/2;
      missed = TRUE;
    }
  }

  // FIXME: wild surge for wild mage
  if ( ch->isWildMagic ) {
  }

  // Modified by SinaC 2001
  if ( !missed ) {
    // Modified by SinaC 2000 for mob class, so they use mana
    //ch->mana -= mana;
    caster->mana -= mana;

    // Modified by SinaC 2000
    int sp_lvl;
    // class has spells
    if (IS_NPC(ch) || class_fMana(ch->cstat(classes)))
      sp_lvl = ch->level;
    else
      sp_lvl = UMAX(3 * ch->level/4,1);

    // Added by SinaC 2003, increased casting improves spell level by 10
    if ( IS_AFFECTED2( ch, AFF2_INCREASED_CASTING ) )
      sp_lvl += number_fuzzy(10);

    // Added by SinaC 2003, when affected by improved_restoration
    //  spells from sphere restoration are upgraded
    if ( is_ability_sphere( sn, "restoration" )
	 && is_affected( ch, gsn_improved_restoration ) )
      sp_lvl += number_fuzzy(10);

    // SinaC 2003, when char data has choosen a wildable class: spell is casted with a +/- 10 random
    if ( ch->isWildMagic )
      sp_lvl = URANGE( 1, sp_lvl+number_range(0,20)-10, MAX_LEVEL );
   
    // Modified by SinaC 2000 so we have the spell level in an ugly global variable
    //spell_level = ch->pcdata->ability_info[sn].learned_lvl;
    //spell_level = get_skill_level( ch, sn );
    int max_casting_level = get_casting_level(ch,sn);
    int casting_level;
    //log_stringf("%s casts '%s' level %d",
    //	NAME(ch), ability_table[sn].name, spell_level );

    // Added by SinaC 2001 so: cast armor target 4 --> cast armor level 4 on target
    if ( arg3[0] == '\0' )
      casting_level = max_casting_level;
    else if ( !is_number(arg3) )
      casting_level = max_casting_level;
    else {
      int clevel = atoi(arg3);
      if ( clevel >= max_casting_level )
	casting_level = max_casting_level;
      else
	casting_level = clevel;
    }
    //casting_level = UMAX( 1, casting_level );
    casting_level = UMAX( 0, casting_level );

    // Added by SinaC 2003 for necrotism
    if ( target == TARGET_CHAR
	 && IS_AFFECTED2( ch, AFF2_NECROTISM )
	 && ( victim == ch 
	      || !is_safe_spell(ch,(CHAR_DATA*)vo,FALSE) )
	 && !IS_IMMORTAL( victim ) ) {
      int percent = number_range(0, 999);
      int chance = 998;
      
      switch( check_immune( victim, DAM_NEGATIVE ) ) {
      case IS_IMMUNE : chance = 1000; break;
      case IS_RESISTANT : chance = 999; break;
      case IS_VULNERABLE : chance = 995; break;
      }
      
      if ( percent >= chance ) {
	CHAR_DATA *victim = (CHAR_DATA *)vo;
	if ( ch == victim ) {
	  static char * const him_her_self [] = { "itself",  "himself", "herself" };
	  static char * const his_her [] = { "its", "his", "her" };
	  char buf[MAX_STRING_LENGTH];
	  sprintf( buf, "$n's {DNecrotism{x strikes %s.",
		   him_her_self[URANGE(0,ch->cstat(sex),2)]);
	  send_to_char("Your {DNecrotism{x strikes yourself.\n\r", ch );
	  act(buf,ch,NULL,NULL,TO_ROOM);
	  sprintf( buf, "has been killed by %s own {DNecrotism{x!",
		   his_her[URANGE(0,ch->cstat(sex),2)] );
	  silent_kill( ch, buf );
	}
	else {
	  act( "The {DNecrotism{x of $n strikes $N.", ch, NULL, victim, TO_NOTVICT );
	  act( "Your {DNecrotism{x strikes $N.", ch, NULL, victim, TO_CHAR );
	  act( "The {DNecrotism{x of $n strikes and KILLS you.", ch, NULL, victim, TO_VICT );
	  sudden_death( ch, victim );
	}
	return;
      }
    }
	 

    spell_failed = FALSE;

    // If the victim is affected by a magic mirror, the spell is reflected, SinaC 2001
    if ( target == TARGET_CHAR
	 && ( ability_table[sn].target == TAR_OBJ_CHAR_OFF
	      || ability_table[sn].target == TAR_CHAR_OFFENSIVE )
	 && IS_AFFECTED2( (CHAR_DATA*) vo, AFF2_MAGIC_MIRROR ) 
	 && chance(50) ) {
      CHAR_DATA *victim = (CHAR_DATA*)vo;
#ifdef VERBOSE
      log_stringf("DO_CAST: MAGIC MIRROR: spell: %s  ch: %s    victim: %s",
		  ability_table[sn].name, NAME(ch), NAME(victim) );
#endif      
      act( "Your spell is mirrored by $N's magic aura.", ch, NULL, victim, TO_CHAR );
      act( "$n's spell is mirrored by your magic aura.", ch, NULL, victim, TO_VICT );
      act( "$n's spell is mirrored by $N's magic aura.", ch, NULL, victim, TO_NOTVICT );
      /*
      (*ability_table[sn].spell_fun) ( sn, sp_lvl, ch, (void*)ch, 
				     target, casting_level );
      */
      if ( ability_table[sn].scriptAbility ) { // SinaC 2003 for script ability
	Value args[] = { ability_table[sn].name, sp_lvl, ch, casting_level, target_name };
	int res = mobprog( ch, NULL, "onCast", args );
	if ( !res ) { // don't know this script spell
	  send_to_char("Nothing happens.\n\r",ch);
	  return;
	}
      }
      else
	(*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, sp_lvl, ch, (void*)ch, target, casting_level );
    }
    else {
      // Modified by SinaC 2001
      //(*ability_table[sn].spell_fun) ( sn, sp_lvl, ch, vo, target, casting_level );
      if ( ability_table[sn].scriptAbility ) { // SinaC 2003 for script ability
	Value args[] = { ability_table[sn].name, sp_lvl, 
			 target == TARGET_CHAR ? (CHAR_DATA *)vo
			 : ( target == TARGET_OBJ ? (OBJ_DATA *)vo
			     : (ENTITY_DATA*)vo ),  // we don't know what target is
			 casting_level, target_name };
	int res = mobprog( ch, NULL, "onCast", args );
	if ( !res ) { // don't know this script spell
	  send_to_char("Nothing happens.\n\r",ch);
	  return;
	}
      }
      else
	(*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, sp_lvl, ch, vo, target, casting_level );

      check_improve(ch,sn,TRUE,1);
    }

    // Added by SinaC 2003 for onSpellTarget trigger, available for mob and obj
    Value args[] = { ch, ability_table[sn].name, casting_level };
    if ( target == TARGET_CHAR )
      mobprog(victim,ch,"onSpellTarget", args);
    else if ( target == TARGET_OBJ )
      objprog(obj,ch,"onSpellTarget",args);
  }

  // Added by SinaC in case of the spell is mirrored and kill ch
  if ( ch == NULL || !ch->valid || ch->in_room == NULL )
    return;

  if ( (ability_table[sn].target == TAR_CHAR_OFFENSIVE
	||   (ability_table[sn].target == TAR_OBJ_CHAR_OFF
	      && target == TARGET_CHAR))
       &&   victim != ch
       &&   victim->master != ch) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if ( victim != NULL )
      multi_hit( ch, victim, TYPE_UNDEFINED ); // SinaC 2003 starts a fight between ch and victim

    for ( vch = ch->in_room->people; vch; vch = vch_next ) {
      vch_next = vch->next_in_room;
      // spell_failed test added by SinaC 2003
      //if ( victim == vch && victim->fighting == NULL && spell_failed == FALSE ) {
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
 * Cast spells at targets using a magical object.
 */
void obj_cast_spell( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) {
  void *vo;
  int target = TARGET_NONE;

  if ( sn <= 0 )
    return;

  // Modified by SinaC 2001
  /*
  if ( sn >= MAX_ABILITY || ability_table[sn].spell_fun == 0 ) {
    bug( "Obj_cast_spell: bad sn %d.", sn );
    return;
  }
  */
  /*
  if ( sn >= MAX_ABILITY || ability_table[sn].type != TYPE_SPELL ) {
    bug( "Obj_cast_spell: bad sn %d.", sn );
    return;
  }
  */
  if ( sn >= MAX_ABILITY
       && ( ability_table[sn].type != TYPE_SPELL || ability_table[sn].type != TYPE_POWER ) ) {
    bug( "Obj_cast_spell: bad sn %d.", sn );
    return;
  }

  if ( ability_table[sn].type == TYPE_POWER ) {
    obj_use_power( sn, level, ch, victim, obj );
    return;
  }
  

  switch ( ability_table[sn].target ) {
  default:
    bug( "Obj_cast_spell: bad target for sn %d.", sn );
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

  // If the victim is affected by a magic mirror, the spell is reflected, SinaC 2001
  if ( target == TARGET_CHAR
       && ( ability_table[sn].target == TAR_OBJ_CHAR_OFF
	    || ability_table[sn].target == TAR_CHAR_OFFENSIVE )
       && IS_AFFECTED2( (CHAR_DATA*) vo, AFF2_MAGIC_MIRROR ) 
       && chance(50) ) {
    CHAR_DATA *victim = (CHAR_DATA*)vo;
#ifdef VERBOSE
    log_stringf("OBJ_CAST_SPELL: MAGIC MIRROR: spell: %s  ch: %s    victim: %s",
		ability_table[sn].name, NAME(ch), NAME(victim) );
#endif
    act( "Your spell is mirrored by $N's magic aura.", ch, NULL, victim, TO_CHAR );
    act( "$n's spell is mirrored by your magic aura.", ch, NULL, victim, TO_VICT );
    act( "$n's spell is mirrored by $N's magic aura.", ch, NULL, victim, TO_NOTVICT );
    //(*ability_table[sn].spell_fun) ( sn, level, ch, (void*)ch, target, 1 );
    (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, level, ch, (void*)ch, target, 1 );
  }
  else {
    // Modified by SinaC 2001
    //(*ability_table[sn].spell_fun) ( sn, level, ch, vo,target, 1 );
    (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, level, ch, vo, 
						target, 1 );
  }

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

/*
void spell_null( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  send_to_char( "That's not a spell!\n\r", ch );
  return;
}
*/


/*
 * Spell functions.
 */

void spell_bless( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* deal with the object case first */
  if (target == TARGET_OBJ) {
    obj = (OBJ_DATA *) vo;
    if (IS_OBJ_STAT(obj,ITEM_BLESS)) {
      act("$p is already blessed.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if (IS_OBJ_STAT(obj,ITEM_EVIL)) {
      AFFECT_DATA *paf;

      paf = affect_find(obj->affected,gsn_curse);
      //if (!saves_dispel( level, paf != NULL ? paf->level : obj->level, 0 ) ) {
      if (!saves_dispel( level, paf, obj->level, 0 ) ) {
	if (paf != NULL)
	  affect_remove_obj(obj,paf);
	act("$p glows a pale blue.",ch,obj,NULL,TO_ALL);
	REM_OBJ_STAT(obj,ITEM_EVIL);
	return;
      }
      else {
	act("The evil of $p is too powerful for you to overcome.",
	    ch,obj,NULL,TO_CHAR);
	return;
      }
    }

    // for casting level  SinaC 2001
    //value = -1 * casting_level;
    //value = -1;
    //newafsetup(af,CHAR,saving_throw,ADD,value,6+level,level,sn,casting_level);
    //afsetup(af,CHAR,saving_throw,ADD,value,6+level,level,sn);
    //affect_to_obj(obj,&af);
    //newafsetup(af,OBJECT,NA,OR,ITEM_BLESS,6+level,level,sn,casting_level);
    //afsetup(af,OBJECT,NA,OR,ITEM_BLESS,6+level,level,sn);
    //affect_to_obj(obj,&af);
    int value = -1;
    createaff(af,6+level,level,sn,0,AFFECT_ABILITY);
    addaff(af,CHAR,saving_throw,ADD,value);
    addaff(af,OBJECT,NA,OR,ITEM_BLESS);
    affect_to_obj(obj,&af);

    act("$p glows with a holy aura.",ch,obj,NULL,TO_ALL);

    return;
  }

  /* character target */
  victim = (CHAR_DATA *) vo;

  if ( victim->position == POS_FIGHTING 
       || is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already blessed.\n\r",ch);
    else
      act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
    return;
  }

  // for casting level  SinaC 2001
  //newafsetup(af,CHAR,hitroll,ADD,level/8+2*casting_level,6+level,level,sn,casting_level);
  //afsetup(af,CHAR,hitroll,ADD,level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  // for casting level  SinaC 2001
  //newafsetup(af,CHAR,saving_throw,ADD,0-level/8-2*casting_level,6+level,level,sn,casting_level);
  //afsetup(af,CHAR,saving_throw,ADD,0-level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,6+level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,saving_throw,ADD,0-level/8);
  addaff(af,CHAR,hitroll,ADD,level/8);
  affect_to_char(victim,&af);

  /*
  if (casting_level == 3) {
    // for casting level  SinaC 2001
    newafsetup(af,CHAR,damroll,ADD,level/8+2*casting_level,6+level,level,sn,casting_level);
    affect_to_char( victim, &af );
  }
  */

  send_to_char( "You feel righteous.\n\r", victim );
  if ( ch != victim )
    act("You grant $N the favor of your god.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_blindness( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_BLIND) 
       || saves_spell(level,victim,DAM_OTHER)) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  //    afsetup(af,CHAR,hitroll,ADD,-4,1+level,level,sn);
  //afsetup(af,CHAR,hitroll,ADD,-level/10,1+level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,1+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,1+level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,-level/10);
  addaff(af,CHAR,affected_by,OR,AFF_BLIND);
  affect_to_char( victim, &af );

  send_to_char( "You are blinded!\n\r", victim );
  act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_call_lightning( int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  char buf[MAX_STRING_LENGTH];
  

  if ( !IS_OUTSIDE(ch) ) {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }

  if ( weather_info.sky < SKY_RAINING ) {
    send_to_char( "You need bad weather.\n\r", ch );
    return;
  }

  dam = dice(level/2, 8);

  // Modified by SinaC 2001 for god
  //send_to_char( "Mota's lightning strikes your foes!\n\r", ch );
  //act( "$n calls Mota's lightning to strike $s foes!",
       //ch, NULL, NULL, TO_ROOM );
  send_to_charf( ch, 
		 "%s's lightning strikes your foes!\n\r",
		 char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
  sprintf( buf, 
	   "$n calls %s's lightning to strike $s foes!",
	   char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
  act( buf, ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room ) {
      if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
	ability_damage( ch, vch, saves_spell( level,vch,DAM_LIGHTNING) 
		? dam / 2 : dam, sn,DAM_LIGHTNING,TRUE, TRUE);
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area
	 &&   IS_OUTSIDE(vch)
	 &&   IS_AWAKE(vch) )
      send_to_char( "Lightning flashes in the sky.\n\r", vch );
  }

  return;
}

/* RT calm spell stops all fighting in the room */

void spell_calm( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;    
  int chance;
  AFFECT_DATA af;

  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    if (vch->position == POS_FIGHTING) {
      count++;
      if (IS_NPC(vch))
	mlevel += vch->level;
      else
	mlevel += vch->level/2;
      high_level = UMAX(high_level,vch->level);
    }
  }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL(ch)) /* always works */
    mlevel = 0;

  if (number_range(0, chance) >= mlevel) { /* hard to stop large fights */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
      if (IS_NPC(vch) 
	  && (IS_SET(vch->cstat(imm_flags),IRV_MAGIC) 
	      || IS_SET(vch->act,ACT_UNDEAD)
	      || IS_SET(vch->cstat(form),FORM_UNDEAD)))
	return;

      if (IS_AFFECTED(vch,AFF_CALM) 
	  || IS_AFFECTED(vch,AFF_BERSERK)
	  ||  is_affected(vch,gsn_frenzy))
	return;
	    
      send_to_char("A wave of calm passes over you.\n\r",vch);

      if (vch->fighting || vch->position == POS_FIGHTING)
	stop_fighting(vch,FALSE);


      //afsetup(af,CHAR,affected_by,OR,AFF_CALM,level/4,level,sn);
      //affect_to_char(vch,&af);
      //afsetup(af,CHAR,hitroll,ADD,IS_PC(vch)?-5:-2,level/4,level,sn);
      //affect_to_char(vch,&af);
      //af.location = ATTR_damroll;
      //affect_to_char(vch,&af);
      createaff(af,level/4,level,sn,0,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,IS_PC(vch)?-5:-2);
      addaff(af,CHAR,damroll,ADD,IS_PC(vch)?-5:-2);
      addaff(af,CHAR,affected_by,OR,AFF_CALM);
      affect_to_char( vch, &af );
    }
  }
}

void spell_cancellation( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;
 
  level += 2;

  if ((!IS_NPC(ch) && IS_NPC(victim) 
       && !(IS_AFFECTED(ch, AFF_CHARM) 
	    && ch->master == victim) ) 
      || (IS_NPC(ch) 
	  && !IS_NPC(victim)) ) {
    send_to_char("You failed, try dispel magic.\n\r",ch);
    return;
  }

  /* unlike dispel magic, the victim gets NO save */

  // the following lines will replaced all these tests
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    if ( ability_table[i].dispellable
	 && check_dispel(level,victim,i ) ) {
      found = TRUE;
      if ( ability_table[i].msg_dispel )
	act(ability_table[i].msg_dispel,victim,NULL,NULL,TO_ROOM);
    }
  }

/*
  // begin running through the spells
 
  if (check_dispel(level,victim,skill_lookup("armor")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("bless")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("blindness"))) {
    found = TRUE;
    act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
  }

  if (check_dispel(level,victim,skill_lookup("calm"))) {
    found = TRUE;
    act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,skill_lookup("change sex"))) {
    found = TRUE;
    act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,skill_lookup("charm person"))) {
    found = TRUE;
    act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,skill_lookup("chill touch"))) {
    found = TRUE;
    act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,skill_lookup("curse")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("detect evil")))
    found = TRUE;

  if (check_dispel(level,victim,skill_lookup("detect good")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("detect hidden")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("detect invis")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("detect magic")))
    found = TRUE;

  if (check_dispel(level,victim,skill_lookup("faerie fire"))) {
    act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,skill_lookup("fly"))) {
    act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("frenzy"))) {
    act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
    found = TRUE;
  }
 
  if (check_dispel(level,victim,skill_lookup("giant strength"))) {
    act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("haste"))) {
    act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,skill_lookup("infravision")))
    found = TRUE;
 
  if (check_dispel(level,victim,skill_lookup("invis"))) {
    act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,skill_lookup("mass invis"))) {
    act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,skill_lookup("pass door")))
    found = TRUE;

  if (check_dispel(level,victim,skill_lookup("protection evil")))
    found = TRUE;

  if (check_dispel(level,victim,skill_lookup("protection good")))
    found = TRUE; 
 
  if (check_dispel(level,victim,skill_lookup("sanctuary"))) {
    act("The white aura around $n's body vanishes.",
	victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("shield"))) {
    act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("sleep")))
    found = TRUE;

  if (check_dispel(level,victim,skill_lookup("slow"))) {
    act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("stone skin"))) {
    act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  if (check_dispel(level,victim,skill_lookup("weaken"))) {
    act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
    
  // modified by Sinac 1997

  if (check_dispel(level,victim,skill_lookup("aid")))
    found = TRUE;
        
  if (check_dispel(level,victim,skill_lookup("levitation"))) {
    act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }    
    
  if (check_dispel(level,victim,skill_lookup("blur"))) {
    act("$n is no longer distorted!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
    
  if (check_dispel(level,victim,skill_lookup("combat mind")))
    found = TRUE;
        
  if (check_dispel(level,victim,skill_lookup("hesitation")))
    found = TRUE;    
        
  if (check_dispel(level,victim,skill_lookup("trouble")))
    found = TRUE;        

  if (check_dispel(level,victim,skill_lookup("dizziness")))
    found = TRUE;
    
  // modified by Seytan 1997
  if (check_dispel(level,victim,skill_lookup("ectoplasmic form")))
    found = TRUE;
    	
  if (check_dispel(level,victim,skill_lookup("displacement")))
    found = TRUE;
    	
  if (check_dispel(level,victim,skill_lookup("adrenaline control")))	
    found = TRUE;
*/

  if (found)
    send_to_char("Ok.\n\r",ch);
  else
    send_to_char("Spell failed.\n\r",ch);
}

void spell_cause_light( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  ability_damage( ch, (CHAR_DATA *) vo, dice(1, 8) + level / 3, sn,DAM_HARM,TRUE, TRUE);
  return;
}

void spell_cause_critical(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  ability_damage( ch, (CHAR_DATA *) vo, dice(3, 8) + level - 6, sn,DAM_HARM,TRUE, TRUE);
  return;
}

void spell_cause_serious(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  ability_damage( ch, (CHAR_DATA *) vo, dice(2, 8) + level / 2, sn,DAM_HARM,TRUE, TRUE);
  return;
}

void spell_change_sex( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn )) {
    if (victim == ch)
      send_to_char("You've already been changed.\n\r",ch);
    else
      act("$N has already had $s(?) sex changed.",ch,NULL,victim,TO_CHAR);
    return;
  }
  if (saves_spell(level , victim,DAM_OTHER)) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  /*
  afsetup(af,CHAR,sex,ADD,0,2*level,level,sn);
  do {
    af.modifier  = number_range( 0, 2 ) - victim->cstat(sex);
  }
  while ( af.modifier == 0 );
  */
  //afsetup(af,CHAR,sex,ASSIGN,number_range(0,2),2*level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level*2,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,sex,ASSIGN,number_range(0,2));
  affect_to_char( victim, &af );


  send_to_char( "You feel different.\n\r", victim );
  act("$n doesn't look like $mself anymore...",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_charm_person( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_safe(ch,victim)) 
    return;

  if ( victim == ch ) {
    send_to_char( "You like yourself even better!\n\r", ch );
    return;
  }

  // !IS_NPC added by SinaC 2000
  if ( !IS_NPC(victim)
       || IS_AFFECTED(victim, AFF_CHARM)
       || IS_AFFECTED(ch, AFF_CHARM)
       || level < victim->level
       || IS_SET(victim->cstat(imm_flags),IRV_CHARM)
       || saves_spell( level, victim,DAM_CHARM) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  if (IS_SET(victim->in_room->cstat(flags),ROOM_LAW)
      && !IS_IMMORTAL(ch)) {
    send_to_char(
		 "The mayor does not allow charming in the city limits.\n\r",ch);
    return;
  }
  
  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );
  victim->leader = ch;

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 4 ),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( victim, &af );

  act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
  if ( ch != victim )
    act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
  return;
}


void spell_continual_light(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  OBJ_DATA *light;

  if (target_name[0] != '\0') { /* do a glow on some object */
    light = get_obj_carry(ch,target_name,ch);
	
    if (light == NULL) {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }

    if (IS_OBJ_STAT(light,ITEM_GLOW)) {
      act("$p is already glowing.",ch,light,NULL,TO_CHAR);
      return;
    }
    
    SET_OBJ_STAT(light,ITEM_GLOW);
    act("$p glows with a white light.",ch,light,NULL,TO_ALL);
    return;
  }

  light = create_object( get_obj_index( OBJ_VNUM_LIGHT_BALL ), 0 );
  obj_to_room( light, ch->in_room );
  // Added by SinaC 2001
  recomproom(ch->in_room);

  act( "$n twiddles $s thumbs and $p appears.",   ch, light, NULL, TO_ROOM );
  act( "You twiddle your thumbs and $p appears.", ch, light, NULL, TO_CHAR );
  return;
}

void spell_control_weather(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)  {
  if ( !str_cmp( target_name, "better" ) )
    weather_info.change += dice( level / 3, 4 );
  else if ( !str_cmp( target_name, "worse" ) )
    weather_info.change -= dice( level / 3, 4 );
  else {
    send_to_char ("Do you want it to get better or worse?\n\r", ch );
    return;
  }

  send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_create_food( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *mushroom;

  mushroom = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
  mushroom->baseval[0] = level / 2;
  mushroom->baseval[1] = level;
  recompobj(mushroom);
  obj_to_room( mushroom, ch->in_room );
  act( "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM );
  act( "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR );
  return;
}

void spell_create_rose( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  OBJ_DATA *rose;
  rose = create_object(get_obj_index(OBJ_VNUM_ROSE), 0);
  act("$n has created a beautiful red rose.",ch,rose,NULL,TO_ROOM);
  send_to_char("You create a beautiful red rose.\n\r",ch);
  obj_to_char(rose,ch);
  return;
}

void spell_create_spring(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  OBJ_DATA *spring;

  spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
  spring->timer = level;
  obj_to_room( spring, ch->in_room );
  act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
  act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );

  // Added by SinaC 2001
  recomproom(ch->in_room);
  return;
}

void spell_create_water( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int water;

  if ( obj->item_type != ITEM_DRINK_CON ) {
    send_to_char( "It is unable to hold water.\n\r", ch );
    return;
  }

  if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 ) {
    send_to_char( "It contains some other liquid.\n\r", ch );
    return;
  }

  water = UMIN(
	       level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
	       obj->value[0] - obj->value[1]
	       );
  
  if ( water > 0 ) {
    obj->baseval[2] = LIQ_WATER;
    obj->baseval[1] += water;
    recompobj(obj);
    if ( !is_name( "water", obj->name ) ) {
      char buf[MAX_STRING_LENGTH];

      sprintf( buf, "%s water", obj->name );
      obj->name = str_dup( buf );
    }
    act( "$p is filled.", ch, obj, NULL, TO_CHAR );
  }

  return;
}

void spell_cure_blindness(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( !is_affected( victim, gsn_blindness ) ) {
    if (victim == ch)
      send_to_char("You aren't blind.\n\r",ch);
    else
      act("$N doesn't appear to be blinded.",ch,NULL,victim,TO_CHAR);
    return;
  }
 
  if (check_dispel(level,victim,gsn_blindness)) {
    send_to_char( "Your vision returns!\n\r", victim );
    act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
  }
  else
    send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_critical( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice(3, 8) + level - 6;
  victim->hit = UMIN( victim->hit + heal, victim->cstat(max_hit) );
  update_pos( victim );
  send_to_char( "You feel better!\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

/* RT added to cure plague */
void spell_cure_disease( int sn, int level, CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  // Modified by SinaC 2003 for black plague
  if ( !is_affected( victim, gsn_plague ) && !is_affected( victim, gsn_black_plague ) ) {
    if (victim == ch)
      send_to_char("You aren't ill.\n\r",ch);
    else
      act("$N doesn't appear to be diseased.",ch,NULL,victim,TO_CHAR);
    return;
  }

  // Modified by SinaC 2003 for black plague
  if (check_dispel(level,victim,gsn_plague) || check_dispel(level,victim,gsn_black_plague) ) {
    send_to_char("Your sores vanish.\n\r",victim);
    act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
  }
  else
    send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_light( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice(1, 8) + level / 3;
  victim->hit = UMIN( victim->hit + heal, victim->cstat(max_hit) );
  update_pos( victim );
  send_to_char( "You feel better!\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_cure_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
 
  if ( !is_affected( victim, gsn_poison ) ) {
    if (victim == ch)
      send_to_char("You aren't poisoned.\n\r",ch);
    else
      act("$N doesn't appear to be poisoned.",ch,NULL,victim,TO_CHAR);
    return;
  }
 
  if (check_dispel(level,victim,gsn_poison)) {
    send_to_char("A warm feeling runs through your body.\n\r",victim);
    act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
  }
  else
    send_to_char("Spell failed.\n\r",ch);
}

void spell_cure_serious( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int heal;

  heal = dice(2, 8) + level /2 ;
  victim->hit = UMIN( victim->hit + heal, victim->cstat(max_hit) );
  update_pos( victim );
  send_to_char( "You feel better!\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* deal with the object case first */
  if (target == TARGET_OBJ) {
    obj = (OBJ_DATA *) vo;
    if (IS_OBJ_STAT(obj,ITEM_EVIL)) {
      act("$p is already filled with evil.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if (IS_OBJ_STAT(obj,ITEM_BLESS) ) {
      AFFECT_DATA *paf;

      paf = affect_find(obj->affected,ability_lookup("bless"));
      //if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0)) {
      if (!saves_dispel(level,paf, obj->level,0 ) ) {
	if (paf != NULL)
	  affect_remove_obj(obj,paf);
	act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);
	REM_OBJ_STAT(obj,ITEM_BLESS);
	return;
      }
      else {
	act("The holy aura of $p is too powerful for you to overcome.",
	    ch,obj,NULL,TO_CHAR);
	return;
      }
    }

    //afsetup(af,CHAR,saving_throw,ADD,+1,2*level,level,sn);
    //affect_to_obj(obj,&af);
    //afsetup(af,OBJECT,NA,OR,ITEM_EVIL,2*level,level,sn);
    //affect_to_obj(obj,&af);
    createaff(af,2*level,level,sn,0,AFFECT_ABILITY);
    addaff(af,OBJECT,NA,OR,ITEM_EVIL);
    addaff(af,CHAR,saving_throw,ADD,+1);
    affect_to_obj( obj, &af );

    act("$p glows with a malevolent aura.",ch,obj,NULL,TO_ALL);

    return;
  }

  /* character curses */
  victim = (CHAR_DATA *) vo;

  if (IS_AFFECTED(victim,AFF_CURSE) 
      || saves_spell(level,victim,DAM_NEGATIVE)) {
    send_to_char("Spell failed.\n\r", ch);
    return;
  }

  //afsetup(af,CHAR,hitroll,ADD,0-level/8,2*level,level,sn);
  //affect_to_char( victim, &af );
  //af.location  = ATTR_saving_throw;
  //af.modifier  = level / 8;
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_CURSE,2*level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,2*level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,0-level/8);
  addaff(af,CHAR,saving_throw,ADD,level/5);
  addaff(af,CHAR,affected_by,OR,AFF_CURSE);
  affect_to_char( victim, &af );

  send_to_char( "You feel unclean.\n\r", victim );
  if ( ch != victim )
    act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
  return;
}

/* RT replacement demonfire spell */
void spell_demonfire(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_NPC(ch) && !IS_EVIL(ch) ) {
    victim = ch;
    send_to_char("The demons turn upon you!\n\r",ch);
  }

  // Modified by SinaC 2001 etho/alignment are attributes now
  // Modified by SinaC 2000 for alignment/etho
  //ch->align.alignment = UMAX(-1000, ch->align.alignment - 50);
  ch->bstat(alignment) = UMAX(-1000, ch->bstat(alignment) - 50);

  if (victim != ch) {
    act("$n calls forth the demons of Hell upon $N!",
	ch,NULL,victim,TO_ROOM);
    act("$n has assailed you with the demons of Hell!",
	ch,NULL,victim,TO_VICT);
    send_to_char("You conjure forth the demons of hell!\n\r",ch);
  }
  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;
  // bool done  added by SinaC 2001
  int done = ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);

  // Modified by SinaC 2001
  if (done == DAMAGE_DONE )
    spell_curse(gsn_curse, 3 * level / 4, ch, (void *) victim,TARGET_CHAR, 1 );
}

void spell_detect_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_EVIL) ) {
    if (victim == ch)
      send_to_char("You can already sense evil.\n\r",ch);
    else
      act("$N can already detect evil.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_EVIL,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_EVIL);
  affect_to_char( victim, &af );

  send_to_char( "Your eyes tingle.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_detect_good( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( IS_AFFECTED(victim, AFF_DETECT_GOOD) ) {
    if (victim == ch)
      send_to_char("You can already sense good.\n\r",ch);
    else
      act("$N can already detect good.",ch,NULL,victim,TO_CHAR);
    return;
  }
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_GOOD,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_GOOD);
  affect_to_char( victim, &af );

  send_to_char( "Your eyes tingle.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_detect_hidden(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_HIDDEN) ) {
    if (victim == ch)
      send_to_char("You are already as alert as you can be. \n\r",ch);
    else
      act("$N can already sense hidden lifeforms.",ch,NULL,victim,TO_CHAR);
    return;
  }
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN);
  affect_to_char( victim, &af );

  send_to_char( "Your awareness improves.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_detect_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_INVIS) ) {
    if (victim == ch)
      send_to_char("You can already see invisible.\n\r",ch);
    else
      act("$N can already see invisible things.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_INVIS,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_INVIS);
  affect_to_char( victim, &af );

  send_to_char( "Your eyes tingle.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_detect_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DETECT_MAGIC) ) {
    if (victim == ch)
      send_to_char("You can already sense magical auras.\n\r",ch);
    else
      act("$N can already detect magic.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC);
  affect_to_char( victim, &af );

  send_to_char( "Your eyes tingle.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_detect_poison( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if ( obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD ) {
    if ( obj->value[3] != 0 )
      send_to_char( "You smell poisonous fumes.\n\r", ch );
    else
      send_to_char( "It looks delicious.\n\r", ch );
  }
  else {
    send_to_char( "It doesn't look poisoned.\n\r", ch );
  }

  return;
}

void spell_dispel_evil( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  char buf[MAX_STRING_LENGTH];
  
  if ( !IS_NPC(ch) && IS_EVIL(ch) )
    victim = ch;
  
  if ( IS_GOOD(victim) ) {
    // Modified by SinaC 2001 for god
    //act( "Mota protects $N.", ch, NULL, victim, TO_ROOM );
    sprintf( buf,
	     "%s protects $N.",
	     char_god_name( ch ));//IS_NPC(victim)?"Mota":god_name(victim->pcdata->god) );
    act( buf, ch, NULL, victim, TO_ROOM );
    return;
  }

  if ( IS_NEUTRAL(victim) ) {
    act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (victim->hit > (ch->level * 4))
    dam = dice( level, 4 );
  else
    dam = UMAX(victim->hit, dice(level,4));
  if ( saves_spell( level, victim,DAM_HOLY) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);
  return;
}

void spell_dispel_good( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  if ( !IS_NPC(ch) && IS_GOOD(ch) )
    victim = ch;
 
  if ( IS_EVIL(victim) ) {
    act( "$N is protected by $S evil.", ch, NULL, victim, TO_ROOM );
    return;
  }
 
  if ( IS_NEUTRAL(victim) ) {
    act( "$N does not seem to be affected.", ch, NULL, victim, TO_CHAR );
    return;
  }
 
  if (victim->hit > (ch->level * 4))
    dam = dice( level, 4 );
  else
    dam = UMAX(victim->hit, dice(level,4));
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
  return;
}

/* modified for enhanced use */

void spell_dispel_magic( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  bool found = FALSE;

  if (saves_spell(level, victim,DAM_OTHER)) {
    send_to_char( "You feel a brief tingling sensation.\n\r",victim);
    send_to_char( "You failed.\n\r", ch);
    return;
  }

  // the following lines replace all these tests
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    if ( ability_table[i].dispellable
	 && check_dispel(level,victim,i ) ) {
      found = TRUE;
      if ( ability_table[i].msg_dispel )
	act(ability_table[i].msg_dispel,victim,NULL,NULL,TO_ROOM);
    }
  }


/*
  // begin running through the spells

  if (check_dispel(level,victim,ability_lookup("armor")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("bless")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("blindness"))) {
    found = TRUE;
    act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,ability_lookup("calm"))) {
    found = TRUE;
    act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,ability_lookup("change sex"))) {
    found = TRUE;
    act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,ability_lookup("charm person"))) {
    found = TRUE;
    act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,ability_lookup("chill touch"))) {
    found = TRUE;
    act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
  }
 
  if (check_dispel(level,victim,ability_lookup("curse")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("detect evil")))
    found = TRUE;

  if (check_dispel(level,victim,ability_lookup("detect good")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("detect hidden")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("detect invis")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("detect magic")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("faerie fire"))) {
    act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("fly"))) {
    act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("frenzy"))) {
    act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("giant strength"))) {
    act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("haste"))) {
    act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("infravision")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("invis"))) {
    act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("mass invis"))) {
    act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("pass door")))
    found = TRUE;
 

  if (check_dispel(level,victim,ability_lookup("protection evil")))
    found = TRUE;

  if (check_dispel(level,victim,ability_lookup("protection good")))
    found = TRUE;
 
  if (check_dispel(level,victim,ability_lookup("sanctuary"))) {
    act("The white aura around $n's body vanishes.",
	victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  // Removed by SinaC 2000, now a mob with an AFF_SANCTUARY in base_affect
  //  will have an sanctuary affect spell with a duration of -1  
  
  //        if (IS_AFFECTED(victim,AFF_SANCTUARY) 
  //    	&& !saves_dispel(level, victim->level,-1)
  //    	&& !is_affected(victim,ability_lookup("sanctuary")))
  //          {
  //    	// ???Oxtal Should remove an affect here
  //    	REMOVE_BIT(victim->bstat(affected_by),AFF_SANCTUARY);
  //    	// Added by SinaC 2000
  //    	REMOVE_BIT(victim->cstat(affected_by),AFF_SANCTUARY);
  
  //    	//recompute(ch);
  
  //            act("The white aura around $n's body vanishes.",
  //                victim,NULL,NULL,TO_ROOM);
  //            found = TRUE;
  //          }
  

  if (check_dispel(level,victim,ability_lookup("shield"))) {
    act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("sleep")))
    found = TRUE;

  if (check_dispel(level,victim,ability_lookup("slow"))) {
    act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("stone skin"))) {
    act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
 
  if (check_dispel(level,victim,ability_lookup("weaken"))) {
    act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }

  // modified by Sinac 1997

  if (check_dispel(level,victim,ability_lookup("aid")))
    found = TRUE;
        
  if (check_dispel(level,victim,ability_lookup("levitation"))) {
    act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }    
    
  if (check_dispel(level,victim,ability_lookup("blur"))) {
    act("$n is no longer distorted!",victim,NULL,NULL,TO_ROOM);
    found = TRUE;
  }
    
  if (check_dispel(level,victim,ability_lookup("combat mind")))
    found = TRUE;
        
  if (check_dispel(level,victim,ability_lookup("hesitation")))
    found = TRUE;    
        
  if (check_dispel(level,victim,ability_lookup("trouble")))
    found = TRUE;        
 
  if (check_dispel(level,victim,ability_lookup("dizziness")))
    found = TRUE;
*/ 

  if (found)
    send_to_char("Ok.\n\r",ch);
  else
    send_to_char("Spell failed.\n\r",ch);
  return;
}

/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
void spell_old_energy_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (victim != ch)
    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000 for alignment/etho
    //ch->align.alignment = UMAX(-1000, ch->align.alignment - 50);
    ch->bstat(alignment) = UMAX(-1000, ch->bstat(alignment) - 50);

  if ( saves_spell( level, victim,DAM_NEGATIVE) ) {
    send_to_char("You feel a momentary chill.\n\r",victim);

    // Added by SinaC 2000
    act( "$n feels a momentary chill.", victim, NULL, NULL, TO_NOTVICT );
    return;
  }

  if ( victim->level <= 2 )
    dam		 = ch->hit + 1;
  else
    dam		 = dice(1, level);
  int done = ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);

  if ( done != DAMAGE_DONE )
    return;

  send_to_char("You feel your life slipping away!\n\r",victim);
  send_to_char("Wow....what a rush!\n\r",ch);

  if( victim->level > 2 ) {
    gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ), TRUE );
    victim->mana	/= 2;
    // Added by SinaC 2001 for mental user
    victim->psp 	/= 2;
    victim->move	/= 2;
    ch->hit		+= dam;
  }
  return;
}


void spell_fireproof(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA af;
 
  if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) {
    act("$p is already protected from burning.",ch,obj,NULL,TO_CHAR);
    return;
  }

  //afsetup(af,OBJECT,NA,OR,ITEM_BURN_PROOF,number_fuzzy(level / 4),level,sn);
  //affect_to_obj(obj,&af);
  createaff(af,number_fuzzy(level / 4),level,sn,0,AFFECT_ABILITY);
  addaff(af,OBJECT,NA,OR,ITEM_BURN_PROOF);
  affect_to_obj( obj, &af );
 
  act("You protect $p from fire.",ch,obj,NULL,TO_CHAR);
  act("$p is surrounded by a protective aura.",ch,obj,NULL,TO_ROOM);
}


void spell_fly( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FLYING) ) {
    if (victim == ch)
      send_to_char("You are already airborne.\n\r",ch);
    else
      act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
    return;
  }
  //afsetup(af,CHAR,affected_by,OR,AFF_FLYING,level+3,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level+3,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_FLYING);
  affect_to_char( victim, &af );

  send_to_char( "Your feet rise off the ground.\n\r", victim );
  act( "$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM );
  return;
}

/* RT clerical berserking spell */

void spell_frenzy(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected(victim,sn) 
       || IS_AFFECTED(victim,AFF_BERSERK)) {
    if (victim == ch)
      send_to_char("You are already in a frenzy.\n\r",ch);
    else
      act("$N is already in a frenzy.",ch,NULL,victim,TO_CHAR);
    return;
  }

  if (is_affected(victim,gsn_calm)
      || IS_AFFECTED(victim,AFF_CALM)) {
    if (victim == ch)
      send_to_char("Why don't you just relax for a while?\n\r",ch);
    else
      act("$N doesn't look like $e wants to fight anymore.",
	  ch,NULL,victim,TO_CHAR);
    return;
  }

  /* Oxtal> I suggest replacing it by abs(ch->align-victim->align) > 200 */
  if ((IS_GOOD(ch) && !IS_GOOD(victim)) ||
      (IS_NEUTRAL(ch) && !IS_NEUTRAL(victim)) ||
      (IS_EVIL(ch) && !IS_EVIL(victim)) ) {
    act("Your god doesn't seem to like $N",ch,NULL,victim,TO_CHAR);
    return;
  }

  // Added by SinaC 2001, thanks Raph (aka Ypnoxyl)
  int etho_bonus = 0;
  if ( IS_CHAOTIC( victim ) || IS_LAWFUL( victim ) )
    etho_bonus = level/10;

  //afsetup(af,CHAR,hitroll,ADD,level/6+etho_bonus,level/3,level,sn);
  //affect_to_char(victim,&af);
  //af.location  = ATTR_damroll;
  //affect_to_char(victim,&af);
  //af.modifier  = 10 * (level / 12)-etho_bonus;
  //af.location  = ATTR_allAC;
  //affect_to_char(victim,&af);
  createaff(af,level/3,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,10 * (level / 12)-etho_bonus);
  addaff(af,CHAR,hitroll,ADD,level/6+etho_bonus);
  addaff(af,CHAR,damroll,ADD,level/6+etho_bonus);
  affect_to_char( victim, &af );

  send_to_char("You are filled with holy wrath!\n\r",victim);
  act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
}

/* RT ROM-style gate */
    
void spell_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
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
    
  act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You step through a gate and vanish.\n\r",ch);
  char_from_room(ch);
  char_to_room(ch,victim->in_room);

  act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
  do_look(ch,"auto");

  if (gate_pet) {
    act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch->pet);
    char_from_room(ch->pet);
    char_to_room(ch->pet,victim->in_room);
    act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
    do_look(ch->pet,"auto");
  }
}

void spell_giant_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
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
  //  level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,STR,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));
  affect_to_char( victim, &af );

  send_to_char( "Your muscles surge with heightened power!\n\r", victim );
  act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = UMAX(  20, victim->hit - dice(1,4) );
  // previous values were UMIN( 50,   and   UMIN( 100,
  if ( saves_spell( level, victim,DAM_HARM) )
    dam = UMIN( 150, dam / 2 );
  dam = UMIN( 300, dam );
  ability_damage( ch, victim, dam, sn, DAM_HARM ,TRUE, TRUE);
  return;
}


void spell_heal( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  // Modified by SinaC  +victim->level
  victim->hit = UMIN( victim->hit + 100+victim->level, victim->cstat(max_hit) );
  update_pos( victim );
  send_to_char( "A warm feeling fills your body.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_heat_metal( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  OBJ_DATA *obj_lose, *obj_next;
  int dam = 0;
  bool fail = TRUE;

  // Added by SinaC 2001
  bool drop = FALSE;

  int 
    min = 1,
    max = level*2;
  min = 1;
  min = UMIN( min, 2*level );
  
  if (!saves_spell(level + 2,victim,DAM_FIRE) 
      && !IS_SET(victim->cstat(imm_flags),IRV_FIRE)) {
    for ( obj_lose = victim->carrying;
	  obj_lose != NULL; 
	  obj_lose = obj_next) {
      obj_next = obj_lose->next_content;
      struct material_type *mat = &(material_table[obj_lose->material]);
      // Modified by SinaC 2001, was number_range(1,2*level) before
      if ( number_range(min,max) > obj_lose->level 
	   && !saves_spell(level,victim,DAM_FIRE)
	   // Removed by SinaC 2001, use material table instead
	   //&& !IS_OBJ_STAT(obj_lose,ITEM_NONMETAL)
	   && mat->metallic
	   && !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF)
	   // Added by SinaC 2001, modified by SinaC 2001
	   && !IS_SET( mat->res, IRV_FIRE )
	   && !IS_SET( mat->imm, IRV_FIRE )
	   && !IS_SET( obj_lose->extra_flags, ITEM_NOCOND )
	   ) {
	switch ( obj_lose->item_type ) {
	case ITEM_ARMOR:
	  if (obj_lose->wear_loc != -1) {/* remove the item */
	    if (can_drop_obj(victim,obj_lose)
		&& (obj_lose->weight / 10) < 
		number_range(1,2 * victim->cstat(DEX))
		&&  remove_obj( victim, obj_lose->wear_loc, TRUE )) {
	      act("$n yells and throws $p to the ground!",
		  victim,obj_lose,NULL,TO_ROOM);
	      act("You remove and drop $p before it burns you.",
		  victim,obj_lose,NULL,TO_CHAR);
	      dam += (number_range(1,obj_lose->level) / 3);
	      obj_from_char(obj_lose);
	      obj_to_room(obj_lose, victim->in_room);
	      OBJPROG(obj_lose,victim,"onDropped",victim); // Added by SinaC 2003
	      fail = FALSE;
	      // Added by SinaC 2001
	      drop = TRUE;
	    }
	    else {/* stuck on the body! ouch! */
	      act("Your skin is seared by $p!",
		  victim,obj_lose,NULL,TO_CHAR);
	      dam += (number_range(1,obj_lose->level));
	      fail = FALSE;
	    }

	  }
	  else {/* drop it if we can */
	    if (can_drop_obj(victim,obj_lose)) {
	      act("$n yells and throws $p to the ground!",
		  victim,obj_lose,NULL,TO_ROOM);
	      act("You drop $p before it burns you.",
		  victim,obj_lose,NULL,TO_CHAR);
	      dam += (number_range(1,obj_lose->level) / 6);
	      obj_from_char(obj_lose);
	      obj_to_room(obj_lose, victim->in_room);
	      OBJPROG(obj_lose,victim,"onDropped",victim); // Added by SinaC 2003
	      fail = FALSE;
	      // Added by SinaC 2001
	      drop = TRUE;
	    }
	    else {/* cannot drop */
	      act("Your skin is seared by $p!",
		  victim,obj_lose,NULL,TO_CHAR);
	      dam += (number_range(1,obj_lose->level) / 2);
	      fail = FALSE;
	    }
	  }
	  break;
	case ITEM_WEAPON:
	  if (obj_lose->wear_loc != -1) {/* try to drop it */
	    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
	      continue;

	    if (can_drop_obj(victim,obj_lose) 
		&&  remove_obj(victim,obj_lose->wear_loc,TRUE)) {
	      act("$n is burned by $p, and throws it to the ground.",
		  victim,obj_lose,NULL,TO_ROOM);
	      send_to_char(
			   "You throw your red-hot weapon to the ground!\n\r",
			   victim);
	      dam += 1;
	      obj_from_char(obj_lose);
	      obj_to_room(obj_lose,victim->in_room);
	      OBJPROG(obj_lose,victim,"onDropped",victim); // Added by SinaC 2003
	      fail = FALSE;
	      // Added by SinaC 2001
	      drop = TRUE;
	    }
	    else {/* YOWCH! */
	      send_to_char("Your weapon sears your flesh!\n\r",
			   victim);
	      dam += number_range(1,obj_lose->level);
	      fail = FALSE;
	    }
	  }
	  else {/* drop it if we can */
	    if (can_drop_obj(victim,obj_lose)) {
	      act("$n throws a burning hot $p to the ground!",
		  victim,obj_lose,NULL,TO_ROOM);
	      act("You and drop $p before it burns you.",
		  victim,obj_lose,NULL,TO_CHAR);
	      dam += (number_range(1,obj_lose->level) / 6);
	      obj_from_char(obj_lose);
	      obj_to_room(obj_lose, victim->in_room);
	      OBJPROG(obj_lose,victim,"onDropped",victim); // Added by SinaC 2003
	      fail = FALSE;
	      // Added by SinaC 2001
	      drop = TRUE;
	    }
	    else {/* cannot drop */
	      act("Your skin is seared by $p!",
		  victim,obj_lose,NULL,TO_CHAR);
	      dam += (number_range(1,obj_lose->level) / 2);
	      fail = FALSE;
	    }
	  }
	  break;
	}
      }
    }
  } 
  if (fail) {
    send_to_char("Your spell had no effect.\n\r", ch);
    send_to_char("You feel momentarily warmer.\n\r",victim);
  }
  else {/* damage! */
    if (saves_spell(level,victim,DAM_FIRE))
      dam = 2 * dam / 3;
    ability_damage(ch,victim,dam,sn,DAM_FIRE,TRUE, TRUE);
  }

  // Added by SinaC 2001
  if ( drop )
    recomproom(ch->in_room);
}

/* RT really nasty high-level attack spell */
// Modified by SinaC 2003
void spell_holy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, divine_num, proevil_num;
   
  bless_num = gsn_bless;
  curse_num = gsn_curse; 
  divine_num = ability_lookup("divine intervention"); // frenzy before
  proevil_num = gsn_protection_evil; 

  act("$n utters a word of divine power!",ch,NULL,NULL,TO_ROOM);
  send_to_char("You utter a word of divine power.\n\r",ch);

  // Added by SinaC 2003
  if (IS_EVIL(ch) ) {
    CHAR_DATA *victim = ch;
    send_to_char("The energy explodes inside you!\n\r",ch);
    
    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_HOLY) )
      dam /= 2;
    
    int align = victim->cstat(alignment);
    align -= 350;
    
    if (align < -1000)
      align = -1000 + (align + 1000) / 3;
    
    dam = (dam * align * align) / 1000000;
    
    ability_damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);
    return;
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if (IS_GOOD(vch) 
	|| IS_NEUTRAL(vch) ) {
      send_to_char("You feel full more powerful.\n\r",vch);

      // Modified by SinaC 2001
      spell_divine_intervention(divine_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
      spell_bless(bless_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
      spell_protection_evil(proevil_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
    }

    else {
      if (!is_safe_spell(ch,vch,TRUE)) {
	send_to_char("You are struck down!\n\r",vch);
	dam = dice(level,6);
	int done = ability_damage(ch,vch,dam,sn,DAM_HOLY,TRUE, TRUE);

	// Modified by SinaC 2001
	if (done == DAMAGE_DONE)
	  spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
      }
    }
  }
    
  send_to_char("You feel drained.\n\r",ch);
  ch->move = 0;
  ch->hit /= 2;
}
 
void spell_identify( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  // Added by SinaC 2000 for object restriction
  RESTR_DATA *restr;
  // Added by SinaC 2000 for skill/spell upgrade
  ABILITY_UPGRADE *upgr;

  if ( IS_OBJ_STAT(obj, ITEM_NOIDENT) ) {
    send_to_char("You don't understand this object concepts.\n\r", ch );
    return;
  }
// Modified by SinaC 2001
  sprintf( buf,
	   "Object '%s' is type %s, extra flags %s.\n\r"
	   "Weight is %d, value is %d, level is %d, condition is %s.\n\r"
	   // Added by SinaC 2001, size added by SinaC 2003
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
    /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
  case ITEM_GRENADE:
    sprintf( buf, "Timer: %d  Damage: %d  %s\n\r",
	     obj->value[0],
	     obj->value[2],
	     obj->value[1]==GRENADE_PULLED? 
	     "Pulled":obj->value[1]==GRENADE_NOTPULLED?"Not pulled":"bad value");
    send_to_char( buf, ch );
    break;
    */
    /* Removed by SinaC 2003
    // Added by SinaC 2000 for throwing item
  case ITEM_THROWING:
    sprintf( buf, "Damage is %dd%d (average %d)\n\rDamage type: %s.\n\r", 
	     obj->value[0],
	     obj->value[1],
	     (1 + obj->value[1]) * obj->value[0] / 2,
	     (obj->value[2] > 0 && obj->value[2] < MAX_DAMAGE_MESSAGE) ?
	     attack_table[obj->value[2]].noun : "undefined");
    if ( obj->value[4]>=0 && obj->value[4]<MAX_ABILITY )
      sprintf( buf, "%sSpell added: %s (level %d)\n\r",
	       buf,
	       ability_table[obj->value[4]].name,
	       obj->value[3]);
    else
      sprintf( buf, "%s No spell added.\n\r", buf );
    send_to_char(buf, ch );
    break;
    */
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
  // Modified by SinaC 2000
  /*  before :
   * if (!obj->...)
   *    for ( paf...
   *  else
   *    for ( paf...
   */
  for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
    // Modified by SinaC 2001
    //afstring(buf,paf,ch,TRUE);
    // SinaC 2003: new affect system
    afstring( buf, paf, ch, FALSE );
    send_to_char(buf,ch);
  }

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next ) {
      // Modified by SinaC 2001
      //afstring(buf,paf,ch,TRUE);
      // SinaC 2003: new affect system
      afstring( buf, paf, ch, FALSE );
      send_to_char(buf,ch);
    }

  // Added by SinaC 2000 for object restriction
  //  for ( restr = obj->restriction; restr != NULL; restr = restr->next ){
  //    restrstring( buf, restr );
  //    send_to_char( buf, ch );
  //  }
  
  // Added by SinaC 2001
  //  if (!obj->enchanted)
    for ( restr = obj->pIndexData->restriction; restr != NULL; restr = restr->next ){
      restrstring( buf, restr );
      send_to_char( buf, ch );
    }
  
  // Added by SinaC 2000 for ability upgrade
    //  for ( upgr = obj->upgrade; upgr != NULL; upgr = upgr->next ){
    //    abilityupgradestring( buf, upgr );
    //    send_to_char( buf, ch );
    //  }
  // Added by SinaC 2003
    //  if (!obj->enchanted)
    for ( upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next ){
      abilityupgradestring( buf, upgr );
      send_to_char( buf, ch );
    }

  return;
}

void spell_infravision( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_INFRARED) ) {
    if (victim == ch)
      send_to_char("You can already see in the dark.\n\r",ch);
    else
      act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
    return;
  }
  act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

  //afsetup(af,CHAR,affected_by,OR,AFF_INFRARED,2*level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,2*level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_INFRARED);
  affect_to_char( victim, &af );

  send_to_char( "Your eyes glow red.\n\r", victim );
  return;
}

void spell_invis( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  /* object invisibility */
  if (target == TARGET_OBJ) {
    obj = (OBJ_DATA *) vo;	

    if (IS_OBJ_STAT(obj,ITEM_INVIS)) {
      act("$p is already invisible.",ch,obj,NULL,TO_CHAR);
      return;
    }
	
    //afsetup(af,OBJECT,NA,OR,ITEM_INVIS,12+level,level,sn);
    //affect_to_obj(obj,&af);
    createaff(af,12+level,level,sn,0,AFFECT_ABILITY);
    addaff(af,OBJECT,NA,OR,ITEM_INVIS);
    affect_to_obj( obj, &af );

    act("$p fades out of sight.",ch,obj,NULL,TO_ALL);
    return;
  }

  /* character invisibility */
  victim = (CHAR_DATA *) vo;

  // modified by SinaC 2000
  //  to allow to cast invisibility even if we already are invis due to an item
  if ( IS_AFFECTED(victim, AFF_INVISIBLE))
    return;

  // Added by SinaC 2001
  if ( IS_AFFECTED2( victim, AFF2_FAERIE_FOG ) ) {
    if ( ch == victim )
      send_to_char("Something prevents you from becoming invisible.\n\r", ch );
    else
      act( "Something prevents $N from becoming invisible.", 
	   ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_INVISIBLE,12+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,12+level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_INVISIBLE);
  affect_to_char( victim, &af );

  if ( IS_AFFECTED(victim, AFF_INVISIBLE ) ) { // so we are becoming invis cos' of the spell
    act( "$n fades out of existence.", victim, NULL, NULL, TO_ROOM );
    send_to_char( "You fade out of existence.\n\r", victim );
  }
  return;
}

void spell_know_alignment(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char *msg;
  char *msg_etho;
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

void spell_locate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = IS_IMMORTAL(ch) ? 200 : 2 * level;

  buffer = new_buf();
 
  for ( obj = object_list; obj != NULL; obj = obj->next ) {
    if ( !can_see_obj( ch, obj ) 
	 || !is_name( target_name, obj->name ) 
	 ||   IS_OBJ_STAT(obj,ITEM_NOLOCATE) 
	 || number_percent() > 2 * level
	 ||   ch->level < obj->level)
      continue;

    found = TRUE;
    number++;

    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
      ;

    if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)) {
      sprintf( buf, "%s is carried by %s\n\r",
	       obj->short_descr, PERS(in_obj->carried_by, ch) );
    }
    else {
      if (IS_IMMORTAL(ch) && in_obj->in_room != NULL)
	sprintf( buf, "%s is in %s [Room %d]\n\r",
		 obj->short_descr,
		 in_obj->in_room->name, in_obj->in_room->vnum);
      else 
	sprintf( buf, "%s is in %s\n\r",
		 obj->short_descr, in_obj->in_room == NULL
		 ? "somewhere" : in_obj->in_room->name );
    }

    buf[0] = UPPER(buf[0]);
    add_buf(buffer,buf);

    if (number >= max_found)
      break;
  }

  if ( !found )
    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);

  return;
}


void spell_mass_healing(int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level)
{
  CHAR_DATA *gch;
  int heal_num, refresh_num;
    
  heal_num = ability_lookup("heal");
  refresh_num = ability_lookup("refresh"); 

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
    if ((IS_NPC(ch) && IS_NPC(gch)) 
	|| (!IS_NPC(ch) && !IS_NPC(gch))) {

      // Modified by SinaC 2001
      spell_heal(heal_num,level,ch,(void *) gch,TARGET_CHAR, 1);
      spell_refresh(refresh_num,level,ch,(void *) gch,TARGET_CHAR, 1 );  
    }
  }
}
	    
void spell_mass_invis( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  AFFECT_DATA af;
  CHAR_DATA *gch;

  //afsetup(af,CHAR,affected_by,OR,AFF_INVISIBLE,24,level/2,sn);
  createaff(af,24,level/2,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_INVISIBLE);

  for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
    if ( !is_same_group( gch, ch ) 
	 || IS_AFFECTED(gch, AFF_INVISIBLE) )
      continue;
    act( "$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM );
    send_to_char( "You slowly fade out of existence.\n\r", gch );

    affect_to_char( gch, &af );
  }
  send_to_char( "Ok.\n\r", ch );

  return;
}

void spell_pass_door( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_PASS_DOOR) ) {
    if (victim == ch)
      send_to_char("You are already out of phase.\n\r",ch);
    else
      act("$N is already shifted out of phase.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_PASS_DOOR,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 4 ),level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_PASS_DOOR);
  affect_to_char( victim, &af );

  act( "$n turns translucent.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You turn translucent.\n\r", victim );
  return;
}

/* RT plague spell, very nasty */

void spell_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (saves_spell(level,victim,DAM_DISEASE) 
      || (IS_NPC(victim) 
	  && ( IS_SET(victim->act,ACT_UNDEAD)
	       || IS_SET(victim->cstat(form),FORM_UNDEAD)))) {
    if (ch == victim)
      send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
    else
      act("$N seems to be unaffected.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_PLAGUE,level,level*3/4,sn);
  //affect_join(victim,&af);
  //afsetup(af,CHAR,STR,ADD,-5,level,level*3/4,sn);
  //affect_join(victim,&af);
  createaff(af,level,3*level/4,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_PLAGUE);
  addaff(af,CHAR,STR,ADD,-5);
  affect_join( victim, &af );
   
  send_to_char
    ("You scream in agony as plague sores erupt from your skin.\n\r",victim);
  act("$n screams in agony as plague sores erupt from $s skin.",
      victim,NULL,NULL,TO_ROOM);
}

void spell_protection_evil(int sn,int level,CHAR_DATA *ch,void *vo, int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
       || IS_AFFECTED(victim, AFF_PROTECT_GOOD)) {
    if (victim == ch)
      send_to_char("You are already protected.\n\r",ch);
    else
      act("$N is already protected.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_PROTECT_EVIL,24,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,saving_throw,ADD,-1,24,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,24,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_PROTECT_EVIL);
  addaff(af,CHAR,saving_throw,ADD,-1);
  affect_to_char( victim, &af );

  send_to_char( "You feel holy and pure.\n\r", victim );
  if ( ch != victim )
    act("$N is protected from evil.",ch,NULL,victim,TO_CHAR);
  return;
}
 
void spell_protection_good(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( IS_AFFECTED(victim, AFF_PROTECT_GOOD) 
       || IS_AFFECTED(victim, AFF_PROTECT_EVIL)) {
    if (victim == ch)
      send_to_char("You are already protected.\n\r",ch);
    else
      act("$N is already protected.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,affected_by,OR,AFF_PROTECT_GOOD,24,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,saving_throw,ADD,-1,24,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,24,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_PROTECT_GOOD);
  addaff(af,CHAR,saving_throw,ADD,-1);
  affect_to_char( victim, &af );

  send_to_char( "You feel aligned with darkness.\n\r", victim );
  if ( ch != victim )
    act("$N is protected from good.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_ray_of_truth (int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;
 
  if (IS_EVIL(ch) ) {
    victim = ch;
    send_to_char("The energy explodes inside you!\n\r",ch);
  }

  if (victim != ch) {
    act("$n raises $s hand, and a blinding ray of light shoots forth!",
	ch,NULL,NULL,TO_ROOM);
    send_to_char(
		 "You raise your hand and a blinding ray of light shoots forth!\n\r",
		 ch);
  }

  if (IS_GOOD(victim)) {
    act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
    send_to_char("The light seems powerless to affect you.\n\r",victim);
    return;
  }

  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_HOLY) )
    dam /= 2;

  // Modified by SinaC 2001 etho/alignment are attributes now
  // Modified by SinaC 2000 for alignment/etho
  //align = victim->align.alignment;
  align = victim->cstat(alignment);
  align -= 350;

  if (align < -1000)
    align = -1000 + (align + 1000) / 3;

  dam = (dam * align * align) / 1000000;

  int done = ability_damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);

  // Modified by SinaC 2001
  if ( done == DAMAGE_DONE )
    spell_blindness(gsn_blindness, 
		    3 * level / 4, ch, (void *) victim,TARGET_CHAR, 1 );
}

void spell_recharge( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int chance, percent;

  if (obj->item_type != ITEM_WAND 
      && obj->item_type != ITEM_STAFF) {
    send_to_char("That item does not carry charges.\n\r",ch);
    return;
  }

  if (obj->value[3] >= 3 * level / 2) {
    send_to_char("Your skills are not great enough for that.\n\r",ch);
    return;
  }

  if (obj->value[1] == 0) {
    send_to_char("That item has already been recharged once.\n\r",ch);
    return;
  }

  chance = 40 + 2 * level;

  chance -= obj->value[3]; /* harder to do high-level spells */
  chance -= (obj->value[1] - obj->value[2]) *
    (obj->value[1] - obj->value[2]);

  chance = UMAX(level/2,chance);

  percent = number_percent();

  if (percent < chance / 2) {
    act("$p glows softly.",ch,obj,NULL,TO_CHAR);
    act("$p glows softly.",ch,obj,NULL,TO_ROOM);
    obj->baseval[2] = UMAX(obj->value[1],obj->value[2]);
    obj->baseval[1] = 0;
    recompobj(obj);
    return;
  }

  else if (percent <= chance) {
    int chargeback,chargemax;

    act("$p glows softly.",ch,obj,NULL,TO_CHAR);
    act("$p glows softly.",ch,obj,NULL,TO_CHAR);

    chargemax = obj->value[1] - obj->value[2];
	
    if (chargemax > 0)
      chargeback = UMAX(1,chargemax * percent / 100);
    else
      chargeback = 0;

    obj->baseval[2] += chargeback;
    obj->baseval[1] = 0;
    recompobj(obj);
    return;
  }	

  else if (percent <= UMIN(95, 3 * chance / 2)) {
    send_to_char("Nothing seems to happen.\n\r",ch);
    if (obj->value[1] > 1)
      obj->value[1]--;
    return;
  }

  else {/* whoops! */
    act("$p glows brightly and explodes!",ch,obj,NULL,TO_CHAR);
    act("$p glows brightly and explodes!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
  }
}

void spell_refresh( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  victim->move = UMIN( victim->move + UMAX(3*level/2,40), victim->cstat(max_move) );
  if (victim->cstat(max_move) == victim->move)
    send_to_char("You feel fully refreshed!\n\r",victim);
  else
    send_to_char( "You feel less tired.\n\r", victim );
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_remove_curse( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  bool found = FALSE;

  /* do object cases first */
  if (target == TARGET_OBJ) {
    obj = (OBJ_DATA *) vo;

    if (IS_OBJ_STAT(obj,ITEM_NODROP) 
	|| IS_OBJ_STAT(obj,ITEM_NOREMOVE)) {
      if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
	  //	  &&  !saves_dispel(level + 2,obj->level,0)) {
	  &&  !saves_dispel(level + 2,NULL,obj->level,0)) {
	REM_OBJ_STAT(obj,ITEM_NODROP|ITEM_NOREMOVE);
	act("$p glows blue.",ch,obj,NULL,TO_ALL);
	return;
      }

      act("The curse on $p is beyond your power.",ch,obj,NULL,TO_CHAR);
      return;
    }
    act("There doesn't seem to be a curse on $p.",ch,obj,NULL,TO_CHAR);
    return;
  }

  /* characters */
  victim = (CHAR_DATA *) vo;

  if (check_dispel(level,victim,gsn_curse)) {
    send_to_char("You feel better.\n\r",victim);
    act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
  }

  for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content) {
    if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
	&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE)) {   /* attempt to remove curse */
      //      if (!saves_dispel(level,obj->level,0)) {
      if (!saves_dispel(level,NULL,obj->level,0)) {
	found = TRUE;
	REM_OBJ_STAT(obj,ITEM_NODROP|ITEM_NOREMOVE);
	act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
	act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
      }
    }
  }
}

void spell_sanctuary( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level)
{
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
  createaff(af,level/6,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  affect_to_char( victim, &af );

  act( "$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a white aura.\n\r", victim );
  return;
}

void spell_stone_skin( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("Your skin is already as hard as a rock.\n\r",ch); 
    else
      act("$N is already as hard as can be.",ch,NULL,victim,TO_CHAR);
    return;
  }

  // Modified by SinaC 2003  -level/2
  //afsetup(af,CHAR,allAC,ADD,-40-level/2,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,-40-level/2);
  affect_to_char( victim, &af );

  act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "Your skin turns to stone.\n\r", victim );
  return;
}

void spell_summon( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim;

  if ( ( victim = get_char_world( ch, target_name ) ) == NULL
       || victim == ch
       || victim->in_room == NULL
       // Modified by SinaC 2001
       || IS_SET(ch->in_room->cstat(flags), ROOM_SAFE)
       || IS_SET(victim->in_room->cstat(flags), ROOM_SAFE)
       || IS_SET(victim->in_room->cstat(flags), ROOM_PRIVATE)
       || IS_SET(victim->in_room->cstat(flags), ROOM_SOLITARY)
       || IS_SET(victim->in_room->cstat(flags), ROOM_NO_RECALL)
       || (IS_NPC(victim) && IS_SET(victim->act,ACT_AGGRESSIVE))
       // Added by SinaC 2001
       ||   IS_SET(victim->act, ACT_IS_SAFE )
       || victim->level >= level + 3
       || (!IS_NPC(victim) && IS_IMMORTAL(victim))
       || victim->fighting != NULL
       || (IS_NPC(victim) && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       || (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
       || (!IS_NPC(victim) && IS_SET(victim->act,PLR_NOSUMMON))
       || (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER)) ) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  act( "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, ch->in_room );
  act( "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM );
  act( "$n has summoned you!", ch, NULL, victim,   TO_VICT );
  do_look( victim, "auto" );
  return;
}

void spell_teleport( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  ROOM_INDEX_DATA *pRoomIndex;

  if ( victim->in_room == NULL
       // Modified by SinaC 2001
       ||   IS_SET(victim->in_room->cstat(flags), ROOM_NO_RECALL)
       || ( victim != ch && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       || ( !IS_NPC(ch) && victim->fighting != NULL )
       || ( victim != ch  && ( saves_spell( level - 5, victim,DAM_OTHER)))) {
    send_to_char( "You failed.\n\r", ch );
    return;
  }

  pRoomIndex = get_random_room(victim);

  if (victim != ch)
    send_to_char("You have been teleported!\n\r",victim);

  // Added by SinaC 2000
  if ( victim->fighting != NULL )
    stop_fighting( victim, FALSE );

  act( "$n vanishes!", victim, NULL, NULL, TO_ROOM );
  char_from_room( victim );
  char_to_room( victim, pRoomIndex );
  act( "$n slowly fades into existence.", victim, NULL, NULL, TO_ROOM );
  do_look( victim, "auto" );

  // Added by SinaC 2003
  MOBPROG(victim,NULL,"onMoved");
  return;
}

void spell_ventriloquate( int sn, int level, CHAR_DATA *ch,void *vo,int target, int casting_level) {
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char speaker[MAX_INPUT_LENGTH];
  // Added by SinaC 2001
  const char *tname;
  CHAR_DATA *vch;

  // Modified by SinaC 2001
  //target_name = one_argument( target_name, speaker );
  tname = one_argument( target_name, speaker );
  sprintf( buf1, "%s says '%s'.\n\r",              speaker, /*target_name*/tname );
  sprintf( buf2, "Someone makes %s say '%s'.\n\r", speaker, /*target_name*/tname );
    
  buf1[0] = UPPER(buf1[0]);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room ) {
    if (!is_exact_name( speaker, vch->name) && IS_AWAKE(vch))
      send_to_char( saves_spell(level,vch,DAM_OTHER) ? buf2 : buf1, vch );
  }

  return;
}

/*
 * NPC spells.
 */
void spell_acid_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam,hpch;

  act("$n spits acid at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n spits a stream of corrosive acid at you.",ch,NULL,victim,TO_VICT);
  act("You spit acid at $N.",ch,NULL,victim,TO_CHAR);

  hpch = UMAX(12,(2*ch->hit)/3);
  hp_dam = number_range(hpch/11 + 1, hpch/6);
  dice_dam = dice(level,16);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
    
  if (saves_spell(level,victim,DAM_ACID)) {
    acid_effect(victim,level/2,dam/4,TARGET_CHAR);
    ability_damage(ch,victim,dam/2,sn,DAM_ACID,TRUE, TRUE);
  }
  else {
    acid_effect(victim,level,dam,TARGET_CHAR);
    ability_damage(ch,victim,dam,sn,DAM_ACID,TRUE, TRUE);
  }
}

void spell_fire_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam,hp_dam,dice_dam;
  int hpch;

  act("$n breathes forth a cone of fire.",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a cone of hot fire over you!",ch,NULL,victim,TO_VICT);
  act("You breath forth a cone of fire.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX( 10, (2*ch->hit)/3 );
  hp_dam  = number_range( hpch/9+1, hpch/5 );
  dice_dam = dice(level,20);

  dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
  fire_effect(victim->in_room,level,dam/2,TARGET_ROOM);

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE) 
	||  (IS_NPC(vch) && IS_NPC(ch) 
	     &&   (ch->fighting != vch || vch->fighting != ch)))
      continue;

    if (vch == victim) {/* full damage */
      if (saves_spell(level,vch,DAM_FIRE)) {
	fire_effect(vch,level/2,dam/4,TARGET_CHAR);
	ability_damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE, TRUE);
      }
      else {
	fire_effect(vch,level,dam,TARGET_CHAR);
	ability_damage(ch,vch,dam,sn,DAM_FIRE,TRUE, TRUE);
      }
    }
    else {/* partial damage */
      if (saves_spell(level - 2,vch,DAM_FIRE)) {
	fire_effect(vch,level/4,dam/8,TARGET_CHAR);
	ability_damage(ch,vch,dam/4,sn,DAM_FIRE,TRUE, TRUE);
      }
      else {
	fire_effect(vch,level/2,dam/4,TARGET_CHAR);
	ability_damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE, TRUE);
      }
    }
  }
}

void spell_frost_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *vch, *vch_next;
  int dam,hp_dam,dice_dam, hpch;

  act("$n breathes out a freezing cone of frost!",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a freezing cone of frost over you!",
      ch,NULL,victim,TO_VICT);
  act("You breath out a cone of frost.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(12,(2*ch->hit)/3);
  hp_dam = number_range(hpch/11 + 1, hpch/6);
  dice_dam = dice(level,16);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
  cold_effect(victim->in_room,level,dam/2,TARGET_ROOM); 

  for (vch = victim->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(ch) 
	     &&   (ch->fighting != vch || vch->fighting != ch)))
      continue;

    if (vch == victim) {/* full damage */
      if (saves_spell(level,vch,DAM_COLD)) {
	cold_effect(vch,level/2,dam/4,TARGET_CHAR);
	ability_damage(ch,vch,dam/2,sn,DAM_COLD,TRUE, TRUE);
      }
      else {
	cold_effect(vch,level,dam,TARGET_CHAR);
	ability_damage(ch,vch,dam,sn,DAM_COLD,TRUE, TRUE);
      }
    }
    else {
      if (saves_spell(level - 2,vch,DAM_COLD)) {
	cold_effect(vch,level/4,dam/8,TARGET_CHAR);
	ability_damage(ch,vch,dam/4,sn,DAM_COLD,TRUE, TRUE);
      }
      else {
	cold_effect(vch,level/2,dam/4,TARGET_CHAR);
	ability_damage(ch,vch,dam/2,sn,DAM_COLD,TRUE, TRUE);
      }
    }
  }
}
    
void spell_gas_breath( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level )
{
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam,hp_dam,dice_dam,hpch;

  act("$n breathes out a cloud of poisonous gas!",ch,NULL,NULL,TO_ROOM);
  act("You breath out a cloud of poisonous gas.",ch,NULL,NULL,TO_CHAR);

  hpch = UMAX(16,(2*ch->hit)/3);
  hp_dam = number_range(hpch/15+1,8);
  dice_dam = dice(level,12);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
  poison_effect(ch->in_room,level,dam,TARGET_ROOM);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    if (is_safe_spell(ch,vch,TRUE)
	||  (IS_NPC(ch) && IS_NPC(vch) 
	     &&   (ch->fighting == vch || vch->fighting == ch)))
      continue;

    if (saves_spell(level,vch,DAM_POISON)) {
      poison_effect(vch,level/2,dam/4,TARGET_CHAR);
      ability_damage(ch,vch,dam/2,sn,DAM_POISON,TRUE, TRUE);
    }
    else {
      poison_effect(vch,level,dam,TARGET_CHAR);
      ability_damage(ch,vch,dam,sn,DAM_POISON,TRUE, TRUE);
    }
  }
}

void spell_lightning_breath(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam,hp_dam,dice_dam,hpch;

  act("$n breathes a bolt of lightning at $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n breathes a bolt of lightning at you!",ch,NULL,victim,TO_VICT);
  act("You breathe a bolt of lightning at $N.",ch,NULL,victim,TO_CHAR);

  hpch = UMAX(10,(2*ch->hit)/3);
  hp_dam = number_range(hpch/9+1,hpch/5);
  dice_dam = dice(level,20);

  dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

  if (saves_spell(level,victim,DAM_LIGHTNING)) {
    shock_effect(victim,level/2,dam/4,TARGET_CHAR);
    ability_damage(ch,victim,dam/2,sn,DAM_LIGHTNING,TRUE, TRUE);
  }
  else {
    shock_effect(victim,level,dam,TARGET_CHAR);
    ability_damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE, TRUE); 
  }
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
void spell_general_purpose(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  dam = number_range( level/2, 3*level/2 );
  if ( saves_spell( level, victim, DAM_PIERCE) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, TRUE);
  return;
}

void spell_high_explosive(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level)
{
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
 
  dam = number_range( 2*level/3, level*2 );
  if ( saves_spell( level, victim, DAM_PIERCE) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, TRUE);
  return;
}
