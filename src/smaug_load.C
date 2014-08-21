// This file has been written by SinaC 2001 to be able to read Smaug area file

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
#include "recycle.h"
//#include "olc.h"
#include "mem.h"



// Constants from Smaug
typedef enum
{
  SMAUG_SECT_INSIDE, 
  SMAUG_SECT_CITY, 
  SMAUG_SECT_FIELD, 
  SMAUG_SECT_FOREST, 
  SMAUG_SECT_HILLS, 
  SMAUG_SECT_MOUNTAIN,
  SMAUG_SECT_WATER_SWIM, 
  SMAUG_SECT_WATER_NOSWIM, 
  SMAUG_SECT_UNDERWATER, 
  SMAUG_SECT_AIR, 
  SMAUG_SECT_DESERT,
  SMAUG_SECT_DUNNO, 
  SMAUG_SECT_OCEANFLOOR, 
  SMAUG_SECT_UNDERGROUND, 
  SMAUG_SECT_LAVA, 
  SMAUG_SECT_SWAMP,
  SMAUG_SECT_MAX
} smaug_sector_types;

typedef enum
{
  SMAUG_WEAR_NONE = -1, 
  SMAUG_WEAR_LIGHT = 0, 
  SMAUG_WEAR_FINGER_L, 
  SMAUG_WEAR_FINGER_R, 
  SMAUG_WEAR_NECK_1,
  SMAUG_WEAR_NECK_2, 
  SMAUG_WEAR_BODY, 
  SMAUG_WEAR_HEAD, 
  SMAUG_WEAR_LEGS, 
  SMAUG_WEAR_FEET, 
  SMAUG_WEAR_HANDS,
  SMAUG_WEAR_ARMS, 
  SMAUG_WEAR_SHIELD, 
  SMAUG_WEAR_ABOUT, 
  SMAUG_WEAR_WAIST, 
  SMAUG_WEAR_WRIST_L, 
  SMAUG_WEAR_WRIST_R,
  SMAUG_WEAR_WIELD, 
  SMAUG_WEAR_HOLD, 
  SMAUG_WEAR_DUAL_WIELD, 
  SMAUG_WEAR_EARS, 
  SMAUG_WEAR_EYES,
  SMAUG_WEAR_MISSILE_WIELD, 
  SMAUG_WEAR_BACK, 
  SMAUG_WEAR_FACE, 
  SMAUG_WEAR_ANKLE_L, 
  SMAUG_WEAR_ANKLE_R,
  SMAUG_MAX_WEAR
} smaug_wear_type;

typedef enum
{
  SMAUG_POS_DEAD, 
  SMAUG_POS_MORTAL, 
  SMAUG_POS_INCAP, 
  SMAUG_POS_STUNNED, 
  SMAUG_POS_SLEEPING, 
  SMAUG_POS_BERSERK,
  SMAUG_POS_RESTING, 
  SMAUG_POS_AGGRESSIVE, 
  SMAUG_POS_SITTING, 
  SMAUG_POS_FIGHTING, 
  SMAUG_POS_DEFENSIVE,
  SMAUG_POS_EVASIVE, 
  SMAUG_POS_STANDING, 
  SMAUG_POS_MOUNTED, 
  SMAUG_POS_SHOVE, 
  SMAUG_POS_DRAG
} smaug_positions;


typedef enum
{
  SMAUG_APPLY_NONE, 
  SMAUG_APPLY_STR, 
  SMAUG_APPLY_DEX, 
  SMAUG_APPLY_INT, 
  SMAUG_APPLY_WIS, 
  SMAUG_APPLY_CON,
  SMAUG_APPLY_SEX, 
  SMAUG_APPLY_CLASS, 
  SMAUG_APPLY_LEVEL, 
  SMAUG_APPLY_AGE, 
  SMAUG_APPLY_HEIGHT, 
  SMAUG_APPLY_WEIGHT,
  SMAUG_APPLY_MANA, 
  SMAUG_APPLY_HIT, 
  SMAUG_APPLY_MOVE, 
  SMAUG_APPLY_GOLD, 
  SMAUG_APPLY_EXP, 
  SMAUG_APPLY_AC,
  SMAUG_APPLY_HITROLL, 
  SMAUG_APPLY_DAMROLL, 
  SMAUG_APPLY_SAVING_POISON, 
  SMAUG_APPLY_SAVING_ROD,
  SMAUG_APPLY_SAVING_PARA, 
  SMAUG_APPLY_SAVING_BREATH, 
  SMAUG_APPLY_SAVING_SPELL, 
  SMAUG_APPLY_CHA,
  SMAUG_APPLY_AFFECT, 
  SMAUG_APPLY_RESISTANT, 
  SMAUG_APPLY_IMMUNE, 
  SMAUG_APPLY_SUSCEPTIBLE,
  SMAUG_APPLY_WEAPONSPELL, 
  SMAUG_APPLY_LCK, 
  SMAUG_APPLY_BACKSTAB, 
  SMAUG_APPLY_PICK, 
  SMAUG_APPLY_TRACK,
  SMAUG_APPLY_STEAL, 
  SMAUG_APPLY_SNEAK, 
  SMAUG_APPLY_HIDE, 
  SMAUG_APPLY_PALM, 
  SMAUG_APPLY_DETRAP, 
  SMAUG_APPLY_DODGE,
  SMAUG_APPLY_PEEK, 
  SMAUG_APPLY_SCAN, 
  SMAUG_APPLY_GOUGE, 
  SMAUG_APPLY_SEARCH, 
  SMAUG_APPLY_MOUNT, 
  SMAUG_APPLY_DISARM,
  SMAUG_APPLY_KICK, 
  SMAUG_APPLY_PARRY, 
  SMAUG_APPLY_BASH, 
  SMAUG_APPLY_STUN, 
  SMAUG_APPLY_PUNCH, 
  SMAUG_APPLY_CLIMB,
  SMAUG_APPLY_GRIP, 
  SMAUG_APPLY_SCRIBE, 
  SMAUG_APPLY_BREW, 
  SMAUG_APPLY_WEARSPELL, 
  SMAUG_APPLY_REMOVESPELL,
  SMAUG_APPLY_EMOTION, 
  SMAUG_APPLY_MENTALSTATE, 
  SMAUG_APPLY_STRIPSN, 
  SMAUG_APPLY_REMOVE, 
  SMAUG_APPLY_DIG,
  SMAUG_APPLY_FULL, 
  SMAUG_APPLY_THIRST, 
  SMAUG_APPLY_DRUNK, 
  SMAUG_APPLY_BLOOD, 
  SMAUG_APPLY_COOK,
  SMAUG_APPLY_RECURRINGSPELL, 
  SMAUG_APPLY_CONTAGIOUS, 
  SMAUG_APPLY_EXT_AFFECT, 
  SMAUG_APPLY_ODOR,
  SMAUG_APPLY_ROOMFLAG, 
  SMAUG_APPLY_SECTORTYPE, 
  SMAUG_APPLY_ROOMLIGHT, 
  SMAUG_APPLY_TELEVNUM,
  SMAUG_APPLY_TELEDELAY, 
  SMAUG_MAX_APPLY_TYPE
} smaug_apply_types;

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

#define SMAUG_EX_ISDOOR		  BV00
#define SMAUG_EX_CLOSED		  BV01
#define SMAUG_EX_LOCKED		  BV02
#define SMAUG_EX_SECRET		  BV03
#define SMAUG_EX_SWIM	       	  BV04
#define SMAUG_EX_PICKPROOF     	  BV05
#define SMAUG_EX_FLY	       	  BV06
#define SMAUG_EX_CLIMB		  BV07
#define SMAUG_EX_DIG	       	  BV08
#define SMAUG_EX_EATKEY		  BV09
#define SMAUG_EX_NOPASSDOOR    	  BV10
#define SMAUG_EX_HIDDEN		  BV11
#define SMAUG_EX_PASSAGE       	  BV12
#define SMAUG_EX_PORTAL        	  BV13
#define SMAUG_EX_RES1	     	  BV14
#define SMAUG_EX_RES2		  BV15
#define SMAUG_EX_xCLIMB		  BV16
#define SMAUG_EX_xENTER		  BV17
#define SMAUG_EX_xLEAVE		  BV18
#define SMAUG_EX_xAUTO		  BV19
#define SMAUG_EX_NOFLEE	  	  BV20
#define SMAUG_EX_xSEARCHABLE	  BV21
#define SMAUG_EX_BASHED           BV22
#define SMAUG_EX_BASHPROOF        BV23
#define SMAUG_EX_NOMOB		  BV24
#define SMAUG_EX_WINDOW		  BV25
#define SMAUG_EX_xLOOK		  BV26
#define SMAUG_EX_ISBOLT		  BV27
#define SMAUG_EX_BOLTED		  BV28
#define MAX_EXFLAG		  28

#define SMAUG_ROOM_DARK		BV00
#define SMAUG_ROOM_DEATH		BV01
#define SMAUG_ROOM_NO_MOB		BV02
#define SMAUG_ROOM_INDOORS		BV03
#define SMAUG_ROOM_LAWFUL		BV04
#define SMAUG_ROOM_NEUTRAL		BV05
#define SMAUG_ROOM_CHAOTIC		BV06
#define SMAUG_ROOM_NO_MAGIC		BV07
#define SMAUG_ROOM_TUNNEL		BV08
#define SMAUG_ROOM_PRIVATE		BV09
#define SMAUG_ROOM_SAFE		BV10
#define SMAUG_ROOM_SOLITARY		BV11
#define SMAUG_ROOM_PET_SHOP		BV12
#define SMAUG_ROOM_NO_RECALL		BV13
#define SMAUG_ROOM_DONATION		BV14
#define SMAUG_ROOM_NODROPALL		BV15
#define SMAUG_ROOM_SILENCE		BV16
#define SMAUG_ROOM_LOGSPEECH		BV17
#define SMAUG_ROOM_NODROP		BV18
#define SMAUG_ROOM_CLANSTOREROOM	BV19
#define SMAUG_ROOM_NO_SUMMON		BV20
#define SMAUG_ROOM_NO_ASTRAL		BV21
#define SMAUG_ROOM_TELEPORT		BV22
#define SMAUG_ROOM_TELESHOWDESC	BV23
#define SMAUG_ROOM_NOFLOOR		BV24
#define SMAUG_ROOM_NOSUPPLICATE       BV25
#define SMAUG_ROOM_ARENA		BV26
#define SMAUG_ROOM_NOMISSILE		BV27
#define SMAUG_ROOM_PROTOTYPE	     	BV30
#define SMAUG_ROOM_DND	     	BV31

typedef enum
{
  SMAUG_ITEM_GLOW, 
  SMAUG_ITEM_HUM, 
  SMAUG_ITEM_DARK,
  SMAUG_ITEM_LOYAL, 
  SMAUG_ITEM_EVIL, 
  SMAUG_ITEM_INVIS, 
  SMAUG_ITEM_MAGIC, 
  SMAUG_ITEM_NODROP, 
  SMAUG_ITEM_BLESS, 
  SMAUG_ITEM_ANTI_GOOD, 
  SMAUG_ITEM_ANTI_EVIL, 
  SMAUG_ITEM_ANTI_NEUTRAL, 
  SMAUG_ITEM_NOREMOVE, 
  SMAUG_ITEM_INVENTORY, 
  SMAUG_ITEM_ANTI_MAGE, 
  SMAUG_ITEM_ANTI_THIEF, 
  SMAUG_ITEM_ANTI_WARRIOR, 
  SMAUG_ITEM_ANTI_CLERIC, 
  SMAUG_ITEM_ORGANIC, 
  SMAUG_ITEM_METAL, 
  SMAUG_ITEM_DONATION, 
  SMAUG_ITEM_CLANOBJECT, 
  SMAUG_ITEM_CLANCORPSE, 
  SMAUG_ITEM_ANTI_VAMPIRE, 
  SMAUG_ITEM_ANTI_DRUID, 
  SMAUG_ITEM_HIDDEN, 
  SMAUG_ITEM_POISONED, 
  SMAUG_ITEM_COVERING, 
  SMAUG_ITEM_DEATHROT, 
  SMAUG_ITEM_BURIED, 
  SMAUG_ITEM_PROTOTYPE, 
  SMAUG_ITEM_NOLOCATE, 
  SMAUG_ITEM_GROUNDROT, 
  SMAUG_ITEM_LOOTABLE, 
  MAX_SMAUG_ITEM_FLAG
} smaug_item_extra_flags;

typedef enum
{
  SMAUG_ITEM_NONE, 
  SMAUG_ITEM_LIGHT, 
  SMAUG_ITEM_SCROLL, 
  SMAUG_ITEM_WAND, 
  SMAUG_ITEM_STAFF, 
  SMAUG_ITEM_WEAPON,
  SMAUG_ITEM_FIREWEAPON, 
  SMAUG_ITEM_MISSILE, 
  SMAUG_ITEM_TREASURE, 
  SMAUG_ITEM_ARMOR, 
  SMAUG_ITEM_POTION,
  SMAUG_ITEM_WORN, 
  SMAUG_ITEM_FURNITURE, 
  SMAUG_ITEM_TRASH, 
  SMAUG_ITEM_OLDTRAP, 
  SMAUG_ITEM_CONTAINER,
  SMAUG_ITEM_NOTE, 
  SMAUG_ITEM_DRINK_CON, 
  SMAUG_ITEM_KEY, 
  SMAUG_ITEM_FOOD, 
  SMAUG_ITEM_MONEY, 
  SMAUG_ITEM_PEN,
  SMAUG_ITEM_BOAT, 
  SMAUG_ITEM_CORPSE_NPC, 
  SMAUG_ITEM_CORPSE_PC, 
  SMAUG_ITEM_FOUNTAIN, 
  SMAUG_ITEM_PILL,
  SMAUG_ITEM_BLOOD, 
  SMAUG_ITEM_BLOODSTAIN, 
  SMAUG_ITEM_SCRAPS, 
  SMAUG_ITEM_PIPE, 
  SMAUG_ITEM_HERB_CON,
  SMAUG_ITEM_HERB, 
  SMAUG_ITEM_INCENSE, 
  SMAUG_ITEM_FIRE, 
  SMAUG_ITEM_BOOK, 
  SMAUG_ITEM_SWITCH, 
  SMAUG_ITEM_LEVER,
  SMAUG_ITEM_PULLCHAIN, 
  SMAUG_ITEM_BUTTON, 
  SMAUG_ITEM_DIAL, 
  SMAUG_ITEM_RUNE, 
  SMAUG_ITEM_RUNEPOUCH,
  SMAUG_ITEM_MATCH, 
  SMAUG_ITEM_TRAP, 
  SMAUG_ITEM_MAP, 
  SMAUG_ITEM_PORTAL, 
  SMAUG_ITEM_PAPER,
  SMAUG_ITEM_TINDER, 
  SMAUG_ITEM_LOCKPICK, 
  SMAUG_ITEM_SPIKE, 
  SMAUG_ITEM_DISEASE, 
  SMAUG_ITEM_OIL, 
  SMAUG_ITEM_FUEL,
  SMAUG_ITEM_EMPTY1, 
  SMAUG_ITEM_EMPTY2, 
  SMAUG_ITEM_MISSILE_WEAPON, 
  SMAUG_ITEM_PROJECTILE, 
  SMAUG_ITEM_QUIVER,
  SMAUG_ITEM_SHOVEL, 
  SMAUG_ITEM_SALVE, 
  SMAUG_ITEM_COOK, 
  SMAUG_ITEM_KEYRING, 
  SMAUG_ITEM_ODOR, 
  SMAUG_ITEM_CHANCE
} item_types;


#define SMAUG_PART_HEAD		  BV00
#define SMAUG_PART_ARMS		  BV01
#define SMAUG_PART_LEGS		  BV02
#define SMAUG_PART_HEART		  BV03
#define SMAUG_PART_BRAINS		  BV04
#define SMAUG_PART_GUTS		  BV05
#define SMAUG_PART_HANDS		  BV06
#define SMAUG_PART_FEET		  BV07
#define SMAUG_PART_FINGERS		  BV08
#define SMAUG_PART_EAR		  BV09
#define SMAUG_PART_EYE		  BV10
#define SMAUG_PART_LONG_TONGUE	  BV11
#define SMAUG_PART_EYESTALKS		  BV12
#define SMAUG_PART_TENTACLES		  BV13
#define SMAUG_PART_FINS		  BV14
#define SMAUG_PART_WINGS		  BV15
#define SMAUG_PART_TAIL		  BV16
#define SMAUG_PART_SCALES		  BV17
// for combat
#define SMAUG_PART_CLAWS		  BV18
#define SMAUG_PART_FANGS		  BV19
#define SMAUG_PART_HORNS		  BV20
#define SMAUG_PART_TUSKS		  BV21
#define SMAUG_PART_TAILATTACK		  BV22
#define SMAUG_PART_SHARPSCALES	  BV23
#define SMAUG_PART_BEAK		  BV24

#define SMAUG_PART_HAUNCH		  BV25
#define SMAUG_PART_HOOVES		  BV26
#define SMAUG_PART_PAWS		  BV27
#define SMAUG_PART_FORELEGS		  BV28
#define SMAUG_PART_FEATHERS		  BV29


typedef enum
{
  ATCK_BITE, 
  ATCK_CLAWS, 
  ATCK_TAIL, 
  ATCK_STING, 
  ATCK_PUNCH, 
  ATCK_KICK,
  ATCK_TRIP, 
  ATCK_BASH, 
  ATCK_STUN, 
  ATCK_GOUGE, 
  ATCK_BACKSTAB, 
  ATCK_FEED,
  ATCK_DRAIN,  
  ATCK_FIREBREATH, 
  ATCK_FROSTBREATH, 
  ATCK_ACIDBREATH,
  ATCK_LIGHTNBREATH, 
  ATCK_GASBREATH, 
  ATCK_POISON, 
  ATCK_NASTYPOISON, 
  ATCK_GAZE,
  ATCK_BLINDNESS, 
  ATCK_CAUSESERIOUS, 
  ATCK_EARTHQUAKE, 
  ATCK_CAUSECRITICAL,
  ATCK_CURSE, 
  ATCK_FLAMESTRIKE, 
  ATCK_HARM, 
  ATCK_FIREBALL, 
  ATCK_COLORSPRAY,
  ATCK_WEAKEN, 
  ATCK_SPIRALBLAST, 
  MAX_ATTACK_TYPE
} attack_types;


//Resistant Immune Susceptible flags
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21


typedef enum
{
  SMAUG_AFF_BLIND, 
  SMAUG_AFF_INVISIBLE, 
  SMAUG_AFF_DETECT_EVIL, 
  SMAUG_AFF_DETECT_INVIS, 
  SMAUG_AFF_DETECT_MAGIC, 
  SMAUG_AFF_DETECT_HIDDEN, 
  SMAUG_AFF_HOLD, 
  SMAUG_AFF_SANCTUARY, 
  SMAUG_AFF_FAERIE_FIRE, 
  SMAUG_AFF_INFRARED, 
  SMAUG_AFF_CURSE, 
  SMAUG_AFF_FLAMING, 
  SMAUG_AFF_POISON, 
  SMAUG_AFF_PROTECT, 
  SMAUG_AFF_PARALYSIS, 
  SMAUG_AFF_SNEAK, 
  SMAUG_AFF_HIDE, 
  SMAUG_AFF_SLEEP, 
  SMAUG_AFF_CHARM, 
  SMAUG_AFF_FLYING, 
  SMAUG_AFF_PASS_DOOR, 
  SMAUG_AFF_FLOATING, 
  SMAUG_AFF_TRUESIGHT, 
  SMAUG_AFF_DETECTTRAPS, 
  SMAUG_AFF_SCRYING, 
  SMAUG_AFF_FIRESHIELD, 
  SMAUG_AFF_SHOCKSHIELD, 
  SMAUG_AFF_HAUS1, 
  SMAUG_AFF_ICESHIELD, 
  SMAUG_AFF_POSSESS, 
  SMAUG_AFF_BERSERK, 
  SMAUG_AFF_AQUA_BREATH, 
  SMAUG_AFF_RECURRINGSPELL,
  SMAUG_AFF_CONTAGIOUS, 
  SMAUG_AFF_ACIDMIST,  
  SMAUG_AFF_VENOMSHIELD, 
  MAX_AFFECTED_BY
} affected_by_types;


#define SMAUG_ACT_IS_NPC		  0		// Auto set for mobs	
#define SMAUG_ACT_SENTINEL		  1		// Stays in one room	
#define SMAUG_ACT_SCAVENGER		  2		// Picks up objects	
#define SMAUG_ACT_AGGRESSIVE		  5		// Attacks PC's		
#define SMAUG_ACT_STAY_AREA		  6		// Won't leave area	
#define SMAUG_ACT_WIMPY		  7		// Flees when hurt	
#define SMAUG_ACT_PET			  8		// Auto set for pets	
#define SMAUG_ACT_TRAIN		  9		// Can train PC's	
#define SMAUG_ACT_PRACTICE		 10		// Can practice PC's	
#define SMAUG_ACT_IMMORTAL		 11		// Cannot be killed	
#define SMAUG_ACT_DEADLY		 12		// Has a deadly poison  
#define SMAUG_ACT_POLYSELF		 13
#define SMAUG_ACT_META_AGGR		 14		// Attacks other mobs	
#define SMAUG_ACT_GUARDIAN		 15		// Protects master	
#define SMAUG_ACT_RUNNING		 16		// Hunts quickly	
#define SMAUG_ACT_NOWANDER		 17		// Doesn't wander	
#define SMAUG_ACT_MOUNTABLE		 18		// Can be mounted      
#define SMAUG_ACT_MOUNTED		 19		// Is mounted		
#define SMAUG_ACT_SCHOLAR              20		// Can teach languages  
#define SMAUG_ACT_SECRETIVE		 21		// actions aren't seen	
#define SMAUG_ACT_HARDHAT	         22		// Immune to falling item damage 
#define SMAUG_ACT_MOBINVIS		 23		// Like wizinvis	
#define SMAUG_ACT_NOASSIST		 24		// Doesn't assist mobs	
#define SMAUG_ACT_AUTONOMOUS		 25		// Doesn't auto switch tanks 
#define SMAUG_ACT_PACIFIST             26		// Doesn't ever fight   
#define SMAUG_ACT_NOATTACK		 27		// No physical attacks 
#define SMAUG_ACT_ANNOYING		 28		// Other mobs will attack 
#define SMAUG_ACT_STATSHIELD		 29		// prevent statting 
#define SMAUG_ACT_PROTOTYPE		 30		// A prototype mob	


#define SMAUG_ITEM_TAKE		        BV00
#define SMAUG_ITEM_WEAR_FINGER	        BV01
#define SMAUG_ITEM_WEAR_NECK		BV02
#define SMAUG_ITEM_WEAR_BODY		BV03
#define SMAUG_ITEM_WEAR_HEAD		BV04
#define SMAUG_ITEM_WEAR_LEGS		BV05
#define SMAUG_ITEM_WEAR_FEET		BV06
#define SMAUG_ITEM_WEAR_HANDS		BV07
#define SMAUG_ITEM_WEAR_ARMS		BV08
#define SMAUG_ITEM_WEAR_SHIELD	        BV09
#define SMAUG_ITEM_WEAR_ABOUT		BV10
#define SMAUG_ITEM_WEAR_WAIST		BV11
#define SMAUG_ITEM_WEAR_WRIST		BV12
#define SMAUG_ITEM_WIELD		BV13
#define SMAUG_ITEM_HOLD		        BV14
#define SMAUG_ITEM_DUAL_WIELD		BV15
#define SMAUG_ITEM_WEAR_EARS		BV16
#define SMAUG_ITEM_WEAR_EYES		BV17
#define SMAUG_ITEM_MISSILE_WIELD	BV18
#define SMAUG_ITEM_WEAR_BACK		BV19
#define SMAUG_ITEM_WEAR_FACE		BV20
#define SMAUG_ITEM_WEAR_ANKLE		BV21
#define SMAUG_ITEM_WEAR_MAX		21


#define SFT(n)  (1L<<(n))
int aff_smaug( int aff ) {
  int new_aff = 0;

  if ( aff & SFT(SMAUG_AFF_BLIND) )         SET_BIT( new_aff, AFF_BLIND );
  if ( aff & SFT(SMAUG_AFF_INVISIBLE) )     SET_BIT( new_aff, AFF_INVISIBLE );
  if ( aff & SFT(SMAUG_AFF_DETECT_EVIL) )   SET_BIT( new_aff, AFF_DETECT_EVIL );
  if ( aff & SFT(SMAUG_AFF_DETECT_INVIS) )  SET_BIT( new_aff, AFF_DETECT_INVIS );
  if ( aff & SFT(SMAUG_AFF_DETECT_MAGIC) )  SET_BIT( new_aff, AFF_DETECT_MAGIC );
  if ( aff & SFT(SMAUG_AFF_DETECT_HIDDEN) ) SET_BIT( new_aff, AFF_DETECT_HIDDEN );
  if ( aff & SFT(SMAUG_AFF_SANCTUARY) )     SET_BIT( new_aff, AFF_SANCTUARY );
  if ( aff & SFT(SMAUG_AFF_FAERIE_FIRE) )   SET_BIT( new_aff, AFF_FAERIE_FIRE );
  if ( aff & SFT(SMAUG_AFF_INFRARED) )      SET_BIT( new_aff, AFF_INFRARED );
  if ( aff & SFT(SMAUG_AFF_CURSE) )         SET_BIT( new_aff, AFF_CURSE );
  if ( aff & SFT(SMAUG_AFF_POISON) )        SET_BIT( new_aff, AFF_POISON );
  if ( aff & SFT(SMAUG_AFF_SNEAK) )         SET_BIT( new_aff, AFF_SNEAK );
  if ( aff & SFT(SMAUG_AFF_HIDE) )          SET_BIT( new_aff, AFF_HIDE );
  if ( aff & SFT(SMAUG_AFF_SLEEP) )         SET_BIT( new_aff, AFF_SLEEP );
  if ( aff & SFT(SMAUG_AFF_CHARM) )         SET_BIT( new_aff, AFF_CHARM );
  if ( aff & SFT(SMAUG_AFF_FLYING) )        SET_BIT( new_aff, AFF_FLYING );
  if ( aff & SFT(SMAUG_AFF_PASS_DOOR) )     SET_BIT( new_aff, AFF_PASS_DOOR );
  if ( aff & SFT(SMAUG_AFF_SLEEP) )         SET_BIT( new_aff, AFF_SLEEP );
  if ( aff & SFT(SMAUG_AFF_BERSERK) )       SET_BIT( new_aff, AFF_BERSERK );

  return new_aff;
}

int aff2_smaug( int aff ) {
  int new_aff = 0;

  if ( aff & SFT(SMAUG_AFF_DETECTTRAPS) )   SET_BIT( new_aff, AFF2_DETECT_EXITS );
  if ( aff & SFT(SMAUG_AFF_AQUA_BREATH) )   SET_BIT( new_aff, AFF2_WATER_BREATH );

  return new_aff;
}

int ris_smaug( int ris ) {
  int new_ris = 0;
  // IMM_   RES_   VULN_    have the same values

  if ( ris & RIS_FIRE )        SET_BIT( new_ris, IRV_FIRE );
  if ( ris & RIS_COLD )        SET_BIT( new_ris, IRV_COLD );
  if ( ris & RIS_ELECTRICITY ) SET_BIT( new_ris, IRV_LIGHTNING );
  if ( ris & RIS_ENERGY )      SET_BIT( new_ris, IRV_ENERGY );
  if ( ris & RIS_BLUNT )       SET_BIT( new_ris, IRV_BASH );
  if ( ris & RIS_PIERCE )      SET_BIT( new_ris, IRV_PIERCE );
  if ( ris & RIS_SLASH )       SET_BIT( new_ris, IRV_SLASH );
  if ( ris & RIS_ACID )        SET_BIT( new_ris, IRV_ACID );
  if ( ris & RIS_POISON )      SET_BIT( new_ris, IRV_POISON );
  if ( ris & RIS_DRAIN )       SET_BIT( new_ris, IRV_NEGATIVE );
  if ( ris & RIS_CHARM )       SET_BIT( new_ris, IRV_CHARM );
  if ( ris & RIS_MAGIC )       SET_BIT( new_ris, IRV_MAGIC );
  if ( ris & RIS_NONMAGIC )    SET_BIT( new_ris, IRV_WEAPON );

  if ( ris & RIS_SLEEP )       SET_BIT( new_ris, IRV_CHARM );
  if ( ris & RIS_PARALYSIS )   SET_BIT( new_ris, IRV_PARALYSIS );

  return new_ris;
}

// I know this is crappy
void convert_af_smaug( AFFECT_DATA *af ) {
  int aff, aff2;
  for ( AFFECT_LIST *laf = af->list; laf != NULL; laf = laf->next ) {
    switch ( laf->location ) {
    default:
      laf->location = ATTR_NA; 
      laf->modifier = 0;
      break;
    case SMAUG_APPLY_STR: case SMAUG_APPLY_DEX: case SMAUG_APPLY_INT: 
    case SMAUG_APPLY_WIS: case SMAUG_APPLY_CON:
      laf->location = ATTR_STR + ( laf->location - SMAUG_APPLY_STR ); break;
    case SMAUG_APPLY_SEX:
      laf->location = ATTR_sex; break;
    case SMAUG_APPLY_MANA:
      laf->location = ATTR_max_mana; break;
    case SMAUG_APPLY_HIT:
      laf->location = ATTR_max_hit; break;
    case SMAUG_APPLY_MOVE:
      laf->location = ATTR_max_move; break;
    case SMAUG_APPLY_AC:
      laf->location = ATTR_allAC; break;
    case SMAUG_APPLY_HITROLL:
      laf->location = ATTR_hitroll; break;
    case SMAUG_APPLY_DAMROLL:
      laf->location = ATTR_damroll; break;
    case SMAUG_APPLY_SAVING_POISON: case SMAUG_APPLY_SAVING_ROD: 
    case SMAUG_APPLY_SAVING_PARA: case SMAUG_APPLY_SAVING_BREATH:
    case SMAUG_APPLY_SAVING_SPELL:
      laf->location = ATTR_saving_throw; break;
    case SMAUG_APPLY_AFFECT:
      aff = aff_smaug(laf->modifier);
      aff2 = aff_smaug(laf->modifier);
      laf->op = AFOP_OR;
      if ( aff != 0 ) {
	laf->location = ATTR_affected_by;
	laf->modifier = aff;
      }
      else if ( aff2 != 0 ) {
	laf->location = ATTR_affected2_by;
	laf->modifier = aff2;
      }
      break;
    case SMAUG_APPLY_RESISTANT: case SMAUG_APPLY_IMMUNE: case SMAUG_APPLY_SUSCEPTIBLE:
      laf->op = AFOP_OR;
      laf->modifier = ris_smaug(laf->modifier);
      break;
    }
  }
}

int item_type_smaug( int it ) {
  if ( it == SMAUG_ITEM_LIGHT      ) return ITEM_LIGHT;
  if ( it == SMAUG_ITEM_SCROLL     ) return ITEM_SCROLL;
  if ( it == SMAUG_ITEM_WAND       ) return ITEM_WAND;
  if ( it == SMAUG_ITEM_STAFF      ) return ITEM_STAFF;
  if ( it == SMAUG_ITEM_WEAPON     ) return ITEM_WEAPON;
  if ( it == SMAUG_ITEM_TREASURE   ) return ITEM_TREASURE;
  if ( it == SMAUG_ITEM_ARMOR      ) return ITEM_ARMOR;
  if ( it == SMAUG_ITEM_POTION     ) return ITEM_POTION;
  if ( it == SMAUG_ITEM_FURNITURE  ) return ITEM_FURNITURE;
  if ( it == SMAUG_ITEM_TRASH      ) return ITEM_TRASH;
  if ( it == SMAUG_ITEM_CONTAINER  ) return ITEM_CONTAINER;
  if ( it == SMAUG_ITEM_DRINK_CON  ) return ITEM_DRINK_CON;
  if ( it == SMAUG_ITEM_KEY        ) return ITEM_KEY;
  if ( it == SMAUG_ITEM_FOOD       ) return ITEM_FOOD;
  if ( it == SMAUG_ITEM_MONEY      ) return ITEM_MONEY;
  if ( it == SMAUG_ITEM_BOAT       ) return ITEM_BOAT;
  if ( it == SMAUG_ITEM_CORPSE_NPC ) return ITEM_CORPSE_NPC;
  if ( it == SMAUG_ITEM_CORPSE_PC  ) return ITEM_CORPSE_PC;
  if ( it == SMAUG_ITEM_FOUNTAIN   ) return ITEM_FOUNTAIN;
  if ( it == SMAUG_ITEM_PILL       ) return ITEM_PILL;
  if ( it == SMAUG_ITEM_MAP        ) return ITEM_MAP;
  if ( it == SMAUG_ITEM_PORTAL     ) return ITEM_PORTAL;

  return ITEM_TRASH;
}

int extra_flags_smaug( int ef ) {
  int new_ef = 0;

  if ( ef & SFT(SMAUG_ITEM_GLOW) ) SET_BIT( new_ef, ITEM_GLOW );
  if ( ef & SFT(SMAUG_ITEM_HUM) ) SET_BIT( new_ef, ITEM_HUM );
  if ( ef & SFT(SMAUG_ITEM_DARK) ) SET_BIT( new_ef, ITEM_DARK );
  if ( ef & SFT(SMAUG_ITEM_EVIL) ) SET_BIT( new_ef, ITEM_EVIL );
  if ( ef & SFT(SMAUG_ITEM_INVIS) ) SET_BIT( new_ef, ITEM_INVIS );
  if ( ef & SFT(SMAUG_ITEM_MAGIC) ) SET_BIT( new_ef, ITEM_MAGIC );
  if ( ef & SFT(SMAUG_ITEM_NODROP) ) SET_BIT( new_ef, ITEM_NODROP );
  if ( ef & SFT(SMAUG_ITEM_BLESS) ) SET_BIT( new_ef, ITEM_BLESS );
  if ( ef & SFT(SMAUG_ITEM_ANTI_GOOD) ) SET_BIT( new_ef, ITEM_ANTI_GOOD );
  if ( ef & SFT(SMAUG_ITEM_ANTI_EVIL) ) SET_BIT( new_ef, ITEM_ANTI_EVIL );
  if ( ef & SFT(SMAUG_ITEM_ANTI_NEUTRAL) ) SET_BIT( new_ef, ITEM_ANTI_NEUTRAL );
  if ( ef & SFT(SMAUG_ITEM_INVENTORY) ) SET_BIT( new_ef, ITEM_INVENTORY );
  if ( ef & SFT(SMAUG_ITEM_DONATION) ) SET_BIT( new_ef, ITEM_DONATED );

  return new_ef;
}

int wear_flags_smaug( int wf ) {

  if ( wf == SMAUG_WEAR_NONE)  return WEAR_NONE ;
  if ( wf == SMAUG_WEAR_LIGHT)  return WEAR_LIGHT ;
  if ( wf == SMAUG_WEAR_FINGER_L)  return WEAR_FINGER_L ;
  if ( wf == SMAUG_WEAR_FINGER_R)  return WEAR_FINGER_R ;
  if ( wf == SMAUG_WEAR_NECK_1)  return WEAR_NECK_1 ;
  if ( wf == SMAUG_WEAR_NECK_2)  return WEAR_NECK_2 ;
  if ( wf == SMAUG_WEAR_BODY)  return WEAR_BODY ;
  if ( wf == SMAUG_WEAR_HEAD)  return WEAR_HEAD ;
  if ( wf == SMAUG_WEAR_LEGS)  return WEAR_LEGS ;
  if ( wf == SMAUG_WEAR_FEET)  return WEAR_FEET ;
  if ( wf == SMAUG_WEAR_HANDS)  return WEAR_HANDS ;
  if ( wf == SMAUG_WEAR_ARMS)  return WEAR_ARMS ;
  if ( wf == SMAUG_WEAR_SHIELD)  return WEAR_SHIELD ;
  if ( wf == SMAUG_WEAR_ABOUT)  return WEAR_ABOUT ;
  if ( wf == SMAUG_WEAR_WAIST)  return WEAR_WAIST ;
  if ( wf == SMAUG_WEAR_WRIST_L)  return WEAR_WRIST_L ;
  if ( wf == SMAUG_WEAR_WRIST_R)  return WEAR_WRIST_R ;
  if ( wf == SMAUG_WEAR_WIELD)  return WEAR_WIELD ;
  if ( wf == SMAUG_WEAR_HOLD)  return WEAR_HOLD ;
  if ( wf == SMAUG_WEAR_DUAL_WIELD)  return WEAR_SECONDARY ;
  if ( wf == SMAUG_WEAR_EARS)  return WEAR_EAR_L ;
  if ( wf == SMAUG_WEAR_EARS)  return WEAR_EAR_R ;
  if ( wf == SMAUG_WEAR_EYES)  return WEAR_EYES ;

  return WEAR_NONE;
}

int item_wear_flags_smaug( int wf ) {
  int new_wf = 0;

  if ( wf & SMAUG_ITEM_TAKE ) SET_BIT( new_wf, ITEM_TAKE );
  if ( wf & SMAUG_ITEM_WEAR_FINGER ) SET_BIT( new_wf, ITEM_WEAR_FINGER );
  if ( wf & SMAUG_ITEM_WEAR_NECK ) SET_BIT( new_wf, ITEM_WEAR_NECK );
  if ( wf & SMAUG_ITEM_WEAR_BODY ) SET_BIT( new_wf, ITEM_WEAR_BODY );
  if ( wf & SMAUG_ITEM_WEAR_HEAD ) SET_BIT( new_wf, ITEM_WEAR_HEAD );
  if ( wf & SMAUG_ITEM_WEAR_LEGS ) SET_BIT( new_wf, ITEM_WEAR_LEGS );
  if ( wf & SMAUG_ITEM_WEAR_FEET ) SET_BIT( new_wf, ITEM_WEAR_FEET );
  if ( wf & SMAUG_ITEM_WEAR_HANDS ) SET_BIT( new_wf, ITEM_WEAR_HANDS );
  if ( wf & SMAUG_ITEM_WEAR_ARMS ) SET_BIT( new_wf, ITEM_WEAR_ARMS );
  if ( wf & SMAUG_ITEM_WEAR_SHIELD ) SET_BIT( new_wf, ITEM_WEAR_SHIELD );
  if ( wf & SMAUG_ITEM_WEAR_ABOUT ) SET_BIT( new_wf, ITEM_WEAR_ABOUT );
  if ( wf & SMAUG_ITEM_WEAR_WAIST ) SET_BIT( new_wf, ITEM_WEAR_WAIST );
  if ( wf & SMAUG_ITEM_WEAR_WRIST ) SET_BIT( new_wf, ITEM_WEAR_WRIST );
  if ( wf & SMAUG_ITEM_WIELD ) SET_BIT( new_wf, ITEM_WIELD );
  if ( wf & SMAUG_ITEM_HOLD ) SET_BIT( new_wf, ITEM_HOLD );
  if ( wf & SMAUG_ITEM_WEAR_EARS ) SET_BIT( new_wf, ITEM_WEAR_EAR );
  if ( wf & SMAUG_ITEM_WEAR_EYES ) SET_BIT( new_wf, ITEM_WEAR_EYES );
  if ( wf & SMAUG_ITEM_DUAL_WIELD ) SET_BIT( new_wf, ITEM_WIELD );

  return new_wf;
}

int act_smaug( int ac ) {
  int new_ac = ACT_IS_NPC;

  if ( ac & SFT(SMAUG_ACT_SENTINEL) ) SET_BIT( new_ac, ACT_SENTINEL );
  if ( ac & SFT(SMAUG_ACT_SCAVENGER) ) SET_BIT( new_ac, ACT_SCAVENGER );
  if ( ac & SFT(SMAUG_ACT_AGGRESSIVE) ) SET_BIT( new_ac, ACT_AGGRESSIVE );
  if ( ac & SFT(SMAUG_ACT_STAY_AREA) ) SET_BIT( new_ac, ACT_STAY_AREA );
  if ( ac & SFT(SMAUG_ACT_WIMPY) ) SET_BIT( new_ac, ACT_WIMPY );
  if ( ac & SFT(SMAUG_ACT_PET) ) SET_BIT( new_ac, ACT_PET );
  if ( ac & SFT(SMAUG_ACT_TRAIN) ) SET_BIT( new_ac, ACT_TRAIN|ACT_GAIN );
  if ( ac & SFT(SMAUG_ACT_PRACTICE) ) SET_BIT( new_ac, ACT_PRACTICE );
  if ( ac & SFT(SMAUG_ACT_IMMORTAL) ) SET_BIT( new_ac, ACT_IS_SAFE );
  if ( ac & SFT(SMAUG_ACT_PRACTICE) ) SET_BIT( new_ac, ACT_PRACTICE );

  return new_ac;
}

int pos_smaug( int po ) {
  if (po>=100)
    po-=100;
  switch(po){
  default: return POS_STANDING;
  case 0: 
  case 1: 
  case 2: 
  case 3: 
  case 4: return po; // dead --> sleeping
  case 5: return POS_RESTING;
  case 6: return POS_SITTING;
  case 7: return POS_FIGHTING;
  case 8: 
  case 9: 
  case 10:
  case 11: return POS_STANDING;
  }
}

int race_smaug( int ra ) {
  switch (ra) {
  case 0: return race_lookup("human");
  case 1: return race_lookup("high-elf");
  case 2: return race_lookup("hill-dwarf");
  case 3: return race_lookup("halfling");
  case 4: return race_lookup("pixie");
  case 5: return race_lookup("vampire");
  case 6: return race_lookup("half-ogre");
  case 7: return race_lookup("half-orc");
  case 8: return race_lookup("troll");
  case 9: return race_lookup("half-elf");
  case 10: return race_lookup("lizardman");
  case 11: return race_lookup("sea-elf");
  case 12: return race_lookup("human"); // githianky
  case 13: return race_lookup("drow");
  case 14: return race_lookup("rock-gnome");
  }
  return race_lookup("human");
}

int exit_smaug( int ex ) {
  int new_ex = 0;

  if ( IS_SET( ex, SMAUG_EX_ISDOOR ) ) SET_BIT( new_ex, EX_ISDOOR );
  if ( IS_SET( ex, SMAUG_EX_CLOSED ) ) SET_BIT( new_ex, EX_CLOSED );
  if ( IS_SET( ex, SMAUG_EX_LOCKED ) ) SET_BIT( new_ex, EX_LOCKED );
  if ( IS_SET( ex, SMAUG_EX_SECRET ) ) SET_BIT( new_ex, EX_HIDDEN );
  if ( IS_SET( ex, SMAUG_EX_PICKPROOF ) ) SET_BIT( new_ex, EX_PICKPROOF );
  if ( IS_SET( ex, SMAUG_EX_NOPASSDOOR ) ) SET_BIT( new_ex, EX_NOPASS );
  if ( IS_SET( ex, SMAUG_EX_PICKPROOF ) ) SET_BIT( new_ex, EX_PICKPROOF );
  if ( IS_SET( ex, SMAUG_EX_HIDDEN ) ) SET_BIT( new_ex, EX_HIDDEN );
  // Added by SinaC 2003
  if ( IS_SET( ex, SMAUG_EX_BASHED ) ) SET_BIT( new_ex, EX_BASHED );
  if ( IS_SET( ex, SMAUG_EX_EATKEY ) ) SET_BIT( new_ex, EX_EATKEY );
  if ( IS_SET( ex, SMAUG_EX_BASHPROOF ) ) SET_BIT( new_ex, EX_BASHPROOF );
  if ( IS_SET( ex, SMAUG_EX_CLIMB ) ) SET_BIT( new_ex, EX_CLIMB );

  return new_ex;
}

int room_flag_smaug( int rf ) {
  int new_rf = 0;
  
  if ( IS_SET( rf, SMAUG_ROOM_DARK ) ) SET_BIT( new_rf, ROOM_DARK );
  if ( IS_SET( rf, SMAUG_ROOM_NO_MOB ) ) SET_BIT( new_rf, ROOM_NO_MOB );
  if ( IS_SET( rf, SMAUG_ROOM_INDOORS ) ) SET_BIT( new_rf, ROOM_INDOORS );
  if ( IS_SET( rf, SMAUG_ROOM_NO_MAGIC ) ) SET_BIT( new_rf, ROOM_NOSPELL );
  if ( IS_SET( rf, SMAUG_ROOM_PRIVATE ) ) SET_BIT( new_rf, ROOM_PRIVATE );
  if ( IS_SET( rf, SMAUG_ROOM_SAFE ) ) SET_BIT( new_rf, ROOM_SAFE );
  if ( IS_SET( rf, SMAUG_ROOM_SOLITARY ) ) SET_BIT( new_rf, ROOM_SOLITARY );
  if ( IS_SET( rf, SMAUG_ROOM_PET_SHOP ) ) SET_BIT( new_rf, ROOM_PET_SHOP );
  if ( IS_SET( rf, SMAUG_ROOM_NO_RECALL ) ) SET_BIT( new_rf, ROOM_NO_RECALL );
  //if ( IS_SET( rf, SMAUG_ROOM_TELEPORT ) ) SET_BIT( new_rf, ROOM_TELEPORT );
  if ( IS_SET( rf, SMAUG_ROOM_ARENA ) ) SET_BIT( new_rf, ROOM_ARENA );
  if ( IS_SET( rf, SMAUG_ROOM_TUNNEL ) ) SET_BIT( new_rf, ROOM_SOLITARY );

  return new_rf;
}

int sector_smaug( int se ) {
  if ( se == SMAUG_SECT_INSIDE ) return SECT_INSIDE;
  if ( se == SMAUG_SECT_CITY ) return SECT_CITY;  
  if ( se == SMAUG_SECT_FIELD ) return SECT_FIELD;
  if ( se == SMAUG_SECT_FOREST ) return SECT_FOREST;
  if ( se == SMAUG_SECT_HILLS ) return SECT_HILLS;
  if ( se == SMAUG_SECT_MOUNTAIN ) return SECT_MOUNTAIN;
  if ( se == SMAUG_SECT_WATER_SWIM ) return SECT_WATER_SWIM;
  if ( se == SMAUG_SECT_WATER_NOSWIM ) return SECT_WATER_NOSWIM;
  if ( se == SMAUG_SECT_UNDERWATER ) return SECT_UNDERWATER;
  if ( se == SMAUG_SECT_AIR ) return SECT_AIR;
  if ( se == SMAUG_SECT_DESERT ) return SECT_DESERT;
  if ( se == SMAUG_SECT_LAVA ) return SECT_BURNING;

  return SECT_INSIDE;
}

void load_version_smaug( FILE *fp ) {
  smaug_version = fread_number(fp);
}

void convert_mob_smaug( MOB_INDEX_DATA *pMobIndex ) {
  pMobIndex->act = act_smaug( pMobIndex->act );
  int save_aff = pMobIndex->affected_by;
  pMobIndex->affected_by = aff_smaug( save_aff );
  pMobIndex->affected2_by = aff2_smaug( save_aff );
  pMobIndex->start_pos = pos_smaug( pMobIndex->start_pos );
  pMobIndex->default_pos = pos_smaug( pMobIndex->default_pos );
  //race
  pMobIndex->race = race_smaug(pMobIndex->race);
  if ( pMobIndex->race < 0 ) {
    bug("convert_mob_smaug: invalid race for mob (%d)", pMobIndex->vnum);
    pMobIndex->race = 0;
  }
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

// Added by SinaC 2001 for  Smaug style
void load_mob_smaug( FILE *fp )
{
  MOB_INDEX_DATA *pMobIndex;
 
  if ( !area_last ) {   /* OLC */
    bug( "Load_mob_smaug: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
 
    letter                          = fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_mob_smaug: # not found.");
      exit( 1 );
    }
 
    vnum                            = fread_number( fp );
    if ( vnum == 0 )
      break;
 
    fBootDb = FALSE;
    if ( get_mob_index( vnum ) != NULL ) {
      bug( "Load_mob_smaug: vnum %d duplicated.", vnum );
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

    // Added by SinaC 2001
    pMobIndex->classes              = 0;

    pMobIndex->act                  = fread_flag( fp );
    pMobIndex->affected_by          = fread_flag( fp );
    // Added by SinaC 2001
    pMobIndex->affected2_by          = 0;

    pMobIndex->pShop                = NULL;
    // Added by SinaC 2001
    pMobIndex->align.etho           = 0;
    pMobIndex->align.alignment       = fread_number( fp );

    letter = fread_letter( fp );

    pMobIndex->group                = 0;

    pMobIndex->level                = fread_number( fp );

    // Added by SinaC 2001
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
    
    if ( letter != 'S' && letter != 'C' ) {
      bug("Invalid letter (%c) for mob (%d)", letter, pMobIndex->vnum );
      exit(-1);
    }

    if ( letter == 'C' ) { // complex mob
      char *ln;
      int x1, x2, x3, x4, x5, x6, x7, x8;

      // skips stats: str, int, wis, dex, con, cha, lck
      fread_number( fp );  fread_number( fp );  fread_number( fp );
      fread_number( fp );  fread_number( fp );  fread_number( fp );
      fread_number( fp );
      // skips saves: saves_poison, saves_wand, saves_para, saves_breath, saves_spell
      fread_number( fp ); fread_number( fp ); fread_number( fp );
      fread_number( fp ); fread_number( fp );

      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
      sscanf( ln, "%d %d %d %d %d %d %d",
	      &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
      pMobIndex->race = x1;
      //x2 is class,   x3 is height,   x4 is weight
      //x5 is speaks,  x6 is speaking, x7 is #attacks

      ln = fread_line( fp );
      x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
      sscanf( ln, "%d %d %d %d %d %d %d %d",
	      &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
      pMobIndex->hitroll = x1;
      pMobIndex->damage[DICE_BONUS] += x2;
      //x3 is xflags
      pMobIndex->res_flags = x4;
      pMobIndex->imm_flags = x5;
      pMobIndex->vuln_flags = x6;
      //x7 is attack,  x8 is defense
    }
    else {
      pMobIndex->race = 0;
      pMobIndex->hitroll = 0;
      pMobIndex->res_flags = 0;
      pMobIndex->imm_flags = 0;
      pMobIndex->vuln_flags = 0;
    }

    convert_mob_smaug( pMobIndex );

    for ( ; ; ) {
      letter = fread_letter( fp );
      // skipping these version of mobprograms
      if (letter == '>') {
	// Added by SinaC 2003
	char prg_name[MAX_STRING_LENGTH];
	sprintf(prg_name,"mob%d", vnum );
	pMobIndex->program = hash_get_prog(prg_name);
	if (!pMobIndex->program)
	  bug("Can't find program for mob vnum %d.", pMobIndex->vnum);	      
	else {
	  if ( get_root_class( pMobIndex->program ) != default_mob_class ) {
	    bug("program for mob vnum %d is not a mob program." ,pMobIndex->vnum);
	    pMobIndex->program = NULL;
	  }
	  else
	    if ( pMobIndex->program->isAbstract )
	      bug("program for mob vnum %d is an ABSTRACT class.", pMobIndex->vnum );
	}

	skip_till_(fp,'|');
      }
      else {
	ungetc(letter,fp);
	break;
      }
    }

    iHash                   = vnum % MAX_KEY_HASH;
    pMobIndex->next         = mob_index_hash[iHash];
    mob_index_hash[iHash]   = pMobIndex;
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
    assign_area_vnum( vnum );                                  /* OLC */
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
  }
 
  return;
}

void convert_room_smaug( ROOM_INDEX_DATA *pRoomIndex ) {
  // room flags
  pRoomIndex->bstat(flags) = room_flag_smaug( pRoomIndex->bstat(flags) );
  // sector
  pRoomIndex->bstat(sector) = sector_smaug( pRoomIndex->bstat(sector) );
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

void convert_exit_smaug( EXIT_DATA *pexit ) {
  pexit->exit_info = pexit->rs_flags = exit_smaug( pexit->exit_info );
}

// Added by SinaC 2001 for  Smaug style
void load_room_smaug( FILE *fp )
{
  ROOM_INDEX_DATA *pRoomIndex;

  if ( area_last == NULL ) {
    bug( "Load_room_smaug: no #AREA seen yet.");
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
      bug( "Load_room_smaug: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_room_index( vnum ) != NULL ) {
      bug( "Load_room_smaug: vnum %d duplicated.", vnum );
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

    ln = fread_line(fp);
    x1 = x2 = x3 = x4 = x5 = x6 = 0;
    sscanf( ln, "%d %d %d %d %d %d",
	    &x1, &x2, &x3, &x4, &x5, &x6 );
    pRoomIndex->bstat(flags)		= x2;
    pRoomIndex->bstat(sector)		= x3;
    pRoomIndex->bstat(maxsize)		= SIZE_NOSIZE;

    pRoomIndex->bstat(light)		= 0;
    for ( door = 0; door < MAX_DIR; door++ ) // Modified by SinaC 2003
      pRoomIndex->exit[door] = NULL;

    // Modified by SinaC 2001
    /* defaults */
    pRoomIndex->bstat(healrate) = 100;
    pRoomIndex->bstat(manarate) = 100;
    // Added by SinaC 2001 for mental user
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
	  bug( "Load_room_smaug: vnum %d has bad door number.", vnum );
	  exit( 1 );
	}

	if ( door == DIR_SPECIAL )
	  log_stringf("DIR_SPECIAL found for room vnum: %d.", vnum );

	//	pexit			= (EXIT_DATA *) alloc_perm( sizeof(*pexit) );
	pexit = new_exit();
	pexit->description	= fread_string( fp );
	pexit->keyword		= fread_string( fp );
	ln = fread_line( fp );
	x1 = x2 = x3 = x4 = x5 = x6 = 0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6 );
	pexit->exit_info	= 0;
	pexit->rs_flags         = 0;                    /* OLC */
	locks			= x1;
	pexit->key		= x2;
	pexit->u1.vnum		= x3;
	pexit->orig_door	= door;			/* OLC */

	switch ( locks ) {
	case 1: pexit->exit_info = pexit->rs_flags = EX_ISDOOR; break;
	case 2: pexit->exit_info = pexit->rs_flags = EX_ISDOOR | EX_PICKPROOF; break;
	default: pexit->exit_info = pexit->rs_flags = locks;
	  break;
	}

	convert_exit_smaug(pexit);
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

      case 'M': // skips
	fread_number( fp );
	fread_number( fp );
	fread_number( fp );
	fread_letter( fp );
	break;

      case '>': // skips
	// Added by SinaC 2003
	char prg_name[MAX_STRING_LENGTH];
	sprintf(prg_name,"room%d", vnum );
	pRoomIndex->program = hash_get_prog(prg_name);
	if (!pRoomIndex->program)
	  bug("Can't find program for room vnum %d.", vnum);	      
	else {
	  if ( get_root_class( pRoomIndex->program ) != default_room_class ) {
	    bug("program for room vnum %d is not a room program." ,pRoomIndex->vnum);
	    pRoomIndex->program = NULL;
	  }
	  else
	    if ( pRoomIndex->program->isAbstract )
	      bug("program for room vnum %d is an ABSTRACT class.", pRoomIndex->vnum );
	}

	skip_till_( fp, '|' );
	break;
	
      default :	    
	bug( "Load_room_smaug: vnum %d has unknown flag.", vnum );
	exit( 1 );
      }
    }

    convert_room_smaug(pRoomIndex);

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum( vnum );                                    /* OLC */
  }

  return;
}

// Added by SinaC 2001 for  Smaug style
void load_reset_smaug( FILE *fp )
{
  RESET_DATA *pReset;
  int         iLastRoom = 0;
  int         iLastObj  = 0;
  int         iLastObj2  = 0;

  if ( !area_last ) {
    bug( "Load_reset_smaug: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
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
      bug( "Load_reset_smaug: bad command '%c'.", letter );
      exit( 1 );
      break;

    case 'M':
      get_mob_index  ( pReset->arg1 );
      /*	    get_room_index ( pReset->arg3 ); */
      // fix added by SinaC 2001
      if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
	bug("reset M has arg2 equals to 0 (room: %d)", pReset->arg3);

      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastRoom = pReset->arg3;
      }
      break;

    case 'O': temp_index = get_obj_index ( pReset->arg1 );
      temp_index->reset_num++; 
      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = pReset->arg3;
	iLastObj2 = pReset->arg1;
      }
      break;

    case 'P':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
      get_obj_index  ( pReset->arg1 );
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
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
 
      if ( pReset->arg2 == 1 ) {
	bug("reset 'G' or 'E' has arg2 equals to 1, setting to 0");
	pReset->arg2 = 0;
      }

      if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = iLastRoom;
	iLastObj2 = pReset->arg1;
      }
      break;

    case 'D':
      pRoomIndex = get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0
	   ||   pReset->arg2 >= MAX_DIR // > (MAX_DIR - 1)  SinaC 2003
	   || !pRoomIndex
	   || !( pexit = pRoomIndex->exit[pReset->arg2] )
	   || !IS_SET( pexit->rs_flags, EX_ISDOOR ) ) {
	bug( "Load_reset_smaug: 'D': exit %d not door.", pReset->arg2 );
	exit( 1 );
      }

      switch ( pReset->arg3 ) {
      default:
	bug( "Load_reset_smaug: 'D': bad 'locks': %d." , pReset->arg3);
      case 0: break;
      case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
	SET_BIT( pexit->exit_info, EX_CLOSED ); break;
      case 2: SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
	SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED ); break;
      }

      break;

    case 'R':
      pRoomIndex		= get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0 || pReset->arg2 > MAX_DIR ) {
	bug( "Load_reset_smaug: 'R': bad exit %d.", pReset->arg2 );
	exit( 1 );
      }

      if ( pRoomIndex )
	new_reset( pRoomIndex, pReset );

      break;

      // Not used
    case 'T':
      break;
      
    case 'H':
      break;
      
    case 'B':
      break;
    }


    /*	if ( area_last->reset_first == NULL )
	area_last->reset_first	= pReset;
	if ( area_last->reset_last  != NULL )
	area_last->reset_last->next	= pReset;
	    
	area_last->reset_last	= pReset;
	pReset->next		= NULL;
    */
  }

  return;
}

void convert_obj_smaug( OBJ_INDEX_DATA *pObjIndex ) {

  pObjIndex->item_type = item_type_smaug( pObjIndex->item_type );
  if ( pObjIndex->item_type == 0 )
    bug("Convert_obj_smaug: invalid item_type for object (%d)",
	pObjIndex->vnum );
  
  pObjIndex->extra_flags = extra_flags_smaug( pObjIndex->extra_flags );
  pObjIndex->wear_flags = item_wear_flags_smaug( pObjIndex->wear_flags );
}

/*
 * Snarf an obj section.  old style 
 */
void load_obj_smaug( FILE *fp )
{
  OBJ_INDEX_DATA *pObjIndex;
  char *word;

  if ( !area_last ) {  /* OLC */
    bug( "Load_obj_smaug: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
    int x1, x2, x3, x4, x5, x6;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_obj_smaug: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_obj_index( vnum ) != NULL ) {
      bug( "Load_obj_smaug: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //    pObjIndex			= (OBJ_INDEX_DATA *) alloc_perm( sizeof(*pObjIndex) );
    pObjIndex = new_obj_index();
    pObjIndex->vnum			= vnum;
    pObjIndex->area                 = area_last;            /* OLC */
    pObjIndex->new_format		= FALSE;
    pObjIndex->reset_num	 	= 0;
    newobjs++;
    pObjIndex->name			= fread_string( fp );
    pObjIndex->short_descr		= fread_string_lower( fp );
    pObjIndex->description		= fread_string_upper( fp );
    fread_string( fp ); // unused

    pObjIndex->material = 0;

    pObjIndex->item_type		= fread_number( fp );
    pObjIndex->extra_flags		= fread_flag( fp );

    // Added by SinaC 2003
    pObjIndex->size = SIZE_NOSIZE;
    
    char* ln;
    ln = fread_line( fp );
    x1=x2=0;
    sscanf( ln, "%d %d",
	    &x1, &x2 );
    pObjIndex->wear_flags		= x1;
    // x2 is not used
    
    ln = fread_line( fp );
    x1=x2=x3=x4=x5=x6=0;
    sscanf( ln, "%d %d %d %d %d %d",
	    &x1, &x2, &x3, &x4, &x5, &x6 );
    pObjIndex->value[0]		= x1;
    pObjIndex->value[1]		= x2;
    pObjIndex->value[2]		= x3;
    pObjIndex->value[3]		= x4;
    pObjIndex->value[4]		= x5;
    // x6 is not used

    pObjIndex->weight		= fread_number( fp );
    pObjIndex->weight = UMAX( 1, pObjIndex->weight );
    pObjIndex->cost			= fread_number( fp );
          fread_number( fp ); // unused

    pObjIndex->condition = 100;
    pObjIndex->level = 1;

    if ( smaug_version == 1 ) {
      switch ( pObjIndex->item_type ) {
      case SMAUG_ITEM_PILL:
      case SMAUG_ITEM_POTION:
      case SMAUG_ITEM_SCROLL:
	pObjIndex->value[1] = ability_lookup ( fread_word( fp )) ;
	pObjIndex->value[2] = ability_lookup ( fread_word( fp )) ;
	pObjIndex->value[3] = ability_lookup ( fread_word( fp )) ;
	break;
      case SMAUG_ITEM_STAFF:
      case SMAUG_ITEM_WAND:
	pObjIndex->value[3] = ability_lookup ( fread_word( fp )) ;
	break;
      case SMAUG_ITEM_SALVE:
	pObjIndex->value[4] = ability_lookup ( fread_word( fp )) ;
	pObjIndex->value[5] = ability_lookup ( fread_word( fp )) ;
	break;
      }
    }
    for ( ; ; ) {
      letter = fread_letter( fp );
      
      if ( letter == 'A' ) {
	AFFECT_DATA *paf;
	
	//	paf                     = (AFFECT_DATA*) alloc_perm( sizeof(*paf) );
	paf = new_affect();
	createaff(*paf,-1,20,-1,0,AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT);
	int loc = fread_number( fp );
	int mod = fread_number(fp);
	if ( loc == SMAUG_APPLY_WEAPONSPELL
	     ||   loc == SMAUG_APPLY_WEARSPELL
	     ||   loc == SMAUG_APPLY_REMOVESPELL
	     ||   loc == SMAUG_APPLY_STRIPSN
	     ||   loc == SMAUG_APPLY_RECURRINGSPELL )
	  mod = slot_lookup( mod );
	addaff2(*paf,AFTO_CHAR,loc,AFOP_ADD,mod);
	//paf->where              = AFTO_CHAR;
	//paf->op                 = AFOP_ADD;
	//paf->type		= -1;
	//paf->level		= 20; /* RT temp fix */
	//paf->duration		= -1;
	//paf->location         = fread_number( fp );
	//if ( paf->location == SMAUG_APPLY_WEAPONSPELL
	//     ||   paf->location == SMAUG_APPLY_WEARSPELL
	//     ||   paf->location == SMAUG_APPLY_REMOVESPELL
	//     ||   paf->location == SMAUG_APPLY_STRIPSN
	//     ||   paf->location == SMAUG_APPLY_RECURRINGSPELL )
	//  paf->modifier		= slot_lookup( fread_number(fp) );
	//else
	//  paf->modifier		= fread_number( fp );

	convert_af_smaug( paf );
	
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
      else if ( letter == '>' ) {
	// Added by SinaC 2003
	char prg_name[MAX_STRING_LENGTH];
	sprintf(prg_name,"obj%d", vnum );
	pObjIndex->program = hash_get_prog(prg_name);
	if (!pObjIndex->program)
	  bug("Can't find program for obj vnum %d.", pObjIndex->vnum);	      
	else {
	  if ( get_root_class( pObjIndex->program ) != default_obj_class ) {
	    bug("program for obj vnum %d is not an obj program." ,pObjIndex->vnum);
	    pObjIndex->program = NULL;
	  }
	  else
	    if ( pObjIndex->program->isAbstract )
	      bug("program for obj vnum %d is an ABSTRACT class.", pObjIndex->vnum );
	}

	skip_till_( fp, '|' );
      }
      else {
	ungetc( letter, fp );
	break;
      }
    }
    // Translate spell "slot numbers" to internal "skill numbers."
    if ( smaug_version == 0 )
      switch ( pObjIndex->item_type ) {
      case SMAUG_ITEM_PILL:
      case SMAUG_ITEM_POTION:
      case SMAUG_ITEM_SCROLL:
	pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	break;
	
      case SMAUG_ITEM_STAFF:
      case SMAUG_ITEM_WAND:
	pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	break;
      case SMAUG_ITEM_SALVE:
	pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
	break;
      }

    convert_obj_smaug( pObjIndex );

    iHash			= vnum % MAX_KEY_HASH;
    pObjIndex->next		= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObjIndex;
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
    assign_area_vnum( vnum );                                   /* OLC */
  }

  return;
}


// SinaC 2003
void load_repair_smaug( FILE *fp ) {
  int v;
  for ( ; ; ) {
    v = fread_number( fp );
    if ( v == 0 )
      break;
    for ( int i = 0; i < 7; i++ ) // read 7 values
      fread_number( fp );
    fread_to_eol( fp ); // skip end of line
  }

  return;
}


/*
// Load a mob section.

void load_mobiles( AREA_DATA *tarea, FILE *fp )
{
    MOB_INDEX_DATA *pMobIndex;
    char *ln;
    int x1, x2, x3, x4, x5, x6, x7, x8;

    if ( !tarea )
    {
	bug( "Load_mobiles: no #AREA seen yet." );
	if ( fBootDb )
	{
	    shutdown_mud( "No #AREA" );
	    exit( 1 );
	}
	else
	    return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	sh_int vnum;
	char letter;
	int iHash;
	bool oldmob;
	bool tmpBootDb;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_mobiles: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_mob_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_mobiles: vnum %d duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pMobIndex = get_mob_index( vnum );
		sprintf( buf, "Cleaning mobile: %d", vnum );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_mob( pMobIndex );
		oldmob = TRUE;
	    }
	}
	else
	{
	  oldmob = FALSE;
	  CREATE( pMobIndex, MOB_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pMobIndex->vnum			= vnum;
	if ( fBootDb )
	{
	    if ( !tarea->low_m_vnum )
		tarea->low_m_vnum	= vnum;
	    if ( vnum > tarea->hi_m_vnum )
		tarea->hi_m_vnum	= vnum;
	}
	pMobIndex->player_name		= fread_string( fp );
	pMobIndex->short_descr		= fread_string( fp );
	pMobIndex->long_descr		= fread_string( fp );
	pMobIndex->description		= fread_string( fp );

	pMobIndex->long_descr[0]	= UPPER(pMobIndex->long_descr[0]);
	pMobIndex->description[0]	= UPPER(pMobIndex->description[0]);

	pMobIndex->act			= fread_bitvector( fp );
	xSET_BIT(pMobIndex->act, ACT_IS_NPC);
	pMobIndex->affected_by		= fread_bitvector( fp );
	pMobIndex->pShop		= NULL;
	pMobIndex->rShop		= NULL;
	pMobIndex->alignment		= fread_number( fp );
	letter				= fread_letter( fp );
	pMobIndex->level		= fread_number( fp );

	pMobIndex->mobthac0		= fread_number( fp );
	pMobIndex->ac			= fread_number( fp );
	pMobIndex->hitnodice		= fread_number( fp );
	// 'd'		
	         fread_letter( fp );
	pMobIndex->hitsizedice		= fread_number( fp );
	// '+'		
           	 fread_letter( fp );
	pMobIndex->hitplus		= fread_number( fp );
	pMobIndex->damnodice		= fread_number( fp );
	// 'd'		
	         fread_letter( fp );
	pMobIndex->damsizedice		= fread_number( fp );
	// '+'
                 fread_letter( fp );
	pMobIndex->damplus		= fread_number( fp );
	pMobIndex->gold			= fread_number( fp );
	pMobIndex->exp			= fread_number( fp );

	// pMobIndex->position		= fread_number( fp );
         pMobIndex->position          = fread_number( fp );
         if(pMobIndex->position<100){
            switch(pMobIndex->position){
                default: 
                case 0: 
                case 1: 
                case 2: 
                case 3: 
                case 4: break;
                case 5: pMobIndex->position=6; break;
                case 6: pMobIndex->position=8; break;
                case 7: pMobIndex->position=9; break;
                case 8: pMobIndex->position=12; break;
                case 9: pMobIndex->position=13; break;
                case 10: pMobIndex->position=14; break;
                case 11: pMobIndex->position=15; break;
            }
         } 
         else
            pMobIndex->position-=100;

	// pMobIndex->defposition		= fread_number( fp );
         pMobIndex->defposition          = fread_number( fp );
         if(pMobIndex->defposition<100){
            switch(pMobIndex->defposition){
                default: 
                case 0: 
                case 1: 
                case 2: 
                case 3: 
                case 4: break;
                case 5: pMobIndex->defposition=6; break;
                case 6: pMobIndex->defposition=8; break;
                case 7: pMobIndex->defposition=9; break;
                case 8: pMobIndex->defposition=12; break;
                case 9: pMobIndex->defposition=13; break;
                case 10: pMobIndex->defposition=14; break;
                case 11: pMobIndex->defposition=15; break;
            }
         } 
         else
            pMobIndex->defposition-=100;


	// Back to meaningful values.
	pMobIndex->sex			= fread_number( fp );

	if ( letter != 'S' && letter != 'C' )
	{
	    bug( "Load_mobiles: vnum %d: letter '%c' not S or C.", vnum,
	        letter );
	    shutdown_mud( "bad mob data" );
	    exit( 1 );
	}
	if ( letter == 'C' ) // Realms complex mob 	-Thoric 
	{
	    pMobIndex->perm_str			= fread_number( fp );
	    pMobIndex->perm_int			= fread_number( fp );
	    pMobIndex->perm_wis			= fread_number( fp );
	    pMobIndex->perm_dex			= fread_number( fp );
	    pMobIndex->perm_con			= fread_number( fp );
	    pMobIndex->perm_cha			= fread_number( fp );
	    pMobIndex->perm_lck			= fread_number( fp );
 	    pMobIndex->saving_poison_death	= fread_number( fp );
	    pMobIndex->saving_wand		= fread_number( fp );
	    pMobIndex->saving_para_petri	= fread_number( fp );
	    pMobIndex->saving_breath		= fread_number( fp );
	    pMobIndex->saving_spell_staff	= fread_number( fp );
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7 );
	    pMobIndex->race		= x1;
	    pMobIndex->class		= x2;
	    pMobIndex->height		= x3;
	    pMobIndex->weight		= x4;
	    pMobIndex->speaks		= x5;
	    pMobIndex->speaking		= x6;
	    pMobIndex->numattacks	= x7;
	    //  Thanks to Nick Gammon for noticing this.
	    //if ( !pMobIndex->speaks )
	    //	pMobIndex->speaks = race_table[pMobIndex->race]->language | LANG_COMMON;
	    //if ( !pMobIndex->speaking )
	    //	pMobIndex->speaking = race_table[pMobIndex->race]->language;
	    if ( !pMobIndex->speaks )
		pMobIndex->speaks = LANG_COMMON;
	    if ( !pMobIndex->speaking )
		pMobIndex->speaking = LANG_COMMON;

#ifndef XBI
	    ln = fread_line( fp );
	    x1=x2=x3=x4=x5=x6=x7=x8=0;
	    sscanf( ln, "%d %d %d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8 );
	    pMobIndex->hitroll		= x1;
	    pMobIndex->damroll		= x2;
	    pMobIndex->xflags		= x3;
	    pMobIndex->resistant	= x4;
	    pMobIndex->immune		= x5;
	    pMobIndex->susceptible	= x6;
	    pMobIndex->attacks		= x7;
	    pMobIndex->defenses		= x8;
#else
	    pMobIndex->hitroll		= fread_number(fp);
	    pMobIndex->damroll		= fread_number(fp);
	    pMobIndex->xflags		= fread_number(fp);
	    pMobIndex->resistant	= fread_number(fp);
	    pMobIndex->immune		= fread_number(fp);
	    pMobIndex->susceptible	= fread_number(fp);
	    pMobIndex->attacks		= fread_bitvector(fp);
	    pMobIndex->defenses		= fread_bitvector(fp);
#endif
	}
	else
	{
	    pMobIndex->perm_str		= 13;
	    pMobIndex->perm_dex		= 13;
	    pMobIndex->perm_int		= 13;
	    pMobIndex->perm_wis		= 13;
	    pMobIndex->perm_cha		= 13;
	    pMobIndex->perm_con		= 13;
	    pMobIndex->perm_lck		= 13;
	    pMobIndex->race		= 0;
	    pMobIndex->class		= 3;
	    pMobIndex->xflags		= 0;
	    pMobIndex->resistant	= 0;
	    pMobIndex->immune		= 0;
	    pMobIndex->susceptible	= 0;
	    pMobIndex->numattacks	= 0;
#ifdef XBI
	    xCLEAR_BITS(pMobIndex->attacks);
	    xCLEAR_BITS(pMobIndex->defenses);
#else
	    pMobIndex->attacks		= 0;
	    pMobIndex->defenses		= 0;
#endif
	}

	letter = fread_letter( fp );
	if ( letter == '>' )
	{
	    ungetc( letter, fp );
	    mprog_read_programs( fp, pMobIndex );
	}
	else ungetc( letter,fp );

	if ( !oldmob )
	{
	    iHash			= vnum % MAX_KEY_HASH;
	    pMobIndex->next		= mob_index_hash[iHash];
	    mob_index_hash[iHash]	= pMobIndex;
	}
    }

    return;
}



// Load an obj section.
void load_objects( AREA_DATA *tarea, FILE *fp )
{
    OBJ_INDEX_DATA *pObjIndex;
    char letter;
    char *ln;
    int x1, x2, x3, x4, x5, x6;

    if ( !tarea )
    {
	bug( "Load_objects: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    for ( ; ; )
    {
	char buf[MAX_STRING_LENGTH];
	int vnum;
	int iHash;
	bool tmpBootDb;
	bool oldobj;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_objects: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_obj_index( vnum ) )
	{
	    if ( tmpBootDb )
	    {
		bug( "Load_objects: vnum %d duplicated.", vnum );
		shutdown_mud( "duplicate vnum" );
		exit( 1 );
	    }
	    else
	    {
		pObjIndex = get_obj_index( vnum );
		sprintf( buf, "Cleaning object: %d", vnum );
		log_string_plus( buf, LOG_BUILD, sysdata.log_level );
		clean_obj( pObjIndex );
		oldobj = TRUE;
	    }
	}
	else
	{
	  oldobj = FALSE;
	  CREATE( pObjIndex, OBJ_INDEX_DATA, 1 );
	}
	fBootDb = tmpBootDb;

	pObjIndex->vnum			= vnum;
	if ( fBootDb )
	{
	  if ( !tarea->low_o_vnum )
	    tarea->low_o_vnum		= vnum;
	  if ( vnum > tarea->hi_o_vnum )
	    tarea->hi_o_vnum		= vnum;
	}
	pObjIndex->name			= fread_string( fp );
	pObjIndex->short_descr		= fread_string( fp );
	pObjIndex->description		= fread_string( fp );
	pObjIndex->action_desc		= fread_string( fp );

        // Commented out by Narn, Apr/96 to allow item short descs like 
        //   Bonecrusher and Oblivion
	//pObjIndex->short_descr[0]	= LOWER(pObjIndex->short_descr[0]);
	pObjIndex->description[0]	= UPPER(pObjIndex->description[0]);

	pObjIndex->item_type		= fread_number(fp);
	pObjIndex->extra_flags		= fread_bitvector(fp);
	ln = fread_line( fp );
	x1=x2=0;
	sscanf( ln, "%d %d",
		&x1, &x2 );
	pObjIndex->wear_flags		= x1;
	pObjIndex->layers		= x2;

	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
		&x1, &x2, &x3, &x4, &x5, &x6 );
	pObjIndex->value[0]		= x1;
	pObjIndex->value[1]		= x2;
	pObjIndex->value[2]		= x3;
	pObjIndex->value[3]		= x4;
	pObjIndex->value[4]		= x5;
	pObjIndex->value[5]		= x6;
	pObjIndex->weight		= fread_number( fp );
	pObjIndex->weight = UMAX( 1, pObjIndex->weight );
	pObjIndex->cost			= fread_number( fp );
	pObjIndex->rent		  	= fread_number( fp ); // unused
        if ( area_version == 1 )
	{
	  switch ( pObjIndex->item_type )
	  {
	  case ITEM_PILL:
	  case ITEM_POTION:
	  case ITEM_SCROLL:
	      pObjIndex->value[1] = ability_lookup ( fread_word( fp )) ;
	      pObjIndex->value[2] = ability_lookup ( fread_word( fp )) ;
	      pObjIndex->value[3] = ability_lookup ( fread_word( fp )) ;
	    break;
	  case ITEM_STAFF:
	  case ITEM_WAND:
	      pObjIndex->value[3] = ability_lookup ( fread_word( fp )) ;
	    break;
	  case ITEM_SALVE:
	      pObjIndex->value[4] = ability_lookup ( fread_word( fp )) ;
	      pObjIndex->value[5] = ability_lookup ( fread_word( fp )) ;
	    break;
	  }
	}
	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'A' )
	    {
		AFFECT_DATA *paf;

		CREATE( paf, AFFECT_DATA, 1 );
		paf->type		= -1;
		paf->duration		= -1;
		paf->location		= fread_number( fp );
		if ( paf->location == APPLY_WEAPONSPELL
		||   paf->location == APPLY_WEARSPELL
		||   paf->location == APPLY_REMOVESPELL
		||   paf->location == APPLY_STRIPSN
		||   paf->location == APPLY_RECURRINGSPELL )
		  paf->modifier		= slot_lookup( fread_number(fp) );
		else
		  paf->modifier		= fread_number( fp );
		xCLEAR_BITS(paf->bitvector);
		LINK( paf, pObjIndex->first_affect, pObjIndex->last_affect,
			   next, prev );
	    }

	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc,
			  next, prev );
	    }
	    else if ( letter == '>' )
	    {
	        ungetc( letter, fp );
	        oprog_read_programs( fp, pObjIndex );
	    }

	    else
	    {
		ungetc( letter, fp );
		break;
	    }
	}

	// Translate spell "slot numbers" to internal "skill numbers."
	if ( area_version == 0 )
	   switch ( pObjIndex->item_type )
	   {
	   case ITEM_PILL:
	   case ITEM_POTION:
	   case ITEM_SCROLL:
	       pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
	       pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
	       pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	       break;
   
	   case ITEM_STAFF:
	   case ITEM_WAND:
	       pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
	       break;
	   case ITEM_SALVE:
	       pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
	       pObjIndex->value[5] = slot_lookup( pObjIndex->value[5] );
	       break;
	   }

	if ( !oldobj )
	{
	  iHash			= vnum % MAX_KEY_HASH;
	  pObjIndex->next	= obj_index_hash[iHash];
	  obj_index_hash[iHash]	= pObjIndex;
	}
    }

    return;
}



// Load a reset section.
void load_resets( AREA_DATA *tarea, FILE *fp )
{
    char buf[MAX_STRING_LENGTH];
    bool not01 = FALSE;
    int count = 0;

    if ( !tarea )
    {
	bug( "Load_resets: no #AREA seen yet." );
	if ( fBootDb )
	{
	  shutdown_mud( "No #AREA" );
	  exit( 1 );
	}
	else
	  return;
    }

    if ( tarea->first_reset )
    {
	if ( fBootDb )
	{
	  RESET_DATA *rtmp;

	  bug( "load_resets: WARNING: resets already exist for this area." );
	  for ( rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next )
		++count;
	}
	else
	{
	  // Clean out the old resets
	  sprintf( buf, "Cleaning resets: %s", tarea->name );
	  log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	  clean_resets( tarea );
	}	
    }

    for ( ; ; )
    {
	ROOM_INDEX_DATA *pRoomIndex;
	EXIT_DATA *pexit;
	char letter;
	int extra, arg1, arg2, arg3;

	if ( ( letter = fread_letter( fp ) ) == 'S' )
	    break;

	if ( letter == '*' )
	{
	    fread_to_eol( fp );
	    continue;
	}

	extra	= fread_number( fp );
	arg1	= fread_number( fp );
	arg2	= fread_number( fp );
	arg3	= (letter == 'G' || letter == 'R')
		  ? 0 : fread_number( fp );
		  fread_to_eol( fp );

	++count;

	// Validate parameters.
	// We're calling the index functions for the side effect.
	switch ( letter )
	{
	default:
	    bug( "Load_resets: bad command '%c'.", letter );
	    if ( fBootDb )
	      boot_log( "Load_resets: %s (%d) bad command '%c'.", tarea->filename, count, letter );
	    return;

	case 'M':
	    if ( get_mob_index( arg1 ) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) 'M': mobile %d doesn't exist.",
		    tarea->filename, count, arg1 );
	    if ( get_room_index( arg3 ) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) 'M': room %d doesn't exist.",
		    tarea->filename, count, arg3 );
	    break;

	case 'O':
	    if ( get_obj_index(arg1) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    if ( get_room_index(arg3) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': room %d doesn't exist.",
		    tarea->filename, count, letter, arg3 );
	    break;

	case 'P':
	    if ( get_obj_index(arg1) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    if ( arg3 > 0 )
		if ( get_obj_index(arg3) == NULL && fBootDb )
		    boot_log( "Load_resets: %s (%d) 'P': destination object %d doesn't exist.",
			tarea->filename, count, arg3 );
	    else if ( extra > 1 )
	      not01 = TRUE;
	    break;

	case 'G':
	case 'E':
	    if ( get_obj_index(arg1) == NULL && fBootDb )
		boot_log( "Load_resets: %s (%d) '%c': object %d doesn't exist.",
		    tarea->filename, count, letter, arg1 );
	    break;

	case 'T':
	    break;

	case 'H':
	    if ( arg1 > 0 )
		if ( get_obj_index(arg1) == NULL && fBootDb )
		    boot_log( "Load_resets: %s (%d) 'H': object %d doesn't exist.",
			tarea->filename, count, arg1 );
	    break;

	case 'B':
	    switch(arg2 & BIT_RESET_TYPE_MASK)
	    {
	    case BIT_RESET_DOOR:
	      {
	      int door;
	      
	      pRoomIndex = get_room_index( arg1 );
	      if ( !pRoomIndex )
	      {
		bug( "Load_resets: 'B': room %d doesn't exist.", arg1 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'B': room %d doesn't exist.",
			tarea->filename, count, arg1 );
	      }
	    
	      door = (arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;

	      if ( !(pexit = get_exit(pRoomIndex, door)) )
	      {
		bug( "Load_resets: 'B': exit %d not door.", door );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'B': exit %d not door.",
			tarea->filename, count, door );
	      }
	      }
	      break;
	    case BIT_RESET_ROOM:
	      if (get_room_index(arg1) == NULL)
	      {
	        bug( "Load_resets: 'B': room %d doesn't exist.", arg1);
	        bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
	            arg3 );
	        if ( fBootDb )
	           boot_log( "Load_resets: %s (%d) 'B': room %d doesn't exist.",
	                tarea->filename, count, arg1 );
	      }
	      break;
	    case BIT_RESET_OBJECT:
	      if (arg1 > 0)
	        if (get_obj_index(arg1) == NULL && fBootDb)
	          boot_log("Load_resets: %s (%d) 'B': object %d doesn't exist.",
	              tarea->filename, count, arg1 );
	      break;
	    case BIT_RESET_MOBILE:
	      if (arg1 > 0)
	        if (get_mob_index(arg1) == NULL && fBootDb)
	          boot_log("Load_resets: %s (%d) 'B': mobile %d doesn't exist.",
	              tarea->filename, count, arg1 );
	      break;
	    default:
	      boot_log( "Load_resets: %s (%d) 'B': bad type flag (%d).",
	           tarea->filename, count, arg2 & BIT_RESET_TYPE_MASK );
	      break;
	    }
	    break;

	case 'D':
	    pRoomIndex = get_room_index( arg1 );
	    if ( !pRoomIndex )
	    {
		bug( "Load_resets: 'D': room %d doesn't exist.", arg1 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'D': room %d doesn't exist.",
			tarea->filename, count, arg1 );
		break;
	    }

	    if ( arg2 < 0
	    ||   arg2 > MAX_DIR+1
	    || ( pexit = get_exit(pRoomIndex, arg2)) == NULL
	    || !IS_SET( pexit->exit_info, EX_ISDOOR ) )
	    {
		bug( "Load_resets: 'D': exit %d not door.", arg2 );
		bug( "Reset: %c %d %d %d %d", letter, extra, arg1, arg2,
		    arg3 );
		if ( fBootDb )
		   boot_log( "Load_resets: %s (%d) 'D': exit %d not door.",
			tarea->filename, count, arg2 );
	    }

	    if ( arg3 < 0 || arg3 > 2 )
	    {
		bug( "Load_resets: 'D': bad 'locks': %d.", arg3 );
		if ( fBootDb )
		  boot_log( "Load_resets: %s (%d) 'D': bad 'locks': %d.",
			tarea->filename, count, arg3 );
	    }
	    break;

	case 'R':
	    pRoomIndex = get_room_index( arg1 );
	    if ( !pRoomIndex && fBootDb )
		boot_log( "Load_resets: %s (%d) 'R': room %d doesn't exist.",
		    tarea->filename, count, arg1 );

	    if ( arg2 < 0 || arg2 > 10 )
	    {
		bug( "Load_resets: 'R': bad exit %d.", arg2 );
		if ( fBootDb )
		  boot_log( "Load_resets: %s (%d) 'R': bad exit %d.",
			tarea->filename, count, arg2 );
		break;
	    }

	    break;
	}

	// finally, add the reset
	add_reset( tarea, letter, extra, arg1, arg2, arg3 );
    }
    
    if ( !not01 )
      renumber_put_resets(tarea);

    return;
}

// Load a room section.
void load_rooms( AREA_DATA *tarea, FILE *fp )
{
    ROOM_INDEX_DATA *pRoomIndex;
    char buf[MAX_STRING_LENGTH];
    char *ln;

    if ( !tarea )
    {
	bug( "Load_rooms: no #AREA seen yet." );
	shutdown_mud( "No #AREA" );
	exit( 1 );
    }

    for ( ; ; )
    {
	int vnum;
	char letter;
	int door;
	int iHash;
	bool tmpBootDb;
	bool oldroom;
	int x1, x2, x3, x4, x5, x6;

	letter				= fread_letter( fp );
	if ( letter != '#' )
	{
	    bug( "Load_rooms: # not found." );
	    if ( fBootDb )
	    {
		shutdown_mud( "# not found" );
		exit( 1 );
	    }
	    else
		return;
	}

	vnum				= fread_number( fp );
	if ( vnum == 0 )
	    break;

	tmpBootDb = fBootDb;
	fBootDb = FALSE;
	if ( get_room_index( vnum ) != NULL )
	{
	    if ( tmpBootDb )
	    {
	      bug( "Load_rooms: vnum %d duplicated.", vnum );
	      shutdown_mud( "duplicate vnum" );
	      exit( 1 );
	    }
	    else
	    {
	      pRoomIndex = get_room_index( vnum );
	      sprintf( buf, "Cleaning room: %d", vnum );
	      log_string_plus( buf, LOG_BUILD, sysdata.log_level );
	      clean_room( pRoomIndex );
	      oldroom = TRUE;
	    }
	}
	else
	{
	  oldroom = FALSE;
	  CREATE( pRoomIndex, ROOM_INDEX_DATA, 1 );
	  pRoomIndex->first_person	= NULL;
	  pRoomIndex->last_person	= NULL;
	  pRoomIndex->first_content	= NULL;
	  pRoomIndex->last_content	= NULL;
	}

	fBootDb = tmpBootDb;
	pRoomIndex->area		= tarea;
	pRoomIndex->vnum		= vnum;
	pRoomIndex->first_extradesc	= NULL;
	pRoomIndex->last_extradesc	= NULL;

	if ( fBootDb )
	{
	  if ( !tarea->low_r_vnum )
	    tarea->low_r_vnum		= vnum;
	  if ( vnum > tarea->hi_r_vnum )
	    tarea->hi_r_vnum		= vnum;
	}
	pRoomIndex->name		= fread_string( fp );
	pRoomIndex->description		= fread_string( fp );

	// Area number			  fread_number( fp );
	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %d %d %d %d %d",
	      &x1, &x2, &x3, &x4, &x5, &x6 );

	pRoomIndex->room_flags		= x2;
	pRoomIndex->sector_type		= x3;
	pRoomIndex->tele_delay		= x4;
	pRoomIndex->tele_vnum		= x5;
	pRoomIndex->tunnel		= x6;

	if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX)
	{
	  bug( "Fread_rooms: vnum %d has bad sector_type %d.", vnum ,
	      pRoomIndex->sector_type);
	  pRoomIndex->sector_type = 1;
	}
	pRoomIndex->light		= 0;
	pRoomIndex->first_exit		= NULL;
	pRoomIndex->last_exit		= NULL;

	for ( ; ; )
	{
	    letter = fread_letter( fp );

	    if ( letter == 'S' )
		break;

	    if ( letter == 'D' )
	    {
		EXIT_DATA *pexit;
		int locks;

		door = fread_number( fp );
		if ( door < 0 || door >= MAX_DOOR )
		{
		    bug( "Fread_rooms: vnum %d has bad door number %d.", vnum,
		        door );
		    if ( fBootDb )
		      exit( 1 );
		}
		else
		{
		  pexit = make_exit( pRoomIndex, NULL, door );
		  pexit->description	= fread_string( fp );
		  pexit->keyword	= fread_string( fp );
		  pexit->exit_info	= 0;
		  ln = fread_line( fp );
		  x1=x2=x3=x4=x5=x6=0;
		  sscanf( ln, "%d %d %d %d %d %d",
		      &x1, &x2, &x3, &x4, &x5, &x6 );

		  locks			= x1;
		  pexit->key		= x2;
		  pexit->vnum		= x3;
		  pexit->vdir		= door;
		  pexit->distance	= x4;
		  pexit->pulltype	= x5;
		  pexit->pull		= x6;

		  switch ( locks )
		  {
		    case 1:  pexit->exit_info = EX_ISDOOR;                break;
		    case 2:  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF; break;
		    default: pexit->exit_info = locks;
		  }
		}
	    }
	    else if ( letter == 'E' )
	    {
		EXTRA_DESCR_DATA *ed;

		CREATE( ed, EXTRA_DESCR_DATA, 1 );
		ed->keyword		= fread_string( fp );
		ed->description		= fread_string( fp );
		LINK( ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc,
			  next, prev );
	    }
	    else if ( letter == 'M' )    // maps
	    {
		MAP_DATA *map;
		MAP_INDEX_DATA *map_index;
		int i, j;

		CREATE( map, MAP_DATA, 1);
                map->vnum                     = fread_number( fp );
                map->x                        = fread_number( fp ); 
                map->y                        = fread_number( fp );
		map->entry		      = fread_letter( fp );
		
                pRoomIndex->map               = map;
		if(  (map_index = get_map_index(map->vnum)) == NULL  )
		{
                     CREATE( map_index, MAP_INDEX_DATA, 1);
		     map_index->vnum = map->vnum;
		     map_index->next = first_map;
                     first_map       = map_index;
		     for (i = 0; i <  49; i++) {
			     for (j = 0; j <  79; j++) {
			       map_index->map_of_vnums[i][j] = -1;
			       // map_index->map_of_ptrs[i][j] = NULL;
                             }
                     }
		}
		if( (map->y <0) || (map->y >48) )
		{
                    bug("Map y coord out of range.  Room %d\n\r", map->y);

		}
		if( (map->x <0) || (map->x >78) )
		{
                    bug("Map x coord out of range.  Room %d\n\r", map->x);

		}
		if(  (map->x >0) 
		   &&(map->x <80) 
		   &&(map->y >0) 
		   &&(map->y <48) )
		   map_index->map_of_vnums[map->y][map->x]=pRoomIndex->vnum;
            }
	    else if ( letter == '>' )
	    {
	      ungetc( letter, fp );
	      rprog_read_programs( fp, pRoomIndex );
            }
	    else
	    {
		bug( "Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum,
		    letter );
		shutdown_mud( "Room flag not DES" );
		exit( 1 );
	    }

	}

	if ( !oldroom )
	{
	  iHash			 = vnum % MAX_KEY_HASH;
	  pRoomIndex->next	 = room_index_hash[iHash];
	  room_index_hash[iHash] = pRoomIndex;
	}
    }

    return;
}

*/
