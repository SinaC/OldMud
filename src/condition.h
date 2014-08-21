#ifndef __CONDITION_H__
#define __CONDITION_H__

const char *show_obj_cond (OBJ_DATA *obj);
void check_damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int chance, int dam_type );
void set_obj_cond (OBJ_DATA *obj, int condition);
int check_immune_obj(OBJ_DATA *obj, int dam_type);
void damage_obj (CHAR_DATA *ch, OBJ_DATA *obj, int damage, int dam_type );


#endif

