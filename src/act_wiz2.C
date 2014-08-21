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
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "classes.h"
#include "olc.h" /* Oxtal */
#include "interp.h"

// Added by SinaC 2000/2001
#include "abilityupgrade.h"
#include "act_wiz.h"
#include "act_wiz2.h"
#include "comm.h"
#include "handler.h"
#include "act_comm.h"
#include "db.h"
#include "affects.h"
#include "special.h"
#include "act_info.h"
#include "restriction.h"
#include "olc_value.h"
#include "names.h"
#include "const.h"
#include "scrhash.h"
#include "bit.h"
#include "act_wiz3.h"
#include "faction.h"
#include "utils.h"
#include "statistics.h"


/*
 * Local functions.
 */
// SinaC 2000
//  for omatch
bool                    is_item_buggy     args( ( OBJ_INDEX_DATA *obj ) );

/* By Oxtal */
void do_setpet(CHAR_DATA * ch, const char * argument) {
  CHAR_DATA * pet;
  if (ch->pet){
    send_to_char("You already have a pet.\n\r",ch);
    return;
  }    
  if (!argument[0]){
    send_to_char("Who do you want as a pet?\n\r",ch);
    return;
  }
  if (!(pet=get_char_room(ch,argument))) {
    send_to_char("Can't find it. Sorry.\n\r",ch);
    return;
  }

  if (IS_PC(pet)) {
    send_to_char("Players are NOT your pets! Sorry.\n\r",ch);
    return;
  }

  /* Code stolen from do_buy */
  SET_BIT(pet->act, ACT_PET);
  SET_BIT(pet->bstat(affected_by), AFF_CHARM);
  recompute(pet);
   
  pet->comm = COMM_NOTELL|COMM_NOSHOUT|COMM_NOCHANNELS;

  add_follower( pet, ch );
  pet->leader = ch;
  ch->pet = pet;

  send_to_char( "Enjoy your pet.\n\r", ch );
  act( "$n takes $N as a pet.", ch, NULL, pet, TO_ROOM );
}


/* RT to replace the 3 stat commands */

void do_stat ( CHAR_DATA *ch, const char *argument )
{
  char arg[MAX_INPUT_LENGTH];
  const char *string;
  OBJ_DATA *obj;
  ROOM_INDEX_DATA *location;
  CHAR_DATA *victim;

  string = one_argument(argument, arg);
  if ( arg[0] == '\0') {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  stat <name>\n\r",ch);
    send_to_char("  stat obj <name>\n\r",ch);
    send_to_char("  stat mob <name>\n\r",ch);
    send_to_char("  stat room <number>\n\r",ch);
    return;
  }

  if (!str_cmp(arg,"room")) {
    do_rstat(ch,string);
    return;
  }
  
  if (!str_cmp(arg,"obj")) {
    do_ostat(ch,string);
    return;
  }

  if(!str_cmp(arg,"char")  || !str_cmp(arg,"mob")) {
    do_mstat(ch,string);
    return;
  }
   
  /* do it the old way */

  obj = get_obj_world(ch,argument);
  if (obj != NULL) {
    do_ostat(ch,argument);
    return;
  }

  victim = get_char_world(ch,argument);
  if (victim != NULL) {
    do_mstat(ch,argument);
    return;
  }

  location = find_location(ch,argument);
  if (location != NULL) {
    do_rstat(ch,argument);
    return;
  }

  send_to_char("Nothing by that name found anywhere.\n\r",ch);
}

   



void do_rstat( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  OBJ_DATA *obj;
  CHAR_DATA *rch;
  int door;

  one_argument( argument, arg );
  location = ( arg[0] == '\0' ) ? ch->in_room : find_location( ch, arg );
  if ( location == NULL ) {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,location) && ch->in_room != location 
      &&  room_is_private( location ) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
    send_to_char( "That room is private right now.\n\r", ch );
    return;
  }

  sprintf( buf, "Name: '%s'\n\rArea: '%s'\n\r",
	   location->name,
	   location->area->name );
  send_to_char( buf, ch );

  // Modified by SinaC 2001 for mental user
  sprintf( buf,
	   "Vnum: %d  Sector: %s  Light: %d  Healing: %d  Mana: %d  Psp: %d\n\r",
	   location->vnum,
	   // Modified by SinaC 2001
	   flag_string( sector_flags, location->cstat(sector) ),
	   location->cstat(light),
	   location->cstat(healrate),
	   location->cstat(manarate), 
	   location->cstat(psprate) );
  /*
  sprintf( buf,
	   "Vnum: %d  Sector: %s  Light: %d  Healing: %d  Mana: %d\n\r",
	   //	"Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n\r",
	   location->vnum,
	   //location->sector_type
	   flag_string( sector_flags, location->sector_type ),
	   location->light,
	   location->heal_rate,
	   location->mana_rate );
  */
  send_to_char( buf, ch );

  // Added by SinaC 2001 for max size
  send_to_charf(ch,
		"Max size: %s\n\r",
		flag_string( size_flags, location->cstat(maxsize)));

  sprintf(buf, "Guild : [%s]\n\r", flag_string(classes_flags,location->guild));
  send_to_char( buf, ch );

  // Modified by SinaC 2000
  sprintf( buf,
	   /*"Room flags: %d.\n\rDescription:\n\r%s",
	     location->room_flags*/
	   "Room flags: %s [%d].\n\rDescription:\n\r%s",
	   // Modified by SinaC 2001
	   flag_string( room_flags, location->cstat(flags) ), location->cstat(flags),
	   location->description );
  send_to_char( buf, ch );

  if ( location->extra_descr != NULL ) {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "Extra description keywords: ", ch );
    /* Modified by SinaC 2001
    for ( ed = location->extra_descr; ed; ed = ed->next ) {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
	send_to_char( " ", ch );
    }
    send_to_char( "'.\n\r", ch );
    */
    for ( ed = location->extra_descr; ed; ed = ed->next ) {
      send_to_char( "[", ch );
      send_to_char( ed->keyword, ch );
      send_to_char( "]", ch );
    }

    send_to_char( "\n\r", ch );
  }

  send_to_char( "Characters:", ch );
  for ( rch = location->people; rch; rch = rch->next_in_room ) {
    if (can_see(ch,rch)) {
      send_to_char( " ", ch );
      one_argument( rch->name, buf );
      send_to_char( buf, ch );
    }
  }

  send_to_char( ".\n\rObjects:   ", ch );
  for ( obj = location->contents; obj; obj = obj->next_content ) {
    send_to_char( " ", ch );
    one_argument( obj->name, buf );
    send_to_char( buf, ch );
  }
  send_to_char( ".\n\r", ch );

  // Added by SinaC 2003 for room program
  if ( location->program != NULL )
    send_to_charf(ch,"Program: %s\n\r", location->program->name );
  if ( location->clazz != NULL )
    send_to_charf(ch,"Clazz: %s\n\r", location->clazz->name );

  // Added by SinaC 2003 for repop time
  send_to_charf(ch,"Time between repop: %4d  Time between repop with people: %4d\n\r"
		"Current repop time: %4d\n\r",
		location->time_between_repop, location->time_between_repop_people,
		location->current_time_repop );

  for ( door = 0; door < MAX_DIR; door++ ) { // Modified by SinaC 2003
    EXIT_DATA *pexit;

    if ( ( pexit = location->exit[door] ) != NULL ) {
      sprintf( buf,
	       "Door: %s.  To: %d.  Key: %d.  Exit flags: %d. [%s]\n\rKeyword: '%s'.  Description: %s",
	       //"Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\n\rKeyword: '%s'.  Description: %s",

	       /*door*/capitalize(dir_name[door]),
	       (pexit->u1.to_room == NULL ? -1 : pexit->u1.to_room->vnum),
	       pexit->key,
	       pexit->exit_info,
	       flag_string( exit_flags, pexit->exit_info ), // SinaC 2003
	       pexit->keyword,
	       pexit->description[0] != '\0'
	       ? pexit->description : "(none).\n\r" );
      send_to_char( buf, ch );
    }
  }

  bool found = FALSE;
  // Added by SinaC 2001 for room affects
  for ( AFFECT_DATA *paf = location->base_affected; paf != NULL; paf = paf->next ) {
    // Modified by SinaC 2001
    //afstring( buf, paf, ch, TRUE );
    // new affect system
    afstring( buf, paf, ch, FALSE );
    send_to_char(buf,ch);
    found = TRUE;
  }
  if ( found )
    send_to_char("\n\r", ch );
  found = FALSE;
  for ( AFFECT_DATA *paf = location->current_affected; paf != NULL; paf = paf->next ) {
    // Modified by SinaC 2001
    //afstring( buf, paf, ch, TRUE );
    // new affect system
    afstring( buf, paf, ch, FALSE );
    send_to_char(buf,ch);
    found = TRUE;
  }
  if ( found )
    send_to_char("\n\r", ch );

  if ( location->ex_fields ) { // SinaC 2003
    send_to_charf(ch,"Extra fields:\n\r");
    dump_extra_fields( ch, location );
  }

  return;
}



void do_ostat( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_DATA *obj;
  // Added by SinaC 2000 for object restrictions
  RESTR_DATA *restr;
  bool found;
  // Added by SinaC 2000 for skill/spell upgrade
  ABILITY_UPGRADE *upgr;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Stat what?\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_world( ch, argument ) ) == NULL ) {
    send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
    return;
  }

  sprintf( buf, "Name(s): %s\n\r",
	   obj->name );
  send_to_char( buf, ch );

  sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d  Enchanted: %s\n\r",
	   obj->pIndexData->vnum, obj->pIndexData->new_format ? "new" : "old",
	   item_name(obj->item_type), obj->pIndexData->reset_num,
	   obj->enchanted?"Yes":"No" );
  send_to_char( buf, ch );

  sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	   obj->short_descr, obj->description );
  send_to_char( buf, ch );

  sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	   wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n\r",
	   1,           get_obj_number( obj ),
	   obj->weight, get_obj_weight( obj ),get_true_weight(obj) );
  send_to_char( buf, ch );

  sprintf( buf, "Level: %d  Cost: %d  Condition: %d  Timer: %d\n\r",
	   obj->level, obj->cost, obj->condition, obj->timer );
  send_to_char( buf, ch );

  // Added by SinaC 2003, size if not anymore a size but an obj stat
  sprintf( buf, "Size: %s\n\r",
	   flag_string(size_flags, obj->size) );
  send_to_char( buf, ch );

  // Added by SinaC 2000, Modified by SinaC 2001
  sprintf( buf, "Owner: %s  Material: %s\n\r", 
	   obj->owner==NULL ? "None" : obj->owner,
	   material_table[obj->material].name );
  send_to_char( buf, ch );

  sprintf( buf,
	   "In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n\r",
	   obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
	   obj->in_obj     == NULL    ? "(none)" : obj->in_obj->short_descr,
	   obj->carried_by == NULL    ? "(none)" : 
	   can_see(ch,obj->carried_by) ? obj->carried_by->name
	   : "someone",
	   obj->wear_loc );
  send_to_char( buf, ch );
    
  // Added by SinaC 2001
  sprintf( buf, "Base values: %d %d %d %d %d\n\r",
	   obj->baseval[0], obj->baseval[1], obj->baseval[2], obj->baseval[3],
	   obj->baseval[4] );
  send_to_char( buf, ch );

  sprintf( buf, "Values: %d %d %d %d %d\n\r",
	   obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	   obj->value[4] );
  send_to_char( buf, ch );

  if ( obj->pIndexData->program != NULL )
    send_to_charf(ch,"Program: %s\n\r", obj->pIndexData->program->name );
  if (  obj->clazz != NULL )
    send_to_charf(ch,"Clazz: %s\n\r", obj->clazz->name );
    
  /* now give out vital statistics as per identify */
    
  switch ( obj->item_type ) {
    //  case ITEM_SKELETON: // SinaC 2003
  case ITEM_CORPSE_NPC:
  case ITEM_CORPSE_PC:
    sprintf( buf2, flag_string( form_flags, obj->value[1]) );
    sprintf( buf3, flag_string( part_flags, obj->value[2]) );
    sprintf( buf4, flag_string( part_flags, obj->value[4]) );
    sprintf( buf,
	     "[v0] Vnum:           [%d]\n\r"
	     "[v1] Form:           %s\n\r"
	     "[v2] Parts:          %s\n\r"
	     "[v3] Skeleton:       %s\n\r"
	     "[v4] Missing parts:  %s\n\r",
	     obj->value[0], buf2, buf3, IS_SET( obj->value[3], CORPSE_SKELETON )?"Yes":"No", buf4 );
    send_to_charf( ch, buf );
    break;
  case ITEM_WINDOW:
    sprintf( buf, "To room: %d\n\r",obj->value[0]);
    send_to_char( buf, ch );
    break;
  case ITEM_PORTAL:
    sprintf( buf2, flag_string( exit_flags, obj->value[1]));
    sprintf( buf3, flag_string( portal_flags , obj->value[2]));
    sprintf( buf,
	     "[v0] Charges:        [%d]\n\r"
	     "[v1] Exit Flags:     %s\n\r"
	     "[v2] Portal Flags:   %s\n\r"
	     "[v3] Goes to (vnum): [%d]\n\r",
	     obj->value[0],
	     buf2,
	     buf3,
	     obj->value[3] );
    send_to_char( buf, ch );
    break;
    /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
  case ITEM_GRENADE:
    sprintf( buf, "Timer: %d  Damage: %d  %s\n\r",
	     obj->value[0],
	     obj->value[2],
	     obj->value[1]==GRENADE_PULLED? 
	     "Pulled":obj->value[1]==GRENADE_NOTPULLED?"Not pulled":"bad value");
    send_to_char( buf, ch );
    break;
    */
    /* Removed by SinaC 2003
    // Added by SinaC 2000 for throwing items
  case ITEM_THROWING:
    sprintf( buf, "Damage is %dd%d (average %d)\n\rDamage type: %s.\n\r", 
	     obj->value[0],
	     obj->value[1],
	     (1 + obj->value[1]) * obj->value[0] / 2,
	     (obj->value[2] > 0 && obj->value[2] < MAX_DAMAGE_MESSAGE) ?
	     attack_table[obj->value[2]].noun : "undefined");
    if ( obj->value[4]>=0 && obj->value[4]<MAX_ABILITY )
      sprintf( buf, "%sSpell added: %s (level %d)\n\r",
	       buf,
	       ability_table[obj->value[4]].name,
	       obj->value[3]);
    else
      sprintf( buf, "%sNo spell added.\n\r",buf );
    send_to_char(buf, ch );
    break;
    */
  case ITEM_SCROLL: 
  case ITEM_POTION:
  case ITEM_PILL:
    // Added by SinaC 2003
  case ITEM_TEMPLATE:
    sprintf( buf, "Level %d spells of:", obj->value[0] );
    send_to_char( buf, ch );

    if ( obj->value[1] >= 0 && obj->value[1] < MAX_ABILITY ){
      send_to_char( " '", ch );
      send_to_char( ability_table[obj->value[1]].name, ch );
      send_to_char( "'", ch );
    }

    if ( obj->value[2] >= 0 && obj->value[2] < MAX_ABILITY ){
      send_to_char( " '", ch );
      send_to_char( ability_table[obj->value[2]].name, ch );
      send_to_char( "'", ch );
    }

    if ( obj->value[3] >= 0 && obj->value[3] < MAX_ABILITY ){
      send_to_char( " '", ch );
      send_to_char( ability_table[obj->value[3]].name, ch );
      send_to_char( "'", ch );
    }

    if (obj->value[4] >= 0 && obj->value[4] < MAX_ABILITY){
      send_to_char(" '",ch);
      send_to_char(ability_table[obj->value[4]].name,ch);
      send_to_char("'",ch);
    }

    send_to_char( ".\n\r", ch );
    break;

  case ITEM_WAND: 
  case ITEM_STAFF: 
    sprintf( buf, "Has %d(%d) charges of level %d",
	     obj->value[1], obj->value[2], obj->value[0] );
    send_to_char( buf, ch );
      
    if ( obj->value[3] >= 0 && obj->value[3] < MAX_ABILITY ){
      send_to_char( " '", ch );
      send_to_char( ability_table[obj->value[3]].name, ch );
      send_to_char( "'", ch );
    }

    send_to_char( ".\n\r", ch );
    break;

  case ITEM_DRINK_CON:
    sprintf(buf,"It holds %s-colored %s.\n\r",
	    liq_table[obj->value[2]].liq_color,
	    liq_table[obj->value[2]].liq_name);
    send_to_char(buf,ch);
    break;
		
      
  case ITEM_WEAPON: {
    send_to_charf(ch,"Weapon type is %s.\n\r",flag_string(weapon_class,obj->value[0]));
    int v1 = GET_WEAPON_DNUMBER(obj);
    int v2 = GET_WEAPON_DTYPE(obj);
    if (obj->pIndexData->new_format)
      sprintf(buf,"Damage is %dd%d (average %d)\n\r",
	      v1,v2,
	      ((1 + v2) * v1) / 2);
    else
      sprintf( buf, "Damage is %d to %d (average %d)\n\r",
	       v1, v2,
	       ( v1 + v2 ) / 2 );
    send_to_char( buf, ch );

    int v3 = GET_WEAPON_DAMTYPE(obj);
    sprintf(buf,"Damage noun is %s.\n\r",
	    (v3 > 0 && v3 < MAX_DAMAGE_MESSAGE) ?
	    attack_table[v3].noun : "undefined");
    send_to_char(buf,ch);
    // SinaC 2003
    int v4 = GET_WEAPON_FLAGS(obj);
    if ( v4 ){  /* weapon flags */
      sprintf(buf,"Weapons flags: %s\n\r",
	      weapon_bit_name(v4));
      send_to_char(buf,ch);
    }

    if ( obj->value[0] == WEAPON_RANGED ) {
      send_to_charf(ch, "String condition: %d.\n\r", obj->value[1] );
      send_to_charf(ch, "String condition modifier probability: %d.\n\r", obj->value[2] );
      send_to_charf(ch, "Strength: %d.\n\r", obj->value[3] );
      send_to_charf(ch, "Max distance: %d.\n\r", obj->value[4] );
    }
    break;
  }
  case ITEM_ARMOR:
    sprintf( buf, 
	     "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n\r",
	     obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
    send_to_char( buf, ch );
    break;

  case ITEM_CONTAINER:
    sprintf(buf,"Capacity: %d#  Maximum weight: %d#  flags: %s\n\r",
	    obj->value[0], obj->value[3], cont_bit_name(obj->value[1]));
    send_to_char(buf,ch);
    if (obj->value[4] != 100) {
      sprintf(buf,"Weight multiplier: %d%%\n\r",
	      obj->value[4]);
      send_to_char(buf,ch);
    }
    break;
  }


  if ( obj->extra_descr != NULL || obj->pIndexData->extra_descr != NULL ) {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "Extra description keywords: ", ch );

    /* Modified by SinaC 2001
       for ( ed = obj->extra_descr; ed != NULL; ed = ed->next ) {
       send_to_char( ed->keyword, ch );
       if ( ed->next != NULL )
       send_to_char( " ", ch );
    */
    for ( ed = obj->extra_descr; ed; ed = ed->next ) {
      send_to_char( "[", ch );
      send_to_char( ed->keyword, ch );
      send_to_char( "]", ch );
    }
    
    for ( ed = obj->pIndexData->extra_descr; ed; ed = ed->next ) {
      send_to_char( "[", ch );
      send_to_char( ed->keyword, ch );
      send_to_char( "]", ch );
    }

    send_to_char( "\n\r", ch );
    /* Modified by SinaC 2001
    for ( ed = obj->pIndexData->extra_descr; ed != NULL; ed = ed->next ) {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
	send_to_char( " ", ch );
    }
    send_to_char( "'\n\r", ch );
    */
  }

  // Added by SinaC 2000 for object restrictions
  found = FALSE;
  //  for ( restr = obj->restriction; restr != NULL; restr = restr->next ){
  //    found = TRUE;
  //    restrstring( buf, restr );
  //  send_to_char( buf, ch );
  //  }
  // Added by SinaC 2001
  //  if (!obj->enchanted)
    for ( restr = obj->pIndexData->restriction; restr != NULL; restr = restr->next ){
      found = TRUE;
      restrstring( buf, restr );
      send_to_char( buf, ch );
    }
  
  if ( found )
    send_to_char("\n\r",ch );

  // Added by SinaC 2000 for skill/spell upgrade
  found = FALSE;
  //  for ( upgr = obj->upgrade; upgr != NULL; upgr = upgr->next ){
  //    found = TRUE;
  //    abilityupgradestring( buf, upgr );
  //    send_to_char( buf, ch );
  //  }
  // Added by SinaC 2003
  //  if (!obj->enchanted)
    for ( upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next ){
      found = TRUE;
      abilityupgradestring( buf, upgr );
      send_to_char( buf, ch );
    }

  if ( found )
    send_to_char("\n\r",ch );

  for ( paf = obj->affected; paf != NULL; paf = paf->next ) {
    // Modified by SinaC 2001
    //afstring( buf, paf, ch, TRUE );
    // SinaC 2003: new affect system
    afstring( buf, paf, ch, FALSE );
    send_to_char(buf,ch);
  }

  if (!obj->enchanted)
    for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next ) {
      // Modified by SinaC 2001
      //afstring(buf,paf,ch, TRUE);
      // SinaC 2003: new affect system
      afstring( buf, paf, ch, FALSE );
      send_to_char( buf, ch );

    }

  if ( obj->ex_fields ) { // SinaC 2003
    send_to_charf(ch,"Extra fields:\n\r");
    dump_extra_fields( ch, obj );
  }

  return;
}

void do_mstat( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  CHAR_DATA *victim;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Stat whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  sprintf( buf, "Name: %s     Id: %ld\n\r",
	   victim->name, victim->id );
  send_to_char( buf, ch );

  sprintf( buf, 
	   "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s  Room: %d\n\r",
	   IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	   IS_NPC(victim) ? victim->pIndexData->new_format ? "new" : "old" : "pc",
	   race_table[victim->cstat(race)].name,
	   IS_NPC(victim) ? victim->group : 0, sex_table[victim->cstat(sex)].name,
	   victim->in_room == NULL    ?        0 : victim->in_room->vnum
	   );
  send_to_char( buf, ch );

  // Added by SinaC 2000
  if ( IS_NPC(victim) ) {
    send_to_charf(ch,
		  "Zone: %s       Area: %s.\n\r",
		  victim->zone==NULL?"(none)":victim->zone->name,
		  victim->in_room==NULL?"(none)":victim->in_room->area->name );
  }
  
  if (IS_NPC(victim)) {
    sprintf(buf,"Count: %d  Killed: %d\n\r",
	    victim->pIndexData->count,victim->pIndexData->killed);
    send_to_char(buf,ch);
  }

  // Added by SinaC 2003 for factions
  if (IS_NPC(victim))
    send_to_charf(ch,"Faction: %s\n\r", faction_table[victim->faction].name );

  sprintf( buf, 
	   "Str: %ld(%ld)  Int: %ld(%ld)  Wis: %ld(%ld)  Dex: %ld(%ld)  Con: %ld(%ld)\n\r",

	   victim->bstat(STR), victim->cstat(STR),
	   victim->bstat(INT), victim->cstat(INT),
	   victim->bstat(WIS), victim->cstat(WIS),
	   victim->bstat(DEX), victim->cstat(DEX),
	   victim->bstat(CON), victim->cstat(CON)
	   );
  send_to_char( buf, ch );

  // Modified by SinaC 2001 for mental user
  /*
  sprintf( buf, "Hp: %d/%ld  Mana: %d/%ld  Move: %d/%ld  Practices: %d\n\r",
	   victim->hit,         victim->cstat(max_hit),
	   victim->mana,        victim->cstat(max_mana),
	   victim->move,        victim->cstat(max_move),
	   IS_NPC(victim) ? 0 : victim->practice );
  */
  // Modified by SinaC 2001
  sprintf( buf, "Hp: %d/%ld  Mana: %d/%ld  Psp: %d/%ld  Move: %d/%ld\n\r",
	   victim->hit,         victim->cstat(max_hit),
	   victim->mana,        victim->cstat(max_mana),
	   victim->psp,         victim->cstat(max_psp),
	   victim->move,        victim->cstat(max_move) );
  send_to_char( buf, ch );

  send_to_charf(ch,
		"Trains: %d   Practices: %d\n\r",
		IS_NPC(victim) ? 0 : victim->train,
		IS_NPC(victim) ? 0 : victim->practice );
	
  sprintf( buf,
	   "Lv: %d Class: %s Etho: %s Align: %ld Gold: %ld Silver: %ld Exp: %d\n\r",
	   victim->level,
	   // Modified by SinaC 2000
	   /*IS_NPC(victim)*/victim->cstat(classes)==0 ? "mobile" : class_name(victim->cstat(classes)),
	   // Modified by SinaC 2001 etho/alignment are attributes now
	   // Modified by SinaC 2000
	   //etho_name(victim->align.etho),
	   etho_name(victim->cstat(etho)),
	   //victim->align.alignment,
	   victim->cstat(alignment),
	   victim->gold, victim->silver, victim->exp );
  send_to_char( buf, ch );

  if ( victim->isWildMagic ) // SinaC 2003
    send_to_charf(ch,"Wild Magic.\n\r");
  
  if ( class_ismulti( victim->cstat(classes) ) ){
    sprintf(buf, "Classes: ");
    long clbit = 1;
    for (int i = 0; i<MAX_CLASS;i++) {
      if (clbit & victim->cstat(classes)) {
	char buf1[MAX_INPUT_LENGTH];
	sprintf(buf1,"   %s",class_table[i].name);
	strcat(buf,buf1);
      }
      clbit<<=1;
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
  }
  
  if ( !IS_NPC(victim) ) { // SinaC 2003: casting rules
    casting_rule_type sk_rule = classes_casting_rule( victim, -TYPE_SKILL );
    casting_rule_type sp_rule = classes_casting_rule( victim, -TYPE_SPELL );
    casting_rule_type po_rule = classes_casting_rule( victim, -TYPE_POWER );
    casting_rule_type so_rule = classes_casting_rule( victim, -TYPE_SONG );
    send_to_charf(ch,
		  "Casting rule: Skill = (%2d,%2d)\n\r"
		  "              Spell = (%2d,%2d)\n\r"
		  "              Power = (%2d,%2d)\n\r"
		  "              Song  = (%2d,%2d)\n\r",
		  sk_rule.highest, sk_rule.other,
		  sp_rule.highest, sp_rule.other,
		  po_rule.highest, po_rule.other,
		  so_rule.highest, so_rule.other );
  }

  send_to_charf(ch,"Optimizing bit: %s\n\r", print_flags( victim->optimizing_bit ) ); // SinaC 2003

  sprintf(buf,"Armor: pierce: %ld  bash: %ld  slash: %ld  magic: %ld  saves: %ld\n\r",
	  GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
	  GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC),
	  victim->cstat(saving_throw));
  send_to_char(buf,ch);

  long hitr = GET_HITROLL(victim), 
    damr = GET_DAMROLL(victim);
  sprintf( buf,
	   "Hit: %ld  Dam: %ld  Size: %s  Position: %s  Wimpy: %d\n\r",
	   hitr, damr,
	   size_table[victim->cstat(size)].name, position_table[victim->position].name,
	   victim->wimpy );
  send_to_char( buf, ch );

  if (IS_NPC(victim) && victim->pIndexData->new_format) {
    sprintf(buf, "Damage: %ldd%ld  Message:  %s\n\r",
	    victim->cstat(DICE_NUMBER),victim->cstat(DICE_TYPE),
	    attack_table[victim->cstat(dam_type)].noun);
    send_to_char(buf,ch);
  }
  sprintf( buf, "Fighting: %s    Stunned: %d\n\r",
	   victim->fighting ? victim->fighting->name : "(none)",
	   victim->stunned);
  send_to_char( buf, ch );


  // Added by Seytan for HUNT skill 1997
  if ( IS_NPC(victim) ) {
    if ( victim->hunting == NULL )
      sprintf(buf,"Hunting victim: (none)\n\r");
    else
      sprintf(buf, "Hunting victim: %s (%s)\n\r",
	      IS_NPC(victim->hunting) ? victim->hunting->short_descr
	      : victim->hunting->name,
	      IS_NPC(victim->hunting) ? "MOB" : "PLAYER" );
    send_to_char(buf,ch);
  }

  if ( !IS_NPC(victim) ) {
    sprintf( buf,
	     "Thirst: %d  Hunger: %d  Full: %d  Drunk: %d\n\r",
	     victim->pcdata->condition[COND_THIRST],
	     victim->pcdata->condition[COND_HUNGER],
	     victim->pcdata->condition[COND_FULL],
	     victim->pcdata->condition[COND_DRUNK] );
    send_to_char( buf, ch );
  }

  sprintf( buf, "Carry number: %d  Carry weight: %ld\n\r",
	   victim->carry_number, get_carry_weight(victim) / 10 );
  send_to_char( buf, ch );


  if (!IS_NPC(victim)) {
    sprintf( buf, 
	     "Age: %d  Played: %d  Last Level: %d  Timer: %d\n\r",
	     get_age(victim), 
	     (int) (victim->played + current_time - victim->logon) / 3600, 
	     victim->pcdata->last_level, 
	     victim->timer );
    send_to_char( buf, ch );
  }

  sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
  send_to_char(buf,ch);
    
  if (victim->comm) {
    sprintf(buf,"Comm: %s\n\r",comm_bit_name(victim->comm));
    send_to_char(buf,ch);
  }

  if (IS_NPC(victim) && victim->off_flags) {
    sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
    send_to_char(buf,ch);
  }

  if (victim->cstat(imm_flags)) {
    sprintf(buf, "Immune: %s\n\r",irv_bit_name(victim->cstat(imm_flags)));
    send_to_char(buf,ch);
  }

  if (victim->cstat(res_flags)) {
    sprintf(buf, "Resist: %s\n\r", irv_bit_name(victim->cstat(res_flags)));
    send_to_char(buf,ch);
  }

  if (victim->cstat(vuln_flags)) {
    sprintf(buf, "Vulnerable: %s\n\r", irv_bit_name(victim->cstat(vuln_flags)));
    send_to_char(buf,ch);
  }

  sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	  form_bit_name(victim->cstat(form)), part_bit_name(victim->cstat(parts)));
  send_to_char(buf,ch);

  if (victim->cstat(affected_by)) {
    sprintf(buf, "Affected by %s\n\r",
	    affect_bit_name(victim->cstat(affected_by)));
    send_to_char(buf,ch);
  }
  // Added by SinaC 2001
  if (victim->cstat(affected2_by)) {
    sprintf(buf, "Also affected by %s\n\r",
	    affect2_bit_name(victim->cstat(affected2_by)));
    send_to_char(buf,ch);
  }


  sprintf( buf, "Master: %s  Leader: %s  Pet: %s\n\r",
	   victim->master      ? victim->master->name   : "(none)",
	   victim->leader      ? victim->leader->name   : "(none)",
	   victim->pet 	    ? victim->pet->name	     : "(none)");
  send_to_char( buf, ch );

  if (!IS_NPC(victim)) {
    sprintf( buf, "Security: %d.\n\r", victim->pcdata->security ); /* OLC */
    send_to_char( buf, ch );					   /* OLC */
  }
  // Added by SinaC 2000
  sprintf( buf, "Trust level: %d.\n\r",victim->trust);
  send_to_char(buf,ch);

  // Added by SinaC 2001 for disease   removed by SinaC 2003
  //send_to_charf(ch,"Disease: %s.\n\r", flag_string( disease_flags, victim->cstat(disease)));

  sprintf( buf, "Short description: %s\n\rLong  description: %s",
	   victim->short_descr,
	   victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
  send_to_char( buf, ch );

  if ( IS_NPC(victim) && victim->spec_fun != 0 ) {
    sprintf(buf,"Mobile has special procedure %s.\n\r",
	    spec_name(victim->spec_fun));
    send_to_char(buf,ch);
  }

  if ( IS_NPC(victim) && victim->pIndexData->program != NULL )
    send_to_charf(ch,"Program: %s\n\r", victim->pIndexData->program->name );
  if ( /*IS_NPC(victim) &&*/ victim->clazz != NULL )
    send_to_charf(ch,"Clazz: %s\n\r", victim->clazz->name );

  for ( paf = victim->affected; paf != NULL; paf = paf->next ) {
    // Modified by SinaC 2001
    //afstring(buf,paf,ch,TRUE);
    // SinaC 2003: new affect system
    afstring( buf, paf, ch, FALSE );
    send_to_char( buf, ch );
  }

  if ( victim->ex_fields ) { // SinaC 2003
    send_to_charf(ch,"Extra fields:\n\r");
    dump_extra_fields( ch, victim );
  }

  return;
}


/* ofind and mfind replaced with vnum, vnum skill also added */

void do_vnum(CHAR_DATA *ch, const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  const char *string;

  string = one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  vnum obj <name>\n\r",ch);
    send_to_char("  vnum mob <name>\n\r",ch);
    send_to_char("  vnum skill/ability <ability name>\n\r",ch);
    return;
  }

  if (!str_cmp(arg,"obj")) {
    do_ofind(ch,string);
    return;
  }

  if (!str_cmp(arg,"mob") || !str_cmp(arg,"char")) { 
    do_mfind(ch,string);
    return;
  }

  if (!str_cmp(arg,"skill") || !str_cmp(arg,"spell")
      || !str_cmp(arg,"power") || !str_cmp(arg,"song")
      || !str_cmp(arg,"ability")) {
    do_slookup(ch,string);
    return;
  }
  /* do both */
  do_mfind(ch,argument);
  do_ofind(ch,argument);
}


void do_mfind( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  MOB_INDEX_DATA *pMobIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    {
      send_to_char( "Find whom?\n\r", ch );
      return;
    }

  fAll	= FALSE; /* !str_cmp( arg, "all" ); */
  found	= FALSE;
  nMatch	= 0;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_mob_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for ( vnum = 0; nMatch < top_mob_index; vnum++ ) {
    if ( ( pMobIndex = get_mob_index( vnum ) ) != NULL ) {
      nMatch++;
      if ( fAll || is_name( argument, pMobIndex->player_name ) ) {
	found = TRUE;
	sprintf( buf, "[%5d] %s\n\r",
		 pMobIndex->vnum, pMobIndex->short_descr );
	send_to_char( buf, ch );
      }
    }
  }
  
  if ( !found )
    send_to_char( "No mobiles by that name.\n\r", ch );

  return;
}



void do_ofind( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  OBJ_INDEX_DATA *pObjIndex;
  int vnum;
  int nMatch;
  bool fAll;
  bool found;

  one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    send_to_char( "Find what?\n\r", ch );
    return;
  }
  
  fAll	= FALSE; /* !str_cmp( arg, "all" ); */
  found	= FALSE;
  nMatch	= 0;

  /*
   * Yeah, so iterating over all vnum's takes 10,000 loops.
   * Get_obj_index is fast, and I don't feel like threading another link.
   * Do you?
   * -- Furey
   */
  for ( vnum = 0; nMatch < top_obj_index; vnum++ ) {
    if ( ( pObjIndex = get_obj_index( vnum ) ) != NULL ) {
      nMatch++;
      if ( fAll || is_name( argument, pObjIndex->name ) ) {
	found = TRUE;
	sprintf( buf, "[%5d] %s\n\r",
		 pObjIndex->vnum, pObjIndex->short_descr );
	send_to_char( buf, ch );
      }
    }
  }

  if ( !found )
    send_to_char( "No objects by that name.\n\r", ch );

  return;
}


void do_owhere(CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  bool found;
  int number = 0, max_found;

  found = FALSE;
  number = 0;
  max_found = 200;

  buffer = new_buf();

  if (argument[0] == '\0') {
    send_to_char("Find what?\n\r",ch);
    return;
  }
 
  for ( obj = object_list; obj != NULL; obj = obj->next ) {
    if ( !can_see_obj( ch, obj ) || !is_name( argument, obj->name )
	 ||   ch->level < obj->level)
      continue;
 
    found = TRUE;
    number++;
 
    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj )
      ;
 
    if ( in_obj->carried_by != NULL && can_see(ch,in_obj->carried_by)
	 &&   in_obj->carried_by->in_room != NULL)
      sprintf( buf, "%3d) %s is carried by %s [Room %d]\n\r",
	       number, obj->short_descr,PERS(in_obj->carried_by, ch),
	       in_obj->carried_by->in_room->vnum );
    else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
      sprintf( buf, "%3d) %s is in %s [Room %d]\n\r",
	       number, obj->short_descr,in_obj->in_room->name, 
	       in_obj->in_room->vnum);
    else
      sprintf( buf, "%3d) %s is somewhere\n\r",number, obj->short_descr);
 
    buf[0] = UPPER(buf[0]);
    add_buf(buffer,buf);
 
    if (number >= max_found)
      break;
  }
 
  if ( !found )
    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
  else
    page_to_char(buf_string(buffer),ch);

}


void do_mwhere( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  bool found;
  int count = 0;

  if ( argument[0] == '\0' ) {
    DESCRIPTOR_DATA *d;

    /* show characters logged */

    buffer = new_buf();
    for (d = descriptor_list; d != NULL; d = d->next) {
      if (d->character != NULL && d->connected == CON_PLAYING
	  &&  d->character->in_room != NULL && can_see(ch,d->character)
	  &&  can_see_room(ch,d->character->in_room)) {
	victim = d->character;
	count++;
	if (d->original != NULL)
	  sprintf(buf,"%3d) %s (in the body of %s) is in %s [%d]\n\r",
		  count, d->original->name,victim->short_descr,
		  victim->in_room->name,victim->in_room->vnum);
	else
	  sprintf(buf,"%3d) %s is in %s [%d]\n\r",
		  count, victim->name,victim->in_room->name,
		  victim->in_room->vnum);
	add_buf(buffer,buf);
      }
    }

    page_to_char(buf_string(buffer),ch);
    return;
  }

  found = FALSE;
  buffer = new_buf();
  for ( victim = char_list; victim != NULL; victim = victim->next ) {
    if ( victim->in_room != NULL
	 &&   is_name( argument, victim->name ) ) {
      found = TRUE;
      count++;
      sprintf( buf, "%3d) [%5d] %-28s [%5d] %s\n\r", count,
	       IS_NPC(victim) ? victim->pIndexData->vnum : 0,
	       IS_NPC(victim) ? victim->short_descr : victim->name,
	       victim->in_room->vnum,
	       victim->in_room->name );
      add_buf(buffer,buf);
    }
  }

  if ( !found )
    act( "You didn't find any $T.", ch, NULL, argument, TO_CHAR );
  else
    page_to_char(buf_string(buffer),ch);

  return;
}


/* RT set replaces sset, mset, oset, and rset */

void do_set( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];

  argument = one_argument(argument,arg);

  if (arg[0] == '\0') {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  set mob   <name> <field> <value>\n\r",ch);
    send_to_char("  set char  <name> <field> <value>\n\r",ch);
    send_to_char("  set obj   <name> <field> <value>\n\r",ch);
    send_to_char("  set room  <room> <field> <value>\n\r",ch);
    // Modified by SinaC 2000
    //send_to_char("  set skill <name> <spell or skill> <value>\n\r",ch);
    send_to_char("  set skill/ability <name> <ability name> perc|casting|level <value>\n\r",ch);
    return;
  }

  if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")) {
    do_mset(ch,argument);
    return;
  }

  if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell")
      || !str_prefix(arg,"power") || !str_prefix(arg,"song")
      || !str_prefix(arg,"ability") ){
    do_sset(ch,argument);
    return;
  }

  if (!str_prefix(arg,"object")) {
    do_oset(ch,argument);
    return;
  }

  if (!str_prefix(arg,"room")) {
    do_rset(ch,argument);
    return;
  }
  /* echo syntax */
  do_set(ch,"");
}

// Modified by SinaC 2000x
void do_sset( CHAR_DATA *ch, const char *argument ) {
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  char arg4 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int value;
  int sn;
  bool fAll;
  int what;

  argument = one_argument( argument, arg1 ); // char name
  argument = one_argument( argument, arg2 ); // skill/spell name
  argument = one_argument( argument, arg4 ); // perc | level | casting
  argument = one_argument( argument, arg3 ); // value

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' ) {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set skill/ability <name> <ability name> perc <value>\n\r", ch);
    send_to_char( "  set skill/ability <name> <ability name> casting <value>\n\r", ch);
    send_to_char( "  set skill/ability <name> <ability name> level <value>\n\r", ch);
    send_to_char( "  set skill/ability <name> all perc <value>\n\r",ch);  
    send_to_char( "   (use the name of the ability, not the number)\n\r",ch);
    send_to_char( "   perc means the ability percentage\n\r",ch);
    send_to_char( "   casting means casting level of the ability\n\r",ch);
    send_to_char( "   level means the player level when he will be able\n\r"
		  "    to use that ability if his/her classes doesn't give\n\r" 
		  "    him that ability\n\r",ch);
    return;
  }
  
  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
  
  if ( IS_NPC(victim) ) {
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }
  
  // Modified by SinaC 2001
  what = 0;
  if ( !str_cmp( arg4, "perc" ) )
    what = 1;
  else if ( !str_cmp( arg4, "casting" ) )
    what = 2;
  else if ( !str_cmp( arg4, "level" ) )
    what = 3;
  else {
    send_to_char("You have to specify 'perc', 'casting' or 'level'!\n\r",ch);
    return;
  }

  fAll = !str_cmp( arg2, "all" );
  if ( fAll && what != 1 ){
    send_to_char( "You may not use 'all' with 'level' or 'learned'!\n\r",ch);
    return;
  }
  sn = 0;
  if ( !fAll && ( sn = ability_lookup( arg2 ) ) < 0 ) {
    send_to_char( "No such ability.\n\r", ch );
    return;
  }

  /*
   * Snarf the value.
   */
  if ( !is_number( arg3 ) ) {
    send_to_char( "Value must be numeric.\n\r", ch );
    return;
  }

  value = atoi( arg3 );
  switch( what ) {
  case 1:
    if ( value < 0 || value > 100 ) {
      send_to_char( "Value range is 0 to 100.\n\r", ch );
      return;
    }
    if ( fAll ) {
      for ( sn = 0; sn < MAX_ABILITY; sn++ ) {
	if ( ability_table[sn].name != NULL // IMPORTANT: already known abilities are set
	     && victim->pcdata->ability_info[sn].learned > 0 )
	  // Modified by SinaC 2000
	  victim->pcdata->ability_info[sn].learned = value;
      }
      send_to_charf(ch,
		    "Every abilities has been set to %d%% for %s.\n\r",
		    value, NAME(victim));
    }
    else {
      // Modified by SinaC 2000
      victim->pcdata->ability_info[sn].learned = value;
      send_to_charf(ch,
		    "%s's '%s' has been set to %d%%.\n\r",
		    NAME(victim),ability_table[sn].name, value);
    }
    break;
  case 2:
    if ( value < 0 || value > ability_table[sn].nb_casting_level ){
      if ( ability_table[sn].nb_casting_level == 0 )
	send_to_charf( ch,
		       "The only acceptable casting level value for '%s' is 0\n\r",
		       ability_table[sn].name );
      else
	send_to_charf( ch,
		       "Casting level value range for '%s' is 0 to %d\n\r",
		       ability_table[sn].name,
		       ability_table[sn].nb_casting_level );
      return;
    }
    victim->pcdata->ability_info[sn].casting_level = value;
    send_to_charf(ch,
		  "%s's '%s' casting level has been set to %d.\n\r",
		  NAME(victim),ability_table[sn].name, value);
    break;
  case 3:
    if ( value < 0 || value > IM ) {
      send_to_charf( ch, "Learned value range is 0 to %d\n\r", IM );
      return;
    }
    victim->pcdata->ability_info[sn].level = value;
    if ( victim->pcdata->ability_info[sn].learned == 0 )
      victim->pcdata->ability_info[sn].learned = 1;
    send_to_charf(ch,
		  "%s's '%s' learned level has been set to %d.\n\r",
		  NAME(victim), ability_table[sn].name, value );
    break;
  }

  return;
}



void do_mset( CHAR_DATA *ch, const char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  char buf[100];
  CHAR_DATA *victim;
  int value;
  // Added by SinaC 2000 so, mobiles we send hunting don't lose they zone
  AREA_DATA *saved_zone;

  //  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' ) {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  set char <name> <field> <value>\n\r",ch);
    send_to_char( "  Field being one of:\n\r",			ch );
    send_to_char( "    str int wis dex con sex class level\n\r",	ch );
    // Modified by SinaC 2001 for mental user
    //send_to_char( "    race group gold silver hp mana move prac\n\r",ch);
    send_to_char( "    race group gold silver hp mana move psp prac\n\r",ch);
    send_to_char( "    align etho train thirst hunger drunk full\n\r",	ch );
    send_to_char( "    hunt security hours god faction\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  // Added by SinaC 2000
  saved_zone = victim->zone;
  /* clear zones for mobs */
  victim->zone = NULL;

  /*
   * Snarf the value (which need not be numeric).
   */
  value = is_number( arg3 ) ? atoi( arg3 ) : -1;

  /*
   * Set something.
   */
  if ( !str_cmp( arg2, "security" ) ) {	/* OLC */
    if ( IS_NPC( victim ) ) {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
    
    if ( value > ch->pcdata->security || value < 0 ) {
      if ( ch->pcdata->security != 0 ) {
	sprintf( buf, "Valid security is 0-%d.\n\r",
		 ch->pcdata->security );
	send_to_char( buf, ch );
      }
      else {
	send_to_char( "Valid security is 0 only.\n\r", ch );
      }
      return;
    }
    send_to_charf(ch, "%s' security set to %d\n\r", NAME( victim ), value );
    victim->pcdata->security = value;
    return;
  }

  if ( !str_cmp( arg2, "str" ) ) {
    if ( value < 3 || value > get_max_train(victim,STAT_STR) ) {
      sprintf(buf,
	      "Strength range is 3 to %d.\n\r",
	      get_max_train(victim,STAT_STR));
      send_to_char(buf,ch);
      return;
    }
    send_to_charf(ch, "%s's strength set to %d.\n\r",
		  NAME(victim), value );
    victim->bstat(STR) = value;
    recompute(victim);
    return;
  }
  
  if ( !str_cmp( arg2, "int" ) ) {
    if ( value < 3 || value > get_max_train(victim,STAT_INT) ) {
      sprintf(buf,
	      "Intelligence range is 3 to %d.\n\r",
	      get_max_train(victim,STAT_INT));
      send_to_char(buf,ch);
      return;
    }
    send_to_charf(ch, "%s's intelligence set to %d.\n\r",
		  NAME(victim), value );
    victim->bstat(INT) = value;
    recompute(victim);
    return;
  }

  if ( !str_cmp( arg2, "wis" ) ) {
    if ( value < 3 || value > get_max_train(victim,STAT_WIS) ) {
      sprintf(buf,
	      "Wisdom range is 3 to %d.\n\r",get_max_train(victim,STAT_WIS));
      send_to_char( buf, ch );
      return;
    }
    send_to_charf(ch, "%s's wisdom set to %d.\n\r",
		  NAME(victim), value );
    victim->bstat(WIS) = value;
    recompute(victim);
    return;
  }
  
  if ( !str_cmp( arg2, "dex" ) ) {
    if ( value < 3 || value > get_max_train(victim,STAT_DEX) ) {
      sprintf(buf,
	      "Dexterity ranges is 3 to %d.\n\r",
	      get_max_train(victim,STAT_DEX));
      send_to_char( buf, ch );
      return;
    }
    send_to_charf(ch, "%s's dexterity set to %d.\n\r",
		  NAME(victim), value );
    victim->bstat(DEX) = value;
    recompute(victim);
    return;
  }
  
  if ( !str_cmp( arg2, "con" ) ) {
    if ( value < 3 || value > get_max_train(victim,STAT_CON) ) {
      sprintf(buf,
	      "Constitution range is 3 to %d.\n\r",
	      get_max_train(victim,STAT_CON));
      send_to_char( buf, ch );
      return;
    }
    send_to_charf(ch, "%s's constitution set to %d.\n\r",
		  NAME(victim), value );
    victim->bstat(CON) = value;
    recompute(victim);
    return;
  }
  
  if ( !str_prefix( arg2, "sex" ) ) {
    if ( ( value = flag_value( sex_flags, arg3 ) ) == NO_FLAG ) {
      send_to_char( "Invalid sex\n\r", ch );
      return;
    }
    send_to_charf(ch, "%s' sex set.\n\r", NAME( victim ) );
    victim->bstat(sex) = value;
    recompute(victim);
    return;
  }

  if ( !str_prefix( arg2, "class" ) ) {
    int iclass;
    
    iclass = class_lookup(arg3,TRUE);
    if ( iclass == -1 ) {
      char buf[MAX_STRING_LENGTH];
      
      strcpy( buf, "Available classes are: " );
      for ( iclass = 0; iclass < MAX_CLASS; iclass++ ) {
	if ( iclass > 0 )
	  strcat( buf, "   " );
	strcat( buf, class_table[iclass].name );
      }
      strcat( buf, ".\n\r" );
      
      send_to_char(buf,ch);
      return;
    }
    
    victim->bstat(classes) ^= 1<<iclass;
    if (victim->bstat(classes) == 0 && !IS_NPC(victim)) {
      send_to_char("Classless players are NOT allowed!\n\r",ch);
      victim->bstat(classes) = 1<<iclass; /* restore the previous class */
      return;
    } 
    else
      recompute(victim);
    send_to_charf(ch, "%s's classes set.\n\r", NAME( victim ) );
    return;
  }
  
  if ( !str_prefix( arg2, "level" ) ) {
    if ( !IS_NPC(victim) ) {
      send_to_char( "Not on PC's.\n\r", ch );
      return;
    }
    
    if ( value < 0 || value > 150 ) {
      send_to_char( "Level range is 0 to 150.\n\r", ch );
      return;
    }
    send_to_charf(ch, "%s's level set to %d.\n\r",
		  NAME(victim), value );
    victim->level = value;
    recompute(victim);
    return;
  }

  if ( !str_prefix( arg2, "gold" ) ) {
    victim->gold = value;
    send_to_charf(ch,
		  "%s has %d gold now.\n\r",
		  NAME(victim), value );
    return;
  }

  if ( !str_prefix(arg2, "silver" ) ) {
    victim->silver = value;
    send_to_charf(ch,
		  "%s has %d silver now.\n\r",
		  NAME(victim), value );
    return;
  }
  
  if ( !str_prefix( arg2, "hp" ) ) {
    if ( value < -10 || value > 30000 ) {
      send_to_char( "Hp range is -10 to 30,000 hit points.\n\r", ch );
      return;
    }
    victim->bstat(max_hit) = value;
    recompute(victim);
    send_to_charf(ch,
		  "%s has %d hp now.\n\r",
		  NAME(victim), value );
    return;
  }
  
  if ( !str_prefix( arg2, "mana" ) ) {
    if ( value < 0 || value > 30000 ) {
      send_to_char( "Mana range is 0 to 30,000 mana points.\n\r", ch );
      return;
    }
    victim->bstat(max_mana) = value;
    recompute(victim);
    send_to_charf(ch,
		  "%s has %d mana now.\n\r",
		  NAME(victim), value );
    return;
  }

  // Added by SinaC 2001 for mental user
  if ( !str_prefix( arg2, "psp" ) ) {
    if ( value < 0 || value > 30000 ) {
      send_to_char( "Psp range is 0 to 30,000 psp points.\n\r", ch );
      return;
    }
    victim->bstat(max_psp) = value;
    recompute(victim);
    send_to_charf(ch,
		  "%s has %d psp now.\n\r",
		  NAME(victim), value );
    return;
  }

  
  if ( !str_prefix( arg2, "move" ) ) {
    if ( value < 0 || value > 30000 ) {
      send_to_char( "Move range is 0 to 30,000 move points.\n\r", ch );
      return;
    }
    victim->bstat(max_move) = value;
    recompute(victim);
    send_to_charf(ch,
		  "%s has %d moves now.\n\r",
		  NAME(victim), value );
    return;
  }
  
  if ( !str_prefix( arg2, "practice" ) ) {
    if ( value < 0 || value > 250 ) {
      send_to_char( "Practice range is 0 to 250 sessions.\n\r", ch );
      return;
    }
    victim->practice = value;
    send_to_charf(ch,
		  "%s has %d practices now.\n\r",
		  NAME(victim), value );
    return;
  }
  
  if ( !str_prefix( arg2, "train" )) {
    if (value < 0 || value > 50 ) {
      send_to_char("Training session range is 0 to 50 sessions.\n\r",ch);
      return;
    }
    victim->train = value;
    send_to_charf(ch,
		  "%s has %d training sessions now.\n\r",
		  NAME(victim), value );
    return;
  }
  
  if ( !str_prefix( arg2, "align" ) ) {
    if ( value < -1000 || value > 1000 ) {
      send_to_char( "Alignment range is -1000 to 1000.\n\r", ch );
      return;
    }
    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000
    //victim->align.alignment = value;
    victim->bstat(alignment) = value;
    recompute(victim);
    send_to_charf(ch,
		  "%s has now an alignment equals to %d.\n\r",
		  NAME(victim), value );
    return;
  }
  
  if ( !str_prefix( arg2, "etho" ) ) {
    // Modified by SinaC 2001 etho/alignment are attributes now
    // Modified by SinaC 2000 for alignment/etho
    if ( !str_cmp( arg3, "chaotic" ) )
      //victim->align.etho = -1;
      victim->bstat(etho) = -1;
    else if ( !str_cmp( arg3, "neutral" ) )
      //victim->align.etho = 0;
      victim->bstat(etho) = 0;
    else if ( !str_cmp( arg3, "lawful" ) )
      //victim->align.etho = 1;
      victim->bstat(etho) = 1;
    else
      send_to_char( "Acceptable ethos are chaotic, neutral and lawful.\n\r", ch );
    recompute(victim);
    send_to_charf(ch,
		  "%s's etho is now %s.\n\r",
		  //NAME(victim), etho_name(victim->align.etho));
		  NAME(victim), etho_name(victim->bstat(etho)));
    return;
  }

  if ( !str_prefix( arg2, "thirst" ) ) {
    if ( IS_NPC(victim) ) {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
    
    if ( value < -1 || value > 100 ) {
      send_to_char( "Thirst range is -1 to 100.\n\r", ch );
      return;
    }
    
    victim->pcdata->condition[COND_THIRST] = value;
    return;
  }
  
  if ( !str_prefix( arg2, "drunk" ) ) {
    if ( IS_NPC(victim) ) {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
    
    if ( value < -1 || value > 100 ) {
      send_to_char( "Drunk range is -1 to 100.\n\r", ch );
      return;
    }
    
    victim->pcdata->condition[COND_DRUNK] = value;
    return;
  }
  
  if ( !str_prefix( arg2, "full" ) ) {
    if ( IS_NPC(victim) ) {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
    
    if ( value < -1 || value > 100 ) {
      send_to_char( "Full range is -1 to 100.\n\r", ch );
      return;
    }
    
    victim->pcdata->condition[COND_FULL] = value;
    return;
  }
  
  if ( !str_prefix( arg2, "hunger" ) ) {
    if ( IS_NPC(victim) ) {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
    
    if ( value < -1 || value > 100 ) {
      send_to_char( "Full range is -1 to 100.\n\r", ch );
      return;
    }
    
    victim->pcdata->condition[COND_HUNGER] = value;
    return;
  }

  if ( !str_prefix( arg2, "hours" ) ) {
    if ( IS_NPC(victim) ) {
      send_to_char( "Not on NPC's.\n\r", ch );
      return;
    }
    
    if ( value < 0 ) {
      send_to_char( "Must be a non-negative number.\n\r", ch );
      return;
    }
    
    victim->played = value*3600;

    send_to_charf(ch,
		  "%s's hours are now %d.\n\r",
		  NAME(victim), value );
    return;
  }
  
  /* Added by Seytan for HUNT skill 1997 */

  if (!str_prefix(arg2, "hunt")) {
    CHAR_DATA *hunted;
    
    if ( !IS_NPC(victim) ) {
      send_to_char( "Not on PC's.\n\r", ch );
      return;
    }
    
    // What's the utility of that test ?
    //        if ( str_cmp( arg3, "." ) )
    if ( (hunted = get_char_world(victim, arg3)) == NULL ) {/* Check for Area */
      send_to_char("Mob couldn't locate the victim to hunt.\n\r", ch);
      return;
    }
    
    // Added by SinaC 2000, the victim gets its zone back
    victim->zone = saved_zone;
    
    victim->hunting = hunted;
    
    send_to_charf(ch,
		  "%s will now hunt %s till death.\n\r",
		  NAME(victim),NAME(hunted));

    return;
  }               
  
  if (!str_prefix( arg2, "race" ) ) {
    int race;
    
    race = race_lookup(arg3);
    
    // Modified by SinaC 2000
    //if ( race == 0)
    if ( race < 0 ) {
      send_to_char("That is not a valid race.\n\r",ch);
      return;
    }
    
    if (!IS_NPC(victim) && !race_table[race].pc_race) {
      send_to_char("That is not a valid player race.\n\r",ch);
      return;
    }

    change_race( victim, race, 0, 0, -1, 0 );
    recompute(victim);

    send_to_charf(ch,
		  "%s's race is now %s.\n\r ",
		  NAME( victim ), race_table[race].name );

    return;
  }
  
  if (!str_prefix(arg2,"group")) {
    if (!IS_NPC(victim)) {
      send_to_char("Only on NPCs.\n\r",ch);
      return;
    }
    victim->group = value;
    return;
  }

  // Added by SinaC 2001 for god
  if ( !str_prefix(arg2,"god")){
    if (IS_NPC(victim)){
      send_to_char("Only on PCs.\n\r",ch);
      return;
    }
    int god = god_lookup( arg3 );
    if ( god < 0 ){
      send_to_char("Invalid god.\n\r",ch);
      return;
    }
    if ( !check_god( victim, god ) ){
      send_to_charf(ch,"That player can't worship %s.\n\r",god_name(god));
      return;
    }
    victim->pcdata->god = god;
    send_to_charf(ch,
		  "%s now worships %s.\n\r",
		  victim->name, god_name( god ) );
    return;
  }

  // SinaC 2003
  if ( !str_prefix( arg2, "faction" ) ) {
    if ( !IS_NPC(victim) ) { // PC:  set char <name> <faction name> <new value>
      char arg4[MAX_STRING_LENGTH];//                    arg4           arg3
      strcpy( arg3, one_argument( arg3, arg4 ) );
      log_stringf("arg3: %s  arg4: %s", arg3, arg4 );
      if ( arg4[0] == '\0' || arg3[0] == '\0'
	   || !is_number( arg3 ) ) {
	send_to_charf(ch,"Syntax: set char <name> <faction name> <new value>\n\r");
	return;
      }
      int factionId = get_faction_id( arg4 );
      if ( factionId < 0 ) {
	send_to_charf(ch,"Faction [%s] doesn't exist.\n\r"
		      "Type 'dlist faction' for a list of factions.\n\r", arg4 );
	return;
      }
      int value = URANGE( MIN_FACTION_VALUE, atoi(arg3), MAX_FACTION_VALUE );
      victim->pcdata->base_faction_friendliness[factionId] = value;
      recompute(victim);
      send_to_charf(ch,"Faction [%s] set to %d\n\r", faction_table[factionId].name, value );
      return;
    }
    else { // NPC: set mob <name> <faction name>
      int factionId = get_faction_id( arg3 );
      if ( factionId < 0 ) {
	send_to_charf(ch,"Faction [%s] doesn't exist.\n\r"
		      "Type 'dlist faction' for a list of factions.\n\r", arg3 );
	return;
      }
      victim->faction = factionId;
      send_to_charf(ch,"Faction set to %s\n\r", faction_table[victim->faction].name );
      return;
    }
  }

  /*
   * Generate usage message.
   */
  do_mset( ch, "" );
  return;
}

void do_string( CHAR_DATA *ch, const char *argument )
{
  char type [MAX_INPUT_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  OBJ_DATA *obj;

  //  smash_tilde( argument );
  argument = one_argument( argument, type );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( type[0] == '\0' || arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' ) {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  string char <name> <field> <string>\n\r",ch);
    send_to_char("    fields: name short long desc title spec\n\r",ch);
    send_to_char("  string obj  <name> <field> <string>\n\r",ch);
    send_to_char("    fields: name short long extended\n\r",ch);
    return;
  }
    
  if (!str_prefix(type,"character") || !str_prefix(type,"mobile")) {
    if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

    /* clear zone for mobs */
    victim->zone = NULL;

    /* string something */

    if ( !str_prefix( arg2, "name" ) ) {
      if ( !IS_NPC(victim) )
	{
	  send_to_char( "Not on PC's.\n\r", ch );
	  return;
	}
      victim->name = str_dup( arg3 );
      return;
    }
    	
    if ( !str_prefix( arg2, "description" ) )	{
      victim->description = str_dup(arg3);
      return;
    }

    if ( !str_prefix( arg2, "short" ) ) {
      victim->short_descr = str_dup( arg3 );
      return;
    }

    if ( !str_prefix( arg2, "long" ) ) {
      strcat(arg3,"\n\r");
      victim->long_descr = str_dup( arg3 );
      return;
    }

    if ( !str_prefix( arg2, "title" ) ) {
      if ( IS_NPC(victim) ) {
	send_to_char( "Not on NPC's.\n\r", ch );
	return;
      }

      set_title( victim, arg3 );
      return;
    }

    if ( !str_prefix( arg2, "spec" ) ) {
      if ( !IS_NPC(victim) ) {
	send_to_char( "Not on PC's.\n\r", ch );
	return;
      }

      if ( ( victim->spec_fun = spec_lookup( arg3 ) ) == 0 ) {
	send_to_char( "No such spec fun.\n\r", ch );
	return;
      }

      return;
    }
  }
    
  if (!str_prefix(type,"object")) {
    /* string an obj */
    	
    if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL ) {
      send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
      return;
    }
    	
    if ( !str_prefix( arg2, "name" ) ) {
      obj->name = str_dup( arg3 );
      return;
    }

    if ( !str_prefix( arg2, "short" ) ) {
      obj->short_descr = str_dup( arg3 );
      return;
    }

    if ( !str_prefix( arg2, "long" ) ) {
      obj->description = str_dup( arg3 );
      return;
    }

    if ( !str_prefix( arg2, "ed" ) 
	 || !str_prefix( arg2, "extended")) {
      EXTRA_DESCR_DATA *ed;

      argument = one_argument( argument, arg3 );
      if ( argument == NULL ) {
	send_to_char( "Syntax: oset <object> ed <keyword> <string>\n\r",
		      ch );
	return;
      }

      char buf_arg[MAX_INPUT_LENGTH];
      sprintf(buf_arg,"%s\n\r",argument);

      ed = new_extra_descr();

      ed->keyword		= str_dup( arg3     );
      ed->description	= str_dup( buf_arg );
      ed->next		= obj->extra_descr;
      obj->extra_descr	= ed;
      return;
    }
  }
    
    	
  /* echo bad use message */
  do_string(ch,"");
}



void do_oset( CHAR_DATA *ch, const char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  int value;
  // Added by SinaC 2000
  CHAR_DATA *owner;

  //  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' ) {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  set obj <object> <field> <value>\n\r",ch);
    send_to_char("  Field being one of:\n\r",				ch );
    send_to_char("    value0 value1 value2 value3 value4 (v1-v4)\n\r",ch );
    send_to_char("    extra wear level weight cost timer\n\r",	ch );
    // Added by SinaC 2000, modified by SinaC 2001, size added by SinaC 2003
    send_to_char("    owner condition size\n\r",ch );
    return;
  }
  
  if ( ( obj = get_obj_world( ch, arg1 ) ) == NULL ) {
    send_to_char( "Nothing like that in heaven or earth.\n\r", ch );
    return;
  }

  /*
   * Snarf the value (which need not be numeric).
   */
  value = atoi( arg3 );

  /*
   * Set something.
   */
  if ( !str_cmp( arg2, "value0" ) || !str_cmp( arg2, "v0" ) ) {
    obj->baseval[0] = value;
    recompobj(obj);
    return;
  }

  if ( !str_cmp( arg2, "value1" ) || !str_cmp( arg2, "v1" ) ) {
    obj->baseval[1] = value;
    recompobj(obj);
    return;
  }

  if ( !str_cmp( arg2, "value2" ) || !str_cmp( arg2, "v2" ) ) {
    obj->baseval[2] = value;
    recompobj(obj);
    return;
  }

  if ( !str_cmp( arg2, "value3" ) || !str_cmp( arg2, "v3" ) ) {
    obj->baseval[3] = value;
    recompobj(obj);
    return;
  }
  
  if ( !str_cmp( arg2, "value4" ) || !str_cmp( arg2, "v4" ) ) {
    obj->baseval[4] = value;
    recompobj(obj);
    return;
  }

  // Added by SinaC 2001
  if ( !str_prefix( arg2, "condition" ) ) {
    if ( value < 0 || value > 100 ) {
      send_to_char("Condition must be between 0 and 100!\n\r", ch );
      return;
    }
    obj->condition = value;
    recompobj( obj );
    send_to_charf(ch,"Condition set to %d.\n\r",value);
    return;
  }
  
  // Added by SinaC 2000
  if ( !str_prefix( arg2, "owner" ) ){
    
    if (!str_cmp(arg3,"none")){
      obj->owner = NULL;
      act("$p has no owner now.",ch,obj,NULL,TO_CHAR);
    }
    else{
      
      if ( ( owner = get_char_world(ch,arg3) ) == NULL ){
	send_to_charf(ch,"You can't find %s.\n\r",arg3);
	return;
      }
      if ( IS_NPC(owner) ){
	send_to_charf(ch,"%s is a NPC!\n\r",arg3);
	return;
      }
      act("$p's owner is now $N.",ch,obj,owner,TO_CHAR);
      //obj->owner = str_dup_capitalize(arg3);
      char buf[MAX_INPUT_LENGTH];
      strcpy(buf,arg3);
      buf[0] = UPPER(buf[0]);
      obj->owner = str_dup(buf);
    }
    return;
  }

  if ( !str_prefix( arg2, "extra" ) ) {
    if ( ( value = flag_value( extra_flags, arg3 ) ) == NO_FLAG ) {
      send_to_char("Invalid extra flags\n\r", ch );
      return;
    }
    obj->base_extra = value;
    recompobj(obj);
    return;
  }
  
  if ( !str_prefix( arg2, "wear" ) ) {
    if ( ( value = flag_value( wear_flags, arg3 ) ) == NO_FLAG ) {
      send_to_char("Invalid wear flags\n\r", ch );
      return;
    }
    obj->wear_flags = value;
    return;
  }
  
  if ( !str_prefix( arg2, "level" ) ) {
    if ( value < 1 || value > 110 ){
      send_to_char("Invalid level range is 1 .. 110 \n\r", ch );
      return;
    }
    obj->level = value;
    return;
  }
	
  if ( !str_prefix( arg2, "weight" ) ) {
    obj->weight = value;
    return;
  }

  if ( !str_prefix( arg2, "cost" ) ) {
    obj->cost = value;
    return;
  }

  if ( !str_prefix( arg2, "timer" ) ) {
    obj->timer = value;
    return;
  }

  // Added by SinaC 2003, size is not anymore a restriction but an obj stat
  if ( !str_prefix( arg2, "size" ) ) {
    if ( ( value = flag_value( size_flags, arg3 ) ) == NO_FLAG ) {
      send_to_char("Invalid size flags\n\r", ch );
      return;
    }
    obj->size = value;
    return;
  }

	
  /*
   * Generate usage message.
   */
  do_oset( ch, "" );
  return;
}



void do_rset( CHAR_DATA *ch, const char *argument )
{
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  ROOM_INDEX_DATA *location;
  int value;

  //  smash_tilde( argument );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  strcpy( arg3, argument );

  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' ) {
    send_to_char( "Syntax:\n\r",ch);
    send_to_char( "  set room <location> <field> <value>\n\r",ch);
    send_to_char( "  Field being one of:\n\r",			ch );
    send_to_char( "    flags sector\n\r",				ch );
    return;
  }
  
  if ( ( location = find_location( ch, arg1 ) ) == NULL ) {
    send_to_char( "No such location.\n\r", ch );
    return;
  }

  if (!is_room_owner(ch,location) 
      && ch->in_room != location 
      && room_is_private(location) 
      && !IS_TRUSTED(ch,IMPLEMENTOR)) {
    send_to_char("That room is private right now.\n\r",ch);
    return;
  }

  /*
   * Snarf the value.
   */
  /*
    if ( !is_number( arg3 ) ) {
    send_to_char( "Value must be numeric.\n\r", ch );
    return;
    }
    value = atoi( arg3 );
  */
  /*
   * Set something.
   */
  if ( !str_prefix( arg2, "flags" ) ) {
    if ( ( value = flag_value( room_flags, arg3 ) ) == NO_FLAG ){
      send_to_char("Invalid flags\n\r", ch);
      return;
    }
    // Modified by SinaC 2001
    location->bstat(flags)	= value;
    return;
  }
  
  if ( !str_prefix( arg2, "sector" ) ) {
    if ( ( value = flag_value( room_flags, arg3 ) ) == NO_FLAG ){
      send_to_char("Invalid sector\n\r", ch);
      return;
    }
    // Modified by SinaC 2001
    location->bstat(sector)	= value;
    return;
  }
  
  /*
   * Generate usage message.
   */
  do_rset( ch, "" );
  return;
}

// Added by Seytan, Oxtal, Sinac 1997,  New additions by SinaC 2000

void do_gtrivia ( CHAR_DATA *ch, const char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
    
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
    
  if ( arg1[0] == '\0' || arg2[0] == '\0' || !is_number( arg2 ) ){
    send_to_char( "Syntax: gtrivia <char> <points>.\n\r", ch );
    return;
  }
  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ){
    send_to_char( "That player is not here.\n\r", ch);
    return;
  }

  if ( IS_NPC(victim) ){
    send_to_char( "Not on NPC's.\n\r", ch );
    return;
  }
    
  victim->pcdata->trivia += atoi (arg2);
  send_to_char("Ok.\n\r",ch);
  return;
}    

void do_olevel(CHAR_DATA *ch, const char *argument) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  OBJ_INDEX_DATA *pObjIndex;
  int vnum, level;
  int nMatch;
  bool found;

  argument=one_argument(argument, arg);
  if (!is_number(arg) ) {
    send_to_char("Syntax: olevel [level]\n\r",ch);
    return;
  }
  level = atoi(arg);	
  buffer=new_buf();
  found=FALSE;
  nMatch=0;
	
  for(vnum=0;nMatch<top_obj_index;vnum++) {
    if ( (pObjIndex=get_obj_index(vnum)) !=NULL) {
      nMatch++;
      if (level == pObjIndex->level) {
	found=TRUE;
	sprintf(buf, "[%5d] %s\n\r",pObjIndex->vnum,	
		pObjIndex->short_descr);
	add_buf(buffer,buf);
      }
    }  
  }
  if (!found)
    send_to_char("No objects by that level.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
  return;
}
/*
same apply for do_mlevel, just rewrite it and replace pObjIndex with 
pMobIndex, and declare it as MOB_INDEX_DATA....

The above two functions are used only in ROM2.4 mud, but Merc muds can alo
use them with porting buffers correspondingly....
*/
void do_mlevel(CHAR_DATA *ch, const char *argument) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  MOB_INDEX_DATA *pMobIndex;
  int vnum, level;
  int nMatch;
  bool found;

  argument=one_argument(argument, arg);
  if (!is_number(arg) ) {
    send_to_char("Syntax: mlevel [level]\n\r",ch);
    return;
  }
  level = atoi(arg);
  buffer=new_buf();
  found=FALSE;
  nMatch=0;
	
  for(vnum=0;nMatch<top_mob_index;vnum++) {
    if ( (pMobIndex=get_mob_index(vnum)) !=NULL) {
      nMatch++;
      if (level == pMobIndex->level) {
	found=TRUE;
	sprintf(buf, "[%5d] %s\n\r",pMobIndex->vnum,	
		pMobIndex->short_descr);
	add_buf(buffer,buf);
      }
    }  
  }
  if (!found)
    send_to_char("No mobiles by that level.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
  return;
}


// Oxtal
void do_xstat(CHAR_DATA *ch, const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  int i;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Stat whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }
    
  for (i=0;i<ATTR_NUMBER;i++) {
    send_to_charf(ch," %17s : ",attr_table[i].name);
    if (attr_table[i].bits == NULL) {
      send_to_charf(ch,"%ld (%ld)\n\r",
		    victim->curattr[i],
		    victim->baseattr[i]);
    }
    else {
      sprintf( buf1, "%s", flag_string(attr_table[i].bits,victim->curattr[i]) );
      sprintf( buf2, "%s", flag_string(attr_table[i].bits,victim->baseattr[i]) );
      send_to_charf(ch,"%s (%s)\n\r",
		    buf1,
		    buf2 );
    }
  }
}

void do_xset(CHAR_DATA *ch, const char * argument) {
  int vloc, vmod;
  CHAR_DATA *vict;
  char loc[MAX_INPUT_LENGTH], vct[MAX_INPUT_LENGTH];
  const char * mod;

  argument = one_argument( argument, vct );
  mod = one_argument( argument, loc );

  if ( vct[0] == 0 || loc[0] == 0 || mod[0] == 0) {
    send_to_char( "Syntax:  xset <char> <attrib> <value>\n\r", ch );
    return;
  }

  if ( (vict = get_char_world(ch,vct)) == NULL) {
    send_to_char("They are not there.\n\r",ch);
    return;   
  }

  if ( ( vloc = flag_value( attr_flags, loc ) ) == NO_FLAG ) {
    send_to_char( "Valid attribs are:\n\r", ch );
    show_help( ch, "attribs" );
    return;
  }

  if ( attr_table[vloc].bits == NULL) {
    if (!is_number(mod)) {
      send_to_char( "Value for this location is numeric.\n\r", ch );
      return;
    }
    vmod = atoi(mod);
  } 
  else {
    if ( ( vmod = flag_value( attr_table[vloc].bits, mod ) ) == NO_FLAG ) {
      send_to_char( "Invalid flag.\n\r", ch );
      return;
    }
  }

  send_to_char("Ok.\n\r",ch);
  // Modified by SinaC 2001
  if ( attr_table[vloc].bits == NULL)
    vict->baseattr[vloc] = vmod;
  else
    vict->baseattr[vloc] ^= vmod;
  recompute(vict);  
}

/* Made by Oxtal and heavily modified by SinaC 2000 */
void omatch_syntax(CHAR_DATA *ch) {
  send_to_char("Syntax: omatch <specifier> '<argument>' [<specifier> 'arg'] ...\n\r",ch);
  send_to_char(" specifiers:\n\r",ch);
  send_to_char(" level {{<level> | <minlevel maxlevel>}\n\r",ch);
  send_to_char(" name <list of names>\n\r",ch);
  send_to_char(" type <item type>\n\r",ch);
  // Added by SinaC 2003
  send_to_char(" size <size>\n\r",ch);
  send_to_char(" wear <wear flags>\n\r",ch);
  send_to_char(" extra <extra flags>\n\r",ch);
  send_to_char(" wclass <weapon class>\n\r",ch);
  send_to_char(" wflag <weapon flags>\n\r",ch);
  send_to_char(" vnum <minlevel maxlevel>\n\r",ch);
  send_to_char(" value0 <value>     or value1, value2, value3, value4\n\r",ch);
  send_to_char(" program <program name>\n\r",ch);
  send_to_char(" material <material name>\n\r",ch);
  send_to_char(" balance   check unbalanced items\n\r", ch );
  send_to_char(" points    show item points\n\r", ch );
  send_to_char(" buggy     check buggy items\n\r\n\r", ch );
  send_to_char("example: omatch level 1 10 balance type weapon\n\r",ch);
  send_to_char("         omatch level 5 buggy name foutain\n\r", ch );
  return;
}
void do_omatch(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  OBJ_INDEX_DATA *pObjIndex;  
  int vnum;
  int nMatch;
  bool found;

  char arg[MAX_INPUT_LENGTH];    
  char arg1[MAX_INPUT_LENGTH];  
  char arg2[MAX_INPUT_LENGTH];  
  char buf[MAX_STRING_LENGTH];  

  /* Criteria vars */
  char names[MAX_INPUT_LENGTH] = "";  
  int mlev=0, Mlev=999;
  long wear=NO_FLAG, typ=NO_FLAG, extra =NO_FLAG, wclass= NO_FLAG, wflag = NO_FLAG, siz = NO_FLAG;
  int mvnum = 0, Mvnum = 100000;
  int value0 = NO_FLAG, value1 = NO_FLAG, value2 = NO_FLAG, value3 = NO_FLAG, value4 = NO_FLAG;
  bool bal=FALSE, buggy=FALSE, points_on = FALSE;
  CLASS_DATA *cl = NULL;
  int imat = NO_FLAG;

  if (!argument[0]) { 
    omatch_syntax(ch);
    return;
  }

  argument=one_argument(argument, arg);
  while (*arg) {
    if (!str_cmp(arg,"balance"))
      bal = TRUE;
    else if (!str_cmp(arg,"buggy"))
      buggy = TRUE;
    else if (!str_cmp( arg, "points" ) )
      points_on = TRUE;
    else if (!str_cmp(arg,"level")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mlev = atoi(arg1);
      if (*arg2)
	Mlev = atoi(arg2);
      else
	Mlev = mlev;
    } 
    else if (!str_cmp(arg,"name")) {
      argument=one_argument(argument, arg1);
      strcpy(names,arg1);
    }
    else if (!str_cmp(arg,"wear")) {
      argument=one_argument(argument, arg1);
      wear = flag_value(wear_flags, arg1);
      if (wear == NO_FLAG) {
	send_to_charf(ch,"Wrong wear flags : %s\n\r",arg1);
	return;
      }        
    }    
    else if (!str_cmp(arg,"type")) {
      argument=one_argument(argument, arg1);
      typ = flag_value(type_flags,arg1);
      if (typ == NO_FLAG) {
	send_to_charf(ch,"Wrong item type : %s\n\r",arg1);
	return;
      }        
    }    
    // Added by SinaC 2003
    else if (!str_cmp(arg,"size")) {
      argument=one_argument(argument, arg1);
      siz = flag_value(size_flags,arg1);
      if (siz == NO_FLAG) {
	send_to_charf(ch,"Wrong size : %s\n\r",arg1);
	return;
      }        
    }    
    // Added by SinaC 2000
    else if (!str_cmp(arg,"extra")) {
      argument=one_argument(argument, arg1);
      extra = flag_value(extra_flags,arg1);
      if (extra == NO_FLAG) {
	send_to_charf(ch,"Wrong extra flags : %s\n\r",arg1);
	return;
      }        
    }    
    else if (!str_cmp(arg,"wclass")) {
      argument=one_argument(argument, arg1);
      wclass = flag_value(weapon_class,arg1);
      if (wclass == NO_FLAG) {
	send_to_charf(ch,"Wrong wclass type : %s\n\r",arg1);
	return;
      }        
    } 
    else if (!str_cmp(arg,"wflag")) {
      argument=one_argument(argument, arg1);
      wflag = flag_value(weapon_type2,arg1);
      if (wflag == NO_FLAG) {
	send_to_charf(ch,"Wrong wflag type : %s\n\r",arg1);
	return;
      }        
    } 
    else if (!str_cmp(arg,"vnum")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mvnum = atoi(arg1);
      if (*arg2)
	Mvnum = atoi(arg2);
      else
	Mvnum = mvnum;

    }
    // Added by SinaC 2001
    else if (!str_cmp(arg,"value0")) {
      argument=one_argument(argument, arg1);
      if ( !is_number(arg1) ) {
	send_to_charf(ch,"Value has to be an integer\n\r");
	return;
      }
      value0 = atoi(arg1);
    }    
    // Added by SinaC 2001
    else if (!str_cmp(arg,"value1")) {
      argument=one_argument(argument, arg1);
      if ( !is_number(arg1) ) {
	send_to_charf(ch,"Value has to be an integer\n\r");
	return;
      }
      value1 = atoi(arg1);
    }    
    // Added by SinaC 2001
    else if (!str_cmp(arg,"value2")) {
      argument=one_argument(argument, arg1);
      if ( !is_number(arg1) ) {
	send_to_charf(ch,"Value has to be an integer\n\r");
	return;
      }
      value2 = atoi(arg1);
    }    
    // Added by SinaC 2001
    else if (!str_cmp(arg,"value3")) {
      argument=one_argument(argument, arg1);
      if ( !is_number(arg1) ) {
	send_to_charf(ch,"Value has to be an integer\n\r");
	return;
      }
      value3 = atoi(arg1);
    }    
    // Added by SinaC 2001
    else if (!str_cmp(arg,"value4")) {
      argument=one_argument(argument, arg1);
      if ( !is_number(arg1) ) {
	send_to_charf(ch,"Value has to be an integer\n\r");
	return;
      }
      value4 = atoi(arg1);
    }    
    // Added by SinaC 2003
    else if (!str_cmp(arg,"program")) {
      argument=one_argument(argument, arg1);
      if (!(cl = silent_hash_get_prog(arg1))) {
	send_to_char("No such program.\n\r",ch);
	return;
      }
    }
    else if ( !str_cmp( arg, "material")) {
      argument=one_argument(argument, arg1);
      if ((imat = material_lookup(arg1))<0) {
	send_to_char("No such material.\n\r",ch);
	return;
      }
    }
    else {
      send_to_charf(ch,"Specifier %s unknown\n\r",arg);
      omatch_syntax(ch);
      return;
    }

    argument=one_argument(argument, arg);
  }
  
  buffer=new_buf();
  found=FALSE;

  nMatch=0;

  for(vnum=0;nMatch<top_obj_index;vnum++) {
    if ( (pObjIndex=get_obj_index(vnum)) !=NULL) {
      nMatch++;
      int problem; // not used
      int points = get_item_points( pObjIndex, problem );
      if ( mlev <= pObjIndex->level
	   && Mlev >= pObjIndex->level
	   && (is_name(names,pObjIndex->name) || !*names )
	   && ((pObjIndex->wear_flags & wear ) || wear == NO_FLAG )
	   // SinaC 2000
	   && ((pObjIndex->extra_flags & extra ) || extra == NO_FLAG )

	   && ((pObjIndex->item_type == ITEM_WEAPON && pObjIndex->value[0]==wclass) || wclass == NO_FLAG )
	   && ((pObjIndex->item_type == ITEM_WEAPON && pObjIndex->value[4]==wflag) || wflag == NO_FLAG )
	   
	   && (pObjIndex->item_type == typ || typ == NO_FLAG)
	   // Added by SinaC 2003
	   && (pObjIndex->size == siz || siz == NO_FLAG)
	   
	   && ( ( points == pObjIndex->level && problem == 0 ) || !bal ) // Modified by SinaC 2003
	   && (is_item_buggy(pObjIndex) || !buggy ) 
	   && mvnum <= pObjIndex->vnum
	   && Mvnum >= pObjIndex->vnum 
	   && ( pObjIndex->value[0] == value0 || value0 == NO_FLAG )
	   && ( pObjIndex->value[1] == value1 || value1 == NO_FLAG )
	   && ( pObjIndex->value[2] == value2 || value2 == NO_FLAG )
	   && ( pObjIndex->value[3] == value3 || value3 == NO_FLAG )
	   && ( pObjIndex->value[4] == value4 || value4 == NO_FLAG )
	   // Added by SinaC 2003
	   && ( pObjIndex->program == cl || cl == NULL )
	   && ( pObjIndex->material == imat || imat == NO_FLAG )
	   ) {
	found=TRUE;
	// Modified by SinaC 2001
	if ( points_on )
	  sprintf(buf, "[%5d]%c%3d%c %-45s %s\n\r",
		  pObjIndex->vnum,
		  problem>0?'<':'[',
		  points,
		  problem>0?'>':']',
		  pObjIndex->short_descr,
		  pObjIndex->area->name);
	else
	  sprintf(buf, "[%5d] %-45s %s\n\r",
		  pObjIndex->vnum,
		  pObjIndex->short_descr,
		  pObjIndex->area->name);
	add_buf(buffer,buf);
      }
    }  

  }
  if (!found)
    send_to_char("No objects matching condition.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
}

/* Added by SinaC 2000 same code as omatch, thanks Oxtal */

void mmatch_syntax(CHAR_DATA *ch) {
  send_to_char("Syntax: mmatch <specifier> '<argument>' [<specifier> 'arg'] ...\n\r",ch);
  send_to_char(" specifiers:\n\r",ch);
  send_to_char(" level {{<level> | <minlevel maxlevel>}\n\r",ch);
  send_to_char(" name <list of names>\n\r",ch);
  send_to_char(" act <act flags>\n\r",ch);
  send_to_char(" off <offensive flags>\n\r",ch);
  send_to_char(" race <race name>\n\r",ch);
  send_to_char(" program <program name>\n\r",ch);
  send_to_char(" spec <spec name>\n\r",ch);
  send_to_char(" align {{<align> | <min align> <max align>}\n\r",ch);
  send_to_char(" vnum <min vnum> <max vnum>\n\r",ch);
  send_to_char(" parts <part flags>\n\r", ch );
  send_to_char(" form <form flags>\n\r", ch );
  send_to_char(" faction <faction name>\n\r", ch );
  send_to_char(" aff <affect flags>\n\r", ch );
  send_to_char(" aff2 <affect flags 2>\n\r", ch );
  return;
}
void do_mmatch(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  MOB_INDEX_DATA *pMobIndex;  
  int vnum;
  int nMatch;
  bool found;

  char arg[MAX_INPUT_LENGTH];    
  char arg1[MAX_INPUT_LENGTH];  
  char arg2[MAX_INPUT_LENGTH];  
  char buf[MAX_STRING_LENGTH];  



  /* Criteria vars */
  char names[MAX_INPUT_LENGTH] = "";  
  int mlev=0, Mlev=999;
  long act=NO_FLAG, off=NO_FLAG, race = NO_FLAG, parts = NO_FLAG, form = NO_FLAG, aff = NO_FLAG, aff2 = NO_FLAG;
  int malign=-1000, Malign=+1000;
  int mvnum=0, Mvnum=100000;
  bool shop = FALSE;
  CLASS_DATA *cl = NULL;
  SPEC_FUN *spec = NULL;
  int faction = NO_FLAG;

  if (!argument[0]) { mmatch_syntax(ch);return;}
  
  argument=one_argument(argument, arg);

  while (*arg) {
    if (!str_cmp(arg,"level")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mlev = atoi(arg1);
      if (*arg2)
	Mlev = atoi(arg2);
      else
	Mlev = mlev;
      if ( mlev > Mlev ) {
	send_to_char("Wrong levels: min level is higher than max level.\n\r", ch );
	return;
      }
    } 
    else if (!str_cmp(arg,"name")) {
      argument=one_argument(argument, arg1);
      strcpy(names,arg1);
    }
    else if (!str_cmp(arg,"act")) {
      argument=one_argument(argument, arg1);
      act = flag_value(act_flags,arg1);
      if (act == NO_FLAG) {
	send_to_charf(ch,"Wrong act flags : %s\n\r",arg1);
	return;
      }        
    }    
    else if (!str_cmp(arg,"off")) {
      argument=one_argument(argument, arg1);
      off = flag_value(off_flags,arg1);
      if (off == NO_FLAG) {
	send_to_charf(ch,"Wrong offensive flags : %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"race")) {
      argument=one_argument(argument, arg1);
      race = race_lookup(arg1);
      // Modified by SinaC 2000
      //if (race == 0) {
      if (race < 0) {
	send_to_charf(ch,"Wrong race : %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"align")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      malign = atoi(arg1);
      if (*arg2)
	Malign = atoi(arg2);
      else
	Malign = malign;

      if ( malign > Malign ) {
	send_to_char("Wrong align: min align is higher than max align.\n\r", ch );
	return;
      }
    }
    else if (!str_cmp(arg,"vnum")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mvnum = atoi(arg1);
      if (*arg2)
	Mvnum = atoi(arg2);
      else
	Mvnum = mvnum;
      if ( mvnum > Mvnum ) {
	send_to_char("Wrong vnum: min vnum is higher than max vnum.\n\r", ch );
	return;
      }
    }
    // Added by SinaC 2003
    else if (!str_cmp(arg,"program")) {
      argument=one_argument(argument, arg1);
      if (!(cl = silent_hash_get_prog(arg1))) {
	send_to_char("No such program.\n\r",ch);
	return;
      }
    }
    else if (!str_cmp(arg,"spec")) {
      argument=one_argument(argument, arg1);
      if (!(spec = spec_lookup(arg1))) {
	send_to_char("No such special function.\n\r",ch);
	return;
      }
    }
    else if (!str_cmp(arg,"shop")) {
      shop = TRUE;
    }
    else if (!str_cmp(arg,"parts")) {
      argument=one_argument(argument, arg1);
      parts = flag_value(part_flags,arg1);
      if (parts == NO_FLAG) {
	send_to_charf(ch,"Wrong parts flags : %s\n\r",arg1);
	return;
      }
    }
    else if (!str_cmp(arg,"form")) {
      argument=one_argument(argument, arg1);
      form = flag_value(part_flags,arg1);
      if (form == NO_FLAG) {
	send_to_charf(ch,"Wrong form flags : %s\n\r",arg1);
	return;
      }
    }
    else if (!str_cmp(arg,"faction")) {
      argument=one_argument(argument, arg1);
      faction = get_faction_id(arg1);
      if (faction < 0) {
	send_to_charf(ch,"Wrong faction : %s\n\r",arg1);
	return;
      }
    }
    else if (!str_cmp(arg,"aff")) {
      argument=one_argument(argument, arg1);
      aff = flag_value(affect_flags,arg1);
      if (aff == NO_FLAG) {
	send_to_charf(ch,"Wrong aff flags : %s\n\r",arg1);
	return;
      }
    }
    else if (!str_cmp(arg,"aff2")) {
      argument=one_argument(argument, arg1);
      aff2 = flag_value(affect2_flags,arg1);
      if (aff2 == NO_FLAG) {
	send_to_charf(ch,"Wrong aff2 flags : %s\n\r",arg1);
	return;
      }
    }
    else {
      send_to_charf(ch,"Specifier %s unknown\n\r",arg);
      mmatch_syntax(ch);
      return;
    }
    argument=one_argument(argument, arg);
  }
  
  buffer=new_buf();
  found=FALSE;

  nMatch=0;

  for(vnum=0;nMatch<top_mob_index;vnum++) {
    if ( (pMobIndex=get_mob_index(vnum)) !=NULL) {
      nMatch++;
      if (   mlev <= pMobIndex->level
	     && Mlev >= pMobIndex->level
	     && (is_name(names,pMobIndex->player_name) || !*names )
	     && ( IS_SET( pMobIndex->act, act ) || act == NO_FLAG )  
	     && ( IS_SET( pMobIndex->off_flags, off ) || off == NO_FLAG )
	     && ((pMobIndex->race == race ) || race == NO_FLAG )
	     && malign <= pMobIndex->align.alignment
	     && Malign >= pMobIndex->align.alignment 
	     && mvnum <= pMobIndex->vnum
	     && Mvnum >= pMobIndex->vnum 
	     // Added by SinaC 2003
	     && ( pMobIndex->program == cl || cl == NULL )
	     && ( pMobIndex->spec_fun == spec || spec == NULL )
	     && ( pMobIndex->pShop != NULL || shop == FALSE )
	     && ( IS_SET( pMobIndex->parts, parts ) || parts == NO_FLAG )
	     && ( IS_SET( pMobIndex->form, form ) || form == NO_FLAG )
	     && ( pMobIndex->faction == faction || faction == NO_FLAG )
	     && ( IS_SET( pMobIndex->affected_by, aff ) || aff == NO_FLAG )
	     && ( IS_SET( pMobIndex->affected2_by, aff2 ) || aff2 == NO_FLAG )
	     ) {
	found=TRUE;
	// Modified by SinaC 2001
	sprintf(buf, "[%5d] %-45s %s\n\r",
		pMobIndex->vnum, 
		pMobIndex->short_descr,
		pMobIndex->area->name);
	add_buf(buffer,buf);
      }
    }  

  }
  if (!found)
    send_to_char("No mobiles matching condition.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
}

/* Added by SinaC 2000 same code as omatch, thanks Oxtal */
void rmatch_syntax(CHAR_DATA *ch)
{
  send_to_char("Syntax: rmatch <specifier> '<argument>' [<specifier> 'arg'] ...\n\r",ch);
  send_to_char(" specifiers:\n\r",ch);
  send_to_char(" name <list of names>\n\r",ch);
  send_to_char(" vnum <min vnum> <max vnum>\n\r",ch);
  send_to_char(" sector <sector flag>\n\r",ch);
  send_to_char(" flag <room flag>\n\r",ch);
  send_to_char(" size <room maximal size>\n\r",ch);
  send_to_char(" program <program name>\n\r",ch);
  send_to_char(" repop <minvalue> <maxvalue>\n\r",ch);
  send_to_char(" repoppeople <minvalue> <maxvalue>\n\r",ch);
  return;
}
void do_rmatch(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  ROOM_INDEX_DATA *pRoomIndex;
  int vnum;
  int nMatch;
  bool found;

  char arg[MAX_INPUT_LENGTH];  
  char arg1[MAX_INPUT_LENGTH];  
  char arg2[MAX_INPUT_LENGTH];  
  char buf[MAX_STRING_LENGTH];  

  /* Criteria vars */
  char names[MAX_INPUT_LENGTH] = "";  
  int mvnum = 0, Mvnum = 100000;
  long sector=NO_FLAG;
  long flag=NO_FLAG;
  long size=NO_FLAG;
  CLASS_DATA *cl = NULL;
  int mrepop = MIN_REPOP_TIME,
    Mrepop = MAX_REPOP_TIME,
    mrepop_people = MIN_REPOP_TIME,
    Mrepop_people = MAX_REPOP_TIME;

  if (!argument[0]) { 
    rmatch_syntax(ch);
    return;
  }
  
  argument=one_argument(argument, arg);

  while (*arg) {
    if (!str_cmp(arg,"name")) {
      argument=one_argument(argument, arg1);
      strcpy(names,arg1);
    }
    else if (!str_cmp(arg,"sector")) {
      argument=one_argument(argument, arg1);
      sector = flag_value(sector_flags,arg1);
      if (sector == NO_FLAG) {
	send_to_charf(ch,"Wrong sector flags : %s\n\r",arg1);
	return;
      }        
    }    
    else if (!str_cmp(arg,"flag")) {
      argument=one_argument(argument, arg1);
      flag = flag_value(room_flags,arg1);
      if (flag == NO_FLAG) {
	send_to_charf(ch,"Wrong room flags : %s\n\r",arg1);
	return;
      }        
    }    
    else if (!str_cmp(arg,"size")) {
      argument=one_argument(argument, arg1);
      size = flag_value(size_flags,arg1);
      if (size == NO_FLAG) {
	send_to_charf(ch,"Wrong room size : %s\n\r",arg1);
	return;
      }        
    }    
    // Added by SinaC 2003
    else if (!str_cmp(arg,"program")) {
      argument=one_argument(argument, arg1);
      if (!(cl = silent_hash_get_prog(arg1))) {
	send_to_char("No such program.\n\r",ch);
	return;
      }
    }
    else if (!str_cmp(arg,"repoppeople")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mrepop_people = atoi(arg1);
      if (*arg2)
	Mrepop_people = atoi(arg2);
      else
	Mrepop_people = mrepop_people;
      if ( mrepop_people > Mrepop_people ) {
	send_to_char("Wrong repoppeople time: min repop is higher than max repop.\n\r", ch );
	return;
      }
    }
    else if (!str_cmp(arg,"repop")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mrepop = atoi(arg1);
      if (*arg2)
	Mrepop = atoi(arg2);
      else
	Mrepop = mrepop;
      if ( mrepop > Mrepop ) {
	send_to_char("Wrong repop: min repop is higher than max repop.\n\r", ch );
	return;
      }
    }
    else if (!str_cmp(arg,"vnum")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mvnum = atoi(arg1);
      if (*arg2)
	Mvnum = atoi(arg2);
      else
	Mvnum = mvnum;
      if ( mvnum > Mvnum ) {
	send_to_char("Wrong vnum: min vnum is higher than max vnum.\n\r", ch );
	return;
      }
    }
    else {
      send_to_charf(ch,"Specifier %s unknown\n\r",arg);
      rmatch_syntax(ch);
      return;
    }
    argument=one_argument(argument, arg);
  }
  
  buffer=new_buf();
  found=FALSE;

  nMatch=0;
	
  for(vnum=0;nMatch<top_room;vnum++) {
    if ( (pRoomIndex=get_room_index(vnum)) !=NULL) {
      nMatch++;
      if ( 
	  (is_name(names,pRoomIndex->name) || !*names )
	  // Modified by SinaC 2001
	  && ((pRoomIndex->cstat(sector) == sector ) || sector == NO_FLAG )
	  && (IS_SET(pRoomIndex->cstat(flags),flag ) || flag == NO_FLAG )
	  && ((pRoomIndex->cstat(maxsize) == size ) || size == NO_FLAG ) 
	  // Added by SinaC 2003
	  && mvnum <= pRoomIndex->vnum
	  && Mvnum >= pRoomIndex->vnum 
	  && (pRoomIndex->program == cl || cl == NULL ) 
	  && (pRoomIndex->time_between_repop_people >= mrepop_people 
	      && pRoomIndex->time_between_repop_people <= Mrepop_people )
	  && (pRoomIndex->time_between_repop >= mrepop 
	      && pRoomIndex->time_between_repop <= Mrepop )
	   ) {
	found=TRUE;
	sprintf(buf, "[%5d] %s\n\r",pRoomIndex->vnum, pRoomIndex->name);
	add_buf(buffer,buf);
      }
    }  
    
  }
  if (!found)
    send_to_char("No rooms matching condition.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
}

void exmatch_syntax(CHAR_DATA *ch) {
  send_to_char("Syntax: exmatch <specifier> '<argument>' [<specifier> 'arg'] ...\n\r",ch);
  send_to_char(" specifiers:\n\r",ch);
  send_to_char(" keyword <list of names>\n\r",ch);
  send_to_char(" vnum <exit leading to room vnum>\n\r",ch);
  send_to_char(" flag <exit flag>\n\r",ch);
  send_to_char(" key <key vnum>\n\r",ch);
  return;
}
void do_exmatch(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  ROOM_INDEX_DATA *pRoomIndex;
  int nMatch;
  bool found;

  char arg[MAX_INPUT_LENGTH];  
  char arg1[MAX_INPUT_LENGTH];  
  char buf[MAX_STRING_LENGTH];  

  /* Criteria vars */
  char keyword[MAX_INPUT_LENGTH] = "";  
  int Rvnum = NO_FLAG;
  long flag=NO_FLAG;
  long key = NO_FLAG;

  if (!argument[0]) { 
    exmatch_syntax(ch);
    return;
  }
  
  argument=one_argument(argument, arg);
  while (*arg) {
    if (!str_cmp(arg,"keyword")) {
      argument=one_argument(argument, arg1);
      strcpy(keyword,arg1);
    }
    else if (!str_cmp(arg,"flag")) {
      argument=one_argument(argument, arg1);
      flag = flag_value(exit_flags,arg1);
      if (flag == NO_FLAG) {
	send_to_charf(ch,"Wrong exit flags: %s\n\r",arg1);
	return;
      }        
    }    
    else if (!str_cmp(arg,"vnum")) {
      argument=one_argument(argument, arg1);
      Rvnum = atoi(arg1);
      if ( Rvnum <= 0 || get_room_index(Rvnum) == NULL ) {
	send_to_charf(ch,"Invalid room vnum: %s(%d)\n\r",arg1,Rvnum);
	return;
      }
    }
    else if (!str_cmp(arg,"key")) {
      argument=one_argument(argument, arg1);
      key = atoi(arg1);
      if ( !( key == -1 || get_obj_index(key) != NULL ) ) {
	send_to_charf(ch,"Invalid key vnum: %s(%ld)\n\r",arg1,key);
	return;
      }
    }
    else {
      send_to_charf(ch,"Specifier %s unknown\n\r",arg);
      exmatch_syntax(ch);
      return;
    }
    argument=one_argument(argument, arg);
  }
  
  buffer=new_buf();
  found=FALSE;

  nMatch=0;
	
  for(int vnum=0;nMatch<top_room;vnum++) {
    if ( (pRoomIndex=get_room_index(vnum)) != NULL) {
      nMatch++;
      for ( int i = 0; i < MAX_DIR; i++ ) {
	EXIT_DATA *pExit = pRoomIndex->exit[i];
	if ( pExit == NULL )
	  continue;
	if ( 
	    (is_name(keyword,pExit->keyword) || !*keyword )
	    && (IS_SET(pExit->exit_info,flag ) || flag == NO_FLAG )
	    && ( Rvnum == pExit->u1.to_room->vnum || Rvnum == NO_FLAG )
	    && ( key == pExit->key || key == NO_FLAG )
	    ) {
	  found=TRUE;
	  sprintf(buf, "[%5d] %s\n\r",pRoomIndex->vnum, pRoomIndex->name);
	  add_buf(buffer,buf);
	}
      }
    }
  }
  if (!found)
    send_to_char("No exits matching condition.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
}


void amatch_syntax(CHAR_DATA *ch) {
  send_to_char("Syntax: amatch <specifier> '<argument>' [<specifier> 'arg'] ...\n\r",ch);
  send_to_char(" specifiers:\n\r",ch);
  send_to_char(" keyword <list of names>\n\r",ch);
  send_to_char(" type <ability type>\n\r",ch);
  send_to_char(" class <class name>\n\r",ch);
  send_to_char(" target <target name>\n\r",ch);
  send_to_char(" position <min position>\n\r",ch);
  send_to_char(" cost <min cost> [<max cost>]\n\r",ch);
  send_to_char(" beats <min beats> [<max beats>]\n\r",ch);
  send_to_char(" mob <mob use flag>\n\r",ch);
  send_to_char(" dispellable YES/NO\n\r",ch);
  send_to_char(" wait <min time> [<max time>]\n\r",ch);
  send_to_char(" craftable YES/NO\n\r",ch);
  send_to_char(" update YES/NO\n\r",ch);
  send_to_char(" wearoff YES/NO\n\r",ch);
  send_to_char(" script YES/NO\n\r",ch);
  return;
}
void do_amatch(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  bool found;

  char arg[MAX_INPUT_LENGTH];  
  char arg1[MAX_INPUT_LENGTH];  
  char arg2[MAX_INPUT_LENGTH];  
  char buf[MAX_STRING_LENGTH];  

  /* Criteria vars */
  char keyword[MAX_INPUT_LENGTH] = "";  
  int type = NO_FLAG, target = NO_FLAG, position = NO_FLAG, mob = NO_FLAG;
  int mcost = 0, Mcost = 999, mbeats = 0, Mbeats = 999, mwait = 0, Mwait = 999;
  long classes = NO_FLAG;
  int dispellable = -1, craftable = -1, update = -1, wearoff = -1, script = -1;

  if (!argument[0]) { 
    amatch_syntax(ch);
    return;
  }
  
  argument=one_argument(argument, arg);
  while (*arg) {
    if (!str_cmp(arg,"keyword")) {
      argument=one_argument(argument, arg1);
      strcpy(keyword,arg1);
    }
    else if (!str_cmp(arg,"type")) {
      argument=one_argument(argument, arg1);
      type = flag_value(ability_type_flags,arg1);
      if (type == NO_FLAG) {
	send_to_charf(ch,"Wrong ability type flags: %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"target")) {
      argument=one_argument(argument, arg1);
      target = flag_value(target_flags,arg1);
      if (target == NO_FLAG) {
	send_to_charf(ch,"Wrong target flags: %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"position")) {
      argument=one_argument(argument, arg1);
      position = flag_value(position_flags,arg1);
      if (position == NO_FLAG) {
	send_to_charf(ch,"Wrong position flags: %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"mob")) {
      argument=one_argument(argument, arg1);
      mob = flag_value(mob_use_flags,arg1);
      if (mob == NO_FLAG) {
	send_to_charf(ch,"Wrong mob use flags: %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"class")) {
      argument=one_argument(argument, arg1);
      classes = flag_value(classes_flags,arg1);
      if (classes == NO_FLAG) {
	send_to_charf(ch,"Wrong classes flags: %s\n\r",arg1);
	return;
      }        
    }
    else if (!str_cmp(arg,"cost")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mcost = atoi(arg1);
      if (*arg2)
	Mcost = atoi(arg2);
      else
	Mcost = mcost;

      if ( mcost > Mcost ) {
	send_to_char("Wrong cost: min cost is higher than max cost.\n\r", ch );
	return;
      }
    }
    else if (!str_cmp(arg,"beats")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mbeats = atoi(arg1);
      if (*arg2)
	Mbeats = atoi(arg2);
      else
	Mbeats = mbeats;

      if ( mbeats > Mbeats ) {
	send_to_char("Wrong beats: min beats is higher than max beats.\n\r", ch );
	return;
      }
    }
    else if (!str_cmp(arg,"wait")) {
      argument=one_argument(argument, arg1);
      if (!is_number(arg1)) {
	send_to_charf(ch,"Number expected instead of %s\n\r",arg1);
	return;
      }
      argument=one_optional_number_argument(argument, arg2);

      mwait = atoi(arg1);
      if (*arg2)
	Mwait = atoi(arg2);
      else
	Mwait = mwait;

      if ( mwait > Mwait ) {
	send_to_char("Wrong wait: min wait is higher than max wait.\n\r", ch );
	return;
      }
    }
    else if (!str_cmp(arg,"dispellable")) {
      argument=one_argument(argument, arg1);
      if ( arg1[0] != '\0' && !str_cmp( arg1, "no" ) )
	dispellable = FALSE;
      else if ( arg1[0] != '\0' && !str_cmp( arg1, "yes" ) )
	dispellable = TRUE;
    }
    else if (!str_cmp(arg,"craftable")) {
      argument=one_argument(argument, arg1);
      if ( arg1[0] != '\0' && !str_cmp( arg1, "no" ) )
	craftable = FALSE;
      else if ( arg1[0] != '\0' && !str_cmp( arg1, "yes" ) )
	craftable = TRUE;
    }
    else if (!str_cmp(arg,"update")) {
      argument=one_argument(argument, arg1);
      if ( arg1[0] != '\0' && !str_cmp( arg1, "no" ) )
	update = FALSE;
      else if ( arg1[0] != '\0' && !str_cmp( arg1, "yes" ) )
	update = TRUE;
    }
    else if (!str_cmp(arg,"wearoff")) {
      argument=one_argument(argument, arg1);
      if ( arg1[0] != '\0' && !str_cmp( arg1, "no" ) )
	wearoff = FALSE;
      else if ( arg1[0] != '\0' && !str_cmp( arg1, "yes" ) )
	wearoff = TRUE;
    }
    else if (!str_cmp(arg,"script")) {
      argument=one_argument(argument, arg1);
      if ( arg1[0] != '\0' && !str_cmp( arg1, "no" ) )
	script = FALSE;
      else if ( arg1[0] != '\0' && !str_cmp( arg1, "yes" ) )
	script = TRUE;
    }
    else {
      send_to_charf(ch,"Specifier %s unknown\n\r",arg);
      amatch_syntax(ch);
      return;
    }
    argument=one_argument(argument, arg);
  }
  
  buffer=new_buf();
  found=FALSE;

  for(int sn=0;sn<MAX_ABILITY;sn++) {
    ability_type *ab = &(ability_table[sn]);
    bool findClasses = TRUE;
    if ( classes != NO_FLAG ) {
      int res = 0;
      for (int i=0;i<MAX_CLASS;i++) {
	if ( ( (1<<i) & classes) && (ability_table[sn].ability_level[i] > 0)) {
	  res = res ? UMIN(res,ability_table[sn].ability_level[i]):
	    ability_table[sn].ability_level[i] ;
	}
      }
      if ( res == 0 )
	findClasses = FALSE;
    }
    if ( findClasses
	&& (is_name( keyword, ab->name) || !*keyword )
	&& ( ab->type == type || type == NO_FLAG )
	&& ( ab->target == target || target == NO_FLAG )
	&& ( ab->minimum_position == position || position == NO_FLAG )
	&& ( ab->mob_use == mob || mob == NO_FLAG )
	&& ( ab->min_cost >= mcost && ab->min_cost <= Mcost )
	&& ( ab->beats >= mbeats && ab->beats <= Mbeats )
	&& ( ab->to_wait >= mwait && ab->to_wait <= Mwait )
	&& ( ab->dispellable == dispellable || dispellable == -1 )
	&& ( ab->craftable == craftable || craftable == -1 )
	&& ( update == -1 || ( update == TRUE && ab->update_fun != NULL ) || ( update == FALSE && ab->update_fun == NULL ) )
	&& ( wearoff == -1 || ( wearoff == TRUE && ab->wearoff_fun != NULL ) || ( wearoff == FALSE && ab->wearoff_fun == NULL ) )
	&& ( ab->scriptAbility == script || script == -1 )
	) {
      found=TRUE;
      sprintf(buf, "[%5d] %s\n\r", sn, ab->name);
      add_buf(buffer,buf);
    }
  }
  if (!found)
    send_to_char("No abilities matching condition.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
}

bool is_item_buggy( OBJ_INDEX_DATA *obj ) {
  if ( obj->vnum <= 100 ) return FALSE;

  if ( obj->short_descr == NULL || obj->name == NULL || obj->description == NULL ) 
    return TRUE;

  if ( obj->level == 0 )
    {
      /*
	if ( obj->vnum > 100 )
	obj->level = 1;
      */
      return TRUE;
    }

  if ( obj->wear_flags == 0 &&  // Jukebox removed by SinaC 2003
       !( obj->item_type == ITEM_FURNITURE /*|| obj->item_type == ITEM_JUKEBOX*/ ||
	  obj->item_type == ITEM_FOUNTAIN || obj->item_type == ITEM_CONTAINER ||
	  obj->item_type == ITEM_TRASH || obj->item_type == ITEM_PORTAL ) ) return TRUE;
  
  /*
    log_string("ITEM_BUGGY");
    sprintf(log_buf,"name: %s  type: %s  wear: %s", obj->name, item_name(obj->item_type), wear_bit_name( obj->wear_flags));
    log_string(log_buf);
  */

  switch( obj->item_type )
    {
      // CAN'T have wear_flags
      // Added by SinaC 2003
    case ITEM_SADDLE:
    case ITEM_ROPE:
      // Added by SinaC 2001 for levers
      //case ITEM_LEVER:   Removed by SinaC 2003
      //case ITEM_JUKEBOX :  removed by SinaC 2003
    case ITEM_FOUNTAIN :
      if ( obj->wear_flags != 0 ) return TRUE;
      break;
      // TAKE or TAKE | HOLD or nothing
    case ITEM_PORTAL :
      if ( !( obj->wear_flags == ITEM_TAKE 
	      || obj->wear_flags == ( ITEM_TAKE | ITEM_HOLD )
	      || obj->wear_flags == 0 ) ) return TRUE;
      break;
      // TAKE  or  TAKE | HOLD
    case ITEM_LIGHT :
    case ITEM_SCROLL :
    case ITEM_POTION :
    case ITEM_KEY :
    case ITEM_FOOD :
    case ITEM_MAP :
    case ITEM_PILL :
      // Added by SinaC 2003
    case ITEM_TEMPLATE:
      if ( !( obj->wear_flags == ITEM_TAKE || 
	      obj->wear_flags == ( ITEM_TAKE | ITEM_HOLD ) ) ) return TRUE;
      break;
      // TAKE | HOLD
      // added by SinaC 2000 for songs for bard
    case ITEM_INSTRUMENT :
    case ITEM_WARP_STONE :
    case ITEM_WAND :
    case ITEM_STAFF :
      if ( obj->wear_flags != ( ITEM_TAKE | ITEM_HOLD ) ) return TRUE;
      break;
      // TAKE | WIELD
    case ITEM_WEAPON :
      if ( obj->wear_flags != ( ITEM_TAKE | ITEM_WIELD ) ) return TRUE;
      break;
      // max 2 wear_flags ITEM_TAKE + another one != WIELD
    case ITEM_DRINK_CON :
    case ITEM_TREASURE :
    case ITEM_ARMOR :
    case ITEM_BOAT :
    case ITEM_GEM :
    case ITEM_JEWELRY :
    case ITEM_CLOTHING :
    case ITEM_FURNITURE :
    case ITEM_TRASH :
    case ITEM_CONTAINER :
      if ( IS_SET( obj->wear_flags, ITEM_WIELD )) return TRUE;
      break;
      // TAKE
    case ITEM_MONEY :
    case ITEM_CORPSE_NPC :
    case ITEM_CORPSE_PC :
      // have replaced ITEM_ROOM_KEY with ITEM_COMPONENT SinaC 2000
      //    case ITEM_ROOM_KEY :
    case ITEM_COMPONENT :
      if ( obj->wear_flags != ITEM_TAKE ) return TRUE;
      break;
      // ??
      // SinaC 2000 : have replaced ITEM_PROTECT with ITEM_THROWING
      //    case ITEM_PROTECT :
      break;
    }

  return FALSE;
}
