#ifndef __PREREQS_H__
#define __PREREQS_H__

#define DEFAULT_PREREQ_COST(j) (UMAX( 1, (j-1)*5))

void new_save_prerequisites();
void new_load_prerequisites();

int check_prerequisites_cycle( const int fromSn, const int fromCasting, 
			       const int toSn, const int toCasting );

#endif
