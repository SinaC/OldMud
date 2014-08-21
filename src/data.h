#ifndef __DATA_H__
#define __DATA_H__

extern int MAX_ABILITY_INIT;
extern bool                abilitynotsaved;
extern bool                clannotsaved;
extern bool                racenotsaved;
extern bool                cmdnotsaved;
extern bool                pcclassnotsaved;
extern bool                godnotsaved;
extern bool                liqnotsaved;
extern bool                materialnotsaved;
extern bool                groupnotsaved;
extern bool                spherenotsaved;
extern bool                schoolnotsaved;
extern bool                superracenotsaved;

void check_struct_integrity();

void save_schools();
void load_schools();

void save_hometown();
void load_hometown();

void init_time_weather();
void save_time();
void load_time();

void new_save_pc_classes();
void new_load_pc_classes();

void new_save_abilities();
void new_load_abilities();

void new_save_liquid();
void new_load_liquid();

void new_save_races();
void new_load_races();
void new_save_pcraces();
void new_load_pcraces();
void new_reload_races();

void new_save_groups();
void new_load_groups();
void check_groups();

void new_save_gods();
void new_load_gods();
void update_group_abilities_with_spheres();

void new_save_material();
void new_load_material();

void new_save_unique_items();
void new_load_unique_items();

void new_save_commands();
void new_load_commands();

void new_save_spheres();
void new_load_spheres();

void save_superraces();
void load_superraces();

#endif
