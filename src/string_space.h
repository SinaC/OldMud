#ifndef __STRING_SPACE_H__
#define __STRING_SPACE_H__

#define FALSE (0)

extern int nStrDup;
extern int nStrDupEmpty;
extern int nStrDupGC;
extern int nStrDupNoGC;
extern int nStrDupUnsafe;
extern int sStrDupNoGC;
extern int sStrDupUnsafe;
extern int nAllocPermStr;
extern int sAllocPermStr;
extern int nAllocPermGC;
extern int nAllocPermHash;
extern int nAllocPermNoHash;

void str_space_init();
const char* alloc_perm_string(const char *s);

// if permAlloc is TRUE: alloc_perm_string is called instead of normal str_dup treatment
//const char *str_dup( const char *str );
const char *str_dup( const char *str, const bool permAlloc = FALSE );
//const char *str_dup_capitalize( const char *str );
const char *str_dup_capitalize( const char *str, const bool permAlloc = FALSE );
//char *str_dup_unsafe( const char *str );
char *str_dup_unsafe( const char *str, const bool permAlloc = FALSE );
//const char *truncate_str( const char *str, const int size );
const char *truncate_str( const char *str, const int size, const bool permAlloc = FALSE );
//const char *clever_str_dup( const char *str );
const char *clever_str_dup( const char *str, const bool permAlloc = FALSE );

int get_str_free_entries();
int get_str_space_size();

extern const char* stringThis;
extern const char* stringResult;

#endif
