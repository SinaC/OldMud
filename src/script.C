/*****************************************************************************\
 *  This implements a script language for Mystery MUD.                       *
 *  Oxtal, 2001                                                              *
\*****************************************************************************/

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
#include "script2.h"
#include <stdarg.h>
#include "scrhash.h"
#include "cxx/struct_skel.hh"
#include "grammar.hh"
#include "db.h"
#include "olc_value.h"
#include "execute.hh"
#include "interp.h"
#include "config.h"
#include "error.hh"
#include "wiznet.h"
#include "const.h"
#include "olc.h"
#include "dbscript.h"


/**************** Extra fields ******************************************/

long topfield;

FIELD_DATA * find_field(FIELD_DATA* l, const char *n) {
  for (;l;l=l->next)
    if (!str_cmp(l->name,n))
      break;
  return l;
}

FIELD_DATA * add_field(FIELD_DATA** l, const char *n) {
  FIELD_DATA * res;
  if (!(res = find_field(*l,n))) {
    topfield++;
    res = new FIELD_DATA;
    res->lifetime = -1;
    res->name = str_dup(n);
    res->next = *l;
    *l = res;
  }
  return res;
}

/*

void del_field(FIELD_DATA** l, char *n)
{
  FIELD_DATA *f,*p = NULL;
  for (f=*l;f;p=f,f=f->next)
    if (!str_cmp(f->name,n)) {
      if (p)
        p->next = f->next;
      else
        *l=f->next;     
      free_mem((char*)f,sizeof(*f));
      topfield--;
      return;
    }
  bug("Warning : Try to remove inexistant field (%s)",n);
}
*/

bool del_extra_field(ENTITY_DATA *entity, const char *n ) {
  FIELD_DATA *p = entity->ex_fields;
  FIELD_DATA *prev = NULL;
  // loop on extra field and remove the right one
  while ( p != NULL ) {
    if ( !strcmp( p->name, n ) ) { // found it
      if ( prev == NULL ) // first in list
	entity->ex_fields = p->next;
      else
	prev->next = p->next;
      return TRUE;
    }
    prev = p;
    p = p->next;
  }
  return FALSE;
}

Value add_extra_field(const Value& v, const char* n) {
  return &(add_field(&(v.asEntity()->ex_fields), n)->var);
}


Value* get_extra_field(ENTITY_DATA* entity, const char* n) {
  FIELD_DATA* f = find_field(entity->ex_fields, n);
  if (f != NULL) 
    return &(f->var);
  else
    return NULL;
}

FIELD_DATA *get_field_data( ENTITY_DATA *entity, const char *n ) {
  FIELD_DATA* f = find_field(entity->ex_fields, n);
  if (f != NULL) 
    return f;
  else
    return NULL;
}

/*************************************************************************\
|**********    Here's the code of the interpeter  ************************|
\*************************************************************************/

#define TRACE log_stringf\
("res at %s (%d) is [%s] {%ld} typ = %d", __FUNCTION__, __LINE__, res.asStr(), res.val.i, res.typ);


void execute(Context& ctx, ENTITY_DATA* entity, const char* cmd) {

  if (entity->kind == CHAR_ENTITY) {
    CHAR_DATA* ch = (CHAR_DATA*) entity;
    
    char buf[MAX_STRING_LENGTH];
    strcpy(buf, cmd);
    if ( SCRIPT_VERBOSE > 6 ) {
      log_stringf("  before interpret [%s] : %x", cmd, (int)ch);
    }
    interpret(ch, buf);
  } 
  else if ( entity->kind == OBJ_ENTITY )
    p_warn("Program tried to interpret [%s] on the object [%s] (%d)", 
	   cmd, entity->name, ((OBJ_DATA*) entity)->pIndexData->vnum);
  else if ( entity->kind == ROOM_ENTITY )
    p_warn("Program tried to interpret [%s] on the room [%s] (%d)", 
	   cmd, entity->name, ((ROOM_INDEX_DATA*) entity)->vnum);
  else
    p_error("Program tried to interpret [%s] on unknown entity [%s] kind:%d",
	    cmd, entity->name, entity->kind );
}

Value runhandler(FCT_DATA * fct, const Value& subject, Value* params, int nparams) {
  Value res;
  ENTITY_DATA* entity = subject.asEntity();

  // SinaC 2003, cannot execute a function on a non-CON_PLAYING player or
  //  an item in a non-CON_PLAYING player's inventory
  // FIXME crappy: each fct/trigger should have flags giving
  //  these kind of informations
  if ( !str_cmp( fct->name, "onCreate" ) ) {
    if ( entity->kind == CHAR_ENTITY ) {
      CHAR_DATA *ch = (CHAR_DATA *)entity;
      if ( ch != NULL
	   && !IS_NPC(ch)
	   && ch->desc
	   && ( ch->desc->connected != CON_PLAYING
		|| IS_SET(ch->comm,COMM_BUILDING)
		|| IS_SET(ch->comm,COMM_AFK) ) ) {
	//      bug("player [%s] tries to use fct [%s] but is not playing",
	//      	  ch->name, fct->name );
	return Value((long)0);
      }
    }
    else if ( entity->kind == OBJ_ENTITY ) {
      OBJ_DATA *obj = (OBJ_DATA *)entity;
      CHAR_DATA *carrier = obj->carried_by;
      if ( carrier != NULL
	   && !IS_NPC(carrier)
	   && carrier->desc
	   && ( carrier->desc->connected != CON_PLAYING
		|| IS_SET(carrier->comm,COMM_BUILDING)
		|| IS_SET(carrier->comm,COMM_AFK)
		|| IS_SET(carrier->comm,COMM_EDITING) ) ) {
	//      bug("obj [%d] on player [%s] tries to use fct [%s] but player is not playing",
	//      	  obj->pIndexData->vnum, carrier->name, fct->name );
	return Value((long)0);
      }
    }
  }

  if (fct->predfct) {
    // Modified by SinaC 2003
    // A predefined function with nbparm == -1 is a function with a variable number of parameters
    if (fct->nbparm != -1 && fct->nbparm != nparams)
      p_error("Wrong number of parameters in predefined function call (%s)[%d/%d] ", 
	      fct->name, nparams, fct->nbparm);
    res = fct->predfct(entity, nparams, params);
    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
    }
  } 
  else {
    if (fct->nbparm != nparams)
      p_error("Wrong number of parameters in function call (%s)[%d/%d] ", 
	      fct->name, nparams, fct->nbparm);

    // If code is null, we check if it's method from an edited Class
    if ( IS_SET( fct->flags, FCT_NO_CODE ) ) {
      // if containing class exists and is not edited, error
      if ( fct->incoming && fct->incoming->lockBit == FALSE )
	p_error("NULL code for function %s.\n\r", fct->name );
      return Value((long)0);
    }

    Context ctx;
    ctx.pushscope();

    if (entity != NULL) {
      Value val = ctx.newVar(stringThis);
      val.setValue(subject);
    }

    Value val = ctx.newVar(stringResult);

    for (int i = 0; i< fct->nbparm; i++) {
      Value val = ctx.newVar(fct->parmname[i]);
      if ( SCRIPT_VERBOSE > 6 ) {
	log_stringf("Parameter '%s' is %x ('%s')",
		    fct->parmname[i], (int)params[i].val.s, params[i].asStr());
      }
      val.setValue(params[i]);
    }

    fct->code->execute(ctx);

    res = *ctx[stringResult];

    ctx.popscope();

    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
    }
  }

  return res;
}

int common_prog(ENTITY_DATA* subject, const char *name, Value * params) {
  if ( subject == NULL // SinaC 2003, invalid entity can't use script
       || !subject->valid )
    return 0;

  // Check if we are calling a trigger or not
  //  only one-non trigger called:  summonedElemental::attackCaster   from spell_summon_elemental
  if ( !isTrigger( name ) )
    log_stringf("SCRIPT: Calling a non-trigger method [%s] on Entity [%s] [kind: %d]",
		name, subject->name, subject->kind );

  FCT_DATA* f;

  f = getMethod( subject, name );

  // Trigger not found
  if ( f == NULL )
    return 0;

  // Check if clazz is valid
  if ( IS_SET( subject->clazz->flags, CLASS_MODIFIED ) ) {
    CLASS_DATA *pClass = subject->clazz;
    if ( pClass->lockBit )
      sprintf( log_buf, "SCRIPT: program %s can't be used because it's being edited by %s.", pClass->name, editor_name( pClass ) );
    else
      sprintf( log_buf, "SCRIPT: program %s is not finalized.", pClass->name );
    log_stringf( log_buf );;
    wiznet(log_buf, NULL, NULL, WIZ_PROGRAM, 0, 0 );
    return 0;
  }

  // SinaC 2003, check entity's position
  if ( subject->kind == CHAR_ENTITY ) {
    CHAR_DATA *mob = (CHAR_DATA *)subject;
    TriggerDescr *t = isTrigger( f );
    if ( t && mob->position < t->min_pos ) {
      log_stringf("CHAR: %s  position [%d] tries to use trigger [%s] position [%d]",
		  NAME(mob), mob->position, t->name, t->min_pos );
      return 0;
    }
  }


  try {
    Value res = runhandler(f, Value(subject), params, f->nbparm);
    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
    }
    return res.asInt();
  } catch (ScriptException e) {
    char subjectName[MAX_STRING_LENGTH];
    switch (subject->kind ) {
    case CHAR_ENTITY: {
      CHAR_DATA *ch = (CHAR_DATA *)subject;
      sprintf(subjectName,"CHAR: %s[%d]", NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:0);
      break;
    }
    case OBJ_ENTITY: {
      OBJ_DATA *obj = (OBJ_DATA *)subject;
      sprintf(subjectName,"OBJ: %s[%d]", obj->short_descr, obj->pIndexData->vnum );
      break;
    }
    case ROOM_ENTITY: {
      ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *)subject;
      sprintf(subjectName,"ROOM: %s[%d]", room->name, room->vnum );
      break;
    }
    default:
      sprintf(subjectName,"Wrong entity type (kind = %d)",subject->kind);
      break;
    }
    sprintf( log_buf, "error in mob/obj/room-program %s.%s: %s", subjectName, name, e.msg );
    bug(log_buf);
    wiznet(log_buf, NULL, NULL, WIZ_PROGRAM, 0, 0 );
    return 0;
  }

}

int mobprog( CHAR_DATA * mob, CHAR_DATA* act, const char *name, Value * params) {

  if (mob == NULL
      || !mob->valid
      || mob == act
      || mob->clazz == NULL
// Added by SinaC 2001
      || mob->clazz == default_mob_class
      // Added by SinaC 2003, sleeping mob are not updated, removed: TriggerDescr contains this info
      //|| ( IS_NPC(mob) && !IS_AWAKE(mob) )
      )
      //|| IS_AFFECTED(mob,AFF_CHARM)) Removed by SinaC 2003
    return 0;

  return common_prog(mob, name, params);
}


int objprog( OBJ_DATA* obj, CHAR_DATA* act, const char *name, Value * params) {

  if ( obj == NULL
       || !obj->valid
       || obj->clazz == NULL
       // Added by SinaC 2001
       || obj->clazz == default_obj_class )
    return 0;

  return common_prog(obj, name, params);
}

// Added by SinaC 2003 for room program
int roomprog( ROOM_INDEX_DATA* room, CHAR_DATA* act, const char *name, Value * params) {

  if ( room == NULL ||
       room->clazz == NULL ||
       // Added by SinaC 2001
       room->clazz == default_room_class )
    return 0;

  return common_prog(room, name, params);
}
