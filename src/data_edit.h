#ifndef __DATA_EDIT_H__
#define __DATA_EDIT_H__

// Datas editing constants
extern int DATA_MINIMUM_SECURITY;


#define ED_ABILITY (10)
#define ED_RACE (11)
#define ED_PCRACE (12)
#define ED_COMMAND (13)
#define ED_PCCLASS (14)
#define ED_GOD (15)
#define ED_LIQUID (16)
#define ED_MATERIAL (17)
#define ED_BREW (18)
#define ED_GROUP (19)
#define ED_FACTION (20)
#define ED_MAGIC_SCHOOL (21)

// Datas editing macros
#define CAN_EDIT_DATA(ch) (!IS_SWITCHED(ch) && ch->pcdata->security >= DATA_MINIMUM_SECURITY)

#define EDIT_ABILITY(Ch, Ability) ( Ability = (ability_type *)Ch->desc->pEdit )
#define EDIT_RACE(Ch, Race) ( Race = (race_type *)Ch->desc->pEdit )
#define EDIT_PCRACE(Ch, Race) ( Race = (pc_race_type *)Ch->desc->pEdit )
#define EDIT_COMMAND(Ch, Command) ( Command = (cmd_type *)Ch->desc->pEdit )
#define EDIT_PCCLASS(Ch, Class) ( Class = (class_type *)Ch->desc->pEdit )
#define EDIT_GOD(Ch, God) ( God = (god_data *)Ch->desc->pEdit )
#define EDIT_LIQUID(Ch, Liq) ( Liq = (liq_type *)Ch->desc->pEdit )
#define EDIT_MATERIAL(Ch, Mat) ( Mat = (material_type *)Ch->desc->pEdit )
#define EDIT_BREW(Ch, Brew) ( Brew = (brew_formula_type *)Ch->desc->pEdit )
#define EDIT_GROUP(Ch, Group) ( Group = (group_type *)Ch->desc->pEdit )
#define EDIT_FACTION(Ch, Faction) ( Faction = (FACTION_DATA *)Ch->desc->pEdit )
#define EDIT_MAGIC_SCHOOL(Ch, School) ( School = (magic_school_type *)Ch->desc->pEdit )

// Datas command table
extern const struct olc_cmd_type ability_edit_table[];
extern const struct olc_cmd_type race_edit_table[];
extern const struct olc_cmd_type pcrace_edit_table[];
extern const struct olc_cmd_type command_edit_table[];
extern const struct olc_cmd_type pcclass_edit_table[];
extern const struct olc_cmd_type god_edit_table[];
extern const struct olc_cmd_type liquid_edit_table[];
extern const struct olc_cmd_type material_edit_table[];
extern const struct olc_cmd_type brew_edit_table[];
extern const struct olc_cmd_type group_edit_table[];
extern const struct olc_cmd_type faction_edit_table[];
extern const struct olc_cmd_type magic_school_edit_table[];

// Interpreter Prototypes
void    ability_edit           args( ( CHAR_DATA *ch, const char *argument ) );
void    race_edit              args( ( CHAR_DATA *ch, const char *argument ) );
void    pcrace_edit            args( ( CHAR_DATA *ch, const char *argument ) );
void    command_edit           args( ( CHAR_DATA *ch, const char *argument ) );
void    pcclass_edit           args( ( CHAR_DATA *ch, const char *argument ) );
void    god_edit               args( ( CHAR_DATA *ch, const char *argument ) );
void    liquid_edit            args( ( CHAR_DATA *ch, const char *argument ) );
void    material_edit          args( ( CHAR_DATA *ch, const char *argument ) );
void    brew_edit              args( ( CHAR_DATA *ch, const char *argument ) );
void    group_edit             args( ( CHAR_DATA *ch, const char *argument ) );
void    faction_edit           args( ( CHAR_DATA *ch, const char *argument ) );
void    magic_school_edit      args( ( CHAR_DATA *ch, const char *argument ) );

// Misc
DECLARE_DO_FUN( do_data_list );
DECLARE_DO_FUN( do_data_show );
DECLARE_DO_FUN( do_data_save );

// Data saving
DECLARE_DO_FUN( do_ability_save );
DECLARE_DO_FUN( do_race_save );
DECLARE_DO_FUN( do_command_save );
DECLARE_DO_FUN( do_pcclasses_save );
DECLARE_DO_FUN( do_god_save );
DECLARE_DO_FUN( do_liquid_save );
DECLARE_DO_FUN( do_material_save );
DECLARE_DO_FUN( do_brew_save );
DECLARE_DO_FUN( do_group_save );
DECLARE_DO_FUN( do_faction_save );
DECLARE_DO_FUN( do_magic_school_save );

// Entry Point for editing
DECLARE_DO_FUN( do_dedit );
DECLARE_DO_FUN( do_ability_edit );
DECLARE_DO_FUN( do_race_edit );
DECLARE_DO_FUN( do_pcrace_edit );
DECLARE_DO_FUN( do_command_edit );
DECLARE_DO_FUN( do_pcclass_edit );
DECLARE_DO_FUN( do_god_edit );
DECLARE_DO_FUN( do_liquid_edit );
DECLARE_DO_FUN( do_material_edit );
DECLARE_DO_FUN( do_brew_edit );
DECLARE_DO_FUN( do_group_edit );
DECLARE_DO_FUN( do_faction_edit );
DECLARE_DO_FUN( do_magic_school_edit );

// Methods for editing datas
//  Abilities
DECLARE_OLC_FUN( ability_edit_show );
DECLARE_OLC_FUN( ability_edit_slot );
DECLARE_OLC_FUN( ability_edit_rating );
DECLARE_OLC_FUN( ability_edit_level );
DECLARE_OLC_FUN( ability_edit_target );
DECLARE_OLC_FUN( ability_edit_position );
DECLARE_OLC_FUN( ability_edit_cost );
DECLARE_OLC_FUN( ability_edit_beats );
DECLARE_OLC_FUN( ability_edit_mobuse );
DECLARE_OLC_FUN( ability_edit_dam_msg );
DECLARE_OLC_FUN( ability_edit_char_wear_off );
DECLARE_OLC_FUN( ability_edit_obj_wear_off );
DECLARE_OLC_FUN( ability_edit_room_wear_off );
DECLARE_OLC_FUN( ability_edit_dispellable );
DECLARE_OLC_FUN( ability_edit_dispel_msg );
DECLARE_OLC_FUN( ability_edit_wait );
DECLARE_OLC_FUN( ability_edit_craftable );
DECLARE_OLC_FUN( ability_edit_addprereqs );
DECLARE_OLC_FUN( ability_edit_delprereqs );
DECLARE_OLC_FUN( ability_edit_setprereqs );
DECLARE_OLC_FUN( ability_edit_name );
DECLARE_OLC_FUN( ability_edit_school );
// Races
DECLARE_OLC_FUN( race_edit_show );
DECLARE_OLC_FUN( race_edit_act );
DECLARE_OLC_FUN( race_edit_affect );
DECLARE_OLC_FUN( race_edit_affect2 );
DECLARE_OLC_FUN( race_edit_offensive );
DECLARE_OLC_FUN( race_edit_immunities );
DECLARE_OLC_FUN( race_edit_resistances );
DECLARE_OLC_FUN( race_edit_vulnerabilities );
DECLARE_OLC_FUN( race_edit_forms );
DECLARE_OLC_FUN( race_edit_parts );
DECLARE_OLC_FUN( race_edit_pcrace );
DECLARE_OLC_FUN( race_edit_size ); // moved out of pc_race_type to race_type
// PC Races
DECLARE_OLC_FUN( pcrace_edit_show );
DECLARE_OLC_FUN( pcrace_edit_whoname );
DECLARE_OLC_FUN( pcrace_edit_experience );
DECLARE_OLC_FUN( pcrace_edit_type );
//DECLARE_OLC_FUN( pcrace_edit_size );
DECLARE_OLC_FUN( pcrace_edit_attributes );
DECLARE_OLC_FUN( pcrace_edit_language );
DECLARE_OLC_FUN( pcrace_edit_abilities );
DECLARE_OLC_FUN( pcrace_edit_alignment );
DECLARE_OLC_FUN( pcrace_edit_classes );
DECLARE_OLC_FUN( pcrace_edit_remort );
DECLARE_OLC_FUN( pcrace_edit_min_remort );
DECLARE_OLC_FUN( pcrace_edit_rebirth );
DECLARE_OLC_FUN( pcrace_edit_superrace );
// Command
DECLARE_OLC_FUN( command_edit_show );
DECLARE_OLC_FUN( command_edit_position );
DECLARE_OLC_FUN( command_edit_level );
DECLARE_OLC_FUN( command_edit_log );
DECLARE_OLC_FUN( command_edit_showing );
// PC Classes
DECLARE_OLC_FUN( pcclass_edit_show );
DECLARE_OLC_FUN( pcclass_edit_whoname );
DECLARE_OLC_FUN( pcclass_edit_attribute );
DECLARE_OLC_FUN( pcclass_edit_weapon );
DECLARE_OLC_FUN( pcclass_edit_adept );
DECLARE_OLC_FUN( pcclass_edit_thac0_00 );
DECLARE_OLC_FUN( pcclass_edit_thac0_32 );
DECLARE_OLC_FUN( pcclass_edit_hpmin );
DECLARE_OLC_FUN( pcclass_edit_hpmax );
DECLARE_OLC_FUN( pcclass_edit_type );
DECLARE_OLC_FUN( pcclass_edit_casting_rule );
DECLARE_OLC_FUN( pcclass_edit_base_group );
DECLARE_OLC_FUN( pcclass_edit_default_group );
DECLARE_OLC_FUN( pcclass_edit_parent );
DECLARE_OLC_FUN( pcclass_edit_choosable );
DECLARE_OLC_FUN( pcclass_edit_alignment );
DECLARE_OLC_FUN( pcclass_edit_obj );
DECLARE_OLC_FUN( pcclass_edit_create );
DECLARE_OLC_FUN( pcclass_edit_delete );
// God
DECLARE_OLC_FUN( god_edit_show );
DECLARE_OLC_FUN( god_edit_title );
DECLARE_OLC_FUN( god_edit_minor );
DECLARE_OLC_FUN( god_edit_major );
DECLARE_OLC_FUN( god_edit_classes );
DECLARE_OLC_FUN( god_edit_priest );
DECLARE_OLC_FUN( god_edit_races );
DECLARE_OLC_FUN( god_edit_story );
DECLARE_OLC_FUN( god_edit_alignment );
DECLARE_OLC_FUN( god_edit_create );
DECLARE_OLC_FUN( god_edit_delete );
DECLARE_OLC_FUN( god_edit_wild );
// Liquid
DECLARE_OLC_FUN( liquid_edit_show );
DECLARE_OLC_FUN( liquid_edit_color );
DECLARE_OLC_FUN( liquid_edit_drunk );
DECLARE_OLC_FUN( liquid_edit_full );
DECLARE_OLC_FUN( liquid_edit_thirst );
DECLARE_OLC_FUN( liquid_edit_food );
DECLARE_OLC_FUN( liquid_edit_ssize );
DECLARE_OLC_FUN( liquid_edit_create );
DECLARE_OLC_FUN( liquid_edit_delete );
// Material
DECLARE_OLC_FUN( material_edit_show );
DECLARE_OLC_FUN( material_edit_color );
DECLARE_OLC_FUN( material_edit_immunities );
DECLARE_OLC_FUN( material_edit_resistances );
DECLARE_OLC_FUN( material_edit_vulnerabilies );
DECLARE_OLC_FUN( material_edit_metallic );
DECLARE_OLC_FUN( material_edit_create );
DECLARE_OLC_FUN( material_edit_delete );
// Brew
DECLARE_OLC_FUN( brew_edit_show );
DECLARE_OLC_FUN( brew_edit_name );
DECLARE_OLC_FUN( brew_edit_name );
DECLARE_OLC_FUN( brew_edit_level );
DECLARE_OLC_FUN( brew_edit_cost );
//DECLARE_OLC_FUN( brew_edit_components );
DECLARE_OLC_FUN( brew_edit_add_components );
DECLARE_OLC_FUN( brew_edit_del_components );
DECLARE_OLC_FUN( brew_edit_effects );
DECLARE_OLC_FUN( brew_edit_create );
DECLARE_OLC_FUN( brew_edit_delete );
// Group
DECLARE_OLC_FUN( group_edit_show );
DECLARE_OLC_FUN( group_edit_contains );
DECLARE_OLC_FUN( group_edit_rating );
DECLARE_OLC_FUN( group_edit_sphere );
DECLARE_OLC_FUN( group_edit_god_rating );
DECLARE_OLC_FUN( group_edit_create );
DECLARE_OLC_FUN( group_edit_delete );
// Faction
DECLARE_OLC_FUN( faction_edit_show );
DECLARE_OLC_FUN( faction_edit_race );
DECLARE_OLC_FUN( faction_edit_friendliness );
DECLARE_OLC_FUN( faction_edit_create );
DECLARE_OLC_FUN( faction_edit_delete );
// Magic School
DECLARE_OLC_FUN( magic_school_edit_show );
DECLARE_OLC_FUN( magic_school_edit_name );
DECLARE_OLC_FUN( magic_school_edit_list );
DECLARE_OLC_FUN( magic_school_edit_create );
DECLARE_OLC_FUN( magic_school_edit_delete );
#endif
