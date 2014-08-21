//#AREA
//	DSA Format~
//	<Filename.are>~
//	<Name of the area>~
//	<Credits>~
//	<MinVnum> <MaxVnum>
//	Descr
//	<Area Description>
//	~
//	Bldrs <Builders>~
//	Sec <Security level>
//	Levels <MinLevel> <MaxLevel>
//	Flags <Area flags>
//	End
//
//
//#HELPS
//<Area help file>
//-1 $~
//
//
//#MOBILES
//	#<mobile vnum>
//	<name>~
//	<short descr>~
//	<long descr>
//	~
//	<description>
//	~
//	<race>~
//	<act>  <affected_by>  <???> <alignment>  <group>
//	<level>  <hitroll>  <hit dice>  <mana dice>  <damage dice>  <damage type>
//	<pierce ac>  <bash ac>  <slash ac>  <exotic ac>
//	<offensive flags>  <immmunites>  <resistances>  <vulnerabilities>
//	<start position>  <default position>  <sex>  <wealth>
//	<form flags>  <part flags>  <size name>  <material>
// *	F <act>  <value>
// *	F <aff>  <value>
// *	F <aff2> <value>
// *	F <off>  <value>
// *	F <imm>  <value>
// *	F <res>  <value>
// *	F <vuln> <value>
// *	F <form> <value>
// *	F <part> <value>
//	~
// *	Damroll <value>
// *	Saves <value>
// *	MaxWorld <value>
//	End
//#0
//
// * are optional
//
//#OBJECTS
//	#<vnum>
//	<name list>~
//	<short description>~
//	<description>~
//	<material name>~
//	<item type name>  <extra flags>  <wear flags>
//	<v0>  <v1>  <v2>  <v3>  <v4>  depending on item_type
//	<level>  <weight>  <cost>  <condition letter>
// *	A <location> <value>
// *     	E <extra description keyword>~
//	  <extra description>~
//	~
// *	Spec <value>~  (flaming, everful_purse)
// *	MaxWorld <value>
//	End
//#0
//
// * are optional
//
//
//#ROOMS
//	#<vnum>
//	<name list>~
//	<description>~
//	0  <room flags>  <sector type>
//	<maximum mobiles allowed size>
// *	E <extra description keyword>~
//	  <extra description>~
// *	D <exit number 0..10>
//	<exit description>~
//	<exit keyword>~
//	<exit flags>  <exit key>  <to room vnum>
// *	Flags <exit flags>
//        End
// *	M <mana recovery rate>  H <heal recovery rate>  P <psp recovery rate>>
// *	C <clan name>~
// *	O <owner name>~
//	S
//	Reset <see reset format>
//	End
//#0
//
// * are optional
//
//
//#SPECIALS
//	M <mobile vnum> <special function name>
//S
//
//#SHOPS
//	<mob#> <items> <profit-buy> <p-sell> <op-hour> <close> 
//	<mobile vnum> <items type 1> ...<items type 5> <profit buy> <profit sell> <open hour> <close hour>
//0
//
//#$

// This file has been written by SinaC 2003 to read Dsa area file

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
#include "config.h"
#include "interp.h"
#include "data_scanner.hh"


// Constants from Dsa
typedef enum {
  DSA_SECT_INSIDE, 
  DSA_SECT_CITY, 
  DSA_SECT_FIELD, 
  DSA_SECT_FOREST, 
  DSA_SECT_HILLS, 
  DSA_SECT_MOUNTAIN,
  DSA_SECT_WATER_SWIM, 
  DSA_SECT_WATER_NOSWIM, 
  DSA_SECT_UNDERWATER, 
  DSA_SECT_AIR, 
  DSA_SECT_MAX
} dsa_sector_types;

typedef enum {
  DSA_WEAR_NONE = -1, 
  DSA_WEAR_LIGHT = 0, 
  DSA_WEAR_FINGER_L, 
  DSA_WEAR_FINGER_R, 
  DSA_WEAR_NECK_1,
  DSA_WEAR_NECK_2, 
  DSA_WEAR_BODY, 
  DSA_WEAR_HEAD, 
  DSA_WEAR_LEGS, 
  DSA_WEAR_FEET, 
  DSA_WEAR_HANDS,
  DSA_WEAR_ARMS, 
  DSA_WEAR_SHIELD, 
  DSA_WEAR_ABOUT, 
  DSA_WEAR_WAIST, 
  DSA_WEAR_WRIST_L, 
  DSA_WEAR_WRIST_R,
  DSA_WEAR_WIELD, 
  DSA_WEAR_HOLD, 
  DSA_MAX_WEAR
} dsa_wear_type;

typedef enum {
  DSA_POS_DEAD, 
  DSA_POS_MORTAL, 
  DSA_POS_INCAP, 
  DSA_POS_STUNNED, 
  DSA_POS_SLEEPING, 
  DSA_POS_RESTING, 
  DSA_POS_AGGRESSIVE, 
  DSA_POS_SITTING, 
  DSA_POS_FIGHTING, 
  DSA_POS_STANDING, 
} dsa_positions;


typedef enum {
  DSA_APPLY_NONE, 
  DSA_APPLY_STR, 
  DSA_APPLY_DEX, 
  DSA_APPLY_INT, 
  DSA_APPLY_WIS, 
  DSA_APPLY_CON,
  DSA_APPLY_CHA,
  DSA_APPLY_CLASS, 
  DSA_APPLY_LEVEL, 
  DSA_APPLY_AGE, 
  DSA_APPLY_WEIGHT,
  DSA_APPLY_HEIGHT, 
  DSA_APPLY_MANA, 
  DSA_APPLY_HIT, 
  DSA_APPLY_MOVE, 
  DSA_APPLY_GOLD, 
  DSA_APPLY_EXP, 
  DSA_APPLY_AC,
  DSA_APPLY_HITROLL, 
  DSA_APPLY_DAMROLL, 
  DSA_APPLY_SAVING_PARA, 
  DSA_APPLY_SAVING_ROD,
  DSA_APPLY_SAVING_PETRI, 
  DSA_APPLY_SAVING_BREATH, 
  DSA_APPLY_SAVING_SPELL, 
  DSA_MAX_APPLY_TYPE
} dsa_apply_types;

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

#define DSA_ROOM_DARK		BV00
#define DSA_ROOM_DEATH		BV01
#define DSA_ROOM_NO_MOB		BV02
#define DSA_ROOM_INDOORS		BV03
#define DSA_ROOM_PEACEFUL		BV04
#define DSA_ROOM_SOUNDPROOF		BV05
#define DSA_ROOM_NOTRACK		BV06
#define DSA_ROOM_NO_MAGIC		BV07
#define DSA_ROOM_TUNNEL		BV08
#define DSA_ROOM_PRIVATE		BV09
#define DSA_ROOM_GODROOM		BV10
#define DSA_ROOM_HOUSE		BV11
#define DSA_ROOM_HOUSE_CRASH		BV12
#define DSA_ROOM_ATRIUM		BV13
#define DSA_ROOM_OLC 		BV14
#define DSA_ROOM_BPS_MARK		BV15

typedef enum {
  DSA_ITEM_GLOW, 
  DSA_ITEM_HUM, 
  DSA_ITEM_NORENT,
  DSA_ITEM_NODONATE, 
  DSA_ITEM_NOINVIS, 
  DSA_ITEM_INVIS, 
  DSA_ITEM_MAGIC, 
  DSA_ITEM_NODROP, 
  DSA_ITEM_BLESS, 
  DSA_ITEM_ANTI_GOOD, 
  DSA_ITEM_ANTI_EVIL, 
  DSA_ITEM_ANTI_NEUTRAL, 
  DSA_ITEM_ANTI_MAGIC_USER, 
  DSA_ITEM_ANTI_CLERIC, 
  DSA_ITEM_ANTI_THIEF, 
  DSA_ITEM_ANTI_WARRIOR, 
  DSA_ITEM_NOSELL, 
  MAX_DSA_ITEM_FLAG
} dsa_item_extra_flags;

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
} dsa_attack_types;

typedef enum {
  DSA_AFF_BLIND, 
  DSA_AFF_INVISIBLE, 
  DSA_AFF_DETECT_ALIGN,
  DSA_AFF_DETECT_INVIS, 
  DSA_AFF_DETECT_MAGIC, 
  DSA_AFF_DETECT_HIDDEN, 
  DSA_AFF_WATERWALK,
  DSA_AFF_SANCTUARY, 
  DSA_AFF_GROUP,
  DSA_AFF_CURSE, 
  DSA_AFF_INFRARED,
  DSA_AFF_POISON, 
  DSA_AFF_PROTECT_EVIL,
  DSA_AFF_PROTECT_GOOD,
  DSA_AFF_SLEEP, 
  DSA_AFF_NOTRACK,
  DSA_AFF_NOTUSED1,
  DSA_AFF_NOTUSED2,
  DSA_AFF_SNEAK,
  DSA_AFF_HIDE,
  DSA_AFF_NOTUSED3,
  DSA_AFF_CHARM, 
  MAX_AFFECTED_BY
} dsa_affected_by_types;


#define DSA_ACT_SPEC                   BV00
#define DSA_ACT_SENTINEL		  BV01		// Stays in one room	
#define DSA_ACT_SCAVENGER		  BV02		// Picks up objects	
#define DSA_ACT_IS_NPC		  BV03		// Auto set for mobs	
#define DSA_ACT_AWARE                  BV04
#define DSA_ACT_AGGRESSIVE		  BV05		// Attacks PC's		
#define DSA_ACT_STAY_AREA		  BV06		// Won't leave area	
#define DSA_ACT_WIMPY		  BV07		// Flees when hurt	
#define DSA_ACT_AGGR_EVIL		  BV08
#define DSA_ACT_AGGR_GOOD		  BV09
#define DSA_ACT_AGGR_NEUTRAL		  BV10
#define DSA_ACT_MEMORY                 BV11
#define DSA_ACT_HELPER                 BV12
#define DSA_ACT_NOCHARM                BV13
#define DSA_ACT_NOSUMMON               BV14
#define DSA_ACT_NOSLEEP                BV15
#define DSA_ACT_NOBASH                 BV16
#define DSA_ACT_NOBLIND                BV17



#define DSA_ITEM_TAKE		        BV00
#define DSA_ITEM_WEAR_FINGER	        BV01
#define DSA_ITEM_WEAR_NECK		BV02
#define DSA_ITEM_WEAR_BODY		BV03
#define DSA_ITEM_WEAR_HEAD		BV04
#define DSA_ITEM_WEAR_LEGS		BV05
#define DSA_ITEM_WEAR_FEET		BV06
#define DSA_ITEM_WEAR_HANDS		BV07
#define DSA_ITEM_WEAR_ARMS		BV08
#define DSA_ITEM_WEAR_SHIELD	        BV09
#define DSA_ITEM_WEAR_ABOUT		BV10
#define DSA_ITEM_WEAR_WAIST		BV11
#define DSA_ITEM_WEAR_WRIST		BV12
#define DSA_ITEM_WIELD		BV13
#define DSA_ITEM_HOLD		        BV14


#define SFT(n)  (1L<<(n))

int aff_dsa( int aff ) {
  int new_aff = 0;

  if ( aff & SFT(DSA_AFF_BLIND) )         SET_BIT( new_aff, AFF_BLIND );
  if ( aff & SFT(DSA_AFF_INVISIBLE) )     SET_BIT( new_aff, AFF_INVISIBLE );
  if ( aff & SFT(DSA_AFF_DETECT_ALIGN) ) { SET_BIT( new_aff, AFF_DETECT_EVIL ); SET_BIT( new_aff, AFF_DETECT_GOOD ); }
  if ( aff & SFT(DSA_AFF_DETECT_INVIS) )  SET_BIT( new_aff, AFF_DETECT_INVIS );
  if ( aff & SFT(DSA_AFF_DETECT_MAGIC) )  SET_BIT( new_aff, AFF_DETECT_MAGIC );
  if ( aff & SFT(DSA_AFF_DETECT_HIDDEN) ) SET_BIT( new_aff, AFF_DETECT_HIDDEN );
  if ( aff & SFT(DSA_AFF_SANCTUARY) )     SET_BIT( new_aff, AFF_SANCTUARY );
  if ( aff & SFT(DSA_AFF_CURSE) )         SET_BIT( new_aff, AFF_CURSE );
  if ( aff & SFT(DSA_AFF_INFRARED) )      SET_BIT( new_aff, AFF_INFRARED );
  if ( aff & SFT(DSA_AFF_POISON) )        SET_BIT( new_aff, AFF_POISON );
  if ( aff & SFT(DSA_AFF_PROTECT_GOOD ) ) SET_BIT( new_aff, AFF_PROTECT_GOOD );
  if ( aff & SFT(DSA_AFF_PROTECT_EVIL ) ) SET_BIT( new_aff, AFF_PROTECT_EVIL );
  if ( aff & SFT(DSA_AFF_SLEEP) )         SET_BIT( new_aff, AFF_SLEEP );
  if ( aff & SFT(DSA_AFF_SNEAK) )         SET_BIT( new_aff, AFF_SNEAK );
  if ( aff & SFT(DSA_AFF_HIDE) )          SET_BIT( new_aff, AFF_HIDE );
  if ( aff & SFT(DSA_AFF_SLEEP) )         SET_BIT( new_aff, AFF_SLEEP );
  if ( aff & SFT(DSA_AFF_CHARM) )         SET_BIT( new_aff, AFF_CHARM );

  return new_aff;
}

int aff2_dsa( int aff ) {
  int new_aff = 0;

  if ( aff & SFT(DSA_AFF_WATERWALK) )   SET_BIT( new_aff, AFF2_WALK_ON_WATER );

  return new_aff;
}

// I know this is crappy
void convert_af_dsa( AFFECT_DATA *af ) {
  int aff, aff2;
  for ( AFFECT_LIST *laf = af->list; laf != NULL; laf = laf->next ) {
    switch ( laf->location ) {
    default:
      laf->location = ATTR_NA; 
      laf->modifier = 0;
      break;
    case DSA_APPLY_STR: case DSA_APPLY_DEX: case DSA_APPLY_INT: 
    case DSA_APPLY_WIS: case DSA_APPLY_CON:
      laf->location = ATTR_STR + ( laf->location - DSA_APPLY_STR ); break;
    case DSA_APPLY_MANA:
      laf->location = ATTR_max_mana; break;
    case DSA_APPLY_HIT:
      laf->location = ATTR_max_hit; break;
    case DSA_APPLY_MOVE:
      laf->location = ATTR_max_move; break;
    case DSA_APPLY_AC:
      laf->location = ATTR_allAC; break;
    case DSA_APPLY_HITROLL:
      laf->location = ATTR_hitroll; break;
    case DSA_APPLY_DAMROLL:
      laf->location = ATTR_damroll; break;
    case DSA_APPLY_SAVING_PETRI: case DSA_APPLY_SAVING_ROD: 
    case DSA_APPLY_SAVING_PARA: case DSA_APPLY_SAVING_BREATH:
    case DSA_APPLY_SAVING_SPELL:
      laf->location = ATTR_saving_throw; break;
    }
  }
}

int extra_flags_dsa( int ef ) {
  int new_ef = 0;

  if ( ef & SFT(DSA_ITEM_GLOW) ) SET_BIT( new_ef, ITEM_GLOW );
  if ( ef & SFT(DSA_ITEM_HUM) ) SET_BIT( new_ef, ITEM_HUM );
  if ( ef & SFT(DSA_ITEM_INVIS) ) SET_BIT( new_ef, ITEM_INVIS );
  if ( ef & SFT(DSA_ITEM_MAGIC) ) SET_BIT( new_ef, ITEM_MAGIC );
  if ( ef & SFT(DSA_ITEM_NODROP) ) SET_BIT( new_ef, ITEM_NODROP );
  if ( ef & SFT(DSA_ITEM_BLESS) ) SET_BIT( new_ef, ITEM_BLESS );
  if ( ef & SFT(DSA_ITEM_ANTI_GOOD) ) SET_BIT( new_ef, ITEM_ANTI_GOOD );
  if ( ef & SFT(DSA_ITEM_ANTI_EVIL) ) SET_BIT( new_ef, ITEM_ANTI_EVIL );
  if ( ef & SFT(DSA_ITEM_ANTI_NEUTRAL) ) SET_BIT( new_ef, ITEM_ANTI_NEUTRAL );
  if ( ef & SFT(DSA_ITEM_NOSELL) ) SET_BIT( new_ef, ITEM_INVENTORY );

  return new_ef;
}

int wear_flags_dsa( int wf ) {

  if ( wf == DSA_WEAR_NONE)  return WEAR_NONE ;
  if ( wf == DSA_WEAR_LIGHT)  return WEAR_LIGHT ;
  if ( wf == DSA_WEAR_FINGER_L)  return WEAR_FINGER_L ;
  if ( wf == DSA_WEAR_FINGER_R)  return WEAR_FINGER_R ;
  if ( wf == DSA_WEAR_NECK_1)  return WEAR_NECK_1 ;
  if ( wf == DSA_WEAR_NECK_2)  return WEAR_NECK_2 ;
  if ( wf == DSA_WEAR_BODY)  return WEAR_BODY ;
  if ( wf == DSA_WEAR_HEAD)  return WEAR_HEAD ;
  if ( wf == DSA_WEAR_LEGS)  return WEAR_LEGS ;
  if ( wf == DSA_WEAR_FEET)  return WEAR_FEET ;
  if ( wf == DSA_WEAR_HANDS)  return WEAR_HANDS ;
  if ( wf == DSA_WEAR_ARMS)  return WEAR_ARMS ;
  if ( wf == DSA_WEAR_SHIELD)  return WEAR_SHIELD ;
  if ( wf == DSA_WEAR_ABOUT)  return WEAR_ABOUT ;
  if ( wf == DSA_WEAR_WAIST)  return WEAR_WAIST ;
  if ( wf == DSA_WEAR_WRIST_L)  return WEAR_WRIST_L ;
  if ( wf == DSA_WEAR_WRIST_R)  return WEAR_WRIST_R ;
  if ( wf == DSA_WEAR_WIELD)  return WEAR_WIELD ;
  if ( wf == DSA_WEAR_HOLD)  return WEAR_HOLD ;

  return WEAR_NONE;
}

int item_wear_flags_dsa( int wf ) {
  int new_wf = 0;

  if ( wf & DSA_ITEM_TAKE ) SET_BIT( new_wf, ITEM_TAKE );
  if ( wf & DSA_ITEM_WEAR_FINGER ) SET_BIT( new_wf, ITEM_WEAR_FINGER );
  if ( wf & DSA_ITEM_WEAR_NECK ) SET_BIT( new_wf, ITEM_WEAR_NECK );
  if ( wf & DSA_ITEM_WEAR_BODY ) SET_BIT( new_wf, ITEM_WEAR_BODY );
  if ( wf & DSA_ITEM_WEAR_HEAD ) SET_BIT( new_wf, ITEM_WEAR_HEAD );
  if ( wf & DSA_ITEM_WEAR_LEGS ) SET_BIT( new_wf, ITEM_WEAR_LEGS );
  if ( wf & DSA_ITEM_WEAR_FEET ) SET_BIT( new_wf, ITEM_WEAR_FEET );
  if ( wf & DSA_ITEM_WEAR_HANDS ) SET_BIT( new_wf, ITEM_WEAR_HANDS );
  if ( wf & DSA_ITEM_WEAR_ARMS ) SET_BIT( new_wf, ITEM_WEAR_ARMS );
  if ( wf & DSA_ITEM_WEAR_SHIELD ) SET_BIT( new_wf, ITEM_WEAR_SHIELD );
  if ( wf & DSA_ITEM_WEAR_ABOUT ) SET_BIT( new_wf, ITEM_WEAR_ABOUT );
  if ( wf & DSA_ITEM_WEAR_WAIST ) SET_BIT( new_wf, ITEM_WEAR_WAIST );
  if ( wf & DSA_ITEM_WEAR_WRIST ) SET_BIT( new_wf, ITEM_WEAR_WRIST );
  if ( wf & DSA_ITEM_WIELD ) SET_BIT( new_wf, ITEM_WIELD );
  if ( wf & DSA_ITEM_HOLD ) SET_BIT( new_wf, ITEM_HOLD );

  return new_wf;
}

int act_dsa( int ac ) {
  int new_ac = ACT_IS_NPC;

  if ( ac & DSA_ACT_SENTINEL ) SET_BIT( new_ac, ACT_SENTINEL );
  if ( ac & DSA_ACT_SCAVENGER ) SET_BIT( new_ac, ACT_SCAVENGER );
  if ( ac & DSA_ACT_AGGRESSIVE ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & DSA_ACT_STAY_AREA ) SET_BIT( new_ac, ACT_STAY_AREA );
  if ( ac & DSA_ACT_WIMPY ) SET_BIT( new_ac, ACT_WIMPY );
  if ( ac & DSA_ACT_AGGR_NEUTRAL ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & DSA_ACT_AGGR_GOOD ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & DSA_ACT_AGGR_EVIL ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  return new_ac;
}

int room_flag_dsa( int rf ) {
  int new_rf = 0;
  
  if ( IS_SET( rf, DSA_ROOM_DARK ) ) SET_BIT( new_rf, ROOM_DARK );
  if ( IS_SET( rf, DSA_ROOM_NO_MOB ) ) SET_BIT( new_rf, ROOM_NO_MOB );
  if ( IS_SET( rf, DSA_ROOM_INDOORS ) ) SET_BIT( new_rf, ROOM_INDOORS );
  if ( IS_SET( rf, DSA_ROOM_PEACEFUL ) ) SET_BIT( new_rf, ROOM_SAFE );
  if ( IS_SET( rf, DSA_ROOM_NO_MAGIC ) ) SET_BIT( new_rf, ROOM_NOSPELL );
  if ( IS_SET( rf, DSA_ROOM_TUNNEL ) ) SET_BIT( new_rf, ROOM_SOLITARY );
  if ( IS_SET( rf, DSA_ROOM_PRIVATE ) ) SET_BIT( new_rf, ROOM_PRIVATE );
  if ( IS_SET( rf, DSA_ROOM_GODROOM ) ) SET_BIT( new_rf, ROOM_GODS_ONLY );

  return new_rf;
}

int sector_dsa( int se ) {
  if ( se == DSA_SECT_INSIDE ) return SECT_INSIDE;
  if ( se == DSA_SECT_CITY ) return SECT_CITY;  
  if ( se == DSA_SECT_FIELD ) return SECT_FIELD;
  if ( se == DSA_SECT_FOREST ) return SECT_FOREST;
  if ( se == DSA_SECT_HILLS ) return SECT_HILLS;
  if ( se == DSA_SECT_MOUNTAIN ) return SECT_MOUNTAIN;
  if ( se == DSA_SECT_WATER_SWIM ) return SECT_WATER_SWIM;
  if ( se == DSA_SECT_WATER_NOSWIM ) return SECT_WATER_NOSWIM;
  if ( se == DSA_SECT_UNDERWATER ) return SECT_UNDERWATER;
  if ( se == DSA_SECT_AIR ) return SECT_AIR;

  return SECT_INSIDE;
}

void convert_mob_dsa( MOB_INDEX_DATA *pMobIndex ) {
  // FIXME: need to convert ?
  //  pMobIndex->act = act_dsa( pMobIndex->act );
  //  int save_aff = pMobIndex->affected_by;
  //  pMobIndex->affected_by = aff_dsa( save_aff );
  //  pMobIndex->affected2_by = aff2_dsa( save_aff );
  fix_parts( pMobIndex );
}

// Added by SinaC 2003 for  Dsa style
void load_mob_dsa( FILE *fp ) {
  MOB_INDEX_DATA *pMobIndex;
 
  if ( !area_last ) {   /* OLC */
    bug( "Load_mob_dsa: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
 
    letter                          = fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_mob_dsa: # not found.");
      exit( 1 );
    }
 
    vnum                            = fread_number( fp );
    if ( vnum == 0 )
      break;
 
    fBootDb = FALSE;
    if ( get_mob_index( vnum ) != NULL ) {
      bug( "Load_mob_dsa: vnum %d duplicated.", vnum );
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

    const char *race = fread_string( fp );
    if ( ( pMobIndex->race = race_lookup( race ) ) < 0 ) {
      bug("Invalid race [%s] for mob [%d], assuming %s",
	  race, vnum, DEFAULT_RACE_NAME );
      pMobIndex->race = DEFAULT_RACE;
    }

    // Added by SinaC 2003
    pMobIndex->classes              = 0;

    pMobIndex->act                  = fread_flag( fp );
    pMobIndex->affected_by          = fread_flag( fp );
    pMobIndex->affected2_by          = 0;
    fread_flag(fp);// FIXME: skips unknown flags
    pMobIndex->pShop                = NULL;
    pMobIndex->align.etho           = 0;
    pMobIndex->align.alignment      = fread_number( fp );
    pMobIndex->group                = fread_number(fp);

    pMobIndex->level                = fread_number( fp );
    fread_number( fp );   // thac0  not used
    /* read hit dice */
    pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
    /* 'd'          */                fread_letter( fp ); 
    pMobIndex->hit[DICE_TYPE]       = fread_number( fp );
    /* '+'          */                fread_letter( fp );   
    pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

    /* read mana dice */
    pMobIndex->mana[DICE_NUMBER]     = fread_number( fp );  
    /* 'd'          */                fread_letter( fp ); 
    pMobIndex->mana[DICE_TYPE]       = fread_number( fp );
    /* '+'          */                fread_letter( fp );   
    pMobIndex->mana[DICE_BONUS]      = fread_number( fp ); 

    pMobIndex->psp[DICE_NUMBER]	= 0;
    pMobIndex->psp[DICE_TYPE]	= 0;
    pMobIndex->psp[DICE_BONUS]	= 0;

    /* read damage dice */
    pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->damage[DICE_BONUS]	= fread_number( fp );

    const char *word = fread_word(fp);
    if ( ( pMobIndex->dam_type = attack_lookup(word) ) < 0 ) {
      bug("Invalid dam type [%s] for mob [%d], assuming NONE",
	  word, vnum );
      pMobIndex->dam_type = 0;
    }

    // Armor class
    pMobIndex->ac[AC_PIERCE] = fread_number( fp );
    pMobIndex->ac[AC_BASH]   = fread_number( fp );
    pMobIndex->ac[AC_SLASH]  = fread_number( fp );
    pMobIndex->ac[AC_EXOTIC] = fread_number( fp );

    /* read flags and add in data from the race table */
    pMobIndex->off_flags		= fread_flag( fp ) 
      | race_table[pMobIndex->race].off;
    pMobIndex->imm_flags		= fread_flag( fp )
      | race_table[pMobIndex->race].imm;
    pMobIndex->res_flags		= fread_flag( fp )
      | race_table[pMobIndex->race].res;
    pMobIndex->vuln_flags		= fread_flag( fp )
      | race_table[pMobIndex->race].vuln;

    /* vital statistics */
    pMobIndex->start_pos		= position_lookup(fread_word(fp));
    if ( pMobIndex->start_pos < 0 ) {
      bug("Invalid starting position for mob %d, assuming standing.", pMobIndex->vnum );
      pMobIndex->start_pos = POS_STANDING;
    }
    pMobIndex->default_pos		= position_lookup(fread_word(fp));
    if ( pMobIndex->default_pos < 0 ) {
      bug("Invalid default position for mob %d, assuming standing.", pMobIndex->vnum );
      pMobIndex->default_pos = POS_STANDING;
    }
    pMobIndex->sex			= sex_lookup(fread_word(fp));
    if ( pMobIndex->sex < 0 ) {
      bug("Invalid sex for mob %d, assuming neutral.", pMobIndex->vnum );
      pMobIndex->sex = SEX_NEUTRAL;
    }
    pMobIndex->wealth		= fread_number( fp );

    // Form & parts
    pMobIndex->form			= fread_flag( fp )
      | race_table[pMobIndex->race].form;
    pMobIndex->parts		= fread_flag( fp )
      | race_table[pMobIndex->race].parts;
    // Size
    pMobIndex->size			= size_lookup(fread_word(fp));
    if ( pMobIndex->size < 0 ) {
      bug("Invalid size for mob %d, assuming medium.", pMobIndex->vnum );
      pMobIndex->size = SIZE_MEDIUM;
    }
    // Material
    pMobIndex->material		= str_dup(fread_word( fp ));

    // Remove flag
    for ( ; ; ) {
      letter = fread_letter( fp );
      if (letter == 'F') {
	long vector;

	word                    = fread_word(fp);
	vector			= fread_flag(fp);

	if (!str_prefix(word,"act"))
	  REMOVE_BIT(pMobIndex->act,vector);
	else if (!str_prefix(word,"aff"))
	  REMOVE_BIT(pMobIndex->affected_by,vector);
	else if (!str_prefix(word,"aff2"))
	  REMOVE_BIT(pMobIndex->affected_by,vector);
	else if (!str_prefix(word,"off"))
	  REMOVE_BIT(pMobIndex->off_flags,vector);
	else if (!str_prefix(word,"imm"))
	  REMOVE_BIT(pMobIndex->imm_flags,vector);
	else if (!str_prefix(word,"res"))
	  REMOVE_BIT(pMobIndex->res_flags,vector);
	else if (!str_prefix(word,"vul"))
	  REMOVE_BIT(pMobIndex->vuln_flags,vector);
	else if (!str_prefix(word,"for"))
	  REMOVE_BIT(pMobIndex->form,vector);
	else if (!str_prefix(word,"par"))
	  REMOVE_BIT(pMobIndex->parts,vector);
	else {
	  bug("Flag remove: flag not found.");
	  exit(1);
	}
      }
      else {
	ungetc(letter,fp);
	break;
      }
    }

    fread_string(fp); // Skips a line

    bool stop = FALSE;
    for ( ; ; ) {
      word   = feof( fp ) ? "End" : fread_word( fp );
      if ( !str_cmp( word, "Damroll" ) )
	pMobIndex->damage[DICE_BONUS] += fread_number(fp);
      else if ( !str_cmp( word, "End" ) )
	stop = TRUE;
      else if ( !str_cmp( word, "Saves" ) )
	//pMobIndex->saving_throw += fread_number(fp);
	fread_number(fp);
      else if ( !str_cmp( word, "MaxWorld" ) )
	fread_number(fp);
      else
	bug("Unknown additional information [%s] for mob [%d]",
	    word, vnum );
      if ( stop )
	break;
    }

    convert_mob_dsa( pMobIndex );

    iHash                   = vnum % MAX_KEY_HASH;
    pMobIndex->next         = mob_index_hash[iHash];
    mob_index_hash[iHash]   = pMobIndex;
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
    assign_area_vnum( vnum );                                  /* OLC */
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
  }
 
  return;
}

void convert_room_dsa( ROOM_INDEX_DATA *pRoomIndex ) {
  // room flags
  pRoomIndex->bstat(flags) = room_flag_dsa( pRoomIndex->bstat(flags) );
  // sector
  pRoomIndex->bstat(sector) = sector_dsa( pRoomIndex->bstat(sector) );
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

// Added by SinaC 2003 for  Dsa style
void load_room_dsa( FILE *fp ) {
  ROOM_INDEX_DATA *pRoomIndex;
  const char *word;

  if ( area_last == NULL ) {
    bug( "Load_room_dsa: no #AREA seen yet.");
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
      bug( "Load_room_dsa: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_room_index( vnum ) != NULL ) {
      bug( "Load_room_dsa: vnum %d duplicated.", vnum );
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

    pRoomIndex->bstat(flags)		= fread_flag(fp);
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
      case 'H' : /* healing room */
	pRoomIndex->bstat(healrate) = fread_number(fp);
	break;
	
      case 'M' : /* mana room */
	pRoomIndex->bstat(manarate) = fread_number(fp);
	break;

      case 'D' : 
	door = fread_number( fp );
	if ( door < 0 || door >= MAX_DIR ) { // Modified by SinaC 2003
	  bug( "Load_room_dsa: vnum %d has bad door number.", vnum );
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

	// SinaC 2003
	if ( IS_SET( pexit->exit_info, EX_CLIMB ) )
	  log_stringf("Room [%d], exit [%d] has CLIMB flag.",
		      vnum, door );

	letter = fread_letter(fp);
	if ( letter == 'F' ) {
	  fread_word(fp); // skips Flags
	  pexit->exit_info = pexit->rs_flags = fread_flag(fp);
	}
	else
	  ungetc(letter,fp);

	pRoomIndex->exit[door]	= pexit;
	pRoomIndex->old_exit[door] = pexit;
	break;

      case 'E' :
	word = fread_word(fp);
	if ( !str_cmp( word, "nd" ) )
	  ; // NOP
	else {
	  //	ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	  ed = new_extra_descr();
	  //ed->keyword		= fread_string( fp );
	  if ( strstr( word, "~" ) ) {
	    ed->keyword		= str_dup(word);
	  }
	  else {
	    char buf[MAX_INPUT_LENGTH];
	    sprintf( buf, "%s%s", word, fread_string(fp) );
	    ed->keyword		= str_dup(buf);
	  }
	  ed->description		= fread_string( fp );
	  ed->next		= pRoomIndex->extra_descr;
	  pRoomIndex->extra_descr	= ed;
	}
	break;

      default :	    
	bug( "Load_room_dsa: vnum %d has unknown flag.", vnum );
	exit( 1 );
      }
    }

    RESET_DATA *pReset;
    int  iLastObj2  = 0;
    OBJ_INDEX_DATA *temp_index;
    bool stop = FALSE;
    for ( ; ; ) {
      word   = feof( fp ) ? "End" : fread_word( fp );
      
      if ( !str_cmp( word, "Reset" ) ) {
	letter = fread_letter( fp );

	if ( !area_last ) {
	  bug( "Load_reset_dsa: no #AREA seen yet.");
	  exit( 1 );
	}
	if ( letter == '*' ) {
	  fread_to_eol( fp );
	  continue;
	}
	pReset = new_reset_data();
	pReset->command	= letter;
	
	pReset->arg1 = fread_number(fp);
	pReset->arg2 = fread_number(fp);
	pReset->arg3 = fread_number(fp); pReset->arg3 = vnum; // arg3 is not used FIXME
	pReset->arg4 = fread_number(fp); pReset->arg4 = pReset->arg2; // arg4 is not used

	fread_to_eol( fp );
	// Validate parameters.
	// We're calling the index functions for the side effect.
	switch ( letter ) {
	default:
	  bug( "Load_reset_dsa: bad command '%c'.", letter );
	  exit( 1 );
	  break;
	  
	case 'M': // Reset M 10308 1 100 0
	  get_mob_index( pReset->arg1 );
	  if ( pReset->arg2 == 0 )
	    bug("reset M has arg2 equals to 0 (room: %d)", vnum );
	  new_reset( pRoomIndex, pReset );
	  break;

	case 'O': // Reset O 32251 1 100 0
	  temp_index = get_obj_index( pReset->arg1 );
	  temp_index->reset_num++; 
	  new_reset( pRoomIndex, pReset );
	  iLastObj2 = pReset->arg1;
	  break;

	case 'P': // Reset P 32218 1 100 0
	  temp_index = get_obj_index( pReset->arg1 );
	  temp_index->reset_num++;
	  pReset->arg3 = iLastObj2;
	  pReset->arg2 = -1;
	  if ( pReset->arg2 == 0 )
	    bug("reset P has arg2 equals to 0 (room: %d)", vnum );
	  new_reset( pRoomIndex, pReset );
	  break;

	case 'G': // Reset G 10307 1 100 0
	case 'E': // Reset E 32244 23 100 0
	  temp_index = get_obj_index( pReset->arg1 );
	  temp_index->reset_num++; 
	  new_reset( pRoomIndex, pReset );
	  pReset->arg3 = pReset->arg2;
	  pReset->arg2 = -1;
	  iLastObj2 = pReset->arg1;
	  break;
	}
      }
      else if ( !str_cmp( word, "End" ) )
	stop = TRUE;
      else
	bug("Unknown extra information [%s] for room [%d]", word, vnum );
      if ( stop )
	break;
    }
    // End

    convert_room_dsa(pRoomIndex);

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum( vnum );                                    /* OLC */
  }

  return;
}

void convert_obj_dsa( OBJ_INDEX_DATA *pObjIndex ) {
  // FIXME: need conversion ?
  //  pObjIndex->extra_flags = extra_flags_dsa( pObjIndex->extra_flags );
  //  pObjIndex->wear_flags = item_wear_flags_dsa( pObjIndex->wear_flags );
}

/*
 * Snarf an obj section.  old style 
 */
void load_obj_dsa( FILE *fp ) {
  OBJ_INDEX_DATA *pObjIndex;
  const char *word;

  if ( !area_last ) {  /* OLC */
    bug( "Load_obj_dsa: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_obj_dsa: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_obj_index( vnum ) != NULL ) {
      bug( "Load_obj_dsa: vnum %d duplicated.", vnum );
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

    word = fread_string(fp);
    int mat = material_lookup(word);
    if ( mat < 0 )
      bug("Invalid material '%s' for object (vnum %d)", 
	  word, vnum );
    pObjIndex->material		    = mat<0 ? 0 : mat;


    pObjIndex->item_type            = item_lookup(fread_word( fp ));
    if ( pObjIndex->item_type < 0 ) {
      bug("Invalid item type for object %d, assuming trash.", pObjIndex->vnum );
      pObjIndex->item_type = ITEM_TRASH;
    }

    pObjIndex->size = SIZE_NOSIZE;

    pObjIndex->extra_flags          = fread_flag( fp );
    pObjIndex->wear_flags           = fread_flag( fp );

    char buffer[512];
    int sn;
    switch(pObjIndex->item_type) {
    case ITEM_WEAPON:
      pObjIndex->value[0]		= weapon_type(fread_word(fp));
      pObjIndex->value[1]		= fread_number(fp);
      pObjIndex->value[2]		= fread_number(fp);
      if ( pObjIndex->value[0] == WEAPON_RANGED ) // SinaC 2003
	pObjIndex->value[3]		= fread_number(fp);
      else
	pObjIndex->value[3]		= attack_lookup(fread_word(fp));
      pObjIndex->value[4]		= fread_flag(fp);
      break;
    case ITEM_CONTAINER:
      pObjIndex->value[0]		= fread_number(fp);
      pObjIndex->value[1]		= fread_flag(fp);
      pObjIndex->value[2]		= fread_number(fp);
      pObjIndex->value[3]		= fread_number(fp);
      pObjIndex->value[4]		= fread_number(fp);
      break;
    case ITEM_DRINK_CON:
    case ITEM_FOUNTAIN:
      pObjIndex->value[0]         = fread_number(fp);
      pObjIndex->value[1]         = fread_number(fp);
      word = fread_word(fp);  /* By Oxtal -- For MZF compatibility */
      if (is_number(word))
	pObjIndex->value[2] = atoi(word);
      else
	pObjIndex->value[2]         = liq_lookup(word);
      pObjIndex->value[3]           = fread_number(fp);
      pObjIndex->value[4]           = fread_flag(fp); /* Oxtal (was number; for MZF) */
      break;
    case ITEM_WAND:
    case ITEM_STAFF:
      pObjIndex->value[0]		= fread_number(fp);
      pObjIndex->value[1]		= fread_number(fp);
      pObjIndex->value[2]		= fread_number(fp);
      //pObjIndex->value[3]		= skill_lookup(fread_word(fp));
      strcpy(buffer, fread_word(fp));
      if ( ( sn = ability_lookup(buffer) ) < 0
	   && buffer[0]!='\0') {
	bug("Invalid value3 '%s' (sn: %d) for object %s (vnum %d)",
	    buffer, sn, pObjIndex->short_descr, vnum );
	sn = 0;
      }
      if ( sn > 0 && ability_table[sn].type != TYPE_SPELL
	   && ability_table[sn].type != TYPE_POWER )
	log_stringf("Value3 '%s' (sn: %d) for object %s (vnum %d) is not a spell/power", 
		    buffer, sn, pObjIndex->short_descr, vnum );
      pObjIndex->value[3]               = sn;
      pObjIndex->value[4]		= fread_number(fp);
      break;
    case ITEM_POTION:
    case ITEM_PILL:
    case ITEM_SCROLL:
    // Added by SinaC 2003
    case ITEM_TEMPLATE:
      pObjIndex->value[0]		= fread_number(fp);
      strcpy(buffer, fread_word(fp));
      if ( ( sn = ability_lookup(buffer) ) < 0 
	   && buffer[0]!='\0') {
	bug("Invalid value1 '%s' (sn: %d) for object %s (vnum %d)",
	    buffer, sn, pObjIndex->short_descr, vnum );
	sn = 0;
      }
      if ( sn > 0 && ability_table[sn].type != TYPE_SPELL
	   && ability_table[sn].type != TYPE_POWER )
	log_stringf("Value1 '%s' (sn: %d) for object %s (vnum %d) is not a spell/power", 
		    buffer, sn, pObjIndex->short_descr, vnum );
      pObjIndex->value[1]		= sn;
      strcpy(buffer, fread_word(fp));
      if ( ( sn = ability_lookup(buffer) ) < 0 
	   && buffer[0]!='\0') {
	bug("Invalid value2 '%s' (sn: %d) for object %s (vnum %d)",
	    buffer, sn, pObjIndex->short_descr, vnum );
	sn = 0;
      }
      if ( sn > 0 && ability_table[sn].type != TYPE_SPELL
	   && ability_table[sn].type != TYPE_POWER )
	log_stringf("Value2 '%s' (sn: %d) for object %s (vnum %d) is not a spell/power",
		    buffer, sn, pObjIndex->short_descr, vnum );
      pObjIndex->value[2]		= sn;
      strcpy(buffer, fread_word(fp));
      if ( ( sn = ability_lookup(buffer) ) < 0 
	   && buffer[0]!='\0') {
	bug("Invalid value3 '%s' (sn: %d) for object %s (vnum %d)",
	    buffer, sn, pObjIndex->short_descr, vnum );
	sn = 0;
      }
      if ( sn > 0 && ability_table[sn].type != TYPE_SPELL
	   && ability_table[sn].type != TYPE_POWER )
	log_stringf("Value3 '%s' (sn: %d) for object %s (vnum %d) is not a spell/power",
		    buffer, sn, pObjIndex->short_descr, vnum );
      pObjIndex->value[3]		= sn;
      strcpy(buffer, fread_word(fp));
      if ( ( sn = ability_lookup(buffer) ) < 0 
	   && buffer[0]!='\0') {
	bug("Invalid value4 '%s' (sn: %d) for object %s (vnum %d)",
	    buffer, sn, pObjIndex->short_descr, vnum );
	sn = 0;
      }
      if ( sn > 0 && ability_table[sn].type != TYPE_SPELL
	   && ability_table[sn].type != TYPE_POWER )
	log_stringf("Value4 '%s' (sn: %d) for object %s (vnum %d) is not a spell/power",
		    buffer, sn, pObjIndex->short_descr, vnum );
      pObjIndex->value[4]		= sn;
      break;
    default:
      pObjIndex->value[0]             = fread_flag( fp );
      pObjIndex->value[1]             = fread_flag( fp );
      pObjIndex->value[2]             = fread_flag( fp );
      pObjIndex->value[3]             = fread_flag( fp );
      pObjIndex->value[4]  	      = fread_flag( fp );
      break;
    }
    pObjIndex->level		    = fread_number( fp );
    pObjIndex->weight               = fread_number( fp );
    pObjIndex->cost                 = fread_number( fp ); 
    letter 				= fread_letter( fp );
    switch (letter)	{
    case ('P') :		pObjIndex->condition = 100; break;
    case ('G') :		pObjIndex->condition =  90; break;
    case ('A') :		pObjIndex->condition =  75; break;
    case ('W') :		pObjIndex->condition =  50; break;
    case ('D') :		pObjIndex->condition =  25; break;
    case ('B') :		pObjIndex->condition =  10; break;
    case ('R') :		pObjIndex->condition =   0; break;
    default:			pObjIndex->condition = 100; break;
    }

    for ( ; ; ) {
      letter = fread_letter( fp );

      if ( letter == 'A' ) {
	AFFECT_DATA *paf;
	
	paf = new_affect();
	createaff(*paf,-1,20,-1,0,AFFECT_INHERENT);
	addaff2(*paf,AFTO_CHAR,fread_number( fp ),AFOP_ADD,fread_number( fp ));

	convert_af_dsa( paf );
	
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

    fread_string(fp); // skips ~

    bool stop = FALSE;
    for ( ; ; ) {
      word   = feof( fp ) ? "End" : fread_word( fp );
      if (!str_cmp( word, "Spec" ) )
	fread_string(fp);
      else if (!str_cmp( word, "MaxWorld" ) )
	fread_number(fp);
      else if (!str_cmp( word, "End" ) )
	stop = TRUE;
      else
	bug("Unknown extra information [%s] for object [%d]",
	    word, vnum );
      if ( stop )
	break;
    }

    convert_obj_dsa( pObjIndex );

    iHash			= vnum % MAX_KEY_HASH;
    pObjIndex->next		= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObjIndex;
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
    assign_area_vnum( vnum );                                   /* OLC */
  }

  return;
}

void load_area_dsa( FILE *fp ) {
  AREA_DATA *pArea;
  const char      *word;
  bool      fMatch;

  pArea = new_area();
  pArea->age          = 15;
  pArea->nplayer      = 0;
  pArea->totalnplayer = 0;
  pArea->file_name    = str_dup( strArea );
  pArea->vnum         = top_area;
  pArea->name         = str_dup( "New Area" );
  pArea->builders     = str_dup( "" );
  pArea->security     = 9;
  pArea->min_vnum     = 0;
  pArea->max_vnum     = 0;
  pArea->area_flags   = 0;
  pArea->earthquake_on = FALSE;
  pArea->earthquake_duration = 0;
  pArea->low_range        = 500;
  pArea->high_range       = 0;

  fread_string(fp); // skips DSA format
  fread_string(fp); // skips filename
  pArea->name = fread_string(fp); // name
  pArea->credits = fread_string(fp); // credits
  pArea->min_vnum = fread_number(fp); // min vnum
  pArea->max_vnum = fread_number(fp); // max vnum

  for ( ; ; ) {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;
    if (!str_cmp( word, "Flags" ) )
      pArea->area_flags = fread_flag(fp);
    else if (!str_cmp( word, "Sec" ) )
      pArea->security = fread_number( fp );
    else if ( !str_cmp( word, "Levels" ) ) {
      pArea->low_range = fread_number(fp);
      pArea->high_range = fread_number(fp);
    }
    else if ( !str_cmp( word, "End" ) ) {
      if ( area_first == NULL )
	area_first = pArea;
      if ( area_last ) { // Modified by SinaC 2003, seems to crash without that
	area_last->next = pArea;
	REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
      }
      area_last   = pArea;
      pArea->next = NULL;
      return;
    }
    else if ( !str_cmp( word, "Bldrs" ) )
      pArea->builders = fread_string(fp);
  }
}
