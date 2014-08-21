#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>

#include "merc.h"
#include "script2.h"
#include "scrhash.h"
#include "classes.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
//#include "olc.h"
#include "act_obj.h"
#include "fight.h"
#include "olc_value.h"
#include "names.h"
#include "lookup.h"
#include "tables.h"
#include "bit.h"
#include "act_comm.h"
#include "clan.h"
#include "affects.h"
#include "names.h"
#include "special.h"
#include "update.h"
#include "scrvalue.h"
#include "faction.h"
#include "ability.h"
#include "config.h"
#include "mem.h"
#include "magic.h"
#include "dbscript.h"
#include "hunt.h"
#include "utils.h"
#include "damage.h"


/* All the pre-defined functions of script language are defined here */

//******* global add affect method
void entity_addAffect( ENTITY_DATA *entity, Value *params ) {
  // params: where, location, operator, modifier, duration, level, type, castingLevel

  // 0: where
  const char *s = params[0].asStr();
  int where = flag_value( afto_type, s );
  if ( where == NO_FLAG ) {
    bug("entity_addAffect: unknown where [%s]", s );
    return;
  }
  // 2: op
  s = params[2].asStr();
  int op = flag_value( ops_flags, s );
  if ( op == NO_FLAG ) {
    bug("entity_addAffect: unknown operator [%s]", s );
    return;
  }
  // 4: duration, 5: level, 7: casting level
  int dur = params[4].asInt();
  int level = params[5].asInt();
  int casting = params[7].asInt();
  // 6: type
  int sn;
  s = params[6].asStr();
  if ( !str_cmp( s, "none" ) )
    sn = -1;
  else if ( ( sn = ability_lookup( s ) ) <= 0 ) {
    bug("entity_addAffect: unknown type [%s]", s );
    return;
  }

  // 1: location  depends on where   3: modifier  depends on location
  int loc = 0;
  int mod = 0;
  switch ( where ) {
  case AFTO_CHAR: {
    s = params[1].asStr();
    if ( ( loc = flag_value( attr_flags, s ) ) == NO_FLAG ) {
      bug("entity_addAffect: invalid attr flags [%s]", s );
      return;
    }
    if ( attr_table[loc].bits == NULL) // numeric location: str, wis, ...
      mod = params[3].asInt();
    else { // bit location
      s = params[3].asStr();
      if ( ( mod = flag_value( attr_table[loc].bits, s ) ) == NO_FLAG ) {
	bug("entity_addAffect: invalid modifier [%s] for attr [%s]",
	    s, attr_table[loc].name );
	return;
      }
    }
    break;
  }
  case AFTO_OBJECT: {
    loc = ATTR_NA; // affect extra_flags
    if ( op == AFOP_ADD ) {
      bug("entity_addAffect: can't use operator ADD for object flags");
      return;
    }
    if ( op == AFOP_ASSIGN ) {
      bug("entity_addAffect: can't use operator ASSIGN for object flags");
      return;
    }
    s = params[3].asStr();
    if ( ( mod = flag_value( extra_flags, s ) ) == NO_FLAG ) {
      bug("entity_addAffect: invalid extra flags [%s]", s );
      return;
    }
    break;
  }
  case AFTO_OBJVAL: {
    loc = params[1].asInt();
    if ( loc < 0 || loc > 4 ) {
      bug("entity_addAffect: invalid value id [%d]", loc );
    }
    mod = params[3].asInt(); // FIXME: should depend on value and item_type
    break;
  }
  case AFTO_WEAPON: {
    loc = 4; // weapon flags
    if ( op == AFOP_ADD ) {
      bug("entity_addAffect: can't use operator ADD for weapon flags");
      return;
    }
    if ( op == AFOP_ASSIGN ) {
      bug("entity_addAffect: can't use operator ASSIGN for weapon flags");
      return;
    }
    s = params[3].asStr();
    if ( ( mod = flag_value( weapon_type2, s ) ) == NO_FLAG ) {
      bug("entity_addAffect: invalid weapon flags [%s]", s );
      return;
    }
    break;
  }
  }

  AFFECT_DATA af;
  createaff(af,dur,level,sn,casting,sn!=-1?AFFECT_ABILITY:AFFECT_INHERENT);
  addaff2(af,where,loc,op,mod);
  //af.where       = where;
  //af.location    = loc;
  //af.op          = op;
  //af.modifier    = mod;
  //af.duration    = dur;
  //af.level       = level;
  //af.type        = sn;
  //af.casting_level = casting;


  switch ( entity->kind ) {
  case CHAR_ENTITY: {
    CHAR_DATA *ch = (CHAR_DATA *)entity;
    //affect_to_char( ch, &af );
    affect_join( ch, &af );
    break;
  }
  case OBJ_ENTITY: {
    OBJ_DATA *obj = (OBJ_DATA *)entity;
    //affect_to_obj( obj, &af );
    affect_join_obj( obj, &af );
    break;
  }
  case ROOM_ENTITY: {
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *)entity;
    //affect_to_room( room, &af );
    affect_join_room( room, &af );
    break;
  }
  }
}

bool entity_savesSpell( ENTITY_DATA *entity, Value *params ) {
  // params: level, dam_type
  int level = params[0].asInt();
  const char *s = params[1].asStr();
  int damType = flag_value( dam_class_flags, s );
  if ( damType == NO_FLAG ) {
    bug("entity_savesSpell: invalid dam class [%s]", s );
    return FALSE;
  }
  switch ( entity->kind ) {
  case CHAR_ENTITY:
    return saves_spell( level, (CHAR_DATA *)entity, damType ); break;
  case OBJ_ENTITY: case ROOM_ENTITY:
    return FALSE; break;
  }
  return FALSE;
}

void entity_affectStrip( ENTITY_DATA *entity, Value *params ) {
  // params: ability name
  const char *s = params[0].asStr();
  int sn = ability_lookup( s );
  if ( sn <= 0 ) {
    bug("entity_affectStrip: invalid ability [%s]", s );
    return;
  }
  switch( entity->kind ) {
  case CHAR_ENTITY: affect_strip( (CHAR_DATA *)entity, sn ); break;
  case OBJ_ENTITY: affect_strip_obj( (OBJ_DATA *)entity, sn ); break;
  case ROOM_ENTITY: affect_strip_room( (ROOM_INDEX_DATA *)entity, sn ); break;
  }
}

//************************* new act function ***************************
void act_ultimate(CHAR_DATA* target, const char *format, ENTITY_DATA** params, const int size ) {
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };

  //non-switch/non charmies NPC doesn't receive any message
  if ( target->desc == NULL && target->master == NULL )
    return;

  // null format doesn't need to be sent
  if ( format == NULL || format[0] == '\0' )
    return;

  char msg[MAX_STRING_LENGTH];
  msg[0] = 0;

  const char* s = format;
  char* t = msg;
  int i = 0;
  while (*s) {
    if (*s == '$') {
      const char* p = "NULL";
      s++;

      if ( i >= size ) {
	bug("act_ultimate: not enough related entity, str:[%s] #parm:[%d].",
	    format, size );
	return;
      }
      
      switch (*s) {
      case '$':
	p = "$";
	break;
      case '\'': { // your/name's
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "your";
	else {
	  static char tmp[MAX_STRING_LENGTH];
	  switch (e->kind) {
	  case CHAR_ENTITY: {
	    strcpy(tmp,PERS((CHAR_DATA*)e, target));
	    break;
	  }
	  case ROOM_ENTITY: {
	    ROOM_INDEX_DATA* room1 = (ROOM_INDEX_DATA*) e;
	    strcpy(tmp,can_see_room( target, room1 )
		   ? room1->description
		   : "somewhere" );
	    break;
	  }
	  case OBJ_ENTITY: {
	    OBJ_DATA* obj1 = (OBJ_DATA*) e;
	    strcpy(tmp,
		   p = can_see_obj( target, obj1 )
		   ? obj1->short_descr
		   : "something" );
	    break;
	  }
	  }
	  if ( tmp[strlen(tmp)-1] == 's' )
	    strcat(tmp,"'");
	  else
	    strcat(tmp,"'s");
	  p = tmp;
	}
	break;
      }
      case 'b': { // BE
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "are";
	else
	  p = "is";
	break;
      }
      case 'h': { // HAVE
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "have";
	else
	  p = "has";
	break;
      }
      case 'n': { // short_descr
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "you";
	else
	  switch (e->kind) {
	  case CHAR_ENTITY:
	    p = PERS((CHAR_DATA*)e, target);
	    break;
	  case ROOM_ENTITY: {
	    ROOM_INDEX_DATA* room1 = (ROOM_INDEX_DATA*) e;
	    p = can_see_room( target, room1 )
	      ? room1->description
	      : "somewhere";
	    break;
	  }
	  case OBJ_ENTITY: {
	    OBJ_DATA* obj1 = (OBJ_DATA*) e;
	    p = can_see_obj( target, obj1 )
	      ? obj1->short_descr
	      : "something";
	    break;
	  }
	  }
	break;
      }
      case 'v':{ // 
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "";
	else
	  if ( *(s-2) == 'y' ) {
	    p = "ies";
	    t--; // remove the verb's "y" in msg
	  }
	  else if ( *(s-2) == 'o' || *(s-2) == 'h' )
	    p = "es";
	  else
	    p = "s";
	break;
      }
      case 's': { // his her its
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "your";
	else
	  switch (e->kind) {
	  case CHAR_ENTITY:
	    p = his_her[URANGE(0, ((CHAR_DATA*)e)->cstat(sex) , 2)];
	    break;
	  default:
	    p = "its";
	  }
	break;
      }
      case 'm': { // him her it
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "you";
	else
	  switch (e->kind) {
	  case CHAR_ENTITY:
	    p = him_her[URANGE(0, ((CHAR_DATA*)e)->cstat(sex) , 2)];
	    break;
	  default:
	    p = "it";
	  }
	break;
      }
      case 'e': { // he she it
	ENTITY_DATA* e = params[i++];
	if (e == target)
	  p = "you";
	else
	  switch (e->kind) {
	  case CHAR_ENTITY:
	    p = he_she[URANGE(0, ((CHAR_DATA*)e)->cstat(sex) , 2)];
	    break;
	  default:
	    p = "it";
	  }
	break;
      }
      default:
	p = " <@@@> ";
	bug("act_ultimate: bad code: %c.", *s );
	break;
      }
      s++;
      while (*p)
	*t++ = *p++;
    } 
    else 
      *t++ = *s++;
    
  }
  *t = '\0';
  if ( msg[0] == '{' && strlen( msg ) > 2 )
    msg[2] = UPPER(msg[2]);
  else
    msg[0] = UPPER(msg[0]);
  send_to_charf(target,"%s\n\r",msg);
}

void act_ultimate_one_related(ENTITY_DATA *related, CHAR_DATA* target, const char *format ) {
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };

  //non-switch/non charmies NPC doesn't receive any message
  if ( target->desc == NULL && target->master == NULL )
    return;

  // null format doesn't need to be sent
  if ( format == NULL || format[0] == '\0' )
    return;

  char msg[MAX_STRING_LENGTH];
  msg[0] = 0;

  const char* s = format;
  char* t = msg;
  while (*s) {
    if (*s == '$') {
      const char* p = "NULL";
      s++;
      switch (*s) {
      case '$':
	p = "$";
	break;
      case '\'': { // your/name's
	if (related == target)
	  p = "your";
	else {
	  static char tmp[MAX_STRING_LENGTH];
	  switch (related->kind) {
	  case CHAR_ENTITY: {
	    strcpy(tmp,PERS((CHAR_DATA*)related, target));
	    break;
	  }
	  case ROOM_ENTITY: {
	    ROOM_INDEX_DATA* room1 = (ROOM_INDEX_DATA*) related;
	    strcpy(tmp,can_see_room( target, room1 )
		   ? room1->description
		   : "somewhere" );
	    break;
	  }
	  case OBJ_ENTITY: {
	    OBJ_DATA* obj1 = (OBJ_DATA*) related;
	    strcpy(tmp,
		   p = can_see_obj( target, obj1 )
		   ? obj1->short_descr
		   : "something" );
	    break;
	  }
	  }
	  if ( tmp[strlen(tmp)-1] == 's' )
	    strcat(tmp,"'");
	  else
	    strcat(tmp,"'s");
	  p = tmp;
	}
	break;
      }
      case 'b': { // BE
	if (related == target)
	  p = "are";
	else
	  p = "is";
	break;
      }
      case 'h': { // HAVE
	if (related == target)
	  p = "have";
	else
	  p = "has";
	break;
      }
      case 'n': { // short_descr
	if (related == target)
	  p = "you";
	else
	  switch (related->kind) {
	  case CHAR_ENTITY:
	    p = PERS((CHAR_DATA*)related, target);
	    break;
	  case ROOM_ENTITY: {
	    ROOM_INDEX_DATA* room1 = (ROOM_INDEX_DATA*) related;
	    p = can_see_room( target, room1 )
	      ? room1->description
	      : "somewhere";
	    break;
	  }
	  case OBJ_ENTITY: {
	    OBJ_DATA* obj1 = (OBJ_DATA*) related;
	    p = can_see_obj( target, obj1 )
	      ? obj1->short_descr
	      : "something";
	    break;
	  }
	  }
	break;
      }
      case 'v':{ // 
	if (related == target)
	  p = "";
	else
	  if ( *(s-2) == 'y' ) {
	    p = "ies";
	    t--; // remove the verb's "y" in msg
	  }
	  else if ( *(s-2) == 'o' || *(s-2) == 'h' )
	    p = "es";
	  else
	    p = "s";
	break;
      }
      case 's': { // his her its
	if (related == target)
	  p = "your";
	else
	  switch (related->kind) {
	  case CHAR_ENTITY:
	    p = his_her[URANGE(0, ((CHAR_DATA*)related)->cstat(sex) , 2)];
	    break;
	  default:
	    p = "its";
	  }
	break;
      }
      case 'm': { // him her it
	if (related == target)
	  p = "you";
	else
	  switch (related->kind) {
	  case CHAR_ENTITY:
	    p = him_her[URANGE(0, ((CHAR_DATA*)related)->cstat(sex) , 2)];
	    break;
	  default:
	    p = "it";
	  }
	break;
      }
      case 'e': { // he she it
	if (related == target)
	  p = "you";
	else
	  switch (related->kind) {
	  case CHAR_ENTITY:
	    p = he_she[URANGE(0, ((CHAR_DATA*)related)->cstat(sex) , 2)];
	    break;
	  default:
	    p = "it";
	  }
	break;
      }
      default:
	p = " <@@@> ";
	bug("act_ultimate: bad code: %c.", *s );
	break;
      }
      s++;
      while (*p)
	*t++ = *p++;
    } 
    else 
      *t++ = *s++;
    
  }
  msg[0] = UPPER(msg[0]);
  *t = '\0';
  send_to_charf(target,"%s\n\r",msg);
}

const char *generalAsStr( const Value val ) {
  static char s[512];
  // if we received a list as params, we try to convert it in a string
  if ( val.typ == SVT_LIST ) {
    ValueList *v = val.asList();
    s[0] = '\0';
    for ( int i = 0; i < v->size; i++ ) {
      strcat( s, v->elems[i].asStr() );
      strcat( s, " " );
    }
  }
  else // otherwise, we just convert it into a string
    strcpy(s,val.asStr());
  return s;
}

/************************* Global predef funvctions ********************/

Value gpf_chance(ENTITY_DATA *e, const int nparams, Value* params) {
  //return number_range(0,99) < params[0].asInt();
  return number_percent() < params[0].asInt();
}

Value gpf_log(ENTITY_DATA *e, const int nparams, Value* params) {
  log_stringf("SCRIPT LOG: %s", params[0].asStr()); fflush(stderr);

  return Value((long)0);
}

Value gpf_random(ENTITY_DATA *e, const int nparams, Value* params) {
  return number_range( 0, params[0].asInt()-1);
}

Value gpf_max(ENTITY_DATA *e, const int nparams, Value* params ) {
  int 
    a = params[0].asInt(),
    b = params[1].asInt();
  return Value(UMAX(a,b));
}

Value gpf_min(ENTITY_DATA *e, const int nparams, Value* params ) {
  int 
    a = params[0].asInt(),
    b = params[1].asInt();
  return Value(UMIN(a,b));
}

Value gpf_range(ENTITY_DATA *e, const int nparams, Value* params ) {
  int 
    a = params[0].asInt(),
    b = params[1].asInt(),
    c = params[2].asInt();
    
  return Value(URANGE(a,b,c));
}

Value gpf_timeHour(ENTITY_DATA *e, const int nparams, Value* params ) {
  return Value(time_info.hour);
}

Value gpf_timeDay(ENTITY_DATA *e, const int nparams, Value* params ) {
  return Value(time_info.day);
}

Value gpf_timeMonth(ENTITY_DATA *e, const int nparams, Value* params ) {
  return Value(time_info.month);
}

Value gpf_timeYear(ENTITY_DATA *e, const int nparams, Value* params ) {
  return Value(time_info.year);
}

Value gpf_dice(ENTITY_DATA *e, const int nparams, Value* params ) {
  int 
    a = params[0].asInt(),
    b = params[1].asInt();
  return Value(dice(a,b));
}

Value gpf_getRoom( ENTITY_DATA *e, const int nparams, Value *params ) {
  int vnum = params[0].asInt();
  return Value(get_room_index(vnum));
}

Value gpf_getCharList( ENTITY_DATA *e, const int nparams, Value *params ) {
  // mobile_count could be use instead but we prefer to count to be sure
  int n = 0;
  for ( CHAR_DATA *wch = char_list; wch != NULL ; wch = wch->next )
    if ( wch->in_room != NULL )
      n++;

  ValueList *charList = ValueList::newList(n);

  int i = 0;
  for ( CHAR_DATA *wch = char_list; wch != NULL ; wch = wch->next )
    if ( wch->in_room != NULL )
      charList->elems[i++] = Value(wch);

  return Value(charList);
}

Value gpf_getObjList( ENTITY_DATA *e, const int nparams, Value *params ) {

  int n = 0;
  for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj->next )
    n++;

  ValueList *objList = ValueList::newList(n);

  int i = 0;
  for ( OBJ_DATA *obj = object_list; obj != NULL; obj = obj->next )
      objList->elems[i++] = Value(obj);

  return Value(objList);
}

Value gpf_getRoomList( ENTITY_DATA *e, const int nparams, Value *params ) {
  int n = 0;
  ROOM_INDEX_DATA *room;
  for ( int i = 0; i < 65535; i++ ) {
    if ( ( room = get_room_index( i ) ) == NULL )
      continue;
    n++;
  }

  ValueList *roomList = ValueList::newList(n);

  int j = 0;
  for ( int i = 0; i < 65535; i++ ) {
    if ( ( room = get_room_index( i ) ) == NULL )
      continue;
    roomList->elems[j++] = Value(room);
  }

  return Value(roomList);
}

Value gpf_dump(ENTITY_DATA *e, const int nparams, Value* params) {
  char msg[MAX_STRING_LENGTH];
  strcpy( msg, params[0].asStr());
  if ( msg[strlen(msg)-1] != '\n'
       && msg[strlen(msg)-1] != '\r' )
    strcat( msg, "\n");

  FILE *fp;
  fclose( fpReserve );
  if (!(fp = fopen ( SCRIPT_DUMP_FILE, "a"))){
    bug("Can't open script dump file: %s.", SCRIPT_DUMP_FILE );
    fpReserve = fopen( NULL_FILE, "r" );
    return Value((long)0);
  }

  fprintf(fp,msg);

  fclose (fp);
  fpReserve = fopen( NULL_FILE, "r" );

  return Value((long)0);
}

Value gpf_act(ENTITY_DATA *e, const int nparams, Value *params ) {
  // First: get room       e is this
  ROOM_INDEX_DATA *room;
  switch( e->kind ) {
  case CHAR_ENTITY: room = ((CHAR_DATA *)e)->in_room; break;
  case OBJ_ENTITY: room = get_room_obj((OBJ_DATA*)e); break;
  case ROOM_ENTITY: room = (ROOM_INDEX_DATA*)e; break;
  default: bug("Invalid entity kind: %d", e->kind );
    return Value((long)0);
  }

  // Second: convert 2nd param in an entity list or an entity
  Value arg2 = params[1].get();
  ENTITY_DATA **entityList;
  int entityListSize;
  if ( arg2.typ == SVT_ENTITY ) {
    entityList = new ENTITY_DATA* [1];
    entityList[0] = arg2.asEntity();
    entityListSize = 1;
  }
  else {
    ValueList *l = arg2.asList();
    entityList = new ENTITY_DATA* [l->size];
    for ( int i = 0; i < l->size; i++ )
      entityList[i] = l->elems[i].asEntity();
    entityListSize = l->size;
  }

  // Third: analyse 3rd param to determine how to send the message
  ValueList *targets;
  Value arg = params[2].get();
  switch( arg.typ ) {
  case SVT_STR: {    // string: TO_OTHER, TO_ALL, TO_RELATED
    const char *s = arg.asStr();
    if ( !str_cmp(s,"TO_ALL" ) ) { // send to everyone in the room
      int count = 0;
      for ( CHAR_DATA *ch = room->people; ch; ch = ch->next_in_room )
	count++;
      targets = ValueList::newList(count);
      int i = 0;
      for ( CHAR_DATA *ch = room->people; ch; ch = ch->next_in_room )
	targets->elems[i++] = ch;
    }
    else if ( !str_cmp(s,"TO_OTHER") ) { // send to everyone which is not in entityList
      int count = 0; // count will be a max value
      for ( CHAR_DATA *ch = room->people; ch; ch = ch->next_in_room )
	count++;
      targets = ValueList::newList(count); // maximum vector size
      count = 0; // fill the vector
      for ( CHAR_DATA *ch = room->people; ch; ch = ch->next_in_room ) {
	bool found = FALSE;
	for ( int i = 0; i < entityListSize; i++ ) {
	  ENTITY_DATA *ent = entityList[i];
	  if ( ent->kind == CHAR_ENTITY && ((CHAR_DATA*)ent) == ch ) {
	    found = TRUE;
	    break;
	  }
	}
	if ( !found )
	  targets->elems[count++] = ch;
      }
      targets->size = count; // real vector size
    }
    else if ( !str_cmp(s,"TO_RELATED") ) { // send to everyone in the entityList
      targets = params[1].asList()->uniqueEquiv().val.l;
    }
    else {
      bug("gpf_act: Invalid type [%s] can't determine how to send the message.",
	  s );
      return Value((long)0);
    }
    break;
  }
  case SVT_LIST: {   // list: list of entity to send to
    targets = arg.asList();
    break;
  }
  case SVT_ENTITY: { // entity: send to this entity
    targets = ValueList::newList(1);
    targets->elems[0] = arg.asEntity();
    break;
  }
  default:
    bug("gpf_act: Invalid type (%d) can't determine how to send the message.",
	arg.typ );
    return Value((long)0);
  }

  if ( targets->size == 0 )
    return Value((long)0);

  // Could be optimized: don't consider NPC when constructing targets
  for ( int i = 0; i < targets->size; i++ ) {
    ENTITY_DATA *ent = targets->elems[i].asEntity();
    if ( ent->kind != CHAR_ENTITY )
      bug("gpf_act: Invalid target (typ: d) [%s]", targets->elems[i].asStr() );
    else
      act_ultimate( (CHAR_DATA *)ent, params[0].asStr(), entityList, entityListSize );
  }

  return Value((long)0);
}

Value gpf_saveAreaState(ENTITY_DATA *ent, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room;
  switch( ent->kind ) {
  case CHAR_ENTITY: room = ((CHAR_DATA *)ent)->in_room; break;
  case OBJ_ENTITY: room = get_room_obj((OBJ_DATA*)ent); break;
  case ROOM_ENTITY: room = (ROOM_INDEX_DATA*)ent; break;
  default: bug("Invalid entity kind: %d", ent->kind );
    return Value((long)0);
  }
  if ( room == NULL ) {
    bug("gpf_saveAreaState: Entity %s is in a NULL room.", ent->name );
    return Value((long)0);
  }
  if ( room->area == NULL ) {
    bug("gpf_saveAreaState: Room %d is not in an area.", room->vnum );
    return Value((long)0);
  }
  // Next area update: this area state will be saved
  SET_BIT( room->area->area_flags, AREA_SAVE_STATE );
  return Value((long)0);
}
/************************* Mob Predifined functions ********************/

#define NEW_MOB_FIELD_METHOD(field_name)			\
Value mob_ ## field_name(ENTITY_DATA* entity, const int nparams, Value* params) {	\
  CHAR_DATA* ch = (CHAR_DATA*) entity;                          \
  return Value(ch->field_name);			                \
}

NEW_MOB_FIELD_METHOD(name);
NEW_MOB_FIELD_METHOD(level);
NEW_MOB_FIELD_METHOD(hit);
NEW_MOB_FIELD_METHOD(silver);
NEW_MOB_FIELD_METHOD(gold);
NEW_MOB_FIELD_METHOD(fighting);


Value mob_isNPC(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  return IS_NPC(ch);
}

Value mob_carryNumber(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  return ch->carry_number;
}

Value mob_carryWeight(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  return get_carry_weight( ch );
}

Value mob_classes(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  long classes = ch->cstat(classes);
  int ncl = class_count(classes);
  int icl = 0;
  ValueList* cls = ValueList::newList(ncl);
  for (int i = 0; i<MAX_CLASS ; i++) {
    if (classes & (1 << i))
      cls->elems[icl++] = Value(class_table[i].name);
  }
  return cls;
}

Value mob_sex(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  return ch->cstat(sex);
}

Value mob_addSilver( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  int silver = params[0].asInt();

  ch->silver += silver;
  if ( ch->silver >= 0 )
    return Value(1);

  ch->silver = 0;
  return Value((long)0);
}

Value mob_addGold( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  int gold = params[0].asInt();

  ch->gold += gold;
  if ( gold >= 0 ) {
    return Value(1);
  }

  ch->gold = 0;
  return Value((long)0);
}

Value mob_addHit( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  ch->hit = UMIN( ch->hit+params[0].asInt(), ch->cstat(max_hit) );
  
  return Value((long)0);
}

Value mob_addMana( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  ch->mana = UMIN( ch->mana+params[0].asInt(), ch->cstat(max_mana) );
  
  return Value((long)0);
}
// Added by SinaC 2001 for mental user
Value mob_addPsp( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  ch->psp = UMIN( ch->psp+params[0].asInt(), ch->cstat(max_psp) );
  
  return Value((long)0);
}

Value mob_getObjCarried( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  char s[512];
  strcpy( s, params[0].asStr() );
  OBJ_DATA *obj = get_obj_carry(ch,s,ch);
  return Value(obj);
}

Value mob_inventory( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  int count = 0;

  // count objects
  for ( OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content ) {
    if ( obj->wear_loc == WEAR_NONE )
      count++;
  }
  ValueList *inventory = ValueList::newList(count);
  int i = 0;
  // put objects in the list
  for ( OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content ) {
    if ( obj->wear_loc == WEAR_NONE )
      inventory->elems[i++] = Value(obj);
  }
  return inventory;
}

Value mob_vnum( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  if ( IS_NPC(ch) )
    return Value(ch->pIndexData->vnum);
  else
    return Value((long)0);
}

Value mob_canCarryN( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  return Value(can_carry_n(ch));
}

Value mob_canCarryW( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  return Value(can_carry_w(ch));
}

Value mob_oLoad( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  OBJ_INDEX_DATA *pObj;
  OBJ_DATA *obj;
  int vnum = params[0].asInt();

  if ( ( pObj = get_obj_index(vnum) ) == NULL ) {
    bug("mob_oLoad: invalid vnum (%d) char (%s)",
	vnum, NAME(ch));
    return Value((long)0);
  }
  obj = create_object(pObj, 0);
  if ( CAN_WEAR( obj, ITEM_TAKE ) )
    obj_to_char( obj, ch );
  else {
    ROOM_INDEX_DATA *pRoom;
    if ( ch->in_room ) pRoom = ch->in_room;
    else pRoom = get_room_index(ROOM_VNUM_LIMBO);
    obj_to_room( obj, pRoom );
    // Added by SinaC 2001
    recomproom( pRoom );
  }

  return Value(obj);
}

Value mob_mLoad( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *victim;
  int vnum = params[0].asInt();

  if ( ( pMob = get_mob_index(vnum) ) == NULL ) {
    bug("mob_mLoad: invalid vnum (%d) char (%s)",
	vnum, NAME(ch));
    return Value((long)0);
  }
  victim = create_mobile(pMob);
  ROOM_INDEX_DATA *pRoom;
  if ( ch->in_room ) pRoom = ch->in_room;
  else {
    pRoom = get_room_index(ROOM_VNUM_LIMBO);
    bug("mob_load: mob loaded in limbo");
  }
  char_to_room( victim, pRoom );
  // Added by SinaC 2001
  //recomproom( pRoom ); NO NEED: done in char_to_room

  return Value(victim);
}

// Can be used on char or obj or room
Value mob_canSee( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  ENTITY_DATA *e = params[0].asEntity();
  switch( e->kind ) {
  case CHAR_ENTITY: {
    CHAR_DATA *victim = (CHAR_DATA *) e;
    
    return can_see( ch, victim );
    break; }
  case OBJ_ENTITY: {
    OBJ_DATA *obj = (OBJ_DATA *) e;
    
    return can_see_obj( ch, obj );
    break; }
    // Added by SinaC 2003 for room program
  case ROOM_ENTITY: {
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) e;
    
    return can_see_room( ch, room );
    break; }
    
  }
  return Value((long)0);
}

Value mob_getAbility( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char buf[512];
  strcpy( buf, params[0].asStr());
  int sn = ability_lookup(buf);
  if ( sn <= 0 ) {
    bug("mob_getAbility: invalid ability: %s", buf );
    return Value(-1);
  }

  return get_ability( ch, sn );
}

Value mob_checkImprove( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char buf[512];
  strcpy( buf, params[0].asStr());
  int sn = ability_lookup(buf);
  if ( sn <= 0 ) {
    bug("mob_checkImprove: invalid ability: %s", buf );
    return Value(-1);
  }
  check_improve( ch, sn, params[1].asInt(), params[2].asInt() );
  return Value((long)0);
}

Value mob_isAwake( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  return Value(IS_AWAKE(ch));
}

Value mob_sendTo( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  char msg[MAX_INPUT_LENGTH];
  strcpy( msg, params[0].asStr() );
  if ( msg[strlen(msg)-1] != '\n'
       && msg[strlen(msg)-1] != '\r' )
    strcat( msg, "\n\r");
  send_to_charf( ch, msg );

  return Value((long) 0);
}

Value mob_isEquiv( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *chara = (CHAR_DATA*) entity;
  CHAR_DATA *charb = (CHAR_DATA*) params[0].asEntity();

  return valueEquiv( chara, charb );
}

Value mob_canLoot( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  OBJ_DATA* obj = (OBJ_DATA*) params[0].asEntity();

  return Value(can_loot(ch,obj));
}

Value mob_alignment( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  return Value(ch->cstat(alignment));
}

Value mob_etho( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  return Value(ch->cstat(etho));
}

Value mob_group( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  if ( IS_NPC(ch) )
    return Value(ch->pIndexData->group);
  else
    return Value(-1);
}

Value mob_isSafe( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  CHAR_DATA* victim = (CHAR_DATA*) params[0].asEntity();

  // no call to is_safe because that function has output
  //return is_safe( ch, victim );

  return silent_is_safe( ch, victim );
  return Value(long(FALSE));// FALSE
}

Value mob_isWearing( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  char s[512];
  strcpy( s, params[0].asStr() );
  return get_obj_wear(ch,s,FALSE) != NULL;
}

Value mob_multiHit( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  CHAR_DATA* victim = (CHAR_DATA*) params[0].asEntity();

  if ( !is_safe( ch, victim ) )
    multi_hit( ch, victim, TYPE_UNDEFINED);
  return Value((long)0);
}

Value mob_shortDescr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  if ( IS_NPC(ch) )
    return Value(ch->short_descr);
  else
    return Value(ch->name);
}

Value mob_damage( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  // 3 arguments: damAmount, damType, dieMsg
  int dam = params[0].asInt();
  int dam_type = flag_value_complete(dam_class_flags, params[1].asStr() );

  if ( dam_type == NO_FLAG ) {
    bug("mob_damage: Invalid dam_type [%s] assuming DAM_NONE.", params[1].asStr() );
    dam_type = DAM_NONE;
  }

  int done = noaggr_damage( ch, dam, dam_type,
			     "",
			     "",
			     params[2].asStr(),
			     FALSE );
	 
  return Value(done);
}

Value mob_transfer( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  int vnum = params[0].asInt();

  ROOM_INDEX_DATA *location;
  if ( ( location = get_room_index( vnum ) ) == NULL ) {
    bug("mob_transfer: invalid location (%d) for char (%s)",
	vnum, NAME(ch));
    return Value((long)0);
  }

  char_from_room( ch );
  char_to_room( ch, location );

  return Value((long)0);
}

Value mob_echo( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char msg[MAX_STRING_LENGTH];
  
  strcpy( msg, params[0].asStr() );
  if ( msg[strlen(msg)-1] != '\n'
       && msg[strlen(msg)-1] != '\r' )
    strcat( msg, "\n\r");

  if ( ch->in_room == NULL ) {
    bug("mob_echo: char (%s) not in a room",
	NAME(ch));
    return Value((long)0);
  }
  CHAR_DATA *fch;
  for ( fch = ch->in_room->people; fch != NULL; fch = fch->next_in_room ) {
    if ( can_see( fch, ch ) )
      send_to_charf( fch,"%s", msg );
  }
  return Value((long)0);
}

Value mob_isAffected( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char name[MAX_STRING_LENGTH];
  strcpy( name, params[0].asStr() );
  int sn = ability_lookup(name);

  if ( sn <= 0 ) {
    bug("mob_is_affected: invalid name %s", name );
    return Value((long)0);
  }

  if ( is_affected( ch, sn ) )
    return Value((long)1);
  else
    return Value((long)0);
}

Value mob_addPractice( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  int pra = params[0].asInt();

  ch->practice += pra;
  if ( ch->practice >= 0 )
    return Value(1);

  ch->practice = 0;
  return Value((long)0);
}

Value mob_destroy( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA*) entity;

  if ( IS_NPC(ch) )
    extract_char( ch, TRUE );
  else
    bug("mob_destroy: Attempting to extract a player (%s)", ch->name);

  return Value(ch);
}

Value mob_getPet( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  return Value(ch->pet);
}

Value mob_setPet( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *mob = (CHAR_DATA*) entity;
  CHAR_DATA *ch = (CHAR_DATA*) params[0].asEntity();

  // mob (this) is set as pet for ch (param)
  if ( !IS_NPC(mob) ) 
    bug("mob_setPet: Attempting to set a player (%s) as pet for (%s)",
	mob->name, ch->name );
  else {
    /* Code stolen from do_buy */
    SET_BIT(mob->act, ACT_PET);
    SET_BIT(mob->bstat(affected_by), AFF_CHARM);
    recompute(mob);
    
    mob->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;
    
    add_follower( mob, ch );
    mob->leader = ch;
    ch->pet = mob;
  }
  return Value(ch);
}

Value mob_setName( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA*) entity;
  char name[MAX_STRING_LENGTH];
  strcpy( name, params[0].asStr() );

  if ( IS_NPC(ch) ) {
    char buf[1024];
    sprintf( buf, "%s %s", ch->name, name );
    ch->name = str_dup( buf );
  }
  else
    bug("mob_setName: Attempting to rename player (%s) as (%s)", ch->name, name );
  return Value(ch);
}

// Added by SinaC 2003
Value mob_clan( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  
  //if (!IS_NPC(ch) && is_clan(ch) )
  return Value( get_clan_table(ch->clan)->name );
  //return Value((long)0);
}

Value mob_carryLocation( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  char name[MAX_STRING_LENGTH];
  strcpy( name, params[0].asStr() );

  int which = flag_value_complete( wear_loc_flags, name );
  if ( which == NO_FLAG ) {
    bug("mob_carryLocation: invalid wear loc (%s) for char (%s)", 
	name, NAME(ch) );
    return Value((long)0);
  }

  return Value( get_eq_char( ch, which ) );
}

Value mob_room( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  return Value(ch->in_room);
}

Value mob_longDescr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  if ( IS_NPC(ch) )
    return Value( ch->long_descr );
  else
    if ( ch->pcdata->stance != NULL 
	 && ch->pcdata->stance[0]!='\0' )
      return Value( ch->pcdata->stance );
    else
      return Value( ch->pcdata->title );
}

Value mob_position( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  return Value( flag_string( position_flags, ch->position ) );
}

Value mob_defaultPosition( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;

  return Value( flag_string( position_flags, ch->default_pos ) );
}

Value mob_equipment( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  int count = 0;

  // count objects
  for ( OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content ) {
    if ( obj->wear_loc != WEAR_NONE )
      count++;
  }

  ValueList *equipment = ValueList::newList(count);

  int i = 0;
  // put objects in the list
  for ( OBJ_DATA *obj = ch->carrying; obj; obj = obj->next_content ) {
    if ( obj->wear_loc != WEAR_NONE )
      equipment->elems[i++] = Value(obj);
  }
  
  return equipment;
}

Value mob_suddenDeath( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  char msg[4096];
  strcpy(msg,params[0].asStr());
  silent_kill( ch, msg );
  return Value((long)0);
}

Value mob_stopFighting( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  if ( ch->fighting == NULL )
    return Value((long)0);
  stop_fighting(ch,FALSE);
  return Value((long)0);
}

Value mob_god( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  //return Value(IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
  return Value(char_god_name( ch ));
}

Value mob_master( ENTITY_DATA *entity, const int nparams, Value *params) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  return Value(ch->master);
}

Value mob_actTo(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  Value arg = params[1].get();

  if ( arg.typ == SVT_ENTITY ) // an entity
    act_ultimate_one_related( arg.asEntity(), ch, params[0].asStr() );
  else { // a list of entity
    ValueList *l = arg.asList();
    ENTITY_DATA *entityList[l->size];
    for ( int i = 0; i < l->size; i++ )
      entityList[i] = l->elems[i].asEntity();
    act_ultimate( ch, params[0].asStr(), entityList, l->size );
  }

  return Value((long)0);
}

Value mob_actRoom(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  ROOM_INDEX_DATA *room = ch->in_room;

  if ( nparams == 2 ) { // 2 params: list of related entities or an entity
    Value arg = params[1].get();
    if ( arg.typ == SVT_ENTITY ) { // an entity
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	if ( fch != ch )
	  act_ultimate_one_related( arg.asEntity(), fch, params[0].asStr() );
      }
    }
    else { // a list of entites
      ValueList *l = arg.asList();
      ENTITY_DATA *entityList[l->size];
      for ( int i = 0; i < l->size; i++ )
	entityList[i] = l->elems[i].asEntity();
      
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	if ( fch != ch )
	  act_ultimate( fch, params[0].asStr(), entityList, l->size );
      }
    }
  }
  else { // only 1 param: related entity is ch
    CHAR_DATA *fch, *fch_next;
    for ( fch = room->people; fch; fch = fch_next ) {
      fch_next = fch->next_in_room;
      if ( fch != ch )
	act_ultimate_one_related( ch, fch, params[0].asStr() );
    }
  }

  return Value((long)0);
}

Value mob_actAll(ENTITY_DATA *entity, const int nparams, Value* params) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  ROOM_INDEX_DATA *room = ch->in_room;

  if ( nparams >= 2 ) { // 2 params: list of related entities or an entity
    Value arg = params[1].get();
    if ( arg.typ == SVT_ENTITY ) { // an entity
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate_one_related( arg.asEntity(), fch, params[0].asStr() );
      }
    }
    else { // a list of entities
      ValueList *l = arg.asList();
      ENTITY_DATA *entityList[l->size];
      for ( int i = 0; i < l->size; i++ )
	entityList[i] = l->elems[i].asEntity();
      
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate( fch, params[0].asStr(), entityList, l->size );
      }
    }
  }
  else { // only 1 param: related entity is ch
    CHAR_DATA *fch, *fch_next;
    for ( fch = room->people; fch; fch = fch_next ) {
      fch_next = fch->next_in_room;
	act_ultimate_one_related( ch, fch, params[0].asStr() );
    }
  }

  return Value((long)0);
}

Value mob_getAct( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  if ( IS_NPC(ch) )
    return wordize(flag_string( act_flags, ch->act ));
  else
    return wordize(flag_string( plr_flags, ch->act ));
}

Value mob_setAct( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  Value val = params[0].get();
  const char *s = generalAsStr( val );
  int value;
  if ( IS_NPC(ch) )
    value = flag_value_complete( act_flags, s );
  else
    value = flag_value_complete( plr_flags, s );
  if ( value == NO_FLAG ) {
    bug("mob_setAct: Invalid act value: %s", s );
    return Value((long)0);
  }
  ch->act = value;
  if ( IS_NPC(ch) )
    return Value(flag_string( act_flags, ch->act ) );
  else
    return Value(flag_string( plr_flags, ch->act ) );
}

Value mob_toggleAct( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  Value val = params[0].get();
  const char *s = generalAsStr( val );
  int value;
  if ( IS_NPC(ch) )
    value = flag_value_complete( act_flags, s );
  else
    value = flag_value_complete( plr_flags, s );
  if ( value == NO_FLAG ) {
    bug("mob_toggleAct: Invalid act value: %s", s );
    return Value((long)0);
  }
  ch->act ^= value;
  if ( IS_NPC(ch) )
    return Value(flag_string( act_flags, ch->act ) );
  else
    return Value(flag_string( plr_flags, ch->act ) );
}

Value mob_checkAct( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  Value val = params[0].get();
  const char *s = generalAsStr( val );
  int value;
  if ( IS_NPC(ch) )
    value = flag_value_complete( act_flags, s );
  else 
    value = flag_value_complete( plr_flags, s );

  if ( value == NO_FLAG ) {
    bug("mob_checkAct: Invalid act value: %s", s );
    return Value((long)0);
  }
  return Value(IS_SET(ch->act,value));
}

Value mob_getBaseAttr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char field[MAX_STRING_LENGTH];

  strcpy( field, params[0].asStr() );
  int which = flag_value_complete( attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("mob_getBaseAttr: invalid field (%s) for char (%s)", 
	field, NAME(ch) );
    return Value((long)0);
  }
  // an integer
  if ( attr_table[which].bits == NULL )
    return Value(ch->baseattr[which]);
  // a list or a string
  else
    if ( is_stat(attr_table[which].bits) )
      return Value(flag_string( attr_table[which].bits, ch->baseattr[which] ));
    else 
      return Value(wordize(flag_string( attr_table[which].bits, ch->baseattr[which] )));
}

Value mob_getAttr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char field[MAX_STRING_LENGTH];

  strcpy( field, params[0].asStr() );
  int which = flag_value_complete( attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("mob_getAttr: invalid field (%s) for char (%s)", 
	field, NAME(ch) );
    return Value((long)0);
  }
  // an integer
  if ( attr_table[which].bits == NULL )
    return Value(ch->curattr[which]);
  // a list or a string
  else
    if ( is_stat(attr_table[which].bits) )
      return Value(flag_string( attr_table[which].bits, ch->curattr[which] ));
    else
      return Value(wordize(flag_string( attr_table[which].bits, ch->curattr[which] )));
}

Value mob_checkAttr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  const char *field = params[0].asStr();
  int which = flag_value_complete( attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("mob_checkAttr: invalid field (%s) for char (%s)", 
	field, NAME(ch) );
    return Value((long)0);
  }
  // an integer
  if ( attr_table[which].bits == NULL ) {
    int val = params[1].asInt(); // 2nd arg is an int
    return Value( ch->curattr[which] == val );
  }
  // a list or a string
  else
    if ( is_stat(attr_table[which].bits) ) {
      const char *arg2 = params[1].asStr(); // 2nd arg is a string
      int val = flag_value_complete( attr_table[which].bits, arg2 );
      if ( val == NO_FLAG ) {
	bug("mob_checkAttr: invalid attr value: %s", arg2 );
	return Value((long)0);
      }
      return Value( ch->curattr[which] == val );
    }
    else {
      Value arg2 = params[1].get(); // 2nd arg is a string or a list of string
      const char *s = generalAsStr( arg2 );
      int val = flag_value_complete( attr_table[which].bits, s );
      if ( val == NO_FLAG ) {
	bug("mob_checkAttr: invalid attr value: %s", s );
	return Value((long)0);
      }
      return Value( IS_SET( ch->curattr[which], val ) );
    }
  return Value((long)0);
}

Value mob_getBaseAffect( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  Value v1 = wordize(flag_string( affect_flags, ch->bstat(affected_by)));
  Value v2 = wordize(flag_string( affect2_flags, ch->bstat(affected2_by)));
  // If no affect neither affect2: return a list with "none"
  if ( ch->bstat(affected_by) == 0 && ch->bstat(affected2_by) == 0 )
    return v1;
  // If no affect: return affect2
  if ( ch->bstat(affected_by) == 0 )
    return v2;
  // If no affect2: return affect
  if ( ch->bstat(affected2_by) == 0 )
    return v1;
  // Else return affect+affect2
  return Value(v1+v2);
}

Value mob_getAffect( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  Value v1 = wordize(flag_string( affect_flags, ch->cstat(affected_by)));
  Value v2 = wordize(flag_string( affect2_flags, ch->cstat(affected2_by)));
  // If no affect neither affect2: return a list with "none"
  if ( ch->cstat(affected_by) == 0 && ch->cstat(affected2_by) == 0 )
    return v1;
  // If no affect: return affect2
  if ( ch->cstat(affected_by) == 0 )
    return v2;
  // If no affect2: return affect
  if ( ch->cstat(affected2_by) == 0 )
    return v1;
  // Else return affect+affect2
  return Value(v1+v2);
}

Value mob_checkAffect( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;

  const char *arg1 = params[0].asStr();
  // find if param is an affect1 or affect2
  int v;
  v = flag_value_complete( affect_flags, arg1 );
  if ( v == NO_FLAG ) {
    v = flag_value_complete( affect2_flags, arg1 );
    if ( v == NO_FLAG ) {
      bug("mob_checkAffect: invalid affect: %s", arg1 );
      return Value(long(0));
    }
    return Value( IS_SET(ch->cstat(affected2_by), v ) );
  }
  return Value( IS_SET(ch->cstat(affected_by), v ) );
}


// 1st arg: aggressor
// 2nd    : damage amount
// 3nd    : damage type
// [4th   : format]
// [5th   : relatedEntity]
Value mob_aggrDamage( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *victim = (CHAR_DATA *)entity;
  ROOM_INDEX_DATA *room = victim->in_room;
  CHAR_DATA *ch = (CHAR_DATA *)params[0].asEntity();

  int dam = params[1].asInt();
  int dam_type = flag_value_complete(dam_class_flags, params[2].asStr() );
  if ( dam_type == NO_FLAG ) {
    bug("mob_aggrDamage: Invalid dam_type [%s] assuming DAM_NONE.", params[2].asStr() );
    dam_type = DAM_NONE;
  }

  // if more than 3 param: a string is sent to room
  if ( nparams >= 4 ) {
    if ( nparams == 5 ) { // 5 param: string with related entities
      // construct related entities vector
      ValueList *l = params[4].asList();
      ENTITY_DATA *entityList[l->size];
      for ( int i = 0; i < l->size; i++ )
	entityList[i] = l->elems[i].asEntity();
      
      // send message to room
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate( fch, params[3].asStr(), entityList, l->size );
      }
    }
    else { // only 6 param: string related entity is victim
      // send message to room
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate_one_related( victim, fch, params[3].asStr() );
      }
    }
  }

  int done = ability_damage( ch, victim,
			     dam, TYPE_UNDEFINED, dam_type, FALSE, FALSE );
  if ( done == DAMAGE_DONE )
    set_fighting( ch, victim );
  return Value(done);
}

Value mob_getFaction( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  const char *s = params[0].asStr();

  if ( !str_cmp( s, "all" ) // if all is specified and ch is a PC, return a list of couple (faction name, value)
       && !IS_NPC(ch) ) {
    ValueList *factionList = ValueList::newList(factions_count);
    for ( int i = 0; i < factions_count; i++ ) {
      ValueList *couple = ValueList::newList(2);
      couple->elems[0] = str_dup( faction_table[i].name );
      couple->elems[1] = ch->pcdata->current_faction_friendliness[i];
      factionList->elems[i] = couple;
    }
    return factionList;
  }
  int factionId = get_faction_id(s);
  
  if ( factionId == -1 ) {
    bug("mob_getFaction: invalid faction [%s]", s );
    return Value((long)0);
  }

  if ( IS_NPC(ch) )
    return faction_table[ch->faction].friendliness[factionId];
  else
    return ch->pcdata->current_faction_friendliness[factionId];
}

Value mob_setFaction( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  if ( nparams < 1 || nparams > 2 ) {
    bug("mob_setFaction: too many or not enough arguments: [%d]", nparams );
    return Value((long)0);    
  }
  if ( !IS_NPC(ch) && nparams < 2 ) { // player: at least 2 argument
    bug("mob_setFaction: missing one argument to set NPC [%s] faction", NAME(ch) );
    return Value((long)0);
  }
  const char *s = params[0].asStr();
  int factionId = get_faction_id(s);
  int value = 0;
  if ( nparams == 2 )
    value = params[1].asInt();

  if ( factionId == -1 ) {
    bug("mob_setFaction: invalid faction [%s]", s );
    return Value((long)0);
  }

  if ( IS_NPC(ch) ) { // NPC: just set faction
    ch->faction = factionId;
    return Value((long)0);
  }

  // PC: change faction's value
  ch->pcdata->base_faction_friendliness[factionId] = range_faction(value);
  return Value(ch->pcdata->base_faction_friendliness[factionId]);
}

Value mob_sayTo( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  const char *s = params[1].asStr();
  ENTITY_DATA *target = params[0].asEntity();
  if ( target->kind == CHAR_ENTITY )
    act("{g$n says '{x%t{g'{x", ch, (CHAR_DATA*)target, s, TO_VICT );
  else
    bug("mob_sayTo: Invalid target [%s] (kind: %d)", target->name, target->kind );
  return Value((long)0);
}

Value mob_toggleFollower( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  CHAR_DATA *victim = (CHAR_DATA *)params[0].asEntity();

  if ( victim == ch ) {
    if ( victim->master != NULL )
      stop_follower(victim);
    return victim->master;
  }

  if ( victim->master != NULL )
    stop_follower( victim );

  add_follower( victim, ch );
  return victim->master;
}

Value mob_addAffect( ENTITY_DATA *entity, const int nparams, Value *params ) {
  entity_addAffect( entity, params );
  return Value((long)0);
}

Value mob_savesSpell( ENTITY_DATA *entity, const int nparams, Value *params ) {
  return entity_savesSpell( entity, params );
}

Value mob_isSafeSpell( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;
  CHAR_DATA* victim = (CHAR_DATA*) params[0].asEntity();
  bool area = params[1].asInt();

  // no call to is_safe because that function has output
  //return is_safe( ch, victim );

  return silent_is_safe_spell( ch, victim, area );
  return Value(long(FALSE));// FALSE
}

// 1st arg: aggressor
// 2nd    : damage amount
// 3nd    : damage type
// 4th    : show
// 5th    : old
// 6th    : ability name
// [7th   : format]
// [8th   : relatedEntity]
Value mob_abilityDamage( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *victim = (CHAR_DATA *)entity;
  ROOM_INDEX_DATA *room = victim->in_room;
  CHAR_DATA *ch = (CHAR_DATA *)params[0].asEntity();

  int dam = params[1].asInt();
  int dam_type = flag_value_complete(dam_class_flags, params[2].asStr() );
  if ( dam_type == NO_FLAG ) {
    bug("mob_abilityDamage: Invalid dam_type [%s] assuming DAM_NONE.", params[2].asStr() );
    dam_type = DAM_NONE;
  }
  const char *s = params[5].asStr();
  int sn = ability_lookup( s );
  if ( sn <= 0 ) {
    bug("mob_abilityDamage: Invalid ability [%s]", s );
    sn = TYPE_UNDEFINED;
  }
  int show = params[3].asInt();
  int old = params[4].asInt();

  // if more than 6 param: a string is sent to room
  if ( nparams >= 7 ) {
    if ( nparams == 8 ) { // 8 param: string with related entities
      // construct related entities vector
      ValueList *l = params[7].asList();
      ENTITY_DATA *entityList[l->size];
      for ( int i = 0; i < l->size; i++ )
	entityList[i] = l->elems[i].asEntity();
      
      // send message to room
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate( fch, params[6].asStr(), entityList, l->size );
      }
    }
    else { // only 6 param: string related entity is victim
      // send message to room
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate_one_related( victim, fch, params[6].asStr() );
      }
    }
  }

  int done = ability_damage( ch, victim, 
			     dam, sn, dam_type, show, old );
  return Value(done);
}

Value mob_affectStrip( ENTITY_DATA *entity, const int nparams, Value *params ) {
  entity_affectStrip( entity, params );
  return Value((long)0);
}

Value mob_addDaze( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  int daze = params[0].asInt();
  DAZE_STATE( ch, daze );

  return Value((long)0);
}

Value mob_setStunned( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  int stunned = params[0].asInt();
  ch->stunned = stunned;

  return Value((long)0);
}

Value mob_getStunned( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;

  return Value(ch->stunned);
}

Value mob_getAttrINT( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *) entity;
  char field[MAX_STRING_LENGTH];

  strcpy( field, params[0].asStr() );
  int which = flag_value_complete( attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("mob_getAttr: invalid field (%s) for char (%s)", 
	field, NAME(ch) );
    return Value((long)0);
  }
  return Value(ch->curattr[which]); // just return the integer value, doesn't care about bitvector/int/...
}

Value mob_hasShop( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  if ( IS_NPC(ch) )
    return Value((long)ch->pIndexData->pShop);
  else
    return Value((long)0);
}

// cast( victim, spellName, level, castingLevel )
Value mob_cast( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *mob = (CHAR_DATA*) entity;
  char spell_name[MAX_STRING_LENGTH];
  int casting_level;
  int level;

  if ( !IS_NPC(mob) ) {
    bug("mob_cast [%s]: only mob may use this function", NAME(mob) );
    return Value((long)0);
  }

  ENTITY_DATA *e = params[0].asEntity();
  strcpy( spell_name, params[1].asStr() );
  level = params[2].asInt();
  casting_level = params[3].asInt();
  int sn;
  sn = ability_lookup( spell_name );

  if ( sn <= 0 || ability_table[sn].type != TYPE_SPELL ) {
    bug("mob_cast: invalid sn or not a spell %s", spell_name );
    return Value((long)0);
  }

  int target = TARGET_NONE;
  switch ( ability_table[sn].target ) {
    // defensive spells
  case TAR_CHAR_DEFENSIVE: case TAR_CHAR_SELF: case TAR_CHAR_OFFENSIVE:
    if ( e->kind == CHAR_ENTITY )
      target = TARGET_CHAR;
    else {
      bug("mob_cast (%d): can't cast spell with tar_char_XXX (spell:%s) (target:%s) and a non-char target", 
	  mob->pIndexData->vnum,
	  spell_name,
	  e->name);
      return Value((long)0);
    }
    break;
  case TAR_OBJ_CHAR_DEF: case TAR_OBJ_CHAR_OFF:
    if ( e->kind == CHAR_ENTITY )
      target = TARGET_CHAR;
    else if ( e->kind == OBJ_ENTITY )
      target = TARGET_OBJ;
    else {
      bug("mob_cast (%d): can't cast spell with tar_obj_char_XXX (spell:%s) (target:%s) and a non-char, non-obj target",
	  mob->pIndexData->vnum,
	  spell_name,
	  e->name);
      return Value((long)0);
    }
    break;
  case TAR_IGNORE:
    target = TARGET_NONE;
    break;
  default:
    bug("mob_cast (%d): invalid target %d (spell:%s) (target:%s)",
	mob->pIndexData->vnum,
	ability_table[sn].target,
	spell_name,
	e->name );
    return Value((long)0);
  }
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, level, mob, (void*)e, target, casting_level );
  return Value((long)0);
}

Value mob_findPath( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  CHAR_DATA *victim = (CHAR_DATA *)params[0].asEntity();
  int depth = params[1].asInt();
  bool in_zone = params[2].asInt();

  if ( victim == NULL || !victim->valid || victim->in_room == NULL ) {
    bug("mob_findPath: victim doesn't exist or in_room is NULL");
    return Value((long)0);
  }
  if ( ch->in_room == NULL ) {
    bug("mob_findPath: ch [%s | %d] in_room is NULL", NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
    return Value((long)0);
  }
  char path[MAX_INPUT_LENGTH];
  return find_path( ch->in_room->vnum, victim->in_room->vnum, ch, depth, in_zone, path );
}

Value mob_isImmortal( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  return Value(IS_IMMORTAL(ch));
}

Value mob_description( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  if ( ch->description == NULL
       || ch->description[0] == '\0' )
    return Value("");
  else
    return Value(ch->description);
}

Value mob_setDescription( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA* ch = (CHAR_DATA*) entity;

  if ( !IS_NPC(ch) ) {
    bug("mob_setDescription: Trying to set description of player [%s]", NAME(ch) );
    return Value((long)0);
  }
  const char *s = params[0].asStr();
  char buf[MAX_STRING_LENGTH*4];
  sprintf( buf, "%s\n\r", s );
  ch->description = str_dup( buf );
  return Value(ch->description);
}

Value mob_giveObj( ENTITY_DATA *entity, const int nparams, Value *params ) {
  CHAR_DATA *ch = (CHAR_DATA *)entity;
  OBJ_DATA *obj = (OBJ_DATA *)params[0].asEntity();
  
  if ( obj == NULL ) {
    bug("mob_giveItem: obj is NULL");
    return Value(long(0));
  }
  
  if ( obj->in_obj != NULL )
    obj_from_obj(obj);
  else if ( obj->carried_by != NULL )
    obj_from_char(obj);
  else if ( obj->in_room != NULL )
    obj_from_room(obj);
  obj_to_char(obj,ch);
  return obj;
}

/************************* Obj Predifined functions ********************/

#define NEW_OBJ_FIELD_METHOD(field_name)			\
Value obj_ ## field_name(ENTITY_DATA *entity, const int nparams, Value* params) {	\
  OBJ_DATA* obj = (OBJ_DATA*) entity;                           \
  return Value(obj->field_name);			        \
}

NEW_OBJ_FIELD_METHOD(name);
NEW_OBJ_FIELD_METHOD(level);
NEW_OBJ_FIELD_METHOD(cost);

Value obj_carriedBy( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;

  return Value(obj->carried_by);
}

Value obj_shortDescr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;

  return Value(obj->short_descr);
}

Value obj_getCondition( ENTITY_DATA *entity, const int nparams, Value *params) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  return Value(obj->condition);
}

Value obj_setCondition( ENTITY_DATA *entity, const int nparams, Value *params) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  int condition = params[0].asInt();

  if ( condition > 0 ) {
    obj->condition = condition;
    return Value(1);
  }

  return Value((long)0);
}

Value obj_say( ENTITY_DATA *entity, const int nparams, Value *params) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  ROOM_INDEX_DATA *room = get_room_obj( obj );
  CHAR_DATA *to = room->people;

  char msg[MAX_INPUT_LENGTH];
  strcpy( msg, params[0].asStr() );
  for ( ; to; to = to->next_in_room )
    send_to_charf( to,
		   "{g%s says '{x%s{g'{x\n\r",
		   obj->short_descr, 
		   msg );
  return Value((long)0);
}

Value obj_tell( ENTITY_DATA *entity, const int nparams, Value *params) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  CHAR_DATA *victim = (CHAR_DATA*) params[0].asEntity();

  if ( victim != NULL )
    send_to_charf( victim,
		       "{g%s tells you '{x%s{g'{x\n\r", 
		       can_see_obj( victim, obj )?obj->short_descr:"Something", 
		       params[1].asStr() );

  return Value((long)0);
}

Value obj_vnum( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *) entity;

  return Value(obj->pIndexData->vnum);
}

Value obj_material( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA*) entity;
  
  return Value(material_table[obj->material].name);
}

Value obj_itemType( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA*) entity;
  
  //return Value(item_name(obj->item_type));
  return Value(flag_string( type_flags, obj->item_type ));
}

Value obj_destroy( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA*) entity;

  extract_obj( obj );

  return Value(obj);
}

Value obj_getNumber( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA*) entity;

  return get_obj_number( obj );
}

Value obj_getWeight( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA*) entity;

  return get_obj_weight( obj );
}

Value obj_isEquiv( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obja = (OBJ_DATA*) entity;
  OBJ_DATA *objb = (OBJ_DATA*) params[0].asEntity();

  return valueEquiv( obja, objb );
}

Value obj_cast( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA*) entity;
  char spell_name[MAX_STRING_LENGTH];
  int casting_level;
  int level;

  ENTITY_DATA *e = params[0].asEntity();
  strcpy( spell_name, params[1].asStr() );
  level = params[2].asInt();
  casting_level = params[3].asInt();
  int sn;
  sn = ability_lookup( spell_name );

  if ( sn <= 0 || ability_table[sn].type != TYPE_SPELL ) {
    bug("obj_cast: invalid sn or not a spell %s", spell_name );
    return Value((long)0);
  }

  int target = TARGET_NONE;
  switch ( ability_table[sn].target ) {
    // defensive spells
  case TAR_CHAR_DEFENSIVE: case TAR_OBJ_CHAR_DEF: case TAR_CHAR_SELF:
    target = TARGET_CHAR;
    break;
 // NOT YET IMPLEMENTED 
  case TAR_IGNORE:
    bug("obj_cast (%d): can't cast spell with tar_ignore (spell:%s) (target:%s)", 
	obj->pIndexData->vnum,	
	spell_name,
	e->name);
    return Value((long)0);
  case TAR_CHAR_OFFENSIVE:
    bug("obj_cast (%d): can't cast offensive spell (spell:%s) (target:%s)", 
	obj->pIndexData->vnum,
	spell_name,
	e->name);
    return Value((long)0);
  case TAR_OBJ_INV: case TAR_OBJ_CHAR_OFF:
    bug("obj_cast (%d): can't cast spell on items (spell:%s) (target:%s)",
	obj->pIndexData->vnum,
	spell_name,
	e->name );
    return Value((long)0);
  default:
    bug("obj_cast (%d): invalid target %d (spell:%s) (target:%s)",
	obj->pIndexData->vnum,
	ability_table[sn].target,
	spell_name,
	e->name );
    return Value((long)0);
  }
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, level, (CHAR_DATA*)e, (void*)e, target, casting_level );
  return Value((long)0);
}

Value obj_echo( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *) entity;
  char msg[MAX_STRING_LENGTH];
  
  strcpy( msg, params[0].asStr() );
  if ( msg[strlen(msg)-1] != '\n'
       && msg[strlen(msg)-1] != '\r' )
    strcat( msg, "\n\r");
  if ( obj->in_room != NULL ) {
    CHAR_DATA *fch;
    for ( fch = obj->in_room->people; fch != NULL; fch = fch->next_in_room ) {
      send_to_charf( fch,"%s", msg );
    }
  } 
  else if ( obj->carried_by != NULL )
    send_to_charf( obj->carried_by,"%s", msg );
  else {
    bug("obj_echo: obj (%s) not in a room or on a player", obj->short_descr);
    return Value((long)0);
  }

  return Value((long)0);
}

Value obj_mLoad( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *) entity;
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *victim;
  int vnum = params[0].asInt();

  if ( ( pMob = get_mob_index(vnum) ) == NULL ) {
    bug("obj_mLoad: invalid vnum (%d) obj (%s)",
	vnum, obj->short_descr);
    return Value((long)0);
  }
  victim = create_mobile(pMob);
  ROOM_INDEX_DATA *pRoom;
  if ( obj->in_room ) 
    pRoom = obj->in_room;
  else {
    pRoom = get_room_index(ROOM_VNUM_LIMBO);
    bug("obj_load: mob loaded in limbo");
  }
  char_to_room( victim, pRoom );
  // Added by SinaC 2001
  //recomproom( pRoom ); NO NEED: done in char_to_room

  return Value(victim);
}

Value obj_oLoad( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *from = (OBJ_DATA *) entity;
  OBJ_INDEX_DATA *pObj;
  OBJ_DATA *obj;
  int vnum = params[0].asInt();

  if ( ( pObj = get_obj_index(vnum) ) == NULL ) {
    bug("obj_oLoad: invalid vnum (%d) obj (%s)",
	vnum, from->short_descr);
    return Value((long)0);
  }
  obj = create_object(pObj, 0);

  ROOM_INDEX_DATA *pRoom;
  if ( from->in_room ) 
    pRoom = from->in_room;
  else {
    pRoom = get_room_index(ROOM_VNUM_LIMBO);
    bug("obj_oLoad: obj loaded in limbo");
  }
  obj_to_room( obj, pRoom );
  // Added by SinaC 2001
  recomproom( pRoom );

  return Value(obj);
}

Value obj_room( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;

  return Value(obj->in_room);
}

Value obj_description( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;

  if ( obj->description == NULL
       || obj->description[0] == '\0' )
    return Value(obj->short_descr );
  else
    return Value(obj->description);
}

Value obj_getTimer( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  return Value(obj->timer);
}

Value obj_setTimer( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  int time = params[0].asInt();

  obj->timer = time;

  return Value((long)0);
}

Value obj_size( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  return Value(flag_string(size_flags, obj->size));
}

Value obj_contains( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *container = (OBJ_DATA *)entity;

  // count objects
  int count = 0;
  for ( OBJ_DATA *obj = container->contains; obj; obj = obj->next_content )
    count++;

  ValueList *contains = ValueList::newList(count);

  // put objects in the list
  int i = 0;
  for ( OBJ_DATA *obj = container->contains; obj; obj = obj->next_content )
    contains->elems[i++] = Value(obj);
  
  return contains;
}

Value obj_getValue( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *) entity;

  int whichOne = params[0].asInt();
  if ( whichOne < 0 || whichOne > 4 ) {
    bug("obj_getValue: Invalid index: %d", whichOne );
    return Value((long)0);
  }
  return Value(obj->value[whichOne]);
}

Value obj_setValue( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  int whichOne = params[0].asInt();
  int val = params[1].asInt();
  
  if ( whichOne < 0 || whichOne > 4 ) {
    bug("obj_setValue: Invalid index: %d", whichOne );
    return Value((long)0);
  }
  //FIXME: we should take care about item type
  // val will not always be an Int
  obj->baseval[whichOne] = val;
  obj->value[whichOne] = val;
  recompobj(obj);
  return Value((long)0); // should return flag_string
}

Value obj_toggleValue( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  int whichOne = params[0].asInt();
  int val = params[1].asInt();
  
  if ( whichOne < 0 || whichOne > 4 ) {
    bug("obj_toggleValue: Invalid index: %d", whichOne );
    return Value((long)0);
  }
  //FIXME: we should take care about item type
  // val will not always be an Int
  obj->baseval[whichOne] ^= val;
  obj->value[whichOne] ^= val;
  recompobj(obj);
  return Value((long)0); // should return flag_string
}

Value obj_checkValue( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  int whichOne = params[0].asInt();
  
  if ( whichOne < 0 || whichOne > 4 ) {
    bug("obj_checkValue: Invalid index: %d", whichOne );
    return Value((long)0);
  }
  // NOT FINISHED: what about other item_type
  const char *s;
  int val;
  switch ( obj->item_type ) {
    // weapon
    // v0 != RANGED
    //  v0: weapon type   v1: number of dice   v2: type of dice   v3: weapon dam msg   v4: special flag
    // v0 == RANGED
    //  v0: weapon type   v1: string cond      v2: string mod     v3: strength         v4: max dist
  case ITEM_WEAPON:
    switch( whichOne ) {
    case 0:
      s = params[1].asStr();
      val = flag_value_complete( weapon_class, s );
      if ( val == NO_FLAG ) {
	bug("obj_checkValue: invalid weapon class [%s]", s );
	return Value((long)0);
      }
      return Value(obj->value[whichOne] == val );
      break;
    case 1:
    case 2:
      val = params[1].asInt();
      return Value(obj->value[whichOne] == val );
      break;
    case 3:
      if ( obj->value[0] == WEAPON_RANGED ) // ranged
	val = params[1].asInt();
      else {
	// non-ranged
	s = params[1].asStr();
	val = attack_lookup( s );
	if ( val < 0 ) {
	  bug("obj_checkValue: invalid weapon attack [%s]", s );
	  return Value((long)0);
	}
      }
      return Value(obj->value[whichOne] == val );
      break;
    case 4:
      if ( obj->value[0] == WEAPON_RANGED ) { // ranged
	val = params[1].asInt();
	return Value(obj->value[whichOne] == val );
      }
      // non-ranged
      s = params[1].asStr();
      val = flag_value_complete( weapon_type2, s );
      if ( val == NO_FLAG ) {
	bug("obj_checkValue: invalid weapon flag [%s]", s );
	return Value((long)0);
      }
      return Value(IS_SET(obj->value[whichOne],val));
      break;
    }
    break;
    // non-weapon just test compare int value
  default:
    val = params[1].asInt();
    return Value(obj->value[whichOne] == val );
    break;
  }
  return Value((long)0);
}

Value obj_getExtraFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;

  return wordize(flag_string( extra_flags, obj->extra_flags ));
}

Value obj_setExtraFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  Value val = params[0].get();

  const char *s = generalAsStr(val);
  int value = flag_value_complete( extra_flags, s );
  if ( value == NO_FLAG ) {
    bug("obj_setExtraFlags: invalid value [%s]", s );
    return Value((long)0);
  }
  
  obj->base_extra = value;
  obj->extra_flags = value;
  return Value((long)0);
}

Value obj_toggleExtraFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  Value val = params[0].get();

  const char *s = generalAsStr(val);
  int value = flag_value_complete( extra_flags, s );
  if ( value == NO_FLAG ) {
    bug("obj_toggleExtraFlags: invalid value [%s]", s );
    return Value((long)0);
  }
  
  obj->base_extra ^= value;
  obj->extra_flags ^= value;
  return Value((long)0); // should return flag_string
}

Value obj_checkExtraFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  Value val = params[0].get();

  const char *s = generalAsStr(val);
  int value = flag_value_complete( extra_flags, s );
  if ( value == NO_FLAG ) {
    bug("obj_checkExtraFlags: invalid value [%s]", s );
    return Value((long)0);
  }
  return Value(IS_SET(obj->extra_flags, value));
}

Value obj_getWearFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;

  return wordize(flag_string( wear_flags, obj->wear_flags ));
}

Value obj_setWearFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  Value val = params[0].get();

  const char *s = generalAsStr(val);
  int value = flag_value_complete( wear_flags, s );
  if ( value == NO_FLAG ) {
    bug("obj_setWearFlags: invalid value [%s]", s );
    return Value((long)0);
  }
  
  obj->wear_flags = value;
  return Value((long)0); // should return flag_string
}

Value obj_toggleWearFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  Value val = params[0].get();

  const char *s = generalAsStr(val);
  int value = flag_value_complete( wear_flags, s );
  if ( value == NO_FLAG ) {
    bug("obj_toggleWearFlags: invalid value [%s]", s );
    return Value((long)0);
  }
  
  obj->wear_flags ^= value;
  return Value((long)0); // should return flag_string
}

Value obj_checkWearFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA* obj = (OBJ_DATA*) entity;
  Value val = params[0].get();

  const char *s = generalAsStr(val);
  int value = flag_value_complete( wear_flags, s );
  if ( value == NO_FLAG ) {
    bug("obj_checkWearFlags: invalid value [%s]", s );
    return Value((long)0);
  }
  return Value(IS_SET(obj->wear_flags, value));
}

Value obj_actAll(ENTITY_DATA *entity, const int nparams, Value* params) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  ROOM_INDEX_DATA *room = get_room_obj( obj );
  if ( room == NULL )
    return Value((long)0);

  if ( nparams >= 2 ) { // 2 params: list of related entities or an entity
    Value arg = params[1].get();
    if ( arg.typ == SVT_ENTITY ) { // an entity
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate_one_related( arg.asEntity(), fch, params[0].asStr() );
      }
    }
    else { // a list of entities
      ValueList *l = arg.asList();
      ENTITY_DATA *entityList[l->size];
      for ( int i = 0; i < l->size; i++ )
	entityList[i] = l->elems[i].asEntity();
      
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate( fch, params[0].asStr(), entityList, l->size );
      }
    }
  }
  else { // only 1 param: related entity is obj
    CHAR_DATA *fch, *fch_next;
    for ( fch = room->people; fch; fch = fch_next ) {
      fch_next = fch->next_in_room;
	act_ultimate_one_related( obj, fch, params[0].asStr() );
    }
  }

  return Value((long)0);
}

Value obj_setName( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  obj->name = str_dup( params[0].asStr() );
  return Value(obj->name);
}

Value obj_setShortDescr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  obj->short_descr = str_dup( params[0].asStr() );
  return Value(obj->short_descr);
}

Value obj_setDescription( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  obj->description = str_dup( params[0].asStr() );
  return Value(obj->description);
}

Value obj_addAffect( ENTITY_DATA *entity, const int nparams, Value *params ) {
  entity_addAffect( entity, params );
  return Value((long)0);
}

Value obj_savesSpell( ENTITY_DATA *entity, const int nparams, Value *params ) {
  return entity_savesSpell( entity, params );
}

Value obj_affectStrip( ENTITY_DATA *entity, const int nparams, Value *params ) {
  entity_affectStrip( entity, params );
  return Value((long)0);
}

Value obj_setLevel( ENTITY_DATA *entity, const int nparams, Value *params ) {
  OBJ_DATA *obj = (OBJ_DATA *)entity;
  int level = params[0].asInt();
  if ( level < 0 || level > 110 ) {
    bug("obj_setLevel: level = [%d]", level );
    return Value((long)0);
  }
  obj->level = level;
  return obj->level;
}

// Added by SinaC 2003
/************************* Room Predifined functions ********************/

#define NEW_ROOM_FIELD_METHOD(field_name)			\
Value room_ ## field_name(ENTITY_DATA *entity, const int nparams, Value* params) {	\
  ROOM_INDEX_DATA* room = (ROOM_INDEX_DATA*) entity;             \
  return Value(room->field_name);			        \
}

NEW_ROOM_FIELD_METHOD(name);
NEW_ROOM_FIELD_METHOD(vnum);
NEW_ROOM_FIELD_METHOD(description);
NEW_ROOM_FIELD_METHOD(owner);

Value room_echo( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA* room = (ROOM_INDEX_DATA*) entity;
  char msg[MAX_STRING_LENGTH];
  
  strcpy( msg, params[0].asStr() );
  if ( msg[strlen(msg)-1] != '\n'
       && msg[strlen(msg)-1] != '\r' )
    strcat( msg, "\n\r");

  CHAR_DATA *fch;
  for ( fch = room->people; fch != NULL; fch = fch->next_in_room ) {
    send_to_charf( fch, "%s", msg );
  }
  return Value((long)0);
}

Value room_getAttr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char field[MAX_STRING_LENGTH];

  strcpy( field, params[0].asStr() );
  int which = flag_value_complete( room_attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("room_getAttr: invalid field (%s) for room (%d)", 
	field, room->vnum );
    return Value((long)0);
  }
  // an integer
  if ( room_attr_table[which].bits == NULL )
    return Value(room->curattr[which]);
  // a list or a string
  else
    if ( is_stat(room_attr_table[which].bits) )
      return Value(flag_string( room_attr_table[which].bits, room->curattr[which] ));
    else
      return Value(wordize(flag_string( room_attr_table[which].bits, room->curattr[which] )));
}

Value room_getBaseAttr( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char field[MAX_STRING_LENGTH];

  strcpy( field, params[0].asStr() );
  int which = flag_value_complete( room_attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("room_getBaseAttr: invalid field (%s) for room (%d)", 
	field, room->vnum );
    return Value((long)0);
  }
  // an integer
  if ( room_attr_table[which].bits == NULL )
    return Value(room->baseattr[which]);
  // a list or a string
  else
    if ( is_stat(room_attr_table[which].bits) )
      return Value(flag_string( room_attr_table[which].bits, room->baseattr[which] ));
    else
      return Value(wordize(flag_string( room_attr_table[which].bits, room->baseattr[which] )));
}

Value room_people( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;

  if ( room->people == NULL )
    return ValueList::emptyList();

  // count mobs
  int n = 0;
  for (CHAR_DATA *fch = room->people; fch; fch = fch->next_in_room )
    n++;
  ValueList *inroom = ValueList::newList(n);

  int i = 0;
  for (CHAR_DATA *fch = room->people; fch; fch = fch->next_in_room ) {
    inroom->elems[i++] = Value(fch);
  }
  return inroom;
}

Value room_contents( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;

  if ( room->contents == NULL )
    return ValueList::emptyList();

  // count objects
  int count = 0;
  for ( OBJ_DATA *obj = room->contents; obj; obj = obj->next_content )
    count++;

  ValueList *contents = ValueList::newList(count);

  int i = 0;
  // put objects in the list
  for ( OBJ_DATA *obj = room->contents; obj; obj = obj->next_content )
    contents->elems[i++] = Value(obj);
  
  return contents;
}

Value room_clan( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;

  return Value( get_clan_table(room->clan)->name );
}

Value room_guild( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;

  return Value(flag_string(classes_flags,room->guild));
}

Value room_exits( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  // count number of exits
  EXIT_DATA *pexit;
  int count = 0;
  for ( int door = 0; door < MAX_DIR; door++ ) // Modified by SinaC 2003
    if ( ( pexit = room->exit[door] ) != NULL
	 && pexit->u1.to_room != NULL )
      count++;

  ValueList *exitsList = ValueList::newList(count);

  int i = 0;
  for ( int door = 0; door < MAX_DIR; door++ ) // Modified by SinaC 2003
    if ( ( pexit = room->exit[door] ) != NULL
	 && pexit->u1.to_room != NULL )
      exitsList->elems[i++] = Value(short_dir_name[door]);

  return exitsList;
}

Value room_roomDir( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];

  strcpy( dirName, params[0].asStr() );

  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_roomDir: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return Value((ROOM_INDEX_DATA *)NULL);
  }


  EXIT_DATA *pexit = room->exit[dirCount];

  if ( pexit == NULL ||
       pexit->u1.to_room == NULL )
    return Value((ROOM_INDEX_DATA *)NULL);
  else
    return Value(pexit->u1.to_room);
}

Value room_mLoad( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  MOB_INDEX_DATA *pMob;
  CHAR_DATA *victim;
  int vnum = params[0].asInt();

  if ( ( pMob = get_mob_index(vnum) ) == NULL ) {
    bug("room_mLoad: invalid vnum (%d) room (%s)[%d]",
	vnum, room->name, room->vnum );
    return Value((long)0);
  }
  victim = create_mobile(pMob);
  char_to_room( victim, room );
  //recomproom( room ); NO NEED: done in char_to_room

  return Value(victim);
}

Value room_oLoad( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  OBJ_INDEX_DATA *pObj;
  OBJ_DATA *obj;
  int vnum = params[0].asInt();

  if ( ( pObj = get_obj_index(vnum) ) == NULL ) {
    bug("room_oLoad: invalid vnum (%d) room (%s)[%d]",
	vnum, room->name, room->vnum );
    return Value((long)0);
  }

  obj = create_object(pObj, 0);
  obj_to_room( obj, room );
  recomproom( room );

  return Value(obj);
}

Value room_linked( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  return Value( !IS_SET( room->area->area_flags, AREA_NOTELEPORT ) );
}

Value room_areaName( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  return Value(parse_name(room->area->name));
}

Value room_transfer( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  ENTITY_DATA *e = params[0].asEntity();
  switch( e->kind ) {
  case CHAR_ENTITY: {
    CHAR_DATA *victim = (CHAR_DATA *) e;
    
    char_from_room(victim);
    char_to_room(victim,room);
    break; }
  case OBJ_ENTITY: {
    OBJ_DATA *obj = (OBJ_DATA *) e;
    if ( obj->in_obj != NULL )
      obj_from_obj(obj);
    else if ( obj->carried_by != NULL )
      obj_from_char(obj);
    else if ( obj->in_room != NULL )
      obj_from_room(obj);
    obj_to_room(obj,room);

    break; }
    // Added by SinaC 2003 for room program
  case ROOM_ENTITY: {
    ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) e;

    bug("room_transfer: %s is a room.", room->name );
    break; }
    
  }
  return Value((long)0);
}

Value room_createDoor( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];
  strcpy( dirName, params[1].asStr() );
  int roomVnum = params[0].asInt();
  ROOM_INDEX_DATA *toRoom = get_room_index(roomVnum);
  EXIT_DATA *pExit;

  if ( toRoom == NULL ) {
    bug("room_createDoor: invalid room vnum (%d) for room (%d)",
	roomVnum, room->vnum );
    return Value((long)0);
  }

  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_createDoor: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( room->exit[dirCount] ) {
    bug("room_createDoor: exit (%s) already exists for room (%d)",
	dirName, room->vnum);
    return Value((long)0);
  }
  int rev = rev_dir[dirCount];
  if ( toRoom->exit[rev] ) {
    bug("room_createDoor: remote side's exit (%s) already exists for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }

  // from  room  to  toRoom
  pExit = new_exit();
  pExit->u1.to_room = toRoom;
  pExit->orig_door = dirCount;
  SET_BIT(pExit->rs_flags,EX_NOSAVING);
  SET_BIT(pExit->exit_info,EX_NOSAVING);
  room->exit[dirCount] = pExit;

  // from toRoom  to  room
  pExit = new_exit();
  pExit->u1.to_room = room;
  pExit->orig_door = rev;
  SET_BIT(pExit->rs_flags,EX_NOSAVING);
  SET_BIT(pExit->exit_info,EX_NOSAVING);
  toRoom->exit[rev] = pExit;
  
  return Value((long)0);
}
Value room_deleteDoor( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];
  strcpy( dirName, params[0].asStr() );

  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_deleteDoor: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( room->exit[dirCount] == NULL ) {
    bug("room_deleteDoor: no (%s) exit found for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( !IS_SET(room->exit[dirCount]->rs_flags,EX_NOSAVING) ) {
    bug("room_deleteDoor: trying to delete a non-dynamic exit (%s) for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }
 
  int rev = rev_dir[dirCount];
  ROOM_INDEX_DATA *pToRoom = room->exit[dirCount]->u1.to_room;

  if ( pToRoom->exit[rev] == NULL ) {
    bug("room_deleteDoor: no remote side (%s) exit found for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( !IS_SET(pToRoom->exit[rev]->rs_flags,EX_NOSAVING) ) {
    bug("room_deleteDoor: trying to delete a non-dynamic remove side exit (%s) for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }

  // Free remote side
  pToRoom->exit[rev] = NULL;

  // Free exit
  room->exit[dirCount] = NULL;
  return Value((long)0);
}

Value room_createOneWayDoor( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];
  strcpy( dirName, params[1].asStr() );
  int roomVnum = params[0].asInt();
  ROOM_INDEX_DATA *toRoom = get_room_index(roomVnum);
  EXIT_DATA *pExit;

  if ( toRoom == NULL ) {
    bug("room_createDoor: invalid room vnum (%d) for room (%d)",
	roomVnum, room->vnum );
    return Value((long)0);
  }

  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_createDoor: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( room->exit[dirCount] ) {
    bug("room_createDoor: exit (%s) already exists for room (%d)",
	dirName, room->vnum);
    return Value((long)0);
  }

  // from  room  to  toRoom
  pExit = new_exit();
  pExit->u1.to_room = toRoom;
  pExit->orig_door = dirCount;
  SET_BIT(pExit->rs_flags,EX_NOSAVING);
  SET_BIT(pExit->exit_info,EX_NOSAVING);
  room->exit[dirCount] = pExit;
  
  return Value((long)0);
}
Value room_deleteOneWayDoor( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];
  strcpy( dirName, params[0].asStr() );

  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_deleteDoor: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( room->exit[dirCount] == NULL ) {
    bug("room_deleteDoor: no (%s) exit found for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }

  if ( !IS_SET(room->exit[dirCount]->rs_flags,EX_NOSAVING) ) {
    bug("room_deleteDoor: trying to delete a non-dynamic exit (%s) for room (%d)",
	dirName, room->vnum );
    return Value((long)0);
  }
 
  room->exit[dirCount] = NULL;
  return Value((long)0);
}

Value room_around( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;

  // count number of exits
  EXIT_DATA *pexit;
  int count = 0;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( ( pexit = room->exit[door] ) != NULL
	 && pexit->u1.to_room != NULL )
      count++;

  ValueList *around = ValueList::newList(count);

  int i = 0;
  for ( int door = 0; door < MAX_DIR; door++ ) //
    if ( ( pexit = room->exit[door] ) != NULL
	 && pexit->u1.to_room != NULL )
      around->elems[i++] = Value(pexit->u1.to_room);

  return around;
}

Value room_getExitFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];

  strcpy( dirName, params[0].asStr() );
  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_getExitFlags: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return ValueList::emptyList();
  }

  EXIT_DATA *pexit = room->exit[dirCount];

  if ( pexit == NULL ||
       pexit->u1.to_room == NULL )
    return ValueList::emptyList();

  // rs_flags is not revelant
  //return Value(wordize(flag_string( exit_flags, pexit->rs_flags )));
  return Value(wordize(flag_string( exit_flags, pexit->exit_info )));
}

Value room_setExitFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];

  strcpy( dirName, params[0].asStr() );
  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_setExitFlags: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return ValueList::emptyList();;
  }

  EXIT_DATA *pexit = room->exit[dirCount];

  if ( pexit == NULL ||
       pexit->u1.to_room == NULL )
    return ValueList::emptyList();;

  // get new flags
  Value val = params[1].get();
  const char *s = generalAsStr(val);

  int value;
  if ( ( value = flag_value_complete( exit_flags, s ) ) == NO_FLAG ) {
    bug("room_setExitFlags: invalid exit flags (%s) for room (%d)", s, room->vnum );
    return ValueList::emptyList();;
  }
  ROOM_INDEX_DATA *pToRoom;
  int rev;

  //rs_flags should be modified only from OLC
  //room->exit[dirCount]->rs_flags = value;
  //room->exit[dirCount]->exit_info = room->exit[dirCount]->rs_flags;
  room->exit[dirCount]->exit_info = value;
  
  //Connected room.
  pToRoom = room->exit[dirCount]->u1.to_room;     /* ROM OLC */
  rev = rev_dir[dirCount];
  
  if (pToRoom->exit[rev] != NULL) {
    //pToRoom->exit[rev]->rs_flags =  value;
    pToRoom->exit[rev]->exit_info = value;
  }

  //rs_flags is not revelant
  //return Value(wordize(flag_string( exit_flags, pexit->rs_flags )));
  return Value(wordize(flag_string( exit_flags, pexit->exit_info )));
}

Value room_toggleExitFlags( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA *) entity;
  char dirName[MAX_STRING_LENGTH];

  strcpy( dirName, params[0].asStr() );
  // which directions ?
  int dirCount = -1;
  for ( int door = 0; door < MAX_DIR; door++ )
    if ( !str_cmp( short_dir_name[door], dirName ) ) {
      dirCount = door;
      break;
    }

  if ( dirCount == -1 ) {
    bug("room_toggleExitFlags: invalid direction (%s) for room (%d)", 
	dirName, room->vnum );
    return Value((long)0);
  }

  EXIT_DATA *pexit = room->exit[dirCount];

  if ( pexit == NULL ||
       pexit->u1.to_room == NULL )
    return Value((long)0);

  // get new flags
  Value val = params[1].get();
  const char *s = generalAsStr(val);
  int value;
  if ( ( value = flag_value_complete( exit_flags, s ) ) == NO_FLAG ) {
    bug("room_toggleExitFlags: invalid exit flags (%s) for room (%d)", s, room->vnum );
    return Value((long)0);
  }
  ROOM_INDEX_DATA *pToRoom;
  int rev;

  //rs_flags should be modified only from OLC
  //TOGGLE_BIT( room->exit[dirCount]->rs_flags, value );
  //room->exit[dirCount]->exit_info = room->exit[dirCount]->rs_flags;
  TOGGLE_BIT( room->exit[dirCount]->exit_info, value );
  
  //Connected room.
  pToRoom = room->exit[dirCount]->u1.to_room;     /* ROM OLC */
  rev = rev_dir[dirCount];
  
  if (pToRoom->exit[rev] != NULL) {
    //TOGGLE_BIT( pToRoom->exit[rev]->rs_flags, value );
    TOGGLE_BIT( pToRoom->exit[rev]->exit_info, value );
  }

  //rs_flags is not revelant
  //return Value(wordize(flag_string( exit_flags, pexit->rs_flags )));
  return Value(wordize(flag_string( exit_flags, pexit->exit_info )));
}

Value room_actAll(ENTITY_DATA *entity, const int nparams, Value* params) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA*) entity;

  if ( nparams >= 2 ) { // 2 params: list of related entities or an entity
    Value arg = params[1].get();
    if ( arg.typ == SVT_ENTITY ) { // an entity
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate_one_related( arg.asEntity(), fch, params[0].asStr() );
      }
    }
    else { // a list of entities
      ValueList *l = arg.asList();
      ENTITY_DATA *entityList[l->size];
      for ( int i = 0; i < l->size; i++ )
	entityList[i] = l->elems[i].asEntity();
      
      CHAR_DATA *fch, *fch_next;
      for ( fch = room->people; fch; fch = fch_next ) {
	fch_next = fch->next_in_room;
	act_ultimate( fch, params[0].asStr(), entityList, l->size );
      }
    }
  }
  else { // only 1 param: related entity is obj
    CHAR_DATA *fch, *fch_next;
    for ( fch = room->people; fch; fch = fch_next ) {
      fch_next = fch->next_in_room;
	act_ultimate_one_related( room, fch, params[0].asStr() );
    }
  }

  return Value((long)0);
}

Value room_checkAttr(ENTITY_DATA *entity, const int nparams, Value* params) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA*) entity;
  const char *field;
  
  field =  params[0].asStr();
  int which = flag_value_complete( room_attr_flags, field );
  if ( which == NO_FLAG ) {
    bug("room_checkAttr: invalid field (%s) for room (%d)", 
	field, room->vnum );
    return Value((long)0);
  }
  // an integer
  if ( room_attr_table[which].bits == NULL ) {
    int val = params[1].asInt(); // 2nd arg is an int
    return Value(room->curattr[which] == val);
  }
  // a list or a string
  else
    if ( is_stat(room_attr_table[which].bits) ) {
      const char *arg2 = params[1].asStr(); // 2nd arg is a string
      int val = flag_value_complete( room_attr_table[which].bits, arg2 );
      if ( val == NO_FLAG ) {
	bug("room_checkAttr: invalid attr value: %s", arg2 );
	return Value((long)0);
      }
      return Value( room->curattr[which] == val );
    }
    else {
      Value arg2 = params[1].get(); // 2nd arg is a string or a list of string
      const char *s = generalAsStr( arg2 );
      int val = flag_value_complete( room_attr_table[which].bits, s );
      if ( val == NO_FLAG ) {
	bug("room_checkAttr: invalid attr value: %s", s );
	return Value((long)0);
      }
      return Value( IS_SET( room->curattr[which], val ) );
    }
  return Value((long)0);
}

Value room_cast( ENTITY_DATA *entity, const int nparams, Value *params ) {
  ROOM_INDEX_DATA *room = (ROOM_INDEX_DATA*) entity;
  char spell_name[1024];
  int casting_level;
  int level;

  ENTITY_DATA *e = params[0].asEntity();
  strcpy( spell_name, params[1].asStr() );
  level = params[2].asInt();
  casting_level = params[3].asInt();
  int sn;
  sn = ability_lookup( spell_name );

  if ( sn <= 0 || ability_table[sn].type != TYPE_SPELL ) {
    bug("room_cast (%d): invalid sn or not a spell %s", room->vnum, spell_name );
    return Value((long)0);
  }

  int target = TARGET_NONE;
  switch ( ability_table[sn].target ) {
    // defensive spells
  case TAR_CHAR_DEFENSIVE: case TAR_OBJ_CHAR_DEF: case TAR_CHAR_SELF:
    target = TARGET_CHAR;
    break;
 // NOT YET IMPLEMENTED 
  case TAR_IGNORE:
    bug("room_cast (%d): can't cast spell with tar_ignore (spell:%s) (target:%s)", 
	room->vnum,	
	spell_name,
	e->name);
    return Value((long)0);
  case TAR_CHAR_OFFENSIVE:
    bug("room_cast (%d): can't cast offensive spell (spell:%s) (target:%s)", 
	room->vnum,
	spell_name,
	e->name);
    return Value((long)0);
  case TAR_OBJ_INV: case TAR_OBJ_CHAR_OFF:
    bug("room_cast (%d): can't cast spell on items (spell:%s) (target:%s)",
	room->vnum,
	spell_name,
	e->name );
    return Value((long)0);
  default:
    bug("room_cast (%d): invalid target %d (spell:%s) (target:%s)",
	room->vnum,
	ability_table[sn].target,
	spell_name,
	e->name );
    return Value((long)0);
  }
  (*(SPELL_FUN*)ability_table[sn].action_fun) ( sn, level, (CHAR_DATA*)e, (void*)e, target, casting_level );
  return Value((long)0);
}

Value room_addAffect( ENTITY_DATA *entity, const int nparams, Value *params ) {
  entity_addAffect( entity, params );
  return Value((long)0);
}

Value room_savesSpell( ENTITY_DATA *entity, const int nparams, Value *params ) {
  return entity_savesSpell( entity, params );
}

Value room_affectStrip( ENTITY_DATA *entity, const int nparams, Value *params ) {
  entity_affectStrip( entity, params );
  return Value((long)0);
}


/*****************/
CLASS_DATA *default_mob_class; 
CLASS_DATA *default_obj_class;
// Added by SinaC 2003
CLASS_DATA *default_room_class;

/*********************************************************************/
void boot_pred_fct() {
  FCT_DATA* nf;

// Mob predef fcts
  // Modified by SinaC 2001
  CLASS_DATA * prgmob = new_prg();
  prgmob->name = "Mob";
  prgmob->parents = NULL;
  prgmob->parents_count = 0;
  default_mob_class = prgmob;
#define NEW_MOB_METHOD(nom,  nparams, parmn...)\
do {                                            \
  nf = new_fct();				\
  nf->predfct = mob_ ## nom;			\
  nf->name = #nom;				\
  char* args[] = {parmn};                       \
  for (int i = 0; i < nparams; i++ )            \
    nf->parmname[i] = str_dup(args[i]);         \
  nf->nbparm = nparams;				\
  nf->incoming = NULL;                          \
  hash_put_fct(nf,prgmob->methods);            \
} while (false)


  NEW_MOB_METHOD(isNPC, 0);
  NEW_MOB_METHOD(name, 0);
  NEW_MOB_METHOD(level, 0);
  NEW_MOB_METHOD(hit, 0);
  NEW_MOB_METHOD(silver, 0);
  NEW_MOB_METHOD(gold, 0);
  NEW_MOB_METHOD(sex, 0);
  NEW_MOB_METHOD(classes, 0);
  NEW_MOB_METHOD(addSilver, 1, "int amount");
  NEW_MOB_METHOD(addGold, 1, "int amount");
  NEW_MOB_METHOD(getObjCarried, 1, "string itemName");
  NEW_MOB_METHOD(addHit, 1, "int amount");
  NEW_MOB_METHOD(addMana, 1, "int amount");
  NEW_MOB_METHOD(addPsp, 1, "int amount");
  NEW_MOB_METHOD(inventory, 0);
  NEW_MOB_METHOD(vnum, 0);
  NEW_MOB_METHOD(carryNumber, 0);
  NEW_MOB_METHOD(carryWeight, 0);
  NEW_MOB_METHOD(canCarryN, 0);
  NEW_MOB_METHOD(canCarryW, 0);
  NEW_MOB_METHOD(oLoad,1, "int objVnum");
  NEW_MOB_METHOD(mLoad, 1, "int mobVnum");
  NEW_MOB_METHOD(canSee, 1, "obj/char/room target");
  NEW_MOB_METHOD(getAbility,1, "string abilityName");
  NEW_MOB_METHOD(checkImprove,3, "string abilityName", "int success", "int multiplier" );
  NEW_MOB_METHOD(isAwake, 0);
  NEW_MOB_METHOD(fighting, 0);
  NEW_MOB_METHOD(sendTo,1, "string msg");
  NEW_MOB_METHOD(isEquiv, 1, "char target");
  NEW_MOB_METHOD(canLoot, 1, "obj item");
  NEW_MOB_METHOD(alignment,0);
  NEW_MOB_METHOD(etho,0);
  NEW_MOB_METHOD(group,0);
  NEW_MOB_METHOD(isSafe, 1, "char target");
  NEW_MOB_METHOD(isWearing, 1, "obj item");
  NEW_MOB_METHOD(multiHit, 1, "char target" );
  NEW_MOB_METHOD(shortDescr,0);
  NEW_MOB_METHOD(damage, 3, "int amount", "string damType", "string dieMsg");
  NEW_MOB_METHOD(transfer, 1, "int roomVnum" );
  NEW_MOB_METHOD(echo, 1, "char msg");
  NEW_MOB_METHOD(isAffected, 1, "string abilityName");
  NEW_MOB_METHOD(addPractice, 1, "int amount");
  NEW_MOB_METHOD(destroy, 0);
  NEW_MOB_METHOD(getPet, 0);
  NEW_MOB_METHOD(setPet, 1, "char mob");
  NEW_MOB_METHOD(setName, 1, "string newName");
  NEW_MOB_METHOD(clan, 0);
  NEW_MOB_METHOD(carryLocation, 1, "string locName");
  NEW_MOB_METHOD(room, 0);
  NEW_MOB_METHOD(longDescr, 0);
  NEW_MOB_METHOD(position,0);
  NEW_MOB_METHOD(defaultPosition, 0);
  NEW_MOB_METHOD(equipment, 0);
  NEW_MOB_METHOD(suddenDeath, 1, "string deathMsg");
  NEW_MOB_METHOD(stopFighting, 0);
  NEW_MOB_METHOD(god, 0);
  NEW_MOB_METHOD(master, 0);
  NEW_MOB_METHOD(actTo,2,"string formatActLike", "list of entity/entity  relatedEntity" );
  NEW_MOB_METHOD(actRoom,-1,"string formatActLike", "list of entity/entity  relatedEntity" );
  NEW_MOB_METHOD(actAll,-1,"string formatActLike", "list of entity/entity  relatedEntity" );
  NEW_MOB_METHOD(getBaseAttr, 1, "string attrName");
  NEW_MOB_METHOD(getAttr, 1, "string attrName");
  NEW_MOB_METHOD(checkAttr, 2, "string attrName", "not defined attrValueToCheck" );
  NEW_MOB_METHOD(getAct,0);
  NEW_MOB_METHOD(setAct, 1, "list of string/string newActValue");
  NEW_MOB_METHOD(toggleAct, 1, "list of string/string newActValue");
  NEW_MOB_METHOD(checkAct, 1, "list of string/string actValueToCheck" );
  NEW_MOB_METHOD(getBaseAffect, 0);
  NEW_MOB_METHOD(getAffect, 0);
  NEW_MOB_METHOD(checkAffect, 1, "list of string/string affectValueToCheck");
  NEW_MOB_METHOD(aggrDamage, -1, "char aggressor", "int amount", "string damType", "[string formatActLike]", "[list of entity relatedEntity]" );
  NEW_MOB_METHOD(getFaction,1,"string factionName/all");
  NEW_MOB_METHOD(setFaction,-1,"string factionName/newFactionName", "int newValue" );
  NEW_MOB_METHOD(sayTo,2,"char target", "string phrase");
  NEW_MOB_METHOD(toggleFollower,1,"char target");
  NEW_MOB_METHOD(addAffect, 8, "string where", "string location", "string operator", "not defined modifier", "int duration", "int level", "string type", "int castinglevel" );
  NEW_MOB_METHOD(savesSpell, 2, "int level", "string damClass" );
  NEW_MOB_METHOD(isSafeSpell, 2, "char target", "bool area" );
  NEW_MOB_METHOD(abilityDamage, -1, "char aggressor", "int amount", "string damType", "int show", "int old", "string abilityName", "[string formatActLike]", "[list of entity relatedEntity]" );
  NEW_MOB_METHOD(affectStrip, 1, "string abilityName");
  NEW_MOB_METHOD(addDaze, 1, "int amount");
  NEW_MOB_METHOD(setStunned, 1, "int amount" );
  NEW_MOB_METHOD(getStunned, 0 );
  NEW_MOB_METHOD(getAttrINT, 1, "string attrName" );
  NEW_MOB_METHOD(hasShop, 0 );
  NEW_MOB_METHOD(cast, 4, "entity victim", "string spellName", "int level", "int castingLevel");
  NEW_MOB_METHOD(findPath, 3, "char victim", "int depth", "bool in_zone");
  NEW_MOB_METHOD(isImmortal, 0 );
  NEW_MOB_METHOD(description, 0 );
  NEW_MOB_METHOD(setDescription, 1, "string newDescription");
  NEW_MOB_METHOD(giveObj, 1, "entity objToGive" );

// Obj predef fcts
  // Modified by SinaC 2001
  CLASS_DATA * prgobj = new_prg();
  prgobj->name = "Obj";
  prgobj->parents = NULL;
  prgobj->parents_count = 0;
  default_obj_class = prgobj;
#define NEW_OBJ_METHOD(nom,  nparams, parmn...)\
do {                                            \
  nf = new_fct();				\
  nf->predfct = obj_ ## nom;			\
  nf->name = #nom;				\
  char* args[] = {parmn};                       \
  for (int i = 0; i < nparams; i++ )            \
    nf->parmname[i] = str_dup(args[i]);         \
  nf->nbparm = nparams;				\
  nf->incoming = NULL;                          \
  hash_put_fct(nf,prgobj->methods);            \
} while (false)


  NEW_OBJ_METHOD(name, 0);
  NEW_OBJ_METHOD(level, 0);
  NEW_OBJ_METHOD(shortDescr, 0);
  NEW_OBJ_METHOD(getCondition, 0);
  NEW_OBJ_METHOD(setCondition, 1, "int newCondition");
  NEW_OBJ_METHOD(cost, 0);
  NEW_OBJ_METHOD(carriedBy, 0);
  NEW_OBJ_METHOD(say, 1, "string msg");
  NEW_OBJ_METHOD(tell, 2, "char target", "string msg");
  NEW_OBJ_METHOD(vnum, 0);
  NEW_OBJ_METHOD(material, 0);
  NEW_OBJ_METHOD(itemType, 0);
  NEW_OBJ_METHOD(destroy, 0);
  NEW_OBJ_METHOD(getNumber, 0);  
  NEW_OBJ_METHOD(getWeight, 0);
  NEW_OBJ_METHOD(isEquiv, 1, "obj target");
  NEW_OBJ_METHOD(cast, 4, "char victim", "string spellName", "int level", "int castingLevel");
  NEW_OBJ_METHOD(echo, 1, "string echo");
  NEW_OBJ_METHOD(oLoad, 1, "int itemVnum");
  NEW_OBJ_METHOD(mLoad, 1, "int mobVnum");
  NEW_OBJ_METHOD(room, 0);
  NEW_OBJ_METHOD(description, 0);
  NEW_OBJ_METHOD(getTimer, 0);
  NEW_OBJ_METHOD(setTimer, 1, "int timer");
  NEW_OBJ_METHOD(size, 0);
  NEW_OBJ_METHOD(contains,0);
  NEW_OBJ_METHOD(getValue, 1, "int valueId");
  NEW_OBJ_METHOD(setValue, 2, "int valueId", "not defined newValue");
  NEW_OBJ_METHOD(toggleValue, 2, "int valueId", "not defined valueToToggle");
  NEW_OBJ_METHOD(checkValue, 2, "int valueId", "not define valueToCheck" );
  NEW_OBJ_METHOD(getExtraFlags, 0);
  NEW_OBJ_METHOD(setExtraFlags, 1, "list of string/string newExtraFlags");
  NEW_OBJ_METHOD(toggleExtraFlags, 1, "list of string/string valueToToggle");
  NEW_OBJ_METHOD(checkExtraFlags, 1, "list of string/string valueToCheck" );
  NEW_OBJ_METHOD(getWearFlags, 0);
  NEW_OBJ_METHOD(setWearFlags, 1, "list of string newWearFlags");
  NEW_OBJ_METHOD(toggleWearFlags, 1, "list of string/string valueToToggle");
  NEW_OBJ_METHOD(checkWearFlags, 1, "list of string/string valueToCheck" );
  NEW_OBJ_METHOD(actAll,-1,"string formatActLike", "list of entity/entity  relatedEntity" );
  NEW_OBJ_METHOD(setName,1,"string newName");
  NEW_OBJ_METHOD(setShortDescr,1,"string newShortDescr");
  NEW_OBJ_METHOD(setDescription,1,"string newDescription");
  NEW_OBJ_METHOD(addAffect, 8, "string where", "string location", "string operator", "not defined modifier", "int duration", "int level", "string type", "int castinglevel" );
  NEW_OBJ_METHOD(savesSpell, 2, "int level", "string damClass" );
  NEW_OBJ_METHOD(affectStrip,1,"string abilityName");
  NEW_OBJ_METHOD(setLevel,1,"int newLevel");


// Room predef fcts, added by SinaC 2003
  CLASS_DATA * prgroom = new_prg();
  prgroom->name = "Room";
  prgroom->parents = NULL;
  prgroom->parents_count = 0;
  default_room_class = prgroom;
#define NEW_ROOM_METHOD(nom,  nparams, parmn...)\
do {                                            \
  nf = new_fct();				\
  nf->predfct = room_ ## nom;			\
  nf->name = #nom;				\
  char* args[] = {parmn};                       \
  for (int i = 0; i < nparams; i++ )            \
    nf->parmname[i] = str_dup(args[i]);         \
  nf->nbparm = nparams;				\
  nf->incoming = NULL;                          \
  hash_put_fct(nf,prgroom->methods);            \
} while (false)

  NEW_ROOM_METHOD(name, 0);
  NEW_ROOM_METHOD(vnum, 0);
  NEW_ROOM_METHOD(echo, 1, "string msg");
  NEW_ROOM_METHOD(description, 0);
  NEW_ROOM_METHOD(getBaseAttr,1, "string attrName");
  NEW_ROOM_METHOD(getAttr,1, "string attrName");
  NEW_ROOM_METHOD(people,0);
  NEW_ROOM_METHOD(contents,0);
  NEW_ROOM_METHOD(owner,0);
  NEW_ROOM_METHOD(guild,0);
  NEW_ROOM_METHOD(clan,0);
  NEW_ROOM_METHOD(exits,0);
  NEW_ROOM_METHOD(roomDir,1, "string dirName");
  NEW_ROOM_METHOD(mLoad,1, "int mobVnum");
  NEW_ROOM_METHOD(oLoad,1, "int objVnum");
  NEW_ROOM_METHOD(linked,0);
  NEW_ROOM_METHOD(areaName,0);
  NEW_ROOM_METHOD(transfer,1);
  NEW_ROOM_METHOD(createDoor,2,"int toRoomVnum", "string dirName" );
  NEW_ROOM_METHOD(deleteDoor,1,"string dirName" );
  NEW_ROOM_METHOD(createOneWayDoor,2,"int toRoomVnum", "string dirName" );
  NEW_ROOM_METHOD(deleteOneWayDoor,1,"string dirName" );
  NEW_ROOM_METHOD(around,0);
  NEW_ROOM_METHOD(getExitFlags,1,"string dirName");
  NEW_ROOM_METHOD(setExitFlags,2,"string dirName", "list of string  newFlags");
  NEW_ROOM_METHOD(toggleExitFlags,2,"string dirName", "list of string  flagsToToggle");
  NEW_ROOM_METHOD(actAll,-1,"string formatActLike", "list of entity/entity  relatedEntity" );
  NEW_ROOM_METHOD(checkAttr,2, "string attrName", "not defined valueToCheck" );
  NEW_ROOM_METHOD(cast, 4, "char victim", "string spellName", "int level", "int castingLevel");
  NEW_ROOM_METHOD(addAffect, 8, "string where", "not defined location", "string operator", "not defined modifier", "int duration", "int level", "string type", "int castinglevel" );
  NEW_ROOM_METHOD(savesSpell, 2, "int level", "string damClass" );
  NEW_ROOM_METHOD(affectStrip,1,"string abilityName");

// Making predefined classes accessible by scripts
  hash_put_prog(prgobj);
  hash_put_prog(prgmob);
  // Added by SinaC 2003 for room program
  hash_put_prog(prgroom);

// Global predef fcts
#define NEW_GPF_METHOD(nom,  nparams)           \
  nf = new_fct();				\
  nf->predfct = gpf_ ## nom;			\
  nf->name = #nom;				\
  nf->nbparm = nparams;				\
  nf->incoming = NULL;                          \
  hash_put_glob(nf->name, nf);

  NEW_GPF_METHOD(chance, 1);
  NEW_GPF_METHOD(log, 1);
  NEW_GPF_METHOD(random, 1);
  NEW_GPF_METHOD(max, 2);
  NEW_GPF_METHOD(min, 2);
  NEW_GPF_METHOD(range, 3);
  NEW_GPF_METHOD(timeHour, 0);
  NEW_GPF_METHOD(timeDay, 0);
  NEW_GPF_METHOD(timeMonth, 0);
  NEW_GPF_METHOD(timeYear, 0);
  NEW_GPF_METHOD(dice, 2);
  NEW_GPF_METHOD(getRoom, 1);
  NEW_GPF_METHOD(getCharList, 0);
  NEW_GPF_METHOD(getObjList, 0);
  NEW_GPF_METHOD(getRoomList, 0);
  NEW_GPF_METHOD(dump, 1);
  NEW_GPF_METHOD(act,3);
  NEW_GPF_METHOD(saveAreaState,0);
}
