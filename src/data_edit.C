/***************************************************************************
 *  File: data_edit.c                                                      *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 *                                                                         *
 *  SinaC 2003                                                             *
 *                                                                         *
 ***************************************************************************/

// TO DO
//  _edit_create/delete for ability(->scriptAbility)/race/pcrace/classes/god/faction
//  hometown edition


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
#include "data_edit.h"
#include "interp.h"
#include "comm.h"
#include "lookup.h"
#include "db.h"
#include "names.h"
#include "bit.h"
#include "wearoff_affect.h"
#include "update_affect.h"
#include "const.h"
#include "tables.h"
#include "handler.h"
#include "prereqs.h"
#include "language.h"
#include "olc_value.h"
#include "data.h"
#include "group.h"
#include "classes.h"
#include "string.h"
#include "prereqs.h"
#include "act_info.h"
#include "brew.h"
#include "faction.h"
#include "olc_act.h"
#include "config.h"
#include "recycle.h"
//#include "ability.h"
#include "magic_schools.h"
#include "super_races.h"


int DATA_MINIMUM_SECURITY;

// Only '_' and letters are allowed
bool is_valid_name( const char *name ) {
  const char *p = name;
  while ( *p != '\0' ) {
    if ( !isalpha( *p ) && *p != '_' )
      return FALSE;
    p++;
  }
  return TRUE;
}

// General edit functions
// Classes
bool edit_classes( CHAR_DATA *ch, int &allowed_class, const char *command, const char *argument ) {
  int cl = flag_value( classes_flags, argument );
  if ( argument[0] == '\0'
       || cl == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  %s [classes name]\n\r"
		  "Type '? classes' for a list of classes.\n\r", command );
    return FALSE;
  }
  allowed_class ^= cl;
  send_to_charf(ch,"%s toggled.\n\r", command);
  return TRUE;
}
void show_classes( CHAR_DATA *ch, int allowed_class, const char *command ) {
  send_to_charf( ch, "%s:          ", command );
  if ( allowed_class == 0 )
    send_to_charf( ch, "No %s\n\r", command );
  else {
    int count = 0;
    for ( int i = 0; i < MAX_CLASS; i++ )
      if ( ( 1 << i ) & allowed_class ) {
	if ( count > 0 && count % 2 == 0 ) send_to_charf( ch, "\n\r                  " );
	send_to_charf( ch, "[%19s] ", class_table[i].name );
	count++;
      }
    send_to_charf( ch, "\n\r");
  }
}
// Alignment
bool edit_alignment( CHAR_DATA *ch, ALIGN_INFO *&allowed_align, int &nb_allowed_align, const char *argument ) {
  ALIGN_INFO align;
  if ( argument[0] == '\0'
       || convert_etho_align_string_to_align_info( argument, align ) == -1 ) {
    send_to_charf(ch,"Syntax:  alignment [lawful/neutral/chaotic] [good/neutral/evil]\n\r");
    return FALSE;
  }
  int aIndex;
  bool found = FALSE;
  for ( aIndex = 0; aIndex < nb_allowed_align; aIndex++ )
    if ( allowed_align[aIndex].etho == align.etho
	 && allowed_align[aIndex].alignment == align.alignment ) {
      found = TRUE;
      break;
    }
  if ( found ) { // found -> remove
    ALIGN_INFO *tmp;
    if ( ( tmp = (ALIGN_INFO*) GC_MALLOC( sizeof(ALIGN_INFO) * (nb_allowed_align-1) ) ) == NULL ) {
      bug("Can't allocate memory for alignment");
      return FALSE;
    }
    int count = 0;
    for ( int i = 0; i < nb_allowed_align; i++ )
      if ( aIndex != i ) {
	tmp[count].etho = allowed_align[i].etho;
	tmp[count].alignment = allowed_align[i].alignment;
	count++;
      }
    nb_allowed_align--;
    allowed_align = tmp;
    send_to_charf(ch,"Alignment removed.\n\r");
  }
  else {         // not found -> add
    ALIGN_INFO *tmp;
    if ( ( tmp = (ALIGN_INFO*) GC_MALLOC( sizeof(ALIGN_INFO) * (nb_allowed_align+1) ) ) == NULL ) {
      bug("Can't allocate memory for alignment");
      return FALSE;
    }
    bcopy( allowed_align, tmp, nb_allowed_align*sizeof(ALIGN_INFO));
    allowed_align = tmp;
    allowed_align[nb_allowed_align].etho = align.etho;
    allowed_align[nb_allowed_align].alignment = align.alignment;
    nb_allowed_align++;
    send_to_charf(ch,"Alignment added.\n\r");
  }
  return TRUE;
}
void show_alignment( CHAR_DATA *ch, ALIGN_INFO *allowed_align, const int nb_allowed_align ) {
  send_to_charf( ch, "Alignment:        ");
  if ( nb_allowed_align == 0 )
    send_to_charf( ch, "No alignments\n\r");
  else
    for ( int i = 0; i < nb_allowed_align; i++ ) {
      ALIGN_INFO *align = &(allowed_align[i]);
      if ( i != 0 ) send_to_charf( ch, "                  ");
      send_to_charf( ch, "[%s]\n\r", etho_align_name(align->etho, 
						     align->alignment) );
    }
}
// Race
bool edit_race( CHAR_DATA *ch, int *&race_list, int &nb_races, const char *str, const char *argument ) {
  int iRace = race_lookup( argument, TRUE );
  if ( argument[0] == '\0'
       || iRace < 0 ) {
    send_to_charf(ch,"Syntax:  %s <race name>\n\r"
		  "Type '? race' for a list of available races.\n\r", str);
    return FALSE;
  }
  
  int index = -1;
  for ( int i = 0; i < nb_races; i++ )
    if ( race_list[i] == iRace ) {
      index = i;
      break;
    }

  if ( index == -1 ) { // add
    int *tmp;
    if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(nb_races+1)) ) == NULL ) {
      bug("Can't allocate memory for race");
      send_to_charf(ch,"Can't allocate memory for race.");
      return FALSE;
    }
    for ( int i = 0; i < nb_races; i++ )
      tmp[i] = race_list[i];
    tmp[nb_races] = iRace;
    race_list = tmp;
    nb_races++;
    send_to_charf(ch,"Race added.\n\r");
  }
  else { // remove
    int *tmp;
    if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(nb_races-1)) ) == NULL ) {
      bug("Can't allocate memory for race");
      send_to_charf(ch,"Can't allocate memory for race.");
      return FALSE;
    }
    int count = 0;
    for ( int i = 0; i < nb_races; i++ )
      if ( i != index )
	tmp[count++] = race_list[i];
    race_list = tmp;
    nb_races--;
    send_to_charf(ch,"Race removed.\n\r");
  }
  return TRUE;
}
void show_races( CHAR_DATA *ch, int *race_list, const int nb_races, const char *str ) {
  char buf[MAX_STRING_LENGTH];
  sprintf(buf,"%s:",str);
  //send_to_charf( ch, "Races:            " );
  send_to_charf(ch,"%-18s",buf);
  if ( nb_races == 0 )
    send_to_charf( ch, "No Races\n\r" );
  else {
    int i;
    for ( i = 0; i < nb_races; i++ ) {
	if ( i > 0 && i % 2 == 0 ) send_to_charf( ch, "\n\r                  " );
	send_to_charf( ch, "[%19s] ", race_table[race_list[i]].name );
      }
    send_to_charf( ch, "\n\r" );
  }
}

//*****************************************************************************
//*****************************************************************************
//************************** GENERAL DATA EDITION *****************************
//*****************************************************************************
//*****************************************************************************

// Allow to see a data without having to edit it
static void ability_show( CHAR_DATA *ch, const ability_type *pAbility );
static void race_show( CHAR_DATA *ch, const race_type *pRace );
static void pcrace_show( CHAR_DATA *ch, const pc_race_type *pRace );
static void command_show( CHAR_DATA *ch, const cmd_type *pCommand );
static void pcclass_show( CHAR_DATA *ch, const class_type *pClass );
static void god_show( CHAR_DATA *ch, const god_data *pGod );
static void liquid_show( CHAR_DATA *ch, const liq_type *pLiquid );
static void material_show( CHAR_DATA *ch, const material_type *pMaterial );
static void brew_show( CHAR_DATA *ch, const brew_formula_type *pBrew );
static void group_show( CHAR_DATA *ch, const group_type *pGroup );
static void faction_show( CHAR_DATA *ch, const FACTION_DATA *pFaction );
static void magic_school_show( CHAR_DATA *ch, const magic_school_type *pSchool );

static void super_race_show( CHAR_DATA *ch, const super_pc_race_type *pSuper ) {
  send_to_charf(ch,"Name:                 [%s]\n\r", pSuper->name );
  send_to_charf(ch,"List:                 \n\r" );
  if ( pSuper->nb_pc_race == 0 )
    send_to_charf(ch,"No Races");
  else {
    int col = 0;
    for ( int i = 0; i < pSuper->nb_pc_race; i++ ) {
      int raceId = pSuper->pc_race_list[i];
      if ( raceId > 0 ) {
	send_to_charf(ch,"%20.19s", pc_race_table[raceId].name );
	if ( ++col % 3 == 0 )
	  send_to_charf(ch, "\n\r" );
      }
    }
    if ( col % 3 != 0 )
      send_to_charf(ch, "\n\r" );
  }
}
void do_data_show( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0'
       || arg2[0] == '\0' ) {
    send_to_charf(ch, "Syntax:  dshow <data type> <data>\n\r" );
    return;
  }
  if ( !str_cmp( arg1, "ability" ) ) {
    int sn = ability_lookup( arg2 );
    if ( sn < 0 ) {
      send_to_charf(ch, "Invalid ability [%s].\n\r", arg2 );
      return;
    }
    ability_show( ch, &(ability_table[sn]) );
    return;
  }
  else if ( !str_cmp( arg1, "race" ) ) {
    int iRace = race_lookup( arg2, TRUE );
    if ( iRace < 0 ) {
      send_to_charf(ch,"Invalid race [%s].\n\r", arg2 );
      return;
    }
    race_show( ch, &(race_table[iRace]) );
    return;
  }
  else if ( !str_cmp( arg1, "pcrace" ) ) {
    int iRace = race_lookup( arg2, TRUE );
    if ( iRace < 0 ) {
      send_to_charf(ch,"Invalid pc race [%s].\n\r", arg2 );
      return;
    }
    if ( !race_table[iRace].pc_race ) {
      send_to_charf(ch,"[%s] is not a pc race.\n\r", arg2 );
      return;
    }
    pcrace_show( ch, &(pc_race_table[iRace]) );
    return;
  }
  else if ( !str_cmp( arg1, "command" ) ) {
    int iCmd = command_lookup( arg2 );
    if ( iCmd < 0 ) {
      send_to_charf(ch,"Invalid command [%s].\n\r", arg2 );
      return;
    }
    command_show( ch, &(cmd_table[iCmd]) );
    return;
  }
  else if ( !str_cmp( arg1, "pcclass" ) ) {
    int iClass = class_lookup( arg2 );
    if ( iClass < 0 ) {
      send_to_charf(ch,"Invalid class [%s].\n\r", arg2 );
      return;
    }
    pcclass_show( ch, &(class_table[iClass]) );
    return;
  }
  else if ( !str_cmp( arg1, "god" ) ) {
    int iGod = god_lookup( arg2 );
    if ( iGod < 0 ) {
      send_to_charf(ch,"Invalid god [%s].\n\r", arg2 );
      return;
    }
    god_show( ch, &(gods_table[iGod]) );
    return;
  }
  else if ( !str_cmp( arg1, "liquid" ) ) {
    int iLiq = liq_lookup( arg2 );
    if ( iLiq < 0 ) {
      send_to_charf(ch,"Invalid liquid [%s].\n\r", arg2 );
      return;
    }
    liquid_show( ch, &(liq_table[iLiq]) );
    return;
  }
  else if ( !str_cmp( arg1, "material" ) ) {
    int iMat = material_lookup( arg2 );
    if ( iMat < 0 ) {
      send_to_charf(ch, "Invalid material [%s].\n\r", arg2 );
      return;
    }
    material_show( ch, &(material_table[iMat]) );
    return;
  }
  else if ( !str_cmp( arg1, "brew" ) ) {
    int iBrew = brew_lookup( arg2 );
    if ( iBrew < 0 ) {
      send_to_charf(ch,"Invalid brew formula [%s].\n\r", arg2 );
      return;
    }
    brew_show( ch, &(brew_formula_table[iBrew]) );
    return;
  }
  else if ( !str_cmp( arg1, "group" ) ) {
    int iGroup = group_lookup( arg2 );
    if ( iGroup < 0 ) {
      send_to_charf(ch,"Invalid group [%s].\n\r", arg2 );
      return;
    }
    group_show( ch, &(group_table[iGroup]) );
    return;
  }
  else if ( !str_cmp( arg1, "faction" ) ) {
    int iFaction = get_faction_id( arg2 );
    if ( iFaction < 0 ) {
      send_to_charf(ch,"Invalid faction [%s].\n\r", arg2 );
      return;
    }
    faction_show( ch, &(faction_table[iFaction]) );
    return;
  }
  else if ( !str_cmp( arg1, "magicschool" ) ) {
    int iSchool = magic_school_lookup( arg2 );
    if ( iSchool < 0 ) {
      send_to_charf(ch,"Invalid magic school [%s].\n\r", arg2 );
      return;
    }
    magic_school_show( ch, &(magic_school_table[iSchool]) );
    return;
  }
  else if ( !str_cmp( arg1, "superrace" ) ) {
    int iSuper = super_race_lookup( arg2 );
    if ( iSuper < 0 ) {
      send_to_charf(ch,"Invalid super race [%s].\n\r", arg2 );
      return;
    }
    super_race_show( ch, &(super_pc_race_table[iSuper]) );
    return;
  }
  else {
    send_to_charf(ch, "Syntax:  dshow <data type> <data>\n\r" );
    return;
  }
  return;
}

void do_data_save( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  dsave <data type>\n\r" );
    return;
  }
  if ( !str_cmp( arg, "ability" ) ) {
    new_save_abilities();
    send_to_charf(ch,"Abilities saved.\n\r");
  }
  else if ( !str_cmp( arg, "race" ) ) {
    new_save_races();
    send_to_charf(ch,"Races saved.\n\r");
  }
  else if ( !str_cmp( arg, "pcrace" ) ) {
    new_save_pcraces();
    send_to_charf(ch,"PC Races saved.\n\r");
  }
  else if ( !str_cmp( arg, "command" ) ) {
    new_save_commands();
    send_to_charf(ch,"Commands saved.\n\r");
  }
  else if ( !str_cmp( arg, "pcclass" ) ) {
    new_save_pc_classes();
    send_to_charf(ch,"PC Classes saved.\n\r");
  }
  else if ( !str_cmp( arg, "god" ) ) {
    new_save_gods();
    send_to_charf(ch,"Gods saved.\n\r");
  }
  else if ( !str_cmp( arg, "liquid" ) ) {
    new_save_liquid();
    send_to_charf(ch,"Liquids saved.\n\r");
  }
  else if ( !str_cmp( arg, "material" ) ) {
    new_save_material();
    send_to_charf(ch,"Materials saved.\n\r");
  }
  else if ( !str_cmp( arg, "brew" ) ) {
    new_save_brew_formula();
    send_to_charf(ch,"Brew formulas saved.\n\r");
  }
  else if ( !str_cmp( arg, "group" ) ) {
    new_save_groups();
    send_to_charf(ch,"Groups saved.\n\r");
  }
  else if ( !str_cmp( arg, "faction" ) ) {
    new_save_factions();
    send_to_charf(ch,"Factions saved.\n\r");
  }
  else if ( !str_cmp( arg, "magicschool" ) ) {
    save_schools();
    send_to_charf(ch,"Magic Schools saved.\n\r");
  }
  else if ( !str_cmp( arg, "components" ) ) {
    new_save_brew_components();
    send_to_charf(ch,"Brew components saved.\n\r");
  }
  else if ( !str_cmp( arg, "superrace" ) ) {
    save_superraces();
    send_to_charf(ch,"Super Races saved.\n\r");
  }
  else
    send_to_charf(ch,"Syntax:  dsave <data type>\n\r" );
  return;
}

void do_data_list( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_charf(ch, "Syntax:  dedit list <data type>\n\r"
		      "         dlist <data type>\n\r");
    return;
  }
  BUFFER *buffer = new_buf();
  int col = 0;
  if ( !str_cmp( arg, "ability" ) ) {
    for ( int i = 0; i < MAX_ABILITY; i++ ) {
      ability_type *ability = &(ability_table[i]);
      if ( ability->lockBit ) sprintf( buf, "[%17.16s]", ability->name );
      else sprintf( buf, "%17.16s  ", ability->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "race" ) ) {
    for ( int i = 0; i < MAX_RACE; i++ ) {
      race_type *race = &(race_table[i]);
      if ( race->lockBit ) sprintf( buf,"[%17.16s]", race->name );
      else sprintf( buf,"%17.16s  ", race->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "pcrace" ) ) {
    for ( int i = 0; i < MAX_PC_RACE; i++ ) {
      pc_race_type *race = &(pc_race_table[i]);
      if ( race->lockBit ) sprintf(buf,"[%17.16s]", race->name );
      else sprintf(buf,"%17.16s  ", race->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "command" ) ) {
    for ( int i = 0; i < MAX_COMMANDS; i++ ) {
      cmd_type *cmd = &(cmd_table[i]);
      if ( cmd->lockBit ) sprintf(buf,"[%17.16s]", cmd->name );
      else sprintf(buf,"%17.16s  ", cmd->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "pcclass" ) ) {
    for ( int i = 0; i < MAX_CLASS; i++ ) {
      class_type *cl = &(class_table[i]);
      if ( cl->lockBit ) sprintf(buf,"[%17.16s]", cl->name );
      else sprintf(buf,"%17.16s  ", cl->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "god" ) ) {
    for ( int i = 0; i < MAX_GODS; i++ ) {
      god_data *god = &(gods_table[i]);
      if ( god->lockBit ) sprintf(buf,"[%17.16s]", god->name );
      else sprintf(buf,"%17.16s  ", god->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "liquid" ) ) {
    for ( int i = 0; i < MAX_CLASS; i++ ) {
      liq_type *liq = &(liq_table[i]);
      if ( liq->lockBit ) sprintf(buf,"[%17.16s]", liq->liq_name );
      else sprintf(buf,"%17.16s  ", liq->liq_name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "material" ) ) {
    for ( int i = 0; i < MAX_MATERIAL; i++ ) {
      material_type *mat = &(material_table[i]);
      if ( mat->lockBit ) sprintf(buf,"[%17.16s]", mat->name );
      else sprintf(buf,"%17.16s  ", mat->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "brew" ) ) {
    for ( int i = 0; i < MAX_BREW_FORMULA; i++ ) {
      brew_formula_type *brew = &(brew_formula_table[i]);
      if ( brew->lockBit ) sprintf(buf,"[%17.16s]", brew->name );
      else sprintf(buf,"%17.16s  ", brew->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "group" ) ) {
    for ( int i = 0; i < MAX_GROUP; i++ ) {
      group_type *gr = &(group_table[i]);
      if ( gr->lockBit ) sprintf(buf,"[%17.16s]", gr->name );
      else sprintf(buf,"%17.16s  ", gr->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "faction" ) ) {
    for ( int i = 0; i < factions_count; i++ ) {
      faction_data *fact = &(faction_table[i]);
      if ( fact->lockBit ) sprintf(buf,"[%17.16s]", fact->name );
      else sprintf(buf,"%17.16s  ", fact->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "magicschool" ) ) {
    for ( int i = 0; i < MAX_SCHOOL; i++ ) {
      magic_school_type *sc = &(magic_school_table[i]);
      if ( sc->lockBit ) sprintf(buf,"[%17.16s]", sc->name );
      else sprintf(buf,"%17.16s  ", sc->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else if ( !str_cmp( arg, "superrace" ) ) {
    for ( int i = 0; i < MAX_SUPERRACE; i++ ) {
      super_pc_race_type *sr = &(super_pc_race_table[i]);
      sprintf(buf,"%17.16s  ", sr->name );
      add_buf( buffer, buf );
      if ( ++col % 3 == 0 )
	add_buf( buffer, "\n\r" );
    }
    if ( col % 3 != 0 )
      add_buf( buffer, "\n\r" );
    page_to_char( buf_string(buffer), ch );
    return;
  }
  else {
    send_to_charf(ch,"Syntax:  dedit list <data type>\n\r"
		     "         dlist <data type>\n\r");
    return;
  }
}

const struct editor_cmd_type data_editor_table[] = {
  {   "ability",        do_ability_edit  },
  {   "race",           do_race_edit     },
  {   "pcrace",         do_pcrace_edit   },
  {   "command",        do_command_edit  },
  {   "pcclass",        do_pcclass_edit  },
  {   "god",            do_god_edit      },
  {   "liquid",         do_liquid_edit   },
  {   "material",       do_material_edit },
  {   "brew",           do_brew_edit     },
  {   "faction",        do_faction_edit  },
  {   "magicschool",    do_magic_school_edit },
  {   "list",           do_data_list     },
  {   NULL,             0                }
};

void do_dedit( CHAR_DATA *ch, const char *argument ) {
  char command[MAX_INPUT_LENGTH];
  int  cmd;

  argument = one_argument( argument, command );

  if ( command[0] == '\0' ) {
    do_help( ch, "dedit" );
    return;
  }
 
  /* Search Table and Dispatch Command. */
  for ( cmd = 0; data_editor_table[cmd].name != NULL; cmd++ ) {
    if ( !str_prefix( command, data_editor_table[cmd].name ) ) {
      (*data_editor_table[cmd].do_fun) ( ch, argument );
      return;
    }
  }

  /* Invalid command, send help. */
  do_help( ch, "dedit" );
  return;
}


//*****************************************************************************
//****************                 ABILITY             ************************
//****************                + PREREQS            ************************
//****************                + CLASS LEVEL        ************************
//****************                + CLASS RATE         ************************
//*****************************************************************************

void do_ability_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( abilitynotsaved )
    new_save_abilities();
  if ( pcclassnotsaved )
    new_save_pc_classes();
  send_to_char( "Skills/Spells/Powers/Songs and classes have been saved.\n\r", ch );
}

#define ABILITYEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type ability_edit_table[] = {
  {   "commands",	show_commands	},

  {   "slot",           ability_edit_slot },
  {   "rating",         ability_edit_rating },
  {   "level",          ability_edit_level },
  {   "target",         ability_edit_target },
  {   "position",       ability_edit_position },
  {   "cost",           ability_edit_cost },
  {   "beats",          ability_edit_beats },
  {   "mobuse",         ability_edit_mobuse },
  {   "damage",         ability_edit_dam_msg },
  {   "charwearoff",    ability_edit_char_wear_off },
  {   "objwearoff",     ability_edit_obj_wear_off },
  {   "roomwearoff",    ability_edit_room_wear_off },
  {   "dispellable",    ability_edit_dispellable },
  {   "dispelmsg",      ability_edit_dispel_msg },
  {   "wait",           ability_edit_wait },
  {   "craftable",      ability_edit_craftable },
  {   "addprereqs",     ability_edit_addprereqs },
  {   "delprereqs",     ability_edit_delprereqs },
  {   "setprereqs",     ability_edit_setprereqs },
  {   "name",           ability_edit_name },
  {   "school",         ability_edit_school },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void ability_edit( CHAR_DATA *ch, const char *argument ) {
  ability_type *pAbility = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int  cmd;
  int  value;

  if ( !OLC_EDIT( ch, pAbility ) )
    return;
  EDIT_ABILITY( ch, pAbility );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit abilities.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    ability_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( cmd = 0; ability_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, ability_edit_table[cmd].name ) ) {
      if ( (*ability_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	abilitynotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing ability. */
void do_ability_edit( CHAR_DATA *ch, const char *argument ) {
  ability_type *pAbility;
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit abilities.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify an ability name.\n\r"
		 "Syntax: abilityedit <ability name>\n\r", ch );
    return;
  }
  int sn = ability_lookup(arg);
  if ( sn <= 0 ) {
    send_to_charf(ch,"Ability (%s) doesn't exist.\n\r", arg );
    return;
  }
  pAbility = &(ability_table[sn]);

  if ( !OLC_EDIT( ch, pAbility ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pAbility;
  ch->desc->editor = ED_ABILITY;
  return;
}

// Ability editing methods
static void ability_show( CHAR_DATA *ch, const ability_type *pAbility ) {
  char buf[MAX_STRING_LENGTH];
  char sk_lvl[MAX_STRING_LENGTH];
  char sk_rat[MAX_STRING_LENGTH];
  char cl_nam[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char conv[16];

  int sn = ability_lookup( pAbility->name );

  send_to_charf( ch, "Name:                 [%s]\n\r", pAbility->name );
  send_to_charf( ch, "Type:                 [%s]\n\r", flag_string( ability_type_flags, pAbility->type ));

  if ( pAbility->type == TYPE_AFFECT ) {
    send_to_charf( ch, "Char wear off msg:    %s\n\r", (pAbility->msg_off==NULL||pAbility->msg_off[0]=='\0')?"None":pAbility->msg_off );
    send_to_charf( ch, "Obj wear off msg:     %s\n\r", (pAbility->msg_obj==NULL||pAbility->msg_obj[0]=='\0')?"None":pAbility->msg_obj );
    send_to_charf( ch, "Room wear off msg:    %s\n\r", (pAbility->msg_room==NULL||pAbility->msg_room[0]=='\0')?"None":pAbility->msg_room );
    send_to_charf( ch, "Dispellable:          [%s]\n\r", pAbility->dispellable?"Yes":"No" );
    send_to_charf( ch, "Dispel msg:           %s\n\r", (pAbility->msg_dispel==NULL||pAbility->msg_dispel[0]=='\0')?"None":pAbility->msg_dispel );
    send_to_charf( ch, "Wear off function:    [%s]\n\r", (pAbility->wearoff_fun==NULL||pAbility->wearoff_fun==wearoff_null)?"No":"Yes" );
    send_to_charf( ch, "Update function:      [%s]\n\r", (pAbility->update_fun==NULL||pAbility->update_fun==update_null)?"No":"Yes" );
    send_to_charf( ch, "Script ability:       [%s]\n\r", pAbility->scriptAbility?"Yes":"No");
  }
  else {
    if ( pAbility->pgsn != NULL )
      send_to_charf( ch, "Gsn:                  [%d]\n\r", *pAbility->pgsn );
    if ( pAbility->slot )
      send_to_charf( ch, "Slot:                 [%d]\n\r", pAbility->slot );
    
    sk_lvl[0]='\0';
    sk_rat[0]='\0';
    cl_nam[0]='\0';
    bool found = FALSE;
    for ( int i = 0; i < MAX_CLASS; i++ ) {
      int rate = pAbility->rating[i];
      int lvl = pAbility->ability_level[i];
      if ( rate > 0 
	   || ( lvl > 0
		&& lvl < IM ) ) {
	found = TRUE;
	sprintf(conv," %3d",lvl);
	strcat(sk_lvl, conv);
	
	sprintf(conv," %3d",rate);
	strcat(sk_rat, conv);
	
	sprintf(conv, " %s",class_table[i].who_name );
	strcat(cl_nam, conv);
      }
    }
    if ( found )
      send_to_charf( ch, 
		     "Classes:             %s\n\r"
		     "Rating:              %s\n\r"
		     "Level:               %s\n\r",
		     cl_nam, sk_rat, sk_lvl );
    else
      send_to_charf(ch,"NOT AVAILABLE FOR ANY CLASSES.\n\r");
    
    send_to_charf( ch, "Target:               [%s]\n\r", flag_string( target_flags, pAbility->target ) );
    send_to_charf( ch, "Min position:         [%s]\n\r", flag_string( position_flags, pAbility->minimum_position ) );
    send_to_charf( ch, "Min cost:             [%3d]\n\r", pAbility->min_cost );
    send_to_charf( ch, "Beats:                [%3d]\n\r", pAbility->beats );
    //send_to_charf( ch, "Mob Use:              [%s]\n\r", flag_string( mob_use_flags, pAbility->mob_use) );
    send_to_charf( ch, "Mob Use:              %s\n\r", flag_string( mob_use_flags, pAbility->mob_use) );
    send_to_charf( ch, "Damage msg:           %s\n\r", (pAbility->noun_damage==NULL||pAbility->noun_damage[0]=='\0')?"None":pAbility->noun_damage );
    send_to_charf( ch, "Char wear off msg:    %s\n\r", (pAbility->msg_off==NULL||pAbility->msg_off[0]=='\0')?"None":pAbility->msg_off );
    send_to_charf( ch, "Obj wear off msg:     %s\n\r", (pAbility->msg_obj==NULL||pAbility->msg_obj[0]=='\0')?"None":pAbility->msg_obj );
    send_to_charf( ch, "Room wear off msg:    %s\n\r", (pAbility->msg_room==NULL||pAbility->msg_room[0]=='\0')?"None":pAbility->msg_room );
    send_to_charf( ch, "Dispellable:          [%s]\n\r", pAbility->dispellable?"Yes":"No" );
    send_to_charf( ch, "Dispel msg:           %s\n\r", (pAbility->msg_dispel==NULL||pAbility->msg_dispel[0]=='\0')?"None":pAbility->msg_dispel );
    send_to_charf( ch, "Wait before new use:  [%d]\n\r", pAbility->to_wait );
    send_to_charf( ch, "Craftable:            [%s]\n\r", pAbility->craftable?"Yes":"No" );
    send_to_charf( ch, "Wear off function:    [%s]\n\r", (pAbility->wearoff_fun==NULL||pAbility->wearoff_fun==wearoff_null)?"No":"Yes" );
    send_to_charf( ch, "Update function:      [%s]\n\r", (pAbility->update_fun==NULL||pAbility->update_fun==update_null)?"No":"Yes" );
    send_to_charf( ch, "Script ability:       [%s]\n\r", pAbility->scriptAbility?"Yes":"No");
    if ( pAbility->school >= 0 )
      send_to_charf( ch, "School:               [%s]\n\r", magic_school_table[pAbility->school].name );
    
    send_to_charf( ch, "Number casting level: [%d]\n\r", pAbility->nb_casting_level );
    if ( pAbility->prereqs != NULL ) {
      sprintf( buf, 
	       "Prereqs:\n\r" );
      for ( int i = 0; i < pAbility->nb_casting_level+1; i++ ) {
	if ( i == 0 && pAbility->prereqs[i].nb_prereq == 0 )
	  continue;
	PREREQ_DATA *prereq = &(pAbility->prereqs[i]);
	sprintf( buf2,
		 " Level %d [cost=%2d  level=%3d%s%s]: ", 
		 i, prereq->cost, prereq->plr_level,
		 (prereq->classes!=ANY_CLASSES)?"  classes=":"",
		 (prereq->classes!=ANY_CLASSES)?flag_string(classes_flags, prereq->classes):""); // SinaC 2003
	if ( pAbility->prereqs[i].nb_prereq == 0 )
	  strcat( buf2, " no prereqs.\n\r");
	else {
	  for ( int j = 0; j < pAbility->prereqs[i].nb_prereq; j++ ) {
	    char buf4[MAX_STRING_LENGTH];
	    buf4[0] = '\0';
	    if ( pAbility->prereqs[i].prereq[j].casting_level > 0 )
	      sprintf( buf4, " casting level %d", pAbility->prereqs[i].prereq[j].casting_level );
	    sprintf( buf3,
		     "%s'%s'%s.\n\r",
		     j == 0 ? "" : "                               ",
		     ability_table[pAbility->prereqs[i].prereq[j].sn].name,
		     buf4 );
	    strcat( buf2, buf3 );
	  }
	}
	strcat( buf, buf2 );
      }
    }
    else
      sprintf( buf,
	       "Prereqs:  none.\n\r" );
    send_to_char( buf, ch );
  }
}
ABILITYEDIT( ability_edit_show ) {
  ability_type *pAbility;

  EDIT_ABILITY(ch, pAbility);
  ability_show( ch, pAbility );

  return FALSE;
}
ABILITYEDIT( ability_edit_slot ) {
  if ( argument[0] == '\0' || !is_number( argument ) ) {
    send_to_char( "Syntax:  slot [number]\n\r", ch );
    return FALSE;
  }

  int slot = atoi( argument );
  if ( exist_slot( slot ) ) {
    send_to_charf(ch, "Slot %d already used.", slot );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->slot = slot;

  send_to_char( "Slot set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_rating ) {
  // Usage rating <class name> <rate value>
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) ) {
    send_to_char( "Syntax:  rating <class name> <train rate>\n\r",ch);
    return FALSE;
  }
  int iClass = class_lookup( arg1 );
  if ( iClass <= 0 ) {
    send_to_charf(ch, "Invalid class %s.\n\r", arg1 );
    send_to_charf(ch,"Type '? class' for a list of classes.\n\r");
    return FALSE;
  }
  int rate = atoi( arg2 );
  if ( rate < 0 ) {
    send_to_char( "Rate must be a positive integer.\n\r", ch );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->rating[iClass] = rate;
  
  send_to_charf(ch,"Rate for class %s has been set.\n\r", class_table[iClass].name );
  return TRUE;
}
ABILITYEDIT( ability_edit_level ) {
  // Usage level <class name> <rate value>
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) ) {
    send_to_char( "Syntax:  level <class name> <level>\n\r",ch);
    return FALSE;
  }
  int iClass = class_lookup( arg1 );
  if ( iClass <= 0 ) {
    send_to_charf(ch, "Invalid class %s.\n\r", arg1 );
    send_to_charf(ch,"Type '? class' for a list of classes.\n\r");
    return FALSE;
  }
  int lvl = UMIN( atoi( arg2 ), IM );
  if ( lvl < 0 ) {
    send_to_char( "Level must be a positive integer.\n\r", ch );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->ability_level[iClass] = lvl;
  
  send_to_charf(ch,"Level for class %s has been set.\n\r", class_table[iClass].name );
  return TRUE;
}
ABILITYEDIT( ability_edit_target ) {
  if ( argument[0] != 0 ) {
    ability_type *pAbility;
    EDIT_ABILITY(ch, pAbility);

    int target = flag_value( target_flags, argument );
    if ( target != NO_FLAG ) {
      pAbility->target = target;
      send_to_charf(ch,"Target set.\n\r");
      return TRUE;
    }
  }

  send_to_char( "Syntax:  target [target name]\n\r"
		"Type '? target' for a list of target.\n\r", ch );
  return FALSE;
}
ABILITYEDIT( ability_edit_position ) {
  if ( argument[0] != 0 ) {
    ability_type *pAbility;
    EDIT_ABILITY(ch, pAbility);

    int position = flag_value_complete( position_flags, argument );
    if ( position != NO_FLAG ) {
      pAbility->minimum_position = position;
      send_to_charf(ch,"Minimum position set.\n\r");
      return TRUE;
    }
  }

  send_to_char( "Syntax:  position [position name]\n\r"
		"Type '? position' for a list of position.\n\r", ch );
  return FALSE;
}
ABILITYEDIT( ability_edit_cost ) {
  int cost = atoi( argument );
  if ( argument[0] == '\0' || !is_number( argument ) || cost < 0 ) {
    send_to_char( "Syntax:  cost [mana/psp/move cost]\n\r", ch );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->min_cost = cost;

  send_to_char( "Minimum cost set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_beats ) {
  int beats = atoi( argument );
  if ( argument[0] == '\0' || !is_number( argument ) || beats < 0 ) {
    send_to_char( "Syntax:  beats [number of ticks]\n\r", ch );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->beats = beats;

  send_to_char( "Beats set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_mobuse ) {
  if ( argument[0] != 0 ) {
    ability_type *pAbility;
    EDIT_ABILITY(ch, pAbility);

    int value = flag_value( mob_use_flags, argument );
    if ( value != NO_FLAG ) {
      TOGGLE_BIT( pAbility->mob_use, value );
      //pAbility->mob_use = value;
      send_to_charf(ch,"Mob use toggled.\n\r");
      //send_to_charf(ch,"Mob use set.\n\r");
      return TRUE;
    }
  }

  send_to_char( "Syntax:  mobuse [value]\n\r"
		"Type '? mobuse' for a list of mob use.\n\r", ch );
  return FALSE;
}
ABILITYEDIT( ability_edit_dam_msg ) {
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  damage [string]\n\r", ch );
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->noun_damage = str_dup( argument, TRUE );
  send_to_char( "Damage message set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_char_wear_off ) {
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  charwearoff [string]\n\r", ch );
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->msg_off = str_dup( argument, TRUE );
  send_to_char( "Char wear off message set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_obj_wear_off ) {
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  objwearoff [string]\n\r", ch );
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->msg_obj = str_dup( argument, TRUE );
  send_to_char( "Object wear off message set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_room_wear_off ) {
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  roomwearoff [string]\n\r", ch );
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->msg_room = str_dup( argument, TRUE );
  send_to_char( "Room wear off message set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_dispellable ) {
  if ( argument[0] != 0 ) {
    ability_type *pAbility;
    EDIT_ABILITY(ch, pAbility);
    int value = -1;
    if ( !str_cmp( argument, "yes" ) || !str_cmp( argument, "true" ) )
      value = 1;
    else if ( !str_cmp( argument, "no" ) || !str_cmp( argument, "false" ) )
      value = 0;
    if ( value != -1 ) {
      pAbility->dispellable = value;
      send_to_char("Dispellable set.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax:  Dispellable [true/false/yes/no]\n\r", ch );
  return FALSE;
}
ABILITYEDIT( ability_edit_dispel_msg ) {
  if ( argument[0] == '\0' ) {
    send_to_char( "Syntax:  dispelmsg [string]\n\r", ch );
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->msg_dispel = str_dup( argument, TRUE );
  send_to_char( "Dispel message set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_craftable ) {
  if ( argument[0] != 0 ) {
    ability_type *pAbility;
    EDIT_ABILITY(ch, pAbility);
    int value = -1;
    if ( !str_cmp( argument, "yes" ) || !str_cmp( argument, "true" ) )
      value = 1;
    else if ( !str_cmp( argument, "no" ) || !str_cmp( argument, "false" ) )
      value = 0;
    if ( value != -1 ) {
      pAbility->craftable = value;
      send_to_char("Craftable set.\n\r", ch );
      return TRUE;
    }
  }

  send_to_char( "Syntax:  Craftable [true/false/yes/no]\n\r", ch );
  return FALSE;
}
ABILITYEDIT( ability_edit_wait ) {
  int wait = atoi( argument );
  if ( argument[0] == '\0' || !is_number( argument ) || wait < 0 ) {
    send_to_char( "Syntax:  wait [number of ticks]\n\r", ch );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  pAbility->to_wait = wait;

  send_to_char( "Wait before new use set.\n\r", ch);
  return TRUE;
}
ABILITYEDIT( ability_edit_addprereqs ) {
  // Usage addprereqs <casting level> <ability name> <ability level>
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  int v1 = atoi(arg1);
  int v2 = atoi(arg3);
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
       || !is_number(arg1) || !is_number(arg3)
       || v1 < 0 || v2 < 0 ) {
    send_to_char("Syntax:  addprereq [casting level] [ability name] [ability casting level]\n\r", ch );
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);
  if ( v1 > pAbility->nb_casting_level ) {
    send_to_charf(ch,"Invalid casting level %d, %s has only %d levels.\n\r",
		  v1, pAbility->name, pAbility->nb_casting_level );
    return FALSE;
  }
  int sn = ability_lookup( arg2 );
  if ( sn <= 0 ) {
    send_to_charf(ch,"Unknown ability %s.\n\r", arg2 );
    return FALSE;
  }
  ability_type *prereq = &(ability_table[sn]);
  if ( v2 > prereq->nb_casting_level ) {
    send_to_charf(ch,"Invalid casting level %d, %s has only %d levels.\n\r",
		  v2, prereq->name, prereq->nb_casting_level );
    return FALSE;
  }

  int fromSn = ability_lookup(pAbility->name);
  int res = check_prerequisites_cycle( fromSn, v1, sn, v2 );
  switch ( res ) {
  case 1: case 3:
    send_to_charf(ch,"Adding this prerequisite would create cyclic prerequisites.\n\r"); 
    return FALSE;
    break;
  case 2:
    send_to_charf(ch,"Problem when adding this prerequisite.\n\r");
    return FALSE;
    break;
  case 0: default: break; // OK
  }

  // we must check invalid/cyclic prereq  TODO  FIXME
  // if no prereq found, we create the vector
  if ( pAbility->prereqs == NULL ) {
    if ((pAbility->prereqs = (PREREQ_DATA *)GC_MALLOC((pAbility->nb_casting_level+1)*sizeof(PREREQ_DATA)))==NULL) {
      bug("Can't allocate memory for prereqs");
      return FALSE;
    }
    for ( int j = 0; j < pAbility->nb_casting_level+1; j++ ) {
      pAbility->prereqs[j].cost = DEFAULT_PREREQ_COST(j);
      pAbility->prereqs[j].plr_level = 1;
      pAbility->prereqs[j].prereq = NULL;
      pAbility->prereqs[j].nb_prereq = 0;
    }
  }
  PREREQ_DATA *pData = &(pAbility->prereqs[v1]); // get info for casting level v1
  PREREQ_LIST *pList = pData->prereq; // get prereq for casting level v1
 // first prereq for that casting level
  if ( pData->nb_prereq == 0 ) {
    if ( ( pList = (PREREQ_LIST*)GC_MALLOC(1*sizeof(PREREQ_LIST)) ) == NULL ) {
      bug("Can't allocate memory for a new prereq");
      return FALSE;
    }
    pList->sn = sn;
    pList->casting_level = v2;
    pData->nb_prereq = 1;
  }
  else { // already existing prereq for that casting level
    PREREQ_LIST *tmp;
    if ( ( tmp = (PREREQ_LIST*)GC_MALLOC((pData->nb_prereq+1)*sizeof(PREREQ_LIST)) ) == NULL ) {
      bug("Can't allocate memory for a new prereq");
      return FALSE;
    }
    bcopy( pList, tmp, pData->nb_prereq*sizeof(PREREQ_LIST));
    pData->prereq = tmp;
    pList = &(pData->prereq[pData->nb_prereq]);
    pList->sn = sn;
    pList->casting_level = v2;
    pData->nb_prereq++;
  }

  send_to_charf(ch,"%s lvl %d is now a prerequisite to get %s lvl %d.\n\r",
		prereq->name, v2, pAbility->name, v1 );
  return TRUE;
}
ABILITYEDIT( ability_edit_delprereqs ) {
  // Usage delprereqs <casting level> <ability name>
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  int v1 = atoi(arg1);
  if ( arg1[0] == '\0' || arg2[0] == '\0'
       || !is_number(arg1)
       || v1 < 0 ) {
    send_to_char("Syntax:  delprereq [casting level] [ability name/all]\n\r", ch );
    return FALSE;
  }
  bool fAll = FALSE;
  int sn = 0;
  if (!str_cmp(arg2,"all") )
    fAll = TRUE;
  else {
    sn = ability_lookup( arg2 );
    if ( sn <= 0 ) {
      send_to_charf(ch,"Unknown ability %s.\n\r", arg2 );
      return FALSE;
    }
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);
  if ( v1 > pAbility->nb_casting_level ) {
    send_to_charf(ch,"Invalid casting level %d, %s has only %d levels.\n\r",
		  v1, pAbility->name, pAbility->nb_casting_level );
    return FALSE;
  }
  PREREQ_DATA *pData;
  if ( pAbility->prereqs == NULL
       || ( pData = &(pAbility->prereqs[v1]) ) == NULL
       || pData->nb_prereq == 0 ) {
    send_to_charf(ch,"No prerequisites for that casting level.\n\r");
    return FALSE;
  }
  if ( pData->nb_prereq == 1 || fAll ) { // if only one prereq or remove all
    pData->nb_prereq = 0;
    pData->prereq = NULL;
  }
  else { // more than one prereq and remove only one
    for ( int i = 0; i < pData->nb_prereq; i++ ) {
      PREREQ_LIST *pList = &(pData->prereq[i]);
      if ( pList->sn == sn ) {
	PREREQ_LIST *tmp;
	if ( ( tmp = (PREREQ_LIST*)GC_MALLOC((pData->nb_prereq-1)*sizeof(PREREQ_LIST)) ) == NULL ) {
	  bug("Can't allocate memory for temporary variable");
	  return FALSE;
	}
	// copy prereqs except one to remove
	int count = 0;
	for ( int j = 0; j < pData->nb_prereq; j++ ) {
	  if ( i != j ) {
	    tmp[count].sn = pData->prereq[j].sn;
	    tmp[count].casting_level = pData->prereq[j].casting_level;
	    count++;
	  }
	}
	pData->nb_prereq--;
	pData->prereq = tmp;
	break;
      }
    }
  }
  send_to_charf(ch,"Prerequisite(s) for %s casting level %d removed.\n\r", pAbility->name, v1 );
  return TRUE;
}
ABILITYEDIT( ability_edit_setprereqs ) {
  // Usage setprereqs <casting level> cost <#trains>
  //       setprereqs <casting level> level <needed level>
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  int v1 = atoi(arg1);
  int v2 = atoi(arg3);
  int which = -1;
  if ( !str_cmp(arg2,"cost") )
    which = 1;
  else if ( !str_cmp(arg2,"level") )
    which = 2;
  else if ( !str_cmp(arg2,"class") )
    which = 3;
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
       || !is_number(arg1) || !is_number(arg3)
       || v1 < 0 
       || ( v2 < 0 && which != 3 )
       || which == -1 ) {
    send_to_char("Syntax:  setprereq [casting level] cost [#trains]\n\r", ch );
    send_to_char("         setprereq [casting level] level [needed level]\n\r", ch );
    send_to_char("         setprereq [casting level] class [allowed class/any]\n\r", ch );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);
  if ( v1 > pAbility->nb_casting_level ) {
    send_to_charf(ch,"Invalid casting level %d, %s has only %d levels.\n\r",
		  v1, pAbility->name, pAbility->nb_casting_level );
    return FALSE;
  }
  // no prereq found: create prereq vector
  if ( pAbility->prereqs == NULL ) {
    if ((pAbility->prereqs = (PREREQ_DATA *)GC_MALLOC((pAbility->nb_casting_level+1)*sizeof(PREREQ_DATA)))==NULL) {
      bug("Can't allocate memory for prereqs.");
      return FALSE;
    }
    for ( int j = 0; j < pAbility->nb_casting_level+1; j++ ) {
      pAbility->prereqs[j].cost = DEFAULT_PREREQ_COST(j);
      pAbility->prereqs[j].plr_level = 1;
      pAbility->prereqs[j].prereq = NULL;
      pAbility->prereqs[j].nb_prereq = 0;
      pAbility->prereqs[j].classes = ANY_CLASSES;
    }
  }
  PREREQ_DATA *pData = &(pAbility->prereqs[v1]);
  switch( which ) {
  case 1: // cost
    pData->cost = v2;
    send_to_charf(ch,"Cost of casting level %d for ability %s set.\n\r", v1, pAbility->name );
    break;
  case 2: // level
    pData->plr_level = v2;
    send_to_charf(ch,"Needed level of casting level %d for ability %s set.\n\r", v1, pAbility->name );
  case 3: // classes
    if ( !str_cmp( arg3, "any" ) )
      pData->classes = ANY_CLASSES;
    else {
      int cl = flag_value( classes_flags, arg3 );
      if ( cl == NO_FLAG ) {
	send_to_charf(ch,"Invalid classes %s.", arg3 );
	return FALSE;
      }
      pData->classes = cl;
    }
    send_to_charf(ch,"Allowed classes of casting level %d for ability %s set.\n\r", v1, pAbility->name );
    break;
  }
  return TRUE;
}
ABILITYEDIT( ability_edit_name ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  name  [ability name]\n\r");
    return FALSE;
  }

  if ( !is_valid_name( argument ) ) {
    send_to_charf(ch,"[%s] is not valid name.\n\r", argument );
    return FALSE;
  }

  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);

  if ( !pAbility->scriptAbility ) {
    send_to_charf(ch,"You can't modify an ability name unless it is a script ability.\n\r");
    return FALSE;
  }
  
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  pAbility->name = str_dup( buf, TRUE );
  
  send_to_charf(ch,"Name set.\n\r");
  return TRUE;
}
ABILITYEDIT( ability_edit_school ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  school  [school name/none]\n\r");
    return FALSE;
  }
  ability_type *pAbility;
  EDIT_ABILITY(ch, pAbility);
  int sn = ability_lookup( pAbility->name );
  if ( !str_cmp( argument, "none" ) ) {
    remove_from_school( sn );
    send_to_charf(ch,"Removed from school.\n\r");
    return TRUE;
  }
  int school = magic_school_lookup(argument);
  if ( school < 0 ) {
    send_to_charf(ch,"Unknown school.\n\r"
		  "Type 'dlist magicschool' for a list of available magic schools.\n\r");
    return FALSE;
  }
  //  remove from old school
  remove_from_school( sn );
  //  add in new school
  add_to_school( school, sn );
  send_to_charf(ch,"Added to school %s.\n\r", magic_school_table[school].name );
  return TRUE;
}


//*****************************************************************************
//****************                 RACE                ************************
//*****************************************************************************

void do_race_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( racenotsaved ) {
    new_save_races();
    new_save_pcraces(); // Modified by SinaC 2003
  }
  send_to_char( "Races and PCRaces have been saved.\n\r", ch );
}

#define RACEEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type race_edit_table[] = {
  {   "commands",	show_commands	},

  {   "act",              race_edit_act },
  {   "affect",           race_edit_affect },
  {   "affect2",          race_edit_affect2 },
  {   "offensive",        race_edit_offensive },
  {   "immunities",       race_edit_immunities },
  {   "resistances",      race_edit_resistances },
  {   "vulnerabilities",  race_edit_vulnerabilities },
  {   "form",             race_edit_forms },
  {   "parts",            race_edit_parts },
  {   "size",             race_edit_size },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void race_edit( CHAR_DATA *ch, const char *argument ) {
  race_type *pRace = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pRace ) )
    return;
  EDIT_RACE( ch, pRace );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit races.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    race_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; race_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, race_edit_table[cmd].name ) ) {
      if ( (*race_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	racenotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing ability. */
void do_race_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit races.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a race name.\n\r"
		 "Syntax: raceedit <race name>\n\r", ch );
    return;
  }
  int iRace = race_lookup(arg,TRUE);
  if ( iRace < 0 ) {
    send_to_charf(ch,"Race (%s) doesn't exist.\n\r", arg );
    return;
  }
  race_type *pRace;
  pRace = &(race_table[iRace]);

  if ( !OLC_EDIT( ch, pRace ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pRace;
  ch->desc->editor = ED_RACE;
  return;
}


// Ability editing methods
static void race_show( CHAR_DATA *ch, const race_type *pRace ) {
  send_to_charf( ch, "Name:             [%s]\n\r", pRace->name );
  send_to_charf( ch, "Act:              [%s]\n\r", flag_string( act_flags, pRace->act ) );
  send_to_charf( ch, "Affected by:      [%s]\n\r", flag_string( affect_flags, pRace->aff ) );
  send_to_charf( ch, "Also affected by: [%s]\n\r", flag_string( affect2_flags, pRace->aff2 ) );
  send_to_charf( ch, "Offensive:        [%s]\n\r", flag_string( off_flags,  pRace->off ) );
  send_to_charf( ch, "Immunities:       [%s]\n\r", flag_string( irv_flags, pRace->imm ) );
  send_to_charf( ch, "Resistances:      [%s]\n\r", flag_string( irv_flags, pRace->res ) );
  send_to_charf( ch, "Vulnerabilities:  [%s]\n\r", flag_string( irv_flags, pRace->vuln ) );
  send_to_charf( ch, "Form:             [%s]\n\r", flag_string( form_flags, pRace->form ) );
  send_to_charf( ch, "Parts:            [%s]\n\r", flag_string( part_flags, pRace->parts ) );
  send_to_charf( ch, "Size:             [%s]\n\r", flag_string( size_flags, pRace->size ) );
  send_to_charf( ch, "PC Race:          [%s]\n\r", pRace->pc_race?"Yes":"No" );
}
RACEEDIT( race_edit_show ) {
  race_type *pRace;

  EDIT_RACE(ch, pRace);

  race_show( ch, pRace );
  
  return FALSE;
}
RACEEDIT( race_edit_act ) {
  if ( argument[0] != 0 ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);

    int act = flag_value( act_flags, argument );
    if ( act != NO_FLAG ) {
      pRace->act ^= act;
      send_to_charf(ch,"Act flag toggled.\n\r");
      return TRUE;
    }
  }

  send_to_char( "Syntax: act [flag]\n\r"
		"Type '? act' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_affect ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int aff = flag_value( affect_flags, argument );
    if ( aff != NO_FLAG ) {
      pRace->aff ^= aff;

      send_to_char( "Affect flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: affect [flag]\n\r"
		"Type '? affect' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_affect2 ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int aff2 = flag_value( affect2_flags, argument );
    if ( aff2 != NO_FLAG ) {
      pRace->aff2 ^= aff2;

      send_to_char( "Affect2 flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: affect2 [flag]\n\r"
		"Type '? affect2' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_offensive ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int off = flag_value( off_flags, argument );
    if ( off != NO_FLAG ) {
      pRace->off ^= off;

      send_to_char( "Offensive flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: off [flag]\n\r"
		"Type '? off' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_immunities ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int imm = flag_value( irv_flags, argument );
    if ( imm != NO_FLAG ) {
      pRace->imm ^= imm;

      send_to_char( "Immunities flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: imm [flag]\n\r"
		"Type '? imm' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_resistances ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int res = flag_value( irv_flags, argument );
    if ( res != NO_FLAG ) {
      pRace->res ^= res;

      send_to_char( "Resistances flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: res [flag]\n\r"
		"Type '? res' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_vulnerabilities ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int vuln = flag_value( irv_flags, argument );
    if ( vuln != NO_FLAG ) {
      pRace->vuln ^= vuln;

      send_to_char( "Vulnerabilities flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: vuln [flag]\n\r"
		"Type '? vuln' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_forms ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int form = flag_value( form_flags, argument );
    if ( form != NO_FLAG ) {
      pRace->form ^= form;

      send_to_char( "Form flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: form [flag]\n\r"
		"Type '? form' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_parts ) {
  if ( argument[0] != '\0' ) {
    race_type *pRace;
    EDIT_RACE(ch, pRace);
    int parts = flag_value( part_flags, argument );
    if ( parts != NO_FLAG ) {
      pRace->parts ^= parts;

      send_to_char( "Part flag toggled.\n\r", ch);
      return TRUE;
    }
  }

  send_to_char( "Syntax: part [flag]\n\r"
		"Type '? part' for a list of flags.\n\r", ch );
  return FALSE;
}
RACEEDIT( race_edit_size ) {
  int size = flag_value( size_flags, argument );
  if ( argument[0] == '\0'
       || size == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  size [size]\n\r"
		  "Type '? size' for a list of size.\n\r" );
    return FALSE;
  }
  race_type *pRace;
  EDIT_RACE(ch, pRace);
  pRace->size = size;
  send_to_charf(ch,"Race size set.\n\r");
  return TRUE;
}
//*****************************************************************************
//****************              PC RACE                ************************
//*****************************************************************************

#define PCRACEEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type pcrace_edit_table[] = {
  {   "commands",	show_commands	},
  
  {   "whoname",        pcrace_edit_whoname },
  {   "experience",     pcrace_edit_experience },
  {   "type",           pcrace_edit_type },
  //  {   "size",           pcrace_edit_size },
  {   "attributes",     pcrace_edit_attributes },
  {   "language",       pcrace_edit_language },
  {   "abilities",      pcrace_edit_abilities },
  {   "alignment",      pcrace_edit_alignment },
  {   "classes",        pcrace_edit_classes },
  {   "remort",         pcrace_edit_remort },
  {   "minremort",      pcrace_edit_min_remort },
  {   "rebirth",        pcrace_edit_rebirth },
  {   "superrace",      pcrace_edit_superrace },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void pcrace_edit( CHAR_DATA *ch, const char *argument ) {
  pc_race_type *pRace = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pRace ) )
    return;
  EDIT_PCRACE( ch, pRace );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit pc races.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    pcrace_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; pcrace_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, pcrace_edit_table[cmd].name ) ) {
      if ( (*pcrace_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	racenotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing ability. */
void do_pcrace_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit pc races.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a pc race name.\n\r"
		 "Syntax: pcraceedit <pc race name>\n\r", ch );
    return;
  }
  int iRace = race_lookup(arg,TRUE);
  if ( iRace < 0 ) {
    send_to_charf(ch,"Race (%s) doesn't exist.\n\r", arg );
    return;
  }
  if ( !race_table[iRace].pc_race ) {
    send_to_charf(ch,"Race (%s) is not a pc race.\n\r", arg );
    return;
  }
  pc_race_type *pRace;
  pRace = &(pc_race_table[iRace]);

  if ( !OLC_EDIT( ch, pRace ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pRace;
  ch->desc->editor = ED_PCRACE;
  return;
}


// Ability editing methods
static void pcrace_show( CHAR_DATA *ch, const pc_race_type *pRace ) {
  send_to_charf( ch, "Name:             [%s]\n\r", pRace->name );
  send_to_charf( ch, "WhoName:          [%s]\n\r", pRace->who_name );
  if ( pRace->super_race >= 0 )
    send_to_charf( ch, "Super race:       [%s]\n\r", super_pc_race_table[pRace->super_race].name );
  else
    send_to_charf( ch, "Super race:     ???\n\r");
  send_to_charf( ch, "Experience:       [%d]\n\r", pRace->expl );
  send_to_charf( ch, "Type:             [%s]\n\r", flag_string( race_type_flags, pRace->type ) );
  send_to_charf( ch, "Attributes:\n\r" );
  for ( int i = 0; i < MAX_STATS; i++ )
    send_to_charf( ch, "  %13s:  [%2d]/[%2d]\n\r",
		   flag_string( attr_flags, ATTR_STR+i ),
		   pRace->stats[i], pRace->max_stats[i] );
  send_to_charf( ch, "Language:         [%s]\n\r", language_name(pRace->language) );
  send_to_charf( ch, "Abilities:        ");
  if ( pRace->nb_abilities == 0 )
    send_to_charf( ch, "No abilities\n\r");
  else 
    for ( int i = 0; i < pRace->nb_abilities; i++ ) {
      if ( i != 0 ) send_to_charf( ch, "                  ");
      send_to_charf( ch, "[%s]\n\r", ability_table[pRace->abilities[i]].name );
    }
  show_alignment( ch, pRace->allowed_align, pRace->nb_allowed_align );
  show_classes( ch, pRace->allowed_class, "Classes" );
  show_races( ch, pRace->remorts, pRace->nb_remorts, "Remort" );
  if ( pRace->min_remort_number > 0 )
    send_to_charf(ch,"Minimum remort:   [%d]\n\r", pRace->min_remort_number );
  //show_races( ch, pRace->rebirth_list, pRace->nb_rebirth, "Rebirth" );
  send_to_charf(ch,"Rebirth:          ");
  if ( pRace->nb_rebirth == 0 )
    send_to_charf( ch, "No Races\n\r" );
  else {
    int i;
    int total = 0;
    for ( i = 0; i < pRace->nb_rebirth; i++ ) {
      if ( i > 0 ) send_to_charf( ch, "                  " );
      total += pRace->rebirth_probability[i];
      send_to_charf( ch, "[%19s] [%3d]\n\r",
		     race_table[pRace->rebirth_list[i]].name,
		     pRace->rebirth_probability[i] );
    }
    if ( total != 100 )
      send_to_charf( ch, "                                 Total: [{r%3d{x]  {RDIFFERENT FROM 100{x\n\r", total );
    else
      send_to_charf( ch, "                                 Total: [%3d]\n\r", total );
  }
}
PCRACEEDIT( pcrace_edit_show ) {
  pc_race_type *pRace;

  EDIT_PCRACE(ch, pRace);

  pcrace_show( ch, pRace );

  return FALSE;
}
PCRACEEDIT( pcrace_edit_whoname ) {
  if ( argument[0] != '\0' ) {
    pc_race_type *pRace;
    EDIT_PCRACE(ch, pRace);
    strncpy( pRace->who_name, argument, 5 );
    send_to_charf(ch,"Who name set.\n\r");
    return TRUE;
  }
  send_to_charf(ch,"Syntax:  whoname [string max 5 char]\n\r");
  return FALSE;
}
PCRACEEDIT( pcrace_edit_experience ) {
  if ( argument[0] == '\0'
       || !is_number( argument ) ) {
    send_to_charf(ch,"Syntax:  experience [value]\n\r");
    return FALSE;
  }
  int expl = atoi( argument );
  if ( expl < 500 || expl > 3000 ) {
    send_to_charf(ch,"Experience value must be between 500 and 3000.\n\r");
    return FALSE;
  }
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  pRace->expl = expl;
  send_to_charf(ch,"Experience set.\n\r");
  return TRUE;
}
PCRACEEDIT( pcrace_edit_type ) {
  int type = flag_value( race_type_flags, argument );
  if ( argument[0] == '\0'
       || type == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  type [race type]\n\r"
		  "Type '? racetype' for a list of race type.\n\r" );
    return FALSE;
  }
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  pRace->type = type;
  send_to_charf(ch,"Race type set.\n\r");
  return TRUE;
}
PCRACEEDIT( pcrace_edit_attributes ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  int attr = flag_value( attr_flags, arg1 );
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
       || !is_number( arg2 ) || !is_number( arg3 )
       || attr < ATTR_STR || attr > ATTR_CON ) {
    send_to_charf(ch,"Syntax:  attributes [strength/../constitution] [base value] [max value]\n\r");
    return FALSE;
  }
  int v1 = atoi( arg2 );
  int v2 = atoi( arg3 );
  if ( v1 < 5 || v1 > 30
       || v2 < 5 || v2 > 30 
       || v2 < v1 + 8 ) {
    send_to_charf(ch,"Attributes value must be between 5 and 30.\n\r"
		  "Max value must be at least base value + 8.\n\r");
    return FALSE;
  }
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  pRace->stats[attr-ATTR_STR] = v1;
  pRace->max_stats[attr-ATTR_STR] = v2;
  send_to_charf(ch,"Race %s set.\n\r", flag_string( attr_flags, attr ) );
  return TRUE;
}
PCRACEEDIT( pcrace_edit_language ) {
  int ln = language_lookup( argument );
  if ( argument[0] == '\0'
       || ln < 0 ) {
    send_to_charf(ch,"Syntax:  language [language name]\n\r"
		  "Type 'help language' for a list of language.\n\r");
    return FALSE;
  }
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  pRace->language = ln;
  send_to_charf(ch,"Race language set.\n\r");
  return TRUE;
}
PCRACEEDIT( pcrace_edit_abilities ) {
  int sn = ability_lookup( argument );
  if ( argument[0] == '\0'
       || sn <= 0  ) {
    send_to_charf(ch,"Syntax:  abilities [ability name]\n\r"
		  "Type '? ability [skill/spell/power/song] [target]' for a list of abilities.\n\r");
    return FALSE;
  }
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  int aIndex;
  bool found = FALSE;
  for ( aIndex = 0; aIndex < pRace->nb_abilities; aIndex++ ) // search in race abilities
    if ( pRace->abilities[aIndex] == sn ) {
      found = TRUE;
      break;
    }
  if ( found ) { // found -> remove
    int *tmp;
    if ( ( tmp = (int*) GC_MALLOC( sizeof(int) * (pRace->nb_abilities-1) ) ) == NULL ) {
      bug("Can't allocate memory for race abilities");
      return FALSE;
    }
    int count = 0;
    for ( int i = 0; i < pRace->nb_abilities; i++ )
      if ( aIndex != i )
	tmp[count++] = pRace->abilities[i];
    pRace->nb_abilities--;
    pRace->abilities = tmp;
    send_to_charf(ch,"Race ability removed.\n\r");
  }
  else {         // not found -> add
    int *tmp;
    if ( ( tmp = (int*) GC_MALLOC( sizeof(int) * (pRace->nb_abilities+1) ) ) == NULL ) {
      bug("Can't allocate memory for race abilities");
      return FALSE;
    }
    bcopy( pRace->abilities, tmp, pRace->nb_abilities*sizeof(int));
    pRace->abilities = tmp;
    pRace->abilities[pRace->nb_abilities] = sn;
    pRace->nb_abilities++;
    send_to_charf(ch,"Race ability added.\n\r");
  }
  return TRUE;
}
PCRACEEDIT( pcrace_edit_alignment ) {
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  return edit_alignment( ch, pRace->allowed_align, pRace->nb_allowed_align, argument );
}
PCRACEEDIT( pcrace_edit_classes ) {
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  return edit_classes( ch, pRace->allowed_class, "Classes", argument );
}
PCRACEEDIT( pcrace_edit_remort ) {
  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  return edit_race( ch, pRace->remorts, pRace->nb_remorts, "remort", argument );
}
PCRACEEDIT( pcrace_edit_min_remort ) {
  int v = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || v < 0 ) {
    send_to_charf(ch,"Syntax:  minremort  <value>\n\r"
		  "value must be a positive number.\n\r" );
    return FALSE;
  }
  send_to_charf(ch,"Min number of remort set.\n\r");
  return TRUE;
}
PCRACEEDIT( pcrace_edit_rebirth ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);
  int iRace = race_lookup( arg1, TRUE );
  int prob = atoi( arg2 );
  if ( arg1[0] == '\0'
       || arg2[0] == '\0'
       || iRace < 0
       || !is_number(arg2)
       || prob < 0 ) {
    send_to_charf(ch,"Syntax:  rebirth <race name> <probability>\n\r"
		  "Type '? race' for a list of available races.\n\r");
    return FALSE;
  }
  if ( pc_race_table[iRace].type != RACE_REBIRTH ) {
    send_to_charf(ch,"Race [%s] is not a rebirth race\n\r", pc_race_table[iRace].name );
    return FALSE;
  }
  int idx = -1; // Find race in rebirth list
  for ( int i = 0; i < pRace->nb_rebirth; i++ )
    if ( pRace->rebirth_list[i] == iRace ) {
      idx = i;
      break;
    }
  if ( idx == -1 ) { // add
    int *tmp;
    int *tmp2;
    if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(pRace->nb_rebirth+1)) ) == NULL ) {
      bug("Can't allocate memory for race");
      send_to_charf(ch,"Can't allocate memory for race.\n\r");
      return FALSE;
    }
    if ( ( tmp2 = (int*)GC_MALLOC(sizeof(int)*(pRace->nb_rebirth+1)) ) == NULL ) {
      bug("Can't allocate memory for race");
      send_to_charf(ch,"Can't allocate memory for race.\n\r");
      return FALSE;
    }
    for ( int i = 0; i < pRace->nb_rebirth; i++ ) {
      tmp[i] = pRace->rebirth_list[i];
      tmp2[i] = pRace->rebirth_probability[i];
    }
    tmp[pRace->nb_rebirth] = iRace;
    tmp2[pRace->nb_rebirth] = prob;
    pRace->rebirth_list = tmp;
    pRace->rebirth_probability = tmp2;
    pRace->nb_rebirth++;
    send_to_charf(ch,"Rebirth race added.\n\r");
  }
  else { // remove
    int *tmp;
    int *tmp2;
    if ( ( tmp = (int*)GC_MALLOC(sizeof(int)*(pRace->nb_rebirth-1)) ) == NULL ) {
      bug("Can't allocate memory for race");
      send_to_charf(ch,"Can't allocate memory for race.\n\r");
      return FALSE;
    }
    if ( ( tmp2 = (int*)GC_MALLOC(sizeof(int)*(pRace->nb_rebirth-1)) ) == NULL ) {
      bug("Can't allocate memory for race");
      send_to_charf(ch,"Can't allocate memory for race.\n\r");
      return FALSE;
    }
    int count = 0;
    for ( int i = 0; i < pRace->nb_rebirth; i++ )
      if ( i != idx ) {
	tmp[count] = pRace->rebirth_list[i];
	tmp2[count] = pRace->rebirth_probability[i];
	count++;
      }
    pRace->rebirth_list = tmp;
    pRace->rebirth_probability = tmp2;
    pRace->nb_rebirth--;
    send_to_charf(ch,"Rebirth race removed.\n\r");
  }
  return TRUE;
}
PCRACEEDIT( pcrace_edit_superrace ) {
  int superId = super_race_lookup( argument );
  if ( superId < 0 ) {
    send_to_charf(ch,"Unknown super race [%s]\n\r"
		  "Type 'dlist superrace' for a list of super races\n\r", argument );
    return FALSE;
  }

  pc_race_type *pRace;
  EDIT_PCRACE(ch, pRace);

  int raceId = race_lookup( pRace->name, TRUE );

  //  remove from old super race
  remove_from_superrace( raceId );
  //  add in new super race
  add_to_school( superId, raceId );
  send_to_charf(ch,"Super race %s set.\n\r", super_pc_race_table[superId].name );

  return TRUE;
}

//*****************************************************************************
//****************              COMMANDS               ************************
//*****************************************************************************

void do_command_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( cmdnotsaved )
    new_save_commands();
  send_to_char("Commands have been saved.\n\r", ch );
}

#define COMMANDEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type command_edit_table[] = {
  {   "commands",	show_commands	},
  
  {   "position",       command_edit_position },
  {   "level",          command_edit_level },
  {   "log",            command_edit_log },
  {   "show",           command_edit_showing },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void command_edit( CHAR_DATA *ch, const char *argument ) {
  cmd_type *pCommand = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pCommand ) )
    return;
  EDIT_COMMAND( ch, pCommand );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit command.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    command_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; command_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, command_edit_table[cmd].name ) ) {
      if ( (*command_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	cmdnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing ability. */
void do_command_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit pc races.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a command name.\n\r"
		 "Syntax: commandedit <command name>\n\r", ch );
    return;
  }
  int iCmd = command_lookup(arg);
  if ( iCmd < 0 ) {
    send_to_charf(ch,"Command (%s) doesn't exist.\n\r", arg );
    return;
  }
  cmd_type *pCommand;
  pCommand = &(cmd_table[iCmd]);
  if ( ch->level < pCommand->level ) {
    send_to_charf(ch,"You are not high level enough to edit this command.\n\r");
    return;
  }

  if ( !OLC_EDIT( ch, pCommand ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pCommand;
  ch->desc->editor = ED_COMMAND;
  return;
}

static void command_show( CHAR_DATA *ch, const cmd_type *pCommand ) {
  send_to_charf( ch, "Command:    [%s]\n\r", pCommand->name );
  send_to_charf( ch, "Position:   [%s]\n\r", flag_string( position_flags, pCommand->position ) );
  send_to_charf( ch, "Level:      [%d]\n\r", pCommand->level );
  send_to_charf( ch, "Log:        [%s]\n\r", flag_string( log_flags, pCommand->log ) );
  send_to_charf( ch, "Showing:    [%s]\n\r", pCommand->show?"Yes":"No" );
}
COMMANDEDIT( command_edit_show ) {
  cmd_type *pCommand;
  EDIT_COMMAND(ch, pCommand);
  
  command_show( ch, pCommand );
  return FALSE;
}
COMMANDEDIT( command_edit_position ) {
  int position = flag_value_complete( position_flags, argument );
  if ( argument[0] == '\0'
       || position == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  position  [position name]\n\r"
		  "Type '? position'  for a list of position.\n\r");
    return FALSE;
  }
  cmd_type *pCommand;
  EDIT_COMMAND(ch, pCommand);
  pCommand->position = position;
  send_to_charf(ch,"Command position set.\n\r");
  return TRUE;
}
COMMANDEDIT( command_edit_level ) {
  int level = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument ) ) {
    send_to_charf(ch,"Syntax:  level [level]\n\r");
    return FALSE;
  }
  if ( level < 0 || level > MAX_LEVEL ) {
    send_to_charf(ch,"Level must be between 0 and %d.\n\r", MAX_LEVEL );
    return FALSE;
  }
  cmd_type *pCommand;
  EDIT_COMMAND(ch, pCommand);
  pCommand->level = level;
  send_to_charf(ch,"Command level set.\n\r");
  return TRUE;
}
COMMANDEDIT( command_edit_log ) {
  int log = flag_value( log_flags, argument );
  if ( argument[0] == '\0'
       || log == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  log  [log value]\n\r"
		  "Type '? log'  for a list of log value.\n\r");
    return FALSE;
  }
  cmd_type *pCommand;
  EDIT_COMMAND(ch, pCommand);
  pCommand->log = log;
  send_to_charf(ch,"Command log set.\n\r");
  return TRUE;
}
COMMANDEDIT( command_edit_showing ) {
  if ( argument[0] == '\0'
       || ( str_cmp( argument, "yes" )
	    && str_cmp( argument, "no" ) ) ) {
    send_to_charf(ch,"Syntax:  show  [yes/no]\n\r");
    return FALSE;
  }
  bool show = !str_cmp( argument, "yes" );
  cmd_type *pCommand;
  EDIT_COMMAND(ch, pCommand);
  pCommand->show = show;
  send_to_charf(ch,"Command show set.\n\r");
  return TRUE;
}

//****************************************************************************
//****************              CLASSES               ************************
//****************************************************************************

#define MAX_THAC0 (100)
#define MIN_THAC0 (-100)

void do_pcclasses_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( pcclassnotsaved )
    new_save_pc_classes();
  send_to_char("PC Classes have been saved.\n\r", ch );
}

#define PCCLASSEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )


const struct olc_cmd_type pcclass_edit_table[] = {
  {   "commands",	show_commands	},
  
  { "whoname",          pcclass_edit_whoname },
  { "attribute",        pcclass_edit_attribute },
  { "weapon",           pcclass_edit_weapon },
  { "adept",            pcclass_edit_adept },
  { "thac0_00",         pcclass_edit_thac0_00 },
  { "thac0_32",         pcclass_edit_thac0_32 },
  { "hpmin",            pcclass_edit_hpmin },
  { "hpmax",            pcclass_edit_hpmax },
  { "type",             pcclass_edit_type },
  { "basegroup",        pcclass_edit_base_group },
  { "defaultgruop",     pcclass_edit_default_group },
  { "parent",           pcclass_edit_parent },
  { "choosable",        pcclass_edit_choosable },
  { "alignment",        pcclass_edit_alignment },
  { "castingrule",      pcclass_edit_casting_rule },
  { "obj",              pcclass_edit_obj },
  { "create",           pcclass_edit_create },
  { "delete",           pcclass_edit_delete },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void pcclass_edit( CHAR_DATA *ch, const char *argument ) {
  class_type *pClass = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pClass ) )
    return;
  EDIT_PCCLASS( ch, pClass );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit command.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    pcclass_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; pcclass_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, pcclass_edit_table[cmd].name ) ) {
      if ( (*pcclass_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	pcclassnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing ability. */
void do_pcclass_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit pc races.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a class name.\n\r"
		 "Syntax: commandedit <command name>\n\r", ch );
    return;
  }

  class_type *pClass;
  if ( !str_cmp( arg, "create" ) ) {
    if ( pcclass_edit_create( ch, argument ) ) {
      pcclassnotsaved = TRUE;
      EDIT_PCCLASS( ch, pClass );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    pcclass_edit_delete( ch, argument );
    return;
  }
  else {
    int iClass = class_lookup(arg,TRUE);
    if ( iClass < 0 ) {
      send_to_charf(ch, "Class (%s) doesn't exist.\n\r", arg );
      return;
    }
    pClass = &(class_table[iClass]);
  }

  if ( !OLC_EDIT( ch, pClass ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pClass;
  ch->desc->editor = ED_PCCLASS;
  return;
}

static void pcclass_show( CHAR_DATA *ch, const class_type *pClass ) {
  send_to_charf( ch, "Name:             [%s]\n\r", pClass->name );
  send_to_charf( ch, "WhoName:          [%s]\n\r", pClass->who_name );
  send_to_charf( ch, "Attribute:        [%s]\n\r", flag_string(attr_flags, pClass->attr_prime+ATTR_stat0) );
  send_to_charf( ch, "Weapon:           [%d]\n\r", pClass->weapon );
  send_to_charf( ch, "Adept:            [%d]\n\r", pClass->ability_adept );
  send_to_charf( ch, "Thac0_00:         [%d]\n\r", pClass->thac0_00 );
  send_to_charf( ch, "Thac0_32:         [%d]\n\r", pClass->thac0_32 );
  send_to_charf( ch, "Hp min:           [%d]\n\r", pClass->hp_min );
  send_to_charf( ch, "Hp max:           [%d]\n\r", pClass->hp_max );
  send_to_charf( ch, "Type:             [%s]\n\r", flag_string( class_type_flags, pClass->type ) );
  //  send_to_charf( ch, "Casting rule:     [%d]\n\r", pClass->max_casting_rule );
  send_to_charf( ch, "Base group:       [%s]\n\r", pClass->base_group );
  send_to_charf( ch, "Default group:    [%s]\n\r", pClass->default_group );
  send_to_charf( ch, "Parent:           [%s]\n\r", class_table[ pClass->parent_class ].name );
  send_to_charf( ch, "Choosable:        [%s]\n\r", flag_string( class_choosable_flags, pClass->choosable ) );
  send_to_charf( ch, "Casting rule:     ");
  int count = 0;
  for ( int i = 0; i < ABILITY_TYPE_COUNT; i++ ) {
    const casting_rule_type *pRule = &(pClass->casting_rule[i]);
    if ( count != 0 )
      send_to_charf( ch, "                  " );
    send_to_charf(ch,"%s -> (%3d,%3d)\n\r", 
		  flag_string( ability_type_flags, i ), pRule->highest, pRule->other );
    count++;
  }
  if ( count == 0 )
    send_to_charf(ch,"\n\r");
  show_alignment( ch, pClass->allowed_align, pClass->nb_allowed_align );
  if ( pClass->num_obj_list == 0 )
    send_to_charf( ch, "Obj:              None\n\r");
  for ( int i = 0; i < pClass->num_obj_list; i++ ) {
    if ( i == 0 )
      send_to_charf( ch, "Obj:              [%d]\n\r", pClass->obj_list[i] );
    else
      send_to_charf( ch, "                  [%d]\n\r", pClass->obj_list[i] );
  }
}
PCCLASSEDIT( pcclass_edit_show ) {
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);

  pcclass_show( ch, pClass );

  return FALSE;
}
PCCLASSEDIT( pcclass_edit_whoname ) {
  if ( argument[0] != '\0' ) {
    class_type *pClass;
    EDIT_PCCLASS(ch, pClass);
    strncpy( pClass->who_name, argument, 5 );
    send_to_charf(ch,"Who name set.\n\r");
    return TRUE;
  }
  send_to_charf(ch,"Syntax:  whoname [string max 5 char]\n\r");
  return FALSE;
}
PCCLASSEDIT( pcclass_edit_attribute ) {
  int value = flag_value( attr_flags, argument );
  if ( argument[0] == '\0'
       || value == NO_FLAG
       || value < ATTR_STR
       || value > ATTR_CON ) {
    send_to_charf(ch,"Syntax:  attribute <strength/wisdom/.../constitution>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->attr_prime = value-ATTR_STR;
  send_to_charf(ch,"Prime attribute set.\n\r");
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_weapon ) {
  int vnum = atoi( argument );
  OBJ_INDEX_DATA *objIndex = get_obj_index( vnum );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || objIndex == NULL ) {
    send_to_charf( ch, "Syntax:  weapon <mudschool obj vnum>\n\r" );
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->weapon = vnum;
  send_to_charf( ch, "Weapon set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_adept ) {
  int value = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || value < 0 || value > 100 ) {
    send_to_charf( ch, "Syntax:  adept <percentage>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->ability_adept = value;
  send_to_charf( ch, "Adept set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_thac0_00 ) {
  int value = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || value < MIN_THAC0 || value > MAX_THAC0 ) {
    send_to_charf( ch, "Syntax:  thac0_00 <value>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->thac0_00 = value;
  send_to_charf( ch, "Thac0_00 set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_thac0_32 ) {
  int value = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || value < MIN_THAC0 || value > MAX_THAC0 ) {
    send_to_charf( ch, "Syntax:  thac0_32 <value>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->thac0_32 = value;
  send_to_charf( ch, "Thac0_32 set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_hpmin ) {
  int value = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || value < 0 ) {
    send_to_charf( ch, "Syntax:  hpmin <value>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->hp_min = value;
  send_to_charf( ch, "Hp min set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_hpmax ) {
  int value = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || value < 0 ) {
    send_to_charf( ch, "Syntax:  hpmax <value>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->hp_max = value;
  send_to_charf( ch, "Hp max set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_type ) {
  int value = flag_value( class_type_flags, argument );
  if ( argument[0] == '\0'
       || value == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  type  <class type>\n\r"
		  "Type '? classtype' for a list of class type.\n\r" );
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  TOGGLE_BIT( pClass->type, value ); // SinaC 2003
  send_to_charf( ch, "Class type toggled.\n\r" );
  if ( pClass->type == 0 || pClass->type == CLASS_WILDABLE )
    send_to_charf(ch,"At least one of the following type must be selected: magic, mental or combat.\n\r");
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_base_group ) {
  int gn = group_lookup( argument );
  if ( argument[0] == '\0'
       || gn < 0 ) {
    send_to_charf(ch,"Syntax:  basegroup <group name>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  int iClass = class_lookup( pClass->name, TRUE );
  if ( class_grouprating( iClass, gn ) <= 0 ) {
    send_to_charf(ch,"That group is not available for this class.\n\r");
    return FALSE;
  }
  pClass->base_group = str_dup( argument, TRUE );
  send_to_charf(ch,"Base group set.\n\r");
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_default_group ) {
  int gn = group_lookup( argument );
  if ( argument[0] == '\0'
       || gn < 0 ) {
    send_to_charf(ch,"Syntax:  defaultgroup <group name>\n\r");
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  int iClass = class_lookup( pClass->name, TRUE );
  if ( class_grouprating( iClass, gn ) <= 0 ) {
    send_to_charf(ch,"That group is not available for this class.\n\r");
    return FALSE;
  }
  pClass->default_group = str_dup( argument, TRUE );
  send_to_charf(ch,"Default group set.\n\r");
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_parent ) {
  int iClass = class_lookup( argument, TRUE );
  if ( argument[0] == '\0'
       || iClass <= 0 ) {
    send_to_charf(ch,"Syntax:  Parent <class name>\n\r"
		  "Type '? class' for a list of classes" );
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->parent_class = iClass;
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_choosable ) {
  int value = flag_value( class_choosable_flags, argument );
  if ( argument[0] == '\0'
       || value == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  choosable  <class choosable type>\n\r"
		  "Type '? classchoosable' for a list of class choosable.\n\r" );
    return FALSE;
  }
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  pClass->choosable = value;
  send_to_charf( ch, "Class type set.\n\r" );
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_alignment ) {
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  return edit_alignment( ch, pClass->allowed_align, pClass->nb_allowed_align, argument );
}
PCCLASSEDIT( pcclass_edit_obj ) {
  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);
  send_to_charf(ch,"Not yet implemented...neither used :)\n\r"); // TO DO
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_casting_rule ) {
// castingrule <ability type> <highest> <other>
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  int type = flag_value( ability_type_flags, arg1 );
  int highest = atoi(arg2);
  int other = atoi(arg3);
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
       || type == NO_FLAG
       || !is_number(arg2) || !is_number(arg3) ) {
    send_to_charf(ch,"Syntax:  casting rule <ability type> <highest> <other>\n\r"
		  "Type '? abilitytype'  for a list of ability type.\n\r");
    return FALSE;
  }

  class_type *pClass;
  EDIT_PCCLASS(ch, pClass);

  pClass->casting_rule[type].highest = highest;
  pClass->casting_rule[type].other = other;

  send_to_charf(ch,"Casting rule set.\n\r");
  return TRUE;
}
PCCLASSEDIT( pcclass_edit_create ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}
PCCLASSEDIT( pcclass_edit_delete ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}

//****************************************************************************
//****************                 GOD                ************************
//****************************************************************************

void do_god_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( godnotsaved )
    new_save_gods();
  send_to_char("Gods have been saved.\n\r", ch );
}

#define GODEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type god_edit_table[] = {
  {   "commands",	show_commands	},

  {   "title",          god_edit_title  },
  {   "minor",          god_edit_minor  },
  {   "major",          god_edit_major  },
  {   "classes",        god_edit_classes },
  {   "priests",        god_edit_priest },
  {   "races",          god_edit_races  },
  {   "story",          god_edit_story  },
  {   "alignment",      god_edit_alignment },
  {   "create",         god_edit_create },
  {   "delete",         god_edit_delete },
  {   "wild",           god_edit_wild },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void god_edit( CHAR_DATA *ch, const char *argument ) {
  god_data *pGod = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pGod ) )
    return;
  EDIT_GOD( ch, pGod );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit god.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    god_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; god_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, god_edit_table[cmd].name ) ) {
      if ( (*god_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	godnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing god. */
void do_god_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit god.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a god name.\n\r"
		 "Syntax: godedit <god name>\n\r", ch );
    return;
  }

  god_data *pGod;
  if ( !str_cmp( arg, "create" ) ) {
    if ( god_edit_create( ch, argument ) ) {
      godnotsaved = TRUE;
      EDIT_GOD( ch, pGod );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    god_edit_delete( ch, argument );
    return;
  }
  else {
    int iGod = god_lookup(arg);
    if ( iGod < 0 ) {
      send_to_charf(ch, "God (%s) doesn't exist.\n\r", arg );
      return;
    }
    pGod = &(gods_table[iGod]);
  }

  if ( !OLC_EDIT( ch, pGod ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pGod;
  ch->desc->editor = ED_GOD;
  return;
}

static void god_show( CHAR_DATA *ch, const god_data *pGod ) {
  send_to_charf(ch,"Name              [%s]\n\r", pGod->name );
  send_to_charf(ch,"Title             [%s]\n\r", pGod->title );
  send_to_charf(ch,"Minor sphere      [%s]\n\r", 
		pGod->minor_sphere < 0 ?"No minor":group_table[pGod->minor_sphere].name );
  send_to_charf(ch,"Major spheres     ");
  int col = 0;
  for ( int i = 0; i < pGod->nb_major_sphere; i++ ) {
    send_to_charf(ch,"%20s", group_table[pGod->major_sphere[i]].name );
    if ( ++col % 2 == 0 )
      send_to_charf(ch,"\n\r                  ");
  }
  if ( col % 2 != 0 )
      send_to_charf(ch,"\n\r");
  show_classes( ch, pGod->allowed_class, "Classes" );
  send_to_charf(ch,"Accept Wild Magic [%s]\n\r", pGod->acceptWildMagic?"Yes":"No");
  show_classes( ch, pGod->priest, "Priests" );
  show_alignment( ch, pGod->allowed_align, pGod->nb_allowed_align );
  show_races( ch, pGod->allowed_race, pGod->nb_allowed_race, "Race" );
  send_to_charf(ch,"Story:\n\r%s\n\r", pGod->story );
}
GODEDIT( god_edit_show ) {
  god_data *pGod;
  EDIT_GOD(ch, pGod);

  god_show( ch, pGod );

  return FALSE;
}
GODEDIT( god_edit_alignment ) {
  god_data *pGod;
  EDIT_GOD(ch, pGod);

  return edit_alignment( ch, pGod->allowed_align, pGod->nb_allowed_align, argument );
}
GODEDIT( god_edit_priest ) {
  god_data *pGod;
  EDIT_GOD(ch, pGod);

  return edit_classes( ch, pGod->priest, "Priests", argument );
}
GODEDIT( god_edit_classes ) {
  god_data *pGod;
  EDIT_GOD(ch, pGod);

  return edit_classes( ch, pGod->allowed_class, "Classes", argument );
}
GODEDIT( god_edit_title ) {
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  title  [title name]\n\r");
    return FALSE;
  }
  god_data *pGod;
  EDIT_GOD(ch, pGod);
  
  pGod->title = str_dup( buf, TRUE );
  send_to_charf(ch,"Title set.\n\r");
  return TRUE;
}
GODEDIT( god_edit_minor ) {
  int gn = group_lookup( argument );
  if ( argument[0] == '\0'
       && gn < 0 ) {
    send_to_charf(ch,"Syntax:  minor  [sphere name]\n\r");
    return FALSE;
  }

  if ( !group_table[gn].isSphere ) {
    send_to_charf(ch,"%s is a group but not a sphere.\n\r", group_table[gn].name );
    return FALSE;
  }

  god_data *pGod;
  EDIT_GOD(ch, pGod);

  pGod->minor_sphere = gn;

  send_to_charf(ch,"Minor sphere set.\n\r");
  return TRUE;
}
GODEDIT( god_edit_major ) {
  int gn = group_lookup( argument );
  if ( argument[0] == '\0'
       || gn < 0 ) {
    send_to_charf(ch,"Syntax:  major  [sphere name]\n\r");
    return FALSE;
  }
  if ( !group_table[gn].isSphere ) {
    send_to_charf(ch,"%s is a group but not a sphere.\n\r", group_table[gn].name );
    return FALSE;
  }

  god_data *pGod;
  EDIT_GOD(ch, pGod);

  int idx = -1;
  for ( int i = 0; i < pGod->nb_major_sphere; i++ )
    if ( pGod->major_sphere[i] == gn ) {
      idx = i;
      break;
    }
  if ( idx == -1 ) { // add
    int *tmp; // create new vector
    if ( ( tmp = (int *)GC_MALLOC( sizeof(int)*(pGod->nb_major_sphere+1) ) ) == NULL ) {
      bug("Can't allocate memory for god major spheres.");
      return FALSE;
    }
    bcopy(pGod->major_sphere,tmp,sizeof(int)*pGod->nb_major_sphere);
    tmp[pGod->nb_major_sphere] = gn;
    pGod->major_sphere = tmp;
    pGod->nb_major_sphere++;
    send_to_charf(ch,"Major sphere added.\n\r");
  }
  else { // remove
    int *tmp; // create new vector
    if ( ( tmp = (int *)GC_MALLOC( sizeof(int)*(pGod->nb_major_sphere-1) ) ) == NULL ) {
      bug("Can't allocate memory for god major spheres.");
      return FALSE;
    }
    int index = 0;
    for ( int i = 0; i < pGod->nb_major_sphere; i++ )
      if ( i != idx )
	tmp[index++] = pGod->major_sphere[i];
    pGod->major_sphere = tmp;
    pGod->nb_major_sphere--;
    send_to_charf(ch,"Major sphere removed.\n\r");
  }
  return TRUE;
}
GODEDIT( god_edit_races ) {
  god_data *pGod;
  EDIT_GOD(ch, pGod);
  return edit_race( ch, pGod->allowed_race, pGod->nb_allowed_race, "race", argument );
}
GODEDIT( god_edit_story ) {
  if ( argument[0] != '\0' ) {
    send_to_charf(ch,"Syntax:  story\n\r");
    return FALSE;
  }
  god_data *pGod;
  EDIT_GOD(ch, pGod);

  string_append( ch, &pGod->story );
  return TRUE;
}
GODEDIT( god_edit_create ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}
GODEDIT( god_edit_delete ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}
GODEDIT( god_edit_wild ) {
  god_data *pGod;
  EDIT_GOD(ch, pGod);
  
  pGod->acceptWildMagic = !pGod->acceptWildMagic;
  if ( pGod->acceptWildMagic )
    send_to_charf(ch,"Wild magicians accepted.\n\r");
  else
    send_to_charf(ch,"Wild magicians refused.\n\r");
  return TRUE;
}

//****************************************************************************
//****************               LIQUID               ************************
//****************************************************************************

void do_liquid_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( liqnotsaved )
    new_save_liquid();
  send_to_char("Liquids have been saved.\n\r", ch );
}

#define LIQUIDEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type liquid_edit_table[] = {
  {   "commands",	show_commands	},

  {   "color",          liquid_edit_color  },
  {   "drunk",          liquid_edit_drunk  },
  {   "full",           liquid_edit_full   },
  {   "thirst",         liquid_edit_thirst },
  {   "food",           liquid_edit_food   },
  {   "ssize",          liquid_edit_ssize  },
  {   "create",         liquid_edit_create },
  {   "delete",         liquid_edit_delete },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void liquid_edit( CHAR_DATA *ch, const char *argument ) {
  liq_type *pLiquid = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pLiquid ) )
    return;
  EDIT_LIQUID( ch, pLiquid );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit liquid.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    liquid_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; liquid_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, liquid_edit_table[cmd].name ) ) {
      if ( (*liquid_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	liqnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing liquid. */
void do_liquid_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit liquid.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a liquid name.\n\r"
		 "Syntax: liquidedit <liquid name>\n\r", ch );
    return;
  }
  liq_type *pLiquid;
  if ( !str_cmp( arg, "create" ) ) {
    if ( liquid_edit_create( ch, argument ) ) {
      liqnotsaved = TRUE;
      EDIT_LIQUID( ch, pLiquid );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    liquid_edit_delete( ch, argument );
    return;
  }
  else {
    int iLiq = liq_lookup(arg);
    if ( iLiq < 0 ) {
      send_to_charf(ch, "Liquid (%s) doesn't exist.\n\r", arg );
      return;
    }
    pLiquid = &(liq_table[iLiq]);
  }
  if ( !OLC_EDIT( ch, pLiquid ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pLiquid;
  ch->desc->editor = ED_LIQUID;
  return;
}

static void liquid_show( CHAR_DATA *ch, const liq_type *pLiquid ) {
  send_to_charf(ch,"Name               [%s]\n\r", pLiquid->liq_name);
  send_to_charf(ch,"Color              [%s]\n\r", pLiquid->liq_color);
  send_to_charf(ch,"Drunk              [%d]\n\r", pLiquid->liq_affect[0] );
  send_to_charf(ch,"Full               [%d]\n\r", pLiquid->liq_affect[1] );
  send_to_charf(ch,"Thirst             [%d]\n\r", pLiquid->liq_affect[2] );
  send_to_charf(ch,"Food               [%d]\n\r", pLiquid->liq_affect[3] );
  send_to_charf(ch,"Ssize              [%d]\n\r", pLiquid->liq_affect[4] );
}
LIQUIDEDIT( liquid_edit_show ) {
  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  liquid_show( ch, pLiquid );

  return FALSE;
}
LIQUIDEDIT( liquid_edit_color ) {
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  color  [color name]\n\r" );
    return FALSE;
  }

  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  pLiquid->liq_color = str_dup(buf, TRUE);

  send_to_charf(ch,"Color set.");
  return TRUE;
}
LIQUIDEDIT( liquid_edit_drunk ) {
  if ( argument[0] == '\0'
       || !is_number(argument ) ) {
    send_to_charf(ch,"Syntax:  drunk  [value]\n\r");
    return FALSE;
  }
  int value = atoi( argument );

  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  pLiquid->liq_affect[0] = value;

  send_to_charf(ch,"Drunk set.\n\r");
  return TRUE;
}
LIQUIDEDIT( liquid_edit_full ) {
  if ( argument[0] == '\0'
       || !is_number(argument ) ) {
    send_to_charf(ch,"Syntax:  full  [value]\n\r");
    return FALSE;
  }
  int value = atoi( argument );

  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  pLiquid->liq_affect[1] = value;

  send_to_charf(ch,"Full set.\n\r");
  return TRUE;
}
LIQUIDEDIT( liquid_edit_thirst ) {
  if ( argument[0] == '\0'
       || !is_number(argument ) ) {
    send_to_charf(ch,"Syntax:  thirst  [value]\n\r");
    return FALSE;
  }
  int value = atoi( argument );

  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  pLiquid->liq_affect[2] = value;

  send_to_charf(ch,"Thirst set.\n\r");
  return TRUE;
}
LIQUIDEDIT( liquid_edit_food ) {
  if ( argument[0] == '\0'
       || !is_number(argument ) ) {
    send_to_charf(ch,"Syntax:  food  [value]\n\r");
    return FALSE;
  }
  int value = atoi( argument );

  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  pLiquid->liq_affect[3] = value;

  send_to_charf(ch,"Food set.\n\r");
  return TRUE;
}
LIQUIDEDIT( liquid_edit_ssize ) {
  if ( argument[0] == '\0'
       || !is_number(argument ) ) {
    send_to_charf(ch,"Syntax:  ssize  [value]\n\r");
    return FALSE;
  }
  int value = atoi( argument );

  liq_type *pLiquid;
  EDIT_LIQUID(ch, pLiquid);

  pLiquid->liq_affect[4] = value;

  send_to_charf(ch,"Ssize set.\n\r");
  return TRUE;
}
bool copy_liquid( liq_type *from, liq_type *to ) {
  to->liq_name = str_dup( from->liq_name, TRUE );
  to->liq_color = str_dup( from->liq_color, TRUE );
  for ( int i = 0; i < 6; i++ )
    to->liq_affect[i] = from->liq_affect[i];
  return TRUE;
}
LIQUIDEDIT( liquid_edit_create ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: create <liquid name>\n\r");
    return FALSE;
  }
  if ( liq_lookup( arg2 ) >= 0 ) {
    send_to_charf(ch,"Liquid %s already exists.\n\r", arg2 );
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit liquid.\n\r", ch);
    return FALSE;
  }
  liq_type *tmp;
  if ( ( tmp = (liq_type*)GC_MALLOC(sizeof(liq_type)*(MAX_LIQUID+1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for liquid.\n\r");
    bug("Can't allocate memory for liquid");
    return FALSE;
  }
  // Copy liquid in new table
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    if ( !copy_liquid( &liq_table[i], &tmp[i] ) ) {
      send_to_charf(ch,"Problem while creating liquid");
      return FALSE;
    }
  }

  // Add new liquid
  tmp[MAX_LIQUID].liq_name = str_dup( arg2, TRUE );
  tmp[MAX_LIQUID].liq_color = str_dup( "no color", TRUE );
  for ( int i = 0; i < 6; i++ )
    tmp[MAX_LIQUID].liq_affect[i] = 0;

  // We must redirect pEdit for every people editing a liquid
  //  because liq_table is modified
  for ( int i = 0; i < MAX_LIQUID; i++ ) {
    liq_type *liquid = &(liq_table[i]);
    if ( liquid->lockBit 
	 && liquid->editedBy != NULL ) // edited
      liquid->editedBy->pEdit = &(tmp[i]); // change the pointer
  }

  liq_table = tmp;
  MAX_LIQUID++;

  update_help_table( "liquid", liq_table );

  ch->desc->pEdit = (void*)(&(liq_table[MAX_LIQUID-1])); // edit last liquid(new one)
  ch->desc->editor = ED_LIQUID;

  send_to_charf(ch,"Liquid %s created.\n\r", arg2 );
  return TRUE;
}
LIQUIDEDIT( liquid_edit_delete ) {
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: delete <liquid name> [<replacing liquid name>]\n\r");
    return FALSE;
  }
  int iLiquid = liq_lookup( arg2 );
  if ( iLiquid < 0 ) {
    send_to_charf(ch,"Liquid %s doesn't exist.\n\r",arg2);
    return FALSE;
  }
  int iLiquidReplace = -1;
  if ( arg3[0] != '\0'
       && ( ( iLiquidReplace = liq_lookup(arg3) ) < 0 ) ) {
    send_to_charf(ch,"Replacing liquid %s doesn't exist.\n\r", arg3 );
    return FALSE;
  }
  if ( iLiquidReplace == iLiquid ) {
    send_to_charf(ch,"You can't specify replacing liquid as deleted liquid.\n\r");
    return FALSE;
  }

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit liquid.\n\r", ch);
    return FALSE;
  }
  if ( iLiquidReplace == -1 ) {
    // search a fountain or a drink container with this liquid (OBJ_INDEX_DATA)
    for ( int i = 0; i < 65536; i++ ) {
      OBJ_INDEX_DATA *pObjIndex = get_obj_index( i );
      if ( pObjIndex != NULL
	   && ( pObjIndex->item_type == ITEM_DRINK_CON
		|| pObjIndex->item_type == ITEM_FOUNTAIN )
	   && pObjIndex->value[2] == iLiquid ) { // If no replacement specified -> error
	send_to_charf(ch,"Object vnum: %d  use this liquid but no replacing liquid\n\r"
		      " has been specified. Liquid deletion aborted.\n\r", i );
	return FALSE;
      }
    }
    // same thing for OBJ_DATA
    for ( OBJ_DATA *obj = object_list; obj; obj = obj->next )
      if ( ( obj->item_type == ITEM_DRINK_CON
	     || obj->item_type == ITEM_FOUNTAIN )
	   && ( obj->baseval[2] == iLiquid
		|| obj->value[2] == iLiquid ) ) { // If no replacement specified -> error
	send_to_charf(ch,"Object %s use this liquid but no replacing liquid\n\r"
		      " has been specified. Liquid deletion aborted.\n\r", obj->short_descr );
	return FALSE;
      }
  }
  
  liq_type *pLiquid = &(liq_table[iLiquid]);
  if ( !CAN_EDIT( ch, pLiquid ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name(pLiquid));
    return FALSE;
  }
  liq_type *tmp; // new liquid table
  if ( ( tmp = (liq_type*)GC_MALLOC(sizeof(liq_type)*(MAX_LIQUID-1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for liquid.\n\r");
    bug("Can't allocate memory for liquid");
    return FALSE;
  }
  int index = 0;
  for ( int i = 0; i < MAX_LIQUID; i++ )
    if ( i != iLiquid )
      if ( !copy_liquid( &liq_table[i], &tmp[index++] ) ) {
	send_to_charf(ch,"Problem while creating liquid");
	return FALSE;
      }

  // search a fountain or a drink container with this liquid (OBJ_INDEX_DATA)
  if ( iLiquidReplace != -1 ) {
    iLiquidReplace = UMAX( 0, iLiquidReplace>iLiquid?iLiquidReplace-1:iLiquidReplace );
    for ( int i = 0; i < 65536; i++ ) {
      OBJ_INDEX_DATA *pObjIndex = get_obj_index( i );
      if ( pObjIndex != NULL
	   && ( pObjIndex->item_type == ITEM_DRINK_CON
		|| pObjIndex->item_type == ITEM_FOUNTAIN ) ) {
	int liq = pObjIndex->value[2];
	if ( liq == iLiquid ) {
	  pObjIndex->value[2] = iLiquidReplace;
	  send_to_charf(ch,"Object vnum: %d  liquid replaced with %s\n\r",
			i, tmp[pObjIndex->value[2]].liq_name );
	}
	else
	  pObjIndex->value[2] = UMAX( 0, liq>iLiquid?liq-1:liq );
      }
    }
    // same thing for OBJ_DATA
    for ( OBJ_DATA *obj = object_list; obj; obj = obj->next )
      if ( obj->item_type == ITEM_DRINK_CON
	   || obj->item_type == ITEM_FOUNTAIN ) {
	int liq = obj->baseval[2];
	if ( liq == iLiquid )
	  obj->baseval[2] = obj->value[2] = iLiquidReplace;
	else
	  obj->baseval[2] = obj->value[2] = UMAX( 0, liq>iLiquid?liq-1:liq );
      }
  }
  
  send_to_charf(ch,"Liquid %s deleted.\n\r", arg2 );
  if ( pLiquid == ch->desc->pEdit ) // just deleted the liquid we were editing -> done
    edit_done(ch);

  // We must redirect pEdit for every people editing a liquid
  //  because liquid_table is modified
  index = 0;
  for ( int i = 0; i < MAX_LIQUID; i++ ) {
    liq_type *liquid = &(liq_table[i]);
    if ( liquid->lockBit 
	 && liquid->editedBy != NULL )  // edited
      liquid->editedBy->pEdit = &(tmp[index]); // change the pointer
    if ( i != iLiquid )
      index++;
  }
  
  liq_table = tmp;
  MAX_LIQUID--;

  update_help_table( "liquid", liq_table );

  return TRUE;
}

//****************************************************************************
//****************               MATERIAL             ************************
//****************************************************************************

void do_material_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( materialnotsaved )
    new_save_material();
  send_to_char("Materials have been saved.\n\r", ch );
}

#define MATERIALEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type material_edit_table[] = {
  {   "commands",	show_commands	},

  {   "color",          material_edit_color },
  {   "immunities",     material_edit_immunities },
  {   "resistances",    material_edit_resistances },
  {   "vulnerabilities",material_edit_vulnerabilies },
  {   "metallic",       material_edit_metallic },
  {   "create",         material_edit_create },
  {   "delete",         material_edit_delete },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void material_edit( CHAR_DATA *ch, const char *argument ) {
  material_type *pMaterial = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pMaterial ) )
    return;
  EDIT_MATERIAL( ch, pMaterial );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit material.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    material_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; material_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, material_edit_table[cmd].name ) ) {
      if ( (*material_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	materialnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing material */
void do_material_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit material.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a material name.\n\r"
		 "Syntax: materialedit <material name>\n\r", ch );
    return;
  }
  material_type *pMaterial;
  if ( !str_cmp( arg, "create" ) ) {
    if ( material_edit_create( ch, argument ) ) {
      materialnotsaved = TRUE;
      EDIT_MATERIAL( ch, pMaterial );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    material_edit_delete( ch, argument );
    return;
  }
  else {
    int iMat = material_lookup(arg);
    if ( iMat < 0 ) {
      send_to_charf(ch, "Material (%s) doesn't exist.\n\r", arg );
      return;
    }
    pMaterial = &(material_table[iMat]);
  }
  if ( !OLC_EDIT( ch, pMaterial ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pMaterial;
  ch->desc->editor = ED_MATERIAL;
  return;
}

static void material_show( CHAR_DATA *ch, const material_type *pMaterial ) {
  send_to_charf(ch,"Name                [%s]\n\r", pMaterial->name );
  send_to_charf(ch,"Color               [%s]\n\r", pMaterial->color );
  send_to_charf(ch,"Immunities          [%s]\n\r", flag_string( irv_flags, pMaterial->imm ) );
  send_to_charf(ch,"Resistances         [%s]\n\r", flag_string( irv_flags, pMaterial->res ) );
  send_to_charf(ch,"Vulnerabilities     [%s]\n\r", flag_string( irv_flags, pMaterial->vuln ) );
  send_to_charf(ch,"Metallic            [%s]\n\r", pMaterial->metallic?"Yes":"No");
}
MATERIALEDIT( material_edit_show ) {
  material_type *pMaterial;
  EDIT_MATERIAL(ch, pMaterial);
  
  material_show( ch, pMaterial );
  return FALSE;
}
MATERIALEDIT( material_edit_color ) {
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  color  [color name]\n\r");
    return FALSE;
  }

  material_type *pMaterial;
  EDIT_MATERIAL(ch, pMaterial);

  pMaterial->color = str_dup(buf, TRUE);

  send_to_charf(ch,"Color set.");
  return TRUE;
}
MATERIALEDIT( material_edit_immunities ) {
  int value = flag_value( irv_flags, argument );
  if ( argument[0] == '\0'
       || value == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  immunities  [imm flags]\n\r"
		  "Type '? imm' for a list of immunities.\n\r");
    return FALSE;
  }
  material_type *pMaterial;
  EDIT_MATERIAL(ch, pMaterial);

  pMaterial->imm = value;

  send_to_charf(ch,"Immunities set.\n\r");
  return TRUE;
}
MATERIALEDIT( material_edit_resistances ) {
  int value = flag_value( irv_flags, argument );
  if ( argument[0] == '\0'
       || value == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  resistances  [res flags]\n\r"
		  "Type '? res' for a list of resistances.\n\r");
    return FALSE;
  }
  material_type *pMaterial;
  EDIT_MATERIAL(ch, pMaterial);

  pMaterial->res = value;

  send_to_charf(ch,"Resistances set.\n\r");
  return TRUE;
}
MATERIALEDIT( material_edit_vulnerabilies ) {
  int value = flag_value( irv_flags, argument );
  if ( argument[0] == '\0'
       || value == NO_FLAG ) {
    send_to_charf(ch,"Syntax:  vulnerabilities  [vuln flags]\n\r"
		  "Type '? vuln' for a list of vulnerabilities.\n\r");
    return FALSE;
  }
  material_type *pMaterial;
  EDIT_MATERIAL(ch, pMaterial);

  pMaterial->vuln = value;

  send_to_charf(ch,"Vulnerabilities set.\n\r");
  return TRUE;
}
MATERIALEDIT( material_edit_metallic ) {
  int value = -1;
  if ( !str_cmp( argument, "yes" ) || !str_cmp( argument, "true" ) )
    value = 1;
  else if ( !str_cmp( argument, "no" ) || !str_cmp( argument, "false" ) )
    value = 0;
  if ( argument[0] == '\0'
       || value == -1 ) {
    send_to_charf(ch,"Syntax:  metallic  [yes/no]\n\r");
    return FALSE;
  }

  material_type *pMaterial;
  EDIT_MATERIAL(ch, pMaterial);

  pMaterial->metallic = value;
  send_to_charf(ch,"%setallic\n\r", !pMaterial->metallic?"Not m":"M");
  return TRUE;
}
bool copy_material( material_type *from, material_type *to ) {
  to->name = str_dup( from->name, TRUE );
  to->color = str_dup( from->color, TRUE );
  to->imm = from->imm;
  to->res = from->res;
  to->vuln = from->vuln;
  to->metallic = from->metallic;
  return TRUE;
}
MATERIALEDIT( material_edit_create ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: create <material name>\n\r");
    return FALSE;
  }
  if ( material_lookup( arg2 ) >= 0 ) {
    send_to_charf(ch,"Material %s already exists.\n\r", arg2 );
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit material.\n\r", ch);
    return FALSE;
  }
  material_type *tmp;
  if ( ( tmp = (material_type*)GC_MALLOC(sizeof(material_type)*(MAX_MATERIAL+1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for material.\n\r");
    bug("Can't allocate memory for material");
    return FALSE;
  }
  // Copy material in new table
  for ( int i = 0; i < MAX_MATERIAL; i++ ) {
    if ( !copy_material( &material_table[i], &tmp[i] ) ) {
      send_to_charf(ch,"Problem while creating material");
      return FALSE;
    }
  }

  // Add new material
  tmp[MAX_MATERIAL].name = str_dup( arg2, TRUE );
  tmp[MAX_MATERIAL].color = str_dup( "no color", TRUE );
  tmp[MAX_MATERIAL].imm = 0;
  tmp[MAX_MATERIAL].res = 0;
  tmp[MAX_MATERIAL].vuln = 0;
  tmp[MAX_MATERIAL].metallic = FALSE;

  // We must redirect pEdit for every people editing a material
  //  because material_table is modified
  for ( int i = 0; i < MAX_MATERIAL; i++ ) {
    material_type *material = &(material_table[i]);
    if ( material->lockBit 
	 && material->editedBy != NULL ) // edited
      material->editedBy->pEdit = &(tmp[i]); // change the pointer
  }

  material_table = tmp;
  MAX_MATERIAL++;

  update_help_table( "material", material_table );

  ch->desc->pEdit = (void*)(&(material_table[MAX_MATERIAL-1])); // edit last material(new one)
  ch->desc->editor = ED_MATERIAL;

  send_to_charf(ch,"Material %s created.\n\r", arg2 );
  return TRUE;
}
MATERIALEDIT( material_edit_delete ) {
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: delete <material name> [<replacing material name>]\n\r");
    return FALSE;
  }
  int iMaterial = material_lookup( arg2 );
  if ( iMaterial < 0 ) {
    send_to_charf(ch,"Material %s doesn't exist.\n\r",arg2);
    return FALSE;
  }
  int iMaterialReplace = -1;
  if ( arg3[0] != '\0'
       && ( ( iMaterialReplace = material_lookup( arg3) ) < 0 ) ) {
    send_to_charf(ch,"Replacing material %s doesn't exist.\n\r", arg3 );
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit material.\n\r", ch);
    return FALSE;
  }

  // search OBJ_INDEX_DATA with this material
  if ( iMaterialReplace == -1 ) {
    for ( int i = 0; i < 65536; i++ ) {
      OBJ_INDEX_DATA *pObjIndex = get_obj_index( i );
      if ( pObjIndex != NULL
	   && pObjIndex->material == iMaterial ) { // If no replacement specified -> error
	send_to_charf(ch,"Object vnum: %d  use this material but no replacing material\n\r"
		      " has been specified. Material deletion aborted.\n\r", i );
	return FALSE;
      }
    }
    // same thing for OBJ_DATA
    for ( OBJ_DATA *obj = object_list; obj; obj = obj->next )
      if ( obj->material == iMaterial ) { // If no replacement specified -> error
	send_to_charf(ch,"Object %s use this liquid but no replacing material\n\r"
		      " has been specified. Material deletion aborted.\n\r", obj->short_descr );
	return FALSE;
      }
  }

  material_type *pMaterial = &(material_table[iMaterial]);
  if ( !CAN_EDIT( ch, pMaterial ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name(pMaterial));
    return FALSE;
  }
  material_type *tmp; // new material table
  if ( ( tmp = (material_type*)GC_MALLOC(sizeof(material_type)*(MAX_MATERIAL-1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for material.\n\r");
    bug("Can't allocate memory for material");
    return FALSE;
  }
  int index = 0;
  for ( int i = 0; i < MAX_MATERIAL; i++ )
    if ( i != iMaterial )
      if ( !copy_material( &material_table[i], &tmp[index++] ) ) {
	send_to_charf(ch,"Problem while creating material");
	return FALSE;
      }

  if ( iMaterialReplace != -1 ) {
    iMaterialReplace = UMAX( 0, iMaterialReplace>iMaterial?iMaterialReplace-1:iMaterialReplace );
    for ( int i = 0; i < 65536; i++ ) {
      OBJ_INDEX_DATA *pObjIndex = get_obj_index( i );
      if ( pObjIndex != NULL ) {
	int mat = pObjIndex->material;
	if ( mat == iMaterial ) {
	  pObjIndex->material = iMaterialReplace;
	  send_to_charf(ch,"Object vnum: %d  material replaced with %s\n\r",
			i, tmp[iMaterialReplace].name );
	}
	else
	  pObjIndex->material = UMAX( 0, mat>iMaterial?mat-1:mat );
      }
    }
    // same thing for OBJ_DATA
    for ( OBJ_DATA *obj = object_list; obj; obj = obj->next ) {
      int mat = obj->material;
      if (  mat == iMaterial )
	obj->material = iMaterialReplace;
      else
	obj->material = UMAX( 0, mat>iMaterial?mat-1:mat );
    }
  }
  
  send_to_charf(ch,"Material %s deleted.\n\r", arg2 );
  if ( pMaterial == ch->desc->pEdit ) // just deleted the material we were editing -> done
    edit_done(ch);

  // We must redirect pEdit for every people editing a material
  //  because material_table is modified
  index = 0;
  for ( int i = 0; i < MAX_MATERIAL; i++ ) {
    material_type *material = &(material_table[i]);
    if ( material->lockBit 
	 && material->editedBy != NULL )  // edited
      material->editedBy->pEdit = &(tmp[index]); // change the pointer
    if ( i != iMaterial )
      index++;
  }

  material_table = tmp;
  MAX_MATERIAL--;

  update_help_table( "material", material_table );

  return TRUE;
}

//****************************************************************************
//****************            BREW FORMULAS           ************************
//****************************************************************************

void do_brew_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( brewnotsaved )
    new_save_brew_formula();
  send_to_char("Brew formulas have been saved.\n\r", ch );
}

#define BREWEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type brew_edit_table[] = {
  {   "commands",	show_commands	},

  {   "name",           brew_edit_name  },
  {   "level",          brew_edit_level },
  {   "cost",           brew_edit_cost  },
  //{   "components",     brew_edit_components },
  {   "effects",        brew_edit_effects },
  {   "create",         brew_edit_create },
  {   "delete",         brew_edit_delete },
  {   "add",            brew_edit_add_components },
  {   "rem",            brew_edit_del_components },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void brew_edit( CHAR_DATA *ch, const char *argument ) {
  brew_formula_type *pBrew = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pBrew ) )
    return;
  EDIT_BREW( ch, pBrew );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit brew formula.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    brew_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; brew_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, brew_edit_table[cmd].name ) ) {
      if ( (*brew_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	brewnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing brew formula */
void do_brew_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit brew formulas.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a brew formula name.\n\r"
		 "Syntax: brewedit <brew formula name>\n\r", ch );
    return;
  }
  brew_formula_type *pBrew;

  if ( !str_cmp( arg, "create" ) ) {
    if ( brew_edit_create( ch, argument ) ) {
      brewnotsaved = TRUE;
      EDIT_BREW( ch, pBrew );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    brew_edit_delete( ch, argument );
    return;
  }
  else {
    int iBrew = brew_lookup(arg);
    if ( iBrew < 0 ) {
      send_to_charf(ch, "Brew (%s) doesn't exist.\n\r", arg );
      return;
    }
    pBrew = &(brew_formula_table[iBrew]);
  }
  if ( !OLC_EDIT( ch, pBrew ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pBrew;
  ch->desc->editor = ED_BREW;
  return;
}

static void brew_show( CHAR_DATA *ch, const brew_formula_type *pBrew ) {
  send_to_charf(ch,"Name:                   [%s]\n\r", pBrew->name );
  send_to_charf(ch,"Potion level:           [%d]\n\r", pBrew->level );
  send_to_charf(ch,"Potion cost:            [%d]\n\r", pBrew->cost );
  send_to_charf(ch,"Components:             ");
  if ( pBrew->num_component == 0 )
    send_to_charf(ch,"     No components.\n\r" );
  else
    for ( int i = 0; i < pBrew->num_component; i++ ) {
      if ( i != 0 )
	send_to_charf(ch,"                        ");
      //send_to_charf(ch,"vnum [%d]\n\r", pBrew->component_list[i]);
      if ( pBrew->component_list[i] != NO_FLAG )
	send_to_charf(ch,"[%s]\n\r", flag_string(brew_component_flags,pBrew->component_list[i]));
      else
	send_to_charf(ch,"[{rINVALID{x]\n\r");
    }
  send_to_charf(ch,"Effects:                " );
  if ( pBrew->num_effect == 0 )
    send_to_charf(ch, "No effects.\n\r");
  else
    for ( int i = 0; i < pBrew->num_effect; i++ ) {
      if ( i != 0 )
	send_to_charf(ch,"                        ");
      send_to_charf(ch,"[%s]\n\r", ability_table[pBrew->effect_list[i]].name );
    }
}
BREWEDIT( brew_edit_show ) {
  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  brew_show( ch, pBrew );
  return FALSE;
}
BREWEDIT( brew_edit_name ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  name  [potion name]\n\r");
    return FALSE;
  }

  if ( !is_valid_name( argument ) ) {
    send_to_charf(ch,"[%s] is not valid name.\n\r", argument );
    return FALSE;
  }

  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  pBrew->name = str_dup( buf, TRUE );
  
  send_to_charf(ch,"Name set.\n\r");
  return TRUE;
}
BREWEDIT( brew_edit_level ) {
  int val = atoi(argument);
  if ( argument[0] == '\0'
       || !is_number(argument)
       || val < 1
       || val > LEVEL_HERO ) {
    send_to_charf(ch,"Syntax:  level  [potion level]\n\r"
		  "  [potion level] must be between 1 and %d.\n\r", LEVEL_HERO );
    return FALSE;
  }

  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  pBrew->level = val;

  send_to_charf(ch,"Level set.\n\r");
  return TRUE;
}
BREWEDIT( brew_edit_cost ) {
  int val = atoi( argument );
  if ( argument[0] == '\0'
       || !is_number( argument )
       || val < 0 ) {
    send_to_charf(ch,"Syntax:  cost  [potion cost]\n\r"
		  "  [potion cost] must be an non-negative number.\n\r" );
    return FALSE;
  }
  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  pBrew->cost = val;

  send_to_charf(ch,"Cost set.\n\r");
  return TRUE;
}
BREWEDIT( brew_edit_add_components ) {
  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  if ( pBrew->num_component >= MAX_COMPONENT ) {
    send_to_charf(ch, "Maximum number of components (%d) already reached.\n\r", MAX_COMPONENT );
    return FALSE;
  }
  int id = flag_value( brew_component_flags, argument );
  if ( id == NO_FLAG ) {
    send_to_charf(ch,"Invalid component [%s]\n\r"
		  "Type '? component' for a list of components\n\r", argument );
    return FALSE;
  }

  int *tmp;
  if ( ( tmp = (int*)GC_MALLOC((pBrew->num_component+1)*sizeof(int))) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for components.\n\r");
    bug("Can't allocate memory for components.");
    return FALSE;
  }
  for ( int i = 0; i < pBrew->num_component; i++ )
    tmp[i] = pBrew->component_list[i];
  tmp[pBrew->num_component] = id;
  pBrew->component_list = tmp;
  pBrew->num_component++;
  send_to_charf(ch,"Component added.\n\r");
  return TRUE;
}
BREWEDIT( brew_edit_del_components ) {
  int id = flag_value( brew_component_flags, argument );
  if ( id == NO_FLAG ) {
    send_to_charf(ch,"Unknown component [%s]\n\r"
		  "Type '? component' for a list of components\n\r", argument );
    return FALSE;
  }

  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  int index = -1;
  for ( int i = 0; i < pBrew->num_component; i++ )
    if ( pBrew->component_list[i] == id ) {
      index = i;
      break;
    }

  if ( index == -1 ) {
    send_to_charf(ch, "Component [%s] is not in brew component list\n\r", argument );
    return FALSE;
  }

  int *tmp;
  if ( ( tmp = (int*)GC_MALLOC((pBrew->num_component-1)*sizeof(int))) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for components.\n\r");
    bug("Can't allocate memory for components.");
    return FALSE;
  }
  int count = 0;
  for ( int i = 0; i < pBrew->num_component; i++ )
    if ( i != index )
      tmp[count++] = pBrew->component_list[i];
  pBrew->component_list = tmp;
  pBrew->num_component--;
  send_to_charf(ch,"Component removed.\n\r");
  return TRUE;
}
//BREWEDIT( brew_edit_components ) {
//  int val = atoi( argument );
//  if ( argument[0] == '\0'
//       || !is_number( argument ) ) {
//    send_to_charf(ch,"Syntax:  components  [component vnum]\n\r");
//    return FALSE;
//  }
//  OBJ_INDEX_DATA *pObjIndex = get_obj_index( val );
//  if ( pObjIndex == NULL ) {
//    send_to_charf(ch,"Obj vnum [%d] doesn't exist.\n\r", val );
//    return FALSE;
//  }
//
//
//  // Search wanted component in component list
//  int index = -1;
//  for ( int i = 0; i < pBrew->num_component; i++ )
//    if ( pBrew->component_list[i] == val ) {
//      index = i;
//      break;
//    }
//
//  if ( index == -1 ) { // Add a component
//  }
//  else { // Remove a component
//  }
//
//  return TRUE;
//}
BREWEDIT( brew_edit_effects ) {
  int sn = ability_lookup( argument );
  if ( argument[0] == '\0'
       || sn < 0 ) {
    send_to_charf(ch,"Syntax:  effects  [ability name]\n\r");
    return FALSE;
  }

  brew_formula_type *pBrew;
  EDIT_BREW(ch, pBrew);

  // Search wanted effect
  int index = -1;
  for ( int i = 0; i < pBrew->num_effect; i++ )
    if ( pBrew->effect_list[i] == sn ) {
      index = i;
      break;
    }

  if ( index == -1 ) { // Add an effect
    if ( pBrew->num_effect >= MAX_BREW_SPELLS ) {
      send_to_charf(ch, "Maximum number of effects (%d) already reached.\n\r", MAX_BREW_SPELLS );
      return FALSE;
    }
    ability_type *ab = &(ability_table[sn]);
    if ( ab->type != TYPE_SPELL
	 || ( ab->target != TAR_CHAR_DEFENSIVE
	      && ab->target != TAR_CHAR_SELF ) ) {
      send_to_charf(ch,"You can't only add defensive or self spell.\n\r"
		    "Type '? ability spell defensive' for a list of available spells.\n\r"
		    " or '? ability spell self'\n\r");
      return FALSE;
    }
    int *tmp;
    if ( ( tmp = (int*)GC_MALLOC((pBrew->num_effect+1)*sizeof(int))) == NULL ) {
      send_to_charf(ch,"Can't allocate memory for effects.\n\r");
      bug("Can't allocate memory for effects.");
      return FALSE;
    }
    for ( int i = 0; i < pBrew->num_effect; i++ )
      tmp[i] = pBrew->effect_list[i];
    tmp[pBrew->num_effect] = sn;
    pBrew->effect_list = tmp;
    pBrew->num_effect++;
    send_to_charf(ch,"Effect added.\n\r");
  }
  else { // Remove an effect
    int *tmp;
    if ( ( tmp = (int*)GC_MALLOC((pBrew->num_effect-1)*sizeof(int))) == NULL ) {
      send_to_charf(ch,"Can't allocate memory for effects.\n\r");
      bug("Can't allocate memory for effects.");
      return FALSE;
    }
    int count = 0;
    for ( int i = 0; i < pBrew->num_effect; i++ )
      if ( i != index )
	tmp[count++] = pBrew->effect_list[i];
    pBrew->effect_list = tmp;
    pBrew->num_effect--;
    send_to_charf(ch,"Effect removed.\n\r");
  }

  return TRUE;
}
bool copy_brew_formula( brew_formula_type *from, brew_formula_type *to ) {
  to->name = str_dup( from->name, TRUE );
  to->cost = from->cost;
  to->level = from->level;
  to->num_component = from->num_component;
  if ( ( to->component_list = (int*) GC_MALLOC_ATOMIC( to->num_component * sizeof(int) ) ) == NULL ) {
    bug("Can't allocate memory for component");
    return FALSE;
  }
  for ( int i = 0; i < to->num_component; i++ )
    to->component_list[i] = from->component_list[i];
  to->num_effect = from->num_component;
  if ( ( to->effect_list = (int*) GC_MALLOC_ATOMIC( to->num_effect * sizeof(int) ) ) == NULL ) {
    bug("Can't allocate memory for effect");
    return FALSE;
  }
  for ( int i = 0; i < to->num_effect; i++ )
    to->effect_list[i] = from->effect_list[i];
  return TRUE;
}
BREWEDIT( brew_edit_create ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: create <brew formula name>\n\r");
    return FALSE;
  }
  if ( brew_lookup( arg2 ) >= 0 ) {
    send_to_charf(ch,"Brew formula %s already exists.\n\r", arg2 );
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit brew formula.\n\r", ch);
    return FALSE;
  }
  brew_formula_type *tmp;
  if ( ( tmp = (brew_formula_type*)GC_MALLOC(sizeof(brew_formula_type)*(MAX_BREW_FORMULA+1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for brew formula.\n\r");
    bug("Can't allocate memory for brew formula");
    return FALSE;
  }
  // Copy formula in new table
  for ( int i = 0; i < MAX_BREW_FORMULA; i++ ) {
    if ( !copy_brew_formula( &brew_formula_table[i], &tmp[i] ) ) {
      send_to_charf(ch,"Problem while creating brew formula");
      return FALSE;
    }
  }

  // Add new formula
  tmp[MAX_BREW_FORMULA].name = str_dup( arg2, TRUE );
  tmp[MAX_BREW_FORMULA].num_component = 0;
  tmp[MAX_BREW_FORMULA].num_effect = 0;
  tmp[MAX_BREW_FORMULA].cost = 0;
  tmp[MAX_BREW_FORMULA].level = 0;

  // We must redirect pEdit for every people editing a brew formula
  //  because brew_formula_table is modified
  for ( int i = 0; i < MAX_BREW_FORMULA; i++ ) {
    brew_formula_type *formula = &(brew_formula_table[i]);
    if ( formula->lockBit 
	 && formula->editedBy != NULL ) // edited
      formula->editedBy->pEdit = &(tmp[i]); // change the pointer
  }

  brew_formula_table = tmp;
  MAX_BREW_FORMULA++;

  ch->desc->pEdit = (void*)(&(brew_formula_table[MAX_BREW_FORMULA-1])); // edit last formula(new one)
  ch->desc->editor = ED_BREW;

  send_to_charf(ch,"Brew formula %s created.\n\r", arg2 );
  return TRUE;
}
BREWEDIT( brew_edit_delete ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: delete <brew formula name>\n\r");
    return FALSE;
  }
  int iBrew = brew_lookup( arg2 );
  if ( iBrew < 0 ) {
    send_to_charf(ch,"Brew formula %s doesn't exist.\n\r",arg2);
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit brew formula.\n\r", ch);
    return FALSE;
  }
  brew_formula_type *pFormula = &(brew_formula_table[iBrew]);
  if ( !CAN_EDIT( ch, pFormula ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name(pFormula));
    return FALSE;
  }
  brew_formula_type *tmp; // new brew formula table
  if ( ( tmp = (brew_formula_type*)GC_MALLOC(sizeof(brew_formula_type)*(MAX_BREW_FORMULA-1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for brew formula.\n\r");
    bug("Can't allocate memory for brew formula");
    return FALSE;
  }
  int index = 0;
  for ( int i = 0; i < MAX_BREW_FORMULA; i++ )
    if ( i != iBrew )
      if ( !copy_brew_formula( &brew_formula_table[i], &tmp[index++] ) ) {
	send_to_charf(ch,"Problem while creating brew formula");
	return FALSE;
      }

  send_to_charf(ch,"Brew formula %s deleted.\n\r", arg2 );
  if ( pFormula == ch->desc->pEdit ) // just deleted the formula we were editing -> done
    edit_done(ch);


  // This loop could be problematic if last formula was deleted by
  //  someone else than its editor but this cannot happen because
  //  we refuse deleting a data edited by someone else
  // We must redirect pEdit for every people editing a brew formula
  //  because brew_formula_table is modified
  index = 0;
  for ( int i = 0; i < MAX_BREW_FORMULA; i++ ) {
    brew_formula_type *formula = &(brew_formula_table[i]);
    if ( formula->lockBit 
	 && formula->editedBy != NULL )  // edited
      formula->editedBy->pEdit = &(tmp[index]); // change the pointer
    if ( i != iBrew )
      index++;
  }

  brew_formula_table = tmp;
  MAX_BREW_FORMULA--;


  return TRUE;
}

//****************************************************************************
//****************               GROUPS               ************************
//****************************************************************************

void do_group_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( groupnotsaved )
    new_save_groups();
  if ( spherenotsaved )
    new_save_spheres();
  if ( pcclassnotsaved )
    new_save_pc_classes();
  if ( spherenotsaved )
    new_save_spheres();
  send_to_char("Groups/spheres and classes have been saved.\n\r", ch );
}

#define GROUPEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type group_edit_table[] = {
  {   "commands",	show_commands	},

  {   "contains",       group_edit_contains },
  {   "rating",         group_edit_rating },
  {   "sphere",         group_edit_sphere },
  {   "godrating",      group_edit_god_rating },
  {   "create",         group_edit_create },
  {   "delete",         group_edit_delete },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void group_edit( CHAR_DATA *ch, const char *argument ) {
  group_type *pGroup = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pGroup ) )
    return;
  EDIT_GROUP( ch, pGroup );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit group.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    group_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; group_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, group_edit_table[cmd].name ) ) {
      if ( (*group_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	groupnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing group */
void do_group_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit groups.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a group name.\n\r"
		 "Syntax: groupedit <group name>\n\r", ch );
    return;
  }
  group_type *pGroup;
  if ( !str_cmp( arg, "create" ) ) {
    if ( group_edit_create( ch, argument ) ) {
      groupnotsaved = TRUE;
      EDIT_GROUP( ch, pGroup );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    group_edit_delete( ch, argument );
    return;
  }
  else {
    int iGroup = group_lookup(arg);
    if ( iGroup < 0 ) {
      send_to_charf(ch, "Group (%s) doesn't exist.\n\r", arg );
      return;
    }
    pGroup = &(group_table[iGroup]);
  }

  if ( !OLC_EDIT( ch, pGroup ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pGroup;
  ch->desc->editor = ED_GROUP;
  return;
}

static void group_show( CHAR_DATA *ch, const group_type *pGroup ) {
  send_to_charf(ch,"Name:               [%s]\n\r", pGroup->name );
  send_to_charf(ch,"Sphere:             [%s]\n\r", pGroup->isSphere?"Yes":"No");
  send_to_charf(ch,"Contains:           ");
  if ( pGroup->spellnb == 0 )
    send_to_charf(ch,"No group/abilities.\n\r");
  else
    for ( int i = 0; i < pGroup->spellnb; i++ ) {
      if ( i != 0 )
	send_to_charf(ch,"                    ");
      int gn = group_lookup(pGroup->spells[i]);
      send_to_charf(ch,"%s %s\n\r",gn<0?"<Ability>":"  <Group>",pGroup->spells[i]);
    }

  char gr_rat[MAX_STRING_LENGTH];
  char cl_nam[MAX_STRING_LENGTH];
  gr_rat[0]='\0';
  cl_nam[0]='\0';
  bool found = FALSE;
  for ( int i = 0; i < MAX_CLASS; i++ ) {
    int rate = pGroup->rating[i];
    //if ( rate >= 0 ) {
    if ( rate > 0 ) { // FIXME: SinaC 2003
      char conv[16];
      found = TRUE;
      sprintf(conv," %3d",rate);
      strcat(gr_rat, conv);
      
      sprintf(conv, " %s",class_table[i].who_name );
      strcat(cl_nam, conv);
    }
  }
  if ( found )
    send_to_charf( ch, 
		   "Classes:           %s\n\r"
		   "Rating:            %s\n\r",
		   cl_nam, gr_rat );
  else
    send_to_charf(ch,"NOT AVAILABLE FOR ANY CLASSES.\n\r");

  if ( pGroup->isSphere ) {
    char gr_rat[MAX_STRING_LENGTH];
    char god_nam[MAX_STRING_LENGTH];
    gr_rat[0]='\0';
    god_nam[0]='\0';
    bool found = FALSE;
    for ( int i = 0; i < MAX_GODS; i++ ) {
      int rate = pGroup->god_rate[i];
      if ( rate > 0 ) {
	char conv[16];
	found = TRUE;
	sprintf(conv," %3d",rate);
	strcat(gr_rat, conv);
	
	sprintf(conv, " %-3s",gods_table[i].name );
	strcat(god_nam, conv);
      }
    }
    if ( found )
      send_to_charf( ch, 
		     "God:               %s\n\r"
		     "Rating:            %s\n\r",
		     god_nam, gr_rat );
    else
      send_to_charf(ch,"NOT AVAILABLE FOR ANY GODS.\n\r");
  }
}
GROUPEDIT( group_edit_show ) {
  group_type *pGroup;
  EDIT_GROUP(ch, pGroup);

  group_show( ch, pGroup );
  return FALSE;
}
GROUPEDIT( group_edit_contains ) {
  int gn = group_lookup( argument );
  int sn = ability_lookup( argument );
  if ( argument[0] == '\0'
       || ( gn < 0 && sn < 0 ) ) {
    send_to_charf(ch,"Syntax:  contains  <group/ability name>\n\r"
		  "Type '? ability <skill/spell/power/song> <target>' for a list of abilities.\n\r"
		  "Type '? groupshow <group name> for a list of groups.\n\r");
    return FALSE;
  }

  group_type *pGroup;
  EDIT_GROUP(ch, pGroup);

  int iGroup = group_lookup( pGroup->name );
  if ( gn >= 0 && iGroup == gn ) {
    send_to_charf(ch,"You can create recursive group contains.\n\r");
    return FALSE;
  }

  const char *s;
  if ( gn >= 0 )
    s = group_table[gn].name;
  else
    s = ability_table[sn].name;
 
  int index = -1;
  for ( int i = 0; i < pGroup->spellnb; i++ ) // Search ability/group in list
    if ( !str_cmp( pGroup->spells[i], s ) ) {
      index = i;
      break;
    }

  if ( index == -1 ) { // add
    const char **tmp;
    if ( ( tmp = (const char**)GC_MALLOC(sizeof(const char*)*(pGroup->spellnb+1))) == NULL ) {
      send_to_charf(ch,"Can't allocate memory for group contains");
      return FALSE;
    }
    for ( int i = 0; i < pGroup->spellnb; i++ )
      tmp[i] = str_dup( pGroup->spells[i], TRUE );
    tmp[pGroup->spellnb] = str_dup( s, TRUE );
    pGroup->spells = tmp;
    pGroup->spellnb++;
    send_to_charf(ch,"%s added.\n\r", gn >= 0 ?"Group":"Ability");
  }
  else { // remove
    const char **tmp;
    if ( ( tmp = (const char**)GC_MALLOC(sizeof(const char*)*(pGroup->spellnb-1))) == NULL ) {
      send_to_charf(ch,"Can't allocate memory for group contains");
      return FALSE;
    }
    int count = 0;
    for ( int i = 0; i < pGroup->spellnb; i++ )
      if ( i != index ) {
	tmp[count] = str_dup( pGroup->spells[i], TRUE );
	count++;
      }
    pGroup->spells[pGroup->spellnb] = str_dup( s, TRUE );
    pGroup->spells = tmp;
    pGroup->spellnb--;
    send_to_charf(ch,"%s removed.\n\r", gn >= 0 ?"Group":"Ability");
  }
  return TRUE;
}
GROUPEDIT( group_edit_rating ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  int iClass = class_lookup( arg1 );
  int rate = atoi( arg2 );
  if ( arg1[0] == '\0'
       || iClass < 0
       || arg2[0] == '\0'
       || rate < 0 ) {
    send_to_charf(ch,"Syntax:  rating <class name> <rate>\n\r");
    send_to_charf(ch,"Type '? class' for a list of classes.\n\r");
    return FALSE;
  }

  group_type *pGroup;
  EDIT_GROUP(ch, pGroup);

  pGroup->rating[iClass] = rate;

  send_to_charf(ch,"Rate for class %s has been set.\n\r", class_table[iClass].name );
  pcclassnotsaved = TRUE;
  return TRUE;
}
GROUPEDIT( group_edit_sphere ) {
  int val = -1;
  if ( !str_cmp( argument, "yes" ) )
    val = 1;
  else if ( !str_cmp( argument, "no" ) )
    val = 0;
  if ( argument[0] == '\0'
       || val == -1 ) {
    send_to_charf(ch,"Syntax:  sphere  [yes/no]\n\r");
    return FALSE;
  }

  group_type *pGroup;
  EDIT_GROUP(ch, pGroup);
  int iGroup = -1;
  for ( int i = 0; i < MAX_GROUP; i++ )
    if ( &(group_table[i]) == pGroup ) {
      iGroup = i;
      break;
    }
  if ( iGroup == -1 ) {
    bug("!!Edited group not found in group_table!!");
    return FALSE;
  }

  if ( pGroup->isSphere ) {
    int iGod = -1;
    for ( int i = 0; i < MAX_GODS; i++ ) {
      god_data *god = &(gods_table[i]);
      if ( god->minor_sphere == iGroup ) {
	iGod = i;
	break;
      }
      bool found = FALSE;
      for ( int j = 0; j < god->nb_major_sphere; i++ )
	if ( god->major_sphere[j] == iGroup ) {
	  iGod = i;
	  found = TRUE;
	  break;
	}
      if ( found )
	break;
    }
    if ( iGod != -1 ) {
      send_to_charf(ch,"You can't remove sphere status for this group because it's\n\r"
		    "a minor/major sphere for god %s.\n\r", gods_table[iGod].name );
      return FALSE;
    }
  }
  
  pGroup->isSphere = val;
  if ( pGroup->isSphere )
    send_to_charf(ch,"This group is now a sphere.\n\r");
  else {
    send_to_charf(ch,"This group is not a sphere anymore.\n\r");
    for ( int i = 0; i < MAX_GODS; i++ )
      pGroup->god_rate[i] = -1;
  }
  spherenotsaved = TRUE;
  return TRUE;
}
GROUPEDIT( group_edit_god_rating ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  int iGod = god_lookup( arg1 );
  int rate = atoi( arg2 );
  if ( arg1[0] == '\0'
       || iGod < 0
       || arg2[0] == '\0'
       || rate < 0 ) {
    send_to_charf(ch,"Syntax:  godrating <class name> <rate>\n\r");
    send_to_charf(ch,"Type 'godinfo' for a list of gods.\n\r");
    return FALSE;
  }

  group_type *pGroup;
  EDIT_GROUP(ch, pGroup);

  if ( !pGroup->isSphere ) {
    send_to_charf(ch,"This group is not a sphere, you can't set god rating.\n\r");
    return FALSE;
  }

  pGroup->god_rate[iGod] = rate;

  send_to_charf(ch,"Rate for god %s has been set.\n\r", gods_table[iGod].name );
  return TRUE;
}
bool copy_group( group_type *from, group_type *to ) {
  to->name = str_dup( from->name, TRUE );
  to->isSphere = from->isSphere;

  to->spellnb = from->spellnb;
  if ( ( to->spells = (const char **) GC_MALLOC_ATOMIC( to->spellnb * sizeof(const char *) ) ) == NULL ) {
    bug("Can't allocate memory for group contains");
    return FALSE;
  }
  if ( ( to->rating = (int *)GC_MALLOC( MAX_CLASS * sizeof(int) ) ) == NULL ) {
    bug("Can't allocate memory for group rating");
    return FALSE;
  }
  if ( ( to->god_rate = (int *)GC_MALLOC( MAX_GODS * sizeof(int) ) ) == NULL ) {
    bug("Can't allocate memory for group god_rate");
    return FALSE;
  }

  for ( int i = 0; i < to->spellnb; i++ )
    to->spells[i] = str_dup(from->spells[i], TRUE);
  for ( int i = 0; i < MAX_CLASS; i++ )
    to->rating[i] = from->rating[i];
  for ( int i = 0; i < MAX_GODS; i++ )
    to->god_rate[i] = from->god_rate[i];
  spherenotsaved = TRUE;
  return TRUE;
}
GROUPEDIT( group_edit_create ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: create <group name>\n\r");
    return FALSE;
  }
  if ( group_lookup( arg2 ) >= 0 ) {
    send_to_charf(ch,"Group %s already exists.\n\r", arg2 );
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit group.\n\r", ch);
    return FALSE;
  }
  group_type *tmp;
  if ( ( tmp = (group_type*)GC_MALLOC(sizeof(group_type)*(MAX_GROUP+1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for group.\n\r");
    bug("Can't allocate memory for group");
    return FALSE;
  }
  // Copy group in new table
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    if ( !copy_group( &group_table[i], &tmp[i] ) ) {
      send_to_charf(ch,"Problem while creating group");
      return FALSE;
    }
  }

  // Add new group
  tmp[MAX_GROUP].name = str_dup( arg2, TRUE );
  tmp[MAX_GROUP].spellnb = 0;
  if ( ( tmp[MAX_GROUP].rating = (int *)GC_MALLOC( MAX_CLASS * sizeof(int) ) ) == NULL ) {
    bug("Can't allocate memory for group rating");
    return FALSE;
  }
  if ( ( tmp[MAX_GROUP].god_rate = (int *)GC_MALLOC( MAX_GODS * sizeof(int) ) ) == NULL ) {
    bug("Can't allocate memory for group god_rate");
    return FALSE;
  }
  for ( int i = 0; i < MAX_CLASS; i++ )
    tmp[MAX_GROUP].rating[i] = -1;
  for ( int i = 0; i < MAX_GODS; i++ )
    tmp[MAX_GROUP].god_rate[i] = 1;
  tmp[MAX_GROUP].isSphere = FALSE;

  // We must redirect pEdit for every people editing a group
  //  because group_table is modified
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    group_type *group = &(group_table[i]);
    if ( group->lockBit 
	 && group->editedBy != NULL ) // edited
      group->editedBy->pEdit = &(tmp[i]); // change the pointer
  }

  // Take care of pc_data->group_known (pc_data->points) 
  //  and gen_data->group_chosen(gen_data->points)
  for ( CHAR_DATA *vch = char_list; vch; vch = vch->next ) {
    if ( vch->pcdata != NULL ) {
      bool *tmp;
      if ( ( tmp = (bool *) GC_MALLOC_ATOMIC((MAX_GROUP+1) * sizeof(bool)) ) == NULL ){
	bug("Can't allocate memory for pcdata group_known");
	return TRUE;
      }
      for ( int i = 0; i < MAX_GROUP; i++ )
	  tmp[i] = vch->pcdata->group_known[i];
      tmp[MAX_GROUP] = FALSE;
      vch->pcdata->group_known = tmp;
    }
    if ( vch->gen_data != NULL ) {
      bool *tmp;
      if ( ( tmp = (bool *) GC_MALLOC_ATOMIC((MAX_GROUP+1) * sizeof(bool)) ) == NULL ){
	bug("Can't allocate memory for gendata group_chosen");
	return TRUE;
      }
      for ( int i = 0; i < MAX_GROUP; i++ )
	  tmp[i] = vch->gen_data->group_chosen[i];
      tmp[MAX_GROUP] = FALSE;
      vch->gen_data->group_chosen = tmp;
    }
  }

  group_table = tmp;
  MAX_GROUP++;

  ch->desc->pEdit = (void*)(&(group_table[MAX_GROUP-1])); // edit last group(new one)
  ch->desc->editor = ED_GROUP;

  send_to_charf(ch,"Group %s created.\n\r", arg2 );
  return TRUE;
}
GROUPEDIT( group_edit_delete ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: delete <group name>\n\r");
    return FALSE;
  }
  int iGroup = group_lookup( arg2 );
  if ( iGroup < 0 ) {
    send_to_charf(ch,"Group %s doesn't exist.\n\r",arg2);
    return FALSE;
  }
  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit group.\n\r", ch);
    return FALSE;
  }

  // The group can't be "rom basics"
  if ( iGroup == DEFAULT_GROUP ) {
    send_to_charf(ch,"You can't delete this group. It's the basic creation group.\n\r");
    return FALSE;
  }

  // The group can't be a class->base_group, class->default_group
  //  god->minor_sphere, god->major_sphere
  group_type *gr = &(group_table[iGroup]);
  for ( int i = 0; i < MAX_CLASS; i++ )
    if ( !str_cmp( class_table[i].base_group, gr->name )
	 || !str_cmp( class_table[i].default_group, gr->name ) ) {
      send_to_charf(ch,"You can't delete this group because it's a base/default\n\r"
		    " group for class %s.\n\r", class_table[i].name );
      return FALSE;
    }
  for ( int i = 0; i < MAX_GODS; i++ ) {
    god_data *god = &(gods_table[i]);
    bool found = FALSE;
    if ( god->minor_sphere == iGroup )
      found = TRUE;
    else
      for ( int j = 0; j < god->nb_major_sphere; j++ )
	if ( god->major_sphere[j] == iGroup ) {
	  found = TRUE;
	  break;
	}
    if ( found ) {
      send_to_charf(ch,"You can't delete this group because it's a minor/major\n\r"
		    " sphere for god %s.\n\r", gods_table[i].name );
      return FALSE;
    }
  }
 
  group_type *pGroup = &(group_table[iGroup]);
  if ( !CAN_EDIT( ch, pGroup ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name(pGroup));
    return FALSE;
  }
  group_type *tmp; // new group table
  if ( ( tmp = (group_type*)GC_MALLOC(sizeof(group_type)*(MAX_GROUP-1)) ) == NULL ) {
    send_to_charf(ch,"Can't allocate memory for group.\n\r");
    bug("Can't allocate memory for group");
    return FALSE;
  }
  int index = 0;
  for ( int i = 0; i < MAX_GROUP; i++ )
    if ( i != iGroup )
      if ( !copy_group( &group_table[i], &tmp[index++] ) ) {
	send_to_charf(ch,"Problem while creating group");
	return FALSE;
      }

  send_to_charf(ch,"Group %s deleted.\n\r", arg2 );
  if ( pGroup == ch->desc->pEdit ) // just deleted the group we were editing -> done
    edit_done(ch);

  // We must redirect pEdit for every people editing a group
  //  because group_table is modified
  index = 0;
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    group_type *group = &(group_table[i]);
    if ( group->lockBit 
	 && group->editedBy != NULL )  // edited
      group->editedBy->pEdit = &(tmp[index]); // change the pointer
    if ( i != iGroup )
      index++;
  }

  // Take care of pc_data->group_known (pc_data->points) 
    //  and gen_data->group_chosen(gen_data->points)
  for ( CHAR_DATA *vch = char_list; vch; vch = vch->next ) {
    // we don't give points back because the ability in the deleted group
    //  are not removed
    if ( vch->pcdata != NULL ) {
      bool *tmp;
      if ( ( tmp = (bool *) GC_MALLOC_ATOMIC((MAX_GROUP-1) * sizeof(bool)) ) == NULL ){
	bug("Can't allocate memory for pcdata group_known");
	return TRUE;
      }
      int count = 0;
      for ( int i = 0; i < MAX_GROUP; i++ )
	if ( i != iGroup )
	  tmp[count++] = vch->pcdata->group_known[i];
      vch->pcdata->group_known = tmp;
    }
    if ( vch->gen_data != NULL ) {
      bool *tmp;
      if ( ( tmp = (bool *) GC_MALLOC_ATOMIC((MAX_GROUP-1) * sizeof(bool)) ) == NULL ){
	bug("Can't allocate memory for gendata group_chosen");
	return TRUE;
      }
      int count = 0;
      if ( vch->gen_data->group_chosen[iGroup] ) // send a msg to char in creation
	send_to_charf(vch,"The group %s has been removed by an immortal.\n\r", pGroup->name );
      for ( int i = 0; i < MAX_GROUP; i++ )
	if ( i != iGroup )
	  tmp[count++] = vch->gen_data->group_chosen[i];
      vch->gen_data->group_chosen = tmp;
    }
  }

  group_table = tmp;
  MAX_GROUP--;

  return TRUE;
}

//****************************************************************************
//****************               FACTIONS             ************************
//****************************************************************************

void do_faction_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  if ( factionnotsaved )
    new_save_factions();
  send_to_char("Factions have been saved.\n\r", ch );
}

#define FACTIONEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type faction_edit_table[] = {
  {   "commands",	show_commands	},

  {   "race",           faction_edit_race },
  {   "friendliness",   faction_edit_friendliness },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void faction_edit( CHAR_DATA *ch, const char *argument ) {
  FACTION_DATA *pFaction = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pFaction ) )
    return;
  EDIT_FACTION( ch, pFaction );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit faction.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    faction_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; faction_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, faction_edit_table[cmd].name ) ) {
      if ( (*faction_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	factionnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing faction */
void do_faction_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit factions.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a faction name.\n\r"
		 "Syntax: factionedit <faction name>\n\r", ch );
    return;
  }

  FACTION_DATA *pFaction;
  if ( !str_cmp( arg, "create" ) ) {
    if ( faction_edit_create( ch, argument ) ) {
      factionnotsaved = TRUE;
      EDIT_FACTION( ch, pFaction );
    }
    else
      return;
  }
  else if ( !str_cmp( arg, "delete" ) ) {
    faction_edit_delete( ch, argument );
    return;
  }
  else {
    int iFaction = get_faction_id(arg);
    if ( iFaction < 0 ) {
      send_to_charf(ch, "Faction (%s) doesn't exist.\n\r", arg );
      return;
    }
    pFaction = &(faction_table[iFaction]);
  }

  if ( !OLC_EDIT( ch, pFaction ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pFaction;
  ch->desc->editor = ED_FACTION;
  return;
}

static void faction_show( CHAR_DATA *ch, const FACTION_DATA *pFaction ) {
  send_to_charf(ch,"Name:                 [%s]\n\r", pFaction->name );
  show_races( ch, pFaction->races, pFaction->nb_races, "Race" );
  for ( int i = 0; i < factions_count; i++ )
    send_to_charf(ch,"Faction: [%20s]  Friendliness: [%5d]\n\r",
		  faction_table[i].name, pFaction->friendliness[i] );
}
FACTIONEDIT( faction_edit_show ) {
  FACTION_DATA *pFaction;
  EDIT_FACTION( ch, pFaction );

  faction_show( ch, pFaction );
  return FALSE;
}
FACTIONEDIT( faction_edit_race ) {
  FACTION_DATA *pFaction;
  EDIT_FACTION( ch, pFaction );

  // Check if this race is already member of another faction
  int iRace = race_lookup( argument,TRUE );
  if ( argument[0] != '\0'
       && iRace >= 0 ) {
    int index = -1;
    for ( int i = 0; i < pFaction->nb_races; i++ )
      if ( pFaction->races[i] == iRace ) {
	index = i;
	break;
      }
    if ( index == -1 ) {
      for ( int i = 0; i < factions_count; i++ ) {
	FACTION_DATA *faction = &(faction_table[i]);
	if ( faction != pFaction )
	  for ( int j = 0; j < faction->nb_races; j++ )
	    if ( faction->races[j] == iRace ) {
	      send_to_charf(ch,"Race [%s] is already in another faction [%s]\n\r",
			    race_table[iRace].name, faction->name );
	      return FALSE;
	    }
      }
    }
  }
  return edit_race( ch, pFaction->races, pFaction->nb_races, "race", argument );
}
FACTIONEDIT( faction_edit_friendliness ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  int id = get_faction_id( arg1 );
  int val = atoi( arg2 );
  if ( arg1[0] == '\0' || arg2[0] == '\0'
       || !is_number( arg2 )
       || val < MIN_FACTION_VALUE || val > MAX_FACTION_VALUE
       || id < 0 ) {
    send_to_charf(ch,"Syntax:  friendliness  <faction> <value (-1000..1000)>\n\r"
		  "Type '<?>' for a list of available faction.\n\r");
    return FALSE;
  }

  FACTION_DATA *pFaction;
  EDIT_FACTION( ch, pFaction );

  pFaction->friendliness[id] = val;
  send_to_charf(ch,"Friendliness set.\n\r");
  return TRUE;
}
bool copy_faction( FACTION_DATA *from, FACTION_DATA *to ) {
  to->name = str_dup( from->name, TRUE );
  to->nb_races = from->nb_races;
  if ( ( to->races = (int*)GC_MALLOC(sizeof(int)*to->nb_races) ) == NULL ) {
    bug("Can't allocate memory for faction");
    return FALSE;
  }
  for ( int i = 0; i < from->nb_races; i++ )
    to->races[i] = from->races[i];
  for ( int i = 0; i < MAX_FACTION; i++ )
    to->friendliness[i] = from->friendliness[i];
  return TRUE;
}
FACTIONEDIT( faction_edit_create ) {
  send_to_charf(ch,"NOT YET IMPLEMENTED.\n\r");
  return FALSE;
}
FACTIONEDIT( faction_edit_delete ) {
  send_to_charf(ch,"NOT YET IMPLEMENTED.\n\r");
  return FALSE;
}

//****************************************************************************
//****************          MAGIC SCHOOLS             ************************
//****************************************************************************

void do_magic_school_save( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  if ( schoolnotsaved )
    save_schools();
  send_to_char("Magic schools have been saved.\n\r", ch );
}

#define MAGICSCHOOLEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type magic_school_edit_table[] = {
  {   "commands",	show_commands	},

  {   "name",           magic_school_edit_name },
  {   "list",           magic_school_edit_list },
  {   "create",         magic_school_edit_create },
  {   "delete",         magic_school_edit_delete },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void magic_school_edit( CHAR_DATA *ch, const char *argument ) {
  magic_school_type *pSchool = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pSchool ) )
    return;
  EDIT_MAGIC_SCHOOL( ch, pSchool );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit magic schools.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    magic_school_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; magic_school_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, magic_school_edit_table[cmd].name ) ) {
      if ( (*magic_school_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	schoolnotsaved = TRUE;
	return;
      }
      else
	return;
    }

  // Default to Standard Interpreter.
  interpret( ch, arg );
  return;
}

// Entry point for editing magic school
void do_magic_school_edit( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_DATA( ch ) ) {
    send_to_char("Insufficient security to edit magic schools.\n\r", ch );
    return;
  }

  argument = one_argument(argument,arg);
  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a magic school name.\n\r"
		 "Syntax: magicschooledit <magic school name>\n\r", ch );
    return;
  }
  int schoolId = magic_school_lookup(arg);
  if ( schoolId < 0 ) {
    send_to_charf(ch, "Magic School (%s) doesn't exist.\n\r", arg );
    return;
  }
  magic_school_type *pSchool;
  pSchool = &(magic_school_table[schoolId]);

  if ( !OLC_EDIT( ch, pSchool ) )
    return;

  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pSchool;
  ch->desc->editor = ED_MAGIC_SCHOOL;
  return;
}
static void magic_school_show( CHAR_DATA *ch, const magic_school_type *pSchool ) {
  send_to_charf(ch,"Name:                 [%s]\n\r", pSchool->name );
  send_to_charf(ch,"List:                 \n\r" );
  if ( pSchool->nb_spells == 0 )
    send_to_charf(ch,"No abilities");
  else {
    int col = 0;
    for ( int i = 0; i < pSchool->nb_spells; i++ ) {
      int sn = pSchool->spells_list[i];
      if ( sn > 0 ) {
	send_to_charf(ch,"%20.19s", ability_table[sn].name );
	if ( ++col % 3 == 0 )
	  send_to_charf(ch, "\n\r" );
      }
    }
    if ( col % 3 != 0 )
      send_to_charf(ch, "\n\r" );
  }
}
MAGICSCHOOLEDIT( magic_school_edit_show ) {
  magic_school_type *pSchool;
  EDIT_MAGIC_SCHOOL( ch, pSchool );

  magic_school_show( ch, pSchool );
  return FALSE;
}
MAGICSCHOOLEDIT( magic_school_edit_name ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}
MAGICSCHOOLEDIT( magic_school_edit_list ) {
  int sn = ability_lookup( argument );
  if ( argument[0] == '\0'
       || sn < 0 ) {
    send_to_charf(ch,"Syntax:  lists  <ability name>\n\r"
		  "Type '? ability <skill/spell/power/song> <target>' for a list of abilities.\n\r");
    return FALSE;
  }
  magic_school_type *pSchool;
  EDIT_MAGIC_SCHOOL( ch, pSchool );
  int schoolId = magic_school_lookup( pSchool->name );
  int pos = ability_in_school( schoolId, sn );
  if ( pos < 0 ) { // Add from list
    if ( ability_table[sn].school >= 0
	 && ability_table[sn].school != schoolId ) { // remove from other school
      send_to_charf(ch,"[%s] removed from school [%s].\n\r",
		    ability_table[sn].name,
		    magic_school_table[ability_table[sn].school].name );
      remove_from_school( sn );
    }
    add_to_school( schoolId, sn );
    send_to_charf(ch,"[%s] added in school [%s].\n\r", 
		  ability_table[sn].name,
		  pSchool->name );
  }
  else { // Remove from list
    remove_from_school( sn );
    send_to_charf(ch,"[%s] removed from school [%s].\n\r",
		  ability_table[sn].name,
		  pSchool->name );
  }
  return TRUE;
}
MAGICSCHOOLEDIT( magic_school_edit_create ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}
MAGICSCHOOLEDIT( magic_school_edit_delete ) {
  send_to_charf(ch,"Not Yet Implemented.\n\r");
  return FALSE;
}
