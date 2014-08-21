#ifndef _DB_H_
#define _DB_H_

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

/* vals from db.c */
/*
 * Memory management.
 * Increase MAX_STRING if you have too.
 * Tune the others only if you understand what you're doing.
 */
//#define			MAX_STRING	3200000 /* 1150976 */ /* OLD : 1413120 */
#define			MAX_STRING	5120000 
#define			MAX_PERM_BLOCK	131072
#define			MAX_MEM_LIST	11


extern int smaug_version;
extern FILE             *fpArea;
extern char             strArea[MAX_INPUT_LENGTH];  
extern const char             * help_greeting;
extern bool             double_xp;
extern bool             fBootDb;
extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA 	* mob_index_hash          [MAX_KEY_HASH];
extern OBJ_INDEX_DATA 	* obj_index_hash          [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int  		top_affect;
extern int		top_ed; 
extern int              top_room;
extern int              top_reset;
extern int              top_area;
extern int              top_exit;

extern AREA_DATA 	* area_first;

// Added by SinaC 2001
extern bool             reset_always;

/* from db2.c */
extern int	social_count;

/* conversion from db.h */
void	convert_mob(MOB_INDEX_DATA *mob);
void	convert_obj(OBJ_INDEX_DATA *obj);

/* macro for flag swapping */
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))

/* Magic number for memory allocation */
#define MAGIC_NUM 52571214

/* func from db.c */
void assign_area_vnum( int vnum );                    /* OLC */

/* db.c */
// Added by SinaC 2000 for 'jog'
void    free_runbuf     args( ( DESCRIPTOR_DATA *d ) );
// end
void	boot_db		args( ( bool fState ) );
void	area_update	args( ( void ) );
CHAR_DATA *	create_mobile	args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile	args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OBJ_DATA *	create_object	args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clone_object	args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char	args( ( CHAR_DATA *ch ) );
const char *	get_extra_descr	args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MOB_INDEX_DATA *	get_mob_index	args( ( int vnum ) );
OBJ_INDEX_DATA *	get_obj_index	args( ( int vnum ) );
ROOM_INDEX_DATA *	get_room_index	args( ( int vnum ) );
void	append_file	args( ( CHAR_DATA *ch, const char *file, const char *str ) );

void	bug		args( ( const char *str, ...) ) __attribute__ ((format (printf, 1, 2)));
/* Modified by Oxtal -- now bug behaves like printf exactly */

void	log_string	args( ( const char *str ) );
#define logf log_stringf
void    log_stringf( const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

void	reset_area      args( ( AREA_DATA * pArea ) );
void	reset_room	args( ( ROOM_INDEX_DATA *pRoom ) );

void    new_reset       args( ( ROOM_INDEX_DATA *pR, RESET_DATA *pReset ) );

long	flag_convert	args( ( char letter) );
long	fread_number	args( ( FILE *fp ) );
long 	fread_flag	args( ( FILE *fp ) );
long    flag_to_long    args( ( const char *s ) );
const char *	fread_string	args( ( FILE *fp ) );
char *  fread_line      args( ( FILE *fp ) );
const char *  fread_string_eol args(( FILE *fp ) );
void	fread_to_eol	args( ( FILE *fp ) );
char *	fread_word	args( ( FILE *fp ) );
bool	str_cmp		args( ( const char *astr, const char *bstr ) );
bool	str_prefix	args( ( const char *astr, const char *bstr ) );
bool	str_infix	args( ( const char *astr, const char *bstr ) );
bool	str_suffix	args( ( const char *astr, const char *bstr ) );
void    skip_till_(FILE *fp, char c );
void    skip_till_0( FILE *fp );
void    skip_till_cardinal0( FILE *fp );
long    number_mm       args( ( void ) );
char	fread_letter	args( ( FILE *fp ) );

const char * parse_name	args(( const char * name));
const char *	fread_string_upper	args( ( FILE *fp ) );
const char *	fread_string_lower	args( ( FILE *fp ) );
const char * fread_string_eol_trim args( ( FILE *fp ) );


// SinaC 2003
void load_one_area( FILE *fp );
void convert_act_flags( MOB_INDEX_DATA *pMob );
// SinaC 2003, calcutate base stat value for mob in function of its level/ACT/OFF/SIZE
// Only for STR, INT, WIS, DEX and CON
int mob_base_stat( const int stat, CHAR_DATA *mob );
float   fread_float     args( ( FILE *fp ) );
void fix_parts( MOB_INDEX_DATA *pMob );


CHAR_DATA * create_mobile_from_corpse args( ( OBJ_DATA *corpse, MOB_INDEX_DATA *pMobIndex ) );





// Added by SinaC 2003 for repop time
// 44640 = 30 * 24 * 60  number of minutes in a month
#define MAX_REPOP_TIME         (44640)
#define BASE_REPOP_TIME        (15)
#define BASE_REPOP_TIME_PEOPLE (30)
#define MIN_REPOP_TIME         (3)



const int BIG_SIZE = 2*1024*1024;
extern char large_buffer[BIG_SIZE];


DECLARE_DO_FUN( do_areas );
DECLARE_DO_FUN( do_memory );
DECLARE_DO_FUN( do_dump );

#endif

