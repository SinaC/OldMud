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
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "olc_value.h"
#include "interp.h"

// Added by SinaC 2000 for skill/spell/class in file  
const struct position_type target_table[]= {
    { "IGNORE",           "ignore" },
    { "CHAR_OFFENSIVE",   "off"    },
    { "CHAR_DEFENSIVE",   "def"    },
    { "CHAR_SELF",        "self"   },
    { "OBJ_INV",          "inv"    },
    { "OBJ_CHAR_DEF",     "odef"   },
    { "OBJ_CHAR_OFF",     "ooff"   },

    { NULL,               NULL     }
};

struct flag_type target_flags[] = {
    {   "ignore",         TAR_IGNORE,         TRUE  },
    {   "offensive",      TAR_CHAR_OFFENSIVE, TRUE  },
    {   "defensive",      TAR_CHAR_DEFENSIVE, TRUE  },
    {   "self",           TAR_CHAR_SELF,      TRUE  },
    {   "inventory",      TAR_OBJ_INV,        TRUE  },
    {   "objdefensive",   TAR_OBJ_CHAR_DEF,   TRUE  },
    {   "objoffensive",   TAR_OBJ_CHAR_OFF,   TRUE  },

    {   NULL,              0,                    0       }
};


// for position 
const struct position_type position_table[] = {
    {	"dead",			"dead"	},
    {	"mortally wounded",	"mort"	},
    {	"incapacitated",	"incap"	},
    {   "paralyzed",            "para"  }, // Added by SinaC 2003
    {	"stunned",		"stun"	},
    {	"sleeping",		"sleep"	},
    {	"resting",		"rest"	},
    {   "sitting",		"sit"   },
    {	"fighting",		"fight"	},
    {	"standing",		"stand"	},
    {	NULL,			NULL	}
};

/* for sex */
const struct sex_type sex_table[] = {
   {	"none"		},
   {	"male"		},
   {	"female"	},
   {	"either"	},
   {	NULL		}
};

/* for sizes */
const struct size_type size_table[] = { 
    {	"tiny"		},
    {	"small" 	},
    {	"medium"	},
    {	"large"		},
    {	"huge", 	},
    {	"giant" 	},
    {	NULL		}
};

/* various flag tables */
 struct flag_type act_flags[] = {
    {	"npc",			ACT_IS_NPC,	FALSE	},
    {	"sentinel",		ACT_SENTINEL,	TRUE	},
    {	"scavenger",		ACT_SCAVENGER,	TRUE	},
    {   "aware",                ACT_AWARE,      TRUE    }, // SinaC 2003
    {	"aggressive",		ACT_AGGRESSIVE,	TRUE	},
    {	"stay_area",		ACT_STAY_AREA,	TRUE	},
    {	"wimpy",		ACT_WIMPY,	TRUE	},
    {	"pet",			ACT_PET,	TRUE	},
    {	"train",		ACT_TRAIN,	TRUE	},
    {	"practice",		ACT_PRACTICE,	TRUE	},
  /* Can leave an area without being extract, SinaC 2001 */
    {   "freewander",           ACT_FREE_WANDER,TRUE    },

    // Added by SinaC 2003
    {   "mountable",            ACT_MOUNTABLE,  TRUE    },
    {   "mounted",              ACT_IS_MOUNTED, TRUE    },

    {	"undead",		ACT_UNDEAD,	TRUE	},
    {   "nosleep",              ACT_NOSLEEP,    TRUE    }, // SinaC 2003
    {	"cleric",		ACT_CLERIC,	TRUE	},
    {	"mage",			ACT_MAGE,	TRUE	},
    {	"thief",		ACT_THIEF,	TRUE	},
    {	"warrior",		ACT_WARRIOR,	TRUE	},
    {	"noalign",		ACT_NOALIGN,	TRUE	},
    {	"nopurge",		ACT_NOPURGE,	TRUE	},
    {	"outdoors",		ACT_OUTDOORS,	TRUE	},
    {	"indoors",		ACT_INDOORS,	TRUE	},
    {	"healer",		ACT_IS_HEALER,	TRUE	},
    {	"gain",			ACT_GAIN,	TRUE	},
    {	"update_always",	ACT_UPDATE_ALWAYS,	TRUE	},
    //{	"changer",		ACT_IS_CHANGER,	TRUE	},
    {   "reserved",             ACT_RESERVED,   TRUE   },
    {	"is_safe",		ACT_IS_SAFE,	TRUE	},
    {   "created",              ACT_CREATED,    TRUE    },
    {	NULL,			0,	FALSE	}
};

struct flag_type plr_flags[] = {
    {	"npc",			PLR_IS_NPC,	FALSE	},
    {	"autoassist",		PLR_AUTOASSIST,	FALSE	},
    {	"autoexit",		PLR_AUTOEXIT,	FALSE	},
    {	"autoloot",		PLR_AUTOLOOT,	FALSE	},
    {	"autosac",		PLR_AUTOSAC,	FALSE	},
    {	"autogold",		PLR_AUTOGOLD,	FALSE	},
    {	"autosplit",		PLR_AUTOSPLIT,	FALSE	},
    {   "autotitle",            PLR_AUTOTITLE,  FALSE   },
    {   "autohaggle",           PLR_AUTOHAGGLE, FALSE   }, // SinaC 2003
    //    {   "questor",              PLR_QUESTOR,    FALSE   },
    {	"holylight",		PLR_HOLYLIGHT,	FALSE	},
    {	"can_loot",		PLR_CANLOOT,	FALSE	},
    {	"nosummon",		PLR_NOSUMMON,	FALSE	},
    {	"nofollow",		PLR_NOFOLLOW,	FALSE	},
    {	"colour",		PLR_COLOUR,	FALSE	},
    {	"permit",		PLR_PERMIT,	TRUE	},
    {	"log",			PLR_LOG,	FALSE	},
    {	"deny",			PLR_DENY,	FALSE	},
    {	"freeze",		PLR_FREEZE,	FALSE	},
    {	"thief",		PLR_THIEF,	FALSE	},
    {	"killer",		PLR_KILLER,	FALSE	},
    // Added by SinaC 2000 for gambling game, removed by SinaC 2003
    //{   "gambling",             PLR_GAMBLER,    FALSE   },
    // Added by SinaC 2000 to allow immortal to become mortal
    {   "mortal",               PLR_MORTAL,     FALSE   },
    {	NULL,			0,	        0	}
};

 struct flag_type affect_flags[] = {
    {	"blind",		AFF_BLIND,	        TRUE	},
    {	"invisible",		AFF_INVISIBLE,	        TRUE	},
    {	"detect_evil",		AFF_DETECT_EVIL,	TRUE	},
    {	"detect_invis",		AFF_DETECT_INVIS,	TRUE	},
    {	"detect_magic",		AFF_DETECT_MAGIC,	TRUE	},
    {	"detect_hidden",	AFF_DETECT_HIDDEN,	TRUE	},
    {	"detect_good",		AFF_DETECT_GOOD,	TRUE	},
    {	"sanctuary",		AFF_SANCTUARY,	        TRUE	},
    {	"faerie_fire",		AFF_FAERIE_FIRE,	TRUE	},
    {	"infrared",		AFF_INFRARED,	        TRUE	},
    {	"curse",		AFF_CURSE,      	TRUE	},
    {	"poison",		AFF_POISON,     	TRUE	},
    {	"protect_evil",		AFF_PROTECT_EVIL,	TRUE	},
    {	"protect_good",		AFF_PROTECT_GOOD,	TRUE	},
    {	"sneak",		AFF_SNEAK,      	TRUE	},
    {	"hide",			AFF_HIDE,       	TRUE	},
    {	"sleep",		AFF_SLEEP,	        TRUE	},
    {	"charm",		AFF_CHARM,       	TRUE	},
    {	"flying",		AFF_FLYING,        	TRUE	},
    {	"pass_door",		AFF_PASS_DOOR,     	TRUE	},
    {	"haste",		AFF_HASTE,       	TRUE	},
    {	"calm",			AFF_CALM,        	TRUE	},
    {	"plague",		AFF_PLAGUE,       	TRUE	},
    {	"weaken",		AFF_WEAKEN,       	TRUE	},
    {	"dark_vision",		AFF_DARK_VISION,	TRUE	},
    {	"berserk",		AFF_BERSERK,      	TRUE	},
    {	"swim",			AFF_SWIM,       	TRUE	},
    {	"regeneration",		AFF_REGENERATION,	TRUE	},
    {	"slow",			AFF_SLOW,       	TRUE	},
    // Added by SinaC 2000 for silenced people
    {   "silence",              AFF_SILENCE,            TRUE },
    // Added by SinaC 2001 for rooted people
    {   "rooted",               AFF_ROOTED,             TRUE },
    // Added by SinaC 2001
    /*
    *{   "walk_on_water",        AFF_WALK_ON_WATER,      TRUE },
    *{   "water_breath",         AFF_WATER_BREATH,       TRUE },
    // Added by SinaC 2001
    *{   "detect_exits",         AFF_DETECT_EXITS,       TRUE },
    // Added by SinaC 2001
    *{   "magic_mirror",         AFF_MAGIC_MIRROR,       TRUE },
    */
    {	NULL,			0,	0	}
};

// Added by SinaC 2001
struct flag_type affect2_flags[] = {
    {   "walk_on_water",        AFF2_WALK_ON_WATER,      TRUE },
    {   "water_breath",         AFF2_WATER_BREATH,       TRUE },
    {   "detect_exits",         AFF2_DETECT_EXITS,       TRUE },
    {   "magic_mirror",         AFF2_MAGIC_MIRROR,       TRUE },
    {   "faerie_fog",           AFF2_FAERIE_FOG,         TRUE },
    {   "no_equipment",         AFF2_NOEQUIPMENT,        TRUE },
    // Added by SinaC 2003
    {   "free_movement",        AFF2_FREE_MOVEMENT,      TRUE },
    {   "increased_casting",    AFF2_INCREASED_CASTING,  TRUE },
    {   "no_spell",             AFF2_NOSPELL,            TRUE },
    {   "necrotism",            AFF2_NECROTISM,          TRUE },
    {   "higher_attributes",    AFF2_HIGHER_MAGIC_ATTRIBUTES, TRUE },
    {   "confusion",            AFF2_CONFUSION,          TRUE },

    {	NULL,			0,	                 0    }
};


struct flag_type off_flags[] = {
    {	"area_attack",		OFF_AREA_ATTACK,	TRUE	},
    {	"backstab",		OFF_BACKSTAB,	TRUE	},
    {	"bash",			OFF_BASH,	TRUE	},
    {	"berserk",		OFF_BERSERK,	TRUE	},
    {	"disarm",		OFF_DISARM,	TRUE	},
    {	"dodge",		OFF_DODGE,	TRUE	},
    {	"fade",			OFF_FADE,	TRUE	},
    {	"fast",			OFF_FAST,	TRUE	},
    {	"kick",			OFF_KICK,	TRUE	},
    {	"dirt_kick",		OFF_KICK_DIRT,	TRUE	},
    {	"parry",		OFF_PARRY,	TRUE	},
    {	"rescue",		OFF_RESCUE,	TRUE	},
    {	"tail",			OFF_TAIL,	TRUE	},
    {	"trip",			OFF_TRIP,	TRUE	},
    {	"crush",		OFF_CRUSH,	TRUE	},
    {	"assist_all",		ASSIST_ALL,	TRUE	},
    {	"assist_align",		ASSIST_ALIGN,	TRUE	},
    {	"assist_race",		ASSIST_RACE,	TRUE	},
    {	"assist_players",	ASSIST_PLAYERS,	TRUE	},
    {	"assist_guard",		ASSIST_GUARD,	TRUE	},
    {	"assist_vnum",		ASSIST_VNUM,	TRUE	},
    // Added by SinaC 2000
    {   "counter",              OFF_COUNTER,    TRUE    },
    // Added by SinaC 2001
    {   "bite",                 OFF_BITE,    TRUE    },
    {	NULL,			0,	0	}
};

// IRV Immune Resistant Vulnerable, SinaC 2003
struct flag_type irv_flags[] = {
  {	"summon",		IRV_SUMMON,	TRUE	},
  {	"charm",		IRV_CHARM,	TRUE	},
  {	"magic",		IRV_MAGIC,	TRUE	},
  {	"weapon",		IRV_WEAPON,	TRUE	},
  {	"bash",			IRV_BASH,	TRUE	},
  {	"pierce",		IRV_PIERCE,	TRUE	},
  {	"slash",		IRV_SLASH,	TRUE	},
  {	"fire",			IRV_FIRE,	TRUE	},
  {	"cold",			IRV_COLD,	TRUE	},
  {	"lightning",		IRV_LIGHTNING,	TRUE	},
  {	"acid",			IRV_ACID,	TRUE	},
  {	"poison",		IRV_POISON,	TRUE	},
  {	"negative",		IRV_NEGATIVE,	TRUE	},
  {	"holy",			IRV_HOLY,	TRUE	},
  {	"energy",		IRV_ENERGY,	TRUE	},
  {	"mental",		IRV_MENTAL,	TRUE	},
  {	"disease",		IRV_DISEASE,	TRUE	},
  {	"drowning",		IRV_DROWNING,	TRUE	},
  {	"light",		IRV_LIGHT,	TRUE	},
  {	"sound",		IRV_SOUND,	TRUE	},
  {     "paralysis",            IRV_PARALYSIS,  TRUE    }, // SinaC 2003
  {	"wood",			IRV_WOOD,	TRUE	},
  {	"silver",		IRV_SILVER,	TRUE	},
  {	"iron",			IRV_IRON,	TRUE	},
  {	"daylight",		IRV_DAYLIGHT,	TRUE	},
  {	"earth",	        IRV_EARTH,      TRUE	},
  {	"weaken",	        IRV_WEAKEN,      TRUE	},
  {	NULL,			0,	0	}
};

// struct flag_type imm_flags[] = {
//    {	"summon",		IMM_SUMMON,	TRUE	},
//    {	"charm",		IMM_CHARM,	TRUE	},
//    {	"magic",		IMM_MAGIC,	TRUE	},
//    {	"weapon",		IMM_WEAPON,	TRUE	},
//    {	"bash",			IMM_BASH,	TRUE	},
//    {	"pierce",		IMM_PIERCE,	TRUE	},
//    {	"slash",		IMM_SLASH,	TRUE	},
//    {	"fire",			IMM_FIRE,	TRUE	},
//    {	"cold",			IMM_COLD,	TRUE	},
//    {	"lightning",		IMM_LIGHTNING,	TRUE	},
//    {	"acid",			IMM_ACID,	TRUE	},
//    {	"poison",		IMM_POISON,	TRUE	},
//    {	"negative",		IMM_NEGATIVE,	TRUE	},
//    {	"holy",			IMM_HOLY,	TRUE	},
//    {	"energy",		IMM_ENERGY,	TRUE	},
//    {	"mental",		IMM_MENTAL,	TRUE	},
//    {	"disease",		IMM_DISEASE,	TRUE	},
//    {	"drowning",		IMM_DROWNING,	TRUE	},
//    {	"light",		IMM_LIGHT,	TRUE	},
//    {	"sound",		IMM_SOUND,	TRUE	},
//    {	"wood",			IMM_WOOD,	TRUE	},
//    {	"silver",		IMM_SILVER,	TRUE	},
//    {	"iron",			IMM_IRON,	TRUE	},
//    {	"daylight",		IMM_DAYLIGHT,	TRUE	},
//    {	"earth",	        IMM_EARTH,       TRUE	},
//    {	NULL,			0,	0	}
//};
//
// struct flag_type res_flags[] = {
//    {   "summon",        RES_SUMMON,           TRUE    },
//    {   "charm",         RES_CHARM,            TRUE    },
//    {   "magic",         RES_MAGIC,            TRUE    },
//    {   "weapon",        RES_WEAPON,           TRUE    },
//    {   "bash",          RES_BASH,             TRUE    },
//    {   "pierce",        RES_PIERCE,           TRUE    },
//    {   "slash",         RES_SLASH,            TRUE    },
//    {   "fire",          RES_FIRE,             TRUE    },
//    {   "cold",          RES_COLD,             TRUE    },
//    {   "lightning",     RES_LIGHTNING,        TRUE    },
//    {   "acid",          RES_ACID,             TRUE    },
//    {   "poison",        RES_POISON,           TRUE    },
//    {   "negative",      RES_NEGATIVE,         TRUE    },
//    {   "holy",          RES_HOLY,             TRUE    },
//    {   "energy",        RES_ENERGY,           TRUE    },
//    {   "mental",        RES_MENTAL,           TRUE    },
//    {   "disease",       RES_DISEASE,          TRUE    },
//    {   "drowning",      RES_DROWNING,         TRUE    },
//    {   "light",         RES_LIGHT,            TRUE    },
//    {   "sound",         RES_SOUND,            TRUE    },
//    {   "wood",          RES_WOOD,             TRUE    },
//    {   "silver",        RES_SILVER,           TRUE    },
//    {   "iron",          RES_IRON,             TRUE    },
//    {	"daylight",	 RES_DAYLIGHT,	       TRUE	},
//    {	"earth",	 RES_EARTH,	       TRUE	},
//    {    NULL,          0,            0    }
//};
//
//
// struct flag_type vuln_flags[] = {
//    {   "summon",        VULN_SUMMON,          TRUE    },
//    {   "charm",         VULN_CHARM,           TRUE    },
//    {   "magic",         VULN_MAGIC,           TRUE    },
//    {   "weapon",        VULN_WEAPON,          TRUE    },
//    {   "bash",          VULN_BASH,            TRUE    },
//    {   "pierce",        VULN_PIERCE,          TRUE    },
//    {   "slash",         VULN_SLASH,           TRUE    },
//    {   "fire",          VULN_FIRE,            TRUE    },
//    {   "cold",          VULN_COLD,            TRUE    },
//    {   "lightning",     VULN_LIGHTNING,       TRUE    },
//    {   "acid",          VULN_ACID,            TRUE    },
//    {   "poison",        VULN_POISON,          TRUE    },
//    {   "negative",      VULN_NEGATIVE,        TRUE    },
//    {   "holy",          VULN_HOLY,            TRUE    },
//    {   "energy",        VULN_ENERGY,          TRUE    },
//    {   "mental",        VULN_MENTAL,          TRUE    },
//    {   "disease",       VULN_DISEASE,         TRUE    },
//    {   "drowning",      VULN_DROWNING,        TRUE    },
//    {   "light",         VULN_LIGHT,           TRUE    },
//    {   "sound",         VULN_SOUND,           TRUE    },
//    {   "wood",          VULN_WOOD,            TRUE    },
//    {   "silver",        VULN_SILVER,          TRUE    },
//    {   "iron",          VULN_IRON,            TRUE    },
//    {	"daylight", 	 VULN_DAYLIGHT,	       TRUE	},
//    {	"earth",	 VULN_EARTH,	       TRUE	},
//    {   NULL,              0,                    0       }
//};

 struct flag_type form_flags[] = {
    {	"edible",		FORM_EDIBLE,		TRUE	},
    {	"poison",		FORM_POISON,		TRUE	},
    {	"magical",		FORM_MAGICAL,		TRUE	},
    {	"instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
    {	"other",		FORM_OTHER,		TRUE	},
    {	"animal",		FORM_ANIMAL,		TRUE	},
    {	"sentient",		FORM_SENTIENT,		TRUE	},
    {	"undead",		FORM_UNDEAD,		TRUE	},
    {	"construct",		FORM_CONSTRUCT,		TRUE	},
    {	"mist",			FORM_MIST,		TRUE	},
    {	"intangible",		FORM_INTANGIBLE,	TRUE	},
    {	"biped",		FORM_BIPED,		TRUE	},
    {	"centaur",		FORM_CENTAUR,		TRUE	},
    {	"insect",		FORM_INSECT,		TRUE	},
    {	"spider",		FORM_SPIDER,		TRUE	},
    {	"crustacean",		FORM_CRUSTACEAN,	TRUE	},
    {	"worm",			FORM_WORM,		TRUE	},
    {	"blob",			FORM_BLOB,		TRUE	},
    {	"mammal",		FORM_MAMMAL,		TRUE	},
    {	"bird",			FORM_BIRD,		TRUE	},
    {	"reptile",		FORM_REPTILE,		TRUE	},
    {	"snake",		FORM_SNAKE,		TRUE	},
    {	"dragon",		FORM_DRAGON,		TRUE	},
    {	"amphibian",		FORM_AMPHIBIAN,		TRUE	},
    {	"fish",			FORM_FISH ,		TRUE	},
    {	"cold_blood",		FORM_COLD_BLOOD,	TRUE	},
    // Added by SinaC 2001
    {   "fur",                  FORM_FUR,               TRUE    },
    // SinaC 2003
    {   "four_arms",            FORM_FOUR_ARMS,         TRUE    },
    {	NULL,			0,			0	}
};

 struct flag_type part_flags[] = {
    {	"head",			PART_HEAD,		TRUE	},
    {	"arms",			PART_ARMS,		TRUE	},
    {	"legs",			PART_LEGS,		TRUE	},
    {	"heart",		PART_HEART,		TRUE	},
    {	"brains",		PART_BRAINS,		TRUE	},
    {	"guts",			PART_GUTS,		TRUE	},
    {	"hands",		PART_HANDS,		TRUE	},
    {	"feet",			PART_FEET,		TRUE	},
    {	"fingers",		PART_FINGERS,		TRUE	},
    {	"ear",			PART_EAR,		TRUE	},
    {	"eye",			PART_EYE,		TRUE	},
    {	"long_tongue",		PART_LONG_TONGUE,	TRUE	},
    {	"eyestalks",		PART_EYESTALKS,		TRUE	},
    {	"tentacles",		PART_TENTACLES,		TRUE	},
    {	"fins",			PART_FINS,		TRUE	},
    {	"wings",		PART_WINGS,		TRUE	},
    {	"tail",			PART_TAIL,		TRUE	},
    {	"claws",		PART_CLAWS,		TRUE	},
    {	"fangs",		PART_FANGS,		TRUE	},
    {	"horns",		PART_HORNS,		TRUE	},
    {	"scales",		PART_SCALES,		TRUE	},
    {	"tusks",		PART_TUSKS,		TRUE	},
    {   "body",                 PART_BODY,              TRUE    }, // SinaC 2003
    {	NULL,			0,			0	}
};

struct flag_type comm_flags[] = {
    {	"quiet",		COMM_QUIET,		TRUE	},
    {   "deaf",			COMM_DEAF,		TRUE	},
    {   "nowiz",		COMM_NOWIZ,		TRUE	},
    {   "noauction",		COMM_NOAUCTION,		TRUE	},
    // Gossip replaced with OOC by SinaC 2001
    //    {   "nogossip",		COMM_NOGOSSIP,		TRUE	},
    {   "noooc",		COMM_NOOOC,		TRUE	},
    {   "noquestion",		COMM_NOQUESTION,	TRUE	},
    //    {   "nomusic",		COMM_NOMUSIC,		TRUE	},  removed by SinaC 2003
    // SinaC 2003, same as COMM_BUILDING but editing datas
    {   "editing",              COMM_EDITING,           FALSE   },

    {   "noclan",		COMM_NOCLAN,		TRUE	},
    {   "noquote",		COMM_NOQUOTE,		TRUE	},
    {   "shoutsoff",		COMM_SHOUTSOFF,		TRUE	},
    {   "notrivia",             COMM_NOTRIVIA,          TRUE    },
    {   "compact",		COMM_COMPACT,		TRUE	},
    {   "brief",		COMM_BRIEF,		TRUE	},
    {   "prompt",		COMM_PROMPT,		TRUE	},
    {   "combine",		COMM_COMBINE,		TRUE	},
    {   "telnet_ga",		COMM_TELNET_GA,		TRUE	},
    {   "show_affects",		COMM_SHOW_AFFECTS,	TRUE	},
    {   "nograts",		COMM_NOGRATS,		TRUE	},
    {   "noemote",		COMM_NOEMOTE,		TRUE	},
    {   "noshout",		COMM_NOSHOUT,		TRUE	},
    {   "notell",		COMM_NOTELL,		TRUE	},
    {   "nochannels",		COMM_NOCHANNELS,	TRUE	},
    {   "snoop_proof",		COMM_SNOOP_PROOF,	TRUE	},
    {   "afk",			COMM_AFK,		TRUE	},
    {   "nohints",              COMM_NOHINTS,           TRUE    },
    // Added by SinaC 2000 for building flag
    {   "building",             COMM_BUILDING,          FALSE   },
    //IC channel added by SinaC 2001
    {   "noic",		        COMM_NOIC,		TRUE	},
    {   "tick",		        COMM_TICK,		TRUE	},
    {	NULL,			0,			0	}
};

struct flag_type area_flags[] =  {
    {	"none",			AREA_NONE,		FALSE	},
    {	"changed",		AREA_CHANGED,		TRUE	},
    {	"added",		AREA_ADDED,		TRUE	},
    {	"loading",		AREA_LOADING,		FALSE	},
// Added by SinaC 2000 for area we don't want player goes in
    {   "noteleport",           AREA_NOTELEPORT,        TRUE    },
// Added by SinaC 2000 to allow earthquake in a area
    {   "earthquake",           AREA_EARTHQUAKE,        TRUE    },
    // Used by scrpred.C:gpf_saveAreaState and area_update
    //  also used by load_world_state (and we are sure this is not set at the end)
    {   "save_state",           AREA_SAVE_STATE,        FALSE   },
    {	NULL,			0,			0	}
};



 struct flag_type sex_flags[] = {
    {	"male",			SEX_MALE,		TRUE	},
    {	"female",		SEX_FEMALE,		TRUE	},
    {	"neutral",		SEX_NEUTRAL,		TRUE	},
    {   "random",               SEX_RANDOM,             TRUE    },   /* ROM */
    {	"none",			SEX_NEUTRAL,		TRUE	},
    {	NULL,			0,			0	}
};



struct flag_type exit_flags[] = {
    {   "door",			EX_ISDOOR,		TRUE    },
    {	"closed",		EX_CLOSED,		TRUE	},
    {	"locked",		EX_LOCKED,		TRUE	},
    {	"pickproof",		EX_PICKPROOF,		TRUE	},
    {   "nopass",		EX_NOPASS,		TRUE	},
    {   "easy",			EX_EASY,		TRUE	},
    {   "hard",			EX_HARD,		TRUE	},
    {	"infuriating",		EX_INFURIATING,		TRUE	},
    {	"noclose",		EX_NOCLOSE,		TRUE	},
    {	"nolock",		EX_NOLOCK,		TRUE	},
    // Added by SinaC 2001
    {   "hidden",               EX_HIDDEN,              TRUE    },
    // Added by SinaC 2003
    {   "bashed",               EX_BASHED,              TRUE    },
    // Added by SinaC 2003, create dynamic exit which are not save whe using asave
    {   "nosaving",             EX_NOSAVING,            FALSE   },
    // Added by SinaC 2003, must be flying or use climb skill to get access to those exits
    {   "climb",                EX_CLIMB,               TRUE    },
    // Added by SinaC 2003, When unlocking exit with a key, the key is destroyed
    {   "eatkey",               EX_EATKEY,              TRUE    },
    // Added by SinaC 2003, those exits cannot be bashed
    {   "bashproof",            EX_BASHPROOF,           TRUE    },
    // SinaC 2003, when set exit_info is not updated
    {   "noreset",              EX_NORESET,             TRUE    },
    {	NULL,			0,			0	}
};



 struct flag_type door_resets[] = {
    {	"open and unlocked",	0,		TRUE	},
    {	"closed and unlocked",	1,		TRUE	},
    {	"closed and locked",	2,		TRUE	},
    {	NULL,			0,		0	}
};



 struct flag_type room_flags[] = {
    {	"dark",			ROOM_DARK,		TRUE	},
    {   "death",                ROOM_DEATH,             TRUE    },
    {	"no_mob",		ROOM_NO_MOB,		TRUE	},
    {	"indoors",		ROOM_INDOORS,		TRUE	},
    {   "soundproof",           ROOM_SOUNDPROOF,        TRUE    },
    {   "notrack",              ROOM_NOTRACK,           TRUE    },
    {	"private",		ROOM_PRIVATE,		TRUE    },
    {	"safe",			ROOM_SAFE,		TRUE	},
    {	"solitary",		ROOM_SOLITARY,		TRUE	},
    {	"pet_shop",		ROOM_PET_SHOP,		TRUE	},
    {	"no_recall",		ROOM_NO_RECALL,		TRUE	},
    {	"imp_only",		ROOM_IMP_ONLY,		TRUE    },
    {	"gods_only",	        ROOM_GODS_ONLY,		TRUE    },
    {	"heroes_only",		ROOM_HEROES_ONLY,	TRUE	},
    {	"newbies_only",		ROOM_NEWBIES_ONLY,	TRUE	},
    {	"law",			ROOM_LAW,		TRUE	},
    {   "nowhere",		ROOM_NOWHERE,		TRUE	},
    {	"bank",			ROOM_BANK,		TRUE	},
    // Added by SinaC 2000 for Arena
    {   "arena",                ROOM_ARENA,             TRUE    },
    // other new flags
    {   "nospell",              ROOM_NOSPELL,           TRUE    },
    {   "noscan",               ROOM_NOSCAN,            TRUE    },
    // Added by SinaC 2000 for teleport, removed by SinaC 2003 can do with scripts
    //{   "teleport",             ROOM_TELEPORT,          TRUE    },
    // Added by SinaC 2001
    {   "nopower",              ROOM_NOPOWER,           TRUE    },
    {   "donation",             ROOM_DONATION,          TRUE    },
    {	NULL,			0,			0	}
};

 struct flag_type sector_flags[] = {
    {	"inside",	SECT_INSIDE,		TRUE	},
    {	"city",		SECT_CITY,		TRUE	},
    {	"field",	SECT_FIELD,		TRUE	},
    {	"forest",	SECT_FOREST,		TRUE	},
    {	"hills",	SECT_HILLS,		TRUE	},
    {	"mountain",	SECT_MOUNTAIN,		TRUE	},
    {	"swim",		SECT_WATER_SWIM,	TRUE	},
    {	"noswim",	SECT_WATER_NOSWIM,	TRUE	},
    // Has deleted SECT_UNUSED and has replaced it by SECT_BURNING
    //    {   "unused",	SECT_UNUSED,		TRUE	},
    {   "burning",      SECT_BURNING,           TRUE    },
    {	"air",		SECT_AIR,		TRUE	},
    {	"desert",	SECT_DESERT,		TRUE	},
    {   "underwater",   SECT_UNDERWATER,        TRUE    },
    {	NULL,		0,			0	}
};

 struct flag_type type_flags[] = {
    {	"light",		ITEM_LIGHT,		TRUE	},
    {	"scroll",		ITEM_SCROLL,		TRUE	},
    {	"wand",			ITEM_WAND,		TRUE	},
    {	"staff",		ITEM_STAFF,		TRUE	},
    {	"weapon",		ITEM_WEAPON,		TRUE	},
    {	"treasure",		ITEM_TREASURE,		TRUE	},
    {	"armor",		ITEM_ARMOR,		TRUE	},
    {	"potion",		ITEM_POTION,		TRUE	},
    {	"furniture",		ITEM_FURNITURE,		TRUE	},
    {	"trash",		ITEM_TRASH,		TRUE	},
    {	"container",		ITEM_CONTAINER,		TRUE	},
    {	"drinkcontainer",	ITEM_DRINK_CON,		TRUE	},
    {	"key",			ITEM_KEY,		TRUE	},
    {	"food",			ITEM_FOOD,		TRUE	},
    {	"money",		ITEM_MONEY,		TRUE	},
    {	"boat",			ITEM_BOAT,		TRUE	},
    {	"npccorpse",		ITEM_CORPSE_NPC,	TRUE	},
    {	"pccorpse",		ITEM_CORPSE_PC,		FALSE	},
    {	"fountain",		ITEM_FOUNTAIN,		TRUE	},
    {	"pill",			ITEM_PILL,		TRUE	},
    /* Removed by SinaC 2003
    // SinaC 2000 : have replaced ITEM_PROTECT with ITEM_THROWING
    //    {	"protect",		ITEM_PROTECT,		TRUE	},
    {   "throwing",             ITEM_THROWING,          TRUE    },
    */
    {	"map",			ITEM_MAP,		TRUE	},
    {   "portal",		ITEM_PORTAL,		TRUE	},
    {   "warpstone",		ITEM_WARP_STONE,	TRUE	},
    // have replaced ITEM_ROOM_KEY with ITEM_COMPONENT  SinaC 2000
    //    {	"roomkey",		ITEM_ROOM_KEY,		TRUE	},
    {   "component",            ITEM_COMPONENT,         TRUE    },

    { 	"gem",			ITEM_GEM,		TRUE	},
    {	"jewelry",		ITEM_JEWELRY,		TRUE	},
    //{	"jukebox",		ITEM_JUKEBOX,		TRUE	},  removed by SinaC 2003
    {   "instrument",           ITEM_INSTRUMENT,        TRUE    },
    {   "clothing",             ITEM_CLOTHING,          TRUE    },
    /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
    {   "grenade",              ITEM_GRENADE,           TRUE    },
    */
    // Added by SinaC 2000 for window
    {   "window",               ITEM_WINDOW,            TRUE    },
    // Added by SinaC 2000 for animate_skeleton spell from Tartarus
    //    {   "skeleton",             ITEM_SKELETON,          TRUE    },
    /* Removed by SinaC 2003
    // Added by SinaC 2001 for lever
    {   "lever",                ITEM_LEVER,             TRUE    },
    */
    // Added by SinaC 2003
    {   "template",             ITEM_TEMPLATE,          TRUE    },
    {   "saddle",               ITEM_SADDLE,            TRUE    },
    // Added by SinaC 2003 for climbing skill
    {   "rope",                 ITEM_ROPE,              TRUE    },
    {	NULL,			0,			0	}
};

struct flag_type extra_flags[] = {
    {	"glow",			ITEM_GLOW,		TRUE	},
    {	"hum",			ITEM_HUM,		TRUE	},
    {	"dark",			ITEM_DARK,		TRUE	},
// Have replaced ITEM_LOCK with ITEM_STAY_DEATH, SinaC 2000
//    {	"lock",			ITEM_LOCK,		TRUE	},
    {   "staydeath",            ITEM_STAY_DEATH,        TRUE    },

    {	"evil",			ITEM_EVIL,		TRUE	},
    {	"invis",		ITEM_INVIS,		TRUE	},
    {	"magic",		ITEM_MAGIC,		TRUE	},
    {	"nodrop",		ITEM_NODROP,		TRUE	},
    {	"bless",		ITEM_BLESS,		TRUE	},
    {	"antigood",		ITEM_ANTI_GOOD,		TRUE	},
    {	"antievil",		ITEM_ANTI_EVIL,		TRUE	},
    {	"antineutral",		ITEM_ANTI_NEUTRAL,	TRUE	},
    {	"noremove",		ITEM_NOREMOVE,		TRUE	},
    {	"inventory",		ITEM_INVENTORY,		TRUE	},
    {	"nopurge",		ITEM_NOPURGE,		TRUE	},
    {	"rotdeath",		ITEM_ROT_DEATH,		TRUE	},
    {	"visdeath",		ITEM_VIS_DEATH,		TRUE	},
    // Removed by SinaC 2001, use material table instead
    //{   "nonmetal",		ITEM_NONMETAL,		TRUE	},
// Added by SinaC 2001 for unique item
    {	"unique",		ITEM_UNIQUE,		TRUE	},

    {	"meltdrop",		ITEM_MELT_DROP,		TRUE	},
    {	"hadtimer",		ITEM_HAD_TIMER,		FALSE	},
    {	"sellextract",		ITEM_SELL_EXTRACT,      TRUE	},
    // Added by SinaC 2001
    {	"nosac",		ITEM_NOSAC,		TRUE	},

    {	"burnproof",		ITEM_BURN_PROOF,	TRUE	},
    {	"nouncurse",		ITEM_NOUNCURSE,		TRUE	},
    {	"donated",		ITEM_DONATED,		TRUE	},
    {   "nolocate",             ITEM_NOLOCATE,          TRUE    },
    // SinaC 2000 NoIdent item
    {   "noident",              ITEM_NOIDENT,           TRUE    },
  // No condition item SinaC 2001
    {   "nocond",               ITEM_NOCOND,            TRUE    },
    {	NULL,			0,			0	}
};


 struct flag_type container_flags[] = {
    {	"closeable",		CONT_CLOSEABLE,		TRUE	},
    {	"pickproof",		CONT_PICKPROOF,		TRUE	},
    {	"closed",		CONT_CLOSED,		TRUE	},
    {	"locked",		CONT_LOCKED,		TRUE	},
    {	"puton",		CONT_PUT_ON,		TRUE	},
    {	NULL,			0,		0	}
};

/****************************************************************************
                      ROM - specific tables:
 ****************************************************************************/


/*Oxtal */

struct flag_type afto_type[] = {
    {   "char",          AFTO_CHAR,            TRUE    },
    {   "object",        AFTO_OBJECT,          TRUE    },
    {   "objval",        AFTO_OBJVAL,          TRUE    },
    {   "weapon",        AFTO_WEAPON,          TRUE    },
    {   "room",          AFTO_ROOM,            TRUE    }, // SinaC 2003
    {   NULL,              0,                    0       }
};


// Modified by SinaC 2000, corresponding table in affects.C
struct flag_type attr_flags[] = {

  {"sex"             ,ATTR_sex,            TRUE},
  {"classes"         ,ATTR_classes,        TRUE},
  {"race"            ,ATTR_race,           TRUE},
  {"hp"              ,ATTR_max_hit,        TRUE},
  {"mana"            ,ATTR_max_mana,       TRUE},
  {"moves"           ,ATTR_max_move,       TRUE},
  {"immunities"      ,ATTR_imm_flags,      TRUE},
  {"resistances"     ,ATTR_res_flags,      TRUE},
  {"vulnerabilities" ,ATTR_vuln_flags,     TRUE},
  {"affects"         ,ATTR_affected_by,    TRUE},
  {"saving throw"    ,ATTR_saving_throw,   TRUE},
  {"hitroll"         ,ATTR_hitroll,        TRUE},
  {"damroll"         ,ATTR_damroll,        TRUE},
  {"ac pierce"       ,ATTR_ac0,            TRUE},
  {"ac bash"         ,ATTR_ac1,            TRUE},
  {"ac slash"        ,ATTR_ac2,            TRUE},
  {"ac magic"        ,ATTR_ac3,            TRUE},
  {"strength"        ,ATTR_STR,            TRUE},
  {"intelligence"    ,ATTR_INT,            TRUE},
  {"wisdom"          ,ATTR_WIS,            TRUE},
  {"dexterity"       ,ATTR_DEX,            TRUE},
  {"constitution"    ,ATTR_CON,            TRUE},
  {"parts"           ,ATTR_parts,          TRUE},
  {"form"            ,ATTR_form,           TRUE},
  {"size"            ,ATTR_size,           TRUE},
  {"dice number"     ,ATTR_DICE_NUMBER,    TRUE},
  {"dice type"       ,ATTR_DICE_TYPE,      TRUE},
  {"dice bonus"      ,ATTR_DICE_BONUS,     TRUE},
  {"dam type"        ,ATTR_dam_type,       TRUE},
  // Added by SinaC 2000
  {"affects2"        ,ATTR_affected2_by,   TRUE},
  // Added by SinaC 2001
  {"etho"            ,ATTR_etho,           TRUE},
  {"alignment"       ,ATTR_alignment,      TRUE},
  // Added by SinaC 2001 for mental user
  {"psp"            ,ATTR_max_psp,         TRUE},
  // Added by SinaC 2001 for disease like LYCANTHROPY, PLAGUE, ...
  //  {"disease"        ,ATTR_disease,         TRUE}, removed by SinaC 2003

  {"AC"              ,ATTR_allAC,          TRUE},
  {"N/A"             ,ATTR_NA,             TRUE},
  {NULL,             0,                    0       }
};

/* removed by SinaC 2003
// Added by SinaC 2001 for disease
struct flag_type disease_flags[] = {
  { "lycanthropy",   DIS_LYCANTHROPY,      TRUE },
  { "plague",        DIS_PLAGUE,           TRUE },
  { NULL,            0,                    0    }
};
*/

 struct flag_type ops_flags[] = {
    {   "or",     AFOP_OR,            TRUE    },
    {   "add",    AFOP_ADD,           TRUE    },
    {   "<-",     AFOP_ASSIGN,        TRUE    },
    // Added by SinaC 2001
    {   "nor",    AFOP_NOR,           TRUE    },
    {   NULL,     0,                  0       }
};


 struct flag_type ac_type[] = {
    {   "pierce",        AC_PIERCE,            TRUE    },
    {   "bash",          AC_BASH,              TRUE    },
    {   "slash",         AC_SLASH,             TRUE    },
    {   "exotic",        AC_EXOTIC,            TRUE    },
    {   NULL,              0,                    0       }
};


 struct flag_type size_flags[] = {
    {   "tiny",          SIZE_TINY,            TRUE    },
    {   "small",         SIZE_SMALL,           TRUE    },
    {   "medium",        SIZE_MEDIUM,          TRUE    },
    {   "large",         SIZE_LARGE,           TRUE    },
    {   "huge",          SIZE_HUGE,            TRUE    },
    {   "giant",         SIZE_GIANT,           TRUE    },
    // Added by SinaC 2003
    {   "no_size",       SIZE_NOSIZE,          TRUE    },
    {   NULL,              0,                    0       },
};


 struct flag_type weapon_class[] = {
    {   "exotic",      WEAPON_EXOTIC,          TRUE    },
    {   "sword",       WEAPON_SWORD,           TRUE    },
    {   "dagger",      WEAPON_DAGGER,          TRUE    },
    {   "spear",       WEAPON_SPEAR,           TRUE    },
    {   "mace",        WEAPON_MACE,            TRUE    },
    {   "axe",         WEAPON_AXE,             TRUE    },
    {   "flail",       WEAPON_FLAIL,           TRUE    },
    {   "whip",        WEAPON_WHIP,            TRUE    },
    {   "polearm",     WEAPON_POLEARM,         TRUE    },
    {   "staff(weapon)",WEAPON_STAFF,          TRUE    }, // Added by SinaC 2003
    {   "arrow",       WEAPON_ARROW,           TRUE    }, // For ranged attack
    {   "ranged",      WEAPON_RANGED,          TRUE    }, //  "    "      "
    {   NULL,          0,                      0       }
};


 struct flag_type weapon_type2[] = {
    {   "flaming",       WEAPON_FLAMING,       TRUE    },
    {   "frost",         WEAPON_FROST,         TRUE    },
    {   "vampiric",      WEAPON_VAMPIRIC,      TRUE    },
    {   "sharp",         WEAPON_SHARP,         TRUE    },
    {   "vorpal",        WEAPON_VORPAL,        TRUE    },
    {   "twohands",      WEAPON_TWO_HANDS,     TRUE    },
    {   "shocking",      WEAPON_SHOCKING,      TRUE    },
    {   "poison",        WEAPON_POISON,        TRUE    },
    // Added by SinaC 2001
    {   "holy",          WEAPON_HOLY,          TRUE    },
    // Added by SinaC 2001 for weighted flag
    {   "weighted",      WEAPON_WEIGHTED,      TRUE    },
    // Added by SinaC 2003
    {   "necrotism",     WEAPON_NECROTISM,     TRUE    },
    {   NULL,            0,                    0       }
};

 struct flag_type position_flags[] = {
    {   "dead",           POS_DEAD,            FALSE   },
    {   "mortal",         POS_MORTAL,          FALSE   },
    {   "incap",          POS_INCAP,           FALSE   },
    {   "paralyzed",      POS_PARALYZED,       TRUE    },    // Added by SinaC 2003
    {   "stunned",        POS_STUNNED,         FALSE   },
    {   "sleeping",       POS_SLEEPING,        TRUE    },
    {   "resting",        POS_RESTING,         TRUE    },
    {   "sitting",        POS_SITTING,         TRUE    },
    {   "fighting",       POS_FIGHTING,        FALSE   },
    {   "standing",       POS_STANDING,        TRUE    },
    {   NULL,              0,                    0       }
};

 struct flag_type portal_flags[]= {
    {   "normal_exit",   GATE_NORMAL_EXIT,     TRUE    },
    {   "no_curse",      GATE_NOCURSE,         TRUE    },
    {   "go_with",       GATE_GOWITH,          TRUE    },
    {   "buggy",         GATE_BUGGY,           TRUE    },
    {   "random",        GATE_RANDOM,          TRUE    },
    {   NULL,            0,                    0       }
};

 struct flag_type furniture_flags[]= {
    {  "stand_at",       STAND_AT,             TRUE    },
    {  "stand_on",       STAND_ON,             TRUE    },
    {  "stand_in",       STAND_IN,             TRUE    },
    {  "sit_at",         SIT_AT,               TRUE    },
    {  "sit_on",         SIT_ON,               TRUE    },
    {  "sit_in",         SIT_IN,               TRUE    },
    {  "rest_at",        REST_AT,              TRUE    },
    {  "rest_on",        REST_ON,              TRUE    },
    {  "rest_in",        REST_IN,              TRUE    },
    {  "sleep_at",       SLEEP_AT,             TRUE    },
    {  "sleep_on",       SLEEP_ON,             TRUE    },
    {  "sleep_in",       SLEEP_IN,             TRUE    },
    {  "put_at",         PUT_AT,               TRUE    },
    {  "put_on",         PUT_ON,               TRUE    },
    {  "put_in",         PUT_IN,               TRUE    },
    {  "put_inside",     PUT_INSIDE,           TRUE    },
    {  NULL,             0,                    0       }
};

// we doesn't need that now, check the file by SinaC 2000
struct flag_type *classes_flags;
struct flag_type *races_flags;

// Added by SinaC 2000 for object restrictions
 struct flag_type restr_flags[]= {
  { "strength",      RESTR_STR,      TRUE },
  { "intelligence",  RESTR_INT,      TRUE },
  { "wisdom",        RESTR_WIS,      TRUE },
  { "dexterity",     RESTR_DEX,      TRUE },
  { "constitution",  RESTR_CON,      TRUE },
  { "sex",           RESTR_SEX,      TRUE },
  { "classes",       RESTR_CLASSES,  TRUE },
  { "race",          RESTR_RACE,     TRUE },
  { "parts",         RESTR_PART,     TRUE },
  { "form",          RESTR_FORM,     TRUE },
  { "etho",          RESTR_ETHO,     TRUE },
  { "alignment",     RESTR_ALIGN,    TRUE },
// Added by SinaC 2000 for skill/spell restriction
  { "<skill/spell name>",        RESTR_ABILITY,   TRUE },
  { NULL,            0,            0    }
};

struct flag_type dam_type_flags[] = {
  {     "hit",		0, TRUE    	},  /*  0 */
  {	"slice", 	1, TRUE  	},	
  {     "stab",		2, TRUE  	},
  {	"slash",	3, TRUE  	},
  {	"whip",		4, TRUE        	},
  {     "claw",		5, TRUE 	        },  /*  5 */
  {	"blast",	6, TRUE 	        },
  {     "pound",	7, TRUE 	        },
  {	"crush",	8, TRUE 	        },
  {     "grep",		9, TRUE 	        },
  {	"bite",		10,TRUE 	},  /* 10 */
  {     "pierce",	11,TRUE 	},
  {     "suction",	12,TRUE 	},
  {	"beating",	13,TRUE 	},
  {     "digestion",	14,TRUE 	},
  {	"charge",	15,TRUE 	},  /* 15 */
  { 	"slap",		16,TRUE 	},
  {	"punch",	17,TRUE 	},
  {	"wrath",	18,TRUE 	},
  {	"magic",	19,TRUE 	},
  {     "divine power",	20,TRUE 	},  /* 20 */
  {	"cleave",	21,TRUE 	},
  {	"scratch",	22,TRUE 	},
  {     "peck",		23,TRUE 	},
  {     "peck",		24,TRUE 	},
  {     "chop",		25,TRUE 	},  /* 25 */
  {     "sting",	26,TRUE 	},
  {     "smash",	27,TRUE 	},
  {     "shocking bite",28,TRUE 	},
  {	"flaming bite", 29,TRUE	        },
  {	"freezing bite",30,TRUE	        },  /* 30 */
  {	"acidic bite", 	31,TRUE	        },
  {	"chomp",	32,TRUE	        },
  {  	"life drain",	33,TRUE	        },
  {     "thrust",	34,TRUE	        },
  {     "slime",	35,TRUE	        },  /* 35 */
  {	"shock",	36,TRUE	        },
  {     "thwack",	37,TRUE	        },
  {     "flame",	38,TRUE	        },
  {     "chill",	39,TRUE	        },  /* 39 */
  {   NULL,		0,		0		}
};

// Added by SinaC 2001 for material
struct flag_type *material_flags;

struct flag_type etho_flags[] = {
  {"chaotic", -1, TRUE},
  {"neutral", 0, TRUE},
  {"lawful", 1, TRUE},
  {NULL,0,0}
};


// Added by SinaC 2001 for room affects
struct flag_type room_attr_flags[] = {
  {"flags"            ,ATTR_flags,          TRUE},
  {"light"            ,ATTR_light,          TRUE},
  {"sector"           ,ATTR_sector,         TRUE},
  {"healrate"         ,ATTR_healrate,       TRUE},
  {"manarate"         ,ATTR_manarate,       TRUE},
  {"psprate"          ,ATTR_psprate,        TRUE},
  {"maxsize"          ,ATTR_maxsize,        TRUE},
  {NULL,              0,                    0       }
};



struct flag_type wear_flags[] = {
  {	"take",			ITEM_TAKE,		TRUE	},
  {	"finger",		ITEM_WEAR_FINGER,	TRUE	},
  {	"neck",			ITEM_WEAR_NECK,		TRUE	},
  {	"body",			ITEM_WEAR_BODY,		TRUE	},
  {	"head",			ITEM_WEAR_HEAD,		TRUE	},
  {	"legs",			ITEM_WEAR_LEGS,		TRUE	},
  {	"feet",			ITEM_WEAR_FEET,		TRUE	},
  {	"hands",		ITEM_WEAR_HANDS,	TRUE	},
  {	"arms",			ITEM_WEAR_ARMS,		TRUE	},
  {	"shield",		ITEM_WEAR_SHIELD,	TRUE	},
  {	"about",		ITEM_WEAR_ABOUT,	TRUE	},
  {	"waist",		ITEM_WEAR_WAIST,	TRUE	},
  {	"wrist",		ITEM_WEAR_WRIST,	TRUE	},
  {	"wield",		ITEM_WIELD,		TRUE	},
  {	"hold",			ITEM_HOLD,		TRUE	},
  // Moved in extra flags, SinaC 2001
  //    {   "nosac",		ITEM_NO_SAC,		TRUE	},
  {     "ear",                  ITEM_WEAR_EAR,          TRUE    },
  {     "eyes",                 ITEM_WEAR_EYES,         TRUE    },
  {     "wearfloat",            ITEM_WEAR_FLOAT,	TRUE	},
  // Added by SinaC 2001
  //{	"brand",		ITEM_BRAND,	TRUE	}, Removed by SinaC 2003
  /*    {   "twohands",            ITEM_TWO_HANDS,         TRUE    }, */
  {	NULL,			0,			0	}
};


/*
 * What is seen.
 */
struct flag_type wear_loc_strings[] = {
  {	"in the inventory",	WEAR_NONE,	TRUE	},
  {	"as a light",		WEAR_LIGHT,	TRUE	},
  {	"on the left finger",	WEAR_FINGER_L,	TRUE	},
  {	"on the right finger",	WEAR_FINGER_R,	TRUE	},
  {	"around the neck (1)",	WEAR_NECK_1,	TRUE	},
  {	"around the neck (2)",	WEAR_NECK_2,	TRUE	},
  {	"on the body",		WEAR_BODY,	TRUE	},
  {	"over the head",	WEAR_HEAD,	TRUE	},
  {	"on the legs",		WEAR_LEGS,	TRUE	},
  {	"on the feet",		WEAR_FEET,	TRUE	},
  {	"on the hands",		WEAR_HANDS,	TRUE	},
  {	"on the arms",		WEAR_ARMS,	TRUE	},
  {	"as a shield",		WEAR_SHIELD,	TRUE	},
  {	"about the shoulders",	WEAR_ABOUT,	TRUE	},
  {	"around the waist",	WEAR_WAIST,	TRUE	},
  {	"on the left wrist",	WEAR_WRIST_L,	TRUE	},
  {	"on the right wrist",	WEAR_WRIST_R,	TRUE	},
  {	"wielded",		WEAR_WIELD,	TRUE	},
  {	"held in the hands",	WEAR_HOLD,	TRUE	},
  {     "on the left ear",      WEAR_EAR_L,     TRUE    },
  {     "on the right ear",     WEAR_EAR_R,     TRUE    },
  {     "on the eyes",          WEAR_EYES,      TRUE    },
  {	"floating nearby",	WEAR_FLOAT,	TRUE	},
  // Added by SinaC 2000
  {     "secondary wielded",    WEAR_SECONDARY, TRUE    },
  //{   "brand mark",           WEAR_BRAND, TRUE    }, Removed by SinaC 2003
  {     "thirdly wielded",      WEAR_THIRDLY,   TRUE    },
  {     "fourthly wielded",     WEAR_FOURTHLY,  TRUE    },
  {	NULL,			0	      , 0	}
};


struct flag_type wear_loc_flags[] = {
  {	"none",		WEAR_NONE,	TRUE	},
  {	"light",	WEAR_LIGHT,	TRUE	},
  {	"lfinger",	WEAR_FINGER_L,	TRUE	},
  {	"rfinger",	WEAR_FINGER_R,	TRUE	},
  {	"neck1",	WEAR_NECK_1,	TRUE	},
  {	"neck2",	WEAR_NECK_2,	TRUE	},
  {	"body",		WEAR_BODY,	TRUE	},
  {	"head",		WEAR_HEAD,	TRUE	},
  {	"legs",		WEAR_LEGS,	TRUE	},
  {	"feet",		WEAR_FEET,	TRUE	},
  {	"hands",	WEAR_HANDS,	TRUE	},
  {	"arms",		WEAR_ARMS,	TRUE	},
  {	"shield",	WEAR_SHIELD,	TRUE	},
  {	"about",	WEAR_ABOUT,	TRUE	},
  {	"waist",	WEAR_WAIST,	TRUE	},
  {	"lwrist",	WEAR_WRIST_L,	TRUE	},
  {	"rwrist",	WEAR_WRIST_R,	TRUE	},
  {	"wielded",	WEAR_WIELD,	TRUE	},
  {	"hold",		WEAR_HOLD,	TRUE	},
  {     "lear",         WEAR_EAR_L,     TRUE    },
  {     "rear",         WEAR_EAR_R,     TRUE    },
  {     "eyes",         WEAR_EYES,      TRUE    },
  {	"floating",	WEAR_FLOAT,	TRUE	},
  // Added by SinaC 2000 was missing
  {     "wield2",       WEAR_SECONDARY, TRUE    },
  // Added by SinaC 2001
  //{	"brand",	WEAR_BRAND,	TRUE	}, Removed by SinaC 2003
  {     "wield3",       WEAR_THIRDLY,   TRUE    },
  {     "wield4",       WEAR_FOURTHLY,  TRUE    },
  {	NULL,		0,		0	}
};


/* damage classes */
struct flag_type dam_class_flags[] = {
  {   "none",        DAM_NONE,        TRUE },
  {   "bash",        DAM_BASH,        TRUE },
  {   "pierce",      DAM_PIERCE,      TRUE},
  {   "slash",       DAM_SLASH,       TRUE},
  {   "fire",        DAM_FIRE,        TRUE},
  {   "cold",        DAM_COLD,        TRUE},
  {   "lightning",   DAM_LIGHTNING,   TRUE},
  {   "acid",        DAM_ACID,        TRUE},
  {   "poison",      DAM_POISON,      TRUE},
  {   "negative",    DAM_NEGATIVE,    TRUE},
  {   "holy",        DAM_HOLY,        TRUE},
  {   "energy",      DAM_ENERGY,      TRUE},
  {   "mental",      DAM_MENTAL,      TRUE},
  {   "disease",     DAM_DISEASE,     TRUE},
  {   "drowning",    DAM_DROWNING,    TRUE},
  {   "light",       DAM_LIGHT,       TRUE},
  {   "other",       DAM_OTHER,       TRUE},
  {   "harm",        DAM_HARM,        TRUE},
  {   "charm",       DAM_CHARM,       TRUE},
  {   "sound",       DAM_SOUND,       TRUE},
  {   "daylight",    DAM_DAYLIGHT,    TRUE},
  // SinaC 2003
  {   "earth",       DAM_EARTH,       TRUE},
  {   "weaken",      DAM_WEAKEN,      TRUE},
  {NULL,0,0}
};


//struct flag_type mob_use_flags[] = {
//  { "none",          MOB_USE_NONE, TRUE },
//  { "combat",        MOB_USE_COMBAT, TRUE },
//  { "automatic",     MOB_USE_AUTOMATIC, TRUE },
//  { "normal",        MOB_USE_NORMAL, TRUE },
//  { NULL, 0, 0 }
//};
// SinaC 2003: bit vector, check explanations in merc.h
struct flag_type mob_use_flags[] = {
  { "charmed",       MOB_USE_CHARMED,   TRUE },
  { "script",        MOB_USE_SCRIPT,    TRUE },
  { "combat",        MOB_USE_COMBAT,    TRUE },
  { NULL, 0, 0 }
};

struct flag_type race_type_flags[] = {
  { "notavailable", RACE_NOTAVAILABLE, TRUE },
  { "creation", RACE_CREATION, TRUE },
  { "rebirth",  RACE_REBIRTH,  TRUE },
  { "remort",   RACE_REMORT,   TRUE },
  { "cursed",   RACE_CURSED,   TRUE },
  { NULL, 0, 0 }
};

struct flag_type log_flags[] = {
  { "normal", LOG_NORMAL, TRUE },
  { "always", LOG_ALWAYS, TRUE },
  { "never",  LOG_NEVER,  TRUE },
  { NULL, 0, 0 }
};

struct flag_type class_type_flags[] = {
  { "magic",  CLASS_MAGIC,  TRUE },
  { "mental", CLASS_MENTAL, TRUE },
  { "combat", CLASS_COMBAT, TRUE },
  { "wildable", CLASS_WILDABLE, TRUE },
  { NULL, 0, 0 }
};

struct flag_type class_choosable_flags[] = {
  { "never",       CLASS_CHOOSABLE_NEVER,       TRUE },
  { "yes",         CLASS_CHOOSABLE_YES,         TRUE },
  { "notcreation", CLASS_CHOOSABLE_NOTCREATION, TRUE },
  { NULL, 0, 0 }
};

struct flag_type ability_type_flags[] = {
  { "Skill",         TYPE_SKILL,  TRUE },
  { "Spell",         TYPE_SPELL,  TRUE },
  { "Power",         TYPE_POWER,  TRUE },
  { "Song",          TYPE_SONG,   TRUE },
  { "SpecialAffect", TYPE_AFFECT, TRUE },
  { NULL, 0, 0 }
};

struct flag_type affect_data_flags[] = {
  { "stay_death",     AFFECT_STAY_DEATH,      TRUE },
  { "nondispellable", AFFECT_NON_DISPELLABLE, TRUE },
  { "ability",        AFFECT_ABILITY,         TRUE },
  { "permanent",      AFFECT_PERMANENT,       TRUE },
  { "inherent",       AFFECT_INHERENT,        TRUE },
  { "invisible",      AFFECT_INVISIBLE,       TRUE },
  { "valid",          AFFECT_IS_VALID,        FALSE }, // set to 0 after an affect_remove
  { NULL, 0, 0 }
};

struct flag_type *brew_component_flags;

//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************
//************************************************************************************************************

// Added by SinaC 2003 for automatic do_wear, checking part
// FIXME: new part system, not bit anymore ... a char have maximum 30 parts (more/less?)
//    2*PART_HAND  <-- 2 arms --> 2 fingers :))
//    PART_NECK    <-- 1 head --> 1 neck
//    PART_TORSO   <-- 1 body --> 1 torso
//    PART_HEAD    <-- 1 head --> 1 head
//    PART_LEGS    <-- 2 legs --> 1 pair of leg
//    PART_FEET    <-- 2 legs --> 1 pair of foot
//    PART_HANDS   <-- 2 arms --> 1 pair of hands
//    PART_ARMS    <-- 2 arms --> 1 pair of arms
//    PART_TORSO   <-- 1 body --> 1 pair of shoulder  --|
//    PART_TORSO   <-- 1 body --> 1 waist             --|--> 2 different parts ?
//    2*PART_WRIST <-- 2 arms --> 2 wrists
//    2*PART_HAND  <-- 2 arms --> 2 hands: can wear 2 weapon, a weapon and a shield, 2 wands, ...
//    2*PART_EAR   <-- 1 head --> 2 ears
//    PART_EYE     <-- 1 head --> 1 pair of eyes
//
// PART_HAND  allows  WIELD or HOLD  and  FINGER
// PART_TORSO allows  WAIST and BODY and TORSO

#define PART_NONE (0)
// following part must be created
#define PART_HAND (0)
#define PART_WRIST (0)
#define PART_NECK (0)
#define PART_TORSO (0)

struct wear_item_type wear_item_table[] =
{
  {
    "in the inventory",
    NULL, NULL,
    ITEM_TAKE, PART_NONE
  },
  {
    "as a light",
    "$n lights $p and holds it.", "You light $p and hold it.",
    ITEM_TAKE, PART_HAND
  },
  {
    "on a finger",
    "$n wears $p on a finger.", "You wear $p on a finger.",
    ITEM_WEAR_FINGER, PART_HAND                                     //<-- new part
  },
  {
    "around the neck",
    "$n wears $p around $s neck.", "You wear $p around your neck.",
    ITEM_WEAR_NECK, PART_NECK
  },
  {
    "on the body",
    "$n wears $p on $s torso.", "You wear $p on your torso.",
    ITEM_WEAR_BODY, PART_TORSO
  },
  {
    "over the head",
    "$n wears $p on $s head.", "You wear $p on your head.",
    ITEM_WEAR_HEAD, PART_HEAD
  },
  {
    "on the legs",
    "$n wears $p on $s legs.", "You wear $p on your legs.",
    ITEM_WEAR_LEGS, PART_LEGS
  },
  {
    "on the feet",
    "$n wears $p on $s feet.", "You wear $p on your feet.",
    ITEM_WEAR_FEET, PART_FEET
  },
  {
    "on the hands",
    "$n wears $p on $s hands.", "You wear $p on your hands.",
    ITEM_WEAR_HANDS, PART_HANDS
  },
  {
    "on the arms",
    "$n wears $p on $s arms.", "You wear $p on your arms.",
    ITEM_WEAR_ARMS, PART_ARMS
  },
  {
    "as a shield",
    "$n wears $p as a shield.", "You wear $p as a shield.",
    ITEM_WEAR_SHIELD, PART_HAND
  },
  {
    "about the shoulders",
    "$n wears $p about $s torso.", "You wear $p about your torso.",
    ITEM_WEAR_ABOUT, PART_TORSO
  },
  {
    "around the waist",
    "$n wears $p about $s waist.", "You wear $p about your waist.",
    ITEM_WEAR_WAIST, PART_TORSO                                          //<-- new part
  },
  {
    "on the wrist",
    "$n wears $p around a wrist.", "You wear $p around a wrist.",
    ITEM_WEAR_WRIST, PART_WRIST                                          //<-- new part
  },
  {
    "wielded",
    "$n wields $p.", "You wield $p.",
    ITEM_WIELD, PART_HAND
  },
  {
    "held in the hands",
    "$n holds $p in $s hand.", "You hold $p in your hand.",
    ITEM_HOLD, PART_HAND
  },
  {
    "on the left ear",
    "$n wears $p on an ear.", "You wear $p on an ear.",
    ITEM_WEAR_EAR, PART_EAR
  },
  {
    "on the eyes",
    "$n wears $p on $s eyes.", "You wear $p on your eyes.",
    ITEM_WEAR_EYES, PART_EYE
  },
  {
    "floating nearby",
    "$n releases $p to float next to $m.", "You release $p and it floats next to you.",
    ITEM_WEAR_FLOAT, PART_NONE
  }
};
