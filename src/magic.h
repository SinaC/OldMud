#ifndef _MAGIC_H_
#define _MAGIC_H_

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

extern char target_name[MAX_STRING_LENGTH];
extern char add_target_name[MAX_STRING_LENGTH];
extern bool spell_failed;

bool check_dispel  args( ( int dis_level, CHAR_DATA *victim, int sn) );

//int	find_spell	args( ( CHAR_DATA *ch, const char *name) );
int 	mana_cost 	args( (CHAR_DATA *ch, int min_mana, int level) );
// Added by SinaC 2001
int 	psp_cost 	args( (CHAR_DATA *ch, int min_psp, int level) );
bool	saves_spell	args( ( int level, CHAR_DATA *victim, int dam_type ) );
void    obj_cast_spell  args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj) );
//bool    saves_dispel   args( ( const int dis_level, const int spell_level, const int duration ) );
bool    saves_dispel   args( ( const int dis_level, AFFECT_DATA *paf, const int spell_level = 0, const int duration = 0 ) );

DECLARE_DO_FUN( do_spells );
DECLARE_DO_FUN( do_cast );

#endif
