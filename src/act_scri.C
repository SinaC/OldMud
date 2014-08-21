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

// Added by SinaC 2001
#include "act_info.h"
#include "db.h"
#include "comm.h"
#include "scrhash.h"
#include "recycle.h"
#include "dbscript.h"
#include "interp.h"


void show_fct(CHAR_DATA * ch, FCT_DATA * fct) {
  if ( isTrigger( fct ) == NULL )
    send_to_charf(ch,"    fct name: ");
  else
    send_to_charf(ch,"trigger name: ");

  if ( fct->nbparm == -1 )
    send_to_charf(ch,"%20s ( variable number of parameters )\n\r", fct->name );
  else if ( fct->nbparm == 0 )
    send_to_charf(ch,"%20s ( )\n\r", fct->name);
  else {
    send_to_charf(ch,"%20s (", fct->name);
    for ( int i = 0; i < fct->nbparm; i++ ) {
      if ( fct->parmname[i] == NULL )
	send_to_charf(ch," no name" );
      else
	send_to_charf(ch," %s", fct->parmname[i] );
      if ( i+1 < fct->nbparm )
	send_to_charf(ch,",");
    }
    send_to_char(" )\n\r",ch);
  }
}

void dump_script_file( CHAR_DATA *ch, const char *argument ) {
  char buf[ MAX_STRING_LENGTH ];
  char buf1[ MAX_STRING_LENGTH ];
  char arg[MAX_INPUT_LENGTH];
  int  col;

  compute_file_list();
  
  argument = one_argument( argument, arg );

  // display every files
  if ( arg[0] == '\0' ) {
    send_to_char("Available script files:\n\r", ch );
    buf1[0] = '\0';
    col = 0;
    for ( int i = 0; i < cur_script_file; i++ )
      if ( script_file_list[i].name != NULL ) {
	sprintf( buf, "%-20s", script_file_list[i].name );
	strcat( buf1, buf );
	if ( ++col % 4 == 0 )
	  strcat( buf1, "\n\r" );
      }
    if ( col % 4 != 0 )
      strcat( buf1, "\n\r" );
    send_to_char(buf1,ch);
    send_to_char("To see available programs in a script file:\n\r"
		 "Use: program list <file name>\n\r", ch );
    return;
  }

  // search a filename in list
  int index = -1;
  for ( int i = 0; i < cur_script_file; i++ ) {
    if ( script_file_list[i].name == NULL )
      break;
    if ( !str_cmp( script_file_list[i].name, arg ) ) {
      index = i;
      break;
    }
  }
  if ( index == -1 ) {
    send_to_charf(ch,"Script file %s not available.\n\r", arg );
    return;
  }

  // display every classes in the file
  send_to_charf(ch,
		"<prg name> are abstract programs: they cannot be assigned directly to a mob,\n\r"
		"  obj, room. They have to be derivated first.\n\r");
  buf1[0] = '\0';
  col = 0;
  file_list *fl = &(script_file_list[index]);
  if ( fl->class_count > 0 ) {
    bool foundMProg = FALSE,
      foundOProg = FALSE,
      foundRProg = FALSE;
    BUFFER *mobBuffer = new_buf();
    BUFFER *objBuffer = new_buf();
    BUFFER *roomBuffer = new_buf();
    int Mcol = 0,
      Ocol = 0,
      Rcol = 0;
    for ( int i = 0; i < fl->class_count; i++ ) {
      CLASS_DATA *cl = fl->class_list[i];
      CLASS_DATA *root = get_root_class( cl );
      char buf[MAX_STRING_LENGTH];
      if ( cl->isAbstract )
	sprintf( buf, "  <%-20s>", cl->name );
      else
	sprintf( buf, "   %-20s ", cl->name );
      if ( root == default_mob_class ) {
	add_buf( mobBuffer, buf );
	if ( ++Mcol % 3 == 0 )
	  add_buf( mobBuffer, "\n\r" );
	foundMProg = TRUE;
      }
      if ( root == default_obj_class ) {
	add_buf( objBuffer, buf );
	if ( ++Ocol % 3 == 0 )
	  add_buf( objBuffer, "\n\r" );
	foundOProg = TRUE;
      }
      if ( root == default_room_class ) {
	add_buf( roomBuffer, buf );
	if ( ++Rcol % 3 == 0 )
	  add_buf( roomBuffer, "\n\r" );
	foundRProg = TRUE;
      }
    }
    if ( foundMProg ) {
      send_to_charf(ch,"Mob programs in file '%s':\n\r", arg );
      if ( Mcol % 3 != 0 )
	add_buf( mobBuffer, "\n\r" );
      page_to_char(buf_string(mobBuffer),ch);
    }
    if ( foundOProg ) {
      send_to_charf(ch,"Obj programs in file '%s':\n\r", arg );
      if ( Ocol % 3 != 0 )
	add_buf( objBuffer, "\n\r" );
      page_to_char(buf_string(objBuffer),ch);
    }
    if ( foundRProg ) {
      send_to_charf(ch,"Mob programs in file '%s':\n\r", arg );
      if ( Rcol % 3 != 0 )
	add_buf( roomBuffer, "\n\r" );
      page_to_char(buf_string(roomBuffer),ch);
    }
  }
  else
    send_to_charf(ch,"No programs found in file %s.\n\r", fl->name );

//  for (int i = 0; i<HASH_PRG; i++)
//    for (CLASS_DATA *cl = prg_table[i]; cl; cl=cl->next) {
//      if ( get_root_class( cl ) != default_mob_class
//	   || cl == default_mob_class
//	   || str_cmp( cl->file, arg ) )
//	continue;
//      found = TRUE;
//      if ( cl->isAbstract )
//	sprintf( buf, "  [%-20s]", cl->name );
//      else
//	sprintf( buf, "  %-20s", cl->name );
//      strcat( buf1, buf );
//      if ( ++col % 4 == 0 )
//	strcat( buf1, "\n\r" );
//    }
//  if ( !found )
//    send_to_char(" No mob programs found.\n\r",ch);
//  else {
//    if ( col % 4 != 0 )
//      strcat( buf1, "\n\r" );
//    send_to_char( buf1, ch );
//  }
//
//  // Modified by SinaC 2002 to keep trace of class's incoming file
//  found = FALSE;
//  send_to_charf(ch,"Obj programs in file '%s':\n\r", arg );
//  buf1[0] = '\0';
//  col = 0;
//  for (int i = 0; i<HASH_PRG; i++)
//    for (CLASS_DATA *cl = prg_table[i]; cl; cl=cl->next) {
//      if ( get_root_class( cl ) != default_obj_class
//	   || cl == default_obj_class
//	   || str_cmp( cl->file, arg ) )
//	continue;
//      // Modified by SinaC 2002 to keep trace of class's incoming file
//      found = TRUE;
//      if ( cl->isAbstract )
//	sprintf( buf, "  [%-20s]", cl->name );
//      else
//	sprintf( buf, "  %-20s", cl->name );
//      strcat( buf1, buf );
//      if ( ++col % 4 == 0 )
//	strcat( buf1, "\n\r" );
//    }
//  if ( !found )
//    send_to_char(" No obj programs found.\n\r",ch);
//  else {
//    if ( col % 4 != 0 )
//      strcat( buf1, "\n\r" );
//    send_to_char( buf1, ch );
//  }
//
//  // Added by SinaC 2003 for room program
//  found = FALSE;
//  send_to_charf(ch,"Room programs in file '%s':\n\r", arg );
//  buf1[0] = '\0';
//  col = 0;
//  for (int i = 0; i<HASH_PRG; i++)
//    for (CLASS_DATA *cl = prg_table[i]; cl; cl=cl->next) {
//      if ( get_root_class( cl ) != default_room_class
//	   || cl == default_room_class
//	   || str_cmp( cl->file, arg ) )
//	continue;
//      found = TRUE;
//      if ( cl->isAbstract )
//	sprintf( buf, "  [%-20s]", cl->name );
//      else
//	sprintf( buf, "  %-20s", cl->name );
//      strcat( buf1, buf );
//      if ( ++col % 4 == 0 )
//	strcat( buf1, "\n\r" );
//    }
//  if ( !found )
//    send_to_char(" No room programs found.\n\r",ch);
//  else {
//    if ( col % 4 != 0 )
//      strcat( buf1, "\n\r" );
//    send_to_char( buf1, ch );
//  }
  return;
}

// Find mob/obj/room who have that program
void find_program( CHAR_DATA *ch, const char *argument ) {
  CLASS_DATA *cl;
  if (!(cl = silent_hash_get_prog(argument))) {
    send_to_char("No such program.\n\r",ch);
    return;
  }
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  buffer=new_buf();
  bool found = FALSE;

  // Mob
  int nMatch=0;
  for(int vnum=0;nMatch<top_mob_index;vnum++) {
    MOB_INDEX_DATA *pMobIndex;
    if ( (pMobIndex=get_mob_index(vnum)) !=NULL) {
      nMatch++;
      if ( pMobIndex->program == cl ) {
	found=TRUE;
	sprintf(buf, "[%5d] %-45s %s\n\r",
		pMobIndex->vnum, 
		pMobIndex->short_descr,
		pMobIndex->area->name);
	add_buf(buffer,buf);
      }
    }
  }

  // Obj
  nMatch=0;
  for(int vnum=0;nMatch<top_obj_index;vnum++) {
    OBJ_INDEX_DATA *pObjIndex;
    if ( (pObjIndex=get_obj_index(vnum)) !=NULL) {
      nMatch++;
      if ( pObjIndex->program == cl ) {
	found=TRUE;
	// Modified by SinaC 2001
	sprintf(buf, "[%5d] %-45s %s\n\r",
		pObjIndex->vnum, 
		pObjIndex->short_descr,
		pObjIndex->area->name);
	add_buf(buffer,buf);
      }
    }
  }

  // Room
  nMatch = 0;
  for( int vnum=0;nMatch<top_room;vnum++ ) {
    ROOM_INDEX_DATA *pRoomIndex;
    if ( (pRoomIndex=get_room_index(vnum)) != NULL) {
      nMatch++;
      if ( pRoomIndex->program == cl ) {
	found=TRUE;
	sprintf(buf, "[%5d] %s\n\r",
		pRoomIndex->vnum, pRoomIndex->name);
	add_buf(buffer,buf);
      }
    }
  }

  if ( !found )
    add_buf(buffer,"No mob/obj/room found.\n\r");

  page_to_char( buf_string(buffer), ch );
}

void program_syntax( CHAR_DATA *ch ) {
  send_to_char("Syntax: program info\n\r"
               "        program list\n\r"
               "        program list <script file>\n\r"
	       "        program dump <prg name>\n\r"
	       "        program dump <prg name> <method name>\n\r"
	       "        program find <prg name>\n\r",ch);
}

void do_programs( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
    
  argument = one_argument(argument, arg);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  if (!*arg) {
    program_syntax( ch );
    return;
  }

  // Show general information
  if (!str_prefix(arg,"info")) {
     send_to_charf(ch,"#fields  : %ld\n\r",topfield);
     send_to_charf(ch,"#scripts : %ld\n\r",topscripts);
     return;
  }

  // Find entity with clazz arg2
  if (!str_prefix(arg,"find")) {
    find_program( ch, arg2 );
    return;
  }

  // Dump script file list and clazz in a script file
  if (!str_prefix(arg,"list") ) {
    dump_script_file( ch, arg2 );
    return;
  }

  /* Dump class methods */
  if (!str_prefix(arg,"dump")) {
    if (!*arg2) {
      send_to_charf(ch,"You must specify a program name.\n\r");
      return;
    }
    CLASS_DATA * cl;
    if (!(cl = silent_hash_get_prog(arg2))) {
      send_to_char("No such class.\n\r",ch);
      return;
    }

    // method specifed
    if ( arg3 != NULL && arg3[0] != '\0' ) {
      FCT_DATA *f = hash_get_fct( arg3, cl->methods );
      if ( f == NULL ) {
	send_to_charf(ch,"No such method in program %s.\n\r", cl->name );
	return;
      }
      show_fct( ch, f );
      return;
    }
    else {
      // Added by SinaC 2003 for multiple inheritance
      char parentsName[MAX_STRING_LENGTH];
      parentsName[0] = '\0';
      for ( int i = 0; i < cl->parents_count; i++ ) {
	char buf[MAX_STRING_LENGTH];
	sprintf(buf," %s", cl->parents[i]->name );
	if ( i+1 < cl->parents_count )
	  strcat(buf,",");
	strcat(parentsName,buf);
      }
      
      send_to_charf(ch,"--- Methods defined in program %s (parent: %s) (file: %s)%s ---\n\r\n\r",
		    cl->name, 
		    cl->parents?/*cl->parent->name*/parentsName:"Root Prg", 
		    cl->file?cl->file:"Root Prg",
		    cl->isAbstract?" [Abstract]":"");
      for ( int i = 0; i < HASH_FCT; i++ ) {
	FCT_DATA *f= cl->methods[i];
	while ( f != NULL ) {
	  show_fct(ch,f);
	  f = f->next;
	}
      }
    }
    return;
  }
  program_syntax( ch );
}
