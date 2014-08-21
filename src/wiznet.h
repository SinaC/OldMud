#ifndef __WIZNET_H__
#define __WIZNET_H__

/* WIZnet flags */
#define WIZ_ON			(A)
#define WIZ_TICKS		(B)
#define WIZ_LOGINS		(C)
#define WIZ_SITES		(D)
#define WIZ_LINKS		(E)
#define WIZ_DEATHS		(F)
#define WIZ_RESETS		(G)
#define WIZ_MOBDEATHS		(H)
#define WIZ_FLAGS		(I)
#define WIZ_PENALTIES		(J)
#define WIZ_SACCING		(K)
#define WIZ_LEVELS		(L)
#define WIZ_SECURE		(M)
#define WIZ_SWITCHES		(N)
#define WIZ_SNOOPS		(O)
#define WIZ_RESTORE		(P)
#define WIZ_LOAD		(Q)
#define WIZ_NEWBIE		(R)
#define WIZ_PREFIX		(S)
#define WIZ_SPAM		(T)
// Added by SinaC 2001
#define WIZ_MEMCHECK            (U)
// Added by SinaC 2001 for name acceptance
#define WIZ_NAMEACCEPT          (V)
// Added by SinaC 2001 for rebirth
#define WIZ_REBIRTH             (W)
// Added by SinaC 2001 for bugs
#define WIZ_BUGS                (X)
// Added by SinaC 2003 for various uses
#define WIZ_VARIOUS             (Y)
// Added by SinaC 2003 for multiclassing
#define WIZ_MULTICLASS          (Z)
// Added by SinaC 2003 for program message
#define WIZ_PROGRAM             (aa)

void wiznet args( (const char *string, CHAR_DATA *ch, OBJ_DATA *obj,
		   long flag, long flag_skip, int min_level ) );

DECLARE_DO_FUN( do_wiznet );
#endif
