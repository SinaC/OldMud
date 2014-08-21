/***************************************************************************
 *  File: olc.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
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
#include "olc.h"
#include "recycle.h"


// Added by SinaC 2001
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "olc_value.h"
#include "interp.h"
#include "act_info.h"
#include "data_edit.h"
#include "olc_act.h"
#include "bit.h"
#include "mem.h"
#include "script_edit.h"
#include "update.h"
#include "brew.h"
#include "utils.h"
#include "restriction.h"


// Added by SinaC 2003: lock bit for structure editable from OLC
bool OLC_EDIT( CHAR_DATA *ch, OLCEditable *edit ) {
  if ( ch == NULL || !ch->valid )
    return FALSE;
  if ( edit == NULL )
    return TRUE;
  if ( edit->lockBit 
       && edit->editedBy != NULL
       && edit->editedBy->character != NULL
       && edit->editedBy->character->desc != NULL
       && edit->editedBy != ch->desc ) {
    send_to_charf(ch,"{RLOCKED:{x already edited by %s.\n\r", 
		  edit->editedBy->character->name );
    return FALSE;
  }
  edit->editedBy = ch->desc;
  edit->lockBit = TRUE;
  return TRUE;
}
// Same as above but don't set neither send anything
bool CAN_EDIT( CHAR_DATA *ch, OLCEditable *edit ) {
  if ( ch == NULL || !ch->valid )
    return FALSE;
  if ( edit == NULL )
    return TRUE;
  if ( edit->lockBit 
       && edit->editedBy != NULL
       && edit->editedBy->character != NULL
       && edit->editedBy->character->desc != NULL
       && edit->editedBy != ch->desc ) // edit by someone else than ch
    return FALSE;
  return TRUE;
}
const char *editor_name( OLCEditable *edit ) {
  return (edit && edit->editedBy && edit->editedBy->character)?edit->editedBy->character->name:"<?>";
}

void OLC_EDIT_DONE( CHAR_DATA *ch ) {
  if ( ch == NULL || ch->desc == NULL || !ch->valid )
    return;
  OLCEditable *edit = (OLCEditable *)ch->desc->pEdit;
  if ( edit == NULL )
    return;
  edit->editedBy = NULL;
  edit->lockBit = FALSE;
}



/*
 * Local functions.
 */


/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor( DESCRIPTOR_DATA *d ) {
  CHAR_DATA *ch = d->character;
  switch ( d->editor ) {
  case ED_AREA:
    REMOVE_BIT(ch->comm, COMM_EDITING);
    aedit( ch, d->incomm );
    break;
  case ED_ROOM:
    REMOVE_BIT(ch->comm, COMM_EDITING);
    redit( ch, d->incomm );
    break;
  case ED_OBJECT:
    REMOVE_BIT(ch->comm, COMM_EDITING);
    oedit( ch, d->incomm );
    break;
  case ED_MOBILE:
    REMOVE_BIT(ch->comm, COMM_EDITING);
    medit( ch, d->incomm );
    break;
    // SinaC 2003
  case ED_ABILITY:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    ability_edit( ch, d->incomm );
    break;
  case ED_RACE:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    race_edit( ch, d->incomm );
    break;
  case ED_PCRACE:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    pcrace_edit( ch, d->incomm );
    break;
  case ED_COMMAND:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    command_edit( ch, d->incomm );
    break;
  case ED_PCCLASS:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    pcclass_edit( ch, d->incomm );
    break;
  case ED_GOD:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    god_edit( ch, d->incomm );
    break;
  case ED_LIQUID:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    liquid_edit( ch, d->incomm );
    break;
  case ED_MATERIAL:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    material_edit( ch, d->incomm );
    break;
  case ED_BREW:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    brew_edit( ch, d->incomm );
    break;
  case ED_GROUP:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    group_edit( ch, d->incomm );
    break;
  case ED_FACTION:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    faction_edit( ch, d->incomm );
    break;
  case ED_MAGIC_SCHOOL:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    magic_school_edit( ch, d->incomm );
    break;

  case ED_SCRIPT:
    REMOVE_BIT(ch->comm, COMM_BUILDING);
    script_edit( ch, d->incomm );
    break;
  default:
    return FALSE;
  }

  return TRUE;
}



char *olc_ed_name( CHAR_DATA *ch ) {
  static char buf[10];
    
  buf[0] = '\0';
  switch (ch->desc->editor) {
  case ED_AREA:
    sprintf( buf, "AEdit" );
    break;
  case ED_ROOM:
    sprintf( buf, "REdit" );
    break;
  case ED_OBJECT:
    sprintf( buf, "OEdit" );
    break;
  case ED_MOBILE:
    sprintf( buf, "MEdit" );
    break;
    // SinaC 2003
  case ED_ABILITY:
    sprintf( buf, "AbilityEdit" );
    break;
  case ED_RACE:
    sprintf( buf, "RaceEdit" );
    break;
  case ED_PCRACE:
    sprintf( buf, "PCRaceEdit" );
    break;
  case ED_COMMAND:
    sprintf( buf, "CommandEdit" );
    break;
  case ED_PCCLASS:
    sprintf( buf, "PCClassEdit" );
    break;
  case ED_GOD:
    sprintf( buf, "GodEdit" );
    break;
  case ED_LIQUID:
    sprintf( buf, "liquidEdit" );
    break;
  case ED_MATERIAL:
    sprintf( buf, "MaterialEdit" );
    break;
  case ED_BREW:
    sprintf( buf, "BrewEdit" );
    break;
  case ED_GROUP:
    sprintf( buf, "GroupEdit" );
    break;
  case ED_FACTION:
    sprintf( buf, "FactionEdit" );
    break;
  case ED_MAGIC_SCHOOL:
    sprintf( buf, "MagicSchoolEdit" );
    break;

  case ED_SCRIPT:
    sprintf( buf, "PEdit" );
    break;
  default:
    //sprintf( buf, "" );
    buf[0] = '\0';
    break;
  }
  return buf;
}



char *olc_ed_vnum( CHAR_DATA *ch ) {
  AREA_DATA *pArea;
  ROOM_INDEX_DATA *pRoom;
  OBJ_INDEX_DATA *pObj;
  MOB_INDEX_DATA *pMob;
  static char buf[10];
	
  buf[0] = '\0';
  switch ( ch->desc->editor ) {
  case ED_AREA:
    pArea = (AREA_DATA *)ch->desc->pEdit;
    sprintf( buf, "[%d]", pArea ? pArea->vnum : 0 );
    break;
  case ED_ROOM:
    pRoom = ch->in_room;
    sprintf( buf, "[%d]", pRoom ? pRoom->vnum : 0 );
    break;
  case ED_OBJECT:
    pObj = (OBJ_INDEX_DATA *)ch->desc->pEdit;
    sprintf( buf, "[%d]", pObj ? pObj->vnum : 0 );
    break;
  case ED_MOBILE:
    pMob = (MOB_INDEX_DATA *)ch->desc->pEdit;
    sprintf( buf, "[%d]", pMob ? pMob->vnum : 0 );
    break;
    // SinaC 2003
  case ED_ABILITY: {
    ability_type *pAbility = (ability_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pAbility ? pAbility->name : "none" );
    break;
  }
  case ED_RACE: {
    race_type *pRace = (race_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pRace ? pRace->name : "none" );
    break;
  }
  case ED_PCRACE: {
    pc_race_type *pRace = (pc_race_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pRace ? pRace->name : "none" );
    break;
  }
  case ED_COMMAND: {
    cmd_type *pCommand = (cmd_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pCommand ? pCommand->name : "none" );
    break;
  }
  case ED_PCCLASS: {
    class_type *pClass = (class_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pClass ? pClass->name : "none" );
    break;
  }
  case ED_GOD: {
    god_data *pGod = (god_data*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pGod ? pGod->name : "none" );
    break;
  }
  case ED_LIQUID: {
    liq_type *pLiquid = (liq_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pLiquid ? pLiquid->liq_name : "none" );
    break;
  }
  case ED_MATERIAL: {
    material_type *pMaterial = (material_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pMaterial ? pMaterial->name : "none" );
    break;
  }
  case ED_BREW: {
    brew_formula_type *pBrew = (brew_formula_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pBrew ? pBrew->name : "none" );
    break;
  }
  case ED_GROUP: {
    group_type *pGroup = (group_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pGroup ? pGroup->name : "none" );
    break;
  }
  case ED_FACTION: {
    FACTION_DATA *pFaction = (FACTION_DATA*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pFaction ? pFaction->name : "none" );
    break;
  }
  case ED_MAGIC_SCHOOL: {
    magic_school_type *pSchool = (magic_school_type*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pSchool ? pSchool->name : "none" );
    break;
  }

  case ED_SCRIPT: {
    CLASS_DATA *pClass = (CLASS_DATA*) ch->desc->pEdit;
    sprintf( buf, "[%s]", pClass ? pClass->name : "none" );
    break;
  }
  default:
    //sprintf( buf, "" );
    buf[0] = '\0';
    break;
  }
  return buf;
}



/*****************************************************************************
 Name:		show_olc_cmds
 Purpose:	Format up the commands from given table.
 Called by:	show_commands(olc_act.c).
 ****************************************************************************/
void show_olc_cmds( CHAR_DATA *ch, const struct olc_cmd_type *olc_table )
{
  char buf  [ MAX_STRING_LENGTH ];
  char buf1 [ MAX_STRING_LENGTH ];
  int  cmd;
  int  col;
 
  buf1[0] = '\0';
  col = 0;
  for (cmd = 0; olc_table[cmd].name != NULL; cmd++) {
    sprintf( buf, "%-20.20s", olc_table[cmd].name );
    strcat( buf1, buf );
    if ( ++col % 4 == 0 )
      strcat( buf1, "\n\r" );
  }
 
  if ( col % 4 != 0 )
    strcat( buf1, "\n\r" );

  send_to_char( buf1, ch );
  return;
}



/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
bool show_commands( CHAR_DATA *ch, const char *argument )
{
  switch (ch->desc->editor) {
  case ED_AREA:
    show_olc_cmds( ch, aedit_table );
    break;
  case ED_ROOM:
    show_olc_cmds( ch, redit_table );
    break;
  case ED_OBJECT:
    show_olc_cmds( ch, oedit_table );
    break;
  case ED_MOBILE:
    show_olc_cmds( ch, medit_table );
    break;
    // SinaC 2003
  case ED_ABILITY:
    show_olc_cmds( ch, ability_edit_table );
    break;
  case ED_RACE:
    show_olc_cmds( ch, race_edit_table );
    break;
  case ED_PCRACE:
    show_olc_cmds( ch, pcrace_edit_table );
    break;
  case ED_COMMAND:
    show_olc_cmds( ch, command_edit_table );
    break;
  case ED_PCCLASS:
    show_olc_cmds( ch, pcclass_edit_table );
    break;
  case ED_GOD:
    show_olc_cmds( ch, god_edit_table );
    break;
  case ED_LIQUID:
    show_olc_cmds( ch, liquid_edit_table );
    break;
  case ED_MATERIAL:
    show_olc_cmds( ch, material_edit_table );
    break;
  case ED_BREW: {
    show_olc_cmds( ch, brew_edit_table );
    break;
  }
  case ED_GROUP: {
    show_olc_cmds( ch, group_edit_table );
    break;
  }
  case ED_FACTION: {
    show_olc_cmds( ch, faction_edit_table );
    break;
  }
  case ED_MAGIC_SCHOOL: {
    show_olc_cmds( ch, magic_school_edit_table );
    break;
  }

  case ED_SCRIPT:
    show_olc_cmds( ch, script_edit_table );
    break;
  }

  return FALSE;
}



/*****************************************************************************
 *                           Interpreter Tables.                             *
 *****************************************************************************/
const struct olc_cmd_type aedit_table[] =
{
  /*  {   command		function	}, */

  {   "age",		aedit_age	},
  {   "builder",	aedit_builder	}, /* s removed -- Hugin */
  {   "commands",	show_commands	},
  {   "create",	aedit_create	},
  {   "filename",	aedit_file	},
  {   "name",		aedit_name	},
  /*  {   "recall",	aedit_recall	},   ROM OLC */
  {   "reset",	aedit_reset	},
  {   "security",	aedit_security	},
  {   "show",		aedit_show	},
  {   "vnum",		aedit_vnum	},
  {   "lvnum",	aedit_lvnum	},
  {   "uvnum",	aedit_uvnum	},
  {   "credits",	aedit_credits	},

  // Added by SinaC 2000 for teleport  removed by SinaC 2003, scripts can do that
  //{   "teleport",     aedit_teleport  },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};



const struct olc_cmd_type redit_table[] =
{
  /*  {   command		function	}, */

  {   "commands",	show_commands	},
  {   "create",	redit_create	},
  {   "desc",		redit_desc	},
  {   "ed",		redit_ed	},
  {   "format",	redit_format	},
  {   "name",		redit_name	},
  {   "show",		redit_show	},
  {   "heal",		redit_heal	},
  {   "mana",		redit_mana	},
  // Added by SinaC 2001 for mental user
  {   "psp",		redit_psp	},
  {   "clan",		redit_clan	},
  {   "guild",	redit_guild	}, /* Oxtal */
	

  {   "north",	        redit_north	},
  {   "south",	        redit_south	},
  {   "east",		redit_east	},
  {   "west",		redit_west	},
  {   "up",		redit_up	},
  {   "down",		redit_down	},
  // Added by SinaC 2003
  {   "northeast",	        redit_northeast	},
  {   "northwest",	        redit_northwest	},
  {   "southeast",	        redit_southeast	},
  {   "southwest",	        redit_southwest	},

  // Added by SinaC 2001
  {   "delete",	redit_delete	},  /* <----RIGHT HERE */
  {   "copy",         redit_copy      },

  /* New reset commands. */
  {	"mreset",	redit_mreset	},
  {	"oreset",	redit_oreset	},
  {	"mlist",	redit_mlist	},
  {	"rlist",	redit_rlist	},
  {	"olist",	redit_olist	},
  {	"mshow",	redit_mshow	},
  {	"oshow",	redit_oshow	},
  {     "owner",	redit_owner	},

  // Added by SinaC 2001 for room maximal size
  {     "maxsize",      redit_maxsize   },
  // Added by SinaC 2001 for sector and flag
  {     "flag",         redit_flag      },
  {     "sector",       redit_sector    },

  // Added by SinaC 2003
  {     "program",      redit_program	},
  // Added by SinaC 2003 for repop time
  {     "repop",        redit_repop_time     },

  {   "addaffect",      redit_addaffect}, // SinaC 2003
  {   "delaffect",	redit_delaffect	},
  {   "showaffect",     redit_showaffect },
  {   "setaffect",      redit_setaffect },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};



const struct olc_cmd_type oedit_table[] =
{
  /*  {   command		function	}, */

  {   "addaffect",	oedit_addaffect	},
  {   "addapply",	oedit_addapply	},
  {   "commands",	show_commands	},
  {   "cost",		oedit_cost	},
  {   "create",	        oedit_create	},
  {   "delaffect",	oedit_delaffect	},
  {   "ed",		oedit_ed	},
  {   "long",		oedit_long	},
  {   "name",		oedit_name	},
  {   "short",	        oedit_short	},
  {   "show",		oedit_show	},
  {   "v0",		oedit_value0	},
  {   "v1",		oedit_value1	},
  {   "v2",		oedit_value2	},
  {   "v3",		oedit_value3	},
  {   "v4",		oedit_value4	},  /* ROM */
  {   "weight",	        oedit_weight	},
// Added by SinaC 2000 for general restrictions
  {   "addrestriction", oedit_addrestriction },
  {   "delrestriction", oedit_delrestriction },
// Added by SinaC 2000 for general restrictions
  {   "addskillupgrade", oedit_addskillupgrade },
  {   "delskillupgrade", oedit_delskillupgrade },

  {   "delete",         oedit_delete    }, // Added by SinaC 2000
  {   "copy",           oedit_copy      }, // Added by SinaC 2000
  {   "extra",          oedit_extra     },  /* ROM */
  {   "wear",           oedit_wear      },  /* ROM */
  {   "type",           oedit_type      },  /* ROM */
  {   "material",       oedit_material  },  /* ROM */
  {   "level",          oedit_level     },  /* ROM */
  {   "condition",      oedit_condition },  /* ROM */

  {   "program",        oedit_program	},
  // Added by SinaC 2003, size is not anymore a restriction but an obj stat
  {   "size",           oedit_size      },
  {   "points",         oedit_points    },

  {   "showaffect",     oedit_showaffect },
  {   "setaffect",      oedit_setaffect },

  {   "?",		show_help	},
  {   "version",	show_version	},
  {	NULL,		0,              }
};



const struct olc_cmd_type medit_table[] =
{
  /*  {   command		function	}, */
  {   "alignment",	medit_align	},
  // Added by SinaC 2000 for etho
  {   "etho",	    medit_etho	},
  {   "commands",	    show_commands	},
  {   "create",	    medit_create	},
  {   "desc",	    medit_desc	},
  {   "level",	    medit_level	},
  {   "long",	    medit_long	},
  {   "name",	    medit_name	},
  {   "shop",	    medit_shop	},
  {   "short",	    medit_short	},
  {   "show",	    medit_show	},
  {   "spec",	    medit_spec	},
  {   "program",	    medit_program	},
  {   "delete",       medit_delete    }, // Added by SinaC 2000
  {   "copy",         medit_copy      }, // Added by SinaC 2000
  {   "sex",          medit_sex       },  /* ROM */
  {   "act",          medit_act       },  /* ROM */
  {   "affect",       medit_affect    },  /* ROM */
  // Added by SinaC 2001
  {   "affect2",      medit_affect2   },  /* ROM */
  {   "armor",        medit_ac        },  /* ROM */
  {   "form",         medit_form      },  /* ROM */
  {   "part",         medit_part      },  /* ROM */
  {   "imm",          medit_imm       },  /* ROM */
  {   "res",          medit_res       },  /* ROM */
  {   "vuln",         medit_vuln      },  /* ROM */
  {   "material",     medit_material  },  /* ROM */
  {   "off",          medit_off       },  /* ROM */
  {   "size",         medit_size      },  /* ROM */
  {   "hitdice",      medit_hitdice   },  /* ROM */
  {   "manadice",     medit_manadice  },  /* ROM */
  // Added by SinaC 2001 for mental user
  {   "pspdice",      medit_pspdice   },  /* ROM */
  {   "damdice",      medit_damdice   },  /* ROM */
  {   "race",         medit_race      },  /* ROM */
  {   "classes",      medit_classes   },    // Added by SinaC 2000 for mobile class
  {   "position",     medit_position  },  /* ROM */
  {   "wealth",       medit_gold      },  /* ROM */
  {   "hitroll",      medit_hitroll   },  /* ROM */
  {   "damtype",      medit_damtype   },  /* ROM */
  {   "group",	      medit_group     },  /* ROM */
  {   "?",	      show_help	      },
  {   "version",      show_version    },
  // Added by SinaC 2001 to automatically set hitdice/damdice/hitroll/manadice/ac
  //  for mobiles
  {   "easy",         medit_easy      },
  {   "normal",       medit_normal    },
  {   "hard",         medit_hard      },
  // Added by SinaC 2001 for disease such lycanthropy and plague
  //  WILL BE REMOVED SOON, replaced with UPDATE_FUN and WEAROFF_FUN
  //{   "disease",      medit_disease   },  removed by SinaC 2003
  {   "mrace",        medit_modify_race },
  {   "faction",      medit_faction   },

  {   "addaffect",   medit_addaffect  }, // SinaC 2003
  {   "delaffect",   medit_delaffect  },
  {   "showaffect",  medit_showaffect },
  {   "setaffect",   medit_setaffect },

  {   NULL,	      0,	      }
};

/*****************************************************************************
 *                          End Interpreter Tables.                          *
 *****************************************************************************/



/*****************************************************************************
 Name:		get_area_data
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *get_area_data( int vnum )
{
  AREA_DATA *pArea;

  for (pArea = area_first; pArea; pArea = pArea->next ) {   
    if (pArea->vnum == vnum)
      return pArea;
  }
  
  return 0;
}



/*****************************************************************************
 Name:		edit_done
 Purpose:	Resets builder information on completion.
 Called by:	aedit, redit, oedit, medit(olc.c)
 ****************************************************************************/
bool edit_done( CHAR_DATA *ch ) {
  // Added by SinaC 2000, for BUILDING flag
  if (IS_SET(ch->comm,COMM_BUILDING)) {
    send_to_char("{bBUILDING{x mode removed.\n\r", ch );
    if (buf_string(ch->pcdata->buffer)[0] != '\0' )
      send_to_char("{rYou have received tells: Type {y'replay'{r to see them.{x\n\r",ch);
    REMOVE_BIT(ch->comm, COMM_BUILDING);
  }
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (IS_SET(ch->comm,COMM_EDITING)) {
    send_to_char("{BEDITING{x mode removed.\n\r", ch );
    if (buf_string(ch->pcdata->buffer)[0] != '\0' )
      send_to_char("{rYou have received tells: Type {y'replay'{r to see them.{x\n\r",ch);
    REMOVE_BIT(ch->comm, COMM_EDITING);
  }

  OLC_EDIT_DONE( ch );

  ch->desc->pEdit = NULL;
  ch->desc->editor = 0;
  return FALSE;
}



/*****************************************************************************
 *                              Interpreters.                                *
 *****************************************************************************/


/* Area Interpreter, called by do_aedit. */
void aedit( CHAR_DATA *ch, const char *argument )
{
  AREA_DATA *pArea;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int  cmd;
  int  value;

  EDIT_AREA(ch, pArea);
  //  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "AEdit:  Insufficient security to modify area.\n\r", ch );
    edit_done( ch );
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    interpret( ch, arg );
    return;
  }

  if ( command[0] == '\0' ) {
    aedit_show( ch, argument );
    return;
  }

  if ( ( value = flag_value( area_flags, command ) ) != NO_FLAG ) {
    // SinaC 2000 : seems to be logic
    SET_BIT( pArea->area_flags, AREA_CHANGED );
    TOGGLE_BIT(pArea->area_flags, value);
      
    send_to_char( "Flag toggled.\n\r", ch );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; aedit_table[cmd].name != NULL; cmd++ ) {
    if ( !str_prefix( command, aedit_table[cmd].name ) ) {
      if ( (*aedit_table[cmd].olc_fun) ( ch, argument ) ) {
	SET_BIT( pArea->area_flags, AREA_CHANGED );
	return;
      }
      else
	return;
    }
  }

  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  return;
}



/* Room Interpreter, called by do_redit. */
void redit( CHAR_DATA *ch, const char *argument )
{
  ROOM_INDEX_DATA *pRoom;
  AREA_DATA *pArea;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int  cmd;
  int  value;

  EDIT_ROOM(ch, pRoom);
  pArea = pRoom->area;

  //  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "REdit:  Insufficient security to modify room.\n\r", ch );
    edit_done( ch );
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    recomproom(pRoom);
    return;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    interpret( ch, arg );
    return;
  }

  if ( command[0] == '\0' ) {
    redit_show( ch, argument );
    return;
  }

  /* Removed by SinaC 2001, replaced with redit_flag and redit_sector
  if ( ( value = flag_value( room_flags, command ) ) != NO_FLAG ) {
    TOGGLE_BIT(pRoom->room_flags, value);

    SET_BIT( pArea->area_flags, AREA_CHANGED );
    send_to_char( "Room flag toggled.\n\r", ch );
    return;
  }

  if ( ( value = flag_value( sector_flags, command ) ) != NO_FLAG ) {
    pRoom->sector_type  = value;

    SET_BIT( pArea->area_flags, AREA_CHANGED );
    send_to_char( "Sector type set.\n\r", ch );
    return;
  }
  */

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; redit_table[cmd].name != NULL; cmd++ ) {
    if ( !str_prefix( command, redit_table[cmd].name ) ) {
      if ( (*redit_table[cmd].olc_fun) ( ch, argument ) ) {
	SET_BIT( pArea->area_flags, AREA_CHANGED );
	return;
      }
      else
	return;
    }
  }

  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  return;
}



/* Object Interpreter, called by do_oedit. */
void oedit( CHAR_DATA *ch, const char *argument )
{
  AREA_DATA *pArea;
  OBJ_INDEX_DATA *pObj;
  char arg[MAX_STRING_LENGTH];
  char command[MAX_INPUT_LENGTH];
  int  cmd;
  /*  int  value;   ROM */

  //  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  EDIT_OBJ(ch, pObj);
  pArea = pObj->area;

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "OEdit: Insufficient security to modify area.\n\r", ch );
    edit_done( ch );
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    interpret( ch, arg );
    return;
  }

  if ( command[0] == '\0' ) {
    oedit_show( ch, argument );
    return;
  }
    
  /* Search Table and Dispatch Command. */
  for ( cmd = 0; oedit_table[cmd].name != NULL; cmd++ ) {
    if ( !str_prefix( command, oedit_table[cmd].name ) ) {
      if ( (*oedit_table[cmd].olc_fun) ( ch, argument ) ) {
	SET_BIT( pArea->area_flags, AREA_CHANGED );
	return;
      }
      else
	return;
    }
  }
    
  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  return;
}



/* Mobile Interpreter, called by do_medit. */
void medit( CHAR_DATA *ch, const char *argument )
{
  AREA_DATA *pArea;
  MOB_INDEX_DATA *pMob;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_STRING_LENGTH];
  int  cmd;
  /*  int  value;    ROM */

  //  smash_tilde( argument );
  strcpy( arg, argument );
  argument = one_argument( argument, command );

  EDIT_MOB(ch, pMob);
  pArea = pMob->area;

  if ( !IS_BUILDER( ch, pArea ) ) {
    send_to_char( "MEdit: Insufficient security to modify area.\n\r", ch );
    edit_done( ch );
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( !IS_BUILDER( ch, pArea ) ) {
    interpret( ch, arg );
    return;
  }

  if ( command[0] == '\0' ) {
    medit_show( ch, argument );
    return;
  }

  /* Search Table and Dispatch Command. */
  for ( cmd = 0; medit_table[cmd].name != NULL; cmd++ ) {
    if ( !str_prefix( command, medit_table[cmd].name ) ) {
      if ( (*medit_table[cmd].olc_fun) ( ch, argument ) ) {
	SET_BIT( pArea->area_flags, AREA_CHANGED );
	return;
      }
      else
	return;
    }
  }

  /* Default to Standard Interpreter. */
  interpret( ch, arg );
  return;
}




const struct editor_cmd_type editor_table[] =
{
  /*  {   command		function	}, */

  {   "area",		do_aedit	},
  {   "room",		do_redit	},
  {   "object",	        do_oedit	},
  {   "mobile",	        do_medit	},
  {	NULL,		0,		}
};


/* Entry point for all editors. */
void do_olc( CHAR_DATA *ch, const char *argument )
{
  char command[MAX_INPUT_LENGTH];
  int  cmd;

  argument = one_argument( argument, command );

  if ( command[0] == '\0' ) {
    do_help( ch, "olc" );
    return;
  }
 
  /* Search Table and Dispatch Command. */
  for ( cmd = 0; editor_table[cmd].name != NULL; cmd++ ) {
    if ( !str_prefix( command, editor_table[cmd].name ) ) {
      (*editor_table[cmd].do_fun) ( ch, argument );
      return;
    }
  }

  /* Invalid command, send help. */
  do_help( ch, "olc" );
  return;
}



/* Entry point for editing area_data. */
void do_aedit( CHAR_DATA *ch, const char *argument )
{
  AREA_DATA *pArea;
  int value;
  char value2[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];

  pArea = ch->in_room->area;

  argument = one_argument(argument,arg);
  if ( is_number( arg ) ) {
    value = atoi( arg );
    if ( !( pArea = get_area_data( value ) ) ) {
      send_to_char( "That area vnum does not exist.\n\r", ch );
      return;
    }
  }
  else {
    if ( !str_cmp( arg, "create" ) ) {
      if (!IS_NPC(ch) && (ch->pcdata->security < 9) ) {
	send_to_char("Insufficient security to create area.\n\r",ch);
	return;
      }
      argument    	=   one_argument(argument,value2);
      value = atoi (value2);
      if (get_area_data(value) != NULL) {
	send_to_char("This area already exists!",ch);
	return;
      }
      pArea               =   new_area();
      area_last->next     =   pArea;
      area_last		=   pArea;	/* Thanks, Walker. */
      SET_BIT( pArea->area_flags, AREA_ADDED );
      send_to_char("Area created.\n\r",ch);
    }
  }

  if (!IS_BUILDER(ch,pArea)) {
    send_to_char("Insufficient security to modify area.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  REMOVE_BIT(ch->comm, COMM_EDITING);
  if (!IS_SET(ch->comm,COMM_BUILDING))
    send_to_char("You are now in {bBUILDING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_BUILDING);
    
  ch->desc->pEdit = (void *)pArea;
  ch->desc->editor = ED_AREA;
  return;
}



/* Entry point for editing room_index_data. */
void do_redit( CHAR_DATA *ch, const char *argument )
{
  ROOM_INDEX_DATA *pRoom, *pRoom2;
  char arg1[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1 );

  pRoom = ch->in_room;

  if ( !str_cmp( arg1, "reset" ) ) {
    if ( !IS_BUILDER( ch, pRoom->area ) ) {
      send_to_char( "Insufficient security to modify room.\n\r" , ch );
      return;
    }

    reset_room( pRoom );
    send_to_char( "Room reset.\n\r", ch );
    return;
  }
  else
    if ( !str_cmp( arg1, "create" ) ) {
      if ( argument[0] == '\0' || atoi( argument ) == 0 ) {
	send_to_char( "Syntax:  edit room create [vnum]\n\r", ch );
	return;
      }
      
      if ( redit_create( ch, argument ) ) {
	char_from_room( ch );
	char_to_room( ch, (ROOM_INDEX_DATA*) ch->desc->pEdit );
	SET_BIT( pRoom->area->area_flags, AREA_CHANGED );
	pRoom = ch->in_room;
      }
    }
  // Added by SinaC 2001
    else if ( !str_cmp( arg1, "delete" ) ) {
      oedit_delete( ch, argument );
      return;
    }
    else {
      pRoom2 = get_room_index(atoi(arg1));
      
      if ( (pRoom2 != NULL) && IS_BUILDER(ch,pRoom2->area) ) {
	char_from_room( ch );
	char_to_room( ch, pRoom2 );
	pRoom = ch->in_room;
      }
      else
	if (atoi(arg1) != 0) {
	  send_to_char("Insufficient security to modify room, or room doesn't exist.\n\r",ch);
	  return;
	}   
    }
  
  if ( !IS_BUILDER( ch, pRoom->area ) ) {
    send_to_char( "Insufficient security to modify room.\n\r" , ch );
    return;
  }
  
  // Added by SinaC 2000
  REMOVE_BIT(ch->comm, COMM_EDITING);
  if (!IS_SET(ch->comm,COMM_BUILDING))
    send_to_char("You are now in {bBUILDING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_BUILDING);
  
  ch->desc->editor = ED_ROOM;
  return;
}



/* Entry point for editing obj_index_data. */
void do_oedit( CHAR_DATA *ch, const char *argument )
{
  OBJ_INDEX_DATA *pObj;
  AREA_DATA *pArea;
  char arg1[MAX_STRING_LENGTH];
  int value;

  if ( IS_NPC(ch) )
    return;

  argument = one_argument( argument, arg1 );

  if ( is_number( arg1 ) ) {
    value = atoi( arg1 );
    if ( !( pObj = get_obj_index( value ) ) ) {
      send_to_char( "OEdit:  That vnum does not exist.\n\r", ch );
      return;
    }

    if ( !IS_BUILDER( ch, pObj->area ) ) {
      send_to_char( "Insufficient security to modify object.\n\r" , ch );
      return;
    }

    // Added by SinaC 2000
    REMOVE_BIT(ch->comm, COMM_EDITING);
    if (!IS_SET(ch->comm,COMM_BUILDING))
      send_to_char("You are now in {bBUILDING{x mode.\n\r",ch);
    SET_BIT(ch->comm, COMM_BUILDING);

    ch->desc->pEdit = (void *)pObj;
    ch->desc->editor = ED_OBJECT;
    return;
  }
  else {
    // Added by SinaC 2001
    if ( !str_cmp( arg1, "delete" ) ) {
      oedit_delete( ch, argument );
      return;
    } 
    else if ( !str_cmp( arg1, "create" ) ) {
      value = atoi( argument );
      if ( argument[0] == '\0' || value == 0 ) {
	send_to_char( "Syntax:  edit object create [vnum]\n\r", ch );
	return;
      }

      pArea = get_vnum_area( value );

      if ( !pArea ) {
	send_to_char( "OEdit:  That vnum is not assigned an area.\n\r", ch );
	return;
      }

      if ( !IS_BUILDER( ch, pArea ) ) {
	send_to_char( "Insufficient security to modify object.\n\r" , ch );
	return;
      }

      if ( oedit_create( ch, argument ) ) {
	SET_BIT( pArea->area_flags, AREA_CHANGED );

	// Added by SinaC 2000
	REMOVE_BIT(ch->comm, COMM_EDITING);
	if (!IS_SET(ch->comm,COMM_BUILDING))
	  send_to_char("You are now in {bBUILDING{x mode.\n\r",ch);
	SET_BIT(ch->comm, COMM_BUILDING);
		
	ch->desc->editor = ED_OBJECT;
      }

      return;
    }
  }

  send_to_char( "OEdit:  There is no default object to edit.\n\r", ch );
  return;
}



/* Entry point for editing mob_index_data. */
void do_medit( CHAR_DATA *ch, const char *argument )
{
  MOB_INDEX_DATA *pMob;
  AREA_DATA *pArea;
  int value;
  char arg1[MAX_STRING_LENGTH];

  argument = one_argument( argument, arg1 );

  if ( is_number( arg1 ) ) {
    value = atoi( arg1 );
    if ( !( pMob = get_mob_index( value ) )) {
      send_to_char( "MEdit:  That vnum does not exist.\n\r", ch );
      return;
    }
    if ( !IS_BUILDER( ch, pMob->area ) ) {
      send_to_char( "Insufficient security to modify mobs.\n\r" , ch );
      return;
    }

    // Added by SinaC 2000
    REMOVE_BIT(ch->comm, COMM_EDITING);
    if (!IS_SET(ch->comm,COMM_BUILDING))
      send_to_char("You are now in {bBUILDING{x mode.\n\r",ch);
    SET_BIT(ch->comm, COMM_BUILDING);

    ch->desc->pEdit = (void *)pMob;
    ch->desc->editor = ED_MOBILE;
    return;
  }
  else {
    if ( !str_cmp( arg1, "delete" ) ) {
      medit_delete( ch, argument );
      return;
    } 
    else if ( !str_cmp( arg1, "create" ) ) {
      value = atoi( argument );
      if ( arg1[0] == '\0' || value == 0 ) {
	send_to_char( "Syntax:  edit mobile create [vnum]\n\r", ch );
	return;
      }
      
      pArea = get_vnum_area( value );
      
      if ( !pArea ) {
	send_to_char( "MEdit:  That vnum is not assigned an area.\n\r", ch );
	return;
      }
      
      if ( !IS_BUILDER( ch, pArea ) ) {
	send_to_char( "Insufficient security to modify mobs.\n\r" , ch );
	return;
      }
      
      if ( medit_create( ch, argument ) ) {
	// Added by SinaC 2000
	REMOVE_BIT(ch->comm, COMM_EDITING);
	if (!IS_SET(ch->comm,COMM_BUILDING))
	  send_to_char("You are now in {bBUILDING{x mode.\n\r",ch);
	SET_BIT(ch->comm, COMM_BUILDING);
	
	SET_BIT( pArea->area_flags, AREA_CHANGED );
	ch->desc->editor = ED_MOBILE;
      }
      return;
    }
  }

  send_to_char( "MEdit:  There is no default mobile to edit.\n\r", ch );
  return;
}



void display_resets( CHAR_DATA *ch )
{
  ROOM_INDEX_DATA	*pRoom;
  RESET_DATA		*pReset;
  MOB_INDEX_DATA	*pMob = NULL;
  char 		buf   [ MAX_STRING_LENGTH ];
  char 		final [ MAX_STRING_LENGTH ];
  int 		iReset = 0;

  EDIT_ROOM(ch, pRoom);
  final[0]  = '\0';
    
  send_to_char ( 
		" No.  Loads    Description       Location         Vnum   Mx Mn Description"
		"\n\r"
		"==== ======== ============= =================== ======== ===== ==========="
		"\n\r", ch );

  for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
    OBJ_INDEX_DATA  *pObj;
    MOB_INDEX_DATA  *pMobIndex;
    OBJ_INDEX_DATA  *pObjIndex;
    OBJ_INDEX_DATA  *pObjToIndex;
    ROOM_INDEX_DATA *pRoomIndex;

    final[0] = '\0';
    sprintf( final, "[%2d] ", ++iReset );

    switch ( pReset->command ) {
    default:
      sprintf( buf, "Bad reset command: %c.\n\r", pReset->command );
      strcat( final, buf );
      break;

    case 'M':
      if ( !( pMobIndex = get_mob_index( pReset->arg1 ) ) ) {
	sprintf( buf, "Load Mobile - Bad Mob %d\n\r", pReset->arg1 );
	strcat( final, buf );
	continue;
      }

      if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) ) {
	sprintf( buf, "Load Mobile - Bad Room %d\n\r", pReset->arg3 );
	strcat( final, buf );
	continue;
      }

      pMob = pMobIndex;
      sprintf( buf, "M[%5d] %-13.13s{x in room             R[%5d] %2d/%2d %-15.15s{x\n\r",
	       pReset->arg1, pMob->short_descr, pReset->arg3,
	       pReset->arg2, pReset->arg4, pRoomIndex->name );
      strcat( final, buf );

      // Remove by SinaC 2003
      /*
       * Check for pet shop.
       * -------------------
       */
      //      {
      //	ROOM_INDEX_DATA *pRoomIndexPrev;
      //
      //	pRoomIndexPrev = get_room_index( pRoomIndex->vnum - 1 );
      //	if ( pRoomIndexPrev
      //     // Modified by SinaC 2001
      //	     && IS_SET( pRoomIndexPrev->bstat(flags), ROOM_PET_SHOP ) )
      //	  final[5] = 'P';
      //      }

      break;

    case 'O':
      if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
	sprintf( buf, "Load Object - Bad Object %d\n\r",
		 pReset->arg1 );
	strcat( final, buf );
	continue;
      }

      pObj       = pObjIndex;

      if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) ) {
	sprintf( buf, "Load Object - Bad Room %d\n\r", pReset->arg3 );
	strcat( final, buf );
	continue;
      }

      sprintf( buf, "O[%5d] %-13.13s{x in room             "
	       "R[%5d]       %-15.15s{x\n\r",
	       pReset->arg1, pObj->short_descr,
	       pReset->arg3, pRoomIndex->name );
      strcat( final, buf );

      break;

    case 'P':
      if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
	sprintf( buf, "Put Object - Bad Object %d\n\r",
		 pReset->arg1 );
	strcat( final, buf );
	continue;
      }

      pObj       = pObjIndex;

      if ( !( pObjToIndex = get_obj_index( pReset->arg3 ) ) ) {
	sprintf( buf, "Put Object - Bad To Object %d\n\r",
		 pReset->arg3 );
	strcat( final, buf );
	continue;
      }

      sprintf( buf,
	       "O[%5d] %-13.13s{x inside              O[%5d] %2d/%2d %-15.15s{x\n\r",
	       pReset->arg1,
	       pObj->short_descr,
	       pReset->arg3,
	       pReset->arg2,
	       pReset->arg4,
	       pObjToIndex->short_descr );
      strcat( final, buf );

      break;

    case 'G':
    case 'E':
      if ( !( pObjIndex = get_obj_index( pReset->arg1 ) ) ) {
	sprintf( buf, "Give/Equip Object - Bad Object %d\n\r",
		 pReset->arg1 );
	strcat( final, buf );
	continue;
      }

      pObj       = pObjIndex;

      if ( !pMob ) {
	sprintf( buf, "Give/Equip Object - No Previous Mobile\n\r" );
	strcat( final, buf );
	break;
      }

      if ( pMob->pShop ) {
	sprintf( buf,
		 "O[%5d] %-13.13s{x in the inventory of S[%5d]       %-15.15s{x\n\r",
		 pReset->arg1,
		 pObj->short_descr,                           
		 pMob->vnum,
		 pMob->short_descr  );
      }
      else
	sprintf( buf,
		 "O[%5d] %-13.13s{x %-19.19s M[%5d]       %-15.15s{x\n\r",
		 pReset->arg1,
		 pObj->short_descr,
		 (pReset->command == 'G') ?
		 flag_string( wear_loc_strings, WEAR_NONE )
		 : flag_string( wear_loc_strings, pReset->arg3 ),
		 pMob->vnum,
		 pMob->short_descr );
      strcat( final, buf );

      break;

      /*
       * Doors are set in rs_flags don't need to be displayed.
       * If you want to display them then uncomment the new_reset
       * line in the case 'D' in load_resets in db.c and here.
       */
    case 'D':
      pRoomIndex = get_room_index( pReset->arg1 );
      sprintf( buf, "R[%5d] %s door of %-19.19s reset to %s\n\r",
	       pReset->arg1,
	       capitalize( dir_name[ pReset->arg2 ] ),
	       pRoomIndex->name,
	       flag_string( door_resets, pReset->arg3 ) );
      strcat( final, buf );

      break;
      /*
       * End Doors Comment.
       */
    case 'R':
      if ( !( pRoomIndex = get_room_index( pReset->arg1 ) ) ) {
	sprintf( buf, "Randomize Exits - Bad Room %d\n\r",
		 pReset->arg1 );
	strcat( final, buf );
	continue;
      }

      sprintf( buf, "R[%5d] Exits are randomized in %s\n\r",
	       pReset->arg1, pRoomIndex->name );
      strcat( final, buf );

      break;
      // Added by SinaC and JyP ( aka Oxtal ) 2000 for random maze
    case 'Z':
      if ( !( pRoomIndex = get_room_index( pReset->arg3 ) ) ) {
	sprintf(buf, "Random Maze - Bad Room %d\n\r",
		pReset->arg3 );
	strcat( final, buf );
	continue;
      }

      sprintf(buf, "Z[%5d] Random maze   (%dx%d) from %d to %d  map: %d.\n\r",
	      pReset->arg3, pReset->arg1, pReset->arg2,
	      pReset->arg3,
	      pReset->arg3 + pReset->arg1 * pReset->arg2-1,
	      pReset->arg4);
      strcat( final, buf );

      break;
    }
    send_to_char( final, ch );
  }

  return;
}



/*****************************************************************************
 Name:		add_reset
 Purpose:	Inserts a new reset in the given index slot.
 Called by:	do_resets(olc.c).
 ****************************************************************************/
void add_reset( ROOM_INDEX_DATA *room, RESET_DATA *pReset, int index )
{
  RESET_DATA *reset;
  int iReset = 0;

  if ( !room->reset_first ) {
    room->reset_first	= pReset;
    room->reset_last	= pReset;
    pReset->next		= NULL;
    return;
  }

  index--;

  if ( index == 0 ) {	/* First slot (1) selected. */
    pReset->next = room->reset_first;
    room->reset_first = pReset;
    return;
  }

  /*
   * If negative slot( <= 0 selected) then this will find the last.
   */
  for ( reset = room->reset_first; reset->next; reset = reset->next ) {
    if ( ++iReset == index )
      break;
  }

  pReset->next	= reset->next;
  reset->next		= pReset;
  if ( !pReset->next )
    room->reset_last = pReset;
  return;
}

// Try to find which mob is in reset before index
MOB_INDEX_DATA *find_related_mob( ROOM_INDEX_DATA *pRoomIndex, const int index ) {
  int vnum = 0;
  int iReset = 0;
  // Run through room's resets and find mob reset before index
  for ( RESET_DATA *pReset = pRoomIndex->reset_first; pReset != NULL; pReset = pReset->next ) {
    if ( pReset->command == 'M' && iReset+1 < index )
      vnum = pReset->arg1; // arg1 is the mob vnum
    if ( ++iReset == index )
      break;
  }
  if ( vnum > 0 )
    return get_mob_index( vnum );
  return NULL;
}

void do_resets( CHAR_DATA *ch, const char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  char arg5[MAX_INPUT_LENGTH];
  char arg6[MAX_INPUT_LENGTH];
  char arg7[MAX_INPUT_LENGTH];
  RESET_DATA *pReset = NULL;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg4 );
  argument = one_argument( argument, arg5 );
  argument = one_argument( argument, arg6 );
  argument = one_argument( argument, arg7 );

  if ( !IS_BUILDER( ch, ch->in_room->area ) ) {
    send_to_char( "Resets: Invalid security for editing this area.\n\r",
		  ch );
    return;
  }

  /*
   * Display resets in current room.
   * -------------------------------
   */
  if ( arg1[0] == '\0' ) {
    if ( ch->in_room->reset_first ) {
      send_to_char(
		   "Resets: M = mobile, R = room, O = object, "
		   "P = pet, S = shopkeeper\n\r", ch );
      display_resets( ch );
    }
    else
      send_to_char( "No resets in this room.\n\r", ch );
  }


  /*
   * Take index number and search for commands.
   * ------------------------------------------
   */
  if ( is_number( arg1 ) ) {
    ROOM_INDEX_DATA *pRoom = ch->in_room;

    /*
     * Delete a reset.
     * ---------------
     */
    if ( !str_cmp( arg2, "delete" ) ) {
      int insert_loc = atoi( arg1 );

      if ( !ch->in_room->reset_first ) {
	send_to_char( "No resets in this area.\n\r", ch );
	return;
      }

      if ( insert_loc-1 <= 0 ) {

	pReset = pRoom->reset_first;
	pRoom->reset_first = pRoom->reset_first->next;
	if ( !pRoom->reset_first )
	  pRoom->reset_last = NULL;
      }
      else {
	int iReset = 0;
	RESET_DATA *prev = NULL;

	for ( pReset = pRoom->reset_first;
	      pReset;
	      pReset = pReset->next ) {
	  if ( ++iReset == insert_loc )
	    break;
	  prev = pReset;
	}

	if ( !pReset ) {
	  send_to_char( "Reset not found.\n\r", ch );
	  return;
	}

	if ( prev )
	  prev->next = prev->next->next;
	else
	  pRoom->reset_first = pRoom->reset_first->next;

	for ( pRoom->reset_last = pRoom->reset_first;
	      pRoom->reset_last->next;
	      pRoom->reset_last = pRoom->reset_last->next );
      }

      // Added by SinaC 2000, so when we modify a reset, the area
      //  is considered as changed
      SET_BIT( pRoom->area->area_flags, AREA_CHANGED );

      send_to_char( "Reset deleted.\n\r", ch );
    }
    else
      /*
       * Add a reset.
       * ------------
       */
      if ( (!str_cmp( arg2, "mob" ) && is_number( arg3 ))
	   || (!str_cmp( arg2, "obj" ) && is_number( arg3 )) ) {
	/*
	 * Check for Mobile reset.
	 * -----------------------
	 */
	if ( !str_cmp( arg2, "mob" ) ) {
	  if (get_mob_index( is_number(arg3) ? atoi( arg3 ) : 1 ) == NULL) {
	    send_to_char("Mob doesn't exists.\n\r",ch);
	    return;
	  }
	  pReset = new_reset_data();
	  pReset->command = 'M';
	  pReset->arg1    = atoi( arg3 );
	  pReset->arg2    = is_number( arg4 ) ? atoi( arg4 ) : 1; // Max #
	  pReset->arg3    = ch->in_room->vnum;
	  pReset->arg4	= is_number( arg5 ) ? atoi( arg5 ) : 1; // Min #
	}
	else
	  /*
	   * Check for Object reset.
	   * -----------------------
	   */
	  if ( !str_cmp( arg2, "obj" ) ) {
	    OBJ_INDEX_DATA *pObjIndex;

	    /* Removed by SinaC 2001 to avoid a memory leak
	    pReset = new_reset_data();
	    pReset->arg1    = atoi( arg3 );
	    */
	    // Added by SinaC 2001 to fix the memory leak
	    int rarg1 = atoi(arg3), 
	      rarg2 = 0,
	      rarg3 = 0,
	      rarg4 = 0;
	    char rcommand = 0;
	    /*
	     * Inside another object.
	     * ----------------------
	     */
	    if ( !str_prefix( arg4, "inside" ) ) {
	      OBJ_INDEX_DATA *temp;

	      temp = get_obj_index(is_number(arg5) ? atoi(arg5) : 1);
	      // SinaC 2000
	      if (!temp) {/* fix by kalahn@pobox.com */
		send_to_char( "Couldn't find Object 2.\n\r",ch);
		return;      
	      }
	      if ( ( temp->item_type != ITEM_CONTAINER ) 
		   && ( temp->item_type != ITEM_CORPSE_NPC ) ) {
		send_to_char( "Object 2 isn't a container.\n\r", ch);
		return;
	      }
	      // Added by SinaC 2001
	      if ( IS_SET( temp->extra_flags, ITEM_UNIQUE ) ) {
		send_to_char("You can't reset a unique item.\n\r", ch );
		return;
	      }

	      rcommand = 'P';
	      // maximum identical item in the mud
	      rarg2    = is_number( arg6 ) ? atoi( arg6 ) : -1;
	      // vnum of container
	      rarg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
	      // minimum item
	      rarg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
	      /* 
		 pReset->command = 'P';
		 //pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : 1;
		 // Modified by SinaC 2001
		 // maximum identical item in the mud
		 pReset->arg2    = is_number( arg6 ) ? atoi( arg6 ) : -1;
		 // vnum of container
		 pReset->arg3    = is_number( arg5 ) ? atoi( arg5 ) : 1;
		 // minimum item
		 pReset->arg4    = is_number( arg7 ) ? atoi( arg7 ) : 1;
	      */
	    }
	    else
	      /*
	       * Inside the room.
	       * ----------------
	       */
	      if ( !str_cmp( arg4, "room" ) ) {
		// Modified by SinaC 2001
		if ((pObjIndex=get_obj_index(atoi(arg3))) == NULL) {
		  send_to_char( "Vnum doesn't exist.\n\r",ch);
		  return;
		}
		// Added by SinaC 2001
		if ( IS_SET( pObjIndex->extra_flags, ITEM_UNIQUE ) ) {
		  send_to_char("You can't reset a unique item.\n\r", ch );
		  return;
		}
		rcommand = 'O';
		rarg2     = 0;
		rarg3     = ch->in_room->vnum;
		rarg4     = 0;
		/* Removed by SinaC 2001 for avoid a memory leak
		   pReset->command  = 'O';
		   pReset->arg2     = 0;
		   pReset->arg3     = ch->in_room->vnum;
		   pReset->arg4     = 0;
		*/
	      }
	      else {
		/*
		 * Into a Mobile's inventory.
		 * --------------------------
		 */
		if ( flag_value( wear_loc_flags, arg4 ) == NO_FLAG ) {
		  send_to_char( "Resets: '? wear-loc'\n\r", ch );
		  return;
		}
		// Modified by SinaC 2001
		if ((pObjIndex=get_obj_index(atoi(arg3))) == NULL) {
		  send_to_char( "Vnum doesn't exist.\n\r",ch);
		  return;
		}
		// Added by SinaC 2001
		if ( IS_SET( pObjIndex->extra_flags, ITEM_UNIQUE ) ) {
		  send_to_char("You can't reset a unique item.\n\r", ch );
		  return;
		}

		rarg1 = atoi(arg3);
		rarg3 = flag_value( wear_loc_flags, arg4 );
		if ( !is_wearable( pObjIndex, rarg3 ) ) {
		  send_to_char("That item can't be worn at that location.\n\r", ch );
		  return;
		}
		MOB_INDEX_DATA *relatedMob = find_related_mob( ch->in_room, atoi( arg1 ) );
		if ( relatedMob == NULL ) {
		  send_to_charf(ch,"No mob can be found before this reset.\n\r");
		  return;
		}
		if ( check_needed_parts( relatedMob->parts, rarg3 ) ) {
		  send_to_charf( ch, "Mob [vnum: %d] needs parts (%s) in order to wear Obj [vnum: %d]\n",
				 relatedMob->vnum,
				 needed_parts_string(rarg3),
				 pObjIndex->vnum );
		  return;
		}
		if ( check_needed_forms( relatedMob->form, rarg3 ) ) {
		  send_to_charf( ch, "Mob [vnum: %d] needs forms (%s) in order to wear Obj [vnum: %d]\n",
				 relatedMob->vnum,
				 needed_forms_string(rarg3),
				 pObjIndex->vnum );
		  return;
		}

		// Check size and restriction, we need to create a mob of this vnum and an obj of this vnum
		CHAR_DATA *tmp_mob = create_mobile( relatedMob );
		small_recompute( tmp_mob ); // recompute is necessary because we don't put mob in a room
		OBJ_DATA *tmp_obj = create_object( pObjIndex, 0 );
		if ( !check_restriction( tmp_mob, tmp_obj ) ) {
		  send_to_charf(ch," Mob [vnum: %d] doesn't meet Obj [vnum: %d] requirements\n",
				relatedMob->vnum,
				pObjIndex->vnum );
		  return;
		}
		if ( !check_size( tmp_mob, tmp_obj, TRUE ) ) {
		  send_to_charf(ch," Mob [vnum: %d] doesn't meet Obj [vnum: %d] size requirement [%s]\n",
				relatedMob->vnum,
				pObjIndex->vnum,
				flag_string( size_flags, pObjIndex->size ) );
		  return;
		}
		extract_char( tmp_mob, TRUE, TRUE );
		extract_obj( tmp_obj );
		// end of size & restriction

		if ( ( IS_SET( pObjIndex->extra_flags, ITEM_ANTI_EVIL ) && relatedMob->align.alignment <= -350 )
		     || ( IS_SET( pObjIndex->extra_flags, ITEM_ANTI_GOOD ) && relatedMob->align.alignment >= 350 )
		     || ( IS_SET( pObjIndex->extra_flags, ITEM_ANTI_NEUTRAL ) && relatedMob->align.alignment > -350 && relatedMob->align.alignment < 350 ) ) {
		  send_to_charf( ch, "Mob [vnum: %d] alignment [%d] doesn't meet Obj [vnum %d] anti-alignment extra flags: %s\n", relatedMob->vnum, relatedMob->align.alignment, pObjIndex->vnum, flag_string( extra_flags, pObjIndex->extra_flags ) );
		  return;
		}
		if ( rarg3 == WEAR_NONE )
		  rcommand = 'G';
		else
		  rcommand = 'E';
		/* Removed by SinaC 2001 to avoid a memory leak
		   pReset->arg1 = atoi(arg3);
		   pReset->arg3 = flag_value( wear_loc_flags, arg4 );
		   if ( !is_wearable( pObjIndex, pReset->arg3 ) ) {
		   send_to_char("That item can't be worn at that location.\n\r", ch );
		   return;
		   }
		   if ( pReset->arg3 == WEAR_NONE )
		   pReset->command = 'G';
		   else
		   pReset->command = 'E';
		*/
	      }
	    // Added by SinaC 2001 to avoid a memory leak
	    pReset = new_reset_data();
	    pReset->command = rcommand;
	    pReset->arg1    = rarg1;
	    pReset->arg2    = rarg2;
	    pReset->arg3    = rarg3;	
	    pReset->arg4    = rarg4;
	  }

	add_reset( ch->in_room, pReset, atoi( arg1 ) );
	SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	send_to_char( "Reset added.\n\r", ch );
      }
      else if (!str_cmp( arg2, "random") && is_number(arg3)) {
	if (atoi(arg3) < 1 || atoi(arg3) > 6) {
	  send_to_char("Invalid argument.\n\r", ch);
	  return;
	}
	pReset = new_reset_data ();
	pReset->command = 'R';
	pReset->arg1 = ch->in_room->vnum;
	pReset->arg2 = atoi(arg3);
	add_reset( ch->in_room, pReset, atoi( arg1 ) );
	SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	send_to_char( "Random exits reset added.\n\r", ch);
      }
    // Added by SinaC and JyP ( aka oxtal ) for random maze
      else if (!str_cmp( arg2, "maze") && is_number(arg3) && is_number(arg4)) {
	int width, height, vnum_map;
	ROOM_INDEX_DATA *pRoomIndex;
 
	width = atoi(arg3); height = atoi(arg4);
	vnum_map = atoi(arg5);
	if ( vnum_map != 0 && !get_obj_index(vnum_map)) {
	  send_to_char("Map vnum invalid!\n\r",ch);
	  return;
	}
	if ( width < 2 || height < 2 || width * height > 100 ) {
	  send_to_char("Invalid argument, width and height have to be greater than 1 and the maximum size for a maze is 100\n\r",ch);
	  return;
	}
	if ( width * height + ch->in_room->vnum > ch->in_room->area->max_vnum ) {
	  send_to_char( "MaZe too large!",ch);
	  return;
	}
	for ( int i = ch->in_room->vnum; 
	      i < ch->in_room->vnum+ width * height; 
	      i++ ) {
	  if ( ( pRoomIndex = get_room_index( i ) ) == NULL ) {
	    send_to_charf( ch,"MaZe: covers an inexisting room(%d)\n\r",i);
	    return;
	  }
	}
	    
	pReset = new_reset_data();
	pReset->command = 'Z';
	pReset->arg1 = width;
	pReset->arg2 = height;
	pReset->arg3 = ch->in_room->vnum;
	pReset->arg4 = vnum_map;
	add_reset( ch->in_room, pReset, atoi( arg1 ) );
	SET_BIT( ch->in_room->area->area_flags, AREA_CHANGED );
	send_to_charf( ch, "A maze has been set from vnum: %d to vnum: %d with map (vnum %d).\n\r",
		       ch->in_room->vnum, 
		       ch->in_room->vnum + width*height-1,
		       vnum_map);
      }
      else
	{
	  send_to_char( "Syntax: RESET <number> OBJ <vnum> <wear_loc>\n\r", ch );
	  send_to_char( "        RESET <number> OBJ <vnum> inside <vnum> [limit] [count]\n\r", ch );
	  send_to_char( "        RESET <number> OBJ <vnum> room\n\r", ch );
	  send_to_char( "        RESET <number> MOB <vnum> [max #x area] [max #x room]\n\r", ch );
	  send_to_char( "        RESET <number> RANDOM [#x exits]\n\r", ch);
	  // Added by SinaC and JyP ( aka oxtal ) for random maze
	  send_to_char( "        RESET <number> MAZE <width> <height> [map vnum]\n\r", ch );
	  send_to_char( "        RESET <number> DELETE\n\r", ch );
	}
  }

  return;
}



/*****************************************************************************
 Name:		do_alist
 Purpose:	Normal command to list areas and display area information.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_alist( CHAR_DATA *ch, const char *argument )
{
  BUFFER *output;
  char buf    [ MAX_STRING_LENGTH ];
  AREA_DATA *pArea;

  output = new_buf();

  // Modified by SinaC 2003
  //sprintf( buf, "[%3s] [%-25s] (%-5s-%5s) [%-10s] %3s [%-10s] %4s\n\r",
  //	   "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders","Ply" );
  sprintf( buf, "[%3s] [%-25s] (%-5s-%5s) [%-10s] %3s [%-10s] %5s\n\r",
  	   "Num", "Area Name", "lvnum", "uvnum", "Filename", "Sec", "Builders","Plyd" );

  add_buf(output,buf);
    
  for ( pArea = area_first; pArea; pArea = pArea->next ) {
    // Modified by SinaC 2003
    //sprintf( buf, "[%3d] %-25.25s (%-5d-%5d) %-12.12s [%d] [%-10.10s] %2.1f\n\r",
    sprintf( buf, "[%3d] %-25.25s (%-5d-%5d) %-12.12s [%d] [%-10.10s] %3.2f\n\r",
	     pArea->vnum,
	     pArea->name,
	     pArea->min_vnum,
	     pArea->max_vnum,
	     pArea->file_name,
	     pArea->security,
	     pArea->builders,
	     /*Oxtal*/  ticks_elapsed ? ((float) pArea->totalnplayer ) / ticks_elapsed : 0.0
	     );
    //strcat( result, buf );
    add_buf( output, buf );
  }

  //    send_to_char( result, ch );
  page_to_char(buf_string(output),ch);
  return;
}

// Added by SinaC 2000 so we can see the help about olc tables without editing
void do_olchelp( CHAR_DATA *ch, const char *argument )
{
  show_help( ch, argument );
}
