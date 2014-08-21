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
#include "db.h"
#include "recycle.h"
#include "magic.h"
#include "classes.h"
//#include "olc.h"
#include "tables.h"
//#define _XOPEN_SOURCE
//#include <unistd.h>

// Added by SinaC 2001
#include "language.h"
#include "comm.h"
#include "handler.h"
#include "act_wiz.h"
#include "save.h"
#include "special.h"
#include "lookup.h"
#include "data.h"
#include "interp.h"
#include "olc_value.h"
#include "affects.h"
#include "restriction.h"
#include "abilityupgrade.h"
#include "names.h"
#include "const.h"
#include "wiznet.h"
#include "skills.h"
#include "wearoff_affect.h"
#include "update_affect.h"
#include "scrhash.h"
#include "dbscript.h"
#include "act_info.h"
#include "faction.h"
#include "group.h"
#include "ban.h"
#include "brew.h"
#include "prereqs.h"
#include "clan.h"
#include "config.h"
#include "olc.h"
#include "bit.h"
#include "dbdata.h"
#include "utils.h"
#include "moons.h"
#include "faction.h"


/*
 * Added by SinaC 2000
 */
void do_setpassword( CHAR_DATA *ch, const char *argument )
{
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  const char *pwdnew;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  argument=one_argument( argument, arg1 );
  argument=one_argument( argument, arg2 );
  
  if (  ( ch->pcdata->pwd != '\0' )
	&& ( arg1[0] == '\0' || arg2[0] == '\0')  ) {
    send_to_char( "Syntax: setpassword <char> <new>.\n\r", ch );
    return;
  }
  
  victim = get_char_world(ch, arg1);

  if( victim == '\0' ) {
    send_to_char( "That person isn't here, they have to be here to reset pwd's.\n\r", ch);
    return;
  }
  if ( IS_NPC( victim ) ) {
    send_to_char( "You cannot change the password of NPCs!\n\r",ch);
    return;
  }
  
  if ( IS_IMMORTAL(victim) ){
    send_to_char( "You can't change the password of that player.\n\r",ch);
    return;
  }


  if ( strlen(arg2) < 5 ) {
    send_to_char( "New password must be at least five characters long.\n\r", ch );
    return;
  }
  
  pwdnew = crypt( arg2, victim->name );
  //strcpy( pwdnew, arg2);
  //pwdnew = str_dup( arg2 );

  victim->pcdata->pwd = str_dup( pwdnew );
  //save_char_obj( victim );
  new_save_pFile(victim, FALSE );
  send_to_char( "Ok.\n\r", ch );
  sprintf( buf, "Your password has been changed to %s.\n\r", arg2 );
  send_to_char( buf, victim);
  return;
}

// Modified by SinaC 2000 to see unsaved areas, classes/skills saved?,...
void do_mudstatus( CHAR_DATA *ch, const char *argument ) {
  char buf[64];
  bool found;
  AREA_DATA *pArea;

  send_to_charf(ch,"{w  Current MUD Status{x\n\r{w---------------------{x\n\r");
  send_to_charf(ch,"   fBootDb is: %s\n\r", fBootDb?"{rON{x":"{yOFF{x" );
  send_to_charf(ch,"   Double XP is: %s\n\r", double_xp ? "{rON{x":"{yOFF{x" );
  send_to_charf(ch,"   Newlock is: %s\n\r", newlock ? "{rON{x" : "{yOFF{x" );
  send_to_charf(ch,"   Wizlock is: %s\n\r", wizlock ? "{rON{x" : "{yOFF{x" );
  send_to_charf(ch,"   Use faction on aggro: %s\n\r", USE_FACTION_ON_AGGRO ? "{rON{x" : "{yOFF{x" );
  send_to_charf(ch,"   Abilities saved: %s\n\r", abilitynotsaved ? "{rNO{x" : "{yYES{x" );
  send_to_charf(ch,"   Groups saved: %s\n\r", groupnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Spheres saved: %s\n\r", spherenotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Races NPC/PC saved: %s\n\r", racenotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Classes saved: %s\n\r", pcclassnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   God saved: %s\n\r", godnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Liquid saved: %s\n\r", liqnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Material saved: %s\n\r", materialnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Factions saved: %s\n\r", pcclassnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Brew formulas saved: %s\n\r", brewnotsaved ? "{rNO{x": "{yYES{x" );
  send_to_charf(ch,"   Clan table saved: %s\n\r", clannotsaved ? "{rNO{x" : "{yYES{x" );
  send_to_charf(ch,"   Command table saved: %s\n\r", cmdnotsaved ? "{rNO{x" : "{yYES{x" );
  send_to_charf(ch,"   Magic school saved: %s\n\r", schoolnotsaved ? "{rNO{x" : "{yYES{x" );
  send_to_charf(ch,"   Super races saved: %s\n\r", superracenotsaved ? "{rNO{x" : "{yYES{x" );
  send_to_charf(ch,"   Unsaved areas:\n\r");
  found = FALSE;
  for( pArea = area_first; pArea; pArea = pArea->next ) {
    if ( IS_SET(pArea->area_flags, AREA_CHANGED) ) {
      send_to_charf( ch,"     {r%24s - '%s'{x\n\r", pArea->name, pArea->file_name );
      found = TRUE;
    }
  }
  if ( !found )
    send_to_char( "     {yNone{x.\n\r", ch );
  send_to_charf(ch,"   Unsaved programs:\n\r");
  found = FALSE;
  for (int i = 0; i<HASH_PRG; i++)
    for (CLASS_DATA *cl = prg_table[i]; cl; cl=cl->next)
      if ( IS_SET(cl->flags, CLASS_NOT_SAVED ) ) {
	send_to_charf( ch,"     {r%24s - '%s.script'{x\n\r", cl->name, cl->file );
	found = TRUE;
      }
  if ( !found )
    send_to_char( "     {yNone{x.\n\r", ch );
}

/***************************************************************************
*                                                                          *
*  To use this snipet you must set the following line in the "check" help  *
*    Coded for Fallen Angels by : Zilber laurent,Despret jerome.           *
*  And send a mail to say you use it ( feel free to comment ) at :         *
*  [despret@ecoledoc.lip6.fr] or/and at [loran@hotmail.com]                *
****************************************************************************/
/***************************************************************************

   This imm command is to fight cheaters ....
   Allow you to quick detect/compare modified chars...

    Syntax: 'check'       display info about players of all players
            'check stats' display info and resume stats of all players
            'check eq'    resume eq of all players
            'check snoop' show who snoop who ( to avoid lowest imm abuse )
    Use the stat command in case of doubt about someone...
*/
void do_check( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  int count = 1;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  one_argument( argument, arg );
  
  if (arg[0] == '\0'|| !str_prefix(arg,"stats")) {
    buffer = new_buf();
    for (victim = char_list; victim != NULL; victim = victim->next) {
      if (IS_NPC(victim) || !can_see(ch,victim)) 
	continue;
    	    	
      if (victim->desc == NULL) {
	sprintf(buf,"%3d) %s is linkdead.\n\r", count, victim->name);
	add_buf(buffer, buf);
	count++;
	continue;	    	
      }
	    
      if (victim->desc->connected >= CON_GET_NEW_RACE
	  && victim->desc->connected <= CON_PICK_WEAPON) {
	sprintf(buf,"%3d) %s is being created.\n\r",
		count, victim->name);
	add_buf(buffer, buf);
	count++;
	continue;
      }
	    
      if ( (victim->desc->connected == CON_GET_OLD_PASSWORD
	    || victim->desc->connected >= CON_READ_IMOTD)
	   && get_trust(victim) <= get_trust(ch) ) {
	sprintf(buf,"%3d) %s is connecting.\n\r",
		count, victim->name);
	add_buf(buffer, buf);
	count++;
	continue; 	    		 
      }
	    
      if (victim->desc->connected == CON_PLAYING) {
	if (get_trust(victim) > get_trust(ch))
	  sprintf(buf,"%3d) %s.\n\r", count, victim->name);
	else {
	  sprintf(buf,"%3d) %s, Level %d connected since %d hours (%d total hours)\n\r",
		  count, victim->name,victim->level,
		  ((int)(current_time - victim->logon)) /3600, 
		  (victim->played + (int)(current_time - victim->logon)) /3600 );
	  add_buf(buffer, buf);
	  if (arg[0]!='\0' && !str_prefix(arg,"stats")) {
	    // Modified by SinaC 2001 for mental user
	    //	    sprintf(buf,"  %ld HP %ld Mana Psp %ld (%ld %ld %ld %ld %ld) %ld gold %d Tr %d Pr %d QP %d Trva.\n\r",
	    sprintf(buf,"  %ld HP %ld Mana Psp %ld (%ld %ld %ld %ld %ld) %ld gold %d Tr %d Pr %d Trva.\n\r",
		    victim->cstat(max_hit), victim->cstat(max_mana), victim->cstat(max_psp),
		    victim->cstat(STR),
		    victim->cstat(INT),
		    victim->cstat(WIS),
		    victim->cstat(DEX),
		    victim->cstat(CON),
		    victim->gold + victim->silver/100,
		    victim->train, victim->practice, 
		    //victim->pcdata->questpoints,
		    victim->pcdata->trivia);
	    add_buf(buffer, buf);
	  }
	  count++;
	}
	continue;
      }

      sprintf(buf,"%3d) bug (oops)...please report to Loran: [%s] [%d]\n\r",
	      count, victim->name, victim->desc->connected);
      bug("do_check: [%s]", buf );
      add_buf(buffer, buf);
      count++;
    }
    page_to_char(buf_string(buffer),ch);
    return;
  }
    
  if (!str_prefix(arg,"eq")) {
    buffer = new_buf();
    for (victim = char_list; victim != NULL; victim = victim->next) {
      if (IS_NPC(victim)
	  || victim->desc == NULL
	  || victim->desc->connected != CON_PLAYING
	  || !can_see(ch,victim)
	  || get_trust(victim) > get_trust(ch) )
	continue;
    	    	
      long hitr = GET_HITROLL(victim),
	damr = GET_DAMROLL(victim);
	sprintf(buf,"%3d) %s, %d items (weight %d) Hit:%ld Dam:%ld Save:%ld AC:%ld %ld %ld %ld.\n\r",
	      count, victim->name, victim->carry_number, victim->carry_weight, 
	      hitr, damr, 
	      victim->cstat(saving_throw),
	      GET_AC(victim, AC_PIERCE), GET_AC(victim, AC_BASH),
	      GET_AC(victim, AC_SLASH), GET_AC(victim, AC_EXOTIC));
      add_buf(buffer, buf);
      count++;  
    }
    page_to_char(buf_string(buffer),ch);
    return;
  }
 
  if (!str_prefix(arg,"snoop")) /* this part by jerome */ {
    char bufsnoop [100];

    if(ch->level < MAX_LEVEL ) {
      send_to_char("You can't use this check option.\n\r",ch);
      return;
    }
    buffer = new_buf();

    for (victim = char_list; victim != NULL; victim = victim->next) {
      if (IS_NPC(victim)
	  || victim->desc == NULL
	  || victim->desc->connected != CON_PLAYING
	  || !can_see(ch,victim)
	  || get_trust(victim) > get_trust(ch) )
	continue;

      if(victim->desc->snoop_by != NULL)
	sprintf(bufsnoop," %15s .",victim->desc->snoop_by->character->name);
      else
	sprintf(bufsnoop,"     (none)      ." );

      sprintf(buf,"%3d %15s : %s \n\r",count,victim->name, bufsnoop);
      add_buf(buffer, buf);
      count++;
    }
    page_to_char(buf_string(buffer),ch);
    return;
  }

        
  send_to_char("Syntax: 'check'       display info about players\n\r",ch);
  send_to_char("        'check stats' display info and resume stats\n\r",ch);
  send_to_char("        'check eq'    resume eq of all players\n\r",ch);
  send_to_char("        'check snoop' show who snoop who ( to avoid lowest imm abuse )\n\r", ch );
  send_to_char("Use the stat command in case of doubt about someone...\n\r",ch);
  return;
}

/*
From: The Anomaly <anomaly@voicenet.com>

I admit, this was something of a joke when it was suggested to me about 
10 minutes ago, but then again, I'm sure there are some sadistic imps out 
there who might like this.  If you do actually add this to your mud, I 
request that you use it wisely and hopefully not frequently.

You may have to fiddle with this to get it to work. I wrote it in 5 
minutes, and I'm sure there are some bugs. If you can't get it to work, 
you probably shouldn't be doing anything except looking for a coder. It's 
reasonably straightforward.

Now, without further ado, the wizskill addlag:

=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
*/
void do_addlag(CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;
 char arg1[MAX_STRING_LENGTH];
 int x;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  argument = one_argument(argument, arg1);

  if (arg1[0] == '\0') {
      send_to_char("addlag to who?\n\r", ch);
      return;
    }

  if ((victim = get_char_world(ch, arg1)) == NULL) {
      send_to_char("They're not here.\n\r", ch);
      return;
    }

  if ((x = atoi(argument)) <= 0) {
      send_to_char("That makes a LOT of sense.\n\r", ch);
      return;
    }

  if (x > 100) {
      send_to_char("There's a limit to cruel and unusual punishment.\n\r", ch);
      return;
    }

  send_to_char("Somebody REALLY doesn't like you\n\r", victim);
  WAIT_STATE(victim, x);
  send_to_char("Adding lag now...\n\r", ch);
  return;
}

/*
 * Added by SinaC 2000, allow to see the resets of an area
 */
const char *str_to_len( const char *to_resize, unsigned int size )
{
  static char buf[MAX_STRING_LENGTH];

  if ( strlen( to_resize ) > size ) {
      strcpy( buf, to_resize );
      buf[size]='\0';
      return buf;
    }
  else
    return to_resize;
}

void do_resetlist( CHAR_DATA *ch, const char *argument )
{
    ROOM_INDEX_DATA	*pRoom;
    RESET_DATA 		*pReset;
    AREA_DATA 		*pArea;
    char 		arg[MAX_INPUT_LENGTH];
    char 		outbuf[ MAX_STRING_LENGTH*8 ];
    bool		found = FALSE;
    bool		fAll = FALSE;
    bool		fMob = FALSE;
    bool		fObj = FALSE;
    int			rvnum;
    int			vnum = 0;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

    pArea = ch->in_room->area;

    if ( argument[0] == '\0' ) {
	fAll = TRUE;

// Added by SinaC 2000 till I find why it doesn't work without an argument
// TO FIX SinaC 2000
	send_to_char( "Syntax: resetlist mob [vnum]\n\r"
		      "        resetlist obj [vnum]\n\r", ch );
	return;
      }
    else {
	argument = one_argument( argument, arg );
	fMob = !str_cmp( arg, "mob" );
	fObj = !str_cmp( arg, "obj" );
	if ( !fMob && !fObj ) {
	    send_to_char( "Syntax: resetlist mob [vnum]\n\r"
			  "        resetlist obj [vnum]\n\r", ch );
	    return;
	}
	else 
	  if ( is_number( argument ) ) {
	    vnum = atoi( argument );
	    if ( ( vnum < pArea->min_vnum ) || ( vnum > pArea->max_vnum ) ) {
		send_to_char( "Invalid vnum for this area!\n\r", ch );
		return;
	    }
	}
    }

    strcpy( outbuf, "  Vnum         Description       Location Room World/Room\n\r" );
    strcat( outbuf, "======== ======================= ======== ==== ==========\n\r" );

    for  ( rvnum = pArea->min_vnum; rvnum <= pArea->max_vnum; rvnum++ ) {
      if ( (pRoom = get_room_index( rvnum )) != NULL ) {
	    for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
	        MOB_INDEX_DATA  *pMob;
	        OBJ_INDEX_DATA  *pObj;
		char buf[ MAX_STRING_LENGTH ];

		switch( pReset->command ) {
		case 'M':
		    if ( fAll || fMob ) {
			if ( fMob && ( pReset->arg1 != vnum ) && (vnum != 0 ))
			    break;
			found = TRUE;
			pMob = get_mob_index( pReset->arg1 );
			sprintf( buf, "M[%5d] %s in room %5d [ %3d/%3d ]\n\r",
				pReset->arg1, str_to_len(pMob->short_descr,23), 
				pReset->arg3, pReset->arg2, pReset->arg4 );
			strcat( outbuf, buf );
		    }
		break;

		case 'O':
		    if ( fAll || fObj ) {
			if ( fObj && ( pReset->arg1 != vnum ) && (vnum != 0) )
			    break;
			found = TRUE;
			pObj = get_obj_index( pReset->arg1 );
			sprintf( buf, "O[%5d] %s in room %5d [ %3d/    ]\n\r",
				pReset->arg1, str_to_len(pObj->short_descr,23), 
				pRoom->vnum, pReset->arg2 );
			strcat( outbuf, buf );
		    }
		break;

		case 'P':
		    if ( fAll || fObj ) {
			if ( fObj && ( pReset->arg1 != vnum ) && (vnum != 0) )
			    break;
			found = TRUE;
			pObj = get_obj_index( pReset->arg1 );
			sprintf( buf, "O[%5d] %s in obj  %5d [ %3d/%3d ]\n\r",
				pReset->arg1, str_to_len(pObj->short_descr,23),
				pRoom->vnum, pReset->arg2, pReset->arg4 );
			strcat( outbuf, buf );
		    }
		break;

		case 'G':
		case 'E':
		    if ( fAll || fObj ) {
			if ( fObj && ( pReset->arg1 != vnum ) && (vnum != 0) )
			    break;
			found = TRUE;
			pObj = get_obj_index( pReset->arg1 );
			sprintf( buf, "O[%5d] %s mob inv %5d [ %3d/    ]\n\r",
				pReset->arg1, str_to_len(pObj->short_descr,23),
				pRoom->vnum, pReset->arg2 );
			strcat( outbuf, buf );
		    }
		break;   
		default:
		    break; /* ignore all other resets ( D, R ) */
		}
	    }
	}
    }

    if ( !found ) {
	send_to_char( "No reset(s) found.\n\r", ch );
	return;
    }

    page_to_char( outbuf, ch );
    return;
}
/*
From: tchaerlach@aol.com

Try using these two functions, skillstat and spellstat, I think this is what
you have been talking about so you can check skills and spells of someone
without having to snoop and force them...
*/
// Modified by SinaC 2000
void do_skillstat(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char skill_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char skill_columns[LEVEL_HERO + 1];
  int sn, level, min_lev = 0, max_lev = LEVEL_HERO;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  // Added by SinaC 2000
  int pra;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
      send_to_char( "List skills for whom?\n\r", ch );
      return;
    }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
      send_to_char( "They aren't here.\n\r", ch );
      return;
    }

/*
  if (IS_NPC(victim)) {
      send_to_char( "Use this for skills on players.\n\r", ch );
      return;
    }
*/
/* Removed by SinaC 2001
  if ( victim->cstat(classes) == 0 ) {
    send_to_char("No skills found.\n\r", ch );
    return;
  }
*/

  /* initialize data */
  for (level = 0; level < LEVEL_HERO + 1; level++) {
      skill_columns[level] = 0;
      skill_list[level][0] = '\0';
    }

  for (sn = 0; sn < MAX_ABILITY; sn++) {
      if (ability_table[sn].name == NULL )
        break;

      // Modified by SinaC 2000 + clan_skill, modified again by SinaC 2000
      // again by SinaC 2001 for god skill
      if ( get_raceability( victim, sn ) 
	   || get_clanability( victim, sn ) 
	   /*|| get_godskill( victim, sn )   removed by SinaC 2003*/){
	level = 0;
	pra = 100;
      }
      else {
	// Modified by SinaC 2001
	level = class_abilitylevel( /*victim->cstat(classes)*/victim,sn);
	//pra = victim->pcdata->skill_info[sn].learned;
	pra = get_ability_simple( victim, sn );
      }

      if (level < LEVEL_HERO + 1
	  &&  level >= min_lev && level <= max_lev
	  // Modified by SinaC 2001
	  //&&  (ability_table[sn].spell_fun == spell_null)
	  && ability_table[sn].type == TYPE_SKILL
	  // Modified by SinaC 2000
	  &&   /*victim->pcdata->learned[sn]*/pra > 0) {
	  found = TRUE;
	  //don't need that SinaC 2000
	  //	  level = ability_table[sn].ability_level[victim->class];
	  if (victim->level < level)
	    sprintf(buf,"%-20s n/a       ", ability_table[sn].name);
	  else {
	    int casting_level = get_casting_level( victim, sn );
	    if ( casting_level > 0 )
	      sprintf(buf,"%-20s %3d%% (%2d) ",ability_table[sn].name,
		      // Modified by SinaC 2000
		      /*victim->pcdata->learned[sn]*/pra,
		      casting_level );
	    else
	      sprintf(buf,"%-20s %3d%%      ",ability_table[sn].name,
		      // Modified by SinaC 2000
		      /*victim->pcdata->learned[sn]*/pra );
	  }

	  if (skill_list[level][0] == '\0')
	    sprintf(skill_list[level],"\n\rLevel %2d: %s",level,buf);
	  else /* append */ {
	    if ( ++skill_columns[level] % 2 == 0)
	      strcat(skill_list[level],"\n\r          ");
	    strcat(skill_list[level],buf);
	  }
      }
  }

  /* return results */

  if (!found) {
    send_to_char("No skills found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level < LEVEL_HERO + 1; level++)
    if (skill_list[level][0] != '\0')
      add_buf(buffer,skill_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}

void do_spellstat(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char buff[100];
  char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char spell_columns[LEVEL_HERO + 1];
  int sn, gn, col, level, min_lev = 0, max_lev = LEVEL_HERO, mana;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int pra;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_char( "List spells for whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

/*
  if (IS_NPC(victim)) {
    send_to_char( "Use this for skills on players.\n\r", ch );
    return;
  }
*/
/* Removed by SinaC 2001
  if ( victim->cstat(classes) == 0 ) {
    send_to_char("No spells found.\n\r", ch );
    return;
  }
*/

  /* initialize data */
  for (level = 0; level < LEVEL_HERO + 1; level++) {
    spell_columns[level] = 0;
    spell_list[level][0] = '\0';
  }

  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;

    // Modified by SinaC 2000 + clan skill, modified again by SinaC 2000
    // again by SinaC 2001 for god skill
    if ( get_raceability( victim, sn ) 
	 || get_clanability( victim, sn ) 
	 /*|| get_godskill( victim, sn )   removed by SinaC 2003*/){
      level = 0;
      pra = 100;
    }
    else{
      // Modified by SinaC 2001
      level = class_abilitylevel( /*victim->cstat(classes)*/victim,sn);
      //pra = victim->pcdata->skill_info[sn].learned;
      pra = get_ability_simple( victim, sn );
    }

    if (level < LEVEL_HERO+ 1
	&&  level >= min_lev && level <= max_lev
	//&&  ability_table[sn].spell_fun != spell_null
	// Added by SinaC 2001
	&&  ability_table[sn].type == TYPE_SPELL
	// Modified by SinaC 2000
	&&  /*victim->pcdata->learned[sn]*/pra > 0) {
      found = TRUE;
      //don't need that SinaC 2000
      //	  level = ability_table[sn].ability_level[victim->class];

      if (victim->level < level)
	sprintf(buf,"%-20s n/a      ", ability_table[sn].name);
      else {
	mana = UMAX(//ability_table[sn].min_mana,
		    ability_table[sn].min_cost,
		    100/(2 + victim->level - level));
	int casting_level = get_casting_level( victim, sn );
	if ( casting_level > 0 )
	  sprintf(buf,"%-20s  %3d (%2d) ",ability_table[sn].name,
		  // Modified by SinaC 2000
		  /*victim->pcdata->learned[sn]*/mana,
		  casting_level );
	else
	  sprintf(buf,"%-20s  %3d mana  ",ability_table[sn].name,mana);
      }

      if (spell_list[level][0] == '\0')
	sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
      else /* append */ {
	if ( ++spell_columns[level] % 2 == 0)
	  strcat(spell_list[level],"\n\r          ");
	strcat(spell_list[level],buf);
      }
    }
  }

  /* return results */

  if (!found) {
    send_to_char("No spells found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level < LEVEL_HERO + 1; level++)
    if (spell_list[level][0] != '\0')
      add_buf(buffer,spell_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}

// Added by SinaC 2001 for mental user
void do_powerstat(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char buff[100];
  char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char spell_columns[LEVEL_HERO + 1];
  int sn, gn, col, level, min_lev = 0, max_lev = LEVEL_HERO, psp;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int pra;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_char( "List powers for whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

/*
  if (IS_NPC(victim)) {
    send_to_char( "Use this for skills on players.\n\r", ch );
    return;
  }
*/
/* Removed by SinaC 2001
  if ( victim->cstat(classes) == 0 ) {
    send_to_char("No powers found.\n\r", ch );
    return;
  }
*/

  /* initialize data */
  for (level = 0; level < LEVEL_HERO + 1; level++) {
    spell_columns[level] = 0;
    spell_list[level][0] = '\0';
  }

  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;

    // Modified by SinaC 2000 + clan skill, modified again by SinaC 2000
    // again by SinaC 2001 for god skill
    if ( get_raceability( victim, sn ) 
	 || get_clanability( victim, sn ) 
	 /*|| get_godskill( victim, sn )   removed by SinaC 2003*/){
      level = 0;
      pra = 100;
    }
    else{
      // Modified by SinaC 2001
      level = class_abilitylevel( /*victim->cstat(classes)*/victim,sn);
      //pra = victim->pcdata->skill_info[sn].learned;
      pra = get_ability_simple( victim, sn );
    }

    if (level < LEVEL_HERO+ 1
	&&  level >= min_lev && level <= max_lev
	//&&  ability_table[sn].spell_fun != spell_null
	// Added by SinaC 2001
	&&  ability_table[sn].type == TYPE_POWER
	// Modified by SinaC 2000
	&&  /*victim->pcdata->learned[sn]*/pra > 0) {
      found = TRUE;
      //don't need that SinaC 2000
      //	  level = ability_table[sn].ability_level[victim->class];

      if (victim->level < level)
	sprintf(buf,"%-20s n/a      ", ability_table[sn].name);
      else {
	psp = UMAX(//ability_table[sn].min_mana,
		   ability_table[sn].min_cost,
		   100/(2 + victim->level - level));
	int casting_level = get_casting_level( victim, sn );
	if ( casting_level > 0 )
	  sprintf(buf,"%-20s %3d%% (%2d) ",ability_table[sn].name,
		  // Modified by SinaC 2000
		  /*victim->pcdata->learned[sn]*/psp,
		  casting_level );
	else
	  sprintf(buf,"%-20s  %3d psp   ",ability_table[sn].name,psp);
      }

      if (spell_list[level][0] == '\0')
	sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
      else /* append */ {
	if ( ++spell_columns[level] % 2 == 0)
	  strcat(spell_list[level],"\n\r          ");
	strcat(spell_list[level],buf);
      }
    }
  }

  /* return results */

  if (!found) {
    send_to_char("No powers found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level < LEVEL_HERO + 1; level++)
    if (spell_list[level][0] != '\0')
      add_buf(buffer,spell_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}

// Added by SinaC 2003
void do_songstat(CHAR_DATA *ch, const char *argument) {
  BUFFER *buffer;
  char buff[100];
  char spell_list[LEVEL_HERO + 1][MAX_STRING_LENGTH];
  char spell_columns[LEVEL_HERO + 1];
  int sn, gn, col, level, min_lev = 0, max_lev = LEVEL_HERO, mana;
  bool found = FALSE;
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int pra;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_char( "List spells for whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  /* initialize data */
  for (level = 0; level < LEVEL_HERO + 1; level++) {
    spell_columns[level] = 0;
    spell_list[level][0] = '\0';
  }

  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;

    if ( get_raceability( victim, sn ) 
	 || get_clanability( victim, sn ) 
	 /*|| get_godskill( victim, sn )   removed by SinaC 2003*/){
      level = 0;
      pra = 100;
    }
    else{
      level = class_abilitylevel( victim,sn);
      pra = get_ability_simple( victim, sn );
    }

    if (level < LEVEL_HERO+ 1
	&&  level >= min_lev && level <= max_lev
	&&  ability_table[sn].type == TYPE_SONG
	&&  pra > 0) {
      found = TRUE;

      if (victim->level < level)
	sprintf(buf,"%-20s n/a      ", ability_table[sn].name);
      else {
	mana = UMAX(ability_table[sn].min_cost,
		    100/(2 + victim->level - level));
	sprintf(buf,"%-20s  %3d mana  ",ability_table[sn].name,mana);
      }

      if (spell_list[level][0] == '\0')
	sprintf(spell_list[level],"\n\rLevel %2d: %s",level,buf);
      else /* append */ {
	if ( ++spell_columns[level] % 2 == 0)
	  strcat(spell_list[level],"\n\r          ");
	strcat(spell_list[level],buf);
      }
    }
  }

  /* return results */
  if (!found) {
    send_to_char("No songs found.\n\r",ch);
    return;
  }

  buffer = new_buf();
  for (level = 0; level < LEVEL_HERO + 1; level++)
    if (spell_list[level][0] != '\0')
      add_buf(buffer,spell_list[level]);
  add_buf(buffer,"\n\r");
  page_to_char(buf_string(buffer),ch);
}



void do_groupstat( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_char( "List groups for whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_world( ch, argument ) ) == NULL ) {
    send_to_char( "They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char("Mobiles don't have groups.\n\r",ch);
    return;
  }

  int col = 0;

  for (int gn = 0; gn < MAX_GROUP; gn++) {
    if (group_table[gn].name == NULL)
      break;
    if (victim->pcdata->group_known[gn]) {
      sprintf(buf,"%-20s ",group_table[gn].name);
      send_to_char(buf,ch);
      if (++col % 3 == 0)
	send_to_char("\n\r",ch);
    }
  }
  if ( col % 3 != 0 ) {
    send_to_char( "\n\r", ch );
    sprintf(buf,"Creation points: %d\n\r",victim->pcdata->points);
    send_to_char(buf,ch);
  }
}

/* Version 1.0
FREE VNUM LISTING by The Mage (c) 1998
This routine I places in act_wiz.c for my builders.  It simply lists
all free vnums in an area based on mob, obj, or room.

The file is released under the GNU license. Feel free to use it.  Give
me credit if you want.

The Mage IMP of The Mage's Lair (lordmage.erols.com port 4000)

NOTE: This assumes you know how to place things in interp.c and interp.h
and it uses Lope's Colour code.  
*/
/* show a list of all used AreaVNUMS */
/* By The Mage */

#define MAX_SHOW_VNUM   310 /* show only 1 - 100*100 */

#define COLUMNS 		5   /* number of columns */
#define MAX_ROW 		((MAX_SHOW_VNUM / COLUMNS)+1) /* rows */

void do_fvlist (CHAR_DATA *ch, const char *argument)
{
  int i,j;
  char arg[MAX_INPUT_LENGTH];
  const char *string;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  string = one_argument(argument,arg);
  
  if (arg[0] == '\0') {
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  fvlist obj\n\r",ch);
    send_to_char("  fvlist mob\n\r",ch);
    send_to_char("  fvlist room\n\r",ch);
    return;
  }
  j=1;
  if (!str_cmp(arg,"obj")) {
    send_to_charf(ch,"{WFree {C%s{W vnum listing for area {C%s{x\n\r",arg,
		  ch->in_room->area->name);
    send_to_charf(ch,"{Y=============================================================================={C\n\r");
    for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
      if (get_obj_index(i) == NULL) {
	send_to_charf(ch,"%8d, ",i);
	if (j == COLUMNS) {
	  send_to_char("\n\r",ch);
	  j=0;
	}
	j++;
      }
    }
    send_to_char("{x\n\r",ch);
    return;
  }
  
  if (!str_cmp(arg,"mob")) { 
    send_to_charf(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
		  ch->in_room->area->name);
    send_to_charf(ch,"{Y=============================================================================={C\n\r");
    for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
      if (get_mob_index(i) == NULL) {
	send_to_charf(ch,"%8d, ",i);
	if (j == COLUMNS) {
	  send_to_char("\n\r",ch);
	  j=0;
	}
	else j++;
      }
    }
    send_to_char("{x\n\r",ch);
    return;
  }
  if (!str_cmp(arg,"room")) { 
    send_to_charf(ch,"{WFree {C%s {Wvnum listing for area {C%s{x\n\r",arg,
		  ch->in_room->area->name);
    send_to_charf(ch,"{Y=============================================================================={C\n\r");
    for (i = ch->in_room->area->min_vnum; i <= ch->in_room->area->max_vnum; i++) {
      if (get_room_index(i) == NULL) {
	send_to_charf(ch,"%8d, ",i);
	if (j == COLUMNS) {
	  send_to_char("\n\r",ch);
	  j=0;
	}
	else j++;
      }
    }
    send_to_char("{x\n\r",ch);
    return;
  }
  send_to_char("Syntax:\n\r",ch);
  send_to_char("  fvlist obj\n\r",ch);
  send_to_char("  fvlist mob\n\r",ch);
  send_to_char("  fvlist room\n\r",ch);
}

// Added by SinaC 2000
void do_immtitle( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  // Modified by SinaC 2001  int  before
  unsigned int i, j;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  //  smash_tilde( argument );
      
  if (argument[0] == '\0'){
    if ( ch->pcdata->immtitle == NULL || ch->pcdata->immtitle[0]=='\0')
      sprintf(buf,"You don't have an immtitle.\n\r");
    else
      sprintf(buf,"Your immtitle is %s\n\r",ch->pcdata->immtitle);
    send_to_char(buf,ch);
    return;
  }
      
  // j gives the strlen(argument) without any color code
  j = 0;
  for ( i = 0; i < strlen( argument ); i++ )
    if ( argument[i]=='{' ) {
      if ( i+1 < strlen(argument) )
	if ( argument[i+1] == '{' )
	  j++;
      i++;
    }
    else j++;
      
  if ( j > 14 ){
    send_to_char("Your immtitle is restricted to 14 characters maximum excluding the code color.\n\r", ch );
    return;
  }
      
  // Now, we center 'argument' in 'buf', so we add enough spaces
  j = 14 - j;
      
  sprintf(buf,"{x");
  for ( i = 0; i < (j+1)/2; i++ )
    strcat(buf," ");
  strcat(buf,argument);
  for ( i = 0; i < j/2; i++ )
    strcat(buf," ");
      
  ch->pcdata->immtitle = str_dup( buf );
      
  sprintf(buf,"Your immtitle is now %s\n\r",ch->pcdata->immtitle );
  send_to_char(buf,ch);
  return;
}

/* This meager bit of code is, oddly enough, copyrighted by 
 * Ferric of MelmothMUD.  Feel free to use it to your hearts content;
 * it's something you could have come up on your own easily enough.
 * If you do use this, or the idea, drop me a line at ferric@uwyo.edu,
 * just so I know my contributions (however small :) are worth something!
 * Enhanced by Dennis Reichel
 */
void do_idle(CHAR_DATA *ch, const char *argument)
{

  CHAR_DATA *vch;
  char	buf[MAX_INPUT_LENGTH];
  char	status[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  send_to_char("Players     Idle  Hours  HpL Position  Status  Host\n\r"
	       "-----------------------------------------------------------\n\r",ch);
  for(vch=char_list; vch != NULL; vch = vch->next){         
    if ( IS_NPC( vch ) || !can_see( ch, vch ) )
      continue;
	 
    if ( vch->desc && vch->desc->editor )
      sprintf( status, olc_ed_name( vch ) ); 
    //else    
    //  sprintf( status, vch->pcdata->countdown > 0 ? "Quest" :"" );
      
    sprintf(buf,"%-12s%4d%7d%5.1f %-10s%-8s%-30.30s\n\r",vch->name, vch->timer,
	    ( vch->played + (int) (current_time - vch->logon) ) / 3600, 
	    (float)( vch->played + (int) (current_time - vch->logon) )/(3600*vch->level), 
	    flag_string(position_flags, vch->position), status,
	    vch->desc ? vch->desc->host : "No descriptor." );
    send_to_char(buf,ch);
  }
  if (number_percent() == 1 ) 
    send_to_char( "You have become better at idleness!\n\r", ch );
  send_to_char( "\n\r", ch );
}

/********************************************************
 * Confiscate code by Keridan of Benevolent Iniquity    *
 * This code is released under no license and is freely *
 * distributable. I require no credit, but would        *
 * appreciate a quick note to keridan@exile.mudsrus.com *
 * if you choose to use it in your mud.                 *
 *       http://stargazer.inetsolve.com/~keridan        *
 *            stargazer.inetsolve.com 1414              *
 ********************************************************/
void do_confiscate(CHAR_DATA *ch,const char *argument)
{
  CHAR_DATA *victim;
  OBJ_DATA *obj; 
  char arg1[MAX_INPUT_LENGTH];
  bool found = FALSE;
  
  argument = one_argument(argument,arg1);

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ((arg1[0] == '0') || (argument[0] == '\0')){ 
    send_to_char("Syntax:\n\r",ch);
    send_to_char("  grab <char> <item>\n\r",ch);
    return; 
  }
  
  if ((victim = get_char_world(ch,arg1)) == NULL){ 
    send_to_char("They aren't here.\n\r",ch);
    return; 
  }
  
  if (IS_NPC(victim)){ 
    send_to_char("They aren't here.\n\r",ch);
    return; 
  }
  
  if ((victim->level >= ch->level)/* && !IS_OWNER(ch) */){ 
    send_to_char("They are too high level for you to do that.\n\r",ch);
    return; 
  }
  
  for ( obj = victim->carrying; obj != NULL; obj = obj->next_content ){
    if ( is_name( argument, obj->name )
	 &&   can_see_obj( ch, obj )){
      found = TRUE;
      break;
    }
  }
  
  if (!found){ 
    send_to_charf(ch,"They don't have %s.\n\r",argument);
    return; 
  }
  
  obj_from_char( obj );  
  obj_to_char( obj, ch );

  // Added by SinaC 2001
  //recompute(victim); NO NEED: done by obj_from_char -> unequip_char (unequipped items doesn't give affect)

  send_to_charf(ch,"Got it.\n\r");
  send_to_charf(victim,"%s has grabbed %s from you.\n\r",PERS(ch,victim),obj->short_descr);
  return;
}

/*
  wrlist m 3000 3001
  3000   wizard              
  3001   baker               
  
  wrlist o 3000 3001
  3000   barrel beer         
  3001   bottle beer         
  
  wrlist r 3000 3001
  3001   The Temple of Midgaard
  Resets: M = mobile, R = room, O = object, P = pet, S = shopkeeper
  No.  Loads    Description       Location         Vnum   Mx Mn
  Description
  ==== ======== ============= =================== ======== =====
  ===========
  [ 1] M[ 3011] Hassan        in room             R[ 3001]  1- 1 The
  Temple of M
  [ 2] O[ 3005] Hassan's scim wielded             M[ 3011]      
  Hassan         
*/
// Added and modified by SinaC 2000
void do_wrlist( CHAR_DATA *ch, const char *argument )
{
  ROOM_INDEX_DATA *room;
  ROOM_INDEX_DATA *in_room;
  MOB_INDEX_DATA *mob;
  OBJ_INDEX_DATA *obj;
  char arg[MAX_STRING_LENGTH];
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  int uvnum;
  int lvnum;
  int MR = 60000;
  int type = -1;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
 
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  uvnum = ( is_number( arg2 ) ) ? atoi( arg2 ) : 0;
  lvnum = ( is_number( arg1 ) ) ? atoi( arg1 ) : 0;  

  if ( !str_prefix( arg, "o" ) )
    type = 2;
  else if ( !str_prefix( arg, "m" ) )
    type = 1;
  else if ( !str_prefix( arg, "r" ) )
    type = 0;

  if ( ( uvnum - lvnum ) > 200 ){
    send_to_char( "That range is too large.\n\r", ch );
    return;
  }
  
  if ( ( uvnum == 0 && lvnum == 0 ) 
       || ( arg[0] == '\0' ) 
       || ( type == -1 ) ){
    send_to_char( "Syntax:\n\r", ch ); 
    send_to_char( "    wrlist [type] [lvnum] [uvnum]\n\r", ch );
    send_to_char( "  type is o(bject), m(obile), r(oom)\n\r", ch );
    return;
  }

  if ( uvnum > MR || uvnum < 1 || lvnum > MR || lvnum < 1 || lvnum > uvnum ){
    send_to_char( "Invalid vnum(s).\n\r", ch );
    return;
  }

  in_room = ch->in_room;  
  if ( type == 0 )
    char_from_room( ch );

  for ( MR = lvnum; MR <= uvnum; MR++ ){
    switch ( type ){
    case 0:
      if ( ( room = get_room_index( MR ) ) ){
	sprintf( log_buf, "{R%-5d  {w%-20s\n\r", room->vnum, room->name );
	send_to_char( log_buf, ch );
	char_to_room( ch, room );
	do_resets( ch, "" );
	char_from_room( ch );
      }
      break;
    case 2:
      if ( ( obj = get_obj_index( MR ) ) ){
	sprintf( log_buf, "{R%-5d  {w%-20s\n\r",  obj->vnum, obj->name );
	send_to_char( log_buf, ch );
      }
      break;
    case 1:
      if ( ( mob = get_mob_index( MR ) ) ){
	sprintf( log_buf, "{R%-5d  {w%-20s\n\r", mob->vnum, mob->player_name );
	send_to_char( log_buf, ch );
      }
      break;
    }
  }
  if ( type == 0 )
    char_to_room( ch, in_room );
  return;
}

void do_owherevnum( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  OBJ_DATA *in_obj;
  char buf[MAX_STRING_LENGTH];
  int vnum;
  BUFFER *buffer;
  bool found;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if (argument[0] == '\0'){
    send_to_char("Find what?\n\r",ch);
    return;
  }
  
  if ( !is_number( argument ) ){
    send_to_char("Argument must be a non-negative number\n\r", ch );
    return;
  }
  vnum = atoi( argument );
  
  buffer = new_buf();

  found = FALSE;
  for ( obj = object_list; obj != NULL; obj = obj->next ){
    if ( obj->pIndexData->vnum != vnum )
      continue;
    
    found = TRUE;
    
    for ( in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj );
    
    if ( in_obj->carried_by != NULL && in_obj->carried_by->in_room != NULL)
      sprintf( buf, "%s is carried by %s [Room %d]\n\r",
	       obj->short_descr,PERS(in_obj->carried_by, ch),
	       in_obj->carried_by->in_room->vnum );
    else if (in_obj->in_room != NULL && can_see_room(ch,in_obj->in_room))
      sprintf( buf, "%s is in %s [Room %d]\n\r",
	       obj->short_descr,in_obj->in_room->name, 
	       in_obj->in_room->vnum);
    else
      sprintf( buf, "%s is somewhere\n\r",obj->short_descr);
    
    buf[0] = UPPER(buf[0]);
    add_buf(buffer,buf);
  }
  
  if (!found)
    send_to_char("No objects with that vnum found.\n\r",ch);
  else
    page_to_char(buf_string(buffer),ch);
}

void do_mwherevnum( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  BUFFER *buffer;
  CHAR_DATA *victim;
  bool found;
  int vnum;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ){
    send_to_char("Find what ?\n\r",ch);
    return;
  }
  
  if ( !is_number( argument ) ){
    send_to_char("Argument must be a non-negative number\n\r", ch );
    return;
  }
  vnum = atoi( argument );

  found = FALSE;
  buffer = new_buf();
  for ( victim = char_list; victim != NULL; victim = victim->next ){
    if ( !IS_NPC( victim ) ) continue;
    
    if ( victim->in_room != NULL
	 &&   victim->pIndexData->vnum == vnum ){
      found = TRUE;
      sprintf( buf, "[%5d] %-28s [%5d] %s\n\r",
	       victim->pIndexData->vnum,
	       victim->short_descr,
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

void do_quake( CHAR_DATA *ch, const char *argument )
{  
  int value;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  if ( argument[0] == '\0' ){
    send_to_char("{gUsage: quake <duration>{x\n\r",ch);
    return;
  }
  
  if ( ch->in_room == NULL ){
    send_to_char("You can't start an earthquake, you're nowhere!\n\r",ch);
    return;
  }
  
  if (!is_number(argument)){
    send_to_char("Enter a numerical argument.\n\r",ch);
    return;
  }
  
  value = atoi(argument);
  if ( value <= 0 ) {
    send_to_char("Please, be serious.\n\r",ch);
    return;
  }
  
  send_to_char("{YAs you wave your hands, the ground begins to shake!{x\n\r",ch);
  
  sprintf(log_buf,"Manual earthquake has been detected in %s.",ch->in_room->area->name);
  log_string(log_buf);

  ch->in_room->area->earthquake_on = TRUE;
  ch->in_room->area->earthquake_duration = value;

  return;
}

void do_bighunt( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *vch, *ch_next;
  CHAR_DATA *victim;
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH]; // hidden argument ;)))
  int mlvl, Mlvl, chance, count;
  bool found;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );
  argument = one_argument( argument, arg4 );
  
  if ( arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0'
       || !is_number(arg2) || !is_number(arg3) ){
    send_to_char("Syntax:\n\r"
		 "   bighunt <target> <min level> <max level>\n\r"
		 ,ch);
    return;
  }

  victim = get_char_world( ch, arg1 );
  if ( victim == NULL || IS_NPC( victim ) ){
    send_to_charf(ch, "Invalid target: %s\n\r",arg1);
    return;
  }
  
  mlvl = atoi(arg2);
  if ( mlvl <= 0 || mlvl >= 100 ){
    send_to_char("Invalid min level, range is from 1 to 99!\n\r",ch);
    return;
  }
  
  Mlvl = atoi(arg3);
  if ( Mlvl <= 0 || Mlvl >= 100 ){
    send_to_char("Invalid max level, range is from 1 to 99!\n\r",ch);
    return;
  }
  
  chance = atoi(arg4);
  if ( chance <= 0 || chance >= 500 )
    chance = 100;

  if ( mlvl >= Mlvl ){
    send_to_char("min level must be strictly inferior to max level!\n\r",ch);
    return;
  }
  
  found = FALSE;
  count = 0;
  for (vch = char_list ; vch != NULL; vch = ch_next ){
    ch_next = vch->next;
    
    if ( IS_NPC( vch )
	 && vch->level >= mlvl && vch->level <= Mlvl  // chance for mob starts hunting
	 && number_range(0,chance) == 0
	 && vch->in_room != NULL
	 // Modified by SinaC 2001
	 && !IS_SET(vch->in_room->cstat(flags),ROOM_PET_SHOP)
	 && !IS_SET(vch->in_room->cstat(flags),ROOM_SAFE)
	 && !IS_SET(vch->in_room->cstat(flags),ROOM_BANK)
	 && vch->pIndexData->pShop == NULL
         && !IS_SET(vch->act,ACT_TRAIN)
	 && !IS_SET(vch->act,ACT_PRACTICE)
	 && !IS_SET(vch->act,ACT_IS_HEALER)
	 //&& !IS_SET(vch->act,ACT_IS_CHANGER)
         && !IS_SET(vch->act,ACT_RESERVED)
	 // neither questor  SinaC 2000
	 //&& vch->spec_fun != spec_lookup( "spec_questmaster" )
	 && !IS_SET(vch->act,ACT_PET)
	 // Added by SinaC 2001
	 && !IS_SET(vch->act,ACT_IS_SAFE)
	 && !IS_AFFECTED(vch,AFF_CHARM)){
      found = TRUE;
      log_stringf("%s vnum: %d  (room: %d)",
		  vch->short_descr,
		  vch->pIndexData->vnum,
		  vch->in_room==NULL?-1:vch->in_room->vnum
		  );
      
      vch->hunting = victim;
      count++;
    }
  }
  if ( found ){
    send_to_charf(ch,"{MEveryone on %s!{x\n\r",victim->name);
    send_to_charf(victim,
		  "{MAlmost every mobile from lvl %2d to %2d will now hunt you!\n\r"
		  "%d mobiles will hunt you.{x\n\r",
		  mlvl, Mlvl,
		  count );
  }
  else
    send_to_charf(ch,"{MThere is any mobiles who wants to play with %s!{x\n\r",victim->name);
}

void afremove_syntax( CHAR_DATA *ch ) {
  send_to_char("Syntax:\n\r"
	       "  afremove   without any argument will remove your affects\n\r"
	       "  afremove char <char name>    remove affects of target player/mob\n\r"
	       "  afremove obj  <obj name>     remove affects of target object\n\r"
	       "  afremove room [<room vnum>]  remove affects of target room\n\r"
	       "   room vnum  is optional, default is current room\n\r",
	       ch);
}

void char_afremove( CHAR_DATA *ch, CHAR_DATA *victim ) {
  while ( victim->affected )
    affect_remove( victim, victim->affected );
  
  act("$N has lost all $S affects.", ch, NULL, victim, TO_NOTVICT );
  if ( ch != victim ){
    send_to_char("You have lost all your affects.\n\r",victim);
    act("You have removed all affects of $N.", ch, NULL, victim, TO_CHAR );
  }
  else 
    send_to_char("You have removed all your affects.\n\r",ch);
}

void obj_afremove( CHAR_DATA *ch, OBJ_DATA *obj ) {
  while ( obj->affected )
    affect_remove_obj( obj, obj->affected );
  
  act("$P has lost all its affects.", ch, NULL, obj, TO_ROOM );
  act("You have removed all affects of $P.", ch, NULL, obj, TO_CHAR );
}

void room_afremove( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) {
  while ( room->current_affected )
    affect_remove_room( room, room->current_affected );
  
  send_to_charf(ch,
		"Room (vnum %d) has lost all its affects.\n\r",
		room->vnum );
}

void do_afremove( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg1 );
  one_argument( argument, arg2 );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( arg1[0] == '\0' )
    char_afremove( ch, ch );
  else if ( !str_cmp( arg1, "char" ) ) {
    CHAR_DATA *vChar;
    vChar = get_char_world( ch, arg2 );
    if ( vChar == NULL ) {
      send_to_charf( ch,"You don't see %s.\n\r", arg2 );
      return;
    }
    char_afremove(ch,vChar);
  }
  else if ( !str_cmp( arg1, "obj" ) ) {
    OBJ_DATA *vObj;
    vObj = get_obj_world( ch, arg2 );
    if ( vObj == NULL ) {
      send_to_charf( ch,"You don't see any %s.\n\r", arg2 );
      return;
    }
    obj_afremove(ch,vObj);
  }
  else if ( !str_cmp( arg1, "room" ) ) {
    ROOM_INDEX_DATA *vRoom;
    int vnum = atoi( arg2 );
    if ( arg2[0] == '\0' )
      vnum = ch->in_room->vnum;
    if ( arg2 <= 0 ) {
      send_to_char("Invalid argument.\n\r", ch );
      return;
    }
    vRoom = get_room_index( vnum );
    if ( vRoom == NULL ) {
      send_to_charf( ch, "That room (vnum %s) doesn't exist.\n\r", arg2 );
      return;
    }
    room_afremove(ch,vRoom);
  }
  else
    afremove_syntax(ch);

  return;
}

// Allow an immortal to become mortal, SinaC 2000
void do_mortal( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC( ch ) ){
    send_to_char("Only immortals can use this command.\n\r",ch );
    return;
  }

  if ( IS_SET( ch->act, PLR_MORTAL ) ){
    REMOVE_BIT( ch->act, PLR_MORTAL );
    send_to_char("{RYOU ARE IMMORTAL AGAIN!...Ouf{x\n\r",ch);
  } 
  else {
    SET_BIT( ch->act, PLR_MORTAL );
    send_to_char("{RBEWARE: {GYOU ARE {RMORTAL{G NOW!{x\n\r",ch);
  }
  return;
}

// Show prereqs, SinaC 2000
void do_prereq( CHAR_DATA *ch, const char *argument ) {
  BUFFER *buffer;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  ability_type *sk;
  prereq_data *prereq;
  bool fAll, found;
  int sn=-1;

  argument = one_argument( argument, arg );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  fAll = arg[0] == '\0';
  if ( !fAll ){
    sn = ability_lookup( arg );
    if ( sn == -1 ){
      send_to_charf( ch,
		     "%s is a unknown skill/spell.\n\r",
		     arg );
      return;
    }
  }
  
  found = FALSE;
  buffer = new_buf();
  for ( int i = 0; i < MAX_ABILITY; i++ ){
    if ( ability_table[i].name == NULL ) break;
    if (!fAll && sn != i ) 
      continue;
    sk = &(ability_table[i]);
    if ( sk->nb_casting_level > 0 ){
      found = TRUE;
      sprintf( buf,
	       "%5s: %24s has %3d levels.\n\r", 
	       // Modified by SinaC 2001 for mental user
	       //sk->spell_fun==spell_null?"Skill":"Spell",
	       //sk->spell_fun==spell_null?"Skill":(sk->type == TYPE_SPELL?"Spell":"Power"),
	       abilitytype_name(sk->type),
	       sk->name,
	       sk->nb_casting_level );
      add_buf(buffer, buf);
    }
    if ( sk->prereqs == NULL && sk->nb_casting_level > 0 ) {
      found = TRUE;
      sprintf( buf, 
	       "  No prereqs.\n\r" );
      add_buf(buffer, buf);
    }
    if ( sk->prereqs != NULL && sk->nb_casting_level >= 0 ) {
      found = TRUE;
      for ( int j = 0; j < sk->nb_casting_level+1; j++ ){ // +1  SinaC 2003
	if ( j == 0 && sk->prereqs[j].nb_prereq == 0 )
	    continue;
	prereq = &(sk->prereqs[j]);
	sprintf( buf, 
		 "  Level %3d costs %3d trains, level %d, classes: %s.\n\r",
		 //prereq->casting_level, prereq->cost );
		 j, prereq->cost, prereq->plr_level, 
		 prereq->classes == ANY_CLASSES ? "Any":flag_string( classes_flags, prereq->classes ) ); // SinaC 2003
	add_buf(buffer, buf);
	if ( prereq->nb_prereq != 0 ){
	  sprintf( buf, 
		   "    Prereqs:\n\r" );
	  add_buf(buffer, buf);
	}
	else {
	  sprintf( buf, 
		   "    No prereqs.\n\r" );
	  add_buf(buffer, buf);
	}
	for ( int k = 0; k < prereq->nb_prereq; k++ ){
	  //int sn;
	  //sn = ability_lookup( prereq->prereq[k].name );
	  sn = prereq->prereq[k].sn;
	  sprintf( buf,
		   "%12s: %24s level %3d.\n\r", 
		   //ability_table[sn].spell_fun==spell_null?"Skill":"Spell",
		   abilitytype_name(ability_table[sn].type),
		   //prereq->prereq[k].name,
		   ability_table[prereq->prereq[k].sn].name,
		   prereq->prereq[k].casting_level );
	  add_buf(buffer, buf);
	}
      }
    }
  }
  if ( !found ){
    send_to_char("No prereqs found.\n\r", ch );
    return;
  }
  else {
    page_to_char(buf_string(buffer),ch);
  }
}

void do_oaffects( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];
  
  one_argument( argument, arg );
  if ( arg[0] == '\0' )
    victim = ch;
  else
    if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
      send_to_char("That player doesn't exist\n\r", ch );
      return;
    }

  show_oaffects( ch, victim, TRUE );
}

void do_godinfo( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  
  one_argument( argument, arg );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }
  
  bool fAll = FALSE;
  int whichOne = 0;
  if ( arg[0] == '\0' )
    fAll = TRUE;
  else {
    whichOne = god_lookup( arg );
    if ( whichOne < 0 ) {
      send_to_char("Invalid god.\n\r", ch );
      return;
    }
  }

  for ( int i = 0; i < MAX_GODS; i++ ) {
    // List every gods or just one ?
    if ( !fAll && whichOne != i )
      continue;

    god_data *god = &gods_table[i];
    send_to_charf(ch,"Name: %s, %s\n\r", god_name(i), god->title);
    send_to_charf(ch,"Story: %s",god->story);
    //send_to_charf(ch,"Skill: %s\n\r",god->sn!=-1?ability_table[god->sn].name:"none");  removed by SinaC 2003
    //send_to_charf(ch,"Brand: %d\n\r",god->brand_mark);
    send_to_charf(ch,"Align: \n\r");
    for ( int j = 0; j < god->nb_allowed_align; j++ )
      send_to_charf(ch,
		    "  [%s]",
		    etho_align_name(god->allowed_align[j].etho, 
				    god->allowed_align[j].alignment));
    send_to_char("\n\r",ch);
    send_to_charf(ch,"Races: \n\r");
    for ( int j = 0; j < god->nb_allowed_race; j++ )
      send_to_charf(ch,
		    "  [%s]",
		    race_table[god->allowed_race[j]].name);
    send_to_char("\n\r",ch);
    send_to_charf(ch,"Classes: \n\r");
    //    for ( int j = 0; j < god->nb_allowed_class; j++ )
    //      send_to_charf(ch,
    //		    "  [%s]",
    //		    class_name( god->allowed_class[j] ) );
    int allowed_class = god->allowed_class;
    for ( long j = 0; j < MAX_CLASS; j++ )
      if ( ( 1 << j ) & allowed_class )
	send_to_charf(ch,"  [%s]", class_table[j].name );
    send_to_char("\n\r",ch);
    send_to_charf(ch,"Minor: %s\n\r", god->minor_sphere>0?group_table[god->minor_sphere].name:"no minor sphere");
    send_to_char("Major: \n\r",ch);
    for ( int j = 0; j < god->nb_major_sphere; j++ )
      send_to_charf(ch,
		    "  [%s]",
		    group_table[god->major_sphere[j]].name );
    send_to_char("\n\r",ch);
    send_to_char("Class getting sphere(Priest): \n\r",ch);
    //for ( int j = 0; j < god->nb_priest; j++ )
    //  send_to_charf(ch,
    //		    "  [%s]",
    //		    class_name(god->priest[j]));
    int priest = god->priest;
    for ( long j = 0; j < MAX_CLASS; j++ )
      if ( ( 1 << j ) & priest )
	send_to_charf(ch,"  [%s]", class_table[j].name );
    send_to_char("\n\r",ch);
  }
}

// Show every skills/spells/powers/songs of a class, SinaC 2001
void do_checkskill( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int iClass;
  BUFFER *buffer;

  one_argument( argument, arg );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0') {
    send_to_char( "You must specify a class name!\n\r", ch );
    return;
  }

  if (  ( iClass = class_lookup( arg, TRUE ) ) == -1 ) {
    send_to_char( "Invalid class name!\n\r", ch );
    return;
  }

  buffer = new_buf();
  
  sprintf( buf, 
	   "Skills/Spells/Powers/Songs available for %s\n\r",
	   class_table[iClass].name );
  add_buf( buffer, buf );
  for ( int i = 1; i < MAX_ABILITY; i++ ) {
    if ( ability_table[i].name == NULL || 
	 ability_table[i].name[0] == '\0' )
      break;
    if ( ability_table[i].rating[iClass] != 0 
	 || ability_table[i].ability_level[iClass] < LEVEL_IMMORTAL ) {
      sprintf( buf,
	       "%5s: %23s  train: %2d  level: %3d\n\r",
	       //(ability_table[i].spell_fun==spell_null)?"skill":"spell",
	       abilitytype_name(ability_table[i].type),
	       ability_table[i].name,
	       ability_table[i].rating[iClass],
	       ability_table[i].ability_level[iClass]);
      add_buf(buffer, buf);
    }
  }
  page_to_char(buf_string(buffer),ch);
}

// Used to accept a name, SinaC 2001
void do_acceptname( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;

  one_argument( argument, arg );
  
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( arg[0] == '\0' ) {
    send_to_char("You must specify a player name!\n\r", ch );
    return;
  }
  
  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char("That player doesn't exist!\n\r", ch );
    return;
  }
  
  if ( IS_NPC( victim ) ) {
    send_to_char("Mobiles can't be accepted!\n\r", ch );
    return;
  }

  if ( victim->pcdata->name_accepted == TRUE ) {
    send_to_charf(ch,"%s is already accepted!\n\r", NAME(victim) );
    return;
  }

  send_to_charf(ch,"%s has been accepted!\n\r", NAME(victim) );
  log_stringf("ACCEPT: %s has been accepted by %s", 
	      NAME(victim), NAME(ch) );
  send_to_char("{CYour name has been accepted by the gods.{x\n\r", victim );
  sprintf( buf, "%s has been accepted!\n\r", NAME(victim) );
  wiznet( buf, NULL, NULL, WIZ_NAMEACCEPT, 0, 0 );
  victim->pcdata->name_accepted = TRUE;
}

bool dump_extra_fields( CHAR_DATA *ch, ENTITY_DATA *e ) {
  bool found = FALSE;
  for ( FIELD_DATA *f = e->ex_fields; f; f = f->next ) {
    found = TRUE;
    switch(f->var.typ) {
    case SVT_INT:
      send_to_charf(ch,
		    "name: %-15s  lifetime: %4d  value: [%ld] INT\n\r",
		    f->name, f->lifetime, f->var.asInt() );
      break;
    case SVT_STR:
      send_to_charf(ch,
		    "name: %-15s  lifetime: %4d  value: [%s] STR\n\r",
		    f->name, f->lifetime, f->var.asStr());
      break;
    case SVT_LIST:
      send_to_charf(ch,
		    "name: %-15s  lifetime: %4d  value: [%s] LIST\n\r",
		    f->name, f->lifetime, f->var.asStr());
      break;
    default:
      send_to_charf(ch,
		    "name: %-15s  lifetime: %4d  value: [%s] (typ %s)\n\r",
		    f->name, f->lifetime, f->var.asStr(), script_type_name( f->var.typ ) );
      break;
    }
  }
  return found;
}
void do_field( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *cvictim = NULL;
  OBJ_DATA *ovictim = NULL;
  ENTITY_DATA *e;
  char arg[MAX_INPUT_LENGTH];

  if ( argument[0] == '\0' ) {
    send_to_charf(ch,
		  "Extra fields for player %s (you)\n\r",
		  NAME(ch));
    e = ch;
  }
  else {
    argument = one_argument( argument, arg );

    if ( !str_cmp(arg,"char" ) || !str_cmp(arg,"mob") ) {
      if ( ( cvictim = get_char_world( ch, argument ) ) == NULL ) {
	send_to_char("They aren't here.\n\r", ch );
	return;
      }
      send_to_charf(ch,
		    "Extra fields for %s %s (vnum %d)\n\r",
		    NAME(cvictim),
		    IS_NPC(cvictim)?"mob":"player",
		    IS_NPC(cvictim)?cvictim->pIndexData->vnum:0);
      e = cvictim;
    } 
    else if ( !str_cmp(arg,"obj" ) ) {
      if ( ( ovictim = get_obj_here( ch, argument ) ) == NULL ) {
	send_to_char("It isn't here.", ch );
	return;
      }
      send_to_charf(ch,
		    "Extra fields for obj %s (vnum %d)\n\r",
		    ovictim->short_descr, 
		    ovictim->pIndexData->vnum);
      e = ovictim;
    }
    else if ( !str_cmp(arg,"room") ) {
      if ( argument[0] == '\0' )
	e = ch->in_room;
      else {
	ROOM_INDEX_DATA *room = get_room_index(atoi(argument));
	if ( room == NULL ) {
	  send_to_charf(ch,"Room %s doesn't exist.\n\r", argument );
	  return;
	}
	send_to_charf(ch, "Extra fields for room %d\n\r", room->vnum );
	e = room;
      }
    }
    else {
      send_to_char("Syntax: \n\r"
		   " field\n\r"
		   " field char <char name>\n\r"
		   " field mob  <mob  name>\n\r"
		   " field obj  <obj  name>\n\r"
		   " field room\n\r"
		   " field room <room vnum>\n\r", ch );
      return;
    }
  }

  bool found = dump_extra_fields( ch, e );
  if (!found)
    send_to_char("no extra fields.\n\r", ch );
}

void do_mstatvnum( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  AFFECT_DATA *paf;
  MOB_INDEX_DATA *victim;
  int vnum;

  if ( argument[0] == '\0' ) {
    send_to_char( "Mstatvnum <vnum>\n\r", ch );
    return;
  }

  vnum = atoi( argument );

  if ( vnum <= 0 ) {
    send_to_char("argument must be an integer.\n\r", ch );
    return;
  }

  if ( ( victim = get_mob_index( vnum ) ) == NULL ) {
    send_to_char("Invalid vnum.\n\r", ch );
    return;
  }

  sprintf( buf, "Name: %s\n\r",
	   victim->player_name);
  send_to_char( buf, ch );

  sprintf( buf, 
	   "Vnum: %d  Format: %s  Race: %s  Group: %d  Sex: %s\n\r",
	   victim->vnum,
	   victim->new_format ? "new" : "old",
	   race_table[victim->race].name,
	   victim->group, sex_table[victim->sex].name );
  send_to_char( buf, ch );

  sprintf(buf,"Count: %d  Killed: %d\n\r",
	  victim->count,victim->killed);
  send_to_char(buf,ch);

  // Added by SinaC 2003 for factions
  if (IS_NPC(victim))
    send_to_charf(ch,"Faction: %s\n\r", faction_table[victim->faction].name );


  // Modified by SinaC 2001 for mental user
  /*
  sprintf( buf, "Hp: %dd%d+%d  Mana: %dd%d+%d\n\r",
	   victim->hit[0],         victim->hit[1],         victim->hit[2],
	   victim->mana[0],        victim->mana[1],        victim->mana[2] );
  */
  sprintf( buf, "Hp: %dd%d+%d  Mana: %dd%d+%d  Psp: %dd%d+%d\n\r",
	   victim->hit[0],         victim->hit[1],         victim->hit[2],
	   victim->mana[0],        victim->mana[1],        victim->mana[2],
	   victim->psp[0],         victim->psp[1],         victim->psp[2] );

  send_to_char( buf, ch );
	
  sprintf( buf,
	   "Lv: %d Class: %s Etho: %s Align: %d Wealth: %ld\n\r",
	   victim->level,
	   victim->classes==0 ? "mobile" : class_name(victim->classes),
	   etho_name(victim->align.etho),
	   victim->align.alignment,
	   victim->wealth );
  send_to_char( buf, ch );
  
  if ( class_ismulti( victim->classes ) ){
    sprintf(buf, "Classes: ");
    long clbit = 1;
    for (int i = 0; i<MAX_CLASS;i++) {
      if (clbit & victim->classes) {
	char buf1[MAX_INPUT_LENGTH];
	sprintf(buf1," %s",class_table[i].name);
	strcat(buf,buf1);
      }
      clbit<<=1;
    }
    strcat(buf,"\n\r");
    send_to_char(buf,ch);
  }

  sprintf(buf,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n\r",
	  victim->ac[0], victim->ac[1],
	  victim->ac[2], victim->ac[3]);
  send_to_char(buf,ch);

  sprintf( buf,
	   "Hitroll: %d  Dam: %dd%d+%d  Start pos: %s  Default pos: %s\n\r",
	   victim->hitroll, victim->damage[0], victim->damage[1], victim->damage[2],
	   position_table[victim->start_pos].name,
	   position_table[victim->default_pos].name );
  send_to_char( buf, ch );

  sprintf(buf, "Act: %s\n\r",act_bit_name(victim->act));
  send_to_char(buf,ch);
    

  if (IS_NPC(victim) && victim->off_flags) {
    sprintf(buf, "Offense: %s\n\r",off_bit_name(victim->off_flags));
    send_to_char(buf,ch);
  }

  if (victim->imm_flags) {
    sprintf(buf, "Immune: %s\n\r",irv_bit_name(victim->imm_flags));
    send_to_char(buf,ch);
  }

  if (victim->res_flags) {
    sprintf(buf, "Resist: %s\n\r", irv_bit_name(victim->res_flags));
    send_to_char(buf,ch);
  }

  if (victim->vuln_flags) {
    sprintf(buf, "Vulnerable: %s\n\r", irv_bit_name(victim->vuln_flags));
    send_to_char(buf,ch);
  }

  sprintf(buf, "Form: %s\n\rParts: %s\n\r", 
	  form_bit_name(victim->form), part_bit_name(victim->parts));
  send_to_char(buf,ch);

  if (victim->affected_by) {
    sprintf(buf, "Affected by %s\n\r",
	    affect_bit_name(victim->affected_by));
    send_to_char(buf,ch);
  }
  // Added by SinaC 2001
  if (victim->affected2_by) {
    sprintf(buf, "Also affected by %s\n\r",
	    affect2_bit_name(victim->affected2_by));
    send_to_char(buf,ch);
  }

  // Added by SinaC 2001 for disease       Removed by SinaC 2003
  //send_to_charf(ch,"Disease: %s.\n\r", flag_string( disease_flags, victim->disease));

  sprintf( buf, "Short description: %s\n\rLong  description: %s",
	   victim->short_descr,
	   victim->long_descr[0] != '\0' ? victim->long_descr : "(none)\n\r" );
  send_to_char( buf, ch );

  if ( victim->spec_fun != 0 ) {
    sprintf(buf,"Mobile has special procedure %s.\n\r",
	    spec_name(victim->spec_fun));
    send_to_char(buf,ch);
  }

  if ( victim->program != NULL ) {
    send_to_charf(ch,"Program: %s\n\r", victim->program->name );
  }

  for ( paf = victim->affected; paf != NULL; paf = paf->next ) {
    afstring( buf, paf, ch, FALSE );
    send_to_char(buf,ch);
  }

  return;
}

void do_ostatvnum( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  char buf4[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  AFFECT_DATA *paf;
  OBJ_INDEX_DATA *obj;
  RESTR_DATA *restr;
  bool found;
  ABILITY_UPGRADE *upgr;
  int vnum;

  if ( argument[0] == '\0' ) {
    send_to_char( "Ostatvnum <vnum>\n\r", ch );
    return;
  }

  vnum = atoi( argument );

  if ( vnum <= 0 ) {
    send_to_char("argument must be an integer.\n\r", ch );
    return;
  }

  if ( ( obj = get_obj_index( vnum ) ) == NULL ) {
    send_to_char("Invalid vnum.\n\r", ch );
    return;
  }

  sprintf( buf, "Name(s): %s\n\r",
	   obj->name );
  send_to_char( buf, ch );

  sprintf( buf, "Vnum: %d  Format: %s  Type: %s  Resets: %d\n\r",
	   obj->vnum, obj->new_format ? "new" : "old",
	   item_name(obj->item_type), obj->reset_num );
  send_to_char( buf, ch );

  sprintf( buf, "Short description: %s\n\rLong description: %s\n\r",
	   obj->short_descr, obj->description );
  send_to_char( buf, ch );

  sprintf( buf, "Wear bits: %s\n\rExtra bits: %s\n\r",
	   wear_bit_name(obj->wear_flags), extra_bit_name( obj->extra_flags ) );
  send_to_char( buf, ch );

  sprintf( buf, "Level: %d  Cost: %d  Condition: %d\n\r",
	   obj->level, obj->cost, obj->condition );
  send_to_char( buf, ch );

  // Added by SinaC 2000, Modified by SinaC 2001
  sprintf( buf, "Material: %s\n\r", 
	   material_table[obj->material].name );
  send_to_char( buf, ch );

  // Added by SinaC 2003, size is not anymore a restriction but an obj stat
  sprintf( buf, "Size: %s\n\r", 
	   flag_string( size_flags, obj->size ) );
  send_to_char( buf, ch );
    
  sprintf( buf, "Values: %d %d %d %d %d\n\r",
	   obj->value[0], obj->value[1], obj->value[2], obj->value[3],
	   obj->value[4] );
  send_to_char( buf, ch );

  if ( obj->program != NULL ) {
    send_to_charf(ch,"Program: %s\n\r", obj->program->name );
  }
    
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
    if (obj->new_format)
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


  if ( obj->extra_descr != NULL ) {
    EXTRA_DESCR_DATA *ed;

    send_to_char( "Extra description keywords: ", ch );

    /* Modified by SinaC 2001
    for ( ed = obj->extra_descr; ed != NULL; ed = ed->next ) {
      send_to_char( ed->keyword, ch );
      if ( ed->next != NULL )
	send_to_char( " ", ch );
    }

    send_to_char( "'\n\r", ch );
    */
    for ( ed = obj->extra_descr; ed; ed = ed->next ) {
      send_to_char( "[", ch );
      send_to_char( ed->keyword, ch );
      send_to_char( "]", ch );
    }

    send_to_char( "\n\r", ch );
  }

  // Added by SinaC 2000 for object restrictions
  found = FALSE;
  for ( restr = obj->restriction; restr != NULL; restr = restr->next ){
    found = TRUE;
    restrstring( buf, restr );
    send_to_char( buf, ch );
  }
  
  if ( found )
    send_to_char("\n\r",ch );

  // Added by SinaC 2000 for skill/spell upgrade
  found = FALSE;
  for ( upgr = obj->upgrade; upgr != NULL; upgr = upgr->next ){
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

  return;
}

void do_findfollower( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  one_argument( argument, arg );

  int god;
  if ( arg[0] == '\0' ) {
    god = god_lookup( ch->name );
    if ( god < 0 ) {
      send_to_char("You are not a choosable god.\n\r", ch );
      return;
    }
  }
  else {
    god = god_lookup( arg );
    if ( god < 0 ) {
      send_to_char("%s is not a valid god.\n\r", ch );
      return;
    }
  }

  bool found = FALSE;
  int col = 0;
  send_to_charf(ch,"This is the list of %s online followers:\n\r",god_name(god));
  for ( CHAR_DATA *victim = char_list; victim != NULL; victim = victim->next )
    if ( !IS_NPC(victim)
	 && victim->pcdata->god == god ) {
      found = TRUE;
      send_to_charf( ch,
		     " %20s", NAME(victim));
      if (++col % 2 == 0)
	send_to_char("\n\r",ch);
    }
  if ( col % 2 != 0)
    send_to_char("\n\r",ch);
  if ( !found )
    send_to_charf(ch,"None is following %s.\n\r", god_name(god));
}

/* Removed by SinaC 2003
// Allow to brand a player
void do_brand( CHAR_DATA *ch, const char *argument ) {
  CHAR_DATA *victim;
  char arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch)) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  one_argument( argument, arg );
  if ( ( victim = get_char_world( ch, arg ) ) == NULL ) {
    send_to_char("They aren't here.\n\r", ch );
    return;
  }

  if ( IS_NPC(victim) ) {
    send_to_char("Only PC's can be branded.\n\r", ch );
    return;
  }

  if ( victim->pcdata->branded ) {
    send_to_charf(ch,"%s is already branded.\n\r",NAME(victim));
    return;
  }

  // is_god is true if the victim worships ch
  bool is_god = FALSE;
  const const char *god = god_name(victim->pcdata->god);
  if ( !str_cmp( god, ch->name ) )
    is_god = TRUE;

  victim->pcdata->branded = TRUE;
  send_to_charf( ch, 
		 "You have branded %s who is following %s.\n\r",
		 NAME(victim), is_god?"you":god);
  send_to_charf( victim,
		 "{YYou have been branded by %s.{x\n\r",
		 god);
  int mark;
  int i = victim->pcdata->god;
  if ( ( mark = gods_table[i].brand_mark ) != 0 ) {
    OBJ_INDEX_DATA *pObj = get_obj_index( mark );
    if ( pObj == NULL ) {
      bug("Invalid brand mark (%d) for god (%s) obj_index_data test",
	  mark, gods_table[i].name );
      return;
    }
    OBJ_DATA *obj = create_object( pObj, 0 );
    if ( obj == NULL ) {
      bug("Invalid brand mark (%d) for god (%s) obj_data test",
	  mark, gods_table[i].name );
      return;
    }
    obj_to_char(obj,victim);
    equip_char(victim,obj,WEAR_BRAND);
  }
}
*/
void do_gset( CHAR_DATA *ch, const char *argument ) {
  int vnum;

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( argument[0] == '\0' ) {
    send_to_charf(ch,
		  "You current default goto location is %d.\n\r", 
		  ch->pcdata->goto_default);
    return;
  }

  if ( !is_number( argument ) ) {
    send_to_charf(ch,"argument for gset must be numerical.\n\r");
    return;
  }

  vnum = atoi( argument );

  if ( get_room_index( vnum ) == NULL ) {
    send_to_charf(ch,"Invalid location, room doesn't exist.\n\r");
    return;
  }

  ch->pcdata->goto_default = vnum;
  send_to_charf(ch,"Your new default goto location is %d.\n\r",
		ch->pcdata->goto_default);
}

void do_areacount( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  int value;
  AREA_DATA *area;

  one_argument( argument, arg );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  if ( arg[0] != '\0' ) {
    if (!is_number(arg)) {
      area = NULL;
      for (AREA_DATA *pArea = area_first; pArea; pArea = pArea->next ) {
	if (is_name(arg,pArea->name)) {
	  area = pArea;
	  break;
	}
      }
      if ( area == NULL ) {
	send_to_char("That area doesn't exist.\n\r",ch);
	return;
      }
    }
    else {
      value = atoi(arg);
      if ( value <= 0 ) {
	send_to_char("You have to choose a positive vnum.\n\r",ch);
	return;
      }
      
      area = get_area_data( value );
      if ( area == NULL ) {
	send_to_char("That area vnum doesn't exist.\n\r",ch);
	return;
      }
    }
  }
  else {
    if ( ch->in_room == NULL || ch->in_room->area == NULL ){
      send_to_char("You are nowhere !!\n\r",ch);
      send_to_char("BTW, how did you do that ?\n\r",ch);
      return;
    }
    area = ch->in_room->area;
  }

  //send_to_charf(ch,"You just have reset %s.\n\r",ch->in_room->area->name);
  //reset_area(ch->in_room->area);
  if ( area == NULL ) {
    bug("do_areacount: Invalid area");
    return;
  }
  
  int mob_count = 0;
  int obj_count = 0;
  int pc_count = 0;
  for ( int vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ ) {
    ROOM_INDEX_DATA *pRoom;
    if ( ( pRoom = get_room_index(vnum) ) == NULL )
      continue;
    for ( CHAR_DATA *vch = pRoom->people; vch != NULL; vch = vch->next_in_room ) {
      if ( IS_NPC(vch) ) mob_count++;
      else pc_count++;
      for ( OBJ_DATA *obj = vch->carrying; obj != NULL; obj = obj->next_content )
	obj_count++;
    }
    for ( OBJ_DATA *obj = pRoom->contents; obj != NULL; obj = obj->next_content )
      obj_count++;
  }

  send_to_charf(ch,"Mobile: %10d\n\r", mob_count);
  send_to_charf(ch,"Player: %10d\n\r", pc_count);
  send_to_charf(ch,"Object: %10d\n\r", obj_count);
}


void do_resetarea( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  int value;
  AREA_DATA *area;

  argument = one_argument( argument, arg );

  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  // Added by SinaC 2001 ... slay every mobiles in the area
  bool hard = FALSE;
  if ( !str_cmp( arg, "hard" ) ) {
    hard = TRUE;
    strcpy( arg, "\0" );
  }
  if ( !str_cmp( argument, "hard" ) )
    hard = TRUE;

  if ( arg[0] != '\0' ) {
    if (!is_number(arg)) {
      area = NULL;
      for (AREA_DATA *pArea = area_first; pArea; pArea = pArea->next ) {
	if (is_name(arg,pArea->name)) {
	  area = pArea;
	  break;
	}
      }
      if ( area == NULL ) {
	send_to_char("That area doesn't exist.\n\r",ch);
	return;
      }
    }
    else {
      value = atoi(arg);
      if ( value <= 0 ) {
	send_to_char("You have to choose a positive vnum.\n\r",ch);
	return;
      }
      
      area = get_area_data( value );
      if ( area == NULL ) {
	send_to_char("That area vnum doesn't exist.\n\r",ch);
	return;
      }
    }
  }
  else {
    if ( ch->in_room == NULL || ch->in_room->area == NULL ){
      send_to_char("You are nowhere !!\n\r",ch);
      send_to_char("BTW, how did you do that ?\n\r",ch);
      return;
    }
    area = ch->in_room->area;
  }

  //send_to_charf(ch,"You just have reset %s.\n\r",ch->in_room->area->name);
  //reset_area(ch->in_room->area);
  if ( area == NULL ) {
    bug("do_resetarea: Invalid area");
    return;
  }

  // Added by SinaC 2001 ... slay every mobiles in the area
  if ( hard )
    for ( int vnum = area->min_vnum; vnum <= area->max_vnum; vnum++ ) {
      ROOM_INDEX_DATA *pRoom;
      if ( ( pRoom = get_room_index(vnum) ) != NULL ) {
	CHAR_DATA *vch_next = NULL;
	for ( CHAR_DATA *vch = pRoom->people; vch != NULL; vch = vch_next ) {
	  vch_next = vch->next_in_room;
	  if ( IS_NPC(vch) && vch->pIndexData->area == area )
	    extract_char( vch, TRUE );
	}
	OBJ_DATA *obj_next = NULL;
	for ( OBJ_DATA *obj = pRoom->contents; obj != NULL; obj = obj_next ) {
	  obj_next = obj->next_content;
	  if ( obj->pIndexData->area == area )
	    extract_obj( obj );
	}
      }
    }
  
  send_to_charf(ch,"You just have reset%s %s.\n\r",
		hard?" {R(hard){x":"",
		area->name );
  // Added by SinaC 2001 so area is resetted even if a player is within
  reset_always = TRUE;
  reset_area(area);
  reset_always = FALSE;

  return;
}

// Added by SinaC 2003
void do_show_group( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_char("Mobiles can't use that command.\n\r", ch );
    return;
  }

  char arg[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg );
  int gn = group_lookup( arg );
  if ( gn < 0 ) {
    send_to_char("Invalid group name\n\r", ch );
    return;
  }
  group_type *gr = &(group_table[gn]);

  send_to_charf(ch,"name: %s\n\r", gr->name );
  send_to_charf(ch,"rating:");
  for ( int i = 0; i < MAX_CLASS; i++ )
    send_to_charf(ch," %3d", gr->rating[i] );
  send_to_charf(ch,"\n\r");
  send_to_charf(ch,"godrate:");
  for ( int i = 0; i < MAX_GODS; i++ )
    send_to_charf(ch," %3d", gr->god_rate[i] );
  send_to_charf(ch,"\n\r");
  send_to_charf(ch,"#spells: %d\n\r", gr->spellnb );
  for ( int i = 0; i < gr->spellnb; i++ )
    send_to_charf(ch," [%s]", gr->spells[i] );
  send_to_charf(ch,"\n\r");

  return;
}

void do_set_script( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1 == NULL || arg1[0] == '\0' ) {
    send_to_char("You must specify a target (player or mob).\n\r", ch );
    return;
  }
  if ( arg2 == NULL || arg2[0] == '\0' ) {
    send_to_char("You must specify a script. Use program list  to get a list of available scripts.\n\r", ch );
    return;
  }

  CHAR_DATA *victim = get_char_world( ch, arg1 );
  if ( victim == NULL ) {
    send_to_char("Target doesn't exist.\n\r", ch );
    return;
  }
  CLASS_DATA *cla;
  if ( ( cla = hash_get_prog( arg2 ) ) == NULL ) {
    send_to_char("Script doesn't exist.\n\r", ch );
    return;
  }
  if ( get_root_class(cla) != default_mob_class ) {
    send_to_char("Script must be a mob script.\n\r", ch );
    return;
  }
  if ( cla->isAbstract ) {
    send_to_char("You can't assign to an ABSTRACT class.", ch );
    return;
  }

  if ( !IS_NPC(victim) )
    send_to_char("{RMODIFYING {BPLAYER {RCLASS !!{x\n\r", ch );
  if ( victim->clazz != NULL )
    send_to_charf(ch,"Replacing %s's class: %s  with  class: %s\n\r",
		 NAME(victim), victim->clazz->name, cla->name );
  else
    send_to_charf(ch,"Assigning class: %s for %s\n\r",
		 cla->name, NAME(victim) );
  victim->clazz = cla;
}

void do_showfaction( CHAR_DATA *ch, const char *argument ) {
  if ( argument[0] == '\0' ) {
    send_to_charf(ch,"%9s","Name\\Name:  ");
    for ( int i = 0; i < factions_count; i++ )
      send_to_charf(ch,"%9s", faction_table[i].name);
    send_to_charf(ch,"\n\r");
    for ( int i = 0; i < factions_count; i++ ) {
      send_to_charf(ch,"%9s: ", faction_table[i].name );
      for ( int j = 0; j < factions_count; j++ )
	send_to_charf(ch,"%9d", faction_table[i].friendliness[j] );
      send_to_charf(ch,"\n\r");
    }
    return;
  }
  char arg[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg2 );
  FACTION_DATA *faction = get_faction( arg );
  if ( faction != NULL ) { // found faction starting with this name
    if ( arg2 == NULL || arg2[0] == '\0' ) {
      send_to_charf(ch,"%9s","Name\\Name:  ");
      for ( int i = 0; i < factions_count; i++ )
	send_to_charf(ch,"%9s", faction_table[i].name);
      send_to_charf(ch,"\n\r");
      send_to_charf(ch,"%9s: ", faction->name );
      for ( int i = 0; i < factions_count; i++ )
	send_to_charf(ch,"%9d", faction->friendliness[i]);
      send_to_charf(ch,"\n\r");
    }
    else if ( !str_cmp( arg2, "races" ) ) {
      send_to_charf(ch,"%9s: ",faction->name);
      for ( int i = 0; i < faction->nb_races; i++ )
	send_to_charf(ch,"  %s", race_table[faction->races[i]].name);
      send_to_charf(ch,"\n\r");
    }
    else {
      int faction2Id = get_faction_id(arg2);
      if ( faction2Id == -1 ) {
	send_to_charf(ch,"Invalid second faction.\n\r");
	return;
      }
      send_to_charf(ch,"Faction: %9s/%9s: %6d\n\r",
		    arg, arg2, faction->friendliness[faction2Id] );
    }
    return;
  }
  // faction not found, try to find a player/mob with this name
  CHAR_DATA *victim = get_char_world( ch, arg );
  if ( victim != NULL ) {
    int *friends_base = faction_table[victim->faction].friendliness;
    int *friends_current = faction_table[victim->faction].friendliness;
    if ( !IS_NPC(victim) ) {
      friends_base = new int [MAX_FACTION];
      friends_current = new int [MAX_FACTION];
      memcpy( friends_base, victim->pcdata->base_faction_friendliness, MAX_FACTION * sizeof(int));
      memcpy( friends_current, victim->pcdata->current_faction_friendliness, MAX_FACTION * sizeof(int));
    }
    if ( arg2 == NULL || arg2[0] == '\0' ) {
      send_to_charf(ch,"%9s","Name\\Name:  ");
      for ( int i = 0; i < factions_count; i++ )
	send_to_charf(ch,"%9s", faction_table[i].name);
      send_to_charf(ch,"\n\r");
      send_to_charf(ch,"%9s: ", IS_NPC(victim)?faction_table[victim->faction].name:"player" );
      for ( int i = 0; i < factions_count; i++ )
	if ( friends_current[i] != friends_base[i] )
	  send_to_charf(ch,"%9d(%d)", friends_current[i], friends_base[i] );
	else
	  send_to_charf(ch,"%9d", friends_current[i] );
      send_to_charf(ch,"\n\r");
    }
    else {
      int faction2Id = get_faction_id(arg2);
      if ( faction2Id == -1 ) {
	send_to_charf(ch,"Invalid second faction.\n\r");
	return;
      }
      if ( friends_base[faction2Id] != friends_current[faction2Id] )
	send_to_charf(ch,"Faction: %9s/%9s: %6d(%d)\n\r",
		      IS_NPC(victim)?faction_table[victim->faction].name:"player",
		      faction_table[faction2Id].name,
		      friends_current[faction2Id], friends_base[faction2Id] );
      else
	send_to_charf(ch,"Faction: %9s/%9s: %6d\n\r",
		      IS_NPC(victim)?faction_table[victim->faction].name:"player",
		      faction_table[faction2Id].name,
		      friends_current[faction2Id] );
    }
    return; 
  }
  send_to_charf(ch,
		"Syntax:\n\r"
		"   showfaction                                  display faction table\n\r"
		"   showfaction <faction name> races             display races getting this faction\n\r"
		"   showfaction <faction name> [<faction name>]  display faction line/entry\n\r"
		"   showfaction <char name> [<faction name>]     display char faction line/entry\n\r");
  return;
}

void reload_syntax( CHAR_DATA *ch ) {
  send_to_charf(ch,
		"Syntax:\n\r"
		"  reload                      reload every tables.\n\r"
		"  reload scripts              reload scripts(mob/obj/room program).\n\r");
}
void do_reload( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  bool loadAll = FALSE;
  if ( arg1 == NULL || arg1[0] == '\0' )
    loadAll = TRUE;
  if ( loadAll || !str_prefix(arg1,"scripts") )
    reload_scripts( ch, argument );
  else if ( !loadAll )
    reload_syntax( ch );
}

void do_wiztest( CHAR_DATA *ch, const char *argument ) {
//  CHAR_DATA *vch_next;
//  int i = 0;
//  for ( CHAR_DATA *vch = char_list; vch; vch = vch_next ) {
//    vch_next = ch->next;
//    if ( i++ % 1000 == 0 )
//      log_stringf("--> %d", i );
//    if ( i > 1000000 )
//      i = 0;
//  }

  if ( !str_cmp(argument, "gc" ) ) {
    send_to_charf(ch, "Garbage collecting...\n\r");
    GC_gcollect();
    send_to_charf(ch,"Done.\n\r");
  }
  else if ( !str_cmp(argument, "save") ) {
    send_to_charf(ch,"Saving datas...\n\r");
    new_save_bans();
    new_save_brew_formula();
    new_save_unique_items();
    new_save_commands();
    new_save_disabled();
    new_save_prerequisites();
    new_save_clans();
    new_save_material();
    new_save_liquid();
    new_save_gods();
    new_save_factions();
    new_save_races();
    new_save_pcraces();
    new_save_spheres();
    new_save_groups();
    new_save_abilities();
    new_save_pc_classes();
    save_schools();
    save_hometown();
    send_to_charf(ch,"Done.\n\r");
  }
  else if ( !str_cmp( argument, "worldsave" ) )
    save_world_state();
  else if ( !str_cmp( argument, "worldload" ) ) {
    // clean world
    send_to_charf(ch,"Cleaning world.\n\r");
    //for (AREA_DATA *pArea = area_first; pArea; pArea = pArea->next )
    for( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
      for( ROOM_INDEX_DATA *pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next ) {
	//if ( pRoomIndex->area == pArea ) {
	CHAR_DATA *vch_next = NULL;
	for ( CHAR_DATA *vch = pRoomIndex->people; vch != NULL; vch = vch_next ) {
	  vch_next = vch->next_in_room;
	  if ( IS_NPC(vch) 
	       && !( vch->master != NULL && vch->master->pet == vch ) )
	    extract_char( vch, TRUE );
	}
	OBJ_DATA *obj_next = NULL;
	for ( OBJ_DATA *obj = pRoomIndex->contents; obj != NULL; obj = obj_next ) {
	  obj_next = obj->next_content;
	  if ( !IS_SET( obj->extra_flags, ITEM_UNIQUE ) ) // don't remove unique items
	    extract_obj( obj );
	}
      }
    send_to_charf(ch,"Done.\n\r");
    // reload world
    send_to_charf(ch,"Reloading world.\n\r");
    load_world_state();
    send_to_charf(ch,"Done.\n\r");
  }
  else if ( !str_cmp( argument, "school" ) ) {
    for ( int i = 0; i < MAX_SCHOOL; i++ ) {
      send_to_charf(ch,"School: %s =", magic_school_table[i].name );
      for ( int j = 0; j < magic_school_table[i].nb_spells; j++ )
	if ( magic_school_table[i].spells_list[j] > 0 )
	  send_to_charf(ch,"  [%s]", ability_table[magic_school_table[i].spells_list[j]].name);
	else
	  send_to_charf(ch,"  [NONE]" );
      send_to_charf(ch,"\n\r");
    }
  }
  else if ( !str_cmp( argument, "timeload" ) ) {
    load_time();
    send_to_charf(ch,"Time loaded.\n\r");
  }
  else if ( !str_cmp( argument, "timesave" ) ) {
    save_time();
    send_to_charf(ch,"Time saved.\n\r");
  }
  else if ( !str_cmp( argument, "day" ) ) {
    time_info.hour = 6;
    send_to_charf(ch,"Hour set to 6.\n\r");
  }
  else if ( !str_cmp( argument, "night" ) ) {
    time_info.hour = 20;
    send_to_charf(ch,"Hour set to 20.\n\r");
  }
  else if ( !str_cmp( argument, "moonON" ) ) {
    moon_info[0].ph_t = (4*moon_info[0].ph_p)/NBR_PHASE;
    time_info.hour = 21;
    moon_info[0].po_t = moon_info[0].po_v+1;
    send_to_charf(ch,"Full moon set.\n\r");
  }
  else if ( !str_cmp( argument, "moonOFF" ) ) {
    moon_info[0].ph_t = (5*moon_info[0].ph_p)/NBR_PHASE;
    time_info.hour = 7;
    moon_info[0].po_t = 0;
    send_to_charf(ch,"Full moon removed.\n\r");
  }
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ability_type *ability = &(ability_table[i]);
    if ( ability->lockBit )
      send_to_charf(ch,"Ability: %s edited by %s.\n\r", 
		    ability->name, ability->editedBy->character->name );
  }
}

// dataconfig <variable name> <new value>
void data_config_syntax( CHAR_DATA *ch ) {
  send_to_charf(ch,
		"Syntax:  dataconfig <variable name> <new value>\n\r"
		"         dataconfig show\n\r");
}
void do_dataconfig( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_STRING_LENGTH];
  char arg2[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( !str_cmp( arg1, "show" ) && arg2[0] == '\0' ) {
    int old_data_verbose = DATA_VERBOSE;
    DATA_VERBOSE = 1000;
    show_config_variable();
    DATA_VERBOSE = old_data_verbose;
    return;
  }
  if ( arg1[0] == '\0' || arg2[0] == '\0' ) {
    data_config_syntax( ch );
    return;
  }
  const int tagId = find_tag( arg1, DATA_CONFIG ); // only CONFIG tags are accepted
  Value v; // Create a valid value depending on tokvalue's type: integer/boolean/string
  if ( is_number( arg2 ) ) // number
    v = atoi(arg2);
  else if ( !str_cmp( arg2, "true" ) ) // boolean
    v = 1;
  else if ( !str_cmp( arg2, "false" ) )
    v = Value((long)0);
  else // string
    v = arg2;
  if ( !assign_config_variable( tagId, v ) ) {
    send_to_charf(ch,"Invalid arg: %s\n\r", arg1 );
    data_config_syntax( ch );
    return;
  }
  send_to_charf(ch,"Variable [%s] set to [%s]\n\r", arg1, arg2 );
}
