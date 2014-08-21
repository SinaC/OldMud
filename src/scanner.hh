#ifndef __SCANNER_HH__
#define __SCANNER_HH__

extern int token;
extern const char* cur_pos;
extern const char* previous_cur_pos;
extern int numline;

int grammar_scan();

#endif
