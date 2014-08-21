/*
 * SillyMUD Distribution V1.1b             (c) 1993 SillyMUD Developement
 * See license.doc for distribution terms.   SillyMUD is based on DIKUMUD
 *
 * Modifications by Rip in attempt to port to merc 2.1
 */

/*
 * Modified by Turtle for Merc22 (07-Nov-94)
 *
 * I got this one from ftp.atinc.com:/pub/mud/outgoing/track.merc21.tar.gz.
 * It cointained 5 files: README, hash.c, hash.h, skills.c, and skills.h.
 * I combined the *.c and *.h files in this hunt.c, which should compile
 * without any warnings or errors.
 */

/*
 * Some systems don't have bcopy and bzero functions in their linked libraries.
 * If compilation fails due to missing of either of these functions,
 * define NO_BCOPY or NO_BZERO accordingly.                 -- Turtle 31-Jan-95
#define NO_BCOPY
#define NO_BZERO
 */

/*
 * Optimized by Oxtal.
 * Added the do_path and do_track commands
 */


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "merc.h"
#include "interp.h"

#include <string.h>
/* command procedures needed */

// Added by SinaC 2001
#include "handler.h"
#include "comm.h"
#include "act_move.h"
#include "db.h"
#include "fight.h"
#include "gsn.h"
#include "olc_value.h"
#include "noncombatabilities.h"
#include "hunt.h"
#include "act_comm.h"
#include "ability.h"
#include "utils.h"

const char * hunt_short_dir_name[]     = {"n","e","s","w","u","d","NE","NW","SE","SW", "X"};

typedef	struct hash_link HASH_LINK;

struct hash_link {
  int			key;
  struct hash_link	*next;
  char			dir;
  char			prec_dir;
  int			prec_room;  
};

struct hash_header {
  int			table_size;
  struct hash_link	**buckets;
};

#define WORLD_SIZE	30000
#define	HASH_KEY(ht,key)((((unsigned int)(key))*17)%(ht)->table_size)



struct hunting_data {
  char			*name;
  struct char_data	**victim;
};

struct room_q {
  int		room_nr;
  struct room_q	*next_q;
};

struct nodes {
  int	visited;
  int	ancestor;
};

#define IS_DIR		(get_room_index(q_head->room_nr)->exit[i])
#define GO_OK		(!IS_SET( IS_DIR->exit_info, EX_CLOSED ))
#define GO_OK_SMARTER	1


void init_hash_table(struct hash_header	*ht, int table_size) {
  ht->table_size= table_size;
  ht->buckets	= (hash_link**)GC_MALLOC(sizeof(struct hash_link**) * table_size);
}

void destroy_hash_table(struct hash_header *ht) {
  int			i;
  struct hash_link	*scan,*temp;
  
  if (!ht->buckets) return;
   
  for(i=0;i<ht->table_size;i++)
    for(scan=ht->buckets[i];scan;) {
      temp = scan->next;
      scan = temp;
    }
}

void hash_enter(struct hash_header *ht,int key,char dir,int prec, char pdir) {
  /* precondition: there is no entry for <key> yet */
  struct hash_link	*temp;

  temp		= (struct hash_link *)GC_MALLOC(sizeof(struct hash_link));
  temp->key	= key;
  temp->next	= ht->buckets[HASH_KEY(ht,key)];
  temp->dir	= dir;
  temp->prec_room = prec;
  temp->prec_dir = pdir;
  ht->buckets[HASH_KEY(ht,key)] = temp;

}

HASH_LINK * hash_find(struct hash_header *ht,int key) {
  struct hash_link *scan;

  scan = ht->buckets[HASH_KEY(ht,key)];

  while(scan && scan->key!=key)
    scan = scan->next;

  return scan;
}

int exit_ok( EXIT_DATA *pexit ) {
  ROOM_INDEX_DATA *to_room;

  if ( ( pexit == NULL )
       ||  ( to_room = pexit->u1.to_room ) == NULL 
       || IS_SET( to_room->cstat(flags), ROOM_NOTRACK ) ) // SinaC 2003
    return 0;

  return 1;
}

// Re-handled by Oxtal
int find_path( int in_room_vnum, int out_room_vnum, CHAR_DATA *ch, 
	       int depth, int in_zone, char *outpath ) {
  struct room_q		*tmp_q, *q_head, *q_tail;
  struct hash_header	x_room;
  int			i, tmp_room, thru_doors, res;
  int			count = 0;
  ROOM_INDEX_DATA	*herep;
  ROOM_INDEX_DATA	*startp;
  EXIT_DATA		*exitp;

  if ( depth <0 ) {
    thru_doors = TRUE;
    depth = -depth;
  }
  else
    thru_doors = FALSE;

  startp = get_room_index( in_room_vnum );

  init_hash_table( &x_room, 2048 );
  hash_enter( &x_room, in_room_vnum, -1, 0, -1);

  /* initialize queue */
  q_head = (struct room_q *) GC_MALLOC(sizeof(struct room_q));
  q_tail = q_head;
  q_tail->room_nr = in_room_vnum;
  q_tail->next_q = 0;

  while(q_head && q_head->room_nr != out_room_vnum)  {
    herep = get_room_index( q_head->room_nr );

    if ( herep->area == startp->area || !in_zone)
      /* for each room test all directions */
      for( i = 0; i < MAX_DIR; i++ ) { // Modified by SinaC 2003
	  
	exitp = herep->exit[i];
	if( exit_ok(exitp) && ( thru_doors ? GO_OK_SMARTER : GO_OK ) ) {      
	  tmp_room = exitp->u1.to_room->vnum;        
	  if(   !hash_find( &x_room, tmp_room )
		&& count < depth ) {
	    count++;
	    /* mark room as visted and put on queue */
	  
	    tmp_q = (struct room_q *) GC_MALLOC(sizeof(struct room_q));
	    tmp_q->room_nr = tmp_room;
	    tmp_q->next_q = 0;
	    q_tail->next_q = tmp_q;
	    q_tail = tmp_q;
	      
	    /* ancestor for first layer is the direction */
	    hash_enter( &x_room, tmp_room,
			( hash_find(&x_room,q_head->room_nr)->dir == -1) 
			? i
			: hash_find(&x_room,q_head->room_nr)->dir,
			q_head->room_nr, i);
	  }
	}
      }  
    /* free queue head and point to next entry */
    tmp_q = q_head->next_q;
    q_head = tmp_q;
  }

  if (q_head) {
    /* have reached our goal so free queue */
    tmp_room = q_head->room_nr;

    for(;q_head;q_head = tmp_q) {
      tmp_q = q_head->next_q;
    }
    
    res = hash_find(&x_room,tmp_room)->dir;
/*    if (outpath) {
      char path[MAX_INPUT_LENGTH];
      int j;
      HASH_LINK * h;
      i = 0;
      while (tmp_room != in_room_vnum) {
        h = hash_find(&x_room,tmp_room);
        path[i++] = dir_name[(int)h->prec_dir][0];
        tmp_room = h->prec_room;
      }

      count = 1; 
      j = 0;
      while (--i>=0) {
        if (i>0 && path[i-1] == path[i])
          count++;
        else {
          if (count>1) {
            sprintf(outpath+j,"%d",count);
            j = strlen(outpath);
          }
          outpath[j++] = path[i];
          count=1;
        }
      }
      outpath[j] = 0;
    }   

  }
*/
    if ( outpath ) {
      int path[MAX_STRING_LENGTH];
      int j;
      HASH_LINK * h;
      i = 0;
      while (tmp_room != in_room_vnum) {
        h = hash_find(&x_room,tmp_room);
        path[i++] = h->prec_dir; // put direction id instead of direction first letter
        tmp_room = h->prec_room;
      }

      count = 1; 
      j = 0;
      while (--i>=0) {
        if (i>0 && path[i-1] == path[i])
          count++;
        else {
          if (count>1) {
            sprintf(outpath+j,"%d",count);
            j = strlen(outpath);
          }
	  strcpy(outpath+j,hunt_short_dir_name[path[i]]);
	  j+=strlen(hunt_short_dir_name[path[i]]);
          count=1;
        }
      }
      outpath[j] = '\0';
    }
  }
  else
    res = -1;
    
  destroy_hash_table(&x_room);

  return res;
}

void do_path( CHAR_DATA *ch, const char *argument )
{
  ROOM_INDEX_DATA *location;
  int direction;
  char path[MAX_INPUT_LENGTH];

  if ( argument[0] == '\0' ) {
    send_to_char( "Path to where?\n\r", ch );
    return;
  }

  if ( ( location = find_location( ch, argument ) ) == NULL ) {
    send_to_char( "No such location.\n\r", ch );
    return;
  }
    
  if (ch->in_room == location) {
    send_to_char( "You are there!\n\r",ch );
    return;
  }
    
  direction = find_path( ch->in_room->vnum, location->vnum,
			 ch, -40000, FALSE ,path );
   
  if (direction == -1){
    send_to_charf(ch,"No path to %s.\n\r",location->name);
    return;
  }

  send_to_charf(ch,"This path will lead to %s. \n\r"
		" {c%s{x\n\r",location->name,path);
}


void do_track( CHAR_DATA *ch, const char *argument )
{
  ROOM_INDEX_DATA *location;
  int direction;

  if ( argument[0] == '\0' ) {
    send_to_char( "Want to go where?\n\r", ch );
    return;
  }

  if ( ( location = find_location( ch, argument ) ) == NULL ) {
    send_to_char( "No such location.\n\r", ch );
    return;
  }
    
  if (ch->in_room == location) {
    send_to_char( "You are already there!\n\r",ch );
    return;
  }
    
  direction = find_path( ch->in_room->vnum, location->vnum,
			 ch, -40000, ch->level < MAX_LEVEL,NULL );
   
  if (direction == -1){
    send_to_charf(ch,"No way to %s.\n\r",location->name);
    return;
  }
  if ( IS_SET( ch->in_room->exit[direction]->exit_info, EX_CLOSED ) )
    do_open(ch, dir_name[direction]);

  send_to_charf(ch,"{r\n\r>>> Going %s!\n\r{x",dir_name[direction] );
  move_char(ch,direction,FALSE, FALSE, FALSE ); // SinaC 2003);
}

void do_hunt( CHAR_DATA *ch, const char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int direction;
  bool fArea;
  /* Sinac 1997 */  
  //  CHAR_DATA *gch;
  //  bool mount;
 
  if (!get_ability(ch,gsn_hunt)) {
    send_to_char("You don't know hunting techniques.\n\r",ch);
    return;
  }

  one_argument( argument, arg );

  if( arg[0] == '\0' ) {
    send_to_char( "Whom are you trying to hunt?\n\r", ch );
    return;
  }

  /* only imps can hunt to different areas */
  fArea = ( get_trust(ch) < MAX_LEVEL );

  if( fArea )
    victim = get_char_world( ch, arg );
  else
    victim = get_char_world( ch, arg );

  if( victim == NULL 
      || !can_see( ch, victim ) ) { // Added by SinaC 2003
    send_to_char("No-one around by that name.\n\r", ch );
    return;
  }

  if( ch->in_room == victim->in_room ) {
    act( "$N is here!", ch, NULL, victim, TO_CHAR );
    return;
  }

  /*
   * Deduct some movement.
   */
  if( ch->move > 2 )
    ch->move -= 3;
  else {
    send_to_char( "You're too exhausted to hunt anyone!\n\r", ch );
    return;
  }

  act( "$n carefully sniffs the air.", ch, NULL, NULL, TO_ROOM );
  WAIT_STATE( ch, BEATS(gsn_hunt) );
  check_improve( ch, gsn_hunt, TRUE, 3 );
  direction = find_path( ch->in_room->vnum, victim->in_room->vnum,
			 ch, -40000, fArea, NULL );

  if( direction == -1 ) {
    act( "You couldn't find a path to $N from here.",
	 ch, NULL, victim, TO_CHAR );
    return;
  }

  if( direction < 0 || direction >= MAX_DIR ) { // Modified by SinaC 2003
    send_to_char( "Hmm... Something seems to be wrong.\n\r", ch );
    return;
  }

  /*
   * Give a random direction if the player misses the die roll.
   */
  if( ( IS_NPC (ch) && number_percent () > 75)        /* NPC @ 25% */
      || (!IS_NPC (ch) && number_percent () >          /* PC @ norm */
	  // Modified by SinaC 2000
	  get_ability(ch,gsn_hunt)/*ch->pcdata->learned[gsn_hunt]*/ ) ) {
    do {
      direction = number_door();
    }
    while( ( ch->in_room->exit[direction] == NULL )
	   || ( ch->in_room->exit[direction]->u1.to_room == NULL) );
  }

  /*
   * Display the results of the search.
   */
  sprintf( buf, "$N is %s from here.", dir_name[direction] );
  act( buf, ch, NULL, victim, TO_CHAR );

  return;
}

bool hunt_victim( CHAR_DATA *ch )
{
  int		dir;
  bool		found;
  CHAR_DATA	*tmp;

  // SinaC 2000
  // if the hunter is fighting ==> return FALSE
  if( ch == NULL 
      || !ch->valid
      || ch->hunting == NULL 
      || !IS_NPC(ch) 
      || !IS_AWAKE(ch) 
      || ch->fighting 
      || IS_AFFECTED(ch,AFF_CHARM) )
    return FALSE;

  // Make sure the victim still exists.
  found = FALSE;
  for ( tmp = char_list; tmp!=NULL; tmp = tmp->next )
    if ( ch->hunting == tmp ) {
      found = TRUE;
      break;
    }

  if( !found 
      || !can_see( ch, ch->hunting ) ) {
    do_say( ch, "Damn!  My prey is gone!!" );
    ch->hunting = NULL;
    return TRUE;
  }

  if( ch->in_room == ch->hunting->in_room 
      && !is_safe( ch, ch->hunting ) ) {
    act( "$n glares at $N and says, 'Ye shall DIE!'",
	 ch, NULL, ch->hunting, TO_NOTVICT );
    act( "$n glares at you and says, 'Ye shall DIE!'",
	 ch, NULL, ch->hunting, TO_VICT );
    act( "You glare at $N and say, 'Ye shall DIE!",
	 ch, NULL, ch->hunting, TO_CHAR);
    multi_hit( ch, ch->hunting, TYPE_UNDEFINED );
    // Removed by SinaC 2000, the hunter continues to hunt his/her victim
    //      ch->hunting = NULL;
    return TRUE;
  }
  
  WAIT_STATE( ch, BEATS(gsn_hunt) );

  // could be fun if the TRUE becomed a FALSE ==> the mob will hunt the player
  //  even if he's in another area  SinaC 2000 ... DONE :)
  dir = find_path( ch->in_room->vnum, ch->hunting->in_room->vnum,
		   ch, -40000, FALSE, NULL );

  // Added by SinaC 2000, if the hunted is in a safe room and the mob is in the same room
  //  the mob will consider it as lost cos' dir will be -1
  // So, if the mob is in the same room and can't kill his victim, we give him a random dir
  if ( ch->in_room == ch->hunting->in_room ) {
    //
    //  act( "$n says 'I can't punish you for the moment $N, but I'll kill you later!'",
    //  ch, NULL, ch->hunting, TO_ROOM );
    do
      dir = number_door();
    while( ( ch->in_room->exit[dir] == NULL )
	   || ( ch->in_room->exit[dir]->u1.to_room == NULL ) );
  }
  else
    if( dir < 0 || dir >= MAX_DIR ) { // Modified by SinaC 2003
      act( "$n says 'Damn!  Lost $M!'", ch, NULL, ch->hunting, TO_ROOM );
      ch->hunting = NULL;
      return TRUE;
    }
  
  /*
   * Give a random direction if the mob misses the die roll.
   */
  if( number_percent () > 75 ) {       /* @ 25% */
    do
      dir = number_door();
    while( ( ch->in_room->exit[dir] == NULL )
	   || ( ch->in_room->exit[dir]->u1.to_room == NULL ) );
  }
  
  if( IS_SET( ch->in_room->exit[dir]->exit_info, EX_CLOSED ) ) {
    // Added by SinaC 2000
    if ( IS_SET(ch->in_room->exit[dir]->exit_info, EX_LOCKED) )	{
      
      do_unlock( ch, (char *) dir_name[dir] );
      do_pick( ch, (char *) dir_name[dir] );
    }
    
    do_open( ch, (char *) dir_name[dir] );
    return TRUE;
  }
  
  move_char( ch, dir, FALSE, FALSE, FALSE ); // SinaC 2003 );
  return TRUE;
}

// Remove every mob hunting a player
void remove_hunter( CHAR_DATA *ch )
{
  CHAR_DATA *hunter;
  
  for ( hunter = char_list; hunter != NULL; hunter = hunter->next ) {
    // a PC can't hunt
    if ( !IS_NPC( hunter ) ) continue;
    if ( hunter->hunting == ch ) {
      log_stringf("remove_hunter: %s stops hunting %s",hunter->short_descr, ch->name );
      hunter->hunting = NULL;
    }
  }
}
