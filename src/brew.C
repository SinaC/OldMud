// Added by SinaC 2003
/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "merc.h"
#include "olc_value.h"
#include "act_comm.h"
#include "handler.h"
#include "gsn.h"
#include "lookup.h"
#include "db.h"
#include "comm.h"
#include "brew.h"
#include "interp.h"
#include "dbdata.h"
#include "ability.h"
#include "config.h"
#include "tables.h"
#include "save.h"
#include "bit.h"
#include "olc.h"
#include "olc_act.h"
#include "utils.h"


bool brewnotsaved;
bool brewcomponentnotsaved;
int MAX_BREW_FORMULA;
int MAX_BREW_COMPONENT;
brew_formula_type *brew_formula_table;

// Return brew formula id
int brew_lookup( const char *name ) {
  for ( int i = 0; i < MAX_BREW_FORMULA; i++ )    
    if (LOWER(name[0]) == LOWER(brew_formula_table[i].name[0])
	&&  !str_prefix( name, brew_formula_table[i].name))
      return i;
  return -1;
}

// FIXME2: instead of creating a potion, try to find an empty potion in the inventory
// FIXME: COMPLEX_FORMULA must be defined or not ???
//IF COMPLEX_FORMULA is defined AND more than 1 formula match the component THEN a mixed potion is created
//  with the most present effect and the highest min cost  !!!! but the problem is the name of the potion
//IF COMPLEX_FORMULA is NOT defined AND .. THEN a random formula is used
//#define COMPLEX_FORMULA
struct effect_info {
  int sn;
  int power;
};
// compare power, if same then compare min cost
static int compare( const void *a, const void *b ) {
  effect_info e1 = (effect_info)(*((const effect_info*)a));
  effect_info e2 = (effect_info)(*((const effect_info*)b));
  if ( e1.power > e2.power )
    return -1;
  if ( e1.power < e2.power )
    return 1;
  if ( ability_table[e1.sn].min_cost > ability_table[e2.sn].min_cost )
    return -1;
  if ( ability_table[e1.sn].min_cost < ability_table[e2.sn].min_cost )
    return 1;
  return 0;
  //  int i1 = ability_table[(int)(*((const int*)a))].min_cost;
  //  int i2 = ability_table[(int)(*((const int*)b))].min_cost;
  //  if ( i1 < i2 ) return -1;
  //  if ( i1 > i2 ) return 1;
  //  return 0;
}
void do_brew( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  // Vector with the number of each component in the brewed item
  int component_list[MAX_BREW_COMPONENT];
  memset( component_list, 0, MAX_BREW_COMPONENT * sizeof(int) );
  // Vector with brewed item, we can't use more than 2 times the number of components allowed in a formula
  OBJ_DATA *obj_list[MAX_COMPONENT*2];
  // Vector with name of brewed item
  char obj_name[MAX_COMPONENT*2][MAX_STRING_LENGTH];
  int obj_count[MAX_COMPONENT*2];
  int num_brewed = 0;

  if ( argument == NULL || argument[0] == '\0' ) {
    send_to_char("Brew what ?\n\r", ch );
    return;
  }

  int chance;
  if ( ( chance = get_ability( ch, gsn_brew ) ) == 0 ) {
    send_to_char("You don't know how to brew potion.\n\r", ch );
    return;
  }

  // Parse argument to find name of brewed item
  for ( int i = 0; i < MAX_COMPONENT*2; i++ ) {
    arg[0] = '\0'; name[0] = '\0';
    argument = one_argument( argument, arg );
    int number = number_argument( arg, name );
    if ( name != NULL && name[0] != '\0' ) {
      strcpy( obj_name[num_brewed], name );
      obj_count[num_brewed] = number;
      //      send_to_charf(ch,"->[%s] [%d]\n\r", obj_name[num_brewed], obj_count[num_brewed] );
      num_brewed++;
      if ( num_brewed >= 2*MAX_COMPONENT ) {
	send_to_charf(ch,"Too many components used, maximum %d are allowed.\n\r", 2*MAX_COMPONENT );
	return;
      }
    }
    else
      break;
  }

  if ( num_brewed == 0 ) {
    send_to_charf(ch,"You must specify a list of components.\n\r" );
    return;
  }

  // Get list of brewed item
  int num_item = 0;
  bool stop = FALSE;
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content )
    if ( obj->wear_loc == WEAR_NONE          // item in inventory
	 && obj->item_type == ITEM_COMPONENT // item is a component
	 && can_see_obj( ch, obj ) ) {       // see item
      for ( int i = 0; i < num_brewed; i++ )     // search in argument list
	if ( is_name( obj_name[i], obj->name )       // name is the same
	     && --obj_count[i] == 0 ) {              // count is good
	  obj_list[num_item++] = obj;                    // store item
	  if ( num_item == num_brewed ) {                // get the right number of item -> stop
	    stop = TRUE;
	    break;
	  }
      }
      if ( stop )
	break;
    }
  if ( num_brewed != num_item ) {// not the right number of item -> error
    for ( int i = 0; i < num_brewed; i++ )
      if ( obj_count[i] > 0 )
	send_to_charf(ch,"Component (%s) not found.\n\r", obj_name[i] );
    return;
  }

  // Get component list from item
  for ( int i = 0; i < num_brewed; i++ ) {
    OBJ_DATA *obj = obj_list[i];
    for ( int j = 0; j < i; j++ )
      if ( obj_list[j] == obj ) {
	send_to_charf(ch,
		      "Component (%s) duplicated.\n\r"
		      "Use 2.%s if you want have 2 times the same item and want to brew both.\n\r",
		      obj_name[j], obj_name[j] );
	return;
      }
    //    send_to_charf(ch,"ITEM: [%s]\n\r", obj->short_descr );
    if ( obj->item_type == ITEM_COMPONENT )
      for ( int j = 0; j < 5; j++ )
	if ( obj->value[j] >= 0
	     && obj->value[j] < MAX_BREW_COMPONENT )
	  component_list[obj->value[j]]++;
  }
  

  // Dump...
  //  for  ( int i = 0; i < MAX_BREW_COMPONENT; i++ )
  //    if ( component_list[i] > 0 )
  //      send_to_charf(ch,"COMPONENT [%s]  -> [%d]\n\r", flag_string(brew_component_flags, i), component_list[i]);

  // Okay, we have a vector with items that will be brewed  (obj_list        size: num_brewed)
  //  and the list of component contained in those items    (component_list  size: MAX_BREW_COMPONENT)

  // Find which formula have the same components as list
  bool formula_list[MAX_BREW_FORMULA];
  memset( formula_list, FALSE, MAX_BREW_FORMULA*sizeof(bool));
  int component_formula[MAX_BREW_COMPONENT];
  bool found = FALSE;
  int exactF = -1;
  int countF = 0;
  for ( int i = 0; i < MAX_BREW_FORMULA; i++ ) {
    brew_formula_type *form = &(brew_formula_table[i]);
    // get the list of component needed for this formula
    memset(component_formula,0,sizeof(int)*MAX_BREW_COMPONENT);
    for ( int j = 0; j < form->num_component; j++ )
      component_formula[form->component_list[j]]++;
    //    send_to_charf(ch,"FORMULA %s\n\r", form->name );
    //    for  ( int j = 0; j < MAX_BREW_COMPONENT; j++ )
    //      if ( component_formula[j] > 0 )
    //    	send_to_charf(ch," == COMPONENT [%s]  -> [%d]\n\r", flag_string(brew_component_flags, j), component_formula[j]);
    // compare with component in list of item
    bool ok = TRUE;
    bool exact = TRUE;
    for ( int j = 0; j < MAX_BREW_COMPONENT; j++ )
      if ( component_list[j] < component_formula[j] ) { // if less component in list than formula -> failed
	ok = FALSE;
	exact = FALSE;
	break;
      }
      else if ( component_list[j] > component_formula[j] )
	exact = FALSE;
    if ( ok ) {
      formula_list[i] = TRUE;
      countF++;
      found = TRUE;
      //      send_to_charf(ch,"->FORMULA (%s)\n\r", form->name );
    }
    if ( exact ) {
      exactF = i;
      found = TRUE;
      //      send_to_charf(ch,"->EXACT FORMULA (%s)\n\r", form->name );
      break; // no need to continue if we have found a perfect match
    }
  }

  // Destroy brewed items
  for ( int i = 0; i < num_brewed; i++ )
    extract_obj(obj_list[i]);

  // Failure test
  // FIXME: chance must be adapted in function of number of component brewed, number of matching formulas, ...
  if ( number_percent() > chance ) {
    send_to_charf(ch,"You failed to brew the potion.\n\r");
    check_improve(ch,gsn_brew,FALSE,4);
    return;
  }

  if ( !found ) { // No formula found -> destroy the brewed items and send nothing happens
    //    send_to_charf(ch,"No formula found.\n\r");
    send_to_charf(ch,"You brew the component%s but nothing happens.\n\r",
		  num_brewed>1?"s":"");
    return;
  }
  // Formula(s) found -> destroy the brewed items and create a potion corresponding to formula(s)
  int effect[MAX_ABILITY]; // construct a vector with every effects formula gives
  memset(effect,0,sizeof(int)*MAX_ABILITY);
  int cost = 0; // sum of formula cost
  int level = 0; // maximum value
  const char *formName; // name of the formula


  if ( exactF >= 0 ) { // perfect match
    brew_formula_type *form = &(brew_formula_table[exactF]);
    //    send_to_charf(ch,"Formula [%s] is a perfect match.\n\r", form->name );
    level = form->level;
    cost = form->cost;
    for ( int j = 0; j < form->num_effect; j++ )
      if ( form->effect_list[j] > 0 )
	effect[form->effect_list[j]]++;
    countF = 1;
    formName = form->name;
  }
  else { // many formulas match
    //int r = number_range(0,countF-1)+1; // get a random formula name
#ifdef COMPLEX_FORMULA
    for ( int i = 0; i < MAX_BREW_FORMULA; i++ )
      if ( formula_list[i] ) {
	brew_formula_type *form = &(brew_formula_table[i]);
	//	send_to_charf(ch,"Formula [%s] matches.\n\r", form->name );
	for ( int j = 0; j < form->num_effect; j++ )
	  if ( form->effect_list[j] > 0 )
	    effect[form->effect_list[j]]++;
	cost += form->cost;
	level = UMAX( level, form->level );
	//if ( --r == 0 )
	formName = form->name; // get the last formula name FIXME: should create a new unique name
      }
#endif
#ifndef COMPLEX_FORMULA
    int r = number_range(0,countF-1); // get a random formula
    brew_formula_type *form = &(brew_formula_table[r]);
    for ( int j = 0; j < form->num_effect; j++ )
      if ( form->effect_list[j] > 0 )
	effect[form->effect_list[j]]++;
    cost = form->cost;
    level = form->level;
    formName = form->name;
    //    send_to_charf(ch,"Random formula [%s] matches.\n\r", form->name );
#endif
    level = level+countF;
  }
  
  //  for ( int i = 0; i < MAX_ABILITY; i++ )
  //    if ( effect[i] > 0 )
  //      send_to_charf(ch,"Giving effect [%s] [%d]\n\r",
  //		    ability_table[i].name,
  //		    effect[i] );
  //  send_to_charf(ch,"Cost [%d]\n\r", cost );
  //  send_to_charf(ch,"Level [%d]\n\r", level );

  // create potion and add effects
  //  if number of effects is <= number of available value (v1 -> v4: 4 effects)
  //  then just copy the effects
  //  else get some affect at random, or get the most important (highest mana cost, highest level, ...)
  OBJ_DATA *potion = create_object( get_obj_index(OBJ_VNUM_EMPTY_POTION), 0 );

  int countE = 0; // count number of effect and list them
  //int effect_list[MAX_BREW_SPELLS*countF];
  //int effect_power[MAX_BREW_SPELLS*countF];
  //memset( effect_list, 0, MAX_BREW_SPELLS*countF*sizeof(int) );
  //memset( effect_power, 0, MAX_BREW_SPELLS*countF*sizeof(int) );
  effect_info effect_list[MAX_BREW_SPELLS*countF];
  memset( effect_list, 0, MAX_BREW_SPELLS*countF*sizeof(effect_list[0]));
  for ( int i = 0; i < MAX_ABILITY; i++ )
    if ( effect[i] > 0 ) {
      effect_list[countE].sn = i;
      effect_list[countE].power = effect[i];
      countE++;
    }
  // IF COMPLEX_FORMULA is not defined THEN following test is TRUE because MAX_BREW_SPELLS is <= 4
  //  if ( countE <= 4 ) { // if less then 4 effects, just copy effects
  //    send_to_charf(ch,"Effects just copied.\n\r");
  //  }
  //  else { // must get some random effects
  if ( countE > 4 ) {
    //    send_to_charf(ch,"Too many effects [%d]\n\r", countE );
    qsort( effect_list, countE, sizeof(effect_list[0]), compare ); // sort effect with power and min cost
    //    for ( int i = 0; i < countE; i++ )
    //      send_to_charf(ch,"SORTED [%s] [%d]\n\r",
    //		    ability_table[effect_list[i].sn].name,
    //		    effect_list[i].power );
    // get the 4 first effect from sorted list
    countE = 4;
  }
  // Copy the effects
  for ( int i = 0; i < countE; i++ ) {
    potion->baseval[1+i] = potion->value[1+i] = effect_list[i].sn;
    // if the effect is found more than once, upgrade level
    level += UMIN(0,(effect_list[i].power-1)*5);
  }

  check_improve(ch,gsn_brew,TRUE,4);

  potion->level = URANGE( 1, number_fuzzy(ch->level-5), 100 );
  potion->cost = cost;
  potion->baseval[0] = potion->value[0] = URANGE( 1, level, 105 );
  char buf[MAX_STRING_LENGTH];
  sprintf( buf, "potion %s", formName );
  potion->name = str_dup( buf );
  sprintf( buf, "A potion of %s", formName );
  potion->short_descr = str_dup( buf );

  act("You brew the component$T and create $p.", ch, potion, num_brewed>1?"s":"", TO_CHAR );

  recompobj(potion);
  obj_to_char(potion,ch);
}

//----------------------------------
// Load/Save brew formula/components

void new_save_brew_components() {
  FILE *fp;

  // Components
  log_stringf("Saving brew component");
  fclose(fpReserve);
  if (!(fp = fopen ( BREWCOMPONENT_FILE, "w"))){
    bug("Could not open file [%s] in order to save brew components.", BREWCOMPONENT_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_BREW_COMPONENT; i++ ) {
    fprintf( fp, "%s\n", brew_component_flags[i].name );
  }
  fprintf( fp, "-1\n");
      
  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  brewcomponentnotsaved = FALSE;
}

void new_save_brew_formula() {
  FILE *fp;

  log_stringf("Saving brew formula");

  fclose(fpReserve);
  if (!(fp = fopen ( BREWFORMULA_FILE, "w"))){
    bug("Could not open file [%s] in order to save brew formulas.", BREWFORMULA_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for ( int i = 0; i < MAX_BREW_FORMULA; i++ ) {
    char buf[MAX_STRING_LENGTH];
    char components[MAX_STRING_LENGTH];
    char effects[MAX_STRING_LENGTH];
    brew_formula_type *formula = &(brew_formula_table[i]);
    components[0] = '\0';
    for ( int j = 0; j < formula->num_component; j++ ) {
      char buf2[64];
      //sprintf( buf2, "%d", formula->component_list[j]);
      if ( formula->component_list[j] == NO_FLAG ) {
	sprintf( buf2, "INVALID" );
	bug("Unknown %d° component in formula %s", j, formula->name );
      }
      else
	sprintf( buf2, "%s", quotify(flag_string(brew_component_flags,formula->component_list[j])));
      strcat( components, buf2  );
      if ( j < formula->num_component-1 )
	strcat( components, ", ");
    }
    effects[0] = '\0';
    for ( int j = 0; j < formula->num_effect; j++ ) {
      strcat( effects, quotify(ability_table[formula->effect_list[j]].name) );
      if ( j < formula->num_effect-1 )
	strcat( effects, ", ");
    }
    fprintf(fp,
	    "BrewFormula %s {\n"
	    "  Component = (%s);\n"
	    "  Level = %d;\n"
	    "  Cost = %d;\n"
	    "  Effects = (%s);\n"
	    "}\n",
	    quotify(formula->name), components, formula->level, formula->cost, effects );
  }

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  brewnotsaved = FALSE;
}

static int step_component_size = 4;
static int cur_component_size = 0;
static int cur_component_ins = 0;
void insert_component_flags( const char *s ) {
  if ( cur_component_ins >= cur_component_size ) { // upgrade size
    int new_size = cur_component_size + step_component_size;
    flag_type *tmp;
    if ( ( tmp = (flag_type *)GC_MALLOC(sizeof(flag_type)*(new_size)) ) == NULL ) {
      bug("Can't allocate memory for component list");
      exit(-1);
    }
    for ( int i = 0; i < cur_component_size; i++ ) {
      tmp[i].name = str_dup(brew_component_flags[i].name);
      tmp[i].bit = brew_component_flags[i].bit;
      tmp[i].settable = brew_component_flags[i].settable;
    }
    brew_component_flags = tmp;
    cur_component_size = new_size;
  }
  //insert in component table
  if ( s == NULL )
    brew_component_flags[cur_component_ins].name = NULL;
  else
    brew_component_flags[cur_component_ins].name = str_dup(s);
  brew_component_flags[cur_component_ins].bit = cur_component_ins;
  brew_component_flags[cur_component_ins].settable = TRUE;
  cur_component_ins++;
}
void new_load_brew_formula() {
  FILE *fp;

  // Components
  log_stringf("Reading brew component");
  fclose(fpReserve);
  if (!(fp = fopen ( BREWCOMPONENT_FILE, "r"))){
    bug("Could not open file [%s] in order to load brew components.", BREWCOMPONENT_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for (;;) {
    if ( feof(fp) )
      break;
    char t[MAX_STRING_LENGTH]; // read a line
    fgets(t,MAX_STRING_LENGTH,fp);
    t[strlen(t)-1]='\0';

    const char *s = replace_char( t, ' ', '_' );
    if ( !str_cmp( s, "-1") )
      break;
    insert_component_flags(s);
    MAX_BREW_COMPONENT++;
  }
  insert_component_flags(NULL); // insert a dummy line

  //  update_flag_stat_table_init( "component", brew_component_flags );
  //  update_flag_stat_table( "component", brew_component_flags );
  //  update_help_table( "component", brew_component_flags );

  log_stringf(" %d brew component found.", MAX_BREW_COMPONENT );

  fclose(fp);
  //  fpReserve = fopen( NULL_FILE, "r" );
  

  // Formulas
  log_stringf("Reading brew formula");

  //  fclose(fpReserve);
  if (!(fp = fopen ( BREWFORMULA_FILE, "r"))){
    bug("Could not open file [%s] in order to load brew formulas.", BREWFORMULA_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  parse_datas( fp );
  log_stringf(" %d brew formula found.", MAX_BREW_FORMULA );

  fclose(fp);
  fpReserve = fopen( NULL_FILE, "r" );

  brewnotsaved = FALSE;
}

void create_brew_formula_table( const int count ) {
  static bool done = FALSE;
  if ( done == TRUE )
    return;
  MAX_BREW_FORMULA = count;
  if ( ( brew_formula_table = (brew_formula_type*)GC_MALLOC(sizeof(brew_formula_type)*MAX_BREW_FORMULA)) == NULL )
    p_error("Can't allocate memory brew formula table");
  done = TRUE;
}

//#define VERBOSE
//#define VERBOSE2

void parse_effects( DATAData *effects, brew_formula_type *formula ) {
  // value:  <List of String (ability (mostly spell) name)>
  ValueList *list = effects->value->eval().asList();
  formula->num_effect = list->size;

  if ( ( formula->effect_list = (int*) GC_MALLOC_ATOMIC( list->size * sizeof(int) ) ) == NULL )
    p_error("Can't allocate memory");

  for ( int i = 0; i < list->size; i++ ) {
    const char *abilityName = list->elems[i].asStr();
    formula->effect_list[i] = ability_lookup( abilityName );
    
    // check validity
    if ( formula->effect_list[i] <= 0 ) {
      bug("Invalid ability (%s) for formula (%s) assuming 0.",
	  ability_table[formula->effect_list[i]].name, formula->name );
      formula->effect_list[i] = 0;
    }
    else if ( ability_table[formula->effect_list[i]].target != TAR_CHAR_DEFENSIVE
	      && ability_table[formula->effect_list[i]].target != TAR_CHAR_SELF ) {
      bug("Ability (%s) for formula (%s) is not a defensive one, assuming 0.",
	  ability_table[formula->effect_list[i]].name, formula->name );
      formula->effect_list[i] = 0;
    }

    if ( DATA_VERBOSE > 2 ) {
      printf(" effects #%d: %s\n\r", i, ability_table[formula->effect_list[i]].name );
    }
  }

  list->explicit_free();
}

void parse_components( DATAData *components, brew_formula_type *formula ) {
  // value:  <List of String (id in brew_component_flags)>
  ValueList *list = components->value->eval().asList();
  formula->num_component = list->size;

  if ( ( formula->component_list = (int*) GC_MALLOC_ATOMIC( list->size * sizeof(int) ) ) == NULL )
    p_error("Can't allocate memory");

  for ( int i = 0; i < list->size; i++ ) {
    const char *s = replace_char( list->elems[i].asStr(), ' ', '_' );
    int id = flag_value( brew_component_flags, s );
    if ( id == NO_FLAG )
      bug("Invalid component [%s] in formula [%s]", s, formula->name );
    formula->component_list[i] = id;

    // check duplicate and validity
    //    OBJ_INDEX_DATA *pObj = get_obj_index(formula->component_list[i]);
    //    if ( pObj == NULL )
    //      p_error("Invalid component (%d) in formula (%s)",
    //	      formula->component_list[i], formula->name );
    //    if ( pObj->item_type != ITEM_COMPONENT )
    //      p_error("Component (%d) is not a component in formula (%s)", 
    //	  formula->component_list[i], formula->name );
    //    for ( int j = 0; j < i; j++ )
    //      if ( formula->component_list[i] == formula->component_list[j] )
    //	bug("Duplicate component (%d) in formula (%s)",
    //	    formula->component_list[i], formula->name );
    
    if ( DATA_VERBOSE > 2 ) {
      //printf(" component #%d: %s (%d)\n\r", i, pObj->short_descr, formula->component_list[i] );
      printf(" component #%d: %s (%d)\n\r", i,
	     flag_string(brew_component_flags, formula->component_list[i]),
	     formula->component_list[i] );
    }
  }

  list->explicit_free();
}

void parse_brew_formula( DATAData *form ) {
  static int index = 0;
  // tag:     BrewFormula
  // value:   <Formula Name>
  // fields:  Component = <List of String (component name in brew_component_flags)>
  // fields:  Level = <Integer (potion effects level)>
  // fields:  Cost = <Integer (potion money cost)>
  // fields:  Effects = <List of String (ability (mainly spell) name)>

  brew_formula_type *formula = &(brew_formula_table[index]);
  // value
  formula->name = str_dup( form->value->eval().asStr() );

  if ( DATA_VERBOSE > 0 ) {
    printf("Formula Name: %s\n\r", formula->name );
  }

  // fields
  for ( int fieldCount = 0; fieldCount < form->fields_count; fieldCount++ ) {
    DATAData *field = form->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
    if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
    }
    switch( tagId ) {
      // Potion effects level
    case TAG_Level:
      formula->level = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("  Level: %d\n\r", formula->level );
}
      break;

      // Potion money cost
    case TAG_Cost:
      formula->cost = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("  Cost: %d\n\r", formula->cost );
}
      break;

      // List of components
    case TAG_Component:
      parse_components( field, formula );
      break;

      // List of effects
    case TAG_Effects:
      parse_effects( field, formula );
      break;
      
      // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

if ( DATA_VERBOSE > 1 ) {
  printf("Name: %s  #component: %d  level: %d  cost: %d  #spell: %d\n\r",
	      formula->name, formula->num_component, formula->level, formula->cost, formula->num_effect );
  for ( int j = 0; j < formula->num_component; j++ )
    printf(" component #%d: %d\n\r", j, formula->component_list[j] );
  for ( int j = 0; j < formula->num_effect; j++ )
    printf(" effect #%d: %s\n\r", j, ability_table[formula->effect_list[j]].name );
}

  index++;
}
