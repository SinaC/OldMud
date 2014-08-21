#ifndef __ABILITY_H__
#define __ABILITY_H__

// default casting rule is everything to level 1
extern casting_rule_type default_casting_rule;
// max casting rule is everything to level 5
extern casting_rule_type max_casting_rule;
// SinaC 2003: wild mage has a special casting rule
extern casting_rule_type wild_magic_casting_rule;

////-1: spells at casting lvl 1 and skills at lvl 5  for warrior-like classes
//// 0: every spells at casting lvl 1                for cleric
//#define CASTING_RULE_SKILL_ALL_LVL5  (-1)
//#define CASTING_RULE_ALL_LVL1        (0)
//// 1: every spells at casting lvl 3                for druid
//// 2: every spells at casting lvl 4                for mage/elementalist
//// 3: one to level 5 and the others to level 3     for necromancer
//// 4: one to level 5 and the others to level 4     for necromancer+mage
//// 5: one to level 4 and the others to level 3     for wild mages
//// 99: every spells at casting lvl 5               for immortals :)
//#define CASTING_RULE_ALL_LVL3        (1)
//#define CASTING_RULE_ALL_LVL4        (2)
//#define CASTING_RULE_LVL5_OTHER_LVL3 (3)
//#define CASTING_RULE_LVL5_OTHER_LVL4 (4)
//#define CASTING_RULE_ALL_LVL5        (99)
//// Added by SinaC 2003 for wild mages
//#define CASTING_RULE_LVL4_OTHER_LVL3 (5)


// return code for can_gain function, also used by check_gain_prereq
// it's okay
#define ERR_OK                   (0)
// player doesn't fit skill/spell prereq
#define ERR_NOT_FIT_PREREQ       (1)
// player can't learn that skill/spell
#define ERR_CANT_LEARN           (2)
// that spell doesn't have levels
#define ERR_SPELL_NO_LEVEL       (3)
// player already has max level for that skill/spell
#define ERR_ALREADY_MASTER       (4)
// player has already the maximal number of non-level 1 skill/spell he could
#define ERR_ALREADY_MAX_LEVEL    (5)
// player must have at least 90% in a skill/spell before learning another level
#define ERR_NOT_ENOUGH_PERC      (6)
// player has that skill/spell as a race/clan or god one
#define ERR_GOD_CLAN_RACE        (7)
// player is too low level to gain that skill/spell
#define ERR_LEVEL_TOO_LOW        (8)
// Added by SinaC 2003 for studied spell
#define ERR_STUDIED_SPELL        (9)


//bool    is_valid_max_casting_rule  args( ( const int rule ) );


int    check_gain_prereq    args( ( CHAR_DATA *ch, int sn ) );


bool 	parse_gen_groups args( ( CHAR_DATA *ch,const char *argument ) );
void 	list_group_costs args( ( CHAR_DATA *ch ) );
void    list_group_known args( ( CHAR_DATA *ch ) );
int 	exp_per_level	args( ( CHAR_DATA *ch, int points ) );
void 	check_improve	args( ( CHAR_DATA *ch, int sn, bool success, 
				int multiplier ) );

int     find_ability    args( ( CHAR_DATA *ch, const char *name, const int type ) );



DECLARE_DO_FUN( do_gain );
DECLARE_DO_FUN( do_teach );

#endif
