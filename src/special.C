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
#include <time.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
//#include "magic.h"
#include "spells_def.h"

// Added by SinaC 2001
#include "act_info.h"
#include "act_move.h"
#include "interp.h"
#include "fight.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "act_obj.h"
#include "lookup.h"
#include "gsn.h"
#include "olc_value.h"
#include "combatabilities.h"
#include "special.h"
#include "act_comm.h"
#include "utils.h"


/*
 * The following special functions are available for mobiles.
 */

/* the function table */
const   struct  spec_type    spec_table[] =
{
  {	"spec_breath_any",		spec_breath_any		},
  {	"spec_breath_acid",		spec_breath_acid	},
  {	"spec_breath_fire",		spec_breath_fire	},
  {	"spec_breath_frost",		spec_breath_frost	},
  {	"spec_breath_gas",		spec_breath_gas		},
  {	"spec_breath_lightning",	spec_breath_lightning	},	
  {	"spec_cast_adept",		spec_cast_adept		},
  {	"spec_cast_cleric",		spec_cast_cleric	},
  {	"spec_cast_judge",		spec_cast_judge		},
  {	"spec_cast_mage",		spec_cast_mage		},
  {	"spec_cast_undead",		spec_cast_undead	},
  {	"spec_executioner",		spec_executioner	},
  {	"spec_fido",			spec_fido		},
  {	"spec_guard",			spec_guard		},
  {	"spec_janitor",			spec_janitor		},
  {	"spec_mayor",			spec_mayor		},
  {	"spec_poison",			spec_poison		},
  {	"spec_thief",			spec_thief		},
  {	"spec_nasty",			spec_nasty		},
  //  {     "spec_questmaster",	        spec_questmaster        },
  {     "spec_cast_necromancer",        spec_cast_necromancer   },
  {     "spec_cast_druid",	        spec_cast_druid         },
  {	NULL,				NULL			}
};

/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup( const char *name )
{
  int i;
 
  for ( i = 0; spec_table[i].name != NULL; i++){
    if (LOWER(name[0]) == LOWER(spec_table[i].name[0])
	&&  !str_prefix( name,spec_table[i].name))
      return spec_table[i].function;
  }
  
  return 0;
}

const char *spec_name( SPEC_FUN *function)
{
  int i;

  for (i = 0; spec_table[i].function != NULL; i++){
    if (function == spec_table[i].function)
      return spec_table[i].name;
  }
  
  return NULL;
}

bool spec_nasty( CHAR_DATA *ch )
{
  CHAR_DATA *victim, *v_next;
  long gold;
 
  if (!IS_AWAKE(ch)) {
    return FALSE;
  }
 
  if (ch->position != POS_FIGHTING) {
    for ( victim = ch->in_room->people; victim != NULL; victim = v_next){
      v_next = victim->next_in_room;
      if (!IS_NPC(victim)
	  && (victim->level > ch->level)
	  && (victim->level < ch->level + 10)){
	do_backstab(ch,victim->name);
	if (ch->position != POS_FIGHTING)
	  do_murder(ch,victim->name);
	/* should steal some coins right away? :) */
	return TRUE;
      }
    }
    return FALSE;    /*  No one to attack */
  }
  
  /* okay, we must be fighting.... steal some coins and flee */
  if ( (victim = ch->fighting) == NULL)
    return FALSE;   /* let's be paranoid.... */
 
  switch ( number_bits(2) ){
  case 0:  
    act( "$n rips apart your coin purse, spilling your gold!",
	 ch, NULL, victim, TO_VICT);
    act( "You slash apart $N's coin purse and gather his gold.",
	 ch, NULL, victim, TO_CHAR);
    act( "$N's coin purse is ripped apart!",
	 ch, NULL, victim, TO_NOTVICT);
    gold = victim->gold / 10;  /* steal 10% of his gold */
    victim->gold -= gold;
    ch->gold     += gold;
    return TRUE;
    
  case 1:  
    do_flee( ch, "");
    return TRUE;
    
  default: 
    return FALSE;
  }
}

/*
 * Core procedure for dragons.
 */
bool dragon( CHAR_DATA *ch, char *spell_name )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  int sn;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ) {
    v_next = victim->next_in_room;
    // Modified by SinaC 2000
    if ( victim->fighting == ch && number_bits( /*3*/ 2 ) == 0 )
      break;
  }
  
  // Added by SinaC 2000
  if (ch->stunned)
    return FALSE;

  if ( victim == NULL )
    return FALSE;

  if ( ( sn = ability_lookup( spell_name ) ) < 0 )
    return FALSE;
  // Modified by SinaC 2001
  //(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR, 1);
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim, TARGET_CHAR, 1);
  return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any( CHAR_DATA *ch )
{
  if ( ch->position != POS_FIGHTING )
    return FALSE;

  switch ( number_bits( 3 ) ){
  case 0: return spec_breath_fire		( ch );
  case 1:
  case 2: return spec_breath_lightning	( ch );
  case 3: return spec_breath_gas		( ch );
  case 4: return spec_breath_acid		( ch );
  case 5:
  case 6:
  case 7: return spec_breath_frost		( ch );
  }
  
  return FALSE;
}



bool spec_breath_acid( CHAR_DATA *ch )
{
  return dragon( ch, "acid breath" );
}



bool spec_breath_fire( CHAR_DATA *ch )
{
  return dragon( ch, "fire breath" );
}



bool spec_breath_frost( CHAR_DATA *ch )
{
  return dragon( ch, "frost breath" );
}



bool spec_breath_gas( CHAR_DATA *ch )
{
  int sn;
  if ( ch->position != POS_FIGHTING )
    return FALSE;

  // Added by SinaC 2000
    if (ch->stunned)
	return FALSE;

  if ( ( sn = ability_lookup( "gas breath" ) ) < 0 )
    return FALSE;

  // Modified by SinaC 2001
  //(*skill_table[sn].spell_fun) ( sn, ch->level, ch, NULL,TARGET_CHAR, 1);
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, NULL, TARGET_CHAR, 1);
  return TRUE;
}



bool spec_breath_lightning( CHAR_DATA *ch )
{
  return dragon( ch, "lightning breath" );
}



bool spec_cast_adept( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;

  if ( !IS_AWAKE(ch) )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    if ( victim != ch && can_see( ch, victim ) && number_bits( 1 ) == 0 
	 && !IS_NPC(victim) && victim->level < 11)
      break;
  }
  
  if ( victim == NULL )
    return FALSE;

  switch ( number_bits( 4 ) ){
  case 0:
    act( "$n utters the word 'abrazak'.", ch, NULL, NULL, TO_ROOM );

    // Modified by SinaC 2001
    spell_armor( ability_lookup( "armor" ), ch->level,ch,victim,TARGET_CHAR, 1);
    return TRUE;
    
  case 1:
    act( "$n utters the word 'fido'.", ch, NULL, NULL, TO_ROOM );

    // Modified by SinaC 2001
    spell_bless( ability_lookup( "bless" ), ch->level,ch,victim,TARGET_CHAR, 1);
    return TRUE;
    
  case 2:
    act("$n utters the words 'judicandus noselacri'.",ch,NULL,NULL,TO_ROOM);

    // Modified by SinaC 2001
    spell_cure_blindness( ability_lookup( "cure blindness" ),
			  ch->level, ch, victim,TARGET_CHAR, 1);
    return TRUE;
    
  case 3:
    act("$n utters the words 'judicandus dies'.", ch,NULL, NULL, TO_ROOM );

    // Modified by SinaC 2001
    spell_cure_light( ability_lookup( "cure light" ),
		      ch->level, ch, victim,TARGET_CHAR, 1);
    return TRUE;
    
  case 4:
    act( "$n utters the words 'judicandus sausabru'.",ch,NULL,NULL,TO_ROOM);

    // Modified by SinaC 2001
    spell_cure_poison( ability_lookup( "cure poison" ),
		       ch->level, ch, victim,TARGET_CHAR, 1);
    return TRUE;
    
  case 5:
    act("$n utters the word 'candusima'.", ch, NULL, NULL, TO_ROOM );

    // Modified by SinaC 2001
    spell_refresh( ability_lookup("refresh"),ch->level,ch,victim,TARGET_CHAR, 1);
    return TRUE;
    
  case 6:
    act("$n utters the words 'judicandus eugzagz'.",ch,NULL,NULL,TO_ROOM);

    // Modified by SinaC 2001
    spell_cure_disease(ability_lookup("cure disease"),
		       ch->level,ch,victim,TARGET_CHAR, 1);
  }
  
  return FALSE;
}



bool spec_cast_cleric( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  int target;

  // Modified by SinaC 2001
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    // Modified by SinaC 2000
    if ( victim->fighting == ch /*&& number_bits( 2 ) == 0*/ )
      break;
  }
  
  if ( victim == NULL )
    return FALSE;

  for ( ;; ){
    int min_level;
    
    // Modified by SinaC 2000
    switch ( number_bits( 4 ) ){
    case  0: min_level =  0; spell = "blindness";      target = 1; break;
    case  1: min_level =  3; spell = "cause serious";  target = 1; break;
    case  2: min_level =  7; spell = "earthquake";     target = 1; break;
    case  3: min_level =  9; spell = "cause critical"; target = 1; break;
    case  4: min_level = 10; spell = "dispel evil";    target = 1; break;
    case  5: min_level = 12; spell = "curse";          target = 1; break;
    case  6: min_level = 12; spell = "change sex";     target = 1; break;
    case  7: min_level = 13; spell = "flamestrike";    target = 1; break;
      
    case  8: min_level = 0;  spell = "heal";           target = 0; break;
    case  9: min_level = 0;  spell = "heal";           target = 0; break;
      
    case 10: min_level = 15; spell = "harm";           target = 1; break;
    case 11: min_level = 15; spell = "plague";	       target = 1; break;
    default: min_level = 16; spell = "dispel magic";   target = 1; break;
    }
    
    if ( ch->level >= min_level )
      break;
  }
  
  // Added by SinaC 2000
  if (ch->stunned)
    return FALSE;
  
  //sprintf(log_buf,"cleric_spec: %s",spell);
  //log_string(log_buf);
  
  if ( ( sn = ability_lookup( spell ) ) < 0 )
    return FALSE;
  
// Modified by SinaC 2000
  if ( target == 1 )
    // Modified by SinaC 2001
    //(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim, TARGET_CHAR, 1 );
    (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim, TARGET_CHAR, 1 );
  else
    // Modified by SinaC 2001
    //(*skill_table[sn].spell_fun) ( sn, ch->level, ch, ch, TARGET_CHAR, 1 );
    (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, ch, TARGET_CHAR, 1 );

  return TRUE;
}

bool spec_cast_judge( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  // Modified by SinaC 2001
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL ) )
    return FALSE;
 
  if ( ch->position != POS_FIGHTING )
    return FALSE;
 
  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    if ( victim->fighting == ch && number_bits( 2 ) == 0 )
      break;
  }
 
  if ( victim == NULL )
    return FALSE;

  // Added by SinaC 2000
    if (ch->stunned)
      return FALSE;
 
  spell = "high explosive";
  if ( ( sn = ability_lookup( spell ) ) < 0 )
    return FALSE;

  // Modified by SinaC 2001
  //(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  return TRUE;
}

bool spec_cast_mage( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  // Modified by SinaC 2001
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    // Modified by SinaC 2000
    if ( victim->fighting == ch/* && number_bits( 2 ) == 0*/ )
      break;
  }
  
  if ( victim == NULL )
    return FALSE;
  
  for ( ;; ){
    int min_level;
    
    switch ( number_bits( 4 ) ){
    case  0: min_level =  0; spell = "blindness";      break;
    case  1: min_level =  3; spell = "chill touch";    break;
    case  2: min_level =  7; spell = "weaken";         break;
    case  3: min_level =  8; spell = "teleport";       break;
    case  4: min_level = 11; spell = "colour spray";   break;
    case  5: min_level = 12; spell = "change sex";     break;
    case  6: min_level = 13; spell = "energy drain";   break;
      // Added by SinaC 2000
    case  7: min_level = 60; spell = "silence";        break;
    case  8: min_level = 40; spell = "acid rain";      break;
    case 11: min_level = 30; spell = "fumble";         break;
    case 12: min_level = 30; spell = "chain lightning";break;
      // end
    case  9: min_level = 15; spell = "fireball";       break;
    case 10: min_level = 20; spell = "plague";	   break;
    default: min_level = 20; spell = "acid blast";     break;
    }
    
    if ( ch->level >= min_level )
      break;
  }
  
  // Added by SinaC 2000
  if (ch->stunned)
    return FALSE;
  
  //sprintf(log_buf,"mage_spec: %s",spell);
  //log_string(log_buf);
  
  if ( ( sn = ability_lookup( spell ) ) < 0 )
    return FALSE;

  // Modified by SinaC 2001
  //(*skill_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  return TRUE;
}

bool spec_cast_undead( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  // Modified by SinaC 2001
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    // Modified by SinaC 2000
    if ( victim->fighting == ch /*&& number_bits( 2 ) == 0*/ )
      break;
  }
  
  if ( victim == NULL )
    return FALSE;

  for ( ;; ){
    int min_level;
    
    switch ( number_bits( 4 ) ){
    case  0: min_level =  0; spell = "curse";          break;
    case  1: min_level =  3; spell = "weaken";         break;
    case  2: min_level =  6; spell = "chill touch";    break;
    case  3: min_level =  9; spell = "blindness";      break;
    case  4: min_level = 12; spell = "poison";         break;
    case  5: min_level = 15; spell = "energy drain";   break;
    case  6: min_level = 18; spell = "harm";           break;
    case  7: min_level = 21; spell = "teleport";       break;
    case  8: min_level = 20; spell = "plague";	   break;
      // Added by SinaC 2000
    case  9: min_level = 30; spell = "vampiric touch"; break;
    case 10: min_level = 60; spell = "word of fear";   break;
      // End
    default: min_level = 18; spell = "harm";           break;
    }
    
    if ( ch->level >= min_level )
      break;
  }
  
  // Added by SinaC 2000
  if (ch->stunned)
    return FALSE;
  
  //sprintf(log_buf,"undead_spec: %s",spell);
  //log_string(log_buf);

  if ( ( sn = ability_lookup( spell ) ) < 0 )
    return FALSE;

  // Modified by SinaC 2001
  //(*ability_table[sn].spell_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  return TRUE;
}

bool spec_executioner( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *crime;

  if ( !IS_AWAKE(ch) || ch->fighting != NULL )
    return FALSE;

  crime = "";
  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
	   &&   can_see(ch,victim)){ 
      crime = "KILLER"; break; 
    }
    
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
	 &&   can_see(ch,victim)){ 
      crime = "THIEF"; break; 
    }
  }
  
  if ( victim == NULL )
    return FALSE;
  
  sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!  MORE BLOOOOD!!!",
	   victim->name, crime );
  REMOVE_BIT(ch->comm,COMM_NOSHOUT);
  do_yell( ch, buf );
  multi_hit( ch, victim, TYPE_UNDEFINED );
  return TRUE;
}

bool spec_fido( CHAR_DATA *ch )
{
  OBJ_DATA *corpse;
  OBJ_DATA *c_next;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if ( !IS_AWAKE(ch) )
    return FALSE;

  for ( corpse = ch->in_room->contents; corpse != NULL; corpse = c_next ){
    c_next = corpse->next_content;
    if ( corpse->item_type != ITEM_CORPSE_NPC )
      continue;
    
    act( "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM );
    for ( obj = corpse->contains; obj; obj = obj_next ){
      obj_next = obj->next_content;
      obj_from_obj( obj );
      obj_to_room( obj, ch->in_room );
    }
    extract_obj( corpse );

    // Added by SinaC 2001
    recomproom(ch->in_room);
    return TRUE;
  }  
  return FALSE;
}

bool spec_guard( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  CHAR_DATA *ech;
  char *crime;
  int max_evil;

  if ( !IS_AWAKE(ch) || ch->fighting != NULL )
    return FALSE;
  
  max_evil = 300;
  ech      = NULL;
  crime    = "";
  
  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    
    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER) 
	 &&   can_see(ch,victim)){ 
      crime = "KILLER"; break; 
    }

    if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF) 
	 &&   can_see(ch,victim)){ 
      crime = "THIEF"; break; 
    }
    
  // Modified by SinaC 2001 etho/alignment are attributes now 
  // Modified by SinaC 2000 for alignment/etho
    if ( victim->fighting != NULL
	 &&   victim->fighting != ch
	 //&&   victim->align.alignment < max_evil ){
	 &&   victim->cstat(alignment) < max_evil ){
      //max_evil = victim->align.alignment;
      max_evil = victim->cstat(alignment);
      ech      = victim;
    }
  }
  
  if ( victim != NULL ){
    sprintf( buf, "%s is a %s!  PROTECT THE INNOCENT!!  BANZAI!!",
	     victim->name, crime );
    REMOVE_BIT(ch->comm,COMM_NOSHOUT);
    do_yell( ch, buf );
    multi_hit( ch, victim, TYPE_UNDEFINED );
    return TRUE;
  }
  
  if ( ech != NULL ){
    act( "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!",
	 ch, NULL, NULL, TO_ROOM );
    multi_hit( ch, ech, TYPE_UNDEFINED );
    return TRUE;
  }
  
  return FALSE;
}



bool spec_janitor( CHAR_DATA *ch )
{
  OBJ_DATA *trash;
  OBJ_DATA *trash_next;

  if ( !IS_AWAKE(ch) )
    return FALSE;

  for ( trash = ch->in_room->contents; trash != NULL; trash = trash_next ){
    trash_next = trash->next_content;
    if ( !IS_SET( trash->wear_flags, ITEM_TAKE ) 
	 || !can_loot(ch,trash)
	 // Added by SinaC 2001
	 || !can_see_obj(ch,trash))
      continue;
    if ( trash->item_type == ITEM_DRINK_CON
	 ||   trash->item_type == ITEM_TRASH
	 ||   trash->cost < 10 ){
      act( "$n picks up some trash.", ch, NULL, NULL, TO_ROOM );
      obj_from_room( trash );
      obj_to_char( trash, ch );
      return TRUE;
    }
  }
  
  return FALSE;
}



bool spec_mayor( CHAR_DATA *ch )
{
  static const char open_path[] =
    "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

  static const char close_path[] =
    "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

  static const char *path;
  static int pos;
  static bool move;
  // Added by SinaC 2000
  CHAR_DATA *vch, *vch_next;

  if ( !move ){
    if ( time_info.hour ==  6 ){
      path = open_path;
      move = TRUE;
      pos  = 0;
    }
    
    if ( time_info.hour == 20 ){
      path = close_path;
      move = TRUE;
      pos  = 0;
    }
  }
  
  if ( ch->fighting != NULL )
    return spec_cast_mage( ch );
  if ( !move || ch->position < POS_SLEEPING )
    return FALSE;
  
  switch ( path[pos] ){
  case '0':
  case '1':
  case '2':
  case '3':
    move_char( ch, path[pos] - '0', FALSE, TRUE, FALSE ); // SinaC 2003 );
    break;
    
  case 'W':
    // Modified by SinaC 2000
    //ch->position = POS_STANDING;
    do_stand(ch,"");
    act( "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM );
    
    // cityguards will now follow the mayor
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next){
      vch_next = vch->next_in_room;
      if ( IS_NPC(vch) 
	   && vch->spec_fun == spec_guard )
	do_follow( vch, ch->name );
    }
    
    break;
    
  case 'S':
    // Modified by SinaC 2000
    //ch->position = POS_SLEEPING;
    do_sleep(ch,"");
    act( "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM );
    
    // cityguards will now stop following the mayor
    for (vch = ch->in_room->people; vch != NULL; vch = vch_next){
      vch_next = vch->next_in_room;
      if ( IS_NPC(vch) && vch->spec_fun == spec_guard ){
	do_follow( vch, "self" );
	vch->bstat(affected_by) |= AFF_HIDE; vch->cstat(affected_by) |= AFF_HIDE;
	vch->bstat(affected_by) |= AFF_INVISIBLE; vch->cstat(affected_by) |= AFF_INVISIBLE;
      }
    }
    break;
    
  case 'a':
    act( "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM );
    break;
    
  case 'b':
    act( "$n says 'What a view!  I must do something about that dump!'",
	 ch, NULL, NULL, TO_ROOM );
    break;
    
  case 'c':
    act( "$n says 'Vandals!  Youngsters have no respect for anything!'",
	 ch, NULL, NULL, TO_ROOM );
    break;
    
  case 'd':
    act( "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM );
    break;
    
  case 'e':
    act( "$n says 'I hereby declare the city of Midgaard open!'",
	 ch, NULL, NULL, TO_ROOM );
    break;
    
  case 'E':
    act( "$n says 'I hereby declare the city of Midgaard closed!'",
	 ch, NULL, NULL, TO_ROOM );
    break;
    
  case 'O':
    /*	do_unlock( ch, "gate" ); */
    do_open( ch, "gate" );
    break;
    
  case 'C':
    do_close( ch, "gate" );
    /*	do_lock( ch, "gate" ); */
    break;
    
  case '.' :
    move = FALSE;
    break;
  }
  
  pos++;
  return FALSE;
}

bool spec_poison( CHAR_DATA *ch )
{
  CHAR_DATA *victim;

  if ( ch->position != POS_FIGHTING
       || ( victim = ch->fighting ) == NULL
       // Modified by SinaC 2000
       ||   number_percent( ) > /*2 **/ ch->level )
    return FALSE;

  // Added by SinaC 2000
    if (ch->stunned)
	return FALSE;

  act( "You bite $N!",  ch, NULL, victim, TO_CHAR    );
  act( "$n bites $N!",  ch, NULL, victim, TO_NOTVICT );
  act( "$n bites you!", ch, NULL, victim, TO_VICT    );

  // Modified by SinaC 2001
  spell_poison( gsn_poison, ch->level, ch, victim,TARGET_CHAR, 1);
  return TRUE;
}

//bool spec_questmaster( CHAR_DATA *ch )
//{
//  return FALSE;
//}

bool spec_thief( CHAR_DATA *ch )
{
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  long gold,silver;

  if ( ch->position != POS_STANDING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    
    if ( IS_NPC(victim)
	 || IS_IMMORTAL(victim)
	 || number_bits( 5 ) != 0 
	 || !can_see(ch,victim))
      continue;
    
    if ( IS_AWAKE(victim) && number_range( 0, ch->level ) == 0 ){
      act( "You discover $n's hands in your wallet!",
	   ch, NULL, victim, TO_VICT );
      act( "$N discovers $n's hands in $S wallet!",
	   ch, NULL, victim, TO_NOTVICT );
      return TRUE;
    }
    else{
      gold = victim->gold * UMIN(number_range(1,20),ch->level / 2) / 100;
      gold = UMIN(gold, ch->level * ch->level * 10 );
      ch->gold     += gold;
      victim->gold -= gold;
      silver = victim->silver * UMIN(number_range(1,20),ch->level/2)/100;
      silver = UMIN(silver,ch->level*ch->level * 25);
      ch->silver	+= silver;
      victim->silver -= silver;
      return TRUE;
    }
  }
  
  return FALSE;
}


// Added by SinaC 2003, just a copy/paste of spec_cast_undead and spec_cast_druid
// FIXME: TO BE REMOVED
bool spec_cast_necromancer( CHAR_DATA *ch ) {
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;

  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    if ( victim->fighting == ch )
      break;
  }
  
  if ( victim == NULL )
    return FALSE;

  for ( ;; ){
    int min_level;
    
    switch ( number_bits( 4 ) ){
    case  0: min_level =  0; spell = "curse";          break;
    case  1: min_level =  3; spell = "weaken";         break;
    case  2: min_level =  6; spell = "chill touch";    break;
    case  3: min_level =  9; spell = "blindness";      break;
    case  4: min_level = 12; spell = "poison";         break;
    case  5: min_level = 15; spell = "energy drain";   break;
    case  6: min_level = 18; spell = "harm";           break;
    case  7: min_level = 21; spell = "teleport";       break;
    case  8: min_level = 20; spell = "plague";	   break;
    case  9: min_level = 30; spell = "vampiric touch"; break;
    case 10: min_level = 60; spell = "word of fear";   break;
    default: min_level = 18; spell = "harm";           break;
    }
    
    if ( ch->level >= min_level )
      break;
  }
  
  if (ch->stunned)
    return FALSE;
  
  if ( ( sn = ability_lookup( spell ) ) < 0 )
    return FALSE;
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim,TARGET_CHAR, 1 );
  return TRUE;
}


bool spec_cast_druid( CHAR_DATA *ch ) {
  CHAR_DATA *victim;
  CHAR_DATA *v_next;
  char *spell;
  int sn;
  int target;

  if ( IS_SET( ch->in_room->cstat(flags), ROOM_NOSPELL ) )
    return FALSE;

  if ( ch->position != POS_FIGHTING )
    return FALSE;

  for ( victim = ch->in_room->people; victim != NULL; victim = v_next ){
    v_next = victim->next_in_room;
    if ( victim->fighting == ch )
      break;
  }
  
  if ( victim == NULL )
    return FALSE;

  for ( ;; ){
    int min_level;
    
    switch ( number_bits( 4 ) ){
    case  0: min_level =  0; spell = "blindness";      target = 1; break;
    case  1: min_level =  3; spell = "cause serious";  target = 1; break;
    case  2: min_level =  7; spell = "earthquake";     target = 1; break;
    case  3: min_level =  9; spell = "cause critical"; target = 1; break;
    case  4: min_level = 10; spell = "dispel evil";    target = 1; break;
    case  5: min_level = 12; spell = "curse";          target = 1; break;
    case  6: min_level = 12; spell = "change sex";     target = 1; break;
    case  7: min_level = 13; spell = "flamestrike";    target = 1; break;
      
    case  8: min_level = 0;  spell = "heal";           target = 0; break;
    case  9: min_level = 0;  spell = "heal";           target = 0; break;
      
    case 10: min_level = 15; spell = "harm";           target = 1; break;
    case 11: min_level = 15; spell = "plague";	       target = 1; break;
    default: min_level = 16; spell = "dispel magic";   target = 1; break;
    }
    
    if ( ch->level >= min_level )
      break;
  }
  
  if (ch->stunned)
    return FALSE;
  
  if ( ( sn = ability_lookup( spell ) ) < 0 )
    return FALSE;
  
  if ( target == 1 )
    (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, victim, TARGET_CHAR, 1 );
  else
    (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, ch->level, ch, ch, TARGET_CHAR, 1 );

  return TRUE;
}
