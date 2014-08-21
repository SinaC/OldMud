/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "classes.h"
#include "act_move.h"
#include "act_comm.h"
#include "handler.h"
#include "db.h"
#include "comm.h"
#include "update.h"
#include "restriction.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "wiznet.h"
#include "magic.h"
#include "interp.h"
#include "ability.h"
#include "clan.h"
#include "utils.h"


// maximum number of identical item a player could buy at a time
//  buy 999999 pie will not work        SinaC 2000
#define MAX_BUY_ONE_TIME (100)


/*
 * Local functions.
 */
#define CD CHAR_DATA
#define OD OBJ_DATA
bool	remove_obj	args( (CHAR_DATA *ch, int iWear, bool fReplace ) );
void	wear_obj	args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) );
CD *	find_keeper	args( (CHAR_DATA *ch ) );
int	get_cost	args( (CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy ) );
void 	obj_to_keeper	args( (OBJ_DATA *obj, CHAR_DATA *ch ) );
OD *	get_obj_keeper	args( (CHAR_DATA *ch,CHAR_DATA *keeper,const char *argument));

#undef OD
#undef	CD


// Removed by SinaC 2003
//// added by SinaC 2000
////  to avoid the switch 'fades into existence' during combat
////  with an item giving invisible flag
//// I know, this trick is really a shit ;)
////bool HasItemGivingFlag( CHAR_DATA *ch, int flag )
//bool HasItemGivingFlag( CHAR_DATA *ch, long flag ) {
//  AFFECT_DATA *paf;
//  OBJ_DATA *obj;
//  int iWear;
//
//  for ( iWear = 0; iWear < MAX_WEAR; iWear++ )  {
//    if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
//      continue;
//
//    if (!obj->enchanted) {
//      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
//	if ( attr_table[ paf->location ].bits != NULL   
//	     && ( paf->modifier == flag ) )
//	  return TRUE;
//    }
//    else
//      for ( paf = obj->affected; paf != NULL; paf = paf->next )
//	if ( attr_table[ paf->location ].bits != NULL   
//	     && ( paf->modifier == flag ) )
//	  return TRUE;
//  }
//    
//  return FALSE;
//}


/* RT part of the corpse looting code */

bool can_loot(CHAR_DATA *ch, OBJ_DATA *obj) {
  CHAR_DATA *owner, *wch;

  if (IS_IMMORTAL(ch))
    return TRUE;

  if (!obj->owner || obj->owner == NULL)
    return TRUE;

  owner = NULL;
  for ( wch = char_list; wch != NULL ; wch = wch->next )
    if (!str_cmp(wch->name,obj->owner))
      owner = wch;

  if (owner == NULL)
    return TRUE;

  if (!str_cmp(ch->name,owner->name))
    return TRUE;

  // Modified by SinaC 2003, clan test
  if (!IS_NPC(owner) && ( IS_SET(owner->act,PLR_CANLOOT) || owner->clan != 0 ) )
    return TRUE;

  if (is_same_group(ch,owner))
    return TRUE;

  return FALSE;
}

/* Donate command added by Seytan 1997 */
/* Corrected by Oxtal so timered items can be donated */

void do_donate( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *pRoom,*lastroom;

  if ( argument[0]=='\0' ) {
    send_to_char( "Donate what?\n\r", ch );
    return;
  }
    
  if ( ( obj = get_obj_carry( ch, argument, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }
    
  if ( !can_drop_obj( ch, obj ) ) {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  /* Removed by Oxtal 1997
     if( IS_SET(obj->extra_flags,ITEM_DONATED) )
     {
     send_to_char( "This is already a DONATED item.\n\r",ch);
     return;
     }
    
     if (obj->timer)
     {
     send_to_char( "Only permanent items may go in this room.\n\r",ch);
     return;
     }*/
    
  /* Find the donation room */
  //pRoom = get_room_index( ROOM_VNUM_DONATION );
  pRoom = get_donation_room( ch );
    
  if(pRoom!=NULL) {
    if (obj->timer) {
      SET_OBJ_STAT( obj, ITEM_HAD_TIMER );
    }
    else
      obj->timer = number_range(100,200);
          
    obj_from_char( obj );
    obj_to_room( obj, pRoom );

    /* Set donated bit */
    SET_OBJ_STAT( obj, ITEM_DONATED);

    // Added by SinaC 2001
    recomproom(pRoom);
    
    act( "You have donated $p.", ch, obj, NULL, TO_CHAR );
    
    lastroom = ch->in_room;
    ch->in_room = pRoom;  /* act in donation room */
    act( "$n has donated $p.", ch, obj, NULL, TO_ROOM );
    ch->in_room = lastroom;
  }
  else
    bug("do_donate: room not found for player [%s] clan [%s] hometown [%s]!",
	NAME(ch), get_clan_table(ch->clan)->name,
	(IS_NPC(ch)||ch->pcdata->hometown<0)?"none":hometown_table[ch->pcdata->hometown].name);
}        

void get_obj( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container ) {
  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];

  if ( !CAN_WEAR(obj, ITEM_TAKE) ) {
    send_to_char( "You can't take that.\n\r", ch );
    return;
  }

  // Added by SinaC 2000 for object ownership
  if ( obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp( obj->owner, ch->name ) 
       && !IS_IMMORTAL(ch) ){
    send_to_charf(ch,
		  "You can't get this object, you're not the owner, %s is the owner.\n\r",
		  obj->owner);
    return;
  }

  if ( ch->carry_number + get_obj_number( obj ) > can_carry_n( ch ) ) {
    act( "$d: you can't carry that many items.",
	 ch, NULL, obj->name, TO_CHAR );
    return;
  }

  if ((!obj->in_obj || obj->in_obj->carried_by != ch)
      &&  (get_carry_weight(ch) + get_obj_weight(obj) > can_carry_w(ch)))  {
    act( "$d: you can't carry that much weight.",
	 ch, NULL, obj->name, TO_CHAR );
    return;
  }

  if (!can_loot(ch,obj)) {
    act("Corpse looting is not permitted.",ch,NULL,NULL,TO_CHAR );
    return;
  }

  if (obj->in_room != NULL) {
    for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
      if (gch->on == obj) {
	act("$N appears to be using $p.",
	    ch,obj,gch,TO_CHAR);
	return;
      }
  }


  if ( container != NULL ) {
    if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  get_trust(ch) < obj->level) {
      send_to_char("You are not powerful enough to use it.\n\r",ch);
      return;
    }

    if (container->pIndexData->vnum == OBJ_VNUM_PIT
	&&  !CAN_WEAR(container, ITEM_TAKE)
	&&  !IS_OBJ_STAT(obj,ITEM_HAD_TIMER))
      obj->timer = 0;	

    // Added by SinaC 2003
    Value args[] = {ch};
    bool result = objprog(obj,ch,"onGetting",args);
    if ( result ) // result == 1, we don't get the item
      return;
    result = roomprog(ch->in_room,ch,"onGetting",args);
    if ( result ) // result == 1, we don't get the item
      return;

    act( "You get $p from $P.", ch, obj, container, TO_CHAR );
    act( "$n gets $p from $P.", ch, obj, container, TO_ROOM );
    REM_OBJ_STAT(obj,ITEM_HAD_TIMER);
    obj_from_obj( obj );
  }
  else {
    //if (ch->in_room->vnum == ROOM_VNUM_DONATION) {
    if ( IS_SET( ch->in_room->cstat(flags), ROOM_DONATION ) ) { // SinaC 2003
      if  (get_trust(ch) < obj->level) {
	send_to_char("You are not powerful enough to use it.\n\r",ch);
	return;
      } 
      else if (!IS_OBJ_STAT(obj,ITEM_HAD_TIMER)) {
	  REM_OBJ_STAT(obj,ITEM_HAD_TIMER);
	  obj->timer = 0;
      }
    }

    // Added by SinaC 2003
    Value args[] = {ch};
    bool result = objprog(obj,ch,"onGetting",args);
    if ( result ) // result == 1, we don't get the item
      return;
    result = roomprog(ch->in_room,ch,"onGetting",args);
    if ( result ) // result == 1, we don't get the item
      return;

    act( "You get $p.", ch, obj, NULL, TO_CHAR );
    act( "$n gets $p.", ch, obj, NULL, TO_ROOM );
    obj_from_room( obj );
  }

  if ( obj->item_type == ITEM_MONEY) {
    ch->silver += obj->value[0];
    ch->gold += obj->value[1];
    if (IS_SET(ch->act,PLR_AUTOSPLIT)) { /* AUTOSPLIT code */
      members = 0;
      for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
	if (!IS_AFFECTED(gch,AFF_CHARM) && is_same_group( gch, ch ) )
	  members++;
      }

      if ( members > 1 && (obj->value[0] > 1 || obj->value[1])) {
	sprintf(buffer,"%d %d",obj->value[0],obj->value[1]);
	do_split(ch,buffer);	
      }
    }

    extract_obj( obj );
  }
  else {
    obj_to_char( obj, ch );
    // Added by SinaC 2001
    ROOM_INDEX_DATA *pRoom = ch->in_room;
    OBJPROG(obj,ch,"onGot",ch);
    ROOMPROG(pRoom,ch,"onGot",ch);
  }

  return;
}

void do_get( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  OBJ_DATA *container;
  bool found;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"from"))
    argument = one_argument(argument,arg2);

  /* Get type. */
  if ( arg1[0] == '\0' ) {
    send_to_char( "Get what?\n\r", ch );
    return;
  }

  if ( arg2[0] == '\0' ) {
    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) ) {
      /* 'get obj' */
      obj = get_obj_list( ch, arg1, ch->in_room->contents );
      if ( obj == NULL ) {
	act( "I see no $T here.", ch, NULL, arg1, TO_CHAR );
	return;
      }

      get_obj( ch, obj, NULL );
    }
    else {
      /* 'get all' or 'get all.obj' */
      found = FALSE;
      for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;
	if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	     &&   can_see_obj( ch, obj ) ) {
	  found = TRUE;
	  get_obj( ch, obj, NULL );
	}
      }

      if ( !found ) {
	if ( arg1[3] == '\0' )
	  send_to_char( "I see nothing here.\n\r", ch );
	else
	  act( "I see no $T here.", ch, NULL, &arg1[4], TO_CHAR );
      }
    }
  }
  else {
    /* 'get ... container' */
    if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) ) {
      send_to_char( "You can't do that.\n\r", ch );
      return;
    }

    if ( ( container = get_obj_here( ch, arg2 ) ) == NULL )	{
      act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
      return;
    }

    switch ( container->item_type )	{
    default:
      send_to_char( "That's not a container.\n\r", ch );
      return;

    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
      break;

    case ITEM_CORPSE_PC:

      if (!can_loot(ch,container)) {
	send_to_char( "You can't do that.\n\r", ch );
	return;
      }
    }

    if ( container->item_type == ITEM_CONTAINER // SinaC 2003
	 && IS_SET(container->value[1], CONT_CLOSED) )	{
      act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
      return;
    }

    if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) ) {
      /* 'get obj container' */
      obj = get_obj_list( ch, arg1, container->contains );
      if ( obj == NULL ) {
	act( "I see nothing like that in the $T.",
	     ch, NULL, arg2, TO_CHAR );
	return;
      }
      get_obj( ch, obj, container );
    }
    else {
      /* 'get all container' or 'get all.obj container' */
      found = FALSE;
      for ( obj = container->contains; obj != NULL; obj = obj_next ) {
	obj_next = obj->next_content;
	if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	     &&   can_see_obj( ch, obj ) ) {
	  found = TRUE;
	  if (container->pIndexData->vnum == OBJ_VNUM_PIT
	      &&  !IS_IMMORTAL(ch)) {
	    send_to_char("Don't be so greedy!\n\r",ch);
	    return;
	  }
	  get_obj( ch, obj, container );
	}
      }

      if ( !found ) {
	if ( arg1[3] == '\0' )
	  act( "I see nothing in the $T.",
	       ch, NULL, arg2, TO_CHAR );
	else
	  act( "I see nothing like that in the $T.",
	       ch, NULL, arg2, TO_CHAR );
      }
    }
  }

  return;
}

void do_put( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *container;
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if (!str_cmp(arg2,"in") || !str_cmp(arg2,"on"))
    argument = one_argument(argument,arg2);

  if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
    send_to_char( "Put what in what?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg2, "all" ) || !str_prefix( "all.", arg2 ) ) {
    send_to_char( "You can't do that.\n\r", ch );
    return;
  }

  if ( ( container = get_obj_here( ch, arg2 ) ) == NULL ) {
    act( "I see no $T here.", ch, NULL, arg2, TO_CHAR );
    return;
  }

  if ( container->item_type != ITEM_CONTAINER ) {
    send_to_char( "That's not a container.\n\r", ch );
    return;
  }

  if ( IS_SET(container->value[1], CONT_CLOSED) ) {
    act( "The $d is closed.", ch, NULL, container->name, TO_CHAR );
    return;
  }

  if ( str_cmp( arg1, "all" ) && str_prefix( "all.", arg1 ) ) {
    /* 'put obj container' */
    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

    if ( obj == container ) {
      send_to_char( "You can't fold it into itself.\n\r", ch );
      return;
    }

    if ( !can_drop_obj( ch, obj ) )	{
      send_to_char( "You can't let go of it.\n\r", ch );
      return;
    }
    /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
    if ( obj->item_type == ITEM_GRENADE && obj->value[1] == GRENADE_PULLED ) {
      send_to_char( "Are you crazy? Drop that grenade!\n\r", ch );
      return;
    }
    */

    // Added by SinaC 2001 for unique item
    if ( IS_SET( obj->extra_flags, ITEM_UNIQUE ) ){
      send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
      return;
    }

    if (WEIGHT_MULT(obj) != 100) {
      send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
      return;
    }

    if (get_obj_weight( obj ) + get_true_weight( container )
	> (container->value[0] * 10) 
	||  get_obj_weight(obj) > (container->value[3] * 10)) {
      send_to_char( "It won't fit.\n\r", ch );
      return;
    }
	
    // Added by SinaC 2003
    Value args[] = {ch,container};
    bool result = objprog( obj,ch, "onPutting", args);
    if ( result ) // if onPutting returns 1,  we don't put the item
      return;

    if (container->pIndexData->vnum == OBJ_VNUM_PIT 
	&&  !CAN_WEAR(container,ITEM_TAKE))
      if (obj->timer) {
	SET_OBJ_STAT(obj,ITEM_HAD_TIMER);
      }
      else
	obj->timer = number_range(100,200);

    obj_from_char( obj );
    obj_to_obj( obj, container );

    if (IS_SET(container->value[1],CONT_PUT_ON)) {
      act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
      act("You put $p on $P.",ch,obj,container, TO_CHAR);
    }
    else {
      act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
      act( "You put $p in $P.", ch, obj, container, TO_CHAR );
    }

    // Added by SinaC 2003
    OBJPROG( obj,ch, "onPut", ch, container );
  }
  else {
    /* 'put all container' or 'put all.obj container' */
    for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;

      /* Removed by SinaC 2003, can be emulate with script
      // Added by SinaC 2000 for grenade
      if ( obj->item_type == ITEM_GRENADE && obj->value[1] == GRENADE_PULLED ) {
	send_to_char( "Are you crazy? Drop that grenade!\n\r", ch );
	continue;
      }
      */
      // Added by SinaC 2001 for unique item
      if ( IS_SET( obj->extra_flags, ITEM_UNIQUE ) ){
	send_to_char("You have a feeling that would be a bad idea.\n\r",ch);
	return;
      }

      if ( ( arg1[3] == '\0' || is_name( &arg1[4], obj->name ) )
	   &&   can_see_obj( ch, obj )
	   &&   WEIGHT_MULT(obj) == 100
	   &&   obj->wear_loc == WEAR_NONE
	   &&   obj != container
	   &&   can_drop_obj( ch, obj )
	   &&   get_obj_weight( obj ) + get_true_weight( container )
	   <= (container->value[0] * 10) 
	   // modified by SinaC 2000,  the  <=  was a  <  before
	   &&   get_obj_weight(obj) <= (container->value[3] * 10)) {

	// Added by SinaC 2003
	Value args[] = {ch,container};
	bool result = objprog( obj,ch, "onPutting", args);
	if ( result ) // if onPutting returns 1,  we don't put the item
	  return;

	if (container->pIndexData->vnum == OBJ_VNUM_PIT
	    &&  !CAN_WEAR(obj, ITEM_TAKE) )
	  if (obj->timer) {
	    SET_OBJ_STAT(obj,ITEM_HAD_TIMER);
	  } 
	  else
	    obj->timer = number_range(100,200);

	obj_from_char( obj );
	obj_to_obj( obj, container );

	if (IS_SET(container->value[1],CONT_PUT_ON)) {
	  act("$n puts $p on $P.",ch,obj,container, TO_ROOM);
	  act("You put $p on $P.",ch,obj,container, TO_CHAR);
	}
	else {
	  act( "$n puts $p in $P.", ch, obj, container, TO_ROOM );
	  act( "You put $p in $P.", ch, obj, container, TO_CHAR );
	}

	// Added by SinaC 2003
	OBJPROG( obj,ch, "onPut", ch, container );

      }
    }
  }

  return;
}

void do_drop( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;
  bool found;

  argument = one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Drop what?\n\r", ch );
    return;
  }

  if ( is_number( arg ) ) {
    /* 'drop NNNN coins' */
    int amount, gold = 0, silver = 0;

    amount   = atoi(arg);
    argument = one_argument( argument, arg );
    if ( amount <= 0
	 || ( str_cmp( arg, "coins" ) && str_cmp( arg, "coin" ) && 
	      str_cmp( arg, "gold"  ) && str_cmp( arg, "silver") ) ) {
      send_to_char( "Sorry, you can't do that.\n\r", ch );
      return;
    }

    if ( !str_cmp( arg, "coins") || !str_cmp(arg,"coin") 
	 ||   !str_cmp( arg, "silver")) {
      if (ch->silver < amount) {
	send_to_char("You don't have that much silver.\n\r",ch);
	return;
      }

      ch->silver -= amount;
      silver = amount;
    }

    else {
      if (ch->gold < amount) {
	send_to_char("You don't have that much gold.\n\r",ch);
	return;
      }

      ch->gold -= amount;
      gold = amount;
    }

    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;

      switch ( obj->pIndexData->vnum ) {
      case OBJ_VNUM_SILVER_ONE:
	silver += 1;
	extract_obj(obj);
	break;

      case OBJ_VNUM_GOLD_ONE:
	gold += 1;
	extract_obj( obj );
	break;

      case OBJ_VNUM_SILVER_SOME:
	silver += obj->value[0];
	extract_obj(obj);
	break;

      case OBJ_VNUM_GOLD_SOME:
	gold += obj->value[1];
	extract_obj( obj );
	break;

      case OBJ_VNUM_COINS:
	silver += obj->value[0];
	gold += obj->value[1];
	extract_obj(obj);
	break;
      }
    }

    obj_to_room( create_money( gold, silver ), ch->in_room );
    act( "$n drops some coins.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "OK.\n\r", ch );
    return;
  }

  if ( str_cmp( arg, "all" ) && str_prefix( "all.", arg ) ) {
    /* 'drop obj' */
    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

    if ( !can_drop_obj( ch, obj ) )	{
      send_to_char( "You can't let go of it.\n\r", ch );
      return;
    }

    /* Donation Room. New. */
    //if( ch->in_room->vnum == ROOM_VNUM_DONATION ) { /* in donation room */
    if ( IS_SET( ch->in_room->cstat(flags), ROOM_DONATION ) ) { // SinaC 2003
      /* Modified by Oxtal 1997 */
      if (obj->timer) {
	SET_OBJ_STAT( obj, ITEM_HAD_TIMER );
      }
      else
	obj->timer = number_range(100,200);
      SET_OBJ_STAT( obj, ITEM_DONATED);
    }
    
    // Added by SinaC 2003
    Value args[] = {ch};
    bool result = objprog( obj,ch, "onDropping", args);
    if ( result ) // if onDropping returns 1,  we don't drop the item
      return;
    result = roomprog( ch->in_room, ch, "onDropping", args );
    if ( result )
      return; // if onDropping returns 1, we don't drop item

    obj_from_char( obj );
    obj_to_room( obj, ch->in_room );
    act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
    act( "You drop $p.", ch, obj, NULL, TO_CHAR );
    if (IS_OBJ_STAT(obj,ITEM_MELT_DROP)) {
      act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
      act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
      extract_obj(obj);
    }
    else {
      ROOM_INDEX_DATA *pRoom = ch->in_room;
      OBJPROG(obj,ch,"onDropped",ch);
      ROOMPROG(pRoom,ch,"onDropped",ch); // SinaC 2003
    }
  }
  else {
    /* 'drop all' or 'drop all.obj' */
    found = FALSE;
    for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;

      if ( ( arg[3] == '\0' || is_name( &arg[4], obj->name ) )
	   &&   can_see_obj( ch, obj )
	   &&   obj->wear_loc == WEAR_NONE
	   &&   can_drop_obj( ch, obj ) ) {
	/* Donation Room. New. */
		
	//if( ch->in_room->vnum == ROOM_VNUM_DONATION ) { /* in donation room */
	if ( IS_SET( ch->in_room->cstat(flags), ROOM_DONATION ) ) { // SinaC 2003
	  if (obj->timer) {
	    send_to_char( "Only permanent items may go in this room.\n\r",ch);
	    continue;
	  }
	  else { // Modified by SinaC 2001
			        /* obj->timer = number_range(100,200); */
	    SET_BIT( obj->extra_flags, ITEM_DONATED);
	    SET_BIT( obj->base_extra, ITEM_DONATED);
	  }
	}	        
	found = TRUE;

	// Added by SinaC 2003
	Value args[] = {ch};
	bool result = objprog( obj,ch, "onDropping", args);
	if ( result ) // if onDropping returns 1,  we don't drop the item
	  return;
	result = roomprog( ch->in_room, ch, "onDropping", args );
	if ( result )
	  return; // if onDropping returns 1, we don't drop item


	obj_from_char( obj );
	obj_to_room( obj, ch->in_room );
	act( "$n drops $p.", ch, obj, NULL, TO_ROOM );
	act( "You drop $p.", ch, obj, NULL, TO_CHAR );
	if (IS_OBJ_STAT(obj,ITEM_MELT_DROP)) {
	  act("$p dissolves into smoke.",ch,obj,NULL,TO_ROOM);
	  act("$p dissolves into smoke.",ch,obj,NULL,TO_CHAR);
	  extract_obj(obj);
	}
	else {
	  ROOM_INDEX_DATA *pRoom = ch->in_room;
	  OBJPROG(obj,ch,"onDropped",ch);
	  ROOMPROG(pRoom,ch,"onDropped",ch); // SinaC 2003
	}
      }
    }

    if ( !found ) {
      if ( arg[3] == '\0' )
	act( "You are not carrying anything.",
	     ch, NULL, arg, TO_CHAR );
      else
	act( "You are not carrying any $T.",
	     ch, NULL, &arg[4], TO_CHAR );
    }
  }

  // Added by SinaC 2001
  recomproom(ch->in_room);

  return;
}

void do_give( CHAR_DATA *ch, const char *argument ) {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA  *obj;

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
    send_to_char( "Give what to whom?\n\r", ch );
    return;
  }

  if ( is_number( arg1 ) ) {
    /* 'give NNNN coins victim' */
    int amount;
    bool silver;

    amount   = atoi(arg1);
    if ( amount <= 0
	 || ( str_cmp( arg2, "coins" ) && str_cmp( arg2, "coin" ) && 
	      str_cmp( arg2, "gold"  ) && str_cmp( arg2, "silver")) ) {
      send_to_char( "Sorry, you can't do that.\n\r", ch );
      return;
    }

    silver = str_cmp(arg2,"gold");

    argument = one_argument( argument, arg2 );
    if ( arg2[0] == '\0' ) {
      send_to_char( "Give what to whom?\n\r", ch );
      return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    if ( (!silver && ch->gold < amount) || (silver && ch->silver < amount) ) {
      send_to_char( "You haven't got that much.\n\r", ch );
      return;
    }

    if (silver) {
      ch->silver		-= amount;
      victim->silver 	+= amount;
    }
    else {
      ch->gold		-= amount;
      victim->gold	+= amount;
    }

    sprintf(buf,"$n gives you %d %s.",amount, silver ? "silver" : "gold");
    act( buf, ch, NULL, victim, TO_VICT    );
    act( "$n gives $N some coins.",  ch, NULL, victim, TO_NOTVICT );
    sprintf(buf,"You give $N %d %s.",amount, silver ? "silver" : "gold");
    act( buf, ch, NULL, victim, TO_CHAR    );
	
    //Value args[] = { ch, silver?amount:amount*100 };
    Value args[] = { ch, amount, silver };
    mobprog(victim,ch,"onBribe",args);

    /* Replaced with a mobprog MoneyChanger
     *if (IS_NPC(victim) && IS_SET(victim->act,ACT_IS_CHANGER)) {
     * int change;
     *
     *change = (silver ? 95 * amount / 100 / 100 
     *	: 95 * amount);
     *
     *if (!silver && change > victim->silver)
     *victim->silver += change;
     *
     *if (silver && change > victim->gold)
     *victim->gold += change;
     *
     *if (change < 1 && can_see(victim,ch)) {
     *act(
     *    "$n tells you 'I'm sorry, you did not give me enough to change.'"
     *    ,victim,NULL,ch,TO_VICT);
     *ch->reply = victim;
     *sprintf(buf,"%d %s %s", 
     *	amount, silver ? "silver" : "gold",ch->name);
     *do_give(victim,buf);
     *}
     *else if (can_see(victim,ch)) {
     *sprintf(buf,"%d %s %s", 
     *	change, silver ? "gold" : "silver",ch->name);
     *do_give(victim,buf);
     *if (silver) {
     *  sprintf(buf,"%d silver %s", 
     *	  (95 * amount / 100 - change * 100),ch->name);
     *  do_give(victim,buf);
     *}
     *act("$n tells you 'Thank you, come again.'",
     *    victim,NULL,ch,TO_VICT);
     *ch->reply = victim;
     *}
     *}
     */
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if ( obj->wear_loc != WEAR_NONE ) {
    send_to_char( "You must remove it first.\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg2 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /* Removed by SinaC 2003, can be emulate with script
  // Added by SinaC 2000 for grenade
  if ( obj->item_type == ITEM_GRENADE && obj->value[1] == GRENADE_PULLED ) {
    send_to_char( "Don't be so cruel! Drop that grenade!\n\r", ch );
    return;
  }
  */

  /* Moved down by SinaC 2002
   *if (IS_NPC(victim) && victim->pIndexData->pShop != NULL) {
   *act("$N tells you 'Sorry, you'll have to sell that.'",
   *ch,NULL,victim,TO_CHAR);
   *ch->reply = victim;
   *return;
   *}
   */

  if ( !can_drop_obj( ch, obj ) ) {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  // Added by SinaC 2001, this test was not there before
  if ( !IS_NPC(ch)
       || IS_AFFECTED(ch,AFF_CHARM)
       || ch->master != NULL
       || ch->leader != NULL ) {
    if ( victim->carry_number + get_obj_number( obj ) > can_carry_n( victim ) ) {
      act( "$N has $S hands full.", ch, NULL, victim, TO_CHAR );
      return;
    }
    if (get_carry_weight(victim) + get_obj_weight(obj) > can_carry_w( victim ) ) {
      act( "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR );
      return;
    }
  }

  if ( !can_see_obj( victim, obj ) ) {
    act( "$N can't see it.", ch, NULL, victim, TO_CHAR );
    return;
  }

  if (IS_NPC(victim) && victim->pIndexData->pShop != NULL) {
    act("$N tells you 'Sorry, you'll have to sell that.'",
	ch,NULL,victim,TO_CHAR);
    ch->reply = victim;
    return;
  }

  // Added by SinaC 2003
  Value args[] = {ch,victim};
  //bool result = objprog(obj,victim,"onGiving",args);
  bool result = objprog(obj,ch,"onGiving",args);
  if ( result ) // result == 1, we don't give the item
    return;
  Value args2[] = {ch,obj};
  //result = mobprog(ch,victim,"onGiving",args);
  result = mobprog(victim,ch,"onGiving",args2);
  if ( result )
    return;

  obj_from_char( obj );
  obj_to_char( obj, victim );

  act( "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT );
  act( "$n gives you $p.",   ch, obj, victim, TO_VICT    );
  act( "You give $p to $N.", ch, obj, victim, TO_CHAR    );

  Value args3[] = {ch,obj};
  mobprog(victim,ch,"onGiven",args3);

  // Added by SinaC 2003
  Value args4[] = {ch,victim};
  objprog(obj,ch,"onGiven",args4);

  return;
}

void do_fill( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  OBJ_DATA *fountain;
  bool found;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Fill what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  found = FALSE;
  for ( fountain = ch->in_room->contents; fountain != NULL;
	fountain = fountain->next_content ) {
    if ( fountain->item_type == ITEM_FOUNTAIN ) {
      found = TRUE;
      break;
    }
  }

  if ( !found ) {
    send_to_char( "There is no fountain here!\n\r", ch );
    return;
  }

  if ( obj->item_type != ITEM_DRINK_CON ) {
    send_to_char( "You can't fill that.\n\r", ch );
    return;
  }

  if ( obj->value[1] != 0 && obj->value[2] != fountain->value[2] ) {
    send_to_char( "There is already another liquid in it.\n\r", ch );
    return;
  }

  if ( obj->value[1] >= obj->value[0] ) {
    send_to_char( "Your container is full.\n\r", ch );
    return;
  }

  sprintf(buf,"You fill $p with %s from $P.",
	  liq_table[fountain->value[2]].liq_name);
  act( buf, ch, obj,fountain, TO_CHAR );
  sprintf(buf,"$n fills $p with %s from $P.",
	  liq_table[fountain->value[2]].liq_name);
  act(buf,ch,obj,fountain,TO_ROOM);
  obj->baseval[2] = fountain->value[2];
  obj->baseval[1] = obj->value[0];
  recompobj(obj);
  return;
}

void do_pour (CHAR_DATA *ch, const char *argument) {
  char arg[MAX_STRING_LENGTH],buf[MAX_STRING_LENGTH];
  OBJ_DATA *out, *in;
  CHAR_DATA *vch = NULL;
  int amount;

  argument = one_argument(argument,arg);
    
  if (arg[0] == '\0' || argument[0] == '\0') {
    send_to_char("Pour what into what?\n\r",ch);
    return;
  }
    

  if ((out = get_obj_carry(ch,arg, ch)) == NULL) {
    send_to_char("You don't have that item.\n\r",ch);
    return;
  }

  if (out->item_type != ITEM_DRINK_CON) {
    send_to_char("That's not a drink container.\n\r",ch);
    return;
  }

  if (!str_cmp(argument,"out")) {
    if (out->value[1] == 0) {
      send_to_char("It's already empty.\n\r",ch);
      return;
    }

    out->baseval[1] = 0;
    out->baseval[3] = 0;
    recompobj(out);
    sprintf(buf,"You invert $p, spilling %s all over the ground.",
	    liq_table[out->value[2]].liq_name);
    act(buf,ch,out,NULL,TO_CHAR);
	
    sprintf(buf,"$n inverts $p, spilling %s all over the ground.",
	    liq_table[out->value[2]].liq_name);
    act(buf,ch,out,NULL,TO_ROOM);
    return;
  }

  if ((in = get_obj_here(ch,argument)) == NULL) {
    vch = get_char_room(ch,argument);

    if (vch == NULL) {
      send_to_char("Pour into what?\n\r",ch);
      return;
    }

    in = get_eq_char(vch,WEAR_HOLD);

    if (in == NULL) {
      send_to_char("They aren't holding anything.",ch);
      return;
    }
  }

  if (in->item_type != ITEM_DRINK_CON) {
    send_to_char("You can only pour into other drink containers.\n\r",ch);
    return;
  }
    
  if (in == out) {
    send_to_char("You cannot change the laws of physics!\n\r",ch);
    return;
  }

  if (in->value[1] != 0 && in->value[2] != out->value[2]) {
    send_to_char("They don't hold the same liquid.\n\r",ch);
    return;
  }

  if (out->value[1] == 0) {
    act("There's nothing in $p to pour.",ch,out,NULL,TO_CHAR);
    return;
  }

  if (in->value[1] >= in->value[0]) {
    act("$p is already filled to the top.",ch,in,NULL,TO_CHAR);
    return;
  }

  amount = UMIN(out->value[1],in->value[0] - in->value[1]);

  in->baseval[1] += amount;
  out->baseval[1] -= amount;
  in->baseval[2] = out->value[2];
  recompobj(in); recompobj(out);

  if (vch == NULL) {
    sprintf(buf,"You pour %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    act(buf,ch,out,in,TO_CHAR);
    sprintf(buf,"$n pours %s from $p into $P.",
	    liq_table[out->value[2]].liq_name);
    act(buf,ch,out,in,TO_ROOM);
  }
  else {
    sprintf(buf,"You pour some %s for $N.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,NULL,vch,TO_CHAR);
    sprintf(buf,"$n pours you some %s.",
	    liq_table[out->value[2]].liq_name);
    act(buf,ch,NULL,vch,TO_VICT);
    sprintf(buf,"$n pours some %s for $N.",
            liq_table[out->value[2]].liq_name);
    act(buf,ch,NULL,vch,TO_NOTVICT);
	
  }
}

void do_drink( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int amount;
  int liquid;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    for ( obj = ch->in_room->contents; obj; obj = obj->next_content ) {
      if ( obj->item_type == ITEM_FOUNTAIN )
	break;
    }

    if ( obj == NULL ) {
      send_to_char( "Drink what?\n\r", ch );
      return;
    }
  }
  else {
    if ( ( obj = get_obj_here( ch, arg ) ) == NULL ) {
      send_to_char( "You can't find it.\n\r", ch );
      return;
    }
  }

  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10 ) {
    send_to_char( "You fail to reach your mouth.  *Hic*\n\r", ch );
    return;
  }

  switch ( obj->item_type ) {
  default:
    send_to_char( "You can't drink from that.\n\r", ch );
    return;

  case ITEM_FOUNTAIN:
    if ( ( liquid = obj->value[2] )  < 0 ) {
      bug( "Do_drink: bad liquid number %d.", liquid );
      liquid = obj->baseval[2] = 0;
      recompobj(obj);
    }
    amount = liq_table[liquid].liq_affect[4] * 3;
    break;

  case ITEM_DRINK_CON:
    if ( obj->value[1] <= 0 ) {
      send_to_char( "It is already empty.\n\r", ch );
      return;
    }

    if ( ( liquid = obj->value[2] )  < 0 ) {
      bug( "Do_drink: bad liquid number %d.", liquid );
      liquid = obj->baseval[2] = 0;
      recompobj(obj);
    }

    amount = liq_table[liquid].liq_affect[4];
    amount = UMIN(amount, obj->value[1]);
    break;
  }
  if (!IS_NPC(ch) && !IS_IMMORTAL(ch) 
      &&  ch->pcdata->condition[COND_FULL] > 45) {
    send_to_char("You're too full to drink more.\n\r",ch);
    return;
  }

  act( "$n drinks $T from $p.",
       ch, obj, liq_table[liquid].liq_name, TO_ROOM );
  act( "You drink $T from $p.",
       ch, obj, liq_table[liquid].liq_name, TO_CHAR );

  gain_condition( ch, COND_DRUNK, amount * liq_table[liquid].liq_affect[COND_DRUNK] / 36 );
  gain_condition( ch, COND_FULL, amount * liq_table[liquid].liq_affect[COND_FULL] / 4 );
  gain_condition( ch, COND_THIRST, amount * liq_table[liquid].liq_affect[COND_THIRST] / 10 );
  gain_condition(ch, COND_HUNGER, amount * liq_table[liquid].liq_affect[COND_HUNGER] / 2 );

  // Modified by SinaC 2001
  if ( !IS_NPC(ch) 
       && !IS_SET( ch->cstat(form), FORM_UNDEAD )
       && ch->pcdata->condition[COND_DRUNK]  > 10 )
    send_to_char( "You feel drunk.\n\r", ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   > 40 )
    send_to_char( "You are full.\n\r", ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] > 40 )
    send_to_char( "Your thirst is quenched.\n\r", ch );
	
  if ( obj->value[3] != 0 ) {
    /* The drink was poisoned ! */

    // Added by SinaC 2003, resist poison ?
    if ( saves_spell( obj->level, ch, DAM_POISON) ) {
      act("$n turns slightly green, but it passes.",ch,NULL,NULL,TO_ROOM);
      send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
    } 
    else {
      
      AFFECT_DATA af;
      
      act( "$n chokes and gags.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You choke and gag.\n\r", ch );
      
      //afsetup(af,CHAR,affected_by,OR,AFF_POISON,3*amount,number_fuzzy(amount),gsn_poison);
      createaff(af,3*amount,number_fuzzy(amount),gsn_poison,0,AFFECT_ABILITY);
      addaff(af,CHAR,affected_by,OR,AFF_POISON);
      affect_join( ch, &af );
    }
  }
	
  /* modified by Sinac 1997 */	
  if ((obj->baseval[0] > 0)&&(obj->baseval[0]!=1000)) {
    obj->baseval[1] -= amount;
    recompobj(obj);
  }

  return;
}



void do_eat( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Eat what?\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }


  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if ( !IS_IMMORTAL(ch) ) {
    if ( obj->item_type != ITEM_FOOD && obj->item_type != ITEM_PILL ) {
      send_to_char( "That's not edible.\n\r", ch );
      return;
    }

    if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL] > 40 ) {   
      send_to_char( "You are too full to eat more.\n\r", ch );
      return;
    }
  }

  act( "$n eats $p.",  ch, obj, NULL, TO_ROOM );
  act( "You eat $p.", ch, obj, NULL, TO_CHAR );

  switch ( obj->item_type ) {

  case ITEM_FOOD:
    if ( !IS_NPC(ch) ) {
      int condition;
      
      condition = ch->pcdata->condition[COND_HUNGER];
      gain_condition( ch, COND_FULL, obj->value[0] );
      gain_condition( ch, COND_HUNGER, obj->value[1]);
      if ( condition == 0 && ch->pcdata->condition[COND_HUNGER] > 0 )
	send_to_char( "You are no longer hungry.\n\r", ch );
      else if ( ch->pcdata->condition[COND_FULL] > 40 )
	send_to_char( "You are full.\n\r", ch );
    } 
    if ( obj->value[3] != 0 ) {
      
      /* The food was poisoned! */
      
      // Added by SinaC 2003, resist poison ?
      if ( saves_spell( obj->level, ch, DAM_POISON) ) {
	act("$n turns slightly green, but it passes.",ch,NULL,NULL,TO_ROOM);
	send_to_char("You feel momentarily ill, but it passes.\n\r",ch);
      } 
      else {

	AFFECT_DATA af;
	
	act( "$n chokes and gags.", ch, 0, 0, TO_ROOM );
	send_to_char( "You choke and gag.\n\r", ch );
	
	createaff(af,2*obj->value[0],number_fuzzy(obj->value[0]),gsn_poison,0,AFFECT_ABILITY);
	addaff(af,CHAR,affected_by,OR,AFF_POISON);
	affect_join( ch, &af );
	//afsetup(af,CHAR,affected_by,OR,AFF_POISON,2*obj->value[0],number_fuzzy(obj->value[0]),gsn_poison);
	//affect_join( ch, &af );
      }
    }
    break;

  case ITEM_PILL:
    gain_condition(ch, COND_HUNGER, 1 );

    obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
    obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
    break;
  }

  extract_obj( obj );
  return;
}



/*
 * Remove an object.
 */
bool remove_obj( CHAR_DATA *ch, int iWear, bool fReplace )
{
  OBJ_DATA *obj;

  if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
    return TRUE;

  if ( !fReplace )
    return FALSE;

  if ( IS_SET(obj->extra_flags, ITEM_NOREMOVE) 
       && !IS_IMMORTAL(ch)) {
    act( "You can't remove $p.", ch, obj, NULL, TO_CHAR );
    return FALSE;
  }

  // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj,ch, "onRemoving", args);
  if ( result ) // if onRemoving returns 1,  we don't remove the item
    return FALSE;

  unequip_char( ch, obj );
  act( "$n stops using $p.", ch, obj, NULL, TO_ROOM );
  act( "You stop using $p.", ch, obj, NULL, TO_CHAR );

  // Added by SinaC 2001
  //OBJPROG( obj, ch, "onRemoved", ch );
  return TRUE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 */
void wear_obj( CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace ) {
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj2;

  if ( IS_AFFECTED2( ch, AFF2_NOEQUIPMENT ) 
       && !CAN_WEAR(obj,ITEM_WEAR_FLOAT)
       /*&& !CAN_WEAR(obj,ITEM_BRAND )  removed by SinaC 2003*/ ) {
    send_to_char("You can't wear any equipment.\n\r",ch);
    return;
  }

  if ( ch->level < obj->level ) {
    sprintf( buf, "You must be level %d to use this object.\n\r",
	     obj->level );
    send_to_char( buf, ch );
    act( "$n tries to use $p, but is too inexperienced.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2000
  if ( obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp(obj->owner,ch->name) ){
    send_to_charf(ch, 
		  "You're not the owner of this object, %s is the owner.\n\r",
		  obj->owner);
    act( "$n tries to use $p, but $e's not the owner.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2003 for object's size
  if (!check_size( ch, obj, FALSE ) )
    return;

  // Added by SinaC 2000 for object restriction
  if (!check_restriction( ch, obj ) ){
    act("You don't meet the requirements to use $p.", ch, obj, NULL, TO_CHAR );
    act("$n doesn't meet the requirements to use $p.", ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2001 for object condition
  if ( obj->condition == 0 ) {
    act("$p is in a so bad condition, that it is unsable.", ch, obj, NULL, TO_CHAR );
    return;
  }

  // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj,ch, "onWearing", args);
  if ( result ) // if onWearing returns 1,  we don't wear the item
    return;

  // Msg send in equip_char now, SinaC 2003
  if ( obj->item_type == ITEM_LIGHT ) {
    if ( !remove_obj( ch, WEAR_LIGHT, fReplace ) )
      return;
    //act( "$n lights $p and holds it.", ch, obj, NULL, TO_ROOM );
    //act( "You light $p and hold it.",  ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LIGHT );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_FINGER ) ) {
    if ( get_eq_char( ch, WEAR_FINGER_L ) != NULL
	 &&   get_eq_char( ch, WEAR_FINGER_R ) != NULL
	 &&   !remove_obj( ch, WEAR_FINGER_L, fReplace )
	 &&   !remove_obj( ch, WEAR_FINGER_R, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_FINGER_L ) == NULL ) {
      //act( "$n wears $p on $s left finger.",    ch, obj, NULL, TO_ROOM );
      //act( "You wear $p on your left finger.",  ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_FINGER_L );
      return;
    }

    if ( get_eq_char( ch, WEAR_FINGER_R ) == NULL )	{
      //act( "$n wears $p on $s right finger.",   ch, obj, NULL, TO_ROOM );
      //act( "You wear $p on your right finger.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_FINGER_R );
      return;
    }

    bug( "Wear_obj: no free finger." );
    send_to_char( "You already wear two rings.\n\r", ch );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_NECK ) ) {
    if ( get_eq_char( ch, WEAR_NECK_1 ) != NULL
	 &&   get_eq_char( ch, WEAR_NECK_2 ) != NULL
	 &&   !remove_obj( ch, WEAR_NECK_1, fReplace )
	 &&   !remove_obj( ch, WEAR_NECK_2, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_NECK_1 ) == NULL ) {
      //act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
      //act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_NECK_1 );
      return;
    }

    if ( get_eq_char( ch, WEAR_NECK_2 ) == NULL ) {
      //act( "$n wears $p around $s neck.",   ch, obj, NULL, TO_ROOM );
      //act( "You wear $p around your neck.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_NECK_2 );
      return;
    }

    bug( "Wear_obj: no free neck." );
    send_to_char( "You already wear two neck items.\n\r", ch );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_BODY ) ) {
    if ( !remove_obj( ch, WEAR_BODY, fReplace ) )
      return;
    //act( "$n wears $p on $s torso.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your torso.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_BODY );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_HEAD ) ) {
    if ( !remove_obj( ch, WEAR_HEAD, fReplace ) )
      return;
    //act( "$n wears $p on $s head.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your head.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_HEAD );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_LEGS ) ) {
    if ( !remove_obj( ch, WEAR_LEGS, fReplace ) )
      return;
    //act( "$n wears $p on $s legs.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your legs.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_LEGS );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_FEET ) ) {
    if ( !remove_obj( ch, WEAR_FEET, fReplace ) )
      return;
    //act( "$n wears $p on $s feet.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your feet.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_FEET );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_HANDS ) ) {
    if ( !remove_obj( ch, WEAR_HANDS, fReplace ) )
      return;
    //act( "$n wears $p on $s hands.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your hands.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_HANDS );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_ARMS ) ) {
    if ( !remove_obj( ch, WEAR_ARMS, fReplace ) )
      return;
    //act( "$n wears $p on $s arms.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your arms.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_ARMS );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_ABOUT ) ) {
    if ( !remove_obj( ch, WEAR_ABOUT, fReplace ) )
      return;
    //act( "$n wears $p about $s torso.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p about your torso.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_ABOUT );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_WAIST ) ) {
    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
      return;
    //act( "$n wears $p about $s waist.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p about your waist.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_WAIST );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_WRIST ) ) {
    if ( get_eq_char( ch, WEAR_WRIST_L ) != NULL
	 &&   get_eq_char( ch, WEAR_WRIST_R ) != NULL
	 &&   !remove_obj( ch, WEAR_WRIST_L, fReplace )
	 &&   !remove_obj( ch, WEAR_WRIST_R, fReplace ) )
      return;

    if ( get_eq_char( ch, WEAR_WRIST_L ) == NULL ) {
      //act( "$n wears $p around $s left wrist.", ch, obj, NULL, TO_ROOM );
      //act( "You wear $p around your left wrist.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_WRIST_L );
      return;
    }

    if ( get_eq_char( ch, WEAR_WRIST_R ) == NULL ) {
      //act( "$n wears $p around $s right wrist.", ch, obj, NULL, TO_ROOM );
      //act( "You wear $p around your right wrist.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_WRIST_R );
      return;
    }

    bug( "Wear_obj: no free wrist." );
    send_to_char( "You already wear two wrist items.\n\r", ch );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_SHIELD ) ) {
    OBJ_DATA *weapon;

    if ( !remove_obj( ch, WEAR_SHIELD, fReplace ) )
      return;

    weapon = get_eq_char(ch,WEAR_WIELD);
    if (weapon != NULL
	&& ( ( ch->cstat(size) < SIZE_LARGE
	     && IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS) )
	     || weapon->value[0] == WEAPON_RANGED )
	//&& ch->cstat(size) < SIZE_LARGE
	//&&  IS_WEAPON_STAT(weapon,WEAPON_TWO_HANDS)) {
	) {
      send_to_char("Your hands are tied up with your weapon!\n\r",ch);
      return;
    }
    if (get_eq_char (ch, WEAR_SECONDARY) != NULL) {
      send_to_char ("You cannot use a shield while using 2 weapons.\n\r",ch);
      return;
    }

    //act( "$n wears $p as a shield.", ch, obj, NULL, TO_ROOM );
    //act( "You wear $p as a shield.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_SHIELD );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WIELD ) ) {
    int sn,skill;

    if ( !remove_obj( ch, WEAR_WIELD, fReplace ) )
      return;

    if ( !IS_NPC(ch)
	 && get_obj_weight(obj) > (str_app[ch->cstat(STR)].wield * 10)) {
      send_to_char( "It is too heavy for you to wield.\n\r", ch );
      return;
    }

    if (   
	!IS_NPC(ch)
	&& ( ( ch->cstat(size) < SIZE_LARGE
	     && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) )
	     || obj->value[0] == WEAPON_RANGED )
	&& ( get_eq_char(ch,WEAR_HOLD) != NULL      /* Added by Sinac */
	     || get_eq_char(ch,WEAR_SHIELD) != NULL 
	     || get_eq_char(ch,WEAR_SECONDARY) != NULL )) { /* Added by Oxtal */
      send_to_char("You need two hands free for that weapon.\n\r",ch);
      return;
    }
	
    /* check if the secondary weapon is at least half as light as the primary weapon */
    /* By Oxtal */ 
    if ( get_eq_char(ch,WEAR_SECONDARY) != NULL 
	 && get_obj_weight(get_eq_char(ch,WEAR_SECONDARY))*2 > get_obj_weight(obj) ) {
      send_to_char ("Your secondary weapon has to be considerably lighter than the primary one.\n\r",ch);
      return;
    }

    //act( "$n wields $p.", ch, obj, NULL, TO_ROOM );
    //act( "You wield $p.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_WIELD );

    /* Removed by SinaC 2003
    sn = get_weapon_sn(ch);

    if (sn == gsn_hand_to_hand)
      return;

    skill = get_weapon_skill(ch,sn);

    if (skill >= 100)
      act("$p feels like a part of you!",ch,obj,NULL,TO_CHAR);
    else if (skill > 85)
      act("You feel quite confident with $p.",ch,obj,NULL,TO_CHAR);
    else if (skill > 70)
      act("You are skilled with $p.",ch,obj,NULL,TO_CHAR);
    else if (skill > 50)
      act("Your skill with $p is adequate.",ch,obj,NULL,TO_CHAR);
    else if (skill > 25)
      act("$p feels a little clumsy in your hands.",ch,obj,NULL,TO_CHAR);
    else if (skill > 1)
      act("You fumble and almost drop $p.",ch,obj,NULL,TO_CHAR);
    else
      act("You don't even know which end is up on $p.",
	  ch,obj,NULL,TO_CHAR);
    */

    return;
  }

  if ( CAN_WEAR( obj, ITEM_HOLD ) ) {
    if ( !remove_obj( ch, WEAR_HOLD, fReplace ) )
      return;

    /* Added by Sinac */
      
    if ( ( obj2 = get_eq_char( ch, WEAR_WIELD ) ) != NULL )  
      if ( ( ( ch->cstat(size) < SIZE_LARGE
	       && IS_WEAPON_STAT(obj2,WEAPON_TWO_HANDS) )
	     || obj2->value[0] == WEAPON_RANGED )
	  //IS_WEAPON_STAT( obj2, WEAPON_TWO_HANDS ) 
	  // && ch->cstat(size) < SIZE_LARGE ) {
	   ) {
	send_to_char( "You cannot hold an item while using a two-handed weapon.\n\r", ch );
	return;
      }
    /* Added by Oxtal */
      
    if ( get_eq_char( ch, WEAR_SECONDARY ) ) {
      send_to_char( "Your off hand already holds a weapon.\n\r", ch );
      return;
    }
      
    //act( "$n holds $p in $s hand.",   ch, obj, NULL, TO_ROOM );
    //act( "You hold $p in your hand.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_HOLD );
    return;
  }

  if ( CAN_WEAR( obj, ITEM_WEAR_EAR ) ) {
    if ( get_eq_char( ch, WEAR_EAR_L ) != NULL 
	 && get_eq_char( ch, WEAR_EAR_R ) != NULL
	 && !remove_obj( ch, WEAR_EAR_L, fReplace ) 
	 && !remove_obj( ch, WEAR_EAR_R, fReplace ) )
      return;
        
    if ( get_eq_char( ch, WEAR_EAR_L ) == NULL ) {
      //act( "$n wears $p on $s left ear.",    ch, obj, NULL, TO_ROOM );
      //act( "You wear $p on your left ear.",  ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_EAR_L );
      return;
    }

    if ( get_eq_char( ch, WEAR_EAR_R ) == NULL ) {
      //act( "$n wears $p on $s right ear.",   ch, obj, NULL, TO_ROOM );
      //act( "You wear $p on your right ear.", ch, obj, NULL, TO_CHAR );
      equip_char( ch, obj, WEAR_EAR_R );
      return;
    }
    bug( "Wear_obj: no free ear." );
    send_to_char( "You already wear two earrings.\n\r", ch );
    return;
  }
    
  if ( CAN_WEAR( obj, ITEM_WEAR_EYES ) ) {
    if ( !remove_obj( ch, WEAR_EYES, fReplace ) )
      return;
    //act( "$n wears $p on $s eyes.",   ch, obj, NULL, TO_ROOM );
    //act( "You wear $p on your eyes.", ch, obj, NULL, TO_CHAR );
    equip_char( ch, obj, WEAR_EYES );
    return;
  }
                                                            
  if ( CAN_WEAR(obj,ITEM_WEAR_FLOAT) ) {
    if (!remove_obj(ch,WEAR_FLOAT, fReplace) )
      return;
    //act("$n releases $p to float next to $m.",ch,obj,NULL,TO_ROOM);
    //act("You release $p and it floats next to you.",ch,obj,NULL,TO_CHAR);
    equip_char(ch,obj,WEAR_FLOAT);
    return;
  }

  /* Removed by SinaC 2003
  // Added by SinaC 2001 for brand mark
  if ( CAN_WEAR(obj,ITEM_BRAND) ) {
    if (!remove_obj(ch,WEAR_BRAND, fReplace) )
      return;
    act("You wear $p as a brand mark.",ch,obj,NULL,TO_CHAR);
    equip_char(ch,obj,WEAR_BRAND);
    return;
  }
  */

  if ( fReplace )
    send_to_char( "You can't wear, wield, or hold that.\n\r", ch );

  return;
}



void do_wear( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Wear, wield, or hold what?\n\r", ch );
    return;
  }

  if ( !str_cmp( arg, "all" ) ) {
    OBJ_DATA *obj_next;

    for ( obj = ch->carrying; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;
      if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj ) )
	wear_obj( ch, obj, FALSE );
    }
    return;
  }
  else {
    if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
      send_to_char( "You do not have that item.\n\r", ch );
      return;
    }

    wear_obj( ch, obj, TRUE );
  }

  return;
}



void do_remove( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Remove what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_wear( ch, arg, TRUE ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  remove_obj( ch, obj->wear_loc, TRUE );
  return;
}



void do_sacrifice( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;
  int silver;
    
  /* variables for AUTOSPLIT */
  CHAR_DATA *gch;
  int members;
  char buffer[100];


  one_argument( argument, arg );

  if ( arg[0] == '\0' || !str_cmp( arg, ch->name ) )  {
    // Modified by SinaC 2001 for god
    //act( "$n offers $mself to Mota, who graciously declines.",
    //ch, NULL, NULL, TO_ROOM );
    //send_to_char(
    //"Mota appreciates your offer and may accept it later.\n\r", ch );
    sprintf( buf, 
	     "$n offers $mself to %s, who graciously declines.",
	     char_god_name( ch ) );//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
    act( buf, ch, NULL, NULL, TO_ROOM );
    send_to_charf( ch,
		   "%s appreciates your offer and may accept it later.\n\r", 
		   char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
    return;
  }

  //  if( ch->in_room->vnum == ROOM_VNUM_DONATION ) { /* in donation room */
  if ( IS_SET( ch->in_room->cstat(flags), ROOM_DONATION ) ) { // SinaC 2003
    send_to_char("Not in the donation room!\n\r",ch);
    return;
  }
    
  if ( !str_cmp( arg, "all" ) ) {
    OBJ_DATA *obj_next;
 
    for ( obj = ch->in_room->contents; obj != NULL; obj = obj_next ) {
      obj_next = obj->next_content;
      do_sacrifice(ch,obj->name);
    }
    return;
  }

  obj = get_obj_list( ch, arg, ch->in_room->contents );
  if ( obj == NULL ) {
    send_to_char( "You can't find it.\n\r", ch );
    return;
  }

  if ( obj->item_type == ITEM_CORPSE_PC ) {
    if (obj->contains) {
      // Modified by SinaC 2001 for god
      //send_to_char( "Mota wouldn't like that.\n\r",ch);
      send_to_charf( ch,
		     "%s wouldn't like that.\n\r",
		     char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
      return;
    }
  }


  if ( !CAN_WEAR(obj, ITEM_TAKE) 
       // Modified by SinaC 2001
       //|| CAN_WEAR(obj, ITEM_NO_SAC)) {
       || IS_SET(obj->extra_flags, ITEM_NOSAC)) {
    act( "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR );
    return;
  }

  if (obj->in_room != NULL) {
    for (gch = obj->in_room->people; gch != NULL; gch = gch->next_in_room)
      if (gch->on == obj) {
	act("$N appears to be using $p.",
	    ch,obj,gch,TO_CHAR);
	return;
      }
  }
		
  silver = UMAX(1,obj->level * 3);

  if (obj->item_type != ITEM_CORPSE_NPC && obj->item_type != ITEM_CORPSE_PC)
    silver = UMIN(silver,obj->cost);

  if (silver == 1)
    // Modified by SinaC 2001 for god
    //send_to_char( "Mota gives you one silver coin for your sacrifice.\n\r", ch );
    send_to_charf( ch,
		   "%s gives you one silver coin for your sacrifice.\n\r",
		   char_god_name( ch ) );//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god) );
  else {
    // Modified by SinaC 2001 for god
    /*
      sprintf(buf,"Mota gives you %d silver coins for your sacrifice.\n\r",
      silver);
      send_to_char(buf,ch);
    */
    send_to_charf( ch, "%s gives you %d silver coin for your sacrifice.\n\r",
		   char_god_name( ch ), silver );//;IS_NPC(ch)?"Mota":god_name(ch->pcdata->god), silver );
  }
    
  ch->silver += silver;
    
  if (IS_SET(ch->act,PLR_AUTOSPLIT) ) { /* AUTOSPLIT code */
    members = 0;
    for (gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ) {
      if ( is_same_group( gch, ch ) )
	members++;
    }

    if ( members > 1 && silver > 1) {
      sprintf(buffer,"%d",silver);
      do_split(ch,buffer);	
    }
  }
  // Modified by SinaC 2001 for god
  //act( "$n sacrifices $p to Mota.", ch, obj, NULL, TO_ROOM );
  sprintf( buf,
	   "$n sacrifices $p to %s.",
	   char_god_name( ch ));//IS_NPC(ch)?"Mota":god_name(ch->pcdata->god));
  act( buf, ch, obj, NULL, TO_ROOM );
  wiznet("$N sends up $p as a burnt offering.",
	 ch,obj,WIZ_SACCING,0,0);
  extract_obj( obj );
  return;
}



void do_quaff( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Quaff what?\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }


  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
    send_to_char( "You do not have that potion.\n\r", ch );
    return;
  }

  if ( obj->item_type != ITEM_POTION ) {
    send_to_char( "You can quaff only potions.\n\r", ch );
    return;
  }

  if (ch->level < obj->level) {
    send_to_char("This liquid is too powerful for you to drink.\n\r",ch);
    return;
  }

  act( "$n quaffs $p.", ch, obj, NULL, TO_ROOM );
  act( "You quaff $p.", ch, obj, NULL ,TO_CHAR );

  gain_condition( ch, COND_THIRST, 1 );

  obj_cast_spell( obj->value[1], obj->value[0], ch, ch, NULL );
  obj_cast_spell( obj->value[2], obj->value[0], ch, ch, NULL );
  obj_cast_spell( obj->value[3], obj->value[0], ch, ch, NULL );
  obj_cast_spell( obj->value[4], obj->value[0], ch, ch, NULL ); //SinaC 2003, v4 was not used

  extract_obj( obj );
  return;
}


/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper( CHAR_DATA *ch )
{
  /*char buf[MAX_STRING_LENGTH];*/
  CHAR_DATA *keeper;
  SHOP_DATA *pShop;

  pShop = NULL;
  for ( keeper = ch->in_room->people; keeper; keeper = keeper->next_in_room ) {
    if ( IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL )
      break;
  }

  if ( pShop == NULL ) {
    send_to_char( "You can't do that here.\n\r", ch );
    return NULL;
  }

  /*
     * Undesirables.
     *
    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_KILLER) )
    {
	do_say( keeper, "Killers are not welcome!" );
	sprintf( buf, "%s the KILLER is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }

    if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_THIEF) )
    {
	do_say( keeper, "Thieves are not welcome!" );
	sprintf( buf, "%s the THIEF is over here!\n\r", ch->name );
	do_yell( keeper, buf );
	return NULL;
    }
	*/
    /*
     * Shop hours.
     */
  if ( time_info.hour < pShop->open_hour ) {
    do_say( keeper, "Sorry, I am closed. Come back later." );
    return NULL;
  }
    
  if ( time_info.hour > pShop->close_hour ) {
    do_say( keeper, "Sorry, I am closed. Come back tomorrow." );
    return NULL;
  }

  /*
     * Invisible or hidden people.
     */
  if ( !can_see( keeper, ch ) ) {
    do_say( keeper, "I don't trade with folks I can't see." );
    return NULL;
  }

  return keeper;
}

/* insert an object at the right spot for the keeper */
void obj_to_keeper( OBJ_DATA *obj, CHAR_DATA *ch )
{
  OBJ_DATA *t_obj, *t_obj_next;

  /* see if any duplicates are found */
  for (t_obj = ch->carrying; t_obj != NULL; t_obj = t_obj_next) {
    t_obj_next = t_obj->next_content;

    if (obj->pIndexData == t_obj->pIndexData 
	&&  !str_cmp(obj->short_descr,t_obj->short_descr)) {
      /* if this is an unlimited item, destroy the new one */
      if (IS_OBJ_STAT(t_obj,ITEM_INVENTORY)) {
	extract_obj(obj);
	return;
      }
      obj->cost = t_obj->cost; /* keep it standard */
      break;
    }
  }

  if (t_obj == NULL) {
    obj->next_content = ch->carrying;
    ch->carrying = obj;
  }
  else {
    obj->next_content = t_obj->next_content;
    t_obj->next_content = obj;
  }

  obj->carried_by      = ch;
  obj->in_room         = NULL;
  obj->in_obj          = NULL;
  ch->carry_number    += get_obj_number( obj );
  ch->carry_weight    += get_obj_weight( obj );
}

/* get an object from a shopkeeper's list */
OBJ_DATA *get_obj_keeper( CHAR_DATA *ch, CHAR_DATA *keeper, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int number;
  int count;
 
  number = number_argument( argument, arg );
  count  = 0;
  for ( obj = keeper->carrying; obj != NULL; obj = obj->next_content ) {
    if (obj->wear_loc == WEAR_NONE
        &&  can_see_obj( keeper, obj )
	&&  can_see_obj(ch,obj)
        &&  is_name( arg, obj->name ) ) {
      if ( ++count == number )
	return obj;
	
      /* skip other objects of the same name */
      while (obj->next_content != NULL
	     && obj->pIndexData == obj->next_content->pIndexData
	     && !str_cmp(obj->short_descr,obj->next_content->short_descr))
	obj = obj->next_content;
    }
  }
 
  return NULL;
}

int get_cost( CHAR_DATA *keeper, OBJ_DATA *obj, bool fBuy )
{
  SHOP_DATA *pShop;
  int cost;

  if ( obj == NULL || !obj->valid || ( pShop = keeper->pIndexData->pShop ) == NULL )
    return 0;

  if ( fBuy ) {
    cost = obj->cost * pShop->profit_buy  / 100;
  }
  else {
    OBJ_DATA *obj2;
    int itype;

    cost = 0;
    for ( itype = 0; itype < MAX_TRADE; itype++ ) {
      if ( obj->item_type == pShop->buy_type[itype] ) {
	cost = obj->cost * pShop->profit_sell / 100;
	break;
      }
    }

    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT))
      for ( obj2 = keeper->carrying; obj2; obj2 = obj2->next_content ) {
	if ( obj->pIndexData == obj2->pIndexData
	     &&   !str_cmp(obj->short_descr,obj2->short_descr) )
	  if (IS_OBJ_STAT(obj2,ITEM_INVENTORY))
	    cost /= 2;
	  else
	    cost = cost * 3 / 4;
      }
  }

  if ( obj->item_type == ITEM_STAFF || obj->item_type == ITEM_WAND ) {
    if (obj->value[1] == 0)
      cost /= 4;
    else
      cost = cost * obj->value[2] / obj->value[1];
  }

  return cost;
}



void do_buy( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  int cost, roll;

  // Added by SinaC 2000
  if (IS_NPC(ch)) {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_char( "Buy what?\n\r", ch );
    return;
  }
    
  //  smash_tilde(argument);

  // Modified by SinaC 2001
  if ( IS_SET(ch->in_room->cstat(flags), ROOM_PET_SHOP) ) {
    char arg[MAX_INPUT_LENGTH];
    char buf[MAX_STRING_LENGTH];
    CHAR_DATA *pet;
    ROOM_INDEX_DATA *pRoomIndexNext;
    ROOM_INDEX_DATA *in_room;

    if ( IS_NPC(ch) )
      return;

    argument = one_argument(argument,arg);

    /* hack to make new thalos pets work */
    if (ch->in_room->vnum == 9621)
      pRoomIndexNext = get_room_index(9706);
    else
      pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );
    if ( pRoomIndexNext == NULL ) {
      bug( "Do_buy: bad pet shop at vnum %d.", ch->in_room->vnum );
      send_to_char( "Sorry, you can't buy that here.\n\r", ch );
      return;
    }

    in_room     = ch->in_room;
    ch->in_room = pRoomIndexNext;
    pet         = get_char_room( ch, arg );
    ch->in_room = in_room;

    if ( pet == NULL || !IS_SET(pet->act, ACT_PET) ) {
      send_to_char( "Sorry, you can't buy that here.\n\r", ch );
      return;
    }

    if ( ch->pet != NULL ) {
      send_to_char("You already own a pet.\n\r",ch);
      return;
    }

    cost = 10 * pet->level * pet->level;

    if ( (ch->silver + 100 * ch->gold) < cost ) {
      send_to_char( "You can't afford it.\n\r", ch );
      return;
    }

    if ( ch->level < pet->level ) {
      send_to_char(
		   "You're not powerful enough to master this pet.\n\r", ch );
      return;
    }

    /* haggle */
    roll = number_percent();
    if (roll < get_ability(ch,gsn_haggle)
	&& (( !IS_NPC(ch) && IS_SET( ch->act, PLR_AUTOHAGGLE) )
	    || IS_NPC(ch)) ) {
      cost -= cost / 2 * roll / 100;
      sprintf(buf,"You haggle the price down to %d coins.\n\r",cost);
      send_to_char(buf,ch);
      check_improve(ch,gsn_haggle,TRUE,4);
	
    }

    deduct_cost(ch,cost);
    pet			= create_mobile( pet->pIndexData );
    SET_BIT(pet->act, ACT_PET);
    SET_BIT(pet->bstat(affected_by), AFF_CHARM);
    //recompute(pet); -> NO NEED, DONE in char_to_room
    pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

    argument = one_argument( argument, arg );
    if ( arg[0] != '\0' ) {
      sprintf( buf, "%s %s", pet->name, arg );
      pet->name = str_dup( buf );
    }

    if ( arg[0] == '\0' )
      sprintf( buf, "%sA neck tag says 'I belong to %s'.\n\r",
	       pet->description, ch->name );
    else
      sprintf( buf, "%sA neck tag says 'My name is '%c%s' and I belong to %s'.\n\r",
	       pet->description, UPPER(arg[0]), arg+1, ch->name );
    pet->description = str_dup( buf );

    char_to_room( pet, ch->in_room );
    add_follower( pet, ch );
    pet->leader = ch;
    ch->pet = pet;
    send_to_char( "Enjoy your pet.\n\r", ch );
    act( "$n bought $N as a pet.", ch, NULL, pet, TO_ROOM );
    return;
  }

  /*

    ===========================================================================
    This snippet was written by Erwin S. Andreasen, 4u2@aabc.dk. You may use
    this code freely, as long as you retain my name in all of the files. You
    also have to mail me telling that you are using it. I am giving this,
    hopefully useful, piece of source code to you for free, and all I require
    from you is some feedback.

    Please mail me if you find any bugs or have any new ideas or just comments.

    All my snippets are publically available at:

    http://login.dknet.dk/~ea/

    If you do not have WWW access, try ftp'ing to login.dknet.dk and examine
    the /pub/ea directory.
    ===========================================================================


    Multiple object buy routine
    ---------------------------

    Last update:  Jan 25, 1997

    Should work on : ROM2.4 

    Version for Rom2.3 adapted by Seytan

    Fixed since last update[B:
    None

    Know bugs and limitations yet to be fixed:
    None?

    Comments:

    This change to do_buy allows the player to type e.g. buy 10 bread and get all
    the 10 pieces of bread at once.

    The code does check if the item is an item sold to the shopkeeper he only has
    one of.

    For ROM 2.3, you have to uncomment a single line near the beginning.


  */

  /* Insert this in the lower part of the do_buy routine, after the pet code */

  else {/* object purchase code begins HERE */

    char arg[MAX_INPUT_LENGTH]; /* Uncomment for ROM 2.3 */

    CHAR_DATA *keeper;
    OBJ_DATA *obj;
    int cost;
    char arg2[MAX_INPUT_LENGTH]; /* 2nd argument */
    int item_count = 1;          /* default: buy only 1 item */

    argument = one_argument (argument, arg);
    argument = one_argument (argument, arg2); /* get another argument, if any */
	
    if (arg2[0]) {/* more than one argument specified? then arg2[0] <> 0 */
      /* check if first of the 2 args really IS a number */

      if (!is_number(arg)) {
	send_to_char ("Syntax for BUY is: BUY [number] <item>\n\r\"number\" is an optional number of items to buy.\n\r",ch);
	return;
      }

      item_count = atoi (arg); /* first argument is the optional count */
      strcpy (arg,arg2);       /* copy the item name to its right spot */
    }

    if ( ( keeper = find_keeper( ch ) ) == NULL ) /* is there a shopkeeper here? */
      return;

    /* find the pointer to the object */
    obj  = get_obj_carry( keeper, arg, keeper );
    cost = get_cost( keeper, obj, TRUE );


/*
    log_stringf("do_buy  arg: %s  obj: %s",
		arg,
		obj?obj->name:"(none)");
*/


    if ( cost <= 0 || !can_see_obj( ch, obj ) ) {/* cant buy what you cant see */
      act( "$n tells you 'I don't sell that -- try 'list''.",keeper, NULL, ch, TO_VICT );
      ch->reply = keeper;
      return;
    }

    /* check for valid positive numeric value entered */
    /* The number has to be between 1 and MAX_BUY_ONE_TIME   SinaC 2000*/
    if (item_count < 1 || item_count > MAX_BUY_ONE_TIME ) {
      send_to_charf( ch, "Number must be between 1 and %d!\n\r", MAX_BUY_ONE_TIME);
      return;
    }

    /* can the character afford it ? */
    if ( (ch->silver + 100 * ch->gold) < (cost * item_count) ) {
      if (item_count == 1) {/* he only wanted to buy one */
	act( "$n tells you 'You can't afford to buy $p'.",keeper, obj, ch, TO_VICT );
      }
      else {
	char buf[MAX_STRING_LENGTH]; /* temp buffer */
	if ( ( (ch->silver + 100 * ch->gold) / cost) > 0) /* how many CAN he afford? */
	  sprintf (buf, "$n tells you 'You can only afford %ld of those!", ( (ch->silver + 100 * ch->gold) / cost));
	else /* not even a single one! what a bum! */
	  sprintf (buf, "$n tells you '%s? You must be kidding - you can't even afford a single one, let alone %d!'",capitalize(obj->short_descr), item_count);

	act(buf,keeper, obj, ch, TO_VICT );
	ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
	return;
      }

      ch->reply = keeper; /* like the character really would reply to the shopkeeper... */
      return;
    }

    /* Can the character use the item at all ? */
    if ( obj->level > ch->level ) {
      act( "$n tells you 'You can't use $p yet'.",
	   keeper, obj, ch, TO_VICT );
      ch->reply = keeper;
      return;
    }
    /* can the character carry more items? */
    if ( ch->carry_number + (get_obj_number(obj)*item_count) > can_carry_n( ch ) ) {
      send_to_char( "You can't carry that many items.\n\r", ch );
      return;
    }

    /* can the character carry more weight? */
    if ( ch->carry_weight + item_count*get_obj_weight(obj) > can_carry_w( ch ) ) {
      send_to_char( "You can't carry that much weight.\n\r", ch );
      return;
    }

    /* check for objects sold to the keeper */
    if ( (item_count > 1) && !IS_SET (obj->extra_flags,ITEM_INVENTORY)) {
      act( "$n tells you 'Sorry - $p is something I have only one of'.",keeper, obj, ch, TO_VICT );
      ch->reply = keeper;
      return;
    }

    // Added by SinaC 2000
    roll = number_percent();
    if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) 
	&& roll < get_ability(ch,gsn_haggle)
	&& (( !IS_NPC(ch) && IS_SET( ch->act, PLR_AUTOHAGGLE) )
	    || IS_NPC(ch))) {
      cost -= obj->cost / 2 * roll / 100;
      sprintf(buf,"You haggle the price down to %d coins.\n\r",cost*item_count);
      send_to_char( buf, ch );
      check_improve(ch,gsn_haggle,TRUE,4);
    }

    /* change this to reflect multiple items bought */
    if (item_count == 1) {
      act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
      act( "You buy $p.", ch, obj, NULL, TO_CHAR );
    }
    else {/* inform of multiple item purchase */
      char buf[MAX_STRING_LENGTH]; /* temporary buffer */

      /* "buys 5 * a piece of bread" seems to be the easiest and least gramatically incorrect solution. */
      sprintf (buf, "$n buys %d * $p.", item_count);
      act (buf, ch, obj, NULL, TO_ROOM); /* to char self */
      sprintf (buf, "You buy %d * $p.", item_count);
      act(buf, ch, obj, NULL, TO_CHAR ); /* to others */
    }

    deduct_cost(ch,cost*item_count);
    keeper->gold += cost*item_count/100;
    keeper->silver += cost*item_count - (cost * item_count/100) * 100;

    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) ) {/* 'permanent' item */
      /* item_count of items */
      for ( ; item_count > 0; item_count--) {/* create item_count objects */
	obj = create_object( obj->pIndexData, obj->level );
	obj_to_char( obj, ch );
	obj->timer = 0;
      }
    }
    else {/* single item */
      obj_from_char( obj );
      obj_to_char( obj, ch );
      obj->timer = 0;
    }
    return;
  } /* else */
} /* do_buy */


/* Removed by Seytan to add Multiple Object buy routine */

/*
    else
    {
	CHAR_DATA *keeper;
	OBJ_DATA *obj,*t_obj;
	char arg[MAX_INPUT_LENGTH];
	int number, count = 1;

	if ( ( keeper = find_keeper( ch ) ) == NULL )
	    return;

	number = mult_argument(argument,arg);
	obj  = get_obj_keeper( ch,keeper, arg );
	cost = get_cost( keeper, obj, TRUE );

	if (number < 1)
	{
	    act("$n tells you 'Get real!",keeper,NULL,ch,TO_VICT);
	    return;
	}

	if ( cost <= 0 || !can_see_obj( ch, obj ) )
	{
	    act( "$n tells you 'I don't sell that -- try 'list''.",
		keeper, NULL, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (!IS_OBJ_STAT(obj,ITEM_INVENTORY))
	{
	    for (t_obj = obj->next_content;
	     	 count < number && t_obj != NULL; 
	     	 t_obj = t_obj->next_content) 
	    {
	    	if (t_obj->pIndexData == obj->pIndexData
	    	&&  !str_cmp(t_obj->short_descr,obj->short_descr))
		    count++;
	    	else
		    break;
	    }

	    if (count < number)
	    {
	    	act("$n tells you 'I don't have that many in stock.",
		    keeper,NULL,ch,TO_VICT);
	    	ch->reply = keeper;
	    	return;
	    }
	}

	if ( (ch->silver + ch->gold * 100) < cost * number )
	{
	    if (number > 1)
		act("$n tells you 'You can't afford to buy that many.",
		    keeper,obj,ch,TO_VICT);
	    else
	    	act( "$n tells you 'You can't afford to buy $p'.",
		    keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}
	
	if ( obj->level > ch->level )
	{
	    act( "$n tells you 'You can't use $p yet'.",
		keeper, obj, ch, TO_VICT );
	    ch->reply = keeper;
	    return;
	}

	if (ch->carry_number +  number * get_obj_number(obj) > can_carry_n(ch))
	{
	    send_to_char( "You can't carry that many items.\n\r", ch );
	    return;
	}

	if ( ch->carry_weight + number * get_obj_weight(obj) > can_carry_w(ch))
	{
	    send_to_char( "You can't carry that much weight.\n\r", ch );
	    return;
	}
	*/
        /* haggle */
    /*	roll = number_percent();
	if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT) 
	&& roll < get_ability(ch,gsn_haggle))
	{
	    cost -= obj->cost / 2 * roll / 100;
	    act("You haggle with $N.",ch,NULL,keeper,TO_CHAR);
	    check_improve(ch,gsn_haggle,TRUE,4);
	}

	if (number > 1)
	{
	    sprintf(buf,"$n buys $p[%d].",number);
	    act(buf,ch,obj,NULL,TO_ROOM);
	    sprintf(buf,"You buy $p[%d] for %d silver.",number,cost * number);
	    act(buf,ch,obj,NULL,TO_CHAR);
	}
	else
	{
	    act( "$n buys $p.", ch, obj, NULL, TO_ROOM );
	    sprintf(buf,"You buy $p for %d silver.",cost);
	    act( buf, ch, obj, NULL, TO_CHAR );
	}
	deduct_cost(ch,cost * number);
	keeper->gold += cost * number/100;
	keeper->silver += cost * number - (cost * number/100) * 100;

	for (count = 0; count < number; count++)
	{
	    if ( IS_SET( obj->extra_flags, ITEM_INVENTORY ) )
	    	t_obj = create_object( obj->pIndexData, obj->level );
	    else
	    {
		t_obj = obj;
		obj = obj->next_content;
	    	obj_from_char( t_obj );
	    }

	    if (t_obj->timer > 0 && !IS_OBJ_STAT(t_obj,ITEM_HAD_TIMER))
	    	t_obj->timer = 0;
            REM_OBJ_STAT(t_obj,ITEM_HAD_TIMER);
	    obj_to_char( t_obj, ch );
	    if (cost < t_obj->cost)
	    	t_obj->cost = cost;
	}
    }
}
*/


void do_list( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  // Added by SinaC 2000
  if (IS_NPC(ch)) {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  // Modified by SinaC 2001
  if ( IS_SET(ch->in_room->cstat(flags), ROOM_PET_SHOP) ) {
    ROOM_INDEX_DATA *pRoomIndexNext;
    CHAR_DATA *pet;
    bool found;

    /* hack to make new thalos pets work */
    if (ch->in_room->vnum == 9621)
      pRoomIndexNext = get_room_index(9706);
    else
      pRoomIndexNext = get_room_index( ch->in_room->vnum + 1 );

    if ( pRoomIndexNext == NULL ) {
      bug( "Do_list: bad pet shop at vnum %d.", ch->in_room->vnum );
      send_to_char( "You can't do that here.\n\r", ch );
      return;
    }

    found = FALSE;
    for ( pet = pRoomIndexNext->people; pet; pet = pet->next_in_room ) {
      if ( IS_SET(pet->act, ACT_PET) ) {
	if ( !found ) {
	  found = TRUE;
	  send_to_char( "Pets for sale:\n\r", ch );
	}
	sprintf( buf, "[%2d] %8d - %s\n\r",
		 pet->level,
		 10 * pet->level * pet->level,
		 pet->short_descr );
	send_to_char( buf, ch );
      }
    }
    if ( !found )
      send_to_char( "Sorry, we're out of pets right now.\n\r", ch );
    return;
  }
  else
    {
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      int cost,count;
      bool found;
      char arg[MAX_INPUT_LENGTH];

      if ( ( keeper = find_keeper( ch ) ) == NULL )
	return;
      one_argument(argument,arg);

      found = FALSE;
      for ( obj = keeper->carrying; obj; obj = obj->next_content ) {
	if ( obj->wear_loc == WEAR_NONE
	     &&   can_see_obj( ch, obj )
	     &&   ( cost = get_cost( keeper, obj, TRUE ) ) > 0 
	     &&   ( arg[0] == '\0'  
		    ||  is_name(arg,obj->name) )) {
	  if ( !found ) {
	    found = TRUE;
	    send_to_char( "[Lv Price Qty] Item\n\r", ch );
	  }

	  if (IS_OBJ_STAT(obj,ITEM_INVENTORY))
	    sprintf(buf,"[%2d %5d -- ] %s\n\r",
		    obj->level,cost,obj->short_descr);
	  else {
	    count = 1;

	    while (obj->next_content != NULL 
		   && obj->pIndexData == obj->next_content->pIndexData
		   && !str_cmp(obj->short_descr,
			       obj->next_content->short_descr)) {
	      obj = obj->next_content;
	      count++;
	    }
	    sprintf(buf,"[%2d %5d %2d ] %s\n\r",
		    obj->level,cost,count,obj->short_descr);
	  }
	  send_to_char( buf, ch );
	}
      }

      if ( !found )
	send_to_char( "You can't buy anything here.\n\r", ch );
      return;
    }
}



void do_sell( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj;
  int cost,roll;

  one_argument( argument, arg );

  // Added by SinaC 2000
  if (IS_NPC(ch)) {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  if ( arg[0] == '\0' ) {
    send_to_char( "Sell what?\n\r", ch );
    return;
  }

  if ( ( keeper = find_keeper( ch ) ) == NULL )
    return;

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
    act( "$n tells you 'You don't have that item'.",
	 keeper, NULL, ch, TO_VICT );
    ch->reply = keeper;
    return;
  }

  if ( !can_drop_obj( ch, obj ) ) {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  if (!can_see_obj(keeper,obj)) {
    act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
    return;
  }

  if ( IS_SET(obj->extra_flags, ITEM_UNIQUE)){
    act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
    return;
  }

  /* and no donated goods added by Seytan 1997 */
  if( IS_SET(obj->extra_flags,ITEM_DONATED) ) {
    act( "$n doesn't want to buy this DONATED item ($p).", keeper, obj, ch, TO_VICT );
    return;
  }
    
  /* won't buy rotting goods added by Seytan 1997 */
  if ( obj->timer || ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 ) {
    act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
    return;
  }
  /* Removed by SinaC 2000 because of previous test
   *if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 )
   * {
   *	act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
   *	return;
   * }
   */
  if ( cost > (keeper-> silver + 100 * keeper->gold) ) {
    act("$n tells you 'I'm afraid I don't have enough wealth to buy $p.",
	keeper,obj,ch,TO_VICT);
    return;
  }

  act( "$n sells $p.", ch, obj, NULL, TO_ROOM );
  /* haggle */
  roll = number_percent();
  if (!IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT)
      && roll < get_ability(ch,gsn_haggle)
      && (( !IS_NPC(ch) && IS_SET( ch->act, PLR_AUTOHAGGLE) )
	  || IS_NPC(ch))) {
    send_to_char("You haggle with the shopkeeper.\n\r",ch);

    cost += obj->cost / 2 * roll / 100;
    cost = UMIN(cost,95 * get_cost(keeper,obj,TRUE) / 100);
    cost = UMIN(cost,(keeper->silver + 100 * keeper->gold));
    check_improve(ch,gsn_haggle,TRUE,4);
  }
  sprintf( buf, "You sell $p for %d silver and %d gold piece%s.",
	   cost - (cost/100) * 100, cost/100, cost == 1 ? "" : "s" );
  act( buf, ch, obj, NULL, TO_CHAR );
  ch->gold     += cost/100;
  ch->silver 	 += cost - (cost/100) * 100;
  deduct_cost(keeper,cost);
  if ( keeper->gold < 0 )
    keeper->gold = 0;
  if ( keeper->silver< 0)
    keeper->silver = 0;

  if ( obj->item_type == ITEM_TRASH || IS_OBJ_STAT(obj,ITEM_SELL_EXTRACT)) {
    extract_obj( obj );
  }
  else {
    obj_from_char( obj );
    if (obj->timer) {
      SET_OBJ_STAT(obj,ITEM_HAD_TIMER);
    } 
    else
      obj->timer = number_range(50,100);
    obj_to_keeper( obj, keeper );
  }

  return;
}



void do_value( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *keeper;
  OBJ_DATA *obj;
  int cost;

  one_argument( argument, arg );

  // Added by SinaC 2000
  if (IS_NPC(ch))  {
    send_to_char("You can't do that.\n\r", ch);
    return;
  }

  if ( arg[0] == '\0' ) {
    send_to_char( "Value what?\n\r", ch );
    return;
  }

  if ( ( keeper = find_keeper( ch ) ) == NULL )
    return;

  if ( ( obj = get_obj_carry( ch, arg, ch ) ) == NULL ) {
    act( "$n tells you 'You don't have that item'.",
	 keeper, NULL, ch, TO_VICT );
    ch->reply = keeper;
    return;
  }

  if (!can_see_obj(keeper,obj)) {
    act("$n doesn't see what you are offering.",keeper,NULL,ch,TO_VICT);
    return;
  }

  if ( !can_drop_obj( ch, obj ) ) {
    send_to_char( "You can't let go of it.\n\r", ch );
    return;
  }

  if ( ( cost = get_cost( keeper, obj, FALSE ) ) <= 0 ) {
    act( "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT );
    return;
  }

  sprintf( buf, 
	   "$n tells you 'I'll give you %d silver and %d gold coins for $p'.", 
	   cost - (cost/100) * 100, cost/100 );
  act( buf, keeper, obj, ch, TO_VICT );
  ch->reply = keeper;

  return;
}

/* wear object as a secondary weapon */
void do_second(CHAR_DATA *ch, const char *argument) {
  OBJ_DATA *obj, *wield;
  char buf[MAX_STRING_LENGTH]; /* overkill, but what the heck */

  if (argument[0] == '\0') {/* empty */
    send_to_char ("Wear which weapon in your off-hand?\n\r",ch);
    return;
  }

  obj = get_obj_carry (ch, argument, ch); /* find the obj withing ch's inventory */

  if (obj == NULL) {
    send_to_char ("You have no such thing in your backpack.\n\r",ch);
    return;
  }   

  if ( IS_AFFECTED2( ch, AFF2_NOEQUIPMENT ) ) {
    send_to_char("You can't wear any equipment.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if ( obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp(obj->owner,ch->name) ){
    send_to_charf(ch, 
		  "You're not the owner of this object, %s is the owner.\n\r",
		  obj->owner);
    act( "$n tries to use $p, but $e's not the owner.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2003 for object's size
  if (!check_size( ch, obj, FALSE ) )
    return;

  // Added by SinaC 2000 for object restriction
  if (!check_restriction( ch, obj ) ){
    act("You don't meet the requirements to use $p", ch, obj, NULL, TO_CHAR );
    act("$n doesn't meet the requirements to use $p", ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2001 for object condition
  if ( obj->condition == 0 ) {
    act("$p is in a so bad condition, that it is unsable.", ch, obj, NULL, TO_CHAR );
    return;
  }

  if ( obj->item_type != ITEM_WEAPON ) {
    send_to_char( "You can only wear a weapon in your off-hand.\n\r", ch );
    return;
  }

  // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj,ch, "onWearing", args);
  if ( result ) // if onWearing returns 1,  we don't wear the item
    return;

  //if (ch->cstat(size) < SIZE_HUGE
  //    &&  IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) {
  if ( ( ch->cstat(size) < SIZE_LARGE
	 && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) )
       || obj->value[0] == WEAPON_RANGED ) {
    send_to_char("You can't handle that weapon with only your off hand!\n\r",ch);
    return;
  }

  /* check if the char is using a shield or a held weapon */
  if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
       (get_eq_char (ch,WEAR_HOLD)   != NULL) ) {
    send_to_char ("You cannot use a secondary weapon while using a shield or holding an item\n\r",ch);
    return;
  }


  if ( ch->level < obj->level ) {
    sprintf( buf, "You must be level %d to use this object.\n\r",
	     obj->level );
    send_to_char( buf, ch );
    act( "$n tries to use $p, but is too inexperienced.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  /* check that the character is using a first weapon at all */
  if ((wield = get_eq_char (ch, WEAR_WIELD)) == NULL) {/* oops - != here was a bit wrong :) */
    send_to_char ("You need to wield a primary weapon, before using a secondary one!\n\r",ch);
    return;
  }

  //if (ch->cstat(size) < SIZE_LARGE
  //    &&  IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS))  {
  if ( ( ch->cstat(size) < SIZE_LARGE
	 && IS_WEAPON_STAT(wield,WEAPON_TWO_HANDS) )
       || wield->value[0] == WEAPON_RANGED ) {
    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
    return;
  }
  
  /* check for str - secondary weapons have to be lighter */
  if ( get_obj_weight( obj ) > ( str_app[ch->cstat(STR)].wield * 10 / 2) ) {
    send_to_char( "This weapon is too heavy to be used as a secondary weapon by you.\n\r", ch );
    return;
  }

  /* check if the secondary weapon is at least half as light as the primary weapon */
  if ( (get_obj_weight (obj)*2) > get_obj_weight(wield) ) {
    send_to_char ("Your secondary weapon has to be considerably lighter than the primary one.\n\r",ch);
    return;
  }  

  /* check if dual wield is known */
  if ( get_ability(ch,gsn_dual_wield) <= 0 ) {
    send_to_char("You don't even know how to dual wield.\n\r", ch );
    return;
  }

  /* at last - the char uses the weapon */

  if (!remove_obj(ch, WEAR_SECONDARY, TRUE)) /* remove the current weapon if any */
    return;                                /* remove obj tells about any no_remove */

  /* char CAN use the item! that didn't take long at aaall */

  //act ("$n wields $p in $s off-hand.",ch,obj,NULL,TO_ROOM); Msg send in equip_char now, SinaC 2003
  //act ("You wield $p in your off-hand.",ch,obj,NULL,TO_CHAR);
  equip_char ( ch, obj, WEAR_SECONDARY);
  return;
}


// Added by SinaC 2000 for grenade and level,  Grenade and Lever removed by SinaC 2003
void do_pull( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];

  one_argument( argument, arg1 );
  if ( arg1[0] == '\0' ) {
    send_to_char( "Pull what ?\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL ) {
    send_to_char("You don't see that.\n\r", ch );
    return;
  }
  /*
    if ( ( obj = get_obj_wear( ch, arg1, TRUE ) ) == NULL )
    if ( ( obj = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You don't have that!\n\r", ch );
    return;
    }
  */
  // we have found an item with the right name

  /*  Removed by SinaC 2003, can be emulate with script
  // Modified by SinaC 2001
  switch ( obj->item_type ) {
  case ITEM_GRENADE:
    // already pulled ?
    if ( obj->value[1] == GRENADE_PULLED ) {
      send_to_char( "This grenade is already pulled!!!!!\n\r", ch );
      return;
    }
    
    // Okay we pull it and send a warning
    obj->value[1] = GRENADE_PULLED;
    act( "You pull $p and wonder why you did that.", ch, obj, NULL, TO_CHAR );
    act( "$n pulls $p and wait to see what will happen.", ch, obj, NULL, TO_ROOM );
    break;
  case ITEM_LEVER:
    if ( obj->value[0] == LEVER_PULLEDDOWN ) {
      act("You pull up $p.", ch, obj, NULL, TO_CHAR );
      act( "$n pulls up $p.", ch, obj, NULL, TO_ROOM );
      obj->value[0] = LEVER_PULLEDUP;
    }
    else {
      act("You pull down $p.", ch, obj, NULL, TO_CHAR );
      act( "$n pulls down $p.", ch, obj, NULL, TO_ROOM );
      obj->value[0] = LEVER_PULLEDDOWN;
    }
    break;
  }
  */
  //
  // grenade ?
  //if ( obj->item_type != ITEM_GRENADE ) {
  //  send_to_char( "That is not a grenade.\n\r", ch );
  //  return;
  //}
  OBJPROG(obj,ch,"onPull",ch); // just call an obj program
  
  return;
}

void do_push( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  char arg1[MAX_INPUT_LENGTH];

  one_argument( argument, arg1 );
  if ( arg1[0] == '\0' ) {
    send_to_char( "Push what ?\n\r", ch );
    return;
  }

  // Modified by SinaC 2001
  if ( ( obj = get_obj_here( ch, arg1 ) ) == NULL ) {
    send_to_char("You don't see that.\n\r", ch );
    return;
  }

  OBJPROG(obj,ch,"onPull",ch); // FIXME: create an onPush trigger ?
  
  return;
}

// Added by SinaC 2000
bool imprint_spell( int sn, int level, CHAR_DATA* ch, OBJ_DATA* obj )
{
  static const int    sucess_rate[] = { 80, 25, 10 };
  
  char      buf[ MAX_STRING_LENGTH ];
  int       free_slots;
  int       i;
  int       mana;
  int       snlev = 0;

  for ( free_slots = i = 1; i < 4; i++ ) {
    if ( obj->baseval[i] > 0 && obj->value[i] > 0 )
      free_slots++;
  }

  if ( free_slots > 3 ) {
    act( "$p cannot contain any more spells.", ch, obj, NULL, TO_CHAR );
    return FALSE;
  }

  mana = 40;
  //mana += ability_table[sn].min_mana;
  mana += ability_table[sn].min_cost;
  if ( !IS_NPC( ch ) && ch->mana < mana ) {
    send_to_char( "You don't have enough mana.\n\r", ch );
    return FALSE;
  }

  if ( number_percent( ) > get_ability( ch,sn ) ) {
    send_to_char( "You lost your concentration.\n\r", ch );
    ch->mana -= mana / 2;
    return FALSE;
  }

  ch->mana -= mana;
  obj->baseval[free_slots] = obj->value[free_slots] = sn;

  if ( number_percent( ) > sucess_rate[free_slots - 1] ) {
    sprintf( buf, "The magic enchantment has failed: the %s vanishes.\n\r",
	     item_name( obj->item_type ) );
    send_to_char( buf, ch );
    extract_obj( obj );
    return FALSE;
  }

  sprintf( buf, "a %s of ", item_name( obj->item_type ) );
  for ( i = 1; i <= free_slots; i++ )
    if ( obj->baseval[i] != -1 && obj->value[ i ] != -1 ) {
      strcat( buf, ability_table[ obj->value[ i ] ].name );
      ( i != free_slots ) ? strcat( buf, ", " ) : strcat( buf, "" );
    }
  obj->short_descr = str_dup( buf );

  // Modified by SinaC 2001
  //sprintf( buf, "%s %s", obj->name, item_name( obj->item_type ) );
  sprintf( buf, "%s %s", item_name( obj->item_type ), ability_table[sn].name );
  obj->name = str_dup( buf );
  // Modified by SinaC 2001
  snlev = class_abilitylevel( /*ch->cstat(classes)*/ch, sn );
  if(obj->level < snlev) 
    obj->level = snlev;
  sprintf( buf, "You have imbued a new spell to the %s.\n\r",
	   item_name( obj->item_type ) );
  send_to_char( buf, ch );
  
  return TRUE;
}

// Added by SinaC 2001
//void do_trophy( CHAR_DATA *ch, const char *argument ) {
//  char buf[MAX_STRING_LENGTH];
//  OBJ_DATA *obj;
//
//  if ( ( obj = get_obj_here( ch, "head" ) ) == NULL ) {
//    send_to_char( "There is no head here.\n\r", ch );
//    return;
//  }
//  
//  if ( obj->pIndexData->vnum != OBJ_VNUM_SEVERED_HEAD ) {
//    send_to_char( "You can't do that on that head.\n\r", ch );
//    return;
//  }
//
//  // The head of %s
//  // 012345678
//  //          ^--- we keep after that
//  sprintf( buf, "a trophy %s", obj->short_descr+9 );
//  log_stringf(buf);
//  obj->name = str_dup( buf );
//  obj->short_descr = str_dup( buf );
//  strcat( buf, "." );
//  buf[0] = UPPER( buf[0] );
//  obj->description = str_dup( buf );
//  obj->timer = 0;
//
//  WAIT_STATE( ch, 21 );
//
//  send_to_charf( ch, "You have fashioned %s\n\r", buf );
//
//  return;
//}


void do_fourth_wield(CHAR_DATA *ch, const char *argument) {
  OBJ_DATA *obj, *wield3;
  char buf[MAX_STRING_LENGTH]; // overkill, but what the heck

  if (argument[0] == '\0') {/* empty */
    send_to_char ("Wear which weapon in your fourth hand?\n\r",ch);
    return;
  }

  obj = get_obj_carry (ch, argument, ch); // find the obj withing ch's inventory

  if (obj == NULL) {
    send_to_char ("You have no such thing in your backpack.\n\r",ch);
    return;
  }   

  if ( !IS_SET(ch->cstat(form),FORM_FOUR_ARMS) ) {
    send_to_char("You don't have four arms.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2( ch, AFF2_NOEQUIPMENT ) ) {
    send_to_char("You can't wear any equipment.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if ( obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp(obj->owner,ch->name) ){
    send_to_charf(ch, 
		  "You're not the owner of this object, %s is the owner.\n\r",
		  obj->owner);
    act( "$n tries to use $p, but $e's not the owner.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2003 for object's size
  if (!check_size( ch, obj, FALSE ) )
    return;

  // Added by SinaC 2000 for object restriction
  if (!check_restriction( ch, obj ) ) {
    act("You don't meet the requirements to use $p", ch, obj, NULL, TO_CHAR );
    act("$n doesn't meet the requirements to use $p", ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2001 for object condition
  if ( obj->condition == 0 ) {
    act("$p is in a so bad condition, that it is unsable.", ch, obj, NULL, TO_CHAR );
    return;
  }

  if ( obj->item_type != ITEM_WEAPON ) {
    send_to_char( "You can only wear a weapon in fourth hand.\n\r", ch );
    return;
  }

  // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj,ch, "onWearing", args);
  if ( result ) // if onWearing returns 1,  we don't wear the item
    return;
        
  //  if ( ch->cstat(size) < SIZE_HUGE
  //      && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) ) {
  if ( ( ch->cstat(size) < SIZE_LARGE
	 && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) )
       || obj->value[0] == WEAPON_RANGED ) {
    send_to_char("You can't handle that weapon with only your fourth hand!\n\r",ch);
    return;
  }

  // No need to test this, the first pair of hands wear shield or hold item
  // check if the char is using a shield or a held weapon
  //  if ( (get_eq_char (ch,WEAR_SHIELD) != NULL) ||
  //       (get_eq_char (ch,WEAR_HOLD)   != NULL) ) {
  //    send_to_char ("You cannot use a fourth weapon while using a shield or holding an item\n\r",ch);
  //    return;
  //  }

  if ( ch->level < obj->level ) {
    sprintf( buf, "You must be level %d to use this object.\n\r",
	     obj->level );
    send_to_char( buf, ch );
    act( "$n tries to use $p, but is too inexperienced.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  // check that the character is using a third weapon at all
  if ((wield3 = get_eq_char (ch, WEAR_THIRDLY)) == NULL) {
    send_to_char ("You need to wield a thirdly weapon, before using a fourthly one!\n\r",ch);
    return;
  }

  //  if (ch->cstat(size) < SIZE_LARGE
  //      &&  IS_WEAPON_STAT(wield3,WEAPON_TWO_HANDS))  {
  if ( ( ch->cstat(size) < SIZE_LARGE
	 && IS_WEAPON_STAT(wield3,WEAPON_TWO_HANDS) )
       || wield3->value[0] == WEAPON_RANGED ) {
    send_to_char("Your hands are tied up with your weapon!\n\r",ch);
    return;
  }
  
  // check for str - fourthly weapons have to be lighter
  int weight = get_obj_weight( obj );
  if ( weight > ( str_app[ch->cstat(STR)].wield * 10 / 2) ) {
    send_to_char( "This weapon is too heavy to be used as a fourthly weapon by you.\n\r", ch );
    return;
  }

  // check if the fourthly weapon is at least half as light as the thirdly weapon
  if ( weight > get_obj_weight(wield3) ) {
    send_to_char("Your fourthly weapon has to be lighter than the thirdly one.\n\r",ch);
    return;
  }

  // check if fourth wield is known
  if ( get_ability(ch,gsn_fourth_wield) <= 0 ) {
    send_to_char("You don't even know how to fourth wield.\n\r", ch );
    return;
  }

  // at last - the char uses the weapon
  if (!remove_obj(ch, WEAR_FOURTHLY, TRUE)) // remove the current weapon if any
    return;                                // remove obj tells about any no_remove

  // char CAN use the item! that didn't take long at aaall */
  //  act ("$n wields $p in $s fourth hand.",ch,obj,NULL,TO_ROOM); Msg send in equip_char now, SinaC 2003
  //  act ("You wield $p in your fourth hand.",ch,obj,NULL,TO_CHAR);
  equip_char( ch, obj, WEAR_FOURTHLY);
  return;
}

void do_third_wield( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  OBJ_DATA *obj;

  if (argument[0] == '\0') {// empty
    send_to_char ("Wield which weapon in your third hand?\n\r",ch);
    return;
  }

  obj = get_obj_carry (ch, argument, ch); // find the obj withing ch's inventory

  if (obj == NULL) {
    send_to_char ("You have no such thing in your backpack.\n\r",ch);
    return;
  }   

  if ( !IS_SET(ch->cstat(form),FORM_FOUR_ARMS) ) {
    send_to_char("You don't have four arms.\n\r",ch);
    return;
  }

  if ( IS_AFFECTED2( ch, AFF2_NOEQUIPMENT ) ) {
    send_to_char("You can't wear any equipment.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  if ( obj->owner != NULL 
       && obj->owner[0] != '\0'
       && str_cmp(obj->owner,ch->name) ){
    send_to_charf(ch, 
		  "You're not the owner of this object, %s is the owner.\n\r",
		  obj->owner);
    act( "$n tries to use $p, but $e's not the owner.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2003 for object's size
  if (!check_size( ch, obj, FALSE ) )
    return;

  // Added by SinaC 2000 for object restriction
  if (!check_restriction( ch, obj ) ) {
    act("You don't meet the requirements to use $p", ch, obj, NULL, TO_CHAR );
    act("$n doesn't meet the requirements to use $p", ch, obj, NULL, TO_ROOM );
    return;
  }

  // Added by SinaC 2001 for object condition
  if ( obj->condition == 0 ) {
    act("$p is in a so bad condition, that it is unsable.", ch, obj, NULL, TO_CHAR );
    return;
  }

  if ( obj->item_type != ITEM_WEAPON ) {
    send_to_char( "You can only wield a weapon in third hand.\n\r", ch );
    return;
  }

  // Added by SinaC 2003
  Value args[] = {ch};
  bool result = objprog( obj,ch, "onWearing", args);
  if ( result ) // if onWearing returns 1,  we don't wear the item
    return;

  if ( ch->level < obj->level ) {
    sprintf( buf, "You must be level %d to use this object.\n\r",
	     obj->level );
    send_to_char( buf, ch );
    act( "$n tries to use $p, but is too inexperienced.",
	 ch, obj, NULL, TO_ROOM );
    return;
  }

  OBJ_DATA *wield4 = get_eq_char(ch,WEAR_FOURTHLY);
  // Check STR, Weight and 2-handed only on PCs
  if ( !IS_NPC(ch) ) {
    // check for str
    int weight = get_obj_weight( obj );
    if ( weight > str_app[ch->cstat(STR)].wield * 10 ) {
      send_to_char( "This weapon is too heavy to be used as a thirdly weapon by you.\n\r", ch );
      return;
    }
    // check for 2-handed weapon
    //    if ( ch->cstat(size) < SIZE_LARGE
    //	 && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)
    //	 && wield4 != NULL ) {
    if ( wield4 != NULL
	 && ( ( ch->cstat(size) < SIZE_LARGE
	      && IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS) )
	      || obj->value[0] == WEAPON_RANGED ) ) {
      send_to_char("You need two hands free for that weapon.\n\r",ch);
      return;
    }
  }

  // check if the fourthly weapon is at least 2/3 as light as the primary weapon
  if ( wield4 != NULL
       && get_obj_weight(wield4)*2 > get_obj_weight(obj) ) {
    send_to_char ("Your fourthly weapon has to be considerably lighter than the thirdly one.\n\r",ch);
    return;
  }

  // check if fourth wield is known
  if ( get_ability(ch,gsn_third_wield) <= 0 ) {
    send_to_char("You don't even know how to third wield.\n\r", ch );
    return;
  }
  
  if ( !remove_obj( ch, WEAR_THIRDLY, TRUE ) )
    return;
    
  //  act ("$n wields $p in $s third hand.",ch,obj,NULL,TO_ROOM); Msg send in equip_char now, SinaC 2003
  //  act ("You wield $p in your third hand.",ch,obj,NULL,TO_CHAR);
  equip_char( ch, obj, WEAR_THIRDLY);
  return;
}
