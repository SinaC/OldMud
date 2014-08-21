#ifndef __SPELLS_DEF_H__
#define __SPELLS_DEF_H__

/*
 * Spell functions.
 * Defined in magic.c, magic2.C, magic3.C, magic4.C and magic5.C
 */
//DECLARE_SPELL_FUN(	spell_null		);
DECLARE_SPELL_FUN(	spell_acid_blast	);
DECLARE_SPELL_FUN(	spell_armor		);
DECLARE_SPELL_FUN(	spell_bless		);
DECLARE_SPELL_FUN(	spell_blindness		);
DECLARE_SPELL_FUN(	spell_burning_hands	);
DECLARE_SPELL_FUN(	spell_call_lightning	);
DECLARE_SPELL_FUN(      spell_calm		);
DECLARE_SPELL_FUN(      spell_cancellation	);
DECLARE_SPELL_FUN(	spell_cause_critical	);
DECLARE_SPELL_FUN(	spell_cause_light	);
DECLARE_SPELL_FUN(	spell_cause_serious	);
DECLARE_SPELL_FUN(	spell_change_sex	);
DECLARE_SPELL_FUN(      spell_chain_lightning   );
DECLARE_SPELL_FUN(	spell_charm_person	);
DECLARE_SPELL_FUN(	spell_chill_touch	);
DECLARE_SPELL_FUN(	spell_colour_spray	);
DECLARE_SPELL_FUN(	spell_continual_light	);
DECLARE_SPELL_FUN(	spell_control_weather	);
DECLARE_SPELL_FUN(	spell_create_food	);
DECLARE_SPELL_FUN(	spell_create_rose	);
DECLARE_SPELL_FUN(	spell_create_spring	);
DECLARE_SPELL_FUN(	spell_create_water	);
DECLARE_SPELL_FUN(	spell_cure_blindness	);
DECLARE_SPELL_FUN(	spell_cure_critical	);
DECLARE_SPELL_FUN(      spell_cure_disease	);
DECLARE_SPELL_FUN(	spell_cure_light	);
DECLARE_SPELL_FUN(	spell_cure_poison	);
DECLARE_SPELL_FUN(	spell_cure_serious	);
DECLARE_SPELL_FUN(	spell_curse		);
DECLARE_SPELL_FUN(      spell_demonfire		);
DECLARE_SPELL_FUN(	spell_detect_evil	);
DECLARE_SPELL_FUN(	spell_detect_good	);
DECLARE_SPELL_FUN(	spell_detect_hidden	);
DECLARE_SPELL_FUN(	spell_detect_invis	);
DECLARE_SPELL_FUN(	spell_detect_magic	);
DECLARE_SPELL_FUN(	spell_detect_poison	);
DECLARE_SPELL_FUN(	spell_dispel_evil	);
DECLARE_SPELL_FUN(      spell_dispel_good       );
DECLARE_SPELL_FUN(	spell_dispel_magic	);
DECLARE_SPELL_FUN(	spell_earthquake	);
DECLARE_SPELL_FUN(	spell_enchant_armor	);
DECLARE_SPELL_FUN(	spell_enchant_weapon	);
DECLARE_SPELL_FUN(	spell_energy_drain	);
DECLARE_SPELL_FUN(	spell_faerie_fire	);
DECLARE_SPELL_FUN(	spell_faerie_fog	);
DECLARE_SPELL_FUN(	spell_fireball		);
DECLARE_SPELL_FUN(	spell_fireproof		);
DECLARE_SPELL_FUN(	spell_flamestrike	);
DECLARE_SPELL_FUN(	spell_floating_disc	);
DECLARE_SPELL_FUN(	spell_fly		);
DECLARE_SPELL_FUN(      spell_frenzy		);
DECLARE_SPELL_FUN(	spell_gate		);
DECLARE_SPELL_FUN(	spell_giant_strength	);
DECLARE_SPELL_FUN(	spell_harm		);
DECLARE_SPELL_FUN(      spell_haste		);
DECLARE_SPELL_FUN(	spell_heal		);
DECLARE_SPELL_FUN(	spell_heat_metal	);
DECLARE_SPELL_FUN(      spell_holy_word		);
DECLARE_SPELL_FUN(	spell_identify		);
DECLARE_SPELL_FUN(	spell_infravision	);
DECLARE_SPELL_FUN(	spell_invis		);
DECLARE_SPELL_FUN(	spell_know_alignment	);
DECLARE_SPELL_FUN(	spell_lightning_bolt	);
DECLARE_SPELL_FUN(	spell_locate_object	);
DECLARE_SPELL_FUN(	spell_magic_missile	);
DECLARE_SPELL_FUN(      spell_mass_healing	);
DECLARE_SPELL_FUN(	spell_mass_invis	);
DECLARE_SPELL_FUN(	spell_nexus		);
DECLARE_SPELL_FUN(	spell_pass_door		);
DECLARE_SPELL_FUN(      spell_plague		);
DECLARE_SPELL_FUN(	spell_poison		);
DECLARE_SPELL_FUN(	spell_portal		);
DECLARE_SPELL_FUN(	spell_protection_evil	);
DECLARE_SPELL_FUN(	spell_protection_good	);
DECLARE_SPELL_FUN(	spell_ray_of_truth	);
DECLARE_SPELL_FUN(	spell_recharge		);
DECLARE_SPELL_FUN(	spell_refresh		);
DECLARE_SPELL_FUN(	spell_remove_curse	);
DECLARE_SPELL_FUN(	spell_sanctuary		);
DECLARE_SPELL_FUN(	spell_shocking_grasp	);
DECLARE_SPELL_FUN(	spell_shield		);
DECLARE_SPELL_FUN(	spell_sleep		);
DECLARE_SPELL_FUN(	spell_slow		);
DECLARE_SPELL_FUN(	spell_stone_skin	);
DECLARE_SPELL_FUN(	spell_summon		);
DECLARE_SPELL_FUN(	spell_teleport		);
DECLARE_SPELL_FUN(	spell_ventriloquate	);
DECLARE_SPELL_FUN(	spell_weaken		);
DECLARE_SPELL_FUN(	spell_acid_breath	);
DECLARE_SPELL_FUN(	spell_fire_breath	);
DECLARE_SPELL_FUN(	spell_frost_breath	);
DECLARE_SPELL_FUN(	spell_gas_breath	);
DECLARE_SPELL_FUN(	spell_lightning_breath	);
DECLARE_SPELL_FUN(	spell_general_purpose	);
DECLARE_SPELL_FUN(	spell_high_explosive	);



/* Added by Sinac & Seytan 1997 */

DECLARE_SPELL_FUN(      spell_black_lotus       );

DECLARE_SPELL_FUN(      spell_resurrect         );
DECLARE_SPELL_FUN(      spell_acid_arrow        );
DECLARE_SPELL_FUN(      spell_flame_arrow       );
// removed by SinaC 2000
//DECLARE_SPELL_FUN(      spell_mount             );
DECLARE_SPELL_FUN(      spell_aid               );

DECLARE_SPELL_FUN(      spell_create_sword      );
DECLARE_SPELL_FUN(      spell_create_dagger     );
DECLARE_SPELL_FUN(      spell_blur              );
DECLARE_SPELL_FUN(	spell_bird_form		);
// SinaC 2000
DECLARE_SPELL_FUN(      spell_beacon            );
DECLARE_SPELL_FUN(      spell_fumble            );
DECLARE_SPELL_FUN(      spell_vaccine           );
DECLARE_SPELL_FUN(      spell_ionwave           );
DECLARE_SPELL_FUN(      spell_banshee_scream    );
DECLARE_SPELL_FUN(      spell_locate_person     );
DECLARE_SPELL_FUN(      spell_silence           );
DECLARE_SPELL_FUN(      spell_acid_rain         );
DECLARE_SPELL_FUN(      spell_layhands          );
DECLARE_SPELL_FUN(      spell_jades_lust        );
DECLARE_SPELL_FUN(      spell_summon_lgolem     );
DECLARE_SPELL_FUN(      spell_summon_ggolem     );
DECLARE_SPELL_FUN(      spell_soul_blade        );
DECLARE_SPELL_FUN(      spell_mind_meld         );
DECLARE_SPELL_FUN(      spell_psi_twister       );
DECLARE_SPELL_FUN(      spell_regeneration      );
DECLARE_SPELL_FUN(      spell_exorcism          );
// Spells from Tartarus
DECLARE_SPELL_FUN(      spell_utter_heal        );
DECLARE_SPELL_FUN(      spell_flesh_golem       );
DECLARE_SPELL_FUN(      spell_lightshield       );
DECLARE_SPELL_FUN(      spell_darkshield        );
DECLARE_SPELL_FUN(      spell_fire_and_ice      );
DECLARE_SPELL_FUN(      spell_shock_sphere      );
DECLARE_SPELL_FUN(      spell_cremate           );
DECLARE_SPELL_FUN(      spell_sunbolt           );
DECLARE_SPELL_FUN(      spell_frostbolt         );
DECLARE_SPELL_FUN(      spell_icelance          );
DECLARE_SPELL_FUN(      spell_lifebane          );
DECLARE_SPELL_FUN(      spell_old_deathspell    );
DECLARE_SPELL_FUN(      spell_new_energy_drain  );
DECLARE_SPELL_FUN(      spell_blade_barrier     );
DECLARE_SPELL_FUN(      spell_holy_fire         );
DECLARE_SPELL_FUN(      spell_revolt            );
DECLARE_SPELL_FUN(      spell_firestream        );
DECLARE_SPELL_FUN(      spell_wither            );
DECLARE_SPELL_FUN(      spell_hand_of_vengeance );
DECLARE_SPELL_FUN(      spell_iceball           );
DECLARE_SPELL_FUN(      spell_cone_of_cold2     );
DECLARE_SPELL_FUN(      spell_spiritblade       );
DECLARE_SPELL_FUN(      spell_drain             );
DECLARE_SPELL_FUN(      spell_earthmaw          );
DECLARE_SPELL_FUN(      spell_windwall          );
DECLARE_SPELL_FUN(      spell_web               );
DECLARE_SPELL_FUN(      spell_old_turn_undead   );
DECLARE_SPELL_FUN(      spell_channel           );
DECLARE_SPELL_FUN(      spell_true_sight        );
DECLARE_SPELL_FUN(      spell_dark_wrath        );
DECLARE_SPELL_FUN(      spell_wrath             );
DECLARE_SPELL_FUN(      spell_new_holy_word     );
DECLARE_SPELL_FUN(      spell_new_demonfire     );
DECLARE_SPELL_FUN(      spell_concatenate       );
DECLARE_SPELL_FUN(      spell_power_word_kill   );
DECLARE_SPELL_FUN(      spell_evil_eye          );
DECLARE_SPELL_FUN(      spell_preserve          );
DECLARE_SPELL_FUN(      spell_decay_corpse      );
DECLARE_SPELL_FUN(      spell_mummify           );
DECLARE_SPELL_FUN(      spell_animate_skeleton  );
DECLARE_SPELL_FUN(      spell_zombify           );
DECLARE_SPELL_FUN(      spell_restoration       );
DECLARE_SPELL_FUN(      spell_embalm            );
//Spells from Dracon
DECLARE_SPELL_FUN(      spell_animate_dead      );
DECLARE_SPELL_FUN(      spell_daemonic_aid      );
DECLARE_SPELL_FUN(      spell_daemonic_carapace );
DECLARE_SPELL_FUN(      spell_daemonic_potency  );
DECLARE_SPELL_FUN(      spell_repulsion         );
DECLARE_SPELL_FUN(      spell_divine_intervention );
DECLARE_SPELL_FUN(      spell_holy_armor        );
DECLARE_SPELL_FUN(      spell_holy_sword        );
DECLARE_SPELL_FUN(      spell_unholy_blade      );
DECLARE_SPELL_FUN(      spell_resist_fire       );
DECLARE_SPELL_FUN(      spell_entangle          );
DECLARE_SPELL_FUN(      spell_wraithform        );
DECLARE_SPELL_FUN(      spell_manashield        );
DECLARE_SPELL_FUN(      spell_shroud            );

// new spells
DECLARE_SPELL_FUN(      spell_drain_blade       );
DECLARE_SPELL_FUN(      spell_shocking_blade    );
DECLARE_SPELL_FUN(      spell_flame_blade       );
DECLARE_SPELL_FUN(      spell_frost_blade       );
DECLARE_SPELL_FUN(      spell_revitalize        );
DECLARE_SPELL_FUN(      spell_death_breath      );
DECLARE_SPELL_FUN(      spell_water_breath      );
DECLARE_SPELL_FUN(      spell_walk_on_water     );
DECLARE_SPELL_FUN(      spell_arcane_concordance);
DECLARE_SPELL_FUN(      spell_magic_mirror      );
DECLARE_SPELL_FUN(      spell_prismatic_spray   );
DECLARE_SPELL_FUN(      spell_creeping_doom     );
DECLARE_SPELL_FUN(      spell_icestorm          );
DECLARE_SPELL_FUN(      spell_moonbeam          );
// Added by SinaC 2003, most spells come from Dracon
DECLARE_SPELL_FUN(      spell_shrink            );
DECLARE_SPELL_FUN(      spell_create_shelter    );
DECLARE_SPELL_FUN(      spell_strength_of_the_bear);
DECLARE_SPELL_FUN(      spell_eyes_of_the_wolf  );
DECLARE_SPELL_FUN(      spell_barkskin          );
DECLARE_SPELL_FUN(      spell_free_movement     );
DECLARE_SPELL_FUN(      spell_rooted_feet       );
DECLARE_SPELL_FUN(      spell_hibernation       );
DECLARE_SPELL_FUN(      spell_nature_blessing   );
DECLARE_SPELL_FUN(      spell_calm_animal       );
DECLARE_SPELL_FUN(      spell_holy_symbol       );
DECLARE_SPELL_FUN(      spell_angelic_strength  );
DECLARE_SPELL_FUN(      spell_angelic_light     );
DECLARE_SPELL_FUN(      spell_inspiration       );
DECLARE_SPELL_FUN(      spell_judgment_bolt    );
DECLARE_SPELL_FUN(      spell_holy_aura         );
DECLARE_SPELL_FUN(      spell_justice           );
DECLARE_SPELL_FUN(      spell_charms_daemon     );
DECLARE_SPELL_FUN(      spell_terror            );
DECLARE_SPELL_FUN(      spell_unholy_word       );
DECLARE_SPELL_FUN(      spell_chaos_bolt        );
DECLARE_SPELL_FUN(      spell_unholy_aura       );
DECLARE_SPELL_FUN(      spell_ray_of_deception  );
DECLARE_SPELL_FUN(      spell_corruption        );
DECLARE_SPELL_FUN(      spell_last_rites        );
DECLARE_SPELL_FUN(      spell_hovering_sphere   );
DECLARE_SPELL_FUN(      spell_cloud_of_revelation );
DECLARE_SPELL_FUN(      spell_final_prayer      );
DECLARE_SPELL_FUN(      spell_create_alcohol    );
DECLARE_SPELL_FUN(      spell_mystic_boat       );
DECLARE_SPELL_FUN(      spell_spiritual_hammer  );
DECLARE_SPELL_FUN(      spell_resist_poison     );
DECLARE_SPELL_FUN(      spell_prismatic_armor   );
DECLARE_SPELL_FUN(      spell_exhaust           );
DECLARE_SPELL_FUN(      spell_mass_harm         );
DECLARE_SPELL_FUN(      spell_combat_will       );
DECLARE_SPELL_FUN(      spell_frozen_blast      );
DECLARE_SPELL_FUN(      spell_fire_blast        );
DECLARE_SPELL_FUN(      spell_final_strike      );
DECLARE_SPELL_FUN(      spell_deplete_strength  );
DECLARE_SPELL_FUN(      spell_detect_weather    );
DECLARE_SPELL_FUN(      spell_lightning_reflexes );
DECLARE_SPELL_FUN(      spell_create_rainbow    );
DECLARE_SPELL_FUN(      spell_protection_lightning );
DECLARE_SPELL_FUN(      spell_summon_thunder    );
DECLARE_SPELL_FUN(      spell_windwalk          );
DECLARE_SPELL_FUN(      spell_earthcrumble      );
DECLARE_SPELL_FUN(      spell_lightning_strike  );
DECLARE_SPELL_FUN(      spell_hurricane         );
DECLARE_SPELL_FUN(      spell_locate_warpstone  );
DECLARE_SPELL_FUN(      spell_unstable_gate     );
DECLARE_SPELL_FUN(      spell_gemwarp           );
DECLARE_SPELL_FUN(      spell_reverse_gravity   );
DECLARE_SPELL_FUN(      spell_protection_negative );
DECLARE_SPELL_FUN(      spell_angelfire         );
DECLARE_SPELL_FUN(      spell_life_transfer     );
DECLARE_SPELL_FUN(      spell_protection_holy   );
DECLARE_SPELL_FUN(      spell_life_drain        );
DECLARE_SPELL_FUN(      spell_nature_shield     );
DECLARE_SPELL_FUN(      spell_anti_venom_spores );
DECLARE_SPELL_FUN(      spell_flower_of_health  );
DECLARE_SPELL_FUN(      spell_flight_of_the_bird );
DECLARE_SPELL_FUN(      spell_polar_adaptation  );
DECLARE_SPELL_FUN(      spell_desert_adaptation );
DECLARE_SPELL_FUN(      spell_speed_of_the_cheetah );
DECLARE_SPELL_FUN(      spell_insect_swarm      );
DECLARE_SPELL_FUN(      spell_animal_friendship );
DECLARE_SPELL_FUN(      spell_thorn_blast       );
DECLARE_SPELL_FUN(      spell_plant_door        );
DECLARE_SPELL_FUN(      spell_choke             );
DECLARE_SPELL_FUN(      spell_thunderclap       );
DECLARE_SPELL_FUN(      spell_hypnotism         );
DECLARE_SPELL_FUN(      spell_globe_invuln      );
DECLARE_SPELL_FUN(      spell_mind_blast        );
DECLARE_SPELL_FUN(      spell_mind_blade        );
DECLARE_SPELL_FUN(      spell_mirror_image      );
DECLARE_SPELL_FUN(      spell_water_to_wine     );
DECLARE_SPELL_FUN(      spell_death_fog         );
DECLARE_SPELL_FUN(      spell_animate_object    );
DECLARE_SPELL_FUN(      spell_antigravity       );
DECLARE_SPELL_FUN(      spell_temporal_stasis   );
DECLARE_SPELL_FUN(      spell_old_energy_drain  );
DECLARE_SPELL_FUN(      spell_command_undead    );
DECLARE_SPELL_FUN(      spell_flamevision       );
DECLARE_SPELL_FUN(      spell_stone_fist        );
DECLARE_SPELL_FUN(      spell_hydroblast        );
DECLARE_SPELL_FUN(      spell_elemental_field   );
DECLARE_SPELL_FUN(      spell_immolation        );
DECLARE_SPELL_FUN(      spell_cone_of_cold      );
DECLARE_SPELL_FUN(      spell_shrieking_blades  );
DECLARE_SPELL_FUN(      spell_full_heal         );
DECLARE_SPELL_FUN(      spell_slumber           );
DECLARE_SPELL_FUN(      spell_darkvision        );
DECLARE_SPELL_FUN(      spell_cloud_of_darkness );
DECLARE_SPELL_FUN(      spell_tsunami           );
DECLARE_SPELL_FUN(      spell_sacred_mists      );
DECLARE_SPELL_FUN(      spell_ride_the_winds    );
DECLARE_SPELL_FUN(      spell_snowy_blast       );
DECLARE_SPELL_FUN(      spell_battle_frenzy     );
DECLARE_SPELL_FUN(      spell_battle_fury       );
DECLARE_SPELL_FUN(      spell_friendship        );
DECLARE_SPELL_FUN(      spell_seduction         );
DECLARE_SPELL_FUN(      spell_find_path         );
DECLARE_SPELL_FUN(      spell_symbiote          );
DECLARE_SPELL_FUN(      spell_soulbind          );
DECLARE_SPELL_FUN(      spell_ray_of_sun        );
DECLARE_SPELL_FUN(      spell_sunblind          );
DECLARE_SPELL_FUN(      spell_knowledge         );
DECLARE_SPELL_FUN(      spell_tongues           );
DECLARE_SPELL_FUN(      spell_fireshield        );
DECLARE_SPELL_FUN(      spell_iceshield         );
DECLARE_SPELL_FUN(      spell_stoneshield       );
DECLARE_SPELL_FUN(      spell_airshield         );
DECLARE_SPELL_FUN(      spell_antimagic_field   );
DECLARE_SPELL_FUN(      spell_flesh_to_stone    );
DECLARE_SPELL_FUN(      spell_turn_undead       );
DECLARE_SPELL_FUN(      spell_quicksand         );
DECLARE_SPELL_FUN(      spell_whirling_sands    );
DECLARE_SPELL_FUN(      spell_lightning_field   );
DECLARE_SPELL_FUN(      spell_arrow_of_anylas   );
DECLARE_SPELL_FUN(      spell_improved_restoration );
DECLARE_SPELL_FUN(      spell_resurrection      );
DECLARE_SPELL_FUN(      spell_vacuum            );
DECLARE_SPELL_FUN(      spell_flame_burst       );
DECLARE_SPELL_FUN(      spell_ice_blast         );
DECLARE_SPELL_FUN(      spell_ice_storm         );
DECLARE_SPELL_FUN(      spell_fire_storm        );
DECLARE_SPELL_FUN(      spell_meteor_shower     );
DECLARE_SPELL_FUN(      spell_necrotism         );
DECLARE_SPELL_FUN(      spell_enlarge           );
DECLARE_SPELL_FUN(      spell_reduce            );
DECLARE_SPELL_FUN(      spell_growth_shrooms    );
DECLARE_SPELL_FUN(      spell_shrinking_shrooms );
DECLARE_SPELL_FUN(      spell_gift_of_nature    );
DECLARE_SPELL_FUN(      spell_tornado           );
DECLARE_SPELL_FUN(      spell_sandstorm         );
DECLARE_SPELL_FUN(      spell_hydrostrike       );
DECLARE_SPELL_FUN(      spell_blinding_radiance );
DECLARE_SPELL_FUN(      spell_polymorph_self    );
DECLARE_SPELL_FUN(      spell_arachnophobia     );
DECLARE_SPELL_FUN(      spell_fear              );
DECLARE_SPELL_FUN(      spell_pool_of_blood     );
DECLARE_SPELL_FUN(      spell_phylactery        );
DECLARE_SPELL_FUN(      spell_invincible_mount  );
DECLARE_SPELL_FUN(      spell_strategic_retreat );
DECLARE_SPELL_FUN(      spell_speak_with_dead   );
DECLARE_SPELL_FUN(      spell_enchant_wand      );
DECLARE_SPELL_FUN(      spell_enchant_staff     );
DECLARE_SPELL_FUN(      spell_inferno           );
DECLARE_SPELL_FUN(      spell_detect_lycanthropy );
DECLARE_SPELL_FUN(      spell_conjure_mount     );
DECLARE_SPELL_FUN(      spell_summon_angel      );
DECLARE_SPELL_FUN(      spell_summon_demon      );
DECLARE_SPELL_FUN(      spell_faerie_aura       );
DECLARE_SPELL_FUN(      spell_word_of_recall    );
DECLARE_SPELL_FUN(      spell_protection_lycanthropy );
DECLARE_SPELL_FUN(      spell_confusion         );
DECLARE_SPELL_FUN(      spell_chaotic_blast     );
DECLARE_SPELL_FUN(      spell_create_vines      );
DECLARE_SPELL_FUN(      spell_flare             );
DECLARE_SPELL_FUN(      spell_animal_follower   );
DECLARE_SPELL_FUN(      spell_spellfire         );
DECLARE_SPELL_FUN(      spell_summon_elemental  );
DECLARE_SPELL_FUN(      spell_necrofire         );
DECLARE_SPELL_FUN(      spell_spectral_hand     );
DECLARE_SPELL_FUN(      spell_rotting_touch     );
DECLARE_SPELL_FUN(      spell_death_spell       );
DECLARE_SPELL_FUN(      spell_summon_ghost      );
DECLARE_SPELL_FUN(      spell_finger_of_death   );
DECLARE_SPELL_FUN(      spell_black_plague      );
DECLARE_SPELL_FUN(      spell_reckless_dweomer  );

#endif
