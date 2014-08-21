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
#include "data.h"

// Added by SinaC 2001
#include "comm.h"
#include "save.h"
#include "db.h"
#include "handler.h"
#include "olc_value.h"
#include "recycle.h"
// Added by SinaC 2003
#include "clan.h"
#include "classes.h"
#include "interp.h"
#include "act_info.h"
//#include "olc.h"
#include "config.h"
#include "olc_save.h"
#include "copyover.h"
#include "fight.h"
#include "act_move.h"
#include "arena.h"


// TRUE: save world state when doing a copyover (emergency or not)
bool SAVE_STATE_ON_COPYOVER;

char * EXE_FILE; /* Oxtal -- main sets this to argv[0] */

/*
Note that you might want to change your close_socket() a bit. I have changed
the connected state so that negative states represent logging-in, while as
positive ones represent states where the character is already inside the game.
close_socket() frees that chararacters with negative state, but just loses
link on those with a positive state. I believe that idea comes from Elwyn
originally.

Oxtal> Think this already done in Mystery.

Things to note: This corresponds to a reboot, followed by the characters
logging in again. This means that stuff like corpses, disarmed weapons etc.
are lost, unless you save those to the pfile. You should probably give the
players some warning before doing a copyover.

The command was inspired by the discussion on merc-l about how Fusion's MUD++
could reboot without players having to re-login :)

*/

extern "C" {
  void	execl( char * path, char * arg, ...);
  int	close		args( ( int fd ) );
}


void do_copyove (CHAR_DATA *ch, const char * argument) {
  send_to_char("If you want to do a copy over, you have to spell it out.\n\r",ch);
}

// SinaC 2003: copyover may have an argument: config name
// This function performs the copyover: called by do_copyover and auto_shutdown
int copyover_perform( char *msgToSend, const char *argument ) {
  FILE *fp;
  DESCRIPTOR_DATA *d, *d_next;
  char buf[100];
  char buf2[100];

  log_string("******* COPYOVER *******");

  fp = fopen (COPYOVER_FILE, "w");

  if (!fp) {
    logf ("Could not write to copyover file: %s", COPYOVER_FILE);
    perror ("do_copyover:fopen");
    return -1;
  }

  log_string("Saving player's descriptor");
  /* For each playing descriptor, save its state */
  for (d = descriptor_list; d ; d = d_next) {
    CHAR_DATA *och = CH(d);
    d_next = d->next; /* We delete from the list , so need to save this */

    if ( !d->character 
	 || ( d->connected > CON_PLAYING 
	      && d->connected < CON_NOTE_TO ) ) {/* drop those logging on */
      write_to_descriptor (d->descriptor, "\n\rSorry, we are rebooting. Come back in a few seconds.\n\r", 0);
      close_socket (d); /* throw'em out */
    }
    else {
      // SinaC 2003, 4th argument is master and 5th is leader, 6th is fighting, 7th betted_on, 8th challenged, 9th is bet amount
      fprintf (fp, "%d %s %s %s %s %s %s %s %d %s\n",
	       d->descriptor,
	       och->name,
	       d->host, 
	       (och->master && !IS_NPC(och->master)) ? NAME(och->master):"none",
	       (och->leader && !IS_NPC(och->leader)) ? NAME(och->leader):"none",
	       (och->fighting && !IS_NPC(och->fighting)) ? NAME(och->fighting):"none",
	       (och->pcdata->challenged && !IS_NPC(och->pcdata->challenged)) ? NAME(och->pcdata->challenged):"none",
	       (och->pcdata->betted_on && !IS_NPC(och->pcdata->betted_on)) ? NAME(och->pcdata->betted_on):"none",
	       och->pcdata->bet_amt,
	       // SinaC 2003, jog: we only saved run_head (run_buf is not needed)
	       d->run_head?d->run_head:"none"
	       );

      //save_char_obj (och);
      new_save_pFile(och,TRUE);
      write_to_descriptor (d->descriptor, msgToSend, 0);
    }
  }

  fprintf (fp, "-1\n");
  fclose (fp);

  // Moved by SinaC 2001 was before characters saving
  // Saving all changed areas
  //do_asave (NULL, "");
  save_on_shutdown();
  // If global save_state_on_copyover is SET, we save the world
  if ( SAVE_STATE_ON_COPYOVER )
    save_world_state();

  /* Close reserve and other always-open files and release other resources */
	
  fclose (fpReserve);
	
  /* exec - descriptors are inherited */
	
  //sprintf (buf, "%d", port);
  // Command line has been modified, SinaC 2003
  sprintf( buf, "-port=%d", port );
  sprintf (buf2, "%d", control);
  // modified by SinaC 2003: config file
  //execl (EXE_FILE, EXE_FILE, buf, "copyover", buf2, (char *) NULL);
  if ( argument[0] != '\0' )
    one_argument( argument, wantedConfig ); // use first argument as config

  log_string(" RESTARTING MUD ON COPYOVER...");

  // if a config is specified it must be the first argument
  //  because command line may contain variable=xxx
  // if config file was read after other command line arg, they will be overwritten
  if ( wantedConfig[0] == '\0' ) // -> 4 arg: exe port copyover/state control
    execl (EXE_FILE, EXE_FILE, 
  	   buf, SAVE_STATE_ON_COPYOVER?"state":"copyover", buf2, (char *) NULL);
  else                           // -> 5 arg: exe config port copyover/state control
    execl (EXE_FILE, EXE_FILE, 
  	   wantedConfig, buf, SAVE_STATE_ON_COPYOVER?"state":"copyover", buf2, (char *) NULL);
  
  /* Failed - sucessful exec will not return */
	
  /* Here you might want to reopen fpReserve */
  fpReserve = fopen( NULL_FILE, "r" );
  return 0;
}
void do_copyover (CHAR_DATA *ch, const char * argument) {
  char buf[MAX_STRING_LENGTH];
  sprintf (buf, "\n\r *** CopyOver by %s - please remain seated!\n\r", ch->name);
  char arg1[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg1 );
  int code;
  code = copyover_perform(buf,arg1);
  if ( code == -1 ) {
    send_to_char ("Copyover file not writeable, aborted.\n\r",ch);
    return;
  }
  perror ("do_copyover: execl");
  send_to_char ("Copyover FAILED!\n\r",ch);
}

// Recover from a copyover - load players
typedef struct ptr_list PTR_LIST; // store master/leader
struct ptr_list {
  CHAR_DATA *ch;
  const char *master;
  const char *leader;
  const char *fighting;
  const char *challenged;
  const char *betted_on;
  int bet_amt;

  PTR_LIST *next;
};
static PTR_LIST *ptr_list_first = NULL;

void copyover_recover () {
  DESCRIPTOR_DATA *d;
  FILE *fp;
  char name [100];
  char host[MAX_STRING_LENGTH];
  int desc;
  bool fOld;
  char master[MAX_INPUT_LENGTH];
  char leader[MAX_INPUT_LENGTH];
  char fighting[MAX_INPUT_LENGTH];
  char challenged[MAX_INPUT_LENGTH];
  char betted_on[MAX_INPUT_LENGTH];
  int bet_amt;
  char jog_run_head[MAX_INPUT_LENGTH];
	
  logf ("Copyover recovery initiated");
	
  fp = fopen (COPYOVER_FILE, "r");
	
  if (!fp) {// there are some descriptors open which will hang forever then ? 
    perror ("copyover_recover:fopen");
    logf ("Copyover file not found. Exitting.\n\r");
    exit (1);
  }

  unlink (COPYOVER_FILE); // In case something crashes - doesn't prevent reading   

  ptr_list_first = NULL;
  for (;;) {
    if ( feof(fp) ) // SinaC 2003
      break;

    // SinaC 2003, 4th is master and 5th is leader, 6th is fighting, 7th challenged and 8th betted_on
    //fscanf (fp, "%d %s %s\n", &desc, name, host);
    fscanf (fp, "%d %s %s %s %s %s %s %s %d %s\n", &desc, name, host, master, leader, fighting, challenged, betted_on, &bet_amt, jog_run_head );
    if (desc == -1)
      break;

    // Write something, and check if it goes error-free
    if (!write_to_descriptor (desc, "\n\rRestoring from copyover...\n\r",0)) {
      close (desc); /* nope */
      continue;
    }

    d = new_descriptor();
    d->descriptor = desc; // set up various stuff
		
    d->host = str_dup (host);
    d->next = descriptor_list;
    descriptor_list = d;
    d->connected = CON_COPYOVER_RECOVER; // 16, so close_socket frees the char
	
    // Now, find the pfile */
		
    //fOld = load_char_obj (d, name);
    fOld = new_load_pFile( d, name );
    
    if (!fOld) {// Player file not found?! 
      write_to_descriptor (desc, "\n\rSomehow, your character was lost in the copyover. Sorry.\n\r", 0);
      close_socket (d);			
    }
    else {// ok! 
      write_to_descriptor (desc, "\n\rCopyover recovery complete.\n\r",0);

      // Just In Case 
      if (!d->character->in_room)
	//d->character->in_room = get_room_index (ROOM_VNUM_TEMPLE);
	d->character->in_room = get_recall_room(d->character);

      // Insert in the char_list
      d->character->next = char_list;
      char_list = d->character;
      CHAR_DATA *ch = d->character;

      char_to_room (ch, ch->in_room);
      if ( !fState ) // look only if word is already created, if fState is true, we have to read the world
	do_look (ch, "auto");
      act ("$n materializes!", ch, NULL, NULL, TO_ROOM);
      d->connected = CON_PLAYING;

      if (ch->pet != NULL) {
	//char_to_room(ch->pet,ch->in_room);
	CHAR_DATA *previous = ch;
	CHAR_DATA *pet = ch->pet;
	while ( pet != NULL ) { // modify by SinaC 2003: pet having pet, ...
	  if ( pet->in_room == NULL )
	    char_to_room(pet,previous->in_room);
	  else
	    char_to_room(pet, pet->in_room ); // SinaC 2003
	  previous = pet;
	  pet = pet->pet;
	}
	act("$n materializes!.",ch->pet,NULL,NULL,TO_ROOM);
      }

      // Finish parsing extra fields
      if ( !fState ) { //only when world is loaded
	use_delayed_parse_extra_fields( ch );
	if ( ch->pet != NULL )
	  use_delayed_parse_extra_fields( ch->pet );
      }

      if ( !fState ) { // set class only when world is loaded
	ch->clazz = default_player_class;
	MOBPROG( ch, NULL, "onLoad" );
      }

      // No jog
      if ( jog_run_head[0] == '\0'
	   || !str_cmp( jog_run_head, "none" ) ) {
	ch->desc->run_buf = NULL;
	ch->desc->run_head = NULL;
      }
      // Found jog
      else {
	ch->desc->run_buf = str_dup_unsafe(jog_run_head);
	ch->desc->run_head = ch->desc->run_buf;
      }

      // Store in temporary struct master and leader
      if ( str_cmp( master, "none" ) && str_cmp( leader, "none" ) ) {
	PTR_LIST *current_ptr = new PTR_LIST; // create
      	current_ptr->ch = ch; // add info
      	if ( str_cmp( master, "none" ) )
      	  current_ptr->master = str_dup(master);
      	if ( str_cmp( leader, "none" ) )
      	  current_ptr->leader = str_dup(leader);
	if ( str_cmp( fighting, "none" ) )
	  current_ptr->fighting = str_dup(fighting);
	if ( str_cmp( challenged, "none" ) )
	  current_ptr->challenged = str_dup(challenged);
	if ( str_cmp( betted_on, "none" ) )
	  current_ptr->betted_on = str_dup(betted_on);
	current_ptr->bet_amt = bet_amt;
      	current_ptr->next = ptr_list_first; // and insert in the list
      	ptr_list_first = current_ptr;
      }
    }
  }

  // reform PC's group, fight, challenged and betted_on
  for ( PTR_LIST *p = ptr_list_first; p != NULL; p = p->next ) {
    if ( p->master ) {
      CHAR_DATA *ch = p->ch;
      CHAR_DATA *master = get_pc_world(p->master);
      if ( master == NULL )
	bug("[%s] master [%s] doesn't exist anymore", NAME(ch), p->master );
      else 
	ch->master = master;
    }
    if ( p->leader ) {
      CHAR_DATA *ch = p->ch;
      CHAR_DATA *leader = get_pc_world(p->leader);
      if ( leader == NULL )
	bug("[%s] leader [%s] doesn't exist anymore", NAME(ch), p->leader );
      else 
	ch->leader = leader;
    }
    if ( p->fighting ) {
      CHAR_DATA *ch = p->ch;
      CHAR_DATA *fighter = get_pc_world(p->fighting);
      if ( fighter == NULL )
	bug("[%s] fighting [%s] doesn't exist anymore", NAME(ch), p->fighting );
      else
	set_fighting( ch, fighter );
    }
    if ( p->challenged ) {
      CHAR_DATA *ch = p->ch;
      CHAR_DATA *challenged = get_pc_world(p->challenged);
      if ( challenged == NULL )
	bug("[%s] challenged [%s] doesn't exist anymore", NAME(ch), p->challenged );
      else
	ch->pcdata->challenged = challenged;
    }
    if ( p->betted_on ) {
      CHAR_DATA *ch = p->ch;
      CHAR_DATA *betted_on = get_pc_world(p->betted_on);
      if ( betted_on == NULL )
	bug("[%s] betted_on [%s] doesn't exist anymore", NAME(ch), p->betted_on );
      else
	ch->pcdata->betted_on = betted_on;
    }
    p->ch->pcdata->bet_amt = p->bet_amt;
  }

  fclose (fp);
}

void auto_shutdown() {  
  log_shutdown("Crashed");
  copyover_perform("\n\r *** Emergency CopyOver - please remain seated!\n\r","");
  perror ("emergency_copyover: execl"); 
}

void log_shutdown( const char *type_reboot ) {
  FILE *fp;
  CHAR_DATA *vch;
  CHAR_DATA *vch_next;
  DESCRIPTOR_DATA *d;
  int sn, count_spell = 0, count_skill = 0, count_power = 0, 
    /*Added by SinaC 2003*/count_song = 0, count_affect = 0;
  char *st;
  char *cla;
  char s[100];
  char idle[10];

  merc_down = TRUE;

  fclose(fpReserve);

  if((fp = fopen(LAST_COMMAND_FILE,"a")) == NULL) {
    bug("Error in log_shutdown opening [%s]", LAST_COMMAND_FILE);
    exit(-1);
  }

  fprintf(fp,"=====================================\n");
  fprintf(fp,"=====================================\n");
  fprintf(fp,"=====================================\n");
  fprintf(fp,"Mud %s: %s\n", type_reboot, (char *) ctime( &current_time));
  fprintf(fp,"Last Command: %s\n", last_command);
  fprintf(fp,"\n===================================\n");
  fprintf(fp,"Socket List:\n");

  fprintf( fp, "[  #     Connected_State     Login@  Idl] Player Name             Host\n" );
  fprintf( fp,
	   "--------------------------------------------------------------------------------------------\n");

  for ( d = descriptor_list; d; d = d->next ) {
    if ( d->character ) {
      switch( d->connected ) {
      case CON_PLAYING:              st = "    PLAYING     ";    break;
      case CON_GET_NAME:             st = "   Get Name     ";    break;
      case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd  ";    break;
      case CON_CONFIRM_NEW_NAME:     st = " Confirm Name   ";    break;
      case CON_GET_NEW_PASSWORD:     st = "Get New Passwd  ";    break;
      case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd  ";    break;
      case CON_GET_NEW_RACE:         st = "  Get New Race  ";    break;
      case CON_GET_NEW_SEX:          st = "  Get New Sex   ";    break;
      case CON_GET_NEW_CLASS:        st = " Get New Class  ";    break;
      case CON_GET_ALIGNMENT:        st = " Get Alignment  ";    break;
      case CON_DEFAULT_CHOICE:       st = "   Customize?   ";    break;
      case CON_GEN_GROUPS:           st = "  Customizing   ";    break;
      case CON_PICK_WEAPON:          st = " Picking weapon ";    break;
      case CON_READ_MOTD:            st = "  Reading MOTD  ";    break;
      case CON_READ_IMOTD:           st = " Reading IMOTD  ";    break;
      case CON_BREAK_CONNECT:        st = " Reconnecting?  ";    break;
      case CON_COPYOVER_RECOVER:     st = "Copyover Recover";    break;
      case CON_NOTE_TO:              st = "  NOTE: To      ";    break;
      case CON_NOTE_SUBJECT:         st = "  NOTE: Subject ";    break;
      case CON_NOTE_EXPIRE:          st = "  NOTE: Expire  ";    break;
      case CON_NOTE_TEXT:            st = "  NOTE: Text    ";    break;
      case CON_NOTE_FINISH:          st = "  NOTE: Finish  ";    break;
      case CON_GET_GOD:              st = "     Get God    ";    break;
      case CON_REBIRTH:              st = "     Rebirth    ";    break;
      case CON_REMORT:               st = "     Remort     ";    break;
      case CON_ANSI:                 st = "    Get Ansi   ";    break;
      case CON_GET_HOMETOWN:         st = "  Get Hometown ";    break;
      default:                       st = "    !UNKNOWN!   ";    break;
      }

      vch = d->original ? d->original : d->character;
      strftime( s, 100, "%I:%M%p", localtime( &vch->logon ) );
      
      if ( vch->timer > 0 )
	sprintf( idle, "%-2d", vch->timer );
      else
	sprintf( idle, "  " );
      
      fprintf( fp, "[%3d     %s    %7s   %2s] %-16s %-64.64s\n",
	       d->descriptor, st, s, idle,
	       ( d->original ) ? d->original->name : ( d->character )  ? d->character->name : "(None!)",
	       d->host );
    }
  }

  fprintf(fp,"\n===================================\n");
  fprintf(fp,"Players online:\n");
  
  int iClass, iRace, iClan;
  int nNumber = 0, nMatch = 0, wlevel = MAX_LEVEL;
  long rgfClass;
  bool rgfRace[MAX_PC_RACE];
  bool rgfClan[UPPER_MAX_CLAN];
  
  rgfClass = 0;
  for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
    rgfRace[iRace] = FALSE;
  for (iClan = 0; iClan < UPPER_MAX_CLAN; iClan++)
    rgfClan[iClan] = FALSE;
  
  
  fprintf( fp, "[   Immortals  ]\n");
  for( wlevel=MAX_LEVEL; wlevel>LEVEL_HERO; wlevel-- ) {
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *vch;
      char const *cla;
      char const *clan_name;
      char const *clanname;
      char const *typemem;

      if ( ( d->connected != CON_PLAYING 
	     && (d->connected < CON_NOTE_TO 
		 || d->connected > CON_NOTE_FINISH) ) )
	continue;

      vch   = ( d->original != NULL ) ? d->original : d->character;

      if( vch->level != wlevel )
	continue;

      nMatch++;
      cla = class_whoname(vch->bstat(classes));
      switch ( vch->level ) {
      default: break;
      case MAX_LEVEL - 0 : cla = "     IMP      ";     break;
      case MAX_LEVEL - 1 : cla = "     CRE      ";     break;
      case MAX_LEVEL - 2 : cla = "     SUP      ";     break;
      case MAX_LEVEL - 3 : cla = "     DEI      ";     break;
      case MAX_LEVEL - 4 : cla = "     GOD      ";     break;
      case MAX_LEVEL - 5 : cla = "     IMM      ";     break;
      case MAX_LEVEL - 6 : cla = "     DEM      ";     break;
      case MAX_LEVEL - 7 : cla = "     ANG      ";     break;
      case MAX_LEVEL - 8 : cla = "     AVA      ";     break;
      case MAX_LEVEL - 9 : cla = "     HERO     ";     break;
      }

      if ( vch->pcdata->immtitle!=NULL && vch->pcdata->immtitle[0]!='\0')
	cla = str_dup("   IMMTITLE   ");
      clanname="";
      typemem="";
      if (is_clan(vch)) {
	typemem=lookup_clan_status(vch->clan_status);
	clanname=get_clan_table(vch->clan)->who_name;	         
      }
      fprintf( fp, "[%s] %s%s%s%s%s%s%s%s%s%s\n",
	       cla,
	       clanname,
	       typemem,
	       IN_BATTLE(vch)||IN_WAITING(vch)?"[BATTLE]":"",
	       IS_SET(vch->comm, COMM_AFK) ? "[AFK] " : "",
	       vch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
	       vch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
	       IS_SET(vch->act, PLR_KILLER) ? "(KILLER) " : "",
	       IS_SET(vch->act, PLR_THIEF)  ? "[THIEF] "  : "",
	       vch->name,
	       IS_NPC(vch) ? "" : vch->pcdata->title);
    }
  }

  fprintf( fp, "[    Mortals   ]\n");
  for( wlevel=LEVEL_HERO; wlevel>0; wlevel-- ) {
    for ( d = descriptor_list; d != NULL; d = d->next ) {
      CHAR_DATA *vch;
      char const *cla;
      char const *clan_name;
      char const *clanname;
      char const *typemem;
      if ( ( d->connected != CON_PLAYING 
	     && (d->connected < CON_NOTE_TO 
		 || d->connected > CON_NOTE_FINISH) ) )
	continue;
      vch   = ( d->original != NULL ) ? d->original : d->character;

      if( vch->level != wlevel )
	continue;
      nMatch++;
      cla = class_whoname(vch->bstat(classes));

      if(vch->level == LEVEL_HERO)
	cla = "HERO";

      if ( vch->pcdata->immtitle!=NULL && vch->pcdata->immtitle[0]!='\0')
	cla = str_dup("   IMMTITLE   ");

      clanname="";
      typemem="";
      if (is_clan(vch)) {
	typemem=lookup_clan_status(vch->clan_status);
	clanname=get_clan_table(vch->clan)->who_name;	         
      }

      if (vch->level==101) {
	fprintf( fp, "[     %s     ]{x %s%s%s%s%s%s%s%s%s%s%s\n",
		 cla,
		 clanname,
		 typemem,
		 IN_BATTLE(vch)||IN_WAITING(vch)?"[BATTLE]":"",
		 vch->pcdata->name_accepted?"":"[NEWBIE]",
		 IS_SET(vch->comm, COMM_AFK) ? "[AFK] " : "",
		 vch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
		 vch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
		 IS_SET(vch->act, PLR_KILLER) ? "(KILLER){x " : "",
		 IS_SET(vch->act, PLR_THIEF)  ? "[THIEF]{x "  : "",
		 vch->name,
		 IS_NPC(vch) ? "" : vch->pcdata->title);
      }
      else {
	char race[10];
	if ( race_table[vch->cstat(race)].pc_race )
	  strcpy( race, pc_race_table[vch->cstat(race)].who_name ); // PC race
	else {
	  strncpy( race, race_table[vch->cstat(race)].name, 5 ); // NPC race
	  race[5] = '\0';
	}
	fprintf( fp, "[%3d %6s %s] %s%s%s%s%s%s%s%s%s%s%s\n",
		 vch->level,
		 //vch->bstat(race) < MAX_PC_RACE ? pc_race_table[vch->bstat(race)].who_name : "     ",
		 race,
		 cla,
		 clanname,
		 typemem,
		 IN_BATTLE(vch)||IN_WAITING(vch)?"[BATTLE]":"",
		 vch->pcdata->name_accepted?"":"[NEWBIE]",
		 IS_SET(vch->comm, COMM_AFK) ? "[AFK] " : "",
		 vch->incog_level >= LEVEL_HERO ? "(Incog) " : "",
		 vch->invis_level >= LEVEL_HERO ? "(Wizi) " : "",
		 IS_SET(vch->act, PLR_KILLER) ? "(KILLER) " : "",
		 IS_SET(vch->act, PLR_THIEF)  ? "[THIEF] "  : "",
		 vch->name,
		 IS_NPC(vch) ? "" : vch->pcdata->title);
      }
    }
  }
  fprintf(fp, "Players Found: %d\n", nMatch);

  fprintf(fp,"=====================================\n");
  fprintf(fp,"Memory Report:\n");

  for (sn = 0; sn < MAX_ABILITY; sn++) {
    if (ability_table[sn].name == NULL )
      break;
    // Modified by SinaC 2003
    switch(ability_table[sn].type) {
    case TYPE_SKILL: count_skill++; break;
    case TYPE_SPELL: count_spell++; break;
    case TYPE_POWER: count_power++; break;
    case TYPE_SONG: count_song++; break;
    case TYPE_AFFECT: count_affect++; break;
    }
    /*
    if (ability_table[sn].type == TYPE_SPELL)
      count_spell += 1;
    else if (ability_table[sn].type == TYPE_POWER)
      count_power += 1;
    else
      count_skill += 1;
    */
  }

  long total = 0;
  fprintf( fp, "Affects     %5d    mem %7d\n", top_affect,
		 top_affect * sizeof( AFFECT_DATA) );
  total += top_affect * sizeof( AFFECT_DATA );
  fprintf( fp, "Areas       %5d    mem %7d\n", top_area,
		 top_area * sizeof( AREA_DATA ));
  total += top_area * sizeof( AREA_DATA );
  fprintf( fp, "ExDes       %5d    mem %7d\n", top_ed,
		 top_ed * sizeof( EXTRA_DESCR_DATA ) );
  total += top_ed * sizeof( EXTRA_DESCR_DATA );
  fprintf( fp, "Exits       %5d    mem %7d\n", top_exit,
		 top_exit * sizeof( EXIT_DATA ) );
  total += top_exit * sizeof( EXIT_DATA );
  fprintf( fp, "Helps       %5d    mem %7d\n", top_help,
		 top_help * sizeof( HELP_DATA ) );
  total += top_help * sizeof( HELP_DATA );
  fprintf( fp, "Socials     %5d    mem %7d\n", social_count,
		 social_count * sizeof(struct social_type ) );
  total += social_count * sizeof(struct social_type );
  fprintf( fp, "Spells      %5d    mem %7d\n", count_spell,
		 count_spell * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_spell * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  fprintf( fp, "Skills      %5d    mem %7d\n", count_skill,
		 count_skill * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_skill * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  fprintf( fp, "Powers      %5d    mem %7d\n", count_power,
		 count_power * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_power * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  fprintf( fp, "Songs       %5d    mem %7d\n", count_song,
		 count_song * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_song * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  fprintf( fp, "SpecialAff  %5d    mem %7d\n", count_affect,
		 count_affect * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int)) );
  total += count_song * (sizeof(struct ability_type)+MAX_CLASS*2*sizeof(int));
  fprintf( fp, "Mobs        %5d    mem %7d\n", top_mob_index,
		 top_mob_index * sizeof( MOB_INDEX_DATA ) );
  total += top_mob_index * sizeof( MOB_INDEX_DATA );
  fprintf( fp, "(in use)    %5d    mem %7d\n", mobile_count,
		 mobile_count * sizeof( CHAR_DATA ) );
  total += mobile_count * sizeof( CHAR_DATA );
  fprintf( fp, "Objs        %5d    mem %7d\n", top_obj_index,
		 top_obj_index * sizeof( OBJ_INDEX_DATA ) );
  total += top_obj_index * sizeof( OBJ_INDEX_DATA );
  fprintf( fp, "(in use)    %5d    mem %7d\n", object_count,
		 object_count * sizeof( OBJ_DATA ) );
  total += object_count * sizeof( OBJ_DATA );
  fprintf( fp, "Resets      %5d    mem %7d\n", top_reset,
		 top_reset * sizeof( RESET_DATA ) );
  total += top_reset * sizeof( RESET_DATA );
  fprintf( fp, "Rooms       %5d    mem %7d\n", top_room,
		 top_room * sizeof( ROOM_INDEX_DATA ) );
  total += top_room * sizeof( ROOM_INDEX_DATA );
  fprintf( fp, "Shops       %5d    mem %7d\n", top_shop,
		 top_shop * sizeof( SHOP_DATA ) );
  total += top_shop * sizeof( SHOP_DATA );
  fprintf( fp, "Commands    %5d    mem %7d\n", MAX_COMMANDS,
		 MAX_COMMANDS * sizeof( cmd_type ));
  total += MAX_COMMANDS * sizeof( cmd_type );
  fprintf( fp, "Classes     %5d    mem %7d\n", MAX_CLASS,
		 MAX_CLASS * sizeof( class_type ));
  total += MAX_CLASS * sizeof(class_type);
  fprintf( fp, "Groups      %5d    mem %7d\n", MAX_GROUP,
		 MAX_GROUP * (sizeof(group_type)+MAX_CLASS*sizeof(int)+MAX_GODS*sizeof(int)));
  total += MAX_GROUP * (sizeof(group_type)+MAX_CLASS*sizeof(int)+MAX_GODS*sizeof(int));
  fprintf( fp, "Gods        %5d    mem %7d\n", MAX_GODS,
		 MAX_GODS * sizeof(god_data));
  total += MAX_GODS * sizeof(god_data); // wrong approx
  fprintf( fp, "Races       %5d    mem %7d\n", MAX_RACE + MAX_PC_RACE,
		 MAX_RACE * sizeof(race_type) + MAX_PC_RACE * sizeof(pc_race_type));
  total += MAX_RACE * sizeof(race_type) + MAX_PC_RACE * sizeof(pc_race_type); // wrong approx

  fprintf( fp, "Total                mem %7ld\n\n", total );

  fprintf(fp,"=====================================\n");

  fprintf(fp,"Garbage Collector:\n"
	  "-----------------\n");
  fprintf(fp,"Heap size      : %d\n", GC_get_heap_size());
  fprintf(fp,"Free bytes     : %d\n", GC_get_free_bytes());
  fprintf(fp,"Bytes allocated: %d\n", GC_get_bytes_since_gc());
  fprintf(fp,"(Uncollectable): %d [upper bound]\n", 
		GC_get_heap_size() - GC_get_free_bytes() - GC_get_bytes_since_gc());
  fprintf(fp,"\n");
  fprintf(fp,
		"String Space:\n"
		"-------------\n");
  fprintf(fp,"Total used     : %d\n", get_str_space_size() );
  fprintf(fp,"Free entries   : %d\n", get_str_free_entries());
  fprintf(fp,"nStrDup         : %d\n", nStrDup );
  fprintf(fp,"nStrDupEmpty    : %d\n", nStrDupEmpty );
  fprintf(fp,"nStrDupGC       : %d\n", nStrDupGC );
  fprintf(fp,"nStrDupNoGC     : %d\n", nStrDupNoGC );
  fprintf(fp,"nStrDupUnsafe   : %d\n", nStrDupUnsafe );
  fprintf(fp,"nAllocPerm      : %d\n", nAllocPermStr );
  fprintf(fp,"sAllocPerm      : %d\n", sAllocPermStr );
  fprintf(fp,"nAllocPermGC    : %d\n", nAllocPermGC );
  fprintf(fp,"nAllocPermHash  : %d\n", nAllocPermHash );
  fprintf(fp,"nAllocPermNoHash: %d\n", nAllocPermNoHash );
  fprintf(fp,"sStrDupNoGC     : %d\n", sStrDupNoGC );
  fprintf(fp,"sStrDupUnsafe   : %d\n", sStrDupUnsafe );
  fprintf(fp,"\n");


  fprintf(fp,"Fighting mob:\n");
  for ( CHAR_DATA *mob = char_list; mob != NULL; mob = mob->next )
    if ( mob->fighting != NULL )
      fprintf(fp, "Room %5d :  %s(%d) is fighting %s(%d)\n",
	      mob->in_room?mob->in_room->vnum:-1,
	      NAME(mob), IS_NPC(mob)?mob->pIndexData->vnum:-1,
	      NAME(mob->fighting), IS_NPC(mob->fighting)?mob->fighting->pIndexData->vnum:-1 );


  fprintf(fp,"=====================================\n");
  fprintf(fp,"<<<<<<<<<<< END OF REPORT >>>>>>>>>>>\n");
  fprintf(fp,"=====================================\n");

  logf("End Of Report");
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
  return;
}

void save_on_shutdown() {
  // Saving clan table -- Oxtal
  new_save_clans();
  // Added by SinaC 2001 to save unique items
  new_save_unique_items();
  // SinaC 2003: current time/weather/moon is saved
  save_time();
}

void auto_copyover_update() {
  // Heap too large ?
  int size = GC_get_heap_size();
  if ( size > MAX_HEAP_SIZE ) {
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "<MAX HEAP SIZE EXCEEDED: %d [max: %d]>", size, MAX_HEAP_SIZE );
    log_stringf( buf );
    log_shutdown( buf );
    copyover_perform("\n\r *** Auto CopyOver - please remain seated!\n\r","");
    perror ("auto_copyover: execl"); 
  }
}
