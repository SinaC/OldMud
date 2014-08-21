#ifndef __ACT_COMM_H__
#define __ACT_COMM_H__

void  	check_sex	args( ( CHAR_DATA *ch) );
void	add_follower	args( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args( ( CHAR_DATA *ch ) );
void 	nuke_pets	args( ( CHAR_DATA *ch ) );
void	die_follower	args( ( CHAR_DATA *ch ) );
bool	is_same_group	args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
const char  * makedrunk       args( (const char *string, CHAR_DATA * ch ) );

DECLARE_DO_FUN( do_quit );
DECLARE_DO_FUN( do_split );
DECLARE_DO_FUN( do_yell );
DECLARE_DO_FUN( do_say );
DECLARE_DO_FUN( do_gtell );
DECLARE_DO_FUN( do_follow );
DECLARE_DO_FUN( do_afk );
DECLARE_DO_FUN( do_answer );

DECLARE_DO_FUN( do_delet );
DECLARE_DO_FUN( do_delete );
DECLARE_DO_FUN( do_channels );
DECLARE_DO_FUN( do_deaf );
DECLARE_DO_FUN( do_quiet );
DECLARE_DO_FUN( do_bet );
DECLARE_DO_FUN( do_pray );
DECLARE_DO_FUN( do_replay );
DECLARE_DO_FUN( do_auction );
DECLARE_DO_FUN( do_trivia );
DECLARE_DO_FUN( do_ooc );
DECLARE_DO_FUN( do_ic );
DECLARE_DO_FUN( do_grats );
DECLARE_DO_FUN( do_quote );
DECLARE_DO_FUN( do_question );
DECLARE_DO_FUN( do_clantalk );
DECLARE_DO_FUN( do_immtalk );
DECLARE_DO_FUN( do_osay );
DECLARE_DO_FUN( do_shout );
DECLARE_DO_FUN( do_otell );
DECLARE_DO_FUN( do_tell );
DECLARE_DO_FUN( do_reply );
DECLARE_DO_FUN( do_emote );
DECLARE_DO_FUN( do_pmote );
DECLARE_DO_FUN( do_pose );
DECLARE_DO_FUN( do_bug );
DECLARE_DO_FUN( do_typo );
DECLARE_DO_FUN( do_rent );
DECLARE_DO_FUN( do_qui );
DECLARE_DO_FUN( do_save );
DECLARE_DO_FUN( do_order );
DECLARE_DO_FUN( do_group );
DECLARE_DO_FUN( do_whisper );
DECLARE_DO_FUN( do_dirsay );
DECLARE_DO_FUN( do_qdirsay );

#endif
