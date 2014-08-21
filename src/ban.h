#ifndef __BAN_H__
#define __BAN_H__


bool	check_ban	args( ( const char *site, int type) );
void new_load_bans();
void new_save_bans();


DECLARE_DO_FUN( do_allow );
DECLARE_DO_FUN( do_ban );
DECLARE_DO_FUN( do_permban );

#endif
