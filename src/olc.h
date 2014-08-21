#ifndef __OLC_H__
#define __OLC_H__

/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */


/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r" \
		"     Port a ROM 2.4 v1.00\n\r"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4b4a\n\r"	\
                "     Por Birdie (itoledo@ramses.centic.utem.cl)\n\r"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\n\r" \
		"     Current Version : 1.5a - Mar 9, 1997\n\r"		
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"




// New typedefs.
typedef	bool OLC_FUN		args( ( CHAR_DATA *ch, const char *argument ) );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun



// Connected states for editor.
#define ED_AREA   (1)
#define ED_ROOM   (2)
#define ED_OBJECT (3)
#define ED_MOBILE (4)



// Interpreter Prototypes
void    aedit           args( ( CHAR_DATA *ch, const char *argument ) );
void    redit           args( ( CHAR_DATA *ch, const char *argument ) );
void    medit           args( ( CHAR_DATA *ch, const char *argument ) );
void    oedit           args( ( CHAR_DATA *ch, const char *argument ) );




// OLC Constants
#define MAX_MOB	1		/* Default maximum number for resetting mobs */




// Structure for an OLC editor command.
 
// Oxtal -- Funny, the guy wrote : char * const
struct olc_cmd_type {
    const char *	name;
    OLC_FUN *		olc_fun;
};




// Structure for an OLC editor startup command.
struct	editor_cmd_type {
    const char *	name;
    DO_FUN *		do_fun;
};




// Utils.
AREA_DATA *get_area_data	args ( ( int vnum ) );
void add_reset			args ( ( ROOM_INDEX_DATA *room, 
				         RESET_DATA *pReset, int index ) );




// Interpreter Table Prototypes
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];


// Editor Commands.
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_olc		);
DECLARE_DO_FUN( do_alist	);
DECLARE_DO_FUN( do_resets	);
DECLARE_DO_FUN( do_olchelp      );


// General Functions
bool show_commands		args ( ( CHAR_DATA *ch, const char *argument ) );
bool show_help			args ( ( CHAR_DATA *ch, const char *argument ) );
bool edit_done			args ( ( CHAR_DATA *ch ) );
bool show_version		args ( ( CHAR_DATA *ch, const char *argument ) );


// Macros

/* ROM OLC */
#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )

#define IS_BUILDER(ch, Area)	( !IS_SWITCHED( ch ) &&			  \
				( ch->pcdata->security >= Area->security  \
				|| strstr( Area->builders, ch->name )	  \
				|| strstr( Area->builders, "All" ) ) )

// Return pointers to what is being edited.
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )

bool	run_olc_editor	args( ( DESCRIPTOR_DATA *d ) );
char	*olc_ed_name	args( ( CHAR_DATA *ch ) );
char	*olc_ed_vnum	args( ( CHAR_DATA *ch ) );



bool OLC_EDIT( CHAR_DATA *ch, OLCEditable *edit );
void OLC_EDIT_DONE( CHAR_DATA *ch );
bool CAN_EDIT( CHAR_DATA *ch, OLCEditable *edit );
const char *editor_name( OLCEditable *edit );

#endif
