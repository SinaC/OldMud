/*
 *
 * Modified by SinaC 2000
 *
 *
 * This is a new automated arena for Smaug 1.4.
 * You can do anything you want with this code, I hope it will be
 * compatible with the DOS version.
 *
 * INSTALLATION:
 * Add to mud.h
 * in pc_data ...
 * char *     betted_on;
 * int 	      bet_amt;
 * down at the bottom of mud.h with all the rest of this stuff ...
 * #define GET_BETTED_ON(ch)    ((ch)->betted_on)
 * #define GET_BET_AMT(ch) ((ch)->bet_amt)
 *
 * change around the Makefile to include this file,
 * You also have to set the room flags in the limbo.are for the arena.
 * The arena bit is 67108864 (It's not included in the help roomflags)
 * This snippet is based on the ROM arena snippet by Kevin Hoogheem
 * It was ported to SMAUG1.4 by LrdElder
 * If you have any cool additions or questions just e-mail me at
 * tdison@swetland.net - LrdElder 10/24/98
 */

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

// Added by SinaC 2001
#include "handler.h"
#include "comm.h"
#include "db.h"
#include "fight.h"
#include "olc_value.h"
#include "interp.h"
#include "arena.h"
#include "act_info.h"
#include "config.h"
#include "act_move.h"
#include "utils.h"

struct hall_of_fame_element {
  char name[MAX_INPUT_LENGTH+1];
  time_t date;
  int award;
  struct  hall_of_fame_element *next;
};


void sportschan(const char *);
void start_arena();
void show_jack_pot();
void do_game();
int num_in_arena();
void find_game_winner();
void do_end_game();
void start_game();
void silent_end();
void find_bet_winners(CHAR_DATA *winner);

void not_enough_end();
void abort_arena();

int num_in_waiting();

void write_fame_list(void);
void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node);
void load_hall_of_fame(void);

struct hall_of_fame_element *fame_list = NULL;

int ppl_challenged = 0;
int ppl_in_arena = 0;
int in_start_arena = 0;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int time_to_start;
int time_left_in_game;
int bet_pot;
int barena = 0;



// Added by SinaC 2000, to be modified use ROOM_ARENA instead of VNUM
void obj_from_arena_to_donation()
{
  OBJ_DATA *obj, *obj_next;
  ROOM_INDEX_DATA *room;

  for ( int vnum = ARENA_FROM; vnum <= ARENA_TO; vnum++ ){
    room = get_room_index( vnum );
    
    for ( obj = room->contents; obj != NULL; obj = obj_next ){
      obj_next = obj->next_content;
      
      if ( !CAN_WEAR( obj, ITEM_TAKE ) ) continue;
      
      SET_OBJ_STAT( obj, ITEM_DONATED);
      
      obj_from_room( obj );
      //obj_to_room( obj, get_room_index( ROOM_VNUM_DONATION ) );
      obj_to_room( obj, get_room_index( DEFAULT_DONATION ) ); //SinaC 2003
    }
  }
}

void do_betarena(CHAR_DATA *ch, const char *argument)
{
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  int newbet;
  CHAR_DATA *bet_on;
       
  argument = one_argument(argument,arg);
  argument = one_argument(argument,buf1);
           
  if (IS_NPC(ch)){
    send_to_char("Mobs cant bet on the arena.\n\r",ch);
    return;
    }
                    
  if(arg[0]=='\0'){
    send_to_char("Usage: betarena <player> <amt>\n\r",ch);
    return;
  }
  else if(!in_start_arena && !ppl_challenged){
    send_to_char("Sorry the arena is closed, wait until it opens up to bet.\n\r", ch);
    return;
  }
  else if(ppl_in_arena){
    send_to_char("Sorry Arena has already started, no more bets.\n\r", ch);
    return;
  }
  // Added by SinaC 2000
  else if (GET_BETTED_ON(ch)!=NULL)
    send_to_charf(ch,"You have already betted %d coins on %s.\n\r", 
		  GET_BET_AMT(ch),GET_BETTED_ON(ch)->name);
  
  else if (!(bet_on = get_char_world(ch, arg)))
    send_to_char("No such person exists in the Necropolis.\n\r", ch);
  else if (bet_on == ch)
    send_to_char("That doesn't make much sense, does it?\n\r", ch);
  else if(!(IN_WAITING(bet_on)))
    send_to_char("Sorry that person is not in the arena.\n\r", ch);
  else {
    if(GET_BET_AMT(ch) > 0){
      // we should never arrive here cos' of precedent test.
      send_to_char("Sorry you have already bet.\n\r", ch);
      ch->pcdata->bet_amt = 0;
      return;
    }
    // Modified by SinaC 2000
    newbet = atoi(buf1);
    if ( newbet < 0 ){
      send_to_char("Your bet has to be positive!\n\r", ch );
      return;
    }

    if(newbet == 0){
      send_to_char("Bet some gold why dont you!\n\r", ch);
      return;
    }
    if (newbet > ch->gold){
      send_to_char("You don't have that much money!\n\r",ch);
      return;
    }
    if(newbet > 10000){
      send_to_char("Sorry the house will not accept that much.\n\r", ch);
      return;
    }
    
    ch->pcdata->betted_on = bet_on;
    ch->gold -= newbet;
    bet_pot += newbet;
    
    GET_BET_AMT(ch) = newbet;
    sprintf(buf, "You place %d coins on %s.\n\r", newbet, GET_BETTED_ON(ch)->name);
    send_to_char(buf, ch);
    /*      sprintf(buf,"{RARENA: %s has placed %d coins on %s.{x\n\r", ch->name,
     *      newbet, ch->betted_on->name);
     *sportschan(buf);
     */
  }
}

void do_arena(CHAR_DATA *ch, const char *argument)
{
  char buf[MAX_INPUT_LENGTH];
 
  if (IS_NPC(ch)){
    send_to_char("Mobs cant play in the arena.\n\r",ch);
    return;
  }

  if (IS_IMMORTAL(ch)){
    send_to_char("Let mortals have fun without you.\n\r",ch);
    return;
  }

  if (ch->fighting != NULL ){
    send_to_char("You're still fighting!\n\r",ch);
    return;
  }
  
  if(!in_start_arena){
    send_to_char("The killing fields are closed right now.\n\r", ch);
    return;
  }
 
  if(ch->level < lo_lim){
    sprintf(buf, "Sorry but you must be at least level %d to enter this arena.\n\r", lo_lim);
    send_to_char(buf, ch);
    return;	
  }
 
  if( ch->level > hi_lim){
    send_to_char("This arena is for lower level characters.\n\r", ch);
    return;
  } 
 
  // Modified by SinaC 2000
  if(IN_BATTLE(ch) || IN_WAITING(ch)){ 
    send_to_char("You are in the arena already\n\r",ch);
    return;
  }	
  else{
    sprintf(buf, "{RARENA: %s has joined the blood bath.{x\n\r", ch->name);
    sportschan(buf);
    act("$n has been whisked away to the killing fields.", ch, NULL, NULL, TO_ROOM);
    char_from_room(ch);
    char_to_room(ch, get_room_index(PREP_START)); 
    act("$n is dropped from the sky.", ch, NULL, NULL, TO_ROOM);
    send_to_char("You have been taken to the killing fields\n\r",ch);
    do_look(ch, "auto");
    
    ch->hit = ch->cstat(max_hit);
    ch->mana = ch->cstat(max_mana);
    // Added by SinaC 2001 for mental user
    ch->psp = ch->cstat(max_psp);
    ch->move = ch->cstat(max_move); 
    return;
  }
}

void do_chaos(CHAR_DATA *ch, const char *argument)
{
  char lolimit[MAX_INPUT_LENGTH];
  char hilimit[MAX_INPUT_LENGTH], 
    start_delay[MAX_INPUT_LENGTH];
  char length[MAX_INPUT_LENGTH], 
    buf[MAX_INPUT_LENGTH];
          
  /*Usage: chaos lo hi start_delay cost/lev length*/

  argument = one_argument(argument, lolimit);
  argument = one_argument(argument, hilimit);
  argument = one_argument(argument, start_delay);
  one_argument(argument, length);

  // Added by SinaC 2000
  // Abort an arena
  if ( !str_cmp( lolimit, "abort" )
       || !str_cmp( lolimit, "reset" )
       || !str_cmp( lolimit, "cancel" ) ){
    abort_arena();
    return;
  }

  // Modified by SinaC 2000
  if ( ppl_in_arena == 1 || in_start_arena == 1 || ppl_challenged == 1 ){
    send_to_charf(ch,"Arena current values: LowLim %d  HiLim %d  Delay %d  Length %d\n\r", lo_lim,
		  hi_lim, start_time, game_length);
    send_to_charf(ch,"                      PplIn %d  PplCh %d\n\r", ppl_in_arena, ppl_challenged );
    send_to_charf(ch,"                      ToStart %d  TimeLeft %d  InStart %d\n\r", time_to_start, time_left_in_game, in_start_arena );
    send_to_charf(ch,"                      NbInArena %d  NbInWait %d\n\r", num_in_arena(),num_in_waiting());
    return;
  }
  if ( !*lolimit || !*hilimit || !*start_delay || !*length ){
    send_to_char("Syntax:\n\r", ch ); 
    send_to_char("  Chaos <lo> <hi> <start_delay> <length>\n\r", ch);
    send_to_char("  Chaos abort/cancel/reset\n\r", ch );
    return;
  }
  
  lo_lim = atoi(lolimit);
  hi_lim = atoi(hilimit);
  start_time = atoi(start_delay);
  game_length = atoi(length);
  
  sprintf(buf,"Chaos has been set to: LowLim %d HiLim %d Delay %d Length %d\n\r", lo_lim,
	  hi_lim, start_time, game_length);
  send_to_char(buf,ch);
  
  if(hi_lim > IMPLEMENTOR ){
    send_to_char("Please choose a hi_lim under the Imps level\n\r", ch);
    return;
  }
              
  if (lo_lim > hi_lim){
    send_to_char("Ya that just might be smart.\n\r", ch);
    return;
  }
                              
  if (lo_lim < 0 || hi_lim < 0 || game_length < 0 || start_time < 0 ){
    send_to_char("I like positive numbers thank you.\n\r", ch);
    return;
  }
                   
  if ( game_length == 0 ){
    send_to_char("Lets at least give them a chance to fight!\n\r", ch);
    return;
  }
                         
  if ( start_time == 0){
    send_to_char("Lets at least give them a chance to enter!\n\r", ch);
    return;
  }
  
  ppl_in_arena = 0;
  in_start_arena = 1;
  time_to_start = start_time;
  time_left_in_game =0;
  bet_pot = 0;
  barena = 1;

  start_arena();
}

void start_arena()
{
  char buf1[MAX_INPUT_LENGTH];
  char buf[MAX_INPUT_LENGTH];
  DESCRIPTOR_DATA *d;
  CHAR_DATA *rch;
  
  if (!(ppl_challenged)){   
    if(time_to_start == 0){
      // Added by SinaC 2000
      if ( num_in_waiting() == 1 ){
	ppl_in_arena = 0;
	ppl_challenged = 0;
	not_enough_end();
	return;
      }

      in_start_arena = 0;
      show_jack_pot();
      ppl_in_arena = 1;    /* start the blood shed */
      time_left_in_game = game_length;
      start_game();
    }
    else{
      if(time_to_start >1){
	sprintf(buf1, "{WThe Killing Fields are open to levels {R%d {Wthru {R%d{x\n\r",
		lo_lim, hi_lim);
	sprintf(buf1, "%s%d {Whours to start{x\n\r", buf1, time_to_start);
	sprintf(buf1, "%s\n\rType {Rarena {Wto enter.{x\n\r", buf1);
      }
      else{
	sprintf(buf1, "{WThe Killing Fields are open to levels {R%d {Wthru {R%d{x\n\r",
		lo_lim, hi_lim);
	sprintf(buf1, "%s1 {Whour to start{x\n\r", buf1);
	sprintf(buf1, "%s\n\rType {Rarena {Wto enter.{x\n\r", buf1);
      }
      for ( d = descriptor_list; d; d = d->next ){
	if ( d->connected != CON_PLAYING ) continue;
	rch = d->original ? d->original : d->character;
	// Modified by SinaC 2000   && !IN_WAITING...
	if ( rch->level>= lo_lim && rch->level <=hi_lim ){
	  if ( IN_WAITING( rch ) ){
	    if ( time_to_start > 1 )
	      sprintf(buf, "{R%d {Whour(s) to start{x\n\r", time_to_start);
	    else
	      sprintf(buf, "{R%d {Whour to start{x\n\r", time_to_start);
	    send_to_char( buf,rch );
	  }
	  else
	    send_to_char( buf1, rch );
	}
	else{
	  sprintf(buf, "{WThe arena has been opened. {R%d {Whour(s) to start.{x\n\r", time_to_start);
	  sprintf(buf, "%sPlace your bets!!!\n\r", buf);
	  send_to_char(buf,rch);
	}
      } 
      time_to_start--;
    }
  }  
  else
    if (!(ppl_in_arena)){
      if(time_to_start == 0){
	ppl_challenged = 0;
	show_jack_pot();
	ppl_in_arena = 1;    /* start the blood shed */
	time_left_in_game = 5;
	game_length = 5;
	start_game();
      }
      else{
	if(time_to_start >1){
	  sprintf(buf1, "{RCHALLENGE: The dual will start in %d hours. Place your bets!{x\n\r",
		  time_to_start);
	}
	else{
	  sprintf(buf1, "{RCHALLENGE: The dual will start in 1 hour. Place your bets!{x\n\r");
	}
	sportschan(buf1);
	time_to_start--;
      }
    }
}                      

void start_game()
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *rch;

  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    // Modified by SinaC 2000
    if (IN_WAITING(rch)){
      send_to_char("\n\rThe floor falls out from below, dropping you in the arena\n\r", rch);
      char_from_room(rch);
      char_to_room(rch,get_room_index(ARENA_START));
      do_look(rch,"auto");
    }
  }
  
  do_game();
}

void do_game()
{
  char buf[MAX_INPUT_LENGTH];
  
  // Added by SinaC 2000

  if(num_in_arena() == 1){
    ppl_in_arena = 0;
    ppl_challenged = 0;
    find_game_winner();
  }
  else if(time_left_in_game == 0){
    do_end_game();
  }
  else if(num_in_arena() == 0){
    ppl_in_arena = 0;
    ppl_challenged = 0;
    silent_end();
  }
  else if(time_left_in_game == 1){
    if (!ppl_challenged)
      sprintf(buf, "{RARENA: With 1 hour left in the game there are %d players left.{x\n\r", num_in_arena());
    else if (!ppl_in_arena)
      sprintf(buf, "{RCHALLENGE: 1 hour left in the game.{x\n\r");
    sportschan(buf);
  }
  else if(time_left_in_game <= 4){
    if (!ppl_challenged)
	sprintf(buf, "{RARENA: With %d hours left in the game there are %d players left.{x\n\r", time_left_in_game, num_in_arena());
    else if(!ppl_in_arena)
      sprintf(buf, "{RCHALLENGE: %d hours left in the game.{x\n\r", time_left_in_game);
    sportschan(buf);
  }
  time_left_in_game--;
}

void find_game_winner() {
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;

  int game_time;

  DESCRIPTOR_DATA *d;
  struct hall_of_fame_element *fame_node;

  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    
    if (!IS_NPC(rch)
	&&IN_BATTLE(rch)
	&& !IS_IMMORTAL(rch)) {
      game_time = game_length-time_left_in_game;
      
      if(game_time == 1){
	if (!ppl_challenged)
	  sprintf(buf, "{RARENA: After 1 hour of battle %s is declared the winner.\n\r{x",rch->name);
	else if (!ppl_in_arena)
	  sprintf(buf, "{RCHALLENGE: After 1 hour of battle %s is declared the winner.\n\r", rch->name);
	sportschan(buf);
      }
      else{
	if (!ppl_challenged)
	  sprintf(buf, "{RARENA: After %d hours of battle %s is declared the winner.\n\r{x", game_time, rch->name);
	else if (!ppl_in_arena)
	  sprintf(buf, "{RCHALLENGE: After %d hours of battle %s is declared the winner.{x\n\r", game_time, rch->name );
	sportschan(buf);
      }
      sprintf(buf, "You have been awarded %d coins for winning the arena.\n\r",
	      (bet_pot/2));
      send_to_char(buf, rch);
      
      rch->hit = rch->cstat(max_hit);
      rch->mana = rch->cstat(max_mana);
      // Added by SinaC 2001 for mental user
      rch->psp = rch->cstat(max_psp);
      rch->move = rch->cstat(max_move);
      rch->pcdata->challenged = NULL;
      
      char_from_room(rch);
      //char_to_room(rch,get_room_index(ROOM_VNUM_TEMPLE));
      char_to_room(rch,get_recall_room(rch));
      do_look(rch, "auto");
      
      act("$n falls from the sky.", rch, NULL, NULL, TO_ROOM);      
      
      rch->gold += bet_pot/2;
      
      sprintf(buf2, "%s awarded %d coins for winning arena.", rch->name,
	      (bet_pot/2));
      log_string(buf2);
      
      // only saving with arena, not challenge
      if (!ppl_challenged){
	fame_node = (struct hall_of_fame_element *) GC_MALLOC( sizeof( struct hall_of_fame_element ) );
	strncpy(fame_node->name, rch->name, MAX_INPUT_LENGTH);
	fame_node->name[MAX_INPUT_LENGTH] = '\0';
	fame_node->date = time(0);
	fame_node->award = (bet_pot/2);
	fame_node->next = fame_list;
	fame_list = fame_node;
	write_fame_list();
      }
      
      find_bet_winners(rch);
      ppl_in_arena = 0;
      ppl_challenged = 0;
    }
  }
  
  // Added by SinaC 2000
  obj_from_arena_to_donation();
}

void show_jack_pot()
{
  char buf1[MAX_INPUT_LENGTH];
  
  if (!ppl_challenged)
    sprintf(buf1, "{RARENA: Let's the fight begin!\n\r       %d coins has been bet on this arena.{x\n\r",bet_pot);
  else if (!ppl_in_arena)
    sprintf(buf1, "{RCHALLENGE: Let's the fight begin!\n\r           %d coins has been bet on this challenge.{x\n\r",bet_pot);
  
  sportschan( buf1 );
}

void silent_end()
{
  char buf[MAX_INPUT_LENGTH];

  ppl_in_arena = 0;
  ppl_challenged = 0;
  in_start_arena = 0;
  start_time = 0;
  game_length = 0;
  time_to_start = 0;
  time_left_in_game = 0;
  bet_pot = 0;

  sprintf(buf, "{RARENA: It looks like no one was brave enough to enter the Arena.{x\n\r");
  sportschan(buf);
}

// Added by SinaC 2000
void not_enough_end()
{
  char buf[MAX_INPUT_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;

  ppl_in_arena = 0;
  ppl_challenged = 0;
  in_start_arena = 0;
  start_time = 0;
  game_length = 0;
  time_to_start = 0;
  time_left_in_game = 0;
  bet_pot = 0;

  sprintf(buf, "{RARENA: There is not enough participants to do a valid arena.{x\n\r");
  sportschan(buf);

  DESCRIPTOR_DATA *d;
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
      
    if (!IS_NPC(rch)
	&& IN_WAITING(rch) && !IS_IMMORTAL(rch)){
      rch->hit = rch->cstat(max_hit);
      rch->mana = rch->cstat(max_mana);
      // Added by SinaC 2001 for mental user
      rch->psp = rch->cstat(max_psp);
      rch->move = rch->cstat(max_move);
      rch->pcdata->challenged = NULL;
      stop_fighting(rch,TRUE);
      char_from_room(rch);
      //char_to_room(rch, get_room_index(ROOM_VNUM_TEMPLE));
      char_to_room(rch, get_recall_room(rch));
      do_look(rch,"auto");
      act("$n falls from the sky.", rch, NULL, NULL, TO_ROOM);
    }
    if ((!IS_NPC(rch)) && (GET_BET_AMT(rch) > 0)){
      sprintf(buf1, "You get back your bet: %d coins.\n\r",GET_BET_AMT(rch));
      send_to_char(buf1, rch);
      rch->gold += GET_BET_AMT(rch);
      GET_BETTED_ON(rch) = NULL;
      GET_BET_AMT(rch) = 0;
    }
  }
}

// Added by SinaC 2000
void abort_arena()
{
  char buf[MAX_INPUT_LENGTH];
  char buf1[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;

  ppl_in_arena = 0;
  ppl_challenged = 0;
  in_start_arena = 0;
  start_time = 0;
  game_length = 0;
  time_to_start = 0;
  time_left_in_game = 0;
  bet_pot = 0;

  sprintf(buf, "{RARENA: The Arena has been aborted.{x\n\r");
  sportschan(buf);

  DESCRIPTOR_DATA *d;
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
      
    if ( !IS_NPC(rch) 
	 && ( IN_WAITING(rch) || IN_BATTLE(rch))
	 && !IS_IMMORTAL(rch)){
      rch->hit = rch->cstat(max_hit);
      rch->mana = rch->cstat(max_mana);
      // Added by SinaC 2001 for mental user
      rch->psp = rch->cstat(max_psp);
      rch->move = rch->cstat(max_move);
      rch->pcdata->challenged = NULL;
      stop_fighting(rch,TRUE);
      char_from_room(rch);
      //char_to_room(rch, get_room_index(ROOM_VNUM_TEMPLE));
      char_to_room(rch, get_recall_room(rch));
      do_look(rch,"auto");
      act("$n falls from the sky.", rch, NULL, NULL, TO_ROOM);
    }
    if ((!IS_NPC(rch)) && (GET_BET_AMT(rch) > 0)){
      sprintf(buf1, "You get back your bet: %d coins.\n\r",GET_BET_AMT(rch));
      send_to_char(buf1, rch);
      rch->gold += GET_BET_AMT(rch);
      GET_BETTED_ON(rch) = NULL;
      GET_BET_AMT(rch) = 0;
    }
  }
}
 
void do_end_game()
{
  char buf[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
  DESCRIPTOR_DATA *d;
      
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    
    if ( !IS_NPC(rch)
	 && IN_BATTLE(rch)){
      rch->hit = rch->cstat(max_hit);
      rch->mana = rch->cstat(max_mana);
      // Added by SinaC 2001 for mental user
      rch->psp = rch->cstat(max_psp);
      rch->move = rch->cstat(max_move);
      rch->pcdata->challenged = NULL;
      stop_fighting(rch,TRUE);
      char_from_room(rch);
      //char_to_room(rch, get_room_index(ROOM_VNUM_TEMPLE));
      char_to_room(rch, get_recall_room(rch));
      do_look(rch,"auto");
      act("$n falls from the sky.", rch, NULL, NULL, TO_ROOM);
    }
  }
  if (!ppl_challenged)
    sprintf(buf, "{RARENA: After %d hours of battle the Match is a draw.{x\n\r",game_length);
  else if (!ppl_in_arena)
    sprintf(buf, "{RCHALLENGE:  After %d hours of battle the Match is a draw.{x\n\r",game_length);
  
  // Added by SinaC 2000
  obj_from_arena_to_donation();
  
  sportschan(buf);
  time_left_in_game = 0;
  ppl_in_arena=0;
  ppl_challenged = 0;
}                       

// Added by SinaC 2000
int num_in_waiting()
{
  CHAR_DATA *rch;
  DESCRIPTOR_DATA *d;
  int num = 0;

  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    
    if (IN_WAITING(rch)){
      // Modified by SinaC 2000
      if(!IS_IMMORTAL(rch))
	num++;
    }
  }
  return num;
}

int num_in_arena()
{
  CHAR_DATA *rch;
  DESCRIPTOR_DATA *d;
  int num = 0;
  
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;  
    
    if (IN_BATTLE(rch)){
      // Modified by SinaC 2000
      if(!IS_IMMORTAL(rch))
	num++;
    }
  }
  return num;
}

void sportschan(const char *argument)
{
  info( argument  );
}

void do_awho(CHAR_DATA *ch, const char *argument)
{
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
  CHAR_DATA *rch;
          
  if ( num_in_arena()==0 && num_in_waiting()==0 ){
    send_to_char("There is noone in the arena right now.\n\r", ch);
    return;
  }
  
  sprintf(buf,"{W  Players in the {BNecropolis{W Arena\n\r");
  sprintf(buf,"%s-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{x", buf);
  sprintf(buf,"%s{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{x\n\r", buf);
  
  // Modified by SinaC 2000
  if ( in_start_arena == 1 )
    sprintf(buf,"%s{WGame Length: {R%-3d   {WTime To Start: {R%-3d{x\n\r", buf, game_length,time_to_start+1);
  else
    sprintf(buf,"%s{WGame Time Left: {R%-3d{x\n\r", buf, time_left_in_game+1 );
  
  sprintf(buf,"%s{WLevel Limits: {R%d {Wto {R%d{x\n\r", buf, lo_lim, hi_lim);
  sprintf(buf,"%s         {WTotal bet: {R%d{x\n\r", buf, bet_pot);
  // Added by SinaC 2000
  if ( num_in_arena() != 0 )
    sprintf(buf,"%s    {WPeople in the arena: {R%d{x\n\r", buf, num_in_arena());
  else
    sprintf(buf,"%s    {WPeople waiting: {R%d{x\n\r", buf, num_in_waiting());
  sprintf(buf,"%s{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B{x", buf);
  sprintf(buf,"%s-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B-{W-{B{x\n\r", buf);
  send_to_char(buf, ch);
  
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    
    if ((IN_BATTLE(rch)||IN_WAITING(rch)) && !IS_IMMORTAL(rch)){
      sprintf(buf2,"{W%-20.11s    level: %d{x\n\r", rch->name, rch->level);
      send_to_char(buf2,ch);
    }
  }
  return;	
}

void do_ahall(CHAR_DATA *ch, const char *argument)
{
  char site[MAX_INPUT_LENGTH], format[MAX_INPUT_LENGTH], *timestr;
  char format2[MAX_INPUT_LENGTH];
  struct hall_of_fame_element *fame_node;
      
  char buf[MAX_INPUT_LENGTH];
  char buf2[MAX_INPUT_LENGTH];
          
  if (!fame_list){
    send_to_char("No-one is in the Hall of Fame.\n\r", ch);
    return;
  }
  
  sprintf(buf2, "{B|---------------------------------------|\n\r");
  strcat(buf2, "| {WPast Winners of The Arena{B              |\n\r");
  strcat(buf2, "|---------------------------------------|{x\n\r\n\r");
  send_to_char(buf2, ch);
  
  strcpy(format, "{R%-25s  {R%-12s  {R%-16s{x\n\r");
  sprintf(buf, format,
	  "Name",
	  "Date",
	  "Award Amt");
  send_to_char(buf, ch);
  
  sprintf(buf, format,
	  "{B------------------------",
	  "{B------------",
	  "{B----------------");
  send_to_char(buf, ch);
  
  strcpy(format2, "{W%-25s  {R%-12s  {Y%-16d{x\n\r");
  for (fame_node = fame_list; fame_node; fame_node = fame_node->next){
    if (fame_node->date){
      timestr = asctime(localtime(&(fame_node->date)));
      *(timestr + 10) = 0;
      strcpy(site, timestr);
    }
    else
      strcpy(site, "Unknown");
    sprintf(buf, format2, fame_node->name, site, fame_node->award);
    send_to_char(buf, ch);
  }
  return;
}

void load_hall_of_fame(void)
{
  FILE *fl;
  int date, award;
  char name[MAX_INPUT_LENGTH + 1];
  struct hall_of_fame_element *next_node;
  int count;
  
  fame_list = 0;
  
  log_string("Reading Hall of Fame");

  if (!(fl = fopen(HALL_FAME_FILE, "r"))){
    sprintf( log_buf, "load_hall_of_fame: Unable to open %s.", HALL_FAME_FILE );
    log_string(log_buf);
    return;
  }
  count = 0;
  while (fscanf(fl, "%s %d %d", name, &date, &award) == 3){
    next_node = (struct hall_of_fame_element *) GC_MALLOC( sizeof( struct hall_of_fame_element ) );
    strncpy(next_node->name, name, MAX_INPUT_LENGTH);
    next_node->date = date;
    next_node->award = award;
    next_node->next = fame_list;
    fame_list = next_node;

    count++;
  }
  
  log_stringf(" %d winners found.",count);

  fclose(fl);
  return;
}
                                                        
void write_fame_list(void)
{
  FILE *fl;
  
  if (!(fl = fopen(HALL_FAME_FILE, "w"))){
    bug("write_fame_list: Error writing _hall_of_fame_list");
    return;
  }
  write_one_fame_node(fl, fame_list);// recursively write from end to start 
  fclose(fl);
  
  return;
}

void write_one_fame_node(FILE * fp, struct hall_of_fame_element * node)
{
  if (node){
    write_one_fame_node(fp, node->next);
    fprintf(fp, "%s %ld %d\n",node->name,(long) node->date, node->award);
  }
}

void find_bet_winners(CHAR_DATA *winner)
{
  DESCRIPTOR_DATA *d;
  CHAR_DATA *rch;
  char buf1[MAX_INPUT_LENGTH];
  int win_bet, win_pot;
  
  win_pot = 0;
  
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    
    if ((!IS_NPC(rch)) && (GET_BET_AMT(rch) > 0) && (GET_BETTED_ON(rch) == winner))
      win_pot += GET_BET_AMT(rch);
  }
  
  for ( d = descriptor_list; d; d = d->next ){
    if ( d->connected != CON_PLAYING ) continue;
    rch = d->original ? d->original : d->character;
    
    if ((!IS_NPC(rch)) && (GET_BET_AMT(rch) > 0) && (GET_BETTED_ON(rch) == winner))
      {
	win_bet = ( GET_BET_AMT(rch) * bet_pot ) / win_pot;
	sprintf(buf1, "You have won %d coins on your bet.\n\r",win_bet);
	send_to_char(buf1, rch);
	rch->gold += win_bet;
	GET_BETTED_ON(rch) = NULL;
	GET_BET_AMT(rch) = 0;
      }
  }
}

void do_challenge(CHAR_DATA *ch, const char *argument)
{
  CHAR_DATA *victim;
  char buf[MAX_INPUT_LENGTH];

  if ( argument[0] == '\0' ){
    send_to_char("Who do you want to challenge?\n\r", ch );
    return;
  }
  
  if ( IS_NPC(ch)){
    send_to_char("Mobiles are not allowed to participate in the arena.\n\r", ch );
    return;
  }
  
  if ( ( victim = get_char_world( ch, argument ) ) == NULL){
    send_to_char("{WThat character is not of these realms!{x\n\r",ch);  
    return;
  }
  
  if ( IS_IMMORTAL(ch) || IS_IMMORTAL(victim) ){
    send_to_char("Sorry, Immortal's are not allowed to participate in the arena.\n\r",ch);
    return;
  }
  
  if (IS_NPC(victim)){
    send_to_char("{WYou cannot challenge mobiles!{x\n\r",ch);
    return;
  }
  
  if (victim->name == ch->name){
    send_to_char("{WYou cannot challenge yourself!{x\n\r",ch);
    return;
  }
  
  if (victim->level<5 || get_age(victim) < 18){
    send_to_char("{WThat character is too young.{x\n\r",ch);
    return;
  }
  
  if ((!(ch->level-15<victim->level))||(!(ch->level+15>victim->level))){
    send_to_char("{WThat character is out of your level range.{x\n\r",ch);
    return;
  }
  
  if ( get_age( ch ) < 18 || ch->level < 5 ){
    send_to_char("You are too young die ( 18+ years).\n\r",ch);
    return;
  }
  
  if (num_in_arena()>0){
    send_to_char("{WSomeone is already in the arena!{x\n\r",ch);
    return;
  }
  sprintf(buf,"{R%s {Whas challenged you to a dual!{x\n\r",ch->name);
  send_to_char(buf,victim);
  send_to_char("{WPlease either accept or decline the challenge.{x\n\r\n\r",victim);
  sprintf(buf,"{RCHALLENGE: %s has challenged %s to a dual!!{x\n\r",ch->name,victim->name);
  sportschan(buf);
  victim->pcdata->challenged = ch;
}

void do_accept(CHAR_DATA *ch, const char *argument)
{
  char buf[MAX_INPUT_LENGTH];
  
  if ( IS_NPC(ch) ) {
    send_to_charf(ch,"Mobiles are not allowed to use this command.\n\r");
    return;
  }

  if (num_in_arena()>0){
    send_to_char("Please wait until the current arena is closed before you accept.\n\r",ch);
    return;
  }
  
  if (!(ch->pcdata->challenged)){
    send_to_char("You have not been challenged!\n\r",ch);
    return;
  }
  else{            
    CHAR_DATA *dch;
    dch = ch->pcdata->challenged;
    sprintf(buf,"{RCHALLENGE: %s has accepted %s's challenge!{x\n\r",
	    ch->name,dch->name);
    sportschan(buf);
    ch->pcdata->challenged = NULL;
    char_from_room(ch);
    char_to_room(ch, get_room_index(PREP_END));
    do_look(ch,"auto");
    char_from_room(dch);
    char_to_room(dch, get_room_index(PREP_START));
    do_look(dch,"auto");
    ppl_in_arena = 0;
    ppl_challenged = 1;
    time_to_start = 3;
    time_left_in_game =0;
    bet_pot = 0;
    start_arena();
    return;
  }
}

void do_decline(CHAR_DATA *ch, const char *argument)
{
  char buf[MAX_INPUT_LENGTH];

  if ( IS_NPC(ch) ) {
    send_to_charf( ch, "Mobiles are not allowed to use this command.\n\r" );
    return;
  }
  
  if (ch->pcdata->challenged){
    sprintf(buf,"{RCHALLENGE: %s has DECLINED %s's challenge! WHAT A WUSS!!!{x\n\r",
	    ch->name,ch->pcdata->challenged->name);
    sportschan(buf);
    ch->pcdata->challenged=NULL;
    return;
  }
  else{
    send_to_char("You have not been challenged!\n\r",ch);
    return;
  }
}
