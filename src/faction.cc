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
#include "faction.h"
#include "db.h"
#include "olc_value.h"
#include "lookup.h"
#include "fight.h"
#include "dbdata.h"
#include "config.h"
#include "handler.h"

//#define VERBOSE

bool USE_FACTION_ON_AGGRO = TRUE; // initialized in config.C
bool factionnotsaved;

int factions_count;
FACTION_DATA faction_table[MAX_FACTION];
const float factionUpdateRate = 0.01; // TO DO   FIXME: find a better value

//------------------------------------ Faction initialization methods
//  <|> are NPC races
//Elf       sylvan-elf, grey-elf, high-elf, half-elf, sea-elf, avariel, fire-elf, desert-elf
//Under     drow, duergar, svirfneblin
//Dwarf     mountain-dwarf, hill-dwarf, half-dwarf
//Gnome     forest-gnome, rock-gnome, half-gnome
//Giant     storm-giant, cloud-giant, fire-giant, frost-giant, hill-giant, half-giant, troll
//Goblin    goblin, hobgoblin, orc, half-orc, half-ogre, ogre, uruk-hai, gnoll
//Undead    shade, mummy, ghost, spectre, wight, death-knight, ghoul, banshee, vampire, lich
//Common    human, nekojin, lizardman, minotaur, halfling, mongrelman, ratling, wolfen
//Dragon    draconian, holy-dragon, faerie-dragon, black-dragon, crystal-dragon, silver-dragon, <|> dragon
//Animal   <|>bat, bear, cat, centipede, dog, fido, fox, fish, horse, lizards, pig, rabbit, snake, song bird, water fowl, wolfs

// ---
// --- Which faction ?
// - PC
//centaur          //satyr          //quickling      //sprite            //merfolk          //diva
//eldar            //dryad          //kenku          //avian             //seraphim         //succubus
//pitfiend         //dracolich      //doppleganger   //gargoyle          //titan
// - NPC
//school monster   //wyvern         //pixie          //cyclops           //kobold           //doll
//modron           //unique

// FIXME: the faction is based on the race and only set at mob creation
//  but mob's race can change

void set_default_faction (MOB_INDEX_DATA *mob) {
  //FIXME
  // race/class...
  int raceFaction = get_race_faction_id( mob->race );
  if ( raceFaction < 0 )
    mob->faction = get_neutral_faction();
  else {
    mob->faction = raceFaction;
#ifdef VERBOSE
    log_stringf(" setting mob [vnum: %d] faction to [%s]", mob->vnum, faction_table[mob->faction].name);
#endif
  }
  //mob->faction = number_range(1,factions_count-1);
}

// ------------------------------------ Faction management methods

int range_faction( const int factionValue ) {
  return URANGE( MIN_FACTION_VALUE, factionValue, MAX_FACTION_VALUE );
}

FACTION_DATA* get_faction(const char* name) {
  for (int i=0; i<factions_count; i++)
    if (!str_cmp(name, faction_table[i].name))
      return &faction_table[i];
  return NULL;
}

//FACTION_DATA* get_neutral_faction() {
//  //neutral faction.
//  return &factions[0];
//}

int get_neutral_faction() {
  //neutral faction.
  return 0;
}

int get_faction_id( FACTION_DATA *faction ) {
  for ( int i = 0; i < factions_count; i++ )
    if ( &faction_table[i] == faction )
      return i;
  return -1;
}

int get_faction_id( const char *name ) {
  for (int i=0; i<factions_count; i++)
    if (!str_cmp(name, faction_table[i].name))
      return i;
  return -1;
}

int get_race_faction_id( const int raceId ) {
  for ( int i = 0; i < factions_count; i++ )
    for ( int j = 0; j < faction_table[i].nb_races; j++ )
      if ( faction_table[i].races[j] == raceId )
	return i;
  return -1;
}

FACTION_DATA *get_race_faction( const int raceId ) {
  for ( int i = 0; i < factions_count; i++ )
    for ( int j = 0; j < faction_table[i].nb_races; j++ )
      if ( faction_table[i].races[j] == raceId )
	return &faction_table[i];
  return NULL;
}


// Update aggressor's faction when killing victim
void update_faction_on_aggressive_move( CHAR_DATA *aggressor, CHAR_DATA *victim ) {
  if ( IS_NPC( aggressor ) )
    return;
  if ( !IS_NPC(victim) )
    return;

  // aggressor: PC   victim: NPC
  int *aggressorFaction = aggressor->pcdata->base_faction_friendliness;
  FACTION_DATA *victimFaction = &(faction_table[victim->faction]);

#ifdef VERBOSE  
  log_stringf("Aggr: %s  vict: %s (%s)", NAME(aggressor), NAME(victim), victimFaction->name );
#endif

  //if ( victimFaction == &faction_table[0] ) // if faction is neutral, no need to compute new values
  if ( victimFaction == &faction_table[get_neutral_faction()] )
    return;

  for ( int i = 0; i < factions_count; i++ ) {
    float delta = factionUpdateRate * ( victimFaction->friendliness[i] );

#ifdef VERBOSE
    log_stringf("Fact: %s [%4.4f]  %d -> %d", 
		faction_table[i].name,
		delta, aggressorFaction[i], (int)((float)aggressorFaction[i] - delta));
#endif

    aggressorFaction[i] = range_faction((int)((float)aggressorFaction[i] - delta));
  }
}

// Update niceGuy's faction when doing a nice action
void update_faction_on_nice_move( CHAR_DATA *niceGuy, CHAR_DATA *helped, const int niceness ) {
  if ( IS_NPC( niceGuy ) )
    return;
  if ( !IS_NPC(helped) )
    return;
  // niceGuy: PC   helped: NPC
  int *niceGuyFaction = niceGuy->pcdata->base_faction_friendliness;
  //FACTION_DATA *helpedFaction = helped->faction;
  FACTION_DATA *helpedFaction = &(faction_table[helped->faction]);
  int id = get_faction_id(helpedFaction);

  niceGuyFaction[id] = range_faction((int)( niceGuyFaction[id] + factionUpdateRate * niceness));
}

// if simple is true, just compute faction hates without any test
bool check_aggro_faction( CHAR_DATA *aggressor, CHAR_DATA *victim, const bool simple ) {
  if ( !simple ) {
    if ( aggressor == victim )
      return FALSE;
    
    if ( !IS_NPC( aggressor ) ) // player can't aggro
      return FALSE;
    
    //if ( victim->fighting != NULL || aggressor->fighting != NULL ) // don't aggro while fighting
    //  return;
    if ( aggressor->fighting != NULL ) // don't aggro wile fighting
      return FALSE;
    
    //  if ( aggressor->faction == &factions[0] ) // neutral faction doesn't aggro
    if ( aggressor->faction == get_neutral_faction() ) // neutral faction doesn't aggro
      return FALSE;
    
    if ( !can_see( aggressor, victim ) )
      return FALSE;
    
    // if victim can't attack aggressor
    // or aggressor can't attack victim  -->  no aggro
    if ( silent_is_safe( victim, aggressor )
	 || silent_is_safe( aggressor, victim) )
      return FALSE;
  }

  int aggressorValue;
  //int aggressorId = get_faction_id(aggressor->faction);
  int aggressorId = aggressor->faction;
  if ( IS_NPC(victim) )
    aggressorValue = faction_table[victim->faction].friendliness[aggressorId];
  else
    aggressorValue = victim->pcdata->current_faction_friendliness[aggressorId];

#ifdef VERBOSE
  log_stringf("check_aggro [%d]: aggr: [%s](%s)  victim: [%s](%s) --> (%d)",
	      aggressor->in_room->vnum,
	      NAME(aggressor),
	      faction_table[aggressorId].name,
	      NAME(victim),
	      IS_NPC(victim)?faction_table[victim->faction].name:"player",
	      aggressorValue );
#endif

  if ( aggressorValue <= AGGR_FACTION_VALUE ) {
#ifdef VERBOSE
    log_stringf("check_aggro -> AGGRO'ED");
#endif
    //multi_hit( aggressor, victim, TYPE_UNDEFINED );
    return TRUE;
  }
  return FALSE;
}


// *********************************
// *************** FACTION LOAD/SAVE

//Faction <STRING Faction Name> {
//  Friendliness = <LIST OF LIST> ( ( <STRING Faction Name>, <INTEGER Value> ), ...);
//}
void new_save_factions() {
  FILE *fp;

  log_stringf("Saving factions");

  fclose( fpReserve );
  if ( ( fp = fopen( FACTION_FILE, "w" ) ) == NULL ) {
    bug("Can't open faction file: %s", FACTION_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  // we start to count from faction 1, don't need to save neutral faction
  for ( int i = 1; i < factions_count; i++ ) {
    char buf[MAX_STRING_LENGTH];
    buf[0] = '\0';
    for ( int j = 1; j < factions_count; j++ ) {
      char buf2[MAX_STRING_LENGTH];
      sprintf( buf2, "( %s, %d )", 
	       quotify(faction_table[j].name,"'"), faction_table[i].friendliness[j] );
      strcat( buf, buf2 );
      if ( j < factions_count-1 )
	strcat( buf, ", ");
    }
    fprintf(fp,
	    "Faction %s {\n"
	    "  Friendliness = (%s);\n"
	    "}\n\n",
	    quotify(faction_table[i].name,"'"),
	    buf );
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  factionnotsaved = FALSE;
}

void new_load_factions() {
  FILE *fp;
  // First faction is the neutral faction
  faction_table[0].name = str_dup("Neutral");

  log_stringf("Reading factions");

  fclose( fpReserve );
  if ( ( fp = fopen( FACTION_FILE, "r" ) ) == NULL ) {
    bug("Can't open faction file: %s", FACTION_FILE );
    factions_count = 1;
    faction_table[0].friendliness[0] = NEUTRAL_FACTION_VALUE;
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );

  // nullify first line/column for neutral faction
  for ( int i = 0; i < factions_count; i++ ) {
    faction_table[0].friendliness[i] = NEUTRAL_FACTION_VALUE; // nullify first line
    faction_table[i].friendliness[0] = NEUTRAL_FACTION_VALUE; // nullify first column
  }
  faction_table[0].nb_races = 1;
  if ( ( faction_table[0].races = (int*)GC_MALLOC( 1 * sizeof(int) ) ) == NULL )
    p_error("Can't allocate memory for factions_table[0].races");
  faction_table[0].races[0] = race_lookup("unique");

  // check that every races is in a faction
  for ( int i = 0; i < MAX_RACE; i++ )
    if ( get_race_faction_id( i ) == -1 )
      bug("Race %s is not considered in any faction.", race_table[i].name );

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  factionnotsaved = FALSE;
}

// We extract faction name from module before extracting the other informations
//  because other (such Friendliness) use faction name
void create_faction_table( const int count, DATAModule *module ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  // No allocation, faction_table maximum size is static
  // But we nullify the matrix
  for ( int i = 0; i < count+1; i++ ) { // +1 for the neutral faction
    for ( int j = 0; j < count+1; j++ )
      faction_table[i].friendliness[j] = NEUTRAL_FACTION_VALUE;
    faction_table[i].nb_races = 0;
    faction_table[i].races = NULL;
  }
  factions_count = count+1;
  for ( int i = 0; i < count; i++ ) // extract every faction name from file
    faction_table[i+1].name = str_dup( module->datas[i]->value->eval().asStr() );
  done = TRUE;
}

void parse_friendliness( DATAData *friends, faction_data *faction ) {
  ValueList *list = friends->value->eval().asList();

  if ( list->size > MAX_FACTION )
    p_error("Too many friends found for faction %s, max value is %d.", 
	    faction->name, MAX_FACTION-1 );

  for ( int i = 0; i < list->size; i++ ) {
    ValueList *couple = list->elems[i].asList();
    if ( couple->size != 2 )
      p_error("Wrong number of elements for couple (<Faction Name>, <Faction Value>), for faction %s", faction->name );
    int factionId = get_faction_id( couple->elems[0].asStr() );
    if ( factionId <= -1 )
      p_error("Wrong faction name %s for faction %s", couple->elems[0].asStr(), faction->name );
    faction->friendliness[factionId] = couple->elems[1].asInt();
    if ( faction->friendliness[factionId] < MIN_FACTION_VALUE 
	 || faction->friendliness[factionId] > MAX_FACTION_VALUE ) {
      bug("Faction value (%d) out of range for faction %s/%s",
	  faction->friendliness[factionId], faction->name, faction_table[factionId].name );
      faction->friendliness[factionId] = range_faction( faction->friendliness[factionId] );
    }
    if ( DATA_VERBOSE > 2 ) {
      printf("    %10s: %6d\n\r", faction_table[factionId].name, faction->friendliness[factionId] );
    }

    couple->explicit_free();
  }

  list->explicit_free();
}
void parse_faction( DATAData *fact ) {
  static int index = 1;
  if ( index >= MAX_FACTION )
    p_error("Too many factions found, max value is %d.", MAX_FACTION-1 );

  faction_data *faction = &(faction_table[index]);
  //faction->name = str_dup( fact->value->eval().asStr() ); already done in create_faction_table

  if ( DATA_VERBOSE > 0 ) {
    printf("Faction %s\n\r", faction->name );
  }

  // fields
  for ( int i = 0; i < fact->fields_count; i++ ) {
    DATAData *field = fact->fields[i];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 2 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
    case TAG_Races: {
      ValueList *list = field->value->eval().asList();
      if ( list->size == 0 ) {
	bug("Invalid races list size (0) for faction %s", faction->name );
	break;
      }
      faction->nb_races = list->size;
      if ( ( faction->races = (int*)GC_MALLOC( faction->nb_races * sizeof(int) ) ) == NULL )
	p_error("Can't allocate memory for faction_table.races[]");
      for ( int i = 0; i < faction->nb_races; i++ ) {
	const char *s = list->elems[i].asStr();
	if ( ( faction->races[i] = race_lookup( s, TRUE ) ) < 0 )
	  bug("Invalid race name %s for faction %s, assuming none", s, faction->name );
      }

      list->explicit_free();
      break;
    }
    case TAG_Friendliness: parse_friendliness( field, faction ); break;
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
  index++;
}
