#ifndef __SCRHASH_H__
#define __SCRHASH_H__

#define HASH_FCT	128
#define HASH_PRG	1024


struct ValList {
  Value v;
  ValList* next;
  const char* name;

  ValList(ValList* n, const Value& vl, const char* aname);
};


extern ValList * glob_table[HASH_FCT];
extern CLASS_DATA* prg_table[HASH_PRG];

void hash_global_init();
void hash_init();
int hash_s(const char * s, int tblsize);

void hash_put_glob(const char*, const Value&);
Value* hash_get_glob(const char *s);

void hash_put_prog(CLASS_DATA * prg);
CLASS_DATA * hash_get_prog(const char *s);
CLASS_DATA * silent_hash_get_prog(const char *s);
// Find a program with a string, it's previous and also the index in prg_table
CLASS_DATA * hash_get_prog_prev(const char *s, CLASS_DATA *&prev, int &idx );
// Find a program, it's previous and also the index in prg_table
CLASS_DATA * hash_get_prog_prev(CLASS_DATA *cl, CLASS_DATA *&prev, int &idx );

FCT_DATA *hash_get_fct(const char *s, FCT_DATA **c);
void hash_put_fct( FCT_DATA *f, FCT_DATA **c);
// Find a function with a string, it's previous and also the index in c
FCT_DATA *hash_get_fct_prev( const char *s, FCT_DATA **c, FCT_DATA *&prev, int &idx );
// Find a function, it's previous and also the index in c
FCT_DATA *hash_get_fct_prev( FCT_DATA *f, FCT_DATA **c, FCT_DATA *&prev, int &idx );

#endif
