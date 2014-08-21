/*         Modified by SinaC 2001
 *
 *  The unique portions of SunderMud code as well as the integration efforts
 *  for code from other sources is based on the efforts of:
 *
 *  Lotherius (elfren@aros.net)
 *
 *  This code can only be used under the terms of the DikuMud, Merc,
 *  and ROM licenses. The same requirements apply to the changes that
 *  have been made.
 *
 * All other copyrights remain in place and in force.
*/


/* Zeran - obj_cond.c written to support object conditions and all related
functions */

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
 **************************************************************************/

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
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "classes.h"
#include "condition.h"

// Added by SinaC 2001
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "olc_value.h"
#include "utils.h"


//#define VERBOSE


/* for immunity, vulnerabiltiy, and resistant
   the 'globals' (magic and weapons) may be overriden
   three other cases -- wood, silver, and iron -- are NOT checked */
int check_immune_obj(OBJ_DATA *obj, int dam_type) {
  int immune, def;
  int bit, mat;
  int imm, res, vuln;

  mat = obj->material;
  imm  = material_table[mat].imm;
  res  = material_table[mat].res;
  vuln = material_table[mat].vuln;

  immune = -1;
  def = IS_NORMAL;

  if (dam_type == DAM_NONE)
    return immune;

  if (dam_type <= 3) {
    if (IS_SET(imm,IRV_WEAPON))
      def = IS_IMMUNE;
    else if (IS_SET(res,IRV_WEAPON))
      def = IS_RESISTANT;
    else if (IS_SET(vuln,IRV_WEAPON))
      def = IS_VULNERABLE;
  }
  else {/* magical attack */
    if (IS_SET(imm,IRV_MAGIC))
      def = IS_IMMUNE;
    else if (IS_SET(res,IRV_MAGIC))
      def = IS_RESISTANT;
    else if (IS_SET(vuln,IRV_MAGIC))
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

  if (IS_SET(imm,bit))
    immune = IS_IMMUNE;
  else if (IS_SET(res,bit) && immune != IS_IMMUNE)
    immune = IS_RESISTANT;
  else if (IS_SET(vuln,bit)) {
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

const char *show_obj_cond (OBJ_DATA *obj) {
  int condition=0;
  
  if (IS_SET(obj->extra_flags, ITEM_NOCOND)) /*no show condition */
    return "(none)";

  /*
  if (obj->condition > 90)
    condition = 0;
  else if (obj->condition > 75)
    condition = 1;
  else if (obj->condition > 50)
    condition = 2;
  else if (obj->condition > 25)
    condition = 3;
  else if (obj->condition > 10)
    condition = 4;
  else if (obj->condition >  0)
    condition = 5;
  else if (obj->condition == 0)
    condition = 6;
  */
  if (obj->condition == 100)
    condition = 0;
  else if (obj->condition > 80)
    condition = 1;
  else if (obj->condition > 60)
    condition = 2;
  else if (obj->condition > 40)
    condition = 3;
  else if (obj->condition > 20)
    condition = 4;
  else if (obj->condition >  0)
    condition = 5;
  else if (obj->condition == 0)
    condition = 6;

  
  return cond_table[condition];
}	

void check_damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int chance, int dam_type ) { 
  /* Assumption - NULL obj means check all equipment */
  bool checkall=FALSE;
  bool done=FALSE;
  int damage_pos;
  OBJ_DATA *dobj=NULL;
  int stop=0;

  if ( IS_IMMORTAL(ch) )
    return;
  
  if (obj == NULL || !obj->valid )
    checkall=TRUE;
	
  if (checkall) {/*damage random equipped item*/
    if (number_percent () <= chance) {/*something dinged up*/
      while ((!done) && (stop <= 20)) {/* stop prevents infinite loop */
	damage_pos = number_range (1, MAX_WEAR);
	if ((dobj = get_eq_char (ch, damage_pos)) != NULL) 
	  done=TRUE;
	stop++;
      }
      if (done)
	damage_obj (ch, dobj, 1, dam_type );
    }
    else return;
  }
  else if (number_percent () <= chance) {/*damage passed in object*/
    damage_obj(ch, obj, 1, dam_type );
    return;
  }
  return;
}		

void damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int damage, int dam_type ) {
  if (obj == NULL || !obj->valid ) {
    bug ("NULL obj passed to damage_obj");
    return;
  }

  int a;
  switch(check_immune_obj(obj,dam_type)) {
  case(IS_IMMUNE):
    a = 1;
    damage = 0;
    break;
  case(IS_RESISTANT):
    a = 2;
    if ( number_percent() < 50 )
      damage -= damage/3;
    break;
  case(IS_VULNERABLE):
    a = 3;
    damage *= 2;
    break;
  default:
    a = 4;
    break;
  }

  if ( IS_SET( obj->extra_flags, ITEM_NOCOND ) ) {
#ifdef VERBOSE
    log_stringf("OBJ DAM: SAFE %s: %s (%3d) dam: %d", 
		ch?NAME(ch):"(null)",obj->short_descr, obj->condition, damage );
#endif
    return;
  }

#ifdef VERBOSE
  log_stringf("OBJ DAM: %s: %s (%3d - %2d)  dam_type: %d", 
	      ch?NAME(ch):"(null)",obj->short_descr, obj->condition, damage, a );
#endif
 
  obj->condition -= damage;
  obj->condition = URANGE (0, obj->condition, 100);

  // Added by SinaC 2001
  if ( damage != 0 )
    OBJPROG( obj, NULL, "onDamage", damage );

  /*Check for item falling apart*/
  if (obj->condition == 0 ) {
    log_stringf("OBJ DAM ==> BROKEN");
    if ( ch != NULL ) {
      char mesbuf[256];
      sprintf( mesbuf,
	       "{C%s{C has becomed too badly damaged to use!{x\n\r",
	       ( (obj->short_descr 
		  && obj->short_descr[0] != '\0' 
		  && can_see_obj( ch, obj ) ) ? obj->short_descr : "Something") );
      send_to_char(mesbuf, ch);
      unequip_char(ch, obj);
      return;
    }
  }

  recompobj( obj );
  //if ( ch != NULL && obj->wear_loc != -1 )
  //  recompute( ch ); NO NEED: done in unequip_char and recomputer was called only if unequip_char was called
  return;
}			
