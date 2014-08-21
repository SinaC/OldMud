#ifndef __ARENA_H__
#define __ARENA_H__

// Arena Macro, SinaC 2000
#define GET_BETTED_ON(ch) ((ch)->pcdata->betted_on)
#define GET_BET_AMT(ch)   ((ch)->pcdata->bet_amt)
//#define IN_BATTLE(ch)   ((IS_SET((ch)->in_room->room_flags, ROOM_ARENA)))
#define IN_BATTLE(ch)     ((IS_SET((ch)->in_room->cstat(flags), ROOM_ARENA)))
#define IN_WAITING(ch)    ((ch)->in_room->vnum == PREP_START || (ch)->in_room->vnum == PREP_END)


// Added by SinaC 2000 for Arena
extern int in_start_arena;
extern int ppl_in_arena;
extern int ppl_challenged;

// Added by SinaC 2000 for arena & challenge
void    load_hall_of_fame  args( ( void ) );
void    start_arena();
void    do_game();
int     num_in_arena();

// Arena commands
DECLARE_DO_FUN( do_chaos );

DECLARE_DO_FUN( do_betarena );
DECLARE_DO_FUN( do_arena );
DECLARE_DO_FUN( do_awho );
DECLARE_DO_FUN( do_challenge );
DECLARE_DO_FUN( do_accept );
DECLARE_DO_FUN( do_decline );
DECLARE_DO_FUN( do_ahall );

#endif
