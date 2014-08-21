#ifndef __STRING_H__
#define __STRING_H__

void	string_edit	args( ( CHAR_DATA *ch, const char **pString ) );
void    string_append   args( ( CHAR_DATA *ch, const char **pString ) );
const char *  string_replace	args( ( const char * orig, const char * old, const char * ) );
void    string_add      args( ( CHAR_DATA *ch, const char *argument ) );
const char *  format_string   args( ( const char *oldstring /*, bool fSpace */ ) );
const char *  first_arg       args( ( const char *argument, char *arg_first, bool fCase ) );
const char *  string_unpad	args( ( const char * argument ) );
const char *  string_proper	args( ( const char * argument ) );

// SinaC 2003
void    string_show     args( ( const char *buffer, CHAR_DATA *ch ) );
const char * string_replace_line args( ( const char *buffer, const int line, const char *newstring ) );
const char *string_delete_line( const char *buffer, const int line );
const char *string_insert_line( const char *buffer, const int line, const char *str );

const char * string_replace_all( const char * orig, const char * old, const char * newstr );
void string_show_without_color( const char *buffer, CHAR_DATA *ch );

const char * string_indent( const char *buffer );

#endif
