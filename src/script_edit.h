#ifndef __SCRIPT_EDIT_H__
#define __SCRIPT_EDIT_H__

// Datas editing constants
extern int SCRIPT_MINIMUM_SECURITY;


#define ED_SCRIPT (1000)


// Datas editing macros
#define CAN_EDIT_SCRIPT(ch) (!IS_SWITCHED(ch) && ch->pcdata->security >= SCRIPT_MINIMUM_SECURITY)

#define EDIT_SCRIPT(Ch, Cl) ( Cl = (CLASS_DATA *)Ch->desc->pEdit )

// Datas command table
extern const struct olc_cmd_type script_edit_table[];

// Interpreter Prototypes
void    script_edit           args( ( CHAR_DATA *ch, const char *argument ) );

// Data saving
DECLARE_DO_FUN( do_script_save );

// Entry Point for editing
DECLARE_DO_FUN( do_script_edit );

// Methods for editing datas
//  Abilities
DECLARE_OLC_FUN( script_edit_show );
DECLARE_OLC_FUN( script_edit_edit_method );
DECLARE_OLC_FUN( script_edit_add_method );
DECLARE_OLC_FUN( script_edit_del_method );
DECLARE_OLC_FUN( script_edit_file );
DECLARE_OLC_FUN( script_edit_name );
DECLARE_OLC_FUN( script_edit_abstract );
DECLARE_OLC_FUN( script_edit_parents );
DECLARE_OLC_FUN( script_edit_update );
DECLARE_OLC_FUN( script_edit_revert );
DECLARE_OLC_FUN( script_edit_definitive );
DECLARE_OLC_FUN( script_edit_copy );
DECLARE_OLC_FUN( script_edit_copy_method );
DECLARE_OLC_FUN( script_edit_create );
DECLARE_OLC_FUN( script_edit_finalize );
DECLARE_OLC_FUN( script_edit_delete );
DECLARE_OLC_FUN( script_edit_show_revert );

#endif
