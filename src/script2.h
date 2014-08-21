#ifndef _SCRIPT_H_
#define _SCRIPT_H_

#include "assert.h"
#include "gc_helper.h"

#define CONTROLS "{}()<>=+-*/:;,.!?&|"


#define MAX_CODE_SIZE	10240
#define MAX_FCT_PARM	16

#define SVT_INT		0
#define SVT_STR		1
#define SVT_ENTITY	2
#define SVT_FCT		3
#define SVT_VAR         5
#define SVT_LIST        6
#define SVT_VOID	10

typedef struct class_data	CLASS_DATA; 
//typedef struct class_list	CLASS_LIST;
typedef struct fct_data		FCT_DATA;
typedef struct field_data	FIELD_DATA;

struct Value;
struct ValueList;
typedef Value GEF_FUN (ENTITY_DATA *, const int, Value*);

struct field_data;

union val_types {
  ENTITY_DATA*	e;
  long	        i;
  const char *	s;
  FCT_DATA *	f;
  Value*        v;
  ValueList*    l;
};


///////////////////////// VALUES //////////////////


struct Value {
  int			typ;  
  union val_types	val;

  const char* asStr() const;
  ENTITY_DATA *asEntity() const;
  long asInt() const;
  FCT_DATA* asFct() const;
  ValueList* asList() const;

  Value() {
    typ = SVT_INT;
    val.i = 0;
  }

  Value(ValueList* l) {
    typ = SVT_LIST;
    val.l = l;
  }

  Value(FCT_DATA* f) {
    typ = SVT_FCT;
    val.f = f;
  }

  Value(long i) {
    typ = SVT_INT;
    val.i = i;
  }

  Value(ENTITY_DATA* e) {
    typ = SVT_ENTITY;
    val.e = e;
  }

  Value(Value* v) {
    typ = SVT_VAR;
    val.v = v;
  }

  Value(const char* s);


  void setValue(const Value& v);

  Value get() const {
    if (typ == SVT_VAR)
      return val.v->get();
    else
      return *this;
  };

  void explicit_free();

  //~Value();
  
  //private:
  //Value& operator=(const Value & v);
};

struct field_data {
  FIELD_DATA *	next;
  const char *	name;
  int		lifetime;
  Value var;
};

struct ValueList {
  int size;
  Value elems[0];

  bool contains(const Value& v) const;
  bool includes(const ValueList* vl) const;
  Value get( const int n) const;
  Value unique() const;
  Value uniqueEquiv() const;
  // Added by SinaC 2003
  int index(const Value& v) const;
  // Added by SinaC 2003
  Value flat() const;

  static ValueList* newList(int size);
  static ValueList* emptyList();

  void explicit_free();
};




int operator==(const Value& a,  const Value& b);
int operator<(const Value& a,  const Value& b);
int operator>(const Value& a,  const Value& b);
int operator!=(const Value& a,  const Value& b);
int operator<=(const Value& a,  const Value& b);
int operator>=(const Value& a,  const Value& b);
Value operator+(const Value& a, const Value& b);
Value operator-(const Value& a, const Value& b);
Value operator*(const Value& a, const Value& b);
Value operator/(const Value& a, const Value& b);
Value operator!(const Value& r);
Value operator-(const Value& r);

bool valueEquiv( const Value& a, const Value& b );




////////////////////////// Context //////////////////////////////////

#define MAX_SCOPE_VARS (32)
#define MAX_SCOPES (32)
#define MAX_SCOPE_VAR_NAME_SIZE (32)

class Scope {
  int numvar;

  Value* val[MAX_SCOPE_VARS];
  const char* name[MAX_SCOPE_VAR_NAME_SIZE];
 public:

  Value* newVar(const char* n);
	
  Value* getVar(const char* n);

  void clean();

  Scope() {
    numvar = 0;
  };
};


class Context {

  int numscope;

  Scope scopes[MAX_SCOPES];
  
 public:

  Context(){
    numscope = 0;
  }

  Value* operator [](const char* n) {
    for (int i = numscope-1; i>=0; i--) {
      Value* res = scopes[i].getVar(n);
      if (res != NULL) {
	return res;
      }
    }
    return NULL;
  }

  Value newVar(const char* name);

  void pushscope();
  void popscope();
};


//////////////////////////////////////////////////////////////////////

struct Compound;

// Fct modified: not updated
#define FCT_MODIFIED (A)
// Fct has not a valid  Compound *code,  testing this bit is equivalent to check if  code == NULL
#define FCT_NO_CODE  (B)

struct fct_data: OLCEditable {
  GEF_FUN *	predfct;
  Compound*	code;
  int		nbparm;
  const char *	parmname[MAX_FCT_PARM];
  const char *	name;

  const char *  scriptCode; // function code in script language
  const char *  scriptCodeSaved; // function code from file, allow a revert
  // SinaC 2003
  int           flags; // bit vector, check values above
  int           positionId; // just used for better look in file


  CLASS_DATA *  incoming; // Class containing this method

  FCT_DATA *	next;                     /* Next in hash or class*/
};


// Class modified: some methods are not parsed or not correct
#define CLASS_MODIFIED  (A)
// Class not saved
#define CLASS_NOT_SAVED (B)
// Mark bit used by check_cyclic_inheritance
#define CLASS_MARKED    (C)

struct class_data: OLCEditable {
  const char * 	name;
  //FCT_DATA * 	methods;   // List of functions of the class
  FCT_DATA   ** methods; // hash list
  //CLASS_DATA *	parent; // Inherited class
  int           parents_count;
  CLASS_DATA **	parents; // Inherited classes  MULTIPLE INHERITANCE, SinaC 2003


  bool          isAbstract; // an abstract class can't be assigned to an entity

  // Added by SinaC 2002 to keep trace of class's incoming file
  const char *  file;
  // SinaC 2003
  int           flags; // bit vector, check values above

  CLASS_DATA *	next;	// Next in hash list
};

extern long topscripts;

int mobprog( CHAR_DATA* mob, CHAR_DATA* act, const char *name, Value * params);
int objprog( OBJ_DATA* obj, CHAR_DATA* act, const char *name, Value * params);
int roomprog( ROOM_INDEX_DATA* room, CHAR_DATA* act, const char *name, Value * params);
int common_prog(ENTITY_DATA* subject, const char *name, Value * params);

extern const char * typ_names[];
extern CLASS_DATA* default_mob_class;
extern CLASS_DATA* default_obj_class;
extern CLASS_DATA* default_room_class;

Value add_extra_field(const Value& v, const char* n);
Value* get_extra_field(ENTITY_DATA* entity, const char* n);
FIELD_DATA *get_field_data( ENTITY_DATA *entity, const char *n );
bool del_extra_field( ENTITY_DATA *entity, const char *n );
void execute(Context& ctx, ENTITY_DATA* entity, const char* cmd);
Value runhandler(FCT_DATA * fct, const Value& subject, Value* params, int nparams);

// encapsulation macro; for ease and optimization purposes

#define MOBPROG(mob, act, name, parms...)		\
do {						\
  Value args[] = {parms};			\
  mobprog(mob, act, name, args);		\
} while (false)


#define OBJPROG(obj, act, name, parms...)		\
do {						\
  Value args[] = {parms};			\
  objprog(obj, act, name, args);		\
} while (false)

// Added by SinaC 2003 for room programs
#define ROOMPROG(room, act, name, parms...)		\
do {						\
  Value args[] = {parms};			\
  roomprog(room, act, name, args);		\
} while (false)
// Added by SinaC 2003 for room programs
#define COMMONPROG(enti, name, parms...)		\
do {						\
  Value args[] = {parms};			\
  common_prog(enti, name, args);		\
} while (false)


#endif

