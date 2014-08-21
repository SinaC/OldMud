/**************************************************************************r
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
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"

// Added by SinaC 2001
#include "classes.h"
#include "abilityupgrade.h"
#include "language.h"
#include "db.h"
#include "comm.h"
#include "act_comm.h"
#include "handler.h"
#include "clan.h"
#include "fight.h"
#include "lookup.h"
#include "restriction.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "special.h"
#include "act_info.h"
#include "interp.h"
#include "act_wiz.h"
#include "ability.h"
#include "config.h"
#include "act_move.h"
#include "dbdata.h"
#include "scrhash.h"
#include "execute.hh"
#include "faction.h"
#include "utils.h"
#include "arena.h"
#include "weather.h"


//#define VERBOSE


void obj_from_char_no_recompute( OBJ_DATA *obj );
void unequip_char_no_recompute( CHAR_DATA *ch, OBJ_DATA *obj );



// SinaC 2003: 2 new tables:
//  1st: message to send to char and room when wearing an item to this location, used in equip_char
//  2nd: PARTS and FORMS needed to wear an item to this location, used in recompute and equip_char
// Room   Char
static const char *wear_msg[MAX_WEAR][2] = {
  { "$n lights $p and holds it.", "You light $p and hold it." },
  { "$n wears $p on $s left finger.", "You wear $p on your left finger." },
  { "$n wears $p on $s right finger.", "You wear $p on your right finger." },
  { "$n wears $p around $s neck.", "You wear $p around your neck." },
  { "$n wears $p around $s neck.", "You wear $p around your neck." },
  { "$n wears $p on $s torso.", "You wear $p on your torso." },
  { "$n wears $p on $s head.", "You wear $p on your head." },
  { "$n wears $p on $s legs.", "You wear $p on your legs." },
  { "$n wears $p on $s feet.", "You wear $p on your feet." },
  { "$n wears $p on $s hands.", "You wear $p on your hands." },
  { "$n wears $p on $s arms.", "You wear $p on your arms." },
  { "$n wears $p as a shield.", "You wear $p as a shield." },
  { "$n wears $p about $s torso.", "You wear $p about your torso." }, 
  { "$n wears $p about $s waist.", "You wear $p about your waist." },
  { "$n wears $p around $s left wrist.", "You wear $p around your left wrist." },
  { "$n wears $p around $s right wrist.", "You wear $p around your right wrist." },
  { "$n wields $p.", "You wield $p." },
  { "$n holds $p in $s hand.", "You hold $p in your hand." },
  { "$n wears $p on $s left ear.", "You wear $p on your left ear." },
  { "$n wears $p on $s right ear.", "You wear $p on your right ear." },
  { "$n wears $p on $s eyes.", "You wear $p on your eyes." },
  { "$n wields $p in $s off-hand.", "You wield $p in your off-hand." },
  { "$n releases $p to float next to $m.", "You release $p and it floats next to you." },
  { "$n wields $p in $s third hand.", "You wield $p in your third hand." },
  { "$n wields $p in $s fourth hand.", "You wield $p in your fourth hand." }
};
static const long wear_needed_parts_form[MAX_WEAR][2] = {
  { PART_HANDS, 0 }, // LIGHT
  { PART_HANDS, 0 }, // FINGER_L
  { PART_HANDS, 0 }, // FINGER_R
  { PART_BODY, 0 }, // NECK_1
  { PART_BODY, 0 }, // NECK_2
  { PART_BODY, 0 }, // BODY
  { PART_HEAD, 0 }, // HEAD
  { PART_LEGS, 0 }, // LEGS
  { PART_FEET, 0 }, // FEET
  { PART_HANDS, 0 }, // HANDS
  { PART_ARMS, 0 }, // ARMS
  { PART_HANDS, 0 }, // SHIELD
  { PART_BODY, 0 }, // ABOUT
  { PART_BODY, 0 }, // WAIST
  { PART_ARMS|PART_HANDS, 0 }, // WRIST_L
  { PART_ARMS|PART_HANDS, 0 }, // WRIST_R
  { PART_HANDS, 0 }, // WIELD
  { PART_HANDS, 0 }, // HOLD
  { PART_EAR, 0 }, // EAR_L
  { PART_EAR, 0 }, // EAR_R
  { PART_EYE, 0 }, // EYES
  { PART_HANDS, 0 }, // SECONDARY
  { 0, 0 }, // FLOAT
  { PART_HANDS, FORM_FOUR_ARMS }, // THIRDLY
  { PART_HANDS, FORM_FOUR_ARMS }, // FOURTHLY
};


/*
 * Local functions.
 */

/* friend stuff -- for NPC's mostly */
bool is_friend(CHAR_DATA *ch,CHAR_DATA *victim) {
  if (is_same_group(ch,victim))
    return TRUE;

  if (!IS_NPC(ch))
    return FALSE;

  if (!IS_NPC(victim)) {
    if (IS_SET(ch->off_flags,ASSIST_PLAYERS))
      return TRUE;
    else
      return FALSE;
  }

  if (IS_AFFECTED(ch,AFF_CHARM))
    return FALSE;

  if (IS_SET(ch->off_flags,ASSIST_ALL))
    return TRUE;

  if (ch->group && ch->group == victim->group)
    return TRUE;

  if (IS_SET(ch->off_flags,ASSIST_VNUM) 
      &&  ch->pIndexData == victim->pIndexData)
    return TRUE;

  if (IS_SET(ch->off_flags,ASSIST_RACE) && ch->cstat(race) == victim->cstat(race))
    return TRUE;

  if (IS_SET(ch->off_flags,ASSIST_ALIGN)
      &&  !IS_SET(ch->act,ACT_NOALIGN) && !IS_SET(victim->act,ACT_NOALIGN)
      &&  ((IS_GOOD(ch) && IS_GOOD(victim))
	   ||	 (IS_EVIL(ch) && IS_EVIL(victim))
	   ||   (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))))
    return TRUE;

  return FALSE;
}

/* returns number of people on an object */
int count_users(OBJ_DATA *obj) {
  CHAR_DATA *fch;
  int count = 0;

  if (obj->in_room == NULL)
    return 0;

  for (fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room)
    if (fch->on == obj)
      count++;

  return count;
}

int weapon_type (const char *name) {
  int type;
 
  for (type = 0; weapon_table[type].name != NULL; type++) {
    if (LOWER(name[0]) == LOWER(weapon_table[type].name[0])
	&&  !str_prefix(name,weapon_table[type].name))
      return weapon_table[type].type;
  }

  return WEAPON_EXOTIC;
}


/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are checked in fight.c */

int check_immune(CHAR_DATA *ch, int dam_type) {
  int immune, def;
  int bit;

  immune = -1;
  def = IS_NORMAL;

  if (dam_type == DAM_NONE)
    return immune;

  if (dam_type <= 3) {
    if (IS_SET(ch->cstat(imm_flags),IRV_WEAPON))
      def = IS_IMMUNE;
    else if (IS_SET(ch->cstat(res_flags),IRV_WEAPON))
      def = IS_RESISTANT;
    else if (IS_SET(ch->cstat(vuln_flags),IRV_WEAPON))
      def = IS_VULNERABLE;
  }
  else {// magical attack
    if (IS_SET(ch->cstat(imm_flags),IRV_MAGIC))
      def = IS_IMMUNE;
    else if (IS_SET(ch->cstat(res_flags),IRV_MAGIC))
      def = IS_RESISTANT;
    else if (IS_SET(ch->cstat(vuln_flags),IRV_MAGIC))
      def = IS_VULNERABLE;
  }

  /* set bits to check -- VULN etc. must ALL be the same or this will fail */
  switch (dam_type) {
  case(DAM_BASH):	bit = IRV_BASH;		break;
  case(DAM_PIERCE):	bit = IRV_PIERCE;	break;
  case(DAM_SLASH):	bit = IRV_SLASH;	break;
  case(DAM_FIRE):	bit = IRV_FIRE;		break;
  case(DAM_COLD):	bit = IRV_COLD;		break;
  case(DAM_LIGHTNING):	bit = IRV_LIGHTNING;	break;
  case(DAM_ACID):	bit = IRV_ACID;		break;
  case(DAM_POISON):	bit = IRV_POISON;	break;
  case(DAM_NEGATIVE):	bit = IRV_NEGATIVE;	break;
  case(DAM_HOLY):	bit = IRV_HOLY;		break;
  case(DAM_ENERGY):	bit = IRV_ENERGY;	break;
  case(DAM_MENTAL):	bit = IRV_MENTAL;	break;
  case(DAM_DISEASE):	bit = IRV_DISEASE;	break;
  case(DAM_DROWNING):	bit = IRV_DROWNING;	break;
  case(DAM_LIGHT):	bit = IRV_LIGHT;	break;
  case(DAM_CHARM):	bit = IRV_CHARM;	break;
  case(DAM_SOUND):	bit = IRV_SOUND;	break;
    // Added by SinaC 2001
  case(DAM_DAYLIGHT):	bit = IRV_DAYLIGHT;	break;
    // Added by SinaC 2003
  case(DAM_EARTH):	bit = IRV_EARTH;	break;
  case(DAM_WEAKEN):	bit = IRV_WEAKEN;	break;
  default:		return def;
  }

  if (IS_SET(ch->cstat(imm_flags),bit))
    immune = IS_IMMUNE;
  else if (IS_SET(ch->cstat(res_flags),bit) && immune != IS_IMMUNE)
    immune = IS_RESISTANT;
  else if (IS_SET(ch->cstat(vuln_flags),bit)) {
    if (immune == IS_IMMUNE)
      immune = IS_RESISTANT;
    else if (immune == IS_RESISTANT)
      immune = IS_NORMAL;
    else
      immune = IS_VULNERABLE;
  }

  if ( def == IS_IMMUNE )
    return def;
  if (immune == -1 )
    return def;
  else
    return immune;
}

bool is_clan(CHAR_DATA *ch) {
  /*  return ch->clan; */
  return !get_clan_table(ch->clan)->independent; /*Oxtal */
}

bool is_same_clan(CHAR_DATA *ch, CHAR_DATA *victim) {
  if (get_clan_table(ch->clan)->independent)
    return FALSE;
  else
    return (ch->clan == victim->clan);
}

/* checks mob format */
bool is_old_mob(CHAR_DATA *ch) {
  if (ch->pIndexData == NULL)
    return FALSE;
  else if (ch->pIndexData->new_format)
    return FALSE;
  return TRUE;
}

/* removed by SinaC 2003
// Added by SinaC 2000 for god skill
bool get_godskill( CHAR_DATA *ch, int sn ) {
  if ( sn == -1 )
    return FALSE;

  if (sn < -1 || sn > MAX_ABILITY) {
    bug("Bad sn %d in get_godskill.",sn);
    return FALSE;
  }

  if ( IS_NPC(ch) )
    return FALSE;

  if ( sn == gods_table[ch->pcdata->god].sn ) 
    return TRUE;

  return FALSE;
}
*/

// Added by SinaC 2000 for clan ability
bool get_clanability( CHAR_DATA *ch, int sn) {
  if ( sn == -1 )
    return FALSE;

  if (sn < -1 || sn > MAX_ABILITY) {
    bug("Bad sn %d in get_clanability.",sn);
    return FALSE;
  }

  if ( !is_clan(ch) ) 
    return FALSE;

  //  if ( get_clan_table( ch->clan )->clan_ability == NULL 
  //       || get_clan_table( ch->clan )->clan_ability[0] == '\0' ) 
  //    return FALSE;
  //
  //  if ( sn == ability_lookup( get_clan_table( ch->clan )->clan_ability ) ) 
  //    return TRUE;
  if ( sn == get_clan_table( ch->clan )->clan_ability )
    return TRUE;

  return FALSE;
}

/* Added by Sinac 1997 */
bool get_raceability( CHAR_DATA *ch, int sn) {
  //int sn1, i;

  if ( sn == -1 )
    return FALSE;

  if (sn < -1 || sn > MAX_ABILITY) {
    bug("Bad sn %d in get_raceability.",sn);
    return FALSE;
  }

  // Added by SinaC 2001
  if ( IS_NPC( ch ) ) {
    if ( race_table[ch->cstat(race)].pc_race == FALSE )
      return FALSE;
  }

  /* Modified by SinaC 2001
  for ( i = 0; i < 5; i++ ) {
    // By Sinac - revised by Oxtal
    if (pc_race_table[ch->cstat(race)].skills[i] == NULL
	|| pc_race_table[ch->cstat(race)].skills[i][0] == '\0' )
      break;
    sn1 = skill_lookup( pc_race_table[ch->cstat(race)].skills[i] );
    if ( sn1 == sn )
      return TRUE;
  }
  */
  // Added by SinaC 2001 for racial language
  int race = ch->cstat(race); // SinaC 2003
  if ( !race_table[race].pc_race ) {   // if current race is not PC race
    race = ch->bstat(race);            //   get base race
    if ( !race_table[race].pc_race ) { //   if base race is not PC race
      race = DEFAULT_PC_RACE;             //     get default race: human
      bug("get_raceability: invalid race for [%s] sn:[%d]",
	  NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
    }
  }
  //int lang = pc_race_table[ch->cstat(race)].language;
  int lang = pc_race_table[race].language;
  if ( lang >= 0 && language_sn( lang ) == sn )
    return TRUE;

  //for ( int i = 0; i < pc_race_table[ch->cstat(race)].nb_abilities; i++ )
  for ( int i = 0; i < pc_race_table[race].nb_abilities; i++ )
    //if ( sn == pc_race_table[ch->cstat(race)].abilities[i] )
    if ( sn == pc_race_table[race].abilities[i] )
      return TRUE;

  return FALSE;
}

// Added by SinaC 2000 for mobile class
int get_random_ability( CHAR_DATA *ch, int target ) {
  bool found;
  int sn;

  found = FALSE;
  for ( int i = 0; i < MAX_ABILITY; i++ ){
    int pra;
    if ( ability_table[i].name == NULL ) break;
    
    if ( ( pra = get_ability( ch, i ) ) != 0 
	 && ability_table[i].target == target ) {
      found = TRUE;
    }
  }

  if (!found) return -1;

  do{
    sn = number_range( 1, MAX_ABILITY-1 );
  } while ( get_ability( ch, sn ) == 0 
	    || ability_table[sn].target != target );

  return sn;
}

// Added by SinaC 2001 to allow certain ability to mobs
int get_random_mob_usable_ability( CHAR_DATA *ch ) {
  bool found;

  found = FALSE;
  for ( int i = 0; i < MAX_ABILITY; i++ ){
    int pra;
    if ( ability_table[i].name == NULL ) break;
    
    if ( ( pra = get_ability( ch, i ) ) != 0 
	 // Modified by SinaC 2001
	 //&& ability_table[i].mob_usable == TRUE )
	 //&& ability_table[i].mob_use == MOB_USE_COMBAT )
	 && IS_SET( ability_table[i].mob_use, MOB_USE_COMBAT )
	 && ability_table[i].action_fun != NULL )
      found = TRUE;
  }

  if (!found) return -1;

  int sn;
  do{
    sn = number_range( 1, MAX_ABILITY-1 );
  } while ( get_ability( ch, sn ) == 0 
	    // Modified by SinaC 2001
	    //|| ability_table[sn].mob_usable == FALSE );
	    // || ability_table[sn].mob_use != MOB_USE_COMBAT );
	    || !IS_SET( ability_table[sn].mob_use, MOB_USE_COMBAT )
	    || ability_table[sn].action_fun == NULL );

  return sn;
}

// Added by SinaC 2001 to get random race ability
int get_random_race_ability( CHAR_DATA *ch ) {
  bool found;
  int sn;

  found = FALSE;
  for ( int i = 0; i < MAX_ABILITY; i++ )
    if ( get_raceability( ch, i ) == TRUE
	 // Modified by SinaC 2001
	 //&& ability_table[i].mob_usable == TRUE )
	 //&& ability_table[i].mob_use == MOB_USE_COMBAT )
	 && IS_SET( ability_table[i].mob_use, MOB_USE_COMBAT ) )
      found = TRUE;

  if (!found) return -1;

  do {
    sn = number_range( 1, MAX_ABILITY-1 );
  } while ( get_raceability( ch, sn ) == FALSE
	    // Modified by SinaC 2001
           //|| ability_table[sn].mob_usable == FALSE );
	   // || ability_table[sn].mob_use != MOB_USE_COMBAT );
	    || !IS_SET( ability_table[sn].mob_use, MOB_USE_COMBAT ) );

  return sn;
}

// return the casting level of the ability
int get_casting_level( CHAR_DATA *cch, int sn ) {
  CHAR_DATA *ch;
  int lvl;
  
  // if cch is switched we get info from the switcher, SinaC 2001
  if ( IS_NPC(cch) && cch->desc != NULL )
    ch = cch->desc->original;
  else
    ch = cch;
  
  if (sn == -1) /* shorthand for level based abilities */
    return 1;

  else if (sn < -1 || sn > MAX_ABILITY) {
    bug("Bad sn %d in get_casting_level.",sn);
    return 0;
  }

  if ( IS_NPC( ch ) 
       // Added by SinaC 2001
       || get_raceability( ch, sn ) )
    if ( ability_table[sn].nb_casting_level == 0 )
      lvl = 0;
    else {
      //   1 --> 25  :  1
      //  26 --> 50  :  2
      //  51 --> 75  :  3
      //  76 --> 100 :  4
      // 101 --> --- :  5
      lvl = URANGE( 1, 1+(ch->level-1)/25, ability_table[sn].nb_casting_level );
      // check prereq to see if ch can get this casting level
      int p_lvl = 1;
      ability_type *pAbility = &(ability_table[sn]);
      if ( pAbility->prereqs != NULL )
	for ( int i = 1; i < pAbility->nb_casting_level+1; i++ ) {
	  PREREQ_DATA *prereq = &(pAbility->prereqs[i]);
	  if ( prereq->classes != 0
	       && !IS_SET( ch->cstat(classes), prereq->classes ) )
	    break;
	  p_lvl = i;
	}
      lvl = UMIN( p_lvl, lvl );
    }
  else
    lvl = ch->pcdata->ability_info[sn].casting_level;

  //log_stringf("%s skill %21s  level: %d", NAME(ch), ability_table[sn].name, lvl );

  return lvl;
}

// Simple get skill
// returns  get_skill( ch, sn ) if mob
// returns  100 if race, god or clan skill
// returns  % if pcdata->ability_info[sn].learned > 0  &&  class_skillrating( ch, sn ) > 0
int get_ability_simple( CHAR_DATA *ch, int sn ) {
  int skill = 0;

  if ( sn < 0 )
    return 0;

  if ( IS_NPC(ch ) )
    return get_ability( ch, sn );

  if ( get_raceability(ch,sn) 
       || get_clanability(ch,sn) 
       /*|| get_godskill( ch, sn )  removed by SinaC 2003*/)
    return 100;

  if ( ( class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level ) > 0 
       && class_abilitylevel( ch, sn ) < LEVEL_HERO+1 ) 
       || IS_IMMORTAL( ch ) )
    skill = ch->pcdata->ability_info[sn].learned;
  
  if ( skill != 0
       && HAS_ABILITY_UPGRADE( ch ) ) // SinaC 2003
    skill += get_ability_upgrade( ch, sn );

  skill = URANGE( 0, skill, 100 );

  return skill;
}
 
/* for returning skill information      Modified by Sinac, revised by Oxtal */
// Modified again by SinaC 2000
// Modified again by SinaC 2001
int get_ability(CHAR_DATA *cch, int sn) {
  CHAR_DATA *ch;
  int skill;

  // if cch is switched we get info from the switcher
  if ( IS_NPC(cch) && cch->desc != NULL )
    ch = cch->desc->original;
  else
    ch = cch;
  
  if (sn == -1) /* shorthand for level based abilities */
    skill = ch->level * 5 / 2;
  else if (sn < -1 || sn > MAX_ABILITY) {
    bug("Bad sn %d in get_ability.",sn);
    skill = 0;
  }
  else if (!IS_NPC(ch)) { // player
    //  if (ch->level < ability_table[sn].skill_level[ch->class])
    //       skill = 0;
    //     else
    //       skill = ch->pcdata->learned[sn];
    // Modified by Sinac 1997, modified again by SinaC 2000
    // again by SinaC 2001 for god skill
    if ( get_raceability(ch,sn) 
	 || get_clanability(ch,sn) 
	 /*|| get_godskill( ch, sn )  removed by SinaC 2003*/)
      skill = 100;
    else
      // Modified by SinaC 2001
      if ( ch->level >= class_abilitylevel(/*ch->cstat(classes)*/ch,sn))
	skill = ch->pcdata->ability_info[sn].learned;
      else 
	skill = 0;
  }
  else {/* mobiles */

    // Added by SinaC 2001
    if ( get_raceability(ch,sn) )
      skill = 100;
    else
    // Added by SinaC 2000 for mobile classes
    if ( ch->cstat(classes) ) { // has at least one class
      // we have to consider mob higher than HERO
      // Modified by SinaC 2001, modified again by SinaC 2001
      if ( UMIN( ch->level, LEVEL_HERO ) >= class_abilitylevel( /*ch->cstat(classes)*/ch, sn ) 
	   // Modified by SinaC 2001
	   //&& ability_table[sn].mob_usable == TRUE )
	   //&& ability_table[sn].mob_use != MOB_USE_NONE )
	   && ability_table[sn].mob_use != 0 )
	skill = 40 + 2 * ch->level;
      else
	skill = 0;
    }
    else { // no class
	
      skill = 0;

      //       Removed by SinaC 2000 for mobile class: mobile cast spells only if they have class
      //       casting spells
      //	 if (ability_table[sn].spell_fun != spell_null)
      //	 skill = 40 + 2 * ch->level;
      //	 // Oxtal> So every mobile can cast any spell... Ridiculous!
      //	 else 
      // Added by SinaC 2001
      if (sn == gsn_bite) {
	if (IS_SET(ch->off_flags,OFF_BITE))
	  skill = 10 + 3 * ch->level;
      }
      // Added by SinaC 2000
      //else if (sn == gsn_stun ) {
      //if (IS_SET(ch->off_flags, OFF_BASH ) 
      //    && IS_SET(ch->act,ACT_WARRIOR))
      //  skill = ch->level/2;
      //}
      else if (sn == gsn_counter) { 
	if (IS_SET(ch->act,ACT_WARRIOR) && IS_SET(ch->off_flags, OFF_COUNTER))
	  skill = 20+(3*ch->level)/4;
      }
      else if (sn == gsn_pick_lock) { 
	if (IS_SET(ch->act,ACT_THIEF))
	  skill = 20+ch->level/2;
      }
      else if (sn == gsn_dual_wield) {
	if (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF))
	  skill = 10 + 2 * ch->level;
      }
      else if (sn == gsn_crush ) {
	if ( IS_SET( ch->off_flags, OFF_CRUSH ) )
	  skill = 10 + ch->level * 2;
      }
      else if (sn == gsn_blindfight ) {
	if ( IS_SET( ch->act, ACT_WARRIOR ) 
	     || IS_SET( ch->act, ACT_THIEF ) )
	  skill = ch->level * 2 - 40; // -40 added by SinaC 2003
      }
      else if ( sn == gsn_fade ) {
	if (IS_SET(ch->off_flags, OFF_FADE))
	  skill = ch->level;
      }
      else if (sn == gsn_sneak || sn == gsn_hide) {
	// Added by SinaC 2000
	if ( IS_SET( ch->act, ACT_THIEF ) )
	  skill = ch->level * 2 + 20;
      }
      // end SinaC addition
	
      else if (sn == gsn_dodge) {
	if (IS_SET(ch->off_flags,OFF_DODGE))
	  skill = ch->level *2;
      }
      else if (sn == gsn_parry) {
	if (IS_SET(ch->off_flags,OFF_PARRY))
	  skill = ch->level * 2;
      }
      else if (sn == gsn_shield_block)
	skill = 10 + 2 * ch->level;
	
      else if (sn == gsn_second_attack) {
	if (IS_SET(ch->act,ACT_WARRIOR) || IS_SET(ch->act,ACT_THIEF))
	  skill = 10 + 3 * ch->level;
      }
      else if (sn == gsn_third_attack) {
	if (IS_SET(ch->act,ACT_WARRIOR))
	  skill = 4 * ch->level - 40;
      }
      // Added by SinaC 2001
      else if (sn == gsn_fourth_attack) {
	if (IS_SET(ch->act,ACT_WARRIOR))
	  skill = 3 * ch->level - 40;
      }
      else if (sn == gsn_fifth_attack) {
	if (IS_SET(ch->act,ACT_WARRIOR))
	  skill = 2 * ch->level - 40;
      }
      else if (sn == gsn_hand_to_hand)
	skill = 40 + 2 * ch->level;
	
      else if (sn == gsn_trip) {
	if  (IS_SET(ch->off_flags,OFF_TRIP))
	  skill = 10 + 3 * ch->level;
      }
      else if (sn == gsn_bash) { 
	if (IS_SET(ch->off_flags,OFF_BASH))
	  skill = 10 + 3 * ch->level;
      }
	
      /* Added by SinaC mobiles having OFF_TAIL, maybe we could add a test to know if
	 the mobile has a tail */
      else if (sn == gsn_tail) { 
	if (IS_SET(ch->off_flags,OFF_TAIL))
	  skill = 10 + 3 * ch->level;
      }
      /* end */
      else if (sn == gsn_disarm) { 
	if ( IS_SET(ch->off_flags,OFF_DISARM)
	     || IS_SET(ch->act,ACT_WARRIOR)
	     || IS_SET(ch->act,ACT_THIEF) )
	  skill = 20 + 3 * ch->level;
      }
      else if (sn == gsn_berserk) { 
	if  (IS_SET(ch->off_flags,OFF_BERSERK))
	  skill = 3 * ch->level;
      }
	
      else if (sn == gsn_kick) {
	if (IS_SET(ch->off_flags,OFF_KICK))
	  skill = 10 + 3 * ch->level;
      }
      else if (sn == gsn_backstab) { 
	if (IS_SET(ch->act,ACT_THIEF))
	  skill = 20 + 2 * ch->level;
      }
	
      else if (sn == gsn_circle) {      /* Added by SinaC 2000 */
	if (IS_SET(ch->off_flags,OFF_BACKSTAB))
	  skill = ch->level;
      }
      else if (sn==gsn_enhanced_damage) {
	if (IS_SET(ch->act,ACT_WARRIOR)
	    ||IS_SET(ch->act,ACT_THIEF))
	  skill = 20 + 3 * ch->level;
      }
	
      else if (sn == gsn_rescue)
	skill = 40 + ch->level;
	
      else if (sn == gsn_recall)
	skill = 40 + ch->level;
	
      else if (sn == gsn_dirt) {
	if (IS_SET(ch->off_flags,OFF_KICK_DIRT))
	  skill = 20 + 2*ch->level; /* by oxtal */
      }
      else if (sn == gsn_roundhousekick) {
	if (IS_SET(ch->off_flags,OFF_KICK))
	  skill = 4 * ch->level - 40;  /* by oxtal */
      }
      else 
	if (sn == gsn_sword
	    ||  sn == gsn_dagger
	    ||  sn == gsn_spear
	    ||  sn == gsn_mace
	    ||  sn == gsn_axe
	    ||  sn == gsn_flail
	    ||  sn == gsn_whip
	    ||  sn == gsn_polearm)
	  skill = 40 + 5 * ch->level / 2;
    }
  }
  
  if (ch->daze > 0) {
    if ( ability_table[sn].type == TYPE_SKILL )
      skill = ( 2 * skill )/ 3;
    else
      skill = skill/2;
    //     Modified by SinaC 2001
    //    if (ability_table[sn].spell_fun != spell_null)
    //    skill = skill/2;
    //    else
    //      skill = ( 2 * skill )/ 3;
  }
  
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10 )
    skill = ( 9 * skill )/ 10;

  // Modified by SinaC 2001
  if ( skill != 0
       && HAS_ABILITY_UPGRADE( ch ) ) // SinaC 2003
    skill += get_ability_upgrade( ch, sn );

  //  if ( get_ability_upgrade( ch, sn ) != 0 )
  //    log_stringf("%s: '%s' %d%%(base:%d%%+bonus:%d%%)",
  //		NAME(ch),
  //		ability_table[sn].name,
  //		skill,
  //		IS_NPC(ch)?skill:ch->pcdata->ability_info[sn].learned,
  //		get_ability_upgrade( ch, sn ));

  return URANGE(0,skill,100);
}

/* for returning weapon information */
//int get_weapon_sn(CHAR_DATA *ch, bool secondary ) {
//  OBJ_DATA *wield;
//  int sn;
//
//  if ( secondary )
//    wield = get_eq_char( ch, WEAR_SECONDARY );
//  else
//    wield = get_eq_char( ch, WEAR_WIELD );
//
//  if (wield == NULL || wield->item_type != ITEM_WEAPON)
//    sn = gsn_hand_to_hand;
//  else switch (wield->value[0]) {
//    // we could use weapon_table
//  default :               sn = -1;                break;
//  case(WEAPON_SWORD):     sn = gsn_sword;         break;
//  case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
//  case(WEAPON_SPEAR):     sn = gsn_spear;         break;
//  case(WEAPON_MACE):      sn = gsn_mace;          break;
//  case(WEAPON_AXE):       sn = gsn_axe;           break;
//  case(WEAPON_FLAIL):     sn = gsn_flail;         break;
//  case(WEAPON_WHIP):      sn = gsn_whip;          break;
//  case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
//  case(WEAPON_STAFF):     sn = gsn_staff;         break; // Added by SinaC 2003
//  case(WEAPON_ARROW):     sn = gsn_spear;         break;
//  case(WEAPON_RANGED):    sn = gsn_bowfire;       break;
//  }
//  return sn;
//}

int get_weapon_sn(CHAR_DATA *ch, int whichWield ) {
  OBJ_DATA *wield;
  int sn;

  switch( whichWield ) {
  case 1: wield = get_eq_char( ch, WEAR_WIELD ); break;
  case 2: wield = get_eq_char( ch, WEAR_SECONDARY ); break;
  case 3: wield = get_eq_char( ch, WEAR_THIRDLY ); break;
  case 4: wield = get_eq_char( ch, WEAR_FOURTHLY ); break;
  default: 
    wield = get_eq_char( ch, WEAR_WIELD ); 
    bug("Invalid whichWield in get_weapon_sn [%d]", whichWield);
    break;
  }

  if (wield == NULL || wield->item_type != ITEM_WEAPON)
    sn = gsn_hand_to_hand;
  else switch (wield->value[0]) {
    // we could use weapon_table
  default :               sn = -1;                break;
  case(WEAPON_SWORD):     sn = gsn_sword;         break;
  case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
  case(WEAPON_SPEAR):     sn = gsn_spear;         break;
  case(WEAPON_MACE):      sn = gsn_mace;          break;
  case(WEAPON_AXE):       sn = gsn_axe;           break;
  case(WEAPON_FLAIL):     sn = gsn_flail;         break;
  case(WEAPON_WHIP):      sn = gsn_whip;          break;
  case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
  case(WEAPON_STAFF):     sn = gsn_staff;         break; // Added by SinaC 2003
  case(WEAPON_ARROW):     sn = gsn_spear;         break;
  case(WEAPON_RANGED):    sn = gsn_bowfire;       break;
  }
  return sn;
}

int get_weapon_sn(CHAR_DATA *ch, OBJ_DATA *wield ) {
  //OBJ_DATA *wield;
  int sn;

  if (wield == NULL || wield->item_type != ITEM_WEAPON)
    sn = gsn_hand_to_hand;
  else switch (wield->value[0]) {
    // we could use weapon_table
  default :               sn = -1;                break;
  case(WEAPON_SWORD):     sn = gsn_sword;         break;
  case(WEAPON_DAGGER):    sn = gsn_dagger;        break;
  case(WEAPON_SPEAR):     sn = gsn_spear;         break;
  case(WEAPON_MACE):      sn = gsn_mace;          break;
  case(WEAPON_AXE):       sn = gsn_axe;           break;
  case(WEAPON_FLAIL):     sn = gsn_flail;         break;
  case(WEAPON_WHIP):      sn = gsn_whip;          break;
  case(WEAPON_POLEARM):   sn = gsn_polearm;       break;
  case(WEAPON_STAFF):     sn = gsn_staff;         break; // Added by SinaC 2003
  case(WEAPON_ARROW):     sn = gsn_spear;         break;
  case(WEAPON_RANGED):    sn = gsn_bowfire;       break;
  }
  return sn;
}

int get_weapon_ability(CHAR_DATA *ch, int sn) {
  int skill;

  /* -1 is EXOTIC */
  if (IS_NPC(ch)) { // Mob
    if (sn == -1) // EXOTIC
      skill = 3 * ch->level;
    else if (sn == gsn_hand_to_hand) // HAND_TO_HAND
      skill = 40 + 2 * ch->level;
    else // OTHERS
      skill = 40 + 5 * ch->level / 2;
  }
  else { // Player
    if ( sn == -1 ) // EXOTIC
      //skill = 3 * ch->level;
      if ( number_percent() <= get_ability( ch, gsn_improved_exotic ) )
	skill = 3 * ch->level;
      else
	skill = 2 * ch->level + 5;
    else
      //skill = ch->pcdata->learned[sn];
      skill = get_ability( ch, sn ); // OTHERS
  }

  if ( HAS_ABILITY_UPGRADE( ch ) ) // SinaC 2003
    skill += get_ability_upgrade( ch, sn );

  return URANGE(0,skill,100);
}

// SinaC 2003: new affect system
void add_one_affect( CHAR_DATA *ch, OBJ_DATA *obj, AFFECT_LIST *laf ) {
  long i, mod = laf->modifier;
  OBJ_DATA * wield;
  switch (laf->where) {
  case AFTO_CHAR:
    if (laf->location >= 0 && laf->location < ATTR_NUMBER)
      switch(laf->op) {
      case AFOP_ADD:
	ch->curattr[laf->location] += mod; break;
      case AFOP_OR:
	ch->curattr[laf->location] |= mod; break;
      case AFOP_ASSIGN:
	ch->curattr[laf->location] = mod; break;
	// Added by SinaC 2001
      case AFOP_NOR:
	ch->curattr[laf->location] &= ~mod; break;

      default : bug("Wrong affect operation!"); break;
      }
    else if (laf->location == ATTR_allAC) {
      /* Here op == ADD */
      for (i=0;i<4;i++)
	ch->cstat(ac0+i) += mod;
    } 
    else if (laf->location != ATTR_NA)
      bug("%s %d : Wrong affect location (%d)",
	  obj ? obj->name : ch->name,
	  obj ? obj->pIndexData->vnum : 0,
	  laf->location);
    break;
  case AFTO_WEAPON:
    switch(laf->op) {
    case AFOP_OR:
      // bug fixed
      //if ((wield = get_eq_char( ch, WEAR_WIELD )) != NULL)    ; <-- removed that
      if ((wield = get_eq_char( ch, WEAR_WIELD )) != NULL)
	SET_BIT(wield->value[4],laf->modifier);
      break;
      // Added by SinaC 2001
    case AFOP_NOR:
      if ((wield = get_eq_char( ch, WEAR_WIELD )) != NULL)
	REMOVE_BIT(wield->value[4],laf->modifier);
      break;

    default : bug("operation != ORing to weapon flags."); break;
    }
    break;
  }
}
void addonegeneralaffect(CHAR_DATA *ch, OBJ_DATA *obj, AFFECT_DATA *paf) {
  /* obj just for debug -- oxtal*/
  // Added by SinaC 2001
  //int i,mod=paf->modifier;

  // AFFECT_DATA now contains a list of affects
  for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
    add_one_affect( ch, obj, laf );
}

void small_recompute( CHAR_DATA *ch ) {
  for (int i=0;i<ATTR_NUMBER;i++)
    ch->curattr[i] = ch->baseattr[i];
}

void recompute(CHAR_DATA *ch) {
  //  int i,loc,max;
  //  AFFECT_DATA *af;
  //  OBJ_DATA *obj, *wield;
  static int recompute_level = 0;

  if ( ch == NULL || !ch->valid ) {
    bug( "Recompute: NULL char to recompute.");
    return;
  }

  if ( ch->in_room == NULL ) { // SinaC 2003
    small_recompute(ch); // copy at least attr from baseattr to curattr
    //bug("Recompute: NULL in_room [%s]", ch->name );
    return;
  }
  //  log_stringf("========>recomputing [%s]", ch->name );


  recompute_level++;

  small_recompute(ch);
  //  for ( int i=0;i<ATTR_NUMBER;i++)
  //    ch->curattr[i] = ch->baseattr[i];

  // 1. char affects from room
  // 2. char affects from equipement obj_index_data
  // 3. char affects from equipement
  // 4. char affects from char
  // 5. check max value
  // 6. check restriction on equipement + unarmed/unarmored + ability_upgrade
  // 7. recursive call if restriction failed
  // 8. update faction with current race
  
  // 1.
  // Added by SinaC 2001 for room affects
  if ( ch->in_room != NULL ) {
    for ( AFFECT_DATA *af = ch->in_room->base_affected; af != NULL; af = af->next )
      addonegeneralaffect(ch,NULL,af);
    for ( AFFECT_DATA *af = ch->in_room->current_affected; af != NULL; af = af->next )
      addonegeneralaffect(ch,NULL,af);
  }

  // 2. 3.
  OBJ_DATA *wield = get_eq_char( ch, WEAR_WIELD );
  if (wield != NULL)
    recompobj(wield);
  
  // Added by SinaC 2001
  bool dropped = FALSE;
  // Added by SinaC 2003
  bool removed = FALSE;

  // Modified by SinaC 2003, following loop has been optimized: get_eq_char loops among every obj
  //  inventory, and so complexity is O(n²)
  //  for (loc = 0; loc < MAX_WEAR; loc++) {
  //    obj = get_eq_char(ch,loc);
  //    if (obj == NULL || !obj->valid)
  //      continue;
  //    // This assumes that any worn object is armor. What about Jewelry?
  //    for (i = 0; i < 4; i++)
  //      ch->cstat(ac0+i) -= apply_ac( obj, loc, i );
  //
  //    // 3.
  //    if (!obj->enchanted)
  //      for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
  //	addonegeneralaffect(ch,obj,af);
  //
  //    // 2.
  //    for ( af = obj->affected; af != NULL; af = af->next )
  //      addonegeneralaffect(ch,obj,af);
  //  }
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj == NULL || !obj->valid // item not valid
	 || obj->wear_loc == -1  ) // item not worn
      continue;
    int loc = obj->wear_loc;
    // Add AC
    for ( int i = 0; i < 4; i++ )
      ch->cstat(ac0+i) -= apply_ac( obj, loc, i );
    // 3.
    if ( !obj->enchanted )
      for ( AFFECT_DATA *af = obj->pIndexData->affected; af != NULL; af = af->next )
	addonegeneralaffect( ch, obj, af );
    // 2.
    for ( AFFECT_DATA *af = obj->affected; af != NULL; af = af->next )
      addonegeneralaffect( ch, obj, af );
  }

  // 4.
  // now add ability effects
  for ( AFFECT_DATA *af = ch->affected; af != NULL; af = af->next)
    addonegeneralaffect( ch, NULL, af );

  // 5.
  // make sure sex is RIGHT!!!!
  if ( ch->cstat(sex) < 0 || ch->cstat(sex) > 2 )
    ch->cstat(sex) = ch->bstat(sex);
  // check overload of max stat
  for ( int i = 0; i < MAX_STATS; i++ ) {
    int max = 0;
    if ( IS_NPC(ch) || IS_IMMORTAL(ch) )
      max = 30;
    else {
      // Modified by SinaC 2000  +7 before
      int race = ch->cstat(race); // SinaC 2003
      if ( !race_table[race].pc_race ) {   // if current race is not PC race
	race = ch->bstat(race);            //   get base race
	if ( !race_table[race].pc_race ) { //   if base race is not PC race
	  race = DEFAULT_PC_RACE;             //     get default race: human
	  bug("recompute: invalid race for [%s] sn:[%d]",
	      NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
	}
      }
      //max = pc_race_table[ch->cstat(race)].max_stats[i] + 5;
      max = pc_race_table[race].max_stats[i] + 5;
      // +2 before
      if (class_hasattr(ch,i))
	max += 3;
      // Modified by SinaC 2000, max+=1 before
      //if ( ch->cstat(race) == race_lookup("human"))
      //max += 2;
      if ( IS_AFFECTED2( ch, AFF2_HIGHER_MAGIC_ATTRIBUTES ) )
	max += 2;
      max = UMIN(max,30);
    }
    ch->cstat(stat0+i) = URANGE( 3, ch->cstat(stat0+i), max);
  }
  //ch->curattr[ATTR_alignment] = URANGE(-1000,ch->curattr[ATTR_alignment],1000);
  //ch->curattr[ATTR_etho] = URANGE(-1,ch->curattr[ATTR_etho],1);
  ch->cstat(alignment) = URANGE(-1000,ch->cstat(alignment),1000);
  ch->cstat(etho) = URANGE(-1,ch->cstat(etho),1);
  

  // 6.  Check if weapon can be wield
  if ( !IS_NPC(ch) && wield != NULL
       && get_obj_weight(wield) > (str_app[ch->cstat(STR)].wield*10)) {
    static int depth = 0;

    if ( depth == 0 ) {
      depth++;
      act( "You drop $p.", ch, wield, NULL, TO_CHAR );
      act( "$n drops $p.", ch, wield, NULL, TO_ROOM );
      obj_from_char_no_recompute( wield ); // SinaC 2003, no need to recompute
      obj_to_room( wield, ch->in_room );
      OBJPROG( wield , ch, "onDropped", ch ); // Added by SinaC 2003
      depth--;

      // Added by SinaC 2001
      dropped = TRUE;
      // Added by SinaC 2003
      removed = TRUE;
    }
  }

  // Modified by SinaC 2003, following loop has been optimized: get_eq_char loops among every obj
  //  inventory, and so complexity is O(n²)
  //  // 6.  Added by SinaC 2003, we check if equipement can be worn
  //  for (loc = 0; loc < MAX_WEAR; loc++) {
  //    obj = get_eq_char(ch,loc);
  //    if (obj == NULL || !obj->valid)
  //      continue;
  //
  //    // Check needed parts, SinaC 2003
  //    // check_size added by SinaC 2003
  //    // replace anti-good, anti-neutral, anti-evil test in fight.C, SinaC 2001
  //    if ( ( wear_needed_parts_form[loc][0]
  //	   && !IS_SET( ch->cstat(parts), wear_needed_parts_form[loc][0] ) )
  //	 || ( wear_needed_parts_form[loc][1]
  //	      && !IS_SET( ch->cstat(form),  wear_needed_parts_form[loc][1] ) )
  //	 || !check_restriction( ch, obj )
  //	 || !check_size( ch, obj, TRUE ) ) {
  //      act("You can't use $p anymore.", ch, obj, NULL, TO_CHAR );
  //      act("$n can't use $p anymore.", ch, obj, NULL, TO_ROOM );
  //      obj_from_char_no_recompute( obj ); // SinaC 2003, no need to recompute
  //      // Added by SinaC 2003
  //      removed = TRUE;
  //      
  //      obj_to_char( obj, ch ); // SinaC 2003, not dropped, just put in inventory
  //      //if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
  //      //obj_to_char( obj, ch );
  //      //else { // Modified by SinaC 2001
  //      //obj_to_room( obj, ch->in_room );
  //      //OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
  //      //dropped = TRUE;
  //      //}
  //      continue;
  //    }
  //  }
  // Unarmed/Unarmored default value is TRUE
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_UNARMED_UNARMORED );
  // Ability upgrade default value is FALSE
  REMOVE_BIT( ch->optimizing_bit, OPTIMIZING_BIT_HAS_ABILITY_UPGRADE );
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj == NULL || !obj->valid // item not valid
	 || obj->wear_loc == -1  ) // item not worn
      continue;
    int loc = obj->wear_loc;
    // Check needed parts, forms, restriction and size
    if ( ( wear_needed_parts_form[loc][0]
  	   && !IS_SET( ch->cstat(parts), wear_needed_parts_form[loc][0] ) )
  	 || ( wear_needed_parts_form[loc][1]
  	      && !IS_SET( ch->cstat(form),  wear_needed_parts_form[loc][1] ) )
  	 || !check_restriction( ch, obj )
  	 || !check_size( ch, obj, TRUE ) ) {
      act("You can't use $p anymore.", ch, obj, NULL, TO_CHAR );
      act("$n can't use $p anymore.", ch, obj, NULL, TO_ROOM );
      obj_from_char_no_recompute( obj ); // SinaC 2003, no need to recompute
      removed = TRUE;
      
      obj_to_char( obj, ch ); // SinaC 2003, not dropped, just put in inventory
      continue;
    }
    // Check if item is worn at a location making player unable to use ability
    //  requiring being unarmed/unarmored, SinaC 2003
    if ( loc == WEAR_HOLD                            // no hold
  	 || obj->wear_loc == WEAR_WIELD              // no wield
  	 || obj->wear_loc == WEAR_SECONDARY          // no secondary
  	 || obj->wear_loc == WEAR_THIRDLY            // no thirdly
  	 || obj->wear_loc == WEAR_FOURTHLY           // no fourthly
  	 || obj->wear_loc == WEAR_SHIELD             // no shield
	 || ( obj->wear_loc != WEAR_NONE             // no armor
  	      && obj->item_type == ITEM_ARMOR ) )
      REMOVE_BIT( ch->optimizing_bit, OPTIMIZING_BIT_UNARMED_UNARMORED );
    // Check if item gives ability upgrade, SinaC 2003
    //    if ( obj->upgrade != NULL
    //	 || ( !obj->enchanted && obj->pIndexData->upgrade != NULL ) )
    if ( obj->pIndexData->upgrade != NULL )
      SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_HAS_ABILITY_UPGRADE );
  }

  
  // 7.
  // Added by SinaC 2001
  if ( dropped )
    recomproom(ch->in_room);
  if ( removed )
    // Added by SinaC 2003, if we wear an item with restriction and the item has affects
    //  countering restrictions, the item will be dropped and we need to recompute to
    //  remove affects given by this item
    recompute(ch);

  // 8.
  // SinaC 2003 for factions, only if master call
  // FIXME: if obj with restriction in faction, remove condition on recompute_level
  if ( recompute_level == 1 )
    if ( !IS_NPC(ch) ) // faction base to current and update if base_race is different from current_race
      if ( ch->bstat(race) != ch->cstat(race) ) { // current and base race are different
	FACTION_DATA *faction = get_race_faction(ch->cstat(race));
	for ( int i = 0; i < MAX_FACTION; i++ ) // update: get maximum of base and race faction
	  ch->pcdata->current_faction_friendliness[i] = UMAX( ch->pcdata->base_faction_friendliness[i], 
							      faction->friendliness[i] );
      }
      else
	for ( int i = 0; i < MAX_FACTION; i++ ) // copy from base to current
	  ch->pcdata->current_faction_friendliness[i] = ch->pcdata->base_faction_friendliness[i];

  recompute_level--;
}

// SinaC 2003: new affect system
void add_one_obj_affect( OBJ_DATA *obj, AFFECT_LIST *laf ) {
  long mod=laf->modifier;
  switch (laf->where) {
  case AFTO_OBJVAL:
    if (laf->location < 0 || laf->location > 4)
      bug("Wrong affect value number!");
    else
      switch(laf->op) {
      case AFOP_ADD:    obj->value[laf->location] += mod; break;
      case AFOP_OR:     obj->value[laf->location] |= mod; break;
	// Added by SinaC 2001
      case AFOP_ASSIGN: obj->value[laf->location] = mod; break;
      case AFOP_NOR:    obj->value[laf->location] &= ~mod; break;
      }
    break;
  case AFTO_OBJECT:
    switch(laf->op) {
    case AFOP_ADD:    bug("ADDing to object flags"); break;
    case AFOP_OR:     obj->extra_flags |= mod; break;
      // Added by SinaC 2001
    case AFOP_ASSIGN: bug("ASSIGNing to object flags");  break;
    case AFOP_NOR:    obj->extra_flags &= ~mod; break;
    }
    break;
  case AFTO_WEAPON:
    if ( obj->item_type != ITEM_WEAPON
	 || obj->value[0] == WEAPON_RANGED ) {
      bug("AFTO_WEAPON affect on a non-weapon item or ranged-weapon [%s  vnum: %d].",
	  obj->short_descr, obj->pIndexData->vnum );
      break;
    }
    switch(laf->op) {
    case AFOP_OR:
	SET_BIT(obj->value[4],laf->modifier);
      break;
      // Added by SinaC 2001
    case AFOP_NOR:
      REMOVE_BIT(obj->value[4],laf->modifier);
      break;

    default : bug("operation != ORing to weapon flags."); break;
    }
    break;
  }
}
void addoneobjgeneralaffect(OBJ_DATA *obj, AFFECT_DATA *paf) {
  for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
    add_one_obj_affect( obj, laf );
}

void recompobj(OBJ_DATA *obj) {
  int i;
  AFFECT_DATA *af;

  if (obj == NULL || !obj->valid ) {
    bug( "RecompObj : NULL obj to recompute.");
    return;
  }

  obj->extra_flags = obj->base_extra;
  for (i=0;i<5;i++)
    obj->value[i] = obj->baseval[i];

  // 1. obj affects from room
  // 2. obj affects from carrier, SinaC 2003
  // 3. change armor's value in function of condition
  // 4. obj affects from obj_index_data
  // 5. obj affects from obj

  // 1.  Added by SinaC 2001 for room affects
  if ( obj->in_room != NULL ) {
    for ( af = obj->in_room->base_affected; af != NULL; af = af->next )
      addoneobjgeneralaffect(obj,af);
    for ( af = obj->in_room->current_affected; af != NULL; af = af->next )
      addoneobjgeneralaffect(obj,af);
  }

  // 2.  SinaC 2003
  if ( obj->carried_by != NULL )
    for ( af = obj->carried_by->affected; af != NULL; af = af->next  )
      addoneobjgeneralaffect(obj,af);

  // 3.  Added by SinaC 2001 for worn armor
  if ( obj->item_type == ITEM_ARMOR )
    for ( i = 0; i < 4; i++ )
      obj->value[i] = ( obj->baseval[i] * obj->condition ) / 100;

  // 4.
  if (!obj->enchanted)
    for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
      addoneobjgeneralaffect(obj,af);

  // 5.
  for ( af = obj->affected; af != NULL; af = af->next )
    addoneobjgeneralaffect(obj ,af);

}

// SinaC 2003: new affect system
void add_one_room_affect( ROOM_INDEX_DATA *room, AFFECT_LIST *laf ) {
  long mod=laf->modifier;

  switch (laf->where) {
  case AFTO_ROOM:
    if (laf->location < 0 || laf->location > MAX_ROOM_ATTR) {
      bug("Wrong affect value number!");
    } 
    else
      switch(laf->op) {
      case AFOP_ADD:    room->curattr[laf->location] += mod; break;
      case AFOP_OR:     room->curattr[laf->location] |= mod; break;
      case AFOP_ASSIGN: room->curattr[laf->location] = mod; break;
	// Added by SinaC 2001
      case AFOP_NOR:    room->curattr[laf->location] &= ~mod; break;
      }
    break;
  }
}
// Added by SinaC 2001 for room affects
void addoneroomgeneralaffect(ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
  for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
    add_one_room_affect( room, laf );
}

void recomproom(ROOM_INDEX_DATA *room) {
  int i;
  AFFECT_DATA *af;

  if (room == NULL) {
    bug( "RecompRoom : NULL room to recompute.");
    return;
  }

  for (i=0;i<MAX_ROOM_ATTR;i++)
    room->curattr[i] = room->baseattr[i];


  // 1. room affects from room
  // 2. room affects from people in the room
  // 3. room affects from obj_index_data in the room
  // 4. room affects from obj in the room

  // 1.
  for ( af = room->base_affected; af != NULL; af = af->next )
    addoneroomgeneralaffect(room,af);
  // 1.1
  for ( af = room->current_affected; af != NULL; af = af->next )
    addoneroomgeneralaffect(room,af);

  // 2.
  for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room ) {
    for ( af = ch->affected; af != NULL; af = af->next )
      addoneroomgeneralaffect(room,af);
  }

  for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content ) {
    // 3.
    if ( !obj->enchanted )
      for ( af = obj->pIndexData->affected; af != NULL; af = af->next )
	addoneroomgeneralaffect(room,af);
    // 4.
    for ( af = obj->affected; af != NULL; af = af->next )
      addoneroomgeneralaffect(room,af);
  }
}


/*
 * Retrieve a character's trusted level for permission checking.
 */
int get_trust( CHAR_DATA *ch ) {
  if ( ch->desc != NULL && ch->desc->original != NULL )
    ch = ch->desc->original;

  if (ch->trust)
    return ch->trust;

  if ( IS_NPC(ch) && ch->level >= LEVEL_HERO )
    return LEVEL_HERO - 1;
  else
    return ch->level;
}


/*
 * Retrieve a character's age.
 */
int get_age( CHAR_DATA *ch ) {
  return 17 + ( ch->played + (int) (current_time - ch->logon) ) / 72000;
}

/* command for returning max training score */
int get_max_train( CHAR_DATA *ch, int stat ) {
  int max;

  if (IS_NPC(ch) || IS_IMMORTAL(ch))
    return 30;

  // Modified by SinaC 2001
  //max = pc_race_table[ch->cstat(race)].max_stats[stat];
  
  int race = ch->bstat(race);            //   get base race
  if ( !race_table[race].pc_race ) { //   if base race is not PC race
    race = DEFAULT_PC_RACE;             //     get default race: human
    bug("get_max_train: invalid race for [%s] sn:[%d]",
	NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
  }
  //max = pc_race_table[ch->bstat(race)].max_stats[stat];
  max = pc_race_table[race].max_stats[stat];

  if (class_hasattr(ch,stat))
    // Removed by SinaC 2001
    // Modified by SinaC 2001
    //if (ch->cstat(race) == race_lookup("human"))
    /*if (ch->bstat(race) == race_lookup("human"))
      max += 3;
    else
      max += 2;*/
    max += 2;

  return UMIN(max,30);
}


/*
 * Retrieve a character's carry capacity.
 */
int can_carry_n( CHAR_DATA *ch ) {
  if ( IS_IMMORTAL(ch) )
    return 1000;

  if ( IS_NPC(ch) 
       && IS_SET(ch->act, ACT_PET) )
    return 0;

  // Added by SinaC 2001
  if ( IS_NPC(ch)
       && !IS_AFFECTED(ch,AFF_CHARM)
       && ch->leader == NULL
       && ch->master == NULL )
    return 1000;
  
  return MAX_WEAR +  2 * ch->cstat(DEX) + ch->level;
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w( CHAR_DATA *ch ) {
  if ( IS_IMMORTAL(ch) )
    return 10000000;

  if ( IS_NPC(ch) 
       && IS_SET(ch->act, ACT_PET) )
    return 0;

  // Added by SinaC 2001
  if ( IS_NPC(ch)
       && !IS_AFFECTED(ch,AFF_CHARM)
       && ch->leader == NULL
       && ch->master == NULL )
    return 10000000;

  return str_app[ch->cstat(STR)].carry * 10 + ch->level * 25;
}



/*
 * See if a string is one of the names of an object.
 */

bool is_name ( const char *str, const char *namelist ) {
  char name[MAX_INPUT_LENGTH], part[MAX_INPUT_LENGTH];
  const char *list, *string;

  /* fix crash on NULL namelist */
  if (namelist == NULL || namelist[0] == '\0')
    return FALSE;

  /* fixed to prevent is_name on "" returning TRUE */
  if (str[0] == '\0')
    return FALSE;

  string = str;
  /* we need ALL parts of string to match part of namelist */
  for ( ; ; ) {  /* start parsing string */
    str = one_argument(str,part);

    if (part[0] == '\0' )
      return TRUE;

    /* check to see if this is part of namelist */
    list = namelist;
    for ( ; ; ) { /* start parsing namelist */
      list = one_argument(list,name);
      if (name[0] == '\0')  /* this name was not found */
	return FALSE;

      if (!str_prefix(string,name))
	return TRUE; /* full pattern match */

      if (!str_prefix(part,name))
	break;
    }
  }
}

bool is_exact_name(const char *str, const char *namelist ) {
  char name[MAX_INPUT_LENGTH];

  if (namelist == NULL)
    return FALSE;

  for ( ; ; ) {
    namelist = one_argument( namelist, name );
    if ( name[0] == '\0' )
      return FALSE;
    if ( !str_cmp( str, name ) )
      return TRUE;
  }
}

// enchanted stuff for eq
//void affect_enchant(OBJ_DATA *obj) {
//  // okay, move all the old flags into new vectors if we have to
//  if (!obj->enchanted) {
//    AFFECT_DATA *paf, *af_new;
//    obj->enchanted = TRUE;
//
//    for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
//      af_new = new_affect();
//
//      af_new->next = obj->affected;
//      obj->affected = af_new;
//
//      afcopy(*af_new,*paf); 
//    }
//
//    // Okay, now we also copy the restriction cos' that can be modified by resize skill
//    RESTR_DATA *restr, *restr_new;
//
//    for (restr = obj->pIndexData->restriction; restr != NULL; restr = restr->next) {
//      restr_new = new_restriction();
//
//      restr_new->next = obj->restriction;
//      obj->restriction = restr_new;
//
//      restr_new->type    = restr->type;
//      restr_new->value   = restr->value;
//      restr_new->not_r   = restr->not_r;
//      restr_new->ability_r = restr->ability_r;
//      restr_new->sn      = restr->sn;
//    }
//  }
//}

// SinaC 2003: new affect system
// enchanted stuff for eq
void affect_copy( AFFECT_DATA *newaf, AFFECT_DATA *oldaf ) {
  // Copy affect
  newaf->type = oldaf->type;
  newaf->flags = oldaf->flags;
  newaf->duration = oldaf->duration;
  newaf->level = oldaf->level;
  newaf->casting_level = oldaf->casting_level;

  // Also copy the list of affects
  for ( AFFECT_LIST *laf = oldaf->list; laf != NULL; laf = laf->next )
    addaff2( *newaf, laf->where, laf->location, laf->op, laf->modifier );
}
void affect_enchant(OBJ_DATA *obj) {
  // Okay, move all the old flags into new vectors if we have to
  if (!obj->enchanted) {
    obj->enchanted = TRUE;
    
    for (AFFECT_DATA *paf = obj->pIndexData->affected; paf != NULL; paf = paf->next) {
      AFFECT_DATA *af_new = new_affect(); // create a new affect

      affect_copy( af_new, paf ); // copy affect
      
      af_new->next = obj->affected; // insert in affect list
      obj->affected = af_new;
    }

  // SinaC 2003, not needed: we can safely use pIndexData because restriction/ability_upgrade
  //  cannot be dynamically modified during play
//    // Okay, now we also copy the restriction cos' that can be modified by resize skill
//    for (RESTR_DATA *restr = obj->pIndexData->restriction; restr != NULL; restr = restr->next) {
//      RESTR_DATA *restr_new = new_restriction(); // create a new restriction
//
//      restr_new->type    = restr->type; // copy restriction
//      restr_new->value   = restr->value;
//      restr_new->not_r   = restr->not_r;
//      restr_new->ability_r = restr->ability_r;
//      restr_new->sn      = restr->sn;
//
//      restr_new->next = obj->restriction; // insert in restriction list
//      obj->restriction = restr_new;
//    }
//
//    // Copy ability upgrade
//    for ( ABILITY_UPGRADE *upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next ) {
//      ABILITY_UPGRADE *upgr_new = new_ability_upgrade();
//      upgr_new->sn = upgr->sn;
//      upgr_new->value = upgr->value;
//      
//      upgr_new->next = obj->upgrade; // insert in upgrade list
//      obj->upgrade = upgr_new;
//    }
  }
}

// find an effect in an affect list
AFFECT_DATA  *affect_find(AFFECT_DATA *paf, int sn) {
  for ( AFFECT_DATA *paf_find = paf; paf_find != NULL; paf_find = paf_find->next )
    if ( paf_find->type == sn )
      return paf_find;

  return NULL;
}

// SinaC 2003: new affect system
//Give an affect to a char.
//void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf ) {
//  AFFECT_DATA *paf_new;
//
//  paf_new = new_affect();
//
//  *paf_new		= *paf;
//
//  paf_new->next	= ch->affected;
//  ch->affected	= paf_new;
//
//  //recompute(ch);
//  // Added by SinaC 2001
//  //if ( ch->in_room != NULL 
//  //     && paf->where == AFTO_ROOM )
//  //  recomproom( ch->in_room );
//
//  switch ( paf->where ) {
//  case AFTO_CHAR:
//      recompute( ch );
//    break;
//  case AFTO_ROOM:
//      recomproom(ch->in_room); 
//    break;
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    bug("affect_to_char: where location is a object location");
//    break;
//  default:
//    bug("affect_to_char: invalid where: %d", paf->where );
//    break;
//  }
//
//  return;
//}

void affect_to_char( CHAR_DATA *ch, AFFECT_DATA *paf ) {
  AFFECT_DATA *paf_new = new_affect();

//  paf_new->type = paf->type;
//  paf_new->flags = paf->flags;
//  paf_new->duration = paf->duration;
//  paf_new->level = paf->level;
//  paf_new->casting_level = paf->casting_level;
//
//  paf_new->list = paf->list; // no need to copy the list just point to it
//  paf->list = NULL; // cut pointer to list
  affect_copy( paf_new, paf ); // copy affect

  paf_new->next	= ch->affected; // insert affect in ch's affects
  ch->affected	= paf_new;

  // SinaC 2003, affect has been modified
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );

  bool recompChar = FALSE, recompRoom = FALSE;
  for ( AFFECT_LIST *laf = paf_new->list; laf != NULL; laf = laf->next ) {
    switch ( laf->where ) {
    case AFTO_CHAR: recompChar = TRUE; break;
    case AFTO_ROOM: recompRoom = TRUE; break;
    case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: 
      bug("affect_to_char: where location is a object location");
      break;
    default:
      bug("affect_to_char: invalid where: %d", laf->where );
      break;
    }
  }
  if ( recompChar ) // recompute ch
    recompute(ch);
  if ( recompRoom ) // recompute ch->in_room
    recomproom(ch->in_room);

  //  switch ( paf->where ) { // recompute
  //  case AFTO_CHAR:
  //      recompute( ch );
  //    break;
  //  case AFTO_ROOM:
  //      recomproom(ch->in_room); 
  //    break;
  //  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
  //    bug("affect_to_char: where location is a object location");
  //    break;
  //  default:
  //    bug("affect_to_char: invalid where: %d", paf->where );
  //    break;
  //  }

  return;
}

// Added by SinaC 2003, same as affect_to_char but don't call
//  recompute at the end, useful when you modify entirely a char
//  and you don't want to recompute before ending modifications
//  only used in change_race
//void affect_to_char_no_recompute( CHAR_DATA *ch, AFFECT_DATA *paf ) {
//  AFFECT_DATA *paf_new;
//
//  paf_new = new_affect();
//
//  *paf_new		= *paf;
//
//  paf_new->next	= ch->affected;
//  ch->affected	= paf_new;
//
//  return;
//}

// SinaC 2003: new affect system   NOT USED ANYMORE
void affect_to_char_no_recompute( CHAR_DATA *ch, AFFECT_DATA *paf ) {
  AFFECT_DATA *paf_new = new_affect();

  //  paf_new->type = paf->type;
  //  paf_new->flags = paf->flags;
  //  paf_new->duration = paf->duration;
  //  paf_new->level = paf->level;
  //  paf_new->casting_level = paf->casting_level;
  //
  //  paf_new->list = paf->list; // no need to copy the list just point to it
  //  paf->list = NULL; // cut pointer to list
  affect_copy( paf_new, paf ); // copy affect

  paf_new->next	= ch->affected; // just insert in ch's affects
  ch->affected	= paf_new;

  // SinaC 2003, affect has been modified
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );

  return;
}

// give an affect to an object
//void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf) {
//  AFFECT_DATA *paf_new;
//
//  paf_new = new_affect();
//
//  *paf_new		= *paf;
//  paf_new->next	= obj->affected;
//  obj->affected	= paf_new;
//
//  //recompobj(obj);
//  // Modified by SinaC 2001
//  //if ( obj->carried_by != NULL && obj->wear_loc != -1 
//  //     && paf->where == AFTO_CHAR )
//  //  recompute(obj->carried_by);
//  // Added by SinaC 2001
//  //if ( obj->in_room != NULL 
//  //     && paf->where == AFTO_ROOM )
//  //  recomproom(obj->in_room);
//  
//  switch ( paf->where ) {
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    recompobj( obj );
//    break;
//  case AFTO_CHAR:
//    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
//      recompute( obj->carried_by );
//    break;
//  case AFTO_ROOM:
//    if ( obj->in_room != NULL )
//      recomproom(obj->in_room); 
//    break;
//  default:
//    bug("affect_to_obj: invalid where: %d", paf->where );
//    break;
//  }
//
//
//  return;
//}

// SinaC 2003: new affect system
void affect_to_obj(OBJ_DATA *obj, AFFECT_DATA *paf) {
  AFFECT_DATA *paf_new = new_affect();

  //  paf_new->type = paf->type;
  //  paf_new->flags = paf->flags;
  //  paf_new->duration = paf->duration;
  //  paf_new->level = paf->level;
  //  paf_new->casting_level = paf->casting_level;
  //
  //  paf_new->list = paf->list; // no need to copy the list just point to it
  //  paf->list = NULL; // cut pointer to list
  affect_copy( paf_new, paf ); // copy affect

  paf_new->next	= obj->affected; // insert in obj's affects
  obj->affected	= paf_new;

  bool recompChar = FALSE, recompRoom = FALSE, recompObj = FALSE;
  for ( AFFECT_LIST *laf = paf_new->list; laf != NULL; laf = laf->next ) {
    switch ( laf->where ) {
    case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: recompObj = TRUE; break;
    case AFTO_CHAR:
      if ( obj->carried_by != NULL && obj->wear_loc != -1 )
	recompChar = TRUE;
      break;
    case AFTO_ROOM:
      if ( obj->in_room != NULL )
	recompRoom = TRUE;
      break;
    default:
      bug("affect_to_obj: invalid where: %d", laf->where );
      break;
    }
  }
  if ( recompObj ) // recompute obj
    recompobj(obj);
  if ( recompChar ) // recompute carrier
    recompute(obj->carried_by);
  if ( recompRoom ) // recompute obj->in_room
    recomproom(obj->in_room);

//  switch ( paf->where ) { // recompute
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    recompobj( obj );
//    break;
//  case AFTO_CHAR:
//    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
//      recompute( obj->carried_by );
//    break;
//  case AFTO_ROOM:
//    if ( obj->in_room != NULL )
//      recomproom(obj->in_room); 
//    break;
//  default:
//    bug("affect_to_obj: invalid where: %d", paf->where );
//    break;
//  }
  return;
}

// Added by SinaC 2001 for room affects
// give an affect to a room
//void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
//  AFFECT_DATA *paf_new;
//
//  paf_new = new_affect();
//
//  *paf_new		= *paf;
//  paf_new->next	= room->affected;
//  room->affected	= paf_new;
//
//  //recomproom(room);
//  // Modified by SinaC 2001
//  //if ( paf->where == AFTO_CHAR )
//  //  for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
//  //    recompute( ch );
//  // Added by SinaC 2001
//  //if ( paf->where == AFTO_OBJECT
//  //     || paf->where == AFTO_WEAPON 
//  //     || paf->where == AFTO_OBJVAL )
//  //for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
//  //  recompobj( obj );
//  
//  switch ( paf->where ) {
//  case AFTO_ROOM:
//    recomproom(room); 
//    break;
//  case AFTO_CHAR:
//    for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
//      recompute( ch );
//    break;
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
//      recompobj( obj );
//    break;
//  default:
//    bug("affect_to_room: invalid where: %d", paf->where );
//    break;
//  }
//
//
//  return;
//}

// SinaC 2003: new affect system
void affect_to_room(ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
  AFFECT_DATA *paf_new = new_affect();

  //  paf_new->type = paf->type;
  //  paf_new->flags = paf->flags;
  //  paf_new->duration = paf->duration;
  //  paf_new->level = paf->level;
  //  paf_new->casting_level = paf->casting_level;
  //
  //  paf_new->list = paf->list; // no need to copy the list just point to it
  //  paf->list = NULL; // cut pointer to list
  affect_copy( paf_new, paf ); // copy affect

  paf_new->next	= room->current_affected; // insert in room's affects
  room->current_affected	= paf_new;

  bool recompChar = FALSE, recompRoom = FALSE, recompObj = FALSE;
  for ( AFFECT_LIST *laf = paf_new->list; laf != NULL; laf = laf->next ) {
    switch ( laf->where ) {
    case AFTO_ROOM: recompRoom = TRUE; break;
    case AFTO_CHAR: recompChar = TRUE; break;
    case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: recompObj = TRUE; break;
    default:
      bug("affect_to_room: invalid where: %d", laf->where );
      break;
    }
  }
  if ( recompObj ) // recompute contents
    for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
      recompobj( obj );
  if ( recompChar ) // recompute people
    for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
      recompute( ch );
  if ( recompRoom ) // recompute room
    recomproom(room);

//  switch ( paf->where ) { // recompute
//  case AFTO_ROOM:
//    recomproom(room); 
//    break;
//  case AFTO_CHAR:
//    for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
//      recompute( ch );
//    break;
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
//      recompobj( obj );
//    break;
//  default:
//    bug("affect_to_room: invalid where: %d", paf->where );
//    break;
//  }
  return;
}

//Remove an affect from a char.
//void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf ) {
//  // Added by SinaC 2001
//  int where = paf->where;
//
//  if ( ch->affected == NULL ) {
//    bug( "Affect_remove: no affect.");
//    return;
//  }
//
//  if ( paf == ch->affected ) {
//    ch->affected	= paf->next;
//  }
//  else {
//    AFFECT_DATA *prev;
//
//    for ( prev = ch->affected; prev != NULL; prev = prev->next ) {
//      if ( prev->next == paf ) {
//	prev->next = paf->next;
//	break;
//      }
//    }
//
//    if ( prev == NULL ) {
//      bug( "Affect_remove: cannot find paf.");
//      return;
//    }
//  }
//
//  //recompute(ch);
//  // Added by SinaC 2001
//  //if ( where == AFTO_ROOM )
//  //  recomproom(ch->in_room);
//  
//  switch ( where ) {
//  case AFTO_CHAR:
//      recompute(ch);
//    break;
//  case AFTO_ROOM:
//      recomproom(ch->in_room);
//    break;
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: 
//    bug("affect_remove: where location is a object location");
//    break;
//  default:
//    bug("affect_remove: invalid where: %d", where );
//    break;
//  }
//
//  return;
//}

// SinaC 2003: new affect system
void affect_remove( CHAR_DATA *ch, AFFECT_DATA *paf ) {
  if ( !affect_remove_no_recompute( ch, paf ) ) // fail to remove affect
    return;

  // SinaC 2003, affect has been modified
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );

  bool recompChar = FALSE, recompRoom = FALSE;
  for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
    switch ( laf->where ) {
    case AFTO_CHAR: recompChar = TRUE; break;
    case AFTO_ROOM: recompRoom = TRUE; break;
    case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: 
      bug("affect_remove: where location is a object location");
      break;
    default:
      bug("affect_remove: invalid where: %d", laf->where );
      break;
    }
  }
  if ( recompChar ) // recompute char
    recompute(ch);
  if ( recompRoom ) // recompute room
    recomproom(ch->in_room);

  return;
}

// Added by SinaC 2003, same as affect_remove but don't call
//  recompute at the end, only used in affect_strip and in some wearoff functions
//void affect_remove_no_recompute( CHAR_DATA *ch, AFFECT_DATA *paf ) {
//  int where = paf->where;
//
//  if ( ch->affected == NULL ) {
//    bug( "Affect_remove: no affect.");
//    return;
//  }
//
//  if ( paf == ch->affected ) {
//    ch->affected	= paf->next;
//  }
//  else {
//    AFFECT_DATA *prev;
//
//    for ( prev = ch->affected; prev != NULL; prev = prev->next ) {
//      if ( prev->next == paf ) {
//	prev->next = paf->next;
//	break;
//      }
//    }
//
//    if ( prev == NULL ) {
//      bug( "Affect_remove: cannot find paf.");
//      return;
//    }
//  }
//  return;
//}

// SinaC 2003: new affect system
bool affect_remove_no_recompute( CHAR_DATA *ch, AFFECT_DATA *paf ) {
  if ( ch->affected == NULL ) {
    bug( "Affect_remove: no affect.");
    return FALSE;
  }

  if ( paf == ch->affected )
    ch->affected	= paf->next;
  else {
    AFFECT_DATA *prev;
    for ( prev = ch->affected; prev != NULL; prev = prev->next ) {
      if ( prev->next == paf ) {
	prev->next = paf->next;
	break;
      }
    }
    if ( prev == NULL ) {
      bug( "Affect_remove: cannot find paf.");
      return FALSE;
    }
  }

  // SinaC 2003, affect is not valid anymore
  REMOVE_BIT( paf->flags, AFFECT_IS_VALID );

  // SinaC 2003, affect has been modified
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );
  return TRUE;
}

//void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf) {
//  // Added by SinaC 2001
//  int where = paf->where;
//
//  if ( obj->affected == NULL ) {
//    bug( "Affect_remove_object: no affect.");
//    return;
//  }
//
//  if ( paf == obj->affected ) {
//    obj->affected    = paf->next;
//  }
//  else {
//    AFFECT_DATA *prev;
//
//    for ( prev = obj->affected; prev != NULL; prev = prev->next ) {
//      if ( prev->next == paf ) {
//	prev->next = paf->next;
//	break;
//      }
//    }
//    if ( prev == NULL ) {
//      bug( "Affect_remove_object: cannot find paf.");
//      return;
//    }
//  }
//
//  //recompobj(obj);
//  // Modified by SinaC 2001
//  //if (obj->carried_by != NULL && obj->wear_loc != -1
//  //    && where == AFTO_CHAR )
//  //  recompute(obj->carried_by);
//  // Added by SinaC 2001
//  //if (obj->in_room != NULL 
//  //    && where == AFTO_ROOM)
//  //  recomproom(obj->in_room);
//  
//  switch ( where ) {
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    recompobj(obj);
//    break;
//  case AFTO_CHAR:
//    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
//      recompute(obj->carried_by);
//    break;
//  case AFTO_ROOM:
//    if ( obj->in_room != NULL )
//      recomproom(obj->in_room);
//    break;
//  default:
//    bug("affect_remove_obj: invalid where: %d", where );
//    break;
//  }
//
//  return;
//}

void affect_remove_obj( OBJ_DATA *obj, AFFECT_DATA *paf) {
  if ( !affect_remove_obj_no_recompute( obj, paf ) ) // fail to remove affect
    return;

  bool recompChar = FALSE, recompRoom = FALSE, recompObj = FALSE;
  for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
    switch ( laf->where ) {
    case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: recompObj = TRUE; break;
    case AFTO_CHAR:
      if ( obj->carried_by != NULL && obj->wear_loc != -1 )
	recompChar = TRUE;
      break;
    case AFTO_ROOM:
      if ( obj->in_room != NULL )
	recompRoom = TRUE;
      break;
    default:
      bug("affect_remove_obj: invalid where: %d", laf->where );
      break;
    }
  }
  if ( recompChar ) // recompute char
    recompute(obj->carried_by);
  if ( recompRoom ) // recompute room
    recomproom(obj->in_room);
  if ( recompObj ) // recompute obj
    recompobj(obj);
  return;
}

// Added by SinaC 2003, same as affect_remove_obj but don't call
//  recompute at the end, only used in affect_strip_obj
//void affect_remove_obj_no_recompute( OBJ_DATA *obj, AFFECT_DATA *paf) {
//  // Added by SinaC 2001
//  int where = paf->where;
//
//  if ( obj->affected == NULL ) {
//    bug( "Affect_remove_object: no affect.");
//    return;
//  }
//
//  if ( paf == obj->affected ) {
//    obj->affected    = paf->next;
//  }
//  else {
//    AFFECT_DATA *prev;
//
//    for ( prev = obj->affected; prev != NULL; prev = prev->next ) {
//      if ( prev->next == paf ) {
//	prev->next = paf->next;
//	break;
//      }
//    }
//    if ( prev == NULL ) {
//      bug( "Affect_remove_object: cannot find paf.");
//      return;
//    }
//  }
//
//  return;
//}

// SinaC 2003: new affect system
bool affect_remove_obj_no_recompute( OBJ_DATA *obj, AFFECT_DATA *paf) {
  if ( obj->affected == NULL ) {
    bug( "Affect_remove_object: no affect.");
    return FALSE;
  }

  if ( paf == obj->affected )
    obj->affected    = paf->next;
  else {
    AFFECT_DATA *prev;

    for ( prev = obj->affected; prev != NULL; prev = prev->next ) {
      if ( prev->next == paf ) {
	prev->next = paf->next;
	break;
      }
    }
    if ( prev == NULL ) {
      bug( "Affect_remove_object: cannot find paf.");
      return FALSE;
    }
  }

  // SinaC 2003, affect is not valid anymore
  REMOVE_BIT( paf->flags, AFFECT_IS_VALID );

  return TRUE;
}

//// Added by SinaC 2001 for room affects
//void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
//  // Added by SinaC 2001
//  int where = paf->where;
//
//  if ( room->affected == NULL ) {
//    bug( "Affect_remove_room: no affect.");
//    return;
//  }
//
//  if ( paf == room->affected ) {
//    room->affected    = paf->next;
//  }
//  else {
//    AFFECT_DATA *prev;
//
//    for ( prev = room->affected; prev != NULL; prev = prev->next ) {
//      if ( prev->next == paf ) {
//	prev->next = paf->next;
//	break;
//      }
//    }
//    if ( prev == NULL ) {
//      bug( "Affect_remove_room: cannot find paf.");
//      return;
//    }
//  }
//
//  //recomproom(room);
//  // Modified by SinaC 2001
//  //if ( paf->where == AFTO_CHAR )
//  //  for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
//  //    recompute( ch );
//  // Added by SinaC 2001
//  //if ( paf->where == AFTO_OBJECT
//  //     || paf->where == AFTO_WEAPON 
//  //     || paf->where == AFTO_OBJVAL )
//  //  for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
//  //    recompobj( obj );
//  switch ( where ) {
//  case AFTO_ROOM:
//    recomproom(room); 
//    break;
//  case AFTO_CHAR:
//    for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
//      recompute( ch );
//    break;
//  case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL:
//    for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
//      recompobj( obj );
//    break;
//  default:
//    bug("affect_remove_room: invalid where: %d", where );
//    break;
//  }
//
//  return;
//}

// SinaC 2003: new affect system
void affect_remove_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
  if ( !affect_remove_room_no_recompute( room, paf ) ) // fail to remove affect
    return;

  bool recompChar = FALSE, recompRoom = FALSE, recompObj = FALSE;
  for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
    switch ( laf->where ) {
    case AFTO_OBJECT: case AFTO_WEAPON: case AFTO_OBJVAL: recompObj = TRUE; break;
    case AFTO_CHAR: recompChar = TRUE; break;
    case AFTO_ROOM: recompRoom = TRUE; break;
    default:
      bug("affect_remove_obj: invalid where: %d", laf->where );
      break;
    }
  }
  if ( recompChar ) // recompute char
    for ( CHAR_DATA *ch = room->people; ch != NULL; ch = ch->next_in_room )
      recompute( ch );
  if ( recompRoom ) // recompute room
    recomproom(room);
  if ( recompObj ) // recompute obj
    for ( OBJ_DATA *obj = room->contents; obj != NULL; obj = obj->next_content )
      recompobj( obj );
  return;
}

// Added by SinaC 2003, same as affect_remove_obj but don't call
//  recompute at the end, only used in affect_strip_obj
//void affect_remove_room_no_recompute( ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
//  // Added by SinaC 2001
//  int where = paf->where;
//
//  if ( room->affected == NULL ) {
//    bug( "Affect_remove_room: no affect.");
//    return;
//  }
//
//  if ( paf == room->affected ) {
//    room->affected    = paf->next;
//  }
//  else {
//    AFFECT_DATA *prev;
//
//    for ( prev = room->affected; prev != NULL; prev = prev->next ) {
//      if ( prev->next == paf ) {
//	prev->next = paf->next;
//	break;
//      }
//    }
//    if ( prev == NULL ) {
//      bug( "Affect_remove_room: cannot find paf.");
//      return;
//    }
//  }
//
//  return;
//}

// SinaC 2003: new affect system
bool affect_remove_room_no_recompute( ROOM_INDEX_DATA *room, AFFECT_DATA *paf) {
  if ( room->current_affected == NULL ) {
    bug( "Affect_remove_room: no affect.");
    return FALSE;
  }

  if ( paf == room->current_affected )
    room->current_affected    = paf->next;
  else {
    AFFECT_DATA *prev;

    for ( prev = room->current_affected; prev != NULL; prev = prev->next ) {
      if ( prev->next == paf ) {
	prev->next = paf->next;
	break;
      }
    }
    if ( prev == NULL ) {
      bug( "Affect_remove_room: cannot find paf.");
      return FALSE;
    }
  }

  // SinaC 2003, affect is not valid anymore
  //  only used in update_char, _obj, _room
  //  because some affects may be removed from a wearoff/update_affect function
  REMOVE_BIT( paf->flags, AFFECT_IS_VALID );

  return TRUE;
}

//Strip all affects of a given sn.
void affect_strip( CHAR_DATA *ch, const int sn ) {
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  for ( paf = ch->affected; paf != NULL; paf = paf_next ) {
    paf_next = paf->next;
    if ( paf->type == sn )
      affect_remove_no_recompute( ch, paf ); // Modified by SinaC 2003
  }

  // SinaC 2003, affect has been modified
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );

  recompute(ch); // recomproom(ch->in_room) should maybe be done
  return;
}
void affect_strip_obj( OBJ_DATA *obj, const int sn ) {
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  for ( paf = obj->affected; paf != NULL; paf = paf_next ) {
    paf_next = paf->next;
    if ( paf->type == sn )
      affect_remove_obj_no_recompute( obj, paf ); // Modified by SinaC 2003
  }

  recompobj(obj); // recomproom(obj->in_room) or recompute(ch->carried_by) should maybe be done
  return;
}
// Added by SinaC 2001 for room affects
void affect_strip_room( ROOM_INDEX_DATA *room, const int sn ) {
  AFFECT_DATA *paf;
  AFFECT_DATA *paf_next;

  for ( paf = room->current_affected; paf != NULL; paf = paf_next ) {
    paf_next = paf->next;
    if ( paf->type == sn )
      affect_remove_room_no_recompute( room, paf ); // Modified by SinaC 2003
  }

  recomproom(room); // recompute(people) or recompobj(contains) should maybe be done
  return;
}

//Return true if a char is affected by a spell.
bool is_affected( CHAR_DATA *ch, const int sn ) {
  AFFECT_DATA *paf;

  for ( paf = ch->affected; paf != NULL; paf = paf->next ) {
    if ( paf->type == sn )
      return TRUE;
  }

  return FALSE;
}
// Added by SinaC 2001 for obj affects
bool is_affected_obj( OBJ_DATA *obj, const int sn ) {
  AFFECT_DATA *paf;

  for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
    if ( IS_SET( paf->flags, AFFECT_IS_VALID ) && paf->type == sn )
      return TRUE;
  }

  return FALSE;
}
// Added by SinaC 2001 for room affects
// Return true if a room is affected by a spell.
bool is_affected_room( ROOM_INDEX_DATA *room, const int sn ) {
  AFFECT_DATA *paf;

  for ( paf = room->base_affected; paf != NULL; paf = paf->next ) {
    if ( IS_SET( paf->flags, AFFECT_IS_VALID ) && paf->type == sn )
      return TRUE;
  }
  for ( paf = room->current_affected; paf != NULL; paf = paf->next ) {
    if ( paf->type == sn )
      return TRUE;
  }

  return FALSE;
}

// Add or enhance an affect.
//void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf ) {
//  AFFECT_DATA *paf_old;
//  bool found;
//
//  found = FALSE;
//  for ( paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next ) {
//    if ( paf_old->type == paf->type && paf->location==paf_old->location
//	 && paf->op == paf_old->op ) {
//      paf->level = (paf->level + paf_old->level) / 2;
//      paf->duration += paf_old->duration;
//      if (paf->op == AFOP_ADD)
//	paf->modifier += paf_old->modifier;
//      affect_remove_no_recompute( ch, paf_old ); // Modified by SinaC 2003
//      break;
//    }
//  }
//
//  affect_to_char( ch, paf );
//  return;
//}

// SinaC 2003: new affect system
// find an affect in ch's affect with same type
//    for each affects in affect->list
//       find an affect with same location and operator
//           adapt modifier
//       if not found, add to affect_list
//    adapt level, duration and casting_level
// if not found, add affects
void affect_join( CHAR_DATA *ch, AFFECT_DATA *paf ) {
  bool found = FALSE;
  // loop on ch's affects
  for ( AFFECT_DATA *paf_old = ch->affected; paf_old != NULL; paf_old = paf_old->next )
    if ( paf_old->type == paf->type ) { // found an affect_data with the same type
      // for each paf->affect_list:
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
	bool lFound = FALSE;
	//   search in affect_list if we find same location, operator and where
	for ( AFFECT_LIST *laf_old = paf_old->list; laf_old != NULL; laf_old = laf_old->next )
	  if ( laf_old->location == laf->location // found an affect_list with same loc, op and where
	       && laf_old->op == laf->op
	       && laf_old->where == laf->where ) {
	    if (laf_old->op == AFOP_ADD)
	      laf_old->modifier += laf->modifier;
	    lFound = TRUE; found = TRUE;
	  }
	if ( !lFound ) // not found: add to affect_list
	  addaff2(*paf_old,laf->where,laf->location,laf->op,laf->modifier);
      }
      if ( found ) { // if affect_list found: change level, duration, and casting_level
	paf_old->level = (paf->level + paf_old->level) / 2;
	if ( paf_old->duration * paf->duration >= 0 ) // both negative or positive
	  paf_old->duration += paf->duration;
	else // only one negative
	  if ( paf->duration < 0 ) { // if new affect has negative duration: use this new duration
	    paf_old->duration = paf->duration;
	    paf_old->flags |= paf->flags; // and get new affect flags
	  }
	paf_old->casting_level = (paf->casting_level + paf_old->casting_level) / 2;
	// FIXME: and flags ?
	break;
      }
    }
  if ( !found ) // not found: add to affects
    affect_to_char( ch, paf );

  // SinaC 2003, affect has been modified
  SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );

  return;
}

// SinaC 2003: new affect system
// find an affect in obj's affect with same type
//    for each affects in affect->list
//       find an affect with same location and operator
//           adapt modifier
//       if not found, add to affect_list
//    adapt level, duration and casting_level
// if not found, add affects
void affect_join_obj( OBJ_DATA *obj, AFFECT_DATA *paf ) {
  bool found = FALSE;
  // loop on ch's affects
  for ( AFFECT_DATA *paf_old = obj->affected; paf_old != NULL; paf_old = paf_old->next )
    if ( paf_old->type == paf->type ) { // found an affect_data with the same type
      // for each paf->affect_list:
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
	bool lFound = FALSE;
	//   search in affect_list of we find same location, operator and where
	for ( AFFECT_LIST *laf_old = paf_old->list; laf_old != NULL; laf_old = laf_old->next )
	  if ( laf_old->location == laf->location // found an affect_list with same loc, op and where
	       && laf_old->op == laf->op
	       && laf_old->where == laf->where ) {
	    if (laf_old->op == AFOP_ADD)
	      laf_old->modifier += laf->modifier;
	    lFound = TRUE; found = TRUE;
	  }
	if ( !lFound ) // not found: add to affect_list
	  addaff2(*paf_old,laf->where,laf->location,laf->op,laf->modifier);
      }
      if ( found ) { // if affect_list found: change level, duration, and casting_level
	paf_old->level = (paf->level + paf_old->level) / 2;
	paf_old->duration += paf_old->duration;
	paf_old->casting_level = (paf->casting_level + paf_old->casting_level) / 2;
	// FIXME: and flags ?
	break;
      }
    }
  if ( !found ) // not found: add to affects
    affect_to_obj( obj, paf );
  return;
}

// SinaC 2003: new affect system
// find an affect in room's affect with same type
//    for each affects in affect->list
//       find an affect with same location and operator
//           adapt modifier
//       if not found, add to affect_list
//    adapt level, duration and casting_level
// if not found, add affects
void affect_join_room( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) {
  bool found = FALSE;
  // loop on ch's affects
  for ( AFFECT_DATA *paf_old = room->current_affected; paf_old != NULL; paf_old = paf_old->next )
    if ( paf_old->type == paf->type ) { // found an affect_data with the same type
      // for each paf->affect_list:
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next ) {
	bool lFound = FALSE;
	//   search in affect_list of we find same location, operator and where
	for ( AFFECT_LIST *laf_old = paf_old->list; laf_old != NULL; laf_old = laf_old->next )
	  if ( laf_old->location == laf->location // found an affect_list with same loc, op and where
	       && laf_old->op == laf->op
	       && laf_old->where == laf->where ) {
	    if (laf_old->op == AFOP_ADD)
	      laf_old->modifier += laf->modifier;
	    lFound = TRUE; found = TRUE;
	  }
	if ( !lFound ) // not found: add to affect_list
	  addaff2(*paf_old,laf->where,laf->location,laf->op,laf->modifier);
      }
      if ( found ) { // if affect_list found: change level, duration, and casting_level
	paf_old->level = (paf->level + paf_old->level) / 2;
	paf_old->duration += paf_old->duration;
	paf_old->casting_level = (paf->casting_level + paf_old->casting_level) / 2;
	// FIXME: and flags ?
	break;
      }
    }
  if ( !found ) // not found: add to affects
    affect_to_room( room, paf );
  return;
}

/*
 * Move a char out of a room.
 */
void char_from_room( CHAR_DATA *ch ) {
  OBJ_DATA *obj;

  if ( ch->in_room == NULL ) {
    bug( "Char_from_room: NULL.");
    return;
  }

  if ( !IS_NPC(ch) )
    --ch->in_room->area->nplayer;

  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
       &&   obj->item_type == ITEM_LIGHT
       &&   obj->value[2] != 0
       // Modified by SinaC 2001
       &&   ch->in_room->bstat(light) > 0 )
    --ch->in_room->bstat(light);

  if ( ch == ch->in_room->people ) {
    ch->in_room->people = ch->next_in_room;
  }
  else {
    CHAR_DATA *prev;
    for ( prev = ch->in_room->people; prev; prev = prev->next_in_room ) {
      if ( prev->next_in_room == ch ) {
	prev->next_in_room = ch->next_in_room;
	break;
      }
    }

    if ( prev == NULL )
      // it happens when a player with a pet starts to log but fail the
      //  password test, SinaC 2000, pet->in_room is set but not already inserted in the room
      bug( "Char_from_room: ch [%s][vnum %d] [in_room: %d] not found.",
	   NAME(ch), ch->pIndexData?ch->pIndexData->vnum:-1,
	   ch->in_room?ch->in_room->vnum:-1);
  }

  // Removed by SinaC 2003
  // Added by SinaC 2001
  //ROOM_INDEX_DATA *pRoom = ch->in_room;

  ch->in_room      = NULL;
  ch->next_in_room = NULL;
  ch->on 	     = NULL;  /* sanity check! */

  // Removed by SinaC 2003, recompute are done in char_to_room.
  //  char_from_room is always followed by a char_to_room
  // Added by SinaC 2001
  //recompute(ch);
  //recomproom(pRoom);

  return;
}



/*
 * Move a char into a room.
 */
void char_to_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex )
{
  OBJ_DATA *obj;

  if ( pRoomIndex == NULL ) {
    ROOM_INDEX_DATA *room;

    bug( "Char_to_room: NULL.");

    //if ((room = get_room_index(ROOM_VNUM_TEMPLE)) != NULL)
    //  char_to_room(ch,room);
    if ( (room = get_recall_room(ch) ) == NULL )
      char_to_room(ch,room);

    return;
  }

  ch->in_room		= pRoomIndex;
  ch->next_in_room	= pRoomIndex->people;
  pRoomIndex->people	= ch;

  if ( !IS_NPC(ch) ) {
    if (ch->in_room->area->empty) {
      ch->in_room->area->empty = FALSE;
      ch->in_room->area->age = 0;
    }
    ++ch->in_room->area->nplayer;
  }

  if ( ( obj = get_eq_char( ch, WEAR_LIGHT ) ) != NULL
       &&   obj->item_type == ITEM_LIGHT
       &&   obj->value[2] != 0 )
    // Modified by SinaC 2001
    ++ch->in_room->bstat(light);

  // Added by SinaC 2001
  recomproom(ch->in_room);
  recompute(ch);

  // FIXME: Plague was propagating on move

  return;
}



/*
 * Give an obj to a char.
 */
void obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch )
{
  // Modified by SinaC 2000
  if (obj->item_type != ITEM_MONEY ) {
    obj->next_content	 = ch->carrying;
    ch->carrying	 = obj;
    obj->carried_by	 = ch;
    obj->in_room	 = NULL;
    obj->in_obj		 = NULL;
    ch->carry_number	+= get_obj_number( obj );
    ch->carry_weight	+= get_obj_weight( obj );

    // Added by SinaC 2001
    recompobj(obj);
  }
  else {
    ch->gold+=obj->value[1];
    ch->silver+=obj->value[0];
    extract_obj( obj );
  }
}



/*
 * Take an obj from its character.
 */
void obj_from_char( OBJ_DATA *obj ) {
  CHAR_DATA *ch;

  if ( ( ch = obj->carried_by ) == NULL ) {
    bug( "Obj_from_char: null ch. (obj: %s[%d])", obj->short_descr, obj->pIndexData->vnum );
    return;
  }

  if ( obj->wear_loc != WEAR_NONE )
    unequip_char( ch, obj );

  if ( ch->carrying == obj ) {
    ch->carrying = obj->next_content;
  }
  else {
    OBJ_DATA *prev;

    for ( prev = ch->carrying; prev != NULL; prev = prev->next_content ) {
      if ( prev->next_content == obj ) {
	prev->next_content = obj->next_content;
	break;
      }
    }

    if ( prev == NULL )
      bug( "Obj_from_char: obj not in list.");
  }

  obj->carried_by	 = NULL;
  obj->next_content	 = NULL;
  ch->carry_number	-= get_obj_number( obj );
  ch->carry_weight	-= get_obj_weight( obj );

  return;
}

void obj_from_char_no_recompute( OBJ_DATA *obj ) {
  CHAR_DATA *ch;

  if ( ( ch = obj->carried_by ) == NULL ) {
    bug( "Obj_from_char: null ch. (obj: %s[%d])", obj->short_descr, obj->pIndexData->vnum );
    return;
  }

  if ( obj->wear_loc != WEAR_NONE )
    unequip_char_no_recompute( ch, obj );

  if ( ch->carrying == obj ) {
    ch->carrying = obj->next_content;
  }
  else {
    OBJ_DATA *prev;

    for ( prev = ch->carrying; prev != NULL; prev = prev->next_content ) {
      if ( prev->next_content == obj ) {
	prev->next_content = obj->next_content;
	break;
      }
    }

    if ( prev == NULL )
      bug( "Obj_from_char: obj not in list.");
  }

  obj->carried_by	 = NULL;
  obj->next_content	 = NULL;
  ch->carry_number	-= get_obj_number( obj );
  ch->carry_weight	-= get_obj_weight( obj );

  return;
}



/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac( OBJ_DATA *obj, int iWear, int type )
{
  if ( obj->item_type != ITEM_ARMOR )
    return 0;

  // floating and light doesn't count in AC  SinaC 2000
  // neither wield and secondary, neither brand mark SinaC 2001
  // neither wield2 and wield3 SinaC 2003

  switch ( iWear ) {
  case WEAR_BODY:	return 3 * obj->value[type];
  case WEAR_HEAD:	return 2 * obj->value[type];
  case WEAR_LEGS:	return 2 * obj->value[type];
  case WEAR_FEET:	return     obj->value[type];
  case WEAR_HANDS:	return     obj->value[type];
  case WEAR_ARMS:	return     obj->value[type];
  case WEAR_SHIELD:	return     obj->value[type];
  case WEAR_FINGER_L:   return     obj->value[type];
  case WEAR_FINGER_R:   return     obj->value[type];
  case WEAR_NECK_1:	return     obj->value[type];
  case WEAR_NECK_2:	return     obj->value[type];
  case WEAR_ABOUT:	return 2 * obj->value[type];
  case WEAR_WAIST:	return     obj->value[type];
  case WEAR_WRIST_L:	return     obj->value[type];
  case WEAR_WRIST_R:	return     obj->value[type];
  case WEAR_HOLD:	return     obj->value[type];
  case WEAR_EAR_L:      return     obj->value[type];
  case WEAR_EAR_R:      return     obj->value[type];
  case WEAR_EYES:       return     obj->value[type];
  }

  return 0;
}

/*
 * Find a piece of eq on a character.
 */
OBJ_DATA *get_eq_char( CHAR_DATA *ch, int iWear )
{
  OBJ_DATA *obj;

  if (ch == NULL || !ch->valid )
    return NULL;

  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj->wear_loc == iWear )
      return obj;
  }

  return NULL;
}

/*
 * Equip a char with an obj.
 */
//void equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) {
// return value:  0: OK
//               -1: item already equipped
//               -2: part missing
//               -3: restriction or size
//               -4: anti-good, ...
//               -5: missing form
int equip_char( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) { // SinaC 2003: returned value used in reset_room
  if ( get_eq_char( ch, iWear ) != NULL ) {
    bug( "Equip_char: already equipped (%d) (ch: %d) (obj: %d).", 
	 iWear, IS_NPC(ch)?ch->pIndexData->vnum:-1, obj->pIndexData->vnum );
    return -1;
  }

  // Check needed parts, SinaC 2003
  //  if ( wear_needed_parts_form[iWear]
  //       && !IS_SET( ch->cstat(parts), wear_needed_parts[iWear] ) ) {
  if ( wear_needed_parts_form[iWear][0]
       && !IS_SET( ch->cstat(parts), wear_needed_parts_form[iWear][0] ) ) {
    act("You need $T in order to wear $p.", ch, obj, 
	list_flag_string(wear_needed_parts_form[iWear][0],part_flags,""," and "),
	TO_CHAR );
    return -2;
  }
  if ( wear_needed_parts_form[iWear][1]
       && !IS_SET( ch->cstat(form),  wear_needed_parts_form[iWear][1] ) ) {
    act("You can't wear $p.", ch, obj, NULL, TO_CHAR );
    return -5;
  }

  // Wear message, SinaC 2003
  act( wear_msg[iWear][0], ch, obj, NULL, TO_ROOM );
  act( wear_msg[iWear][1], ch, obj, NULL, TO_CHAR );

  // check_size added by SinaC 2003
  // replace anti-good, anti-neutral, anti-evil test in fight.C, SinaC 2001
  if (!check_restriction( ch, obj )
      || !check_size( ch, obj, TRUE ) ) {
    act("You can't use $p.", ch, obj, NULL, TO_CHAR );
    act("$n can't use $p.", ch, obj, NULL, TO_ROOM );
    obj_from_char( obj );
    
    obj_to_char( obj, ch ); // SinaC 2003, items are not dropped anymore, just put in inventory
    //if ( IS_OBJ_STAT(obj,ITEM_NODROP) )
    //  obj_to_char( obj, ch );
    //else { // Modified by SinaC 2001
    //  obj_to_room( obj, ch->in_room );
    //  OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
    //  recomproom(ch->in_room);
    //}
    return -3;
  }

  // Too be replaced with restrictions
  if ( ( IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)    && IS_EVIL(ch)    )
       ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_GOOD)    && IS_GOOD(ch)    )
       ||   ( IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch) ) ) {
     //Thanks to Morgenes for the bug fix here!
    act( "You are zapped by $p and drop it.", ch, obj, NULL, TO_CHAR );
    act( "$n is zapped by $p and drops it.",  ch, obj, NULL, TO_ROOM );
    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    OBJPROG(obj,ch,"onDropped",ch); // Added by SinaC 2003
    recomproom(ch->in_room);
    return -4;
  }

  obj->wear_loc	 = iWear;

  recompute(ch);

  if ( obj->item_type == ITEM_LIGHT
       &&   obj->value[2] != 0
       &&   ch->in_room != NULL ) {
    // Modified by SinaC 2001
    ++ch->in_room->bstat(light);
    // Added by SinaC 2001, not really 'drop' but we need to recompute the room
    recomproom(ch->in_room);
  }

  OBJPROG( obj, ch, "onWorn", ch );

  return 0;
}

/*
 * Unequip a char with an obj.
 */
void unequip_char( CHAR_DATA *ch, OBJ_DATA *obj )
{
  if ( obj->wear_loc == WEAR_NONE ) {
    bug( "Unequip_char: already unequipped.");
    return;
  }

  obj->wear_loc	 = -1;
  recompute(ch);

  if ( obj->item_type == ITEM_LIGHT
       &&   obj->value[2] != 0
       &&   ch->in_room != NULL
       // Modified by SinaC 2001
       &&   ch->in_room->bstat(light) > 0 ) {
    --ch->in_room->bstat(light);
    // Added by SinaC 2001
    recomproom(ch->in_room);
  }

  OBJPROG( obj, ch, "onRemoved", ch );

  return;
}

void unequip_char_no_recompute( CHAR_DATA *ch, OBJ_DATA *obj ) {
  if ( obj->wear_loc == WEAR_NONE ) {
    bug( "Unequip_char: already unequipped.");
    return;
  }

  obj->wear_loc	 = -1;

  if ( obj->item_type == ITEM_LIGHT
       &&   obj->value[2] != 0
       &&   ch->in_room != NULL
       // Modified by SinaC 2001
       &&   ch->in_room->bstat(light) > 0 )
    --ch->in_room->bstat(light);

  OBJPROG( obj, ch, "onRemoved", ch );

  return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list( OBJ_INDEX_DATA *pObjIndex, OBJ_DATA *list )
{
  OBJ_DATA *obj;
  int nMatch;

  nMatch = 0;
  for ( obj = list; obj != NULL; obj = obj->next_content ) {
    if ( obj->pIndexData == pObjIndex )
      nMatch++;
  }

  return nMatch;
}



/*
 * Move an obj out of a room.
 */
void obj_from_room( OBJ_DATA *obj )
{
  ROOM_INDEX_DATA *in_room;
  CHAR_DATA *ch;

  if ( ( in_room = obj->in_room ) == NULL ) {
    bug( "obj_from_room: NULL.");
    return;
  }

  for (ch = in_room->people; ch != NULL; ch = ch->next_in_room)
    if (ch->on == obj)
      ch->on = NULL;

  if ( obj == in_room->contents ) {
    in_room->contents = obj->next_content;
  }
  else {
    OBJ_DATA *prev;

    for ( prev = in_room->contents; prev; prev = prev->next_content ) {
      if ( prev->next_content == obj ) {
	prev->next_content = obj->next_content;
	break;
      }
    }

    if ( prev == NULL ) {
      bug( "Obj_from_room: obj not found.");
      return;
    }
  }

  obj->in_room      = NULL;
  obj->next_content = NULL;
  return;
}



/*
 * Move an obj into a room.
 */
void obj_to_room( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) {
  obj->next_content		= pRoomIndex->contents;
  pRoomIndex->contents	= obj;
  obj->in_room		= pRoomIndex;
  obj->carried_by		= NULL;
  obj->in_obj			= NULL;

  // Added by SinaC 2001
  recompobj(obj);

  return;
}



/*
 * Move an object into an object.
 */
void obj_to_obj( OBJ_DATA *obj, OBJ_DATA *obj_to ) {
  obj->next_content		= obj_to->contains;
  obj_to->contains		= obj;
  obj->in_obj			= obj_to;
  obj->in_room		= NULL;
  obj->carried_by		= NULL;
  if (obj_to->pIndexData->vnum == OBJ_VNUM_PIT)
    obj->cost = 0; 

  for ( ; obj_to != NULL; obj_to = obj_to->in_obj ) {
    if ( obj_to->carried_by != NULL ) {
      obj_to->carried_by->carry_number += get_obj_number( obj );
      obj_to->carried_by->carry_weight += get_obj_weight( obj )
	* WEIGHT_MULT(obj_to) / 100;
    }
  }

  return;
}



/*
 * Move an object out of an object.
 */
void obj_from_obj( OBJ_DATA *obj ) {
  OBJ_DATA *obj_from;

  if ( ( obj_from = obj->in_obj ) == NULL ) {
    bug( "Obj_from_obj: null obj_from.");
    return;
  }

  if ( obj == obj_from->contains ) {
    obj_from->contains = obj->next_content;
  }
  else {
    OBJ_DATA *prev;

    for ( prev = obj_from->contains; prev; prev = prev->next_content ) {
      if ( prev->next_content == obj ) {
	prev->next_content = obj->next_content;
	break;
      }
    }

    if ( prev == NULL ) {
      bug( "Obj_from_obj: obj not found.");
      return;
    }
  }

  obj->next_content = NULL;
  obj->in_obj       = NULL;

  for ( ; obj_from != NULL; obj_from = obj_from->in_obj ) {
    if ( obj_from->carried_by != NULL ) {
      obj_from->carried_by->carry_number -= get_obj_number( obj );
      obj_from->carried_by->carry_weight -= get_obj_weight( obj ) 
	* WEIGHT_MULT(obj_from) / 100;
    }
  }

  return;
}

/*
 * Extract an obj from the world.
 */
void extract_obj( OBJ_DATA *obj ) {
  OBJ_DATA *obj_content;
  OBJ_DATA *obj_next;

  if ( obj == NULL || !obj->valid ) {
    bug( "Extract_obj: obj %d already extracted.", obj!=NULL?(obj->pIndexData?obj->pIndexData->vnum:-1):-1 );
    return;
  }

  if ( IS_SET( obj->extra_flags, ITEM_UNIQUE ) )
    log_stringf("Extract_obj: obj [%d] is unique.", obj->pIndexData->vnum );

  // Modified by SinaC 2001
  if ( obj->in_room != NULL ) {
    ROOM_INDEX_DATA *room = obj->in_room;

    // Added by SinaC 2003 for furniture vanishing
    for (CHAR_DATA *fch = room->people; fch != NULL; fch = fch->next_in_room)
      if (fch->on == obj)
	fch->on = NULL;

    obj_from_room( obj );
    recomproom(room);
  }
  // Modified by SinaC 2001
  else if ( obj->carried_by != NULL ) {
    CHAR_DATA *ch = obj->carried_by;
    obj_from_char( obj );
    recompute(ch);
  }
  else if ( obj->in_obj != NULL )
    obj_from_obj( obj );

  for ( obj_content = obj->contains; obj_content; obj_content = obj_next ) {
    obj_next = obj_content->next_content;
    extract_obj( obj_content );
  }

  if ( object_list == obj ) {
    object_list = obj->next;
  }
  else {
    OBJ_DATA *prev;

    for ( prev = object_list; prev != NULL; prev = prev->next ) {
      if ( prev->next == obj ) {
	prev->next = obj->next;
	break;
      }
    }

    if ( prev == NULL ) {
      bug( "Extract_obj: obj %d not found.", obj->pIndexData->vnum );
      return;
    }
  }

  --obj->pIndexData->count;
  free_obj(obj);
  return;
}



/*
 * Extract a char from the world.
 */
// SinaC 2003: call with notInRoom TRUE if you want to remove a mob which is not in a room
// notInRoom default value is FALSE
void extract_char( CHAR_DATA *ch, bool fPull, const bool notInRoom ) {
  CHAR_DATA *wch;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if ( ch == NULL || !ch->valid ) {
    bug( "Extract_char: char %s already extracted.", ch!=NULL?(ch->name?ch->name:"NULL"):"NULL" );
    return;
  }

  if ( !notInRoom ) { // SinaC 2003
    if ( ch->in_room == NULL ) {
      // it happens when a player with a pet starts to log but fail the
      //  password test, SinaC 2000
      // FIXED: Not happens anymore: pet's room are saved in pFile, the problem happens later: char_from_room
      bug( "Extract_char: in_room is NULL.");
      return;
    }
  }
  nuke_pets(ch);
  ch->pet = NULL; /* just in case */
  
  // Added by SinaC 2000 for stunned people
  ch->stunned = 0;
  
  if ( fPull )
    die_follower( ch );
  
  stop_fighting( ch, TRUE );
  
  for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;
    
    // if stay death stays in the right wear loc       SinaC 2001
    //  but add an extra test in extract_char to avoid losing it definitively
    if ( !fPull && !IS_NPC(ch) && IS_SET( obj->extra_flags, ITEM_STAY_DEATH ) )
      continue;
    extract_obj( obj );
  }
  
  if ( !notInRoom ) { // SinaC 2003
    // Added by SinaC 2001
    ROOM_INDEX_DATA *room = ch->in_room;
    char_from_room( ch );
    recomproom(room);
  }

  /* Death room is set in the clan table now */
  if ( !fPull ) {
    //if ( ch->clan )
    //  char_to_room(ch,get_room_index(get_clan_table(ch->clan)->hall));
    //else
    //  char_to_room(ch, get_hall_room(ch) );
    char_to_room( ch, get_hall_room( ch ) );
    return;
  }

  if ( IS_NPC(ch) )
    --ch->pIndexData->count;

  if ( ch->desc != NULL && ch->desc->original != NULL ) {
    do_return( ch, "" );
    ch->desc = NULL;
  }

  for ( wch = char_list; wch != NULL; wch = wch->next ) {
    if ( wch->reply == ch )
      wch->reply = NULL;
  }

  if ( ch == char_list ) {
    char_list = ch->next;
  }
  else {
    CHAR_DATA *prev;
    
    for ( prev = char_list; prev != NULL; prev = prev->next ) {
      if ( prev->next == ch ) {
	prev->next = ch->next;
	break;
      }
    }
    
    if ( prev == NULL )	{
      bug( "Extract_char: char not found.");
      return;
    }
  }
  
  if ( ch->desc != NULL )
    ch->desc->character = NULL;
  
  free_char( ch );
  return;
}



/*
 * Find a char in the room.
 */
CHAR_DATA *get_char_room( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  int number;
  int count;

  number = number_argument( argument, arg );

#ifdef VERBOSE
  log_stringf("get_char_room: %s  looking for  %d.%s", NAME(ch), number, arg );
#endif

  count  = 0;
  if ( !str_cmp( arg, "self" ) )
    return ch;
  for ( rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room ) {
    
#ifdef VERBOSE
    log_stringf("   rch: %s  count: %d",
		NAME(rch), count );
#endif

    if ( !can_see( ch, rch ) || !is_name( arg, rch->name ) )
      continue;

    if ( ++count == number )
      return rch;
  }

#ifdef VERBOSE
  log_stringf("%d.%s NOT found", number, arg );
#endif

  return NULL;
}




/*
 * Find a char in the world.
 */

/* OLD ROUTINE REPLACED
 * CHAR_DATA *get_char_world( CHAR_DATA *ch, const char *argument )
 *{
 *   char arg[MAX_INPUT_LENGTH];
 *   CHAR_DATA *wch;
 *   int number;
 *   int count;
 *
 *   if ( ( wch = get_char_room( ch, argument ) ) != NULL )
 *     return wch;
 *
 *   number = number_argument( argument, arg );
 *   count  = 0;
 *   for ( wch = char_list; wch != NULL ; wch = wch->next )
 *   {
 *       if ( wch->in_room == NULL || !can_see( ch, wch )
 *	||   !is_name( arg, wch->name ) )
 *	    continue;
 *	if ( ++count == number )
 *	    return wch;
 *   }
 *
 *   return NULL;
 *}
 */

// Modified by SinaC 2001
// Search in the room
// Search in the area
// Search everywhere else
CHAR_DATA *get_char_world( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *wch;
  DESCRIPTOR_DATA *d;
  int number;
  int count;

  // in room
  if ( ( wch = get_char_room( ch, argument ) ) != NULL )
    return wch;

  number = number_argument( argument, arg );

  // in area
  count = 0;
  if ( ch->in_room != NULL )
  for (d = descriptor_list; d != NULL; d = d->next) {
    if (d->connected == CON_PLAYING && d->character
	&&  d->character->in_room && can_see(ch,d->character)
	&&  is_name(arg,d->character->name) && ++count == number
	&&  d->character->in_room != NULL
	&&  d->character->in_room->area == ch->in_room->area )
      return d->character;
  }

  count  = 0;
  if ( ch->in_room != NULL )
  for ( wch = char_list; wch != NULL ; wch = wch->next ) {
    if ( wch->in_room == NULL || !can_see( ch, wch )
	 ||   !is_name( arg, wch->name ) 
	 || wch->in_room == NULL
	 || wch->in_room->area != ch->in_room->area )
      continue;
    if ( ++count == number )
      return wch;
  }

  // everywhere else
  count = 0;
  for (d = descriptor_list; d != NULL; d = d->next) {
    if (d->connected == CON_PLAYING && d->character
	&&  d->character->in_room && can_see(ch,d->character)
	&&  is_name(arg,d->character->name) && ++count == number)
      return d->character;
  }

  count  = 0;
  for ( wch = char_list; wch != NULL ; wch = wch->next ) {
    if ( wch->in_room == NULL || !can_see( ch, wch )
	 ||   !is_name( arg, wch->name ) )
      continue;
    if ( ++count == number )
      return wch;
  }

  return NULL;
}

// Added by SinaC 2003
CHAR_DATA *get_pc_world( const char *argument ) {
  for ( DESCRIPTOR_DATA *d = descriptor_list; d != NULL; d = d->next ) {
    if ( d->connected == CON_PLAYING 
	 &&  d->character
	 &&  d->character->in_room
	 &&  is_name( argument, d->character->name ) )
      return d->character;
  }
  return NULL;
}

/*
 * Find some object with a given index data.
 * Used by area-reset 'P' command.
 */
OBJ_DATA *get_obj_type( OBJ_INDEX_DATA *pObjIndex )
{
  OBJ_DATA *obj;

  for ( obj = object_list; obj != NULL; obj = obj->next ) {
    if ( obj->pIndexData == pObjIndex )
      return obj;
  }

  return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list( CHAR_DATA *ch, const char *argument, OBJ_DATA *list )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = list; obj != NULL; obj = obj->next_content ) {
    if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) ) {
      if ( ++count == number )
	return obj;
    }
  }

  return NULL;
}



/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry( CHAR_DATA *ch, const char *argument, CHAR_DATA *viewer )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj->wear_loc == WEAR_NONE
	 &&   (can_see_obj( viewer, obj ) ) 
	 &&   is_name( arg, obj->name ) ) {
      if ( ++count == number )
	return obj;
    }
  }

  return NULL;
}



/*
 * Find an obj in player's equipment.
 */
// Modified by SinaC 2000 to allow player to remove item they are wearing
//  even if they can't see it         added a new param 'bool rem'
OBJ_DATA *get_obj_wear( CHAR_DATA *ch, const char *argument, bool rem )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj->wear_loc != WEAR_NONE
	 // Added by SinaC 2000    || rem 
	 &&   ( can_see_obj( ch, obj ) || rem )
	 &&   is_name( arg, obj->name ) ) {
      if ( ++count == number )
	return obj;
    }
  }

  return NULL;
}



/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;

  obj = get_obj_list( ch, argument, ch->in_room->contents );
  if ( obj != NULL )
    return obj;

  if ( ( obj = get_obj_carry( ch, argument, ch ) ) != NULL )
    return obj;

  // Modified by SinaC 2000 to allow to remove invis item
  if ( ( obj = get_obj_wear( ch, argument, FALSE ) ) != NULL )
    return obj;

  return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;

  if ( ( obj = get_obj_here( ch, argument ) ) != NULL )
    return obj;

  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = object_list; obj != NULL; obj = obj->next ) {
    if ( can_see_obj( ch, obj ) && is_name( arg, obj->name ) ) {
      if ( ++count == number )
	return obj;
    }
  }

  return NULL;
}

/* deduct cost from a character */

void deduct_cost(CHAR_DATA *ch, int cost)
{
  int silver = 0, gold = 0;

  silver = UMIN(ch->silver,cost); 

  if (silver < cost) {
    gold = ((cost - silver + 99) / 100);
    silver = cost - 100 * gold;
  }

  ch->gold -= gold;
  ch->silver -= silver;

  if (ch->gold < 0) {
    bug("deduct costs: gold %ld < 0",ch->gold);
    ch->gold = 0;
  }
  if (ch->silver < 0) {
    bug("deduct costs: silver %ld < 0",ch->silver);
    ch->silver = 0;
  }
}   
/*
 * Create a 'money' obj.
 */
OBJ_DATA *create_money( int gold, int silver )
{
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;

  if ( gold < 0 || silver < 0 || (gold == 0 && silver == 0) ) {
    bug( "Create_money: zero or negative money.");
    gold = UMAX(1,gold);
    silver = UMAX(1,silver);
  }

  if (gold == 0 && silver == 1) {
    obj = create_object( get_obj_index( OBJ_VNUM_SILVER_ONE ), 0 );
  }
  else if (gold == 1 && silver == 0) {
    obj = create_object( get_obj_index( OBJ_VNUM_GOLD_ONE), 0 );
  }
  else if (silver == 0) {
    obj = create_object( get_obj_index( OBJ_VNUM_GOLD_SOME ), 0 );
    sprintf( buf, obj->short_descr, gold );
    obj->short_descr        = str_dup( buf );
  }
  else if (gold == 0) {
    obj = create_object( get_obj_index( OBJ_VNUM_SILVER_SOME ), 0 );
    sprintf( buf, obj->short_descr, silver );
    obj->short_descr        = str_dup( buf );
  }
  else {
    obj = create_object( get_obj_index( OBJ_VNUM_COINS ), 0 );
    sprintf( buf, obj->short_descr, silver, gold );
    obj->short_descr        = str_dup( buf );
  }
    
  obj->weight = MONEY_WEIGHT(gold,silver); /* Oxtal */
  obj->cost = 100*gold+silver;
  obj->baseval[0] = silver;
  obj->baseval[1] = gold;

  recompobj(obj);

  return obj;
}



/*
 * Return # of objects which an object counts as.
 * Thanks to Tony Chamberlain for the correct recursive code here.
 */
int get_obj_number( OBJ_DATA *obj )
{
  int number;
 
  if (obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_MONEY
      ||  obj->item_type == ITEM_GEM || obj->item_type == ITEM_JEWELRY)
    number = 0;
  else
    number = 1;
 
  for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
    number += get_obj_number( obj );
 
  return number;
}


/*
 * Return weight of an object, including weight of contents.
 */
int get_obj_weight( OBJ_DATA *obj )
{
  int weight;
  OBJ_DATA *tobj;

  weight = obj->weight;
  for ( tobj = obj->contains; tobj != NULL; tobj = tobj->next_content )
    weight += get_obj_weight( tobj ) * WEIGHT_MULT(obj) / 100;

  return weight;
}

int get_true_weight(OBJ_DATA *obj)
{
  int weight;
 
  weight = obj->weight;
  for ( obj = obj->contains; obj != NULL; obj = obj->next_content )
    weight += get_obj_weight( obj );
 
  return weight;
}

/*
 * True if room is dark.
 */
bool room_is_dark( ROOM_INDEX_DATA *pRoomIndex ) {
  // Added by SinaC 2003
  if ( pRoomIndex == NULL )
    return TRUE;

  // Modified by SinaC 2001
  if ( pRoomIndex->cstat(light) > 0 )
    return FALSE;

  if ( IS_SET(pRoomIndex->cstat(flags), ROOM_DARK) )
    return TRUE;

  // Modified by SinaC 2001
  if ( pRoomIndex->cstat(sector) == SECT_INSIDE
       || pRoomIndex->cstat(sector) == SECT_CITY 
       // Added by SinaC 2001
       || IS_SET(pRoomIndex->cstat(flags), ROOM_INDOORS) )
    return FALSE;

  if ( weather_info.sunlight == SUN_SET
       || weather_info.sunlight == SUN_DARK )
    return TRUE;

  return FALSE;
}


bool is_room_owner(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
  if (room->owner == NULL || room->owner[0] == '\0')
    return FALSE;

  return is_name(ch->name,room->owner);
}

/*
 * True if room is private.
 */
bool room_is_private( ROOM_INDEX_DATA *pRoomIndex )
{
  CHAR_DATA *rch;
  int count;

  if (pRoomIndex->owner != NULL && pRoomIndex->owner[0] != '\0')
    return TRUE;

  count = 0;
  for ( rch = pRoomIndex->people; rch != NULL; rch = rch->next_in_room )
    count++;

  // Modified by SinaC 2001
  if ( IS_SET(pRoomIndex->cstat(flags), ROOM_PRIVATE)  && count >= 2 )
    return TRUE;

  if ( IS_SET(pRoomIndex->cstat(flags), ROOM_SOLITARY) && count >= 1 )
    return TRUE;
    
  if ( IS_SET(pRoomIndex->cstat(flags), ROOM_IMP_ONLY) )
    return TRUE;

  return FALSE;
}

/* visibility on a room -- for entering and exits */
bool can_see_room( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) {
  // Added by SinaC 2000 to set area people can't enter it
  // Useful when creating a new area and we don't want people to enter it.
  // !!!Can see a noteleport room only if we are in battle SinaC 2000!!!
  if (IS_SET(pRoomIndex->area->area_flags, AREA_NOTELEPORT ) 
      && !IS_IMMORTAL(ch) && !IN_BATTLE(ch)) return FALSE;

  // Modified by SinaC 2001
  if (IS_SET(pRoomIndex->cstat(flags), ROOM_IMP_ONLY) 
      &&  get_trust(ch) < MAX_LEVEL)
    return FALSE;

  // Modified by SinaC 2001
  if (IS_SET(pRoomIndex->cstat(flags), ROOM_GODS_ONLY)
      &&  !IS_IMMORTAL(ch))
    return FALSE;

  // Modified by SinaC 2001
  if (IS_SET(pRoomIndex->cstat(flags), ROOM_HEROES_ONLY)
      &&  !IS_IMMORTAL(ch))
    return FALSE;

  // Modified by SinaC 2001
  if (IS_SET(pRoomIndex->cstat(flags),ROOM_NEWBIES_ONLY)
      &&  ch->level > 5 && !IS_IMMORTAL(ch))
    return FALSE;

  if (!IS_IMMORTAL(ch) 
      // Added by SinaC 2003, mobs can go in every clan rooms
      && !IS_NPC(ch)
      && pRoomIndex->clan && ch->clan != pRoomIndex->clan)
    return FALSE;

  return TRUE;
}


// this can_see is used in command like who, whois
bool can_see_ooc( CHAR_DATA *ch, CHAR_DATA *victim ) {

  /* RT changed so that WIZ_INVIS has levels */
  if ( ch == victim )
    return TRUE;

  if ( get_trust(ch) < victim->invis_level)
    return FALSE;

  if (get_trust(ch) < victim->incog_level 
      && ch->in_room != victim->in_room)
    return FALSE;

  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT )
       && IS_IMMORTAL(ch) )
    return TRUE;

  if ( IS_AFFECTED(ch, AFF_BLIND) )
    return FALSE;

  if ( IS_AFFECTED(victim, AFF_INVISIBLE)
       && !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
    return FALSE;
  
  /* sneaking */
  if ( IS_AFFECTED(victim, AFF_SNEAK)
       && !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
       && victim->fighting == NULL) {
    int chance;
    chance = get_ability(victim,gsn_sneak);
    chance += victim->cstat(DEX) * 3/2;
    chance -= ch->cstat(INT) * 2;
    chance -= ch->level - victim->level * 3/2;

    if (number_percent() < chance)
      return FALSE;
  }

  if ( IS_AFFECTED(victim, AFF_HIDE)
       &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
       &&   victim->fighting == NULL)
    return FALSE;

  return TRUE;
}



/*
 * True if char can see victim.
 */
bool can_see( CHAR_DATA *ch, CHAR_DATA *victim ) {

  /* RT changed so that WIZ_INVIS has levels */
  if ( ch == victim )
    return TRUE;

  if ( get_trust(ch) < victim->invis_level)
    return FALSE;

  if (get_trust(ch) < victim->incog_level 
      && ch->in_room != victim->in_room)
    return FALSE;

  /* Modified by SinaC 2000
   *if ( (!IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT)) 
   *||   (IS_NPC(ch) && IS_IMMORTAL(ch)))
   * return TRUE;
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT )
       && IS_IMMORTAL(ch) )
    return TRUE;

  if ( IS_AFFECTED(ch, AFF_BLIND) )
    return FALSE;

  // added by SinaC 2000 seems to be logic,  PARALYZED added by SinaC 2003
  if ( !IS_AWAKE(ch) && ch->position != POS_PARALYZED ) 
    return FALSE;
  // end

  if ( room_is_dark( ch->in_room ) 
       && !IS_AFFECTED(ch, AFF_INFRARED)
       // Added by SinaC 2000
       && !IS_AFFECTED(ch, AFF_DARK_VISION) )
    return FALSE;
  
  if ( IS_AFFECTED(victim, AFF_INVISIBLE)
       && !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
    return FALSE;
  
  /* sneaking */
  if ( IS_AFFECTED(victim, AFF_SNEAK)
       && !IS_AFFECTED(ch,AFF_DETECT_HIDDEN)
       && victim->fighting == NULL) {
    int chance;
    chance = get_ability(victim,gsn_sneak);
    chance += victim->cstat(DEX) * 3/2;
    chance -= ch->cstat(INT) * 2;
    chance -= ch->level - victim->level * 3/2;

    if (number_percent() < chance)
      return FALSE;
  }

  if ( IS_AFFECTED(victim, AFF_HIDE)
       &&   !IS_AFFECTED(ch, AFF_DETECT_HIDDEN)
       &&   victim->fighting == NULL)
    return FALSE;

  return TRUE;
}



/*
 * True if char can see obj.
 */
//#define IS_QUEST_TOKEN( obj ) (( (obj)->pIndexData->vnum >= QUEST_OBJQUEST1 && (obj)->pIndexData->vnum <= QUEST_OBJQUEST5 ) || (obj)->pIndexData->vnum == QUEST_OBJQUEST6)

bool can_see_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
  /* Modified by SinaC 2000
   * if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT) )
   * 	return TRUE;
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_HOLYLIGHT ) && IS_IMMORTAL(ch) )
    return TRUE;
  
  if ( IS_SET(obj->extra_flags,ITEM_VIS_DEATH))
    return FALSE;

  if ( IS_AFFECTED( ch, AFF_BLIND ) && obj->item_type != ITEM_POTION)
    return FALSE;

  if ( obj->item_type == ITEM_LIGHT && obj->value[2] != 0 )
    return TRUE;

  if ( IS_SET(obj->extra_flags, ITEM_INVIS)
       &&   !IS_AFFECTED(ch, AFF_DETECT_INVIS) )
    return FALSE;

  // Removed by SinaC 2003
  // Added by SinaC 2000,  quest tokens can be only see by people who are
  //  searching them and the questmaster, modified by SinaC 2003
  //  if ( IS_QUEST_TOKEN(obj) )
  //    if ( IS_NPC(ch) )
  //      if ( ch->spec_fun == spec_lookup( "spec_questmaster" ) )
  //        return TRUE;  // questmaster
  //      else
  //        return FALSE; // not a questmaster
  //    else
  //      if ( ch->pcdata->questobj != obj->pIndexData->vnum )
  //    return FALSE; // player not doing this quest
  //    else
  //        return TRUE;  // player doing this quest
  // end of modif

  if ( IS_OBJ_STAT(obj,ITEM_GLOW))
    return TRUE;

  if ( room_is_dark( ch->in_room ) && !IS_AFFECTED(ch, AFF_INFRARED) )
    return FALSE;

  return TRUE;
}



/*
 * True if char can drop obj.
 */
bool can_drop_obj( CHAR_DATA *ch, OBJ_DATA *obj )
{
  if ( !IS_SET(obj->extra_flags, ITEM_NODROP) )
    return TRUE;

  if ( !IS_NPC(ch) && IS_IMMORTAL(ch) )
    return TRUE;

  return FALSE;
}


// Added by SinaC

CHAR_DATA *get_char( CHAR_DATA *ch )
{
  if ( !ch->pcdata )
    return ch->desc->original;
  else
    return ch;
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
bool fumble_obj( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj_drop, int level, bool drop ) {
  if ( drop ){
    if ( !can_drop_obj( victim, obj_drop ) )
      return FALSE;
  }
  else{
    if ( IS_OBJ_STAT( obj_drop, ITEM_NOREMOVE ) )
      return FALSE;
  }

  if ( saves_spell( level, victim, DAM_OTHER ) ){
    act( "You nearly $T $p, but manage to keep your grip.",
	 victim, obj_drop, drop ? "drop" : "lose hold of", TO_CHAR );
    act( "$n nearly $T $p, but manages to keep $s grip.",
	 victim, obj_drop, drop ? "drops" : "loses hold of", TO_ROOM );
    return FALSE;
  }
  
  if ( drop ) {
    Value args[] = {ch};
    bool result = objprog( obj_drop, ch, "onDropping", args);
    if ( result ) // if onDropping returns 1,  item is not dropped
      return FALSE;
    obj_from_char( obj_drop );
    obj_to_room( obj_drop, victim->in_room );
    OBJPROG(obj_drop,victim,"onDropped",ch); // Added by SinaC 2003
  }
  else {
    unequip_char( victim, obj_drop );
  }
  
  act( "You fumble and $T $p!",
       victim, obj_drop, drop ? "drop" : "lose hold of", TO_CHAR );
  act( "$n fumbles and $T $p!",
       victim, obj_drop, drop ? "drops" : "loses hold of", TO_ROOM );
  return TRUE;
}

bool is_made_flesh( CHAR_DATA *victim ) {
  if ( IS_SET( victim->cstat(form), FORM_ANIMAL ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_CENTAUR ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_MAMMAL ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_BIRD ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_REPTILE ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_SNAKE ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_DRAGON ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_AMPHIBIAN ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_FISH ) ) return TRUE;
  if ( IS_SET( victim->cstat(form), FORM_COLD_BLOOD ) ) return TRUE;

  return FALSE;
}


// Used for items with ITEM_STAY_DEATH flag
// it transfers every item with that flag from corpse to player
// crappy but it was the fastest and easiest way to do
void transfer_obj_stay_death( CHAR_DATA *ch, OBJ_DATA *corpse )
{
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  if ( corpse == NULL ) return;

  for ( obj = corpse->contains; obj != NULL; obj = obj_next ){
    obj_next = obj->next_content;
    
    if ( IS_SET( obj->extra_flags, ITEM_STAY_DEATH ) ){
      obj_from_obj( obj );
      obj_to_char( obj, ch );
      /* Removed by SinaC 2003
      if ( CAN_WEAR(obj, ITEM_BRAND ) )
	obj->wear_loc = WEAR_BRAND;
      */
    }
  }
}

int check_position( const char *pos )
{
  int i;

  for ( i = 0; position_table[i].name!=NULL; i++ )
    if ( !str_cmp( pos, position_table[i].short_name ) ) return i;

  return -1;
}

int check_target( const char *tar )
{
  int i;

  for ( i = 0; target_table[i].name!=NULL; i++ )
    if ( !str_cmp( tar, target_table[i].short_name ) ) return i;

  return -1;
}

bool exist_slot( int slot )
{
  int i;

  for ( i = 0; i < MAX_ABILITY; i++ ){
    if ( ability_table[i].name == NULL || ability_table[i].name[0] == '\0' ) 
      continue;
    if ( ability_table[i].slot == slot ) return TRUE;
  }
  return FALSE;
}

// Added by SinaC 2001 to check if a player can get that etho/align, 
//  based on race and class
bool check_etho_align( CHAR_DATA *ch, int etho, int align ) {
  // Modified by SinaC 2001: based on class too
  int race = ch->bstat(race); // SinaC 2003
  if ( !race_table[race].pc_race ) { //   if base race is not PC race
    race = DEFAULT_PC_RACE;             //     get default race: human
    bug("check_etho_align: invalid race for [%s] sn:[%d]",
	NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
  }
  //pc_race_type *pc_race = &(pc_race_table[ch->bstat(race)]);
  pc_race_type *pc_race = &(pc_race_table[race]);
  class_type *pc_class = &(class_table[class_firstclass(ch->bstat(classes))]);

  // SinaC 2003, wild mage can't be lawful
  if ( etho == ETHO_LAWFUL && ch->isWildMagic )
    return FALSE;

  /*
  // we check the allowed etho/align for the player race
  for ( int i = 0; i < pc_race->nb_allowed_align; i++ ){
    bool ethofound = ( pc_race->allowed_align[i].etho == etho );
    bool alignfound = ( pc_race->allowed_align[i].alignment == align );
    if ( ethofound && alignfound ) return TRUE;
  }
  */
  bool etho_race = FALSE, 
    etho_class = FALSE,
    align_race = FALSE, 
    align_class = FALSE;

  for ( int i = 0; i < pc_race->nb_allowed_align; i++ ){
    etho_race = ( pc_race->allowed_align[i].etho == etho );
    align_race = ( pc_race->allowed_align[i].alignment == align );
    if ( etho_race && align_race ) break;
  }
  if ( etho_race && align_race ) {
    // the test based on race is done, let's go with the class
    for ( int i = 0; i < pc_class->nb_allowed_align; i++ ){
      etho_class = ( pc_class->allowed_align[i].etho == etho );
      align_class = ( pc_class->allowed_align[i].alignment == align );
      if ( etho_class && align_class ) break;
    }
    if ( etho_class && align_class ) return TRUE;
  }

  return FALSE;
}

// Added by SinaC 2001 to check if a player can get that class, based on race
bool check_class_race( CHAR_DATA *ch, int iClass ) {
  int race = ch->bstat(race); // SinaC 2003
  if ( !race_table[race].pc_race ) { //   if base race is not PC race
    race = DEFAULT_PC_RACE;             //     get default race: human
    bug("check_class_race: invalid race for [%s] sn:[%d]",
	NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
  }
  //pc_race_type *pc_race = &(pc_race_table[ch->bstat(race)]);
  pc_race_type *pc_race = &(pc_race_table[race]);
  int allowed_class = pc_race->allowed_class;

  //  for ( int i = 0; i < pc_race->nb_allowed_class; i++ )
  //    if ( pc_race->allowed_class[i] == iClass ) return TRUE;
  if ( ( 1 << iClass ) & allowed_class )
    return TRUE;

  return FALSE;
}

// Added by SinaC 2001
// Check if a player can choose/worship a specific god
bool check_god( CHAR_DATA *ch, int who ) {
  god_data *god;
  bool ok;

  god = &(gods_table[who]);

  // We check the etho/align
  ok = FALSE;
  for ( int i = 0; i < god->nb_allowed_align && ok == FALSE; i++ ){
    // Modified by SinaC 2001 etho/alignment are attributes now
    //bool etho = god->allowed_align[i].etho == ch->align.etho;
    bool etho = god->allowed_align[i].etho == ch->bstat(etho);
    bool align = FALSE;
    if ( god->allowed_align[i].alignment <= -350 && BASE_IS_EVIL( ch ) )    align = TRUE;
    if ( god->allowed_align[i].alignment ==    0 && BASE_IS_NEUTRAL( ch ) ) align = TRUE;
    if ( god->allowed_align[i].alignment >=  350 && BASE_IS_GOOD( ch ) )    align = TRUE;
    if ( etho && align )
      ok = TRUE;
  }
  if ( !ok ) return FALSE;

  // We check race
  ok = FALSE;
  for ( int i = 0; i < god->nb_allowed_race && ok == FALSE; i++ ) {
    if ( god->allowed_race[i] == ch->bstat(race) )
      ok = TRUE;
  }
  if ( !ok ) return FALSE;

  // We check class
  //ok = FALSE;
  //for ( int i = 0; i < god->nb_allowed_class && ok == FALSE; i++ ){
  //  if ( (god->allowed_class[i] & ch->bstat(classes)) == god->allowed_class[i] ) 
  //    ok = TRUE;
  //}
  //if ( !ok ) return FALSE;
  if ( ( god->allowed_class & ch->bstat(classes) ) != ch->bstat(classes) )
    return FALSE;

  return TRUE;
}

// Added by SinaC 2001 to check if a god accept that class
bool check_class_god( int iClass, int who ) {
  god_data *god = &(gods_table[who]);
  int allowed_class = god->allowed_class;
  
  //  for ( int i = 0; i < god->nb_allowed_class; i++ )
  //    if ( god->allowed_class[i] == iClass )
  //      return TRUE;
  if ( ( 1 << iClass ) & allowed_class )
    return TRUE;

  return FALSE;
}

// Added by SinaC 2001 to easy race changement
void change_race( CHAR_DATA *victim, int race, int timer, int level, int sn, int cast ) {
  // Modified by SinaC 2003, only sn == -1 means permanent changement
  //  timer == -1 only means no duration, stays until another change

  // sn == -1 or timer <= -1  means permanent changement
  //if ( sn == -1 || timer <= -1 ) {
  if ( sn == -1 ) {
    victim->bstat(race)        = race;
    victim->bstat(affected_by) = race_table[race].aff;
    victim->bstat(affected2_by) = race_table[race].aff2;
    victim->bstat(imm_flags)   = race_table[race].imm;
    victim->bstat(res_flags)   = race_table[race].res;
    victim->bstat(vuln_flags)  = race_table[race].vuln;
    victim->bstat(parts)       = race_table[race].parts;
    victim->bstat(form)        = race_table[race].form;
    //if ( race_table[race].pc_race )
    //victim->bstat(size)      = pc_race_table[race].size;
    victim->bstat(size)        = race_table[race].size;
  }
  // temporary changement
  else {
    // Modified by SinaC 2003, previously OR were ASSIGN
    AFFECT_DATA af;
    createaff(af,timer,level,sn,cast,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,race,ASSIGN,race);
    addaff(af,CHAR,parts,ASSIGN,race_table[race].parts);
    addaff(af,CHAR,form,ASSIGN,race_table[race].form);
    addaff(af,CHAR,imm_flags,OR,race_table[race].imm);
    addaff(af,CHAR,res_flags,OR,race_table[race].res);
    addaff(af,CHAR,vuln_flags,OR,race_table[race].vuln);
    addaff(af,CHAR,affected_by,OR,race_table[race].aff);
    addaff(af,CHAR,affected2_by,OR,race_table[race].aff2);
    //    if ( race_table[race].pc_race )
    //      addaff(af,CHAR,size,ASSIGN,pc_race_table[race].size);
    addaff(af,CHAR,size,ASSIGN,race_table[race].size);
    affect_to_char(victim,&af);
  }
}

/*
 * return 'victim->name' if there is only one char with that name
 * return 2.'victim->name'
 *        3.
 *        ...     if there is more than one char with that name
 */
void chardata_to_str( CHAR_DATA *ch, CHAR_DATA *victim, 
		       char *strbuf, CHAR_DATA *&victim2 ) {
  char buf[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  char name2[MAX_STRING_LENGTH];
  int count;
  bool found;

  strcpy( buf, victim->name );
  one_argument( buf, name );

  found = FALSE;
  count = 1;
  while (TRUE) {
    sprintf( name2, "%d.%s", count, name );
    victim2 = get_char_room( ch, name2 );
    if ( victim == victim2 ) {
      found = TRUE;
      break;
    }
    if ( victim2 == NULL )
      break;
    count++;
  }

  if ( found ) {
    //log_stringf("********* %s *********",
    //NAME(victim2));
    strcpy( strbuf, name2 );
    return;
  }
  strbuf[0] = '\0';
}

// When a player die, we check if player's alignment is still accepted by god
void check_rebirth( CHAR_DATA *ch ) {
  if ( IS_NPC(ch) )
    return;

  god_data *god = &(gods_table[ch->pcdata->god]);

  bool ok = FALSE;
  if ( race_table[ch->cstat(race)].pc_race // can't rebirth a second time
       && pc_race_table[ch->cstat(race)].type == RACE_REBIRTH )
    ok = TRUE;
  for ( int i = 0; i < god->nb_allowed_align && ok == FALSE; i++ ){
    // Modified by SinaC 2001 etho/alignment are attributes now
    //bool etho = god->allowed_align[i].etho == ch->align.etho;
    bool etho = god->allowed_align[i].etho == ch->cstat(etho);
    bool align = FALSE;
    if ( god->allowed_align[i].alignment <= -350 && IS_EVIL( ch ) )    align = TRUE;
    if ( god->allowed_align[i].alignment ==    0 && IS_NEUTRAL( ch ) ) align = TRUE;
    if ( god->allowed_align[i].alignment >=  350 && IS_GOOD( ch ) )    align = TRUE;
    if ( etho && align ) ok = TRUE;
  }
  if ( !ok && ch->level < LEVEL_IMMORTAL && ch->level >= 15 ) {
    send_to_charf( ch, 
		   "%s tells you '{RYou have betrayed me. Be banished !{x'\n\r", 
		   god_name(ch->pcdata->god) );
    ch->desc->connected = CON_REBIRTH;
  }
  else
    send_to_charf(ch,"You are resurrected by {W%s{x\n\r", god_name( ch->pcdata->god ) );
}

// Added by SinaC 2001
CHAR_DATA* find_mob( MOB_INDEX_DATA *mob ) {
  for ( CHAR_DATA *ch = char_list; ch != NULL; ch = ch->next ) {
    if ( !IS_NPC(ch) )
      continue;
    if ( ch->pIndexData == mob )
      return ch;
  }

  return NULL;
}

ROOM_INDEX_DATA *find_location( CHAR_DATA *ch, const char *arg ) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  if ( is_number(arg) )
    return get_room_index( atoi( arg ) );

  if ( ( victim = get_char_world( ch, arg ) ) != NULL )
    return victim->in_room;

  if ( ( obj = get_obj_world( ch, arg ) ) != NULL )
    return obj->in_room;

  return NULL;
}

bool is_wearable( OBJ_INDEX_DATA *obj, int wear_loc ) {

  if ( wear_loc == WEAR_LIGHT && !CAN_WEAR(obj, ITEM_LIGHT ) )          return FALSE;
  if ( wear_loc == WEAR_FINGER_L && !CAN_WEAR(obj, ITEM_WEAR_FINGER ) ) return FALSE;
  if ( wear_loc == WEAR_FINGER_R && !CAN_WEAR(obj, ITEM_WEAR_FINGER ) ) return FALSE;
  if ( wear_loc == WEAR_NECK_1 && !CAN_WEAR(obj, ITEM_WEAR_NECK ) )     return FALSE;
  if ( wear_loc == WEAR_NECK_2 && !CAN_WEAR(obj, ITEM_WEAR_NECK ) )     return FALSE;
  if ( wear_loc == WEAR_BODY && !CAN_WEAR(obj, ITEM_WEAR_BODY ) )       return FALSE;
  if ( wear_loc == WEAR_HEAD && !CAN_WEAR(obj, ITEM_WEAR_HEAD ) )       return FALSE;
  if ( wear_loc == WEAR_LEGS && !CAN_WEAR(obj, ITEM_WEAR_LEGS ) )       return FALSE;
  if ( wear_loc == WEAR_FEET && !CAN_WEAR(obj, ITEM_WEAR_FEET ) )       return FALSE;
  if ( wear_loc == WEAR_HANDS && !CAN_WEAR(obj, ITEM_WEAR_HANDS ) )     return FALSE;
  if ( wear_loc == WEAR_ARMS && !CAN_WEAR(obj, ITEM_WEAR_ARMS ) )       return FALSE;
  if ( wear_loc == WEAR_BODY && !CAN_WEAR(obj, ITEM_WEAR_BODY ) )       return FALSE;
  if ( wear_loc == WEAR_SHIELD && !CAN_WEAR(obj, ITEM_WEAR_SHIELD ) )   return FALSE;
  if ( wear_loc == WEAR_ABOUT && !CAN_WEAR(obj, ITEM_WEAR_ABOUT ) )     return FALSE;
  if ( wear_loc == WEAR_WAIST && !CAN_WEAR(obj, ITEM_WEAR_WAIST ) )     return FALSE;
  if ( wear_loc == WEAR_WRIST_L && !CAN_WEAR(obj, ITEM_WEAR_WRIST ) )   return FALSE;
  if ( wear_loc == WEAR_WRIST_R && !CAN_WEAR(obj, ITEM_WEAR_WRIST ) )   return FALSE;
  if ( wear_loc == WEAR_WIELD && !CAN_WEAR(obj, ITEM_WIELD ) )          return FALSE;
  if ( wear_loc == WEAR_HOLD && !CAN_WEAR(obj, ITEM_HOLD ) )            return FALSE;
  if ( wear_loc == WEAR_EAR_L && !CAN_WEAR(obj, ITEM_WEAR_EAR ) )       return FALSE;
  if ( wear_loc == WEAR_EAR_R && !CAN_WEAR(obj, ITEM_WEAR_EAR ) )       return FALSE;
  if ( wear_loc == WEAR_EYES && !CAN_WEAR(obj, ITEM_WEAR_EYES ) )       return FALSE;
  if ( wear_loc == WEAR_FLOAT && !CAN_WEAR(obj, ITEM_WEAR_FLOAT ) )     return FALSE;
  if ( wear_loc == WEAR_SECONDARY && !CAN_WEAR(obj, ITEM_WIELD ) )      return FALSE;
  // Added by SinaC 2001
  //if ( wear_loc == WEAR_BRAND && !CAN_WEAR(obj, ITEM_BRAND ) )     return FALSE;   removed by SinaC 2003
  if ( wear_loc == WEAR_THIRDLY && !CAN_WEAR(obj, ITEM_WIELD ) )        return FALSE;
  if ( wear_loc == WEAR_FOURTHLY && !CAN_WEAR(obj, ITEM_WIELD ) )       return FALSE;


  return TRUE;
}

int find_suitable_wearloc( OBJ_DATA *obj ) {

  if ( CAN_WEAR(obj, ITEM_WEAR_FINGER ) )     return WEAR_FINGER_R; // why not _L ?
  if ( CAN_WEAR(obj, ITEM_WEAR_NECK ) )       return WEAR_NECK_2;   // why not _1 ?
  if ( CAN_WEAR(obj, ITEM_WEAR_BODY ) )       return WEAR_BODY;
  if ( CAN_WEAR(obj, ITEM_WEAR_HEAD ) )       return WEAR_HEAD;
  if ( CAN_WEAR(obj, ITEM_WEAR_LEGS ) )       return WEAR_LEGS;
  if ( CAN_WEAR(obj, ITEM_WEAR_FEET ) )       return WEAR_FEET;
  if ( CAN_WEAR(obj, ITEM_WEAR_HANDS ) )      return WEAR_HANDS;
  if ( CAN_WEAR(obj, ITEM_WEAR_ARMS ) )       return WEAR_ARMS;
  if ( CAN_WEAR(obj, ITEM_WEAR_BODY ) )       return WEAR_BODY;
  if ( CAN_WEAR(obj, ITEM_WEAR_SHIELD ) )     return WEAR_SHIELD;
  if ( CAN_WEAR(obj, ITEM_WEAR_ABOUT ) )      return WEAR_ABOUT;
  if ( CAN_WEAR(obj, ITEM_WEAR_WAIST ) )      return WEAR_WAIST;
  if ( CAN_WEAR(obj, ITEM_WEAR_WRIST ) )      return WEAR_WRIST_R;  // why not _L ?
  if ( CAN_WEAR(obj, ITEM_WIELD ) )           return WEAR_WIELD;
  if ( CAN_WEAR(obj, ITEM_HOLD ) )            return WEAR_HOLD;
  if ( CAN_WEAR(obj, ITEM_WEAR_EAR ) )        return WEAR_EAR_R;    // why not _L ?
  if ( CAN_WEAR(obj, ITEM_WEAR_EYES ) )       return WEAR_EYES;
  if ( CAN_WEAR(obj, ITEM_WEAR_FLOAT ) )      return WEAR_FLOAT;
  // Added by SinaC 2001
  //if ( CAN_WEAR(obj, ITEM_BRAND ) )           return WEAR_BRAND;   removed by SinaC 2003

  if ( CAN_WEAR(obj, ITEM_TAKE ) )            return WEAR_NONE;

  return -99; // can't be worn
}

// Added by SinaC 2003
bool is_ability_sphere( int sn, const char *sphere_name ) {
  //int sphereId = sphere_lookup( sphere_name );
  //if ( ability_table[sn].sphere == sphereId )  // maybe .sphere should be a vector
  //  return TRUE;
  //return FALSE;
  return FALSE;
}

int is_ability_in_sphere( const int sn ) {
  for ( int i = 0; i < MAX_GROUP; i++ )
    if ( group_table[i].isSphere )
      for ( int j = 0; j < group_table[i].spellnb; j++ )
	if ( !str_cmp( group_table[i].spells[j], ability_table[sn].name ) )
	  return i;
  return -1;
}

// modify obj's size by increasing/decreasing 'amount' size (amount can be negative to reduce item)
void change_size_obj( OBJ_DATA *obj, int amount ) {
  if ( obj->size == SIZE_NOSIZE ) // if item has no size, starts with medium size
    obj->size = SIZE_MEDIUM;
  obj->size = URANGE( SIZE_TINY, obj->size+amount, SIZE_GIANT );
}

// Ch knocks items off victim's hand/equipement
void knock_off_item( CHAR_DATA *ch, CHAR_DATA *victim, bool fromEquip ) {
  OBJ_DATA *obj, *obj_drop;
  int carry;
  int drop, count, check;
  bool dropped = FALSE;

  carry = 0;
  for ( obj = victim->carrying; obj; obj = obj->next_content )
    carry++;
  
  drop = carry - can_carry_n( victim ) + 5;
  
  for ( check = 0; check < drop; check++ ){
    obj_drop = NULL;
    count = 0;
    
    for ( obj = victim->carrying; obj; obj = obj->next_content ){
      if ( ( obj->wear_loc == WEAR_NONE
	     || fromEquip )
	   && number_range( 0, count++ ) == 0 )
	obj_drop = obj;
    }
    
    if ( !obj_drop )
      break;

    // obj_drop is the item to drop
    // item in equipement, unequip the item
    if ( obj->wear_loc != WEAR_NONE ) {
      if ( IS_OBJ_STAT( obj_drop, ITEM_NOREMOVE ) ) // no remove?
	continue;
      Value args[] = {ch}; // script no remove ?
      bool result = objprog( obj, ch, "onRemoving", args);
      if ( result )
	continue;
      // okay, item can be removed
      unequip_char( victim, obj_drop );
    }
    // drop the item
    if ( can_drop_obj( victim, obj_drop ) ) { // can be dropped?
      Value args[] = {ch}; // script no drop?
      bool result = objprog( obj_drop, ch, "onDropping", args);
      if ( !result ) {
	obj_from_char( obj_drop );
	obj_to_room( obj_drop, victim->in_room );
	OBJPROG(obj_drop,victim,"onDropped",ch); // script dropped?
	dropped = TRUE;
      }
    }
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "$n knocks $p off your %s!", dropped?"inventory":"equipement");
    act( buf, ch, obj_drop, victim, TO_VICT );
    sprintf( buf, "$n knocks $p off $N's %s!", dropped?"inventory":"equipement");
    act( buf, ch, obj_drop, victim, TO_NOTVICT );
    sprintf( buf, "You knock $p off $N's %s!", dropped?"inventory":"equipement");
    act( buf, ch, obj_drop, victim, TO_CHAR );
    dropped = FALSE;
  }
}

// Added by SinaC 2003
bool can_remove_obj( CHAR_DATA *ch, OBJ_DATA *obj ) {
  if ( obj->carried_by != ch )
    return FALSE;

  if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) 
       && !IS_IMMORTAL(ch)) {
    act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
    return FALSE;
  }

  // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj,ch, "onRemoving", args);
  if ( result ) // if onRemoving returns 1,  we don't remove the item
    return FALSE;
  
  return TRUE;
}

// Check if a player is affected by phylactery, if yes, we remove affect and give him/her
//  some hp back (stored in affect)
// Called when a player would die.
bool check_phylactery( CHAR_DATA *ch ) {
  AFFECT_DATA *af = affect_find( ch->affected, gsn_phylactery );

  if ( af == NULL ) // not affected by phylactery
    return FALSE;

  if ( chance(20) ) { // 20% chance to fail
    send_to_char("The {rCrimson Aura{x around you shimmers but nothing seems to happen.\n\r", ch );
    act("The {rCrimson Aura{x around $n shimmers but nothing seems to happen.", ch, NULL, NULL, TO_ROOM );
    affect_remove( ch, af );
    return FALSE;
  }

  send_to_char("{WThe {rCrimson Aura {Waround you shimmers.{x\n\r", ch );
  send_to_char("{WYou get some power back from your Phylactery.\n\r", ch );
  act("{WThe {rCrimson Aura {Waround $n shimmers.{x", ch, NULL, NULL, TO_ROOM );
  if ( af->list == NULL ) {
    bug("check_phylactery: af->list is NULL on [%s]", NAME(ch));
    ch->hit = ch->bstat(max_hit);
  }
  else
    ch->hit = af->list->modifier; // get first affect_list modifier
  affect_remove( ch, af );

  return TRUE;
}

// Utility functions for MOUNT code
// Get char's mount
CHAR_DATA *get_mount( CHAR_DATA *ch ) {
  CHAR_DATA *pet;
  if ( ( pet = ch->pet ) != NULL
       && pet->leader == ch
       && IS_SET( pet->act, ACT_IS_MOUNTED ) )
    return pet;
  return NULL;
}
// Get mount's master
CHAR_DATA *get_mount_master( CHAR_DATA *mount ) {
  if ( !IS_NPC(mount ) )
    return NULL;
  CHAR_DATA *leader = mount->leader;
  if ( IS_SET( mount->act, ACT_IS_MOUNTED ) 
       && leader != NULL
       && leader->pet == mount )
    return leader;
  return NULL;
}
// When a bonded mount dies, it's master is drained really badly
void mount_dying_drawback( CHAR_DATA *mount ) {
  if ( !IS_NPC(mount ) )
    return;
  CHAR_DATA *leader = get_mount_master( mount );
  if ( leader != NULL
       && is_affected( mount, gsn_bond ) ) {
    act("Your beloved mount '$N' has just died, you feel an intense pain.",
	leader, NULL, mount, TO_CHAR );
    act("Your beloved master '$n' feels an intense pain because of your death.", 
	leader, NULL, mount, TO_VICT );
    leader->psp = leader->mana = 0; leader->move = 0; leader->hit = 1;
    affect_strip( leader, gsn_bond );
    affect_strip( mount, gsn_bond );
  }
}
// Mount gets back to its master
void mount_to_master( CHAR_DATA *mount, CHAR_DATA *master, bool silent ) {
  if ( master->in_room == NULL ) {
    bug("mount_to_master: master %s in_room is NULL", NAME(master));
    return;
  }
  char_from_room(mount);
  char_to_room(mount,master->in_room);
  if (!silent) {
    act("$n has arrived.", mount, NULL, master, TO_NOTVICT );
    send_to_char("Your mount found you back.\n\r", master );
    send_to_char("You found back your beloved master.\n\r", mount );
  }
}

// Added by SinaC 2003 for strategic retreat affect:
//  When a player affected will take lethal damage, it teleports the player to
//  his hometown recall
bool check_strategic_retreat( CHAR_DATA *ch ) {
  if ( !IS_NPC(ch)
       && is_affected( ch, gsn_strategic_retreat )
       && chance(90) ) {
    ROOM_INDEX_DATA *location;
    ch->hit = 1;
    stop_fighting( ch, TRUE );
    //int dest_vnum = hometown_table[ch->pcdata->hometown].recall;
    //location = get_room_index( dest_vnum );
    location = get_recall_room( ch );
    //    if ( location == NULL )
    //      location = get_room_index(ROOM_VNUM_TEMPLE);
    affect_strip(ch,gsn_strategic_retreat);
    send_to_char("{WYour strategic retreat gives you the opportunity to avoid death.{x\n\r", ch );
    act("{W$n avoids death thanks to $s strategic retreat.{x", ch, NULL, NULL, TO_ROOM );
    if ( ch->in_room == location ) {
      send_to_char("You already are in your hometown recall.\n\r", ch );
      return TRUE;
    }
    else {
      char_from_room( ch );
      char_to_room( ch, location );
      act("$T transfers you to your hometown recall.", 
	  ch, NULL, god_name(ch->pcdata->god), TO_CHAR );
      act("$T has just saved $n from death.", 
	  ch, NULL, god_name(ch->pcdata->god), TO_ROOM );
      do_look( ch, "auto" );
      return TRUE;
    }
  }
  return FALSE;
}

OBJ_DATA *get_item_here( CHAR_DATA *ch, const int item_type ) {
  OBJ_DATA *obj, *obj_next = NULL;
  for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;
    if ( can_see_obj( ch, obj ) && 
	 obj->pIndexData != NULL && obj->pIndexData->item_type == item_type )
      return obj;
  }
  for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;
    if ( can_see_obj( ch, obj ) && 
	 obj->pIndexData != NULL && obj->pIndexData->item_type == item_type )
      return obj;
  }
  return NULL;
}

int find_exit( const char *arg ) {
  if ( !str_cmp( arg, "n" ) || !str_cmp( arg, "north" ) ) return DIR_NORTH;
  else if ( !str_cmp( arg, "e" ) || !str_cmp( arg, "east"  ) ) return DIR_EAST;
  else if ( !str_cmp( arg, "s" ) || !str_cmp( arg, "south" ) ) return DIR_SOUTH;
  else if ( !str_cmp( arg, "w" ) || !str_cmp( arg, "west"  ) ) return DIR_WEST;
  else if ( !str_cmp( arg, "u" ) || !str_cmp( arg, "up"    ) ) return DIR_UP;
  else if ( !str_cmp( arg, "d" ) || !str_cmp( arg, "down"  ) ) return DIR_DOWN;
  // Added by SinaC 2003
  else if ( !str_cmp( arg, "ne" ) || !str_cmp( arg, "northeast"  ) ) return DIR_NORTHEAST;
  else if ( !str_cmp( arg, "nw" ) || !str_cmp( arg, "northwest"  ) ) return DIR_NORTHWEST;
  else if ( !str_cmp( arg, "se" ) || !str_cmp( arg, "southeast"  ) ) return DIR_SOUTHEAST;
  else if ( !str_cmp( arg, "sw" ) || !str_cmp( arg, "southwest"  ) ) return DIR_SOUTHWEST;
  return -1;
}

// If item is carried, return carrier's room
// If item on the ground, return room
ROOM_INDEX_DATA *get_room_obj( OBJ_DATA *obj ) {
  if ( obj->in_room != NULL ) // on the ground ?
    return obj->in_room;
  else 
    if ( obj->carried_by != NULL ) // a carrier ?
      return obj->carried_by->in_room;
    else // not in a room, neither on a carrier -> in a container or loaded but not put somewhere
      return NULL;
  return NULL;
}


int convert_etho_align_string_to_align_info( const char *input, align_info &align ) {
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  
  if (input[0] == '\0')
    return -1;

  strcpy( buf, input );
  char *s;

  int nwords = 0;
  //count words
  for (s=buf+1; *s; s++) {
    if (*s == ' ' && *(s-1) != ' ' )
      nwords++;
  }

  if (*(s-1) != ' ' )
    nwords++;

  if (nwords != 2 )
    return -1;

  s = buf;

  char* tok;
  tok = strsep(&s, " ");
  strcpy( buf1, tok );
  tok = strsep(&s, " ");
  strcpy( buf2, tok );

  if ( !str_cmp( buf1, "chaotic" ) ) 
    align.etho = -1;
  else if ( !str_cmp( buf1, "neutral" ) || !str_cmp( buf, "true" ) ) 
    align.etho =  0;
  else if ( !str_cmp( buf1, "lawful" ) )  
    align.etho =  1;
  else return -1;
  
  if ( !str_cmp( buf2, "evil" ) )    
    align.alignment = -350;
  else if ( !str_cmp( buf2, "neutral" ) ) 
    align.alignment =    0;
  else if ( !str_cmp( buf2, "good" ) )    
    align.alignment =  350;
  else return -1;

  return 0;
}


// Return ch's god
const char *char_god_name( CHAR_DATA *ch ) {
  return god_name( IS_NPC(ch) ? DEFAULT_GOD : ch->pcdata->god );
}


// Removed by SinaC 2003, replaced with OPTIMIZING_BIT_UNARMED_UNARMORED set in recompute
//// Useful for monk
//// Can't wear armor, shield, can't hold an item and can't wield a weapon
//bool is_unarmed_and_unarmored( CHAR_DATA *ch ) {
//  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
//    if ( obj->wear_loc == WEAR_HOLD                  // no hold
//	 || obj->wear_loc == WEAR_WIELD              // no wield
//	 || obj->wear_loc == WEAR_SECONDARY          // no secondary
//	 || obj->wear_loc == WEAR_THIRDLY            // no thirdly
//	 || obj->wear_loc == WEAR_FOURTHLY           // no fourthly
//	 || obj->wear_loc == WEAR_SHIELD             // no shield
//	 || ( obj->wear_loc != WEAR_NONE             // no armor
//	      && obj->item_type == ITEM_ARMOR ) )
//      return FALSE;
//  }
//  return TRUE;
//}

// Check if a char is skilled in 'pure body'
//  TO DO: add a test in every disease skill/spell/...
//'PURE BODY'
//A character with the pure body skill gains the natural ability to prevent
//diseases from affecting him or her.  Whenever the character would otherwise
//contract a disease, the pure body skill is automatically checked, and if
//successful, the disease is prevented.  Pure body is also effective against
//spells that drain one's strength, such as weaken, energy, and deplete
//strength.  Pure body will not prevent lycanthropy.
bool is_pure_body_skilled( const int level, CHAR_DATA *ch ) {
  int chance = get_ability( ch, gsn_pure_body );
  if ( chance == 0 )
    return FALSE;
  chance += ch->level - level;
  if ( number_percent() > chance ) { // failed
    check_improve( ch, gsn_pure_body, FALSE, 5 );
    return FALSE;
  }
  else {
    check_improve( ch, gsn_pure_body, TRUE, 5 );
    return TRUE;
  }
  return FALSE;
}


// Check if a char is skilled in 'diamond body'
//  TO DO: add a test in every poison skill/spell/...
//'DIAMOND BODY'
//A character with the diamond body skill gains the natural ability to
//prevent poisons from entering his or her bloodstream and causing damage or
//even death.  Whenever a character would otherwise be envenomed by a poison,
//the diamond body skill is automatically checked, and if successful, the
//poison is prevented.  Diamond body is more effective against lower-grade
//poisons, but does have a chance to work on even the most powerful of
//poisons.  Diamond body is also effective in downgrading the effect of
//certain ice/cold based spells such as chill touch.
bool is_diamond_body_skilled( const int level, CHAR_DATA *ch ) {
  int chance = get_ability( ch, gsn_diamond_body );
  if ( chance == 0 )
    return FALSE;
  if ( number_percent() > chance ) { // failed
    check_improve( ch, gsn_diamond_body, FALSE, 5 );
    return FALSE;
  }
  else {
    check_improve( ch, gsn_diamond_body, TRUE, 5 );
    return TRUE;
  }
  return FALSE;
}

// SinaC 2003 for hometown creation choice
bool check_hometown( CHAR_DATA *ch, int hometown ) {
  // Maybe a formula using race/class/alignment/...
  return hometown_table[hometown].choosable;
}

// SinaC 2003
void check_mount_class( CHAR_DATA *pet ) {
  CLASS_DATA *cl = silent_hash_get_prog( "mount" );
  if ( cl == NULL ) {
    bug("check_mount_class: class Mount cannot be found.");
    return;
  }
  if ( !findAscendancy( pet, "mount" ) )
    if ( pet->clazz == NULL || pet->clazz == default_mob_class ) {
      bug("Mount [%s | %d]'s clazz is not set as Mount (or Mount's child) -> fixing", NAME(pet), pet->pIndexData->vnum );
      pet->clazz = cl;
    }
    else {
      bug("Mount [%s | %d]'s clazz is not Mount (or Mount's child) but [%s]", NAME(pet), pet->pIndexData->vnum, pet->clazz->name );
    }
}

// SinaC 2003
// return true if 'wear_loc' need parts not found in 'parts'
bool check_needed_parts( const int parts, const int wear_loc ) {
  return ( wear_needed_parts_form[wear_loc][0]
	   && !IS_SET( parts, wear_needed_parts_form[wear_loc][0] ) );
}
// return true if 'wear_loc' need forms not found in 'forms'
bool check_needed_forms( const int forms, const int wear_loc ) {
  return ( wear_needed_parts_form[wear_loc][1]
	   && !IS_SET( forms, wear_needed_parts_form[wear_loc][1] ) );
}
const char *needed_parts_string( const int wear_loc ) {
  return list_flag_string(wear_needed_parts_form[wear_loc][0], part_flags,""," and ");
}
const char *needed_forms_string( const int wear_loc ) {
  return list_flag_string(wear_needed_parts_form[wear_loc][1], form_flags,""," and ");
}
