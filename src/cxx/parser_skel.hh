#ifndef __PARSER_SKEL_HH__
#define __PARSER_SKEL_HH__

#include "../gc_helper.h"
#include <stdlib.h>
#include <string.h>
#include "struct_skel.hh"

extern char token_image[4096];
extern int numline;
NonTerminal* parse();
NonTerminal** listAsArray(ListNonTerminal* list, int & out_count);
char* shift(int i);


void init_grammar_parsing();
void init_data_parsing();

NonTerminal *parse_one_class( const char *s );
NonTerminal *parse_one_method( const char *s );

#endif
