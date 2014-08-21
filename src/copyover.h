#ifndef __COPYOVER_H__
#define __COPYOVER_H__

extern bool SAVE_STATE_ON_COPYOVER;

extern char * EXE_FILE; /* Oxtal */

void auto_shutdown           args( ( void ) );
void copyover_recover(void);

void save_on_shutdown();
// Added by SinaC 2003 for SIGSEGV capture
void log_shutdown( const char *typeReboot);


void auto_copyover_update();


DECLARE_DO_FUN( do_copyove );
DECLARE_DO_FUN( do_copyover );

#endif
