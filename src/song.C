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
*       ROM 2.4 is copyright 1993-1998 Russ Taylor                         *
*       ROM has been brought to you by the ROM consortium                  *
*           Russ Taylor (rtaylor@hypercube.org)                            *
*           Gabrielle Taylor (gtaylor@hypercube.org)                       *
*           Brian Moore (zump@rom.org)                                     *
*       By using this code, you have agreed to follow the terms of the     *
*       ROM license, in the file Rom24/doc/rom.license                     *
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
#include "song.h"
#include "gsn.h"
#include "handler.h"
#include "comm.h"
#include "olc_value.h"
#include "magic.h"
#include "classes.h"
#include "db.h"
#include "act_comm.h"
#include "fight.h"
#include "interp.h"
#include "lookup.h"
#include "ability.h"
#include "const.h"
#include "recycle.h"
#include "config.h"
#include "act_wiz.h"
#include "utils.h"
#include "damage.h"


// Added by SinaC 2003 for bard (songs)
void do_songs(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char arg[MAX_INPUT_LENGTH];
  char spell_list[MAX_LEVEL+1][MAX_STRING_LENGTH];
  char spell_columns[MAX_LEVEL+1];
  int sn, level, 
    min_lev = 0, 
    max_lev = MAX_LEVEL,
    mana,
    pra;
  bool fAll = FALSE, found = FALSE;
  char buf[MAX_STRING_LENGTH];
 
  if (IS_NPC(ch))
    return;

  if (argument[0] != '\0') {
    fAll = TRUE;
    
    if (str_prefix(argument,"all")) {
      argument = one_argument(argument,arg);
      if (!is_number(arg)) {
	send_to_char("Arguments must be numerical or all.\n\r",ch);
	return;
      }
      max_lev = atoi(arg);
      
      if (max_lev < 0 || max_lev > LEVEL_HERO) {
	sprintf(buf,"Levels must be between 1 and %d.\n\r",LEVEL_HERO);
	send_to_char(buf,ch);
	return;
      }
      
      if (argument[0] != '\0') {
	argument = one_argument(argument,arg);
	if (!is_number(arg)) {
	  send_to_char("Arguments must be numerical or all.\n\r",ch);
	  return;
	}
	min_lev = max_lev;
	max_lev = atoi(arg);
	if (max_lev < 0 || max_lev > LEVEL_HERO) {
	  sprintf(buf,
		  "Levels must be between 1 and %d.\n\r",LEVEL_HERO);
	  send_to_char(buf,ch);
	  return;
	}
	
	if (min_lev > max_lev) {
	  send_to_char("That would be silly.\n\r",ch);
	  return;
	}
      }
    }
  } 

  /* initialize data */
  for (level = 0; level < MAX_LEVEL + 1; level++) {
    spell_columns[level] = 0;
    spell_list[level][0] = '\0';
  }
  /***************************/ 

  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;
    
    level = class_abilitylevel( ch, sn);
    pra = get_ability_simple( ch, sn );

    if ( get_raceability( ch, sn ) 
	 || get_clanability(ch, sn )
	 /*|| get_godskill( ch, sn )    removed by SinaC 2003*/) {
      level = 0;
      pra = 100;
    }
    
    if ( ( level < LEVEL_HERO + 1 
	   || ch->level >= IM  )
	 &&  (fAll || level <= ch->level)
	 &&  level >= min_lev && level <= max_lev
	 && ability_table[sn].type == TYPE_SONG
	 &&  pra > 0 ) {
      found = TRUE;
      if (ch->level < level)
	sprintf(buf,"%-20s  n/a       ", ability_table[sn].name);
      else {
	if (ch->level + 2 == level )
	  mana = 50;
	else
	  mana = UMAX( ability_table[sn].min_cost,
		       100 / ( 2 + ch->level -  level ) );

	int casting_level = get_casting_level( ch, sn );
	if ( casting_level != 0 )
	  sprintf(buf,
		  "%-20s  %3d  (%2d) ",
		  ability_table[sn].name,
		  mana,
		  casting_level);
	else
	  sprintf(buf,
		  "%-20s  %3d       ",
		  ability_table[sn].name,
		  mana);
      }
      
      if (spell_list[level][0] == '\0')
	sprintf(spell_list[level],"\n\rLvl %3d: %s",level,buf);
      else { /* append */
	if ( ++spell_columns[level] % 2 == 0)
	  strcat(spell_list[level],"\n\r         ");
	strcat(spell_list[level],buf);
      }
    }
  }
  
  /* return results */
 
  if (!found) {
    send_to_char("No songs found.\n\r",ch);
    return;
  }

  // Added by SinaC 2000
  send_to_charf( ch,
		 "Level    Song name            Mana   lvl"
		 " Song name            Mana  lvl\n\r"
		 "----------------------------------------"
		 " ------------------------------");

  buffer = new_buf();
  for (level = 0; level < MAX_LEVEL+1; level++)
    if (spell_list[level][0] != '\0')
      add_buf(buffer,spell_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}

void do_song( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *instrument;
  char buf[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  int songnum, chance, mana, level;
  AFFECT_DATA af;

  if ((chance = get_ability(ch,gsn_song)) == 0) {
    send_to_char("You have no clue how to play music.\n\r", ch);
    return;
  }

  if ((instrument = get_eq_char(ch, WEAR_HOLD)) == NULL) {
    send_to_char("You aren't carrying an instrument.\n\r", ch);
    return;
  }

  if (instrument->item_type != ITEM_INSTRUMENT) {
    send_to_char("You aren't carrying an instrument.\n\r", ch);
    return;
  }

  // Added by SinaC 2000
  if ( is_affected( ch, gsn_song ) ) {
    send_to_char("You're still occupied to song, wait before singing a new one.\n\r",ch);
    return;
  }

  if (argument[0] == '\0') {
    send_to_char("Song what?\n\r", ch);
    return;
  }

  one_argument(argument,arg1);

  if( //(songnum = find_spell(ch,arg1)) < 1   Modified by SinaC 2003
     (songnum = find_ability(ch,arg1,TYPE_SONG)) < 1
     || ability_table[songnum].type != TYPE_SONG
     || get_ability(ch,songnum)==0 ) {
    send_to_char( "You don't know any songs of that name.\n\r", ch );
    if ( SCRIPT_VERBOSE > 0 ) {
      if ( IS_NPC(ch) )
	log_stringf("%s (%d) tries to song: %s",
		    NAME(ch), ch->pIndexData->vnum,
		    arg1 );
    }
    return;
  }

  ability_type *sk = &(ability_table[songnum]);

  if (ch->stunned) {
    send_to_char("You're still a little woozy.\n\r",ch);
    return;
  }

  if ( ch->position < sk->minimum_position ) {
    send_to_char( "You need to be standing up to play that song.\n\r", ch );
    return;
  }

  if (ch->level + 2 == class_abilitylevel(ch,songnum))
    mana = 50;
  else
    mana = UMAX( sk->min_cost,
		 100 / ( 2 + ch->level -  class_abilitylevel(ch,songnum) ) );

  if (ch->mana < mana) {
    send_to_char("You don't have enough mana.\n\r", ch);
    return;
  }

  act("$n plays a melody on $p.",ch,instrument,NULL,TO_ROOM);
  act("You play a melody on $p.",ch,instrument,NULL,TO_CHAR);

  WAIT_STATE( ch, BEATS(songnum) );

  /* average of level of player and level of instrument */
  level = (ch->level + instrument->level) / 2;

  // 2 skill checks
  if ( number_percent() > chance
       || number_percent() > get_ability(ch,songnum)) {
    ch->mana -= mana / 2;
    act("$n's fingers slip and the song ends abruptly.",ch,NULL,NULL,TO_ROOM);
    send_to_char("Your fingers slip and the song ends abruptly.\n\r", ch);
    check_improve(ch,songnum,FALSE,1);
    check_improve(ch,gsn_song,FALSE,1);
    return;
  }
  else {/* actually start playing the song */
    ch->mana -= mana;

    // Added by SinaC 2000
    sprintf(buf,"$n starts to sing the %s.",sk->name);
    act(buf,ch,NULL,NULL,TO_ROOM);
    sprintf(buf,"You start to sing the %s.\n\r",sk->name);
    send_to_char(buf,ch);

    if (IS_NPC(ch) || class_fMana(ch->cstat(classes)))
      (*(SONG_FUN*)sk->action_fun) ( songnum, level, ch );
    else
      (*(SONG_FUN*)sk->action_fun) ( songnum, 3 * level/4, ch );
    check_improve(ch,songnum,TRUE,1);
    check_improve(ch,gsn_song,TRUE,1);

    if (sk->to_wait && !IS_IMMORTAL(ch)) {
      //afsetup(af, CHAR, NA, ADD, 0, sk->to_wait, level, gsn_song);
      //affect_to_char(ch, &af);
      createaff(af,sk->to_wait,level,gsn_song,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
      //addaff(af,CHAR,NA,ADD,0);
      affect_to_char( ch, &af );
      // FIXME: we could store which song we are singing
      //  and the last verse we have song
      //  so the song should be linked to a song file
      //  containing song's verse
    }
  }

  return;
}


void song_of_huma( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch;
  //AFFECT_DATA af1, af2;
  AFFECT_DATA af;

  //afsetup( af1, CHAR, hitroll, ADD, level/8, level/2, level, songnum );
  //afsetup( af2, CHAR, allAC, ADD, -20, level/2, level, songnum );
  createaff(af,level/2,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,level/8);
  addaff(af,CHAR,allAC,ADD,-20);

  if (!is_affected( ch, songnum )) {
    act("$n glows briefly.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You glow briefly.\n\r", ch);
    //affect_to_char(ch,&af1);
    act("$n is suddenly surrounded by a glowing suit of armor.",ch,NULL,NULL,TO_ROOM);
    send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", ch);
    //affect_to_char(ch,&af2);
    affect_to_char(ch,&af);
  }

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room ) {
    if (ch == vch)
      continue;

    if (!IS_NPC(vch) || IS_AFFECTED(vch, AFF_CHARM) ) {
      if (number_percent() <= 75 && !is_affected( vch, songnum )) {
	act("$N glows briefly.",ch,NULL,vch,TO_ROOM);
	act("$N glows briefly.",ch,NULL,vch,TO_CHAR);
	send_to_char("You glow briefly.\n\r", vch);
	//affect_to_char( vch, &af1 );
	act("$N is suddenly surrounded by a glowing suit of armor.",ch,NULL,vch,TO_ROOM);
	act("$N is suddenly surrounded by a glowing suit of armor.",ch,NULL,vch,TO_CHAR);
	send_to_char("You are suddenly surrounded by a glowing suit of armor.\n\r", vch);
	//affect_to_char( vch, &af2 );
	affect_to_char( vch, &af );
      }
    }
  }

  return;
}

// Songs coming from Solace mud
// Create spring
void song_ballad_of_spring( int songnum, int level, CHAR_DATA *ch ) {
  OBJ_DATA *spring;

  spring = create_object( get_obj_index( OBJ_VNUM_SPRING ), 0 );
  spring->timer = level;
  obj_to_room( spring, ch->in_room );
  act( "$p flows from the ground.", ch, spring, NULL, TO_ROOM );
  act( "$p flows from the ground.", ch, spring, NULL, TO_CHAR );

  // Added by SinaC 2001
  recomproom(ch->in_room);
}

// Ice Storm
void song_chill_winter_of_vingaard( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  int dam;

  act("$n's voice delivers a chilling blast.", ch, NULL, NULL, TO_ROOM);

  dam = dice(level+2,10);
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    if (saves_spell(level,vch,DAM_COLD))
      ability_damage(ch,vch,dam/2,songnum,DAM_COLD,TRUE,TRUE);
    else
      ability_damage(ch,vch,dam,songnum,DAM_COLD,TRUE,TRUE);
  }
}

// Calm
void song_crysanias_song( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch;
  int mlevel = 0;
  int count = 0;
  int high_level = 0;    
  int chance;
  AFFECT_DATA af;

  /* get sum of all mobile levels in the room */
  for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
    if (vch->position == POS_FIGHTING) {
      count++;
      if (IS_NPC(vch))
	mlevel += vch->level;
      else
	mlevel += vch->level/2;
      high_level = UMAX(high_level,vch->level);
    }
  }

  /* compute chance of stopping combat */
  chance = 4 * level - high_level + 2 * count;

  if (IS_IMMORTAL(ch)) /* always works */
    mlevel = 0;

  if (number_range(0, chance) >= mlevel) { /* hard to stop large fights */
    for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
      if (IS_NPC(vch) 
	  && (IS_SET(vch->cstat(imm_flags),IRV_MAGIC) 
	      || IS_SET(vch->act,ACT_UNDEAD)
	      || IS_SET(vch->cstat(form),FORM_UNDEAD)))
	return;

      if (IS_AFFECTED(vch,AFF_CALM) 
	  || IS_AFFECTED(vch,AFF_BERSERK)
	  ||  is_affected(vch,gsn_frenzy))
	return;
	    
      send_to_char("A wave of calm passes over you.\n\r",vch);

      if (vch->fighting || vch->position == POS_FIGHTING)
	stop_fighting(vch,FALSE);

      //afsetup(af,CHAR,affected_by,OR,AFF_CALM,level/4,level,songnum);
      //affect_to_char(vch,&af);
      //afsetup(af,CHAR,hitroll,ADD,IS_PC(vch)?-5:-2,level/4,level,songnum);
      //affect_to_char(vch,&af);
      //af.location = ATTR_damroll;
      //affect_to_char(vch,&af);
      createaff(af,level/4,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
      addaff(af,CHAR,hitroll,ADD,IS_PC(vch)?-5:-2);
      addaff(af,CHAR,damroll,ADD,IS_PC(vch)?-5:-2);
      addaff(af,CHAR,affected_by,OR,AFF_CALM);
      affect_to_char( vch, &af );
    }
  }
}

void song_dragons_come( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;
  AFFECT_DATA af;
  bool bad_fail, flee_fail;
  int range, dur, mod;


  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    
    bad_fail = FALSE;
    flee_fail = FALSE;
    
    if (is_affected(vch,songnum) || vch == ch )
      continue;
    
    if (!IS_AWAKE(vch)) {
      act("$n shivers momentarily but it passes.",vch,0,0,TO_ROOM);
      send_to_char("You feel a brief terror, but it passes away in your dreams.\n\r",
		   vch);
      return;
    }

    if (saves_spell(level,vch,DAM_CHARM)) {
      act("$n shivers momentarily but it passes.",vch,0,0,TO_ROOM);
      send_to_char("You feel a brief terror, but it passes.\n\r",vch);
      return;
    }
    
    if (!saves_spell(level - 2,vch,DAM_CHARM)) {
      bad_fail = TRUE;
      if (!saves_spell(level - 2,vch,DAM_CHARM))
	flee_fail = TRUE;
    }
  
    act("$n's eyes widen in shock and $s entire body freezes in momentary terror.",vch,NULL,NULL,TO_ROOM);
    send_to_char("You feel an overwhelming terror and you shudder in momentary shock.\n\r",vch);
    
    range = level/10;

    dur = (number_range(1,5) + range);

    //afsetup( af, CHAR, allAC, ADD, number_range(2,range), dur, level, songnum );
    //affect_to_char( vch, &af );
    //afsetup( af, CHAR, saving_throw, ADD, number_range(2,range), dur, level, songnum );
    //affect_to_char( vch, &af );
    createaff(af,dur,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,allAC,ADD,number_range(2,range));
    addaff(af,CHAR,saving_throw,ADD,number_range(2,range));
    affect_to_char( vch, &af );

    if ( flee_fail ) {
      if (vch->position == POS_FIGHTING) do_flee(vch,"");
      if (vch->position == POS_FIGHTING) do_flee(vch,"");
      if (vch->position == POS_FIGHTING) do_flee(vch,"");
    }
    
    if (bad_fail) WAIT_STATE(vch,12);
  }
}

void song_hymn_for_the_ancestors( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;

  int slumber_sn = ability_lookup("slumber");

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    if ( vch != ch      // 40% chance not healing other people in the room
	 && chance(40) )
      continue;

    if (check_dispel(level,vch,gsn_sleep)
	|| check_dispel(level,vch,slumber_sn)) {
      send_to_char("You feel less tired.\n\r", vch);
      act("$n is less tired.",vch,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,vch,gsn_poison)) {
      send_to_char("A warm feeling runs through your body.\n\r",vch);
      act("$n looks much better.",vch,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,vch,gsn_plague)) {
      send_to_char("Your sores vanish.\n\r",vch);
      act("$n looks relieved as $s sores vanish.",vch,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,vch,gsn_curse)) {
      send_to_char("You feel better.\n\r",vch);
      act("$n looks more relaxed.",vch,NULL,NULL,TO_ROOM);
    }
    if (check_dispel(level,vch,gsn_blindness)) {
      send_to_char( "Your vision returns!\n\r", vch );
      act("$n is no longer blinded.",vch,NULL,NULL,TO_ROOM);
    }
  }
}

void song_journeymans_rhyme( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    if ( vch == vch )
      vch->move = UMIN( vch->move + UMAX(3*level/2,40), vch->cstat(max_move) );
    else
      vch->move = UMIN( vch->move + UMAX(3*level/4,40), vch->cstat(max_move) );
    if (vch->cstat(max_move) == vch->move)
      send_to_char("You feel fully refreshed!\n\r",vch);
    else
      send_to_char( "You feel less tired.\n\r", vch );
  }
}

void song_knights_ride_forth( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;

  //AFFECT_DATA af1, af2;
  AFFECT_DATA af;
  //afsetup( af1, CHAR, hitroll, ADD, level/8, level/2, level, songnum );
  //affect_to_char( ch, &af1 );
  //afsetup( af2, CHAR, damroll, ADD, level/8, level/2, level, songnum );
  //affect_to_char( ch, &af2 );
  createaff(af,level/2,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,level/8);
  addaff(af,CHAR,damroll,ADD,level/8);
  affect_to_char( ch, &af );

  //af1.modifier /= 2;
  //af2.modifier /= 2;
  createaff(af,level/2,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,hitroll,ADD,level/16);
  addaff(af,CHAR,damroll,ADD,level/16);
		  
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    if ( vch != ch ) {
      //affect_to_char( vch, &af1 );
      //affect_to_char( vch, &af2 );
      affect_to_char( vch, &af );
    }
  }
}

// ONE DAY BEFORE BEING ABLE TO USE THIS SONG AGAIN
void song_looking_in_the_mirror( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *clone;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may sing this song.\n\r", ch );
    return;
  }

  if ( ch->fighting == NULL ) {
    send_to_char("You must be fighting in order to song this song.\n\r", ch );
    return;
  }

  CHAR_DATA *parent = ch->fighting;

  clone = create_mobile( get_mob_index(MOB_VNUM_MIRROR_IMAGE) );
  /* start fixing values */
  for (int i = 0; i < ATTR_NUMBER; i++)
    clone->baseattr[i] = parent->baseattr[i];

  clone->name        	= str_dup("mirror image");
  clone->level	        = parent->level;
  clone->hit		= parent->hit;
  clone->mana		= parent->mana;
  clone->psp		= parent->psp;
  clone->move		= parent->move;
  clone->gold		= 0;
  clone->silver	        = 0;
  clone->act		= ACT_PET | ACT_IS_NPC;

  AFFECT_DATA *paf_next;
  for (AFFECT_DATA *paf = clone->affected; paf != NULL; paf = paf_next) {
    paf_next = paf->next;
    affect_remove(clone,paf);
  }
  
  /* now add the affects */
  for (AFFECT_DATA *paf = parent->affected; paf != NULL; paf = paf->next)
    affect_to_char(clone,paf);

  for (OBJ_DATA *obj = parent->carrying; obj != NULL; obj = obj->next_content) {
    OBJ_DATA *new_obj = create_object(obj->pIndexData,0);
    clone_object(obj,new_obj);
    SET_BIT( new_obj->base_extra, ITEM_NODROP|ITEM_NOREMOVE|ITEM_NOUNCURSE ); 
    SET_BIT( new_obj->extra_flags, ITEM_NODROP|ITEM_NOREMOVE|ITEM_NOUNCURSE ); 
    recursive_clone2(obj,new_obj);
    obj_to_char(new_obj,clone);
    new_obj->wear_loc = obj->wear_loc;
  }

  char buf[MAX_STRING_LENGTH];
  sprintf(buf, clone->short_descr, parent->name );
  clone->short_descr = str_dup( buf );

  sprintf(buf, clone->long_descr, parent->name );
  clone->long_descr = str_dup( buf );

  sprintf(buf, clone->description, parent->name );
  clone->description = str_dup( buf );

  char_to_room(clone,parent->in_room);
  act("$n has created $N's mirror image.",ch,parent,clone,TO_ROOM);
  act("You create $N's mirror image.",ch,parent,clone,TO_CHAR);

  AFFECT_DATA af;
  //afsetup(af, CHAR, NA, ADD, 0, -1, level, songnum);
  //affect_to_char(clone,&af);
  createaff(af,-1,level,songnum,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
  //addaff(af,CHAR,NA,ADD,0);
  affect_to_char( clone, &af );

  clone->position = POS_STANDING;
  multi_hit( clone, parent, TYPE_UNDEFINED ); // initiates the fight

  return;
}

void song_mishakals_song( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    int heal = dice(4, 10) + level - 6;
    vch->hit = UMIN( vch->hit + heal, vch->cstat(max_hit) );
    update_pos( vch );
    send_to_char( "You feel better!\n\r", vch );
  }
}

void song_neverending_pain( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;
  int dam = dice(level+2,10);
  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) || is_affected(vch,songnum) )
      continue;
    if (saves_spell(level,vch,DAM_NEGATIVE)) // DAM_SOUND ?
      ability_damage(ch,vch,dam/2,songnum,DAM_NEGATIVE,TRUE,TRUE);
    else {
      int done = ability_damage(ch,vch,dam,songnum,DAM_NEGATIVE,TRUE,TRUE);
      if ( done == DAMAGE_DONE ) {
	AFFECT_DATA af;
	//afsetup(af, CHAR, NA, ADD, 0, number_range(level,level*2), level, songnum);
	//affect_to_char(vch,&af);
	createaff(af,number_range(level,level*2),level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
	//addaff(af,CHAR,NA,ADD,0);
	affect_to_char( vch, &af );
      }
    }
  }
}

void song_palistan_song( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;

  for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
    vch_next = vch->next_in_room;

    int heal = dice(3, 8) + level/2;
    vch->hit = UMIN( vch->hit + heal, vch->cstat(max_hit) );
    update_pos( vch );
    send_to_char( "You feel better!\n\r", vch );
  }
}

void song_rest_of_dragons( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;
  AFFECT_DATA af;
  
  //afsetup(af,CHAR,affected_by,OR,AFF_SLEEP,UMAX(level/5,10),level,songnum);
  createaff(af,UMAX(level/5,10),level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  addaff(af,CHAR,affected_by,OR,AFF_SLEEP);

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( is_safe_spell(ch,vch,TRUE) || is_same_group( vch, ch ) || is_affected( vch, songnum ) )
      continue;

    if ( IS_AFFECTED(vch, AFF_SLEEP)
	 || ( IS_NPC(vch) 
	      && ( IS_SET(vch->act,ACT_UNDEAD)
		   || IS_SET(vch->act, ACT_NOSLEEP ) ) )
	 || IS_SET( vch->cstat(form),FORM_UNDEAD)
	 || (level + 2) < vch->level
	 || saves_spell( level-4, vch,DAM_CHARM) ) {
      ability_damage(ch,vch,0,songnum,DAM_NONE,TRUE, TRUE);
      continue;
    }

    affect_join( vch, &af );
    
    if ( IS_AWAKE(vch) ) {
      send_to_char( "You feel very sleepy ..... zzzzzz.\n\r", vch );
      act( "$n goes to sleep.", vch, NULL, NULL, TO_ROOM );
      vch->position = POS_SLEEPING;
    }
  }
}

void song_ride_of_glory( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;
  AFFECT_DATA af;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( !is_same_group( vch, ch ) )
      continue;

    if ( is_affected(vch,gsn_frenzy)
	 || IS_AFFECTED(vch,AFF_BERSERK))
      continue;

    if (is_affected(vch,gsn_calm)
	|| IS_AFFECTED(vch,AFF_CALM) )
      continue;

    //afsetup(af,CHAR,hitroll,ADD,level/6,level/3,level,gsn_frenzy);
    //affect_to_char(vch,&af);
    //af.location  = ATTR_damroll;
    //affect_to_char(vch,&af);
    createaff(af,level/3,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,hitroll,ADD,level/6);
    addaff(af,CHAR,damroll,ADD,level/6);
    affect_to_char( vch, &af );

    send_to_char("You are filled with a battle frenzy!\n\r",vch);
    act("$n gets a wild look in $s eyes!",vch,NULL,NULL,TO_ROOM);
  }
}

void song_shadowborn( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *shadow;
  OBJ_DATA *corpse;
  OBJ_DATA *corpse_next;
  OBJ_DATA *obj_next;
  OBJ_DATA *obj;
  CHAR_DATA *search;
  AFFECT_DATA af;
  const char *name;
  char buf1[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int chance;
  int z_level;
  int control;

  if ( IS_NPC(ch) ) {
    send_to_char("Only players may sing this song.\n\r", ch );
    return;
  }

  if ( ch->in_room == NULL )
    return;

  int base_chance = get_ability(ch,songnum);

  for ( corpse = ch->in_room->contents; corpse != NULL; corpse = corpse_next ) {
    corpse_next = corpse->next_content;

    control = 6;
    
    for (search = char_list; search != NULL; search = search->next) {
      if (IS_NPC(search) && (search->master == ch)
	  && search->pIndexData->vnum == MOB_VNUM_SHADOW)
	control += 6;
      else if (IS_NPC(search) && (search->master == ch)
	       && search->pIndexData->vnum == MOB_VNUM_ZOMBIE)
	control += 6;
      else if (IS_NPC(search) && (search->master == ch)
	       && search->pIndexData->vnum == MOB_VNUM_SKELETON)
	control += 4;
      else if (IS_NPC(search) && (search->master == ch)
	       && search->pIndexData->vnum == MOB_VNUM_MUMMY)
	control += 12;
    }
    
    if ((ch->level < 30 && control > 12) || (ch->level < 35 && control > 18)
	|| (ch->level < 40 && control > 24) || (ch->level < 52 && control > 30)
	|| control > 36) {
      send_to_char("You already control as many undead as you can.\n\r",ch);
      return;
    }
    
    if ((corpse->item_type != ITEM_CORPSE_NPC) 
	&& (corpse->item_type != ITEM_CORPSE_PC) )
      continue;
    
    for (obj = corpse->contains; obj != NULL; obj = obj_next) {
      obj_next = obj->next_content;
      obj_from_obj(obj);
      obj_to_room(obj,ch->in_room);
    }

    chance = base_chance;
    
    if (ch->level < corpse->level) {
      chance += (3*ch->level);
      chance -= (3*corpse->level);
    }
    
    chance = URANGE(20,chance,90);
    
    if (number_percent() > chance) {
      act("$p's shadow wont rise up.",ch,corpse,NULL,TO_CHAR);
      extract_obj(corpse);
      continue;
    }
    
    act("$p's shadow rises up to serve $n.",ch,corpse,NULL,TO_ROOM);
    act("$p's shadow rises up to serve you.",ch,corpse,NULL,TO_CHAR);
    
    shadow = create_mobile(get_mob_index(MOB_VNUM_SHADOW));
    
    z_level = UMAX(1,corpse->level - number_range(3,6));
    shadow->level = z_level;
    shadow->bstat(max_hit) = UMAX((dice(z_level, 15))+(z_level*20),1);
    shadow->hit = shadow->bstat(max_hit);
    shadow->bstat(DICE_TYPE) = UMAX(1, z_level/3);
    if ( z_level >= 1  && z_level <= 10 )
      shadow->bstat(DICE_NUMBER) = 4;
    if ( z_level >= 11 && z_level <= 20 )
      shadow->bstat(DICE_NUMBER) = 5;
    if ( z_level >= 21 && z_level <= 30 )
      shadow->bstat(DICE_NUMBER) = 6;
    if ( z_level > 30 )
      shadow->bstat(DICE_NUMBER) = 7;
    
    shadow->bstat(alignment) = -1000;
    shadow->bstat(etho) = ETHO_LAWFUL;
    
    // So name starts just after 'The corpse of'
    name = corpse->short_descr+14;
    
    sprintf( buf1, "the shadow of %s", name);
    sprintf( buf2, "A shadow of %s is standing here.\n\r", name);

    if ( IS_SET( corpse->baseval[1], FORM_FOUR_ARMS ) )
      SET_BIT(shadow->bstat(form), FORM_FOUR_ARMS );
    
    extract_obj(corpse);
    
    shadow->short_descr = str_dup(buf1);
    shadow->long_descr = str_dup(buf2);
    
    //afsetup(af,CHAR,affected_by,OR,AFF_CHARM,-1,level,songnum);
    //affect_to_char( shadow, &af );
    createaff(af,-1,level,songnum,0,AFFECT_ABILITY|AFFECT_PERMANENT|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,affected_by,OR,AFF_CHARM);
    affect_to_char( shadow, &af );

    SET_BIT(shadow->act, ACT_CREATED ); // SinaC 2003

    char_to_room(shadow,ch->in_room);
    add_follower(shadow,ch);
    shadow->leader = ch;
    
    //recompute(shadow); NO NEED: done in char_to_room
  }
  recomproom(ch->in_room); // NEEDED: every corpses can be destroyed without creating any shadow
}

void song_shallow_grave( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;
  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;
    // works only on undead NPC
    if ( !IS_NPC(vch)
	 || !( IS_SET(vch->act,ACT_UNDEAD)
	       || IS_SET( vch->cstat(form),FORM_UNDEAD)) )
      continue;
    
    // and not in the same group as caster
    if ( is_same_group( vch, ch ) || is_safe_spell(ch,vch,TRUE) )
      continue;
    
    // if really low level victim ... destroy it
    if ( !saves_spell( level-5, vch, DAM_CHARM ) 
	 && chance(UMAX(level-vch->level,0)) ) {
      act("$N decays into dust.", ch, NULL,vch,TO_NOTVICT);
      send_to_charf(vch,"You decay into dust!\n\r");
      act("$N decays into dust.",ch,NULL,vch,TO_CHAR);
      sudden_death( ch, vch );
      continue;
    }
    
    // saves spells ?
    if ( saves_spell( level, vch, DAM_CHARM ) )
      continue;
    
    if (vch->position == POS_FIGHTING) do_flee(vch,"");
    if (vch->position == POS_FIGHTING) do_flee(vch,"");
    if (vch->position == POS_FIGHTING) do_flee(vch,"");
    // Flee successfull ??
    if ( vch->in_room == ch->in_room ) // not sucessfull, does some damage
      ability_damage( ch, vch, dice(level,8), songnum, DAM_CHARM, TRUE, TRUE );
  }
  return;
}

void song_shield_of_words( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch;
  //AFFECT_DATA af1, af2;
  AFFECT_DATA af;

  //afsetup(af1,CHAR,affected_by,OR,AFF_SANCTUARY,level/6,level,songnum);
  //afsetup(af2,CHAR,allAC,ADD,-40,level/6,level,songnum);

  bool add = FALSE;
  createaff(af,level/6,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
  if ( !IS_AFFECTED( ch, AFF_SANCTUARY ) && !is_affected( ch, songnum ) ) {
    act( "$n is suddenly surrounded by a {Wwhite aura{x.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You are suddenly surrounded by a {Wwhite aura{x.\n\r", ch );
    addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
    add = TRUE;
  }
  if ( !is_affected( ch, songnum ) ) {
    act( "$n becomes shielded from harm.", ch, NULL, NULL, TO_ROOM );
    send_to_char( "You become shielded from harm.\n\r", ch );
    addaff(af,CHAR,allAC,ADD,-40);
    add = TRUE;
  }
  if ( add )
    affect_to_char( ch, &af );

  for ( vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room ) {
    if (ch == vch)
      continue;
      
    if (!IS_NPC(vch) || IS_AFFECTED(vch, AFF_CHARM) ) {
      add = FALSE;
      createaff(af,level/6,level,songnum,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
      if ( number_percent() <= 70 && !is_affected( vch, songnum )) {
	act("$N becomes shielded from harm",ch,NULL,vch,TO_ROOM);
	act("$N becomes shielded from harm",ch,NULL,vch,TO_CHAR);
	send_to_char("You become shielded from harm.\n\r", vch);
	addaff(af,CHAR,allAC,ADD,-40);
	add = TRUE;
      }
      if ( number_percent() <= 50 && !IS_AFFECTED( ch, AFF_SANCTUARY ) ) {
	act("$N is suddenly surrounded by a {Wwhite aura{x.",ch,NULL,vch,TO_ROOM);
	act("$N is suddenly surrounded by a {Wwhite aura{x.",ch,NULL,vch,TO_CHAR);
	send_to_char("You are suddenly surrounded by a {Wwhite aura{x.\n\r", vch);
	addaff(af,CHAR,affected_by,OR,AFF_SANCTUARY);
	add = TRUE;
      }
      if ( add )
	affect_to_char( vch, &af );
    }
  }
}

void song_wail_of_banshees( int songnum, int level, CHAR_DATA *ch ) {
  CHAR_DATA *vch, *vch_next;
  int dam;
  /* This spell will when used.. wipe out your mana */
  for ( vch = ch->in_room->people; vch; vch = vch_next ) {
    vch_next = vch->next_in_room;
    if ( vch == ch )
      continue;
    if ( !is_safe_spell( ch, vch, TRUE ) && !is_same_group(vch,ch) ) {
      act( "{c$n screams a horrible sound! Your ears pop{x!", ch, NULL, vch, TO_VICT );
      dam = dice(UMAX(ch->level/5,7),UMAX(ch->level/2,7));
      if ( saves_spell( ch->level, vch, DAM_SOUND ) )	
	dam /= 2;
      ability_damage( ch, vch, dam, songnum, DAM_SOUND,TRUE,TRUE);
    } 
    else
      act( "{c$n screams a horrible sound!{x", ch, NULL, vch, TO_VICT );
  }
  return;
}

void song_when_the_sun_goes_down( int songnum, int level, CHAR_DATA *ch ) {
  AFFECT_DATA af;
  CHAR_DATA *vch, *vch_next;

  for ( vch = ch->in_room->people; vch != NULL; vch = vch_next ) {
    vch_next = vch->next_in_room;

    if ( is_safe_spell(ch,vch,TRUE) || is_same_group( vch, ch ) )
      continue;
    
    if ( saves_spell( level, vch,DAM_OTHER) || is_affected( vch, songnum ) ) {
      ability_damage(ch,vch,0,songnum,DAM_NONE,TRUE, TRUE);
      continue;
    }

    //afsetup(af,CHAR,STR,ADD,-level/5,level/2,level,songnum);
    //affect_to_char( vch, &af );
    //afsetup(af,CHAR,affected_by,OR,AFF_WEAKEN,level/2,level,songnum);
    //affect_to_char( vch, &af );
    createaff(af,level/2,level,gsn_song,0,AFFECT_ABILITY|AFFECT_NON_DISPELLABLE);
    addaff(af,CHAR,STR,ADD,-level/5);
    addaff(af,CHAR,affected_by,OR,AFF_WEAKEN);
    affect_to_char( vch, &af );
    
    send_to_char( "You feel your strength slip away.\n\r", vch );
    act("$n looks tired and weak.",vch,NULL,NULL,TO_ROOM);
  }
}
