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
#include "update_affect.h"
#include "olc_value.h"
#include "handler.h"
#include "fight.h"
#include "act_comm.h"
#include "gsn.h"
#include "db.h"
#include "comm.h"
#include "magic.h"
#include "update.h"
#include "condition.h"
#include "lookup.h"
#include "utils.h"
#include "moons.h"


// ****************************** UPDATE functions
void update_poison( AFFECT_DATA *af, void *vo, int target ) {
 
 if ( target != TARGET_CHAR )
    return;

  CHAR_DATA *ch = (CHAR_DATA *) vo;
  if ( !IS_AFFECTED( ch, AFF_POISON ) ) {
    bug("%s is affected by spell poison but doesn't have poison affect bit set.", NAME(ch));
    return;
  }
  // when slowed or immortal, poison affected is also slowed
  if ( IS_AFFECTED( ch, AFF_SLOW ) || IS_IMMORTAL( ch ) )
    return;

  int dam = af->level/10;
  dam += af->casting_level*5;
  if ( af->casting_level == 5 )
    dam *= 2;
  dam = UMAX( 1, dam );

  noaggr_damage( ch, dam, DAM_POISON,
		 "You shiver and suffer.",
		 "$n shivers and suffers.",
		 "is dead due to poison.",
		 FALSE );
}

// Added by SinaC 2003, plague case in char_update has been replaced with an update function
void update_plague( AFFECT_DATA *af, void *vo, int target ) {
 
 if ( target != TARGET_CHAR )
    return;

  CHAR_DATA *ch = (CHAR_DATA *) vo;
  if ( !IS_AFFECTED( ch, AFF_PLAGUE ) ) {
    bug("%s is affected by spell plague but doesn't have plague affect bit set.", NAME(ch));
    return;
  }
  // if immortal, not affected
  if ( IS_IMMORTAL( ch ) )
    return;
  
  //AFFECT_DATA plg1, plg2;
  AFFECT_DATA plg;
  int dam;
  int level = af->level;

  // Computer new affect for people in the room
  createaff(plg,number_range(1,2 * level-1), level-1, gsn_plague,af->casting_level,af->flags);
  addaff(plg,CHAR,affected_by,OR,AFF_PLAGUE);
  addaff(plg,CHAR,STR,ADD,-5);
  //afsetup( plg1, CHAR, affected_by, OR, AFF_PLAGUE,
  //	  number_range(1,2 * level-1), level-1, gsn_plague );
  //afsetup( plg2, CHAR, STR, ADD, -5, 
  //   plg1.duration, level-1, gsn_plague );
  dam = UMIN( level, level/5+1 );

  for ( CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    //if ( !saves_spell(plg1.level - 2,vch,DAM_DISEASE )
    if ( !saves_spell(level - 2,vch,DAM_DISEASE )
	&& !IS_IMMORTAL(vch)
	&& !IS_AFFECTED(vch,AFF_PLAGUE) 
	&& number_bits(4) == 0 ) {
      send_to_char("You feel hot and feverish.\n\r",vch);
      act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
      //affect_join(vch,&plg1);
      //affect_join(vch,&plg2);
      affect_join(vch,&plg);
    }
  }

  ch->mana -= dam;
  ch->move -= dam;
  ch->psp  -= dam;

  noaggr_damage( ch, dam, DAM_DISEASE,
		 "You writhe in agony from the plague.",
		 "$n writhes in agony as plague sores erupt from $s skin.",
		 "is dead from plague.",
		 FALSE );
}

void update_neverending_pain( AFFECT_DATA *af, void *vo, int target ) {
  if ( target != TARGET_CHAR )
    return;

  CHAR_DATA *ch = (CHAR_DATA *) vo;
  if ( IS_IMMORTAL( ch ) )
    return;

  int dam = af->level/4;
  dam = UMAX( 1, dam );

  noaggr_damage( ch, dam, DAM_NEGATIVE,
		 "Your mind is wrought with agonising sounds.",
		 "$n's mind is wrought with agonising sounds.",
		 "is dead because of neverending pain.",
		 FALSE );
}

void update_looking_in_the_mirror( AFFECT_DATA *af, void *vo, int target ) {
  if ( target != TARGET_CHAR )
    return;

  CHAR_DATA *ch = (CHAR_DATA *) vo;
  if (!IS_NPC(ch)) {
    bug("update_looking_in_the_mirror: player %s is affected.", NAME(ch));
    affect_strip( ch, af->type );
    return;
  }
  log_stringf("looking in the mirror  update affect: %s", NAME(ch));
  // When affected mirror image is not fighting anymore, the image fades away
  if ( ch->fighting == NULL ) {
    log_stringf("extracting mirror.");
    act("$n fades away.",ch,NULL,NULL,TO_ALL);
    extract_char(ch,TRUE);
  }
}

void update_black_plague( AFFECT_DATA *af, void *vo, int target ) {
  if ( target != TARGET_CHAR )
    return;

  CHAR_DATA *ch = (CHAR_DATA *) vo;
  if ( !IS_AFFECTED( ch, AFF_PLAGUE ) ) {
    bug("%s is affected by spell black plague but doesn't have plague affect bit set.", NAME(ch));
    return;
  }
  // if immortal, not affected
  if ( IS_IMMORTAL( ch ) )
    return;

  //AFFECT_DATA plg1, plg2;
  AFFECT_DATA plg;
  int dam;
  int level = af->level;
  int sn = gsn_black_plague;
  int casting_level = af->casting_level;

  // Computer new affect for people in the room
  //  afsetup( plg1, CHAR, affected_by, OR, AFF_PLAGUE,
  //	  number_range(1,2 * level-1)*casting_level, level-1, sn );
  //afsetup( plg2, CHAR, STR, ADD, -5-casting_level,
  //   plg1.duration, level-1, sn );
  createaff( plg, number_range(1,2 * level-1)*casting_level, level-1, sn, 0, af->flags );
  addaff( plg, CHAR, affected_by, OR, AFF_PLAGUE );
  addaff( plg, CHAR, STR, ADD, -5-casting_level );

  dam = UMIN( level, level/5+1 )*casting_level;

  for ( CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    //if ( !saves_spell(plg1.level - 2,vch,DAM_DISEASE )
    if ( !saves_spell(level - 2,vch,DAM_DISEASE )
	 && !IS_IMMORTAL(vch)
	 && !IS_AFFECTED(vch,AFF_PLAGUE) 
	 && number_bits(4) == 0 ) {
      send_to_char("You feel hot and feverish.\n\r",vch);
      act("$n shivers and looks very ill.",vch,NULL,NULL,TO_ROOM);
      //affect_join(vch,&plg1);
      //affect_join(vch,&plg2);
      affect_join(vch,&plg);
    }
  }

  ch->mana -= dam;
  ch->move -= dam;
  ch->psp  -= dam;

  noaggr_damage( ch, dam, DAM_DISEASE,
		 "You writhe in agony from the black plague.",
		 "$n writhes in agony as black plague sores erupt from $s skin.",
		 "is dead from black plague.",
		 FALSE );
}

void update_ignite_arrow( AFFECT_DATA *af, void *vo, int target ) {
  if ( target != TARGET_OBJ )
    return;

  OBJ_DATA *obj = (OBJ_DATA *) vo;
  switch ( check_immune_obj( obj, DAM_FIRE ) ) {
  case IS_IMMUNE: 
    return;
    break;
  case IS_RESISTANT:
    obj->condition -= 5;
    break;
  case IS_VULNERABLE: default:
    obj->condition -= 10;
    break;
  }

  if ( obj->condition <= 0 ) {
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
  else {
    if (obj->carried_by != NULL){
      CHAR_DATA *rch = obj->carried_by;
      act("{RThe fire consumes $p{x", rch, obj, NULL, TO_CHAR);
    }
    if (obj->in_room != NULL 
	&& obj->in_room->people != NULL){
      CHAR_DATA *rch = obj->in_room->people;
      act("{RThe fire consumes $p{x", rch, obj, NULL, TO_ALL);
    }
  }
}

// Special Affect such as Lycanthropy
// Lycanthropy has 2 states:
//   passive: only a invisible affect
//   active:  tons of affect modifying race/hit/...
void update_lycanthropy( AFFECT_DATA *af, void *vo, int target ) {
  int sn = af->type;

  if ( target != TARGET_CHAR ) {
    bug("wearoff_lycanthropy called without target = TARGET_CHAR (%d) (sn:%d[%s])", 
	target, sn,
	sn > 0 ? ability_table[sn].name:"");
    return;
  }

  CHAR_DATA *ch = (CHAR_DATA *) vo;

  if ( af->list != NULL ) { // active lycanthropy to passive
    if ( !( moon_full(0) // as long as Blue moon is visible and full: lycanthropy stays
	    && moon_visible(0) ) ) {
      log_stringf("LYCANTHROPY ACTIVE to PASSIVE [%s]", NAME(ch));
      af->list = NULL; // remove special affect
      SET_BIT(af->flags,AFFECT_INVISIBLE); // affect invisible again
      // FIXME cstat(race) can be another race than werewolf
      send_to_charf(ch,"You are not a %s anymore.\n\r", pc_race_table[ch->cstat(race)].name);
    }
  }
  else { // passive lycanthropy to active
    if ( moon_full(0) // when Blue moon is visible and full: lycanthropy becomes active
	 && moon_visible(0) ) {
      log_stringf("LYCANTHROPY PASSIVE TO ACTIVE [%s]", NAME(ch));
      int level = ch->level;
      int timer = number_fuzzy( level );
      int race = race_lookup( "werewolf" );
      if ( race < 0 ) {
	bug("LYCANTHROPY: race [lycanthropy] doesn't exist");
	return;
      }
//      int cl = class_lookup( "warrior" );
//      if ( cl <= 0 ) {
//	bug("LYCANTHROPY: class [warrior] doesn't exist");
//	return;
//      }
//      int classes = 1<<class_lookup( "warrior" );
      // Unequip and drop every worn items, NPC's stuff are only removed
      bool dropped = FALSE;
      for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
	if ( obj == NULL || !obj->valid // item not valid
	     || obj->wear_loc == -1 // item not worn
	     || obj->wear_loc == WEAR_FLOAT ) // floaties are not dropped
	  continue;
      //      for ( int iWear = 0; iWear < MAX_WEAR; iWear++ ) {
      //	OBJ_DATA *obj;
      //	if ( iWear == WEAR_FLOAT
      //	     || ( obj = get_eq_char( ch, iWear ) ) == NULL )
      //	  continue;
	if ( IS_NPC(ch) )
	  unequip_char( ch, obj );
	else {
	  obj_from_char(obj);
	  obj_to_room(obj,ch->in_room);
	  dropped = TRUE;
	}
      }

      // change race, form, parts, imm, res, vuln, aff, size
      //createaff(af,timer,level,sn,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
      addaff(*af,CHAR,race,ASSIGN,race);
      addaff(*af,CHAR,parts,ASSIGN,race_table[race].parts);
      addaff(*af,CHAR,form,ASSIGN,race_table[race].form);
      addaff(*af,CHAR,imm_flags,OR,race_table[race].imm);
      addaff(*af,CHAR,res_flags,OR,race_table[race].res);
      addaff(*af,CHAR,vuln_flags,OR,race_table[race].vuln);
      addaff(*af,CHAR,affected_by,OR,race_table[race].aff);
      //if ( race_table[race].pc_race )
      //addaff(*af,CHAR,size,ASSIGN,pc_race_table[race].size);
      addaff(*af,CHAR,size,ASSIGN,race_table[race].size);
      // classes
      //addaff(*af,CHAR,classes,ASSIGN,classes); don't modify class
      // hitroll & damroll
      addaff(*af,CHAR,hitroll,ADD,2*level);
      addaff(*af,CHAR,damroll,ADD,2*level);
      // raise max hp
      addaff(*af,CHAR,max_hit,ADD,10*level);
      // Add STR, DEX, CON
      addaff(*af,CHAR,STR,ADD,10);
      addaff(*af,CHAR,CON,ADD,5);
      addaff(*af,CHAR,DEX,ADD,5);
      // Set INT and WIS to 6
      addaff(*af,CHAR,INT,ASSIGN,6);
      addaff(*af,CHAR,WIS,ASSIGN,6);
      // Can't wear any equipment
      addaff(*af,CHAR,affected2_by,OR,AFF2_NOEQUIPMENT);
      // Better AC
      addaff(*af,CHAR,allAC,ADD,-3*level);

      // Lycanthropy affect is now visible
      REMOVE_BIT(af->flags,AFFECT_INVISIBLE);
      
      // FIXME: ambient text
      send_to_charf( ch, "You transform yourself into %s.\n\r", pc_race_table[race].name );
      act("$n transforms $mself into a $t.", ch, pc_race_table[race].name, NULL, TO_ROOM );
      if (dropped) {
	send_to_char("Your equipement lies on the ground.\n\r", ch);
	recomproom( ch->in_room );
      }
    }
  }
  recompute(ch); // recompute needed

  return;
}
