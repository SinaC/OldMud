#ifndef __DBSCRIPT_H__
#define __DBSCRIPT_H__

#define END_OF_SCRIPT_LIST '~'

extern int MAX_SCRIPTS;

// we upgrade script_file_list's size by this step
#define STEP_SCRIPT_FILE_LIST (128)
// we upgrade class_list's size by this step
#define STEP_CLASS_LIST (64)
struct file_list {
  const char *name;
  bool to_save; // true if a class in this file has been modified and not yet saved
  int max_class_count; // maximum number of entries in class_list
  int class_count; // current number of entries in class_list
  CLASS_DATA **class_list;
};
extern int cur_script_file; // current number of entries in script_file_list
extern file_list *script_file_list;
// Fill script_file_list, called in dump_script_file, read_scripts and do_script_save
bool compute_file_list();
void update_file_list(); // update to_save value
// TRUE: if class file list has no need to be updated
//  modified by compute_file_list, do_script_edit, script_edit_file
extern bool FILE_LIST_COMPUTED;
// TRUE: if no class has been modified
extern bool FILE_LIST_UPDATED;


// Trigger structure
struct TriggerDescr {
  const char* name;
  int nparms;
  int min_pos; // minimal position, only available when calling a MOBPROG
  const char *parmname[MAX_FCT_PARM];
};
// trigger list
extern TriggerDescr triggers[];
TriggerDescr *isTrigger( FCT_DATA *fct );
TriggerDescr *isTrigger( const char *name, const bool caseSensitive = TRUE );


extern long topfield; 

bool isRootClass( CLASS_DATA *pClass );
bool check_cyclic_inheritance_one_class( CLASS_DATA *cl, CLASS_DATA *additional_parent );

void boot_scripts();
void check_needed_scripts();
CLASS_DATA* get_root_class( CLASS_DATA *cla );
void reload_scripts( CHAR_DATA *ch, const char *argument );

#endif

