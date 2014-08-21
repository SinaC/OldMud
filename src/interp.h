#ifndef __INTERP_H__
#define __INTERP_H__

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/* this is a listing of all the commands and command related data */

#define COM_INGORE	1


/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

//extern const char *logName[];
//int getLogName( const char *name );


/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type: OLCEditable {
  const char * 	name;
  DO_FUN *	do_fun;
  int		position;
  int		level;
  int		log;
  int           show;
};

struct	cmd_type_init {
  const char * 	name;
  DO_FUN *	do_fun;
};

/* the command table itself */
extern	struct	cmd_type	*cmd_table;
// Added by SinaC 2001
extern	const	struct	cmd_type_init	cmd_table_init[];
extern          char                    last_command[MAX_STRING_LENGTH];


void	interpret	args( ( CHAR_DATA *ch, const char *argument ) );
bool	is_number	args( ( const char *arg ) );
int	number_argument	args( ( const char *argument, char *arg ) );
int	mult_argument	args( ( const char *argument, char *arg) );
const char * one_argument	args( ( const char *argument, char *arg_first ) );
const char * one_optional_number_argument( const char *argument, char *arg_first ) ;
const char * no_lower_one_argument args( ( const char *argument, char *arg_first ) );


void new_save_disabled args( ( void ) );
void new_load_disabled args( ( void ) );


DECLARE_DO_FUN( do_commands );
DECLARE_DO_FUN( do_wizhelp );
DECLARE_DO_FUN( do_disable );
DECLARE_DO_FUN( do_disable_plr );

#endif
