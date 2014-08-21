#ifndef _BOARD_H_
#define _BOARD_H_

/* Includes for board system */
/* This is version 2 of the board system, (c) 1995-96 erwin@pip.dknet.dk */


#define CON_NOTE_TO					20
#define CON_NOTE_SUBJECT				21
#define CON_NOTE_EXPIRE					22
#define CON_NOTE_TEXT					23
#define CON_NOTE_FINISH					24


#define DEF_NORMAL  0 /* No forced change, but default (any string)   */
#define DEF_INCLUDE 1 /* 'names' MUST be included (only ONE name!)    */
#define DEF_EXCLUDE 2 /* 'names' must NOT be included (one name only) */

#define MAX_BOARD 	  8

#define DEFAULT_BOARD 0 /* default board is board #0 in the boards      */
#define WELCOME_BOARD 1 /* Where the newcomers begin */
                        /* They should be readable by everyone!           */
                        
#define MAX_LINE_LENGTH 80 /* enforce a max length of 80 on text lines, reject longer lines */
						   /* This only applies in the Body of the note */                        
						   
#define MAX_NOTE_TEXT (4*MAX_STRING_LENGTH - 1000)
						
#define BOARD_NOTFOUND -1 /* Error code from board_lookup() and board_number */

/* Data about a board */
struct board_data
{
	char *short_name; /* Max 8 chars */
	char *long_name;  /* Explanatory text, should be no more than 40 ? chars */
	
	int read_level; /* minimum level to see board */
	int write_level;/* minimum level to post notes */

	char *names;       /* Default recipient */
	int force_type; /* Default action (DEF_XXX) */
	
	int purge_days; /* Default expiration */

	/* Non-constant data */
		
	NOTE_DATA *note_first; /* pointer to board's first note */
	bool changed; /* currently unused */
		
};

typedef struct board_data BOARD_DATA;


/* External variables */

extern BOARD_DATA boards[MAX_BOARD]; /* Declare */


/* Prototypes */

void finish_note (BOARD_DATA *board, NOTE_DATA *note); /* attach a note to a board */
void load_boards (void); /* load all boards */
int board_lookup (const char *name); /* Find a board with that name */
bool is_note_to (CHAR_DATA *ch, NOTE_DATA *note); /* is that note to ch? */
void personal_message (const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void make_note (const char* board_name, const char *sender, const char *to, const char *subject, const int expire_days, const char *text);
void save_boards ();

/* for nanny */
void handle_con_note_to 		(DESCRIPTOR_DATA *d, const char * argument);
void handle_con_note_subject 	(DESCRIPTOR_DATA *d, const char * argument);
void handle_con_note_expire 	(DESCRIPTOR_DATA *d, const char * argument);
void handle_con_note_text 		(DESCRIPTOR_DATA *d, const char * argument);
void handle_con_note_finish 	(DESCRIPTOR_DATA *d, const char * argument);


/* Commands */

DECLARE_DO_FUN (do_nwrite       );
DECLARE_DO_FUN (do_note		);
DECLARE_DO_FUN (do_board	);

#endif
