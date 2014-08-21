#include <time.h>  // added by SinaC 2003
#include <stdio.h>
#include "merc.h"
#include "db.h"
#include <string.h>
#include "names.h" // added by SinaC 2003 for script_type_name( type )
#include "config.h"
#include "error.hh"


Value:: Value(const char * s) {
  typ = SVT_STR;
  val.s = str_dup(s);
}

//Value:: ~Value() {
//}

bool valueEquiv( const Value& a, const Value& b ) {
  Value tmpa = a.get(), 
    tmpb = b.get();
  if ( tmpa.typ != tmpb.typ )
    return FALSE;
  switch ( tmpa.typ ) {
  case SVT_INT:
    return tmpa.val.i == tmpb.val.i;
  case SVT_STR:
    return !str_cmp( tmpa.asStr(), tmpb.asStr() ); // different from == case insensitive
  case SVT_ENTITY:
    if ( tmpa.val.e->kind != tmpb.val.e->kind )
      return FALSE;
    if ( tmpa.val.e->kind == CHAR_ENTITY ) {
      CHAR_DATA *chara = (CHAR_DATA*) tmpa.val.e;
      CHAR_DATA *charb = (CHAR_DATA*) tmpb.val.e;
      return chara->pIndexData == charb->pIndexData;
    }
    else if ( tmpa.val.e->kind == OBJ_ENTITY ){
      OBJ_DATA *obja = (OBJ_DATA*) tmpa.val.e;
      OBJ_DATA *objb = (OBJ_DATA*) tmpb.val.e;

      return obja->pIndexData == objb->pIndexData
	&& !str_cmp(obja->short_descr,
		    objb->short_descr);
    }
    // Added by SinaC 2003 for room program
    else if ( tmpa.val.e->kind == ROOM_ENTITY ){
      ROOM_INDEX_DATA *rooma = (ROOM_INDEX_DATA*) tmpa.val.e;
      ROOM_INDEX_DATA *roomb = (ROOM_INDEX_DATA*) tmpb.val.e;

      // Should this test be rooma->vnum == roomb->vnum ??
      return rooma == roomb;
    }
    else
      return FALSE;
  default:
    return FALSE;
  }
  return FALSE;
}

int operator==(const Value& a,  const Value& b) {
  Value tmpa = a.get(), 
    tmpb = b.get();

  // SinaC 2003, special case if one is a NULL entity
  //  integer 0 is equal to NULL
  //  string NULL or empty string is equal to NULL
  //  empty list is equal to NULL
  //  null entity is equal to NULL
  // -> other case not equal
  if ( tmpa.typ == SVT_ENTITY && ( tmpa.val.e == NULL || !tmpa.val.e->valid ) ) { // 1st value is NULL entity
    if ( tmpb.typ == SVT_INT && tmpb.val.i == 0 ) // integer
      return 1;
    else if ( tmpb.typ == SVT_STR && ( tmpb.val.s == NULL || tmpb.val.s[0] == '\0' ) ) // string
      return 1;
    else if ( tmpb.typ == SVT_LIST && tmpb.val.l->size == 0 ) // list
      return 1;
    else if ( tmpb.typ == SVT_ENTITY && ( tmpb.val.e == NULL || !tmpb.val.e->valid ) ) // entity
      return 1;
    return 0; // var/fct/void -> not equal
  }
  else if ( tmpb.typ == SVT_ENTITY && ( tmpb.val.e == NULL || !tmpb.val.e->valid ) ) { // 2nd value is NULL entity
    if ( tmpa.typ == SVT_INT && tmpa.val.i == 0 ) // integer
      return 1;
    else if ( tmpa.typ == SVT_STR && ( tmpa.val.s == NULL || tmpa.val.s[0] == '\0' ) ) // string
      return 1;
    else if ( tmpa.typ == SVT_LIST && tmpa.val.l->size == 0 ) // list
      return 1;
    else if ( tmpa.typ == SVT_ENTITY && ( tmpa.val.e == NULL || !tmpa.val.e->valid ) ) // entity
      return 1;
    return 0; // var/fct/void -> not equal
  }

  if ( tmpa.typ != tmpb.typ ) {
    p_error("Trying to compare differently typed values (typ %s[%s] | typ %s[%s])",
	    script_type_name( tmpa.typ ), tmpa.asStr(),
	    script_type_name( tmpb.typ ), tmpb.asStr() );
  }

  switch ( tmpa.typ ) {
  case SVT_INT:
    return tmpa.val.i == tmpb.val.i;
  case SVT_STR:
    return !strcmp( tmpa.asStr(), tmpb.asStr() );
  case SVT_ENTITY: {
    //return tmpa.val.e == tmpb.val.e; SinaC 2003
    ENTITY_DATA *e1 = tmpa.val.e;
    ENTITY_DATA *e2 = tmpb.val.e;
    if ( ( e1 == NULL || !e1->valid ) // if both are NULL -> true
	 && ( e2 == NULL || !e2->valid ) )
      return 1;
    if ( e1 == NULL || !e1->valid
	 || e2 == NULL || !e2->valid ) // only one is NULL -> false
      return 0;
    return e1 == e2;
  }
  default:
    p_error("Trying to compare unsupported types (typ %s | typ %s)",
	    script_type_name( tmpa.typ ), script_type_name( tmpb.typ ) );
  }
}

int operator<(const Value& a,  const Value& b) {
  return a.asInt() < b.asInt();
}
int operator>(const Value& a,  const Value& b) {
  return a.asInt() > b.asInt();
}
int operator!=(const Value& a,  const Value& b) {
  return !( a == b );
}
int operator<=(const Value& a,  const Value& b) {
  return a.asInt() <= b.asInt();
}
int operator>=(const Value& a,  const Value& b) {
  return a.asInt() >= b.asInt();
}

Value operator+(const Value& a, const Value& b) {
  const Value& a0 = a.get();
  const Value& b0 = b.get();

  if ( a0.typ == SVT_STR ) {
    char* buf = (char*) GC_MALLOC_ATOMIC(strlen(a0.val.s) + strlen(b0.asStr()) + 1);
    strcpy(buf, a0.val.s);
    strcat(buf, b0.asStr());
    Value res = Value(buf);
    return res;
  }

  if (a0.typ != b0.typ) 
    p_error("Operator + must be applied to homogenous values %s (typ %s) and %s (typ %s)", 
	    a0.asStr(), script_type_name( a0.typ ), 
	    b0.asStr(), script_type_name( b0.typ ) );
  switch (a0.typ) {
  case SVT_INT:
    return a0.val.i + b0.val.i;
    break;
  case SVT_LIST: {
    ValueList* vl = ValueList::newList(a0.val.l->size + b0.val.l->size);
    memcpy(vl->elems,                  a0.val.l->elems, sizeof(Value) * a0.val.l->size);
    memcpy(vl->elems + a0.val.l->size, b0.val.l->elems, sizeof(Value) * b0.val.l->size);
    return vl;
    break;
  }
  default:
    p_error("Operator + cannot be applied to this type of values (only int, string and list) (typ %s)",
	    script_type_name( a0.typ ));
  }
}
Value operator-(const Value& a, const Value& b) {
  return a.asInt() - b.asInt();
}

Value operator*(const Value& a, const Value& b) {
  return a.asInt() * b.asInt();
}

Value operator/(const Value& a, const Value& b) {
  return a.asInt() / b.asInt();
}

Value operator!(const Value& r) {
  return r.asInt() == 0;
}

Value operator-(const Value& r) {
  return -r.asInt();
}


Value* Scope::newVar(const char* n) {
  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("new variable: %s   (%d)", n, numvar+1 );
  }

  //ASSERT(numvar < MAX_SCOPE_VARS - 1, "scope layer has too much variables");
  if ( numvar >= MAX_SCOPE_VARS-1 )
    p_error("Scope layer has too much variables: %d [max: %d]", numvar, MAX_SCOPE_VARS );

  if ( getVar(n) != NULL )
    p_error("Variable %s has already been declared in this scope.", n );

  Value* res = new Value((long int)0);
  val[numvar] = res;
  name[numvar] = n;
  numvar++;
  return res;
}

Value* Scope::getVar(const char* n) {
  for (int i = 0; i<numvar; i++) {
    if (!strcmp(n, name[i]))
      return val[i];
  }
  return NULL;
}

void Scope::clean() {
  for (int i =0; i<numvar; i++) {
    if ( SCRIPT_VERBOSE > 6 ) {
      log_stringf("freeing scope %d) '%s' value '%s'", i, name[i], val[i]->asStr());
    }
  }
  numvar = 0;
}

void Value::setValue(const Value& v) {
  if (typ == SVT_VAR) 
    *(val.v) = v.get();
  else
    log_stringf("Trying to set constant! :(");
}

ENTITY_DATA* Value::asEntity() const {
  Value tmp = get();
  if ( tmp.typ != SVT_ENTITY )
    p_error("Failed to convert into an entity (%s)", tmp.asStr() );
  if ( tmp.val.e == NULL || !tmp.val.e->valid )
    return NULL;
  return tmp.val.e;
}

char *computeListString(ValueList *l) {
  char *buf = new char [4096];
  int start = 0;
  strcpy(buf,"[");
  if ( l->size > 25 ) {
    strcat(buf,"...");
    start = l->size-25;
  }
  for ( int i = start; i < l->size; i++ ) {
    strcat(buf, l->elems[i].asStr());
    if ( i < l->size-1 )
      strcat(buf,",");
  }
  strcat(buf,"]");
  return buf;
}

const char* Value::asStr() const {
  Value tmp = get();
  // Modified by SinaC 2003, size was 1024 before
  static char buf[4096];

  switch(tmp.typ) {
  case SVT_STR:
    return tmp.val.s;
  case SVT_INT:
    sprintf( buf, "%ld", tmp.val.i );
    return buf;
  case SVT_ENTITY:
    // Added by SinaC 2001, valid: SinaC 2003
    if ( tmp.val.e == NULL || !tmp.val.e->valid )
      sprintf(buf,"NULL");
    else
      sprintf(buf, "'%s'", tmp.val.e->name );
    return buf;
  case SVT_LIST:
    return computeListString(tmp.val.l);
  default:
    sprintf( buf, "<typ=%d>", tmp.typ );
    return buf;
  }
};

long Value::asInt() const {
  Value tmp = get();
  if ( tmp.typ != SVT_INT ) {
    sprintf(log_buf, "Trying to cast into int a non int (%s typ:%s)", tmp.asStr(), script_type_name(tmp.typ) );
    p_error(log_buf);
  }
  return tmp.val.i;
}

ValueList* Value::asList() const {
  Value tmp = get();

  switch (tmp.typ) {
  case SVT_LIST:
    return tmp.val.l;

    /* a good idea ?
  // if it's a string we wordize it
  case SVT_STR:
    return wordize(tmp.val.s).val.l;
    */
  default:
    p_error("Failed to convert into a list (%s)", tmp.asStr() );
  }
}

void Value::explicit_free() { // SinaC 2003
  //if ( typ == SVT_LIST )
  //  val.l->explicit_free();
  if ( typ == SVT_LIST )
    GC_FREE( val.l );
}

void ValueList::explicit_free() { // SinaC 2003
  //for ( int i = 0; i < size; i++ )
  //  elems[i].explicit_free();
  //GC_FREE(elems); //--> FIXME some GC_MALLOC are GC_malloc_debug, others are GC_malloc
  GC_FREE(this);
}

ValueList* ValueList::newList(int size) {
  //ValueList* vl = (ValueList*) GC_MALLOC(size*sizeof(Value) + sizeof(int));
  ValueList* vl = (ValueList*) GC_MALLOC(size*sizeof(Value) + sizeof(ValueList));
  vl->size = size;
  memset(vl->elems, 0, size*sizeof(Value));
  return vl;
}

ValueList* ValueList::emptyList() {
  return newList(0);
}

bool ValueList::includes(const ValueList* vl) const {
  for (int i= 0; i<vl->size; i++) {
    bool found = false;
    for (int j = 0; j<size; j++) {
      if (vl->elems[i] == elems[j]) {
	found = true;
	break;
      }
    }
    if (!found)
      return false;
  }
  return true;
}

bool ValueList::contains(const Value& v) const{
  for (int i = 0; i < size; i++) {
    if (elems[i] == v)
      return true;
  }
  return false;
}

Value ValueList::get(const int n) const{
  if ( n >= size || n < 0 )
    p_error("Invalid parameter [%d] for get (>= size[%d] or < 0)", n, this->size );
  return Value(elems[n]);
}

Value ValueList::unique() const {
  ValueList *tmp = ValueList::newList(size);
  int cur = 0;
  for ( int i = 0; i < size; i++ ) {
    bool exist = FALSE;
    for ( int j = 0; j < cur && !exist; j++ ) {
      if ( tmp->elems[j] == elems[i] )
	exist = TRUE;
    }
    if (!exist)
      tmp->elems[cur++] = elems[i];
  }

  ValueList *l = ValueList::newList(cur);

  memcpy( l->elems, tmp->elems, sizeof(Value) * cur );

  return l;
}

Value ValueList::uniqueEquiv() const {
  ValueList *tmp = ValueList::newList(size);
  int cur = 0;
  for ( int i = 0; i < size; i++ ) {
    bool exist = FALSE;
    for ( int j = 0; j < cur && !exist; j++ ) {
      if ( valueEquiv(tmp->elems[j],elems[i] ) )
	exist = TRUE;
    }
    if (!exist)
      tmp->elems[cur++] = elems[i];
  }

  ValueList *l = ValueList::newList(cur);

  memcpy( l->elems, tmp->elems, sizeof(Value) * cur );

  return l;
}

// Added by SinaC 2003, returns index(position) of param in list
int ValueList::index(const Value& v) const{
  for (int i = 0; i < size; i++) {
    if (elems[i] == v)
      return i;
  }
  return -1;
}

// Added by SinaC 2003 to flat a list
// count number of elements in a list
int countListElem( const ValueList *l ) {
  //  log_stringf("countListElem: [size: %d]", l->size );
  int count = 0;
  for ( int i = 0; i < l->size; i++ )
    if ( l->elems[i].typ == SVT_LIST )
      count += countListElem(l->elems[i].asList());
  else
    count++;
  return count;
}
ValueList *flatPointer;
int flatIndex = 0;
void flatList( const Value &v ) {
  //  log_stringf("v asStr: %s   [type %d]", v.asStr(), v.typ );
  if ( v.typ == SVT_LIST ) { // if v is a list: recursive call
    ValueList *l = v.asList();
    for ( int i = 0; i < l->size; i++ )
      flatList( l->elems[i] );
  }
  else // if v is not a list: add elem
    flatPointer->elems[flatIndex++] = v;
}
Value ValueList::flat() const {
  int count = countListElem( this );

  //  log_stringf("count: %d", count );

  if ( count == 0 )
    return ValueList::emptyList();

  ValueList *l = ValueList::newList(count);
  flatPointer = l;
  flatIndex = 0;
  for ( int i = 0; i < this->size; i++ )
    flatList( this->elems[i] );

  return l;
}

FCT_DATA* Value::asFct() const {
  Value tmp = get();
  if ( tmp.typ != SVT_FCT ) {
    p_error("Trying to cast into a FCT (non fct)");
  }
  return tmp.val.f;
}


Value Context::newVar(const char* name) {
  Value* ref = scopes[numscope-1].newVar(name);
  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("Context:newVar  What's the ref? %x", (int)ref);
  }
  return Value(ref);
}

//////////////////////////// other utility functions //////////////////

Value wordize(const char* input, const char* delim = " " ) {
  char s1[MAX_INPUT_LENGTH];
  char *s;
  char delimC;

  if (input[0] == '\0')
    return ValueList::emptyList();  

  delimC = delim[0];

  strcpy(s1, input);
  
  int nwords = 0;
  //count words
  for (s=s1+1; *s; s++) {
    //if (*s == ' ' && *(s-1) != ' ')
    if (*s == delimC && *(s-1) != delimC )
      nwords++;
  }

  //if (*(s-1) != ' ')
  if (*(s-1) != delimC )
    nwords++;

  if (nwords == 0)
    return ValueList::emptyList();  

  ValueList* vl = ValueList::newList(nwords);

  //  if ( nwords == 1 ) {
  //    vl->elems[0] = input;
  //    return vl;
  //  }

  s = s1;

  char* tok;
  //tok = strsep(&s, " ");
  tok = strsep(&s, delim );
  int i = 0;
  while ( tok != NULL ) {
    if ( tok[0] != '\0' )
      vl->elems[i++] = str_dup(tok);
    //tok = strsep(&s, " ");
    tok = strsep(&s, delim );
  }
  ASSERT(i == nwords, "number of words must match" );

  return vl;
}


//////////////////// context


void Context::pushscope() {
  numscope++;

  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("pushscope: %d", numscope);
  }

  //ASSERT(numscope < MAX_SCOPES, "scope depth overloaded");
  if ( numscope >= MAX_SCOPES )
    p_error("Scope depth overloaded: %d [max= %d]", numscope, MAX_SCOPES );
}

void Context::popscope() {    
  //ASSERT(numscope > 0, "scope depth underloaded");
  if ( numscope <= 0 )
    p_error("Scope depth underloaded" );

  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("popscope: %d", numscope);
  }

  numscope--;
  scopes[numscope].clean();
}
