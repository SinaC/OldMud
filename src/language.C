/*
 * Added by SinaC 2001 to use racial languages
 */

#if defined( macintosh )
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <errno.h>		
#include <sys/time.h>
#endif
#include <ctype.h>		
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "merc.h"
//#include "olc.h"
#include "tables.h"
#include "lookup.h"
#include "bit.h"
#include "classes.h"
#include "language.h"
#include "olc_value.h"

// Added by SinaC 2001
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "gsn.h"
#include "interp.h"
#include "ability.h"
#include "config.h"
#include "utils.h"


//#define VERBOSE


struct syl_type {
  char *	old;
  char *	newsyl;
};

const struct syl_type common_table[] = {
  { " ",                 " " },

  {"a", "a"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

const struct syl_type elvish_table[] = {
  { " ",	         " "	        },

  { "a", "natha" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
  { "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
  { "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
  { "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
  { "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
  { "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
  { "y", "l" }, { "z", "k" },
  { "", "" }
};

const struct syl_type dwarfish_table[] = {
  { " ", " "},
  { "group",             "akh"          },

  {"a", "a"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

const struct syl_type gnomish_table[] = {
  { " ",                 " "            },

  {"a", "a"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

const struct syl_type giantish_table[] = {
  { " ",                 " "            },

  {"a", "uh"},   {"b", "gr"},   {"c", "cha"},  {"d", "dah"},  
  {"e", "eh"},   {"f", "kk"},   {"g", "ga"},   {"h", "ash"},  
  {"i", "u"},    {"j", "tch"},  {"k", "t"},    {"l", "r"},
  {"m", "chk"},  {"n", "i"},    {"o", "ec"},   {"p", "vt"},  
  {"q", "d"},    {"r", "f"},    {"s", "uk"},   {"t", "tch"},  
  {"u", "uuz"},  {"v", "z"},    {"w", "wch"},  {"x", "n"},
  {"y", "l"},    {"z", "k"},
  {"",  "" },
};

const struct syl_type goblinish_table[] = {
  { " ",                 " "            },

  { "green", "miden" },
  { "blood", "nir"},

  {"a", "a"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

const struct syl_type faerish_table[] = {
  { " ",                 " "            },

  {"a", "a"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

const struct syl_type dragonish_table[] = {
  { " ",                 " "            },

  {"a", "a"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

// was underdark tongue in Dracon (debarrian)
const struct syl_type underdish_table[] = {
  { " ",                 " "            },
  { "ally",              "abban"        },
  { "all",               "jal"          },
  { "al",                "ol"           },
  { "and",               "luet"         },
  { "as",                "izil"         },
  { "are",               "phuul"        },
  { "around",            "bauth"        },
  { "backstab",          "rathahnathab" },
  { "back",              "rath"         },
  { "below",             "harl"         },
  { "behind",            "rathrae"      },
  { "best",              "alurl"        },
  { "be",                "tlu"          },
  { "black",             "vel'oloth"    },
  { "darkness",          "oloth"        },
  { "death",             "elghinn"      },
  { "do",                "xun"          },
  { "er",                "uk"           },
  { "es",                "ir"           },
  { "fun",               "jivvin"       },
  { "give",              "belbau"       },
  { "guard",             "kyrol"        },
  { "her",               "usstils"      },
  { "him",               "usstil"       },
  { "holy",              "orthae"       },
  { "ion",               "mm"           },
  { "in",                "wun"          },
  { "is",                "zhah"         },
  { "item",              "bol"          },
  { "kill",              "ellg"         },
  { "life",              "dro"          },
  { "magic",             "faer"         },
  { "ment",              "lay"          },
  { "mother",            "ilhar"        },
  { "no",                "nau"          },
  { "of",                "del"          },
  { "one",               "uss"          },
  { "on",                "pholor"       },
  { "out",               "doeb"         },
  { "patron",            "ilharn"       },
  { "shit",              "iblith"       },
  { "slay",              "ellg"         },
  { "take",              "plynn"        },
  { "temple",            "yath"         },
  { "the",               "lil"          },
  { "thing",             "bol"          },
  { "those",             "nind"         },
  { "three",             "llar"         },
  { "th",                "nn"           },
  { "to",                "uss"          },
  { "two",               "draa"         },
  { "under",             "harl"         },
  { "us",                "usstens"      },
  { "walk",              "z'hin"        },
  { "war",               "thalack"      },
  { "warrior",           "sargltin"     },
  { "yes",               "lir"          },
  { "you",               "dos"          },

  {"a", "natha"},  {"b", "b"},  {"c", "q"},  {"d", "e"},  
  {"e", "z"},  {"f", "y"},  {"g", "o"},  {"h", "p"},  
  {"i", "u"},  {"j", "y"},  {"k", "t"},  {"l", "r"},
  {"m", "w"},  {"n", "i"},  {"o", "a"},  {"p", "s"},  
  {"q", "d"},  {"r", "f"},  {"s", "g"},  {"t", "h"},  
  {"u", "j"},  {"v", "z"},  {"w", "x"},  {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

const struct syl_type lizardish_table[] = {
  { " ",                 " "            },

  {"a", "x"},  {"b", "g"},   {"c", "lx"},  {"d", "r"},  
  {"e", "z"},  {"f", "sh"},  {"g", "s"},   {"h", "p"},  
  {"i", "l"},  {"j", "y"},   {"k", "t"},   {"l", "r"},
  {"m", "r"},  {"n", "gr"},  {"o", "x"},   {"p", "s"},  
  {"q", "d"},  {"r", "f"},   {"s", "g"},   {"t", "l"},  
  {"u", "x"},  {"v", "z"},   {"w", "x"},   {"x", "n"},
  {"y", "l"},  {"z", "k"},
  {"",  "" },
};

struct lang_type {
  char *name;
  int sn;
  const struct syl_type *syl_table;
};

struct lang_type language_table[MAX_LANGUAGE] = {
  { "common",    0, common_table    },
  { "elfish",    0, elvish_table    },
  { "dwarfish",  0, dwarfish_table  },
  { "gnomish",   0, gnomish_table   },
  { "giantish",  0, giantish_table  },
  { "goblinish", 0, goblinish_table },
  { "faerish",   0, faerish_table   },
  { "dragonish", 0, dragonish_table },
  { "underdish", 0, underdish_table },
  { "lizardish", 0, lizardish_table }
};

void assign_language_sn() {
  log_stringf(" %d languages found.", MAX_LANGUAGE );
  log_stringf("Assigning sn to language");
  for ( int i = 0; i < MAX_LANGUAGE; i++ ) {
    language_table[i].sn = ability_lookup(language_table[i].name);
    if ( language_table[i].sn <= 0 ) {
      bug("Language %s doesn't have an associated ability, assuming %s.",
	  language_table[i].name, DEFAULT_LANGUAGE_NAME );
      //language_table[i].sn = ability_lookup("common");
      language_table[i].sn = DEFAULT_LANGUAGE;
    }
  }
}

bool is_language( const int sn ) {
  for ( int i = 0; i < MAX_LANGUAGE; i++ )
    if ( language_table[i].sn == sn )
      return TRUE;

  return FALSE;
}

int language_lookup( const char *name ) {
  for ( int i = 0; i < MAX_LANGUAGE; i++ )
    if ( !str_cmp( language_table[i].name, name ) )
      return i;

  return -1;
}

int language_sn( int i ) {
  if ( i < 0 || i >= MAX_LANGUAGE )
    return -1;

  return language_table[i].sn;
}

char *language_name( int i ) {
  if ( i < 0 || i >= MAX_LANGUAGE )
    return "common";

  return language_table[i].name;
}

char *language_name_known( CHAR_DATA *ch, int lang ) {
  static char buf[MAX_STRING_LENGTH];

  if ( lang < 0 || lang >= MAX_LANGUAGE )
    lang = 0;

  int sk = get_ability( ch, language_sn( lang ) );
  if ( sk > 0 )
    sprintf(buf,"(%s) ", language_table[lang].name);
  else
    strcpy(buf,"");

  return buf;
}

int language_count( CHAR_DATA *ch ) {
  int count = 0;

  for ( int i = 0; i < MAX_LANGUAGE; i++ ) {
    int sn = language_sn( i );
    if ( sn > 0 
	 && get_ability( ch, sn ) > 0 )
      count++;
  }
  return count;
}

void do_language( CHAR_DATA *ch, const char *argument ){
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC( ch ) ) {
    send_to_char("NPC's can't use that command.\n\r", ch );
    return;
  }

  one_argument( argument, arg );
  // if no argument ==> shows the list of all known language
  if ( arg[0] == '\0' ) {
    send_to_charf( ch,
		   "Current spoken language: %s.\n\r",
		   language_name( ch->pcdata->language));
    send_to_char("Known language(s):\n\r", ch );
    bool found = FALSE;
    for ( int i = 0; i < MAX_LANGUAGE; i++ ) {
      int sn = language_sn( i );
      int pra;
      if ( sn < 0 )
	continue;
      if ( ( pra = get_ability( ch, sn ) ) > 0 ) {
	found = TRUE;
	send_to_charf( ch,
		       "%-20s  %3d%%\n\r",
		       language_name(i), pra );
      }
    }
    if ( !found )
      send_to_char("none  ==> PLEASE WARN AN IMMORTAL\n\r", ch );
    return;
  }

  // if an argument ==> current spoken language will be that one
  int lang = language_lookup( arg );
  if ( lang < 0 ) {
    send_to_char("Invalid language.\n\r", ch );
    return;
  }
  int sn = language_sn( lang );
  if ( sn <= 0 
       || get_ability( ch, sn ) <= 0 ) {
    send_to_char("You don't know how to speak that language.\n\r", ch );
    return;
  }
  
  ch->pcdata->language = lang;
  send_to_charf( ch, "Language set to %s.\n\r", language_name( lang ) );
}


int lang_test( int perc ) {
  // 90 --> 100    chance = 100-3*(100-%)    ( 90=>70 --> 100=>100 )
  //  0 -->  89    chance = %/2              (  0=> 0 -->  89=> 44 )
  if ( perc >= 90 )
    //chance = 100 - 3*( 100 - perc );
    return perc*3 - 200; // same as above but lighter
  else
    return perc/2;
}

/*
char *str_to_language( CHAR_DATA *ch, CHAR_DATA *victim, char *argument )
{
  static char buf[MAX_INPUT_LENGTH];
  char buf2[2];

  char *pName;
  int iSyl;
  int length = 0, length2 = 0;
  bool found;
  int lang;
  int lang_sn;
  int ch_perc, vict_perc;
  bool miss;
  const struct syl_type *syl_table = common_table;

  memset(buf,'\0',MAX_INPUT_LENGTH);

  // mob don't crypt what they say or what they hear
  if ( IS_NPC(ch) || IS_NPC(victim) ) {
    strcpy( buf, argument );
    return buf;
  }

  if ( IS_PC( ch ) )
    lang = ch->pcdata->language;
  else
    if ( race_table[ch->cstat(race)].pc_race )
      lang = pc_race_table[ch->cstat(race)].language;
    else
      // Mobs speak 'common'(=0) if their race is not a pc race
      lang = 0;

  lang_sn = language_sn( lang );
  ch_perc = get_ability( ch, lang_sn );
  vict_perc = get_ability( victim, lang_sn );

  // So mobiles are big master in every languages
  if ( IS_NPC(ch) )
    ch_perc = 100;
  if ( IS_NPC(victim) )
    vict_perc = 100;

  syl_table = language_table[lang].syl_table;

  buf[0]	= '\0';
  for ( pName = argument; *pName != '\0'; pName += length ) {
    found = FALSE;

    // ch or victim can miss, if missed we "crypt"
    int chance;
    miss = FALSE;
    // Can't miss non alphanumeric character
    if ( isalnum(*pName) ) {
      chance = lang_test( ch_perc );
      // ch missed ?
      if ( number_percent() > chance )
	miss = TRUE;
      // ch not missed, maybe vict will miss
      else {
	chance = lang_test( vict_perc );
	// vict missed
	if ( number_percent() > chance )
	  miss = TRUE;
      }
    }
    send_to_charf(ch,"missed ?: %s (%s) [%s]\n\r", 
		  miss?"TRUE ":"FALSE", pName, buf );
    for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ ) {
      if ( !str_prefix( syl_table[iSyl].old, pName ) ) {
	found = TRUE;
	// if missed we copy the new syl
	if ( miss )
	  strcat( buf, syl_table[iSyl].newsyl );
	// if NOT missed we copy the old syl
	else
	  strcat( buf, syl_table[iSyl].old );
	break;
      }
    }
    if ( length == 0 ) {
      length = 1;
      length2 = 1;
    }
    else
      // if missed we have to add the length of the new syl
      if ( miss )
	length2 = strlen(syl_table[iSyl].newsyl);
      // if NOT missed we have to add the length of the old syl
      else
	length2 = strlen(syl_table[iSyl].old);

    // add unknown character
    if (!found) {
      sprintf(buf2, "%c", *pName );
      strcat( buf, buf2 );
    }
    // If the 1st letter of the syl was in upper case, the new is in upper case too
    if (isupper(*pName))
      buf[strlen(buf)-length2] = UPPER(buf[strlen(buf)-length2]);
  }

  return buf;
}
*/

int get_current_language( CHAR_DATA *ch ) {
  if ( IS_PC( ch ) )
    return ch->pcdata->language;
  else
    if ( race_table[ch->cstat(race)].pc_race )
      return pc_race_table[ch->cstat(race)].language;
    else
      // Mobs speak 'common'(=0) if their race is not a pc race
      return 0;
}

// 'from' talks to 'ch'
char *str_to_language( CHAR_DATA *from, CHAR_DATA *ch, int language, char *argument ) {
  static char buf[MAX_INPUT_LENGTH];
  char buf2[2];

  char *pName;
  int iSyl;
  int length = 0, length2 = 0;
  bool found;
  int lang;
  int lang_sn;
  int ch_perc;
  const struct syl_type *syl_table = common_table;

  memset(buf,'\0',MAX_INPUT_LENGTH);

  // mob don't crypt what they say or what they hear
  if ( IS_NPC(ch) || IS_NPC(from) ) {
    strcpy( buf, argument );
    return buf;
  }

  lang_sn = language_sn( language );
  ch_perc = get_ability( ch, lang_sn );

  // Added by SinaC 2003 for comprehend languages skill
  // Get the highest value between comprehend languages and language
  ch_perc = UMAX( get_ability( ch, gsn_comprehend_languages ), ch_perc );
  // Added by SinaC 2003
  if ( is_affected( ch, gsn_comprehend_languages ) )
    ch_perc = 100;

  syl_table = language_table[language].syl_table;

  buf[0]	= '\0';
#ifdef VERBOSE
  int nb_miss = 0,
    nb_total = 0;
#endif
  for ( pName = argument; *pName != '\0'; pName += length ) {
    found = FALSE;

    // if missed we "crypt"
    int chance;
    bool miss = FALSE;
    // Can't miss non alphanumeric character
    if ( isalnum(*pName) ) {
      chance = lang_test( ch_perc );
      // ch missed ?
      if ( number_percent() > chance )
	miss = TRUE;
    }

    if (number_percent()<=2)
      if ( miss ) { // Modified by SinaC 2003 for comprehend languages
	check_improve( ch, lang_sn, FALSE, 10 );
	check_improve( ch, gsn_comprehend_languages, FALSE, 10 );
      }
      else {
	check_improve( ch, lang_sn, TRUE, 10 );
	check_improve( ch, gsn_comprehend_languages, TRUE, 10 );
      }

#ifdef VERBOSE
    if (miss)
      nb_miss++;
    nb_total++;
#endif

#ifdef VERBOSE
    send_to_charf(ch,"missed ?: %s (%s) [%s]\n\r", 
		  miss?"TRUE ":"FALSE", pName, buf );
#endif
    for ( iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++ ) {
      if ( !str_prefix( syl_table[iSyl].old, pName ) ) {
	found = TRUE;
	// if missed we copy the new syl
	if ( miss )
	  strcat( buf, syl_table[iSyl].newsyl );
	// if NOT missed we copy the old syl
	else
	  strcat( buf, syl_table[iSyl].old );
	break;
      }
    }
    if ( length == 0 ) {
      length = 1;
      length2 = 1;
    }
    else
      // if missed we have to add the length of the new syl
      if ( miss )
	length2 = strlen(syl_table[iSyl].newsyl);
      // if NOT missed we have to add the length of the old syl
      else
	length2 = strlen(syl_table[iSyl].old);

    // add unknown character
    if (!found) {
      sprintf(buf2, "%c", *pName );
      strcat( buf, buf2 );
    }
    // If the 1st letter of the syl was in upper case, the new is in upper case too
    if (isupper(*pName))
      buf[strlen(buf)-length2] = UPPER(buf[strlen(buf)-length2]);
  }
#ifdef VERBOSE
  send_to_charf(ch,"%3.2f%%  missed.\n\r", 
		100.0*(float)nb_miss/(float)nb_total);
#endif
  return buf;
}
