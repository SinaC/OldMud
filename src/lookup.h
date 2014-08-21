#ifndef _LOOKUP_H_
#define _LOOKUP_H_
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


int     flag_lookup     args( (const char *name, struct flag_type *flag_table) );
int	position_lookup	args( (const char *name) );
int 	sex_lookup	args( (const char *name) );
int 	size_lookup	args( (const char *name) );
int     hometown_lookup args( (const char *name) );
int     material_lookup args( (const char *name) );
//int     race_lookup     args( (const char *name) );
int     race_lookup     args( (const char *name, const bool complete = FALSE) );
int     liq_lookup      args( (const char *name) );
int     weapon_lookup   args( (const char *name) );
int     item_lookup     args( (const char *name) );
int     attack_lookup   args( (const char *name) );
long    wiznet_lookup   args( (const char *name) );
//int     class_lookup    args( (const char *name, int n = -1 ) );
int class_lookup (const char *name, const bool fComplete = FALSE, int n = -1 );
int     god_lookup      args( (const char *name) );
int     ability_lookup    args( (const char *name, const int from = 0 ) );
int     slot_lookup     args( (const int slot  ) );
int     command_lookup  args( (const char *name) );
//int     classtype_lookup args( ( const char *name ) );
int     abilitytype_lookup args( ( const char *name ) );
int     script_type_lookup args( ( const char *name ) );
int     magic_school_lookup args( ( const char *name ) );
int     super_race_lookup args( ( const char *name ) );

#endif

