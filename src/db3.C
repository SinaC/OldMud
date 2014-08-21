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
#include "db3.h"
//#include "olc.h"
#include "tables.h"
#include "db.h"
#include "handler.h"
#include "special.h"
#include "clan.h"
#include "comm.h"
#include "olc_value.h"
#include "names.h"
#include "interp.h"
#include "lookup.h"
#include "scrhash.h"
#include "dbscript.h"
#include "abilityupgrade.h"
#include "recycle.h"
#include "dbdata.h"
#include "bit.h"
#include "config.h"
#include "mem.h"
#include "faction.h"


void assign_race_flags( MOB_INDEX_DATA *pMobIndex, const int race ) {
  pMobIndex->act	  = race_table[race].act | ACT_IS_NPC;
  pMobIndex->affected_by  = race_table[race].aff;
  pMobIndex->affected2_by = race_table[race].aff2;
  pMobIndex->off_flags    = race_table[race].off;
  pMobIndex->imm_flags    = race_table[race].imm;
  pMobIndex->res_flags    = race_table[race].res;
  pMobIndex->vuln_flags   = race_table[race].vuln;
  pMobIndex->form         = race_table[race].form;
  pMobIndex->parts        = race_table[race].parts;
}
void parse_mobile_dice( DATAData *diceD, const char *type, const int vnum, int dice[3] ) {
  ValueList *list = diceD->value->eval().asList();

  if ( list->size != 3 ) {
    bug("Wrong number of elements in %s dice for mob %d, <nbr dice><dice val><bonus>", type, vnum );
    for ( int i = 0; i < 3; i++ )
      dice[i] = 0;
  }

  dice[DICE_NUMBER] = list->elems[0].asInt();
  dice[DICE_TYPE] = list->elems[1].asInt();
  dice[DICE_BONUS] = list->elems[2].asInt();

  list->explicit_free();
}
long parse_mobile_difference( DATAData *field, const int vnum, struct flag_type *flag_table ) {
  int flag = 0;
  if ( ( flag = list_flag_value_complete( field->value, flag_table ) ) == NO_FLAG ) {
    bug("Invalid %s for mob %d, assuming none", get_flag_table_name( flag_table), vnum );
    flag = 0;
  }
  return flag;
}
void parse_mobile( DATAData *mobD, AREA_DATA *pArea ) {
  MOB_INDEX_DATA *pMobIndex;

  int vnum = mobD->value->eval().asInt(); // get vnum

  fBootDb = FALSE;
  if ( get_mob_index( vnum ) != NULL ) { // check vnum
    bug( "Load_mobiles: vnum %d duplicated.", vnum );
    exit( 1 );
  }
  fBootDb = TRUE;
  
  pMobIndex = new_mob_index();
  pMobIndex->vnum               = vnum;
  
  pMobIndex->area               = area_last; // OLC
  pMobIndex->new_format		= TRUE;
  newmobs++;

if ( DATA_VERBOSE > 1 ) {
  printf("--MOB: %d\n\r", pMobIndex->vnum );
}

  // fields
  for ( int fieldIndex = 0; fieldIndex < mobD->fields_count; fieldIndex++ ) {
    DATAData *field = mobD->fields[fieldIndex];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Name: pMobIndex->player_name = str_dup(field->value->eval().asStr() ); break;
    case TAG_ShD: pMobIndex->short_descr = str_dup(field->value->eval().asStr() ); break;
    case TAG_LnD: pMobIndex->long_descr = str_dup(field->value->eval().asStr() ); break;
    case TAG_Desc: pMobIndex->description = str_dup(field->value->eval().asStr() ); break;
    case TAG_Race: {
      const char *s = field->value->eval().asStr();
      if ( ( pMobIndex->race = flag_value_complete( races_flags, s ) ) == NO_FLAG ) {
	bug("Invalid race [%s] for mob %d, assuming unique", s, vnum );
	pMobIndex->race = flag_value_complete( races_flags, "unique" );
      }
      else
	assign_race_flags( pMobIndex, pMobIndex->race );
      break;
    }
    case TAG_Alignment: pMobIndex->align.alignment = field->value->eval().asInt(); break;
    case TAG_Etho: {
      const char *s = field->value->eval().asStr();
      if ( ( pMobIndex->align.etho = flag_value_complete( etho_flags, s ) ) == NO_FLAG ) {
	bug("Invalid etho %s for mob %d, assuming neutral", s, vnum );
	pMobIndex->align.etho = 0;
      }
      break;
    }
    case TAG_Level: pMobIndex->level = field->value->eval().asInt(); break;
    case TAG_Hitroll: pMobIndex->hitroll = field->value->eval().asInt(); break;
    case TAG_Hit: parse_mobile_dice( field, "hit", vnum, pMobIndex->hit ); break;
    case TAG_Mana: parse_mobile_dice( field, "mana", vnum, pMobIndex->mana ); break;
    case TAG_Psp: parse_mobile_dice( field, "psp", vnum, pMobIndex->psp ); break;
    case TAG_Damage: parse_mobile_dice( field, "damage", vnum, pMobIndex->damage ); break;
    case TAG_DamType: {
      const char *s = field->value->eval().asStr();
      //if ( ( pMobIndex->dam_type = attack_lookup( s ) ) == 0 ) {
      //bug("Invalid dam type %s for mob %d, assuming punch", s, vnum );
      //pMobIndex->dam_type = attack_lookup("punch");
      //}
      pMobIndex->dam_type = attack_lookup( s ); // no check
      break;
    }
    case TAG_ACs: {
      ValueList *ac = field->value->eval().asList();
      if ( ac->size != 4 ) {
	bug("Wrong number of elements in ACs for mob %d, assuming 0 for each AC type", vnum );
	for ( int i = 0; i < 4; i++ )
	  pMobIndex->ac[AC_PIERCE+i] = 0;
	break;
      }
      for ( int i = 0; i < 4; i++ )
	pMobIndex->ac[AC_PIERCE+i] = ac->elems[i].asInt() * 10;

      ac->explicit_free();
      break;
    }
    case TAG_Position: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 2 ) {
	bug("Wrong number of elements in position for mob %d, assuming STANDING", vnum );
	pMobIndex->start_pos = pMobIndex->default_pos = POS_STANDING;
	break;
      }
      const char *s1 = list->elems[0].asStr();
      if ( ( pMobIndex->start_pos = flag_value_complete( position_flags, s1 ) ) == NO_FLAG ) {
	bug("Invalid start position %s for mob %d, assuming STANDING", s1, vnum );
	pMobIndex->start_pos = POS_STANDING;
      }
      const char *s2 = list->elems[1].asStr();
      if ( ( pMobIndex->start_pos = flag_value_complete( position_flags, s2 ) ) == NO_FLAG ) {
	bug("Invalid start position %s for mob %d, assuming STANDING", s2, vnum );
	pMobIndex->start_pos = POS_STANDING;
      }

      list->explicit_free();
      break;
    }
    case TAG_Sex: {
      const char *s = field->value->eval().asStr();
      if ( ( pMobIndex->sex = flag_value_complete( sex_flags, s ) ) == NO_FLAG ) {
	bug("Invalid sex %s for mob %d, assuming male", s, vnum );
	pMobIndex->sex = 0;
      }
      break;
    }
    case TAG_Wealth: pMobIndex->wealth = field->value->eval().asInt(); break;
    case TAG_Size: {
      const char *s = field->value->eval().asStr();
      if ( ( pMobIndex->size = flag_value_complete( size_flags, s ) ) == NO_FLAG ) {
	bug("Invalid size %s for mob %d, assuming MEDIUM", s, vnum );
	pMobIndex->size = SIZE_MEDIUM;
      }
      break;
    }
    case TAG_Material: pMobIndex->material = str_dup( field->value->eval().asStr() ); break;
    case TAG_Classes: {
      if ( ( pMobIndex->classes = list_flag_value_complete( field->value, classes_flags ) ) == NO_FLAG ) {
	bug("Invalid classes for mob %d, assuming no class", vnum );
	pMobIndex->classes = 0;
      }
      break;
    }
    case TAG_Program: {
      const char *s = field->value->eval().asStr();
      pMobIndex->program = hash_get_prog(s);
      if (!pMobIndex->program)
	bug("Can't find program %s for mob %d.", s, vnum );	      
      else {
	if ( get_root_class( pMobIndex->program ) != default_mob_class ) {
	  bug("program %s for mob %d is not a mob program.", s, vnum );
	  pMobIndex->program = NULL;
	}
	else
	  if ( pMobIndex->program->isAbstract )
	    bug("program for mob vnum %d is an ABSTRACT class.", vnum );
      }
      break;
    }
    case TAG_Group: pMobIndex->group = field->value->eval().asInt(); break;
      //    case TAG_Act: pMobIndex->act ^= parse_mobile_difference( field, vnum, act_flags ); break;
      //    case TAG_AfBy: pMobIndex->affected_by ^= parse_mobile_difference( field, vnum, affect_flags ); break;
      //    case TAG_AfBy2: pMobIndex->affected2_by ^= parse_mobile_difference( field, vnum, affect2_flags ); break;
      //    case TAG_Offensive: pMobIndex->off_flags ^= parse_mobile_difference( field, vnum, off_flags ); break;
      //    case TAG_Immunities: pMobIndex->imm_flags ^= parse_mobile_difference( field, vnum, irv_flags ); break;
      //    case TAG_Resistances: pMobIndex->res_flags ^= parse_mobile_difference( field, vnum, irv_flags ); break;
      //    case TAG_Vulnerabilities: pMobIndex->vuln_flags ^= parse_mobile_difference( field, vnum, irv_flags ); break;
      //    case TAG_Forms: pMobIndex->form ^= parse_mobile_difference( field, vnum, form_flags ); break;
      //    case TAG_Parts: pMobIndex->parts ^= parse_mobile_difference( field, vnum, part_flags ); break;
    case TAG_Act: REMOVE_BIT( pMobIndex->act, parse_mobile_difference( field, vnum, act_flags ) ); break;
    case TAG_AfBy: REMOVE_BIT( pMobIndex->affected_by, parse_mobile_difference( field, vnum, affect_flags ) ); break;
    case TAG_AfBy2: REMOVE_BIT( pMobIndex->affected2_by, parse_mobile_difference( field, vnum, affect2_flags ) ); break;
    case TAG_Offensive: REMOVE_BIT( pMobIndex->off_flags, parse_mobile_difference( field, vnum, off_flags ) ); break;
    case TAG_Immunities: REMOVE_BIT( pMobIndex->imm_flags, parse_mobile_difference( field, vnum, irv_flags ) ); break;
    case TAG_Resistances: REMOVE_BIT( pMobIndex->res_flags, parse_mobile_difference( field, vnum, irv_flags ) ); break;
    case TAG_Vulnerabilities: REMOVE_BIT( pMobIndex->vuln_flags, parse_mobile_difference( field, vnum, irv_flags ) ); break;
    case TAG_Forms: REMOVE_BIT( pMobIndex->form, parse_mobile_difference( field, vnum, form_flags ) ); break;
    case TAG_Parts: REMOVE_BIT( pMobIndex->parts, parse_mobile_difference( field, vnum, part_flags ) ); break;
    case TAG_Faction: {
      const char *s = field->value->eval().asStr();
      if ( ( pMobIndex->faction = get_faction_id(s) ) < 0 ) {
	set_default_faction(pMobIndex);
	bug("Invalid faction [%s] for mob [%d], assuming default_faction [%s]", s, pMobIndex->vnum, faction_table[pMobIndex->faction].name );
      }
    }
    case TAG_Special: {
      const char *s = field->value->eval().asStr();
      if ( ( pMobIndex->spec_fun = spec_lookup( s ) ) == NULL )
	bug("Invalid special function [%s] for mob [%d], assuming none", s, vnum );
      break;
    }
    case TAG_Shop: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 5 ) {
	bug("Invalid number of elements in shop for mob %d, assuming no shop", vnum );
	break;
      }
      SHOP_DATA *pShop;
      pShop = new_shop();
      pShop->keeper		= vnum;
      if ( pShop->keeper == 0 )
	break;
      // buy_type
      ValueList *itemList = list->elems[0].asList();
      int size = itemList->size;
      if ( size > MAX_TRADE ) {
	bug("Too many item type in buy type for shop for mob %d.", vnum );
	size = MAX_TRADE;
      }
      for ( int iTrade = 0; iTrade < MAX_TRADE; iTrade++ ) // clear buy_type
	pShop->buy_type[iTrade] = 0;
      for ( int iTrade = 0; iTrade < size; iTrade++ ) {
	const char *s = itemList->elems[iTrade].asStr();
	if ( ( pShop->buy_type[iTrade] = flag_value_complete( type_flags, s ) ) == NO_FLAG ) {
	  bug("Invalid item type %s in buy type for shop for mob %d.", s, vnum );
	  pShop->buy_type[iTrade] = 0;
	}
      }
      itemList->explicit_free();

      // profit & hours
      pShop->profit_buy	= list->elems[1].asInt();
      pShop->profit_sell = list->elems[2].asInt();
      pShop->open_hour	= list->elems[3].asInt();
      pShop->close_hour	= list->elems[4].asInt();
      pMobIndex->pShop	= pShop;
      
      if ( shop_first == NULL )
	shop_first = pShop;
      if ( shop_last  != NULL )
	shop_last->next = pShop;
      
      shop_last	= pShop;
      pShop->next	= NULL;

      list->explicit_free();
    break;
    }
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }

  pMobIndex->act |= ACT_IS_NPC;

  int iHash               = vnum % MAX_KEY_HASH;
  pMobIndex->next         = mob_index_hash[iHash];
  mob_index_hash[iHash]   = pMobIndex;
  top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  // OLC 
  assign_area_vnum( vnum );                                  // OLC 
  kill_table[URANGE(0, pMobIndex->level, MAX_LEVEL-1)].number++;
}

bool check_enough_elements( ValueList *list, const int wantedSize, const int vnum ) {
  if ( list->size != wantedSize ) {
    bug("Wrong number of elements for values for obj %d", vnum );
    return FALSE;
  }
  return TRUE;
}
void parse_object_values( DATAData *values, OBJ_INDEX_DATA *pObjIndex ) {
  ValueList *list = values->value->eval().asList();
  const int vnum = pObjIndex->vnum;

  switch ( pObjIndex->item_type ) {
  case ITEM_ARMOR: case ITEM_WINDOW:  // armor: 0->4, window: 0
  case ITEM_MONEY: default:           // money: 0, 1
    for ( int i = 0; i < list->size; i++ )
      pObjIndex->value[i] = list->elems[i].asInt();
    break;
  case ITEM_TRASH: { // special case, because we set item_type to TRASH if an error occured
    for ( int i = 0; i < list->size; i++ ) {
      Value v = list->elems[i];
      if ( v.typ == SVT_INT )
	pObjIndex->value[i] = v.asInt();
      else
	pObjIndex->value[i] = 0;
    }
    break;
  }
  case ITEM_COMPONENT: { // SinaC 2003
    check_enough_elements( list, 5, vnum );
    for ( int i = 0; i < 5; i++ ) {
      const char *s = list->elems[i].asStr();
      if ( !str_cmp( s, "none" ) )
	continue;
      pObjIndex->value[i] = flag_value(brew_component_flags, s );
      if ( pObjIndex->value[i] == NO_FLAG )
	bug("Invalid %d° component [%s] for item %d", i, s, vnum );
    }
    break;
  }
  case ITEM_LIGHT: 
    check_enough_elements( list, 1, vnum );
    pObjIndex->value[2] = list->elems[0].asInt(); 
    break;
  case ITEM_DRINK_CON: {
    check_enough_elements( list, 4, vnum );
    const char *s = list->elems[2].asStr();
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[1] = list->elems[1].asInt();
    pObjIndex->value[3] = list->elems[3].asInt();
    if ( ( pObjIndex->value[2] = liq_lookup( s ) ) == NO_FLAG ) {
      bug("Invalid liquid %s for drink container %d, assuming 0", s, vnum );
      pObjIndex->value[2] = 0;
    }
    break;
  }
  case ITEM_FOUNTAIN: {
    check_enough_elements( list, 3, vnum );
    const char *s = list->elems[2].asStr();
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[1] = list->elems[1].asInt();
    if ( ( pObjIndex->value[2] = liq_lookup( s ) ) == NO_FLAG ) {
      bug("Invalid liquid %s for fountain %d, assuming 0", s, vnum );
      pObjIndex->value[2] = 0;
    }
    break;
  }
  case ITEM_CONTAINER:
    check_enough_elements( list, 5, vnum );
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[2] = list->elems[2].asInt();
    pObjIndex->value[3] = list->elems[3].asInt();
    pObjIndex->value[4] = list->elems[4].asInt();
    if ( ( pObjIndex->value[1] = list_flag_value_complete( list->elems[1].asList(), container_flags ) ) == NO_FLAG ) {
      bug("Invalid flags for container %d, assuming none", vnum );
      pObjIndex->value[1] = 0;
    }
    break;
  case ITEM_FOOD:
    check_enough_elements( list, 3, vnum );
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[1] = list->elems[1].asInt();
    pObjIndex->value[3] = list->elems[2].asInt();
    break;
  case ITEM_PORTAL:
    check_enough_elements( list, 4, vnum );
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[3] = list->elems[3].asInt();
    if ( ( pObjIndex->value[1] = list_flag_value_complete( list->elems[1].asList(), exit_flags ) ) == NO_FLAG ) {
      bug("Invalid exit flags for portal %d, assuming none", vnum );
      pObjIndex->value[1] = 0;
    }
    if ( ( pObjIndex->value[2] = list_flag_value_complete( list->elems[2].asList(), portal_flags ) ) == NO_FLAG ) {
      bug("Invalid flags for portal %d, assuming none", vnum );
      pObjIndex->value[2] = 0;
    }
    break;
  case ITEM_FURNITURE:
    check_enough_elements( list, 5, vnum );
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[1] = list->elems[1].asInt();
    pObjIndex->value[3] = list->elems[3].asInt();
    pObjIndex->value[4] = list->elems[4].asInt();
    if ( ( pObjIndex->value[2] = list_flag_value_complete( list->elems[2].asList(), furniture_flags ) ) == NO_FLAG ) {
      bug("Invalid flags for furniture %d, assuming none", vnum );
      pObjIndex->value[2] = 0;
    }
    break;
  case ITEM_WEAPON: {
    check_enough_elements( list, 5, vnum );
    const char *s1 = list->elems[0].asStr();
    const char *s2 = list->elems[3].asStr();
    pObjIndex->value[1] = list->elems[1].asInt();
    pObjIndex->value[2] = list->elems[2].asInt();
    if ( ( pObjIndex->value[0] = flag_value_complete( weapon_class, s1 ) ) == NO_FLAG ) {
      bug("Invalid weapon class %s for weapon %d, assuming exotic", s1, vnum );
      pObjIndex->value[0] = WEAPON_EXOTIC;
    }
    if ( pObjIndex->value[0] == WEAPON_RANGED ) // SinaC 2003
      pObjIndex->value[3] = list->elems[3].asInt();
    else
      pObjIndex->value[3] = attack_lookup( s2 ); // no check
    //if ( ( pObjIndex->value[3] = attack_lookup( s2 ) ) == 0 ) {
    //  bug("Invalid attack %s for weapon %d, assuming none", s2, vnum );
    //  pObjIndex->value[3] = 1;
    //}
    if ( pObjIndex->value[0] == WEAPON_RANGED ) // SinaC 2003
      pObjIndex->value[4] = list->elems[4].asInt();
    else
      if ( ( pObjIndex->value[4] = list_flag_value_complete( list->elems[4].asList(), weapon_type2 ) ) == NO_FLAG ) {
	bug("Invalid flags for weapon %d, assuming none", vnum );
	pObjIndex->value[4] = 0;
      }
    break;
  }
  case ITEM_PILL:
  case ITEM_POTION:
  case ITEM_SCROLL:
  case ITEM_TEMPLATE: {
    check_enough_elements( list, 5, vnum );
    pObjIndex->value[0] = list->elems[0].asInt();
    for ( int i = 1; i < list->size; i++ ) {
      const char *s = list->elems[i].asStr();
      if ( !str_cmp( s, "none" ) )
	continue;
      const int sn = ability_lookup( s );
      if ( sn < 0 ) {
	bug("Invalid %d) ability %s for pill/potion/scroll/template %d", i, s, vnum );
	continue;
      }
      pObjIndex->value[i] = sn;
    }
    break;
  }
  case ITEM_STAFF:
  case ITEM_WAND: {
    check_enough_elements( list, 4, vnum );
    pObjIndex->value[0] = list->elems[0].asInt();
    pObjIndex->value[1] = list->elems[1].asInt();
    pObjIndex->value[2] = list->elems[2].asInt();
    const char *s = list->elems[3].asStr();
    if ( !str_cmp( s, "none" ) )
      break;
    const int sn = ability_lookup( s );
    if ( sn < 0 ) {
      bug("Invalid ability %s for staff/wand %d", s, vnum );
      break;
    }
    pObjIndex->value[3] = sn;
    break;
  }
  }

  list->explicit_free();
}
void parse_object( DATAData *objD, AREA_DATA *pArea ) {
  OBJ_INDEX_DATA *pObjIndex;

  int vnum = objD->value->eval().asInt(); // get vnum

  fBootDb = FALSE;
  if ( get_obj_index( vnum ) != NULL ) // check vnum
    p_error( "Load_objects: vnum %d duplicated.", vnum );
  fBootDb = TRUE;

  pObjIndex = new_obj_index(); // create new room
  pObjIndex->vnum                 = vnum;
  pObjIndex->area                 = pArea; // OLC
  pObjIndex->new_format           = TRUE;
  pObjIndex->reset_num		= 0;
  newobjs++;

if ( DATA_VERBOSE > 1 ) {
  printf("--OBJ: %d\n\r", pObjIndex->vnum );
}

  // fields
  for ( int fieldIndex = 0; fieldIndex < objD->fields_count; fieldIndex++ ) {
    DATAData *field = objD->fields[fieldIndex];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Name: pObjIndex->name = str_dup(field->value->eval().asStr() ); break;
    case TAG_ShD: pObjIndex->short_descr = str_dup(field->value->eval().asStr() ); break;
    case TAG_Desc: pObjIndex->description = str_dup(field->value->eval().asStr() ); break;
    case TAG_Material: {
      const char *s = field->value->eval().asStr();
      if ( ( pObjIndex->material = flag_value_complete( material_flags, s ) ) == NO_FLAG ) {
	bug("Invalid material %s for obj %d, assuming <not entered>", s, vnum );
	pObjIndex->material = 0;
      }
      break;
    }
    case TAG_ItemType: {
      const char *s = field->value->eval().asStr();
      if ( ( pObjIndex->item_type = flag_value_complete( type_flags, s ) ) == NO_FLAG ) {
	bug("Invalid item type %s for obj %d, assuming trash", s, vnum );
	pObjIndex->item_type = ITEM_TRASH;
      }
      break;
    }
    case TAG_Level: pObjIndex->level = field->value->eval().asInt(); break;
    case TAG_Weight: pObjIndex->weight = field->value->eval().asInt(); break;
    case TAG_Cost: pObjIndex->cost = field->value->eval().asInt(); break;
    case TAG_Program: {
      const char *s = field->value->eval().asStr();
      pObjIndex->program = hash_get_prog(s);
      if (!pObjIndex->program)
	bug("Can't find program %s for obj %d.", s, vnum );	      
      else {
	if ( get_root_class( pObjIndex->program ) != default_obj_class ) {
	  bug("program %s for obj %d is not an obj program.", s, vnum );
	  pObjIndex->program = NULL;
	}
	else
	  if ( pObjIndex->program->isAbstract )
	    bug("program for room vnum %d is an ABSTRACT class.", vnum );
      }
      break;
    }
    case TAG_Condition: pObjIndex->condition = field->value->eval().asInt(); break;
    case TAG_Size: {
      const char *s = field->value->eval().asStr();
      if ( ( pObjIndex->size = flag_value_complete( size_flags, s ) ) == NO_FLAG ) {
	bug("Invalid size %s for obj %d, assuming NOSIZE", s, vnum );
	pObjIndex->size = SIZE_NOSIZE;
      }
      break;
    }
    case TAG_ExtraFlags: {
      if ( ( pObjIndex->extra_flags = list_flag_value_complete( field->value, extra_flags ) ) == NO_FLAG ) {
	bug("Invalid extra flags for obj %d, assuming none", vnum );
	pObjIndex->extra_flags = 0;
      }
      break;
    }
    case TAG_WearFlags: {
      if ( ( pObjIndex->wear_flags = list_flag_value_complete( field->value, wear_flags ) ) == NO_FLAG ) {
	bug("Invalid wear flags for obj %d, assuming none", vnum );
	pObjIndex->wear_flags = 0;
      }
      break;
    }
    case TAG_Values: parse_object_values( field, pObjIndex ); break;
    case TAG_Restriction: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 4 ) {
	bug("Wrong number of elements in restriction <kind><type name><value><not r?> for obj %d", vnum );
	break;
      }
      int kind = list->elems[0].asInt();
      RESTR_DATA *restr = new_restriction();
      if ( kind == 0 ) {
	restr->type = flag_value_complete( restr_flags, list->elems[1].asStr() );
	if ( restr->type == NO_FLAG ) {
	  bug("Invalid restriction type");
	  break;
	}
	//restr->ability_r = FALSE;
	restr->sn = -1;
	restr->value = list->elems[2].asInt();
	restr->not_r = list->elems[3].asInt();
      }
      else if ( kind == 1 ) {
	restr->type = RESTR_ABILITY;
	//restr->ability_r = TRUE;
	const char *s = list->elems[1].asStr();
	restr->sn = ability_lookup( s );
	if ( restr->sn < 0 ) {
	  bug("Invalid ability restriction %s", s );
	  break;
	}
	restr->value = list->elems[2].asInt();
	restr->not_r = list->elems[3].asInt();
      }
      else {
	bug("Invalid restriction kind");
	break;
      }

      restr->next                = pObjIndex->restriction;
      pObjIndex->restriction     = restr;

      list->explicit_free();
      break;
    }
    case TAG_Upgrade: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 2 ) {
	bug("Wrong number of elements in upgrade <ability><value> for obj %d", vnum );
	break;
      }
      const char *s = list->elems[0].asStr();
      const int sn = ability_lookup( s );
      if ( sn < 0 ) {
	bug("Invalid ability %s in upgrade for obj %d", s, vnum );
	break;
      }
      ABILITY_UPGRADE *upgr = new_ability_upgrade();
      upgr->sn = sn;
      upgr->value = list->elems[1].asInt();

      upgr->next             = pObjIndex->upgrade;
      pObjIndex->upgrade        = upgr;

      list->explicit_free();
      break;
    }
    case TAG_Affect: {
      AFFECT_DATA *paf = new_affect();
      if ( parse_affect( field, paf ) ) { // add affect in list only if no error
	paf->next               = pObjIndex->affected;
	pObjIndex->affected     = paf;
      }
      break;
    }
    case TAG_ExtraDescr: {
      ValueList *couple = field->value->eval().asList();
      if ( couple->size != 2 ) {
	bug("Wrong number of elements in extra description <keyword><description> for obj %d", vnum );
	break;
      }
      EXTRA_DESCR_DATA *ed = new_extra_descr();
      ed->keyword		= str_dup(couple->elems[0].asStr());
      ed->description		= str_dup(couple->elems[1].asStr());
      ed->next		= pObjIndex->extra_descr;
      pObjIndex->extra_descr	= ed;

      couple->explicit_free();
      break;
    }
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }

  int iHash                   = vnum % MAX_KEY_HASH;
  pObjIndex->next         = obj_index_hash[iHash];
  obj_index_hash[iHash]   = pObjIndex;
  top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;   // OLC
  assign_area_vnum( vnum );                                   // OLC
}

// iLastRoom & iLastObj are not needed because we are sure of the room
void parse_room_reset( DATAData *resets, ROOM_INDEX_DATA *pRoomIndex ) {
  OBJ_INDEX_DATA *temp_index;
  ValueList *list = resets->value->eval().asList();
  int vnum = pRoomIndex->vnum;
  char letter = list->elems[0].asStr()[0]; // get first letter from first list's elem

  RESET_DATA *pReset;
  pReset = new_reset_data();
  pReset->command = letter;
  pReset->arg1 = pReset->arg2 = pReset->arg3 = pReset->arg4 = 0;
  
  switch ( letter ) {
  default:
    bug("Invalid reset '%c' for room %d", letter, vnum );
    break;
  case 'M':
    if ( list->size != 4 ) {
      bug("Wrong number of elements in reset 'M' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg2 = list->elems[2].asInt();
    pReset->arg3 = vnum;
    pReset->arg4 = list->elems[3].asInt();
    
    if ( get_mob_index( pReset->arg1 ) == NULL )
      p_error("Invalid mobile %d in reset 'M' for room %d", pReset->arg1, vnum );
    if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
      bug("reset M has arg2 or arg4 equal to 0 (room: %d)", vnum );
    new_reset( pRoomIndex, pReset );
    break;

  case 'O': 
    if ( list->size != 2 ) {
      bug("Wrong number of elements in reset 'O' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg3 = vnum;

    if ( ( temp_index = get_obj_index( pReset->arg1 ) ) == NULL )
      p_error("Invalid object %d in reset 'O' for room %d", pReset->arg1, vnum );
    temp_index->reset_num++; 
    new_reset( pRoomIndex, pReset );
    break;
    
  case 'P':
    if ( list->size != 5 ) {
      bug("Wrong number of elements in reset 'P' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg2 = list->elems[2].asInt();
    pReset->arg3 = list->elems[3].asInt();
    pReset->arg4 = list->elems[4].asInt();
    
    if ( ( temp_index = get_obj_index( pReset->arg1 ) ) == NULL )
      p_error("Invalid object %d in reset 'P' for room %d", pReset->arg1, vnum );
    temp_index->reset_num++;
    if ( get_obj_index( pReset->arg1 ) == NULL )
      p_error("Invalid container %d in reset 'P' for room %d", pReset->arg1, vnum );
    if ( pReset->arg2 == 0 || pReset->arg4 == 0 )
      bug("reset P has arg2 or arg4 equal to 0 (room: %d)", vnum );
    new_reset( pRoomIndex, pReset );
    break;
    
  case 'G':
    if ( list->size != 3 ) {
      bug("Wrong number of elements in reset 'G' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg2 = list->elems[2].asInt();

    temp_index = get_obj_index( pReset->arg1 );
    temp_index->reset_num++; 
    new_reset( pRoomIndex, pReset );
    break;
  case 'E': {
    if ( list->size != 4 ) {
      bug("Wrong number of elements in reset 'E' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg2 = list->elems[2].asInt();
    const char *s = list->elems[3].asStr();
    if ( ( pReset->arg3 = flag_value( wear_loc_flags, s ) ) == NO_FLAG ) {
      bug("Invalid wear location %s in reset 'E' for room %d, assuming none", s, vnum );
      pReset->arg3 = 0;
    }
    temp_index = get_obj_index( pReset->arg1 );
    temp_index->reset_num++; 
    new_reset( pRoomIndex, pReset );
    break;
  }
  case 'R':
    if ( list->size != 3 ) {
      bug("Wrong number of elements in reset 'R' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg2 = list->elems[2].asInt();
    
    if ( pReset->arg2 < 0 || pReset->arg2 >= MAX_DIR ) {
      bug( "bad exit %d in reset 'R' for room %d.", pReset->arg2, vnum );
      break;
    }
    new_reset( pRoomIndex, pReset );
    break;

  case 'Z':
    if ( list->size != 4 ) {
      bug("Wrong number of elements in reset 'Z' for room %d", vnum );
      break;
    }
    pReset->arg1 = list->elems[1].asInt();
    pReset->arg2 = list->elems[2].asInt();
    pReset->arg3 = vnum;
    pReset->arg4 = list->elems[3].asInt();

    if ( pReset->arg1 < 2 
	 || pReset->arg2 < 2 
	 || pReset->arg1 * pReset->arg2 > 100 ) {
      bug("bad width, height in reset 'Z' for room %d.", vnum );
      break;
    }
    if ( pReset->arg4 != 0 && !get_obj_index( pReset->arg4 ) ) {
      bug("bad map vnum %d for room %d", pReset->arg4, vnum );
      break;
    }
    new_reset( pRoomIndex, pReset );
    break;
  }

  list->explicit_free();
}
void parse_room( DATAData *roomD, AREA_DATA *pArea ) {
  ROOM_INDEX_DATA *pRoomIndex;

  int vnum = roomD->value->eval().asInt(); // get vnum

  fBootDb = FALSE;
  if ( get_room_index( vnum ) != NULL ) // check vnum
    p_error( "Load_rooms: vnum %d duplicated.", vnum );
  fBootDb = TRUE;
  
  pRoomIndex = new_room_index(); // create new room
  pRoomIndex->area = pArea;
  pRoomIndex->vnum = vnum;

if ( DATA_VERBOSE > 1 ) {
  printf("--ROOM: %d\n\r", pRoomIndex->vnum );
}

  // fields
  for ( int fieldIndex = 0; fieldIndex < roomD->fields_count; fieldIndex++ ) {
    DATAData *field = roomD->fields[fieldIndex];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Name: pRoomIndex->name = str_dup(field->value->eval().asStr() ); break;
    case TAG_Desc: pRoomIndex->description = str_dup(field->value->eval().asStr() ); break;
    case TAG_Sector: {
      const char *s = field->value->eval().asStr();
      if ( ( pRoomIndex->bstat(sector) = flag_value_complete( sector_flags, s ) ) == NO_FLAG ) {
	bug("Invalid room sector %s for room %d, assuming inside.", s, vnum );
	pRoomIndex->bstat(sector) = SECT_INSIDE;
      }
      break;
    }
    case TAG_Flags: {
      if ( ( pRoomIndex->bstat(flags) = list_flag_value_complete( field->value, room_flags ) ) == NO_FLAG ) {
	bug("Invalid room flags for room %d, assuming none", vnum );
	pRoomIndex->bstat(flags) = 0;
      }
      break;
    }
    case TAG_MaxSize: {
      const char *s = field->value->eval().asStr();
      if ( ( pRoomIndex->bstat(maxsize) = flag_value_complete( size_flags, s ) ) == NO_FLAG ) {
	bug("Invalid room maxsize %s for room %d, assuming NOSIZE", s, vnum );
	pRoomIndex->bstat(maxsize) = SIZE_NOSIZE;
      }
      break;
    }
    case TAG_Rate: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 3 ) {
	bug("Wrong number of elements in room rate for room %d, assuming 100%%", vnum );
	pRoomIndex->bstat(manarate) = pRoomIndex->bstat(healrate) = pRoomIndex->bstat(psprate) = 100;
	break;
      }
      pRoomIndex->bstat(manarate) = list->elems[0].asInt();
      pRoomIndex->bstat(healrate) = list->elems[1].asInt();
      pRoomIndex->bstat(psprate) = list->elems[2].asInt();

      list->explicit_free();
      break;
    }
    case TAG_Clan: {
      const char *s = field->value->eval().asStr();
      if ( ( pRoomIndex->clan = clan_lookup( s ) ) == 0 )
	bug("Invalid clan %s for room %d, assuming none.", s, vnum );
      break;
    }
    case TAG_Owner: pRoomIndex->owner = str_dup( field->value->eval().asStr() ); break;
    case TAG_Guilds: {
      if ( ( pRoomIndex->guild = list_flag_value_complete( field->value, classes_flags ) ) == NO_FLAG ) {
	bug("Invalid guilds for room %d, assuming none", vnum );
	pRoomIndex->guild = 0;
    }
      break;
    }
    case TAG_Program: {
      const char *s = field->value->eval().asStr();
      pRoomIndex->program = hash_get_prog(s);
      if (!pRoomIndex->program)
	bug("Can't find program %s for room %d.", s, vnum );	      
      else {
	if ( get_root_class( pRoomIndex->program ) != default_room_class ) {
	  bug("program %s for room %d is not a room program.", s, vnum );
	  pRoomIndex->program = NULL;
	}
	else
	  if ( pRoomIndex->program->isAbstract )
	    bug("program for room vnum %d is an ABSTRACT class.", vnum );
      }
      break;
    }
    case TAG_ExtraDescr: {
      ValueList *couple = field->value->eval().asList();
      if ( couple->size != 2 ) {
	bug("Wrong number of elements in extra description <keyword><description> for room %d", vnum );
	break;
      }
      EXTRA_DESCR_DATA *ed = new_extra_descr();
      ed->keyword		= str_dup(couple->elems[0].asStr());
      ed->description		= str_dup(couple->elems[1].asStr());
      ed->next		= pRoomIndex->extra_descr;
      pRoomIndex->extra_descr	= ed;

      couple->explicit_free();
      break;
    }
    case TAG_Exit: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 6 ) {
	bug("Wrong number of elements in exit for room %d", vnum );
	break;
      }
      int door = list->elems[0].asInt();
      if ( door < 0 || door >= MAX_DIR ) {
	bug( "Invalid door number %d for room %d.", door, vnum );
	break;
      }
      EXIT_DATA *pexit = new_exit();
      pexit->description = list->elems[1].asStr();
      pexit->keyword = list->elems[2].asStr();
      int flags;
      if ( ( flags = list_flag_value_complete( list->elems[3].asList(), exit_flags ) ) == NO_FLAG ) {
	bug("Invalid exit flags for room %d, assuming none", vnum );
	flags = 0;
      }
      pexit->rs_flags = pexit->exit_info = flags;
      pexit->key = list->elems[4].asInt();
      pexit->u1.vnum = list->elems[5].asInt();
      pexit->orig_door	= door;			// OLC
      pRoomIndex->exit[door]	= pexit;
      pRoomIndex->old_exit[door] = pexit;

      list->explicit_free();
      break;
    }
    case TAG_Repop: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 2 ) {
	bug("Wrong number of elements in repop for room %d, assuming default value", vnum );
	break;
      }
      pRoomIndex->time_between_repop = list->elems[0].asInt();
      pRoomIndex->time_between_repop_people = list->elems[1].asInt();

      list->explicit_free();
      break;
    }
    case TAG_Reset: parse_room_reset( field, pRoomIndex ); break;
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }


  // Added by SinaC 2003 for repop time
  pRoomIndex->current_time_repop = MAX_REPOP_TIME; // so the room will be immediatly updated
  if ( pRoomIndex->time_between_repop < MIN_REPOP_TIME
       || pRoomIndex->time_between_repop >= MAX_REPOP_TIME
       || pRoomIndex->time_between_repop_people < MIN_REPOP_TIME
       || pRoomIndex->time_between_repop_people >= MAX_REPOP_TIME ) {
    pRoomIndex->time_between_repop = BASE_REPOP_TIME;
    pRoomIndex->time_between_repop_people = BASE_REPOP_TIME_PEOPLE;
  }
  
  // Added by SinaC 2003 for room programs
  if ( pRoomIndex->program != NULL )
    pRoomIndex->clazz = pRoomIndex->program;
  else
    pRoomIndex->clazz = default_room_class;
  
  int iHash		= vnum % MAX_KEY_HASH;
  pRoomIndex->next	= room_index_hash[iHash];
  room_index_hash[iHash]	= pRoomIndex;
  top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room; // OLC
  assign_area_vnum( vnum );                                    // OLC
}

void nullify_area( AREA_DATA *pArea ) {
  pArea->age          = 15;
  pArea->nplayer      = 0;
  pArea->totalnplayer = 0;
  pArea->file_name    = str_dup( strArea );
  pArea->vnum         = top_area;
  pArea->name         = str_dup( "New Area" );
  pArea->builders     = str_dup( "" );
  pArea->security     = 9;
  pArea->min_vnum     = 0;
  pArea->max_vnum     = 0;
  pArea->area_flags   = AREA_LOADING;
  pArea->earthquake_on = FALSE;
  pArea->earthquake_duration = 0;
  pArea->low_range        = 500;
  pArea->high_range       = 0;
}
void parse_area( DATAData *areaD ) {
  AREA_DATA *pArea;
  
  pArea = new_area();
  nullify_area( pArea );

  pArea->name = str_dup( areaD->value->eval().asStr() );

  if ( area_first == NULL )
    area_first = pArea;
  if ( area_last ) { // Modified by SinaC 2003, seems to crash without that
    area_last->next = pArea;
    REMOVE_BIT(area_last->area_flags, AREA_LOADING);        // OLC
  }
  area_last   = pArea;
  pArea->next = NULL;


  // fields
  for ( int fieldIndex = 0; fieldIndex < areaD->fields_count; fieldIndex++ ) {
    DATAData *field = areaD->fields[fieldIndex];
    const char *tagName = field->tag->image;
    const int tagId = find_tag( tagName );
if ( DATA_VERBOSE > 4 ) {
      printf("tagName: %s  tagId: %d\n\r", tagName, tagId );
}
    switch( tagId ) {
    case TAG_Builders: pArea->builders = str_dup( field->value->eval().asStr() ); break;
    case TAG_Vnums: {
      ValueList *list = field->value->eval().asList();
      if ( list->size != 2 )
	p_error("Wrong number of elements in area vnums <min vnum> <max vnum>");
      pArea->min_vnum = list->elems[0].asInt();
      pArea->max_vnum = list->elems[1].asInt();

      list->explicit_free();
      break;
    }
    case TAG_Credits: pArea->credits = str_dup( field->value->eval().asStr() ); break;
    case TAG_Security: pArea->security = field->value->eval().asInt(); break;
    case TAG_Flags: {
      if ( ( pArea->area_flags = list_flag_value_complete( field->value, area_flags ) ) == NO_FLAG ) {
	bug("Invalid area flags for area %s, assuming none", pArea->name );
	pArea->area_flags = 0;
      }
      pArea->area_flags |= AREA_LOADING;
      break;
    }
    case TAG_Mobile: parse_mobile( field, pArea ); break;
    case TAG_Object: parse_object( field, pArea ); break;
    case TAG_Room: parse_room( field, pArea ); break;
    default: bug("Invalid Tag: %s", tagName ); break;
    }
  }

  REMOVE_BIT(pArea->area_flags, AREA_LOADING);
}
 
void new_load_area( FILE *fp ) {
  log_stringf( " New format" );
  parse_datas(fp);
}

void new_load_list_of_areas( FILE *fpList, const char *dirName ) {
  for ( ; ; ) {
    strcpy( strArea, fread_word( fpList ) );
    if ( strArea[0] == '$' )
      break;
    if ( strArea[0] == '-' )
      fpArea = stdin;
    else {
      log_stringf( " Reading %s", strArea );
      char fileName[MAX_STRING_LENGTH];
      sprintf( fileName, "%s%s", dirName, strArea );
      if ( ( fpArea = fopen( fileName, "r" ) ) == NULL ) {
	perror( strArea );
	exit( 1 );
      }
    }

    // Little trick to determine which kind of area file we'll read
    int c = fgetc( fpArea ); // get 1st char
    ungetc( c, fpArea ); // unget it
    if ( c == '#' ) // old style, starts with #AREADATA or #STYLE
      load_one_area( fpArea );
    else
      new_load_area( fpArea ); // new style

    if ( fpArea != stdin )
      fclose( fpArea );
    fpArea = NULL;
  }
}
