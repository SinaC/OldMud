#ifndef __UPDATE_H__
#define __UPDATE_H__

extern  int     pulse_area;
extern  int     pulse_mobile;
extern  int     pulse_violence;
extern  int     pulse_point;
extern  int     pulse_auction;
extern  int     pulse_hints;
extern  int     pulse_start_arena;
extern  int     pulse_arena;
extern  int     pulse_room;
extern  int     pulse_quake;
extern  int     pulse_double_xp;
extern  int     pulse_double_xp_duration;

extern int         aucstat;
extern CHAR_DATA * aucfrom;
extern OBJ_DATA  * aucobj;
extern int         lastbet;
extern int         betgold;
extern int         betsilver;
extern CHAR_DATA * lastbetter;
extern long ticks_elapsed;


/* update.c */
void	advance_level	args( ( CHAR_DATA *ch, bool hide ) );
// Modified by SinaC 2001
void	gain_exp	args( ( CHAR_DATA *ch, int gain, bool silent ) );
void	gain_condition	args( ( CHAR_DATA *ch, int iCond, int value ) );
void	update_handler	args( ( void ) );

// Added by SinaC 2003, silent_kill replaces sudden_death when victim kills himself
void    silent_kill     args( ( CHAR_DATA *victim, const char *death_msg ) );
int     silent_damage   args( ( CHAR_DATA *victim, int dam, int dam_type ) );
void    silent_update   args( ( CHAR_DATA *victim ) );
// Added by SinaC 2000 for noaggr_damage
// for noaggr_damage return value, check fight.h
int    noaggr_damage   args( ( CHAR_DATA *victim, int dam, int dam_type,
				const char *dam_msg_vict, const char *dam_msg_other,
				const char *death_msg,
				bool showdam ) );

#endif
