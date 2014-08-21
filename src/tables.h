#ifndef _TABLES_H_
#define _TABLES_H_

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

// New tables
extern struct flag_type *brew_component_flags;
// New tables 2003
extern struct flag_type mob_use_flags[];
extern struct flag_type race_type_flags[];
extern struct flag_type log_flags[];
extern struct flag_type class_type_flags[];
extern struct flag_type class_choosable_flags[];
extern struct flag_type ability_type_flags[];
extern struct flag_type affect_data_flags[];

/* game tables */
extern  const   struct  position_type   target_table[];
extern	const	struct	position_type	position_table[];
extern	const	struct	sex_type	sex_table[];
extern	const	struct	size_type	size_table[];

/* flag tables */
extern	struct	flag_type	plr_flags[];
extern	struct	flag_type	comm_flags[];
extern	struct	flag_type	weapon_flags[];
extern	struct	flag_type	portal_flags[];


struct flag_type {
  const char *name;
  long bit;
  bool settable;
};

struct position_type {
    const char *name;
    const char *short_name;
};

struct sex_type {
    const char *name;
};

struct size_type {
    const char *name;
};

// Added by SinaC 2003 for automatic do_wear, checking part
struct wear_item_type {
  const char *wear_loc_name;
  const char *wear_string_TO_CHAR; // string send to ch when he/she wears an item on this wear location
  const char *wear_string_TO_ROOM; // string send to room when someone wears an item on this wear location
  int wear_loc_flag; // wear location flag: primary key of the table
  int part_needed; // <-- not yet implemented, new part system (not bit anymore)
};

// new tables, SinaC 2003  NOT YET USED
extern  struct  wear_item_type  wear_item_table[];

#endif
