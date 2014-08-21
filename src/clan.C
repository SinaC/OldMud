
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
#include "clan.h"
#include "classes.h"

// Added by SinaC 2001
#include "clan.h"
#include "db.h"
#include "handler.h"
#include "comm.h"
#include "olc_value.h"
#include "lookup.h"
#include "interp.h"
#include "dbdata.h"
//#include "olc.h"
#include "config.h"
#include "bit.h"
#include "utils.h"


CLAN_TYPE *clan_table = NULL;

void write_clan_table(void);

int clan_count;

const char *lookup_clan_status(int cs) {
  switch (cs) {
  case CLAN_LEADER: return "the leader";
  case CLAN_SECOND: return "the second in command";
  case CLAN_DEPUTY: return "a deputy";
  case CLAN_SENIOR: return "a senior member";
  case CLAN_JUNIOR: return "a junior member";
  }
  return "a member";
}
int min_level_for_status(int cs) {
  switch (cs) {
  case CLAN_LEADER: return 25;
  case CLAN_SECOND: return 25;
  case CLAN_DEPUTY: return 20;
  case CLAN_SENIOR: return 15;
  case CLAN_JUNIOR: return 10;
  }
  return 10;
}


const char *short_lookup_clan_status(int cs) {
  switch (cs) {
  case CLAN_LEADER: return "leader";
  case CLAN_SECOND: return "second";
  case CLAN_DEPUTY: return "deputy";
  case CLAN_SENIOR: return "senior";
  case CLAN_JUNIOR: return "junior";
  }
  return "a member";
}

int parse_clan_status(const char *s) {
  if (s[0]) {
    if (!str_prefix(s, "junior")) return CLAN_JUNIOR;
    if (!str_prefix(s, "senior")) return CLAN_SENIOR;
    if (!str_prefix(s, "deputy")) return CLAN_DEPUTY;
    if (!str_prefix(s, "second")) return CLAN_SECOND;
    if (!str_prefix(s, "leader")) return CLAN_LEADER;
  }
  return -1;
}


CHAR_DATA * find_player(const char * name) {
  DESCRIPTOR_DATA * d;

  if (name==NULL)
    return NULL;

  for (d = descriptor_list; d != NULL; d = d->next)
    if (d->connected == CON_PLAYING && d->character
	&&  d->character->in_room && !strcmp(name,d->character->name) )
      break;

  if (d==NULL) return NULL;
  return d->character;
}

void update_clan_member(CLAN_MEMBER_DATA *member) {

  CHAR_DATA * ch;

  if ((ch = find_player(member->name))==NULL) return;

  member->level = ch->level;
  member->race = ch->cstat(race);
  member->status = ch->clan_status;
  member->classes = ch->cstat(classes);
}

CLAN_MEMBER_DATA * add_clan_member(CLAN_MEMBER_DATA** list, const char * name) {
  CLAN_MEMBER_DATA * m;

  if ((m = find_clan_member(*list,name)) != NULL) {
    bug("Try to duplicate clan member.");
    return NULL;
  }
  
  m = (CLAN_MEMBER_DATA*) GC_MALLOC(sizeof(CLAN_MEMBER_DATA));
  m->name = str_dup(name);
  m->next = *list;
  *list = m;

  //return *list = m;
  return m;
}

void remove_clan_member(CLAN_MEMBER_DATA** list, const char * name) {
  CLAN_MEMBER_DATA * prec,*m;

                       
  prec=NULL;

  for (m=*list; m!=NULL; prec=m, m=m->next)
    if (!strcmp(m->name,name))
      break;

  if (m == NULL)
    return;
  

  if (prec == NULL) 
    *list = m->next;
  else
    prec->next = m->next;

}

CLAN_MEMBER_DATA * find_clan_member(CLAN_MEMBER_DATA* list, const char * name) {
  CLAN_MEMBER_DATA * m;

  for (m = list; m!=NULL;m=m->next)
    if (!str_cmp(m->name,name))
      return m;
  return NULL;
}

CLAN_MEMBER_DATA * is_clan_member(CLAN_MEMBER_DATA* list, const char * name) {
  CLAN_MEMBER_DATA * m;
  for (m = list; m!=NULL;m=m->next)
    if (is_name(name,m->name))
      return m;
  return NULL;
}

void fix_clan_status_enter(const char * name) {

  CHAR_DATA * ch;

  if ((ch=find_player(name)) == NULL)
    return;

  /* Fix clan petition accept/reject */
  if (ch->petition && find_clan_member(get_clan_table(ch->petition)->petitions,ch->name) == NULL) {
    if (find_clan_member(get_clan_table(ch->petition)->members,ch->name) != NULL) {
      send_to_char("\n\r{gYour clan application was successful!{x\n\r",ch);
      ch->clan = ch->petition;
    }
    else
      send_to_char("\n\r{rYour clan application has been rejected.{x\n\r\n\r",ch);
    ch->petition = 0;
  }

  /* Fix clan status itself */
  if (is_clan(ch)) {
    CLAN_MEMBER_DATA * m = find_clan_member(get_clan_table(ch->clan)->members,ch->name);

    if (m == NULL)
      update_clan_member(add_clan_member(&get_clan_table(ch->clan)->members,ch->name));
    else
      if (ch->clan_status != m->status) {     
        send_to_charf(ch,"{xYou are now {b%s{x of the clan %s.\n\r\n\r",
		      lookup_clan_status(m->status),
		      get_clan_table(ch->clan)->name);
        ch->clan_status = m->status;
      }
  }
}

void fix_clan_status_leave(CHAR_DATA * ch) {
  if (is_clan(ch))
    update_clan_member(find_clan_member(get_clan_table(ch->clan)->members,ch->name));
}

void do_myclan(CHAR_DATA *ch, const char * argument) {
  CLAN_MEMBER_DATA * m;

  if (!is_clan(ch)) {
    send_to_char("You aren't in a clan.\n\r",ch);
    return;
  }

  send_to_charf(ch,"You are %s of the clan '%s'.\n\r",
		lookup_clan_status(ch->clan_status),
		get_clan_table(ch->clan)->name);

  send_to_char("List of members of your clan\n\r",ch);

  for (m = get_clan_table(ch->clan)->members;m != NULL; m = m->next) {
    update_clan_member(m);
    send_to_charf(ch,"%15s, %-15s %7s %-7s %d\n\r",
		  m->name,
		  lookup_clan_status(m->status),
		  race_table[m->race].name,
		  class_name(m->classes),
		  m->level);
  }
}

/*
 *  petition [clan]
 *  petition accept [player]
 *  petition reject [player]
 *  petition remove [player]
 *  petition raise <player> <status>
 */

void petition_list(int clan, CHAR_DATA *ch) {
  CLAN_MEMBER_DATA * m = get_clan_table(ch->clan)->petitions;

  if (m != NULL) {
    send_to_char("The following characters have petitioned your clan.\n\r",ch);

    for (;m != NULL; m = m->next) {
      update_clan_member(m);
      send_to_charf(ch,"%15s  %7s %-7s %d\n\r",
		    m->name,
		    race_table[m->race].name,
		    class_name(m->classes),
		    m->level);
    }
  } 
  else
    send_to_char("No one has petitioned your clan.\n\r",ch);
}

void do_petition(CHAR_DATA *ch, const char *argument) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  CLAN_MEMBER_DATA * m;
  short status;

  argument = one_argument(argument, arg1);
  argument = one_argument(argument, arg2);
  argument = one_argument(argument, arg3);

  status = ch->clan ? ch->clan_status : 0;

  if ( IS_NPC(ch) ) {
    send_to_char("NPC can't petition for a clan!\n\r", ch );
    return;
  }

  if (arg1[0] == 0) {
    if (ch->petition) {
      remove_clan_member(&get_clan_table(ch->petition)->petitions,ch->name);
      ch->petition = 0;
      send_to_char("You withdraw your petition.\n\r", ch);
      return;
    }
    if (status >= CLAN_DEPUTY) {
      petition_list(ch->clan, ch);
      return;
    }
  }
  else if (arg2[0] == 0) {

    int clan;
    if (status >= CLAN_DEPUTY) {
      send_to_charf(ch, "%s of a clan may not petition another clan.\n\r",
		    capitalize(lookup_clan_status(ch->clan_status)));
      return;
    }

    clan = clan_lookup(arg1);
    if (!clan || get_clan_table(clan)->independent) {
      send_to_char("There is no clan by that name.\n\r", ch);
      return;
    }
    if (clan==ch->clan) {
      send_to_charf(ch,"You're already member of that clan!\n\r"); return;
    }
    if (ch->petition)
      remove_clan_member(&get_clan_table(ch->petition)->petitions,ch->name);
    ch->petition = clan;
    update_clan_member(add_clan_member(&get_clan_table(clan)->petitions,ch->name));
    send_to_charf(ch, "You have petitioned the clan %s for membership.\n\r",
		  capitalize(get_clan_table(clan)->name));
    return;

  }
  else if (status >= CLAN_DEPUTY) {
    if (arg3[0] == 0) {
      if (!str_prefix(arg1, "accept")) {
	CLAN_MEMBER_DATA * nm;
	m = is_clan_member(get_clan_table(ch->clan)->petitions,arg2);
	if (m == NULL) {
	  send_to_charf(ch,"They have not petitioned your clan.\n\r");
	  return;
	}

	nm = add_clan_member(&get_clan_table(ch->clan)->members,m->name);
	nm->status = CLAN_JUNIOR;
	nm->level = m->level;
	nm->classes = m->classes;
	nm->race = m->race;
	remove_clan_member(&get_clan_table(ch->clan)->petitions,m->name);
	fix_clan_status_enter(nm->name);

	send_to_char("You have accepted them in your clan.\n\r", ch);
	return;
      }
      else if (!str_prefix(arg1, "reject")) {

	m = is_clan_member(get_clan_table(ch->clan)->petitions,arg2);
	if (m == NULL) {
	  send_to_char("They have not petitioned your clan.\n\r", ch);
	  return;
	}
	remove_clan_member(&get_clan_table(ch->clan)->petitions,m->name);
	fix_clan_status_enter(m->name);

	send_to_char("You have rejected their application.\n\r", ch);
	return;
      }
      else if (!str_prefix(arg1, "remove") && status >= CLAN_SECOND) {
	if ((victim = get_char_world(ch, arg2)) == NULL) {
	  send_to_char("They are not playing.\n\r", ch);
	  return;
	}
	if (victim->clan != ch->clan) {
	  send_to_char("They are not a member of your clan.\n\r", ch);
	  return;
	}
	if (victim->clan_status >= ch->clan_status ||
	    (victim->clan_status >= CLAN_SENIOR && ch->clan_status <= CLAN_DEPUTY) ) {
	  send_to_char("You failed.\n\r", ch);
	  return;
	}
	remove_clan_member(&get_clan_table(ch->clan)->members,victim->name);
	victim->clan = 0;
	victim->clan_status = 0;
	send_to_char("You have removed them from the clan.\n\r", ch);
	send_to_char("You have been removed from the clan.\n\r", victim);
	return;
      }
    } 
    else if (status >= CLAN_SECOND) {
      if (!str_prefix(arg1, "raise")) {
	int nstatus;

	m = is_clan_member(get_clan_table(ch->clan)->members,arg2);
	if (m == NULL) {
	  send_to_char("They are not a member of your clan.\n\r", ch);
	  return;
	}
	nstatus = parse_clan_status(arg3);
	if (nstatus < 0) {
	  send_to_char("There is no such status in a clan.\n\r", ch);
	  return;
	}
	if (nstatus >= status || m->status >= status
	    || (m->status >= CLAN_SENIOR && ch->clan_status <= CLAN_DEPUTY) ) {
	  send_to_char("You failed.\n\r", ch);
	  return;
	} 
	m->status = nstatus;
	send_to_charf(ch, "They are now %s of the clan.\n\r",
		      lookup_clan_status(nstatus));
	fix_clan_status_enter(m->name);
	return;
      }
    }     
  }

  if (ch->clan && ch->clan_status >= CLAN_SECOND)
    send_to_char(   "usage: petition accept [player]\n\r"
		    "       petition reject [player]\n\r"
		    "       petition remove [player]\n\r"
		    "       petition raise <player> <status>\n\r", ch);
  else if (ch->clan && ch->clan_status >= CLAN_DEPUTY)
    send_to_char(   "usage: petition accept [player]\n\r"
		    "       petition reject [player]\n\r"
		    "       petition remove [player]\n\r", ch);
  else
    send_to_char("usage: petition <clan>\n\r", ch);
}

// Added by SinaC 2000
void show_clan_member( CHAR_DATA *ch, CLAN_TYPE *c ) {
  CLAN_MEMBER_DATA *m;
  bool found;

  found = FALSE;
  for (m=c->members; m != NULL; m = m->next) {
    found = TRUE;
    send_to_charf(ch,"  %10s : %s \n\r",
		  m->name,
		  lookup_clan_status(m->status));
  }
  if ( found == FALSE )
    send_to_char("No members.\n\r",ch);
}

void do_clans(CHAR_DATA *ch, const char *argument) {

  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char arg4[MAX_INPUT_LENGTH];
  
  CLAN_TYPE *c;
  //    CLAN_MEMBER_DATA *m;

  if (!ch) return;

  argument = one_argument(argument, arg1);

  if (arg1[0]) {

    if (!str_prefix(arg1, "list")) {
      send_to_char(" id name                 whoname    hall  recall skill donation morgue\n\r",ch);
      send_to_char(" -- -------------------- ---------- ----- ------ ----- -------- ------\n\r",ch);
      for (c=clan_table; c!=NULL; c=c->next) {
	// Modified by SinaC 2000 for clan skill
        send_to_charf(ch,
		      "%2d %-20s %-10s %5d %5d %-20s %5d %5d\n\r",
		      c->clan_id, c->name, c->who_name, c->hall, c->recall,
		      c->clan_ability==-1?"none":ability_table[c->clan_ability].name,
		      c->donation, c->morgue );
	// Modified by SinaC 2000, to see the members of a clan:  clans member <id|'name'>
      }
      return;
    }

    if (!str_prefix(arg1, "load")) {
      //read_clan_table(ch);
      new_load_clans();
      send_to_charf(ch,"There are %d clans available.\n\r",clan_count);
      return;
    }

    if (!str_prefix(arg1, "save")) {
      //write_clan_table();
      new_save_clans();
      send_to_char("Clans saved.\n\r",ch);
      return;
    }

    if (!str_prefix(arg1, "member")) {
      CLAN_TYPE *c;

      argument = one_argument(argument, arg2);
	
      if (arg2[0]) {
	if (is_number(arg2))
	  c = get_clan_table(atoi(arg2));
	else
	  c = get_clan_table(clan_lookup(arg2));
	    
	if (!c->clan_id) {
	  send_to_char("Invalid clan id or name\n\r",ch);
	  return;
	}
	send_to_charf(ch, "Members of the clan %s:\n\r",
		      c->name);
	show_clan_member( ch, c );
      }
      return;
    }
    
    if (!str_prefix(arg1, "new")) {
      CLAN_TYPE *c;
      int id;

      argument = one_argument(argument, arg2);

      if (is_number(arg2) && argument[0]) {
        id = atoi(arg2);
        c=get_clan_table(id);
        if (c->clan_id == 0) {
          c=(CLAN_TYPE*)GC_MALLOC(sizeof(struct clan_type));
          if (c!=NULL) {
            c->next = clan_table;
            c->clan_id = id;
            c->name = str_dup(argument);
            c->who_name = str_dup(argument);
	    //c->hall = ROOM_VNUM_ALTAR;
	    c->hall = DEFAULT_HALL;
	    //c->recall = ROOM_VNUM_TEMPLE;
	    c->recall = DEFAULT_RECALL;
	    c->donation = DEFAULT_DONATION;
	    c->morgue = DEFAULT_MORGUE;
            c->independent = FALSE;
	    c->clan_ability = -1;
            clan_table = c;
            send_to_charf(ch,"New clan : %s, with id %d\n\r",c->name,id);
            clan_count++;
          }
          else
            send_to_char("Alloc failed",ch);
        }
        else
          send_to_charf(ch,"This id is already used by '%s'.\n\r",c->name);
        return;
      }

    } /* End new clan*/

    if (!str_prefix(arg1, "edit")) {
      CLAN_TYPE *c;
      int ok = FALSE;

      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);
      one_argument( argument, arg4 );

      if (arg2[0]) {
        if (is_number(arg2))
          c = get_clan_table(atoi(arg2));
        else
          c = get_clan_table(clan_lookup(arg2));

        if (!c->clan_id) {
          send_to_char("Invalid clan id or name\n\r",ch);
          return;
        }

        if ( arg3[0] && arg4[0]) {
	  // Added by SinaC 2000
	  if (!str_prefix(arg3, "skill")||!str_prefix(arg3,"ability")){
	    int sn;
	    // delete the clan skill
	    if ( !str_cmp( arg4, "none" ) ) {
	      //c->clan_ability = str_dup( "" );
	      c->clan_ability = -1;
	      ok = TRUE;
	    } 
	    else if ( ( sn = ability_lookup( arg4 ) ) <= 0 ) {
	      send_to_charf(ch,"Skill: %s doesn't exist!\n\r",
			    arg4);
	      ok = FALSE;
	    }
	    else {
	      //c->clan_ability = str_dup(arg4);
	      c->clan_ability = sn;
	      ok = TRUE;
	      
	      send_to_char("Clan ability set.\n\r",ch);
	    }
	  }
	  // end
	  else if (!str_prefix(arg3, "name")) {
            c->name = str_dup(arg4);
            ok = TRUE;

	    send_to_char("Clan name set.\n\r",ch);
          } 
	  else if (!str_prefix(arg3, "whoname")) {
            c->who_name = str_dup(arg4);
            ok = TRUE;

	    send_to_char("Clan whoname set.\n\r",ch);
          } 
	  else if (!str_prefix(arg3, "hall")) {
            if (is_number(arg4)) {
              c->hall = atoi(arg4);
              ok = TRUE;

	      send_to_char("Clan hall set.\n\r",ch);
            }
          } 
	  else if (!str_prefix(arg3, "recall")) {
            if (is_number(arg4)) {
              c->recall = atoi(arg4);
              ok = TRUE;

	      send_to_char("Clan recall set.\n\r",ch);
            }
	  }
	  else if (!str_prefix(arg3, "donation")) {
            if (is_number(arg4)) {
              c->donation = atoi(arg4);
              ok = TRUE;
	      
	      send_to_char("Clan donation set.\n\r",ch);
            }
          }
	  else if (!str_prefix(arg3, "morgue")) {
            if (is_number(arg4)) {
              c->morgue = atoi(arg4);
              ok = TRUE;
	      
	      send_to_char("Clan morgue set.\n\r",ch);
            }
          }
	  /* End swith what to modif */
	  if (ok) {
	    // Added by SinaC 2000 for mudstatus
	    clannotsaved = TRUE;
	    
            send_to_charf(ch,
			  "id      : %d\n\r"
			  "name    : %s\n\r"
			  "whoname : %s\n\r"
			  "hall    : %d\n\r"
			  "recall  : %d\n\r"
			  // Added by SinaC 2000 for clan skill
			  //"skill   : %s\n\r",
			  "ability : %s\n\r"
			  "donation: %d\n\r"
			  "morgue  : %d\n\r",
			  c->clan_id,c->name,c->who_name,c->hall,c->recall,
			  // Added by SinaC 2000 for clan skill
			  c->clan_ability==-1?"none":ability_table[c->clan_ability].name,
			  c->donation, c->morgue );
            return;
          }
        }
	
      }
    } /* End edit clan*/
    
  }
  send_to_char("usage: \n\r"
	       "  clans list\n\r"
	       "        load\n\r"
	       "        save\n\r"
	       "        member <id | 'name'>\n\r"
	       "        new  <id> <name>\n\r"
	       "        edit <id | 'name'> <what to change> value\n\r"
	       // Modified by SinaC 2000 for clan skill
	       " what to change may be : name, whoname, hall, recall, skill, donation or morgue\n\r", ch);
}

void do_guild( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH],arg2[MAX_INPUT_LENGTH];
  char arg3[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  CHAR_DATA *victim;
  int clan;
  int level = CLAN_JUNIOR;
  CLAN_MEMBER_DATA *m;          // added by SinaC 2000 cos' of a bug

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  argument = one_argument( argument, arg3 );

  if (arg3[0]) {
    if (arg3[0] != 0 && !str_prefix(arg3,"junior"))
      level = CLAN_JUNIOR;
    else if (arg3[0] != 0 && !str_prefix(arg3,"senior"))
      level = CLAN_SENIOR;
    else if (arg3[0] != 0 && !str_prefix(arg3,"deputy"))
      level = CLAN_DEPUTY;
    else if (arg3[0] != 0 && !str_prefix(arg3,"second"))
      level = CLAN_SECOND;
    else if (arg3[0] != 0 && !str_prefix(arg3,"leader"))
      level = CLAN_LEADER;
    else
      level = -1;
  }

  if ( arg1[0] == '\0' || arg2[0] == '\0' || level == -1) {
    send_to_char( "Syntax: guild <char> <cln name> [rank]\n\r",ch);
    return;
  }
  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL ) {
    send_to_char( "They aren't playing.\n\r", ch );
    return;
  }

  /*
    if (victim->level < 10) {
    send_to_char("Their level is too low to join a clan.\n\r", ch);
    return;
    }
  */
  if (!str_prefix(arg2,"none")) {
    send_to_char("They are now clanless.\n\r",ch);
    send_to_char("You are no longer a member of a clan!\n\r",victim);

    remove_clan_member(&get_clan_table(victim->clan)->members,victim->name);
    victim->clan = 0;
    victim->clan_status = 0;
    return;
  }

  if ((clan = clan_lookup(arg2)) == 0) {
    send_to_char("No such clan exists.\n\r",ch);
    return;
  }

  if (get_clan_table(clan)->independent) {
    sprintf(buf,"They are now a %s.\n\r", get_clan_table(clan)->name);
    send_to_char(buf,ch);
    sprintf(buf,"You are now a %s.\n\r", get_clan_table(clan)->name);
    send_to_char(buf,victim);
  }
  else {
    sprintf(buf,"They are now %s of clan %s.\n\r",
	    lookup_clan_status(level),
	    capitalize(get_clan_table(clan)->name));
    send_to_char(buf,ch);
    sprintf(buf,"You are now %s of clan %s.\n\r",
	    lookup_clan_status(level),
	    capitalize(get_clan_table(clan)->name));
    send_to_char(buf,victim);
  }

  remove_clan_member(&get_clan_table(victim->clan)->members,victim->name);
  victim->clan = clan;
  victim->clan_status = level;

  if (!get_clan_table(clan)->independent) {
	
    m = add_clan_member(&get_clan_table(victim->clan)->members,victim->name) ;
	
    if ( m == NULL ) {
      send_to_char( "Hmm ... a tiny problem with this guild command!\n\r", ch );
      bug("Problem with guild command!");
      return;
    }
	
    update_clan_member(m);
  }
}


struct clan_type *get_clan_table(int id) {
  static struct clan_type null_clan = {
    //0, "", "", ROOM_VNUM_ALTAR, ROOM_VNUM_TEMPLE, TRUE ,NULL,NULL,NULL};
    0, "", "", DEFAULT_HALL, DEFAULT_RECALL, DEFAULT_DONATION, DEFAULT_MORGUE, TRUE, NULL, NULL, NULL };

  //  log_stringf("null_clan: hall: %d  recall: %d  donation: %d  morgue: %d",
  //	     DEFAULT_HALL, DEFAULT_RECALL, DEFAULT_DONATION, DEFAULT_MORGUE );

  CLAN_TYPE * c;
  for (c=clan_table; c!=NULL; c=c->next)
    if (c->clan_id == id)
      return c;
  return &null_clan;
}

int clan_lookup(const char *name) {
  CLAN_TYPE * c;
  for (c=clan_table; c!=NULL; c=c->next)
    if (UPPER(name[0]) == UPPER(c->name[0])
	&& !str_prefix(name, c->name))
      return c->clan_id;
  return 0;
}

// Added by SinaC 2000
void do_clanlist( CHAR_DATA *ch, const char *argument ){
  CLAN_TYPE *c;

  send_to_char("Clan list:\n\r",ch);
  send_to_charf(ch,
		"%-20s  %s\n\r",
		"Clan name",
		"Leader(s)");
  send_to_charf(ch,
		"---------             ---------\n\r");
  for (c=clan_table; c!=NULL; c=c->next) {
    CLAN_MEMBER_DATA *m;
    bool found;

    if (c->clan_id == 0 ) continue;

    send_to_charf(ch, "%-20s", c->name );
    found = FALSE;
    for (m=c->members; m != NULL; m = m->next){
      if ( m->status == CLAN_LEADER ){
	found = TRUE;
	send_to_charf(ch,"  %-10s", m->name);
      }
    }
    if (!found)
      send_to_char("  [No Leader]",ch);
    send_to_char("\n\r",ch);
  }
}


//********************************** new clan table load & save
//#define VERBOSE
//#define VERBOSE2

//Clan <STRING Clan Name> {
//  Id = <INTEGER Clan Id>;
//  WhoName = <STRING Who/Whois name>;
//  Hall = <INTEGER Clan hall vnum>;
//  Recall = <INTEGER Clan recall vnum>;
//  Donation = <INTEGER Clan donation vnum>;
//  Morgue = <INTEGER Clan morgue vnum>;
//  Independent = <BOOLEAN Is clan independent (True/False)>;
//  Ability = <STRING Ability Name>;
//  Member/Petition <STRING Member Name> {
//    Races = <STRING Race name>;
//    Classes = <INTEGER Classes>; // should maybe be a list of string
//    Level = <INTEGER Level>;
//    Status = <INTEGER Clan status>; // should maybe be a string
//  }
//}

void new_save_members( FILE *fp, CLAN_MEMBER_DATA *m, const char *header ) {
  for (;m!=NULL;m=m->next) {
    update_clan_member(m);
    fprintf( fp,
	     "  %s %s {\n"
	     "    Races = %s;\n"
	     "    Classes = (%s);\n"
	     "    Level = %d;\n"
	     "    Status = %s;\n"
	     "  }\n",
	     header,
	     quotify(m->name), quotify(race_table[m->race].name),
	     list_flag_string(m->classes, classes_flags, "'", ", " ), 
	     m->level, quotify(short_lookup_clan_status(m->status)) );
  }
}
void new_save_clans() {
  FILE *fp;
  log_string("Saving clan table");
  
  fclose( fpReserve );
  if ( ( fp = fopen( CLAN_FILENAME, "w" ) ) == NULL ) {
    bug( "Can't access clan file: [%s]!", CLAN_FILENAME );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  for (CLAN_TYPE *c=clan_table; c!=NULL; c=c->next) {
    fprintf( fp,
	     "Clan %s {\n"
	     "  Id = %d;\n"
	     "  WhoName = %s;\n"
	     "  Hall = %d;\n"
	     "  Recall = %d;\n"
	     "  Donation = %d;\n"
	     "  Morgue = %d;\n"
	     "  Independent = %s;\n",
	     quotify( c->name ), c->clan_id, quotify( c->who_name ),
	     c->hall, c->recall,
	     c->donation, c->morgue,
	     c->independent?"true":"false" );
    if ( c->clan_ability > -1 )
      fprintf( fp,
	       "  Ability = %s;\n",
	       quotify(ability_table[c->clan_ability].name) );
    new_save_members( fp, c->members, "Member" );
    new_save_members( fp, c->petitions, "Petition" );
    fprintf( fp,
	     "}\n\n");
  }
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}
void new_load_clans() {
  FILE *fp;

  log_string("Reading clan table");

  fclose( fpReserve );
  if ( ( fp = fopen( CLAN_FILENAME, "r" ) ) == NULL ) {
    bug( "Can't access clan file: [%s]!", CLAN_FILENAME );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  clan_table = NULL;
  int count = parse_datas( fp );
  log_stringf(" %d clans found.", count );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}
void parse_clan_member( DATAData *member, CLAN_MEMBER_DATA ** list ) {
  CLAN_MEMBER_DATA * m;
  
  if ( ( m = (CLAN_MEMBER_DATA*) GC_MALLOC(sizeof(CLAN_MEMBER_DATA)) ) == NULL )
    p_error("Can't allocate memory for clan member.");
  m->status = CLAN_JUNIOR; // dummy
  
  m->name = str_dup(member->value->eval().asStr());
if ( DATA_VERBOSE > 1 ) {
  printf("  Member: %s\n\r", m->name );
}
  
  // fields
  for ( int fieldCount = 0; fieldCount < member->fields_count; fieldCount++ ) {
    DATAData *field = member->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Races: {
      const char *s = field->value->eval().asStr();
      m->race = race_lookup(s,TRUE);
      if ( m->race == -1 )
	bug("Invalid member race [%s] for member [%s]", s, m->name );
if ( DATA_VERBOSE > 2 ) {
      printf("    Race: %s\n\r", race_table[m->race].name );
}
      break;
    }
    case TAG_Classes: 
      m->classes = list_flag_value( field->value, classes_flags );
      if ( m->classes <= 0 )
	bug("Invalid member class for member [%s]", m->name );
if ( DATA_VERBOSE > 2 ) {
      printf("    Classes: %s\n\r", flag_string( classes_flags, m->classes ) );
}
      break;
    case TAG_Level:
      m->level = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("    Level: %d\n\r", m->level );
}
      break;
    case TAG_Status: {
      const char *s = field->value->eval().asStr();
      m->status = parse_clan_status(s);
      if ( m->status == -1 ) {
	bug("Invalid member status [%s] for member [%s], assuming JUNIOR",
	    s, m->name );
	m->status = CLAN_JUNIOR;
      }
      break;
    }
    // Other
    default: p_error("Invalid Tag: %s", tagName ); break;
    }
  }

  m->next=*list;
  *list=m;
}
void parse_clan( DATAData *clan ) {
  CLAN_TYPE *cl;
  if ( ( cl = (CLAN_TYPE*) GC_MALLOC(sizeof(CLAN_TYPE)) ) == NULL )
    p_error("Can't allocate memory for clan table");
  //cl->hall = ROOM_VNUM_ALTAR; // dummy information
  cl->hall = DEFAULT_HALL;
  //cl->recall = ROOM_VNUM_TEMPLE;
  cl->recall = DEFAULT_RECALL;
  cl->donation = DEFAULT_DONATION;
  cl->morgue = DEFAULT_MORGUE;
  cl->members = cl->petitions = NULL;

  cl->name = str_dup(clan->value->eval().asStr());
if ( DATA_VERBOSE > 0 ) {
  printf("Clan: %s\n\r", cl->name );
}

  // fields
  for ( int fieldCount = 0; fieldCount < clan->fields_count; fieldCount++ ) {
    DATAData *field = clan->fields[fieldCount];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Id: 
      cl->clan_id = field->value->eval().asInt(); 
if ( DATA_VERBOSE > 2 ) {
      printf("  id: %d\n\r", cl->clan_id );
}
      break;
    case TAG_WhoName:
      cl->who_name = str_dup(field->value->eval().asStr());
if ( DATA_VERBOSE > 2 ) {
      printf("  who name: %s\n\r", cl->who_name);
}
      break;
    case TAG_Hall:
      cl->hall = field->value->eval().asInt();
      if ( get_room_index(cl->hall) == NULL ) {
	bug("Invalid clan hall %d for clan [%s]", cl->hall, cl->name );
	//cl->hall = ROOM_VNUM_ALTAR;
	cl->hall = DEFAULT_HALL;
      }
if ( DATA_VERBOSE > 2 ) {
      printf("  hall: %d\n\r", cl->hall );
}
      break;
    case TAG_Recall:
      cl->recall = field->value->eval().asInt();
      if ( get_room_index(cl->recall) == NULL ) {
	bug("Invalid clan recall %d for clan [%s]", cl->recall, cl->name );
	//cl->recall = ROOM_VNUM_TEMPLE;
	cl->recall = DEFAULT_RECALL;
      }
if ( DATA_VERBOSE > 2 ) {
      printf("  recall: %d\n\r", cl->recall );
}
      break;
    case TAG_Donation:
      cl->donation = field->value->eval().asInt();
      if ( get_room_index(cl->donation) == NULL ) {
	bug("Invalid clan donation %d for clan [%s]", cl->donation, cl->name );
	//cl->recall = ROOM_VNUM_TEMPLE;
	cl->donation = DEFAULT_DONATION;
      }
if ( DATA_VERBOSE > 2 ) {
      printf("  donation: %d\n\r", cl->donation );
}
      break;
    case TAG_Morgue:
      cl->morgue = field->value->eval().asInt();
      if ( get_room_index(cl->morgue) == NULL ) {
	bug("Invalid clan morgue %d for clan [%s]", cl->morgue, cl->name );
	//cl->recall = ROOM_VNUM_TEMPLE;
	cl->morgue = DEFAULT_MORGUE;
      }
if ( DATA_VERBOSE > 2 ) {
      printf("  morgue: %d\n\r", cl->morgue );
}
      break;
    case TAG_Independent:
      cl->independent = field->value->eval().asInt();
if ( DATA_VERBOSE > 2 ) {
      printf("  independent: %s\n\r", cl->independent?"true":"false");
}
      break;
    case TAG_Ability: {
      const char *s = field->value->eval().asStr();
      cl->clan_ability = ability_lookup( s );
      if ( cl->clan_ability == -1 )
	bug("Invalid clan ability %s for clan [%s]", s, cl->name );
if ( DATA_VERBOSE > 2 ) {
      printf("  ability: %s\n\r", ability_table[cl->clan_ability].name );
}
      break;
    }
    case TAG_Member: parse_clan_member( field, &cl->members ); break;
    case TAG_Petition: parse_clan_member( field, &cl->petitions ); break;
      break;
      
      // Other
    default: p_error("Invalid Tag: [%s]", tagName ); break;
    }
  }

  clan_count++;
  cl->next = clan_table;
  clan_table = cl;
}
