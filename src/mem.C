/***************************************************************************
 *  File: mem.c                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

// Added by SinaC 2001
#include "db.h"
#include "handler.h"
#include "lookup.h"
#include "olc_value.h"
// Added by SinaC 2003
#include "script2.h"
#include "update.h"
#include "faction.h"
#include "config.h"
#include "scrhash.h"


RESET_DATA *new_reset_data( void )
{
    RESET_DATA *pReset = new RESET_DATA;

    pReset->next        =   NULL;
    pReset->command     =   'X';
    pReset->arg1        =   0;
    pReset->arg2        =   0;
    pReset->arg3        =   0;
    pReset->arg4	=   0;

    top_reset++;

    return pReset;
}


AREA_DATA *new_area( void )
{
    AREA_DATA *pArea = new AREA_DATA;
    char buf[MAX_INPUT_LENGTH];

    top_area++;

    pArea->next             =   NULL;
    pArea->name             =   str_dup( "New area" );
/*    pArea->recall           =   ROOM_VNUM_TEMPLE;      ROM OLC */
    pArea->area_flags       =   AREA_ADDED;
    pArea->security         =   1;
    pArea->builders         =   str_dup( "None" );
    pArea->min_vnum            =   0;
    pArea->max_vnum            =   0;
    pArea->age              =   0;
    pArea->nplayer          =   0;
    // Added by SinaC 2001
    pArea->low_range        = 500;
    pArea->high_range       = 0;
    pArea->empty            =   TRUE;              /* ROM patch */
    sprintf( buf, "area%d.are", pArea->vnum );
    pArea->file_name        =   str_dup( buf );
    pArea->vnum             =   top_area-1;

    return pArea;
}

EXIT_DATA *new_exit( void )
{
    EXIT_DATA *pExit = new EXIT_DATA;

    top_exit++;

    pExit->u1.to_room   =   NULL;                  /* ROM OLC */
/*  pExit->vnum         =   0;                        ROM OLC */
    pExit->exit_info    =   0;
    pExit->key          =   0;
    pExit->keyword      =   &str_empty[0];
    pExit->description  =   &str_empty[0];
    pExit->rs_flags     =   0;

    return pExit;
}


ROOM_INDEX_DATA *new_room_index( void )
{
    ROOM_INDEX_DATA *pRoom = new ROOM_INDEX_DATA;

    top_room++;

    pRoom->next             =   NULL;
    pRoom->people           =   NULL;
    pRoom->contents         =   NULL;
    pRoom->extra_descr      =   NULL;
    pRoom->area             =   NULL;

    for (int door=0; door < MAX_DIR; door++ )
        pRoom->exit[door]   =   NULL;

    pRoom->name             =   &str_empty[0];
    pRoom->description      =   &str_empty[0];
    pRoom->owner	    =	&str_empty[0];
    pRoom->vnum             =   0;
    // Modified by SinaC 2001
    pRoom->bstat(flags)       =   0;
    pRoom->bstat(light)       =   0;
    pRoom->bstat(sector)      =   0;
    pRoom->clan		    =	0;
    pRoom->bstat(healrate)	    =   100;
    pRoom->bstat(manarate)	    =   100;
    // Added by SinaC 2001 for mental user
    pRoom->bstat(psprate)	    =   100;

    // Added by SinaC 2001 for maximal room size
    pRoom->bstat(maxsize)         = SIZE_NOSIZE;

    // Added by SinaC 2003 for room programs, removed by SinaC 2003 we don't need that
    //pRoom->program = default_room_class;
    //pRoom->clazz = default_room_class;

    pRoom->guild		= 0; // Not a guild

    // Added by SinaC 2003 for modifiable repop time
    pRoom->current_time_repop = MAX_REPOP_TIME; // so the room will be immediatly updated
    pRoom->time_between_repop = BASE_REPOP_TIME;
    pRoom->time_between_repop_people = BASE_REPOP_TIME_PEOPLE;

    pRoom->program = default_room_class;
    pRoom->clazz = default_room_class;

    pRoom->valid = TRUE; // SinaC 2003

    return pRoom;
}

SHOP_DATA *new_shop( void )
{
    SHOP_DATA *pShop = new SHOP_DATA;
    int buy;

    pShop->next         =   NULL;
    pShop->keeper       =   0;

    for ( buy=0; buy<MAX_TRADE; buy++ )
        pShop->buy_type[buy]    =   0;

    pShop->profit_buy   =   100;
    pShop->profit_sell  =   100;
    pShop->open_hour    =   0;
    pShop->close_hour   =   23;

    top_shop++;

    return pShop;
}


OBJ_INDEX_DATA *new_obj_index( void ) {
    OBJ_INDEX_DATA *pObj = new OBJ_INDEX_DATA;
    int value;

    pObj->next          =   NULL;
    pObj->extra_descr   =   NULL;
    pObj->affected      =   NULL;
    pObj->area          =   NULL;
    pObj->name          =   str_dup( "no name" );
    pObj->short_descr   =   str_dup( "(no short description)" );
    pObj->description   =   str_dup( "(no description)" );
    pObj->vnum          =   0;
    pObj->item_type     =   ITEM_TRASH;
    pObj->extra_flags   =   0;
    pObj->wear_flags    =   0;
    pObj->count         =   0;
    pObj->weight        =   0;
    pObj->cost          =   0;
    // Added by SinaC 2003, size is not anymore a restriction but an obj stat
    pObj->size          =   SIZE_NOSIZE;

    // Modified by SinaC 2001
    //pObj->material      =   str_dup( "unknown" );      /* ROM */
    pObj->material        = 0;


    pObj->condition     =   100;                        /* ROM */
    for ( value = 0; value < 5; value++ )               /* 5 - ROM */
        pObj->value[value]  =   0;

    pObj->new_format    = TRUE; /* ROM */

    top_obj_index++;

    return pObj;
}


MOB_INDEX_DATA *new_mob_index( void ) {
    MOB_INDEX_DATA *pMob = new MOB_INDEX_DATA;

    pMob->next          =   NULL;
    pMob->spec_fun      =   NULL;
    pMob->pShop         =   NULL;
    pMob->area          =   NULL;
    pMob->player_name   =   str_dup( "no name" );
    pMob->short_descr   =   str_dup( "(no short description)" );
    //    pMob->long_descr    =   str_dup( "(no long description)\n\r" );
    pMob->long_descr    =   str_dup( "(no long description)" );
    pMob->description   =   &str_empty[0];
    pMob->vnum          =   0;
    pMob->count         =   0;
    pMob->killed        =   0;
    pMob->killed_by     =   0;
    pMob->sex           =   0;
    pMob->level         =   0;
    pMob->act           =   ACT_IS_NPC;
    pMob->affected_by   =   0;
  // Modified by SinaC 2000 for alignment/etho
    pMob->align.alignment     =   0;
    pMob->align.etho    = 0;

    pMob->hitroll	=   0;
    //pMob->race          =   race_lookup( "human" ); /* - Hugin */
    pMob->race          = DEFAULT_RACE;
//    pMob->form          =   0;           /* ROM patch -- Hugin */
//    pMob->parts         =   0;           /* ROM patch -- Hugin */
// Added by SinaC 2000
    pMob->form          =   race_table[pMob->race].form;
    pMob->parts         =   race_table[pMob->race].parts;

    pMob->imm_flags     =   0;           /* ROM patch -- Hugin */
    pMob->res_flags     =   0;           /* ROM patch -- Hugin */
    pMob->vuln_flags    =   0;           /* ROM patch -- Hugin */
    pMob->material      =   str_dup("unknown"); /* -- Hugin */
    pMob->off_flags     =   0;           /* ROM patch -- Hugin */
    pMob->size          =   SIZE_MEDIUM; /* ROM patch -- Hugin */
    pMob->ac[AC_PIERCE]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_BASH]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_SLASH]	=   0;           /* ROM patch -- Hugin */
    pMob->ac[AC_EXOTIC]	=   0;           /* ROM patch -- Hugin */
    pMob->hit[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->hit[DICE_BONUS]	=   0;   /* ROM patch -- Hugin */
    pMob->mana[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->mana[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->mana[DICE_BONUS]	=   0;   /* ROM patch -- Hugin */
    // Added by SinaC 2001 for mental user
    pMob->psp[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->psp[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->psp[DICE_BONUS]	=   0;   /* ROM patch -- Hugin */

    pMob->damage[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_TYPE]	=   0;   /* ROM patch -- Hugin */
    pMob->damage[DICE_NUMBER]	=   0;   /* ROM patch -- Hugin */
    pMob->start_pos             =   POS_STANDING; /*  -- Hugin */
    pMob->default_pos           =   POS_STANDING; /*  -- Hugin */
    pMob->wealth                =   0;

    pMob->dam_type              = 0;

    //pMob->faction               = get_neutral_faction();
    if ( fBootDb )
      pMob->faction             = -1; // so faction will be fix in fix_faction, only when booting
    else
      pMob->faction             = get_neutral_faction(); // when creating with OLC, set neutral

    pMob->new_format            = TRUE;  /* ROM */

    top_mob_index++;

    return pMob;
}


/****************************** Script Memory **********************************/
CLASS_DATA * new_prg() {
  // Modified by SinaC 2001
  CLASS_DATA * prg;
  if ( ( prg = (CLASS_DATA *)GC_MALLOC(sizeof(*prg)) ) == NULL ) {
    bug("Memory allocation error: new_prg");
    exit(-1);
  }
  prg->name = NULL;
  prg->methods = NULL;

  if ( ( prg->methods = (FCT_DATA**)GC_MALLOC(HASH_FCT*sizeof(FCT_DATA*))) == NULL ) {
    bug("Memory allocation error: methods");
    exit(-1);
  }
  prg->next = NULL;
  prg->flags = 0;
  prg->parents_count = 0;
  prg->parents = NULL;
  prg->file = str_dup( DEFAULT_SCRIPT_FILENAME );
  prg->next = NULL;

  return prg;
}

FCT_DATA * new_fct() {  
  // Modified by SinaC 2001
  FCT_DATA * fct ;
  if ( ( fct = (FCT_DATA *)GC_MALLOC(sizeof(*fct)) ) == NULL ) {
    bug("Memory allocation error: new_fct");
    exit(-1);
  }
  fct->predfct = NULL;
  fct->code = NULL;
  fct->name = NULL;
  fct->next = NULL;
  fct->nbparm = 0;
  fct->scriptCode = NULL;
  fct->scriptCodeSaved = NULL;
  fct->flags = 0;
  SET_BIT(fct->flags, FCT_NO_CODE );
  
  return fct;
}
