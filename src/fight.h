#ifndef __FIGHT_H__
#define __FIGHT_H__

/*Oxtal */
//#define IN_BATTLE(ch) ((ch)->in_room->vnum < 100 && (ch)->in_room->vnum >= 4)

bool 	is_safe		args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell	args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update	args( ( void ) );
void	multi_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
void	set_fighting	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
// Added by SinaC 2000
void	one_hit	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt, int whichWield ) );

/*  Removed by SinaC 2000
 *bool    damage_old      args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
 *                               int dt, int, bool show ) );
 */
// Added by SinaC 2000 for sudden_death
void    sudden_death    args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

void	update_pos	args( ( CHAR_DATA *victim ) );
void	stop_fighting	args( ( CHAR_DATA *ch, bool fBoth ) );
void	check_killer	args( ( CHAR_DATA *ch, CHAR_DATA *victim) );

// Added by SinaC 2000
void	raw_kill	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
// Modified by SinaC 2000 and 2003
//void	make_corpse	args( ( CHAR_DATA *ch, OBJ_DATA *&corpse, CHAR_DATA *killer ) );
void	make_corpse	args( ( CHAR_DATA *ch, OBJ_DATA *&corpse, CHAR_DATA *killer, const long parts ) );
// SinaC 2003: toRoom
void    dam_message 	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				int dt, bool immune, bool toRoom ) );

// Added by SinaC 2003
bool silent_is_safe( CHAR_DATA *ch, CHAR_DATA *victim );
bool silent_is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area );


// Added by SinaC 2001 for some cool spell effect additions
bool    check_armor_absorb args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
//bool    check_shroud    args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
bool    check_shroud    args( ( CHAR_DATA *victim, CHAR_DATA *ch, const int dam ) );

// Added by SinaC 2003: additional affects from funky weapon
void additional_hit_affect( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield );

bool check_lycanthropy( CHAR_DATA *ch, CHAR_DATA *victim, const int dt, OBJ_DATA *wield );

int combat_damage( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int dam_type,
		   bool show, OBJ_DATA *wield );
void	group_gain	args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );


// Constants used by damage and noaggr_damage
#define DAMAGE_NOTDONE (0)
#define DAMAGE_DONE    (1)
#define DAMAGE_DEADLY  (2)


DECLARE_DO_FUN( do_kill );
DECLARE_DO_FUN( do_murde );
DECLARE_DO_FUN( do_murder );
DECLARE_DO_FUN( do_flee );
DECLARE_DO_FUN( do_sla );
DECLARE_DO_FUN( do_slay );

#endif
