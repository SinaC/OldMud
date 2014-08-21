#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>

#include "merc.h"
#include "html.h"
#include "comm.h"
#include "config.h"
#include "utils.h"
#include "db.h"
#include "names.h"
#include "language.h"
#include "act_info.h"
#include "tables.h"
#include "group.h"
#include "bit.h"
#include "lookup.h"
#include "const.h"
#include "dbdata.h"
#include "handler.h"
#include "interp.h"
#include "olc_value.h"
#include "arena.h"
#include "clan.h"
#include "classes.h"

static const char *convert_color_to_html( const char *s ) {
  static char buf[MAX_STRING_LENGTH*10];
  char t[2]; 
  const char *p = s;
  buf[0] = '\0';
  bool color = FALSE;
  while( *p != '\0' ) {
    if ( *p == '{' ) {
      p++;
      if ( *p == 'x' ) { // end of color marker
	if ( color )
	  strcat( buf, "</FONT>");
	color = FALSE;
	p++;
      }
      else if ( *p == '{' ) { // normal {
	strcat( buf, "{" );
	p++;
      }
      else if ( *p == '/' ) { // CR
	strcat( buf, "\n" );
	p++;
      }
      else if ( *p == '*' ) // Bip
	p++;
      else { // color code
	if ( color ) // if already in color mode, remove current color code
	  strcat( buf, "</FONT>");
	switch( *p ) {
	case 'b': strcat( buf, "<FONT COLOR=\"#0000aa\">"); color = TRUE; break;
	case 'c': strcat( buf, "<FONT COLOR=\"#00aaaa\">"); color = TRUE; break;
	case 'g': strcat( buf, "<FONT COLOR=\"#00aa00\">"); color = TRUE; break;
	case 'm': strcat( buf, "<FONT COLOR=\"#aa00aa\">"); color = TRUE; break;
	case 'r': strcat( buf, "<FONT COLOR=\"#aa0000\">"); color = TRUE; break;
	case 'w': strcat( buf, "<FONT COLOR=\"#aaaaaa\">"); color = TRUE; break;
	case 'y': strcat( buf, "<FONT COLOR=\"#aaaa00\">"); color = TRUE; break;
	case 'B': strcat( buf, "<FONT COLOR=\"#0000ff\">"); color = TRUE; break;
	case 'C': strcat( buf, "<FONT COLOR=\"#00ffff\">"); color = TRUE; break;
	case 'G': strcat( buf, "<FONT COLOR=\"#00ff00\">"); color = TRUE; break;
	case 'M': strcat( buf, "<FONT COLOR=\"#ff00ff\">"); color = TRUE; break;
	case 'R': strcat( buf, "<FONT COLOR=\"#ff0000\">"); color = TRUE; break;
	case 'W': strcat( buf, "<FONT COLOR=\"#ffffff\">"); color = TRUE; break;
	case 'Y': strcat( buf, "<FONT COLOR=\"#ffff00\">"); color = TRUE; break;
	case 'D': strcat( buf, "<FONT COLOR=\"#ff00ff\">"); color = TRUE; break;
	default: t[0] = *p; t[1] = '\0'; strcat( buf, t ); break;
	}
	p++;
      }
    }
    else {
      t[0] = *p;
      t[1] = '\0';
      strcat( buf, t );
      p++;
    }
  }
  return buf;
}

static const char *convert_txt_to_html( const char *s ) {
  static char buf[MAX_STRING_LENGTH*10];
  const char *p = s;
  char *q = buf;
  while( *p != '\0' ) {
    if ( *p == '\n' ) {
      *q++ = '<';
      *q++ = 'B';
      *q++ = 'R';
      *q++ = '>';
      *p++;
    }
    else if ( *p == '<' ) {
      *q++ = '&';
      *q++ = 'l';
      *q++ = 't';
      //      *q++ = '<';
      //      *q++ = ' ';
      p++;
    }
    else if ( *p == '>' ) {
      *q++ = '&';
      *q++ = 'a';
      *q++ = 'm';
      *q++ = 'p';
      //      *q++ = ' ';
      //      *q++ = '>';
      p++;
    }
    else if ( *p == '&' ) {
      *q++ = '&';
      *q++ = 'g';
      *q++ = 't';
    }
    else if ( *p == '\t' ) {
      *q++ = '&';
      *q++ = 'n';
      *q++ = 'b';
      *q++ = 's';
      *q++ = 'p';
      *q++ = ';';
      p++;
    }
    else if ( *p != '\r' )
      *q++ = *p++;
    else
      p++;
  }
  *q = '\0';
  return convert_color_to_html(buf);
}

static void generate_header( FILE *fp, const char *title ) {
  fprintf( fp, "<HTML>\n" );
  fprintf( fp, "<HEAD>\n");
  fprintf( fp, "<TITLE>%s</TITLE>\n", title );
  fprintf( fp, "<META HTTP-EQUIV=\"Generator\" CONTENT=\"MysteryMud\">\n");
  fprintf( fp, "<META HTTP-EQUIV=\"Date\" CONTENT=\"%s\">\n", (char *) ctime( &current_time ));
  fprintf( fp, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">\n");
  fprintf( fp, "</HEAD>\n");
  fprintf( fp, "<BODY BGCOLOR=\"#dddddd\">\n");
}

static void generate_ender( FILE *fp ) {
  fprintf( fp, "<BR>Last modified: %s\n", (char *) ctime( &current_time ));
  fprintf( fp, "</BODY>\n<HTML>\n" );
}

void generate_html_races_help_files() {
  
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", HTML_DIR, "races.html" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("generate_html_races_help_files: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  generate_header( fp, "Races" );

  fprintf( fp, "<p align=\"left\"><font face=\"Verdana\">\n");
  for ( int i = 0; i < MAX_PC_RACE; i++ )
    fprintf( fp, "<a href=\"#race_%s\">%s</a>&nbsp;&nbsp;\n", race_table[i].name, capitalize(race_table[i].name));
  fprintf( fp, "</font></p><HR>\n");

  for ( int i = 0; i < MAX_PC_RACE; i++ ) {
    pc_race_type *pcRace = &(pc_race_table[i]);
    race_type *race = &(race_table[i]);

    // Link to this race
    fprintf( fp, "<a name=\"race_%s\"></a>\n", race->name );

    // Name
    fprintf( fp, "<FONT COLOR=\"#000000\">%s</FONT><BR>\n", capitalize(pcRace->name) );
    if ( pcRace->type == RACE_NOTAVAILABLE )
      fprintf( fp, "<FONT COLOR=\"#ff0000\">NOT AVAILABLE FOR THE MOMENT</FONT><BR>\n" );
    // Small description
    char buf[MAX_STRING_LENGTH];
    sprintf(buf,"race_%s",race->name);
    fprintf( fp, "<FONT COLOR=\"#888888\">%s</FONT><BR>\n", convert_txt_to_html( small_help_string(buf)));
    // Attributes
    fprintf( fp,
	     "<TABLE CELLPADDING=0 CELLSPACING=0>\n"
	     //	     "<TR><FONT COLOR=\"#cccc00\">"
	     "<TR>\n"
	     "<TD ALIGN=left>   &nbsp;&nbsp;</TD>\n"
	     "<TD ALIGN=left>Str&nbsp;&nbsp;</TD>\n"
	     "<TD ALIGN=left>Int&nbsp;&nbsp;</TD>\n"
	     "<TD ALIGN=left>Wis&nbsp;&nbsp;</TD>\n"
	     "<TD ALIGN=left>Dex&nbsp;&nbsp;</TD>\n"
	     //	     "<TD ALIGN=right>CON</TD></FONT>\n"
	     "<TD ALIGN=left>Con&nbsp;&nbsp;</TD>\n"
	     "</TR>\n"
 	     "<TR>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#555555\">Base attributes&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "</TR>\n"
 	     "<TR>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#555555\">Max attributes&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "<TD ALIGN=right><FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT></TD>\n"
	     "</TR>\n"
	     "</TABLE>\n",
	     pcRace->stats[0], pcRace->stats[1],
	     pcRace->stats[2], pcRace->stats[3],
	     pcRace->stats[4],
	     pcRace->max_stats[0], pcRace->max_stats[1],
	     pcRace->max_stats[2], pcRace->max_stats[3],
	     pcRace->max_stats[4]);
    // Alignments
    fprintf( fp, "<FONT COLOR=\"#000000\">Alignments allowed:&nbsp;</FONT><FONT COLOR=\"#555555\">\n");
    if ( pcRace->nb_allowed_align == 9 )
      fprintf( fp, "Any");
    else
      for ( int i = 0; i < pcRace->nb_allowed_align; i++ ) {
	int etho = pcRace->allowed_align[i].etho + 1;
	int align = pcRace->allowed_align[i].alignment/350 + 1;
	int index = etho + align * 3;
	fprintf( fp, "%2s", short_etho_align[index] );
	if ( i < pcRace->nb_allowed_align-1 )
	  fprintf( fp, ",&nbsp;");
      }
    fprintf( fp, "</FONT><BR>\n");
    // Classes
    fprintf( fp, "<FONT COLOR=\"#000000\">Classes allowed:&nbsp;</FONT><FONT COLOR=\"#555555\">\n" );
    int allowed_class = pcRace->allowed_class;
    int count_class = count_bit( allowed_class );
    if ( count_class == MAX_CLASS )
      fprintf( fp, "Any" );
    else {
      int count = 0;
      for ( long i = 0; i < MAX_CLASS; i++ )
	if ( ( 1 << i ) & allowed_class ) {
	  fprintf( fp, "<a href=\"classes.html#class_%s\">%s</a>\n", class_table[i].name, capitalize(class_table[i].name) );
	  if ( count < count_class-1 )
	    fprintf( fp, ",&nbsp;");
	  //	  if ( (count+1) % 5 == 0 )
	  //	    fprintf( fp, "<BR>\n");
	  count++;
	}
    }
    fprintf( fp, "</FONT><BR>\n");
    // Size  Exp/level
    fprintf( fp,
	     "<FONT COLOR=\"#000000\">Size:&nbsp;</FONT><FONT COLOR=\"#cc00cc\">%s</FONT><BR>\n"
	     "<FONT COLOR=\"#000000\">Base experience cost per level:&nbsp;</FONT><FONT COLOR=\"#cc0000\">%d</FONT><BR>\n",
	     //capitalize(size_table[pcRace->size].name),
	     capitalize(size_table[race->size].name),
	     pcRace->expl);
    // Affects & Affects2
    const char *aff;
    const char *aff2;
    aff = list_flag_string_init( race->aff, affect_flags, "", ",&nbsp;");
    aff2 = list_flag_string_init( race->aff2, affect2_flags, "", ",&nbsp;");
    if ( !str_cmp( aff, "none" ) ) // no affect
      if ( !str_cmp( aff2, "none" ) ) // no affect neither affect2
	fprintf( fp, "<FONT COLOR=\"#000000\">Affected by:&nbsp;</FONT><FONT COLOR=\"#00cc00\">none</FONT><BR\n>\n");
      else // not affect but affect2
	fprintf( fp, "<FONT COLOR=\"#000000\">Affected by:&nbsp;</FONT><FONT COLOR=\"#00cc00\">%s</FONT><BR>\n", aff2 );
    else // affect
      if ( !str_cmp( aff2, "none") ) // affect but no affect2
	fprintf( fp, "<FONT COLOR=\"#000000\">Affected by:&nbsp;</FONT><FONT COLOR=\"#00cc00\">%s</FONT><BR>\n", aff );
      else // affect and affect 2
	fprintf( fp, "<FONT COLOR=\"#000000\">Affected by:&nbsp;</FONT><FONT COLOR=\"#00cc00\">%s,&nbsp;%s</FONT><BR>\n", aff, aff2 );
    // Immunities & Resistances & Vulnerabilities
    fprintf( fp, "<FONT COLOR=\"#000000\">Immunities:&nbsp;</FONT><FONT COLOR=\"#cccc00\">%s</FONT><BR>\n", list_flag_string_init(race->imm,irv_flags, "", ",&nbsp;"));
    fprintf( fp, "<FONT COLOR=\"#000000\">Resistances:&nbsp;</FONT><FONT COLOR=\"#cc00000\">%s</FONT><BR>\n", list_flag_string_init(race->res,irv_flags, "", ",&nbsp;"));
    fprintf( fp, "<FONT COLOR=\"#000000\">Vulnerabilities:&nbsp;</FONT><FONT COLOR=\"#0000cc\">%s</FONT><BR>\n", list_flag_string_init(race->vuln,irv_flags, "", ",&nbsp;"));
    // Natural born abilities
    fprintf( fp, "<FONT COLOR=\"#000000\">Special abilities:&nbsp;</FONT><FONT COLOR=\"#00cc00\">\n");
    bool found = FALSE;
    for ( int i = 0; i < pcRace->nb_abilities; i++ ) {
      if ( pcRace->abilities[i] <= 0 )
	continue;
      found = TRUE;
      fprintf( fp,"<a href=\"abilities.html#ability_%s\">%s</a>\n", ability_table[pcRace->abilities[i]].name, capitalize(ability_table[pcRace->abilities[i]].name) );
      if ( i < pcRace->nb_abilities-1 )
	fprintf( fp, ",&nbsp;");
    }
    if (!found)
      fprintf( fp, "none" );
    fprintf( fp, "</FONT><BR>\n");
    // Languages
    fprintf( fp, "<FONT COLOR=\"#000000\">Racial language:&nbsp;</FONT><FONT COLOR=\"#cc00cc\">%s</FONT><BR>\n", capitalize(language_name(pcRace->language)));
    // Remort
    fprintf( fp, "<FONT COLOR=\"#000000\">Remort options:&nbsp;</FONT><FONT COLOR=\"#555555\">\n" );
    found = FALSE;
    for ( int i = 0; i < pcRace->nb_remorts; i++ ) {
      if ( pcRace->remorts[i] < 0 )
	continue;
      found = TRUE;
      fprintf( fp, "<a href=\"#race_%s\">%s</a>\n", pc_race_table[pcRace->remorts[i]].name, capitalize(pc_race_table[pcRace->remorts[i]].name) );
      if ( i < pcRace->nb_remorts-1 )
	fprintf( fp, ",&nbsp;");
    }
    if (!found)
      fprintf( fp, "none" );
    fprintf( fp, "</FONT><BR>\n");
    fprintf( fp, "<HR>\n");
  }

  generate_ender( fp );

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

static int iClass;
static int compare( const void *a, const void *b ) {
  int i1 = ability_table[(int)(*((const int*)a))].ability_level[iClass];
  int i2 = ability_table[(int)(*((const int*)b))].ability_level[iClass];
  if ( i1 < i2 ) return -1;
  if ( i1 > i2 ) return 1;
  return 0;
}
static void generate_abilities_list( FILE *fp, const int nb_abilities, const int *abilities_list, const char *name ) {
  if ( nb_abilities <= 0 )
    return;
  fprintf( fp, "<FONT COLOR=\"#000000\">%s:</FONT><FONT COLOR=\"#555555\">\n", name );
  fprintf( fp, "<TABLE CELLPADDING=0 CELLSPACING=0>\n");
  fprintf( fp, "<TR><TD ALIGN=right>Name&nbsp;&nbsp;</TD><TD ALIGN=right>Lvl&nbsp;&nbsp;</TD><TD ALIGN=right>Name</TD><TD ALIGN=right>Lvl</TD><TD ALIGN=right>Name</TD><TD ALIGN=right>Lvl</TD></TR>\n");
  int col = 0;
  for ( int i = 0; i < nb_abilities; i++ ) {
    fprintf( fp, "<TD ALIGN=right><a href=\"abilities.html#ability_%s\">%s</a>&nbsp;&nbsp;</TD>\n", ability_table[abilities_list[i]].name, capitalize(ability_table[abilities_list[i]].name) );
    fprintf( fp, "<TD ALIGN=right>%d&nbsp;&nbsp;</TD>\n", ability_table[abilities_list[i]].ability_level[iClass] );
    if (++col % 3 == 0)
      fprintf( fp, "</TR><TR>\n");
  }
  if ( col % 3 != 0 )
    fprintf( fp, "</TR>\n" );
  fprintf( fp, "</TABLE></FONT><BR>\n");
}
static void generate_group_list( FILE *fp, const int gn ) {
  fprintf( fp, "<FONT COLOR=\"#cccc00\">\n" );
  fprintf( fp, "<TABLE CELLPADDING=0 CELLSPACING=0><TR>\n");
  group_type *gr = &(group_table[gn]);
  int col = 0;
  for ( int sn = 0; sn < gr->spellnb; sn++) {
    int tsn = group_lookup( gr->spells[sn] );
    if ( tsn > 0 )
      fprintf( fp, "<TD ALIGN=right>%s&nbsp;&nbsp;&nbsp;</TD>\n", capitalize(group_table[tsn].name));
    else {
      tsn = ability_lookup( gr->spells[sn] );
      if ( tsn <= 0 ) continue;
      fprintf( fp,"<TD ALIGN=right><a href=\"abilities.html#ability_%s\">%s</a>&nbsp;&nbsp;&nbsp;</TD>\n", gr->spells[sn], capitalize(gr->spells[sn]) );
    }
    if (++col % 3 == 0)
      fprintf( fp, "</TR><TR>\n");
  }
  if ( col % 3 != 0 )
    fprintf( fp, "</TR>\n" );
  fprintf( fp, "</TABLE></FONT><BR>\n");
}
void generate_html_classes_help_files() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", HTML_DIR, "classes.html" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("generate_html_classes_help_files: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  generate_header( fp, "Classes" );

  fprintf( fp, "<p align=\"left\"><font face=\"Verdana\">\n");
  for ( int i = 0; i < MAX_CLASS; i++ )
    fprintf( fp, "<a href=\"#class_%s\">%s</a>&nbsp;&nbsp;\n", class_table[i].name, capitalize(class_table[i].name));
  fprintf( fp, "</font></p><HR>\n");

  int curspell, curskill, curpower, cursong;
  int skills[MAX_ABILITY+1];
  int spells[MAX_ABILITY+1];
  int powers[MAX_ABILITY+1];
  int songs[MAX_ABILITY+1];
  for ( iClass = 0; iClass < MAX_CLASS; iClass++ ) {
    class_type *cl = &(class_table[iClass]);
    // Link to this class
    fprintf( fp, "<a name=\"class_%s\"></a>\n", cl->name );

    // Count abilities
    curskill = 0; curspell = 0; curpower = 0; cursong = 0;
    
    for ( int i = 1; i < MAX_ABILITY; i++ ) {
      if ( ability_table[i].name == NULL || 
	   ability_table[i].name[0] == '\0' )
	break;
      if ( ability_table[i].rating[iClass] > 0
	   || ability_table[i].ability_level[iClass] < LEVEL_IMMORTAL ) {
	switch( ability_table[i].type ) {
	case TYPE_SKILL: skills[curskill++] = i; break;
	case TYPE_SPELL: spells[curspell++] = i; break;
	case TYPE_POWER: powers[curpower++] = i; break;
	case TYPE_SONG: songs[cursong++] = i; break;
	}
      }
    }
    if ( curskill > 0 ) qsort( skills, curskill, sizeof(skills[0]), compare );
    if ( curspell > 0 ) qsort( spells, curspell, sizeof(spells[0]), compare );
    if ( curpower > 0 ) qsort( powers, curpower, sizeof(powers[0]), compare );
    if ( curspell > 0 ) qsort( songs, cursong, sizeof(songs[0]), compare );

    // Name
    fprintf( fp, "<FONT COLOR=\"#000000\">%s</FONT><BR>\n", capitalize(cl->name) );
    if ( cl->choosable == CLASS_CHOOSABLE_NEVER )
      fprintf( fp, "<FONT COLOR=\"#ff0000\">NOT AVAILABLE FOR THE MOMENT</FONT><BR>\n" );

    // Small description
    char buf[MAX_STRING_LENGTH];
    sprintf(buf,"class_%s", cl->name);
    fprintf( fp, "<FONT COLOR=\"#888888\">%s</FONT><BR>\n", convert_txt_to_html( small_help_string(buf)));

    // Alignments
    fprintf( fp, "<FONT COLOR=\"#000000\">Alignments allowed:&nbsp;</FONT><FONT COLOR=\"#555555\">\n");
    if ( cl->nb_allowed_align == 9 )
      fprintf( fp, "Any");
    else
      for ( int i = 0; i < cl->nb_allowed_align; i++ ) {
	int etho = cl->allowed_align[i].etho + 1;
	int align = cl->allowed_align[i].alignment/350 + 1;
	int index = etho + align * 3;
	fprintf( fp, "%2s", short_etho_align[index] );
	if ( i < cl->nb_allowed_align-1 )
	  fprintf( fp, ",&nbsp;");
      }
    fprintf( fp, "</FONT><BR>\n");

    // Prime stat
    fprintf( fp, "<FONT COLOR=\"#000000\">Prime Requisite Stat:&nbsp;</FONT><FONT COLOR=\"#555555\">%s</FONT><BR>\n",
	     capitalize( flag_string_init( attr_flags, cl->attr_prime+ATTR_stat0 ) ) );

    // Sub-classes
    if ( cl->nb_sub_classes != 0 ) {
      fprintf( fp, "<FONT COLOR=\"#000000\">Sub-Classes:&nbsp;</FONT><FONT COLOR=\"#555555\">\n");
      for ( int i = 0; i < cl->nb_sub_classes; i++ ) {
	fprintf( fp,"<a href=\"#class_%s\">%s</a>\n", class_table[cl->sub_classes[i]].name, capitalize(class_table[cl->sub_classes[i]].name) );
	if ( i < cl->nb_sub_classes-1 )
	  fprintf( fp, ",&nbsp;");
      }
      fprintf( fp, "<FONT><BR>\n");
    }

    // Basic abilities
    int gn = group_lookup(cl->base_group);
    if ( gn >= 0 ) {
      fprintf( fp, "<FONT COLOR=\"#000000\">Basic %s abilities:</FONT>", capitalize(cl->name) );
      generate_group_list( fp, gn );
    }
    else
      fprintf( fp, "<FONT COLOR=\"#ff0000\">No basic group</FONT><BR>\n");

    // Abilities
    int col;
    generate_abilities_list( fp, curskill, skills, "Skills" );
    generate_abilities_list( fp, curspell, spells, "Spells" );
    generate_abilities_list( fp, curpower, powers, "Powers" );
    generate_abilities_list( fp, cursong, songs, "Songs" );

    fprintf( fp, "</FONT><HR>\n");
  }

  generate_ender( fp );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

const char *classes_to_string( const int classes ) {
  static char buf[MAX_STRING_LENGTH];
  buf[0] = '\0';
  int countClass = count_bit( classes );
  int count = 0;
  for ( int i = 0; i < MAX_CLASS; i++ )
    if ( ( 1 << i ) & classes ) {
      char buf2[MAX_INPUT_LENGTH];
      sprintf( buf2, "<a href=\"classes.html#class_%s\">%s</a>", class_table[i].name, capitalize( class_table[i].name ) );
      if ( count < countClass-1 )
	strcat( buf2, ",&nbsp;");
      count++;
      strcat( buf, buf2 );
    }
  return buf;
}
static void generate_abilities_list2( FILE *fp, const int count, const int *list ) {
  for ( int ability = 0; ability < count; ability++ ) {
    int sn = list[ability];
    ability_type *pAbility = &(ability_table[sn]);
    // Link to this ability
    fprintf( fp, "<a name=\"ability_%s\"></a>\n", pAbility->name );
    // Name + type
    fprintf( fp, "<FONT COLOR=\"#000000\">%s:&nbsp;</FONT><FONT COLOR=\"0000ff\">%s</FONT>\n",
	     abilitytype_name( pAbility->type ),
	     capitalize(pAbility->name) );
    // # Casting level
    if ( pAbility->nb_casting_level != 0 )
      fprintf( fp, "&nbsp;&nbsp;<FONT COLOR=\"#aa00aa\">%2d</FONT>&nbsp;<FONT COLOR=\"#000000\">level%s</FONT>\n", pAbility->nb_casting_level, pAbility->nb_casting_level > 1 ?"s":"" );
    fprintf( fp, "<BR>\n");

    // School
    if ( pAbility->school > 0 )
      fprintf( fp, "<FONT COLOR=\"#000000\">School:</FONT>&nbsp;<FONT COLOR=\"#000099\">%s</FONT><BR>\n", capitalize(magic_school_table[pAbility->school].name) );
    // Sphere
    int sphereId = is_ability_in_sphere( sn );
    if ( sphereId >= 0 )
      fprintf( fp, "<FONT COLOR=\"#000000\">Sphere:</FONT>&nbsp;<FONT COLOR=\"#000099\">%s</FONT><BR>\n", capitalize(group_table[sphereId].name) );

    // Automatic
    //if ( pAbility->mob_use == MOB_USE_AUTOMATIC ) SinaC 2003
    if ( pAbility->action_fun == NULL )
      fprintf( fp, "<FONT COLOR=\"#555555\">Automatic skill.</FONT><BR>\n");
  
    // Small description
    char buf[MAX_STRING_LENGTH];
    sprintf( buf, "'%s_%s'", abilitytype_name(pAbility->type), pAbility->name);
    fprintf( fp, "<FONT COLOR=\"#888888\">%s</FONT><BR>\n", convert_txt_to_html( small_help_string( buf ) ) );

    // Prerequisites
    if ( pAbility->prereqs != NULL ) {
      fprintf( fp, "<FONT COLOR=\"#000000\">Prerequisites:</FONT><BR>\n" );
      for ( int i = 0; i < pAbility->nb_casting_level+1; i++ ) {
	if ( i == 0 && pAbility->prereqs[i].nb_prereq == 0 )
	  continue;
	PREREQ_DATA *prereq = &(pAbility->prereqs[i]);
	char buf3[MAX_STRING_LENGTH];
	bool noLevel = FALSE;
	if ( i != 0 || pAbility->nb_casting_level != 0 ) {
	  if ( prereq->plr_level > 1 )
	    sprintf( buf3, "&nbsp;level=%3d", prereq->plr_level );
	  else
	    buf3[0] = '\0';
	  char buf2[MAX_STRING_LENGTH];
	  if ( prereq->classes!=ANY_CLASSES )
	    sprintf( buf2, "&nbsp;classes=%s", classes_to_string(prereq->classes) );
	  else
	    buf2[0] = '\0';
	  fprintf( fp, "<TABLE CELLPADDING=0 CELLSPACING=0><TR>\n" );
	  fprintf( fp, "<TD ALIGN=left>&nbsp;<FONT COLOR=\"#00aa00\">Level %d&nbsp;</FONT><FONT COLOR=\"#aa0000\">[cost=%2d%s%s]:&nbsp;</FONT></TD>\n",
		   i, prereq->cost, buf3, buf2 );
	}
	else
	  noLevel = TRUE;
	if ( pAbility->prereqs[i].nb_prereq == 0 )
	  fprintf( fp, "<TD ALIGN=left>&nbsp;&nbsp;no prerequisites.</TD></TR>");
	else {
	  for ( int j = 0; j < pAbility->prereqs[i].nb_prereq; j++ ) {
	    buf3[0] = '\0';
	    if ( pAbility->prereqs[i].prereq[j].casting_level > 0 )
	      sprintf( buf3, "&nbsp;level %d", pAbility->prereqs[i].prereq[j].casting_level );
	    if ( j > 0 )
	      fprintf( fp, "<TR><TD ALIGN=left></TD>\n");
	    if ( !noLevel )
	      fprintf( fp, "<TD ALIGN=left>");
	    fprintf( fp,
		     "%s<FONT COLOR=\"#\"><a href=\"#ability_%s\">%s</a></FONT><FONT COLOR=\"#\">%s</FONT>",
		     noLevel?"&nbsp;":"&nbsp;&nbsp;",
		     ability_table[pAbility->prereqs[i].prereq[j].sn].name,
		     capitalize(ability_table[pAbility->prereqs[i].prereq[j].sn].name),
		     buf3 );
	    if ( !noLevel )
	      fprintf( fp, "</TD>\n");
	    else if ( j < pAbility->prereqs[i].nb_prereq-1 )
	      fprintf( fp, ",&nbsp;" );
	    if ( j > 0 )
	      fprintf( fp, "</TR>\n");
	  }
	}
	if ( !noLevel )
	  fprintf( fp, "</TABLE>\n");
      }
      fprintf( fp, "<BR>\n");
    }

    // Classes
    if ( pAbility->type == TYPE_SKILL
	 || pAbility->type == TYPE_SPELL
	 || pAbility->type == TYPE_POWER
	 || pAbility->type == TYPE_SONG ) {
      sprintf( buf, "<FONT COLOR=\"#000000\">Classes:<FONT><BR>\n"
	       "<TABLE CELLPADDING=0 CELLSPACING=0><TR>\n");
      int col = 0;
      for ( int i = 0; i < MAX_CLASS; i++ ) {
	int level;
	if ( ( level = pAbility->ability_level[i] ) < IM ) {
	  char buf2[MAX_INPUT_LENGTH];
	  sprintf( buf2, "<TD ALIGN=left>&nbsp;<FONT COLOR=\"#aaaa00\"><a href=\"classes.html#class_%s\">%s</a>&nbsp;&nbsp;</FONT></TD><TD ALIGN=left>level:&nbsp;<FONT COLOR=\"#cc00cc\">%d&nbsp;&nbsp;</FONT><TD>\n", class_table[i].name, capitalize(class_table[i].name), level );
	  if ( ++col % 3 == 0 )
	    strcat( buf2, "</TR><TR>\n");
	  strcat( buf, buf2 );
	}
      }
      if ( col > 0 ) {
	fprintf( fp, buf );
	if ( col % 3 != 0 )
	  fprintf( fp, "</TR>\n");
	fprintf( fp, "</TABLE>\n");
      }
      else
	fprintf( fp, "<FONT COLOR=\"#ff0000\">Not available for any classes</FONT>\n");
      fprintf( fp, "<BR>\n");
    }

    // Natural born abilities
    bool found = FALSE;
    sprintf( buf, "<FONT COLOR=\"#000000\">Natural born ability for:&nbsp;</FONT><FONT COLOR=\"#aa00aa\">" );
    for ( int i = 0; i < MAX_PC_RACE; i++ ) {
      for ( int j = 0; j < pc_race_table[i].nb_abilities; j++ ) {
	if ( pc_race_table[i].abilities[j] <= 0 )
	  continue;
	if ( pc_race_table[i].abilities[j] == sn ) {
	  char buf2[MAX_INPUT_LENGTH];
	  sprintf( buf2, "<a href=\"races.html#race_%s\">%s</a>&nbsp;&nbsp;", pc_race_table[ i ].name, capitalize(pc_race_table[ i ].name) );
	  strcat( buf, buf2 );
	  found = TRUE;
	  break;
	}
      }
    }
    if ( found )
      fprintf( fp, "%s</FONT><BR>\n", buf );
    fprintf( fp, "<HR>\n");
  }
}
void generate_html_abilities_help_files() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", HTML_DIR, "abilities.html" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("generate_html_abilities_help_files: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  generate_header( fp, "Abilities" );

  // Links to skills/spells/powers/songs
  fprintf( fp, "<p align=\"center\"><font face=\"Verdana\">\n");
  fprintf( fp, "<a href=\"#SKILLS\">Skills</a>&nbsp;&nbsp;&nbsp;\n");
  fprintf( fp, "<a href=\"#SPELLS\">Spells</a>&nbsp;&nbsp;&nbsp;\n");
  fprintf( fp, "<a href=\"#POWERS\">Powers</a>&nbsp;&nbsp;&nbsp;\n");
  fprintf( fp, "<a href=\"#SONGS\">Songs</a>\n");
  fprintf( fp, "</font></p>\n");

  // Separate different type of abilities
  int skills[MAX_ABILITY]; int skillC = 0;
  int spells[MAX_ABILITY]; int spellC = 0;
  int powers[MAX_ABILITY]; int powerC = 0;
  int songs[MAX_ABILITY]; int songC = 0;
  for ( int i = 1; i < MAX_ABILITY; i++ ) {
    switch( ability_table[i].type ) {
    case TYPE_SKILL: skills[skillC++] = i; break;
    case TYPE_SPELL: spells[spellC++] = i; break;
    case TYPE_POWER: powers[powerC++] = i; break;
    case TYPE_SONG: songs[songC++] = i; break;
    default: break; // special abilities
    }
  }

  // Link to Skills
  fprintf( fp, "<a name=\"SKILLS\"></a><FONT COLOR=\"#000000\">SKILLS</FONT><BR>\n" );
  // Generate Skills
  generate_abilities_list2( fp, skillC, skills );
  fprintf( fp, "<HR>\n" );
  // Link to Spells
  fprintf( fp, "<a name=\"SPELLS\"></a><FONT COLOR=\"#000000\">SPELLS</FONT><BR>\n" );
  // Generate Spells
  generate_abilities_list2( fp, spellC, spells );
  fprintf( fp, "<HR>\n" );
  // Link to Powers
  fprintf( fp, "<a name=\"POWERS\"></a><FONT COLOR=\"#000000\">POWERS</FONT><BR>\n" );
  // Generate Powers
  generate_abilities_list2( fp, powerC, powers );
  fprintf( fp, "<HR>\n" );
  // Link to Songs
  fprintf( fp, "<a name=\"SONGS\"></a><FONT COLOR=\"#000000\">SONGS</FONT><BR>\n" );
  // Generate Songs
  generate_abilities_list2( fp, songC, songs );

  generate_ender( fp );

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void generate_html_gods_help_files() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", HTML_DIR, "gods.html" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("generate_html_gods_help_files: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  generate_header( fp, "Gods" );

  fprintf( fp, "<p align=\"left\"><font face=\"Verdana\">\n");
  for ( int i = 0; i < MAX_GODS; i++ )
    fprintf( fp, "<a href=\"#god_%s\">%s</a>&nbsp;&nbsp;\n", gods_table[i].name, capitalize(gods_table[i].name));
  fprintf( fp, "</font></p><HR>\n");

  for ( int godId = 0; godId < MAX_GODS; godId++ ) {
    char buf[MAX_STRING_LENGTH];
    god_data *god = &(gods_table[godId]);

    // Link to this god
    fprintf( fp, "<a name=\"god_%s\"></a>\n", god->name );

    // Name
    fprintf( fp, "<FONT COLOR=\"#000000\">%s, %s</FONT><BR>\n", capitalize(god->name), god->title );

    // Story
    fprintf( fp, "<FONT COLOR=\"#888888\">%s</FONT><BR>\n", convert_txt_to_html(god->story) );

    // Alignments
    fprintf( fp, "<FONT COLOR=\"#000000\">Alignments allowed:&nbsp;</FONT><FONT COLOR=\"#555555\">\n");
    if ( god->nb_allowed_align == 9 )
      fprintf( fp, "Any");
    else
      for ( int i = 0; i < god->nb_allowed_align; i++ ) {
	int etho = god->allowed_align[i].etho + 1;
	int align = god->allowed_align[i].alignment/350 + 1;
	int index = etho + align * 3;
	fprintf( fp, "%2s", short_etho_align[index] );
	if ( i < god->nb_allowed_align-1 )
	  fprintf( fp, ",&nbsp;");
      }
    fprintf( fp, "</FONT><BR>\n");

    // Classes
    if ( !god->acceptWildMagic ) sprintf( buf, " (non-wild)" );
    else buf[0] = '\0';
    fprintf( fp, "<FONT COLOR=\"#000000\">Classes allowed%s:&nbsp;</FONT><FONT COLOR=\"#555555\">\n", buf );
    int allowed_class = god->allowed_class;
    int count_class = count_bit( allowed_class );
    if ( count_class == MAX_CLASS )
      fprintf( fp, "Any" );
    else {
      int count = 0;
      for ( long i = 0; i < MAX_CLASS; i++ )
	if ( ( 1 << i ) & allowed_class ) {
	  fprintf( fp, "<a href=\"classes.html#class_%s\">%s</a>\n", class_table[i].name, capitalize(class_table[i].name) );
	  if ( count < count_class-1 )
	    fprintf( fp, ",&nbsp;");
	  //	  if ( (count+1) % 5 == 0 )
	  //	    fprintf( fp, "<BR>\n");
	  count++;
	}
    }
    fprintf( fp, "</FONT><BR>\n");

    // Races allowed
    fprintf( fp, "<FONT COLOR=\"#000000\">Races allowed:&nbsp;</FONT><FONT COLOR=\"#555555\">\n" );
    for ( int i = 0; i < god->nb_allowed_race; i++ ) {
      fprintf( fp, "<a href=\"races.html#race_%s\">%s</a>\n", race_table[god->allowed_race[i]].name, capitalize(race_table[god->allowed_race[i]].name) );
      if ( i < god->nb_allowed_race-1 )
	fprintf( fp, ",&nbsp;");
    }
    fprintf( fp, "</FONT><BR>\n");

    // Minor Sphere
    if ( god->minor_sphere > 0 )
      fprintf( fp, "<FONT COLOR=\"#000000\">Minor Sphere:&nbsp;</FONT><FONT COLOR=\"#555555\"><a href=\"groups.html#group_%s\">%s</a></FONT>\n",
	       group_table[god->minor_sphere].name, capitalize(group_table[god->minor_sphere].name));
    else
      fprintf( fp, "<FONT COLOR=\"#000000\">Minor Sphere:&nbsp;</FONT><FONT COLOR=\"#555555\">no minor sphere</FONT>\n");
    fprintf( fp, "<BR>\n");

    // Major sphere
    fprintf( fp, "<FONT COLOR=\"#000000\">Major spheres: </FONT><FONT COLOR=\"#555555\">\n" );
    fprintf( fp, "<TABLE CELLPADDING=0 CELLSPACING=0>\n");
    int col = 0;
    for ( int i = 0; i < god->nb_major_sphere; i++ ) {
      fprintf( fp, "<TD ALIGN=right><a href=\"groups.html#group_%s\">%s</a>&nbsp;&nbsp;</TD>\n", group_table[god->major_sphere[i]].name, capitalize(group_table[god->major_sphere[i]].name) );
      if (++col % 2 == 0)
	fprintf( fp, "</TR><TR>\n");
    }
    if ( col % 2 != 0 )
      fprintf( fp, "</TR>\n" );
    fprintf( fp, "</TABLE></FONT><BR>\n");
   
    // Priest
    fprintf( fp, "<FONT COLOR=\"#000000\">Classes getting minor sphere (Priest):&nbsp;</FONT><FONT COLOR=\"#555555\">\n" );
    allowed_class = god->priest;
    count_class = count_bit( allowed_class );
    if ( count_class == MAX_CLASS )
      fprintf( fp, "Any" );
    else {
      int count = 0;
      for ( long i = 0; i < MAX_CLASS; i++ )
	if ( ( 1 << i ) & allowed_class ) {
	  fprintf( fp, "<a href=\"classes.html#class_%s\">%s</a>\n", class_table[i].name, capitalize(class_table[i].name) );
	  if ( count < count_class-1 )
	    fprintf( fp, ",&nbsp;");
	  //	  if ( (count+1) % 5 == 0 )
	  //	    fprintf( fp, "<BR>\n");
	  count++;
	}
    }
    fprintf( fp, "</FONT><BR>\n");

    fprintf( fp, "</FONT><BR>\n");
    fprintf( fp, "<HR>\n");
  }

  generate_ender( fp );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void generate_html_groups_help_files() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", HTML_DIR, "groups.html" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("generate_html_groups_help_files: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }
  generate_header( fp, "Groups" );

  for ( int groupId = 0; groupId < MAX_GROUP; groupId++ ) {
    char buf[MAX_STRING_LENGTH];
    group_type *gr = &(group_table[groupId]);

    // Link to this group
    fprintf( fp, "<a name=\"group_%s\"></a>\n", gr->name );

    // Name
    if ( gr->isSphere )
      fprintf( fp, "<FONT COLOR=\"#000000\">Sphere:&nbsp;</FONT><FONT COLOR=\"0000ff\">%s</FONT><BR>\n", capitalize(gr->name) );
    else
      fprintf( fp, "<FONT COLOR=\"#000000\">Group:&nbsp;</FONT><FONT COLOR=\"0000ff\">%s</FONT><BR>\n", capitalize(gr->name) );

    fprintf( fp, "<TABLE CELLPADDING=0 CELLSPACING=0>\n");
    int col = 0;
    for ( int i = 0; i < gr->spellnb; i++ ) {
      int tsn = group_lookup( gr->spells[i] );
      if ( tsn > 0 )
	fprintf( fp, "<TD ALIGN=right><a href=\"groups.html#group_%s\">%s</a>&nbsp;&nbsp;</TD>\n", gr->spells[i], capitalize(gr->spells[i]) );
      else {
	tsn = ability_lookup(gr->spells[i]);
	if ( tsn <= 0 ) {
	  bug("group [%s] contains an unknown group/ability [%s]", gr->name, gr->spells[i] );
	  continue;
	}
	fprintf( fp, "<TD ALIGN=right><a href=\"abilities.html#ability_%s\">%s</a>&nbsp;&nbsp;</TD>\n", gr->spells[i], capitalize(gr->spells[i]) );
      }
      if (++col % 3 == 0)
	fprintf( fp, "</TR><TR>\n");
    }
    if ( col % 3 != 0 )
      fprintf( fp, "</TR>\n" );
    fprintf( fp, "</TABLE></FONT><BR><HR>\n");
  }
 
  generate_ender( fp );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void do_generate_html( CHAR_DATA *ch, const char *argument ) {
  bool fAll = FALSE;
  bool found = FALSE;
  if ( argument[0] == '\0' || !str_cmp( argument, "all" ) )
    fAll = TRUE;
  if ( !str_cmp( argument, "races" ) || fAll ) {
    generate_html_races_help_files();
    send_to_charf(ch,"Races help files generated.\n\r");
    found = TRUE;
  }
  if ( !str_cmp( argument, "classes" ) || fAll ) {
    generate_html_classes_help_files();
    send_to_charf(ch,"Classes help files generated.\n\r");
    found = TRUE;
  }
  if ( !str_cmp( argument, "abilities" ) || fAll ) {
    generate_html_abilities_help_files();
    send_to_charf(ch,"Abilities help files generated.\n\r");
    found = TRUE;
  }
  if ( !str_cmp( argument, "gods" ) || fAll ) {
    generate_html_gods_help_files();
    send_to_charf(ch,"Gods help files generated.\n\r");
    found = TRUE;
  }
  if ( !str_cmp( argument, "groups" ) || fAll ) {
    generate_html_gods_help_files();
    send_to_charf(ch,"Groups help files generated.\n\r");
    found = TRUE;
  }
  if ( !str_cmp( argument, "who" ) || fAll ) {
    generate_html_who();
    send_to_charf(ch,"Who generated.\n\r");
    found = TRUE;
  }
  if ( !found ) {
    send_to_charf(ch,
		  "Syntax: generatehtml all       generates every help files\n\r"
		  "        generatehtml races     generates races help files\n\r"
		  "        generatehtml classes   generates classes help files\n\r"
		  "        generatehtml abilities generates abilities help files\n\r"
		  "        generatehtml gods      generates gods help files\n\r"
		  "        generatehtml groups    generates groups help files\n\r"
		  "        generatehtml who       generates who\n\r");
    return;
  }
}


void generate_html_who() {
  fclose(fpReserve);
  char filename[MAX_STRING_LENGTH];
  sprintf( filename, "%s%s", HTML_DIR, "who.html" );
  FILE *fp;
  if ( ( fp = fopen( filename, "w" ) ) == NULL ) {
    bug("generate_html_who: Can't open file %s", filename );
    fpReserve = fopen( NULL_FILE, "r" );
    return;
  }

  fprintf( fp, "<HTML>\n" );
  fprintf( fp, "<HEAD>\n");
  fprintf( fp, "<TITLE>Who</TITLE>\n" );
  fprintf( fp, "<META HTTP-EQUIV=\"Generator\" CONTENT=\"MysteryMud\">\n");
  fprintf( fp, "<META HTTP-EQUIV=\"Date\" CONTENT=\"%s\">\n", (char *) ctime( &current_time ));
  fprintf( fp, "<META HTTP-EQUIV=\"Content-Type\" CONTENT=\"text/html; charset=iso-8859-1\">\n");
  fprintf( fp, "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"60\">\n");
  fprintf( fp, "</HEAD>\n");
  fprintf( fp, "<BODY BGCOLOR=\"#dddddd\">\n");

  fprintf( fp, "<TABLE CELLPADDING=0 CELLSPACING=0>\n");
  extern int max_players;
  int nMatch = 0;
  for ( DESCRIPTOR_DATA *d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *wch;
    
    if ( (d->connected != CON_PLAYING 
	  && (d->connected < CON_NOTE_TO 
	      || d->connected > CON_NOTE_FINISH) ) )
      continue;
    
    wch   = ( d->original != NULL ) ? d->original : d->character;
    
    nMatch++;

    // Figure out what to print for class.
    const char *cla = class_whoname(wch->bstat(classes));
    switch ( wch->level ) {
    default: break;
    case MAX_LEVEL - 0 : cla = "IMP"; break;
    case MAX_LEVEL - 1 : cla = "CRE"; break;
    case MAX_LEVEL - 2 : cla = "SUP"; break;
    case MAX_LEVEL - 3 : cla = "DEI"; break;
    case MAX_LEVEL - 4 : cla = "GOD"; break;
    case MAX_LEVEL - 5 : cla = "IMM"; break;
    case MAX_LEVEL - 6 : cla = "DEM"; break;
    case MAX_LEVEL - 7 : cla = "ANG"; break;
    case MAX_LEVEL - 8 : cla = "AVA"; break;
    case MAX_LEVEL - 9 : cla = "{cHERO{x"; break;
    }
    if ( wch->pcdata->immtitle!=NULL 
	 && wch->pcdata->immtitle[0]!='\0')
      cla = wch->pcdata->immtitle;
    // Format it up.
    char buf[MAX_STRING_LENGTH*10];
    buf[0] = '\0';
    if( wch->level>MAX_LEVEL-10 )
      sprintf( buf, "<TR><TD ALIGN=left>{y[{x&nbsp;</TD><TD ALIGN=center>%s&nbsp;</TD><TD ALIGN=right>{y]{x&nbsp;</TD><TD ALIGN=left> %s%s%s%s%s%s%s%s %s%s %s</TD></TR>\n",
	       cla,
	       IN_BATTLE(wch)||IN_WAITING(wch)?"{r[BATTLE]{x":"",
	       IS_SET(wch->comm, COMM_AFK) ? "{G[AFK]{x " : "",
	       IS_SET(wch->comm, COMM_BUILDING) ? "{b[BUILDING]{x " : "",
  // SinaC 2003, same as COMM_BUILDING but editing datas
	       IS_SET(wch->comm, COMM_EDITING) ? "{B[EDITING]{x " : "",
	       wch->incog_level >= LEVEL_HERO ? "{Y(Incog){x " : "",
	       wch->invis_level >= LEVEL_HERO ? "{W(Wizi){x " : "",
	       IS_SET(wch->act, PLR_KILLER) ? "{R(KILLER){x " : "",
	       IS_SET(wch->act, PLR_THIEF)  ? "{R(THIEF){x "  : "",
	       wch->name,
	       wch->pcdata==NULL? "" : wch->pcdata->title,
	       get_clan_table(wch->clan)->who_name);
    else {
      char race[10];
      if ( race_table[wch->cstat(race)].pc_race )
	strcpy( race, pc_race_table[wch->cstat(race)].who_name ); // PC race
      else {
	strncpy( race, race_table[wch->cstat(race)].name, 5 ); // NPC race
	race[5] = '\0';
      }
      sprintf( buf, "<TR><TD ALIGN=left>{y[{x&nbsp;</TD><TD ALIGN=center>%3d&nbsp;%6s&nbsp;%s&nbsp;</TD><TD ALIGN=right>{y]{x&nbsp;</TD><TD ALIGN=left> %s%s%s%s%s%s%s%s%s %s%s %s</TD></TR>\n",
	       wch->level,
	       race,
	       cla,
	       IN_BATTLE(wch)||IN_WAITING(wch)?"{r[BATTLE]{x":"",
	       wch->pcdata->name_accepted?"":"{c[NEWBIE]{x",
	       IS_SET(wch->comm, COMM_AFK) ? "{G[AFK]{x " : "",
	       IS_SET(wch->comm, COMM_BUILDING) ? "{b[BUILDING]{x " : "",
  // SinaC 2003, same as COMM_BUILDING but editing datas
	       IS_SET(wch->comm, COMM_EDITING) ? "{B[EDITING]{x " : "",
	       wch->incog_level >= LEVEL_HERO ? "{Y(Incog){x " : "",
	       wch->invis_level >= LEVEL_HERO ? "{W(Wizi){x " : "",
	       IS_SET(wch->act, PLR_KILLER) ? "{R(KILLER){x " : "",
	       IS_SET(wch->act, PLR_THIEF)  ? "{R(THIEF){x "  : "",
	       wch->name,
	       wch->pcdata==NULL ? "" : wch->pcdata->title,
	       get_clan_table(wch->clan)->who_name);
    }
    // dump buf in html
    fprintf( fp, convert_color_to_html(buf) );
  }

  fprintf( fp, "</TABLE><BR>\n");

  if ( max_players < nMatch )
    max_players = nMatch;
  fprintf( fp, "Players found: %d - Max today: %d\n<BR>", nMatch, max_players );

  generate_ender( fp );
  
  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}
