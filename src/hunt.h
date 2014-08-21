#ifndef __HUNT_H__
#define __HUNT_H__

bool    hunt_victim      args( ( CHAR_DATA *ch ) );
// Added by SinaC 2000
void    remove_hunter    args( ( CHAR_DATA *ch ) );
int     find_path        args( ( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, 
				 int depth, int in_zone, char *outpath ) );


DECLARE_DO_FUN( do_track );
DECLARE_DO_FUN( do_path );
DECLARE_DO_FUN( do_hunt );

#endif
