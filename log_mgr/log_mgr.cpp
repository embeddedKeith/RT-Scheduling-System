#ifdef KRD
#define LOG_FILE "/tmp/krd_log_mgr.log"
// #define LOG_FILE "log.log"
#else
#define LOG_FILE "/tmp/log_mgr.log"
#endif
#define LOG_CTL_FILE "/tmp/log_ctl.cfg"
#define SHOW_ALL_SOCK_MESSAGES
//-----------------------------------------------------------------------------|
//
// log_mgr.cpp
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/12/01
//
// last mod: 8/29/01
//-----------------------------------------------------------------------------|
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <sys/neutrino.h>
#include "nov_log.hpp"
#include "rt_errors.hpp"
#include "asif_protocol.hpp"
#include "rtc_config.hpp"
#include "w2char.hpp"
#include "rt_mqueue.hpp"
#include "log_mgr.hpp"

FILE* logfile = stderr;

static int stop_requested = 0;
static int still_trying = 1;

extern char* mID_name[NUM_APP_mIDs];

log_ctl_node log_ctl[NUM_APP_mIDs];

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[4] = "LO";

// this array keeps track of just the mqueues log_mgr uses out of
// all the mqueues in the system
//
mod_mqueue mq[NUM_LMMQ_VALS] =
{
	// which-mqueue         read/write		block/non-block	rt_mqueue pointer
	{ WMQ_ALL_TO_MSG,		rw_read_only,	blk_blocking,	NULL}
};

//-----------------------------------------------------------------------------|
// log_proc_rcv_msg
//
void
log_proc_rcv_msg(char * buf, size_t size)
{
	assert(size >= sizeof(mq_msg_header));
	mq_msg_header * p_header = (mq_msg_header *)&buf[0];

	// fprintf(stderr,"log_mgr Received message from mqueue");

	switch(p_header->msg_type)
	{
	// hand_shake is universal to all processes
	case hand_shake:
		// LLOG(DIAG5,TEMP_mID,
		fprintf(stderr,	"Received hand_shake msg from mqueue\n");
		break;
	case send_rtc_sys_log:
	{
		int showing = 0;
		mq_send_rtc_sys_log_msg * p_rtc_sys_msg;
		p_rtc_sys_msg = (mq_send_rtc_sys_log_msg *)p_header;
		p_rtc_sys_msg->buf[p_rtc_sys_msg->buf_len]='\0';
		// always show prio 0 messages
		if(p_rtc_sys_msg->priority == 0)
			showing = 1;

		// and show priorities indicated by entry in log_ctl array
		if(log_ctl[p_rtc_sys_msg->mod_id].level >= p_rtc_sys_msg->priority)
			showing = 1;

		// NOTE: IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// this should be the only path to stderr output for any process
		// in order to prevent overlapping, or interleaved (mixed) streams
		if(showing)
		{
			// fprintf(stderr,"logging %d chars in log_msg\n",
			// 		strlen((char*)(&p_rtc_sys_msg->buf[0])));
			fprintf(logfile,"%s", (char *)(&p_rtc_sys_msg->buf[0]));
			fprintf(stderr,"%s", (char *)(&p_rtc_sys_msg->buf[0]));
		}
		fflush(logfile);
		fflush(stderr);
	}
		break;
	case send_log_control:
	{
		// fprintf(stderr,"Received send_log_control from mqueue\n");
		mq_send_log_control_msg * p_log_ctl = 
								(mq_send_log_control_msg *)p_header;
		if(p_log_ctl->request.mod_id == ALL_mID)
		{
			for(int mid = 0;mid < NUM_APP_mIDs;mid++)
			{
				log_ctl[mid].level = p_log_ctl->request.priority;
				log_ctl[mid].module = p_log_ctl->request.module;
				 fprintf(stderr,"Set %c prio level for %d: %s to %d\n",
					  p_log_ctl->request.module, mid, mID_name[mid],
					 	  p_log_ctl->request.priority);
			}
		}
		else
		{
			log_ctl[p_log_ctl->request.mod_id].level = 
										p_log_ctl->request.priority;
			log_ctl[p_log_ctl->request.mod_id].module = 
										p_log_ctl->request.module;
			 fprintf(stderr,"Setting %c prio level for %d: %s to %d\n",
				 p_log_ctl->request.module, p_log_ctl->request.mod_id,
				 mID_name[p_log_ctl->request.mod_id],
					  p_log_ctl->request.priority);
		}
		// save new log_ctl values in file
		FILE* lcfile = fopen(LOG_CTL_FILE,"w");
		if(lcfile)
		{
			fwrite((char*)(&log_ctl[0]),sizeof(log_ctl_node),
							NUM_APP_mIDs,lcfile);
			fclose(lcfile);
		}
	}
		break;
	default:
		// LLOG(DIAG5,TEMP_mID,
			fprintf(stderr,"ERROR: unknown message type %d received\n",p_header->msg_type);
		break;
	}
};
//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
int
main()
{
	int i;

	// start logging facility
	 // embeddedInitLogging('L');


#if 0
	for(i=0;i<30;i++)
	{
		sleep(1);
		printf("log_mgr %d\n",i);
	}
#endif

#ifdef LOG_FILE
	if((logfile = fopen(LOG_FILE,"w+"))==NULL)
	{
		int error_val = errno;
		printf("ERROR: opening log file %s, use stderr, error %d, %s\n",
				LOG_FILE,error_val,strerror(error_val));
		logfile = stderr;
	}
#endif
	
	still_trying = 1;
	stop_requested = 0;

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

	still_trying = 1;

	while(still_trying)
	{
		fprintf(stderr,"*********************************************\n");
		//MLOG(2,mID_WARN,"Starting mqueues in log_mgr\n");
		fprintf(stderr,"*********************************************\n");
		//
		// instantiate and open all of the message queues to and from other procs
		//
		for(i=0;i<NUM_LMMQ_VALS;i++) 
		{
			if((mq[i].rt_mq = 
				new rt_mqueue(mq[i].which,mq[i].rw,mq[i].blk))==NULL)
			{
				//LLOG(DIAG5,TEMP_mID,
					fprintf(stderr,"ERROR: Failed to open message queue for %s in log_mgr\n",
						rt_mq_names[mq[i].which]);
			}
			else
			{
				//LLOG(DIAG5,TEMP_mID,
				fprintf(stderr,	"opened %s\n",rt_mq_names[mq[i].which]);
			}
		}
		embeddedInitLogging('L',&mq[LMMQ_TO_MSG]);

		// get log control from last time
		int num_read;
		FILE* lcfile = fopen(LOG_CTL_FILE,"r");
		if(lcfile)
		{
			if((num_read = fread((char*)(&log_ctl[0]),sizeof(log_ctl_node),
							NUM_APP_mIDs,lcfile))!= NUM_APP_mIDs)
			{
				sleep(1);
				fprintf(stderr,
					"read  %d, less than all %d nodes, zero the rest\n",
						num_read, NUM_APP_mIDs);
				bzero(&log_ctl[num_read],
						sizeof(log_ctl_node)*(NUM_APP_mIDs - num_read));
			}
			fclose(lcfile);
		}
		else
		{
			sleep(1);
			fprintf(stderr,"no log_ctl.cfg, zero all levels\n");
			// init all logging levels to show only level 0, or FATAL
			bzero(&log_ctl[0],sizeof(log_ctl_node)*NUM_APP_mIDs);
		}
	
		stop_requested = 0;
		while(!stop_requested)
		{
			int n;
			ssize_t rcv_size;
			// wait for messages from asif_mgr (automation system interface manager)
	
			// variables for setting up select()
			fd_set readfds;
			// fd_set writefds
			// int num_wfds = 0;
			// fd_set exceptfds;
	
	
			// prepare variables used as args to select()
			FD_ZERO(&readfds);
			// FD_ZERO(&writefds);
			// FD_ZERO(&exceptfds);
			int maxfd = 0;
			for(i=0;i<NUM_LMMQ_VALS;i++)
			{
				if((mq[i].rw == rw_read_only) || (mq[i].rw == rw_read_write))
				{
					FD_SET(mq[i].rt_mq->rt_mqueue_descr, &readfds);
					if(mq[i].rt_mq->rt_mqueue_descr > maxfd)
						maxfd = mq[i].rt_mq->rt_mqueue_descr;
				}
				// if((mq[i].rw == rw_write_only) || (mq[i].rw == rw_read_write))
				// {
					// FD_SET(mq[i].rt_mq, &writefds);
					// num_wfds++;
					// if(mq[i].rt_mq > maxfd)
						// maxfd = mq[i].rt_mq;
				// }
			}
			// wait for messages from any of the incoming message queues
			switch ( n = select ( maxfd+1, &readfds, 0, 0, NULL))
			{
			case -1:
				{
				int error_val = errno;
				// LLOG(DIAG5,TEMP_mID,
				fprintf(stderr,	"ERROR: select failed - %d, %s\n",
								error_val, strerror(error_val));
				}
				break;
			case 0:
				//LLOG(DIAG5,TEMP_mID,
				fprintf(stderr,	"select timed out\n");
				break;
			default:
				//LLOG(DIAG5,TEMP_mID,
				//fprintf(stderr,	"%d descriptors ready in select\n",n);
	
				// loop through all message queues used in log_mgr
				// look first for ones with read data ready
				for(i=0;i<NUM_LMMQ_VALS;i++)
				{
					if(FD_ISSET(mq[i].rt_mq->rt_mqueue_descr, &readfds))
					{
						ssize_t rcv_buf_size;
						char * rcv_buf;
						// get size used for this mq's messages
						if(mq[i].rt_mq->get_msg_size(&rcv_buf_size)==FAILURE)
						{
							//LLOG(DIAG5,TEMP_mID,
							 fprintf(stderr, "ERROR: failed to get msg size from rt_mqueue\n");
								abort();
						}
						// and allocate the buffer
						if((rcv_buf = (char *)malloc(rcv_buf_size))==NULL)
						{
							//LLOG(DIAG5,TEMP_mID,
							fprintf(stderr,	"ERROR: failed to malloc for mq buf\n");
							abort();
						}

						//LLOG(DIAG5,TEMP_mID,
						//fprintf(stderr,	"%s has read data pending\n",
									//rt_mq_names[mq[i].which]);
						// get message from rt_mqueue
						rcv_size = mq[i].rt_mq->receive_from_mqueue(rcv_buf);
						// do whatever is indicated in message
						log_proc_rcv_msg(rcv_buf, rcv_size);
						// done with buffer
						free(rcv_buf);
					}
				}
				break;
			}
		}
		// --------------------------- body -----------------------------

	
		// close all of the message queues used
		for(i=0;i<NUM_LMMQ_VALS;i++) 
		{
			delete mq[i].rt_mq;
		}
	}
	// close logging facility
	embeddedStopLogging();

	return(0);
}
