// Added by SinaC 2000
#ifndef _AFFECTS_H_
#define _AFFECTS_H_

// Added by SinaC 2002
extern const struct attr_type *room_attr_table;
void init_room_attr_table();


// Modified by SinaC 2000
void init_attr_table();
// Added by SinaC 2001
void update_attr_table( const char *name, flag_type *flag );


int locoldtonew(int oldloc);
// SinaC 2003 new affect system
void afstring ( char * buf, AFFECT_DATA *af, CHAR_DATA *ch, const bool care_level );
// Modified by SinaC 2001
//void afstring (char * buf, AFFECT_DATA *af, CHAR_DATA *ch, bool spl );
//void afstring_nospell (char * buf, AFFECT_DATA *af, CHAR_DATA *ch, bool spl );

#endif
