#ifndef __GSN_H__
#define __GSN_H__

/*
 * These are skill_lookup return values for common skills and spells.
 */
extern  int     gsn_throw; /* Oxtal 1997*/
// Added by Seytan for HUNT skill 1997
extern  int     gsn_hunt;
// Added by Seytan for Fire skill 1997
extern  int     gsn_fire;
// Added by Sinac for Sharpen skill 1997
extern  int     gsn_sharpen;

// Added by Sinac 1997
extern  int     gsn_roundhousekick;
extern  int     gsn_enhanced_hand;
// added by SinaC 2000
extern  int     gsn_tail;

extern	int	gsn_backstab;
extern	int	gsn_dodge;
extern  int     gsn_envenom;
extern	int	gsn_hide;
extern	int	gsn_peek;
extern	int	gsn_pick_lock;
extern	int	gsn_sneak;
extern	int	gsn_steal;

extern	int	gsn_disarm;
extern	int	gsn_enhanced_damage;
extern	int	gsn_kick;
extern	int	gsn_parry;
extern	int	gsn_rescue;
extern	int	gsn_second_attack;
extern	int	gsn_third_attack;
// added by SinaC 2000
extern  int     gsn_fourth_attack;
extern  int     gsn_fifth_attack;
extern  int     gsn_sixth_attack;
extern  int     gsn_seventh_attack;
extern  int     gsn_dual_wield;
extern  int     gsn_circle;
extern  int     gsn_fade;
extern  int     gsn_counter;
extern  int     gsn_whirlwind;
extern  int     gsn_critical_strike;
extern  int     gsn_deathgrip;
extern  int     gsn_familiar;
extern  int     gsn_song;
//extern  int     gsn_throwing;  removed by SinaC 2003
extern  int     gsn_meditate;
extern  int     gsn_blindfight;
extern  int     gsn_butcher;
extern  int     gsn_grip;
extern  int     gsn_lure;
extern  int     gsn_pillify;
extern  int     gsn_bladethirst;
//extern  int     gsn_barkskin;
extern  int     gsn_crush;
extern  int     gsn_warcry;
extern  int     gsn_spike;
extern  int     gsn_pugil;
extern  int     gsn_lash;
extern  int     gsn_forage;
extern  int     gsn_rear_kick;
extern  int     gsn_find_water;
extern  int     gsn_shield_cleave;
extern  int     gsn_endure;
extern  int     gsn_nerve;
extern  int     gsn_bandage;
extern  int     gsn_herb;
extern  int     gsn_cleave;
extern  int     gsn_stun;
extern  int     gsn_freeflee;
// Added by SinaC 2001
extern  int     gsn_stake;
extern  int     gsn_align_detect;
extern  int     gsn_magic_detect;
extern  int     gsn_poison_detect;
extern  int     gsn_exits_detect;
extern  int     gsn_wail;
extern  int     gsn_wraithform;
extern  int	gsn_repulsion;
extern  int     gsn_resize;
extern  int     gsn_forge;
extern  int     gsn_vorpalize;

extern  int     gsn_iron_hand;
extern  int     gsn_energy_fist;
extern  int     gsn_burning_fist;
extern  int     gsn_buddha_palm;
extern  int     gsn_concentration;
extern  int     gsn_fist_of_fury;

extern  int     gsn_speedup;
extern  int     gsn_invisible;
extern  int     gsn_feed;
extern  int     gsn_war_drums;
extern  int     gsn_shroud;
extern  int     gsn_repair;
extern  int     gsn_morph;
extern  int     gsn_flight;
extern  int     gsn_mistform;
extern  int     gsn_bite;

extern int      gsn_protection_evil;
extern int      gsn_protection_good;
extern int      gsn_chill_touch;
extern int      gsn_fire_breath;

// special addition
extern  int     gsn_acid_breath;
// end

extern	int	gsn_blindness;
extern	int	gsn_charm_person;
extern	int	gsn_curse;
extern	int	gsn_invis;
extern	int	gsn_mass_invis;
extern  int     gsn_plague;
extern	int	gsn_poison;
extern	int	gsn_sleep;
extern  int     gsn_fly;
extern  int     gsn_sanctuary;
extern  int     gsn_haste;
extern  int     gsn_blend;

/* new gsns */
extern int      gsn_axe;
extern int      gsn_dagger;
extern int      gsn_flail;
extern int      gsn_mace;
extern int      gsn_polearm;
extern int      gsn_shield_block;
extern int      gsn_spear;
extern int      gsn_sword;
extern int      gsn_whip;
extern int      gsn_staff;
 
extern int      gsn_bash;
extern int      gsn_berserk;
extern int      gsn_dirt;
extern int      gsn_hand_to_hand;
extern int      gsn_trip;
 
extern int      gsn_fast_healing;
extern int      gsn_haggle;
extern int      gsn_lore;
extern int      gsn_meditation;
 
extern int      gsn_scrolls;
extern int      gsn_staves;
extern int      gsn_wands;
extern int      gsn_recall;


// Added by SinaC 2003
extern int      gsn_improved_exotic;
extern int      gsn_dual_parry;
//extern int      gsn_dual_wield2;
extern int      gsn_dual_disarm;
extern int      gsn_appraisal;
extern int      gsn_purify;
extern int      gsn_gouge;
extern int      gsn_evasion;
extern int      gsn_signal;
extern int      gsn_improved_steal;
extern int      gsn_comprehend_languages;
extern int      gsn_assassination;
extern int      gsn_armslore;
extern int      gsn_headbutt;
extern int      gsn_shield_bash;
extern int      gsn_justice;
extern int      gsn_angelic_light;
extern int      gsn_manashield;
extern int      gsn_corruption;
extern int      gsn_levitation;
extern int      gsn_nature_shield;
extern int      gsn_flight_bird;
extern int      gsn_speedofcheetah;
extern int      gsn_lightning_bolt;
extern int      gsn_frenzy;
extern int      gsn_bless;
extern int      gsn_slow;
extern int      gsn_armor;
extern int      gsn_shield;
extern int      gsn_mummify;
extern int      gsn_zombify;
extern int      gsn_animate_skeleton;
//extern int      gsn_animate_dead;
extern int      gsn_temporal_stasis;
extern int      gsn_globe_invuln;
extern int      gsn_entangle;
extern int      gsn_rooted_feet;
extern int      gsn_soothe;
extern int      gsn_stone_fist;
extern int      gsn_elemental_field;
extern int      gsn_weaken;
extern int      gsn_sacred_mists;
extern int      gsn_calm;
extern int      gsn_symbiote;
extern int      gsn_fireshield;
extern int      gsn_iceshield;
extern int      gsn_stoneshield;
extern int      gsn_airshield;
extern int      gsn_lightning_field;
extern int      gsn_improved_restoration;
extern int      gsn_enlarge;
extern int      gsn_reduce;
extern int      gsn_polymorph_self;
extern int      gsn_phylactery;
extern int      gsn_scribe;
extern int      gsn_riding;
extern int      gsn_mounted_combat;
extern int      gsn_charge;
extern int      gsn_bond;
extern int      gsn_invincible_mount;
extern int      gsn_strategic_retreat;
extern int      gsn_study;
extern int      gsn_flame_blade;
extern int      gsn_frost_blade;
extern int      gsn_drain_blade;
extern int      gsn_shocking_blade;
extern int      gsn_specialization;
extern int      gsn_disguise;
extern int      gsn_counterfeit;
extern int      gsn_favored_enemy;
//extern int      gsn_riding2;
extern int      gsn_climbing;
extern int      gsn_shapechange;
extern int      gsn_brew;
extern int      gsn_black_plague;

extern int      gsn_bowfire;

extern int      gsn_fleetfooted;
extern int      gsn_tumble;
extern int      gsn_block;
extern int      gsn_pure_body;
extern int      gsn_deflect;
extern int      gsn_diamond_body;
extern int      gsn_icy_fist;
extern int      gsn_acid_fist;
extern int      gsn_draining_fist;
extern int      gsn_hands_of_multitude;
extern int      gsn_flurry_of_fists;

extern int      gsn_fletcher;
extern int      gsn_disarming_shot;
extern int      gsn_bowyer;
extern int      gsn_strength_shot;
extern int      gsn_ignite_arrow;
extern int      gsn_long_range_shot;
extern int      gsn_sharpen_arrow;
extern int      gsn_point_blank_shot;
extern int      gsn_critical_shot;
extern int      gsn_third_wield;
extern int      gsn_fourth_wield;
extern int      gsn_protection_lycanthropy;
extern int      gsn_reckless_dweomer;

extern int      gsn_accelerate;

// Special Affect such as Lycanthropy
extern int      gsn_lycanthropy;


#endif
