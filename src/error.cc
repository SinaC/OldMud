#include <stdarg.h>
#include "merc.h"
#include "error.hh"
#include "db.h"
#include "wiznet.h"
#include "const.h"

void p_error(const char* msg, ...) {
  va_list argptr;

  static char buf[MAX_STRING_LENGTH]; 

  va_start(argptr, msg);
  vsprintf(buf, msg, argptr);
  va_end(argptr);

  wiznet(buf, NULL, NULL, WIZ_PROGRAM, 0, 0 );

  throw ScriptException(buf);
}

void p_warn(const char* msg, ...) {
  va_list argptr;

  static char buf[MAX_STRING_LENGTH]; 

  va_start(argptr, msg);
  vsprintf(buf, msg, argptr);
  bug("warn in mob/obj/room-program : %s", buf);
  va_end(argptr);
}

void g_error( const char* msg, ...) {
  va_list argptr;

  static char buf[MAX_STRING_LENGTH]; 

  va_start(argptr, msg);
  vsprintf(buf, msg, argptr);
  va_end(argptr);

  throw GeneralException(buf);
}
