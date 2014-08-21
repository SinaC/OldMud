#ifndef _MOONS_H_
#define _MOONS_H_

struct moon_data {
  int ph_t;
  int ph_p;
  
  int po_t;
  int po_v;
  int po_p;
  char * name;
};

#define NBR_MOON 2
#define NBR_PHASE 8

extern struct moon_data moon_info[NBR_MOON];
extern const char * moon_phase_msg[NBR_PHASE];


int moon_lookup( const char *name );
int moon_phase( const int);
bool moon_night();
bool moon_visible(const int n);
bool moon_insky(const int n);
bool moon_full( const int n );
void update_moons(char * buf);

#endif
