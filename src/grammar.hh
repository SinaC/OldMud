#ifndef __GRAMMAR_HH__
#define __GRAMMAR_HH__

#include "merc.h"
#include "script2.h"
#include "db.h"

#define E_O_F (-1)
#define CLASS (-2)
#define EXTENDS (-3)
#define ABSTRACT (-4)
#define COMMAND_LAST (-5)
#define FORCE (-6)
#define WHILE (-7)
#define IF (-8)
#define ELSE (-9)
#define VAR (-10)
#define OBJVAR (-11)
#define DELVAR (-12)
#define LIFETIMEVAR (-13)
#define COMMAND (-14)
#define LEFTARROW (-15)
#define ANY (-16)
#define FIRST (-17)
#define PTPT (-18)
#define IS (-19)
#define HAS (-20)
#define IN (-21)
#define LE (-22)
#define GE (-23)
#define EQ (-24)
#define NE (-25)
#define EQUIV (-26)
#define AND (-27)
#define OR (-28)
#define DBLPT (-29)
#define IDENT (-30)
#define STRING (-31)
#define INTEGER (-32)
#define NULL_TOK (-33)
struct Module;

struct Class;

struct Method;

struct Instruction;

struct CmdParm;

struct Command;

struct ForceInstr;

struct Compound;

struct ExprInstr;

struct WhileInstr;

struct IfInstr;

struct VarDecl;

struct ObjvarDecl;

struct DelvarDecl;

struct LifetimevarDecl;

struct Expression;

struct UnaryExpr;

struct ComprehensionExpr;

struct AnyListExpr;

struct FirstListExpr;

struct PartialList;

struct OtherBinaryExpr;

struct BinaryExpr;

struct ConsList;

struct CallExpr;

struct QualifyExpr;

struct MethodCallExpr;

struct ScopeCallExpr;

struct Identifier;

struct StringLiteral;

struct IntegerLiteral;

struct NullLiteral;

struct Module: NonTerminal{
  virtual void dump(int);
  virtual ~Module();

Class** classes;int classes_count;Module(){
classes = 0; 

}};
struct Class: NonTerminal{
  virtual void dump(int);
  virtual ~Class();

  Identifier* ident;
  int isAbstract;
  Method** methods;
  int methods_count;
  Identifier** extends;
  int extends_count;
  Class() {
    ident = 0; 
    isAbstract = 0; 
    methods = 0; 
    extends = 0; 
  }
};
struct Method: NonTerminal{
  virtual void dump(int);
  virtual ~Method();

  // SinaC 2003 to store method's code
  char *code;

  Compound* body;
  Identifier** params;
  int params_count;
  Identifier* ident;
  Method(){
    body = 0; 
    params = 0; 
    ident = 0; 
    code = 0;
  }
};
struct Instruction: NonTerminal{
  virtual void dump(int);
  virtual ~Instruction();

       virtual void execute(Context& ctx) const;
    
Instruction(){
}};
struct CmdParm: NonTerminal{
  virtual void dump(int);
  virtual ~CmdParm();

       const char* verb;
    
Expression* expr;CmdParm(){
expr = 0; 

}};
struct Command: Instruction{
  virtual void dump(int);
  virtual ~Command();

       const char* verb;
       const char* eval(Context& ctx) const;
     

       virtual void execute(Context& ctx) const;
    
CmdParm** parms;int parms_count;Command(){
parms = 0; 

}};
struct ForceInstr: Instruction{
  virtual void dump(int);
  virtual ~ForceInstr();


       virtual void execute(Context& ctx) const;
    
Expression* ch;Command* content;ForceInstr(){
ch = 0; 

content = 0; 

}};
struct Compound: Instruction{
  virtual void dump(int);
  virtual ~Compound();


       virtual void execute(Context& ctx) const;
    
Instruction** instr;int instr_count;Compound(){
instr = 0; 

}};
struct ExprInstr: Instruction{
  virtual void dump(int);
  virtual ~ExprInstr();


       virtual void execute(Context& ctx) const;
    
Expression* expr;ExprInstr(){
expr = 0; 

}};
struct WhileInstr: Instruction{
  virtual void dump(int);
  virtual ~WhileInstr();


       virtual void execute(Context& ctx) const;
    
Instruction* body;Expression* cond;WhileInstr(){
body = 0; 

cond = 0; 

}};
struct IfInstr: Instruction{
  virtual void dump(int);
  virtual ~IfInstr();


       virtual void execute(Context& ctx) const;
    
Expression* cond;Instruction* thenpart;Instruction* elsepart;IfInstr(){
cond = 0; 

thenpart = 0; 

elsepart = 0; 

}};
struct VarDecl: Instruction{
  virtual void dump(int);
  virtual ~VarDecl();


       virtual void execute(Context& ctx) const;
    
Expression* defval;Identifier* ident;VarDecl(){
defval = 0; 

ident = 0; 

}};
struct ObjvarDecl: Instruction{
  virtual void dump(int);
  virtual ~ObjvarDecl();


       virtual void execute(Context& ctx) const;
    
Expression* object;Identifier* ident;Expression* defval;ObjvarDecl(){
object = 0; 

ident = 0; 

defval = 0; 

}};
struct DelvarDecl: Instruction{
  virtual void dump(int);
  virtual ~DelvarDecl();


       virtual void execute(Context& ctx) const;
    
Expression* object;Identifier* ident;DelvarDecl(){
object = 0; 

ident = 0; 

}};
struct LifetimevarDecl: Instruction{
  virtual void dump(int);
  virtual ~LifetimevarDecl();


       virtual void execute(Context& ctx) const;
    
Expression* object;Identifier* ident;Expression* lifetime;LifetimevarDecl(){
object = 0; 

ident = 0; 

lifetime = 0; 

}};
struct Expression: NonTerminal{
  virtual void dump(int);
  virtual ~Expression();
 
       virtual Value eval(Context& ctx) const;
    
Expression(){
}};
struct UnaryExpr: Expression{
  virtual void dump(int);
  virtual ~UnaryExpr();

 
       virtual Value eval(Context& ctx) const;
    
int op;Expression* expr;UnaryExpr(){
op = 0; 

expr = 0; 

}};
struct ComprehensionExpr: Expression{
  virtual void dump(int);
  virtual ~ComprehensionExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* generator;Expression* condition;Expression* inputList;Identifier* variable;ComprehensionExpr(){
generator = 0; 

condition = 0; 

inputList = 0; 

variable = 0; 

}};
struct AnyListExpr: Expression{
  virtual void dump(int);
  virtual ~AnyListExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* condition;Expression* inputList;Identifier* variable;AnyListExpr(){
condition = 0; 

inputList = 0; 

variable = 0; 

}};
struct FirstListExpr: Expression{
  virtual void dump(int);
  virtual ~FirstListExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* condition;Expression* inputList;Identifier* variable;FirstListExpr(){
condition = 0; 

inputList = 0; 

variable = 0; 

}};
struct PartialList: Expression{
  virtual void dump(int);
  virtual ~PartialList();

 
       virtual Value eval(Context& ctx) const;
    
Expression* inputList;Expression* lowerBound;Expression* upperBound;PartialList(){
inputList = 0; 

lowerBound = 0; 

upperBound = 0; 

}};
struct OtherBinaryExpr: Expression{
  virtual void dump(int);
  virtual ~OtherBinaryExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* left;int op;Identifier* right;OtherBinaryExpr(){
left = 0; 

op = 0; 

right = 0; 

}};
struct BinaryExpr: Expression{
  virtual void dump(int);
  virtual ~BinaryExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* left;int op;Expression* right;BinaryExpr(){
left = 0; 

op = 0; 

right = 0; 

}};
struct ConsList: Expression{
  virtual void dump(int);
  virtual ~ConsList();

 
       virtual Value eval(Context& ctx) const;
    
Expression** elems;int elems_count;ConsList(){
elems = 0; 

}};
struct CallExpr: Expression{
  virtual void dump(int);
  virtual ~CallExpr();

 
       virtual Value eval(Context& ctx) const;
    
Identifier* left;Expression** params;int params_count;CallExpr(){
left = 0; 

params = 0; 

}};
struct QualifyExpr: Expression{
  virtual void dump(int);
  virtual ~QualifyExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* left;Identifier* ident;QualifyExpr(){
left = 0; 

ident = 0; 

}};
struct MethodCallExpr: Expression{
  virtual void dump(int);
  virtual ~MethodCallExpr();

 
       virtual Value eval(Context& ctx) const;
    
Expression* left;Identifier* ident;Expression** params;int params_count;MethodCallExpr(){
left = 0; 

ident = 0; 

params = 0; 

}};
struct ScopeCallExpr: Expression{
  virtual void dump(int);
  virtual ~ScopeCallExpr();

 
       virtual Value eval(Context& ctx) const;
    
Identifier* left;Identifier* ident;Expression** params;int params_count;ScopeCallExpr(){
left = 0; 

ident = 0; 

params = 0; 

}};
struct Identifier: Expression{
  virtual void dump(int);
  virtual ~Identifier();

       const char* image;
     
 
       virtual Value eval(Context& ctx) const;
    
Identifier(){
}};
struct StringLiteral: Expression{
  virtual void dump(int);
  virtual ~StringLiteral();

       const char* image;
     
 
       virtual Value eval(Context& ctx) const;
    
StringLiteral(){
}};
struct IntegerLiteral: Expression{
  virtual void dump(int);
  virtual ~IntegerLiteral();

       int value;
     
 
       virtual Value eval(Context& ctx) const;
    
IntegerLiteral(){
}};
struct NullLiteral: Expression{
  virtual void dump(int);
  virtual ~NullLiteral();

 
       virtual Value eval(Context& ctx) const;
    
NullLiteral(){
}};


// Used by parser
extern const int grammar_initialState;
extern const int grammar_initialMethodState;
extern const char *startMethodCode;
extern const int *grammar_action_table[];
extern const int *grammar_goto_table[];
extern int grammar_goto_entry[];
extern int grammar_rule_depth_table[];
int grammar_map_token(int tok);
NonTerminal* grammar_reduce_rule(int rule_id, stack_elem* rule);

#endif
