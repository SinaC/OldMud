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
//#include "olc.h"

// Added by SinaC 2001
#include "handler.h"
#include "db.h"
#include "olc_value.h"
#include "names.h"
#include "affects.h"
#include "bit.h"


// Added by SinaC 2003 for clean affect wearoff and update
void wearoff_null( AFFECT_DATA *af, void *vo, int target ) {
}

void update_null( AFFECT_DATA *af, void *vo, int target ) {
}

// Added by SinaC 2000, see db.C
void init_attr_table() {
  extern const struct attr_type *attr_table;
  static const struct attr_type attr_table_source[] =
    // Corresponding table in table.h */
  { {"sex"             ,sex_flags},
    {"classes"         ,classes_flags},
    {"race"            ,races_flags},      // modified by SinaC 2000
    {"hp"              ,NULL},
    {"mana"            ,NULL},
    {"moves"           ,NULL},

    // SinaC 2003, same tables
    //{"immunites"       ,imm_flags},
    //{"resisitances"    ,res_flags},
    //{"vulnerabilities" ,vuln_flags},

    {"immunites"       ,irv_flags},
    {"resistances"    ,irv_flags},
    {"vulnerabilities" ,irv_flags},

    {"affects"         ,affect_flags},
    {"saving throw"    ,NULL},
    {"hitroll"         ,NULL},
    {"damroll"         ,NULL},
    {"ac pierce"       ,NULL},
    {"ac bash"         ,NULL},
    {"ac slash"        ,NULL},
    {"ac magic"        ,NULL},
    {"strength"        ,NULL},
    {"intelligence"    ,NULL},
    {"wisdom"          ,NULL},
    {"dexterity"       ,NULL},
    {"constitution"    ,NULL},
    {"parts"           ,part_flags},
    {"form"            ,form_flags},
    {"size"            ,size_flags},
    {"dice number"     ,NULL},
    {"dice type"       ,NULL},
    {"dice bonus"      ,NULL},
    // Modified by SinaC 2001
    //{"dam type"        ,NULL},
    {"dam type"        ,dam_type_flags},
    {"affects2"        ,affect2_flags},
    // Added by SinaC 2001
    {"etho"            ,etho_flags},
    {"alignment"       ,NULL},
    // Added by SinaC 2001 for mental user
    {"psp"             ,NULL},
    // Added by SinaC 2001 for disease
    //{"disease"         ,disease_flags},   removed by SinaC 2003
    {"AC"              ,NULL},
    {"N/A"             ,NULL}
    // Added by SinaC 2001
  };

  attr_table = attr_table_source;
}

void update_attr_table( const char *name, flag_type *flag ) {
  int i;

  extern const struct attr_type *attr_table;
  struct attr_type *l_attr_table = (struct attr_type*) attr_table;

  for ( i = 0; l_attr_table[i].name != NULL; i++ )
    if ( !str_cmp( l_attr_table[i].name, name ) )
      break;
  if ( l_attr_table[i].name == NULL ) {
    bug("update_attr_table: Invalid attr table entry: [%s]", name );
    exit(-1);
  }
  l_attr_table[i].bits = flag;
}

// Initialized in db.C
const struct attr_type *attr_table;


// Added by SinaC 2000, see db.C
void init_room_attr_table()
{
  extern const struct attr_type *room_attr_table;
  static const struct attr_type room_attr_table_source[] =
    { {"flags"           ,room_flags},
      {"light"           ,NULL},
      {"sector"          ,sector_flags},
      {"healrate"        ,NULL},
      {"manarate"        ,NULL},
      {"psprate"         ,NULL},
      {"maxsize"         ,size_flags}
    };
  room_attr_table = room_attr_table_source;
}

// Added by SinaC 2001 for room affects, initialized in db.C
const struct attr_type *room_attr_table;


// Corresponding table in table.h */



const int locmap[] = {
/*OLD_APPLY_NONE                   */ ATTR_NA,
/*OLD_APPLY_STR                    */ ATTR_STR,
/*OLD_APPLY_DEX                    */ ATTR_DEX,
/*OLD_APPLY_INT                    */ ATTR_INT,
/*OLD_APPLY_WIS                    */ ATTR_WIS,
/*OLD_APPLY_CON                    */ ATTR_CON,
/*OLD_APPLY_SEX                    */ ATTR_sex,
/*OLD_APPLY_CLASS                  */ ATTR_classes,
/*OLD_APPLY_LEVEL                  */ ATTR_NA,
/*OLD_APPLY_AGE                    */ ATTR_NA,
/*OLD_APPLY_HEIGHT                 */ ATTR_NA,
/*OLD_APPLY_WEIGHT                 */ ATTR_NA,
/*OLD_APPLY_MANA                   */ ATTR_max_mana,
/*OLD_APPLY_HIT                    */ ATTR_max_hit,
/*OLD_APPLY_MOVE                   */ ATTR_max_move,
/*OLD_APPLY_GOLD                   */ ATTR_NA,
/*OLD_APPLY_EXP                    */ ATTR_NA,
/*OLD_APPLY_AC                     */ ATTR_allAC, /* special */
/*OLD_APPLY_HITROLL                */ ATTR_hitroll,
/*OLD_APPLY_DAMROLL                */ ATTR_damroll,
/*OLD_APPLY_SAVES                  */ ATTR_saving_throw,
/*OLD_APPLY_SAVING_ROD             */ ATTR_saving_throw,
/*OLD_APPLY_SAVING_PETRI           */ ATTR_saving_throw,
/*OLD_APPLY_SAVING_BREATH          */ ATTR_saving_throw,
/*OLD_APPLY_SAVING_SPELL           */ ATTR_saving_throw,
/*OLD_APPLY_SPELL_AFFECT           */ ATTR_NA
};

const char * op_table[] = {
  "",                 // AFOP_ADD
  " adding",          // AFOP_OR
  " setting them to", // AFOP_ASSIGN 
  " removing"         // AFOP_NOR
};

int locoldtonew(int oldloc) {
  return (oldloc <= OLD_APPLY_SPELL_AFFECT && oldloc >= 0 )
    ? locmap[oldloc] : ATTR_NA;
}

// SinaC 2003: new affect system and new affect string
void get_affect_modifier( char *buf, AFFECT_LIST *laf ) {
  char str1[MAX_STRING_LENGTH];
  buf[0] = '\0';
  switch ( laf->where ) {
  case AFTO_OBJVAL: // affect OBJECT value: not yet supported
    sprintf(str1,"{cmodifies {yobject value %d{c by%s {y%ld", laf->location, op_table[laf->op], laf->modifier );
    bug("Bad affect in AfStr (AFTO_OBJVAL)" );
    strcat( buf, str1 );
    break;
  case AFTO_OBJECT: // affect OBJECT extra flags
    sprintf(str1,"{cmodifies {yextra flag {cby%s {y%s", op_table[laf->op], extra_bit_name(laf->modifier));
    strcat( buf, str1 );
    break;
  case AFTO_WEAPON: // affect WEAPON flags
    sprintf(str1,"{cmodifies {yweapon flag {cby%s {y%s", op_table[laf->op], weapon_bit_name(laf->modifier));
    strcat( buf, str1 );
    break;
  case AFTO_ROOM: // affect ROOM attributes
    sprintf(str1,"{cmodifies {yroom %s {cby%s ", room_attr_table[laf->location].name, op_table[laf->op]);
    strcat(buf,str1);
    if (room_attr_table[laf->location].bits == NULL)
      sprintf(str1,"{y%ld",laf->modifier);
    else
      sprintf(str1,"{y%s",flag_string(room_attr_table[laf->location].bits,laf->modifier));
    strcat(buf,str1);
    break;
  case AFTO_CHAR: // affect CHAR attributes
    sprintf(str1,"{cmodifies {y%s {cby%s ",attr_table[laf->location].name,op_table[laf->op]);
    strcat(buf,str1);
    if (attr_table[laf->location].bits == NULL)
      sprintf(str1,"{y%ld",laf->modifier);
    else
      sprintf(str1,"{y%s",flag_string(attr_table[laf->location].bits,laf->modifier));
    strcat(buf,str1);
    break;
  }
}
void afstring( char *buf, AFFECT_DATA *af, CHAR_DATA *ch, const bool care_level ) {
  buf[0] = '\0';

  if ( IS_SET( af->flags, AFFECT_INVISIBLE ) && !IS_IMMORTAL(ch) ) {
    sprintf( buf, "{R%-20s{x\n\r", "something" );
    return;
  }

  // affect type
  char type[128];
  if ( IS_SET( af->flags, AFFECT_INHERENT ) ) // inherent affect
    sprintf( type, "inherent affect");
  else if ( IS_SET( af->flags, AFFECT_ABILITY ) ) // ability affect
    sprintf( type, "%s",ability_table[(int) af->type].name);
  else if ( af->type > 0 ) // ability affect without AFFECT_ABILITY bit
    sprintf( type, "[%s]",ability_table[(int) af->type].name);
  else  // unknown affect
    sprintf( type, "unknown");
  // affect level
  char level[128];
  sprintf(level,"(level %3d)",af->level);
  // affect casting level
  char cast[128];
  if ( af->casting_level != 0 ) // If affect has a casting level: show it
    sprintf(cast,"(lvl %2d)",af->casting_level);
  else
    cast[0] = '\0';
  // affect duration
  char duration[128];
  if (af->duration >= 0 && !IS_SET(af->flags,AFFECT_PERMANENT))
    if (af->duration == 0 )
      sprintf(duration,"{cless than one hour left" );
    else
      sprintf(duration,"{y%4d {chours left", af->duration );
  else if ( IS_SET(af->flags,AFFECT_PERMANENT) )
    sprintf(duration,"{r%4s Permanent","");
  else
    duration[0] = '\0';
  
  // if lvl < 10, only type and casting level
  if ( ch->level < 10 && care_level ) {
    sprintf( buf, "{b%-20s{g%-8s{x\n\r", type, cast );
    return;
  }
  // if lvl < 15, only type, casting level and duration
  if ( ch->level < 15 && care_level ) {
    sprintf( buf, "{b%-25s{g%-8s%-19s{x\n\r", type, cast, duration );
    return;
  }

  // if lvl >= 15, type, casting level, duration and affect_list
  if ( IS_IMMORTAL(ch) ) // Immortals see affect level
    sprintf( buf, "{b%-25s%-13s{g%-8s%-s%s%s%s{x\n\r",
	     type, level, cast, duration,
	     IS_SET( af->flags, AFFECT_NON_DISPELLABLE )?" {R**{x":"",
	     IS_SET( af->flags, AFFECT_INVISIBLE )?" {Y**{x":"",
	     IS_SET( af->flags, AFFECT_STAY_DEATH )?" {G**{x":""
	     );
  else
    sprintf( buf, "{b%-25s{g%-8s%-19s{x\n\r", type, cast, duration );

  for ( AFFECT_LIST *laf = af->list; laf != NULL; laf = laf->next ) {
    char modifier[MAX_STRING_LENGTH];
    get_affect_modifier(modifier,laf);
    strcat(buf,"      ");
    strcat(buf,modifier);
    strcat(buf,"\n\r{x");
  }
}
