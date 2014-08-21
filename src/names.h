#ifndef __NAMES_H__
#define __NAMES_H__

const char *god_name       args( ( int who ) );
const char *etho_name      args( ( int etho ) );
const char *align_name     args( ( int align ) );
const char *etho_align_name args( ( int etho, int align ) );
const char *weapon_name	args( ( int weapon_Type) );
const char *item_name	args( ( int item_type) ); 

const char *	affect_bit_name	args( ( long vector ) );
const char *	extra_bit_name	args( ( int extra_flags ) );
const char * 	wear_bit_name	args( ( int wear_flags ) );
const char *	act_bit_name	args( ( int act_flags ) );
const char *	off_bit_name	args( ( int off_flags ) );
//const char *    imm_bit_name	args( ( int imm_flags ) );
const char *    irv_bit_name    args( ( int irv_flags ) );
const char * 	form_bit_name	args( ( int form_flags ) );
const char *	part_bit_name	args( ( int part_flags ) );
const char *	weapon_bit_name	args( ( int weapon_flags ) );
const char *  comm_bit_name	args( ( int comm_flags ) );
const char *	cont_bit_name	args( ( int cont_flags) );
// Added by SinaC 2000
const char *  damtype_name      args( ( int dam_type ) );
// Added by SinaC 2001
const char *	affect2_bit_name	args( ( long vector ) );
// Added by SinaC 2001
//const char *    classtype_name  args( ( int type ) );
// Added by SinaC 2001
const char *    abilitytype_name  args( ( int type ) );

// Added by SinaC 2003
const char *    script_type_name args( ( int type ) );
#endif
