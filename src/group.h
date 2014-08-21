#ifndef __GROUP_H__
#define __GROUP_H__

void    add_minor_sphere      args( ( CHAR_DATA *ch, int godId ) );

int 	group_lookup	args( (const char *name) );
void	gn_add		args( ( CHAR_DATA *ch, int gn) );
void 	gn_remove	args( ( CHAR_DATA *ch, int gn) );
void 	group_add	args( ( CHAR_DATA *ch, const char *name, bool deduct) );
void	group_remove	args( ( CHAR_DATA *ch, const char *name) );
void    groups_help     args( (const char *argument, BUFFER *output ) );


DECLARE_DO_FUN( do_groups );

#endif
