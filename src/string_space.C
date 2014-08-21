#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "merc.h"
#include "gc_helper.h"
#include "db.h"
#include "string_space.h"

static const int STR_HASH_SIZE  = 256*1024; //size of the string hash space, in number of entries

int perm_string_space = 0;

static char** str_hash_table;
static int str_hash_count = 0;

const char* stringThis;
const char* stringResult;


// Incremented in str_dup and str_dup_unsafe
// nStrDup = nStrGC + nStrNoGC
int nStrDup = 0;
int nStrDupEmpty = 0;
int nStrDupGC = 0;
int nStrDupNoGC = 0;
int nStrDupUnsafe = 0;
int sStrDupNoGC = 0;
int sStrDupUnsafe = 0;
// Incremented in alloc_perm_string
// nAllocPermStr = nAllocPermGC + nAllocPermHash + nAllocPermNoHash
int nAllocPermStr = 0;
int nAllocPermGC = 0;
int nAllocPermHash = 0;
int nAllocPermNoHash = 0;
int sAllocPermStr = 0;


int get_str_free_entries() {
  return STR_HASH_SIZE - str_hash_count;
}

int get_str_space_size() {
  int size = STR_HASH_SIZE * sizeof(char*);
  for ( int i = 0; i < STR_HASH_SIZE; i++ )
    if ( str_hash_table[i] != NULL )
      size += strlen( str_hash_table[i] );
  return size;
}

void str_space_init() {
  str_hash_table = (char**) GC_MALLOC_UNCOLLECTABLE(STR_HASH_SIZE * sizeof(char*));
  stringResult = alloc_perm_string("result");
  stringThis = alloc_perm_string("this");
  dump_GC_info();
}

static int hash_string(const char *s) {
  const int a = 97;
  const int b = 1001;
  unsigned int res = 1;
  while (*s) {
    res = res * b + a;
    res += *s;
    s++;
  }
  return res % STR_HASH_SIZE;
}

const char* alloc_perm_string(const char *s) {
  nAllocPermStr++;

  //  log_stringf("old_gc_str_dup: allocating %s", s);
  if (GC_base((void*)s) == s) {
    nAllocPermGC++;
    // string in gc_space, thus constant.
    return s;
  } 
  else {
    // try to find string in hash space
    int index = hash_string(s);
    while (str_hash_table[index] != NULL && strcmp(str_hash_table[index], s))
      index = (index + 1) % STR_HASH_SIZE; //better: re-hash.

    char* s2;
    if (str_hash_table[index] == NULL) {
      nAllocPermNoHash++;

      // not found!
      int size =  strlen(s) + 1;
      //s2 = (char *)GC_MALLOC_ATOMIC(size);
      s2 = (char *)GC_MALLOC_UNCOLLECTABLE(size);
      if (str_hash_count >= STR_HASH_SIZE/2) {
	// not enough free hash keys.
	bug("alloc_perm_string: string space overloaded [%s] str_hash_count=%d\n\r", s, str_hash_count);
      } 
      else {
	sAllocPermStr += size;
	str_hash_table[index] = s2;
	str_hash_count++;
      }
      strcpy(s2, s);
    } 
    else {
      nAllocPermHash++;

      s2 = str_hash_table[index];
    }
    return s2;
  }
}

/*
 * Duplicate a string into dynamic memory.
 * Fread_strings are read-only and shared.
 */
//const char *str_dup( const char *str ) {
// permAlloc default: FALSE
const char *str_dup( const char *str, const bool permAlloc ) {
  if (fBootDb || permAlloc )
    return alloc_perm_string(str); // at boot time, we can safely allocate each string permanently.

  nStrDup++;
  char *str_new;

  if ( str[0] == '\0' ) {
    nStrDupEmpty++;

    return &str_empty[0];
  }

  void* dummy = GC_base((void*)str);

  if ( GC_base((void*)str) != NULL ) { // Since strings in the heap cannot be changed, it is ok just to return the pointer.
    nStrDupGC++;

    return str;
  }
  else {
    nStrDupNoGC++;
    sStrDupNoGC += strlen(str)+1;

    str_new = (char *) GC_MALLOC_ATOMIC( strlen(str) + 1 );
    strcpy( str_new, str );
    return str_new;
  }
}

//const char *clever_str_dup( const char *str ) {
// permAlloc default: FALSE
const char *clever_str_dup( const char *str, const bool permAlloc) {
  //  return str_dup(str); // FIXME: hashing!
  return str_dup(str, permAlloc ); // FIXME: hashing!
}

// this is unsafe because others could copy the result. Further modifications would have global changes.
// USE WITH CARE. Change strings only locally. 
// (cast to "const char *" before passing to another proc or put in data structures.)
// See "str_dup_capitalize" for an example.
//char *str_dup_unsafe( const char *str) {
// permAlloc default: FALSE
char *str_dup_unsafe( const char *str, const bool permAlloc ) { // permAlloc not used
  nStrDupUnsafe++;
  sStrDupUnsafe += strlen(str)+1;

  char* str_new = (char *) GC_MALLOC_ATOMIC( strlen(str) + 1 );
  strcpy( str_new, str );
  return str_new;
}

//const char *str_dup_capitalize( const char *str ) {
// permAlloc default: FALSE
const char *str_dup_capitalize( const char *str, const bool permAlloc ) {
  //char* tmp = str_dup_unsafe(str);
  //tmp[0] = UPPER(tmp[0]);
  //return tmp;


  if ( isupper(str[0]) )
    //return str_dup(str);
    return str_dup( str, permAlloc );
  char tmp[MAX_STRING_LENGTH];
  strcpy(tmp,str);
  tmp[0] = UPPER(tmp[0]);
  //return str_dup(tmp);
  return str_dup( tmp, permAlloc );
}

//const char *truncate_str( const char *str, const int size ) {
// permAlloc default: FALSE
const char *truncate_str( const char *str, const int size, const bool permAlloc ) {
  char buf[MAX_STRING_LENGTH];
  strncpy(buf,str,size);
  //  return str_dup(buf);
  return str_dup( buf, permAlloc );
}


/*
int main() {
  const char *s;
  str_space_init();
  
  s = alloc_perm_string("araze");
  printf("returned : %s\n", s); 
  s = alloc_perm_string("araze");
  printf("returned : %s\n", s); 
  s = alloc_perm_string("araze");
  printf("returned : %s\n", s); 
  s = alloc_perm_string("araze");
  printf("returned : %s\n", s); 
  alloc_perm_string("araze");
  printf("returned : %s\n", s); 
  alloc_perm_string("azeraze");
  printf("returned : %s\n", s); 
  alloc_perm_string("araze");
  printf("returned : %s\n", s); 
  alloc_perm_string("azerraze");
  alloc_perm_string("araze");
  s = alloc_perm_string("arzeraze");
  printf("returned : %s\n", s); 
  s = alloc_perm_string("arazzee");
  printf("returned : %s\n", s); 
  alloc_perm_string("arazer");
  alloc_perm_string("arazez");
  alloc_perm_string("araze");
  alloc_perm_string("araze");
  alloc_perm_string("araezze");
  alloc_perm_string("araze");
  alloc_perm_string("araze");
  alloc_perm_string("arezraze");
  alloc_perm_string("araze");
  alloc_perm_string("araze");
  alloc_perm_string("arzeraze");
  s = alloc_perm_string("arazzere");
  alloc_perm_string("arazerze");
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string(s);
  alloc_perm_string("araze");
  alloc_perm_string("arazzere");
}
*/
