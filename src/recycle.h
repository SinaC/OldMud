#ifndef _RECYCLE_H_
#define _RECYCLE_H_

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

/* externs */
extern char str_empty[1];
extern int mobile_count;
extern int object_count;// SinaC 2003

/* note recycling */
#define ND NOTE_DATA
ND	*new_note args( (void) );
#undef ND

/* ban data recycling */
#define BD BAN_DATA
BD	*new_ban args( (void) );
#undef BD

/* descriptor recycling */
#define DD DESCRIPTOR_DATA
DD	*new_descriptor args( (void) );
#undef DD

/* char gen data recycling */
#define GD GEN_DATA
GD 	*new_gen_data args( (void) );
#undef GD

/* extra descr recycling */
#define ED EXTRA_DESCR_DATA
ED	*new_extra_descr args( (void) );
#undef ED

/* affect recycling */
#define AD AFFECT_DATA
AD	*new_affect args( (void) );
#undef AD

// Added by SinaC 2000 for object restrictions
RESTR_DATA *new_restriction args( (void) );

// Added by SinaC 2000 for object skill/spell upgrade
ABILITY_UPGRADE *new_ability_upgrade args( (void) );

/* object recycling */
#define OD OBJ_DATA
OD	*new_obj args( (void) );
void	free_obj args( (OBJ_DATA *obj) );
#undef OD

/* character recyling */
#define CD CHAR_DATA
#define PD PC_DATA
CD	*new_char args( (void) );
void	free_char args( (CHAR_DATA *ch) );
PD	*new_pcdata args( (void) );
#undef PD
#undef CD


/* mob id and memory procedures */
#define MD MEM_DATA
long 	get_pc_id args( (void) );
long	get_mob_id args( (void) );
#undef MD

/* buffer procedures */

BUFFER	*new_buf args( (void) );
bool	add_buf args( (BUFFER *buffer, const char *string) );
void	clear_buf args( (BUFFER *buffer) );
char	*buf_string args( (BUFFER *buffer) );

#endif
