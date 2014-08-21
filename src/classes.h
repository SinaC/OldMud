#ifndef _CLASSES_H_
#define _CLASSES_H_

// Removed by SinaC 2001
//int class_mult(CHAR_DATA *ch);

int class_hpmin(long c);
int class_hpmax(long c);

int class_hasattr(CHAR_DATA *ch,int attr);
int class_fMana(long c);
// Added by SinaC 2001 for mental user
int class_fPsp(long c);

int class_thac0_00(long c);
int class_thac0_32(long c);

int class_firstclass(long c);
int class_ismulti(long c);
const char * class_whoname(long c);
const char * class_name(long c);

// Modified by SinaC 2001
//int class_skilllevel(long c,int sn);
int class_abilitylevel(CHAR_DATA *ch,int sn);
int class_abilityrating(CHAR_DATA *ch,int sn, int lvl);
int class_grouprating(const long c, const int sn);
int class_abilityadept(long c);

// Added by SinaC 2000
int class_count( long c );

// Added by SinaC 2001
//int class_max_casting_rule( CHAR_DATA *ch );
casting_rule_type classes_casting_rule( CHAR_DATA *ch, const int sn );

// Added by SinaC 2003
int god_grouprating( CHAR_DATA *ch, int gn );
int class_gainabilityrating( CHAR_DATA *ch, int sn, int lvl );
bool group_available_every_gods( int gn );

bool check_guild_room( int guild, long classes );

bool isWildable( long classes );

DECLARE_DO_FUN( do_multiclass );
DECLARE_DO_FUN( do_specialize );

#endif

