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
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "lookup.h"
#include "magic.h"
#include "fight.h"
#include "gsn.h"
#include "act_comm.h"
#include "moons.h"
#include "effects.h"
#include "names.h"
#include "recycle.h"
#include "act_enter.h"
#include "act_info.h"
#include "update.h"
#include "spells_def.h"
#include "hunt.h"
#include "condition.h"
#include "act_obj.h"
#include "interp.h"
#include "act_wiz.h"
#include "noncombatabilities.h"
#include "utils.h"
#include "classes.h"
#include "ability.h"
#include "const.h"
#include "damage.h"
#include "weather.h"


void spell_create_shelter( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {

  OBJ_DATA *obj;
  
  obj = create_object( get_obj_index( OBJ_VNUM_SHELTER ), 0 );
  
//  [v0] Max people
//  [v1] Max weight
//  [v2] Furniture Flags
//  [v3] Heal bonus
//  [v4] Mana bonus
  obj->value[1] = 1000;
  if ( level <= 30 ) {
    obj->baseval[0] = 1;
    obj->baseval[3] = obj->baseval[4] = 115;
    obj->timer = 3;
  }
  else if ( level <= 60 ) {
    obj->baseval[0] = 2;
    obj->baseval[3] = obj->baseval[4] = 135;
    obj->timer = 8;
  }
  else {
    obj->baseval[0] = 3;
    obj->baseval[3] = obj->baseval[4] = 165;
    obj->timer = 15;
  }

  obj_to_room( obj, ch->in_room );
  recomproom( ch->in_room );

  act( "$n created a magical tent.", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You have created a magic tent.\n\r", ch );
}

void spell_strength_of_the_bear( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ){
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
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,STR,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));

  affect_to_char( victim, &af );
  send_to_char( "Your roar as a bear and your muscle surge with heightened power!\n\r", victim );
  act("$n roar as a bear and $s muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_barkskin( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("Your skin is already covered in bark.\n\r",ch);
    else
      act("$N's skin is already covered in bark.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,allAC,ADD,-20-(level*2)/3,level,level,sn);
  //affect_to_char(ch,&af);
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,-20-(level*2)/3);
  affect_to_char(victim,&af);

  act("$n's skin slowly becomes covered in bark.",victim,NULL,NULL,TO_ROOM);
  send_to_char("Your skin slowly becomes covered in hardened bark.\n\r",victim);
  if ( victim != ch )
    send_to_char("Ok.\n\r", ch );
}

void spell_eyes_of_the_wolf( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You already have the gift of sight equal to that of the powerful and fierce wolf.\n\r", ch );
    else
      act("$N already has the gift of sight.",ch,NULL,victim,TO_CHAR);
    return;
  }

  bool done = FALSE;
  createaff(af,2*level,level,sn,casting_level,AFFECT_ABILITY);
  if ( !IS_AFFECTED(victim, AFF_DARK_VISION) ) {
    //afsetup(af,CHAR,affected_by,OR,AFF_INFRARED,2*level,level,sn);
    //affect_to_char( victim, &af );
    addaff(af,CHAR,affected_by,OR,AFF_DARK_VISION);
    done = TRUE;
  }

  if ( !IS_AFFECTED(victim, AFF_DETECT_HIDDEN) ) {
    //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN,2*level,level,sn);
    //affect_to_char( victim, &af );
    addaff(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN);
    done = TRUE;
  }
  
  if ( done ) {
    affect_to_char( victim, &af );
    send_to_char("You are now empowered with the gift of sight equal to that of the wolf.\n\r",victim);
    act("$n's eyes glow red.", ch, NULL, NULL, TO_ROOM );
  }
  return;
}

void spell_free_movement( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You already walking with free movement.\n\r", ch );
    else
      act("$N is already walking with free movement.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,DEX,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32),
  //  level/2, level, sn ); // sn == gsn_free_movement
  //  affect_to_char( victim, &af );
  //  afsetup(af,CHAR,affected2_by,OR,AFF2_FREE_MOVEMENT,level/2,level,sn);
  //  affect_to_char( victim, &af );
  createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_FREE_MOVEMENT);
  addaff(af,CHAR,DEX,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));
  affect_to_char( victim, &af );

  send_to_char("You feel yourself moving quickly and cautiously.\n\r",victim);
  act("$n is moving more quickly and more cautiously.",victim,NULL,NULL,TO_ROOM);
}

void spell_rooted_feet( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int chance = 0;
  
  if ( IS_AFFECTED( victim, AFF_FLYING ) ) {
    act("$N is flying and can't be imprisonned into roots.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if ( ch->in_room == NULL ) {
    send_to_char("rooted feet: please WARN an immortal", ch );
    bug("rooted_feet: ch (%s) not in a room", NAME(ch) );
    return;
  }

  switch( ch->in_room->cstat(sector) ) {
  case SECT_INSIDE: chance = 25; break;
  case SECT_CITY: chance = 30; break;
  case SECT_FIELD: chance = 75; break;
  case SECT_FOREST: chance = 100; break;
  case SECT_HILLS: chance = 75; break;
  case SECT_MOUNTAIN: chance = 50; break;
  case SECT_WATER_SWIM:
  case SECT_WATER_NOSWIM:
  case SECT_BURNING:
  case SECT_AIR: chance = 0; break;
  case SECT_DESERT: chance = 15; break;
  case SECT_UNDERWATER: chance = 0; break;
  default: chance = 0; break;
  }

  if ( chance == 0 ) {
    send_to_char("This spell is useless in this kind of environment.\n\r", ch );
    return;
  }

  if ( chance < number_percent() ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  send_to_char("{gRoots{x comes from the ground and immobilise your legs.{x\n\r", victim );
  act("{gRoots{x comes from the ground and immobilise $N's legs.{x", ch, NULL, victim, TO_ROOM );
  act("{gRoots{x comes from the ground and immobilise $N's legs.{x", ch, NULL, victim, TO_CHAR );

  if ( !is_affected( victim, sn )
       && !is_affected( victim, gsn_entangle )
       && !IS_AFFECTED( victim, AFF_ROOTED ) ) {
    int timer = number_range( 2, 4 );
    //afsetup( af, CHAR, affected_by, OR, AFF_ROOTED, timer, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,timer,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected_by,OR,AFF_ROOTED);
    affect_to_char( victim, &af );
  }

  // does minor damage
  int dam = UMAX( dice( level/2, level/15 ), 10 );
  ability_damage( ch, victim, dam, sn, DAM_BASH, TRUE, TRUE );
  return;
}

void spell_hibernation( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *vch, *vch_next;
  AFFECT_DATA af;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( is_same_group( vch, ch ) 
	 || is_affected( vch, sn ) )
      continue;

    if ( IS_AFFECTED(vch, AFF_SLEEP)
	 || ( IS_NPC(vch) 
	      && ( IS_SET(vch->act,ACT_UNDEAD)
		   || IS_SET(vch->act, ACT_NOSLEEP ) ) )
	 || IS_SET( vch->cstat(form),FORM_UNDEAD)
	 || (level + 2) < vch->level
	 || saves_spell( level-4, vch,DAM_CHARM) ) {
      send_to_char("Spell failed.\n\r", ch );
      ability_damage(ch,vch,0,sn,DAM_NONE,TRUE, TRUE);
      continue;
    }
    
    //afsetup(af,CHAR,affected_by,OR,AFF_SLEEP,4+level,level,sn);
    createaff(af,4+level,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected_by,OR,AFF_SLEEP);

    affect_join( vch, &af );
    
    if ( IS_AWAKE(vch) ) {
      send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", vch );
      act( "$n goes to sleep.", vch, NULL, NULL, TO_ROOM );
      vch->position = POS_SLEEPING;
    }
  }
  return;
}

/* same as Holy Word for ranger */
void spell_nature_blessing(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int bless_num, curse_num, frenzy_num;
  AFFECT_DATA af;
   
  bless_num = gsn_bless;
  curse_num = gsn_curse;
  frenzy_num = gsn_frenzy;
  
  act("$n summons the force of {GNature{!",ch,NULL,NULL,TO_ROOM);
  send_to_char("You summon the force of {GNature{x.\n\r",ch);
  
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    // natural are blessed
    if ( !IS_SET(vch->act,ACT_UNDEAD)
	 && !IS_SET( vch->cstat(form),FORM_UNDEAD)) {
      send_to_char("You feel full more powerful.\n\r",vch);
      
      // Add hit/saves/AC
      //afsetup(af,CHAR,hitroll,ADD,level/8,6+level,level,sn);
      //affect_to_char( vch, &af );
      //afsetup(af,CHAR,saving_throw,ADD,0-level/8,6+level,level,sn);
      //affect_to_char( vch, &af );
      //afsetup(af,CHAR,allAC,ADD,-20,6+level,level,sn);
      //affect_to_char( vch, &af );
      createaff(af,6+level,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,level/8);
      addaff(af,CHAR,allAC,ADD,-20);
      addaff(af,CHAR,saving_throw,ADD,0-level/8);
      affect_to_char( vch, &af );
    }
    // unnatural (undead) are cursed
    else if (!is_safe_spell(ch,vch,TRUE)) {
      send_to_char("You are struck down!\n\r",vch);
      
      // Remove hit/saves/AC
      //afsetup(af,CHAR,hitroll,ADD,0-level/8,6+level,level,sn);
      //affect_to_char( vch, &af );
      //afsetup(af,CHAR,saving_throw,ADD,level/8,6+level,level,sn);
      //affect_to_char( vch, &af );
      //afsetup(af,CHAR,allAC,ADD,20,6+level,level,sn);
      //affect_to_char( vch, &af );
      createaff(af,6+level,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,0-level/8);
      addaff(af,CHAR,allAC,ADD,20);
      addaff(af,CHAR,saving_throw,ADD,level/8);
      affect_to_char( vch, &af );
      
      dam = dice(level,6);
      ability_damage(ch,vch,dam,sn,DAM_ENERGY,TRUE, TRUE);
    }
  }

  send_to_char("You feel drained.\n\r",ch);
  ch->move = 0;
  ch->hit /= 2;
}

void spell_calm_animal( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;    
  int chance;
  AFFECT_DATA af;

  /* get sum of all animal mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    if (vch->position == POS_FIGHTING) {
      count++;
      if (IS_NPC(vch)
	  && IS_SET(vch->cstat(form),FORM_ANIMAL)
	  && !IS_SET(vch->cstat(imm_flags),IRV_MAGIC) ) {
	mlevel += vch->level;
	high_level = UMAX(high_level,vch->level);
      }
    }
  }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL(ch)) /* always works */
    mlevel = 0;

  if (number_range(0, chance) >= mlevel) { /* hard to stop large fights */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
      if ( IS_SET(vch->cstat(imm_flags),IRV_MAGIC) 
	   || !IS_SET( vch->cstat(form), FORM_ANIMAL) )
	return;

      if (IS_AFFECTED(vch,AFF_CALM) 
	  || IS_AFFECTED(vch,AFF_BERSERK)
	  ||  is_affected(vch,gsn_frenzy))
	return;
	    
      send_to_char("A wave of calm passes over you.\n\r",vch);

      if (vch->fighting || vch->position == POS_FIGHTING)
	stop_fighting(vch,FALSE);

      //      afsetup(af,CHAR,affected_by,OR,AFF_CALM,level/4,level,sn);
      //      affect_to_char(vch,&af);
      //      afsetup(af,CHAR,hitroll,ADD,IS_PC(vch)?-5:-2,level/4,level,sn);
      //      affect_to_char(vch,&af);
      //      af.location = ATTR_damroll;
      //      affect_to_char(vch,&af);
      createaff(af,level/4,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,affected_by,OR,AFF_CALM);
      addaff(af,CHAR,hitroll,ADD,IS_PC(vch)?-5:-2);
      addaff(af,CHAR,damroll,ADD,IS_PC(vch)?-5:-2);
      affect_to_char( vch, &af );

    }
  }
}

void spell_holy_symbol( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) ) {
    send_to_char("Only players can use this spell.\n\r", ch );
    return;
  }

  obj = create_object( get_obj_index( OBJ_VNUM_HOLY_SYMBOL ), 0 );
  obj->level = level;

  sprintf(buf,"{yThe Symbol of {Y%s{x", god_name(ch->pcdata->god));

  obj->short_descr = str_dup(buf);

  act( "$n has created $p.", ch, obj, NULL, TO_ROOM );
  act( "You have created $p.", ch, obj, NULL, TO_CHAR );
  obj_to_char( obj, ch );
  return;
}

void spell_angelic_strength(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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
  //	  level,level,sn);
  //  affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,STR,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));
  affect_to_char( victim, &af );

  send_to_char( "Angelic strength flows through your veins!\n\r", victim );
  act("Angelic strength flows through the $n's veins.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_angelic_light(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if ( IS_AFFECTED(victim, AFF_SANCTUARY) ) {
    if (victim == ch)
      send_to_char("You are already affected by a sanctuary like spell.\n\r",ch);
    else
      act("$N is already affected by a sanctuary like spell.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //  afsetup(af,CHAR,affected_by,OR,AFF_SANCTUARY,level/6,level,sn);
  //  affect_to_char( victim, &af );
  //  afsetup(af,CHAR,hitroll,ADD,level/8,level/6,level,sn);
  //  affect_to_char( victim, &af );
  createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,hitroll,ADD,level/8);
  affect_to_char( victim, &af );

  act( "$n is surrounded by an {yangelic{x aura.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a {yangelic{x aura.\n\r", victim );
  return;
}

void spell_inspiration(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA  *gch;
  AFFECT_DATA af;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    if ( !is_same_group( gch, ch ) 
	 || is_affected( gch, sn ) )
      continue;

    send_to_char( "You feel inspirated.\n\r", gch );
    act( "$N feels inspirated.", gch, NULL, gch, TO_ROOM );

    //afsetup(af,CHAR,hitroll,ADD,level/10,level/8,level,sn);
    //affect_to_char( gch, &af );
    //afsetup(af,CHAR,damroll,ADD,level/10,level/8,level,sn);
    //affect_to_char( gch, &af );
    //afsetup(af,CHAR,STR,ADD,2,level/8,level,sn);
    //affect_to_char( gch, &af );
    //afsetup(af,CHAR,CON,ADD,2,level/8,level,sn);
    //affect_to_char( gch, &af );
    createaff(af,level/8,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,hitroll,ADD,level/10);
    addaff(af,CHAR,damroll,ADD,level/10);
    addaff(af,CHAR,STR,ADD,2);
    addaff(af,CHAR,CON,ADD,2);
    affect_to_char( gch, &af );
  }
  return;
}

void spell_judgment_bolt (int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;
 
  if (IS_EVIL(ch) ) {
    victim = ch;
    send_to_char("The energy explodes inside you!\n\r",ch);
  }

  if (victim != ch) {
    act("$n raises $s hand, and the power of law and order shoots forth!",
	ch,NULL,NULL,TO_ROOM);
    send_to_char( "You raise your hand and the power of law and order shoots forth!\n\r", ch);
  }

  if (IS_GOOD(victim)) {
    act("$n seems unharmed by the power of law and order.",victim,NULL,victim,TO_ROOM);
    send_to_char("The power of law and order seems powerless to affect you.\n\r",victim);
    return;
  }

  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_HOLY) )
    dam /= 2;

  align = victim->cstat(alignment);
  align -= 350;

  if (align < -1000)
    align = -1000 + (align + 1000) / 3;

  dam = (dam * align * align) / 1000000;

  ability_damage( ch, victim, dam, sn, DAM_HOLY ,TRUE, TRUE);
}

void spell_holy_aura(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA  *gch;
  AFFECT_DATA af;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    if ( !is_same_group( gch, ch ) 
	 || is_affected( gch, sn )
	 || IS_AFFECTED( gch, AFF2_INCREASED_CASTING )
	 || !IS_GOOD(gch) )
      continue;

    send_to_char( "You are surrounded by an holy aura.\n\r", gch );
    act( "$N is surrounded by an holy aura.", gch, NULL, gch, TO_ROOM );

    //afsetup(af,CHAR,affected2_by,OR,AFF2_INCREASED_CASTING,level/10,level,sn);
    //affect_to_char( gch, &af );
    createaff(af,level/10,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected2_by,OR,AFF2_INCREASED_CASTING);
    affect_to_char( gch, &af );
  }
  return;
}

void spell_justice(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You are already surrounded by an aura of truth and justice.\n\r", ch );
    else
      act("$N is already surrounded by an aura of truth and justice.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (IS_EVIL(victim) ) {
    if ( victim == ch )
      send_to_char("You are not good enough!\n\r",ch);
    else
      act("$N is not good enough!", ch, NULL, victim, TO_CHAR );
    return;
  }

  send_to_char( "You are surrounded by an aura of {WTruth{x and {YJustice{x.\n\r", victim );
  act( "$n is surrounded by an aura of {WTruth{x and {YJustice{x.", ch, NULL, NULL, TO_ROOM );

  //afsetup(af, CHAR, res_flags, OR, IRV_NEGATIVE, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_DISEASE, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_POISON, level/8, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/8,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_NEGATIVE);
  addaff(af,CHAR,res_flags,OR,IRV_DISEASE);
  addaff(af,CHAR,res_flags,OR,IRV_POISON);
  affect_to_char( victim, &af );
  return;
}

void spell_charms_daemon( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  AFFECT_DATA af;
  int value;

  /* deal with the object case first */
  if (target == TARGET_OBJ) {
    obj = (OBJ_DATA *) vo;
    if (IS_OBJ_STAT(obj,ITEM_EVIL)) {
      act("$p is already charmed by the daemon.",ch,obj,NULL,TO_CHAR);
      return;
    }

    if (IS_OBJ_STAT(obj,ITEM_BLESS)) {
      AFFECT_DATA *paf;

      paf = affect_find(obj->affected,gsn_bless);
      //      if (!saves_dispel(level,paf != NULL ? paf->level : obj->level,0)) {
      if (!saves_dispel(level,paf,obj->level,0)) {
	if (paf != NULL)
	  affect_remove_obj(obj,paf);
	act("$p glows a ruby red.",ch,obj,NULL,TO_ALL);
	REM_OBJ_STAT(obj,ITEM_BLESS);
	return;
      }
      else {
	act("The good of $p is too powerful for you to overcome.",
	    ch,obj,NULL,TO_CHAR);
	return;
      }
    }

    value = -1;

    //afsetup(af,CHAR,saving_throw,ADD,value,6+level,level,sn);
    //affect_to_obj(obj,&af);
    //afsetup(af,OBJECT,NA,OR,ITEM_EVIL,6+level,level,sn);
    //affect_to_obj(obj,&af);
    createaff(af,level+6,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,OBJECT,NA,OR,ITEM_EVIL);
    addaff(af,CHAR,saving_throw,ADD,value);
    affect_to_obj( obj, &af );

    act("$p glows with a red aura.",ch,obj,NULL,TO_ALL);

    return;
  }

  /* character target */
  victim = (CHAR_DATA *) vo;

  if ( victim->position == POS_FIGHTING 
       || is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already charmed by the daemon.\n\r",ch);
    else
      act("$N already has daemon favor.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,damroll,ADD,level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,saving_throw,ADD,0-level/8,6+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/8,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,damroll,ADD,level/8);
  addaff(af,CHAR,saving_throw,ADD,0-level/8);
  affect_to_char( victim, &af );

  send_to_char( "You feel evil.\n\r", victim );
  if ( ch != victim )
    act("You grant $N the favor of the daemon.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_terror(int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  bool bad_fail, flee_fail;
  int range, dur, mod;

  bad_fail = FALSE;
  flee_fail = FALSE;

  if (is_affected(victim,sn)) { 	
    send_to_char("They are already affected by a terror.\n\r",ch);
    send_to_char("You feel a shiver pass through you but it has no further affect.\n\r",victim);
    return;
  }

  if (victim == ch) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }

  act("$n points at $N and utters the word 'Terror!'",ch,0,victim,TO_NOTVICT);
  act("$n points at you and utters the word 'Terror!'",ch,0,victim,TO_VICT);
  act("You point at $N and utter the word 'Terror!'",ch,0,victim,TO_CHAR);

  if (!IS_AWAKE(victim)) {
    act("$n shivers momentarily but it passes.",victim,0,0,TO_ROOM);
    send_to_char("You feel a brief terror, but it passes away in your dreams.\n\r",
		 victim);
    return;
  }

  if (saves_spell(level,victim,DAM_NEGATIVE)) {
    act("$n shivers momentarily but it passes.",victim,0,0,TO_ROOM);
    send_to_char("You feel a brief terror, but it passes.\n\r",victim);
    return;
  }
  
  if (!saves_spell(level - 2,victim,DAM_NEGATIVE)) {
    bad_fail = TRUE;
    if (!saves_spell(level - 2,victim,DAM_NEGATIVE))
      flee_fail = TRUE;
  }
  
  act("$n's eyes widen in shock and $s entire body freezes in momentary terror.",victim,NULL,NULL,TO_ROOM);
  send_to_char("You feel an overwhelming terror and you shudder in momentary shock.\n\r",victim);

  range = level/10;

  dur = (number_range(1,5) + range);

  //afsetup( af, CHAR, damroll, ADD, -number_range(2,range), dur, level, sn );
  //affect_to_char( victim, &af );
  //afsetup( af, CHAR, hitroll, ADD, -number_range(2,range), dur, level, sn );
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,damroll,ADD,0-number_range(2,range));
  addaff(af,CHAR,saving_throw,ADD,0-number_range(2,range));
  affect_to_char( victim, &af );

  if ( flee_fail ) {
    if (victim->position == POS_FIGHTING) do_flee(victim,"");
    if (victim->position == POS_FIGHTING) do_flee(victim,"");
    if (victim->position == POS_FIGHTING) do_flee(victim,"");
  }

  if (bad_fail) WAIT_STATE(victim,12);

  return;
}

void spell_unholy_word(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  int charms_num, curse_num, daemonic_num, progood_num;

  charms_num = ability_lookup("charms of the daemon");
  curse_num = gsn_curse;
  daemonic_num = ability_lookup("daemonic aid");
  progood_num = gsn_protection_good;
   
  act("$n utters a word of daemon power!",ch,NULL,NULL,TO_ROOM);
  send_to_char("You utter a word of daemon power.\n\r",ch);
 
  if (IS_GOOD(ch) ) {
    CHAR_DATA *victim = ch;
    send_to_char("The energy explodes inside you!\n\r",ch);
    
    dam = dice( level, 10 );
    if ( saves_spell( level, victim,DAM_NEGATIVE) )
      dam /= 2;
    
    int align = victim->cstat(alignment);
    align += 350;
    
    if (align > 1000)
      align = 1000 - (align - 1000) / 3;
    
    dam = (dam * align * align) / 1000000;
    
    ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
    return;
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( IS_EVIL(vch)
	 || IS_NEUTRAL(vch)) {
      send_to_char("You feel full more powerful.\n\r",vch);

      spell_daemonic_aid(daemonic_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
      spell_charms_daemon(charms_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
      spell_protection_good(progood_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
    }
    else {
      if (!is_safe_spell(ch,vch,TRUE)) {
	send_to_char("You are struck down!\n\r",vch);
	dam = dice(level,6);
	int done = ability_damage(ch,vch,dam,sn,DAM_NEGATIVE,TRUE, TRUE);

	if (done == DAMAGE_DONE)
	  spell_curse(curse_num,level,ch,(void *) vch,TARGET_CHAR, 1 );
      }
    }
  }  
    
  send_to_char("You feel drained.\n\r",ch);
  ch->move = 0;
  ch->hit /= 2;
}

void spell_chaos_bolt(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;
 
  if (IS_GOOD(ch) ) {
    victim = ch;
    send_to_char("The energy explodes inside you!\n\r",ch);
  }

  if (victim != ch) {
    act("$n raises $s hand, and the power of chaos and disorder shoots forth!",
	ch,NULL,NULL,TO_ROOM);
    send_to_char( "You raise your hand and the power of chaos and disorder shoots forth!\n\r", ch);
  }

  if (IS_EVIL(victim)) {
    act("$n seems unharmed by the power of chaos and disorder.",victim,NULL,victim,TO_ROOM);
    send_to_char("The power of chaos seems powerless to affect you.\n\r",victim);
    return;
  }

  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;

  align = victim->cstat(alignment);
  align += 350;

  if (align > 1000)
    align = 1000 - (align - 1000) / 3;

  dam = (dam * align * align) / 1000000;

  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
}

void spell_unholy_aura(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *gch;
  AFFECT_DATA af;

  for ( gch = ch->in_room->people; gch; gch = gch->next_in_room ) {
    if ( !is_same_group( gch, ch ) 
	 || is_affected( gch, sn ) 
	 || IS_AFFECTED( gch, AFF2_INCREASED_CASTING )
	 || !IS_EVIL(gch) )
      continue;

    send_to_char( "You are surrounded by an unholy aura.\n\r", gch );
    act( "$N is surrounded by an unholy aura.", gch, NULL, gch, TO_ROOM );

    //afsetup(af,CHAR,affected2_by,OR,AFF2_INCREASED_CASTING,level/10,level,sn);
    //affect_to_char( gch, &af );
    createaff(af,level/10,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected2_by,OR,AFF2_INCREASED_CASTING);
    affect_to_char( gch, &af );
  }
  return;
}

void spell_ray_of_deception(int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, align;
 
  if (IS_GOOD(ch) ) {
    victim = ch;
    send_to_char("The energy explodes inside you!\n\r",ch);
  }

  if (victim != ch) {
    act("$n raises $s hand, and a blinding ray of darkness shoots forth!",
	ch,NULL,NULL,TO_ROOM);
    send_to_char(
		 "You raise your hand and a blinding ray of darkness shoots forth!\n\r",
		 ch);
  }

  if (IS_EVIL(victim)) {
    act("$n seems unharmed by the power of the darkness.",victim,NULL,victim,TO_ROOM);
    send_to_char("The power of the darkness seems powerless to affect you.\n\r",victim);
    return;
  }

  dam = dice( level, 10 );
  if ( saves_spell( level, victim,DAM_NEGATIVE) )
    dam /= 2;

  align = victim->cstat(alignment);
  align += 350;

  if (align > 1000)
    align = 1000 - (align - 1000) / 3;

  dam = (dam * align * align) / 1000000;

  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
}

void spell_corruption(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You are already surrounded by an aura of Lie and Corruption.\n\r", ch );
    else
      act("$N is already surrounded by an aura of Lie and Corruption.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (IS_GOOD(victim) ) {
    if ( victim == ch )
      send_to_char("You are not evil enough!\n\r",ch);
    else
      act("$N is not evil enough!", ch, NULL, victim, TO_CHAR );
    return;
  }

  send_to_char( "You are surrounded by an aura of {DLie{x and {DCorruption{x.\n\r", victim );
  act( "$n is surrounded by an aura of {DLie{x and {DCorruption{x.", ch, NULL, NULL, TO_ROOM );

  //afsetup(af, CHAR, res_flags, OR, IRV_HOLY, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_DISEASE, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_POISON, level/8, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/8,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_HOLY);
  addaff(af,CHAR,res_flags,OR,IRV_DISEASE);
  addaff(af,CHAR,res_flags,OR,IRV_POISON);
  affect_to_char( victim, &af );
  return;
}

void spell_last_rites(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  OBJ_DATA *corpse;
  OBJ_DATA *obj_next;
  int chance;

  if (target_name[0] == '\0') {
    send_to_char("Last rites which corpse?\n\r",ch);
    return;
  }
  
  corpse = get_obj_here(ch,target_name);
  
  if (corpse == NULL) {
    send_to_char("You can't find that object.\n\r",ch);
    return;
  }
  
  if ((corpse->item_type != ITEM_CORPSE_NPC) 
      && (corpse->item_type != ITEM_CORPSE_PC)) {
    send_to_char("You can't perform the last rites on that.\n\r",ch);
    return;
  }
  if ( !can_loot( ch, corpse ) ) {
    act("You are not allowed to perform the last rites on $p.",ch,corpse,NULL,TO_CHAR);
    return;
  }

  for (OBJ_DATA *obj = corpse->contains; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    obj_from_obj(obj);
    obj_to_room(obj,ch->in_room);
  }
  recomproom(ch->in_room);

  char godname[MAX_STRING_LENGTH];
  strcpy( godname, char_god_name( ch ) );//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );

  if ( number_range(0,200) < level ){ // so even at lvl 100, only 50% chance to get bonuses
    AFFECT_DATA af;
    send_to_charf(ch,"%s looks favorably upon your offering.\n\r", godname );
    //afsetup(af, CHAR, allAC, ADD, -2, 5, level, sn );
    //affect_join( ch, &af );
    //afsetup(af, CHAR, hitroll, ADD, 1, 5, level, sn );
    //affect_join( ch, &af );
    createaff(af,5,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,allAC,ADD,-2);
    addaff(af,CHAR,hitroll,ADD,1);
    if ( number_percent() == 0 ) { // 1 chance out 100
      //afsetup(af, CHAR, res_flags, OR, IRV_WEAPON, 3, level, sn );
      //affect_to_char( ch, &af );
      addaff(af,CHAR,res_flags,OR,IRV_WEAPON);
    }
    affect_join( ch, &af );
  }

  char buf[MAX_STRING_LENGTH];
  int silver = UMAX(1,corpse->level * 3);
  if (silver == 1)
    sprintf(buf,"You perform the last rites on $p and %s gives you one silver coin.",
	     godname );
  else
    sprintf(buf,"You perform the last rites on $p and %s gives you %d silver coins.",
	    godname, silver );

  act(buf,ch,corpse,NULL,TO_CHAR);
  act("$n performs the last rites on $p.",ch,corpse,NULL,TO_ROOM);

  ch->silver += silver;

  char buffer[100];
  if ( !IS_NPC(ch) 
       && IS_SET(ch->act,PLR_AUTOSPLIT) ) { /* AUTOSPLIT code */
    int members = 0;
    for (CHAR_DATA *gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
      if ( is_same_group( gch, ch ) )
	members++;
    }

    if ( members > 1 && silver > 1) {
      sprintf(buffer,"%d",silver);
      do_split(ch,buffer);	
    }
  }
  
  extract_obj(corpse);
  return;
}

void spell_hovering_sphere( int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level ) {
  OBJ_DATA *sphere, *floating;
  int timer;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  floating = get_eq_char(victim,WEAR_FLOAT);
  if ( floating != NULL && !can_remove_obj( victim, floating ) ) {
    act("You can't remove $p.",victim,floating,NULL,TO_CHAR);
    return;
  }

  if ( is_affected( victim, sn ) ) {
    send_to_char("You already have a hovering sphere.\n\r", ch );
    return;
  }

  timer = level * 2 - number_range(0,level / 2);

  sphere = create_object(get_obj_index(OBJ_VNUM_HOVERING), 0);
  sphere->baseval[0]      = level * 10; /* 10 pounds per level capacity */
  sphere->baseval[3]      = level * 5; /* 5 pounds per level max per item */
  sphere->timer		= timer;
  recompobj(sphere);
  act("$n has created an hovering sphere.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You create an hovering sphere.\n\r",ch);
  obj_to_char(sphere,victim);
  wear_obj(victim,sphere,TRUE);

  AFFECT_DATA af;
  //afsetup( af, CHAR, NA, ADD, 0, timer, level, sn ); 
  //affect_to_char(ch, &af);
  createaff(af,timer,level,sn,casting_level,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char(victim, &af);

  return;
}

void spell_cloud_of_revelation( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *ich;

  act( "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM );
  send_to_char( "You conjure a cloud of purple smoke.\n\r", ch );

  for ( ich = ch->in_room->people; ich != NULL; ich = ich->next_in_room ) {
    if (ich->invis_level > 0)
      continue;

    if ( ich == ch 
	 || is_safe_spell( ch, ich, TRUE )
	 || saves_spell( level, ich,DAM_OTHER) )
      continue;

    affect_strip ( ich, gsn_invisible		);

    affect_strip ( ich, gsn_invis		);
    affect_strip ( ich, gsn_mass_invis		);
    affect_strip ( ich, gsn_sneak		);
    affect_strip ( ich, gsn_blend		);
    if ( IS_NPC(ich) ) {
      REMOVE_BIT   ( ich->bstat(affected_by), AFF_HIDE	);
      REMOVE_BIT   ( ich->bstat(affected_by), AFF_INVISIBLE	);
      REMOVE_BIT   ( ich->bstat(affected_by), AFF_SNEAK	);
    }
    recompute(ich);

    act( "$n is revealed!", ich, NULL, NULL, TO_ROOM );
    send_to_char( "You are revealed!\n\r", ich );
  }

  return;
}

void spell_final_prayer( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  char buf[MAX_STRING_LENGTH];
  bool fail = 0;

  if ( ch->fighting == NULL ) {
    send_to_char("This spell can only be cast during combat.\n\r", ch );
    return;
  }

  send_to_char("You start to say your last prayer.\n\r", ch );
  act("$n starts to say $s last prayer.", ch, NULL, NULL, TO_ROOM );

  // Hard to succeed
  if ( number_percent() > get_ability( ch, sn ) )
    fail = 1;
  if ( number_range(0,300) >= level ) // even at level 100, 1/3 chance success
    fail = 1;

  if ( IS_IMMORTAL( ch ) )
    fail = 0;

  if ( fail ) {
    ch->hit = 1;
    ch->mana = 0;
    ch->move = 0;
    send_to_charf(ch,"But %s chooses to not hear your prayer.\n\r", 
		  char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
    sprintf(buf,"But %s chooses to not hear $s prayer.",
	    char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
    act(buf,ch,NULL,NULL,TO_ROOM);
  }
  else {
    send_to_charf(ch,"%s chooses to hear your prayer and saves you.\n\r",
		  char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
    sprintf(buf,"%s chooses to hear $s prayer and save $m.",
	    char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
    act(buf,ch,NULL,NULL,TO_ROOM);
    affect_strip(ch,gsn_plague);
    affect_strip(ch,gsn_poison);
    affect_strip(ch,gsn_blindness);
    affect_strip(ch,gsn_sleep);
    affect_strip(ch,gsn_curse);
    
    ch->hit        = ch->cstat(max_hit);
    ch->mana       = ch->cstat(max_mana);
    ch->psp        = ch->cstat(max_psp);
    ch->move       = ch->cstat(max_move);
    update_pos(ch);
  }

  return;
}

void spell_create_alcohol( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int alcohol;
  int whichone;

  //--  // check liquid.txt to see available alcohol
  //--  int choice[15] = {
  //--    1, 2, 3, 4, 5, 16, 18, 19, 20, 22, 25, 26, 28, 32, 34
  //--  };

  //--  for ( int i = 0; i < 15; i++ )
  //--    log_stringf("%d -> [%s]", i, liq_table[choice[i]].liq_name);
  int choice[MAX_LIQUID];
  int choiceCount = 0;
  for ( int i = 0; i < MAX_LIQUID; i++ )
    if ( liq_table[i].liq_affect[COND_DRUNK] > 0 ) {
      choice[choiceCount++] = i;
    }

  if ( obj->item_type != ITEM_DRINK_CON ) {
    send_to_char( "It is unable to hold alcohol.\n\r", ch );
    return;
  }

  if ( obj->value[2] != LIQ_WATER && obj->value[1] != 0 ) {
    send_to_char( "It contains some other liquid.\n\r", ch );
    return;
  }

  alcohol = UMIN( level * 4 , obj->value[0] - obj->value[1] );
  //--whichone = number_range( 0, 15 );
  whichone = number_range( 0, choiceCount );

  if ( alcohol > 0 ) {
    obj->baseval[2] = choice[whichone];
    obj->baseval[1] += alcohol;
    recompobj(obj);
    if ( !is_name( "alcohol", obj->name ) ) {
      char buf[MAX_STRING_LENGTH];
      
      sprintf( buf, "%s alcohol", obj->name );
      obj->name = str_dup( buf );
    }
    act( "$p is filled.", ch, obj, NULL, TO_CHAR );
  }

  return;
}

void spell_mystic_boat( int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level ) {
  OBJ_DATA *boat;

  boat = create_object( get_obj_index( OBJ_VNUM_MYSTIC_BOAT ), 0 );
  boat->timer = level;

  send_to_char("You have created a mystic boat.\n\r", ch );
  act("$n has created a mystic boat.", ch, NULL, NULL, TO_ROOM );

  obj_to_char( boat, ch );
}

void spell_spiritual_hammer( int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level ) {
  OBJ_DATA *hammer;
  AFFECT_DATA af;

  hammer = create_object( get_obj_index( OBJ_VNUM_SPIRITUAL_HAMMER ), 0 );
  hammer->timer = level;
  hammer->baseval[1] = UMAX(level/5,5);
  hammer->baseval[2] = number_fuzzy(8);

  recompobj(hammer);
  hammer->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(hammer,&af);
  //afsetup(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)),level,level,sn);
  //affect_to_obj(hammer,&af);
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,number_fuzzy(UMAX(level/5,3)));
  addaff(af,CHAR,damroll,ADD,number_fuzzy(UMAX(level/5,3)));
  affect_to_obj( hammer, &af);

  obj_to_char( hammer, ch );
  act( "$n has created $p.", ch, hammer, NULL, TO_ROOM );
  act( "You have created $p.", ch, hammer, NULL, TO_CHAR );
  return;
}

void spell_resist_poison( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_POISON)
       || IS_SET(victim->cstat(imm_flags),IRV_POISON)) {
    if ( victim == ch )
      send_to_char("You are already resisting to poison.\n\r",ch);
    else
      act("$N is already resisting to poison.", ch, NULL, victim, TO_CHAR );
    return;
  }
  
  //afsetup(af, CHAR, res_flags, OR, IRV_POISON, level, level, sn );
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_POISON);
  affect_to_char( victim, &af);
  
  send_to_char( "{GYou are now resisting to poison.{x\n\r", victim );
  if ( ch != victim )
    act("$N is now resisting to {Gpoison{x.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_prismatic_armor( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You are already surrounded by a {rp{yr{bi{ms{Cm{ca{wt{gi{yc armor.\n\r",ch);
    else
      act("$N is already surrounded by a {rp{yr{bi{ms{Cm{ca{wt{gi{yc armor.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,allAC,ADD,-20,level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,saving_throw,ADD,0-level/8,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,-20);
  addaff(af,CHAR,saving_throw,ADD,0-level/8);
  affect_to_char( victim, &af);

  send_to_char( "You are surrounded by a {rp{yr{bi{ms{Cm{ca{wt{gi{yc armor.\n\r", victim );
  if ( ch != victim )
    act("$N is protected by a {rp{yr{bi{ms{Cm{ca{wt{gi{yc armor.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_exhaust( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = ( CHAR_DATA * ) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || saves_spell(level,victim,DAM_HARM)) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }
  
  //afsetup(af,CHAR,DEX,ADD,-2,1+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,1+level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,DEX,ADD,-2);
  affect_to_char( victim, &af);
  
  send_to_char( "You feel exhausted.\n\r", victim );
  act( "$n feels exhausted.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_mass_harm( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( vch != ch && !is_safe_spell(ch,vch,TRUE)) {
      dam = UMAX(  20, vch->hit - dice(1,4) );
      if ( saves_spell( level, vch,DAM_HARM) )
	dam = UMIN( 250, dam / 2 );
      dam = UMIN( 500, dam );
      ability_damage( ch, vch, dam, sn, DAM_HARM ,TRUE, TRUE);
    }
  }
  
  return;
}

void spell_combat_will( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("Your combat will is as high as possible.\n\r",ch);
    else
      act("$N's combat will is as high as possible.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,CON,ADD,2,level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,CON,ADD,2);
  affect_to_char( victim, &af);

  send_to_char( "Your combat will increases.\n\r", victim );
  if ( ch != victim )
    act("$N's combat will increases.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_frozen_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] =   {
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
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 2 ), 2 );

  if ( saves_spell( level, victim, DAM_COLD) )
    dam /= 2;
  int done = ability_damage( ch, victim, dam, sn, DAM_COLD ,TRUE, TRUE);

  if ( done == DAMAGE_DONE && chance(20) && !saves_spell( level, victim, DAM_COLD ) ) 
    cold_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_fire_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] =   {
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
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 2 ), 2 );

  if ( saves_spell( level, victim, DAM_FIRE) )
    dam /= 2;
  int done = ability_damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);

  if ( done == DAMAGE_DONE && chance(20) && !saves_spell( level, victim, DAM_FIRE ) ) 
    fire_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_final_strike(int sn,int level,CHAR_DATA *ch,void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  char buf[MAX_STRING_LENGTH];
  char god[MAX_STRING_LENGTH];
  int saves = 0;

  strcpy( god, char_god_name( ch ) );//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );

  sprintf(buf,"$n summons the full wrath and fury of %s.", god );
  act(buf, ch,NULL,victim,TO_NOTVICT);
  act(buf, ch,NULL,victim,TO_VICT);
  send_to_charf(ch,"You summon the full wrath and fury of %s.\n\r", god );
  
  for ( int modify = 0; modify < 2; modify++) {
    if (saves_spell(level,victim,DAM_ENERGY) )
      saves++;
  }
  if (saves == 0) {
    sprintf(buf,"%s comes and smite $N!", god );
    act(buf, ch, NULL,victim,TO_NOTVICT);
    send_to_charf(ch,"%s comes and smite you!\n\r", god);
    sprintf(buf,"%s comes, looks at $N and smite $M!", god );
    act(buf,ch,NULL,victim,TO_CHAR);
    sudden_death( ch, victim );
    return;
  }
  else {
    sprintf("The wrath and fury of %s struck $n instead of $N", god );
    act(buf,ch,NULL,victim,TO_NOTVICT);
    send_to_charf(ch,"The wrath and fury of %s struck YOU!",god);
    //sudden_death( ch, ch );
    sprintf(buf,"has been killed by the wrath and fury of %s.", god );
    silent_kill( ch, buf );
    return;
  }
  return;
}

void spell_deplete_strength( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = ( CHAR_DATA * ) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) 
       || saves_spell(level,victim,DAM_WEAKEN)) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }
  
  //  afsetup(af,CHAR,STR,ADD,-3,1+level,level,sn);
  //  affect_to_char( victim, &af );
  createaff(af,1+level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,STR,ADD,-3);
  affect_to_char( victim, &af);
  
  send_to_char( "You feel enfeebled.\n\r", victim );
  act( "$n feels enfeebled.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_detect_weather( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  static char * const sky_look[4] = {
    "cloudless",
    "cloudy",
    "rainy",
    "lit by flashes of lightning"
  };
  
  send_to_charf( ch, "The sky is %s and %s.\n\r",
		 sky_look[weather_info.sky],
		 weather_info.change >= 0
		 ? "a warm southerly breeze blows"
		 : "a cold northern gust blows"
		 );

  if (moon_night()) {
    int i;
    for (i=0;i<NBR_MOON;i++)
      if (moon_visible(i))
	send_to_charf(ch, moon_phase_msg[moon_phase(i)], moon_info[i].name);
  }
  return;
}

void spell_lightning_reflexes( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
  if ( is_affected( victim, sn ) 
       || IS_AFFECTED(victim,AFF_HASTE)
       || IS_SET(victim->off_flags,OFF_FAST)) {
    if (victim == ch)
      send_to_char("Your reflexes can't be faster!\n\r",ch);
    else
      act("$N's reflexes are already as fast as $E can.",
	  ch,NULL,victim,TO_CHAR);
    return;
  }


  //  afsetup(af,CHAR,DEX,ADD,3, victim == ch ? level/2 : level/4, level, sn );
  //  affect_to_char( victim, &af );
  createaff(af, victim == ch ? level/2 : level/4,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,DEX,ADD,3);
  affect_to_char( victim, &af);

  send_to_char( "You start to move at lightning speed.\n\r", victim );
  act("$n starts moving at lightning speed.",victim,NULL,NULL,TO_ROOM);
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_create_rainbow( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
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
  dam		= UMAX( number_range( dam_each[level],  dam_each[level] * 2 ), 2 );

  if ( saves_spell( level, victim,DAM_LIGHT) )
    dam /= 2;

  int done = ability_damage( ch, victim, dam, sn, DAM_LIGHT,TRUE, TRUE );

  // Blind opponent ?
  if ( done == DAMAGE_DONE && chance(20) && !saves_spell( level, victim, DAM_LIGHT) ) {
    if ( IS_AFFECTED(victim, AFF_BLIND) 
	 || saves_spell(level,victim,DAM_OTHER))
      return;

    AFFECT_DATA af;
    //afsetup(af,CHAR,hitroll,ADD,-level/10,1+level,level,sn);
    //affect_to_char( victim, &af );
    //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,1+level,level,sn);
    //affect_to_char( victim, &af );
    createaff(af,1+level,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,hitroll,ADD,-level/10);
    addaff(af,CHAR,affected_by,OR,AFF_BLIND);
    affect_to_char( victim, &af);
    
    send_to_char( "You are blinded!\n\r", victim );
    act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
  }
  return;
}

void spell_protection_lightning(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_LIGHTNING)
       || IS_SET(victim->cstat(imm_flags),IRV_LIGHTNING)) {
    if ( victim == ch )
      send_to_char("You are already protected from lightning.\n\r",ch);
    else
      act("$N is already protected from lightning.",ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_LIGHTNING, level, level, sn );
  //  affect_to_char( ch, &af );
  createaff(af,1+level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_LIGHTNING);
  affect_to_char( victim, &af);

  send_to_char( "You feel protected from {ylightning{x.\n\r", victim );
  if ( ch != victim )
    send_to_char("Ok.\n\r", ch );
  return;
}

void spell_summon_thunder( int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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

  dam = dice(level/2, 15); // more damage than call lightning

  send_to_charf( ch, 
		 "%s's booming crashes of thunder strikes your foes!\n\r",
		 char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
  sprintf( buf, 
	   "$n calls %s's crashes of thunder to strike $s foes!",
	   char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
  act( buf, ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room ) {
      if ( vch != ch && ( IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch) ) )
	ability_damage( ch, vch, saves_spell( level,vch,DAM_SOUND)
		? dam / 2 : dam, sn,DAM_SOUND,TRUE, TRUE);
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area
	 &&   IS_OUTSIDE(vch)
	 &&   IS_AWAKE(vch) )
      send_to_char( "Crashes of thunder booms in sky.\n\r", vch );
  }

  return;
}

void spell_windwalk( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You already feel as walking on wind.\n\r", ch );
    else
      act("$N already feels as walking on wind.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af,CHAR,affected2_by,OR,AFF2_FREE_MOVEMENT,level/2,level,sn);
  //affect_to_char( ch, &af );
  createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_FREE_MOVEMENT);
  affect_to_char( victim, &af );

  send_to_char("You feel as walking on wind.\n\r",victim);
  if ( ch != victim )
    send_to_char("Ok.\n\r", ch );
}

void spell_earthcrumble( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;

  if ( ch->in_room->cstat(sector) == SECT_AIR
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char( "There is no earth around!\n\r", ch );
    return;
  }
  
  send_to_char( "The earth shakes beneath your feet!\n\r", ch );
  act( "$n makes the earth shake and crumble.", ch, NULL, NULL, TO_ROOM );
  
  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL )
      continue;
    if ( vch->in_room == ch->in_room ) {
      if ( vch != ch && !is_safe_spell(ch,vch,TRUE))
	if (IS_AFFECTED(vch,AFF_FLYING))
	  ability_damage(ch,vch,0,sn,DAM_EARTH,TRUE, TRUE);
	else
	  ability_damage( ch,vch,dice(level/4, 6), sn, DAM_EARTH,TRUE, TRUE);
      continue;
    }
      
    if ( vch->in_room->area == ch->in_room->area )
      send_to_char( "The earth shakes and crumbles.\n\r", vch );
  }
  
  return;
}

void spell_lightning_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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
  dam		= UMAX( number_range( dam_each[level]*3/2, dam_each[level] * 3 ), 2 );
  if ( saves_spell( level, victim,DAM_LIGHTNING) )
    dam /= 2;
  int done = ability_damage( ch, victim, dam, sn, DAM_LIGHTNING ,TRUE, TRUE);
  if ( done == DAMAGE_DONE && !saves_spell( level, victim,DAM_LIGHTNING) && chance(20) )
    shock_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_hurricane(int sn, int level, CHAR_DATA *ch,void *vo, int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  AFFECT_DATA af;
  static char * const him_her_self[] = { "itself", "himself", "herself" };

  if ( ( ch->in_room->cstat(sector) == SECT_WATER_SWIM )
       || ( ch->in_room->cstat(sector) == SECT_WATER_NOSWIM ) 
       || ( ch->in_room->cstat(sector) == SECT_UNDERWATER ) ) {
    send_to_char("You can't do that in this environment.\n\r",ch);
    return;
  }

  send_to_char("You raise a violent hurricane to strike your foes.\n\r",ch);
  act("$n raises a violent hurricane, sending debris flying!",ch,NULL,NULL,TO_ROOM);
  dam = dice(level, 5);
  if ( dam > 20 )
    dam += dice(level, 2);
  if ( dam > 40 )
    dam += dice(level, 2);
  if ( dam > 60 )
    dam += dice(level, 2);
  if ( dam > 80 )
    dam += dice(level, 2);

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    if ((number_range(0,3) == 0) && !saves_spell(level,vch,DAM_BASH)) {
      act("$n appears blinded by the debris.",vch,NULL,NULL,TO_ROOM);
      int dur = number_fuzzy(2);
      //afsetup(af,CHAR,hitroll,ADD,-3,dur,level,sn);
      //affect_to_char(vch,&af);
      //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,dur,level,sn);
      //affect_to_char( vch, &af );
      createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,-3);
      addaff(af,CHAR,affected_by,OR,AFF_BLIND);
      affect_to_char( vch, &af );
    }
    ability_damage(ch,vch,dam,sn,DAM_BASH,TRUE,TRUE);

    if ( is_affected( vch, gsn_levitation )
	 || is_affected( vch, gsn_fly) 
	 || is_affected( vch, gsn_flight) 
	 || is_affected( vch, gsn_flight_bird) ) {
      if (!saves_spell(level,vch,DAM_BASH) ) {
	act("$n is thrown wildly to the ground by the air blast!",vch,NULL,NULL,TO_ROOM);
	send_to_char("You are thrown down by the air blast!\n\r",vch);
	affect_strip(vch,gsn_fly);
	affect_strip(vch,gsn_levitation);
	affect_strip(vch,gsn_flight);
	affect_strip(vch,gsn_flight_bird);
      }
    }

    // Caster is sometimes strikes back
    if ( ch == vch && chance(30) && !saves_spell(level,ch,DAM_BASH) ) {
      char buf[MAX_STRING_LENGTH];
      sprintf( buf, "kills %s with a hurricane!", 
	       him_her_self[URANGE(0,ch->cstat(sex),2)] );
      noaggr_damage( ch, dam, DAM_LIGHTNING,
		     "You are struck by your own hurricane!",
		     "The wind strikes $n back...whoops!",
		     buf,
		     TRUE );
    }
  }
  return;
}

void spell_locate_warpstone( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = IS_IMMORTAL(ch) ? 50 : level;

  buffer = new_buf();
 
  for ( obj = object_list; obj != NULL; obj = obj->next ) {
    if ( !can_see_obj( ch, obj ) 
	 || obj->item_type != ITEM_WARP_STONE 
	 || IS_OBJ_STAT(obj,ITEM_NOLOCATE) 
	 || number_percent() > 2 * level
	 || ch->level < obj->level)
      continue;

    found = TRUE;
    number++;

    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
      ;
    
    CHAR_DATA *carrier = in_obj->carried_by;
    ROOM_INDEX_DATA *room = in_obj->in_room;

    if ( carrier != NULL && can_see(ch,carrier) 
	 && carrier->in_room != NULL
	 && ( carrier->in_room->area == ch->in_room->area 
	      || IS_IMMORTAL(ch) ) ) {
      sprintf( buf, "%s is carried by %s\n\r",
	       obj->short_descr, PERS(carrier, ch) );
    }
    else
      if (room != NULL) {
	if ( IS_IMMORTAL(ch) )
	  sprintf( buf, "%s is in %s [Room %d]\n\r",
		   obj->short_descr,
		   room->name, room->vnum);
	else if ( room->area == ch->in_room->area )
	  sprintf( buf, "%s is in %s\n\r",
		   obj->short_descr, 
		   room == NULL ? "somewhere" : room->name );
      }

    buf[0] = UPPER(buf[0]);
    add_buf(buffer,buf);

    if (number >= max_found)
      break;
  }

  if ( !found )
    send_to_char( "You can't locate any warpstones nearby.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);

  return;
}

void spell_unstable_gate( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim;
  bool gate_pet;

  victim = get_char_world( ch, target_name );
  if ( victim == NULL 
       || victim == ch
       || IS_SET(victim->act, ACT_IS_SAFE )
       || victim->level >= level + 3
       || (is_clan(victim) && !is_same_clan(ch,victim))
       || (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
       || (IS_NPC(victim) && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       || (IS_NPC(victim) && saves_spell( level, victim,DAM_OTHER) )
       || IS_SET(ch->in_room->cstat(flags), ROOM_NO_RECALL) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  ROOM_INDEX_DATA *to_room;
  if ( chance(50) ) // 50% chance to go elsewhere
    to_room = get_random_room(ch);
  else
    to_room = victim->in_room;

  if ( to_room == NULL
       || !can_see_room(ch,to_room) 
       || IS_SET(to_room->cstat(flags), ROOM_SAFE)
       || IS_SET(to_room->cstat(flags), ROOM_PRIVATE)
       || IS_SET(to_room->cstat(flags), ROOM_SOLITARY)
       || IS_SET(to_room->cstat(flags), ROOM_NO_RECALL) ) {
    send_to_char( "Spell failed.\n\r", ch );
    return;
  }

  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;
    
  act("$n steps through a gate and vanishes.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You step through a gate and vanish.\n\r",ch);
  char_from_room(ch);
  char_to_room(ch,to_room);

  act("$n has arrived through a gate.",ch,NULL,NULL,TO_ROOM);
  do_look(ch,"auto");

  if (gate_pet) {
    act("$n steps through a gate and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
    send_to_char("You step through a gate and vanish.\n\r",ch->pet);
    char_from_room(ch->pet);
    char_to_room(ch->pet,to_room);
    act("$n has arrived through a gate.",ch->pet,NULL,NULL,TO_ROOM);
    do_look(ch->pet,"auto");
  }
  return;
}

void spell_gemwarp( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  AFFECT_DATA *paf; 
  int result, fail;
  int ac_bonus, added;
  bool ac_found = FALSE;

  if (obj->item_type != ITEM_GEM) {
    send_to_char("That isn't an gem.\n\r",ch);
    return;
  }

  if (obj->wear_loc != -1) {
    send_to_char("The item must be carried to be converted in a warpstone.\n\r",ch);
    return;
  }

  fail = 25; // base 25% chance of failure

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
	fail += 20;
  
  for ( paf = obj->affected; paf != NULL; paf = paf->next )
    fail += 20;

  /* apply other modifiers */
  fail -= level;

  if (IS_OBJ_STAT(obj,ITEM_BLESS))
    fail -= 15;
  if (IS_OBJ_STAT(obj,ITEM_GLOW))
    fail -= 5;

  fail = URANGE(5,fail,85);

  result = number_percent();

  /* the moment of truth */
  if (result < (fail / 5)) { /* item destroyed */
    act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_CHAR);
    act("$p flares blindingly... and evaporates!",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    return;
  }

  if (result < (fail / 3)) {/* item disenchanted and become trash */
    AFFECT_DATA *paf_next;

    act("$p glows brightly, then fades...oops.",ch,obj,NULL,TO_CHAR);
    act("$p glows brightly, then fades.",ch,obj,NULL,TO_ROOM);
    /* remove all affects */
    obj->affected = NULL;

    /* Clear flags */
    REM_OBJ_STAT(obj,ITEM_GLOW|ITEM_HUM|ITEM_MAGIC);
    obj->item_type = ITEM_TRASH;
    obj->baseval[0] = obj->baseval[1] = obj->baseval[2] = obj->baseval[3] = obj->baseval[4] = 0;
    recompobj( obj );
    return;
  }

  if ( result <= fail ) { /* failed, no bad result */
    send_to_char("Nothing seemed to happen.\n\r",ch);
    return;
  }

  obj->item_type = ITEM_WARP_STONE;
  obj->baseval[0] = obj->baseval[1] = obj->baseval[2] = obj->baseval[3] = obj->baseval[4] = 0;
  recompobj( obj );
  
  act("You successfully transform $p in a warp stone.", ch, obj, NULL, TO_CHAR );
  return;
}

void spell_protection_negative(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_NEGATIVE)
       || IS_SET(victim->cstat(imm_flags),IRV_NEGATIVE)) {
    if ( ch == victim )
      send_to_char("You are already protected from negative.\n\r",ch);
    else
      act("$N is already protected from negative.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_NEGATIVE, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_NEGATIVE);
  affect_to_char( victim, &af );
  
  send_to_char( "You feel protected from negative.\n\r", victim );
  if ( victim != ch )
    send_to_char("Ok.\n\r", ch);
  return;
}

void spell_reverse_gravity(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( ch->in_room->cstat(sector) == SECT_AIR
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char("You can't do that in this environment.\n\r",ch);
    return;
  }

  int dam = dice( level, 10 );
  if ( saves_spell( level, ch, DAM_BASH ) )
    dam /= 2;

  act("You change the gravitational field surrounding $N.", ch, NULL, victim, TO_CHAR );
  act("$N shoot upwards and immediatly crash back down to earth.", ch, NULL, victim, TO_NOTVICT );
  act("You suddenly shoot upwards, then immediatly crash back down to earth.",ch,NULL,victim,TO_VICT);
  ability_damage( ch, (CHAR_DATA *) vo, dam, sn, DAM_BASH, TRUE, TRUE );
}

void spell_angelfire(int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_NPC(ch) && !IS_GOOD(ch) ) {
    victim = ch;
    send_to_char("The Angels turn to attack you!\n\r",ch);
  }

  ch->bstat(alignment) = UMAX(1000, ch->bstat(alignment) + 50);

  if (victim != ch) {
    char buf[MAX_STRING_LENGTH];
    sprintf(buf,"$n invokes the wrath of %s on $N!",
	    char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
    act(buf, ch,NULL,victim,TO_ROOM);
    act("$n has called forth the Angels of heaven to exorcise you!",
	ch,NULL,victim,TO_VICT);
    send_to_charf(ch,"You call forth Angels of %s!\n\r",
		  char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
  }
  dam = dice( level, 10 );
  if ( saves_spell( level, victim, DAM_HOLY ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_HOLY, TRUE, TRUE );
}

void spell_life_transfer(int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int        hpch;

  if ( ch == victim ) {
    send_to_char( "Transfer life to yourself ?\n\r", ch );
    return;
  }

  hpch = UMIN( 150, victim->cstat(max_hit) - victim->hit );
  if ( hpch == 0 ) {
    act( "Nice thought, but $N doesn't need healing.", ch, NULL,
	 victim, TO_CHAR );
    return;
  }
  
  if ( ch->hit-hpch < 150 ) {
    send_to_char( "You aren't healthy enough yourself!\n\r", ch );
    return;
  }
  victim->hit += hpch;
  ch->hit     -= hpch;
  update_pos( victim );
  update_pos( ch );

  act( "You transfer some life to $N.", ch, NULL, victim, TO_CHAR );
  act( "$n transfers you some life.",     ch, NULL, victim, TO_VICT );

  return;
}


void spell_protection_holy(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_HOLY)
       || IS_SET(victim->cstat(imm_flags),IRV_HOLY)) {
    if ( victim == ch )
      send_to_char("You are already protected from holy.\n\r",ch);
    else
      act("$N is already protected from holy.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_HOLY, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_HOLY);
  affect_to_char( victim, &af );

  send_to_char( "You feel protected from holy.\n\r", victim );
  if ( ch == victim )
    send_to_char("Ok.\n\r", ch );
  return;
}

void spell_life_drain( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if (victim != ch)
    ch->bstat(alignment) = UMAX(-1000, ch->bstat(alignment) - 50);

  if ( saves_spell( level, victim,DAM_NEGATIVE) ) {
    send_to_char("You feel a momentary chill.\n\r",victim);

    act( "$n feels a momentary chill.", victim, NULL, NULL, TO_NOTVICT );
    return;
  }


  if ( victim->level <= 2 ) {
    dam		 = ch->hit + 1;
  }
  else {
    gain_exp( victim, 0 - number_range( level/2, 3 * level / 2 ), TRUE );
    victim->mana	/= 2;
    victim->psp 	/= 2;
    victim->move	/= 2;
    dam		 = dice(1, level);
    ch->hit		+= dam;
  }

  send_to_char("You feel your life slipping away!\n\r",victim);
  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE ,TRUE, TRUE);
  act("You drain life from $N!",ch,NULL,victim,TO_CHAR);

  return;
}

void spell_nature_shield(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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
  createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  affect_to_char( victim, &af );

  act( "$n is surrounded by a shield of {gnature{x.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a shield of {gnature{x.\n\r", victim );
  return;
}

void spell_anti_venom_spores(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_POISON)
       || IS_SET(victim->cstat(imm_flags),IRV_POISON)) {
    if ( victim == ch )
      send_to_char("You are already resisting to poison.\n\r",ch);
    else
      act("$N is already resisting to poison.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_POISON, level/10, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/10,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_POISON);
  affect_to_char( victim, &af );

  act("$n summons forth a small handful of powerful magic spores", ch, NULL, NULL, TO_ROOM );
  send_to_char("You summon forth a small handful of powerful magic spores trickle down onto you.\n\r", victim );
  return;
}

void spell_flower_of_health(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;
 
  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_DISEASE)
       || IS_SET(victim->cstat(imm_flags),IRV_DISEASE)) {
    if ( ch == victim )
      send_to_char("You are already resisting to disease.\n\r",ch);
    else
      act("$N is already resisting to disease.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_DISEASE, level/10, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/10,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_DISEASE);
  affect_to_char( victim, &af );

  act("$n summons forth the image of a large {YYellow{x flower.", ch, NULL, NULL, TO_ROOM );
  send_to_char("You summon forth the image of a large {YYellow{x flower.\n\r", victim );
  return;
}

void spell_flight_of_the_bird( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
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
  createaff(af,3+level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_FLYING);
  affect_to_char( ch, &af );

  send_to_char( "Your start flying like a bird.\n\r", victim );
  act( "$n starts flying as a bird.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_polar_adaptation(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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

  //afsetup(af, CHAR, res_flags, OR, IRV_COLD, level/10, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/10,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_COLD);
  affect_to_char( victim, &af );

  act("$n is tuned with polar creatures.", ch, NULL, NULL, TO_ROOM );
  send_to_char("You are tuned with polar creatures.\n\r",victim);
  return;
}

void spell_desert_adaptation(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;
 
  if ( is_affected( victim, sn ) 
       || IS_SET(victim->cstat(res_flags),IRV_FIRE)
       || IS_SET(victim->cstat(imm_flags),IRV_FIRE)) {
    if ( ch == victim )
      send_to_char("You are already resisting to heat and fire.\n\r",ch);
    else
      act("$N is already resisting to heat and fire.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_FIRE, level/10, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/10,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_FIRE);
  affect_to_char( victim, &af );

  act("$n is tuned with desert creatures.", ch, NULL, NULL, TO_ROOM );
  send_to_char("You are tuned with desert creatures.\n\r",victim);
  return;
}

void spell_speed_of_the_cheetah( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
 
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

  //newafsetup(af,CHAR,DEX,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32),
  //	     victim == ch ? level/2 : level/4, level, sn, 2 );
  //affect_to_char( victim, &af );
  //af.op = AFOP_OR;
  //af.modifier = AFF_HASTE;
  //af.location = ATTR_affected_by;
  //affect_to_char( victim, &af );
  createaff(af,victim == ch ? level/2 : level/4,level,sn,2,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_HASTE);
  addaff(af,CHAR,DEX,ADD,1 + (level >= 18) + (level >= 25) + (level >= 32));
  affect_to_char( victim, &af );

  send_to_char( "You feel yourself moving as quick as a cheetah.\n\r", victim );
  act("$n is moving as quick as a cheetah.",victim,NULL,NULL,TO_ROOM);
  if ( ch != victim )
    send_to_char( "Ok.\n\r", ch );
  return;
}

void spell_insect_swarm( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  send_to_char( "You summon a horde of insects!\n\r", ch );
  act( "$n summons a horde of insects!", ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL || vch == ch )
      continue;
    if ( vch->in_room == ch->in_room ) {
      if ( is_safe_spell(ch,vch,TRUE) || is_same_group(vch,ch) )
	  continue;

      dam = dice(level/2, 10);
      int tmp_dam = dam,
	tmp_lvl = level;
      if ( saves_spell( level, vch, DAM_POISON ) ) {
	tmp_dam = dam/2;
	tmp_lvl = level/2;
      }
      int done = ability_damage( ch, vch, tmp_dam, sn, DAM_POISON, TRUE, TRUE );
      if ( done == DAMAGE_DONE && chance(20) && !saves_spell( level, vch, DAM_POISON) ) {
	AFFECT_DATA af;
	//afsetup(af,CHAR,affected_by,OR,AFF_POISON,level,level,sn);
	//affect_join( vch, &af );
	//afsetup(af,CHAR,STR,ADD,-2,level,level,sn);
	//affect_join( vch, &af );
	createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
	addaff(af,CHAR,affected_by,OR,AFF_POISON);
	addaff(af,CHAR,STR,ADD,-2);
	affect_to_char( vch, &af );
	
	send_to_char( "You feel very sick.\n\r", vch );
	act("$n looks very ill.",vch,NULL,NULL,TO_ROOM);
      }
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area )
      send_to_char( "From somewhere nearby you hear a buzzing sound.\n\r", vch );
  }

  return;
}

void spell_animal_friendship( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if (is_safe(ch,victim)) 
    return;

  if ( victim == ch ) {
    send_to_char( "You like yourself even better!\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim)
       || IS_AFFECTED(victim, AFF_CHARM)
       || IS_AFFECTED(ch, AFF_CHARM)
       || level < victim->level
       || IS_SET(victim->cstat(imm_flags),IRV_CHARM)
       || saves_spell( level, victim,DAM_CHARM) 
       || !IS_SET( victim->cstat(form), FORM_ANIMAL) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  if (IS_SET(victim->in_room->cstat(flags),ROOM_LAW)
      && !IS_IMMORTAL(ch)) {
    send_to_char("The mayor does not allow charming in the city limits.\n\r",ch);
    return;
  }

  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );
  victim->leader = ch;

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 4 ),level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( victim, &af );

  act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
  if ( ch != victim )
    act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_thorn_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] =   {
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
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 2 ), 2 );

  act("You summon a blast of sharp, piercing thorns that fly at $N.", ch, NULL, victim, TO_CHAR );
  act("$n summons a blast of sharp, piercing {Gthorns{x that fly at you.", ch, NULL, victim, TO_VICT );
  act("$n summons a blast of sharp, piercing {Gthorns{x that fly at $N.", ch, NULL, victim, TO_NOTVICT );

  if ( saves_spell( level, victim, DAM_PIERCE) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn, DAM_PIERCE ,TRUE, TRUE);

  return;
}

void spell_plant_door( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim;
  bool gate_pet;

  if ( ch->in_room->cstat(sector) == SECT_CITY
       || ch->in_room->cstat(sector) == SECT_INSIDE ) {
    send_to_char("A plant door can only be created between to outdoors location.\n\r", ch );
    return;
  }

  victim = get_char_world( ch, target_name );
  if ( victim == NULL 
       || victim == ch
       || IS_SET(victim->act, ACT_IS_SAFE )
       || victim->level >= level + 5 // 3  for spell_gate
       || (is_clan(victim) && !is_same_clan(ch,victim))
       || (!IS_NPC(victim) && victim->level >= LEVEL_HERO)  /* NOT trust */ 
       || (IS_NPC(victim) && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))
       || (IS_NPC(victim) && saves_spell( level-5, victim,DAM_OTHER) ) // level  for spell_gate
       || IS_SET(ch->in_room->cstat(flags), ROOM_NO_RECALL) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  ROOM_INDEX_DATA *to_room = victim->in_room;

  if ( to_room == NULL
       || !can_see_room(ch,to_room) 
       || IS_SET(to_room->cstat(flags), ROOM_SAFE)
       || IS_SET(to_room->cstat(flags), ROOM_PRIVATE)
       || IS_SET(to_room->cstat(flags), ROOM_SOLITARY)
       || IS_SET(to_room->cstat(flags), ROOM_NO_RECALL) ) {
    send_to_char( "Spell failed.\n\r", ch );
    return;
  }

  if ( to_room->cstat(sector) == SECT_CITY 
       || to_room->cstat(sector) == SECT_INSIDE ) {
    send_to_char("A plant door can only be created between to outdoors location.\n\r", ch );
    return;
  }

  if (ch->pet != NULL && ch->in_room == ch->pet->in_room)
    gate_pet = TRUE;
  else
    gate_pet = FALSE;
    
  act("$n steps through a plant and vanishes.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You step through a plant and vanish.\n\r",ch);
  char_from_room(ch);
  char_to_room(ch,to_room);

  act("$n has arrived through a plant.",ch,NULL,NULL,TO_ROOM);
  do_look(ch,"auto");

  if (gate_pet) {
    act("$n steps through a plant and vanishes.",ch->pet,NULL,NULL,TO_ROOM);
    send_to_char("You step through a plant and vanish.\n\r",ch->pet);
    char_from_room(ch->pet);
    char_to_room(ch->pet,to_room);
    act("$n has arrived through a plant.",ch->pet,NULL,NULL,TO_ROOM);
    do_look(ch->pet,"auto");
  }
  return;
}

void spell_choke( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_SET( victim->cstat(form), FORM_UNDEAD ) 
       || IS_SET( victim->cstat(form), FORM_INTANGIBLE ) 
       || IS_SET( victim->cstat(form), FORM_MIST ) ) {
    act("$N is unaffected by this kind of spell.", ch, NULL, victim, TO_CHAR);
    return;
  }

  send_to_char("Something grabs your throat and attempts to suffocate you",victim);

  // if casting_level >= 2, chance to reduce victim->hit  to within an inch of death (5% total hp)
  if ( casting_level >= 2 ) {
    int max = 100; // 1 chance out 1
    if ( casting_level == 2 ) // if % == 100 then 1 chance out 200
      max = 100*200;
    else if ( casting_level == 3 ) // if % == 100 then 1 chance out 50
      max = 100*50;
    else if ( casting_level == 4 ) // if % == 100 then 1 chance out 10
      max = 100*10;
    else if ( casting_level == 5 ) // if % == 100 then 2 chance out 3
      max = 150;
    if ( number_range( 0, max ) < get_ability( ch, sn )
	 && victim->hit > victim->cstat(max_hit)/20 
	 && !saves_spell( level, victim, DAM_NEGATIVE ) ) {
      send_to_char("You suffocate and almost die...\n\r",victim);
      act("$N suffocates and almost die...", ch, NULL, victim, TO_CHAR );
      victim->hit = victim->cstat(max_hit)/20;
      update_pos(victim);
      return;
    }
  }

  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level < 3 )
    dam /= 2;

  send_to_char("You suffocate...\n\r", victim );
  act("$N suffocates...", ch, NULL, victim, TO_CHAR );
  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE );
}

void spell_thunderclap( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  
  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_SOUND )
       && casting_level < 3 )
    dam /= 2;
  if ( casting_level == 5 )
    dam *= 2;

  ability_damage( ch, victim, dam, sn, DAM_SOUND, TRUE, TRUE );
  if ( casting_level >= 4 ) { // additional lightnings
    send_to_char("Bolts of lightning come out the thunder and strike you.", victim );
    if ( chance(50) )
      ability_damage( ch, victim, dam/2, gsn_lightning_bolt, DAM_LIGHTNING, TRUE, TRUE );
    ability_damage( ch, victim, dam/2, gsn_lightning_bolt, DAM_LIGHTNING, TRUE, TRUE );
  }
}

void spell_entangle( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  int chance = 0;

  // Can be used in combat only if casting level 4 or 5
  if ( casting_level <= 3
       && ch->fighting != NULL ) {
    send_to_char("You can't cast 'entangle' during combat.\n\r", ch );
    return;
  }
  
  if ( IS_AFFECTED( victim, AFF_FLYING ) ) {
    act("$N is flying and can't be imprisonned into roots.",ch,NULL,victim,TO_CHAR);
    return;
  }
  
  if ( ch->in_room == NULL ) {
    send_to_char("entangle: please WARN an immortal", ch );
    bug("entangle: ch (%s) not in a room", NAME(ch) );
    return;
  }

  switch( ch->in_room->cstat(sector) ) {
  case SECT_INSIDE: chance = 25; break;
  case SECT_CITY: chance = 30; break;
  case SECT_FIELD: chance = 75; break;
  case SECT_FOREST: chance = 100; break;
  case SECT_HILLS: chance = 75; break;
  case SECT_MOUNTAIN: chance = 50; break;
  case SECT_WATER_SWIM:
  case SECT_WATER_NOSWIM:
  case SECT_BURNING:
  case SECT_AIR: chance = 0; break;
  case SECT_DESERT: chance = 15; break;
  case SECT_UNDERWATER: chance = 0; break;
  default: chance = 0; break;
  }

  if ( chance == 0 ) {
    send_to_char("This spell is useless in this kind of environment.\n\r", ch );
    return;
  }

  if ( chance < number_percent() ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  // at higher casting_level, more chance for target being rooted
  if ( !is_affected( victim, sn )
       && !IS_AFFECTED( victim, AFF_ROOTED )
       && !is_affected( victim, gsn_entangle )
       && number_percent() < (get_ability(ch,sn)*casting_level)/4) {
    send_to_char("{gRoots{x comes from the ground and immobilise your legs.{x\n\r", victim );
    act("{gRoots{x comes from the ground and immobilise $N's legs.{x", ch, NULL, victim, TO_ROOM );
    act("{gRoots{x comes from the ground and immobilise $N's legs.{x", ch, NULL, victim, TO_CHAR );
    
    int timer = number_range( 2*casting_level, 4*casting_level );
    //afsetup( af, CHAR, affected_by, OR, AFF_ROOTED, timer, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,timer,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected_by,OR,AFF_ROOTED);
    affect_to_char( victim, &af );
  }

  // does minor damage, but more damage at high casting_level
  int dam = (UMAX( dice( level/2, level/15 ), 10*casting_level )*casting_level*2)/3;
  ability_damage( ch, victim, dam, sn, DAM_BASH, TRUE, TRUE );
  return;
}

void spell_hypnotism( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA*)vo;
  AFFECT_DATA af;

  if (IS_NPC(victim) 
      && (IS_SET(victim->cstat(imm_flags),IRV_MAGIC) 
	  || IS_SET(victim->act,ACT_UNDEAD)
	  || IS_SET(victim->cstat(form),FORM_UNDEAD))) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  if (IS_AFFECTED(victim,AFF_CALM) 
      || IS_AFFECTED(victim,AFF_BERSERK)
      || is_affected(victim,gsn_frenzy)) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }
	    
  act("You are hypnotised by $n.",ch,NULL,victim,TO_VICT);
  act("You have hypnotised $N.", ch, NULL, victim, TO_CHAR );

  if (victim->fighting || victim->position == POS_FIGHTING)
    stop_fighting(victim,FALSE);

  //afsetup(af,CHAR,affected_by,OR,AFF_CALM,level/4,level,sn);
  //affect_to_char(victim,&af);
  createaff(af,level/4,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CALM);
  affect_to_char( victim, &af );
}

void spell_globe_invuln(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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
  createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,saving_throw,ADD,-level/10);
  affect_to_char( victim, &af );

  act( "$n is surrounded by an {BHazy{x aura.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by a {BHazy{x aura.\n\r", victim );
  return;
}

void spell_mind_blast(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim       = (CHAR_DATA *) vo;
  static const int        dam_each [ ] = {
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
  dam      = number_range( dam_each[level], dam_each[level] * 3);
  if ( saves_spell( level, victim, DAM_MENTAL ) && casting_level < 3 )
    dam /= 2;
  dam *= casting_level;

  ability_damage( ch, victim, dam, sn, DAM_MENTAL, TRUE, TRUE );
  return;
}

void spell_mind_blade( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *blade;
  AFFECT_DATA af;

  blade = create_object( get_obj_index( OBJ_VNUM_MIND_BLADE ), 0 );
  blade->timer = level;
  blade->baseval[1] = UMAX(level/5,5);
  blade->baseval[2] = number_fuzzy(8);

  SET_BIT( blade->extra_flags, ITEM_NODROP );

  blade->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,INT,ADD,4,level,level,sn);
  //affect_to_obj(blade,&af);
  //afsetup(af,CHAR,saving_throw,ADD,0-number_fuzzy(UMAX(level/10,3)),level,level,sn);
  //affect_to_obj(blade,&af);
  //afsetup(af,CHAR,max_mana,ADD,level*3,level,level,sn);
  //affect_to_obj(blade,&af);
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,saving_throw,ADD,0-number_fuzzy(UMAX(level/10,3)));
  addaff(af,CHAR,INT,ADD,4);
  addaff(af,CHAR,max_mana,ADD,level*3);
  affect_to_obj( blade, &af );

  recompobj(blade);

  obj_to_char( blade, ch );
  act( "$n has created $p.", ch, blade, NULL, TO_ROOM );
  act( "You have created $p.", ch, blade, NULL, TO_CHAR );
  return;
}


void spell_mirror_image( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *clone;
  CHAR_DATA *parent = ch;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ( ch->pet != NULL ) {
    send_to_char("You already have a familiar.\n\r", ch );
    return;
  }

  clone = create_mobile( get_mob_index(MOB_VNUM_MIRROR_IMAGE) );
  /* start fixing values */
  for (int i = 0; i < ATTR_NUMBER; i++)
    clone->baseattr[i] = parent->baseattr[i];

  clone->name        	= str_dup("mirror image");
  clone->level	        = parent->level;
  clone->hit		= parent->hit;
  clone->mana		= parent->mana;
  clone->psp		= parent->psp;
  clone->move		= parent->move;
  clone->gold		= 0;
  clone->silver	        = 0;
  clone->act		= ACT_PET | ACT_IS_NPC;

  AFFECT_DATA *paf_next;
  for (AFFECT_DATA *paf = clone->affected; paf != NULL; paf = paf_next) {
    paf_next = paf->next;
    affect_remove(clone,paf);
  }
  
  /* now add the affects */
  for (AFFECT_DATA *paf = parent->affected; paf != NULL; paf = paf->next)
    affect_to_char(clone,paf);

  for (OBJ_DATA *obj = parent->carrying; obj != NULL; obj = obj->next_content) {
    OBJ_DATA *new_obj = create_object(obj->pIndexData,0);
    clone_object(obj,new_obj);
    SET_BIT( new_obj->base_extra, ITEM_NODROP|ITEM_NOREMOVE|ITEM_NOUNCURSE ); 
    SET_BIT( new_obj->extra_flags, ITEM_NODROP|ITEM_NOREMOVE|ITEM_NOUNCURSE ); 
    recursive_clone2(obj,new_obj);
    obj_to_char(new_obj,clone);
    new_obj->wear_loc = obj->wear_loc;
  }

  char buf[MAX_STRING_LENGTH];
  sprintf(buf, clone->short_descr, parent->name );
  clone->short_descr = str_dup( buf );

  sprintf(buf, clone->long_descr, parent->name );
  clone->long_descr = str_dup( buf );

  sprintf(buf, clone->description, parent->name );
  clone->description = str_dup( buf );

  add_follower( clone, ch );
  clone->leader = ch;
  ch->pet = clone;
  SET_BIT(clone->act, ACT_PET);
  SET_BIT(clone->bstat(affected_by), AFF_CHARM);
  SET_BIT(clone->cstat(affected_by), AFF_CHARM);

  SET_BIT(clone->act, ACT_CREATED ); // SinaC 2003

  char_to_room(clone,parent->in_room);
  act("$n has created $s mirror image.",ch,NULL,clone,TO_ROOM);
  act("You create your mirror image.",ch,NULL,clone,TO_CHAR);

  //recompute(clone); NO NEED: done in char_to_room

  return;
}

void spell_water_to_wine( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  bool wine = FALSE;
  bool water = FALSE;

#define containsWINE(v) ( (v) == 2 || (v) == 17 || (v) == 18 || (v) == 21 || (v) == 22 || (v) == 30 )

  if ( obj->item_type != ITEM_DRINK_CON ) {
    send_to_char( "It is not a drink container.\n\r", ch );
    return;
  }

  wine = containsWINE( obj->value[2] );
  water = obj->value[2] == LIQ_WATER;

  if ( !wine && !water ) {
    send_to_char( "It contains another liquid then wine or water.\n\r", ch );
    return;
  }
  
  if ( wine ) {
    obj->baseval[2] = 0;
    send_to_char("You changed wine into water.\n\r", ch );
    act("$n changed wine into water.",ch,NULL,NULL,TO_ROOM);
  } 
  else {
    obj->baseval[2] = 2;
    send_to_char("You changed water into wine.\n\r", ch );
    act("$n changed water into wine.",ch,NULL,NULL,TO_ROOM);
  }
  recompobj(obj);

  return;
}

void spell_death_fog(int sn, int level, CHAR_DATA *ch,void *vo, int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;
  AFFECT_DATA af;

  send_to_char("You create a poisonous cloud of fog.\n\r",ch);
  act("$n creates a poisonous cloud of fog!",ch,NULL,NULL,TO_ROOM);

  dam = dice(level, 5) * casting_level;
  if ( casting_level == 5 )
    dam *= 2;

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    if ( !saves_spell( level, vch, DAM_POISON )
	 && chance(5*casting_level) 
	 && casting_level >= 4 ) {
      int dur = level * casting_level;
      if ( casting_level == 5 )
	dur *= 2;
      int cast_lvl = UMAX( casting_level-3, 1 );
      
      //newafsetup(af,CHAR,affected_by,OR,AFF_POISON,dur,level,sn,cast_lvl);
      //affect_join( vch, &af );
      //newafsetup(af,CHAR,STR,ADD,-2*casting_level,dur,level,sn,cast_lvl);
      //affect_join( vch, &af );
      createaff(af,dur,level,sn,cast_lvl,AFFECT_ABILITY);
      addaff(af,CHAR,STR,ADD,-2*casting_level);
      addaff(af,CHAR,affected_by,OR,AFF_POISON);
      affect_join( vch, &af );

      send_to_char( "You feel very sick.\n\r", vch );
      act("$n looks very ill.",vch,NULL,NULL,TO_ROOM);
    }

    if ( casting_level == 5 )
      for ( int i = 0; i < number_fuzzy(3); i++ )
	if ( saves_spell( level, vch, DAM_POISON )
	     && casting_level < 3 )
	  ability_damage( ch, vch, dam/6, sn, DAM_POISON ,TRUE, TRUE);
	else
	  ability_damage( ch, vch, dam/3, sn, DAM_POISON ,TRUE, TRUE);
    else 
      if ( saves_spell( level, vch, DAM_POISON )
	   && casting_level < 3 )
	ability_damage( ch, vch, dam/2, sn, DAM_POISON ,TRUE, TRUE);
      else
	ability_damage( ch, vch, dam, sn, DAM_POISON ,TRUE, TRUE);
  }
  return;
}

/*
void spell_animate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if ( obj->item_type == ITEM_CONTAINER
       || obj->item_type == ITEM_DRINK_CON 
       //|| find_suitable_wearloc( obj ) <= -1 ) {
       || !IS_SET( obj->wear_flags, ITEM_HOLD ) ) {
    act("You can't animate that $p.", ch, obj, NULL, TO_CHAR );
    return;
  }

  if ( ch->pet != NULL ) {
    send_to_char("You already have a familiar.\n\r", ch );
    return;
  }

  int chance;
  if (ch->level < obj->level) {
    chance += (2*ch->level);
    chance -= (3*obj->level);
  }

  chance = URANGE(10,chance,90);

  if (number_percent() > chance) {
    act("You fail and destroy $p",ch,obj,NULL,TO_CHAR);
    act("$n tries to animate $p but destroys it.",ch,obj,NULL,TO_ROOM);
    extract_obj(obj);
    return;
  }

  CHAR_DATA *mob = create_mobile( get_mob_index( MOB_VNUM_ANIM_OBJ ) );
  mob->level = obj->level;
  mob->bstat(max_hit) = UMAX((dice(mob->level, 15))+(mob->level*20),1);
  mob->hit = mob->bstat(max_hit);
  mob->bstat(DICE_TYPE) = UMAX(1, mob->level/3);
  if ( mob->level >= 1  && mob->level <= 10 )
    mob->bstat(DICE_NUMBER) = 4;
  if ( mob->level >= 11 && mob->level <= 20 )
    mob->bstat(DICE_NUMBER) = 5;
  if ( mob->level >= 21 && mob->level <= 30 )
    mob->bstat(DICE_NUMBER) = 6;
  if ( mob->level > 30 )
    mob->bstat(DICE_NUMBER) = 7;

  mob->bstat(alignment) = 0;
  mob->bstat(etho) = ETHO_NEUTRAL;
  
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  sprintf( buf1, "(Animated) %s", obj->short_descr );
  sprintf( buf2, "(Animated) %s is standing here.\n\r", obj->short_descr );
  sprintf( buf, "%s %s", mob->name, obj->name );
 
  mob->short_descr = str_dup(buf1);
  mob->long_descr = str_dup(buf2);
  mob->name = str_dup( buf );

  act("$n utters an incantation and $p starts to move.",ch,obj,NULL,TO_ROOM);
  act("$p starts to move!",ch,obj,NULL,TO_ROOM);
  act("$p starts to move.",ch,obj,NULL,TO_CHAR);

  char_to_room( mob, ch->in_room);
  add_follower( mob, ch );
  mob->leader = ch;
  ch->pet = mob;
  SET_BIT(mob->act, ACT_PET);
  SET_BIT(mob->bstat(affected_by), AFF_CHARM);
  SET_BIT(mob->cstat(affected_by), AFF_CHARM);

  recompute(mob);
  extract_obj(obj);
}
*/

void spell_animate_object( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *) vo;
  int hp = 1, damroll = 1, move = 1, hitroll = 1;
  int ilevel, type, chance, ac, acm;
  bool found = FALSE;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ( ch->pet != NULL ) {
    send_to_char("You already have a familiar.\n\r", ch );
    return;
  }

  if (obj->wear_loc != WEAR_NONE) {
    send_to_char("You can only animate objects you are carrying.\n\r",ch);
    return;
  }
  ilevel = obj->level;

  type = obj->item_type;
  if (type != ITEM_WEAPON
      && type != ITEM_ARMOR
      && type != ITEM_KEY
      && type != ITEM_FURNITURE) {
    send_to_char("You can't animate that kind of object.\n\r",ch);
    return;
  }
  if (type == ITEM_ARMOR 
      && ( !IS_SET(obj->wear_flags,ITEM_WEAR_BODY)
	   && !IS_SET(obj->wear_flags,ITEM_WEAR_HANDS)
	   && !IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD) ) ) {
    send_to_char("You can only animate armor that is body, hands, or shields.\n\r",ch);
    return;
  }

  chance = get_ability( ch, sn );
  chance = URANGE(2, chance + (level - ilevel)*3, 95);

  if (number_percent() > chance) {
    act("$p shudders for a moment then flares up and vaporises!",ch,obj,0,TO_ROOM);
    act("$p shudders for a moment then flares up and vaporises!",ch,obj,0,TO_CHAR);
    extract_obj(obj);
    return;
  }

  CHAR_DATA *mob = create_mobile( get_mob_index( MOB_VNUM_ANIM_OBJ ) );
  mob->level = obj->level;
  mob->bstat(alignment) = 0;
  mob->bstat(etho) = ETHO_LAWFUL;
  
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  sprintf( buf1, "(Animated) %s", obj->short_descr );
  sprintf( buf2, "(Animated) %s is standing here.\n\r", obj->short_descr );
  sprintf( buf, "%s %s", mob->name, obj->name );
 
  mob->short_descr = str_dup(buf1);
  mob->long_descr = str_dup(buf2);
  mob->name = str_dup( buf );

  if (type == ITEM_ARMOR) {
    hp = (50*ilevel);
    ac = (50 - 5*ilevel);
    move = ilevel*10;
    acm = (-ilevel*2);
    damroll = ilevel/3;
    if (IS_SET(obj->wear_flags,ITEM_WEAR_BODY))
      SET_BIT(mob->off_flags,OFF_BASH);
    if (IS_SET(obj->wear_flags,ITEM_WEAR_HANDS)
        || IS_SET(obj->wear_flags,ITEM_WEAR_SHIELD))
      SET_BIT(mob->off_flags,OFF_PARRY);
  } 
  else if (type == ITEM_FURNITURE) {
    hp = (40*ilevel);
    ac = (100 - 4*ilevel);
    move = ilevel;
    acm = -(ilevel);
    damroll = ilevel/4;
    hitroll = damroll;
    SET_BIT(mob->off_flags,OFF_BASH);
  } 
  else if (type == ITEM_KEY) {
    hp = ilevel;
    ac = 100;
    move = 400;
    acm = 100;
    damroll = 0;
    hitroll = 5;
  } 
  else  {
    hp = ilevel*25;
    ac = 100 - (4*ilevel);
    acm = 100 - (4*ilevel);
    move = ilevel*3;
    damroll = (ilevel);
    hitroll = damroll;
  }
  mob->bstat(max_hit) = hp;
  mob->hit = mob->bstat(max_hit);
  mob->bstat(max_move) = move;
  mob->move = mob->bstat(max_move);
  mob->bstat(DICE_TYPE) = UMAX(1, mob->level/3);
  if ( mob->level >= 1  && mob->level <= 10 )
    mob->bstat(DICE_NUMBER) = 4;
  if ( mob->level >= 11 && mob->level <= 20 )
    mob->bstat(DICE_NUMBER) = 5;
  if ( mob->level >= 21 && mob->level <= 30 )
    mob->bstat(DICE_NUMBER) = 6;
  if ( mob->level > 30 )
    mob->bstat(DICE_NUMBER) = 7;

  mob->bstat(ac0+AC_PIERCE) = mob->cstat(ac0+AC_PIERCE) =
    mob->bstat(ac0+AC_BASH) = mob->cstat(ac0+AC_BASH) =
    mob->bstat(ac0+AC_SLASH) = mob->cstat(ac0+AC_SLASH) = acm;
  mob->bstat(ac0+AC_EXOTIC) = mob->cstat(ac0+AC_EXOTIC) = acm/2;
  mob->bstat(hitroll) = hitroll;
  mob->bstat(damroll) = damroll;

  add_follower( mob, ch );
  mob->leader = ch;
  ch->pet = mob;
  SET_BIT(mob->act, ACT_PET);
  SET_BIT(mob->bstat(affected_by), AFF_CHARM);
  SET_BIT(mob->cstat(affected_by), AFF_CHARM);
  SET_BIT(mob->act, ACT_CREATED ); // SinaC 2003

  act("$p shudders and slowly rises into the air to follow $n!",ch,obj,0,TO_ROOM);
  act("$p shudders for a moment then slowly rises up beside you!",ch,obj,0,TO_CHAR);
  char_to_room( mob, ch->in_room);

  //  recompute(mob); NO NEED: done in char_to_room
  extract_obj(obj);
  return;
}


void spell_antigravity(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( ch->in_room->cstat(sector) == SECT_AIR
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char("You can't do that in this environment.\n\r",ch);
    return;
  }

  int dam = dice( level, 5 ) * casting_level;
  if ( saves_spell( level, ch, DAM_BASH ) )
    dam /= 2;
  if ( casting_level == 5 )
    dam *= 2;

  act("You change the gravitational field surrounding $N.", ch, NULL, victim, TO_CHAR );
  act("$N shoot upwards and immediatly crash back down to earth.", ch, NULL, victim, TO_NOTVICT );
  act("You suddenly shoot upwards, then immediatly crash back down to earth.",ch,NULL,victim,TO_VICT);
  ability_damage( ch, (CHAR_DATA *) vo, dam, sn, DAM_BASH, TRUE, TRUE );
}

void spell_temporal_stasis(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( ch->fighting == NULL ) {
    send_to_char("You need to be fighting in order to use this spell.\n\r", ch );
    spell_failed = TRUE;
    return;
  }

  if ( is_affected( ch, sn ) || is_affected( victim, sn ) ) {
    send_to_char("The timelines are not yet realigned.\n\r",ch);
    return;
  }

  if ( saves_spell( level, victim, DAM_CHARM ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  act("You create a separate time field around $N.", ch, NULL, victim, TO_CHAR );
  act("$n has stased you.", ch, NULL, victim, TO_VICT );
  act("$n has created a separate time field around $N.",ch,NULL,victim, TO_NOTVICT );

  //  victim gets  ADD affect
  //  caster gets  OR  affect

  // Victim
  int dur = 1;
  if ( casting_level == 5 )
    dur = 3;
  //afsetup(af,CHAR,NA,ADD,0,dur,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,NA,ADD,0);
  affect_to_char( victim, &af );

  victim->position = POS_PARALYZED;

  // Caster
  int dur2 = 23 - 3*casting_level; // 1: 20  2:17  3:14  4:11  5:4
  if ( casting_level == 5 )
    dur2 = 4; // more than 3
  //afsetup(af,CHAR,NA,OR,0,dur2,level,sn);
  //affect_to_char( ch, &af );
  createaff(af,dur2,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,NA,OR,0);
  affect_to_char( ch, &af );

  update_pos( victim );
}

void spell_command_undead(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  
  if (is_safe(ch,victim)) 
    return;

  if ( victim == ch ) {
    send_to_char( "You like yourself even better!\n\r", ch );
    return;
  }

  if ( !IS_NPC(victim)
       || IS_AFFECTED(victim, AFF_CHARM)
       || IS_AFFECTED(ch, AFF_CHARM)
       || level < victim->level
       || IS_SET(victim->cstat(imm_flags),IRV_CHARM)
       || saves_spell( level, victim,DAM_CHARM) 
       || !IS_SET( victim->cstat(form), FORM_UNDEAD) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  if (IS_SET(victim->in_room->cstat(flags),ROOM_LAW)
      && !IS_IMMORTAL(ch)) {
    send_to_char("The mayor does not allow charming in the city limits.\n\r",ch);
    return;
  }

  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );
  victim->leader = ch;

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy( level / 4 ),level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( victim, &af );

  act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
  if ( ch != victim )
    act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_flamevision(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("You are already affected by flamevision.\n\r",ch);
    else
      act("$N is already affected by flamevision.", ch, NULL, victim, TO_CHAR );
    return;
  }

  // dark, hidden, invis, magic, good, evil
  int dur = level;
  //afsetup(af,CHAR,affected_by,OR,AFF_INFRARED,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_INVIS,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_GOOD,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_DETECT_EVIL,dur,level,sn);
  //affect_to_char( ch, &af );
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_INFRARED);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_EVIL);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_GOOD);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_MAGIC);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_INVIS);
  addaff(af,CHAR,affected_by,OR,AFF_DETECT_HIDDEN);
  affect_to_char( victim, &af );

  act("$n's eyes glow.", ch, NULL, NULL, TO_ROOM );
  send_to_char("Your eyes glow.\n\r",victim);
}

void spell_stone_fist(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, gsn_iron_hand )
       || is_affected( victim, gsn_stone_fist ) ) {
    if ( ch == victim )
      send_to_char( "Your fists are already as hard as stone.\n\r", ch );
    else
      act("$N's fists are already as hard as stone.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, NA, ADD, 0, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( victim, &af );

  send_to_char("Your fists turn into stone.\n\r", victim );
  act("$n's fists turn into stone.\n\r", ch, NULL, NULL, TO_ROOM );
  
  return;
}

void spell_hydroblast( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice( level, 12 );

  // added by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim, DAM_DROWNING ) && casting_level < 3 )
    dam /= 2;

  bool done = FALSE;
  if ( casting_level == 5 )
    for ( int i = 0; i < number_fuzzy(3); i++ ) {
      int damdone = ability_damage( ch, victim, dam/3, sn, DAM_DROWNING, TRUE, TRUE);
      if ( damdone == DAMAGE_DEADLY )
	return;
      done |= damdone;
    }
  else {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_DROWNING, TRUE, TRUE);
      if ( damdone == DAMAGE_DEADLY )
	return;
      done |= damdone;
  }
  if( done && casting_level >= 4 )
    acid_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_elemental_field(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
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
  //afsetup(af,CHAR,damroll,ADD,level/8,level/6,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,damroll,ADD,level/8);
  affect_to_char( victim, &af );

  act( "$n is surrounded by an elemental field.", victim, NULL, NULL, TO_ROOM );
  send_to_char( "You are surrounded by an elemental field.\n\r", victim );
  return;
}

void spell_immolation( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
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

  int done = FALSE;
  if ( casting_level == 5 && chance(15))
    done = ability_damage( ch, victim, dam*2, sn, DAM_FIRE ,TRUE, TRUE);
  else
    done = ability_damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);

  if( done == DAMAGE_DONE && casting_level >= 4 )
    fire_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_cone_of_cold( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  dam = dice( level, 12 );

  // added by SinaC 2001 for spell level
  dam          *= casting_level;
  if ( casting_level == 5 )
    dam        *= 2;
  if ( saves_spell( level, victim, DAM_COLD ) && casting_level < 3 )
    dam /= 2;

  bool done = FALSE;
  if ( casting_level == 5 )
    for ( int i = 0; i < number_fuzzy(3); i++ ) {
      int damdone = ability_damage( ch, victim, dam/3, sn, DAM_COLD, TRUE, TRUE);
      if ( damdone == DAMAGE_DEADLY )
	return;
      done |= damdone;
    }
  else {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_COLD, TRUE, TRUE);
    if ( damdone == DAMAGE_DEADLY )
      return;
    done |= damdone;
  }
  if( done && casting_level >= 4 )
    acid_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_shrieking_blades(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam, spins;
  int i;

  dam = dice(level, 3*casting_level);
  spins = number_range(2,UMAX(level/20,3))+casting_level;
  if ( casting_level == 5 )
    spins += 3;

  act("You create a whirlwind of spinning blades to strike down $N!",ch,0,victim,TO_CHAR);
  act("$n creates a deadly blade barrier that tears into $N!",ch,0,victim,TO_NOTVICT);
  act("$n creates a deadly blade barrier that tears into you!",ch,0,victim,TO_VICT);
  ability_damage(ch,victim,dam,sn,DAM_SLASH,TRUE,TRUE);
  
  for (i = 0; i < spins; i++) {
    if (victim->in_room != ch->in_room) return;
    
    if (saves_spell(level,victim,DAM_SLASH))
      dam /= 2;
    
    dam *= 3;
    dam /= 4;
    act("The blades spin and slice away at $n.",victim,0,0,TO_ROOM);
    act("The blades spin and slice away at you.",victim,0,0,TO_CHAR);
    
    int done = ability_damage(ch,victim,dam,sn,DAM_SLASH,TRUE,TRUE);
    if ( done == DAMAGE_DONE && casting_level == 5 && chance(80) ) { // special effect
      switch ( number_range(0,4) ) {
      case 0: fire_effect( (void *) victim, level/2, dam, TARGET_CHAR ); break;
      case 1: cold_effect( (void *) victim, level/2, dam, TARGET_CHAR ); break;
      case 2: shock_effect( (void *) victim, level/2, dam, TARGET_CHAR ); break;
      case 3: acid_effect( (void *) victim, level/2, dam, TARGET_CHAR ); break;
      case 4: poison_effect( (void *) victim, level/2, dam, TARGET_CHAR ); break;
      default: fire_effect( (void *) victim, level/2, dam, TARGET_CHAR ); break;
      }
    }
  }

  act("The blade barrier fades away.",ch,0,0,TO_ROOM);
  act("Your blade barrier fades away.",ch,0,0,TO_CHAR);
  return;
}

void spell_full_heal(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  if (ch->fighting != NULL) {
    send_to_char("You are too occupied to full heal anyone.{x\n\r",ch);
    return;
  }
  
  if ( victim->fighting != NULL) {
    send_to_char("You cannot full heal someone in combat!{x\n\r",ch);
    return;
  }

  send_to_char( "Godly essence flows through your body knitting your wounds!\n\r", victim );
  send_to_char( "You call divine power to heal!\n\r",ch);
  
  affect_strip(victim,gsn_plague);
  affect_strip(victim,gsn_poison);
  affect_strip(victim,gsn_weaken);
  affect_strip(victim,gsn_blindness);
  affect_strip(victim,gsn_sleep);
  affect_strip(victim,gsn_curse);
  
  victim->hit        = victim->cstat(max_hit);
  victim->move       = victim->cstat(max_move);
  update_pos(victim);
}

void spell_slumber( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_SLEEP)
       || (IS_NPC(victim) 
	   && ( IS_SET(victim->act,ACT_UNDEAD)
		|| IS_SET( victim->cstat(form),FORM_UNDEAD) 
		|| IS_SET(victim->act, ACT_NOSLEEP ) ) )
       || (level + 2) < victim->level
       || saves_spell( level-4, victim,DAM_CHARM) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  int dur = 4+level;

  //afsetup(af,CHAR,affected_by,OR,AFF_SLEEP,dur,level,sn);
  //affect_join( victim, &af );
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

void spell_darkvision( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_DARK_VISION) ) {
    if (victim == ch)
      send_to_char("You can already see in the dark.\n\r",ch);
    else
      act("$N already has infravision.\n\r",ch,NULL,victim,TO_CHAR);
    return;
  }
  act( "$n's eyes glow red.\n\r", ch, NULL, NULL, TO_ROOM );

  //afsetup(af,CHAR,affected_by,OR,AFF_DARK_VISION,2*level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,2*level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_DARK_VISION);
  affect_to_char( victim, &af );

  send_to_char( "Your eyes glow red.\n\r", victim );
  return;
}

void spell_cloud_of_darkness( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  AFFECT_DATA af;
  CHAR_DATA *vch, *vch_next;

  send_to_char("You summon an enveloping {Dcloud of darkness{x.\n\r", ch );
  act("$n summons an enveloping {Dcloud of darkness{x.", ch, NULL, NULL, TO_ROOM );
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
 
    if ( IS_AFFECTED(vch,AFF_DARK_VISION ) ) // unaffected if affected by dark vision
      continue;
    
    if ( is_affected( vch, sn )  // unaffected if already affected by slow
	 || IS_AFFECTED(vch,AFF_SLOW))
      continue;
    
    if (saves_spell(level,vch,DAM_OTHER) // if saves spell   or  immune to magic
	||  IS_SET(vch->cstat(imm_flags),IRV_MAGIC))
      continue;
    
    if (IS_AFFECTED(vch,AFF_HASTE) // remove haste if affected by haste
	&& check_dispel(level,vch,gsn_haste)) {
      act("$n is moving less quickly.",vch,NULL,NULL,TO_ROOM);
      continue;
    }
    
    
    //afsetup(af,CHAR,affected_by,OR,AFF_SLOW,level/2,level,sn);
    //affect_to_char( vch, &af );
    //afsetup(af,CHAR,DEX,ADD,-1 - (level >= 18) - (level >= 25) - (level >= 32),
    //	    level/2,level,sn);
    //affect_to_char( vch, &af );
    createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected_by,OR,AFF_SLOW);
    addaff(af,CHAR,DEX,ADD,-1 - (level >= 18) - (level >= 25) - (level >= 32));
    affect_to_char( vch, &af );
    
    send_to_char( "You feel yourself slowing d o w n...\n\r", vch );
    act("$n starts to move in slow motion.",vch,NULL,NULL,TO_ROOM);
  }
  return;
}

void spell_tsunami(int sn,int level,CHAR_DATA *ch,void *vo, int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  if (ch->in_room->cstat(sector) != SECT_WATER_SWIM
      && ch->in_room->cstat(sector) != SECT_WATER_NOSWIM) {
    send_to_char("You need to be on water to do that.\n\r",ch);
    return;
  }

  act("$n raises $s arms and causes the waters to rise up in violence.",ch,NULL,NULL,TO_ROOM);
  send_to_char("You call upon the water around you to surge at your foes.\n\r",ch);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    dam = dice(level, 11);
    if( saves_spell( level, vch, DAM_DROWNING ) )
      dam /= 2;
    ability_damage(ch,vch,dam,sn,DAM_DROWNING,TRUE,TRUE);
  }
  act("$n's tsunami washes past and subsides.",ch,NULL,NULL,TO_ROOM);
  send_to_char("Your tsunami washes past and subsides.\n\r",ch);

  return;
}

void spell_shroud( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char("You are already engulfed by {Dshroud{x.\n\r", ch );
    else
      act("$N is already engulfed by {Dshroud{x.", ch, NULL, victim, TO_CHAR );
    return;
  }

  // Modified by SinaC 2001
  // shroud will absorb between three and six attack
  //afsetup( af, CHAR, NA, ADD, 0, (level*3)/2, number_range( 3, 6 ), sn );
  
  //afsetup( af, CHAR, NA, ADD, 0, (level*3)/2, level, sn );
  //af.casting_level = number_range( 3, 6 );
  //affect_to_char( ch, &af );

  //createaff(af,(level*3)/2,level,sn,number_range( 3, 6 ),AFFECT_ABILITY);
  // level gives amount of damage shroud can absorbed before fading away
  createaff(af,(level*3)/2,level*number_range(3,6)*number_range(5,level/10),sn,number_range(3,6),AFFECT_ABILITY);
  affect_to_char( victim, &af );

  send_to_char("You are engulfed by an {Devil dark shroud{x.\n\r", victim );
  if ( ch != victim )
    send_to_char("Ok.\n\r", ch );
  return;
}

void spell_sacred_mists(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char( "Your are already tuned with ancient songs of the seas.\n\r", ch );
    else
      act("$N is already tuned with ancient songs of the seas.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, affected2_by, OR, AFF2_WATER_BREATH, level/8, level, sn ); 
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_COLD, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_FIRE, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, res_flags, OR, IRV_DROWNING, level/8, level, sn );
  //affect_to_char( ch, &af );
  //afsetup(af, CHAR, vuln_flags, OR, IRV_LIGHTNING, level/8, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level/8,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_WATER_BREATH);
  addaff(af,CHAR,res_flags,OR,IRV_COLD);
  addaff(af,CHAR,res_flags,OR,IRV_FIRE);
  addaff(af,CHAR,res_flags,OR,IRV_DROWNING);
  addaff(af,CHAR,vuln_flags,OR,IRV_LIGHTNING);
  affect_to_char( victim, &af );

  send_to_char("You are surrounded with an aura of misting energy.\n\r", ch );
  act("$n is tuned with ancient songs of the seas.", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_ride_the_winds( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED(victim, AFF_FLYING) ) {
    if (victim == ch)
      send_to_char("You are already airborne.\n\r",ch);
    else
      act("$N doesn't need your help to fly.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup( af, CHAR, max_move, ADD, level*2, level+3, level, sn );
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_FLYING,level+3,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,3+level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_FLYING);
  addaff(af,CHAR,max_move,ADD,level*3);
  affect_to_char( victim, &af );

  send_to_char( "Your ride on the winds.\n\r", victim );
  act( "$n rides on the winds.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_snowy_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] =   {
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

  act("You conjure a blast of snow and ice to strike $N.", ch, NULL, victim, TO_CHAR );
  act("$n conjure a blast of snow and ice to strike $N.", ch, NULL, victim, TO_NOTVICT );
  act("$n conjure a blast of snow and ice to strike you.", ch, NULL, victim, TO_VICT );

  level	= UMIN(level, ( int)(sizeof(dam_each)/sizeof(dam_each[0]) - 1));
  level	= UMAX(0, level);
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 2 ), 2 );

  if ( saves_spell( level, victim, DAM_COLD) )
    dam /= 2;
  int done = ability_damage( ch, victim, dam, sn, DAM_COLD ,TRUE, TRUE);

  if ( done == DAMAGE_DONE && chance(20) && !saves_spell( level, victim, DAM_COLD ) ) 
    cold_effect( (void *) victim, level/2, dam, TARGET_CHAR );
  return;
}

void spell_battle_frenzy( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *vch, *vch_next;
  AFFECT_DATA af;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( !is_same_group( vch, ch ) )
      continue;

    if ( is_affected(vch,gsn_frenzy)
	 || IS_AFFECTED(vch,AFF_BERSERK))
      continue;

    if (is_affected(vch,gsn_calm)
	|| IS_AFFECTED(vch,AFF_CALM) )
      continue;

    //afsetup(af,CHAR,hitroll,ADD,level/6,level/3,level,gsn_frenzy);
    //affect_to_char(vch,&af);
    //af.location  = ATTR_damroll;
    //affect_to_char(vch,&af);
    createaff(af,level/3,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,hitroll,ADD,level/6);
    addaff(af,CHAR,damroll,ADD,level/6);
    affect_to_char( vch, &af );

    send_to_char("You are filled with a battle frenzy!\n\r",vch);
    act("$n gets a wild look in $s eyes!",vch,NULL,NULL,TO_ROOM);
  }
}

void spell_battle_fury(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char( "You are already furious as you could be.\n\r", ch );
    else
      act("$N is already furious as $e could be.", ch, NULL, victim, TO_CHAR );
    return;
  }

  int dur = 5;
  //afsetup(af,CHAR,hitroll,ADD,level/8,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,damroll,ADD,level/8,dur,level,sn);
  //affect_to_char( ch, &af );
  //afsetup(af,CHAR,allAC,ADD,-20-level/4,dur,level,sn);
  //affect_to_char(ch,&af);
  //newafsetup(af,CHAR,affected_by,OR,AFF_HASTE,dur,level,sn,3);
  //affect_to_char( ch, &af );
  createaff(af,dur,level,sn,3,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,level/8);
  addaff(af,CHAR,damroll,ADD,level/8);
  addaff(af,CHAR,allAC,ADD,-20-level/4);
  addaff(af,CHAR,affected_by,OR,AFF_HASTE);
  affect_to_char( victim, &af );

  send_to_char("Gorack turns you into a ferocious battler.\n\r", victim );
  act("$n gets a wild look in $s eyes.",ch,NULL,NULL,TO_ROOM);
  return;
}

void spell_friendship( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA*)vo;
  AFFECT_DATA af;

  if (IS_NPC(victim)
      && (IS_SET(victim->cstat(imm_flags),IRV_MAGIC) 
	  || IS_SET(victim->act,ACT_UNDEAD)
	  || IS_SET(victim->cstat(form),FORM_UNDEAD))) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  if (IS_AFFECTED(victim,AFF_CALM) 
      || IS_AFFECTED(victim,AFF_BERSERK)
      || is_affected(victim,gsn_frenzy)) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }
	    
  act("You become friend with $n.",ch,NULL,victim,TO_VICT);
  act("You have befriended $N.", ch, NULL, victim, TO_CHAR );

  if (victim->fighting || victim->position == POS_FIGHTING)
    stop_fighting(victim,FALSE);

  //afsetup(af,CHAR,affected_by,OR,AFF_CALM,level/4,level,sn);
  //affect_to_char(victim,&af);
  createaff(af,level/4,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CALM);
  affect_to_char( victim, &af );
}

void spell_seduction( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
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
       || level < victim->level-5 // easier than charm person
       || IS_SET(victim->cstat(imm_flags),IRV_CHARM)
       || saves_spell( level-5, victim,DAM_CHARM) ) { // easier than charm person
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  if (IS_SET(victim->in_room->cstat(flags),ROOM_LAW)
      && !IS_IMMORTAL(ch)) {
    send_to_char("The mayor does not allow charming in the city limits.\n\r",ch);
    return;
  }
  
  if ( victim->master )
    stop_follower( victim );
  add_follower( victim, ch );
  victim->leader = ch;

  //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,number_fuzzy( level / 4 ),level,sn);
  //affect_to_char( victim, &af );
  createaff(af,number_fuzzy(level/4),level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected_by,OR,AFF_CHARM);
  affect_to_char( victim, &af );

  act( "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
  if ( ch != victim )
    act("$N looks at you with adoring eyes.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_find_path( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int direction;
  bool fArea;

  victim = get_char_world( ch, target_name );
 
  if( victim == NULL 
      || !can_see( ch, victim ) ) {
    send_to_char("No-one around by that name.\n\r", ch );
    return;
  }

  if( ch->in_room == victim->in_room ) {
    act( "$N is here!", ch, NULL, victim, TO_CHAR );
    return;
  }

  if ( ch->in_room->area != victim->in_room->area ) {
    act("You couln't find a path to $N from here.", ch, NULL, victim, TO_CHAR );
    return;
  }

  direction = find_path( ch->in_room->vnum, victim->in_room->vnum,
			 ch, -40000, TRUE, NULL );

  if( direction == -1 ) {
    act( "You couldn't find a path to $N from here.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }

  if( direction < 0 || direction > 5 ) {
    send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
    return;
  }

  sprintf( buf, "$N is %s from here.", dir_name[direction] );
  act( buf, ch, NULL, victim, TO_CHAR );

  return;
}

void spell_symbiote(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( victim == ch )
      send_to_char( "You already provide a place to live for a symbiote.\n\r", ch );
    else
      act("$N already provides a place to live for a symbiote.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, NA, ADD, 0, level/6, level, sn ); 
  //affect_to_char( ch, &af );
  createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( victim, &af );

  send_to_char("You provide a place to live for a symbiote.\n\r", victim );
  act("A symbiote joins $n's body.", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_ray_of_sun( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;

  if ( !IS_OUTSIDE(ch) ) {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }
  
  if ( weather_info.sky > SKY_CLOUDLESS && weather_info.sunlight != SUN_DARK ) {
    send_to_char( "You need good sunny weather.\n\r", ch );
    return;
  }

  dam = dice( level, 11 );
  if ( saves_spell( level, victim, DAM_LIGHT ) )
    dam /= 2;
  ability_damage( ch, victim, dam, sn,DAM_LIGHT,TRUE,TRUE);
  return;
}

void spell_sunblind( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !IS_OUTSIDE(ch) ) {
    send_to_char( "You must be out of doors.\n\r", ch );
    return;
  }
  
  if ( weather_info.sky > SKY_CLOUDLESS && weather_info.sunlight != SUN_DARK ) {
    send_to_char( "You need good sunny weather.\n\r", ch );
    return;
  }

  if ( IS_AFFECTED(victim, AFF_BLIND) 
       || saves_spell(level,victim,DAM_OTHER)) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  //afsetup(af,CHAR,hitroll,ADD,-level/10,1+level,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,1+level,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,1+level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,-level/10);
  addaff(af,CHAR,affected_by,OR,AFF_BLIND);
  affect_to_char( victim, &af );

  act("$n casts the light of the sun directly in your eyes. You are blinded!", ch, NULL, victim, TO_VICT);
  act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);
  return;
}

void spell_knowledge( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( is_affected( victim, sn ) ) {
    if (victim == ch)
      send_to_char("You couldn't be wiser.\n\r",ch);
    else
      act("$N couldn't be wiser.",ch,NULL,victim,TO_CHAR);
    return;
  }

  //afsetup(af,CHAR,WIS,ADD,2,level/8,level,sn);
  //affect_to_char( victim, &af );
  //afsetup(af,CHAR,INT,ADD,2,level/8,level,sn);
  //affect_to_char( victim, &af );
  createaff(af,level/8,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,WIS,ADD,2);
  addaff(af,CHAR,INT,ADD,2);
  affect_to_char( victim, &af );

  send_to_char( "You feel wiser.\n\r", victim );
  if ( ch != victim )
    act("$N feels wiser.",ch,NULL,victim,TO_CHAR);
  return;
}

void spell_tongues(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char( "You already understand all languages.\n\r", ch );
    else
      act("$N already understand all languages.", ch, NULL, victim, TO_CHAR );
    return;
  }
  
  //afsetup( af, CHAR, NA, ADD, 0, level/6, level, gsn_comprehend_languages ); 
  //affect_to_char( ch, &af );
  createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( victim, &af );
  
  send_to_char("You feel confident with tongues.\n\r", victim );
  act("$n feels confident with tongues.", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_fireshield(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (is_affected(victim,gsn_fireshield)) {
    if ( ch == victim )
      send_to_char("You are already wreathed in a flaming halo.\n\r",ch);
    else
      act("$N is already wreathed in a flaming halo.", ch, NULL, victim, TO_CHAR );
    return;
  }
  if (is_affected(victim,gsn_iceshield)) {
    send_to_char("Your iceshield vaporises in a steaming explosion!\n\r",victim);
    act("$N's iceshield vaporises in a blast of steam!",ch,NULL,victim,TO_NOTVICT);
    affect_strip(victim,gsn_iceshield);
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_FIRE, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_FIRE);
  affect_to_char( victim, &af );

  send_to_char("You are surrounded in a flaming halo of heat.\n\r",victim);
  act("$N is surrounded by a flaming halo of heat.",ch,NULL,victim,TO_NOTVICT);
  return;
}

void spell_iceshield(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (is_affected(victim,gsn_iceshield)) {
    if ( ch == victim )
      send_to_char("You are already surrounded by a frozen aura.\n\r",ch);
    else
      act("$N is already surrounded by a frozen aura.", ch, NULL, victim, TO_CHAR );
    return;
  }
  if (is_affected(victim,gsn_fireshield)) {
    send_to_char("Your fireshield vaporises in a steaming explosion!\n\r",victim);
    act("$N's fireshield vaporises in a blast of steam!",ch,NULL,victim,TO_NOTVICT);
    affect_strip(victim,gsn_fireshield);
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_COLD, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_COLD);
  affect_to_char( victim, &af );

  send_to_char("You are surrounded by an aura of freezing air.\n\r",victim);
  act("$N is surrounded by a freezing aura of air.",ch,NULL,NULL,TO_NOTVICT);
  return;
}

void spell_stoneshield(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (is_affected(victim,gsn_stoneshield)) {
    if ( victim == ch )
      send_to_char("You are already surrounded by a protective shield of stone.\n\r",ch);
    else
      act("$N is already surrounded by a protective shield of stone.", ch, NULL, victim, TO_CHAR);
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_BASH, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_BASH);
  affect_to_char( victim, &af );

  send_to_char("You are surrounded by a protective shield of stone.\n\r",victim);
  act("$N is surrounded by a protective shield of stone.",ch,NULL,victim,TO_NOTVICT);
  return;
}

void spell_airshield(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  
  if (is_affected(victim,gsn_airshield)) {
    if ( ch == victim )
      send_to_char("You are already surrounded by a protective shield of air.\n\r", ch);
    else
      act("$N is already surrounded by a protective shield of air.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_LIGHTNING, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_LIGHTNING);
  affect_to_char( victim, &af );

  send_to_char("You are surrounded by a protective shield of air.\n\r",victim);
  act("$N is surrounded by a protective shield of air.",ch,NULL,victim,TO_ROOM);
  return;
}

void spell_antimagic_field( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA  *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
    
  if ( IS_AFFECTED2( victim, AFF2_NOSPELL ) 
       || is_affected( victim, sn )
       || saves_spell( level, victim, DAM_OTHER ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  //afsetup( af, CHAR, affected2_by, OR, AFF2_NOSPELL, 2, level, sn ); 
  //affect_to_char( victim, &af );
  createaff(af,2,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_NOSPELL);
  affect_to_char( victim, &af );

  act("{W$N cannot cast spells!{x", ch, NULL, victim, TO_CHAR );
  if ( victim != ch )
    send_to_char("{WYou cannot cast spells!{x\n\r", victim );
  act("{W$N cannot cast spells!{x", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_flesh_to_stone( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( !is_made_flesh( victim )
/*
          (IS_NPC(victim) 
	   && ( IS_SET(victim->act,ACT_UNDEAD)
		|| IS_SET( victim->cstat(form),FORM_UNDEAD)) )
*/
       || (level + 2) < victim->level
       || saves_spell( level, victim, DAM_CHARM)
       || victim->position <= POS_PARALYZED
       || is_affected( victim, sn ) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  int dur = number_fuzzy( 2 ) + ( ( level > 70 ) ? 1 : 0 );

  //afsetup(af,CHAR,NA,ADD,victim->hit,dur,level,sn); // store current hp
  //affect_join( victim, &af );
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,NA,ADD,victim->hit);
  affect_to_char( victim, &af );

  if ( IS_AWAKE(victim) ) {
    send_to_char( "Your skin turns to stone, you can't move any muscle.\n\r", victim );
    act( "$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM );
    //if (victim->fighting || victim->position == POS_FIGHTING)
    //  stop_fighting(victim,TRUE);
    victim->position = POS_PARALYZED;
    update_pos( victim );
  }
  return;
}

void spell_turn_undead(int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  char god[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];

  strcpy( god,char_god_name( ch ));// IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
  sprintf(buf,"$n summons the full wrath and fury of %s.", god );
  act(buf,ch,NULL,NULL,TO_ROOM);
  send_to_charf(ch,"You summon the full wrath and fury of %s.\n\r", god );

  CHAR_DATA *vch, *vch_next;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    // works only on undead NPC
    if ( !IS_NPC(vch)
	 || !( IS_SET(vch->act,ACT_UNDEAD)
	       || IS_SET( vch->cstat(form),FORM_UNDEAD)) )
      continue;

    // and not in the same group as caster
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    // if really low level victim ... destroy it
    if ( !saves_spell( level-5, vch, DAM_CHARM ) 
	 && chance(UMAX(level-vch->level,0)) ) {
      act("$N decays into dust.", ch, NULL,vch,TO_NOTVICT);
      send_to_charf(vch,"You decay into dust!\n\r");
      act("$N decays into dust.",ch,NULL,vch,TO_CHAR);
      sudden_death( ch, vch );
      continue;
    }

    // saves spells ?
    if ( saves_spell( level, vch, DAM_CHARM ) )
      continue;

    if (vch->position == POS_FIGHTING) do_flee(vch,"");
    if (vch->position == POS_FIGHTING) do_flee(vch,"");
    if (vch->position == POS_FIGHTING) do_flee(vch,"");
    // Flee successfull ??
    if ( vch->in_room == ch->in_room ) // not sucessfull, does some damage
      ability_damage( ch, vch, 10, sn, DAM_CHARM, TRUE, TRUE );
  }

  return;
}

void spell_quicksand( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  if ( IS_AFFECTED( victim, AFF_FLYING ) ) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }

  // must be on earth
  if ( ch->in_room->cstat(sector) == SECT_AIR
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char( "There is no earth around!\n\r", ch );
    return;
  }

  act("You create a pool of quicksand under $N's feet.", ch, NULL, victim, TO_CHAR );
  act("$n creates a pool of quicksand under your feet.", ch, NULL, victim, TO_VICT );
  act("$n creates a pool of quicksand under $N's feet.", ch, NULL, victim, TO_NOTVICT );

  // Undead are unaffected
  if ( ( IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD) )
       || IS_SET( victim->cstat(form),FORM_UNDEAD) ) {
    send_to_char("Spell failed.\n\r", ch);
    ability_damage(ch,victim,0,sn,DAM_DROWNING,TRUE, TRUE);
    return;
  }

  bool saves = FALSE;
  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_DROWNING ) ) {
    saves = TRUE;
    dam /= 2;
  }

  // if casting level == 5 and !saves --> 10% chance dying instantly
  if ( saves_spell( level, victim, DAM_DROWNING ) 
       && casting_level == 5
       && chance(10) ) {
    act("$N drown to death instantly.", ch, NULL, victim, TO_CHAR );
    act("You drown to death instantly.", ch, NULL, victim, TO_VICT );
    act("$N drown to death instantly.", ch, NULL, victim, TO_NOTVICT );
    sudden_death( ch, victim );
    return;
  }

  if ( !is_affected( victim, sn ) && !saves ) {
    //afsetup(af,CHAR,DEX,ADD, -3-casting_level, 5+casting_level, level, sn );
    //affect_to_char( victim, &af );
    createaff(af,-3-casting_level,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,DEX,ADD,5+casting_level);
    affect_to_char( victim, &af );
  }

  ability_damage(ch,victim,dam,sn,DAM_DROWNING,TRUE,TRUE);
}

void spell_whirling_sands(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  if ( ch->in_room->cstat(sector) == SECT_AIR
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char( "There is no earth around!\n\r", ch );
    return;
  }

  send_to_char("You summon a whirling sandstorm.\n\r", ch );
  act("$n summons a whirling sandstorm.", ch, NULL, NULL, TO_ROOM);

  CHAR_DATA *vch, *vch_next;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    // and not in the same group as caster
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    int dam = dice( level, 8 );
    if ( saves_spell( level, vch, DAM_BASH ) )
      ability_damage( ch, vch, dam/2, sn, DAM_BASH, TRUE, TRUE );
    else
      ability_damage( ch, vch, dam, sn, DAM_BASH, TRUE, TRUE );
  }
}

void spell_lightning_field(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (is_affected(victim,gsn_airshield)
      || is_affected(victim,gsn_lightning_field) ) {
    if ( ch == victim )
      send_to_char("You are already surrounded by a protective field of lightning.\n\r",ch);
    else
      act("$N is already surrounded by a protective field of lightning.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup(af, CHAR, res_flags, OR, IRV_LIGHTNING, level, level, sn );
  //affect_to_char( ch, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,res_flags,OR,IRV_LIGHTNING);
  affect_to_char( victim, &af );

  send_to_char("You are surrounded by a protective field of lightning.\n\r",victim);
  act("$N is surrounded by a protective field of lightning.",ch,NULL,victim,TO_NOTVICT);
  return;
}

void spell_arrow_of_anylas( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  send_to_char("You summon an arrow from Anylas Gil-Ganad's bow.\n\r", ch );
  act("$n summons an arrow from Anylas Gil-Ganad's bow.", ch, NULL, NULL, TO_ROOM );
  // if victim has an heart and !saves --> 10% chance dying instantly
  if ( saves_spell( level, victim, DAM_PIERCE ) 
       && chance(10) 
       && IS_SET(ch->cstat(parts), PART_HEART ) ) {
    act("Anylas's arrow strikes $N in the heart.", ch, NULL, victim, TO_CHAR );
    act("Anylas's arrow strikes you in the heart.", ch, NULL, victim, TO_VICT );
    act("Anylas's arrow strikes $N in the heart.", ch, NULL, victim, TO_NOTVICT );
    sudden_death( ch, victim );
    return;
  }

  int dam = dice( level, 10 );
  if ( saves_spell( level, victim, DAM_PIERCE ) )
    dam /= 2;

  act("Anyla's arrow strikes you.",ch, NULL, victim, TO_VICT);
  ability_damage(ch,victim,dam,sn,DAM_PIERCE,TRUE,TRUE);
}

void spell_improved_restoration( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("You can't be more accurate with restoration sphere.\n\r", ch );
    else
      act("$N can't be more accurate with restoration sphere.", ch, NULL, victim, TO_CHAR );
    return;
  }

  //afsetup( af, CHAR, NA, ADD, 0, level/2, level, sn ); 
  //affect_to_char( ch, &af );
  createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( victim, &af );

  send_to_char("You feel more accurate with restoration sphere.\n\r", victim );
  if ( ch != victim )
    send_to_char("Ok.\n\r", ch );
  return;
}

void spell_resurrection( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  OBJ_DATA *corpse;
  OBJ_DATA *obj, *obj_next;


  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if (target_name[0] == '\0') {
    send_to_char("Resurrect which corpse?\n\r",ch);
    return;
  }
  
  corpse = get_obj_here(ch,target_name);

  if (corpse == NULL) {
    send_to_char("You can't find that corpse.\n\r",ch);
    return;
  }
  
  if( corpse->item_type != ITEM_CORPSE_NPC ) {
    if( corpse->item_type == ITEM_CORPSE_PC )
      send_to_char( "You can't resurrect players.\n\r", ch );
    else
      send_to_char( "It would serve no purpose...\n\r", ch );
    return;
  }

  if( corpse->level > (ch->level + 2) ) {
    send_to_char( "You couldn't call forth such a great spirit.\n\r", ch );
    return;
  }

  if ( !can_loot( ch, corpse ) ) {
    act("You are not allowed to resurrect $p.", ch, corpse, NULL, TO_CHAR );
    return;
  }

  if ( corpse->baseval[0] <= 0 ) {
    act("$p can't be resurrected.", ch, corpse, NULL, TO_CHAR );
    return;
  }

  // SinaC 2003: if corpse has missing parts -> no resurrect
  if ( corpse->baseval[4] != 0 ) {
    act("$p is not completed and so cannot be resurrected.", ch, corpse, NULL, TO_CHAR );
    return;
  }

  MOB_INDEX_DATA *mobIndex;
  if ( ( mobIndex = get_mob_index( corpse->baseval[0] ) ) == NULL ) {
    act("$p can't be resurrected.", ch, corpse, NULL, TO_CHAR );
    return;
  }

  int chance = (ch->level - corpse->level) + ch->cstat(INT)*5;
  if ( number_percent() >= chance ) {
    act("You failed to resurrect $p and destroy the corpse.", ch, corpse, NULL, TO_CHAR );
    extract_obj( corpse );
    return;
  }

  CHAR_DATA *mob;
  mob = create_mobile( mobIndex );
  for ( obj = corpse->contains; obj != NULL; obj = obj_next ) {
    obj_next = obj->next_content;
    obj_from_obj( obj );
    obj_to_char( obj, mob );
  }
  //recompute( ch ); NO NEED: done in char_to_room
  extract_obj( corpse );

  act("You bring $N back to life.", ch, NULL, mob, TO_CHAR );
  act("$n brings $N back to life.", ch, NULL, mob, TO_NOTVICT );
  act("$n brings you back to life.", ch, NULL, mob, TO_VICT );

  char_to_room( mob, ch->in_room );
}

void spell_vacuum(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  if ( ch->in_room->cstat(sector) == SECT_UNDERWATER ) {
    send_to_char( "There is no air around!\n\r", ch );
    return;
  }

  send_to_char("The air is sucked out of the area.\n\r", ch );
  act("The air is sucked out of the area.", ch, NULL, NULL, TO_ROOM);

  CHAR_DATA *vch, *vch_next;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    // and not in the same group as caster
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    if ( saves_spell( level, vch, DAM_BASH ) 
	 && chance(20)
	 && casting_level == 5
	 && !( ( IS_NPC(vch) && IS_SET(vch->act,ACT_UNDEAD) )
	       || IS_SET( vch->cstat(form),FORM_UNDEAD) ) ) {
      send_to_char("The air rushes back with incredible force and your body implodes.",vch);
      sudden_death( ch, vch );
      continue;
    }

    send_to_char("The air rushes back with incredible force.\n\r", vch );
    int dam = dice( (level*casting_level)/5, number_fuzzy(8) );
    if ( saves_spell( level, vch, DAM_BASH ) )
      ability_damage( ch, vch, dam/2, sn, DAM_BASH, TRUE, TRUE );
    else
      ability_damage( ch, vch, dam, sn, DAM_BASH, TRUE, TRUE );
  }
}

void spell_flame_burst( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] =   {
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
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 2 ), 2 ) * casting_level;
  if ( casting_level == 5 )
    level *= 2;

  if ( saves_spell( level, victim, DAM_FIRE) )
    dam /= 2;
  bool done = FALSE;
  for ( int i = 0; i < UMAX( casting_level-2, 1 ); i++ ) {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_FIRE ,TRUE, TRUE);
    if ( damdone == DAMAGE_DEADLY )
      return;
    done |= damdone;
  }

  if ( done && chance(20) 
       && !saves_spell( level, victim, DAM_FIRE )
       && casting_level >= 3 )
    fire_effect( (void *) victim, level, dam, TARGET_CHAR );
  return;
}

void spell_ice_blast( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  static const int dam_each[] =   {
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
  dam		= UMAX( number_range( dam_each[level], dam_each[level] * 2 ), 2 ) * casting_level;
  if ( casting_level == 5 )
    level *= 2;

  if ( saves_spell( level, victim, DAM_COLD) )
    dam /= 2;
  bool done = FALSE;
  for ( int i = 0; i < UMAX( casting_level-2, 1 ); i++ ) {
    int damdone = ability_damage( ch, victim, dam, sn, DAM_COLD ,TRUE, TRUE);
    if ( damdone == DAMAGE_DEADLY )
      return;
    done |= damdone;
  }

  if ( done && chance(20) 
       && !saves_spell( level, victim, DAM_COLD )
       && casting_level >= 3 )
    cold_effect( (void *) victim, level, dam, TARGET_CHAR );
  return;
}

void spell_ice_storm( int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  static char * const him_her_self[] = { "itself", "himself", "herself" };
  int dam;
  
  dam = dice(level+2,10)*casting_level;

  send_to_char("Huge balls of ice come crashing down.\n\r", ch );
  act("Huge balls of ice come crashing down.", ch, NULL, NULL, TO_CHAR );

  if ( casting_level != 5 ) {
    // hits caster
    int effDam = dam;
    if (saves_spell(level,ch,DAM_COLD))
      effDam /= 2;
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "kills %s with a ball of ice!", 
	     him_her_self[URANGE(0,ch->cstat(sex),2)] );
    noaggr_damage( ch, effDam, DAM_COLD,
		   "You are hurt by the storm!",
		   "A ball of ice hurts $n!",
		   buf,
		   TRUE );
  }
  if ( ch == NULL || !ch->valid )
    return;
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    if (saves_spell(level,vch,DAM_COLD))
      ability_damage(ch,vch,dam/2,sn,DAM_COLD,TRUE,TRUE);
    else
      ability_damage(ch,vch,dam,sn,DAM_COLD,TRUE,TRUE);
  }
  return;
}

void spell_fire_storm( int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  static char * const him_her_self[] = { "itself", "himself", "herself" };
  int dam;
  
  dam = dice(level+2,10)*casting_level;

  send_to_char("Fire rains down.\n\r", ch );
  act("Fire rains down.", ch, NULL, NULL, TO_CHAR );

  if ( casting_level != 5 ) {
    // hits caster
    int effDam = dam;
    if (saves_spell(level,ch,DAM_FIRE))
      effDam /= 2;
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "kills %s with a rain of fire!", 
	     him_her_self[URANGE(0,ch->cstat(sex),2)] );
    noaggr_damage( ch, effDam, DAM_FIRE,
		   "You are hurt by a rain of fire!",
		   "A rain of fire hurts $n!",
		   buf,
		   TRUE );
  }
  if ( ch == NULL || ch->in_room == NULL || !ch->valid )
    return;
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    if (saves_spell(level,vch,DAM_FIRE))
      ability_damage(ch,vch,dam/2,sn,DAM_FIRE,TRUE,TRUE);
    else
      ability_damage(ch,vch,dam,sn,DAM_FIRE,TRUE,TRUE);
  }
  return;
}

void spell_meteor_shower( int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  static char * const him_her_self[] = { "itself", "himself", "herself" };
  int dam;
  
  dam = dice(level+2,10)*casting_level;

  send_to_char("Showers of meteors crashes down.\n\r", ch );
  act("Showers of meteors crashes down.", ch, NULL, NULL, TO_CHAR );

  if ( casting_level < 5 ) {
    // hits caster
    int effDam = dam;
    if (saves_spell(level,ch,DAM_BASH))
      effDam /= 2;
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "kills %s with a shower of meteors!", 
	     him_her_self[URANGE(0,ch->cstat(sex),2)] );
    noaggr_damage( ch, effDam, DAM_BASH,
		   "You are hurt by a shower of meteors!",
		   "A shower of meteors hurts $n!",
		   buf,
		   TRUE );
  }
  if ( ch == NULL || ch->in_room == NULL || !ch->valid )
    return;
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    if (saves_spell(level,vch,DAM_BASH))
      ability_damage(ch,vch,dam/2,sn,DAM_BASH,TRUE,TRUE);
    else
      ability_damage(ch,vch,dam,sn,DAM_BASH,TRUE,TRUE);
  }
  return;
}

void spell_necrotism( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  AFFECT_DATA af;
  
  if ( target == TARGET_OBJ ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    
    if ( is_affected( ch, sn ) ) { // can't cast necrotic spell when affected by necrotism
      send_to_char("You can't cast necrotism when affected.\n\r", ch );
      return;
    }

    if(obj->item_type != ITEM_WEAPON) {
      send_to_char("You can only cast this spell on weapons or yourself.\n\r",ch);
      return;
    }
    
    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_char("You can't cast this spell on that kind of weapon.\n\r", ch );
      return;
    }

    if(IS_WEAPON_STAT(obj,WEAPON_NECROTISM)) {
      send_to_char("This weapon is already necrotic.\n\r", ch);
      return;
    }
    
    //afsetup(af,WEAPON,NA,OR,WEAPON_NECROTISM,level/5,level,sn);
    //affect_to_obj(obj, &af);
    createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,WEAPON,NA,OR,WEAPON_NECROTISM);
    affect_to_obj( obj, &af );
    
    act("The power of death imbues $p.", ch, obj, NULL, TO_CHAR );
    act("The power of death imbues $p.", ch, obj, NULL, TO_ROOM );
  }
  else if ( target == TARGET_CHAR ) {
    if ( is_affected( ch, sn ) ) {
      send_to_char("You are already using death has a weapon.\n\r", ch );
      return;
    }
    
    //afsetup(af,CHAR,affected2_by,OR,AFF2_NECROTISM,level/2,level,sn);
    //affect_to_char( ch, &af );
    createaff(af,level/6,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected2_by,OR,AFF2_NECROTISM);
    affect_to_char( ch, &af );
    
    send_to_char("Death power flows through your veins.\n\r", ch );
  }
  return;
}

void spell_enlarge( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  if ( target == TARGET_OBJ ) {
    OBJ_DATA *obj = (OBJ_DATA*) vo;

    if ( obj->size == SIZE_GIANT ) {
      act("$p can't be bigger.", ch, obj, NULL, TO_CHAR );
      return;
    }

    int mod = +1;
    if ( chance(10) ) {
      mod = +5; // immediatly giant size
      act("You have enlarged $p to giant size.", ch, obj, NULL, TO_CHAR );
      act("$n enlarges $p to giant size.", ch, obj, NULL, TO_ROOM );
    }
    else {
      act("You have enlarged $p.", ch, obj, NULL, TO_CHAR );
      act("$n has enlarged $p.", ch, obj, NULL, TO_ROOM );
    }
    change_size_obj( obj, mod );
  } 
  else if ( target == TARGET_CHAR ) {
    int currentSize = ch->cstat(size);
    if ( currentSize == SIZE_GIANT ) {
      send_to_char("You can't be taller.\n\r", ch );
      return;
    }

    int newSize = currentSize+1;
    if ( chance(10) ) { // enlarge by 2 size
      newSize = SIZE_GIANT; // immediatly giant size
      send_to_char("You start to grow and become as tall as a giant.\n\r", ch );
      act("$n starts to grow and becomes as tall as a giant.\n\r", ch, NULL, NULL, TO_ROOM );
    }
    else {
      send_to_char("You become taller.\n\r", ch );
      act("$n becomes taller.\n\r", ch, NULL, NULL, TO_ROOM );
    }

    affect_strip( ch, gsn_enlarge ); // remove existing enlarge/reduce
    affect_strip( ch, gsn_reduce );

    newSize = URANGE( SIZE_TINY, newSize, SIZE_GIANT );

    // Change caster's size
    AFFECT_DATA af;
    //afsetup( af, CHAR, size, ASSIGN, newSize, DURATION_PERMANENT, level, sn);
    //affect_to_char(ch,&af);
    createaff(af,-1,level,sn,casting_level,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_STAY_DEATH);
    addaff(af,CHAR,size,ASSIGN,newSize);
    affect_to_char( ch, &af );

    // Change worn item's size only if item has a size
    for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
      if ( obj->wear_loc != WEAR_NONE 
	   && obj->size != SIZE_NOSIZE )
	obj->size = newSize;
  }
}

void spell_reduce( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level) {
  if ( target == TARGET_OBJ ) {
    OBJ_DATA *obj = (OBJ_DATA*) vo;

    if ( obj->size == SIZE_TINY ) {
      act("$p can't be smaller.", ch, obj, NULL, TO_CHAR );
      return;
    }

    int mod = -1;
    if ( chance(10) ) {
      mod = -5; // immediatly tiny size
      act("You have reduced $p to tiny size.", ch, obj, NULL, TO_CHAR );
      act("$n reduced $p to tiny size.", ch, obj, NULL, TO_ROOM );
    }
    else {
      act("You have reduced $p.", ch, obj, NULL, TO_CHAR );
      act("$n has reduced $p.", ch, obj, NULL, TO_ROOM );
    }
    change_size_obj( obj, mod );
  } 
  else if ( target == TARGET_CHAR ) {
    int currentSize = ch->cstat(size);
    if ( currentSize == SIZE_TINY ) {
      send_to_char("You can't be smaller.\n\r", ch );
      return;
    }

    int newSize = currentSize-1;
    if ( chance(10) ) { // enlarge by 2 size
      newSize = SIZE_TINY; // immediatly giant size
      send_to_char("You start to shrink and become as small as a faerie.\n\r", ch );
      act("$n starts to shrink and becomes as small as a faerie.\n\r", ch, NULL, NULL, TO_ROOM );
    }
    else {
      send_to_char("You become smaller.\n\r", ch );
      act("$n becomes smaller.\n\r", ch, NULL, NULL, TO_ROOM );
    }

    affect_strip( ch, gsn_enlarge ); // remove existing enlarge/reduce
    affect_strip( ch, gsn_reduce );

    newSize = URANGE( SIZE_TINY, newSize, SIZE_GIANT );

    // Change caster's size
    AFFECT_DATA af;
    //afsetup( af, CHAR, size, ASSIGN, newSize, DURATION_PERMANENT, level, sn);
    //affect_to_char(ch,&af);
    createaff(af,-1,level,sn,casting_level,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_STAY_DEATH);
    addaff(af,CHAR,size,ASSIGN,newSize);
    affect_to_char( ch, &af );

    // Change worn item's size only if item has a size
    for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
      if ( obj->wear_loc != WEAR_NONE 
	   && obj->size != SIZE_NOSIZE )
	obj->size = newSize;
  }
}

void spell_growth_shrooms( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  if ( time_info.hour != 12 ) {
    send_to_char("You can only cast this spell during the noon hour.\n\r", ch );
    return;
  }

  if ( !IS_OUTSIDE(ch) ) {
    send_to_char("You can only cast this spell outdoors.\n\r", ch );
    return;
  }

  if ( ch->in_room->cstat(sector) == SECT_CITY ) {
    send_to_char("You can't cast this spell within the limits of a city.\n\r", ch );
    return;
  }

  obj = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
  obj->level = level;
  obj->item_type = ITEM_PILL;
  for ( int i = 0; i < 5; i++ )
    obj->baseval[i] = obj->value[i] = 0;
  obj->baseval[0] = obj->value[0] = level;
  obj->baseval[1] = obj->value[1] = gsn_enlarge;

  obj->name = str_dup("growth shroom mushroom blue");
  obj->short_descr = str_dup("a {Bbright blue mushroom{x");
  obj->description = str_dup("A delicious {Bbright blue mushroom{x is here.");

  act( "$n has created $p.", ch, obj, NULL, TO_ROOM );
  act( "You have created $p.", ch, obj, NULL, TO_CHAR );
  obj_to_char( obj, ch );
  return;
}

void spell_shrinking_shrooms( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *obj;
  char buf[MAX_STRING_LENGTH];

  if ( time_info.hour != 0 && time_info.hour != 24 ) {
    send_to_char("You can only cast this spell during the midnight hour.\n\r", ch );
    return;
  }

  if ( !IS_OUTSIDE(ch) ) {
    send_to_char("You can only cast this spell outdoors.\n\r", ch );
    return;
  }

  if ( ch->in_room->cstat(sector) == SECT_CITY ) {
    send_to_char("You can't cast this spell within the limits of a city.\n\r", ch );
    return;
  }

  obj = create_object( get_obj_index( OBJ_VNUM_MUSHROOM ), 0 );
  obj->level = level;
  obj->item_type = ITEM_PILL;
  for ( int i = 0; i < 5; i++ )
    obj->baseval[i] = obj->value[i] = 0;
  obj->baseval[0] = obj->value[0] = level;
  obj->baseval[1] = obj->value[1] = gsn_reduce;

  obj->name = str_dup("shrinking shroom mushroom red");
  obj->short_descr = str_dup("a {Rbright red mushroom{x");
  obj->description = str_dup("A delicious {Rbright red mushroom{x is here.");

  act( "$n has created $p.", ch, obj, NULL, TO_ROOM );
  act( "You have created $p.", ch, obj, NULL, TO_CHAR );
  obj_to_char( obj, ch );
  return;
}

void spell_gift_of_nature( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  OBJ_DATA *staff;
  AFFECT_DATA af;

  staff = create_object( get_obj_index( OBJ_VNUM_OAKEN_STAFF ), 0 );
  staff->timer = level;
  staff->baseval[1] = UMAX(level/5,5);
  staff->baseval[2] = number_fuzzy(8);

  recompobj(staff);
  staff->level    = ch->level;   /* so low-levels can't wear them */

  //afsetup(af,CHAR,saving_throw,ADD,0-number_fuzzy(UMAX(level/10,3)),level,level,sn);
  //affect_to_obj(staff,&af);
  //afsetup(af, CHAR, res_flags, OR, IRV_FIRE, level, level, sn );
  //affect_to_obj( staff, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,saving_throw,ADD,0-number_fuzzy(UMAX(level/10,3)));
  addaff(af,CHAR,res_flags,OR,IRV_FIRE);
  affect_to_obj( staff, &af );

  obj_to_char( staff, ch );
  act( "$n has created $p.", ch, staff, NULL, TO_ROOM );
  act( "You have created $p.", ch, staff, NULL, TO_CHAR );
  return;
}

void spell_tornado( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  act("Huge gusts of wind buffet $N.", ch, NULL, victim, TO_CHAR );
  act("Huge gusts of wind buffet you.", ch, NULL, victim, TO_VICT );
  act("Huge gusts of wind buffet $N.", ch, NULL, victim, TO_NOTVICT );
  
  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_BASH )
       && casting_level < 3 )
    dam /= 2;
  if ( casting_level == 5 )
    dam *= 2;

  // A small chance to be suck up and transport to a random location
  if ( casting_level == 5
       && chance(10)
       && victim->in_room != NULL
       && !IS_SET(victim->in_room->cstat(flags), ROOM_NO_RECALL)
       && !( victim != ch && IS_SET(victim->cstat(imm_flags),IRV_SUMMON))) {
    ROOM_INDEX_DATA *pRoomIndex;
    pRoomIndex = get_random_room(victim);
    
    if (victim != ch)
      send_to_char("You are sucked up by the tornado!\n\r",victim);
    
    // Added by SinaC 2000
    if ( victim->fighting != NULL )
      stop_fighting( victim, FALSE );
    
    act( "$n is sucked up by the tornado!", victim, NULL, NULL, TO_ROOM );
    char_from_room( victim );
    char_to_room( victim, pRoomIndex );
    if ( !IS_AFFECTED( victim, AFF_FLYING ) )
      noaggr_damage( victim, dam, DAM_BASH,
		     "The tornado stops and throws you back on the ground!",
		     "A tornado arrives and throw $n back on the ground.",
		     "has been killed by a tornado.",
		     TRUE );
    else {
      send_to_char("The tornado stops.\n\r", ch );
      act( "A tornado arrives and drops $n.", victim, NULL, NULL, TO_ROOM );
    }
    if ( victim != NULL && victim->in_room != NULL ) {
      do_look( victim, "auto" );
      MOBPROG(ch,NULL,"onMoved");
    }
    return;
  }

  ability_damage( ch, victim, dam, sn, DAM_BASH, TRUE, TRUE );
}

void spell_sandstorm( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  if ( ch->in_room->cstat(sector) == SECT_UNDERWATER 
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_AIR ) {
    send_to_char( "There is no earth around!\n\r", ch );
    return;
  }
  CHAR_DATA *vch, *vch_next;
  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    if ( casting_level >= 3  // blind target?
	 && !IS_AFFECTED(vch, AFF_BLIND) 
	 && !saves_spell(level,vch,DAM_OTHER)
	 && chance(5*casting_level)
	 && !is_affected(vch,sn) ) {

      AFFECT_DATA af;
      
      //afsetup(af,CHAR,hitroll,ADD,-level/10,4,level,sn);
      //affect_to_char( vch, &af );
      //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,4,level,sn);
      //affect_to_char( vch, &af );
      createaff(af,4,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,-level/10);
      addaff(af,CHAR,affected_by,OR,AFF_BLIND);
      affect_to_char( vch, &af );
      
      send_to_char( "The sand is so intense that it blinds you!\n\r", vch );
      act("The sand is so intense that it blinds $n.",vch,NULL,NULL,TO_ROOM);
    }
    if (saves_spell(level,vch,DAM_BASH))
      ability_damage(ch,vch,dam/2,sn,DAM_BASH,TRUE,TRUE);
    else {
      if ( casting_level == 5 ) // at highest level, hit many times
	for ( int i = 0; i < number_fuzzy(2); i++ )
	  ability_damage(ch,vch,dam,sn,DAM_BASH,TRUE,TRUE);
      else
	ability_damage(ch,vch,dam,sn,DAM_BASH,TRUE,TRUE);
    }
  }
}

void spell_hydrostrike( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int dam;
  
  dam = dice( level, 12 );
  if ( saves_spell( level, victim, DAM_DROWNING ) )
    dam /= 2;
  
  if ( chance(5) // small chance to kill in one hit
       && !saves_spell( level, victim, DAM_DROWNING )
       && !(IS_NPC(victim) && IS_SET(victim->act,ACT_UNDEAD) )
       && !IS_SET( victim->cstat(form),FORM_UNDEAD)
       && victim != ch ) {
    act("$n's strike of water drowns you.", ch, NULL, victim, TO_VICT );
    act("Your strike of water drowns $N.", ch, NULL, victim, TO_CHAR );
    act("$n's strike of water drowns $N.", ch, NULL, victim, TO_NOTVICT );
    sudden_death( ch, victim );
    return;
  }

  int done = ability_damage( ch, victim, dam, sn, DAM_DROWNING, TRUE, TRUE);
  if( done == DAMAGE_DONE && chance(15) ) { // small chance to destroy some equipment
    OBJ_DATA *obj_lose, *obj_next;
    for ( obj_lose = victim->carrying; obj_lose; obj_lose = obj_next ) {
      obj_next = obj_lose->next_content;
      
      if ( number_bits( 4 ) != 0  // 1 chance out 8 to destroy one item from inventory
	   || IS_SET(obj_lose->extra_flags, ITEM_NOCOND)
	   || check_immune_obj(obj_lose,DAM_DROWNING) )
	continue;
      
      act( "$p is destroyed!",      victim, obj_lose, NULL, TO_CHAR );
      act( "$n's $p is destroyed!", victim, obj_lose, NULL, TO_ROOM );
      extract_obj( obj_lose ) ;
    }
  }
  return;
}

void spell_blinding_radiance( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  if (target == TARGET_OBJ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    AFFECT_DATA af;

    if ( chance(40)
	 && material_table[obj->material].metallic ) { // metallic items melt down 40%
      act("$p melts down.", ch, obj, NULL, TO_ALL );
      extract_obj(obj);
      return;
    }

    int dur = level/5;
    //afsetup(af,OBJECT,NA,OR,ITEM_GLOW,dur,level,sn);
    //affect_to_obj(obj,&af);
    createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,OBJECT,NA,OR,ITEM_GLOW);
    affect_to_obj( obj, &af );
    
    act("$p glows.",ch,obj,NULL,TO_ALL);
    return;
  }
  else if (target == TARGET_CHAR) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;

    if ( ch == victim ) {
      send_to_char("You cannot cast this on you.\n\r", ch );
      return;
    }

    if ( chance(5) ) { // 5% chance to kill in one hit
      char god[100];
      char buf[MAX_STRING_LENGTH];
      strcpy( god, char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
      sprintf(buf,"%s comes and smite $N!", god );
      act(buf, ch, NULL,victim,TO_NOTVICT);
      send_to_charf(ch,"%s comes and smite you!\n\r", god);
      sprintf(buf,"%s comes, looks at $N and smite $M!", god );
      act(buf,ch,NULL,victim,TO_CHAR);
      sudden_death( ch, victim );
      return;
    }
    if ( !IS_AFFECTED(victim, AFF_BLIND) 
	 && !saves_spell(level,victim,DAM_OTHER)
	 && !is_affected(victim,sn) ) {
      AFFECT_DATA af;

      //afsetup(af,CHAR,hitroll,ADD,-level/10,level/2,level,sn);
      //affect_to_char( victim, &af );
      //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,level/2,level,sn);
      //affect_to_char( victim, &af );
      createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,hitroll,ADD,-level/10);
      addaff(af,CHAR,affected_by,OR,AFF_BLIND);
      affect_to_char( victim, &af );
      
      send_to_char( "You are blinded by the blinding radiance!\n\r", victim );
      act("The blinding radiance blinds $n.",victim,NULL,NULL,TO_ROOM);
    }
    else
      send_to_char("Spell has no effect.\n\r", ch );
  }
}

void spell_polymorph_self( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  char *art;
  char race[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int iRace, timer;

  if ( is_affected( ch, sn )
       || is_affected( ch, gsn_morph ) ) {
    send_to_char( "You are already under the effect of the morph!\n\r", ch );
    return;
  }

  one_argument( target_name, race );

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

  timer = UMAX( 10, level/5 );
  // change equipement before changing race because of recompute at the end of change_race
  //int newSize = pc_race_table[iRace].size;
  int newSize = race_table[iRace].size;
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
    if ( obj->wear_loc != WEAR_NONE 
	 && obj->size != SIZE_NOSIZE )
      obj->size = newSize;

  // change race, form, parts, imm, res, vuln, aff
  change_race( ch, iRace, timer, level, sn, 0 );

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

void spell_arachnophobia( int sn, int level, CHAR_DATA *ch, void *vo,int target, int casting_level ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  send_to_char( "You summon a swarm of spiders!\n\r", ch );
  act( "$n summons a swarm of spiders", ch, NULL, NULL, TO_ROOM );

  for ( vch = char_list; vch != NULL; vch = vch_next ) {
    vch_next	= vch->next;
    if ( vch->in_room == NULL || vch == ch )
      continue;
    if ( vch->in_room == ch->in_room ) {
      if ( is_safe_spell(ch,vch,TRUE) || is_same_group(vch,ch) )
	  continue;

      dam = dice(level/2, 10);
      int tmp_dam = dam,
	tmp_lvl = level;
      if ( saves_spell( level, vch, DAM_POISON ) ) {
	tmp_dam = dam/2;
	tmp_lvl = level/2;
      }
      int done = ability_damage( ch, vch, tmp_dam, sn, DAM_POISON, TRUE, TRUE );
      if ( done == DAMAGE_DONE && chance(20) && !saves_spell( level, vch, DAM_POISON) ) {
	AFFECT_DATA af;
	//afsetup(af,CHAR,affected_by,OR,AFF_POISON,level,level,sn);
	//affect_join( vch, &af );
	//afsetup(af,CHAR,STR,ADD,-2,level,level,sn);
	//affect_join( vch, &af );
	createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
	addaff(af,CHAR,STR,ADD,-2);
	addaff(af,CHAR,affected_by,OR,AFF_POISON);
	affect_join( vch, &af );
	
	send_to_char( "You feel very sick.\n\r", vch );
	act("$n looks very ill.",vch,NULL,NULL,TO_ROOM);
      }
      continue;
    }

    if ( vch->in_room->area == ch->in_room->area )
      send_to_char( "From somewhere nearby you hear a buzzing sound.\n\r", vch );
  }

  return;
}

void spell_fear(int sn, int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;
  bool bad_fail, utter_fail;
  int range, dur, mod;

  bad_fail = FALSE;
  utter_fail = FALSE;

  if (victim == ch) {
    send_to_char("Spell failed.\n\r",ch);
    return;
  }

  act("$n points at $N and utters the word 'Fear!'",ch,0,victim,TO_NOTVICT);
  act("$n points at you and utters the word 'Fear!'",ch,0,victim,TO_VICT);
  act("You point at $N and utter the word 'Fear!'",ch,0,victim,TO_CHAR);

  if (!IS_AWAKE(victim)) {
    act("$n shivers momentarily but it passes.",victim,0,0,TO_ROOM);
    send_to_char("You feel a brief terror, but it passes away in your dreams.\n\r",
		 victim);
    return;
  }
  if (is_affected(victim,sn)) { 	
    send_to_char("They are already affected by a word of power.\n\r",ch);
    send_to_char("You feel a shiver pass through you but it has no further affect.\n\r",victim);
    return;
  }

  if (saves_spell(level,victim,DAM_OTHER) && casting_level < 3 ) {
    act("$n shivers momentarily but it passes.",victim,0,0,TO_ROOM);
    send_to_char("You feel a brief terror, but it passes.\n\r",victim);
    return;
  }
  
  if (!saves_spell(level - 2,victim,DAM_OTHER)
      && chance(20*casting_level)
      && casting_level >= 3 ) {
    bad_fail = TRUE;
    if (!saves_spell(level - 5,victim,DAM_OTHER)
	&& chance(5*casting_level)
	&& casting_level >= 3 )
      utter_fail = TRUE;
  }
  
  if ( !victim->in_room
       // Modified by SinaC 2001
       || IS_SET( victim->in_room->cstat(flags), ROOM_SAFE      )
       || IS_SET( victim->in_room->cstat(flags), ROOM_PRIVATE   )
       || IS_SET( victim->in_room->cstat(flags), ROOM_SOLITARY  )
       || IS_SET( victim->in_room->cstat(flags), ROOM_NO_RECALL )
       || victim->in_room->area != ch->in_room->area )
    utter_fail = FALSE;

  if (utter_fail) {
    act("$n's eyes widen and $s heart ruptures from shock!",victim,0,0,TO_ROOM);
    send_to_char("You feel a terror so intense your heart stops dead!\n\r",victim);
    sudden_death( ch, victim );
    return;
  }
  
  act("$n's eyes widen in shock and $s entire body freezes in momentary terror.",victim,NULL,NULL,TO_ROOM);
  send_to_char("You feel an overwhelming terror and you shudder in momentary shock.\n\r",victim);

  range = (casting_level*level)/10;
  dur = (number_range(1,5) + range);
  //afsetup( af, CHAR, damroll, ADD, -number_range(2,range), dur, level, sn );
  //affect_to_char( victim, &af );
  //afsetup( af, CHAR, hitroll, ADD, -number_range(2,range), dur, level, sn );
  //affect_to_char( victim, &af );
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,hitroll,ADD,-number_range(2,range));
  addaff(af,CHAR,damroll,ADD,-number_range(2,range));
  affect_to_char( victim, &af );

  if (victim->position == POS_FIGHTING) do_flee(victim,"");
  if (victim->position == POS_FIGHTING) do_flee(victim,"");
  if (victim->position == POS_FIGHTING) do_flee(victim,"");

  if (bad_fail) WAIT_STATE(victim,12);

  return;
}

void spell_pool_of_blood( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *vch, *vch_next;
  AFFECT_DATA af;

  if ( ch->in_room->cstat(sector) == SECT_UNDERWATER 
       || ch->in_room->cstat(sector) == SECT_WATER_SWIM 
       || ch->in_room->cstat(sector) == SECT_WATER_NOSWIM
       || ch->in_room->cstat(sector) == SECT_AIR ) {
    send_to_char( "There is no earth around!\n\r", ch );
    return;
  }
  send_to_char("You create a pool of blood.\n\r", ch );
  act("$n summons a pool of blood from the bowels of the earth.", ch, NULL, NULL, TO_ROOM );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( is_same_group( vch, ch ) || is_affected( vch, sn ) || is_safe_spell(ch,vch,TRUE))
      continue;

    if ( (IS_NPC(vch) && IS_SET(vch->act,ACT_UNDEAD) )
	 || IS_SET( vch->cstat(form),FORM_UNDEAD)
	 || saves_spell( level, vch,DAM_NEGATIVE))
      continue;

    if ( IS_AFFECTED(vch,AFF_SLOW))
      continue;

    send_to_char("You slip in the pool.\n\r", vch );
    act("$N slips in the pool.", ch, NULL, vch, TO_CHAR );
 
    if (IS_AFFECTED(vch,AFF_HASTE)) {
      if (!check_dispel(level,vch,gsn_haste)) {
	send_to_char("You feel momentarily slower.\n\r",vch);
	act("$N feels momentarily slower.", ch, NULL, vch, TO_CHAR );
	continue;
      }
      act("$n is moving less quickly.",vch,NULL,NULL,TO_ROOM);
      continue;
    }
    //afsetup(af,CHAR,affected_by,OR,AFF_SLOW,level/5,level,sn);
    //affect_to_char( vch, &af );
    //afsetup(af,CHAR,DEX,ADD,-1 - (level >= 18) - (level >= 25) - (level >= 32),
    //	       level/5,level,sn);
    //affect_to_char( vch, &af );
    //afsetup(af,CHAR,allAC,ADD,20+(level*2)/3,level/5,level,sn);
    //affect_to_char(vch,&af);
    createaff(af,level/5,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,DEX,ADD,-1 - (level >= 18) - (level >= 25) - (level >= 32));
    addaff(af,CHAR,allAC,ADD,20+(level*2)/3);
    addaff(af,CHAR,affected_by,OR,AFF_SLOW);
    affect_to_char( vch, &af );

    send_to_char( "You feel yourself slowing d o w n...\n\r", vch );
    act("$n starts to move in slow motion.",vch,NULL,NULL,TO_ROOM);
  }
  return;
}

void spell_phylactery( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("You already have a phylactery.\n\r", ch );
    else
      act("$N already has a phylactery.", ch, NULL, victim, TO_CHAR );
    return;
  }

  int hp;
  int perc = number_percent();
  if ( perc <= 20 )      // 20% chance getting a really really poor phylactery: only 1 hp stored
    hp = 1;
  else if ( perc <= 50 ) // 30% chance getting a mean phylactery: 1/2 hp
    hp = UMAX( victim->hit/2, 1 );
  else if ( perc <= 80 ) // 30% chance getting a good phylactery: 3/4 hp
    hp = UMAX( (3*victim->hit)/4, 1 );
  else                   // 20% chance getting a perfect phylactery: all hp
    hp = victim->hit;

  AFFECT_DATA af;
  int timer = number_range(10,level/3);
  //afsetup( af, CHAR, NA, ADD, hp, timer, level, sn ); 
  //affect_to_char(ch, &af);
  createaff(af,timer,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,NA,ADD,hp);
  affect_to_char( victim, &af );
  
  send_to_char("You store a portion of life essence in a Phylactery.\n\r", victim );
  act("$N is surrounded by a {rCrimson Aura{x.", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_invincible_mount( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( !IS_NPC(victim)
       || !get_mount_master(victim) ) {
    send_to_char("This spell can only be used on mount.\n\r", ch );
    return;
  }

  if ( is_affected( victim, sn ) ) {
    act("$N is already an invincible mount.", ch, NULL, victim, TO_CHAR );
    return;
  }

  AFFECT_DATA af;
  int dur = number_range( level/2, level*2);
  //afsetup(af,CHAR,allAC,ADD,-20-(level*2)/3,dur,level,sn);
  //affect_to_char(victim, &af);
  createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,-20-(level*2)/3);
  affect_to_char( victim, &af );

  act("$N is now an invincible mount.", ch, NULL, victim, TO_CHAR );
  act("You are now an invincible mount.", ch, NULL, victim, TO_VICT );
  act("$N is now an invincible mount.", ch, NULL, victim, TO_NOTVICT );
  return;
}

void spell_strategic_retreat( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if ( is_affected( victim, sn ) ) {
    if ( ch == victim )
      send_to_char("You already know how to get a path of retreat.\n\r", ch );
    else
      act("$N already knows how to get a path of retreat.", ch, NULL, victim, TO_CHAR );
    return;
  }

  AFFECT_DATA af;
  int timer = number_range(10,level/3);
  //afsetup( af, CHAR, NA, ADD, 0, timer, level, sn ); 
  //affect_to_char( ch, &af );
  createaff(af,timer,level,sn,casting_level,AFFECT_ABILITY);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( victim, &af );

  send_to_char("You now know how to get a path of retreat.\n\r", victim );
  act("$n now know how to find a path of retreat.", ch, NULL, NULL, TO_ROOM );
  return;
}

void spell_speak_with_dead(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  OBJ_DATA *corpse;

  if ( target_name[0] == '\0' ) {
    send_to_char("Speak with which corpse?\n\r",ch);
    return;
  }
  
  corpse = get_obj_here(ch,target_name);
  
  if (corpse == NULL) {
    send_to_char("You can't find that object.\n\r",ch);
    return;
  }
  
  if ((corpse->item_type != ITEM_CORPSE_NPC) 
      && (corpse->item_type != ITEM_CORPSE_PC)) {
    send_to_char("You can't speak with that.\n\r",ch);
    return;
  }
  if ( !can_loot( ch, corpse ) ) {
    act("You are not allowed to speak with that corpse.",ch,corpse,NULL,TO_CHAR);
    return;
  }

  // SinaC 2003: if corpse has no head -> no talk   or was not sentient
  if ( !IS_SET( corpse->baseval[2], PART_HEAD ) || !IS_SET( corpse->baseval[1], FORM_SENTIENT ) ) {
    act("$p cannot speak.", ch, corpse, NULL, TO_CHAR );
    return;
  }

  char field[512];
  Value *v = get_extra_field( corpse, "killer_name" );
  bool noValue = FALSE;
  if ( v != NULL )
    strcpy( field, v->asStr() );
  else
    noValue = TRUE;
  if ( noValue || field == NULL || field[0] == '\0' ) {
    act( "$p begins to move slowly and says 'I don't know who killed me.'", 
	 ch, corpse, NULL, TO_ALL );
    return;
  }
  if ( !str_cmp( field, "myself" ) ) {
    act( "$p begins to move slowly and says 'I killed myself.'", 
	 ch, corpse, NULL, TO_ALL );
    return;
  }
  act("$p begins to move slowly and says 'I have been killed by '$T'.'", 
      ch, corpse, field, TO_ALL );
  return;
}

void spell_enchant_wand( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *)vo;

  // It's a wand?
  if ( obj->item_type != ITEM_WAND ) {
    send_to_char("You can only use this spell on an empty wand.\n\r", ch );
    return;
  }

  // Already enchanted?
  for ( int i = 0; i < 5; i++ )
    if ( obj->baseval[i] > 0
	 || obj->value[i] > 0 ) {
      act("$p is already enchanted.", ch, obj, NULL, TO_CHAR );
      return;
    }

  // Find the spell
  int spell_sn = ability_lookup( add_target_name );
  if ( spell_sn <= 0 ) {
    send_to_char("This is not an existing spell.\n\r", ch );
    return;
  }
  ability_type *sk = &(ability_table[spell_sn]);
  if ( sk->type != TYPE_SPELL ) {
    send_to_char("This is not a spell.\n\r", ch );
    return;
  }
  if ( !sk->craftable ) {
    act("You cannot enchant $p with that spell.", ch, obj, NULL, TO_CHAR );
    return;
  }
  int perc = get_ability( ch, spell_sn );
  if ( perc == 0 ) {
    send_to_char("You don't know that spell.\n\r", ch );
    return;
  }

  // Miss spell check?
  if ( number_percent() > perc ) {
    act("You failed to enchant $p with '$T'.", ch, obj, sk->name, TO_CHAR );
    act("$n failed to enchant $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  // Okay, let's go
  //  "[v0] Level:          [%d]\n\r"
  //  "[v1] Charges Total:  [%d]\n\r"
  //  "[v2] Charges Left:   [%d]\n\r"
  //  "[v3] Spell:          %s\n\r",
  act("You successfully enchant $p with '$T'.", ch, obj, sk->name, TO_CHAR );
  act("$n enchants successfully $p.", ch, obj, NULL, TO_ROOM );

  obj->baseval[0] = UMAX( 1, number_fuzzy(level-5));
  obj->baseval[1] = number_range( 1, 10 );
  obj->baseval[2] = obj->baseval[1];
  obj->baseval[3] = spell_sn;
  for ( int i = 0; i < 5; i++ )
    obj->value[i] = obj->baseval[i];

  char buf[MAX_STRING_LENGTH];
  sprintf(buf,"A wand of '%s'", sk->name );
  obj->short_descr = str_dup( buf );
}

void spell_enchant_staff( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  OBJ_DATA *obj = (OBJ_DATA *)vo;

  // It's a staff?
  if ( obj->item_type != ITEM_STAFF ) {
    send_to_char("You can only use this spell on an empty staff.\n\r", ch );
    return;
  }

  // Already enchanted?
  for ( int i = 0; i < 5; i++ )
    if ( obj->baseval[i] > 0
	 || obj->value[i] > 0 ) {
      act("$p is already enchanted.", ch, obj, NULL, TO_CHAR );
      return;
    }

  // Find the spell
  int spell_sn = ability_lookup( add_target_name );
  if ( spell_sn <= 0 ) {
    send_to_char("This is not an existing spell.\n\r", ch );
    return;
  }
  ability_type *sk = &(ability_table[spell_sn]);
  if ( sk->type != TYPE_SPELL ) {
    send_to_char("This is not a spell.\n\r", ch );
    return;
  }
  if ( !sk->craftable ) {
    act("You cannot enchant $p with that spell.", ch, obj, NULL, TO_CHAR );
    return;
  }
  int perc = get_ability( ch, spell_sn );
  if ( perc == 0 ) {
    send_to_char("You don't know that spell.\n\r", ch );
    return;
  }

  // Miss spell check?
  if ( number_percent() > perc ) {
    act("You failed to enchant $p with '$T'.", ch, obj, sk->name, TO_CHAR );
    act("$n failed to enchant $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  // Okay, let's go
  //  "[v0] Level:          [%d]\n\r"
  //  "[v1] Charges Total:  [%d]\n\r"
  //  "[v2] Charges Left:   [%d]\n\r"
  //  "[v3] Spell:          %s\n\r",
  act("You successfully enchant $p with '$T'.", ch, obj, sk->name, TO_CHAR );
  act("$n enchants successfully $p.", ch, obj, NULL, TO_ROOM );

  obj->baseval[0] = UMAX( 1, number_fuzzy(level-5));
  obj->baseval[1] = number_range( 1, 10 );
  obj->baseval[2] = obj->baseval[1];
  obj->baseval[3] = spell_sn;
  for ( int i = 0; i < 5; i++ )
    obj->value[i] = obj->baseval[i];

  char buf[MAX_STRING_LENGTH];
  sprintf(buf,"A staff of '%s'", sk->name );
  obj->short_descr = str_dup( buf );
  return;
}

void spell_inferno(int sn,int level,CHAR_DATA *ch,void *vo,int target, int casting_level) {
  send_to_char("You summon a wall of incredible flame and fire.\n\r", ch );
  act("$n summons a wall of incredible flame and fire.", ch, NULL, NULL, TO_ROOM );

  CHAR_DATA *vch, *vch_next;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    // and not in the same group as caster
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;

    if ( saves_spell( level, vch, DAM_FIRE ) 
	 && chance(20)
	 && casting_level == 5 ) {
      send_to_char("The fire consumes you causing instantaneous death.\n\r", ch );
      sudden_death( ch, vch );
      continue;
    }

    send_to_char("The fire consumes you.\n\r", vch );
    int dam = dice( (level*casting_level)/5, number_fuzzy(8) );
    if ( saves_spell( level, vch, DAM_FIRE ) )
      ability_damage( ch, vch, dam/2, sn, DAM_FIRE, TRUE, TRUE );
    else
      ability_damage( ch, vch, dam, sn, DAM_FIRE, TRUE, TRUE );
  }
}

/*
'DETECT LYCANTHROPY'
Syntax: cast 'detect lycanthropy' <target>

Spheres: DIVINATION, DRUIDIC

This spell enables the caster to detect whether the target character is a
lycanthrope, or if the target character has become infected with a form
of lycanthropy.  For more information on lycanthropy, read 'help
lycanthropy'.
*/
void spell_detect_lycanthropy( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;
  
  if ( is_affected( victim, gsn_lycanthropy ) )
    if ( victim == ch )
      send_to_char("You are infected by lycanthropy.\n\r", ch );
    else
      act("$N is infected by lycanthropy.", ch, NULL, victim, TO_CHAR );
  else
    if ( victim == ch )
      send_to_char("You are not infected by lycanthropy.\n\r", ch );
    else
      act("$N is not infected by lycanthropy.", ch, NULL, victim, TO_CHAR );
  return;
}

void spell_conjure_mount( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  int casting_level_riding = get_casting_level( ch, gsn_riding );

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  OBJ_DATA *saddle = NULL;
  if ( casting_level_riding < 2 ) { // only casting level 1 needs a saddle
    for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
      if ( obj->wear_loc == WEAR_NONE && obj->item_type == ITEM_SADDLE )
	saddle = obj;
    }
    if ( saddle == NULL ) {
      send_to_char("You need a saddle before conjuring a mount.\n\r", ch );
      return;
    }
  }
  int percent;
  if ( ( percent = get_ability( ch, gsn_riding ) ) == 0 ) {
    send_to_char("You need to be skilled in riding before conjuring a mount.\n\r", ch );
    return;
  }
  if ( number_percent() > percent ) {
    send_to_char("You failed.\n\r", ch);
    return;
  }

  MOB_INDEX_DATA *pMob = get_mob_index( MOB_VNUM_CONJURE_MOUNT );
  if ( pMob == NULL ) {
    send_to_char("Something is wrong with this spell. Warn an IMP.\n\r", ch );
    bug("CONJURE_MOUNT: missing MOB_VNUM_CONJURE_MOUNT (vnum: %d)", MOB_VNUM_CONJURE_MOUNT );
    return;
  }

  CHAR_DATA *mount = create_mobile( pMob );
  mount->bstat(size) = ch->bstat(size);
  mount->level = (level*2)/3;
  mount->bstat(max_hit) = UMAX((dice(mount->level, 15))+(mount->level*20),1);
  mount->hit = mount->bstat(max_hit);
  mount->bstat(DICE_TYPE) = UMAX(1, mount->level/3);
  if ( mount->level >= 1  && mount->level <= 10 )
    mount->bstat(DICE_NUMBER) = 4;
  if ( mount->level >= 11 && mount->level <= 20 )
    mount->bstat(DICE_NUMBER) = 5;
  if ( mount->level >= 21 && mount->level <= 30 )
    mount->bstat(DICE_NUMBER) = 6;
  if ( mount->level > 30 )
    mount->bstat(DICE_NUMBER) = 7;
  mount->bstat(alignment) = 0;
  mount->bstat(etho) = ETHO_LAWFUL;
  mount->bstat(ac0+AC_PIERCE) = mount->cstat(ac0+AC_PIERCE) =
    mount->bstat(ac0+AC_BASH) = mount->cstat(ac0+AC_BASH) =
    mount->bstat(ac0+AC_SLASH) = mount->cstat(ac0+AC_SLASH) = -2*mount->level;
  mount->bstat(ac0+AC_EXOTIC) = mount->cstat(ac0+AC_EXOTIC) = -mount->level;
  mount->bstat(hitroll) = mount->level/4;
  mount->bstat(damroll) = mount->level/4;

  char buf[MAX_STRING_LENGTH];
  if ( target_name[0] != '\0' ) {
    sprintf( buf, "%s %s", mount->name, target_name );
    mount->name = str_dup( buf );
  }
  
  if ( target_name[0] == '\0' )
    sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	     mount->description, ch->name );
  else
    sprintf( buf, "%sA neck tag says 'My name is '%c%s' and I belong to %s'.\n\r",
	     mount->description, UPPER(target_name[0]), target_name+1, ch->name );
  mount->description = str_dup( buf );
  

  SET_BIT(mount->act, ACT_PET);
  SET_BIT(mount->bstat(affected_by), AFF_CHARM); SET_BIT(mount->cstat(affected_by), AFF_CHARM);
  SET_BIT(mount->act, ACT_IS_MOUNTED);
  SET_BIT(mount->act, ACT_CREATED ); // SinaC 2003

  char_to_room(mount,ch->in_room);
  act("$n conjures a mount.",ch,NULL,NULL,TO_ROOM);
  act("You conjure a mount.",ch,NULL,NULL,TO_CHAR);

  check_mount_class(mount);

  add_follower( mount, ch );
  mount->leader = ch;
  ch->pet = mount;

  if ( saddle ) {
    act("You place your saddle on $N's back and ride it.", ch, NULL, mount, TO_CHAR );
    act("$n places $s saddle on $N's back and ride it.", ch, NULL, mount, TO_NOTVICT );
    act("$n places $s saddle on your back and ride you.", ch, NULL, mount, TO_VICT );
    obj_from_char(saddle);
    obj_to_char(saddle,mount);
  }
  else {
    act("You ride $N.", ch, NULL, mount, TO_CHAR );
    act("$n rides $N.", ch, NULL, mount, TO_NOTVICT );
    act("$n rides you.", ch, NULL, mount, TO_VICT );
  }
  //  recompute(mount); NOT NEEDED
  return;
}

/*
'SUMMON ANGEL'
Syntax: cast 'summon angel'

Sphere: ANGELIC

This spell summons a holy, divine angel to be a companion of the caster.
The power of the angel is roughly equivalent to that of the caster.  This
spell should not be cast by those of evil or neutral persuasions, less
they suffer the consequences.
*/
void spell_summon_angel( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  send_to_char("Not yet implemented.\n\r", ch );
  return;
}

/*
'SUMMON DEMON'
Syntax: cast 'summon demon'

Sphere: DAEMONIC

This spell summons an unholy, evil demon to be a companion of the caster.
The power of the demon is roughly equivalent to that of the caster.  This
spell should not be cast by those of good or neutral persuasions, less
they suffer the consequences.
*/
void spell_summon_demon( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  send_to_char("Not yet implemented.\n\r", ch );
  return;
}

void spell_faerie_aura( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  //newafsetup(af,CHAR,allAC,ADD,2*level,level,level,sn,casting_level);
  //affect_to_char( victim, &af );
  createaff(af,level,level,sn,casting_level,AFFECT_ABILITY);
  addaff(af,CHAR,allAC,ADD,2*level);
  affect_to_char( victim, &af );

  send_to_char( "You are surrounded by a faerie aura.\n\r", victim );
  act( "$n is surrounded by a faerie aura.", victim, NULL, NULL, TO_ROOM );
  return;
}

void spell_word_of_recall( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  do_recall( (CHAR_DATA *) vo, target_name );
  return;
}

void spell_protection_lycanthropy( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  AFFECT_DATA af;

  createaff(af,level/2,level,sn,casting_level,AFFECT_ABILITY);
  affect_to_char( victim, &af );

  send_to_char( "You are now protected from lycanthropy infection.\n\r", victim );
  act( "$n is now protected from lycanthropy infection.", victim, NULL, NULL, TO_ROOM );
  return;
}

/*
CONFUSION
Syntax: cast 'confusion' <victim>

Sphere: MISCHIEF

School: ENCHANTMENT
Max Spell Level: 0

This spell confuses the victim for a few minutes.  The victim will be uncertain
who his or her friends are, and who his or her foes are.  He or she will begin
to randomly attack any target in the room each round that this spell remains in
effect.

Prerequisites: None
*/
void spell_confusion( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
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
       || saves_spell( level, victim, DAM_CHARM) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  if (IS_SET(victim->in_room->cstat(flags),ROOM_LAW)
      && !IS_IMMORTAL(ch)) {
    send_to_char("The mayor does not allow using this in the city limits.\n\r",ch);
    return;
  }
  
  createaff(af,number_range( 4, number_fuzzy( level / 10 ) ), level,sn,0,AFFECT_ABILITY);
  addaff(af,CHAR,affected2_by,OR,AFF2_CONFUSION);
  affect_to_char( victim, &af );

  act("$N looks confused.", ch, NULL, victim, TO_CHAR );
  act("$N looks confused.", ch, NULL, victim, TO_NOTVICT );
  return;
}

/*
'CHAOTIC BLAST'
Syntax: cast 'chaotic blast' <victim>

Sphere: MISCHIEF

Chaotic blast is a powerful spell granted to the most powerful clerics of
Yuri Longshadow, Goddess of Mischief.  One casting of this spell will
cast up to five randomly selected spells known by the cleric on his or
her opponent.
*/
void spell_chaotic_blast( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  int random_spell[5];
  int cst_lvl_spell[5];
  int nb_random_spell;
  for ( nb_random_spell = 0; nb_random_spell < 5; nb_random_spell++ ) { // get 5 random spells known by the caster
    int start = number_range(1,MAX_ABILITY);
    int rsn = start;
    //    log_stringf("chaotic_blast: start: %d", rsn );
    while(1) {
      //      if ( ch->pcdata->ability_info[rsn].learned > 0
      //	   && ability_table[rsn].type == TYPE_SPELL // only spell
      //	   && !ability_table[rsn].scriptAbility
      //	   && rsn != sn )
      //	log_stringf("chaotic_blast: [%s]  %d  %d  %d[%d]",
      //		    ability_table[rsn].name,
      //		    ch->pcdata->ability_info[rsn].learned,
      //		    ability_table[rsn].target,
      //		    ability_table[rsn].minimum_position,
      //		    ch->position);
      if ( ( ( !IS_NPC(ch) && ch->pcdata->ability_info[rsn].learned > 0 ) // known spell
	     || IS_NPC(ch) )
	   && rsn != sn // cannot cast chaotic blast again
	   && ability_table[rsn].type == TYPE_SPELL // only spell
	   && !ability_table[rsn].scriptAbility // no script ability
	   && ( ability_table[rsn].target == TAR_CHAR_OFFENSIVE // offensive or defensive spells
		|| ability_table[rsn].target == TAR_CHAR_DEFENSIVE
		|| ability_table[rsn].target == TAR_OBJ_CHAR_OFF
		|| ability_table[rsn].target == TAR_OBJ_CHAR_DEF )
	   && ability_table[rsn].minimum_position <= ch->position ) { // position ok
	cst_lvl_spell[nb_random_spell] = ch->pcdata->ability_info[rsn].casting_level;
	random_spell[nb_random_spell] = rsn;
	break;
      }
      rsn = (rsn+1) % MAX_ABILITY;
      if ( rsn == start ) { // if we have made a whole loop -> caster has less than 5 spells
	rsn = -1;
	break;
      }
    }
    if ( rsn == -1 ) // stop if cast has less than 5 spells
      break;
  }

  // Cast the spells
  for ( int i = 0; i < nb_random_spell; i++ ) {
    // FIXME: add ambient sentence
    int rsn = random_spell[i];
    int cst_lvl = cst_lvl_spell[i];
    (*(SPELL_FUN*)ability_table[rsn].action_fun) ( rsn, level, ch, vo, target, cst_lvl );
  }

  return;
}

/*
'CREATE VINES'
Syntax: cast 'create vines'

Sphere: NATURE, PLANT

This spell creates a length of vine.  This vine functions as a rope.
Ropes are needed to pass between certain rooms.  See 'help climb' for
more information.
*/
void spell_create_vines( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  int chance = 0;

  switch( ch->in_room->cstat(sector) ) {
  case SECT_INSIDE: chance = 25; break;
  case SECT_CITY: chance = 30; break;
  case SECT_FIELD: chance = 75; break;
  case SECT_FOREST: chance = 100; break;
  case SECT_HILLS: chance = 75; break;
  case SECT_MOUNTAIN: chance = 50; break;
  case SECT_WATER_SWIM:
  case SECT_WATER_NOSWIM:
  case SECT_BURNING:
  case SECT_AIR: chance = 0; break;
  case SECT_DESERT: chance = 15; break;
  case SECT_UNDERWATER: chance = 0; break;
  default: chance = 0; break;
  }

  if ( chance == 0 ) {
    send_to_char("This spell is useless in this kind of environment.\n\r", ch );
    return;
  }

  if ( chance < number_percent() ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  OBJ_INDEX_DATA *pObjIndex = get_obj_index( OBJ_VNUM_VINES );
  if ( pObjIndex == NULL ) {
    bug("OBJ_VNUM_VINES [%d] doesn't exist", OBJ_VNUM_VINES );
    send_to_charf(ch,"spell_create_vines: please warn an immortal");
    return;
  }
  OBJ_DATA *vines = create_object( pObjIndex, 0 );
  obj_to_room(vines,ch->in_room);
  vines->timer = UMIN( 5, level/2 );

  act("{gWild vines{x grow from the ground.{x\n\r", ch, NULL, NULL, TO_ROOM  );
  act("{gWild vines{x grow from the ground.{x\n\r", ch, NULL, NULL, TO_CHAR  );
  return;
}

/*
FLARE
Syntax: cast 'flare' <victim>

Flare allows the caster to summon the power of his or her god to surround
both the caster and the target character that the caster is currently
fighting.  While affected by the flare, neither the caster nor the victim
can take any combat actions; however, other members of the caster's group
may continue their attacks.  Flare has a relatively short duration,
typically not lasting more than 2 or 3 combat rounds.  It should be noted
that flare will only work if the target is currently attacking the caster,
in other words, if the caster is "tanking".
*/
void spell_flare( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  send_to_char("Not yet implemented.\n\r", ch );
  return;
}

/*
'ANIMAL FOLLOWER 1'
Syntax: cast 'animal follower 1'

This spell allows a ranger to summon a natural creature to him or her.  This
creature will follow and assist the ranger, like a pet, though in truth the
bond is more that of a companion than a servant.

See also, 'help animal follower 2'.

'ANIMAL FOLLOWER 2'
Syntax: cast 'animal follower 2'

This powerful spell is available only to the strongest rangers.  Much like
'animal follower 1', it allows the ranger to summon a companion.  However,
the companion summoned is far from a typical woodlands creature.  Instead,
this spell summons a mighty beast of almost legendary status to assist the
ranger.

See also, 'help animal follower 1'.
*/
void spell_animal_follower( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  MOB_INDEX_DATA *pMobIndex;
  CHAR_DATA *companion;
  int i, clevel;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ( ch->pet != NULL ){
    send_to_char("One familiar at a time is enough!\n\r",ch);
    return;
  }

  if ( casting_level <= 1 ) { // casting level 1 gives a companion based on room sector
    pMobIndex = get_mob_index( MOB_VNUM_FAMILIAR );

    companion = create_mobile(pMobIndex);

    clevel = companion->level = UMAX( 1, number_fuzzy( ch->level-5 ) );
    companion->hit = companion->bstat(max_hit) = number_fuzzy( clevel ) * 40 + 10;
    companion->mana = companion->bstat(max_mana) = 0;
    // Added by SinaC 2001 for mental user
    companion->psp = companion->bstat(max_psp) = 0;
    companion->move = companion->bstat(max_move) = number_fuzzy( clevel ) * 10 + 100;

    for ( i=0; i < MAX_STATS; i++)
      companion->bstat(stat0+i)=16+clevel/12;

    for(i = 0; i < 4; i++)
      companion->bstat(ac0+i) = number_fuzzy(clevel * -3 + 20 );

    companion->bstat(hitroll) = 
      companion->bstat(damroll) = UMAX( 1, number_fuzzy(clevel / 2) );

    companion->bstat(DICE_NUMBER)=UMAX(clevel/20,2);
    companion->bstat(DICE_TYPE)=UMAX(clevel/2,2);
    companion->bstat(DICE_BONUS)=UMAX(clevel/5,2);


    // Choose a companion based on room sector
    // Modified by SinaC 2001
    int race;
    switch ( ch->in_room->cstat(sector) ) {
    case ( SECT_INSIDE ) :
      companion->name = str_dup("giant cockroach familiar");
      companion->short_descr = str_dup("a giant cockroach");
      companion->long_descr = str_dup("A giant cockroach scuttles about here.\n\r");
      companion->description =
	str_dup("This grisly cockroach is larger than any you have ever\n\r"
		"seen, perhaps even big enough to tear you up and digest\n\r"
		"you in his sick little way.\n\r");
      race = race_lookup( "centipede", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: centipede");
      }
      change_race(companion,race,-1,clevel,-1,0);
      companion->bstat(dam_type) = 22;          /* scratch */
      break;
    
    case ( SECT_CITY ) :
      companion->name = str_dup("vicious sewer rat familiar");
      companion->short_descr = str_dup("a vicious sewer rat");
      companion->long_descr = str_dup("A vicious sewer rat peers around with beady eyes.\n\r");
      companion->description =
	str_dup("This vile animal looks (and smells) like it has crawled\n\r"
		"out of a sewer to where he stands now. Absolutely gross.\n\r");
      race = race_lookup( "rat", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: rat");
      }
      change_race(companion,race,-1,clevel,-1,0);
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
      race = race_lookup( "fox", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: fox");
      }
      change_race(companion,race,-1,clevel,-1,0);
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
      race = race_lookup( "bear", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: bear");
      }
      change_race(companion,race,-1,clevel,-1,0);
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
      race = race_lookup( "snake", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: snake");
      }
      change_race(companion,race,-1,clevel,-1,0);
      companion->bstat(dam_type) = 31;          /* acidic bite */
      break;
    
    case ( SECT_WATER_SWIM ) :
    case ( SECT_WATER_NOSWIM ) :
    case ( SECT_UNDERWATER ) :
      companion->name = str_dup("water fowl familiar");
      companion->short_descr = str_dup("a water fowl");
      companion->long_descr = str_dup("A water fowl shark is here, searching for a quick meal.\n\r");
      companion->description =
	str_dup("This water fowl flies about effortlessly, swingings its long tail\n\r"
		"back and forth in smooth motions. It looks slender and quite\n\r"
		"beautiful, but you know its bite is deadly.\n\r");
      race = race_lookup( "water fowl", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: water fowl");
      }
      change_race(companion,race,-1,clevel,-1,0);
      companion->bstat(dam_type) = 32;          // chomp
      break;

    case ( SECT_HILLS ) :
      companion->name = str_dup("hawk familiar");
      companion->short_descr = str_dup("a hawk");
      companion->long_descr = str_dup("A dark-eyed hawk keeps a watchful eye over the room.\n\r");
      companion->description =
	str_dup("This graceful bird is both glamorous and deadly. It has very long\n\r"
		"wings, and short legs with deadly claws. Its sharp beak is stained\n\r"
		"red from the flesh of recent prey.\n\r");
      // Logic isn't it ?
      //SET_BIT(companion->bstat(affected_by),AFF_FLYING);
      race = race_lookup( "song bird", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: song bird");
      }
      change_race(companion,race,-1,clevel,-1,0);
      companion->bstat(dam_type) = 23;          /* peck */
      break;
    
    case ( SECT_MOUNTAIN ) :
      companion->name = str_dup("mountain lion familiar");
      companion->short_descr = str_dup("a mountain lion");
      companion->long_descr = str_dup("A mountain lion paces slowly in circles, smelling the area.\n\r");
      companion->description =
	str_dup("This deadly cat is the epitome of speed, grace, and power. It is made\n\r"
		"made by nature to be a killing machine, and its sharp eyes and sharper\n\r"
		"claws serve it well to this end.\n\r");
      race = race_lookup( "cat", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: cat");
      }
      change_race(companion,race,-1,clevel,-1,0);
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
      // logic isn't it ?
      //SET_BIT(companion->bstat(affected_by),AFF_FLYING);
      race = race_lookup( "wyvern", TRUE );
      if ( race < 0 ) {
	race = race_lookup("unique");
	bug("Animal_follower: invalid race: wyvern");
      }
      change_race(companion,race,-1,clevel,-1,0);
      companion->bstat(dam_type) = 29;          /* flaming bite */
      break;
    }
  }
  else { // casting level 2 gives a special companion
         //  a mighty beast of almost legendary status
    pMobIndex = get_mob_index( MOB_VNUM_FAMILIAR );
    
    companion = create_mobile(pMobIndex);
    
    clevel = companion->level = number_fuzzy( ch->level+2 );
    companion->hit = companion->bstat(max_hit) = number_fuzzy( clevel ) * 65 + 1500;
    companion->mana = companion->bstat(max_mana) = 0;
    companion->psp = companion->bstat(max_psp) = 0;
    companion->move = companion->bstat(max_move) = number_fuzzy( clevel ) * 30 + 500;

    for ( i=0; i < MAX_STATS; i++)
      companion->bstat(stat0+i)=16+clevel/8;

    for(i = 0; i < 4; i++)
      companion->bstat(ac0+i) = number_fuzzy(clevel * -5 );

    companion->bstat(hitroll) = 
      companion->bstat(damroll) = UMAX( 1, number_fuzzy(clevel) );

    companion->bstat(DICE_NUMBER)=UMAX(clevel/11,2);
    companion->bstat(DICE_TYPE)=UMAX((2*clevel)/3,2);
    companion->bstat(DICE_BONUS)=UMAX(clevel/4,2);

    companion->name = str_dup("legendary beast mighty");
    companion->short_descr = str_dup("a mighty beast");
    companion->long_descr = str_dup("A mighty beast waiting an order from its master.\n\r");
    companion->description =
      str_dup("This deadly mighty beast is the epitome of speed, grace, and power. It is made\n\r"
	      "made by nature to be a killing machine, and its sharp eyes and sharper\n\r"
	      "claws serve it well to this end.\n\r");
    int race = race_lookup( "unique", TRUE );
    change_race(companion,race,-1,clevel,-1,0);
    companion->bstat(affected_by) = AFF_REGENERATION | AFF_SWIM | AFF_DARK_VISION | 
      AFF_DETECT_INVIS | AFF_DETECT_HIDDEN | AFF_INFRARED;
    companion->bstat(affected2_by) = AFF2_WALK_ON_WATER | AFF2_WATER_BREATH | AFF2_NOEQUIPMENT;
    companion->bstat(form) = FORM_MAGICAL | FORM_ANIMAL | FORM_FOUR_ARMS;
    companion->bstat(parts) = PART_HEAD | PART_ARMS | PART_LEGS | PART_HEART | 
      PART_BRAINS | PART_GUTS | PART_HANDS | PART_FEET | 
      PART_FINGERS | PART_EAR | PART_EYE | PART_LONG_TONGUE | 
      PART_BODY | PART_CLAWS | PART_FANGS | PART_HORNS | 
      PART_TUSKS;
    companion->bstat(sex) = SEX_NEUTRAL;
    companion->bstat(classes) = 0;
    companion->bstat(imm_flags) = IRV_SUMMON | IRV_WOOD | IRV_DROWNING | IRV_MENTAL;
    companion->bstat(res_flags) = IRV_WEAPON | IRV_BASH | IRV_EARTH;
    companion->bstat(vuln_flags) = IRV_FIRE;
    companion->bstat(saving_throw) = -clevel/9;
    companion->bstat(size) = SIZE_HUGE;
    companion->bstat(dam_type) = 5;          // claw
    companion->bstat(etho) = ETHO_LAWFUL;
    companion->bstat(alignment) = 0;
    companion->act = ACT_IS_NPC | ACT_PET | ACT_WARRIOR;
  }
  
  act("You chant slowly, and $N answers your call!",ch,NULL,companion,TO_CHAR);
  act("$n chants slowly, and $N answers the call!",ch,NULL,companion,TO_ROOM);
  
  SET_BIT(companion->act, ACT_PET);
  SET_BIT(companion->bstat(affected_by), AFF_CHARM);
  SET_BIT(companion->act, ACT_CREATED ); // SinaC 2003
  
  char_to_room( companion, ch->in_room );
  add_follower( companion,ch );
  companion->leader = ch;
  ch->pet = companion;
}

/*
Syntax: cast 'spellfire' <character>

School: ELEMENTAL
Max Casting Level: 0

The most powerful spell available to a general elementalist, spellfire taps
into the raw magic that flows through the elements.  The results are very
unpredictable, though often very devastating to the victim.  Spellfire
causes a random elemental spell from any of the four elemental schools to
be cast upon the victim.  Even if the caster does not know the spell, it
may end up being cast anyhow.  In addition, there is a chance that a non-
elemental spell will target the victim due to the raw magic that is being
tapped.  Spellfire has a chance to cast a beneficial spell on one's
opponent, or a harmful spell when cast upon oneself, though the opposite
affect is more likely (harmful spells on opponent and beneficial on
oneself).

Prerequisites:
-------------
Immolation Level 2, Thunderclap Level 2, Cone of Cold Level 2, Entangle
Level 2
*/
void spell_spellfire( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  if ( target != TARGET_CHAR ) {
    send_to_charf(ch,"You can only cast this spell on a mob or a player.\n\r");
    return;
  }

  int elemental_fire = magic_school_lookup("elemental fire");
  int elemental_water = magic_school_lookup("elemental water");
  int elemental_air = magic_school_lookup("elemental air");
  int elemental_earth = magic_school_lookup("elemental earth");
  if ( elemental_fire < 0
       || elemental_water < 0
       || elemental_air < 0
       || elemental_earth < 0 )
    bug("spell_spellfire: missing elemental schools: fire[%d] water[%d] air[%d] earth[%d]",
	elemental_fire, elemental_water, elemental_air, elemental_earth );

  bool get_non_elemental = FALSE;
  if ( chance(5) ) // 5% chance to get a non-elemental spell
    get_non_elemental = TRUE;

  bool reverse = FALSE;
  if ( chance(10) ) // 10% chance to get a offensive spell instead of defensive and vice versa
    reverse = TRUE;

  //log_stringf("spellfire: non-elemental [%d]   reverse [%d]", get_non_elemental, reverse );

  CHAR_DATA *victim = (CHAR_DATA *)vo;
  int tar1, tar2;
  if ( victim == ch        // target is self: defensive
       || ( !IS_NPC(victim) && silent_is_safe_spell(ch,victim,FALSE) )  // target is player and is safe
       || is_same_group( ch, victim) ) {  // target is the same group
    //log_stringf("spellfire: [%s] FRIEND", NAME(victim) );
    tar1 = TAR_CHAR_DEFENSIVE;
    tar2 = TAR_OBJ_CHAR_DEF;
    // switch to offensive only if self
    if ( reverse && victim == ch ) {
      tar1 = TAR_CHAR_OFFENSIVE;
      tar2 = TAR_OBJ_CHAR_OFF;
    }
  }
  else { // not-in-same-group mob or non-safe player
    //log_stringf("spellfire: [%s] ENEMY", NAME(victim) );
    tar1 = TAR_CHAR_OFFENSIVE;
    tar2 = TAR_OBJ_CHAR_OFF;
    if ( reverse ) {
      tar1 = TAR_CHAR_DEFENSIVE;
      tar2 = TAR_OBJ_CHAR_DEF;
    }
  }
  //log_stringf("spellfire: target [%d]  [%d]", tar1, tar2 );

  int count = 0; // count number of available spell
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ability_type *sk = &(ability_table[i]);
    if ( sk->type == TYPE_SPELL                  // get a spell
	 && ( get_non_elemental                  // if non-elemental
	      || sk->school == elemental_fire    // or elemental
	      || sk->school == elemental_water
	      || sk->school == elemental_air
	      || sk->school == elemental_earth )
	 && ( sk->target == tar1                 // right target
	      || sk->target == tar2 )
	 && ( i != sn ) )                        // can't cast spellfire
      count++;
  }
  int which = number_range( 0, count-1 ) + 1;
  int whichSpell = -1;

  //log_stringf("spellfire: count [%d]  which [%d]", count, which );

  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ability_type *sk = &(ability_table[i]);
    if ( sk->type == TYPE_SPELL                  // get a spell
	 && ( get_non_elemental                  // if non-elemental
	      || sk->school == elemental_fire    // or elemental
	      || sk->school == elemental_water
	      || sk->school == elemental_air
	      || sk->school == elemental_earth )
	 && ( sk->target == tar1                 // right target
	      || sk->target == tar2 )
	 && ( i != sn )                          // can't cast spellfire
	 && (--which == 0 ) ) {                  // found the right spell
	whichSpell = i;
	break;
    }
  }

  //log_stringf("spellfire: count [%d]  which [%d]", count, which );

  if ( whichSpell == -1 ) {
    send_to_char("Nothing happens.\n\r",ch);
    bug("spellfire: can't find a valid spell count [%d]  which [%d]", count, which );
    return;
  }
  
  int cast_lvl = 0;
  if ( ability_table[whichSpell].nb_casting_level > 0 )
    cast_lvl = number_range( 1, ability_table[whichSpell].nb_casting_level );

  log_stringf("spellfire: [%s]  casting_level [%d]", ability_table[whichSpell].name, cast_lvl );

  if ( ability_table[whichSpell].scriptAbility ) { // script ability
    Value args[] = { ability_table[whichSpell].name, level, 
		     victim,
		     cast_lvl, target_name };
    int res = mobprog( ch, NULL, "onCast", args );
    if ( !res ) { // don't know this script spell
      send_to_char("Nothing happens.\n\r",ch);
      return;
    }
  }
  else // normal ability
    (*(SPELL_FUN*)ability_table[whichSpell].action_fun) ( whichSpell, level, ch, vo, target, cast_lvl );

  if ( (ability_table[whichSpell].target == TAR_CHAR_OFFENSIVE
	|| ability_table[whichSpell].target == TAR_OBJ_CHAR_OFF )
       &&   victim != ch
       &&   victim->master != ch) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;
    
    if ( victim != NULL )
      multi_hit( ch, victim, TYPE_UNDEFINED );

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
'SUMMON ELEMENTAL'
Syntax: cast 'summon elemental'

School: ELEMENTAL
Max Casting Level: 0

This powerful spell is used to summon an elemental creature to assist
the caster.  The strength of the being summoned is relative to the
strength of the caster.  The type of being summoned (fire, air, water, or
earth elemental) depends on the caster's specialty.  Non-elementalists
and general elementalists that study this spell should do so with caution,
as the elementals have been known to turn against such individuals.

Prerequisites:
-------------
Elemental Field
*/
void spell_summon_elemental( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ( ch->pet != NULL ){
    send_to_char("One familiar at a time is enough!\n\r",ch);
    return;
  }

  int air = class_lookup( "air-elementalist" );
  int fire = class_lookup( "fire-elementalist" );
  int water = class_lookup( "water-elementalist" );
  int earth = class_lookup( "earth-elementalist" );
  if ( air < 0 || fire < 0 | water < 0 || earth < 0 )
    bug("spell_summon_elemental: invalid class air[%d] fire[%d] water[%d] earth[%d]",
	air, fire, water, earth );

  MOB_INDEX_DATA *index = get_mob_index( MOB_VNUM_ELEMENTAL );
  if ( index == NULL ) {
    bug("spell_summon_elemental: invalid mob index [%d]", MOB_VNUM_ELEMENTAL );
    return;
  }

  CHAR_DATA *elemental = create_mobile( index );

  int chance;
  int type;
  if ( IS_SET( ch->cstat(classes), 1<<earth ) ) { // earth elemental
    chance = 100;
    type = 1;
  }
  else if ( IS_SET( ch->cstat(classes), 1<<fire ) ) { // fire elemental
    chance = 100;
    type = 2;
  }
  else if ( IS_SET( ch->cstat(classes), 1<<water ) ) { // water elemental
    chance = 100;
    type = 3;
  }
  else if ( IS_SET( ch->cstat(classes), 1<<air ) ) { // air elemental
    chance = 100;
    type = 4;
  }
  else { // random elemental
    chance = (3*ch->level)/4; // probability for the elemental to attack caster
    type = number_range( 1, 4 );
  }

  switch( type ) {
  case 1: // earth
    elemental->name = str_dup("earth elemental familiar");
    elemental->short_descr = str_dup("an earth elemental");
    elemental->long_descr = str_dup("An earth elemental is here.\n\r");
    elemental->description = str_dup("An earth made humanoid with black eyes.\n\r");
    elemental->bstat(imm_flags) = IRV_EARTH;
    elemental->bstat(dam_type) = attack_lookup("crush");
    SET_BIT( elemental->bstat(classes), 1 << earth );
    break;
  case 2: // fire
    elemental->name = str_dup("fire elemental familiar");
    elemental->short_descr = str_dup("a fire elemental");
    elemental->long_descr = str_dup("A fire elemental is here.\n\r");
    elemental->description = str_dup("A fire made humanoid with black eyes.\n\r");
    elemental->bstat(imm_flags) = IRV_FIRE;
    elemental->bstat(dam_type) = attack_lookup("flbite");
    SET_BIT( elemental->bstat(classes), 1 << fire );
    break;
  case 3: // water
    elemental->name = str_dup("water elemental familiar");
    elemental->short_descr = str_dup("a water elemental");
    elemental->long_descr = str_dup("A water elemental is here.\n\r");
    elemental->description = str_dup("A water made humanoid with black eyes.\n\r");
    elemental->bstat(imm_flags) = IRV_DROWNING;
    elemental->bstat(dam_type) = attack_lookup("frbite");
    SET_BIT( elemental->bstat(classes), 1 << water );
    break;
  case 4: // air
    elemental->name = str_dup("air elemental familiar");
    elemental->short_descr = str_dup("an air elemental");
    elemental->long_descr = str_dup("An air elemental is here.\n\r");
    elemental->description = str_dup("An air made humanoid with black eyes.\n\r");
    elemental->bstat(imm_flags) = IRV_LIGHTNING;
    elemental->bstat(dam_type) = attack_lookup("shbite");
    SET_BIT( elemental->bstat(classes), 1 << air );
    break;
  default: // bug, should never happen
    elemental->name = str_dup("elemental familiar");
    elemental->short_descr = str_dup("an elemental");
    elemental->long_descr = str_dup("An elemental is here.\n\r");
    elemental->description = str_dup("An humanoid with black eyes.\n\r");
    elemental->bstat(dam_type) = attack_lookup("crush");
    break;
  }

  elemental->level = level;
  elemental->hit = elemental->bstat(max_hit) = number_fuzzy( level ) * 40 + 10;
  elemental->mana = elemental->bstat(max_mana) = number_fuzzy( level ) * 40 + 10;
  elemental->psp = elemental->bstat(max_psp) = 0;
  elemental->move = elemental->bstat(max_move) = number_fuzzy( level ) * 10 + 100;
  
  for ( int i = 0; i < MAX_STATS; i++ )
    elemental->bstat(stat0+i)=16+level/12;
  
  for ( int i = 0; i < 4; i++ )
    elemental->bstat(ac0+i) = number_fuzzy(level * -3 + 20 );
  
  elemental->bstat(hitroll) = 
    elemental->bstat(damroll) = UMAX( 1, number_fuzzy(level / 2) );
  
  elemental->bstat(DICE_NUMBER)=UMAX(level/20,2);
  elemental->bstat(DICE_TYPE)=UMAX(level/2,2);
  elemental->bstat(DICE_BONUS)=UMAX(level/5,2);

  AFFECT_DATA af;
  createaff(af,-1,level,gsn_elemental_field,casting_level,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE|AFFECT_INHERENT);
  addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
  addaff(af,CHAR,damroll,ADD,level/8);
  affect_to_char( elemental, &af );

  send_to_charf(ch,"You summoned an elemental.\n\r");
  act("$n summons an elemental.", ch, NULL, NULL, TO_ROOM );
  char_to_room( elemental, ch->in_room );

  if ( number_percent() <= chance ) { // elemental becomes caster's pet
    SET_BIT(elemental->act, ACT_PET);
    SET_BIT(elemental->bstat(affected_by), AFF_CHARM);
    SET_BIT(elemental->cstat(affected_by), AFF_CHARM);
    SET_BIT(elemental->act, ACT_CREATED ); // SinaC 2003

    send_to_charf(ch,"%s join your group.\n\r", NAME(elemental) );
    act("$N joins $n's group.", ch, NULL, elemental, TO_ROOM );
    
    add_follower( elemental,ch );
    elemental->leader = ch;
    ch->pet = elemental;
  }
  else // elemental attacks caster
    MOBPROG(elemental,ch, "attackCaster", ch );
  return;
}

/*
NECROFIRE
Syntax: cast 'necrofire' <target>

School: NECROMANCY
Max Casting Level: 5

Necrofire is a spell of blackest evil, and as such can only be used by those
who follow the paths of darkness.  It conjures forth undead spirits to
inflict terrible wounds on the enemies of the caster.  At higher casting
levels, necrofire inflicts greater damage, negates saving throws, and
puts a curse upon the victim.  At its highest level, the caster is able
to attempt to have the spirits remove the victim from his or her presence.

Prerequisites:
-------------
1: None
2: Daemon Potency
3: Daemonic Carapace
4: Spectral Hand, Finger of Death
5: Spectral Hand Level 2, Finger of Death Level 2
*/
void spell_necrofire( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (saves_spell(level,victim,DAM_NEGATIVE) && casting_level < 3 ) {
    ability_damage(ch,victim,0,sn,DAM_NEGATIVE,TRUE,TRUE);
    return;
  }

  if ( casting_level == 5 // at highest casting lvl: 15% chance to kill in one hit
       && chance(15) ) {
    send_to_char("The {Dundead spirits{x bring you with them.\n\r", victim );
    act("The {Dundead spirits bring $N with them.", ch, NULL, victim, TO_CHAR );
    act("The {Dundead spirits bring $N with them.", ch, NULL, victim, TO_NOTVICT );
    sudden_death( ch, victim );
    return;
  }

  // Does some damage, more than death spell but a really lesser chance to kill in one hit
  int dam = dice( (level*casting_level)/3, number_fuzzy(11) );

  if ( saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level < 3 )
    dam /= 2;
  int done = ability_damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE );

  // Curse at higher casting level
  if ( done == DAMAGE_DONE && casting_level >= 3 && !is_affected( victim, sn ) ) {
    AFFECT_DATA af;
    int dur = 5+casting_level * casting_level;
    //newafsetup(af,CHAR,hitroll,ADD,0-level/8,dur,level,sn, casting_level);
    //affect_to_char( victim, &af );
    //af.location  = ATTR_saving_throw;
    //af.modifier  = level / 8;
    //affect_to_char( victim, &af );
    //newafsetup(af,CHAR,affected_by,OR,AFF_CURSE,dur,level,sn, casting_level);
    //affect_to_char( victim, &af );
    createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,hitroll,ADD,0-level/8);
    addaff(af,CHAR,saving_throw,ADD,level/8);
    addaff(af,CHAR,affected_by,OR,AFF_CURSE);
    affect_to_char( victim, &af );

    send_to_char( "You feel unclean.\n\r", victim );
    if ( ch != victim )
      act("$N looks very uncomfortable.",ch,NULL,victim,TO_CHAR);
  }

  return;
}

// Almost same as energy drain
void spell_spectral_hand( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;
  int amount, amounthp;

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

  if (saves_spell(level,victim,DAM_NEGATIVE) && casting_level < 3 ) {
    act("$n turns pale and shivers briefly.",victim,0,0,TO_ROOM);
    ability_damage(ch,victim,amounthp,sn,DAM_NEGATIVE,TRUE,TRUE);
    return;
  }

  int done = ability_damage(ch,victim,amount + amounthp,sn,DAM_NEGATIVE,TRUE,TRUE);
  if ( done != DAMAGE_DONE )
    return;

  switch(casting_level) {
  case 5:
    victim->mana = URANGE(0,victim->mana - amount,victim->cstat(max_mana));
    ch->mana += amount/3;
  case 4:
    ch->hit += amount/3;
  case 3: 
    act("$N's heart fibrolates and $S lungs collapse!\n\r", ch, NULL, victim, TO_CHAR );
    victim->move = URANGE(0,victim->move - amount, victim->cstat(max_move));
    ch->move += amount/3;
    send_to_char("Your heart fibrolate and your lungs collapse!\n\r",victim);
  case 0: case 1: case 2: // only damage
    break;
  }
  return;
}

void spell_rotting_touch( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_SET( victim->cstat(form), FORM_UNDEAD ) 
       || IS_SET( victim->cstat(form), FORM_INTANGIBLE ) 
       || IS_SET( victim->cstat(form), FORM_MIST ) ) {
    act("$N is unaffected by this kind of spell.", ch, NULL, victim, TO_CHAR);
    return;
  }

  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level < 3 )
    dam /= 2;

  send_to_char("You summon forth death and decay from your fingertips.\n\r",ch);
  act("$n summons forth death and decay from $s fingertips.",ch,NULL,victim,TO_ROOM);
  int done = ability_damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE );

  if ( done == DAMAGE_DONE && !is_affected( victim, sn ) ) {
    AFFECT_DATA af;
    int dur = (level*casting_level)/5;
    //newafsetup(af,CHAR,affected_by,OR,AFF_PLAGUE,level,dur,sn,casting_level);
    //affect_to_char(victim,&af);
    createaff(af,dur,level,sn,casting_level,AFFECT_ABILITY);
    addaff(af,CHAR,affected_by,OR,AFF_PLAGUE);
    if ( casting_level == 5 ) {
      //newafsetup(af,CHAR,STR,ADD,-5,dur,level,sn,casting_level);
      //affect_to_char(victim,&af);
      addaff(af,CHAR,STR,ADD,-5);
    }
    affect_to_char( victim, &af );
  }
  return;
}

void spell_death_spell( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *)vo;

  if (!IS_SET(victim->cstat(parts),PART_HEART) ) {
    send_to_char("Spell failed.\n\r", ch );
    return;
  }

  if ( saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level == 5 // at highest casting lvl: 95% chance to kill in one hit
       && chance(95) ) {
    send_to_char("Your heart explode in your chest.\n\r", victim );
    act("$N's heart explodes in $S chest.", ch, NULL, victim, TO_CHAR );
    act("$N's heart explodes in $S chest.", ch, NULL, victim, TO_NOTVICT );
    sudden_death( ch, victim );
    return;
  }

  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level < 3 )
    dam /= 2;

  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE );
  return;
}

/*
'SUMMON GHOST'
Syntax: cast 'summon ghost'

School: NECROMANCY
Max Casting Level: 0

This spell summons a ghostly apparition to the aid of the caster.  The
ghost's power is roughly equivalent to that of the caster.

Prerequisites:
-------------
Speak with Dead
*/
void spell_summon_ghost( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  send_to_char("Not yet implemented.\n\r", ch );
  return;
}

void spell_finger_of_death( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *victim = (CHAR_DATA *) vo;

  if ( IS_SET( victim->cstat(form), FORM_UNDEAD ) 
       || IS_SET( victim->cstat(form), FORM_INTANGIBLE ) 
       || IS_SET( victim->cstat(form), FORM_MIST ) ) {
    act("$N is unaffected by this kind of spell.", ch, NULL, victim, TO_CHAR);
    return;
  }
  
  act("You point your finger to $N.",ch,NULL,victim,TO_CHAR);
  act("$n points $n finger to $N.",ch,NULL,victim,TO_NOTVICT);
  act("$n points $s finger to you.",ch,NULL,victim,TO_VICT);

  if ( !saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level == 5
       && chance(95) ) { // at casting lvl 5, 95% chance to kill in one hit
    sudden_death(ch,victim);
    return;
  }

  int dam = dice( (level*casting_level)/5, number_fuzzy(10) );
  if ( saves_spell( level, victim, DAM_NEGATIVE )
       && casting_level < 3 )
    dam /= 2;

  ability_damage( ch, victim, dam, sn, DAM_NEGATIVE, TRUE, TRUE );
  return;
}

void spell_black_plague( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level) {
  CHAR_DATA *vch, *vch_next;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( is_same_group( vch, ch ) )
      continue;

    if ( ( saves_spell(level,vch,DAM_DISEASE)
	   && casting_level < 4 )
	|| (IS_NPC(vch)
	    && IS_SET(vch->act,ACT_UNDEAD) )
	|| IS_SET(vch->cstat(form),FORM_UNDEAD)) {
      act("$N seems to be unaffected.",ch,NULL,vch,TO_CHAR);
      continue;
    }

    if ( casting_level == 5 && chance(10) ) {
      act("$N has been wiped out by the black plague.", ch, NULL,vch,TO_NOTVICT);
      send_to_char("You have been wiped out by the black plague!\n\r",vch);
      act("$N has been wiped out by the black plague.",ch,NULL,vch,TO_CHAR);
      sudden_death( ch, vch );
      continue;
    }

    send_to_char("You scream in agony as black plague sores erupt from your skin.\n\r",vch);
    act("$n screams in agony as black plague sores erupt from $s skin.",
	vch,NULL,NULL,TO_ROOM);
    int dam = dice( (level*casting_level)/5, number_fuzzy(7) );
    int done = ability_damage( ch, vch, dam, sn, DAM_DISEASE, TRUE, TRUE );

    if ( done == DAMAGE_DONE /*&& !is_affected(vch,sn)*/ ) { // SinaC 2003: can be affected many times
      AFFECT_DATA af;
      //newafsetup(af,CHAR,affected_by,OR,AFF_PLAGUE,level,level*3/4,sn,casting_level);
      //affect_join(vch,&af);
      //newafsetup(af,CHAR,STR,ADD,-5,level,level*3/4,sn,casting_level);
      //affect_join(vch,&af);
      createaff(af,3*level/4,level,sn,casting_level,AFFECT_ABILITY);
      addaff(af,CHAR,affected_by,OR,AFF_PLAGUE);
      addaff(af,CHAR,STR,ADD,-5);
      affect_join(vch,&af);
    }
  }
  return;
}

/*
Syntax: cast 'animate dead' <corpse>

School: NECROMANCY
Max Spell Level: 5

The signature necromantic spell, it raises a skeleton or corpse from death
back to life, as a servant of the necromancer. The strength of the corpses
raised is based upon the casting level of the spell.  Thus, it is
possible to raise a creature more powerful than the caster if the spell
is trained to its highest levels.
*/
void spell_animate_dead( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {
  const char *name;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may cast this spell.\n\r", ch );
    return;
  }

  if ( ch->pet != NULL ){
    send_to_char("One familiar at a time is enough!\n\r",ch);
    return;
  }

  if (target_name[0] == '\0') {
    send_to_char("Animate which corpse?\n\r",ch);
    return;
  }

  OBJ_DATA *corpse = get_obj_here(ch,target_name);

  if (corpse == NULL) {
    send_to_char("You can't animate that.\n\r",ch);
    return;
  }

  if ((corpse->item_type != ITEM_CORPSE_NPC) 
      && (corpse->item_type != ITEM_CORPSE_PC) ) {
    send_to_char("You can't animate that.\n\r",ch);
    return;
  }

  // SinaC 2003: only flesh non-construct corpse can be animated with head, arms and legs
  if ( !( ( IS_SET( corpse->value[1], FORM_ANIMAL )
	    || IS_SET( corpse->value[1], FORM_CENTAUR )
	    || IS_SET( corpse->value[1], FORM_MAMMAL )
	    || IS_SET( corpse->value[1], FORM_BIRD )
	    || IS_SET( corpse->value[1], FORM_REPTILE )
	    || IS_SET( corpse->value[1], FORM_SNAKE )
	    || IS_SET( corpse->value[1], FORM_DRAGON )
	    || IS_SET( corpse->value[1], FORM_AMPHIBIAN )
	    || IS_SET( corpse->value[1], FORM_FISH )
	    || IS_SET( corpse->value[1], FORM_COLD_BLOOD ) )
	  && !IS_SET( corpse->value[1], FORM_CONSTRUCT ) ) ) {
    send_to_char("You can't animate this corpse.\n\r", ch );
    return;
  }
  if ( !( IS_SET( corpse->value[2], PART_HEAD )
	  && IS_SET( corpse->value[2], PART_LEGS ) ) ) {
    send_to_char("Something such as head or legs is missing to animate this corpse.\n\r",ch);
    return;
  }


  OBJ_DATA *obj_next;
  OBJ_DATA *obj;
  for (obj = corpse->contains; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    obj_from_obj(obj);
    obj_to_room(obj,ch->in_room);
  }
  // Added by SinaC 2001
  recomproom(ch->in_room);

  int chance = get_ability(ch,sn);

  if (level < corpse->level) {
    chance += (3*level);
    chance -= (3*corpse->level);
  }

  chance = URANGE(20-casting_level,chance,90+casting_level);

  if (number_percent() > chance) {
    act("You fail and destroy $p",ch,corpse,NULL,TO_CHAR);
    act("$n tries to animate a corpse but destroys it.",ch,NULL,NULL,TO_ROOM);
    extract_obj(corpse);
    return;
  }

  // FIXME: if corpse doesn't have eyes how the hell a red glow glow can flare in its eyes
  //  also check PART_EYE or change the message
  act("$n utters an incantation and a burning red glow flares into the eyes of $p.",ch,corpse,NULL,TO_ROOM);
  act("$p shudders and comes to life!",ch,corpse,NULL,TO_ROOM);
  act("You call upon the powers of the dark to give life to $p.",ch,corpse,NULL,TO_CHAR);
 
  CHAR_DATA *zombie = create_mobile(get_mob_index(MOB_VNUM_ZOMBIE));
  // Set form
  SET_BIT( zombie->bstat(form), corpse->value[1] ); SET_BIT( zombie->cstat(form), corpse->value[1] );
  // Copy parts
  zombie->bstat(parts) = zombie->cstat(parts) = corpse->value[2];
  // Remove unwanted form/parts
  REMOVE_BIT( zombie->bstat(parts), PART_HEART|PART_GUTS|PART_BRAINS );
  REMOVE_BIT( zombie->bstat(form), FORM_EDIBLE|FORM_MIST|FORM_INTANGIBLE|FORM_CONSTRUCT);
 
  int z_level = UMAX( 1, corpse->level - number_range(3,6) + casting_level );
  if ( casting_level == 5 )
    z_level += casting_level;
  zombie->level = z_level;
  zombie->bstat(max_hit) = UMAX((dice(z_level, 15))+(z_level*20),1);
  if ( casting_level == 5 )
    zombie->bstat(max_hit) *= 2;
  zombie->hit = zombie->bstat(max_hit);
  zombie->bstat(DICE_TYPE) = UMAX(1, z_level/3);
  if ( casting_level == 5 )
    zombie->bstat(DICE_TYPE) += z_level/5;
  if ( z_level >= 1  && z_level <= 10 )
    zombie->bstat(DICE_NUMBER) = 4;
  if ( z_level >= 11 && z_level <= 20 )
    zombie->bstat(DICE_NUMBER) = 5;
  if ( z_level >= 21 && z_level <= 30 )
    zombie->bstat(DICE_NUMBER) = 6;
  if ( z_level > 30 )
    zombie->bstat(DICE_NUMBER) = 7;
  if ( casting_level == 5 )
    zombie->bstat(DICE_NUMBER) += 4;

  // Modified by SinaC 2001 etho/alignment are attributes now 
  // Modified by SinaC 2000 for alignment/etho
  //zombie->align.alignment = -1000;
  zombie->bstat(alignment) = -1000;
  zombie->bstat(etho) = ETHO_LAWFUL;
  
  // So name starts just after 'The corpse of'
  name = corpse->short_descr+14;

  sprintf( buf1, "the zombie of %s", name);
  sprintf( buf2, "A zombie of %s is standing here.\n\r", name);

  extract_obj(corpse);
  
  zombie->short_descr = str_dup(buf1);
  zombie->long_descr = str_dup(buf2);

  SET_BIT(zombie->act, ACT_PET);
  SET_BIT(zombie->bstat(affected_by), AFF_CHARM);
  SET_BIT(zombie->act, ACT_CREATED ); // SinaC 2003

  if( casting_level >= 5 )
    zombie->bstat(classes) = 1<<class_lookup("warrior");
  
  char_to_room( zombie, ch->in_room );
  add_follower( zombie, ch );
  zombie->leader = ch;
  ch->pet = zombie;

  //char_to_room(zombie,ch->in_room);
  //recompute(zombie); NO NEED: done in char_to_room

  return;
}

//-1 'SPELL_RECKLESS DWEOMER'~
//Syntax: cast 'reckless dweomer' <spell> <spell target>
//
//The signature spell of Wild Magic users, reckless dweomer summons the raw,
//wild magic of the world through the caster.  The caster then attempts to
//craft the magic into the desired spell effect.  The actual chance of this
//spell working is low, and decreases if the caster does not know the spell
//that he or she is trying to simulate.  It decreases even further if the
//caster is trying to cast a spell that is not normally castable by mages
//of the caster's type.  If the caster does not successfully produce the
//desired spell effect, he or she will Wild Surge.
//
//It is important to remember to add the target or any other phrase normally
//placed after the spell name when casting reckless dweomer, or else even
//if the spell would have worked, it may still fail.
void spell_reckless_dweomer( int sn, int level, CHAR_DATA *ch, void *vo, int target, int casting_level ) {

  //  log_stringf("target: %s  add: %s", target_name, add_target_name );

  // target name -> arg1 + target_name(arg2) + add_target_name(arg3)
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  strcpy( buf, target_name );
  const char *argument = buf;
  
  argument = one_argument( argument, arg1 );
  strcpy(target_name,argument);
  argument = one_argument( argument, arg2 );
  one_argument(argument, arg3 );
  strcpy(add_target_name,arg3); // Added by SinaC 2003

  //  log_stringf("arg1: [%s]  arg2: [%s]  arg3: [%s]  target: [%s]  add: [%s]", arg1, arg2, arg3, target_name, add_target_name );

  // Find the spell
  if ( arg1 == NULL || arg1[0] == '\0' ) {
    send_to_charf(ch,"You must specify a spell.\n\r");
    return;
  }
  int spellSn = ability_lookup( arg1 );
  if ( spellSn < 0 ) {
    send_to_charf(ch,"That spell doesn't exist!\n\r");
    return;
  }
  if ( ability_table[spellSn].type != TYPE_SPELL ) {
    send_to_charf(ch,"That's not a spell!\n\r");
    return;
  }
  bool available = FALSE;
  for ( int i = 0; i < MAX_CLASS; i++ )
    if ( ability_table[sn].ability_level[i] < IM ) {
      available = TRUE;
      break;
    }
  if ( !available && !IS_IMMORTAL(ch) ) {
    send_to_charf(ch,"You can't use reckless dweomer with that spell.\n\r");
    return;
  }

  int pra = get_ability( ch, spellSn );
  int rate = class_abilityrating( ch, spellSn, 0 );
  int lvl = 0;
  int real_pra = 0;
  if ( !IS_NPC(ch) ) {
    lvl = ch->pcdata->ability_info[spellSn].level;
    real_pra = ch->pcdata->ability_info[spellSn].learned;
  }

  // if pra == 0 && rate == 0 -> cannot be learned
  // if pra == 0 && rate > 0 && real_pra == 0 -> not yet learned
  // if pra == 0 && rate > 0 && real_pra > 0 -> learned but higher level than current
  // if pra > 0 && rate == 0 && lvl > 0 -> learned with scribe
  // if pra > 0 && rate == 0 -> learned in really weird way (probably only immortal can have this)
  // if pra > 0 && rate > 0 -> known
  int perc = 0;
  if ( pra == 0 && rate == 0 ) perc = 5; // 5% chance
  else if ( pra == 0 && rate > 0 && real_pra == 0 ) perc = 15; // 15% chance
  else if ( pra == 0 && rate > 0 && real_pra > 0 ) perc = 25; // 25% chance
  else if ( pra > 0 && rate == 0 && lvl > 0 ) perc = 35; // 35% chance
  else if ( pra > 0 && rate == 0 ) perc = 30; // 30% chance
  else if ( pra > 0 && rate > 0 ) perc = 60; // 60% chance

  //  log_stringf("spell: %s    pra: %d    rate: %d   lvl: %d  real_pra: %d --> chance: %d",
  //	      ability_table[spellSn].name, pra, rate, lvl, real_pra, perc );

  // Failure test + wild surge
  if ( number_percent() > perc && !IS_IMMORTAL(ch) ) { // failed
    send_to_charf(ch,"You failed!\n\r");
    check_improve(ch,sn,FALSE,1);
    return;
  }

  // check position
  if ( ch->position < ability_table[spellSn].minimum_position ) {
    send_to_char( "You can't concentrate enough.\n\r", ch );
    return;
  }

  // find target
  CHAR_DATA *victim	= NULL;
  OBJ_DATA *obj	= NULL;
  vo		= NULL;
  target	= TARGET_NONE;
  switch ( ability_table[spellSn].target ) {
  default:
    bug( "Do_cast: bad target for sn %d.", spellSn );
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
    else
      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    if ( is_safe(ch,victim) && victim != ch ) {
      send_to_char("Not on that target.\n\r",ch);
      return;
    }
    if ( !IS_NPC(ch) )
      check_killer( ch,victim );
    if ( IS_AFFECTED( ch, AFF_CHARM )
	 && ch->master == victim ) {
      send_to_char( "You can't do that on your own follower.\n\r",
		    ch );
      return;
    }
    vo = (void *) victim;
    target = TARGET_CHAR;
    break;
  case TAR_CHAR_DEFENSIVE:
    if ( arg2[0] == '\0' )
      victim = ch;
    else
      if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
	send_to_char( "They aren't here.\n\r", ch );
	return;
      }
    vo = (void *) victim;
    target = TARGET_CHAR;
    break;
  case TAR_CHAR_SELF:
    if ( arg2[0] != '\0' && !is_name( arg2, ch->name ) ) {
      send_to_char( "You cannot cast this spell on another.\n\r", ch );
      return;
    }
    victim = ch;
    vo = (void *) ch;
    target = TARGET_CHAR;
    break;
  case TAR_OBJ_INV:
    if ( arg2[0] == '\0' ) {
      send_to_char( "What should the spell be cast upon?\n\r", ch );
      return;
    }
    if ( ( obj = get_obj_carry( ch, arg2, ch ) ) == NULL ) {
      send_to_char( "You are not carrying that.\n\r", ch );
      return;
    }
    vo = (void *) obj;
    target = TARGET_OBJ;
    break;
  case TAR_OBJ_CHAR_OFF:
    if ( arg2[0] == '\0' ) {
      if ( (victim = ch->fighting) == NULL ) {
	send_to_char("Cast the spell on whom or what?\n\r",ch);
	return;
      }
      target = TARGET_CHAR;
    }
    else if ( (victim = get_char_room(ch,arg2)) != NULL )
      target = TARGET_CHAR;
    if ( target == TARGET_CHAR ) {
      if ( is_safe_spell(ch,victim,FALSE) && victim != ch ) {
	send_to_char("Not on that target.\n\r",ch);
	return;
      }
      if ( IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim ) {
	send_to_char( "You can't do that on your own follower.\n\r", ch );
	return;
      }
      if ( !IS_NPC(ch) )
	check_killer(ch,victim);
      vo = (void *) victim;
    }
    else if ( (obj = get_obj_here(ch,arg2)) != NULL ) {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
    break;
  case TAR_OBJ_CHAR_DEF:
    if ( arg2[0] == '\0' ) {
      vo = (void *) ch;
      target = TARGET_CHAR;                                                 
    }
    else if ( (victim = get_char_room(ch,arg2)) != NULL ) {
      vo = (void *) victim;
      target = TARGET_CHAR;
    }
    else if ( (obj = get_obj_carry(ch,arg2,ch)) != NULL ) {
      vo = (void *) obj;
      target = TARGET_OBJ;
    }
    else {
      send_to_char("You don't see that here.\n\r",ch);
      return;
    }
    break;
  }

  // check if see target
  bool notsee;
  if ( vo != NULL )
    if ( target == TARGET_CHAR )
      notsee = !can_see( ch, (CHAR_DATA*)vo );
    else
      notsee = !can_see_obj( ch, (OBJ_DATA*)vo );
  else
    notsee = FALSE;

  bool missed = FALSE;
  if ( notsee // target missed
       && number_percent() > 50
       && !IS_IMMORTAL(ch) ) {
    send_to_char( "You missed your target.\n\r", ch );
    check_improve(ch,sn,FALSE,1);
    missed = TRUE;
  }

  if ( !missed ) {
    int casting_level = 1;
    if ( target == TARGET_CHAR
	 && ( ability_table[spellSn].target == TAR_OBJ_CHAR_OFF
	      || ability_table[spellSn].target == TAR_CHAR_OFFENSIVE )
	 && IS_AFFECTED2( (CHAR_DATA*) vo, AFF2_MAGIC_MIRROR ) 
	 && chance(50) ) {
      CHAR_DATA *victim = (CHAR_DATA*)vo;
      act( "Your spell is mirrored by $N's magic aura.", ch, NULL, victim, TO_CHAR );
      act( "$n's spell is mirrored by your magic aura.", ch, NULL, victim, TO_VICT );
      act( "$n's spell is mirrored by $N's magic aura.", ch, NULL, victim, TO_NOTVICT );
      if ( ability_table[spellSn].scriptAbility ) {
	Value args[] = { ability_table[spellSn].name, level, ch, casting_level, target_name };
	int res = mobprog( ch, NULL, "onCast", args );
	if ( !res ) {
	  send_to_char("Nothing happens.\n\r",ch);
	  return;
	}
      }
      else
	(*(SPELL_FUN*)ability_table[spellSn].action_fun) ( spellSn, level, ch, (void*)ch, target, casting_level );
    }
    else {
      if ( ability_table[spellSn].scriptAbility ) {
	Value args[] = { ability_table[spellSn].name, level, 
			 target == TARGET_CHAR ? (CHAR_DATA *)vo
			 : ( target == TARGET_OBJ ? (OBJ_DATA *)vo
			     : (ENTITY_DATA*)vo ),
			 casting_level, target_name };
	int res = mobprog( ch, NULL, "onCast", args );
	if ( !res ) {
	  send_to_char("Nothing happens.\n\r",ch);
	  return;
	}
      }
      else
	(*(SPELL_FUN*)ability_table[spellSn].action_fun) ( spellSn, level, ch, vo, target, casting_level );
      
      check_improve(ch,sn,TRUE,1);
    }
    
    Value args[] = { ch, ability_table[spellSn].name, casting_level };
    if ( target == TARGET_CHAR )
      mobprog(victim,ch,"onSpellTarget", args);
    else if ( target == TARGET_OBJ )
      objprog(obj,ch,"onSpellTarget",args);
  }

  if ( ch == NULL || !ch->valid || ch->in_room == NULL )
    return;

  if ( (ability_table[spellSn].target == TAR_CHAR_OFFENSIVE
	||   (ability_table[spellSn].target == TAR_OBJ_CHAR_OFF
	      && target == TARGET_CHAR))
       &&   victim != ch
       &&   victim->master != ch) {
    CHAR_DATA *vch;
    CHAR_DATA *vch_next;

    if ( victim != NULL )
      multi_hit( ch, victim, TYPE_UNDEFINED );

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
