// This file has been written by SinaC 2003 to read Circle area file

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#endif

#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"
#include <stdarg.h>

#include "comm.h"
#include "handler.h"
#include "olc_value.h"
#include "affects.h"
#include "scrhash.h"
#include "dbscript.h"
#include "mem.h"


long flag_convert_circle(char letter ) {
  long bitsum = 0;
  char i;

  if ('a' <= letter && letter <= 'z') {
    bitsum = 1;
    for (i = letter; i > 'a'; i--)
      bitsum *= 2;
  }
  // Added by SinaC 2001 to avoid negative flag
  if ( bitsum < 0 ) {
    bug("Negative bit!!!!");
    return 0;
  }
  return bitsum;
}

long fread_flag_circle( FILE *fp) {
  long number;
  char c;
  bool negative = FALSE;

  do {
    c = getc(fp);
  } while ( isspace(c));
    
  /* By Oxtal for MZF compatibility */
  if (c == '\'') {
    c = getc(fp);
    if (c != '\'')
      bug("Wrong flag format. (Double quote expected)");
    return 0;
  } 

  if (c == '-') {
    negative = TRUE;
    c = getc(fp);
  }

  number = 0;       

  if (!isdigit(c)) {
    while ('a' <= c && c <= 'z') {
      number += flag_convert_circle(c);
      c = getc(fp);
    }
  }

  while (isdigit(c)) {
    number = number * 10 + c - '0';
    c = getc(fp);
  }

  if (c == '|')
    number += fread_flag(fp);

  else if  ( c != ' ')
    ungetc(c,fp);

  if (negative)
    return -1 * number;

  return number;
}



// Constants from Circle
typedef enum {
  CIRCLE_SECT_INSIDE, 
  CIRCLE_SECT_CITY, 
  CIRCLE_SECT_FIELD, 
  CIRCLE_SECT_FOREST, 
  CIRCLE_SECT_HILLS, 
  CIRCLE_SECT_MOUNTAIN,
  CIRCLE_SECT_WATER_SWIM, 
  CIRCLE_SECT_WATER_NOSWIM, 
  CIRCLE_SECT_UNDERWATER, 
  CIRCLE_SECT_AIR, 
  CIRCLE_SECT_MAX
} circle_sector_types;

typedef enum {
  CIRCLE_WEAR_NONE = -1, 
  CIRCLE_WEAR_LIGHT = 0, 
  CIRCLE_WEAR_FINGER_L, 
  CIRCLE_WEAR_FINGER_R, 
  CIRCLE_WEAR_NECK_1,
  CIRCLE_WEAR_NECK_2, 
  CIRCLE_WEAR_BODY, 
  CIRCLE_WEAR_HEAD, 
  CIRCLE_WEAR_LEGS, 
  CIRCLE_WEAR_FEET, 
  CIRCLE_WEAR_HANDS,
  CIRCLE_WEAR_ARMS, 
  CIRCLE_WEAR_SHIELD, 
  CIRCLE_WEAR_ABOUT, 
  CIRCLE_WEAR_WAIST, 
  CIRCLE_WEAR_WRIST_L, 
  CIRCLE_WEAR_WRIST_R,
  CIRCLE_WEAR_WIELD, 
  CIRCLE_WEAR_HOLD, 
  CIRCLE_MAX_WEAR
} circle_wear_type;

typedef enum {
  CIRCLE_POS_DEAD, 
  CIRCLE_POS_MORTAL, 
  CIRCLE_POS_INCAP, 
  CIRCLE_POS_STUNNED, 
  CIRCLE_POS_SLEEPING, 
  CIRCLE_POS_RESTING, 
  CIRCLE_POS_AGGRESSIVE, 
  CIRCLE_POS_SITTING, 
  CIRCLE_POS_FIGHTING, 
  CIRCLE_POS_STANDING, 
} circle_positions;


typedef enum {
  CIRCLE_APPLY_NONE, 
  CIRCLE_APPLY_STR, 
  CIRCLE_APPLY_DEX, 
  CIRCLE_APPLY_INT, 
  CIRCLE_APPLY_WIS, 
  CIRCLE_APPLY_CON,
  CIRCLE_APPLY_CHA,
  CIRCLE_APPLY_CLASS, 
  CIRCLE_APPLY_LEVEL, 
  CIRCLE_APPLY_AGE, 
  CIRCLE_APPLY_WEIGHT,
  CIRCLE_APPLY_HEIGHT, 
  CIRCLE_APPLY_MANA, 
  CIRCLE_APPLY_HIT, 
  CIRCLE_APPLY_MOVE, 
  CIRCLE_APPLY_GOLD, 
  CIRCLE_APPLY_EXP, 
  CIRCLE_APPLY_AC,
  CIRCLE_APPLY_HITROLL, 
  CIRCLE_APPLY_DAMROLL, 
  CIRCLE_APPLY_SAVING_PARA, 
  CIRCLE_APPLY_SAVING_ROD,
  CIRCLE_APPLY_SAVING_PETRI, 
  CIRCLE_APPLY_SAVING_BREATH, 
  CIRCLE_APPLY_SAVING_SPELL, 
  CIRCLE_MAX_APPLY_TYPE
} circle_apply_types;

/* 32bit bitvector defines */
#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

#define CIRCLE_ROOM_DARK		BV00
#define CIRCLE_ROOM_DEATH		BV01
#define CIRCLE_ROOM_NO_MOB		BV02
#define CIRCLE_ROOM_INDOORS		BV03
#define CIRCLE_ROOM_PEACEFUL		BV04
#define CIRCLE_ROOM_SOUNDPROOF		BV05
#define CIRCLE_ROOM_NOTRACK		BV06
#define CIRCLE_ROOM_NO_MAGIC		BV07
#define CIRCLE_ROOM_TUNNEL		BV08
#define CIRCLE_ROOM_PRIVATE		BV09
#define CIRCLE_ROOM_GODROOM		BV10
#define CIRCLE_ROOM_HOUSE		BV11
#define CIRCLE_ROOM_HOUSE_CRASH		BV12
#define CIRCLE_ROOM_ATRIUM		BV13
#define CIRCLE_ROOM_OLC 		BV14
#define CIRCLE_ROOM_BPS_MARK		BV15

typedef enum {
  CIRCLE_ITEM_GLOW, 
  CIRCLE_ITEM_HUM, 
  CIRCLE_ITEM_NORENT,
  CIRCLE_ITEM_NODONATE, 
  CIRCLE_ITEM_NOINVIS, 
  CIRCLE_ITEM_INVIS, 
  CIRCLE_ITEM_MAGIC, 
  CIRCLE_ITEM_NODROP, 
  CIRCLE_ITEM_BLESS, 
  CIRCLE_ITEM_ANTI_GOOD, 
  CIRCLE_ITEM_ANTI_EVIL, 
  CIRCLE_ITEM_ANTI_NEUTRAL, 
  CIRCLE_ITEM_ANTI_MAGIC_USER, 
  CIRCLE_ITEM_ANTI_CLERIC, 
  CIRCLE_ITEM_ANTI_THIEF, 
  CIRCLE_ITEM_ANTI_WARRIOR, 
  CIRCLE_ITEM_NOSELL, 
  MAX_CIRCLE_ITEM_FLAG
} circle_item_extra_flags;

typedef enum {
  CIRCLE_ITEM_NONE, 
  CIRCLE_ITEM_LIGHT, 
  CIRCLE_ITEM_SCROLL, 
  CIRCLE_ITEM_WAND, 
  CIRCLE_ITEM_STAFF, 
  CIRCLE_ITEM_WEAPON,
  CIRCLE_ITEM_FIREWEAPON, 
  CIRCLE_ITEM_MISSILE, 
  CIRCLE_ITEM_TREASURE, 
  CIRCLE_ITEM_ARMOR, 
  CIRCLE_ITEM_POTION,
  CIRCLE_ITEM_WORN, 
  CIRCLE_ITEM_OTHER, 
  CIRCLE_ITEM_TRASH, 
  CIRCLE_ITEM_OLDTRAP, 
  CIRCLE_ITEM_CONTAINER,
  CIRCLE_ITEM_NOTE, 
  CIRCLE_ITEM_DRINK_CON, 
  CIRCLE_ITEM_KEY, 
  CIRCLE_ITEM_FOOD, 
  CIRCLE_ITEM_MONEY, 
  CIRCLE_ITEM_PEN,
  CIRCLE_ITEM_BOAT, 
  CIRCLE_ITEM_FOUNTAIN, 
} circle_item_types;

typedef enum {
  ATCK_HIT, 
  ATCK_STING, 
  ATCK_WHIP, 
  ATCK_SLASH, 
  ATCK_BITE, 
  ATCK_BLUDGE,
  ATCK_CRUSH, 
  ATCK_POUND, 
  ATCK_CLAW, 
  ATCK_MAUL, 
  ATCK_TRASH, 
  ATCK_PIERCE,
  ATCK_BLAST,  
  ATCK_PUNCH, 
  ATCK_STAB, 
  MAX_ATTACK_TYPE
} circle_attack_types;

typedef enum {
  CIRCLE_AFF_BLIND, 
  CIRCLE_AFF_INVISIBLE, 
  CIRCLE_AFF_DETECT_ALIGN,
  CIRCLE_AFF_DETECT_INVIS, 
  CIRCLE_AFF_DETECT_MAGIC, 
  CIRCLE_AFF_DETECT_HIDDEN, 
  CIRCLE_AFF_WATERWALK,
  CIRCLE_AFF_SANCTUARY, 
  CIRCLE_AFF_GROUP,
  CIRCLE_AFF_CURSE, 
  CIRCLE_AFF_INFRARED,
  CIRCLE_AFF_POISON, 
  CIRCLE_AFF_PROTECT_EVIL,
  CIRCLE_AFF_PROTECT_GOOD,
  CIRCLE_AFF_SLEEP, 
  CIRCLE_AFF_NOTRACK,
  CIRCLE_AFF_NOTUSED1,
  CIRCLE_AFF_NOTUSED2,
  CIRCLE_AFF_SNEAK,
  CIRCLE_AFF_HIDE,
  CIRCLE_AFF_NOTUSED3,
  CIRCLE_AFF_CHARM, 
  MAX_AFFECTED_BY
} circle_affected_by_types;


#define CIRCLE_ACT_SPEC                   BV00
#define CIRCLE_ACT_SENTINEL		  BV01		// Stays in one room	
#define CIRCLE_ACT_SCAVENGER		  BV02		// Picks up objects	
#define CIRCLE_ACT_IS_NPC		  BV03		// Auto set for mobs	
#define CIRCLE_ACT_AWARE                  BV04
#define CIRCLE_ACT_AGGRESSIVE		  BV05		// Attacks PC's		
#define CIRCLE_ACT_STAY_AREA		  BV06		// Won't leave area	
#define CIRCLE_ACT_WIMPY		  BV07		// Flees when hurt	
#define CIRCLE_ACT_AGGR_EVIL		  BV08
#define CIRCLE_ACT_AGGR_GOOD		  BV09
#define CIRCLE_ACT_AGGR_NEUTRAL		  BV10
#define CIRCLE_ACT_MEMORY                 BV11
#define CIRCLE_ACT_HELPER                 BV12
#define CIRCLE_ACT_NOCHARM                BV13
#define CIRCLE_ACT_NOSUMMON               BV14
#define CIRCLE_ACT_NOSLEEP                BV15
#define CIRCLE_ACT_NOBASH                 BV16
#define CIRCLE_ACT_NOBLIND                BV17



#define CIRCLE_ITEM_TAKE		        BV00
#define CIRCLE_ITEM_WEAR_FINGER	        BV01
#define CIRCLE_ITEM_WEAR_NECK		BV02
#define CIRCLE_ITEM_WEAR_BODY		BV03
#define CIRCLE_ITEM_WEAR_HEAD		BV04
#define CIRCLE_ITEM_WEAR_LEGS		BV05
#define CIRCLE_ITEM_WEAR_FEET		BV06
#define CIRCLE_ITEM_WEAR_HANDS		BV07
#define CIRCLE_ITEM_WEAR_ARMS		BV08
#define CIRCLE_ITEM_WEAR_SHIELD	        BV09
#define CIRCLE_ITEM_WEAR_ABOUT		BV10
#define CIRCLE_ITEM_WEAR_WAIST		BV11
#define CIRCLE_ITEM_WEAR_WRIST		BV12
#define CIRCLE_ITEM_WIELD		BV13
#define CIRCLE_ITEM_HOLD		        BV14


#define SFT(n)  (1L<<(n))
int aff_circle( int aff ) {
  int new_aff = 0;

  if ( aff & SFT(CIRCLE_AFF_BLIND) )         SET_BIT( new_aff, AFF_BLIND );
  if ( aff & SFT(CIRCLE_AFF_INVISIBLE) )     SET_BIT( new_aff, AFF_INVISIBLE );
  if ( aff & SFT(CIRCLE_AFF_DETECT_ALIGN) ) { SET_BIT( new_aff, AFF_DETECT_EVIL ); SET_BIT( new_aff, AFF_DETECT_GOOD ); }
  if ( aff & SFT(CIRCLE_AFF_DETECT_INVIS) )  SET_BIT( new_aff, AFF_DETECT_INVIS );
  if ( aff & SFT(CIRCLE_AFF_DETECT_MAGIC) )  SET_BIT( new_aff, AFF_DETECT_MAGIC );
  if ( aff & SFT(CIRCLE_AFF_DETECT_HIDDEN) ) SET_BIT( new_aff, AFF_DETECT_HIDDEN );
  if ( aff & SFT(CIRCLE_AFF_SANCTUARY) )     SET_BIT( new_aff, AFF_SANCTUARY );
  if ( aff & SFT(CIRCLE_AFF_CURSE) )         SET_BIT( new_aff, AFF_CURSE );
  if ( aff & SFT(CIRCLE_AFF_INFRARED) )      SET_BIT( new_aff, AFF_INFRARED );
  if ( aff & SFT(CIRCLE_AFF_POISON) )        SET_BIT( new_aff, AFF_POISON );
  if ( aff & SFT(CIRCLE_AFF_PROTECT_GOOD ) ) SET_BIT( new_aff, AFF_PROTECT_GOOD );
  if ( aff & SFT(CIRCLE_AFF_PROTECT_EVIL ) ) SET_BIT( new_aff, AFF_PROTECT_EVIL );
  if ( aff & SFT(CIRCLE_AFF_SLEEP) )         SET_BIT( new_aff, AFF_SLEEP );
  if ( aff & SFT(CIRCLE_AFF_SNEAK) )         SET_BIT( new_aff, AFF_SNEAK );
  if ( aff & SFT(CIRCLE_AFF_HIDE) )          SET_BIT( new_aff, AFF_HIDE );
  if ( aff & SFT(CIRCLE_AFF_SLEEP) )         SET_BIT( new_aff, AFF_SLEEP );
  if ( aff & SFT(CIRCLE_AFF_CHARM) )         SET_BIT( new_aff, AFF_CHARM );

  return new_aff;
}

int aff2_circle( int aff ) {
  int new_aff = 0;

  if ( aff & SFT(CIRCLE_AFF_WATERWALK) )   SET_BIT( new_aff, AFF2_WALK_ON_WATER );

  return new_aff;
}

// I know this is crappy
void convert_af_circle( AFFECT_DATA *af ) {
  int aff, aff2;
  for ( AFFECT_LIST *laf = af->list; laf != NULL; laf = laf->next ) {
    switch ( laf->location ) {
    default:
      laf->location = ATTR_NA; 
      laf->modifier = 0;
      break;
    case CIRCLE_APPLY_STR: case CIRCLE_APPLY_DEX: case CIRCLE_APPLY_INT: 
    case CIRCLE_APPLY_WIS: case CIRCLE_APPLY_CON:
      laf->location = ATTR_STR + ( laf->location - CIRCLE_APPLY_STR ); break;
    case CIRCLE_APPLY_MANA:
      laf->location = ATTR_max_mana; break;
    case CIRCLE_APPLY_HIT:
      laf->location = ATTR_max_hit; break;
    case CIRCLE_APPLY_MOVE:
      laf->location = ATTR_max_move; break;
    case CIRCLE_APPLY_AC:
      laf->location = ATTR_allAC; break;
    case CIRCLE_APPLY_HITROLL:
      laf->location = ATTR_hitroll; break;
    case CIRCLE_APPLY_DAMROLL:
      laf->location = ATTR_damroll; break;
    case CIRCLE_APPLY_SAVING_PETRI: case CIRCLE_APPLY_SAVING_ROD: 
    case CIRCLE_APPLY_SAVING_PARA: case CIRCLE_APPLY_SAVING_BREATH:
    case CIRCLE_APPLY_SAVING_SPELL:
      laf->location = ATTR_saving_throw; break;
    }
  }
}

int item_type_circle( int it ) {
  if ( it == CIRCLE_ITEM_LIGHT      ) return ITEM_LIGHT;
  if ( it == CIRCLE_ITEM_SCROLL     ) return ITEM_SCROLL;
  if ( it == CIRCLE_ITEM_WAND       ) return ITEM_WAND;
  if ( it == CIRCLE_ITEM_STAFF      ) return ITEM_STAFF;
  if ( it == CIRCLE_ITEM_WEAPON     ) return ITEM_WEAPON;
  if ( it == CIRCLE_ITEM_FIREWEAPON ) return ITEM_WEAPON;
  if ( it == CIRCLE_ITEM_MISSILE    ) return ITEM_WEAPON;
  if ( it == CIRCLE_ITEM_TREASURE   ) return ITEM_TREASURE;
  if ( it == CIRCLE_ITEM_ARMOR      ) return ITEM_ARMOR;
  if ( it == CIRCLE_ITEM_POTION     ) return ITEM_POTION;
  if ( it == CIRCLE_ITEM_CONTAINER  ) return ITEM_CONTAINER;
  if ( it == CIRCLE_ITEM_DRINK_CON  ) return ITEM_DRINK_CON;
  if ( it == CIRCLE_ITEM_KEY        ) return ITEM_KEY;
  if ( it == CIRCLE_ITEM_FOOD       ) return ITEM_FOOD;
  if ( it == CIRCLE_ITEM_MONEY      ) return ITEM_MONEY;
  if ( it == CIRCLE_ITEM_BOAT       ) return ITEM_BOAT;
  if ( it == CIRCLE_ITEM_FOUNTAIN   ) return ITEM_FOUNTAIN;

  return ITEM_TRASH;
}

int extra_flags_circle( int ef ) {
  int new_ef = 0;

  if ( ef & SFT(CIRCLE_ITEM_GLOW) ) SET_BIT( new_ef, ITEM_GLOW );
  if ( ef & SFT(CIRCLE_ITEM_HUM) ) SET_BIT( new_ef, ITEM_HUM );
  if ( ef & SFT(CIRCLE_ITEM_INVIS) ) SET_BIT( new_ef, ITEM_INVIS );
  if ( ef & SFT(CIRCLE_ITEM_MAGIC) ) SET_BIT( new_ef, ITEM_MAGIC );
  if ( ef & SFT(CIRCLE_ITEM_NODROP) ) SET_BIT( new_ef, ITEM_NODROP );
  if ( ef & SFT(CIRCLE_ITEM_BLESS) ) SET_BIT( new_ef, ITEM_BLESS );
  if ( ef & SFT(CIRCLE_ITEM_ANTI_GOOD) ) SET_BIT( new_ef, ITEM_ANTI_GOOD );
  if ( ef & SFT(CIRCLE_ITEM_ANTI_EVIL) ) SET_BIT( new_ef, ITEM_ANTI_EVIL );
  if ( ef & SFT(CIRCLE_ITEM_ANTI_NEUTRAL) ) SET_BIT( new_ef, ITEM_ANTI_NEUTRAL );
  if ( ef & SFT(CIRCLE_ITEM_NOSELL) ) SET_BIT( new_ef, ITEM_INVENTORY );

  return new_ef;
}

int wear_flags_circle( int wf ) {

  if ( wf == CIRCLE_WEAR_NONE)  return WEAR_NONE ;
  if ( wf == CIRCLE_WEAR_LIGHT)  return WEAR_LIGHT ;
  if ( wf == CIRCLE_WEAR_FINGER_L)  return WEAR_FINGER_L ;
  if ( wf == CIRCLE_WEAR_FINGER_R)  return WEAR_FINGER_R ;
  if ( wf == CIRCLE_WEAR_NECK_1)  return WEAR_NECK_1 ;
  if ( wf == CIRCLE_WEAR_NECK_2)  return WEAR_NECK_2 ;
  if ( wf == CIRCLE_WEAR_BODY)  return WEAR_BODY ;
  if ( wf == CIRCLE_WEAR_HEAD)  return WEAR_HEAD ;
  if ( wf == CIRCLE_WEAR_LEGS)  return WEAR_LEGS ;
  if ( wf == CIRCLE_WEAR_FEET)  return WEAR_FEET ;
  if ( wf == CIRCLE_WEAR_HANDS)  return WEAR_HANDS ;
  if ( wf == CIRCLE_WEAR_ARMS)  return WEAR_ARMS ;
  if ( wf == CIRCLE_WEAR_SHIELD)  return WEAR_SHIELD ;
  if ( wf == CIRCLE_WEAR_ABOUT)  return WEAR_ABOUT ;
  if ( wf == CIRCLE_WEAR_WAIST)  return WEAR_WAIST ;
  if ( wf == CIRCLE_WEAR_WRIST_L)  return WEAR_WRIST_L ;
  if ( wf == CIRCLE_WEAR_WRIST_R)  return WEAR_WRIST_R ;
  if ( wf == CIRCLE_WEAR_WIELD)  return WEAR_WIELD ;
  if ( wf == CIRCLE_WEAR_HOLD)  return WEAR_HOLD ;

  return WEAR_NONE;
}

int item_wear_flags_circle( int wf ) {
  int new_wf = 0;

  if ( wf & CIRCLE_ITEM_TAKE ) SET_BIT( new_wf, ITEM_TAKE );
  if ( wf & CIRCLE_ITEM_WEAR_FINGER ) SET_BIT( new_wf, ITEM_WEAR_FINGER );
  if ( wf & CIRCLE_ITEM_WEAR_NECK ) SET_BIT( new_wf, ITEM_WEAR_NECK );
  if ( wf & CIRCLE_ITEM_WEAR_BODY ) SET_BIT( new_wf, ITEM_WEAR_BODY );
  if ( wf & CIRCLE_ITEM_WEAR_HEAD ) SET_BIT( new_wf, ITEM_WEAR_HEAD );
  if ( wf & CIRCLE_ITEM_WEAR_LEGS ) SET_BIT( new_wf, ITEM_WEAR_LEGS );
  if ( wf & CIRCLE_ITEM_WEAR_FEET ) SET_BIT( new_wf, ITEM_WEAR_FEET );
  if ( wf & CIRCLE_ITEM_WEAR_HANDS ) SET_BIT( new_wf, ITEM_WEAR_HANDS );
  if ( wf & CIRCLE_ITEM_WEAR_ARMS ) SET_BIT( new_wf, ITEM_WEAR_ARMS );
  if ( wf & CIRCLE_ITEM_WEAR_SHIELD ) SET_BIT( new_wf, ITEM_WEAR_SHIELD );
  if ( wf & CIRCLE_ITEM_WEAR_ABOUT ) SET_BIT( new_wf, ITEM_WEAR_ABOUT );
  if ( wf & CIRCLE_ITEM_WEAR_WAIST ) SET_BIT( new_wf, ITEM_WEAR_WAIST );
  if ( wf & CIRCLE_ITEM_WEAR_WRIST ) SET_BIT( new_wf, ITEM_WEAR_WRIST );
  if ( wf & CIRCLE_ITEM_WIELD ) SET_BIT( new_wf, ITEM_WIELD );
  if ( wf & CIRCLE_ITEM_HOLD ) SET_BIT( new_wf, ITEM_HOLD );

  return new_wf;
}

int act_circle( int ac ) {
  int new_ac = ACT_IS_NPC;

  if ( ac & CIRCLE_ACT_SENTINEL ) SET_BIT( new_ac, ACT_SENTINEL );
  if ( ac & CIRCLE_ACT_SCAVENGER ) SET_BIT( new_ac, ACT_SCAVENGER );
  if ( ac & CIRCLE_ACT_AGGRESSIVE ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & CIRCLE_ACT_STAY_AREA ) SET_BIT( new_ac, ACT_STAY_AREA );
  if ( ac & CIRCLE_ACT_WIMPY ) SET_BIT( new_ac, ACT_WIMPY );
  if ( ac & CIRCLE_ACT_AGGR_NEUTRAL ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & CIRCLE_ACT_AGGR_GOOD ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & CIRCLE_ACT_AGGR_EVIL ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  return new_ac;
}

int pos_circle( int po ) {
  return po;
}

int room_flag_circle( int rf ) {
  int new_rf = 0;
  
  if ( IS_SET( rf, CIRCLE_ROOM_DARK ) ) SET_BIT( new_rf, ROOM_DARK );
  if ( IS_SET( rf, CIRCLE_ROOM_NO_MOB ) ) SET_BIT( new_rf, ROOM_NO_MOB );
  if ( IS_SET( rf, CIRCLE_ROOM_INDOORS ) ) SET_BIT( new_rf, ROOM_INDOORS );
  if ( IS_SET( rf, CIRCLE_ROOM_PEACEFUL ) ) SET_BIT( new_rf, ROOM_SAFE );
  if ( IS_SET( rf, CIRCLE_ROOM_NO_MAGIC ) ) SET_BIT( new_rf, ROOM_NOSPELL );
  if ( IS_SET( rf, CIRCLE_ROOM_TUNNEL ) ) SET_BIT( new_rf, ROOM_SOLITARY );
  if ( IS_SET( rf, CIRCLE_ROOM_PRIVATE ) ) SET_BIT( new_rf, ROOM_PRIVATE );
  if ( IS_SET( rf, CIRCLE_ROOM_GODROOM ) ) SET_BIT( new_rf, ROOM_GODS_ONLY );

  return new_rf;
}

int sector_circle( int se ) {
  if ( se == CIRCLE_SECT_INSIDE ) return SECT_INSIDE;
  if ( se == CIRCLE_SECT_CITY ) return SECT_CITY;  
  if ( se == CIRCLE_SECT_FIELD ) return SECT_FIELD;
  if ( se == CIRCLE_SECT_FOREST ) return SECT_FOREST;
  if ( se == CIRCLE_SECT_HILLS ) return SECT_HILLS;
  if ( se == CIRCLE_SECT_MOUNTAIN ) return SECT_MOUNTAIN;
  if ( se == CIRCLE_SECT_WATER_SWIM ) return SECT_WATER_SWIM;
  if ( se == CIRCLE_SECT_WATER_NOSWIM ) return SECT_WATER_NOSWIM;
  if ( se == CIRCLE_SECT_UNDERWATER ) return SECT_UNDERWATER;
  if ( se == CIRCLE_SECT_AIR ) return SECT_AIR;

  return SECT_INSIDE;
}

void convert_mob_circle( MOB_INDEX_DATA *pMobIndex ) {
  pMobIndex->act = act_circle( pMobIndex->act );
  int save_aff = pMobIndex->affected_by;
  pMobIndex->affected_by = aff_circle( save_aff );
  pMobIndex->affected2_by = aff2_circle( save_aff );
  pMobIndex->start_pos = pos_circle( pMobIndex->start_pos );
  pMobIndex->default_pos = pos_circle( pMobIndex->default_pos );
  pMobIndex->race = 0;
  //imm,res,vuln
  pMobIndex->off_flags = race_table[pMobIndex->race].off;
  pMobIndex->imm_flags  |= race_table[pMobIndex->race].imm;
  pMobIndex->res_flags  |= race_table[pMobIndex->race].res;
  pMobIndex->vuln_flags |= race_table[pMobIndex->race].vuln;
  pMobIndex->form      = race_table[pMobIndex->race].form;
  pMobIndex->parts     = race_table[pMobIndex->race].parts;
  
  pMobIndex->size		= SIZE_MEDIUM;
  pMobIndex->material		= str_dup("");

  fix_parts( pMobIndex );
}

// Added by SinaC 2003 for  Circle style
void load_mob_circle( FILE *fp ) {
  MOB_INDEX_DATA *pMobIndex;
 
  log_stringf(" MOBILES");

  if ( !area_last ) {   /* OLC */
    bug( "Load_mob_circle: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
 
    letter                          = fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_mob_circle: # not found.");
      exit( 1 );
    }
 
    vnum                            = fread_number( fp );
    if ( vnum == 0 )
      break;
 
    fBootDb = FALSE;
    if ( get_mob_index( vnum ) != NULL ) {
      bug( "Load_mob_circle: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;
 
    //    pMobIndex                       = (MOB_INDEX_DATA*) alloc_perm( sizeof(*pMobIndex) );
    pMobIndex = new_mob_index();
    pMobIndex->vnum                 = vnum;

    pMobIndex->area                 = area_last;               /* OLC */
    pMobIndex->new_format		= TRUE;
    newmobs++;
    pMobIndex->player_name          = fread_string( fp );
    pMobIndex->short_descr          = fread_string( fp );
    pMobIndex->long_descr           = fread_string_upper( fp );
    pMobIndex->description          = fread_string_upper( fp );

    // Added by SinaC 2003
    pMobIndex->classes              = 0;

    pMobIndex->act                  = fread_flag_circle( fp );
    pMobIndex->affected_by          = fread_flag_circle( fp );
    // Added by SinaC 2003
    pMobIndex->affected2_by          = 0;

    pMobIndex->pShop                = NULL;
    // Added by SinaC 2003
    pMobIndex->align.etho           = 0;
    pMobIndex->align.alignment       = fread_number( fp );

    letter = fread_letter( fp ); // mobile type S or E

    pMobIndex->group                = 0;

    pMobIndex->level                = fread_number( fp );

    // Added by SinaC 2003
    if ( pMobIndex->level < pMobIndex->area->low_range )
      pMobIndex->area->low_range = pMobIndex->level;
    if ( pMobIndex->level > pMobIndex->area->high_range )
      pMobIndex->area->high_range = pMobIndex->level;


           fread_number( fp );   // thac0  not used
    pMobIndex->ac[AC_PIERCE] = fread_number( fp ); // ac
    pMobIndex->ac[AC_BASH]   = pMobIndex->ac[AC_PIERCE];
    pMobIndex->ac[AC_SLASH]  = pMobIndex->ac[AC_PIERCE];
    pMobIndex->ac[AC_EXOTIC] = pMobIndex->ac[AC_PIERCE];

    /* read hit dice */
    pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
    /* 'd'          */                fread_letter( fp ); 
    pMobIndex->hit[DICE_TYPE]       = fread_number( fp );
    /* '+'          */                fread_letter( fp );   
    pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

    pMobIndex->mana[DICE_NUMBER]= 0;
    pMobIndex->mana[DICE_TYPE]	= 0;
    pMobIndex->mana[DICE_BONUS]	= 0;

    pMobIndex->psp[DICE_NUMBER]	= 0;
    pMobIndex->psp[DICE_TYPE]	= 0;
    pMobIndex->psp[DICE_BONUS]	= 0;

    /* read damage dice */
    pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->damage[DICE_BONUS]	= fread_number( fp );

    //pMobIndex->wealth		= fread_number( fp ) * 100;  *100 was not needed
    pMobIndex->wealth		= fread_number( fp );
          fread_number(fp);  // Xp not used

    pMobIndex->start_pos		= fread_number(fp);
    pMobIndex->default_pos		= fread_number(fp);
    pMobIndex->sex			= fread_number(fp);
    
    if ( letter != 'S' && letter != 'E' ) {
      bug("Invalid letter (%c) for mob (%d), valid letter are S or E", letter, pMobIndex->vnum );
      exit(-1);
    }

    pMobIndex->race = 0;
    pMobIndex->hitroll = 0;
    pMobIndex->res_flags = 0;
    pMobIndex->imm_flags = 0;
    pMobIndex->vuln_flags = 0;
    if ( letter == 'E' ) { // complex mob
      while(1) {
	char *ln = fread_line( fp );
	if ( ln[0] == 'E' )
	  break;
      }
    }

    while(1) {
      letter = fread_letter( fp );
      if ( letter == 'T' ) { // Script
	int scriptVnum = fread_number( fp );
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "mob%d", vnum );
	pMobIndex->program = hash_get_prog(buf);
	if (!pMobIndex->program)
	  bug("Can't find program for mob vnum %d.", pMobIndex->vnum);
	else {
	  if ( get_root_class( pMobIndex->program ) != default_mob_class ) {
	    bug("program for mob vnum %d is not a mob program.", pMobIndex->vnum);
	    pMobIndex->program = NULL;
	  }
	  else
	    if ( pMobIndex->program->isAbstract )
	      bug("program for mob vnum %d is an ABSTRACT class.", pMobIndex->vnum );
	}
      }
      else {
	ungetc(letter,fp);
	break;
      }
    }

    convert_mob_circle( pMobIndex );

    iHash                   = vnum % MAX_KEY_HASH;
    pMobIndex->next         = mob_index_hash[iHash];
    mob_index_hash[iHash]   = pMobIndex;
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
    assign_area_vnum( vnum );                                  /* OLC */
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
  }
 
  return;
}

void convert_room_circle( ROOM_INDEX_DATA *pRoomIndex ) {
  // room flags
  pRoomIndex->bstat(flags) = room_flag_circle( pRoomIndex->bstat(flags) );
  // sector
  pRoomIndex->bstat(sector) = sector_circle( pRoomIndex->bstat(sector) );
  // Added by SinaC 2000
  if ( pRoomIndex->bstat(sector) < 0 || pRoomIndex->bstat(sector) > SECT_MAX-1 )
    bug("Invalid sector type (%d) in room %d",
	pRoomIndex->bstat(sector), pRoomIndex->vnum );

  // Added by SinaC 2003 for repop time
  pRoomIndex->current_time_repop = MAX_REPOP_TIME; // so the room will be immediatly updated
  if ( pRoomIndex->time_between_repop < MIN_REPOP_TIME
       || pRoomIndex->time_between_repop >= MAX_REPOP_TIME
       || pRoomIndex->time_between_repop_people < MIN_REPOP_TIME
       || pRoomIndex->time_between_repop_people >= MAX_REPOP_TIME ) {
    pRoomIndex->time_between_repop = BASE_REPOP_TIME;
    pRoomIndex->time_between_repop_people = BASE_REPOP_TIME_PEOPLE;
  }

  // Added by SinaC 2003 for room programs
  if ( pRoomIndex->program != NULL )
    pRoomIndex->clazz = pRoomIndex->program;
  else
    pRoomIndex->clazz = default_room_class;
}

void convert_exit_circle( EXIT_DATA *pexit ) {
  pexit->exit_info = pexit->rs_flags = pexit->exit_info;
}

// Added by SinaC 2003 for  Circle style
void load_room_circle( FILE *fp ) {
  ROOM_INDEX_DATA *pRoomIndex;

  log_stringf(" ROOMS");

  if ( area_last == NULL ) {
    bug( "Load_room_circle: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int door;
    int iHash;
    char *ln;
    int x1, x2, x3, x4, x5, x6;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_room_circle: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_room_index( vnum ) != NULL ) {
      bug( "Load_room_circle: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //    pRoomIndex			= (ROOM_INDEX_DATA *) alloc_perm( sizeof(*pRoomIndex) );
    pRoomIndex = new_room_index();
    pRoomIndex->guild		= 0; /* Not a guild */
    pRoomIndex->owner		= str_dup("");
    pRoomIndex->people		= NULL;
    pRoomIndex->contents		= NULL;
    pRoomIndex->extra_descr		= NULL;
    pRoomIndex->area		= area_last;
    pRoomIndex->vnum		= vnum;
    pRoomIndex->name		= fread_string( fp );
    pRoomIndex->description		= fread_string( fp );
    /* Area number */		  fread_number( fp );

    pRoomIndex->bstat(flags)		= fread_flag_circle(fp);
    pRoomIndex->bstat(sector)		= fread_number(fp);
    pRoomIndex->bstat(maxsize)		= SIZE_NOSIZE;

    pRoomIndex->bstat(light)		= 0;
    for ( door = 0; door < MAX_DIR; door++ ) // Modified by SinaC 2003
      pRoomIndex->exit[door] = NULL;

    // Modified by SinaC 2003
    /* defaults */
    pRoomIndex->bstat(healrate) = 100;
    pRoomIndex->bstat(manarate) = 100;
    // Added by SinaC 2003 for mental user
    pRoomIndex->bstat(psprate) = 100;

    for ( ; ; ) {
      EXIT_DATA *pexit;
      int locks;
      EXTRA_DESCR_DATA *ed;
    
      letter = fread_letter( fp );

      if ( letter == 'S' )
	break;
	    
      switch ( letter ) {
      case 'D' : 
	door = fread_number( fp );
	if ( door < 0 || door >= MAX_DIR ) { // Modified by SinaC 2003
	  bug( "Load_room_circle: vnum %d has bad door number.", vnum );
	  exit( 1 );
	}

	if ( door == DIR_SPECIAL )
	  log_stringf("DIR_SPECIAL found for room vnum: %d.", vnum );

	//	pexit			= (EXIT_DATA *) alloc_perm( sizeof(*pexit) );
	pexit = new_exit();
	pexit->description	= fread_string( fp );
	pexit->keyword		= fread_string( fp );
	pexit->exit_info	= 0;
	pexit->rs_flags         = 0;                    /* OLC */
	locks			= fread_number(fp);
	pexit->key		= fread_number(fp);
	pexit->u1.vnum		= fread_number(fp);
	pexit->orig_door	= door;			/* OLC */

	switch ( locks ) {
	case 1: pexit->exit_info = pexit->rs_flags = EX_ISDOOR; break;
	case 2: pexit->exit_info = pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF; break;
	default: pexit->exit_info = pexit->rs_flags = locks;
	  break;
	}

	convert_exit_circle(pexit);
	// SinaC 2003
	if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
	  log_stringf("Room [%d], exit [%d] has CLIMB flag.",
		      vnum, door );

	pRoomIndex->exit[door]	= pexit;
	pRoomIndex->old_exit[door] = pexit;
	break;

      case 'E' :
	//	ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	ed = new_extra_descr();
	ed->keyword		= fread_string( fp );
	ed->description		= fread_string( fp );
	ed->next		= pRoomIndex->extra_descr;
	pRoomIndex->extra_descr	= ed;
	break;

      default :	    
	bug( "Load_room_circle: vnum %d has unknown flag.", vnum );
	exit( 1 );
      }
    }

    while (1) {
      letter = fread_letter( fp );
      if ( letter == 'T' ) { // Script
	int scriptVnum = fread_number( fp );
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "room%d", scriptVnum );
	pRoomIndex->program = hash_get_prog(buf);
	if (!pRoomIndex->program)
	  bug("Can't find program for room vnum %d.", pRoomIndex->vnum);
	else {
	  if ( get_root_class( pRoomIndex->program ) != default_room_class ) {
	    bug("program for mob vnum %d is not a room program.", pRoomIndex->vnum);
	    pRoomIndex->program = NULL;
	  }
	  else
	    if ( pRoomIndex->program->isAbstract )
	      bug("program for room vnum %d is an ABSTRACT class.", pRoomIndex->vnum );
	}
      }
      else {
	ungetc(letter,fp);
	break;
      }
    }

    convert_room_circle(pRoomIndex);

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum( vnum );                                    /* OLC */
  }

  return;
}

// Added by SinaC 2003 for  Circle style
void load_reset_circle( FILE *fp ) {
  RESET_DATA *pReset;
  int         iLastRoom = 0;
  int         iLastObj  = 0;
  int         iLastObj2  = 0;

  log_stringf(" RESETS");

  if ( !area_last ) {
    bug( "Load_reset_circle: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    ROOM_INDEX_DATA *pRoomIndex;
    char letter;
    OBJ_INDEX_DATA *temp_index;
    /* int temp; */

    if ( ( letter = fread_letter( fp ) ) == 'S' )
      break;

    if ( letter == '*' ) {
      fread_to_eol( fp );
      continue;
    }
	
    //    pReset		= (RESET_DATA *) alloc_perm( sizeof(*pReset) );
    pReset = new_reset_data();
    pReset->command	= letter;

           fread_number( fp ); // not used
    pReset->arg1	= fread_number( fp );
    pReset->arg2	= fread_number( fp );
    pReset->arg3	= (letter == 'G' || letter == 'R') ? 0 : fread_number( fp );
    fread_to_eol( fp );
    pReset->arg4 = pReset->arg2;
    
    /*
     * Validate parameters.
     * We're calling the index functions for the side effect.
     */
    switch ( letter ) {
    default:
      bug( "Load_reset_circle: bad command '%c'.", letter );
      exit( 1 );
      break;

    case 'M':
      //get_mob_index  ( pReset->arg1 );  Circle areas may use mob from areas which will be read later
      /*	    get_room_index ( pReset->arg3 ); */
      // fix added by SinaC 2003
      if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
	bug("reset M has arg2 equals to 0 (room: %d)", pReset->arg3);

      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastRoom = pReset->arg3;
      }
      break;

    case 'O':
      //temp_index = get_obj_index ( pReset->arg1 ); // same for obj
      //temp_index->reset_num++; 
      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = pReset->arg3;
	iLastObj2 = pReset->arg1;
      }
      break;

    case 'P':
      //temp_index = get_obj_index  ( pReset->arg1 ); // same for obj
      //temp_index->reset_num++;
      //get_obj_index  ( pReset->arg1 );
      if ( pReset->arg3 == 0 ) {
	log_stringf("reset P has arg3 equals to 0, fixing to: %d", iLastObj2 );
	pReset->arg3 = iLastObj2;
      }
      if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
	bug("reset P has arg2 equals to 0 (room: %d)", iLastObj);

      if ( ( pRoomIndex = get_room_index ( iLastObj ) ) ) {
	new_reset( pRoomIndex, pReset );
      }
      break;

    case 'G':
    case 'E':
      //temp_index = get_obj_index  ( pReset->arg1 ); // same for obj
      //temp_index->reset_num++; 
      if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = iLastRoom;
	iLastObj2 = pReset->arg1;
      }
      break;

    case 'D': {
      pRoomIndex = get_room_index( pReset->arg1 );
      EXIT_DATA *pexit = NULL;

      if ( pReset->arg2 < 0
	   ||   pReset->arg2 >= MAX_DIR // > (MAX_DIR - 1)  SinaC 2003
	   || !pRoomIndex
	   || !( pexit = pRoomIndex->exit[pReset->arg2] )
	   || !IS_SET( pexit->rs_flags, EX_ISDOOR ) ) {
	if ( pexit == NULL ) {
	  bug("Load_reset_circle: 'D': exit from room [%d] not found", pReset->arg1 );
	  exit(1);
	}
	else
	  bug( "Load_reset_circle: 'D': exit %d not door.", pReset->arg2 );
	break;
	//exit( 1 );
      }

      switch ( pReset->arg3 ) {
      default:
	bug( "Load_reset_circle: 'D': bad 'locks': %d." , pReset->arg3);
      case 0: break;
      case 1:
	SET_BIT( pexit->rs_flags, EX_CLOSED );
	SET_BIT( pexit->exit_info, EX_CLOSED ); break;
      case 2:
	SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
	SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED ); break;
      }

      break;
    }
    case 'R':
      break;
    }
  }
  
  return;
}

void convert_obj_circle( OBJ_INDEX_DATA *pObjIndex ) {

  pObjIndex->item_type = item_type_circle( pObjIndex->item_type );
  if ( pObjIndex->item_type == 0 )
    bug("Convert_obj_circle: invalid item_type for object (%d)",
	pObjIndex->vnum );
  if ( pObjIndex->item_type == ITEM_TRASH )
    bug("Convert_obj_circle: item type is trash for object (%d)",
	pObjIndex->vnum );
  
  pObjIndex->extra_flags = extra_flags_circle( pObjIndex->extra_flags );
  pObjIndex->wear_flags = item_wear_flags_circle( pObjIndex->wear_flags );
}

/*
 * Snarf an obj section.  old style 
 */
void load_obj_circle( FILE *fp ) {
  OBJ_INDEX_DATA *pObjIndex;
  char *word;

  log_stringf(" OBJECTS");

  if ( !area_last ) {  /* OLC */
    bug( "Load_obj_circle: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
    int x1, x2, x3, x4, x5, x6;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_obj_circle: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_obj_index( vnum ) != NULL ) {
      bug( "Load_obj_circle: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    pObjIndex = new_obj_index();
    pObjIndex->vnum			= vnum;
    pObjIndex->area                 = area_last;            /* OLC */
    pObjIndex->new_format		= FALSE;
    pObjIndex->reset_num	 	= 0;
    newobjs++;
    pObjIndex->name			= fread_string( fp );
    pObjIndex->short_descr		= fread_string_lower( fp );
    pObjIndex->description		= fread_string_upper( fp );
    fread_string( fp ); // unused:  action desc

    pObjIndex->material = 0;

    pObjIndex->item_type		= fread_number( fp );
    pObjIndex->extra_flags		= fread_flag_circle( fp );
    pObjIndex->wear_flags		= fread_flag_circle( fp );
    fread_flag_circle(fp); // unused ?

    pObjIndex->size = SIZE_NOSIZE;
    
    pObjIndex->value[0]		= fread_number(fp);
    pObjIndex->value[1]		= fread_number(fp);
    pObjIndex->value[2]		= fread_number(fp);
    pObjIndex->value[3]		= fread_number(fp);

    pObjIndex->weight		= UMAX( 1, fread_number( fp ) );
    pObjIndex->cost			= fread_number( fp );
    fread_number( fp ); // unused:  rent/day
    fread_number(fp); // unused ?

    pObjIndex->condition = 100;
    pObjIndex->level = 1;

    for ( ; ; ) {
      letter = fread_letter( fp );

      if ( letter == 'A' ) {
	AFFECT_DATA *paf;
	
	paf = new_affect();
	createaff(*paf,-1,20,-1,0,AFFECT_INHERENT);
	addaff2(*paf,AFTO_CHAR,fread_number( fp ),AFOP_ADD,fread_number( fp ));
	//paf->where              = AFTO_CHAR;
	//paf->op                 = AFOP_ADD;
	//paf->type		= -1;
	//paf->level		= 20; /* RT temp fix */
	//paf->duration		= -1;
	//paf->location         = fread_number( fp );
	//paf->modifier		= fread_number( fp );

	convert_af_circle( paf );
	
	paf->next		= pObjIndex->affected;
	pObjIndex->affected	= paf;
      }
      else if ( letter == 'E' )	{
	EXTRA_DESCR_DATA *ed;
	
	//	ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	ed = new_extra_descr();
	ed->keyword		= fread_string( fp );
	ed->description		= fread_string( fp );
	ed->next		= pObjIndex->extra_descr;
	pObjIndex->extra_descr	= ed;
      }
      else if ( letter == 'T' ) { // script
	int scriptVnum = fread_number( fp );
	char buf[MAX_STRING_LENGTH];
	sprintf( buf, "obj%d", scriptVnum );
	pObjIndex->program = hash_get_prog(buf);
	if (!pObjIndex->program)
	  bug("Can't find program for obj vnum %d.", pObjIndex->vnum);
	else {
	  if ( get_root_class( pObjIndex->program ) != default_obj_class ) {
	    bug("program for obj vnum %d is not a mob program." ,pObjIndex->vnum);
	    pObjIndex->program = NULL;
	  }
	  else
	    if ( pObjIndex->program->isAbstract )
	      bug("program for Obj vnum %d is an ABSTRACT class.", pObjIndex->vnum );
	}
      }
      else {
	ungetc( letter, fp );
	break;
      }
    }

    switch ( pObjIndex->item_type ) {
    case CIRCLE_ITEM_POTION:
    case CIRCLE_ITEM_SCROLL:
      pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
      pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
      pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
      break;
      
    case CIRCLE_ITEM_STAFF:
    case CIRCLE_ITEM_WAND:
      pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
      break;
    }

    convert_obj_circle( pObjIndex );

    iHash			= vnum % MAX_KEY_HASH;
    pObjIndex->next		= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObjIndex;
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
    assign_area_vnum( vnum );                                   /* OLC */
  }

  return;
}

/*
 * Snarf a shop section.
 */
void load_shops_circle( FILE *fp ) {
  SHOP_DATA *pShop;

  log_stringf(" SHOPS");
  bool OLD = TRUE;
  for ( ; ; ) {
    MOB_INDEX_DATA *pMobIndex;
    int iTrade = 0;

    pShop = new_shop();

    //    log_stringf("====START");

    bool stop = FALSE;
    while (1) {
      char c = fread_letter(fp);
      //      log_stringf("=>[%c]", c );
      if ( c == '0' ) { // If shopvnum is 0: end of shop section
	stop = TRUE;
	break;
      }
      else if ( c == '#' ) {
	const char *s = fread_string(fp);
	//	log_stringf("->[%s]", s );
	break;
      }
      else if ( c < '0' || c > '9' ) { // File starts with a string
	ungetc(c,fp);
	const char *s = fread_string(fp);
	if ( !str_prefix("CircleMUD v3.0 Shop File",s) )
	  OLD = FALSE;
	log_stringf("))[%s]  [%d]", s, !str_prefix( "CircleMUD v3.0 Shop File", s ) );
      }
      else {
	int x = fread_number(fp); // should never happens
	//	log_stringf(">>>[%d]", x );
	break;
      }
    }
    if (stop)
      break;
    log_stringf("OLD: %s", OLD?"yes":"no");
    // Skips infinite obj vnum
    int count = 0;
    while (1) {
      int tmp = fread_number(fp);
      if ( tmp == -1 )
	break;
      count++;
      if ( count >= 5 && OLD )
	break;
    }

    pShop->profit_buy	= (int)(fread_float( fp )*100.0);
    pShop->profit_sell	= (int)(fread_float( fp )*100.0);

    // Read item type
    while (1) {
      //int tmp = fread_number(fp); fread_to_eol(fp); // skips end of line (special ShopV3 format)
      int tmp = 0;
      char letter = fread_letter(fp);
      if ( ( letter >= '0' && letter <= '9' ) || letter == '-' ) { // if we read a number -> item_type
	ungetc(letter,fp);
	tmp = fread_number(fp);
      }
      else                                    // else -> skips
	fread_to_eol(fp);
      if ( tmp == -1 )
	break;
      if ( iTrade < MAX_TRADE )
	pShop->buy_type[iTrade++]	= item_type_circle(tmp);
      else
	log_stringf("Too many item type, [%d] not stored", tmp );
      if ( iTrade >= 5 && OLD )
	break;
    }
    
    // Skips 7 string
    for ( int i = 0; i < 7; i++ ) {
      const char *s = fread_string(fp);
    }

    fread_number(fp); // skips temper
    fread_number(fp); // skips shop bitvector
    pShop->keeper		= fread_number( fp ); // get shop mob vnum
    fread_number(fp); // skips with who bitvector
    // skips shop room vnum
    while (1) {
      int tmp = fread_number(fp);
      if ( tmp == -1 )
	break;
      if ( OLD ) // only one shop vnum for OLD format
	break;
    }
    pShop->open_hour	= URANGE(0,fread_number( fp ),23); // get opening hour
    pShop->close_hour	= URANGE(0,fread_number( fp ),23);
    fread_number(fp); // skips second opening hour
    fread_number(fp); //  "

    pMobIndex		= get_mob_index( pShop->keeper );
    pMobIndex->pShop	= pShop;

    if ( shop_first == NULL )
      shop_first = pShop;
    if ( shop_last  != NULL )
      shop_last->next = pShop;

    shop_last	= pShop;
    pShop->next	= NULL;
  }

  return;
}
