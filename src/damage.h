#ifndef __DAMAGE_H__
#define __DAMAGE_H__

extern const int max_damage;

// modified by SinaC 2000
int	ability_damage	args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
			        int dt, int dam_type, bool show, bool old_dam ) );
// Inner damage, ranged_damage methods
void killing_payoff( CHAR_DATA *ch, CHAR_DATA *victim );
void wimp_out( CHAR_DATA *ch, CHAR_DATA *victim, const int dam );
void position_msg( CHAR_DATA *victim, const int dam );
bool link_dead( CHAR_DATA *victim );


#endif
