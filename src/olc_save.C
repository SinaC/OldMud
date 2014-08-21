/**************************************************************************
 *  File: olc_save.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/* OLC_SAVE.C
 * This takes care of saving all the .are information.
 * Notes:
 * -If a good syntax checker is used for setting vnum ranges of areas
 *  then it would become possible to just cycle through vnums instead
 *  of using the iHash stuff and checking that the room or reset or
 *  mob etc is part of that area.
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "tables.h"

// Added by SinaC 2001
#include "db.h"
#include "handler.h"
#include "special.h"
#include "clan.h"
#include "comm.h"
#include "olc_value.h"
#include "names.h"
#include "interp.h"
#include "dbdata.h"
#include "config.h"
#include "bit.h"
#include "faction.h"
#include "save.h"


//#define AREADEBUG
#ifdef AREADEBUG
#define fprintf(fp,parm...) fprintf(fp,parm); fflush(fp);
#endif


#define DIF(a,b) (~((~a)|(b)))

#define NEW_DIF(a,b) ((a)^(b))

void new_save_area( AREA_DATA *pArea );
void new_save_mobiles( FILE *fp, AREA_DATA *pArea );
void new_save_objects( FILE *fp, AREA_DATA *pArea );
void new_save_rooms( FILE *fp, AREA_DATA *pArea );


/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */

/* #define VERBOSE */

/*****************************************************************************
 Name:		fix_string
 Purpose:	Returns a string without \r and ~.
 ****************************************************************************/
char *fix_string( const char *str ) {
  static char strfix[MAX_STRING_LENGTH * 2];
  int i;
  int o;
  
  if ( str == NULL )
    return '\0';
  
  for ( o = i = 0; str[i+o] != '\0'; i++ ) {
    if (str[i+o] == '\r' || str[i+o] == '~')
      o++;
    // modified by SinaC 2000, no  if...break   before
    //        strfix[i] = str[i+o];
    if ((strfix[i] = str[i+o]) == '\0')
      break;
  }
  strfix[i] = '\0';
  return strfix;
}


// general save affects (oldstyle)
void save_affects( FILE *fp, AFFECT_DATA *pAf ) {
  for ( AFFECT_LIST *laf = pAf->list; laf != NULL; laf = laf->next ) {
    //      if (pAf->where==AFTO_CHAR) {
    //	fprintf( fp, "Y\n%s ",
    //		 flag_string(afto_type,pAf->where));
    //	fprintf( fp, "'%s' %d %ld\n",
    //		 flag_string(attr_flags,pAf->location),
    //		 pAf->op,pAf->modifier);
    //      }
    //      else
    //	fprintf( fp, "Y\n%s %d %d %ld\n",
    //		 flag_string(afto_type,pAf->where),
    //		 pAf->location,pAf->op,pAf->modifier);
    if ( laf->where==AFTO_CHAR ) {
      fprintf( fp, "Y\n%s ",
	       flag_string(afto_type,laf->where));
      fprintf( fp, "'%s' %d %ld\n",
	       flag_string(attr_flags,laf->location),
	       laf->op, laf->modifier);
    }
    else if ( laf->where == AFTO_ROOM ) {
      fprintf( fp, "Y\n%s ", flag_string(afto_type,laf->where) );
      fprintf( fp, "'%s' %d %ld\n",
	       flag_string( room_attr_flags, laf->location ), 
	       laf->op, laf->modifier );
    }
    else
      fprintf( fp, "Y\n%s %d %d %ld\n",
	       flag_string(afto_type,laf->where),
	       laf->location, laf->op, laf->modifier);
  }
}

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
****************************************************************************/
void save_area_list() {
  FILE *fp;
  AREA_DATA *pArea;

  log_string("Saving area list" );

  if ( ( fp = fopen( AREA_LIST, "w" ) ) == NULL ) {
    bug( "Save_area_list: fopen");
    perror( "area.lst" );
  }
  else {
    /*
     * Add any help files that need to be loaded at
     * startup to this section.
     */
    // Those files are not in area.lst anymore, SinaC 2003
    //    fprintf( fp, "help.hlp\n"   );
    //    fprintf( fp, "social.are\n" );    /* ROM OLC */
    //    fprintf( fp, "rom.hlp\n"    );    /* ROM OLC */
    //    fprintf( fp, "group.hlp\n"  );    /* ROM OLC */
    //    fprintf( fp, "olc.hlp\n"    );
    //    fprintf( fp, "clans.hlp\n"  );
    //    fprintf( fp, "race.hlp\n"   );
    //    fprintf( fp, "class.hlp\n"  );
    //    fprintf( fp, "skills.hlp\n" );
    //    fprintf( fp, "spells.hlp\n" );
    //    fprintf( fp, "powers.hlp\n" );
    //    fprintf( fp, "songs.hlp\n" );
    //    fprintf( fp, "imm.hlp\n"    );

    for( pArea = area_first; pArea; pArea = pArea->next )
      fprintf( fp, "%s\n", pArea->file_name );
    fprintf( fp, "$\n" );
    fclose( fp );
  }

  return;
}


/*
 * ROM OLC
 * Used in save_mobile and save_object below.  Writes
 * flags on the form fread_flag reads.
 * 
 * buf[] must hold at least 32+1 characters.
 *
 * -- Hugin
 */
char *fwrite_flag( long flags, char buf[] ) {
  //char offset;
  long offset;
  char *cp;

  //buf[0] = '\0';
  strcpy( buf, "\0" );

  if ( flags == 0 ) {
    strcpy( buf, "0" );
    return buf;
  }

  /* 32 -- number of bits in a long */

  for ( offset = 0, cp = buf; offset < 32; offset++ )
    if ( flags & ( 1 << offset ) ) {
      if ( offset <= 'Z' - 'A' )
	*(cp++) = 'A' + offset;
      else
	*(cp++) = 'a' + offset - ( 'Z' - 'A' + 1 );
    }

  *cp = '\0';

  return buf;
}




/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
****************************************************************************/
void save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex ) {
  char buf[MAX_STRING_LENGTH];
  long temp;
  int race = pMobIndex->race;

  fprintf( fp, "#%d\n",         pMobIndex->vnum );
  fprintf( fp, "%s~\n",         pMobIndex->player_name );
  fprintf( fp, "%s~\n",         pMobIndex->short_descr );
  fprintf( fp, "%s~\n",         fix_string( pMobIndex->long_descr ) );
  fprintf( fp, "%s~\n",         fix_string( pMobIndex->description) );
  fprintf( fp, "%s~\n",         race_table[race].name );
  // Added by SinaC 2000 for mobile classes
  fprintf( fp, "%s\n",          fwrite_flag( pMobIndex->classes,     buf ) );

  fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->act,         buf ) );
  fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->affected_by, buf ) );
  // Added by SinaC 2001
  fprintf( fp, "%s ",	          fwrite_flag( pMobIndex->affected2_by, buf ) );
  // Modified by SinaC 2000 for alignment/etho
  fprintf( fp, "%d %d %d\n",        
	   pMobIndex->align.etho,
	   pMobIndex->align.alignment,
	   pMobIndex->group);
  fprintf( fp, "%d ",	          pMobIndex->level );
  fprintf( fp, "%d ",	          pMobIndex->hitroll );
  fprintf( fp, "%dd%d+%d ",     pMobIndex->hit[DICE_NUMBER], 
	   pMobIndex->hit[DICE_TYPE], 
	   pMobIndex->hit[DICE_BONUS] );
  fprintf( fp, "%dd%d+%d ",     pMobIndex->mana[DICE_NUMBER], 
	   pMobIndex->mana[DICE_TYPE], 
	   pMobIndex->mana[DICE_BONUS] );
  //Added by SinaC 2001 for mental user
  fprintf( fp, "%dd%d+%d ",     pMobIndex->psp[DICE_NUMBER],
	   pMobIndex->psp[DICE_TYPE], 
	   pMobIndex->psp[DICE_BONUS] );

  fprintf( fp, "%dd%d+%d ",     pMobIndex->damage[DICE_NUMBER], 
	   pMobIndex->damage[DICE_TYPE], 
	   pMobIndex->damage[DICE_BONUS] );
  fprintf( fp, "%s\n",          attack_table[pMobIndex->dam_type].name );
  fprintf( fp, "%d %d %d %d\n", pMobIndex->ac[AC_PIERCE] / 10, 
	   pMobIndex->ac[AC_BASH]   / 10, 
	   pMobIndex->ac[AC_SLASH]  / 10, 
	   pMobIndex->ac[AC_EXOTIC] / 10 );
  fprintf( fp, "%s ",           fwrite_flag( pMobIndex->off_flags,  buf ) );
  fprintf( fp, "%s ",	        fwrite_flag( pMobIndex->imm_flags,  buf ) );
  fprintf( fp, "%s ",           fwrite_flag( pMobIndex->res_flags,  buf ) );
  fprintf( fp, "%s\n",          fwrite_flag( pMobIndex->vuln_flags, buf ) );
  fprintf( fp, "%s %s %s %ld\n",
	   position_table[pMobIndex->start_pos].short_name,
	   position_table[pMobIndex->default_pos].short_name,
	   sex_table[pMobIndex->sex].name,
	   pMobIndex->wealth );
  fprintf( fp, "%s ",           fwrite_flag( pMobIndex->form,  buf ) );
  fprintf( fp, "%s ",      	  fwrite_flag( pMobIndex->parts, buf ) );

  fprintf( fp, "%s ",           size_table[pMobIndex->size].name );
  fprintf( fp, "%s\n" , ((pMobIndex->material[0] != '\0') ? pMobIndex->material : "unknown") );

  if ((temp = DIF(race_table[race].act,pMobIndex->act)))
    fprintf( fp, "F act %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].aff,pMobIndex->affected_by)))
    fprintf( fp, "F aff %s\n", fwrite_flag(temp, buf) );

  // Added by SinaC 2001
  if ((temp = DIF(race_table[race].aff2,pMobIndex->affected2_by)))
    fprintf( fp, "F aff2 %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].off,pMobIndex->off_flags)))
    fprintf( fp, "F off %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].imm,pMobIndex->imm_flags)))
    fprintf( fp, "F imm %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].res,pMobIndex->res_flags)))
    fprintf( fp, "F res %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].vuln,pMobIndex->vuln_flags)))
    fprintf( fp, "F vul %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].form,pMobIndex->form)))
    fprintf( fp, "F for %s\n", fwrite_flag(temp, buf) );

  if ((temp = DIF(race_table[race].parts,pMobIndex->parts)))
    fprintf( fp, "F par %s\n", fwrite_flag(temp, buf) );

  if (pMobIndex->program) 
    fprintf( fp, "M\n%s\n",pMobIndex->program->name);

  for( AFFECT_DATA *pAf = pMobIndex->affected; pAf; pAf = pAf->next )
   save_affects( fp, pAf );

  // Added by SinaC 2001         Removed by SinaC 2003
  //if (pMobIndex->disease)
  //  fprintf( fp, "D\n%s\n",fwrite_flag(pMobIndex->disease,buf));
  return;
}

/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
****************************************************************************/
void save_mobiles( FILE *fp, AREA_DATA *pArea ) {
  int i;
  MOB_INDEX_DATA *pMob;

  fprintf( fp, "#MOBILES\n" );

  for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ ) {
    if ( (pMob = get_mob_index( i )) )
      save_mobile( fp, pMob );
  }

  fprintf( fp, "#0\n\n\n\n" );
  return;
}





/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
****************************************************************************/
void save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex ) {
  char letter;
  AFFECT_DATA *pAf;
  EXTRA_DESCR_DATA *pEd;
  char buf[MAX_STRING_LENGTH];
  // Added by SinaC 2000 for object restrictions
  RESTR_DATA *restr;
  // Added by SinaC 2000 for object skill/spell upgrade
  ABILITY_UPGRADE *upgr;

  fprintf( fp, "#%d\n",    pObjIndex->vnum );
  fprintf( fp, "%s~\n",    pObjIndex->name );
  fprintf( fp, "%s~\n",    pObjIndex->short_descr );
  fprintf( fp, "%s~\n",    fix_string( pObjIndex->description ) );
  // Modified by SinaC 2001 for material
  //fprintf( fp, "%s~\n",    pObjIndex->material );
  fprintf( fp, "%s~\n",     material_table[pObjIndex->material].name);

  fprintf( fp, "%s ",      item_name(pObjIndex->item_type));
  fprintf( fp, "%s ",      fwrite_flag( pObjIndex->extra_flags, buf ) );
  fprintf( fp, "%s\n",     fwrite_flag( pObjIndex->wear_flags,  buf ) );

  /*
   *  Using fwrite_flag to write most values gives a strange
   *  looking area file, consider making a case for each
   *  item type later.
   */

  switch ( pObjIndex->item_type ) {
  default:
    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[0], buf ) );
    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[1], buf ) );
    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[2], buf ) );
    fprintf( fp, "%s ",  fwrite_flag( pObjIndex->value[3], buf ) );
    fprintf( fp, "%s\n", fwrite_flag( pObjIndex->value[4], buf ) );
    break;
    // SinaC 2003, component
  case ITEM_COMPONENT: {
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_INPUT_LENGTH];
    char buf4[MAX_INPUT_LENGTH];
    if ( pObjIndex->value[0] == NO_FLAG ) buf[0] = '\0';
    else sprintf( buf, "%s", flag_string(brew_component_flags, pObjIndex->value[0]) );
    if ( pObjIndex->value[1] == NO_FLAG ) buf1[0] = '\0';
    else sprintf( buf1, "%s", flag_string(brew_component_flags, pObjIndex->value[1]) );
    if ( pObjIndex->value[2] == NO_FLAG ) buf2[0] = '\0';
    else sprintf( buf2, "%s", flag_string(brew_component_flags, pObjIndex->value[2]) );
    if ( pObjIndex->value[3] == NO_FLAG ) buf3[0] = '\0';
    else sprintf( buf3, "%s", flag_string(brew_component_flags, pObjIndex->value[3]) );
    if ( pObjIndex->value[4] == NO_FLAG ) buf4[0] = '\0';
    else sprintf( buf4, "%s", flag_string(brew_component_flags, pObjIndex->value[4]) );
    fprintf( fp, "'%s' '%s' '%s' '%s' '%s'\n",
	     buf, buf1, buf2, buf3, buf4 );
    break;
  }
    // Added by SinaC 2000 for window
  case ITEM_WINDOW:
    fprintf( fp, "%d 0 0 0 0\n",
	     pObjIndex->value[0] );
    break;
    /* Removed by SinaC 2003, can be emulate with script
    // Added by SinaC 2000 for grenade
    case ITEM_GRENADE:
    fprintf( fp,"%d 0 %d 0 0\n", 
    pObjIndex->value[0], pObjIndex->value[2]);
    break;
    */
    /* Removed by SinaC 2003
    // Added by SinaC 2000 for throwing items
    case ITEM_THROWING:
    fprintf( fp, "%d %d %s %d '%s'\n",
    pObjIndex->value[0],
    pObjIndex->value[1],
    attack_table[pObjIndex->value[2]].name,
    pObjIndex->value[3],
    //skill_table[pObjIndex->value[4]].name
    pObjIndex->value[4] > 0 ?
    skill_table[pObjIndex->value[4]].name
    : ""
    );
    break;
    */
  case ITEM_LIGHT:
    fprintf( fp, "0 0 %d 0 0\n",
	     pObjIndex->value[2] < 1 ? 999  /* infinite */
	     : pObjIndex->value[2] );
    break;

  case ITEM_MONEY:
    fprintf( fp, "%d %d 0 0 0\n",
	     pObjIndex->value[0],
	     pObjIndex->value[1]);
    break;
            
  case ITEM_DRINK_CON:
    fprintf( fp, "%d %d '%s' %d 0\n",
	     pObjIndex->value[0],
	     pObjIndex->value[1],
	     liq_table[pObjIndex->value[2]].liq_name,
	     /*                   fwrite_flag( pObjIndex->value[3], buf ) ); */
	     pObjIndex->value[3]);
    break;
                    
  case ITEM_FOUNTAIN:
    fprintf( fp, "%d %d '%s' 0 0\n",
	     pObjIndex->value[0],
	     pObjIndex->value[1],
	     liq_table[pObjIndex->value[2]].liq_name);
    break;
	    
  case ITEM_CONTAINER:
    fprintf( fp, "%d %s %d %d %d\n",
	     pObjIndex->value[0],
	     fwrite_flag( pObjIndex->value[1], buf ),
	     pObjIndex->value[2],
	     pObjIndex->value[3],
	     pObjIndex->value[4]);
    break;
            
  case ITEM_FOOD:
    fprintf( fp, "%d %d 0 %s 0\n",
	     pObjIndex->value[0],
	     pObjIndex->value[1],
	     fwrite_flag( pObjIndex->value[3], buf ) );
    break;
            
  case ITEM_PORTAL:
    fprintf( fp, "%d %s %s %d 0\n",
	     pObjIndex->value[0],
	     fwrite_flag( pObjIndex->value[1], buf ),
	     fwrite_flag( pObjIndex->value[2], buf ),
	     pObjIndex->value[3]);
    break;
            
  case ITEM_FURNITURE:
    fprintf( fp, "%d %d %s %d %d\n",
	     pObjIndex->value[0],
	     pObjIndex->value[1],
	     fwrite_flag( pObjIndex->value[2], buf),
	     pObjIndex->value[3],
	     pObjIndex->value[4]);
    break;
            
  case ITEM_WEAPON:
    if ( pObjIndex->value[0] == WEAPON_RANGED )
      fprintf( fp, "%s %d %d %d %d\n",
	       weapon_name(pObjIndex->value[0]),
	       pObjIndex->value[1],
	       pObjIndex->value[2],
	       pObjIndex->value[3],
	       pObjIndex->value[4] );
    else
      fprintf( fp, "%s %d %d %s %s\n",
	       weapon_name(pObjIndex->value[0]),
	       pObjIndex->value[1],
	       pObjIndex->value[2],
	       attack_table[pObjIndex->value[3]].name,
	       fwrite_flag( pObjIndex->value[4], buf ) );
    break;
            
  case ITEM_ARMOR:
    fprintf( fp, "%d %d %d %d %d\n",
	     pObjIndex->value[0],
	     pObjIndex->value[1],
	     pObjIndex->value[2],
	     pObjIndex->value[3],
	     pObjIndex->value[4]);
    break;
            
  case ITEM_PILL:
  case ITEM_POTION:
  case ITEM_SCROLL:
    // Added by SinaC 2003
  case ITEM_TEMPLATE:  
    fprintf( fp, "%d '%s' '%s' '%s' '%s'\n",
	     pObjIndex->value[0] > 0 ? /* no negative numbers */
	     pObjIndex->value[0]
	     : 0,
	     //pObjIndex->value[1] != -1 ?
	     pObjIndex->value[1] > 0 ?
	     ability_table[pObjIndex->value[1]].name
	     : "",
	     //pObjIndex->value[2] != -1 ?
	     pObjIndex->value[2] > 0 ?
	     ability_table[pObjIndex->value[2]].name
	     : "",
	     //pObjIndex->value[3] != -1 ?
	     pObjIndex->value[3] > 0 ?
	     ability_table[pObjIndex->value[3]].name
	     : "",
	     //pObjIndex->value[4] != -1 ?
	     pObjIndex->value[4] > 0 ?
	     ability_table[pObjIndex->value[4]].name
	     : "");
    break;

  case ITEM_STAFF:
  case ITEM_WAND:
    fprintf( fp, "%d ", pObjIndex->value[0] );
    fprintf( fp, "%d ", pObjIndex->value[1] );
    fprintf( fp, "%d '%s' 0\n",
	     pObjIndex->value[2],
	     /*
	       pObjIndex->value[3] != -1 ?
	       ability_table[pObjIndex->value[3]].name
	       : 0 
	     */
	     pObjIndex->value[3] > 0 ?
	     ability_table[pObjIndex->value[3]].name
	     : ""
	     );
    break;
  }

  fprintf( fp, "%d ", pObjIndex->level );
  fprintf( fp, "%d ", pObjIndex->weight );
  fprintf( fp, "%d ", pObjIndex->cost );

  if ( pObjIndex->condition >  90 ) letter = 'P';
  else if ( pObjIndex->condition >  75 ) letter = 'G';
  else if ( pObjIndex->condition >  50 ) letter = 'A';
  else if ( pObjIndex->condition >  25 ) letter = 'W';
  else if ( pObjIndex->condition >  10 ) letter = 'D';
  else if ( pObjIndex->condition >   0 ) letter = 'B';
  else                                   letter = 'R';
  fprintf( fp, "%c\n", letter );

  if ( pObjIndex->size != SIZE_NOSIZE )
    fprintf( fp, "S\n '%s'\n", flag_string(size_flags,pObjIndex->size) );


  // Modified by SinaC 2000 for NOT restriction
  // Added by SinaC 2000 for object restrictions
  for (restr = pObjIndex->restriction; restr; restr = restr->next ){
    //    if ( !restr->ability_r ) {
    if ( restr->type != RESTR_ABILITY ) {
      // Modified by SinaC 2001
      fprintf(fp,
	      "R\n'%s' %d %d\n",
	      flag_string(restr_flags,restr->type),
	      restr->value,
	      restr->not_r );
    }
    // Added by SinaC 2000 for skill/spell restrictions
    else
      fprintf(fp,
	      "W\n'%s' %d %d\n",
	      ability_table[restr->sn].name,
	      restr->value,
	      restr->not_r );
  }
  // End of object restrictions

  // Added by SinaC 2000 for object skill/spell upgrade
  for (upgr = pObjIndex->upgrade; upgr; upgr = upgr->next ){
    fprintf(fp,
	    "Z\n'%s' %d\n",
	    ability_table[upgr->sn].name,
	    upgr->value );
  }

  for( pAf = pObjIndex->affected; pAf; pAf = pAf->next )
    save_affects( fp, pAf );

  for( pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next ) {
    fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
	     fix_string( pEd->description ) );
  }

  if (pObjIndex->program) 
    fprintf( fp, "M\n%s\n",pObjIndex->program->name);

  return;
}
 



/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
****************************************************************************/
void save_objects( FILE *fp, AREA_DATA *pArea ) {
  int i;
  OBJ_INDEX_DATA *pObj;
  
  fprintf( fp, "#OBJECTS\n" );

  for( i = pArea->min_vnum; i <= pArea->max_vnum; i++ ) {
    if ( (pObj = get_obj_index( i )) )
      save_object( fp, pObj );
  }

  fprintf( fp, "#0\n\n\n\n" );
  return;
}
 




/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
****************************************************************************/
void save_rooms( FILE *fp, AREA_DATA *pArea ) {
  ROOM_INDEX_DATA *pRoomIndex;
  EXTRA_DESCR_DATA *pEd;
  EXIT_DATA *pExit;
  int iHash;
  int door;
  char buf[MAX_STRING_LENGTH];
  
  fprintf( fp, "#ROOMS\n" );
  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next ) {
      if ( pRoomIndex->area == pArea ) {
	fprintf( fp, "#%d\n",		pRoomIndex->vnum );
	fprintf( fp, "%s~\n",		pRoomIndex->name );
	fprintf( fp, "%s~\n",		fix_string( pRoomIndex->description ) );
	fprintf( fp, "0 " );
	// Modified by SinaC 2001
	fprintf( fp, "%d ",		pRoomIndex->bstat(flags) );
	fprintf( fp, "%d\n",		pRoomIndex->bstat(sector));
	// Added by SinaC 2001 for maximal size
	fprintf( fp, "%d\n",		pRoomIndex->bstat(maxsize) );

	for ( pEd = pRoomIndex->extra_descr; pEd;
	      pEd = pEd->next ) {
	  fprintf( fp, "E\n%s~\n%s~\n", pEd->keyword,
		   fix_string( pEd->description ) );
	}
	for( door = 0; door < MAX_DIR; door++ )	{/* I hate this! */
	  if ( ( pExit = pRoomIndex->exit[door] )
	       && !IS_SET(pExit->rs_flags, EX_NOSAVING )// Added by SinaC 2003 
	       && !IS_SET(pExit->exit_info, EX_NOSAVING )// Added by SinaC 2003 
	       && pExit->u1.to_room ) {
	    /* Removed by SinaC 2001
	       int locks = 0;
	       if ( IS_SET( pExit->rs_flags, EX_ISDOOR ) 
	       && ( !IS_SET( pExit->rs_flags, EX_PICKPROOF ) ) 
	       && ( !IS_SET( pExit->rs_flags, EX_NOPASS ) ) )
	       locks = 1;
	       if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
	       && ( IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
	       && ( !IS_SET( pExit->rs_flags, EX_NOPASS ) ) )
	       locks = 2;
	       if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
	       && ( !IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
	       && ( IS_SET( pExit->rs_flags, EX_NOPASS ) ) )
	       locks = 3;
	       if ( IS_SET( pExit->rs_flags, EX_ISDOOR )
	       && ( IS_SET( pExit->rs_flags, EX_PICKPROOF ) )
	       && ( IS_SET( pExit->rs_flags, EX_NOPASS ) ) )
	       locks = 4;
	    */
	    
	    fprintf( fp, "D%d\n",      pExit->orig_door );
	    fprintf( fp, "%s~\n",      fix_string( pExit->description ) );
	    fprintf( fp, "%s~\n",      pExit->keyword );
	    /* Modified by SinaC 2001
	       fprintf( fp, "%d %d %d\n", locks,
	       pExit->key,
	       pExit->u1.to_room->vnum );
	    */
	    fprintf( fp, "%s %d %d\n", 
		     fwrite_flag( pExit->rs_flags, buf ),
		     pExit->key,
		     pExit->u1.to_room->vnum );
	  }
	}
	//Modifed by SinaC 2001 for mental user
	/*
	  if (pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100)
	  fprintf ( fp, "M %d H %d\n",pRoomIndex->mana_rate,
	  pRoomIndex->heal_rate);
	*/
	// Modified by SinaC 2001
	if (pRoomIndex->bstat(manarate) != 100 
	    || pRoomIndex->bstat(healrate) != 100 
	    || pRoomIndex->bstat(psprate) != 100 )
	  fprintf ( fp, "M %d H %d P %d\n",
		    pRoomIndex->bstat(manarate),
		    pRoomIndex->bstat(healrate),
		    pRoomIndex->bstat(psprate));

	if (pRoomIndex->clan > 0)
	  fprintf ( fp, "C %s~\n" , get_clan_table(pRoomIndex->clan)->name );
		 			     
	if (pRoomIndex->owner && str_cmp(pRoomIndex->owner,""))
	  fprintf ( fp, "O %s~\n" , pRoomIndex->owner );

	if (pRoomIndex->guild) { /* Oxtal */
	  char buf[33];
	  fprintf ( fp, "G %s\n", fwrite_flag( pRoomIndex->guild, buf) );
	}

	// Added by SinaC 2003 for room program
	if (pRoomIndex->program) 
	  fprintf( fp, "Z\n%s\n", pRoomIndex->program->name);

	// Added by SinaC 2003 for repop time
	if ( pRoomIndex->time_between_repop != BASE_REPOP_TIME 
	     || pRoomIndex->time_between_repop_people != BASE_REPOP_TIME_PEOPLE )
	  fprintf( fp, "R %d %d\n",
		   pRoomIndex->time_between_repop, pRoomIndex->time_between_repop_people );

	for( AFFECT_DATA *pAf = pRoomIndex->base_affected; pAf; pAf = pAf->next )
	  save_affects( fp, pAf );
	
	fprintf( fp, "S\n" );
      }
    }
  }
  fprintf( fp, "#0\n\n\n\n" );
  return;
}



/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
****************************************************************************/
void save_specials( FILE *fp, AREA_DATA *pArea ) {
  int iHash;
  MOB_INDEX_DATA *pMobIndex;
    
  fprintf( fp, "#SPECIALS\n" );

  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next ) {
      if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->spec_fun ) {
#if defined( VERBOSE )
	fprintf( fp, "M %d %s Load to: %s\n", pMobIndex->vnum,
		 spec_name( pMobIndex->spec_fun ),
		 pMobIndex->short_descr );
#else
	fprintf( fp, "M %d %s\n", pMobIndex->vnum,
		 spec_name( pMobIndex->spec_fun ) );
#endif
      }
    }
  }

  fprintf( fp, "S\n\n\n\n" );
  return;
}



/*
 * This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin.
 */
void save_door_resets( FILE *fp, AREA_DATA *pArea ) {
  int iHash;
  ROOM_INDEX_DATA *pRoomIndex;
  EXIT_DATA *pExit;
  int door;

  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next ) {
      if ( pRoomIndex->area == pArea ) {
	for( door = 0; door < MAX_DIR; door++ ) {
	  if ( ( pExit = pRoomIndex->exit[door] )
	       && pExit->u1.to_room 
	       && ( IS_SET( pExit->rs_flags, EX_CLOSED )
		    || IS_SET( pExit->rs_flags, EX_LOCKED ) ) )
#if defined( VERBOSE )
	    fprintf( fp, "D 0 %d %d %d The %s door of %s is %s\n", 
		     pRoomIndex->vnum,
		     pExit->orig_door,
		     IS_SET( pExit->rs_flags, EX_LOCKED) ? 2 : 1,
		     dir_name[ pExit->orig_door ],
		     pRoomIndex->name,
		     IS_SET( pExit->rs_flags, EX_LOCKED) ? "closed and locked"
		     : "closed" );
#endif
#if !defined( VERBOSE )
	  fprintf( fp, "D 0 %d %d %d\n", 
		   pRoomIndex->vnum,
		   pExit->orig_door,
		   IS_SET( pExit->rs_flags, EX_LOCKED) ? 2 : 1 );
#endif
	}
      }
    }
  }
  return;
}




/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
****************************************************************************/
void save_resets( FILE *fp, AREA_DATA *pArea ) {
  RESET_DATA *pReset;
  MOB_INDEX_DATA *pLastMob = NULL;
  OBJ_INDEX_DATA *pLastObj;
  ROOM_INDEX_DATA *pRoom;
  int iHash;
  
  fprintf( fp, "#RESETS\n" );

  save_door_resets( fp, pArea );

  for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
    for( pRoom = room_index_hash[iHash]; pRoom; pRoom = pRoom->next ) {
      if ( pRoom->area == pArea ) {
	for ( pReset = pRoom->reset_first; pReset; pReset = pReset->next ) {
	  switch ( pReset->command ) {
	  default:
	    bug( "Save_resets: bad command %c.", pReset->command );
	    break;

#if defined( VERBOSE )
	  case 'M':
            pLastMob = get_mob_index( pReset->arg1 );
	    fprintf( fp, "M 0 %d %d %d %d Load %s\n", 
		     pReset->arg1,
		     pReset->arg2,
		     pReset->arg3,
		     pReset->arg4,
		     pLastMob->short_descr );
            break;
	    
	  case 'O':
            pLastObj = get_obj_index( pReset->arg1 );
            pRoom = get_room_index( pReset->arg3 );
	    fprintf( fp, "O 0 %d 0 %d %s loaded to %s\n", 
		     pReset->arg1,
		     pReset->arg3,
		     capitalize(pLastObj->short_descr),
		     pRoom->name );
            break;

	  case 'P':
            pLastObj = get_obj_index( pReset->arg1 );
	    fprintf( fp, "P 0 %d %d %d %d %s put inside %s\n", 
		     pReset->arg1,
		     pReset->arg2,
		     pReset->arg3,
		     pReset->arg4,
		     capitalize(get_obj_index( pReset->arg1 )->short_descr),
		     pLastObj->short_descr );
            break;

	  case 'G':
	    fprintf( fp, "G 0 %d 0 %s is given to %s\n",
		     pReset->arg1,
		     capitalize(get_obj_index( pReset->arg1 )->short_descr),
		     pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
            if ( !pLastMob )
	      bug("Save_resets: !NO_MOB! in [%s]", pArea->file_name );
            break;

	  case 'E':
	    fprintf( fp, "E 0 %d 0 %d %s is loaded %s of %s\n",
		     pReset->arg1,
		     pReset->arg3,
		     capitalize(get_obj_index( pReset->arg1 )->short_descr),
		     flag_string( wear_loc_strings, pReset->arg3 ),
		     pLastMob ? pLastMob->short_descr : "!NO_MOB!" );
            if ( !pLastMob )
	      bug( "Save_resets: !NO_MOB! in [%s]", pArea->file_name );
            break;

	  case 'D':
            break;

	  case 'R':
	    pRoom = get_room_index( pReset->arg1 );
	    fprintf( fp, "R 0 %d %d Randomize %s\n",
		     pReset->arg1,
		     pReset->arg2,
		     pRoom->name );
	    break;
	    // Added by SinaC and JyP ( aka Oxtal ) 2000 for random maze
	  case 'Z':
	    fprintf( fp, "Z 0 %d %d %d %d Maze\n",
		     pReset->arg1,
		     pReset->arg2,
		     pReset->arg3,
		     pReset->arg4);
	    break;
	  }
#endif
#if !defined( VERBOSE )
	case 'M':
	  pLastMob = get_mob_index( pReset->arg1 );
	  fprintf( fp, "M 0 %d %d %d %d\n", 
		   pReset->arg1,
		   pReset->arg2,
		   pReset->arg3,
		   pReset->arg4 );
	  break;

	case 'O':
	  pLastObj = get_obj_index( pReset->arg1 );
	  pRoom = get_room_index( pReset->arg3 );
	  fprintf( fp, "O 0 %d 0 %d\n", 
		   pReset->arg1,
		   pReset->arg3 );
	  break;
	  
	case 'P':
	  pLastObj = get_obj_index( pReset->arg1 );
	  fprintf( fp, "P 0 %d %d %d %d\n", 
		   pReset->arg1,
		   pReset->arg2,
		   pReset->arg3,
		   pReset->arg4 );
	  break;

	case 'G':
	  fprintf( fp, "G 0 %d 0\n", pReset->arg1 );
	  if ( !pLastMob )
	    bug("Save_resets: !NO_MOB! in [%s]", pArea->file_name );
	  break;

	case 'E':
	  fprintf( fp, "E 0 %d 0 %d\n",
		   pReset->arg1,
		   pReset->arg3 );
	  if ( !pLastMob )
	    bug("Save_resets: !NO_MOB! in [%s]", pArea->file_name );
	  break;

	case 'D':
	  break;

	case 'R':
	  pRoom = get_room_index( pReset->arg1 );
	  fprintf( fp, "R 0 %d %d\n", 
		   pReset->arg1,
		   pReset->arg2 );
	  break;
	  // Added by SinaC and JyP ( aka Oxtal ) 2000 for random maze
	case 'Z':
	  fprintf( fp, "Z 0 %d %d %d %d\n",
		   pReset->arg1,
		   pReset->arg2,
		   pReset->arg3,
		   pReset->arg4);
	  break;
	  
	}
#endif
      }
    }	/* End if correct area */
  }	/* End for pRoom */
}	/* End for iHash */
fprintf( fp, "S\n\n\n\n" );
return;
}

/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
****************************************************************************/
void save_shops( FILE *fp, AREA_DATA *pArea ) {
SHOP_DATA *pShopIndex;
MOB_INDEX_DATA *pMobIndex;
int iTrade;
int iHash;
  
fprintf( fp, "#SHOPS\n" );

for( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
for( pMobIndex = mob_index_hash[iHash]; pMobIndex; pMobIndex = pMobIndex->next ) {
if ( pMobIndex && pMobIndex->area == pArea && pMobIndex->pShop ) {
pShopIndex = pMobIndex->pShop;

fprintf( fp, "%d ", pShopIndex->keeper );
for ( iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) {
if ( pShopIndex->buy_type[iTrade] != 0 )
{
fprintf( fp, "%d ", pShopIndex->buy_type[iTrade] );
}
else
fprintf( fp, "0 ");
}
fprintf( fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell );
fprintf( fp, "%d %d\n", pShopIndex->open_hour, pShopIndex->close_hour );
}
}
}

fprintf( fp, "0\n\n\n\n" );
return;
}



/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
****************************************************************************/
void save_area( AREA_DATA *pArea ) {
FILE *fp;
char buf[MAX_STRING_LENGTH];

log_stringf(" Saving area: %s (fname: %s)",pArea->name,pArea->file_name);

char fileName[MAX_STRING_LENGTH];
sprintf( fileName, "%s%s", AREA_DIR, pArea->file_name );

fclose( fpReserve );
if ( !( fp = fopen( fileName, "w" ) ) )
{
  bug( "save_area: can't open file %s", fileName );
  //perror( pArea->file_name );
  return;
}
 
// Added by SinaC 2000 to save the area_flags
// Little trick to avoid saving flag we don't want to see in the AREA FILE
 REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
 REMOVE_BIT( pArea->area_flags, AREA_ADDED );
 REMOVE_BIT( pArea->area_flags, AREA_LOADING );

 fprintf( fp, "#AREADATA\n" );
 fprintf( fp, "Name %s~\n",        pArea->name );
 fprintf( fp, "Builders %s~\n",        fix_string( pArea->builders ) );
 fprintf( fp, "VNUMs %d %d\n",      pArea->min_vnum, pArea->max_vnum );
 fprintf( fp, "Credits %s~\n",	 pArea->credits );
 fprintf( fp, "Security %d\n",         pArea->security );
 // Added by SinaC 2000/2001 to save the area_flags and teleport room
 fprintf( fp, "Flags %s\n", fwrite_flag( pArea->area_flags, buf ) );
 // Removed by SinaC 2003, scripts can do that
 //fprintf( fp, "Teleport %d\n", pArea->teleport_room );
 fprintf( fp, "End\n\n\n\n" );
 save_mobiles( fp, pArea );
 save_objects( fp, pArea );
 save_rooms( fp, pArea );
 save_specials( fp, pArea );
 save_resets( fp, pArea );
 save_shops( fp, pArea );

 fprintf( fp, "#$\n" );

 fclose( fp );
 fpReserve = fopen( NULL_FILE, "r" );
 return;
}


/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
****************************************************************************/
void do_asave( CHAR_DATA *ch, const char *argument ) {
  char arg1 [MAX_INPUT_LENGTH];
  AREA_DATA *pArea;
  FILE *fp;
  int value;

  fp = NULL;

  if ( !ch ) {      /* Do an autosave */
    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next )
      if (IS_SET( pArea->area_flags, AREA_CHANGED ) ) { 
	save_area( pArea );
	REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
      }
    return;
  }
  //  smash_tilde( argument );
  strcpy( arg1, argument );
  if ( arg1[0] == '\0' )
    {
      send_to_char( "Syntax:\n\r", ch );
      send_to_char( "  asave <vnum>   - saves a particular area\n\r",	ch );
      send_to_char( "  asave list     - saves the area.lst file\n\r",	ch );
      send_to_char( "  asave area     - saves the area being edited\n\r",	ch );
      send_to_char( "  asave changed  - saves all changed zones\n\r",	ch );
      send_to_char( "  asave world    - saves the world! (db dump)\n\r",	ch );
      send_to_char( "  {RNEVER use asave world if you wanna keep ya lvl{x\n\r", ch );
      return;
    }

  /* Snarf the value (which need not be numeric). */
  value = atoi( arg1 );
  if ( !( pArea = get_area_data( value ) ) && is_number( arg1 ) ) {
    send_to_char( "That area does not exist.\n\r", ch );
    return;
  }
  /* Save area of given vnum. */
  /* ------------------------ */

  if ( is_number( arg1 ) ) {
    if ( !IS_BUILDER( ch, pArea ) ) {
      send_to_char( "You are not a builder for this area.\n\r", ch );
      return;
    }
    save_area_list();
    save_area( pArea );
    send_to_charf(ch,"Area %s saved.\n\r",pArea->name);
    return;
  }
  /* Save the world, only authorized areas. */
  /* -------------------------------------- */

  if ( !str_cmp( "world", arg1 ) ) {
      
    if ( ch->level != IMPLEMENTOR ) {
      send_to_char("You are not high enough to save the world!\n\r", ch );
      return;
    }
      
    sprintf(log_buf,"Log %s: ASAVE WORLD",ch->name);
    log_string(log_buf);

    save_area_list();
    for( pArea = area_first; pArea; pArea = pArea->next ) {
      /* Builder must be assigned this area. */
      if ( !IS_BUILDER( ch, pArea ) )
	continue;	  

      new_save_area( pArea );
      REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    }
    send_to_char( "You saved the world.\n\r", ch );
    /*	send_to_all_char( "Database saved.\n\r" );                 ROM OLC */
    return;
  }

  /* Save changed areas, only authorized areas. */
  /* ------------------------------------------ */

  if ( !str_cmp( "changed", arg1 ) ) {
    char buf[MAX_INPUT_LENGTH];

    save_area_list();

    send_to_char( "Saved zones:\n\r", ch );
    sprintf( buf, "None.\n\r" );

    for( pArea = area_first; pArea; pArea = pArea->next ) {
      /* Builder must be assigned this area. */
      if ( !IS_BUILDER( ch, pArea ) )
	continue;

      /* Save changed areas. */
      if ( IS_SET(pArea->area_flags, AREA_CHANGED) ) {
	save_area( pArea );
	sprintf( buf, "%24s - '%s'\n\r", pArea->name, pArea->file_name );
	send_to_char( buf, ch );
	REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
      }
    }
    if ( !str_cmp( buf, "None.\n\r" ) )
      send_to_char( buf, ch );
    return;
  }

  /* Save the area.lst file. */
  /* ----------------------- */
  if ( !str_cmp( arg1, "list" ) ) {
    save_area_list();
    send_to_char("Area list saved.\n\r",ch);
    return;
  }

  /* Save area being edited, if authorized. */
  /* -------------------------------------- */
  if ( !str_cmp( arg1, "area" ) ) {
    /* Is character currently editing. */
    if ( ch->desc->editor == 0 ) {
      send_to_char( "You are not editing an area, "
		    "therefore an area vnum is required.\n\r", ch );
      return;
    }
	
    /* Find the area to save. */
    switch (ch->desc->editor) {
    case ED_AREA:
      pArea = (AREA_DATA *)ch->desc->pEdit;
      break;
    case ED_ROOM:
      pArea = ch->in_room->area;
      break;
    case ED_OBJECT:
      pArea = ( (OBJ_INDEX_DATA *)ch->desc->pEdit )->area;
      break;
    case ED_MOBILE:
      pArea = ( (MOB_INDEX_DATA *)ch->desc->pEdit )->area;
      break;
    default:
      pArea = ch->in_room->area;
      break;
    }

    if ( !IS_BUILDER( ch, pArea ) ) {
      send_to_char( "You are not a builder for this area.\n\r", ch );
      return;
    }

    save_area_list();
    save_area( pArea );
    REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
    send_to_charf( ch, "Area %s saved.\n\r", pArea->name );
    return;
  }

  /* Show correct syntax. */
  /* -------------------- */
  do_asave( ch, "" );
  return;
}


//***********************************************************************************************************
//******************************************  Area  save  ***************************************************


void fseparator( FILE *fp ) {
  fprintf( fp, "\n//=======================================\n");
}

void new_save_extra_descr( FILE *fp, EXTRA_DESCR_DATA *pEd ) {
  fprintf( fp, "  ExtraDescr = ( %s, %s);\n", 
	   quotify(pEd->keyword), quotify(fix_string( pEd->description )) );
}

void new_save_area( AREA_DATA *pArea ) {
  FILE *fp;
  char fileName[MAX_STRING_LENGTH];

  sprintf( fileName, "%snewformat/%s", AREA_DIR, pArea->file_name );

  log_stringf("Saving area: %s (fname: %s)",pArea->name, fileName);

  fclose( fpReserve );
  if ( !( fp = fopen( fileName, "w" ) ) ) {
    bug( "new_save_area: can't open file %s", fileName );
    //perror( fileName );
    return;
  }

  // Little trick to avoid saving flag we don't want to see in the AREA FILE
  REMOVE_BIT( pArea->area_flags, AREA_CHANGED );
  REMOVE_BIT( pArea->area_flags, AREA_ADDED );
  REMOVE_BIT( pArea->area_flags, AREA_LOADING );

  fprintf( fp, 
	   "Area %s {\n"
	   "  Builders = %s;\n"
	   "  Vnums = ( %d, %d );\n"
	   "  Credits = %s;\n"
	   "  Security = %d;\n",
	   quotify(pArea->name), quotify(fix_string( pArea->builders )),
	   pArea->min_vnum, pArea->max_vnum,
	   quotify(pArea->credits), pArea->security );
  if ( pArea->area_flags != 0 )
    fprintf( fp,
	     "  Flags = (%s);\n",
	     list_flag_string( pArea->area_flags, area_flags ) );

  fseparator(fp);

  new_save_mobiles( fp, pArea ); fseparator(fp);
  new_save_objects( fp, pArea ); fseparator(fp);
  new_save_rooms( fp, pArea ); fseparator(fp);
  //new_save_specials( fp, pArea ); specials are saved with mobiles
  //new_save_resets( fp, pArea ); resets are saved with rooms
  //new_save_shops( fp, pArea ); shops are saved with mobiles

  fprintf( fp, "}\n\n");

  fclose( fp );
  fpReserve = fopen( NULL_FILE, "r" );
}

void new_save_room_resets( FILE *fp, ROOM_INDEX_DATA *pRoom ) { // called from  new_save_rooms
  MOB_INDEX_DATA *pLastMob = NULL;
  OBJ_INDEX_DATA *pLastObj = NULL;
  for ( RESET_DATA *pReset = pRoom->reset_first; pReset; pReset = pReset->next )
    switch ( pReset->command ) {
    default:
      bug( "new_save_room_resets: bad command %c.", pReset->command );
      break;

    case 'M': // mobile in room
      pLastMob = get_mob_index( pReset->arg1 );
      fprintf( fp, 
	       "  Reset = ( 'M', %d, %d, %d );\n",
	       pReset->arg1, pReset->arg2, pReset->arg4 );
      break;
      
    case 'O': // item in room
      pLastObj = get_obj_index( pReset->arg1 );
      fprintf( fp, 
	       "  Reset = ( 'O', %d );\n",
	       pReset->arg1 );
      break;
	  
    case 'P': // item in container
      pLastObj = get_obj_index( pReset->arg1 );
      fprintf( fp, 
	       "  Reset = ( 'P', %d, %d, %d, %d );\n",
	       pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4 );
	  break;
	  
    case 'G': // item on mobile in room  (Give)
      fprintf( fp, 
	       "  Reset = ( 'G', %d, %d );\n",
	       pReset->arg1, pReset->arg2 );
      if ( !pLastMob )
	bug("new_save_resets: !NO_MOB!" );
      break;
      
    case 'E': // item on mobile in room (Equip)
      fprintf( fp, 
	       "  Reset = ( 'E', %d, %d, %s );\n",
	       pReset->arg1, pReset->arg2, quotify(flag_string( wear_loc_flags, pReset->arg3 )) );
      if ( !pLastMob )
	bug("new_save_resets: !NO_MOB!" );
      break;
      
    case 'D': // not used anymore
      break;
      
    case 'R': // randomize exits
      pRoom = get_room_index( pReset->arg1 );
      fprintf( fp, 
	       "  Reset = ( 'R', %d, %d );\n",
	       pReset->arg1, pReset->arg2 );
      break;

    case 'Z': // random maze
      fprintf( fp, 
	       "  Reset = ( 'Z', %d, %d, %d );\n",
	       pReset->arg1, pReset->arg2, pReset->arg4);
      break;
      
    }
  fprintf( fp, "\n" );
}

void new_save_room( FILE *fp, ROOM_INDEX_DATA *pRoomIndex ) {
  fprintf( fp,
	      "Room %d {\n"
	      "  Name = %s;\n"
	      "  Desc = %s;\n"
	      "  Sector = %s;\n",
	      pRoomIndex->vnum, quotify(pRoomIndex->name), 
	      quotify(fix_string( pRoomIndex->description )), 
	      quotify(flag_string( sector_flags, pRoomIndex->bstat(sector) ) ) );
  if ( pRoomIndex->bstat(flags) != 0 )
    fprintf( fp,"  Flags = (%s);\n",
		list_flag_string( pRoomIndex->bstat(flags), room_flags ) );
  if ( pRoomIndex->bstat(maxsize) != SIZE_NOSIZE )
    fprintf( fp, "  MaxSize = %s;\n", 
		quotify(flag_string(size_flags,pRoomIndex->bstat(maxsize) ) ) );
  if ( pRoomIndex->bstat(manarate) != 100 
       || pRoomIndex->bstat(healrate) != 100 
       || pRoomIndex->bstat(psprate) != 100 )
    fprintf ( fp, "  Rate = ( %d, %d, %d );\n",
		 pRoomIndex->bstat(manarate), pRoomIndex->bstat(healrate), pRoomIndex->bstat(psprate));
  if ( pRoomIndex->clan > 0 )
    fprintf ( fp, "  Clan = %s;\n" , 
		 quotify(get_clan_table(pRoomIndex->clan)->name) );
  if ( pRoomIndex->owner && str_cmp(pRoomIndex->owner,"") )
    fprintf ( fp, "  Owner = %s;\n" , 
		 quotify(pRoomIndex->owner) );
  if ( pRoomIndex->guild )
    fprintf ( fp, "  Guilds = (%s);\n", 
		 list_flag_string( pRoomIndex->guild, classes_flags ) );
  if ( pRoomIndex->program ) 
    fprintf( fp, "  Program = %s;\n", 
		quotify(pRoomIndex->program->name));
  if ( pRoomIndex->time_between_repop != BASE_REPOP_TIME 
       || pRoomIndex->time_between_repop_people != BASE_REPOP_TIME_PEOPLE )
    fprintf( fp, "  Repop = ( %d, %d);\n",
		pRoomIndex->time_between_repop, pRoomIndex->time_between_repop_people );
  
  for ( EXTRA_DESCR_DATA *pEd = pRoomIndex->extra_descr; pEd;
	pEd = pEd->next )
    new_save_extra_descr( fp, pEd );
  
  for( int door = 0; door < MAX_DIR; door++ ) {
    EXIT_DATA *pExit;
    if ( ( pExit = pRoomIndex->exit[door] )
	 && !IS_SET(pExit->rs_flags, EX_NOSAVING )
	 && !IS_SET(pExit->exit_info, EX_NOSAVING )
	 && pExit->u1.to_room )
      fprintf( fp, "  Exit = ( %d, %s, %s, (%s), %d, %d );\n",
		  pExit->orig_door, quotify(fix_string( pExit->description )),
		  quotify(pExit->keyword), list_flag_string( pExit->rs_flags, exit_flags ),
		  pExit->key, pExit->u1.to_room->vnum );
  }
  
  new_save_room_resets( fp, pRoomIndex );
  
  fprintf( fp, "}\n");
}

void new_save_rooms( FILE *fp, AREA_DATA *pArea ) {
  for( int iHash = 0; iHash < MAX_KEY_HASH; iHash++ )
    for( ROOM_INDEX_DATA *pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next )
      if ( pRoomIndex->area == pArea )
	new_save_room( fp, pRoomIndex );
  fprintf( fp, "\n");
}

void new_save_object( FILE *fp, OBJ_INDEX_DATA *pObjIndex ) {
  fprintf( fp,
	   "Object %d {\n"
	   "  Name = %s;\n"
	   "  ShD = %s;\n"
	   "  Desc = %s;\n"
	   "  ItemType = %s;\n"
	   "  Level = %d;\n"
	   "  Weight = %d;\n"
	   "  Cost = %d;\n",
	   pObjIndex->vnum, quotify(pObjIndex->name), quotify(pObjIndex->short_descr),
	   quotify(fix_string( pObjIndex->description)), 
	   quotify(flag_string( type_flags, pObjIndex->item_type )),
	   pObjIndex->level, pObjIndex->weight, pObjIndex->cost );

  if ( pObjIndex->material != 0 )
    fprintf( fp, "  Material = %s;\n",	   
	     quotify(flag_string( material_flags, pObjIndex->material )) );
  if ( pObjIndex->program ) 
    fprintf( fp, "  Program = %s;\n",
	     quotify(pObjIndex->program->name) );
  if ( pObjIndex->condition != 100 )
    fprintf( fp, "  Condition = %d;\n",
	     pObjIndex->condition );
  if ( pObjIndex->size != SIZE_NOSIZE )
    fprintf( fp, "  Size = %s;\n",
	     quotify(flag_string(size_flags,pObjIndex->size)) );
  if ( pObjIndex->extra_flags != 0 )
    fprintf( fp, "  ExtraFlags = (%s);\n",
	     list_flag_string( pObjIndex->extra_flags, extra_flags ) );
  if ( pObjIndex->wear_flags != 0 )
    fprintf( fp, "  WearFlags = (%s);\n",
	     list_flag_string( pObjIndex->wear_flags, wear_flags ) );

//  Values = ( <STRING/INTEGER value0>, .. <STRING/INTEGER value4> );
  switch ( pObjIndex->item_type ) {
  default:
    fprintf( fp, "  Values = ( %d, %d, %d, %d, %d );\n",
	     pObjIndex->value[0], pObjIndex->value[1], pObjIndex->value[2],
	     pObjIndex->value[3], pObjIndex->value[4] );
    break;
  case ITEM_COMPONENT: { // SinaC 2003
    char buf[MAX_INPUT_LENGTH];
    char buf1[MAX_INPUT_LENGTH];
    char buf2[MAX_INPUT_LENGTH];
    char buf3[MAX_INPUT_LENGTH];
    char buf4[MAX_INPUT_LENGTH];
    if ( pObjIndex->value[0] == NO_FLAG ) sprintf( buf, "'none'" );
    else sprintf( buf, "%s", quotify(flag_string(brew_component_flags, pObjIndex->value[0])) );
    if ( pObjIndex->value[1] == NO_FLAG ) sprintf( buf1, "'none'" );
    else sprintf( buf1, "%s", quotify(flag_string(brew_component_flags, pObjIndex->value[1])) );
    if ( pObjIndex->value[2] == NO_FLAG ) sprintf( buf2, "'none'" );
    else sprintf( buf2, "%s", quotify(flag_string(brew_component_flags, pObjIndex->value[2])) );
    if ( pObjIndex->value[3] == NO_FLAG ) sprintf( buf3, "'none'" );
    else sprintf( buf3, "%s", quotify(flag_string(brew_component_flags, pObjIndex->value[3])) );
    if ( pObjIndex->value[4] == NO_FLAG ) sprintf( buf4, "'none'" );
    else sprintf( buf4, "%s", quotify(flag_string(brew_component_flags, pObjIndex->value[4])) );
    fprintf( fp, "  Values = ( %s, %s, %s, %s, %s );\n",
	     buf, buf1, buf2, buf3, buf4 );
    break;
  }
  case ITEM_WINDOW:
    fprintf( fp, "  Values = ( %d );\n", pObjIndex->value[0] );
    break;
  case ITEM_LIGHT: // 999: infinite
    fprintf( fp, "  Values = ( %d );\n", pObjIndex->value[2] < 1 ? 999 : pObjIndex->value[2] );
    break;
  case ITEM_MONEY:
    fprintf( fp, "  Values = ( %d, %d );\n", pObjIndex->value[0], pObjIndex->value[1]);
    break;
  case ITEM_DRINK_CON:
    fprintf( fp, "  Values = ( %d, %d, %s, %s );\n",
	     pObjIndex->value[0], pObjIndex->value[1],
	     quotify( liq_table[pObjIndex->value[2]].liq_name ), pObjIndex->value[3]?"true":"false" );
    break;
  case ITEM_FOUNTAIN:
    fprintf( fp, "  Values = ( %d, %d, %s );\n",
	     pObjIndex->value[0], pObjIndex->value[1],
	     quotify( liq_table[pObjIndex->value[2]].liq_name ) );
    break;
  case ITEM_CONTAINER:
    fprintf( fp, "  Values = ( %d, (%s), %d, %d, %d );\n",
	     pObjIndex->value[0], list_flag_string( pObjIndex->value[1], container_flags ),
	     pObjIndex->value[2], pObjIndex->value[3], pObjIndex->value[4]);
    break;
  case ITEM_FOOD:
    fprintf( fp, "  Values = ( %d, %d, %s );\n",
	     pObjIndex->value[0], pObjIndex->value[1],
	     pObjIndex->value[3]?"true":"false" );
    break;
  case ITEM_PORTAL:
    fprintf( fp, "  Values = ( %d, (%s), (%s), %d );\n",
	     pObjIndex->value[0], list_flag_string( pObjIndex->value[1], exit_flags ),
	     list_flag_string( pObjIndex->value[2], portal_flags ), pObjIndex->value[3] );
    break;
  case ITEM_FURNITURE:
    fprintf( fp, "  Values = ( %d, %d, (%s), %d, %d );\n",
	     pObjIndex->value[0], pObjIndex->value[1],
	     list_flag_string( pObjIndex->value[2], furniture_flags ),
	     pObjIndex->value[3], pObjIndex->value[4] );
    break;
  case ITEM_WEAPON:
    if ( pObjIndex->value[0] == WEAPON_RANGED )
      fprintf( fp, "  Values = ( %s, %d, %d, %d, %d );\n",
	       quotify(flag_string(weapon_class,pObjIndex->value[0])),
	       pObjIndex->value[1], pObjIndex->value[2],
	       pObjIndex->value[3], pObjIndex->value[4] );
    else
      fprintf( fp, "  Values = ( %s, %d, %d, %s, (%s) );\n",
	       quotify(flag_string(weapon_class,pObjIndex->value[0])),
	       pObjIndex->value[1], pObjIndex->value[2],
	       quotify(attack_table[pObjIndex->value[3]].name),
	       list_flag_string( pObjIndex->value[4], weapon_type2 ) );
    break;
  case ITEM_ARMOR:
    fprintf( fp, "  Values = ( %d, %d, %d, %d, %d );\n",
	     pObjIndex->value[0], pObjIndex->value[1], pObjIndex->value[2],
	     pObjIndex->value[3], pObjIndex->value[4] );
    break;
  case ITEM_PILL:
  case ITEM_POTION:
  case ITEM_SCROLL:
  case ITEM_TEMPLATE:  
    fprintf( fp, "  Values = ( %d, %s, %s, %s, %s );\n",
	     UMAX( pObjIndex->value[0], 0 ),
	     pObjIndex->value[1] > 0 ? quotify( ability_table[pObjIndex->value[1]].name ) : "'none'",
	     pObjIndex->value[2] > 0 ? quotify( ability_table[pObjIndex->value[2]].name ) : "'none'",
	     pObjIndex->value[3] > 0 ? quotify( ability_table[pObjIndex->value[3]].name ) : "'none'",
	     pObjIndex->value[4] > 0 ? quotify( ability_table[pObjIndex->value[4]].name ) : "'none'" );
    break;
  case ITEM_STAFF:
  case ITEM_WAND:
    fprintf( fp, "  Values = ( %d, %d, %d, %s );\n",
	     pObjIndex->value[0], pObjIndex->value[1], pObjIndex->value[2], 
	     pObjIndex->value[3] > 0 ? quotify( ability_table[pObjIndex->value[3]].name ) : "'none'" );
    break;
  }

  for ( RESTR_DATA *restr = pObjIndex->restriction; restr; restr = restr->next )
    //if ( !restr->ability_r )
    if ( restr->type != RESTR_ABILITY )
      fprintf( fp, "  Restriction = ( %d, %s, %d, %d );\n",
	       0, quotify(flag_string(restr_flags,restr->type)),
	       restr->value, restr->not_r );
    else
      fprintf( fp, "  Restriction = ( %d, %s, %d, %d );\n",
	       1, quotify(ability_table[restr->sn].name),
	       restr->value, restr->not_r );

  // Added by SinaC 2000 for object skill/spell upgrade
  for ( ABILITY_UPGRADE *upgr = pObjIndex->upgrade; upgr; upgr = upgr->next )
    fprintf( fp, "Upgrade = ( %s, %d );\n",
	     quotify(ability_table[upgr->sn].name), upgr->value );

  //for ( AFFECT_DATA *pAf = pObjIndex->affected; pAf; pAf = pAf->next )
  //  new_save_affects( pAf, fp );
  new_save_affects( pObjIndex->affected, fp );

  for ( EXTRA_DESCR_DATA *pEd = pObjIndex->extra_descr; pEd; pEd = pEd->next )
    new_save_extra_descr( fp, pEd );

  fprintf( fp, "}\n");
}

void new_save_objects( FILE *fp, AREA_DATA *pArea ) {
  for( int i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    if ( OBJ_INDEX_DATA *pObj = get_obj_index( i ) )
      new_save_object( fp, pObj );
  fprintf( fp, "\n");
}

void new_save_mobile( FILE *fp, MOB_INDEX_DATA *pMobIndex ) {
  fprintf( fp,
	   "Mobile %d {\n"
	   "  Name = %s;\n"
	   "  ShD = %s;\n"
	   "  LnD = %s;\n"
	   "  Desc = %s;\n"
	   "  Race = %s;\n"
	   "  Alignment = %d;\n"
	   "  Etho = %s;\n"
	   "  Level = %d;\n"
	   "  Hitroll = %d;\n"
	   "  Hit = ( %d, %d, %d );\n"
	   "  Mana = ( %d, %d, %d );\n"
	   "  Psp = ( %d, %d, %d );\n"
	   "  Damage = ( %d, %d, %d );\n"
	   "  ACs = ( %d, %d, %d, %d );\n"
	   "  Position = ( %s, %s );\n"
	   "  Sex = %s;\n"
	   "  Wealth = %ld;\n"
	   "  Size = %s;\n"
	   "  Faction = %s;\n",
	   pMobIndex->vnum, quotify(pMobIndex->player_name), quotify(pMobIndex->short_descr),
	   quotify(fix_string( pMobIndex->long_descr )), quotify(fix_string( pMobIndex->description)),
	   quotify(flag_string( races_flags, pMobIndex->race) ),
	   pMobIndex->align.alignment, quotify( flag_string( etho_flags, pMobIndex->align.etho ) ),
	   pMobIndex->level, pMobIndex->hitroll,
	   pMobIndex->hit[DICE_NUMBER], pMobIndex->hit[DICE_TYPE], pMobIndex->hit[DICE_BONUS],
	   pMobIndex->mana[DICE_NUMBER], pMobIndex->mana[DICE_TYPE], pMobIndex->mana[DICE_BONUS],
	   pMobIndex->psp[DICE_NUMBER], pMobIndex->psp[DICE_TYPE], pMobIndex->psp[DICE_BONUS],
	   pMobIndex->damage[DICE_NUMBER], pMobIndex->damage[DICE_TYPE], pMobIndex->damage[DICE_BONUS],
	   pMobIndex->ac[AC_PIERCE]/10, pMobIndex->ac[AC_BASH]/10, pMobIndex->ac[AC_SLASH]/10, pMobIndex->ac[AC_EXOTIC]/10,
	   quotify(flag_string( position_flags, pMobIndex->start_pos )),
	   quotify(flag_string( position_flags, pMobIndex->default_pos )),
	   quotify(flag_string( sex_flags, pMobIndex->sex )),
	   pMobIndex->wealth, quotify(flag_string(size_flags, pMobIndex->size) ),
	   quotify( faction_table[pMobIndex->faction].name ) );
  if ( pMobIndex->dam_type != 0 )
    fprintf( fp, "  DamType = %s;\n", quotify(attack_table[pMobIndex->dam_type].name) );
  if ( pMobIndex->material[0] != 0 
       && str_cmp(pMobIndex->material, "0" )
       && str_cmp(pMobIndex->material, "unknown" ) )
    fprintf( fp, "  Material = %s;\n", quotify( pMobIndex->material ) );
  if ( pMobIndex->classes != 0 )
    fprintf( fp, "  Classes = (%s);\n", list_flag_string( pMobIndex->classes, classes_flags ) );
  if ( pMobIndex->program != NULL ) 
    fprintf( fp, "  Program = %s;\n", quotify(pMobIndex->program->name));
  if ( pMobIndex->group != 0 )
    fprintf( fp, "  Group = %d;\n", pMobIndex->group );

  // Following comments is not available anymore, NEW_DIF replaces with DIF, SinaC 2003
  // be careful when you modify a race
  //  because we have made the DIF between race and mob
  //  and will use this diff to load the mob too
  //  so if a race flag is removed/added, a mob with this race will not get the flag anymore
  int race = pMobIndex->race;
  int temp;
  if ( ( temp = DIF( race_table[race].act, pMobIndex->act ) ) )
    fprintf( fp, "  Act = (%s);\n", list_flag_string( temp, act_flags ) );
  if ( ( temp = DIF( race_table[race].aff, pMobIndex->affected_by ) ) )
    fprintf( fp, "  AfBy = (%s);\n", list_flag_string( temp, affect_flags ) );
  if ( ( temp = DIF( race_table[race].aff2, pMobIndex->affected2_by ) ) )
    fprintf( fp, "  AfBy2 = (%s);\n", list_flag_string( temp, affect2_flags ) );
  if ( ( temp = DIF( race_table[race].off, pMobIndex->off_flags ) ) )
    fprintf( fp, "  Offensive = (%s);\n", list_flag_string( temp, off_flags ) );
  if ( ( temp = DIF( race_table[race].imm, pMobIndex->imm_flags ) ) )
    fprintf( fp, "  Immunities = (%s);\n", list_flag_string( temp, irv_flags ) );
  if ( ( temp = DIF( race_table[race].res, pMobIndex->res_flags ) ) )
    fprintf( fp, "  Resistances = (%s);\n", list_flag_string( temp, irv_flags ) );
  if ( ( temp = DIF( race_table[race].vuln, pMobIndex->vuln_flags ) ) )
    fprintf( fp, "  Vulnerabilities = (%s);\n", list_flag_string( temp, irv_flags ) );
  if ( ( temp = DIF( race_table[race].form, pMobIndex->form ) ) )
    fprintf( fp, "  Forms = (%s);\n", list_flag_string( temp, form_flags ) );
  if ( ( temp = DIF( race_table[race].parts, pMobIndex->parts ) ) )
    fprintf( fp, "  Parts = (%s);\n", list_flag_string( temp, part_flags ) );

  if ( pMobIndex->spec_fun )
    fprintf( fp, "  Special = %s;\n", quotify(spec_name( pMobIndex->spec_fun )) );
  
  if ( pMobIndex->pShop ) { // shop
	SHOP_DATA *pShopIndex = pMobIndex->pShop;
	// count buy type list number of elements
	char itemList[MAX_STRING_LENGTH];
	int countItemList = 0;
	itemList[0] = '\0';
	for ( int i = 0; i < MAX_TRADE; i++ )
	  if ( pShopIndex->buy_type[i] != 0 )
	    countItemList++;
	// create string with  buy type list
	if ( countItemList > 0 ) {
	  int count = 0;
	  for ( int i = 0; i < MAX_TRADE; i++ )
	    if ( pShopIndex->buy_type[i] != 0 ) {
	      char buf[MAX_STRING_LENGTH];
	      strcat( itemList, quotify( flag_string( type_flags, pShopIndex->buy_type[i] ) ) );
	      if ( count < countItemList-1 )
		strcat( itemList, ", " );
	      count++;
	    }
	}
	
	fprintf( fp, "  Shop = ( (%s), %d, %d, %d, %d );\n",
		 itemList,
		 pShopIndex->profit_buy, pShopIndex->profit_sell,
		 pShopIndex->open_hour, pShopIndex->close_hour );
      }

  fprintf( fp, "}\n");
}

void new_save_mobiles( FILE *fp, AREA_DATA *pArea ) {
  for( int i = pArea->min_vnum; i <= pArea->max_vnum; i++ )
    if ( MOB_INDEX_DATA *pMob = get_mob_index( i ) )
      new_save_mobile( fp, pMob );
  fprintf( fp, "\n" );
}

