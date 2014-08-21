#include <stdio.h>
#include "gc_helper.h"
#include "merc.h"
#include "config.h"

void dump_GC_info() {
  if ( GC_VERBOSE > 0 ) {
    if ( GC_VERBOSE > 1 )
      GC_gcollect();
    printf("Garbage Collector:\n\r"
	   "-----------------\n\r");
    printf("Heap size      : %d\n\r", GC_get_heap_size());
    printf("Free bytes     : %d\n\r", GC_get_free_bytes());
    printf("Bytes allocated: %d\n\r", GC_get_bytes_since_gc());
    printf("Uncollectable  : %d\n\r", GC_get_heap_size() - GC_get_free_bytes() - GC_get_bytes_since_gc());
    fflush(stdout);
  }
}
