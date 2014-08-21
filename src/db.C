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
#include <stdarg.h> /* For log_stringf & bug, By Oxtal*/

#include "merc.h"
#include "db.h"
#include "recycle.h"
#include "lookup.h"
#include "interp.h"
#include "bit.h"
#include "affects.h"
#include "restriction.h"
#include "language.h"
#include "data.h"
#include "clan.h"
#include "arena.h"
#include "handler.h"
#include "comm.h"
#include "special.h"
#include "hints.h"
#include "qmaze.h"
#include "prereqs.h"
#include "olc_value.h"
#include "wiznet.h"
#include "scrhash.h"
#include "dbscript.h"
#include "brew.h"
#include "faction.h"
#include "dbdata.h"
#include "ban.h"
#include "db3.h"
#include "config.h"
#include "mem.h"
#include "olc.h"
#include "olc_act.h"
#include "wearoff_affect.h"
#include "update_affect.h"
#include "db2.h"
#include "save.h"
#include "tables.h"
#include "utils.h"


//#define VERBOSE

#if defined(__CYGWIN32__)
#define OLD_RAND
#endif


#if !defined(macintosh)
extern	int	_filbuf		args( (FILE *) );
#endif

#if !defined(OLD_RAND)
//long random();
void srandom(unsigned int);
extern "C" int getpid();
time_t time(time_t *tloc);
#endif

// SinaC 2003, large buffer used by scanner/parsers
char large_buffer[BIG_SIZE];


/*
 * Globals.
 */
HELP_DATA *		help_first;
HELP_DATA *		help_last;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

CHAR_DATA *		char_list;
const char *		help_greeting;
char			log_buf		[2*MAX_INPUT_LENGTH];
KILL_DATA		kill_table	[MAX_LEVEL];
OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;


// Added by SinaC 2001 for double_xp
bool                    double_xp = FALSE;


int                     gsn_throw;       /* Throw skill by Oxtal 1997 */
int                     gsn_fire;        /* Fire Skill added by Seytan */
int                     gsn_sharpen;     /* Sharpen Skill added by Sinac 1997 */
/* RoundHouse Kick added by Sinac 1997 */
int                     gsn_roundhousekick;
int                     gsn_enhanced_hand;
// added by SinaC 2000
int                     gsn_tail;

int			gsn_backstab;
int			gsn_dodge;
int			gsn_envenom;
int			gsn_hide;
int			gsn_peek;
int			gsn_pick_lock;
int			gsn_sneak;
int			gsn_steal;

int			gsn_disarm;
int			gsn_enhanced_damage;
int			gsn_kick;
int			gsn_parry;
int			gsn_rescue;
int			gsn_second_attack;
int			gsn_third_attack;
// added by SinaC 2000
int                  gsn_fourth_attack;
int                  gsn_fifth_attack;
int                  gsn_sixth_attack;
int                  gsn_seventh_attack;
int                  gsn_dual_wield;
int                  gsn_circle;
int                  gsn_fade;
int                  gsn_counter;
int                  gsn_whirlwind;
int                  gsn_critical_strike;
int                  gsn_deathgrip;
int                  gsn_familiar;
int                  gsn_song;
//int                  gsn_throwing;         removed by SinaC 2003
int                  gsn_meditate;
int                  gsn_blindfight;
int                  gsn_butcher;
int                  gsn_grip;
int                  gsn_lure;
int                  gsn_pillify;
int                  gsn_bladethirst;
//int                  gsn_barkskin;
int                  gsn_crush;
int                  gsn_warcry;
int                  gsn_spike;
int                  gsn_pugil;
int                  gsn_lash;
int                  gsn_forage;
int                  gsn_rear_kick;
int                  gsn_find_water;
int                  gsn_shield_cleave;
int                  gsn_endure;
int                  gsn_nerve;
int                  gsn_bandage;
int                  gsn_herb;
int                  gsn_cleave;
int                  gsn_stun;
int                  gsn_freeflee;
// Added by SinaC 2001
int                  gsn_stake;
int                  gsn_align_detect;
int                  gsn_magic_detect;
int                  gsn_poison_detect;
int                  gsn_exits_detect;
int                  gsn_wail;
int		     gsn_repulsion;
int		     gsn_wraithform;
int                  gsn_resize;
int                  gsn_forge;
int                  gsn_vorpalize;

int                  gsn_iron_hand;
int                  gsn_energy_fist;
int                  gsn_burning_fist;
int                  gsn_buddha_palm;
int                  gsn_concentration;
int                  gsn_fist_of_fury;

int                  gsn_speedup;
// for sprites
int                  gsn_invisible;
int                  gsn_feed;
int                  gsn_war_drums;
int                  gsn_shroud;
int                  gsn_repair;
int                  gsn_morph;
int                  gsn_flight;
int                  gsn_mistform;
int                  gsn_bite;

// special addition
int                  gsn_acid_breath;
// end


int			gsn_blindness;
int			gsn_charm_person;
int			gsn_curse;
int			gsn_invis;
int			gsn_mass_invis;
int			gsn_poison;
int			gsn_plague;
int			gsn_sleep;
int			gsn_sanctuary;
int			gsn_fly;

int                     gsn_haste;
/* new gsns */
int                     gsn_blend;

int                  gsn_hunt; /* Added by Seytan for HUNT skill 1997 */

int  		gsn_axe;
int  		gsn_dagger;
int  		gsn_flail;
int  		gsn_mace;
int  		gsn_polearm;
int		gsn_shield_block;
int  		gsn_spear;
int  		gsn_sword;
int  		gsn_whip;
int             gsn_staff; // Added by SinaC 2003
 
int  		gsn_bash;
int  		gsn_berserk;
int  		gsn_dirt;
int  		gsn_hand_to_hand;
int  		gsn_trip;
 
int  		gsn_fast_healing;
int  		gsn_haggle;
int  		gsn_lore;
int  		gsn_meditation;
 
int  		gsn_scrolls;
int  		gsn_staves;
int  		gsn_wands;
int  		gsn_recall;

int             gsn_protection_evil;
int             gsn_protection_good;
int             gsn_chill_touch;
int             gsn_fire_breath;


// Added by SinaC 2003
int             gsn_improved_exotic;
int             gsn_dual_parry;
//int             gsn_dual_wield2;
int             gsn_dual_disarm;
int             gsn_appraisal;
int             gsn_purify;
int             gsn_gouge;
int             gsn_evasion;
int             gsn_signal;
int             gsn_improved_steal;
int             gsn_comprehend_languages;
int             gsn_assassination;
int             gsn_armslore;
int             gsn_headbutt;
int             gsn_shield_bash;
int             gsn_justice;
int             gsn_angelic_light;
int             gsn_manashield;
int             gsn_corruption;
int             gsn_levitation;
int             gsn_nature_shield;
int             gsn_flight_bird;
int             gsn_speedofcheetah;
int             gsn_lightning_bolt;
int             gsn_frenzy;
int             gsn_bless;
int             gsn_slow;
int             gsn_armor;
int             gsn_shield;
int             gsn_mummify;
int             gsn_animate_skeleton;
//int             gsn_animate_dead;
int             gsn_zombify;
int             gsn_temporal_stasis;
int             gsn_globe_invuln;
int             gsn_entangle;
int             gsn_rooted_feet;
int             gsn_soothe;
int             gsn_stone_fist;
int             gsn_elemental_field;
int             gsn_weaken;
int             gsn_sacred_mists;
int             gsn_calm;
int             gsn_symbiote;
int             gsn_fireshield;
int             gsn_iceshield;
int             gsn_stoneshield;
int             gsn_airshield;
int             gsn_lightning_field;
int             gsn_improved_restoration;
int             gsn_enlarge;
int             gsn_reduce;
int             gsn_polymorph_self;
int             gsn_phylactery;
int             gsn_scribe;
int             gsn_riding;
int             gsn_mounted_combat;
int             gsn_charge;
int             gsn_bond;
int             gsn_invincible_mount;
int             gsn_strategic_retreat;
int             gsn_study;
int             gsn_flame_blade;
int             gsn_frost_blade;
int             gsn_drain_blade;
int             gsn_shocking_blade;
int             gsn_specialization;
int             gsn_disguise;
int             gsn_counterfeit;
int             gsn_favored_enemy;
//int             gsn_riding2;
int             gsn_climbing;
int             gsn_shapechange;
int             gsn_brew;
int             gsn_black_plague;

int             gsn_bowfire;

int             gsn_fleetfooted;
int             gsn_tumble;
int             gsn_block;
int             gsn_pure_body;
int             gsn_deflect;
int             gsn_diamond_body;
int             gsn_icy_fist;
int             gsn_acid_fist;
int             gsn_draining_fist;
int             gsn_hands_of_multitude;
int             gsn_flurry_of_fists;

int             gsn_fletcher;
int             gsn_disarming_shot;
int             gsn_bowyer;
int             gsn_strength_shot;
int             gsn_ignite_arrow;
int             gsn_long_range_shot;
int             gsn_sharpen_arrow;
int             gsn_point_blank_shot;
int             gsn_critical_shot;
int             gsn_third_wield;
int             gsn_fourth_wield;
int             gsn_protection_lycanthropy;
int             gsn_reckless_dweomer;

int             gsn_accelerate;

// Special Affect such as Lycanthropy
int             gsn_lycanthropy;








/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

AREA_DATA *		area_first;
AREA_DATA *		area_last;

char			str_empty	[1];

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int                     top_vnum_room;  /* OLC */
int                     top_vnum_mob;   /* OLC */
int                     top_vnum_obj;   /* OLC */
int 			mobile_count = 0;
int 			object_count = 0; // SinaC 2003
int			newmobs = 0;
int			newobjs = 0;



/*
 * Semi-locals.
 */
bool			fBootDb;
FILE *			fpArea;
char			strArea[MAX_INPUT_LENGTH];
// Own, Merc, Rom2.4, Smaug, Classic, Circle
#define STYLE_OWN     (0)
#define STYLE_MERC    (1)
#define STYLE_ROM     (2)
#define STYLE_SMAUG   (3)
#define STYLE_CLASSIC (4)
#define STYLE_CIRCLE  (5)
#define STYLE_DSA     (6)
#define STYLE_OLDMERC (7)
int                     Style;








/*
 * Local booting procedures.
*/


void    init_mm         args( ( void ) );


// General functions, called the right load functions depending on Style global variable
//void    load_list_of_areas   args( ( FILE *fpList ) );  OLD STYLE
void    load_style           args( ( FILE *fp ) );
void    load_area_general    args( ( FILE *fp ) );
void    load_mobiles_general args( ( FILE *fp ) );
void    load_objects_general args( ( FILE *fp ) );
void    load_rooms_general   args( ( FILE *fp ) );
void    load_resets_general  args( ( FILE *fp ) );
void    load_shops_general   args( ( FILE *fp ) );

// Own area file style
void    load_area       args( ( FILE *fp ) );   /* OLC */
void	load_helps	args( ( FILE *fp ) );
void 	load_mobiles	args( ( FILE *fp ) );
void 	load_objects	args( ( FILE *fp ) );
void	load_resets	args( ( FILE *fp ) );
void	load_rooms	args( ( FILE *fp ) );
void	load_shops	args( ( FILE *fp ) );
void 	load_socials	args( ( FILE *fp ) );
void	load_specials	args( ( FILE *fp ) );
void	load_bans	args( ( void ) );

// Added by SinaC 2001 to read Merc files
void	load_old_area	args( ( FILE *fp ) );
void	load_old_mob	args( ( FILE *fp ) );
void	load_old_obj	args( ( FILE *fp ) );
void	load_old_rooms	args( ( FILE *fp ) );
void    load_old_resets args( ( FILE *fp ) );

// Added by SinaC 2001 to read Rom2.4 files
void    load_mob_rom    args( ( FILE *fp ) );
void    load_obj_rom    args( ( FILE *fp ) );
void    load_room_rom   args( ( FILE *fp ) );

// Added by SinaC 2001 to read Smaug files
void    load_mob_smaug    args( ( FILE *fp ) );
void    load_obj_smaug    args( ( FILE *fp ) );
void    load_room_smaug   args( ( FILE *fp ) );
void    load_reset_smaug  args( ( FILE *fp ) );
void    load_version_smaug args( ( FILE *fp ) );
void    load_repair_smaug args( ( FILE *fp ) ); // SinaC 2003
int     smaug_version;

// Added by SinaC 2003 to read Circle files
void    load_mob_circle   args( ( FILE *fp ) );
void    load_obj_circle   args( ( FILE *fp ) );
void    load_room_circle  args( ( FILE *fp ) );
void    load_reset_circle args( ( FILE *fp ) );
void    load_shops_circle args( ( FILE *fp ) );

// Added by SinaC 2003 to read Dsa files
void    load_area_dsa  args( ( FILE *fp ) );
void    load_mob_dsa   args( ( FILE *fp ) );
void    load_obj_dsa   args( ( FILE *fp ) );
void    load_room_dsa  args( ( FILE *fp ) );

// Added by SinaC 2003 for an old area format
void    load_old_merc_area args( ( FILE *fp ) );

void	fix_exits	args( ( void ) );
// Added by SinaC 2003 to automatically set faction
void	fix_factions	args( ( void ) );

void fix_parts( MOB_INDEX_DATA *pMob ) {
  if ( pMob->race == race_lookup("unique") ) {
    log_stringf("fix_parts: mob [%d] is race unique", pMob->vnum );
    return;
  }
  //  head, arms, legs, heart, brains, guts
  //  head, arms, legs, heart, brains, guts, hands, feet, fingers, ear, eye, body);

  if ( IS_SET( pMob->parts, PART_ARMS ) && !IS_SET( pMob->parts, PART_HANDS )
       && !IS_SET( pMob->form, FORM_ANIMAL ) ) {
    log_stringf("fix_parts: mob [%d] have arms but no hands", pMob->vnum );
    //SET_BIT( pMob->parts, PART_HANDS );
  }
  if ( IS_SET( pMob->parts, PART_LEGS ) && !IS_SET( pMob->parts, PART_FEET )
       && !IS_SET( pMob->form, FORM_ANIMAL ) ) {
    log_stringf("fix_parts: mob [%d] have legs but no feet", pMob->vnum );
    //SET_BIT( pMob->parts, PART_FEET );
  }
  if ( IS_SET( pMob->parts, PART_HANDS ) && !IS_SET( pMob->parts, PART_FINGERS )
       && !IS_SET( pMob->form, FORM_ANIMAL ) ) {
    log_stringf("fix_parts: mob [%d] have hands but no fingers", pMob->vnum );
    //SET_BIT( pMob->parts, PART_FINGERS );
  }
  if ( IS_SET( pMob->parts, PART_HEAD ) && !IS_SET( pMob->parts, PART_EAR )
       && !IS_SET( pMob->form, FORM_ANIMAL ) ) {
    log_stringf("fix_parts: mob [%d] have head but no ears", pMob->vnum );
    //SET_BIT( pMob->parts, PART_EAR );
  }
  if ( IS_SET( pMob->parts, PART_HEAD ) && !IS_SET( pMob->parts, PART_EYE )
       && !IS_SET( pMob->form, FORM_ANIMAL ) ) {
    log_stringf("fix_parts: mob [%d] have head but no ears", pMob->vnum );
    //SET_BIT( pMob->parts, PART_EYE );
  }
  if ( ( IS_SET( pMob->parts, PART_ARMS )
	 || IS_SET( pMob->parts, PART_LEGS )
	 || IS_SET( pMob->parts, PART_HEAD) )
       && !IS_SET( pMob->parts, PART_BODY )
       && !IS_SET( pMob->form, FORM_ANIMAL ) ) {
    log_stringf("fix_parts: mob [%d] have arms/legs/head but no body", pMob->vnum );
    //SET_BIT( pMob->parts, PART_BODY );
  }
}

#define OLD_ITEM_FIREWEAPON (6)
#define OLD_ITEM_MISSILE (7)
#define OLD_ITEM_TRAP (14)
#define OLD_ITEM_NOTE (16)
#define OLD_ITEM_PEN (21)
#define OLD_ITEM_FOUNTAIN  (23)

void convert_shop( SHOP_DATA *pShop ) {
  for ( int i = 0; i < MAX_TRADE; i++ )
    if ( pShop->buy_type[i] == OLD_ITEM_FIREWEAPON ) {
      bug("Converting old item type #[%d] FIREWEAPON->WEAPON in shop, keeper [%d]", i, pShop->keeper );
      pShop->buy_type[i] = ITEM_WEAPON;
    }
    else if ( pShop->buy_type[i] == OLD_ITEM_MISSILE ) {
      bug("Converting old item type #[%d] MISSILE->WEAPON in shop, keeper [%d]", i, pShop->keeper );
      pShop->buy_type[i] = ITEM_WEAPON;
    }
    else if ( pShop->buy_type[i] == OLD_ITEM_TRAP ) {
      bug("Converting old item type #[%d] TRAP->TRASH in shop, keeper [%d]", i, pShop->keeper );
      pShop->buy_type[i] = ITEM_TRASH;
    }
    else if ( pShop->buy_type[i] == OLD_ITEM_NOTE ) {
      bug("Converting old item type #[%d] NOTE->TRASH in shop, keeper [%d]", i, pShop->keeper );
      pShop->buy_type[i] = ITEM_TRASH;
    }
    else if ( pShop->buy_type[i] == OLD_ITEM_PEN ) {
      bug("Converting old item type #[%d] PEN->TRASH in shop, keeper [%d]", i, pShop->keeper );
      pShop->buy_type[i] = ITEM_TRASH;
    }
    else if ( pShop->buy_type[i] == OLD_ITEM_FOUNTAIN ) {
      bug("Converting old item type #[%d] FOUNTAIN or CORPSE_NPC in shop, keeper [%d]", i, pShop->keeper );
      pShop->buy_type[i] = ITEM_TRASH;
    }
}

#define OLD_ROOM_PEACEFUL (E)
#define OLD_ROOM_NOMAGIC (H)
#define OLD_ROOM_TUNNEL (I)
void convert_room_flags( ROOM_INDEX_DATA *pRoom ) { // SinaC 2003
  int *flags = &pRoom->bstat(flags);
  bool found = FALSE;
  if ( IS_SET( *flags, OLD_ROOM_TUNNEL ) ) { 
    REMOVE_BIT( *flags, OLD_ROOM_TUNNEL ); 
    SET_BIT( *flags, ROOM_SOLITARY );
    bug("Converting old room flags [tunnel] for room %d", pRoom->vnum );
    found = TRUE;
  }
  if ( IS_SET( *flags, OLD_ROOM_NOMAGIC ) ) {
    REMOVE_BIT( *flags, OLD_ROOM_NOMAGIC );
    SET_BIT( *flags, ROOM_NOSPELL );
    bug("Converting old room flags [nomagic] for room %d", pRoom->vnum );
    found = TRUE;
  }
  if ( IS_SET( *flags, OLD_ROOM_PEACEFUL ) ) {
    REMOVE_BIT( *flags, OLD_ROOM_PEACEFUL );
    SET_BIT( *flags, ROOM_SAFE );
    bug("Converting old room flags [peaceful] for room %d", pRoom->vnum );
    found = TRUE;
  }
}

#define OLD_ACT_IS_NPC (D)
#define OLD_UNKNOWN (X)
void convert_act_flags( MOB_INDEX_DATA *pMob ) { // SinaC 2003
  long int *flags = &pMob->act;
  bool found = FALSE;
  if ( IS_SET( *flags, OLD_ACT_IS_NPC ) ) {
    REMOVE_BIT( *flags, OLD_ACT_IS_NPC );
    found = TRUE;
  }
  if ( IS_SET( *flags, OLD_UNKNOWN ) )
    bug("Found old act flags X for mob %d", pMob->vnum );
  if ( found )
    bug("Converting old act flags for mob %d", pMob->vnum );
}




// SinaC 2003: forbidden player name at creation
#define FORBIDDEN_LIST_STEP (32)
#define FORBIDDEN_END_OF_FILE '~'
void load_forbidden_names() {
  int current_max_size = 0;
  MAX_FORBIDDEN_NAME = 0;

  log_stringf("Reading forbidden names");

  FILE *fp;
  fclose( fpReserve );
  if ( ( fp = fopen( FORBIDDEN_LIST, "r" ) ) == NULL ) {
    fpReserve = fopen( NULL_FILE, "r" );
    bug("Can't open forbidden name list file: %s.", FORBIDDEN_LIST );
    return;
  }

  char name[MAX_STRING_LENGTH];
  do {
    fgets(name,MAX_STRING_LENGTH,fp);
    name[strlen(name)-1]='\0';

    if ( name[0] == FORBIDDEN_END_OF_FILE )
      break;

    if ( MAX_FORBIDDEN_NAME >= current_max_size ) { // realloc
      int new_size = current_max_size + FORBIDDEN_LIST_STEP;
      //      log_stringf("realloc  %d  -->  %d", current_max_size, new_size );
      const char **tmp;
      if ( ( tmp = (const char **)GC_MALLOC(new_size*sizeof(const char*))) == NULL ) {
	bug("Can't allocate memory for forbidden name list");
	break;
      }
      for ( int i = 0; i < current_max_size; i++ )
	tmp[i] = str_dup(forbidden_name[i]);
      forbidden_name = tmp;
      current_max_size = new_size;
    }

    forbidden_name[MAX_FORBIDDEN_NAME++] = str_dup( name ); // add in list
    //    log_stringf("  name: %s", name );

  } while ( name != NULL );

  log_stringf(" %d forbidden name found.", MAX_FORBIDDEN_NAME );

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}


// assign pgsn and spell_fun
//  SinaC 2000 for skills/spells in file
void assign_gsn_ability_fun() {
  int sn, i;
  log_string("Assigning pgsn & spell_fun");

  abilitynotsaved = FALSE;

  for ( sn = 0; sn < MAX_ABILITY_INIT; sn++ ) {
    // stop when no more skill to assign
    if ( ability_table_init[sn].name == NULL ) break;

    // search in ability_table
    for ( i = 0; i < MAX_ABILITY; i++ ) {
      // Added by SinaC 2001
      // stop when no more ability in ability_table
      if ( ability_table[i].name == NULL ) break;
      // stop if we have found the skill in ability_table
      if ( !str_cmp( ability_table[i].name, ability_table_init[sn].name ) ) break;
    }
    // skill not found in ability_table, error message and create it
    if ( ability_table[i].name == NULL ) {
      sprintf(log_buf,"Error in assign_gsn_ability: ability (%s) not found in skill file ==> creation of a new entry.",
	      ability_table_init[sn].name );
      log_string( log_buf );
      
      ability_table[i].name = str_dup(ability_table_init[sn].name);
      ability_table[i].target = 0;
      ability_table[i].minimum_position = 0;
      ability_table[i].slot = 0;
      //ability_table[i].min_mana = 0;
      ability_table[i].min_cost = 0;
      ability_table[i].beats = 0;
      ability_table[i].noun_damage = str_dup("");
      ability_table[i].msg_off = str_dup("");
      ability_table[i].msg_obj = str_dup("");
      // Added by SinaC 2001
      ability_table[i].msg_room = str_dup("");
      // Added by SinaC 2001 for mob classes
      ability_table[i].mob_use = 0;
      // Added by SinaC 2001 for mental user
      ability_table[i].type = 0;
      // SinaC 2003
      ability_table[i].scriptAbility = FALSE;

      ability_table[i].prereqs = NULL;

      ability_table[i].action_fun = NULL;
      ability_table[i].nb_casting_level = 0;
      ability_table[i].wearoff_fun = wearoff_null;
      ability_table[i].update_fun = update_null;

      abilitynotsaved = TRUE;
    }
    // assign pgsn
    if ( ability_table_init[sn].pgsn != NULL ) {
      ability_table[i].pgsn = ability_table_init[sn].pgsn;
      *ability_table[i].pgsn = i;
    }
    /* Modified by SinaC 2001
    // assign spell_fun
    if ( ability_table_init[sn].spell_fun != spell_null )
      ability_table[i].spell_fun = ability_table_init[sn].spell_fun;
    else
      ability_table[i].spell_fun = spell_null;
    */
#ifdef VERBOSE2
    log_stringf("%d) [%s]  (%d)", 
		i, ability_table[i].name, ability_table[i].type );
#endif
    ability_table[i].action_fun = ability_table_init[sn].action_fun;

    // Added by SinaC 2000 for spell/skill level & prereq
    // assign number of level
    ability_table[i].nb_casting_level = ability_table_init[sn].nb_casting_level;
    // NULLify the prereqs
    ability_table[i].prereqs = NULL;

    // Added by SinaC 2003 for clean affect wearoff/update
    ability_table[i].wearoff_fun = ability_table_init[sn].wearoff_fun;
    ability_table[i].update_fun = ability_table_init[sn].update_fun;

    // Modified by SinaC 2001
    // Added by SinaC 2001 for do_fun for USE command
    //ability_table[i].do_fun = ability_table_init[sn].do_fun;
  }
}


void check_needed_mobs() {
  extern ValueList *NEEDED_MOB;
  log_stringf("Checking needed mobiles...");
  bool saveBoot = fBootDb;
  fBootDb = FALSE;
  if ( NEEDED_MOB != NULL )
    for ( int i = 0; i < NEEDED_MOB->size; i++ ) {
      int vnum = NEEDED_MOB->elems[i].asInt();
      if ( get_mob_index( vnum ) == NULL )
	bug("  Mobile [%d] is missing for a correct mud running", vnum );
    }
  fBootDb = saveBoot;
  log_stringf("Done.");
}

void check_needed_objs() {
  extern ValueList *NEEDED_OBJ;
  log_stringf("Checking needed Objects...");
  bool saveBoot = fBootDb;
  fBootDb = FALSE;
  if ( NEEDED_OBJ != NULL )
    for ( int i = 0; i < NEEDED_OBJ->size; i++ ) {
      int vnum = NEEDED_OBJ->elems[i].asInt();
      if ( get_obj_index( vnum ) == NULL )
	bug("  Object [%d] is missing for a correct mud running", vnum );
    }
  fBootDb = saveBoot;
  log_stringf("Done.");
}

/*
 * Big mama top level function.
 */
// if fState is TRUE, no need to do area_update
void mini_boot_db() { // Used by parse_command_line  if generateHTML is specified
  fBootDb = TRUE;
  init_mm( );
  if ( START_WITH_SAVED_TIME )
    load_time();
  else
    init_time_weather();
  init_flag_stat_table_init();
  new_load_abilities();
  assign_gsn_ability_fun();
  assign_language_sn();
  new_load_groups();
  load_schools();
  new_load_pc_classes();
  update_flag_stat_table_init( "classes", classes_flags );
  new_load_prerequisites();
  new_load_spheres();
  check_groups();
  new_load_races();
  new_load_pcraces();
  new_load_gods();
  update_group_abilities_with_spheres();
  FILE *fpList;
  log_string("Reading Helps...");
  if ( ( fpList = fopen( HELP_LIST, "r" ) ) == NULL ) {
    perror( HELP_LIST );
    exit( 1 );
  }
  new_load_list_of_areas( fpList, HELP_DIR );
  fclose( fpList );
  log_stringf("%d help found", top_help );
}
void boot_db( const bool fState ) {


  log_string("Booting...");
  fBootDb = TRUE;
  
  /*
   * Init random number generator.
   */
  init_mm( );
  
  /*
   * Set time and weather.
   */
  //init_time_weather();
  if ( START_WITH_SAVED_TIME )
    load_time();
  else
    init_time_weather();

  // lol
  init_flag_stat_table_init();

  boot_scripts(); // Oxtal

  dump_GC_info();

  //  init_flag_stat_table();
  //  init_attr_table();
  //  init_restr_table();
  //  init_help_table();
  //  init_room_attr_table();

  new_load_commands(); // Added by SinaC 2001

  new_load_abilities(); // Added by SinaC 2000/2001
  assign_gsn_ability_fun();
  assign_language_sn();

  new_load_groups(); // Groups need abilities

  load_schools(); // Magic school, SinaC 2003

  //load_classtable(); // Classes need groups
  //load_pc_classes();
  new_load_pc_classes();
  update_flag_stat_table_init( "classes", classes_flags );

  new_load_prerequisites(); // prerequisites need abilities and classes

  new_load_spheres(); // Added by SinaC 2003
  check_groups(); // Spheres need classes and groups

  new_load_races();
  new_load_pcraces();
  update_flag_stat_table_init( "race", races_flags );

  load_superraces(); // SinaC 2003

  new_load_liquid();
  new_load_material();
  update_flag_stat_table_init( "material", material_flags );

  new_load_gods(); // Added by SinaC 2001
  update_group_abilities_with_spheres(); // SinaC 2003
  new_load_factions(); // Added by SinaC 2003

  // Added by SinaC 2003
  new_load_brew_formula();
  update_flag_stat_table_init( "component", brew_component_flags );

  // Added by SinaC 2000
  // Init the flag_stat_table used in bit.C
  //  we need to initialize it here because classes_flags and races_flags 
  // are not known at starting but only after load_classtable() & load_pcrace()

  init_flag_stat_table();
  init_attr_table();
  init_restr_table();
  init_help_table();
  // Added by SinaC 2003
  init_room_attr_table();

  dump_GC_info();

  /*
   * Read in all the area files.
   */
  {
    FILE *fpList;
    // Load additional files: help, social, ...  SinaC 2003
    log_string("Reading Helps...");
    if ( ( fpList = fopen( HELP_LIST, "r" ) ) == NULL ) {
      perror( HELP_LIST );
      exit( 1 );
    }
    new_load_list_of_areas( fpList, HELP_DIR );
    fclose( fpList );
    log_stringf("%d help found", top_help );

    log_string("Reading Socials..."); // SinaC 2003
    char fileName[MAX_STRING_LENGTH];
    strcpy( strArea, SOCIAL_FILE );
    sprintf( fileName, "%s", SOCIAL_FILE );
    if ( ( fpArea = fopen( fileName, "r" ) ) == NULL ) {
      perror( strArea );
      exit( 1 );
    }
    load_one_area( fpArea );
    log_stringf("%d socials found", social_count );

    // Load area.lst
    log_string("Reading Areas...");
    if ( ( fpList = fopen( AREA_LIST, "r" ) ) == NULL ) {
      perror( AREA_LIST );
      exit( 1 );
    }
    new_load_list_of_areas( fpList, AREA_DIR );
    fclose( fpList );

    log_stringf("%d areas, %d rooms, %d mobiles, %d objects and %d shops found",
		top_area, top_room, top_mob_index, top_obj_index, top_shop );
  }
  
  dump_GC_info();

  /*
   * Fix up exits.
   * Declare db booting over.
   * Reset all areas once.
   * Load up the songs, notes and ban files.
   */
  {
    fBootDb = TRUE;
    fix_exits();
    
    // Added by SinaC 2003 for factions
    fix_factions();

    fBootDb = FALSE; // use get_obj_index and we don't want to exit the mud when find a unknown obj
    convert_objects();           /* ROM OLC */
    fBootDb = TRUE;

    new_load_clans(); // moved by SinaC 2003
    new_load_bans();
    //load_songs();  removed by SinaC 2003
    load_boards();
    save_boards(); // Removes old notes
    // Load disabled commands and hints Sinac/Oxtal 1997, SinaC 2000/2001
    new_load_disabled();
    load_hints();
    load_hall_of_fame();
    // Added by SinaC 2001 for unique items
    new_load_unique_items();
    load_hometown(); // SinaC 2003, hometown are in file
    load_forbidden_names(); // SinaC 2003, forbidden player name

    check_needed_scripts(); // SinaC 2003, check script/obj/mob needed by abilities
    check_needed_objs();
    check_needed_mobs();

    assign_default_struct(); // SinaC 2003, need hometown info

    fBootDb	= FALSE;

    // if fState is TRUE, world state has been saved, so there is no need to
    //  update world: it will be create by load_world_state
    if ( !fState ) {
      log_stringf("Creating World...");
      area_update();
      log_stringf("World Created.");
    }

    check_struct_integrity(); // SinaC 2003, check integrity of structures
  }

  dump_GC_info();

  GC_gcollect();
  GC_enable_incremental();
  return;
}

void load_one_area( FILE *fp ) { // SinaC 2003
  Style = STYLE_OWN;
  smaug_version = 0;
  
  for ( ; ; ) {
    char *word;
    
    if ( fread_letter( fpArea ) != '#' ) {
      bug( "Boot_db: # not found.");
      exit( 1 );
    }
    
    word = fread_word( fpArea );
    
    if ( word[0] == '$'               )                 break;
    // Modified by SinaC 2001
    else if ( !str_cmp( word, "SOCIALS"  ) ) load_socials         (fpArea);
    else if ( !str_cmp( word, "HELPS"    ) ) load_helps           (fpArea);
    
    else if ( !str_cmp( word, "STYLE"    ) ) load_style           (fpArea);
    else if ( !str_cmp( word, "AREA"     ) ) load_area_general    (fpArea);
    else if ( !str_cmp( word, "AREADATA" ) ) load_area_general    (fpArea);
    else if ( !str_cmp( word, "MOBILES"  ) ) load_mobiles_general (fpArea);
    else if ( !str_cmp( word, "OBJECTS"  ) ) load_objects_general (fpArea);
    else if ( !str_cmp( word, "RESETS"   ) ) load_resets_general  (fpArea);
    else if ( !str_cmp( word, "ROOMS"    ) ) load_rooms_general   (fpArea);
    else if ( !str_cmp( word, "SHOPS"    ) ) load_shops_general   (fpArea);
    else if ( !str_cmp( word, "SPECIALS" ) ) load_specials        (fpArea);
    
    // Special for Smaug version
    else if ( !str_cmp( word, "VERSION" ) )  load_version_smaug   (fpArea);
    
    // Things I don't need
    else if ( !str_cmp( word, "MOBPROGS" ) ) skip_till_cardinal0 (fpArea);
    else if ( !str_cmp( word, "REPAIRS"  ) ) load_repair_smaug( fpArea);//skip_till_0         (fpArea);
    else if ( !str_cmp( word, "TRIGGERS" ) ) skip_till_          (fpArea,'S');
    else {
      bug( "Boot_db: bad section name.");
      exit( 1 );
    }
  }
}

// Old STYLE, check db3.C
//void load_list_of_areas( FILE *fpList ) {
//  for ( ; ; ) {
//    strcpy( strArea, fread_word( fpList ) );
//    if ( strArea[0] == '$' )
//      break;
//
//    if ( strArea[0] == '-' ) {
//      fpArea = stdin;
//    }
//    else {
//      log_stringf( "Reading %s", strArea );
//      if ( ( fpArea = fopen( strArea, "r" ) ) == NULL ) {
//	perror( strArea );
//	exit( 1 );
//      }
//    }
//
//    load_one_area( fpArea );
//
//    if ( fpArea != stdin )
//      fclose( fpArea );
//    fpArea = NULL;
//  }
//}

void load_style( FILE *fp ) {
  char *word;

  word = fread_word(fp);
  
  if (!str_cmp( word, "OWN" ) )
    Style = STYLE_OWN;
  else if (!str_cmp( word, "CLASSIC" ) ) {
    log_stringf(" Classic style");
    Style = STYLE_CLASSIC;
  }
  else if (!str_cmp( word, "MERC" ) ) {
    log_stringf(" Merc style");
    Style = STYLE_MERC;
  }
  else if (!str_cmp( word, "ROM" ) ) {
    log_stringf(" Rom style");
    Style = STYLE_ROM;
  }
  else if (!str_cmp( word, "SMAUG" ) ) {
    log_stringf(" Smaug style");
    Style = STYLE_SMAUG;
  }
  else if (!str_cmp( word, "Circle" ) ) {
    log_stringf(" Circle style");
    Style = STYLE_CIRCLE;
  }
  else if (!str_cmp( word, "DSA" ) ) {
    log_stringf(" Dsa style");
    Style = STYLE_DSA;
  }
  else if (!str_cmp( word, "OLDMERC" ) ) {
    log_stringf(" Old merc style");
    Style = STYLE_OLDMERC;
  }
  else {
    bug("Invalid Area Style: %s", word );
    exit(-1);
  }
  return;
}

void load_area_general( FILE *fp ) {
  switch( Style ) {
  case STYLE_OWN:
    load_area(fp);
    break;
  case STYLE_MERC:
    load_old_area(fp);
    break;
  case STYLE_ROM:
    load_old_area(fp);
    break;
  case STYLE_SMAUG:
    load_old_area(fp);
    break;
  case STYLE_CLASSIC:
    load_area(fp);
    break;
  case STYLE_CIRCLE:
    load_area(fp);
    break;
  case STYLE_DSA:
    load_area_dsa(fp);
    break;
  case STYLE_OLDMERC:
    load_old_merc_area(fp);
    break;
  default:
    bug("Invalid Area Style: (%d)", Style );
    break;
  }
  return;
}

void load_mobiles_general( FILE *fp ) {
  switch( Style ) {
  case STYLE_OWN:
    load_mobiles(fp);
    break;
  case STYLE_MERC:
    load_old_mob(fp);
    break;
  case STYLE_ROM:
    load_mob_rom(fp);
    break;
  case STYLE_SMAUG:
    load_mob_smaug(fp);
    break;
  case STYLE_CLASSIC:
    load_mob_rom(fp);
    break;
  case STYLE_CIRCLE:
    load_mob_circle(fp);
    break;
  case STYLE_DSA:
    load_mob_dsa(fp);
    break;
  case STYLE_OLDMERC:
    load_old_mob(fp);
    break;
  default:
    bug("Invalid Area Style: (%d)", Style );
    break;
  }
  return;
}

void load_objects_general( FILE *fp ) {
  switch( Style ) {
  case STYLE_OWN:
    load_objects(fp);
    break;
  case STYLE_MERC:
    load_old_obj(fp);
    break;
  case STYLE_ROM:
    load_obj_rom(fp);
    break;
  case STYLE_SMAUG:
    load_obj_smaug(fp);
    break;
  case STYLE_CLASSIC:
    load_obj_rom(fp);
    break;
  case STYLE_CIRCLE:
    load_obj_circle(fp);
    break;
  case STYLE_DSA:
    load_obj_dsa(fp);
    break;
  case STYLE_OLDMERC:
    load_old_obj(fp);
    break;
  default:
    bug("Invalid Area Style: (%d)", Style );
    break;
  }
  return;
}

void load_rooms_general( FILE *fp ) {
  switch( Style ) {
  case STYLE_OWN:
    load_rooms(fp);
    break;
  case STYLE_MERC:
    load_old_rooms(fp);
    break;
  case STYLE_ROM:
    load_room_rom(fp);
    break;
  case STYLE_SMAUG:
    load_room_smaug(fp);
    break;
  case STYLE_CLASSIC:
    load_room_rom(fp);
    break;
  case STYLE_CIRCLE:
    load_room_circle(fp);
    break;
  case STYLE_DSA:
    load_room_dsa(fp);
    break;
  case STYLE_OLDMERC:
    load_old_rooms(fp);
    break;
  default:
    bug("Invalid Area Style: (%d)", Style );
    break;
  }
  return;
}

void load_resets_general( FILE *fp ) {
  switch( Style ) {
  case STYLE_OWN:
    load_resets(fp);
    break;
  case STYLE_MERC:
    load_old_resets(fp);
    break;
  case STYLE_ROM:
    load_resets(fp);
    break;
  case STYLE_SMAUG:
    load_reset_smaug(fp);
    break;
  case STYLE_CLASSIC:
    load_resets(fp);
    break;
  case STYLE_CIRCLE:
    load_reset_circle(fp);
    break;
  case STYLE_DSA:
    bug("RESETS section inexistant in DSA format");
    break;
  case STYLE_OLDMERC:
    load_old_resets(fp);
    break;
  default:
    bug("Invalid Area Style: (%d)", Style );
    break;
  }
  return;
}

void load_shops_general( FILE *fp ) {
  switch( Style ) {
  case STYLE_OWN:
    load_shops(fp);
    break;
  case STYLE_MERC:
    load_shops(fp);
    break;
  case STYLE_ROM:
    load_shops(fp);
    break;
  case STYLE_SMAUG:
    load_shops(fp);
    break;
  case STYLE_CLASSIC:
    load_shops(fp);
    break;
  case STYLE_CIRCLE:
    load_shops_circle(fp);
    break;
  case STYLE_DSA:
    load_shops(fp);
    break;
  case STYLE_OLDMERC:
    load_shops(fp);
    break;
  default:
    bug("Invalid Area Style: (%d)", Style );
    break;
  }
  return;
}



// Added by SinaC for  Rom 2.4 style
/*
 * Snarf a mob section.  rom 2.4 style
 */
void load_mob_rom( FILE *fp ) {
  MOB_INDEX_DATA *pMobIndex;
 
  if ( !area_last ) {   /* OLC */
    bug( "Load_mob_rom: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
 
    letter                          = fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_mob_rom: # not found.");
      exit( 1 );
    }
 
    vnum                            = fread_number( fp );
    if ( vnum == 0 )
      break;
 
    fBootDb = FALSE;
    if ( get_mob_index( vnum ) != NULL ) {
      bug( "Load_mob_rom: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;
 
    //pMobIndex                       = (MOB_INDEX_DATA*) alloc_perm( sizeof(*pMobIndex) );
    pMobIndex = new_mob_index();

    pMobIndex->vnum                 = vnum;

    pMobIndex->area                 = area_last;               /* OLC */
    pMobIndex->new_format		= TRUE;
    newmobs++;
    pMobIndex->player_name          = fread_string( fp );
    pMobIndex->short_descr          = fread_string( fp );
    pMobIndex->long_descr           = fread_string_upper( fp );
    pMobIndex->description          = fread_string_upper( fp );

    const char *word = fread_string( fp );
    pMobIndex->race	 	    = race_lookup(word,TRUE);
    // Added by SinaC 2001
    if ( pMobIndex->race < 0 ) {
      bug("invalid race for %s (vnum: %d) [%s]", pMobIndex->player_name, pMobIndex->vnum, word );
      pMobIndex->race = 0;
    } 
    else // SinaC 2003
      if ( str_cmp( race_table[pMobIndex->race].name, word ) )
	bug("Problem race, read [%s] and table [%s], for %s (vnum: %d)",
	    race_table[pMobIndex->race].name, word,
	    pMobIndex->player_name, pMobIndex->vnum );

    // Added by SinaC 2001
    pMobIndex->classes              = 0;

    // Modified by SinaC 2001
    /*
     * pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC
     * | race_table[pMobIndex->race].act;
     * pMobIndex->affected_by          = fread_flag( fp )
     * | race_table[pMobIndex->race].aff;
    */
    pMobIndex->act                  = fread_flag( fp ) | ACT_IS_NPC;
    pMobIndex->affected_by          = fread_flag( fp );
    // Added by SinaC 2001
    pMobIndex->affected2_by          = 0;

    pMobIndex->pShop                = NULL;
    // Added by SinaC 2001
    pMobIndex->align.etho           = 0;
    pMobIndex->align.alignment       = fread_number( fp );

    pMobIndex->group                = fread_number( fp );

    pMobIndex->level                = fread_number( fp );

    // Added by SinaC 2001
    if ( pMobIndex->level < pMobIndex->area->low_range )
      pMobIndex->area->low_range = pMobIndex->level;
    if ( pMobIndex->level > pMobIndex->area->high_range )
      pMobIndex->area->high_range = pMobIndex->level;

    pMobIndex->hitroll              = fread_number( fp );  

    /* read hit dice */
    pMobIndex->hit[DICE_NUMBER]     = fread_number( fp );  
    /* 'd'          */                fread_letter( fp ); 
    pMobIndex->hit[DICE_TYPE]   	= fread_number( fp );
    /* '+'          */                fread_letter( fp );   
    pMobIndex->hit[DICE_BONUS]      = fread_number( fp ); 

    /* read mana dice */
    pMobIndex->mana[DICE_NUMBER]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->mana[DICE_TYPE]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->mana[DICE_BONUS]	= fread_number( fp );

    // Added by SinaC 2001 for mental user
    // read psp dice
    pMobIndex->psp[DICE_NUMBER]	= 0;
    pMobIndex->psp[DICE_TYPE]	= 0;
    pMobIndex->psp[DICE_BONUS]	= 0;
    


    /* read damage dice */
    pMobIndex->damage[DICE_NUMBER]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->damage[DICE_TYPE]	= fread_number( fp );
    fread_letter( fp );
    pMobIndex->damage[DICE_BONUS]	= fread_number( fp );
    pMobIndex->dam_type		= attack_lookup(fread_word(fp));

    /* read armor class */
    pMobIndex->ac[AC_PIERCE]	= fread_number( fp ) * 10;
    pMobIndex->ac[AC_BASH]		= fread_number( fp ) * 10;
    pMobIndex->ac[AC_SLASH]		= fread_number( fp ) * 10;
    pMobIndex->ac[AC_EXOTIC]	= fread_number( fp ) * 10;

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

    pMobIndex->form			= fread_flag( fp )
      | race_table[pMobIndex->race].form;
    pMobIndex->parts		= fread_flag( fp )
      | race_table[pMobIndex->race].parts;
    /* size */
    pMobIndex->size			= size_lookup(fread_word(fp));
    if ( pMobIndex->size < 0 ) {
      bug("Invalid size for mob %d, assuming medium.", pMobIndex->vnum );
      pMobIndex->size = SIZE_MEDIUM;
    }
    pMobIndex->material		= str_dup(fread_word( fp ));
 
    for ( ; ; ) {
      letter = fread_letter( fp );

      if (letter == 'F') {
	char *word;
	long vector;

	word                    = fread_word(fp);
	vector			= fread_flag(fp);

	if (!str_prefix(word,"act"))
	  REMOVE_BIT(pMobIndex->act,vector);
	else if (!str_prefix(word,"aff"))
	  REMOVE_BIT(pMobIndex->affected_by,vector);
	// Added by SinaC 2001
	else if (!str_prefix(word,"aff2"))
	  REMOVE_BIT(pMobIndex->affected2_by,vector);

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
      // skipping these version of mobprograms
      else if (letter == 'M') {
	fread_to_eol(fp);
      } 
      else {
	ungetc(letter,fp);
	break;
      }
    }

    convert_act_flags( pMobIndex );
    fix_parts( pMobIndex );

    iHash                   = vnum % MAX_KEY_HASH;
    pMobIndex->next         = mob_index_hash[iHash];
    mob_index_hash[iHash]   = pMobIndex;
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
    assign_area_vnum( vnum );                                  /* OLC */
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
  }
 
  return;
}

/*
 * Snarf an obj section. rom 2.4 style
 */
void load_obj_rom( FILE *fp )
{
  OBJ_INDEX_DATA *pObjIndex;
    
  const char *word; /* Oxtal -- Makes the assumption that fread_word buffer is static */ 
 
  if ( !area_last ) {   /* OLC */
    bug( "Load_objects: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;
 
    letter                          = fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_objects: # not found.");
      exit( 1 );
    }
 
    vnum                            = fread_number( fp );
#ifdef ADBG
    log_stringf(" Loading Object #%d",vnum);
#endif
    if ( vnum == 0 )
      break;
 
    fBootDb = FALSE;
    if ( get_obj_index( vnum ) != NULL ) {
      bug( "Load_objects: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;
 
    //pObjIndex                       = (OBJ_INDEX_DATA*) alloc_perm( sizeof(*pObjIndex) );
    pObjIndex = new_obj_index();
    pObjIndex->vnum                 = vnum;
    pObjIndex->area                 = area_last;            /* OLC */
    pObjIndex->new_format           = TRUE;
    pObjIndex->reset_num            = 0;
    newobjs++;
    pObjIndex->name                 = fread_string( fp );
    pObjIndex->short_descr          = fread_string( fp );
    pObjIndex->description          = fread_string( fp );
    word = fread_string(fp);
    int mat = material_lookup(word);
    if ( mat < 0 )
      bug("Invalid material '%s' for object (vnum %d), assuming <Not Defined>", word, vnum );
    pObjIndex->material		    = mat<0 ? 0 : mat;

    // Added by SinaC 2003
    pObjIndex->size = SIZE_NOSIZE;

    pObjIndex->item_type            = item_lookup(fread_word( fp ));
    if ( pObjIndex->item_type < 0 ) {
      bug("Invalid item type for object %d, assuming trash.", pObjIndex->vnum );
      pObjIndex->item_type = ITEM_TRASH;
    }
    pObjIndex->extra_flags          = fread_flag( fp );

    pObjIndex->wear_flags           = fread_flag( fp );
    char buffer[512];
    int sn;
    switch(pObjIndex->item_type) {
    /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
    case ITEM_GRENADE:
      pObjIndex->value[0]		= fread_number(fp);
      pObjIndex->value[1]		= fread_number(fp);
      pObjIndex->value[2]		= fread_number(fp);
      pObjIndex->value[3]		= fread_number(fp);
      pObjIndex->value[4]		= fread_number(fp);
      break;
    */
    /* Removed by SinaC 2003
      // Added by SinaC 2000 for throwing item
    case ITEM_THROWING:
      pObjIndex->value[0]		= fread_number(fp);
      pObjIndex->value[1]		= fread_number(fp);
      pObjIndex->value[2]		= attack_lookup(fread_word(fp));
      pObjIndex->value[3]		= fread_number(fp);
      //pObjIndex->value[4]		= skill_lookup(fread_word(fp));
      strcpy(buffer, fread_word(fp));
      if ( ( sn = skill_lookup(buffer) ) <= 0 
	   && buffer[0]!='\0') {
	bug("Invalid value4 '%s' (sn: %d) for object %s (vnum %d)",
	    buffer, sn, pObjIndex->short_descr, vnum );
	sn = 0;
      }
      if ( sn > 0 && ability_table[sn].type != TYPE_SPELL
	   && ability_table[sn].type != TYPE_POWER )
	log_stringf("Value4 '%s' (sn: %d) for object %s (vnum %d) is not a spell/power",
		    buffer, sn, pObjIndex->short_descr, vnum );
      pObjIndex->value[4]               = sn;
      break;
    */
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
      if ( ( sn = ability_lookup(buffer) ) <= 0
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
      /*
      pObjIndex->value[1]		= ability_lookup(fread_word(fp));
      pObjIndex->value[2]		= ability_lookup(fread_word(fp));
      pObjIndex->value[3]		= ability_lookup(fread_word(fp));
      pObjIndex->value[4]		= ability_lookup(fread_word(fp));
      */
      strcpy(buffer, fread_word(fp));
      if ( ( sn = ability_lookup(buffer) ) <= 0 
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
      if ( ( sn = ability_lookup(buffer) ) <= 0 
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
      if ( ( sn = ability_lookup(buffer) ) <= 0 
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
      if ( ( sn = ability_lookup(buffer) ) <= 0 
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

    /* condition */
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
      char letter;
 
      letter = fread_letter( fp );
 
      if ( letter == 'A' ) {
	AFFECT_DATA *paf;
	//JyP: following line was duplicated !
	//paf			= (AFFECT_DATA*) alloc_perm( sizeof(*paf) );
	paf = new_affect();
	createaff(*paf,-1,20,-1,0,AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT); // FIXME: this doesn't follow new affect philosophy
	addaff2(*paf,AFTO_CHAR,locoldtonew(fread_number( fp )),AFOP_ADD,fread_number( fp ));
	
//	paf->where              = AFTO_CHAR;
//	paf->op                 = AFOP_ADD;
//	paf->type		= -1;
//	paf->level		= 20; /* RT temp fix */
//	paf->duration		= -1;
//	paf->location           = locoldtonew(fread_number( fp ));
//	paf->modifier		= fread_number( fp );
	paf->next               = pObjIndex->affected;
	pObjIndex->affected     = paf;
      }
      else if (letter == 'F') {
	long loc2,bitvector;
	AFFECT_DATA *paf;

	//paf                     = (AFFECT_DATA*) alloc_perm( sizeof(*paf) );
	paf = new_affect();

	//paf->where = AFTO_CHAR; 

	letter 			= fread_letter(fp);
	switch (letter)	{
	case 'A':
	  loc2       = ATTR_affected_by;
	  break;
	case 'I':
	  loc2       = ATTR_imm_flags;
	  break;
	case 'R':
	  loc2       = ATTR_res_flags;
	  break;
	case 'V':
	  loc2       = ATTR_vuln_flags;
	  break;
	default:
	  bug( "Load_objects: Bad location on affect flag set.");
	  exit( 1 );
	}
	createaff(*paf,-1,pObjIndex->level,-1,0,AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT); // FIXME: this doesn't follow new affect philosophy
	int loc = locoldtonew(fread_number( fp ));
	int mod = fread_number( fp );
	bitvector               = fread_flag(fp);
	if ( mod && loc != ATTR_NA ) {
	  if ( bitvector )
	    bug("Bitvector ignored in old affect");
	  addaff2(*paf,AFTO_CHAR,loc,AFOP_ADD,mod);
	}
	else {
	  addaff2(*paf,AFTO_CHAR,loc2,AFOP_OR,bitvector);
	}
	if ( mod && loc != ATTR_NA ) {
	  paf->next               = pObjIndex->affected;
	  pObjIndex->affected     = paf;
	}
	else
	  bug("Affect has no effect");

	//paf->type               = -1;
	//paf->level              = pObjIndex->level;
	//paf->duration           = -1;
	//paf->location           = locoldtonew(fread_number(fp));
	//paf->modifier           = fread_number(fp);
	//bitvector               = fread_flag(fp);

	//if (paf->modifier && (paf->location != ATTR_NA)) {
	//  if (bitvector) bug("Bitvector ignored in old affect");
	//  paf->op = AFOP_ADD;
	//}
	//else {
	//  paf->op = AFOP_OR;
	//  paf->modifier = bitvector;
	//  paf->location = loc2;
	//}
	//if (paf->modifier && (paf->location != ATTR_NA)) {
	//  paf->next               = pObjIndex->affected;
	//  pObjIndex->affected     = paf;
	//} 
	//else
	//  bug("Affect has no effect");
      }
      else if ( letter == 'E' ) {
	EXTRA_DESCR_DATA *ed;
 
	//ed                      = (EXTRA_DESCR_DATA*) alloc_perm( sizeof(*ed) );
	ed = new_extra_descr();
	ed->keyword             = fread_string( fp );
	ed->description         = fread_string( fp );
	ed->next                = pObjIndex->extra_descr;
	pObjIndex->extra_descr  = ed;
      }
      // FIXME: ????  what's this ?  SinaC 2003
      else if ( letter == 'L' ) {
	fread_number(fp);
      }
      else {
	ungetc( letter, fp );
	break;
      }
    }
 
    iHash                   = vnum % MAX_KEY_HASH;
    pObjIndex->next         = obj_index_hash[iHash];
    obj_index_hash[iHash]   = pObjIndex;
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
    assign_area_vnum( vnum );                                   /* OLC */
  }
 
  return;
}


/*
 * Snarf a room section.
 */
void load_room_rom( FILE *fp )
{
  ROOM_INDEX_DATA *pRoomIndex;

  if ( area_last == NULL ) {
    bug( "Load_room_rom: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int door;
    int iHash;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_room_rom: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_room_index( vnum ) != NULL ) {
      bug( "Load_room_rom: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //pRoomIndex			= (ROOM_INDEX_DATA *) alloc_perm( sizeof(*pRoomIndex) );
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
    // Modified by SinaC 2001
    pRoomIndex->bstat(flags)		= fread_flag( fp );
    convert_room_flags( pRoomIndex ); // SinaC 2003

    pRoomIndex->bstat(maxsize)		= SIZE_NOSIZE;

    /* horrible hack */
    /* we don't need that now, flags are saved with OLC, SinaC 2000
       if ( 3000 <= vnum && vnum < 3400)
       SET_BIT(pRoomIndex->room_flags,ROOM_LAW);
    */

    // Modified by SinaC 2001
    pRoomIndex->bstat(sector)		= fread_number( fp );
    // Added by SinaC 2000
    if ( pRoomIndex->bstat(sector) < 0 || pRoomIndex->bstat(sector) > SECT_MAX-1 )
      bug("Invalid sector type (%d) in room %d",
	  pRoomIndex->bstat(sector), pRoomIndex->vnum );

    // Modified by SinaC 2001
    pRoomIndex->bstat(light)		= 0;
    for ( door = 0; door < MAX_DIR; door++ )
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
      case 'H' : /* healing room */
	// Modified by SinaC 2001
	pRoomIndex->bstat(healrate) = fread_number(fp);
	break;
	
      case 'M' : /* mana room */
	// Modified by SinaC 2001
	pRoomIndex->bstat(manarate) = fread_number(fp);
	break;
		
      case 'C' : /* clan */	   
	if (pRoomIndex->clan) {
	  bug("Load_room_rom: duplicate clan fields.");
	  exit(1);
	}
	pRoomIndex->clan = clan_lookup(fread_string(fp));
	break;

      case 'D' : 
	door = fread_number( fp );
	//if ( door < 0 || door > 5 ) { // Modified by SinaC 2003
	if ( door < 0 || door >= MAX_DIR ) {
	  bug( "Load_room_rom: vnum %d has bad door number.", vnum );
	  exit( 1 );
	}

	//pexit			= (EXIT_DATA *) alloc_perm( sizeof(*pexit) );
	pexit = new_exit();
	pexit->description	= fread_string( fp );
	pexit->keyword		= fread_string( fp );
	pexit->exit_info	= 0;
	pexit->rs_flags         = 0;                    /* OLC */
	locks			= fread_number( fp );
	pexit->key		= fread_number( fp );
	pexit->u1.vnum		= fread_number( fp );
	pexit->orig_door	= door;			/* OLC */

	switch ( locks ) {
	case 1: pexit->exit_info = EX_ISDOOR;               
	  pexit->rs_flags  = EX_ISDOOR;		     break;
	case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
	  pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF; break;
	case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;    
	  pexit->rs_flags  = EX_ISDOOR | EX_NOPASS;    break;
	case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
	  pexit->rs_flags  = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
	  break;
	}

	pRoomIndex->exit[door]	= pexit;
	pRoomIndex->old_exit[door] = pexit;
	break;

      case 'E' :
	//ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	ed = new_extra_descr();
	ed->keyword		= fread_string( fp );
	ed->description		= fread_string( fp );
	ed->next		= pRoomIndex->extra_descr;
	pRoomIndex->extra_descr	= ed;
	break;

      case 'O' :
	if (pRoomIndex->owner[0] != '\0') {
	  bug("Load_room_rom: duplicate owner.");
	  exit(1);
	}

	pRoomIndex->owner = fread_string(fp);
	/* But before we did a str_dup("") !! -- Oxtal*/
	break;
      case 'G' :
	pRoomIndex->guild = fread_flag(fp);
	break;		

      default :	    
	bug( "Load_room_rom: vnum %d has unknown flag.", vnum );
	exit( 1 );
      }

    }

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

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum( vnum );                                    /* OLC */
  }

  return;
}

// Added by SinaC 2001              FOR OLD VERSION     (MERC)
/*
 * Snarf an 'area' header line.
 */
void load_old_area( FILE *fp )
{
  AREA_DATA *pArea;

#ifdef VERBOSE
  log_stringf("Loading area header (Merc/Rom/Smaug)");
#endif

  //pArea		= (AREA_DATA *) alloc_perm( sizeof(*pArea) );
  pArea = new_area();
  /*  pArea->reset_first	= NULL;
      pArea->reset_last	= NULL; */
  //pArea->file_name	= fread_string(fp);
  pArea->file_name    = str_dup( strArea );
#ifdef VERBOSE
  log_stringf("file name read: %s", pArea->file_name);
#endif

  pArea->area_flags   = AREA_LOADING;         /* OLC */
  pArea->security     = 9;                    /* OLC */ /* 9 -- Hugin */
  pArea->builders     = str_dup( "None" );    /* OLC */
  pArea->vnum         = top_area;             /* OLC */

  pArea->name		= fread_string(fp);
#ifdef VERBOSE
  log_stringf("name read: %s", pArea->name);
#endif
  pArea->credits	= fread_string(fp);
#ifdef VERBOSE
  log_stringf("credits read: %s", pArea->credits);
#endif
  pArea->min_vnum	= fread_number(fp);
#ifdef VERBOSE
  log_stringf("min vnum read: %d", pArea->min_vnum);
#endif
  pArea->max_vnum	= fread_number(fp);
#ifdef VERBOSE
  log_stringf("max vnum read: %d", pArea->max_vnum);
#endif
  pArea->age		= 15;
  pArea->nplayer	= 0;
  pArea->totalnplayer = 0;
  pArea->empty	= FALSE;
  // Added by SinaC 2001
  pArea->low_range        = 500;
  pArea->high_range       = 0;


  if ( !area_first )
    area_first = pArea;
  if ( area_last ) {
    area_last->next = pArea;
    REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
  }
  area_last	= pArea;
  pArea->next	= NULL;

  return;
}

/*
 * Snarf a reset section.  old version
 */
void load_old_resets( FILE *fp )
{
  RESET_DATA *pReset;
  int         iLastRoom = 0;
  int         iLastObj  = 0;

  if ( !area_last ) {
    bug( "Load_old_resets: no #AREA seen yet.");
    exit( 1 );
  }

#ifdef VERBOSE
  log_stringf("loading old resets");
#endif

  for ( ; ; ) {
    ROOM_INDEX_DATA *pRoomIndex;
    EXIT_DATA *pexit;
    char letter;
    OBJ_INDEX_DATA *temp_index;
    /* int temp; */

    if ( ( letter = fread_letter( fp ) ) == 'S' )
      break;

#ifdef VERBOSE
    log_stringf("letter: %c", letter );
#endif

    if ( letter == '*' ) {
      fread_to_eol( fp );
      continue;
    }
	
    //pReset		= (RESET_DATA *) alloc_perm( sizeof(*pReset) );
    pReset = new_reset_data();
    pReset->command	= letter;
    /* if_flag */	  fread_number( fp );
#ifdef VERBOSE
    log_stringf("if_flag read");
#endif
    pReset->arg1	= fread_number( fp );
#ifdef VERBOSE
    log_stringf("arg1 read: %d", pReset->arg1 );
#endif
    pReset->arg2	= fread_number( fp );
#ifdef VERBOSE
    log_stringf("arg2 read: %d", pReset->arg2 );
#endif
    //    pReset->arg3	= fread_number( fp ); SinaC 2003
    pReset->arg3	= (letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
#ifdef VERBOSE
    log_stringf("arg3 read: %d", pReset->arg3 );
#endif
    // no Arg4 in old version
    pReset->arg4 = pReset->arg2;
    
    fread_to_eol( fp );
	
    /*
     * Validate parameters.
     * We're calling the index functions for the side effect.
     */
    switch ( letter ) {
    default:
      bug( "Load_old_resets: bad command '%c'.", letter );
      exit( 1 );
      break;

    case 'M':
      get_mob_index  ( pReset->arg1 );
      /*	    get_room_index ( pReset->arg3 ); */
      if ( pReset->arg2 == 0 )
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
      }
      break;

    case 'P':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
      get_obj_index  ( pReset->arg1 );
      if ( pReset->arg2 == 0 )
	bug("reset M has arg2 equals to 0 (room: %d)", iLastObj );
      if ( ( pRoomIndex = get_room_index ( iLastObj ) ) ) {
	new_reset( pRoomIndex, pReset );
      }
      break;

    case 'G':
    case 'E':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++; 
      if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = iLastRoom;
      }
      break;

    case 'D':
      pRoomIndex = get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0
	   ||   pReset->arg2 >= MAX_DIR    // > ( MAX_DIR-1)  Modified by SinaC 2003
	   || !pRoomIndex
	   || !( pexit = pRoomIndex->exit[pReset->arg2] )
	   || !IS_SET( pexit->rs_flags, EX_ISDOOR ) ) {
	bug( "Load_old_resets: 'D': exit %d not door.", pReset->arg2 );
	//exit( 1 );
	continue;
      }

      switch ( pReset->arg3 ) {
      default:
	bug( "Load_old_resets: 'D': bad 'locks': %d." , pReset->arg3);
      case 0: break;
      case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
	SET_BIT( pexit->exit_info, EX_CLOSED ); break;
      case 2: SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
	SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED ); break;
      }

      break;

    case 'R':
      pRoomIndex		= get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0 || pReset->arg2 >= MAX_DIR ) {
	bug( "Load_old_resets: 'R': bad exit %d.", pReset->arg2 );
	exit( 1 );
      }

      if ( pRoomIndex )
	new_reset( pRoomIndex, pReset );

      break;
    }
  }

  return;
}

// Added by SinaC 2001
/*
 * Snarf a room section.   old style
 */
void load_old_rooms( FILE *fp )
{
  ROOM_INDEX_DATA *pRoomIndex;

  if ( area_last == NULL ) {
    bug( "Load_old_rooms: no #AREA seen yet.");
    exit( 1 );
  }

#ifdef VERBOSE
  log_stringf("Loading old rooms");
#endif

  for ( ; ; ) {
    int vnum;
    char letter;
    int door;
    int iHash;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_old_rooms: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );

    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_room_index( vnum ) != NULL ) {
      bug( "Load_old_rooms: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //pRoomIndex			= (ROOM_INDEX_DATA *) alloc_perm( sizeof(*pRoomIndex) );
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
    // Modified by SinaC 2001
    pRoomIndex->bstat(flags)		= fread_flag( fp );
    convert_room_flags( pRoomIndex ); // SinaC 2003

    /* horrible hack */
    /* we don't need that now, has been saved with OLC, SinaC 2000
       if ( 3000 <= vnum && vnum < 3400)
       SET_BIT(pRoomIndex->room_flags,ROOM_LAW);
    */

    // Modified by SinaC 2001
    pRoomIndex->bstat(sector)		= fread_number( fp );
    // Added by SinaC 2000
    if ( pRoomIndex->bstat(sector) < 0 || pRoomIndex->bstat(sector) > SECT_MAX-1 )
      bug("Invalid sector type (%d) in room %d",
	  pRoomIndex->bstat(sector), pRoomIndex->vnum );

    // Added by SinaC 2001
    pRoomIndex->bstat(maxsize) = SIZE_NOSIZE;

    // Modified by SinaC 2001
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
      case 'H' : /* healing room */
	// Modified by SinaC 2001
	pRoomIndex->bstat(healrate) = fread_number(fp);
	break;
	
      case 'M' : /* mana room */
	// Modified by SinaC 2001
	pRoomIndex->bstat(manarate) = fread_number(fp);
	break;

	// Added by SinaC 2001 for mental user
      case 'P' : // psp room
	// Modified by SinaC 2001
	pRoomIndex->bstat(psprate) = fread_number(fp);
	break;
	
		
      case 'C' : /* clan */	   
	if (pRoomIndex->clan) {
	  bug("Load_old_rooms: duplicate clan fields.");
	  exit(1);
	}
	pRoomIndex->clan = clan_lookup(fread_string(fp));
	break;

      case 'D' : 
	door = fread_number( fp );
	if ( door < 0 || door >= MAX_DIR ) { // Modified by SinaC 2003
	  bug( "load_old_rooms: vnum %d has bad door number.", vnum );
	  exit( 1 );
	}

	//pexit			= (EXIT_DATA *) alloc_perm( sizeof(*pexit) );
	pexit = new_exit();
	pexit->description	= fread_string( fp );
	pexit->keyword		= fread_string( fp );
	pexit->exit_info	= 0;
	pexit->rs_flags         = 0;                    /* OLC */
	locks			= fread_number( fp );
	pexit->key		= fread_number( fp );
	pexit->u1.vnum		= fread_number( fp );
	pexit->orig_door	= door;			/* OLC */

	switch ( locks ) {
	case 1: pexit->exit_info = EX_ISDOOR;               
	  pexit->rs_flags  = EX_ISDOOR;		     break;
	case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
	  pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF; break;
	case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;    
	  pexit->rs_flags  = EX_ISDOOR | EX_NOPASS;    break;
	case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
	  pexit->rs_flags  = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
	  break;
	}

	pRoomIndex->exit[door]	= pexit;
	pRoomIndex->old_exit[door] = pexit;
	break;

      case 'E' :
	//ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	ed = new_extra_descr();
	ed->keyword		= fread_string( fp );
	ed->description		= fread_string( fp );
	ed->next		= pRoomIndex->extra_descr;
	pRoomIndex->extra_descr	= ed;
	break;

      case 'O' :
	if (pRoomIndex->owner[0] != '\0') {
	  bug("Load_old_rooms: duplicate owner.");
	  exit(1);
	}

	pRoomIndex->owner = fread_string(fp);
	/* But before we did a str_dup("") !! -- Oxtal*/
	break;
      case 'G' :
	pRoomIndex->guild = fread_flag(fp);
	break;		

      default :	    
	bug( "Load_old_rooms: vnum %d has unknown flag.", vnum );
	exit( 1 );
      }

    }

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

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum( vnum );                                    /* OLC */
  }

  return;
}



/*
 * Snarf a mob section.  old style 
 */
void load_old_mob( FILE *fp )
{
  MOB_INDEX_DATA *pMobIndex;
  /* for race updating */
  int race;
  char name[MAX_STRING_LENGTH];

  if ( !area_last ) {  /* OLC */
    bug( "Load_old_mob: no #AREA seen yet.");
    exit( 1 );
  }

#ifdef VERBOSE
  log_stringf("Loading old mobiles");
#endif

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_old_mob: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_mob_index( vnum ) != NULL ) {
      bug( "Load_old_mob: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //pMobIndex			= (MOB_INDEX_DATA *) alloc_perm( sizeof(*pMobIndex) );
    pMobIndex = new_mob_index();
    pMobIndex->vnum			= vnum;
    pMobIndex->area                 = area_last;               /* OLC */
    pMobIndex->new_format		= FALSE;
    pMobIndex->player_name		= fread_string( fp );
    pMobIndex->short_descr		= fread_string( fp );
    pMobIndex->long_descr		= fread_string_upper( fp );
    pMobIndex->description		= fread_string_upper( fp );

    pMobIndex->act			= fread_flag( fp ) | ACT_IS_NPC;
    pMobIndex->affected_by		= fread_flag( fp );
    // Added by SinaC 2001
    pMobIndex->affected2_by		= 0; // no affect2

    pMobIndex->pShop		= NULL;
    // Modified by SinaC 2000 for alignment/etho
    pMobIndex->align.alignment		= fread_number( fp );
    pMobIndex->align.etho               = 0; // neutral
    letter				= fread_letter( fp );
    pMobIndex->level		= fread_number( fp );
    // Added by SinaC 2001
    if ( pMobIndex->level < pMobIndex->area->low_range )
      pMobIndex->area->low_range = pMobIndex->level;
    if ( pMobIndex->level > pMobIndex->area->high_range )
      pMobIndex->area->high_range = pMobIndex->level;


    /*
     * The unused stuff is for imps who want to use the old-style
     * stats-in-files method.
     */
    fread_number( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    /* 'd'		*/		  fread_letter( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    /* '+'		*/		  fread_letter( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    /* 'd'		*/		  fread_letter( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    /* '+'		*/		  fread_letter( fp );	/* Unused */
    fread_number( fp );	/* Unused */
    pMobIndex->wealth               = fread_number( fp )/20;	
    /* xp can't be used! */		  fread_number( fp );	/* Unused */
    pMobIndex->start_pos		= fread_number( fp );	/* Unused */
    pMobIndex->default_pos		= fread_number( fp );	/* Unused */

    if (pMobIndex->start_pos < POS_SLEEPING)
      pMobIndex->start_pos = POS_STANDING;
    if (pMobIndex->default_pos < POS_SLEEPING)
      pMobIndex->default_pos = POS_STANDING;

    /*
     * Back to meaningful values.
     */
    pMobIndex->sex			= fread_number( fp );

    /* compute the race BS */
    one_argument(pMobIndex->player_name,name);
 
    // Modified by SinaC 2000
    //if (name[0] == '\0' || (race =  race_lookup(name)) == 0)
    if (name[0] == '\0' || (race =  race_lookup(name,TRUE)) < 0)	{
      bug("Invalid race for mob %d [%s], assuming %s.", pMobIndex->vnum, name, DEFAULT_RACE_NAME );
      /* fill in with blanks */
      //pMobIndex->race = race_lookup("human");
      pMobIndex->race = DEFAULT_RACE;
      pMobIndex->off_flags = OFF_DODGE|OFF_DISARM|OFF_TRIP|ASSIST_VNUM;
      pMobIndex->imm_flags = 0;
      pMobIndex->res_flags = 0;
      pMobIndex->vuln_flags = 0;
      pMobIndex->form = FORM_EDIBLE|FORM_SENTIENT|FORM_BIPED|FORM_MAMMAL;
      pMobIndex->parts = PART_HEAD|PART_ARMS|PART_LEGS|PART_HEART|
	PART_BRAINS|PART_GUTS;
    }
    else {
      pMobIndex->race = race;
      pMobIndex->off_flags = OFF_DODGE|OFF_DISARM|OFF_TRIP|ASSIST_RACE|
	race_table[race].off;
      pMobIndex->imm_flags = race_table[race].imm;
      pMobIndex->res_flags = race_table[race].res;
      pMobIndex->vuln_flags = race_table[race].vuln;
      pMobIndex->form = race_table[race].form;
      pMobIndex->parts = race_table[race].parts;
    }

    if ( letter != 'S' ) {
      bug( "Load_mobiles: vnum %d non-S.", vnum );
      exit( 1 );
    }

    for ( ; ; ) {
      letter = fread_letter( fp );

      // skipping these version of mobprograms
      if ( letter == 'E' ) {
	fread_to_eol(fp); // skips the trigger
	skip_till_(fp,'~'); // skips action(s)
      }
      else {
	ungetc(letter,fp);
	break;
      }
    }

    convert_act_flags( pMobIndex );
    convert_mobile( pMobIndex );                           /* ROM OLC */
    fix_parts( pMobIndex );

    iHash			= vnum % MAX_KEY_HASH;
    pMobIndex->next		= mob_index_hash[iHash];
    mob_index_hash[iHash]	= pMobIndex;
    top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
    assign_area_vnum( vnum );                                  /* OLC */
    kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
  }

  return;
}

/*
 * Snarf an obj section.  old style 
 */
void load_old_obj( FILE *fp )
{
  OBJ_INDEX_DATA *pObjIndex;

  if ( !area_last ) {  /* OLC */
    bug( "Load_old_obj: no #AREA seen yet.");
    exit( 1 );
  }

#ifdef VERBOSE
  log_stringf("Loading old objects");
#endif

  for ( ; ; ) {
    int vnum;
    char letter;
    int iHash;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_old_obj: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_obj_index( vnum ) != NULL ) {
      bug( "Load_old_obj: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //pObjIndex			= (OBJ_INDEX_DATA *) alloc_perm( sizeof(*pObjIndex) );
    pObjIndex = new_obj_index();
    pObjIndex->vnum			= vnum;
    pObjIndex->area                 = area_last;            /* OLC */
    pObjIndex->new_format		= FALSE;
    pObjIndex->reset_num	 	= 0;
    pObjIndex->name			= fread_string( fp );
    pObjIndex->short_descr		= fread_string_lower( fp );
    pObjIndex->description		= fread_string_upper( fp );
    /* Action description */	  fread_string( fp );

    // Modified by SinaC 2001
    //pObjIndex->material		= str_dup("");
    pObjIndex->material		= 0;

    pObjIndex->item_type		= fread_number( fp );
    pObjIndex->extra_flags		= fread_flag( fp );
    pObjIndex->wear_flags		= fread_flag( fp );
    pObjIndex->value[0]		= fread_number( fp );
    pObjIndex->value[1]		= fread_number( fp );
    pObjIndex->value[2]		= fread_number( fp );
    pObjIndex->value[3]		= fread_number( fp );
    pObjIndex->value[4]		= 0;
    pObjIndex->level		= 0;
    pObjIndex->condition 		= 100;
    pObjIndex->weight		= fread_number( fp );
    pObjIndex->cost			= fread_number( fp );	/* Unused */
    /* Cost per day */		  fread_number( fp );

    // Added by SinaC 2003
    pObjIndex->size = SIZE_NOSIZE;

    if (pObjIndex->item_type == ITEM_WEAPON) {
      if (is_name("two",pObjIndex->name) 
	  ||  is_name("two-handed",pObjIndex->name) 
	  ||  is_name("claymore",pObjIndex->name))
	SET_BIT(pObjIndex->value[4],WEAPON_TWO_HANDS);
    }

    for ( ; ; ) {
      char letter;

      letter = fread_letter( fp );

      if ( letter == 'A' ) {
	AFFECT_DATA *paf;

	//paf                     = (AFFECT_DATA*) alloc_perm( sizeof(*paf) );
	paf = new_affect();
	createaff(*paf,-1,20,-1,0,AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT); // FIXME: this doesn't follow new affect philosophy
	addaff2(*paf,AFTO_CHAR,locoldtonew(fread_number( fp )),AFOP_ADD,fread_number( fp ));
	//paf->where              = AFTO_CHAR;
	//paf->op                 = AFOP_ADD;
	//paf->type		= -1;
	//paf->level		= 20; /* RT temp fix */
	//paf->duration		= -1;
	//paf->location           = locoldtonew(fread_number( fp ));
	//paf->modifier		= fread_number( fp );

	paf->next		= pObjIndex->affected;
	pObjIndex->affected	= paf;
      }
      else if ( letter == 'E' ) {
	EXTRA_DESCR_DATA *ed;

	//ed			= (EXTRA_DESCR_DATA *) alloc_perm( sizeof(*ed) );
	ed = new_extra_descr();
	ed->keyword		= fread_string( fp );
	ed->description		= fread_string( fp );
	ed->next		= pObjIndex->extra_descr;
	pObjIndex->extra_descr	= ed;
      }
      else {
	ungetc( letter, fp );
	break;
      }
    }

    /* fix armors */
    if (pObjIndex->item_type == ITEM_ARMOR) {
      pObjIndex->value[1] = pObjIndex->value[0];
      pObjIndex->value[2] = pObjIndex->value[1];
    }

    /*
     * Translate spell "slot numbers" to internal "skill numbers."
     */
    switch ( pObjIndex->item_type ) {
    case ITEM_PILL:
    case ITEM_POTION:
    case ITEM_SCROLL:
    // Added by SinaC 2003
    case ITEM_TEMPLATE:
      pObjIndex->value[1] = slot_lookup( pObjIndex->value[1] );
      pObjIndex->value[2] = slot_lookup( pObjIndex->value[2] );
      pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
      pObjIndex->value[4] = slot_lookup( pObjIndex->value[4] );
      break;

    case ITEM_STAFF:
    case ITEM_WAND:
      pObjIndex->value[3] = slot_lookup( pObjIndex->value[3] );
      break;
    }

    iHash			= vnum % MAX_KEY_HASH;
    pObjIndex->next		= obj_index_hash[iHash];
    obj_index_hash[iHash]	= pObjIndex;
    top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   /* OLC */
    assign_area_vnum( vnum );                                   /* OLC */
  }

  return;
}




/*
 * OLC
 * Use these macros to load any new area formats that you choose to
 * support on your MUD.  See the new_load_area format below for
 * a short example.
 */
#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )                \
                if ( !str_cmp( word, literal ) )    \
                {                                   \
                    field  = value;                 \
                    fMatch = TRUE;                  \
                    break;                          \
                                }

#define SKEY( string, field )                       \
                if ( !str_cmp( word, string ) )     \
                {                                   \
                    field = fread_string( fp );     \
                    fMatch = TRUE;                  \
                    break;                          \
                                }



/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
void load_area( FILE *fp )
{
  AREA_DATA *pArea;
  const char      *word;
  bool      fMatch;

  //pArea               = (AREA_DATA *) alloc_perm( sizeof(*pArea) );
  pArea = new_area();
  pArea->age          = 15;
  pArea->nplayer      = 0;
  pArea->totalnplayer = 0;
  pArea->file_name    = str_dup( strArea );
  pArea->vnum         = top_area;
  pArea->name         = str_dup( "New Area" );
  pArea->builders     = str_dup( "" );
  pArea->security     = 9;                    /* 9 -- Hugin */
  pArea->min_vnum     = 0;
  pArea->max_vnum     = 0;
  pArea->area_flags   = 0;
  /*  pArea->recall       = ROOM_VNUM_TEMPLE;        ROM OLC */
  // Added by SinaC 2000 for teleport room
  //pArea->teleport_room= 0;
  // Added by SinaC 2000 for area earthquake
  pArea->earthquake_on = FALSE;
  pArea->earthquake_duration = 0;
  // Added by SinaC 2001
  pArea->low_range        = 500;
  pArea->high_range       = 0;

  for ( ; ; ) {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

    switch ( UPPER(word[0]) ) {
      /* Removed by SinaC 2003, scripts can do that
      // Added by SinaC 2000 for teleport room
    case 'T':
      KEY( "Teleport", pArea->teleport_room, fread_number( fp ) );
      break;
      */
      // Added by SinaC 2000 to save AREA FLAG
    case 'F':
      KEY( "Flags", pArea->area_flags, fread_flag(fp) );
      break;

    case 'N':
      SKEY( "Name", pArea->name );
      break;
    case 'S':
      KEY( "Security", pArea->security, fread_number( fp ) );
      break;
    case 'V':
      if ( !str_cmp( word, "VNUMs" ) ) {
	pArea->min_vnum = fread_number( fp );
	pArea->max_vnum = fread_number( fp );
      }
      break;
    case 'E':
      if ( !str_cmp( word, "End" ) ) {
	fMatch = TRUE;
	if ( area_first == NULL )
	  area_first = pArea;
	//	if ( area_last  != NULL )
	//	  area_last->next = pArea;
	if ( area_last ) { // Modified by SinaC 2003, seems to crash without that
	  area_last->next = pArea;
	  REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
	}
	area_last   = pArea;
	pArea->next = NULL;
	return;
      }
      break;
    case 'B':
      SKEY( "Builders", pArea->builders );
      break;
    case 'C':
      SKEY( "Credits", pArea->credits );
      break;
    }
  }
}

// Old merc format
void load_old_merc_area( FILE *fp ) {
  AREA_DATA *pArea;

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

  const char *s = fread_string(fp);
  pArea->credits = str_dup( s );
  //{Low  High}  Builders  Areaname
  char build[128];
  char name[128];
  sscanf( s, "{%2d %2d} %s",
	  &pArea->low_range, &pArea->high_range,
	  build );
  pArea->builders = str_dup( build );
  
  strcpy( name, index( index( index( s, '}' ), ' ' )+1, ' ' )+1 );
  pArea->name = str_dup( trim( name ) );

  if ( !area_first )
    area_first = pArea;
  if ( area_last ) {
    area_last->next = pArea;
    REMOVE_BIT(area_last->area_flags, AREA_LOADING);        /* OLC */
  }
  area_last	= pArea;
  pArea->next	= NULL;
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void assign_area_vnum( int vnum )
{
  if ( area_last->min_vnum == 0 || area_last->max_vnum == 0 )
    area_last->min_vnum = area_last->max_vnum = vnum;
  if ( vnum != URANGE( area_last->min_vnum, vnum, area_last->max_vnum ) )
    if ( vnum < area_last->min_vnum )
      area_last->min_vnum = vnum;
    else
      area_last->max_vnum = vnum;
  return;
}

/*
 * Snarf a help section.
 */
const int MAX_PER_LINE = 80;
void load_helps( FILE *fp )
{
  HELP_DATA *pHelp;

  for ( ; ; ) {
    //pHelp		= (HELP_DATA *) alloc_perm( sizeof(*pHelp) );
    pHelp = new HELP_DATA;
    top_help++;

    pHelp->level	= fread_number( fp );
    pHelp->keyword	= fread_string( fp );
    if ( pHelp->keyword[0] == '$' )
      break;
    pHelp->text	= fread_string( fp );

    // SinaC 2003, check number of character/line
    const char *p = pHelp->text;
    int count = 0;
    while ( *p != '\0' ) {
      if ( *p == '\n' ) {
	if ( count > MAX_PER_LINE ) {
	  log_stringf("Help [%s] has too many characters/line [%d]", pHelp->keyword, count );
	  break; // no need to continue
	}
	count = 0;
      }
      else if ( *p == '{' ) { // skips color code
	p++;
	if ( *p == '\0' ) // to be sure
	  break;
      }
      else if ( *p != '\r' ) // \r doesn't count
	count++;
      p++;
    }

    if ( !str_cmp( pHelp->keyword, "greeting" ) )
      help_greeting = pHelp->text;

    if ( help_first == NULL )
      help_first = pHelp;
    if ( help_last  != NULL )
      help_last->next = pHelp;

    help_last	= pHelp;
    pHelp->next	= NULL;
  }
  return;
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset( ROOM_INDEX_DATA *pR, RESET_DATA *pReset )
{
  RESET_DATA *pr;

  if ( !pR )
    return;

  pr = pR->reset_last;

  if ( !pr ) {
    pR->reset_first = pReset;
    pR->reset_last  = pReset;
  }
  else {
    pR->reset_last->next = pReset;
    pR->reset_last       = pReset;
    pR->reset_last->next = NULL;
  }

  return;
}

/*
 * Snarf a reset section.
 */
void load_resets( FILE *fp )
{
  RESET_DATA *pReset;
  int         iLastRoom = 0;
  int         iLastObj  = 0;

  if ( !area_last ) {
    bug( "Load_resets: no #AREA seen yet.");
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
	
    //pReset		= (RESET_DATA *) alloc_perm( sizeof(*pReset) );
    pReset = new_reset_data();
    pReset->command	= letter;
    /* if_flag */	  fread_number( fp );
    pReset->arg1	= fread_number( fp );
    pReset->arg2	= fread_number( fp );
    pReset->arg3	= (letter == 'G' || letter == 'R' ) ? 0 : fread_number( fp );
    // Modified by SinaC 2000  fuck
    pReset->arg4	= (letter == 'P' || letter == 'M' || letter == 'Z' ) ? fread_number(fp) : 0;
    /*	pReset->arg4	= (letter == 'P' || letter == 'M')
	? ((temp == 0)? 1 : fread_number(fp)) : 0; */
    fread_to_eol( fp );
	
    /*
     * Validate parameters.
     * We're calling the index functions for the side effect.
     */
    switch ( letter ) {
    default:
      bug( "Load_resets: bad command '%c'.", letter );
      exit( 1 );
      break;

    case 'M':
      get_mob_index  ( pReset->arg1 );
      /*	    get_room_index ( pReset->arg3 ); */
      // fix added by SinaC 2001
      if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
	bug("reset M has arg2 or arg4 equal to 0 (room: %d)", pReset->arg3);

      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastRoom = pReset->arg3;
      }
      break;

    case 'O': 
      temp_index = get_obj_index ( pReset->arg1 );
      temp_index->reset_num++; 
      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = pReset->arg3;
      }
      break;

    case 'P':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++;
      get_obj_index  ( pReset->arg1 );
      if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
	bug("reset P has arg2 or arg4 equal to 0 (room: %d)", iLastObj);

      if ( ( pRoomIndex = get_room_index ( iLastObj ) ) ) {
	new_reset( pRoomIndex, pReset );
      }
      break;

    case 'G':
    case 'E':
      temp_index = get_obj_index  ( pReset->arg1 );
      temp_index->reset_num++; 
      if ( ( pRoomIndex = get_room_index ( iLastRoom ) ) ) {
	new_reset( pRoomIndex, pReset );
	iLastObj = iLastRoom;
      }
      break;

    case 'D':
      pRoomIndex = get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0
	   ||   pReset->arg2 >= MAX_DIR // > (MAX_DIR - 1)   Modified by SinaC 2003
	   || !pRoomIndex
	   || !( pexit = pRoomIndex->exit[pReset->arg2] )
	   || !IS_SET( pexit->rs_flags, EX_ISDOOR ) ) {
	bug( "Load_resets: 'D': exit %d not door.", pReset->arg2 );
	exit( 1 );
      }

      switch ( pReset->arg3 ) {
      default:
	bug( "Load_resets: 'D': bad 'locks': %d." , pReset->arg3);
      case 0: break;
      case 1: SET_BIT( pexit->rs_flags, EX_CLOSED );
	SET_BIT( pexit->exit_info, EX_CLOSED ); break;
      case 2: SET_BIT( pexit->rs_flags, EX_CLOSED | EX_LOCKED );
	SET_BIT( pexit->exit_info, EX_CLOSED | EX_LOCKED ); break;
      }

      break;

    case 'R':
      pRoomIndex		= get_room_index( pReset->arg1 );

      if ( pReset->arg2 < 0 || pReset->arg2 >= MAX_DIR ) {
	bug( "Load_resets: 'R': bad exit %d.", pReset->arg2 );
	exit( 1 );
      }

      if ( pRoomIndex )
	new_reset( pRoomIndex, pReset );

      break;
      // Added by SinaC and JyP ( aka Oxtal ) 2000 for random maze
    case 'Z':
      if ( pReset->arg1 < 2 
	   || pReset->arg2 < 2 
	   || pReset->arg1 * pReset->arg2 > 100 ) {
	bug("Load_resets: 'Z': bad width, height (room %d).",pReset->arg3);
	exit( 1 );
      }
      if ( pReset->arg4 != 0 && !get_obj_index( pReset->arg4 )) {
	bug("Load_resets: 'Z': bad map vnum: %d",pReset->arg4);
	exit( 1 );
      }
      if ( ( pRoomIndex = get_room_index ( pReset->arg3 ) ) ) {
	new_reset( pRoomIndex, pReset );
	//    iLastRoom = pReset->arg3;
      }
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

/*
 * Snarf a room section.
 */
void load_rooms( FILE *fp )
{
  ROOM_INDEX_DATA *pRoomIndex;

  if ( area_last == NULL ) {
    bug( "Load_rooms: no #AREA seen yet.");
    exit( 1 );
  }

  for ( ; ; ) {
    int vnum;
    char letter;
    int door;
    int iHash;

    letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "Load_rooms: # not found.");
      exit( 1 );
    }

    vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;

    fBootDb = FALSE;
    if ( get_room_index( vnum ) != NULL ) {
      bug( "Load_rooms: vnum %d duplicated.", vnum );
      exit( 1 );
    }
    fBootDb = TRUE;

    //pRoomIndex			= (ROOM_INDEX_DATA *) alloc_perm( sizeof(*pRoomIndex) );
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
    // Modified by SinaC 2001
    pRoomIndex->bstat(flags)		= fread_flag( fp );
    convert_room_flags( pRoomIndex ); // SinaC 2003

    /* horrible hack */
    /* we don't need that now, has been saved with OLC, SinaC 2000
       if ( 3000 <= vnum && vnum < 3400)
       SET_BIT(pRoomIndex->room_flags,ROOM_LAW);
    */

    // Modified by SinaC 2001
    pRoomIndex->bstat(sector)		= fread_number( fp );
    // Added by SinaC 2000
    if ( pRoomIndex->bstat(sector) < 0 || pRoomIndex->bstat(sector) > SECT_MAX-1 )
      bug("Invalid sector type (%d) in room %d",
	  pRoomIndex->bstat(sector), pRoomIndex->vnum );

    // Modified by SinaC 2001
    // Added by SinaC 2001 for maximal room size
    pRoomIndex->bstat(maxsize)                = fread_number( fp );
    if ( pRoomIndex->bstat(maxsize) == SIZE_GIANT ) // little trick
      pRoomIndex->bstat(maxsize) == SIZE_NOSIZE;

    // Modified by SinaC 2001
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
      //int locks; removed by SinaC 2001
      EXTRA_DESCR_DATA *ed;
    
      letter = fread_letter( fp );

      if ( letter == 'S' )
	break;
	    
      switch ( letter ) {
      case 'H' : /* healing room */
	// Modified by SinaC 2001
	pRoomIndex->bstat(healrate) = fread_number(fp);
	break;
	
      case 'M' : /* mana room */
	// Modified by SinaC 2001
	pRoomIndex->bstat(manarate) = fread_number(fp);
	break;

	// Added by SinaC 2001 for mental user
      case 'P' : // psp room
	// Modified by SinaC 2001
	pRoomIndex->bstat(psprate) = fread_number(fp);
	break;
	
		
      case 'C' : /* clan */	   
	if (pRoomIndex->clan) {
	  bug("Load_rooms: duplicate clan fields.");
	  exit(1);
	}
	pRoomIndex->clan = clan_lookup(fread_string(fp));
	break;

      case 'D' : 
	door = fread_number( fp );
	if ( door < 0 || door >= MAX_DIR ) { // Modified by SinaC 2003
	  bug( "Fread_rooms: vnum %d has bad door number.", vnum );
	  exit( 1 );
	}

	//	pexit			= (EXIT_DATA *) alloc_perm( sizeof(*pexit) );
	pexit = new_exit();
	pexit->description	= fread_string( fp );
	pexit->keyword		= fread_string( fp );
	pexit->exit_info	= 0;
	pexit->rs_flags         = 0;                    /* OLC */
	// Modified by SinaC 2001
	//locks			= fread_number( fp );
	pexit->rs_flags = pexit->exit_info = fread_flag( fp );
	pexit->key		= fread_number( fp );
	pexit->u1.vnum		= fread_number( fp );
	pexit->orig_door	= door;			/* OLC */

	/* Removed by SinaC 2001
	switch ( locks ) {
	case 1: pexit->exit_info = EX_ISDOOR;               
	  pexit->rs_flags  = EX_ISDOOR;		     break;
	case 2: pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
	  pexit->rs_flags  = EX_ISDOOR | EX_PICKPROOF; break;
	case 3: pexit->exit_info = EX_ISDOOR | EX_NOPASS;    
	  pexit->rs_flags  = EX_ISDOOR | EX_NOPASS;    break;
	case 4: pexit->exit_info = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
	  pexit->rs_flags  = EX_ISDOOR|EX_NOPASS|EX_PICKPROOF;
	  break;
	}
	*/

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

      case 'O' :
	if (pRoomIndex->owner[0] != '\0') {
	  bug("Load_rooms: duplicate owner.");
	  exit(1);
	}

	pRoomIndex->owner = fread_string(fp);
	/* But before we did a str_dup("") !! -- Oxtal*/
	break;
      case 'G' :
	pRoomIndex->guild = fread_flag(fp);
	break;		

	// Added by SinaC 2003 for room programs
	// !! for object and mobile value is 'M'
      case 'Z':
	pRoomIndex->program = hash_get_prog(fread_word(fp));
	if (!pRoomIndex->program)
	  bug("Can't find program for room vnum %d.", pRoomIndex->vnum);	      
	else {
	  if ( get_root_class( pRoomIndex->program ) != default_room_class ) {
	    bug("program for room vnum %d is not a room program.", pRoomIndex->vnum );
	    pRoomIndex->program = NULL;
	  }
	  else
	    if ( pRoomIndex->program->isAbstract )
	      bug("program for room vnum %d is an ABSTRACT class.", pRoomIndex->vnum );
	}
	break;

	// Added by SinaC 2003 for repop time
      case 'R':
	pRoomIndex->time_between_repop = fread_number(fp);
	pRoomIndex->time_between_repop_people = fread_number(fp);
	break;

      case 'Y': {
	AFFECT_DATA *paf;
	char buf[100];
 
	//	paf = (AFFECT_DATA*) alloc_perm( sizeof(*paf) );
	paf = new_affect();
	
	strcpy( buf, fread_word(fp) );
	int where = flag_value( afto_type, buf );
	if ( where == NO_FLAG ) {
	  bug("bad affect location: Room (vnum %d) where: %s",
	      vnum, buf );
	  exit(-1);
	}
	//paf->where = flag_value( afto_type, fread_word(fp));
	strcpy(buf,fread_word(fp));

	int loc = ATTR_NA;
	if (is_number(buf))
	  //paf->location = atoi(buf);
	  loc = atoi(buf);
	else if ( where == AFTO_CHAR )
	  //paf->location = flag_value( attr_flags, buf);
	  loc = flag_value( attr_flags, buf);
	else if ( where == AFTO_ROOM )
	  loc = flag_value( room_attr_flags, buf);
	// Added by SinaC 2000
	//if ( paf->location == NO_FLAG ){
	if ( loc == NO_FLAG ){
	  bug("bad affect location: Room: (vnum %d)  where: %s  location: %s",
	      vnum, flag_string( afto_type, where ), buf );
	  exit(-1);
	}
	createaff(*paf,-1,100,-1,0,AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT); // FIXME: this doesn't follow new affect philosophy
	addaff2(*paf,where,loc,fread_number(fp),fread_number(fp));

	//paf->op = fread_number(fp);
	//paf->modifier = fread_number(fp);
	//paf->duration = -1;
	//paf->level = pObjIndex->level;
	//paf->type = -1;
	paf->next               = pRoomIndex->base_affected;
	pRoomIndex->base_affected     = paf;
	break;
      }
	
      default :	    
	bug( "Load_rooms: vnum %d has unknown flag.", vnum );
	exit( 1 );
      }
    }

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

    iHash			= vnum % MAX_KEY_HASH;
    pRoomIndex->next	= room_index_hash[iHash];
    room_index_hash[iHash]	= pRoomIndex;
    top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; /* OLC */
    assign_area_vnum( vnum );                                    /* OLC */
  }

  return;
}



/*
 * Snarf a shop section.
 */
void load_shops( FILE *fp ) {
  SHOP_DATA *pShop;

  for ( ; ; ) {
    MOB_INDEX_DATA *pMobIndex;
    int iTrade;

    //pShop			= (SHOP_DATA *) alloc_perm( sizeof(*pShop) );
    pShop = new_shop();
    pShop->keeper		= fread_number( fp );
    if ( pShop->keeper == 0 )
      break;
    for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ )
      pShop->buy_type[iTrade]	= fread_number( fp );
    pShop->profit_buy	= fread_number( fp );
    pShop->profit_sell	= fread_number( fp );
    pShop->open_hour	= fread_number( fp );
    pShop->close_hour	= fread_number( fp );
    fread_to_eol( fp );
    pMobIndex		= get_mob_index( pShop->keeper );
    pMobIndex->pShop	= pShop;

    convert_shop( pShop ); // SinaC 2003

    if ( shop_first == NULL )
      shop_first = pShop;
    if ( shop_last  != NULL )
      shop_last->next = pShop;

    shop_last	= pShop;
    pShop->next	= NULL;
  }

  return;
}


/*
 * Snarf spec proc declarations.
 */
void load_specials( FILE *fp )
{
  for ( ; ; ) {
    MOB_INDEX_DATA *pMobIndex;
    char letter;

    switch ( letter = fread_letter( fp ) ) {
    default:
      bug( "Load_specials: letter '%c' not *MS.", letter );
      exit( 1 );

    case 'S':
      return;

    case '*':
      break;

    case 'M':
      pMobIndex		= get_mob_index	( fread_number ( fp ) );
      char *word = fread_word(fp);
      pMobIndex->spec_fun	= spec_lookup	( word );
      if ( pMobIndex->spec_fun == 0 ) {
	bug( "Load_specials: 'M' (vnum %d) invalid special function (%s).", 
	     pMobIndex->vnum, word );
	//exit( 1 );
	pMobIndex->spec_fun = NULL;
      }
      break;
    }

    fread_to_eol( fp );
  }
}


/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits( void ) {
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *pRoomIndex;
  ROOM_INDEX_DATA *to_room;
  EXIT_DATA *pexit;
  EXIT_DATA *pexit_rev;
  int iHash;
  int door;

  log_stringf("Fixing exits.");

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for ( pRoomIndex  = room_index_hash[iHash];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next ) {
      bool fexit;

      fexit = FALSE;
      for ( door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
	if ( ( pexit = pRoomIndex->exit[door] ) != NULL ) {
	  if ( pexit->u1.vnum <= 0 
	       || get_room_index(pexit->u1.vnum) == NULL)
	    pexit->u1.to_room = NULL;
	  else {
	    fexit = TRUE; 
	    pexit->u1.to_room = get_room_index( pexit->u1.vnum );
	  }
	}
      }
      // Modified by SinaC 2001
      if (!fexit)
	SET_BIT(pRoomIndex->cstat(flags),ROOM_NO_MOB);
    }
  }

  for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for ( pRoomIndex  = room_index_hash[iHash];
	  pRoomIndex != NULL;
	  pRoomIndex  = pRoomIndex->next ) {
      for ( door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
	if ( ( pexit     = pRoomIndex->exit[door]       ) != NULL
	     &&   ( to_room   = pexit->u1.to_room            ) != NULL
	     &&   ( pexit_rev = to_room->exit[rev_dir[door]] ) != NULL
	     &&   pexit_rev->u1.to_room != pRoomIndex 
	     /*&&   (pRoomIndex->vnum < 1200 || pRoomIndex->vnum > 1299)*/) {
	  sprintf( buf, "Fix_exits: %d:%d -> %d:%d -> %d.",
		   pRoomIndex->vnum, door,
		   to_room->vnum,    rev_dir[door],
		   (pexit_rev->u1.to_room == NULL)
		   ? 0 : pexit_rev->u1.to_room->vnum );
	  bug( buf, 0 );
	}
      }
    }
  }
  return;
}

// Added by SinaC 2003 to automatically set mob's faction
void fix_factions() {
  log_stringf("Fixing factions.");
  for ( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for ( MOB_INDEX_DATA *pMobIndex  = mob_index_hash[iHash];
	  pMobIndex != NULL;
	  pMobIndex  = pMobIndex->next )
      if ( pMobIndex->faction == -1 ) // only non-already-set faction
	set_default_faction(pMobIndex);
  }
}

/*
 * Repopulate areas periodically.
 */
/*
void area_update( void ) {
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];
  
  for ( pArea = area_first; pArea != NULL; pArea = pArea->next ) {

    sprintf(buf,"area_update: %s   age: %d  empty: %d  nplayer: %d.", 
	    pArea->name, pArea->age, pArea->empty, pArea->nplayer );
    wiznet(buf,NULL,NULL,WIZ_VARIOUS,0,0);

    if ( ++pArea->age < 3 )
      continue;
    // Check age and reset.
    // Note: Mud School resets every 3 minutes (not 15).
    // Area repops every 31 minutes if there is people in
    //             every 15 minutes if there isn't anyone in
    //             every  3 minutes if it's the mudschool
    // Note by SinaC 2003
    // Area repops when the last player in the area leaves it
    //             every 31 minutes if there isn't anyone in !!!!!!!!!!!
    //             every 15 minutes if there is people in
    //             every  3 minutes if it's the mudschool
    if ( ( !pArea->empty 
	   && (pArea->nplayer == 0 || pArea->age >= 15))
	 ||    pArea->age >= 31) {
      ROOM_INDEX_DATA *pRoomIndex;

      reset_area( pArea );
      sprintf(buf,"%s has just been reset.",pArea->name);
      wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);

      pArea->age = number_range( 0, 3 );
      pRoomIndex = get_room_index( ROOM_VNUM_SCHOOL );
      if ( pRoomIndex != NULL && pArea == pRoomIndex->area )
	pArea->age = 15 - 2;
      else if (pArea->nplayer == 0) 
	pArea->empty = TRUE;
    }
  }
  return;
}
*/

// New area update created by SinaC 2003
// Each room can have its own repop time
void area_update( void ) {
  AREA_DATA *pArea;
  char buf[MAX_STRING_LENGTH];

  // When loading a room, don't forget to set the current_time_till_repop to INFINITE so
  //  we are sure to reset every rooms when the mud has booted
  for ( pArea = area_first; pArea != NULL; pArea = pArea->next ) {
    // SinaC 2003, if AREA_SAVE_STATE is set, we do a save_area_state
    if ( IS_SET( pArea->area_flags, AREA_SAVE_STATE ) )
      save_area_state( pArea );

    /*
    sprintf(buf,"area_update: %s   age: %d  empty: %d  nplayer: %d.", 
	    pArea->name, pArea->age, pArea->empty, pArea->nplayer );
    wiznet(buf,NULL,NULL,WIZ_VARIOUS,0,0);
    */
    int countReset = 0;
    int countRoom = 0;
    for ( int vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ ) {
      ROOM_INDEX_DATA *pRoom;
      if ( ( pRoom = get_room_index(vnum) ) ) {
        pRoom->current_time_repop++;
        if ( ( !pArea->empty 
               && ( pRoom->current_time_repop >= pRoom->time_between_repop
		    || pArea->nplayer == 0 ) )
             || pRoom->current_time_repop >= pRoom->time_between_repop_people ) {
	/*
        if ( pArea->empty && pRoom->current_time_repop >= pRoom->time_between_repop
	     || pRoom->current_time_repop >= pRoom->time_between_repop_people ) {
         */
          reset_room(pRoom);
          recomproom(pRoom);
          //pRoom->current_time_repop = number_fuzzy(2)-1; // do we really need this randomize?
	  pRoom->current_time_repop = 0;
          countReset++;
        }
	countRoom++;
      }
    }
    if ( countReset > 0 ) {
      sprintf(buf,"%s (%d/%d rooms) has just been reset.", 
	      pArea->name, countReset, countRoom );
      wiznet(buf,NULL,NULL,WIZ_RESETS,0,0);
    }

    // Still take care about area's age and emptyness
    pArea->age++;
    if ( ( !pArea->empty 
	   && (pArea->nplayer == 0 || pArea->age >= 15))
	 ||    pArea->age >= 31) {
      pArea->age = number_range( 0, 3 );
      if (pArea->nplayer == 0) 
	pArea->empty = TRUE;
    }
  }
  return;
}


// Added by SinaC 2001, if TRUE, room is resetted even if there is a player within
//  set to true in do_resetarea
bool reset_always = FALSE;

/* OLC
 * Reset one room.  Called by reset_area, area_update and olc.
 */
void reset_room( ROOM_INDEX_DATA *pRoom ) {
  RESET_DATA  *pReset;
  CHAR_DATA   *pMob;
  CHAR_DATA	*mob;
  OBJ_DATA    *pObj;
  CHAR_DATA   *LastMob = NULL;
  OBJ_DATA    *LastObj = NULL;
  int level = 0;
  bool last;
  static char maze_map[MAX_STRING_LENGTH];
  int countpc;
  CHAR_DATA *LastRepop = NULL; // SinaC 2003, we call onRepop only when mob+its stuff has been loaded
  int countRepop = 0, countDelayedRepop = 0; // SinaC 2003

    
  if ( !pRoom )
    return;

  pMob        = NULL;
  last        = FALSE;
    
  // Added by SinaC 2003
  // FIXME: call onReset on every mobiles in the room ?
  //  Filthy uses onReset but onReset is never called on mob except when load a world state
  OBJ_DATA *obj, *obj_next = NULL;
  for ( obj = pRoom->contents; obj; obj = obj_next ) {
    obj_next = obj->next_content;
    OBJPROG( obj, NULL, "onReset" );
  }

  for ( int iExit = 0;  iExit < MAX_DIR;  iExit++ ) {
    EXIT_DATA *pExit;
    if ( ( pExit = pRoom->exit[iExit] )
	 // Added by SinaC 2003, if door bashed: only 1/4 chance of updating it
	 && !( IS_SET( pExit->exit_info, EX_BASHED ) 
	       && number_range(0,3) == 0 )
	 // SinaC 2003, when EX_NORESET is set  exit is not updated
	 && !IS_SET( pExit->rs_flags, EX_NORESET ) ) {

      // Added by SinaC 2001 for INFURIATING door
      // 50% chance of being opened/closed cos' the 2 exit sides 
      // need to pass the chance test
      if ( IS_SET( pExit->exit_info, EX_ISDOOR ) 
	   && IS_SET( pExit->exit_info, EX_INFURIATING ) ) {
	EXIT_DATA *pRev = NULL;
	if ( pExit->u1.to_room != NULL )
	  pRev = pExit->u1.to_room->exit[rev_dir[iExit]];
	// if it's closed ==> 50% chance being open
	if ( IS_SET( pExit->exit_info, EX_CLOSED ) ) {
	  if ( number_percent() < 50 ) {
	    log_stringf("INFURIATING OPENED at %d  twoside: %s",
			pRoom->vnum, pRev?"true":"false" );
	    REMOVE_BIT( pExit->exit_info, EX_CLOSED );
	    if ( pRev ) {
	      /* nail the other side */
	      REMOVE_BIT( pRev->exit_info, EX_CLOSED );
	    }
	  }
	}
	// if it's not closed ==> 50% chance being closed
	else {
	  if ( number_percent() < 50 ) {
	    log_stringf("INFURIATING CLOSED at %d  twoside: %s", 
			pRoom->vnum, pRev?"true":"false" );
	    SET_BIT( pExit->exit_info, EX_CLOSED );
	    if ( pRev ) {
	      /* nail the other side */
	      SET_BIT( pRev->exit_info, EX_CLOSED );
	    }
	  }
	}
	continue;
      } // end INFURIATING CODE

      pExit->exit_info = pExit->rs_flags;
      if ( ( pExit->u1.to_room != NULL )
	   && ( ( pExit = pExit->u1.to_room->exit[rev_dir[iExit]] ) ) ) {
	/* nail the other side */
	pExit->exit_info = pExit->rs_flags;
      }
    }
  }

  for ( pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next ) {
    MOB_INDEX_DATA  *pMobIndex;
    OBJ_INDEX_DATA  *pObjIndex;
    OBJ_INDEX_DATA  *pObjToIndex;
    ROOM_INDEX_DATA *pRoomIndex = NULL;
    char buf[MAX_STRING_LENGTH];
    int count,limit=0;
    int i;
    
    switch ( pReset->command ) {
    default:
      bug( "Reset_room: bad command %c.", pReset->command );
      break;
	  
      // Added by SinaC and JyP ( aka Oxtal ) 2000
      // maze: arg1 x arg2 rooms
    case 'Z':
      // too large ?
      if ( pReset->arg1 * pReset->arg2 + pRoom->vnum > pRoom->area->max_vnum ) {
	bug( "Reset_room: ma'Z'e: too large (room %d)",pRoom->vnum);
	break;
      }
      // every room exists ?
      for ( i = pRoom->vnum; i < pRoom->vnum+pReset->arg1 * pReset->arg2; i++ ) {
	if ( ( pRoomIndex = get_room_index( i ) ) == NULL ) {
	  bug( "Reset_room: ma'Z'e: covers an inexisting room(%d)",i);
	  break;
	}
      }
      // if not :))
      if ( i != pRoom->vnum+pReset->arg1 * pReset->arg2 ) 
	break;

      // Okay, now we 'create' the maze
      reset_maze( pRoom->vnum, pReset->arg1, pReset->arg2, maze_map );
      // set the description of the map
      if (pReset->arg4!=0) {
	OBJ_INDEX_DATA *map;
	      
	map = get_obj_index( pReset->arg4 );
	if ( map == NULL ) {
	  bug("Reset_room: ma'Z'e: inexisting map (vnum :%d)",pReset->arg4);
	  break;
	}
	if ( map->extra_descr == NULL ) {
	  bug("Reset_room: ma'Z'e: inexisting extra descr 'map' (vnum: %d)",
	      pReset->arg4);
	  break;
	}
	map->extra_descr->description = str_dup(maze_map);
      }
	  
      break;

    case 'M':
      if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) ) {
	bug( "Reset_room: 'M': bad vnum %d.", pReset->arg1 );
	continue;
      }

      if ( ( pRoomIndex = get_room_index( pReset->arg3 ) ) == NULL ) {
	bug( "Reset_room: 'R': bad vnum %d.", pReset->arg3 );
	continue;
      }

      // Added by SinaC 2001, reupdate mobiles objects condition
      countpc = 0;
      for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room) {
	if ( !IS_NPC(mob) && !IS_IMMORTAL(mob))
	  countpc++;
	if ( !IS_NPC(mob)
	     || IS_AFFECTED(mob, AFF_CHARM)
	     || mob->master != NULL
	     || mob->leader != NULL )
	  continue;
	for ( pObj = mob->carrying; pObj != NULL; pObj = pObj->next_content ) {
	  if ( pObj->condition != pObj->pIndexData->condition ) {
	    log_stringf("UPDATE_COND: [%s]  {%s}  (%d)",
			NAME(mob),pObj->short_descr,
			pObj->condition);
	    pObj->condition = pObj->pIndexData->condition;
	  }
	}
      }
      // If there is a player in the room, no repop, Modified by SinaC 2001
      if ( countpc > 0 && !reset_always ) {
	last = FALSE;
	break;
      }

      if ( pMobIndex->count >= pReset->arg2 ) {
	last = FALSE;
	break;
      }
      
      /* */
      count = 0;
      for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room) {
	if (mob->pIndexData == pMobIndex) {
	  count++;
	  if (count >= pReset->arg4) {
	    last = FALSE;
	    break;
	  }
	}
      }

      if (count >= pReset->arg4)
	break;

      /* */

      pMob = create_mobile( pMobIndex );



      /* Oxtal doesn't agree with this code ... -> commented out
	 if ( room_is_dark( pRoom ) )
	 SET_BIT(pMob->affected_by, AFF_INFRARED);

      */
            
      /*
       * Pet shop mobiles get ACT_PET set.
       */
      // Removed by SinaC 2003
      //      {
      //	ROOM_INDEX_DATA *pRoomIndexPrev;
 
      //	pRoomIndexPrev = get_room_index( pRoom->vnum - 1 );
      //	if ( pRoomIndexPrev
	     // Modified by SinaC 2001
      //	     && IS_SET( pRoomIndexPrev->cstat(flags), ROOM_PET_SHOP ) )
      //	  SET_BIT( pMob->act, ACT_PET);
      //      }

      // Added by SinaC 2000, so the wanders home test done in update.C is used
      //  Mobiles who are not in their area, not charmed, not figthing are extracted
      pMob->zone           = pRoom->area;

      char_to_room( pMob, pRoom );

      //      log_stringf("READING: [%s] [%x]", NAME(pMob), (int)pMob );
      countRepop++;

      // Added by SinaC 2001, removed by SinaC 2003
      //MOBPROG( pMob, NULL, "onRepop" );
      // Added by SinaC 2003
      if ( LastMob && LastMob != LastRepop ) {
	LastRepop = LastMob;
	//	log_stringf("DELAYED REPOP: [%s] [%x]", NAME(LastMob), (int)LastMob );
	MOBPROG( LastMob, NULL, "onRepop", 1 ); // 1 means real repop
	countDelayedRepop++;
      }

      LastMob = pMob;
      level  = URANGE( 0, pMob->level - 2, LEVEL_HERO - 1 ); /* -1 ROM */
      last = TRUE;
      break;

    case 'O':

      if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
	bug( "Reset_room: 'O' 1 : bad vnum %d", pReset->arg1 );
	sprintf (buf,"%d %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3,
		 pReset->arg4 );
	bug(buf,1);
	continue;
      }

      if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) ) {
	bug( "Reset_room: 'O' 2 : bad vnum %d.", pReset->arg3 );
	sprintf (buf,"%d %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3,
		 pReset->arg4 );
	bug(buf,1);
	continue;
      }

      if ( IS_SET( pObjIndex->extra_flags, ITEM_UNIQUE ) ) {
	bug("Reset_room: 'O'  obj: %s (%d) is unique",
	    pObjIndex->short_descr, pObjIndex->vnum );
	continue;
      }

      // Added by SinaC 2001
      countpc = 0;
      for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room) {
	if ( !IS_NPC(mob) && !IS_IMMORTAL(mob))
	  countpc++;
      }
      // If there is a player in the room, no repop, modified by SinaC 2001
      if ( countpc > 0 && !reset_always ) {
	last = FALSE;
	break;
      }

      // We don't need that, SinaC 2001
      // We really need the first test part?  SinaC 2001
      if ( /*pRoom->area->nplayer > 0
	   ||*/
	  count_obj_list( pObjIndex, pRoom->contents ) > 0 ) {
	last = FALSE;
	break;
      }

      pObj = create_object( pObjIndex,              /* UMIN - ROM OLC */
			    UMIN(number_fuzzy( level ), LEVEL_HERO -1) );

      //pObj->cost = 0;  Can't figure why we do that
      obj_to_room( pObj, pRoom );
      // Added by SinaC 2001
      recomproom(pRoom);

      // Added by SinaC 2001
      OBJPROG( pObj, NULL, "onRepop", 1 ); // 1 means real repop

      last = TRUE;
      break;

    case 'P':
      if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
	bug( "Reset_room (%d): 'P': bad arg1 vnum %d.", pRoom->vnum, pReset->arg1 );
	continue;
      }

      if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) ) {
	bug( "Reset_room (%d): 'P': bad arg3 vnum %d.", pRoom->vnum, pReset->arg3 );
	continue;
      }

      if ( IS_SET( pObjIndex->extra_flags, ITEM_UNIQUE ) ) {
	bug("Reset_room (%d): 'P'  obj: %s (%d) is unique", pRoom->vnum,
	    pObjIndex->short_descr, pObjIndex->vnum );
	continue;
      }

      // Added by SinaC 2001
      countpc = 0;
      // Ehehe, that caused a crash :))
      //for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room) {
      for (mob = pRoom->people; mob != NULL; mob = mob->next_in_room) {
	if ( !IS_NPC(mob) && !IS_IMMORTAL(mob) )
	  countpc++;
      }
      // If there is a player in the room, no repop, modified by SinaC 2001
      if ( countpc > 0 && !reset_always ) {
	last = FALSE;
	break;
      }

      if (pReset->arg2 > 50) /* old format */
	limit = 6;
      else if (pReset->arg2 == -1) /* no limit */
	limit = 999;
      else
	limit = pReset->arg2;

      // Removed by SinaC 2001, we don't need that.
      if ( /*pRoom->area->nplayer > 0
	   ||*/
	  ( LastObj = get_obj_type( pObjToIndex ) ) == NULL
	   || ( LastObj->in_room == NULL && !last)
	   || ( pObjIndex->count >= limit /* && number_range(0,4) != 0 */ )
	   || ( count = count_obj_list( pObjIndex, LastObj->contains ) ) > pReset->arg4  ) {
	last = FALSE;
	break;
      }
      /* lastObj->level  -  ROM */

      while (count < pReset->arg4) {
	pObj = create_object( pObjIndex, number_fuzzy( LastObj->level ) );
	obj_to_obj( pObj, LastObj );

	// Added by SinaC 2001
	OBJPROG( pObj, NULL, "onRepop", 1 ); // 1 means real repop

	count++;
	if (pObjIndex->count >= limit)
	  break;
      }

      /* fix object lock state! */
      LastObj->baseval[1] = LastObj->pIndexData->value[1];
      recompobj(LastObj);
      last = TRUE;
      break;

    case 'G':
    case 'E':
      if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
	bug( "Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1 );
	continue;
      }

      if ( IS_SET( pObjIndex->extra_flags, ITEM_UNIQUE ) ) {
	bug("Reset_room: 'E' or 'G'  obj: %s (%d) is unique",
	    pObjIndex->short_descr, pObjIndex->vnum);
	continue;
      }

      if ( !last )
	break;

      if ( !LastMob ) {
	bug( "Reset_room: 'E' or 'G': null mob for vnum %d.",
	     pReset->arg1 );
	last = FALSE;
	break;
      }

      if ( LastMob->pIndexData->pShop ) {  /* Shop-keeper? */
	int olevel=0,i,j;

	if (!pObjIndex->new_format)
	  switch ( pObjIndex->item_type ) {
	  default:                olevel = 0;                      break;
	  case ITEM_PILL:
	  case ITEM_POTION:
	  case ITEM_SCROLL:
	    // Added by SinaC 2003
	  case ITEM_TEMPLATE:
	    olevel = 53;
	    for (i = 1; i < 5; i++) {
	      if (pObjIndex->value[i] > 0) {
		for (j = 0; j < MAX_CLASS; j++) {
		  olevel = UMIN(olevel,
				ability_table[pObjIndex->value[i]].
				ability_level[j]);
		}
	      }
	    }

	    olevel = UMAX(0,(olevel * 3 / 4) - 2);
	    break;
		    
	  case ITEM_WAND:         olevel = number_range( 10, 20 ); break;
	  case ITEM_STAFF:        olevel = number_range( 15, 25 ); break;
	  case ITEM_ARMOR:        olevel = number_range(  5, 15 ); break;
	    /* ROM patch weapon, treasure */
	  case ITEM_WEAPON:       olevel = number_range(  5, 15 ); break;
	  case ITEM_TREASURE:     olevel = number_range( 10, 20 ); break;
	    // Added by SinaC 2000 for songs for bard
	  case ITEM_INSTRUMENT:   olevel = number_range( 1, 10 ); break;
#if 0 /* envy version */
	  case ITEM_WEAPON:       if ( pReset->command == 'G' )
	    olevel = number_range( 5, 15 );
	  else
	    olevel = number_fuzzy( level );
#endif /* envy version */

	  break;
	  }

	pObj = create_object( pObjIndex, olevel );

	SET_BIT( pObj->extra_flags, ITEM_INVENTORY );  /* ROM OLC */
	SET_BIT( pObj->base_extra, ITEM_INVENTORY );  /* ROM OLC */
	
#if 0 /* envy version */
	if ( pReset->command == 'G' )
	  SET_BIT( pObj->extra_flags, ITEM_INVENTORY );
#endif /* envy version */

      }
      else {  /* ROM OLC else version */

	// Added by SinaC 2001
       	countpc = 0;
	// Ehehe, that caused a crash :))  SinaC 2003
	//for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room) {
	for (mob = pRoom->people; mob != NULL; mob = mob->next_in_room) {
	  if ( !IS_NPC(mob) && !IS_IMMORTAL(mob) )
	    countpc++;
	}
	// If there is a player in the room, no repop, modified by SinaC 2001
	if ( countpc > 0 && !reset_always ) {
	  last = FALSE;
	  break;
	}


	int limit;
	if (pReset->arg2 > 50 )  /* old format */
	  limit = 6;
	else if ( pReset->arg2 == -1 || pReset->arg2 == 0 )  /* no limit */
	  limit = 999;
	else
	  limit = pReset->arg2;

	if ( limit == 1 ) {
	  log_stringf("RESET 'E' or 'G': limit = 1  for item %d in room %d", 
		      pObjIndex->vnum, pRoom->vnum );
	}

	if ( pObjIndex->count < limit || number_range(0,4) == 0 ) {
	  pObj = create_object( pObjIndex, 
				UMIN( number_fuzzy( level ), LEVEL_HERO - 1 ) );
	  /* error message if it is too high */
	  if (pObj->level > LastMob->level + 5
	      ||  (pObj->item_type == ITEM_WEAPON 
		   &&   pReset->command == 'E' 
		   &&   pObj->level < LastMob->level -5 && pObj->level < 45))
	    fprintf(stderr,
		    "Err: obj %s (%d) -- %d, mob %s (%d) -- %d\n",
		    pObj->short_descr,pObj->pIndexData->vnum,pObj->level,
		    LastMob->short_descr,LastMob->pIndexData->vnum,LastMob->level);
	}
	else
	  break;
      }
								 
#if 0 /* envy else version */
      else {
	pObj = create_object( pObjIndex, number_fuzzy( level ) );
      }
#endif /* envy else version */

      obj_to_char( pObj, LastMob );
      /* Previous code to wear equipement, without any integrity test
      if ( pReset->command == 'E' )
	    equip_char( LastMob, pObj, pReset->arg3 );
      */
      // Find
      if ( pReset->command == 'E' ) {
	//if ( !is_wearable( pObj->pIndexData, pReset->arg3 ) )
	  //bug("Item (%d) wrong location, mob (%d) room (%d)",
	    //  pObj->pIndexData->vnum, LastMob->pIndexData->vnum, pRoom->vnum );
	int res = equip_char( LastMob, pObj, pReset->arg3 ); // SinaC 2003, return value used
	if ( res != 0 )
	  bug("Item (%d) on mob (%d) in room (%d) has not been worn on loc (%d): res [%d]",
	      pObj->pIndexData->vnum, LastMob->pIndexData->vnum, pRoom->vnum, pReset->arg3, res );
      }

      /* Find and fix
      if ( pReset->command == 'E' ) {
	// Added by SinaC 2001
	bool bug_found = FALSE;
	if ( !is_wearable( pObj->pIndexData, pReset->arg3 ) ) {
	  bug("Item (%d) wrong location, mob (%d) room (%d) trying to FIX",
	      pObj->pIndexData->vnum, LastMob->pIndexData->vnum, pRoom->vnum );
	  pReset->arg3 = find_suitable_wearloc( pObj );
	  if ( pReset->arg3 == -99 ) { // really big problem, can't be worn
	    pReset->arg3 = WEAR_NONE;
	    bug("ITEM CAN'T BE WORN!!!!!");
	  }
	  SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	  log_stringf("Area (%s) modified, don't forget to save",
		      pRoom->area->name );
	  bug_found = TRUE;
	}
	// Added by SinaC 2001
	if ( pReset->arg3 == WEAR_NONE )
	  pReset->command = 'G';
	else
	  if ( get_eq_char( LastMob, pReset->arg3 ) == NULL )
	    equip_char( LastMob, pObj, pReset->arg3 );
	  else {
	    if ( !bug_found )
	      bug("Item (%d) location already used, mob (%d) room (%d) set it to NONE",
		  pObj->pIndexData->vnum, LastMob->pIndexData->vnum, pRoom->vnum );
	    else {
	      bug("Location already used, set it to NONE" );
	      log_stringf("Area (%s) modified, don't forget to save",
			  pRoom->area->name );
	    }
	    SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	    pReset->command = 'G';
	    pReset->arg3 == WEAR_NONE;
	  }
      }
      */
      // Added by SinaC 2001
      OBJPROG( pObj, NULL, "onRepop", 1 ); // 1 means real repop

      last = TRUE;
      break;

    case 'D':
      break;

    case 'R':
      if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) ) {
	bug( "Reset_room: 'R': bad vnum %d.", pReset->arg1 );
	continue;
      }

      {
	EXIT_DATA *pExit;
	int d0;
	int d1;

	for ( d0 = 0; d0 < pReset->arg2 - 1; d0++ ) {
	  d1                   = number_range( d0, pReset->arg2-1 );
	  pExit                = pRoomIndex->exit[d0];
	  pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
	  pRoomIndex->exit[d1] = pExit;
	}
      }
      break;
    }
  }

  if ( LastMob && LastMob != LastRepop ) {
    //    log_stringf("DELAYED REPOP: [%s] [%x]", NAME(LastMob), (int)LastMob );
    MOBPROG( LastMob, NULL, "onRepop", 1 ); // 1 means real repop
    countDelayedRepop++;
  }
  if ( countDelayedRepop != countRepop )
    bug("Invalid mob delayed repop [%d/%d]", countDelayedRepop, countRepop );

  return;
}

/* OLC
 * Reset one area.
 */
void reset_area( AREA_DATA *pArea ) {
  ROOM_INDEX_DATA *pRoom;
  DESCRIPTOR_DATA *d;
  int  vnum;
  char buf[MAX_STRING_LENGTH];

  for ( vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++ ) {
    if ( ( pRoom = get_room_index(vnum) ) ) {
      reset_room(pRoom);
      // Added by SinaC 2001
      recomproom(pRoom);
    }
  }

  // Send a message to everyone on in the area, Added by SinaC 2000
  // Removed by SinaC 2001, players didn't like that
  //sprintf(buf, "%s has just been reset.\n\r", pArea->name );
  //for (d = descriptor_list; d; d = d->next) {
  //  if (d->connected == CON_PLAYING
  //	  &&  d->character->in_room != NULL 
  //	  &&  d->character->in_room->area == pArea)
  //    send_to_char(buf,d->character);
  //}
  return;
}



// SinaC 2003, calcutate base stat value for mob in function of its level/ACT/OFF/SIZE
// Only for STR, INT, WIS, DEX and CON
/* Raised up to 30 by oxtal to follow sinac's changes*/
int mob_base_stat( const int stat, CHAR_DATA *mob ) {
  if ( stat < ATTR_STR || stat > ATTR_STR + MAX_STATS )
    return mob->baseattr[stat];

  int res = UMIN(30,11 + mob->level/4);

  if (IS_SET(mob->act,ACT_WARRIOR) && stat == ATTR_STR ) res += 3;
  if (IS_SET(mob->act,ACT_WARRIOR) && stat == ATTR_INT ) res -= 1;
  if (IS_SET(mob->act,ACT_WARRIOR) && stat == ATTR_CON ) res += 2;
  if (IS_SET(mob->act,ACT_THIEF) && stat == ATTR_DEX ) res += 3;
  if (IS_SET(mob->act,ACT_THIEF) && stat == ATTR_INT ) res += 1;
  if (IS_SET(mob->act,ACT_THIEF) && stat == ATTR_WIS ) res -= 3;
  if (IS_SET(mob->act,ACT_CLERIC) && stat == ATTR_DEX ) res -= 1;
  if (IS_SET(mob->act,ACT_CLERIC) && stat == ATTR_STR ) res += 1;
  if (IS_SET(mob->act,ACT_CLERIC) && stat == ATTR_WIS ) res += 3;
  if (IS_SET(mob->act,ACT_MAGE) && stat == ATTR_DEX ) res += 1;
  if (IS_SET(mob->act,ACT_MAGE) && stat == ATTR_INT ) res += 3;
  if (IS_SET(mob->act,ACT_MAGE) && stat == ATTR_STR ) res -= 1;
  if (IS_SET(mob->off_flags,OFF_FAST) && stat == ATTR_DEX ) res += 2;

  if ( stat == ATTR_STR ) res += mob->bstat(size) - SIZE_MEDIUM;
  if ( stat == ATTR_CON ) res += ( mob->bstat(size) - SIZE_MEDIUM ) / 2;

  return res;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile( MOB_INDEX_DATA *pMobIndex ) {
  CHAR_DATA *mob;
  int i;
  AFFECT_DATA af;

  mobile_count++;

  if ( pMobIndex == NULL ) {
    bug( "Create_mobile: NULL pMobIndex.");
    exit( 1 );
  }

  mob = new_char();

  mob->pIndexData	= pMobIndex;
  if (pMobIndex->program != NULL)
    mob->clazz = pMobIndex->program;
  else
    mob->clazz = default_mob_class;

  mob->name		= str_dup( pMobIndex->player_name );    /* OLC */
  mob->short_descr	= str_dup( pMobIndex->short_descr );    /* OLC */
  mob->long_descr	= str_dup( pMobIndex->long_descr );     /* OLC */
  mob->description	= str_dup( pMobIndex->description );    /* OLC */
  mob->id		= get_mob_id();
  mob->spec_fun	= pMobIndex->spec_fun;
  mob->prompt		= NULL;

  if (pMobIndex->wealth == 0) {
    mob->silver = 0;
    mob->gold   = 0;
  }
  else {
    long wealth;

    wealth = number_range(pMobIndex->wealth/2, 3 * pMobIndex->wealth/2);
    mob->gold = number_range(wealth/200,wealth/100);
    mob->silver = wealth - (mob->gold * 100);
  }

  if (pMobIndex->new_format) {
    /* load in new style */
    /* read from prototype */
    mob->group		= pMobIndex->group;
    mob->act 		= pMobIndex->act;
    // SinaC 2000 ;)
    //	mob->comm		= COMM_NOCHANNELS|COMM_NOSHOUT|COMM_NOTELL;
    // Added by SinaC 2000
    mob->bstat(affected_by)	= mob->cstat(affected_by)	= pMobIndex->affected_by;
    // Added by SinaC 2001
    mob->bstat(affected2_by)	= mob->cstat(affected2_by)	= pMobIndex->affected2_by;

    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000 for alignment and etho
    //mob->align.alignment	= pMobIndex->align.alignment;
    //mob->align.etho	        = pMobIndex->align.etho;
    mob->bstat(alignment)	= pMobIndex->align.alignment;
    mob->bstat(etho)	        = pMobIndex->align.etho;

    mob->level		        = pMobIndex->level;
    mob->bstat(hitroll)		= pMobIndex->hitroll;
    mob->bstat(damroll)		= pMobIndex->damage[DICE_BONUS];
    mob->bstat(max_hit)		= dice(pMobIndex->hit[DICE_NUMBER],
				       pMobIndex->hit[DICE_TYPE])
      + pMobIndex->hit[DICE_BONUS];
    mob->hit		= mob->bstat(max_hit);
    mob->bstat(max_mana)		= dice(pMobIndex->mana[DICE_NUMBER],
					       pMobIndex->mana[DICE_TYPE])
      + pMobIndex->mana[DICE_BONUS];
    mob->mana		= mob->bstat(max_mana);
    // Added by SinaC 2001 for mental user
    mob->bstat(max_psp)		= dice(pMobIndex->psp[DICE_NUMBER],
				       pMobIndex->psp[DICE_TYPE])
      + pMobIndex->psp[DICE_BONUS];
    mob->psp		= mob->bstat(max_psp);

    mob->bstat(DICE_NUMBER)= pMobIndex->damage[DICE_NUMBER];
    mob->bstat(DICE_TYPE)	= pMobIndex->damage[DICE_TYPE];
    mob->bstat(dam_type)		= pMobIndex->dam_type;
    if (mob->bstat(dam_type) == 0)
      switch(number_range(1,3)) {
      case (1): mob->bstat(dam_type) = 3;        break;  /* slash */
      case (2): mob->bstat(dam_type) = 7;        break;  /* pound */
      case (3): mob->bstat(dam_type) = 11;       break;  /* pierce */
      }
    for (i = 0; i < 4; i++)
      mob->bstat(ac0+i)	= pMobIndex->ac[i];
    mob->off_flags	= pMobIndex->off_flags;
    mob->bstat(imm_flags	)	= pMobIndex->imm_flags;
    mob->bstat(res_flags	)	= pMobIndex->res_flags;
    mob->bstat(vuln_flags)		= pMobIndex->vuln_flags;
    mob->start_pos		= pMobIndex->start_pos;
    mob->default_pos	= pMobIndex->default_pos;
    mob->bstat(sex)	= pMobIndex->sex == 3 ? number_range(1,2) : pMobIndex->sex;
    mob->bstat(race)		= pMobIndex->race;
    mob->bstat(form)		= pMobIndex->form;
    mob->bstat(parts)		= pMobIndex->parts;
    mob->bstat(size)		= pMobIndex->size;
    mob->material		= str_dup(pMobIndex->material);

    // Added by SinaC 2000 for mobile classes
    mob->bstat(classes)             = pMobIndex->classes;

    // Added by SinaC 2001 for disease
    //mob->bstat(disease)         = pMobIndex->disease;   removed by SinaC 2003

    /* computed on the spot */
    for ( i = ATTR_STR; i < ATTR_STR+MAX_STATS; i++ )
      mob->baseattr[i] = mob_base_stat( i, mob );
  }
  else {/* read in old format and convert */
    mob->act		= pMobIndex->act;
    mob->bstat(affected_by)	= pMobIndex->affected_by;
    // Added by SinaC 2001
    mob->bstat(affected2_by)	= pMobIndex->affected2_by;

    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000 for alignment and etho
    //mob->align.alignment	= pMobIndex->align.alignment;
    //mob->align.etho	        = pMobIndex->align.etho;
    mob->bstat(alignment)	= pMobIndex->align.alignment;
    mob->bstat(etho)	        = pMobIndex->align.etho;

    mob->level		        = pMobIndex->level;
    mob->bstat(hitroll)		= pMobIndex->hitroll;
    mob->bstat(damroll)		= 0;
    mob->bstat(max_hit)		= mob->level * 8 + number_range(
								mob->level * mob->level/4,
								mob->level * mob->level);
    mob->bstat(max_hit) = mob->bstat(max_hit)  * 9 / 10;
    mob->hit		= mob->bstat(max_hit);
    mob->bstat(max_mana)		= 100 + dice(mob->level,10);
    mob->mana		= mob->bstat(max_mana);
    // Added by SinaC 2001 for mental user
    mob->bstat(max_psp)		= 100 + dice(mob->level,10);
    mob->psp		= mob->bstat(max_psp);

    switch(number_range(1,3)) {
    case (1): mob->bstat(dam_type) = 3; 	break;  /* slash */
    case (2): mob->bstat(dam_type) = 7;	break;  /* pound */
    case (3): mob->bstat(dam_type) = 11;	break;  /* pierce */
    }
    for (i = 0; i < 3; i++)
      mob->bstat(ac0+i)	= interpolate(mob->level,100,-100);
    mob->bstat(ac0+3)		= interpolate(mob->level,100,0);
    mob->bstat(race)		= pMobIndex->race;
    mob->off_flags		= pMobIndex->off_flags;
    mob->bstat(imm_flags	)	= pMobIndex->imm_flags;
    mob->bstat(res_flags	)	= pMobIndex->res_flags;
    mob->bstat(vuln_flags)		= pMobIndex->vuln_flags;
    mob->start_pos		= pMobIndex->start_pos;
    mob->default_pos	= pMobIndex->default_pos;
    mob->bstat(sex)		= pMobIndex->sex;
    mob->bstat(form)		= pMobIndex->form;
    mob->bstat(parts)		= pMobIndex->parts;
    mob->bstat(size)		= SIZE_MEDIUM;
    mob->material		= "";

    // Added by SinaC 2000 for mobile classes
    mob->bstat(classes)             = 0;

    for (i = 0; i < MAX_STATS; i ++)
      mob->bstat(stat0+i) = 11 + mob->level/4;
  }

  mob->faction = pMobIndex->faction; // faction :)

  // SinaC 2003: mob_index_data may have affect, instead of keeping a bit enchanted (like object)
  //  we copy the affects but we save them only if OPTIMIZING_BIT_MODIFIED_AFFECT is set
  for ( AFFECT_DATA *paf = pMobIndex->affected; paf != NULL; paf = paf->next )
    affect_to_char( mob, paf ); // affect_to_char set OPTIMIZING_BIT_MODIFIED_AFFECT
    
  // let's get some spell action
  // SinaC 2003: new affect system
  if (IS_AFFECTED(mob,AFF_SANCTUARY)) {
    createaff(af,-1,mob->level,gsn_sanctuary,0,AFFECT_ABILITY|AFFECT_PERMANENT);
    addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
    affect_to_char(mob,&af);
    //afsetup(af,CHAR,affected_by,OR,AFF_SANCTUARY,-1,mob->level,/*ability_lookup("sanctuary")*/gsn_sanctuary);
    //affect_to_char( mob, &af );
    REMOVE_BIT( mob->bstat(affected_by), AFF_SANCTUARY );
  }
    
  if (IS_AFFECTED(mob,AFF_HASTE)){
    createaff(af,-1,mob->level,gsn_haste,0,AFFECT_ABILITY|AFFECT_PERMANENT);
    addaff(af,CHAR,affected_by,OR,AFF_HASTE);
    addaff(af,CHAR,DEX,ADD,1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 32));
    affect_to_char(mob,&af);
    //afsetup(af,CHAR,affected_by,OR,AFF_HASTE,-1,mob->level,/*ability_lookup("haste")*/gsn_haste);
    //affect_to_char( mob, &af );
    //afsetup(af,CHAR,DEX,ADD,
    //    1 + (mob->level >= 18) + (mob->level >= 25) + (mob->level >= 32),
    //    -1,mob->level,/*ability_lookup("haste")*/gsn_haste);
    //affect_to_char( mob, &af );
      
    REMOVE_BIT( mob->bstat(affected_by), AFF_HASTE );
  }
    
  if (IS_AFFECTED(mob,AFF_PROTECT_EVIL)){
    createaff(af,-1,mob->level,gsn_protection_evil,0,AFFECT_ABILITY|AFFECT_PERMANENT);
    addaff(af,CHAR,affected_by,OR,AFF_PROTECT_EVIL);
    addaff(af,CHAR,saving_throw,ADD,-1);
    affect_to_char(mob,&af);
    //afsetup(af,CHAR,affected_by,OR,AFF_PROTECT_EVIL,-1,mob->level, gsn_protection_evil);
    //affect_to_char( mob, &af );
    //afsetup(af,CHAR,saving_throw,ADD,-1,-1,mob->level, gsn_protection_evil);
    //affect_to_char( mob, &af );
      
    REMOVE_BIT( mob->bstat(affected_by), AFF_PROTECT_EVIL );
  }
    
  if (IS_AFFECTED(mob,AFF_PROTECT_GOOD)){
    createaff(af,-1,mob->level,gsn_protection_good,0,AFFECT_ABILITY|AFFECT_PERMANENT);
    addaff(af,CHAR,affected_by,OR,AFF_PROTECT_GOOD);
    addaff(af,CHAR,saving_throw,ADD,-1);
    affect_to_char(mob,&af);
    //afsetup(af,CHAR,affected_by,OR,AFF_PROTECT_GOOD,-1,mob->level, gsn_protection_good );
    //affect_to_char( mob, &af );
    //afsetup(af,CHAR,saving_throw,ADD,-1,-1,mob->level, gsn_protection_good );
    //affect_to_char( mob, &af );
      
    REMOVE_BIT( mob->bstat(affected_by), AFF_PROTECT_GOOD );
  }

  // affect_to_char set OPTIMIZING_BIT_MODIFIED_AFFECT to true
  REMOVE_BIT( mob->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT ); // affect are not modified
    
  mob->position = mob->start_pos;

  // Needed because onCreate could use cstat for example repairShop: use getAbility which use ch->cstat(race)
  recompute(mob);

  /* link the mob to the world list */
  mob->next		= char_list;
  char_list		= mob;
  pMobIndex->count++;

  // Added by SinaC 2001
  MOBPROG( mob, NULL, "onCreate" );

  return mob;
}

// Added by SinaC 2003, to create a mobile from a corpse
CHAR_DATA *create_mobile_from_corpse( OBJ_DATA *corpse, MOB_INDEX_DATA *pMobIndex ) {
  CHAR_DATA *mob = create_mobile( pMobIndex );
  if ( corpse->item_type != ITEM_CORPSE_NPC
       && corpse->item_type != ITEM_CORPSE_PC ) {
    bug("create_mobile_from_corpse: invalid corpse [%d] type [%d]",
	corpse->pIndexData->vnum, corpse->item_type);
    return mob;
  }
  SET_BIT( mob->bstat(form), corpse->value[1] );
  SET_BIT( mob->bstat(parts), corpse->value[2] );
  mob->cstat(form) = mob->bstat(form);
  mob->cstat(parts) = mob->bstat(parts);
  //-> no need of recompute
  return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mobile(CHAR_DATA *parent, CHAR_DATA *clone) {
  int i;

  if ( parent == NULL 
       || clone == NULL 
       || !IS_NPC(parent))
    return;

  /* start fixing values */

  for (i = 0; i < ATTR_NUMBER; i++)
    clone->baseattr[i] = parent->baseattr[i];

  clone->clazz          = parent->clazz;
  clone->name 	        = str_dup(parent->name);
  clone->version	= parent->version;
  clone->short_descr	= str_dup(parent->short_descr);
  clone->long_descr	= str_dup(parent->long_descr);
  clone->description	= str_dup(parent->description);
  clone->group	        = parent->group;
  clone->level	        = parent->level;
  clone->trust	        = 0;
  clone->timer	        = parent->timer;
  clone->wait		= parent->wait;
  clone->hit		= parent->hit;
  clone->mana		= parent->mana;
  // Added by SinaC 2001 for mental user
  clone->psp		= parent->psp;

  clone->move		= parent->move;
  clone->gold		= parent->gold;
  clone->silver	        = parent->silver;
  clone->exp		= parent->exp;
  clone->act		= parent->act;
  clone->comm		= parent->comm;
  clone->invis_level	= parent->invis_level;
  clone->position	= parent->position;
  clone->practice	= parent->practice;
  clone->train	        = parent->train;

  clone->wimpy	        = parent->wimpy;
  clone->material	= str_dup(parent->material);
  clone->off_flags	= parent->off_flags;
  clone->start_pos	= parent->start_pos;
  clone->default_pos	= parent->default_pos;
  clone->spec_fun	= parent->spec_fun;

  clone->faction        = parent->faction;

  // Modified by SinaC 2001   before we removed every affect
  AFFECT_DATA *paf_next;
  for (AFFECT_DATA *paf = clone->affected; paf != NULL; paf = paf_next) {
    paf_next = paf->next;
    affect_remove(clone,paf);
  }
  
  /* now add the affects */
  for (AFFECT_DATA *paf = parent->affected; paf != NULL; paf = paf->next)
    affect_to_char(clone,paf);

  //  recompute(clone);  NO NEED TO RECOMPUTE because create_mobile is always followed by a char_to_room
  //    and affect_to_char performs a recompute
}




/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object( OBJ_INDEX_DATA *pObjIndex, int level )
{
  OBJ_DATA *obj;
  int i;

  if ( pObjIndex == NULL )
    {
      bug( "Create_object: NULL pObjIndex.");
      exit( 1 );
    }

  obj = new_obj();

  obj->pIndexData	= pObjIndex;
  obj->in_room	= NULL;
  obj->enchanted	= FALSE;

  if (pObjIndex->program != NULL)
    obj->clazz = pObjIndex->program;
  else
    obj->clazz = default_obj_class;


  if (pObjIndex->new_format)
    obj->level = pObjIndex->level;
  else
    obj->level		= UMAX(0,level);
  obj->wear_loc	= -1;

  obj->name		= str_dup( pObjIndex->name );           /* OLC */
  obj->short_descr	= str_dup( pObjIndex->short_descr );    /* OLC */
  obj->description	= str_dup( pObjIndex->description );    /* OLC */
  // Modified by SinaC 2001
  //obj->material	        = str_dup(pObjIndex->material);
  obj->material	        = pObjIndex->material;

  obj->item_type	= pObjIndex->item_type;
  obj->base_extra       = pObjIndex->extra_flags;
  obj->wear_flags	= pObjIndex->wear_flags;
  obj->baseval[0]       = pObjIndex->value[0];
  obj->baseval[1]       = pObjIndex->value[1];
  obj->baseval[2]       = pObjIndex->value[2];
  obj->baseval[3]       = pObjIndex->value[3];
  obj->baseval[4]       = pObjIndex->value[4];
  obj->weight		= pObjIndex->weight;

  // What about the condition ???  SinaC 2000
  obj->condition      = pObjIndex->condition;

  // Removed by SinaC 2003, not needed: restriction/ability_upgrade cannot be modified during play
  //  so we can safely used pIndexData
  // Added by SinaC 2000 for object restrictions, I'M really stupid
  //obj->restriction = NULL;//pObjIndex->restriction;
  //  obj->restriction = NULL; // enchanted flag is used in the same way for restriction
  // Added by SinaC 2000 for object upgrading skill/spell, I'M really stupid
  // Removed by SinaC 2003, copied only if enchanted
  //  obj->upgrade = NULL;//pObjIndex->upgrade;

  // Added by SinaC 2003, size is not anymore a restriction but an obj stat
  obj->size = pObjIndex->size;

  if (level == -1 || pObjIndex->new_format)
    obj->cost	= pObjIndex->cost;
  else
    obj->cost	= number_fuzzy( 10 )
      * number_fuzzy( level ) * number_fuzzy( level );

  /*
   * Mess with object properties.
   */
  switch ( obj->item_type ) {
  default:
    bug( "Read_object: vnum %d bad type.", pObjIndex->vnum );
    break;

  case ITEM_LIGHT:
    if (obj->baseval[2] == 999)
      obj->baseval[2] = -1;
    break;

  case ITEM_FURNITURE:
  case ITEM_TRASH:
  case ITEM_CONTAINER:
  case ITEM_DRINK_CON:
  case ITEM_KEY:
  case ITEM_FOOD:
  case ITEM_BOAT:
  case ITEM_CORPSE_NPC:
  case ITEM_CORPSE_PC:
  case ITEM_FOUNTAIN:
  case ITEM_MAP:
  case ITEM_CLOTHING:
  case ITEM_PORTAL:
    if (!pObjIndex->new_format)
      obj->cost /= 5;
    break;

  case ITEM_TREASURE:
    // SinaC 2000 : have replaced ITEM_PROTECT with ITEM_THROWING
    //    case ITEM_PROTECT:
    //case ITEM_THROWING:  removed by SinaC 2003
  case ITEM_WARP_STONE:
    //    case ITEM_ROOM_KEY:
    // have replaced ITEM_ROOM_KEY with ITEM_COMPONENT  SinaC 2000
  case ITEM_COMPONENT:

  case ITEM_GEM:
  case ITEM_JEWELRY:
    // Added by SinaC 2000 for songs for bard
  case ITEM_INSTRUMENT:
        /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
  case ITEM_GRENADE:
	*/
    // Added by SinaC 2000 for windows
  case ITEM_WINDOW:
    // Added by SinaC 2000 for Tartarus's spell animate_skeleton
    //  case ITEM_SKELETON:
  // Added by SinaC 2001 for levers
  //case ITEM_LEVER:        Removed by SinaC 2003
    // Added by SinaC 2003
  case ITEM_SADDLE:
  case ITEM_ROPE:
    break;

    /* Removed by SinaC 2003    
       case ITEM_JUKEBOX:
       for (i = 0; i < 5; i++)
       obj->baseval[i] = -1;
       break;
    */

  case ITEM_SCROLL:
    // Added by SinaC 2003
  case ITEM_TEMPLATE:
    if (level != -1 && !pObjIndex->new_format)
      obj->baseval[0]       = number_fuzzy( obj->baseval[0] );
    break;

  case ITEM_WAND:
  case ITEM_STAFF:
    if (level != -1 && !pObjIndex->new_format) {
      obj->baseval[0]       = number_fuzzy( obj->baseval[0] );
      obj->baseval[1]       = number_fuzzy( obj->baseval[1] );
      obj->baseval[2]       = obj->value[1];
    }
    if (!pObjIndex->new_format)
      obj->cost *= 2;
    break;

  case ITEM_WEAPON:
    if (level != -1 && !pObjIndex->new_format && obj->baseval[0] != WEAPON_RANGED ) {
      obj->baseval[1] = number_fuzzy( number_fuzzy( 1 * level / 4 + 2 ) );
      obj->baseval[2] = number_fuzzy( number_fuzzy( 3 * level / 4 + 6 ) );
    }
    break;

  case ITEM_ARMOR:
    if (level != -1 && !pObjIndex->new_format) {
      obj->baseval[0]       = number_fuzzy( level / 5 + 3 );
      obj->baseval[1]       = number_fuzzy( level / 5 + 3 );
      obj->baseval[2]       = number_fuzzy( level / 5 + 3 );
    }
    break;

  case ITEM_POTION:
  case ITEM_PILL:
    if (level != -1 && !pObjIndex->new_format)
      obj->baseval[0] = number_fuzzy( number_fuzzy( obj->value[0] ) );
    break;

  case ITEM_MONEY:
    if (!pObjIndex->new_format)
      obj->baseval[0]       = obj->cost;
    break;
  }
  /* Removed by Oxtal
     for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) 
     if ( paf->location == APPLY_SPELL_AFFECT )
     affect_to_obj(obj,paf); */

  recompobj(obj);

  obj->next		= object_list;
  object_list		= obj;
  pObjIndex->count++;

  // Added by SinaC 2001
  OBJPROG( obj, NULL, "onCreate" );

  return obj;
}

/* duplicate an object exactly -- except contents */
void clone_object(OBJ_DATA *parent, OBJ_DATA *clone)
{
  int i;
  AFFECT_DATA *paf;
  EXTRA_DESCR_DATA *ed,*ed_new;

  if (parent == NULL || clone == NULL 
      || IS_SET(parent->extra_flags, ITEM_UNIQUE ) ) // Added by SinaC 2003
    return;

  /* start fixing the object */
  clone->clazz = parent->clazz;
  clone->name 	= str_dup(parent->name);
  clone->short_descr 	= str_dup(parent->short_descr);
  clone->description	= str_dup(parent->description);
  clone->item_type	= parent->item_type;
  clone->base_extra   = parent->base_extra;
  clone->wear_flags	= parent->wear_flags;
  clone->weight	= parent->weight;
  clone->cost		= parent->cost;
  clone->level	= parent->level;
  clone->condition	= parent->condition;
  // Modified by SinaC 2001
  //clone->material	= str_dup(parent->material);
  clone->material	= parent->material;
  // Added by SinaC 2003
  clone->size           = parent->size;

  clone->timer	= parent->timer;

  for (i = 0;  i < 5; i ++)
    clone->baseval[i] = parent->baseval[i];

  /* affects */
  clone->enchanted	= parent->enchanted;
  
  for (paf = parent->affected; paf != NULL; paf = paf->next) 
    affect_to_obj(clone,paf);

  // SinaC 2003, not needed: we can safely use pIndexData because restriction/ability_upgrade
  //  cannot be dynamically modified during play
//  // Added by SinaC 2001 for restrictions
//  for (RESTR_DATA *restr = parent->restriction; restr != NULL; restr = restr->next) {
//    RESTR_DATA *restr_new;
//    
//    restr_new = new_restriction();
//    
//    *restr_new		= *restr;
//    
//    restr_new->next	= clone->restriction;
//    clone->restriction	= restr_new;
//  }
//
//  for (ABILITY_UPGRADE *upgr = parent->upgrade; upgr != NULL; upgr = upgr->next) {
//    ABILITY_UPGRADE *upgr_new;
//    
//    upgr_new = new_restriction();
//    
//    *upgr_new		= *upgr;
//    
//    upgr_new->next	= clone->upgrade;
//    clone->upgrade	= upgr_new;
//  }

  /* extended desc */
  for (ed = parent->extra_descr; ed != NULL; ed = ed->next) {
    ed_new                  = new_extra_descr();
    ed_new->keyword    	    = str_dup( ed->keyword);
    ed_new->description     = str_dup( ed->description );
    ed_new->next            = clone->extra_descr;
    clone->extra_descr      = ed_new;
  }

  // Added by SinaC 2000
  recompobj( clone );

}

/*
 * Get an extra description from a list.
 */
const char *get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed ) {
  for ( ; ed != NULL; ed = ed->next ) {
    if ( is_name( (char *) name, ed->keyword ) )
      return ed->description;
  }
  return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index( int vnum ) {
  MOB_INDEX_DATA *pMobIndex;

  for ( pMobIndex  = mob_index_hash[vnum % MAX_KEY_HASH];
	pMobIndex != NULL;
	pMobIndex  = pMobIndex->next ) {
    if ( pMobIndex->vnum == vnum )
      return pMobIndex;
  }

  if ( fBootDb ) {
    bug( "Get_mob_index: bad vnum %d.", vnum );
    exit( 1 );
  }

  return NULL;
}



/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index( int vnum ) {
  OBJ_INDEX_DATA *pObjIndex;

  for ( pObjIndex  = obj_index_hash[vnum % MAX_KEY_HASH];
	pObjIndex != NULL;
	pObjIndex  = pObjIndex->next ) {
    if ( pObjIndex->vnum == vnum )
      return pObjIndex;
  }

  if ( fBootDb ) {
    log_stringf("Can't hash obj #%d (msg just in case bug fails)",vnum); 
    bug( "Get_obj_index: bad vnum %d.", vnum );
    exit( 1 );
  }
  return NULL;
}



/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index( int vnum )
{
  ROOM_INDEX_DATA *pRoomIndex;

  for ( pRoomIndex  = room_index_hash[vnum % MAX_KEY_HASH];
	pRoomIndex != NULL;
	pRoomIndex  = pRoomIndex->next ) {
    if ( pRoomIndex->vnum == vnum )
      return pRoomIndex;
  }

  if ( fBootDb ) {

    bug( "Get_room_index: bad vnum %d.", vnum );
    return get_room_index(1); // new phylosophy, allows to load linked to unknown room

    exit( 1 );
  }

  return NULL;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp( const char *astr, const char *bstr ) {
  if ( astr == NULL ) {
    bug( "Str_cmp: null astr.");
    return TRUE;
  }

  if ( bstr == NULL ) {
    bug( "Str_cmp: null bstr." );
    return TRUE;
  }

  for ( ; *astr || *bstr; astr++, bstr++ ) {
    if ( LOWER(*astr) != LOWER(*bstr) )
      return TRUE;
  }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix( const char *astr, const char *bstr ) {
  if ( astr == NULL ) {
    bug( "Strn_cmp: null astr.");
    return TRUE;
  }

  if ( bstr == NULL ) {
    bug( "Strn_cmp: null bstr." );
    return TRUE;
  }

  for ( ; *astr; astr++, bstr++ ) {
    if ( LOWER(*astr) != LOWER(*bstr) )
      return TRUE;
  }

  return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix( const char *astr, const char *bstr ) {
  int sstr1;
  int sstr2;
  int ichar;
  char c0;

  if ( ( c0 = LOWER(astr[0]) ) == '\0' )
    return FALSE;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);

  for ( ichar = 0; ichar <= sstr2 - sstr1; ichar++ ) {
    if ( c0 == LOWER(bstr[ichar]) && !str_prefix( astr, bstr + ichar ) )
      return FALSE;
  }

  return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix( const char *astr, const char *bstr ) {
  int sstr1;
  int sstr2;

  sstr1 = strlen(astr);
  sstr2 = strlen(bstr);
  if ( sstr1 <= sstr2 && !str_cmp( astr, bstr + sstr2 - sstr1 ) )
    return FALSE;
  else
    return TRUE;
}

// Added by SinaC 2001
void skip_till_0( FILE *fp ) {
  for ( ; ; ) {
    int vnum = fread_number(fp);
    if ( vnum == 0 )
      break;
  }
  return;
}

void skip_till_( FILE *fp, char c ) {
  for ( ; ; ) {
    char letter = fread_letter(fp);
    if (letter == c )
      break;
    fread_to_eol(fp);
  }
}

void skip_till_cardinal0( FILE *fp ) {
  for ( ; ; ) {

    char letter				= fread_letter( fp );
    if ( letter != '#' ) {
      bug( "skip_till_cardinal0: # not found.");
      exit( 1 );
    }

    int vnum				= fread_number( fp );
    if ( vnum == 0 )
      break;
    
    skip_till_( fp, '~' );
  }
  return;
}


/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
   back to the system srandom call.  If this doesn't work for you, 
   define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif
 
void init_mm( )
{
#if defined (OLD_RAND)
  int *piState;
  int iState;
 
  piState     = &rgiState[2];
 
  piState[-2] = 55 - 55;
  piState[-1] = 55 - 24;
 
  piState[0]  = ((int) current_time) & ((1 << 30) - 1);
  piState[1]  = 1;
  for ( iState = 2; iState < 55; iState++ ) {
    piState[iState] = (piState[iState-1] + piState[iState-2])
      & ((1 << 30) - 1);
  }
#else
  srandom(time(NULL)^getpid());
#endif
  return;
}

long number_mm( void ) {
#if defined (OLD_RAND)
  int *piState;
  int iState1;
  int iState2;
  int iRand;
 
  piState             = &rgiState[2];
  iState1             = piState[-2];
  iState2             = piState[-1];
  iRand               = (piState[iState1] + piState[iState2])
    & ((1 << 30) - 1);
  piState[iState1]    = iRand;
  if ( ++iState1 == 55 )
    iState1 = 0;
  if ( ++iState2 == 55 )
    iState2 = 0;
  piState[-2]         = iState1;
  piState[-1]         = iState2;
  return iRand >> 6;
#else
  return random() >> 6;
#endif
}



/*
 * Read a number from a file.
 */
long fread_number( FILE *fp ) {
  long number;
  bool sign;
  char c;

  do {
    c = getc( fp );
  } while ( isspace(c) );

  number = 0;

  sign   = FALSE;
  if ( c == '+' ) {
    c = getc( fp );
  } 
  else if ( c == '-' ) {
    sign = TRUE;
    c = getc( fp );
  }

  if ( !isdigit(c) ) {
    bug( "Fread_number: bad format.");
    exit( 1 );
  }

  while ( isdigit(c) ) {
    number = number * 10 + c - '0';
    c      = getc( fp );
  }

  if ( sign )
    number = 0 - number;

  if ( c == '|' )
    number += fread_number( fp );
  else if ( c != ' ' )
    ungetc( c, fp );

  return number;
}

long flag_to_long( const char *s ) {
  long number;
  const char *p = s;
  bool negative = FALSE;

  while( isspace(*p) ) {
    p++;
  }
    
  /* By Oxtal for MZF compatibility */
  if (*p == '\'') {
    p++;
    if (*p != '\'')
      bug("Wrong flag format. (Double quote expected)");
    return 0;
  }

  if (*p == '-') {
    negative = TRUE;
    p++;
  }

  number = 0;       

  if (!isdigit(*p))
    while (('A' <= *p && *p <= 'Z') || ('a' <= *p && *p <= 'z')) {
      number += flag_convert(*p);
      p++;
    }

  while (isdigit(*p)) {
    number = number * 10 + (*p) - '0';
    p++;
  }

  if (*p == '|')
    number += flag_to_long(p);

  if (negative)
    return -1 * number;

  return number;
}

long fread_flag( FILE *fp) {
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
    while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
      number += flag_convert(c);
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

long flag_convert(char letter ) {
  long bitsum = 0;
  char i;

  if ('A' <= letter && letter <= 'Z') {
    bitsum = 1;
    for (i = letter; i > 'A'; i--)
      bitsum *= 2;
  }
  else if ('a' <= letter && letter <= 'z') {
    bitsum = 67108864; /* 2^26 */
    for (i = letter; i > 'a'; i --)
      bitsum *= 2;
  }
  /* Oxtal suggests (1 << 26) << (letter - 'a') */

  // Added by SinaC 2001 to avoid negative flag
  if ( bitsum < 0 ) {
    bug("Negative bit!!!!");
    return 0;
  }
  return bitsum;
}

/*
 * Allocate space for a string from a file.
 * These strings are read-only and shared.
 * Strings are hashed for better sharing. See string_space.
 */
static const char* boot_alloc_string(char* buf) {
  if ( fBootDb )
    return alloc_perm_string(buf);
  else
    return clever_str_dup(buf);
}

// Read a string from a file
char *fread_string_noalloc( FILE *fp ) {

  static char buf[MAX_STRING_LENGTH];
  char *plast = buf;
  char c;

  /*
   * Skip blanks.
   * Read first char.
   */
  do {
    c = getc( fp );
  } while ( isspace(c) );

  if ( ( *plast++ = c ) == '~' )
    return &str_empty[0];

  for ( ; ; ) {
    /*
     * Back off the char type lookup,
     *   it was too dirty for portability.
     *   -- Furey
     */

    switch ( *plast = getc(fp) ) {
    default:
      plast++;
      break;
 
    case EOF:
      /* temp fix */
      bug( "Fread_string: EOF");
      return NULL;
      /* exit( 1 ); */
      break;
 
    case '\n':
      plast++;
      *plast++ = '\r';
      break;
 
    case '\r':
      break;
 
    case '~':
      *plast = '\0'; // smash tilde :)
      return buf;
    }
  }
}

const char* fread_string( FILE *fp ) {
  return boot_alloc_string( fread_string_noalloc( fp ) );
}

const char *fread_string_upper( FILE *fp ) {
  char *s = fread_string_noalloc(fp);
  s[0] = UPPER(s[0]);
  return boot_alloc_string(s);
}

const char *fread_string_lower( FILE *fp ) {
  char *s = fread_string_noalloc(fp);
  s[0] = LOWER(s[0]);
  return boot_alloc_string(s);
}


// Read a string from a file, till EOL
char *fread_string_eol_noalloc( FILE *fp )
{
  static bool char_special[256-EOF];
  static char buf[MAX_STRING_LENGTH];
  char *plast = buf;
  char c;
 
  if ( char_special[EOF-EOF] != TRUE ) {
    char_special[EOF -  EOF] = TRUE;
    char_special['\n' - EOF] = TRUE;
    char_special['\r' - EOF] = TRUE;
  }
 
 
  /*
   * Skip blanks.
   * Read first char.
   */
  do {
    c = getc( fp );
  } while ( isspace(c) );
 
  if ( ( *plast++ = c ) == '\n')
    return &str_empty[0];
 
  for ( ;; ) {
    if ( !char_special[ ( *plast++ = getc( fp ) ) - EOF ] )
      continue;
 
    switch ( plast[-1] ) {
    default:
      break;
      
    case EOF:
      bug( "Fread_string_eol  EOF");
      exit( 1 );
      break;
      
    case '\n':  case '\r':
      plast[-1] = '\0';
      return buf;
    }
  }
}

const char* fread_string_eol( FILE *fp ) {
  return boot_alloc_string( fread_string_eol_noalloc( fp ) );
}


const char * fread_string_eol_trim ( FILE *fp ) {
  char *s = fread_string_eol_noalloc(fp);

  char* t=s+strlen(s)-1;

  while (t>=s && isspace(*t))
    t--;
  t++;

  *t = '\0';

  return boot_alloc_string(s);
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line( FILE *fp ) {
  static char line[MAX_STRING_LENGTH];
  char *pline;
  char c;
  int ln;
  
  pline = line;
  line[0] = '\0';
  ln = 0;
  
  /*
   * Skip blanks.
   * Read first char.
   */
  do {
    if ( feof(fp) ) {
      bug("fread_line: EOF encountered on read.\n\r");
      if ( fBootDb )
	exit(1);
      strcpy(line, "");
      return line;
    }
    c = getc( fp );
  }
  while ( isspace(c) );
  
  ungetc( c, fp );
  do {
    if ( feof(fp) ) {
      bug("fread_line: EOF encountered on read.\n\r");
      if ( fBootDb )
	exit(1);
      *pline = '\0';
      return line;
    }
    c = getc( fp );
    *pline++ = c; ln++;
    if ( ln >= (MAX_STRING_LENGTH - 1) ) {
      bug( "fread_line: line too long" );
      break;
    }
  }
  while ( c != '\n' && c != '\r' );
  
  do {
    c = getc( fp );
  }
  while ( c == '\n' || c == '\r' );
  
  ungetc( c, fp );
  *pline = '\0';
  return line;
}


/*
 * Read to end of line (for comments).
 */
void fread_to_eol( FILE *fp ) {
  char c;
  
  do {
    c = getc( fp );
  } while ( c != '\n' && c != '\r' );
  
  do {
    c = getc( fp );
  } while ( c == '\n' || c == '\r' );
  
  ungetc( c, fp );
  return;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word( FILE *fp ) {
  static char word[MAX_INPUT_LENGTH];
  char *pword;
  char cEnd;

  word[0] = '\0';

  do {
    cEnd = getc( fp );
  } while ( isspace( cEnd ) );

  if ( cEnd == '\'' || cEnd == '"' ) {
    pword   = word;
  }
  else {
    word[0] = cEnd;
    pword   = word+1;
    cEnd    = ' ';
  }

  for ( ; pword < word + MAX_INPUT_LENGTH; pword++ ) {
    *pword = getc( fp );
    if ( cEnd == ' ' ? isspace(*pword) : *pword == cEnd ) {
      if ( cEnd == ' ' )
	ungetc( *pword, fp );
      *pword = '\0';
      return word;
    }
  }

  bug( "Fread_word: word too long.");
  exit( 1 );
  return NULL;
}

/* Modified by Sinac 1997 */
void do_areas( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  AREA_DATA *pArea1;
  AREA_DATA *pArea2;
  int iArea;
  int iAreaHalf;
  // Added by SinaC 2001
  int mlev = 0,
    Mlev = 150;
  char arg1[MAX_INPUT_LENGTH],
    arg2[MAX_INPUT_LENGTH];

  /*
  if (argument[0] != '\0') {
    send_to_char_bw("No argument is used with this command.\n\r",ch);
    return;
  }
  iAreaHalf = (top_area + 1) / 2;
  pArea1    = area_first;
  pArea2    = area_first;
  for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    pArea2 = pArea2->next;
  buffer = new_buf();
  for ( iArea = 0; iArea < iAreaHalf; iArea++ ) {
    sprintf( buf, "%-39s%-39s\n\r",
	     pArea1->credits, (pArea2 != NULL) ? pArea2->credits : "" );
    //send_to_char_bw( buf, ch );
    add_buf( buffer, buf );
    pArea1 = pArea1->next;
    if ( pArea2 != NULL )
      pArea2 = pArea2->next;
  }
  page_to_char( buf_string( buffer ), ch );
  free_buf( buffer );
  */
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] != '\0' ) {
    if ( !is_number( arg1 ) ) {
      send_to_char("area [<min level>] [<max level>]\n\r",ch);
      return;
    }
    if ( arg2[0] == '\0' )
      Mlev = atoi(arg1);
    else
      mlev = atoi( arg1 );
  }

  if ( arg2[0] != '\0' ) {
    if ( !is_number( arg2 ) ) {
      send_to_char("area [<min level>] [<max level>]\n\r",ch);
      return;
    }
    Mlev = atoi( arg2 );
  }

  if ( mlev > Mlev ) {
    send_to_char("area [<min level>] [<max level>]\n\r",ch);
    return;
  }

  buffer = new_buf();

  int col = 0;
  for ( pArea1 = area_first; pArea1; pArea1 = pArea1->next )
    if ( mlev <= pArea1->low_range 
	 && Mlev >= pArea1->high_range ) {
      sprintf(buf,
	      "%-39s",
	      pArea1->credits );
      if (++col % 2 == 0)
	strcat(buf,"\n\r");

      add_buf( buffer, buf );
    }
  if ( col % 3 != 0 )
    add_buf( buffer, "\n\r" );
  page_to_char( buf_string( buffer ), ch );
  return;
}


void do_memory( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  send_to_char("Oldstyle:\n\r",ch);
  send_to_charf( ch, "Mobs    %5d(%d new format)\n\r", top_mob_index,newmobs ); 
  send_to_charf( ch, "Objs    %5d(%d new format)\n\r", top_obj_index,newobjs ); 

  // Added by SinaC 2003
  int count_skill = 0,
    count_spell = 0,
    count_power = 0,
    count_song = 0,
    count_affect = 0;
  for (int sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;
    // Modified by SinaC 2003
    switch(ability_table[sn].type) {
    case TYPE_SKILL: count_skill++; break;
    case TYPE_SPELL: count_spell++; break;
    case TYPE_POWER: count_power++; break;
    case TYPE_SONG: count_song++; break;
    case TYPE_AFFECT: count_affect++; break;
    }
  }
  send_to_char("Memory used:\n\r", ch );
  long total = 0;
  send_to_charf( ch, "Affects     %5d    mem %7d\n\r", top_affect,
		 top_affect * sizeof( AFFECT_DATA) );
  total += top_affect * sizeof( AFFECT_DATA );
  send_to_charf( ch, "Areas       %5d    mem %7d\n\r", top_area,
		 top_area * sizeof( AREA_DATA ));
  total += top_area * sizeof( AREA_DATA );
  send_to_charf( ch, "ExDes       %5d    mem %7d\n\r", top_ed,
		 top_ed * sizeof( EXTRA_DESCR_DATA ) );
  total += top_ed * sizeof( EXTRA_DESCR_DATA );
  send_to_charf( ch, "Exits       %5d    mem %7d\n\r", top_exit,
		 top_exit * sizeof( EXIT_DATA ) );
  total += top_exit * sizeof( EXIT_DATA );
  send_to_charf( ch, "Helps       %5d    mem %7d\n\r", top_help,
		 top_help * sizeof( HELP_DATA ) );
  total += top_help * sizeof( HELP_DATA );
  send_to_charf( ch, "Socials     %5d    mem %7d\n\r", social_count,
		 social_count * sizeof(struct social_type ) );
  total += social_count * sizeof(struct social_type );
  send_to_charf( ch, "Spells      %5d    mem %7d\n\r", count_spell,
		 count_spell * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_spell * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  send_to_charf( ch, "Skills      %5d    mem %7d\n\r", count_skill,
		 count_skill * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_skill * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  send_to_charf( ch, "Powers      %5d    mem %7d\n\r", count_power,
		 count_power * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_power * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  send_to_charf( ch, "Songs       %5d    mem %7d\n\r", count_song,
		 count_song * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_song * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  send_to_charf( ch, "SpecialAff  %5d    mem %7d\n\r", count_affect,
		 count_affect * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_affect * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  send_to_charf( ch, "Mobs        %5d    mem %7d\n\r", top_mob_index,
		 top_mob_index * sizeof( MOB_INDEX_DATA ) );
  total += top_mob_index * sizeof( MOB_INDEX_DATA );
  send_to_charf( ch, "(in use)    %5d    mem %7d\n\r", mobile_count,
		 mobile_count * sizeof( CHAR_DATA ) );
  total += mobile_count * sizeof( CHAR_DATA );
  send_to_charf( ch, "Objs        %5d    mem %7d\n\r", top_obj_index,
		 top_obj_index * sizeof( OBJ_INDEX_DATA ) );
  total += top_obj_index * sizeof( OBJ_INDEX_DATA );
  send_to_charf( ch, "(in use)    %5d    mem %7d\n\r", object_count,
		 object_count * sizeof( OBJ_DATA ) );
  total += object_count * sizeof( OBJ_DATA );
  send_to_charf( ch, "Resets      %5d    mem %7d\n\r", top_reset,
		 top_reset * sizeof( RESET_DATA ) );
  total += top_reset * sizeof( RESET_DATA );
  send_to_charf( ch, "Rooms       %5d    mem %7d\n\r", top_room,
		 top_room * sizeof( ROOM_INDEX_DATA ) );
  total += top_room * sizeof( ROOM_INDEX_DATA );
  send_to_charf( ch, "Shops       %5d    mem %7d\n\r", top_shop,
		 top_shop * sizeof( SHOP_DATA ) );
  total += top_shop * sizeof( SHOP_DATA );
  send_to_charf( ch, "Commands    %5d    mem %7d\n\r", MAX_COMMANDS,
		 MAX_COMMANDS * sizeof( cmd_type ));
  total += MAX_COMMANDS * sizeof( cmd_type );
  send_to_charf( ch, "Classes     %5d    mem %7d\n\r", MAX_CLASS,
		 MAX_CLASS * sizeof( class_type ));
  total += MAX_CLASS * sizeof(class_type);
  send_to_charf( ch, "Groups      %5d    mem %7d\n\r", MAX_GROUP,
		 MAX_GROUP * (sizeof(group_type)+MAX_CLASS*sizeof(int)+MAX_GODS*sizeof(int)));
  total += MAX_GROUP * (sizeof(group_type)+MAX_CLASS*sizeof(int)+MAX_GODS*sizeof(int));
  send_to_charf( ch, "Gods        %5d    mem %7d\n\r", MAX_GODS,
		 MAX_GODS * sizeof(god_data));
  total += MAX_GODS * sizeof(god_data); // wrong approx
  send_to_charf( ch, "Races       %5d    mem %7d\n\r", MAX_RACE,
		 MAX_RACE * sizeof(race_type) + MAX_PC_RACE * sizeof(pc_race_type));
  total += MAX_RACE * sizeof(race_type) + MAX_PC_RACE * sizeof(pc_race_type); // wrong approx

  send_to_charf( ch, "Total                mem %7ld\n\r\n\r", total );


  send_to_charf(ch,
		"Garbage Collector:\n\r"
		"-----------------\n\r");
  send_to_charf(ch,"Heap size      : %d\n\r", GC_get_heap_size());
  send_to_charf(ch,"Free bytes     : %d\n\r", GC_get_free_bytes());
  send_to_charf(ch,"Bytes allocated: %d\n\r", GC_get_bytes_since_gc());
  send_to_charf(ch,"(Uncollectable): %d [upper bound]\n\r", 
		GC_get_heap_size() - GC_get_free_bytes() - GC_get_bytes_since_gc());


  send_to_charf(ch,"\n\r");

  send_to_charf(ch,
		"String Space:\n\r"
		"-------------\n\r");
  //  send_to_charf(ch,"Free bytes     : %d\n\r", get_str_free_space());
  send_to_charf(ch,"Total used     : %d\n\r", get_str_space_size() );
  send_to_charf(ch,"Free entries   : %d\n\r", get_str_free_entries());
  send_to_charf(ch,"nStrDup         : %d\n\r", nStrDup );
  send_to_charf(ch,"nStrDupEmpty    : %d\n\r", nStrDupEmpty );
  send_to_charf(ch,"nStrDupGC       : %d\n\r", nStrDupGC );
  send_to_charf(ch,"nStrDupNoGC     : %d\n\r", nStrDupNoGC );
  send_to_charf(ch,"nStrDupUnsafe   : %d\n\r", nStrDupUnsafe );
  send_to_charf(ch,"nAllocPerm      : %d\n\r", nAllocPermStr );
  send_to_charf(ch,"sAllocPerm      : %d\n\r", sAllocPermStr );
  send_to_charf(ch,"nAllocPermGC    : %d\n\r", nAllocPermGC );
  send_to_charf(ch,"nAllocPermHash  : %d\n\r", nAllocPermHash );
  send_to_charf(ch,"nAllocPermNoHash: %d\n\r", nAllocPermNoHash );
  send_to_charf(ch,"sStrDupNoGC     : %d\n\r", sStrDupNoGC );
  send_to_charf(ch,"sStrDupUnsafe   : %d\n\r", sStrDupUnsafe );
  send_to_charf(ch,"\n\r");

  return;
}

void do_dump( CHAR_DATA *ch, const char *argument ) {
  int count,count2,num_pcs,aff_count;
  CHAR_DATA *fch;
  MOB_INDEX_DATA *pMobIndex;
  PC_DATA *pc;
  OBJ_DATA *obj;
  OBJ_INDEX_DATA *pObjIndex;
  ROOM_INDEX_DATA *room;
  EXIT_DATA *exit;
  DESCRIPTOR_DATA *d;
  AFFECT_DATA *af;
  FILE *fp;
  int vnum,nMatch = 0;

  /* open file */
  fclose(fpReserve);
  if ( ( fp = fopen("../data/mem.dmp","w") ) == NULL ) {
    bug("Can't open dump file");
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  /* report use of data structures */
    
  num_pcs = 0;
  aff_count = 0;

  /* mobile prototypes */
  fprintf(fp,"MobProt	%4d (%8d bytes)\n",
	  top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

  /* mobs */
  count = 0;  count2 = 0;
  for (fch = char_list; fch != NULL; fch = fch->next) {
    count++;
    if (fch->pcdata != NULL)
      num_pcs++;
    for (af = fch->affected; af != NULL; af = af->next)
      aff_count++;
  }

  fprintf(fp,"Mobs	%4d (%8d bytes)\n",
	  count, count * (sizeof(*fch)));

  /* pcdata */

  fprintf(fp,"Pcdata	%4d (%8d bytes)\n",
	  num_pcs, num_pcs * (sizeof(*pc)));

  /* descriptors */
  count = 0; count2 = 0;
  for (d = descriptor_list; d != NULL; d = d->next)
    count++;

  fprintf(fp, "Descs	%4d (%8d bytes)\n",
	  count, count * (sizeof(*d)));

  /* object prototypes */
  for ( vnum = 0; nMatch < top_obj_index; vnum++ )
    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL ) {
      for (af = pObjIndex->affected; af != NULL; af = af->next)
	aff_count++;
      nMatch++;
    }

  fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
	  top_obj_index, top_obj_index * (sizeof(*pObjIndex)));


  /* objects */
  count = 0;  count2 = 0;
  for (obj = object_list; obj != NULL; obj = obj->next) {
    count++;
    for (af = obj->affected; af != NULL; af = af->next)
      aff_count++;
  }
  fprintf(fp,"Objs	%4d (%8d bytes)\n",
	  count, count * (sizeof(*obj)));

  /* affects */
  fprintf(fp,"Affects	%4d (%8d bytes)\n",
	  aff_count, aff_count * (sizeof(*af)));

  /* rooms */
  fprintf(fp,"Rooms	%4d (%8d bytes)\n",
	  top_room, top_room * (sizeof(*room)));

  /* exits */
  fprintf(fp,"Exits	%4d (%8d bytes)\n",
	  top_exit, top_exit * (sizeof(*exit)));

  fclose(fp);

  /* start printing out mobile data */
  fp = fopen("../stat/mob.dmp","w");

  fprintf(fp,"\nMobile Analysis\n");
  fprintf(fp,  "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_mob_index; vnum++)
    if ((pMobIndex = get_mob_index(vnum)) != NULL) {
      nMatch++;
      fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
	      pMobIndex->vnum,pMobIndex->count,
	      pMobIndex->killed,pMobIndex->short_descr);
    }
  fclose(fp);

  /* start printing out object data */
  fp = fopen("../stat/obj.dmp","w");

  fprintf(fp,"\nObject Analysis\n");
  fprintf(fp,  "---------------\n");
  nMatch = 0;
  for (vnum = 0; nMatch < top_obj_index; vnum++)
    if ((pObjIndex = get_obj_index(vnum)) != NULL) {
      nMatch++;
      fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
	      pObjIndex->vnum,pObjIndex->count,
	      pObjIndex->reset_num,pObjIndex->short_descr);
    }

  /* close file */
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  send_to_char("Dump done.\n\r", ch );
}



/*
 * Append a string to a file.
 */
void append_file( CHAR_DATA *ch, const char *file, const char *str ) {
  FILE *fp;

  if ( IS_NPC(ch) || str[0] == '\0' )
    return;

  fclose( fpReserve );
  if ( ( fp = fopen( file, "a" ) ) == NULL ) {
    perror( file );
    send_to_char( "Could not open the file!\n\r", ch );
  }
  else {
    fprintf( fp, "[%5d] %s: %s\n",
	     ch->in_room ? ch->in_room->vnum : 0, ch->name, str );
    fclose( fp );
  }

  fpReserve = fopen( NULL_FILE, "r" );
  return;
}



/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
  va_list argptr;
  char buf[MAX_STRING_LENGTH];

  if ( fpArea != NULL ) {
    int iLine;
    int iChar;

    if ( fpArea == stdin ) {
      iLine = 0;
    }
    else {
      iChar = ftell( fpArea );
      fseek( fpArea, 0, 0 );
      for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ ) {
	while ( getc( fpArea ) != '\n' )
	  ;
      }
      fseek( fpArea, iChar, 0 );
    }

    sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
    log_string( buf );
  }

  strcpy( buf, "[*****] BUG: " );

  va_start(argptr, str); /* Oxtal */
  vsprintf(buf + strlen(buf) , str, argptr);
  va_end(argptr);

  log_string( buf );

  // Added by SinaC 2001
  wiznet(buf,NULL,NULL,WIZ_BUGS,0,0);

  /* RT removed due to bug-file spamming 
     fclose( fpReserve );
     if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
     {
     fprintf( fp, "%s\n", buf );
     fclose( fp );
     }
     fpReserve = fopen( NULL_FILE, "r" );
  */

  return;
}



/*
 * Writes a string to the log.
 */
void log_string( const char *str )
{
  char *strtime;

  strtime                    = ctime( &current_time );
  strtime[strlen(strtime)-1] = '\0';
  fprintf( stderr, "%s :: %s\n", strtime, str );
  return;
}

/*
 * Writes a string to the log, w/ format.
 */

void log_stringf(const char *fmt, ...)
{
  va_list argptr;

  static char buf[MAX_STRING_LENGTH]; /* Enough ?*/

  va_start(argptr, fmt);
  vsprintf(buf, fmt, argptr);
  log_string(buf);

  va_end(argptr);
}


// Added by SinaC 2000 for 'jog'
void free_runbuf(DESCRIPTOR_DATA *d)
{
  if (d && d->run_buf)
    {
      d->run_buf = NULL;
      d->run_head = NULL;
    }
  return;
}


/* Parse name function. Very ugly. */
const char * parse_name(const char * name)
{
  const char *c;
  
  if(!(c=name)) return ("");
  
  while(*c && *c!='}') c++;
  if(!*c) return(name);
  
  while(*c && *++c==' ');
  while(*c && *++c!=' ');
  while(*c && *++c==' ');
  return(c);
}


/*
 * Read a float from a file.
 */
float fread_float( FILE *fp ) {
  float number;
  bool sign;
  char c;

  do {
    c = getc( fp );
  } while ( isspace(c) );

  number = 0;

  sign   = FALSE;
  if ( c == '+' ) {
    c = getc( fp );
  } 
  else if ( c == '-' ) {
    sign = TRUE;
    c = getc( fp );
  }

  if ( !isdigit(c) ) {
    bug( "Fread_number: bad format.");
    exit( 1 );
  }

  // before mantissa
  while ( isdigit(c) ) {
    number = number * 10.0 + float(c - '0');
    c      = getc( fp );
  }

  if ( sign )
    number = 0 - number;

  // after mantissa
  float mant = 0;
  float n = 1;
  if ( c == '.' || c == ',' ) {
    c      = getc( fp );
    while ( isdigit(c) ) {
      mant = mant * 10.0 + float(c - '0');
      c      = getc( fp );
      n*=10.0;
    }
  }

  if ( c != ' ' )
    ungetc( c, fp );

  return number + ( mant / n );
}


/*
 * Read a letter from a file.
 */
char fread_letter( FILE *fp ) {
  char c;

  do {
    c = getc( fp );
  } while ( isspace(c) );

  return c;
}
