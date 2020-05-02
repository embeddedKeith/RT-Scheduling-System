//-----------------------------------------------------------------------------|
//
// stat_mgr.cpp
//
// This module maintains system status and provides asyncronous reporting
// thereof, with synchronous reports pulled by sched_mgr each frame (or
// perhaps field) interrupt for report to Automation system via stat_mgr.
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/12/01
//
// last mod: 8/29/01
//-----------------------------------------------------------------------------|
#include <unistd.h>
#include <stdio.h>
#include "w2char.hpp"
#include "stat_mgr.hpp"


static int stop_requested = 0;
static int still_trying = 1;

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	fprintf(stderr,"RECEIVED USR1 SIGNAL %d, restart stat_mgr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	fprintf(stderr,"RECEIVED USR2 SIGNAL %d, stop stat_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
int
main()
{

	int i;
	// this array keeps track of just the mqueues stat_mgr uses out of
	// all the mqueues in the system
	//
	static stat_mqueue mq[NUM_STMQ_VALS] =
	{
		// which-mqueue         read/write		rt_mqueue pointer
		{ WMQ_STAT_TO_ASIF, 	rw_write_only, 	NULL},
		{ WMQ_ASIF_TO_STAT, 	rw_read_only, 	NULL},
		{ WMQ_STAT_TO_DEV, 		rw_write_only, 	NULL},
		{ WMQ_DEV_TO_STAT, 		rw_read_only, 	NULL},
		{ WMQ_STAT_TO_SCHED, 	rw_write_only, 	NULL},
		{ WMQ_SCHED_TO_STAT, 	rw_read_only, 	NULL},
	};

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

	//
	// instantiate and open all of the message queues to and from other procs
	//
	for(i=0;i<NUM_STMQ_VALS;i++) 
	{
		if((mq[i].rt_mq = new rt_mqueue(mq[i].which,mq[i].rw))==NULL)
		{
			printf("s_ERROR: Failed to open message queue for %s in stat_mgr\n",
							rt_mq_names[mq[i].which]);
		}
	}

	for(i=0;i<30;i++)
	{
		sleep(1);
		printf("stat_mgr %d\n",i);
	}




	// --------------------------- body -----------------------------
#undef STAT_REAL
#ifdef STAT_REAL

	// initialize status elements not handled by status constructor
		// device status

#else
	for(i=0;i<30;i++)
	{
		sleep(1);
		printf("stat_mgr %d\n",i);
	}
#endif  // STAT_REAL


	
	// close all of the message queues used
	for(i=0;i<NUM_STMQ_VALS;i++) 
	{
		delete mq[i].rt_mq;
	}
	return(0);
}
