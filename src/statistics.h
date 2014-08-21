#ifndef __STATISTICS_H__
#define __STATISTICS_H__

extern const char *problem_list[];
int get_item_points( OBJ_INDEX_DATA *pObjIndex, int &problem );

DECLARE_DO_FUN( do_writestat );

#endif
