#ifndef __LANGUAGE_H__
#define __LANGUAGE_H__

#define MAX_LANGUAGE 10

void   assign_language_sn  args( ( void ) );
bool   is_language         args( ( const int sn ) );
int    language_lookup     args( ( const char *name ) );
int    language_sn         args( ( int i ) );
char   *language_name      args( ( int i ) );
char   *language_name_known args( ( CHAR_DATA *ch, int lang ) );

int    get_current_language args( ( CHAR_DATA *ch ) );
//char   *str_to_language    args( ( CHAR_DATA *ch, int language, char *argument ) );
char   *str_to_language    args( ( CHAR_DATA *from, CHAR_DATA *ch, int language, char *argument ) );


DECLARE_DO_FUN( do_language );

#endif

