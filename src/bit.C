/***************************************************************************
 *  File: bit.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 The code below uses a table lookup system that is based on suggestions
 from Russ Taylor.  There are many routines in handler.c that would benefit
 with the use of tables.  You may consider simplifying your code base by
 implementing a system like below with such functions. -Jason Dinkel
*/

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "bit.h"

// Added by SinaC 2001
#include "db.h"
#include "interp.h"
#include "utils.h"


bool FLAG_VALUE_ERROR;


char *convert_flag( long flags ) {
  static char buf[MAX_STRING_LENGTH];
  long offset;
  char *cp;

  buf[0] = '\0';

  if ( flags == 0 ) {
    strcpy( buf, "0" );
    return buf;
  }

  // 32 -- number of bits in a long 
  for ( offset = 0, cp = buf; offset < 32; offset++ )
    if ( flags & ( 1 << offset ) ) {
      if ( offset <= 'Z' - 'A' )
	*(cp++) = 'A' + offset;
      else
	*(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
    }

  *cp = '\0';

  return buf;
}




// Added by SinaC 2000
// Init the flag_stat_table used in bit.C
//  we need to initialize it here because classes_flags and races_flags are not known
//  at starting but only after load_classtable() & load_pcrace()
void init_flag_stat_table() {
  extern const struct flag_stat_type* flag_stat_table;
  static const struct flag_stat_type flag_stat_table_source[] = {
/*  {	structure		stat	}, */
    {	"area",   area_flags,		FALSE	},
    {   "sex",    sex_flags,		TRUE	},
    {   "exit",   exit_flags,		FALSE	},
    {   "door",   door_resets,		TRUE	},
    {   "room",   room_flags,		FALSE	},
    {   "sector", sector_flags,		TRUE	},
    {   "type",   type_flags,		TRUE	},
    {   "extra",  extra_flags,		FALSE	},
    {   "wear",   wear_flags,		FALSE	},
    {   "act",    act_flags,		FALSE	},
    {   "affect", affect_flags,		FALSE	},
    {   "wlocf",  wear_loc_flags,	TRUE	},
    {   "wlocs",  wear_loc_strings,	TRUE	},
    {   "container", container_flags,	FALSE	},
    {   "afto",   afto_type,              TRUE    },  
    {   "attr",   attr_flags,             TRUE    },
    {   "ops",    ops_flags,              TRUE    },
/* ROM specific flags: */
    {   "form",   form_flags,             FALSE   },
    {   "part",   part_flags,             FALSE   },
    {   "ac",     ac_type,                TRUE    },
    {   "size",   size_flags,             TRUE    },
    {   "position", position_flags,         TRUE    },
    {   "off",    off_flags,              FALSE   },

    // SinaC 2003, same tables
    //{   "imm",    imm_flags,              FALSE   },
    //{   "res",    res_flags,              FALSE   },
    //{   "vuln",   vuln_flags,             FALSE   },
    {   "imm",    irv_flags,               FALSE   },
    {   "res",    irv_flags,               FALSE   },
    {   "vuln",   irv_flags,               FALSE   },


    // Modified by SinaC 2001
    {   "wclass", weapon_class,           TRUE    },
    {   "damtype", dam_type_flags,         TRUE    },

    {   "weapon2", weapon_type2,           FALSE   },
// Added by SinaC 2000 for object restrictions
    {   "restr",  restr_flags,            TRUE    },
/* Oxtal */
    {   "classes", classes_flags,		FALSE	},
/* SinaC 2000 */
    {   "race",  races_flags,            TRUE    },
    // Added by SinaC 2001
    {   "affect2", affect2_flags,		FALSE	},
    // Added by SinaC 2001
    {   "material", material_flags,         TRUE    },
    // Added by SinaC 2001
    {   "etho", etho_flags,             TRUE    },
    // Added by SinaC 2001 for room affects
    {   "roomattr", room_attr_flags,        TRUE    }, // FALSE?
    {   "damclass", dam_class_flags,        TRUE   },
    // SinaC 2003
    {   "plrflags", plr_flags,              FALSE },
    {   "target", target_flags,         TRUE    },
    //{   "mobuse", mob_use_flags,        TRUE }, SinaC 2003
    {   "mobuse", mob_use_flags,        FALSE },
    {   "racetype", race_type_flags,        TRUE },
    {   "log", log_flags,               TRUE },
    {   "classtype", class_type_flags,               FALSE },
    {   "classchoosable", class_choosable_flags,               TRUE },
    {   "abilitytype", ability_type_flags,               TRUE },
    {   "affectflag", affect_data_flags,                 FALSE },
    {   "component", brew_component_flags,               TRUE },

    // Added by SinaC 2001 for disease
    //{   "disease",  disease_flags,          FALSE},  removed by SinaC 2003
    // Added by SinaC 2000
    {   0,			0	} 
  };

  flag_stat_table = flag_stat_table_source;
}

// Added by SinaC 2001
void update_flag_stat_table( const char *name, flag_type *flag ) {
  int i;

  extern const struct flag_stat_type* flag_stat_table;
  struct flag_stat_type *l_flag_stat_table = (struct flag_stat_type *) flag_stat_table;

  for ( i = 0; l_flag_stat_table[i].name != NULL; i++ )
    if ( !str_cmp( l_flag_stat_table[i].name, name ) )
      break;
  if ( l_flag_stat_table[i].name == NULL ) {
    bug("update_flag_stat_table: Invalid flag stat table entry: [%s]", name );
    exit(-1);
  }
  l_flag_stat_table[i].structure = flag;
}

/*****************************************************************************
 Name:		flag_stat_table
 Purpose:	This table categorizes the tables following the lookup
 		functions below into stats and flags.  Flags can be toggled
 		but stats can only be assigned.  Update this table when a
 		new set of flags is installed.
 ****************************************************************************/

// Initialized in db.C  init_flag_stat_table()
const struct flag_stat_type* flag_stat_table;

/*****************************************************************************
 Name:		is_stat( table )
 Purpose:	Returns TRUE if the table is a stat table and FALSE if flag.
 Called by:	flag_value and flag_string. + add_affect
 Note:		This function is NOT local and used NOT only in bit.c.
 ****************************************************************************/
bool is_stat( struct flag_type *flag_table ) {
  int flag;

  for (flag = 0; flag_stat_table[flag].structure; flag++) {
    if ( flag_stat_table[flag].structure == flag_table
	 && flag_stat_table[flag].stat )
      return TRUE;
  }
  return FALSE;
}

/*****************************************************************************
 Name:		get_flag_table_name( table )
 Purpose:	Returns flag table name
 Called by:	flag_string, flag_string_init, list_flag_string
 Note:		This function is NOT local and used NOT only in bit.c.
 ****************************************************************************/
const char *get_flag_table_name( struct flag_type *flag_table ) {
  int flag;

  for (flag = 0; flag_stat_table[flag].structure; flag++)
    if ( flag_stat_table[flag].structure == flag_table )
      return flag_stat_table[flag].name;
  return "none";
}



/*
 * This function is Russ Taylor's creation.  Thanks Russ!
 * All code copyright (C) Russ Taylor, permission to use and/or distribute
 * has NOT been granted.  Use only in this OLC package has been granted.
 */
/*****************************************************************************
 Name:		flag_lookup2( flag, table )
 Purpose:	Returns the value of a single, settable flag from the table.
 Called by:	flag_value and flag_string.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
long flag_lookup2 (const char *name, struct flag_type *flag_table) {
  int flag;
 
  for (flag = 0; flag_table[flag].name != NULL; flag++) {
    if ( !str_cmp( name, flag_table[flag].name )
	 && flag_table[flag].settable )
      return flag_table[flag].bit;
  }
 
  return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_lookup3( flag, table )
 Purpose:	Returns the value of a single, settable or not flag from the table.
 Called by:	flag_value_complete and flag_value_complete_init.
 Note:		This function is local and used only in bit.c.
 ****************************************************************************/
long flag_lookup3 (const char *name, struct flag_type *flag_table) {
  int flag;
 
  for (flag = 0; flag_table[flag].name != NULL; flag++) {
    if ( !str_cmp( name, flag_table[flag].name ) )
      return flag_table[flag].bit;
  }
 
  return NO_FLAG;
}

/*****************************************************************************
 Name:		flag_value( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
long flag_value( struct flag_type *flag_table, const char *argument) {
    char word[MAX_INPUT_LENGTH];
    long  bit;
    //int  marked = 0;
    long marked = 0;
    bool found = FALSE;

    if ( flag_table == NULL ) {
      bug("flag_value called with a NULL flag_table");
      return NO_FLAG;
    }
    if ( fBootDb )
      return flag_value_init( flag_table, argument );

    if ( is_stat( flag_table ) )
      return flag_lookup2(argument,flag_table);   

    for (; ;) {
      argument = one_argument( argument, word );

      if ( word[0] == '\0' )
	return found?marked:NO_FLAG;
	
      if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG ) {
	SET_BIT( marked, bit );
	found = TRUE;
      }
      else
	return NO_FLAG;
    }
}


/*****************************************************************************
 Name:		flag_string( table, flags/stat )
 Purpose:	Returns string with name(s) of the flags or stat entered.
 Called by:     affects.c, act_olc.c, olc.c, and olc_save.c.
 ****************************************************************************/
const char *flag_string( struct flag_type *flag_table, long bits ) {
  static char buf[512];
  
  int countBit = count_bit( bits );
  int countWord = 0;
  buf[0] = '\0';
  
  bool is_st = is_stat(flag_table);

  for (int flag = 0; flag_table[flag].name != NULL; flag++) {

    //if ( !is_stat( flag_table ) && IS_SET(bits, flag_table[flag].bit) ) {
    if ( !is_st && IS_SET(bits, flag_table[flag].bit) ) {
	
      strcat( buf, " " );
      strcat( buf, flag_table[flag].name );
      countWord++;
    }
    else if ( flag_table[flag].bit == bits ) {

      strcat( buf, " " );
      strcat( buf, flag_table[flag].name );
      countWord = countBit; // artifically set countWord to avoid entering bug test
      break;
    }
  }
  if ( countWord != countBit )
    bug("Flag_String: invalid number of word: %d [%s], number of bits %d [flag = %ld (%s)] [table: %s]",
	countWord, buf[0] != 0 ? buf+1: "none", countBit, 
	bits, convert_flag( bits), get_flag_table_name(flag_table) );
  return (buf[0] != '\0') ? buf+1 : "none";
}

/*****************************************************************************
 Name:		flag_value_maximum( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
                 if a bit doesn't exist, we don't return NO_FLAG, we continue
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
long flag_value_maximum( struct flag_type *flag_table, const char *argument) {
  char word[MAX_INPUT_LENGTH];
  long  bit;
  long marked = 0;
  bool found = FALSE;
  
  FLAG_VALUE_ERROR = FALSE;
  
  if ( flag_table == NULL ) {
    bug("flag_value_maximum called with a NULL flag_table");
    return NO_FLAG;
  }
  if ( fBootDb )
    return flag_value_maximum_init( flag_table, argument );
  
  if ( is_stat( flag_table ) )
    return flag_lookup2(argument,flag_table);   
  
  for (; ;) {
    argument = one_argument( argument, word );
    
    if ( word[0] == '\0' )
      return found?marked:NO_FLAG;
    
    if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG ) {
      SET_BIT( marked, bit );
      found = TRUE;
    }
    else
      FLAG_VALUE_ERROR = TRUE;
  }
  return marked;
}


/*****************************************************************************
 Name:		flag_value_complete( table, flag )
 Purpose:	Returns the value of the flags entered.  Multi-flags accepted.
 Called by:	olc.c and olc_act.c.
 ****************************************************************************/
long flag_value_complete( struct flag_type *flag_table, const char *argument) {
  char word[MAX_INPUT_LENGTH];
  long  bit;
  //int  marked = 0;
  long marked = 0;
  bool found = FALSE;

  if ( flag_table == NULL ) {
    bug("flag_value_complete called with a NULL flag_table");
    return NO_FLAG;
  }
  if ( fBootDb )
    return flag_value_complete_init( flag_table, argument );

  if ( is_stat( flag_table ) )
    return flag_lookup3(argument,flag_table);   

  if ( !str_cmp(argument,"none") ) // SinaC 2003, easy way to clear a flag
    return 0;                      // after is_stat because we only want to do this on a flag

  for (; ;) {
    argument = one_argument( argument, word );

    if ( word[0] == '\0' )
      return found?marked:NO_FLAG;

    if ( ( bit = flag_lookup3( word, flag_table ) ) != NO_FLAG ) {
      SET_BIT( marked, bit );
      found = TRUE;
    }
    else
      return NO_FLAG;
  }
}

// Used for the initialisation done in data.C such read classes and races
// I know, this is crappy, SinaC 2000
void init_flag_stat_table_init() {
  extern const struct flag_stat_type* flag_stat_table_init;
  static const struct flag_stat_type flag_stat_table_init_source[] = {
/*  {	structure		stat	}, */
    {	"area",   area_flags,		FALSE	},
    {   "sex",    sex_flags,		TRUE	},
    {   "exit",   exit_flags,		FALSE	},
    {   "door",   door_resets,		TRUE	},
    {   "room",   room_flags,		FALSE	},
    {   "sector", sector_flags,		TRUE	},
    {   "type",   type_flags,		TRUE	},
    {   "extra",  extra_flags,		FALSE	},
    {   "wear",   wear_flags,		FALSE	},
    {   "act",    act_flags,		FALSE	},
    {   "affect", affect_flags,		FALSE	},
    {   "wlocf",  wear_loc_flags,	TRUE	},
    {   "wlocs",  wear_loc_strings,	TRUE	},
    {   "container", container_flags,	FALSE	},
    {   "afto",   afto_type,              TRUE    },  
    {   "attr",   attr_flags,             TRUE    },
    {   "ops",    ops_flags,              TRUE    },
/* ROM specific flags: */
    {   "form",   form_flags,             FALSE   },
    {   "part",   part_flags,             FALSE   },
    {   "ac",     ac_type,                TRUE    },
    {   "size",   size_flags,             TRUE    },
    {   "position", position_flags,         TRUE    },
    {   "off",    off_flags,              FALSE   },

    // SinaC 2003, same tables
    {   "imm",    irv_flags,              FALSE   },
    {   "res",    irv_flags,              FALSE   },
    {   "vuln",   irv_flags,             FALSE   },
    //    {   "imm",    imm_flags,              FALSE   },
    //    {   "res",    res_flags,              FALSE   },
    //    {   "vuln",   vuln_flags,             FALSE   },

    // Modified by SinaC 2001
    {   "wclass", weapon_class,           TRUE    },
    {   "damtype", dam_type_flags,         TRUE    },

    {   "weapon2", weapon_type2,           FALSE   },
// Added by SinaC 2000 for object restrictions
    {   "restr",  restr_flags,            TRUE    },
/* Oxtal */
    {   "classes", classes_flags,		FALSE	},
/* SinaC 2000 */
    {   "race",  races_flags,            TRUE    },
    // Added by SinaC 2001
    {   "affect2", affect2_flags,		FALSE	},
    // Added by SinaC 2001
    {   "material", material_flags,         TRUE    },
    // Added by SinaC 2001
    {   "etho", etho_flags,             TRUE    },
    // Added by SinaC 2001 for room affects
    {   "roomattr", room_attr_flags,        TRUE    },
    {   "damclass", dam_class_flags,        TRUE   },
    // SinaC 2003
    {   "plrflags", plr_flags,              FALSE },
    {   "target", target_flags,         TRUE    },
    //{   "mobuse", mob_use_flags,        TRUE }, SinaC 2003
    {   "mobuse", mob_use_flags,        FALSE },
    {   "racetype", race_type_flags,        TRUE },
    {   "log", log_flags,               TRUE },
    {   "classtype", class_type_flags,               FALSE },
    {   "classchoosable", class_choosable_flags,               TRUE },
    {   "abilitytype", ability_type_flags,               TRUE },
    {   "affectflag", affect_data_flags,                 FALSE },
    {   "component", brew_component_flags,               TRUE },

    // Added by SinaC 2001 for disease
    //{   "disease",  disease_flags,          FALSE},  removed by SinaC 2003

    {   0,			0	}
  };

  flag_stat_table_init = flag_stat_table_init_source;
}
const struct flag_stat_type *flag_stat_table_init;

// Added by SinaC 2001
void update_flag_stat_table_init( const char *name, flag_type *flag ) {
  int i;

  extern const struct flag_stat_type* flag_stat_table_init;
  struct flag_stat_type *l_flag_stat_table = (struct flag_stat_type *) flag_stat_table_init;

  for ( i = 0; l_flag_stat_table[i].name != NULL; i++ )
    if ( !str_cmp( l_flag_stat_table[i].name, name ) )
      break;
  if ( l_flag_stat_table[i].name == NULL ) {
    bug("update_flag_stat_table: Invalid flag stat table entry: [%s]", name );
    exit(-1);
  }
  l_flag_stat_table[i].structure = flag;
}

bool is_stat_init( struct flag_type *flag_table ) {
  int flag;
  
  for (flag = 0; flag_stat_table_init[flag].structure; flag++) {
    if ( flag_stat_table_init[flag].structure == flag_table
	 && flag_stat_table_init[flag].stat )
      return TRUE;
  }
  return FALSE;
}

const char *get_flag_table_name_init( struct flag_type *flag_table ) {
  int flag;

  for (flag = 0; flag_stat_table_init[flag].structure; flag++)
    if ( flag_stat_table_init[flag].structure == flag_table )
      return flag_stat_table_init[flag].name;
  return "none";
}

const char *flag_string_init( struct flag_type *flag_table, long bits ) {
  static char buf[512];
  
  int countBit = count_bit( bits );
  int countWord = 0;
  buf[0] = '\0';
  
  for (int flag = 0; flag_table[flag].name != NULL; flag++) {

    if ( !is_stat_init( flag_table ) && IS_SET(bits, flag_table[flag].bit) ) {
	
      strcat( buf, " " );
      strcat( buf, flag_table[flag].name );
      countWord++;
    }
    else if ( flag_table[flag].bit == bits ) {

      strcat( buf, " " );
      strcat( buf, flag_table[flag].name );
      countWord = countBit; // artifically set countWord to avoid entering bug test
      break;
    }
  }
  if ( countWord != countBit )
    bug("Flag_String_Init: invalid number of word: %d [%s], number of bits %d [flag = %ld (%s)] [table: %s]",
	countWord, buf[0] != 0 ? buf+1: "none", countBit, 
	bits, convert_flag( bits), get_flag_table_name(flag_table) );
  return (buf[0] != '\0') ? buf+1 : "none";
}

long flag_value_init( struct flag_type *flag_table, const char *argument) {
  char word[MAX_INPUT_LENGTH];
  long  bit;
  long marked = 0;
  bool found = FALSE;

  // Added by SinaC 2001
  if ( strstr( argument, "none" ) )
    return 0;
  
  if ( is_stat_init( flag_table ) )
    return flag_lookup2(argument,flag_table);   
  
  for (; ;) {
    argument = one_argument( argument, word );
    
    if ( word[0] == '\0' )
      return found?marked:NO_FLAG;

    if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG ) {
      SET_BIT( marked, bit );
      found = TRUE;
    }
    else
      return NO_FLAG;
  }
}

long flag_value_maximum_init( struct flag_type *flag_table, const char *argument) {
  char word[MAX_INPUT_LENGTH];
  long  bit;
  long marked = 0;
  bool found = FALSE;

  FLAG_VALUE_ERROR = FALSE;

  // Added by SinaC 2001
  if ( strstr( argument, "none" ) )
    return 0;
  
  if ( is_stat_init( flag_table ) )
    return flag_lookup2(argument,flag_table);   
  
  for (; ;) {
    argument = one_argument( argument, word );
    
    if ( word[0] == '\0' )
      return found?marked:NO_FLAG;

    if ( ( bit = flag_lookup2( word, flag_table ) ) != NO_FLAG ) {
      SET_BIT( marked, bit );
      found = TRUE;
    }
    else
      FLAG_VALUE_ERROR = TRUE;
  }
  return marked;
}

long flag_value_complete_init( struct flag_type *flag_table, const char *argument) {
  char word[MAX_INPUT_LENGTH];
  long  bit;
  //int  marked = 0;
  long marked = 0;
  bool found = FALSE;

  // Added by SinaC 2001
  if ( strstr( argument, "none" ) ) {
    return 0;
  }
  
  if ( is_stat_init( flag_table ) )
    return flag_lookup3(argument,flag_table);   
  
  for (; ;) {
    argument = one_argument( argument, word );
    
    if ( word[0] == '\0' )
      return found?marked:NO_FLAG;
    
    if ( ( bit = flag_lookup3( word, flag_table ) ) != NO_FLAG ) {
      SET_BIT( marked, bit );
      found = TRUE;
    }
    else
      return NO_FLAG;
  }
}
