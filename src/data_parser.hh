
#ifndef __DATA_PARSER_HH__
#define __DATA_PARSER_HH__

#include "merc.h"
#include "script2.h"
#include "db.h"
#include "error.hh"
  
#define E_O_F (-1)
#define DATA_IDENT (-2)
#define DATA_BOOLEAN (-3)
#define DATA_STRING (-4)
#define DATA_INTEGER (-5)
struct DATAModule;

struct DATAData;

struct DATAExpression;

struct DATAList;

struct DATABinaryExpr;

struct DATAIdentifier;

struct DATABooleanLiteral;

struct DATAStringLiteral;

struct DATAIntegerLiteral;

struct DATAModule: NonTerminal{
  virtual void dump(int);
  virtual ~DATAModule();

DATAData** datas;int datas_count;DATAModule(){
datas = 0; 

}};
struct DATAData: NonTerminal{
  virtual void dump(int);
  virtual ~DATAData();

DATAIdentifier* tag;DATAExpression* value;DATAData** fields;int fields_count;DATAExpression* parent;DATAData(){
tag = 0; 

value = 0; 

fields = 0; 

parent = 0; 

}};
struct DATAExpression: NonTerminal{
  virtual void dump(int);
  virtual ~DATAExpression();
 
       virtual Value eval() const;
    
DATAExpression(){
}};
struct DATAList: DATAExpression{
  virtual void dump(int);
  virtual ~DATAList();

 
       virtual Value eval() const;
    
DATAExpression** elems;int elems_count;DATAList(){
elems = 0; 

}};
struct DATABinaryExpr: DATAExpression{
  virtual void dump(int);
  virtual ~DATABinaryExpr();

 
       virtual Value eval() const;
    
int op;DATAExpression* left;DATAExpression* right;DATABinaryExpr(){
op = 0; 

left = 0; 

right = 0; 

}};
struct DATAIdentifier: DATAExpression{
  virtual void dump(int);
  virtual ~DATAIdentifier();

       const char* image;
     
 
       virtual Value eval() const;
    
DATAIdentifier(){
}};
struct DATABooleanLiteral: DATAExpression{
  virtual void dump(int);
  virtual ~DATABooleanLiteral();

      bool value;
    
 
       virtual Value eval() const;
    
DATABooleanLiteral(){
}};
struct DATAStringLiteral: DATAExpression{
  virtual void dump(int);
  virtual ~DATAStringLiteral();

       const char* image;
     
 
       virtual Value eval() const;
    
DATAStringLiteral(){
}};
struct DATAIntegerLiteral: DATAExpression{
  virtual void dump(int);
  virtual ~DATAIntegerLiteral();

       int value;
     
 
       virtual Value eval() const;
    
DATAIntegerLiteral(){
}};

// Used by parser
extern const int data_initialState;
extern const int *data_action_table[];
extern const int *data_goto_table[];
extern int data_goto_entry[];
extern int data_rule_depth_table[];
int data_map_token(int tok);
NonTerminal* data_reduce_rule(int rule_id, stack_elem* rule);

#endif
