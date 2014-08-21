#include <stdio.h>
#include <stdlib.h>
#include "gc.h"
#include "include/gc_cpp.h"

//#define GC_MALLOC(x) malloc(x)

// extern inline void* operator new( size_t size );
// extern inline void operator delete( void* obj );
// extern inline void* operator new []( size_t size );
// extern inline void operator delete[]( void* obj );

class test {
public:
  void tadaa() {
    printf("Tadaa!!!\n\r");
  }
};

int main() {
  int *a = new int [10];
  char *b = new char[20];
  test *t = new test;
  t->tadaa();
}
