#ifdef KRD
#define LOG_FILE "/tmp/krd_time_mgr.log"
#else
#define LOG_FILE "/tmp/time_mgr.log"
#endif
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, Keith DeWald, All rights reserved.
//-----------------------------------------------------------------------------|
//
//	Module:	time_mgr
//
//	Description:	
//
//	FileName:	time_mgr.cpp
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       02/9/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include <unistd.h>
#include <stdio.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include "w2char.hpp"
#include "time_mgr.hpp"


static int stop_requested = 0;
static int still_trying = 1;

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	fprintf(stderr,"TIME RECEIVED USR1 SIGNAL %d, restart time_mgr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	fprintf(stderr,"TIME RECEIVED USR2 SIGNAL %d, stop time_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
int
main(int argc, char** argv)
{
	mqd_t mqd;
	struct mq_attr mqa;
	int i;

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

	// this array keeps track of just the mqueues time_mgr uses out of
	// all the mqueues in the system
	//
	static mod_mqueue mq[NUM_TMMQ_VALS] =
	{
		// which-mqueue		read/write		blk/non-blk		rt_mqueue pointer
		{ WMQ_ALL_TO_MSG,	rw_write_only,	blk_blocking,	NULL},
		{ WMQ_TC_TO_SCHED, 	rw_write_only, 	blk_blocking,	NULL},
		{ WMQ_SCHED_TO_TC, 	rw_read_only, 	blk_blocking,	NULL},
		{ WMQ_TC_TO_STAT, 	rw_write_only, 	blk_blocking,	NULL},
		{ WMQ_STAT_TO_TC, 	rw_read_only, 	blk_blocking,	NULL},
	};

	//
	// instantiate and open all of the message queues to and from other procs
	//
	for(i=0;i<NUM_TMMQ_VALS;i++) 
	{
		if((mq[i].rt_mq = new rt_mqueue(mq[i].which,mq[i].rw))==NULL)
		{
			printf("t_ERROR: Failed to open message queue for %s in time_mgr\n",
							rt_mq_names[mq[i].which]);
		}
	}

	for(i=0;i<30;i++)
	{
		sleep(1);
		printf("time_mgr %d\n",i);
	}


	mqa.mq_maxmsg = 1800;
	mqa.mq_msgsize = 64;
	mqa.mq_flags = 0;

	// run loop simulating messages sent on vertical interrupt
	while(1)
	{
		char mqmsg[64];
		sleep(1);

		if(mq_send(mqd,mqmsg,strlen(mqmsg),0)==-1)
		{
			int error_val = errno;
			printf("t_ERROR: Failed on call to mq_receive %d, %s\n",error_val,
					strerror(error_val));
		}
	}


	// close all of the message queues used
	for(i=0;i<NUM_TMMQ_VALS;i++) 
	{
		delete mq[i].rt_mq;
	}
	return(0);
}
