#ifndef __UTILS_H__
#define __UTILS_H__

void strip_char( char *s, const char c );
void strip_char_leading( char *s, const char c );
void strip_char_ending( char *s, const char c );


char *  trim            ( const char *str );
char *	print_flags	( const long flag );
int	number_fuzzy	( const int number );
int	number_range	( const int from, int to );
bool    chance          ( const int num );
int     count_bit       ( const long l );
int	number_percent	();
int	number_door	();
long	number_bits	( const long width );
int	dice		( const int number, const int size );
int	interpolate	( const int level, const int value_00, const int value_32 );
char *	capitalize	( const char *str );
char    *str_to_upper   ( const char *str );
const   char *tokenize  ( const char *argument );

#endif
