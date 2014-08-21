#ifndef __FACTION_H__
#define __FACTION_H__

extern bool factionnotsaved;
extern bool USE_FACTION_ON_AGGRO;


#define BASE_FACTION_VALUE (0)
#define NEUTRAL_FACTION_VALUE (0)
#define MAX_FACTION_VALUE  (1000)
#define MIN_FACTION_VALUE  (-1000)
#define AGGR_FACTION_VALUE (-400)


int range_faction( const int factionValue );

void set_default_faction (MOB_INDEX_DATA* mob);
//FACTION_DATA* get_neutral_faction();
int get_neutral_faction();

FACTION_DATA* get_faction(const char* name);
int get_faction_id( FACTION_DATA *faction );
int get_faction_id( const char *name );
int get_race_faction_id( const int raceId );
FACTION_DATA *get_race_faction( const int raceId );

void update_faction_on_aggressive_move( CHAR_DATA *aggressor, CHAR_DATA *victim );
void update_faction_on_nice_move( CHAR_DATA *niceGuy, CHAR_DATA *helped, const int niceness );
bool check_aggro_faction( CHAR_DATA *aggressor, CHAR_DATA *victim, const bool simple );

void new_save_factions();
void new_load_factions();

#endif
