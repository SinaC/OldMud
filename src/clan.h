#ifndef _clan_h_
#define _clan_h_

#define CLAN_JUNIOR 0
#define CLAN_SENIOR 1
#define CLAN_DEPUTY 5
#define CLAN_SECOND 9
#define CLAN_LEADER 10

typedef struct clan_type CLAN_TYPE;
typedef struct clan_member_data CLAN_MEMBER_DATA;

void fix_clan_status_enter(const char * name);
void fix_clan_status_leave(CHAR_DATA * ch);

void destroy_memberlist(CLAN_MEMBER_DATA* list);
void update_clan_member(CLAN_MEMBER_DATA *member);
CLAN_MEMBER_DATA * add_clan_member(CLAN_MEMBER_DATA** list, const char * name);
void remove_clan_member(CLAN_MEMBER_DATA** list, const char * name);
CLAN_MEMBER_DATA * find_clan_member(CLAN_MEMBER_DATA* list, const char * name);

void new_load_clans();
void new_save_clans();


struct clan_type {
  int	clan_id;
  const char	*name;
  const char	*who_name;
  int	hall;
  int	recall;
  int   donation; // SinaC 2003
  int   morgue; // SinaC 2003

  bool	independent;
  CLAN_MEMBER_DATA * members;
  CLAN_MEMBER_DATA * petitions;
  CLAN_TYPE *next;

  // Added by SinaC 2000 for a clan ability
  //const char *clan_ability;
  int clan_ability;
};

struct clan_member_data {
  const char  * name;
  int   level;
  int   race;
  int   status;
  long  classes;
  CLAN_MEMBER_DATA * next;    
};

int clan_lookup(const char *name);
const char *lookup_clan_status(int);
struct clan_type *get_clan_table(int);
int parse_clan_status(const char *);
const char *short_lookup_clan_status(int cs);

#define UPPER_MAX_CLAN 50      /* Used in do_who */


// Do functions
DECLARE_DO_FUN( do_myclan );
DECLARE_DO_FUN( do_petition );
DECLARE_DO_FUN( do_clans );
DECLARE_DO_FUN( do_guild );
DECLARE_DO_FUN( do_clanlist );

#endif
