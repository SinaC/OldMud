#ifndef __ACT_WIZ_H__
#define __ACT_WIZ_H__

void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone);
void recursive_clone2(OBJ_DATA *obj, OBJ_DATA *clone);
void info		args( (const char * argument) );
void wiznet		args( (const char *string, CHAR_DATA *ch, OBJ_DATA *obj,
			       long flag, long flag_skip, int min_level ) );
DECLARE_DO_FUN(do_mload);
DECLARE_DO_FUN(do_oload);
DECLARE_DO_FUN(do_slookup);
DECLARE_DO_FUN(do_outfit);
DECLARE_DO_FUN(do_advance);

DECLARE_DO_FUN( do_at );
DECLARE_DO_FUN( do_xpbonus );
DECLARE_DO_FUN( do_smote );
DECLARE_DO_FUN( do_bamfin );
DECLARE_DO_FUN( do_bamfout );
DECLARE_DO_FUN( do_pardon );
DECLARE_DO_FUN( do_echo );
DECLARE_DO_FUN( do_recho );
DECLARE_DO_FUN( do_zecho );
DECLARE_DO_FUN( do_pecho );
DECLARE_DO_FUN( do_transfer );
DECLARE_DO_FUN( do_goto );
DECLARE_DO_FUN( do_violate );
DECLARE_DO_FUN( do_reboo );
DECLARE_DO_FUN( do_reboot );
DECLARE_DO_FUN( do_shutdow );
DECLARE_DO_FUN( do_shutdown );
DECLARE_DO_FUN( do_switch );
DECLARE_DO_FUN( do_return );
DECLARE_DO_FUN( do_clone );
DECLARE_DO_FUN( do_load );
DECLARE_DO_FUN( do_purge );
DECLARE_DO_FUN( do_trust );
DECLARE_DO_FUN( do_restore );
DECLARE_DO_FUN( do_peace );
DECLARE_DO_FUN( do_wizlock );
DECLARE_DO_FUN( do_newlock );
DECLARE_DO_FUN( do_sockets );
DECLARE_DO_FUN( do_invis );
DECLARE_DO_FUN( do_incognito );
DECLARE_DO_FUN( do_holylight );
DECLARE_DO_FUN( do_prefi );
DECLARE_DO_FUN( do_prefix );
DECLARE_DO_FUN( do_spy );

#endif
