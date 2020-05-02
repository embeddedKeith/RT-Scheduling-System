#ifndef _PROC_MGR_
#define _PROC_MGR_
//-----------------------------------------------------------------------------|
// proc_mgr.h
//
// This is the framework and process manager for the QNX Realtime System
// It starts the other processes and monitors them, restarting failed
// child processes if appropriate, or shuting down when necessary.
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/6/01
//
// last mod: 8/7/01
//-----------------------------------------------------------------------------|
#include <sys/timeb.h>
#include <process.h>
#include <map>

enum
en_proc_mgr_error_code
{
	EPM_SUCCESS = 0,
	EPM_FAILED,
	NUM_EPM_VALS
};

typedef char * proc_name_t;

enum
which_proc_t
{       
	//WP_STAT_MGR=0,
	WP_LOG_MGR = 0,
	WP_MSG_MGR,
    WP_DEV_MGR,
    //WP_TIME_MGR,
    WP_SCHED_MGR,
    WP_ASIF_MGR, 
	//WP_TCD_MGR,
    NUM_WP_VALS
};

// list of mqueues used in proc_mgr, these names are a local nomenclature
// (local to the proc_mgr process)
//
enum
en_proc_mqueues
{
	PMMQ_TO_MSG = 0,
	NUM_PMMQ_VALS
};



//-----------------------------------------------------------------------------|
// Class definition for pm_child_proc

class
pm_child_proc
{
private:
	struct timeb 	time_started;
public:
	id_t 			proc_id;
					pm_child_proc(which_proc_t wp,char * path, char * name);
					~pm_child_proc();
};

//-----------------------------------------------------------------------------|
// Class definition for proc_mgr
class
proc_mgr
{
private:
	int 		max_procs_allowed;
public:
				proc_mgr(int max_procs) {max_procs_allowed = max_procs;}
	int			start_child_proc(which_proc_t wp, pm_child_proc ** p_proc);
	int			get_wp_for_pid(pid_t pid, which_proc_t * p_wp);
};


#if 0
//-----------------------------------------------------------------------------|
// Class definition for process registry (TODO)

class 
proc_registry
{
private:
	proc_reg_entry * 		p_procs;
	unsigned long 			num_procs;
	map<pid_t,

public:
					proc_registry() {num_procs=0;p_procs=NULL;}
					~proc_registry();
	int	 			register_proc(pid_t pid,proc_name_t pname);
	int		 		remove_proc(proc_name_t & nname);
	int				get_entry_by_pid(pid_t pid, proc_reg_entry * p_entry);

};

//-----------------------------------------------------------------------------|
// Class definition for process registry entry (TODO)
class
proc_reg_entry
{
private:
	pid_t					pid;
	proc_name_t				name;
	
public:
					proc_reg_entry(pid_t pid, proc_name_t pname);
					~proc_reg_entry();
	char *			get_name();
	pid_t			get_pid();
	
}
#endif

#endif // _PROC_MGR_
