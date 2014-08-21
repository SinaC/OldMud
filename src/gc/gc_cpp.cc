/*************************************************************************
Copyright (c) 1994 by Xerox Corporation.  All rights reserved.
 
THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 
    Last modified on Sat Nov 19 19:31:14 PST 1994 by ellis
                  on Sat Jun  8 15:10:00 PST 1994 by boehm

Permission is hereby granted to copy this code for any purpose,
provided the above notices are retained on all copies.

This implementation module for gc_c++.h provides an implementation of
the global operators "new" and "delete" that calls the Boehm
allocator.  All objects allocated by this implementation will be
non-collectable but part of the root set of the collector.

You should ensure (using implementation-dependent techniques) that the
linker finds this module before the library that defines the default
built-in "new" and "delete".

Authors: John R. Ellis and Jesse Hull

**************************************************************************/
/* Boehm, December 20, 1994 7:26 pm PST */

#include <stdio.h>
#include "include/gc_cpp.h"

void* operator new( size_t size ) {
  void * tmp = GC_MALLOC( size );
#ifdef GC_VERBOSE_GLOBAL
  if ( GC_VERBOSE > GC_VERBOSE_VALUE ) {
    printf("GC --- gc_cpp.cc: operator new (%d) [%x]\n\r", size, (int)tmp ); fflush(stdout);
  }
#endif
  //    return GC_MALLOC_UNCOLLECTABLE( size );}
  return tmp;
}
  
void operator delete( void* obj ) {
#ifdef GC_VERBOSE_GLOBAL
  if ( GC_VERBOSE > GC_VERBOSE_VALUE ) {
    printf("GC --- gc_cpp.cc: operator delete (%x)\n\r", (int)obj); fflush(stdout);
  }
#endif
  GC_FREE( obj );
}
  
#ifdef _MSC_VER
// This new operator is used by VC++ in case of Debug builds !
void* operator new( size_t size,
                    int nBlockUse,
                    const char * szFileName,
                    int nLine
                    ) {
  if ( GC_VERBOSE > GC_VERBOSE_VALUE ) {
    printf("GC --- gc_cpp.cc:operator new (%d, %d, %s, %d)\n\r",
	   size, nBlockUse, szFileName, nLine ); fflush(stdout);
  }
# ifndef GC_DEBUG
  return GC_malloc_uncollectable( size );
# else
  return GC_debug_malloc_uncollectable(size, szFileName, nLine);
# endif
}
#endif

#ifdef OPERATOR_NEW_ARRAY

void* operator new[]( size_t size ) {
#ifdef GC_VERBOSE_GLOBAL
  if ( GC_VERBOSE > GC_VERBOSE_VALUE ) {
    printf("GC --- gc_cpp.cc:operator new[] (%d)\n\r", size ); fflush(stdout);
  }
#endif
  return GC_MALLOC( size );
}
  
void operator delete[]( void* obj ) {
#ifdef GC_VERBOSE_GLOBAL
  if ( GC_VERBOSE > GC_VERBOSE_VALUE ) {
    printf("GC --- gc_cpp.cc:operator delete[] (%x)\n\r", (int)obj ); fflush(stdout);
  }
#endif
  GC_FREE( obj );
}

#endif /* OPERATOR_NEW_ARRAY */


