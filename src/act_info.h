#ifndef __ACT_INFO_H__
#define __ACT_INFO_H__

// Added by SinaC 2000
const char   *num_punct args( ( int foo ) );
void set_title args( ( CHAR_DATA *ch, const char *title ) );
void show_oaffects args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool immortal ) );
const char *small_help_string( const char *argument );


DECLARE_DO_FUN( do_exits );
DECLARE_DO_FUN( do_help );
DECLARE_DO_FUN( do_affects );
DECLARE_DO_FUN( do_autoassist	);
DECLARE_DO_FUN( do_autoexit	);
DECLARE_DO_FUN( do_autogold	);
DECLARE_DO_FUN( do_autolist	);
DECLARE_DO_FUN( do_autoloot	);
DECLARE_DO_FUN( do_autosac	);
DECLARE_DO_FUN( do_autosplit	);
DECLARE_DO_FUN( do_autotitle );

DECLARE_DO_FUN( do_brief );
DECLARE_DO_FUN( do_scroll );
DECLARE_DO_FUN( do_socials );
DECLARE_DO_FUN( do_wizlist );
DECLARE_DO_FUN( do_autoaff );
DECLARE_DO_FUN( do_compact );
DECLARE_DO_FUN( do_combine );
DECLARE_DO_FUN( do_prompt );
DECLARE_DO_FUN( do_noloot );
DECLARE_DO_FUN( do_nofollow );
DECLARE_DO_FUN( do_nosummon );
DECLARE_DO_FUN( do_colour );
DECLARE_DO_FUN( do_wimpy );
DECLARE_DO_FUN( do_config );
DECLARE_DO_FUN( do_look );
DECLARE_DO_FUN( do_read );
DECLARE_DO_FUN( do_examine );
DECLARE_DO_FUN( do_score );
DECLARE_DO_FUN( do_time );
DECLARE_DO_FUN( do_weather );
DECLARE_DO_FUN( do_whois );
DECLARE_DO_FUN( do_who );
DECLARE_DO_FUN( do_count );
DECLARE_DO_FUN( do_inventory );
DECLARE_DO_FUN( do_equipment );
DECLARE_DO_FUN( do_compare );
DECLARE_DO_FUN( do_credits );
DECLARE_DO_FUN( do_where );
DECLARE_DO_FUN( do_consider );
DECLARE_DO_FUN( do_title );
DECLARE_DO_FUN( do_description );
DECLARE_DO_FUN( do_report );
DECLARE_DO_FUN( do_practice );
DECLARE_DO_FUN( do_password );
DECLARE_DO_FUN( do_stance );
DECLARE_DO_FUN( do_showinfo );
DECLARE_DO_FUN( do_autotick );

#endif
