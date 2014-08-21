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
#include "wearoff_affect.h"
#include "db.h"
#include "handler.h"
#include "comm.h"
#include "olc_value.h"
#include "fight.h"
#include "condition.h"
#include "utils.h"


// ****************************** WEAR OFF functions

// Automatically called when morph affect wears off
void wearoff_morph( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_CHAR ) {
    bug("wearoff_morph called without target = TARGET_CHAR (%d) (sn:%d[%s])", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }
  CHAR_DATA *ch = (CHAR_DATA *) vo;

  // We remove only PERMANENT affect: like invisible, speedup, ...
  AFFECT_DATA *paf, *paf_next;
  for ( paf = ch->affected; paf; paf = paf_next ) {
    paf_next = paf->next;
    //if ( paf->duration == DURATION_PERMANENT
    if ( IS_SET( paf->flags, AFFECT_PERMANENT )
	 && IS_SET( paf->flags, AFFECT_ABILITY )
	 && paf != af ) // but don't remove morph affect -> needed by char_update
      affect_remove( ch, paf );
  }

  send_to_char( ability_table[sn].msg_off, ch );
  send_to_char( "\n\r", ch );

  // set language to common
  //int lang = language_lookup( "common" );
  //ch->pcdata->language = lang;
}

void wearoff_flamestrike( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;
  if ( target != TARGET_OBJ ) {
    bug("wearoff_flamestrike called without target = TARGET_OBJ (%d) (sn:%d[%s])",
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }
  OBJ_DATA *obj = (OBJ_DATA *) vo;

  if ( af->casting_level <= 4  // if casting level <= 4, 33% chance to consume weapon
       && chance(33) ) {
    if (obj->carried_by != NULL){
      CHAR_DATA *rch = obj->carried_by;
      act("{RThe fire consumes $p{x", rch,obj,NULL,TO_CHAR);
    }
    if (obj->in_room != NULL 
	&& obj->in_room->people != NULL){
      CHAR_DATA *rch = obj->in_room->people;
      act("{RThe fire consumes $p{x", rch,obj,NULL,TO_ALL);
    }
    extract_obj( obj );
  }
  else {
    if ( obj->carried_by != NULL ) { // carried item -> message to carrier
      CHAR_DATA *rch = obj->carried_by;
      act(ability_table[sn].msg_obj, rch,obj,NULL,TO_CHAR);
    }
    if (obj->in_room != NULL  // item on the ground -> message to everyone in the room
	&& obj->in_room->people != NULL){
      CHAR_DATA *rch = obj->in_room->people;
      act(ability_table[sn].msg_obj, rch,obj,NULL,TO_ALL);
    }
  }
}

void wearoff_temporal_stasis( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_CHAR ) {
    bug("wearoff_temporal_stasis called without target = TARGET_CHAR (%d) (sn:%d[%s]))", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }
  CHAR_DATA *ch = (CHAR_DATA *) vo;

  AFFECT_LIST *laf = af->list;
  if ( laf == NULL ) {
    bug("Problem in wearoff_temporal_stasis: af->list is NULL  [%s]", NAME(ch));
    return;
  }

  // REALLY CRAPPY
  //  OR   means  caster
  //  ADD  means  victim
  if ( laf->op == AFOP_OR ) { // caster
    send_to_char("The timelines are realigned.\n\r",ch);
    return;
  }
  else if ( laf->op == AFOP_ADD ) { // victim
    if ( ch->position == POS_PARALYZED ) {
      ch->position = POS_STANDING;
      send_to_char("You can move again.\n\r", ch );
      update_pos( ch );
      return;
    } 
    else {
      log_stringf("wearoff_temporal_stasis: position is %d for %s (%d)", 
		  ch->position,
		  NAME(ch),
		  IS_NPC(ch)?ch->pIndexData->vnum:0);
      return;
    }
  }
  else {
    bug("wearoff_temporal_stasis: invalid op (%d) should be OR or AND", 
	laf->op );
    return;
  }
}

void wearoff_flesh_to_stone( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_CHAR ) {
    bug("wearoff_flesh_to_stone called without target = TARGET_CHAR (%d) (sn:%d[%s]))", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }

  // affect is correct, so give hp back
  CHAR_DATA *ch = (CHAR_DATA *) vo;
  AFFECT_LIST *laf = af->list;
  if ( laf == NULL ) {
    bug("Problem in wearoff_flesh_to_stone: af->list is NULL  [%s]", NAME(ch) );
    return;
  }
  ch->hit = laf->modifier;
  
  if ( ch->position == POS_PARALYZED ) {
    ch->position = POS_STANDING;
    send_to_char( ability_table[sn].msg_off, ch );
    send_to_char( "\n\r", ch );
    return;
  } 
  else {
    log_stringf("wearoff_flesh_to_stone: position is %d for %s (%d)", 
		ch->position,
		NAME(ch),
		IS_NPC(ch)?ch->pIndexData->vnum:0);
    return;
  }
}

// Automatically called when 'polymorph self' affect wears off
void wearoff_polymorph_self( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_CHAR ) {
    bug("wearoff_polymorph_self called without target = TARGET_CHAR (%d) (sn:%d[%s]))", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }
  CHAR_DATA *ch = (CHAR_DATA *) vo;
  AFFECT_LIST *laf = af->list;
  if ( laf == NULL ) {
    bug("Problem in wearoff_flesh_to_stone: af->list is NULL  [%s]", NAME(ch) );
    return;
  }

  if ( chance(1)) { // 1% chance to be stuck in the new form
    int race = laf->modifier;
    send_to_char("You are unable to return to your natural state.\n\r", ch );
    ch->bstat(race) = race;
    return;
  }

  send_to_char( ability_table[sn].msg_off, ch );
  send_to_char( "\n\r", ch );

  // we resize equipement to fit size of char's base race
  int race = ch->bstat(race);
  int newSize = race_table[race].size;
  if (race >= 0 && race_table[race].pc_race) { // if race is correct, we resize items
    for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
      if ( obj->wear_loc != WEAR_NONE 
	   && obj->size != SIZE_NOSIZE )
	//obj->size = pc_race_table[race].size;
	obj->size = newSize;
  }

  // We remove PERMANENT affects: invisible, speedup, ...
  AFFECT_DATA *paf, *paf_next;
  for ( paf = ch->affected; paf; paf = paf_next ) {
    paf_next = paf->next;
    //if ( paf->duration == DURATION_PERMANENT
    if ( IS_SET( paf->flags, AFFECT_PERMANENT )
	 && IS_SET( paf->flags, AFFECT_ABILITY )
	 && paf != af )
      affect_remove_no_recompute( ch, paf ); // aff_rem_no_rec used to avoid problems with items (resized)
  }
  // no recompute needed: a recompute will be automatically done in update_char
  //  when removing the affect
}

void wearoff_ignite_arrow( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_OBJ ) {
    bug("wearoff_ignite_arrow alled without target = TARGET_OBJ (%d) (sn:%d[%s])", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }

  OBJ_DATA *obj = (OBJ_DATA *) vo;
  switch ( check_immune_obj( obj, DAM_FIRE ) ) {
  case IS_IMMUNE: break;
    return;
  case IS_RESISTANT:
    if ( chance(50) )
      return;
    break;
  case IS_VULNERABLE: default:
    break;
  }
  if (obj->carried_by != NULL){
    CHAR_DATA *rch = obj->carried_by;
    act("{RThe fire consumes and destroys $p{x", rch, obj, NULL, TO_CHAR);
  }
  if (obj->in_room != NULL 
      && obj->in_room->people != NULL){
    CHAR_DATA *rch = obj->in_room->people;
    act("{RThe fire consumes and destroys $p{x", rch, obj, NULL, TO_ALL);
  }
  extract_obj( obj );
}

// Special Affect such as Lycanthropy
void wearoff_lycanthropy( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_CHAR ) {
    bug("wearoff_lycanthropy called without target = TARGET_CHAR (%d) (sn:%d[%s])", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }

  return;
}
