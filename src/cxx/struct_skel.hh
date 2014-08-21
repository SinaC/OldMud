#ifndef _STRUCT_SKEL
#define _STRUCT_SKEL

#include <stdlib.h>
#include "../gc_helper.h"

struct stack_elem {
  int state;
  int elem;
  char* tok_image;

  stack_elem() {
    tok_image = NULL;
  }  
};

struct NonTerminal {
  virtual void dump(int depth) {
    //    printf("Dump not implemented, sorry");
  };

  virtual ~NonTerminal() {};
};



struct ListNonTerminal : NonTerminal {
  NonTerminal* head;
  ListNonTerminal* tail;

  ~ListNonTerminal() {
    delete tail;
  }

  static ListNonTerminal* newList(stack_elem* rule, int hidx, int tidx) {
    ListNonTerminal* res = new ListNonTerminal;
    res->head = (NonTerminal*) rule[hidx].elem;
    if (tidx == -1) 
      res->tail = NULL;
    else
      res->tail = (ListNonTerminal*) rule[tidx].elem;
    return res;
  }

};


#endif
