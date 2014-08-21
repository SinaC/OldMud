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

// Added by SinaC 2001
#include "db.h"
#include "save.h"
#include "handler.h"
#include "olc_value.h"


NOTE_DATA *new_note() {
  return new NOTE_DATA;
}
    
BAN_DATA *new_ban(void) {
  BAN_DATA *ban = new BAN_DATA;

  ban->name = &str_empty[0];
  return ban;
}

DESCRIPTOR_DATA *new_descriptor(void) {
  DESCRIPTOR_DATA *d = new DESCRIPTOR_DATA;
  d->connected	= CON_ANSI;
  d->outsize		= 2000;
  d->editor		= 0;			/* OLC */
  d->outbuf		= (char*) GC_MALLOC_ATOMIC( d->outsize );
  memset( d->outbuf, 0, d->outsize );

  d->lineMode           = FALSE; // SinaC 2003
  d->stringColorMode    = TRUE;

  d->pEdit = NULL;
  d->pString = NULL;
  d->pStringFun = NULL;
  d->pStringArg = NULL;

  return d;
}
 
GEN_DATA *new_gen_data(void) {
  GEN_DATA *gen = new GEN_DATA;

  // Added by SinaC 2000
  if ( ( gen->ability_chosen = (bool *)GC_MALLOC_ATOMIC( MAX_ABILITY * sizeof(bool)) ) == NULL ){
    bug("Can't allocate memory for gen_data");
    exit(-1);
  }
  memset( gen->ability_chosen, 0, MAX_ABILITY * sizeof(bool) );
  if ( ( gen->group_chosen = (bool *)GC_MALLOC_ATOMIC( MAX_GROUP * sizeof(bool)) ) == NULL ){
    bug("Can't allocate memory for gen_data");
    exit(-1);
  }
  memset( gen->group_chosen, 0, MAX_GROUP * sizeof(bool) );
  return gen;
}


EXTRA_DESCR_DATA *new_extra_descr(void) {
  EXTRA_DESCR_DATA *ed = new EXTRA_DESCR_DATA;

  top_ed++;

  ed->keyword = &str_empty[0];
  ed->description = &str_empty[0];
  return ed;
}

AFFECT_DATA *new_affect(void) {
  top_affect++;

  //return new AFFECT_DATA;
  AFFECT_DATA *af = new AFFECT_DATA;
  af->list = NULL;
  af->flags = AFFECT_IS_VALID;
  return af;
}

RESTR_DATA *new_restriction(void) {
  return new RESTR_DATA;
}

ABILITY_UPGRADE *new_ability_upgrade(void) {
  return new ABILITY_UPGRADE;
}

OBJ_DATA *new_obj(void) {
  object_count++;
  OBJ_DATA *obj = new OBJ_DATA;

  obj->valid = TRUE;

  return obj;
}

void free_obj(OBJ_DATA *obj) {
  object_count--;
  obj->valid = FALSE;
}


CHAR_DATA *new_char (void) {
  CHAR_DATA *ch = new CHAR_DATA;
  // Added by SinaC 2000
  ch->version                 = PLAYER_VERSION;
  // Added by SinaC 2001
  ch->clazz                   = default_mob_class;

  ch->name                    = &str_empty[0];
  ch->short_descr             = &str_empty[0];
  ch->long_descr              = &str_empty[0];
  ch->description             = &str_empty[0];
  ch->prompt                  = &str_empty[0];
  ch->prefix		      = &str_empty[0];
  ch->logon                   = current_time;
  ch->lines                   = PAGELEN;
  for (int i = 0; i < 4; i++)
    ch->bstat(ac0+i)          = 100;
  ch->position                = POS_STANDING;
  ch->hit                     = 20;
  ch->bstat(max_hit)          = 20;
  ch->mana                    = 100;
  ch->bstat(max_mana)         = 100;
  // Added by SinaC 2001 for mental user
  ch->psp                     = 100;
  ch->bstat(max_psp)          = 100;

  ch->move                    = 100;
  ch->bstat(max_move)         = 100;
  for (int i = 0; i < MAX_STATS; i ++)
    ch->bstat(stat0+i) = 13;

  ch->isWildMagic = FALSE;
  ch->optimizing_bit = 0;

  ch->valid = TRUE;

  return ch;
}


void free_char (CHAR_DATA *ch) {
  OBJ_DATA *obj;
  OBJ_DATA *obj_next;

  
  if (IS_NPC(ch))
    mobile_count--;

  for (obj = ch->carrying; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    extract_obj(obj);
  }

  ch->valid = FALSE;

  return;
}

PC_DATA *new_pcdata(void) {
  PC_DATA *pcdata = new PC_DATA;

  // Added by SinaC 2001 for name acceptance
  pcdata->name_accepted = FALSE;
  pcdata->buffer = new_buf();

  // Added by SinaC 2000
  if ( ( pcdata->group_known = (bool *) GC_MALLOC_ATOMIC(MAX_GROUP * sizeof(bool)) ) == NULL ){
    bug("Can't allocate memory for pcdata (2)");
    exit(-1);
  }
  memset( pcdata->group_known, 0, MAX_GROUP * sizeof(bool) );
  // Modified by SinaC 2000
  if ( ( pcdata->ability_info = (ABILITY_INFO *) GC_MALLOC(MAX_ABILITY * sizeof(ABILITY_INFO)))==NULL){
    bug("Can't allocate memory for pcdata (1)");
    exit(-1);
  }

  // Added by SinaC 2003 for factions, start with a null vector
  for ( int i = 0; i < factions_count; i++ ) {
    pcdata->base_faction_friendliness[i] = 0;
    pcdata->current_faction_friendliness[i] = 0;
  }

  pcdata->tmpRace = -1; // no tmpRace
    
  return pcdata;
}


/* stuff for providing a crash-proof buffer */

#define MAX_BUF		16384
#define MAX_BUF_LIST 	11
#define BASE_BUF 	1024

/* valid states */
#define BUFFER_SAFE	0
#define BUFFER_OVERFLOW	1
#define BUFFER_FREED 	2

/* buffer sizes */
const int buf_size[MAX_BUF_LIST] =
{
  16,32,64,128,256,1024,2048,4096,8192,16384, 32768
};

/* local procedure for finding the next acceptable size */
/* -1 indicates out-of-boundary error */
int get_size (int val)
{
  int i;

  for (i = 0; i < MAX_BUF_LIST; i++)
    if (buf_size[i] >= val) {
      return buf_size[i];
    }
    
  return -1;
}

BUFFER *new_buf()
{
  BUFFER *buffer;

  buffer = new BUFFER;

  buffer->state	= BUFFER_SAFE;
  buffer->size	= get_size(BASE_BUF);

  buffer->string	= (char*) GC_MALLOC_ATOMIC(buffer->size);
  buffer->string[0]	= '\0';

  return buffer;
}


bool add_buf(BUFFER *buffer, const char *string)
{
  int len;
  char *oldstr;
  int oldsize;

  oldstr = buffer->string;
  oldsize = buffer->size;

  if (buffer->state == BUFFER_OVERFLOW) /* don't waste time on bad strings! */
    return FALSE;

  len = strlen(buffer->string) + strlen(string) + 1;

  while (len >= buffer->size) {/* increase the buffer size */
    buffer->size 	= get_size(buffer->size + 1);
    if (buffer->size == -1) {/* overflow */
      buffer->size = oldsize;
      buffer->state = BUFFER_OVERFLOW;
      bug("buffer overflow past size %d",buffer->size);
      return FALSE;
    }
  }

  if (buffer->size != oldsize) {
    buffer->string	= (char*) GC_MALLOC_ATOMIC(buffer->size);

    strcpy(buffer->string,oldstr);
  }

  strcat(buffer->string,string);
  return TRUE;
}


void clear_buf(BUFFER *buffer)
{
  buffer->string[0] = '\0';
  buffer->state     = BUFFER_SAFE;
}


char *buf_string(BUFFER *buffer)
{
  return buffer->string;
}


// Remains of recycle.C
/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
  int val;

  val = (current_time <= last_pc_id) ? last_pc_id + 1 : current_time;
  last_pc_id = val;
  return val;
}

long get_mob_id(void)
{
  last_mob_id++;
  return last_mob_id;
}
