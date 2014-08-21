#ifndef __HANDLER_H__
#define __HANDLER_H__

// These bits are set in recompute and avoid computing tons of non-useful things
//  such as: unarmed/unarmored, ability upgrade, ...
#define OPTIMIZING_BIT_UNARMED_UNARMORED   (A)
#define OPTIMIZING_BIT_HAS_ABILITY_UPGRADE (B)
// This bit is set if mob's affected has changed (affect_strip, affect_remove, affect_to_char, affect_join)
#define OPTIMIZING_BIT_MODIFIED_AFFECT     (C)

#define IS_UNARMED_UNARMORED( ch ) (IS_SET( ch->optimizing_bit, OPTIMIZING_BIT_UNARMED_UNARMORED ))
#define HAS_ABILITY_UPGRADE( ch ) (IS_SET( ch->optimizing_bit, OPTIMIZING_BIT_HAS_ABILITY_UPGRADE ))



// Added by SinaC 2001
void    change_race     args( ( CHAR_DATA *victim, int race, int timer, int level, int sn, int cast ) );
void    chardata_to_str args( ( CHAR_DATA *ch, CHAR_DATA *victim, char *buf, CHAR_DATA *&victim2 ) );

void    check_rebirth   args( ( CHAR_DATA *ch ) );

int     check_position  args( ( const char *pos ) );
int     check_target    args( ( const char *tar ) );
bool    exist_slot      args( ( int slot ) );
// Added by SinaC 2000 for stay_death item
void    transfer_obj_stay_death args( ( CHAR_DATA *ch, OBJ_DATA *corpse ) );
// Added by SinaC 2000 for mobile class
int     get_random_ability args( ( CHAR_DATA *ch, int target ) );
// Added by SinaC 2000 for casting ability level
int     get_casting_level args( ( CHAR_DATA *cch, int sn ) );
// Added by SinaC 2001 to allow certain ability to mobs
int     get_random_mob_usable_ability args( ( CHAR_DATA *ch ) );
// Added by SinaC 2001 to have a simple way to do get_ability
int     get_ability_simple args( ( CHAR_DATA *ch, int sn ) );
// Added by SinaC 2001 to get random race ability
int     get_random_race_ability args( ( CHAR_DATA *ch ) );

/* Oxtal */
void recompute(CHAR_DATA *ch);
void recompobj(OBJ_DATA *ch);
// Added by SinaC 2001 for room affects
void recomproom(ROOM_INDEX_DATA *ch);

void    affect_copy     args( ( AFFECT_DATA *newaf, AFFECT_DATA *oldaf ) );
AFFECT_DATA *affect_find args( (AFFECT_DATA *paf, int sn));
void	affect_check	args( (CHAR_DATA *ch, int where, int vector) );
int	count_users	args( (OBJ_DATA *obj) );
void 	deduct_cost	args( (CHAR_DATA *ch, int cost) );
void	affect_enchant	args( (OBJ_DATA *obj) );
int 	check_immune	args( (CHAR_DATA *ch, int dam_type) );
int	weapon_type	args( ( const char *name) );
bool	is_clan		args( (CHAR_DATA *ch) );
bool	is_same_clan	args( (CHAR_DATA *ch, CHAR_DATA *victim));
bool	is_old_mob	args ( (CHAR_DATA *ch) );
int	get_ability	args( ( CHAR_DATA *ch, int sn ) );
/* Added by Sinac 1997 */
bool    get_raceability   args( ( CHAR_DATA *ch, int sn ) );
// SinaC 2000
CHAR_DATA *    get_char        args( ( CHAR_DATA *ch ) );
// Modified by SinaC 2003
//bool    fumble_obj      args( ( CHAR_DATA *victim, OBJ_DATA *obj_drop, int level, bool drop ) );
bool    fumble_obj      args( ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj_drop, int level, bool drop ) );
bool    is_made_flesh   args( ( CHAR_DATA *victim ) );
// Added by SinaC 2000 for clan ability
bool    get_clanability   args( ( CHAR_DATA *victim, int sn ) );
// Added by SinaC 2000 for god skill
//bool    get_godskill    args( ( CHAR_DATA *victim, int sn ) );      removed by SinaC 2003
// Added by SinaC 2001 for etho/alignement restriction based on race
bool    check_etho_align args( ( CHAR_DATA *ch, int etho, int align ) );
// Added by SinaC 2001 for class restriction based on race
bool    check_class_race args( ( CHAR_DATA *ch, int iClass ) );
// Added by SinaC 2001 for god
bool    check_god       args( ( CHAR_DATA *ch, int who ) );
bool    check_class_god args( ( int iClass, int who ) );

//int	get_weapon_sn	args( ( CHAR_DATA *ch, bool secondary ) );
int	get_weapon_sn	args( ( CHAR_DATA *ch, int whichWield ) ); // SinaC 2003
int     get_weapon_sn   args( (CHAR_DATA *ch, OBJ_DATA *wield ) ); // SinaC 2003
int	get_weapon_ability args(( CHAR_DATA *ch, int sn ) );
int     get_age         args( ( CHAR_DATA *ch ) );
int	get_trust	args( ( CHAR_DATA *ch ) );
int 	get_max_train	args( ( CHAR_DATA *ch, int stat ) );
int	can_carry_n	args( ( CHAR_DATA *ch ) );
int	can_carry_w	args( ( CHAR_DATA *ch ) );
bool	is_name		args( ( const char *str, const char *namelist ) );
bool	is_exact_name	args( ( const char *str, const char *namelist ) );
void	affect_to_char	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_to_obj	args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
// Added by SinaC 2001 for room affects
void    affect_to_room  args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf ) );

void	affect_remove	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove_obj args( (OBJ_DATA *obj, AFFECT_DATA *paf ) );
// Added by SinaC 2001 for room affects
void    affect_remove_room args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf) );

void	affect_strip	args( ( CHAR_DATA *ch, const int sn ) );
// Added by SinaC 2001
void    affect_strip_obj args( ( OBJ_DATA *obj, const int sn ) );
// Added by SinaC 2001 for room affects
void    affect_strip_room args( ( ROOM_INDEX_DATA *room, const int sn ) );

bool	is_affected	args( ( CHAR_DATA *ch, const int sn ) );
// Added by SinaC 2001 for room affects
bool    is_affected_room args( ( ROOM_INDEX_DATA *room, const int sn ) );

void	affect_join	args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void    affect_join_obj args( ( OBJ_DATA *obj, AFFECT_DATA *paf ) );
void    affect_join_room args( ( ROOM_INDEX_DATA *obj, AFFECT_DATA *paf ) );

void	char_from_room	args( ( CHAR_DATA *ch ) );
void	char_to_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args( ( OBJ_DATA *obj ) );
int	apply_ac	args( ( OBJ_DATA *obj, int iWear, int type ) );
OBJ_DATA *	get_eq_char	args( ( CHAR_DATA *ch, int iWear ) );
//void	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
int 	equip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );//SinaC 2003, return value used reset_room
void	unequip_char	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args( ( OBJ_DATA *obj ) );
void	obj_to_room	args( ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj	args( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args( ( OBJ_DATA *obj ) );
void	extract_obj	args( ( OBJ_DATA *obj ) );
void	extract_char	args( ( CHAR_DATA *ch, bool fPull, const bool notInRoom = FALSE ) ); // SinaC 2003
CHAR_DATA *	get_char_room	args( ( CHAR_DATA *ch, const char *argument ) );
CHAR_DATA *	get_char_world	args( ( CHAR_DATA *ch, const char *argument ) );
OBJ_DATA *	get_obj_type	args( ( OBJ_INDEX_DATA *pObjIndexData ) );
OBJ_DATA *	get_obj_list	args( ( CHAR_DATA *ch, const char *argument,
				OBJ_DATA *list ) );
OBJ_DATA *	get_obj_carry	args( ( CHAR_DATA *ch, const char *argument, 
				CHAR_DATA *viewer ) );
//OD *	get_obj_wear	args( ( CHAR_DATA *ch, const char *argument ) );
// Modified by SinaC 2000 to allow player to remove an item they are wearing even
//  if they can't see it
OBJ_DATA *	get_obj_wear	args( ( CHAR_DATA *ch, const char *argument, bool rem ) );

OBJ_DATA *	get_obj_here	args( ( CHAR_DATA *ch, const char *argument ) );
OBJ_DATA *	get_obj_world	args( ( CHAR_DATA *ch, const char *argument ) );
OBJ_DATA *	create_money	args( ( int gold, int silver ) );
int	get_obj_number	args( ( OBJ_DATA *obj ) );
int	get_obj_weight	args( ( OBJ_DATA *obj ) );
int	get_true_weight	args( ( OBJ_DATA *obj ) );
bool	room_is_dark	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	is_room_owner	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private	args( ( ROOM_INDEX_DATA *pRoomIndex ) );
// Added by SinaC 2001
bool    can_see_ooc     args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room	args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj	args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );

// Added by SinaC 2001
CHAR_DATA* find_mob     args( ( MOB_INDEX_DATA *mob ) );

ROOM_INDEX_DATA *	find_location	args( ( CHAR_DATA *ch, const char *arg ) );

bool    is_wearable    args( ( OBJ_INDEX_DATA *obj, int wear_loc ) );
int     find_suitable_wearloc args( ( OBJ_DATA *obj ) );

// Added by SinaC 2003
bool    is_ability_sphere args( ( int sn, const char *sphere_name ) );
int     is_ability_in_sphere args( ( const int sn ) );
void    change_size_obj args( ( OBJ_DATA *obj, int amount ) );
void    knock_off_item  args( ( CHAR_DATA *ch, CHAR_DATA *victim, bool fromEquip ) );
bool    can_remove_obj  args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool    check_phylactery args( ( CHAR_DATA *ch ) );
bool    affect_remove_no_recompute args( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
bool    affect_remove_obj_no_recompute args( ( OBJ_DATA *obj, AFFECT_DATA *paf) );
bool    affect_remove_room_no_recompute args( ( ROOM_INDEX_DATA *room, AFFECT_DATA *paf) );
// mount code
CHAR_DATA *get_mount     args( ( CHAR_DATA *ch ) );
CHAR_DATA *get_mount_master args( ( CHAR_DATA *mount ) );
void    mount_dying_drawback args( ( CHAR_DATA *mount ) );
void    mount_to_master  args( ( CHAR_DATA *mount, CHAR_DATA *master, bool silent ) );
bool    check_strategic_retreat args( ( CHAR_DATA *ch ) );
OBJ_DATA *get_item_here  args( ( CHAR_DATA *ch, const int item_type ) );
int     find_exit        args( ( const char *arg ) );
ROOM_INDEX_DATA *get_room_obj args( ( OBJ_DATA *obj ) );
int convert_etho_align_string_to_align_info args( ( const char *input, align_info &align ) );

CHAR_DATA *get_pc_world( const char *argument );

const char *char_god_name( CHAR_DATA *ch );

// Useful for monk
//bool is_unarmed_and_unarmored( CHAR_DATA *ch ); replaced with a bit OPTIMIZING_BIT_UNARMED_UNARMORED
bool is_pure_body_skilled( const int level, CHAR_DATA *ch );
bool is_diamond_body_skilled( const int level, CHAR_DATA *ch );

// SinaC 2003
bool is_affected_obj( OBJ_DATA *obj, const int sn );
bool check_hometown( CHAR_DATA *ch, int hometown );
void check_mount_class( CHAR_DATA *pet );
bool check_needed_parts( const int parts, const int wear_loc );
bool check_needed_forms( const int forms, const int wear_loc );
const char *needed_parts_string( const int wear_loc );
const char *needed_forms_string( const int wear_loc );
void small_recompute( CHAR_DATA *ch );

#endif
