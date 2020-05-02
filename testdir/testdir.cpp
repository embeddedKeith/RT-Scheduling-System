#include <unistd.h>
#include <stdio.h>
#include <mqueue.h>
#include <errno.h>
#include <string.h>
#include "testdir.hpp"


int
main()
{
	mqd_t mqd;
#if 0
	struct mq_attr mqa;

	mqa.mq_maxmsg = 16;
	mqa.mq_msgsize = 128;
	mqa.mq_flags = 0;
#endif

	if((mqd=mq_open("/testq1",O_RDWR | O_CREAT, S_IRUSR | S_IWUSR,NULL))==-1)
	{
		int error_val = errno;
		printf("ERROR: Couldn't create message queue, %d, %s\n",error_val,
				strerror(error_val));
	}
	else
	{
		char mqbuf[4096];
		unsigned int prio;
		char  mqmesg[] = "Test message queue testmq1";
#if 0
		if(mq_send(mqd,mqmesg,strlen(mqmesg),1)==-1)
		{
			int error_val = errno;
			printf("ERROR: Failed on call to mq_receive %d, %s\n",error_val,
					strerror(error_val));
		}

		sleep(30);
#endif
		if(mq_receive(mqd,mqbuf,4096,&prio)==-1)
		{
			int error_val = errno;
			printf("ERROR: Failed on call to mq_receive %d, %s\n",error_val,
					strerror(error_val));
		}
		if(mq_close(mqd)==-1)

		{
			int error_val = errno;
			printf("ERROR: Couldn't close message queue, %d, %s\n",error_val,
					strerror(error_val));
		}
	}

	return(0);
}
