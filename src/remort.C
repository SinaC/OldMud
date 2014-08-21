#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
#include "db.h"
#include "comm.h"
#include "handler.h"
#include "remort.h"
#include "olc_value.h"
#include "interp.h"
#include "config.h"


void remort_syntax( CHAR_DATA *ch ) {
  send_to_charf(ch,
		"Syntax: remort <remort race>\n\r"
		"        remort list           display available remort race\n\r");
  return;
}
void do_remort( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) ) {
    send_to_charf(ch,"Mob can't use this command.\n\r");
    return;
  }

  if (IS_IMMORTAL(ch)) {
    send_to_char("Immortals are damned to be immortals.\n\r",ch);
    return;
  }

  char arg[MAX_STRING_LENGTH];
  argument = one_argument( argument, arg );
  if ( arg[0] == '\0' ) {
    remort_syntax( ch );
    return;
  }
  int raceId = ch->cstat(race); // SinaC 2003
  if ( !race_table[raceId].pc_race ) {   // if current race is not PC race
    raceId = ch->bstat(race);            //   get base race
    if ( !race_table[raceId].pc_race ) { //   if base race is not PC race
      raceId = DEFAULT_PC_RACE;             //     get default race: human
      bug("do_remort: invalid race for [%s] sn:[%d]",
	  NAME(ch), IS_NPC(ch)?ch->pIndexData->vnum:-1);
    }
  }
  //pc_race_type *race = &(pc_race_table[ch->bstat(race)]);
  pc_race_type *race = &(pc_race_table[raceId]);
  if ( !str_cmp( arg, "list" ) ) {
    if ( race->nb_remorts == 0 ) {
      send_to_charf(ch,"No remort available.\n\r");
      return;
    }
    send_to_charf(ch,"Available remort:\n\r");
    for ( int i = 0; i < race->nb_remorts; i++ )
      send_to_charf(ch," %s\n\r", pc_race_table[race->remorts[i]].name );
    return;
  }
  int iRace = race_lookup(arg, TRUE);
  if ( iRace < 0 ) {
    send_to_charf(ch,"%s is not a race.\n\r", arg);
    return;
  }
  if ( race->nb_remorts == 0 ) {
    send_to_charf(ch,"%s doesn't have any remort races.\n\r", race->name );
    return;
  }
  int iRemort = -1;
  for ( int i = 0; i < race->nb_remorts; i++ )
    if ( race->remorts[i] == iRace ) {
      iRemort = iRace;
      break;
    }

  if ( iRemort == -1 ) {
    send_to_charf(ch,"Invalid remort race.\n\r");
    return;
  }

  if ( ch->pcdata->remort_count < pc_race_table[iRemort].min_remort_number ) {
    send_to_charf(ch,"You are not yet ready to remort in this race.\n\r");
    return;
  }

  send_to_charf(ch,"Starting remort in %s\n\r", pc_race_table[iRemort].name);
  ch->desc->connected = CON_REMORT;
  ch->bstat(race) = iRemort;
  ch->pcdata->remort_count++; // one more remort
  return;
}
