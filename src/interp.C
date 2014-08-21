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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

// Added by SinaC 2001
#include "classes.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "act_wiz.h"
#include "olc_value.h"
#include "noncombatabilities.h"
#include "combatabilities.h"
#include "raceabilities.h"
#include "const.h"
#include "recycle.h"
#include "power.h"
#include "wiznet.h"
#include "act_wiz2.h"
#include "act_info.h"
#include "act_move.h"
#include "skills.h"
#include "magic.h"
#include "clan.h"
#include "act_obj.h"
#include "act_wiz3.h"
#include "act_enter.h"
#include "fight.h"
#include "board.h"
#include "olc.h"
#include "act_comm.h"
#include "healer.h"
#include "classes.h"
#include "copyover.h"
#include "flags.h"
#include "punish.h"
#include "scan.h"
#include "alias.h"
#include "hints.h"
#include "wizutil.h"
#include "arena.h"
#include "act_scri.h"
#include "wiznet.h"
#include "hunt.h"
#include "ban.h"
#include "language.h"
#include "dbdata.h"
#include "song.h"
#include "ability.h"
#include "group.h"
#include "config.h"
#include "data_edit.h"
#include "olc_save.h"
#include "script_edit.h"
#include "error.hh"
#include "remort.h"
#include "ranged.h"
#include "statistics.h"
#include "save.h"
#include "utils.h"
#include "html.h"
#include "asciimap.h"




// Added by SinaC 2003 for SIGSEGV capture
char    last_command[MAX_STRING_LENGTH];

bool	check_social	args( ( CHAR_DATA *ch, const char *command,
				const char *argument ) );

// disabled commands
// Added by SinaC 2001 for player disabled command
bool check_disabled_plr( CHAR_DATA *ch, struct cmd_type *command );
/* Add for disable a command by Sinac 1997 */
bool    check_disabled( struct cmd_type *command );



DISABLED_DATA *disabled_first;
#define END_MARKER      "END"

//const char *logName[] = {
//  "normal",
//  "always",
//  "never"
//};

//int getLogName( const char *name ) {
//  for ( int i = 0; i < 3; i++ )
//    if ( !str_cmp( name, logName[i] ) )
//      return i;
//  return -1;
//}


/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;




// Added by SinaC 2001
struct	cmd_type	*cmd_table;


/*
 * Init command table., Modified by SinaC 2001,  other infos are in a file now
 */
const struct	cmd_type_init	cmd_table_init	[] =
{

  // Common movement commands.
  { "north",		do_north },
  { "east",		do_east },
  { "south",		do_south },
  { "west",		do_west },
  { "up",		do_up },
  { "down",		do_down },
  // Added by SinaC 2003
  { "northeast",        do_northeast },
  { "northwest",        do_northwest },
  { "southeast",        do_southeast },
  { "southwest",        do_southwest },
  { "ne",               do_northeast },
  { "nw",               do_northwest },
  { "se",               do_southeast },
  { "sw",               do_southwest },
     //Common other commands.
     //Placed here so one and two letter abbreviations work.
    
  // Added by SinaC 2000 for bard songs
  { "sing",	    do_song },
  { "songs",        do_songs },
    // Added by SinaC 2001 for mental user, modified by SinaC 2003   psi  were  power before
  { "psi",              do_power},
  
  { "at",               do_at },
  { "cast",		do_cast },
  { "auction",          do_auction },
  { "buy",		do_buy },
  { "channels",         do_channels },
  { "exits",		do_exits },
  { "get",		do_get },
  { "goto",             do_goto },
  { "group",            do_group },
  // Removed by SinaC 2000
  //    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, 0 },
  { "inventory",	do_inventory },
  { "kill",		do_kill },
  { "look",		do_look },
  { "clantalk",		do_clantalk },
  //  { "music",            do_music },  removed by SinaC 2003

  { "order",		do_order },
  { "practice",         do_practice },
  { "rest",		do_rest },
  { "sit",		do_sit },
  { "sockets",          do_sockets },
  { "stand",		do_stand },
  { "tell",		do_tell },
  { "unlock",           do_unlock },
  { "wield",		do_wear },
  { "wizhelp",	        do_wizhelp },

  // Informational commands.
  { "affects",	        do_affects },
  { "areas",		do_areas },
  { "bug",		do_bug },
  { "commands",	        do_commands },
  { "compare",	        do_compare },
  { "consider",	        do_consider },
  { "count",		do_count },
  { "credits",	        do_credits },
  { "equipment",	do_equipment },
  { "examine",	        do_examine },
  //  { "groups",		do_groups,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
  { "help",		do_help },
  { "info",             do_groups },
//  { "motd",		do_motd },
  { "read",		do_read },
  { "report",		do_report },
//  { "rules",		do_rules },
  { "score",		do_score },
  { "skills",		do_skills },
  { "socials",	        do_socials },
  //    { "show",		do_show,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
  { "spells",		do_spells },
    // Added by SinaC 2001 for mental user, modified by SinaC 2003  was psipowers before
  { "powers",           do_psipowers },

//  { "story",		do_story },
  { "time",		do_time },
  { "typo",		do_typo },
  { "weather",	        do_weather },
  { "who",		do_who },
  { "whois",		do_whois },
  { "wizlist",	        do_wizlist },
//  { "worth",		do_worth },

  // Added commands by Seytan 1997

  { "scan",           do_scan },
//  { "hunt",           do_hunt },
  { "bet",            do_bet },
//  { "campfire",       do_fire },

  // Added command by Seytan for Quest Code 1997
  //  { "quest",          do_quest },
   
   // Configuration commands.
  { "alia",		do_alia },
  { "alias",		do_alias },
  { "autolist",	        do_autolist },
  { "autoassist",	do_autoassist },
  { "autoexit",	        do_autoexit },
  { "autogold",	        do_autogold },
  { "autoloot",	        do_autoloot },
  { "autosac",	        do_autosac },
  { "autosplit",	do_autosplit },
  { "autotitle",        do_autotitle },
  { "brief",		do_brief },
  //  { "channels",	do_channels,	POS_DEAD,	 0,  LOG_NORMAL, 1 },
  { "color",		do_colour },
  { "combine",	        do_combine },
  { "compact",	        do_compact },
  { "description",	do_description },
  { "delet",		do_delet },
  { "delete",		do_delete },
  { "nofollow",	        do_nofollow },
  { "noloot",		do_noloot },
  { "nosummon",	        do_nosummon },
  { "outfit",		do_outfit },
  { "password",	        do_password },
  { "prompt",		do_prompt },
  { "scroll",		do_scroll },
  { "title",		do_title },
  { "unalias",	        do_unalias },
  { "wimpy",		do_wimpy },

    // Added by SinaC 2001
  { "config",           do_config },

  // Communication commands.
  { "afk",		do_afk },
  { "answer",		do_answer },
  //  { "auction",	do_auction,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
  { "deaf",		do_deaf },
  { "emote",		do_emote },
  { "pmote",		do_pmote },
  // Gossip replaced with OOC by SinaC 2001
  //{ ".",		do_gossip,	POS_SLEEPING,	 0,  LOG_NORMAL, 0 },
  //{ "gossip",		do_gossip,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
  { ".",		do_ooc },
  { "ooc",		do_ooc },
  { ",",		do_emote },
  { "grats",		do_grats },
  { "gtell",		do_gtell },
  { ";",		do_gtell },
  { "hints",		do_hints },
  //  { "music",		do_music,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
  { "note",		do_note },
  { "nwrite",		do_nwrite },
  { "board",		do_board },
  { "pose",		do_pose },
  { "pray",             do_pray },
  { "question",	        do_question },
  { "quote",		do_quote },
  { "quiet",		do_quiet },
  { "reply",		do_reply },
  { "replay",		do_replay },
  { "say",		do_say },
  { "'",		do_say },
  { "shout",		do_shout },
  { "yell",		do_yell },
  { "trivia",         do_trivia },

     //Object manipulation commands.
  { "brandish",	        do_brandish },
  { "close",		do_close },
  { "donate",           do_donate },
  { "drink",		do_drink },
  { "drop",		do_drop },
  { "eat",		do_eat },
//  { "envenom",	        do_envenom },
  { "fill",		do_fill },
  { "give",		do_give },
  { "heal",		do_heal },
  { "hold",		do_wear },
  { "list",		do_list },
  { "lock",		do_lock },
  { "open",		do_open },
//  { "pick",		do_pick },
  { "pour",		do_pour },
  { "put",		do_put },
  { "quaff",		do_quaff },
  { "recite",		do_recite },
  { "remove",		do_remove },
  { "sell",		do_sell },
  { "second",		do_second },
  { "take",		do_get },
  { "sacrifice",	do_sacrifice },
  { "junk",             do_sacrifice },
  { "tap",      	do_sacrifice },
  //  { "unlock",		do_unlock,	POS_RESTING,	 0,  LOG_NORMAL, 1 },
  { "value",		do_value },
  { "wear",		do_wear },
  { "zap",		do_zap },
  // Added by Sinac 1997

//  { "sharpen",	        do_sharpen },
//  { "roundkick",        do_roundhousekick },
  // added by SinaC 2000
//  { "tailsweep",        do_tail },

  // Added by Sinac 1997
       
//  { "level",          do_level,     	POS_DEAD,        0,  LOG_NORMAL, 1 },
    
  // autoaffect, oaffect  SinaC 2000

  { "autoaffect",       do_autoaff },

  //Combat commands.
//  { "backstab",	        do_backstab },
//  { "bash",		do_bash },
//  { "bs",		do_backstab },
//  { "berserk",	        do_berserk },
//  { "dirt",		do_dirt },
//  { "disarm",		do_disarm },
  { "flee",		do_flee },
//  { "kick",		do_kick },
  { "murde",		do_murde },
  { "murder",		do_murder },
//  { "rescue",		do_rescue },
//  { "trip",		do_trip },
//  { "lore",		do_lore },
//  { "throw",            do_throw },

     //Miscellaneous commands.
  { "enter", 		do_enter },
  { "follow",		do_follow },
  { "gain",		do_gain },
  { "go",		do_enter },
  //  { "group",		do_group,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
  { "groups",		do_groups },
//  { "hide",		do_hide },
//  { "play",		do_play },  removed by SinaC 2003
  //  { "practice",	do_practice,	POS_SLEEPING,	 0,  LOG_NORMAL, 1 },
  { "qui",		do_qui },
  { "quit",		do_quit },
  { "recall",		do_recall },
  { "/",		do_recall },
  { "rent",		do_rent },
  { "save",		do_save },
  { "sleep",		do_sleep },
//  { "sneak",		do_sneak },
  { "split",		do_split },
//  { "steal",		do_steal },
  { "train",		do_train },
  { "visible",	        do_visible },
  { "wake",		do_wake },
  { "where",		do_where },
  { "multiclass",	do_multiclass },
  { "remort",           do_remort }, // SinaC 2003

  //CLANS

  { "myclan",           do_myclan },
  { "petition",	        do_petition },

  { "hometown",	        do_hometown },


  //Wizutil

  { "exlist",           do_exlist },
  { "vlist",            do_vlist },
  { "rename",           do_rename },
  { "for",              do_for },

  // Oxtal
  { "track",		do_track },
  { "path",		do_path },
  { "omatch",		do_omatch },
  // SinaC 2000
  { "mmatch",		do_mmatch },

     //Immortal commands.
  { "programs",	        do_programs },
     

  { "gtrivia",          do_gtrivia },
  //  { "gquest",           do_gquest },

  { "xpbonus",	        do_xpbonus },
  { "spy",              do_spy },
  { "advance",	        do_advance },
  { "dump",	        do_dump },
  { "trust",	        do_trust },
  { "violate",	        do_violate },
  { "copyove",	        do_copyove },
  { "copyover",	        do_copyover },

  { "allow",		do_allow },
  { "ban",		do_ban },
  { "deny",		do_deny },
  { "disconnect",	do_disconnect },
  { "flag",		do_flag },
  { "freeze",		do_freeze },
  { "permban",	        do_permban },
  { "protect",	        do_protect },
  { "reboo",		do_reboo },
  { "reboot",		do_reboot },
  { "set",		do_set },
  { "xset",		do_xset },
  { "shutdow",	        do_shutdow },
  { "shutdown",	        do_shutdown },
  //    { "sockets",	do_sockets,	POS_DEAD,	L4,  LOG_NORMAL, 1 },
  { "wizlock",	        do_wizlock },
    
  { "setpet",	        do_setpet },

  { "force",		do_force },
  { "load",		do_load },
  { "newlock",	        do_newlock },
  { "nochannels",	do_nochannels },
  { "noemote",	        do_noemote },
  { "noshout",	        do_noshout },
  { "notell",		do_notell },
  { "pecho",		do_pecho },
  { "pardon",		do_pardon },
  { "purge",		do_purge },
  { "restore",	        do_restore },
  { "sla",		do_sla },
  { "slay",		do_slay },
  { "teleport",	        do_transfer },
  { "transfer",	        do_transfer },

//  { "at",		do_at,		POS_DEAD,	L6,  LOG_NORMAL, 1 },
  { "poofin",		do_bamfin },
  { "poofout",	        do_bamfout },
  { "gecho",		do_echo },
  //  { "goto",		do_goto,	POS_DEAD,	L8,  LOG_NORMAL, 1 },
  { "holylight",	do_holylight },
  { "incognito",	do_incognito },
  { "log",		do_log },
  { "memory",		do_memory },
  { "mwhere",		do_mwhere },
  { "owhere",		do_owhere },
  { "peace",		do_peace },
  { "echo",		do_recho },
  { "return",           do_return },
  { "snoop",		do_snoop },
  { "stat",		do_stat },
  // Added by SinaC 2000
  { "mstat",            do_mstat },
  { "ostat",            do_ostat },
  { "rstat",            do_rstat },
  // Added by SinaC 2000
  { "mset",             do_mset },
  { "oset",             do_oset },
  { "rset",             do_rset },
  { "sset",             do_sset },

// Oxtal 1997
  { "xstat",		do_xstat },
  { "string",		do_string },
  { "switch",		do_switch },
  { "wizinvis",	        do_invis },
  { "vnum",		do_vnum },
  { "zecho",		do_zecho },

  { "clone",		do_clone },

    { "wiznet",		do_wiznet },
//  { "imotd",            do_imotd },
  { "immtalk",	        do_immtalk },
  { ":",		do_immtalk },
  // Modified by SinaC 2000, I can't figure out why mortals can't use that
  { "smote",		do_smote },

  { "prefi",		do_prefi },
  { "prefix",		do_prefix },

  // Imm command by Sinac 1997
  { "olevel",           do_olevel },
  { "mlevel",           do_mlevel },
  { "disable",          do_disable },

  // Added by Sinac 1997
  { "slookup",          do_slookup },
    

  // CLANS : Immortals
  { "clans",		do_clans },
  { "guild",		do_guild },

  //OLC
  { "edit",		do_olc },
  { "asave",            do_asave },
  { "alist",		do_alist },
  { "resets",		do_resets },
  { "redit",         	do_redit },
  { "medit",         	do_medit },
  { "aedit",         	do_aedit },
  { "oedit",         	do_oedit },
  // SinaC 2003,  datas online edition
  { "abilityedit",      do_ability_edit },
  { "abilitysave",      do_ability_save },
  { "raceedit",         do_race_edit    },
  { "pcraceedit",       do_pcrace_edit  },
  { "racesave",         do_race_save    },
  { "commandedit",      do_command_edit },
  { "commandsave",      do_command_save },
  { "pcclassedit",      do_pcclass_edit   },
  { "pcclasssave",      do_pcclasses_save },
  { "godedit",          do_god_edit     },
  { "godsave",          do_god_save     },
  { "liquidedit",       do_liquid_edit  },
  { "liquidsave",       do_liquid_save  },
  { "materialedit",     do_material_edit },
  { "materialsave",     do_material_save },
  { "brewedit",         do_brew_edit },
  { "brewsave",         do_brew_save },
  { "groupedit",        do_group_edit },
  { "groupsave",        do_group_save },
  { "factionedit",      do_faction_edit },
  { "factionsave",      do_faction_save },
  { "magicschooledit",  do_magic_school_edit },
  { "magicschoolsave",  do_magic_school_save },
  { "dedit",            do_dedit },
  { "dlist",            do_data_list },
  { "dshow",            do_data_show },
  { "dsave",            do_data_save },


  { "fire",             do_bowfire },

  { "pedit",            do_script_edit },
  { "psave",            do_script_save },

// Added by SinaC 2000
    // Immortal
  { "oaffect",	      do_oaffects },
  { "setpassword",    do_setpassword },
  { "mudstatus",      do_mudstatus },
  { "check",          do_check },
  { "addlag",         do_addlag },
  { "resetlist",      do_resetlist },
  { "skillstat",      do_skillstat },
  { "spellstat",      do_spellstat },
  { "groupstat",      do_groupstat },
  // Added by SinaC 2001 for mental user
  { "powerstat",      do_powerstat },
  // Added by SinaC 2003 for bard
  { "songstat",       do_songstat },
  { "fvlist",         do_fvlist },
  { "immtitle",       do_immtitle },
  { "idle",           do_idle },
  { "grab",           do_confiscate },
  { "wrlist",         do_wrlist },
  { "groupshow",      do_show_group }, // Added by SinaC 2003
  { "owherevnum",     do_owherevnum },
  { "mwherevnum",     do_mwherevnum },
  { "writestat",      do_writestat },
  { "resetarea",      do_resetarea },
  { "quake",          do_quake },
  { "bighunt",        do_bighunt },
  { "afremove",       do_afremove },
  { "mortal",         do_mortal },
// :))
  { "wiztest",        do_wiztest },
//
  { "?",              do_olchelp },
  { "prereq",         do_prereq },
  { "rmatch",         do_rmatch },
  { "godinfo",        do_godinfo },
  { "displrcmd",      do_disable_plr },
  { "checkskill",     do_checkskill },
  { "acceptname",     do_acceptname },
  { "field",          do_field },
  { "ostatvnum",      do_ostatvnum },
  { "mstatvnum",      do_mstatvnum },
  { "findfollower",   do_findfollower },
  //{ "branding",       do_brand },
  { "gset",           do_gset },
  { "areacount",      do_areacount },
  { "dataconfig",     do_dataconfig },

    // Mortal
//  { "whirlwind",      do_whirlwind },
  { "jog",            do_jog },  // !!!Don't forget to change interpret if you modify this command!!!
//  { "circle",         do_circle },
  { "knock",          do_knock },
//  { "findfamilar" ,   do_familiar },
//  { "dartthrow",      do_throwing },
  { "stance",         do_stance },
//  { "meditate",       do_meditate },
//  { "deathgrip",      do_deathgrip },
//  { "butcher",        do_butcher },
//  { "lure",           do_lure },
//  { "pillify",        do_pillify },
    
//  { "bladethirst",    do_bladethirst },
  { "pull",           do_pull },
//  { "barkskin",       do_barkskin },
//  { "crush",          do_crush },
//  { "warcry",         do_warcry },
//  { "spike",          do_spike },
//  { "pugil",          do_pugil },
//  { "lash",           do_lash },
//  { "forage",         do_forage },
//  { "rearkick",       do_rear_kick },
//  { "findwater",      do_find_water },
//  { "shcleave",       do_shield_cleave },
//  { "endure",         do_endure },
//  { "nerve",          do_nerve },
//  { "bandage",        do_bandage },
//  { "herb",           do_herb },
//  { "cleave",         do_cleave },
  { "clanlist",       do_clanlist },
// Removed by SinaC 2001
//  { "gamble",         do_gamble },
//  { "uncover",        do_uncover },
//  { "stake",          do_stake },
//  { "poisondetect",   do_poison_detect },
//  { "aligndetect",    do_align_detect },
//  { "magicdetect",    do_magic_detect },
//  { "exitsdetect",    do_exits_detect },
//  { "ironhand",       do_iron_hand },
  { "whisper",        do_whisper },
  { "ic",             do_ic },
  { "osay",	      do_osay },
  { "language",       do_language },
  { "teach",          do_teach },
  { "use",            do_use },
  { "dirsay",         do_dirsay },
  { "qdirsay",        do_qdirsay },
  { "autotick",	      do_autotick },


  // Test command for rebirth
//  { "lycanthropy",    do_lycanthropy },
//  { "speedup",        do_speedup },
//  { "invisible",      do_invisible },
//  { "feed",           do_feed },
//  { "wardrums",       do_war_drums },
//  { "repair",         do_repair },
//  { "wail",           do_wail },
//  { "resize",         do_resize },
//  { "forge",          do_forge },
//  { "vorpalize",      do_vorpalize },
//  { "morph",          do_morph },

  // Added by SinaC 2003 for mount code
  { "mount",          do_mount },
  { "dismount",       do_dismount },
  // Added by SinaC 2003 for sub-class system
  { "specialize",     do_specialize },
  { "otell",          do_otell },
  { "push",           do_push },
  { "setscript",      do_set_script },
  { "showinfo",       do_showinfo },
  { "dive",           do_dive },
  { "exmatch",        do_exmatch },
  { "climb",          do_climbing },
  { "restring",       do_restring },
  { "strengthen",     do_strengthen },
  { "third",          do_third_wield },
  { "fourth",         do_fourth_wield },

  { "showfaction",    do_showfaction },
  { "reload",         do_reload },
  { "amatch",         do_amatch },
  { "generatehtml",   do_generate_html },

    //Arena Commands
  { "chaos",          do_chaos },

  { "betarena",       do_betarena },
  { "arena",          do_arena },
  { "awho",           do_awho },
  { "challenge",      do_challenge },
  { "accept",         do_accept },
  { "decline",        do_decline },
  { "ahall",          do_ahall },
  // end Arena commands

  { "bigmap",         do_bigmap },


  /*
     * End of list.
     */
  { "",		0 }
};


// Added by SinaC
#define MAX_ERRORS 4
struct error_type
{
  char *      text;
};

const struct error_type error_table [MAX_ERRORS] =
{
  {"What?" },
  {"Speak common please!"},
  {"I don't understand Klingon."},
  {"Huh????"}
}; 

void random_error( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  int number;

  number = number_range( 0, MAX_ERRORS-1);

  sprintf ( buf, "%s\n\r", error_table[number].text);
  send_to_char ( buf, ch );
  return;
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret( CHAR_DATA *ch, const char *argument0 )
{
  char command[MAX_INPUT_LENGTH];
  char logline[MAX_INPUT_LENGTH];
  int cmd;
  int trust;
  bool found;
  // Added by SinaC 2001, true if it's a mob program command
  bool found_mob;

  char *argument = new char [MAX_INPUT_LENGTH];
  strcpy(argument,argument0);
  for( char *s = argument; *s; s++ )
    *s = ( *s == '~' ? '-' : *s );

  /*
   * Strip leading spaces.
   */
  while ( isspace(*argument) )
    argument++;
  if ( argument[0] == '\0' )
    return;

  // Added by SinaC 2001
  if ( ch->in_room == NULL ) {
    send_to_char("You're not in a room ... How did you do that ?", ch );
    return;
  }


  /*
   * No hiding.
   */
  /* Oxtal> I still don't figure out what this does */
  // when we're Hiding, if we do something, Hide disappears  SinaC 2000
  if (IS_SET(ch->bstat(affected_by),AFF_HIDE)) {
    REMOVE_BIT( ch->bstat(affected_by), AFF_HIDE );
    recompute(ch);
  }

  /*
   * Implement freeze command.
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_FREEZE) ) {
    send_to_char( "You're totally frozen!\n\r", ch );
    return;
  }

  /*
   * Grab the command word.
   * Special parsing so ' can be a command,
   *   also no spaces needed after punctuation.
   */
  strcpy( logline, argument );

  sprintf(last_command,"%s(%d) in room[%d]: %s.",
	  ch->name,
	  IS_NPC(ch)?ch->pIndexData->vnum:-1,
	  ch->in_room?ch->in_room->vnum:-1,
	  argument );

  const char* argument2;
  if ( !isalpha(argument[0]) && !isdigit(argument[0])) {
    command[0] = argument[0];
    command[1] = '\0';
    argument++;
    while ( isspace(*argument) )
      argument++;

    argument2 = argument;
  }
  else {
    // Added by SinaC 2003, really crappy
    if ( !str_prefix( "jog", argument ) || !str_prefix( "jo", argument ) ) // for jog command
      argument2 = no_lower_one_argument( argument, command );
    else
      argument2 = one_argument( argument, command );
  }

  /*
   * Look for command in command table.
   */
  // Added by SinaC 2001
  found_mob = FALSE;
  found = FALSE;
  trust = get_trust( ch );
  //for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ ) {
  for ( cmd = 0; cmd < MAX_COMMANDS; cmd++ ) {
    if ( command[0] == cmd_table[cmd].name[0]
	 &&   !str_prefix( command, cmd_table[cmd].name )
	 &&   cmd_table[cmd].level <= trust ) {
      found = TRUE;
      break;
    }
  }

  /*
   * Log and snoop.
   */
  if ( found && cmd_table[cmd].log == LOG_NEVER )
    strcpy( logline, "" );

  if ( found && 
       ( ( !IS_NPC(ch) && IS_SET(ch->act, PLR_LOG) )
	 ||   fLogAll
	 ||   cmd_table[cmd].log == LOG_ALWAYS ) ) {
    // Modified by SinaC 2000
    sprintf( log_buf, "Log %s: %s (%s)", 
	     ch->name, 
	     logline, 
	     cmd_table[cmd].name
	     );
    wiznet(log_buf,ch,NULL,WIZ_SECURE,0,get_trust(ch));
    log_string( log_buf );
  }

  
  // Added by SinaC 2001, if player was afk, afk is removed.
  if ( IS_SET(ch->comm,COMM_AFK)
       // unless player is trying to remove afk with afk command
       && ( found && str_cmp(cmd_table[cmd].name,"afk") )
       && !IS_IMMORTAL(ch) ) {
    send_to_char("{GAFK{x mode removed.\n\r",ch);
    if (buf_string(ch->pcdata->buffer)[0] != '\0' )
      send_to_char("{rYou have received tells: Type {Y'replay'{r to see them.{x\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_AFK);
  }


  if ( ch->desc != NULL && ch->desc->snoop_by != NULL ) {
    write_to_buffer( ch->desc->snoop_by, "% ",    2 );
    write_to_buffer( ch->desc->snoop_by, logline, 0 );
    write_to_buffer( ch->desc->snoop_by, "\n\r",  2 );
  }

  if ( !found ) {
    if( !check_social( ch, command, argument2 ) ) {
      // Modified by SinaC 2000
      //	    send_to_char( "Huh?\n\r", ch );
      random_error( ch );
      if ( SCRIPT_VERBOSE > 0 ) {
	if ( IS_NPC(ch) ) // Added by SinaC 2003
	  log_stringf("%s (%d) tries to use command: %s %s", 
		      NAME(ch), ch->pIndexData->vnum,
		      command, argument2 );
      }
    }
      
      return;
  }
  else {
    if ( check_disabled(&cmd_table[cmd]) ) {
      send_to_char( "This command has been temporarily disabled.\n\r", ch );
      return;
    }
    // Added by SinaC 2001 for player disabled commands
    if ( check_disabled_plr( ch, &cmd_table[cmd] ) ) {
      send_to_char( "The gods has removed your ability to use that command.\n\r", ch );
      return;
    }
  }

  /*
   * Character not in position for command?
   */
  if ( ch->position < cmd_table[cmd].position ) {
    switch( ch->position ) {
    case POS_DEAD:
      send_to_char( "Lie still; you are DEAD.\n\r", ch );
      break;

    case POS_MORTAL:
    case POS_INCAP:
      send_to_char( "You are hurt far too bad for that.\n\r", ch );
      break;

      // Added by SinaC 2003
    case POS_PARALYZED:
      send_to_char( "You are paralyzed, you can't move.\n\r", ch);
      break;

    case POS_STUNNED:
      send_to_char( "You are too stunned to do that.\n\r", ch );
      break;

    case POS_SLEEPING:
      send_to_char( "In your dreams, or what?\n\r", ch );
      break;

    case POS_RESTING:
      send_to_char( "Nah... You feel too relaxed...\n\r", ch);
      break;

    case POS_SITTING:
      send_to_char( "Better stand up first.\n\r",ch);
      break;

    case POS_FIGHTING:
      send_to_char( "No way!  You are still fighting!\n\r", ch);
      break;

    }
    return;
  }

  /*
   * Dispatch the command.
   */

  /* Oxtal -- Output the commands */
  /* SinaC --- We don't need that for the moment
   *if ( cmd_table[cmd].log != LOG_NEVER ) {
   * char *strtime;
   *
   * strtime = ctime( &current_time );
   * strtime[strlen(strtime)-1] = '\0';
   * fprintf( stdout, "%s %15s [%5d] %s %s\n",
   *  strtime,
   *  ch->name,
   *  ch->in_room ? ch->in_room->vnum : 0,
   *  cmd_table[cmd].name,
   *  argument);
   *}
   */ // removed by SinaC 2000
  
  // Modified by SinaC 2001 to detect memory leak
  char buf[MAX_STRING_LENGTH];

  // Modified by SinaC 2001
  if ( found )
    (*cmd_table[cmd].do_fun) ( ch, argument2 );

  return;
}



bool check_social( CHAR_DATA *ch, const char *command, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int cmd;
  bool found;

  found  = FALSE;
  for ( cmd = 0; social_table[cmd].name[0] != '\0'; cmd++ ) {
    if ( command[0] == social_table[cmd].name[0]
	 &&   !str_prefix( command, social_table[cmd].name ) ) {
      found = TRUE;
      break;
    }
  }

  if ( !found )
    return FALSE;

  if ( !IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE) ) {
    send_to_char( "You are anti-social!\n\r", ch );
    return TRUE;
  }

  switch ( ch->position ) {
  case POS_DEAD:
    send_to_char( "Lie still; you are DEAD.\n\r", ch );
    return TRUE;

  case POS_INCAP:
  case POS_MORTAL:
    send_to_char( "You are hurt far too bad for that.\n\r", ch );
    return TRUE;

    // Added by SinaC 2003
  case POS_PARALYZED:
    send_to_char( "You are paralyzed, you can't move.\n\r", ch);
    //send_to_char( "You can't move.\n\r", ch);
    return TRUE;

  case POS_STUNNED:
    send_to_char( "You are too stunned to do that.\n\r", ch );
    return TRUE;

  case POS_SLEEPING:
    /*
     * I just know this is the path to a 12" 'if' statement.  :(
     * But two players asked for it already!  -- Furey
     */
    if ( !str_cmp( social_table[cmd].name, "snore" ) )
      break;
    send_to_char( "In your dreams, or what?\n\r", ch );
    return TRUE;

  }

  one_argument( argument, arg );
  victim = NULL;
  if ( arg[0] == '\0' ) {
    act( social_table[cmd].others_no_arg, ch, NULL, victim, TO_ROOM    );
    act( social_table[cmd].char_no_arg,   ch, NULL, victim, TO_CHAR    );

    // everyone in the room
    CHAR_DATA *vch_next = NULL;
    for ( CHAR_DATA *vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
      vch_next = vch->next_in_room;
      if ( vch != ch )
	MOBPROG( vch, NULL, "onSocial", ch, social_table[cmd].name );
    }
  }
  else if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
  }
  else if ( victim == ch ) {
    act( social_table[cmd].others_auto,   ch, NULL, victim, TO_ROOM    );
    act( social_table[cmd].char_auto,     ch, NULL, victim, TO_CHAR    );
  }
  else {
    act( social_table[cmd].others_found,  ch, NULL, victim, TO_NOTVICT );
    act( social_table[cmd].char_found,    ch, NULL, victim, TO_CHAR    );
    act( social_table[cmd].vict_found,    ch, NULL, victim, TO_VICT    );

    if ( !IS_NPC(ch) && IS_NPC(victim)
	 &&   !IS_AFFECTED(victim, AFF_CHARM)
	 &&   IS_AWAKE(victim) 
	 &&   victim->desc == NULL) {
      switch ( number_bits( 4 ) ) {
      case 0:

      case 1: case 2: case 3: case 4:
      case 5: case 6: case 7: case 8:
	act( social_table[cmd].others_found,
	     victim, NULL, ch, TO_NOTVICT );
	act( social_table[cmd].char_found,
	     victim, NULL, ch, TO_CHAR    );
	act( social_table[cmd].vict_found,
	     victim, NULL, ch, TO_VICT    );
	break;

      case 9: case 10: case 11: case 12:
	act( "$n slaps $N.",  victim, NULL, ch, TO_NOTVICT );
	act( "You slap $N.",  victim, NULL, ch, TO_CHAR    );
	act( "$n slaps you.", victim, NULL, ch, TO_VICT    );
	break;
      }
    }

    MOBPROG( victim, NULL, "onSocial", ch, social_table[cmd].name ); // only send to victim
  }

  return TRUE;
}



/*
 * Return true if an argument is completely numeric.
 */
bool is_number ( const char *arg )
{
  if ( *arg == '\0' )
    return FALSE;
 
  if ( *arg == '+' || *arg == '-' )
    arg++;
 
  for ( ; *arg != '\0'; arg++ ) {
    if ( !isdigit( *arg ) )
      return FALSE;
  }
 
  return TRUE;
}



/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
int number_argument( const char *argument, char *arg )
{
  char buf[MAX_INPUT_LENGTH];
  strcpy(buf, argument);
  char *pdot;
  int number;
    
  for ( pdot = buf; *pdot != '\0'; pdot++ ) {
    if ( *pdot == '.' ) {
      *pdot = '\0';
      number = atoi( buf );
      *pdot = '.';
      strcpy( arg, pdot+1 );
      return number;
    }
  }

  strcpy( arg, buf );
  return 1;
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
*/
int mult_argument(const char *argument, char *arg)
{
  char buf[MAX_INPUT_LENGTH];
  strcpy(buf, argument);
  char *pdot;
  int number;

  for ( pdot = buf; *pdot != '\0'; pdot++ ) {
    if ( *pdot == '*' ) {
      *pdot = '\0';
      number = atoi( buf );
      *pdot = '*';
      strcpy( arg, pdot+1 );
      return number;
    }
  }
 
  strcpy( arg, buf);
  return 1;
}



/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument( const char *argument, char *arg_first )
{
  char cEnd;

  while ( isspace(*argument) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' ) {
    if ( *argument == cEnd ) {
      argument++;
      break;
    }
    *arg_first = LOWER(*argument);
    arg_first++;
    argument++;
  }
  *arg_first = '\0';

  while ( isspace(*argument) )
    argument++;

  return argument;
}

const char *one_optional_number_argument( const char *argument, char *arg_first ) {
  const char* res = one_argument(argument, arg_first);
  if (is_number(arg_first))
    return res;
  else {
    arg_first[0] = '\0';
    return argument;
  }
}


// Added by SinaC 2003
const char *no_lower_one_argument( const char *argument, char *arg_first ) {
  char cEnd;

  while ( isspace(*argument) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' ) {
    if ( *argument == cEnd ) {
      argument++;
      break;
    }
    *arg_first = *argument;
    arg_first++;
    argument++;
  }
  *arg_first = '\0';

  while ( isspace(*argument) )
    argument++;

  return argument;
}


/*
 * Contributed by Alander.
 */
void do_commands( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  int cmd;
  int col;
 
  col = 0;
  //for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ ) {
  for ( cmd = 0; cmd < MAX_COMMANDS; cmd++ ) {
    if ( cmd_table[cmd].level < LEVEL_HERO
	 &&   cmd_table[cmd].level <= get_trust( ch ) 
	 &&   cmd_table[cmd].show) {
      sprintf( buf, "%-13s", cmd_table[cmd].name );
      send_to_char( buf, ch );
      if ( ++col % 6 == 0 )
	send_to_char( "\n\r", ch );
    }
  }
 
  if ( col % 6 != 0 )
    send_to_char( "\n\r", ch );
  return;
}

// Modified by SinaC 2001
void do_wizhelp( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  int cmd;
  int col;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't see immortal command.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  int lvl = get_trust(ch);
  if ( argument[0] != '\0' && is_number(argument)) {
    lvl = UMIN( atoi(argument), lvl );
  }

  col = 0;
  //for ( cmd = 0; cmd_table[cmd].name[0] != '\0'; cmd++ ) {
  for ( cmd = 0; cmd < MAX_COMMANDS; cmd++ ) {
    if ( cmd_table[cmd].level >= LEVEL_HERO
	 &&   cmd_table[cmd].level <= /*get_trust( ch )*/lvl // Modified by SinaC 2001
	 &&   cmd_table[cmd].show) {
      sprintf( buf, "%-12s", cmd_table[cmd].name );
      send_to_char( buf, ch );
      if ( ++col % 6 == 0 )
	send_to_char( "\n\r", ch );
    }
  }
 
  if ( col % 6 != 0 )
    send_to_char( "\n\r", ch );
  return;
}
/*
===========================================================================
This snippet was written by Erwin S. Andreasen, erwin@pip.dknet.dk. You may 
use this code freely, as long as you retain my name in all of the files. You
also have to mail me telling that you are using it. I am giving this,
hopefully useful, piece of source code to you for free, and all I require
from you is some feedback.

Please mail me if you find any bugs or have any new ideas or just comments.

All my snippets are publically available at:

http://pip.dknet.dk/~pip1773/

If you do not have WWW access, try ftp'ing to pip.dknet.dk and examine
the /pub/pip1773 directory.
===========================================================================
*/
/* Syntax is:
disable - shows disabled commands
disable <command> - toggles disable status of command
*/

void do_disable (CHAR_DATA *ch, const char *argument) {
  int i;
  DISABLED_DATA *p,*q;
  char buf[100];
	
  if (IS_NPC(ch)) {
    send_to_char ("RETURN first.\n\r",ch);
    return;
  }
	
  if (!argument[0]) {/* Nothing specified. Show disabled commands. */
    if (!disabled_first) {/* Any disabled at all ? */
      send_to_char ("There are no commands disabled.\n\r",ch);
      return;
    }

    send_to_char ("Disabled commands:\n\r"
		  "Command      Level   Disabled by\n\r",ch);
		                
    for (p = disabled_first; p; p = p->next) {
      sprintf (buf, "%-12s %5d   %-12s\n\r",p->command->name, p->level, p->disabled_by);
      send_to_char (buf,ch);
    }
    return;
  }
	
  /* command given */

  /* First check if it is one of the disabled commands */
  for (p = disabled_first; p ; p = p->next)
    if (!str_cmp(argument, p->command->name))
      break;
			
  if (p) {/* this command is disabled */
    /* Optional: The level of the imm to enable the command must equal or exceed level
       of the one that disabled it */
	
    if (get_trust(ch) < p->level) {
      send_to_char ("This command was disabled by a higher power.\n\r",ch);
      return;
    }
		
    /* Remove */
		
    if (disabled_first == p) /* node to be removed == head ? */
      disabled_first = p->next;
    else {/* Find the node before this one */
      for (q = disabled_first; q->next != p; q = q->next); /* empty for */
      q->next = p->next;
    }
		
    new_save_disabled(); /* save to disk */
    send_to_char ("Command enabled.\n\r",ch);
  }
  else {/* not a disabled command, check if that command exists */
    /* IQ test */
    if (!str_cmp(argument,"disable")) {
      send_to_char ("You cannot disable the disable command.\n\r",ch);
      return;
    }

    /* Search for the command */
    //for (i = 0; cmd_table[i].name[0] != '\0'; i++)
    for (i = 0; i < MAX_COMMANDS; i++)
      if (!str_cmp(cmd_table[i].name, argument))
	break;

    /* Found? */				
    if (cmd_table[i].name[0] == '\0') {
      send_to_char ("No such command.\n\r",ch);
      return;
    }

    /* Can the imm use this command at all ? */				
    if (cmd_table[i].level > get_trust(ch))	{
      send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
      return;
    }
		
    /* Disable the command */

    p = (DISABLED_DATA*) GC_MALLOC (sizeof(DISABLED_DATA));

    p->command = &cmd_table[i];
    p->disabled_by = str_dup (ch->name); /* save name of disabler */
    p->level = get_trust(ch); /* save trust */
    p->next = disabled_first;
    disabled_first = p; /* add before the current first element */
		
    send_to_char ("Command disabled.\n\r",ch);
    new_save_disabled(); /* save to disk */
  }
}

// Added by SinaC 2001 for player disabled commands 
void do_disable_plr(CHAR_DATA *ch, const char *argument) {
  int i;
  DISABLED_CMD_DATA *p,*q;
  char buf[100];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
    
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (IS_NPC(ch)) {
    send_to_char ("Mobiles can't use that command.\n\r",ch);
    return;
  }

  if ( !arg1[0] ) {
    send_to_char("You must specify a target player.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
    send_to_char("Invalid target.\n\r", ch );
    return;
  }

  if (IS_NPC(victim)) {
    send_to_char("You can't disable NPC commands.\n\r", ch );
    return;
  }
   
  if (!arg2[0]) {/* Nothing specified. Show disabled commands. */
    if (!victim->pcdata->disabled_cmd) {/* Any disabled at all ? */
      send_to_char ("That player have no commands disabled.\n\r",ch);
      return;
    }

    send_to_charf (ch,"Disabled commands for player %s:\n\r"
		  "Command         Disabled by\n\r",
		  NAME(victim));
		                
    for (p = victim->pcdata->disabled_cmd; p; p = p->next) {
      sprintf (buf, "%-12s    %-12s\n\r",p->cmd->name, p->disabled_by );
      send_to_char (buf,ch);
    }
    return;
  }
	
  /* command given */

  /* First check if it is one of the disabled commands */
  for (p = victim->pcdata->disabled_cmd; p ; p = p->next)
    if (!str_cmp(arg2, p->cmd->name))
      break;
			
  if (p) {/* this command is disabled */
    /* Remove */
		
    if (victim->pcdata->disabled_cmd == p)/* node to be removed == head ? */
      victim->pcdata->disabled_cmd = p->next;
    else {/* Find the node before this one */
      for (q = victim->pcdata->disabled_cmd; q->next != p; q = q->next); /* empty for */
      q->next = p->next;
    }
    
    send_to_char ("Command enabled.\n\r",ch);
  }
  else {/* not a disabled command, check if that command exists */
    /* IQ test */
    if (!str_cmp(arg2,"disableplr")) {
      send_to_char ("You cannot disable the disableplr command.\n\r",ch);
      return;
    }

    /* Search for the command */
    for (i = 0; cmd_table[i].name[0] != '\0'; i++)
      if (!str_cmp(cmd_table[i].name, arg2))
	break;

    /* Found? */				
    if (cmd_table[i].name[0] == '\0') {
      send_to_char ("No such command.\n\r",ch);
      return;
    }

    /* Can the imm use this command at all ? */				
    if (cmd_table[i].level > get_trust(ch))	{
      send_to_char ("You dot have access to that command; you cannot disable it.\n\r",ch);
      return;
    }
		
    /* Disable the command */

    if ( ( p = (DISABLED_CMD_DATA *) GC_MALLOC( sizeof( DISABLED_CMD_DATA ) ) ) == NULL ) {
      bug("Memory allocation error in disable_cmd_data\n\r");
      exit(-1);
    }
    p->cmd = &cmd_table[i];
    p->disabled_by = str_dup(ch->name); /* save name of disabler */
    p->next = victim->pcdata->disabled_cmd;
    victim->pcdata->disabled_cmd = p; /* add before the current first element */
		
    send_to_char ("Command disabled.\n\r",ch);
  }
  new_save_pFile( victim, FALSE ); // force pFile saving
}

// Added by SinaC 2001 for player disabled commands
bool check_disabled_plr( CHAR_DATA *ch, struct cmd_type *command ) {
  DISABLED_CMD_DATA *p;
	
  if ( IS_NPC(ch) )
    return FALSE;

  for (p = ch->pcdata->disabled_cmd; p ; p = p->next)
    if (p->cmd->do_fun == command->do_fun)
      return TRUE;

  return FALSE;
}

/* Check if that command is disabled 
   Note that we check for equivalence of the do_fun pointers; this means
   that disabling 'chat' will also disable the '.' command
*/   
bool check_disabled (struct cmd_type *command) {
  DISABLED_DATA *p;
	
  for (p = disabled_first; p ; p = p->next)
    if (p->command->do_fun == command->do_fun)
      return TRUE;

  return FALSE;
}

//****************************************************************
//************************** Disable *****************************

//Disable <STRING Command Name> {
//  Level = <INTEGER Level>;
//  Player = <STRING Immortal who has disabled the command>;
//}

// Save disabled commands
void new_save_disabled() {
  FILE *fp;
  DISABLED_DATA *p;
  fclose(fpReserve);
  if (!(fp = fopen (DISABLED_FILE, "w"))) {
    bug ("Could not open %s for writing", DISABLED_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for (p = disabled_first; p ; p = p->next)
    fprintf( fp,
	     "Disable '%s' {\n"
	     "  Level = %d;\n"
	     "  Player = '%s';\n"
	     "}\n\n",
	     p->command->name, p->level, p->disabled_by );
  fclose (fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

// Load disabled commands
void new_load_disabled() {
  FILE *fp;

  log_string("Reading disabled commands");
  disabled_first = NULL;
  fclose(fpReserve);
  if (!(fp = fopen (DISABLED_FILE, "r"))) { // No disabled file.. no disabled commands :)
    log_stringf("Couldn't open disabled commands file: %s", DISABLED_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  int count = parse_datas( fp );
  log_stringf(" %d disabled commands found.", count );

  fclose (fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

void parse_disabled( DATAData *disabled ) {
// tag:    Disable
// value:  <Command Name>
// fields: Level = <Disabler level>;
// fields: Player = <Disabler name>;
  // value
  const char *commandName = disabled->value->eval().asStr();
  int i;
  for ( i = 0; i < MAX_COMMANDS; i++ )
    if ( !str_cmp(cmd_table[i].name, commandName) )
      break;
  
  if (cmd_table[i].name == NULL || !cmd_table[i].name[0] ) { // command does not exist?
    bug ("Skipping unknown command %s in %s file.", commandName, DISABLED_FILE );
    return;
  }
if ( DATA_VERBOSE > 0 ) {
  printf("Command: %s [%d]\n\r", commandName, i );
}

  // create a new list elem and insert it in the list
  DISABLED_DATA *p = (DISABLED_DATA*) GC_MALLOC(sizeof(DISABLED_DATA));
  p->command = &cmd_table[i];
  p->next = disabled_first;
  disabled_first = p;

  // REMOVED FOR THE MOMENT
//  // Inheritance Find a parent disabled command and copy it
//  if ( disabled->parent != NULL ) {
//    const char *parentName = disabled->parent->eval().asStr();
//    DISABLED_DATA *q = disabled_first;
//    while ( q != NULL && str_cmp( q->command->name, parentName ) )
//      q = q->next;
//    if ( q == NULL ) { // not found
//      bug("Invalid parent command %s for command %s.", parentName, commandName );
//      p->level = 0;
//      p->disabled_by = str_dup("none");
//    }
//    else { // found, copy informations
//      p->level = q->level;
//      p->disabled_by = str_dup( q->disabled_by );
//    }
//  }
//  else { // no parent: dummy information
//    p->level = 0;
//    p->disabled_by = str_dup("none");
//  }

  // fields
  for ( int fieldCount = 0; fieldCount < disabled->fields_count; fieldCount++ ) {
    DATAData *field = disabled->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {

      // Disabler level
    case TAG_Level:
      p->level = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("level: %d\n\r", p->level );
}
      break;
    
      // Disabler name
    case TAG_Player:
      p->disabled_by = str_dup( field->value->eval().asStr() );
if ( DATA_VERBOSE > 2 ) {
      printf("disabler: %s\n\r", p->disabled_by );
}
      break;

    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

if ( DATA_VERBOSE > 1 ) {
  printf("Command: %s by %s lvl %d\n\r", p->command->name, p->disabled_by, p->level );
}
}

