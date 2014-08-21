/**************************************************************************
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
 **************************************************************************/

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
#include "recycle.h"
#include "tables.h"
#include "lookup.h"
////#define _XOPEN_SOURCE
////#include <unistd.h>
#include "classes.h"
#include "abilityupgrade.h"
#include "comm.h"
#include "db.h"
#include "handler.h"
#include "affects.h"
#include "act_comm.h"
#include "clan.h"
#include "fight.h"
#include "save.h"
#include "act_info.h"
#include "moons.h"
#include "interp.h"
#include "gsn.h"
#include "olc_value.h"
#include "condition.h"
#include "names.h"
#include "const.h"
#include "language.h"
#include "ability.h"
#include "group.h"
#include "bit.h"
#include "utils.h"
#include "dbscript.h"
#include "arena.h"
#include "weather.h"


// added by SinaC 2000
void    affects_pet( CHAR_DATA *ch, CHAR_DATA *pet );
// Added by SinaC 2000 for window code
void    look_window     args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );

// Added by SinaC 2001
/*
void    racehelp( CHAR_DATA *ch, int race );
void    classhelp( CHAR_DATA *ch, int classindex );
void    skillhelp( CHAR_DATA *ch, int sn );
*/

void    racehelp(int race, BUFFER *output );
void    classhelp(int classindex, BUFFER *output );
void    abilityhelp(int sn, BUFFER *output );
void    triggerHelp( const TriggerDescr *trigger, BUFFER *output ); // SinaC 2003
void    godHelp( const int godId, BUFFER *output );

static const char *	const	where_name [] = { // this table must be in the same order than WEAR_XXX in olc_value.h
  "{C<used as light>		{x",
  "{C<worn on left finger>	{x",
  "{C<worn on right finger>	{x",
  "{C<worn around neck>	{x",
  "{C<worn around neck>	{x",
  "{C<worn on torso>		{x",
  "{C<worn on head>		{x",
  "{C<worn on legs>		{x",
  "{C<worn on feet>		{x",
  "{C<worn on hands>		{x",
  "{C<worn on arms>		{x",
  "{C<worn as shield>        {x",
  "{C<worn about body>	{x",
  "{C<worn about waist>	{x",
  "{C<worn around wrist>	{x",
  "{C<worn around wrist>	{x",
  "{c<wielded>		{x",
  "{C<held>			{x",
  "{C<worn on left ear>	{x",
  "{C<worn on right ear>	{x",
  "{C<worn on the eyes>	{x",
  "{c<secondary weapon>	{x",
  "{C<floating nearby>	{x",
  //  "{B<brand mark>            {x"
  "{c<third weapon>          {x",
  "{c<fourth weapon>         {x",
};

// Used in do_equipment for a nicer output: from head to feet
static int mapping_for_nicer_output [] = { // same size as MAX_WEAR and above vector
  WEAR_LIGHT,
  WEAR_HEAD,
  WEAR_EYES,
  WEAR_EAR_L,
  WEAR_EAR_R,
  WEAR_NECK_1,
  WEAR_NECK_2,
  WEAR_ARMS,
  WEAR_HANDS,
  WEAR_WRIST_R,
  WEAR_FINGER_R,
  WEAR_WIELD,
  WEAR_WRIST_L,
  WEAR_FINGER_L,
  WEAR_SECONDARY,
  WEAR_THIRDLY,
  WEAR_FOURTHLY,
  WEAR_HOLD,
  WEAR_SHIELD,
  WEAR_BODY,
  WEAR_ABOUT,
  WEAR_WAIST,
  WEAR_LEGS,
  WEAR_FEET,
  WEAR_FLOAT,

  WEAR_NONE
};

/* for do_count */
int max_on = 0;

//bool help_flag = FALSE;


/*
 * Local functions.
 */
const char *	format_obj_to_char	args( ( OBJ_DATA *obj, CHAR_DATA *ch,
						bool fShort ) );
void	show_list_to_char	args( ( OBJ_DATA *list, CHAR_DATA *ch,
					bool fShort, bool fShowNothing ) );
void	show_char_to_char_0	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char_1	args( ( CHAR_DATA *victim, CHAR_DATA *ch ) );
void	show_char_to_char	args( ( CHAR_DATA *list, CHAR_DATA *ch ) );
bool	check_blind		args( ( CHAR_DATA *ch ) );


const char *format_obj_to_char( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort ) {
  static char buf[MAX_STRING_LENGTH];
  
  buf[0] = '\0';
  
  // modified by SinaC 2000
  if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0'))
      ||  (obj->description == NULL || obj->description[0] == '\0')) {
    // Added by SinaC 2000
    bug("format_obj_to_char: Item vnum: %d has no description!", obj->pIndexData->vnum);
    strcpy(buf,"This object has no description. Please inform the IMP.");
    return buf;
  }

  // Added by SinaC 2001
  if ( fShort ) {
    if ( !IS_SET(obj->extra_flags, ITEM_NOCOND) ) {
      char buf2[128];
      sprintf(buf2,"[%s] ", show_obj_cond(obj));
      strcat( buf, buf2 );
    }
    else
      strcat( buf, "[perfect] " );
  }

/*
({YGlowing{x)
({yHumming{x)
({DVampiric{x)  ({DVile{x)
({YSparking{x)
({CFrost{x)     ({CMisting{x)
({GEnvenomed{x)   ({GPoisonned{x) ({GVenomed{x)
({RFlaming{x)
({CHoly{x)
({yHeavy{x)
({BSharp{x) ({BVicous{x)
*/

  // Added by SinaC 2000
  if ( obj->item_type == ITEM_WEAPON 
       && (IS_AFFECTED( ch, AFF_DETECT_MAGIC )
	   || IS_SET(ch->act, PLR_HOLYLIGHT))) {
    if (IS_WEAPON_STAT( obj, WEAPON_FLAMING))  strcat( buf, "{R(Flaming){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_FROST))    strcat( buf, "{C(Frost){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_VAMPIRIC)) strcat( buf, "{D(Vampiric){x ");
    //if (IS_WEAPON_STAT( obj, WEAPON_SHARP))    strcat( buf, "{r(Sharp){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_SHARP))    strcat( buf, "{B(Sharp){x ");
    //if (IS_WEAPON_STAT( obj, WEAPON_VORPAL))   strcat( buf, "{m(Vorpal){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_VORPAL))   strcat( buf, "{B(Vorpal){x ");
    //if (IS_WEAPON_STAT( obj, WEAPON_SHOCKING)) strcat( buf, "{y(Sparkling){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_SHOCKING)) strcat( buf, "{Y(Sparkling){x ");
    //if (IS_WEAPON_STAT( obj, WEAPON_POISON))   strcat( buf, "{g(Envenomed){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_POISON))   strcat( buf, "{G(Envenomed){x ");
    // Added by SinaC 2001
    if (IS_WEAPON_STAT( obj, WEAPON_HOLY))     strcat( buf, "{C(Holy){x ");
    // Added by SinaC 2001 for weighted mace
    //if (IS_WEAPON_STAT( obj, WEAPON_WEIGHTED)) strcat( buf, "{M(Weighted){x ");
    if (IS_WEAPON_STAT( obj, WEAPON_WEIGHTED)) strcat( buf, "{W(Weighted){x ");
    // Added by SinaC 2003
    if (IS_WEAPON_STAT( obj, WEAPON_NECROTISM))    strcat( buf, "{D(Necrotic){x ");
  }
  
  if ( IS_OBJ_STAT(obj, ITEM_INVIS)
       && (IS_AFFECTED(ch,AFF_DETECT_INVIS)
	   || IS_SET(ch->act, PLR_HOLYLIGHT))) strcat( buf, "{y(Invis){x "   );
  // Modified by SinaC 2000
  if ( IS_AFFECTED(ch, AFF_DETECT_EVIL)
       && IS_OBJ_STAT(obj, ITEM_EVIL)   )   strcat( buf, "{R(Evil){x "  );
    if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
  //    && IS_OBJ_STAT(obj,ITEM_BLESS))	    strcat(buf,  "{B(Blessed){x " );
	&& IS_OBJ_STAT(obj,ITEM_BLESS))	    strcat(buf,  "{C(Blessed){x " );

  if ( (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
	|| IS_SET(ch->act, PLR_HOLYLIGHT))
       && IS_OBJ_STAT(obj, ITEM_MAGIC)  )   strcat( buf, "{b(Magical){x "   );
  //if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "{W(Glowing){x "   );
  if ( IS_OBJ_STAT(obj, ITEM_GLOW)      )   strcat( buf, "{Y(Glowing){x "   );
  //if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "{C(Humming){x "   );
  if ( IS_OBJ_STAT(obj, ITEM_HUM)       )   strcat( buf, "{y(Humming){x "   );
  // modified by SinaC 2000
  //    if ( IS_OBJ_STAT(obj, ITEM_BLESS)       )   strcat( buf, "{B(Bless){x "   );
  // end
  
  if ( fShort ) {
    if ( obj->short_descr != NULL )
      strcat( buf, obj->short_descr );
  }
  else {
    if ( obj->description != NULL)
      strcat( buf, obj->description );
  }
  
  // Added by SinaC 2000
  if (strlen(buf)<=0) {
    bug("format_obj_to_char: Item vnum: %d has no description!", obj->pIndexData->vnum );
    strcpy(buf,"This object has no description. Please inform the IMP.");
  }

  return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing ) {
  char buf[MAX_STRING_LENGTH];
  BUFFER *output;
  const char **prgpstrShow;
  int *prgnShow;
  const char *pstrShow;
  OBJ_DATA *obj;
  int nShow;
  int iShow;
  int count;
  bool fCombine;

  if ( ch->desc == NULL )
    return;

  /*
   * Alloc space for output lines.
   */
  output = new_buf();

  count = 0;
  for ( obj = list; obj != NULL; obj = obj->next_content )
    count++;
  // Added by SinaC 2000
  if ( count > 5000 ) {
    send_to_char( "That is WAY too much junk!  Drop some of it!\n\r", ch );
    return;
  }

  prgpstrShow	= (const char **) GC_MALLOC( count * sizeof(const char *) );
  prgnShow    = (int *) GC_MALLOC_ATOMIC( count * sizeof(int)    );
  memset( prgnShow, 0, count*sizeof(int));
  nShow	= 0;

  /*
   * Format the list of objects.
   */
  for ( obj = list; obj != NULL; obj = obj->next_content ) { 
    if ( obj->wear_loc == WEAR_NONE && can_see_obj( ch, obj )) {
      pstrShow = format_obj_to_char( obj, ch, fShort );

      fCombine = FALSE;

      if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) ) {
	/*
	 * Look for duplicates, case sensitive.
	 * Matches tend to be near end so run loop backwords.
	 */
	for ( iShow = nShow - 1; iShow >= 0; iShow-- ) {
	  if ( !strcmp( prgpstrShow[iShow], pstrShow ) ) {
	    prgnShow[iShow]++;
	    fCombine = TRUE;
	    break;
	  }
	}
      }

      /*
       * Couldn't combine, or didn't want to.
       */
      if ( !fCombine ) {
	prgpstrShow [nShow] = str_dup( pstrShow );
	prgnShow    [nShow] = 1;
	nShow++;
      }
    }
  }

  /*
   * Output the formatted list.
   */
  for ( iShow = 0; iShow < nShow; iShow++ ) {
    if (prgpstrShow[iShow][0] == '\0') {
      continue;
    }

    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) ) {
      if ( prgnShow[iShow] != 1 ) {
	sprintf( buf, "{W(%2d){x ", prgnShow[iShow] );
	add_buf(output,buf);
      }
      else {
	add_buf(output,"     ");
      }
    }
    add_buf(output,prgpstrShow[iShow]);
    add_buf(output,"\n\r");
  }

  if ( fShowNothing && nShow == 0 ) {
    if ( IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE) )
      send_to_char( "     ", ch );
    send_to_char( "Nothing.\n\r", ch );
  }
  page_to_char(buf_string(output),ch);


  return;
}

void show_char_to_char_0( CHAR_DATA *victim, CHAR_DATA *ch ) {
  char buf[MAX_STRING_LENGTH],message[MAX_STRING_LENGTH];
  // Useless, SinaC 2000
  int i; /* Added for Quest Code by Seytan 1997 */
    
  buf[0] = '\0';


  // Added by SinaC 2001
  if (!IS_NPC(victim) && victim->desc == NULL)
    strcat( buf, "{D[LINKDEAD]{x ");

  // Added by SinaC 2000
  if (IS_NPC(victim)
      && victim->hunting != NULL )
    strcat( buf, "{M[HUNTING]{x ");

  // Moved by SinaC 2000
  //  if (IS_NPC(victim) 
  //      && !IS_NPC(ch) 
  //      && ch->pcdata->questmob > 0 
  //      && victim->pIndexData->vnum == ch->pcdata->questmob)
  //    strcat( buf, "{R[TARGET]{x ");
  
  if ( IS_SET(victim->comm,COMM_AFK	)   ) strcat( buf, "{G[AFK]{x "        );
  // Added by SinaC 2000
  if ( IS_SET(victim->comm,COMM_BUILDING)   ) strcat( buf, "{b[BUILDING]{x "   );
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if ( IS_SET(victim->comm,COMM_EDITING)   ) strcat( buf,  "{B[EDITING]{x "   );
  // Moved by SinaC 2003
  if ( IS_AFFECTED(victim, AFF_CHARM)       ) strcat( buf, "{C(Charmed){x "    );
  // Added by SinaC 2001
  if ( IS_AFFECTED( victim, AFF_FLYING )    ) strcat( buf, "{c(Flying){x "     );
  // Added by SinaC 2003
  if ( IS_AFFECTED2( victim, AFF2_NECROTISM ) ) strcat( buf, "{D(Necrotic){x " );
  // Added by SinaC 2003
  if ( is_affected( victim, gsn_phylactery ) ) strcat( buf, "{r(Crimson Aura){x " );
  // Added by SinaC 2001
  if ( is_affected( victim, gsn_shroud )    ) strcat( buf, "{D(D{xA{DR{xK{DN{xE{DS{xS{D){x " );
  // Added by SinaC 2003
  if ( is_affected( victim, gsn_sacred_mists ) ) strcat( buf, "{B({cM{Bi{cs{Bt{ci{Bn{cg{B){x ");
  if ( is_affected( victim, gsn_justice )   )  strcat( buf, "{W({YJ{Wu{Ys{Wt{Yi{Wc{Ye{W){x " );
  if ( is_affected( victim, gsn_corruption ))  strcat( buf, "{D({xC{Do{xr{Dr{xu{Dp{xt{Di{xo{Dn{x){x " );
  if ( is_affected( victim, gsn_symbiote )  )  strcat( buf, "{G({gSymbiote{G){x ");
  // Added by SinaC 2001
  if ( IS_AFFECTED(victim, AFF_ROOTED )     ) strcat( buf, "{g(Rooted){x "     ); 
  if ( IS_AFFECTED(victim, AFF_INVISIBLE)   ) strcat( buf, "{y(Invis){x "      );
  if ( victim->invis_level >= LEVEL_HERO    ) strcat( buf, "{W(Wizi){x "       );
  if ( IS_AFFECTED(victim, AFF_HIDE)        ) strcat( buf, "{b(Hide){x "       );
  if ( IS_AFFECTED(victim, AFF_SNEAK)       ) strcat( buf, "{R(Sneaking){x "   );
  if ( IS_AFFECTED(victim, AFF_PASS_DOOR)   ) strcat( buf, "{c(Translucent){x ");
  if ( IS_AFFECTED(victim, AFF_FAERIE_FIRE) ) strcat( buf, "{m(Pink Aura){x "  );
  // Added by SinaC 2001
  if ( IS_AFFECTED2(victim, AFF2_FAERIE_FOG) )strcat( buf, "{B(Faerie Fog){x " );
  // SinaC 2003
  if ( IS_AFFECTED2(victim, AFF2_CONFUSION) ) strcat( buf, "{R(Confused){x ");
  if ( IS_EVIL(victim)
       &&   IS_AFFECTED(ch, AFF_DETECT_EVIL)) strcat( buf, "{r(Red Aura){x "   );
  if ( IS_GOOD(victim)
       &&   IS_AFFECTED(ch, AFF_DETECT_GOOD)) strcat( buf, "{Y(Golden Aura){x ");
  // Modified by SinaC 2001
  if ( IS_AFFECTED(victim, AFF_SANCTUARY)   ) {
    if ( is_affected( victim, gsn_repulsion ) ) 
      strcat( buf, "{D(Black Aura){x " );
    else if ( is_affected( victim, gsn_wraithform ) )
      strcat( buf, "{D(Wra{xithf{Dorm){x " );
    else if ( is_affected( victim, gsn_manashield ) )
      strcat( buf, "{Y(Manashield){x " );
    // Added by SinaC 2003
    else if ( is_affected( victim, gsn_angelic_light ) )
      strcat( buf, "{y(Angelic){x " );
    else if ( is_affected( victim, gsn_nature_shield ) )
      strcat( buf, "{G({gN{Ga{gt{Gu{gr{Ge{g){x " );
    else if ( is_affected( victim, gsn_globe_invuln ) )
      strcat( buf, "{B(Hazy Aura){x " );
    else if ( is_affected( victim, gsn_elemental_field ) )
      strcat( buf, "{R(El{Yem{Ben{Gtal){x " );
    else
      strcat( buf, "{W(White Aura){x " );
  }
  if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_KILLER ) )
                                              strcat( buf, "{R[KILLER]{x "     );
  if ( !IS_NPC(victim) && IS_SET(victim->act, PLR_THIEF  ) )
                                              strcat( buf, "{R[THIEF]{x "      );
    
  /* Added for Quest Seytan 1997 */
  // Modified by SinaC 2000
  //  if ( victim->position == victim->start_pos 
  if ( victim->position == victim->default_pos 
       && victim->long_descr[0] != '\0' ) {
    //send_to_char("{Y",ch);
    strcat( buf, victim->long_descr );

    for(i=strlen(buf);i>0;i--) if(buf[i]=='\n' || buf[i]=='\r') buf[i]=0;
    /*      I prefer TARGET before the mob name, SinaC 2000        
     *
     *	if (IS_NPC(victim) && !IS_NPC(ch) && ch->pcdata->questmob > 0 && victim->pIndexData->vnum == ch->pcdata->questmob)
     *	strcat( buf, "{R [TARGET]");
     */      
    //strcat( buf, "{x\r\n");
    strcat( buf, "\n\r");

    send_to_char( buf, ch );
    return;
  }
                
    
  /* Removed by SinaC 2000, it's useless
   *if ( victim->position == victim->start_pos && victim->long_descr[0] != '\0' )
   *{
   *	strcat( buf, victim->long_descr );
   *	send_to_char( buf, ch );
   *	return;
   *}
   */

  // Added by SinaC 2000 for stances
  if ( !IS_NPC(victim) 
       && victim->pcdata->stance != NULL 
       && victim->pcdata->stance[0]!='\0' && !IS_SET(ch->comm, COMM_BRIEF) 
       && victim->position == POS_STANDING && ch->on == NULL )
    strcat( buf, victim->pcdata->stance );
  else {
    //strcat( buf, "{Y");
    strcat( buf, PERS( victim, ch ) );
    if ( !IS_NPC(victim) && !IS_SET(ch->comm, COMM_BRIEF) 
	 && victim->position == POS_STANDING && ch->on == NULL )
      strcat( buf, victim->pcdata->title );
  }

  switch ( victim->position ) {
  case POS_DEAD:     strcat( buf, " is DEAD!!" );              break;
  case POS_MORTAL:   strcat( buf, " is mortally wounded." );   break;
  case POS_INCAP:    strcat( buf, " is incapacitated." );      break;
    // Added by SinaC 2003 for paralyzing
  case POS_PARALYZED:strcat( buf, " is paralyzed." );          break;
  case POS_STUNNED:  strcat( buf, " is lying here stunned." ); break;
  case POS_SLEEPING: 
    if (victim->on != NULL){
      if (IS_SET(victim->on->value[2],SLEEP_AT)) {
	sprintf(message," is sleeping at %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else if (IS_SET(victim->on->value[2],SLEEP_ON)) {
	sprintf(message," is sleeping on %s.",
		victim->on->short_descr); 
	strcat(buf,message);
      }
      else {
	sprintf(message, " is sleeping in %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
    }
    else 
      strcat(buf," is sleeping here.");
    break;
  case POS_RESTING:  
    if (victim->on != NULL) {
      if (IS_SET(victim->on->value[2],REST_AT)) {
	sprintf(message," is resting at %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else if (IS_SET(victim->on->value[2],REST_ON)) {
	sprintf(message," is resting on %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else {
	sprintf(message, " is resting in %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
    }
    else
      strcat( buf, " is resting here." );       
    break;
  case POS_SITTING:  
    if (victim->on != NULL) {
      if (IS_SET(victim->on->value[2],SIT_AT)) {
	sprintf(message," is sitting at %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else if (IS_SET(victim->on->value[2],SIT_ON)) {
	sprintf(message," is sitting on %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else {
	sprintf(message, " is sitting in %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
    }
    else
      strcat(buf, " is sitting here.");
    break;
  case POS_STANDING: 
    // Added by SinaC 2000 for stance
    if ( !IS_NPC( victim ) && victim->pcdata->stance != NULL && victim->pcdata->stance[0]!='\0' )
      break;
    if (victim->on != NULL) {
      if (IS_SET(victim->on->value[2],STAND_AT)) {
	sprintf(message," is standing at %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else if (IS_SET(victim->on->value[2],STAND_ON)) {
	sprintf(message," is standing on %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
      else {
	sprintf(message," is standing in %s.",
		victim->on->short_descr);
	strcat(buf,message);
      }
    }
    else
      strcat( buf, " is here." );               
    break;
  case POS_FIGHTING:
    strcat( buf, " is here, fighting " );
    if ( victim->fighting == NULL )
      strcat( buf, "thin air??" );
    else if ( victim->fighting == ch )
      strcat( buf, "YOU!" );
    else if ( victim->in_room == victim->fighting->in_room ) {
      strcat( buf, PERS( victim->fighting, ch ) );
      strcat( buf, "." );
    }
    else
      strcat( buf, "someone who left??" );
    break;
  }

  /* Removed by SinaC 2000
     if (IS_NPC(victim) 
     && !IS_NPC(ch) 
     && ch->pcdata->questmob > 0 
     && victim->pIndexData->vnum == ch->pcdata->questmob)
     strcat( buf, "{R [TARGET]{x ");
  */  
  
  //strcat( buf, "{x\n\r" );
  strcat( buf, "\n\r" );
  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );
  return;
}

void show_char_to_char_1( CHAR_DATA *victim, CHAR_DATA *ch ) {
  char buf[MAX_STRING_LENGTH];
  //  OBJ_DATA *obj;
  //  int iWear;
  int percent;
  bool found;

  if ( can_see( victim, ch ) ) {
    if (ch == victim)
      act( "$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
    else {
      act( "$n looks at you.", ch, NULL, victim, TO_VICT    );
      act( "$n looks at $N.",  ch, NULL, victim, TO_NOTVICT );
    }
  }

  // Added by SinaC 2001
  Value args[] = {ch};
  bool result = mobprog(victim,ch,"onLooking", args);
  if ( !result ) { // result == 0, we see the mob description
    if ( victim->description[0] != '\0' ) {
      send_to_char( victim->description, ch );
    }
    else {
      act( "You see nothing special about $M.", ch, NULL, victim, TO_CHAR );
    }
  }    
  
  if ( victim->cstat(max_hit) > 0 )
    percent = ( 100 * victim->hit ) / victim->cstat(max_hit);
  else
    percent = -1;

  strcpy( buf, PERS(victim, ch) );

  if (percent >= 100)
    strcat( buf, " is in excellent condition.\n\r");
  else if (percent >= 90)
    strcat( buf, " has a few scratches.\n\r");
  else if (percent >= 75) 
    strcat( buf," has some small wounds and bruises.\n\r");
  else if (percent >=  50) 
    strcat( buf, " has quite a few wounds.\n\r");
  else if (percent >= 30)
    strcat( buf, " has some big nasty wounds and scratches.\n\r");
  else if (percent >= 15)
    strcat ( buf, " looks pretty hurt.\n\r");
  else if (percent >= 0 )
    strcat (buf, " is in awful condition.\n\r");
  else
    strcat(buf, " is bleeding to death.\n\r");

  buf[0] = UPPER(buf[0]);
  send_to_char( buf, ch );

  found = FALSE;
  for ( int iWear = 0; iWear < MAX_WEAR; iWear++ ) {
    int mapped = mapping_for_nicer_output[iWear]; // SinaC 2003
    //if ( ( obj = get_eq_char( victim, iWear ) ) != NULL
    OBJ_DATA *obj;
    if ( ( obj = get_eq_char( victim, mapped ) ) != NULL
	 &&   can_see_obj( ch, obj ) ) {
      if ( !found ) {
	send_to_char( "\n\r", ch );
	act( "$N is using:", ch, NULL, victim, TO_CHAR );
	found = TRUE;
      }
      //send_to_char( where_name[iWear], ch );
      send_to_char( where_name[mapped], ch );
      send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
      send_to_char( "\n\r", ch );
    }
  }

  if ( victim != ch
       && !IS_NPC(ch)
       && number_percent( ) < get_ability(ch,gsn_peek)) {
    send_to_char( "\n\rYou peek at the inventory:\n\r", ch );
    check_improve(ch,gsn_peek,TRUE,4);
    show_list_to_char( victim->carrying, ch, TRUE, TRUE );
  }

  return;
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch ) {
  CHAR_DATA *rch;

  for ( rch = list; rch != NULL; rch = rch->next_in_room ) {
    if ( rch == ch )
      continue;

    if ( get_trust(ch) < rch->invis_level)
      continue;

    if ( can_see( ch, rch ) ) {
      show_char_to_char_0( rch, ch );
    }
    else if ( room_is_dark( ch->in_room )
	      && IS_AFFECTED(rch, AFF_INFRARED ) ) {
      send_to_char( "You see glowing red eyes watching YOU!\n\r", ch );
    }
  }

  return;
} 

bool check_blind( CHAR_DATA *ch ) {
  /* Modified by SinaC 2000
   *if (!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT))
   * return TRUE;
   */
  if ( !IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT) && IS_IMMORTAL(ch) )
    return TRUE;

  if ( IS_AFFECTED(ch, AFF_BLIND) ) { 
    send_to_char( "You can't see a thing!\n\r", ch ); 
    return FALSE; 
  }

  return TRUE;
}

/* changes your scroll */
void do_scroll(CHAR_DATA *ch, const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  char buf[100];
  int lines;

  one_argument(argument,arg);
    
  if (IS_NPC(ch))
    return;

  if (arg[0] == '\0') {
    if (ch->lines == 0)
      send_to_char("You do not page long messages.\n\r",ch);
    else {
      sprintf(buf,"You currently display %d lines per page.\n\r",
	      ch->lines + 2);
      send_to_char(buf,ch);
    }
    return;
  }

  if (!is_number(arg)) {
    send_to_char("You must provide a number.\n\r",ch);
    return;
  }

  lines = atoi(arg);

  if (lines == 0) {
    send_to_char("Paging disabled.\n\r",ch);
    ch->lines = 0;
    return;
  }

  if (lines < 10 || lines > 100) {
    send_to_char("You must provide a reasonable number.\n\r",ch);
    return;
  }

  sprintf(buf,"Scroll set to %d lines.\n\r",lines);
  send_to_char(buf,ch);
  ch->lines = lines - 2;
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, const char *argument) {
  char buf[MAX_STRING_LENGTH];
  int iSocial;
  int col;
     
  col = 0;
   
  for (iSocial = 0; social_table[iSocial].name[0] != '\0'; iSocial++) {
    sprintf(buf,"%-12s",social_table[iSocial].name);
    send_to_char(buf,ch);
    if (++col % 6 == 0)
      send_to_char("\n\r",ch);
  }

  if ( col % 6 != 0)
    send_to_char("\n\r",ch);
  return;
}
 
/* Removed by SinaC 2001
// RT Commands to replace news, motd, imotd, etc from ROM
void do_motd(CHAR_DATA *ch, const char *argument)
{
  do_help(ch,"motd");
}

void do_imotd(CHAR_DATA *ch, const char *argument)
{  
  do_help(ch,"imotd");
}

void do_rules(CHAR_DATA *ch, const char *argument)
{
  do_help(ch,"rules");
}

void do_story(CHAR_DATA *ch, const char *argument)
{
  do_help(ch,"story");
}
*/
void do_wizlist(CHAR_DATA *ch, const char *argument) {
  do_help(ch,"wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */
void do_autolist(CHAR_DATA *ch, const char *argument) {
  /* lists most player flags */
  if (IS_NPC(ch))
    return;

  // Modified by SinaC 2001

  send_to_char("   action        status         action     status\n\r",ch);
  send_to_char("-------------------------------------------------\n\r",ch);

  send_to_char("autoassist        ",ch);
  if (IS_SET(ch->act,PLR_AUTOASSIST))
    send_to_char("ON ",ch);
  else
    send_to_char("OFF",ch); 

  send_to_char("      autoexit          ",ch);
  if (IS_SET(ch->act,PLR_AUTOEXIT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("autogold          ",ch);
  if (IS_SET(ch->act,PLR_AUTOGOLD))
    send_to_char("ON ",ch);
  else
    send_to_char("OFF",ch);

  send_to_char("      autoloot          ",ch);
  if (IS_SET(ch->act,PLR_AUTOLOOT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("autosac           ",ch);
  if (IS_SET(ch->act,PLR_AUTOSAC))
    send_to_char("ON ",ch);
  else
    send_to_char("OFF",ch);

  send_to_char("      autosplit         ",ch);
  if (IS_SET(ch->act,PLR_AUTOSPLIT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);
        
  send_to_char("autotitle         ",ch);
  if (IS_SET(ch->act,PLR_AUTOTITLE))
    send_to_char("ON ",ch);
  else
    send_to_char("OFF",ch);        

  send_to_char("      autoaffect        ",ch);
  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  // Added by SinaC 2001
  send_to_char("autotick          ",ch);
  if (IS_SET(ch->comm,COMM_TICK))
    send_to_char("ON",ch);
  else
    send_to_char("OFF",ch);

  // SinaC 2003, autohaggle
  send_to_charf(ch,"      autohaggle        %s\n\r",IS_SET(ch->act,PLR_AUTOHAGGLE)?"ON":"OFF");

  send_to_char("\n\r", ch);

  send_to_char("compact mode      ",ch);
  if (IS_SET(ch->comm,COMM_COMPACT))
    send_to_char("ON ",ch);
  else
    send_to_char("OFF",ch);

  send_to_char("      prompt            ",ch);
  if (IS_SET(ch->comm,COMM_PROMPT))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("combine items     ",ch);
  if (IS_SET(ch->comm,COMM_COMBINE))
    send_to_char("ON",ch);
  else
    send_to_char("OFF",ch);

  // Added by SinaC 2001
  send_to_char("       brief room descr  ",ch);
  if (IS_SET(ch->comm,COMM_BRIEF))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  // Added by SinaC 2001
  send_to_char("ansi color        ",ch);
  if (IS_SET(ch->act,PLR_COLOUR))
    send_to_char("ON\n\r",ch);
  else
    send_to_char("OFF\n\r",ch);

  send_to_char("\n\r", ch);

  if (!IS_SET(ch->act,PLR_CANLOOT))
    send_to_char("Your corpse is safe from thieves.\n\r",ch);
  else 
    send_to_char("Your corpse may be looted.\n\r",ch);

  if (IS_SET(ch->act,PLR_NOSUMMON))
    send_to_char("You cannot be summoned.\n\r",ch);
  else
    send_to_char("You can be summoned.\n\r",ch);
   
  if (IS_SET(ch->act,PLR_NOFOLLOW))
    send_to_char("You do not welcome followers.\n\r",ch);
  else
    send_to_char("You accept followers.\n\r",ch);

  send_to_char("\n\r", ch);

  // Added by SinaC 2001
  send_to_charf(ch,"Scroll set to %d lines.\n\r",ch->lines+2);
  send_to_charf(ch,"Wimpy set to %d.\n\r",ch->wimpy);
}

// Added by SinaC 2001
void do_autotick(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
    
  if (IS_SET(ch->comm,COMM_TICK)) {
    send_to_char("Autotick removed.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_TICK);
  }
  else {
    send_to_char("You will now see tick.\n\r",ch);
    SET_BIT(ch->comm,COMM_TICK);
  }
}

void do_autoassist(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
    
  if (IS_SET(ch->act,PLR_AUTOASSIST)) {
    send_to_char("Autoassist removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOASSIST);
  }
  else {
    send_to_char("You will now assist when needed.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOASSIST);
  }
}

void do_autoexit(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOEXIT)) {
    send_to_char("Exits will no longer be displayed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOEXIT);
  }
  else {
    send_to_char("Exits will now be displayed.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOEXIT);
  }
}

void do_autogold(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOGOLD)) {
    send_to_char("Autogold removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOGOLD);
  }
  else {
    send_to_char("Automatic gold looting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOGOLD);
  }
}

void do_autoloot(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOLOOT)) {
    send_to_char("Autolooting removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOLOOT);
  }
  else {
    send_to_char("Automatic corpse looting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOLOOT);
  }
}

void do_autosac(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOSAC)) {
    send_to_char("Autosacrificing removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOSAC);
  }
  else {
    send_to_char("Automatic corpse sacrificing set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOSAC);
  }
}

void do_autosplit(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOSPLIT)) {
    send_to_char("Autosplitting removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOSPLIT);
  }
  else {
    send_to_char("Automatic gold splitting set.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOSPLIT);
  }
}

void do_autotitle(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOTITLE)) {
    send_to_char("Autotitle removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOTITLE);
  }
  else {
    send_to_char("Your title now change when you raise level.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOTITLE);
  }
}

void do_autoaff(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS)) {
    send_to_char("You are no longer seeing your affects in the score.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
  }
  else {
    send_to_char("You now see your affects in the score.\n\r",ch);
    SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
  }
}

void do_brief(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->comm,COMM_BRIEF)) {
    send_to_char("Full descriptions activated.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_BRIEF);
  }
  else {
    send_to_char("Short descriptions activated.\n\r",ch);
    SET_BIT(ch->comm,COMM_BRIEF);
  }
}

void do_compact(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->comm,COMM_COMPACT)) {
    send_to_char("Compact mode removed.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_COMPACT);
  }
  else {
    send_to_char("Compact mode set.\n\r",ch);
    SET_BIT(ch->comm,COMM_COMPACT);
  }
}

/* Unused cos' of autoaffect  SinaC 2000
 *void do_show(CHAR_DATA *ch, const char *argument)
 *{
 *   if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
 *   {
 *     send_to_char("Affects will no longer be shown in score.\n\r",ch);
 *     REMOVE_BIT(ch->comm,COMM_SHOW_AFFECTS);
 *   }
 *   else
 *   {
 *     send_to_char("Affects will now be shown in score.\n\r",ch);
 *     SET_BIT(ch->comm,COMM_SHOW_AFFECTS);
 *   }
 *}
 */
void do_prompt(CHAR_DATA *ch, const char *argument) {
  char buf[MAX_STRING_LENGTH];

  if (IS_NPC(ch))
    return;
 
  if ( argument[0] == '\0' ) {
    if (IS_SET(ch->comm,COMM_PROMPT)) {
      send_to_char("You will no longer see prompts.\n\r",ch);
      REMOVE_BIT(ch->comm,COMM_PROMPT);
    }
    else {
      send_to_char("You will now see prompts.\n\r",ch);
      SET_BIT(ch->comm,COMM_PROMPT);
    }
    return;
  }
 
  if( !strcmp( argument, "all" ) )
    //strcpy( buf, "{c<%hhp %mm %vmv>{x " );
    strcpy( buf, "{c<%h/%HHp %m/%MMn %v/%VMv %XNxt %b>{x");
  else {
    strcpy( buf, argument );
    if ( strlen(buf) > 50 )
      buf[50] = '\0';
    //    smash_tilde( buf );
    if (str_suffix("%c",buf))
      strcat(buf," ");
	
  }
 
  ch->prompt = str_dup( buf );
  sprintf(buf,"Prompt set to %s\n\r",ch->prompt );
  send_to_char(buf,ch);
  return;
}

void do_combine(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;

  if (IS_SET(ch->comm,COMM_COMBINE)) {
    send_to_char("Long inventory selected.\n\r",ch);
    REMOVE_BIT(ch->comm,COMM_COMBINE);
  }
  else {
    send_to_char("Combined inventory selected.\n\r",ch);
    SET_BIT(ch->comm,COMM_COMBINE);
  }
}

void do_noloot(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_CANLOOT)) {
    send_to_char("Your corpse is now safe from thieves.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_CANLOOT);
  }
  else {
    send_to_char("Your corpse may now be looted.\n\r",ch);
    SET_BIT(ch->act,PLR_CANLOOT);
  }
}

void do_nofollow(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_NOFOLLOW)) {
    send_to_char("You now accept followers.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_NOFOLLOW);
  }
  else {
    send_to_char("You no longer accept followers.\n\r",ch);
    SET_BIT(ch->act,PLR_NOFOLLOW);
    die_follower( ch );
  }
}

void do_nosummon(CHAR_DATA *ch, const char *argument) {
  // Added by SinaC 2000
  if (IS_NPC(ch))
    return;

  if (IS_NPC(ch)) {
    if (IS_SET(ch->bstat(imm_flags),IRV_SUMMON)) {
      send_to_char("You are no longer immune to summon.\n\r",ch);
      REMOVE_BIT(ch->bstat(imm_flags),IRV_SUMMON);
    }
    else {
      send_to_char("You are now immune to summoning.\n\r",ch);
      SET_BIT(ch->bstat(imm_flags),IRV_SUMMON);
    }
    recompute(ch);
  }
  else  {
    if (IS_SET(ch->act,PLR_NOSUMMON)) {
      send_to_char("You are no longer immune to summon.\n\r",ch);
      REMOVE_BIT(ch->act,PLR_NOSUMMON);
    }
    else {
      send_to_char("You are now immune to summoning.\n\r",ch);
      SET_BIT(ch->act,PLR_NOSUMMON);
    }
  }
}

/*
 * Colour setting and unsetting, way cool, Lope Oct '94
 */
void do_colour( CHAR_DATA *ch, const char *argument ) {
  char 	arg[ MAX_STRING_LENGTH ];

  if (IS_NPC(ch))
    return;

  argument = one_argument( argument, arg );

  if( !*arg ) {
    if( !IS_SET( ch->act, PLR_COLOUR ) ) {
      SET_BIT( ch->act, PLR_COLOUR );
      send_to_char( "{bC{ro{yl{co{mu{gr{x is now {rON{x, Way Cool!\n\r", ch );
    }
    else {
      send_to_char_bw( "Colour is now OFF, <sigh>\n\r", ch );
      REMOVE_BIT( ch->act, PLR_COLOUR );
    }
    return;
  }
  else {
    send_to_char_bw( "Colour Configuration is unavailable in this\n\r", ch );
    send_to_char_bw( "version of colour, sorry\n\r", ch );
  }

  return;
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  int wimpy;

  one_argument( argument, arg );

  if ( arg[0] == '\0' )
    wimpy = ch->cstat(max_hit) / 5;
  else
    wimpy = atoi( arg );

  if ( wimpy < 0 ){
    send_to_char( "Your courage exceeds your wisdom.\n\r", ch );
    return;
  }

  if ( wimpy > ch->cstat(max_hit)/2 ){
    send_to_char( "Such cowardice ill becomes you.\n\r", ch );
    return;
  }

  ch->wimpy	= wimpy;
  sprintf( buf, "Wimpy set to %d hit points.\n\r", wimpy );
  send_to_char( buf, ch );
  return;
}

void do_autohaggle(CHAR_DATA *ch, const char *argument) {
  if (IS_NPC(ch))
    return;
 
  if (IS_SET(ch->act,PLR_AUTOHAGGLE)) {
    send_to_char("Autohaggle removed.\n\r",ch);
    REMOVE_BIT(ch->act,PLR_AUTOHAGGLE);
  }
  else {
    send_to_char("You'll try to haggle when trading with shopkeepers.\n\r",ch);
    SET_BIT(ch->act,PLR_AUTOHAGGLE);
  }
}

// Added by SinaC 2001 to group every auto_XXX, combine, compact, scroll, ...
void config_syntax( CHAR_DATA *ch ) {
  send_to_charf(ch,
		"Syntax:  config <specifier> [<modifier>]\n\r"
		" specifiers :\n\r"
                " no arg    : display current configuration\n\r"
                " autolist  : display current configuration\n\r"
		" autoassist: makes you help group members in combat\n\r"
		" autoexit  : display room exits upon entering a room\n\r"
		" autogold  : take all gold from dead mobiles\n\r"
		" autoloot  : take all equipment from dead mobiles\n\r"
		" autosac   : sacrifice dead monsters (if autoloot is on, only empty corpses)\n\r"
		" autosplit : split up spoils from combat among your group members\n\r"
		" autotitle : your title change when you gain a level\n\r"
		" autoaffect: display your affects when looking your score\n\r"
		" autotick  : you receive 'TICK' each tick\n\r"
		" autohaggle: you automatically use haggle skill when trading with shopkeepers\n\r" // SinaC 2003
		" compact   : removes the extra blank line before your prompt\n\r"
		" prompt    : turn your prompt on or off\n\r"
		" combine   : toggle between long and short inventory\n\r"
		" brief     : show room description when you move around\n\r"
		" color     : toggles color mode on/off\n\r"
		" noloot    : turn your corpse safety from thieves on or off\n\r"
		" nosummon  : turn your immune to summon on or off\n\r"
		" nofollow  : toggle between accepting or not followers.\n\r"
		" scroll [<#lines>]: changes the number of lines the mud sends you in a page\n\r"
		" wimpy <number>: sets your wimpy value\n\r");
}
void do_config( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  if ( arg1[0] == '\0' )                         do_autolist(ch,"");
  else if ( !str_prefix( arg1, "autolist" ) )    do_autolist(ch,"");
  else if ( !str_prefix( arg1, "autoassist" ) )  do_autoassist(ch,"");
  else if ( !str_prefix( arg1, "autoexit" ) )    do_autoexit(ch,"");
  else if ( !str_prefix( arg1, "autogold" ) )    do_autogold(ch,"");
  else if ( !str_prefix( arg1, "autoloot" ) )    do_autoloot(ch,"");
  else if ( !str_prefix( arg1, "autosac" ) )     do_autosac(ch,"");
  else if ( !str_prefix( arg1, "autosplit" ) )   do_autosplit(ch,"");
  else if ( !str_prefix( arg1, "autotitle" ) )   do_autotitle(ch,"");
  else if ( !str_prefix( arg1, "autoaffect" ) )  do_autoaff(ch,"");
  else if ( !str_prefix( arg1, "autotick" ) )    do_autotick(ch,"");
  else if ( !str_prefix( arg1, "compact" ) )     do_compact(ch,"");
  else if ( !str_prefix( arg1, "prompt" ) )      do_prompt(ch,"");
  else if ( !str_prefix( arg1, "combine" ) )     do_combine(ch,"");
  else if ( !str_prefix( arg1, "brief" ) )       do_brief(ch,"");
  else if ( !str_prefix( arg1, "color" ) )       do_colour(ch,"");
  else if ( !str_prefix( arg1, "noloot" ) )      do_noloot(ch,"");
  else if ( !str_prefix( arg1, "nosummon" ) )    do_nosummon(ch,"");
  else if ( !str_prefix( arg1, "nofollow" ) )    do_nofollow(ch,"");
  else if ( !str_prefix( arg1, "scroll" ) )      do_scroll(ch,arg2);
  else if ( !str_prefix( arg1, "wimpy" ) )       do_wimpy(ch,arg2);
  else if ( !str_prefix( arg1, "autohaggle" ) )  do_autohaggle(ch,"");
  else                                           config_syntax( ch );
}

// Find an extra description, if found return <description> else return NULL
//  <number> extra description with keyword <name> are considered before returning description
//  if not enough description are found, NULL is returned  but count is updated
const char *look_get_extra_descr( const char *name, EXTRA_DESCR_DATA *ed,
				  int &count, const int number ) {
  for ( ; ed != NULL; ed = ed->next )
    if ( is_name( (char *) name, ed->keyword ) )
      if ( ++count == number )
	return ed->description;
  return NULL;
}

void do_look( CHAR_DATA *ch, const char *argument ) {
  char buf  [MAX_STRING_LENGTH];
  char arg1 [MAX_INPUT_LENGTH];
  char arg2 [MAX_INPUT_LENGTH];
  char arg3 [MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  CHAR_DATA *victim;
  OBJ_DATA *obj;
  const char *pdesc;
  int door;
  int number,count;

  /* Removed by JyP 2000 ;)
     if ( ch->desc == NULL )
     return;
  */

  if ( ch->position < POS_SLEEPING ) {
    send_to_char( "You can't see anything but stars!\n\r", ch );
    return;
  }

  if ( ch->position == POS_SLEEPING ) {
    send_to_char( "You can't see anything, you're sleeping!\n\r", ch );
    return;
  }

  if ( !check_blind( ch ) )
    return;

  if ( !IS_NPC(ch)
       && !IS_SET(ch->act, PLR_HOLYLIGHT)
       // Added by SinaC 2000
       && !IS_IMMORTAL(ch)
       && room_is_dark( ch->in_room ) 
       /* added by Sinac 1997 */    
       && !IS_AFFECTED(ch,AFF_INFRARED )
       && !IS_AFFECTED(ch,AFF_DARK_VISION) ) {
    send_to_char( "It is pitch black ... \n\r", ch );
    show_char_to_char( ch->in_room->people, ch );
    return;
  }

  // look in 2.chest: arg1 = in         arg2 = 2.chest   arg3 = in        number = 1
  // look 2.picture:         2.picture         /                picture            2
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  number = number_argument(arg1,arg3);
  count = 0;

  if ( arg1[0] == '\0' || !str_cmp( arg1, "auto" ) ) {
    
    /* 'look' or 'look auto' */
    /*
      send_to_char("{m", ch);
      send_to_char( ch->in_room->name, ch );
      send_to_char("{x", ch);
    */
    // modified by SinaC 2000
    //sprintf(buf,"{m%s{x",ch->in_room->name);
    sprintf(buf,"{c%s{x",ch->in_room->name);
    send_to_char(buf,ch);

    if (IS_IMMORTAL(ch) && (IS_NPC(ch) || IS_SET(ch->act,PLR_HOLYLIGHT))) {
      sprintf(buf," [{cRoom %d{x]",ch->in_room->vnum);
      send_to_char(buf,ch);
    }

    send_to_char( "\n\r", ch );

    // Added by SinaC 2003 for room program
    Value args[] = {ch};
    int value = roomprog(ch->in_room,ch,"onLooking",args);
    bool noRoomDesc = IS_SET_BIT(value,0);
    bool noExitDesc = IS_SET_BIT(value,1);
    bool noCharDesc = IS_SET_BIT(value,2);
    bool noObjDesc = IS_SET_BIT(value,3);

    // Added by SinaC 2003
    if ( !noRoomDesc ) {
      if ( arg1[0] == '\0'
	   || ( !IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF) ) ) {
	// Added by SinaC 2000, not test previously  for 'jog'
	if (ch->desc && !ch->desc->run_buf) {
	  send_to_char( "  ",ch);
	  send_to_char( ch->in_room->description, ch );
	}
      }
    }

    // Added by SinaC 2003
    if ( !noExitDesc )
      if ( !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT) ) {
	send_to_char("\n\r",ch);
	do_exits( ch, "auto" );
      }

    // Added by SinaC 2003
    if ( !noObjDesc )
      show_list_to_char( ch->in_room->contents, ch, FALSE, FALSE );
    // Added by SinaC 2003
    if ( !noCharDesc )
      show_char_to_char( ch->in_room->people,   ch );
    return;
  }

  if ( !str_cmp( arg1, "i" ) || !str_cmp(arg1, "in")  || !str_cmp(arg1,"on")) {
    /* 'look in' */
    if ( arg2[0] == '\0' ) {
      send_to_char( "Look in what?\n\r", ch );
      return;
    }

    if ( ( obj = get_obj_here( ch, arg2 ) ) == NULL ) {
      send_to_char( "You do not see that here.\n\r", ch );
      return;
    }

    switch ( obj->item_type )	{
    default:
      send_to_char( "That is not a container.\n\r", ch );
      break;

    case ITEM_DRINK_CON:
      if ( obj->value[1] <= 0 ) {
	send_to_char( "It is empty.\n\r", ch );
	break;
      }

      sprintf( buf, "It's %sfilled with  a %s liquid.\n\r",
	       obj->value[1] <     obj->value[0] / 4
	       ? "less than half-" :
	       obj->value[1] < 3 * obj->value[0] / 4
	       ? "about half-"     : "more than half-",
	       liq_table[obj->value[2]].liq_color
	       );

      send_to_char( buf, ch );
      break;

    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
    case ITEM_CONTAINER:
      if ( IS_SET(obj->value[1], CONT_CLOSED) ) {
	send_to_char( "It is closed.\n\r", ch );
	break;
      }

      act( "$p holds:", ch, obj, NULL, TO_CHAR );
      show_list_to_char( obj->contains, ch, TRUE, TRUE );
      break;
    }
    return;
  }

  if ( ( victim = get_char_room( ch, arg1 ) ) != NULL ) {
    show_char_to_char_1( victim, ch );
    return;
  }

  for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( can_see_obj( ch, obj ) ) {  /* player can see object */

      pdesc = get_extra_descr( arg3, obj->extra_descr );
      if ( pdesc != NULL )
	if (++count == number) {

	  // Added by SinaC 2001
	  Value args[] = {ch};
	  bool result = objprog(obj,ch,"onLooking", args);
	  if ( result ) 
	    return;

	  send_to_char( pdesc, ch );

	  // Added by SinaC 2001
          //OBJPROG(obj,ch,"onLook", ch );
	  return;
	}
	else continue;
      
      pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
      if ( pdesc != NULL )
	if (++count == number) {	

	  // Added by SinaC 2001
	  Value args[] = {ch};
	  bool result = objprog(obj,ch,"onLooking", args);
	  if ( result ) 
	    return;
	  
	  send_to_char( pdesc, ch );

	  // Added by SinaC 2001
          //OBJPROG(obj,ch,"onLook", ch );
	  return;
	}
	else continue;
      
      if ( is_name( arg3, obj->name ) )
	if (++count == number) {
	  // Added by SinaC 2001
	  Value args[] = {ch};
	  bool result = objprog(obj,ch,"onLooking", args);
	  if ( result ) 
	    return;

	  send_to_char( obj->description, ch );
	  send_to_char( "\n\r",ch);
	  // SinaC 2003, corpse->value4 gives missing parts
	  if ( obj->item_type == ITEM_CORPSE_NPC
	       || obj->item_type == ITEM_CORPSE_PC ) {
	    if ( IS_SET(obj->value[4], PART_HEAD ) )
	      send_to_char("The head of the corpse is missing.\n\r",ch);
	    if ( IS_SET(obj->value[4], PART_ARMS ) )
	      send_to_char("The arms of the corpse are missing.\n\r",ch);
	    if ( IS_SET(obj->value[4], PART_LEGS ) )
	      send_to_char("The legs of the corpse are missing.\n\r",ch);
	  }

	  // Added by SinaC 2001
          //OBJPROG(obj,ch,"onLook", ch );
	  return;
	}
    }
  }

  for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content ) {
    if ( can_see_obj( ch, obj ) ) {
      // Added by SinaC 2000 for window code
      /* Voltecs Window code */
      
      if (is_name(obj->name,arg1) && obj->item_type == ITEM_WINDOW) {
	// Added by SinaC 2001
	Value args[] = {ch};
	bool result = objprog(obj,ch,"onLooking", args);
	if ( result ) 
	  return;

	look_window(ch, obj);
	return;
      } 

      pdesc = get_extra_descr( arg3, obj->extra_descr );
      if ( pdesc != NULL )
	if (++count == number) {
	  // Added by SinaC 2001
	  Value args[] = {ch};
	  bool result = objprog(obj,ch,"onLooking", args);
	  if ( result ) 
	    return;

	  send_to_char( pdesc, ch );

	  // Added by SinaC 2001
          //OBJPROG(obj,ch,"onLook", ch );
	  return;
	}

      pdesc = get_extra_descr( arg3, obj->pIndexData->extra_descr );
      if ( pdesc != NULL )
	if (++count == number) {
	  // Added by SinaC 2001
	  Value args[] = {ch};
	  bool result = objprog(obj,ch,"onLooking", args);
	  if ( result ) 
	    return;
	  
	  send_to_char( pdesc, ch );

	  // Added by SinaC 2001
          //OBJPROG(obj,ch,"onLook", ch );
	  return;
	}

      if ( is_name( arg3, obj->name ) )
	if (++count == number) {
	  // Added by SinaC 2001
	  Value args[] = {ch};
	  bool result = objprog(obj,ch,"onLooking", args);
	  if ( result ) 
	    return;

	  send_to_char( obj->description, ch );
	  send_to_char("\n\r",ch);
	  // SinaC 2003, corpse->value4 gives missing parts
	  if ( obj->item_type == ITEM_CORPSE_NPC
	       || obj->item_type == ITEM_CORPSE_PC ) {
	    if ( IS_SET(obj->value[4], PART_HEAD ) )
	      send_to_char("The head of the corpse is missing.\n\r",ch);
	    if ( IS_SET(obj->value[4], PART_ARMS ) )
	      send_to_char("The arms of the corpse are missing.\n\r",ch);
	    if ( IS_SET(obj->value[4], PART_LEGS ) )
	      send_to_char("The legs of the corpse are missing.\n\r",ch);
	  }
            
	  // Added by SinaC 2001
          //OBJPROG(obj,ch,"onLook", ch );
	  return;
	}
    }
  }

  pdesc = look_get_extra_descr( arg3, ch->in_room->extra_descr, count, number );
  if ( pdesc != NULL ) {
    send_to_char( pdesc, ch );
    return;
  }
  //  pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
  //if (pdesc != NULL) {
  //  if (++count == number) {
  //    send_to_char(pdesc,ch);
  //    return;
  //  }
  //}

  if (count > 0 && count != number) {
    if (count == 1)
      sprintf(buf,"You only see one %s here.\n\r",arg3);
    else
      sprintf(buf,"You only see %d of those here.\n\r",count);
    	
    send_to_char(buf,ch);
    return;
  }

  if ( !str_cmp( arg1, "n" ) || !str_cmp( arg1, "north" ) ) door = DIR_NORTH;
  else if ( !str_cmp( arg1, "e" ) || !str_cmp( arg1, "east"  ) ) door = DIR_EAST;
  else if ( !str_cmp( arg1, "s" ) || !str_cmp( arg1, "south" ) ) door = DIR_SOUTH;
  else if ( !str_cmp( arg1, "w" ) || !str_cmp( arg1, "west"  ) ) door = DIR_WEST;
  else if ( !str_cmp( arg1, "u" ) || !str_cmp( arg1, "up"    ) ) door = DIR_UP;
  else if ( !str_cmp( arg1, "d" ) || !str_cmp( arg1, "down"  ) ) door = DIR_DOWN;
  // Added by SinaC 2003
  else if ( !str_cmp( arg1, "ne" ) || !str_cmp( arg1, "northeast"  ) ) door = DIR_NORTHEAST;
  else if ( !str_cmp( arg1, "nw" ) || !str_cmp( arg1, "northwest"  ) ) door = DIR_NORTHWEST;
  else if ( !str_cmp( arg1, "se" ) || !str_cmp( arg1, "southeast"  ) ) door = DIR_SOUTHEAST;
  else if ( !str_cmp( arg1, "sw" ) || !str_cmp( arg1, "southwest"  ) ) door = DIR_SOUTHWEST;
  else {
    send_to_char( "You do not see that here.\n\r", ch );
    return;
  }

  /* 'look direction' */
  if ( ( pexit = ch->in_room->exit[door] ) == NULL ) {
    send_to_char( "Nothing special there.\n\r", ch );
    return;
  }

  if ( pexit->description != NULL && pexit->description[0] != '\0' )
    send_to_char( pexit->description, ch );
  else
    send_to_char( "Nothing special there.\n\r", ch );

  if ( pexit->keyword    != NULL
       &&   pexit->keyword[0] != '\0'
       &&   pexit->keyword[0] != ' ' ) {
    if ( IS_SET(pexit->exit_info, EX_CLOSED) ) {
      act( "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );
    }
    else if ( IS_SET(pexit->exit_info, EX_ISDOOR) ) {
      act( "The $d is open.",   ch, NULL, pexit->keyword, TO_CHAR );
    }
  }

  return;
}

/* RT added back for the hell of it */
void do_read (CHAR_DATA *ch, const char *argument ) {
  do_look(ch,argument);
}

void do_examine( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  OBJ_DATA *obj;
  CHAR_DATA *vict;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Examine what or whom?\n\r", ch );
    return;
  }

  /* Oxtal */ // Modified by SinaC 2000
  if ( ( vict = get_char_room ( ch, arg ) ) != NULL ) {
    /* One can examine himself */    
    act("You examine $N.",ch,NULL,vict,TO_CHAR);
    act("$n examines you.",ch,NULL,vict,TO_VICT);
    act("$n examines $N.",ch,NULL,vict,TO_NOTVICT);
    do_look( ch, arg );

    if (vict->cstat(race)) {
      sprintf(buf,"$E looks like... %s.", 	     
	      race_table[vict->cstat(race)].name);
      act(buf,ch,NULL,vict,TO_CHAR);
    }
    sprintf(buf,"$E is %s.",size_table[vict->cstat(size)].name);
    act(buf,ch,NULL,vict,TO_CHAR);
  } 
  else if ( ( obj = get_obj_here( ch, arg ) ) != NULL ) {
    act("You examine $p.",ch,obj,NULL,TO_CHAR);
    act("$n examines $p.",ch,obj,NULL,TO_ROOM);
    do_look( ch, arg );
    switch ( obj->item_type ) {
    default:
      break;
  /* Removed by SinaC 2003    
    case ITEM_JUKEBOX:
      do_play(ch,"list");
      break;
  */
    case ITEM_MONEY:
      if (obj->value[0] == 0) {
	if (obj->value[1] == 0)
	  sprintf(buf,"Odd...there's no coins in the pile.\n\r");
	else if (obj->value[1] == 1)
	  sprintf(buf,"Wow. One gold coin.\n\r");
	else
	  sprintf(buf,"There are %d gold coins in the pile.\n\r",
		  obj->value[1]);
      }
      else if (obj->value[1] == 0) {
	if (obj->value[0] == 1)
	  sprintf(buf,"Wow. One silver coin.\n\r");
	else
	  sprintf(buf,"There are %d silver coins in the pile.\n\r",
		  obj->value[0]);
      }
      else
	sprintf(buf,
		"There are %d gold and %d silver coins in the pile.\n\r",
		obj->value[1],obj->value[0]);
      send_to_char(buf,ch);
      break;
      
    case ITEM_DRINK_CON:
    case ITEM_CONTAINER:
    case ITEM_CORPSE_NPC:
    case ITEM_CORPSE_PC:
      sprintf(buf,"in %s",argument);
      do_look( ch, buf );
    }
  }
  else {
    send_to_charf(ch,"You don't see any %s.\n\r", arg );
  }
  return;
}

/*
 * Thanks to Zrin for auto-exit part.
 * Redone by SinaC 2003
 */
void do_exits( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  char buf3[MAX_STRING_LENGTH];
  EXIT_DATA *pExit;
  bool found, fAuto;

  fAuto  = !str_cmp( argument, "auto" );

  if ( !check_blind( ch ) )
    return;
  
  if (fAuto)
    sprintf(buf,"[Exits:");
  else if (IS_IMMORTAL(ch))
    sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
  else
    sprintf(buf,"Obvious exits:\n\r");

  found = FALSE;
  for ( int door = 0; door < MAX_DIR; door++ ) {
    if ( door == DIR_SPECIAL && !IS_IMMORTAL(ch) )  // special dir: dive in an hole
      continue;

    pExit = ch->in_room->exit[door];
    if ( pExit == NULL 
	 || pExit->u1.to_room == NULL 
	 || !can_see_room(ch,pExit->u1.to_room)
	 || ( IS_SET( pExit->exit_info, EX_HIDDEN )
	      && !IS_SET( pExit->exit_info, EX_BASHED )
	      && !IS_AFFECTED2( ch, AFF2_DETECT_EXITS )
	      && ( !IS_NPC(ch) && 
		   !IS_SET( ch->act, PLR_HOLYLIGHT ) ) ) )
      continue;
    found = TRUE;
    if ( fAuto ) {
      if ( door == DIR_SPECIAL ) // special dir: dive in an hole
	strcpy( buf2, "<<dive>>" );
      else {
	buf2[0] = '\0';
	if ( IS_SET( pExit->exit_info, EX_HIDDEN ) && !IS_SET( pExit->exit_info, EX_BASHED ) )
	  strcat( buf2, "[");
	if ( IS_SET( pExit->exit_info, EX_CLOSED ) )
	  strcat( buf2, "(" );
	if ( IS_SET( pExit->exit_info, EX_BASHED ) )
	  strcat( buf2, "{{" );
	
	strcat( buf2, dir_name[door] );
	
	if ( IS_SET( pExit->exit_info, EX_BASHED ) )
	  strcat( buf2, "}" );
	if ( IS_SET( pExit->exit_info, EX_CLOSED ) )
	  strcat( buf2, ")");
	if ( IS_SET( pExit->exit_info, EX_HIDDEN ) && !IS_SET( pExit->exit_info, EX_BASHED ) )
	  strcat( buf2, "]" );
      }
      strcat( buf, " " );
      strcat( buf, buf2 ); 
    }
    else {
      char buf3[512];
      if ( room_is_dark( pExit->u1.to_room ) )
	strcpy(buf3,"Too dark to tell");
      else if ( IS_SET( pExit->exit_info, EX_CLOSED ) )
	strcpy(buf3,"A closed door");
      else
	if ( door == DIR_SPECIAL ) // special dir: dive in an hole
	  strcpy( buf3, "<<dive>>" );
	else
	  strcpy(buf3,pExit->u1.to_room->name);

      sprintf( buf2, "%-5s - %s %s%s%s",
	       capitalize( dir_name[door] ),
	       buf3,
	       IS_SET( pExit->exit_info, EX_CLOSED )?"(DOOR)":"",
	       IS_SET( pExit->exit_info, EX_HIDDEN )?"[HIDDEN]":"",
	       IS_SET( pExit->exit_info, EX_BASHED )?"{{BASHED}":"" );
      if (IS_IMMORTAL(ch)) {
	char tmp[MAX_STRING_LENGTH];
	sprintf( tmp, 
		 " (room %d)\n\r", pExit->u1.to_room->vnum);
	strcat( buf2, tmp );
      }
      else
	strcat( buf2, "\n\r" );
      strcat( buf, buf2 );
    }
  }
  if ( !found )
    strcat( buf, fAuto ? " none" : "None.\n\r" );
  if ( fAuto )
    strcat( buf, "]\n\r" );
  send_to_char( buf, ch );
  return;
}

/* Removed by SinaC 2001
*void do_worth( CHAR_DATA *ch, const char *argument )
*{
*  char buf[MAX_STRING_LENGTH];
*
*  if (IS_NPC(ch)) {
*    sprintf(buf,"You have %ld gold and %ld silver.\n\r",
*	    ch->gold,ch->silver);
*    send_to_char(buf,ch);
*    return;
*  }
*
*  sprintf(buf, 
*	  "You have %ld gold, %ld silver, and %d experience (%d exp to level).\n\r",
*	  ch->gold, ch->silver,ch->exp,
*	  (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
*
*  send_to_char(buf,ch);
*
*  return;
*}
*/

void do_score( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf1[MAX_STRING_LENGTH];
  int i;
  long clbit;

  /* Removed by SinaC 2001
  if ( IS_NPC( ch ) )
    return;
  */

  sprintf( buf, " {C-----------------------------------------------------------\n\r");
  send_to_char( buf , ch );

  sprintf( buf, " ***********************************************************\n\r\n\r{x");
  send_to_char ( buf , ch );
	
  sprintf( buf, "       {RScore Board {Wto %s ({G%s {W){x\n\r\n\r",ch->name,
	   IS_NPC(ch) ? "" : ch->pcdata->title);
  send_to_char( buf , ch );
	
  sprintf( buf, " {C***********************************************************\n\r");
  send_to_char ( buf , ch );

  sprintf( buf, " -----------------------------------------------------------\n\r\n\r{x");
  send_to_char( buf , ch );

  sprintf( buf, "  {MLevel   {Y[{W%-3d{Y]{Y              Str {Y[{W%-2ld{M/{W%-2ld{Y]        {RHp    {Y[{W%-5d{M/{W%-5ld{Y]\n\r",
	   ch->level,
	   ch->cstat(STR),
	   ch->bstat(STR),
	   ch->hit,
	   ch->cstat(max_hit) );
  send_to_char( buf, ch );

  sprintf( buf, "  {MRace    {Y[{W%-14s{Y]{Y   Int {Y[{W%-2ld{M/{W%-2ld{Y]        {RMana  {Y[{W%-5d{M/{W%-5ld{Y]\n\r",
	   race_table[ch->cstat(race)].name,
	   ch->cstat(INT),
	   ch->bstat(INT),
	   ch->mana,
	   ch->cstat(max_mana) );
  send_to_char( buf, ch );

/* Modified by SinaC 2001 for mental user
  sprintf( buf, "  {MAge     {Y[{W%-4d{M/{W%-5dHr{Y]{Y     Wis {Y[{W%-2ld{M/{W%-2ld{Y]        {RMove  {Y[{W%d{M/{W%ld{Y]\n\r",
	   get_age(ch),
	   ( ch->played + (int) (current_time - ch->logon) ) / 3600,
	   ch->cstat(WIS),
	   ch->bstat(WIS),
	   ch->move,
	   ch->cstat(max_move) );
  send_to_char( buf, ch );

  sprintf( buf, "  {MSex     {Y[{W%-12s{Y]{Y     Dex {Y[{W%-2ld{M/{W%-2ld{Y]        {RPrac  {Y[{W%-3d{Y]\n\r",
	   ch->cstat(sex) == 0 ? "sexless" : ch->cstat(sex) == 1 ? "male" : "female",
	   ch->cstat(DEX),
	   ch->bstat(DEX),
	   ch->practice);
  send_to_char( buf, ch );

  sprintf( buf, "  {MClass   {Y[{W%-12s{Y]{Y     Con {Y[{W%-2ld{M/{W%-2ld{Y]        {RTrain {Y[{W%-3d{Y]{W\n\r",
	   IS_NPC(ch) ? "mobile" : class_name(ch->cstat(classes)),
	   ch->cstat(CON),
	   ch->bstat(CON),
	   ch->train);
  send_to_char( buf, ch );
*/

  sprintf( buf, "  {MAge     {Y[{W%-4d{M/{W%-5dHr{Y]{Y     Wis {Y[{W%-2ld{M/{W%-2ld{Y]        {RPsp   {Y[{W%-5d{M/{W%-5ld{Y]\n\r",
	   get_age(ch),
	   ( ch->played + (int) (current_time - ch->logon) ) / 3600,
	   ch->cstat(WIS),
	   ch->bstat(WIS),
	   ch->psp,
	   ch->cstat(max_psp) );
  send_to_char( buf, ch );

  sprintf( buf, "  {MSex     {Y[{W%-12s{Y]{Y     Dex {Y[{W%-2ld{M/{W%-2ld{Y]        {RMove  {Y[{W%-5d{M/{W%-5ld{Y]\n\r",
	   ch->cstat(sex) == 0 ? "sexless" : ch->cstat(sex) == 1 ? "male" : "female",
	   ch->cstat(DEX),
	   ch->bstat(DEX),
	   ch->move,
	   ch->cstat(max_move) );
  send_to_char( buf, ch );

  sprintf( buf, "  {MClass   {Y[{W%-12s{Y]{Y     Con {Y[{W%-2ld{M/{W%-2ld{Y]\n\r",
	   IS_NPC(ch) ? "mobile" : class_name(ch->cstat(classes)),
	   ch->cstat(CON),
	   ch->bstat(CON) );

  send_to_char( buf, ch );

  if ( ch->isWildMagic ) // SinaC 2003
    send_to_charf( ch, "  {MWild Magic\n\r" );

  if (class_ismulti(ch->cstat(classes))) {
    sprintf(buf, "  {MClasses {Y[{W");
    clbit = 1;
    for (i = 0; i<MAX_CLASS;i++) {
      if (clbit & ch->cstat(classes)) {
	sprintf(buf1,"   %s",class_table[i].name);
	strcat(buf,buf1);
      }
      clbit<<=1;
    }
    strcat(buf,"   {Y]{x\n\r");
    send_to_char(buf,ch);
  }
  send_to_char("\n\r", ch );

  sprintf( buf, "  {GItem    {Y[{W%-4d{M/{W%-4d{Y]{G    Weight   {Y[{W%-7ld{M/{W%-7d{Y]{W\n\r",
	   ch->carry_number, 
	   can_carry_n(ch),
	   get_carry_weight(ch), /* was ch->carry_w */ 
	   can_carry_w(ch));
  send_to_char( buf, ch );
        
  if ( ch->level >= 15 ) {
    // Can't figure why I had to put that but doesn't work without it, SinaC 2001
    long hitr, damr;
    hitr = GET_HITROLL(ch);
    damr = GET_DAMROLL(ch);
    send_to_charf( ch, 
		   "  {GHit     {Y[{W%-3ld{Y]{G          Dam      {Y[{W%-3ld{Y]{W\n\r",
		   hitr,damr);
  }
  sprintf( buf, "  {RTrain   {Y[{W%-3d{Y]{W          {RPractice {Y[{W%-3d{Y]\n\r",
	   ch->train, 
	   ch->practice );
  
  send_to_char( buf, ch );

  //  if ( !IS_NPC(ch) ) 
  //    send_to_charf( ch, "  {WQuest   {Y[{W%-5d{Y]{W        Trivia   {Y[{W%-5d{Y]{W\n\r", ch->pcdata->questpoints, ch->pcdata->trivia );
  if ( !IS_NPC(ch) )
    send_to_charf( ch, "  {WTrivia  {Y[{W%-5d{Y]{W\n\r", ch->pcdata->trivia );
	
  sprintf( buf, "  {YGold    [{W%-10ld{Y]{W   {WSilver   {Y[{W%-10ld{Y]{B   Wimpy {Y[{W%-5d{Y]{W\n\r",ch->gold,ch->silver,ch->wimpy);
  send_to_char( buf, ch );

  if ( !IS_IMMORTAL(ch) && !IS_NPC(ch) ) {    
    sprintf( buf, " {M Xp to level {Y[{W%-6d{Y]{M   Total xp {Y[{W%d{Y]\n\r", 
	     ((ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp),
	     ch->exp);
    send_to_char( buf, ch );
  }
                        
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]   > 10 )
    send_to_char( "  {RYou are drunk.\n\r",   ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] ==  0 )
    send_to_char( "  {BYou are thirsty.\n\r", ch );
  if ( !IS_NPC(ch) && ch->pcdata->condition[COND_FULL]   ==  0 )
    send_to_char( "  {CYou are hungry.\n\r",  ch );
 
  switch ( ch->position ) {
  case POS_DEAD:     
    send_to_char( "  {CYou are DEAD{x!!\n\r",		ch );
    break;
  case POS_MORTAL:
    send_to_char( "  You are mortally wounded.\n\r",	ch );
    break;
  case POS_INCAP:
    send_to_char( "  You are incapacitated.\n\r",	ch );
    break;
    // Added by SinaC 2003
  case POS_PARALYZED:
    send_to_char( "  {GYou are paralyzed{x.\n\r",	ch );
    break;
  case POS_STUNNED:
    send_to_char( "  {bYou are stunned{x.\n\r",		ch );
    break;
  case POS_SLEEPING:
    send_to_char( "  {BYou are sleeping{x.\n\r",		ch );
    break;
  case POS_RESTING:
    send_to_char( "  {RYou are resting{x.\n\r",		ch );
    break;
  case POS_STANDING:
    send_to_char( "  {RYou are standing{x.\n\r",		ch );
    break;
  case POS_FIGHTING:
    send_to_char( "  {MYou are fighting{x.\n\r",		ch );
    break;
  }
 
  if ( get_trust( ch ) != ch->level ) {
    sprintf( buf, "  {RYou are trusted at level %d{x.\n\r", get_trust( ch ) );
    send_to_char( buf, ch );
  }
     
  /* print AC values */
  if (ch->level >= 15) {	
    sprintf( buf,"  {BArmor{W: {Gpierce: {W%ld  {Cbash: {W%ld  {Rslash: {W%ld  {Mmagic: {W%ld{x\n\r",
	     GET_AC(ch,AC_PIERCE),
	     GET_AC(ch,AC_BASH),
	     GET_AC(ch,AC_SLASH),
	     GET_AC(ch,AC_EXOTIC));
    send_to_char(buf,ch);
    sprintf( buf, "  {CSaves: %ld{x\n\r\n\r", ch->cstat(saving_throw));
    send_to_char( buf, ch );
  }
 
  for (i = 0; i < 4; i++) {
    const char * temp;
 
    switch(i) {
    case(AC_PIERCE):	temp = "{Gpiercing{x";	break;
    case(AC_BASH):	temp = "{Cbashing{x";	break;
    case(AC_SLASH):	temp = "{Rslashing{x";	break;
    case(AC_EXOTIC):	temp = "{Mmagic{x";	break;
    default:		temp = "{Yerror{x";	break;
    }

    send_to_char("  You are ", ch);

/*
    if      (GET_AC(ch,i) >=  101 )
      sprintf(buf,"{Chopelessly vulnerable to %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 80)
      sprintf(buf,"{Cdefenseless against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 60)
      sprintf(buf,"{Gbarely protected from %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 40)
      sprintf(buf,"{Gslighty armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 20)
      sprintf(buf,"{Rsomewhat armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 0)
      sprintf(buf,"{Rarmored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -20)
      sprintf(buf,"{Mwell-armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -40)
      sprintf(buf,"{Mvery well-armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -60)
      sprintf(buf,"{Bheavily armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -80)
      sprintf(buf,"{Bsuperbly armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -100)
      sprintf(buf,"{Yalmost invulnerable to %s.\n\r",temp);
    else
      sprintf(buf,"{Ydivinely armored against %s.\n\r",temp);
*/
    // Modified by SinaC 2001
    if      (GET_AC(ch,i) >=  101)
      sprintf(buf,"{Chopelessly vulnerable to %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 60)
      sprintf(buf,"{Cdefenseless against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= 20)
      sprintf(buf,"{Gbarely protected from %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -20)
      sprintf(buf,"{Gslighty armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -60)
      sprintf(buf,"{Rsomewhat armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -100)
      sprintf(buf,"{Rarmored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -140)
      sprintf(buf,"{Mwell-armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -180)
      sprintf(buf,"{Mvery well-armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -220)
      sprintf(buf,"{Bheavily armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -260)
      sprintf(buf,"{Bsuperbly armored against %s.\n\r",temp);
    else if (GET_AC(ch,i) >= -300)
      sprintf(buf,"{Yalmost invulnerable to %s.\n\r",temp);
    else
      sprintf(buf,"{Ydivinely armored against %s.\n\r",temp);
    send_to_char(buf,ch);
  }
 
 
  /* RT wizinvis and holy light */
  if ( IS_IMMORTAL(ch)) {
    send_to_char("\n\r  {RHoly Light: ",ch);
    if (IS_SET(ch->act,PLR_HOLYLIGHT))
      send_to_char("{WOn",ch);
    else
      send_to_char("{WOff",ch);
  
    if (ch->invis_level) {
      sprintf( buf, "  {RInvisible: {Wlevel %d",ch->invis_level);
      send_to_char(buf,ch);
    }
      
    if (ch->incog_level) {
      sprintf( buf, "  {RIncognito: {Wlevel %d",ch->incog_level);
      send_to_char(buf,ch);
    }
    send_to_char("\n\r",ch);
  }
  
  // Added by SinaC 2000 to allow Immortals to become mortal
  if ( ch->level >= LEVEL_IMMORTAL ){
    if ( IS_SET(ch->act, PLR_MORTAL ) ) send_to_char("\n\r",ch);
    send_to_charf(ch,
		  "  {RMortal: {W%s{x\n\r",
		  IS_SET( ch->act, PLR_MORTAL )?"Yes":"No");
  }


  // Modified by SinaC 2000
  /*
  if ( ch->level >= 10 ) {
    sprintf( buf, "  {RAlignment: {W%d.  ", ch->align.alignment );
    send_to_char( buf, ch );
  }
  */
  // Modified by SinaC 2001 etho/alignment are attributes now
  // Modified by SinaC 2000 for align/etho
  send_to_charf( ch,
		 "  {RAlignment: {W%s", 
		 //etho_align_name( ch->align.etho, ch->align.alignment ) );
		 etho_align_name( ch->cstat(etho), ch->cstat(alignment) ) );
  if ( ch->level >= 15 )
    //send_to_charf( ch," (%5d).{x\n\r", ch->align.alignment );
    send_to_charf( ch," (%5ld).{x\n\r", ch->cstat(alignment) );
  else
    send_to_charf( ch, ".{x\n\r");
  if ( !IS_NPC(ch) )
    /* Modified by SinaC 2003
    send_to_charf( ch,
		   "  {RYou worship: {W%-20s {Y%s{x\n\r",
		   IS_NPC(ch)?"Mota":god_name( ch->pcdata->god ),
		   ch->pcdata->branded?"[Branded]":"[NOT Branded]");
    */
    //send_to_charf( ch,
    //	   "  {RYou worship: {W%-20s{x\n\r",
    //	   IS_NPC(ch)?"Mota":god_name( ch->pcdata->god ) );
    send_to_charf( ch,
		   "  {RYou worship: {W%-20s{x\n\r", char_god_name( ch ) );
  /*
  send_to_char( "  {xYou are ", ch );
  if ( ch->align.alignment >  900 ) send_to_char( "{Bangelic.{x\n\r", ch );
  else if ( ch->align.alignment >  700 ) send_to_char( "{Bsaintly.{x\n\r", ch );
  else if ( ch->align.alignment >  350 ) send_to_char( "{Rgood.{x\n\r",    ch );
  else if ( ch->align.alignment >  100 ) send_to_char( "{Gkind.{x\n\r",    ch );
  else if ( ch->align.alignment > -100 ) send_to_char( "{Wneutral.{x\n\r", ch );
  else if ( ch->align.alignment > -350 ) send_to_char( "{Ymean.\n\r{x",    ch );
  else if ( ch->align.alignment > -700 ) send_to_char( "{Mevil.\n\r{x",    ch );
  else if ( ch->align.alignment > -900 ) send_to_char( "{Cdemonic.{x\n\r", ch );
  else                             send_to_char( "{Csatanic.{x\n\r", ch );
  send_to_char ( "\n\r", ch );
  */
    
  if (IS_SET(ch->comm,COMM_SHOW_AFFECTS))
    do_affects(ch,"");
     
  return;
}

//void do_affects(CHAR_DATA *ch, const char *argument ) {
//  AFFECT_DATA *paf, *paf_last = NULL;
//  char buf[MAX_STRING_LENGTH];
//    
//  if ( ch->affected != NULL ) {
//    // Modified by SinaC 2003
//    send_to_char( "{cYou are affected by the following spells:{x\n\r", ch );
//    //send_to_char( "-----------------------------------------\n\r", ch );
//    for ( paf = ch->affected; paf != NULL; paf = paf->next ) {
//
//      if (paf_last != NULL && paf->type == paf_last->type)
//	// Modified by SinaC 2001
//	//if (ch->level < 20)
//	//  continue;
//	//else
//	//
//	  // modified by SinaC 2000, modified by SinaC 2001
//	afstring_nospell(buf,paf,ch,FALSE);
//      //afstring(buf, paf);
//      else
//	// Modified by SinaC 2001
//	//if (ch->level < 20)
//	//  sprintf( buf, "%-20s\n\r", ability_table[paf->type].name );
//	//else
//	
//	afstring(buf,paf,ch,FALSE);
//  	  
//      send_to_char(buf,ch);
//      paf_last = paf;
//    }
//  }
//  else 
//    send_to_char("{cYou are not affected by any spells.{x\n\r",ch);
//
//  if ( ch->pet != NULL )
//    affects_pet( ch, ch->pet );
//
//  return;
//}

// SinaC 2003: new affect system
void do_affects(CHAR_DATA *ch, const char *argument ) {
  AFFECT_DATA *paf;
  char buf[MAX_STRING_LENGTH];
    
  if ( ch->affected != NULL ) {
    send_to_char( "{cYou are affected by the following spells:{x\n\r", ch );
    for ( paf = ch->affected; paf != NULL; paf = paf->next ) {
      afstring(buf,paf,ch,TRUE);
      send_to_char(buf,ch);
    }
  }
  else
    send_to_char("{cYou are not affected by any spells.{x\n\r",ch);

  if ( ch->pet != NULL )
    affects_pet( ch, ch->pet );

  return;
}

//void affects_pet( CHAR_DATA *ch, CHAR_DATA *pet ) {
//  AFFECT_DATA *paf, *paf_last = NULL;
//  char buf[MAX_STRING_LENGTH];
//    
//  if ( pet->affected != NULL ) {
//    // Modified by SinaC 2003
//    send_to_char( "{cYour pet is affected by the following spells:{x\n\r", ch );
//    //send_to_char( "---------------------------------------------\n\r", ch );
//    for ( paf = pet->affected; paf != NULL; paf = paf->next ) {
//
//      if (paf_last != NULL && paf->type == paf_last->type)
//	/*
//	if (ch->level < 20)
//	  continue;
//	else
//	  // modified by SinaC 2000, modified by SinaC 2001
//	  */
//	  afstring_nospell(buf,paf,ch,FALSE);
//      //afstring(buf, paf);
//      else	
//	/*
//	if (ch->level < 20)
//	  sprintf( buf, "%-20s\n\r", ability_table[paf->type].name );
//	else
//	*/
//	  //Modified by SinaC 2001
//	  afstring(buf,paf,ch,FALSE);
//  	  
//      send_to_char(buf,ch);
//      paf_last = paf;
//    }
//  }
//  else 
//    send_to_char("{cYour pet is not affected by any spells.{x\n\r",ch);
//
//  return;
//}

// SinaC 2003: new affect system
void affects_pet( CHAR_DATA *ch, CHAR_DATA *pet ) {
  AFFECT_DATA *paf;
  char buf[MAX_STRING_LENGTH];
    
  if ( pet->affected != NULL ) {
    send_to_char( "{cYour pet is affected by the following spells:{x\n\r", ch );
    for ( paf = pet->affected; paf != NULL; paf = paf->next ) {
      afstring(buf,paf,ch,TRUE);
      send_to_char(buf,ch);
    }
  }
  else 
    send_to_char("{cYour pet is not affected by any spells.{x\n\r",ch);

  return;
}

// Added by SinaC 2000
//void show_oaffects( CHAR_DATA *ch, CHAR_DATA *victim, bool immortal ) {
//  AFFECT_DATA *paf;
//  char buf[MAX_INPUT_LENGTH];
//  char item_name[MAX_INPUT_LENGTH];
//  char list_aff[MAX_STRING_LENGTH];
//  OBJ_DATA *obj;
//  int iWear;
//  bool foundA, found;
//  BUFFER *buffer;
//
//  buffer = new_buf();
//
//  found = FALSE;
//  for ( iWear = 0; iWear < MAX_WEAR; iWear++ ) {
//    list_aff[0] = '\0';
//    foundA = FALSE;
//    if ( ( obj = get_eq_char( victim, iWear ) ) == NULL )
//      continue;
//	
//    if ( immortal == TRUE || ch->level >= 20 )
//      if ( can_see_obj( ch, obj ) )
//	sprintf( item_name, "  %s :\n\r", format_obj_to_char( obj, ch, TRUE ) );
//      else
//	strcpy( item_name, "  Something :\n\r" );
//
//    if (!obj->enchanted) {
//      for ( paf = obj->pIndexData->affected; paf != NULL; paf = paf->next )
//	if (attr_table[paf->location].bits != NULL) {
//	  // modified by SinaC 2001
//	  afstring(buf,paf,ch,FALSE);
//	  strcat( list_aff, buf );
//	  foundA = TRUE;
//	}
//      // Added by SinaC 2003 to see items giving skill bonus
//      ABILITY_UPGRADE *upgr;
//      for ( upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next ) {
//	abilityupgradestring(buf,upgr);
//	strcat( list_aff, buf );
//	foundA = TRUE;
//      }
//    }
//    else
//      for ( paf = obj->affected; paf != NULL; paf = paf->next )
//	if (attr_table[paf->location].bits != NULL) {
//          //Modified by SinaC 2001
//	  afstring(buf,paf,ch,FALSE);
//	  strcat( list_aff, buf );
//	  foundA = TRUE;
//	}
//    // if there is an affect on the object
//    if (foundA) {
//      add_buf( buffer, item_name );
//      add_buf( buffer, list_aff );
//      found = TRUE; 
//    }
//  }
//  if (found) {
//    if ( victim == ch )
//      send_to_char( "Your equipement gives you the following affects:\n\r", ch );
//    else
//      send_to_charf( ch, "%s equipement gives him/her the following affects:\n\r", NAME(victim) );
//    send_to_char( "------------------------------------------------\n\r", ch );
//    page_to_char( buf_string( buffer ), ch );
//  }
//  else
//    if ( victim == ch )
//      send_to_char( "You don't wear any items giving you affects.\n\r", ch );
//    else
//      send_to_charf( ch, "%s doesn't wear any items giving him/her affects.\n\r", NAME(victim) );
//
//}

// SinaC 2003: new affect system
void show_oaffects( CHAR_DATA *ch, CHAR_DATA *victim, bool immortal ) {
  char buf[MAX_INPUT_LENGTH];
  bool found;
  BUFFER *buffer;

  buffer = new_buf();

  found = FALSE;
  for ( OBJ_DATA *obj = ch->carrying; obj != NULL; obj = obj->next_content ) {
    if ( obj == NULL || !obj->valid // item not valid
	 || obj->wear_loc == -1  ) // item not worn
      continue;
  //  for ( int iWear = 0; iWear < MAX_WEAR; iWear++ ) {
    char item_name[MAX_INPUT_LENGTH];
    char list_aff[MAX_STRING_LENGTH];
    list_aff[0] = '\0';
    item_name[0] = '\0';
    bool foundA = FALSE;
    //    OBJ_DATA *obj;
    //    if ( ( obj = get_eq_char( victim, iWear ) ) == NULL )
    //      continue;
	
    if ( immortal == TRUE || ch->level >= 20 )
      if ( can_see_obj( ch, obj ) )
	sprintf( item_name, "  %s :\n\r", format_obj_to_char( obj, ch, TRUE ) );
      else
	strcpy( item_name, "  Something :\n\r" );

    for ( AFFECT_DATA *paf = obj->affected; paf != NULL; paf = paf->next ) {
      afstring(buf,paf,ch,TRUE);
      strcat( list_aff, buf );
      foundA = TRUE;
    }
    //    for ( ABILITY_UPGRADE *upgr = obj->upgrade; upgr != NULL; upgr = upgr->next ) {
    //      abilityupgradestring(buf,upgr);
    //      strcat( list_aff, buf );
    //      foundA = TRUE;
    //    }
    if (!obj->enchanted) {
      for ( AFFECT_DATA *paf = obj->pIndexData->affected; paf != NULL; paf = paf->next ) {
	afstring(buf,paf,ch,TRUE);
	strcat( list_aff, buf );
	foundA = TRUE;
      }
    }
    for ( ABILITY_UPGRADE *upgr = obj->pIndexData->upgrade; upgr != NULL; upgr = upgr->next ) {
      abilityupgradestring(buf,upgr);
      strcat( list_aff, buf );
      foundA = TRUE;
    }
    //    }
    // if there is an affect/upgrade on the object
    if (foundA) {
      add_buf( buffer, item_name );
      add_buf( buffer, list_aff );
      found = TRUE; 
    }
  }
  if (found) {
    if ( victim == ch )
      send_to_char( "Your equipement gives you the following affects:\n\r", ch );
    else
      send_to_charf( ch, "%s equipement gives him/her the following affects:\n\r", NAME(victim) );
    send_to_char( "------------------------------------------------\n\r", ch );
    page_to_char( buf_string( buffer ), ch );
  }
  else
    if ( victim == ch )
      send_to_char( "You don't wear any items giving you affects.\n\r", ch );
    else
      send_to_charf( ch, "%s doesn't wear any items giving him/her affects.\n\r", NAME(victim) );

}

void do_time( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  const char *suf;
  int day;

  day     = time_info.day + 1;

  if ( day > 4 && day <  20 ) suf = "th";
  else if ( day % 10 ==  1       ) suf = "st";
  else if ( day % 10 ==  2       ) suf = "nd";
  else if ( day % 10 ==  3       ) suf = "rd";
  else                             suf = "th";

  // SinaC 2003: year
  sprintf( buf,
	   "It is %d o'clock %s, Day of %s, %d%s the Month of %s, Year %d.\n\r",
	   (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
	   time_info.hour >= 12 ? "pm" : "am",
	   day_name[day % NBR_DAYS_IN_WEEK],
	   day, suf,
	   month_name[time_info.month],
	   time_info.year);
  send_to_char(buf,ch);
  sprintf(buf,"ROM started up at %s\n\rThe system time is %s\n\r",
	  str_boot_time,
	  (char *) ctime( &current_time )
	  );

  send_to_char( buf, ch );
  return;
}

void do_weather( CHAR_DATA *ch, const char *argument ) {
  if ( !IS_OUTSIDE(ch) && !IS_IMMORTAL(ch) ) {
    send_to_char( "You can't see the weather indoors.\n\r", ch );
    return;
  }

  send_to_charf( ch, "The sky is %s and %s.\n\r",
		 sky_look[weather_info.sky],
		 weather_info.change >= 0
		 ? "a warm southerly breeze blows"
		 : "a cold northern gust blows"
		 );

  if (moon_night()) {
    int i;
    for (i=0;i<NBR_MOON;i++)
      if (moon_visible(i))
	send_to_charf(ch, moon_phase_msg[moon_phase(i)], moon_info[i].name);
  }

  // show moons info if IMMORTAL, Oxtal
  if (IS_IMMORTAL(ch)) {
    int i;
    send_to_char(" name  pos phase sky? phase_nbr\n\r",ch);
    for (i=0;i<NBR_MOON;i++)
      send_to_charf(ch,         
		    "%6s %3d %5d %4d %d\n\r",
		    moon_info[i].name, moon_info[i].po_t, moon_info[i].ph_t,
		    moon_insky(i), moon_phase(i));
  }
  return;
}

/* Removed by SinaC 2001, replaced with a mobprogram
* BANKS: do_account, do_deposit, do_withdraw. Seytan 1997 modified by Sinac
*void do_account( CHAR_DATA *ch, const char *argument )
*{
*  char buf[MAX_STRING_LENGTH];
*
*  if(IS_NPC(ch)) {
*    send_to_char("Mobs have no accounts.\n\r",ch);
*    return;
*  }
*
*  sprintf(buf,"You have %ld gold, %ld silver on you and %ld gold, %ld silver in the bank\r\n",ch->gold,ch->silver,
*	  ch->pcdata->acctgold, ch->pcdata->acctsilver);
*  send_to_char(buf,ch);    
*}
*
*void do_deposit( CHAR_DATA *ch, const char *argument )
*{
*  char arg1[MAX_INPUT_LENGTH];
*  char arg2[MAX_INPUT_LENGTH];
*  char buf[MAX_STRING_LENGTH];
*  long amount;
*  bool gold;
*  
*  argument = one_argument( argument, arg1 );
*  argument = one_argument( argument, arg2 );
* 
*  if ( IS_NPC( ch ) ) {
*    send_to_char( "Mobs don't have an account.\n\r", ch );
*    return;
*  } 
* 
*  if ( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) ) {
*    send_to_char( "You are not in a bank.\n\r", ch );
*    return;
*  }
* 
*  if ( arg1[0] == '\0' ) {
*    send_to_char( "You must specify an amount.\n\r", ch );
*    return;    
*  }
*  if ( ( str_cmp( arg2, "gold" ) 
*	 && str_cmp( arg2, "silver" ) ) 
*       || ( arg2[0] == '\0' ) ) {
*    send_to_char( "You must specify gold or silver.\n\r", ch );
*    return;
*  }
*  if ( !str_cmp( arg2, "gold" ) ) gold = TRUE;
*  else gold = FALSE;
*  amount = atoi( arg1 );
*  if ( amount <= 0 && str_cmp( arg1, "all" ) ) {
*    send_to_char( "You can't do that. Amount must be numerical, >= 0 or ALL.\n\r", ch );
*    return;
*  }
*  if ( !str_cmp( arg1, "all" ) ) {
*    sprintf( buf, "You deposit all your " );
*    if ( gold )
*      amount = ch->gold;
*    else
*      amount = ch->silver;
*    if ( amount == 0 ) {
*      if ( gold )
*	send_to_char( "You don't have any gold coins.\n\r", ch );
*      else
*	send_to_char( "You don't have any silver coins.\n\r", ch );  
*      return;  
*    }  
*  }
*  else
*    sprintf( buf, "You deposit %ld ", amount );
*  if ( gold ) {
*    if ( amount > ch->gold ) {
*      send_to_char( "You don't have this amount of gold coins.\n\r", ch );
*      return;
*    }
*    if ( amount > 1 ) 
*      strcat( buf, "gold coins.\n\r" );
*    else 
*      strcat( buf, "gold coin.\n\r" );
*    ch->gold-=amount;
*    ch->pcdata->acctgold+=amount;
*  }
*  else {
*    if ( amount > ch->silver ) {
*      send_to_char( "You don't have this amount of silver coins.\n\r", ch );
*      return;
*    }
*    if ( amount > 1 ) strcat( buf, "silver coins.\n\r" );
*    else strcat( buf, "silver coin.\n\r" );
*    ch->silver-=amount;
*    ch->pcdata->acctsilver+=amount;
*  }  
*  send_to_char( buf, ch );
*}
*
*void do_withdraw( CHAR_DATA *ch, const char *argument )
*{
*  char arg1[MAX_INPUT_LENGTH];
*  char arg2[MAX_INPUT_LENGTH];
*  char buf[MAX_STRING_LENGTH];
*  long amount;
*  bool gold;
*  
*  argument = one_argument( argument, arg1 );
*  argument = one_argument( argument, arg2 );
*
*  if ( IS_NPC( ch ) ) {
*    send_to_char( "Mobs don't have an account.\n\r", ch );
*    return;
*  }
*  
*  if ( !IS_SET( ch->in_room->room_flags, ROOM_BANK ) ) {
*    send_to_char( "You are not in a bank.\n\r", ch );
*    return;
*  }
*
*  if ( arg1[0] == '\0' ) {
*    send_to_char( "You must specify an amount.\n\r", ch );
*    return;    
*  }
*  if ( ( str_cmp( arg2, "gold" ) 
*	 && str_cmp( arg2, "silver" ) ) 
*       || ( arg2[0] == '\0' ) ) {
*    send_to_char( "You must specify gold or silver.\n\r", ch );
*    return;
*  }
*  if ( !str_cmp( arg2, "gold" ) ) gold = TRUE;
*  else gold = FALSE;
*  amount = atoi( arg1 );
*  if ( amount <= 0 && str_cmp( arg1, "all" ) ) {
*    send_to_char( "You can't do that. Amount must be numerical, >= 0 or ALL.\n\r", ch );
*    return;
*  }
*  if ( !str_cmp( arg1, "all" ) ) {
*    sprintf( buf, "You withdraw all your " );
*    if ( gold )
*      amount = ch->pcdata->acctgold;
*    else
*      amount = ch->pcdata->acctsilver;
*    if ( amount == 0 ) {
*      if ( gold )
*	send_to_char( "You don't have any gold coins in the bank.\n\r", ch );
*      else
*	send_to_char( "You don't have any silver coins in the bank.\n\r", ch );  
*      return;  
*    }
*  }
*  else
*    sprintf( buf, "You withdraw %ld ", amount );
*  if ( gold ) {
*    if ( amount > ch->pcdata->acctgold ) {
*      send_to_char( "You don't have this amount of gold coins in the bank.\n\r", ch );
*      return;
*    }
*    if ( amount > 1 ) 
*      strcat( buf, "gold coins.\n\r" );
*    else 
*      strcat( buf, "gold coin.\n\r" );
*    ch->gold+=amount;
*    ch->pcdata->acctgold-=amount;
*  }
*  else {
*    if ( amount > ch->pcdata->acctsilver ) {
*      send_to_char( "You don't have this amount of silver coins in the bank.\n\r", ch );
*      return;
*    }
*    if ( amount > 1 ) 
*      strcat( buf, "silver coins.\n\r" );
*    else 
*      strcat( buf, "silver coin.\n\r" );
*    ch->silver+=amount;
*    ch->pcdata->acctsilver-=amount;
*  }  
*  send_to_char( buf, ch );
*}
* End BANK CODE
*/

void do_help( CHAR_DATA *ch, const char *argument ) {
  HELP_DATA *pHelp;
  BUFFER *output;
  bool found = FALSE;
  char argall[MAX_INPUT_LENGTH],argone[MAX_INPUT_LENGTH];
  int level;
  int i;

  output = new_buf();

  if ( argument[0] == '\0' )
    argument = "summary";

  if ( strlen(argument) <= 1 ) {
    send_to_char("help <topic>   <topic> must contain at least 2 characters.\n\r", ch);
    return;
  }

  strcpy( argone, argument );
  strip_char( argone, '\'' ); // remove quote
  for ( i = 0; i < MAX_PC_RACE; i++ ) {
    if (LOWER(argone[0]) == LOWER(race_table[i].name[0])
	&&  !str_prefix( argone,race_table[i].name)) {
    if ( found )
      add_buf(output,
	      "\n\r============================================================\n\r\n\r");
      racehelp(i, output );
      found = TRUE;
      //      break;
    }
  }
  //if ( ( i = class_lookup(argone) ) >= 0 ) {
  for ( i = 0; i < MAX_CLASS; i++ ) {
    if (LOWER(argone[0]) == LOWER(class_table[i].name[0])
	&&  !str_prefix( argone,class_table[i].name)) {
    if ( found )
      add_buf(output,
	      "\n\r============================================================\n\r\n\r");
    classhelp(i, output );
    found = TRUE;
    }
  }
  for ( i = 1; i < MAX_ABILITY; i++ ) {
    if ( ability_table[i].name == NULL ) break;
    if ( LOWER(argone[0]) == LOWER(ability_table[i].name[0])
	 &&   !str_prefix( argone, ability_table[i].name ) ) {
    if ( found )
      add_buf(output,
	      "\n\r============================================================\n\r\n\r");
      abilityhelp(i, output );
      found = TRUE;
      //      break;
    }
  }
  // SinaC 2003 for triggers
  TriggerDescr *t = isTrigger( argone, FALSE );
  if ( t != NULL ) {
    if ( found )
      add_buf(output,
	      "\n\r============================================================\n\r\n\r");
    triggerHelp( t, output );
    found = TRUE;
  }
  // SinaC 2003 for god
  int godId = god_lookup( argone );
  if ( godId >= 0 ) {
    if ( found )
      add_buf(output,
	      "\n\r============================================================\n\r\n\r");
    godHelp( godId, output );
    found = TRUE;
  }

//--  one_argument(argument, argone );
//--  for ( i = 0; i < MAX_PC_RACE; i++) {
//--    if (LOWER(argone[0]) == LOWER(race_table[i].name[0])
//--	&&  !str_prefix( argone,race_table[i].name)) {
//--      if ( found )
//--	add_buf(output,
//--		      "\n\r============================================================\n\r\n\r");
//--      racehelp(i, output );
//--      found = TRUE;
//--    }
//--  }
//--  if ( ( i = class_lookup(argone) ) >= 0 ) {
//--    if ( found )
//--      add_buf(output,
//--	      "\n\r============================================================\n\r\n\r");
//--    classhelp(i, output );
//--    found = TRUE;
//--  }
//--  for ( i = 1; i < MAX_ABILITY; i++ ) {
//--    if ( ability_table[i].name == NULL ) break;
//--    if ( LOWER(argone[0]) == LOWER(ability_table[i].name[0])
//--	 &&   !str_prefix( argone, ability_table[i].name ) ) {
//--      if ( found )
//--	add_buf(output,
//--		"\n\r============================================================\n\r\n\r");
//--      abilityhelp(i, output );
//--      found = TRUE;
//--    }
//--  }


  /* this parts handles help a b so that it returns help 'a b' */
  argall[0] = '\0';
  while (argument[0] != '\0' ) {
    argument = one_argument(argument,argone);
    if (argall[0] != '\0')
      strcat(argall," ");
    strcat(argall,argone);
  }

  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next ) {
    level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

    if (level > get_trust( ch ) )
      continue;

    if ( is_name( argall, pHelp->keyword ) ) {
      /* add seperator if found */
      if (found)
	add_buf(output,
		"\n\r============================================================\n\r\n\r");
      if ( pHelp->level >= 0 && str_cmp( argall, "imotd" ) ) {
	add_buf(output,pHelp->keyword);
	add_buf(output,"\n\r");
      }

      /*
       * Strip leading '.' to allow initial blanks.
       */
      if ( pHelp->text[0] == '.' )
	add_buf(output,pHelp->text+1);
      else
	add_buf(output,pHelp->text);
      found = TRUE;
      /* small hack :) */
      /*
      if (ch->desc != NULL && ch->desc->connected != CON_PLAYING 
	  &&  		    ch->desc->connected != CON_GEN_GROUPS)
	break;
	*/
    }
  }

  if (!found)
    send_to_char( "No help on that word.\n\r", ch );
  else
    page_to_char(buf_string(output),ch);
}

/* whois command */
void do_whois (CHAR_DATA *ch, const char *argument) {
  char arg[MAX_INPUT_LENGTH];
  BUFFER *output;
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *d;
  bool found = FALSE;

  int i;
  long clbit;

  one_argument(argument,arg);
  
  if (arg[0] == '\0') {
    send_to_char("You must provide a name.\n\r",ch);
    return;
  }

  output = new_buf();

  for (d = descriptor_list; d != NULL; d = d->next) {
    CHAR_DATA *wch;
    char const *cla;
      
    // Modified by SinaC 2001
    //if (d->connected != CON_PLAYING || !can_see(ch,d->character))
    if (d->connected != CON_PLAYING || !can_see_ooc(ch,d->character))
      continue;
      
    wch = ( d->original != NULL ) ? d->original : d->character;
    
    // Modified by SinaC 2001
    //if (!can_see(ch,wch))
    if (!can_see_ooc(ch,wch))
      continue;
      
    if (!str_prefix(arg,wch->name)) {
      found = TRUE;
	  
      /* work out the printing */
      cla = class_whoname(wch->cstat(classes));
      switch(wch->level) {
      case MAX_LEVEL - 0 : cla = "IMP"; break;
      case MAX_LEVEL - 1 : cla = "CRE";	break;
      case MAX_LEVEL - 2 : cla = "SUP";	break;
      case MAX_LEVEL - 3 : cla = "DEI";	break;
      case MAX_LEVEL - 4 : cla = "GOD";	break;
      case MAX_LEVEL - 5 : cla = "IMM";	break;
      case MAX_LEVEL - 6 : cla = "DEM";	break;
      case MAX_LEVEL - 7 : cla = "ANG";	break;
      case MAX_LEVEL - 8 : cla = "AVA";	break;
      }
	  
      // Modified by SinaC 2001 for name acceptance
      /* a little formatting */
      char race[10];
      if ( race_table[wch->cstat(race)].pc_race )
	strcpy( race, pc_race_table[wch->cstat(race)].who_name ); // PC race
      else {
	strncpy( race, race_table[wch->cstat(race)].name, 5 ); // NPC race
	race[5] = '\0';
      }
      sprintf(buf, "\n\r%s[%2d {y%6s {x%s] %s%s%s%s%s%s%s %s%s ",
	      wch->pcdata->name_accepted?"":"{c[NEWBIE]{x",
	      wch->level,
	      //wch->cstat(race) < MAX_PC_RACE ? pc_race_table[wch->cstat(race)].who_name
	      //: "     ",
	      race,
	      cla,
	      IS_SET(wch->comm, COMM_AFK) ? "{G[AFK]{x " : "",
	      IS_SET(wch->comm, COMM_BUILDING) ? "{b[BUILDING]{x " : "",
  // SinaC 2003, same as COMM_BUILDING but editing datas
	      IS_SET(wch->comm, COMM_EDITING) ? "{B[EDITING]{x " : "",
	      wch->incog_level >= LEVEL_HERO ? "{y(Incog){x ": "",
	      wch->invis_level >= LEVEL_HERO ? "{y(Wizi){x " : "",
	      wch->name, 
	      IS_NPC(wch) ? "" : wch->pcdata->title,
	      IS_SET(wch->act,PLR_KILLER) ? "{r(KILLER){x " : "",
	      IS_SET(wch->act,PLR_THIEF) ? "{r(THIEF){x " : ""
	      );
      add_buf(output,buf);

      if ( wch->isWildMagic ) // SinaC 2003
	add_buf(output,"\n\r{MWild Magic{x");
	  
      if (is_clan(wch)) {
	// Modified by SinaC 2000
	sprintf(buf,"\n\r{rClan{x :   %s of %s",
		lookup_clan_status(wch->clan_status),
		get_clan_table(wch->clan)->name
		);
	add_buf(output,buf);
      }
	  
      sprintf(buf,"{w\n\r");
      add_buf(output,buf);
	  
      if (IS_IMMORTAL(ch) || wch == ch) {
	//sprintf(buf,"{mTrivia points:{x [%d]   {mQuest points:{x [%d]\n\r",
	//		wch->pcdata->trivia,wch->pcdata->questpoints);	    
	sprintf(buf,"{mTrivia points:{x [%d]\n\r",
		wch->pcdata->trivia);	    
	add_buf(output,buf);
      }
	  
      if (class_ismulti(wch->cstat(classes))) {
	sprintf(buf, "{cClasses:{x");
	add_buf(output,buf);
	clbit = 1;
	for (i = 0; i<MAX_CLASS;i++) {
	  if (clbit & wch->cstat(classes)) {
	    sprintf(buf,"   %s",class_table[i].name);
	    add_buf(output,buf);
	  }
	  clbit<<=1;
	}
      }
      else {
	sprintf(buf,"{cClass:{x %s", class_table[class_firstclass(wch->cstat(classes))].name);
	add_buf(output,buf);
      }
      add_buf(output, "\n\r" );

      // Added by SinaC 2001
      sprintf( buf, 
	       "{yGod:{x %s\n\r",
	       god_name( wch->pcdata->god ) );
      add_buf(output, buf );
      
    }
  }
    
  if (!found) {
    send_to_char("No one of that name is playing.\n\r",ch);
    return;
  }
    
  page_to_char(buf_string(output),ch);
}


/*
 * New 'who' command originally by Alander of Rivers of Mud.
 */

int max_players = 0;

void do_who( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  BUFFER *output;
  DESCRIPTOR_DATA *d;
  int iClass;
  int iRace;
  int iClan;
  int iLevelLower;
  int iLevelUpper;
  int nNumber;
  int nMatch;
  long rgfClass; /*now it's a set of bits / Oxtal */
  bool rgfRace[MAX_PC_RACE];
  bool rgfClan[UPPER_MAX_CLAN];
  bool fClassRestrict = FALSE;
  bool fClanRestrict = FALSE;
  bool fClan = FALSE;
  bool fRaceRestrict = FALSE;
  bool fImmortalOnly = FALSE;
  bool fWildMagic = FALSE;
 
  /*
   * Set default arguments.
   */
  iLevelLower    = 0;
  iLevelUpper    = MAX_LEVEL;
  rgfClass = 0;
  for ( iRace = 0; iRace < MAX_PC_RACE; iRace++ )
    rgfRace[iRace] = FALSE;
  for (iClan = 0; iClan < UPPER_MAX_CLAN; iClan++)
    rgfClan[iClan] = FALSE;
 
  /*
   * Parse arguments.
   */
  nNumber = 0;
  for ( ;; ) {
    char arg[MAX_STRING_LENGTH];
    
    argument = one_argument( argument, arg );
    if ( arg[0] == '\0' )
      break;
    
    if ( is_number( arg ) ) {
      switch ( ++nNumber )
	{
	case 1: iLevelLower = atoi( arg ); break;
	case 2: iLevelUpper = atoi( arg ); break;
	default:
	  send_to_char( "Only two level numbers allowed.\n\r", ch );
	  return;
	}
    }
    else {
      
      /*
       * Look for classes to turn on.
       */
      if (!str_prefix(arg,"immortals")) {
	fImmortalOnly = TRUE;
      }
      else if (!str_prefix(arg,"wild")) {
	fWildMagic = TRUE;
      }
      else  {
	iClass = class_lookup(arg);
	if (iClass == -1) {
	  iRace = race_lookup(arg);
	  
	  // Modified by SinaC 2000
	  //if (iRace == 0 || iRace >= MAX_PC_RACE) {
	  if (iRace < 0 || iRace > MAX_PC_RACE) {
	    if (!str_prefix(arg,"clan"))
	      fClan = TRUE;
	    else {
	      iClan = clan_lookup(arg);
	      if (iClan) {
		fClanRestrict = TRUE;
		rgfClan[iClan] = TRUE;
	      }
	      else {
		send_to_char(
			     "That's not a valid race, class, or clan.\n\r",
			     ch);
		return;
	      }
	    }
	  }
	  else {
	    fRaceRestrict = TRUE;
	    rgfRace[iRace] = TRUE;
	  }
	}
	else {
	  fClassRestrict = TRUE;
	  rgfClass|= 1 << iClass;
	}
      }
    }
  }
  
  /*
   * Now show matching chars.
   */
  nMatch = 0;
  buf[0] = '\0';
  output = new_buf();
  for ( d = descriptor_list; d != NULL; d = d->next ) {
    CHAR_DATA *wch;
    char const *cla;
 
    /*
     * Check for match against restrictions.
     * Don't use trust as that exposes trusted mortals.
     */
    if ( (d->connected != CON_PLAYING 
	  && (d->connected < CON_NOTE_TO 
	      || d->connected > CON_NOTE_FINISH) ) 
	 //|| !can_see( ch, d->character ) )
	 // Modified by SinaC 2001
	 || !can_see_ooc( ch, d->character ) )
      continue;
 
    wch   = ( d->original != NULL ) ? d->original : d->character;

    // Modified by SinaC 2001
    //if (!can_see(ch,wch))
    if (!can_see_ooc(ch,wch))
      continue;

    if ( 
	wch->level < iLevelLower
	|| wch->level > iLevelUpper
	|| ( fImmortalOnly  && !IS_IMMORTAL(wch) )
	|| ( fClassRestrict && !IS_SET( wch->bstat(classes), rgfClass ) )
	|| ( fRaceRestrict && !rgfRace[wch->bstat(race)])
	|| ( fClan && wch->clan)
	|| ( fClanRestrict && !rgfClan[wch->clan])
	|| ( fWildMagic && !wch->isWildMagic ) // SinaC 2003
	)
      continue;

    nMatch++;
 
    /*
     * Figure out what to print for class.
     */
    cla = class_whoname(wch->bstat(classes)); // no information about wild magic
    switch ( wch->level ) {
    default: break;
    case MAX_LEVEL - 0 : cla = "      IMP     ";     break;
    case MAX_LEVEL - 1 : cla = "      CRE     ";     break;
    case MAX_LEVEL - 2 : cla = "      SUP     ";     break;
    case MAX_LEVEL - 3 : cla = "      DEI     ";     break;
    case MAX_LEVEL - 4 : cla = "      GOD     ";     break;
    case MAX_LEVEL - 5 : cla = "      IMM     ";     break;
    case MAX_LEVEL - 6 : cla = "      DEM     ";     break;
    case MAX_LEVEL - 7 : cla = "      ANG     ";     break;
    case MAX_LEVEL - 8 : cla = "      AVA     ";     break;
    case MAX_LEVEL - 9 : cla = "     {cHERO{x     ";     break;
    }
    // Modified by SinaC 2000
    if ( wch->pcdata->immtitle!=NULL 
	 && wch->pcdata->immtitle[0]!='\0')
      cla = wch->pcdata->immtitle;
    /*
     * Format it up.
     */
    if(wch->level>MAX_LEVEL-10)
      sprintf( buf, "{y[{x%s{y]{x %s%s%s%s%s%s%s%s %s%s %s\n\r",
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
	       IS_NPC(wch) ? "" : wch->pcdata->title,
	       get_clan_table(wch->clan)->who_name);
    else {
      char race[10];
      if ( race_table[wch->cstat(race)].pc_race )
	strcpy( race, pc_race_table[wch->cstat(race)].who_name ); // PC race
      else {
	strncpy( race, race_table[wch->cstat(race)].name, 5 ); // NPC race
	race[5] = '\0';
      }
      // Modified by SinaC 2001 for name acceptance
      //sprintf( buf, "{y[{x%3d %6s %s{y]{x %s%s%s%s%s%s %s%s %s\n\r",
      sprintf( buf, "{y[{x%3d %6s %s{y]{x %s%s%s%s%s%s%s%s%s %s%s %s\n\r",
	       wch->level, // Why bstat ? SinaC 2003
	       //wch->bstat(race) < MAX_PC_RACE ? pc_race_table[wch->bstat(race)].who_name
	       //: "     ",
	       race,
	       cla,
	       // Added by SinaC 2001
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
	       IS_NPC(wch) ? "" : wch->pcdata->title,
	       get_clan_table(wch->clan)->who_name);
    }
    add_buf(output,buf);
  }

  if (max_players<nMatch) max_players=nMatch;
  sprintf( buf2, "\n\rPlayers found: %d - Max today: %d\n\r", nMatch,max_players );
  add_buf(output,buf2);
  page_to_char( buf_string(output), ch );
  return;
}

void do_count( CHAR_DATA *ch, const char *argument ) {
  int count;
  DESCRIPTOR_DATA *d;
  char buf[MAX_STRING_LENGTH];

  count = 0;

  for ( d = descriptor_list; d != NULL; d = d->next )
    if ( d->connected == CON_PLAYING 
	 // Modified by SinaC 2001
	 //&& can_see( ch, d->character ) )
	 && can_see_ooc( ch, d->character ) )
      count++;

  max_on = UMAX(count,max_on);

  if (max_on == count)
    sprintf(buf,"There are %d characters on, the most so far today.\n\r",
	    count);
  else
    sprintf(buf,"There are %d characters on, the most on today was %d.\n\r",
	    count,max_on);

  send_to_char(buf,ch);
}

void do_inventory( CHAR_DATA *ch, const char *argument ) {
  send_to_char( "You are carrying:\n\r", ch );
  show_list_to_char( ch->carrying, ch, TRUE, TRUE );
  return;
}

void do_equipment( CHAR_DATA *ch, const char *argument ) {
  OBJ_DATA *obj;
  int iWear;
  bool found;

  send_to_char( "You are using:\n\r", ch );
  found = FALSE;
  for ( iWear = 0; iWear < MAX_WEAR; iWear++ ) {
    int mapped = mapping_for_nicer_output[iWear]; // SinaC 2003
    //if ( ( obj = get_eq_char( ch, iWear ) ) == NULL )
    if ( ( obj = get_eq_char( ch, mapped ) ) == NULL )
      continue;
   
    //send_to_char( where_name[iWear], ch );
    send_to_char( where_name[mapped], ch );
    if ( can_see_obj( ch, obj ) ) {
      send_to_char( format_obj_to_char( obj, ch, TRUE ), ch );
      send_to_char( "\n\r", ch );
    }
    else {
      send_to_char( "something.\n\r", ch );
    }
    found = TRUE;
  }
  
  if ( !found )
    send_to_char( "Nothing.\n\r", ch );

  return;
}

void do_compare( CHAR_DATA *ch, const char *argument ) {
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  OBJ_DATA *obj1;
  OBJ_DATA *obj2;
  int value1;
  int value2;
  const char *msg;

  if ( IS_NPC( ch ) ) {
    send_to_char("Mobiles can't use this command!\n\r",ch);
    return;
  }

  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );
  if ( arg1[0] == '\0' ) {
    send_to_char( "Compare what to what?\n\r", ch );
    return;
  }

  if ( ( obj1 = get_obj_carry( ch, arg1, ch ) ) == NULL ) {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
  }

  if (arg2[0] == '\0') {
    for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content) {
      if (obj2->wear_loc != WEAR_NONE
	  && can_see_obj(ch,obj2)
	  && obj1->item_type == obj2->item_type
	  && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0 )
	break;
    }

    if (obj2 == NULL)	{
      send_to_char("You aren't wearing anything comparable.\n\r",ch);
      return;
    }
  } 

  else if ( (obj2 = get_obj_carry(ch,arg2,ch) ) == NULL ) {
    send_to_char("You do not have that item.\n\r",ch);
    return;
  }

  msg		= NULL;
  value1	= 0;
  value2	= 0;

  if ( obj1 == obj2 ) {
    msg = "You compare $p to itself.  It looks about the same.";
  }
  else if ( obj1->item_type != obj2->item_type ) {
    msg = "You can't compare $p and $P.";
  }
  else {
    switch ( obj1->item_type ) {
    default:
      msg = "You can't compare $p and $P.";
      break;

    case ITEM_ARMOR:
      value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
      value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
      break;

    case ITEM_WEAPON: {
      int v11 = GET_WEAPON_DNUMBER(obj1);
      int v12 = GET_WEAPON_DTYPE(obj1);
      int v21 = GET_WEAPON_DNUMBER(obj2);
      int v22 = GET_WEAPON_DTYPE(obj2);
      if (obj1->pIndexData->new_format)
	value1 = (1 + v12) * v11;
      else
	value1 = v11 + v12;

      if (obj2->pIndexData->new_format)
	value2 = (1 + v22) * v21;
      else
	value2 = v21 + v22;
      break;
    }
    }
  }

  if ( msg == NULL ) {
    if      ( value1 == value2 ) msg = "$p and $P look about the same.";
    else if ( value1  > value2 ) msg = "$p looks better than $P.";
    else                         msg = "$p looks worse than $P.";
  }

  act( msg, ch, obj1, obj2, TO_CHAR );
  return;
}

void do_credits( CHAR_DATA *ch, const char *argument ) {
  do_help( ch, "diku" );
  return;
}

void do_where( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  DESCRIPTOR_DATA *d;
  bool found;

  one_argument( argument, arg );

  if ( IS_NPC( ch ) ) {
    send_to_char("Mobiles can't use this command!\n\r",ch);
    return;
  }

  /* Added by SinaC 2000 */
  // Modified by SinaC 2001
  if (!IS_SET(ch->in_room->cstat(flags),ROOM_NOWHERE) || IS_IMMORTAL(ch) )
    sprintf( buf, "[%s].\n\r", ch->in_room->area->name );
  else
    sprintf( buf, "Somewhere.\n\r");
  send_to_char( buf, ch );

  if ( arg[0] == '\0' ) {
    send_to_char( "Players near you:\n\r", ch );
    found = FALSE;
    for ( d = descriptor_list; d; d = d->next ) {
      if ( d->connected == CON_PLAYING
	   && ( victim = d->character ) != NULL
	   && !IS_NPC(victim)
	   && victim->in_room != NULL
	 // Modified by SinaC 2001
	   && !IS_SET(victim->in_room->cstat(flags),ROOM_NOWHERE)
	   && (is_room_owner(ch,victim->in_room) 
	       || !room_is_private(victim->in_room))
	   && victim->in_room->area == ch->in_room->area
	   && can_see( ch, victim ) ) {
	found = TRUE;
	sprintf( buf, "%-28s %s\n\r",
		 victim->name, victim->in_room->name );
	send_to_char( buf, ch );
      }
    }
    if ( !found )
      send_to_char( "None\n\r", ch );
  }
  else {
    found = FALSE;
    for ( victim = char_list; victim != NULL; victim = victim->next ) {
      if ( victim->in_room != NULL
	   && victim->in_room->area == ch->in_room->area
	   && !IS_AFFECTED(victim, AFF_HIDE)
	   && !IS_AFFECTED(victim, AFF_SNEAK)
	   && can_see( ch, victim )
	   && is_name( arg, victim->name ) ) {
	found = TRUE;
	sprintf( buf, "%-28s %s\n\r",
		 PERS(victim, ch), victim->in_room->name );
	send_to_char( buf, ch );
	break;
      }
    }
    if ( !found )
      act( "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
  }

  return;
}

void do_consider( CHAR_DATA *ch, const char *argument ) {
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *victim;
  const char *msg;
  int diff;

  one_argument( argument, arg );

  if ( arg[0] == '\0' ) {
    send_to_char( "Consider killing whom?\n\r", ch );
    return;
  }

  if ( ( victim = get_char_room( ch, arg ) ) == NULL ) {
    send_to_char( "They're not here.\n\r", ch );
    return;
  }

  if (is_safe(ch,victim)) {
    send_to_char("Don't even think about it.\n\r",ch);
    return;
  }

  diff = victim->level - ch->level;

  if      ( diff <= -10 ) msg = "You can kill $N naked and weaponless.";
  else if ( diff <=  -5 ) msg = "$N is no match for you.";
  else if ( diff <=  -2 ) msg = "$N looks like an easy kill.";
  else if ( diff <=   1 ) msg = "The perfect match!";
  else if ( diff <=   4 ) msg = "$N says 'Do you feel lucky, punk?'.";
  else if ( diff <=   9 ) msg = "$N laughs at you mercilessly.";
  else                    msg = "Death will thank you for your gift.";

  act( msg, ch, NULL, victim, TO_CHAR );
  return;
}

void set_title( CHAR_DATA *ch, const char *title )
{
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) ) {
    bug( "Set_title: NPC." );
    return;
  }

  if ( title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?' ) {
    buf[0] = ' ';
    strcpy( buf+1, title );
  }
  else
    strcpy( buf, title );

  ch->pcdata->title = str_dup( buf );
  return;
}

void do_title( CHAR_DATA *ch, const char *argument ) {
  if ( IS_NPC(ch) )
    return;

  if ( argument[0] == '\0' ) {
    send_to_char( "Change your title to what?\n\r", ch );
    return;
  }

  argument = truncate_str(argument, 45);

  //  smash_tilde( argument );
  set_title( ch, argument );
  send_to_char( "Ok.\n\r", ch );
}

void do_description( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  if ( argument[0] != '\0' ) {
    buf[0] = '\0';
    //    smash_tilde( argument );

    if (argument[0] == '-') {
      int len;
      bool found = FALSE;
 
      if (ch->description == NULL || ch->description[0] == '\0') {
	send_to_char("No lines left to remove.\n\r",ch);
	return;
      }
	
      strcpy(buf,ch->description);
 
      for (len = strlen(buf); len > 0; len--) {
	if (buf[len] == '\r') {
	  if (!found) {  /* back it up */
	    if (len > 0)
	      len--;
	    found = TRUE;
	  }
	  else { /* found the second one */
	    buf[len + 1] = '\0';
	    ch->description = str_dup(buf);
	    send_to_char( "Your description is:\n\r", ch );
	    send_to_char( ch->description ? ch->description : 
			  "(None).\n\r", ch );
	    return;
	  }
	}
      }
      buf[0] = '\0';
      ch->description = str_dup(buf);
      send_to_char("Description cleared.\n\r",ch);
      return;
    }
    if ( argument[0] == '+' )	{
      if ( ch->description != NULL )
	strcat( buf, ch->description );
      argument++;
      while ( isspace(*argument) )
	argument++;
    }

    if ( strlen(buf) >= 1024)	{
      send_to_char( "Description too long.\n\r", ch );
      return;
    }

    strcat( buf, argument );
    strcat( buf, "\n\r" );
    ch->description = str_dup( buf );
  }

  send_to_char( "Your description is:\n\r", ch );
  send_to_char( ch->description ? ch->description : "(None).\n\r", ch );
  return;
}

void do_report( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_INPUT_LENGTH];

  sprintf(buf,"I have %d/%ld hp %d/%ld mana %d/%ld psp %d/%ld mv %d xp.",
	  ch->hit,  ch->cstat(max_hit),
	  ch->mana, ch->cstat(max_mana),
	  ch->psp,  ch->cstat(max_psp),
	  ch->move, ch->cstat(max_move),
	  ch->exp );

  do_say( ch, buf );

  /* Modified by SinaC 2001 for mental user
  sprintf( buf,
	   "You say 'I have %d/%ld hp %d/%ld mana %d/%ld mv %d xp.'\n\r",
	   ch->hit,  ch->cstat(max_hit),
	   ch->mana, ch->cstat(max_mana),
	   ch->move, ch->cstat(max_move),
	   ch->exp   );

  send_to_char( buf, ch );

  sprintf( buf, "$n says 'I have %d/%ld hp %d/%ld mana %d/%ld mv %d xp.'",
	   ch->hit,  ch->cstat(max_hit),
	   ch->mana, ch->cstat(max_mana),
	   ch->move, ch->cstat(max_move),
	   ch->exp   );

  act( buf, ch, NULL, NULL, TO_ROOM );
  */
  return;
}

// Modified by SinaC 2000
void do_practice( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  int sn;
  // Added by SinaC 2000
  int pra, upgr;
  BUFFER *buffer;

  if ( IS_NPC(ch) )
    return;
  
  if ( argument[0] == '\0' ){
    int col;
    
    buffer = new_buf();
    
    col    = 0;
    add_buf( buffer, 
	     "ability name           perc  bonus   "
	     "ability name           perc  bonus\n\r"
	     "----------------       ----  -----   "
	     "----------------       ----  -----\n\r");
    for ( sn = 1; sn < MAX_ABILITY; sn++ ){
      if ( ability_table[sn].name == NULL )
	break;
    
      // Modified by Sinac 1997, revised by Oxtal
      // Modified again by SinaC 2000
      // Modified again by SinaC 2001 for god skill
      // Modified again by SinaC 2001
      int level = class_abilitylevel(ch,sn);
      //pra = ch->pcdata->ability_info[sn].learned;
      pra = get_ability( ch, sn );
      if ( get_raceability( ch, sn ) 
	   || get_clanability( ch, sn ) 
	   /*|| get_godskill( ch, sn )  removed by SinaC 2003*/ ) {
	pra = 100;
	level = 0;
      }
      if ( pra == 0 )
	continue;
      // Added again by SinaC 2001
      // modified by SinaC 2001

      if (ch->level < level )
	continue;

      if ( ( upgr = get_ability_upgrade( ch, sn ) ) != 0 ) {
	if (upgr>0)
	  sprintf(buf2," +%2d%%    ",upgr);
	else
	  sprintf(buf2," -%2d%%    ",-upgr);
      }
      else
	sprintf( buf2, "         ");
      // Modified by SinaC 2000
      sprintf( buf, "%-22s %3d%% %s",
	       ability_table[sn].name, pra,
	       buf2 );
      //send_to_char( buf, ch );
      add_buf(buffer,buf);
      if ( ++col % 2 == 0 )
	//send_to_char( "\n\r", ch );
	add_buf(buffer,"\n\r");
    }
    
    if ( col % 2 != 0 )
      //send_to_char( "\n\r", ch );
      add_buf(buffer,"\n\r");
    
    sprintf( buf, "You have %d practice sessions left.\n\r",
	     ch->practice );
    //send_to_char( buf, ch );
    add_buf(buffer,buf);
    
    page_to_char(buf_string(buffer),ch);
  }
  else{
    CHAR_DATA *mob;
    int adept;
    
    if ( !IS_AWAKE(ch) ){
      send_to_char( "In your dreams, or what?\n\r", ch );
      return;
    }
    
    for ( mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room ){
      if ( IS_NPC(mob) && IS_SET(mob->act, ACT_PRACTICE) )
	break;
    }
    
    if ( mob == NULL ){
      send_to_char( "You can't do that here... No one can help you to practice!\n\r", ch );
      return;
    }
    
    if ( ch->practice <= 0 ){
      send_to_char( "You have no practice sessions left.\n\r", ch );
      return;
    }

    // Added by SinaC 2001
    sn = ability_lookup( argument );
    if ( sn <= 0 ) {
      send_to_char("That's not a valid ability.\n\r", ch );
      return;
    }
    
    // Added by SinaC 2000
    pra = ch->pcdata->ability_info[sn].learned;
    // Added by Sinac 1997  and  modified by SinaC 2000 for clan skill
    // modified by SinaC 2001 for god skill
    // Modified again by SinaC 2001
    if ( get_raceability( ch, sn ) 
	 || get_clanability( ch, sn ) 
	 /*|| get_godskill( ch, sn )  removed by SinaC 2003*/ ) {
      sprintf( buf, "You don't need to practice %s.\n\r",
	       ability_table[sn].name );
      send_to_char( buf, ch );
      return;
    }
    /*
    if ( !get_raceskill( ch, sn ) 
	 && !get_clanskill( ch, sn ) 
	 && !get_godskill( ch, sn ))
    */
    // Modified by SinaC 2001
      /*if ( ( sn = find_spell( ch, argument ) ) < 0
	   || ( !IS_NPC(ch)
		// Modified by SinaC 2001
		&&*/
        if ( ( (ch->level < class_abilitylevel(/*ch->cstat(classes)*/ch,sn)
		      // skill is not known
		      // Modified by SinaC 2000
		      ||    /*ch->pcdata->learned[sn] < 1*/ pra < 1
		      // Removed by SinaC 2000
		      /*||    class_skillrating(ch,sn) == 0*/))){
	//send_to_char( "You can't practice that.\n\r", ch );
	send_to_charf( ch, "You can't practice '%s'.\n\r", ability_table[sn].name );
	return;
      }
    
    adept = IS_NPC(ch) ? 100 : class_abilityadept(ch->cstat(classes));
    // Added by SinaC 2001
    if ( is_language( sn ) )
      adept = 90;
    
    // Modified by SinaC 2000
    if ( /*ch->pcdata->learned[sn]*/pra >= adept ){
      sprintf( buf, "You are already learned at %s.\n\r",
	       ability_table[sn].name );
      send_to_char( buf, ch );
    }
    else {
      ch->practice--;
      // Modified by SinaC 2000
      int skrate = class_abilityrating(ch,sn, ch->pcdata->ability_info[sn].casting_level);
      // if skill couldn't be learned we artificially consider difficulty equals to 10, SinaC 2000
      int add = 1; // SinaC 2003
      if ( skrate == 0 )
	add = UMAX( 1, int_app[ch->cstat(INT)].learn/10 );
      else
	add = UMAX( 1, int_app[ch->cstat(INT)].learn/skrate );
      ch->pcdata->ability_info[sn].learned += add;
      // Modified by SinaC 2000
      ch->pcdata->ability_info[sn].learned = URANGE( 0, ch->pcdata->ability_info[sn].learned, 100 );
      if ( ch->pcdata->ability_info[sn].learned < adept ){
	// Modified by SinaC 2000
	sprintf(buf,"You practice $T to %d%%.", ch->pcdata->ability_info[sn].learned);
	act( buf, ch, NULL, ability_table[sn].name, TO_CHAR );
	/*
	  act( "You practice $T.",
	  ch, NULL, ability_table[sn].name, TO_CHAR );
	*/
	act( "$n practices $T.",
	     ch, NULL, ability_table[sn].name, TO_ROOM );
      }
      else{
      // Modified by SinaC 2000
	ch->pcdata->ability_info[sn].learned = adept;
	act( "You are now learned at $T.",
	     ch, NULL, ability_table[sn].name, TO_CHAR );
	act( "$n is now learned at $T.",
	     ch, NULL, ability_table[sn].name, TO_ROOM );
      }
    }
  }
  return;
}

void do_password( CHAR_DATA *ch, const char *argument0 ) {
  char argument1[MAX_STRING_LENGTH];
  char arg1[MAX_INPUT_LENGTH];
  char arg2[MAX_INPUT_LENGTH];
  char *pArg;
  char *pwdnew;
  char *p;
  char cEnd;

  if ( IS_NPC(ch) )
    return;
  
  strcpy(argument1, argument0);
  char* argument = argument1;
  /*
   * Can't use one_argument here because it smashes case.
   * So we just steal all its code.  Bleagh.
   */
  pArg = arg1;
  while ( isspace(*argument) )
    argument++;

  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;

  while ( *argument != '\0' ){
    if ( *argument == cEnd ){
      argument++;
      break;
    }
    *pArg++ = *argument++;
  }
  *pArg = '\0';
  
  pArg = arg2;
  while ( isspace(*argument) )
    argument++;
  
  cEnd = ' ';
  if ( *argument == '\'' || *argument == '"' )
    cEnd = *argument++;
  
  while ( *argument != '\0' ){
    if ( *argument == cEnd ){
      argument++;
      break;
    }
    *pArg++ = *argument++;
  }
  *pArg = '\0';
  
  if ( arg1[0] == '\0' || arg2[0] == '\0' ){
    send_to_char( "Syntax: password <old> <new>.\n\r", ch );
    return;
  }
  
  //if ( strcmp( /*crypt(*/ arg1/*, ch->pcdata->pwd )*/, ch->pcdata->pwd ) ){
  if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) ) {
    WAIT_STATE( ch, 40 );
    send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
    return;
  }

  if ( strlen(arg2) < 5 ){
    send_to_char( "New password must be at least five characters long.\n\r", ch );
    return;
  }

  /*
   * No tilde allowed because of player file format.
   */
  pwdnew = crypt( arg2, ch->name );
  //strcpy( pwdnew, arg2 );
  // Modified by SinaC 2000
  //pwdnew = str_dup(arg2);
  for ( p = pwdnew; *p != '\0'; p++ ){
    if ( *p == '~' ){
      send_to_char( "New password not acceptable, try again.\n\r", ch );
      return;
    }
  }

  ch->pcdata->pwd = str_dup( pwdnew );
  //save_char_obj( ch );
  new_save_pFile(ch, FALSE );
  send_to_char( "Ok.\n\r", ch );
  return;
}

void do_stance( CHAR_DATA *ch, const char *argument ) {
  char buf[MAX_STRING_LENGTH];

  if ( IS_NPC(ch) )
    return;

  //  smash_tilde( argument );

  if (argument[0] == '\0'){
    if ( ch->pcdata->stance == NULL || ch->pcdata->stance[0]=='\0' )
      sprintf(buf,"You don't have any stance.\n\r");
    else
      sprintf(buf,"Your stance is %s\n\r",ch->pcdata->stance);
    send_to_char(buf,ch);
    return;
  }

  if ( !str_cmp( argument, "none" ) ){
    ch->pcdata->stance = str_dup("");
    send_to_char("You have removed your stance.\n\r", ch );
    return;
  }

  if ( strstr(argument,ch->name) == NULL){
    send_to_char("You must include your name.\n\r",ch);
    return;
  }
	     
  ch->pcdata->stance = str_dup( argument );

  sprintf(buf,"Your stance is now %s\n\r",ch->pcdata->stance);
  send_to_char(buf,ch);

  return;
}

// Added by SinaC 2000 for window code
void look_window( CHAR_DATA *ch, OBJ_DATA *obj )    /* Voltecs Window code 1998 */
{
  char buf[MAX_STRING_LENGTH];
  ROOM_INDEX_DATA *window_room;   
  
  if ( obj->value[0] == 0 ){
    sprintf(buf, "%s\n\r", obj->description );
    send_to_char(buf, ch);
    return;
  }
  
  window_room = get_room_index( obj->value[0] );
  
  if ( window_room == NULL ){
    send_to_char( "!!BUG!! Window looks into a NULL room! Please report!\n\r", ch );
    bug( "Window %d looks into a null room!!!", obj->pIndexData->vnum );
    return;
  }
  
  if ( !IS_NPC(ch) ){
    send_to_char( "Looking through the window you can see ",ch);
    send_to_char( window_room->name, ch );
    send_to_char( "\n\r", ch);
    show_list_to_char( window_room->contents, ch, FALSE, FALSE );
    show_char_to_char( window_room->people,   ch );
    return;
  }
}

/**********************************************************************
*   Function to format big numbers, so you can easy understand it.    *
*    Added by Desden, el Chaman Tibetano (J.L.Sogorb) in Oct-1998     *
*                Email: jlalbatros@retemail.es                        *
*								      *
**********************************************************************/
const char *num_punct(int foo)
{
  // Modified by SinaC 2001  int  before
  unsigned int index,index_new,rest;
  char buf[64];
  static char buf_new[64];
  
  sprintf(buf,"%d",foo);
  rest = strlen(buf)%3;
  
  for (index=index_new=0;index<strlen(buf);index++,index_new++){
    if (index!=0 && (index-rest)%3==0 ){
      buf_new[index_new]='.';
      index_new++;
      buf_new[index_new]=buf[index];
    }
    else
      buf_new[index_new] = buf[index];
  }
  buf_new[index_new]='\0';
  return buf_new;
}


// Added by SinaC 2001
void small_help( const char *argument, BUFFER *output ) {
  HELP_DATA *pHelp;
  char arg1[MAX_INPUT_LENGTH];
  strcpy( arg1, str_to_upper( argument ) );

  bool found = FALSE;
  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next ) {
    int level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

    //    if ( !str_cmp( argument, pHelp->keyword ) ) {
    if ( strstr( str_to_upper(pHelp->keyword), arg1 ) ) {
      found = TRUE;
      if ( pHelp->text[0] == '.' )
	add_buf(output,pHelp->text+1);
      else
	add_buf(output,pHelp->text);
    }
  }
  if ( !found )
    add_buf(output,"No help available for the moment.");
}
// SinaC 2003
const char *small_help_string( const char *argument ) {
  HELP_DATA *pHelp;
  static char buf[MAX_STRING_LENGTH*4];
  char arg1[MAX_INPUT_LENGTH];
  strcpy( arg1, str_to_upper( argument ) );

  buf[0] = '\0';
  bool found = FALSE;
  for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next ) {
    int level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

    //if ( !str_cmp( argument, pHelp->keyword ) ) {
    if ( strstr( str_to_upper(pHelp->keyword), arg1 ) ) {
      found = TRUE;
      if ( pHelp->text[0] == '.' )
	strcat( buf, pHelp->text+1 );
      else
	strcat( buf, pHelp->text );
    }
  }
  if ( !found )
    strcpy( buf ,"No help available for the moment.");
  return buf;
}

void racehelp( int race, BUFFER *output ) {
  bool found;
  char buf[MAX_STRING_LENGTH];

  if (race < 0 || !race_table[race].pc_race)
    return;

  if ( pc_race_table[race].type == RACE_NOTAVAILABLE )
    add_buf( output, "\n\r{RNOT AVAILABLE FOR THE MOMENT{x\n\r" );

  sprintf( buf, "\n\r{W%s:{x\n\r",
	   capitalize(race_table[race].name));
  add_buf( output, buf );

  sprintf(buf,"race_%s",race_table[race].name);
  small_help(buf,output);

  add_buf(output,
	  "\n\r{WInfos:{x\n\r");

  add_buf(output,
	  "{Y                 STR  INT  WIS  DEX  CON{x\n\r");
  sprintf( buf,
	  "Base attributes: {C%3d  %3d  %3d  %3d  %3d{x\n\r",
	  pc_race_table[race].stats[0],
	  pc_race_table[race].stats[1],
	  pc_race_table[race].stats[2],
	  pc_race_table[race].stats[3],
	  pc_race_table[race].stats[4]);
  add_buf( output, buf );

  sprintf( buf,
	  "Max. attributes: {B%3d  %3d  %3d  %3d  %3d{x\n\r",
	  pc_race_table[race].max_stats[0],
	  pc_race_table[race].max_stats[1],
	  pc_race_table[race].max_stats[2],
	  pc_race_table[race].max_stats[3],
	  pc_race_table[race].max_stats[4]);
  add_buf(output,buf);

  add_buf(output,
	  "Alignments allowed:{W");
  if ( pc_race_table[race].nb_allowed_align == 9 )
    add_buf(output,
	    " Any");
  else
    for ( int i = 0; i < pc_race_table[race].nb_allowed_align; i++ ) {
      int etho = pc_race_table[race].allowed_align[i].etho + 1;
      int align = pc_race_table[race].allowed_align[i].alignment/350 + 1;
      int index = etho + align * 3;
      sprintf( buf,
	       " %2s", short_etho_align[index] );
      add_buf( output, buf );
    }

  add_buf(output,
	  "{x\n\r" );
  add_buf(output,
	  "Classes allowed:{W" );

  int allowed_class = pc_race_table[race].allowed_class;
  if ( count_bit( allowed_class ) == MAX_CLASS )
    add_buf( output, " Any" );
  else
    for ( long i = 0; i < MAX_CLASS; i++ )
      if ( ( 1 << i ) & allowed_class ) {
	sprintf( buf, "  %s", capitalize(class_table[i].name) );
	add_buf( output, buf );
      }
  add_buf( output, "{x\n\r\n\r" );
//  if ( pc_race_table[race].nb_allowed_class == MAX_CLASS )
//    add_buf(output,
//	    " Any");
//  else
//    for ( int i = 0; i < pc_race_table[race].nb_allowed_class; i++ ) {
//      sprintf( buf,
//	       "   %s", 
//	       class_table[class_firstclass(pc_race_table[race].allowed_class[i])].name );
//      add_buf( output, buf );
//    }
//  add_buf( output, "{x\n\r\n\r" );

  sprintf( buf,
	   "Size: {m%-10s{x         Exp/level: {r%d{x\n\r\n\r",
	   //capitalize(size_table[pc_race_table[race].size].name),
	   capitalize(size_table[race_table[race].size].name),
	   pc_race_table[race].expl);
  add_buf( output, buf );

  const char *aff;
  const char *aff2;
  aff = affect_bit_name(race_table[race].aff);
  aff2 = affect2_bit_name(race_table[race].aff2);
  if ( !str_cmp( aff, "none" ) ) // no affect
    if ( !str_cmp( aff2, "none" ) ) // no affect neither affect2
      sprintf( buf, "Affected by     : {gnone{x\n\r");
    else // not affect but affect2
      sprintf( buf, "Affected by     : {g%s{x\n\r", aff2 );
  else // affect
    if ( !str_cmp( aff2, "none") ) // affect but no affect2
      sprintf( buf, "Affected by     : {g%s{x\n\r", aff );
    else // affect and affect 2
      sprintf( buf, "Affected by     : {g%s %s{x\n\r", aff, aff2 );
  add_buf( output, buf );

  sprintf(buf,  "Immunities      : {y%s{x\n\r",
	  irv_bit_name(race_table[race].imm));
  add_buf( output, buf );
  sprintf(buf,"Resistances     : {b%s{x\n\r",
	  irv_bit_name(race_table[race].res));
  add_buf( output, buf );
  sprintf(buf,"Vulnerabilities : {r%s{x\n\r",
	  irv_bit_name(race_table[race].vuln));
  add_buf( output, buf );

  found = FALSE;
  add_buf( output, "\n\rNatural born abilities:{g" );
  for ( int i = 0; i < pc_race_table[race].nb_abilities; i++ ) {
    if ( pc_race_table[race].abilities[i] <= 0 )
      continue;
    found = TRUE;
    sprintf( buf,"   %s", capitalize(ability_table[pc_race_table[race].abilities[i]].name) );
    add_buf( output, buf );
  }
  if (!found)
    add_buf( output, "   none" );

  add_buf( output, "{x\n\r" );

  sprintf( buf,
	   "Racial language:          {m%s{x\n\r",
	   capitalize(language_name(pc_race_table[race].language)));
  add_buf( output, buf );

  add_buf( output, "Remort option: {W" );
  found = FALSE;
  for ( int i = 0; i < pc_race_table[race].nb_remorts; i++ ) {
    if ( pc_race_table[race].remorts[i] < 0 )
      continue;
    found = TRUE;
    sprintf( buf, "  %s", capitalize(pc_race_table[pc_race_table[race].remorts[i]].name) );
    add_buf( output, buf );
  }
  if ( !found )
    add_buf( output, "  none");
  add_buf( output, "{x\n\r" );
  return;
}

static int iClass;
static int compare( const void *a, const void *b ) {
  int i1 = ability_table[(int)(*((const int*)a))].ability_level[iClass];
  int i2 = ability_table[(int)(*((const int*)b))].ability_level[iClass];
  if ( i1 < i2 ) return -1;
  if ( i1 > i2 ) return 1;
  return 0;
}
void classhelp( int classindex, BUFFER *output ) {
  char arg[MAX_INPUT_LENGTH];
  char buf[MAX_STRING_LENGTH];
  int skills[MAX_ABILITY+1];
  int spells[MAX_ABILITY+1];
  int powers[MAX_ABILITY+1];
  int songs[MAX_ABILITY+1];
  //int affects[MAX_ABILITY+1];
  int curspell, curskill, curpower,
  // Added by SinaC 2003
    cursong;//, curaff;

  iClass = classindex;


  curskill = 0;
  curspell = 0;
  curpower = 0;
  // Added by SinaC 2003
  cursong = 0;
  //curaff = 0;

  for ( int i = 1; i < MAX_ABILITY; i++ ) {
    // to be sure
    if ( ability_table[i].name == NULL || 
	 ability_table[i].name[0] == '\0' )
      break;
    //if ( ability_table[i].rating[iClass] != 0 
    // SinaC 2003, FIXME  if we uncomment previous test, sphere are also shown
    if ( ability_table[i].rating[iClass] > 0
	 || ability_table[i].ability_level[iClass] < LEVEL_IMMORTAL ) {
      switch( ability_table[i].type ) {
      case TYPE_SKILL: skills[curskill++] = i; break;
      case TYPE_SPELL: spells[curspell++] = i; break;
      case TYPE_POWER: powers[curpower++] = i; break;
	// Added by SinaC 2003
      case TYPE_SONG: songs[cursong++] = i; break;
	//case TYPE_AFFECT: affects[curaff++] = i; break;
      }
    }
  }

  qsort( skills, curskill, sizeof(skills[0]), compare );
  qsort( spells, curspell, sizeof(spells[0]), compare );
  qsort( powers, curpower, sizeof(powers[0]), compare );
  qsort( songs, cursong, sizeof(songs[0]), compare );
  //qsort( affects, curaff, sizeof(affects[0]), compare );

  if ( class_table[iClass].choosable == CLASS_CHOOSABLE_NEVER )
    add_buf( output, "\n\r{RNOT AVAILABLE FOR THE MOMENT{x\n\r" );

  sprintf( buf,"\n\r{W%s:{x\n\r",
	   capitalize(class_table[iClass].name));
  add_buf( output, buf );

  sprintf( buf,"class_%s", class_table[iClass].name );
  small_help(buf,output);

  add_buf( output, "\n\rAlignments Allowed: {W");
  if ( class_table[iClass].nb_allowed_align == 9 )
    add_buf(output,
	    " Any{x");
  else
    for ( int i = 0; i < class_table[iClass].nb_allowed_align; i++ ) {
      int etho = class_table[iClass].allowed_align[i].etho + 1;
      int align = class_table[iClass].allowed_align[i].alignment/350 + 1;
      int index = etho + align * 3;
      sprintf( buf,
	       " %2s", short_etho_align[index] );
      add_buf( output, buf );
    }
  char buf2[MAX_STRING_LENGTH];
  strcpy( buf2, flag_string(attr_flags, class_table[iClass].attr_prime+ATTR_stat0) );
  buf2[0] = UPPER( buf2[0] );
  sprintf( buf, "\n\r{xPrime Requisite Stat: {W%s{x", buf2 );
  add_buf( output, buf );

  // Added by SinaC 2003 for subclasses
  if ( class_table[iClass].nb_sub_classes != 0 ) {
    add_buf(output,"\n\r{WSub-classes:");
    for ( int i = 0; i < class_table[iClass].nb_sub_classes; i++ ) {
      sprintf(buf,"   %s", capitalize(class_table[class_table[iClass].sub_classes[i]].name) );
      add_buf(output,buf);
      if ( i > 0 && i % 3 == 0 )
	add_buf(output,"\n\r            ");
    }
    add_buf(output,"{x");
  }

  sprintf(buf,"\n\r{WBasic %s abilities:{x\n\r", capitalize(class_table[iClass].name));
  add_buf(output,buf);
  //sprintf( buf,"%s basics", class_table[iClass].name );
  //groups_help( buf, output );
  groups_help( class_table[iClass].base_group, output );

  int col;
 
  if ( curskill != 0 ) { 
    sprintf( buf,
	     "{WSkills:{x\n\r"
	     " {W%-20s %3s  %-20s %3s  %-20s %3s{x\n\r",
	     "Name", "Lvl", "Name", "Lvl", "Name", "Lvl" );
    add_buf( output, buf );
    col = 0;
    for ( int i = 0; i < curskill; i++ ) {
      sprintf( buf,
	       " {y%-20s {c%3d{x",
	       capitalize(ability_table[skills[i]].name),
	       ability_table[skills[i]].ability_level[iClass]);
      add_buf(output, buf);
      if ( ++col % 3 == 0 )
	add_buf(output,"\n\r");
      else
	add_buf(output," ");
    }
    if ( col % 3 != 0 )
      add_buf(output,"\n\r");
  }

  if ( curspell != 0 ) {
    sprintf( buf,
	     "{WSpells:{x\n\r"
	     " {W%-20s %3s  %-20s %3s  %-20s %3s{x\n\r",
	     "Name", "Lvl", "Name", "Lvl", "Name", "Lvl" );
    add_buf( output, buf );
    col = 0;
    for ( int i = 0; i < curspell; i++ ) {
      sprintf( buf,
	       " {y%-20s {c%3d{x",
	       capitalize(ability_table[spells[i]].name),
	       ability_table[spells[i]].ability_level[iClass]);
      add_buf(output, buf);
      if ( ++col % 3 == 0 )
	add_buf(output,"\n\r");
      else
	add_buf(output," ");
    }
    if ( col % 3 != 0 )
      add_buf(output,"\n\r");
  }

  if ( curpower != 0 ) {
    sprintf( buf,
	     "{WPowers:{x\n\r"
	     " {W%-20s %3s  %-20s %3s  %-20s %3s{x\n\r",
	     "Name", "Lvl", "Name", "Lvl", "Name", "Lvl" );
    add_buf( output, buf );
    col = 0;
    for ( int i = 0; i < curpower; i++ ) {
      sprintf( buf,
	       " {y%-20s {c%3d{x",
	       capitalize(ability_table[powers[i]].name),
	       ability_table[powers[i]].ability_level[iClass]);
      add_buf(output, buf);
      if ( ++col % 3 == 0 )
	add_buf(output,"\n\r");
      else
	add_buf(output," ");
    }
    if ( col % 3 != 0 )
      add_buf(output,"\n\r");
  }
  if ( cursong != 0 ) {
    sprintf( buf,
	     "{WSongs:{x\n\r"
	     " {W%-20s %3s  %-20s %3s  %-20s %3s{x\n\r",
	     "Name", "Lvl", "Name", "Lvl", "Name", "Lvl" );
    add_buf( output, buf );
    col = 0;
    for ( int i = 0; i < cursong; i++ ) {
      sprintf( buf,
	       " {y%-20s {c%3d{x",
	       capitalize(ability_table[songs[i]].name),
	       ability_table[songs[i]].ability_level[iClass]);
      add_buf(output, buf);
      if ( ++col % 3 == 0 )
	add_buf(output,"\n\r");
      else
	add_buf(output," ");
    }
    if ( col % 3 != 0 )
      add_buf(output,"\n\r");
  }
}

void abilityhelp( int sn, BUFFER *output ) {
  char buf[MAX_STRING_LENGTH], buf2[MAX_STRING_LENGTH], buf3[MAX_STRING_LENGTH];
  int i, level;

  sprintf( buf, "   {W%s : {y%s{x", 
	   abilitytype_name(ability_table[sn].type ),
	   capitalize(ability_table[sn].name));
  if ( ability_table[sn].nb_casting_level != 0 ) {
    char buf2[128];
    if ( ability_table[sn].nb_casting_level > 1 )
      sprintf( buf2, "   {c%2d{x levels\n\r", ability_table[sn].nb_casting_level );
    else
      sprintf( buf2, "   {c%2d{x level\n\r", ability_table[sn].nb_casting_level );
    strcat( buf, buf2 );
  }
  //else
  //  strcat( buf, "    no level\n\r" );
  else
    strcat( buf, "\n\r" );
  add_buf(output,buf);

  // SinaC 2003: school
  if ( ability_table[sn].school > 0 ) {
    sprintf(buf,"{gSchool: {b%s{x\n\r", capitalize(magic_school_table[ability_table[sn].school].name) );
    add_buf(output,buf);
  }
  // SinaC 2003: sphere
  int sphereId = is_ability_in_sphere( sn );
  if ( sphereId >= 0 ) {
    sprintf(buf,"{gSphere: {b%s{x\n\r", capitalize(group_table[sphereId].name) );
    add_buf(output,buf);
  }

//  switch(ability_table[sn].mob_use) {
//  case MOB_USE_AUTOMATIC:
//    add_buf( output,"   {WAutomatic skill.{x\n\r\n\r");
//    break;
//    // Mob_use_none means mobiles can't use that skill/spell/power
//  case MOB_USE_NONE:
//  case MOB_USE_COMBAT:
//  case MOB_USE_NORMAL:
//    break;
//  }
  // SinaC 2003, an ability is automatic when action_fun is null
  if ( ability_table[sn].action_fun == NULL )
    add_buf( output, "   {WAutomatic skill.{x\n\r\n\r");
  
  // Add '' to be sure to found it in the help file
  sprintf( buf, "'%s_%s'", abilitytype_name(ability_table[sn].type), ability_table[sn].name);
  small_help(buf,output );

  add_buf( output,"\n\r");

//  if ( ability_table[sn].prereqs != NULL ) {
//    buf2[0] = '\0';
//    sprintf( buf, 
//	     "{WPrerequisites:{x\n\r" );
//    for ( i = 0; i < ability_table[sn].nb_casting_level+1; i++ ) {
//      if ( i == 0 && ability_table[sn].prereqs[i].nb_prereq == 0 )
//	continue;
//      else
//	if ( i != 0 || ability_table[sn].nb_casting_level > 1 )
//	  sprintf( buf2,
//		   " {glevel %2d:{x ", i );
//	else
//	  sprintf( buf2, 
//		   "           " );
//      if ( ability_table[sn].prereqs[i].nb_prereq == 0 )
//	strcat( buf2, "none\n\r");
//      else {
//	for ( int j = 0; j < ability_table[sn].prereqs[i].nb_prereq; j++ ) {
//	  if ( ability_table[sn].prereqs[i].prereq[j].casting_level == 0 )
//	    sprintf( buf3,
//		     "%s{y%s{x\n\r",
//		     j == 0 ? "" : "           ",
//		     ability_table[ability_table[sn].prereqs[i].prereq[j].sn].name );
//	  else
//	    sprintf( buf3,
//		     "%s{y%s{x {glevel %2d{x\n\r",
//		     j == 0 ? "" : "           ",
//		     ability_table[ability_table[sn].prereqs[i].prereq[j].sn].name,
//		     ability_table[sn].prereqs[i].prereq[j].casting_level );
//	  strcat( buf2, buf3 );
//	}
//      }
//      strcat( buf, buf2 );
//    }
//    add_buf(output, buf );
//  }
  ability_type *pAbility = &(ability_table[sn]);
  if ( pAbility->prereqs != NULL ) {
    sprintf( buf, 
	     "{WPrerequisites:{x\n\r" );
    for ( int i = 0; i < pAbility->nb_casting_level+1; i++ ) {
      char spaces[MAX_STRING_LENGTH];
      spaces[0] = '\0';
      if ( i == 0 && pAbility->prereqs[i].nb_prereq == 0 )
	continue;
      PREREQ_DATA *prereq = &(pAbility->prereqs[i]);

      if ( i != 0 || pAbility->nb_casting_level != 0 ) {
	if ( prereq->plr_level > 1 ) {
	  sprintf( buf3, "  level=%3d", prereq->plr_level );
	  strcat( spaces, "           " );
	}
	else
	  buf3[0] = '\0';
	sprintf( buf2,
		 " {gLevel %d {r[cost=%2d%s%s%s]:{x ", 
		 i, prereq->cost, buf3,
		 (prereq->classes!=ANY_CLASSES)?"  classes=":"",
		 (prereq->classes!=ANY_CLASSES)?flag_string(classes_flags, prereq->classes):""); // SinaC 2003
	strcat( spaces, "                    "); 
      }
      else {
	strcat( spaces, " ");
	buf2[0] = '\0';
      }
      
      if ( pAbility->prereqs[i].nb_prereq == 0 )
	strcat( buf2, " no prerequisites.\n\r");
      else {
	for ( int j = 0; j < pAbility->prereqs[i].nb_prereq; j++ ) {
	  char buf4[MAX_STRING_LENGTH];
	  buf4[0] = '\0';
	  if ( pAbility->prereqs[i].prereq[j].casting_level > 0 )
	    sprintf( buf4, " level %d", pAbility->prereqs[i].prereq[j].casting_level );
	  sprintf( buf3,
		   "%s{y'%s'{g%s{x\n\r",
		   j == 0 && i != 0 ? "" : spaces,
		   capitalize(ability_table[pAbility->prereqs[i].prereq[j].sn].name),
		   buf4 );
	  strcat( buf2, buf3 );
	}
      }
      strcat( buf, buf2 );
    }
    add_buf(output,buf);
  }

  if ( pAbility->type == TYPE_SKILL
       || pAbility->type == TYPE_SPELL
       || pAbility->type == TYPE_POWER
       || pAbility->type == TYPE_SONG ) {
    add_buf(output, "{WClasses :{x\n\r" );
    int col = 0;
    for ( i = 0; i < MAX_CLASS; i++ ) {
      //      if ( ( level = ability_table[sn].ability_level[i] ) == IM )
      //	sprintf( buf, "  {y%-20s{x  level : N/A", class_table[i].name );
      //      else  
      //	sprintf( buf, "  {y%-20s{x  level : {c%3d{x", class_table[i].name, level );
      //if ( col % 2 != 0 )
      //	strcat( buf, "\n\r" );
      //      else
      //	strcat( buf, "     " );
      if ( ( level = ability_table[sn].ability_level[i] ) < IM ) {
      	sprintf( buf, "  {y%-20s{x  level : {c%3d{x", capitalize(class_table[i].name), level );
	if ( col % 2 != 0 )
	  strcat( buf, "\n\r" );
	else
	  strcat( buf, "     " );
	add_buf(output,buf);
	col++;
      }
    }
    if ( col > 0 ) {
      if ( col % 2 != 0 )
	add_buf(output, "\n\r" );
    }
    else
      add_buf(output,"  {RNot available for any classes{x\n\r");
    
    bool found = FALSE;
    sprintf( buf, "{WNatural born ability for :{c " );
    for ( i = 0; i < MAX_PC_RACE; i++ ) {
      for ( int j = 0; j < pc_race_table[i].nb_abilities; j++ ) {
	if ( pc_race_table[i].abilities[j] <= 0 )
	  continue;
	if ( pc_race_table[i].abilities[j] == sn ) {
	  sprintf( buf2, "%s ", capitalize(pc_race_table[ i ].name) );
	  strcat( buf, buf2 );
	  found = TRUE;
	  break;
	}
      }
    }
    //if ( !found ) 
    //  strcat( buf, "none" );
    if ( found ) {
      strcat( buf, "{x\n\r" );
      add_buf(output,buf);
    }
  }
    
  return;  
}

// SinaC 2003
void triggerHelp( const TriggerDescr *trigger, BUFFER *output ) {
  char buf[MAX_STRING_LENGTH];

  sprintf( buf, "{WTrigger: {y%s", trigger->name );
  add_buf( output, buf );

  buf[0] = '\0';
  if ( trigger->nparms == -1 )
    strcpy( buf,"( variable number of parameters )");
  else if ( trigger->nparms == 0 )
    strcpy( buf, "()" );
  else {
    strcpy( buf, "(" );
    for ( int i = 0; i < trigger->nparms; i++ ) {
      char buf2[MAX_INPUT_LENGTH];
      if ( trigger->parmname[i] == NULL )
	strcpy( buf2, " no name" );
      else
	sprintf( buf2," %s", trigger->parmname[i] );
      strcat( buf, buf2 );
      if ( i+1 < trigger->nparms )
	strcat( buf, "," );
    }
    strcat( buf, " )");
  }
  strcat( buf, "{x\n\r");
  add_buf( output, buf );

  sprintf( buf, "{WMinimal position: {y%s{x\n\r", flag_string( position_flags, trigger->min_pos ) );
  add_buf( output, buf );

  sprintf( buf, "'TRIGGER_%s'", trigger->name );
  small_help( buf, output );

  add_buf( output, "\n\r" );
  return;
}

void godHelp( const int godId, BUFFER *output ) {
  char buf[MAX_STRING_LENGTH];
  god_data *god = &(gods_table[godId]);

  // Name + title
  sprintf( buf, "{WGod: {y%s, %s{x\n\r", capitalize(god->name), god->title );
  add_buf( output, buf );
  // Story
  add_buf( output, god->story );
  add_buf( output, "\n\r" );
  // Align
  add_buf(output,
	  "Alignments allowed:{W");
  if ( god->nb_allowed_align == 9 )
    add_buf(output, " Any");
  else
    for ( int i = 0; i < god->nb_allowed_align; i++ ) {
      int etho = god->allowed_align[i].etho + 1;
      int align = god->allowed_align[i].alignment/350 + 1;
      int index = etho + align * 3;
      sprintf( buf, " %2s", short_etho_align[index] );
      add_buf( output, buf );
    }
  add_buf(output,
	  "{x\n\r" );
  // Classes
  add_buf(output,
	  "Classes allowed:{W" );
  int allowed_class = god->allowed_class;
  if ( count_bit( allowed_class ) == MAX_CLASS )
    add_buf( output, " Any" );
  else
    for ( long i = 0; i < MAX_CLASS; i++ )
      if ( ( 1 << i ) & allowed_class ) {
	sprintf( buf, "  %s", capitalize(class_table[i].name) );
	add_buf( output, buf );
      }
  add_buf( output, "{x\n\r" );
  // Races
  add_buf( output, "Races allowed: {W" );
  for ( int i = 0; i < god->nb_allowed_race; i++ ) {
    sprintf( buf, "  %s", capitalize(race_table[god->allowed_race[i]].name) );
  }
  add_buf( output, "{x\n\r" );
  // Minor sphere
  sprintf( buf, "Minor sphere: {W%s{x\n\r",
	   god->minor_sphere > 0 ? capitalize(group_table[god->minor_sphere].name):"no minor sphere");
  add_buf( output, buf );
  // Major sphere
  add_buf( output, "Major: {W");
  int col = 0;
  for ( int j = 0; j < god->nb_major_sphere; j++ ) {
    sprintf( buf, "  %s", capitalize(group_table[god->major_sphere[j]].name) );
    add_buf( output, buf );
    if ( ++col % 3 == 0 )
      add_buf( output, "\n\r         ");
  }
  if ( col % 3 != 0 )
    add_buf( output, "{x\n\r");
  // Priest
  add_buf( output, "Classes getting minor sphere (Priest): {W");
  int priest = god->priest;
  for ( long j = 0; j < MAX_CLASS; j++ )
    if ( ( 1 << j ) & priest ) {
      sprintf( buf, "  %s", capitalize(class_table[j].name) );
      add_buf( output, buf );
    }
  add_buf( output, "{x\n\r" );

  return;
}


// Added by SinaC 2003
void do_showinfo( CHAR_DATA *ch, const char *argument ) {
  MOBPROG(ch,NULL,"onInformation",argument); // just call a mobprogram
}
