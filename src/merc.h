#ifndef __MERC_H__         // fix multi includes
#define __MERC_H__         // fix multi includes


// Added by SinaC 2003   cygwin port
#define NOCRYPT

#include "gc_helper.h"
#include "string_space.h"

#define free(x) NO_free(x)
#define malloc(x) NO_malloc(x)
#define calloc(x,y) NO_calloc(x,y)
#define realloc(x,y) NO_realloc(x,y)


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

#include <stdlib.h>
#include <stdio.h>

// Accommodate old non-Ansi compilers.
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
// Added by SinaC 2000 for song for bards
#define DECLARE_SONG_FUN( fun )         void fun( )
// Added by SinaC 2001 for power for mental user
#define DECLARE_POWER_FUN( fun )	void fun( )

// Added by SinaC 2003, for clean affect wear_off and update
#define DECLARE_WEAROFF_FUN( fun )     void fun( )
#define DECLARE_UPDATE_FUN( fun )       void fun( )

#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
// Added by SinaC 2000 for song for bards
#define DECLARE_SONG_FUN( fun )         SONG_FUN  fun 
// Added by SinaC 2001 for power for mental user
#define DECLARE_POWER_FUN( fun )	POWER_FUN fun

// Added by SinaC 2003, for clean affect wear_off and update
#define DECLARE_WEAROFF_FUN( fun )     WEAROFF_FUN fun
#define DECLARE_UPDATE_FUN( fun )       UPDATE_FUN fun

#endif

// system calls
extern "C" {
  int unlink(char * c);
//  char *crypt(const char*key, const char *salt);
#ifndef __GNUC__
  int system(const char * c);
#endif
}

#ifdef __GNUC__
#  include <stdlib.h>
#endif




// Short scalar types.
// Diavolo reports AIX compiler has bugs with short types.
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif


//#if	defined(_AIX)
//#if	!defined(const)
//#define const
//#endif
//typedef int				int;
//typedef int				bool;
//#define unix
//#else
//typedef short   int			int;
//#endif

// Structure types.
typedef struct	affect_data		AFFECT_DATA;
//typedef struct  old_affect_data         OLD_AFFECT_DATA; // New affect system by oxtal


typedef struct	area_data		AREA_DATA;
typedef struct	ban_data		BAN_DATA;
typedef struct 	buf_type	 	BUFFER;
typedef struct	char_data		CHAR_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	kill_data		KILL_DATA;
typedef struct	mem_data		MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data		NOTE_DATA;
typedef struct  entity_data             ENTITY_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct  gen_data		GEN_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	weather_data		WEATHER_DATA;
// Added for disabling command  by Sinac 1997
typedef struct  disabled_data           DISABLED_DATA;
// Added by SinaC 2000 for restriction
typedef struct  restr_data              RESTR_DATA;
// Added by SinaC 2000 for ability prereqs
typedef struct  prereq_data             PREREQ_DATA;
typedef struct  prereq_list             PREREQ_LIST;
// Added by SinaC 2000 for ability level
typedef struct  ability_info            ABILITY_INFO;
// Added by SinaC 2000 for ability upgrade on item
typedef struct  ability_upgrade         ABILITY_UPGRADE;

// Added by SinaC 2000 for alignment/etho info
typedef struct  align_info              ALIGN_INFO;

// Added by SinaC 2000 for gods
typedef struct  god_data                GOD_DATA;
// Added by SinaC 2001 for player disabled command
typedef struct  disabled_cmd_data       DISABLED_CMD_DATA;
typedef struct  faction_data            FACTION_DATA;


// Function types.
typedef	void DO_FUN	args( ( CHAR_DATA *ch, const char *argument ) );
typedef bool SPEC_FUN	args( ( CHAR_DATA *ch ) );
// Modified by SinaC 2001
//typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
//				int target ) );
typedef void SPELL_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target, int casting_level ) );
// Added by SinaC 2000 for song for bard, modified by SinaC 2003
typedef void SONG_FUN   args( ( int sn, int level, CHAR_DATA *ch ) );
// Added by SinaC 2001 for power for mental user
typedef void POWER_FUN	args( ( int sn, int level, CHAR_DATA *ch, void *vo,
				int target ) );

// Added by SinaC 2003, for clean affect wear_off and update
typedef void WEAROFF_FUN  args( ( AFFECT_DATA *af, 
				   void *vo, int target ) );
typedef void UPDATE_FUN    args( ( AFFECT_DATA *af, 
				   void *vo, int target ) );


struct OLCEditable {
  bool lockBit;
  DESCRIPTOR_DATA *editedBy;
  long editFlags;

  OLCEditable() {
    lockBit = 0;
    editedBy = NULL;
  }
};


#include "board.h" // Oxtal -- new board system
#include "script2.h" // Oxtal


// String and memory management parameters.
#define	MAX_KEY_HASH		 1024
#define MAX_STRING_LENGTH	 4608
#define MAX_INPUT_LENGTH	  256
// Modified by SinaC 2001
//#define PAGELEN			   22
#define PAGELEN			   24


// Game parameters.
// Increase the max'es if you add more of something.
// Adjust the pulse numbers to suit yourself.

// CLANS
// Removed by SinaC 2000
//extern int MAX_CLAN;

#define MAX_SOCIALS		  256
// MAX_SKILL is now a variable, SinaC 2000
extern int MAX_ABILITY;

// a variable now, SinaC 2000
extern int MAX_GROUP;

// School
extern int MAX_SCHOOL;

// Super race
extern int MAX_SUPERRACE;

// Maximum gods, SinaC 2000
extern int MAX_GODS;

// Removed by SinaC 2000
//#define MAX_IN_GROUP		   25
#define MAX_ALIAS		   30
// MAX_CLASS is now a variable, SinaC 2000
extern int MAX_CLASS;
// MAX_PC_RACE is now a variable, SinaC 2000
extern int MAX_PC_RACE;
// MAX_RACE is now a variable, SinaC 2000
extern int MAX_RACE;

// Added by SinaC 2000
extern int MAX_POSE;
// Added by SinaC 2000
extern int MAX_LIQUID;
// Added by SinaC 2001
extern int MAX_MATERIAL;
// Added by SinaC 2001
extern int MAX_COMMANDS;
// SinaC 2003
extern int MAX_HOMETOWN;

// Added by SinaC 2003 for factions
#define MAX_FACTION                (32)



#define MAX_DAMAGE_MESSAGE	   41
#define MAX_LEVEL		   110
#define LEVEL_HERO		   (MAX_LEVEL - 9)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 8)

#define PULSE_PER_SECOND	    4
#define PULSE_VIOLENCE		  ( 3 * PULSE_PER_SECOND)
#define PULSE_MOBILE		  ( 4 * PULSE_PER_SECOND)
//#define PULSE_MUSIC		  ( 6 * PULSE_PER_SECOND)   removed by SinaC 2003
#define PULSE_TICK                (60 * PULSE_PER_SECOND)

#define PULSE_AREA		  (120 * PULSE_PER_SECOND)
#define PULSE_HINTS               (30 * PULSE_PER_SECOND)
// Added by SinaC 2000 for Arena
#define PULSE_ARENA               (30 * PULSE_PER_SECOND)
// Added by SinaC 2000 for global earthquake
#define PULSE_EARTHQUAKE          (5 * PULSE_PER_SECOND)
// Added by SinaC 2001
#define PULSE_HOUR                (60 * 60 * PULSE_PER_SECOND)

#define IMPLEMENTOR		MAX_LEVEL
#define	CREATOR			(MAX_LEVEL - 1)
#define SUPREME			(MAX_LEVEL - 2)
#define DEITY			(MAX_LEVEL - 3)
#define GOD			(MAX_LEVEL - 4)
#define IMMORTAL		(MAX_LEVEL - 5)
#define DEMI			(MAX_LEVEL - 6)
#define ANGEL			(MAX_LEVEL - 7)
#define AVATAR			(MAX_LEVEL - 8)
#define HERO			LEVEL_HERO

// Colour stuff by Lope of Loping Through The MUD
// Resets Colour
#define CLEAR		"\033[0m"
// Normal Colours
#define C_RED		"\033[0;31m"
#define C_GREEN		"\033[0;32m"
#define C_YELLOW	"\033[0;33m"
#define C_BLUE		"\033[0;34m"
#define C_MAGENTA	"\033[0;35m"
#define C_CYAN		"\033[0;36m"
#define C_WHITE		"\033[0;37m"
// Light Colors
#define C_D_GREY	"\033[1;30m"
#define C_B_RED		"\033[1;31m"
#define C_B_GREEN	"\033[1;32m"
#define C_B_YELLOW	"\033[1;33m"
#define C_B_BLUE	"\033[1;34m"
#define C_B_MAGENTA	"\033[1;35m"
#define C_B_CYAN	"\033[1;36m"
#define C_B_WHITE	"\033[1;37m"

// Site ban structure.
#define BAN_SUFFIX		A
#define BAN_PREFIX		B
#define BAN_NEWBIES		C
#define BAN_ALL			D	
#define BAN_PERMIT		E
#define BAN_PERMANENT		F


struct	ban_data {
  BAN_DATA *	next;
  int	ban_flags;
  int	level;
  const char *	name;
};

// Buffer structure
struct buf_type {
  int      state;  // error state of the buffer
  int      size;   // size in k
  char *      string; // buffer's string
};


struct	time_info_data {
  int		hour;
  int		day;
  int		month;
  int		year;
};

struct	weather_data {
  int		mmhg;
  int		change;
  int		sky;
  int		sunlight;
};

// Added by Sinac 1997 for disabling command
struct  disabled_data {
  DISABLED_DATA *next;             // Pointer to next node
  struct cmd_type *command;  // Pointer to the disabled command
  const char *disabled_by;               // Name of disabler
  int level;                    // Level of disabler
};

extern DISABLED_DATA * disabled_first;


// Connected state for a channel.
#define CON_PLAYING			 (0)
#define CON_GET_NAME			 (1)
#define CON_GET_OLD_PASSWORD		 (2)
#define CON_CONFIRM_NEW_NAME		 (3)
#define CON_GET_NEW_PASSWORD		 (4)
#define CON_CONFIRM_NEW_PASSWORD	 (5)
#define CON_GET_NEW_RACE		 (6)
#define CON_GET_NEW_SEX			 (7)
#define CON_GET_NEW_CLASS		 (8)
#define CON_GET_ALIGNMENT		 (9)
#define CON_DEFAULT_CHOICE		(10)
#define CON_GEN_GROUPS			(11)
#define CON_PICK_WEAPON			(12)
#define CON_READ_IMOTD			(13)
#define CON_READ_MOTD			(14)
#define CON_BREAK_CONNECT		(15)
// Added by Oxtal 1997 for copyover
#define CON_COPYOVER_RECOVER		(16)
// Added by SinaC 2000 for god
#define CON_GET_GOD                     (17)
// Added by SinaC 2001 for rebirth
#define CON_REBIRTH                     (18)
// Added by SinaC 2001 for color greetings
#define CON_ANSI                        (19)
// SinaC 2003 for remort, note use CON_20 to CON_24
#define CON_REMORT                      (25)
// SinaC 2003 for hometown creation choice
#define CON_GET_HOMETOWN                (26)
// SinaC 2003 for wild magic choice
#define CON_GET_WILD_MAGIC              (27)


// Descriptor (channel) structure.
struct	descriptor_data {
  DESCRIPTOR_DATA *	next;
  DESCRIPTOR_DATA *	snoop_by;
  CHAR_DATA *		character;
  CHAR_DATA *		original;
  const char *		host;
  int		descriptor;
  int		connected;
  bool		fcommand;
  char		inbuf		[4 * MAX_INPUT_LENGTH];
  char		incomm		[MAX_INPUT_LENGTH];
  char		inlast		[MAX_INPUT_LENGTH];
  int			repeat;
  char *		outbuf;
  int			outsize;
  int			outtop;
  char *		showstr_head;
  char *		showstr_point;
  void *                pEdit;		// OLC 
  const char **         pString;	// OLC 
  int			editor;		// OLC 
  void *                pStringFun;     // function called when we stop editing a string
  void *                pStringArg;     // pStringFun's arguments


  // Added by SinaC 2000 for 'jog'
  char *              run_buf;
  char *              run_head;

  // Added by SinaC 2001 for color greet
  bool                ansi;

  // Added by SinaC 2003 for line number mode in string editior
  bool                lineMode;
  // Added by SinaC 2003, show color code when using string editor
  //  useful to edit scripts
  bool                stringColorMode;
};




// Attribute bonus structures.
struct	str_app_type {
//  int	tohit;  Removed by SinaC 2001
  int	todam;
  int	carry;
  int	wield;
};

struct	int_app_type {
  int	learn;
};

struct	wis_app_type {
  int	practice;
};

struct	dex_app_type {
  int	tohit; // Added by SinaC 2001
  int	defensive;
};

struct	con_app_type {
  int	hitp;
  int	shock;
};



// TO types for act.
#define TO_ROOM		    0
#define TO_NOTVICT	    1
#define TO_VICT		    2
#define TO_CHAR		    3
#define TO_ALL		    4



// Help table types.
struct	help_data {
  HELP_DATA *	next;
  int	level;
  const char *	keyword;
  const char *	text;
};




// Shop types.
#define MAX_TRADE	 5

struct	shop_data {
  SHOP_DATA *	next;			// Next shop in list		
  int	keeper;			// Vnum of shop keeper mob	
  int	buy_type [MAX_TRADE];	// Item types shop will buy	
  int	profit_buy;		// Cost multiplier for buying	
  int	profit_sell;		// Cost multiplier for selling	
  int	open_hour;		// First opening hour		
  int	close_hour;		// First closing hour		
};



//Per-class stuff.
//#define MAX_GUILD 	2 // To be removed -- Oxtal
#define MAX_STATS 	5
#define STAT_STR 	0
#define STAT_INT	1
#define STAT_WIS	2
#define STAT_DEX	3
#define STAT_CON	4

struct	hometown_type {
  const char *	name;
  int		recall;
  int		hall;
  int           school;
  int           morgue;
  int           donation;
  bool          choosable;
};

// constants for type in class_type
//#define CLASS_MAGIC    0
//#define CLASS_MENTAL   1
//#define CLASS_COMBAT   2
// SinaC 2003: class type is now a bitvector
#define CLASS_MAGIC    (A)
#define CLASS_MENTAL   (B)
#define CLASS_COMBAT   (C)
// If class is wildable: at creation, after class choose an additional menu to choose if wild or not
//  char_data->wild will be set to true or false
// A wild class gain access to spell reckless dweomer + every casted spell's level is in a range +/- 10
//  + sometimes a wild surge is added to the spell
#define CLASS_WILDABLE (D)

// constants for choosable in class_type
#define CLASS_CHOOSABLE_NEVER       (0)
#define CLASS_CHOOSABLE_YES         (1)
#define CLASS_CHOOSABLE_NOTCREATION (2)

#define LAWFUL_GOOD      (A)
#define NEUTRAL_GOOD     (B)
#define CHAOTIC_GOOD     (C)
#define LAWFUL_NEUTRAL   (D)
#define TRUE_NEUTRAL     (E)
#define CHAOTIC_NEUTRAL  (F)
#define LAWFUL_EVIL      (G)
#define NEUTRAL_EVIL     (H)
#define CHAOTIC_EVIL     (I)

#define EVERY_ETHO_ALIGN (LAWFUL_GOOD|NEUTRAL_GOOD|CHAOTIC_GOOD\
|LAWFUL_NEUTRAL|TRUE_NEUTRAL|CHAOTIC_NEUTRAL\
|LAWFUL_EVIL|NEUTRAL_EVIL|CHAOTIC_EVIL)

#define MAX_ETHO_ALIGN   (9)

#define ETHO_CHAOTIC (-1)
#define ETHO_NEUTRAL (0)
#define ETHO_LAWFUL  (1)

// SinaC 2003
#define ABILITY_TYPE_COUNT          (5)
// Constants for type in ability_type
#define TYPE_SKILL                  (0)
#define TYPE_SPELL                  (1)
#define TYPE_POWER                  (2)
#define TYPE_SONG                   (3)
// This ability type is a bit special: don't have any action_fun, only used
//  for affect like LYCANTHROPY, ... mostly used for UPDATE_FUN and WEAROFF_FUN
#define TYPE_AFFECT                 (4)


// Added by SinaC 2003
// if 'other' = -1: every abilities can be learned at 'highest' level
// if not         : one ability can be learn at 'highest' and other abilities to 'other'
struct casting_rule_type {
  int highest;
  int other;

  casting_rule_type& operator=(const casting_rule_type& c) {
    highest = c.highest;
    other = c.other;
    return *this;
  } 
};
inline bool operator==(const casting_rule_type& c1, const casting_rule_type& c2 ) {
 return c1.highest == c2.highest && c1.other == c2.other;
}
inline bool operator!=(const casting_rule_type& c1, const casting_rule_type& c2 ) {
 return c1.highest != c2.highest || c1.other != c2.other;
}

struct	class_type: OLCEditable {
  const char *	name;			// the full name of the class 
  char 	        who_name	[4];	// Three-letter name for 'who'	
  int	        attr_prime;		// Prime attribute		
  int	        weapon;			// First weapon			
  //int	guild[MAX_GUILD];	Vnum of guild rooms		
  // Replaced by a room flag -- Oxtal 1997
    
  int	        ability_adept;		// Maximum ability percentage with pra 
  int	        thac0_00;		// Thac0 for level  0		
  int	        thac0_32;		// Thac0 for level 32		
  int	        hp_min;			// Min hp gained on leveling	
  int	        hp_max;			// Max hp gained on leveling	
  //bool	        fMana;			// Class gains mana on level	
  int           type;                   // type: CLASS_MAGIC, CLASS_MENTAL, CLASS_COMBAT, CLASS_WILDABLE
  const char *	base_group;		// base groups gained		
  const char *	default_group;		// default groups gained	

  // Added by SinaC 2001 for align restriction
  int           nb_allowed_align;
  ALIGN_INFO    *allowed_align;
  //int             allowed_align;       // bit 0, 1, 2  are for etho      check constants
                                         // bit 3, 4, 5  are for alignment

  // Removed by SinaC 2003
  //int           max_casting_rule; // 1: every spells at casting lvl 3              for other
                                  // 2: every spells at casting lvl 4              for mage/elementalist
                                  // 3: one to level 5 and the others to level 3   for necromancer
                                  // 4: one to level 5 and the others to level 4   for necromancer+mage
                                  // 5: one to level 4 and the others to level 3   for wild mages
                                  // 99: every spells at casting lvl 5             for immortals :)
  // Added by SinaC 2003 for subclass system
  int parent_class;               // index of parent class
  int nb_sub_classes;             // number of subclasses
  int *sub_classes;               // vector of index to child classes

  // Added by SinaC 2003
  int choosable;                  // Can the class be picked during creation
                                  // 0: can't be picked (not yet finished, not implemented, ...)
                                  // 1: can be picked during creation or multiclass or specialization
                                  // 2: can't be picked during creation but can be accessed through
                                  //    specialization

  // SinaC 2003: replace   class_type->weapon  and  hard coded outfit
  int num_obj_list;               // number of entry in obj_list
  int *obj_list;                  // list of object vnum given to a player at creation

  // There is one casting rule for each ability type, SinaC 2003
  casting_rule_type casting_rule[ABILITY_TYPE_COUNT]; // vector with casting rule for each ability type
};


struct item_type {
  int		type;
  const char *	name;
};

struct weapon_type {
  const char *	name;
  int	vnum;
  int	type;
  int	*gsn;
};

struct wiznet_type {
  const char *	name;
  long 	flag;
  int		level;
};

struct attack_type {
  const char *	name;			// name
  const char *	noun;			// message
  int   	damage;			// damage class
};

struct race_type: OLCEditable {
  const char *	name;			// call name of the race 
  bool	pc_race;		// can be chosen by PC's 
  long	act;			// act bits for the race 
  long	aff;			// aff bits for the race 
  // Added by SinaC 2001
  long	aff2;			// aff bits 2nd part for the race 
  long	off;			// off bits for the race 
  long	imm;			// imm bits for the race 
  long  res;			// res bits for the race 
  long	vuln;			// vuln bits for the race 
  long	form;			// default form flag for the race 
  long	parts;			// default parts for the race 
  int   size;                   // size for the race, moved from pc_race_type by SinaC 2003
};


struct pc_race_type: OLCEditable {  // additional data for pc races 
  const char *	name;			// MUST be in race_type 
  char 	who_name[6];
  // Removed by SinaC 2001
  //int	points;			// cost in points of the race 

  // Modified by SinaC 2001
  //int *class_mult;	// exp multiplier for class, * 100 
  int expl; // number of base exp/level needed for that class

  // Modified by SinaC 2001
  //char *	skills[5];		// bonus skills for the race
  int   nb_abilities;
  int * abilities;

  int 	stats[MAX_STATS];	// starting stats
  int	max_stats[MAX_STATS];	// maximum stats
  //int	size;			// size of the race, moved in race_type by SinaC 2003

  // Added by SinaC 2001 for align/class restriction
  int nb_allowed_align;
  ALIGN_INFO *allowed_align;
  //int   allowed_align;          // bit 0, 1, 2  for etho
                                // bit 1, 2, 3  for etho

  //  int nb_allowed_class;
  //  int *allowed_class;
  int allowed_class;

  // Added by SinaC 2001 to tell if the race is pickable at creation
  //int pickable;  modified by SinaC 2003, we know need more information than choosable or not
  int type; // 0: not defined
            // 1: available at creation
            // 2: rebirth race
            // 3: remort race
            // 4: cursed race (werewolf, vampire, ...)

  // Added by SinaC 2001 for racial language
  int language; // language sn

  // Added by SinaC 2003 for remort race
  int nb_remorts; // nbr of remorts
  int *remorts; // list of remort's race id
  int min_remort_number; // minimum number of player's remort before accessing this race

  // Added by SinaC 2003 for super-race, non-choosable race; regroupement of races
  //  such as Elf, Dwarf, Gnome, Giant, Rebirth, ...
  int super_race;

  // Added by SinaC 2003 for rebirth race
  int nb_rebirth; // nbr of rebirths
  int *rebirth_list; // list of rebirth's race id
  int *rebirth_probability; // probability of getting this race when rebirthing
};

#define RACE_NOTAVAILABLE (0)
#define RACE_CREATION (1)
#define RACE_REBIRTH  (2)
#define RACE_REMORT   (3)
#define RACE_CURSED   (4)


struct super_pc_race_type {
  const char *name;
  int nb_pc_race;
  int *pc_race_list;
};

extern struct super_pc_race_type *super_pc_race_table;

struct spec_type {
  const char * 	name;			// special function name
  SPEC_FUN *	function;		// the function
};



// Data structure for notes.
#define NOTE_NOTE	0
#define NOTE_IDEA	1
#define NOTE_PENALTY	2
#define NOTE_NEWS	3
#define NOTE_CHANGES	4
struct	note_data {
  NOTE_DATA *	next;
  int	type;
  const char *	sender;
  const char *	date;
  const char *	to_list;
  const char *	subject;
  const char *	text;
  time_t  	date_stamp;
  time_t	expire;
};



// An OLD affect.
// Unused, SinaC 2001
//struct  old_affect_data {
//  OLD_AFFECT_DATA *   next;
//
//  int		where;
//  int		type;
//  int		level;
//  int		duration;
//  int		location;
//  int		modifier;
//  int		bitvector;
//};


// Modified by SinaC 2003
// An affect.
//struct	affect_data {
//  AFFECT_DATA *	next;
//
//  int                 where;     // char-obj-weapon-room 
//  int                 location;  // ATTR_STR - ATTR_immflags ... NA if where <> char
//
//  int                 op;        // +, or ...
//  // Modified by SinaC 2001 long before
//  long                modifier;
//
//  int		      duration;
//  int                 level; // The spell level 
//  int                 type; // ability number of the spell that caused - 0 if NA
//
//  // Added by SinaC 2000 for ability casting level
//  int                 casting_level; // casting level of the affect
//
//  long                flags; // Affect flags: AFFECT_STAY_DEATH, AFFECT_NON_DISPELLABLE, ...
//};

typedef struct affect_list AFFECT_LIST;

// A general affect (once for each instance of an ability)
struct affect_data {//: entity_data {
  AFFECT_DATA *next; // next affect

  int type;          // ability number (sn) if AFFECT_ABILITY set
  int flags;         // affect prop. such as AFFECT_ABILITY, AFFECT_PERMANENT (duration non-revelant), ...
  int duration;      // affect duration
  int level;         // affect level
  int casting_level; // affect casting level, revelant only if AFFECT_ABILITY set
  
  AFFECT_LIST *list; // list of affects linked to this general affect
  
  //CLASS_DATA *program;   // just an idea, affect would have trigger such as onHitted, onHitting, ...
  //FIELD_DATA *ex_fields; //  -> could remove sanctuary, protect evil, ... test
};

struct affect_list {
  int where;     // AFTO_CHAR, ...
  int location;  // ATTR_XXX, non-revelant is AFFECT_ABILITY not set
  int op;        // AFOP_ADD, ...
  long modifier; // depends on location
  
  AFFECT_LIST *next; // next affect
};

// permanent non-dispellable duration such as sprite invisibility and
//  quickling haste, SinaC 2001  NOT ANYMORE NEEDED, replaced with flags
//#define DURATION_PERMANENT (-99)


// where definitions
#define AFTO_CHAR       0
#define AFTO_OBJECT     1
#define AFTO_WEAPON     2
#define AFTO_OBJVAL     3
// Added by SinaC 2001 for room affects
#define AFTO_ROOM       4

// op definitions
#define AFOP_ADD        0
#define AFOP_OR         1
#define AFOP_ASSIGN     2
// Added by SinaC 2001
#define AFOP_NOR        3

// Macro to ease building of affects

// Create a new general affect
#define createaff(gaf,dur,lev,typ,clev,fl)\
(gaf).duration = (dur);\
(gaf).level = (lev);\
(gaf).type = (typ);\
(gaf).casting_level = (clev);\
(gaf).flags = (fl)|AFFECT_IS_VALID;\
(gaf).list = NULL

// Add an affect to a general affect
#define addaff(gaf,wh,loc,opc,mod)\
do{\
  AFFECT_LIST *add_af = new AFFECT_LIST;\
  add_af->where = AFTO_##wh;\
  add_af->location = ATTR_##loc;\
  add_af->op = AFOP_##opc;\
  add_af->modifier = (mod);\
  add_af->next = (gaf).list;\
  (gaf).list = add_af;\
}while(false)

#define addaff2(gaf,wh,loc,opc,mod)\
do{\
  AFFECT_LIST *add_af = new AFFECT_LIST;\
  add_af->where = (wh);\
  add_af->location = (loc);\
  add_af->op = (opc);\
  add_af->modifier = (mod);\
  add_af->next = (gaf).list;\
  (gaf).list = add_af;\
}while(false)

// affect_to_char, affect_to_obj and affect_to_room add a general affect to an entity
//  see handler.C

// Added by SinaC 2001
/*
#define newafsetup(af,wh,lo,opc,mod,dur,lev,typ,tlev) \
(af).where       = AFTO_##wh; \
(af).location    = ATTR_##lo; \
(af).op          = AFOP_##opc;\
(af).modifier    = (mod);\
(af).duration    = (dur);\
(af).level       = (lev);\
(af).type        = (typ);\
(af).casting_level = (tlev);\
(af).flags = AFFECT_ABILITY;
*/

/*
// Modified by SinaC 2000
#define afsetup(af,wh,lo,opc,mod,dur,lev,typ) \
(af).where       = AFTO_##wh; \
(af).location    = ATTR_##lo; \
(af).op          = AFOP_##opc;\
(af).modifier    = (mod);\
(af).duration    = (dur);\
(af).level       = (lev);\
(af).type        = (typ);\
(af).casting_level = 0;\
(af).flags = AFFECT_ABILITY;
*/
/*
// Modified by SinaC 2000
#define afcopy(newaf,oldaf) \
(newaf).where       = (oldaf).where; \
(newaf).location    = (oldaf).location; \
(newaf).op          = (oldaf).op; \
(newaf).modifier    = (oldaf).modifier; \
(newaf).duration    = (oldaf).duration;\
(newaf).level       = (oldaf).level;\
(newaf).type        = UMAX(0,(oldaf).type);\
(newaf).casting_level = (oldaf).casting_level;\
(newaf).flags = (oldaf).flags;
*/

// old where definitions
// removed by SinaC 2001 not used anymore
//#define OLD_TO_AFFECTS      0
//#define OLD_TO_OBJECT       1
//#define OLD_TO_IMMUNE       2
//#define OLD_TO_RESIST       3
//#define OLD_TO_VULN         4
//#define OLD_TO_WEAPON       5


// A kill structure (indexed by level).
struct	kill_data {
  int		number;
  int		killed;
};



// RT ASCII conversions -- used so we can have letters in this file
// 1 << x (x:0-->30) would be better
#define A		  	(1)
#define B			(2)
#define C			(4)
#define D			(8)
#define E			(16)
#define F			(32)
#define G			(64)
#define H			(128)

#define I			(256)
#define J			(512)
#define K		        (1024)
#define L		 	(2048)
#define M			(4096)
#define N		 	(8192)
#define O			(16384)
#define P			(32768)

#define Q			(65536)
#define R			(131072)
#define S			(262144)
#define T			(524288)
#define U			(1048576)
#define V			(2097152)
#define W			(4194304)
#define X			(8388608)

#define Y			(16777216)
#define Z			(33554432)
#define aa			(67108864)
#define bb			(134217728)
#define cc			(268435456) 
#define dd			(536870912)
#define ee			(1073741824)



// Conditions.
#define COND_DRUNK		      0
#define COND_FULL		      1
#define COND_THIRST		      2
#define COND_HUNGER		      3



// Positions.
#define POS_DEAD		      0
#define POS_MORTAL		      1
#define POS_INCAP		      2
// Added by SinaC 2003: POS_PARALYZED added, others have been updated by 1
// POS_PARALYZED is almost the same as POS_STUNNED ... in fact it's the same ...
//  it's maybe more contraignant than stunned
#define POS_PARALYZED                 3
#define POS_STUNNED		      4
#define POS_SLEEPING		      5
#define POS_RESTING		      6
#define POS_SITTING		      7
#define POS_FIGHTING		      8
#define POS_STANDING		      9
//#define POS_SLEEPING		      4
//#define POS_RESTING		      5
//#define POS_SITTING		      6
//#define POS_FIGHTING		      7
//#define POS_STANDING		      8



// ACT bits for players.
#define PLR_IS_NPC		(A)		// Don't EVER set.

// RT auto flags
#define PLR_AUTOASSIST		(C)
#define PLR_AUTOEXIT		(D)
#define PLR_AUTOLOOT		(E)
#define PLR_AUTOSAC             (F)
#define PLR_AUTOGOLD		(G)
#define PLR_AUTOSPLIT		(H)
#define PLR_AUTOTITLE           (I)
// SinaC 2003
#define PLR_AUTOHAGGLE          (J)
// 4 bits reserved, J-M 
//#define PLR_QUESTOR		(J)  no longer used; replaced with script
//#define PLR_LEADER              (K)  //Oxtal> No longer used; new clan system

// RT personal flags
#define PLR_HOLYLIGHT		(N)
#define PLR_CANLOOT		(P)
#define PLR_NOSUMMON		(Q)
#define PLR_NOFOLLOW		(R)
// Colour stuff by Lope of Loping Through The MUD
// Colour Flag By Lope
#define PLR_COLOUR		(T)
// 1 bit reserved, S 

// penalty flags
#define PLR_PERMIT		(U)
#define PLR_LOG			(W)
#define PLR_DENY		(X)
#define PLR_FREEZE		(Y)
#define PLR_THIEF		(Z)
#define PLR_KILLER		(aa)
//Added by SinaC 2000 for gambling game, removed by SinaC 2003
//#define PLR_GAMBLER             (bb)
//Added by SinaC 2000 to allow Immortals to become Mortal
#define PLR_MORTAL              (cc)
 

// RT comm flags -- may be used on both mobs and chars
#define COMM_QUIET              (A)
#define COMM_DEAF            	(B)
#define COMM_NOWIZ              (C)
#define COMM_NOAUCTION          (D)
// Gossip replaced with OOC by SinaC 2001
//#define COMM_NOGOSSIP           (E)
#define COMM_NOOOC              (E)
#define COMM_NOQUESTION         (F)
//#define COMM_NOMUSIC            (G)   removed by SinaC 2003
// SinaC 2003, same as COMM_BUILDING but editing datas
#define COMM_EDITING            (G)

#define COMM_NOCLAN		(H)
#define COMM_NOQUOTE		(I)
#define COMM_SHOUTSOFF		(J)
#define COMM_NOTRIVIA           (K)

// 4 channels reserved, H-K
// Darn we had some COMM with the same number, SinaC 2000
//#define COMM_NOTRIVIA           (I)
//#define COMM_NOPARLE            (J)
//#define COMM_NOSPREEK           (K)

// display flags
#define COMM_COMPACT		(L)
#define COMM_BRIEF		(M)
#define COMM_PROMPT		(N)
#define COMM_COMBINE		(O)
#define COMM_TELNET_GA		(P)
#define COMM_SHOW_AFFECTS	(Q)
#define COMM_NOGRATS		(R)

// penalties 
#define COMM_NOEMOTE		(T)
#define COMM_NOSHOUT		(U)
#define COMM_NOTELL		(V)
#define COMM_NOCHANNELS		(W) 
#define COMM_SNOOP_PROOF	(Y)
#define COMM_AFK		(Z)
// Hints -- Oxtal
#define COMM_NOHINTS		(aa)
// BUILDING flag, SinaC 2000
#define COMM_BUILDING           (bb)
// Added by SinaC 2001 for IC channel
#define COMM_NOIC               (cc)
// Added by SinaC 2001
#define COMM_TICK               (dd)


// struct for alignment and etho, added by SinaC 2000
struct align_info {
  int etho;        // etho:  chaotic(-1)/neutral(0)/lawful(1)
  int alignment;   // align: -1000 .. -350 .. 350 .. 1000
                   //           evil    neutral    good 
};

// struct for gods, added by SinaC 2000
struct god_data: OLCEditable {
  // god name
  const char *name;

  // god title
  const char *title;

  // god story
  const char *story;

  // allowed etho/alignements
  int nb_allowed_align;
  ALIGN_INFO *allowed_align;
  //int             allowed_align;       // bit 0, 1, 2  are for etho      check constants
                                         // bit 3, 4, 5  are for alignment

  // allowed races
  int nb_allowed_race;
  int *allowed_race;

  // allowed classes
  //int nb_allowed_class;
  //int *allowed_class;
  int allowed_class; // bit vector

  // additional skill
  //int sn; Removed by SinaC 2003
  // Added by SxinaC 2003
  int minor_sphere; // sphere is a group only available for certain classes following that god
                    // this minor sphere is automatically added to the player
  int nb_major_sphere; // number of major sphere god follower with certain classes can get
  int *major_sphere; // spheres available to 
  //int nb_priest; // number of classes able to get spheres
  //int *priest; // list of classes getting sphere (cleric, shaman)
  int priest; // bit vector
    
  bool acceptWildMagic; // SinaC 2003: true if god accept wild magicians as followers

  // Added by SinaC 2001, brand mark is a special item given by the god when he/she brand the player
  //int brand_mark;
};

// Prototype for a mob.
// This is the in-memory version of #MOBILES.
struct	mob_index_data: OLCEditable {
  MOB_INDEX_DATA *	next;
  SPEC_FUN *		spec_fun;
  SHOP_DATA *		pShop;
  AREA_DATA *		area;		// OLC
  int		        vnum;
  int		        group;
  bool		        new_format;
  int		        count;
  int		        killed;
  // Added by SinaC 2001 to store what are the most dangerous mob
  int                   killed_by;

  const char *		player_name;
  const char *		short_descr;
  const char *		long_descr;
  const char *		description;
  long		act;

  long		affected_by;
  // Added by SinaC 2001
  long		affected2_by;
  // Modified by SinaC 2000 for alignment and etho
  //int		alignment;
  ALIGN_INFO    align;
  int		level;
  int		hitroll;
  int		hit[3];
  int		mana[3];
  // Added by SinaC 2001 for mental user
  int		psp[3];

  int		damage[3];
  int		ac[4];
  int 		dam_type;
  long		off_flags;
  long		imm_flags;
  long		res_flags;
  long		vuln_flags;
  int		start_pos;
  int		default_pos;
  int		sex;
  int		race;
  long		wealth;
  long		form;
  long		parts;
  int		size;
  const char *		material;
  CLASS_DATA *	program; // Oxtal

  // Added by SinaC 2001 for disease
  //long          disease; removed by SinaC 2003

  // Added by SinaC 2000 for mobile classes
  long           classes;
  
  //  FACTION_DATA*  faction;
  int            faction; // the faction this char belongs to.

  // SinaC 2003: mob_index_data may have affects
  AFFECT_DATA    *affected;
};

struct attr_type {
  char                          name[20];
  struct flag_type *      bits;
  int			default_op;
};

#define ATTR_sex           0
#define ATTR_classes       1
#define ATTR_race          2
#define ATTR_max_hit       3
#define ATTR_max_mana      4
#define ATTR_max_move      5
#define ATTR_imm_flags     6
#define ATTR_res_flags     7
#define ATTR_vuln_flags    8
#define ATTR_affected_by   9
#define ATTR_saving_throw  10
#define ATTR_hitroll       11
#define ATTR_damroll       12
#define ATTR_AC_PIERCE     13
#define ATTR_AC_BASH       14
#define ATTR_AC_SLASH      15
#define ATTR_AC_MAGIC      16
#define ATTR_STR           17
#define ATTR_INT           18
#define ATTR_WIS           19
#define ATTR_DEX           20
#define ATTR_CON           21
#define ATTR_parts         22
#define ATTR_form          23
#define ATTR_size          24
#define ATTR_DICE_NUMBER   25
#define ATTR_DICE_TYPE     26
#define ATTR_DICE_BONUS    27
#define ATTR_dam_type      28
// Added by SinaC 2001
#define ATTR_affected2_by  29
#define ATTR_etho          30
#define ATTR_alignment     31
// Added by SinaC 2001
#define ATTR_max_psp           32
// Added by SinaC 2001
//#define ATTR_disease       33  removed by SinaC 2003

#define ATTR_ac0           ATTR_AC_PIERCE
#define ATTR_ac1           (ATTR_ac0+1)
#define ATTR_ac2           (ATTR_ac0+2)
#define ATTR_ac3           (ATTR_ac0+3)
#define ATTR_stat0         ATTR_STR

#define ATTR_NUMBER        33


#define ATTR_allAC         33
#define ATTRLOC_NUMBER     34
#define ATTR_NA            ATTRLOC_NUMBER



#define ATTR_0             0 // for object values modifiers
#define ATTR_1             1
#define ATTR_2             2
#define ATTR_3             3
#define ATTR_4             4

// easy access
#define cstat(id) curattr[ATTR_##id]
#define bstat(id) baseattr[ATTR_##id]

// Added by SinaC 2000 for beacon spell
#define MAX_BEACONS  (3)


// A 'script handled' structure, Oxtal (JyP) & SinaC
#define CHAR_ENTITY 0
#define OBJ_ENTITY 1
// Added by SinaC 2003 for room program
#define ROOM_ENTITY 2

// An entity is a super class englobing Char, Object and Room
// It's mainly used in script
struct entity_data {
  CLASS_DATA* clazz;
  FIELD_DATA* ex_fields;  
  int kind; // char/obj/room
  const char *		name;
  bool        valid; // false if entity has been extracted
};


// One character (PC or NPC).
struct	char_data: entity_data {

  char_data() {
    kind = CHAR_ENTITY;
    valid = TRUE;
  }

  CHAR_DATA *		next;
  CHAR_DATA *		next_in_room;
  CHAR_DATA *		master;
  CHAR_DATA *		leader;
  CHAR_DATA *		fighting;
  CHAR_DATA *		reply;
  CHAR_DATA *		pet;
  //  MEM_DATA *		memory;  not anymore defined/used
  SPEC_FUN *		spec_fun;
  MOB_INDEX_DATA *	pIndexData;
  DESCRIPTOR_DATA *	desc;
  AFFECT_DATA  *	affected;
  OBJ_DATA *		carrying;
  OBJ_DATA *		on;
  ROOM_INDEX_DATA *	in_room;
  ROOM_INDEX_DATA *	was_in_room;
  AREA_DATA *		zone;
  PC_DATA *		pcdata;
  GEN_DATA *		gen_data;
  long		        id;
  int		        version;
  //char *		name;
  const char *		short_descr;
  const char *		long_descr;
  const char *		description;
  const char *		prompt;
  const char *		prefix;
  int		        group;
  int		        clan;
  int		        clan_status;
  int		        petition;
  int		        level;
  int		        trust;
  int			played;
  int			lines;  // for the pager
  time_t		logon;
  int		        timer;
  int	 	        wait;
  int		        daze;
  int		        hit;
  int		        mana;
  // Added by SinaC 2001 for mental user
  int                   psp;
  int		        move;
  long		        gold;
  long		        silver;
  int			exp;
  long		        act;
  long		        comm;   // RT added to pad the vector
  long		        wiznet; // wiz stuff
  int		        invis_level;
  int		        incog_level;
  int		        position;
  int		        practice;
  int		        train;
  int		        carry_weight;
  int		        carry_number;
  // Modified by SinaC 2000, alignment and etho now
  //int		        alignment;
  // Removed by SinaC 2001, etho/alignment are in attributes now
  //ALIGN_INFO            align;
  int		        wimpy;
  const char*		        material;
  // mobile stuff
  long		        off_flags;
  int		        start_pos;
  int		        default_pos;

  // Modified by SinaC 2001 long before
  long 	                baseattr[ATTR_NUMBER];  // by Oxtal
  long 	                curattr[ATTR_NUMBER];

  CHAR_DATA*            hunting;          // Used by hunting routine

  // Added by SinaC 2000
  int                   stunned;
  
  //FACTION_DATA*         faction; // the faction this char belongs to.
  int                   faction;  // the faction this char belongs to.

  int                   ptr_before_copyover; // not used for the moment, SinaC 2003

  // SinaC 2003, those bits are computed in recompute
  // and avoid computing repetitive code such as unarmed/unarmored, ability_upgrade, ...
  long                  optimizing_bit;

  // SinaC 2003
  bool                  isWildMagic; // true if player has a wildable class and choose it at creation
};

// Added by SinaC 2000
struct ability_info {
  int learned;           // ability percentage
  int casting_level;     // casting level of the ability
  int level;             // level which the player will get the ability
};

// Added by SinaC 2001 for player disabled cmd
struct disabled_cmd_data {
  DISABLED_CMD_DATA *next;
  
  struct cmd_type *cmd; // pointer in the command table
  const char *disabled_by;               // Name of disabler
};

#define MUD_KNOWLEDGE_NOT_KNOW            (0)
#define MUD_KNOWLEDGE_ALREADY_PLAYED_ONE  (1)
#define MUD_KNOWLEDGE_ALREADY_PLAYED_THIS (2)

// Data which only PC's have.
struct	pc_data {
  BUFFER * 		buffer;
  const char *		pwd;
  const char *		bamfin;
  const char *		bamfout;
  const char *		title;
  time_t                last_changes;
  int			last_level;
  int		        condition	[4];
  
//    //  Modified by SinaC 2000
//    int *                 learned;
//    //  Added by SinaC 2000 to store skill/spell level 
//    int *                 learned_lvl;
   
// Modified by SinaC 2000
  ABILITY_INFO *          ability_info;

  // Modified by SinaC 2000
  bool *group_known;

  int		        points;
  bool              	confirm_delete;
  const char *		alias[MAX_ALIAS];
  const char * 		alias_sub[MAX_ALIAS];
  int 		        security;	// OLC Builder security

  int                   trivia;         // Trivia points
// Removed by SinaC 2001, use extra fields now
//  long                  acctgold;		// Bank account
//  long		        acctsilver;

  // QUEST CODE,  no longer used  replaced with script and extra fields
  //  CHAR_DATA *           questgiver; // Vassago
  //  int                   questpoints;  // Vassago
  //  int                   nextquest; // Vassago
  //  int                   countdown; // Vassago
  //  int                   questobj; // Vassago
  //  int                   questmob; // Vassago
  //  int                   questobjloc; // added SinaC 2001

  BOARD_DATA *	        board; // The current board
  time_t		last_note[MAX_BOARD]; // last note for the boards
  NOTE_DATA *		in_progress;
    
  int                   hometown;	// Oxtal
  
  // Added by SinaC 2000 to allow immortals to have something
  //  different in the who and whois that IMP, IMM, ...
  const char *                immtitle;
  
  // Added by SinaC 2000, it's quite fun to be able to put anything
  //  we want people are seeing when they are in the same room as us
  const char *                stance;
  
  // which god we follow ? Index in god_table
  int                   god;

  // for beacon spell by SinaC 2000
  int                   beacons[MAX_BEACONS]; 

  // Added by SinaC 2001 for player disabled commands
  DISABLED_CMD_DATA *disabled_cmd;

  // This field is false till a god accept the name, Added by SinaC 2001
  bool                  name_accepted;
  // This is field is false if the player have not been branded by
  //  his/her god. The brand allows the player to use the godspell
  //bool                  branded; Removed by SinaC 2003

  // Added by SinaC 2001, for gset   goto set
  int                   goto_default;

  // Added by SinaC 2001
  int                   language;

  int                   base_faction_friendliness[MAX_FACTION]; // does faction x like this PC ?
  int                   current_faction_friendliness[MAX_FACTION]; // does faction x like this PC ?

  int                   remort_count; // number of remort already done

  int                   mud_knowledge; // degree of mud knowledge

  // moved by SinaC 2003, previously in char_data
  // Arena SinaC 2000
  CHAR_DATA*            betted_on;
  int                   bet_amt;
  CHAR_DATA*            challenged;

  // SinaC 2003, used to store rebirth race of player which leave before end of rebirth
  int                   tmpRace;
};

// Data for generating characters -- only used during generation
struct gen_data {
  //  GEN_DATA *            next;
  // Modified by SinaC 2000
  bool *                ability_chosen;

  // Modified by SinaC 2000
  bool *                group_chosen;
  int		        points_chosen;

  // Modified by SinaC 2001 to count the number of weapon choosable
  int                   count;

  // Mud's player knowledge
  int                   mud_knowledge;
};



// Liquids.
struct	liq_type: OLCEditable {
  const char *	liq_name;
  const char *	liq_color;
  int	liq_affect[5];
};


// Extra description data for a room or object.
struct	extra_descr_data {
  EXTRA_DESCR_DATA *next;	// Next in list                     
  const char *keyword;              // Keyword in look/examine          
  const char *description;          // What to see                      
};

// struct for restriction by SinaC 2000
struct restr_data {
  RESTR_DATA *next;
  
  // restriction type RESTR_STR .. RESTR_ABILITY
  int type;
  // restriction value
  int value;
  
  // Added by SinaC 2000 so we can make restriction like not being en elf, can't be 
  //  a certain class, ...
  int not_r;

// Added by SinaC 2000 so we can make restriction on ability
//  when ability_r is set, value retains the ability percentage needed and sn retains
//  the ability number
  int sn; // ability_r doesn't exist anymore but type = RESTR_ABILITY gives us the same information

};

// constants for object's requirements, Added by SinaC 2000
// Be careful when modifying this: restr_table and restr_attr index must correspond to these values
//  first must be strength, second intelligence, 8th form, ...
// integer between 0 and 30
#define RESTR_STR      0
#define RESTR_INT      1
#define RESTR_WIS      2
#define RESTR_DEX      3
#define RESTR_CON      4
// check sex_flags in tables.C
#define RESTR_SEX      5
// check classes_flags
#define RESTR_CLASSES  6
// check race_flags
#define RESTR_RACE     7
// check part_flags
#define RESTR_PART     8
// check form_flags
#define RESTR_FORM    9
// check etho
#define RESTR_ETHO    10
// check alignment
#define RESTR_ALIGN   11
// check for ability
#define RESTR_ABILITY  12



// struct for ability upgrade, added by SinaC 2000
struct ability_upgrade {
  ABILITY_UPGRADE *next;
  // ability number
  int sn;
  // value of the upgrade
  int value;
};

// Prototype for an object.
struct	obj_index_data: OLCEditable {
  OBJ_INDEX_DATA *	next;
  EXTRA_DESCR_DATA *	extra_descr;
  AFFECT_DATA *	        affected;
  AREA_DATA *		area;		// OLC
  bool		        new_format;
  const char *		name;
  const char *		short_descr;
  const char *		description;
  int		        vnum;
  int		        reset_num;
  // Modified by SinaC 2001 ==> index in material_table
  //char *		material;
  int                   material;

  int		        item_type;
  int			extra_flags;
  int			wear_flags;
  int		        level;
  int 		        condition;
  int		        count;
  int		        weight;
  int			cost;
  int			value[5];
  // Added by SinaC 2003, size is not anymore a restriction but an obj stat
  int                   size;

  // Added by SinaC 2000 for restriction
  RESTR_DATA *          restriction;
  ABILITY_UPGRADE *     upgrade;

  CLASS_DATA*           program;
};



// One object.
struct	obj_data: entity_data {

  obj_data() {
    kind = OBJ_ENTITY;
    valid = TRUE;
  }

  OBJ_DATA *		next;
  OBJ_DATA *		next_content;
  OBJ_DATA *		contains;
  OBJ_DATA *		in_obj;
  OBJ_DATA *		on;
  CHAR_DATA *		carried_by;
  EXTRA_DESCR_DATA *	extra_descr;
  AFFECT_DATA *	        affected;
  OBJ_INDEX_DATA *	pIndexData;
  ROOM_INDEX_DATA *	in_room;
  bool		        enchanted;
  const char *	        owner;
  //char *		name;
  const char *		short_descr;
  const char *		description;
  int		        item_type;
  int                   base_extra; //Oxtal
  int			extra_flags;
  int			wear_flags;
  int		        wear_loc;
  int		        weight;
  int			cost;
  int		        level;
  int 		        condition;
  // Modified by SinaC 2001 ==> index in material_table
  //char *		material;
  int                   material;

  int		        timer;
  int                   baseval [5]; // Oxtal
  int			value	[5];
  // Added by SinaC 2003, size is not anymore a restriction but an obj stat
  int                   size;

  // Added by SinaC 2000 for restriction
  // SinaC 2003: not needed because they can't be modified during play
  //  RESTR_DATA *          restriction;
  //  ABILITY_UPGRADE *     upgrade;
};




// Exit data.
// Moved in olc_value.h
//#define MAX_DIR	6          Modified by SinaC 2003, 4 additional directions NE, NW, SE, SW
//#define MAX_DIR (10)
// D0: NORTH
// D1: EAST
// D2: SOUTH
// D3: WEST
// D4: UP
// D5: DOWN
// D6: NORTHEAST
// D7: NORTHWEST
// D8: SOUTHEAST
// D9: SOUTHWEST
// D10: SPECIAL
#define MAX_DIR (11)

struct	exit_data {
  union {
    ROOM_INDEX_DATA *	to_room;
    int			vnum;
  } u1;
  int		        exit_info;
  int		        key;
  const char *		keyword;
  const char *		description;
  //  EXIT_DATA *		next;		// OLC
  int			rs_flags;	// OLC
  int			orig_door;	// OLC
};




// Reset commands:
//   '*': comment
//   'M': read a mobile 
//   'O': read an object
//   'P': put object in object
//   'G': give object to mobile
//   'E': equip object to mobile
//   'D': set state of door
//   'R': randomize room exits
//   'S': stop (end of list)
// Area-reset definition.
struct	reset_data {
  RESET_DATA *	next;
  char		command;
  int		arg1;
  int		arg2;
  int		arg3;
  int		arg4;
};



// Area definition.
struct	area_data: OLCEditable {
  AREA_DATA *		next;
  RESET_DATA *	        reset_first;
  RESET_DATA *	        reset_last;
  const char *		file_name;
  const char *		name;
  const char *		credits;
  int		        age;
  int		        nplayer;
  int		        low_range;
  int		        high_range;
  int 		        min_vnum;
  int		        max_vnum;
  bool		        empty;
  const char *		builders;	// OLC  Listing of 
  int			vnum;		// OLC  Area vnum 
  int			area_flags;	// OLC 
  int			security;	// OLC  Value 1-9 
  int                   minlevel;       // Minimum level for entry 
  long                  totalnplayer;   // Oxtal : sum of nplayers trough time 
  
  // Added by SinaC 2000
  // when a room in a area has ROOM_TELEPORT set, it teleports
  //  to that room  
  //int                   teleport_room;  removed by SinaC 2003, scripts can do that

  // Added by SinaC 2000
  // there is an earthquake in the area
  // how many time left till the end of the earthquake
  bool                  earthquake_on;
  int                   earthquake_duration;
};




// Constants for room affects
#define ATTR_flags       (0)
#define ATTR_light       (1)
#define ATTR_sector      (2)
#define ATTR_healrate    (3)
#define ATTR_manarate    (4)
#define ATTR_psprate     (5)
#define ATTR_maxsize     (6)
// Number of room attributes
#define MAX_ROOM_ATTR    (7)

// Room type.
// Modified by SinaC 2003 for room program
struct	room_index_data : entity_data, OLCEditable {
  room_index_data() {
    kind = ROOM_ENTITY;
    valid = TRUE;
  }

  ROOM_INDEX_DATA *	next;
  CHAR_DATA *		people;
  OBJ_DATA *		contents;
  EXTRA_DESCR_DATA *	extra_descr;
  AREA_DATA *		area;
  //EXIT_DATA *		exit	[6];
  //EXIT_DATA * 	        old_exit[6];
  // Modified by SinaC 2003
  EXIT_DATA *		exit	[MAX_DIR];
  EXIT_DATA * 	        old_exit[MAX_DIR];
  RESET_DATA *	        reset_first;	// OLC
  RESET_DATA *	        reset_last;	// OLC
  //  const char *		name;
  const char *		description;
  const char *		owner;
  long	                guild; // Oxtal
  int		        vnum;
  //int			room_flags;
  //int		        light;
  //int		        sector_type;
  //int		        heal_rate;
  //int 		mana_rate;
  // Added by SinaC 2001 for mental user
  //int                 psp_rate;
  int		        clan;

  // Added by SinaC 2001, maximal size of the room
  // people greater than that size can't go to that room
  //int                   max_size;

  //Added by SinaC 2001 for room affects
  int                   baseattr[MAX_ROOM_ATTR];
  int                   curattr[MAX_ROOM_ATTR];

  AFFECT_DATA          *base_affected;    // Affects add in room edition
  AFFECT_DATA          *current_affected; // Affects add with abilities

  // Added by SinaC 2003 for room program, don't need this field
  //  but has been added so the code is more uniform
  // If a day, we decide to add rooms on the fly, we will need that
  //  room_data will be different from room_index_data
  CLASS_DATA *program;

  // Added by SinaC 2003 for modifiable repop time
  int current_time_repop;                  // time elapsed since last repop
  int time_between_repop,           // time between 2 repops if nobody is in the area
    time_between_repop_people;      // time between 2 repops if someone is in the area
};


// Types of attacks.
// Must be non-overlapping with spell/skill types,
// but may be arbitrary beyond that.
#define TYPE_UNDEFINED               -1
#define TYPE_HIT                     10000



// Target types.
#define TAR_IGNORE		    0
#define TAR_CHAR_OFFENSIVE	    1
#define TAR_CHAR_DEFENSIVE	    2
#define TAR_CHAR_SELF		    3
#define TAR_OBJ_INV		    4
#define TAR_OBJ_CHAR_DEF	    5
#define TAR_OBJ_CHAR_OFF	    6

#define TARGET_CHAR		    0
#define TARGET_OBJ		    1
#define TARGET_ROOM		    2
#define TARGET_NONE		    3

// Constants for mob_use in ability_type, bit vector SinaC 2003
// Ability can be used if mob is charmed
#define MOB_USE_CHARMED             (A)
// Ability can be used from script
#define MOB_USE_SCRIPT              (B)
// Automatically used by mob in combat
#define MOB_USE_COMBAT              (C)
//#define MOB_USE_NONE                0
//#define MOB_USE_COMBAT              1
//#define MOB_USE_AUTOMATIC           2
//#define MOB_USE_NORMAL              3


// structures for prereq by SinaC 2000, modified by SinaC 2003
// see example in ability_type
struct prereq_list {
  int sn;            // ability number
  int casting_level; // casting ability level
};

struct prereq_data {
  int  classes;         // classes allowed to get this casting level
  int  cost;            // number of trains to buy this casting level
  int  plr_level;       // player level, only used if > 0 ... rarely used
  int  nb_prereq;       // number of prereqs + 1, from level 0 to level X, level 0 is to gain it
  PREREQ_LIST *prereq;  // "list" of the prereq for that ability casting level
};


// Added by SinaC 2000
// Init ability type: name, pgsn and spell_fun
struct	ability_type_init {
  const char *	name;			// Name of ability		
  
  void *  action_fun; // will be converted into do_fun, spell_fun or power_fun
                      // depending on type in ability_type

  // Removed by SinaC 2001, use a void * instead
  //SPELL_FUN *	spell_fun;		// Spell pointer (for spells)	
  int *	pgsn;			// Pointer to associated gsn	

  // Removed by SinaC 2001, use a void * instead
  // Added by SinaC 2001 for 'USE' command
  //DO_FUN *do_fun;       // pointer to associated do_fun

  int nb_casting_level; // number of level for that ability

  // Added by SinaC 2003, for clean affect wear_off and update
  UPDATE_FUN * update_fun;    // called each tick when affected by the ability
  WEAROFF_FUN * wearoff_fun;  // called when duration = 0 or when dispelled if affected by
                              //  ability
};


// ability_type is used as skill_type, spell_type, power_type and song_type
struct	ability_type: OLCEditable {
  const char *	name;			// Name of ability		
  // Modified by SinaC 2000
  int *         ability_level;	        // Level needed by class	

  // Modified by SinaC 2000
  int *         rating;	// How hard it is to learn	
  // > 0: can gain this ability and can use it
  // = 0: can't gain this ability, neither use it unless ability_info[sn].level > 0
  //       if the spell has been learned later than creation with command teach or study ability
  // < 0: can't gain this ability, can only be accessed through in a group
  //       to get how hard it is to learn: |rating|

  void *        action_fun;             // action_fun is pointer to ability method
                                        // skill/spell/power/song
  // Removed by SinaC 2001, use a void * instead
  //SPELL_FUN *	spell_fun;		// Spell pointer (for spells)	
  int	        target;			// Legal targets		
  int	        minimum_position;	// Position for caster / user	
  int *	        pgsn;			// Pointer to associated gsn	
  int	        slot;			// Slot for #OBJECT loading	
  // min_mana and min_psp are the same, SinaC 2001 for mental user
  //int	        min_mana;		// Minimum mana used		
  int           min_cost;               // Minimum cost in mana or psp
  int	        beats;			// Waiting time after use	
  const char *	noun_damage;		// Damage message		
  const char *	msg_off;		// Wear off message		
  const char *	msg_obj;		// Wear off message for objects
  // Added by SinaC 2001 for room affects
  const char *        msg_room;               // Wear off message for rooms
  

  // Modified by SinaC 2001
  // So certain ability are allowed for mobs
  //bool          mob_usable;
//  int mob_use;   // 0: not used by mob                                       (MOB_USE_NONE)
//                 // 1: can be used in combat by the mob itself               (MOB_USE_COMBAT)
//                 // 2: automatic like second attack, fast healing, dodge, ...(MOB_USE_AUTOMATIC)
//                 // 3: can be use through order or scripts                   (MOB_USE_NORMAL)

  // mob_use is a bit vector, check values above
  int           mob_use;

  // Added by SinaC 2000 for ability casting level
  int           nb_casting_level;
  // Added by SinaC 2000 for prereqs
  PREREQ_DATA  *prereqs;    // NULL means no prereq

  // example:
  // fireball has a casting level of 3
  // prereq = { 
  //            { 1, 0, NULL },
  //            { 2, 2, { { fireproof, 1 }, { chill touch, 1 } } }, 
  //            { 3, 1, { { fireproof, 2 } } }
  //          }
  //
  //   means    fireball level 1: 0 prereq  : /
  //            fireball level 2: 2 prereqs : fireproof lvl 1 and chill touch lvl 1
  //            fireball level 3: 1 prereq  : fireproof lvl 2

  // Removed by SinaC 2001, use a void * instead
  // Added by SinaC 2001 for 'USE' command
  //DO_FUN *do_fun;

  // Added by SinaC 2001 for power for mental user, see constants
  // if type = TYPE_SKILL   action_fun will be converted into DO_FUN
  //         = TYPE_SPELL   action_fun will be converted into SPELL_FUN
  //         = TYPE_POWER   action_fun will be converted into POWER_FUN
  //         = TYPE_SONG    action_fun will be converted into SONG_FUN
  int type;

  bool dispellable;  // true if the spell can be dispelled/cancelled
  const char *msg_dispel;  // msg when the spell is dispelled

  // Added by SinaC 2003
  // Time to wait before being able to use this ability again, used for songs
  // Will be used for spells like: animate dead, summon lesser golem, ...
  int to_wait;
  // May this ability be put in a staff/pill/wand/scroll...
  int craftable;

  // Added by SinaC 2003, for clean affect wear_off and update
  UPDATE_FUN * update_fun;    // called each tick when affected by the ability
  WEAROFF_FUN * wearoff_fun;  // called when duration = 0 or when dispelled if affected by
                              //  ability

  bool scriptAbility; // if true: ability is not in ability_table_init
                      //          it's an ability created from a special script charSpell

  int school; // school index, only for spells
};

struct  group_type: OLCEditable {
  const char *	name;
  int *         rating; // < 0: means can't get this group   > 0: can get this group
                        // = 0: means can't gain this group but can be added as a base group
                        // rom basics, class basics, minor sphere, ...
  // Added by SinaC 2000
  int           spellnb;
  const char **       spells;

  // Added by SinaC 2003 to create groups available only if a player has a certain god
  int *god_rate;  
  // <= 0: means can't get this group   
  // >0: can get this group

  // almost every group are available for every gods, only spheres
  //  will be available to only certain god followers
  bool isSphere;
};

struct magic_school_type: OLCEditable {
  const char *name; // school name
  int nb_spells; // number of spell in this school
  int *spells_list; // list of spell in this school
};


// Utility macros.

// This is the handy CH() macro. I think that it was Tom Adriansen (sp?)
#define CH(descriptor)  ((descriptor)->original ? \
(descriptor)->original : (descriptor)->character)


// Modified by SinaC 2001
//#define UMIN(a, b)		((a) < (b) ? (a) : (b))
//#define UMAX(a, b)		((a) > (b) ? (a) : (b))
//#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
inline int UMIN( const int a, const int b ) {
  if ( a < b )
    return a;
  return b;
}
inline int UMAX( const int a, const int b ) {
  if ( a > b )
    return a;
  return b;
}
inline int URANGE( const int a, const int b, const int c ) {
  if ( b < a )
    return a;
  if ( b > c )
    return c;
  return b;
}


#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
// Added by SinaC 2003
#define IS_SET_BIT(flag, bitIndex) ((flag) & (1<<(bitIndex)))


//Character macros.
#define IS_NPC(ch)		(IS_SET((ch)->act, ACT_IS_NPC))

#define IS_PC(ch)			(!IS_NPC(ch))
// By Oxtal, modified by SinaC 2000
// Added by SinaC 2001
//#define IS_DISEASED(ch,dis)     (IS_SET((ch)->cstat(disease), (dis)))  removed by SinaC 2003
#define IS_IMMORTAL(ch)		(IS_PC(ch)\
                                 &&(get_trust(ch) >= LEVEL_IMMORTAL)\
                                 &&(!IS_SET((ch)->act,PLR_MORTAL)))
#define IS_HERO(ch)		(IS_PC(ch)&&(get_trust(ch) >= LEVEL_HERO))
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(IS_SET((ch)->cstat(affected_by), (sn)))
// Added by SinaC 2001
#define IS_AFFECTED2(ch, sn)	(IS_SET((ch)->cstat(affected2_by), (sn)))

#define GET_AGE(ch)		((int) (17 + ((ch)->played \
				    + current_time - (ch)->logon )/72000))

// Modified by SinaC 2001 etho/alignment are attributes now
// Added/Modified by SinaC 2000 for align/etho
//#define IS_LAWFUL(ch)           (ch->align.etho == 1)
//#define IS_CHAOTIC(ch)          (ch->align.etho == -1)
//#define IS_NEUTRAL_ETHO(ch)     (ch->align.etho == 0)

//#define IS_GOOD(ch)		(ch->align.alignment >= 350)
//#define IS_EVIL(ch)		(ch->align.alignment <= -350)
//#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_LAWFUL(ch)           (ch->cstat(etho) == 1)
#define IS_CHAOTIC(ch)          (ch->cstat(etho) == -1)
#define IS_NEUTRAL_ETHO(ch)     (ch->cstat(etho) == 0)

#define IS_GOOD(ch)		(ch->cstat(alignment) >= 350)
#define IS_EVIL(ch)		(ch->cstat(alignment) <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define BASE_IS_LAWFUL(ch)           (ch->bstat(etho) == 1)
#define BASE_IS_CHAOTIC(ch)          (ch->bstat(etho) == -1)
#define BASE_IS_NEUTRAL_ETHO(ch)     (ch->bstat(etho) == 0)

#define BASE_IS_GOOD(ch)		(ch->bstat(alignment) >= 350)
#define BASE_IS_EVIL(ch)		(ch->bstat(alignment) <= -350)
#define BASE_IS_NEUTRAL(ch)		(!BASE_IS_GOOD(ch) && !BASE_IS_EVIL(ch))



#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)

// Modified by SinaC 2001
#define IS_OUTSIDE(ch)		(!IS_SET(				    \
				    (ch)->in_room->cstat(flags),	    \
				    ROOM_INDOORS))


#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))

// Modified by SinaC 2001
//#define MONEY_WEIGHT(gold,sil)	( (gold)*2/5+(sil)/10 )
#define MONEY_WEIGHT(gold,sil)	( (gold)/10+(sil)/40 )
#define get_carry_weight(ch)	((ch)->carry_weight + \
				MONEY_WEIGHT( (ch)->gold, (ch)->silver ))

//#define IS_QUESTOR(ch)     (IS_SET((ch)->act, PLR_QUESTOR))


// Object macros.
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))

// Oxtal
#define SET_OBJ_STAT(obj, stat)  SET_BIT((obj)->base_extra, (stat)); recompobj(obj)
#define REM_OBJ_STAT(obj, stat)  REMOVE_BIT((obj)->base_extra, (stat)); recompobj(obj)
// Modified by SinaC 2003, RANGED doesn't have special power
//#define IS_WEAPON_STAT(obj,stat) (((obj)->value[0]!=WEAPON_RANGED) && (IS_SET((obj)->value[4],(stat))))
// ranged weapon are always two-handed and has no other flags
#define IS_WEAPON_STAT(obj,stat) ( ( (obj)->value[0] == WEAPON_RANGED) ? ( (stat) == WEAPON_TWO_HANDS ? 1:0 ) : ( IS_SET((obj)->value[4], (stat)) ) )
#define GET_WEAPON_DAMTYPE(obj) ( ((obj)->value[0]==WEAPON_RANGED) ? (TYPE_THWACK) : ((obj)->value[3]) )
#define GET_WEAPON_DNUMBER(obj) ( ((obj)->value[0]==WEAPON_RANGED) ? (UMAX((obj)->level/25,1)) : ((obj)->value[1]) )
#define GET_WEAPON_DTYPE(obj) ( ((obj)->value[0]==WEAPON_RANGED) ? (UMAX((obj)->level/15,1)) : ((obj)->value[2]) )
#define GET_WEAPON_FLAGS(obj) ( ((obj)->value[0]==WEAPON_RANGED) ? (0) : ((obj)->value[4]) )

#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? \
	(obj)->value[4] : 100)


// SinaC 2001
#define BEATS(sn) (ability_table[sn].beats)

// Description macros.
#define PERS(ch, looker)	( can_see( looker, (ch) ) ?		\
				( IS_NPC(ch) ? (ch)->short_descr	\
				: (ch)->name ) : "someone" )
// SinaC 2000
#define NAME(ch)                (IS_NPC(ch)?(ch)->short_descr:(ch)->name)

// Structure for a social in the socials table.

struct	social_type {
  char      name[20];
  const char *    char_no_arg;
  const char *    others_no_arg;
  const char *    char_found;
  const char *    others_found;
  const char *    vict_found;
  const char *    char_not_found;
  const char *      char_auto;
  const char *      others_auto;
};



// Global constants.
// Modified by SinaC 2000
extern const struct attr_type *attr_table;


extern	const	struct	str_app_type	str_app		[31];
extern	const	struct	int_app_type	int_app		[31];
extern	const	struct	wis_app_type	wis_app		[31];
extern	const	struct	dex_app_type	dex_app		[31];
extern	const	struct	con_app_type	con_app		[31];

extern struct	hometown_type	*hometown_table;
// Modified by SinaC 2000
extern struct	class_type	*class_table;

extern	const	struct	weapon_type	weapon_table	[];
extern  const   struct  item_type	item_table	[];
extern	const	struct	wiznet_type	wiznet_table	[];
extern	const	struct	attack_type	attack_table	[];
// Modified by SinaC 2000, in a file now
extern	struct  race_type	*race_table;
// Modified by SinaC 2000, in a file now
extern	struct	pc_race_type	*pc_race_table;

extern  const	struct	spec_type	spec_table	[];

// Modified by SinaC 2000, it's in a file now
extern struct liq_type *liq_table;

// Modified by SinaC 2000 for classes save
extern struct	ability_type	*ability_table;

// Added by SinaC 2000
extern                  struct  ability_type_init ability_table_init[];


// Modified by SinaC 2000
extern  struct  group_type      *group_table;

extern          struct social_type      social_table	[MAX_SOCIALS];

// Modified by SinaC 2000, in a file now
extern const char ****title_table;




// Global variables.
extern		HELP_DATA	  *	help_first;
extern		SHOP_DATA	  *	shop_first;

extern		CHAR_DATA	  *	char_list;
extern		DESCRIPTOR_DATA   *	descriptor_list;
extern		OBJ_DATA	  *	object_list;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		KILL_DATA		kill_table	[];
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;


// OS-dependent declarations.
// These are all very standard library functions,
//   but some systems have incomplete or non-ansi header files.
extern "C" {
 
#if	defined(_AIX)
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(apollo)
  int	atoi		args( ( const char *string ) );
  void *	calloc		args( ( unsigned nelem, size_t size ) );
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(hpux)
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(linux)
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(macintosh)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(MIPS_OS)
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(MSDOS)
#define NOCRYPT
#if	defined(unix)
#undef	unix
#endif
#endif

#if	defined(NeXT)
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if	defined(sequent)
  char *	crypt		args( ( const char *key, const char *salt ) );
  int	fclose		args( ( FILE *stream ) );
  int	fprintf		args( ( FILE *stream, const char *format, ... ) );
  int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
  int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
  void	perror		args( ( const char *s ) );
  int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(sun)
  char *	crypt		args( ( const char *key, const char *salt ) );
  int	fclose		args( ( FILE *stream ) );
  int	fprintf		args( ( FILE *stream, const char *format, ... ) );
#if	defined(SYSV)
  siz_t	fread		args( ( void *ptr, size_t size, size_t n, 
				FILE *stream) );
#else
  int	fread		args( ( void *ptr, int size, int n, FILE *stream ) );
#endif
  int	fseek		args( ( FILE *stream, long offset, int ptrname ) );
  void	perror		args( ( const char *s ) );
  int	ungetc		args( ( int c, FILE *stream ) );
#endif

#if	defined(ultrix)
  char *	crypt		args( ( const char *key, const char *salt ) );
#endif

#if defined(__CYGWIN32__)
  //#define unix   removed by SinaC 2003
#define NOCRYPT
  // By Oxtal 1997 for Cygwin port
#endif

} // Extern "C"

// The crypt(3) function is not available on some operating systems.
// In particular, the U.S. Government prohibits its export from the
//   United States to foreign countries.
// Turn on NOCRYPT to keep passwords in plain text.
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif




// *****************************************************************************
// *                                    OLC                                    *
// *****************************************************************************

#define NO_FLAG -99LL	// Must not be used in flags or stats.



// Global Constants
extern	char *	const	dir_name        [];
extern  const   char *  short_dir_name  [];
extern	const	int	rev_dir         [];          // int - ROM OLC

// Global variables
extern          AREA_DATA *             area_first;
extern          AREA_DATA *             area_last;
extern  	SHOP_DATA *             shop_last;

extern          int                     top_affect;
extern          int                     top_area;
extern          int                     top_ed;
extern          int                     top_exit;
extern          int                     top_help;
extern          int                     top_mob_index;
extern          int                     top_obj_index;
extern          int                     top_reset;
extern          int                     top_room;
extern          int                     top_shop;

extern          int                     top_vnum_mob;
extern          int                     top_vnum_obj;
extern          int                     top_vnum_room;

extern          char                    str_empty       [1];

extern  MOB_INDEX_DATA *        mob_index_hash  [MAX_KEY_HASH];
extern  OBJ_INDEX_DATA *        obj_index_hash  [MAX_KEY_HASH];
extern  ROOM_INDEX_DATA *       room_index_hash [MAX_KEY_HASH];


// bit.c
// const has been removed by SinaC 2000
extern struct flag_type area_flags[];
extern struct flag_type	sex_flags[];
extern struct flag_type	exit_flags[];
extern struct flag_type	door_resets[];
extern struct flag_type	room_flags[];
extern struct flag_type	sector_flags[];
extern struct flag_type	type_flags[];
extern struct flag_type	extra_flags[];
extern struct flag_type	wear_flags[];
extern struct flag_type	act_flags[];
extern struct flag_type	affect_flags[];
// Added by SinaC 2001
extern struct flag_type affect2_flags[];

extern struct flag_type	wear_loc_strings[];
extern struct flag_type	wear_loc_flags[];
extern struct flag_type	container_flags[];

// ROM OLC:

//Oxtal
// Modified by SinaC 2000
extern struct flag_type   *classes_flags;

extern struct flag_type   ops_flags[];
extern struct flag_type   attr_flags[];
extern struct flag_type   afto_type[];

extern struct flag_type   form_flags[];
extern struct flag_type   part_flags[];
extern struct flag_type   ac_type[];
extern struct flag_type   size_flags[];
extern struct flag_type   off_flags[];
//IRV Immune Resistant Vulnerable, SinaC 2003
extern struct flag_type   irv_flags[];
//extern struct flag_type   imm_flags[];
//extern struct flag_type   res_flags[];
//extern struct flag_type   vuln_flags[];
extern struct flag_type   position_flags[];
extern struct flag_type   weapon_class[];
extern struct flag_type   weapon_type2[];
extern struct flag_type   furniture_flags[];
extern struct flag_type   apply_types[];
// SinaC 2003
extern struct flag_type   target_flags[];

// Added by SinaC 2000
// Modified by SinaC 2000
extern struct flag_type   *races_flags;

// Not yet used, SinaC 2001
//struct bit_type
//{
//  struct  flag_type *     table;
//  char *                          help;
//};
//
//extern struct bit_type   bitvector_type[];

// Added by SinaC 2000 for general restriction
extern const struct attr_type *restr_table;
extern struct flag_type restr_flags[];

// Added by SinaC 2001 for room affects
extern struct flag_type   room_attr_flags[];

// Added by SinaC 2000
// Used in act_comm.C and initialized in class.C
extern const char ****pose_table;

extern GOD_DATA *gods_table;

// Added by SinaC 2001 for object condition
extern const char* cond_table[];

extern struct flag_type dam_type_flags[];

// Added by SinaC 2001
extern const char* short_etho_align[9];

// Added by SinaC 2001
struct material_type: OLCEditable {
  const char *name;     // material name
  const char *color;    // material color
  long imm;  // immunities
  long res;  // resistances
  long vuln; // vulnerabilities
  bool metallic;  // is this material metallic ?
};

extern struct material_type *material_table;
extern struct flag_type *material_flags;
extern struct flag_type etho_flags[];

// Added by SinaC 2001
//extern struct flag_type disease_flags[];  removed by SinaC 2003

// SinaC 2003
extern struct magic_school_type *magic_school_table;


extern int factions_count; // current number of factions <= MAX_FACTION
struct faction_data: OLCEditable {
  const char* name;
  int nb_races; // number of races getting this faction
  int *races; // races getting this faction
  int friendliness[MAX_FACTION]; // does this faction like other faction ?
};
extern FACTION_DATA faction_table[MAX_FACTION];

extern struct flag_type dam_class_flags[];

#endif                  // end of fix multi includes
