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
#include "magic.h"
#include "tables.h"
#include "db.h"
#include "lookup.h"
#include "dbdata.h"
#include "config.h"
#include "prereqs.h"
#include "const.h"


// Added by SinaC 2000 for prereqs, modified by SinaC 2003

//#define VERBOSE
//#define VERBOSE2

// Added by SinaC 2000 for ability level this constants is only used to
//  get a upper bound of the number of nodes in a prereq graph
#define MAX_CASTING_LEVEL (5)

//******************************************* NEW PREREQS ***************************************************
// Examples: 
// Fireball has 5 casting levels and prereqs for casting level 3 to 5
//   0: None
//   1: None
//   2: None
//   3: Faerie Fire
//   4: Faerie Fire Level 2
//   5: Burning Hands Level 2, Flamestrike Level 2
// Magic mirror has no casting level but prereqs in order to gain it
//   0: Colour Spray Level 2, Gate, Dispel Magic




//******************** GRAPH STRUCTURE TO STORE PREREQ ****************************/

#define MAX_NEXT   (16)

struct graph {
  
  int nb_node; // number node
  //const char **name_node; // node name
  int *sn_node; // node ability number
  int *casting_node; // node casting level
  
  int *nb_next; // number next for each node
  int **next; // next for each node
  
  // maximum n node and m next per node
  graph(const int nnode, const int nnext);
  ~graph();

  const char *node_name( const int i ) const;
  //int get_node( const char *name ) const;
  int get_node( const int sn, const int casting ) const;
  //void add_node( const char *name );
  int add_node( const int sn, const int casting );
  bool search_next( const int iFrom, const int iTo );
  //void add_next_create( const char *from, const char *to );
  void add_next_create( const int fromSn, const int fromCasting, 
			const int toSn, const int toCasting );

  void dump() const;
};

graph::graph(const int nnode, const int nnext) {
  nb_node = 0;
  //name_node = (const char**) GC_MALLOC(nnode* sizeof( char*));
  sn_node = (int*) GC_MALLOC_ATOMIC(nnode* sizeof(int));
  memset( sn_node, 0, sizeof(int)*nnode);
  casting_node = (int*) GC_MALLOC_ATOMIC(nnode* sizeof(int));
  memset( casting_node, 0, sizeof(int)*nnode);

  nb_next = (int*) GC_MALLOC_ATOMIC(nnode* sizeof( int));
  next = (int**) GC_MALLOC(nnode* sizeof( int*));
  for ( int i = 0; i < nnode; i++ ) {
    //name_node[i] = NULL;
    nb_next[i] = 0;
    next[i] = (int*) GC_MALLOC(nnext* sizeof( int ));
    for ( int j = 0; j < nnext; j++ )
      next[i][j] = 0;
  }
}

graph::~graph() {
}

void graph::dump() const {
  printf("dumping...  %d nodes\n", nb_node);

  for ( int i = 0; i < nb_node; i++ ) {
    //printf("%d) %s\n", i, name_node[i]);
    printf("%d) %s\n", i, node_name(i) );
    for ( int j = 0; j < nb_next[i]; j++ )
      //printf("  (%s)", name_node[next[i][j]]);
      printf("  (%s)", node_name(next[i][j]));
    if ( nb_next[i] == 0 )
      printf("  no next");
    printf("\n");
  }
}

const char *graph::node_name( const int i ) const {
  if ( i >= nb_node )
    return "ERROR";
  char buf[MAX_STRING_LENGTH];
  sprintf( buf, "%s[%d]", ability_table[sn_node[i]].name, casting_node[i]);
  return str_dup(buf);
}

//int graph::get_node( const char *name ) const {
//  for ( int i = 0; i < nb_node; i++ )
//    if ( !str_cmp( name, name_node[i] ) )
//      return i;
//
//  return -1;
//}
int graph::get_node( const int sn, const int casting ) const {
  for ( int i = 0; i < nb_node; i++ )
    if ( sn == sn_node[i] 
	 && casting == casting_node[i] )
      return i;
  return -1;
}

//void graph::add_node( const char *name ) {
//  // already in the graph ?
//  if ( get_node( name ) >= 0 ) {
//    printf("node '%s' already exits\n", name );
//    return;
//  }
//
//#ifdef VERBOSE
//  printf("  Adding '%s'\n", name);
//#endif
//
//  //name_node[nb_node] = str_dup(name);
//  name_node[nb_node] = name;
//  nb_node++;
//}
int graph::add_node( const int sn, const int casting ) {
  int i = get_node( sn, casting );
  if ( i >= 0 ) {
    printf("Node %s already exists\n", node_name(i) );
    return nb_node;
  }

  sn_node[nb_node] = sn;
  casting_node[nb_node] = casting;
  nb_node++;
  return nb_node-1;
}

bool graph::search_next( const int iFrom, const int iTo ) {
  for ( int i = 0; i < nb_next[iFrom]; i++ )
    if ( next[iFrom][i] == iTo )
      return TRUE;
  return FALSE;
}

//void graph::add_next_create( const char *from, const char *to ) {
//  int iFrom, iTo;
//  iFrom = get_node( from ); 
//  iTo = get_node( to );
//  if ( iFrom < 0 ) {
//    add_node( from );
//    iFrom = get_node( from );
//  }
//  if ( iTo < 0 ) {
//    add_node( to );
//    iTo = get_node( to );
//  }
//  if ( search_next( iFrom, iTo ) == TRUE ) {
//    printf("edge '%s' -> '%s' already exists\n",
//    	   name_node[iFrom], name_node[iTo] );
//    return;
//  }
//#ifdef VERBOSE
//  printf("  Linking '%s' and '%s'\n", name_node[iFrom],name_node[iTo]);
//#endif
//
//  next[iFrom][nb_next[iFrom]] = iTo;
//  nb_next[iFrom]++;
//}

void graph::add_next_create( const int fromSn, 
			     const int fromCasting,
			     const int toSn,
			     const int toCasting) {
  int iFrom = get_node( fromSn, fromCasting );
  if ( iFrom < 0 )
    iFrom = add_node( fromSn, fromCasting );
  int iTo = get_node( toSn, toCasting );
  if ( iTo < 0 )
    iTo = add_node( toSn, toCasting );
  if ( search_next( iFrom, iTo ) == TRUE ) {
    printf("edge %s -> %s already exists\n",
    	   node_name(iFrom), node_name(iTo) );
    return;
  }
#ifdef VERBOSE
  printf("  Linking %s and %s\n", node_name(iFrom), node_name(iTo));
#endif

  next[iFrom][nb_next[iFrom]] = iTo;
  nb_next[iFrom]++;
}

bool *mark;
bool *mark2;
int lastSn;
int lastCasting;
int startSn;
int startCasting;

bool isCyclic( graph &g, int node ) {
#ifdef VERBOSE
  printf("node: %d  (%s)  nb_next: %d", 
	 node,
	 g.node_name(node),
	 g.nb_next[node]);
  printf("  next: ");
  for ( int i = 0; i < g.nb_next[node]; i++ )
    printf("   %s %s",
	   g.node_name(g.next[node][i]),
	   mark[g.next[node][i]]?"T":"F");
  printf("\n\r");
#endif

  mark[node] = TRUE;
  mark2[node] = TRUE;

  for ( int i = 0; i < g.nb_next[node]; i++ ) {
    if ( mark[g.next[node][i]] )
      return TRUE;
    if ( isCyclic( g, g.next[node][i] ) )
      return TRUE;
    else {
      lastSn = g.sn_node[i];
      lastCasting = g.casting_node[i];
    }
  }
  
  mark[node] = FALSE;
  return FALSE;
}

// ************************** Check prerequisites: acyclic, impossible prereqs, ...
bool add_edge( graph &g, const int fromSn, const int fromCasting, const int toSn, const int toCasting ) {
  bool error = FALSE;
  if ( toCasting > ability_table[toSn].nb_casting_level ) {
    bug("%s[%d] needs %s[%d] but has only [%d] casting levels",
	ability_table[fromSn].name, fromCasting,
	ability_table[toSn].name, toCasting,
	ability_table[toSn].nb_casting_level );
    error = TRUE;
  }
  g.add_next_create( fromSn, fromCasting, toSn, toCasting );
  return error;
}
bool add_ability_in_graph( graph &g, const int i, 
			   const int addFromCasting, const int addToSn, const int addToCasting ) {
  bool error = FALSE;
  int precSn = -1;
  int precCasting = -1;
  ability_type *skFrom = &(ability_table[i]);
  if ( skFrom->prereqs != NULL ) { // ability has prereqs
    for ( int j = 0; j < skFrom->nb_casting_level+1; j++ ) { // +1 because of level 0
      prereq_data *p = &(skFrom->prereqs[j]);
      for ( int k = 0; k < p->nb_prereq; k++ ) {
	int snTo = p->prereq[k].sn;
	ability_type *skTo = &(ability_table[snTo]);
	error |= add_edge( g, i, j, snTo, p->prereq[k].casting_level );
      }
      if ( precSn >= 0 )
	g.add_next_create( i, j, precSn, precCasting );
      precSn = i;
      precCasting = j;
    }
  }
  if ( addFromCasting >= 0 ) // if an additional prereq was specified
    error |= add_edge( g, i, addFromCasting, addToSn, addToCasting );
  return error;
}
//void check_prerequisites() {
//  graph g(MAX_ABILITY*(MAX_CASTING_LEVEL+1), MAX_NEXT);
//  // should improve this size by computing how many nodes we need
//  bool error = FALSE;
//
//  log_stringf("Checking prereqs...");
//
//  for ( int i = 0; i < MAX_ABILITY; i++ )
//    error |= add_ability_in_graph( graph, &(ability_table[i]) );
//  // For all abilities
//  for ( int i = 0; i < MAX_ABILITY; i++ ) {
//    ability_type *sk = &(ability_table[i]);
//    char prec[512];
//    prec[0] = '\0';
//    // If skill has prereqs
//    if ( sk->prereqs != NULL )
//      // For every casting level of that skill
//      for ( int j = 0; j < (sk->nb_casting_level+1); j++ ) {
//	prereq_data *p = &(sk->prereqs[j]);
//	char from[512];
//	sprintf( from, 
//		 "%s %d",
//		 ability_table[i].name,
//		 //p->casting_level);
//		 j); // SinaC 2003
//
//	// For every prereq of that casting level
//	for ( int k = 0; k < p->nb_prereq; k++ ) {
//	  char to[512];
//	  sprintf( to, 
//		   "%s %d",
//		   ability_table[p->prereq[k].sn].name,
//		   p->prereq[k].casting_level);
//	  int needed_lvl = UMAX(ability_table[p->prereq[k].sn].nb_casting_level,1);
//	  if ( p->prereq[k].casting_level >  needed_lvl ) {
//	    bug("%s needs '%s' but has only '%d' casting levels",
//		from, to, 
//		//ability_table[p->prereq[k].sn].nb_casting_level );
//		needed_lvl );
//	    error = TRUE;
//	  }
//
//	  g.add_next_create( from, to );
//	}
//	if ( prec[0] != '\0' ) {
//	  g.add_next_create( from, prec );
//	}
//	strcpy( prec, from );
//      }
//  }
//
//#ifdef VERBOSE
//  g.dump();
//#endif
//
//  // Now we check cyclic prereqs
//  mark = (bool *) GC_MALLOC_ATOMIC(g.nb_node * sizeof(bool));
//  memset( mark, 0, g.nb_node * sizeof(bool));
//  mark2 = (bool *) GC_MALLOC_ATOMIC(g.nb_node * sizeof(bool));
//  memset( mark2, 0, g.nb_node * sizeof(bool));
//  int node;
//  bool cyclic;
//  for ( node = 0; node < g.nb_node; node++ ) {
//    if ( mark2[node] == FALSE ) {
//      cyclic = isCyclic( g, node );
//#ifdef VERBOSE
//      printf("  cyclic: %s\n\r", cyclic?"True":"False");
//#endif
//      if ( cyclic )
//	break;
//    }
//  }
//  if ( cyclic ) {
//    bug("Cyclic prereq found ending with %s", 
//	g.node_name(last));
//    error = TRUE;
//  }
//  if ( !error )
//    log_stringf("Everything is okay with prereqs.");
//  else
//    log_stringf("Errors found in prereqs!!!");
//}

// return 0 if prereq are okay    00
//        1 if cycle              01
//        2 if error              10
//        3 if error & cycle      11
int check_prerequisites_cycle( const int fromSn, const int fromCasting, 
			       const int toSn, const int toCasting ) {
  graph g(MAX_ABILITY*(MAX_CASTING_LEVEL+1), MAX_NEXT);
  bool error = FALSE;

  for ( int i = 0; i < MAX_ABILITY; i++ )
    if ( fromSn == i )  // if an additional node has been specified
      error |= add_ability_in_graph( g, i, fromCasting, toSn, toCasting );
    else
      error |= add_ability_in_graph( g, i, -1, -1, -1 );

#ifdef VERBOSE
  g.dump();
#endif

  mark = (bool *) GC_MALLOC_ATOMIC(g.nb_node * sizeof(bool));
  memset( mark, 0, g.nb_node * sizeof(bool));
  mark2 = (bool *) GC_MALLOC_ATOMIC(g.nb_node * sizeof(bool));
  memset( mark2, 0, g.nb_node * sizeof(bool));
  int node;
  bool cyclic;
  for ( node = 0; node < g.nb_node; node++ )
    if ( mark2[node] == FALSE ) {
#ifdef VERBOSE
      printf("   starting with %s\n\r", g.node_name(node));
#endif
      startSn = g.sn_node[node];
      startCasting = g.casting_node[node];
      if ( isCyclic( g, node ) )
	if ( error )
	  return 3;
	else
	  return 1;
    }
  if ( error )
    return 2;
  return 0;
}

void check_prerequisites() {
  log_stringf("Checking prereqs...");

  int error = check_prerequisites_cycle( -1, -1, -1, -1 );

  switch( error ) {
  case 0: log_stringf("Everything is okay with prereqs."); break;
  case 1: case 3: log_stringf("Cycling prerequisites starting %s[%d] and ending with %s[%d].",
			      ability_table[startSn].name, startCasting, 
			      ability_table[lastSn].name, lastCasting ); break;
  case 2: log_stringf("Problems with prerequisites."); break;
  }
}

//**************************** SAVE and LOAD prerequisites

void new_save_prerequisites() {
  FILE *fp;

  log_string("Saving prerequisites");

  fclose(fpReserve);
  if (!(fp = fopen ( PREREQ_FILE, "w"))){
    bug("Could not open file %s in order to load prerequisites.", PREREQ_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ability_type *ability = &(ability_table[i]);
    if ( ability->name == NULL )
      break;
    if ( ability->prereqs != NULL ) {
//Prereq <STRING Ability Name> {
//  NumberCasting = <INTEGER Number of ability casting level>;
//  Casting <INTEGER Casting level> {
//    Level = <INTEGER Player level to get that casting level>;
//    Cost = <INTEGER Train cost>;
//    List = <LIST OF LIST> ( ( <STRING Ability Name>, <INTEGER Casting level> ), ... );
//  }
//}
      fprintf(fp,
	      "Prereq '%s' {\n"
	      "  NumberCasting = %d\n", 
	      ability->name, ability->nb_casting_level );
      for ( int j = 0; j < ability->nb_casting_level+1; j++ ) {
	PREREQ_DATA *prereq = &(ability->prereqs[j]);
	if ( prereq->cost != DEFAULT_PREREQ_COST(j)
	     || prereq->plr_level != 1
	     || prereq->nb_prereq != 0 ) {
	  fprintf(fp,"Casting %d {\n",j);
	  if ( prereq->cost != DEFAULT_PREREQ_COST(j) ) fprintf(fp, "  Cost = %d;\n", prereq->cost );
	  if ( prereq->plr_level != 0 ) fprintf(fp,"  Level = %d;\n", prereq->plr_level );
	  if ( prereq->nb_prereq != 0 ) {
	    char buf[MAX_STRING_LENGTH];
	    buf[0] = '\0';
	    for ( int k = 0; k < prereq->nb_prereq; k++ ) {
	      char buf2[MAX_STRING_LENGTH];
	      sprintf(buf2,"(%s,%d)", ability_table[prereq->prereq[k].sn].name, prereq->prereq[k].casting_level);
	      strcat( buf, buf2);
	      if ( k < prereq->nb_prereq-1 )
		strcat( buf, ",");
	    }
	    fprintf(fp,"  List = (%s);\n",buf);
	  }
	  fprintf(fp,"}");
	}
      }
      fprintf(fp,"}\n");
    }
  }
  
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_load_prerequisites() {
  FILE *fp;

  log_string("Reading prerequisites");

  fclose(fpReserve);
  if (!(fp = fopen ( PREREQ_FILE, "r"))){
    bug("Could not open file %s in order to load prerequisites.", PREREQ_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  int count = parse_datas( fp );
  log_stringf(" %d prerequisites found.", count );

  if ( count != 0 )
    check_prerequisites();
  
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );
}


//Prereq <STRING Ability Name> {
//  NumberCasting = <INTEGER Number of ability casting level>;
//  Casting <INTEGER Casting level> {
//    Level = <INTEGER Player level to get that casting level>;
//    Cost = <INTEGER Train cost>;
//    List = <LIST OF LIST> ( ( <STRING Ability Name>, <INTEGER Casting level> ), ... );
//  }
//}

void parse_prereqList( DATAData *prereqList, PREREQ_DATA *prereq ) {
  // value: ( ( <Ability Name>, <Casting Level> ), ... )   <List of 2-uple>

  ValueList *list = prereqList->value->eval().asList();

  prereq->nb_prereq = list->size;
if ( DATA_VERBOSE > 2 ) {
  printf("   nb prereq: %d\n\r", prereq->nb_prereq );
}
  if ( prereq->nb_prereq == 0 )
    p_error("list of prereq is empty");

  if ( ( prereq->prereq = (PREREQ_LIST *)GC_MALLOC( prereq->nb_prereq * sizeof( PREREQ_LIST ) ) ) == NULL )
    p_error("Can't allocate memory");

  for ( int i = 0; i < list->size; i++ ) {
    ValueList *couple = list->elems[i].asList();
    PREREQ_LIST *pList = &(prereq->prereq[i]);
    if ( couple->size != 2 )
      p_error("wrong number of elements in couple <Ability Name>, <Casting Level>");
    const char *abilityName = couple->elems[0].asStr();
    int sn = ability_lookup( abilityName );
    if ( sn < 0 )
      p_error("Invalid ability name: %s", abilityName );
    pList->sn = sn;
    pList->casting_level = couple->elems[1].asInt();
if ( DATA_VERBOSE > 2 ) {
    printf("  prereq #%d: ability: %s  casting level: %d\n\r", i, abilityName, pList->casting_level );

    couple->explicit_free();
}
  }

  list->explicit_free();
}

void parse_oneCastingLevel( DATAData *onePrereq, ability_type *ability ) {
  // tag: Casting
  // value: <Casting Level>
  // fields: Cost = <Train Cost>
  // fields: Level = <Player level to gain that casting level>
  // fields: List = <List of 2-uple ( <Ability Name>, <Casting Level> ) >

  int castingLevel = onePrereq->value->eval().asInt();
  if ( castingLevel < 0 || castingLevel > ability->nb_casting_level )
    p_error("Invalid Casting Level: %d (max: %d)", castingLevel, ability->nb_casting_level);
if ( DATA_VERBOSE > 2 ) {
  printf(" Casting level: %d\n\r", castingLevel );
}
  PREREQ_DATA *prereq = &(ability->prereqs[castingLevel]);

  for ( int fieldCount = 0; fieldCount < onePrereq->fields_count; fieldCount++ ) {
    DATAData *field = onePrereq->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {

      // Cost
    case TAG_Cost:
      prereq->cost = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("  Cost: %d\n\r", prereq->cost );
}
      break;

      // Level
    case TAG_Level:
      prereq->plr_level = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("  Level: %d\n\r", prereq->plr_level );
}
      break;

      // List of prereq
    case TAG_List:
      parse_prereqList( field, prereq );
      break;

    case TAG_Classes: {
      int classes = list_flag_value( field->value, classes_flags );
      if ( classes == NO_FLAG )
	bug("Invalid classes for ability %s casting %d", ability->name, castingLevel );
      else
	prereq->classes = classes;
      break;
    }

      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
}

void parse_prerequisite( DATAData *prereq ) { // called by  dbdata.C:parse_datas()
  // tag:    Prereq
  // value:  <Ability Name>
  // fields: CastingLevel = <Number Casting Level>;
  // fields: Casting <Casting Level >= <List of Prereq>

  // value
  const char *abilityName = prereq->value->eval().asStr();
  int sn = ability_lookup( abilityName );
  if ( sn < 0 )
    p_error("Invalid ability: %s", abilityName );
  ability_type *ability = &(ability_table[sn]);
 
if ( DATA_VERBOSE > 0 ) {
  printf("Prerequisites: %s\n\r", ability->name );
}
  // fields
  for ( int fieldCount = 0; fieldCount < prereq->fields_count; fieldCount++ ) {
    DATAData *field = prereq->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {

      // Number of casting level
    case TAG_NumberCasting: {
      int nbCastingLevel = field->value->eval().asInt();
      if ( nbCastingLevel != ability->nb_casting_level )
	bug("%s has invalid nb_casting_level: %d in file but %d in ability_table_init",
	    ability->name, nbCastingLevel, ability->nb_casting_level );
if ( DATA_VERBOSE > 2 ) {
      printf("#Casting level: %d\n\r", nbCastingLevel );
}
      ability->nb_casting_level = nbCastingLevel;
      //+1 for level 0: prereqs needed to gain that ability
      if ((ability->prereqs = (PREREQ_DATA *)GC_MALLOC((ability->nb_casting_level+1)*sizeof(PREREQ_DATA)))==NULL)
	p_error("Can't allocate memory");
      for ( int j = 0; j < nbCastingLevel+1; j++ ) {
	ability->prereqs[j].cost = DEFAULT_PREREQ_COST(j);
	ability->prereqs[j].plr_level = 1;
	ability->prereqs[j].prereq = NULL;
	ability->prereqs[j].nb_prereq = 0;
	ability->prereqs[j].classes = ANY_CLASSES; // available for all classes
      }
      break;
    }

      // List of prereq for each casting level
    case TAG_Casting: 
      if ( ability->prereqs == NULL )
	p_error("Number of casting level not known.");
      parse_oneCastingLevel( field, ability );
      break;

      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }
}

