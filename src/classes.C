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
#else
#include <sys/types.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "tables.h"
#include "classes.h"
#include "interp.h"


// Added by SinaC 2001
#include "act_wiz.h"
#include "comm.h"
#include "handler.h"
#include "db.h"
#include "act_comm.h"
#include "fight.h"
#include "lookup.h"
#include "olc_value.h"
#include "names.h"
#include "gsn.h"
//#include "olc.h"
#include "wiznet.h"
#include "ability.h"
#include "group.h"
#include "prereqs.h"
#include "bit.h"
#include "clan.h"
#include "act_move.h"

/* Made by oxtal for multiclassing */

void do_multiclass(CHAR_DATA *ch, const char *argument) {
  int iClass;
  //  char arg[MAX_INPUT_LENGTH];

  ROOM_INDEX_DATA *pRoom;
  OBJ_DATA *obj;

  if (IS_NPC(ch)){
    send_to_char("Mobs can't multiclass!\n\r",ch);
    return;
  }

  if (IS_IMMORTAL(ch)) {
    send_to_char("Immortals are damned to be immortals.\n\r",ch);
    return;
  }

  //  argument = one_argument(argument,arg);

  if (argument[0] == 0) {
    send_to_char("You must provide a class name to add to your class list.\n\r",ch);
    return;
  }

  iClass = class_lookup(argument, TRUE );
  if (iClass == -1) {
    send_to_char("You must provide an existing class name to add to your class list.\n\r",ch);
    return;
  }

  if ((1<<iClass) & ch->bstat(classes)) {
    send_to_char("Choose a class that you don't already have.\n\r",ch);
    return;
  }

  // Added by SinaC 2001
  if ( !check_class_god( iClass, ch->pcdata->god ) ){
    send_to_charf(ch,
		  "%s doesn't allow that class.\n\r",
		  god_name(ch->pcdata->god));
    return;
  }
  if ( !check_class_race( ch, iClass ) ){
    send_to_char( "You can't choose that class because of your race.\n\r", ch );
    return;
  }
  // Added by SinaC 2003 for subclass system
  //  If trying to multiclass in a subclass without having the parent class
  //  Should be able to get only one sub-class for each parent-class
  //   Cannot be fire and water elementalist
  //   Cannot be enchanter and transmuter
  //   But can be assassin and fire elementalist
  if ( class_table[iClass].parent_class != iClass
       && !((1<<class_table[iClass].parent_class) & ch->bstat(classes))) {
    send_to_char("You can't choose this sub-class because you don't have the parent class.\n\r", ch );
    return;
  }
  // Added by SinaC 2003 to determine if a class can be picked during creation/multiclass
  if ( class_table[iClass].choosable != CLASS_CHOOSABLE_YES ) {
    send_to_char("This class cannot be picked.\n\r", ch );
    return;
  }

  // check wild-magic
  if ( ch->isWildMagic                                                    // can't get non-wild magic class
       && IS_SET( class_table[iClass].type, CLASS_MAGIC )                 //  if wild mage
       && !IS_SET( class_table[iClass].type, CLASS_WILDABLE ) ) {
    send_to_charf(ch,"This class cannot be picked by a wild-mage.\n\r");
    return;
  }

  // kill pet and charmies
  die_follower( ch );

  /* Test if fighting ? */
  stop_fighting( ch, TRUE );

  /* Go to donation room */

  pRoom = get_donation_room( ch );
  //pRoom = get_room_index( ROOM_VNUM_DONATION );
  if (pRoom==NULL) {
    bug("do_multiclass: donation room not found for player [%s] clan [%s] hometown [%s]!",
	NAME(ch), get_clan_table(ch->clan)->name,
	(IS_NPC(ch)||ch->pcdata->hometown<0)?"none":hometown_table[ch->pcdata->hometown].name);
    //bug("multiclass: donation room not found %d!",ROOM_VNUM_DONATION);
    return;
  }

  if (ch->in_room != pRoom) {
    act("$n disapears!",ch,NULL,NULL,TO_ROOM);
    char_from_room( ch );
    char_to_room( ch, pRoom );
    act( "$n appears in the donation room.", ch, NULL, NULL, TO_ROOM );
  }

  ch->level = 1;
  ch->bstat(classes) |= 1<<iClass;
  ch->hit = 20;        /* I should place constants instead of "hard code" */
  ch->bstat(max_hit) = 20;
  ch->mana = 100;
  ch->bstat(max_mana) = 100;
  // Added by SinaC 2001 for mental user
  ch->psp = 100;
  ch->bstat(max_psp) = 100;
  ch->move = 100;
  ch->bstat(max_move) = 100;
  ch->wimpy = 0;

  /* ch->pcdata->points = ? creation points*/
  ch->exp = exp_per_level(ch,ch->pcdata->points);

  /* Train - Pra ?*/

  /* Gain base group*/
  group_add(ch,class_table[iClass].base_group,FALSE);

  /*group_add(ch,class_table[iClass].default_group,TRUE);*/
  /* adding this would raise creation points too much*/

  /*group_add(ch,class_table[iClass].default_group,FALSE);*/
  /* adding this would be too easy for the player */

  /* gold & silver ?*/

  /* Dealing with equipment */
  // FIXME: item STAY_DEATH or owned equipement must be dropped ?
  while ( ch->carrying != NULL ) {
    /* Remove the obj if it is worn */
    obj = ch->carrying;
    if (obj->wear_loc != WEAR_NONE)
      unequip_char( ch, obj );

    obj_from_char( obj );
    obj_to_room( obj, pRoom );
    SET_OBJ_STAT( obj, ITEM_DONATED);
  }

  // Added by SinaC 2001
  recompute(ch); 
  // Added by SinaC 2001
  recomproom(pRoom);

  do_outfit(ch,"");

  send_to_char("You are now mortal again...\n\r",ch);
  send_to_char("You see all your possesions lying on the ground...\n\r",ch);
  send_to_char("Probably few things are still usable, you'd better\n\r"
	       "leave them here.\n\r",ch);

  char buf[MAX_INPUT_LENGTH];
  sprintf( buf, "$N has multiclassed in %s.", class_table[iClass].name );
  wiznet(buf, ch, NULL, WIZ_MULTICLASS, 0, 0 );
}

int class_hpmax(long c) {
  int res = 0;
  long i = 0;

  for (i=0;i<MAX_CLASS;i++)
    if ((1LL<<i) & c)
      res = UMAX(res,class_table[i].hp_max);
  return res;

}


int class_hpmin(long c) {
  int res = 0;
  long i = 0;

  for (i=0;i<MAX_CLASS;i++)
    if ((1<<i) & c)
      res = UMAX(res,class_table[i].hp_min);
  return res;
}

/* Removed by SinaC 2001
*int class_mult(CHAR_DATA *ch)
*{
* int i,n=0,res=0;
*
* int r = ch->cstat(race);
* long c = ch->cstat(classes);
*
*  for (i=0;i<MAX_CLASS;i++) {
*    if ( (1<<i) & c) {
*      n++;
*      res+= pc_race_table[r].class_mult[i];
*    }
*  }
*  return res/n;
*}
*/

int class_hasattr(CHAR_DATA *ch,int attr) {
 int i;
 long c;

 c = ch->cstat(classes);

 for (i=0;i<MAX_CLASS;i++) {
   if( ( (1LL<<i) & c) && class_table[i].attr_prime == attr) {
     return 1;
   }
 }
 return 0;
}

int class_firstclass(long c) {
  int i = 0;
  long bit = 1;
  while ( (bit & c) == 0) {
    i++;
    bit = bit << 1;
  }
  return i;
}

int class_fMana(long c) {
  int i;
  for (i=0;i<MAX_CLASS;i++) {
    //if ( (class_table[i].type == CLASS_MAGIC) && ((1LL<<i)&c)) {
    if ( IS_SET( class_table[i].type, CLASS_MAGIC ) && ((1LL<<i)&c) ) {
      return 1;
    }
  }
  return 0;
}

// Added by SinaC 2001 for mental user
int class_fPsp(long c) {
  int i;
  for (i=0;i<MAX_CLASS;i++) {
    //if( (class_table[i].type == CLASS_MENTAL ) && ((1LL<<i)&c)) {
    if ( IS_SET( class_table[i].type, CLASS_MENTAL ) && ((1LL<<i)&c) ) {
      return 1;
    }
  }
  return 0;
}


int class_ismulti(long c) {
  int res=0;
  int i;
  for (i=0;i<MAX_CLASS;i++) {
    if (c & 1) {
      res++;
    }
    c = c>>1;
  }
  return res > 1;
}

const char * class_name(long c) {
  // Added by SinaC 2000
  if ( c == 0 )
    return "none";

  if( class_ismulti(c))
    return "Multiclass";
  else
    return class_table[class_firstclass(c)].name;
}

const char * class_whoname(long c) {
  if ( c == 0 )
    return "---";

  if ( class_ismulti(c) )
    return "Mlt";
  else
    return class_table[class_firstclass(c)].who_name;
}

int class_abilitylevel(CHAR_DATA *ch,int sn) {
  int res = 0;
  int i = 0;
  long c = ch->cstat(classes);

  for (i=0;i<MAX_CLASS;i++) {
    if ( ( (1<<i) & c) && (ability_table[sn].ability_level[i] > 0)) {
      res = res ? UMIN(res,ability_table[sn].ability_level[i]):
	ability_table[sn].ability_level[i] ;
    }
  }

  // Added by SinaC 2001, if the spell has been learned at a level > 0
  //  means not a creation
  if ( !IS_NPC(ch) && ch->pcdata->ability_info[sn].level > 0  ) {
    res = UMIN( ch->pcdata->ability_info[sn].level, res );
  }

  return res;
}

int class_thac0_32(long c) {
  int res = 500;
  int i = 0;

  for (i=0;i<MAX_CLASS;i++) {
    if ((1LL<<i) & c) {
      res = UMIN(res,class_table[i].thac0_32);
    }
  }
  return res;
}


int class_thac0_00(long c) {
  int res = 500;
  int i = 0;

  for (i=0;i<MAX_CLASS;i++) {
    if ((1LL<<i) & c) {
      res = UMIN(res,class_table[i].thac0_00);
    }
  }
  return res;
}

int class_abilityadept(long c) {
  int res = 0;
  int i = 0;

  for (i=0;i<MAX_CLASS;i++) {
    if ((1<<i) & c) {
      res = UMAX(res,class_table[i].ability_adept);
    }
  }
  return res;
}


int class_abilityrating(CHAR_DATA *ch,int sn, int lvl ) {
  int res = 0;
  int i = 0;

  long c = ch->cstat(classes);

  for (i=0;i<MAX_CLASS;i++) {
    // Modified by SinaC 2003,   added  abs
    if ( ((1<<i) & c) && (abs(ability_table[sn].rating[i]) > 0)) {
      res = res ? UMIN(res,abs(ability_table[sn].rating[i])) : abs(ability_table[sn].rating[i]);
    }
  }

  // Added by SinaC 2001, if the spell has been learned at a level <> 0
  //  we return that level, only if the previous result was 0 (class can't learn that ability)
  if ( !IS_NPC(ch) && ch->pcdata->ability_info[sn].level > 0 && res == 0 ) {
    //res = ch->pcdata->ability_info[sn].level; // 1 before  modified by SinaC 2003
    res = 10; // difficulty for learned spell is 10, SinaC 2003
  }

  // Modified by SinaC 2001 for ability level, level but no prereq
  if ( ability_table[sn].nb_casting_level > 0 
       && ability_table[sn].prereqs == NULL ) {
    return UMAX( res, DEFAULT_PREREQ_COST(lvl) );
  }
  // Modified by SinaC 2000 for ability level, level and prereq
  if ( ability_table[sn].nb_casting_level > 0 
       && ability_table[sn].prereqs != NULL 
       && res > 0 ){
    //if ( ability_table[sn].prereqs[lvl] != NULL )
      return UMAX(ability_table[sn].prereqs[lvl].cost, res );
    //    for ( int j = 0; j < ability_table[sn].nb_casting_level; j++ ){
    //       if ( skill_table[sn].prereqs[j].casting_level == lvl )
    //	return UMAX(ability_table[sn].prereqs[j].cost, res );
    //}
  }
  return res;
}

int class_grouprating( const long c, const int sn) {
  int res = 0;
  int i = 0;

  //long c = ch->cstat(classes);

  for (i=0;i<MAX_CLASS;i++) {
    if ( ((1LL<<i) & c) && (group_table[sn].rating[i] > 0)) {
      res = res ? UMIN(res,group_table[sn].rating[i]) : group_table[sn].rating[i];
    }
  }
  return res;
}

// Added by SinaC 2000
int class_count(long c) {
  int res=0;
  int i;
  for (i=0;i<MAX_CLASS;i++) {
    if (c & 1) {
      res++;
    }
    c = c>>1;
  }
  return res;
}

// if both 'other' == -1  return ( max( highest ), -1 )
// if both 'other' != -1  return ( max( highest ), max( other ) )
// else if c1.other == -1
//        if c1.highest >= c2.highest  return ( c1.highest, -1 )
//        else                         return ( c2.highest, max( c1.highest, c2.other ) )
// else if c2.other == -1
//        if c2.highest >= c1.highest  return ( c2.highest, -1 )
//        else                         return ( c1.highest, max( c2.highest, c1.other ) )
casting_rule_type compose_casting_rule( const casting_rule_type c1, const casting_rule_type c2 ) {
  casting_rule_type c3;
  if ( c1.other == -1 && c2.other == -1 ) {
    c3.highest = UMAX( c1.highest, c2.highest );
    c3.other = -1;
    return c3;
  }
  if ( c1.other != -1 && c2.other != -1 ) {
    c3.highest = UMAX( c1.highest, c2.highest );
    c3.other = UMAX( c1.other, c2.other );
    return c3;
  }
  if ( c1.other == -1 ) {
    //c3.highest = max( c1.highest, c2.highest );
    if ( c1.highest >= c2.highest ) {
      c3.highest = c1.highest;
      c3.other = -1;
    }
    else {
      c3.highest = c2.highest;
      c3.other = UMAX( c1.highest, c2.other );
    }
    return c3;
  }
  // we are sure following test is true:  if ( c2.other == -1 ) {
  if ( c2.highest >= c1.highest ) {
    c3.highest = c2.highest;
    c3.other = -1;
  }
  else {
    c3.highest = c1.highest;
    c3.other = UMAX( c2.highest, c1.other );
  }
  return c3;
}

// We need ch->classes and sn's ability type
casting_rule_type classes_casting_rule( CHAR_DATA *ch, const int sn ) {
  casting_rule_type rule = default_casting_rule;
  if ( IS_IMMORTAL(ch) )
    return max_casting_rule;
  long c = ch->bstat(classes);
  int sk_type;
  if ( sn < 0 ) { // trick to ask directly for an ability type
    sk_type = -sn;
    if ( sn >= ABILITY_TYPE_COUNT )
      return rule; // error
  }
  else
    sk_type = ability_table[sn].type;
  for ( int i = 0; i < MAX_CLASS; i++ ) {
    if (!( ( 1 << i ) & c )) // class i not in ch's classes
      continue;
    if ( IS_SET( class_table[i].type, CLASS_WILDABLE ) // if class is wildable,  SinaC 2003
	 && sk_type == TYPE_SPELL                      // and testing a spell
	 && ch->isWildMagic )                          // and player is wild magician
      rule = compose_casting_rule( wild_magic_casting_rule, rule ); // use wild magic casting rule
    else
      rule = compose_casting_rule( class_table[i].casting_rule[sk_type], rule );
  }
  return rule;
}
/*
int class_max_casting_rule( CHAR_DATA *ch ){
  int res=0;
  if ( IS_IMMORTAL(ch) )
    return CASTING_RULE_ALL_LVL5;

  long c = ch->bstat(classes);
  for (int i=0;i<MAX_CLASS;i++) {
    if (c & 1) {
      if ( res == 0 )
	res = class_table[i].max_casting_rule;
      else
	switch( res ) {
	case CASTING_RULE_SKILL_ALL_LVL5:
	  res = class_table[i].max_casting_rule;
	  break;
	case CASTING_RULE_ALL_LVL1:
	  res = class_table[i].max_casting_rule;
	  break;
	case CASTING_RULE_ALL_LVL3: 
	  res = class_table[i].max_casting_rule;
	  break;
	case CASTING_RULE_ALL_LVL4: 
	  if ( class_table[i].max_casting_rule == CASTING_RULE_LVL5_OTHER_LVL3 
	       || class_table[i].max_casting_rule == CASTING_RULE_LVL5_OTHER_LVL4 )
	    res = CASTING_RULE_LVL5_OTHER_LVL4;
	  break;
	case CASTING_RULE_LVL5_OTHER_LVL3:
	  if ( class_table[i].max_casting_rule == CASTING_RULE_ALL_LVL4 
	       || class_table[i].max_casting_rule == CASTING_RULE_LVL5_OTHER_LVL4 )
	    res = CASTING_RULE_LVL5_OTHER_LVL4; 
	  break;
	case CASTING_RULE_LVL5_OTHER_LVL4:
	  res = CASTING_RULE_LVL5_OTHER_LVL4;
	  break;
	case CASTING_RULE_LVL4_OTHER_LVL3:
	  if ( class_table[i].max_casting_rule == CASTING_RULE_ALL_LVL4 )
	    res = CASTING_RULE_ALL_LVL4;
	  else if ( class_table[i].max_casting_rule == CASTING_RULE_LVL5_OTHER_LVL3 )
	    res = CASTING_RULE_LVL5_OTHER_LVL3;
	  else if ( class_table[i].max_casting_rule == CASTING_RULE_LVL5_OTHER_LVL4 )
	    res = CASTING_RULE_LVL5_OTHER_LVL4;
	  else
	    res = CASTING_RULE_LVL4_OTHER_LVL3;
	  break;
	}
    }
    c = c>>1;
  }
  return res;
}
*/

// Added by SinaC 2003
int god_grouprating( CHAR_DATA *ch, int gn ) {
  if ( IS_NPC(ch) )
    return -1;

  int god = ch->pcdata->god;
  return group_table[gn].god_rate[god];
}

// When try to gain an ability, ability with a negative rating doesn't have to be considered
int class_gainabilityrating(CHAR_DATA *ch, int sn, int lvl ) {
  int res = 0;
  int i = 0;

  long c = ch->cstat(classes);

  for (i=0;i<MAX_CLASS;i++) {
    if ( ((1<<i) & c) && (ability_table[sn].rating[i] > 0 )) {
      res = res ? UMIN(res,ability_table[sn].rating[i]) : ability_table[sn].rating[i];
    }
  }

  // Added by SinaC 2001, if the spell has been learned at a level <> 0
  //  we return that level, only if the previous result was 0 (class can't learn that ability)
  if ( !IS_NPC(ch) && ch->pcdata->ability_info[sn].level > 0 && res == 0 ) {
    res = ch->pcdata->ability_info[sn].level; // 1 before  modified by SinaC 2003
  }

  // Modified by SinaC 2001 for ability level, level but no prereq
  if ( ability_table[sn].nb_casting_level > 0 
       && ability_table[sn].prereqs == NULL
       & res > 0 ) {
    return UMAX( res, DEFAULT_PREREQ_COST(lvl) );
  }
  // Modified by SinaC 2000 for ability level, level and prereq
  if ( ability_table[sn].nb_casting_level > 0 
       && ability_table[sn].prereqs != NULL 
       && res > 0 ) {
    //if ( ability_table[sn].prereqs[j] != NULL )
      return UMAX( ability_table[sn].prereqs[lvl].cost, res );
      //for ( int j = 0; j < ability_table[sn].nb_casting_level; j++ ){
      //if ( ability_table[sn].prereqs[j].casting_level == lvl )
      //return UMAX( ability_table[sn].prereqs[j].cost, res );
    //}
  }
  return res;
}

// Return if a group is available for every gods
bool group_available_every_gods( int gn ) {
  for ( int i = 0; i < MAX_GODS; i++ )
    if ( group_table[gn].god_rate[i] <= 0 )
      return FALSE;
  return TRUE;
}


// Added by SinaC 2003 for sub-classes system
void specialize_syntax( CHAR_DATA *ch ) {
  send_to_char("Syntax: specialize list                list available sub-classes.\n\r", ch );
  send_to_char("        specialize weapon              list available weapons.\n\r", ch );
  send_to_char("        specialize <class name>        specialize in that class.\n\r", ch );
  send_to_char("        specialize <weapon type name>  specialize in that weapon type.\n\r", ch );
}
void do_specialize( CHAR_DATA *ch, const char *argument ) {
  int iClass, iWeapon;
  //  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)){
    send_to_char("Mobs can't specialize!\n\r",ch);
    return;
  }

  // find a practicer
  CHAR_DATA *practicer;
  for ( practicer = ch->in_room->people; practicer != NULL; practicer = practicer->next_in_room)
    if (IS_NPC(practicer) && IS_SET(practicer->act,ACT_PRACTICE))
      break;
  
  if (practicer == NULL || !can_see(ch,practicer)) {
    send_to_char("You can't do that here.\n\r",ch);
    return;
  }

  //  argument = one_argument(argument,arg);

  // No arg
  if (argument[0] == 0) {
    Value *v = get_extra_field( ch, "specialized_weapon" );
    if ( v != NULL ) { // extra field found
      int specialized = v->asInt();
      send_to_charf(ch, "You are specialized in %s.\n\r", flag_string(weapon_class,specialized));
    }
    else
      specialize_syntax( ch );
  }
  // List available classes
  else if ( !str_cmp( argument, "list" ) ) {
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    buf[0] = '\0';
    for ( int i = 0; i < MAX_CLASS; i++ ) {
      if ( class_table[i].name == NULL ) continue;
      if ( (1 << i) & ch->bstat(classes) ) continue;                     // already has that class
      if ( class_table[i].parent_class == i ) continue;                  // not a sub-class
      if ( !check_class_god( i, ch->pcdata->god ) ) continue;            // god doesn't like that class
      if ( !check_class_race( ch, i ) ) continue;                        // race can't be that class
      if (class_table[i].parent_class != i                               // doesn't have parent class
	  && !((1<<class_table[i].parent_class) & ch->bstat(classes)) ) continue;
      if ( class_table[i].choosable == CLASS_CHOOSABLE_NEVER ) continue; // can't be picked
      if ( ch->isWildMagic                                               // can't get non-wild magic
	   && IS_SET( class_table[i].type, CLASS_MAGIC )                 //  class if wild mage
	   && !IS_SET( class_table[i].type, CLASS_WILDABLE ) ) continue;

      sprintf(buf2,"  %s\n\r", class_table[i].name );
      strcat(buf,buf2);
      found = TRUE;
    }
    
    if (found) {
      send_to_char("Available classes to specialize in:\n\r",ch);
      send_to_char(buf,ch);
    }
    else
      send_to_char("You can't specialize in a class.\n\r", ch );
  }
  // List weapon available weapon
  else if ( !str_cmp( argument, "weapon" ) ) {
    if ( get_ability( ch, gsn_specialization ) == 0 ) {
      send_to_char("You don't know how to specialize in a weapon type.\n\r", ch );
      return;
    }
    bool found = FALSE;
    char buf[MAX_STRING_LENGTH];
    char buf2[MAX_STRING_LENGTH];
    buf[0] = '\0';
    for (int type = 0; weapon_table[type].name != NULL; type++)
      if ( weapon_table[type].gsn != NULL 
	   && get_ability( ch, *(weapon_table[type].gsn) ) == 100 ) {
	found = TRUE;
	sprintf(buf2,"  %s\n\r", weapon_table[type].name );
	strcat(buf,buf2);
      }
    if ( found ) {
      send_to_char("Available weapons to specialize in:\n\r",ch);
      send_to_char(buf,ch);
    }
    else
      send_to_char("You are not proficient enough in any weapon type.\n\r", ch );
  }
  // Weapon
  else if ( ( iWeapon = weapon_lookup(argument) ) >= 0 ) {
    if ( get_ability( ch, gsn_specialization ) == 0 ) {
      send_to_char("You don't know how to specialize in a weapon type.\n\r", ch );
      return;
    }
    if ( weapon_table[iWeapon].gsn
	 && get_ability( ch, *(weapon_table[iWeapon].gsn) ) != 100 ) {
      send_to_charf(ch,"You are not proficient enough in %s.\n\r", weapon_table[iWeapon].name );
      return;
    }
    Value *sp = get_extra_field( ch, "specialized_weapon" );
    if ( sp != NULL ) {
      int w = sp->asInt();
      send_to_charf(ch,"You already specialized in %s.\n\r", w>=0?weapon_table[w].name:"Unknown" );
      return;
    }
    // Add an extra field storing in which weapon the player is specialized
    Value v = add_extra_field( ch, "specialized_weapon" );
    v.setValue( weapon_table[iWeapon].type );
    send_to_charf(ch,"You are now specialized in %s.\n\r", weapon_table[iWeapon].name );
  }
  // Class
  else if ( ( iClass = class_lookup(argument, TRUE) ) >= 0 ) {
    if ((1<<iClass) & ch->bstat(classes)) {
      send_to_char("Choose a class that you don't already have.\n\r",ch);
      return;
    }
    if ( class_table[iClass].parent_class == iClass ) {
      send_to_char("This is not a sub-class.\n\r", ch );
      return;
    }
    if ( !check_class_god( iClass, ch->pcdata->god ) ){
      send_to_charf(ch,
		    "%s doesn't allow that class.\n\r",
		    god_name(ch->pcdata->god));
      return;
    }
    if ( !check_class_race( ch, iClass ) ){
      send_to_char( "You can't choose that class because of your race.\n\r", ch );
      return;
    }
    if ( class_table[iClass].parent_class != iClass
	 && !((1<<class_table[iClass].parent_class) & ch->bstat(classes))) {
      send_to_char("You can't choose this sub-class because you don't have the parent class.\n\r", ch );
      return;
    }
    // Added by SinaC 2003 to determine if a class can be picked during creation/multiclass
    if ( class_table[iClass].choosable == CLASS_CHOOSABLE_NEVER ) {
      send_to_char("This class cannot be picked.\n\r", ch );
      return;
    }
    if ( ch->isWildMagic                                                 // can't get non-wild magic class
	 && IS_SET( class_table[iClass].type, CLASS_MAGIC )              //  if wild mage
	 && !IS_SET( class_table[iClass].type, CLASS_WILDABLE ) ) {
      send_to_charf(ch,"This class cannot be picked by a wild-mage.\n\r");
      return;
    }

    // Remove parent class
    REMOVE_BIT( ch->bstat(classes), 1<<class_table[iClass].parent_class );
    // Add sub-class
    SET_BIT( ch->bstat(classes), 1<<iClass );
    
    send_to_charf(ch,"You specialize yourself as %s.\n\r", class_table[iClass].name );
    recompute(ch);
  }
  else
    specialize_syntax(ch);
}

// check if classes (and classes' parent) contains guild
bool check_guild_room( int guild, long classes ) {
  for ( int i = 0; i < MAX_CLASS; i++ ) {
    if ( ( 1 << i ) & classes ) { // class in classes
      if ( ( 1 << i ) & guild ) // class in guild
	return TRUE;
      if ( class_table[i].parent_class != i // class has a parent class
	   && check_guild_room( guild, 1 << class_table[i].parent_class ) ) // check parent
	return true;
    }
  }
  return FALSE;
}

// return if a class bitvector is wildable
bool isWildable( long classes ) {
  for ( int i = 0; i < MAX_CLASS; i++ )
    if ( ( ( 1 << i ) & classes ) && IS_SET( class_table[i].type, CLASS_WILDABLE ) )
      return TRUE;
  return FALSE;
}
