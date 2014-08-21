#ifndef _RESTRICTION_H_
#define _RESTRICTION_H_

// Added by SinaC 2000
void init_restr_table();
// Added by SinaC 2001
void update_restr_table( const char *name, flag_type *flag );

// Modified by SinaC 2001
//RESTR_DATA* find_restriction( OBJ_DATA *obj, int restriction );
RESTR_DATA* find_restriction( RESTR_DATA* restr_start, int restriction );
bool    check_restriction args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
void    restrstring     args( ( char *buf, RESTR_DATA *restr ) );

// Added by SinaC 2003
bool    check_size      args( ( CHAR_DATA *ch, OBJ_DATA *obj, bool silent ) );
#endif

