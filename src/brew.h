#ifndef __BREW_H__
#define __BREW_H__

extern bool brewnotsaved;
extern bool brewcomponentnotsaved;

// Maximum component in a formula
#define MAX_COMPONENT (5)
// Maximum spells in a potion
#define MAX_BREW_SPELLS (4)

// structure for one brew formula
struct brew_formula_type: OLCEditable {
  const char *name;           // formula name
  int num_component;          // number of components
  int *component_list;        // list of components (id in brew_component_flags)
  int level;                  // level of spells put in potion
  int cost;                   // potion's cost
  int num_effect;             // number of spells in potion
  int *effect_list;           // list of effect (ability, mostly spell)
};

extern int MAX_BREW_FORMULA;  // number of formula
extern int MAX_BREW_COMPONENT;  // number of components
extern brew_formula_type *brew_formula_table; // table with every formula

// Load brew formula file and fill brew formula table
void new_save_brew_formula();
void new_load_brew_formula();
void create_brew_formula_table( const int count );
void new_save_brew_components();

int brew_lookup( const char *name );

// Brew skill
DECLARE_DO_FUN( do_brew         );

#endif
