#ifndef __ERROR_HH__
#define __ERROR_HH__

struct GeneralException {
  const char* msg;

  GeneralException( const char* m) {
    msg = str_dup(m);
  }
};


struct ScriptException {
  const char* msg;

  ScriptException(const char* m) {
    msg = str_dup(m);
  }
};


void g_error( const char* msg, ...) __attribute__ ((format (printf, 1, 2)));
void p_error(const char* msg, ...) __attribute__ ((noreturn, format (printf, 1, 2)));
void p_warn(const char* msg, ...) __attribute__ ((format (printf, 1, 2)));


#endif
