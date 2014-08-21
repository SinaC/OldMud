#include <time.h>  // added by SinaC 2003
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "db.h"
#include "scrhash.h"

//#define VERBOSE

ValList::ValList(ValList* n, const Value& vl, const char* aname) {
  next = n;
  v = vl;
  name = str_dup(aname);
}

ValList * glob_table[HASH_FCT];     // global predefined functions
CLASS_DATA* prg_table[HASH_PRG];    // 'list' of classes

void hash_init() {
  for (int i = 0; i < HASH_PRG; i++ )
    prg_table[i] = 0;
  for (int i=0;i<HASH_FCT;i++)
    glob_table[i] = 0;
}

int hash_s(const char * s, int tblsize) {  
  unsigned int i;
  long res = 0;
  
  for (i = 0; i<strlen(s); i++)
    res += (s[i] | 0x20)*i; // Case insensitive
  return res % tblsize;   
  
  /*
  unsigned int n = UMIN(strlen(s),16);
  for (i = 0; i<n; i++)
    res += (s[i] | 0x20)*i; // Case insensitive
  return res % tblsize;   
  */
}

void hash_put_glob(const char* name, const Value& v) {
  int idx = hash_s(name,HASH_FCT);
  ValList* vl = new ValList(glob_table[idx], v, name);
  glob_table[idx] = vl;
}

Value* hash_get_glob(const char *s) {
  ValList* res = glob_table[hash_s(s,HASH_FCT)];
  if (res) {
    for (;res && str_cmp(res->name,s);res = res->next) /* empty */;
    if (res)
      return &res->v;
    else {
      bug("problem in hash fct, trying to find fct '%s'",s);
      return NULL;
    }
  } 
  else
    return NULL;
}

void hash_put_prog(CLASS_DATA * prg) {
  int idx = hash_s(prg->name,HASH_PRG);
  //  if ( prg_table[idx] != NULL && !strcmp( prg->name, prg_table[idx]->name ) )
  //    p_error("Class %s duplicated.", prg->name );

  prg->next = prg_table[idx];
  prg_table[idx] = prg;
}

CLASS_DATA * hash_get_prog(const char *s) {
  CLASS_DATA * res = prg_table[hash_s(s,HASH_PRG)];
  if (res) {
    for (;res && str_cmp(res->name,s);res = res->next) /* empty */;
    if (res)
      return res;
    else {
      bug("problem in hash fct, trying to find prog '%s'",s);
      return NULL;
    }
  } 
  else
    return NULL;
}

CLASS_DATA * hash_get_prog_prev(const char *s, CLASS_DATA *&prev, int &idx ) {
  idx = hash_s(s,HASH_PRG);
  prev = prg_table[idx];
  CLASS_DATA * res = prg_table[idx];
  if (res) {
    while ( res && str_cmp(res->name,s) ) {
      prev = res;
      res = res->next;
    }
    return res;
  }
  return NULL;
}

CLASS_DATA * hash_get_prog_prev(CLASS_DATA *cl, CLASS_DATA *&prev, int &idx ) {
  for ( idx = 0; idx < HASH_PRG; idx++ ) {
    CLASS_DATA * res = prg_table[idx];
    prev = prg_table[idx];
    while ( res != NULL ) {
      if ( res == cl )
	return cl;
      prev = res;
      res = res->next;
    }
  }
  return NULL;
}

CLASS_DATA * silent_hash_get_prog(const char *s) {
  CLASS_DATA * res = prg_table[hash_s(s,HASH_PRG)];
  if (res) {
    for (;res && str_cmp(res->name,s);res = res->next) /* empty */;
    if (res)
      return res;
    else
      return NULL;
  } 
  else
    return NULL;
}

int max = 0;

FCT_DATA *hash_get_fct( const char *s, FCT_DATA **c ) {
  int i = hash_s(s,HASH_FCT);

  FCT_DATA * res = c[i];
#ifdef VERBOSE
  int count = 0;
#endif
  if (res) {
    for (;res && str_cmp(res->name,s);res = res->next) /* empty */
#ifdef VERBOSE
      count++
#endif
	;
#ifdef VERBOSE
    if ( count > max )
      max = count;
    log_stringf("%3d     [%3d]", count, max  );
#endif
    if (res)
      return res;
    else {
      //bug("problem in hash fct, trying to find fct '%s'",s);
      return NULL;
    }
  } 
  else
    return NULL;
}

void hash_put_fct( FCT_DATA *f, FCT_DATA **c ) {
  int idx = hash_s(f->name,HASH_FCT);
  //  if ( c[idx] != NULL && !strcmp( f->name, c[idx]->name ) )
  //    p_error("Method %s duplicated.", f->name );

  f->next = c[idx];
  c[idx] = f;
}

FCT_DATA *hash_get_fct_prev( const char *s, FCT_DATA **c, FCT_DATA *&prev, int &idx ) {
  idx = hash_s(s,HASH_FCT);
  prev = c[idx];
  FCT_DATA * res = c[idx];
  if (res) {
    while ( res && str_cmp(res->name,s) ) {
      prev = res;
      res = res->next;
    }
    return res;
  }
  return NULL;
}

FCT_DATA *hash_get_fct_prev( FCT_DATA *f, FCT_DATA **c, FCT_DATA *&prev, int &idx ) {
  for ( idx = 0; idx < HASH_FCT; idx++ ) {
    FCT_DATA *res = c[idx];
    prev = c[idx];
    while ( res != NULL ) {
      if ( res == f )
	return f;
      prev = res;
      res = res->next;
    }
  }
  return NULL;
}
