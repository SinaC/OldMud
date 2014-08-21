#ifndef __POWER_H__
#define __POWER_H__

DECLARE_DO_FUN( do_power);
DECLARE_DO_FUN( do_psipowers );

void    obj_use_power  args( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj) );

DECLARE_POWER_FUN(	power_ballistic_attack	);
DECLARE_POWER_FUN(	power_create_sound	);
DECLARE_POWER_FUN(	power_mental_barrier	);
DECLARE_POWER_FUN(	power_neuro_healing	);
DECLARE_POWER_FUN(	power_mind_thrust	);
DECLARE_POWER_FUN(	power_thought_shield	);
DECLARE_POWER_FUN(	power_enhanced_strength	);
DECLARE_POWER_FUN(	power_cell_adjustment	);
DECLARE_POWER_FUN(	power_flesh_armor	);
DECLARE_POWER_FUN(	power_neuro_drain	);
DECLARE_POWER_FUN(	power_adrenaline_control);
DECLARE_POWER_FUN(	power_ego_whip		);
DECLARE_POWER_FUN(      power_minor_pain        );
DECLARE_POWER_FUN(	power_inflict_pain	);
DECLARE_POWER_FUN(	power_share_strength	);
DECLARE_POWER_FUN(      power_levitation        );
DECLARE_POWER_FUN(	power_awe		);
DECLARE_POWER_FUN(	power_agitation		);
DECLARE_POWER_FUN(      power_trouble           );
DECLARE_POWER_FUN(	power_displacement	);
DECLARE_POWER_FUN(      power_mind_fear         );
DECLARE_POWER_FUN(	power_control_flames	);
DECLARE_POWER_FUN(      power_lend_health       );
DECLARE_POWER_FUN(	power_energy_containment);
DECLARE_POWER_FUN(      power_dizziness         );
DECLARE_POWER_FUN(	power_neuro_crush	);
DECLARE_POWER_FUN(	power_intellect_fortress);
DECLARE_POWER_FUN(      power_hesitation        );
DECLARE_POWER_FUN(      power_major_pain        );
DECLARE_POWER_FUN(	power_aura_sight	);
DECLARE_POWER_FUN(	power_biofeedback	);
DECLARE_POWER_FUN(	power_ectoplasmic_form	);
DECLARE_POWER_FUN(	power_domination	);
DECLARE_POWER_FUN(	power_detonate		);
DECLARE_POWER_FUN(	power_project_force	);
DECLARE_POWER_FUN(      power_psychic_impact    );
DECLARE_POWER_FUN(	power_neuro_blast	);
DECLARE_POWER_FUN(      power_combat_mind       );
DECLARE_POWER_FUN(	power_raw_flesh		);
DECLARE_POWER_FUN(      power_complete_healing  );
DECLARE_POWER_FUN(	power_ultrablast	);
DECLARE_POWER_FUN(	power_disintegrate	);

DECLARE_POWER_FUN(      power_endure            );
DECLARE_POWER_FUN(      power_iron_hand         );
DECLARE_POWER_FUN(      power_telepathic_gate   );
DECLARE_POWER_FUN(      power_kihai             );
DECLARE_POWER_FUN(      power_burning_fist      );
DECLARE_POWER_FUN(      power_energy_fist       );
DECLARE_POWER_FUN(      power_inertial_barrier  );
DECLARE_POWER_FUN(      power_icy_fist          );
DECLARE_POWER_FUN(      power_acid_fist         );
DECLARE_POWER_FUN(      power_draining_fist     );
DECLARE_POWER_FUN(      power_ki_pass           );
DECLARE_POWER_FUN(      power_lifetouch         );

DECLARE_POWER_FUN(      power_disrupt           );
DECLARE_POWER_FUN(      power_warmth            );
DECLARE_POWER_FUN(      power_telekinesis       );
DECLARE_POWER_FUN(      power_mind_over_body    );
DECLARE_POWER_FUN(      power_soften            );
DECLARE_POWER_FUN(      power_mind_freeze       );
DECLARE_POWER_FUN(      power_accelerate        );
DECLARE_POWER_FUN(      power_nerve_shock       );
DECLARE_POWER_FUN(      power_probability_travel);
DECLARE_POWER_FUN(      power_cause_decay       );
DECLARE_POWER_FUN(      power_acidic_touch      );
DECLARE_POWER_FUN(      power_neural_burn       );
DECLARE_POWER_FUN(      power_enhance_armor     );
DECLARE_POWER_FUN(      power_trauma            );
DECLARE_POWER_FUN(      power_willpower         );



DECLARE_POWER_FUN(      power_regain_power      );

DECLARE_POWER_FUN(      power_death_field       );


#endif
