#include <time.h>  // added by SinaC 2003
#include <string.h>
#include "merc.h"
#include "script2.h"
#include "cxx/struct_skel.hh"
#include "data_parser.hh"
#include "assert.h"
#include "db.h"
#include "scrvalue.h"
#include "dbdata.h"

Value DATABooleanLiteral::eval() const {
  return Value(value);
}

Value DATAStringLiteral::eval() const {
  return Value(image);
}

Value DATAIntegerLiteral::eval() const {
  return Value(value);
}

Value DATAIdentifier::eval() const {
  Value* slot = getContext(image);
  if ( slot == NULL )
    p_error("Tag [%s] not found in data context.", image );
  return Value(slot);
}

Value DATAList::eval() const {
  if (elems_count == 0) {
    return ValueList::emptyList();
  }
  ValueList* l = ValueList::newList(elems_count);
  for (int i = 0; i<elems_count; i++)
    l->elems[i] = elems[i]->eval().get(); // .get()  added by SinaC 2003
  return l;
}

Value DATABinaryExpr::eval() const {
  switch(op) {
  case '+':
    return left->eval() + right->eval();
  default:
    ASSERT(false, "parsed unknown operator");
  }
}

Value DATAExpression::eval() const {
  ASSERT(false, "Eval is not to be called on abstract expressions");
}
