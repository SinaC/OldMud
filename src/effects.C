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
#include "comm.h"
#include "handler.h"
#include "update.h"
#include "gsn.h"
#include "olc_value.h"
#include "condition.h"
#include "magic.h"
#include "utils.h"


void acid_effect(void *vo, int level, int dam, int target) {
  if (target == TARGET_ROOM) { /* nail objects on the floor */
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
    OBJ_DATA *obj, *obj_next;

    for (obj = room->contents; obj != NULL; obj = obj_next) {
	  obj_next = obj->next_content;
	  acid_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recomproom(room);
    return;
  }
  
  if (target == TARGET_CHAR) {  /* do the effect on a victim */
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    /* let's toast some gear */
    for (obj = victim->carrying; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      acid_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recompute(victim);
    return;
  }
  
  if (target == TARGET_OBJ) {/* toast an object */
    
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *t_obj,*n_obj;
    int chance;
    char *msg;
    
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	// Added by SinaC 2001
	|| IS_SET( obj->extra_flags, ITEM_NOCOND )
	||  number_range(0,4) == 0)
      return;

    chance = level / 4 + dam / 10;
    
    if (chance > 25)
      chance = (chance - 25) / 2 + 25;
    if (chance > 50)
      chance = (chance - 50) / 2 + 50;
    
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      chance -= 5;
    
    chance -= obj->level * 2;

    // Added by SinaC 2001
    switch(check_immune_obj(obj,DAM_ACID)) {
    case IS_IMMUNE:
      return;
    case IS_RESISTANT:
      chance/=2; break;
    case IS_VULNERABLE:
      chance*=2; break;
    default:
      break;
    }
    
    switch (obj->item_type) {
    default:
      return;
    case ITEM_CONTAINER:
    case ITEM_CORPSE_PC:
    case ITEM_CORPSE_NPC:
      msg = "$p fumes and dissolves.";
      break;
    case ITEM_ARMOR:
      msg = "$p is pitted and etched.";
      break;
    case ITEM_CLOTHING:
      msg = "$p is corroded into scrap.";
      break;
    case ITEM_STAFF:
    case ITEM_WAND:
      chance -= 10;
      msg = "$p corrodes and breaks.";
      break;
    case ITEM_SCROLL:
    // Added by SinaC 2003
    case ITEM_TEMPLATE:
      chance += 10;
      msg = "$p is burned into waste.";
      break; 
    }
    
    chance = URANGE(5,chance,95);
    
    if (number_percent() > chance)
      return;
    
    if (obj->carried_by != NULL)
      act(msg,obj->carried_by,obj,NULL,TO_ALL);
    else if (obj->in_room != NULL && obj->in_room->people != NULL)
      act(msg,obj->in_room->people,obj,NULL,TO_ALL);
    
    if (obj->item_type == ITEM_ARMOR) {/* etch it */
      //AFFECT_DATA *paf;
      //bool af_found = FALSE;
      
      affect_enchant(obj);

      AFFECT_DATA af;
      createaff(af,-1,level,gsn_acid_breath,0,AFFECT_ABILITY);
      addaff(af,CHAR,allAC,ADD,-1);
      affect_join_obj( obj, &af );

      // SinaC 2003: new affect system
      //for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
      //if ( paf->type == gsn_acid_breath ) {
      //  af_found = TRUE;
      //  paf->modifier += 1;
      //  paf->level = UMAX(paf->level,level);
      //  break;
      //}
      //}

      /* Modified by SinaC 2001
      for ( paf = obj->affected; paf != NULL; paf = paf->next) {
	if ( paf->where == AFTO_CHAR
	     && paf->location == ATTR_allAC
	     && paf->op == AFOP_ADD ) {
	  af_found = TRUE;
	  // Modified by SinaC 2000
	  //paf->type = -1;
	  paf->type = gsn_acid_breath;
	  paf->modifier += 1;
	  paf->level = UMAX(paf->level,level);
	  break;
	}
      }
      */

      //if (!af_found) { // needs a new affect
      //	paf = new_affect();
      //// Modified by SinaC 2000
      ////afsetup(*paf,CHAR,allAC,ADD,1,-1,level,-1);
      //afsetup(*paf,CHAR,allAC,ADD,1,-1,level,gsn_acid_breath);
      //paf->next       = obj->affected;
      //obj->affected   = paf;
      //}
      
      // Added by SinaC 2001
      damage_obj( obj->carried_by, obj, 5, DAM_ACID );

      /* done in damage_obj
      recompobj(obj);
      if ( obj->carried_by != NULL && obj->wear_loc != -1 )
	recompute(obj->carried_by);
      */
      return;
    }
    
    // Added by SinaC 2001
    //bool drop = FALSE;

    /* get rid of the object */
    if (obj->contains) { /* dump contents */
      for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
	n_obj = t_obj->next_content;
	obj_from_obj(t_obj);
	if (obj->in_room != NULL) {
	  obj_to_room(t_obj,obj->in_room);
	  // Added by SinaC 2001
	  //drop = TRUE;
	}
	else if (obj->carried_by != NULL) {
	  obj_to_room(t_obj,obj->carried_by->in_room);
	  // Added by SinaC 2001
	  //drop = TRUE;
	}
	// Modified by SinaC 2001
	else if ( !IS_SET(t_obj->extra_flags, ITEM_NOCOND) ) {
	  extract_obj(t_obj);
	  continue;
	}
	
	acid_effect(t_obj,level/2,dam/2,TARGET_OBJ);
      }
    }

    /*
    CHAR_DATA *carrier = obj->carried_by;
    int wloc = obj->wear_loc;
    */
    extract_obj(obj);
    /*
    if ( carrier != NULL && wloc != -1 )
      recompute(carrier);
    // Added by SinaC 2001
    if ( carrier != NULL && drop )
      recomproom(carrier->in_room);
    */
    return;
  }
}

void cold_effect(void *vo, int level, int dam, int target){
  if (target == TARGET_ROOM) {/* nail objects on the floor */
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    for (obj = room->contents; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      cold_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recomproom(room);
    return;
  }
  
  if (target == TARGET_CHAR) {/* whack a character */
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    /* chill touch effect */
    if (!saves_spell(level/4 + dam / 20, victim, DAM_COLD)) {
      AFFECT_DATA af;
      
      act("$n turns blue and shivers.",victim,NULL,NULL,TO_ROOM);
      act("A chill sinks deep into your bones.",victim,NULL,NULL,TO_CHAR);

      createaff(af,6,level,gsn_chill_touch,0,AFFECT_ABILITY);
      addaff(af,CHAR,STR,ADD,-1);
      affect_join( victim, &af );
      //afsetup(af,CHAR,STR,ADD,-1,6,level, gsn_chill_touch );
      //affect_join( victim, &af );
    }
    
    /* hunger! (warmth sucked out */
    if (!IS_NPC(victim))
      gain_condition(victim,COND_HUNGER,dam/20);
    
    /* let's toast some gear */
    for (obj = victim->carrying; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      cold_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recompute(victim);
    return;
  }
  
  if (target == TARGET_OBJ) {/* toast an object */
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance;
    char *msg;
    
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	// Added by SinaC 2001
	|| IS_SET( obj->extra_flags, ITEM_NOCOND )
	||  number_range(0,4) == 0)
      return;
    
    chance = level / 4 + dam / 10;
    
    if (chance > 25)
      chance = (chance - 25) / 2 + 25;
    if (chance > 50)
      chance = (chance - 50) / 2 + 50;
    
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      chance -= 5;
    
    chance -= obj->level * 2;

    // Added by SinaC 2001
    switch(check_immune_obj(obj,DAM_COLD)) {
    case IS_IMMUNE:
      return;
    case IS_RESISTANT:
      chance/=2; break;
    case IS_VULNERABLE:
      chance*=2; break;
    default:
      break;
    }
    
    switch(obj->item_type) {
    default:
      return;
    case ITEM_POTION:
      msg = "$p freezes and shatters!";
      chance += 25;
      break;
    case ITEM_DRINK_CON:
      msg = "$p freezes and shatters!";
      chance += 5;
      break;
    }
    
    chance = URANGE(5,chance,95);
    
    if (number_percent() > chance)
      return;
    
    if (obj->carried_by != NULL)
      act(msg,obj->carried_by,obj,NULL,TO_ALL);
    else if (obj->in_room != NULL && obj->in_room->people != NULL)
      act(msg,obj->in_room->people,obj,NULL,TO_ALL);

    /*
    CHAR_DATA *carrier = obj->carried_by;
    int wloc = obj->wear_loc;
    */
    extract_obj(obj);
    /*
    if ( carrier != NULL && wloc != -1 )
      recompute(carrier);
    // Added by SinaC 2001
    if ( carrier != NULL && drop )
      recomproom(carrier->in_room);
    */
    return;
  }
}

void fire_effect(void *vo, int level, int dam, int target){
  if (target == TARGET_ROOM) { /* nail objects on the floor */
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    for (obj = room->contents; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      fire_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recomproom(room);
    return;
  }
  
  if (target == TARGET_CHAR) {  /* do the effect on a victim */
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    /* chance of blindness */
    if (!IS_AFFECTED(victim,AFF_BLIND)
	&&  !saves_spell(level / 4 + dam / 20, victim,DAM_FIRE)) {
      AFFECT_DATA af;
      act("$n is blinded by smoke!",victim,NULL,NULL,TO_ROOM);
      act("Your eyes tear up from smoke...you can't see a thing!",
	  victim,NULL,NULL,TO_CHAR);
      
      // Modified by SinaC 2001
      int duration = number_range(0,level/10);

      createaff(af,duration,level,gsn_fire_breath,0,AFFECT_ABILITY);
      addaff(af,CHAR,affected_by,OR,AFF_BLIND);
      addaff(af,CHAR,hitroll,ADD,-4);
      affect_to_char( victim, &af );
      //afsetup(af,CHAR,affected_by,OR,AFF_BLIND,
      //      duration,level,gsn_fire_breath);
      //affect_to_char(victim,&af);
      //afsetup(af,CHAR,hitroll,ADD,-4,
      //      duration, level, gsn_fire_breath);
      //affect_to_char(victim,&af);
    }
    
    /* getting thirsty */
    if (!IS_NPC(victim))
      gain_condition(victim,COND_THIRST,dam/20);
    
    /* let's toast some gear! */
    for (obj = victim->carrying; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      
      fire_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recompute(victim);
    return;
  }
  
  if (target == TARGET_OBJ) { /* toast an object */
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    OBJ_DATA *t_obj,*n_obj;
    int chance;
    char *msg;
    
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	// Added by SinaC 2001
	|| IS_SET( obj->extra_flags, ITEM_NOCOND )
	||  number_range(0,4) == 0)
      return;
    
    chance = level / 4 + dam / 10;
    
    if (chance > 25)
      chance = (chance - 25) / 2 + 25;
    if (chance > 50)
      chance = (chance - 50) / 2 + 50;
    
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      chance -= 5;
    chance -= obj->level * 2;

    // Added by SinaC 2001
    switch(check_immune_obj(obj,DAM_FIRE)) {
    case IS_IMMUNE:
      return;
    case IS_RESISTANT:
      chance/=2; break;
    case IS_VULNERABLE:
      chance*=2; break;
    default:
      break;
    }
    
    switch ( obj->item_type ) {
    default:             
      return;
    case ITEM_CONTAINER:
      msg = "$p ignites and burns!";
      break;
    case ITEM_POTION:
      chance += 25;
      msg = "$p bubbles and boils!";
      break;
    case ITEM_SCROLL:
    // Added by SinaC 2003
    case ITEM_TEMPLATE:
      chance += 50;
      msg = "$p crackles and burns!";
      break;
    case ITEM_STAFF:
      chance += 10;
      msg = "$p smokes and chars!";
      break;
    case ITEM_WAND:
      msg = "$p sparks and sputters!";
      break;
    case ITEM_FOOD:
      msg = "$p blackens and crisps!";
      break;
    case ITEM_PILL:
      msg = "$p melts and drips!";
      break;
    }
    
    chance = URANGE(5,chance,95);
    
    if (number_percent() > chance)
      return;
    
    if (obj->carried_by != NULL)
      act( msg, obj->carried_by, obj, NULL, TO_ALL );
    else if (obj->in_room != NULL && obj->in_room->people != NULL)
      act(msg,obj->in_room->people,obj,NULL,TO_ALL);
    
    if (obj->contains) {
      /* dump the contents */
      
      for (t_obj = obj->contains; t_obj != NULL; t_obj = n_obj) {
	n_obj = t_obj->next_content;
	obj_from_obj(t_obj);
	if (obj->in_room != NULL)
	  obj_to_room(t_obj,obj->in_room);
	else if (obj->carried_by != NULL)
	  obj_to_room(t_obj,obj->carried_by->in_room);
	else if ( !IS_SET(t_obj->extra_flags, ITEM_NOCOND) ) {
	  extract_obj(t_obj);
	  continue;
	}
	fire_effect(t_obj,level/2,dam/2,TARGET_OBJ);
      }
    }
    
    //CHAR_DATA *carrier = obj->carried_by;
    extract_obj(obj);
    /*
    if ( carrier != NULL && obj->wear_loc != -1 )
      recompute(carrier);
    */
    return;
  }
}

void poison_effect(void *vo,int level, int dam, int target) {
  if (target == TARGET_ROOM) { /* nail objects on the floor */
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    for (obj = room->contents; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      poison_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recomproom(room);
    return;
  }
  
  if (target == TARGET_CHAR) {  /* do the effect on a victim */
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    /* chance of poisoning */
    if (!saves_spell(level / 4 + dam / 20,victim,DAM_POISON)) {
      AFFECT_DATA af;
      
      send_to_char("You feel poison coursing through your veins.\n\r",
		   victim);
      act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
      
      createaff(af,level/2,level,gsn_poison,0,AFFECT_ABILITY);
      addaff(af,CHAR,affected_by,OR,AFF_POISON);
      addaff(af,CHAR,STR,ADD,-1);
      affect_join( victim, &af );
      //      afsetup(af,CHAR,affected_by,OR,AFF_POISON,level/2,level,gsn_poison);
      //      affect_join( victim, &af );
      //      afsetup(af,CHAR,STR,ADD,-1,level/2,level,gsn_poison);
      //      affect_join( victim, &af );
    }
    
    /* equipment */
    for (obj = victim->carrying; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      poison_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recompute(victim);
    return;
  }
  
  if (target == TARGET_OBJ) { /* do some poisoning */
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance;
    
    
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_BLESS)
	// Added by SinaC 2001
	|| IS_SET( obj->extra_flags, ITEM_NOCOND )
	||  number_range(0,4) == 0)
      return;
    
    chance = level / 4 + dam / 10;
    if (chance > 25)
      chance = (chance - 25) / 2 + 25;
    if (chance > 50)
      chance = (chance - 50) / 2 + 50;
    
    chance -= obj->level * 2;

    // Added by SinaC 2001
    switch(check_immune_obj(obj,DAM_POISON)) {
    case IS_IMMUNE:
      return;
    case IS_RESISTANT:
      chance/=2; break;
    case IS_VULNERABLE:
      chance*=2; break;
    default:
      break;
    }
    
    switch (obj->item_type) {
    default:
      return;
    case ITEM_FOOD:
      break;
    case ITEM_DRINK_CON:
      if (obj->value[0] == obj->value[1])
	return;
      break;
    }
    
    chance = URANGE(5,chance,95);
    
    if (number_percent() > chance)
      return;
    
    obj->baseval[3] = 1;
    recompobj(obj);
    /*
    if ( obj->carried_by != NULL && obj->wear_loc != -1 )
      recompute(obj->carried_by);
    */
    return;
  }
}


void shock_effect(void *vo,int level, int dam, int target){
  if (target == TARGET_ROOM) {
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    for (obj = room->contents; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      shock_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recomproom(room);
    return;
  }
  
  if (target == TARGET_CHAR) {
    CHAR_DATA *victim = (CHAR_DATA *) vo;
    OBJ_DATA *obj, *obj_next;
    
    /* daze and confused? */
    if (!saves_spell(level/4 + dam/20,victim,DAM_LIGHTNING)) {
      send_to_char("Your muscles stop responding.\n\r",victim);
      DAZE_STATE(victim,UMAX(12,level/4 + dam/20));
      // Added by SinaC 2000, removed by SinaC 2001
      //victim->stunned = 1;
      // Added by SinaC 2001
      WAIT_STATE(victim,PULSE_VIOLENCE);
    }

    /* toast some gear */
    for (obj = victim->carrying; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      shock_effect(obj,level,dam,TARGET_OBJ);
    }

    // Added by SinaC 2001
    recompute(victim);
    return;
  }
  
  if (target == TARGET_OBJ) {
    OBJ_DATA *obj = (OBJ_DATA *) vo;
    int chance;
    char *msg;
    
    if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF)
	||  IS_OBJ_STAT(obj,ITEM_NOPURGE)
	// Added by SinaC 2001
	|| IS_SET( obj->extra_flags, ITEM_NOCOND )
	||  number_range(0,4) == 0)
      return;
    
    chance = level / 4 + dam / 10;
    
    if (chance > 25)
      chance = (chance - 25) / 2 + 25;
    if (chance > 50)
      chance = (chance - 50) /2 + 50;
    
    if (IS_OBJ_STAT(obj,ITEM_BLESS))
      chance -= 5;
    
    chance -= obj->level * 2;

    // Added by SinaC 2001
    switch(check_immune_obj(obj,DAM_LIGHTNING)) {
    case IS_IMMUNE:
      return;
    case IS_RESISTANT:
      chance/=2; break;
    case IS_VULNERABLE:
      chance*=2; break;
    default:
      break;
    }
    
    switch(obj->item_type) {
    default:
      return;
    case ITEM_WAND:
    case ITEM_STAFF:
      chance += 10;
      msg = "$p overloads and explodes!";
      break;
    case ITEM_JEWELRY:
      chance -= 10;
      msg = "$p is fused into a worthless lump.";
    }
    
    chance = URANGE(5,chance,95);
    
    if (number_percent() > chance)
      return;
    
    if (obj->carried_by != NULL)
      act(msg,obj->carried_by,obj,NULL,TO_ALL);
    else if (obj->in_room != NULL && obj->in_room->people != NULL)
      act(msg,obj->in_room->people,obj,NULL,TO_ALL);
    
    //CHAR_DATA *carrier = obj->carried_by;
    extract_obj(obj);
    /*
    if ( carrier != NULL && obj->wear_loc != -1 )
      recompute(carrier);
    */
    return;
  }
}
