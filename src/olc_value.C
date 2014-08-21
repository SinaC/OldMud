// Added by SinaC 2001 to automatically set hitdice/damdice/ac/manadice/hit roll
// for mobiles
// XXX_dice: XXX_nb D XXX_val + XXX_add
#include <math.h>
// EASY
// hitroll
int hitroll_easy( float level ) {
  if ( level-5 < 0 )
    return (int)rint(0);
  return (int)rint(level-5);
}
// mana
int mana_nb_easy( float level ) {
  return (int)rint(level);
}
int mana_val_easy( float level ) {
  return (int)rint(9);
}
int mana_add_easy( float level ) {
  return (int)rint(85);
}
// hitdice
int hit_nb_easy( float level ) {
  if ( level < 10  ) return (int)rint(8/9 + level/9);
  if ( level < 25  ) return (int)rint(4/3 + level/15);
  if ( level < 50  ) return (int)rint(-19 + 22*level/25);
  if ( level < 75  ) return (int)rint(-25 + level);
  if ( level < 100 ) return (int)rint( 50);
  return (int)rint(50);
}
int hit_val_easy( float level ) {
  if ( level < 10  ) return (int)rint(44/9 + level/9);
  if ( level < 25  ) return (int)rint(4 + level/5);
  if ( level < 50  ) return (int)rint(8 + level/25);
  if ( level < 75  ) return (int)rint(10);
  if ( level < 100 ) return (int)rint( 10);
  return (int)rint(-310/9 + 4*level/9);
}
int hit_add_easy( float level ) {
  if ( level < 10  ) return (int)rint(40/9 + 50*level/9);
  if ( level < 25  ) return (int)rint(-122 + 91*level/5);
  if ( level < 50  ) return (int)rint(-2334 + 2667*level/25);
  if ( level < 75  ) return (int)rint(-12800 + 316*level);
  if ( level < 100 ) return (int)rint( 3400 + 100*level);
  return (int)rint(-99400/9 + 2200*level/9);
}
// damage
int dam_nb_easy( float level ) {
  if ( level < 10  ) return (int)rint(1);
  if ( level < 25  ) return (int)rint(1/3 + level/15);
  if ( level < 50  ) return (int)rint(-4 + 6*level/25);
  if ( level < 75  ) return (int)rint(4 + 2*level/25);
  if ( level < 100 ) return (int)rint( 19 - 3*level/25);
  return (int)rint(-37/9 + level/9);
}
int dam_val_easy( float level ) {
  if ( level < 10  ) return (int)rint(14/9 + 4*level/9);
  if ( level < 25  ) return (int)rint(14/3 + 2*level/15);
  if ( level < 50  ) return (int)rint(12 - 4*level/25);
  if ( level < 75  ) return (int)rint(2 + level/25);
  if ( level < 100 ) return (int)rint(-10 + level/5);
  return (int)rint(-10/9 + level/9);
}
int dam_add_easy( float level ) {
  if ( level < 10  ) return (int)rint(1);
  if ( level < 25  ) return (int)rint(-5/3 + 4*level/15);
  if ( level < 50  ) return (int)rint(-6 + 11*level/25);
  if ( level < 75  ) return (int)rint(-26 + 21*level/25);
  if ( level < 100 ) return (int)rint( -47 + 28*level/25);
  return (int)rint(-715/9 + 13*level/9);
}
// armor
int armor_easy( float level ) {
  if ( level < 10  ) return (int)rint(950/9 - 50*level/9);
  if ( level < 25  ) return (int)rint(340/3 - 19*level/3);
  if ( level <  50 ) return (int)rint(56 - 101*level/25);
  if ( level <  75 ) return (int)rint(361 - 254*level/25);
  if ( level < 100 ) return (int)rint(350 - 10*level);
  return (int)rint(7150/9 - 130*level/9);
}

// NORMAL
// hitroll
int hitroll_norm( float level ) {
  return (int)rint(level);
}
// mana
int mana_nb_norm( float level ) {
  return (int)rint(level);
}
int mana_val_norm( float level ) {
  return (int)rint(10);
}
int mana_add_norm( float level ) {
  return (int)rint(100);
}
// hitdice
int hit_nb_norm( float level ) {
  if ( level < 10  ) return (int)rint(8/9 + level/9);
  if ( level < 25  ) return (int)rint(level/5);
  if ( level < 50  ) return (int)rint(-40 + 9*level/5);
  if ( level < 75  ) return (int)rint(50);
  if ( level < 100 ) return (int)rint( 50);
  return (int)rint(50);
}
int hit_val_norm( float level ) {
  if ( level < 10  ) return (int)rint(1/3 + 2*level/3);
  if ( level < 25  ) return (int)rint(5 + level/5);
  if ( level < 50  ) return (int)rint(10);
  if ( level < 75  ) return (int)rint(10);
  if ( level < 100 ) return (int)rint( 10);
  return (int)rint(-90 + level);
}
int hit_add_norm( float level ) {
  if ( level < 10  ) return (int)rint(179/9 + 91*level/9);
  if ( level < 25  ) return (int)rint(-165 + 143*level/5);
  if ( level < 50  ) return (int)rint(-3400 + 158*level);
  if ( level < 75  ) return (int)rint(-9300 + 276*level);
  if ( level < 100 ) return (int)rint( 3900 + 100*level);
  return (int)rint(-494900/9 + 6200*level/9);
}
// damage
int dam_nb_norm( float level ) {
  if ( level < 10  ) return (int)rint(2);
  if ( level < 25  ) return (int)rint(2);
  if ( level < 50  ) return (int)rint(-1 + 3*level/25);
  if ( level < 75  ) return (int)rint(-3 + 4*level/25);
  if ( level < 100 ) return (int)rint( 12 - level/25);
  return (int)rint(-28/9 + level/9);
}
int dam_val_norm( float level ) {
  if ( level < 10  ) return (int)rint(26/9 + level/9);
  if ( level < 25  ) return (int)rint(2*level/5);
  if ( level < 50  ) return (int)rint(12 - 2*level/25);
  if ( level < 75  ) return (int)rint(12 - 2*level/25);
  if ( level < 100 ) return (int)rint(-6 + 4*level/25);
  return (int)rint(-10/9 + level/9);
}
int dam_add_norm( float level ) {
  if ( level < 10  ) return (int)rint(8/9 + level/9);
  if ( level < 25  ) return (int)rint(-4/3 + level/3);
  if ( level < 50  ) return (int)rint(-5 + 12*level/25);
  if ( level < 75  ) return (int)rint(-25 + 22*level/25);
  if ( level < 100 ) return (int)rint( -46 + 29*level/25);
  return (int)rint(-130 + 2*level);
}
int armor_norm( float level ) {
  if ( level < 10  ) return (int)rint(590/9 - 50*level/9);
  if ( level < 25  ) return (int)rint(190/3 - 16*level/3);
  if ( level <  50 ) return (int)rint(60 - 26*level/5);
  if ( level <  75 ) return (int)rint(300 - 10*level);
  if ( level < 100 ) return (int)rint(300 - 10*level);
  return (int)rint(3700/9 - 100*level/9);
}

// HARD
// hitroll
int hitroll_hard( float level ) {
  return (int)rint(level+5);
}
// mana
int mana_nb_hard( float level ) {
  return (int)rint(level);
}
int mana_val_hard( float level ) {
  return (int)rint(11);
}
int mana_add_hard( float level ) {
  return (int)rint(110);
}
// hitdice
int hit_nb_hard( float level ) {
  if ( level < 10  ) return (int)rint(17/9 + level/9);
  if ( level < 25  ) return (int)rint(1 + level/5);
  if ( level < 50  ) return (int)rint(-38 + 44*level/25);
  if ( level < 75  ) return (int)rint(50);
  if ( level < 100 ) return (int)rint( 50);
  return (int)rint(50);
}
int hit_val_hard( float level ) {
  if ( level < 10  ) return (int)rint(17/3 + level/3);
  if ( level < 25  ) return (int)rint(7 + level/5);
  if ( level < 50  ) return (int)rint(14 - 2*level/25);
  if ( level < 75  ) return (int)rint(10);
  if ( level < 100 ) return (int)rint( -5 + level/5);
  return (int)rint(-265/9 + 4*level/9);
}
int hit_add_hard( float level ) {
  if ( level < 10  ) return (int)rint(88 + 12*level);
  if ( level < 25  ) return (int)rint(-626/3 + 125*level/3);
  if ( level < 50  ) return (int)rint(-5334 + 6167*level/25);
  if ( level < 75  ) return (int)rint(-2800 + 196*level);
  if ( level < 100 ) return (int)rint(-1300 + 176*level);
  return (int)rint(-233300/9 + 3800*level/9);
}
// damage
int dam_nb_hard( float level ) {
  if ( level < 10  ) return (int)rint(2);
  if ( level < 25  ) return (int)rint(2/3 + 2*level/15);
  if ( level < 50  ) return (int)rint(-2 + 6*level/25);
  if ( level < 75  ) return (int)rint(18 - 4*level/25);
  if ( level < 100 ) return (int)rint(-3 + 3*level/25);
  return (int)rint(9);
}
int dam_val_hard( float level ) {
  if ( level < 10  ) return (int)rint(8/3 + level/3);
  if ( level < 25  ) return (int)rint(6);
  if ( level < 50  ) return (int)rint(8 - 2*level/25);
  if ( level < 75  ) return (int)rint(-8 + 6*level/25);
  if ( level < 100 ) return (int)rint(10);
  return (int)rint(-10/9 + level/9);
}
int dam_add_hard( float level ) {
  if ( level < 10  ) return (int)rint(7/9 + 2*level/9);
  if ( level < 25  ) return (int)rint(-1/3 + level/3);
  if ( level < 50  ) return (int)rint(-7 + 3*level/5);
  if ( level < 75  ) return (int)rint(-21 + 22*level/25);
  if ( level < 100 ) return (int)rint( -60 + 7*level/5);
  return (int)rint(-80/9 + 8*level/9);
}
int armor_hard( float level ) {
  if ( level < 10  ) return (int)rint(40/3 - 10*level/3);
  if ( level < 25  ) return (int)rint(30 - 5*level);
  if ( level <  50 ) return (int)rint(60 - 31*level/5);
  if ( level <  75 ) return (int)rint(250 - 10*level);
  if ( level < 100 ) return (int)rint(400 - 12*level);
  return (int)rint(800/9 - 80*level/9);
}
