#ifndef __ACT_OBJ_H__
#define __ACT_OBJ_H__

bool    can_loot	args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void    get_obj         args( ( CHAR_DATA *ch, OBJ_DATA *obj,
				OBJ_DATA *container ) );
// added by SinaC 2000
//bool HasItemGivingFlag  args( ( CHAR_DATA *ch, int flag ) );
bool HasItemGivingFlag  args( ( CHAR_DATA *ch, long flag ) );
bool imprint_spell      args( ( int sn, int level, CHAR_DATA *ch, OBJ_DATA *vo ) );

void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );


DECLARE_DO_FUN( do_donate );
DECLARE_DO_FUN( do_get );
DECLARE_DO_FUN( do_put );
DECLARE_DO_FUN( do_drop );
DECLARE_DO_FUN( do_give );
DECLARE_DO_FUN( do_fill );
DECLARE_DO_FUN( do_pour );
DECLARE_DO_FUN( do_drink );
DECLARE_DO_FUN( do_eat );
DECLARE_DO_FUN( do_wear );
DECLARE_DO_FUN( do_remove );
DECLARE_DO_FUN( do_sacrifice );
DECLARE_DO_FUN( do_quaff );
DECLARE_DO_FUN( do_buy );
DECLARE_DO_FUN( do_list );
DECLARE_DO_FUN( do_sell );
DECLARE_DO_FUN( do_value );
DECLARE_DO_FUN( do_pull );
DECLARE_DO_FUN( do_push );

// Skills
DECLARE_DO_FUN( do_second );
DECLARE_DO_FUN( do_third_wield );
DECLARE_DO_FUN( do_fourth_wield );
#endif
