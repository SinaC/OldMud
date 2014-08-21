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
#include <stdarg.h>
#include <ctype.h>
#include "scrhash.h"
#include "cxx/parser_skel.hh"
#include "grammar.hh"
#include "db.h"
#include "scrhash.h"
#include "olc_value.h"
#include "dbscript.h"
#include "scrpred.h"
#include "comm.h"
#include "config.h"
#include "mem.h"
#include "scanner.hh"
#include "error.hh"
#include "data_scanner.hh"


// Class assigned to player
CLASS_DATA *default_player_class;


int MAX_SCRIPTS;
long topscripts = 0;

bool isRootClass( CLASS_DATA *pClass ) {
  return ( pClass == default_mob_class
	   || pClass == default_obj_class
	   || pClass == default_room_class );
}

// Multiple Inherited, SinaC 2003
CLASS_DATA* get_root_class( CLASS_DATA *cla ) {
  CLASS_DATA *c = cla;
  CLASS_DATA *c_prec = c;
  while ( c != NULL ) {
    c_prec = c;
    if ( c->parents_count == 0 )
      c = NULL;
    else if ( c->parents_count == 1 )
      c = c->parents[0];
    else {
      CLASS_DATA *store_c = get_root_class( c->parents[0] ); // get first root class
      CLASS_DATA *new_c = NULL;
      for ( int i = 1; i < c->parents_count; i++ ) { // get other root class and compare
	new_c = get_root_class( c->parents[i] );
	if ( new_c != store_c ) {
	  bug("Problem with multiple inheritance: parents of (%s) have different root class.",
	      cla->name );
	  exit(-1);
	}
      }
      return store_c;
    }
  }
  return c_prec;
}


/*************************** Basic read ********************************/

TriggerDescr triggers[] = {
  // name          nb params    min_pos  params name

  {"onGreetLeader",    2, POS_RESTING, {"act","from"}},
  {"onExitingLeader",  2, POS_RESTING, {"act","from"}},
  {"onExitedLeader",   2, POS_RESTING, {"act","from"}},
  {"onEnteringLeader", 2, POS_RESTING, {"act","from"}},
  {"onEnteredLeader",  2, POS_RESTING, {"act","from"}},

  {"onGreet",          2, POS_RESTING, {"act","from"}},
  {"onExiting",        2, POS_RESTING, {"act","to"}},
  {"onExited",         2, POS_RESTING, {"act","to"}},
  {"onEntering",       2, POS_RESTING, {"act","from"}},
  {"onEntered",        2, POS_RESTING, {"act","from"}},
  {"onMoved",          0, POS_RESTING},
  {"onKnock",          2, POS_RESTING, {"act","from"}},

  {"onSpeech",         2, POS_RESTING, {"act","msg"}},
  {"onWhisper",        2, POS_RESTING, {"act","msg"}},
  {"onTell",           2, POS_RESTING, {"act","msg"}},
  {"onSocial",         2, POS_RESTING, {"act","socialName"}},
  {"onBribe",          3, POS_RESTING, {"act","amount","isSilver"}},

  {"onReset",          0, POS_DEAD},
  {"onRepop",          1, POS_DEAD,    {"real"}},
  {"onCreate",         0, POS_DEAD},
  {"onPulseMobile",    0, POS_DEAD},
  {"onPulseTick",      0, POS_DEAD},

  {"onFight",          1, POS_FIGHTING, {"act"}},

  {"onLooking",        1, POS_RESTING, {"act"}},

  {"onGetting",        1, POS_RESTING, {"act"}},
  {"onGot",            1, POS_RESTING, {"act"}},
  {"onDamage",         1, POS_FIGHTING, {"act"}},
  {"onPull",           1, POS_RESTING, {"act"}},
  {"onWearing",        1, POS_RESTING, {"act"}},
  {"onWorn",           1, POS_RESTING, {"act"}},
  {"onRemoving",       1, POS_RESTING, {"act"}},
  {"onRemoved",        1, POS_RESTING, {"act"}},
  {"onDropping",       1, POS_RESTING, {"act"}},
  {"onDropped",        1, POS_RESTING, {"act"}},
  {"onPutting",        2, POS_RESTING, {"act","container"}},
  {"onPut",            2, POS_RESTING, {"act","container"}},
  {"onGiving",         2, POS_RESTING, {"param1","param2"}},
  {"onGiven",          2, POS_RESTING, {"param1","param2"}},

  {"onSpellTarget",    3, POS_DEAD,    {"act","spellName","castingLevel"}},

  {"onFleeing",        1, POS_RESTING, {"act"}},
  {"onFlee",           2, POS_RESTING, {"act","to"}},
  {"onSit",            1, POS_RESTING, {"act"}},
  {"onRest",           1, POS_RESTING, {"act"}},
  {"onSleep",          1, POS_RESTING, {"act"}},
  {"onStand",          1, POS_RESTING, {"act"}},
  {"onDisarmed",       2, POS_RESTING, {"act","obj"}},

  {"onKilled",         1, POS_DEAD, {"act"}},

  {"onCast",           5, POS_RESTING, {"spellName","level","target","castingLevel","addParam"}},
  {"onPsi",            5, POS_RESTING, {"powerName","level","target","castingLevel","addParam"}},
  {"onSong",           5, POS_RESTING, {"songName","level","target","castingLevel","addParam"}},
  {"onSkill",          5, POS_RESTING, {"skillName","level","target","castingLevel","addParam"}},

  {"onKill",           1, POS_RESTING, {"act"}},

  {"onLoad",           0, POS_DEAD},
  {"onQuitting",       0, POS_DEAD},
  {"onInformation",    1, POS_DEAD, {"parm"}},

  { NULL,              0}
};

TriggerDescr *isTrigger( FCT_DATA *fct ) {
  for (TriggerDescr* t = triggers; t->name != NULL; t++)
    if (!strcmp(t->name, fct->name))
      return t;
  return NULL;
}
TriggerDescr *isTrigger( const char *name, const bool caseSensitive ) {
  if ( caseSensitive ) {
    for ( TriggerDescr *t = triggers; t->name != NULL; t++ )
      if ( !strcmp( t->name, name ) )
	return t;
  }
  else
    for ( TriggerDescr *t = triggers; t->name != NULL; t++ )
      if ( !str_cmp( t->name, name ) )
	return t;
  return NULL;
}

// Compute list of class->file, we compute it once at boot
//  and each time someone use  program command  or ? command
// But it's computed if a file has changed or a class has been created/deleted
bool FILE_LIST_COMPUTED; // TRUE: if the file list has no need to be updated
bool FILE_LIST_UPDATED; // TRUE: if no class has been modified
int max_script_file; // maximum number of entries in script_file_list
int cur_script_file; // current number of entries in script_file_list
file_list *script_file_list;

// Upgrade size of fl->class_list by STEP_CLASS_LIST * nbStep   ~ REALLOC
void upgrade_class_list( file_list *fl, const int nbStep ) {
  CLASS_DATA **tmp;
  // create new vector
  int newSize = fl->class_count + (STEP_CLASS_LIST*nbStep);
  if ( ( tmp = (CLASS_DATA **)GC_MALLOC( sizeof(CLASS_DATA*) * newSize ) ) == NULL )
    g_error("Can't allocate memory for class_list");
  // copy information
  for ( int i = 0; i < fl->class_count; i++ )
    tmp[i] = fl->class_list[i];
  // points to new vector
  fl->class_list = tmp;
  fl->max_class_count = newSize;
}
// Upgrade size of script_file_list by STEP_SCRIPT_FILE_LIST   ~ REALLOC
void upgrade_script_file_list() {
  file_list *tmp;
  // create new vector
  int newSize = cur_script_file + STEP_SCRIPT_FILE_LIST;
  if ( ( tmp = (file_list*) GC_MALLOC( sizeof(file_list) * newSize ) ) == NULL )
    g_error("Can't allocate memory for class_list");
  // copy information
  for ( int i = 0; i < cur_script_file; i++ ) {
    file_list *fl = &(script_file_list[i]);
    tmp[i].name = str_dup( fl->name );
    tmp[i].to_save = fl->to_save;
    // create class_list vector
    CLASS_DATA **tmp2;
    if ( ( tmp2 = (CLASS_DATA **)GC_MALLOC( sizeof(CLASS_DATA*) * fl->max_class_count ) ) == NULL )
      g_error("Can't allocate memory for class_list");
    // copy information
    for ( int j = 0; j < fl->class_count; j++ )
      tmp2[j] = fl->class_list[j];
    // points to new vector
    tmp[i].max_class_count = fl->max_class_count;
    tmp[i].class_count = fl->class_count;
    tmp[i].class_list = tmp2;
  }
  // points to new vector
  script_file_list = tmp;
  max_script_file = newSize;
}
void compute_file_list_one_class( CLASS_DATA *cl ) {
  for ( int i = 0; i < cur_script_file; i++ ) {
    file_list *fl = &(script_file_list[i]);
    if ( !str_cmp( fl->name, cl->file ) ) { // Class file found
      // add class in class list
      if ( fl->class_count >= fl->max_class_count )
	upgrade_class_list( fl, 1 );
      fl->class_list[fl->class_count++] = cl;
      if ( IS_SET( cl->flags, CLASS_NOT_SAVED ) )
	fl->to_save = TRUE;
      return;
    }
  }
  // Class file not found, create a new entry
  // and add class in class list
  if ( cur_script_file >= max_script_file )
    upgrade_script_file_list();
  file_list *fl = &(script_file_list[cur_script_file++]);
  fl->name = str_dup(cl->file);
  fl->class_count = 0;
  upgrade_class_list( fl, 1 );
  fl->class_count = 1;
  fl->class_list[0] = cl;
  if ( IS_SET( cl->flags, CLASS_NOT_SAVED ) )
    fl->to_save = TRUE;
}
// Create script file list
bool compute_file_list() {
  // FILE_LIST_COMPUTE is modified in this function and when adding/removing a class 
  //  or editing class->file
  if ( FILE_LIST_COMPUTED ) // if file list already computed, no need to be done
    return TRUE;

  try {
    cur_script_file = 0;
    max_script_file = 0;
    for ( int i = 0; i < HASH_PRG; i++ ) {
      CLASS_DATA *cl = prg_table[i];
      while ( cl != NULL ) {
	if ( !isRootClass(cl) )
	  compute_file_list_one_class( cl );
	cl = cl->next;
      }
    }
  } catch ( GeneralException g ) {
    bug("Problem with computing file list: %s", g.msg );
    return FALSE;
  }
  FILE_LIST_COMPUTED = TRUE;
  FILE_LIST_UPDATED = TRUE;
  return TRUE;
}

// Update script file to_save value
void update_file_list() {
  if ( FILE_LIST_UPDATED ) // if file list to_save values are correct
    return;

  for ( int i = 0; i < cur_script_file; i++ ) {
    file_list *fl = &(script_file_list[i]);
    for ( int j = 0; j < fl->class_count; j++ ) {
      CLASS_DATA *cl = fl->class_list[j];
      if ( IS_SET( cl->flags, CLASS_NOT_SAVED ) )
	fl->to_save = TRUE;
    }
  }
}





// Structures used to save every class's parents
// We create parents vector only after reading every classes
// So we don't have take care about reading order for inheritance
#define MAX_CLASS_PARENT (16)
struct save_class_parent {
  const char *name; // class name
  int parents_count; // number of parent
  const char *parents[MAX_CLASS_PARENT]; // parents name
};
int save_class_parent_index;
save_class_parent save_class_parent_list[HASH_PRG]; // for each class, we save the parents
// Create parents list with saved informations
void use_save_class_parent_list() {
  for ( int i = 0; i < save_class_parent_index; i++ ) {
    save_class_parent *save = &(save_class_parent_list[i]);
    CLASS_DATA *cl = hash_get_prog( save->name ); // first we find the class
    if ( cl == NULL ) {
      bug("Class %s has parents but doesn't exist itself!!!", save->name );
      continue;
    }
    cl->parents_count = save->parents_count;
    if ( ( cl->parents = (CLASS_DATA **) GC_MALLOC( sizeof(CLASS_DATA *)*cl->parents_count ) ) == NULL ) {
      bug("Can't allocate memory for Class %s parents", cl->name );
      exit(-1);
    }
    for ( int j = 0; j < save->parents_count; j++ ) {
      CLASS_DATA *p = hash_get_prog( save->parents[j] );
      if ( p == NULL ) {
        bug("'%s' extends an unknown class '%s' replaced with DefaultMobClass", 
	    cl->name, save->parents[j] );
        cl->parents[j] = default_mob_class;
	continue;
      }
      cl->parents[j] = p;
      if ( SCRIPT_VERBOSE > 1 ) {
	log_stringf("Class %s extends %s.", cl->name, p->name );
      }
    }
  }
  log_stringf("Classes parents informations updated.");
}

// Methods to check if there is a cycle in classes's inheritance
int cycleDepth;
bool isCyclic( CLASS_DATA *cl, CLASS_DATA *additional_parent ) {
  SET_BIT( cl->flags, CLASS_MARKED );
  cycleDepth++;
  // Normal parents
  for ( int i = 0; i < cl->parents_count; i++ ) {
    if ( IS_SET( cl->parents[i]->flags, CLASS_MARKED ) )
      return TRUE;
    if ( isCyclic( cl->parents[i], NULL ) ) // further call doesn't care about addiotional_parent
      return TRUE;
  }
  // Additional parent
  if ( additional_parent ) {
    if ( isCyclic( additional_parent, NULL ) ) // further call doesn't care about addiotional_parent
      return TRUE;
  }

  cycleDepth--;
  REMOVE_BIT( cl->flags, CLASS_MARKED );
  return FALSE;
}
void emptyMark() {
  for ( int i = 0; i < HASH_PRG; i++ ) {
    CLASS_DATA *cl = prg_table[i];
    while ( cl != NULL ) {
      REMOVE_BIT( cl->flags, CLASS_MARKED );
      cl = cl->next;
    }
  }
}
bool check_cyclic_inheritance_one_class( CLASS_DATA *cl, CLASS_DATA *additional_parent ) {
  emptyMark();
  cycleDepth = -1;
  if ( isCyclic(cl,additional_parent) )
    g_error( "Cyclic inheritance starting with class [%s], cycle depth [%d].",
	     cl->name, cycleDepth );
  return FALSE;
}
void check_cyclic_inheritance() {
  // For each class which is not a root class, check if there is a cyclic inheritance
  for ( int i = 0; i < HASH_PRG; i++ ) {
    CLASS_DATA *cl = prg_table[i];
    while ( cl != NULL ) {
      if ( !isRootClass( cl ) ) {
	try {
	  check_cyclic_inheritance_one_class( cl, NULL );
	} catch (GeneralException g) {
	  bug("ERROR: %s", g.msg );
	  exit(-1);
	}
      }
      cl = cl->next;
    }
  }
}


// Create a function from a method
FCT_DATA* make_method(Method* method, const char *prg_name, bool eraseRevert ) {
  FCT_DATA * fct= new_fct();
  
  for (int i = 0; i<method->params_count; i++) {
    fct->parmname[i] = str_dup(method->params[i]->image);
  }
  fct->nbparm = method->params_count;

  fct->name = str_dup(method->ident->image);

  if ( SCRIPT_VERBOSE > 1 ) {
    log_stringf("fct name: %s (%d)", fct->name, fct->nbparm);
  }

  // Check if function is trigger and has the right number of parameters
  TriggerDescr *t = isTrigger( fct );
  if ( t != NULL
       && ( t->nparms != fct->nbparm ) )
    p_error("Wrong number parameters in trigger %s in class %s.", fct->name, prg_name );

  // Assign function's code
  fct->code = method->body;
  REMOVE_BIT( fct->flags, FCT_NO_CODE );

  // SinaC 2003: method's code are stored and can so be modified
  strcpy( large_buffer, method->code );
  strip_char_leading( large_buffer, '\n' );
  //fct->scriptCode = str_dup( method->code );
  fct->scriptCode = str_dup( large_buffer );
  // we also keep a save of the code to allow a revert
  if ( eraseRevert )
    //fct->scriptCodeSaved = str_dup( method->code );
    fct->scriptCodeSaved = str_dup( large_buffer );
  if ( SCRIPT_VERBOSE > 1 ) {
    log_stringf("Code: %s", fct->scriptCode );
  }
  
  return fct;
}

// Modified by SinaC 2002 to keep trace of class's incoming file
void make_class(Class* c, const char *filename ) {
  CLASS_DATA * prg;
  FCT_DATA * f;
  
  prg = new_prg();
  prg->name = c->ident->image;

  // Added by SinaC 2002
  prg->file = str_dup( filename );

  prg->parents = NULL;

  // Modified by SinaC 2003  for multiple inheritance
  //prg->parents_count = c->extends_count;
  //  if ( ( prg->parents = (CLASS_DATA **)GC_MALLOC(sizeof(CLASS_DATA*)*prg->parents_count) ) == NULL ) {
  //    bug("Memory allocation error: parents for class %s", prg->name );
  //    exit(-1);
  //  }
  //  for ( int i = 0; i < prg->parents_count; i++ ) {
  //    prg->parents[i] = hash_get_prog(c->extends[i]->image);
  //    if ( prg->parents[i] == NULL ) {
  //      bug("'%s' extends an unknown class '%s' replaced with DefaultMobClass", 
  //	  prg->name, c->extends[i]->image );
  //      prg->parents[i] = default_mob_class;
  //    }
  //  }
  // We store parents until we have read every files
  save_class_parent *save = &(save_class_parent_list[save_class_parent_index]);
  save->name = str_dup( prg->name );
  save->parents_count = c->extends_count;
  for ( int i = 0; i < c->extends_count; i++ )
    save->parents[i] = str_dup( c->extends[i]->image );
  save_class_parent_index++;

  prg->isAbstract = c->isAbstract != 0;

  // Added by SinaC 2001
  //  if ( ( prg->methods = (FCT_DATA**)GC_MALLOC(HASH_FCT*sizeof(FCT_DATA*))) == NULL ) {
  //    bug("Memory allocation error: methods for class %s", prg->name );
  //    exit(-1);
  //  }
  for ( int i = 0; i < HASH_FCT; i++ )
    prg->methods[i] = NULL;

  for (int i = 0; i<c->methods_count; i++) {
    try {
      // prg->name added for more explicit error messages
      f = make_method(c->methods[i], prg->name, true );

      f->incoming = prg; // we store a ptr to the Class containing this method
      f->positionId = i; // store method's position in the file
      hash_put_fct( f, prg->methods );
    } catch (ScriptException e) {
      log_stringf("Error while parsing %s.%s: %s", prg->name, c->methods[i]->ident->image, e.msg );
    }
  }

  // Added by SinaC 2001 to check if script already exists
  CLASS_DATA *cl;
  if ( ( cl = silent_hash_get_prog(prg->name) ) )
    bug("class [%s] has been already found in file [%s.script].", prg->name, cl->file );

  hash_put_prog(prg);    
}

const char *getFullFName( const char *s ) {
  static char buf[MAX_STRING_LENGTH];
  strcpy(buf, SCRIPT_DIR );
  strcat( buf, s );
  return buf;
}
int read_script_file(const char *fname, const char *shortfname) {
  FILE *fp;

  init_grammar_parsing(); // SinaC 2003

  log_stringf(" Reading script: %s", fname );
  
  //char buf[MAX_STRING_LENGTH];
  //strcpy(buf, getFullFName(fname));
  const char *buf = getFullFName(fname);
  
  if ( !(fp = fopen(buf ,"r")))
    p_error("Unable to read script");

  int actual_size = 0;

  large_buffer[0] = '\0';
  actual_size = fread(large_buffer, 1, BIG_SIZE, fp);
  large_buffer[actual_size] = '\0';

  if ( SCRIPT_VERBOSE > 3 ) {
    log_stringf("dump:\n\r"
		"%s\n\r"
		"actual size: %d bytes\n\r",
		large_buffer,actual_size);
  }

  cur_pos = large_buffer;

  int count = 0;
  try {
    Module* module = (Module*)parse();
    if ( SCRIPT_VERBOSE > 3 ) {
      module->dump(1);
    }

    topscripts += module->classes_count;
    count = module->classes_count;

    for (int i = 0; i<module->classes_count; i++)
      make_class(module->classes[i], shortfname );
  } catch (ScriptException e) {
    bug("Error while parsing scripts. %s", e.msg );
    exit(-1);
  }

  return count;

  fclose(fp);
}

const char * getShortFName( const char *s ) {
  char buf[MAX_STRING_LENGTH];
  unsigned int i = 0;
  for ( i = 0; i < strlen(s); i++ )
    if ( isalnum(s[i]) )
      buf[i] = s[i];
    else
      break;
  buf[i] = '\0';
  return str_dup(buf);
}
void read_scripts() {
  FILE *fp;
  log_string("Reading scripts (Mob/Obj/Room Programs)");
  if ( !(fp = fopen(SCRIPT_LIST,"r"))) {
    perror("script: list file opening");
    return;
  }

  save_class_parent_index = 0; // used to find class's parents

  MAX_SCRIPTS = 0;

  char w[MAX_STRING_LENGTH];
  char name[MAX_STRING_LENGTH];
  int nb = 0;
  do {
    fgets(w,MAX_STRING_LENGTH,fp);
    w[strlen(w)-1]='\0';

    if ( w[0] == END_OF_SCRIPT_LIST )
      break;

    if ( w[0] == '\0' )
      continue;

    // copy 'w' to 'name' until a non-alpha  for example '.'   in  player.script
    const char *p = w;
    char *q = name;
    while( *p != '\0' ) {
      if ( !isalnum(*p) )
	break;
      *q++ = *p++;
    }
    *q = '\0';

    MAX_SCRIPTS += read_script_file( w, name );

    nb++;

  } while ( w != NULL );

  fclose(fp);
  
  log_stringf("%d scripts file read", nb );
  log_stringf("%ld scripts found", topscripts );

  use_save_class_parent_list(); // used to find class's parents
  check_cyclic_inheritance(); // check if there is cyclic parents
  FILE_LIST_COMPUTED = FALSE;
  compute_file_list();

  default_player_class = hash_get_prog( "player" );
  if ( default_player_class == NULL ) {
    bug("Class player not found!! Using default_mob_class");
    default_player_class = default_mob_class;
  }
}


void boot_scripts() {
  hash_init();
  boot_pred_fct();
  
  read_scripts();
}

void check_needed_scripts() {
  extern ValueList *NEEDED_SCRIPT;
  log_stringf("Checking needed scripts...");
  if ( NEEDED_SCRIPT != NULL ) {
    for ( int i = 0; i < NEEDED_SCRIPT->size; i++ ) {
      const char *s = NEEDED_SCRIPT->elems[i].asStr();
      if ( silent_hash_get_prog( s ) == NULL )
	log_stringf("  Script [%s] is missing for a correct mud running", s );
    }
  }
  log_stringf("Done.");
}

// *************************  Script reloading stuff  *****************************
typedef struct entity_save { // linked list storing entity with a non-default class
  ENTITY_DATA *e;
  const char *prgname;
  entity_save *next;
};
entity_save *entity_save_head;

typedef struct index_data_save { // linked list storing _index_data with a non-default class
  void *e;
  int kind;
  const char *prgname;
  index_data_save *next;
};
index_data_save *index_data_save_head;

static void save_world() {
  // Save mobiles/players
  for ( CHAR_DATA *ch = char_list; ch != NULL; ch = ch->next ) {
    if ( ch->clazz != NULL
	 && ch->clazz != default_mob_class ) {
      entity_save *p = new entity_save;
      p->e = ch;
      p->prgname = str_dup( ch->clazz->name );
      p->next = entity_save_head;
      entity_save_head = p;
    }
  }
  // Save objects
  for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj->next ) {
    if ( obj->clazz != NULL
	 && obj->clazz != default_obj_class ) {
      entity_save *p = new entity_save;
      p->e = obj;
      p->prgname = str_dup( obj->clazz->name );
      p->next = entity_save_head;
      entity_save_head = p;
    }
  }
  // Save room
  for ( int i = 0; i < 65535; i++ ) {
    ROOM_INDEX_DATA *room;
    if ( ( room = get_room_index( i ) ) != NULL 
	 && room->clazz != NULL
	 && room->clazz != default_room_class ) {
      entity_save *p = new entity_save;
      p->e = room;
      p->prgname = str_dup( room->clazz->name );
      p->next = entity_save_head;
      entity_save_head = p;
    }
  }
  // Save mob index
  int nMatch=0;
  for(int vnum=0;nMatch<top_mob_index;vnum++) {
    MOB_INDEX_DATA *mob;
    if ( (mob=get_mob_index(vnum)) == NULL)
      continue;
    nMatch++;
    if ( mob->program != NULL 
	 && mob->program != default_mob_class ) {
      index_data_save *p = new index_data_save;
      p->e = (void*)mob;
      p->kind = CHAR_ENTITY;
      p->prgname = str_dup( mob->program->name );
      p->next = index_data_save_head;
      index_data_save_head = p;
    }
  }
  // Save obj index
  nMatch=0;
  for(int vnum=0;nMatch<top_obj_index;vnum++) {
    OBJ_INDEX_DATA *obj;
    if ( (obj=get_obj_index(vnum)) == NULL)
      continue;
    nMatch++;
    if ( obj->program != NULL 
	 && obj->program != default_obj_class ) {
      index_data_save *p = new index_data_save;
      p->e = (void*)obj;
      p->kind = OBJ_ENTITY;
      p->prgname = str_dup( obj->program->name );
      p->next = index_data_save_head;
      index_data_save_head = p;
    }
  }
  // Save room index
  for ( int i = 0; i < 65535; i++ ) {
    ROOM_INDEX_DATA *room;
    if ( ( room = get_room_index( i ) ) != NULL 
	 && room->program != NULL
	 && room->program != default_room_class ) {
      index_data_save *p = new index_data_save;
      p->e = (void*)room;
      p->kind = ROOM_ENTITY;
      p->prgname = str_dup( room->program->name );
      p->next = index_data_save_head;
      index_data_save_head = p;
    }
  }
}

static void clean_world() {
  // Free program hash table
  for (int i = 0; i < HASH_PRG; i++ )
    prg_table[i] = NULL;

  // After freeing we rehash default class
  hash_put_prog( default_mob_class );
  hash_put_prog( default_obj_class );
  hash_put_prog( default_room_class );
  topscripts = 0;
  // FIXME: Free non-player extra fields ?  onCreate/onRepop is called in update_world
}

static void update_world() {
  // Set mobiles/players/object/room classes
  while ( entity_save_head != NULL ) {
    ENTITY_DATA *e = entity_save_head->e;
    CLASS_DATA *cl = hash_get_prog( entity_save_head->prgname );
    e->clazz = cl;
    if ( cl == NULL ) {
      char *type;
      int vnum = -1;
      switch( e->kind ) {
      case CHAR_ENTITY: {
	type = "mob"; 
	CHAR_DATA *ch = (CHAR_DATA*)e; 
	if ( IS_NPC(ch) ) 
	  vnum = ch->pIndexData->vnum;
	e->clazz = default_mob_class;
	break; }
      case OBJ_ENTITY: 
	type = "obj"; 
	vnum = ((OBJ_DATA*)e)->pIndexData->vnum;
	e->clazz = default_obj_class;
	break;
      case ROOM_ENTITY: 
	type = "room"; 
	vnum = ((ROOM_INDEX_DATA*)e)->vnum;
	e->clazz = default_room_class;
	break;
      default: type = "unknown"; break;
      }
      bug("Reload_scripts: %s [%d] (entity %s) has lost its program (%s)",
	  e->name, vnum, type, entity_save_head->prgname );
    }
    COMMONPROG(e,"onCreate");
    COMMONPROG(e,"onRepop", Value((long)0) ); // 0 means reloading scripts
    entity_save_head = entity_save_head->next;
  }
  // Set mob/obj/room index classes
  while ( index_data_save_head != NULL ) {
    CLASS_DATA *cl = hash_get_prog( index_data_save_head->prgname );
    char *type = "NULL";
    char name[MAX_STRING_LENGTH];
    int vnum = 0;
    switch( index_data_save_head->kind ) {
    case CHAR_ENTITY: {
      MOB_INDEX_DATA *mob = (MOB_INDEX_DATA*)index_data_save_head->e;
      strcpy(name,mob->short_descr);
      type = "mob";
      vnum = mob->vnum;
      mob->program = cl;
      if ( cl == NULL )
	mob->program = default_mob_class;
      break; }
    case OBJ_ENTITY: {
      OBJ_INDEX_DATA *obj = (OBJ_INDEX_DATA*)index_data_save_head->e;
      strcpy(name,obj->short_descr);
      type = "obj";
      vnum = obj->vnum;
      obj->program = cl;
      if ( cl == NULL )
	obj->program = default_obj_class;
      break; }
    case ROOM_ENTITY: {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA*)index_data_save_head->e;
      strcpy(name,room->description);
      type = "room";
      vnum = room->vnum;
      room->program = cl;
      if ( cl == NULL )
	room->program = default_room_class;
      break; }
    default: break;
    }
    if ( cl == NULL )
      bug("Reload_scripts: %s [%d] (index %s) has lost its program (%s)",
	  name, vnum, type, index_data_save_head->prgname );
    index_data_save_head = index_data_save_head->next;
  }
}

// FIXME: first argument will be script file name, better class name
// Something like:
//  reload_scripts fileName className
void reload_scripts( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];

  log_stringf("Reloading scripts.");

  // Save plr/mob/obj/room classes
  // Save mob_index/obj_index classes
  log_stringf("  Saving world clazz/program.");
  save_world();

  // Clean hash_list
  log_stringf("  Cleaning world.");
  clean_world();

  // Reload programs
  fBootDb = TRUE;
  log_stringf("  Reloading.");
  read_scripts();
  fBootDb = FALSE;

  // Reassign clazz/program to mob/obj/room
  log_stringf("  Reassigning clazz/program.");
  update_world();

  log_stringf("  Done.");
  send_to_char("Done.", ch );
}
