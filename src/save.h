#ifndef __SAVE_H__
#define __SAVE_H__

// Added by SinaC 2000
//#define PLAYER_VERSION  (9)
//#define PLAYER_VERSION  (10)
#define PLAYER_VERSION  (11)

bool	load_char_obj	args( ( DESCRIPTOR_DATA *d, char *name ) );

void use_delayed_parse_extra_fields( ENTITY_DATA *ent );

void new_save_pFile( CHAR_DATA *ch, bool leaving );
bool new_load_pFile( DESCRIPTOR_DATA *d, char *name );

const char * replace_char( const char *s, char from, char to );

void new_save_affects( AFFECT_DATA *pafList, FILE *fp );

// Load/Save world's mob/obj
void save_world_state();
void load_world_state();
const char *save_area_state( AREA_DATA *pArea );

#endif
