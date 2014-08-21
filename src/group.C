/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include "magic.h"
#include "recycle.h"
#include "interp.h"

// Added by SinaC 2001
#include "classes.h"
#include "act_info.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "update.h"
#include "skills.h"
#include "lookup.h"
#include "olc_value.h"
#include "const.h"
#include "act_comm.h"
#include "act_obj.h"
#include "group.h"


// Added by SinaC 2001
void groups_help(const char *argument, BUFFER *output ) {
  char buf[MAX_STRING_LENGTH];
  int gn,sn,col,
    tsn;

  col = 0;
 
  /* show the sub-members of a group */
  gn = group_lookup(argument);
  if (gn <= -1)
    return;
  
  sprintf(buf,"{WGroup name: {c%-20s{x\n\r",group_table[gn].name);
  add_buf(output,buf);
  
  for (sn = 0; sn < group_table[gn].spellnb; sn++) {
    tsn = group_lookup( group_table[gn].spells[sn] );
    if ( tsn > 0 ) {
      sprintf(buf," {y%-20s{x ",group_table[tsn].name);
    }
    else {
      tsn = ability_lookup(group_table[gn].spells[sn]);
      if ( tsn <= 0 ) continue;
      sprintf(buf," {y%-20s{x ",group_table[gn].spells[sn]);
    }
    add_buf(output,buf);
    if (++col % 3 == 0)
      add_buf(output,"\n\r");
  }
  if ( col % 3 != 0 )
    add_buf(output,"\n\r" );
}


/* shows all groups, or the sub-members of a group */
void do_groups(CHAR_DATA *ch, const char *argument) {
  char buf[100];
  int gn,sn,col,
    // Added by SinaC 2000
    tsn;
  bool found;

  if (IS_NPC(ch))
    return;

  col = 0;

  if (argument[0] == '\0') {   /* show known all groups */
    for (gn = 0; gn < MAX_GROUP; gn++) {
      // Removed by SinaC 2000
      //if (group_table[gn].name == NULL)
	//break;
      if (ch->pcdata->group_known[gn]) {
	// Modified by SinaC 2003
	//sprintf(buf,"{y%-20s{x ",group_table[gn].name);
	if ( group_table[gn].isSphere ) {
	  char buf2[MAX_STRING_LENGTH];
	  sprintf(buf2,"(%s)",group_table[gn].name);
	  sprintf(buf,"{y%-20s{x ",buf2);
	}
	else
	  sprintf(buf,"{y%-20s{x ",group_table[gn].name);
	send_to_char(buf,ch);
	if (++col % 3 == 0)
	  send_to_char("\n\r",ch);
      }
    }
    if ( col % 3 != 0 )
      send_to_char( "\n\r", ch );
    // Added by SinaC 2003
    send_to_char("{WGroups within () are god spheres.{w\n\r",ch);

    sprintf(buf,"{WCreation points: {c%d.{x\n\r",ch->pcdata->points);
    send_to_char(buf,ch);
    return;
  }
  
  if (!str_cmp(argument,"all")) {   /* show all groups */
    for (gn = 0; gn < MAX_GROUP; gn++) {
      // Removed by SinaC 2000
      //if (group_table[gn].name == NULL)
	//break;
	// Modified by SinaC 2003
	//sprintf(buf,"{y%-20s{x ",group_table[gn].name);
      if ( group_table[gn].isSphere ) {
	char buf2[MAX_STRING_LENGTH];
	sprintf(buf2,"(%s)",group_table[gn].name);
	sprintf(buf,"{y%-20s{x ",buf2);
      }
      else
	sprintf(buf,"{y%-20s{x ",group_table[gn].name);
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
    if ( col % 3 != 0 )
      send_to_char( "\n\r", ch );

    // Added by SinaC 2003
    send_to_char("{WGroups within () are god spheres.{w",ch);
    return;
  }
  
  
  /* show the sub-members of a group */
  gn = group_lookup(argument);
  if (gn <= -1) {
    send_to_char("No group of that name exist.\n\r",ch);
    send_to_char(
		 "Type 'groups all' or 'info all' for a full listing.\n\r",ch);
    return;
  }
  
  found = FALSE;
  
  if ( group_table[gn].isSphere )
    send_to_charf(ch,"{WSphere name: {c%-20s{x\n\r",group_table[gn].name);
  else
    send_to_charf(ch,"{WGroup name: {c%-20s{x\n\r",group_table[gn].name);
  
  // Added by SinaC 2000
  for (sn = 0; sn < /*MAX_IN_GROUP*/group_table[gn].spellnb; sn++) {
    // Modified by SinaC 2001
    tsn = group_lookup( group_table[gn].spells[sn] );
    if ( tsn > 0 ) {
      sprintf(buf," {y%-20s{x ",group_table[tsn].name);
    }
    else {
      // Removed by SinaC 2000
      //if (group_table[gn].spells[sn] == NULL)
      //break;
      // Added by SinaC 2000
      tsn = ability_lookup(group_table[gn].spells[sn]);
      if ( tsn <= 0 ) {
	bug("do_groups: Invalid skill %s in group %s", 
	    group_table[gn].spells[sn], group_table[gn].name );
	continue;
      }
      // Modified by SinaC 2003
      /*
      if ( class_gainskillrating( ch, tsn, ch->pcdata->ability_info[sn].casting_level ) > 0
	   // Added by SinaC 2001
	   //|| ( ch->desc->connected > CON_GET_PLAYING 
	   //     && ch->desc->connected < CON_GET_ALIGNMENT )
	   // Removed by SinaC 2001
	   //|| help_flag == TRUE )
	   )
	sprintf(buf," {y%-20s{x ",group_table[gn].spells[sn]);
      else {
	sprintf(buf,"{R*{y%-20s{x ",group_table[gn].spells[sn]);
	found = TRUE;
      }
      */
      sprintf(buf," {y%-20s{x ",group_table[gn].spells[sn]);
    }
    send_to_char(buf,ch);
    if (++col % 3 == 0)
      send_to_char("\n\r",ch);
  }
  if ( col % 3 != 0 )
    send_to_char( "\n\r", ch );
  // Added by SinaC 2000
  /*
  if ( found ) 
    send_to_char("{WYou can't get skills/spells/powers preceed by a '{R*{W'.{x\n\r",ch);
  */
}

/* returns a group index number given the name */
int group_lookup( const char *name ) {
  int gn;
 
  for ( gn = 0; gn < MAX_GROUP; gn++ ) {
    // Removed by SinaC 2000
    //if ( group_table[gn].name == NULL )
      //break;
    if ( LOWER(name[0]) == LOWER(group_table[gn].name[0])
	 &&   !str_prefix( name, group_table[gn].name ) )
      return gn;
    }
  
  return -1;
}

/* recursively adds a group given its number -- uses group_add */
void gn_add( CHAR_DATA *ch, int gn ) {
  int i;
  
  ch->pcdata->group_known[gn] = TRUE;
  // Added by SinaC 2000
  for ( i = 0; i < /*MAX_IN_GROUP*/group_table[gn].spellnb; i++) {
    // Removed by SinaC 2000
    //if (group_table[gn].spells[i] == NULL)
    //  break;
    group_add(ch,group_table[gn].spells[i],FALSE);
  }
}

/* recusively removes a group given its number -- uses group_remove */
void gn_remove( CHAR_DATA *ch, int gn) {
  int i;

  ch->pcdata->group_known[gn] = FALSE;

  // Added by SinaC 2000
  for ( i = 0; i < /*MAX_IN_GROUP*/group_table[gn].spellnb; i++) {
    // Removed by SinaC 2000
    //if (group_table[gn].spells[i] == NULL)
      //break;
    group_remove(ch,group_table[gn].spells[i]);
  }
}
	
/* use for processing a skill or group for addition  */
// Modified by SinaC 2003, if level <> 0, we set ch->pcdata->ability_info[sn].level  to  level;
void group_add( CHAR_DATA *ch, const char *name, bool deduct ) {
  int sn,gn;

  if (IS_NPC(ch)) /* NPCs cannot gain skills */
    return;

  // first check skills/spells/powers

  sn = ability_lookup(name);

  if (sn != -1) { 
#ifdef VERBOSE
    log_stringf("->Ability %s (%s)", ability_table[sn].name, name );
#endif

    if (ch->pcdata->ability_info[sn].learned == 0) {/* i.e. not known */
      ch->pcdata->ability_info[sn].learned = 1;
      // Modified by SinaC 2001, if the skill/spell has levels, set to 1 instead of 0
      if ( ability_table[sn].nb_casting_level > 0 )
	ch->pcdata->ability_info[sn].casting_level = 1;
      else
	ch->pcdata->ability_info[sn].casting_level = 0;
      if (deduct)
	ch->pcdata->points += class_abilityrating( ch, sn, ch->pcdata->ability_info[sn].casting_level );
    }
    return;
  }
  
  /* now check groups */
  
  gn = group_lookup(name);
  
  if (gn != -1) {
#ifdef VERBOSE
    log_stringf("->Group %s (%s)", group_table[gn].name, name );
#endif

    if (ch->pcdata->group_known[gn] == FALSE) {
      ch->pcdata->group_known[gn] = TRUE;
      if (deduct)
	ch->pcdata->points += class_grouprating( ch->bstat(classes), gn );
    }
    gn_add(ch,gn); /* make sure all skills in the group are known */
  }
}

/* used for processing a skill or group for deletion -- no points back! */

void group_remove(CHAR_DATA *ch, const char *name) {
  int sn, gn;
    
  sn = ability_lookup(name);

  if (sn != -1) {
      // Modified by SinaC 2001
    ch->pcdata->ability_info[sn].learned = 0;
    ch->pcdata->ability_info[sn].casting_level = 0;
    return;
  }
  
  /* now check groups */
  
  gn = group_lookup(name);
  
  if (gn != -1 && ch->pcdata->group_known[gn] == TRUE) {
    ch->pcdata->group_known[gn] = FALSE;
    gn_remove(ch,gn);  /* be sure to call gn_remove on all remaining groups */
  }
}

// Added by SinaC 2003 for god sphere
void add_minor_sphere( CHAR_DATA *ch, int godId ) {
  if (IS_NPC(ch)) {
    bug("using add_sphere on a mob: %s", ch->short_descr );
    return;
  }

  if ( godId < 0 || godId >= MAX_GODS ) {
    bug("add_sphere: invalid god [%d] for player: %s", godId, ch->name );
    return;
  }

  god_data *god = &(gods_table[godId]);
  if ( god->minor_sphere < 0 ) {
    bug("add_sphere: god (%s) has no sphere", god->name );
    return;
  }

  //  if ( god->nb_priest == 0 ) {
  if ( god->priest == 0 ) {
    bug("add_sphere: god (%s) has no classes getting sphere(priests)", god->name );
    return;
  }

  //  for ( int i = 0; i < god->nb_priest; i++ ) {
  //    int cla = god->priest[i];
  //    if ( (ch->bstat(classes) & cla) == cla ) {
  //      log_stringf("Adding %s's sphere (%s) to %s", 
  //		  god->name, group_table[god->minor_sphere].name, ch->name );
  //      gn_add( ch, god->minor_sphere ); // add the minor sphere
  //      break; // we don't want to add the sphere more than once
  //             //  if the player has more than one class accepted by god
  //    }
  //  }
  if ( ch->bstat(classes) & god->priest ) { // found player's classes in god's priest
    log_stringf("Adding %s's sphere (%s) to %s", 
		god->name, group_table[god->minor_sphere].name, ch->name );
    gn_add( ch, god->minor_sphere ); // add the minor sphere
  }
}
