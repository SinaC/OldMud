#ifndef __ACT_MOVE_H__
#define __ACT_MOVE_H__

extern const int rev_dir [];


/* act_move.c */
// Modified by SinaC 2003
void	move_char	args( ( CHAR_DATA *ch, int door, const bool follow, const bool climbing,
				const bool passDoor ) );
int	find_door	args( ( CHAR_DATA *ch, const char *arg ) );
// return recall point, hall, morgue, donation and mud school, SinaC 2003
ROOM_INDEX_DATA *get_recall_room( CHAR_DATA *ch );
ROOM_INDEX_DATA *get_hall_room( CHAR_DATA *ch );
ROOM_INDEX_DATA *get_morgue_room( CHAR_DATA *ch );
ROOM_INDEX_DATA *get_donation_room( CHAR_DATA *ch );
ROOM_INDEX_DATA *get_school_room( CHAR_DATA *ch );


DECLARE_DO_FUN( do_stand );
DECLARE_DO_FUN( do_wake );
DECLARE_DO_FUN( do_sleep );
DECLARE_DO_FUN( do_open );
DECLARE_DO_FUN( do_close );
DECLARE_DO_FUN( do_north );
DECLARE_DO_FUN( do_west );
DECLARE_DO_FUN( do_south );
DECLARE_DO_FUN( do_east );
DECLARE_DO_FUN( do_up );
DECLARE_DO_FUN( do_down );
DECLARE_DO_FUN( do_northeast );
DECLARE_DO_FUN( do_northwest );
DECLARE_DO_FUN( do_southeast );
DECLARE_DO_FUN( do_southwest );
DECLARE_DO_FUN( do_lock );
DECLARE_DO_FUN( do_unlock );
DECLARE_DO_FUN( do_rest );
DECLARE_DO_FUN( do_sit );
DECLARE_DO_FUN( do_visible );
DECLARE_DO_FUN( do_hometown );
DECLARE_DO_FUN( do_train );
DECLARE_DO_FUN( do_knock );
DECLARE_DO_FUN( do_jog );
DECLARE_DO_FUN( do_dive );
#endif
