//-----------------------------------------------------------------------------|
//
// msg_mgr.cpp
//
// Copyright(c) 2020, embeddedKeith
//
// msgeloped by: Keith DeWald
//
// started: 8/12/01
//
// last mod: 8/29/01
//-----------------------------------------------------------------------------|
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include <assert.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include "nov_log.hpp"
#include "rt_mqueue.hpp"
#include "rt_errors.hpp"
#include "w2char.hpp"
#include "asif_protocol.hpp"
#include "rtc_config.hpp"
#include "msg_mgr.hpp"

#pragma pack(1)

static int stop_requested = 0;
static int still_trying = 1;

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[3] = "MS";

//-----------------------------------------------------------------------------|
// this array keeps track of just the mqueues msg_mgr uses out of
// all the mqueues in the system
//
#define MSG_MQ WMQ_ALL_TO_MSG
mod_mqueue mq[NUM_MMMQ_VALS] =
{
	// which-mqueue         read/write		block/non-block rt_mqueue pointer
	{ WMQ_ALL_TO_MSG,		rw_write_only,	blk_blocking, NULL}
};
//-----------------------------------------------------------------------------|
// msg_proc_msg_handler
//
void
msg_proc_msg_handler(char * buf, size_t size)
{
	assert(size >= sizeof(mq_msg_header));
	mq_msg_header * p_header = (mq_msg_header *)&buf[0];

	switch(p_header->msg_type)
	{
	// hand_shake is universal to all processes
	case hand_shake:
		//stdPRE "Received mq_msg hand_shake\n" stdPOST
		break;
	// message types sent from asif_mgr
	default:
		// stdPRE "ERROR: unknown message type %d received\n", p_header->msg_type stdPOST
		break;
	}
};
//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	fprintf(stderr,"MSG RECEIVED USR1 SIGNAL %d, restart msg_mgr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	fprintf(stderr,"MSG RECEIVED USR2 SIGNAL %d, stop msg_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
int
main()
{
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

	still_trying = 1;
	stop_requested = 0;

	while(still_trying)
	{
		//
		// instantiate and open all of the message queues to and from other procs
		//
		for(i=0;i<NUM_MMMQ_VALS;i++) 
		{
			if((mq[i].rt_mq = 
				new rt_mqueue(mq[i].which,mq[i].rw,mq[i].blk))==NULL)
			{
				printf(
				"m_ERROR: Failed to open message queue for %s in msg_mgr\n",
								rt_mq_names[mq[i].which]);
			}
		}
		embeddedInitLogging('M',&mq[MMMQ_TO_MSG]);
	
		while(!stop_requested)
		{
	
			int n;
			ssize_t rcv_size;
	
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
			for(i=0;i<NUM_MMMQ_VALS;i++)
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
				fprintf(stderr,"ERROR: select failed - %d, %s\n",
								error_val, strerror(error_val));
				}
				break;
			case 0:
				fprintf(stderr,"select timed out\n");
				break;
			default:
				// fprintf(stderr,"%d descriptors ready in select\n",n);
	
				// loop through all message queues used in sched_mgr
				// look first for ones with read data ready
				for(i=0;i<NUM_MMMQ_VALS;i++)
				{
					if(FD_ISSET(mq[i].rt_mq->rt_mqueue_descr, &readfds))
					{
						ssize_t rcv_buf_size;
						char * rcv_buf;
						// get size used for this mq's messages
						if(mq[i].rt_mq->get_msg_size(&rcv_buf_size)==FAILURE)
						{
							  fprintf(stderr,"ERROR: failed to get "
									  "msg size from rt_mqueue\n");
								abort();
						}
						// and allocate the buffer
						// fprintf(stderr,"receive buffer into buffer size %d\n",
									// rcv_buf_size);
						if((rcv_buf = (char *)malloc(rcv_buf_size))==NULL)
						{
							fprintf(stderr,"ERROR: failed to malloc for mq buf\n" 
								);
							abort();
						}

						// fprintf(stderr,"%s has read data pending\n",
									// rt_mq_names[mq[i].which]);
						// get message from rt_mqueue
						rcv_size = mq[i].rt_mq->receive_from_mqueue(rcv_buf);
						// do whatever is indicated in message
						msg_proc_msg_handler(rcv_buf, rcv_size);
						// done with buffer
						free(rcv_buf);
					}
				}
				break;
			}
		}
	
		// close all of the message queues used
		for(i=0;i<NUM_MMMQ_VALS;i++) 
		{
			delete mq[i].rt_mq;
		}
	}
	return(0);
}
