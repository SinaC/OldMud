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
#include <string.h>
#include <time.h>

#if !defined(__CYGWIN32__)
#include <malloc.h>
#endif

#include "merc.h"
#include "recycle.h"
#include "lookup.h"
#include "tables.h"

#include "classes.h"

// Added by SinaC 2001
#include "interp.h"
#include "clan.h"
#include "handler.h"
#include "save.h"
#include "db.h"
//#include "olc.h"
#include "gsn.h"
#include "olc_value.h"
#include "const.h"
//#include "scrhash.h"
#include "faction.h"
#include "ability.h"
#include "group.h"
#include "dbdata.h"
#include "names.h"
#include "language.h"
#include "clan.h"
#include "config.h"
#include "act_comm.h"
#include "fight.h"
#include "bit.h"
#include "olc.h"
#include "comm.h" // fState
#include "act_move.h"
#include "mem.h"
#include "scrhash.h"
#include "dbscript.h"
#include "utils.h"


// structure used to reform group and restart fight after a copyover
typedef struct ptr_list PTR_LIST;
struct ptr_list {
  CHAR_DATA *ch; // Ch's ptr address after copyover
  int previous_ptr_address; // Ch's ptr address before copyover
  int master_ptr_address; // Ch's master ptr address before copyover
  int leader_ptr_address; // Ch's leader ptr address before copyover
  int fighting_ptr_address; // Ch's fighting ptr address before copyover
  // only for NPCs
  int hunting_ptr_address; // Ch's hunting ptr address before copyover
  // only for PCs
  int betted_on_ptr_address; // Ch's betted on ptr address before copyover
  int challenged_ptr_address; // Ch's challenged ptr address before copyover

  PTR_LIST *next;
};
static PTR_LIST *ptr_list_first = NULL;


#if !defined(macintosh)
extern  int     _filbuf         args( (FILE *) );
#endif


void create_new_char( DESCRIPTOR_DATA *d, const char *name ) {
  CHAR_DATA *ch = new_char();
  ch->pcdata = new_pcdata();

  d->character			        = ch;
  ch->desc				= d;
  ch->name				= str_dup( name );
  ch->id				= get_pc_id();
  //ch->bstat(race)			= race_lookup("human");
  ch->bstat(race)			= DEFAULT_PC_RACE;
  ch->act				= PLR_NOSUMMON | PLR_AUTOHAGGLE;
  ch->comm				= COMM_COMBINE | COMM_PROMPT;
  ch->prompt                            = str_dup("{c<%h/%HHp %m/%MMn %p/%PPsp %v/%VMv %XNxt %b>{x");
  ch->pcdata->confirm_delete		= FALSE;

  // every characters starts at default board from login.. this board
  //   should be read_level == 0 !
  ch->pcdata->board		= &boards[DEFAULT_BOARD];
  ch->pcdata->language = 0;
  ch->pcdata->goto_default = 1;
  ch->pcdata->pwd			= str_dup( "" );
  ch->pcdata->bamfin			= str_dup( "" );
  ch->pcdata->bamfout			= str_dup( "" );
  ch->pcdata->title			= str_dup( "" );
  for (int stat =0; stat < MAX_STATS; stat++)
    ch->bstat(stat0+stat)		= 13;
  ch->pcdata->condition[COND_THIRST]	= 48;
  ch->pcdata->condition[COND_FULL]	= 48;
  ch->pcdata->condition[COND_HUNGER]	= 48;
  ch->pcdata->security		= 0;
  ch->pcdata->trivia                  = 0;
  //ch->pcdata->hometown		= hometown_lookup("midgaard");
  ch->pcdata->hometown		      = DEFAULT_HOMETOWN;
  ch->pcdata->name_accepted = FALSE;
  //  ch->pcdata->questgiver              = NULL;
  //  ch->pcdata->questpoints             = 0;
  //  ch->pcdata->nextquest               = 0;
  //  ch->pcdata->countdown               = 0;
  //  ch->pcdata->questobj                = 0;
  //  ch->pcdata->questobjloc             = 0;
  //  ch->pcdata->questmob                = 0;
  ch->pcdata->stance = str_dup("");
  ch->pcdata->immtitle = str_dup("");
  for (int stat = 0; stat < MAX_BEACONS; stat++ )
    ch->pcdata->beacons[stat] = 0;
  ch->pcdata->god = 0;
  ch->clan_status = CLAN_JUNIOR;
  ch->petition = 0;
}

//void questfix( CHAR_DATA *ch ) {
//  log_stringf("Invalid quest data ==> fixing");
//  ch->pcdata->questgiver = NULL;
//  ch->pcdata->countdown = 0;
//  ch->pcdata->questmob = 0;
//  ch->pcdata->questobj = 0;
//  ch->pcdata->questobjloc = 0;
//  REMOVE_BIT( ch->act, PLR_QUESTOR );
//}

void fix_char( CHAR_DATA *ch ) {
  if (ch->version < 6) {
    for (int i = 0; i<4; i++)
      ch->bstat(ac0+i) = 100;
    
    ch->bstat(hitroll)		= 0;
    ch->bstat(damroll)		= 0;
    ch->bstat(saving_throw)	= 0;
  }
  
  if (ch->version < 7){
    // init race
    // Modified by SinaC 2000
    //if (ch->bstat(race) == 0)
    if (ch->bstat(race) < 0)
      //ch->bstat(race) = race_lookup("human");
      ch->bstat(race) = DEFAULT_PC_RACE;
    int raceId = ch->bstat(race); // SinaC 2003
    if ( !race_table[raceId].pc_race ) { //   if base race is not PC race
      raceId = DEFAULT_PC_RACE;             //     get default race: human
      bug("do_remort: invalid race for [%s] sn:[%d]",
	  NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
    }
    //ch->bstat(size) = pc_race_table[ch->bstat(race)].size;
    //ch->bstat(size) = pc_race_table[raceId].size;
    ch->bstat(size) = race_table[raceId].size;
    ch->bstat(affected_by) = ch->bstat(affected_by) | race_table[ch->bstat(race)].aff;
    // Added by SinaC 2001
    ch->bstat(affected2_by) = ch->bstat(affected2_by) | race_table[ch->bstat(race)].aff2;
    ch->bstat(imm_flags)	= ch->bstat(imm_flags) | race_table[ch->bstat(race)].imm;
    ch->bstat(res_flags)	= ch->bstat(res_flags) | race_table[ch->bstat(race)].res;
    ch->bstat(vuln_flags)	= ch->bstat(vuln_flags) | race_table[ch->bstat(race)].vuln;
    ch->bstat(form)	= race_table[ch->bstat(race)].form;
    ch->bstat(parts)	= race_table[ch->bstat(race)].parts;
    //if (IS_SET(PART_CLAWS,ch->bstat(parts)))
    if ( IS_SET(ch->bstat(parts),PART_CLAWS))
      ch->bstat(dam_type) = 5; // claws,  By Oxtal
    else
      ch->bstat(dam_type) = 17; //punch
  }
  // RT initialize skills
  //if ( ch->version < 2) { //   need to add the new skills
  //  group_add(ch,"rom basics",FALSE);
  //  group_add(ch,class_table[ch->class].base_group,FALSE);
  //  group_add(ch,class_table[ch->class].default_group,TRUE);
  //  ch->pcdata->learned[gsn_recall] = 50;
  //}
  // fix levels
  if (ch->version < 3 && (ch->level > 35 || ch->trust > 35)){
    switch (ch->level){
    case(40) : ch->level = 60;	break;  // imp -> imp
    case(39) : ch->level = 58; 	break;	// god -> supreme
    case(38) : ch->level = 56;  break;	// deity -> god
    case(37) : ch->level = 53;  break;	// angel -> demigod
    }
    switch (ch->trust){
    case(40) : ch->trust = 60;  break;	// imp -> imp
    case(39) : ch->trust = 58;  break;	// god -> supreme
    case(38) : ch->trust = 56;  break;	// deity -> god
    case(37) : ch->trust = 53;  break;	// angel -> demigod
    case(36) : ch->trust = 51;  break;	// hero -> hero
    }
  }
  // realm gold */
  if (ch->version < 4)
    ch->gold   /= 100;
  // Added by SinaC 2001 for quest data
  // fix quest data
  //  if ( ch->pcdata->questmob != 0
  //       || ch->pcdata->questobj != 0 )
  //    if( ch->pcdata->questgiver == NULL
  //	|| ch->pcdata->countdown == 0 ) 
  //      questfix(ch);
  //  if( ch->pcdata->questgiver != NULL
  //      || ch->pcdata->countdown != 0 )
  //    if ( ch->pcdata->questmob == 0
  //	 && ch->pcdata->questobj == 0 )
  //      questfix(ch);
  
  // If quest was an item, reload the item in the right room.
  //  if ( ch->pcdata->questobj != 0
  //       && ch->pcdata->questobjloc != 0 ) {
  //    ROOM_INDEX_DATA *room = get_room_index(ch->pcdata->questobjloc);
  //    // room still exists ?
  //    if ( room == NULL )
  //      questfix(ch);
  //    else {
  //      OBJ_INDEX_DATA *questobj = get_obj_index(ch->pcdata->questobj);
  //      // questobj still exists ?
  //      if ( questobj == NULL )
  //	questfix(ch);
  //      else {
  //	bool already = FALSE;
  //	for (OBJ_DATA *objroom = room->contents; objroom != NULL; objroom = objroom->next_content )
  //	  if ( objroom->pIndexData == questobj ) {
  //	    already = TRUE;
  //	    break;
//	  }
//	// not already in the room
//	if ( !already ) {
//	  // not in player's inventory/equipement
//	  for (OBJ_DATA* objch = ch->carrying; objch != NULL; objch = objch->next_content )
//	    if ( objch->pIndexData == questobj ) {
//	      already = TRUE;
//	      break;
//	    }
//	  if ( !already ) {
//	    OBJ_DATA *questitem = create_object( questobj, ch->level );
//	    questitem->timer=100;
//	    obj_to_room(questitem, room);
//	  }
//	}
//      }
//    }
//  }
}


int rename(const char *oldfname, const char *newfname);

/*
 * Array of containers read for proper re-nesting of objects.
 */
#define MAX_NEST	100
static	OBJ_DATA *	rgObjNest	[MAX_NEST];

/*
 * Local functions.
 */
void	fread_char	args( ( CHAR_DATA *ch,  FILE *fp ) );
void    fread_pet	args( ( CHAR_DATA *ch,  FILE *fp ) );
void	fread_obj	args( ( CHAR_DATA *ch,  FILE *fp ) );



/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA *d, char *name ) {
  char strsave[MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *ch;
  FILE *fp;
  bool found;
  int stat;

  create_new_char( d, name ); // Sinac 2003
  ch = d->character;

  found = FALSE;
  fclose( fpReserve );

#if defined(unix)
  /* decompress if .gz file exists */
  sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize(name),".gz");
  if ( ( fp = fopen( strsave, "r" ) ) != NULL ){
    fclose(fp);
    sprintf(buf,"gzip -dfq %s",strsave);
    system(buf);
  }
#endif

  sprintf( strsave, "%s%s", PLAYER_DIR, capitalize( name ) );
  if ( ( fp = fopen( strsave, "r" ) ) != NULL ){
    int iNest;

    for ( iNest = 0; iNest < MAX_NEST; iNest++ )
      rgObjNest[iNest] = NULL;

    found = TRUE;
    for ( ; ; ){
      char letter;
      char *word;

      letter = fread_letter( fp );
      if ( letter == '*' ){
	fread_to_eol( fp );
	continue;
      }

      if ( letter != '#' ){
	bug( "Load_char_obj: # not found." );
	break;
      }

      word = fread_word( fp );
      if      ( !str_cmp( word, "PLAYER" ) ) fread_char ( ch, fp );
      else if ( !str_cmp( word, "OBJECT" ) ) fread_obj  ( ch, fp );
      else if ( !str_cmp( word, "O"      ) ) fread_obj  ( ch, fp );
      else if ( !str_cmp( word, "PET"    ) ) fread_pet  ( ch, fp );
      // Added by SinaC 2003
      else if ( !str_cmp( word, "PETOBJ"    ) ) fread_obj  ( ch->pet, fp );
      else if ( !str_cmp( word, "END"    ) ) break;
      else {
	bug( "Load_char_obj: bad section.");
	break;
      }
    }
    fclose( fp );
  }

  fpReserve = fopen( NULL_FILE, "r" );

  if (found) {
    fix_char(ch); // SinaC 2003
    //recompute(ch); // Added along copyover -- Oxtal  NOT NEEDED: done in char_to_room in nanny:CON_MOTD
  }
  
  return found;
}

/*
 * Read in a char.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/* provided to free strings */
#if defined(KEYS)
#undef KEYS
#endif

#define KEYS( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


// Added by SinaC 2001 to read extra fields
void fread_ex_fields( ENTITY_DATA *e, FILE *fp ) {
  char name[MAX_STRING_LENGTH];
  int typ;
  int lifetime;
  int valint;
  //char valstr[MAX_STRING_LENGTH];
  const char *valstr;
  Value v;

  strcpy( name, fread_word(fp) );
  typ = fread_number(fp);
  lifetime = fread_number(fp);
  switch(typ) {
  case SVT_INT:
    valint = fread_number(fp);
    if ( DATA_VERBOSE > 2 ) {
      log_stringf("INTEGER ex_fields  name: %s  lifetime: %d  value: %d",
		  name, lifetime, valint );
    }
    v = add_extra_field( e, name );
    v.setValue(valint);
    break;
  case SVT_STR:
    valstr = fread_string(fp);
    if ( DATA_VERBOSE > 2 ) {
      log_stringf("STRING  ex_fields  name: %s  lifetime: %d  value: %s",
		  name, lifetime, valstr );
    }
    v = add_extra_field( e, name );
    v.setValue(valstr);
    break;
  default:
    /*
    strcpy( valstr, fread_word(fp) );
    log_stringf("??? ex_fields  name: %s  lifetime: %d  value: %s",
		name, lifetime, valstr );
    v = add_extra_field( e, name );
    v.setValue(valstr);
    */
    break;
  }

}

void fread_char( CHAR_DATA *ch, FILE *fp ) {
  char buf[MAX_STRING_LENGTH];
  const char *word;
  bool fMatch;
  int count = 0;
  int lastlogoff = current_time;
  int percent;

  log_stringf("Loading %s.",ch->name);

  for ( ; ; ) {
    word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

    switch ( UPPER(word[0]) ){
    case '*':
      fMatch = TRUE;
      fread_to_eol( fp );
      break;

    case 'A':
      KEY( "Act",		ch->act,		fread_flag( fp ) );
      KEY( "AffectedBy",	ch->bstat(affected_by),	fread_flag( fp ) );
      KEY( "AfBy",	ch->bstat(affected_by),	fread_flag( fp ) );
      // Added by SinaC 2001
      KEY( "AffectedBy2",	ch->bstat(affected2_by),	fread_flag( fp ) );
      KEY( "AfBy2",	ch->bstat(affected2_by),	fread_flag( fp ) );

      // Added by SinaC 2001
      if ( !str_cmp( word, "Address" ) ) {
	strcpy( buf, fread_word(fp) );
	fMatch = TRUE;
	break;
      }

      // Modified by SinaC 2001, etho/alignment are attributes now
      // Modified by SinaC 2000 for alignment/etho
      //KEY( "Alignment",	ch->align.alignment,		fread_number( fp ) );
      //KEY( "Alig",	ch->align.alignment,		fread_number( fp ) );
      if (!str_cmp( word, "Alig" ) 
	  || !str_cmp( word, "Alignment" ) ) {
	ch->bstat(alignment) = fread_number(fp);
	fMatch = TRUE;
	break;
      }


// Removed by SinaC 2001, use extra fields now
//      KEY( "AcctGold",        ch->pcdata->acctgold,       fread_number( fp ) );
//      KEY( "AcctSilver",        ch->pcdata->acctsilver,       fread_number( fp ) );

      if (!str_cmp( word, "Alia")) {
	if (count >= MAX_ALIAS){
	  fread_to_eol(fp);
	  fMatch = TRUE;
	  break;
	}

	ch->pcdata->alias[count] 	= str_dup(fread_word(fp));
	ch->pcdata->alias_sub[count]	= str_dup(fread_word(fp));
	count++;
	fMatch = TRUE;
	break;
      }

      if (!str_cmp( word, "Alias")){
	if (count >= MAX_ALIAS){
	  fread_to_eol(fp);
	  fMatch = TRUE;
	  break;
	}

	ch->pcdata->alias[count]        = str_dup(fread_word(fp));
	ch->pcdata->alias_sub[count]    = fread_string(fp);
	count++;
	fMatch = TRUE;
	break;
      }

      if (!str_cmp( word, "AC") || !str_cmp(word,"Armor")){
	fread_to_eol(fp);
	fMatch = TRUE;
	break;
      }

      if (!str_cmp(word,"ACs")){
	int i;

	for (i = 0; i < 4; i++)
	  ch->bstat(ac0+i) = fread_number(fp);
	fMatch = TRUE;
	break;
      }

      if (!str_cmp(word, "Affects")){
	AFFECT_DATA *paf;
	int sn;
 
	paf = new_affect();

	sn = ability_lookup(fread_word(fp));
	int type = sn;
	if (sn < 0) {
	  bug("Fread_char: unknown skill.");
	  //paf->type = 0;
	  type = 0;
	}
	else
	  //paf->type = sn;
	  type = sn;

	int loc;
	int where = fread_number(fp);
	//paf->where      = fread_number(fp);
	// Added by SinaC 2001
	if ( ch->version > 9 ) {
	  char buf2[MAX_STRING_LENGTH];
	  strcpy( buf2, fread_word(fp));
	  //paf->location = flag_value( attr_flags, buf2);
	  loc = flag_value( attr_flags, buf2);
	}
	else {
	  //paf->location   = fread_number( fp );
	  loc  = fread_number( fp );

	  log_stringf("before=== affect: %s  location: %d",
		      ability_table[sn].name,
		      //paf->location);
		      loc);

	  // Added by SinaC 2001
	  //if ( paf->location == 30 ) // 30 is the old ATTR_NUMBER
	  //  paf->location = ATTR_NUMBER;
	  //else if ( paf->location == 31 )
	  //  paf->location = ATTR_NA;
	  if ( loc == 30 ) // 30 is the old ATTR_NUMBER
	    loc = ATTR_NUMBER;
	  else if ( loc == 31 )
	    loc = ATTR_NA;

	  log_stringf("after=== affect: %s  location: %d",
		      ability_table[sn].name,
		      //paf->location);
		      loc);

	}

	//paf->op         = fread_number( fp );
	//paf->modifier   = fread_number( fp );
	//paf->duration   = fread_number( fp );
	//paf->level      = fread_number( fp );
	int op         = fread_number( fp );
	int modifier   = fread_number( fp );
	int duration   = fread_number( fp );
	int level      = fread_number( fp );
	int casting;
	// Modified by SinaC 2000
	if ( ch->version > 7 )
	  //paf->casting_level = fread_number( fp );
	  casting = fread_number( fp );
	else
	  //paf->casting_level = 0;
	  casting = 0;

	createaff(*paf,duration,level,type,casting,type>0?AFFECT_ABILITY:AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT);
	addaff2(*paf,where,loc,op,modifier);

	paf->next       = ch->affected;
	ch->affected    = paf;
	fMatch = TRUE;
	break;
      }

      if ( !str_cmp( word, "AttrMod"  ) || !str_cmp(word,"AMod")) {     /*
									  int stat;
									  for (stat = 0; stat < MAX_STATS; stat ++)
									  ch->mod_stat[stat] = fread_number(fp);
									  fMatch = TRUE;*/
	break;
      }

      if ( !str_cmp( word, "AttrPerm" ) || !str_cmp(word,"Attr")) {
	int stat;

	for (stat = 0; stat < MAX_STATS; stat++)
	  ch->bstat(stat0+stat) = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      break;

    case 'B':
      KEY( "Bamfin",	ch->pcdata->bamfin,	fread_string( fp ) );
      KEY( "Bamfout",	ch->pcdata->bamfout,	fread_string( fp ) );
      KEY( "Bin",		ch->pcdata->bamfin,	fread_string( fp ) );
      KEY( "Bout",	ch->pcdata->bamfout,	fread_string( fp ) );
      // Added by SinaC 2001 for branding
      //KEY( "Branded", ch->pcdata->branded,      fread_number( fp ) );   removed by SinaC 2003
	    
      // Added by SinaC 2000 for beacon spell
      if (!str_cmp(word, "Beacons")) {
	for ( int i = 0; i < MAX_BEACONS; i++ )
	  ch->pcdata->beacons[i] = fread_number(fp);
	fMatch = TRUE;
      }
      // end of beacon

      /* Read in board status */	    
      if (!str_cmp(word, "Boards" )) {
	int i,num = fread_number (fp); /* number of boards saved */
	char *boardname;
 			
	/* for each of the board saved */
	for (; num ; num-- ){
	  boardname = fread_word (fp);
				/* find board number */
	  i = board_lookup (boardname); 
	  /* Does board still exist ? */
	  if (i == BOARD_NOTFOUND){
	    log_stringf("fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
	    fread_number (fp); /* read last_note and skip info */
	  }
	  else /* Save it */
	    ch->pcdata->last_note[i] = fread_number (fp);
	}	 /* for */
 			
	fMatch = TRUE;
      } /* Boards */
      break;

    case 'C':
      KEY( "Class",	ch->bstat(classes),	1<<	fread_number( fp ) );
      KEY( "Classes",	ch->bstat(classes),	fread_number( fp ) );
      KEY( "Cla",		ch->bstat(classes),	1<<	fread_number( fp ) );
      KEY( "Clan",	ch->clan,	clan_lookup(fread_string(fp)));
      KEY( "ClanStatus", ch->clan_status, fread_number(fp) );

      KEY( "Cles",	ch->bstat(classes),	fread_number( fp ) );

      if ( !str_cmp( word, "Condition" ) || !str_cmp(word,"Cond")){
	ch->pcdata->condition[0] = fread_number( fp );
	ch->pcdata->condition[1] = fread_number( fp );
	ch->pcdata->condition[2] = fread_number( fp );
	fMatch = TRUE;
	break;
      }
      if (!str_cmp(word,"Cnd")){
	ch->pcdata->condition[0] = fread_number( fp );
	ch->pcdata->condition[1] = fread_number( fp );
	ch->pcdata->condition[2] = fread_number( fp );
	ch->pcdata->condition[3] = fread_number( fp );
	fMatch = TRUE;
	break;
      }
      KEY("Comm",		ch->comm,		fread_flag( fp ) );

      break;

    case 'D':
      KEY( "Damroll",	ch->bstat(damroll),		fread_number( fp ) );
      KEY( "Dam",		ch->bstat(damroll),		fread_number( fp ) );
      KEY( "DamType",	ch->bstat(dam_type),		attack_lookup( fread_word(fp) ));
      KEY( "Description",	ch->description,	fread_string( fp ) );
      KEY( "Desc",	ch->description,	fread_string( fp ) );
      
      // Added by SinaC 2001 for player disabled commands
      if ( !str_cmp( word, "DisabledCmd" ) ) {
	int i;
	DISABLED_CMD_DATA *p;
	char *temp;
	char cmd[512];
	char disabler[512];

	temp = fread_word( fp ); strcpy(cmd, temp);
	temp = fread_word( fp ); strcpy(disabler, temp);

	/* Search for the command */
	for (i = 0; cmd_table[i].name[0] != '\0'; i++)
	  if (!str_cmp(cmd_table[i].name, cmd))
	    break;
	
	/* Found? */				
	if (cmd_table[i].name[0] == '\0') {
	  bug("Invalid player disabled command: skipping ");
	  fMatch = FALSE;
	}
	else {
	  /* Disable the command */
	  if ( ( p = (DISABLED_CMD_DATA *) GC_MALLOC( sizeof( DISABLED_CMD_DATA ) ) ) == NULL ) {
	    bug("Memory allocation error in disable_cmd_data\n\r");
	    exit(-1);
	  }
	  p->cmd = &cmd_table[i];
	  p->disabled_by = str_dup(disabler); /* save name of disabler */
	  p->next = ch->pcdata->disabled_cmd;
	  ch->pcdata->disabled_cmd = p; /* add before the current first element */
	  fMatch = TRUE;
	}
      }
      break;

    case 'E':
      // Added by SinaC 2001
      if ( !str_cmp( word, "ExField" ) ) {
	fMatch = TRUE;
	fread_ex_fields( ch, fp );
      }
      if ( !str_cmp( word, "End" ) ){
	/* adjust hp mana move up  -- here for speed's sake */
	percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);

	percent = UMIN(percent,100);

	// Added by SinaC 2003
	if ( ch->position != POS_STANDING
	     && ch->position != POS_SITTING
	     && ch->position != POS_RESTING
	     && ch->position != POS_SLEEPING )
	  ch->position = POS_STANDING;

	// Added by SinaC 2001 to correct possible error
	if ( ch->pcdata->god < 0 || ch->pcdata->god >= MAX_GODS ){
	  log_string("Invalid god ==> fixing");
	  ch->pcdata->god = DEFAULT_GOD;
	}
	if ( ch->pcdata->hometown < 0 && DEFAULT_HOMETOWN >= 0  ) {
	  log_string("Invalid hometown ==> fixing");
	  ch->pcdata->hometown = 0;
	}

	if ( ch->bstat(race) < 0 ) {
	  log_stringf("Invalid race ==> fixing");
	  ch->bstat(race) = DEFAULT_PC_RACE;
	}

	if ( ch->bstat(classes) <= 0 ) {
	  log_stringf("Invalid classes ==> fixing");
	  ch->bstat(race) = 1;
	}

	if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
	    &&  !IS_AFFECTED(ch,AFF_PLAGUE)){
	  ch->hit     += (ch->bstat(max_hit) - ch->hit) * percent / 100;
	  ch->mana    += (ch->bstat(max_mana) - ch->mana) * percent / 100;
	  ch->move    += (ch->bstat(max_move) - ch->move)* percent / 100;
	  // Added by SinaC 2001 for mental user
	  ch->psp     += (ch->bstat(max_psp) - ch->psp) * percent / 100;
	}

	return;
      }
      KEY( "Exp",		ch->exp,		fread_number( fp ) );
      // Modified by SinaC 2001
      // Added by SinaC 2000 for etho
      //KEY( "Etho",              ch->align.etho,         fread_number( fp ) );
      if (!str_cmp( word, "Etho" ) ) {
	ch->bstat(etho) = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      break;
    case 'F':

      KEY( "Form",	ch->bstat(form),	fread_flag( fp ) );
      break;

    case 'G':
      KEY( "Gold",	ch->gold,		fread_number( fp ) );
      // Added by SinaC 2001 for god
      KEY( "God",       ch->pcdata->god,        god_lookup(fread_word(fp)));
      KEY( "Gset",      ch->pcdata->goto_default, fread_number(fp));

      if ( !str_cmp( word, "Group" )  || !str_cmp(word,"Gr")){
	int gn;
	char *temp;

	temp = fread_word( fp ) ;
	gn = group_lookup(temp);
	/* gn    = group_lookup( fread_word( fp ) ); */
	if ( gn < 0 ){
	  fprintf(stderr,"%s",temp);
	  bug( "Fread_char: unknown group. " );
	}
	else
	  gn_add(ch,gn);
	fMatch = TRUE;
      }
      break;

    case 'H':
      KEY( "Hitroll",	ch->bstat(hitroll),		fread_number( fp ) );
      KEY( "Hit",		ch->bstat(hitroll),		fread_number( fp ) );
      KEY( "Hometown",	ch->pcdata->hometown,		hometown_lookup(fread_word(fp)));

      if ( !str_cmp( word, "HpManaMove" ) || !str_cmp(word,"HMV")){
	ch->hit		= fread_number( fp );
	ch->bstat(max_hit)	= fread_number( fp ); /* ignored if version < 6 */
	ch->mana	= fread_number( fp );
	ch->bstat(max_mana)	= fread_number( fp );
	ch->move	= fread_number( fp );
	ch->bstat(max_move)	= fread_number( fp );
	fMatch = TRUE;
	break;
      }

      if ( !str_cmp( word, "HpManaMovePerm" ) || !str_cmp(word,"HMVP")){
	ch->bstat(max_hit)	= fread_number( fp );
	ch->bstat(max_mana)	= fread_number( fp );
	ch->bstat(max_move)	= fread_number( fp );

	fMatch = TRUE;
	break;
      }

      break;

    case 'I':
      KEY( "Id",		ch->id,			fread_number( fp ) );
      KEY( "Immune",	ch->bstat(imm_flags),	fread_flag( fp ) );
      KEY( "InvisLevel",	ch->invis_level,	fread_number( fp ) );
      KEY( "Inco",	ch->incog_level,	fread_number( fp ) );
      KEY( "Invi",	ch->invis_level,	fread_number( fp ) );
      //Added by SinaC 2000 for immtitle
      //KEY( "Immtit",      ch->pcdata->immtitle,   fread_string( fp ) );
      if ( !str_cmp( word, "Immtit" ) ){
	ch->pcdata->immtitle = fread_string( fp );
	fMatch = TRUE;
      }

      break;

    case 'L':
      KEY( "LastLevel",	ch->pcdata->last_level, fread_number( fp ) );
      KEY( "LLev",	ch->pcdata->last_level, fread_number( fp ) );
      KEY( "Level",	ch->level,		fread_number( fp ) );
      KEY( "Lev",	ch->level,		fread_number( fp ) );
      KEY( "Levl",	ch->level,		fread_number( fp ) );
      KEY( "LogO",	lastlogoff,		fread_number( fp ) );
      KEY( "LongDescr",	ch->long_descr,		fread_string( fp ) );
      KEY( "LnD",	ch->long_descr,		fread_string( fp ) );
      // Added by SinaC 2001
      KEY( "Lang",      ch->pcdata->language,   fread_number( fp ) );
      // Added by SinaC 2003
      KEY( "LineMode",  ch->desc->lineMode,     fread_number( fp ) );
      break;

    case 'N':
      KEYS( "Name",	ch->name,		fread_string( fp ) );
      // Added by SinaC 2001 for name acceptance
      KEY( "NameAccepted", ch->pcdata->name_accepted, fread_number( fp ) );
      break;

    case 'P':
      KEY( "Parts",	ch->bstat(parts),	fread_flag( fp ) );
      KEY( "Password",	ch->pcdata->pwd,	fread_string( fp ) );
      KEY( "Pass",	ch->pcdata->pwd,	fread_string( fp ) );
      KEY( "Petition",	ch->petition,	        clan_lookup(fread_string(fp)));
      KEY( "Played",	ch->played,		fread_number( fp ) );
      KEY( "Plyd",	ch->played,		fread_number( fp ) );
      KEY( "Points",	ch->pcdata->points,	fread_number( fp ) );
      KEY( "Pnts",	ch->pcdata->points,	fread_number( fp ) );
      KEY( "Position",	ch->position,		fread_number( fp ) );
      KEY( "Pos",	ch->position,		fread_number( fp ) );
      KEY( "Practice",	ch->practice,		fread_number( fp ) );
      KEY( "Prac",	ch->practice,		fread_number( fp ) );
      KEYS( "Prompt",   ch->prompt,             fread_string( fp ) );
      KEY( "Prom",	ch->prompt,		fread_string( fp ) );
      // Added by SinaC 2001 for mental user
      if ( !str_cmp( word, "Psp" )){
	ch->psp	= fread_number( fp );
	ch->bstat(max_psp)	= fread_number( fp );
	fMatch = TRUE;
	break;
      }
      break;

      //    case 'Q':
      //      KEY( "QuestPnts",   ch->pcdata->questpoints, fread_number( fp ) );
      //      KEY( "QuestNext",   ch->pcdata->nextquest,   fread_number( fp ) );
      //      KEY( "QuestCount",  ch->pcdata->countdown,   fread_number( fp ) );
      //      KEY( "QuestObj",    ch->pcdata->questobj,    fread_number( fp ) );
      //      KEY( "QuestObjLoc", ch->pcdata->questobjloc, fread_number( fp ) );
      //      KEY( "QuestMob",    ch->pcdata->questmob,    fread_number( fp ) );

      //      if ( !str_cmp( word, "QuestGiver" ) ){
      //	int vnum = fread_number( fp );
      //	MOB_INDEX_DATA *pMob = get_mob_index( vnum );
      //	if ( pMob == NULL )
      //	  bug("load_char_obj: Mob (questGiver) %d doesn't exist anymore", vnum );
      //	else
      //	  ch->pcdata->questgiver = find_mob( pMob );
      //	fMatch = TRUE;
      //	break;
      //      }
      //      break;
      
    case 'R':
      KEY( "Race",        ch->bstat(race), race_lookup(fread_string( fp ), TRUE) );
      KEY( "Resist",	ch->bstat(res_flags),	fread_flag( fp ) );

      if ( !str_cmp( word, "Room" ) ){
	ch->in_room = get_room_index( fread_number( fp ) );
	if ( ch->in_room == NULL )
	  ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
	fMatch = TRUE;
	break;
      }

      break;

    case 'S':
      KEY( "SavingThrow",	ch->bstat(saving_throw),	fread_number( fp ) );
      KEY( "Save",	ch->bstat(saving_throw),	fread_number( fp ) );
      KEY( "Scro",	ch->lines,		fread_number( fp ) );
      KEY( "Sex",		ch->bstat(sex),		fread_number( fp ) );
      KEY( "ShortDescr",	ch->short_descr,	fread_string( fp ) );
      KEY( "ShD",		ch->short_descr,	fread_string( fp ) );
      KEY( "Sec",         ch->pcdata->security,	fread_number( fp ) );	/* OLC */
      KEY( "Silv",        ch->silver,             fread_number( fp ) );
      KEY( "Size",        ch->bstat(size),        size_lookup( fread_word( fp )) );
       
      // Added by SinaC 2000 for stance
      KEY( "Stance",      ch->pcdata->stance,   fread_string( fp ) );

      if ( !str_cmp( word, "Skill" ) || !str_cmp(word,"Sk")) {
	int sn;
	int value, value2, value3;
	char *temp;

	value = fread_number( fp );
	// Added by SinaC 2000 for skill/spell level
	if ( ch->version > 7 ){
	  value2 = fread_number(fp);
	}
	else 
	  value2 = 0;
	// Added by SinaC 2001 for skill/spell level
	if ( ch->version > 8 ){
	  value3 = fread_number(fp);
	}
	else 
	  value3 = 0;


	temp = fread_word( fp ) ;
	sn = ability_lookup(temp);

	/* sn    = ability_lookup( fread_word( fp ) ); */
	if ( sn < 0 ){
	  //fprintf(stderr,"%s",temp);
	  bug( "Fread_char: unknown skill: %s.", temp );
	}
	else{
	  // Modified by SinaC 2000
	  ch->pcdata->ability_info[sn].learned = value;
	  if ( value2 < 0 ){
	    bug( "Fread_char: skill/spell '%s' invalid level (%d).", 
		 temp, value2 );
	    value2 = 0;
	  }
	  if ( value2 > ability_table[sn].nb_casting_level ){
	    bug( "Fread_char: skill/spell '%s' casting_level (%d) too high.", 
		 temp, value2 );
	    value2 = ability_table[sn].nb_casting_level;
	  }
	  if ( value2 == 0 
	       && ability_table[sn].nb_casting_level > 0 )
	    value2 = 1;
	  ch->pcdata->ability_info[sn].casting_level = value2;
	  // Added by SinaC 2001
	  if ( value3 < 0 || value3 > IM ) {
	    bug( "Fread_char: skill/spell '%s' invalid level (%d).", 
		 temp, value3 );
	    value3 = 0;
	  }
	  ch->pcdata->ability_info[sn].level = value3;
	}

	fMatch = TRUE;
      }

      break;

    case 'T':
      KEY( "TrueSex",     ch->bstat(sex),  	fread_number( fp ) );
      KEY( "TSex",	ch->bstat(sex),   fread_number( fp ) );
      KEY( "Trai",	ch->train,		fread_number( fp ) );
      KEY( "Trust",	ch->trust,		fread_number( fp ) );
      KEY( "Tru",		ch->trust,		fread_number( fp ) );
      KEY( "Triv",        ch->pcdata->trivia,     fread_number( fp ) );

      if ( !str_cmp( word, "Title" )  || !str_cmp( word, "Titl")){
	ch->pcdata->title = fread_string( fp );
	if (ch->pcdata->title[0] != '.' && ch->pcdata->title[0] != ',' 
	    &&  ch->pcdata->title[0] != '!' && ch->pcdata->title[0] != '?'){
	  sprintf( buf, " %s", ch->pcdata->title );
	  ch->pcdata->title = str_dup( buf );
	}
	fMatch = TRUE;
	break;
      }

      break;

    case 'V':
      KEY( "Version",     ch->version,		fread_number ( fp ) );
      KEY( "Vers",	ch->version,		fread_number ( fp ) );
      if ( !str_cmp( word, "Vnum" ) ){
	ch->pIndexData = get_mob_index( fread_number( fp ) );
	fMatch = TRUE;
	break;
      }
      KEY( "Vulner",	ch->bstat(vuln_flags),	fread_flag( fp ) );
      break;

    case 'W':
      KEY( "Wimpy",	ch->wimpy,		fread_number( fp ) );
      KEY( "Wimp",	ch->wimpy,		fread_number( fp ) );
      KEY( "Wizn",	ch->wiznet,		fread_flag( fp ) );
      break;
    }

    if ( !fMatch ){
      bug("Fread_char: Key '%s' doesn't match.",word );
      fread_to_eol( fp );
    }
  }
}

/* load a pet from the forgotten reaches */
void fread_pet( CHAR_DATA *ch, FILE *fp ) {
  const char *word;
  CHAR_DATA *pet;
  bool fMatch;
  int lastlogoff = current_time;
  int percent;

  /* first entry had BETTER be the vnum or we barf */
  word = feof(fp) ? "END" : fread_word(fp);
  if (!str_cmp(word,"Vnum")){
    int vnum;

    vnum = fread_number(fp);
    if (get_mob_index(vnum) == NULL){
      bug("Fread_pet: bad vnum %d.",vnum);
      pet = create_mobile(get_mob_index(MOB_VNUM_DUMMY));
    }
    else
      pet = create_mobile(get_mob_index(vnum));
  }
  else{
    bug("Fread_pet: no vnum in file.");
    pet = create_mobile(get_mob_index(MOB_VNUM_DUMMY));
  }
    
  for ( ; ; ){
    word 	= feof(fp) ? "END" : fread_word(fp);
    fMatch = FALSE;

    switch (UPPER(word[0])){
    case '*':
      fMatch = TRUE;
      fread_to_eol(fp);
      break;

    case 'A':
      KEY( "Act",		pet->act,		fread_flag(fp));
      KEY( "AfBy",	pet->bstat(affected_by),	fread_flag(fp));
      // Added by SinaC 2001
      KEY( "AfBy2",	pet->bstat(affected2_by),	fread_flag(fp));
      // Modified by SinaC 2001
      // Modified by SinaC 2000 for alignment/etho
      //KEY( "Alig",	pet->align.alignment,		fread_number(fp));
      if (!str_cmp( word, "Alig" )
	  || !str_cmp( word, "Alignment" ) ) {
	pet->bstat(alignment) = fread_number(fp);
	fMatch = TRUE;
	break;
      }
    	    
      if (!str_cmp(word,"ACs")){
	int i;
    	    	
	for (i = 0; i < 4; i++)
	  pet->bstat(ac0+i) = fread_number(fp);
	fMatch = TRUE;
	break;
      }

      if (!str_cmp(word,"Affects")){
	AFFECT_DATA *paf;
	int sn;
 
	paf = new_affect();
 
	sn = ability_lookup(fread_word(fp));
	int type = sn;
	if (sn < 0)
	  bug("Fread_char: unknown skill.");
	else
	  //paf->type = sn;
	  type = sn;
 
	//paf->where  = fread_number(fp);
	int where  = fread_number(fp);
	int loc;
	// Added by SinaC 2001
	if ( pet->version > 9 ) {
	  char buf2[MAX_STRING_LENGTH];
	  strcpy( buf2, fread_word(fp));
	  //paf->location = flag_value( attr_flags, buf2);
	  loc = flag_value( attr_flags, buf2);
	}
	else {
	  //paf->location   = fread_number( fp );
	  loc   = fread_number( fp );

	  log_stringf("before=== affect: %s  location: %d",
		      ability_table[sn].name,
		      //paf->location);
		      loc);

	  // Added by SinaC 2001
//	  if ( paf->location == 30 ) // 30 is the old ATTR_NUMBER
//	    paf->location = ATTR_NUMBER;
//	  else if ( paf->location == 31 )
//	    paf->location = ATTR_NA;
	  if ( loc == 30 ) // 30 is the old ATTR_NUMBER
	    loc = ATTR_NUMBER;
	  else if ( loc == 31 )
	    loc = ATTR_NA;

	  log_stringf("after=== affect: %s  location: %d",
		      ability_table[sn].name,
		      //paf->location);
		      loc);
	}
	//paf->op         = fread_number( fp );
	//paf->modifier   = fread_number( fp );
	//paf->duration   = fread_number( fp );
	//paf->level      = fread_number( fp );
	int op         = fread_number( fp );
	int modifier   = fread_number( fp );
	int duration   = fread_number( fp );
	int level      = fread_number( fp );

	int casting;
	// Modified by SinaC 2000
	if ( pet->version > 7 )
	  //paf->casting_level = fread_number( fp );
	  casting = fread_number( fp );
	else
	  //paf->casting_level = 0;
	  casting = 0;

	//Don't need to add a sanctuary/haste/protection evil/protection good cos' 
        // it's done in create_mobile, added by SinaC 2001
	if ( sn == gsn_sanctuary 
	     || sn == gsn_haste
	     || sn == gsn_protection_evil
	     || sn == gsn_protection_good ) {
	  fMatch = TRUE;
	  break;
	}

	createaff(*paf,duration,level,type,casting,type>0?AFFECT_ABILITY:AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT);
	addaff2(*paf,where,loc,op,modifier);

	paf->next       = pet->affected;
	pet->affected   = paf;
	fMatch          = TRUE;
	break;
      }

      if (!str_cmp(word,"AMod")){
	int stat;

	for (stat = 0; stat < MAX_STATS; stat++)
	  pet->bstat(stat0+stat) = fread_number(fp);
	fMatch = TRUE;
	break;
      }
    	     
      if (!str_cmp(word,"Attr")){
	int stat;

	for (stat = 0; stat < MAX_STATS; stat++)
	  pet->bstat(stat0+stat) = fread_number(fp);
	fMatch = TRUE;
	break;
      }
      break;
    	     
    case 'C':
      KEY( "Clan",       pet->clan,       clan_lookup(fread_string(fp)));
      KEY( "Comm",	pet->comm,		fread_flag(fp));
      break;
    	     
    case 'D':
      KEY( "Dam",	pet->bstat(damroll),		fread_number(fp));
      KEY( "Desc",	pet->description,	fread_string(fp));
      // Added by SinaC 2000
      KEY( "Dnumber",    pet->bstat(DICE_NUMBER),        fread_number(fp));
      KEY( "Dtype",      pet->bstat(DICE_TYPE),          fread_number(fp));
      KEY( "Damtype",    pet->bstat(dam_type),           fread_number(fp));

      break;
    	     
    case 'E':
      // Added by SinaC 2001
      if ( !str_cmp( word, "ExField" ) ) {
	fMatch = TRUE;
	fread_ex_fields( pet, fp );
      }

      if (!str_cmp(word,"End")){
	pet->leader = ch;
	pet->master = ch;
	ch->pet = pet;

	// Added by SinaC 2000
	SET_BIT(pet->bstat(affected_by), AFF_CHARM);
	//recompute(pet); NO NEED: done in char_to_room in nanny:CON_MOTD
	// Damn, that's really ugly

	/* adjust hp mana move up  -- here for speed's sake */
	percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
 
	if (percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
	    &&  !IS_AFFECTED(ch,AFF_PLAGUE)){
	  percent = UMIN(percent,100);
	  pet->hit    += (pet->bstat(max_hit) - pet->hit) * percent / 100;
	  pet->mana   += (pet->bstat(max_mana) - pet->mana) * percent / 100;
	  pet->move   += (pet->bstat(max_move) - pet->move)* percent / 100;
	  // Added by SinaC 2001 for mental user
	  pet->psp    += (pet->bstat(max_psp) - pet->psp)* percent / 100;
	}
	return;
      }
      KEY( "Exp",	pet->exp,		fread_number(fp));

      // Added by SinaC 2001 for etho
      if (!str_cmp( word, "Etho" ) ) {
	pet->bstat(etho) = fread_number(fp);
	fMatch = TRUE;
	break;
      }

      break;
    	     
    case 'G':
      KEY( "Gold",	pet->gold,		fread_number(fp));
      break;
    	     
    case 'H':
      KEY( "Hit",	pet->bstat(hitroll),		fread_number(fp));

      if (!str_cmp(word,"HMV")){
	pet->hit	= fread_number(fp);
	pet->bstat(max_hit)	= fread_number(fp);
	pet->mana	= fread_number(fp);
	pet->bstat(max_mana)	= fread_number(fp);
	pet->move	= fread_number(fp);
	pet->bstat(max_move)	= fread_number(fp);
	fMatch = TRUE;
	break;
      }
      break;
    	     
    case 'L':
      KEY( "Levl",	pet->level,		fread_number(fp));
      KEY( "LnD",	pet->long_descr,	fread_string(fp));
      KEY( "LogO",	lastlogoff,		fread_number(fp));
      break;
    	     
    case 'N':
      KEY( "Name",	pet->name,		fread_string(fp));
      break;
    	     
    case 'P':
      KEY( "Pos",	pet->position,		fread_number(fp));
      // Added by SinaC 2001 for mental user
      if (!str_cmp(word,"Psp")){
	pet->psp	= fread_number(fp);
	pet->bstat(max_psp)	= fread_number(fp);
	fMatch = TRUE;
	break;
      }

      break;
    	     
    case 'R':
      KEY( "Race",	pet->bstat(race), race_lookup(fread_string(fp), TRUE));
      break;

    case 'S' :
      KEY( "Save",	pet->bstat(saving_throw),	fread_number(fp));
      KEY( "Sex",		pet->bstat(sex),		fread_number(fp));
      KEY( "ShD",		pet->short_descr,	fread_string(fp));
      KEY( "Silv",        pet->silver,            fread_number( fp ) );
      break;
    	    
      if ( !fMatch ){
	bug("Fread_pet: no match.");
	fread_to_eol(fp);
      }

    }
  }
}

void fread_obj( CHAR_DATA *ch, FILE *fp ) {
  OBJ_DATA *obj;
  const char *word;
  int iNest;
  bool fMatch;
  bool fNest;
  bool fVnum;
  bool first;
  bool new_format;  /* to prevent errors */
  bool make_new;    /* update object */
    
  fVnum = FALSE;
  obj = NULL;
  first = TRUE;  /* used to counter fp offset */
  new_format = FALSE;
  make_new = FALSE;
    
    
  word   = feof( fp ) ? "End" : fread_word( fp );
  if (!str_cmp(word,"Vnum" )){
    int vnum;
    first = FALSE;  /* fp will be in right place */
 
    vnum = fread_number( fp );
    if (  get_obj_index( vnum )  == NULL ){
      bug( "Fread_obj: bad vnum %d.", vnum );
    }
    else{
      obj = create_object(get_obj_index(vnum),-1);
      new_format = TRUE;
    }
	    
  }

  if (obj == NULL || !obj->valid ){  /* either not found or old style */
    obj = new_obj();
    obj->name		= str_dup( "" );
    obj->short_descr	= str_dup( "" );
    obj->description	= str_dup( "" );
    obj->pIndexData = get_obj_index( OBJ_VNUM_DUMMY ); // SinaC 2003
  }

  fNest		= FALSE;
  fVnum		= TRUE;
  iNest		= 0;

  for ( ; ; ){
    if (first)
      first = FALSE;
    else
      word   = feof( fp ) ? "End" : fread_word( fp );
    fMatch = FALSE;

    switch ( UPPER(word[0]) ){
    case '*':
      fMatch = TRUE;
      fread_to_eol( fp );
      break;

    case 'A':
      if (!str_cmp(word,"Affects")){
	AFFECT_DATA *paf;
	int sn;
 
	paf = new_affect();

	sn = ability_lookup(fread_word(fp));
	int type = sn;
	if (sn < 0)
	  bug("Fread_obj: unknown skill.");
	else
	  //paf->type = sn;
	  type = sn;
 
	//paf->where  = fread_number(fp);
	int where  = fread_number(fp);
	int loc;
	// Added by SinaC 2001
	if ( ch->version > 9 ) {
	  char buf2[MAX_STRING_LENGTH];
	  strcpy( buf2, fread_word(fp));
	  //paf->location = flag_value( attr_flags, buf2);
	  loc = flag_value( attr_flags, buf2);
	}
	else {
	  //paf->location   = fread_number( fp );
	  loc   = fread_number( fp );

	  log_stringf("before=== affect: %s  location: %d",
		      ability_table[sn].name,
		      //paf->location);
		      loc);

	  // Added by SinaC 2001
	  //if ( paf->location == 30 ) // 30 is the old ATTR_NUMBER
	  //  paf->location = ATTR_NUMBER;
	  //else if ( paf->location == 31 )
	  //  paf->location = ATTR_NA;
	  if ( loc == 30 ) // 30 is the old ATTR_NUMBER
	    loc = ATTR_NUMBER;
	  else if ( loc == 31 )
	    loc = ATTR_NA;
	  log_stringf("after=== affect: %s  location: %d",
		      ability_table[sn].name,
		      //paf->location);
		      loc);

	}
	//	paf->op         = fread_number( fp );
	//	paf->modifier   = fread_number( fp );
	//	paf->duration   = fread_number( fp );
	//	paf->level      = fread_number( fp );

	int op         = fread_number( fp );
	int modifier   = fread_number( fp );
	int duration   = fread_number( fp );
	int level      = fread_number( fp );

	int casting;
	// Modified by SinaC 2000
	if ( ch->version > 7 )
	  //paf->casting_level = fread_number( fp );
	  casting = fread_number(fp);
	else
	  //paf->casting_level = 0;
	  casting = 0;

	createaff(*paf,duration,level,type,casting,type>0?AFFECT_ABILITY:AFFECT_INHERENT|AFFECT_NON_DISPELLABLE|AFFECT_PERMANENT);
	addaff2(*paf,where,loc,op,modifier);

	paf->next       = obj->affected;
	obj->affected   = paf;
	fMatch          = TRUE;
	break;
      }
      break;

    case 'C':
      KEY( "Cond",	obj->condition,		fread_number( fp ) );
      KEY( "Cost",	obj->cost,		fread_number( fp ) );
      break;

    case 'D':
      KEY( "Description",	obj->description,	fread_string( fp ) );
      KEY( "Desc",	obj->description,	fread_string( fp ) );
      break;

    case 'E':
      // Added by SinaC 2001
      if ( !str_cmp( word, "ExField" ) ) {
	fMatch = TRUE;
	fread_ex_fields( obj, fp );
      }

      if ( !str_cmp( word, "Enchanted")){
	obj->enchanted = TRUE;
	fMatch 	= TRUE;
	break;
      }

      KEY( "ExtraFlags",  obj->base_extra,       fread_number( fp ) );
      KEY( "ExtF",        obj->base_extra,       fread_number( fp ) );

      if ( !str_cmp( word, "ExtraDescr" ) || !str_cmp(word,"ExDe")){
	EXTRA_DESCR_DATA *ed;

	ed = new_extra_descr();

	ed->keyword		= fread_string( fp );
	ed->description		= fread_string( fp );
	ed->next		= obj->extra_descr;
	obj->extra_descr	= ed;
	fMatch = TRUE;
      }

      if ( !str_cmp( word, "End" ) ){
	if ( !fNest || ( fVnum && obj->pIndexData == NULL ) ){
	  bug( "Fread_obj: incomplete object");
	  free_obj(obj);
	  return;
	}
	else{
	  if ( !fVnum ){
	    free_obj( obj );
	    obj = create_object( get_obj_index( OBJ_VNUM_DUMMY ), 0 );
	  }

	  if (!new_format){
	    obj->next	= object_list;
	    object_list	= obj;
	    obj->pIndexData->count++;
	  }

	  if (!obj->pIndexData->new_format 
	      && obj->item_type == ITEM_ARMOR
	      &&  obj->baseval[1] == 0){
	    obj->baseval[1] = obj->baseval[0];
	    obj->baseval[2] = obj->baseval[0];
	  }
	  if (make_new){
	    int wear;

	    wear = obj->wear_loc;
	    extract_obj(obj);

	    obj = create_object(obj->pIndexData,0);
	    obj->wear_loc = wear;
	  }
	  recompobj(obj);

	  if ( obj->size < SIZE_TINY || obj->size > SIZE_NOSIZE ) {
	    bug("Invalid size, assumin SIZE_NOSIZE");
	    obj->size = SIZE_NOSIZE;
	  }

	  /*
	    for ( int i = 0; i <= iNest; i++ )
	    log_stringf("rgObjNest[%2d]: %s", i, rgObjNest[i]?rgObjNest[i]->name:"(null)");
	  */

	  // Modified by SinaC 2001
	  int baseNest = iNest;
	  while ( iNest > 0
		  && ( rgObjNest[iNest-1] == NULL
		       || rgObjNest[iNest-1]->item_type != ITEM_CONTAINER 
		       || rgObjNest[iNest-1] == obj ) )
	    iNest--;
	  if ( baseNest != iNest )
	    bug("Item (%d) nested down %d levels",
		obj->pIndexData->vnum,
		baseNest - iNest );

	  if ( iNest == 0 )
	    obj_to_char( obj, ch );
	  else
	    obj_to_obj( obj, rgObjNest[iNest-1] );

	  /* Old system
	  if ( iNest == 0 || rgObjNest[iNest] == NULL )
	    obj_to_char( obj, ch );
	  else
	    obj_to_obj( obj, rgObjNest[iNest-1] );
	  */
	  return;
	}
      }
      break;

    case 'I':
      KEY( "ItemType",	obj->item_type,		fread_number( fp ) );
      KEY( "Ityp",	obj->item_type,		fread_number( fp ) );
      break;

    case 'L':
      KEY( "Level",	obj->level,		fread_number( fp ) );
      KEY( "Lev",		obj->level,		fread_number( fp ) );
      break;

    case 'N':
      KEY( "Name",	obj->name,		fread_string( fp ) );

      if ( !str_cmp( word, "Nest" ) ){
	iNest = fread_number( fp );
	if ( iNest < 0 || iNest >= MAX_NEST ){
	  bug( "Fread_obj: bad nest %d.", iNest );
	}
	else{
	  // Modified by SinaC 2001
	  /*
	  if ( obj->item_type == ITEM_CONTAINER )
	    rgObjNest[iNest] = obj;
	  else
	    rgObjNest[iNest] = NULL;
	  */
	  rgObjNest[iNest] = obj;
	  fNest = TRUE;
	}
	fMatch = TRUE;
      }
      break;

    case 'O':
      // Added by SinaC 2000
      if ( !str_cmp( word, "Owner" ) ){
	obj->owner = str_dup( fread_word(fp) );
	fMatch = TRUE;
      }

      if ( !str_cmp( word,"Oldstyle" ) ){
	if (obj->pIndexData != NULL && obj->pIndexData->new_format)
	  make_new = TRUE;
	fMatch = TRUE;
      }
      break;

      
  // SinaC 2003, not needed: we can safely use pIndexData because restriction/ability_upgrade
  //  cannot be dynamically modified during play
      // Added by SinaC 2001
//    case 'R':
//      if ( !str_cmp( word, "Restriction" ) ) {
//	RESTR_DATA *restr;
//	char buf[128];
//	int kind = fread_number(fp);
//	if ( kind == 0 ) {
//	  restr = new_restriction();
//	  strcpy( buf, fread_word(fp) );
//	  if ( is_number( buf ) ) 
//	    restr->type              = atoi( buf ); 
//	  else
//	    restr->type              = flag_value( restr_flags, buf );
//	  restr->ability_r             = FALSE;
//	  restr->sn                  = -1;
//	  // value
//	  restr->value               = fread_number(fp);
//	  // NOT ?
//	  restr->not_r               = fread_number(fp);
//	  
//	  // update the linked list
//	  restr->next                = obj->restriction;
//	  obj->restriction     = restr;
//	  fMatch = TRUE;
//	}
//	else if ( kind == 1 ) {
//	  restr = new_restriction();
//	  restr->type                = RESTR_ABILITY;
//	  restr->ability_r             = TRUE;
//	  restr->sn                  = ability_lookup(fread_word(fp));
//	  // value
//	  restr->value               = fread_number(fp);
//	  // NOT ?
//	  restr->not_r               = fread_number(fp);
//	  
//	  // update the linked list
//	  restr->next                = obj->restriction;
//	  obj->restriction     = restr;
//	  fMatch = TRUE;
//	}
//	else {
//	  bug("Invalid restriction!!!");
//	  fread_word(fp); fread_number(fp); fread_number(fp);
//	}
//      }

    case 'S':
      KEY( "ShortDescr",	obj->short_descr,	fread_string( fp ) );
      KEY( "ShD",		obj->short_descr,	fread_string( fp ) );
      // Added by SinaC 2003
      KEY( "Size",              obj->size,              size_lookup(fread_word(fp) ) );

      if ( !str_cmp( word, "Spell" ) ){
	int iValue;
	int sn;

	iValue = fread_number( fp );
	sn     = ability_lookup( fread_word( fp ) );
	if ( iValue < 0 || iValue > 3 ){
	  bug( "Fread_obj: bad iValue %d.", iValue );
	}
	else if ( sn < 0 ){
	  bug( "Fread_obj: unknown ability.");
	}
	else{
	  obj->value[iValue] = sn;
	}
	fMatch = TRUE;
	break;
      }

      break;

    case 'T':
      KEY( "Timer",	obj->timer,		fread_number( fp ) );
      KEY( "Time",	obj->timer,		fread_number( fp ) );
      break;

    case 'V':
      if ( !str_cmp( word, "Values" ) || !str_cmp(word,"Vals")){
	obj->baseval[0]   = fread_number( fp );
	obj->baseval[1]   = fread_number( fp );
	obj->baseval[2]   = fread_number( fp );
	obj->baseval[3]   = fread_number( fp );
	if (obj->item_type == ITEM_WEAPON && obj->baseval[0] == 0)
	  obj->baseval[0] = obj->pIndexData->value[0];
	fMatch		= TRUE;
	break;
      }

      if ( !str_cmp( word, "Val" ) ){
	obj->baseval[0] = fread_number( fp );
	obj->baseval[1] = fread_number( fp );
	obj->baseval[2] = fread_number( fp );
	obj->baseval[3] = fread_number( fp );
	obj->baseval[4] = fread_number( fp );
	fMatch = TRUE;
	break;
      }

      if ( !str_cmp( word, "Vnum" ) ){
	int vnum;

	vnum = fread_number( fp );
	if ( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
	  bug( "Fread_obj: bad vnum %d.", vnum );
	else
	  fVnum = TRUE;
	fMatch = TRUE;
	break;
      }
      break;

    case 'W':
      KEY( "WearFlags",	obj->wear_flags,	fread_number( fp ) );
      KEY( "WeaF",	obj->wear_flags,	fread_number( fp ) );
      KEY( "WearLoc",	obj->wear_loc,		fread_number( fp ) );
      KEY( "Wear",	obj->wear_loc,		fread_number( fp ) );
      KEY( "Weight",	obj->weight,		fread_number( fp ) );
      KEY( "Wt",		obj->weight,		fread_number( fp ) );
      break;

    }

    if ( !fMatch ){
      bug( "Fread_obj: no match.");
      fread_to_eol( fp );
    }
  }
}

//**************************************************************************************************
//***************************************** New save ***********************************************
//Player
//Player/Pet <Player name/Pet vnum>
//  Act = <LIST OF STRING>;
//  AfBy = <LIST OF STRING> ( <STRING aff>, ...);
//  AfBy2 = <LIST OF STRING> ( <STRING aff2>, ...);
//  Address = <STRING>;
//  Alignment = <INTEGER>;
//  Alias = ( <STRING alias name>, <STRING alias substitution> );
//  ACs = ( <INTEGER>, <INTEGER>, <INTEGER>, <INTEGER> ); // pierce, bash, slash, magic
//  Affect = ( <STRING name>, <INTEGER where>, <STRING location>, <INTEGER operation>,
//              <INTEGER modifier>, <INTEGER duration>, <INTEGER level>, <INTEGER casting level> );
//  Attr = ( <INTEGER Str>, <INTEGER Int>, <INTEGER Wis>, <INTEGER Dex>, <INTEGER Con>);
//  Bin = <STRING>;
//  Bout = <STRING>;
//  Beacons = ( <INTEGER>, ... ); // MAX_BEACONS
//  Boards = ( ( <STRING>, <INTEGER> ), ...);
//  Classes = <INTEGER>; // classes is the right value
//  Clan = <STRING>;
//  ClanStatus = <INTEGER>;
//  Condition = ( <INTEGER>, <INTEGER>, <INTEGER> ); // pcdata->condition[0] [1] [2]
//  Comm = <LIST OF STRING>;
//  Damroll = <INTEGER>;
//  Dnumber = <INTEGER>; // mob only
//  Dtype = <INTEGER>; // mob only
//  DamType = <STRING>;
//  Desc = <STRING>;
//  DisabledCmd = ( <STRING cmd name>, <STRING disabler name> );
//  Experience = <INTEGER>;
//  Etho = <STRING>;
//  Forms = <LIST OF STRING>;
//  Faction = ( <STRING faction name>, <INTEGER value> );
//  Gold = <INTEGER>;
//  God = <STRING>;
//  Gset = <INTEGER>;
//  Group = <STRING>;
//  Hitroll = <INTEGER>;
//  Hometown = <STRING>;
//  HMVP = ( ( <INTEGER hp>, <INTEGER max hp> ), ... mana, move, psp );
//  Id = <INTEGER>;
//  Immunities = <LIST OF STRING>;
//  Invis = <INTEGER>;
//  Inco = <INTEGER>;
//  Immtitle = <STRING>;
//  LastLevel = <INTEGER>;
//  Level = <INTEGER>;
//  LogO = <INTEGER>;
//  LnD = <STRING>;
//  Language = <INTEGER>;
//  LineMode = <INTEGER>;
//  Name = <STRING>;
//  NameAccepted = <INTEGER>;
//  Parts = <LIST OF STRING>;
//  Password = <STRING>;
//  Petition = <STRING>;
//  Played = <INTEGER>;
//  Pnts = <INTEGER>;
//  Position = <INTEGER>;
//  Prac = <INTEGER>;
//  Prompt = <STRING>;
//--  QuestPnts = <INTEGER>;
//--  QuestNext = <INTEGER>;
//--  QuestCount = <INTEGER>;
//--  QuestObj = <INTEGER>;
//--  QuestObjLoc = <INTEGER>;
//--  QuestMob = <INTEGER>;
//--  QuestGiver = <INTEGER>;
//  Race = <STRING>;
//  Resististances = <LIST OF STRING>;
//  Room = <INTEGER>;
//  SavingThrow = <INTEGER>;
//  Scroll = <INTEGER>;
//  Sex = <INTEGER>;
//  ShD = <STRING>;
//  Security = <INTEGER>;
//  Silver = <INTEGER>;
//  Size = <STRING>;
//  Stance = <STRING>;
//  Ability = ( <INTEGER learned>, <INTEGER casting_level>, <INTEGER level>, <STRING name> );
//  Train = <INTEGER>;
//  Trust = <INTEGER>;
//  Trivia = <INTEGER>;
//  Title = <STRING>;
//  Version = <INTEGER>;
//  Vulnerabilities = <LIST OF STRING>;
//  Wimpy = <INTEGER>;
//  Wiznet = <STRING>;
//  Exfield = ( <STRING name>, <STRING type> <INTEGER lifetime>, <STRING/INTEGER/LIST value> );
//  Pet <Pet vnum> {
//   ...
//  }
//  Obj <Obj name> {
//   ...
//  }
//
//Object
//
//Obj <INTEGER Object Vnum> {
//  Affect = <See Player/Pet>;
//  Cond = <INTEGER>;
//  Cost = <INTEGER>;
//  Desc = <STRING>;
//  ExField = <See Player/Pet>
//  Enchanted = <BOOLEAN>; // not necessary if False
//  ExtraFlags = <INTEGER>;
//  ExtraDescr = ( <STRING keyword>, <STRING description> );
//  ItemType = <INTEGER>;
//  Level = <INTEGER>;
//  Name = <STRING>;
//  Owner = <STRING>;
//  Oldstyle = <BOOLEAN>; // not necessary if False
//  Restriction = ( <INTEGER kind>, <STRING type/ability name>, <INTEGER value>, <INTEGER not_r???> );
//  ShD = <STRING>;
//  Size = <INTEGER>;
//  Spell = ( <INTEGER iValue 0-->4 >, <STRING ability name> );
//  Timer = <INTEGER>;
//  Values = ( <INTEGER v0>, ...<INTEGER v4> );
//  Vnum = <INTEGER>;
//  WearFlags = <INTEGER>;
//  WearLoc = <INTEGER>;
//  Weight = <INTEGER>;
//  Obj <INTEGER Obj Vnum> {
//  ...
//  }
//}

// Tag used in pFile and world state to store entities in extra fields
#define TAG_NULL_ENTITY "<NULL>"
#define TAG_CHAR_ENTITY "<char>"
#define TAG_OBJ_ENTITY  "<obj>"
#define TAG_ROOM_ENTITY "<room>"
// Common extra fields: if extra fields we need to save is master/carrier/container
//  we can only save a special tag, because we can find back related entity
// We must be sure that master, container and carrier have been read
#define TAG_CHAR_MASTER "<master>"
#define TAG_OBJ_CONTAINER "<container>"
#define TAG_CHAR_CARRIER "<carrier>"

const char *stringify_entity( ENTITY_DATA *entityToSave, ENTITY_DATA *e ) {
  char *buf = new char [1024];
  if ( e == NULL || !e->valid )
    return quotify(TAG_NULL_ENTITY);
  switch ( e->kind ) {
  case CHAR_ENTITY: {
    CHAR_DATA *ch = (CHAR_DATA*)e;
    bool done = FALSE;
    if ( entityToSave->kind == OBJ_ENTITY ) { // if entity to save is an obj, maybe field is the carrier
      OBJ_DATA *obj = (OBJ_DATA *)entityToSave;
      if ( obj->carried_by == ch ) { // it's the carrier
	sprintf( buf, "( %s, %s )",
		 quotify(TAG_CHAR_ENTITY),
		 quotify(TAG_CHAR_CARRIER));
	done = TRUE;
      }
    }
    else if ( entityToSave->kind == CHAR_ENTITY ) { // if entity to save is a mob, maybe field is the master
      CHAR_DATA *slave = (CHAR_DATA *)entityToSave;
      if ( slave->master == ch ) { // it's the master
	sprintf( buf, "( %s, %s )",
		 quotify(TAG_CHAR_ENTITY),
		 quotify(TAG_CHAR_MASTER));
	done = TRUE;
      }
    }
    if ( !done ) { // not carrier, neither master: normal save
      if ( IS_NPC(ch) ) // NPC: store vnum and room vnum
	sprintf( buf, "( %s, %d, %d )", 
		 quotify(TAG_CHAR_ENTITY),
		 ch->pIndexData?ch->pIndexData->vnum:-1,
		 ch->in_room?ch->in_room->vnum:-1 );
      else              // PC: store name
	sprintf( buf, "( %s, %s )", 
		 quotify(TAG_CHAR_ENTITY),
		 quotify(NAME(ch)) );
    }
    break;
  }
  case OBJ_ENTITY: {
    OBJ_DATA *obj = (OBJ_DATA *)e;
    bool done = FALSE;
    if ( entityToSave->kind == OBJ_ENTITY ) { // if entity to save is an obj, maybe field is the container
      OBJ_DATA *obj = (OBJ_DATA *)entityToSave;
      if ( obj->in_obj == obj ) { // it's the container
	sprintf( buf, "( %s, %s )",
		 quotify(TAG_OBJ_ENTITY),
		 quotify(TAG_OBJ_CONTAINER));
	done = TRUE;
      }
    }
    if ( !done ) { // not container: normal save
      if ( obj->in_room ) { // OBJ in room: store obj vnum and recursive call for room
	const char *s = stringify_entity( entityToSave, obj->in_room );
	if ( s )
	  sprintf( buf, "( %s, %d, %s )",
		   quotify(TAG_OBJ_ENTITY),
		   obj->pIndexData?obj->pIndexData->vnum:-1, s );
	else
	  return NULL;
      }
      else if ( obj->in_obj ) { // OBJ in obj: store obj vnum and recursive call for container
	const char *s = stringify_entity( entityToSave, obj->in_obj );
	if ( s )
	  sprintf( buf, "( %s, %d, %s )",
		   quotify(TAG_OBJ_ENTITY),
		   obj->pIndexData?obj->pIndexData->vnum:-1,s);
	else
	  return NULL;
      }
      else if ( obj->carried_by ) { // OBJ on char: store obj vnum and recursive call for carrier
	const char *s = stringify_entity( entityToSave, obj->carried_by );
	if ( s )
	  sprintf( buf, "( %s, %d, %s )",
		   quotify(TAG_OBJ_ENTITY),
		   obj->pIndexData?obj->pIndexData->vnum:-1,s);
	else
	  return NULL;
      }
      else
	return NULL;
    }
    break;
  }
  case ROOM_ENTITY: {
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *)e;
    sprintf( buf, "( %s, %d )",
	     quotify(TAG_ROOM_ENTITY),
	     room->vnum );
    break;
  }
  }
  return buf;
}
const char *format_extra_field( ENTITY_DATA *ent, const Value &v ) {
  switch(v.typ) {
  case SVT_INT: return v.asStr(); break;
  case SVT_STR: return quotify(v.asStr()); break;
  case SVT_ENTITY: return stringify_entity( ent, v.asEntity() ); break;
  case SVT_LIST: {
    ValueList *l = v.asList();
    char *buf = new char [4096];
    strcpy(buf,"(");
    for ( int i = 0; i < l->size; i++ ) {
      const char *s = format_extra_field( ent, l->elems[i]);
      if ( s == NULL )
	return NULL;
      strcat(buf, s);
      if ( i < l->size-1 )
	strcat(buf,",");
    }
    strcat(buf,")");

    //l->explicit_free();
    return buf;
    break;
  }
  default:
    return NULL;
    break;
  }
}
void new_save_extra_fields( ENTITY_DATA *ent, FILE *fp ) {
  for ( FIELD_DATA *f = ent->ex_fields; f; f = f->next ) {
    const char *format = format_extra_field( ent, f->var );
    if ( format != NULL )
      sh_fprintf( fp,
		 "  ExField = ( %s, %s, %d, %s );\n",
		 quotify(f->name), quotify(script_type_name(f->var.typ)), 
		 f->lifetime, format );

  }
}

void new_save_affects( AFFECT_DATA *pafList, FILE *fp ) {
  // Slick recursion to write lists backwards,
  //  so loading them will load in forwards order.
  if ( pafList == NULL )
    return;
  if ( pafList->next != NULL )
    new_save_affects( pafList->next, fp );
  new_save_affect( pafList, fp );
}

void new_save_obj_list( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp ) {

  if ( obj == NULL || !obj->valid )
    return;

  // SinaC: this recursion may consume too memory, if obj list is too big
  //   solution: create a temporary OBJ_DATA list storing obj list in reverse order
  //              and then save this list without any recursion
  // Slick recursion to write lists backwards,
  //  so loading them will load in forwards order.
  if ( obj->next_content != NULL )
    new_save_obj_list( ch, obj->next_content, fp );

  // Unique items are not save in pFile
  if ( IS_SET( obj->extra_flags, ITEM_UNIQUE ) )
    return;
  // Castrate storage characters.
  if ( ch != NULL
       && ( !IS_NPC(ch) || ch->master != NULL || ch->leader != NULL ) // only player and charmed mob
       && ( (ch->level < obj->level - 5 && obj->item_type != ITEM_CONTAINER )
	    ||   obj->item_type == ITEM_KEY
	    ||   (obj->item_type == ITEM_MAP && !obj->value[0])) )
    return;

  data_depth++;

  sh_fprintf( fp, "Obj %d {\n", obj->pIndexData->vnum );

  if (!obj->pIndexData->new_format)
    sh_fprintf( fp, "  Oldstyle = true;\n");
  if (obj->enchanted)
    sh_fprintf( fp,"  Enchanted = true;\n");

  // these datas are only used if they do not match the defaults
  if ( obj->name != obj->pIndexData->name)
    sh_fprintf( fp, "  Name = %s;\n", quotify(obj->name) );
  if ( obj->short_descr != obj->pIndexData->short_descr)
    sh_fprintf( fp, "  ShD = %s;\n", quotify(obj->short_descr) );
  if ( obj->description != obj->pIndexData->description)
    sh_fprintf( fp, "  Desc = %s;\n", quotify(obj->description) );
  if ( obj->base_extra != obj->pIndexData->extra_flags)
    sh_fprintf( fp, "  ExtraFlags = (%s);\n", list_flag_string( obj->base_extra, extra_flags, "'", ", ") );
  if ( obj->wear_flags != obj->pIndexData->wear_flags)
    sh_fprintf( fp, "  WearFlags = (%s);\n", list_flag_string( obj->wear_flags, wear_flags, "'", ", ") );
  if ( obj->item_type != obj->pIndexData->item_type)
    sh_fprintf( fp, "  ItemType = %s;\n", quotify(flag_string(type_flags, obj->item_type)) );
  if ( obj->weight != obj->pIndexData->weight)
    sh_fprintf( fp, "  Weight = %d;\n",	obj->weight );
  if ( obj->condition != obj->pIndexData->condition)
    sh_fprintf( fp, "  Condition = %d;\n", obj->condition );
  if (obj->level != obj->pIndexData->level)
    sh_fprintf( fp, "  Level = %d;\n", obj->level );
  if ( obj->cost != obj->pIndexData->cost )
    sh_fprintf( fp, "  Cost = %d;\n", obj->cost );
  if ( obj->size != obj->pIndexData->size )
    sh_fprintf( fp, "  Size %s;\n",   size_table[obj->size].name );
  if ( obj->baseval[0] != obj->pIndexData->value[0]
      || obj->baseval[1] != obj->pIndexData->value[1]
      || obj->baseval[2] != obj->pIndexData->value[2]
      || obj->baseval[3] != obj->pIndexData->value[3]
      || obj->baseval[4] != obj->pIndexData->value[4] )
    sh_fprintf( fp, "  Values = ( %d, %d, %d, %d, %d );\n",
		obj->baseval[0], obj->baseval[1], obj->baseval[2], obj->baseval[3],
		obj->baseval[4]	);
  if ( obj->material != obj->pIndexData->material )
    sh_fprintf( fp, "  Material = %s;\n", quotify( flag_string( material_flags, obj->material ) ) );
  if ( obj->clazz != obj->pIndexData->program
       && obj->clazz != default_obj_class )
    sh_fprintf( fp, "  Clazz = %s;\n", quotify( obj->clazz->name ) );

  // variable data
  if ( obj->owner != NULL && obj->owner[0] != '\0' )
    sh_fprintf( fp, "  Owner = %s;\n", quotify(obj->owner) );
  if ( obj->wear_loc != WEAR_NONE )
    sh_fprintf( fp, "  WearLoc = %s;\n", quotify(flag_string( wear_loc_flags, obj->wear_loc )) );
  if ( obj->timer != 0 )
    sh_fprintf( fp, "  Timer = %d;\n", obj->timer );

  // enchanted scroll, potion, ...
  switch ( obj->item_type ) {
  case ITEM_POTION:
  case ITEM_SCROLL:
  case ITEM_PILL:
  case ITEM_TEMPLATE: {
    for ( int i = 1; i < 5; i++ )
      if ( obj->value[i] != obj->baseval[i] && obj->value[i] > 0 )
	sh_fprintf( fp, "  Spell = ( %d, %s );\n",
		    i, quotify(ability_table[obj->value[i]].name) );
    break;
  }
  case ITEM_STAFF:
  case ITEM_WAND:
    if ( obj->value[3] != obj->baseval[3] && obj->value[3] > 0 )
      sh_fprintf( fp, "  Spell = ( 3, %s );\n", 
		  quotify(ability_table[obj->value[3]].name) );
    break;
  }

  // affects
  //for ( AFFECT_DATA *paf = obj->affected; paf != NULL; paf = paf->next )
  //  new_save_affects( paf, fp );
  new_save_affects( obj->affected, fp );

  // SinaC 2003, not needed: we can safely use pIndexData because restriction/ability_upgrade
  //  cannot be dynamically modified during play
//  // restrictions
//  for ( RESTR_DATA *restr = obj->restriction; restr != NULL; restr = restr->next )
//    if ( !restr->ability_r )
//      sh_fprintf(fp,
//		 "  Restriction = ( %d, %s, %d, %d );\n",
//		 0, quotify(flag_string(restr_flags,restr->type)),
//		 restr->value, restr->not_r );
//    else
//      sh_fprintf(fp,
//		 "  Restriction = ( %d, %s, %d, %d );\n",
//		 1, quotify(ability_table[restr->sn].name),
//		 restr->value, restr->not_r );
//
//  // ability upgrade
//  for ( ABILITY_UPGRADE *upgr = obj->upgrade; upgr != NULL; upgr = upgr->next )
//    fprintf( fp,
//	     "  Upgrade = ( %s, %d );\n",
//	     quotify(ability_table[upgr->sn].name),
//	     upgr->value );

  // extra descr
  for ( EXTRA_DESCR_DATA *ed = obj->extra_descr; ed != NULL; ed = ed->next )
    sh_fprintf( fp, "  ExtraDescr = ( %s, %s );\n",
		quotify(ed->keyword), quotify(ed->description) );

  new_save_extra_fields( obj, fp );

  // container?
  if ( obj->contains != NULL )
    //for ( OBJ_DATA *obj2 = obj->contains; obj2 != NULL; obj2 = obj2->next_content )
    new_save_obj_list( ch, obj->contains, fp );

  sh_fprintf( fp, "}\n" );

  data_depth--;
}

void new_save_char( CHAR_DATA *ch, FILE *fp ) {
  data_depth++;
  int saveact, savecomm;

  if ( ch == NULL || !ch->valid )
    return;

  saveact = ch->act;
  savecomm = ch->comm;

  if ( !IS_NPC(ch) ) { // a player
    sh_fprintf( fp, "Player %s {\n", quotify( ch->name ) );
    REMOVE_BIT(saveact,PLR_MORTAL);
    REMOVE_BIT(savecomm,COMM_BUILDING);
    REMOVE_BIT(savecomm,COMM_EDITING);
    sh_fprintf( fp, "  Id = %ld;\n", ch->id );
    sh_fprintf( fp, "  Version = %d;\n", PLAYER_VERSION );
    sh_fprintf( fp, "  LogO = %ld;\n", current_time );
    if ( ch->desc->connected == CON_REBIRTH )
      sh_fprintf( fp, "  Rebirth = %s;\n", quotify( flag_string( races_flags, ch->pcdata->tmpRace ) ) );
    else if ( ch->desc->connected == CON_REMORT )
      sh_fprintf( fp, "  Remort = %s;\n", quotify( flag_string( races_flags, ch->pcdata->tmpRace ) ) );
  }
  else if ( IS_NPC(ch) && ch->master != NULL && ch->master->pet == ch ) { // a pet
    sh_fprintf( fp, "Pet %d {\n", ch->pIndexData->vnum );
    sh_fprintf( fp, "  LogO = %ld;\n", current_time );
  }
  else if ( IS_NPC(ch) ) { // a mobile: used to save mud state before a copyover
    sh_fprintf( fp, "Mobile %d {\n", ch->pIndexData->vnum );
    // store ptr address to be able to reform group and restarts fight after copyover
    sh_fprintf( fp, "  Ptr = %d;\n", (int)ch );
    if ( ch->master != NULL )
	 //&& ch->master->in_room == ch->in_room ) // only if master in the same room
      if ( IS_NPC(ch->master) ) // if master is NPC: store ptr address
	sh_fprintf( fp, "  Master = %d;\n", (int)ch->master );
      else // if master is PC: store player name
	sh_fprintf( fp, "  Master = %s;\n", quotify(NAME(ch->master)));
    if ( ch->leader != NULL )
	 //&& ch->master->in_room == ch->in_room ) // only if master in the same room
      if ( IS_NPC(ch->leader) ) // if leader is NPC: store ptr address
	sh_fprintf( fp, "  Leader = %d;\n", (int)ch->leader );
      else // if leader is PC: store player name
	sh_fprintf( fp, "  Leader = %s;\n", quotify(NAME(ch->leader)));
    if ( ch->fighting != NULL
	 && ch->fighting->in_room == ch->in_room ) // only if fighting in the same room
      if ( IS_NPC(ch->fighting) ) // if fighting is NPC: store ptr address
	sh_fprintf( fp, "  Fighting = %d;\n", (int)ch->fighting );
      else // if fighting is PC: store player name
	sh_fprintf( fp, "  Fighting = %s;\n", quotify(NAME(ch->fighting)) );
    if ( ch->hunting != NULL )
      if ( IS_NPC( ch->hunting ) ) // if hunted is NPC: store ptr address
	sh_fprintf( fp, "  Hunting = %s;\n", (int)ch->hunting );
      else // if hunted is PC: store player name
	sh_fprintf( fp, "  Hunting = %s;\n", quotify(NAME(ch->hunting)) );
  }
  else
    p_error("Big problem in new_save_char: trying to save a non-NPC non-PC char!!!!");

  if ( IS_NPC(ch) && ( ch->name != ch->pIndexData->player_name ) )
    sh_fprintf( fp, "  Name = %s;\n", quotify( ch->name ) );
  if ( ( !IS_NPC(ch) && ch->short_descr[0] != '\0')
       || ( IS_NPC(ch) && ch->short_descr != ch->pIndexData->short_descr ) )
    sh_fprintf( fp, "  ShD = %s;\n", quotify(ch->short_descr));
  if ( ( !IS_NPC(ch) && ch->long_descr[0] != '\0')
       || ( IS_NPC(ch) && ch->long_descr != ch->pIndexData->long_descr ) )
    sh_fprintf( fp, "  LnD = %s;\n", quotify(ch->long_descr));
  if ( ( !IS_NPC(ch) && ch->description[0] != '\0')
       || ( IS_NPC(ch) && ch->description != ch->pIndexData->description ) )
    sh_fprintf( fp, "  Desc = %s;\n", quotify(ch->description));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(race) != ch->pIndexData->race ) )
    //sh_fprintf( fp, "  Race = %s;\n", quotify(pc_race_table[ch->bstat(race)].name) );
    sh_fprintf( fp, "  Race = %s;\n", quotify(race_table[ch->bstat(race)].name) );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(sex) != ch->pIndexData->sex ) )
    sh_fprintf( fp, "  Sex = %s;\n", quotify(sex_table[ch->bstat(sex)].name) );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(size) != ch->pIndexData->size ) )
    sh_fprintf( fp, "  Size = %s;\n", quotify(size_table[ch->bstat(size)].name) );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(dam_type) != ch->pIndexData->dam_type) )
    sh_fprintf( fp, "  DamType = %s;\n", quotify(attack_table[ch->bstat(dam_type)].name));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(imm_flags) != ch->pIndexData->imm_flags ) )
    sh_fprintf( fp, "  Immunities = (%s);\n", list_flag_string( ch->bstat(imm_flags), irv_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(res_flags) != ch->pIndexData->res_flags ) )
    sh_fprintf( fp, "  Resistances = (%s);\n", list_flag_string( ch->bstat(res_flags), irv_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(vuln_flags) != ch->pIndexData->vuln_flags ) )
    sh_fprintf( fp, "  Vulnerabilities = (%s);\n", list_flag_string( ch->bstat(vuln_flags), irv_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(form) != ch->pIndexData->form ) )
    sh_fprintf( fp, "  Forms = (%s);\n", list_flag_string( ch->bstat(form), form_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(parts) != ch->pIndexData->parts ) )
    sh_fprintf( fp, "  Parts = (%s);\n", list_flag_string( ch->bstat(parts), part_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(affected_by) != ch->pIndexData->affected_by ) )
    sh_fprintf( fp, "  AfBy = (%s);\n", list_flag_string( ch->bstat(affected_by), affect_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(affected2_by) != ch->pIndexData->affected2_by ) )
    sh_fprintf( fp, "  AfBy2 = (%s);\n", list_flag_string( ch->bstat(affected2_by), affect2_flags, "'", ", " ));
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(classes) != ch->pIndexData->classes ) )
    sh_fprintf( fp, "  Classes = (%s);\n", list_flag_string( ch->bstat(classes), classes_flags, "'", ", " ) );
  sh_fprintf( fp, "  isWildMagic = %s;\n", ch->isWildMagic?"true":"false"); // SinaC 2003
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->level != ch->pIndexData->level ) )
    sh_fprintf( fp, "  Level = %d;\n", ch->level );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->exp > 0 ) )
    sh_fprintf( fp, "  Experience = %d;\n", ch->exp );
  if ( IS_NPC(ch) && saveact != ch->pIndexData->act )
    sh_fprintf( fp, "  Act = (%s);\n", list_flag_string( saveact, act_flags, "'", ", " ) );
  else if ( !IS_NPC(ch) && saveact != 0 )
    sh_fprintf( fp, "  Act = (%s);\n", list_flag_string( saveact, plr_flags, "'", ", " ) );
  if ( savecomm != 0 )
    sh_fprintf( fp, "  Comm = (%s);\n", list_flag_string( savecomm, comm_flags, "'", ", " ) );

  if ( ( IS_NPC(ch) && ch->position != ch->pIndexData->start_pos )
       || !IS_NPC(ch) )
    sh_fprintf( fp, "  Position = %s;\n",
		quotify( flag_string( position_flags,
				      ch->position == POS_FIGHTING ? POS_STANDING : ch->position ) ) );
  if ( IS_NPC(ch) ) {// Save NPC's str, int, wis, dex and con only if different from base
    bool changed = FALSE;
    for ( int i = 0; i < MAX_STATS; i++ )
      if ( ch->baseattr[ATTR_STR+i] != mob_base_stat( ATTR_STR+i, ch ) ) {
	changed = TRUE;
	break;
      }
    if ( changed )
      sh_fprintf( fp, "  Attr = ( %ld, %ld, %ld, %ld, %ld );\n",
		  ch->bstat(STR), ch->bstat(INT), ch->bstat(WIS), ch->bstat(DEX), ch->bstat(CON) );
  }
  else
    sh_fprintf( fp, "  Attr = ( %ld, %ld, %ld, %ld, %ld );\n",
		ch->bstat(STR), ch->bstat(INT), ch->bstat(WIS), ch->bstat(DEX), ch->bstat(CON) );

  if ( IS_NPC(ch) ) { // Save NPC's ac only if different from base
    bool changed = FALSE;
    for ( int i = 0; i < 4; i++ )
      if ( ch->bstat(ac0+i) != ch->pIndexData->ac[i] ) {
	changed = TRUE;
	break;
      }
    if ( changed )
      sh_fprintf( fp, "  ACs = ( %ld, %ld, %ld, %ld );\n",
		  ch->bstat(ac0),ch->bstat(ac1),ch->bstat(ac2),ch->bstat(ac3));
  }
  else
    sh_fprintf( fp, "  ACs = ( %ld, %ld, %ld, %ld );\n",
		ch->bstat(ac0),ch->bstat(ac1),ch->bstat(ac2),ch->bstat(ac3));

  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  buf[0] = '\0';
  if ( ch->hit==ch->bstat(max_hit) )   sprintf( buf2, " ( %d ), ", ch->hit );
  else                                 sprintf( buf2, " ( %d, %ld ), ", ch->hit, ch->bstat(max_hit) );
  strcat( buf, buf2 );
  if ( ch->mana==ch->bstat(max_mana) ) sprintf( buf2, " ( %d ), ", ch->mana );
  else                                 sprintf( buf2, " ( %d, %ld ), ", ch->mana, ch->bstat(max_mana) );
  strcat( buf, buf2 ); 
  if ( ch->move==ch->bstat(max_move) ) sprintf( buf2, " ( %d ), ", ch->move );
  else                                 sprintf( buf2, " ( %d, %ld ), ", ch->move, ch->bstat(max_move) );
  strcat( buf, buf2 );
  if ( ch->psp==ch->bstat(max_psp) )   sprintf( buf2, " ( %d )", ch->psp );
  else                                 sprintf( buf2, " ( %d, %ld )", ch->psp, ch->bstat(max_psp) );
  strcat( buf, buf2 );
  sh_fprintf( fp, "  HMVP = ( %s );\n", buf );
//  sh_fprintf( fp, "  HMVP = ( ( %d, %ld ), ( %d, %ld ), ( %d, %ld ), ( %d, %ld ) );\n",
//	      ch->hit, ch->bstat(max_hit), 
//	      ch->mana, ch->bstat(max_mana), 
//	      ch->move, ch->bstat(max_move), 
//	      ch->psp, ch->bstat(max_psp) );

  if ( ( !IS_NPC(ch)&& ch->bstat(hitroll) != 0 )
       || ( IS_NPC(ch) && ch->bstat(hitroll) != ch->pIndexData->hitroll) )
    sh_fprintf( fp, "  Hitroll = %ld;\n", ch->bstat(hitroll));
  if ( ( !IS_NPC(ch) && ch->bstat(damroll) != 0 )
       || ( IS_NPC(ch) && ch->bstat(hitroll) != ch->pIndexData->hitroll) )
    sh_fprintf( fp, "  Damroll = %ld;\n", ch->bstat(damroll));
  if ( ch->bstat(saving_throw) != 0 )
    sh_fprintf( fp, "  SavingThrow = %ld;\n", ch->bstat(saving_throw) );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(alignment) != ch->pIndexData->align.alignment ) )
    sh_fprintf( fp, "  Alignment = %ld;\n", ch->bstat(alignment) );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && ch->bstat(etho) != ch->pIndexData->align.etho ) )
    sh_fprintf( fp, "  Etho = %s;\n", quotify(flag_string(etho_flags,ch->bstat(etho))) );
  if (ch->gold > 0)
    sh_fprintf( fp, "  Gold = %ld;\n", ch->gold );
  if (ch->silver > 0)
    sh_fprintf( fp, "  Silver = %ld;\n", ch->silver );

  if (ch->daze > 0)
    sh_fprintf( fp, "  Daze = %ld;\n", ch->daze );
  if (ch->stunned > 0)
    sh_fprintf( fp, "  Stunned = %ld;\n", ch->stunned );
  if (ch->timer > 0)
    sh_fprintf( fp, "  Timer = %ld;\n", ch->timer );

  sh_fprintf( fp, "  Room = %d;\n", // SinaC 2003, even mob's room is saved (redundant if save_world_state)
	      ( ch->in_room == get_room_index( ROOM_VNUM_LIMBO )
		&& ch->was_in_room != NULL )
	      ? ch->was_in_room->vnum
	      //: ch->in_room == NULL ? ROOM_VNUM_TEMPLE : ch->in_room->vnum );
	      : ch->in_room == NULL ? get_recall_room(ch)->vnum : ch->in_room->vnum );

  // Specific to player
  if ( !IS_NPC(ch) ) {
    if ( ch->pcdata->remort_count > 0 )
      sh_fprintf( fp, "  RemortCount = %d;\n", ch->pcdata->remort_count );

    if ( ch->pcdata->betted_on != NULL )
      if ( IS_NPC( ch->pcdata->betted_on ) ) // if betted_on is NPC: store ptr address -> SHOULD NEVER HAPPEN
	sh_fprintf( fp, "  BettedOn = %s;\n", (int)ch->pcdata->betted_on );
      else // if betted_on is PC: store player name
	sh_fprintf( fp, "  BettedOn = %s;\n", quotify(NAME(ch->pcdata->betted_on)) );
    if ( ch->pcdata->challenged != NULL )
      if ( IS_NPC( ch->pcdata->challenged ) ) // if challenged is NPC: store ptr address -> SHOULD NEVER HAPPEN
	sh_fprintf( fp, "  Challenged = %s;\n", (int)ch->pcdata->challenged );
      else // if challenged is PC: store player name
	sh_fprintf( fp, "  Challenged = %s;\n", quotify(NAME(ch->pcdata->challenged)) );
    if ( ch->pcdata->bet_amt > 0 )
      sh_fprintf( fp, "  BetAmt = %d;\n", ch->pcdata->bet_amt );

    if ( ch->prompt != NULL )
      sh_fprintf( fp, "  Prompt = %s;\n", quotify(duplicate(ch->prompt,'%')));
    if ( ch->clan ) {
      sh_fprintf( fp, "  Clan = %s;\n", quotify(get_clan_table(ch->clan)->name));
      sh_fprintf( fp, "  ClanStatus = %s;\n", quotify(short_lookup_clan_status(ch->clan_status)));
    }
    if (ch->petition)
      sh_fprintf( fp, "  Petition = %s;\n", quotify(get_clan_table(ch->petition)->name));
    if (ch->trust != 0)
      sh_fprintf( fp, "  Trust = %d;\n", ch->trust );
    sh_fprintf( fp, "  Played = %d;\n",
		ch->played + (int) (current_time - ch->logon) );
    sh_fprintf( fp, "  Scroll = %d;\n", ch->lines );
    if (ch->wiznet)
      sh_fprintf( fp, "  Wiznet = %s;\n", quotify(print_flags(ch->wiznet)) );
    if (ch->invis_level)
      sh_fprintf( fp, "  Invis = %d;\n", ch->invis_level );
    if (ch->incog_level)
      sh_fprintf( fp,"   Inco = %d;\n", ch->incog_level );
    if (ch->practice != 0)
      sh_fprintf( fp, "  Prac = %d;\n", ch->practice );
    if (ch->train != 0)
      sh_fprintf( fp, "  Train = %d;\n", ch->train );
    if (ch->wimpy !=0 )
      sh_fprintf( fp, "  Wimpy = %d;\n", ch->wimpy );
    sh_fprintf( fp, "  LastLevel = %d;\n", ch->pcdata->last_level );
    sh_fprintf( fp, "  Password = %s;\n", quotify(ch->pcdata->pwd) );
    if ( ch->desc != NULL && ch->desc->host != NULL )
      sh_fprintf( fp, "  Address = %s;\n", quotify(ch->desc->host) );
    if ( ch->desc != NULL )
      sh_fprintf( fp, "  LineMode = %d;\n", ch->desc->lineMode );
    sh_fprintf( fp, "  Language = %s;\n", quotify(language_name(ch->pcdata->language)) );
    sh_fprintf( fp, "  Beacons = ( %d, %d, %d );\n", 
		ch->pcdata->beacons[0], ch->pcdata->beacons[1], ch->pcdata->beacons[2]);
    sh_fprintf( fp, "  God = %s;\n", quotify(gods_table[ch->pcdata->god].name) );
    sh_fprintf( fp, "  NameAccepted = %s;\n", ch->pcdata->name_accepted?"true":"false" );
    if ( ch->pcdata->hometown < 0 )
      sh_fprintf( fp, "  Hometown = 'unknown';\n" );
    else
      sh_fprintf( fp, "  Hometown = %s;\n", quotify(hometown_table[ch->pcdata->hometown].name) );
    if (ch->pcdata->stance[0] != '\0' )
      sh_fprintf( fp, "  Stance = %s;\n", quotify(ch->pcdata->stance));

    sh_fprintf ( fp, "  Boards = (");
    for (int i = 0; i < MAX_BOARD; i++) {
      sh_fprintf( fp, "( %s, %ld)", quotify(boards[i].short_name), ch->pcdata->last_note[i]);
      if ( i < MAX_BOARD-1 ) {
	sh_fprintf( fp, ",\n");
	sh_fprintf( fp, "            ");
      }
    }
    sh_fprintf( fp, ");\n");

    sh_fprintf( fp, "  Title = %s;\n", quotify(trim(ch->pcdata->title)));

    sh_fprintf( fp, "  Condition = ( %d, %d, %d, %d );\n",
	     ch->pcdata->condition[0], ch->pcdata->condition[1],
	     ch->pcdata->condition[2], ch->pcdata->condition[3] );

    for (int i = 0; i < MAX_ALIAS; i++) {
      if (ch->pcdata->alias[i] == NULL
	  ||  ch->pcdata->alias_sub[i] == NULL)
	break;

      sh_fprintf(fp,"  Alias = ( %s, %s );\n",
		 quotify(ch->pcdata->alias[i]),
		 quotify(ch->pcdata->alias_sub[i]));
    }

    DISABLED_CMD_DATA *p = ch->pcdata->disabled_cmd;
    while ( p != NULL ) {
      sh_fprintf(fp, "  DisabledCmd = ( %s, %s );\n", 
	      quotify(p->cmd->name), quotify(p->disabled_by) );
      p = p->next;
    }

    sh_fprintf( fp, "  Trivia = %d;\n", ch->pcdata->trivia );
    //    if (ch->pcdata->questpoints != 0)
    //      sh_fprintf( fp, "  QuestPnts = %d;\n", ch->pcdata->questpoints );
    //    if (ch->pcdata->nextquest != 0)
    //      sh_fprintf( fp, "  QuestNext = %d;\n", ch->pcdata->nextquest );
    //    if (ch->pcdata->countdown != 0)
    //      sh_fprintf( fp, "  QuestCount = %d;\n", ch->pcdata->countdown );
    //    if (ch->pcdata->questobj != 0)
    //      sh_fprintf( fp, "  QuestObj = %d;\n", ch->pcdata->questobj );
    //    if (ch->pcdata->questmob != 0)
    //      sh_fprintf( fp, "  QuestMob = %d;\n", ch->pcdata->questmob );
    //    if (ch->pcdata->questgiver != NULL )
    //      sh_fprintf( fp, "  QuestGiver = %d;\n", ch->pcdata->questgiver->pIndexData->vnum );
    //    if (ch->pcdata->questobj != 0)
    //      sh_fprintf( fp, "  QuestObjLoc = %d;\n", ch->pcdata->questobjloc );

    for ( int i = 0; i < factions_count; i++ )
      sh_fprintf( fp, "  Faction = ( %s, %d );\n", 
		  quotify(faction_table[i].name), ch->pcdata->base_faction_friendliness[i] );

    sh_fprintf( fp, "  Pnts = %d;\n", ch->pcdata->points );
    for ( int sn = 0; sn < MAX_ABILITY; sn++ )
      if ( ability_table[sn].name != NULL 
	   && ch->pcdata->ability_info[sn].learned > 0 )
	sh_fprintf( fp, "  Ability = ( %d, %d, %d, %s );\n",
		    ch->pcdata->ability_info[sn].learned, 
		    ch->pcdata->ability_info[sn].casting_level,
		    ch->pcdata->ability_info[sn].level,
		    quotify(ability_table[sn].name) );

    for ( int gn = 0; gn < MAX_GROUP; gn++ )
      if ( group_table[gn].name != NULL && ch->pcdata->group_known[gn])
	sh_fprintf( fp, "  Group = %s;\n", quotify(group_table[gn].name));

    // Imm related infos
    sh_fprintf( fp, "  Gset = %d;\n", ch->pcdata->goto_default );
    sh_fprintf( fp, "  Security = %d;\n", ch->pcdata->security	);
    if (ch->pcdata->bamfin[0] != '\0')
      sh_fprintf( fp, "  Bin = %s;\n", quotify(ch->pcdata->bamfin));
    if (ch->pcdata->bamfout[0] != '\0')
      sh_fprintf( fp, "  Bout = %s;\n",	quotify(ch->pcdata->bamfout));
    if (ch->pcdata->immtitle[0] != '\0' )
      sh_fprintf( fp, "  Immtitle = %s;\n", quotify(ch->pcdata->immtitle));
  }
  // Specific to mob
  else {
    if ( ch->clazz != ch->pIndexData->program
	 && ch->clazz != default_mob_class ) // SinaC 2003
      sh_fprintf(fp,"  Clazz = %s;\n", quotify( ch->clazz->name ) );
    if ( ch->bstat(DICE_NUMBER) != ch->pIndexData->damage[DICE_NUMBER] )
      sh_fprintf(fp, "  Dnumber = %ld;\n", ch->bstat(DICE_NUMBER) );
    if ( ch->bstat(DICE_TYPE) != ch->pIndexData->damage[DICE_TYPE] )
      sh_fprintf(fp, "  Dtype = %ld;\n", ch->bstat(DICE_TYPE));
  }

  // Affects, only of PC or  NPC and OPTIMIZING_BIT_MODIFIED_AFFECT set
  if ( IS_NPC(ch) && IS_SET( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT ) )
    sh_fprintf( fp, "  Enchanted = %s;\n", IS_SET( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT )?"true":"false" );
  if ( !IS_NPC(ch)
       || ( IS_NPC(ch) && IS_SET( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT ) ) )
    //for ( AFFECT_DATA *paf = ch->affected; paf != NULL; paf = paf->next )
    //  new_save_affects( paf, fp );
    new_save_affects( ch->affected, fp );
     
  // Extra fields
  if ( ch->ex_fields != NULL )
    new_save_extra_fields( ch, fp );

  // Save pet
  if ( ch->pet != NULL )
    new_save_char( ch->pet, fp );

  // Save inventory/equipement
  if ( ch->carrying != NULL )
    //for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
    new_save_obj_list( ch, ch->carrying, fp );

  sh_fprintf( fp, "}\n");

  data_depth--;
}
void new_save_pFile( CHAR_DATA *ch, bool leaving ) {
  data_depth = 0;

  char strsave[MAX_INPUT_LENGTH];
  FILE *fp;

  if ( IS_NPC(ch) )
    return;

  fix_clan_status_leave(ch); // Oxtal

  if ( ch->desc != NULL && ch->desc->original != NULL )
    ch = ch->desc->original;
  
  if ( leaving ) {
    OLC_EDIT_DONE( ch ); // release lock bit, SinaC 2003
    // Added by SinaC 2003: call onQuitting method
    MOBPROG( ch, NULL, "onQuitting" );
  }

#if defined(unix)
  // create god log
  if (IS_IMMORTAL(ch)){
    fclose(fpReserve);
    sprintf(strsave, "%s%s", GOD_DIR, capitalize(ch->name) );
    if ((fp = fopen(strsave,"w")) == NULL){
      bug("Save_char_obj: fopen");
      perror(strsave);
    }
      
    sh_fprintf(fp,"Lev %2d Trust %2d  %s%s\n",
	    ch->level, get_trust(ch), ch->name, trim( ch->pcdata->title) );
    fclose( fp );
    fpReserve = fopen( NULL_FILE, "r" );
  }
#endif

  fclose( fpReserve );
  sprintf( strsave, "%s%s.new", PLAYER_DIR, capitalize( ch->name ) );
  if ( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL ){
    bug( "new_save_pFile: fopen" );
    perror( strsave );
  }
  else {
    log_stringf("Saving player %s.", ch->name );
    new_save_char( ch, fp );
  }

  fclose( fp );
  rename(TEMP_FILE,strsave);
  fpReserve = fopen( NULL_FILE, "r" );
}

static CHAR_DATA *player_loading;
bool new_load_pFile( DESCRIPTOR_DATA *d, char *name ) {
  char buf[MAX_STRING_LENGTH];
  char strsave[MAX_STRING_LENGTH];
  int lastlogoff = current_time;
  FILE *fp;

  fclose( fpReserve );
  // First, we try to find a new compressed format pFile
#if defined(unix)
  // decompress if .gz file exists
  sprintf( strsave, "%s%s%s%s", PLAYER_DIR, capitalize(name), ".new", ".gz");
  if ( ( fp = fopen( strsave, "r" ) ) != NULL ){
    fclose(fp);
    sprintf(buf,"gzip -dfq %s",strsave);
    system(buf);
  }
#endif

  // Secondly, we try to find a new format pFile, if not found : use old format
  sprintf( strsave, "%s%s%s", PLAYER_DIR, capitalize( name ), ".new" );
  if ( ( fp = fopen( strsave, "r" ) ) == NULL ) {
    fpReserve = fopen( NULL_FILE, "r" );
    return load_char_obj( d, name ); // call old load method
  }

  // New format found, read it
  create_new_char( d, name );
  player_loading = d->character;

  parse_datas( fp );

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );

  // SinaC 2003
  // When reading a pFile, we store ENTITY extra fields as list but we don't
  //  extract informations from this list, because it can refer
  //  to player itself (which is not yet in char_list when loading it)
  //  or to other items in inventory (which may not yet been read)
  // We do this only if we are not rebooting with a world state saved
  //  if we are, use_delayed_parse_extra_fields will be done in copyover_recover_state
  if ( !fState ) {
    use_delayed_parse_extra_fields( player_loading );
    if ( player_loading->pet != NULL )
      use_delayed_parse_extra_fields( player_loading->pet );
  }

  fix_char( player_loading );

  recompute(player_loading); // Added along copyover -- Oxtal
  
  return TRUE;
}

bool getValueExtraFieldsOK;
int getValueExtraFieldsType;
Value getValueExtraFields( ENTITY_DATA *ent, const Value &v ) {
  switch( v.typ ) {
  case SVT_LIST: { // list or entity (special format)
    ValueList *list = v.asList();
    bool done = FALSE;
    if ( list->size >= 2 ) {
      const char *s = list->elems[0].asStr();
      if ( !strcmp( s, TAG_ROOM_ENTITY ) && list->size == 2 ) { // room entity doesn't have any special case
	done = TRUE;
	int vnum = list->elems[1].asInt();
	ROOM_INDEX_DATA *room = get_room_index(vnum);
	if ( room == NULL ) {
	  log_stringf("Room [%d] in extra fields doesn't exist anymore", vnum );
	  getValueExtraFieldsOK = FALSE;
	  getValueExtraFieldsType = SVT_ENTITY;
	  return Value((long)0);
	}
	return Value(room);
      }
      else if ( !strcmp( s, TAG_OBJ_ENTITY ) ) { // obj entity has container case
	const char *t = list->elems[1].asStr();
	if ( ent->kind == OBJ_ENTITY  // if to parse entity is an obj, field is maybe the container
	     && list->size == 2
	     && !strcmp( t, TAG_OBJ_CONTAINER ) )
	  return ((OBJ_DATA*)ent)->in_obj;
	if ( list->size == 3 ) { // else
	  int vnum = list->elems[1].asInt();
	  OBJ_INDEX_DATA *pObj = get_obj_index( vnum );
	  if ( pObj == NULL ) {
	    log_stringf("Obj index [%d] doesn't exist anymore.", vnum );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  Value v = getValueExtraFields( ent, list->elems[2].asList() );
	  if ( getValueExtraFieldsOK == FALSE || v.typ != SVT_ENTITY ) {
	    bug("Invalid extra fields.");
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  ENTITY_DATA *e = v.asEntity();
	  switch ( e->kind ) {
	  case CHAR_ENTITY: {
	    CHAR_DATA *ch = (CHAR_DATA*) e;
	    for ( OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content )
	      if ( obj->pIndexData == pObj ) // pick first one
		return obj;
	    log_stringf("No obj vnum [%d] found in char [%s].", vnum, NAME(ch) );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  case OBJ_ENTITY: {
	    OBJ_DATA *container = (OBJ_DATA*) e;
	    for ( OBJ_DATA *obj = container->contains; obj; obj = obj->next_content )
	      if ( obj->pIndexData == pObj ) // pick first one
		return obj;
	    log_stringf("No obj vnum [%d] found in obj [%s].", vnum, container->short_descr );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  case ROOM_ENTITY: {
	    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA*) e;
	    for ( OBJ_DATA *obj = room->contents; obj; obj = obj->next_content )
	      if ( obj->pIndexData == pObj ) // pick first one
		return obj;
	    log_stringf("No obj vnum [%d] found in room [%d].", vnum, room->vnum );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  default:	
	    bug("Invalid entity type in extra fields.");
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  done = TRUE;
	}
      }
      else if ( !strcmp( s, TAG_CHAR_ENTITY ) ) { // char entity has master and carrier cases
	const char *t = list->elems[1].asStr();
	if ( ent->kind == OBJ_ENTITY  // if to parse entity is an obj, field is maybe the carrier
	     && list->size == 2
	     && !strcmp( t, TAG_CHAR_CARRIER ) )
	  return ((OBJ_DATA *)ent)->carried_by;
	if ( ent->kind == CHAR_ENTITY // if to parse entity is a char, field is maybe the master
	     && list->size == 2
	     && !strcmp( t, TAG_CHAR_MASTER ) )
	  return ((CHAR_DATA *)ent)->master;
	if ( list->size == 2 ) { // PC: pc name
	  const char *s = list->elems[1].asStr();
	  CHAR_DATA *ch = get_pc_world(s);
	  if ( ch == NULL || !ch->valid ) {
	    log_stringf("Player [%s] is not yet/anymore connected.", s );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  return Value(ch);
	}
	else if ( list->size == 3 ) { // NPC: mob vnum, room vnum
	  int mobVnum = list->elems[1].asInt();
	  int roomVnum = list->elems[2].asInt();
	  MOB_INDEX_DATA *pMob = get_mob_index( mobVnum );
	  ROOM_INDEX_DATA *pRoom = get_room_index( roomVnum );
	  if ( pMob == NULL ) {
	    log_stringf("Mob index [%d] doesn't exist anymore.", mobVnum );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  if ( pMob->count == 1 ) { // unique mob
	    for ( CHAR_DATA *rch = char_list; rch; rch = rch->next )
	      if ( rch->pIndexData == pMob )
		return Value(rch);
	  }
	  if ( pRoom == NULL ) {
	    log_stringf("Room [%d] doesn't exist anymore.", roomVnum );
	    getValueExtraFieldsOK = FALSE;
	    getValueExtraFieldsType = SVT_ENTITY;
	    return Value((long)0);
	  }
	  for ( CHAR_DATA *rch = pRoom->people; rch; rch = rch->next_in_room ) {
	    if ( IS_NPC(rch)
		 && rch->pIndexData == pMob ) { // pick first one in the room
	      return Value(rch);
	    }
	  }
	  log_stringf("No mob vnum [%d] found in room [%d].", mobVnum, roomVnum );
	  getValueExtraFieldsOK = FALSE;
	  getValueExtraFieldsType = SVT_ENTITY;
	  return Value((long)0);
	}
	else {
	  bug("Invalid extra field." );
	  getValueExtraFieldsOK = FALSE;
	  getValueExtraFieldsType = SVT_ENTITY;
	  return Value((long)0);
	}
	done = TRUE;
      }
    }
    if ( !done ) { // classic list
      ValueList *res = ValueList::newList(list->size);
      for ( int i = 0; i < list->size; i++ ) {
	res->elems[i] = getValueExtraFields( ent, list->elems[i]);
	//if ( getValueExtraFieldsOK == FALSE )
	//  return Value((long)0);
	if ( getValueExtraFieldsOK == FALSE ) { // test extra field type before nuking the list
	  switch(getValueExtraFieldsType) {
	  case SVT_VOID: return Value((long)0); break; // BIG PROBLEM
	  case SVT_ENTITY: // element was an entity, inserting NULL and continue
	    res->elems[i] = Value((ENTITY_DATA*)NULL);
	    getValueExtraFieldsOK = TRUE; // continue...
	    break;
	  default: // should never happen
	    bug("Error in extra field: invalid type [%d] in list error fixing", getValueExtraFieldsType );
	    return Value((long)0); break;
	    break;
	  }
	}
      }

      //      list->explicit_free();
      return res;
    }
    break;
  }
  case SVT_INT: return v.asInt(); break;
  case SVT_STR: {
    const char *s = v.asStr();
    if ( !strcmp( s, TAG_NULL_ENTITY ) ) // SinaC 2003: null entity in list
      return (ENTITY_DATA*)NULL;
    else 
      return s;
    break;
  }
  case SVT_ENTITY: return v.asEntity(); break;
  default: 
    bug("Error in extra field: invalid type %d [%s]", v.typ, v.asStr() ); 
    getValueExtraFieldsOK = FALSE;
    getValueExtraFieldsType = SVT_VOID; // INVALID extra fields
    return Value((long)0);
    break;
  }
  return Value((long)0);
}
// Some extra fields as entity are delayed
// We use informations contained in the extra fields, only 
//  when we are sure everything is loaded for example
// return FALSE if extra fields is wrong  but return value is not used anymore
bool delayed_parse_extra_field( ENTITY_DATA *ent, FIELD_DATA *f ) {
  if ( f->var.typ != SVT_LIST )
    return TRUE;
  Value v;
  getValueExtraFieldsOK = TRUE;
  v = getValueExtraFields( ent, f->var.asList() );
  //    printf("DELAYED EXTRA FIELD: %s [%s]\n\r", f->name, f->var.asStr() );
  if ( getValueExtraFieldsOK ) {
    // Don't use setValue because f already exists -> trying to set constant
    f->var.typ = v.typ;
    switch( v.typ ) {
    case SVT_INT: f->var.val.i = v.val.i; break;
    case SVT_STR: f->var.val.s = str_dup( v.val.s ); break;
    case SVT_ENTITY: f->var.val.e = v.val.e; break;
    case SVT_FCT: f->var.val.f = v.val.f; break;
    case SVT_VAR: f->var.val.v = v.val.v; break;
    case SVT_LIST: f->var.val.l = v.val.l; break;
    default: bug("Unknown Value typ: %d", v.typ ); break;
    }
    //        printf(" -> %s\n\r", f->var.asStr() );
    return TRUE;
  }
  else {
    log_stringf("getValueExtraFieldsOK is FALSE for extra field: %s", f->name );
    // If the extra field was incorrect or referred value doesn't exist anymore
    //  we just store NULL, because only entity AND list of entities can provoke problems
    f->var.typ = SVT_ENTITY;
    f->var.val.e = (ENTITY_DATA *)NULL;
    return FALSE;
  }
  //  printf("Problem\n\r");
  return FALSE;
}
void delayed_parse_extra_fields( ENTITY_DATA *ent ) {
  FIELD_DATA *f = ent->ex_fields;
  FIELD_DATA *prev = f;
  while ( f != NULL ) { // extra fields
    //    if ( !delayed_parse_extra_field( ent, f ) ) // if parse not correct, remove extra fields from list
    //      if ( f == prev ) { // if head of list, change head
    //	ent->ex_fields = f->next;
    //	prev = ent->ex_fields;
    //	f = ent->ex_fields;
    //	if ( ent->ex_fields == NULL ) // if only one field and an incorrect one
    //	  return;
    //      }
    //      else // else we just by-pass f
    //	prev->next = f->next;
    //    else

    // we can't remove extra fields, because some scripts
    //  may use those extra fields, such as player:questobj  and onLoad is called before this loop
    delayed_parse_extra_field( ent, f );
    prev = f;
    f = f->next;
  }
}
void use_delayed_parse_extra_fields( ENTITY_DATA *ent ) {
  switch ( ent->kind ) {
  case CHAR_ENTITY: {  // extra fields + inventory extra fields
    CHAR_DATA *ch = (CHAR_DATA*)ent;
    delayed_parse_extra_fields( ch ); // char extra fields
    for ( OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content ) // inventory extra fields
      use_delayed_parse_extra_fields( obj );
    break;
  }
  case OBJ_ENTITY: {  // extra fields + if container: contained extra fields
    OBJ_DATA *obj = (OBJ_DATA*)ent;
    delayed_parse_extra_fields( obj ); // obj extra fields
    for ( OBJ_DATA *objC = obj->contains; objC; objC = objC->next_content ) // contained extra fields
      use_delayed_parse_extra_fields( objC );
    break;
  }
  case ROOM_ENTITY: {  // extra fields
    delayed_parse_extra_fields( ent ); // room extra fields
    break;
  }
  default: bug("Invalid extra entity type %d for entity %s.", ent->kind, ent->name );
  }
}
void parse_extra_fields( DATAData *ex, ENTITY_DATA *ent ) {
  ValueList *list = ex->value->eval().asList();
  
  if ( list->size != 4 ) {
    bug("Wrong number of elements for extra field: <name> <type> <lifetime> <value>");
    return;
  }
  const char *fName = list->elems[0].asStr();
  const char *fType = list->elems[1].asStr();
  const int lifetime = list->elems[2].asInt(); // not yet used
  const int typ = script_type_lookup( fType );

  if ( DATA_VERBOSE > 2 ) {
    log_stringf("ex: %s [%s]  value: %s", fName, fType, list->elems[3].asStr()); fflush(stderr);
  }

  switch( typ ) {
  default: bug("Invalid extra field type %s", fType ); break;
  case SVT_INT: {
    Value var;
    var = add_extra_field( ent, fName );
    var.setValue(list->elems[3].asInt());
    break;
  }
  case SVT_STR: {
    Value var;
    var = add_extra_field( ent, fName );
    var.setValue(list->elems[3].asStr());
    break;
  }
  case SVT_ENTITY: {
    //    Value val;
    //    getValueExtraFieldsOK = TRUE;
    //    if ( !strcmp( list->elems[3].asStr(), "<NULL>" ) )
    //      val = Value((ENTITY_DATA*)NULL);
    //    else
    //       val = getValueExtraFields( list->elems[3].asList() );
    //    if ( getValueExtraFieldsOK ) {
    //      Value var;
    //      var = add_extra_field( ent, eName );
    //      var.setValue(val);
    // We delayed extra fields parsing because we are not sure that referred entities are already loaded
    Value var;
    var = add_extra_field( ent, fName );
    if ( !strcmp( list->elems[3].asStr(), TAG_NULL_ENTITY ) )
      var.setValue((ENTITY_DATA*)NULL);
    else {
      var.setValue(list->elems[3].asList());
      //            printf("Delaying %s...\n\r", fName );
    }
    break;
  }
  case SVT_LIST: {
    //    getValueExtraFieldsOK = TRUE;
    //    Value val = getValueExtraFields( list->elems[3].asList() );
    //    if ( getValueExtraFieldsOK != FALSE ) {
    //      Value var;
    //      var = add_extra_field( ent, eName );
    //      var.setValue(val);
    // We delayed extra fields parsing because we are not sure that referred entities are already loaded
    Value var;
    var = add_extra_field( ent, fName );
    var.setValue(list->elems[3].asList());
    break;
  }
  }
  //  list->explicit_free();
}
void parse_obj( DATAData *objD, OBJ_DATA *obj, bool new_format ) {
  bool make_new = FALSE;
  // vnum has already been extracted

  OBJ_INDEX_DATA *objIndex = obj->pIndexData;
  if ( objIndex == NULL ) {
    bug("Obj %s doesn't have pIndexData", obj->name );
    return;
  }

  // fields
  for ( int fieldCount = 0; fieldCount < objD->fields_count; fieldCount++ ) {
    DATAData *field = objD->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Affect: {
      AFFECT_DATA *paf = new_affect();
      if ( parse_affect( field, paf ) ) { // add affect in list only if no error
	paf->next       = obj->affected;
	obj->affected   = paf;
      }
      break;
    }
    case TAG_Condition: obj->condition = field->value->eval().asInt(); break;
    case TAG_Cost: obj->cost = field->value->eval().asInt(); break;
    case TAG_WearLoc: {
      const char *s = field->value->eval().asStr();
      if ( ( obj->wear_loc = flag_value( wear_loc_flags, s ) ) == NO_FLAG ) {
	bug("Invalid wear loc flags %s for obj %s, assuming NONE", s, obj->name );
	obj->wear_loc = WEAR_NONE;
      }
      break;
    }
    case TAG_Desc: obj->description = field->value->eval().asStr(); break;
    case TAG_Enchanted: obj->enchanted = field->value->eval().asInt(); break;
    case TAG_ExtraFlags:
      if ( ( obj->base_extra = list_flag_value_complete( field->value, extra_flags ) ) == NO_FLAG ) {
	bug("Invalid extra flags for obj %s, assuming %s", obj->name, 
	    flag_string( extra_flags, objIndex->extra_flags ) );
	//obj->base_extra = 0;
	obj->base_extra = objIndex->extra_flags;
      }
      break;
    case TAG_ExtraDescr: {
      ValueList *couple = field->value->eval().asList();
      if ( couple->size != 2 ) {
	bug("Wrong number of elements in extra description <keyword><description>");
	break;
      }
      EXTRA_DESCR_DATA *ed = new_extra_descr();
      ed->keyword		= str_dup(couple->elems[0].asStr());
      ed->description		= str_dup(couple->elems[1].asStr());
      ed->next		= obj->extra_descr;
      obj->extra_descr	= ed;

      couple->explicit_free();
      break;
    }
    case TAG_ItemType: {
      const char *s = field->value->eval().asStr();
      if ( ( obj->item_type = flag_value( type_flags, s ) ) == NO_FLAG ) {
	bug("Invalid item type %s for obj %s, assuming %s", s, obj->name, 
	    flag_string( type_flags, objIndex->item_type ) );
	//obj->item_type = ITEM_TRASH;
	obj->item_type = objIndex->item_type;
      }
      break;
    }
    case TAG_Level: obj->level = field->value->eval().asInt(); break;
    case TAG_Material: {
      const char *s = field->value->eval().asStr();
      if ( ( obj->material = flag_value( material_flags, s ) ) == NO_FLAG ) {
	bug("Invalid material %s for obj %s, assuming %s", s, obj->name,
	    flag_string( material_flags, objIndex->material ) );
	obj->material = objIndex->material;
      }
      break;
    }
    case TAG_Name: obj->name = str_dup( field->value->eval().asStr() ); break;
    case TAG_Owner: obj->owner = str_dup( field->value->eval().asStr() ); break;
    case TAG_Oldstyle: {
      if ( obj->pIndexData != NULL && obj->pIndexData->new_format )
	make_new = TRUE;
      break;
    }
  // SinaC 2003, not needed: we can safely use pIndexData because restriction/ability_upgrade
  //  cannot be dynamically modified during play
//    case TAG_Upgrade: { // SinaC 2003
//      ValueList *list = field->value->eval().asList();
//      if ( list->size != 2 ) {
//	bug("Wrong number of elements in upgrade <ability name><value>");
//	break;
//      }
//      const char *s = list->elems[0].asStr();
//      ABILITY_UPGRADE *upgr = new_ability_upgrade();
//      if ( ( upgr->sn = ability_lookup( s ) ) <= 0 ) {
//	bug("Invalid ability name [%s] in upgrade for obj [%s]", s, obj->name );
//	break;
//      }
//      upgr->value = list->elems[1].asInt();
//      upgr->next = obj->upgrade;
//      obj->upgrade = upgr;
//
//      list->explicit_free();
//      break;
//    }
//    case TAG_Restriction: {
//      ValueList *list = field->value->eval().asList();
//      if ( list->size != 4 ) {
//	bug("Wrong number of elements in restriction <kind><type name><value><not r?>");
//	break;
//      }
//      int kind = list->elems[0].asInt();
//      RESTR_DATA *restr = new_restriction();
//      if ( kind == 0 ) {
//	restr->type = flag_value( restr_flags, list->elems[1].asStr() );
//	if ( restr->type == NO_FLAG ) {
//	  bug("Invalid restriction type");
//	  break;
//	}
//	restr->ability_r = FALSE;
//	restr->sn = -1;
//	restr->value = list->elems[2].asInt();
//	restr->not_r = list->elems[3].asInt();
//      }
//      else if ( kind == 1 ) {
//	restr->type = RESTR_ABILITY;
//	restr->ability_r = TRUE;
//	const char *s = list->elems[1].asStr();
//	restr->sn = ability_lookup( s );
//	if ( restr->sn < 0 ) {
//	  bug("Invalid ability restriction %s", s );
//	  break;
//	}
//	restr->value = list->elems[2].asInt();
//	restr->not_r = list->elems[3].asInt();
//      }
//      else {
//	bug("Invalid restriction kind");
//	break;
//      }
//      restr->next                = obj->restriction;
//      obj->restriction     = restr;
//
//      list->explicit_free();
//      break;
//    }
    case TAG_ShD: obj->short_descr = field->value->eval().asStr(); break;
    case TAG_Size: {
      const char *s = field->value->eval().asStr();
      if ( ( obj->size = flag_value( size_flags, s ) ) == NO_FLAG ) {
	bug("Invalid size %s for obj %s, assuming %s", s, obj->name,
	    flag_string( size_flags, objIndex->size ) );
	//obj->size = SIZE_NOSIZE;
	obj->size = objIndex->size;
      }
      break;
    }
    case TAG_Spell: {
      ValueList *couple = field->value->eval().asList();
      if ( couple->size != 2 ) {
	bug("Wrong number of elements in spell <value id><spell name>");
	break;
      }
      int iValue = couple->elems[0].asInt();
      if ( iValue < 0 || iValue > 4 ) {
	bug("Invalid value id %d for obj %s", iValue, obj->name );
	break;
      }
      const char *s = couple->elems[1].asStr();
      const int sn = ability_lookup( s );
      if ( sn < 0 ) {
	bug("Invalid ability name %s for obj %s", s, obj->name );
	break;
      }
      obj->baseval[iValue] = sn;
      obj->value[iValue] = sn;

      couple->explicit_free();
      break;
    }
    case TAG_Timer: obj->timer = field->value->eval().asInt(); break;
    case TAG_Values: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 5 ) {
	bug("Invalid number of values %d for obj %s", list->size, obj->name );
	break;
      }
      for ( int i = 0; i < 5; i++ )
	obj->baseval[i] = list->elems[i].asInt();

      list->explicit_free();
      break;
    }
    case TAG_WearFlags:
      if ( ( obj->wear_flags = list_flag_value_complete( field->value, wear_flags ) ) == NO_FLAG ) {
	bug("Invalid wear flags for obj %s, assuming %s", obj->name,
	    flag_string( wear_flags, objIndex->wear_flags ) );
	//obj->wear_flags = ITEM_TAKE;
	obj->wear_flags = objIndex->wear_flags;
      }
      break;
    case TAG_Weight: obj->weight = field->value->eval().asInt(); break;
    case TAG_ExField: parse_extra_fields( field, (ENTITY_DATA*)obj ); break;
    case TAG_Obj: {
      OBJ_DATA *obj2 = NULL;
      bool new_format2 = FALSE;
      int vnum = field->value->eval().asInt(); // get obj vnum
      if (  get_obj_index( vnum ) == NULL ) {
	bug( "new_load_pFile: bad obj vnum %d.", vnum );
	//obj2 = new_obj();
	//obj2->name		= str_dup( "" );
	//obj2->short_descr	= str_dup( "" );
	//obj2->description	= str_dup( "" );
      }
      else {
	obj2 = create_object(get_obj_index(vnum),-1);
	new_format2 = TRUE;
	parse_obj( field, obj2, new_format2 );
	if ( obj2 != NULL )
	  obj_to_obj( obj2, obj );
      }
      break;
    }
      // SinaC 2003, store dynamic class if different from static one
    case TAG_Clazz: {
      const char *s = field->value->eval().asStr();
      CLASS_DATA *cl = silent_hash_get_prog(s);
      if ( cl == NULL ) {
	bug("parse_obj: obj [%s] dynamic clazz [%s] cannot be found", obj->name, s );
	break;
      }
      if ( get_root_class(cl) != default_obj_class ) {
	bug("parse_obj: obj [%s] dynamic clazz [%s] is not anymore an obj class", obj->name, s );
	break;
      }
      obj->clazz = cl;
      break;
    }
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }

  if ( obj->pIndexData == NULL ) {
    bug( "parse_obj: incomplete object");
    free_obj(obj);
    return;
  }
  if (!new_format){
    obj->next	= object_list;
    object_list	= obj;
    obj->pIndexData->count++;
  }
  if (make_new){
    int wear;
    wear = obj->wear_loc;
    extract_obj(obj);
    obj = create_object(obj->pIndexData,0);
    obj->wear_loc = wear;
  }
  recompobj(obj);
}
void parse_char( DATAData *chD, CHAR_DATA *ch ) {
  int countAlias = 0;
  int lastlogoff = current_time;
  PTR_LIST *current_ptr = NULL;

  // get player name or pet vnum
  if ( !IS_NPC(ch) ) // PC
    ch->name = str_dup(chD->value->eval().asStr());
  else { // NPC, vnum has already been extracted
    // but we create a new entry in ptr_list if it's a mobile we are reading
    if ( find_tag( chD->tag->image ) == TAG_Mobile ) {
      current_ptr = new PTR_LIST; // create
      current_ptr->ch = ch; // add info
      current_ptr->previous_ptr_address = 0;
      current_ptr->master_ptr_address = 0;
      current_ptr->leader_ptr_address = 0;
      current_ptr->fighting_ptr_address = 0;
      current_ptr->hunting_ptr_address = 0;
      current_ptr->betted_on_ptr_address = 0;
      current_ptr->challenged_ptr_address = 0;
      current_ptr->next = ptr_list_first; // and insert in the list
      ptr_list_first = current_ptr;
    }
  }

  // fields
  for ( int fieldCount = 0; fieldCount < chD->fields_count; fieldCount++ ) {
    DATAData *field = chD->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Name: ch->name = str_dup( field->value->eval().asStr() ); break;
    case TAG_Id: ch->id = field->value->eval().asInt(); break;
    case TAG_Version: ch->version = field->value->eval().asInt(); break;
    case TAG_LogO: lastlogoff = field->value->eval().asInt(); break;
    case TAG_ShD: ch->short_descr = str_dup( field->value->eval().asStr() ); break;
    case TAG_LnD: ch->long_descr = str_dup( field->value->eval().asStr() ); break;
    case TAG_Desc: ch->description = str_dup( field->value->eval().asStr() ); break;
    case TAG_Race: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->bstat(race) = race_lookup( s, TRUE ) ) < 0 ) {
	bug("Invalid race %s for char %s, assuming %s", s, ch->name, DEFAULT_RACE_NAME );
	//ch->bstat(race) = race_lookup("human");
	ch->bstat(race) = DEFAULT_RACE;
      }
      break;
    }
    case TAG_Act:
      if ( IS_NPC(ch) ) {
	if ( ( ch->act = list_flag_value_complete( field->value, act_flags ) ) == NO_FLAG ) {
	  bug("Invalid act value for mob %s, assuming NPC", ch->name );
	  ch->act = ACT_IS_NPC;
	}
      }
      else
	if ( ( ch->act = list_flag_value_complete( field->value, plr_flags ) ) == NO_FLAG ) {
	  bug("Invalid act value for player %s, assuming NOSUMMON|AUTOHAGGLE", ch->name );
	  ch->act = PLR_NOSUMMON | PLR_AUTOHAGGLE;
	}
      break;
    case TAG_Address: break; // nothing to do
    case TAG_Enchanted:
      if ( IS_NPC(ch) && field->value->eval().asInt() == TRUE ) {
	// if mob is enchanted, we don't need the affect from mob_index_data, they will be found in TAG_Affect
	ch->affected = NULL;
	SET_BIT( ch->optimizing_bit, OPTIMIZING_BIT_MODIFIED_AFFECT );
      }
      break;
    case TAG_Affect: {
      AFFECT_DATA *paf = new_affect();
      if ( parse_affect( field, paf ) ) { // add affect in list only if no error
	// removed by SinaC 2003, affect are saved only if OPTIMIZING_BIT_MODIFIED_AFFECT is set
	//	if ( IS_NPC(ch) 
	//	     && ( paf->type == gsn_sanctuary  // these affects are added in create_mobile
	//		  || paf->type == gsn_haste
	//		  || paf->type == gsn_protection_evil
	//		  || paf->type == gsn_protection_good ) ) {
	//	  break;
	//	}
	paf->next       = ch->affected;
	ch->affected    = paf;
      }
      break;
    }
    case TAG_Classes:
      if ( ( ch->bstat(classes) = list_flag_value_complete( field->value, classes_flags ) ) == NO_FLAG ) {
	bug("Invalid classes for char %s, assuming one class", ch->name );
	ch->bstat(classes) = 1;
      }
      break;
    case TAG_Clan: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->clan = clan_lookup(s) ) <= 0 )
	bug("Invalid clan %s for char %s, assuming no clan", s, ch->name );
      break;
    }
    case TAG_ClanStatus: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->clan_status = parse_clan_status(s) ) == -1 ) {
	bug("Invalid clan status %s for char %s, assuming junior", s, ch->name );
	ch->clan_status = CLAN_JUNIOR;
      }
      break;
    }
    case TAG_Condition: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 4 ) {
	bug("Wrong number of elements in list <condition 1><condition 2><condition 3><condition 4>");
	for ( int i = 0; i < 4; i++ )
	  ch->pcdata->condition[i] = 48;
	break;
      }
      for ( int i = 0; i < list->size; i++ )
	ch->pcdata->condition[i] = list->elems[i].asInt();

      list->explicit_free();
      break;
    }
    case TAG_Comm:
      if ( ( ch->comm = list_flag_value_complete( field->value, comm_flags ) ) == NO_FLAG ) {
	bug("Invalid comm for char %s, assuming no comm flags", ch->name );
	ch->comm = 0;
      }
      break;
    case TAG_Damroll: ch->bstat(damroll) = field->value->eval().asInt(); break;
    case TAG_DamType: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->bstat(dam_type) = attack_lookup( s ) ) == 0 ) {
	bug("Invalid dam type %s for char %s, assuming punch", s, ch->name );
	ch->bstat(dam_type) = attack_lookup("punch");
      }
      break;
    }
    case TAG_DisabledCmd: {
      DISABLED_CMD_DATA *p;
      ValueList *couple = field->value->eval().asList();
      if ( couple->size != 2 )
	bug("Wrong number of elements in couple <disabled command> <disabler name>");
      const char *cmd = couple->elems[0].asStr();
      const char *disabler = couple->elems[1].asStr();
      bool found = FALSE;
      int i;
      for ( i = 0; cmd_table[i].name[0] != '\0'; i++ )
	if (!str_cmp(cmd_table[i].name, cmd)) {
	  found = TRUE;
	  break;
	}
      if ( !found )
	bug("Invalid player disabled command %s for char %s", cmd, ch->name );
      else {
	if ( ( p = (DISABLED_CMD_DATA *) GC_MALLOC( sizeof( DISABLED_CMD_DATA ) ) ) == NULL )
	  p_error("Memory allocation error in disable_cmd_data");
	p->cmd = &cmd_table[i];
	p->disabled_by = str_dup(disabler); // save name of disabler
	p->next = ch->pcdata->disabled_cmd;
	ch->pcdata->disabled_cmd = p; // add before the current first element
      }

      couple->explicit_free();
      break;
    }
    case TAG_ExField: parse_extra_fields( field, (ENTITY_DATA*)ch ); break;
    case TAG_Experience: ch->exp = field->value->eval().asInt(); break;
    case TAG_Etho: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->bstat(etho) = flag_value( etho_flags, s ) ) == NO_FLAG ) {
	bug("Invalid etho %s for char %s, assuming neutral", s, ch->name );
	ch->bstat(etho) = ETHO_NEUTRAL;
      }
      break;
    }
    case TAG_Forms:
      if ( ( ch->bstat(form) = list_flag_value_complete( field->value, form_flags ) ) == NO_FLAG ) {
	bug("Invalid form for char %s, assuming %s's form", ch->name, DEFAULT_RACE_NAME );
	//ch->bstat(form) = race_table[race_lookup("human")].form;
	ch->bstat(form) = race_table[DEFAULT_RACE].form;
      }
      break;
    case TAG_Faction: {
      ValueList *couple = field->value->eval().asList();
      const char *fName = couple->elems[0].asStr();
      const int fValue =  couple->elems[1].asInt();
      const int id = get_faction_id( fName );
      if ( id < 0 )
	bug("Invalid faction name %s for char %s", fName, ch->name );
      else
	ch->pcdata->base_faction_friendliness[id] = fValue;

      couple->explicit_free();
      break;
    }
    case TAG_Gold: ch->gold = field->value->eval().asInt(); break;
    case TAG_Silver: ch->silver = field->value->eval().asInt(); break;
    case TAG_God: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->pcdata->god = god_lookup( s ) ) < 0 ) {
	bug("Invalid god %s for char %s, assuming god %s", s, ch->name, DEFAULT_GOD_NAME );
	//ch->pcdata->god = god_lookup("mota");
	ch->pcdata->god = DEFAULT_GOD;
      }
      break;
    }
    case TAG_Gset: ch->pcdata->goto_default = field->value->eval().asInt(); break;
    case TAG_Group: {
      const char *s = field->value->eval().asStr();
      int gn = group_lookup( s );
      if ( gn < 0 )
	bug("Invalid group %s for char %s", s, ch->name );
      else
	gn_add( ch, gn );
      break;
    }
    case TAG_Hitroll: ch->bstat(hitroll) = field->value->eval().asInt(); break;
    case TAG_Hometown: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->pcdata->hometown = hometown_lookup( s ) ) < 0 ) {
	if ( DEFAULT_HOMETOWN >= 0 )
	  bug("Invalid hometown %s for char %s, assuming %s", s, ch->name, DEFAULT_HOMETOWN_NAME );
	else
	  bug("Invalid hometown %s for char %s, assuming recall %d and hall %d",
	      s, ch->name, DEFAULT_RECALL, DEFAULT_HALL );
	//ch->pcdata->hometown = hometown_lookup("midgaard");
	ch->pcdata->hometown = DEFAULT_HOMETOWN;
      }
      break;
    }
    case TAG_HMVP: {
      ValueList *quartet = field->value->eval().asList();
      if ( quartet->size != 4 ) {
	bug("Wrong number of elements in quartet <hp cur/max> <mana cur/max> <move cur/max> <psp cur/max>, assuming 20");
	ch->hit = ch->mana = ch->move = ch->psp = 20;
	ch->bstat(max_hit) = ch->bstat(max_mana) = ch->bstat(max_move) = ch->bstat(max_psp) = 20;
	break;
      }
      long value[8];
      int index = 0;
      for ( int i = 0; i < quartet->size; i++ ) {
	ValueList *couple = quartet->elems[i].asList();
	if ( couple->size >= 2 ) { // ( current, max )
	  value[index] = couple->elems[0].asInt();
	  value[index+1] = couple->elems[1].asInt();
	}
	else { // ( current )      because current == max
	  value[index] = couple->elems[0].asInt();
	  value[index+1] = value[index];
	}
	index+=2;

	couple->explicit_free();
      }
      ch->hit = value[0]; ch->bstat(max_hit) = value[1];
      ch->mana = value[2]; ch->bstat(max_mana) = value[3];
      ch->move = value[4]; ch->bstat(max_move) = value[5];
      ch->psp = value[6]; ch->bstat(max_psp) = value[7];

      quartet->explicit_free();
      break;
    }
    case TAG_Immunities:
      if ( ( ch->bstat(imm_flags) = list_flag_value_complete( field->value, irv_flags ) ) == NO_FLAG ) {
	bug("Invalid immunities for char %s, assuming none", ch->name );
	ch->bstat(imm_flags) = 0;
      }
      break;
    case TAG_Resistances:
      if ( ( ch->bstat(res_flags) = list_flag_value_complete( field->value, irv_flags ) ) == NO_FLAG ) {
	bug("Invalid resistances for char %s, assuming none", ch->name );
	ch->bstat(res_flags) = 0;
      }
      break;
    case TAG_Vulnerabilities:
      if ( ( ch->bstat(vuln_flags) = list_flag_value_complete( field->value, irv_flags ) ) == NO_FLAG ) {
	bug("Invalid vulnerabilities for char %s, assuming none", ch->name );
	ch->bstat(vuln_flags) = 0;
      }
      break;
    case TAG_Invis: ch->invis_level = field->value->eval().asInt(); break;
    case TAG_Inco: ch->incog_level = field->value->eval().asInt(); break;
    case TAG_Immtitle: ch->pcdata->immtitle = str_dup( field->value->eval().asStr() ); break;
    case TAG_LastLevel: ch->pcdata->last_level = field->value->eval().asInt(); break;
    case TAG_Level: ch->level = field->value->eval().asInt(); break;
    case TAG_Language: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->pcdata->language = language_lookup( s ) ) < 0 ) {
	bug("Invalid language %s for char %s, assuming %s", s, ch->name, DEFAULT_LANGUAGE_NAME );
	//ch->pcdata->language = language_lookup( "common" );
	ch->pcdata->language = DEFAULT_LANGUAGE;
      }
      break;
    }
    case TAG_LineMode: ch->desc->lineMode = field->value->eval().asInt(); break;
    case TAG_NameAccepted: ch->pcdata->name_accepted = field->value->eval().asInt(); break;
    case TAG_Parts:
      if ( ( ch->bstat(parts) = list_flag_value_complete( field->value, part_flags ) ) == NO_FLAG ) {
	bug("Invalid parts for char %s, assuming %s's parts", ch->name, DEFAULT_RACE_NAME );
	//ch->bstat(parts) = race_table[race_lookup("human")].parts;
	ch->bstat(parts) = race_table[DEFAULT_RACE].parts;
      }
      break;
    case TAG_Password: ch->pcdata->pwd = str_dup( field->value->eval().asStr() ); break;
    case TAG_Petition: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->petition = clan_lookup(s) ) <= 0 )
	bug("Invalid petition %s for char %s, assuming no clan", s, ch->name );
      break;
    }
    case TAG_Played: ch->played = field->value->eval().asInt(); break;
    case TAG_Pnts: ch->pcdata->points = field->value->eval().asInt(); break;
    case TAG_Position: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->position = flag_value( position_flags, s ) ) == NO_FLAG ) {
	bug("Invalid position %s for char %s assuming standing", s, ch->name );
	ch->position = POS_STANDING;
      }
      break;
    }
    case TAG_Prac: ch->practice = field->value->eval().asInt(); break;
    case TAG_Prompt: ch->prompt = str_dup( field->value->eval().asStr() ); break;
      //    case TAG_QuestPnts: ch->pcdata->questpoints = field->value->eval().asInt(); break;
      //    case TAG_QuestNext: ch->pcdata->nextquest = field->value->eval().asInt(); break;
      //    case TAG_QuestCount: ch->pcdata->countdown = field->value->eval().asInt(); break;
      //    case TAG_QuestObj: ch->pcdata->questobj = field->value->eval().asInt(); break;
      //    case TAG_QuestObjLoc: ch->pcdata->questobjloc = field->value->eval().asInt(); break;
      //    case TAG_QuestMob: ch->pcdata->questmob = field->value->eval().asInt(); break;
      //    case TAG_QuestGiver: {
      //      const int vnum = field->value->eval().asInt();
      //      MOB_INDEX_DATA *mob = get_mob_index( vnum );
      //      if ( mob == NULL )
      //	bug("Invalid quest giver %d for char %s", vnum, ch->name );
      //      else
      //	ch->pcdata->questgiver = find_mob( mob );
      //      break;
      //    }
    case TAG_Room: {
      const int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *room = get_room_index( vnum );
      if ( !room ) {
	bug("Invalid room vnum %d for char %s", vnum, ch->name );
	ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
      }
      else
	ch->in_room = room;
      break;
    }
    case TAG_SavingThrow: ch->bstat(saving_throw) = field->value->eval().asInt(); break;
    case TAG_Scroll: ch->lines = field->value->eval().asInt(); break;
    case TAG_Sex: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->bstat(sex) = flag_value( sex_flags, s ) ) == NO_FLAG ) {
	bug("Invalid sex %s for char %s, assuming male", s, ch->name );
	ch->bstat(sex) = 0;
      }
      break;
    }
    case TAG_Security: ch->pcdata->security = field->value->eval().asInt(); break;
    case TAG_Size: {
      const char *s = field->value->eval().asStr();
      if ( ( ch->bstat(size) = flag_value( size_flags, s ) ) == NO_FLAG ) {
	bug("Invalid size %s for char %s, assuming medium", s, ch->name );
	ch->bstat(size) = SIZE_MEDIUM;
      }
      break;
    }
    case TAG_Stance: ch->pcdata->stance = field->value->eval().asStr(); break;
    case TAG_Ability: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 4 ) {
	bug("Wrong number of elements in list <perc> <casting level> <level> <ability name>");
	break;
      }
      const char *s = list->elems[3].asStr();
      const int sn = ability_lookup(s);
      if ( sn < 0 ) {
	bug("Invalid ability name %s for char %s", s, ch->name );
	break;
      }
      ch->pcdata->ability_info[sn].learned = list->elems[0].asInt();
      ch->pcdata->ability_info[sn].casting_level = list->elems[1].asInt();
      if ( ch->pcdata->ability_info[sn].casting_level > ability_table[sn].nb_casting_level ) {
	int v = UMIN(ch->pcdata->ability_info[sn].casting_level, ability_table[sn].nb_casting_level);
	bug("Invalid casting level %d for ability %s (max value: %d), setting to %d",
	    ch->pcdata->ability_info[sn].casting_level, ability_table[sn].name,
	    ability_table[sn].nb_casting_level, v );
	ch->pcdata->ability_info[sn].casting_level = v;
      }
      if ( ch->pcdata->ability_info[sn].casting_level == 0
	   && ability_table[sn].nb_casting_level > 0 ) {
	bug("Ability %s has casting level but player has level 0, setting to 1",
	    ability_table[sn].name );
	ch->pcdata->ability_info[sn].casting_level = 1;
      }
      ch->pcdata->ability_info[sn].level = list->elems[2].asInt();
      if ( ch->pcdata->ability_info[sn].level > IM ) {
	bug("Invalid level %d for ability %s for char %s, assuming %d",
	    ch->pcdata->ability_info[sn].level, s, ch->name, IM );
	ch->pcdata->ability_info[sn].level = IM;
      }

      list->explicit_free();
      break;
    }
    case TAG_Train: ch->train = field->value->eval().asInt(); break;
    case TAG_Trust: ch->trust = field->value->eval().asInt(); break;
    case TAG_Trivia: ch->pcdata->trivia = field->value->eval().asInt(); break;
    case TAG_AfBy:
      if ( ( ch->bstat(affected_by) = list_flag_value_complete( field->value, affect_flags ) ) == NO_FLAG ) {
	bug("Invalid affect for char %s, assuming none", ch->name );
	ch->bstat(affected_by) = 0;
      }
      break;
    case TAG_AfBy2:
      if ( ( ch->bstat(affected2_by) = list_flag_value_complete( field->value, affect2_flags ) ) == NO_FLAG ) {
	bug("Invalid affect2 for char %s, assuming none", ch->name );
	ch->bstat(affected2_by) = 0;
      }
      break;
    case TAG_Attr: {
      ValueList *attr = field->value->eval().asList();
      if ( attr->size != MAX_STATS ) {
	bug("Wrong number of elements in attributes, assuming 13");
	for ( int i = 0; i < MAX_STATS; i++ )
	  ch->baseattr[ATTR_STR+i] = 13;
	break;
      }
      for ( int i = 0; i < attr->size; i++ )
	ch->baseattr[ATTR_STR+i] = attr->elems[i].asInt();

      attr->explicit_free();
      break;
    }
    case TAG_ACs: {
      ValueList *ac = field->value->eval().asList();
      if ( ac->size != 4 ) {
	bug("Wrong number of elements in ACs, assuming 0");
	for ( int i = 0; i < 4; i++ )
	  ch->baseattr[ATTR_ac0+i] = 0;
	break;
      }
      for ( int i = 0; i < 4; i++ )
	ch->baseattr[ATTR_ac0+i] = ac->elems[i].asInt();

      ac->explicit_free();
      break;
    }
    case TAG_Alignment: ch->bstat(alignment) = field->value->eval().asInt(); break;
    case TAG_Wiznet: ch->wiznet = flag_to_long( field->value->eval().asStr() ); break;
    case TAG_Wimpy: ch->wimpy = field->value->eval().asInt(); break;
    case TAG_Beacons: {
      ValueList *list = field->value->eval().asList();
      int n = list->size;
      if ( list->size > MAX_BEACONS ) {
	bug("Wrong number of beacons for char %s", ch->name );
	n = MAX_BEACONS;
      }
      for ( int i = 0; i < n; i++ )
	ch->pcdata->beacons[i] = list->elems[i].asInt();

      list->explicit_free();
      break;
    }
    case TAG_Boards: {
      ValueList *list = field->value->eval().asList();
      for ( int i = 0; i < list->size; i++ ) {
	ValueList *couple = list->elems[i].asList();
	if ( couple->size != 2 ) {
	  bug("Wrong number of elements in couple <board name> <last note id>");
	  continue;
	}
	const char *s = couple->elems[0].asStr();
	const int boardId = board_lookup(s);
	if ( boardId == BOARD_NOTFOUND ) {
	  bug("Invalid board name %s for char %s", s, ch->name );
	  continue;
	}
	ch->pcdata->last_note[boardId] = couple->elems[1].asInt();

	couple->explicit_free();
      }

      list->explicit_free();
      break;
    }
    case TAG_Title: {
      const char *s = field->value->eval().asStr();
      if ( s[0] != '.' && s[0] != ',' && s[0] != '!' && s[0] != '?' ) {
	char buf[MAX_STRING_LENGTH];
	sprintf(buf," %s", s );
	ch->pcdata->title = str_dup(buf);
      }
      else
	ch->pcdata->title = str_dup(s);
      break;
    }
    case TAG_Alias: {
      if ( countAlias >= MAX_ALIAS ) {
	bug("Too many aliases, max: %d", MAX_ALIAS );
	break;
      }
      ValueList *couple = field->value->eval().asList();
      if ( couple->size != 2 ) {
	bug("Wrong number of elements in couple <alias> <substitute>");
	break;
      }
      ch->pcdata->alias[countAlias] = str_dup(couple->elems[0].asStr());
      ch->pcdata->alias_sub[countAlias] = str_dup(couple->elems[1].asStr());
      countAlias++;

      couple->explicit_free();
      break;
    }
    case TAG_Bin: ch->pcdata->bamfin = str_dup( field->value->eval().asStr() ); break;
    case TAG_Bout: ch->pcdata->bamfout = str_dup( field->value->eval().asStr() ); break;
    case TAG_Dnumber: ch->bstat(DICE_NUMBER) = field->value->eval().asInt(); break;
    case TAG_Dtype: ch->bstat(DICE_TYPE) = field->value->eval().asInt(); break;

    case TAG_Timer: ch->timer = field->value->eval().asInt(); break;
    case TAG_Daze: ch->daze = field->value->eval().asInt(); break;
    case TAG_Stunned: ch->stunned = field->value->eval().asInt(); break;
    case TAG_BetAmt: ch->pcdata->bet_amt = field->value->eval().asInt(); break;

    case TAG_IsWildMagic: ch->isWildMagic = field->value->eval().asInt(); break; // SinaC 2003

    case TAG_Rebirth: { // SinaC 2003: save player doing a rebirth/remort
      const char *s = field->value->eval().asStr();
      if ( ( ch->pcdata->tmpRace = race_lookup( s, TRUE ) ) < 0 ) {
	bug("new_load_pFile: invalid rebirth race [%s], assuming %s",
	    s, DEFAULT_RACE_NAME );
	ch->pcdata->tmpRace = DEFAULT_PC_RACE;
      }
      break;
    }
    case TAG_Remort: { // SinaC 2003: save player doing a rebirth/remort
      const char *s = field->value->eval().asStr();
      if ( ( ch->pcdata->tmpRace = race_lookup( s, TRUE ) ) < 0 ) {
	bug("new_load_pFile: invalid remort race [%s], assuming %s",
	    s, DEFAULT_RACE_NAME );
	ch->pcdata->tmpRace = DEFAULT_PC_RACE;
      }
      break;
    }

    case TAG_Pet: {
      CHAR_DATA *pet;
      int vnum = field->value->eval().asInt(); // get pet vnum
      if ( get_mob_index(vnum) == NULL ) {
	bug("new_load_pFile: bad pet vnum %d.", vnum );
	pet = create_mobile(get_mob_index(MOB_VNUM_DUMMY));
      }
      else
	pet = create_mobile(get_mob_index(vnum));

      parse_char( field, pet );

      pet->leader = ch;
      pet->master = ch;
      ch->pet = pet;
      SET_BIT(pet->bstat(affected_by), AFF_CHARM);
      recompute(pet);
      break;
    }
    case TAG_Obj: {
      OBJ_DATA *obj = NULL;
      bool new_format = FALSE;
      int vnum = field->value->eval().asInt(); // get obj vnum
      if (  get_obj_index( vnum ) == NULL ) {
	bug( "new_load_pFile: bad obj vnum %d.", vnum );
	obj = new_obj();
	obj->name		= str_dup( "" );
	obj->short_descr	= str_dup( "" );
	obj->description	= str_dup( "" );
	obj->pIndexData = get_obj_index( OBJ_VNUM_DUMMY );
      }
      else {
	obj = create_object(get_obj_index(vnum),-1);
	new_format = TRUE;
      }
      parse_obj( field, obj, new_format );
      if ( obj != NULL && obj->valid )
        obj_to_char( obj, ch );
      break;
    }

    case TAG_Mobile: { // special case to load mobile from a mud state file after a copyover for example
                       // should never happen because we don't save follower
      int vnum = field->value->eval().asInt(); // get pet vnum
      if ( get_mob_index(vnum) == NULL ) {
	bug("new_load_char: mobile vnum %d doesn't exist anymore.", vnum );
	break;
      }
      CHAR_DATA *mob;
      mob = create_mobile(get_mob_index(vnum));
      mob->in_room = ch->in_room; // already set the in_room so we can test 
                                  //  if master/fighting is in the same room

      parse_char( field, mob );

      char_to_room( mob, ch->in_room );
      break;
    }
    case TAG_Ptr:
      current_ptr->previous_ptr_address = field->value->eval().asInt();
      ch->ptr_before_copyover = current_ptr->previous_ptr_address; // SinaC 2003
      break;
    case TAG_Master: {
      Value v = field->value->eval();
      if ( v.typ == SVT_INT )
	current_ptr->master_ptr_address = field->value->eval().asInt();
      else if ( v.typ == SVT_STR ) {
	const char *s = v.asStr();
	CHAR_DATA *master = get_pc_world( s );
	if ( master == NULL ) {
	  bug("new_load_char: mobile [%s]'s master (%s) doesn't exist anymore", NAME(ch), s );
	  break;
	}
	//	if ( master->in_room != ch->in_room ) {
	//	  bug("new_load_char: mobile [%s] and master (%s) are not in the same room anymore", NAME(ch), s );
	//	  break;
	//	}
	log_stringf("[%s] was following [%s]", NAME(ch), NAME(master));
	//add_follower( ch, master );
	ch->master = master;
      }
      break;
    }
    case TAG_Leader: {
      Value v = field->value->eval();
      if ( v.typ == SVT_INT )
	current_ptr->leader_ptr_address = field->value->eval().asInt();
      else if ( v.typ == SVT_STR ) {
	const char *s = v.asStr();
	CHAR_DATA *leader = get_pc_world( s );
	if ( leader == NULL ) {
	  bug("new_load_char: mobile [%s]'s leader (%s) doesn't exist anymore", NAME(ch), s );
	  break;
	}
	//	if ( leader->in_room != ch->in_room ) {
	//	  bug("new_load_char: mobile [%s] and master (%s) are not in the same room anymore", NAME(ch), s );
	//	  break;
	//	}
	log_stringf("[%s] was in [%s]'s group", NAME(ch), NAME(leader));
	//add_follower( ch, master );
	ch->leader = leader;
      }
      break;
    }
    case TAG_Fighting: {
      Value v = field->value->eval();
      if ( v.typ == SVT_INT )
	current_ptr->fighting_ptr_address = field->value->eval().asInt();
      else if ( v.typ == SVT_STR ) {
	const char *s = v.asStr();
	CHAR_DATA *fighter = get_pc_world( s );
	if ( fighter == NULL ) {
	  bug("new_load_char: mobile [%s]'s PC fighter (%s) doesn't exist anymore", NAME(ch), s );
	  break;
	}
	if ( fighter->in_room != ch->in_room ) {
	  bug("new_load_char: mobile [%s] and PC fighter (%s) are not in the same room anymore", NAME(ch), s );
	  break;
	}
	log_stringf("Restarting fight between [%s] and [%s]", NAME(ch), NAME(fighter));
	set_fighting( ch, fighter );
      }
      break;
    }
    case TAG_Hunting: {
      Value v = field->value->eval();
      if ( v.typ == SVT_INT )
	current_ptr->hunting_ptr_address = field->value->eval().asInt();
      else if ( v.typ == SVT_STR ) {
	const char *s = v.asStr();
	CHAR_DATA *hunted = get_pc_world( s );
	if ( hunted == NULL ) {
	  bug("new_load_char: mobile [%s]'s PC hunted (%s) doesn't exist anymore", NAME(ch), s );
	  break;
	}
	log_stringf("[%s] hunts [%s] again", NAME(ch), NAME(hunted));
	ch->hunting = hunted;
      }
      break;
    }
    case TAG_BettedOn: {
      Value v = field->value->eval();
      if ( v.typ == SVT_INT )
	current_ptr->betted_on_ptr_address = field->value->eval().asInt();
      else if ( v.typ == SVT_STR ) {
	const char *s = v.asStr();
	CHAR_DATA *betted_on = get_pc_world( s );
	if ( betted_on == NULL ) {
	  bug("new_load_char: mobile [%s]'s PC betted_on (%s) doesn't exist anymore", NAME(ch), s );
	  break;
	}
	log_stringf("[%s] betted on [%s]", NAME(ch), NAME(betted_on));
	ch->pcdata->betted_on = betted_on;
      }
      break;
    }
    case TAG_Challenged: {
      Value v = field->value->eval();
      if ( v.typ == SVT_INT )
	current_ptr->challenged_ptr_address = field->value->eval().asInt();
      else if ( v.typ == SVT_STR ) {
	const char *s = v.asStr();
	CHAR_DATA *challenged = get_pc_world( s );
	if ( challenged == NULL ) {
	  bug("new_load_char: mobile [%s]'s PC challenged (%s) doesn't exist anymore", NAME(ch), s );
	  break;
	}
	if ( challenged->in_room != ch->in_room ) {
	  bug("new_load_char: mobile [%s] and PC challenged (%s) are not in the same room anymore", NAME(ch), s );
	  break;
	}
	log_stringf("[%s] challenged [%s]", NAME(ch), NAME(challenged));
	ch->pcdata->challenged = challenged;
      }
      break;
    }
    case TAG_Remort_Count: ch->pcdata->remort_count = field->value->eval().asInt(); break;
      // SinaC 2003, store dynamic class if different from static one
    case TAG_Clazz: {
      const char *s = field->value->eval().asStr();
      CLASS_DATA *cl = silent_hash_get_prog(s);
      if ( cl == NULL ) {
	bug("new_load_char: mobile [%s] dynamic clazz [%s] cannot be found", NAME(ch), s );
	break;
      }
      if ( get_root_class(cl) != default_mob_class ) {
	bug("new_load_char: mobile [%s] dynamic clazz [%s] is not anymore a mob class", NAME(ch), s );
	break;
      }
      ch->clazz = cl;
      break;
    }

    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }

  // adjust hp mana move up  -- here for speed's sake
  int percent = (current_time - lastlogoff) * 25 / ( 2 * 60 * 60);
  percent = UMIN(percent,100);

  if ( ch->position != POS_STANDING
       && ch->position != POS_SITTING
       && ch->position != POS_RESTING
       && ch->position != POS_SLEEPING )
    ch->position = POS_STANDING;
  
  if ( percent > 0 && !IS_AFFECTED(ch,AFF_POISON)
       && !IS_AFFECTED(ch,AFF_PLAGUE)){
    ch->hit     += (ch->bstat(max_hit) - ch->hit) * percent / 100;
    ch->mana    += (ch->bstat(max_mana) - ch->mana) * percent / 100;
    ch->move    += (ch->bstat(max_move) - ch->move)* percent / 100;
    ch->psp     += (ch->bstat(max_psp) - ch->psp) * percent / 100;
  }
}
void parse_player( DATAData *player ) {
  log_stringf("Loading %s.", player_loading->name);
  parse_char( player, player_loading );
  log_stringf(" done.");
}



//******************************************************************************************************
//******************************************************************************************************
//*********************** SAVE/LOAD WORLD STATE BEFORE/AFTER A SHUTDOWN/COPYOVER/... *******************
//******************************************************************************************************
//******************************************************************************************************
const char * replace_char( const char *s, char from, char to ) {
  static char buf[MAX_STRING_LENGTH];
  const char *q = s;
  char *p = buf;

  while ( *q != '\0' ) {
    if ( *q == from )
      *p = to;
    else
      *p = *q;
    p++; q++;
  }
  *p = '\0';

  return buf;
}
// -- SAVE
const char *save_area_state( AREA_DATA *pArea ) {
  static char fileName[MAX_STRING_LENGTH];
  FILE *fp;
  data_depth = 1;
  sprintf( fileName, "%s%s%s", DUMP_DIR, replace_char(pArea->file_name, '/', '_'), ".state" );
  log_stringf("  saving area state: %s  in  %s", pArea->name, fileName );
  if ( ( fp = fopen( fileName, "w" ) ) == NULL ) {
    bug("Can't open world state file %s.", fileName );
    fclose(fp);
    return NULL;
  }
  sh_fprintf( fp, "AreaState %s {\n", quotify(pArea->name) );
  // FOR EACH ROOMS
  for( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    for( ROOM_INDEX_DATA *pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      if ( pRoomIndex->area == pArea ) {
	// don't save empty room
	if ( pRoomIndex->people == NULL 
	     && pRoomIndex->contents == NULL 
	     && pRoomIndex->current_affected == NULL
	     && pRoomIndex->ex_fields == NULL
	     && pRoomIndex->current_time_repop == 0 )
	  continue;
	data_depth++;
	sh_fprintf( fp, "Room %d {\n", pRoomIndex->vnum );
	// if repop_time <> 0, save it
	if ( pRoomIndex->current_time_repop != 0 )
	  sh_fprintf( fp, "  Repop = %d;\n", pRoomIndex->current_time_repop );
	// FOR EACH ROOM's EXTRA FIELDS: save extra fields
	new_save_extra_fields( pRoomIndex, fp );
	// FOR EACH ROOM's AFFECT: save affect
	//for ( AFFECT_DATA *paf = pRoomIndex->current_affected; paf != NULL; paf = paf->next )
	//  new_save_affects( paf, fp );
	new_save_affects( pRoomIndex->current_affected, fp );
	// FOR EACH EXIT: save modified exit, SinaC 2003
	for ( int door = 0; door < MAX_DIR; door++ ) {
	  EXIT_DATA *pExit = pRoomIndex->exit[door];
	  if ( pExit != NULL 
	       && pExit->u1.to_room != NULL 
	       && ( pExit->exit_info != pExit->rs_flags // flags have been modified
		    || IS_SET( pExit->exit_info, EX_NOSAVING ) ) // this exit has been created
	       ) {
	    sh_fprintf( fp, "  Exit = ( %d, %d, (%s), (%s) );\n",
			door, pExit->u1.to_room->vnum,
			list_flag_string( pExit->exit_info, exit_flags ),
			list_flag_string( pExit->rs_flags, exit_flags ) );
	  }
	}
	// FOR EACH MOBS: save mob in room +equipement+inventory
	for ( CHAR_DATA *mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room )
	  if ( IS_NPC(mob) 
	       && !( mob->master != NULL && mob->master->pet == mob ) )
	    new_save_char( mob, fp );
	// FOR EACH OBJS: save obj in room
	if ( pRoomIndex->contents != NULL )
	  new_save_obj_list( NULL, pRoomIndex->contents, fp );
	sh_fprintf( fp, "}\n" );
	data_depth--;
      }

  REMOVE_BIT( pArea->area_flags, AREA_SAVE_STATE );

  sh_fprintf( fp, "}\n" );
  fclose( fp );

  return fileName;
}

void save_world_state() {
  log_stringf("Saving world state.");

  FILE *fpList;
  char listName[MAX_STRING_LENGTH];
  sprintf( listName, "%s%s", DUMP_DIR, "list" ); // "list" should a global string WORLD_STATE_LIST_FILE
  if ( ( fpList = fopen( listName, "w" ) ) == NULL ) {
    bug("Can't open world state list file.");
    return;
  }
  // FOR EACH AREA
  for( AREA_DATA *pArea = area_first; pArea; pArea = pArea->next ) {
    const char *fileName = save_area_state( pArea ); // save the area state and return filename
    if ( fileName != NULL )
      fprintf( fpList, "%s\n", fileName ); // print filename in area list file
  }
  fprintf( fpList, "$\n" );
  fclose( fpList );
}

// -- LOAD
void parse_room_state( DATAData *room, ROOM_INDEX_DATA *pRoomIndex ) {
  // room->value is not important: room vnum

  // fields
  for ( int fieldIndex = 0; fieldIndex < room->fields_count; fieldIndex++ ) {
    DATAData *field = room->fields[fieldIndex];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
    printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Repop: pRoomIndex->current_time_repop = field->value->eval().asInt(); break;
    case TAG_ExField: parse_extra_fields( field, pRoomIndex ); break;
    case TAG_Affect: {
      AFFECT_DATA *paf = new_affect();
      if ( parse_affect( field, paf ) ) { // add affect in list only if no error
	paf->next       = pRoomIndex->current_affected;
	pRoomIndex->current_affected   = paf;
      }
      break;
    }
    case TAG_Exit: { // SinaC 2003
      ValueList *l = field->value->eval().asList();
      if ( l->size != 4 ) {
	bug("Invalid exit elements (%d) for room [%d]", l->size, pRoomIndex->vnum );
	break;
      }
      int door = l->elems[0].asInt();
      if ( door < 0 || door >= MAX_DIR ) {
	bug("Invalid exit door [%d] for room [%d]", door, pRoomIndex->vnum );
	break;
      }
      int to_room = l->elems[1].asInt();
      ROOM_INDEX_DATA *toRoom = get_room_index( to_room );
      if ( toRoom == NULL ) {
	bug("Invalid exit to_room [%d] for room [%d]", to_room, pRoomIndex->vnum );
	break;
      }
      int exit_info;
      if ( ( exit_info = list_flag_value_complete( l->elems[2].asList(), exit_flags ) ) == NO_FLAG ) {
	bug("Invalid exit exit_info exit [%d] in room [%d]", door, pRoomIndex->vnum );
	break;
      }
      int rs_flags;
      if ( ( rs_flags = list_flag_value_complete( l->elems[3].asList(), exit_flags ) ) == NO_FLAG ) {
	bug("Invalid exit rs_flags for exit [%d] in room [%d]", door, pRoomIndex->vnum );
	break;
      }
      EXIT_DATA *pExit = pRoomIndex->exit[door];
      if ( pExit == NULL
	   && pExit->u1.to_room == NULL ) // exit doesn't exist
	if ( !IS_SET( exit_info, EX_NOSAVING ) ) { // no flag 'nosaving' -> problem
	  char buf1[MAX_STRING_LENGTH];
	  char buf2[MAX_STRING_LENGTH];
	  strcpy( buf1, flag_string( exit_flags, exit_info ) );
	  strcpy( buf2, flag_string( exit_flags, rs_flags ) );
	  bug("Inexistant exit [%d] exit_info [%s] rs_flags [%s] in room [%d]. 'nosaving' flag missing.",
	      door, buf1, buf2, pRoomIndex->vnum );
	  break;
	}
	else { // flag 'nosaving' found -> create the door
	  log_stringf("Recreating exit door [%d] between rooms [%d] and [%d]",
		      door, pRoomIndex->vnum, to_room );
	  pExit = new_exit();
	  pExit->u1.to_room = toRoom;
	  pExit->orig_door = door;
	  pRoomIndex->exit[door] = pExit;
	  pExit->rs_flags = rs_flags;
	  pExit->exit_info = exit_info;
	}
      else // exit already exists
	if ( pExit->u1.to_room != toRoom ) { // exit points to a different room, may happen in maze
	  bug("Exit door [%d] found but to room [%d] instead of [%d]",
	      door, pExit->u1.to_room->vnum, to_room );
	  break;
	}
	else { // exit exists and points to the right room -> change flags
	  pExit->rs_flags = rs_flags;
	  pExit->exit_info = exit_info;
	  log_stringf("Updating exit door [%d] rs_flags & exit_info for room [%d]",
		      door, pRoomIndex->vnum );
	}
      break;
    } 
   case TAG_Mobile: { // mob in room
      int vnum = field->value->eval().asInt(); // get mob vnum
      MOB_INDEX_DATA *pMobIndex = get_mob_index(vnum);
      if ( pMobIndex == NULL ) {
	bug("Mobile %d in room %d doesn't exist anymore.", vnum, pRoomIndex->vnum );
	break;
      }
      CHAR_DATA *mob;
      mob = create_mobile( pMobIndex );
      mob->in_room = pRoomIndex; // already set the in_room so we can test 
                                 //  if master/fighting is in the same room

      parse_char( field, mob );

      char_to_room( mob, pRoomIndex );
      break;
    }
    case TAG_Obj: { // obj in room
      int vnum = field->value->eval().asInt(); // get obj vnum
      OBJ_INDEX_DATA *pObjIndex = get_obj_index(vnum);
      if ( pObjIndex == NULL ) {
	bug("Object %d in room %d doesn't exist anymore.", vnum, pRoomIndex->vnum );
	break;
      }
      OBJ_DATA *obj;
      obj = create_object( pObjIndex, -1 );

      parse_obj( field, obj, TRUE );

      obj_to_room( obj, pRoomIndex );
      break;
    }
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }
}
void parse_area_state( DATAData *area ) {
  // area->value is not interesting: just the area name

  // fields
  for ( int fieldIndex = 0; fieldIndex < area->fields_count; fieldIndex++ ) {
    DATAData *field = area->fields[fieldIndex];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Room: {
      int vnum = field->value->eval().asInt();
      ROOM_INDEX_DATA *pRoomIndex = get_room_index( vnum );
      if ( pRoomIndex == NULL ) {
	bug("Room %d doesn't exist anymore.", vnum );
	break;
      }

      // nullify mob's ptr list, this list may not contain pointer to another area
      ptr_list_first = NULL;

      parse_room_state( field, pRoomIndex );
      recomproom(pRoomIndex);

      // we set AREA_SAVE_STATE to FALSE
      AREA_DATA *pArea = pRoomIndex->area;
      if ( pArea != NULL )
	REMOVE_BIT( pArea->area_flags, AREA_SAVE_STATE );

      // use the ptr_list to recreate groups and restart fights
      for ( PTR_LIST *p = ptr_list_first; p != NULL; p = p->next ) {
	if ( p->master_ptr_address != 0 ) { // had a master before
	  // find it
	  bool found = FALSE;
	  for ( PTR_LIST *q = ptr_list_first; q != NULL; q = q->next )
	    if ( q->previous_ptr_address == p->master_ptr_address ) {
	      log_stringf("[%s] was following [%s]", NAME(p->ch), NAME(q->ch));
	      //add_follower( p->ch, q->ch );
	      p->ch->master = q->ch;
	      found = TRUE;
	      break;
	    }
	  if (!found)
	    bug("new_load_char: mobile [%s]'s master (address %d) doesn't exist anymore", 
		NAME(p->ch), p->master_ptr_address);
	}
	if ( p->leader_ptr_address != 0 ) { // had a leader before
	  // find it
	  bool found = FALSE;
	  for ( PTR_LIST *q = ptr_list_first; q != NULL; q = q->next )
	    if ( q->previous_ptr_address == p->leader_ptr_address ) {
	      log_stringf("[%s] was in [%s]'s group", NAME(p->ch), NAME(q->ch));
	      //add_follower( p->ch, q->ch );
	      p->ch->leader = q->ch;
	      found = TRUE;
	      break;
	    }
	  if (!found)
	    bug("new_load_char: mobile [%s]'s leader (address %d) doesn't exist anymore", 
		NAME(p->ch), p->leader_ptr_address);
	}
	if ( p->fighting_ptr_address != 0 ) { // was in a fight before
	  // find opponent
	  bool found = FALSE;
	  for ( PTR_LIST *q = ptr_list_first; q != NULL; q = q->next )
	    if ( q->previous_ptr_address == p->fighting_ptr_address ) {
	      log_stringf("Restarting fight between [%s] and [%s]", NAME(p->ch), NAME(q->ch));
	      set_fighting( p->ch, q->ch );
	      found = TRUE;
	      break;
	    }
	  if (!found)
	    bug("new_load_char: mobile [%s]'s fighter (address %d) doesn't exist anymore", 
		NAME(p->ch), p->fighting_ptr_address);
	}
	if ( p->hunting_ptr_address != 0 ) { // was hunting before, should only hunt PC
	  // find opponent
	  bool found = FALSE;
	  for ( PTR_LIST *q = ptr_list_first; q != NULL; q = q->next )
	    if ( q->previous_ptr_address == p->hunting_ptr_address ) {
	      log_stringf("Restarting hunting between [%s] and [%s]", NAME(p->ch), NAME(q->ch));
	      p->ch->hunting = q->ch;
	      found = TRUE;
	      break;
	    }
	  if (!found)
	    bug("new_load_char: mobile [%s]'s hunting (address %d) doesn't exist anymore", 
		NAME(p->ch), p->hunting_ptr_address);
	}
	if ( p->challenged_ptr_address != 0 ) { // was challenging before, only PC should use this
	  // find opponent
	  bool found = FALSE;
	  for ( PTR_LIST *q = ptr_list_first; q != NULL; q = q->next )
	    if ( q->previous_ptr_address == p->challenged_ptr_address ) {
	      log_stringf("Restarting challenge between [%s] and [%s]", NAME(p->ch), NAME(q->ch));
	      p->ch->pcdata->challenged = q->ch;
	      found = TRUE;
	      break;
	    }
	  if (!found)
	    bug("new_load_char: mobile [%s]'s challenging (address %d) doesn't exist anymore", 
		NAME(p->ch), p->challenged_ptr_address);
	}
	if ( p->betted_on_ptr_address != 0 ) { // was betting before, only PC should use this
	  // find opponent
	  bool found = FALSE;
	  for ( PTR_LIST *q = ptr_list_first; q != NULL; q = q->next )
	    if ( q->previous_ptr_address == p->betted_on_ptr_address ) {
	      log_stringf("Restarting betting on between [%s] and [%s]", NAME(p->ch), NAME(q->ch));
	      p->ch->pcdata->betted_on = q->ch;
	      found = TRUE;
	      break;
	    }
	  if (!found)
	    bug("new_load_char: mobile [%s]'s betting on (address %d) doesn't exist anymore", 
		NAME(p->ch), p->betted_on_ptr_address);
	}
      }

      break;
    }
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }
}

void load_world_state() {
  log_stringf("Loading world state.");

  FILE *fpList;
  char listName[MAX_STRING_LENGTH];
  sprintf( listName, "%s%s", DUMP_DIR, "list" ); // "list" should be a global string WORLD_STATE_LIST_FILE
  if ( ( fpList = fopen( listName, "r" ) ) == NULL ) {
    bug("Can't open world state list file -> calling area_update");
    area_update();
    return;
  }

  // We set AREA_SAVE_STATE to true before load area state, so we can detect which area
  //  have no area state file
  for( AREA_DATA *pArea = area_first; pArea; pArea = pArea->next )
    SET_BIT( pArea->area_flags, AREA_SAVE_STATE );

  fBootDb = TRUE;
  for ( ; ; ) { // we use strArea and fpArea to get messages from bug
    strcpy( strArea, fread_word( fpList ) );
    if ( strArea[0] == '$' )
      break;
    log_stringf( "  loading area state:%s", strArea );
    if ( ( fpArea = fopen( strArea, "r" ) ) == NULL ) {
      bug("Can't open find area stat file: %s", strArea );
      continue;
    }

    parse_datas( fpArea );
  
    fclose( fpArea );
  }
  fBootDb = FALSE;
  fclose(fpList);

  fpArea = NULL;

  // If after reading state file, an area already has it's AREA_SAVE_STATE set
  //  then we must do an reset_area
  for( AREA_DATA *pArea = area_first; pArea; pArea = pArea->next )
    if ( IS_SET( pArea->area_flags, AREA_SAVE_STATE ) ) {
      log_stringf("%s had no state file -> use reset_area", pArea->name );
      reset_area( pArea );
      REMOVE_BIT( pArea->area_flags, AREA_SAVE_STATE );
    }

  // When loading world state, some extra fields have been delayed, we have to parse them
  CHAR_DATA *ch_next;
  for ( CHAR_DATA *ch = char_list; ch; ch = ch_next ) {
    ch_next = ch->next;
    use_delayed_parse_extra_fields( ch );

    MOBPROG( ch, NULL, "onReset" ); // SinaC 2003
  }

  OBJ_DATA *obj_next;
  for ( OBJ_DATA *obj = object_list; obj; obj = obj_next ) {
    obj_next = obj->next;
    use_delayed_parse_extra_fields( obj );

    OBJPROG( obj, NULL, "onReset" ); // SinaC 2003
  }

  for ( int i = 0; i < 65535; i++ ) {
    ROOM_INDEX_DATA *room;
    if ( ( room = get_room_index( i ) ) == NULL )
      continue;

    use_delayed_parse_extra_fields( room );
    ROOMPROG( room, NULL, "onReset" ); // SinaC 2003, not used for the moment
  }

  log_stringf("Done.");
}
