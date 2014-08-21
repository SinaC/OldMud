#ifndef __COMM_H__
#define __COMM_H__

extern bool START_WITH_SAVED_TIME;

extern bool CAPTURE_SIGSEGV;
extern bool TESTING_CHARMIES;
extern bool DUMP_MOB_MSG;


extern bool fState; // set to true if you want to start mud with a world state
extern int port;
extern int control;
extern bool merc_down;
extern char str_boot_time[];
extern bool wizlock;
extern bool newlock;
extern bool abilitynotsaved;
extern bool clannotsaved;
extern bool cmdnotsaved;
extern bool racenotsaved;
extern bool pcclassnotsaved;

// List of forbidden player name, loaded at the end of boot_db
extern int MAX_FORBIDDEN_NAME;
extern const char **forbidden_name;


void	show_string	args( ( struct descriptor_data *d, char *input) );
void	close_socket	args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer	args( ( DESCRIPTOR_DATA *d, const char *txt,
				int length ) );
void 	write_to_char	args( ( DESCRIPTOR_DATA *desc, const char *txt ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
// Added by SinaC 2001 for color greet
void    send_to_desc    args( (DESCRIPTOR_DATA * desc, const char *txt ) );

void	send_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_charf  args( ( CHAR_DATA *ch, const char *fmt, ...) ) __attribute__ ((format (printf, 2, 3)));
void	page_to_char	args( ( const char *txt, CHAR_DATA *ch ) );
void	act		args( ( const char *format, CHAR_DATA *ch,
				const void *arg1, const void *arg2, int type ) );
void	act_new		args( ( const char *format, CHAR_DATA *ch, 
				const void *arg1, const void *arg2, int type,
				int min_pos) );
/*
 * Colour stuff by Lope of Loping Through The MUD
 */
int	colour		args( ( char type, CHAR_DATA *ch, char *string ) );
void	colourconv	args( ( char *buffer, const char *txt, CHAR_DATA *ch ) );
void	send_to_char_bw	args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char_bw	args( ( const char *txt, CHAR_DATA *ch ) );
void    info            args( ( const char *argument ) );

#endif
