#ifndef _BIT_H_
#define _BIT_H_

// char *name     added by SinaC 2001
struct flag_stat_type {
  const char *name;
  struct flag_type *structure;
  bool stat;
};


extern bool FLAG_VALUE_ERROR;

// Added by SinaC 2000, flag_stat_type needs to be seen from db.C
void init_flag_stat_table();
// Added by SinaC 2001
void update_flag_stat_table( const char *name, flag_type *flag );
void init_flag_stat_table_init();
void update_flag_stat_table_init( const char *name, flag_type *flag );

const char *flag_string_init( struct flag_type *flag_table, long bits );
long flag_value_init( struct flag_type *flag_table, const char *argument);
long flag_value_complete_init( struct flag_type *flag_table, const char *argument);

// Added by SinaC 2003, called from predefined functions
long flag_value_complete( struct flag_type *flag_table, const char *argument);
// Those functions never return NO_FLAG, but modified a global value: FLAG_VALUE_ERROR
long flag_value_maximum( struct flag_type *flag_table, const char *argument);
long flag_value_maximum_init( struct flag_type *flag_table, const char *argument);

const char *get_flag_table_name( struct flag_type *flag_table );
const char *get_flag_table_name_init( struct flag_type *flag_table );

long flag_value			args ( ( struct flag_type *flag_table, const char *argument) );
const char *flag_string		args ( ( struct flag_type *flag_table, long bits ) );
bool is_stat( struct flag_type *flag_table );
bool is_stat_init( struct flag_type *flag_table );


#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))


#endif
