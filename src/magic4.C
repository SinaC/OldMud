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
#include "handler.h"
#include "spells_def.h"
#include "act_obj.h"
#include "update.h"
#include "mem.h"
#include "utils.h"
#include "damage.h"


void spell_acid_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice( level, 12 );

  // added by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim, DAM_ACID ) && casting_level < 3 )
    dam /= 2;

  bool done = FALSE;
  if ( casting_level == 5 )
    for ( int i = 0; i < number_fuzzy(3); i++ ) {
      int damdone = ability_damage( ch, victim, dam/3, sn, DAM_ACID, TRUE, TRUE);
      if ( damdone == DAMAGE_DEADLY )
	return;
      done |= damdone;
    }
  else {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_ACID, TRUE, TRUE);
    if ( damdone == DAMAGE_DEADLY )
      return;
    done |= damdone;
  }
  if( done && casting_level >= 4 )
    acid_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already armored.\n\r",ch);
    else
      act("$N is already armored.",ch,NULL,victim,TO_CHAR);
    return;
  }

  // Added by SinaC 2000 to test spell/skill level
  //newafsetup(af,CHAR,allAC,ADD,-20*casting_level,level+5*casting_level,level,sn, casting_level );
  //if ( casting_level == 5 )
  //  af.modifier *= 2;
  createaff(af,level+5*casting_level,level,sn,casting_level,AFFECT_ABILITY);
  int mod = -20*casting_level;
  if ( casting_level == 5 )
    mod *= 2;
  addaff(af,CHAR,allAC,ADD,mod);

  affect_to_char( victim, &af );

  send_to_char( "You feel someone protecting you.\n\r", victim );
  if ( ch != victim )
    act("$N is protected by your magic.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_burning_hands(int sn,int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    5,  6,  9,  11,	14,	17, 20, 23, 26, 29,
    29, 29, 30, 30,	31,	31, 32, 32, 33, 33,
    34, 34, 35, 35,	36,	36, 37, 37, 38, 38,
    39, 39, 40, 40,	41,	41, 42, 42, 43, 43,
    44, 44, 45, 45,	46,	46, 47, 47, 48, 48,
    
    49, 50, 51, 52, 53,     54, 54, 55, 57, 59,
    60, 61, 62, 62, 65,     65, 67, 67, 68, 69,
    70, 72, 74, 75, 76,     78, 79, 80, 81, 82,
    83, 84, 85, 87, 87,     89, 90, 92, 93, 94,
    95, 95, 96, 96, 97,     97, 98, 98, 99, 99
  };
  int dam;

  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  //dam		= UMAX( number_range( dam_each[level] / 2, dam_each[level] * 2 ), 2 );
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 4 ), 2 );
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim, DAM_FIRE ) && casting_level < 3 )
    dam /= 2;

  int done = ability_damage( ch, victim, dam, sn, DAM_FIRE,TRUE, TRUE);
  if ( done == DAMAGE_DONE && casting_level == 5 )
    fire_effect( (void*) victim, level, dam/2, TARGET_CHAR );
  return;
}


void dam_increment_chain_lightning( int level, CHAR_DATA *victim, int casting_level, 
				    int &decrement, int &dam ) {
  dam = dice(level,6);
  switch( casting_level ) {
  default: case 0: case 1: decrement = 5; dam = dice(level,6); break; // 4
  case 2: decrement = 4; dam = dice(level,6); break; // 3
  case 3: decrement = 4; dam = dice(level,7); break; // 3
  case 4: decrement = 3; dam = dice(level,7); break; // 2
  case 5: decrement = 2; dam = dice(level,9); break; // 1
  }
  if (saves_spell(level,victim,DAM_LIGHTNING))
    dam /= 3;
}
void spell_chain_lightning(int sn,int level,CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  CHAR_DATA *tmp_vict,*last_vict,*next_vict;
  bool found;
  // Added by SinaC 2000
  char buf[MAX_STRING_LENGTH];
  static char * const him_her_self[] = { "itself", "himself", "herself" };
  int decrement;
  int dam;

  /* first strike */

  act("A lightning bolt leaps from $n's hand and arcs to $N.",
      ch,NULL,victim,TO_ROOM);
  act("A lightning bolt leaps from your hand and arcs to $N.",
      ch,NULL,victim,TO_CHAR);
  act("A lightning bolt leaps from $n's hand and hits you!",
      ch,NULL,victim,TO_VICT);  

  dam_increment_chain_lightning( level, victim, casting_level, decrement, dam );
  
  ability_damage(ch,victim,dam,sn,DAM_LIGHTNING,TRUE, TRUE);
  last_vict = victim;
  level -= decrement;   /* decrement damage */

  /* new targets */
  while (level > 0) {
    if ( ch == NULL || ch->in_room == NULL || !ch->valid ) // Modified by SinaC 2001
      break;
    found = FALSE;
    for (tmp_vict = ch->in_room->people; 
	 tmp_vict != NULL; 
	 tmp_vict = next_vict) {
      next_vict = tmp_vict->next_in_room;
      if ( !is_safe_spell(ch,tmp_vict,TRUE) 
	   && tmp_vict != last_vict) {
	found = TRUE;
	last_vict = tmp_vict;
	act("The bolt arcs to $n!",tmp_vict,NULL,NULL,TO_ROOM);
	act("The bolt hits you!",tmp_vict,NULL,NULL,TO_CHAR);
	dam_increment_chain_lightning( level, tmp_vict, casting_level, decrement, dam );
	ability_damage(ch,tmp_vict,dam,sn,DAM_LIGHTNING,TRUE, TRUE);
	level -= decrement;  /* decrement damage */
      }
    }   /* end target searching loop */
	
    if (!found ) {/* no target found, hit the caster */
      if (ch == NULL || !ch->valid )
	return;

      if (last_vict == ch ) {/* no double hits */
	act("The bolt seems to have fizzled out.",ch,NULL,NULL,TO_ROOM);
	act("The bolt grounds out through your body.",
	    ch,NULL,NULL,TO_CHAR);
	return;
      }

      last_vict = ch;
      // Modified by SinaC 2000
      /*
       *act("The bolt arcs to $n...whoops!",ch,NULL,NULL,TO_ROOM);
       *send_to_char("You are struck by your own lightning!\n\r",ch);
       *ability_damage(ch,ch,dam,sn,DAM_LIGHTNING,TRUE, TRUE);
       */
      dam_increment_chain_lightning( level, ch, casting_level, decrement, dam );
      if ( casting_level < 5 ) { // casting 5: caster doesn't take damage
	// Modified by SinaC 2000
	sprintf( buf, "kills %s with a lightning!", 
		 him_her_self[URANGE(0,ch->cstat(sex),2)] );
	noaggr_damage( ch, dam, DAM_LIGHTNING,
		       "You are struck by your own lightning!",
		       "The bolt arcs to $n...whoops!",
		       buf,
		       TRUE );
      }

      level -= decrement;  /* decrement damage */
    }
    /* now go back and find more targets */
  }
}

void spell_chill_touch( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    2,  4,  6,  7,  8,	 9, 12, 13, 13, 13,
    14, 14, 14, 15, 15,	15, 16, 16, 16, 17,
    17, 17, 18, 18, 18,	19, 19, 19, 20, 20,
    20, 21, 21, 21, 22,	22, 22, 23, 23, 23,
    24, 24, 24, 25, 25,	25, 26, 26, 26, 27,
    
    29, 29, 32, 32, 32,     33, 34, 34, 34, 35,
    35, 36, 36, 37, 38,     38, 38, 39, 40, 40,
    41, 41, 42, 42, 43,     44, 45, 46, 46, 46, 
    47, 47, 48, 49, 50,     51, 51, 52, 53, 53,
    54, 54, 55, 56, 56,     57, 58, 58, 59, 60
  };
  AFFECT_DATA af;
  int dam;


  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  // Modified by SinaC 2001
  dam		= UMAX( number_range( dam_each[level] *2, dam_each[level] *6 ), 2 );

  // added by SinaC 2001 for spell level
  //dam          *= casting_level *4;
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( !saves_spell( level, victim,DAM_COLD ) 
       || ( casting_level == 3
	    && chance(50) )
       || casting_level >= 4 ) {
    act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
    // lose more STR when higher level
    //newafsetup(af,CHAR,STR,ADD,-1*casting_level,6,level,sn,casting_level);
    createaff(af,6,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,STR,ADD,-1*casting_level);

    affect_join( victim, &af );
    if ( ( casting_level == 4 && chance(33))
	 || casting_level == 5 )
      cold_effect( (void *)victim, level, dam/2, TARGET_CHAR );
  }
  else
    dam /= 2;

  ability_damage( ch, victim, dam, sn, DAM_COLD,TRUE, TRUE );
  return;
}

void spell_colour_spray( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    5,  6,  7,  10,  12,	15, 19, 21, 24, 27,
    30, 35, 40, 45, 50,	55, 55, 55, 56, 57,
    58, 58, 59, 60, 61,	61, 62, 63, 64, 64,
    65, 66, 67, 67, 68,	69, 70, 70, 71, 72,
    73, 73, 74, 75, 76,	76, 77, 78, 79, 79,
    
    80, 81, 82, 83, 84,     85, 86, 87, 88, 89,
    89, 90, 91, 93, 93,     95, 95, 97, 97, 99, 
    100,100,100,102,102,    105,105,107,108,109,
    110,111,112,115,117,    118,120,121,123,125,
    129,130,131,132,133,    135,138,140,145,150
  };
  int dam;

  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  // Modified by SinaC 2001, was /2, *2  instead of ., *4
  dam		= UMAX( number_range( dam_each[level],  dam_each[level] * 4 ), 2 );

  // added by SinaC 2001 for spell level
  //dam          *= casting_level*2;
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim,DAM_LIGHT) && casting_level < 3 )
    dam /= 2;

  int done = ability_damage( ch, victim, dam, sn, DAM_LIGHT,TRUE, TRUE );
  if ( done == DAMAGE_DONE && casting_level >= 3 ) {
    spell_blindness(gsn_blindness,
		    level/2,ch,(void *) victim,TARGET_CHAR, 1);
    if ( casting_level == 5 ) {
      DAZE_STATE( victim, number_fuzzy(2)*PULSE_VIOLENCE );
      if( !is_affected( victim, sn ) ) {
	AFFECT_DATA af;
	//newafsetup(af,CHAR,allAC,ADD,number_fuzzy(3)*casting_level,6+level,level,sn,casting_level);
	createaff(af,6+level,level,sn,casting_level,AFFECT_ABILITY);
	addaff(af,CHAR,allAC,ADD,number_fuzzy(3)*casting_level);

	affect_to_char( victim, &af );
      }
    }
  }
  return;
}

void spell_earthquake( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  // Modified by SinaC 2001  
  if ( ch->in_room->cstat(sector) == SECT_AIR
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char( "There is no earth around!\n\r", ch );
    return;
  }
  
  send_to_char( "The earth trembles beneath your feet!\n\r", ch );
  act( "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM );
  
  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room ) {
      int dam = level*casting_level + dice(2, 8);
      if ( saves_spell( level, vch, DAM_BASH ) )
	dam /= 2;
      if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	if (IS_AFFECTED(vch,AFF_FLYING)) {
	  if ( casting_level >= 3 
	       && (is_affected( vch, gsn_levitation )
		   || is_affected( vch, gsn_fly) 
		   || is_affected( vch, gsn_flight)
		   || is_affected( vch, gsn_flight_bird) )
	       && !saves_spell(level,vch,DAM_BASH) ) {
	    act("$n is thrown wildly to the ground by the earthquake!",vch,NULL,NULL,TO_ROOM);
	    send_to_char("You are thrown down by the earthquake!\n\r",vch);
	    affect_strip(vch,gsn_fly);
	    affect_strip(vch,gsn_levitation);
	    affect_strip(vch,gsn_flight);
	    affect_strip(vch,gsn_flight_bird);
	  }
	  ability_damage(ch,vch,0,sn,DAM_BASH,TRUE, TRUE);
	}
	else
	  ability_damage( ch,vch, dam, sn, DAM_BASH,TRUE, TRUE);
      continue;
    }
      
    if ( vch->in_room->area == ch->in_room->area )
      send_to_char( "The earth trembles and shivers.\n\r", vch );
  }
  
  return;
}

/*
 Add AC to an armor
 Bonus increases with casting_level
 Lvl 4 and Lvl 5: gives saving throw bonus instead of AC
                  malus: armor is linked to caster
*/
void spell_enchant_armor( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf; 
  int result, fail;
  int ac_bonus, added, saving_bonus; // saving bonus added by SinaC 2003
  bool ac_found = FALSE, saving_found = FALSE;

  if (obj->item_type != ITEM_ARMOR) {
    send_to_char("That isn't an armor.\n\r",ch);
    return;
  }

  if (obj->wear_loc != -1) {
    send_to_char("The item must be carried to be enchanted.\n\r",ch);
    return;
  }

  // Added by SinaC 2003
  if ( casting_level >= 4 
       && obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp(obj->owner,ch->name) ) {
    send_to_charf(ch, "You're not the owner of this object, %s is the owner. Try a lower casting level.\n\r",
		  obj->owner);
    return;
  }

  // Higher casting level should involve less failure
  int base_fail = 25;	// base 25% chance of failure
  if ( casting_level > 1 ) base_fail = URANGE( 5, base_fail/casting_level, 25 );
  int fact_fail = 5;    // faction factor
  if ( casting_level > 1 ) fact_fail = URANGE( 2, fact_fail/casting_level, 5 );
  int add_fail = 20;    // additional fail if not saves/hitroll/damroll
  if ( casting_level > 1 ) add_fail = URANGE( 5, add_fail/casting_level, 20 );
  int max_fail = 85;
  if ( casting_level > 1 ) max_fail = URANGE( 85, max_fail + casting_level*3, 100 );
  /* this means they have no bonus */
  ac_bonus = 0;
  saving_bonus = 0;
  fail = base_fail;

  /* find the bonuses */
  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
      for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
	if ( laf->location == ATTR_allAC ) {
	  ac_bonus = laf->modifier;
	  ac_found = TRUE;
	  fail += fact_fail * (ac_bonus * ac_bonus);
	}
	else if ( laf->location == ATTR_saving_throw ) { // Added by SinaC 2003
	  saving_bonus = laf->modifier;
	  saving_found = TRUE;
	  fail += fact_fail * (saving_bonus * saving_bonus);
	}
	else  /* things get a little harder */
	  fail += add_fail;
 
  for ( paf = obj->affected; paf != NULL; paf = paf->next )
    for ( AFFECT_LIST *laf = paf->list; laf != NULL; laf = laf->next )
      if ( laf->location == ATTR_allAC ) {
	ac_bonus = laf->modifier;
	ac_found = TRUE;
	fail += fact_fail * (ac_bonus * ac_bonus);
      }
      else if ( laf->location == ATTR_saving_throw ) { // Added by SinaC 2003
	saving_bonus = laf->modifier;
	saving_found = TRUE;
	fail += fact_fail * (saving_bonus * saving_bonus);
      }
      else /* things get a little harder */
	fail += add_fail;

  /* apply other modifiers */
  fail -= level;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,max_fail);

  result = number_percent();

  /* the moment of truth */
  if (result < (fail / 5)) { /* item destroyed */
    act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
    act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    return;
  }

  if (result < (fail / 3)) {/* item disenchanted */
    act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
    act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
    obj->enchanted = TRUE;

    /* remove all affects */
    obj->affected = NULL;

    /* Clear flags */
    REM_OBJ_STAT(obj,ITEM_GLOW|ITEM_HUM|ITEM_MAGIC);
    return;
  }

  if ( result <= fail ) { /* failed, no bad result */
    send_to_char("Nothing seemed to happen.\n\r",ch);
    return;
  }

  affect_enchant(obj);

  if (result <= (90 - level/5)) { /* success! */
    act("$p shimmers with a gold aura.",ch,obj,NULL,TO_CHAR);
    act("$p shimmers with a gold aura.",ch,obj,NULL,TO_ROOM);
    SET_OBJ_STAT(obj, ITEM_MAGIC);
    added = -1;
  }
    
  else { /* exceptional enchant */
    act("$p glows a brillant gold!",ch,obj,NULL,TO_CHAR);
    act("$p glows a brillant gold!",ch,obj,NULL,TO_ROOM);
    SET_OBJ_STAT(obj,ITEM_MAGIC|ITEM_GLOW);
    added = -2;
  }

  // Added by SinaC 2003, higher level gives better result
  if ( casting_level > 2 )
    added -= (casting_level - 2);

  /* now add the enchantments */ 

  if (obj->level < LEVEL_HERO)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

  AFFECT_DATA af;
  createaff(af,-1,level,sn,casting_level,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  if ( casting_level <= 3 )
    addaff(af,CHAR,allAC,ADD,added);
  else
    addaff(af,CHAR,saving_throw,ADD,added);
  affect_join_obj( obj, &af );
  
  if ( casting_level >= 4 ) { // link weapon to the player
    obj->owner = str_dup(ch->name);
  }
}

// Lvl 1: only hitroll
// Lvl 2: hitroll & damroll
// Lvl 3: higher bonus
// Lvl 4: weapon linked to caster (lvl 5 too)
// Lvl 5: convert weapon bonuses (vampiric, ...) to permanent bonus
void spell_enchant_weapon(int sn,int level,CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf; 
  int result, fail;
  int hit_bonus, dam_bonus, added;
  bool hit_found = FALSE, dam_found = FALSE;

  if (obj->item_type != ITEM_WEAPON) {
    send_to_char("That isn't a weapon.\n\r",ch);
    return;
  }

  if (obj->wear_loc != -1) {
    send_to_char("The item must be carried to be enchanted.\n\r",ch);
    return;
  }

  // Added by SinaC 2003
  if ( casting_level >= 4 
       && obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp(obj->owner,ch->name) ) {
    send_to_charf(ch, "You're not the owner of this object, %s is the owner. Try a lower casting level.\n\r",
		  obj->owner);
    return;
  }

 
  int base_fail = 25;	// base 25% chance of failure
  if ( casting_level > 1 ) base_fail = URANGE( 5, base_fail/casting_level, 25 );
  int fact_fail = 2;    // faction factor
  if ( casting_level > 1 ) fact_fail = URANGE( 1, fact_fail/casting_level, 2 );
  int add_fail = 25;    // additional fail if not saves/hitroll/damroll
  if ( casting_level > 1 ) add_fail = URANGE( 5, add_fail/casting_level, 25 );
  int max_fail = 95;
  if ( casting_level > 1 ) max_fail = URANGE( 95, max_fail + casting_level*3, 100 );
  /* this means they have no bonus */
  hit_bonus = 0;
  dam_bonus = 0;
  fail = base_fail;

  /* find the bonuses */
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
	else  /* things get a little harder */
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
      else /* things get a little harder */
	fail += add_fail;

  /* apply other modifiers */
  fail -= 3 * level/2;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,max_fail);

  result = number_percent();

  /* the moment of truth */
  if (result < (fail / 5)) { /* item destroyed */
    act("$p shivers violently and explodes!",ch,obj,NULL,TO_CHAR);
    act("$p shivers violently and explodeds!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    return;
  }

  if (result < (fail / 2)) {/* item disenchanted */
    act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
    act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
    obj->enchanted = TRUE;

    /* remove all affects */
    obj->affected = NULL;

    /* clear all flags */
    REM_OBJ_STAT(obj,ITEM_GLOW|ITEM_HUM|ITEM_MAGIC);
    return;
  }

  if ( result <= fail ) { /* failed, no bad result */
    send_to_char("Nothing seemed to happen.\n\r",ch);
    return;
  }

  affect_enchant(obj);

  if (result <= (100 - level/5)) { /* success! */
    act("$p glows blue.",ch,obj,NULL,TO_CHAR);
    act("$p glows blue.",ch,obj,NULL,TO_ROOM);
    SET_OBJ_STAT(obj, ITEM_MAGIC);
    added = 1;
  }
    
  else { /* exceptional enchant */
    act("$p glows a brillant blue!",ch,obj,NULL,TO_CHAR);
    act("$p glows a brillant blue!",ch,obj,NULL,TO_ROOM);
    SET_OBJ_STAT(obj,ITEM_MAGIC|ITEM_GLOW);
    added = 2;
  }
		
  // Added by SinaC 2003
  if ( casting_level > 2 )
    added += casting_level-2;

  /* now add the enchantments */ 

  if (obj->level < LEVEL_HERO - 1)
    obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

  AFFECT_DATA af;
  createaff(af,-1,level,sn,casting_level,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,added);
  addaff(af,CHAR,damroll,ADD,added);
  affect_join_obj( obj, &af );

  // Added by SinaC 2003, additional effect if casting_level > 1
  if ( casting_level >= 4 ) { // link weapon to the player
    obj->owner = str_dup(ch->name);
  }
  if ( casting_level == 5 ) { // transfer temporary effect into permanent effect
    for ( AFFECT_DATA *paf = obj->affected; paf != NULL; paf = paf->next )
      if ( paf->type == gsn_shocking_blade
	   || paf->type == gsn_frost_blade
	   || paf->type == gsn_flame_blade
	   || paf->type == gsn_drain_blade ) {
	paf->duration = -1; // permanent
	SET_BIT(paf->flags,AFFECT_PERMANENT);
      }
  }
}

void spell_energy_drain(int sn, int level, CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int amount, amounthp;
  AFFECT_DATA af;

  if (victim == ch) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }

  switch ( casting_level ) {
  case 5:         
    amount = dice(level,11);   // amount that will be drained
    amounthp = dice(level,20); // amount that will be as damage
    break;
  case 4:         
    amount = dice(level,7);
    amounthp = dice(level,15);
    break;
  case 3:         
    amount = dice(level,4);
    amounthp = dice(level,11);
    break;
  case 2:         
    amount = dice(level,3);
    amounthp = dice(level,9);
    break;
  default: case 0: case 1: 
    amount = dice(level,2);
    amounthp = dice(level,7);
    break;
  }

  send_to_char("You feel an icy hand brush against your soul.\n\r",victim);
  if (saves_spell(level,victim,DAM_NEGATIVE) && casting_level < 3 ) {
    act("$n turns pale and shivers briefly.",victim,0,0,TO_ROOM);
    ability_damage(ch,victim,amounthp,sn,DAM_NEGATIVE,TRUE,TRUE);
    return;
  }

  int done = ability_damage(ch,victim,amount + amounthp,sn,DAM_NEGATIVE,TRUE,TRUE);
  if ( done != DAMAGE_DONE )
    return;

  act("$n gets a horrible look of pain in $s face and shudders in shock.",victim,0,0,TO_ROOM);
  switch(casting_level) {
  case 5:
    victim->mana = URANGE(0,victim->mana - amount,victim->cstat(max_mana));
    send_to_char("Your draining sends warm energy through you!\n\r",ch);
    //ch->mana = URANGE(0,ch->mana + amount/3, ch->cstat(max_mana));
    ch->mana += amount/3;
    send_to_char("You feel part of your mind being savagely drained.\n\r",victim);

  case 3: case 4:
    send_to_char("Your energy draining invigorates you!\n\r",ch);
    victim->move = URANGE(0,victim->move - amount, victim->cstat(max_move));
    //ch->move = URANGE(0,ch->move + amount/2, ch->cstat(max_move));
    ch->move += amount/3;
    send_to_char("You feel tired and weakened.\n\r",victim);

  case 0: case 1: case 2:
    act("You drain $N's vitality with vampiric magic.",ch,0,victim,TO_CHAR);
    send_to_char("You feel your body being mercilessly drained.\n\r",victim);
    ch->hit += amount/3;
    break;
  }
  return;
}

void spell_faerie_fire( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  //newafsetup(af,CHAR,affected_by,OR,AFF_FAERIE_FIRE,level,level,sn,casting_level);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_FAERIE_FIRE);

  // Added by SinaC 2001 for spell level
  int value;
  switch ( casting_level ) {
  default: case 0: case 1: value = 2*level; break;
  case 2: value = (23*level)/10; break;
  case 3: value = 3*level; break;
  }
  //newafsetup(af,CHAR,allAC,ADD,value,level,level,sn,casting_level);
  addaff(af,CHAR,allAC,ADD,value);
  affect_to_char( victim, &af );

  send_to_char( "You are surrounded by a pink outline.\n\r", victim );
  act( "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_faerie_fog( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *ich;

  act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

  for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room ) {
    if (ich->invis_level > 0)
      continue;

    if ( ich == ch 
	 // Added by SinaC 2001
	 || is_safe_spell( ch, ich, TRUE )
	 || saves_spell( level, ich,DAM_OTHER) )
      continue;

    // Added by SinaC 2001
    affect_strip ( ich, gsn_invisible		);

    affect_strip ( ich, gsn_invis		);
    affect_strip ( ich, gsn_mass_invis		);
    affect_strip ( ich, gsn_sneak		);
    // Added by SinaC 2003
    affect_strip ( ich, gsn_blend		);
    //REMOVE_BIT   ( ich->cstat(affected_by), AFF_HIDE	);
    //REMOVE_BIT   ( ich->cstat(affected_by), AFF_INVISIBLE	);
    //REMOVE_BIT   ( ich->cstat(affected_by), AFF_SNEAK	);
    if ( IS_NPC(ich) ) {
      REMOVE_BIT   ( ich->bstat(affected_by), AFF_HIDE	);
      REMOVE_BIT   ( ich->bstat(affected_by), AFF_INVISIBLE	);
      REMOVE_BIT   ( ich->bstat(affected_by), AFF_SNEAK	);
    }
    recompute(ich);

    // Added by SinaC 2001 for casting_level
    AFFECT_DATA af;
    switch (casting_level) {
    case 3:
      //newafsetup(af,CHAR,affected2_by,OR,AFF2_FAERIE_FOG,90,level,sn, casting_level );
      //affect_to_char(ich, &af);
      createaff(af,90,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,affected2_by,OR,AFF2_FAERIE_FOG);
      affect_to_char(ich, &af);
      break;
    case 2:
      //newafsetup(af,CHAR,affected2_by,OR,AFF2_FAERIE_FOG,15,level,sn, casting_level );
      //affect_to_char(ich, &af);
      createaff(af,15,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,affected2_by,OR,AFF2_FAERIE_FOG);
      affect_to_char(ich, &af);
      break;
    default: case 0: case 1: break;
    }

    act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
    send_to_char( "You are revealed!\n\r", ich );
  }

  return;
}

void spell_fireball( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    3,   4,   5,   7,   8,	 10,  13,  15,  16,  18,
    20,  23,  25,  28,  30,	 35,  40,  45,  50,  55,
    60,  65,  70,  75,  80,	 82,  84,  86,  88,  90,
    92,  94,  96,  98, 100,	102, 104, 106, 108, 110,
    112, 114, 116, 118, 120,	122, 124, 126, 128, 130, 
    
    133, 136, 139, 142, 145,        146, 149, 152, 155, 158,
    161, 163, 165, 168, 170,        173, 176, 179, 181, 184, 
    187, 190, 193, 196, 200,        204, 207, 209, 212, 215,
    218, 220, 224, 228, 232,        235, 238, 241, 244, 247, 
    250, 253, 256, 259, 260,        263, 265, 266, 269, 272
  };
  int dam;

  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  // Modified by SinaC 2001  was /2, *2  before instead of ., *4
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 4 ), 2 );

  // added by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( saves_spell( level, victim, DAM_FIRE) && casting_level < 3 )
    dam /= 2;

  bool done = FALSE;
  if ( casting_level == 5 )
    for ( int i = 0; i < number_fuzzy(3); i++ ) {
      int damdone = ability_damage( ch, victim, dam/3, sn, DAM_FIRE ,TRUE, TRUE);
      if ( damdone == DAMAGE_DEADLY )
	return;
      done |= damdone;
    }
  else {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);
    if ( damdone == DAMAGE_DEADLY )
      return;
    done |= damdone;
  }
  if( done && casting_level >= 4 )
    fire_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_flamestrike( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  if (target == TARGET_OBJ) {
    if ( casting_level <= 3 ) {
      send_to_char("You can cast flamestrike on weapons only at level 4 or 5.\n\r", ch );
      return;
    }

    OBJ_DATA *obj = (OBJ_DATA *) vo;
    if (obj->item_type == ITEM_WEAPON) {
      if ( obj->value[0] == WEAPON_RANGED ) {
	send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
	return;
      }

      if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)) {
	act("$p is already flaming.",ch,obj,NULL,TO_CHAR);
	return;
      }

      AFFECT_DATA af;

      int dur = level/5;
      dur *= casting_level;
      //newafsetup(af,WEAPON,NA,OR,WEAPON_FLAMING,dur,level/2,sn,casting_level);
      createaff(af,dur,level/2,sn,casting_level,AFFECT_ABILITY);
      addaff(af,WEAPON,NA,OR,WEAPON_FLAMING);

      affect_to_obj(obj,&af);

      act("$p gets a {Rfiery aura{x.",ch,obj,NULL,TO_ALL);
      return;
    }

    send_to_char("You can cast this spell only on a weapon.\n\r", ch );
    return;
  }

  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice(6 + level / 2, 8);

  // added by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;

  if ( saves_spell( level, victim,DAM_FIRE) && casting_level < 3 )
    dam /= 2;

  int done = ability_damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);
  if ( done == DAMAGE_DONE && casting_level == 5 )
    fire_effect( (void *) victim, level/2, dam, TARGET_CHAR );

  return;
}

void spell_floating_disc( int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level ) {
  OBJ_DATA *disc, *floating;
  int timer;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  floating = get_eq_char(victim,WEAR_FLOAT);
  if ( floating != NULL && !can_remove_obj( victim, floating ) ) {
    act("You can't remove $p.",victim,floating,NULL,TO_CHAR);
    return;
  }

  if ( is_affected( victim, sn ) ) {
    send_to_char("You already have a floating disc.\n\r", ch );
    return;
  }

  timer = level * 2 - number_range(0,level / 2);

  disc = create_object(get_obj_index(OBJ_VNUM_DISC), 0);
  disc->baseval[0]      = level * 10; /* 10 pounds per level capacity */
  disc->baseval[3]      = level * 5; /* 5 pounds per level max per item */
  disc->timer		= timer;
  recompobj(disc);
  act("$n has created a floating black disc.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You create a floating disc.\n\r",ch);
  obj_to_char(disc,victim);
  wear_obj(victim,disc,TRUE);

  // Added by SinaC 2001 for spell level
  AFFECT_DATA af;
  createaff(af,timer,level,sn,casting_level,AFFECT_ABILITY);
  switch( casting_level) {
  case 5: 
    //newafsetup( af, CHAR, max_mana, ADD, level*10, timer, level, sn, casting_level );
    //affect_to_char( ch, &af );
    addaff(af,CHAR,max_mana,ADD,level*10);
  case 4:
    //newafsetup( af, CHAR, WIS, ADD, UMAX(level/30,1), timer, level, sn, casting_level );
    //affect_to_char(ch,&af);
    addaff(af,CHAR,WIS,ADD,UMAX(level/30,1));
  case 3:
    //newafsetup( af, CHAR, CON, ADD, UMAX(level/30,1), timer, level, sn, casting_level );
    //affect_to_char(ch,&af);
    addaff(af,CHAR,CON,ADD,UMAX(level/30,1));
  case 2:
    //newafsetup( af, CHAR, INT, ADD, UMAX(level/30,1), timer, level, sn, casting_level );
    //affect_to_char(ch,&af);
    addaff(af,CHAR,INT,ADD,UMAX(level/30,1));
  case 0: case 1: default:
    //newafsetup( af, CHAR, NA, ADD, 0, timer, level, sn, casting_level ); 
    //affect_to_char(ch, &af);
    //addaff(af,CHAR,NA,ADD,0);
    break;
  }
  affect_to_char(victim, &af);
  return;
}

/* RT haste spell */

void spell_haste( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( casting_level >= 5
       && victim != ch
       && !IS_IMMORTAL( ch ) ) {
    send_to_charf(ch,"You can cast that casting level only upon yourself.\n\r");
    return;
  }

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

  // for spell level, SinaC 2001
  //newafsetup(af,CHAR,DEX,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32),
  //	     victim == ch ? level/2 : level/4, level, sn, casting_level );
  //  affect_to_char( victim, &af );
  createaff(af,victim == ch ? level/2 : level/4,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,DEX,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));

  //af.op = AFOP_OR;
  //af.modifier = AFF_HASTE;
  //af.location = ATTR_affected_by;
  //affect_to_char( victim, &af );
  addaff(af,CHAR,affected_by,OR,AFF_HASTE);
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself moving faster.\n\r", victim );
  act("$n is moving faster.",victim,NULL,NULL,TO_ROOM);
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_magic_missile( int sn, int level, CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
      3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
      7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
      9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
      11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
      13, 13, 13, 13, 13,	14, 14, 14, 14, 14,

      15, 15, 15, 15, 16,     16, 16, 16, 17, 17,
      17, 17, 18, 18, 18,     18, 19, 19, 19, 19,
      20, 20, 20, 20, 21,     21, 22, 22, 22, 23, 
      23, 23, 24, 24, 24,     25, 25, 25, 26, 26,
      26, 27, 27, 28, 28,     29, 29, 30, 30, 32
      };
  /*
    int dam;
    
    level	= UMIN(level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
    level	= UMAX(0, level);
    dam		= number_range( dam_each[level] / 2, dam_each[level] * 2 );
    if ( saves_spell( level, victim,DAM_ENERGY) )
    dam /= 2;
    ability_damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE, TRUE);
  */

  int dam,i;

  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  /* 1 Missile for every ten levels of the caster */

  for( level<10 ? i=1 : i=level/10; 
       i>0; 
       i--) {
    dam= UMAX( number_range( dam_each[level] / 2, dam_each[level] * 2 ), 2 );

    // Added by SinaC 2001 for casting_level
    dam *= UMAX( casting_level, 1 );
    if ( casting_level == 5 )
      dam *= 2;
    if ( saves_spell( level, victim,DAM_ENERGY) )
      dam /= 2;
    ability_damage( ch, victim, dam, sn, DAM_ENERGY ,TRUE, TRUE);
  }
  return;
}

void spell_lightning_bolt(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    4,  5,  6,  7,  9,	10, 13, 20, 25, 28,
    31, 34, 37, 40, 40,	41, 42, 42, 43, 44,
    44, 45, 46, 46, 47,	48, 48, 49, 50, 50,
    51, 52, 52, 53, 54,	54, 55, 56, 56, 57,
    58, 58, 59, 60, 60,	61, 62, 62, 63, 64,
    
    66, 68, 69, 70, 71,     74, 76, 78, 79, 80, 
    81, 82, 84, 86, 88,     90, 92, 94, 96, 98,
    99,102,106,108,110,    112,115,118,120,121,
    123,124,126,127,128,   129,131,133,134,136,
    139,140,142,144,146,   148,150,152,155,158
  };
  int dam;

  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  dam		= UMAX( number_range( dam_each[level] / 2, dam_each[level] * 2 ), 2 );
  // added by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim, DAM_LIGHTNING ) && casting_level < 3 )
    dam /= 2;

  bool done = FALSE;
  if ( casting_level == 5 )
    for ( int i = 0; i < number_fuzzy(3); i++ ) {
      int damdone = ability_damage( ch, victim, dam/3, sn, DAM_LIGHTNING, TRUE, TRUE);
      if ( damdone == DAMAGE_DEADLY )
	return;
      done |= damdone;
    }
  else {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_LIGHTNING, TRUE, TRUE);
    if ( damdone == DAMAGE_DEADLY )
      return;
    done |= damdone;
  }
  if( done && casting_level >= 4 )
    shock_effect( (void *) victim, level/2, dam, TARGET_CHAR );

  return;
}

// lvl 4 and 5: weapon is linked to caster
void spell_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;

  if (target == TARGET_OBJ) {
    obj = (OBJ_DATA *) vo;

    if (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON) {
      if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF)) {
	act("Your spell fails to corrupt $p.",ch,obj,NULL,TO_CHAR);
	return;
      }
      obj->baseval[3] = 1;
      recompobj(obj);
      act("$p is infused with poisonous vapors.",ch,obj,NULL,TO_ALL);
      return;
    }

    if (obj->item_type == ITEM_WEAPON) {
      if ( obj->value[0] == WEAPON_RANGED ) {
	send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
	return;
      }

      if (IS_WEAPON_STAT(obj,WEAPON_POISON)) {
	act("$p is already envenomed.",ch,obj,NULL,TO_CHAR);
	return;
      }

      int dur = level/5;
      dur *= casting_level;
      //newafsetup(af,WEAPON,NA,OR,WEAPON_POISON,dur,level/2,sn,casting_level);
      //affect_to_obj(obj,&af);
      createaff(af,dur,level/2,sn,casting_level,AFFECT_ABILITY);
      addaff(af,WEAPON,NA,OR,WEAPON_POISON);
      affect_to_obj( obj, &af );

      if ( !IS_NPC(ch) && casting_level >= 4 ) { // bind the weapon to caster if player
	obj->owner = str_dup( ch->name );
      }

      act("$p is coated with deadly venom.",ch,obj,NULL,TO_ALL);
      return;
    }

    act("You can't poison $p.",ch,obj,NULL,TO_CHAR);
    return;
  }

  victim = (CHAR_DATA *) vo;

  if ( saves_spell( level, victim,DAM_POISON) && casting_level < 3 ) {
    act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
    send_to_char("You feel momentarily ill, but it passes.\n\r",victim);
    return;
  }

  int dur = level * casting_level;
  if ( casting_level == 5 )
    dur *= 2;

  //newafsetup(af,CHAR,affected_by,OR,AFF_POISON,dur,level,sn,casting_level);
  //affect_join( victim, &af );
  //newafsetup(af,CHAR,STR,ADD,-2*casting_level,dur,level,sn,casting_level);
  //affect_join( victim, &af );
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_POISON);
  addaff(af,CHAR,STR,ADD,-2*casting_level);
  affect_join( victim, &af );

  if ( casting_level == 5 )
    ability_damage( ch, victim, level, sn, DAM_POISON ,TRUE, TRUE);

  send_to_char( "You feel very sick.\n\r", victim );
  act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_shield( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already shielded from harm.\n\r",ch);
    else
      act("$N is already protected by a shield.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //newafsetup(af,CHAR,allAC,ADD,-20*casting_level,level+5*casting_level,level,sn, casting_level );
  //if ( casting_level == 5 )
  //  af.modifier *= 2;
  int mod = -20*casting_level;
  if ( casting_level == 5 )
    mod *= 2;
  createaff(af,level+5*casting_level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,mod);

  affect_to_char( victim, &af );
  act( "$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a force shield.\n\r", victim );
  return;
}

void spell_shocking_grasp(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] = {
    0,
    4,  7,  9,  12,  15,	18, 20, 25, 29, 33,
    36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
    43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
    48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
    53, 53, 54, 54, 55,	55, 56, 56, 57, 57,
    
    58, 59, 60, 61, 62,     63, 64, 65, 66, 67,
    68, 69, 70, 71, 72,     73, 74, 75, 76, 77, 
    78, 79, 80, 81, 82,     83, 84, 85, 86, 87,
    88, 89, 90, 91, 92,     93, 94, 95, 96, 97,
    98, 99,100,102,104,    106,108,110,112,114
  };
  int dam;

  level	= UMIN(level, (int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  //dam		= UMAX( number_range( dam_each[level] / 2, dam_each[level] * 2 ), 2 );
  dam		= UMAX( number_range( dam_each[level],  dam_each[level] * 4 ), 2 );

  // added by SinaC 2003 for spell level
  //dam          *= casting_level*2;
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim,DAM_LIGHTNING) && casting_level < 3 )
    dam /= 2;

  int done = ability_damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, TRUE);
  if ( done == DAMAGE_DONE && casting_level >= 4 )
    shock_effect( victim, level, dam/2, TARGET_CHAR );
  return;
}

void spell_sleep( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( casting_level <= 3 && ch->fighting != NULL ) {
    send_to_char("You can only use sleep to initiate combat.\n\r", ch );
    return;
  }
  
  if ( IS_AFFECTED(victim, AFF_SLEEP)
       || ( IS_NPC(victim) 
	    && ( IS_SET(victim->act,ACT_UNDEAD)
		 || IS_SET( victim->cstat(form),FORM_UNDEAD)
		 || IS_SET(victim->act, ACT_NOSLEEP ) ) )
       || (level + 2) < victim->level
       || ( saves_spell( level-4, victim,DAM_CHARM) 
	    && casting_level < 3 ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  int dur = 4+10*casting_level+level;
  if ( casting_level == 4 )
    dur *= 2;
  if ( casting_level == 5 )
    dur = -1; // never awake

  //newafsetup(af,CHAR,affected_by,OR,AFF_SLEEP,dur,level,sn,casting_level);
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SLEEP);
  affect_join( victim, &af );

  if ( IS_AWAKE(victim) ) {
    send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", victim );
    act( "$n goes to sleep.", victim, NULL, NULL, TO_ROOM );
    if (victim->fighting || victim->position == POS_FIGHTING)
      stop_fighting(victim,TRUE);
    victim->position = POS_SLEEPING;
  }
  return;
}

void spell_slow( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn ) 
       || IS_AFFECTED(victim,AFF_SLOW)) {
    if (victim == ch)
      send_to_char("You can't move any slower!\n\r",ch);
    else
      act("$N can't get any slower than that.",
	  ch,NULL,victim,TO_CHAR);
    return;
  }
 
  if (saves_spell(level,victim,DAM_OTHER) 
      ||  IS_SET(victim->cstat(imm_flags),IRV_MAGIC)) {
    if (victim != ch)
      send_to_char("Nothing seemed to happen.\n\r",ch);
    send_to_char("You feel momentarily lethargic.\n\r",victim);
    return;
  }
 
  if (IS_AFFECTED(victim,AFF_HASTE)) {
    if (!check_dispel(level,victim,gsn_haste)) {
      if (victim != ch)
	send_to_char("Spell failed.\n\r",ch);
      send_to_char("You feel momentarily slower.\n\r",victim);
      return;
    }

    act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
    return;
  }
 

  //newafsetup(af,CHAR,affected_by,OR,AFF_SLOW,level/2,level,sn,casting_level);
  //affect_to_char( victim, &af );
  //newafsetup(af,CHAR,DEX,ADD,-1 - (level >= 18) - (level >= 25) - (level >= 32),
  //  level/2,level,sn,casting_level);
  //affect_to_char( victim, &af );
  createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SLOW);
  addaff(af,CHAR,DEX,ADD,-1 - (level >= 18) - (level >= 25) - (level >= 32));
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself slowing d o w n...\n\r", victim );
  act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_weaken( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || ( saves_spell( level, victim,DAM_WEAKEN) 
	    && casting_level < 3 ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  //  newafsetup(af,CHAR,STR,ADD,-level/5,level/2,level,sn,casting_level);
  //  affect_to_char( victim, &af );
  //  newafsetup(af,CHAR,affected_by,OR,AFF_WEAKEN,level/2,level,sn,casting_level);
  //  affect_to_char( victim, &af );
  createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_WEAKEN);
  addaff(af,CHAR,STR,ADD,-level/5);

  if ( casting_level >= 2 ) {
    //newafsetup(af,CHAR,DEX,ADD,-level/5,level/2,level,sn,casting_level);
    //affect_to_char( victim, &af );
    addaff(af,CHAR,DEX,ADD,-level/5);
  }
  if ( casting_level >= 3 ) {
    //newafsetup(af,CHAR,CON,ADD,-level/5,level/2,level,sn,casting_level);
    //affect_to_char( victim, &af );
    addaff(af,CHAR,CON,ADD,-level/5);
  }
  if ( casting_level >= 4 ) {
    //newafsetup(af,CHAR,INT,ADD,-level/5,level/2,level,sn,casting_level);
    //affect_to_char( victim, &af );
    //newafsetup(af,CHAR,WIS,ADD,-level/5,level/2,level,sn,casting_level);
    //affect_to_char( victim, &af );
    addaff(af,CHAR,INT,ADD,-level/5);
    addaff(af,CHAR,WIS,ADD,-level/5);
  }

  affect_to_char( victim, &af );

  send_to_char( "You feel your strength slip away.\n\r", victim );
  act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
  if ( casting_level >= 5 ) {
    DAZE_STATE(victim, 3 * PULSE_VIOLENCE);
    victim->stunned = 5;
    act("{gYou are stunned, and have trouble getting back up!{x", ch,NULL,victim,TO_VICT);
    act("{g$N is stunned.!{x",ch,NULL,victim,TO_CHAR);
    act("{g$N is stunned, and have trouble getting back up.{x", ch,NULL,victim,TO_NOTVICT);
  }
  return;
}
