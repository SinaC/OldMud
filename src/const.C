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

#include "spells_def.h"
#include "gsn.h"
#include "olc_value.h"
#include "combatabilities.h"
#include "noncombatabilities.h"
#include "raceabilities.h"
#include "const.h"
#include "power.h"
#include "wiznet.h"
#include "update_affect.h"
#include "wearoff_affect.h"
#include "song.h"
#include "interp.h"
#include "brew.h"
#include "classes.h"
#include "ranged.h"


// Added by SinaC 2001 for etho/align choose
const char* short_etho_align[MAX_ETHO_ALIGN] = 
{
  "CE",     // chaotic evil
  "NE",     // neutral evil
  "LE",     // lawful evil
  "CN",     // chaotic neutral
  "TN",     // neutral neutral ( true neutral )
  "LN",     // lawful neutral
  "CG",     // chaotic good
  "NG",     // neutral good
  "LG"      // lawful good
};

struct hometown_type	*hometown_table;
//{
//  {"Drow City",	5119,	5119},
//  {"Kerofk",	5669,	5669},
//  {"Midgaard",	ROOM_VNUM_TEMPLE, ROOM_VNUM_ALTAR},
//  {"New Thalos",9609,	9605},
//  {"Ofcol",	669,	669},
//  {"",		0,	0}
//};

/* item type list */
const struct item_type		item_table	[]	=
{
  {	ITEM_LIGHT,	"light"		},
  {	ITEM_SCROLL,	"scroll"	},
  {	ITEM_WAND,	"wand"		},
  {     ITEM_STAFF,	"staff"		},
  {     ITEM_WEAPON,	"weapon"	},
  {     ITEM_TREASURE,	"treasure"	},
  {     ITEM_ARMOR,	"armor"		},
  {	ITEM_POTION,	"potion"	},
  {	ITEM_CLOTHING,	"clothing"	},
  {     ITEM_FURNITURE,	"furniture"	},
  {	ITEM_TRASH,	"trash"		},
  {	ITEM_CONTAINER,	"container"	},
  {	ITEM_DRINK_CON, "drink"		},
  {	ITEM_KEY,	"key"		},
  {	ITEM_FOOD,	"food"		},
  {	ITEM_MONEY,	"money"		},
  {	ITEM_BOAT,	"boat"		},
  {	ITEM_CORPSE_NPC,"npc_corpse"	},
  {	ITEM_CORPSE_PC,	"pc_corpse"	},
  {     ITEM_FOUNTAIN,	"fountain"	},
  {	ITEM_PILL,	"pill"		},
  // SinaC 2000 : have replaced ITEM_PROTECT with ITEM_THROWING
  //    {	ITEM_PROTECT,	"protect"	},
  //{     ITEM_THROWING,  "throwing"      }, removed by SinaC 2003
  {	ITEM_MAP,	"map"		},
  {	ITEM_PORTAL,	"portal"	},
  {	ITEM_WARP_STONE,"warp_stone"	},
  // have replaced ITEM_ROOM_KEY with ITEM_COMPONENT  SinaC 2000
  //    {	ITEM_ROOM_KEY,	"room_key"	},
  {     ITEM_COMPONENT, "component"     },

  {	ITEM_GEM,	"gem"		},
  {	ITEM_JEWELRY,	"jewelry"	},
  //{     ITEM_JUKEBOX,	"jukebox"	},  removed by SinaC 2003
  // Added by SinaC 2000 for songs for bard
  {     ITEM_INSTRUMENT, "instrument"   },
  /* Removed by SinaC 2003, can be emulate with script
  // Added by SinaC 2000 for grenade
  {     ITEM_GRENADE,    "grenade"      },
  */
  // Added by SinaC 2000 for windows
  {     ITEM_WINDOW,     "window"       },
  // Added by SinaC 2000 for animate_skeleton spell from Tartarus
  //  {     ITEM_SKELETON,   "skeleton"     },
  // Added by SinaC 2001 for levers
  //{     ITEM_LEVER,      "lever"        },       Removed by SinaC 2003
  // Added by SinaC 2003
  {     ITEM_TEMPLATE,   "template"     },
  {     ITEM_SADDLE,     "saddle"       },
  {     ITEM_ROPE,       "rope"         },
  
  {   0,		NULL		}
};

/* weapon selection table */
const	struct	weapon_type	weapon_table	[]	=
{
  { "sword",	OBJ_VNUM_SCHOOL_SWORD,	WEAPON_SWORD,	&gsn_sword	},
  { "mace",	OBJ_VNUM_SCHOOL_MACE,	WEAPON_MACE,	&gsn_mace 	},
  { "dagger",	OBJ_VNUM_SCHOOL_DAGGER,	WEAPON_DAGGER,	&gsn_dagger	},
  { "axe",	OBJ_VNUM_SCHOOL_AXE,	WEAPON_AXE,	&gsn_axe	},
  { "spear",	OBJ_VNUM_SCHOOL_SPEAR,	WEAPON_SPEAR,	&gsn_spear	}, // was "staff" before
  { "flail",	OBJ_VNUM_SCHOOL_FLAIL,	WEAPON_FLAIL,	&gsn_flail	},
  { "whip",	OBJ_VNUM_SCHOOL_WHIP,	WEAPON_WHIP,	&gsn_whip	},
  { "polearm",	OBJ_VNUM_SCHOOL_POLEARM,WEAPON_POLEARM,	&gsn_polearm	},
  // Added by SinaC 2003
  { "staff(weapon)",	OBJ_VNUM_SCHOOL_STAFF,WEAPON_STAFF,	&gsn_staff	},
  //  { "arrow",    OBJ_VNUM_SCHOOL_ARROW,  WEAPON_ARROW,   &gsn_spear      }, // For ranged attack
  { "arrow",    OBJ_VNUM_SCHOOL_ARROW,  WEAPON_ARROW,   NULL      }, // For ranged attack
  { "ranged",   OBJ_VNUM_SCHOOL_RANGED, WEAPON_RANGED,  &gsn_bowfire    }, //  "    "      "
  { "hand_to_hand", 0,                  0,              &gsn_hand_to_hand }, // SinaC 2003
  { NULL,	0,				0,	NULL		}
};

// Added by SinaC 2001 for object condition
const char* cond_table[] =
{
  "perfect",
  "  good ",
  "awesome",
  " worse ",
  " defect",
  "  bad  ",
  " ruined",
  NULL
};

 
/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              = {
  {    "on",            WIZ_ON,         IM },
  {    "prefix",	WIZ_PREFIX,	IM },
  {    "ticks",         WIZ_TICKS,      IM },
  {    "logins",        WIZ_LOGINS,     IM },
  {    "sites",         WIZ_SITES,      L4 },
  {    "links",         WIZ_LINKS,      L7 },
  {    "newbies",	WIZ_NEWBIE,	IM },
  {    "spam",		WIZ_SPAM,	L5 },
  {    "deaths",        WIZ_DEATHS,     IM },
  {    "resets",        WIZ_RESETS,     L4 },
  {    "mobdeaths",     WIZ_MOBDEATHS,  L4 },
  {    "flags",	        WIZ_FLAGS,	L5 },
  {    "penalties",	WIZ_PENALTIES,	L5 },
  {    "saccing",	WIZ_SACCING,	L5 },
  {    "levels",	WIZ_LEVELS,	IM },
  {    "load",		WIZ_LOAD,	L2 },
  {    "restore",	WIZ_RESTORE,	L2 },
  {    "snoops",	WIZ_SNOOPS,	L2 },
  {    "switches",	WIZ_SWITCHES,	L2 },
  {    "secure",	WIZ_SECURE,	L1 },
  // Added by SinaC 2001
  {    "memcheck",      WIZ_MEMCHECK,   ML },
  // Added by SinaC 2001 for name acceptance
  {    "accept",        WIZ_NAMEACCEPT, IM },
  // Added by SinaC 2001 for rebirth
  {    "rebirth",       WIZ_REBIRTH,    IM },
  {    "bugs",          WIZ_BUGS,       ML },
  {    "various",       WIZ_VARIOUS,    L1 },
  {    "multiclass",    WIZ_MULTICLASS, L1 },
  {    "program",       WIZ_PROGRAM,    ML },

  {    NULL,		0,		0  }
};


/* attack table  -- not very organized :( */
const 	struct attack_type	attack_table	[MAX_DAMAGE_MESSAGE]	=
{
  { 	"none",		"hit",		-1		},  /*  0 */
  {	"slice",	"slice", 	DAM_SLASH	},	
  {     "stab",		"stab",		DAM_PIERCE	},
  {	"slash",	"slash",	DAM_SLASH	},
  {	"whip",		"whip",		DAM_SLASH	},
  {     "claw",		"claw",		DAM_SLASH	},  /*  5 */
  {	"blast",	"blast",	DAM_BASH	},
  {     "pound",	"pound",	DAM_BASH	},
  {	"crush",	"crush",	DAM_BASH	},
  {     "grep",		"grep",		DAM_SLASH	},
  {	"bite",		"bite",		DAM_PIERCE	},  /* 10 */
  {     "pierce",	"pierce",	DAM_PIERCE	},
  {     "suction",	"suction",	DAM_BASH	},
  {	"beating",	"beating",	DAM_BASH	},
  {     "digestion",	"digestion",	DAM_ACID	},
  {	"charge",	"charge",	DAM_BASH	},  /* 15 */
  { 	"slap",		"slap",		DAM_BASH	},
  {	"punch",	"punch",	DAM_BASH	},
  {	"wrath",	"wrath",	DAM_ENERGY	},
  {	"magic",	"magic",	DAM_ENERGY	},
  {     "divine",	"divine power",	DAM_HOLY	},  /* 20 */
  {	"cleave",	"cleave",	DAM_SLASH	},
  {	"scratch",	"scratch",	DAM_PIERCE	},
  {     "peck",		"peck",		DAM_PIERCE	},
  {     "peckb",	"peck",		DAM_BASH	},
  {     "chop",		"chop",		DAM_SLASH	},  /* 25 */
  {     "sting",	"sting",	DAM_PIERCE	},
  {     "smash",	"smash",	DAM_BASH	},
  {     "shbite",	"shocking bite",DAM_LIGHTNING	},
  {	"flbite",	"flaming bite", DAM_FIRE	},
  {	"frbite",	"freezing bite",DAM_COLD	},  /* 30 */
  {	"acbite",	"acidic bite", 	DAM_ACID	},
  {	"chomp",	"chomp",	DAM_PIERCE	},
  {  	"drain",	"life drain",	DAM_NEGATIVE	},
  {     "thrust",	"thrust",	DAM_PIERCE	},
  {     "slime",	"slime",	DAM_ACID	},  /* 35 */
  {	"shock",	"shock",	DAM_LIGHTNING	},
  {     "thwack",	"thwack",	DAM_BASH	},
  {     "flame",	"flame",	DAM_FIRE	},
  {     "chill",	"chill",	DAM_COLD	},  /* 39 */
  {   NULL,		NULL,		0		}
};

// race table
// Modified by SinaC 2000, in a file now
struct	race_type	*race_table;

// pc race table
// Modified by SinaC 2000, in a file now
struct	pc_race_type	*pc_race_table;

/*
 * Class table.
 */
// Modified by SinaC 2000, in files now
struct	class_type	*class_table;

// Super-race list
struct super_pc_race_type *super_pc_race_table;

// Magic school
struct magic_school_type *magic_school_table;


/*
 * Titles.
 */
// Modified by SinaC 2000, it's in a file now
const char ****			title_table;

/*
 * Attribute bonus tables.
 */
// Modified by SinaC 2001
const	struct	str_app_type	str_app		[31]		=
{
/*
  { -5, -4,   0,  0 },  // 0  
  { -5, -4,   3,  1 },  // 1  
  { -3, -2,   3,  2 },
  { -3, -1,  10,  3 },  // 3  
  { -2, -1,  25,  4 },
  { -2, -1,  55,  5 },  // 5  
  { -1,  0,  80,  6 },
  { -1,  0,  90,  7 },
  {  0,  0, 100,  8 },
  {  0,  0, 100,  9 },
  {  0,  0, 115, 10 }, // 10  
  {  0,  0, 115, 11 },
  {  0,  0, 130, 12 },
  {  0,  0, 130, 13 }, // 13  
  {  0,  1, 140, 14 },
  {  1,  1, 150, 15 }, // 15  
  {  1,  2, 165, 16 },
  {  2,  3, 180, 22 },
  {  2,  3, 200, 25 }, // 18  
  {  3,  4, 225, 30 },
  {  3,  5, 250, 35 }, // 20  
  {  4,  6, 275, 40 },
  {  4,  6, 300, 45 },
  {  5,  7, 325, 50 },
  {  5,  8, 350, 55 },
  {  6,  9, 380, 60 },  // 25
  {  8,  11, 420, 70 },   // 6,  9, 410, 65
  {  10, 13, 455, 80 },   // 7, 10, 440, 70
  {  12, 15, 490, 90 },   // 7, 10, 470, 75
  {  15, 18, 535, 100 },  // 8, 11, 510, 80
  {  20, 23, 600, 120 }   // 9, 12, 550, 85  // 30   
*/
  { -4,   0,  0 },  // 0  
  { -4,   3,  1 },  // 1  
  { -2,   3,  2 },
  { -1,  10,  3 },  // 3  
  { -1,  25,  4 },
  { -1,  55,  5 },  // 5  
  {  0,  80,  6 },
  {  0,  90,  7 },
  {  0, 100,  8 },
  {  0, 100,  9 },
  {  0, 115, 10 }, // 10  
  {  0, 115, 11 },
  {  0, 130, 12 },
  {  0, 130, 13 }, // 13  
  {  1, 140, 14 },
  {  1, 150, 15 }, // 15  
  {  2, 165, 16 },
  {  3, 180, 22 },
  {  3, 200, 25 }, // 18  
  {  4, 225, 30 },
  {  5, 250, 35 }, // 20  
  {  6, 275, 40 },
  {  6, 300, 45 },
  {  7, 325, 50 },
  {  8, 350, 55 },
  {  9, 380, 60 },  // 25
  {  11, 420, 70 },   // 6,  9, 410, 65
  {  13, 455, 80 },   // 7, 10, 440, 70
  {  15, 490, 90 },   // 7, 10, 470, 75
  {  18, 535, 100 },  // 8, 11, 510, 80
  {  23, 600, 120 }   // 9, 12, 550, 85  // 30   

};

const	struct	int_app_type	int_app		[31]		=
{
  {  3 },	/*  0 */
  {  5 },	/*  1 */
  {  7 },
  {  8 },	/*  3 */
  {  9 },
  { 10 },	/*  5 */
  { 11 },
  { 12 },
  { 13 },
  { 15 },
  { 17 },	/* 10 */
  { 19 },
  { 22 },
  { 25 },
  { 28 },
  { 31 },	/* 15 */
  { 34 },
  { 37 },
  { 40 },	/* 18 */
  { 43 },
  { 46 },	/* 20 */
  { 50 },
  { 53 },
  { 56 },
  { 60 },
  { 65 },	/* 25 */
  { 70 },
  { 75 },
  { 80 },
  { 85 },
  { 90 }      /* 30 */
};

const	struct	wis_app_type	wis_app		[31]		=
{
  { 0 },	/*  0 */
  { 0 },	/*  1 */
  { 0 },
  { 0 },	/*  3 */
  { 0 },
  { 1 },	/*  5 */
  { 1 },
  { 1 },
  { 1 },
  { 1 },
  { 1 },	/* 10 */
  { 1 },
  { 1 },
  { 1 },
  { 1 },
  { 2 },	/* 15 */
  { 2 },
  { 2 },
  { 3 },	/* 18 */
  { 3 },
  { 3 },	/* 20 */
  { 3 },
  { 4 },
  { 4 },
  { 4 },
  { 5 },	/* 25 */
  { 5 },
  { 5 },
  { 6 },
  { 6 },
  { 7 }         /* 30 */
};

// Modified by SinaC 2001
const	struct	dex_app_type	dex_app		[31]		=
{
  { -5,    60 },   /* 0 */
  { -5,   50 },   /* 1 */
  { -3,   50 },
  { -3,   40 },
  { -2,   30 },
  { -2,   20 },   /* 5 */
  { -1,   10 },
  { -1,    0 },
  {  0,    0 },
  {  0,    0 },
  {  0,    0 },   /* 10 */
  {  0,    0 },
  {  0,    0 },
  {  0,    0 },
  {  0,    0 },
  {  1, - 10 },   /* 15 */
  {  1, - 15 },
  {  2, - 20 },
  {  2, - 30 },
  {  3, - 40 },
  {  3, - 50 },   /* 20 */
  {  4, - 60 },
  {  4, - 75 },
  {  5, - 90 },
  {  5, -105 },
  {  6, -120 },    /* 25 */
  {  8, -150 }, // -135
  { 10, -180 }, // -150
  { 12, -220 }, // -165
  { 15, -260 }, // -180
  { 20, -320 }  // -200     /* 30 */
};

const	struct	con_app_type	con_app		[31]		=
{
  /*
  { -4, 20 },   //  0 
  { -3, 25 },   //  1
  { -2, 30 },
  { -2, 35 },	//  3 
  { -1, 40 },
  { -1, 45 },   //  5 
  { -1, 50 },
  {  0, 55 },
  {  0, 60 },
  {  0, 65 },
  {  0, 70 },   // 10 
  {  0, 75 },
  {  0, 80 },
  {  0, 85 },
  {  0, 88 },
  {  1, 90 },   // 15 
  {  2, 95 },
  {  2, 97 },
  {  3, 99 },   // 18 
  {  3, 99 },
  {  4, 99 },   // 20 
  {  4, 99 },
  {  5, 99 },
  {  6, 99 },
  {  7, 99 },
  {  8, 99 },    // 25 
  {  9, 99 }, // 9 before
  { 10, 99 }, // 9
  { 11, 99 }, // 10
  { 12, 99 }, // 11
  { 13, 99 }     // 30   // 11
  */
  // Added by SinaC 2001
  { -4, 20 },   /*  0  */
  { -3, 25 },   /*  1  */
  { -2, 30 },
  { -2, 35 },	/*  3  */
  { -1, 40 },
  { -1, 45 },   /*  5  */
  { -1, 50 },
  {  0, 55 },
  {  0, 60 },
  {  0, 65 },
  {  0, 70 },   /* 10  */
  {  0, 75 },
  {  0, 80 },
  {  0, 85 },
  {  0, 88 },
  {  1, 90 },   /* 15  */
  {  2, 95 },
  {  2, 97 },
  {  3, 99 },   /* 18  */
  {  3, 99 },
  {  4, 99 },   /* 20  */
  {  4, 99 },
  {  5, 99 },
  {  6, 99 },
  {  7, 99 },
  {  9, 99 },   /* 25  */
  { 10, 99 },//10   
  { 12, 99 },//11 
  { 16, 99 },//13 
  { 20, 99 },//15 
  { 24, 99 } //18    /* 30  */
};

/*
 * Liquid properties.
 */
// Modified by SinaC 2000, it's in a file now
struct	liq_type	*liq_table;

// Modified by SinaC 2000 for classes/skills/spells saved in files
//struct	skill_type	skill_table[MAX_SKILL];
struct	ability_type	*ability_table;


//struct	skill_type_init	skill_table_init[MAX_SKILL]	=
struct	ability_type_init	ability_table_init[]	= {
  // Magic spells.
  {
    "reserved",
    NULL,
    NULL,
    0
  },
  {
    "acid blast",
    (void*)spell_acid_blast, 
    NULL,
    5
  },
  {
    "armor",
    (void*)spell_armor,	
    &gsn_armor,
    5
  },
  {
    "bless",
    (void*)spell_bless,	
    &gsn_bless,
    0
  },
  {
    "blindness",
    (void*)spell_blindness,
    &gsn_blindness,
    0
  },

  {
    "burning hands",
    (void*)spell_burning_hands,	
    NULL,
    5
  },

  {
    "call lightning",	
    (void*)spell_call_lightning,	
    NULL,
    0
  },

  {   
    "calm",			
    (void*)spell_calm,		
    &gsn_calm,
    0
  },

  {
    "cancellation",		
    (void*)spell_cancellation,	
    NULL,
    0
  },

  {
    "cause critical",	
    (void*)spell_cause_critical,	
    NULL,
    0
  },

  {
    "cause light",		
    (void*)spell_cause_light,	
    NULL,
    0
  },

  {
    "cause serious",	
    (void*)spell_cause_serious,	
    NULL,
    0
  },

  {   
    "chain lightning",	
    (void*)spell_chain_lightning,	
    NULL,
    5
  }, 

  {
    "change sex",		
    (void*)spell_change_sex,	
    NULL,
    0
  },

  {
    "charm person",		
    (void*)spell_charm_person,	
    &gsn_charm_person,
    0
  },

  {
    "chill touch",		
    (void*)spell_chill_touch,	
    &gsn_chill_touch,
    5
  },

  {
    "colour spray",		
    (void*)spell_colour_spray,	
    NULL,
    5
  },

  {
    "continual light",	
    (void*)spell_continual_light,	
    NULL,
    0
  },

  {
    "control weather",	
    (void*)spell_control_weather,	
    NULL,
    0
  },

  {
    "create food",		
    (void*)spell_create_food,	
    NULL,
    0
  },

  {
    "create rose",		
    (void*)spell_create_rose,	
    NULL,
    0
  },  

  {
    "create spring",	
    (void*)spell_create_spring,	
    NULL,
    0
  },

  {
    "create water",		
    (void*)spell_create_water,	
    NULL,
    0
  },
  {
    "cure blindness",	
    (void*)spell_cure_blindness,	
    NULL,
    0
  },
  {
    "cure critical",	
    (void*)spell_cure_critical,	
    NULL,
    0
  },

  {
    "cure disease",		
    (void*)spell_cure_disease,	
    NULL,
    0
  },

  {
    "cure light",		
    (void*)spell_cure_light,	
    NULL,
    0
  },

  {
    "cure poison",		
    (void*)spell_cure_poison,	
    NULL,
    0
  },

  {
    "cure serious",		
    (void*)spell_cure_serious,	
    NULL,
    0
  },

  {
    "curse",		
    (void*)spell_curse,		
    &gsn_curse,
    0
  },

  {
    "demonfire",		
    (void*)spell_demonfire,	
    NULL,
    0
  },	

  {
    "detect evil",		
    (void*)spell_detect_evil,	
    NULL,
    0
  },

  {
    "detect good",          
    (void*)spell_detect_good,      
    NULL,
    0
  },

  {
    "detect hidden",	
    (void*)spell_detect_hidden,	
    NULL,
    0
  },

  {
    "detect invis",		
    (void*)spell_detect_invis,	
    NULL,
    0
  },

  {
    "detect magic",		
    (void*)spell_detect_magic,	
    NULL,
    0
  },

  {
    "detect poison",	
    (void*)spell_detect_poison,	
    NULL,
    0
  },

  {
    "dispel evil",		
    (void*)spell_dispel_evil,	
    NULL,
    0
  },

  {
    "dispel good",          
    (void*)spell_dispel_good,      
    NULL,
    0
  },

  {
    "dispel magic",		
    (void*)spell_dispel_magic,	
    NULL,
    0
  },

  {
    "earthquake",		
    (void*)spell_earthquake,	
    NULL,
    5
  },

  {
    "enchant armor",	
    (void*)spell_enchant_armor,	
    NULL,
    5
  },

  {
    "enchant weapon",	
    (void*)spell_enchant_weapon,	
    NULL,
    5
  },

  {
    "vampiric touch",	
    (void*)spell_old_energy_drain,	
    NULL,
    0
  },

  {
    "faerie fire",		
    (void*)spell_faerie_fire,	
    NULL,
    3
  },

  {
    "faerie fog",		
    (void*)spell_faerie_fog,	
    NULL,
    3
  },

  {
    "fireball",
    (void*)spell_fireball,		
    NULL,
    5
  },

  {
    "fireproof",		
    (void*)spell_fireproof,	
    NULL,
    0
  },

  {
    "flamestrike",		
    (void*)spell_flamestrike,	
    NULL,
    5,
    update_null,
    wearoff_flamestrike
  },

  {
    "fly",			
    (void*)spell_fly,		
    &gsn_fly,
    0
  },

  {
    "floating disc",	
    (void*)spell_floating_disc,	
    NULL,
    5
  },

  {
    "frenzy",               
    (void*)spell_frenzy,           
    &gsn_frenzy,
    0
  },

  {
    "gate",			
    (void*)spell_gate,		
    NULL,
    0
  },

  {
    "giant strength",	
    (void*)spell_giant_strength,	
    NULL,
    0
  },

  {
    "harm",			
    (void*)spell_harm,		
    NULL,
    0
  },
  
  {
    "haste",		
    (void*)spell_haste,		
    &gsn_haste,
    5
  },

  {
    "heal",			
    (void*)spell_heal,		
    NULL,
    0
  },
  
  {
    "heat metal",		
    (void*)spell_heat_metal,	
    NULL,
    0
  },

  {
    "holy word",		
    (void*)spell_holy_word,	
    NULL,
    0
  },

  {
    "identify",		
    (void*)spell_identify,		
    NULL,
    0
  },

  {
    "infravision",		
    (void*)spell_infravision,	
    NULL,
    0
  },

  {
    "invisibility",		
    (void*)spell_invis,		
    &gsn_invis,
    0
  },

  {
    "know alignment",	
    (void*)spell_know_alignment,	
    NULL,
    0
  },

  {
    "lightning bolt",	
    (void*)spell_lightning_bolt,	
    &gsn_lightning_bolt, // Added by SinaC 2003
    5
  },

  {
    "locate object",	
    (void*)spell_locate_object,	
    NULL,
    0
  },

  {
    "magic missile",	
    (void*)spell_magic_missile,	
    NULL,
    5
  },

  {
    "mass healing",		
    (void*)spell_mass_healing,	
    NULL,
    0
  },

  {
    "mass invis",		
    (void*)spell_mass_invis,	
    &gsn_mass_invis,
    0
  },

  {
    "nexus",                
    (void*)spell_nexus,            
    NULL,
    0
  },

  {
    "pass door",		
    (void*)spell_pass_door,	
    NULL,
    0
  },

  {
    "plague",		
    (void*)spell_plague,		
    &gsn_plague,
    0,
    // Added by SinaC 2003
    update_plague,
    wearoff_null
  },

  {
    "poison",		
    (void*)spell_poison,		
    &gsn_poison,
    5,
    // Added by SinaC 2003
    update_poison,
    wearoff_null
  },

  {
    "portal",               
    (void*)spell_portal,           
    NULL,
    0
  },

  {
    "protection evil",	
    (void*)spell_protection_evil,	
    &gsn_protection_evil,
    0
  },

  {
    "protection good",      
    (void*)spell_protection_good,  
    &gsn_protection_good,
    0
  },

  {
    "ray of truth",         
    (void*)spell_ray_of_truth,     
    NULL,
    0
  },

  {
    "recharge",		
    (void*)spell_recharge,		
    NULL,
    0
  },

  {
    "refresh",		
    (void*)spell_refresh,		
    NULL,
    0
  },

  {
    "remove curse",		
    (void*)spell_remove_curse,	
    NULL,
    0
  },

  {
    "sanctuary",		
    (void*)spell_sanctuary,	
    &gsn_sanctuary,
    0
  },

  {
    "shield",		
    (void*)spell_shield,		
    &gsn_shield,
    5
  },

  {
    "shocking grasp",	
    (void*)spell_shocking_grasp,	
    NULL,
    5
  },

  {
    "sleep",		
    (void*)spell_sleep,		
    &gsn_sleep,
    5
  },

  {
    "slow",                 
    (void*)spell_slow,             
    NULL,
    5
  },

  {
    "stone skin",		
    (void*)spell_stone_skin,	
    NULL,
    0
  },

  {
    "summon",		
    (void*)spell_summon,		
    NULL,
    0
  },

  {
    "teleport",		
    (void*)spell_teleport,		
    NULL,
    0
  },

  {
    "ventriloquate",	
    (void*)spell_ventriloquate,	
    NULL,
    0
  },

  {
    "weaken",		
    (void*)spell_weaken,		
    &gsn_weaken,
    5
  },

  // Dragon breath
  {
    "acid breath",		
    (void*)spell_acid_breath,	
    &gsn_acid_breath,
    0
  },

  {
    "fire breath",		
    (void*)spell_fire_breath,	
    &gsn_fire_breath,
    0
  },

  {
    "frost breath",		
    (void*)spell_frost_breath,	
    NULL,
    0
  },

  {
    "gas breath",		
    (void*)spell_gas_breath,	
    NULL,
    0
  },

  {
    "lightning breath",	
    (void*)spell_lightning_breath,	
    NULL,
    0
  },

  //Spells for mega1.are from Glop/Erkenbrand.
  {
    "general purpose",      
    (void*)spell_general_purpose,  
    NULL,
    0
  },
 
  {
    "high explosive",       
    (void*)spell_high_explosive,   
    NULL,
    0
  },

  { 
    "black lotus",          
    (void*)spell_black_lotus,      
    NULL,
    0
  },
    
  {
    "mindfear",
    (void*)power_mind_fear,             
    NULL,
    0
  },    
    
  {
    "resurrect",           	
    (void*)spell_resurrect,       	
    NULL,
    0
  },
    
  {
    "acid arrow",          	
    (void*)spell_acid_arrow,     	
    NULL,
    0
  },
    
  {
    "flame arrow",         	
    (void*)spell_flame_arrow,    	
    NULL,
    0
  },
    
  {
    "create flame blade",          	
    (void*)spell_create_sword,     	
    NULL,
    0
  },
    
  {
    "create ice knife",            	
    (void*)spell_create_dagger,    	
    NULL,
    0
  },
    
  {
    "aid",		      	
    (void*)spell_aid,	      	
    NULL,
    0
  },
    
  {
    "blur",  	      	
    (void*)spell_blur,            	
    NULL,
    0
  },    
  // For MindMage - Psionicist - Neuromancer
  {
    "death field",         	
    (void*)power_death_field,     	
    NULL,
    0
  },

  {
    "lend health",         	
    (void*)power_lend_health,     	
    NULL,
    0
  },
    
  {
    "levitation",          	
    (void*)power_levitation,      	
    &gsn_levitation,
    0
  },
    
  {
    "dizziness",           	
    (void*)power_dizziness,       	
    NULL,
    0
  },
    
  {
    "hesitation",          	
    (void*)power_hesitation,      	
    NULL,
    0
  },
    
  {
    "trouble",             	
    (void*)power_trouble,         	
    NULL,
    0
  },
    
  {
    "minor pain",          	
    (void*)power_minor_pain,     	
    NULL,
    0
  },
    
  {
    "major pain",          	
    (void*)power_major_pain,     	
    NULL,
    0
  },
    
  {
    "psychic impact",      	
    (void*)power_psychic_impact, 	
    NULL,
    0
  },
    
  {
    "combat mind",	      	
    (void*)power_combat_mind,     	
    NULL,
    0
  },    
    
  {
    "adrenaline control",	
    (void*)power_adrenaline_control,
    NULL,
    0
  },

  {
    "agitation",		
    (void*)power_agitation,        
    NULL,
    0
  },

  {
    "aura sight",		
    (void*)power_aura_sight,       
    NULL,
    0
  },

  {
    "awe",			
    (void*)power_awe,              
    NULL,
    0
  },

  {
    "ballistic attack",	
    (void*)power_ballistic_attack, 
    NULL,
    0
  },

  {
    "biofeedback",		
    (void*)power_biofeedback,      
    NULL,
    0
  },

  {
    "cell adjustment",	
    (void*)power_cell_adjustment,  
    NULL,
    0
  },

  {
    "complete healing",	
    (void*)power_complete_healing, 
    NULL,
    0
  },

  {
    "control flames",	
    (void*)power_control_flames,   
    NULL,
    0
  },

  {
    "create sound",		
    (void*)power_create_sound,     
    NULL,
    0
  },

  {
    "detonate",		
    (void*)power_detonate,         
    NULL,
    0
  },

  {
    "disintegrate",		
    (void*)power_disintegrate,     
    NULL,
    0
  },

  {
    "displacement",		
    (void*)power_displacement,     
    NULL,
    0
  },

  {
    "ectoplasmic form",	
    (void*)power_ectoplasmic_form, 
    NULL,
    0
  },

  {
    "ego whip",		
    (void*)power_ego_whip,         
    NULL,
    0
  },

  {
    "energy containment",	
    (void*)power_energy_containment,
    NULL,
    0
  },

  {
    "enhanced strength",	
    (void*)power_enhanced_strength,
    NULL,
    0
  },

  {
    "flesh armor",		
    (void*)power_flesh_armor,      
    NULL,
    0
  },

  {
    "inflict pain",		
    (void*)power_inflict_pain,     
    NULL,
    0
  },

  {
    "intellect fortress",	
    (void*)power_intellect_fortress,
    NULL,
    0
  },

  {
    "mental barrier",	
    (void*)power_mental_barrier,   
    NULL,
    0
  },

  {
    "mind thrust",		
    (void*)power_mind_thrust,      
    NULL,
    0
  },

  {
    "project force",	
    (void*)power_project_force,    
    NULL,
    0
  },

  {
    "neuro blast",		
    (void*)power_neuro_blast,    	
    NULL,
    0
  },

  {
    "neuro crush",		
    (void*)power_neuro_crush,    
    NULL,
    0
  },

  {
    "neuro drain",		
    (void*)power_neuro_drain,    
    NULL,
    0
  },

  {
    "neuro healing",	
    (void*)power_neuro_healing,  
    NULL,
    0
  },

  {
    "share strength",	
    (void*)power_share_strength,   
    NULL,
    0
  },

  {
    "thought shield",	
    (void*)power_thought_shield,
    NULL,
    0
  },

  {
    "ultrablast",		
    (void*)power_ultrablast,       
    NULL,
    0
  },

  {
    "raw flesh",		
    (void*)power_raw_flesh,        
    NULL,
    0
  },

  // Added by SinaC 2001, dunno why that spell was coded but not used
  {
    "domination",
    (void*)power_domination,
    NULL,
    0
  },

  // combat and weapons skills
  {
    "axe",			
    NULL,             
    &gsn_axe,
    0
  },

  {
    "dagger",               
    NULL,             
    &gsn_dagger,
    0
  },
 
  {
    "flail",		
    NULL,             
    &gsn_flail,
    0
  },

  {
    "mace",			
    NULL,             
    &gsn_mace,
    0
  },

  {
    "polearm",		
    NULL,             
    &gsn_polearm,
    0
  },
    
  {
    "shield block",		
    NULL,		
    &gsn_shield_block,
    0
  },
 
  {
    "spear",		
    NULL,             
    &gsn_spear,
    0
  },

  {
    "sword",		
    NULL,             
    &gsn_sword,
    0
  },

  {
    "whip",			
    NULL,             
    &gsn_whip,
    0
  },

  { // Added by SinaC 2003
    "staff(weapon)",
    NULL,
    &gsn_staff,
    0
  },

  {
    "backstab",             
    (void*)do_backstab,
    &gsn_backstab,
    0
  },

  {
    "bash",	
    (void*)do_bash,
    &gsn_bash,
    3              // bash has 3 levels
  },

  {
    "berserk",		
    (void*)do_berserk,
    &gsn_berserk,
    0
  },

  {
    "dirt kicking",		
    (void*)do_dirt,
    &gsn_dirt,
    0
  },

  {
    "disarm",               
    (void*)do_disarm,
    &gsn_disarm,
    0
  },

  {
    "dodge",                
    NULL,             
    &gsn_dodge,
    0
  },
  {
    "enhanced damage",      
    NULL,             
    &gsn_enhanced_damage,
    2 // enhanced damage has 2 levels now
  },
  {
    "envenom",		
    (void*)do_envenom,
    &gsn_envenom,
    0
  },
  {
    "hand to hand",		
    NULL,		
    &gsn_hand_to_hand,
    5
  },
  {
    "kick",                 
    (void*)do_kick,
    &gsn_kick,
    0
  },
  {
    "parry",                
    NULL,             
    &gsn_parry,
    0
  },
  {
    "rescue",               
    (void*)do_rescue,
    &gsn_rescue,
    0
  },
  {
    "trip",			
    (void*)do_trip,
    &gsn_trip,
    0
  },
  {
    "second attack",        
    NULL,             
    &gsn_second_attack,
    0
  },
  {
    "third attack",         
    NULL,             
    &gsn_third_attack,
    0
  },
  // added by SinaC 2000
  {
    "fourth attack",        
    NULL,             
    &gsn_fourth_attack,
    0
  },
  {
    "fifth attack",         
    NULL,             
    &gsn_fifth_attack,
    0
  },
  {
    "sixth attack",         
    NULL,             
    &gsn_sixth_attack,
    0
  },
  {
    "seventh attack",       
    NULL,             
    &gsn_seventh_attack,
    0
  },
  {
    "dual wield",         
    NULL,           
    &gsn_dual_wield,
    2
  },
  {
    "circle",         
    (void*)do_circle,
    &gsn_circle,
    0
  },
  // end
  // Added by Sinac 1997
  {
    "sharpen",		
    (void*)do_sharpen,
    &gsn_sharpen,
    0
  },
  {
    "roundhouse kick",      
    (void*)do_roundhousekick,
    &gsn_roundhousekick,
    0
  },
  // Removed by SinaC 2001
  //{
  //  "enhanced punch",       
  //  NULL,             
  //  &gsn_enhanced_hand     
  //},
  // by SinaC 2000
  {
    "tailsweep",      
    (void*)do_tail,
    &gsn_tail,
    0
  },
  {
    "fade",       
    NULL,    
    &gsn_fade,
    0
  },
  {
    "counter",     
    NULL,    
    &gsn_counter,
    0
  },
  {
    "whirlwind",   
    (void*)do_whirlwind,
    &gsn_whirlwind,
    0
  },
  {
    "critical strike",
    NULL,       
    &gsn_critical_strike,
    0
  },
  {
    "beacon",		
    (void*)spell_beacon,           
    NULL,
    0
  },
  {
    "fumble",		
    (void*)spell_fumble,           
    NULL,
    0
  },
  {
    "find familiar",         
    (void*)do_familiar,
    &gsn_familiar,
    0
  },
  {
    "ionwave",		
    (void*)spell_ionwave,          
    NULL,
    0
  },
  {
    "banshee scream",	
    (void*)spell_banshee_scream,   
    NULL,
    0
  },
  {
    "vaccine",		
    (void*)spell_vaccine,          
    NULL,
    0
  },
  {
    "locate person",	
    (void*)spell_locate_person,    
    NULL,
    0
  },
  /* Removed by SinaC 2003
  {
    "dartthrow",            
    (void*)do_throwing,
    &gsn_throwing,
    0
  },
  */
  {
    "meditate",               
    (void*)do_meditate,
    &gsn_meditate,
    0
  },
  {
    "silence",		
    (void*)spell_silence,           
    NULL,
    0
  },
  {
    "deathgrip",            
    (void*)do_deathgrip,
    &gsn_deathgrip,
    0
  },
  {
    "blind fighting",  
    NULL,        
    &gsn_blindfight,
    0
  },
  {
    "butcher",         
    (void*)do_butcher,
    &gsn_butcher,
    0
  },
  {
    "acid rain",		
    (void*)spell_acid_rain,          
    NULL,
    0
  },
  {
    "lay hands",		
    (void*)spell_layhands,		
    NULL,
    0
  },
  {
    "jades lust",		
    (void*)spell_jades_lust,       
    NULL,
    0
  },
  {
    "grip",               
    NULL,           
    &gsn_grip,
    0
  },
  {
    "lure",               
    (void*)do_lure,
    &gsn_lure,
    0
  },
  {
    "pillify",            
    (void*)do_pillify,
    &gsn_pillify,
    0
  },
  {
    "bladethirst",        
    (void*)do_bladethirst,
    &gsn_bladethirst,
    0
  },
  {
    "summon lesser golem",
    (void*)spell_summon_lgolem,  
    NULL,
    0
  },
  {
    "summon greater golem",
    (void*)spell_summon_ggolem,   
    NULL,
    0
  },
  {
    "soul blade",   
    (void*)spell_soul_blade,
    NULL,
    0
  },
  {
    "regeneration",
    (void*)spell_regeneration,	
    NULL,
    0
  },
  {
    "mind meld",	
    (void*)spell_mind_meld,  
    NULL,
    0
  },
  {
    "psi twister",	
    (void*)spell_psi_twister,
    NULL,
    0
  },
  {
    "exorcism",	
    (void*)spell_exorcism,   
    NULL,
    0
  },
  // Spells from Tartarus
  {
    "utter heal",	
    (void*)spell_utter_heal,
    NULL,
    0	
  },
  {
    "light shield",	
    (void*)spell_lightshield,
    NULL,
    0
  },
  {
    "dark shield",	
    (void*)spell_darkshield,
    NULL,
    0
  },
  {
    "summon flesh golem",
    (void*)spell_flesh_golem,
    NULL,
    0
  },
  {
    "fire and ice",	
    (void*)spell_fire_and_ice,
    NULL,
    0
  },
  {
    "shock sphere",		
    (void*)spell_shock_sphere,
    NULL,
    0
  },
  {
    "cremate",		
    (void*)spell_cremate,
    NULL,
    0
  },
  {
    "sun bolt",		
    (void*)spell_sunbolt,
    NULL,
    0
  },
  {
    "frost bolt",		
    (void*)spell_frostbolt,
    NULL,
    0
  },
  {
    "ice lance",		
    (void*)spell_icelance,
    NULL,
    0
  },
  {
    "life bane",		
    (void*)spell_lifebane,
    NULL,
    0
  },
  {
    "old death spell",		
    (void*)spell_old_deathspell,
    NULL,
    0
  },
  {
    "energy drain",		
    (void*)spell_energy_drain,
    NULL,
    5
  },
  {
    "blade barrier",		
    (void*)spell_blade_barrier, 
    NULL,
    0
  },
  {
    "holy fire",		
    (void*)spell_holy_fire,
    NULL,
    0
  },
  {
    "revolt",		
    (void*)spell_revolt, 
    NULL,
    0
  },
  {
    "firestream",	
    (void*)spell_firestream,
    NULL,
    0
  },
  {
    "wither",	
    (void*)spell_wither,
    NULL,
    0
  },
  {
    "hand of vengeance",
    (void*)spell_hand_of_vengeance,
    NULL,
    0
  },
  {
    "iceball",		
    (void*)spell_iceball,
    NULL,
    0
  },
  {
    "old cone of cold",
    (void*)spell_cone_of_cold2,
    NULL,
    0
  },
  {
    "spiritblade",	
    (void*)spell_spiritblade,
    NULL,
    0
  },
  {
    "drain",
    (void*)spell_drain,
    NULL,
    0
  },
  {
    "earthmaw",
    (void*)spell_earthmaw,
    NULL,
    0
  },
  {
    "windwall",
    (void*)spell_windwall,
    NULL,
    0
  },
  {
    "web",
    (void*)spell_web,
    NULL,
    0
  },
  {
    "old turn undead",
    (void*)spell_old_turn_undead,
    NULL,
    0
  },
  {
    "channel",
    (void*)spell_channel,
    NULL,
    0
  },
  {
    "true sight",
    (void*)spell_true_sight,
    NULL,
    0
  },
  {
    "dark wrath",
    (void*)spell_dark_wrath,
    NULL,
    0
  },
  {
    "wrath",		
    (void*)spell_wrath,	
    NULL,
    0
  },
  {
    "higher holy word",
    (void*)spell_new_holy_word,
    NULL,
    0
  },
  {
    "higher demonfire",
    (void*)spell_new_demonfire,
    NULL,
    0
  },	
  {
    "concatenate",		
    (void*)spell_concatenate,	
    NULL,
    0
  },
  {
    "word of kill",		
    (void*)spell_power_word_kill,	
    NULL,
    0
  },
  {
    "evil eye",		
    (void*)spell_evil_eye,	
    NULL,
    0
  },
  {
    "preserve",
    (void*)spell_preserve,
    NULL,
    0
  },
  {
    "decay corpse",
    (void*)spell_decay_corpse,
    NULL,
    0
  },
  {
    "mummify",
    (void*)spell_mummify,
    &gsn_mummify,
    0
  },
  {
    "animate skeleton",
    (void*)spell_animate_skeleton,
    &gsn_animate_skeleton,
    0
  },
  {
    "animate dead",
    (void*)spell_animate_dead,
    NULL,//&gsn_animate_dead,
    5
  },
  {
    "restoration",
    (void*)spell_restoration,
    NULL,
    0
  },

  {
    "stun",
    NULL,
    &gsn_stun,
    0
  },
  {
    "cleave", 
    (void*)do_cleave,
    &gsn_cleave,
    0
  },
  {
    "herb", 
    (void*)do_herb,
    &gsn_herb,
    0
  },
  {
    "bandage",
    (void*)do_bandage,
    &gsn_bandage,
    0
  },
  {
    "nerve",
    (void*)do_nerve,
    &gsn_nerve,
    0
  },
  {
    "endure",  
    (void*)power_endure,
    &gsn_endure,
    0
  }, 
  {
    "shield cleave",
    (void*)do_shield_cleave,
    &gsn_shield_cleave,
    0
  },
  {
    "findwater",       
    (void*)do_find_water,
    &gsn_find_water,
    0
  },    
  {
    "rearkick",        
    (void*)do_rear_kick,
    &gsn_rear_kick,
    0
  },
  {
    "forage",       
    (void*)do_forage,
    &gsn_forage,
    0
  },    
  {
    "lash",         
    (void*)do_lash,
    &gsn_lash,
    0
  },
  {
    "pugil",        
    (void*)do_pugil,
    &gsn_pugil,
    0
  }, 
  {
    "spike",      
    (void*)do_spike,
    &gsn_spike,
    0
  },    
  {
    "warcry",       
    (void*)do_warcry,
    &gsn_warcry,
    0
  },    
  {
    "crush",      
    (void*)do_crush,
    &gsn_crush,
    0
  },    
  // Converted in a spell now
//  {
//    "barkskin",     
//    (void*)do_barkskin,
//    &gsn_barkskin,
//    0
//  },
  // Added for thieves
  {
    "freeflee",
    NULL,
    &gsn_freeflee,
    0
  },
  // Added by SinaC 2001 for Dracon 1 spell
  {
    "stake",
    (void*)do_stake,
    &gsn_stake,
    0
  },
  {
    "daemonic aid",
    (void*)spell_daemonic_aid,
    NULL,
    0
  },
  {
    "daemonic potency",
    (void*)spell_daemonic_potency,
    NULL,
    0
  },
  {
    "daemonic carapace",
    (void*)spell_daemonic_carapace,
    NULL,
    0
  },
  {
    "repulsion",
    (void*)spell_repulsion,
    &gsn_repulsion,
    0
  },
  {
    "divine intervention",
    (void*)spell_divine_intervention,
    NULL,
    0
  },
  {
    "holy armor",
    (void*)spell_holy_armor,
    NULL,
    0
  },
  {
    "holy sword",
    (void*)spell_holy_sword,
    NULL,
    0
  },
  {
    "unholy blade",
    (void*)spell_unholy_blade,
    NULL,
    0
  },
  {
    "resist fire",
    (void*)spell_resist_fire,
    NULL,
    0
  },
  {
    "vampiric blade",
    (void*)spell_drain_blade,
    &gsn_drain_blade,
    0
  },
  {
    "shock blade",
    (void*)spell_shocking_blade,
    &gsn_shocking_blade,
    0
  },
  {
    "flame blade",
    (void*)spell_flame_blade,
    &gsn_flame_blade,
    0
  },
  {
    "frost blade",
    (void*)spell_frost_blade,
    &gsn_frost_blade,
    0
  },  
  {
    "align detect",
    (void*)do_align_detect,
    &gsn_align_detect,
    0
  },
  {
    "magic detect",
    (void*)do_magic_detect,
    &gsn_magic_detect,
    0
  },
  {
    "detect exits",
    (void*)do_exits_detect,
    &gsn_exits_detect,
    0
  },
  {
    "poison detect",
    (void*)do_poison_detect,
    &gsn_poison_detect,
    0
  },
  {
    "entangle",
    (void*)spell_entangle,
    &gsn_entangle,
    5
  },
  {
    "wraithform",
    (void*)spell_wraithform,
    &gsn_wraithform,
    0
  },

  // Added by SinaC 2001 for banshee
  {
    "wail",
    (void*)do_wail,
    &gsn_wail,
    0
  },
  // Added by SinaC 2001 for dwarf
  {
    "resize",
    (void*)do_resize,
    &gsn_resize,
    0
  },
  {
    "forge",
    (void*)do_forge,
    &gsn_forge,
    0
  },
  {
    "vorpalize",
    (void*)do_vorpalize,
    &gsn_vorpalize,
    0
  },

  {
    "iron hand",
    (void*)power_iron_hand,
    &gsn_iron_hand,
    0
  },

  {
    "energy fist",
    (void*)power_energy_fist,
    &gsn_energy_fist,
    0
  },

  {
    "burning fist",
    (void*)power_burning_fist,
    &gsn_burning_fist,
    0
  },
  
  {
    "revitalize",
    (void*)spell_revitalize,
    NULL,
    0
  },

  // Added for songs for bard, not a skill anymore SinaC 2003
  {
    "sing",
    (void*)do_song,
    &gsn_song,
    0
  },
  // By oxtal
  {
    "throw",      
    (void*)do_throw,
    &gsn_throw,
    0
  },

  // non-combat skills

    // Fire Skill by Seytan 1997
  {
    "campfire",  
    (void*)do_fire,
    &gsn_fire,
    0
  },
  {
    "hunt",      
    (void*)do_hunt,
    &gsn_hunt,
    0
  },
  {
    "fast healing",
    NULL,
    &gsn_fast_healing,
    0
  },
  {
    "haggle",		
    NULL,		
    &gsn_haggle,
    0
  },
  {
    "hide",			
    (void*)do_hide,
    &gsn_hide,
    0
  },
  {
    "lore",			
    (void*)do_lore,
    &gsn_lore,
    0
  },
  {
    "meditation",		
    NULL,		
    &gsn_meditation,
    2
  },
  {
    "peek",			
    NULL,		
    &gsn_peek,
    0
  },
  {
    "pick lock",		
    (void*)do_pick,
    &gsn_pick_lock,
    0
  },
  {
    "sneak",		
    (void*)do_sneak,
    &gsn_sneak,
    0
  },
  {
    "steal",		
    (void*)do_steal,
    &gsn_steal,
    0
  },
  {
    "scrolls",		
    (void*)do_recite,
    &gsn_scrolls,
    0
  },
  {
    "staves",		
    (void*)do_brandish,
    &gsn_staves,
    0
  },
  {
    "wands",		
    (void*)do_zap,
    &gsn_wands,
    0
  },
  {
    "recall",		
    (void*)do_recall,
    &gsn_recall,
    0
  },
  // By Oxtal and SinaC 2001, just for the fun of it!
  {
    "bird form",        
    (void*)spell_bird_form,    
    NULL,
    0
  },
  {
    "arcane concordance",
    (void*)spell_arcane_concordance,
    NULL,
    0
  },

  {
    "speedup",
    (void*)do_speedup,
    &gsn_speedup,
    0
  },

  {
    "invisible",
    (void*)do_invisible,
    &gsn_invisible,
    0
  },

  {
    "death breath",
    (void*)spell_death_breath,
    NULL,
    0
  },

  {
    "water breath",
    (void*)spell_water_breath,
    NULL,
    0
  },

  {
    "walk on water",
    (void*)spell_walk_on_water,
    NULL,
    0
  },

  {
    "feed",
    (void*)do_feed,
    &gsn_feed,
    0
  },

  {
    "war drums",
    (void*)do_war_drums,
    &gsn_war_drums,
    0
  },

  {
    "shroud",
    (void*)spell_shroud,
    &gsn_shroud,
    0
  },

  {
    "repair",
    (void*)do_repair,
    &gsn_repair,
    0
  },

  {
    "morph",
    (void*)do_morph,
    &gsn_morph,
    0,
    // Added by SinaC 2003
    update_null,
    wearoff_morph
  },

  {
    "manashield",
    (void*)spell_manashield,
    &gsn_manashield,
    0
  },

  {
    "magic mirror",
    (void*)spell_magic_mirror,
    NULL,
    0
  },
  {
    "prismatic spray",
    (void*)spell_prismatic_spray,
    NULL,
    0
  },
  {
    "creeping doom",
    (void*)spell_creeping_doom,
    NULL,
    0
  },

  {
    "buddha palm",
    (void*)do_buddha_palm,
    &gsn_buddha_palm,
    0
  },

  {
    "concentration",
    NULL,
    &gsn_concentration,
    0
  },

  {
    "telepathic gate",
    (void*)power_telepathic_gate,
    NULL,
    0
  },

  {
    "ki hai",
    (void*)power_kihai,
    NULL,
    0
  },

  {
    "fist of fury",
    (void*)do_fist_of_fury,
    &gsn_fist_of_fury,
    0
  },

  {
    "inertial barrier",
    (void*)power_inertial_barrier,
    NULL,
    0
  },

  {
    "moonbeam",
    (void*)spell_moonbeam,
    NULL,
    0
  },

  {
    "flight",
    (void*)do_flight,
    &gsn_flight,
    0
  },

  {
    "mistform",
    (void*)do_mistform,
    &gsn_mistform,
    0
  },


  // language
  {
    "common",
    NULL,
    NULL,
    0
  },
  {
    "elfish",
    NULL,
    NULL,
    0
  },
  {
    "dwarfish",
    NULL,
    NULL,
    0
  },
  {
    "gnomish",
    NULL,
    NULL,
    0
  },
  {
    "giantish",
    NULL,
    NULL,
    0
  },
  {
    "goblinish",
    NULL,
    NULL,
    0
  },
  {
    "faerish",
    NULL,
    NULL,
    0
  },
  {
    "dragonish",
    NULL,
    NULL,
    0
  },
  {
    "underdish",
    NULL,
    NULL,
    0
  },
  {
    "lizardish",
    NULL,
    NULL,
    0
  },
  {
    "bite",                 
    (void*)do_bite,
    &gsn_bite,
    0
  },

  // Added by SinaC 2003
  {
    "shrink",
    (void*)spell_shrink,
    NULL,
    0
  },
  { 
    "regain power",          
    (void*)power_regain_power,      
    NULL,
    0
  },
  {
    "blend",
    (void*)do_blend,
    &gsn_blend,
    0
  },
  {
    "improved exotic",      
    NULL,             
    &gsn_improved_exotic,
    0
  },
  {
    "dual parry",                
    NULL,             
    &gsn_dual_parry,
    0
  },
  //  {
  //    "dual wield 2",
  //    NULL,           
  //    &gsn_dual_wield2,
  //    0
  //  },
  {
    "dual disarm",               
    (void*)do_dual_disarm,
    &gsn_dual_disarm,
    0
  },
  {
    "breath of the fish",
    (void*)spell_water_breath,
    NULL,
    0
  },
  {
    "create shelter",
    (void*)spell_create_shelter,
    NULL,
    0
  },
  {
    "strength of the bear",	
    (void*)spell_strength_of_the_bear,	
    NULL,
    0
  },
  {
    "eyes of the wolf",
    (void*)spell_eyes_of_the_wolf,
    NULL,
    0
  },
  {
    "barkskin",     
    (void*)spell_barkskin,
    NULL,
    0
  },
  {
    "free movement",
    (void*)spell_free_movement,
    NULL,
    0
  },
  {
    "rooted feet",
    (void*)spell_rooted_feet,
    NULL,
    0
  },
  {
    "hibernation",
    (void*)spell_hibernation,
    NULL,
    0
  },
  {
    "nature blessing",
    (void*)spell_nature_blessing,
    NULL,
    0
  },

  {
    "calm animal",
    (void*)spell_calm_animal,
    NULL,
    0
  },
  {
    "appraisal",
    (void*)do_appraisal,
    &gsn_appraisal,
    0
  },
  {
    "purify",
    (void*)do_purify,
    &gsn_purify,
    0
  },
  {
    "gouge",		
    (void*)do_gouge,
    &gsn_gouge,
    0
  },
  {
    "evasion",                
    NULL,             
    &gsn_evasion,
    0
  },
  {
    "signal",
    (void*)do_signal,
    &gsn_signal,
    0
  },
  {
    "improved steal",
    NULL,
    &gsn_improved_steal,
    0
  },
  {
    "comprehend languages",
    NULL,
    &gsn_comprehend_languages,
    0
  },
  {
    "assassination",
    NULL,
    &gsn_assassination,
    0
  },
  {
    "armslore",
    (void*)do_armslore,
    &gsn_armslore,
    0
  },
  {
    "headbutt",
    (void*)do_headbutt,
    &gsn_headbutt,
    0
  },
  {
    "shield bash",
    (void*)do_shield_bash,
    &gsn_shield_bash,
    0
  },
  {
    "holy symbol",
    (void*)spell_holy_symbol,
    NULL,
    0
  },
  {
    "angelic strength",
    (void*)spell_angelic_strength,
    NULL,
    0
  },
  {
    "angelic light",
    (void*)spell_angelic_light,
    &gsn_angelic_light,
    0
  },
  {
    "inspiration",
    (void*)spell_inspiration,
    NULL,
    0
  },
  {
    "judgment bolt",
    (void*)spell_judgment_bolt,
    NULL,
    0
  },
  {
    "holy aura",
    (void*)spell_holy_aura,
    NULL,
    0
  },
  {
    "justice",
    (void*)spell_justice,
    &gsn_justice,
    0
  },
  {
    "charms of the daemon",
    (void*)spell_charms_daemon,
    NULL,
    0
  },
  {
    "terror",
    (void*)spell_terror,
    NULL,
    0
  },
  {
    "unholy word",
    (void*)spell_unholy_word,
    NULL,
    0
  },
  {
    "chaos bolt",
    (void*)spell_chaos_bolt,
    NULL,
    0
  },
  {
    "unholy aura",
    (void*)spell_unholy_aura,
    NULL,
    0
  },
  {
    "ray of deception",
    (void*)spell_ray_of_deception,
    NULL,
    0
  },
  {
    "corruption",
    (void*)spell_corruption,
    &gsn_corruption,
    0
  },
  {
    "last rites",
    (void*)spell_last_rites,
    NULL,
    0
  },
  {
    "hovering sphere",
    (void*)spell_hovering_sphere,
    NULL,
    0
  },
  {
    "cloud of revelation",
    (void*)spell_cloud_of_revelation,
    NULL,
    0
  },
  {
    "final prayer",
    (void*)spell_final_prayer,
    NULL,
    0
  },
  {
    "create alcohol",
    (void*)spell_create_alcohol,
    NULL,
    0
  },
  {
    "mystic boat",
    (void*)spell_mystic_boat,
    NULL,
    0
  },
  {
    "spiritual hammer",
    (void*) spell_spiritual_hammer,
    NULL,
    0
  },
  {
    "resist poison",
    (void*) spell_resist_poison,
    NULL,
    0
  },
  {
    "prismatic armor",
    (void*)spell_prismatic_armor,
    NULL,
    0
  },
  {
    "exhaust",
    (void*)spell_exhaust,
    NULL,
    0
  },
  {
    "mass harm",
    (void*)spell_mass_harm,
    NULL,
    0
  },
  {
    "combat will",
    (void*)spell_combat_will,
    NULL,
    0
  },
  {
    "frozen blast",
    (void*)spell_frozen_blast,
    NULL,
    0
  },
  {
    "fire blast",
    (void*)spell_fire_blast,
    NULL,
    0
  },
  {
    "final strike",
    (void*)spell_final_strike,
    NULL,
    0
  },
  {
    "deplete strength",
    (void*)spell_deplete_strength,
    NULL,
    0
  },
  {
    "detect weather",
    (void*)spell_detect_weather,
    NULL,
    0
  },
  {
    "lightning reflexes",
    (void*)spell_lightning_reflexes,
    NULL,
    0
  },
  {
    "create rainbow",
    (void*)spell_create_rainbow,
    NULL,
    0
  },
  {
    "protection lightning",
    (void*)spell_protection_lightning,
    NULL,
    0
  },
  {
    "summon thunder",
    (void*)spell_summon_thunder,
    NULL,
    0
  },
  {
    "windwalk",
    (void*)spell_windwalk,
    NULL,
    0
  },
  {
    "earthcrumble",
    (void*)spell_earthcrumble,
    NULL,
    0
  },
  {
    "lightning strike",
    (void*)spell_lightning_strike,
    NULL,
    0
  },
  {
    "hurricane",
    (void*)spell_hurricane,
    NULL,
    0
  },
  {
    "locate warpstone",
    (void*)spell_locate_warpstone,
    NULL,
    0
  },
  {
    "unstable gate",
    (void*)spell_unstable_gate,
    NULL,
    0
  },
  {
    "gemwarp",
    (void*)spell_gemwarp,
    NULL,
    0
  },
  {
    "reverse gravity",
    (void*)spell_reverse_gravity,
    NULL,
    0
  },
  {
    "protection negative",
    (void*)spell_protection_negative,
    NULL,
    0
  },
  {
    "angelfire",
    (void*)spell_angelfire,
    NULL,
    0
  },
  {
    "life transfer",
    (void*)spell_life_transfer,
    NULL,
    0
  },
  {
    "protection holy",
    (void*)spell_protection_holy,
    NULL,
    0
  },
  {
    "life drain",
    (void*)spell_life_drain,
    NULL,
    0
  },
  {
    "nature shield",
    (void*)spell_nature_shield,
    &gsn_nature_shield,
    0
  },
  {
    "anti-venom spores",
    (void*)spell_anti_venom_spores,
    NULL,
    0
  },
  {
    "flower of health",
    (void*)spell_flower_of_health,
    NULL,
    0
  },
  {
    "flight of the bird",
    (void*)spell_flight_of_the_bird,
    &gsn_flight_bird,
    0
  },
  {
    "polar adaptation",
    (void*)spell_polar_adaptation,
    NULL,
    0
  },
  {
    "desert adaptation",
    (void*)spell_desert_adaptation,
    NULL,
    0
  },
  {
    "speed of the cheetah",
    (void*)spell_speed_of_the_cheetah,
    &gsn_speedofcheetah,
    0
  },
  {
    "insect swarm",
    (void*)spell_insect_swarm,
    NULL,
    0
  },
  {
    "animal friendship",
    (void*)spell_animal_friendship,
    NULL,
    0
  },
  {
    "thorn blast",
    (void*)spell_thorn_blast,
    NULL,
    0
  },
  {
    "plant door",
    (void*)spell_plant_door,
    NULL,
    0
  },
  {
    "choke",
    (void*)spell_choke,
    NULL,
    5
  },
  {
    "thunderclap",
    (void*)spell_thunderclap,
    NULL,
    5
  },
  {
    "hypnotism",
    (void*)spell_hypnotism,
    NULL,
    0
  },
  {
    "globe of invulnerability",
    (void*)spell_globe_invuln,
    &gsn_globe_invuln,
    0
  },
  {
    "mind blast",
    (void*)spell_mind_blast,
    NULL,
    5
  },
  {
    "mind blade",
    (void*)spell_mind_blade,
    NULL,
    0
  },
  {
    "mirror image",
    (void*)spell_mirror_image,
    NULL,
    0
  },
  {
    "water to wine",
    (void*)spell_water_to_wine,
    NULL,
    0
  },
  {
    "death fog",
    (void*)spell_death_fog,
    NULL,
    5
  },
  {
    "animate object",
    (void*)spell_animate_object,
    NULL,
    0
  },
  {
    "antigravity",
    (void*)spell_antigravity,
    NULL,
    5
  },
  {
    "temporal stasis",
    (void*)spell_temporal_stasis,
    &gsn_temporal_stasis,
    5,
    update_null,
    wearoff_temporal_stasis
  },
  {
    "command undead",
    (void*)spell_command_undead,
    NULL,
    0
  },
  {
    "soothe",
    (void*)do_soothe,
    &gsn_soothe,
    0
  },
  {
    "flamevision",
    (void*)spell_flamevision,
    NULL,
    0
  },
  {
    "stone fist",
    (void*)spell_stone_fist,
    &gsn_stone_fist,
    0
  },
  {
    "hydroblast",
    (void*)spell_hydroblast,
    NULL,
    5
  },
  {
    "elemental field",
    (void*)spell_elemental_field,
    &gsn_elemental_field,
    0
  },
  {
    "immolation",
    (void*)spell_immolation,
    NULL,
    5
  },
  {
    "cone of cold",
    (void*)spell_cone_of_cold,
    NULL,
    5
  },
  {
    "shrieking blades",
    (void*)spell_shrieking_blades,
    NULL,
    5
  },
  {
    "full heal",
    (void*)spell_full_heal,
    NULL,
    0
  },
  {
    "slumber",
    (void*)spell_slumber,
    NULL,
    0
  },
  {
    "darkvision",
    (void*)spell_darkvision,
    NULL,
    0
  },
  {
    "cloud of darkness",
    (void*)spell_cloud_of_darkness,
    NULL,
    0
  },
  {
    "tsunami",
    (void*)spell_tsunami,
    NULL,
    0
  },
  {
    "sacred mists",
    (void*)spell_sacred_mists,
    &gsn_sacred_mists,
    0
  },
  {
    "ride the winds",
    (void*)spell_ride_the_winds,
    NULL,
    0
  },
  {
    "snowy blast",
    (void*)spell_snowy_blast,
    NULL,
    0
  },
  {
    "battle frenzy", // this spell's sn is never used, we use gsn_frenzy instead
    (void*)spell_battle_frenzy,
    NULL,
    0
  },
  {
    "battle fury",
    (void*)spell_battle_fury,
    NULL,
    0
  },
  {
    "friendship",
    (void*)spell_friendship,
    NULL,
    0
  },
  {
    "seduction",
    (void*)spell_seduction,
    NULL,
    0
  },
  {
    "find path",
    (void*)spell_find_path,
    NULL,
    0
  },
  {
    "symbiote",
    (void*)spell_symbiote,
    &gsn_symbiote,
    0
  },
  {
    "soulbind",
    (void*)spell_soulbind,
    NULL,
    0
  },
  {
    "ray of sun",		
    (void*)spell_ray_of_sun,          
    NULL,
    0
  },
  {
    "sunblind",
    (void*)spell_sunblind,
    NULL,
    0
  },
  {
    "knowledge",
    (void*)spell_knowledge,
    NULL,
    0
  },
  {
    "tongues",
    (void*)spell_tongues,
    NULL,
    0
  },
  {
    "fire shield",
    (void*)spell_fireshield,
    &gsn_fireshield,
    0
  },
  {
    "ice shield",
    (void*)spell_iceshield,
    &gsn_iceshield,
    0
  },
  {
    "stone shield",
    (void*)spell_stoneshield,
    &gsn_stoneshield,
    0
  },
  {
    "air shield",
    (void*)spell_airshield,
    &gsn_airshield,
    0
  },
  {
    "anti-magic field",
    (void*)spell_antimagic_field,
    NULL,
    0
  },
  {
    "flesh to stone",
    (void*)spell_flesh_to_stone,
    NULL,
    0,
    update_null,
    wearoff_flesh_to_stone
  },
  {
    "turn undead",
    (void*)spell_turn_undead,
    NULL,
    0
  },
  {
    "quicksand",
    (void*)spell_quicksand,
    NULL,
    5
  },
  {
    "whirling sands",
    (void*)spell_whirling_sands,
    NULL,
    0
  },
  {
    "lightning field",
    (void*)spell_lightning_field,
    &gsn_lightning_field,
    0
  },
  {
    "arrow of anylas",
    (void*)spell_arrow_of_anylas,
    NULL,
    0
  },
  {
    "improved restoration",
    (void*)spell_improved_restoration,
    &gsn_improved_restoration,
    0
  },
  {
    "resurrection",
    (void*)spell_resurrection,
    NULL,
    0
  },
  {
    "vacuum",
    (void*)spell_vacuum,
    NULL,
    5
  },
  {
    "flame burst",
    (void*)spell_flame_burst,
    NULL,
    5
  },
  {
    "ice blast",
    (void*)spell_ice_blast,
    NULL,
    5
  },
  {
    "ice storm",
    (void*)spell_ice_storm,
    NULL,
    5
  },
  {
    "fire storm",
    (void*)spell_fire_storm,
    NULL,
    5
  },
  {
    "meteor shower",
    (void*)spell_meteor_shower,
    NULL,
    5
  },
  {
    "necrotism",
    (void*)spell_necrotism,
    NULL,
    0
  },
  {
    "enlarge",
    (void*)spell_enlarge,
    &gsn_enlarge,
    0
  },
  {
    "reduce",
    (void*)spell_reduce,
    &gsn_reduce,
    0
  },
  {
    "growth shrooms",
    (void*)spell_growth_shrooms,
    NULL,
    0
  },
  {
    "shrinking shrooms",
    (void*)spell_shrinking_shrooms,
    NULL,
    0
  },
  {
    "gift of nature",
    (void*)spell_gift_of_nature,
    NULL,
    0
  },
  {
    "tornado",
    (void*)spell_tornado,
    NULL,
    5
  },
  {
    "sandstorm",
    (void*)spell_sandstorm,
    NULL,
    5
  },
  {
    "hydrostrike",
    (void*)spell_hydrostrike,
    NULL,
    0
  },
  {
    "blinding radiance",
    (void*)spell_blinding_radiance,
    NULL,
    0
  },
  {
    "polymorph self",
    (void*)spell_polymorph_self,
    &gsn_polymorph_self,
    0,
    update_null,
    wearoff_polymorph_self
  },
  {
    "arachnophobia",
    (void*)spell_arachnophobia,
    NULL,
    0
  },
  {
    "fear",		
    (void*)spell_fear,
    NULL,
    5
  },
  {
    "pool of blood",
    (void*)spell_pool_of_blood,
    NULL,
    0
  },
  {
    "phylactery",
    (void*)spell_phylactery,
    &gsn_phylactery,
    0
  },
  {
    "scribe",
    (void*)do_scribe,
    &gsn_scribe,
    0
  },
  {
    "riding",
    (void*)do_mount,
    &gsn_riding,
    2
  },
  {
    "mounted combat",
    NULL,
    &gsn_mounted_combat,
    0
  },
  {
    "charge",
    (void*)do_charge,
    &gsn_charge,
    0
  },
  {
    "bond",
    (void*)do_bond,
    &gsn_bond,
    0
  },
  {
    "invincible mount",
    (void*)spell_invincible_mount,
    &gsn_invincible_mount,
    0
  },
  {
    "strategic retreat",
    (void*)spell_strategic_retreat,
    &gsn_strategic_retreat,
    0
  },
  {
    "study",
    (void*)do_study,
    &gsn_study,
    0
  },
  {
    "speak with dead",
    (void*)spell_speak_with_dead,
    NULL,
    0
  },
  {
    "enchant wand",
    (void*)spell_enchant_wand,
    NULL,
    0
  },
  {
    "enchant staff",
    (void*)spell_enchant_staff,
    NULL,
    0
  },
  {
    "inferno",
    (void*)spell_inferno,
    NULL,
    5
  },
  {
    "detect lycanthropy",
    (void*)spell_detect_lycanthropy,
    NULL,
    0
  },
  {
    "conjure mount",
    (void*)spell_conjure_mount,
    NULL,
    0
  },
  {
    "summon angel",
    (void*)spell_summon_angel,
    NULL,
    0
  },
  {
    "summon demon",
    (void*)spell_summon_demon,
    NULL,
    0
  },
  {
    "faerie aura",
    (void*)spell_faerie_aura,
    NULL,
    0
  },
  {
    "word of recall",
    (void*)spell_word_of_recall,
    NULL,
    0
  },
  {
    "protection lycanthropy",
    (void*)spell_protection_lycanthropy,
    &gsn_protection_lycanthropy,
    0
  },
  {
    "confusion",
    (void*)spell_confusion,
    NULL,
    0
  },
  {
    "chaotic blast",
    (void*)spell_chaotic_blast,
    NULL,
    0
  },
  {
    "create vines",
    (void*)spell_create_vines,
    NULL,
    0
  },
  {
    "song of huma",
    (void*)song_of_huma,
    NULL,
    0
  },
  {
    "ballad of spring",
    (void*)song_ballad_of_spring,
    NULL,
    0
  },
  {
    "chill winter of vingaard",
    (void*)song_chill_winter_of_vingaard,
    NULL,
    0
  },
  {
    "crysanias song",
    (void*)song_crysanias_song,
    NULL,
    0
  },
  {
    "dragons come",
    (void*)song_dragons_come,
    NULL,
    0
  },
  {
    "hymn for the ancestors",
    (void*)song_hymn_for_the_ancestors,
    NULL,
    0
  },
  {
    "journeymans hymne",
    (void*)song_journeymans_rhyme,
    NULL,
    0
  },
  {
    "knights ride forth",
    (void*)song_knights_ride_forth,
    NULL,
    0
  },
  {
    "specialization",
    (void*)do_specialize,
    &gsn_specialization,
    0
  },
  {
    "disguise",
    (void*)do_disguise,
    &gsn_disguise,
    0
  },
  {
    "counterfeit",
    (void*)do_counterfeit,
    &gsn_counterfeit,
    0
  },
  {
    "favored enemy",
    (void*)do_favored_enemy,
    &gsn_favored_enemy,
    0
  },
  //  {
  //    "riding 2",
  //    (void*)do_riding2,
  //    &gsn_riding2,
  //    0
  //  },
  {
    "climbing",
    (void*)do_climbing,
    &gsn_climbing,
    0
  },
  {
    "flare",
    (void*)spell_flare,
    NULL,
    0
  },
  {
    "animal follower",
    (void*)spell_animal_follower,
    NULL,
    2
  },
  {
    "shapechange",
    (void*)do_shapechange,
    &gsn_shapechange,
    3
  },
  {
    "brew",
    (void*)do_brew,
    &gsn_brew,
    0
  },
  {
    "looking in the mirror",
    (void*)song_looking_in_the_mirror,
    NULL,
    0,
    update_looking_in_the_mirror,
    wearoff_null
  },
  {
    "mishakals song",
    (void*)song_mishakals_song,
    NULL,
    0
  },
  {
    "neverending pain",
    (void*)song_neverending_pain,
    NULL,
    0,
    update_neverending_pain,
    wearoff_null
  },
  {
    "palistan song",
    (void*)song_palistan_song,
    NULL,
    0
  },
  {
    "rest of dragons",
    (void*) song_rest_of_dragons,
    NULL,
    0
  },
  {
    "ride of glory",
    (void*)song_ride_of_glory,
    NULL,
    0
  },
  {
    "shadowborn",
    (void*)song_shadowborn,
    NULL,
    0
  },
  {
    "shallow grave",
    (void*)song_shallow_grave,
    NULL,
    0
  },
  {
    "shield of words",
    (void*)song_shield_of_words,
    NULL,
    0
  },
  {
    "wail of banshees",
    (void*)song_wail_of_banshees,
    NULL,
    0
  },
  {
    "when the sun goes down",
    (void*)song_when_the_sun_goes_down,
    NULL,
    0
  },
  {
    "spellfire",
    (void*)spell_spellfire,
    NULL,
    0
  },
  {
    "summon elemental",
    (void*)spell_summon_elemental,
    NULL,
    0
  },
  {
    "necrofire",
    (void*)spell_necrofire,
    NULL,
    5
  },
  {
    "spectral hand",
    (void*)spell_spectral_hand,
    NULL,
    5
  },
  {
    "rotting touch",
    (void*)spell_rotting_touch,
    NULL,
    5
  },
  {
    "death spell",
    (void*)spell_death_spell,
    NULL,
    5
  },
  {
    "summon ghost",
    (void*)spell_summon_ghost,
    NULL,
    0
  },
  {
    "finger of death",
    (void*)spell_finger_of_death,
    NULL,
    5
  },
  {
    "black plague",
    (void*)spell_black_plague,
    &gsn_black_plague,
    5,
    update_black_plague,
    wearoff_null
  },

  {
    "fleetfooted",
    NULL,
    &gsn_fleetfooted,
    0
  },
  {
    "icy fist",
    (void*)power_icy_fist,
    &gsn_icy_fist,
    0
  },
  {
    "acid fist",
    (void*)power_acid_fist,
    &gsn_acid_fist,
    0
  },
  {
    "draining fist",
    (void*)power_draining_fist,
    &gsn_draining_fist,
    0
  },
  {
    "hands of multitude",
    NULL,
    &gsn_hands_of_multitude,
    0
  },
  {
    "pure body",		
    NULL,		
    &gsn_pure_body,
    0
  },
  {
    "diamond body",		
    NULL,		
    &gsn_diamond_body,
    0
  },
  {
    "tumble",                
    NULL,             
    &gsn_tumble,
    0
  },
  {
    "block",                
    NULL,             
    &gsn_block,
    0
  },
  {
    "deflect",                
    NULL,             
    &gsn_deflect,
    0
  },
  { // same as telepathic gate
    "ki step",
    (void*)power_telepathic_gate,
    NULL,
    0
  },
  {
    "ki pass",
    (void*)power_ki_pass,
    NULL,
    0
  },
  {
    "lifetouch",
    (void*)power_lifetouch,
    NULL,
    0
  },
  {
    "flurry of fists",                
    NULL,             
    &gsn_flurry_of_fists,
    0
  },

 
  {
    "archery",
    (void*)do_bowfire,
    &gsn_bowfire,
    4
  },
  {
    "fletcher",
    (void*)do_fletcher,
    &gsn_fletcher,
    0
  },
  {
    "disarming shot",
    (void*)do_disarming_shot,
    &gsn_disarming_shot,
    0
  },
  {
    "bowyer",
    (void*)do_restring,
    &gsn_bowyer,
    0
  },
  {
    "strength shot",
    NULL,
    &gsn_strength_shot,
    0
  },
  {
    "ignite arrow",
    (void*)do_ignite_arrow,
    &gsn_ignite_arrow,
    0,
    update_ignite_arrow,
    wearoff_ignite_arrow
  },
  {
    "long range shot",
    NULL,
    &gsn_long_range_shot,
    0
  },
  {
    "sharpen arrow",
    (void*)do_sharpen_arrow,
    &gsn_sharpen_arrow,
    0
  },
  {
    "point blank shot",
    NULL,
    &gsn_point_blank_shot,
    0
  },
  {
    "critical shot",
    NULL,
    &gsn_critical_shot,
    0
  },
  {
    "reckless dweomer",
    (void*)spell_reckless_dweomer,
    &gsn_reckless_dweomer,
    0
  },
  {
    "third wield", 
    NULL,//(void*)do_third_wield,
    &gsn_third_wield,
    0
  },
  {
    "fourth wield", 
    NULL,//(void*)do_fourth_wield,
    &gsn_fourth_wield,
    0
  },
  {
    "embalm",
    (void*)spell_embalm,
    NULL,
    0
  },
  {
    "zombify",
    (void*)spell_zombify,
    &gsn_zombify,
    0
  },
  {
    "disrupt",
    (void*)power_disrupt,
    NULL,
    0
  },
  {
    "warmth",
    (void*)power_warmth,
    NULL,
    0
  },
  {
    "telekinesis",
    (void*)power_telekinesis,
    NULL,
    0
  },
  {
    "mind over body",
    (void*)power_mind_over_body,
    NULL,
    0
  },
  {
    "soften",
    (void*)power_soften,
    NULL,
    0
  },
  {
    "mind freeze",
    (void*)power_mind_freeze,
    NULL,
    0
  },
  {
    "accelerate",
    (void*)power_accelerate,
    &gsn_accelerate,
    0
  },
  {
    "nerve shock",
    (void*)power_nerve_shock,
    NULL,
    0
  },
  {
    "probability travel",
    (void*)power_probability_travel,
    NULL,
    0
  },
  {
    "cause decay",
    (void*)power_cause_decay,
    NULL,
    0
  },
  {
    "acidic touch",
    (void*)power_acidic_touch,
    NULL,
    0
  },
  {
    "neural burn",
    (void*)power_neural_burn,
    NULL,
    0
  },
  {
    "enhance armor",
    (void*)power_enhance_armor,
    NULL,
    0
  },
  {
    "trauma",
    (void*)power_trauma,
    NULL,
    0
  },
  {
    "willpower",
    (void*)power_willpower,
    NULL,
    0
  },

  // Special Affect such as Lycanthropy
  {
    "lycanthropy",
    NULL,
    &gsn_lycanthropy,
    0,
    update_lycanthropy,
    wearoff_lycanthropy
  },

/* Doesn't need dummy skill anymore, SinaC 2003  MAX_ABILITY gives us the number of ability
  {
    NULL,
    NULL,
    NULL,
    0
  }
*/
};

// Modified by SinaC 2000
struct  group_type      *group_table;

// Added by SinaC 2000
struct  god_data        *gods_table;

// Added by SinaC 2001
struct material_type    *material_table;
