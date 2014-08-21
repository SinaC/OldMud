#ifndef __OLC_VALUE_H__
#define __OLC_VALUE_H__

// OLCEditable->editFlags
#define OLCEDIT_LOADING  (A)
#define OLCEDIT_MODIFIED (B)
#define OLCEDIT_NOTSAVED (C)
#define OLCEDIT_MARKED   (D)





// Affect flag, stored in affect_data
#define AFFECT_STAY_DEATH (A)
#define AFFECT_NON_DISPELLABLE (B)
// set if affect has been created by an ability, affect->type is revelant
#define AFFECT_ABILITY (C)
// set if duration is not revelant
#define AFFECT_PERMANENT (D)
// not from an ability same as if AFFECT_ABILITY not set
#define AFFECT_INHERENT (E)
// invisible affect such as Lycanthropy
#define AFFECT_INVISIBLE (F)
// Always true unless affect has been removed with affect_remove
#define AFFECT_IS_VALID (ee)









// Used by easy, normal and hard command in medit
int hitroll_easy( float level );
int mana_nb_easy( float level );
int mana_val_easy( float level );
int mana_add_easy( float level );
int hit_nb_easy( float level );
int hit_val_easy( float level );
int hit_add_easy( float level );
int dam_nb_easy( float level );
int dam_val_easy( float level );
int dam_add_easy( float level );
int armor_easy( float level );
int hitroll_norm( float level );
int mana_nb_norm( float level );
int mana_val_norm( float level );
int mana_add_norm( float level );
int hit_nb_norm( float level );
int hit_val_norm( float level );
int hit_add_norm( float level );
int dam_nb_norm( float level );
int dam_val_norm( float level );
int dam_add_norm( float level );
int armor_norm( float level );
int hitroll_hard( float level );
int mana_nb_hard( float level );
int mana_val_hard( float level );
int mana_add_hard( float level );
int hit_nb_hard( float level );
int hit_val_hard( float level );
int hit_add_hard( float level );
int dam_nb_hard( float level );
int dam_val_hard( float level );
int dam_add_hard( float level );
int armor_hard( float level );


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
//#define MOB_VNUM_FIDO		   3090
//#define MOB_VNUM_CITYGUARD	   3060
//#define MOB_VNUM_VAMPIRE	   3404

//#define MOB_VNUM_PATROLMAN	   2106
//#define GROUP_VNUM_TROLLS	   2100
//#define GROUP_VNUM_OGRES	   2101

//#define MOB_VNUM_PONY                 2
//#define MOB_VNUM_DONKEY               3
//#define MOB_VNUM_HORSE                4
//#define MOB_VNUM_CAMEL                5
//#define MOB_VNUM_ELEPHANT             6
//#define MOB_VNUM_GRIFFON              7

#define MOB_VNUM_DUMMY               99

#define MOB_VNUM_FAMILIAR             9

// Added by SinaC 2000 for Tartarus's spell
#define MOB_VNUM_LGOLEM              20
#define MOB_VNUM_GGOLEM              21

#define MOB_VNUM_FLESHGOLEM          24

#define MOB_VNUM_ZOMBIE               1
#define MOB_VNUM_SKELETON            22
#define MOB_VNUM_MUMMY               23
// Added by SinaC 2003
#define MOB_VNUM_MIRROR_IMAGE        (25)
#define MOB_VNUM_ANIM_OBJ            (26)
#define MOB_VNUM_SHADOW              (27)
#define MOB_VNUM_CONJURE_MOUNT       (28)
#define MOB_VNUM_ELEMENTAL           (30)


/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		(A)		/* Auto set for mobs	*/
#define ACT_SENTINEL	    	(B)		/* Stays in one room	*/
#define ACT_SCAVENGER	      	(C)		/* Picks up objects	*/
#define ACT_AWARE               (E)             // can't be backstab
#define ACT_AGGRESSIVE		(F)    		/* Attacks PC's		*/
#define ACT_STAY_AREA		(G)		/* Won't leave area	*/
#define ACT_WIMPY		(H)
#define ACT_PET			(I)		/* Auto set for pets	*/
#define ACT_TRAIN		(J)		/* Can train PC's	*/
#define ACT_PRACTICE		(K)		/* Can practice PC's	*/
// Added by SinaC 2001
#define ACT_FREE_WANDER         (L)  /* Can leave an area without being extract, SinaC 2001 */
// Added by SinaC 2003, ACT_MOUNTABLE tells if a mob can be mounted using mount/dismount command
// ACT_IS_MOUNTED tells if a mob is mounted
#define ACT_MOUNTABLE           (M)
#define ACT_IS_MOUNTED          (N)
#define ACT_UNDEAD		(O)	
#define ACT_NOSLEEP             (P)
#define ACT_CLERIC		(Q)
#define ACT_MAGE		(R)
#define ACT_THIEF		(S)
#define ACT_WARRIOR		(T)
#define ACT_NOALIGN		(U)
#define ACT_NOPURGE		(V)
#define ACT_OUTDOORS		(W)
#define ACT_INDOORS		(Y)
// Added by SinaC 2003, set if the mob has been created with an ability such as summon elemental, ...
#define ACT_CREATED             (Z)
#define ACT_IS_HEALER		(aa)
#define ACT_GAIN		(bb)
#define ACT_UPDATE_ALWAYS	(cc)
//#define ACT_IS_CHANGER		(dd)
// Added by SinaC 2003
// ACT_RESERVED is used to mark a mob from script, so that mob can't be a quest target
#define ACT_RESERVED		(dd)
#define ACT_IS_SAFE		(ee)

/* damage classes */
#define DAM_NONE                0
#define DAM_BASH                1
#define DAM_PIERCE              2
#define DAM_SLASH               3
#define DAM_FIRE                4
#define DAM_COLD                5
#define DAM_LIGHTNING           6
#define DAM_ACID                7
#define DAM_POISON              8
#define DAM_NEGATIVE            9
#define DAM_HOLY                10
#define DAM_ENERGY              11
#define DAM_MENTAL              12
#define DAM_DISEASE             13
#define DAM_DROWNING            14
#define DAM_LIGHT		15
#define DAM_OTHER               16
#define DAM_HARM		17
#define DAM_CHARM		18
#define DAM_SOUND		19
// Added by SinaC 2001
#define DAM_DAYLIGHT            20
// Added by SinaC 2003
#define DAM_EARTH               21
#define DAM_WEAKEN              22

/* OFF bits for mobiles */
#define OFF_AREA_ATTACK         (A)
#define OFF_BACKSTAB            (B)
#define OFF_BASH                (C)
#define OFF_BERSERK             (D)
#define OFF_DISARM              (E)
#define OFF_DODGE               (F)
#define OFF_FADE                (G)
#define OFF_FAST                (H)
#define OFF_KICK                (I)
#define OFF_KICK_DIRT           (J)
#define OFF_PARRY               (K)
#define OFF_RESCUE              (L)
#define OFF_TAIL                (M)
#define OFF_TRIP                (N)
#define OFF_CRUSH		(O)

#define ASSIST_ALL       	(P)
#define ASSIST_ALIGN	        (Q)
#define ASSIST_RACE    	     	(R)
#define ASSIST_PLAYERS      	(S)
#define ASSIST_GUARD        	(T)
#define ASSIST_VNUM		(U)
// Added by SinaC 2000 to add some fun :)))) counter-attack for mobiles
#define OFF_COUNTER             (V)
#define OFF_BITE                (W)

/* return values for check_imm */
#define IS_NORMAL		0
#define IS_IMMUNE		1
#define IS_RESISTANT		2
#define IS_VULNERABLE		3

// IRV IMMUNE RESISTANT VULNERABLE, SinaC 2003
#define IRV_SUMMON              (A)
#define IRV_CHARM               (B)
#define IRV_MAGIC               (C)
#define IRV_WEAPON              (D)
#define IRV_BASH                (E)
#define IRV_PIERCE              (F)
#define IRV_SLASH               (G)
#define IRV_FIRE                (H)
#define IRV_COLD                (I)
#define IRV_LIGHTNING           (J)
#define IRV_ACID                (K)
#define IRV_POISON              (L)
#define IRV_NEGATIVE            (M)
#define IRV_HOLY                (N)
#define IRV_ENERGY              (O)
#define IRV_MENTAL              (P)
#define IRV_DISEASE             (Q)
#define IRV_DROWNING            (R)
#define IRV_LIGHT		(S)
#define IRV_SOUND		(T)
#define IRV_PARALYSIS           (V)
#define IRV_WOOD                (X)
#define IRV_SILVER              (Y)
#define IRV_IRON                (Z)
// Added by SinaC 2001
#define IRV_DAYLIGHT            (aa)
// Added by SinaC 2003
#define IRV_EARTH               (bb)
// SinaC 2003
#define IRV_WEAKEN              (cc)

//// IMM bits for mobs
//#define IMM_SUMMON              (A)
//#define IMM_CHARM               (B)
//#define IMM_MAGIC               (C)
//#define IMM_WEAPON              (D)
//#define IMM_BASH                (E)
//#define IMM_PIERCE              (F)
//#define IMM_SLASH               (G)
//#define IMM_FIRE                (H)
//#define IMM_COLD                (I)
//#define IMM_LIGHTNING           (J)
//#define IMM_ACID                (K)
//#define IMM_POISON              (L)
//#define IMM_NEGATIVE            (M)
//#define IMM_HOLY                (N)
//#define IMM_ENERGY              (O)
//#define IMM_MENTAL              (P)
//#define IMM_DISEASE             (Q)
//#define IMM_DROWNING            (R)
//#define IMM_LIGHT		(S)
//#define IMM_SOUND		(T)
//#define IMM_WOOD                (X)
//#define IMM_SILVER              (Y)
//#define IMM_IRON                (Z)
//// Added by SinaC 2001
//#define IMM_DAYLIGHT            (aa)
//// Added by SinaC 2003
//#define IMM_EARTH               (bb)
//
// 
//// RES bits for mobs
//#define RES_SUMMON		(A)
//#define RES_CHARM		(B)
//#define RES_MAGIC               (C)
//#define RES_WEAPON              (D)
//#define RES_BASH                (E)
//#define RES_PIERCE              (F)
//#define RES_SLASH               (G)
//#define RES_FIRE                (H)
//#define RES_COLD                (I)
//#define RES_LIGHTNING           (J)
//#define RES_ACID                (K)
//#define RES_POISON              (L)
//#define RES_NEGATIVE            (M)
//#define RES_HOLY                (N)
//#define RES_ENERGY              (O)
//#define RES_MENTAL              (P)
//#define RES_DISEASE             (Q)
//#define RES_DROWNING            (R)
//#define RES_LIGHT		(S)
//#define RES_SOUND		(T)
//#define RES_WOOD                (X)
//#define RES_SILVER              (Y)
//#define RES_IRON                (Z)
//// Added by SinaC 2001
//#define RES_DAYLIGHT            (aa)
//// Added by SinaC 2003
//#define RES_EARTH               (bb)
//
//// VULN bits for mobs
//#define VULN_SUMMON		(A)
//#define VULN_CHARM		(B)
//#define VULN_MAGIC              (C)
//#define VULN_WEAPON             (D)
//#define VULN_BASH               (E)
//#define VULN_PIERCE             (F)
//#define VULN_SLASH              (G)
//#define VULN_FIRE               (H)
//#define VULN_COLD               (I)
//#define VULN_LIGHTNING          (J)
//#define VULN_ACID               (K)
//#define VULN_POISON             (L)
//#define VULN_NEGATIVE           (M)
//#define VULN_HOLY               (N)
//#define VULN_ENERGY             (O)
//#define VULN_MENTAL             (P)
//#define VULN_DISEASE            (Q)
//#define VULN_DROWNING           (R)
//#define VULN_LIGHT		(S)
//#define VULN_SOUND		(T)
//#define VULN_WOOD               (X)
//#define VULN_SILVER             (Y)
//#define VULN_IRON		(Z)
//// Added by SinaC 2001
//#define VULN_DAYLIGHT           (aa)
//// Added by SinaC 2003
//#define VULN_EARTH               (bb)

 
/* body form */
#define FORM_EDIBLE             (A)
#define FORM_POISON             (B)
#define FORM_MAGICAL            (C)
#define FORM_INSTANT_DECAY      (D)
#define FORM_OTHER              (E)  /* defined by material bit */
 
/* actual form */
#define FORM_ANIMAL             (G)
#define FORM_SENTIENT           (H)
#define FORM_UNDEAD             (I)
#define FORM_CONSTRUCT          (J)
#define FORM_MIST               (K)
#define FORM_INTANGIBLE         (L)
 
#define FORM_BIPED              (M)
#define FORM_CENTAUR            (N)
#define FORM_INSECT             (O)
#define FORM_SPIDER             (P)
#define FORM_CRUSTACEAN         (Q)
#define FORM_WORM               (R)
#define FORM_BLOB		(S)

#define FORM_MAMMAL             (V)
#define FORM_BIRD               (W)
#define FORM_REPTILE            (X)
#define FORM_SNAKE              (Y)
#define FORM_DRAGON             (Z)
#define FORM_AMPHIBIAN          (aa)
#define FORM_FISH               (bb)
#define FORM_COLD_BLOOD		(cc)
// Added by SinaC 2001
#define FORM_FUR                (dd)
// SinaC 2003
#define FORM_FOUR_ARMS          (ee)
 
/* body parts */
#define PART_HEAD               (A)
#define PART_ARMS               (B)
#define PART_LEGS               (C)
#define PART_HEART              (D)
#define PART_BRAINS             (E)
#define PART_GUTS               (F)
#define PART_HANDS              (G)
#define PART_FEET               (H)
#define PART_FINGERS            (I)
#define PART_EAR                (J)
#define PART_EYE		(K)
#define PART_LONG_TONGUE        (L)
#define PART_EYESTALKS          (M)
#define PART_TENTACLES          (N)
#define PART_FINS               (O)
#define PART_WINGS              (P)
#define PART_TAIL               (Q)
// PART_BODY: SinaC 2003  set for almost every races except blob and things we don't want to see with armor
#define PART_BODY               (R)
// S and T not used
/* for combat */
#define PART_CLAWS              (U)
#define PART_FANGS              (V)
#define PART_HORNS              (W)
#define PART_SCALES             (X)
#define PART_TUSKS		(Y)
// Z, aa, bb, cc, dd, ee not used

/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 */
#define AFF_BLIND		(A)
#define AFF_INVISIBLE		(B)
#define AFF_DETECT_EVIL		(C)
#define AFF_DETECT_INVIS	(D)
#define AFF_DETECT_MAGIC	(E)
#define AFF_DETECT_HIDDEN	(F)
#define AFF_DETECT_GOOD		(G)
#define AFF_SANCTUARY		(H)
#define AFF_FAERIE_FIRE		(I)
#define AFF_INFRARED		(J)
#define AFF_CURSE		(K)
// Modified by SinaC 2001
//#define AFF_UNUSED_FLAG		(L)	/* unused */
#define AFF_ROOTED              (L)
#define AFF_POISON		(M)
#define AFF_PROTECT_EVIL	(N)
#define AFF_PROTECT_GOOD	(O)
#define AFF_SNEAK		(P)
#define AFF_HIDE		(Q)
#define AFF_SLEEP		(R)
#define AFF_CHARM		(S)
#define AFF_FLYING		(T)
#define AFF_PASS_DOOR		(U)
#define AFF_HASTE		(V)
#define AFF_CALM		(W)
#define AFF_PLAGUE		(X)
#define AFF_WEAKEN		(Y)
#define AFF_DARK_VISION		(Z)
#define AFF_BERSERK		(aa)
#define AFF_SWIM		(bb)
#define AFF_REGENERATION        (cc)
#define AFF_SLOW		(dd)
// Added by SinaC 2000 for SILENCED people, can't cast spell
#define AFF_SILENCE             (ee)
// Added by SinaC 2001 for submarine area and race
/*
#define AFF_WALK_ON_WATER       (ff)
#define AFF_WATER_BREATH        (gg)
// Added by SinaC 2001 to allow the detection of hidden exits
#define AFF_DETECT_EXITS        (hh)
// Added by SinaC 2001 for the very funny Magic Mirror spell
#define AFF_MAGIC_MIRROR        (ii)
*/
// Added by SinaC 2001
#define AFF2_WALK_ON_WATER      (A)
#define AFF2_WATER_BREATH       (B)
#define AFF2_DETECT_EXITS       (C)
#define AFF2_MAGIC_MIRROR       (D)
#define AFF2_FAERIE_FOG         (E)
#define AFF2_NOEQUIPMENT        (F)
// Added by SinaC 2003
#define AFF2_FREE_MOVEMENT      (G)
#define AFF2_INCREASED_CASTING  (H)
#define AFF2_NOSPELL            (I)
#define AFF2_NECROTISM          (J)
#define AFF2_HIGHER_MAGIC_ATTRIBUTES (K)
#define AFF2_CONFUSION          (L)

// Added by SinaC 2001 for disease          Removed by SinaC 2003
//#define DIS_LYCANTHROPY         (A)
//#define DIS_PLAGUE              (B)


/*
 * Sex.
 * Used in #MOBILES.
 */
#define SEX_NEUTRAL		      0
#define SEX_MALE		      1
#define SEX_FEMALE		      2
#define SEX_RANDOM                    3

/* AC types */
#define AC_PIERCE			0
#define AC_BASH				1
#define AC_SLASH			2
#define AC_EXOTIC			3

/* dice */
#define DICE_NUMBER			0
#define DICE_TYPE			1
#define DICE_BONUS			2

/* size */
#define SIZE_TINY			0
#define SIZE_SMALL			1
#define SIZE_MEDIUM			2
#define SIZE_LARGE			3
#define SIZE_HUGE			4
#define SIZE_GIANT			5
// Added by SinaC 2003
#define SIZE_NOSIZE                     6



/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE	      1
#define OBJ_VNUM_GOLD_ONE	      2
#define OBJ_VNUM_GOLD_SOME	      3
#define OBJ_VNUM_SILVER_SOME	      4
#define OBJ_VNUM_COINS		      5

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_GUTS		     16
#define OBJ_VNUM_BRAINS		     17

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22
#define OBJ_VNUM_DISC		     23
#define OBJ_VNUM_PORTAL		     25
#define OBJ_VNUM_FIRE                37

// Added by SinaC 2000 for butcher skill
#define OBJ_VNUM_STEAK               60
// Added by SinaC 2000 for pillify
#define OBJ_VNUM_PILL                61
// Added by SinaC 2000 for blade_thirst
#define OBJ_VNUM_MITHRIL             62
// Added by SinaC 2000 for soul blade
#define OBJ_VNUM_SOULBLADE           63
// Added by SinaC 2000 for forage
#define OBJ_VNUM_BERRY               64
// Added by SinaC 2000 for animate_skeleton/decay_corpse spells from Tartarus
#define OBJ_VNUM_SKELETON            65

// Used by ice knife and flame blade, Sinac 1997
#define OBJ_VNUM_KNIFE               38
#define OBJ_VNUM_BLADE               39

// Used by holy/unholy blade, Added by SinaC 2001
#define OBJ_VNUM_HOLY_SWORD               42
#define OBJ_VNUM_UNHOLY_BLADE             43

// Added by SinaC 2001
#define OBJ_VNUM_BLOOD_PUDDLE             97
// Added by SinaC 2003
#define OBJ_VNUM_SHELTER           (24)
#define OBJ_VNUM_HOVERING          (71)
#define OBJ_VNUM_HOLY_SYMBOL       (72)
#define OBJ_VNUM_MYSTIC_BOAT       (73)
#define OBJ_VNUM_SPIRITUAL_HAMMER  (74)
#define OBJ_VNUM_MIND_BLADE        (75)
#define OBJ_VNUM_OAKEN_STAFF       (76)
#define OBJ_VNUM_EMPTY_POTION      (77)
#define OBJ_VNUM_ARROW             (78)
#define OBJ_VNUM_VINES             (81)

//#define OBJ_VNUM_ROSE		   1001
#define OBJ_VNUM_ROSE		   (7)

//#define OBJ_VNUM_PIT		   3010
#define OBJ_VNUM_PIT		   (6)



#define OBJ_VNUM_SCHOOL_MACE	   3700
#define OBJ_VNUM_SCHOOL_DAGGER	   3701
#define OBJ_VNUM_SCHOOL_SWORD	   3702
#define OBJ_VNUM_SCHOOL_SPEAR	   3717
#define OBJ_VNUM_SCHOOL_STAFF	   3718
#define OBJ_VNUM_SCHOOL_AXE	   3719
#define OBJ_VNUM_SCHOOL_FLAIL	   3720
#define OBJ_VNUM_SCHOOL_WHIP	   3721
#define OBJ_VNUM_SCHOOL_POLEARM    3722
#define OBJ_VNUM_SCHOOL_ARROW      3726
#define OBJ_VNUM_SCHOOL_RANGED     3727


#define OBJ_VNUM_SCHOOL_VEST	   3703
#define OBJ_VNUM_SCHOOL_SHIELD	   3704
#define OBJ_VNUM_SCHOOL_BANNER     3716
#define OBJ_VNUM_MAP		   3162

//#define OBJ_VNUM_WHISTLE	   2116

// from quest.C
// Object vnums for Quest Rewards
//#define QUEST_ITEM1                  26
//#define QUEST_ITEM2                  27
//#define QUEST_ITEM3                  28
//#define QUEST_ITEM4                  29
//#define QUEST_ITEM5                  31
//#define QUEST_ITEM6                  40
//#define QUEST_ITEM7                  44

/* Object vnums for object quest 'tokens'. In Moongate, the tokens are
   things like 'the Shield of Moongate', 'the Sceptre of Moongate'. These
   items are worthless and have the rot-death flag, as they are placed
   into the world when a player receives an object quest. */
//#define QUEST_OBJQUEST1              32 
//#define QUEST_OBJQUEST2              33
//#define QUEST_OBJQUEST3              34
//#define QUEST_OBJQUEST4              35
//#define QUEST_OBJQUEST5              36
//#define QUEST_OBJQUEST6              41

/*
 * Item types.
 * Used in #OBJECTS.
 */
#define ITEM_LIGHT		      1
#define ITEM_SCROLL		      2
#define ITEM_WAND		      3
#define ITEM_STAFF		      4
#define ITEM_WEAPON		      5
#define ITEM_TREASURE		      8
#define ITEM_ARMOR		      9
#define ITEM_POTION		     10
#define ITEM_CLOTHING		     11
#define ITEM_FURNITURE		     12
#define ITEM_TRASH		     13
#define ITEM_CONTAINER		     15
#define ITEM_DRINK_CON		     17
#define ITEM_KEY		     18
#define ITEM_FOOD		     19
#define ITEM_MONEY		     20
#define ITEM_BOAT		     22
#define ITEM_CORPSE_NPC		     23
#define ITEM_CORPSE_PC		     24
#define ITEM_FOUNTAIN		     25
#define ITEM_PILL		     26
// SinaC 2000 : have replaced ITEM_PROTECT with ITEM_THROWING
//#define ITEM_PROTECT		     27
//#define ITEM_THROWING                27  removed by SinaC 2003
#define ITEM_MAP		     28
#define ITEM_PORTAL		     29
#define ITEM_WARP_STONE		     30
// SinaC 2000 : have replaced ITEM_ROOM_KEY with ITEM_COMPONENT
//#define ITEM_ROOM_KEY		     31
#define ITEM_COMPONENT               31

#define ITEM_GEM		     32
#define ITEM_JEWELRY		     33
//#define ITEM_JUKEBOX		     34   removed by SinaC 2003
// Added by SinaC 2000 for bard and song
#define ITEM_INSTRUMENT              35
// Removed by SinaC 2003, can be emulate with script
// Added by SinaC 2000 for grenade, the grenade has to be rot_death to avoid prob
//  they can't be put in container if they are pulled
// has to be donated too
// can't give it when pulled
//#define ITEM_GRENADE                 36

// for window to see in another room, SinaC 2000
#define ITEM_WINDOW                  37
// for animate_skeleton spell from Tartarus's spell, SinaC 2000
//#define ITEM_SKELETON                38, removed by SinaC 2003, ITEM_CORPSE with value3 bit B set are skeletons
// Added by SinaC 2001 for levers
//#define ITEM_LEVER                   39        removed by SinaC 2003
// Added by SinaC 2003
#define ITEM_TEMPLATE                40
#define ITEM_SADDLE                  41
#define ITEM_ROPE                    42

#define CORPSE_SKELETON              (B)


// Removed by SinaC 2003, can be emulate with script
// constants used by lever code
//#define LEVER_PULLEDDOWN              0
//#define LEVER_PULLEDUP                1

// Removed by SinaC 2003, can be emulate with script
// constants used by grenade code
//#define GRENADE_NOTPULLED             0
//#define GRENADE_PULLED                1


/*
 * Extra flags.
 * Used in #OBJECTS.
 */
#define ITEM_GLOW		(A)
#define ITEM_HUM		(B)
#define ITEM_DARK		(C)
// Has replaced ITEM_LOCK with ITEM_STAY_DEATH, SinaC 2000
//#define ITEM_LOCK		(D)
#define ITEM_STAY_DEATH         (D)

#define ITEM_EVIL		(E)
#define ITEM_INVIS		(F)
#define ITEM_MAGIC		(G)
#define ITEM_NODROP		(H)
#define ITEM_BLESS		(I)
#define ITEM_ANTI_GOOD		(J)
#define ITEM_ANTI_EVIL		(K)
#define ITEM_ANTI_NEUTRAL	(L)
#define ITEM_NOREMOVE		(M)
#define ITEM_INVENTORY		(N)
#define ITEM_NOPURGE		(O)
#define ITEM_ROT_DEATH		(P)
#define ITEM_VIS_DEATH		(Q)
#define ITEM_DONATED            (R)
// Removed by SinaC 2001, use material table instead  see magic.C  'heat metal' spell
//#define ITEM_NONMETAL		(S)
// Added by SinaC 2001 for unique item
#define ITEM_UNIQUE             (S)

#define ITEM_NOLOCATE		(T)
#define ITEM_MELT_DROP		(U)
#define ITEM_HAD_TIMER		(V)
#define ITEM_SELL_EXTRACT	(W)
// Added by SinaC 2001, was in wear flags before
#define ITEM_NOSAC              (X)

#define ITEM_BURN_PROOF		(Y)
#define ITEM_NOUNCURSE          (Z)
// NoIdent item SinaC 2000
#define ITEM_NOIDENT            (aa)
// No condition item SinaC 2001
#define ITEM_NOCOND             (bb)

/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		(A)
#define ITEM_WEAR_FINGER	(B)
#define ITEM_WEAR_NECK		(C)
#define ITEM_WEAR_BODY		(D)
#define ITEM_WEAR_HEAD		(E)
#define ITEM_WEAR_LEGS		(F)
#define ITEM_WEAR_FEET		(G)
#define ITEM_WEAR_HANDS		(H)
#define ITEM_WEAR_ARMS		(I)
#define ITEM_WEAR_SHIELD	(J)
#define ITEM_WEAR_ABOUT		(K)
#define ITEM_WEAR_WAIST		(L)
#define ITEM_WEAR_WRIST		(M)
#define ITEM_WIELD		(N)
#define ITEM_HOLD		(O)
// Moved in extra flags, SinaC 2001
//#define ITEM_NO_SAC		(P)
#define ITEM_WEAR_FLOAT		(Q)

#define ITEM_WEAR_EAR           (R)
#define ITEM_WEAR_EYES          (S)
// Added by SinaC 2001 for brand mark
//#define ITEM_BRAND              (T)  removed by SinaC 2003


#define TYPE_THWACK             (37)

/* weapon class */
#define WEAPON_EXOTIC		(0)
#define WEAPON_SWORD		(1)
#define WEAPON_DAGGER		(2)
#define WEAPON_SPEAR		(3)
#define WEAPON_MACE		(4)
#define WEAPON_AXE		(5)
#define WEAPON_FLAIL		(6)
#define WEAPON_WHIP		(7)
#define WEAPON_POLEARM		(8)
// Added by SinaC 2003
#define WEAPON_STAFF		(9)
// Added by SinaC 2003 for ranged attack
#define WEAPON_ARROW            (10)
#define WEAPON_RANGED           (11)

/* weapon types */
#define WEAPON_FLAMING		(A)
#define WEAPON_FROST		(B)
#define WEAPON_VAMPIRIC		(C)
#define WEAPON_SHARP		(D)
#define WEAPON_VORPAL		(E)
#define WEAPON_TWO_HANDS	(F)
#define WEAPON_SHOCKING		(G)
#define WEAPON_POISON		(H)
// Added by SinaC 2001
#define WEAPON_HOLY             (I)
// Added by SinaC 2001 for weighted equivalent to sharp for mace
#define WEAPON_WEIGHTED         (J)
// Added by SinaC 2003
#define WEAPON_NECROTISM        (K)


/* gate flags */
#define GATE_NORMAL_EXIT	(A)
#define GATE_NOCURSE		(B)
#define GATE_GOWITH		(C)
#define GATE_BUGGY		(D)
#define GATE_RANDOM		(E)

/* furniture flags */
#define STAND_AT		(A)
#define STAND_ON		(B)
#define STAND_IN		(C)
#define SIT_AT			(D)
#define SIT_ON			(E)
#define SIT_IN			(F)
#define REST_AT			(G)
#define REST_ON			(H)
#define REST_IN			(I)
#define SLEEP_AT		(J)
#define SLEEP_ON		(K)
#define SLEEP_IN		(L)
#define PUT_AT			(M)
#define PUT_ON			(N)
#define PUT_IN			(O)
#define PUT_INSIDE		(P)




/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
#define OLD_APPLY_NONE                    0
#define OLD_APPLY_STR                     1
#define OLD_APPLY_DEX                     2
#define OLD_APPLY_INT                     3
#define OLD_APPLY_WIS                     4
#define OLD_APPLY_CON                     5
#define OLD_APPLY_SEX                     6
#define OLD_APPLY_CLASS                   7
#define OLD_APPLY_LEVEL                   8
#define OLD_APPLY_AGE                     9
#define OLD_APPLY_HEIGHT                 10
#define OLD_APPLY_WEIGHT                 11
#define OLD_APPLY_MANA                   12
#define OLD_APPLY_HIT                    13
#define OLD_APPLY_MOVE                   14
#define OLD_APPLY_GOLD                   15
#define OLD_APPLY_EXP                    16
#define OLD_APPLY_AC                     17
#define OLD_APPLY_HITROLL                18
#define OLD_APPLY_DAMROLL                19
#define OLD_APPLY_SAVES                  20
#define OLD_APPLY_SAVING_PARA            20
#define OLD_APPLY_SAVING_ROD             21
#define OLD_APPLY_SAVING_PETRI           22
#define OLD_APPLY_SAVING_BREATH          23
#define OLD_APPLY_SAVING_SPELL           24
#define OLD_APPLY_SPELL_AFFECT           25

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		      A
#define CONT_PICKPROOF		      B
#define CONT_CLOSED		      C
#define CONT_LOCKED		      D
#define CONT_PUT_ON		      E



/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   1200
//#define ROOM_VNUM_TEMPLE	   3001  depend on hometown, SinaC 2003
//#define ROOM_VNUM_ALTAR		   3054
//#define ROOM_VNUM_SCHOOL	   3700
//#define ROOM_VNUM_DONATION         3360 /* Added by Seytan for Donate */
//#define ROOM_VNUM_MORGUE	   3390 /* Added by Seytan for Morgue */
/* Added by Sinac for Battle  2000 */
#define PREP_START  3   /* vnum of first prep room */
#define PREP_END    99   /* vnum of last prep room */
#define ARENA_FROM  4
#define ARENA_TO    28
#define ARENA_START number_range( ARENA_FROM, ARENA_TO )    /* vnum of first real arena room*/
// Rebirth room where people rebirthing are transfered
#define ROOM_VNUM_REBIRTH            50
#define ROOM_VNUM_REMORT             51


/*
 * Room flags.
 * Used in #ROOMS.
 */
#define ROOM_DARK		(A)
#define ROOM_DEATH              (B)
#define ROOM_NO_MOB		(C)
#define ROOM_INDOORS		(D)
#define ROOM_SOUNDPROOF         (F)
#define ROOM_NOTRACK            (G)
#define ROOM_PRIVATE		(J)
#define ROOM_SAFE		(K)
#define ROOM_SOLITARY		(L)
#define ROOM_PET_SHOP		(M)
#define ROOM_NO_RECALL		(N)
#define ROOM_IMP_ONLY		(O)
#define ROOM_GODS_ONLY		(P)
#define ROOM_HEROES_ONLY	(Q)
#define ROOM_NEWBIES_ONLY	(R)
#define ROOM_LAW		(S)
#define ROOM_NOWHERE		(T)
#define ROOM_BANK		(U)

// SinaC 2000 for Arena
#define ROOM_ARENA              (V)
// other new flags SinaC 2000
#define ROOM_NOSPELL            (W)
#define ROOM_NOSCAN             (X)
// Added by SinaC 2000 for teleporting room (see teleport vnum in area struct)
//#define ROOM_TELEPORT           (Y) removed by SinaC 2003, scripts can do that
// Added by SinaC 2001
#define ROOM_NOPOWER            (Z)
// SinaC 2003
#define ROOM_DONATION           (aa)

/*
 * Directions.
 * Used in #ROOMS.
 */
// Modified by SinaC 2003
#define DIR_NORTH     (0)
#define DIR_EAST      (1)
#define DIR_SOUTH     (2)
#define DIR_WEST      (3)
#define DIR_UP        (4)
#define DIR_DOWN      (5)
#define DIR_NORTHEAST (6)
#define DIR_NORTHWEST (7)
#define DIR_SOUTHEAST (8)
#define DIR_SOUTHWEST (9)
#define DIR_SPECIAL (10)


/*
 * Exit flags.
 * Used in #ROOMS.
 */
#define EX_ISDOOR		      (A)
#define EX_CLOSED		      (B)
#define EX_LOCKED		      (C)
#define EX_PICKPROOF		      (F)
#define EX_NOPASS		      (G)
#define EX_EASY			      (H)
#define EX_HARD			      (I)
#define EX_INFURIATING		      (J)
#define EX_NOCLOSE		      (K)
#define EX_NOLOCK		      (L)
// Added by SinaC 2001
#define EX_HIDDEN                     (M)
// Added by SinaC 2003
#define EX_BASHED                     (N)
// Added by SinaC 2003, create dynamic exit which are not save whe using asave
#define EX_NOSAVING                   (O)
#define EX_CLIMB                      (P)
#define EX_EATKEY                     (R)
#define EX_BASHPROOF                  (S)
// SinaC 2003, when set the exit_info is not updated
#define EX_NORESET                    (T)

/*
 * Sector types.
 * Used in #ROOMS.
 */
#define SECT_INSIDE		      0
#define SECT_CITY		      1
#define SECT_FIELD		      2
#define SECT_FOREST		      3
#define SECT_HILLS		      4
#define SECT_MOUNTAIN		      5
#define SECT_WATER_SWIM		      6
#define SECT_WATER_NOSWIM	      7
// Has deleted SECT_UNUSED and has replaced it by SECT_BURNING
//#define SECT_UNUSED		      8
#define SECT_BURNING                  8

#define SECT_AIR		      9
#define SECT_DESERT		     10
#define SECT_UNDERWATER              11
#define SECT_MAX		     12



/*
 * Equipment wear locations.
 * Used in #RESETS.
 */
#define WEAR_NONE		     -1
#define WEAR_LIGHT		      0
#define WEAR_FINGER_L		      1
#define WEAR_FINGER_R		      2
#define WEAR_NECK_1		      3
#define WEAR_NECK_2		      4
#define WEAR_BODY		      5
#define WEAR_HEAD		      6
#define WEAR_LEGS		      7
#define WEAR_FEET		      8
#define WEAR_HANDS		      9
#define WEAR_ARMS		     10
#define WEAR_SHIELD		     11
#define WEAR_ABOUT		     12
#define WEAR_WAIST		     13
#define WEAR_WRIST_L		     14
#define WEAR_WRIST_R		     15
#define WEAR_WIELD		     16
#define WEAR_HOLD		     17
#define WEAR_EAR_L                   18
#define WEAR_EAR_R                   19
#define WEAR_EYES                    20
#define WEAR_SECONDARY		     21
#define WEAR_FLOAT                   22
// Added by SinaC 2001 for brand mark
//#define WEAR_BRAND                   23 removed by SinaC 2003
//#define MAX_WEAR		     23
#define WEAR_THIRDLY                 23
#define WEAR_FOURTHLY                24
//#define MAX_WEAR		     23
#define MAX_WEAR                     25



/*
 * Object defined in limbo.are
 * Used in save.c to load objects that don't exist.
 */
#define OBJ_VNUM_DUMMY	30

/*
 * Area flags.
 */
#define         AREA_NONE         (0)
#define         AREA_CHANGED      (A)	/* Area has been modified. */
#define         AREA_ADDED        (B)	/* Area has been added to. */
#define         AREA_LOADING      (C)	/* Used for counting in db.c */
// only the 2 last flags are saved in file
// Used for unlinked area or area we don't want people goes in
#define         AREA_NOTELEPORT   (D)
// Used for earthquake, set if we want earthquake in that area
#define         AREA_EARTHQUAKE   (E)
// Set this to true if you want to auto-save area state
//  check in area_update, modified in area_update and scrpred.C:gpf_saveAreaState
#define         AREA_SAVE_STATE   (F)



#define LIQ_WATER        0


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

#endif
