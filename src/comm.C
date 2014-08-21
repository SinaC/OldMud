/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1996 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@efn.org)				   *
*	    Gabrielle Taylor						   *
*	    Brian Moore (zump@rom.org)					   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, signals, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */



#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h> /* For send_to_charf, by Oxtal */
#include "merc.h"
#include "recycle.h"
//#define _XOPEN_SOURCE
//#include <unistd.h>

#include "classes.h"
#include "clan.h"
#include "act_info.h"
#include "handler.h"
#include "db.h"
#include "act_wiz.h"
#include "fight.h"
#include "act_comm.h"
#include "comm.h"
#include "save.h"
#include "update.h"
#include "alias.h"
#include "ban.h"
#include "string.h"
#include "olc.h"
#include "lookup.h"
#include "gsn.h"
#include "olc_value.h"
#include "names.h"
#include "wiznet.h"
#include "copyover.h"
#include "interp.h"
#include "ability.h"
#include "group.h"
#include "config.h"
#include "dbdata.h"
#include "act_move.h"
#include "utils.h"
#include "html.h"
#include "arena.h"


int MAX_FORBIDDEN_NAME;
const char **forbidden_name;

// set this to TRUE if you want to start the mud with time info stored in data/time.ini
bool START_WITH_SAVED_TIME = FALSE;

bool TESTING_CHARMIES = FALSE; // set this to TRUE if you want to redirect to immortal's master 
// what charmies and pet receives (act/send_to_char)
bool CAPTURE_SIGSEGV = FALSE; // set this to TRUE if you want to do a copyover when a SIGSEGV occurs
bool DUMP_MOB_MSG = FALSE; // set this to TRUE if you want to log every act/send_to_char mob receive


// Used by main, boot_db(passed in param), nanny
// If true: we are rebooting with a world state saved
// Can also be assigned in config.C if case we want to start the mud with a world state file
bool fState = FALSE; // SinaC 2003, copyover with world state saved


// for extern checkpointing, SinaC 2003 ... detect infinite loops
int tics = 0;


// Added by SinaC 2001 for rebirth
void create_rebirth( DESCRIPTOR_DATA *d );
void create_remort( DESCRIPTOR_DATA *d );

/*
 * Malloc debugging stuff.
 */
#if defined(sun)
#undef MALLOC_DEBUG
#endif

#if defined(MALLOC_DEBUG)
#include <malloc.h>
extern	int	malloc_debug	args( ( int  ) );
extern	int	malloc_verify	args( ( void ) );
#endif



/*
 * Signal handling.
 * Apollo has a problem with __attribute(atomic) in signal.h,
 *   I dance around it.
 */
#if defined(apollo)
#define __attribute(x)
#endif

#if defined(unix)
#include <signal.h>
#endif

#if defined(apollo)
#undef __attribute
#endif



/*
 * Socket and TCP/IP stuff.
 */
#if	defined(macintosh) || defined(MSDOS)
const	char	echo_off_str	[] = { '\0' };
const	char	echo_on_str	[] = { '\0' };
const	char 	go_ahead_str	[] = { '\0' };
#endif

#if	defined(unix)
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "telnet.h"
const	char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const	char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const	char 	go_ahead_str	[] = { IAC, GA, '\0' };
#endif

extern "C" {

  /*
   * OS-dependent declarations.
   */
#if	defined(_AIX)
#include <sys/select.h>
  int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
  int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
  void	bzero		args( ( char *b, int length ) );
  int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
  int	listen		args( ( int s, int backlog ) );
  int	setsockopt	args( ( int s, int level, int optname, void *optval,
				int optlen ) );
  int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(apollo)
#include <unistd.h>
  void	bzero		args( ( char *b, int length ) );
#endif

#if	defined(__hpux)
  int	accept		args( ( int s, void *addr, int *addrlen ) );
  int	bind		args( ( int s, const void *addr, int addrlen ) );
  void	bzero		args( ( char *b, int length ) );
  int	getpeername	args( ( int s, void *addr, int *addrlen ) );
  int	getsockname	args( ( int s, void *name, int *addrlen ) );
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
  int	listen		args( ( int s, int backlog ) );
  int	setsockopt	args( ( int s, int level, int optname,
 				const void *optval, int optlen ) );
  int	socket		args( ( int domain, int type, int protocol ) );
#endif

#if	defined(interactive)
#include <net/errno.h>
#include <sys/fnctl.h>
#endif

// Added by SinaC 2002
#if defined(__CYGWIN32__)
  int	close		args( ( int fd ) );
//  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
  int	read		args( ( int fd, char *buf, int nbyte ) );
//  int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
//				fd_set *exceptfds, struct timeval *timeout ) );
//  int	socket		args( ( int domain, int type, int protocol ) );
  int	write		args( ( int fd, char *buf, int nbyte ) );

#endif

// || defined(unix)   added by SinaC 2003
#if	defined(linux) || defined(unix)
  /*
    Linux shouldn't need these. If you have a problem compiling, try
    uncommenting accept and bind.
    int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
    int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
  */

  int	close		args( ( int fd ) );
  /*
    int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
    int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
  */
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
  //int	listen		args( ( int s, int backlog ) );
  int	read		args( ( int fd, char *buf, int nbyte ) );
  int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
  int	socket		args( ( int domain, int type, int protocol ) );
  int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(macintosh)
#include <console.h>
#include <fcntl.h>
#include <unix.h>
  struct	timeval
  {
    time_t	tv_sec;
    time_t	tv_usec;
  };
#if	!defined(isascii)
#define	isascii(c)		( (c) < 0200 )
#endif
  static	long			theKeys	[4];

  int	gettimeofday		args( ( struct timeval *tp, void *tzp ) );
#endif

#if	defined(MIPS_OS)
  extern	int		errno;
#endif

#if	defined(MSDOS)
  int	gettimeofday	args( ( struct timeval *tp, void *tzp ) );
  int	kbhit		args( ( void ) );
#endif

#if	defined(NeXT)
  int	close		args( ( int fd ) );
  int	fcntl		args( ( int fd, int cmd, int arg ) );
#if	!defined(htons)
  u_short	htons		args( ( u_short hostshort ) );
#endif
#if	!defined(ntohl)
  u_long	ntohl		args( ( u_long hostlong ) );
#endif
  int	read		args( ( int fd, char *buf, int nbyte ) );
  int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
  int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if	defined(sequent)
  int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
  int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
  int	close		args( ( int fd ) );
  int	fcntl		args( ( int fd, int cmd, int arg ) );
  int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
#if	!defined(htons)
  u_short	htons		args( ( u_short hostshort ) );
#endif
  int	listen		args( ( int s, int backlog ) );
#if	!defined(ntohl)
  u_long	ntohl		args( ( u_long hostlong ) );
#endif
  int	read		args( ( int fd, char *buf, int nbyte ) );
  int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
  int	setsockopt	args( ( int s, int level, int optname, caddr_t optval,
				int optlen ) );
  int	socket		args( ( int domain, int type, int protocol ) );
  int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

  /* This includes Solaris Sys V as well */
#if defined(sun)
  int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
  int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
  void	bzero		args( ( char *b, int length ) );
  int	close		args( ( int fd ) );
  int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
  int	listen		args( ( int s, int backlog ) );
  int	read		args( ( int fd, char *buf, int nbyte ) );
  int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
#if defined(SYSV)
  int setsockopt		args( ( int s, int level, int optname,
					const char *optval, int optlen ) );
#else
  int	setsockopt	args( ( int s, int level, int optname, void *optval,
				int optlen ) );
#endif
  int	socket		args( ( int domain, int type, int protocol ) );
  int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

#if defined(ultrix)
  int	accept		args( ( int s, struct sockaddr *addr, int *addrlen ) );
  int	bind		args( ( int s, struct sockaddr *name, int namelen ) );
  void	bzero		args( ( char *b, int length ) );
  int	close		args( ( int fd ) );
  int	getpeername	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	getsockname	args( ( int s, struct sockaddr *name, int *namelen ) );
  int	gettimeofday	args( ( struct timeval *tp, struct timezone *tzp ) );
  int	listen		args( ( int s, int backlog ) );
  int	read		args( ( int fd, char *buf, int nbyte ) );
  int	select		args( ( int width, fd_set *readfds, fd_set *writefds,
				fd_set *exceptfds, struct timeval *timeout ) );
  int	setsockopt	args( ( int s, int level, int optname, void *optval,
				int optlen ) );
  int	socket		args( ( int domain, int type, int protocol ) );
  int	write		args( ( int fd, char *buf, int nbyte ) );
#endif

} /* extern */


/*
 * Global variables.
 */
DESCRIPTOR_DATA *   descriptor_list;	/* All open descriptors		*/
DESCRIPTOR_DATA *   d_next;		/* Next descriptor in loop	*/
FILE *		    fpReserve;		/* Reserved file handle		*/
bool		    god;		/* All new chars are gods!	*/
bool		    merc_down;		/* Shutdown			*/
bool		    wizlock;		/* Game is wizlocked		*/
bool		    newlock;		/* Game is newlocked		*/
char		    str_boot_time[MAX_INPUT_LENGTH];
time_t		    current_time;	/* time of this pulse */


/*
 * OS-dependent local functions.
 */
#if defined(macintosh) || defined(MSDOS)
void	game_loop_mac_msdos	args( ( void ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif

#if defined(unix)
void	game_loop_unix		args( ( int control ) );
int	init_socket		args( ( int port ) );
void	init_descriptor		args( ( int control ) );
bool	read_from_descriptor	args( ( DESCRIPTOR_DATA *d ) );
bool	write_to_descriptor	args( ( int desc, char *txt, int length ) );
#endif




/*
 * Other local functions (OS-independent).
 */
bool	check_parse_name	args( ( const char *name ) );
bool	check_reconnect		args( ( DESCRIPTOR_DATA *d, const char *name,
					bool fConn ) );
bool	check_playing		args( ( DESCRIPTOR_DATA *d, const char *name ) );
int	main			args( ( int argc, char **argv ) );
void	nanny			args( ( DESCRIPTOR_DATA *d, const char *argument ) );
bool	process_output		args( ( DESCRIPTOR_DATA *d, bool fPrompt ) );
void	read_from_buffer	args( ( DESCRIPTOR_DATA *d ) );
void	stop_idling		args( ( CHAR_DATA *ch ) );
void    bust_a_prompt           args( ( CHAR_DATA *ch ) );

// Added by SinaC 2003 for SIGSEGV capture, thanks to Treize
void    init_signals               args( ( void ) );
void    sig_handler                args( ( int sig ) );


void parse_command_line( int argc, char **argv ); // SinaC 2003


bool fCopyOver = FALSE;
int port; /* For Copyover -- Oxtal*/

#if defined(unix)
int control;
#endif

int main( int argc, char **argv ) {
  struct timeval now_time;    
  //bool fCopyOver = FALSE;

  /*
   * Init some data space stuff.
   */
  //GC_VERBOSE = 7;
  str_space_init();
  //GC_expand_hp(20*1024*1024);
  

  /*
   * Memory debugging if needed.
   */
#if defined(MALLOC_DEBUG)
  malloc_debug( 2 );
#endif

  /*
   * Init time.
   */
  gettimeofday( &now_time, NULL );
  current_time 	= (time_t) now_time.tv_sec;
  strcpy( str_boot_time, ctime( &current_time ) );


  // SinaC 2003, first thing: assign default values
  assign_default_values();
  //assign_tags();
  check_tags();

  /*
   * Macintosh console initialization.
   */
#if defined(macintosh)
  console_options.nrows = 31;
  cshow( stdout );
  csetmode( C_RAW, stdin );
  cecho2file( "log file", 1, stderr );
#endif

  /*
   * Reserve one channel for our use.
   */
  if ( ( fpReserve = fopen( NULL_FILE, "r" ) ) == NULL ) {
    perror( NULL_FILE );
    exit( 1 );
  }

  /*
   * Get exe file -- Oxtal
   */
  EXE_FILE = argv[0];

  parse_command_line( argc, argv );

  // Replaced with parse_command_line, SinaC 2003
//  // 6 cases possible
//  // exe <port>
//  // exe <config>
//  //  2 following are called from the mud when a copyover occured
//  // exe <port> copyover <control>
//  // exe <config> <port> copyover <control>
//  //  2 following are called from the mud when a copyover with world state saved occured
//  // exe <port> state <control>
//  // exe <config> <port> state <control>
//
//  // Modified by SinaC 2003, argument may be a number (port) or a configFile:configName
//  //  // Get the port number.
//  //  port = 4000;
//  //  if ( argc > 1 ) {
//  //    if ( !is_number( argv[1] ) ) {
//  //      fprintf( stderr, "Usage: %s [port #]\n", argv[0] );
//  //      exit( 1 );
//  //    }
//  //    else if ( ( port = atoi( argv[1] ) ) <= 1024 ) {
//  //      fprintf( stderr, "Port number must be above 1024.\n" );
//  //      exit( 1 );
//  //    }
//  //  }
//  if ( argc > 1 ) { // found at least 1 argument
//    if ( !is_number( argv[1] ) ) { // if 1st arg is not a number it must be a config
//      if ( !load_config( argv[1] ) ) {
//	fprintf( stderr, "Config [%s] not found.", argv[1] );
//	exit(1);
//      }
//    }
//    else // 1st arg is a number so it's the port
//      port = atoi( argv[1] );
//    if ( port <= 1024 ) {
//      fprintf( stderr, "Port number must be above 1024.\n" );
//      exit( 1 );
//    }
//  }
//	
//  //Check Copyovering -- Oxtal.   modified by SinaC 2003: config file
//  //if (argc > 2 && argv[2][0]) { // FALSE, minimum 4 arguments
//  //  fCopyOver = TRUE;
//  //  control = atoi(argv[3]);
//  //}
//  if ( argc > 3 ) { // copyover, minimum 4 arguments
//    if ( !is_number( argv[1] ) ) { // argv: exe <config> <port> copyover/state <control>
//      if ( !str_cmp( argv[3], "copyover" ) ) {
//	// port in config file may be modified but we want old port
//	//  so we get it from argument
//	port = atoi( argv[2] );
//	fCopyOver = TRUE;
//	fState = FALSE;
//	control = atoi(argv[4]);
//      }
//      else if ( !str_cmp( argv[3], "state" ) ) {
//	// port in config file may be modified but we want old port
//	//  so we get it from argument
//	port = atoi( argv[2] );
//	fCopyOver = TRUE;
//	fState = TRUE;
//	control = atoi(argv[4]);
//      }
//      else {
//	fprintf( stderr, "Invalid 4th argument\n" );
//	exit(1);
//      }
//    }
//    else {                         // argv: exe <port> copyover/state <control>
//      if ( !str_cmp( argv[2], "copyover" ) ) {
//	// port has already been read
//	fCopyOver = TRUE;
//	fState = FALSE;
//	control = atoi(argv[3]);
//      }
//      else if ( !str_cmp( argv[2], "state" ) ) {
//	// port has already been read
//	fCopyOver = TRUE;
//	fState = TRUE;
//	control = atoi(argv[3]);
//      }
//      else {
//	fprintf( stderr, "Invalid 3rd argument\n" );
//	exit(1);
//      }
//    }
//  }
    
  /*
   * Run the game.
   */
#if defined(macintosh) || defined(MSDOS)
  boot_db( fCopyOver );
  log_string( "Merc is ready to rock." );
  game_loop_mac_msdos( );
#endif

#if defined(unix)

  if (!fCopyOver) /* We have already the port if copyover'ed */
    control = init_socket (port);
  boot_db( fState );
  sprintf( log_buf, "ROM is ready to rock on port %d.", port );
  log_string( log_buf );
  if (fCopyOver)
    copyover_recover();
  if (fState) {
    load_world_state(); // read world state
    if ( fCopyOver ) { // only if we were doing a copyover
      // we have to finish parsing player extra fields and set player clazz, SinaC 2003
      DESCRIPTOR_DATA *d_next = NULL;
      for ( DESCRIPTOR_DATA *d = descriptor_list; d != NULL; d = d_next ) {
	d_next = d->next;
	CHAR_DATA *ch = d->character;
	if ( ch == NULL || !ch->valid ) {
	  log_stringf("Player: host %s has been lost between copyover_recover and load_world_state", d->host );
	  continue;
	}
	do_look (ch, "auto");
	// Finish parsing extra fields
	use_delayed_parse_extra_fields( ch );
	if ( ch->pet != NULL )
	  use_delayed_parse_extra_fields( ch->pet );
	
	ch->clazz = default_player_class;
	MOBPROG( ch, NULL, "onLoad" );
      }
    }
    fState = FALSE; // DON'T FORGET TO RESET fState
  }

  // Added by SinaC 2003 for SIGSEGV capture
  if ( CAPTURE_SIGSEGV )
    init_signals();

  // Start the engine :)
  game_loop_unix( control );
  close (control);
#endif

  /*
   * That's all, folks.
   */
  log_string( "Normal termination of game." );
  exit( 0 );
  return 0;
}



#if defined(unix)
int init_socket( int port )
{
  static struct sockaddr_in sa_zero;
  struct sockaddr_in sa;
  int x = 1;
  int fd;

  if ( ( fd = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 ) {
    perror( "Init_socket: socket" );
    exit( 1 );
  }

  if ( setsockopt( fd, SOL_SOCKET, SO_REUSEADDR,
		   (char *) &x, sizeof(x) ) < 0 ) {
    perror( "Init_socket: SO_REUSEADDR" );
    close(fd);
    exit( 1 );
  }

#if defined(SO_DONTLINGER) && !defined(SYSV)
  {
    struct	linger	ld;

    ld.l_onoff  = 1;
    ld.l_linger = 1000;

    if ( setsockopt( fd, SOL_SOCKET, SO_DONTLINGER,
		     (char *) &ld, sizeof(ld) ) < 0 ) {
      perror( "Init_socket: SO_DONTLINGER" );
      close(fd);
      exit( 1 );
    }
  }
#endif

  sa		    = sa_zero;
  sa.sin_family   = AF_INET;
  sa.sin_port	    = htons( port );

  if ( bind( fd, (struct sockaddr *) &sa, sizeof(sa) ) < 0 ) {
    perror("Init socket: bind" );
    close(fd);
    exit(1);
  }


  if ( listen( fd, 3 ) < 0 ) {
    perror("Init socket: listen");
    close(fd);
    exit(1);
  }

  return fd;
}
#endif



#if defined(macintosh) || defined(MSDOS)
void game_loop_mac_msdos( void )
{
  struct timeval last_time;
  struct timeval now_time;
  static DESCRIPTOR_DATA dcon;

  gettimeofday( &last_time, NULL );
  current_time = (time_t) last_time.tv_sec;

  /*
   * New_descriptor analogue.
   */
  dcon.descriptor	= 0;
  dcon.connected	= CON_GET_NAME;

  dcon.host		= str_dup( "localhost" );
  dcon.outsize	= 2000;
  dcon.outbuf		= GC_MALLOC_ATOMIC( dcon.outsize );
  memset( dcon.outbuf, 0, dcon.outsize );
  dcon.next		= descriptor_list;
  dcon.showstr_head	= NULL;
  dcon.showstr_point	= NULL;
  dcon.pEdit		= NULL;			/* OLC */
  dcon.pString	= NULL;			/* OLC */
  dcon.editor		= 0;			/* OLC */
  descriptor_list	= &dcon;

  /*
   * Send the greeting.
   */
  {
    if ( help_greeting[0] == '.' )
      write_to_buffer( &dcon, help_greeting+1, 0 );
    else
      write_to_buffer( &dcon, help_greeting  , 0 );
  }

  /* Main loop */
  while ( !merc_down ) {
    DESCRIPTOR_DATA *d;

    /*
     * Process input.
     */
    for ( d = descriptor_list; d != NULL; d = d_next ) {
      d_next	= d->next;
      d->fcommand	= FALSE;

#if defined(MSDOS)
      if ( kbhit( ) )
#endif
	{
	  if ( d->character != NULL )
	    d->character->timer = 0;
	  if ( !read_from_descriptor( d ) ) {
	    if ( d->character != NULL)
	      //save_char_obj( d->character );
	      new_save_pFile(d->character, TRUE );
	    d->outtop	= 0;
	    close_socket( d );
	    continue;
	  }
	}

      if (d->character != NULL && d->character->daze > 0)
	--d->character->daze;

      if ( d->character != NULL && d->character->wait > 0 ) {
	--d->character->wait;
	continue;
      }

      read_from_buffer( d );

      // Added by SinaC 2001 for rebirth
      // after being in CON_REBIRTH, the race will be automatically selected, 
      //  and the player will start in the customize section
      if ( d->connected == CON_REBIRTH || d->connected == CON_REMORT ) {
	stop_fighting( d->character, TRUE ); // to be sure
	nanny( d, d->incomm );
      }	
	if ( d->incomm[0] != '\0' ) {
	d->fcommand	= TRUE;
	stop_idling( d->character );

	/* OLC */
	if ( d->showstr_point )
	  show_string( d, d->incomm );
	else
	  if ( d->pString )
	    string_add( d->character, d->incomm );
	  else
	    switch ( d->connected ) {
	    case CON_PLAYING:
	      if ( !run_olc_editor( d ) )
		substitute_alias( d, d->incomm );
	      break;
	    default:
	      nanny( d, d->incomm );
	      break;
	    }

	d->incomm[0]	= '\0';
      }
    }



    /*
     * Autonomous game motion.
     */
    update_handler( );



    /*
     * Output.
     */
    for ( d = descriptor_list; d != NULL; d = d_next ) {
      d_next = d->next;

      if ( ( d->fcommand || d->outtop > 0 ) ) {
	if ( !process_output( d, TRUE ) ) {
	  if ( d->character != NULL && d->character->level > 1)
	    //save_char_obj( d->character );
	    new_save_pFile( d->character, TRUE );
	  d->outtop	= 0;
	  close_socket( d );
	}
      }
    }



    /*
     * Synchronize to a clock.
     * Busy wait (blargh).
     */
    now_time = last_time;
    for ( ; ; ) {
      int delta;

#if defined(MSDOS)
      if ( kbhit( ) )
#endif
	{
	  if ( dcon.character != NULL )
	    dcon.character->timer = 0;
	  if ( !read_from_descriptor( &dcon ) ) {
	    if ( dcon.character != NULL && d->character->level > 1)
	      //save_char_obj( d->character );
	      new_save_pFile(dcon->character, TRUE );
	    dcon.outtop	= 0;
	    close_socket( &dcon );
	  }
#if defined(MSDOS)
	  break;
#endif
	}

      gettimeofday( &now_time, NULL );
      delta = ( now_time.tv_sec  - last_time.tv_sec  ) * 1000 * 1000
	+ ( now_time.tv_usec - last_time.tv_usec );
      if ( delta >= 1000000 / PULSE_PER_SECOND )
	break;
    }
    last_time    = now_time;
    current_time = (time_t) last_time.tv_sec;
  }

  return;
}
#endif


// SinaC 2003
//#define TRACE_CODE
#ifdef TRACE_CODE
#define TRACE log_stringf\
("EXEC TRACE %s (%d)", __PRETTY_FUNCTION__, __LINE__)
#endif
#ifndef TRACE_CODE
#define TRACE
#endif

#if defined(unix)
void game_loop_unix( int control )
{
  static struct timeval null_time;
  struct timeval last_time;

  signal( SIGPIPE, SIG_IGN );
  gettimeofday( &last_time, NULL );
  current_time = (time_t) last_time.tv_sec;

  /* Main loop */
  while ( !merc_down ) {
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;
    DESCRIPTOR_DATA *d;
    int maxdesc;

#if defined(MALLOC_DEBUG)
    if ( malloc_verify( ) != 1 )
      abort( );
#endif

    /*
     * Poll all active descriptors.
     */
    FD_ZERO( &in_set  );
    FD_ZERO( &out_set );
    FD_ZERO( &exc_set );
    FD_SET( control, &in_set );

    maxdesc	= control;
    for ( d = descriptor_list; d; d = d->next ) {
      TRACE;

      maxdesc = UMAX( maxdesc, d->descriptor );
      FD_SET( d->descriptor, &in_set  );
      FD_SET( d->descriptor, &out_set );
      FD_SET( d->descriptor, &exc_set );
    }

    if ( select( maxdesc+1, &in_set, &out_set, &exc_set, &null_time ) < 0 ) {
      perror( "Game_loop: select: poll" );
      exit( 1 );
    }

    /*
     * New connection?
     */
    // FIXME: after 2 consecutive copyover, FD_ISSET stays true even after init_descriptor
    //  accept is frozen
    //  +  previous select doesn't reset  control, &in_set
    if ( FD_ISSET( control, &in_set ) )
      init_descriptor( control );
    
    /*
     * Kick out the freaky folks.
     */
    for ( d = descriptor_list; d != NULL; d = d_next ) {
      TRACE;

      d_next = d->next;
      if ( FD_ISSET( d->descriptor, &exc_set ) ) {
	FD_CLR( d->descriptor, &in_set  );
	FD_CLR( d->descriptor, &out_set );
	if ( d->character && d->character->level > 1)
	  //save_char_obj( d->character );
	  new_save_pFile(d->character,TRUE);
	d->outtop	= 0;
	close_socket( d );
      }
    }

    /*
     * Process input.
     */
    for ( d = descriptor_list; d != NULL; d = d_next ) {
      TRACE;

      d_next	= d->next;
      d->fcommand	= FALSE;
      
      if ( FD_ISSET( d->descriptor, &in_set ) ) {
	if ( d->character != NULL )
	  d->character->timer = 0;
	if ( !read_from_descriptor( d ) ) {
	  FD_CLR( d->descriptor, &out_set );
	  if ( d->character != NULL && d->character->level > 1)
	    //save_char_obj( d->character );
	    new_save_pFile(d->character,TRUE);
	  d->outtop	= 0;
	  close_socket( d );
	  continue;
	}
      }

      if (d->character != NULL && d->character->daze > 0)
	--d->character->daze;

      if ( d->character != NULL && d->character->wait > 0 ) {
	--d->character->wait;
	continue;
      }

      read_from_buffer( d );

      // Added by SinaC 2001 for rebirth
      // after being in CON_REBIRTH, the race will be automatically selected, 
      //  and the player will start in the customize section
      if ( d->connected == CON_REBIRTH || d->connected == CON_REMORT ) {
	stop_fighting( d->character, TRUE ); // to be sure
	nanny( d, d->incomm );
      }

      if ( d->incomm[0] != '\0' ) {
	d->fcommand	= TRUE;
	stop_idling( d->character );

	/* OLC */
	if ( d->showstr_point )
	  show_string( d, d->incomm );
	else
	  if ( d->pString )
	    string_add( d->character, d->incomm );
	  else
	    switch ( d->connected ) {
	    case CON_PLAYING:
	      if ( !run_olc_editor( d ) )
		substitute_alias( d, d->incomm );
	      break;
	    default:
	      nanny( d, d->incomm );
	      break;
	    }

	d->incomm[0]	= '\0';
      }
    }

    /*
     * Autonomous game motion.
     */
    update_handler( );



    /*
     * Output.
     */
    for ( d = descriptor_list; d != NULL; d = d_next ) {
      TRACE;

      d_next = d->next;

      if ( ( d->fcommand || d->outtop > 0 )
	   &&   FD_ISSET(d->descriptor, &out_set) ) {
	if ( !process_output( d, TRUE ) ) {
	  if ( d->character != NULL && d->character->level > 1)
	    //save_char_obj( d->character );
	    new_save_pFile(d->character,TRUE);
	  d->outtop	= 0;
	  close_socket( d );
	}
      }
    }


    /*
     * Synchronize to a clock.
     * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
     * Careful here of signed versus unsigned arithmetic.
     */
    {
      struct timeval now_time;
      long secDelta;
      long usecDelta;

      gettimeofday( &now_time, NULL );
      usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
	+ 1000000 / PULSE_PER_SECOND;
      secDelta	= ((int) last_time.tv_sec ) - ((int) now_time.tv_sec );
      while ( usecDelta < 0 ) {
	usecDelta += 1000000;
	secDelta  -= 1;
      }

      while ( usecDelta >= 1000000 ) {
	usecDelta -= 1000000;
	secDelta  += 1;
      }

      if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) ) {
	struct timeval stall_time;

	stall_time.tv_usec = usecDelta;
	stall_time.tv_sec  = secDelta;
	if ( select( 0, NULL, NULL, NULL, &stall_time ) < 0 ) {
	  perror( "Game_loop: select: stall" );
	  exit( 1 );
	}
      }
    }

    gettimeofday( &last_time, NULL );
    current_time = (time_t) last_time.tv_sec;

    tics++; // SinaC 2003
  }

  return;
}
#endif



#if defined(unix)
void init_descriptor( int control )
{
  char buf[MAX_STRING_LENGTH];
  DESCRIPTOR_DATA *dnew;
  struct sockaddr_in sock;
  struct hostent *from;
  int desc;
  // Modified for GNUC3 ;)
  socklen_t size;

  size = sizeof(sock);
  log_string("init_descriptor: start");
  sock.sin_family = 0; sock.sin_port = 0; sock.sin_addr.s_addr = 0;
  log_stringf("sock: fam:%d  port:%d  addr:%d  [%s]", 
	     sock.sin_family, sock.sin_port, sock.sin_addr.s_addr, sock.__pad );
  log_string("before getsockname");
  getsockname( control, (struct sockaddr *) &sock, &size );
  log_string("after getsockname");
  log_stringf("sock: fam:%d  port:%d  addr:%d  [%s]", 
	     sock.sin_family, sock.sin_port, sock.sin_addr.s_addr, sock.__pad );
  log_string("before accept");
  if ( ( desc = accept( control, (struct sockaddr *) &sock, &size) ) < 0 ) {
    perror( "New_descriptor: accept" );
    return;
  }
  log_string("after accept");
  log_stringf("sock: fam:%d  port:%d  addr:%d  [%s]", 
	     sock.sin_family, sock.sin_port, sock.sin_addr.s_addr, sock.__pad );

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

 if ( fcntl( desc, F_SETFL, FNDELAY ) == -1 ) {
    perror( "New_descriptor: fcntl: FNDELAY" );
    return;
  }

  /*
   * Cons a new descriptor.
   */
  dnew = new_descriptor(); /* Also allocates now */
  dnew->descriptor	= desc;

  size = sizeof(sock);
  if ( getpeername( desc, (struct sockaddr *) &sock, &size ) < 0 ) {
    perror( "New_descriptor: getpeername" );
    dnew->host = str_dup( "(unknown)" );
  }
  else {
    /*
     * Would be nice to use inet_ntoa here but it takes a struct arg,
     * which ain't very compatible between gcc and system libraries.
     */
    int addr;

    addr = ntohl( sock.sin_addr.s_addr );
    sprintf( buf, "%d.%d.%d.%d",
	     ( addr >> 24 ) & 0xFF, ( addr >> 16 ) & 0xFF,
	     ( addr >>  8 ) & 0xFF, ( addr       ) & 0xFF
	     );
    sprintf( log_buf, "Sock.sinaddr:  %s", buf );
    log_string( log_buf );
    /*
    from = gethostbyaddr( (char *) &sock.sin_addr,
			  sizeof(sock.sin_addr), AF_INET );
    dnew->host = str_dup( from ? from->h_name : buf );
    */
    dnew->host = str_dup( buf );
  }

  /*
   * Swiftest: I added the following to ban sites.  I don't
   * endorse banning of sites, but Copper has few descriptors now
   * and some people from certain sites keep abusing access by
   * using automated 'autodialers' and leaving connections hanging.
   *
   * Furey: added suffix check by request of Nickel of HiddenWorlds.
   */
  if ( check_ban(dnew->host,BAN_ALL)) {
    write_to_descriptor( desc,
			 "Your site has been banned from this mud.\n\r", 0 );
    close( desc );
    return;
  }

  /*
   * Init descriptor data.
   */
  dnew->next			= descriptor_list;
  descriptor_list		= dnew;

  /*
   * Send the greeting.
   */
  // Modified by SinaC 2001 for color greet
  /*
  {
    if ( help_greeting[0] == '.' )
      write_to_buffer( dnew, help_greeting+1, 0 );
    else
      write_to_buffer( dnew, help_greeting  , 0 );
  }
  */
  write_to_buffer( dnew, "\n\rDo you want ANSI color? [Y/N] ", 0 );

  return;
}
#endif



void close_socket( DESCRIPTOR_DATA *dclose )
{
  CHAR_DATA *ch;

  if ( dclose->outtop > 0 )
    process_output( dclose, FALSE );

  if ( dclose->snoop_by != NULL ) {
    write_to_buffer( dclose->snoop_by,
		     "Your victim has left the game.\n\r", 0 );
  }

  {
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d != NULL; d = d->next ) {
      if ( d->snoop_by == dclose )
	d->snoop_by = NULL;
    }
  }

  //log_string("CLOSE_SOCKET: Avant ch==dclose->character");

  if ( ( ch = dclose->character ) != NULL ) {
    sprintf( log_buf, "Closing link to %s.", ch->name );
    log_string( log_buf );
    /* cut down on wiznet spam when rebooting */
    if ( ( dclose->connected == CON_PLAYING
	   || (dclose->connected >= CON_NOTE_TO && dclose->connected <= CON_NOTE_FINISH) ) 
	 && !merc_down ) {
      act( "$n has lost $s link.", ch, NULL, NULL, TO_ROOM );
      wiznet("Net death has claimed $N.",ch,NULL,WIZ_LINKS,0,0);
      ch->desc = NULL;
    }
    else {
      // nuke_pets added by SinaC 2000
      nuke_pets(dclose->original ? dclose->original : 
		dclose->character);

      free_char(dclose->original ? dclose->original : 
		dclose->character );
    }
  }

  //log_string("CLOSE_SOCKET: Avant free_run");

  // Added by SinaC 2000 for 'jog'
  free_runbuf(dclose);
  //log_string("CLOSE_SOCKET: Avant d_next == d_close");

  if ( d_next == dclose )
    d_next = d_next->next;   

  if ( dclose == descriptor_list ) {
    descriptor_list = descriptor_list->next;
  }
  else {
    DESCRIPTOR_DATA *d;

    for ( d = descriptor_list; d && d->next != dclose; d = d->next )
      ;
    if ( d != NULL )
      d->next = dclose->next;
    else
      bug( "Close_socket: dclose not found." );
  }

  //log_string("CLOSE_SOCKET: Avant close( dclose..");

  close( dclose->descriptor );

#if defined(MSDOS) || defined(macintosh)
  exit(1);
#endif
  return;
}



bool read_from_descriptor( DESCRIPTOR_DATA *d )
{
  int iStart;

  /* Hold horses if pending command already. */
  if ( d->incomm[0] != '\0' )
    return TRUE;

  /* Check for overflow. */
  iStart = strlen(d->inbuf);
  if ( iStart >= (int)sizeof(d->inbuf) - 10 ) {
    sprintf( log_buf, "%s input overflow!", d->host );
    log_string( log_buf );
    write_to_descriptor( d->descriptor,
			 "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
    return FALSE;
  }

  /* Snarf input. */
#if defined(macintosh)
  for ( ; ; ) {
    int c;
    c = getc( stdin );
    if ( c == '\0' || c == EOF )
      break;
    putc( c, stdout );
    if ( c == '\r' )
      putc( '\n', stdout );
    d->inbuf[iStart++] = c;
    if ( iStart > sizeof(d->inbuf) - 10 )
      break;
  }
#endif

#if defined(MSDOS) || defined(unix)
  for ( ; ; ) {
    int nRead;

    nRead = read( d->descriptor, d->inbuf + iStart,
		  sizeof(d->inbuf) - 10 - iStart );
    if ( nRead > 0 ) {
      iStart += nRead;
      if ( d->inbuf[iStart-1] == '\n' || d->inbuf[iStart-1] == '\r' )
	break;
    }
    else if ( nRead == 0 ) {
      log_string( "EOF encountered on read." );
      return FALSE;
    }
    else if ( errno == EWOULDBLOCK )
      break;
    else {
      perror( "Read_from_descriptor" );
      return FALSE;
    }
  }
#endif

  d->inbuf[iStart] = '\0';
  return TRUE;
}



/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer( DESCRIPTOR_DATA *d )
{
  int i, j, k;

  /*
   * Hold horses if pending command already.
   */
  if ( d->incomm[0] != '\0' )
    return;

  // FIXME: non constant string <---
  // Added by SinaC 2000 for 'jog'
  if ( d->run_buf ) {
    // Read number
    while (isdigit(*d->run_head) && *d->run_head != '\0') {
      char *s,*e;
	 
      s = d->run_head;
      while( isdigit( *s ) )
	s++;
      e = s;
      while( *(--s) == '0' && s != d->run_head );
      if ( isdigit( *s ) && *s != '0' && *e != 'o') {
	// Added by SinaC 2003
	if ( *e == 'N' || *e == 'S' ) {
	  // don't need to check if next char is E or W, already checked in do_jog
	  d->incomm[0] = LOWER(*e); e++;
	  d->incomm[1] = LOWER(*e);
	  d->incomm[2] = '\0';
	}
	else {
	  d->incomm[0] = *e;
	  d->incomm[1] = '\0';
	}
	s[0]--;
	while (isdigit(*(++s)))
	  *s = '9';
	return;
      }
      if (*e == 'o')
	d->run_head = e;
      else {
	if ( *s == '0' && ( *e == 'N' || *e == 'S' ) ) // Added by SinaC 2003
	  d->run_head = ++e;
	d->run_head = ++e;
      }
    }
    // Read direction
    if (*d->run_head != '\0') {
      // move
      if (*d->run_head != 'o') {
	// Added by SinaC 2003, 4 new directions: NE, NW, SE, SW
	if ( *d->run_head == 'N' || *d->run_head == 'S' ) {
	  // don't need to check if next char is E or W, already checked in do_jog
	  d->incomm[0] = LOWER(*d->run_head); d->run_head++;
	  d->incomm[1] = LOWER(*d->run_head); d->run_head++;
	  d->incomm[2] = '\0';
	}
	else {
	  d->incomm[0] = *d->run_head++;
	  d->incomm[1] = '\0';
	}
	return;
      }
      // open door
      else {
	char buf[MAX_INPUT_LENGTH];
	     
	d->run_head++;
	     
	sprintf( buf, "open " );
	switch( *d->run_head ) { // SinaC 2003: strcat were sprintf(buf+strlen(buf)) before
	case 'n' : strcat(buf, "north" ); break;
	case 's' : strcat(buf, "south" ); break;
	case 'e' : strcat(buf, "east"  ); break;
	case 'w' : strcat(buf, "west"  ); break;
	case 'u' : strcat(buf, "up"    ); break;
	case 'd' : strcat(buf, "down"  ); break;
	  // Added by SinaC 2003, 4 new directions: NE, NW, SE, SW
	case 'N': d->run_head++; 
	  if ( *d->run_head == 'E' ) strcat(buf,"northeast"); 
	  else strcat(buf,"northwest");
	  break;
	case 'S': d->run_head++; 
	  if ( *d->run_head == 'E' ) strcat(buf,"southeast"); 
	  else strcat(buf,"southwest");
	  break;
	default: return;
	}
	     
	strcpy( d->incomm, buf );
	d->run_head++;
	return;
      }
    }

    send_to_char( "{GYou have reached your destination.{x\n\r", d->character );
    free_runbuf(d);
    do_look( d->character, "auto" );

  }
  //end of addition for 'jog'   

  /*
   * Look for at least one new line.
   */
  for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ ) {
    if ( d->inbuf[i] == '\0' )
      return;
  }

  /*
   * Canonical input processing.
   */
  for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ ) {
    if ( k >= MAX_INPUT_LENGTH - 2 ) {
      write_to_descriptor( d->descriptor, "Line too long.\n\r", 0 );

      /* skip the rest of the line */
      for ( ; d->inbuf[i] != '\0'; i++ ) {
	if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
	  break;
      }
      d->inbuf[i]   = '\n';
      d->inbuf[i+1] = '\0';
      break;
    }

    if ( d->inbuf[i] == '\b' && k > 0 )
      --k;
    else if ( isascii(d->inbuf[i]) && isprint(d->inbuf[i]) )
      d->incomm[k++] = d->inbuf[i];
  }

  /*
   * Finish off the line.
   */
  if ( k == 0 )
    d->incomm[k++] = ' ';
  d->incomm[k] = '\0';

  /*
   * Deal with bozos with #repeat 1000 ...
   */

  if ( k > 1 || d->incomm[0] == '!' ) {
    if ( d->incomm[0] != '!' && strcmp( d->incomm, d->inlast ) ) {
      d->repeat = 0;
    }
    else {
      if (++d->repeat >= 25 && d->character
	  &&  d->connected == CON_PLAYING) {
	// Modified by SinaC 2000
	sprintf( log_buf, "%s (from %s) input spamming!", 
		 d->character->name,d->host );
	log_string( log_buf );
	wiznet("Spam spam spam $N spam spam spam spam spam!",
	       d->character,NULL,WIZ_SPAM,0,get_trust(d->character));
	if (d->incomm[0] == '!')
	  wiznet(d->inlast,d->character,NULL,WIZ_SPAM,0,
		 get_trust(d->character));
	else
	  wiznet(d->incomm,d->character,NULL,WIZ_SPAM,0,
		 get_trust(d->character));

	d->repeat = 0;
	/*
	  write_to_descriptor( d->descriptor,
	  "\n\r*** PUT A LID ON IT!!! ***\n\r", 0 );
	  strcpy( d->incomm, "quit" );
	*/
      }
    }
  }


  /*
   * Do '!' substitution.
   */
  if ( d->incomm[0] == '!' )
    strcpy( d->incomm, d->inlast );
  else
    strcpy( d->inlast, d->incomm );

  /*
   * Shift the input buffer.
   */
  while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
    i++;
  for ( j = 0; ( d->inbuf[j] = d->inbuf[i+j] ) != '\0'; j++ )
    ;
  return;
}



/*
 * Low level output function.
 */
bool process_output( DESCRIPTOR_DATA *d, bool fPrompt )
{

  /*
   * Bust a prompt.
   */
  if ( !merc_down )
    if ( d->showstr_point )
      write_to_buffer( d, "[Hit Return to continue]\n\r", 0 );
    else if ( fPrompt && d->pString && d->connected == CON_PLAYING )
      write_to_buffer( d, "> ", 2 );
    else if ( fPrompt && d->connected == CON_PLAYING ) {
      CHAR_DATA *ch;
      CHAR_DATA *victim;

      ch = d->character;

      /* battle prompt */
      if ( (victim = ch->fighting) != NULL 
	   && can_see(ch,victim)
	   && victim->in_room == ch->in_room ) { // SinaC 2003
	int percent;
	char wound[100];
	char *pbuff;
	char buf[MAX_STRING_LENGTH];
	char buffer[MAX_STRING_LENGTH*2];
 
	if (victim->cstat(max_hit) > 0)
	  percent = victim->hit * 100 / victim->cstat(max_hit);
	else
	  percent = -1;
 
	if (percent >= 100)
	  sprintf(wound,"is in excellent condition.");
	else if (percent >= 90)
	  sprintf(wound,"has a few scratches.");
	else if (percent >= 75)
	  sprintf(wound,"has some small wounds and bruises.");
	else if (percent >= 50)
	  sprintf(wound,"has quite a few wounds.");
	else if (percent >= 30)
	  sprintf(wound,"has some big nasty wounds and scratches.");
	else if (percent >= 15)
	  sprintf(wound,"looks pretty hurt.");
	else if (percent >= 0)
	  sprintf(wound,"is in awful condition.");
	else
	  sprintf(wound,"is bleeding to death.");
 
	sprintf(buf,"%s %s \n\r", 
		IS_NPC(victim) ? victim->short_descr : victim->name,wound);
	buf[0]	= UPPER( buf[0] );
	pbuff	= buffer;
	colourconv( pbuff, buf, d->character );
	write_to_buffer( d, buffer, 0);

	// Added by SinaC 2000
	if (victim->stunned) {
	  sprintf(buf,"{f%s is stunned.{x\n\r", 
		  IS_NPC(victim) ? victim->short_descr : victim->name);
	  send_to_char(buf, ch);
	}
      }


      ch = d->original ? d->original : d->character;
      if (!IS_SET(ch->comm, COMM_COMPACT) )
	write_to_buffer( d, "\n\r", 2 );


      if ( IS_SET(ch->comm, COMM_PROMPT) )
	bust_a_prompt( d->character );

      if (IS_SET(ch->comm,COMM_TELNET_GA))
	write_to_buffer(d,go_ahead_str,0);
    }

  /*
   * Short-circuit if nothing to write.
   */
  if ( d->outtop == 0 )
    return TRUE;

  /*
   * Snoop-o-rama.
   */
  if ( d->snoop_by != NULL ) {
    if (d->character != NULL)
      write_to_buffer( d->snoop_by, d->character->name,0);
    write_to_buffer( d->snoop_by, "> ", 2 );
    write_to_buffer( d->snoop_by, d->outbuf, d->outtop );
  }

  /*
   * OS-dependent output.
   */
  if ( !write_to_descriptor( d->descriptor, d->outbuf, d->outtop ) ) {
    d->outtop = 0;
    return FALSE;
  }
  else {
    d->outtop = 0;
    return TRUE;
  }
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 */
void bust_a_prompt( CHAR_DATA *ch )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  const char *str;
  const char *i;
  char *point;
  char *pbuff;
  char buffer[ MAX_STRING_LENGTH*2 ];
  char doors[MAX_INPUT_LENGTH];
  EXIT_DATA *pexit;
  bool found;
  //const char *dir_name[] = {"N","E","S","W","U","D"};       Removed by SinaC 2003
  int door;
  CHAR_DATA *victim;
  int percent=0; 


  point = buf;
  str = ch->prompt;
  if( !str || str[0] == '\0') {
    // Added by SinaC 2001 for mental user
    /*
    sprintf( buf, "{c<%dhp %dm %dmv>{x %s",
	     ch->hit, ch->mana, ch->move, ch->prefix );
    */
    sprintf( buf, "{c<%dhp %dm %dp %dmv>{x %s",
	     ch->hit, ch->mana, ch->psp, ch->move, ch->prefix );
    send_to_char( buf, ch );
    return;
  }

  // Added by SinaC 2000
  if (IS_SET(ch->comm,COMM_BUILDING))
    send_to_char("{b<BUILDING>{x ",ch);
  // SinaC 2003, same as COMM_BUILDING but editing datas
  if (IS_SET(ch->comm,COMM_EDITING))
    send_to_char("{B<EDITING>{x ",ch);

  if (IS_SET(ch->comm,COMM_AFK)) {
    send_to_char("{G<AFK>{x ",ch);
    return;
  }


  while( *str != '\0' ) {
    if( *str != '%' ) {
      *point++ = *str++;
      continue;
    }
    ++str;
    switch( *str ) {
    default :
      i = " "; break;
    case 'e':
      found = FALSE;
      doors[0] = '\0';
      for (door = 0; door < MAX_DIR; door++) { // Modified by SinaC 2003
	if ((pexit = ch->in_room->exit[door]) != NULL
	    &&  pexit ->u1.to_room != NULL
	    &&  (can_see_room(ch,pexit->u1.to_room)
		 ||   (IS_AFFECTED(ch,AFF_INFRARED) 
		       &&    !IS_AFFECTED(ch,AFF_BLIND)))
	    &&  !IS_SET(pexit->exit_info,EX_CLOSED))
	  {
	    found = TRUE;
	    strcat(doors,capitalize(dir_name[door])); // Modified by SinaC 2003
	  }
      }
      if (!found)
	strcat(buf,"none");
      sprintf(buf2,"%s",doors);
      i = buf2; break;
    case 'b' :
      if ((victim = ch->fighting) != NULL
	  && victim->in_room == ch->in_room ) { // SinaC 2003
	if (victim->cstat(max_hit) > 0)
	  percent = victim->hit * 100 / victim->cstat(max_hit);
	else
	  percent = -1;
	sprintf(buf2," Enemy: %d%%",percent);
      }
      else
	buf2[0]=0;
      i = buf2; break;
    case 'c' :
      sprintf(buf2,"%s","\n\r");
      i = buf2; break;
    case 'h' :
      sprintf( buf2, "%d", ch->hit );
      i = buf2; break;
    case 'H' :
      sprintf( buf2, "%ld", ch->cstat(max_hit) );
      i = buf2; break;
    case 'm' :
      sprintf( buf2, "%d", ch->mana );
      i = buf2; break;
    case 'M' :
      sprintf( buf2, "%ld", ch->cstat(max_mana) );
      i = buf2; break;
      // Added by SinaC 2001 for mental user
    case 'p' :
      sprintf( buf2, "%d", ch->psp );
      i = buf2; break;
    case 'P' :
      sprintf( buf2, "%ld", ch->cstat(max_psp) );
      i = buf2; break;

    case 'v' :
      sprintf( buf2, "%d", ch->move );
      i = buf2; break;
    case 'V' :
      sprintf( buf2, "%ld", ch->cstat(max_move) );
      i = buf2; break;
    case 'x' :
      sprintf( buf2, "%d", ch->exp );
      i = buf2; break;
    case 'X' :
      sprintf(buf2, "%d", IS_NPC(ch) ? 0 :
	      (ch->level + 1) * exp_per_level(ch,ch->pcdata->points) - ch->exp);
      i = buf2; break;
    case 'g' :
      sprintf( buf2, "%ld", ch->gold);
      i = buf2; break;
    case 's' :
      sprintf( buf2, "%ld", ch->silver);
      i = buf2; break;
    case 'a' :
      if( ch->level > 9 )
	// Modified by SinaC 2001 etho/alignment are attributes now
	// Modified by SinaC 2000
	//sprintf( buf2, "%d", ch->align.alignment );
	sprintf( buf2, "%ld", ch->cstat(alignment));
      else
	sprintf( buf2, "%s", IS_GOOD(ch) ? "good" : IS_EVIL(ch) ?
		 "evil" : "neutral" );
      i = buf2; break;
    case 'r' :
      if( ch->in_room != NULL )
	// Modified by SinaC 2000
	sprintf( buf2, "%s", 
		 ((!IS_NPC(ch) && IS_SET(ch->act,PLR_HOLYLIGHT) && IS_IMMORTAL(ch)) ||
		  (!IS_AFFECTED(ch,AFF_BLIND) && !room_is_dark( ch->in_room )))
		 ? ch->in_room->name : "darkness");
      else
	sprintf( buf2, " " );
      i = buf2; break;
    case 'R' :
      if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
	sprintf( buf2, "%d", ch->in_room->vnum );
      else
	sprintf( buf2, " " );
      i = buf2; break;
    case 'z' :
      if( IS_IMMORTAL( ch ) && ch->in_room != NULL )
	sprintf( buf2, "%s", ch->in_room->area->name );
      else
	sprintf( buf2, " " );
      i = buf2; break;
    case '%' :
      sprintf( buf2, "%%" );
      i = buf2; break;
    case 'o' :
      sprintf( buf2, "%s", olc_ed_name(ch) );
      i = buf2; break;
    case 'O' :
      sprintf( buf2, "%s", olc_ed_vnum(ch) );
      i = buf2; break;   	
    }
    ++str;
    while( (*point = *i) != '\0' )
      ++point, ++i;
  }
  *point	= '\0';
  pbuff	= buffer;
  colourconv( pbuff, buf, ch );
  write_to_buffer( ch->desc, buffer, 0 );

  if (ch->prefix[0] != '\0')
    write_to_buffer(ch->desc,ch->prefix,0);
  return;
}



/*
 * Append onto an output buffer.
 */
void write_to_buffer( DESCRIPTOR_DATA *d, const char *txt, int length )
{
  /*
   * Find length in case caller didn't.
   */
  if ( length <= 0 )
    length = strlen(txt);

    /*
     * Initial \n\r if needed.
     */
  if ( d->outtop == 0 && !d->fcommand ) {
    d->outbuf[0]	= '\n';
    d->outbuf[1]	= '\r';
    d->outtop	= 2;
  }

  /*
     * Expand the buffer as needed.
     */
  while ( d->outtop + length >= d->outsize ) {
    char *outbuf;

    if (d->outsize >= 32000) {
      bug("Buffer overflow. Closing.\n\r");
      close_socket(d);
      return;
    }
    outbuf      = (char *)GC_MALLOC_ATOMIC( 2 * d->outsize );
    memset( outbuf, 0, 2*d->outsize );
    strncpy( outbuf, d->outbuf, d->outtop );
    d->outbuf   = outbuf;
    d->outsize *= 2;
  }

  /*
     * Copy.
     */
    // modified by SinaC 2000
    //    strcpy( d->outbuf + d->outtop, txt );
  strncpy( d->outbuf + d->outtop, txt, length );

  d->outtop += length;
  return;
}



/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
bool write_to_descriptor( int desc, char *txt, int length )
{
  int iStart;
  int nWrite;
  int nBlock;

#if defined(macintosh) || defined(MSDOS)
  if ( desc == 0 )
    desc = 1;
#endif

  if ( length <= 0 )
    length = strlen(txt);

  for ( iStart = 0; iStart < length; iStart += nWrite ) {
    nBlock = UMIN( length - iStart, 4096 );
    if ( ( nWrite = write( desc, txt + iStart, nBlock ) ) < 0 ) { 
      perror( "Write_to_descriptor" ); 
      return FALSE; 
    }
  } 

  return TRUE;
}


// Added by SinaC 2003, items given to player at creation are stored in class_table
// TO DO   outfit will be replaced with that
void give_creation_items( CHAR_DATA *ch ) {
  do_outfit(ch,"");
  obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);

  int classId = class_firstclass(ch->bstat(classes));
  class_type *cl = &(class_table[classId]);
  for ( int i = 0; i < cl->num_obj_list; i++ ) {
    OBJ_INDEX_DATA *pObj = get_obj_index(cl->obj_list[i]);
    if ( pObj == NULL ) {
      bug("Invalid obj_list [%d] = %d for class [%s]", i, cl->obj_list[i], cl->name );
      continue;
    }
    OBJ_DATA *obj = create_object( pObj, 0 );
    obj_to_char( obj, ch );
  }
}


/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny( DESCRIPTOR_DATA *d, const char *argument0 )
{
  DESCRIPTOR_DATA *d_old, *d_next;
  char buf[MAX_STRING_LENGTH];
  char arg[MAX_INPUT_LENGTH];
  CHAR_DATA *ch;
  char *pwdnew;
  char *p;
  // Modified by SinaC 2001, SinaC 2003
  int iClass,race,i,weapon,god,j, home;
  bool fOld;
  // removed by SinaC 2000
  //int sn;
  // Removed by SinaC 2001
  // Added by SinaC 2000
  //static int count = -1;
  // Added by SinaC 2001
  bool rebirth;
  bool remort;
  // Added by SinaC 2001
  int col;

  char *argument = new char [MAX_INPUT_LENGTH];
  strcpy(argument,argument0);

  if (d->connected != CON_NOTE_TEXT)
    while ( isspace(*argument) )
      argument++;

  ch = d->character;

  // Added by SinaC 2001
  rebirth = FALSE;
  remort = FALSE;
  if ( ch != NULL && ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_REBIRTH )
    rebirth = TRUE;
  else if ( ch != NULL && ch->in_room != NULL && ch->in_room->vnum == ROOM_VNUM_REMORT )
    remort = TRUE;
  

  switch ( d->connected ) {

  default:
    bug( "Nanny: bad d->connected %d.", d->connected );
    close_socket( d );
    return;
    
    // Added by SinaC 2001 for color greet
    // Every write_to_buffer has been modified to send_to_desc
  case CON_ANSI:
    if ( argument[0] == '\0' || UPPER(argument[0]) == 'Y' ) {
      d->ansi = TRUE;
      send_to_desc(d,"\n\r{RAnsi enabled!{x\n\r");
      d->connected = CON_GET_NAME;
      {
	if ( help_greeting[0] == '.' )
	  send_to_desc( d, help_greeting+1);
	else
	  send_to_desc( d, help_greeting);
      }
      break;
    }
    
    if (UPPER(argument[0]) == 'N') {
      d->ansi = FALSE;
      send_to_desc( d, "\n\rAnsi disabled!\n\r");
      d->connected = CON_GET_NAME;
      {
	if ( help_greeting[0] == '.' )
	  send_to_desc(d,  help_greeting+1);
	else
	  send_to_desc(d,  help_greeting);
      }
      break;
    }
    else {
      send_to_desc( d, "Do you want ANSI color? [Y/N] ");
      return;
    }
    break;
  case CON_GET_NAME:
    if ( argument[0] == '\0' ) {
      close_socket( d );
      return;
    }

    argument[0] = UPPER(argument[0]);
    if ( !check_parse_name( argument ) ) {
      //write_to_buffer( d, "Illegal name, try another.\n\rName: ", 0 );
      send_to_desc( d, "Illegal name, try another.\n\rName: " );
      log_stringf("Illegal name: %s", argument);
      return;
    }

    //fOld = load_char_obj( d, argument );
    fOld = new_load_pFile( d, argument );
    ch   = d->character;

    // Added by SinaC 2001
    // Added by SinaC 2001 for color greet
    if (d->ansi)
      SET_BIT(ch->act, PLR_COLOUR);
    else 
      REMOVE_BIT(ch->act, PLR_COLOUR);


    if (IS_SET(ch->act, PLR_DENY)) {
      sprintf( log_buf, "Denying access to %s@%s.", argument, d->host );
      log_string( log_buf );
      //write_to_buffer( d, "You are denied access.\n\r", 0 );
      send_to_desc( d, "{RYou are denied access.{x\n\r" );
      close_socket( d );
      return;
    }

    if (check_ban(d->host,BAN_PERMIT) && !IS_SET(ch->act,PLR_PERMIT)) {
      //write_to_buffer(d,"Your site has been banned from this mud.\n\r",0);
      send_to_desc(d,"{RYour site has been banned from this mud.{x\n\r");
      close_socket(d);
      return;
    }

    if ( check_reconnect( d, argument, FALSE ) ) {
      fOld = TRUE;
    }
    else {
      if ( wizlock && !IS_IMMORTAL(ch)) {
	//write_to_buffer( d, "The game is wizlocked.\n\r", 0 );
	send_to_desc( d, "{MThe game is wizlocked.{x\n\r" );
	close_socket( d );
	return;
      }
    }

    if ( fOld ) {
      /* Old player */
      //write_to_buffer( d, "Password: ", 0 );
      send_to_desc( d, "{cPassword: {x");
      //write_to_buffer( d, echo_off_str, 0 );
      send_to_desc( d, echo_off_str );
      d->connected = CON_GET_OLD_PASSWORD;
      return;
    }
    else {
      /* New player */
      if (newlock) {
	//write_to_buffer( d, "The game is newlocked.\n\r", 0 );
	send_to_desc( d, "{MThe game is newlocked.{x\n\r" );
	close_socket( d );
	return;
      }

      if (check_ban(d->host,BAN_NEWBIES)) {
	//write_to_buffer(d,
	//		"New players are not allowed from your site.\n\r",0);
	send_to_desc(d,
			"{RNew players are not allowed from your site.{x\n\r");
	close_socket(d);
	return;
      }

      do_help(d->character, "advertice" );
      //sprintf( buf, "Did I get that right, %s (Y/N)? ", argument );
      //write_to_buffer( d, buf, 0 );
      sprintf( buf, "{cDid I get that right, {y%s{c (Y/N)?{x ", argument );
      send_to_desc( d, buf );

      d->connected = CON_CONFIRM_NEW_NAME;
      return;
    }
    break;

  case CON_GET_OLD_PASSWORD:
#if defined(unix)
    write_to_buffer( d, "\n\r", 2 );
#endif

//    if ( strcmp( /*crypt(*/ argument/*, ch->pcdata->pwd )*/, ch->pcdata->pwd )) {
    if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd) ) {
      //write_to_buffer( d, "Wrong password.\n\r", 0 );
      send_to_desc( d, "{rWrong password.{x\n\r" );
      log_stringf("Wrong password: %s@%s", ch->name, d->host );
      close_socket( d );
      return;
    }
 
    // Added by SinaC 2000 to avoid cheating bastards
    if ( ch->level < 0 || ch->level > IMPLEMENTOR ){
      /*
      write_to_buffer( d, 
		       "Your level is not in an acceptable range.\n\r"
		       "Buh-Bye.\n\r", 0 );
      */
      send_to_desc( d, 
		       "{RYour level is not in an acceptable range.\n\r"
		       "Buh-Bye.{x\n\r" );
      log_stringf("CHEATER: %s@%s 's level is not in an acceptable range (lvl %d)",
		  ch->name, d->host, ch->level );
      close_socket( d );
      return;
    }

    //write_to_buffer( d, echo_on_str, 0 );
    send_to_desc( d, echo_on_str );

    if (check_playing(d,ch->name))
      return;

    if ( check_reconnect( d, ch->name, TRUE ) )
      return;

    sprintf( log_buf, "%s@%s has connected.", ch->name, d->host );
    log_string( log_buf );
    wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

    // SinaC 2003, pFile is saved in create_rebirth and create_remort
    //  added because player could just kill their connection if they don't want to rebirth
    if ( ch->pcdata->tmpRace > 0 ) // FIXME: how distinguish REBIRTH than REMORT
      d->connected = CON_REBIRTH;
    //else if ( ch->pcdata->connectedBefore == CON_REMORT )
    //  d->connected = CON_REMORT;
    else { // No remort/rebirth
      if ( IS_IMMORTAL(ch) ) {
	do_help( ch, "imotd" );
	d->connected = CON_READ_IMOTD;
      }
      else {
	do_help( ch, "motd" );
	d->connected = CON_READ_MOTD;
      }
    }
    break;

    /* RT code for breaking link */
 
  case CON_BREAK_CONNECT:
    switch( *argument ) {
    case 'y' : case 'Y':
      for ( d_old = descriptor_list; d_old != NULL; d_old = d_next ) {
	d_next = d_old->next;
	if (d_old == d || d_old->character == NULL)
	  continue;

	if (str_cmp(ch->name,d_old->original ?
		    d_old->original->name : d_old->character->name))
	  continue;

	// Added by SinaC 2001 to avoid people trying to open a new session to a player
	//  creating/rebirthing
	// before the creating/rebirthing were killed and the new session stayed
	if ( d_old->connected != CON_PLAYING )
	  continue;

	close_socket(d_old);
      }
      if (check_reconnect(d,ch->name,TRUE))
	return;
      //write_to_buffer(d,"Reconnect attempt failed.\n\rName: ",0);
      send_to_desc(d,"{cReconnect attempt failed.\n\rName: {x");
      if ( d->character != NULL ) {
	// nuke_pets added by SinaC 2000
	nuke_pets( d->character );

	free_char( d->character );
	d->character = NULL;
      }
      d->connected = CON_GET_NAME;
      break;

    case 'n' : case 'N':
      //write_to_buffer(d,"Name: ",0);
      send_to_desc(d,"{cName: {x");
      if ( d->character != NULL ) {
	// nuke_pets added by SinaC 2000
	nuke_pets( d->character );

	free_char( d->character );
	d->character = NULL;
      }
      d->connected = CON_GET_NAME;
      break;

    default:
      //write_to_buffer(d,"Please type Y or N? ",0);
      send_to_desc(d,"{CPlease type Y or N? {x");
      break;
    }
    break;

  case CON_CONFIRM_NEW_NAME:
    switch ( *argument ) {
    case 'y': case 'Y':
      //sprintf( buf, "New character.\n\rGive me a password for %s: %s",
      //       ch->name, echo_off_str );
      //write_to_buffer( d, buf, 0 );
      sprintf( buf, "{yNew character.\n\r{cGive me a password for {y%s{c: {x",
	       ch->name );
      send_to_desc( d, buf );
      send_to_desc( d, echo_off_str );
      d->connected = CON_GET_NEW_PASSWORD;
      break;

    case 'n': case 'N':
      //write_to_buffer( d, "Ok, what IS it, then? ", 0 );
      send_to_desc( d, "{yOk, what IS it, then?{x " );
	    
      if ( d->character != NULL ) {
	// nuke_pets added by SinaC 2000
	nuke_pets( d->character );
	
	free_char( d->character );
	d->character = NULL;
      }
      d->connected = CON_GET_NAME;
      break;

    default:
      //write_to_buffer( d, "Please type Yes or No? ", 0 );
      send_to_desc( d, "{CPlease type Yes or No?{x " );
      break;
    }
    break;

  case CON_GET_NEW_PASSWORD:
#if defined(unix)
    write_to_buffer( d, "\n\r", 2 );
#endif

    if ( strlen(argument) < 5 ) {
      //write_to_buffer( d, "Password must be at least five characters long.\n\rPassword: ",  0 );
      send_to_desc( d, 
		    "{CPassword must be at least five characters long.\n\r{cPassword: {x" );
      return;
    }

    pwdnew = crypt( argument, ch->name );
    //pwdnew = (char *)malloc(sizeof(argument));
    //strcpy( pwdnew, argument );
    //pwdnew = str_dup( argument );

    for ( p = pwdnew; *p != '\0'; p++ ) {
      if ( *p == '~' ) {
	//write_to_buffer( d, "New password not acceptable, try again.\n\rPassword: ", 0 );
	send_to_desc( d, "{CNew password not acceptable, try again.\n\r{cPassword: {x" );
	return;
      }
    }

    ch->pcdata->pwd	= str_dup( pwdnew );
    //write_to_buffer( d, "Please retype password: ", 0 );
    send_to_desc( d, "{cPlease retype password: {x");
    d->connected = CON_CONFIRM_NEW_PASSWORD;
    break;

  case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
    write_to_buffer( d, "\n\r", 2 );
#endif

    // Added by SinaC 2001
    if ( d->ansi )
      SET_BIT(ch->act, PLR_COLOUR);
    else
      REMOVE_BIT(ch->act, PLR_COLOUR);
    
//    if ( strcmp( /*crypt(*/ argument/*, ch->pcdata->pwd )*/, ch->pcdata->pwd ) ) {
    if ( strcmp( crypt( argument, ch->pcdata->pwd ), ch->pcdata->pwd ) ) {
      //write_to_buffer( d, "Passwords don't match.\n\rRetype password: ",
	//	       0 );
      send_to_desc( d, "{CPasswords don't match.\n\r{cRetype password: {x" );
      d->connected = CON_GET_NEW_PASSWORD;
      return;
    }

    // Added by SinaC 2003
    send_to_desc(d,"{WSetting new character flag.\n\r{x");
    send_to_desc(d,"{WNew character flag will be removed upon acceptance of you name.{x\n\r\n\r");


    // Removed by SinaC 2001
    //write_to_buffer( d, "==> Don't forget to check the newbie help ( typing help newbie ) <==\n\r", 0 );
    //write_to_buffer( d, echo_on_str, 0 );
    write_to_buffer(d,"The following races are available:\n\r\n\r",0);
    write_to_buffer( d, echo_on_str, 0 );

    //do_help( ch, "race creation" ); -> replaced with super race
    for ( race = 0; race < MAX_SUPERRACE; race++ ) {
      bool found = FALSE;
      for ( int k = 0; k < super_pc_race_table[race].nb_pc_race; k++ )
	if ( pc_race_table[super_pc_race_table[race].pc_race_list[k]].type == RACE_CREATION ) {
	  found = TRUE;
	  break;
	}
      if ( found ) {
	sprintf( buf, "{c%-12s{x: ", super_pc_race_table[race].name );
	send_to_desc( d, buf );
	int col = 0;
	found = FALSE;
	for ( int k = 0; k < super_pc_race_table[race].nb_pc_race; k++ )
	  if ( pc_race_table[super_pc_race_table[race].pc_race_list[k]].type == RACE_CREATION ) {
	    if ( found ) {
	      send_to_desc( d, "              ");
	      found = FALSE;
	    }
	    sprintf( buf, "{y%-15s{x ", pc_race_table[super_pc_race_table[race].pc_race_list[k]].name );
	    send_to_desc( d, buf );
	    if ( ++col % 3 == 0 ) {
	      send_to_desc( d, "\n\r" );
	      found = TRUE;
	    }
	  }
	if ( col % 3 != 0 )
	  send_to_desc( d, "\n\r" );
      }
    }

    send_to_desc(d,"\n\r");
    send_to_desc(d,"What is your race (help <race name> for more information)? ");
    d->connected = CON_GET_NEW_RACE;
    break;

  case CON_GET_NEW_RACE:
    one_argument(argument,arg);

    if (!strcmp(arg,"help")) {
      argument = (char *) one_argument(argument, arg); // OK coz argument is discarded here.
      if (argument[0] == '\0')
	do_help(ch,"race help");
      else
	do_help(ch,argument);
      write_to_buffer(d,
		      "What is your race (help <race name> for more information)? ",0);
      break;
    }

    race = race_lookup(argument);

    // Modified by SinaC 2000
    //if (race == 0 || !race_table[race].pc_race)
    if (race < 0 || !race_table[race].pc_race 
	// Added by SinaC 2001, modified by SinaC 2003
	//|| !pc_race_table[race].pickable ) {
	|| pc_race_table[race].type != RACE_CREATION ) {
      write_to_buffer(d,"That is not a valid race.\n\r",0);
      write_to_buffer(d,"The following races are available:\n\r\n\r",0);

      //do_help( ch, "race creation" );
      for ( race = 0; race < MAX_SUPERRACE; race++ ) {
	bool found = FALSE;
	for ( int k = 0; k < super_pc_race_table[race].nb_pc_race; k++ )
	  if ( pc_race_table[super_pc_race_table[race].pc_race_list[k]].type == RACE_CREATION ) {
	    found = TRUE;
	    break;
	  }
	if ( found ) {
	  sprintf( buf, "{c%-12s{x: ", super_pc_race_table[race].name );
	  send_to_desc( d, buf );
	  int col = 0;
	  found = FALSE;
	  for ( int k = 0; k < super_pc_race_table[race].nb_pc_race; k++ )
	    if ( pc_race_table[super_pc_race_table[race].pc_race_list[k]].type == RACE_CREATION ) {
	      if ( found ) {
		send_to_desc( d, "              ");
		found = FALSE;
	      }
	      sprintf( buf, "%-15s ", pc_race_table[super_pc_race_table[race].pc_race_list[k]].name );
	      send_to_desc( d, buf );
	      if ( ++col % 3 == 0 ) {
		send_to_desc( d, "\n\r" );
		found = TRUE;
	      }
	    }
	  if ( col % 3 != 0 )
	    send_to_desc( d, "\n\r" );
	}
      }
      
      write_to_buffer(d,"\n\r",0);
      write_to_buffer(d,
		      "What is your race? (help for more information) ",0);
      break;
    }

    ch->bstat(race) = race;
    /* initialize stats */
    for (i = 0; i < MAX_STATS; i++)
      ch->bstat(stat0+i) = pc_race_table[race].stats[i];
    ch->bstat(affected_by) = ch->bstat(affected_by)|race_table[race].aff;
    // Added by SinaC 2001
    ch->bstat(affected2_by) = ch->bstat(affected2_by)|race_table[race].aff2;
    ch->bstat(imm_flags)	= ch->bstat(imm_flags)|race_table[race].imm;
    ch->bstat(res_flags)	= ch->bstat(res_flags)|race_table[race].res;
    ch->bstat(vuln_flags)	= ch->bstat(vuln_flags)|race_table[race].vuln;
    ch->bstat(form)	= race_table[race].form;
    ch->bstat(parts)	= race_table[race].parts;


    /* By Oxtal*/
    //if (IS_SET(PART_CLAWS,ch->bstat(parts)))
    if (IS_SET(ch->bstat(parts),PART_CLAWS)) // SinaC 2003
      ch->bstat(dam_type) = 5; /* claws */
    else
      ch->bstat(dam_type) = 17; /*punch */

    /* add cost */
    // Modified by SinaC 2001
    //ch->pcdata->points = pc_race_table[race].points;
    ch->pcdata->points = 0;
    //ch->bstat(size) = pc_race_table[race].size;
    ch->bstat(size) = race_table[race].size;

    write_to_buffer( d, "\n\r", 0 );
    write_to_buffer( d, "What is your sex (Male/Female)? ", 0 );
    d->connected = CON_GET_NEW_SEX;
    break;


  case CON_GET_NEW_SEX:
    switch ( argument[0] ) {
    case 'm': case 'M': ch->bstat(sex) = SEX_MALE;
      break;
    case 'f': case 'F': ch->bstat(sex) = SEX_FEMALE;
      break;
    default:
      write_to_buffer( d, "That's not a sex.\n\rWhat IS your sex? ", 0 );
      return;
    }

    // Modified by SinaC 2001
    /*
    strcpy( buf, "Select a class:\n\r" );
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ ) {
      if ( check_class( ch, 1<<iClass ) ){
	if ( iClass > 0 )
	  strcat( buf, "  " );
	strcat( buf, class_table[iClass].name );
      }
    }
    write_to_buffer( d, buf, 0 );
    */
    write_to_buffer( d, "\n\r", 0 );
    write_to_buffer( d, "The following classes are available:\n\r", 0 ); 
    col = 0;
    for ( iClass = 0; iClass < MAX_CLASS; iClass++ ) {
      // second test added by SinaC 2003
      if ( !check_class_race( ch, iClass ) 
	   || class_table[iClass].choosable != CLASS_CHOOSABLE_YES )
	continue;
      sprintf( buf, "  {y%-15s{x", class_table[iClass].name);
      //write_to_buffer( d, buf, 0 );
      send_to_desc( d, buf );
      if ( ++col % 4 == 0 )
	write_to_buffer( d, "\n\r", 0 );
    }
    write_to_buffer( d, "\n\r", 0 );
    write_to_buffer( d, "What is your classes (help <class name> for more information) ?",
    	     0 );

    d->connected = CON_GET_NEW_CLASS;
    break;

  case CON_GET_NEW_CLASS:

    // Added by SinaC 2001
    one_argument(argument,arg);
    if (!strcmp(arg,"help")) {
      argument = (char *) one_argument(argument,arg); // OK coz argument is discarded here
      if (argument[0] == '\0')
	write_to_buffer( d, "You must specify a class.\n\r", 0 );
      else
	do_help(ch,argument);
      write_to_buffer( d, "What IS your class (help <class name> for more information) ?",
		       0 );
      //break;
      return;
    }

    iClass = class_lookup(argument); // should use  class_lookup( argument, TRUE )  ?

    // Modified by SinaC 2001, choosable test added by SinaC 2003
    if ( iClass == -1 
	 || !check_class_race( ch, iClass )
	 || class_table[iClass].choosable != CLASS_CHOOSABLE_YES ) {
      write_to_buffer( d,
		       "That's not an valid/allowed class.\n\r", 0 );
      write_to_buffer( d, "The following classes are available:\n\r", 0 ); 
      col = 0;
      for ( iClass = 0; iClass < MAX_CLASS; iClass++ ) {
	if ( !check_class_race( ch, iClass ) 
	     || class_table[iClass].choosable != CLASS_CHOOSABLE_YES ) 
	  continue;
	sprintf( buf, "  {y%-15s{x", class_table[iClass].name);
	//write_to_buffer( d, buf, 0 );
	send_to_desc( d, buf );
	if ( ++col % 4 == 0 )
	  write_to_buffer( d, "\n\r", 0 );
      }
      write_to_buffer( d, "\n\r", 0 );
      write_to_buffer( d,"What IS your class (help <class name> for more information) ?",
		       0 );
      return;
    }

    ch->bstat(classes) = 1<<iClass;
    small_recompute(ch); // FIXME: really useful ?

    sprintf( log_buf, "%s@%s new player.", ch->name, d->host );
    log_string( log_buf );
    wiznet("Newbie alert!  $N sighted.",ch,NULL,WIZ_NEWBIE,0,0);
    wiznet(log_buf,NULL,NULL,WIZ_SITES,0,get_trust(ch));

    write_to_buffer( d, "\n\r", 2 );

    if ( isWildable(ch->bstat(classes)) ) { // SinaC 2003: if wildable: goes in wildable choice
      write_to_buffer( d, "Would you like to be a wild magician (help wild magic for more information) ?", 0 );
      d->connected = CON_GET_WILD_MAGIC;
    }
    else {
      // Modified by SinaC 2001, ugly but it was the fastest/shortest/funniest way to do it
      /*
	write_to_buffer( d, "You may be good, neutral, or evil.\n\r",0);
	write_to_buffer( d, "Which alignment (G/N/E)? ",0);
      */
      for ( j = 1; j >= -1; j-- )
	for ( i = 1; i >= -1; i-- ){
	  if ( check_etho_align( ch, i, j*350 ) ){
	    sprintf( buf,
		     "  {W[%s] %s{x\n\r",
		     short_etho_align[(i+1)+(j+1)*3],
		     etho_align_name( i, j*350 ) );
	    //write_to_buffer( d, buf, 0 );
	    send_to_desc( d, buf );
	  }
	}
      write_to_buffer( d, "Which alignment (the 2 letters which are between [] or help)? ",0);
      d->connected = CON_GET_ALIGNMENT;
    }
    break;

    // SinaC 2003, if player has choosen a wildable class, ask if wild magic before alignment
  case CON_GET_WILD_MAGIC:
    one_argument(argument,arg);
    if (!strcmp(arg,"help")) {
      argument = (char *) one_argument(argument,arg); // OK coz argument is discarded here
      if (argument[0] != '\0')
	do_help(ch,argument);
      write_to_buffer( d, "Answer yes or no (help wild magic for more information).", 0 );
      return;
    }
    else if ( !strcmp( arg, "yes" ) ) { // he/she choosed to be a wild mage -> reckless dweomer added
      ch->pcdata->ability_info[gsn_reckless_dweomer].learned = 1; // 1%
      ch->pcdata->ability_info[gsn_reckless_dweomer].casting_level = 0; // no casting level
      ch->pcdata->ability_info[gsn_reckless_dweomer].level = 10; // will get at level 10
      ch->isWildMagic = TRUE;
      write_to_buffer( d, "You have choosen the way of the wild magic.\n\r", 0 );
    }
    else if ( !strcmp( arg, "no" ) ) { // ok, nothing to do: just go to alignment selection
      ch->isWildMagic = FALSE;
      write_to_buffer( d, "You have choosen the way of the non-wild magic.\n\r", 0 );
    }
    else {
      write_to_buffer( d, "Please, answer yes or no (help wild magic for more information).", 0 );
      return;
    }

    // Enter alignment choice
    write_to_buffer( d, "\n\r", 0 );

    for ( j = 1; j >= -1; j-- )
      for ( i = 1; i >= -1; i-- ){
	if ( check_etho_align( ch, i, j*350 ) ){
	  sprintf( buf,
		   "  {W[%s] %s{x\n\r",
		   short_etho_align[(i+1)+(j+1)*3],
		   etho_align_name( i, j*350 ) );
	  //write_to_buffer( d, buf, 0 );
	  send_to_desc( d, buf );
	}
      }
    write_to_buffer( d, "Which alignment (the 2 letters which are between [] or help)? ",0);
    d->connected = CON_GET_ALIGNMENT;
    break;

  case CON_GET_ALIGNMENT:
    // Modified by SinaC 2000, again in 2001
    /*
      ch->align.etho = 0;
      switch( argument[0]) {
      case 'g' : case 'G' : ch->align.alignment = 750;  break;
      case 'n' : case 'N' : ch->align.alignment = 0;    break;
      case 'e' : case 'E' : ch->align.alignment = -750; break;
      default:
      write_to_buffer(d,"That's not a valid alignment.\n\r",0);
      write_to_buffer(d,"Which alignment (G/N/E)? ",0);
      return;
      }
    */
    // Yeah, I know this is crappy code but ... see above  SinaC 2001

    // Added by SinaC 2001
    one_argument(argument,arg);
    if (!strcmp(arg,"help")) {
      argument = (char*) one_argument(argument,arg); // OK coz argument is discarded here
      if (argument[0] == '\0')
	do_help(ch,"alignment");
      else
	do_help(ch,argument);
      write_to_buffer( d, "Which alignment (the 2 letters which are between [] )? ",0);
      break;
    }

    bool found;

    found = FALSE;
    for ( i = 0; i < 9 && !found; i++ )
      // ok, we found the right etho/align
      if ( !str_cmp( argument, short_etho_align[i]) ){
	int etho = (i%3)-1;
	int align = (i/3)-1;
	
	if ( check_etho_align( ch, etho, align*350 ) ){
	  found = TRUE;
	  
	  // Modified by SinaC 2001 etho/alignment are attributes now
	  //ch->align.etho = etho;
	  //ch->align.alignment = align*750;
	  ch->bstat(etho) = etho;
	  ch->bstat(alignment) = align*750;
	}
      }
    
    // invalid etho/align
    if ( !found ) {
      write_to_buffer( d, "That's not a valid/allowed alignment.\n\r\n\r",0);
      for ( j = 1; j >= -1; j-- )
	for ( i = 1; i >= -1; i-- ){
	  if ( check_etho_align( ch, i, j*350 ) ){
	    sprintf( buf,
		     "  {W[%s] %s{x\n\r",
		     short_etho_align[(i+1)+(j+1)*3],
		     etho_align_name( i, j*350 ) );
	    //write_to_buffer( d, buf, 0 );
	    send_to_desc( d, buf );
	  }
	}
      write_to_buffer( d, "Which alignment (the 2 letters which are between [] )? ",0);
      return;
    }

    write_to_buffer(d,"\n\r",0);

    // hometown choice
    for ( i = 0; i < MAX_HOMETOWN; i++ )
      if ( check_hometown( ch, i ) ) {
	sprintf( buf, "  {y%-15s{x\n\r", hometown_table[i].name );
	//write_to_buffer( d, buf, 0 );
	send_to_desc( d, buf );
      }
    write_to_buffer(d,"What's your hometown ? ", 0 );
    d->connected = CON_GET_HOMETOWN;
    break;

    // SinaC 2003: hometown choice
  case CON_GET_HOMETOWN:
    home = hometown_lookup( argument );
    if ( home < 0 || home >= MAX_HOMETOWN || !check_hometown( ch, home ) ) {
      write_to_buffer(d,"That's not a valid/allowed hometown.\n\r\n\r",0);
      for ( i = 0; i < MAX_HOMETOWN; i++ )
	if ( check_hometown( ch, i ) ) {
	  sprintf( buf, "  {y%-15s{x\n\r", hometown_table[i].name );
	  //write_to_buffer( d, buf, 0 );
	  send_to_desc( d, buf );
	}
      write_to_buffer(d,"What's your hometown ? ", 0 );
      return;
    }

    ch->pcdata->hometown = home;

    write_to_buffer(d,"\n\r",0);
    for ( i = 0; i < MAX_GODS; i++ )
      if ( check_god( ch, i ) ){
	sprintf( buf, "  {c%s, {y%s{x\n\r", 
		 god_name( i ),
		 gods_table[i].title );
	//write_to_buffer( d, buf, 0 );
	send_to_desc( d, buf );
      }
    write_to_buffer( d, "Which God ? ", 0 );
    d->connected = CON_GET_GOD;
    break;
    
    // Added by SinaC 2001
  case CON_GET_GOD:
    // Added by SinaC 2004
    one_argument(argument,arg);
    if (!strcmp(arg,"help")) {
      argument = (char *) one_argument(argument,arg); // OK coz argument is discarded here
      if (argument[0] == '\0')
	do_help(ch,"gods");
      else
	do_help(ch,argument);
      write_to_buffer( d, "What IS your god (help <god name> for more information) ?",
		       0 );
      //break;
      return;
    }

    god = god_lookup( argument );
    if ( god < 0 || god >= MAX_GODS || !check_god( ch, god ) ){
      write_to_buffer(d,"That's not a valid/allowed god.\n\r\n\r",0);
      for ( i = 0; i < MAX_GODS; i++ )
	if ( check_god( ch, i ) ){
	  sprintf( buf, "  {c%s, {y%s{x\n\r", 
		   god_name( i ),
		   gods_table[i].title );
	  //write_to_buffer( d, buf, 0 );
	  send_to_desc( d, buf );
	}
      write_to_buffer(d,"What's your god ? ", 0);
      return;
    }
    ch->pcdata->god = god;
    write_to_buffer(d,"\n\r",0);
    
    // Added by SinaC 2003 for god sphere
    add_minor_sphere( ch, ch->pcdata->god );

    //group_add(ch,DEFAULT_GROUP_NAME"rom basics",FALSE);
    group_add(ch,DEFAULT_GROUP_NAME,FALSE);
    group_add(ch,class_table[class_firstclass(ch->bstat(classes))].base_group,FALSE);
    ch->pcdata->ability_info[gsn_recall].learned = 50;
    //ch->pcdata->hometown = hometown_lookup("midgaard");
    //ch->pcdata->hometown = DEFAULT_HOMETOWN;, hometown choice added before god choice
    // Added by SinaC 2001
    do_help(ch,"customize");
    write_to_buffer(d,"Do you wish to customize this character?\n\r",0);
    //write_to_buffer(d,"Customization takes time, but allows a wider range of skills and abilities.\n\r",0);
    write_to_buffer(d,"Customize (Y/N)? ",0);

    d->connected = CON_DEFAULT_CHOICE;
    break;
    
  case CON_DEFAULT_CHOICE:
    write_to_buffer(d,"\n\r",2);
    // Moved by SinaC 2001
    ch->gen_data = new_gen_data();
    switch ( argument[0] ) {
    case 'y': case 'Y':
      // Moved by SinaC 2001
      //ch->gen_data = new_gen_data();
      ch->gen_data->points_chosen = ch->pcdata->points;

      do_help(ch,"group header");
      list_group_costs(ch);
      write_to_buffer(d,"You already have the following abilities:\n\r",0);
      // removed by SinaC 2000, replaced by known command
      //do_skills(ch,"");
      list_group_known(ch);

      do_help(ch,"menu choice");
      d->connected = CON_GEN_GROUPS;
      break;
    case 'n': case 'N':
      group_add(ch,class_table[class_firstclass(ch->bstat(classes))].default_group,TRUE);

      /* Modified by Sinac 1997 for Monk class */
      /* re-removed by SinaC 2000
	 // un-removed by SinaC 2000  
	 *            if ( ch->bstat(classes) == (1<<CLASS_MONK ))
	 *              {
	 *                send_to_char( "No weapon choice as you are a monk.\n\r\n\r", ch );
	 *                send_to_char( "[Hit Return to continue]", ch );
	 *                d->connected = CON_PICK_WEAPON;
	 *                break;
	 *              }
	 // removed by SinaC 2000
      */
      write_to_buffer( d, "\n\r", 2 );
      write_to_buffer(d,
		      "Please pick a weapon from the following choices:\n\r",0);
      buf[0] = '\0';
      // Modified by SinaC 2001
      // Added by SinaC 2000
      //count = 0;
      ch->gen_data->count = 0;
      strcat( buf, "{W");
      for ( i = 0; weapon_table[i].name != NULL; i++)
	// Modified by SinaC 2000
	if (weapon_table[i].gsn != NULL
	    && ch->pcdata->ability_info[*weapon_table[i].gsn].learned > 0) {
	  // Modified by SinaC 2001
	  // Added by SinaC 2000
	  //count++;
	  ch->gen_data->count++;
	  strcat(buf,weapon_table[i].name);
	  strcat(buf,"  ");
	}
      // Modified by SinaC 2001
      // Added by SinaC 2000
      if ( /*count == 0*/ch->gen_data->count == 0 ){
	write_to_buffer(d,"No weapons choice.\n\r",0);
	write_to_buffer(d,"[Hit Return to continue]\n\r",0);
	d->connected = CON_PICK_WEAPON;
	break;
      }
      strcat( buf, "{x");

      strcat(buf,"\n\rYour choice? ");
      //write_to_buffer(d,buf,0);
      send_to_desc( d, buf );
      d->connected = CON_PICK_WEAPON;
      break;
    default:
      write_to_buffer( d, "Please answer (Y/N)? ", 0 );
      return;
    }
    break;

  case CON_PICK_WEAPON:

    /* Modified by Sinac 1997 for Monk class */
    /* re-removed by SinaC 2000
       // un-removed by SinaC 2000
       *      if ( ch->bstat(classes) == 1<<CLASS_MONK )
       *	{
       *	  ch->pcdata->learned[gsn_hand_to_hand] = 40;
       *	  ch->pcdata->learned[gsn_enhanced_hand] = 40;
       *	  do_help(ch,"motd");
       *	  d->connected = CON_READ_MOTD;
       *	  break;
       *	}
       // removed by SinaC 2000
    */
    // Modified by SinaC 2001
    // Added by SinaC 2000
    if ( /*count == 0*/ch->gen_data->count == 0 ){
      // Modified by SinaC 2000
      ch->pcdata->ability_info[gsn_hand_to_hand].learned = 40;
      // Removed by SinaC 2001
      //ch->pcdata->ability_info[gsn_enhanced_hand].learned = 40;
      // Added by SinaC 2001 for rebirth
      if ( rebirth || remort )
	write_to_buffer(d,"[Hit Return to continue]", 0 );
      else
	do_help(ch,"motd");
      d->connected = CON_READ_MOTD;
      break;
    }
      
    write_to_buffer(d,"\n\r",2);
    weapon = weapon_lookup(argument);
    if (weapon == -1
	|| weapon_table[weapon].gsn == NULL
	|| ch->pcdata->ability_info[*weapon_table[weapon].gsn].learned <= 0) {
      write_to_buffer(d,
		      "That's not a valid selection. Choices are:\n\r",0);
      buf[0] = '\0';
      strcat( buf, "{W");
      for ( i = 0; weapon_table[i].name != NULL; i++)
	if ( weapon_table[i].gsn != NULL
	     && ch->pcdata->ability_info[*weapon_table[i].gsn].learned > 0) {
	  strcat(buf,weapon_table[i].name);
	  strcat(buf," ");
	}
      strcat( buf, "{x");
      strcat(buf,"\n\rYour choice? ");
      //write_to_buffer(d,buf,0);
      send_to_desc( d, buf );
      return;
    }
      
    ch->pcdata->ability_info[*weapon_table[weapon].gsn].learned = 40;

    write_to_buffer(d,"\n\r",2);

    // Added by SinaC 2001 for color greet
    if (ch->desc->ansi)
      SET_BIT(ch->act, PLR_COLOUR);
    else 
      REMOVE_BIT(ch->act, PLR_COLOUR);
    
    // Added by SinaC 2001 for rebirth
    if ( rebirth || remort )
      write_to_buffer(d,"[Hit Return to continue]", 0 );
    else
      do_help(ch,"motd");
    d->connected = CON_READ_MOTD;
    break;
      
  case CON_GEN_GROUPS:
    send_to_char("\n\r",ch);
    if (!str_cmp(argument,"done")) {
      sprintf(buf,"{WCreation points: {c%d{x\n\r",ch->pcdata->points);
      send_to_char(buf,ch);
      sprintf(buf,"{WExperience per level: {c%d{x\n\r",
	      exp_per_level(ch,ch->gen_data->points_chosen));
      // Moved by SinaC 2001
      //free_gen_data(ch->gen_data);
      send_to_char(buf,ch);

      /* Modified by Sinac 1997 for Monk class */
      /*
	// un-removed by SinaC 2000
	*            if ( ch->bstat(classes) == 1<<CLASS_MONK )
	*              {
	*                send_to_char( "\n\r\n\rNo weapon choice as you are a monk.\n\r\n\r", ch );
	*                send_to_char( "[Hit Return to continue]", ch );
	*                d->connected = CON_PICK_WEAPON;
	*                break;
	*              }
	// removed by SinaC 2000
      */

      write_to_buffer( d, "\n\r", 2 );
      write_to_buffer(d,
		      "Please pick a weapon from the following choices:\n\r",0);
      buf[0] = '\0';
      // Modified by SinaC 2001
      // Added by SinaC 2000
      //count = 0;
      ch->gen_data->count = 0;
      for ( i = 0; weapon_table[i].name != NULL; i++)
	if ( weapon_table[i].gsn != NULL
	     && ch->pcdata->ability_info[*weapon_table[i].gsn].learned > 0) {
	  // Modified by SinaC 2001
	  // Added by SinaC 2000
	  //count++;
	  ch->gen_data->count++;
	  strcat(buf,weapon_table[i].name);
	  strcat(buf," ");
	}
      // Modified by SinaC 2001
      // Added by SinaC 2000
      if ( /*count == 0*/ch->gen_data->count == 0 ){
	write_to_buffer(d,"No weapons choice.\n\r",0);
	write_to_buffer(d,"[Hit Return to continue]\n\r",0);
	d->connected = CON_PICK_WEAPON;
	break;
      }
	    
      strcat(buf,"\n\rYour choice? ");
      write_to_buffer(d,buf,0);
      d->connected = CON_PICK_WEAPON;
      break;
    }

    if (!parse_gen_groups(ch,argument))
      send_to_char( "Choices are: list,spell,power,learned,known,premise,add,drop,info,help and done.\n\r" ,ch);
	
    do_help(ch,"menu choice");
    break;

  case CON_READ_IMOTD:

    write_to_buffer(d,"\n\r",2);
    do_help( ch, "motd" );
    d->connected = CON_READ_MOTD;
    break;

  case CON_READ_MOTD:
    if ( ch->pcdata == NULL || ch->pcdata->pwd[0] == '\0') {
      /*
      write_to_buffer( d, "************ Warning! Null password!                     ************\n\r",0 );
      write_to_buffer( d, "************ Type 'password null <new password>' to fix. ************\n\r",0);*/
    // Modified by SinaC 2001
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R******* {YWarning! Null password!{R                     ********{x\n\r");
      send_to_desc( d, "{R******* {YType 'password null <new password>' to fix.{R ********{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");
      send_to_desc( d, "{R************************************************************{x\n\r");

      bug("Null password for [%s]", NAME(ch) );
    }

    // Added by SinaC 2001 for rebirth
    if ( !rebirth && !remort )
      write_to_buffer( d,
		       "\n\rWelcome to ROM 2.4.  Please do not feed the mobiles.\n\r",
		       0 );

    // Insert player in char_list
    ch->next	= char_list;
    char_list	= ch;
    d->connected	= CON_PLAYING;
	
    recompute(ch); // NEEDED because we use cstat if level == 0
    
    if ( ch->level == 0 ) {

      // Modified by SinaC 2001, was +3 before
      // Modified SinaC 2000  was currattr[...] before
      ch->baseattr[ATTR_stat0+class_table[class_firstclass(ch->bstat(classes))].attr_prime] += 2;
	
      // Moved by SinaC 2001
      ch->level	= 1;

      ch->exp	= exp_per_level(ch,ch->pcdata->points);
      ch->hit	= ch->cstat(max_hit);
      ch->mana	= ch->cstat(max_mana);
      // Added by SinaC 2001 for mental user
      ch->psp	= ch->cstat(max_psp);
      ch->move	= ch->cstat(max_move);
      if (ch->pcdata->points < 40)
	ch->train = UMAX( (40 - ch->pcdata->points + 1) / 2, 5 );
      else
	ch->train = 5;
      ch->practice = 10;
      sprintf( buf, "the %s",
	       title_table [class_firstclass(ch->cstat(classes))] [ch->level]
	       [ch->cstat(sex) == SEX_FEMALE ? 1 : 0] );
      set_title( ch, buf );

      //      do_outfit(ch,"");
      //      obj_to_char(create_object(get_obj_index(OBJ_VNUM_MAP),0),ch);
      give_creation_items(ch);

      // Modified by SinaC 2001 for rebirth
      if ( rebirth ) {
	send_to_char("\n\r\n\r{RRebirth completed.{x\n\r\n\r",ch);
	wiznet("$N ends rebirth.", ch, NULL, WIZ_REBIRTH, 0, 0 );
      }
      else if ( remort ) {
	send_to_char("\n\r\n\r{RRemort completed.{x\n\r\n\r",ch);
	wiznet("$N ends remort.", ch, NULL, WIZ_REBIRTH, 0, 0 );
      }
      else
	do_help(ch,"NEWBIE");
      send_to_char("\n\r",ch);

      // Added by SinaC 2001 for rebirth
      if ( rebirth || remort && ch->pcdata->tmpRace < 0 )
	char_from_room( ch );

      // Added by SinaC 2001 for racial language
      ch->pcdata->language = pc_race_table[ch->bstat(race)].language;

      //char_to_room( ch, get_room_index( ROOM_VNUM_SCHOOL ) );
      char_to_room( ch, get_school_room( ch ) ); // SinaC 2003
      send_to_char("\n\r",ch);

      // Modified by SinaC 2001 for rebirth
      if ( !rebirth && !remort ) {
	// Added by SinaC 2000
	do_autotitle( ch, "" );
	do_autoexit( ch, "" );
	send_to_char("\n\r",ch);
      }

      ch->pcdata->board = &boards[WELCOME_BOARD];

      // Added by SinaC 2001 for rebirth
      ch->pcdata->tmpRace = -1; // erase tmp race
      if ( rebirth || remort )
	//save_char_obj( ch );
	new_save_pFile(ch,FALSE);
	    
      // Added by SinaC 2001 for rebirth
      if ( !rebirth && !remort )
      /* By Oxtal on player request */
      {
	DESCRIPTOR_DATA *d2;

	sprintf(buf,"\n\r\n\r{Y %s is a new player in Mystery.\n\r A welcome would be appreciated.\n\r{x",ch->name);
        
	for ( d2 = descriptor_list; d2; d2 = d2->next )
	  if ( d2->connected == CON_PLAYING && d2 != d)
	    send_to_char( buf, d2->character );           
      }
    }
    else if ( ch->in_room != NULL ) {
      // Added by SinaC 2000
      // if we connect in a room of the battle or waiting room ==> temple
      // Modified by SinaC 2001, if rebirth ==> temple
      if ( IN_BATTLE( ch ) || IN_WAITING( ch ) ) {
	//char_to_room( ch, get_room_index( ROOM_VNUM_TEMPLE ) );
	char_to_room( ch, get_recall_room(ch)); // SinaC 2003
      }
      else {
	char_to_room( ch, ch->in_room );
      }
    }
    else if ( IS_IMMORTAL(ch) ) {
      char_to_room( ch, get_room_index( ROOM_VNUM_CHAT ) );
    }
    else {
      //char_to_room( ch, get_room_index(hometown_table[ch->pcdata->hometown].recall) );
      char_to_room( ch, get_recall_room(ch) ); // SinaC 2003
    }

    // Added by SinaC 2003

    if ( !rebirth && !remort ) {
      // Added by SinaC 2003
      ch->clazz = default_player_class;
      MOBPROG( ch, NULL, "onLoad" );
      act( "$n has entered the game.", ch, NULL, NULL, TO_ROOM );
      do_look( ch, "auto" );

      wiznet("$N has left real life behind.",ch,NULL,
	     WIZ_LOGINS,WIZ_SITES,get_trust(ch));
      
      if (ch->pet != NULL) {
	//char_to_room(ch->pet,ch->in_room);
	CHAR_DATA *previous = ch;
	CHAR_DATA *pet = ch->pet;
	while ( pet != NULL ) { // modify by SinaC 2003: pet having pet, ...
	  if ( pet->in_room == NULL )
	    char_to_room(pet,previous->in_room);
	  else
	    char_to_room(pet, pet->in_room ); // SinaC 2003
	  previous = pet;
	  pet = pet->pet;
	}
	act("$n has entered the game.",ch->pet,NULL,NULL,TO_ROOM);
      }

      // Warns incoming player that double xp is on
      if ( double_xp == TRUE )
	send_to_char("\n\r{R    ==DOUBLE XP IS ON=={x\n\r\n\r", ch );
      
      do_board(ch,"");
    }
    /*Oxtal*/ 
    fix_clan_status_enter(ch->name);
    break;

    /* states for new note system, (c)1995-96 erwin@pip.dknet.dk */
    /* ch MUST be PC here; have nwrite check for PC status! */
		
  case CON_NOTE_TO:
    handle_con_note_to (d, argument);
    break;
		
  case CON_NOTE_SUBJECT:
    handle_con_note_subject (d, argument);
    break;
	
  case CON_NOTE_EXPIRE:
    handle_con_note_expire (d, argument);
    break;

  case CON_NOTE_TEXT:
    handle_con_note_text (d, argument);
    break;
		
  case CON_NOTE_FINISH:
    handle_con_note_finish (d, argument);
    break;

    // Added by SinaC 2001 for rebirth race
  case CON_REBIRTH:
    
    log_stringf("REBIRTH: %s", NAME(d->character));
    wiznet("$N starts rebirth.", ch, NULL, WIZ_REBIRTH, 0, 0 );
    create_rebirth(d);
    if ( IS_SET( ch->act, PLR_COLOUR ) )
      ch->desc->ansi = TRUE;
    else
      ch->desc->ansi = FALSE;

    break;

  case CON_REMORT:

    log_stringf("REMORT: %s", NAME(d->character));
    wiznet("$N starts remort.", ch, NULL, WIZ_REBIRTH, 0, 0 );
    create_remort(d);
    if ( IS_SET( ch->act, PLR_COLOUR ) )
      ch->desc->ansi = TRUE;
    else
      ch->desc->ansi = FALSE;
    break;
  }

  return;
}

// Used to rebirth a player, called from nanny: CON_REBIRTH
int get_rebirth_race( int playerRace ) {
  if ( race_table[ playerRace ].pc_race // player race has a rebirth list: race + proba
       && pc_race_table[ playerRace ].nb_rebirth > 0 ) {
    int n = number_percent();
    int add = 0;
    int race = pc_race_table[ playerRace ].rebirth_list[0];
    for ( int i = 0; i < pc_race_table[ playerRace ].nb_rebirth; i++ )
      if ( add <= n
	   && n <= add+pc_race_table[ playerRace ].rebirth_probability[i] ) {
	race = pc_race_table[ playerRace ].rebirth_list[i];
	add += pc_race_table[ playerRace ].rebirth_probability[i];
	break;
      }
    return race;
  }
  else { // player race doesn't have rebirth_list so, get a race at random
    int rebirth_list[MAX_PC_RACE];
    int number_rebirth = 0;
    for ( int i = 0; i < MAX_PC_RACE; i++ )
      if ( pc_race_table[i].type == RACE_REBIRTH )
	rebirth_list[number_rebirth++] = i;
    if ( number_rebirth == 0 ) { // have a problem, no rebirth race available, assuming human
      bug("REBIRTH: no rebirth race available, assuming human");
      return DEFAULT_PC_RACE;
    }
    int n = URANGE( 0, number_range( 0, number_rebirth ), number_rebirth-1 );
    return rebirth_list[n];
  }
  return DEFAULT_PC_RACE;
  //return race_lookup("ghost");
}
void create_rebirth( DESCRIPTOR_DATA *d ) {
  CHAR_DATA *ch = d->character;
  CHAR_DATA *wch;
  OBJ_DATA *obj, *obj_next;
  AFFECT_DATA *paf, *paf_next;

  send_to_char("{RRebirth started.{x\n\r",ch);
  send_to_char("{RPlease Wait...{x\n\r\n\r", ch );

  // Save pFile so players can leave during rebirth creation, when they reconnect they will
  //  restart rebirth with the same rebirth race
  int race;
  if ( ch->pcdata->tmpRace < 0 ) { // normal rebirth
    // Nuke pets and charmies
    die_follower( ch );
    
    // Teleport the player to a special room
    char_from_room( ch );
    char_to_room( ch, get_room_index( ROOM_VNUM_REBIRTH ) );

    // Choose a random race
    race = get_rebirth_race( ch->bstat(race ) );
    ch->pcdata->tmpRace = race;
    new_save_pFile( ch, TRUE );

    // Remove the player from the char_list
    // before    -> prec -> ch -> next ->
    // after     -> prec -> next ->           and  ch is not in the list
    
    // Is ch the only one char ?
    if ( ch == char_list && ch->next == NULL ) {
      bug("REBIRTH: ch (%s) is the only char!!!!", NAME(ch));
      char_list = NULL;
    }
    // there is other char
    else
      // ch is the first ?
      if ( ch == char_list )
	char_list = ch->next;
    // ch is not the first
      else {
	for ( wch = char_list; wch != NULL; wch = wch->next )
	  if ( wch->next == ch ) 
	    break;
	if ( wch == NULL ) {
	  bug("REBIRTH: wch is null!!!!");
	  d->connected = CON_PLAYING;
	  return;
	}
	wch->next = ch->next;
	ch->next = NULL;
      }
  }
  else // reconnect rebirth
    race = ch->pcdata->tmpRace;
  
  // Remove every skills, trains, practices, qp, trivia the player could have
  // Set hp, mana, move, silver, gold, .... to inital
  if ( is_clan(ch) ) {
    remove_clan_member(&get_clan_table(ch->clan)->members,ch->name);
    ch->clan = 0;
    ch->clan_status = 0;
  }
  ch->level = 0;
  ch->trust = 0;

  ch->isWildMagic = FALSE; // SinaC 2003

  ch->cstat(max_hit) = ch->bstat(max_hit) = 20;
  ch->cstat(max_mana) = ch->bstat(max_mana) = 100;
  // Added by SinaC 2001 for mental user
  ch->cstat(max_psp) = ch->bstat(max_psp) = 100;
  ch->cstat(max_move) = ch->bstat(max_move) = 100;

  ch->played /= 2; // divide playtime by 2
  ch->logon = current_time;
  ch->timer = 0;
  
  ch->wait = ch->daze = 0;
  ch->hit = 20;
  ch->mana = 100;
  // Added by SinaC 2001 for mental user
  ch->psp = 100;
  ch->move = 100;
  ch->silver = ch->gold = 0;
  ch->exp = 0;
  ch->position = POS_STANDING;
  ch->wimpy = 0;

  ch->pcdata->betted_on = NULL;
  ch->pcdata->bet_amt = 0;
  ch->pcdata->challenged = NULL;
  
  ch->stunned = 0;
  
  ch->pcdata->last_level = 0;
  /*
  time_t                last_changes;
  */
  ch->pcdata->condition[COND_HUNGER] = 10;
  ch->pcdata->condition[COND_THIRST] = 10;
  ch->pcdata->condition[COND_FULL] = 0;
  ch->pcdata->condition[COND_DRUNK] = 0;
  
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ch->pcdata->ability_info[i].learned = 0;
    ch->pcdata->ability_info[i].casting_level = 0;
    ch->pcdata->ability_info[i].level = 0;
  }
  
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    ch->pcdata->group_known[i] = FALSE;
  }
  ch->pcdata->confirm_delete = FALSE;

  ch->pcdata->trivia = 0;
// Removed by SinaC 2001, use extra fields now
//  ch->pcdata->acctgold = ch->pcdata->acctsilver = 0;

//  ch->pcdata->questgiver = NULL;
//  ch->pcdata->questpoints =
//    ch->pcdata->nextquest =
//    ch->pcdata->countdown =
//    ch->pcdata->questobj =
//    ch->pcdata->questmob = 
//    ch->pcdata->questobjloc = 0;

//  ch->pcdata->hometown = 2;, SinaC 2003

  for ( int i = 0; i < MAX_BEACONS; i++ ) 
    ch->pcdata->beacons[i] = 0;

  ch->pcdata->name_accepted = TRUE;
  // Added by SinaC 2001
  //ch->pcdata->branded = FALSE;   removed by SinaC 2003

  // Destroy all the equipement, inventory and affect
  for (obj = ch->carrying; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    extract_obj(obj);
  }

  for (paf = ch->affected; paf != NULL; paf = paf_next) {
    paf_next = paf->next;
    affect_remove(ch,paf);
  }

  // nuke notes
  ch->pcdata->in_progress = NULL;

  // Modified by SinaC 2003
  // New rebirth race
  /* Aarghhh, that was awful
  *int begin_race = race_lookup( "ghost" );
  *int end_race = race_lookup( "banshee");
  *if ( begin_race < 0  || end_race < 0 ) {
  *  bug("ghost or banshee race NOT found");
  *  race = 0;
  *}
  *else
  *  race = number_range( begin_race, end_race );
  */

  log_stringf("REBIRTH: ch: %s  race: %s", NAME(ch), race_table[race].name );

  send_to_char("{RRace set.{x\n\r\n\r",ch );
  ch->bstat(race) = race;
  // initialize stats
  for ( int i = 0; i < MAX_STATS; i++)
    ch->bstat(stat0+i) = pc_race_table[race].stats[i];
  ch->bstat(affected_by) = ch->bstat(affected_by)|race_table[race].aff;
  // Added by SinaC 2001
  ch->bstat(affected2_by) = ch->bstat(affected2_by)|race_table[race].aff2;
  ch->bstat(imm_flags)	= ch->bstat(imm_flags)|race_table[race].imm;
  ch->bstat(res_flags)	= ch->bstat(res_flags)|race_table[race].res;
  ch->bstat(vuln_flags)	= ch->bstat(vuln_flags)|race_table[race].vuln;
  ch->bstat(form)	= race_table[race].form;
  ch->bstat(parts)	= race_table[race].parts;
  
  if (IS_SET(PART_CLAWS,ch->bstat(parts)))
    ch->bstat(dam_type) = 5; /* claws */
  else
    ch->bstat(dam_type) = 17; /*punch */

  // add cost
  // Modified by SinaC 2001
  //ch->pcdata->points = pc_race_table[race].points;
  ch->pcdata->points = 0;
  //ch->bstat(size) = pc_race_table[race].size;
  ch->bstat(size) = race_table[race].size;

  // jumps at sex selection
  write_to_buffer( d, "What is your sex (Male/Female)? ", 0 );
  d->connected = CON_GET_NEW_SEX;
}

// Used to remort a player, called from nanny: CON_REBIRTH
void create_remort( DESCRIPTOR_DATA *d ) {
  int race;
  CHAR_DATA *ch = d->character;
  CHAR_DATA *wch;
  OBJ_DATA *obj, *obj_next;
  AFFECT_DATA *paf, *paf_next;

  send_to_char("{RRemort started.{x\n\r",ch);
  send_to_char("{RPlease Wait...{x\n\r\n\r", ch );

  // Nuke pets and charmies
  die_follower( ch );
  
  // Teleport the player to a special room
  char_from_room( ch );
  char_to_room( ch, get_room_index( ROOM_VNUM_REMORT ) );
  
  // Remove the player from the char_list
  // before    -> prec -> ch -> next ->
  // after     -> prec -> next ->           and  ch is not in the list

  // Is ch the only one char ?
  if ( ch == char_list && ch->next == NULL ) {
    bug("REMORT: ch (%s) is the only char!!!!", NAME(ch));
    char_list = NULL;
  }
  // there is other char
  else
    // ch is the first ?
    if ( ch == char_list )
      char_list = ch->next;
  // ch is not the first
    else {
      for ( wch = char_list; wch != NULL; wch = wch->next )
	if ( wch->next == ch ) 
	  break;
      if ( wch == NULL ) {
	bug("REMORT: wch is null!!!!");
	d->connected = CON_PLAYING;
	return;
      }
      wch->next = ch->next;
      ch->next = NULL;
    }

  // Remove every skills, trains, practices, qp, trivia the player could have
  // Set hp, mana, move, silver, gold, .... to inital
  if ( is_clan(ch) ) {
    remove_clan_member(&get_clan_table(ch->clan)->members,ch->name);
    ch->clan = 0;
    ch->clan_status = 0;
  }
  ch->level = 0;
  ch->trust = 0;

  ch->isWildMagic = FALSE; // SinaC 2003

  ch->cstat(max_hit) = ch->bstat(max_hit) = 20;
  ch->cstat(max_mana) = ch->bstat(max_mana) = 100;
  // Added by SinaC 2001 for mental user
  ch->cstat(max_psp) = ch->bstat(max_psp) = 100;
  ch->cstat(max_move) = ch->bstat(max_move) = 100;

  ch->played /= 2; // divide playtime by 2
  ch->logon = current_time;
  ch->timer = 0;
  
  ch->wait = ch->daze = 0;
  ch->hit = 20;
  ch->mana = 100;
  // Added by SinaC 2001 for mental user
  ch->psp = 100;
  ch->move = 100;
  ch->silver = ch->gold = 0;
  ch->exp = 0;
  ch->position = POS_STANDING;
  ch->wimpy = 0;

  ch->pcdata->betted_on = NULL;
  ch->pcdata->bet_amt = 0;
  ch->pcdata->challenged = NULL;
  
  ch->stunned = 0;
  
  ch->pcdata->last_level = 0;
  /*
  time_t                last_changes;
  */
  ch->pcdata->condition[COND_HUNGER] = 10;
  ch->pcdata->condition[COND_THIRST] = 10;
  ch->pcdata->condition[COND_FULL] = 0;
  ch->pcdata->condition[COND_DRUNK] = 0;
  
  for ( int i = 0; i < MAX_ABILITY; i++ ) {
    ch->pcdata->ability_info[i].learned = 0;
    ch->pcdata->ability_info[i].casting_level = 0;
    ch->pcdata->ability_info[i].level = 0;
  }
  
  for ( int i = 0; i < MAX_GROUP; i++ ) {
    ch->pcdata->group_known[i] = FALSE;
  }
  ch->pcdata->confirm_delete = FALSE;

  ch->pcdata->trivia = 0;
// Removed by SinaC 2001, use extra fields now
//  ch->pcdata->acctgold = ch->pcdata->acctsilver = 0;

//  ch->pcdata->questgiver = NULL;
//  ch->pcdata->questpoints =
//    ch->pcdata->nextquest =
//    ch->pcdata->countdown =
//    ch->pcdata->questobj =
//    ch->pcdata->questmob = 
//    ch->pcdata->questobjloc = 0;

//  ch->pcdata->hometown = 2;  SinaC 2003

  for ( int i = 0; i < MAX_BEACONS; i++ ) 
    ch->pcdata->beacons[i] = 0;

  ch->pcdata->name_accepted = TRUE;
  // Added by SinaC 2001
  //ch->pcdata->branded = FALSE;   removed by SinaC 2003

  // Destroy all the equipement, inventory and affect
  for (obj = ch->carrying; obj != NULL; obj = obj_next) {
    obj_next = obj->next_content;
    extract_obj(obj);
  }

  for (paf = ch->affected; paf != NULL; paf = paf_next) {
    paf_next = paf->next;
    affect_remove(ch,paf);
  }

  // nuke notes
  ch->pcdata->in_progress = NULL;

  race = ch->bstat(race); // race has already been set in do_remort
  log_stringf("REMORT: ch: %s as race: %s", NAME(ch), race_table[race].name );

  send_to_char("{RRace set.{x\n\r\n\r",ch );
  // initialize stats
  for ( int i = 0; i < MAX_STATS; i++)
    ch->bstat(stat0+i) = pc_race_table[race].stats[i];
  ch->bstat(affected_by) = ch->bstat(affected_by)|race_table[race].aff;
  // Added by SinaC 2001
  ch->bstat(affected2_by) = ch->bstat(affected2_by)|race_table[race].aff2;
  ch->bstat(imm_flags)	= ch->bstat(imm_flags)|race_table[race].imm;
  ch->bstat(res_flags)	= ch->bstat(res_flags)|race_table[race].res;
  ch->bstat(vuln_flags)	= ch->bstat(vuln_flags)|race_table[race].vuln;
  ch->bstat(form)	= race_table[race].form;
  ch->bstat(parts)	= race_table[race].parts;
  
  if (IS_SET(PART_CLAWS,ch->bstat(parts)))
    ch->bstat(dam_type) = 5; /* claws */
  else
    ch->bstat(dam_type) = 17; /*punch */

  // add cost
  // Modified by SinaC 2001
  //ch->pcdata->points = pc_race_table[race].points;
  ch->pcdata->points = 0;
  //ch->bstat(size) = pc_race_table[race].size;
  ch->bstat(size) = race_table[race].size;

  // jumps at sex selection
  write_to_buffer( d, "What is your sex (Male/Female)? ", 0 );
  d->connected = CON_GET_NEW_SEX;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name( const char *name ) {
  /*
   * Reserved words.
     */
/* Replaced with a list of reserved/forbidden words
  if ( is_name( name,
		"all auto immortal self someone something the you demise balance circle loner honor") )
		return FALSE;
*/
  // SinaC 2003
  for ( int i = 0; i < MAX_FORBIDDEN_NAME; i++ )
    if ( !str_prefix( name, forbidden_name[i] ) )
      return FALSE;

  // SinaC 2003
  // Can't name themself as a class/race/god/abilities
  if ( class_lookup( name ) >= 0 )
    return FALSE;
  if ( race_lookup( name ) >= 0 )
    return FALSE;
  if ( god_lookup( name ) >= 0 )
    return FALSE;
  if ( ability_lookup( name ) >= 0 )
    return FALSE;

  //  char buf[MAX_STRING_LENGTH];
  // can't have same name as a god
  //  buf[0] = '\0';
  //  for ( int i = 0; i < MAX_GODS; i++ ) {
  //    strcat( buf, gods_table[i].name );
    //    strcat( buf, " " );
  //  }
  //  if ( is_name( name, buf ) )
  //    return FALSE;
  // can't have same name as a class
  //  for ( int i = 0; i < MAX_CLASS; i++ )
  //    strcat( buf, class_table[i].name );
  //    strcat( buf, " " );
  //  }
  //  if ( is_name( name, buf ) )
  //    return FALSE;
  // can't have same name as a race
  //  buf[0] = '\0';
  //  for ( int i = 0; i < MAX_RACE; i++ ) {
  //    strcat( buf, race_table[i].name );
  //    strcat( buf, " " );
  //  }
  //  if ( is_name( name, buf ) )
  //    return FALSE;
  // can't have same name as a ability
  //  buf[0] = '\0';
  //  for ( int i = 0; i < MAX_RACE; i++ ) {
  //    strcat( buf, race_table[i].name );
  //    strcat( buf, " " );
  //  }
  //  if ( is_name( name, buf ) )
  //    return FALSE;

/* Removed by SinaC 2001
  if (str_cmp(capitalize(name),"Alander") && (!str_prefix("Alan",name)
					      || !str_suffix("Alander",name)))
    return FALSE;
*/
    /*
     * Length restrictions.
     */
  if ( strlen(name) <  2 )
    return FALSE;

#if defined(MSDOS)
  if ( strlen(name) >  8 )
    return FALSE;
#endif

#if defined(macintosh) || defined(unix)
  if ( strlen(name) > 12 )
    return FALSE;
#endif

  /*
     * Alphanumerics only.
     * Lock out IllIll twits.
     */
  {
    const char *pc;
	bool fIll,adjcaps = FALSE,cleancaps = FALSE;
 	int total_caps = 0;

	fIll = TRUE;
	for ( pc = name; *pc != '\0'; pc++ ) {
	    if ( !isalpha(*pc) )
		return FALSE;

	    if ( isupper(*pc)) {/* ugly anti-caps hack */
		if (adjcaps)
		    cleancaps = TRUE;
		total_caps++;
		adjcaps = TRUE;
	    }
	    else
		adjcaps = FALSE;

	    if ( LOWER(*pc) != 'i' && LOWER(*pc) != 'l' )
		fIll = FALSE;
	}

	if ( fIll )
	    return FALSE;

	if (cleancaps || (total_caps > (int)(strlen(name)) / 2 && strlen(name) < 3))
	    return FALSE;
    }

    /*
     * Prevent players from naming themselves after mobs.
     */
    {
	MOB_INDEX_DATA *pMobIndex;
	int iHash;

	for ( iHash = 0; iHash < MAX_KEY_HASH; iHash++ ) {
	    for ( pMobIndex  = mob_index_hash[iHash];
		  pMobIndex != NULL;
		  pMobIndex  = pMobIndex->next ) {
		if ( is_name( name, pMobIndex->player_name ) )
		    return FALSE;
	    }
	}
    }

  // Added by SinaC 2003
  /*
   * Prevent players from naming themselves after same name as a player creating his/her char
   */
  DESCRIPTOR_DATA *dold;
  for ( dold = descriptor_list; dold; dold = dold->next ) {
    if ( dold->character != NULL
	 && dold->connected != CON_PLAYING
	 && is_name( name,  dold->original
		     ? dold->original->name : dold->character->name ) )
      return FALSE;
  }
  
  return TRUE;
}



/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect( DESCRIPTOR_DATA *d, const char *name, bool fConn )
{
    CHAR_DATA *ch;

    for ( ch = char_list; ch != NULL; ch = ch->next ) {
	if ( !IS_NPC(ch)
	&&   (!fConn || ch->desc == NULL)
	&&   !str_cmp( d->character->name, ch->name ) ) {
	    if ( fConn == FALSE ) {
		d->character->pcdata->pwd = str_dup( ch->pcdata->pwd );
	    }
	    else {
		free_char( d->character );
		d->character = ch;
		ch->desc	 = d;
		ch->timer	 = 0;
		if ( ch->desc->pEdit != NULL ) // Inform character of a build/edit in progress
		  send_to_char("You were editing/building. Type show to continue.\n\r", ch );
		else { // remove EDITING and BUILDING bit to be sure
		  REMOVE_BIT(ch->comm,COMM_EDITING);
		  REMOVE_BIT(ch->comm,COMM_BUILDING);
		}
		send_to_char(
		    "Reconnecting. Type replay to see missed tells.\n\r", ch );
		act( "$n has reconnected.", ch, NULL, NULL, TO_ROOM );

		sprintf( log_buf, "%s@%s reconnected.", ch->name, d->host );
		log_string( log_buf );
		wiznet("$N groks the fullness of $S link.",
		    ch,NULL,WIZ_LINKS,0,0);
		d->connected = CON_PLAYING;

 		/* Inform the character of a note in progress and the possibility of continuation! */
 		if (ch->pcdata->in_progress)
		  send_to_char ("You have a note in progress. Type NWRITE to continue it.\n\r",ch);
		
	    }
	    return TRUE;
	}
    }

    return FALSE;
}



/*
 * Check if already playing.
 */
bool check_playing( DESCRIPTOR_DATA *d, const char *name )
{
    DESCRIPTOR_DATA *dold;

    for ( dold = descriptor_list; dold; dold = dold->next ) {
      if ( dold != d
	   &&   dold->character != NULL
	   &&   dold->connected != CON_GET_NAME
	   &&   dold->connected != CON_GET_OLD_PASSWORD
	   &&   !str_cmp( name, dold->original
			  ? dold->original->name : dold->character->name ) ) {
	    write_to_buffer( d, "That character is already playing.\n\r",0);
	    write_to_buffer( d, "Do you wish to connect anyway (Y/N)?",0);
	    d->connected = CON_BREAK_CONNECT;
	    return TRUE;
	}
    }

    return FALSE;
}



void stop_idling( CHAR_DATA *ch )
{
    if ( ch == NULL
	 ||   ch->desc == NULL
	 ||   ch->desc->connected != CON_PLAYING
	 ||   ch->was_in_room == NULL 
	 ||   ch->in_room != get_room_index(ROOM_VNUM_LIMBO))
	return;

    ch->timer = 0;
    char_from_room( ch );
    char_to_room( ch, ch->was_in_room );
    ch->was_in_room	= NULL;
    act( "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
    return;
}


/*
 * Write to one char, new colour version, by Lope.
 */

void send_to_char( const char *txt, CHAR_DATA *ch ) {
  if ( ch == NULL || !ch->valid )
    return;
  // Added by SinaC 2000 for some test, charmies and pet sends to master
  if ( TESTING_CHARMIES ) {
    if ( IS_NPC(ch) && ch->master != NULL && IS_IMMORTAL(ch->master)) {
      char buf[MAX_INPUT_LENGTH];
      
      sprintf(buf,"{R[%s:]{x %s\n\r",ch->short_descr,txt);
      send_to_char(buf,ch->master);
      return;
    }
  }
  if ( DUMP_MOB_MSG ) {
    if ( IS_NPC(ch) )
      log_stringf("DUMP MOB [%s (%d)]: %s", 
		  ch->short_descr,
		  ch->pIndexData->vnum,
		  txt );
  }
  /* Test and redirect to write, simply -- Oxtal */
  if ( ch->desc
       && ( ch->desc->connected < CON_NOTE_TO
	    || ch->desc->connected > CON_NOTE_FINISH ) )  
    write_to_char(ch->desc,txt);
  /* So people see nothing when writing a note. */
}

/*
 * Write to one char.  
 */
void send_to_char_bw( const char *txt, CHAR_DATA *ch )
{
  if ( txt != NULL && ch->desc != NULL 
       && (ch->desc->connected < CON_NOTE_TO || ch->desc->connected > CON_NOTE_FINISH) )
    write_to_buffer( ch->desc, txt, strlen(txt) );
  return;
}

// Added by SinaC 2001 for color greet
void send_to_desc(DESCRIPTOR_DATA * desc, const char *txt )
{
  const	char 	*point;
  char 	*point2;
  char 	buf[ MAX_STRING_LENGTH*4 ];
  int	skip = 0;

  buf[0] = '\0';
  point2 = buf;
  if ( txt && desc ) {
    if ( desc->ansi == TRUE ) {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  skip = colour( *point, desc->character, point2 );
	  while( skip-- > 0 )
	    ++point2;
	  continue;
	}
	*point2 = *point;
	*++point2 = '\0';
      }			
      *point2 = '\0';
      write_to_buffer( desc, buf, point2 - buf );
    }
    else {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  continue;
	}
	*point2 = *point;
	*++point2 = '\0';
      }
      *point2 = '\0';
      write_to_buffer( desc, buf, point2 - buf );
    }
  }
  return;
}


/*
 * Write text to one char, doesn't test if writing note -- Oxtal
 */

void write_to_char(DESCRIPTOR_DATA * desc, const char *txt )
{
  const	char 	*point;
  char 	*point2;
  char 	buf[ MAX_STRING_LENGTH*4 ];
  int	skip = 0;

  buf[0] = '\0';
  point2 = buf;
  if ( txt && desc ) {
    if( IS_SET( desc->character->act, PLR_COLOUR ) ) {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  skip = colour( *point, desc->character, point2 );
	  while( skip-- > 0 )
	    ++point2;
	  continue;
	}
	*point2 = *point;
	*++point2 = '\0';
      }			
      *point2 = '\0';
      write_to_buffer( desc, buf, point2 - buf );
    }
    else {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  continue;
	}
	*point2 = *point;
	*++point2 = '\0';
      }
      *point2 = '\0';
      write_to_buffer( desc, buf, point2 - buf );
    }
  }
  return;
}


/* Send formatted text to a char , By Oxtal*/

void send_to_charf(CHAR_DATA *ch, const char *fmt, ...)
{
  va_list argptr;

  static char buf[MAX_STRING_LENGTH*4]; /* Enough ?     remove static char buf  SinaC 2003*/
  buf[0] = '\0'; // static re-added

  va_start(argptr, fmt);
  vsprintf(buf, fmt, argptr);
  send_to_char(buf,ch);

  va_end(argptr);
}


/*
 * Send a page to one char.
 */
void page_to_char_bw( const char *txt, CHAR_DATA *ch )
{
  if ( txt == NULL || ch->desc == NULL)


    if (ch->lines == 0 ) {
      send_to_char(txt,ch);
      return;
    }
	
#if defined(macintosh)
  send_to_char(txt,ch);
#else
  ch->desc->showstr_head = (char *)GC_MALLOC_ATOMIC(strlen(txt) + 1);
  strcpy(ch->desc->showstr_head,txt);
  ch->desc->showstr_point = ch->desc->showstr_head;
  show_string(ch->desc,"");
#endif
}

/*
 * Page to one char, new colour version, by Lope.
 */
void page_to_char( const char *txt, CHAR_DATA *ch )
{
  const	char	*point;
  char	*point2;
  char	buf[ MAX_STRING_LENGTH * 9 ]; // 9*4608 > 32768   9 instead of 8 for color code
  int	skip = 0;

  buf[0] = '\0';
  point2 = buf;
  if( txt && ch->desc ) {
    if( IS_SET( ch->act, PLR_COLOUR ) ) {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  skip = colour( *point, ch, point2 );
	  while( skip-- > 0 )
	    ++point2;
	  continue;
	}
	*point2 = *point;
	*++point2 = '\0';
      }			
      *point2 = '\0';
      ch->desc->showstr_head  = (char *)GC_MALLOC_ATOMIC( strlen( buf ) + 1 );
      strcpy( ch->desc->showstr_head, buf );
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string( ch->desc, "" );
    }
    else {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  continue;
	}
	*point2 = *point;
	*++point2 = '\0';
      }
      *point2 = '\0';
      ch->desc->showstr_head  = (char *)GC_MALLOC_ATOMIC( strlen( buf ) + 1 );
      strcpy( ch->desc->showstr_head, buf );
      ch->desc->showstr_point = ch->desc->showstr_head;
      show_string( ch->desc, "" );
    }
  }
  return;
}


/* string pager */
void show_string(struct descriptor_data *d, char *input)
{
  //char buffer[4*MAX_STRING_LENGTH];
  char buffer[MAX_STRING_LENGTH*9]; // 9 * 4608 > 32768
  char buf[MAX_INPUT_LENGTH];
  register char *scan, *chk;
  int lines = 0, toggle = 1;
  int show_lines;

  one_argument(input,buf);
  if (buf[0] != '\0') {
    if (d->showstr_head) {
      d->showstr_head = 0;
    }
    d->showstr_point  = 0;
    return;
  }

  if (d->character)
    show_lines = d->character->lines;
  else
    show_lines = 0;

  for (scan = buffer; ; scan++, d->showstr_point++) {
    if (((*scan = *d->showstr_point) == '\n' || *scan == '\r')
	&& (toggle = -toggle) < 0)
      lines++;

    else if (!*scan || (show_lines > 0 && lines >= show_lines)) {
      *scan = '\0';
      write_to_buffer(d,buffer,strlen(buffer));
      for (chk = d->showstr_point; isspace(*chk); chk++);
      {
	if (!*chk) {
	  if (d->showstr_head) {
	    d->showstr_head = 0;
	  }
	  d->showstr_point  = 0;
	}
      }
      return;
    }
  }
  return;
}

void act (const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2,
	  int type) {
  /* to be compatible with older code */
  act_new(format,ch,arg1,arg2,type,POS_RESTING);
}

/*
 * The colour version of the act_new( ) function, -Lope
 */
void act_new( const char *format, CHAR_DATA *ch, const void *arg1, 
	      const void *arg2, int type, int min_pos ) {
  static char * const he_she  [] = { "it",  "he",  "she" };
  static char * const him_her [] = { "it",  "him", "her" };
  static char * const his_her [] = { "its", "his", "her" };
 
  CHAR_DATA 		*to;
  CHAR_DATA 		*vch = ( CHAR_DATA * ) arg2;
  OBJ_DATA 		*obj1 = ( OBJ_DATA  * ) arg1;
  OBJ_DATA 		*obj2 = ( OBJ_DATA  * ) arg2;
  const 	char 	*str;
  const char 		*i = NULL;
  char 		*point;
  char 		*pbuff;
  char 		buffer[ MAX_STRING_LENGTH*2 ];
  char 		buf[ MAX_STRING_LENGTH   ];
  char 		fname[ MAX_INPUT_LENGTH  ];
  bool		fColour = FALSE;

  /*
   * Discard null and zero-length messages.
   */
  if( !format || !*format )
    return;

  /* discard null rooms and chars */
  if( !ch || !ch->in_room )
    return;

  to = ch->in_room->people;
  if( type == TO_VICT ) {
    if( !vch ) {
      bug( "Act: null vch with TO_VICT." );
      return;
    }

    if( !vch->in_room )
      return;

    to = vch->in_room->people;
  }
 
  for( ; to ; to = to->next_in_room ) {
    if ( DUMP_MOB_MSG ) {
      if ( IS_NPC(to) )
	log_stringf("DUMP MOB [%s]: %s", to->short_descr, format );
    }

    if( !to->desc || to->position < min_pos )
      continue;
    /*
      // Modified by SinaC for some test
      if ( to->position < min_pos )
      continue;
    */
    if( ( type == TO_CHAR ) && to != ch )
      continue;
    if( type == TO_VICT && ( to != vch || to == ch ) )
      continue;
    if( type == TO_ROOM && to == ch )
      continue;
    if( type == TO_NOTVICT && (to == ch || to == vch) )
      continue;
      
    point   = buf;
    str     = format;
    while( *str != '\0' ) {
      if( *str != '$' ) {
	*point++ = *str++;
	continue;
      }
	  
      fColour = TRUE;
      ++str;
      i = " <@@@> ";
      if( !arg2 && *str >= 'A' && *str <= 'Z' ) {
	bug( "Act: missing arg2 for code %d.", *str );
	i = " <@@@> ";
      }
      else {
	switch ( *str ) {
	default:  bug( "Act: bad code %d.", *str );
	  i = " <@@@> ";                                break;
	  /* Thx alex for 't' idea */
	case 't': i = (char *) arg1;                            break;
	case 'T': i = (char *) arg2;                            break;
	case 'n': i = PERS( ch,  to  );                         break;
	case 'N': i = PERS( vch, to  );                         break;
	case 'e': i = he_she  [URANGE(0, ch  ->cstat(sex) , 2)];        break;
	case 'E': i = he_she  [URANGE(0, vch ->cstat(sex) , 2)];        break;
	case 'm': i = him_her [URANGE(0, ch  ->cstat(sex) , 2)];        break;
	case 'M': i = him_her [URANGE(0, vch ->cstat(sex) , 2)];        break;
	case 's': i = his_her [URANGE(0, ch  ->cstat(sex) , 2)];        break;
	case 'S': i = his_her [URANGE(0, vch ->cstat(sex) , 2)];        break;
		  
	case 'p':
	  i = can_see_obj( to, obj1 )
	    ? obj1->short_descr
	    : "something";
	  break;
		  
	case 'P':
	  i = can_see_obj( to, obj2 )
	    ? obj2->short_descr
	    : "something";
	  break;
		  
	case 'd':
	  if ( arg2 == NULL || ((char *) arg2)[0] == '\0' ) {
	    i = "door";
	  }
	  else {
	    one_argument( (char *) arg2, fname );
	    i = fname;
	  }
	  break;
	}
      }
	  
      ++str;
      while ( ( *point = *i ) != '\0' )
	++point, ++i;
    }
      
    *point++ = '\n';
    *point++ = '\r';
    *point	 = '\0';
    buf[0]   = UPPER(buf[0]);
    if ( TESTING_CHARMIES ) {
      // Added by SinaC 2000 for some test
      if ( IS_NPC(to) && to->master != NULL && IS_IMMORTAL(to->master)) {
	char buf2[MAX_STRING_LENGTH];
	sprintf(buf2,"{R[%s:]{x %s\n\r",to->short_descr,buf);
	pbuff = buffer;
	colourconv(pbuff,buf2,to->master);
	write_to_buffer(to->master->desc,buffer,0);
      }
    }
    else {
      pbuff	 = buffer;
      colourconv( pbuff, buf, to );
      
      if ( to->desc && to->desc->connected == CON_PLAYING )
	write_to_buffer( to->desc, buffer, 0 );
    }
  }
  return;
}



/*
 * Macintosh support functions.
 */
#if defined(macintosh)
int gettimeofday( struct timeval *tp, void *tzp )
{
  tp->tv_sec  = time( NULL );
  tp->tv_usec = 0;
}
#endif

int colour( char type, CHAR_DATA *ch, char *string )
{
  char	code[ 20 ];
  char	*p = '\0';

  // Modified by SinaC 2001 for color greet: ch != NULL
  if( ch != NULL && IS_NPC( ch ) )
    return( 0 );

  switch( type ) {
  default:
    sprintf( code, CLEAR );
    break;
  case 'x':
    sprintf( code, CLEAR );
    break;
  case 'b':
    sprintf( code, C_BLUE );
    break;
  case 'c':
    sprintf( code, C_CYAN );
    break;
  case 'g':
    sprintf( code, C_GREEN );
    break;
  case 'm':
    sprintf( code, C_MAGENTA );
    break;
  case 'r':
    sprintf( code, C_RED );
    break;
  case 'w':
    sprintf( code, C_WHITE );
    break;
  case 'y':
    sprintf( code, C_YELLOW );
    break;
  case 'B':
    sprintf( code, C_B_BLUE );
    break;
  case 'C':
    sprintf( code, C_B_CYAN );
    break;
  case 'G':
    sprintf( code, C_B_GREEN );
    break;
  case 'M':
    sprintf( code, C_B_MAGENTA );
    break;
  case 'R':
    sprintf( code, C_B_RED );
    break;
  case 'W':
    sprintf( code, C_B_WHITE );
    break;
  case 'Y':
    sprintf( code, C_B_YELLOW );
    break;
  case 'D':
    sprintf( code, C_D_GREY );
    break;
  case '*':
    sprintf( code, "%c", 007 );
    break;
  case '/':
    sprintf( code, "%c", 012 );
    break;
  case '{':
    sprintf( code, "%c", '{' );
    break;
  }

  p = code;
  while( *p != '\0' ) {
    *string = *p++;
    *++string = '\0';
  }

  return( strlen( code ) );
}

void colourconv( char *buffer, const char *txt, CHAR_DATA *ch )
{
  const	char	*point;
  int	skip = 0;

  if( ch->desc && txt ) {
    if( IS_SET( ch->act, PLR_COLOUR ) ) {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  skip = colour( *point, ch, buffer );
	  while( skip-- > 0 )
	    ++buffer;
	  continue;
	}
	*buffer = *point;
	*++buffer = '\0';
      }
      *buffer = '\0';
    }
    else {
      for( point = txt ; *point ; point++ ) {
	if( *point == '{' ) {
	  point++;
	  continue;
	}
	*buffer = *point;
	*++buffer = '\0';
      }
      *buffer = '\0';
    }
  }
  return;
}

/* RY 1995 INFO (BATTLE et all..) channel */
void info( const char *argument )
{
  DESCRIPTOR_DATA *d;
        
  for ( d = descriptor_list; d; d = d->next ) {
    if ( d->connected == CON_PLAYING ) {
      send_to_char( "\n\r",d->character );
      send_to_char( argument, d->character );
      /*send_to_char( "\n\r", d->character );*/
    }
  }
  return;
}

// Added by SinaC 2003 for SIGSEGV capture
void init_signals () {
  signal (SIGBUS, sig_handler);
  signal (SIGTERM, sig_handler);
  signal (SIGABRT, sig_handler);
  signal (SIGSEGV, sig_handler);

  struct itimerval itime;
  struct timeval interval;
  interval.tv_sec = 180;
  interval.tv_usec = 0;
  itime.it_interval = interval;
  itime.it_value = interval;
  setitimer(ITIMER_VIRTUAL, &itime, NULL);
  signal (SIGVTALRM, sig_handler);
}

void sig_handler( int sig ) {
  switch(sig) {
    // SinaC 2003
  case SIGVTALRM:
    if (!tics) {
      bug("Sig handler SIGVTALRM: tics not updated. (Infinite loop suspected)");
      auto_shutdown();
    }
    else
      tics = 0;
    break;
  case SIGBUS:
    bug("Sig handler SIGBUS.");
    auto_shutdown();
    break;
  case SIGTERM:
    bug("Sig handler SIGTERM.");
    auto_shutdown();
    break;
  case SIGABRT:
    bug("Sig handler SIGABRT");  
    auto_shutdown();
    break;
  case SIGSEGV:
    bug("Sig handler SIGSEGV");
    auto_shutdown();
    break;
  }
}

char *getToken( const char *s ) {
  static char buf[MAX_INPUT_LENGTH];
  buf[0] = '\0';
  // +1: skips '-'
  const char *t = strstr( s+1, "=" );
  if ( t == NULL )
    return NULL;
  strncpy( buf, s+1, t-s-1 ); // copy from - to =
  buf[t-s-1] = '\0';
  return buf;
}
char *getTokenValue( const char *s ) {
  static char buf[MAX_INPUT_LENGTH];
  buf[0] = '\0';
  const char *t = strstr( s+1, "=" );
  if ( t == NULL )
    return NULL;
  strncpy( buf, t+1, strlen( t ) ); // copy after =
  buf[strlen(t)] = '\0';

  return buf;
}
void use_command_line_token( const char *token, const Value &tokvalue ) {
  const int tagId = find_tag( token, DATA_CONFIG ); // only CONFIG tags are accepted
  //  printf("argName: %s  tagId: %d\n\r", token, tagId );
  if ( !assign_config_variable( tagId, tokvalue ) ) {
    printf("Invalid arg: %s\n", token );
    exit(-1);
  }
}
// available arguments are:
//  state <control id>           --> generated command line by the mud with copyover
//  copyover <control id>        -->    "         "      "   "  "   "    "     "
//  <config entry>               --> better to be 1st argument if you want to overwrite variable
//                                    later in command line
//  <config-file>:<config entry> -->  same as above
//  <config variable>=<value>    --> config value that can be found in a config file (see config.C)
//                                    or dbdata.C:tagList  with whichData containing DATA_CONFIG
extern void mini_boot_db();
void parse_command_line( int argc, char **argv ) {
  //  printf("argc: %d\n\r", argc );
  for ( int i = 1; i < argc; i++ ) {
    //    log_stringf("arg[%d]: [%s]", i, argv[i] );
    const char *s = argv[i];
    if ( s[0] == '-' ) {
      char *token = getToken( s );
      char *tokvalue = getTokenValue( s );
      //      log_stringf("TOKEN: %s  VALUE: %s", token, tokvalue );
      if ( token == NULL || tokvalue == NULL ) {
	printf( "Bad argument[%d]: [%s]", i, argv[i] );
	exit(-1);
      }
      Value v; // Create a valid value depending on tokvalue's type: integer/boolean/string
      if ( is_number( tokvalue ) )
	v = atoi(tokvalue);
      else if ( !str_cmp( tokvalue, "true" ) )
	v = 1;
      else if ( !str_cmp( tokvalue, "false" ) )
	v = Value((long)0);
      else
	v = tokvalue;
      use_command_line_token( token, v );
    }
    else
      // with copyover and state, next argument must be the control id: an integer
      if ( !str_cmp( s, "copyover" ) ) {
	fCopyOver = TRUE;
	fState = FALSE;
	//	log_stringf("COPYOVER");
	if ( i == argc-1 ) { // missing control
	  printf("Missing control id\n\r");
	  exit(-1);
	}
	if ( !is_number( argv[i+1] ) ) { // bad control
	  printf("Invalid control id: [%s]  should be an integer\n\r", argv[i+1 ]);
	  exit(-1);
	}
	control = atoi( argv[i+1] );
	i++;
	//	log_stringf("CONTROL %d", control );
      }
      else if ( !str_cmp( s, "state" ) ) {
	fCopyOver = TRUE;
	fState = TRUE;
	//	log_stringf("STATE");
	if ( i == argc-1 ) { // missing control
	  printf("Missing control id\n\r");
	  exit(-1);
	}
	if ( !is_number( argv[i+1] ) ) { // bad control
	  printf("Invalid control id: [%s]  should be an integer\n\r", argv[i+1 ]);
	  exit(-1);
	}
	control = atoi( argv[i+1] );
	i++;
	//	log_stringf("CONTROL %d", control );
      }
      else if ( !str_cmp( s, "generateHTML" ) ) {
	mini_boot_db();
	generate_html_races_help_files();
	printf("Races help files generated.\n\r");
	generate_html_classes_help_files();
	printf("Classes help files generated.\n\r");
	generate_html_abilities_help_files();
	printf("Abilities help files generated.\n\r");
	generate_html_gods_help_files();
	printf("Gods help files generated.\n\r");
	generate_html_groups_help_files();
	printf("Groups help files generated.\n\r");
	printf("Closing...\n\r");
	exit(-1);
      }
      else {
	const char *p = strstr( s, ":" );
	if ( p != NULL ) { // config file:config entry
	  int size = strlen(s)-strlen(p);
	  strncpy( CONFIG_FILE, s, size );
	  CONFIG_FILE[size] = '\0';
	  s = p+1;
	}
	//	log_stringf("CONFIG: %s", s );
	if ( !load_config( s ) ) {
	  fprintf( stderr, "Config [%s] not found.", s );
	  exit(1);
	}
      }
  }

  //  show_config_variable();
}
