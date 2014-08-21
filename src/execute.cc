#include <time.h>  // added by SinaC 2003
#include <string.h>
#include "merc.h"
#include "script2.h"
#include "cxx/struct_skel.hh"
#include "grammar.hh"
#include "assert.h"
#include "scrhash.h"
#include "db.h"
#include "scrvalue.h"
#include "interp.h"
#include "names.h"
#include "config.h"
#include "error.hh"
#include "utils.h"


#define TRACE log_stringf\
("EXEC TRACE %s (%d)", __PRETTY_FUNCTION__, __LINE__);

/////////////////////

bool innerFindAscendancy( CLASS_DATA *c, CLASS_DATA *cla ) {
  while ( c != NULL ) {
    if ( c == cla )
      return TRUE;
    if ( c->parents_count == 0 )
      return FALSE;
    else if ( c->parents_count == 1 )
      c = c->parents[0];
    else
      for ( int i = 0; i < c->parents_count; i++ )
	if ( innerFindAscendancy( c->parents[i], cla ) )
	  return TRUE;
  }
  return FALSE;
}

bool findAscendancy( ENTITY_DATA *e, const char *claName ) {
  CLASS_DATA *cla = silent_hash_get_prog( claName );
  if ( claName == NULL )
    p_error("findAscendancy: Invalid class name: %s.", claName );
  return innerFindAscendancy( e->clazz, cla );
}

//////////////////////

bool find_in_context (Value& res, const Value& enclosing, const char* field) {
  
  ENTITY_DATA* entity =enclosing.asEntity();
  Value* slot = get_extra_field(entity, field);
  if (slot == NULL) {
    return false;
  } else {
    res = Value(slot);
    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
    }
    return true;
  }
}

void Instruction::execute(Context& ctx) const {
  printf("do NOTHING; whoops\n");
}

void Compound::execute(Context& ctx) const {
  ctx.pushscope();
  for (int i=0; i<instr_count; i++) {
    instr[i]->execute(ctx);
  }
  ctx.popscope();
}


const char* Command::eval(Context& ctx) const{
  char buffer[4096];
  buffer[0] = 0;

  for (int i=0; i<parms_count; i++) {
    strcat(buffer, parms[i]->verb);
    strcat(buffer, parms[i]->expr->eval(ctx).asStr());
  }

  strcat(buffer, verb);

  return str_dup(buffer);
}

void Command::execute(Context& ctx) const {
  const char* c = eval(ctx);
  ::execute(ctx, ctx["this"]->asEntity(), c);
}

void ForceInstr::execute(Context& ctx) const {
  const char* c = content->eval(ctx);
  ::execute(ctx, ch->eval(ctx).asEntity(), c);
}

void IfInstr::execute(Context& ctx) const {
  if (cond->eval(ctx).asInt() != 0) {
    thenpart->execute(ctx);
  } else {
    if (elsepart)
      elsepart->execute(ctx);
  }
}

void ObjvarDecl::execute(Context& ctx) const {
  Value obj = object->eval(ctx);

  Value* isvar = get_extra_field(obj.asEntity(), ident->image);
  Value newvar = add_extra_field(obj, ident->image);

  if (isvar == NULL) {
    newvar.setValue(defval->eval(ctx));
  }
}
// SinaC 2003
void DelvarDecl::execute(Context& ctx) const {
  Value obj = object->eval(ctx);

  if ( !del_extra_field( obj.asEntity(), ident->image ) )
    bug("Trying to delete inexistant objvar: %s  on entity: %s",
	ident->image, obj.asStr() );
}
// SinaC 2003
void LifetimevarDecl::execute(Context& ctx) const {
  Value obj = object->eval(ctx);

  FIELD_DATA *f = get_field_data( obj.asEntity(), ident->image );
  if ( f != NULL )
    f->lifetime = lifetime->eval(ctx).asInt();
  else
    bug("Trying to set lifetime of inexistant objvar: %s  on entity: %s",
	ident->image, obj.asStr() );
}

void VarDecl::execute(Context& ctx) const {

 Value newvar = ctx.newVar(ident->image);
 if (defval != NULL)
   newvar.setValue(defval->eval(ctx));
}

#define MAX_LOOP (10000)
void WhileInstr::execute(Context& ctx) const {
  int loopCount = 0;
  while (cond->eval(ctx).asInt() != 0) {
    if ( loopCount > MAX_LOOP )
      p_error("Max loop iteration exceeded, %d", MAX_LOOP );
    
    body->execute(ctx);

    loopCount++;
  }
}

void ExprInstr::execute(Context& ctx) const {
  expr->eval(ctx);
}

Value ConsList::eval(Context& ctx) const {
  if (elems_count == 0) {
    return ValueList::emptyList();
  }
  ValueList* l = ValueList::newList(elems_count);
  for (int i = 0; i<elems_count; i++)
    l->elems[i] = elems[i]->eval(ctx).get(); // .get()  added by SinaC 2003
  return l;
}

Value Expression::eval(Context& ctx) const {
  ASSERT(false, "Eval is not to be called on abstract expressions");
}

Value AnyListExpr::eval(Context& ctx) const {
  // 1. Evaluate input list
  // 2. for each element:
  // 21. declare the bound variable with the element as value
  // 22. evaluate the condition, 
  // 221. if true, insert index in matching vector
  // 3. return NULL if no match found, else return a random matching elements

  ValueList* il = inputList->eval(ctx).asList();

  // create vector storing index of matching elements
  int *matching = new int [il->size];
  int count_matching = 0;
  
  ctx.pushscope();
  // declare and set the value.
  Value newvar = ctx.newVar(variable->image);
  for (int i = 0; i < il->size; i++) {
    newvar.setValue(il->elems[i]);

    if (condition == NULL || condition->eval(ctx).asInt() != 0) {
      matching[count_matching] = i;
      count_matching++;
    }
  }
  ctx.popscope();

  if ( count_matching <= 0 ) // no matching elements
    return Value((ENTITY_DATA *)NULL);
  else
    return il->elems[matching[number_range(0,count_matching-1)]];
}

Value FirstListExpr::eval(Context& ctx) const {
  // 1. Evaluate input list
  // 2. for each element:
  // 21. declare the bound variable with the element as value
  // 22. evaluate the condition, if true stop the loop and return value
  // 3. return NULL if no match found, else return matching elements

  ValueList* il = inputList->eval(ctx).asList();

  ctx.pushscope();
  int found = -1;
  // declare and set the value.
  Value newvar = ctx.newVar(variable->image);
  for (int i = 0; i < il->size; i++) {
    newvar.setValue(il->elems[i]);
    if (condition == NULL || condition->eval(ctx).asInt() != 0) {
      found = i;
      break;
    }
  }
  ctx.popscope();

  if ( found == -1 )
    return Value((ENTITY_DATA *)NULL);
  else
    return il->elems[found];
}

Value ComprehensionExpr::eval(Context& ctx) const {
  // 1. Evaluate input list
  // 2. for each element:
  // 21. declare the bound variable with the element as value
  // 22. evaluate the condition, 
  // 221. if true, evaluate the generator and append to the result.
  // 3. return!

  ValueList* il = inputList->eval(ctx).asList();

  ValueList* result = ValueList::newList(il->size);
  int count_result = 0;
  
  ctx.pushscope();
  // declare and set the value.
  Value newvar = ctx.newVar(variable->image);
  for (int i = 0; i < il->size; i++) {
    newvar.setValue(il->elems[i]);

    if (condition == NULL || condition->eval(ctx).asInt() != 0) {
      Value newElem = generator->eval(ctx).get();
      result->elems[count_result] = newElem;
      count_result ++;
    }
  }
  ctx.popscope();

  result->size = count_result;

  return result;
}

Value PartialList::eval(Context& ctx) const {
  // 1. Evaluate input list
  // 2. Evalute lower/upper bound
  // 21. Test lower/upper bound validity
  // 3. Create the new list: [ lowerBound .. upperBound ]
  // 4. return!

  ValueList* il = inputList->eval(ctx).asList();
  int lb;
  if ( lowerBound )  lb = lowerBound->eval(ctx).asInt();
  else               lb = 0;
  int ub;
  if ( upperBound )  ub = upperBound->eval(ctx).asInt()+1; // list starts at 0
  else               ub = il->size;
  if ( lb < 0 || lb > il->size ) {
    p_warn("partialList: invalid lower bound [%d] list size [%d].",
	   lb, il->size );
    lb = URANGE( 0, lb, il->size );
  }
  if ( ub < 0 || ub > il->size ) {
    p_warn("partialList: invalid upper bound [%d] list size [%d].",
	   ub, il->size );
    ub = URANGE( 0, ub, il->size );
  }
  if ( lb > ub ) {
    p_warn("partialList: lower bound [%d] greater than upper bound [%d], taking whole list.",
	   lb, ub );
    lb = 0;
    ub = il->size;
  }

  int newSize = ub - lb;
  ValueList* result = ValueList::newList(newSize);

  for (int i = lb; i < ub; i++ )
    result->elems[i-lb] = il->elems[i];

  return result;
}

Value UnaryExpr::eval(Context& ctx) const {
  switch (op) {
  case '!':
    return ! expr->eval(ctx);
  case '-':
    return - expr->eval(ctx);
  default:
    ASSERT(false, "unhandled unary operator");
  }
}

Value NullLiteral::eval(Context& ctx) const {
  return Value((ENTITY_DATA*) NULL);
}

Value StringLiteral::eval(Context& ctx) const {
    return Value(image);
}

Value IntegerLiteral::eval(Context& ctx) const {
    return Value(value);
}


Value Identifier::eval(Context& ctx) const {
  Value* slot = ctx[image];
  Value res;
  if (slot == NULL) {
    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
      log_stringf("--> looking in THIS");
    }
    bool found = find_in_context(res, ctx["this"], image);

    if (!found) {
      p_error("undeclared variable: %s", image);
      res = Value((long) 0);
      if ( SCRIPT_VERBOSE > 6 ) {
	TRACE;
      }
    }
  } else {

    res = Value(slot);
    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
    }
  }
  return res;
}

Value OtherBinaryExpr::eval(Context& ctx) const {
  switch(op) {
  case IS: {
    Value l = left->eval(ctx).get();
    if ( l.typ != SVT_ENTITY )
      p_error("Operator IS has to be used on entity, %s(%s).", l.asStr(), script_type_name(l.typ));
    return findAscendancy( l.asEntity(), right->image );
    break;
  }
  case HAS: {
    Value l = left->eval(ctx).get();
    if ( l.typ != SVT_ENTITY )
      p_error("Operator HAS has to be used on entity, %s(%s).", l.asStr(), script_type_name(l.typ));
    return ( get_extra_field( l.asEntity(), right->image ) != NULL );
    break;
  }
  default:
    ASSERT(false, "parsed unknown operator");
  }
}

Value BinaryExpr::eval(Context& ctx) const {

  switch(op) {
  case '[':
    return left->eval(ctx).asList()->get(right->eval(ctx).asInt());
  case '+':
    return left->eval(ctx) + right->eval(ctx);
  case '-':
    return left->eval(ctx) - right->eval(ctx);
  case '*':
    return left->eval(ctx) * right->eval(ctx);
  case '/':
    return left->eval(ctx) / right->eval(ctx);
  case AND:
    return left->eval(ctx).asInt() && right->eval(ctx).asInt();
  case OR:
    return left->eval(ctx).asInt() || right->eval(ctx).asInt();
  case '<':
    return left->eval(ctx) < right->eval(ctx);
  case '>':
    return left->eval(ctx) > right->eval(ctx);
  case LE:
    return left->eval(ctx) <= right->eval(ctx);
  case GE:
    return left->eval(ctx) >= right->eval(ctx);
  case EQ:
    return left->eval(ctx) == right->eval(ctx);
  case NE:
    return left->eval(ctx) != right->eval(ctx);
  case IN:
    return right->eval(ctx).asList()->contains(left->eval(ctx));
    break;
  case EQUIV:
    return valueEquiv( left->eval(ctx), right->eval(ctx) );
    break;
  case '=': {
    Value res = left->eval(ctx);
    res.setValue(right->eval(ctx));
    return res;
    break;
  }
  default:
    ASSERT(false, "parsed unknown operator");
  }
}

FCT_DATA *innerGetMethod( CLASS_DATA *cl, const char *methodname ) {
  while (cl != NULL ) {
    if ( SCRIPT_VERBOSE > 6 ) {
      log_stringf("looking for %s in %s", methodname, cl->name );
    }
    FCT_DATA *f = hash_get_fct( methodname, cl->methods );
    if ( f != NULL )
      return f;
    if ( SCRIPT_VERBOSE > 6 ) {
      log_stringf(" #parent: %d", cl->parents_count );
    }
    //cl = cl->parent;
    // Modified for multiple inheritance, SinaC 2003
    if ( cl->parents_count == 0 )
      cl = NULL;
    else if ( cl->parents_count == 1 )
      cl = cl->parents[0];
    else {
      for ( int i = 0; i < cl->parents_count; i++ ) {
	if ( SCRIPT_VERBOSE > 6 ) {
	  log_stringf(" calling with parent [%d] %s", i, cl->parents[i]->name );
	}
	f = innerGetMethod(cl->parents[i], methodname );
	if ( f != NULL )
	  return f;
      }
      cl = NULL;
    }
  }
  return NULL;
}

FCT_DATA *getMethod( ENTITY_DATA* entity, const char *methodname ) {
  if ( entity == NULL )
    p_error("Trying to get a method [%s] from a NULL entity.", methodname );

  CLASS_DATA* cl = entity->clazz;

  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("searching %s for %s", entity->name, methodname );
  }

  return innerGetMethod(cl, methodname);
}

Value CallExpr::eval(Context& ctx) const {

  Value parms[params_count];
  for (int i = 0; i<params_count; i++)
    parms[i] = params[i]->eval(ctx);

  Value* actor = ctx["this"];
  if ( actor != NULL ) {
    FCT_DATA* f = getMethod(actor->asEntity(), left->image);
    if ( f != NULL )
      return runhandler(f, *actor, parms, params_count);
  }

  Value* tmp = hash_get_glob(left->image);
  if (tmp != NULL) {
    FCT_DATA* f= tmp->asFct();

    // ---- Modified by SinaC 2003, subject = "this" even in global predefined functions ----
    //Value subject((ENTITY_DATA*) NULL);
    //return runhandler(f, subject, parms, params_count);
    return runhandler(f, *actor, parms, params_count);
  } 
  else
    p_error("Unknown function or method: %s", left->image);
  return Value((long)0);
}

Value MethodCallExpr::eval(Context& ctx) const {

  Value leftval = left->eval(ctx).get();
  const char* rightval = ident->image;

  Value parms[params_count];
  for (int i = 0; i<params_count; i++)
    parms[i] = params[i]->eval(ctx);



  switch(leftval.typ) {
    
    // methods on list: size(), includes(list), get(n), unique(), uniqueEquiv(), index(x), random(), flat()
  case SVT_LIST:
    if (!strcmp(rightval, "size")) {
      return Value(leftval.asList()->size);
    } 
    else if (!strcmp(rightval, "includes")) {
      if (params_count != 1)
	p_error("includes() expects one parameter");
      return leftval.asList()->includes(parms[0].asList());
    } 
    else if (!strcmp(rightval, "get")) {
      if (params_count != 1)
	p_error("get() expects one parameter");
      return leftval.asList()->get(parms[0].asInt());
    }
    else if (!strcmp(rightval, "unique")) {
      return leftval.asList()->unique();
    }
    else if (!strcmp(rightval, "uniqueEquiv")) {
      return leftval.asList()->uniqueEquiv();
    }
    // Added by SinaC 2003, returns index(position) of param in list
    else if (!strcmp(rightval, "index")) {
      if (params_count != 1)
	p_error("index expects one parameter");
      return leftval.asList()->index(parms[0]);
    }
    // Added by SinaC 2003, returns a random element from list
    else if (!strcmp(rightval, "random")) {
      if (params_count != 0)
	p_error("random expects no parameter");
      ValueList *l = leftval.asList();
      if ( l->size == 0 )
	return Value((ENTITY_DATA*)NULL); // return null because we don't know list type
      return Value(l->elems[number_range( 0, l->size-1)]);
    }
    // Added by SinaC 2003, transforms a list of list into a list
    else if (!strcmp(rightval, "flat")) {
      if (params_count != 0)
	p_error("random expects no parameter");
      return leftval.asList()->flat();
    }
    break;
    // methods on string: size(), words(), asInt(), pad(n), startsWith(string), tokenize(c)
  case SVT_STR:
    if (!strcmp(rightval, "size")) {
      return Value(strlen(leftval.asStr()));
    }
    else if (!strcmp(rightval, "words")) {
      return wordize(leftval.asStr());
    } 
    else if (!strcmp(rightval,"asInt")){
      if (is_number(leftval.asStr()))
	return Value(atoi(leftval.asStr()));
      else
	return Value((long)-1);
    }
    else if (!strcmp(rightval,"startsWith")){
      if (params_count != 1 ) 
	p_error("startsWith expects one parameter");
      //log_stringf("%s %s", leftval.asStr(), parms[0].asStr() );
      return !str_prefix(parms[0].asStr(),leftval.asStr());
    }
    else if (!strcmp(rightval,"pad")){
      if (params_count != 1 ) 
	p_error("pad() expects one parameter");

      char format[64];
      char formatted[64];
      sprintf(format,"%%%lds", parms[0].asInt());
      sprintf(formatted,format,leftval.asStr());
      return Value(formatted);
    }
    else if (!strcmp(rightval,"tokenize") ) {
      if (params_count != 1)
	p_error("tokenize expects one parameter");
      return wordize(leftval.asStr(), parms[0].asStr());
    }
    break;
    // methods on int: asStr(), pad(n)
  case SVT_INT:
    if (!strcmp(rightval, "asStr")) {
      char s[512];
      sprintf(s, "%ld", leftval.asInt());
      return Value(s);
    }
    else if (!strcmp(rightval,"pad")){
      if (params_count != 1 ) 
	p_error("pad() expects one parameter");

      char format[64];
      char formatted[64];
      sprintf(format,"%%%ldd", parms[0].asInt());
      sprintf(formatted,format,leftval.asInt());
      return Value(formatted);
    }
    break;
  case SVT_ENTITY: {
    ENTITY_DATA* entity = leftval.asEntity();
    
    FCT_DATA* fct = getMethod( entity, rightval );
    if ( fct != NULL )
      return runhandler(fct, leftval, parms, params_count);
    break;
  }
  }

  char buf[512];
  if ( leftval.typ == SVT_ENTITY )
    sprintf(buf,"Method (%s) not found in this context (%s) [type: ENTITY, %d]",
	    rightval, leftval.asStr(), leftval.asEntity()->kind );
  else
    sprintf(buf,"Method (%s) not found in this context (%s) [type: %d]",
	    rightval, leftval.asStr(), leftval.typ );
  p_error(buf);
  return Value((long)0);
}

// Value SuperExpression::eval(Context& ctx) const {
//   const char* rightval = ident->image;

//   Value parms[params_count];
//   for (int i = 0; i<params_count; i++)
//     parms[i] = params[i]->eval(ctx);

//   Value* actor = ctx["this"];

//   // Modified by SinaC 2003
//   // Find method in parent class
//   ENTITY_DATA* entity = actor->asEntity();
  
// //   FCT_DATA *f = NULL;
// //   CLASS_DATA* cl = entity->clazz->parent;
// //   while (cl != NULL ) {
// //     f = hash_get_fct( rightval, cl->methods );
// //     if ( f != NULL )
// //       break;
// //     cl = cl->parent;
// //   }
  
//   if ( entity->clazz->parent_count == 0 ) {
//     p_error("Can't call super method from a root class");
//     return Value((long)0);
//   }
// #ifdef VERBOSE2
//   log_stringf("Super: searching %s (%s) for %s",
// 	      entity->clazz->name,
// 	      entity->clazz->parent[0]->name, rightval );
// #endif

//   FCT_DATA *f = innerGetMethod( entity->clazz->parent[0], rightval );

//   if ( f != NULL )
//     return runhandler(f, *actor, parms, params_count );

//   char buf[512];
//   sprintf(buf,"Super: method (%s) not found in this context (%s)",
// 	  rightval, actor->asStr() );
//   p_error(buf);
//   return Value((long)0);
// }

Value ScopeCallExpr::eval(Context& ctx) const {
  CLASS_DATA *cl = hash_get_prog( left->image );
  if ( cl == NULL ) {
    char buf[512];
    sprintf(buf,"ScopeCallExpr: class %s not found", left->image );
    p_error(buf);
    return Value((long)0);
  }

  Value parms[params_count];
  for (int i = 0; i<params_count; i++)
    parms[i] = params[i]->eval(ctx);

  Value* actor = ctx["this"];

  // Find method in scoped class
  const char* rightval = ident->image;
  
//   FCT_DATA *f = NULL;
//   while (cl != NULL ) {
//     f = hash_get_fct( rightval, cl->methods );
//     if ( f != NULL )
//       break;
//     cl = cl->parent;
//   }
  
  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("ScopeCallExpr: searching %s (%s) for %s", 
		  actor->asEntity()->clazz->name, cl->name, rightval );
  }
  FCT_DATA *f = innerGetMethod( cl, rightval );
  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("ScopeCallExpr: method found");
  }

  // If the method has been found, call it
  if ( f != NULL )
    return runhandler(f, *actor, parms, params_count);

  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("ScopeCallExpr: method found: after runhandler");
  }

  char buf[512];
  sprintf(buf,"Method (%s) not found in scoped class (%s)",
	  rightval, left->image );
  p_error(buf);
  return Value((long)0);
}


Value QualifyExpr::eval(Context& ctx) const {
  if ( SCRIPT_VERBOSE > 6 ) {
    log_stringf("--> looking in THIS");
    TRACE;
  }
  Value leftval = left->eval(ctx);
  Value res;
  
  bool found = find_in_context(res, leftval, ident->image);
  
  if (!found) {    
    p_error("field undeclared variable: %s", ident->image);
    res = Value((long) 0);
    if ( SCRIPT_VERBOSE > 6 ) {
      TRACE;
    }
  }
  return res;
}
