#ifdef KRD
#define LOG_FILE "/tmp/krd_proc_mgr.log"
#else
#define LOG_FILE "/tmp/proc_mgr.log"
#endif
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, Keith DeWald, All rights reserved.
//-----------------------------------------------------------------------------|
//
//	Module:	proc_mgr
//
//	Description:	
//
//	FileName:	proc_mgr.cpp
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       02/5/2020      Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <spawn.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <signal.h>
#include <sys/siginfo.h>
#include <sys/wait.h>
#include <sys/neutrino.h>
#include "w2char.hpp"
#include "nov_log.hpp"
#include "proc_mgr.hpp"
#include "rt_mqueue.hpp"
#include "rt_errors.hpp"

FILE* logfile = stderr;

static int stop_requested = 0;
static int still_trying = 1;

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[3] = "PR";

#define MSG_MQ PMMQ_TO_MSG
//=============================================================================|

#ifdef KRD
static const char * const proc_names[NUM_WP_VALS] = { 	
	// "krd_stat_mgr",
	"krd_log_mgr",
	"krd_msg_mgr",
	"krd_dev_mgr",
	// "krd_time_mgr",
	"krd_sched_mgr",
	"krd_asif_mgr"
	//"krd_tc_drvr" 
};
static const char * const cp_paths[NUM_WP_VALS] = {
		// "../bin/krd_stat_mgr",
		"../bin/krd_log_mgr",
		"../bin/krd_msg_mgr",
		"../bin/krd_dev_mgr",
		// "../bin/krd_time_mgr",
		"../bin/krd_sched_mgr",
		"../bin/krd_asif_mgr"
		//"../bin/krd_tc_drvr" 
};
#else
static const char * const proc_names[NUM_WP_VALS] = { 	
	// "stat_mgr",
	"log_mgr",
	"msg_mgr",
	"dev_mgr",
	// "time_mgr",
	"sched_mgr",
	"asif_mgr"
	//"tc_drvr" 
};
static const char * const cp_paths[NUM_WP_VALS] = {
		// "../bin/stat_mgr",
		"../bin/log_mgr",
		"../bin/msg_mgr",
		"../bin/dev_mgr",
		// "../bin/time_mgr",
		"../bin/sched_mgr",
		"../bin/asif_mgr"
		//"../bin/tc_drvr" 
};
#endif
static proc_mgr pm_proc_mgr(NUM_WP_VALS);

static pm_child_proc * child_procs[NUM_WP_VALS];

// this array keeps track of just the mqueues asif_mgr uses out of
// all the mqueues in the system
//
mod_mqueue mq[NUM_PMMQ_VALS] =
{
	// which-mqueue         read/write		block/non-block	rt_mqueue pointer
	{ WMQ_ALL_TO_MSG,		rw_write_only,	blk_blocking,	NULL}
};
//=============================================================================|
// pm_child_proc methods
//
int proc_mgr::start_child_proc(which_proc_t wp, pm_child_proc ** p_proc)
{
	int error_code = SUCCESS;
	
	PLOG(DIAG5,TEMP_mID,
		"Starting %s child process\n",proc_names[wp]);
	if((*p_proc = new pm_child_proc(wp,(char *)(cp_paths[wp]),
		(char *)(proc_names[wp])))== NULL)
	{
		error_code = FAILURE;
		*p_proc = NULL;
	}
	return(error_code);
};

int proc_mgr::get_wp_for_pid(pid_t pid, which_proc_t * p_wp)
{
	int error_code = SUCCESS;
	which_proc_t wp;
	bool found = 0;
	for(wp=(which_proc_t)0; wp<NUM_WP_VALS && !found; (int)wp+=1)
	
	{
		if(child_procs[wp]->proc_id==pid)
		{
			found = 1;
			*p_wp = wp;
			break;
		}
	}
	if(!found)
	{
		PLOG(DIAG5,TEMP_mID,
			"p_ERROR: pid %d not for legal child proc type in "
			"get_wp_for_pid.\n",pid);
		error_code = FAILURE;
	}
	
	return(error_code);
};

// cl_process_registry	qnx_proc_reg; // process inforor all started here

//=============================================================================|
// pm_child_proc methods
//

pm_child_proc::pm_child_proc(which_proc_t wp, 	char * path, char * name)
{
	pid_t new_proc_id = 0;
//	struct inheritance inherit;
	int error_val;
	char *const argv[2] = { path,NULL };
	en_rt_error_type result;

	// Start the process
	if((new_proc_id = spawn(path,0,NULL,
		(spawn_inheritance_type *)NULL,argv,NULL)) == -1)
	{
		error_val = errno;
		PERR(error_val,FATAL,TEMP_mID,
			"p_Error # %d starting %s, %s\n",error_val,name,
			strerror(error_val));
		result = ERT_CONSTR_FAILED;
	}
	else if(new_proc_id == 0)
	{
		error_val = errno;
		PLOG(DIAG5,TEMP_mID,
			"Bad return value of %d from spawn\n",new_proc_id);
		if(error_val != EOK)
		{
			PLOG(DIAG5,TEMP_mID,
				"errno was set to %d : %s\n",error_val,strerror(error_val));
		}
		else
		{
			PLOG(DIAG5,TEMP_mID, "errno was not set\n");
		}
		result = ERT_CONSTR_FAILED;
	}
	else
	{
		this->proc_id = new_proc_id;
	}
};

pm_child_proc::~pm_child_proc()
{
	// find process and check status

	// if found remove process from registry

};

#if 0
//=============================================================================|
// proc_registry methods
//

int proc_registry::register_proc(pid_t pid,proc_name_t pname)
{
	proc_reg_entry * preg_entry;
	int error_val = SUCCESS;
	if((preg_entry = new proc_reg_enty(pid,pname))!=SUCCESS)
	{
		PLOG(DIAG5,TEMP_mID,
			"Failed to construct proc_reg_entry in register_proc\n");
		error_val = ERT_CONSTR_FAILED;
		goto ERROR_EXIT;
	}
	else
	{
			
	}
ERROR_EXIT:
	return(error_val);
};

proc_registry::remove_proc(proc_name_t pname)
{
	
};

int proc_registry::get_proc_name(pid_t pid, char * name, int namelen)
{
};
	
#endif

void restart_handler( int signo )
{
	PLOG(DIAG5,TEMP_mID,
		"PROC RECEIVED USR1 SIGNAL %d, restart proc_mgr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	PLOG(DIAG5,TEMP_mID,
		"PROC RECEIVED USR2 SIGNAL %d, stop proc_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//=============================================================================|
//
// This is the main thread of the process manager.  It starts all of the
// system processes and then monitors them for any changes.  If a child
// process dies or teminates then it restarts it.  If all child processes
// die or terminate, then it restarts all of them.
//
// TODO: add thrashing protection -- detect multiple cycles of termination
// and restarting, and shut down completely after logging status and errors.
//
int
main()
{
//	int survival_likely = 1;
	which_proc_t wp;
	pm_child_proc * p_proc;
	// rt_mqueue * p_mq[NUM_WMQ_VALS];
	// en_which_mqueue wq;

	struct sigaction restart_act;
	struct sigaction stop_act;
	sigset_t restart_set;
	sigset_t stop_set;

	sigemptyset( &restart_set );
	sigaddset( &restart_set, SIGUSR1 );
	restart_act.sa_flags = 0;
	restart_act.sa_mask = restart_set;
	restart_act.sa_handler = &restart_handler;
	sigaction( SIGUSR1, &restart_act, NULL);
	
	sigemptyset( &stop_set );
	sigaddset( &stop_set, SIGUSR2 );
	stop_act.sa_flags = 0;
	stop_act.sa_mask = stop_set;
	stop_act.sa_handler = &stop_handler;
	sigaction( SIGUSR2, &stop_act, NULL);

#ifdef LOG_FILE
	if((logfile = fopen(LOG_FILE,"w+"))==NULL)
	{
		int error_val = errno;
		printf("ERROR: opening log file %s, use stderr, error %d, %s\n",
				LOG_FILE,error_val,strerror(error_val));
		logfile = stderr;
	}
#endif
	rt_set_logfile("PR",logfile);

#ifdef KRD
	PLOG(DIAG5,TEMP_mID,"Running as KRD\n");
#else
	PLOG(DIAG5,TEMP_mID,"NOT Running as KRD\n");
#endif

	while(still_trying)
	{
#if 0
		// create all message queues for interprocess communications
		for(wq=(en_which_mqueue)0;wq<NUM_WMQ_VALS; 
						wq = (en_which_mqueue)((int)wq+1))
		{
			if((p_mq[wq] = 
				new rt_mqueue::rt_mqueue(wq,rw_read_only,blk_blocking))==NULL)
			{
				PLOG(DIAG5,TEMP_mID,
					"ERROR: failed to construct new rt_mqueue");
			}
			else
				PLOG(DIAG5,TEMP_mID,
					"Opened mqueue %d\n",p_mq[wq]->rt_mqueue_descr);
		}
#endif
		PLOG(DIAG5,TEMP_mID,"Prepare to start child procs\n");

		// Start with NULL child_proc pointers
		for(wp=(which_proc_t)0;wp<NUM_WP_VALS;(int)wp+=1)
		{
			child_procs[wp] = NULL;
			PLOG(DIAG5,TEMP_mID,"setting ptr to NULL\n");
		}

		// Start all the child procs one time
		for(wp=(which_proc_t)0;wp<NUM_WP_VALS;(int)wp+=1)
		{
			PLOG(DIAG5,TEMP_mID,"starting %s\n",proc_names[wp]);
			if((pm_proc_mgr.start_child_proc(wp,&p_proc))
				!=SUCCESS)
			{
				PLOG(DIAG5,TEMP_mID,
					"ERROR: failed to start %s\n",proc_names[wp]);
			}
			else
			{
				child_procs[wp] = p_proc;
			}
		}
		stop_requested = 0;
		int loop=0;
		while(!stop_requested)
		{
			loop++;
			sleep(10);
		}
#if 0

		//
		// Monitor children processes and take action when necessary
		//
		survival_likely = 1;
		while(survival_likely)
		{
			siginfo_t siginfo;
			pid_t wait_pid;
			int error_val = EOK;
			which_proc_t wp;

			// Check for change in status of childrens' status
			bzero(&siginfo,sizeof(siginfo_t));
			if((wait_pid = waitid(P_ALL, getpgid(getpid()), 
											&siginfo, WEXITED|WSTOPPED))!=0)
//=WAIT===================
			{
				error_val = errno;
				PLOG(DIAG5,TEMP_mID,
					"Error %d after call to waitid(), %s\n",error_val,strerror(error_val));
				if(error_val == ECHILD)
				{
					PLOG(DIAG5,TEMP_mID,
						"no child processes left, restart all\n");
					survival_likely = 0;
					continue;
				}

			}
			else
			{
				if(pm_proc_mgr.get_wp_for_pid(siginfo.__data.__proc.__pid,
						&wp)==SUCCESS)
				{
					PLOG(DIAG5,TEMP_mID,
						"%s child process %d, changed state\n",
						proc_names[wp], siginfo.__data.__proc.__pid);
					// restart process that stopped
					if((pm_proc_mgr.start_child_proc(wp,p_proc))
						!=SUCCESS)
					{
						PLOG(DIAG5,TEMP_mID,
							"p_ERROR: failed to start %s\n",proc_names[wp]);
					}
					child_procs[wp] = p_proc;
				}
				else
					PLOG(DIAG5,TEMP_mID, "Couldn't get wp for pid %d\n", 
						siginfo.__data.__proc.__pid);
			}

		}
#endif	

		PLOG(DIAG5,TEMP_mID,"close everything down before restart or stop\n");
		// Shutdown and kill all child processes
		for(wp=(which_proc_t)(NUM_WP_VALS-1);wp >= 0;(int)wp-=1)
		{
			struct timespec nslts;
			nslts.tv_sec = 0;
			nslts.tv_nsec = 100000000L;
			PLOG(DIAG5,TEMP_mID,"Telling %s to shutdown\n",proc_names[wp]);
			if(kill(child_procs[wp]->proc_id,SIGUSR2)==-1)
			{
				int error_val = errno;
				PLOG(DIAG5,TEMP_mID,
					"kill %d failed for %s with errno %d, %s\n",
								child_procs[wp]->proc_id,
								proc_names[wp],error_val,strerror(error_val));
			}
			delete child_procs[wp];
			nanosleep(&nslts,NULL);
			// sleep(1);
		}
		// Clean up everything, shut down for a restart.
#if 0
		for(wq=(en_which_mqueue)0;wq<NUM_WMQ_VALS;
						wq = (en_which_mqueue)((int)wq+1))
		{
			delete p_mq[wq];
		}
		destroy_mq_msg_id_lock_and_sem();
#endif
	}
	// Log what we can about what happened
	
#ifdef LOG_FILE
	if(logfile!=stderr)
		fclose(logfile);
#endif
		
	// Shut down
		
	return(0);
};

