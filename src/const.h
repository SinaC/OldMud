#ifndef __CONST_H__
#define __CONST_H__

/* for command types */
#define ML 	MAX_LEVEL	/* implementor */
#define L1	MAX_LEVEL - 1  	/* creator */
#define L2	MAX_LEVEL - 2	/* supreme being */
#define L3	MAX_LEVEL - 3	/* deity */
#define L4 	MAX_LEVEL - 4	/* god */
#define L5	MAX_LEVEL - 5	/* immortal */
#define L6	MAX_LEVEL - 6	/* demigod */
#define L7	MAX_LEVEL - 7	/* angel */
#define L8	MAX_LEVEL - 8	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* avatar */
#define HE	LEVEL_HERO	/* hero */


#define ANY_CLASSES ((1<<MAX_CLASS)-1)


// SinaC 2003 for ranged weapon
// max v1 for ranged weapon
#define MAX_STRING_CONDITION (100)
// max v1 dynamic modifier for ranged weapon
#define MIN_STRING_DAMAGE    (1)
#define MAX_STRING_DAMAGE    (10)
// max v3 for ranged weapon
#define MAX_RANGED_STRENGTH  (10)
// max repairable string condition
#define MAX_REPAIRABLE_STRING_CONDITION (20)
// max ranged weapon shoot distance
#define MAX_RANGED_DISTANCE (3)

#define cRangedStringCondition value[1]
#define bRangedStringCondition baseval[1]
#define cRangedStringConditionModifier value[2]
#define bRangedStringConditionModifier baseval[2]
#define cRangedStrength value[3]
#define bRangedStrength baseval[3]
#define cRangedDistance value[4]
#define bRangedDistance baseval[4]


#define GET_AC(ch,type)		( (ch)->cstat(ac0+type) 			    \
		        + ( IS_AWAKE(ch)			    \
			? dex_app[ch->cstat(DEX)].defensive : 0 ) )

// Modified by SinaC 2001
/*#define GET_HITROLL(ch)	\
		((ch)->cstat(hitroll)+str_app[ch->cstat(STR)].tohit)*/
#define GET_HITROLL(ch)	\
		((ch)->cstat(hitroll)+dex_app[ch->cstat(DEX)].tohit)


#define GET_DAMROLL(ch) \
		((ch)->cstat(damroll)+str_app[ch->cstat(STR)].todam)

#endif

