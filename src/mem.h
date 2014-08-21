#ifndef __MEM_H__
#define __MEM_H__

RESET_DATA	 *new_reset_data	args ( ( void ) );
AREA_DATA	 *new_area		args ( ( void ) );
EXIT_DATA	 *new_exit		args ( ( void ) );
EXTRA_DESCR_DATA *new_extra_descr	args ( ( void ) );
ROOM_INDEX_DATA  *new_room_index	args ( ( void ) );
AFFECT_DATA	 *new_affect		args ( ( void ) );
SHOP_DATA	 *new_shop		args ( ( void ) );
OBJ_INDEX_DATA	 *new_obj_index		args ( ( void ) );
MOB_INDEX_DATA	 *new_mob_index		args ( ( void ) );
RESTR_DATA       *new_restriction       args ( ( void ) );

CLASS_DATA       *new_prg               args( ( void ) );
FCT_DATA         *new_fct               args( ( void ) );  

#endif
