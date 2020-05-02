//-----------------------------------------------------------------------------|
//
// mqsend.cpp
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
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include "rt_mqueue.hpp"
#include "testdir.hpp"


int main(int argc, char** argv)
{
	mqd_t mqd;
	struct mq_attr mqa;
	en_which_mqueue wq;
	rt_mqueue * p_mq[NUM_WMQ_VALS];

	mqa.mq_maxmsg = 256;
	mqa.mq_msgsize = 256;
	mqa.mq_flags = 0;


	for(wq=(en_which_mqueue)0;wq<NUM_WMQ_VALS; 
					wq = (en_which_mqueue)((int)wq+1))
	{
		if((p_mq[wq] = new rt_mqueue::rt_mqueue(wq,rw_write_only))==NULL)
		{
			printf("ERROR: failed to construct new rt_mqueue");
		}
		else
			printf("Opened mqueue %d - %s\n",p_mq[wq]->rt_mqueue_descr,
							rt_mq_names[wq]);
	}

	if(argc > 1) 
	{
		char mqbuf[256];
		while(mqbuf[0]!=argv[1][0])
		{
			printf("stop when msg char1 = %c\n",argv[1][0]);

			int wharg;
			scanf("%s %d",mqbuf,&wharg);
			wq = (en_which_mqueue)(wharg % NUM_WMQ_VALS);
			mqbuf[strlen(mqbuf)]='\0';
			p_mq[wq]->send_to_mqueue((char *)&mqbuf[0],strlen(mqbuf));
		}
		if(mq_close(mqd)==-1)
		{
			int error_val = errno;
			printf("ERROR: Couldn't close message queue, %d, %s\n",error_val,
					strerror(error_val));
		}
		sleep(15);
		for(wq=(en_which_mqueue)0;wq<NUM_WMQ_VALS;
						wq = (en_which_mqueue)((int)wq+1))
		{
			delete p_mq[wq];
		}
	}
	return(0);
}

