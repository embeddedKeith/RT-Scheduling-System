#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include "log_mgr.hpp"
#include "nov_log.hpp"
#include "rtc_time.hpp"
#include "w2char.hpp"
#include "asif_protocol.hpp"

#define logfile stderr

static	int sockfd;

extern char* mID_name[NUM_APP_mIDs];

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[3] = "DE";

int
test_send(int fd, const void * buf, size_t len, int zero)
{
	fprintf(stderr,"sending %d bytes to socket for asif\n",len);
	hex_byte_dump_to_file(stderr,(char*)buf,len);
	return send(fd,(const void *)buf,len,zero);
};


int 
main(int argc, char** argv)
{
	embeddedLogControlRequestMsg log_msg;
	bzero(&log_msg.header.sender,sizeof(RTC_HOSTNAME));
	log_msg.header.id = 0;
	log_msg.header.app_enty = 33;
	log_msg.header.type = RTC_LOG_CONTROL_REQUEST;
	log_msg.header.size = sizeof(embeddedLogControlRequestMsg);
	int i;
	char module = 'X';
	char priority = '\0';
	unsigned short mod_id = MISC_mID;

	struct sockaddr_in als_sin;
	size_t als_sin_size = sizeof(als_sin);
	als_sin.sin_family = AF_INET;
#ifdef KRD
	als_sin.sin_port = htons(1222);
#else
	als_sin.sin_port = htons(1234);
#endif
	als_sin.sin_addr.s_addr = INADDR_ANY;

	sockfd = socket(AF_INET,SOCK_STREAM,0);

	if(connect(sockfd, (const struct sockaddr *)&als_sin, als_sin_size)==-1)
	{
		int error_val = errno;
		fprintf(stderr,"ERROR %d on connect: %s\n",error_val,
						strerror(error_val));
		abort();
	}
	fprintf(stderr,"sockfd is %d\n",sockfd);

	const char test_sender[] = "LOG_TOOL_SENDER=============================";
	char_string_to_w2char((w2char_t *)log_msg.header.sender,
					(char *)test_sender,16);
	unsigned long id = 1;
	while(module!='q')
	{
		log_msg.header.id = id++;

		fprintf(stderr,"\nmIDs that level can be set for:\n");
		for(i=0;i<NUM_APP_mIDs;i++)
		{
			fprintf(stderr,"%d: %s\n",i,mID_name[i]);
		}
		int mod_id_int, priority_int;
		fprintf(stderr,
			"Enter module letter, mID numeric value and level 0 to 9:\n>");
		scanf("%c %d %d",&module,&mod_id_int,&priority_int);
		if(module=='q')
			break;
		mod_id = (unsigned short)mod_id_int;
		priority = (char)priority_int;
		fprintf(stderr,"Setting level for %s to %d\n",mID_name[mod_id],
						priority);
		log_msg.header.size = sizeof(embeddedLogControlRequestMsg);
		log_msg.request.mod_id = mod_id;
		log_msg.request.priority = priority;
		log_msg.request.module = module;
		test_send(sockfd,(const void *)&log_msg, 
							sizeof(embeddedLogControlRequestMsg),0);
	}
	close(sockfd);
	return(1);
};
