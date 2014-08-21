#ifndef __OLC_ACT_H__
#define __OLC_ACT_H__


void show_progs(CHAR_DATA *, const char *);

AREA_DATA *get_vnum_area	args ( ( int vnum ) );
void		show_material		args ( ( CHAR_DATA *ch ) );
void		show_liqlist		args ( ( CHAR_DATA *ch ) );
void		show_damlist		args ( ( CHAR_DATA *ch ) );

void init_help_table();
//void update_help_table( const char *name, flag_type *flag );
void update_help_table( const char *name, void *flag );


/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show		);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_name		);
DECLARE_OLC_FUN( aedit_file		);
DECLARE_OLC_FUN( aedit_age		);
/* DECLARE_OLC_FUN( aedit_recall	);       ROM OLC */
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_vnum		);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_uvnum		);
DECLARE_OLC_FUN( aedit_credits		);
// Added by SinaC 2000 for teleport, removed by SinaC 2003 scripts can do that
//DECLARE_OLC_FUN( aedit_teleport         );


/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN( redit_show		);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name		);
DECLARE_OLC_FUN( redit_desc		);
DECLARE_OLC_FUN( redit_ed		);
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_east		);
DECLARE_OLC_FUN( redit_west		);
DECLARE_OLC_FUN( redit_up		);
DECLARE_OLC_FUN( redit_down		);

DECLARE_OLC_FUN( redit_northeast	); // Added by SinaC 2003
DECLARE_OLC_FUN( redit_northwest	);
DECLARE_OLC_FUN( redit_southeast	);
DECLARE_OLC_FUN( redit_southwest	);

DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_rlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_heal		);
DECLARE_OLC_FUN( redit_mana		);
// Added by SinaC 2001 for mental user
DECLARE_OLC_FUN( redit_psp		);
DECLARE_OLC_FUN( redit_clan		);
DECLARE_OLC_FUN( redit_owner		);
DECLARE_OLC_FUN( redit_guild		);
DECLARE_OLC_FUN( redit_maxsize		);
// Added by SinaC 2001
DECLARE_OLC_FUN( redit_flag             );
DECLARE_OLC_FUN( redit_sector           );
// Added by SinaC 2003
DECLARE_OLC_FUN( redit_program		);
DECLARE_OLC_FUN( redit_repop_time       );
// SinaC 2003
DECLARE_OLC_FUN( redit_addaffect       );
DECLARE_OLC_FUN( redit_delaffect       );
DECLARE_OLC_FUN( redit_showaffect      );
DECLARE_OLC_FUN( redit_setaffect       );

/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN( oedit_show		);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_name		);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_long		);
DECLARE_OLC_FUN( oedit_addaffect	);
DECLARE_OLC_FUN( oedit_addapply		);
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);  /* ROM */
DECLARE_OLC_FUN( oedit_weight		);
DECLARE_OLC_FUN( oedit_cost		);
DECLARE_OLC_FUN( oedit_ed		);

DECLARE_OLC_FUN( oedit_extra            );  /* ROM */
DECLARE_OLC_FUN( oedit_wear             );  /* ROM */
DECLARE_OLC_FUN( oedit_type             );  /* ROM */
DECLARE_OLC_FUN( oedit_affect           );  /* ROM */
DECLARE_OLC_FUN( oedit_material		);  /* ROM */
DECLARE_OLC_FUN( oedit_level            );  /* ROM */
DECLARE_OLC_FUN( oedit_condition        );  /* ROM */
DECLARE_OLC_FUN( oedit_program		);
// Added by SinaC 2003
DECLARE_OLC_FUN( oedit_size             );
DECLARE_OLC_FUN( oedit_addrestriction   );
DECLARE_OLC_FUN( oedit_delrestriction   );
DECLARE_OLC_FUN( oedit_addskillupgrade  );
DECLARE_OLC_FUN( oedit_delskillupgrade  );
DECLARE_OLC_FUN( oedit_points           );
DECLARE_OLC_FUN( oedit_showaffect      );
DECLARE_OLC_FUN( oedit_setaffect       );


/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN( medit_show		);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_name		);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long		);
DECLARE_OLC_FUN( medit_shop		);
DECLARE_OLC_FUN( medit_desc		);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_spec		);
DECLARE_OLC_FUN( medit_program		);

DECLARE_OLC_FUN( medit_sex		);  /* ROM */
DECLARE_OLC_FUN( medit_act		);  /* ROM */
DECLARE_OLC_FUN( medit_affect		);  /* ROM */
DECLARE_OLC_FUN( medit_ac		);  /* ROM */
DECLARE_OLC_FUN( medit_form		);  /* ROM */
DECLARE_OLC_FUN( medit_part		);  /* ROM */
DECLARE_OLC_FUN( medit_imm		);  /* ROM */
DECLARE_OLC_FUN( medit_res		);  /* ROM */
DECLARE_OLC_FUN( medit_vuln		);  /* ROM */
DECLARE_OLC_FUN( medit_material		);  /* ROM */
DECLARE_OLC_FUN( medit_off		);  /* ROM */
DECLARE_OLC_FUN( medit_size		);  /* ROM */
DECLARE_OLC_FUN( medit_hitdice		);  /* ROM */
DECLARE_OLC_FUN( medit_manadice		);  /* ROM */
// Added by SinaC 2001 for mental user
DECLARE_OLC_FUN( medit_pspdice		);  /* ROM */
DECLARE_OLC_FUN( medit_damdice		);  /* ROM */
DECLARE_OLC_FUN( medit_race		);  /* ROM */
DECLARE_OLC_FUN( medit_position		);  /* ROM */
DECLARE_OLC_FUN( medit_gold		);  /* ROM */
DECLARE_OLC_FUN( medit_hitroll		);  /* ROM */
DECLARE_OLC_FUN( medit_damtype		);  /* ROM */
DECLARE_OLC_FUN( medit_group		);  /* ROM */
// Added by SinaC 2000 for mobile class
DECLARE_OLC_FUN( medit_classes          );
// Added by SinaC 2000 for etho
DECLARE_OLC_FUN( medit_etho             );
// Added by SinaC 2001
DECLARE_OLC_FUN( medit_easy             );
DECLARE_OLC_FUN( medit_normal           );
DECLARE_OLC_FUN( medit_hard             );
DECLARE_OLC_FUN( medit_affect2		);
DECLARE_OLC_FUN( medit_modify_race      );
DECLARE_OLC_FUN( medit_faction          );
// General affect edition, SinaC 2003
DECLARE_OLC_FUN( medit_addaffect       );
DECLARE_OLC_FUN( medit_delaffect       );
DECLARE_OLC_FUN( medit_showaffect      );
DECLARE_OLC_FUN( medit_setaffect       );


// Added by SinaC 2000
// Deletion Prototypes
DECLARE_OLC_FUN( redit_delete           );
DECLARE_OLC_FUN( oedit_delete           );
DECLARE_OLC_FUN( medit_delete           );

// Copying Prototypes
DECLARE_OLC_FUN( redit_copy             );
DECLARE_OLC_FUN( oedit_copy             );
DECLARE_OLC_FUN( medit_copy             );

#endif
