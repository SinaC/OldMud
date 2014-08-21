#include "cxx/parser_skel.hh"
#include "data_parser.hh"
#include <stdio.h>
const int data_initialState = 32;
int data_map_token(int tok) {
  switch (tok) {case E_O_F: return 1;
case '=': return 2;
case ';': return 3;
case '{': return 4;
case '}': return 5;
case ':': return 6;
case '(': return 7;
case ')': return 8;
case '+': return 9;
case DATA_IDENT: return 10;
case DATA_BOOLEAN: return 11;
case DATA_STRING: return 12;
case DATA_INTEGER: return 13;
case ',': return 14;
default : 
  printf("maptoken: got unknown token: %d '%c'!\
", tok, tok);
  return 0;
}}
const int actionrow0[] = {0, -17, 0, 0, 0, 0, -17, 0, 0, -17, -17, -17, -17, 0};
const int actionrow1[] = {0, 0, -9, -9, 0, -9, 0, -9, -9, 0, 0, 0, 0, -9};
const int actionrow2[] = {0, 0, -10, -10, 0, -10, 0, -10, -10, 0, 0, 0, 0, -10};
const int actionrow3[] = {0, 0, -11, -11, 0, -11, 0, -11, -11, 0, 0, 0, 0, -11};
const int actionrow4[] = {0, 0, -12, -12, 0, -12, 0, -12, -12, 0, 0, 0, 0, -12};
const int actionrow5[] = {0, 0, -8, -8, 0, -8, 0, -8, -8, 0, 0, 0, 0, -8};
const int actionrow6[] = {0, 0, 0, 0, 0, 0, 14, 0, 0, 2, 3, 4, 5, 0};
const int actionrow7[] = {0, 0, 0, 0, 0, 0, 0, -15, 7, 0, 0, 0, 0, -15};
const int actionrow8[] = {0, 0, 0, 0, 0, 0, 0, -16, 7, 0, 0, 0, 0, -16};
const int actionrow9[] = {0, 0, 0, 0, 0, 0, 0, -14, 0, 0, 0, 0, 0, 10};
const int actionrow10[] = {0, 0, -7, -7, 0, -7, 0, -7, -7, 0, 0, 0, 0, -7};
const int actionrow11[] = {0, 0, 0, 0, 0, 0, 0, 12, 0, 0, 0, 0, 0, 0};
const int actionrow12[] = {0, 0, 0, 0, 0, 0, 14, -13, 0, 2, 3, 4, 5, 0};
const int actionrow13[] = {-4, 0, 0, 0, -4, 0, 0, 0, 0, -4, 0, 0, 0, 0};
const int actionrow14[] = {0, 0, 15, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0};
const int actionrow15[] = {-3, 0, 0, 0, -3, 0, 0, 0, 0, -3, 0, 0, 0, 0};
const int actionrow16[] = {-6, 0, 0, 0, -6, 0, 0, 0, 0, -6, 0, 0, 0, 0};
const int actionrow17[] = {0, 0, 0, 0, 19, 0, 0, 0, 0, 1, 0, 0, 0, 0};
const int actionrow18[] = {0, 0, 0, 0, -2, 0, 0, 0, 0, -2, 0, 0, 0, 0};
const int actionrow19[] = {0, 0, 0, 21, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0};
const int actionrow20[] = {-5, 0, 0, 0, -5, 0, 0, 0, 0, -5, 0, 0, 0, 0};
const int actionrow21[] = {0, 0, 0, 0, 24, 0, 0, 0, 0, 1, 0, 0, 0, 0};
const int actionrow22[] = {0, 0, 0, 26, 0, 23, 0, 0, 7, 0, 0, 0, 0, 0};
const int actionrow23[] = {0, 17, 0, 0, 0, 0, 14, 0, 0, 2, 3, 4, 5, 0};
const int actionrow24[] = {-1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0};
const int actionrow25[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const int actionrow26[] = {30, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const int actionrow27[] = {-2, 0, 0, 0, 0, 0, 0, 0, 0, -2, 0, 0, 0, 0};
const int *data_action_table[] = {actionrow0, actionrow1, actionrow2, actionrow3, actionrow4, actionrow5, actionrow6, actionrow7, actionrow8, actionrow6, actionrow9, actionrow10, actionrow11, actionrow12, actionrow13, actionrow14, actionrow6, actionrow15, actionrow16, actionrow17, actionrow18, actionrow19, actionrow6, actionrow20, actionrow21, actionrow18, actionrow22, actionrow23, actionrow24, actionrow25, actionrow26, actionrow27};
const int gotorow0[] = {0, 0, 0, 0, 0, 0, 0};
const int gotorow1[] = {0, 0, 0, 6, 0, 0, 0};
const int gotorow2[] = {0, 0, 0, 9, 0, 0, 0};
const int gotorow3[] = {0, 0, 0, 8, 13, 11, 0};
const int gotorow4[] = {0, 0, 0, 16, 0, 0, 0};
const int gotorow5[] = {0, 0, 18, 0, 0, 0, 28};
const int gotorow6[] = {0, 20, 0, 0, 0, 0, 0};
const int gotorow7[] = {0, 0, 0, 22, 0, 0, 0};
const int gotorow8[] = {0, 25, 0, 0, 0, 0, 0};
const int gotorow9[] = {0, 0, 0, 27, 0, 0, 0};
const int gotorow10[] = {31, 29, 0, 0, 0, 0, 0};
const int *data_goto_table[] = {gotorow0, gotorow0, gotorow0, gotorow0, gotorow0, gotorow0, gotorow1, gotorow0, gotorow0, gotorow2, gotorow0, gotorow0, gotorow0, gotorow3, gotorow0, gotorow0, gotorow4, gotorow0, gotorow0, gotorow5, gotorow6, gotorow0, gotorow7, gotorow0, gotorow5, gotorow8, gotorow0, gotorow9, gotorow5, gotorow0, gotorow0, gotorow10};
int data_goto_entry[] = {0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 5, 5, 6};int data_rule_depth_table[] = {1, 0, 2, 4, 5, 7, 3, 3, 1, 1, 1, 1, 0, 1, 1, 3, 1};NonTerminal* data_reduce_rule(int rule_id, stack_elem* rule) {
   switch(rule_id) {
case 1: {DATAModule*  result = new DATAModule;result->datas = (DATAData**) listAsArray((ListNonTerminal*) rule[0].elem, result->datas_count);
return result;}case 2: {return NULL;
}case 3: {return ListNonTerminal::newList(rule, 1, 0);
}case 4: {DATAData*  result = new DATAData;result->tag = (DATAIdentifier*) rule[0].elem;

result->value = (DATAExpression*) rule[2].elem;

return result;}case 5: {DATAData*  result = new DATAData;result->tag = (DATAIdentifier*) rule[0].elem;
result->value = (DATAExpression*) rule[1].elem;

result->fields = (DATAData**) listAsArray((ListNonTerminal*) rule[3].elem, result->fields_count);

return result;}case 6: {DATAData*  result = new DATAData;result->tag = (DATAIdentifier*) rule[0].elem;
result->value = (DATAExpression*) rule[1].elem;

result->parent = (DATAExpression*) rule[3].elem;

result->fields = (DATAData**) listAsArray((ListNonTerminal*) rule[5].elem, result->fields_count);

return result;}case 7: {DATAList*  result = new DATAList;
result->elems = (DATAExpression**) listAsArray((ListNonTerminal*) rule[1].elem, result->elems_count);

return result;}case 8: {DATABinaryExpr*  result = new DATABinaryExpr;result->left = (DATAExpression*) rule[0].elem;
result->op = (int) rule[1].elem;
result->right = (DATAExpression*) rule[2].elem;
return result;}case 9: {DATAIdentifier*  result = new DATAIdentifier;
       result->image=str_dup(rule[0].tok_image);
     
return result;}case 10: {DATABooleanLiteral*  result = new DATABooleanLiteral;
      if ( !str_cmp(rule[0].tok_image, "true" ) )
        result->value = 1;
      else if ( !str_cmp(rule[0].tok_image, "false" ) )
        result->value = 0;
      else
        p_error("Invalid boolean value");
    
return result;}case 11: {DATAStringLiteral*  result = new DATAStringLiteral;
       result->image=str_dup(rule[0].tok_image);
     
return result;}case 12: {DATAIntegerLiteral*  result = new DATAIntegerLiteral;
       result->value = atoi(rule[0].tok_image);
     
return result;}case 13: {return NULL;
}case 14: {return (ListNonTerminal*) rule[0].elem;
}case 15: {return ListNonTerminal::newList(rule, 0, -1);
}case 16: {return ListNonTerminal::newList(rule, 2, 0);
}case 17: {DATAIdentifier*  result = new DATAIdentifier;
       result->image=str_dup(rule[0].tok_image);
     
return result;} default:   printf("reduce_rule: got an invalid rule id.");   exit(-1);   }}
DATAModule::~DATAModule(){
  //  if ( datas != NULL ) {
  //    for (int i=0; i<datas_count; i++)
  //      if ( datas[i] != NULL )
  //	delete datas[i];
  //    delete[] datas;
  //  }

}void DATAModule::dump(int depth) {
  printf("%sDATAModule\n", shift(depth-1));
if (datas) for (int i=0; i<datas_count; i++) {
  printf("%sdatas[%d]\n", shift(depth), i);
  datas[i]->dump(depth+2);
}

}
DATAData::~DATAData(){
  if ( fields != NULL ) {
    for (int i=0; i<fields_count; i++)
      if ( fields[i] != NULL )
	delete fields[i];
    delete[] fields;
  }

  if ( tag != NULL )
    delete tag;
  if ( value != NULL )
    delete value;
  if ( parent != NULL )
    delete parent;
}void DATAData::dump(int depth) {
  printf("%sDATAData\n", shift(depth-1));
if (tag) {printf("%stag\n", shift(depth));
 tag->dump(depth+2);
}

if (value) {printf("%svalue\n", shift(depth));
 value->dump(depth+2);
}

if (fields) for (int i=0; i<fields_count; i++) {
  printf("%sfields[%d]\n", shift(depth), i);
  fields[i]->dump(depth+2);
}

if (parent) {printf("%sparent\n", shift(depth));
 parent->dump(depth+2);
}

}
DATAExpression::~DATAExpression(){
}void DATAExpression::dump(int depth) {
  printf("%sDATAExpression\n", shift(depth-1));
}
DATAList::~DATAList(){
  if ( elems != NULL ) {
    for (int i=0; i<elems_count; i++)
      if ( elems[i] != NULL )
	delete elems[i];
    delete[] elems;
  }

}void DATAList::dump(int depth) {
  printf("%sDATAList\n", shift(depth-1));
if (elems) for (int i=0; i<elems_count; i++) {
  printf("%selems[%d]\n", shift(depth), i);
  elems[i]->dump(depth+2);
}

}
DATABinaryExpr::~DATABinaryExpr(){
  if ( left != NULL )
    delete left;
  if ( right != NULL )
    delete right;
}void DATABinaryExpr::dump(int depth) {
  printf("%sDATABinaryExpr\n", shift(depth-1));
if (op) printf("%sop = %d [%c]\n", shift(depth), op, op);

if (left) {printf("%sleft\n", shift(depth));
 left->dump(depth+2);
}

if (right) {printf("%sright\n", shift(depth));
 right->dump(depth+2);
}

}
DATAIdentifier::~DATAIdentifier(){
}void DATAIdentifier::dump(int depth) {
  printf("%sDATAIdentifier\n", shift(depth-1));
}
DATABooleanLiteral::~DATABooleanLiteral(){
}void DATABooleanLiteral::dump(int depth) {
  printf("%sDATABooleanLiteral\n", shift(depth-1));
}
DATAStringLiteral::~DATAStringLiteral(){
}void DATAStringLiteral::dump(int depth) {
  printf("%sDATAStringLiteral\n", shift(depth-1));
}
DATAIntegerLiteral::~DATAIntegerLiteral(){
}void DATAIntegerLiteral::dump(int depth) {
  printf("%sDATAIntegerLiteral\n", shift(depth-1));
}

