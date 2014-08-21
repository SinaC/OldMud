/*
Newbie hints channel -- By Oxtal
I load the whole file in memory.
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
#include <ctype.h>
#include "merc.h"

// Added by SinaC 2001
#include "db.h"
#include "comm.h"
#include "olc_value.h"
#include "interp.h"
#include "act_info.h"
#include "config.h"
#include "utils.h"


#define HINT_NBR   256;
#define HINT_STEP  32;   /* Must be > 0 */



const char ** hints = NULL;
int hintcount = 0;
int hintsize = 0;

const char * readhint(FILE * f)
{
  char buf[MAX_INPUT_LENGTH];
  int pos = 0;
  char c = getc(f);
  while (isspace(c))
    c = getc(f);

  while (c != EOF && c != '\n') {
    buf[pos++] = c;
    c = getc(f);
  }
  
  if (pos == 0)
    return NULL;
  buf[pos] = '\0';
  return str_dup(buf);
}

void load_hints()
{
  log_string("Reading hints");

  if (hints) {
   bug("Reloading of hints not (yet) supported!"); // Uh-oh
   exit(1);
  }
  
  hintcount = 0;
  hintsize = HINT_NBR;  
  hints = (const char **) GC_MALLOC(hintsize * sizeof(const char *));

  FILE * f = fopen(HINT_FILE,"r");
  
  if (!f) {
    bug("Could not open hint file!");
    return; // Childlish behaviour -- Tired am i of this err handling stuff
  }
  
  const char * s;
  
  while ((s=readhint(f))) {
    if (hintcount >= hintsize) {
      int old_size = hintsize;
      hintsize += HINT_STEP;
      //hints = (const char **) GC_REALLOC(hints, hintsize * sizeof(const char *));
      const char **tmp = (const char **)GC_MALLOC( hintsize * sizeof( const char *));
      bcopy(hints,tmp,old_size*sizeof(const char *));
      hints = tmp;
    }
    hints[hintcount++] = s;
  }

  log_stringf(" %d hints found.",hintcount);
  
  fclose(f);
}

void hints_update()
{
  /* Pick an hint */
  const char * s = hints[number_range(0,hintcount-1)];
  
  DESCRIPTOR_DATA * d;
  for (d=descriptor_list; d; d= d->next)
    if (
    d->character 
    && d->connected == CON_PLAYING 
    && !IS_SET(CH(d)->comm,COMM_NOHINTS) )
      send_to_charf(d->character,"Hint : %s\n\r",s);
   
}

void do_hints(CHAR_DATA * ch, const char * argument)
{
  char arg[MAX_INPUT_LENGTH];
  argument = one_argument(argument,arg);

  if ( IS_NPC(ch))
    {
      send_to_char("Mobiles can't show tips!\n\r",ch);
      return;
    }

  if (!*arg) {
    ch->comm ^= COMM_NOHINTS;
    send_to_charf(ch,"Hints turned %s.\n\r", 
     IS_SET(ch->comm,COMM_NOHINTS) ? "OFF" : "ON");
    return;
  }

  if (is_number(arg)) {
    int h = atoi(arg);
    if (h < 0 || h >= hintcount) {
      send_to_charf(ch,"There no hint number %d.\n\r",h);
      return;
    } 
    send_to_charf(ch,"Hint %d : %s\n\r",h,hints[h]);
    return;
  }
  
  do_help(ch,"hints");
}

