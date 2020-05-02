//-----------------------------------------------------------------------------|
//
// mqrcv.cpp
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/17/01
//
// last mod: 8/17/01
//-----------------------------------------------------------------------------|
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "rt_mqueue.hpp"
#include "testdir.hpp"


int
main(int argc, char** argv)
{
	int i;
	// mqd_t mqd;
	struct mq_attr mqa;
	// ssize_t msg_size;
	en_which_mqueue wq;
	rt_mqueue * p_mq[NUM_WMQ_VALS];
	static int zero_read_count = 0;
	ssize_t size_read = 0;
	ssize_t rcv_size;
	fd_set arfd;
	char namestr[17];


	mqa.mq_maxmsg = 256;
	mqa.mq_msgsize = 256;
	mqa.mq_flags = 0;


	for(wq=(en_which_mqueue)0;wq<NUM_WMQ_VALS; 
					wq = (en_which_mqueue)((int)wq+1))
	{
		if((p_mq[wq] = new rt_mqueue::rt_mqueue(wq,rw_read_only))==NULL)
		{
			printf("ERROR: failed to construct new rt_mqueue");
		}
		else
			printf("Opened mqueue %d\n",p_mq[wq]->rt_mqueue_descr);
	}

	while(1)
	{
		int j;
		int n;
		char buf[256];
		size_t size;
		
		// prepare variables used as args to select()
		FD_ZERO(&arfd);
		int maxfd = 0;
		for(i=0;i<NUM_WMQ_VALS;i++)
		{
			FD_SET(p_mq[i]->rt_mqueue_descr, &arfd);
			if(p_mq[i]->rt_mqueue_descr > maxfd)
				maxfd = p_mq[i]->rt_mqueue_descr;
		}

		// wait for messages from connection to automation system(s)
		switch ( n = select (maxfd+1, &arfd, 0, 0, NULL))
		{
		case -1:
			{
				int error_val = errno;
				printf("<a>ERROR: select failed with %d, %s\n",error_val,
							strerror(error_val));
			}
			break;
		case 0:
			printf("select timed out\n");
			break;
		default:
			printf("%d descriptors ready in select\n",n);

			for(i=0;i<NUM_WMQ_VALS;i++)
			{
				if(FD_ISSET(p_mq[i]->rt_mqueue_descr, &arfd))
				{
					if((rcv_size = p_mq[i]->receive_from_mqueue(buf))==-1)
					{
						printf("ERROR: rcv_size was -1\n");
						abort();
					}
	
					printf("received message size %d on mqueue %d with data:\n",
										rcv_size,p_mq[i]->rt_mqueue_descr);
					for(j=0;j<rcv_size;j++)
						printf("%02x%c",(unsigned int)buf[j],
										((j%16)==15)?'\n':' ');
					printf("\n");
				}
			}
			break;
		}
	}
	

	for(wq=(en_which_mqueue)0;wq<NUM_WMQ_VALS;
					wq = (en_which_mqueue)((int)wq+1))
	{
		delete p_mq[wq];
	}

	return(0);
}
