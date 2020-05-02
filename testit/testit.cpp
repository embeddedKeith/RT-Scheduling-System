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
#define UEYEAR 1970

static	int sockfd;
static int zerocount = 0;

int i;
struct timespec tmspec;
unsigned long event_ids = 40;
w2char_t test_device_id[24];

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[3] = "DE";

// unsigned int mdays[] = 	{ 0,31,28,31,30,31,30,31,31,30,31,30,31 };
unsigned int mtotaldays[]={ 0,31,59,90,120,151,181,212,243,273,304,334,365 };
char * tmnames[] = { "    ","Jan ","Feb ","Mar ","Apr ","May ","Jun ",
					 "Jul ","Aug ","Sep ","Oct ","Nov ","Dec " };
unsigned long tulmnames[13];

// #define TEST_DEVICE_ID "VTR0 in rack number 1"
// #define TEST_DEVICE_ID "PROFILE VIDEO SERVER"
#define TEST_DEVICE_ID "BTS Router Number 1"

const int num_log_ctls = 10;

const embeddedLogControlRequest	mod_log_ctls[num_log_ctls] =
{
	{'X',5,LOG_CONTROL_mID},
//	{'X',4,MALLOC_mID},
//	{'X',4,SOCKET_mID},
//	{'X',4,SOCK_MSG_mID},
//	{'X',4,MS_SOCK_MSG_IN_mID},
//	{'X',4,MS_SOCK_MSG_OUT_mID},
	{'X',4,EVENT_mID},
//	{'X',4,DEV_MSG_IN_mID},
//	{'X',4,DEV_MSG_OUT_mID},
//	{'X',4,SCHED_MSG_IN_mID},
//	{'X',4,SCHED_MSG_OUT_mID},
//	{'X',4,ASIF_MQ_MSG_IN_mID},
//	{'X',4,ASIF_MQ_MSG_OUT_mID},
//	{'X',4,RTMQ_mID},
//	{'X',4,DEV_HOLDER_mID},
	{'X',0,LOG_CONTROL_mID}
};

static int device_count = 0;

void form_time(RTC_NTIMECODE & p_ntc)
{
	char timestr[27];
	long frames;

	clock_gettime(CLOCK_REALTIME, &tmspec);
	ctime_r(&tmspec.tv_sec,timestr);
	timestr[26]=0;
	fprintf(stderr,"form_time got %s\n",timestr);
	*((char *)&p_ntc[6])=0;
	memcpy((void *)&p_ntc[0],(const void *)&timestr[11],2);	// hours
	memcpy((void *)&p_ntc[2],(const void *)&timestr[14],2);	// minutes
	memcpy((void *)&p_ntc[4],(const void *)&timestr[17],2);	// seconds
	frames = tmspec.tv_nsec/33366700L;
	snprintf(&p_ntc[6],2,"%02ld",frames);
};
//-----------------------------------------------------------------------------|
void *
read_response(void * p_varg)
{
	fd_set arfd;
	embeddedHeader ReplyMsg;
	int rcv_size;
	int n;


	while(1)
	{
		// prepare variables used as args to select()
		FD_ZERO(&arfd);
		FD_SET(sockfd ,&arfd);
		
		// wait for messages from connection to automation system(s)
		switch ( n = select (sockfd+1, &arfd, 0, 0, NULL))
		{
		case -1:
			{
				int error_val = errno;
					fprintf(logfile,"ERROR: asif socket select failed with %d, %s\n",error_val,
							strerror(error_val));
			}
			break;
		case 0:
				fprintf(logfile,"select timed out\n");
			break;
		default:
			fprintf(logfile,"%d descriptors ready in select for socket %d\n",n,
							sockfd);

			if(FD_ISSET(sockfd, &arfd))
			{
				{
					// if there is any response, read it
					if((rcv_size = read(sockfd,(void *)&ReplyMsg,
											sizeof(embeddedHeader)))==-1)
					{
						int error_val = errno;
						fprintf(stderr,"ERROR %d on receive from socket, %s\n",error_val,
										strerror(error_val));
					}
					else if(rcv_size != 0)
					{
						size_t size_to_get;
						const int MAX_RESPONSE_SIZE = 4096;
			
						fprintf(stderr,"Received header we think, %d bytes\n",rcv_size);
			
						if(rcv_size == sizeof(embeddedHeader))
						{
							fprintf(stderr,"response header:\n");
							hex_byte_dump_to_file(stderr,(char*)&ReplyMsg,rcv_size);
							fprintf(stderr,"\n");
							// receive rest of message
							size_to_get = ReplyMsg.size - sizeof(embeddedHeader);
							char rbuf[MAX_RESPONSE_SIZE];
							fprintf(stderr,"Will now try to read %d more bytes\n",
											size_to_get);
							if((rcv_size = 
									read(sockfd,(void *)&rbuf[0],size_to_get))==-1)
							{
								int error_val = errno;
								fprintf(stderr,"ERROR %d on receive from socket, %s\n",
												error_val, strerror(error_val));
							}
							else if(rcv_size != 0)
							{
								fprintf(stderr,"response data:\n");
								hex_byte_dump_to_file(stderr,&rbuf[0],rcv_size);
							}
						}
						zerocount = 0;
					}
					else
					{
						if(zerocount < 5)
						{
							fprintf(stderr,"Zero bytes available at socket\n");
							zerocount++;
						}
						else
						{
							fprintf(stderr,
								"Five zero reads, close socket and quit\n");
							close(sockfd);
							exit(1);
						}
					}
				}
			}
			break;
		}
	}
};


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
	unsigned long appl_entity;
	embeddedHeader SimpleMsg;

	pthread_attr_t attr;
	// creating threads in detached state allows them to end without us helping
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	// static int dev_add_count = 0;

	if(argc>1)
		sscanf(argv[1],"%ld",&appl_entity);
	else
		appl_entity = 0;

	fprintf(stderr,"appl_entity is %ld\n",appl_entity);
	enum msg_type
	{
		status_request = 0,
		file_tx_request,
		file_tx_response,
		ack_response,
		shutdown_request,
		restart_request,
		config_request,
		config_response,
		add_device_request,
		delete_device_request,
		add_event_request,
		drop_event_request,
		get_event_request,
	// 	exec_cmd_request,
	// 	exec_cmd_response,
		log_control,
		send_subscription_request
	};
	
	const int NUM_TO_SEND = 10;
	msg_type msgs_to_send[NUM_TO_SEND] = 
	{
		// log_control,
		// status_request,
		add_device_request,
		// config_request,
		// send_subscription_request,
		// add_device_request,
		// config_request,
		// add_device_request,
		// config_request,
		// add_device_request,
		// config_request,
		// delete_device_request,
		// config_request,
		add_event_request,
		add_event_request,
		add_event_request,
		add_event_request,
		add_event_request,
		add_event_request,
		add_event_request,
		add_event_request,
		add_event_request,
		// drop_event_request,
		// drop_event_request,
		// drop_event_request,
		// drop_event_request,
		// delete_device_request,
		// config_request,
		// delete_device_request,
		// config_request,
		// delete_device_request,
		// config_request,
	};

	const int NUM_MSGS = 16;
	unsigned long msg_type[NUM_MSGS] =
	{
		RTC_RT_CONTROLLER_STATUS_REQUEST,	// 0
		RTC_FILE_TRANSFER_TO_RT_REQUEST,	// 1
		RTC_FILE_TRANSFER_TO_RT_RESPONSE,	// 2
		RTC_ACKNOWLEDGEMENT_RESPONSE,		// 3
		RTC_SHUTDOWN_REQUEST,				// 4
		RTC_RESTART_REQUEST,				// 5
		RTC_RT_CONTROLLER_CONFIG_REQUEST,	// 6
		RTC_RT_CONTROLLER_CONFIG_RESPONSE,	// 7
		RTC_RT_CONTROLLER_ADD_REQUEST,		// 8
		RTC_RT_CONTROLLER_DELETE_REQUEST,	// 9
		RTC_ADD_SCHEDULED_EVENT_REQUEST,	// 10
		RTC_DROP_SCHEDULED_EVENT_REQUEST,	// 11
		RTC_GET_SCHEDULED_EVENT_REQUEST,	// 12
		//RTC_EXEC_DEV_COMMAND_REQUEST,		// xx
		//RTC_EXEC_DEV_COMMAND_RESPONSE,	// xx
		RTC_LOG_CONTROL_REQUEST,			// 13
		RTC_DEVICE_STATUS_SUBSCRIPTION 	 	// 14
	};

	bzero((void *)&tulmnames[i],52);
	for(i=0;i<=12;i++)
		memcpy((void *)&tulmnames[i],(const void *)&tmnames[i][0],4);

	const char test_sender[] = "TEST_MS_SENDER=============================";

	bzero(&SimpleMsg.sender,sizeof(RTC_HOSTNAME));
	SimpleMsg.id = 0;
	SimpleMsg.type = RTC_RT_CONTROLLER_CONFIG_REQUEST;
	SimpleMsg.size = sizeof(embeddedHeader);
	int i;

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

	// make socket have timeout of zero for linger
	linger lingerStruct;
	lingerStruct.l_onoff = 1;
	lingerStruct.l_linger = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_LINGER, (char*)&lingerStruct,
					sizeof(lingerStruct));
	
	if(connect(sockfd, (const struct sockaddr *)&als_sin, als_sin_size)==-1)
	{
		int error_val = errno;
		fprintf(stderr,"ERROR %d on connect: %s\n",error_val,
						strerror(error_val));
		abort();
	}
	fprintf(stderr,"sockfd is %d\n",sockfd);

	char_string_to_w2char((w2char_t *)test_device_id,
					TEST_DEVICE_ID,24);

	char_string_to_w2char((w2char_t *)SimpleMsg.sender,(char *)test_sender,16);

	// start thread to wait for messages from other processes
	pthread_create(NULL, &attr, read_response, NULL);
	for(i=0;i<NUM_TO_SEND;i++)
	// for(i=0;i<100000;i++)
	{
		SimpleMsg.id = (unsigned long)i;
		SimpleMsg.appl_entity = appl_entity;
		SimpleMsg.type = msg_type[msgs_to_send[i%NUM_TO_SEND]];
		SimpleMsg.id = i;
		fprintf(stderr,"Set msg type to %ld\n",SimpleMsg.type);

		switch(msg_type[msgs_to_send[i%NUM_TO_SEND]])
		{
		case RTC_RT_CONTROLLER_STATUS_REQUEST:
			{
				fprintf(stderr,"test command RTC_RT_CONTROLLER_STATUS_REQUEST\n");
				test_send(sockfd,(const void *)&SimpleMsg,sizeof(embeddedHeader),0);
			}
			break;
		case RTC_FILE_TRANSFER_TO_RT_REQUEST:
			{
				fprintf(stderr,"test command RTC_FILE_TRANSFER_TO_RT_REQUEST\n");
			}
			break;
		case RTC_SHUTDOWN_REQUEST:
			{
				fprintf(stderr,"test command RTC_SHUTDOWN_REQUEST\n");
			}
			break;
		case RTC_RESTART_REQUEST:
			{
				fprintf(stderr,"test command RTC_RESTART_REQUEST\n");
			}
			break;
		case RTC_RT_CONTROLLER_CONFIG_REQUEST:
			{
				fprintf(stderr,"test command RTC_RT_CONTROLLER_CONFIG_REQUEST\n");
				embeddedHeader cfg_msg;
				bzero(&cfg_msg,sizeof(embeddedHeader));
				cfg_msg = SimpleMsg;
				cfg_msg.size = sizeof(embeddedHeader);
				cfg_msg.type = RTC_RT_CONTROLLER_CONFIG_REQUEST;

				test_send(sockfd,(const void *)&cfg_msg,
							sizeof(embeddedHeader),0);
			}
			break;
		case RTC_RT_CONTROLLER_ADD_REQUEST:
			{
#if 0 // VTR
					fprintf(stderr,"test command RTC_RT_CONTROLLER_ADD_REQUEST\n");
					embeddedRTControllerAddDeviceRequestMsg add_msg;
					bzero(&add_msg,sizeof(embeddedRTControllerAddDeviceRequestMsg ));
					add_msg.header = SimpleMsg;
					add_msg.header.size = sizeof(embeddedRTControllerAddDeviceRequestMsg);
					
					char char_dev_id[24];
					snprintf(char_dev_id,24,TEST_DEVICE_ID "dev_count %d\n",device_count++);
					char char_inet_addr[RTC_INET_ADDR_LENGTH];
					snprintf(char_inet_addr,RTC_INET_ADDR_LENGTH,"200.100.0.64");
	
    				add_msg.request.type = VTR_DEVICE;
					char_string_to_w2char((w2char_t *)&add_msg.request.device_id,
									TEST_DEVICE_ID,24);
					add_msg.request.vtr.type = SonyVTR;
    				add_msg.request.access.wchars.wchar_hi = 0;
					add_msg.request.access.wchars.wchar_lo = 'S';
    				add_msg.request.mode.wchars.wchar_hi =  0;
    				add_msg.request.mode.wchars.wchar_lo = 'M';
					char_string_to_w2char((w2char_t *)&add_msg.request.inet_addr,
									char_inet_addr, RTC_INET_ADDR_LENGTH);
    				add_msg.request.port_number = 1;
    				add_msg.request.port_mode.wchars.wchar_hi = 0;
					add_msg.request.port_mode.wchars.wchar_lo = '4';
    				add_msg.request.baud_rate = 5600;
    				add_msg.request.parity = 1;
    				add_msg.request.stop_bits = 2;
				
					test_send(sockfd,(const void *)&add_msg,
							sizeof(embeddedRTControllerAddDeviceRequestMsg),0);
#endif
#if 0 // SERVER
					fprintf(stderr,"test command RTC_RT_CONTROLLER_ADD_REQUEST\n");
					embeddedRTControllerAddDeviceRequestMsg add_msg;
					bzero(&add_msg,sizeof(embeddedRTControllerAddDeviceRequestMsg ));
					add_msg.header = SimpleMsg;
					add_msg.header.size = sizeof(embeddedRTControllerAddDeviceRequestMsg);
					
					char char_dev_id[24];
					snprintf(char_dev_id,24,TEST_DEVICE_ID "dev_count %d\n",device_count++);
					char char_inet_addr[RTC_INET_ADDR_LENGTH];
					snprintf(char_inet_addr,RTC_INET_ADDR_LENGTH,"200.100.0.64");
	
   		 			add_msg.request.type = VIDEO_SERVER_DEVICE;
	
					char_string_to_w2char((w2char_t *)&add_msg.request.device_id,
									char_dev_id,24);
					
   		 			add_msg.request.access.wchars.wchar_hi = 0;
					add_msg.request.access.wchars.wchar_lo = 'S';
   		 			add_msg.request.mode.wchars.wchar_hi =  0;
   		 			add_msg.request.mode.wchars.wchar_lo = 'M';
					char_string_to_w2char((w2char_t *)&add_msg.request.inet_addr,
									char_inet_addr, RTC_INET_ADDR_LENGTH);
   		 			add_msg.request.port_number = 3;
   		 			add_msg.request.port_mode.wchars.wchar_hi = 0;
					add_msg.request.port_mode.wchars.wchar_lo = '3';
   		 			add_msg.request.baud_rate = 38400;
   		 			add_msg.request.parity = 1;
   		 			add_msg.request.stop_bits = 2;
	
					add_msg.request.video_server.type =  TekVServ;

					add_msg.request.video_server.unit_number =  0;
					add_msg.request.video_server.record_channel  =  1;
					add_msg.request.video_server.play_channel =  1;
					add_msg.request.video_server.control_unit =  0;
					add_msg.request.video_server.record_only =  0;
					add_msg.request.video_server.play_only =  0;
					
					test_send(sockfd,(const void *)&add_msg,
							sizeof(embeddedRTControllerAddDeviceRequestMsg),0);
#endif
#if 1 // ROUTER
					fprintf(stderr,"test command RTC_RT_CONTROLLER_ADD_REQUEST\n");
					embeddedRTControllerAddDeviceRequestMsg add_msg;
					bzero(&add_msg,sizeof(embeddedRTControllerAddDeviceRequestMsg ));
					add_msg.header = SimpleMsg;
					add_msg.header.size = sizeof(embeddedRTControllerAddDeviceRequestMsg);
					
					char char_dev_id[24];
					snprintf(char_dev_id,24,TEST_DEVICE_ID "dev_count %d\n",device_count++);
					char char_inet_addr[RTC_INET_ADDR_LENGTH];
					snprintf(char_inet_addr,RTC_INET_ADDR_LENGTH,"200.100.0.64");
	
   		 			add_msg.request.type = ROUTER_DEVICE;
	
					char_string_to_w2char((w2char_t *)&add_msg.request.device_id,
						char_dev_id,24);
					
   		 			add_msg.request.access.wchars.wchar_hi = 0;
					add_msg.request.access.wchars.wchar_lo = 'S';
   		 			add_msg.request.mode.wchars.wchar_hi =  0;
   		 			add_msg.request.mode.wchars.wchar_lo = 'M';
					char_string_to_w2char((w2char_t *)&add_msg.request.inet_addr,
						char_inet_addr, RTC_INET_ADDR_LENGTH);
   		 			add_msg.request.port_number = device_count;
   		 			add_msg.request.port_mode.wchars.wchar_hi = 0;
					add_msg.request.port_mode.wchars.wchar_lo = '4';
   		 			add_msg.request.baud_rate = 38400;
   		 			add_msg.request.parity = 1;
   		 			add_msg.request.stop_bits = 2;
	
					add_msg.request.video_router.type =  BTS_ROUTER;
					add_msg.request.video_router.protocol = 0;
					add_msg.request.video_router.number_inputs  =  128;
					add_msg.request.video_router.number_outputs =  128;
					add_msg.request.video_router.number_levels =  16;
					bzero((void*)(&add_msg.request.video_router.level_map[0]),
						sizeof(unsigned long)*MAX_ROUTER_LEVELS);
					// assign to same values as Brian's test, arbitrary
					add_msg.request.video_router.level_map[0] = 4;
					add_msg.request.video_router.level_map[1] = 5;
					add_msg.request.video_router.level_map[2] = 6;
					add_msg.request.video_router.level_map[5] = 1;
					add_msg.request.video_router.level_map[6] = 2;
					add_msg.request.video_router.level_map[7] = 3;

					bzero((void*)(&add_msg.request.video_router.level_array[0]),
						sizeof(unsigned long)*MAX_ROUTER_LEVELS);
					// assign to same values as Brian's test, arbitrary
					add_msg.request.video_router.level_array[0] = 4;
					add_msg.request.video_router.level_array[1] = 5;
					add_msg.request.video_router.level_array[2] = 6;
					add_msg.request.video_router.level_array[5] = 1;
					add_msg.request.video_router.level_array[6] = 2;
					add_msg.request.video_router.level_array[7] = 3;

					test_send(sockfd,(const void *)&add_msg,
						sizeof(embeddedRTControllerAddDeviceRequestMsg),0);
#endif
			}
			break;
		case RTC_RT_CONTROLLER_DELETE_REQUEST:
			{
				fprintf(stderr,"test command RTC_RT_CONTROLLER_DELETE_REQUEST\n");
				embeddedRTControllerDeleteDeviceRequestMsg delete_msg;
				bzero(&delete_msg,sizeof(embeddedRTControllerDeleteDeviceRequestMsg ));
				delete_msg.header = SimpleMsg;
				delete_msg.header.size 
						= sizeof(embeddedRTControllerDeleteDeviceRequestMsg);

				char char_dev_id[24];
				snprintf(char_dev_id,24,TEST_DEVICE_ID "dev_count %d\n",--device_count);

				char_string_to_w2char((w2char_t *)&delete_msg.request.device_id,
								char_dev_id,24);

				test_send(sockfd,(const void *)&delete_msg,
							sizeof(embeddedRTControllerDeleteDeviceRequestMsg),0);
			}
			break;
		case RTC_ADD_SCHEDULED_EVENT_REQUEST:
			{
				// values for event types mean:
				//
				//  1:  START_RECORD
				//  2:  START_PLAY
				//  3:  STOP  -  no extra data
				//  4:  PAUSE  -  no extra data
				//  5:  JOG
				//  6:  SHUTTLE
				//  7:  STANDBY 
				//  8:  EJECT -  no extra data
				//  9:  SETUP
				//  20. START_KEY
				//  30: START_CG
				//  40: START_AUDIO
				//  50: SET ROUTER
				//  60: SET MIXER
#define num_dev_events 9
				unsigned long event_types[num_dev_events] = { 
						50, 50, 50, 50, 50, 50, 50, 50, 50 };
				static int aet_count = 0;
				static int cncount = 0;
				const long SEDEL = 10;
				const long SERAND = 5;
				long randval;
				int addsecs;
				fprintf(stderr,"test command RTC_ADD_SCHEDULED_EVENT_REQUEST\n");
				char ntcstr[9];
				RTC_NTIMECODE ntc;
				embeddedAddScheduledEventRequestMsg add_msg;
				add_msg.header = SimpleMsg;
				add_msg.header.size = sizeof(embeddedAddScheduledEventRequestMsg);
				add_msg.request.event_id = event_ids++;
				add_msg.request.device_id = test_device_id;
				add_msg.request.event_type = event_types[aet_count % num_dev_events];
				
				if(add_msg.request.event_type == 50) // set router xpoint cmd
				{
				}
#if 0
				if(add_msg.request.event_type == 7)
					add_msg.request.dev_data.standby.mode = (aet_count%num_dev_events)?0:1;
				if(add_msg.request.event_type == 9 )// || add_msg.request.event_type == 2)
				{
					char clipName[128];
					snprintf(clipName,20,"1-B-076-%c",'3'+cncount++);
					w2char_t w2ClipName[128];
					char_string_to_w2char(w2ClipName,clipName,128);
					add_msg.request.dev_data.play.clip = w2ClipName;
				}

				// if(add_msg.request.event_type == 6)
				// 	add_msg.request.dev_data.shuttle.speed = 3;
#endif
				add_msg.request.immediate = 1;
				form_time(ntc);
				memcpy(ntcstr,&ntc,8);
				ntcstr[8]='\0';
				randval = (int)((float)2.0*(float)SERAND*(float)random()/
						(float)0x7fffffff-(float)SERAND);
				// schedule SEDEL seconds (+/- SERAND seconds) in the future
				addsecs = SEDEL + randval;
				add_secs_to_ntc(&ntc,addsecs);	
				memcpy(ntcstr,&ntc,8);
				ntcstr[8]='\0';
				fprintf(stderr,"added %d secs, tc now %s\n",addsecs,ntcstr);
				add_msg.request.event_time = ntc;
				test_send(sockfd,(const void *)&add_msg,
							sizeof(embeddedAddScheduledEventRequestMsg),0);
				aet_count++;
			}
			break;
		case RTC_DROP_SCHEDULED_EVENT_REQUEST:
			{
				fprintf(stderr,"test command RTC_DROP_SCHEDULED_EVENT_REQUEST\n");
				embeddedDropScheduledEventRequestMsg drop_msg;
				drop_msg.header = SimpleMsg;
				drop_msg.header.size = sizeof(embeddedDropScheduledEventRequestMsg);
				drop_msg.request.event_id = --event_ids;
				test_send(sockfd,(const void *)&drop_msg,
							sizeof(embeddedDropScheduledEventRequestMsg),0);
			}
			break;
		case RTC_GET_SCHEDULED_EVENT_REQUEST:
			{
				fprintf(stderr,"test command RTC_GET_SCHEDULED_EVENT_REQUEST\n");
			}
			break;
		// case RTC_EXEC_DEV_COMMAND_REQUEST:
			// {
				// fprintf(stderr,"test command RTC_EXEC_DEV_COMMAND_REQUEST\n");
			// }
			// break;
		case RTC_LOG_CONTROL_REQUEST:
			{
				fprintf(stderr,"test command RTC_LOG_CONTROL_REQUEST\n");
				embeddedLogControlRequestMsg log_msg;
				log_msg.header = SimpleMsg;
				log_msg.header.size = sizeof(embeddedLogControlRequestMsg);
				for(int ilc=0;ilc<num_log_ctls;ilc++)
				{
					log_msg.request = mod_log_ctls[ilc];
					test_send(sockfd,(const void *)&log_msg,
						sizeof(embeddedLogControlRequestMsg),0);
				}
			}
			break;
		case RTC_DEVICE_STATUS_SUBSCRIPTION:
			{
				fprintf(stderr,"Sending device status subscription\n");
				embeddedDevStatSubscriptionRequestMsg subscr_msg;
				subscr_msg.header = SimpleMsg;
				subscr_msg.header.size = 
						sizeof(embeddedDevStatSubscriptionRequestMsg);
				char_string_to_w2char((w2char_t*)(subscr_msg.request.device_id),
					TEST_DEVICE_ID,RTC_DEVICE_ID_LENGTH);
				subscr_msg.request.subscribe = true;
				test_send(sockfd,(const void*)&subscr_msg,
						sizeof(embeddedDevStatSubscriptionRequestMsg),0);
			}
			break;
		default:
			{
				test_send(sockfd,(const void *)&SimpleMsg,sizeof(embeddedHeader),0);
			}
			break;
		}
/*
		struct timespec sltm;
		sltm.tv_sec = 0;
		sltm.tv_nsec = 500000000;
		nanosleep(&sltm, NULL);
*/
		sleep(5);
	}
	sleep(9);
	close(sockfd);
	return(1);
};
