#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "recycle.h"
#include "tables.h"
#include "olc_value.h"
#include "names.h"
#include "script2.h"


const char *item_name(int item_type) {
  int type;

  for (type = 0; item_table[type].name != NULL; type++)
    if (item_type == item_table[type].type)
      return item_table[type].name;
  return "none";
}

const char *weapon_name( int weapon_type) {
  int type;
 
  for (type = 0; weapon_table[type].name != NULL; type++)
    if (weapon_type == weapon_table[type].type)
      return weapon_table[type].name;
  return "exotic";
}

/*
 * Return ascii name of an affect bit vector.
 */
const char *affect_bit_name( long vector ) {
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';
  if ( vector & AFF_BLIND         ) strcat( buf, " blind"         );
  if ( vector & AFF_INVISIBLE     ) strcat( buf, " invisible"     );
  if ( vector & AFF_DETECT_EVIL   ) strcat( buf, " detect_evil"   );
  if ( vector & AFF_DETECT_GOOD   ) strcat( buf, " detect_good"   );
  if ( vector & AFF_DETECT_INVIS  ) strcat( buf, " detect_invis"  );
  if ( vector & AFF_DETECT_MAGIC  ) strcat( buf, " detect_magic"  );
  if ( vector & AFF_DETECT_HIDDEN ) strcat( buf, " detect_hidden" );
  if ( vector & AFF_SANCTUARY     ) strcat( buf, " sanctuary"     );
  if ( vector & AFF_FAERIE_FIRE   ) strcat( buf, " faerie_fire"   );
  if ( vector & AFF_INFRARED      ) strcat( buf, " infrared"      );
  if ( vector & AFF_CURSE         ) strcat( buf, " curse"         );
  if ( vector & AFF_POISON        ) strcat( buf, " poison"        );
  if ( vector & AFF_PROTECT_EVIL  ) strcat( buf, " prot_evil"     );
  if ( vector & AFF_PROTECT_GOOD  ) strcat( buf, " prot_good"     );
  if ( vector & AFF_SLEEP         ) strcat( buf, " sleep"         );
  if ( vector & AFF_SNEAK         ) strcat( buf, " sneak"         );
  if ( vector & AFF_HIDE          ) strcat( buf, " hide"          );
  if ( vector & AFF_CHARM         ) strcat( buf, " charm"         );
  if ( vector & AFF_FLYING        ) strcat( buf, " flying"        );
  if ( vector & AFF_PASS_DOOR     ) strcat( buf, " pass_door"     );
  if ( vector & AFF_BERSERK	  ) strcat( buf, " berserk"	  );
  if ( vector & AFF_CALM	  ) strcat( buf, " calm"	  );
  if ( vector & AFF_HASTE	  ) strcat( buf, " haste"	  );
  if ( vector & AFF_SLOW          ) strcat( buf, " slow"          );
  if ( vector & AFF_PLAGUE	  ) strcat( buf, " plague" 	  );
  if ( vector & AFF_DARK_VISION   ) strcat( buf, " dark_vision"   );
  if ( vector & AFF_WEAKEN        ) strcat( buf, " weaken"        );
  if ( vector & AFF_SWIM          ) strcat( buf, " swim"          );
  if ( vector & AFF_REGENERATION  ) strcat( buf, " regeneration"  );
  // Added by SinaC 2000 for silenced people
  if ( vector & AFF_SILENCE       ) strcat( buf, " silence"       );
  // Added by SinaC 2001 for rooted people
  if ( vector & AFF_ROOTED        ) strcat( buf, " rooted"        );
  /* Removed by SinaC 2001
  // Added by SinaC 2001
  *if ( vector & AFF_WATER_BREATH  ) strcat( buf, " water_breath"  );
  *if ( vector & AFF_WALK_ON_WATER ) strcat( buf, " walk_on_water" );
  *if ( vector & AFF_DETECT_EXITS  ) strcat( buf, " detect_exits"  );  
  *if ( vector & AFF_MAGIC_MIRROR  ) strcat( buf, " magic_mirror"  );
  */
  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

// Added by SinaC 2001
const char *affect2_bit_name( long vector ) {
  static char buf[MAX_STRING_LENGTH];

  buf[0] = '\0';
  if ( vector & AFF2_WATER_BREATH  ) strcat( buf, " water_breath"  );
  if ( vector & AFF2_WALK_ON_WATER ) strcat( buf, " walk_on_water" );
  if ( vector & AFF2_DETECT_EXITS  ) strcat( buf, " detect_exits"  );  
  if ( vector & AFF2_MAGIC_MIRROR  ) strcat( buf, " magic_mirror"  );
  if ( vector & AFF2_FAERIE_FOG    ) strcat( buf, " faerie_fog"    );
  if ( vector & AFF2_NOEQUIPMENT   ) strcat( buf, " no_equipment"  );
  // Added by SinaC 2003
  if ( vector & AFF2_FREE_MOVEMENT ) strcat( buf, " free_movement" );
  if ( vector & AFF2_INCREASED_CASTING ) strcat( buf, " increased_casting" );
  if ( vector & AFF2_NOSPELL ) strcat( buf, " no_spell" );
  if ( vector & AFF2_NECROTISM ) strcat( buf, " necrotism" );
  if ( vector & AFF2_HIGHER_MAGIC_ATTRIBUTES ) strcat( buf, " higher_attributes" );
  if ( vector & AFF2_CONFUSION     ) strcat( buf, " confusion" );
  return ( buf[0] != '\0' ) ? buf+1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
const char *extra_bit_name( int extra_flags ) {
  static char buf[512];

  buf[0] = '\0';
  if ( extra_flags & ITEM_GLOW           ) strcat( buf, " glow"         );
  if ( extra_flags & ITEM_HUM            ) strcat( buf, " hum"          );
  if ( extra_flags & ITEM_DARK           ) strcat( buf, " dark"         );
  // Have replaced ITEM_LOCK with ITEM_STAY_DEATH, SinaC 2000
  //    if ( extra_flags & ITEM_LOCK         ) strcat( buf, " lock"         );
  if ( extra_flags & ITEM_STAY_DEATH     ) strcat( buf, " stay_death"    );

  if ( extra_flags & ITEM_EVIL           ) strcat( buf, " evil"         );
  if ( extra_flags & ITEM_INVIS          ) strcat( buf, " invis"        );
  if ( extra_flags & ITEM_MAGIC          ) strcat( buf, " magic"        );
  if ( extra_flags & ITEM_NODROP         ) strcat( buf, " nodrop"       );
  if ( extra_flags & ITEM_BLESS          ) strcat( buf, " bless"        );
  if ( extra_flags & ITEM_ANTI_GOOD      ) strcat( buf, " anti-good"    );
  if ( extra_flags & ITEM_ANTI_EVIL      ) strcat( buf, " anti-evil"    );
  if ( extra_flags & ITEM_ANTI_NEUTRAL   ) strcat( buf, " anti-neutral" );
  if ( extra_flags & ITEM_NOREMOVE       ) strcat( buf, " noremove"     );
  if ( extra_flags & ITEM_INVENTORY      ) strcat( buf, " inventory"    );
  if ( extra_flags & ITEM_NOPURGE	 ) strcat( buf, " nopurge"	);
  if ( extra_flags & ITEM_VIS_DEATH	 ) strcat( buf, " vis_death"	);
  if ( extra_flags & ITEM_ROT_DEATH	 ) strcat( buf, " rot_death"	);
  if ( extra_flags & ITEM_NOLOCATE	 ) strcat( buf, " no_locate"	);
  if ( extra_flags & ITEM_DONATED        ) strcat( buf, " donated"      );
  if ( extra_flags & ITEM_SELL_EXTRACT   ) strcat( buf, " sell_extract" );
  if ( extra_flags & ITEM_BURN_PROOF	 ) strcat( buf, " burn_proof"	);
  if ( extra_flags & ITEM_NOUNCURSE	 ) strcat( buf, " no_uncurse"	);
  // Added by SinaC 2000  removed by SinaC 2001, use material table instead
  //if ( extra_flags & ITEM_NONMETAL	 ) strcat( buf, " non_metal"	);
// Added by SinaC 2001 for unique item
  if ( extra_flags & ITEM_UNIQUE	 ) strcat( buf, " unique"	);

  if ( extra_flags & ITEM_MELT_DROP	 ) strcat( buf, " melt_drop"	);
  if ( extra_flags & ITEM_HAD_TIMER	 ) strcat( buf, " had_timer"	);
  // NoIdent item
  if ( extra_flags & ITEM_NOIDENT        ) strcat( buf, " no_ident"     );
  // No condition item SinaC 2001
  if ( extra_flags & ITEM_NOCOND         ) strcat( buf, " no_cond"      );
  if ( extra_flags & ITEM_NOSAC	         ) strcat( buf, " nosac"        );

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

/* return ascii name of an act vector */
const char *act_bit_name( int act_flags ) {
  static char buf[512];

  buf[0] = '\0';

  if (IS_SET(act_flags,ACT_IS_NPC)) { 
    strcat(buf," npc");
    if (act_flags & ACT_SENTINEL 	) strcat(buf, " sentinel");
    if (act_flags & ACT_SCAVENGER	) strcat(buf, " scavenger");
    if (act_flags & ACT_AGGRESSIVE	) strcat(buf, " aggressive");
    if (act_flags & ACT_STAY_AREA	) strcat(buf, " stay_area");
    if (act_flags & ACT_WIMPY 	        ) strcat(buf, " wimpy");
    if (act_flags & ACT_PET		) strcat(buf, " pet");
    if (act_flags & ACT_TRAIN         	) strcat(buf, " train");
    if (act_flags & ACT_PRACTICE	) strcat(buf, " practice");
  /* Can leave an area without being extract, SinaC 2001 */
    if (act_flags & ACT_FREE_WANDER	) strcat(buf, " free_wander");
    if (act_flags & ACT_UNDEAD  	) strcat(buf, " undead");
    if (act_flags & ACT_CLERIC  	) strcat(buf, " cleric");
    if (act_flags & ACT_MAGE    	) strcat(buf, " mage");
    if (act_flags & ACT_THIEF   	) strcat(buf, " thief");
    if (act_flags & ACT_WARRIOR 	) strcat(buf, " warrior");
    if (act_flags & ACT_NOALIGN 	) strcat(buf, " no_align");
    if (act_flags & ACT_NOPURGE 	) strcat(buf, " no_purge");
    if (act_flags & ACT_IS_HEALER	) strcat(buf, " healer");
    if (act_flags & ACT_RESERVED        ) strcat(buf, " reserved");
    //if (act_flags & ACT_IS_CHANGER      ) strcat(buf, " changer");
    if (act_flags & ACT_GAIN	        ) strcat(buf, " gain");
    if (act_flags & ACT_UPDATE_ALWAYS   ) strcat(buf, " update_always");
    if (act_flags & ACT_IS_SAFE         ) strcat(buf, " is_safe");
    // Added by SinaC 2003
    if (act_flags & ACT_MOUNTABLE       ) strcat(buf, " mountable");
    if (act_flags & ACT_IS_MOUNTED      ) strcat(buf, " mounted");
    if (act_flags & ACT_CREATED         ) strcat(buf, " created");
  }
  else {
    strcat(buf," player");
    if (act_flags & PLR_AUTOASSIST	) strcat(buf, " autoassist");
    if (act_flags & PLR_AUTOEXIT	) strcat(buf, " autoexit");
    if (act_flags & PLR_AUTOLOOT	) strcat(buf, " autoloot");
    if (act_flags & PLR_AUTOSAC       	) strcat(buf, " autosac");
    if (act_flags & PLR_AUTOGOLD	) strcat(buf, " autogold");
    if (act_flags & PLR_AUTOSPLIT	) strcat(buf, " autosplit");
    if (act_flags & PLR_HOLYLIGHT	) strcat(buf, " holy_light");
    if (act_flags & PLR_CANLOOT 	) strcat(buf, " loot_corpse");
    if (act_flags & PLR_NOSUMMON	) strcat(buf, " no_summon");
    if (act_flags & PLR_NOFOLLOW	) strcat(buf, " no_follow");
    if (act_flags & PLR_FREEZE  	) strcat(buf, " frozen");
    if (act_flags & PLR_COLOUR  	) strcat(buf, " colour");
    if (act_flags & PLR_THIEF	        ) strcat(buf, " thief");
    if (act_flags & PLR_KILLER  	) strcat(buf, " killer");
    // Added by SinaC 2000, removed by SinaC 2003
    //if (act_flags & PLR_GAMBLER         ) strcat(buf, " gambling");
    if (act_flags & PLR_MORTAL          ) strcat(buf, " MORTAL");
    if (act_flags & PLR_AUTOHAGGLE      ) strcat(buf, " autohaggle");
  }
  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *comm_bit_name(int comm_flags) {
  static char buf[512];

  buf[0] = '\0';

  if (comm_flags & COMM_QUIET		) strcat(buf, " quiet");
  if (comm_flags & COMM_DEAF		) strcat(buf, " deaf");
  if (comm_flags & COMM_NOWIZ		) strcat(buf, " no_wiz");
  if (comm_flags & COMM_NOAUCTION	) strcat(buf, " no_auction");
  if (comm_flags & COMM_NOTRIVIA        ) strcat(buf, " no_trivia");
  // Gossip replaced with OOC by SinaC 2001
  //  if (comm_flags & COMM_NOGOSSIP	) strcat(buf, " no_gossip");
  if (comm_flags & COMM_NOOOC	        ) strcat(buf, " no_ooc");
  //IC channel added by SinaC 2001
  if (comm_flags & COMM_NOIC	        ) strcat(buf, " no_ic");
  if (comm_flags & COMM_NOQUESTION	) strcat(buf, " no_question");
  //  if (comm_flags & COMM_NOMUSIC	        ) strcat(buf, " no_music");  removed by SinaC 2003
  if (comm_flags & COMM_NOQUOTE	        ) strcat(buf, " no_quote");
  if (comm_flags & COMM_COMPACT      	) strcat(buf, " compact");
  if (comm_flags & COMM_BRIEF		) strcat(buf, " brief");
  if (comm_flags & COMM_PROMPT	        ) strcat(buf, " prompt");
  if (comm_flags & COMM_COMBINE	        ) strcat(buf, " combine");
  if (comm_flags & COMM_NOEMOTE	        ) strcat(buf, " no_emote");
  if (comm_flags & COMM_NOSHOUT       	) strcat(buf, " no_shout");
  if (comm_flags & COMM_NOTELL  	) strcat(buf, " no_tell");
  if (comm_flags & COMM_NOCHANNELS	) strcat(buf, " no_channels");
  // Added by SinaC 2000
  if (comm_flags & COMM_NOCLAN          ) strcat(buf, " no_clan");
  if (comm_flags & COMM_NOTRIVIA        ) strcat(buf, " no_trivia");
  if (comm_flags & COMM_TELNET_GA       ) strcat(buf, " telnet_ga");
  if (comm_flags & COMM_SHOW_AFFECTS    ) strcat(buf, " show_affects");
  if (comm_flags & COMM_NOGRATS         ) strcat(buf, " no_grats");
  if (comm_flags & COMM_SNOOP_PROOF     ) strcat(buf, " snoop_proof");
  if (comm_flags & COMM_AFK             ) strcat(buf, " afk");
  if (comm_flags & COMM_BUILDING        ) strcat(buf, " building");
  // Added by SinaC 2001
  if (comm_flags & COMM_TICK            ) strcat(buf, " tick");
  if (comm_flags & COMM_NOHINTS         ) strcat(buf, " nohints");
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (comm_flags & COMM_EDITING         ) strcat( buf, " editing");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *irv_bit_name(int irv_flags) {
  static char buf[512];

  buf[0] = '\0';

  if (irv_flags & IRV_SUMMON		) strcat(buf, " summon");
  if (irv_flags & IRV_CHARM		) strcat(buf, " charm");
  if (irv_flags & IRV_MAGIC		) strcat(buf, " magic");
  if (irv_flags & IRV_WEAPON		) strcat(buf, " weapon");
  if (irv_flags & IRV_BASH		) strcat(buf, " blunt");
  if (irv_flags & IRV_PIERCE		) strcat(buf, " piercing");
  if (irv_flags & IRV_SLASH		) strcat(buf, " slashing");
  if (irv_flags & IRV_FIRE		) strcat(buf, " fire");
  if (irv_flags & IRV_COLD		) strcat(buf, " cold");
  if (irv_flags & IRV_LIGHTNING	        ) strcat(buf, " lightning");
  if (irv_flags & IRV_ACID		) strcat(buf, " acid");
  if (irv_flags & IRV_POISON		) strcat(buf, " poison");
  if (irv_flags & IRV_NEGATIVE	        ) strcat(buf, " negative");
  if (irv_flags & IRV_HOLY		) strcat(buf, " holy");
  if (irv_flags & IRV_ENERGY		) strcat(buf, " energy");
  if (irv_flags & IRV_MENTAL		) strcat(buf, " mental");
  if (irv_flags & IRV_DISEASE	        ) strcat(buf, " disease");
  if (irv_flags & IRV_DROWNING	        ) strcat(buf, " drowning");
  if (irv_flags & IRV_LIGHT		) strcat(buf, " light");
  if (irv_flags & IRV_IRON		) strcat(buf, " iron");
  if (irv_flags & IRV_WOOD		) strcat(buf, " wood");
  if (irv_flags & IRV_SILVER	        ) strcat(buf, " silver");
  // Added by SinaC 2000, that was missing
  if (irv_flags & IRV_SOUND             ) strcat(buf, " sound");
  // Added by SinaC 2001
  if (irv_flags & IRV_DAYLIGHT          ) strcat(buf, " daylight");
  // Added by SinaC 2003
  if (irv_flags & IRV_EARTH             ) strcat(buf, " earth");
  // SinaC 2003
  if (irv_flags & IRV_PARALYSIS         ) strcat(buf, " paralysis");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *wear_bit_name(int wear_flags) {
  static char buf[512];

  buf [0] = '\0';
  if (wear_flags & ITEM_TAKE		) strcat(buf, " take");
  if (wear_flags & ITEM_WEAR_FINGER	) strcat(buf, " finger");
  if (wear_flags & ITEM_WEAR_NECK	) strcat(buf, " neck");
  if (wear_flags & ITEM_WEAR_BODY	) strcat(buf, " torso");
  if (wear_flags & ITEM_WEAR_HEAD	) strcat(buf, " head");
  if (wear_flags & ITEM_WEAR_LEGS	) strcat(buf, " legs");
  if (wear_flags & ITEM_WEAR_FEET	) strcat(buf, " feet");
  if (wear_flags & ITEM_WEAR_HANDS	) strcat(buf, " hands");
  if (wear_flags & ITEM_WEAR_ARMS	) strcat(buf, " arms");
  if (wear_flags & ITEM_WEAR_SHIELD	) strcat(buf, " shield");
  if (wear_flags & ITEM_WEAR_ABOUT	) strcat(buf, " body");
  if (wear_flags & ITEM_WEAR_WAIST	) strcat(buf, " waist");
  if (wear_flags & ITEM_WEAR_WRIST	) strcat(buf, " wrist");
  if (wear_flags & ITEM_WIELD		) strcat(buf, " wield");
  if (wear_flags & ITEM_HOLD		) strcat(buf, " hold");
  if (wear_flags & ITEM_WEAR_EAR        ) strcat(buf, " ear");
  if (wear_flags & ITEM_WEAR_EYES       ) strcat(buf, " eyes");
  // Moved in extra_flags,  SinaC 2001
  //if (wear_flags & ITEM_NO_SAC	        ) strcat(buf, " nosac");
  if (wear_flags & ITEM_WEAR_FLOAT	) strcat(buf, " float");
  // Added by SinaC 2001 for brand mark
  //if (wear_flags & ITEM_BRAND	) strcat(buf, " brand");  removed by SinaC 2003

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *form_bit_name(int form_flags) {
  static char buf[512];

  buf[0] = '\0';
  if (form_flags & FORM_POISON	        ) strcat(buf, " poison");
  else if (form_flags & FORM_EDIBLE	) strcat(buf, " edible");
  if (form_flags & FORM_MAGICAL	        ) strcat(buf, " magical");
  if (form_flags & FORM_INSTANT_DECAY	) strcat(buf, " instant_rot");
  if (form_flags & FORM_OTHER		) strcat(buf, " other");
  if (form_flags & FORM_ANIMAL         	) strcat(buf, " animal");
  if (form_flags & FORM_SENTIENT	) strcat(buf, " sentient");
  if (form_flags & FORM_UNDEAD	        ) strcat(buf, " undead");
  if (form_flags & FORM_CONSTRUCT	) strcat(buf, " construct");
  if (form_flags & FORM_MIST		) strcat(buf, " mist");
  if (form_flags & FORM_INTANGIBLE	) strcat(buf, " intangible");
  if (form_flags & FORM_BIPED		) strcat(buf, " biped");
  if (form_flags & FORM_CENTAUR	        ) strcat(buf, " centaur");
  if (form_flags & FORM_INSECT	        ) strcat(buf, " insect");
  if (form_flags & FORM_SPIDER	        ) strcat(buf, " spider");
  if (form_flags & FORM_CRUSTACEAN	) strcat(buf, " crustacean");
  if (form_flags & FORM_WORM		) strcat(buf, " worm");
  if (form_flags & FORM_BLOB		) strcat(buf, " blob");
  if (form_flags & FORM_MAMMAL	        ) strcat(buf, " mammal");
  if (form_flags & FORM_BIRD		) strcat(buf, " bird");
  if (form_flags & FORM_REPTILE	        ) strcat(buf, " reptile");
  if (form_flags & FORM_SNAKE		) strcat(buf, " snake");
  if (form_flags & FORM_DRAGON	        ) strcat(buf, " dragon");
  if (form_flags & FORM_AMPHIBIAN	) strcat(buf, " amphibian");
  if (form_flags & FORM_FISH		) strcat(buf, " fish");
  if (form_flags & FORM_COLD_BLOOD 	) strcat(buf, " cold_blooded");
  if (form_flags & FORM_FUR  		) strcat(buf, " fur");
  if (form_flags & FORM_FOUR_ARMS       ) strcat(buf, " four_arms");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *part_bit_name(int part_flags) {
  static char buf[512];

  buf[0] = '\0';
  if (part_flags & PART_HEAD		) strcat(buf, " head");
  if (part_flags & PART_ARMS		) strcat(buf, " arms");
  if (part_flags & PART_LEGS		) strcat(buf, " legs");
  if (part_flags & PART_HEART		) strcat(buf, " heart");
  if (part_flags & PART_BRAINS	        ) strcat(buf, " brains");
  if (part_flags & PART_GUTS		) strcat(buf, " guts");
  if (part_flags & PART_HANDS		) strcat(buf, " hands");
  if (part_flags & PART_FEET		) strcat(buf, " feet");
  if (part_flags & PART_FINGERS	        ) strcat(buf, " fingers");
  if (part_flags & PART_EAR		) strcat(buf, " ears");
  if (part_flags & PART_EYE		) strcat(buf, " eyes");
  if (part_flags & PART_LONG_TONGUE	) strcat(buf, " long_tongue");
  if (part_flags & PART_EYESTALKS	) strcat(buf, " eyestalks");
  if (part_flags & PART_TENTACLES	) strcat(buf, " tentacles");
  if (part_flags & PART_FINS		) strcat(buf, " fins");
  if (part_flags & PART_WINGS		) strcat(buf, " wings");
  if (part_flags & PART_TAIL		) strcat(buf, " tail");
  if (part_flags & PART_CLAWS		) strcat(buf, " claws");
  if (part_flags & PART_FANGS		) strcat(buf, " fangs");
  if (part_flags & PART_HORNS		) strcat(buf, " horns");
  if (part_flags & PART_SCALES	        ) strcat(buf, " scales");
  if (part_flags & PART_TUSKS	        ) strcat(buf, " tusks");
  if (part_flags & PART_BODY            ) strcat(buf, " body");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *weapon_bit_name(int weapon_flags) {
  static char buf[512];

  buf[0] = '\0';
  if (weapon_flags & WEAPON_FLAMING	) strcat(buf, " flaming");
  if (weapon_flags & WEAPON_FROST	) strcat(buf, " frost");
  if (weapon_flags & WEAPON_VAMPIRIC	) strcat(buf, " vampiric");
  if (weapon_flags & WEAPON_SHARP	) strcat(buf, " sharp");
  if (weapon_flags & WEAPON_VORPAL	) strcat(buf, " vorpal");
  if (weapon_flags & WEAPON_TWO_HANDS   ) strcat(buf, " two-handed");
  if (weapon_flags & WEAPON_SHOCKING 	) strcat(buf, " shocking");
  if (weapon_flags & WEAPON_POISON	) strcat(buf, " poison");
  // Added by SinaC 2001
  if (weapon_flags & WEAPON_HOLY	) strcat(buf, " holy");
  // Added by SinaC 2001 for weighted flag
  if (weapon_flags & WEAPON_WEIGHTED	) strcat(buf, " weighted");
  // Added by SinaC 2003 for necrotism
  if (weapon_flags & WEAPON_NECROTISM	) strcat(buf, " necrotism");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *cont_bit_name( int cont_flags) {
  static char buf[512];

  buf[0] = '\0';

  if (cont_flags & CONT_CLOSEABLE	) strcat(buf, " closable");
  if (cont_flags & CONT_PICKPROOF	) strcat(buf, " pickproof");
  if (cont_flags & CONT_CLOSED        	) strcat(buf, " closed");
  if (cont_flags & CONT_LOCKED	        ) strcat(buf, " locked");

  return (buf[0] != '\0' ) ? buf+1 : "none";
}


const char *off_bit_name(int off_flags) {
  static char buf[512];

  buf[0] = '\0';

  if (off_flags & OFF_AREA_ATTACK	) strcat(buf, " area attack");
  if (off_flags & OFF_BACKSTAB	        ) strcat(buf, " backstab");
  if (off_flags & OFF_BASH		) strcat(buf, " bash");
  if (off_flags & OFF_BERSERK		) strcat(buf, " berserk");
  if (off_flags & OFF_DISARM		) strcat(buf, " disarm");
  if (off_flags & OFF_DODGE		) strcat(buf, " dodge");
  if (off_flags & OFF_FADE		) strcat(buf, " fade");
  if (off_flags & OFF_FAST		) strcat(buf, " fast");
  if (off_flags & OFF_KICK		) strcat(buf, " kick");
  if (off_flags & OFF_KICK_DIRT	        ) strcat(buf, " kick_dirt");
  if (off_flags & OFF_PARRY		) strcat(buf, " parry");
  if (off_flags & OFF_RESCUE		) strcat(buf, " rescue");
  if (off_flags & OFF_TAIL		) strcat(buf, " tail");
  if (off_flags & OFF_TRIP		) strcat(buf, " trip");
  if (off_flags & OFF_CRUSH		) strcat(buf, " crush");
  if (off_flags & ASSIST_ALL		) strcat(buf, " assist_all");
  if (off_flags & ASSIST_ALIGN	        ) strcat(buf, " assist_align");
  if (off_flags & ASSIST_RACE		) strcat(buf, " assist_race");
  if (off_flags & ASSIST_PLAYERS	) strcat(buf, " assist_players");
  if (off_flags & ASSIST_GUARD	        ) strcat(buf, " assist_guard");
  if (off_flags & ASSIST_VNUM		) strcat(buf, " assist_vnum");
  // Added by SinaC 2000
  if (off_flags & OFF_COUNTER           ) strcat(buf, " counter");

  return ( buf[0] != '\0' ) ? buf+1 : "none";
}

const char *damtype_name( int dam_type ) {
  static char buf[512];

  buf[0] = '\0';
  switch ( dam_type ){
  case DAM_NONE:      strcpy( buf, "none" ); break;
  case DAM_BASH:      strcpy( buf, "bash" ); break;
  case DAM_PIERCE:    strcpy( buf, "pierce" ); break;
  case DAM_SLASH:     strcpy( buf, "slash" ); break;
  case DAM_FIRE:      strcpy( buf, "fire" ); break;
  case DAM_COLD:      strcpy( buf, "cold" ); break;
  case DAM_LIGHTNING: strcpy( buf, "lightning" ); break;
  case DAM_ACID:      strcpy( buf, "acid" ); break;
  case DAM_POISON:    strcpy( buf, "poison" ); break;
  case DAM_NEGATIVE:  strcpy( buf, "negative" ); break;
  case DAM_HOLY:      strcpy( buf, "holy" ); break;
  case DAM_ENERGY:    strcpy( buf, "energy" ); break;
  case DAM_MENTAL:    strcpy( buf, "mental" ); break;
  case DAM_DISEASE:   strcpy( buf, "disease" ); break;
  case DAM_DROWNING:  strcpy( buf, "drowning" ); break;
  case DAM_LIGHT:     strcpy( buf, "light" ); break;
  case DAM_HARM:      strcpy( buf, "harm" ); break;
  case DAM_CHARM:     strcpy( buf, "charm" ); break;
  case DAM_SOUND:     strcpy( buf, "sound" ); break;
    // Added by SinaC 2001
  case DAM_DAYLIGHT:  strcpy( buf, "daylight"); break;
    // Added by SinaC 2003
  case DAM_EARTH:     strcpy( buf, "earth"); break;
  case DAM_WEAKEN:    strcpy( buf, "weaken" ); break;
  default:            strcpy( buf, "UNKNOWN"); break;
  }
  return buf;
};

// Added by SinaC 2000 for etho
const char *etho_name( int etho ) {
  switch( etho ){
  case -1: return "Chaotic"; break;
  case  0: return "Neutral"; break;
  case  1: return "Lawful"; break;
  }
  return "None";
}

const char *align_name( int align ) {
  if ( align <= -350 )
    return "Evil";
  if ( align >= 350 )
    return "Good";
  if ( align > -350 && align < 350 )
    return "Neutral";
  return "None";
}

// Added by SinaC 2001
const char *etho_align_name( int etho, int align ) {
  static char buf[512];

  buf[0] = '\0';

  if ( etho == 0 && align > -350 && align < 350 )
    return "True Neutral";

  strcpy( buf, etho_name( etho ) );
  strcat( buf, " " );
  strcat( buf, align_name( align ) );

  return buf;
}

// Added by SinaC 2001
const char *god_name( int who ) {
  static char buf[512];
  
  if ( who >= MAX_GODS )
    return "none";

  strcpy( buf, gods_table[who].name );
  buf[0] = UPPER(buf[0]);

  return buf;
}

// Added by SinaC 2001, removed by SinaC 2003: must use flag_string/flag_value
//const char *classtype_name( int type ) {
//  switch ( type ) {
//  case CLASS_MAGIC: return "magic"; break;
//  case CLASS_MENTAL: return "mental"; break;
//  case CLASS_COMBAT: return "combat"; break;
//  default: return "unknown"; break;
//  }
//}

// Modified by SinaC 2003 for bard (songs)
const char *    abilitytype_name( int type ) {
  switch ( type ) {
  case TYPE_SKILL: return "Skill"; break;
  case TYPE_SPELL: return "Spell"; break;
  case TYPE_POWER: return "Power"; break;
  case TYPE_SONG:  return "Song"; break;
  case TYPE_AFFECT: return "SpecialAffect"; break;
  default: return "unknown"; break;
  }
}

// Added by SinaC 2003
const char *script_type_name( int type ) {
  switch ( type ) {
  case SVT_INT: return "int"; break;
  case SVT_STR: return "string"; break;
  case SVT_ENTITY: return "entity"; break;
  case SVT_FCT: return "function"; break;
  case SVT_VAR: return "var"; break;
  case SVT_LIST: return "list"; break;
  case SVT_VOID: return "void"; break;
  default: return "unknown"; break;
  }
}
