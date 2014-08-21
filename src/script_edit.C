/***************************************************************************
 *  File: script_edit.c                                                      *
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
#include "script_edit.h"
#include "string.h"
#include "interp.h"
#include "db.h"
#include "comm.h"
#include "scrhash.h"
#include "dbscript.h"
#include "cxx/parser_skel.hh"
#include "grammar.hh"
#include "mem.h"
#include "config.h"
#include "data_scanner.hh"
#include "error.hh"
#include "olc_value.h" // for ACT_IS_NPC


const char *defaultFunctionName = "defaultFunctionName";
int SCRIPT_MINIMUM_SECURITY;

//*****************************************************************************
//******************************** SCRIPTS ************************************
//*****************************************************************************

//-- structure used to store fct_data sorted with positionId
struct show_fct {
  FCT_DATA *f;
  show_fct *next_fct;
};
static show_fct *show_fct_head;
static void insert_show_fct( FCT_DATA *f ) {
  show_fct *p = show_fct_head, *q = show_fct_head;
  while ( p != NULL && p->f->positionId <= f->positionId ) {
    q = p;
    p = p->next_fct;
  }
  if ( p == NULL ) { // end of list
    p = new show_fct;
    p->f = f;
    p->next_fct = NULL;
    if ( show_fct_head == NULL ) // null list
      show_fct_head = p;
    else
      q->next_fct = p;
  }
  else {
    if ( p == show_fct_head ) { // head of list
      p = new show_fct;
      p->f = f;
      p->next_fct = show_fct_head;
      show_fct_head = p;
    }
    else {
      p = new show_fct;
      p->f = f;
      p->next_fct = q->next_fct;
      q->next_fct = p;
    }
  }
}


// -- Class manipulation
// Remove a parent from a class
void remove_one_parent( CLASS_DATA *pClass, CLASS_DATA *parent ) {
  CLASS_DATA **tmp; // create new vector
  if ( ( tmp = (CLASS_DATA **)GC_MALLOC(sizeof(CLASS_DATA*)*(pClass->parents_count-1)) ) == NULL ) {
    bug("Memory allocation error: parents for class %s", pClass->name );
    exit(-1);
  }
  int idx = 0;
  for ( int i = 0; i < pClass->parents_count; i++ ) // copy pointer
    if ( pClass->parents[i] != parent )
      tmp[idx++] = pClass->parents[i];
  pClass->parents = tmp; // pointer to parent is the new vector
  pClass->parents_count--; // one more parent
}
// Add a parent to a class
void add_one_parent( CLASS_DATA *pClass, CLASS_DATA *parent ) {
  CLASS_DATA **tmp; // create new vector
  if ( ( tmp = (CLASS_DATA **)GC_MALLOC(sizeof(CLASS_DATA*)*(pClass->parents_count+1)) ) == NULL ) {
    bug("Memory allocation error: parents for class %s", pClass->name );
    exit(-1);
  }
  for ( int i = 0; i < pClass->parents_count; i++ ) // copy pointer
    tmp[i] = pClass->parents[i];
  tmp[pClass->parents_count] = parent; // add the new parent
  pClass->parents = tmp; // pointer to parent is the new vector
  pClass->parents_count++; // one more parent
}
// Search a class in parent
bool find_parent( CLASS_DATA *pClass, CLASS_DATA *parent ) {
  for ( int i = 0; i < pClass->parents_count; i++ )
    if ( parent == pClass->parents[i] )
      return TRUE;
  return FALSE;
}
// Remove extra parents: such as default_XXX_class
// If more than one parent, don't need default_XXX_class, because we are sure
//  the other parents have the right root class
void remove_extra_parents( CLASS_DATA *pClass ) {
  if ( pClass->parents_count > 1 ) {
    if ( find_parent( pClass, default_mob_class ) ) // mob default class found, rip it
      remove_one_parent( pClass, default_mob_class );
    if ( find_parent( pClass, default_obj_class ) ) // obj default class found, rip it
      remove_one_parent( pClass, default_obj_class );
    if ( find_parent( pClass, default_room_class ) ) // room default class found, rip it
      remove_one_parent( pClass, default_room_class );
  }
}
// Check if there is MODIFIED or NON-CODE CORRECT methods in a class
bool is_finalized( CLASS_DATA *pClass ) {
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    while ( f != NULL ) {
      if ( IS_SET( f->flags, FCT_MODIFIED )
	   || IS_SET( f->flags, FCT_NO_CODE ) )
	return FALSE;
      f = f->next;
    }
  }
  return TRUE;
}

// -- Fct manipulation
extern FCT_DATA* make_method(Method* method, const char *prg_name, bool eraseRevert );
FCT_DATA *update_method( CHAR_DATA *ch, CLASS_DATA *cl, FCT_DATA *f ) {
  try {
    Method *m = (Method *)parse_one_method( f->scriptCode );
    if ( SCRIPT_VERBOSE > 3 ) {
      m->dump(1);
    }
    return make_method( m, cl->name, FALSE);
  } catch (ScriptException e) {
    send_to_charf(ch,"Error while parsing %s: %s\n\r", f->name, e.msg );
    return NULL;
  }
  return NULL;
}
bool update_one_method( CHAR_DATA *ch, CLASS_DATA *pClass, FCT_DATA *prev, FCT_DATA *&f, const int idx ) {
  FCT_DATA *nf = update_method( ch, pClass, f );
  if ( nf == NULL ) {
    send_to_charf(ch,"Problem while parsing %s.\n\r", f->name );
    SET_BIT( f->flags, FCT_NO_CODE );
    return FALSE;
  }
  if ( strcmp( f->name, defaultFunctionName )
       && strcmp( nf->name, f->name ) ) {
    send_to_charf(ch,"New method/trigger (%s) doesn't have the same name as old (%s).\n\r",
		  nf->name, f->name );
    SET_BIT( f->flags, FCT_NO_CODE );
    return FALSE;
  }
  TriggerDescr *t = isTrigger(nf);
  if ( t != NULL
       && t->nparms != nf->nbparm ) {
    send_to_charf(ch,"New trigger (%s) must have %d params.\n\r", nf->name, t->nparms );
    SET_BIT( f->flags, FCT_NO_CODE );
    return FALSE;
  }
  send_to_charf(ch,"%s updated.\n\r", f->name );
  nf->next = f->next;
  nf->positionId = f->positionId;
  if ( f == prev ) // if head of list: replace head with nf  only true at first loop in while
    pClass->methods[idx] = nf;
  else
    prev->next = nf;
  REMOVE_BIT( nf->flags, FCT_MODIFIED );
  REMOVE_BIT( nf->flags, FCT_NO_CODE );
  f = nf;
  return TRUE;
}


// -- Special method (update_after_edit_method) called in string.C:string_add
// --  it's a special call: the function to call is initialized in script_edit_method
void recover_syntax( CHAR_DATA *ch, FCT_DATA *f ) {
  send_to_charf(ch,"type 'editmethod %s' to fix the problem(s).\n\r", f->name );
}
// Return TRUE if s contains something useful
bool isRevelant( const char *s ) {
  const char *p = s;
  if ( *p == '\0' )
    return FALSE;
  while ( p != NULL ) {
    if ( isalnum(*p ) )
      return TRUE;
    p++;
  }
  return FALSE;
}
void update_after_edit_method( CHAR_DATA *ch, FCT_DATA *f ) {
  CLASS_DATA *pClass = f->incoming;
  ch->desc->pStringFun = NULL;
  if ( pClass == NULL ) {
    send_to_charf(ch,"PROBLEM while updating Method/trigger.\n\r");
    bug("PROBLEM while updating method %s: CANT FIND IT IN HASH LIST", f->name );
    SET_BIT( f->flags, FCT_NO_CODE );
    //recover_syntax( ch, f );
    return;
  }

  if ( f->scriptCode == NULL
       || f->scriptCode[0] == '\0'
       || !isRevelant( f->scriptCode ) ) {
    send_to_charf(ch,"Script code is empty or doesn't contain anything revelant.\n\r");
    recover_syntax( ch, f );
    SET_BIT( f->flags, FCT_NO_CODE );
    return;
  }

  FCT_DATA *nf = update_method( ch, pClass, f );
  // Error while parsing: msg already shown in update_method
  if ( nf == NULL ) {
    send_to_charf(ch,"Problem while parsing method/trigger.\n\r" );
    recover_syntax( ch, f );
    SET_BIT( f->flags, FCT_NO_CODE );
    return;
  }
  // Find f in hash list, f is a temporary function we have created in script_edit_add_method
  FCT_DATA *prev;
  int idx;
  if ( hash_get_fct_prev( f, pClass->methods, prev, idx ) == NULL ) {
    send_to_charf( ch, "Problem while searching temporary method/trigger.\n\r" );
    recover_syntax( ch, f );
    SET_BIT( f->flags, FCT_NO_CODE );
    return;
  }
  // Is trigger: must have the right number of parameters
  TriggerDescr *t = isTrigger(nf);
  if ( t != NULL
       && t->nparms != nf->nbparm ) {
    send_to_charf(ch,"New trigger (%s) must have %d params.\n\r", nf->name, t->nparms );
    recover_syntax( ch, f );
    SET_BIT( nf->flags, FCT_NO_CODE );
    return;
  }

  nf->positionId = f->positionId;

  REMOVE_BIT( nf->flags, FCT_MODIFIED );
  REMOVE_BIT( nf->flags, FCT_NO_CODE );

  // Remove f from hash list
  if ( f == prev )
    pClass->methods[idx] = f->next;
  else
    prev->next = f->next;

  // Copy scriptCodeSaved
  if ( f->scriptCodeSaved != NULL )
    nf->scriptCodeSaved = str_dup( f->scriptCodeSaved, TRUE );

  // Set incoming class
  nf->incoming = pClass;
  // Hash nf in list
  hash_put_fct( nf, pClass->methods );

  send_to_charf( ch, "%s %s added/modified.\n\r", t == NULL ?"Method":"Trigger", nf->name );

  if ( is_finalized(pClass) )
    REMOVE_BIT( pClass->flags, CLASS_MODIFIED );
}



// ------------------------------- SAVE
const char *class_header( CLASS_DATA *cl ) {
  static char buf[MAX_STRING_LENGTH];
  char parentList[MAX_STRING_LENGTH];
  parentList[0] = '\0';
  for ( int i = 0; i < cl->parents_count; i++ ) {
    strcat( parentList, cl->parents[i]->name );
    if ( i < cl->parents_count-1 )
      strcat( parentList, ", ");
  }
  sprintf( buf, "%sclass %s extends %s {", cl->isAbstract?"abstract ":"", cl->name, parentList );
  return buf;
}
void save_one_script_file( CHAR_DATA *ch, const int idx ) {
  file_list *fl = &(script_file_list[idx]);
  char buf[MAX_STRING_LENGTH];
  sprintf( buf, "%s%s.script", SCRIPT_DIR, fl->name );
  FILE *fp = fopen( buf, "w" );
  if ( fp == NULL ) {
    send_to_charf(ch,"Can't open file %s to save programs.", buf );
    return;
  }
  if ( SCRIPT_VERBOSE > 1 ) {
    log_stringf("File: %s", buf );
  }
  log_stringf(" Saving script file: %s.script [%d programs]", fl->name, fl->class_count );
  send_to_charf(ch," Saving script file: %s.script [%d programs]\n\r", fl->name, fl->class_count );
  for ( int i = 0; i < fl->class_count; i++ ) {
    CLASS_DATA *cl = fl->class_list[i];
    if ( SCRIPT_VERBOSE > 1 ) {
      log_stringf("  Class: [[%s]]\n\r", class_header( cl ) );
    }
    bool stop = FALSE;
    show_fct_head = NULL; // SinaC 2003
    FCT_DATA *f;
    for ( int j = 0; j < HASH_FCT; j++ ) {
      for ( f = cl->methods[j]; f; f = f->next ) {
	if ( f->scriptCodeSaved == NULL ) {
	  stop = TRUE;
	  break;
	}
	insert_show_fct( f ); // SinaC 2003
      }
      if ( stop )
	break;
    }
    if ( stop ) {
      send_to_charf(ch,"Program %s contains a non-definitive method %s.\n\r"
		    "-> NOT SAVED. Please finalize %s.\n\r", cl->name, f->name, cl->name );
      log_stringf("save_one_script_file: Class %s contains a non-definitive method %s.", cl->name, f->name);
      break;
    }

    fprintf( fp, "%s\n",class_header( cl ) );
    for ( show_fct *p = show_fct_head; p != NULL; p = p->next_fct ) {
      FCT_DATA *f = p->f;
      if ( SCRIPT_VERBOSE > 1 ) {
	log_stringf("    Method: %s\n\r", f->name );
      }
      strcpy( large_buffer, f->scriptCodeSaved );
      strip_char( large_buffer, '\r' ); // strip CR before saving
      strip_char_leading( large_buffer, '\n' ); // strip leading CR
      //fwrite( large_buffer, 1, strlen(large_buffer), fp );
      fprintf( fp, large_buffer );
    }
//    for ( int j = 0; j < HASH_FCT; j++ ) {  SinaC 2003
//      FCT_DATA *f = fl->class_list[i]->methods[j];
//      while ( f != NULL ) {
//	if ( SCRIPT_VERBOSE > 1 ) {
//	  log_stringf("    Method: %s\n\r", f->name );
//	}
//	strcpy( large_buffer, f->scriptCodeSaved );
//	strip_char( large_buffer, '\r' ); // strip CR before saving
//	strip_char_leading( large_buffer, '\n' ); // strip leading CR
//	//fwrite( large_buffer, 1, strlen(large_buffer), fp );
//	fprintf( fp, large_buffer );
//	f = f->next;
//      }
//    }
    REMOVE_BIT( cl->flags, CLASS_NOT_SAVED );
    fprintf( fp, "}\n\n"); // end of a class
  }
  fclose(fp);
}
void script_save_syntax( CHAR_DATA *ch ) {
  send_to_charf(ch,"\n\r"
		"Syntax: psave changed      - saves all changed programs\n\r"
		"        psave all          - saves all programs\n\r");
}
void do_script_save( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_STRING_LENGTH];
  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    script_save_syntax( ch );
    return;
  }
  bool fAll = FALSE;
  bool fModified = FALSE;
  if ( !str_cmp( arg, "all" ) )
    fAll = TRUE;
  else if ( !str_cmp( arg, "changed" ) )
    fModified = TRUE;
  else {
    script_save_syntax( ch );
    return;
  }

  // 1.0: we create the list of every script files and every classes in these files
  if (!compute_file_list()) {
    send_to_charf(ch,"Problem while saving programs\n\r" );
    return; 
  }
  // 1.1: if the list has already been computed, maybe it needs an update
  update_file_list();

  // TEST
  //  for ( int i = 0; i < cur_script_file; i++ ) {
  //    send_to_charf(ch,"filename: %s\n\r", script_file_list[i].name );
  //    send_to_charf(ch,"to_save: %s   count: %d/%d\n\r", 
  //		  script_file_list[i].to_save?"true":"false", 
  //		  script_file_list[i].class_count, script_file_list[i].max_class_count );
  //    for ( int j = 0; j < script_file_list[i].class_count; j++ )
  //      send_to_charf(ch,"  class: %s\n\r", script_file_list[i].class_list[j]->name );
  //  }
  //  return;

  // 2.0: we save the scripts list
  log_string("Saving script list" );
  send_to_charf(ch,"Saving script list.\n\r");
  fclose(fpReserve);
  FILE *fp = fopen( SCRIPT_LIST, "w" );
  if ( fp == NULL ) {
    send_to_charf(ch,"Can't open script list %s", SCRIPT_LIST );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  for ( int i = 0; i < cur_script_file; i++ )
    fprintf( fp, "%s.script\n", script_file_list[i].name );
  fprintf( fp, "%c\n", END_OF_SCRIPT_LIST );
  fclose(fp);

  // 3.0: we save the scripts in the right file
  bool found = FALSE;
  for ( int i = 0; i < cur_script_file; i++ )
    if ( fAll
	 || ( fModified && script_file_list[i].to_save ) ) {
      save_one_script_file( ch, i );
      found = TRUE;
    }

  if ( found )
    send_to_charf(ch,"Done.\n\r");
  else
    send_to_charf(ch,"No programs need to be saved.\n\r");

  fpReserve = fopen( NULL_FILE, "r" );
}

// ------------------------------- EDIT

// Only alphanumeric and _ are allowed
// And cannot start with a numeric
bool is_valid_class_method_name( const char *name ) {
  const char *p = name;
  if ( *p >= '0' && *p <= '9' )
    return FALSE;
  while ( *p != '\0' ) {
    if ( *p == '_' || isalnum( *p ) )
      p++;
    else
      return FALSE;
  }
  return TRUE;
}

#define SCRIPTEDIT( fun ) bool fun( CHAR_DATA *ch, const char *argument )

const struct olc_cmd_type script_edit_table[] = {
  {   "commands",	show_commands	},

  {   "editmethod",     script_edit_edit_method },
  {   "addmethod",      script_edit_add_method  },
  {   "delmethod",      script_edit_del_method  },
  {   "file",           script_edit_file        },
  {   "name",           script_edit_name        },
  {   "abstract",       script_edit_abstract    },
  {   "parents",        script_edit_parents     },
  {   "update",         script_edit_update      },
  {   "revert",         script_edit_revert      },
  {   "definitive",     script_edit_definitive  },
  {   "copy",           script_edit_copy        },
  {   "copymethod",     script_edit_copy_method },
  {   "finalize",       script_edit_finalize    },
  {   "create",         script_edit_create      },
  {   "delete",         script_edit_delete      },
  {   "showrevert",     script_edit_show_revert },

  {   "?",		show_help	},
  {   "version",	show_version	},

  {	NULL,		0,		}
};

void script_edit( CHAR_DATA *ch, const char *argument ) {
  CLASS_DATA *pClass = NULL;
  char command[MAX_INPUT_LENGTH];
  char arg[MAX_INPUT_LENGTH];

  if ( !OLC_EDIT( ch, pClass ) )
    return;
  EDIT_SCRIPT( ch, pClass );

  strcpy( arg, argument );
  argument = one_argument( argument, command );

  if ( !CAN_EDIT_SCRIPT( ch ) ) {
    send_to_char("Insufficient security to edit program.\n\r", ch);
    return;
  }

  if ( !str_cmp(command, "done") ) {
    if ( !is_finalized( pClass ) || IS_SET( pClass->flags, CLASS_MODIFIED ) )
      send_to_charf(ch,"{RBE CAREFUL: Program not finalized.{x\n\r");
    edit_done( ch );
    return;
  }

  if ( command[0] == '\0' ) {
    script_edit_show( ch, argument );
    return;
  }

  // Search Table and Dispatch Command.
  for ( int cmd = 0; script_edit_table[cmd].name != NULL; cmd++ )
    if ( !str_prefix( command, script_edit_table[cmd].name ) ) {
      if ( (*script_edit_table[cmd].olc_fun) ( ch, argument ) ) {
	// We must get Class from pEdit because it may have been modified by script_edit_create
	EDIT_SCRIPT( ch, pClass ); // pClass can even be NULL after a script_edit_delete
	if ( pClass != NULL ) {
	  SET_BIT( pClass->flags, CLASS_MODIFIED );
	  SET_BIT( pClass->flags, CLASS_NOT_SAVED );
	}
	FILE_LIST_UPDATED = FALSE;
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
void do_script_edit( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_STRING_LENGTH];

  if ( !CAN_EDIT_SCRIPT( ch ) ) {
    send_to_char("Insufficient security to edit program.\n\r", ch );
    return;
  }

  argument = one_argument( argument, arg1 );
  if ( arg1[0] == '\0' ) {
    send_to_char("You must specify a program.\n\r"
		 "Syntax: scriptedit <program name>\n\r", ch );
    return;
  }

  CLASS_DATA *pClass;
  if ( !str_cmp( arg1, "create" ) ) {
    if ( script_edit_create( ch, argument ) )
      EDIT_SCRIPT( ch, pClass );
    else
      return;
  }
  else if ( !str_cmp( arg1, "delete" ) ) {
    script_edit_delete( ch, argument );
    return;
  }
  else {
    pClass = silent_hash_get_prog( arg1 );
    if ( pClass == NULL ) {
      send_to_charf(ch,"Program (%s) doesn't exist.\n\r", arg1 );
      return;
    }
    if ( isRootClass( pClass ) ) {
      send_to_charf(ch,"You CANNOT edit root programs.\n\r");
      return;
    }
  }
  if ( !OLC_EDIT( ch, pClass ) )
    return;
  
  REMOVE_BIT(ch->comm, COMM_BUILDING);
  if (!IS_SET(ch->comm,COMM_EDITING))
    send_to_char("You are now in {BEDITING{x mode.\n\r",ch);
  SET_BIT(ch->comm, COMM_EDITING);

  ch->desc->pEdit = (void *)pClass;
  ch->desc->editor = ED_SCRIPT;
  return;
}

// Script editing methods
void show_fct_list( CHAR_DATA *ch, CLASS_DATA *pClass, bool trigger ) {
  bool found = FALSE;
  show_fct_head = NULL;
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    while ( f != NULL ) {
      TriggerDescr *t = isTrigger(f);
      if ( t != NULL && trigger
	   || t == NULL && !trigger ) {
	found = TRUE;
	insert_show_fct(f);
      }
      f = f->next;
    }
  }
  if ( !found ) {
    send_to_charf(ch,"  no %s.\n\r", trigger?"triggers":"methods");
    return;
  }
  for ( show_fct *p = show_fct_head; p != NULL; p = p->next_fct ) {
    FCT_DATA *f = p->f;
    bool pFound = FALSE;
    send_to_charf(ch,"<%2d>", f->positionId );
    if ( IS_SET( f->flags, FCT_NO_CODE ) ) {
      send_to_charf(ch,"{R[NO CODE]{x ");
      pFound = TRUE;
    }
    if ( IS_SET( f->flags, FCT_MODIFIED ) ) {
      send_to_charf(ch,"{G[NOT UPDATED]{x ");
      pFound = TRUE;
    }
    if ( f->scriptCodeSaved == NULL ) {
      send_to_charf(ch,"{B[NO REVERT]{x ");
      pFound = TRUE;
    }
    if ( f->scriptCode != NULL
	 && f->scriptCodeSaved != NULL
	 && strcmp( f->scriptCodeSaved, f->scriptCode ) ) {
      send_to_charf(ch,"{Y[NO DEFINITIVE]{x ");
      pFound = TRUE;
    }
    if ( !pFound )
      send_to_charf(ch,"       ");
    if ( f->nbparm == -1 )
      send_to_charf(ch,"%s (variable number of parameters)\n\r", f->name );
    else if ( f->nbparm == 0 )
      send_to_charf(ch,"%s ( )\n\r", f->name);
    else {
      send_to_charf(ch,"%s (", f->name);
      for ( int i = 0; i < f->nbparm; i++ ) {
	if ( f->parmname[i] == NULL )
	  send_to_charf(ch," <no name>" );
	else
	  send_to_charf(ch," %s", f->parmname[i] );
	if ( i+1 < f->nbparm )
	  send_to_charf(ch,",");
      }
      send_to_char(" )\n\r",ch);
    }
  }
}
//void show_fct_list( CHAR_DATA *ch, CLASS_DATA *pClass, bool trigger ) {
//  int count = 0;
//  bool found = FALSE;
//  for ( int i = 0; i < HASH_FCT; i++ ) {
//    FCT_DATA *f= pClass->methods[i];
//    while ( f != NULL ) {
//      TriggerDescr *t = isTrigger(f);
//      if ( t != NULL && trigger
//	   || t == NULL && !trigger ) {
//	found = TRUE;
//	bool pFound = FALSE;
//	if ( IS_SET( f->flags, FCT_NO_CODE ) ) {
//	  send_to_charf(ch,"{R[NO CODE]{x ");
//	  pFound = TRUE;
//	}
//	if ( IS_SET( f->flags, FCT_MODIFIED ) ) {
//	  send_to_charf(ch,"{G[NOT UPDATED]{x ");
//	  pFound = TRUE;
//	}
//	if ( f->scriptCodeSaved == NULL ) {
//	  send_to_charf(ch,"{B[NO REVERT]{x ");
//	  pFound = TRUE;
//	}
//	if ( f->scriptCode != NULL
//	     && f->scriptCodeSaved != NULL
//	     && strcmp( f->scriptCodeSaved, f->scriptCode ) ) {
//	  send_to_charf(ch,"{Y[NO DEFINITIVE]{x ");
//	  pFound = TRUE;
//	}
//	if ( !pFound )
//	  send_to_charf(ch,"       ");
//	if ( f->nbparm == -1 )
//	  send_to_charf(ch,"%s (variable number of parameters)\n\r", f->name );
//	else if ( f->nbparm == 0 )
//	  send_to_charf(ch,"%s ( )\n\r", f->name);
//	else {
//	  send_to_charf(ch,"%s (", f->name);
//	  for ( int i = 0; i < f->nbparm; i++ ) {
//	    if ( f->parmname[i] == NULL )
//	      send_to_charf(ch," <no name>" );
//	    else
//	      send_to_charf(ch," %s", f->parmname[i] );
//	    if ( i+1 < f->nbparm )
//	      send_to_charf(ch,",");
//	  }
//	  send_to_char(" )\n\r",ch);
//	}
//	count++;
//      }
//      f = f->next;
//    }
//  }
//  if ( !found )
//    send_to_charf(ch,"  no %s.\n\r", trigger?"triggers":"methods");
//}
SCRIPTEDIT( script_edit_show ) {
  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  send_to_charf(ch,"Name:               [%s]\n\r", pClass->name );
  send_to_charf(ch,"File:               [%s]\n\r", pClass->file?pClass->file:"NO FILE" );
  send_to_charf(ch,"Abstract:           [%s]\n\r", pClass->isAbstract?"Yes":"No");
  CLASS_DATA *root = get_root_class(pClass);
  send_to_charf(ch,"Root program:       [%s]\n\r", root?root->name:"NO ROOT");
  send_to_charf(ch,"Parents:            ");
  if ( pClass->parents_count == 0 )
    send_to_charf(ch,"Root\n\r");
  else
    for ( int i = 0; i < pClass->parents_count; i++ ) {
      if ( i != 0 )
	send_to_charf(ch,"                    ");
      send_to_charf(ch,"%s\n\r", pClass->parents[i]->name );
  }
  if ( pClass->methods == NULL )
    send_to_charf(ch,"NO methods/triggers\n\r");
  else {
    send_to_charf(ch,"Methods:\n\r");
    show_fct_list( ch, pClass, FALSE );
    send_to_charf(ch,"Triggers:\n\r");
    show_fct_list( ch, pClass, TRUE );
  }
  return FALSE;
}
SCRIPTEDIT( script_edit_edit_method ) {
  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  editmethod <method name>\n\r");
    return FALSE;
  }

  FCT_DATA *f = hash_get_fct( argument, pClass->methods );
  if ( f == NULL ) {
    send_to_charf(ch,"This method/trigger doesn't exist.\n\r");
    return FALSE;
  }

  if ( ch->desc->stringColorMode ) {
    send_to_charf(ch,"String editing color mode desactivated.\n\r" );
    ch->desc->stringColorMode = FALSE;
  }
  if ( !ch->desc->lineMode ) {
    send_to_charf(ch,"Line number mode activated.\n\r");
    ch->desc->lineMode = TRUE;
  }

  SET_BIT( f->flags, FCT_MODIFIED );
  // update_after_edit_method will be called with param f, when leaving SEditor
  ch->desc->pStringFun = (void*) update_after_edit_method; // function to call after string edition
  ch->desc->pStringArg = (void*) f; // function parameter

  string_append( ch, &f->scriptCode );

  return TRUE;
}
// Modifiy method positionId by adding 'add' if positionId is >= 'fromId'
void update_method_positionId( CLASS_DATA *pClass, const int fromId, const int add ) {
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    while ( f != NULL ) {
      if ( f->positionId >= fromId )
	f->positionId += add;
      f = f->next;
    }
  }
}
// return highest positionId
int get_last_positionId( CLASS_DATA *pClass ) {
  int positionId = 0;
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    while ( f != NULL ) {
      if ( f->positionId >= positionId )
	positionId = f->positionId;
      f = f->next;
    }
  }
  return positionId;
}
SCRIPTEDIT( script_edit_add_method ) {
  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  FCT_DATA *f = NULL;

  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  argument = no_lower_one_argument( argument, arg ); // new method name
  argument = no_lower_one_argument( argument, arg1 ); // new position id (before a method)
  bool found = FALSE;
  if ( arg[0] != '\0' ) { // method name specified-> is a trigger?
    TriggerDescr* t;
    for (t = triggers; t->name != NULL; t++)
      if (!str_cmp(t->name, arg)) {
	found = TRUE;
	break;
      }
    if ( found ) { // it's a trigger name
      FCT_DATA *trig = hash_get_fct( t->name, pClass->methods );
      if ( trig != NULL ) { // trigger already exists
	send_to_charf(ch,"Trigger %s already exists.\n\r", t->name );
	return FALSE;
      }
      // Create a new function
      f = new_fct();
      f->nbparm = t->nparms;
      // a valid/non-existing trigger has been specified
      //  so we add the trigger header to stringCode
      char buf[MAX_STRING_LENGTH];
      char buf2[MAX_STRING_LENGTH];
      int idx = 0;
      buf2[0] = '\0';
      for ( int i = 0; i < t->nparms; i++ ) { // Create param list
	if ( t->parmname[i] != NULL ) { // parm has a name?
	  strcat( buf2, t->parmname[i] );
	  f->parmname[i] = str_dup(t->parmname[i], TRUE);
	}
	else { // if not we create one  parmX
	  char buf3[MAX_STRING_LENGTH];
	  sprintf( buf3, "parm%d", idx );
	  idx++;
	  strcat( buf2, buf3 );
	  f->parmname[i] = str_dup(buf3, TRUE);
	}
	if ( i < t->nparms-1 )
	  strcat( buf2, ", ");
      }
      sprintf( buf, "%s ( %s ) {\n\r", t->name, buf2  );
      f->scriptCode = str_dup( buf, TRUE );
      f->name = str_dup(t->name, TRUE); // we set name to trigger name
      send_to_charf(ch,"%s is a trigger.\n\r", t->name );
    }
  }
  if ( !found ) { // if a name was specified but not a trigger or no name was specified
    // We search if defaultFunctionName already exists, we can only edit one function
    // To create a new function the last created incorrected function must be completed
    f = hash_get_fct( defaultFunctionName, pClass->methods );
    if ( f != NULL ) {
      send_to_charf(ch,"There is already a new method which is not updated.\n\r"
		    "You cannot create a new method if there is an un-updated method.\n\r");
      return FALSE;
    }
    
    // We create a new function and add it to Class' methods
    // But be careful, this function is not completed: no code, no scriptCode, ...
    // It will be completed in update_after_edit_method
    f = new_fct();
    f->name = str_dup(defaultFunctionName, TRUE); // we set name to an artificial value
  }
  f->incoming = pClass; // we store a ptr to the Class containing this method
  // positionId can be specified -> modify other method's positionId    TO DO
  //f->positionId = pClass->lastPositionId + 1;
  if ( arg1[0] == '\0' ) // no position specified
    f->positionId = get_last_positionId( pClass )+1;
  else {
    FCT_DATA *f2 = hash_get_fct( arg1, pClass->methods );
    if ( f2 == NULL ) { // method not found
      send_to_charf( ch, "Method [%s] not found, inserting in end.\n\r", arg1 );
      f->positionId = get_last_positionId( pClass )+1;
    }
    else {
      f->positionId = f2->positionId; // new method gets old method position
      update_method_positionId( pClass, f2->positionId, 1 ); // update every method positionId
    }
  }
  send_to_charf(ch,"---->Method #%d\n\r",f->positionId);

  SET_BIT( f->flags, FCT_MODIFIED );
  SET_BIT( f->flags, FCT_NO_CODE );
  hash_put_fct( f, pClass->methods );

  if ( ch->desc->stringColorMode ) {
    send_to_charf(ch,"String editing color mode desactivated.\n\r" );
    ch->desc->stringColorMode = FALSE;
  }
  if ( !ch->desc->lineMode ) {
    send_to_charf(ch,"Line number mode activated.\n\r");
    ch->desc->lineMode = TRUE;
  }
  // update_after_edit_method will be called with param f, when leaving SEditor
  ch->desc->pStringFun = (void*) update_after_edit_method;
  ch->desc->pStringArg = (void*) f;
  string_append( ch, &f->scriptCode );
  return TRUE;
}
SCRIPTEDIT( script_edit_del_method ) {

  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  delmethod [method/trigger name]\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  int idx;
  FCT_DATA *prev;
  FCT_DATA *f = hash_get_fct_prev( argument, pClass->methods, prev, idx );
  if ( f == NULL ) {
    send_to_charf(ch, "Method/trigger %s doesn't exist.\n\r", argument );
    return FALSE;
  }
  if ( f == prev ) // head of list
    pClass->methods[idx] = f->next;
  else
    prev->next = f->next;

  send_to_charf( ch, "Method/trigger %s deleted.\n\r", f->name );
  
  return TRUE;
}
SCRIPTEDIT( script_edit_file ) {
  char arg[MAX_STRING_LENGTH];
  no_lower_one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  file [file name]\n\r");
    return FALSE;
  }

  if ( !is_valid_class_method_name(arg) ) {
    send_to_charf(ch,"%s is not a valid name, only alphanumeric and '_' are allowed.", arg );
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  pClass->file = str_dup(arg, TRUE);
  send_to_charf(ch,"Program file name set.\n\r");

  FILE_LIST_COMPUTED = FALSE;

  return TRUE;
}
SCRIPTEDIT( script_edit_abstract ) {
  int val = -1;
  if ( !str_cmp( argument, "yes" ) )
    val = 1;
  else if ( !str_cmp( argument, "no" ) )
    val = 0;
  if ( argument[0] == '\0'
       || val == -1 ) {
    send_to_charf(ch,"Syntax:  abstract  [yes/no]\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  pClass->isAbstract = val;
  send_to_charf(ch,"Abstract %s.\n\r", pClass->isAbstract?"set":"removed");

  return TRUE;
}
SCRIPTEDIT( script_edit_parents ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  parents [parent name]\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  CLASS_DATA * parent = hash_get_prog( argument );
  if ( parent == NULL ) {
    send_to_charf(ch,"Program %s doesn't exist.\n\r", argument );
    return FALSE;
  }
  bool found = find_parent( pClass, parent );
  if ( !found ) { // add
    if ( get_root_class(parent) != get_root_class(pClass) ) { // check roots
      send_to_charf(ch,"%s has a different root program than %s.\n\r", parent->name, pClass->name );
      return FALSE;
    }

    // check if this new parent will create a cycle
    try {
      check_cyclic_inheritance_one_class( pClass, parent );
    } catch (GeneralException g) {
      send_to_charf(ch,"Error when adding %s as parent: %s\n\r", parent->name, g.msg );
      return FALSE;
    }

    add_one_parent( pClass, parent );
    send_to_charf(ch,"Parent added.\n\r");
  }
  else { // remove
    if ( pClass->parents_count == 1 ) { // only 1 parent: can't remove it
      send_to_charf(ch,"You can't remove the last parent.\n\r");
      return FALSE;
    }
    remove_one_parent( pClass, parent );
    send_to_charf(ch,"Parent removed.\n\r");
  }

  return TRUE;
}
SCRIPTEDIT( script_edit_update ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  update [method/trigger name  | all]\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  // Remove extra parents: such as default_XXX_class
  remove_extra_parents( pClass );

  bool found = FALSE;
  bool fAll = FALSE;
  if ( !str_cmp(argument, "all" ) )
    fAll = TRUE;

  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    FCT_DATA *prev = pClass->methods[i];
    while ( f != NULL ) {
      if ( fAll
	   || !strcmp( f->name, argument ) ) {
	found = TRUE;
	update_one_method( ch, pClass, prev, f, i );
      }
      prev = f;
      f = f->next;
    }
  }
  if ( is_finalized(pClass) )
    REMOVE_BIT( pClass->flags, CLASS_MODIFIED );
  if ( !found ) {
    send_to_charf(ch,"Method/trigger %s not found.\n\r", argument );
    return FALSE;
  }

  return TRUE;
}
SCRIPTEDIT( script_edit_revert ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  revert [method/trigger name  | all]\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  bool found = FALSE;
  bool fAll = FALSE;
  if ( !str_cmp(argument, "all" ) )
    fAll = TRUE;

  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    while ( f != NULL ) {
      if ( fAll
	   || !strcmp( f->name, argument ) ) {
	found = TRUE;
	if ( f->scriptCodeSaved == NULL )
	  send_to_charf(ch,"%s doesn't have revert code.\n\r", f->name );
	else {
	  f->scriptCode = str_dup( f->scriptCodeSaved, TRUE );
	  send_to_charf(ch,"%s reverted.\n\r", f->name );
	  SET_BIT( f->flags, FCT_MODIFIED );
	}
      }
      f = f->next;
    }
  }
  if ( !found ) {
    send_to_charf(ch,"Method/trigger %s doesn't exist.\n\r", argument );
    return FALSE;
  }
  return TRUE;
}
SCRIPTEDIT( script_edit_definitive ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  definitive [method/trigger name  | all]\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  bool found = FALSE;
  bool fAll = FALSE;
  if ( !str_cmp(argument, "all" ) )
    fAll = TRUE;
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    while ( f != NULL ) {
      if ( fAll
	   || !strcmp( f->name, argument ) ) {
	found = TRUE;
	if ( IS_SET( f->flags, FCT_NO_CODE ) )
	  send_to_charf(ch,"%s can't be definitive because has no valid code.\n\r", f->name );
	else {
	  f->scriptCodeSaved = str_dup( f->scriptCode, TRUE );
	  send_to_charf(ch,"%s revert code replaced with current code.\n\r", f->name );
	}
      }
      f = f->next;
    }
  }
  if ( !found ) {
    send_to_charf(ch,"Method/trigger %s doesn't exist.\n\r", argument );
    return FALSE;
  }
  return TRUE;
}
bool copy_method( CHAR_DATA *ch, FCT_DATA *f, CLASS_DATA *pClass, const char *methodName ) {
  if ( f->code == NULL
       || f->scriptCode == NULL
       || f->scriptCodeSaved == NULL
       || f->name == NULL ) {
    send_to_charf(ch,"Method/trigger %s misses informations to be copied:\n\r%s%s%s\n\r",
		  f->name?f->name:" [NO NAME]",
		  f->code?"":" [NO CODE]",
		  f->scriptCode?"":" [NO STRING CODE]",
		  f->scriptCodeSaved?"":" [NO REVERT CODE]" );
    return FALSE;
  }

  FCT_DATA *nf = new_fct(); // create a new function and copy information
  for (int i = 0; i < f->nbparm; i++)
    nf->parmname[i] = str_dup(f->parmname[i], TRUE);
  nf->nbparm = f->nbparm;
  if ( methodName[0] == '\0' ) // just to be sure
    nf->name = str_dup(f->name, TRUE);
  else
    nf->name = str_dup(methodName, TRUE);
  nf->scriptCode = str_dup( f->scriptCode, TRUE );
  nf->scriptCodeSaved = str_dup( f->scriptCode, TRUE );
  nf->code = f->code; // code are the same until we do an update
  nf->flags = f->flags;

  nf->incoming = pClass; // we store a ptr to the Class containing this method
  hash_put_fct( nf, pClass->methods ); // add function to class

  send_to_charf(ch,"Method/trigger %s copied.\n\r", nf->name );
  return TRUE;
}
SCRIPTEDIT( script_edit_copy_method ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  copymethod  [program name] [method name] <[new method name]>\n\r");
    return FALSE;
  }
  CLASS_DATA *cl = silent_hash_get_prog( arg1 );
  if ( cl == NULL ) {
    send_to_charf(ch,"Program %s doesn't exist.\n\r", arg1 );
    return FALSE;
  }
  FCT_DATA *f = hash_get_fct( arg2, cl->methods );
  if ( f == NULL ) {
    send_to_charf(ch,"Program %s doesn't have any method/trigger called %s.\n\r", arg1, arg2 );
    return FALSE;
  }
  
  if ( isRootClass( cl ) ) {
    send_to_charf(ch,"You CANNOT copy method from root programs.\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  if ( get_root_class( pClass ) != get_root_class( cl ) ) {
    send_to_charf(ch,"You can't copy method from a program which doesn't have the same root program.");
    return FALSE;
  }

  // Arg3 will be the name of the copied method, if specified...
  if ( arg3[0] == '\0' ) {
    if ( hash_get_fct( f->name, pClass->methods ) ) {
      send_to_charf(ch,"This program already has a method/trigger called %s.\n\r", arg2 );
      return FALSE;
    }
    return copy_method( ch, f, pClass, f->name );
  }
  else {
    if ( hash_get_fct( arg3, pClass->methods ) ) {
      send_to_charf(ch,"This program already has a method/trigger called %s.\n\r", arg3 );
      return FALSE;
    }
    return copy_method( ch, f, pClass, arg3 );
  }
  return FALSE; // should never come here.
}
SCRIPTEDIT( script_edit_copy ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  copy  [program name]\n\r");
    return FALSE;
  }
  CLASS_DATA *cl = silent_hash_get_prog( argument );
  if ( cl == NULL ) {
    send_to_charf(ch,"Program %s doesn't exist.\n\r", argument );
    return FALSE;
  }

  if ( isRootClass( cl ) ) {
    send_to_charf(ch,"You CANNOT copy root programs.\n\r");
    return FALSE;
  }
  
  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  int pCount = cl->parents_count; // we first do the allocations, if there is a problem
  CLASS_DATA **parents;           //  the class has not yet been modified
  if ( ( parents = (CLASS_DATA **)GC_MALLOC(sizeof(CLASS_DATA*)*pCount) ) == NULL ) {
    bug("Memory allocation error: parents" );
    return FALSE;
  }
  FCT_DATA **methods;
  if ( ( methods = (FCT_DATA**)GC_MALLOC(HASH_FCT*sizeof(FCT_DATA*))) == NULL ) {
    bug("Memory allocation error: methods" );
    return FALSE;
  }

  // Copy information
  pClass->name = cl->name; // name
  // filename not copied

  pClass->parents = parents; // parents
  pClass->parents_count = pCount;
  for ( int i = 0; i < pCount; i++ )
    pClass->parents[i] = cl->parents[i];
  pClass->isAbstract = cl->isAbstract;

  pClass->methods = methods; // methods
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f = cl->methods[i];
    while ( f != NULL ) {
      copy_method( ch, f, pClass, f->name );
      f = f->next; // next function
    }
  }
  pClass->flags = cl->flags;

  send_to_charf(ch,"Program copied.\n\r");
  if ( !str_cmp( pClass->file, cl->file ) )
    send_to_charf(ch,"{RBE CAREFUL:{x the program you just copied is in the same file as the one you're editing, change its name or its file before saving.\n\r");

  return TRUE;
}
SCRIPTEDIT( script_edit_name ) {
  char buf[MAX_STRING_LENGTH];
  strcpy( buf, argument );
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  name  [program name]\n\r");
    return FALSE;
  }
  CLASS_DATA *cla = hash_get_prog( argument );
  if ( cla != NULL ) {
    send_to_charf(ch,"There is already a program named %s.\n\r", argument );
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  // when we modify Class name, we have to remove it from hash list and then re-hash it
  int idx;
  CLASS_DATA *prev;
  CLASS_DATA *cl = hash_get_prog_prev( pClass, prev, idx );
  if ( cl == NULL ) {
    send_to_charf(ch,"BIG PROBLEM with script_edit_name");
    return FALSE;
  }

  // remove from hash list
  if ( prev == cl ) // head of list
    prg_table[idx] = cl->next;
  else
    prev->next = cl->next;

  // re-hash it
  pClass->name = str_dup( buf, TRUE );
  hash_put_prog(pClass);

  send_to_charf(ch,"Name set.\n\r");
  return TRUE;
}
SCRIPTEDIT( script_edit_create ) {
  char arg2[MAX_STRING_LENGTH];
  char arg3[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  create <program name> [<parent program name> (default is Mob)]\n\r");
    return FALSE;
  }
  CLASS_DATA *cl = silent_hash_get_prog( arg2 );
  if ( cl != NULL ) {
    send_to_charf(ch,"Program %s already exists!\n\r", arg2 );
    return FALSE;
  }
  if ( !is_valid_class_method_name(arg2) ) {
    send_to_charf(ch,"%s is not a valid name, only alphanumeric and '_' are allowed.", arg2 );
    return FALSE;
  }
  if ( !CAN_EDIT_SCRIPT( ch ) ) {
    send_to_char("Insufficient security to edit program.\n\r", ch);
    return FALSE;
  }
  CLASS_DATA *default_parent = default_mob_class;
  if ( arg3[0] != '\0' ) { // if a parent is specified
    default_parent = silent_hash_get_prog( arg3 );
    if ( default_parent == NULL ) {
      send_to_charf(ch,"Program %s doesn't exist.\n\r", arg3 );
      return FALSE;
    }
  }
  CLASS_DATA *pClass = new_prg();
  pClass->name = str_dup( arg2, TRUE );
  add_one_parent( pClass, default_parent );
  SET_BIT( pClass->flags, CLASS_MODIFIED );
  SET_BIT( pClass->flags, CLASS_NOT_SAVED );
  hash_put_prog( pClass );
  send_to_charf(ch,"Program %s created.\n\r",arg2);
  
  ch->desc->pEdit = (void *)pClass;
  ch->desc->editor = ED_SCRIPT;

  MAX_SCRIPTS++;

  FILE_LIST_COMPUTED = FALSE;
  
  return TRUE;
}
void update_entities( CLASS_DATA *pClass ) {
  // Call onCreate and onRepop for each entities having this class
  CLASS_DATA *root = get_root_class( pClass );
  if ( root == default_mob_class ) {
    CHAR_DATA *ch_next;
    for ( CHAR_DATA *ch = char_list; ch; ch = ch_next ) {
      ch_next = ch->next;
      if ( ch->in_room != NULL && ch->clazz == pClass ) {
	MOBPROG( ch, NULL, "onCreate" );
	MOBPROG( ch, NULL, "onRepop", Value((long)0) ); // 0 means reloading/modifying scripts
      }
    }
  }
  else if ( root == default_obj_class ) {
    OBJ_DATA *obj_next;
    for ( OBJ_DATA *obj = object_list; obj; obj = obj_next ) {
      obj_next = obj->next;
      if ( obj->clazz == pClass ) {
	OBJPROG( obj, NULL, "onCreate" );
	OBJPROG( obj, NULL, "onRepop", Value((long)0) ); // 0 means reloading/modifying scripts
      }
    }
  }
  else if ( root == default_room_class ) {
    for ( int i = 0; i < 65535; i++ ) {
      ROOM_INDEX_DATA *room;
      if ( ( room = get_room_index( i ) ) == NULL )
	continue;
      if ( room->clazz == pClass ) {
	ROOMPROG( room, NULL, "onCreate" );
	ROOMPROG( room, NULL, "onRepop", Value((long)0) ); // 0 means reloading/modifying scripts
      }
    }
  }
  else
    bug("update_entities: wrong root program");
}
SCRIPTEDIT( script_edit_finalize ) {
  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);

  remove_extra_parents( pClass );

  // Update and definitive every modified methods
  for ( int i = 0; i < HASH_FCT; i++ ) {
    FCT_DATA *f= pClass->methods[i];
    FCT_DATA *prev = pClass->methods[i];
    while ( f != NULL ) {
      if ( IS_SET( f->flags, FCT_MODIFIED ) // only update modified/newly created methods
	   || IS_SET( f->flags, FCT_NO_CODE ) ) {
	// update
	if ( !update_one_method( ch, pClass, prev, f, i ) ) {
	  send_to_charf(ch,"{%s not updated => {RFINALIZE ABORTED{x.\n\r", f->name );
	  return TRUE;
	}
      }
      // if update successfull, replace revert code
      f->scriptCodeSaved = str_dup( f->scriptCode, TRUE );

      prev = f;
      f = f->next;
    }
  }

  update_entities( pClass );

  send_to_charf(ch,"Finalize done.\n\r");
  // Class is not modified because every methods are correct and revert code has been replaced
  if ( is_finalized(pClass) )
    REMOVE_BIT( pClass->flags, CLASS_MODIFIED );
  return FALSE;
}
// Set entity's clazz to default_XXX_class
// And set _index program to NULL
void rootify_entities( CHAR_DATA *ch, CLASS_DATA *pClass ) {
  CLASS_DATA *root = get_root_class( pClass );
  if ( root == default_mob_class ) {
    CHAR_DATA *vch_next;
    for ( CHAR_DATA *vch = char_list; vch; vch = vch_next ) {
      vch_next = vch->next;
      if ( vch->clazz == pClass )
	if ( IS_NPC(vch) )
	  vch->clazz = default_mob_class;
	else
	  vch->clazz = default_player_class;
    }
    for ( int i = 0; i < 65535; i++ ) {
      MOB_INDEX_DATA *mob;
      if ( ( mob = get_mob_index( i ) ) == NULL )
	continue;
      if ( mob->program == pClass ) {
	mob->program = NULL;
	send_to_charf(ch,"Mob vnum %d program removed.\n\r", i );
      }
    }
  }
  else if ( root == default_obj_class ) {
    OBJ_DATA *obj_next;
    for ( OBJ_DATA *obj = object_list; obj; obj = obj_next ) {
      obj_next = obj->next;
      if ( obj->clazz == pClass )
	obj->clazz = default_obj_class;
    }
    for ( int i = 0; i < 65535; i++ ) {
      OBJ_INDEX_DATA *obj;
      if ( ( obj = get_obj_index( i ) ) == NULL )
	continue;
      if ( obj->program == pClass ) {
	obj->program = NULL;
	send_to_charf(ch,"Obj vnum %d program removed.\n\r", i );
      }
    }
  }
  else if ( root == default_room_class ) {
    for ( int i = 0; i < 65535; i++ ) {
      ROOM_INDEX_DATA *room;
      if ( ( room = get_room_index( i ) ) == NULL )
	continue;
      if ( room->clazz == pClass ) {
	room->program = NULL;
	room->clazz = default_room_class;
	send_to_charf(ch,"Room vnum %d program removed.\n\r", i );
      }
    }
  }
  else
    bug("rootify_entities: wrong root program");
}
SCRIPTEDIT( script_edit_delete ) {
  char arg2[MAX_STRING_LENGTH];
  one_argument( argument, arg2 );
  if ( arg2[0] == '\0' ) {
    send_to_charf(ch,"Syntax: delete <program name>\n\r");
    return FALSE;
  }
  if ( !CAN_EDIT_SCRIPT( ch ) ) {
    send_to_char("Insufficient security to edit program.\n\r", ch);
    return FALSE;
  }
  int idx;
  CLASS_DATA *prev;
  CLASS_DATA *pClass = hash_get_prog_prev( arg2, prev, idx );
  if ( pClass == NULL ) {
    send_to_charf(ch,"Program %s doesn't exist.\n\r", arg2 );
    return FALSE;
  }

  if ( !CAN_EDIT( ch, pClass ) ) {
    send_to_charf(ch,"{RLOCKED:{x edited by %s.\n\r", editor_name(pClass));
    return FALSE;
  }

  if ( pClass == default_player_class
       || isRootClass( pClass ) ) {
    send_to_charf(ch,"You can't delete root programs neither player program.\n\r");
    return FALSE;
  }

  // remove from hash list
  if ( prev == pClass ) // head of list
    prg_table[idx] = pClass->next;
  else
    prev->next = pClass->next;

  rootify_entities( ch, pClass );
  MAX_SCRIPTS--;

  send_to_charf(ch,"Program %s deleted.\n\r", arg2 );
  if ( pClass == ch->desc->pEdit ) // just deleted the class we were editing -> done
    edit_done(ch);

  return TRUE;
}
SCRIPTEDIT( script_edit_show_revert ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"Syntax:  showrevert <trigger/method name>\n\r");
    return FALSE;
  }

  CLASS_DATA *pClass;
  EDIT_SCRIPT(ch, pClass);
  FCT_DATA *f = hash_get_fct( argument, pClass->methods );
  if ( f == NULL ) {
    send_to_charf(ch,"Trigger/Method [%s] not found.\n\r", argument );
    return FALSE;
  }
  if ( f->scriptCodeSaved == NULL )
    send_to_charf(ch,"%s doesn't have revert code.\n\r", f->name );
  else {
    //send_to_charf(ch,"[%s] revert code:\n\r%s\n\r", f->name, f->scriptCodeSaved );
    send_to_charf(ch,"[%s] revert code:\n\r", f->name );
    bool colorMode = ch->desc->stringColorMode;
    ch->desc->stringColorMode = FALSE;
    string_show_without_color( f->scriptCodeSaved, ch );
    ch->desc->stringColorMode = colorMode;
  }
  return FALSE;
}
