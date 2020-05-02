#ifdef KRD
#define LOG_FILE "/tmp/krd_asif_mgr.log"
#else
#define LOG_FILE "/tmp/asif_mgr.log"
#endif
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, Keith DeWald, All rights reserved.
//-----------------------------------------------------------------------------|
//
//	Module:	asif_mgr
//
//	Description:	
//
//	FileName:	asif_mgr.cpp
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       02/6/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <sys/neutrino.h>
#include "osinc.hpp"
#include "rt_errors.hpp"
#include "asif_protocol.hpp"
#include "rtc_config.hpp"
#include "w2char.hpp"
#include "nov_log.hpp"
#include "asif_mgr.hpp"
#include "rt_mqueue.hpp"

FILE* logfile = stderr;

#pragma pack(1)


// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[4] = "AS";
#undef ACK_EVERYTHING_FROM_MS

static int stop_requested = 0;
static int still_trying = 1;

static w2char_t this_sender[16];

extern char* mq_user_name[num_mq_user_vals];

// little struct to hold this pointer and void * arg so wrapper
// function can start member function for pthread_create.
struct
thread_entry_struct
{
	void * this_ptr;
	void * p_vargs;
};

#define SIMPLE_ACK_SIZE sizeof(embeddedHeader)

#define LOOKUP_NAME_OK 
#ifdef LOOKUP_NAME_OK 
struct
type_string_tbl_entry
{
	int type_num;
	char * type_string;
};

type_string_tbl_entry type_string_tbl[] = {
		{1024,"RTC_RT_CONTROLLER_STATUS_REQUEST"},
		{1025,"RTC_RT_CONTROLLER_STATUS_RESPONSE"},
		{1032,"RTC_FILE_TRANSFER_TO_RT_REQUEST"},
		{1033,"RTC_FILE_TRANSFER_TO_RT__RESPONSE"},
		{1065,"RTC_ACKNOWLEDGEMENT_RESPONSE"},
		{1064,"RTC_SHUTDOWN_REQUEST"},
		{1066,"RTC_RESTART_REQUEST"},
		{1080,"RTC_RT_CONTROLLER_CONFIG_REQUEST"},
		{1081,"RTC_RT_CONTROLLER_CONFIG_RESPONSE"},
		{1100,"RTC_RT_CONTROLLER_ADD_REQUEST"},
		{1124,"RTC_RT_CONTROLLER_DELETE_REQUEST"},
		{1200,"RTC_ADD_SCHEDULED_EVENT_REQUEST"},
		{1201,"RTC_DROP_SCHEDULED_EVENT_REQUEST"},
		{1202,"RTC_GET_SCHEDULED_EVENT_REQUEST"},
		{1280,"RTC_STATUS_REQUEST"}
};
#endif

#define NumTypes 15
static embeddedHeader simple_ack_msg;
static unsigned long send_ms_msg_id = 0;

unsigned long next_send_ms_msg_id()
{
	return(++send_ms_msg_id);
};

//-----------------------------------------------------------------------------|
// get_next_msg_id -- atomic function to get next message id for a mqueue 
// message so that all messages can have consecutive message ids across all 
// mqueue messages
//
static pthread_mutex_t  msg_id_lock;
static _Uint32t running_msg_id_val = 0;

void init_msg_id_lock()
{
	int result = 0;
	pthread_mutex_unlock(&msg_id_lock);
	pthread_mutex_destroy(&msg_id_lock);
	if((result = pthread_mutex_init(&msg_id_lock,NULL))!=EOK)
	{
		ALOG(WARN,LOCK_mID,"ERROR: pthread_mutex_init returned %d, not EOK\n", 
				result);
		abort();
	}
};

_Uint32t get_next_msg_id()
{
	_Uint32t retval = 0;

	pthread_mutex_lock(&msg_id_lock);
	retval = running_msg_id_val++;
	pthread_mutex_unlock(&msg_id_lock);
	return(retval);
};

//-----------------------------------------------------------------------------|
// process private storage and objects 
//
#ifdef KRD
const int MS_PORT_NUMBER = 1222;
#else
const int MS_PORT_NUMBER = 1234;
#endif

// this is a set of flags for controlling shutdown of threads or process
struct
{
	bool asif_socks;
	bool sched_mqs;
	bool stat_mqs;
	bool dev_mqs;
	bool process;
} finished;

// this array keeps track of just the mqueues asif_mgr uses out of
// all the mqueues in the system
//
mod_mqueue mq[NUM_AMMQ_VALS] =
{
	// which-mqueue         read/write		block/non-block	rt_mqueue pointer
	{ WMQ_ALL_TO_MSG,		rw_write_only,	blk_blocking,	NULL},
	{ WMQ_ASIF_TO_DEV, 		rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_DEV_TO_ASIF, 		rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_ASIF_TO_SCHED, 	rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_SCHED_TO_ASIF, 	rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_ASIF_TO_STAT, 	rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_STAT_TO_ASIF, 	rw_read_only, 	blk_blocking,	NULL}
};

// one socket manager for MS connections
static ms_sockets_mgr	sock_mgr(MS_PORT_NUMBER,MAX_MS_CONNECTIONS);	


char * 
lookup_type_name(int type_number)
{
#ifdef LOOKUP_NAME_OK 
	int i = 0;
	while((type_string_tbl[i].type_num != type_number) && (i<NumTypes))
			i++;
	ALOG(DIAG5,MISC_mID,"found or end of table with i at %d\n",i);
	return((i<NumTypes)?type_string_tbl[i].type_string:NULL);
#else
	return(NULL);
#endif
}

//-----------------------------------------------------------------------------|
// process_mq_msg
//
void
process_mq_msg(char * buf, size_t size)
{
	mq_msg_header * p_header = (mq_msg_header *)&buf[0];
	// mq_msg_header * p_reply_header;

	assert(size >= sizeof(mq_msg_header));

	switch(p_header->msg_type)
	{
	// hand_shake is universal to all processes
	case hand_shake:
		ALOG(DIAG2,HAND_SHAKE_mID,"Received mq_msg hand_shake\n");
		break;
	// message types sent from asif_mgr
	case request_system_status:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_system_status\n");
		break;
	case request_all_devices_status:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_all_devices_status\n");
		break;
	case request_one_device_status:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_one_device_status\n");
		break;
	case send_port_device_assignment:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_port_device_assignment\n");
		break;
	case request_timecode:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_timecode\n");
		break;
	case request_all_event_brief:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_all_event_brief\n");
		break;
	case request_one_event:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_one_event\n");
		break;
	case send_add_event:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_add_event\n");
		break;
	case send_drop_event:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_drop_event\n");
		break;
	case send_device_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_device_config\n");
		break;
	case send_time_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_time_config\n");
		break;
	case send_system_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_system_config\n");
		break;
	case send_port_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_port_config\n");
		break;
	case send_log_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_log_config\n");
		break;
	case request_port_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_port_config\n");
		break;
	case request_port_device_assignment:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_port_device_assignment\n");
		break;
	// message types sent from dev_mgr
	case reply_with_all_devices_status:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_all_devices_status\n");
		{
			size_t resp_size;
			ssize_t size_sent;
			int dev_count;
			mq_all_dev_reply_msg * p_dev_response;
			embeddedRTConfigurationResponseMsg * p_response;

			// get connection for this cookie
			ms_connection * p_conn;
			if((p_conn = sock_mgr.get_appl_entity_connection(
					(unsigned long)(p_header->cookie)))==NULL)
			{
				//log something
				ALOG(FATAL,SOCKET_mID,"Failed to get connection for "
								"appl_entitity cookie from dev_mgr, %d\n",
								p_header->cookie);
			}
			else
			{
				// good connection, send response
				p_dev_response = (mq_all_dev_reply_msg *)p_header;
				dev_count = p_dev_response->device_count;
				ALOG(DIAG3,DEV_CONF_mID,
							"device count in msg is %d\n", dev_count);
				resp_size = sizeof(embeddedHeader) + sizeof(unsigned long)
							+ dev_count * sizeof(Device);	
				ALOG(DIAG3,DEV_CONF_mID,"resp_size is %d\n",resp_size);
	
				p_response = NULL;
				ALOG(WARN,MALLOC_mID,
					"mallocing %d bytes for all_dev_config_resp\n", resp_size);
				p_response = (embeddedRTConfigurationResponseMsg *)malloc(resp_size);
				if(p_response==NULL)
				{
					AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
						"ERROR: couldn't %d bytes in process_mq_msg\n",
						resp_size);
				}
				else
				{
					// copy all but the header into the socket message,
					if(resp_size > (sizeof(embeddedHeader) + sizeof(unsigned long)))
					{
						memmove(&p_response->response,&p_dev_response->response,
							resp_size - sizeof(embeddedHeader));
					}
					p_response->response.record_size = 
							sizeof(Device) * dev_count;
		
					p_response->header.sender = this_sender;
					p_response->header.id = get_next_msg_id();
					p_response->header.type = RTC_RT_CONTROLLER_CONFIG_RESPONSE;
					p_response->header.size = resp_size;
		
					ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
						"cookie retrieved is %p\n",p_header->cookie);
					ALOG(DIAG2,MS_SOCK_MSG_OUT_mID,
					"Sending device configuration response to socket client\n");
					if((size_sent = (p_conn->send_msg_to_ms_connection(
						(char *)p_response,resp_size)) != (ssize_t)resp_size))
					{
						ALOG(DIAG2,RTMQ_mID,
							"ERROR: tried sending %d bytes to connection, "
							"sent %d\n", resp_size,size_sent);
					}
					else
						ALOG(DIAG3,MS_SOCK_MSG_OUT_mID,
							"sent all dev config response to client\n");
					free(p_response);
					p_response = NULL;
				}
			}
		}
		break;
	case send_event_device_status:
		ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
						"Received mq_msg send_event_device_status\n");
		{
			size_t resp_size;
			ssize_t size_sent;
			mq_send_event_device_status_msg * p_dev_response;
			embeddedScheduledStatusResponseMsg * p_response;

			p_dev_response = (mq_send_event_device_status_msg *)p_header;
			resp_size = sizeof(embeddedScheduledStatusResponseMsg);

			p_response = NULL;
			ALOG(DIAG3,MALLOC_mID,
					"mallocing %d bytes for send_event_device_status\n",
							resp_size);
			if((p_response = (embeddedScheduledStatusResponseMsg *)malloc(
						resp_size))==NULL)
			{
				AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
					"ERROR: couldn't %d bytes in process_mq_msg\n",resp_size);
				abort();
			}
			// copy all but the header into the socket message,
			p_response->response = p_dev_response->response;

			p_response->header.sender = this_sender;
			p_response->header.id = get_next_msg_id();
			p_response->header.type = RTC_STATUS_RESPONSE;
			p_response->header.size = resp_size;

			ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
						"cookie retrieved is %p\n",p_header->cookie);
			// get connection for this cookie
			ms_connection * p_conn;
			if((p_conn = sock_mgr.get_appl_entity_connection(
							(unsigned long)(p_header->cookie)))==NULL)
			{
				ALOG(DIAG2,EVENT_mID,"Got event status who's intended"
								" owner is not connected, no action\n");
			}
			else
			{
				// char hexstr[1024];
				// hex_byte_dump_to_string(hexstr,1024,(char*)p_response,
						// sizeof(embeddedScheduledStatusResponseMsg) - 160, 16);
				// ALOG(CONVFP,TEMP_mID,"status message dump: %s\n",hexstr);

				ALOG(DIAG2,MS_SOCK_MSG_OUT_mID,
				"Sending event status response to socket client\n");
				if((size_sent = (p_conn->send_msg_to_ms_connection(
						(char *)p_response,resp_size)) != (ssize_t)resp_size))
				{
					ALOG(WARN,RTMQ_mID,
					"ERROR: tried sending %d bytes to connection, sent %d\n",
								resp_size,size_sent);
				}
			}
			free(p_response);
			p_response = NULL;
		}
		break;
	// message types sent from log_mgr
	case reply_with_log_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_log_config\n");
		break;
	// message types sent from msg_mgr

	// message types sent from proc_mgr
	case request_process_to_shutdown:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_process_to_shutdown\n");
		break;
	case request_process_to_start_operations:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
				"Received mq_msg request_process_to_start_operations\n");
		break;
	case request_process_to_suspend_operations:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
				"Received mq_msg request_process_to_suspend_operations\n");
		break;
	case request_process_to_reinit:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_process_to_reinit\n");
		break;

	// message types sent from sched_mgr
	case reply_with_all_event_structs:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_all_event_structs\n");
		break;
	case reply_with_one_event_struct:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_one_event_struct\n");
		break;
	case reply_with_next_event_time:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_next_event_time\n");
		break;
	case reply_with_latest_event_time:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_latest_event_time\n");
		break;
	case reply_with_number_of_events:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_number_of_events\n");
		break;
	case reply_with_all_one_device_event_structs:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
				"Received mq_msg reply_with_all_one_device_event_structs\n");
		break;
	case reply_with_total_event_capacity:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
				"Received mq_msg reply_with_total_event_capacity\n");
		break;
	case request_all_device_status:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_all_device_status\n");
		break;
	case request_current_timecode:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg request_current_timecode\n");
		break;
	case send_device_control_data:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg send_device_control_data\n");
		break;
	
	// message types sent from stat_mgr
	case reply_with_system_status:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_system_status\n");
		break;
	case reply_with_system_config:
		break;
	
	// message types sent from time_mgr
	case announce_vi_interrupt_with_tc_val:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
				"Received mq_msg announce_vi_interrupt_with_tc_val\n");
		break;
	case reply_with_timecode:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_timecode\n");
		break;
	case reply_with_time_config:
	ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,"Received mq_msg reply_with_time_config\n");
		break;
	default:
		ALOG(DIAG2,ASIF_MQ_MSG_IN_mID,
			"ERROR: unknown message type %d received\n",p_header->msg_type);
		break;
	}
};

//-----------------------------------------------------------------------------|
// ms_connection::ms_connection 
// 
ms_connection::ms_connection(int new_sock_fd)
{
	sock_fd = new_sock_fd;
	ALOG(DIAG2,SOCKET_mID,"saving new sock_fd of %d\n",sock_fd);
	ALOG(WARN,SOCKET_mID,
		"********************* opened new connection **********************\n");

};

//-----------------------------------------------------------------------------|
// ms_connection::~ms_connection 
// 
ms_connection::~ms_connection()
{
	sock_mgr.remove_ms_connection(this);
	ALOG(WARN,SOCKET_mID,
		"********************* closed connection **********************\n");
};

//-----------------------------------------------------------------------------|
// ms_connection::print_sock_fd 
// 
void ms_connection::print_sock_fd()
{
	ALOG(DIAG2,SOCKET_mID,"printing sock_fd as %d\n",sock_fd);
};

//-----------------------------------------------------------------------------|
// ms_connection::shutdown_ms_connection 
//
void ms_connection::shutdown_ms_connection()
{
	// set linger to off so socket will close immediately
	linger sock_linger;
	sock_linger.l_onoff = 0;
	sock_linger.l_linger = 0;
	if(setsockopt(sock_fd,SOL_SOCKET,SO_LINGER,&sock_linger,sizeof(linger))==-1)
	{
		int error_val = errno;
		ALOG(FATAL,SOCKET_mID,
			"Failed to set SO_LINGER on socket %d, error %d: %s\n",
				sock_fd,error_val,strerror(error_val));
	}
	if(shutdown(sock_fd,2)==-1)
	{
		int error_val = errno;
		ALOG(FATAL,SOCKET_mID, "Failed to shutdown socket %d, error %d: %s\n",
				sock_fd,error_val,strerror(error_val));
	}
	if(close(sock_fd)==-1)
	{
		int error_val = errno;
		ALOG(FATAL,SOCKET_mID,
			"ERROR: close(sock_fd) failed with errno %d, %s\n",
						error_val, strerror(error_val));
	}
	delete this;
	pthread_exit(NULL);
};

//-----------------------------------------------------------------------------|
// ms_connection::read_from_ms_connection
//
// called to get specific known quantity of data from ms_connection
// continues to read and wait until that amount is received, may add
// timeout later (TODO)
ssize_t ms_connection::read_from_ms_connection(char * buf, size_t size)
{
	static int zero_read_count = 0;
	ssize_t size_read = 0;
	ssize_t rcv_size;
	fd_set arfd;
#if 0
	char namestr[17];
#endif


	while(size_read < (ssize_t)size)
	{
		int n;
		
		// prepare variables used as args to select()
		FD_ZERO(&arfd);
		FD_SET(sock_fd ,&arfd);

		ALOG(DIAG5,MS_SOCK_MSG_IN_mID,"select on socket got something\n");

		// wait for messages from connection to automation system(s)
		switch ( n = select (sock_fd+1, &arfd, 0, 0, NULL))
		{
		case -1:
			{
				int error_val = errno;
				ALOG(WARN,SOCKET_mID,
					"ERROR: asif socket select failed with %d, %s\n",error_val,
							strerror(error_val));
			}
			break;
		case 0:
			ALOG(DIAG5,SOCKET_mID,
				"select timed out\n");
			break;
		default:
			ALOG(DIAG5,SOCKET_mID, "%d descriptors ready in select\n",n);
			// QLOG(DIAG5,SOCKET_mID, "%d descriptors ready in select\n",n);

			if(FD_ISSET(sock_fd, &arfd))
			{
				ALOG(DIAG5,SOCKET_mID, "asif socket has read data pending\n");
				rcv_size = recv(sock_fd, (void*)&buf[size_read], 
												(size_t)(size-size_read), 0);

				size_read += rcv_size;

				if(rcv_size==-1)
				{
					int error_val = errno;
					ALOG(SHOW,SOCK_MSG_mID,
						"ERROR: socket read returned -1 and error %d, %s\n",
						error_val, strerror(error_val));
					if(error_val==ECONNRESET)
					{
						// kill off this connection and its thread
						ALOG(SHOW,SOCKET_mID,
						"got connection reset by peer message,"
						"close socket and thread.\n");
						shutdown_ms_connection();
					}
					else
						abort();
				}
				else if (rcv_size==0)
				{
					if(++zero_read_count > 5)
					{
						// kill off this connection and its thread
						ALOG(SHOW,SOCKET_mID,
						"got 5  zero size reads, close socket and thread.\n");
						shutdown_ms_connection();
					}
				}
				else
				{
					char hexstr[8192];
					hex_byte_dump_to_string(hexstr,5122,buf,rcv_size,31);
					ALOG(DIAG5,MS_SOCK_MSG_IN_mID,
						"received message size %d on socket %d",
											rcv_size,sock_fd);
					ALOG(DIAG5,MS_SOCK_IN_HEX_mID," with data:\n%s\n",hexstr);
					zero_read_count = 0;
				}
			}
			break;
		}
	}
	return(size_read);
};

//-----------------------------------------------------------------------------|
// ms_connection::send_msg_to_ms_connection 
//
ssize_t ms_connection::send_msg_to_ms_connection(char * buf, size_t size)
{
	ssize_t size_sent = 0;

	char hexstr[5124];
	hex_byte_dump_to_string(hexstr,5122,buf,size,31);
	ALOG(DIAG4,SOCK_MSG_mID,"sending %d bytes to socket:\n",size);
	ALOG(DIAG5,MS_SOCK_OUT_HEX_mID,"%s\n",hexstr);

	if((size_sent = send(sock_fd,(void *)buf,size,0))
			!=(ssize_t)size)
	{
		ALOG(DIAG5,SOCKET_mID,
			"ERROR: send failed in send_msg_to_ms_connection\n");
	}
	return(size_sent);
};

//-----------------------------------------------------------------------------|
// ms_connection::get_complete_ms_msg 
//
ssize_t ms_connection::get_complete_ms_msg(ms_raw_message * p_buf, size_t size)
{
	ssize_t size_read = 0;

	if((size_read = read_from_ms_connection(
								(char *)&p_buf->msg_header, sizeof(embeddedHeader)))
		!=sizeof(embeddedHeader))
	{
		ALOG(DIAG5,SOCKET_mID,
			"ERROR: couldn't read message header in get_complete_ms_msg\n");
	}
	else 
	{
#ifdef ACK_EVERYTHING_FROM_MS
		{
			ssize_t size_sent = 0;
			simple_ack_msg.id = next_send_ms_msg_id();
			if((size_sent = send_msg_to_ms_connection(
					(char *)&simple_ack_msg, SIMPLE_ACK_SIZE))
							!=SIMPLE_ACK_SIZE)
			{
				ALOG(DIAG5,SOCKET_mID,
					"ERROR: sending simple_ack_msg in get_complete_ms_msg\n");
			}
		}
#endif
		ALOG(DIAG5,SOCKET_mID,
			"In get_complete_ms_msg type is %ld and size is %ld\n",
						p_buf->msg_header.type,p_buf->msg_header.size);

		if(p_buf->msg_header.size > sizeof(embeddedHeader))
		{
			if(p_buf->msg_header.size > size)
			{
				ALOG(CONVFP,SOCKET_mID,
					"message size is %ld, larger than buffer, don't read it!\n",
						p_buf->msg_header.size);
				return -1;
			}
			if((size_read += read_from_ms_connection(
				&((char *)p_buf)[sizeof(embeddedHeader)],
				((int)p_buf->msg_header.size - (int)sizeof(embeddedHeader))))
					!= (int)p_buf->msg_header.size)
			{
				ALOG(DIAG5,SOCKET_mID,
					"couldn't read the rest of the message\n");
			}
		}

	}
#if 0
	{
		char hexstr[5122];
		hex_byte_dump_to_string(hexstr,5122,(char*)(&p_buf[0]),size_read,16);
		ALOG(DIAG3,MS_SOCK_MSG_IN_mID,
			"dump of message received from socket connection %d: %s\n",
				sock_fd,hexstr);
	}
#endif

	return(size_read);
};

//-----------------------------------------------------------------------------|
// ms_connection::process_complete_ms_msg 
//
int ms_connection::process_complete_ms_msg(ms_raw_message * p_buf, size_t size)
{
	int error_code = SUCCESS;

	embeddedHeader * p_header = &p_buf->msg_header;

#ifdef LOOKUP_NAME_OK
	char * p_name;
	if((p_name = lookup_type_name(p_header->type))!= NULL)
	{
		ALOG(DIAG5,SOCKET_mID,
			"Got message %s command type %ld from MS\n", p_name, 
					p_header->type);
	}
#endif
	// check appl_entity, if changed, warn and save change
	if(p_header->appl_entity != this->appl_entity)
	{
		ALOG(WARN,SOCK_MSG_mID,
			"appl_entity for connection changed from %ld to %ld\n",
			this->appl_entity, p_header->appl_entity);
		this->appl_entity = p_header->appl_entity;
		// make sure no other open socket has this appl_entity
		for(int iae=0;iae<sock_mgr.num_ms_connections;iae++)
		{
			if(	(sock_mgr.ms[iae]->appl_entity == this->appl_entity) &&
				(sock_mgr.ms[iae] != this))
			{
				AERR(NE_CONN_ERR,WARN,SOCK_MSG_mID,
							"ERROR: connections have same appl_entity\n");
				// shutdown this connection, it is a duplicate
				shutdown_ms_connection();
				error_code = FAILURE;
			}
		}
	}
	// do whatever is needed for this message
	if(error_code==SUCCESS) 
	{
	switch(p_header->type)
	{
	case RTC_RT_CONTROLLER_STATUS_REQUEST:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_RT_CONTROLLER_STATUS_REQUEST\n");

		size_t resp_size;
		ssize_t size_sent;

		char hwdesc[80];
		char swdesc[80];
		bzero(hwdesc,80);
		bzero(swdesc,80);
		snprintf(hwdesc,80,"test hardware description");
		snprintf(swdesc,80,"test software description");
		embeddedRTControllerResponseMsg * p_response;

		resp_size = sizeof(embeddedRTControllerResponseMsg);
		p_response = (embeddedRTControllerResponseMsg *)malloc(resp_size);
		if(p_response==NULL)
		{
			AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
				"ERROR: malloc %d bytes in process_mq_msg\n",resp_size);
		}
		else
		{
			p_response->header.sender = this_sender;
			p_response->header.id = get_next_msg_id();
			p_response->header.type = RTC_RT_CONTROLLER_STATUS_RESPONSE;
			p_response->header.size = resp_size;
	
			p_response->response.time_code_status.wchars.wchar_hi = 0;
			p_response->response.time_code_status.wchars.wchar_lo = 'Y';;
			p_response->response.network_card_count = 1;
			char_string_to_w2char(p_response->response.hardware_desc, 
				hwdesc,80);	
			char_string_to_w2char(p_response->response.software_desc, 
				swdesc,80);	
			p_response->response.major = 1;
			p_response->response.minor = 0;
			ALOG(DIAG2,MS_SOCK_MSG_OUT_mID,
					"Sending controller status response to socket client\n");
			if((size_sent = 
					send_msg_to_ms_connection((char *)p_response,resp_size))
				!= (ssize_t)resp_size)
			{
				ALOG(DIAG2,MS_SOCK_MSG_OUT_mID,
					"ERROR: tried sending %d bytes to connection, sent %d\n",
								resp_size,size_sent);
			}
			free(p_response);
			p_response = NULL;
		}
		break;
	case RTC_RT_CONTROLLER_STATUS_RESPONSE:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_RT_CONTROLLER_STATUS_RESPONSE\n");
		// ERROR
		break;
	case RTC_FILE_TRANSFER_TO_RT_REQUEST:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_FILE_TRANSFER_TO_RT_REQUEST\n");
		break;
	case RTC_FILE_TRANSFER_TO_RT_RESPONSE:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_FILE_TRANSFER_TO_RT_RESPONSE\n");
		// ERROR
		break;
	case RTC_ACKNOWLEDGEMENT_RESPONSE:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_ACKNOWLEDGEMENT_RESPONSE\n");
		// ERROR
		break;
	case RTC_SHUTDOWN_REQUEST:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_SHUTDOWN_REQUEST\n");
		break;
	case RTC_RESTART_REQUEST:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_RESTART_REQUEST\n");
		break;
	case RTC_RT_CONTROLLER_CONFIG_REQUEST:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_RT_CONTROLLER_CONFIG_REQUEST\n");
	{
		mq_msg_header * p_request_msg = new mq_msg_header;
		ssize_t size_sent;

		// build up content of req_all_dev_status message
		*p_request_msg = req_all_dev_status_msg;
		p_request_msg->msg_id = mq_get_next_msg_id();
		// Pack along appl_entity so reply can be routed to right connection
		p_request_msg->cookie = (void *)p_header->appl_entity;

		if((size_sent = mq[AMMQ_TO_DEV].rt_mq->send_to_mqueue(
			(char *)p_request_msg, (ssize_t)sizeof(mq_msg_header)))
						!=sizeof(mq_msg_header))
		{
			ALOG(DIAG2,RTMQ_mID,"ERROR: size sent %d instead of %d in "\
							"process_complete_ms_msg\n",size_sent,
							sizeof(mq_msg_header));
		}
		delete p_request_msg;
	}
		break;
	case RTC_RT_CONTROLLER_ADD_REQUEST: 
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_RT_CONTROLLER_ADD_REQUEST\n");
	{
		mq_send_add_device_msg * p_request_msg;
		embeddedRTControllerAddDeviceRequestMsg * p_add_msg
				= (embeddedRTControllerAddDeviceRequestMsg *)p_header;

		ssize_t msg_size = sizeof(mq_send_add_device_msg);
		ssize_t size_sent;

		if((p_request_msg = (mq_send_add_device_msg *)malloc(msg_size))==NULL)
		{
			AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
				"ERROR: malloc failed for mq_msg for add_device message\n");
		}
		else
		{
			// build up content of send_add_device message
			p_request_msg->header = send_add_device_msg;
			p_request_msg->header.msg_id = mq_get_next_msg_id();
	
			// copy message data into mq message
			p_request_msg->request = p_add_msg->request;
	
			if((size_sent = mq[AMMQ_TO_DEV].rt_mq->send_to_mqueue(
				(char *)p_request_msg, msg_size)) !=sizeof(mq_msg_header))
			{
				ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"ERROR: size sent %d instead of %d in "\
								"process_complete_ms_msg\n",size_sent,msg_size);
			}
			free(p_request_msg);
			p_request_msg = NULL;
		}
	}
		break;
	case RTC_RT_CONTROLLER_DELETE_REQUEST: 
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_RT_CONTROLLER_DELETE_REQUEST\n");
	{
		mq_send_delete_device_msg * p_request_msg;
		embeddedRTControllerDeleteDeviceRequestMsg * p_delete_msg
				= (embeddedRTControllerDeleteDeviceRequestMsg *)p_header;

		ssize_t msg_size = sizeof(mq_send_delete_device_msg);
		ssize_t size_sent;

		if((p_request_msg=(mq_send_delete_device_msg *)malloc(msg_size))==NULL)
		{
			AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
				"ERROR: malloc failed for mq_msg for delete_device message\n");
		}
		else
		{
			// build up content of send_delete_device message
			p_request_msg->header = send_delete_device_msg;
			p_request_msg->header.msg_id = mq_get_next_msg_id();
	
			// copy message data into mq message
			p_request_msg->request = p_delete_msg->request;
	
			if((size_sent = mq[AMMQ_TO_DEV].rt_mq->send_to_mqueue(
				(char *)p_request_msg, msg_size)) !=sizeof(mq_msg_header))
			{
				ALOG(DIAG2,RTMQ_mID,"ERROR: size sent %d instead of %d in "\
								"process_complete_ms_msg\n",size_sent,msg_size);
			}
			free(p_request_msg);
			p_request_msg = NULL;
		}
	}
		break;
	case RTC_ADD_SCHEDULED_EVENT_REQUEST:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_ADD_SCHEDULED_EVENT_REQUEST\n");
	{
		// pointer to mq message to sched_mgr we will build, malloc'd below
		mq_send_add_event_msg * p_request_msg;
		// struct for message we are processing from MS
		embeddedAddScheduledEventRequestMsg * p_add_msg
				= (embeddedAddScheduledEventRequestMsg *)p_header;

		ssize_t msg_size = sizeof(mq_send_add_event_msg); 
		ssize_t size_sent;

		if((p_request_msg = (mq_send_add_event_msg *)malloc(msg_size))==NULL)
		{
			AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
				"ERROR: malloc failed for mq_msg for add_event message\n");
		}
		else
		{
			// build up content of send_add_event message
			p_request_msg->header = send_add_event_msg;
			p_request_msg->header.msg_id = mq_get_next_msg_id();
			// Pack along appl_entity so reply can be routed to right connection
			p_request_msg->header.cookie = (void *)p_header->appl_entity;
	
			p_request_msg->owner_id = p_header->sender;
			// copy message data into mq message
			p_request_msg->request = p_add_msg->request;
	
			{
				char idstr[32];
				w2char_string_to_char(&p_add_msg->request.device_id[0],idstr,24);
#if 0
				char hexstr[5124];
				hex_byte_dump_to_string(hexstr,5122,
					(char *)(&p_add_msg->request),
					sizeof(embeddedAddScheduledEventRequest),16);
#endif
				ALOG(DIAG3,RECORD_DEVICE_mID,
				 	"Received add event with device id %s and event_type %ld\n",
				 	idstr,p_request_msg->request.event_type);
#if 0
				hex_byte_dump_to_string(hexstr,5122,
					(char *)(&p_request_msg->request),
					sizeof(embeddedAddScheduledEventRequest),16);
#endif
				w2char_string_to_char(&p_request_msg->request.device_id[0],idstr,24);
				ALOG(DIAG3,RECORD_DEVICE_mID,
				 	"Sending add event with device id %s and event_type %ld\n",
				 	idstr,p_request_msg->request.event_type);
				// QLOG(FATAL,TEMP_mID,"Sending add event with device id %s and event_type %ld\n",
				 // 	idstr,p_request_msg->request.event_type);
			}
	
			// send message to dev_mgr to send back device config
	
			if((size_sent = mq[AMMQ_TO_SCHED].rt_mq->send_to_mqueue(
				(char *)p_request_msg, msg_size)) !=msg_size)
			{
				ALOG(DIAG2,RTMQ_mID,"ERROR: size sent %d instead of %d in "\
								"process_complete_ms_msg\n",size_sent,msg_size);
			}
			// sched_mgr will send back a reply which will be processed on arrival
			free(p_request_msg);
			p_request_msg = NULL;
		}
	}
		break;
	case RTC_DROP_SCHEDULED_EVENT_REQUEST: 
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_DROP_SCHEDULED_EVENT_REQUEST\n");
	{
		// pointer to mq message to sched_mgr we will build, malloc'd below
		mq_send_drop_event_msg * p_request_msg;
		// struct for message we are processing from MS
		embeddedDropScheduledEventRequestMsg * p_drop_msg
				= (embeddedDropScheduledEventRequestMsg *)p_header;

		ssize_t msg_size = sizeof(mq_send_drop_event_msg); 
		ssize_t size_sent;

		if((p_request_msg = (mq_send_drop_event_msg *)malloc(msg_size))==NULL)
		{
			AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
				"ERROR: malloc failed for mq_msg for drop_event message\n");
			abort();
		}
		// build up content of send_drop_event message
		p_request_msg->header = send_drop_event_msg;
		p_request_msg->header.msg_id = mq_get_next_msg_id();
		// Pack along appl_entity so reply can be routed to right connection
		p_request_msg->header.cookie = (void *)p_header->appl_entity;

		p_request_msg->owner_id = p_header->sender;
		// copy message data into mq message
		p_request_msg->request = p_drop_msg->request;

		if((size_sent = mq[AMMQ_TO_SCHED].rt_mq->send_to_mqueue(
			(char *)p_request_msg, msg_size)) !=sizeof(mq_msg_header))
		{
			ALOG(DIAG2,RTMQ_mID,"ERROR: size sent %d instead of %d in "\
							"process_complete_ms_msg\n",size_sent,msg_size);
		}
		// sched_mgr will send back a reply which will be processed on arrival
		free(p_request_msg);
		p_request_msg = NULL;
	}
		break;
	case RTC_STATUS_RESPONSE:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,"Receiving RTC_STATUS_RESPONSE\n");
		break;
	case RTC_LOG_CONTROL_REQUEST:
	{
		ALOG(SHOW,MS_SOCK_MSG_IN_mID,"Receiving RTC_LOG_CONTROL_REQUEST\n");
		ssize_t msg_size = sizeof(mq_send_log_control_msg);
		ssize_t size_sent;
		embeddedLogControlRequestMsg * p_in_msg = 
									(embeddedLogControlRequestMsg *)p_header;
		mq_send_log_control_msg * p_log_ctl_msg;
		if((p_log_ctl_msg = (mq_send_log_control_msg *)malloc(msg_size))==NULL)
		{
			ALOG(FATAL,MALLOC_mID,
				"ERROR: failed to malloc %d bytes\n",msg_size);
			abort();
		}
		p_log_ctl_msg->header = send_log_control_msg;
		p_log_ctl_msg->header.msg_id = mq_get_next_msg_id();
		p_log_ctl_msg->request = p_in_msg->request;
		if((size_sent = mq[AMMQ_TO_MSG].rt_mq->send_to_mqueue(
			(char *)p_log_ctl_msg,msg_size)) != msg_size)
		{
			ALOG(DIAG2,RTMQ_mID,"ERROR: size sent %d instead of %d in "\
							"process_complete_ms_msg\n",size_sent,msg_size);
		}
		free(p_log_ctl_msg);
		p_log_ctl_msg = NULL;
	}
		break;
	case RTC_DEVICE_STATUS_SUBSCRIPTION:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,
				"Receiving RTC_DEVICE_STATUS_SUBSCRIPTION\n");
		ALOG(DIAG2,STAT_SUBSCR_mID,
				"Receiving RTC_DEVICE_STATUS_SUBSCRIPTION\n");
		{
		
			embeddedDevStatSubscriptionRequestMsg* p_request_msg =
				(embeddedDevStatSubscriptionRequestMsg*)p_header; 

			// static embeddedDevStatSubscriptionRequest last_request;

			//if(!memcmp((void*)(&p_request_msg->request),
			// 	(void*)(&last_request),sizeof(embeddedDevStatSubscriptionRequest)))
			{
				// build a message to send to dev_mgr with subscription information
				mq_send_subscription_msg * p_subscr_msg = NULL;
				ssize_t msg_size = sizeof(mq_send_subscription_msg);
				if((p_subscr_msg 
						= (mq_send_subscription_msg*)malloc(msg_size))==NULL)
				{
					ALOG(FATAL,MALLOC_mID,
						"ERROR: couldn't malloc %d bytes for mq_send_subscr_msg\n",
							msg_size);
					abort();
				}
	
				p_subscr_msg->header = send_subscription_msg;
				// mq message uses same request struct as socket message
				p_subscr_msg->request = p_request_msg->request;
	
				// include appl_entity so reply can be routed to right connection
				p_subscr_msg->header.cookie = (void *)p_header->appl_entity;
	
				if((size_sent = mq[AMMQ_TO_DEV].rt_mq->send_to_mqueue(
					(char *)p_subscr_msg,msg_size)) != msg_size)
				{
					ALOG(DIAG2,RTMQ_mID,
						"ERROR: sent %d instead of %d in process_complete_ms_msg\n",
						size_sent,msg_size);
				}
				free(p_subscr_msg);
				p_subscr_msg = NULL;
			}
			// else
				// last_request = p_request_msg->request;
		}
		break;
	default:
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,
			"*******************************************************\n");
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,
			"************* ERROR: undefined command type %ld from MS\n",
						p_header->type);
		ALOG(DIAG2,MS_SOCK_MSG_IN_mID,
			"*******************************************************\n");
		break;
	}
	}

	return(error_code);
};

//-----------------------------------------------------------------------------|
// ms_connection::run_ms_connection
//

// first the little wrapper function pthread_create uses
void * ms_conn_entry_func(void * p_entry_struct)
{
	ms_connection * this_ptr = (ms_connection *)(
						((thread_entry_struct *)p_entry_struct)->this_ptr);

	this_ptr->run_ms_connection(
					((thread_entry_struct *)p_entry_struct)->p_vargs);

	// end thread when real function returns
	pthread_exit(NULL);

	return(NULL);
};

// then the real function
void *
ms_connection::run_ms_connection(void * p_vargs)
{
	size_t msg_size;
	ms_raw_message raw_msg;
	int result;

	while(1)
	{
		if((msg_size 
			= get_complete_ms_msg(&raw_msg, MAX_MSG_SIZE))==0)
		{
			ALOG(WARN,SOCK_MSG_mID,
				"ERROR: failed to get_complete_msg in run_ms_connection\n");
		}
		else
		{
			ALOG(WARN,SOCK_MSG_mID,
				"In run_ms_connection type is %ld\n",raw_msg.msg_header.type);
			if((result = process_complete_ms_msg(&raw_msg, msg_size))!=SUCCESS)
			{
				ALOG(WARN,SOCK_MSG_mID,
					"ERROR: failed to process_complete_ms_msg\n");
			}
		}
	}
	return(NULL);
};


//-----------------------------------------------------------------------------|
// ms_sockets_mgr::ms_sockets_mgr 
//
ms_sockets_mgr::ms_sockets_mgr(int port_num, int max_connections)
{
	ms_sock_sin.sin_family = AF_INET;
	ms_sock_sin.sin_port = htons(port_num);
	ms_sock_sin.sin_addr.s_addr = INADDR_ANY;

	ms_sock_sin_size = sizeof(ms_sock_sin);

	num_ms_connections = 0;
	ALOG(DIAG3,SOCKET_mID,"Setting num_ms_connections to zero\n");
	max_ms_connections = max_connections;
	port_number = port_num;
}


//-----------------------------------------------------------------------------|
// ms_sockets_mgr::~ms_sockets_mgr 
//
ms_sockets_mgr::~ms_sockets_mgr()
{
	ALOG(WARN,SOCK_MSG_mID, "End of ms_sockets_mgr\n");
};

//-----------------------------------------------------------------------------|
// ms_sockets_mgr::get_appl_entity_connection
//
ms_connection *	ms_sockets_mgr::get_appl_entity_connection(unsigned long appl_entity)
{
	ms_connection * p_ms_ret = NULL;
	int i;
	for(i=0;i<num_ms_connections;i++)
	{
		if(ms[i]->appl_entity == appl_entity)
		{
			p_ms_ret = ms[i];
			break;
		}
	}
	return p_ms_ret;
};

//-----------------------------------------------------------------------------|
// ms_sockets_mgr::listen_for_ms_connections
//

// first the little wrapper function pthread_create uses
void * listen_for_entry_func(void * p_entry_struct)
{
	ms_sockets_mgr* this_ptr = (ms_sockets_mgr *)(
						((thread_entry_struct *)p_entry_struct)->this_ptr);

	this_ptr->listen_for_ms_connections(
					((thread_entry_struct *)p_entry_struct)->p_vargs);

	// end thread when real function returns
	pthread_exit(NULL);

	return(NULL);
};

// then the real function
void *
ms_sockets_mgr::listen_for_ms_connections(void * p_varg)
{
	int i;
	int als_fd;
	int new_sock_fd;
	pthread_attr_t attr;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	thread_entry_struct thread_entry;
	thread_entry.this_ptr = (void *)this;
	thread_entry.p_vargs = NULL;

	als_fd = socket(AF_INET,SOCK_STREAM,0);
	fprintf(stderr, "opened new socked with als_fd = %d\n",als_fd);

	if(setsockopt(als_fd,SOL_SOCKET,SO_REUSEADDR,NULL,0)==-1)
	{
		int error_val = errno;
		ALOG(FATAL,SOCKET_mID,
			"Failed to set SO_REUSEADDR on socket %d, error %d: %s\n",
				als_fd,error_val,strerror(error_val));
	}
	if(setsockopt(als_fd,SOL_SOCKET,SO_REUSEPORT,NULL,0)==-1)
	{
		int error_val = errno;
		ALOG(FATAL,SOCKET_mID,
			"Failed to set SO_REUSEPORT on socket %d, error %d: %s\n",
				als_fd,error_val,strerror(error_val));
	}

	bind(als_fd,(struct sockaddr *)&ms_sock_sin, ms_sock_sin_size);  

	fprintf(stderr,"bound socket, now listen on port %d\n",port_number);

	listen(als_fd,64);

	// now wait for MSs to talk to this port and create a new
	// socket thread for each one
	while(!finished.asif_socks)
	{
		new_sock_fd 
			= accept(als_fd,(struct sockaddr *)&ms_sock_sin,&ms_sock_sin_size);  
		ALOG(DIAG3,SOCK_MSG_mID,
			"accept returned new sock fd of %d whch will be number %d\n",
				new_sock_fd, num_ms_connections+1);

		int this_one = num_ms_connections;
		ALOG(DIAG3,SOCKET_mID,"Setting this_one to num_ms_connections, %d,%d\n",
						this_one,num_ms_connections);

		// create new ms_connection with this new socket
		sock_mgr.ms[num_ms_connections++] 
							= new ms_connection(new_sock_fd);
		ALOG(DIAG3,SOCKET_mID,"Incrementing num_ms_connections to %d\n",
						num_ms_connections);

		ALOG(DIAG5,SOCKET_mID, "ms[%d] is %p\n",this_one, ms[this_one]);	
		ms[this_one]->print_sock_fd();
		
	
	
		// start thread to wait for messages from automation system
		thread_entry.this_ptr = (void *)ms[this_one];
		thread_entry.p_vargs = NULL;

		pthread_create(NULL, &attr, 
						ms_conn_entry_func, (void *)&thread_entry);
	}
	// tell each of the connections in existance to shutdown
	for(i=0;i<num_ms_connections;i++)
		ms[i]->shutdown_ms_connection();
	delete ms[i];
	// close the socket used to listen for ms connections
	close(als_fd);
	// end this thread
	// pthread_exit(NULL);

	// just for style and warning suppression
	return((void*)NULL);
}; 

//-----------------------------------------------------------------------------|
// remove_ms_connection
//
int
ms_sockets_mgr::remove_ms_connection(ms_connection * p_ms_conn)
{
	int i,j;
	ALOG(DIAG2,SOCKET_mID,"removing %p from one of %d entries:\n",p_ms_conn,
					num_ms_connections);
	for(i=0;i<num_ms_connections;i++)
			ALOG(DIAG4,SOCKET_mID,"%p\n",ms[i]);
	// find this connection 
	// remove this connection and move others down if this is not last
	for(i=0;i<num_ms_connections;i++)
	{
		if(ms[i]==p_ms_conn)
		{
			if(i<(num_ms_connections-1))
			{
				for(j=i;j<(num_ms_connections-1);j++)
				{
						ms[j]=ms[j+1];
				}
			}
			num_ms_connections--;
			ALOG(DIAG4,SOCKET_mID,"Decrementing num_ms_connections to %d\n",
							num_ms_connections);
			ALOG(DIAG2,SOCKET_mID,"num_ms_connections is now %d\n",
							num_ms_connections);
			return(SUCCESS);
		}
	}
	return(FAILURE);
};
//-----------------------------------------------------------------------------|
// proc_msg_hndlr
//
void *
proc_msg_hndlr(void * p_varg)
{
	while(!finished.process)
	{
		int n;
		ssize_t rcv_size;
		fd_set prfd;

		int i;
		// prepare variables used as args to select()
		FD_ZERO(&prfd);
		int maxfd = 0;
		for(i=0;i<NUM_AMMQ_VALS;i++)
		{
			if((mq[i].rw == rw_read_only) || (mq[i].rw == rw_read_write))
			{
				FD_SET(mq[i].rt_mq->rt_mqueue_descr, &prfd);
				if(mq[i].rt_mq->rt_mqueue_descr > maxfd)
					maxfd = mq[i].rt_mq->rt_mqueue_descr;
			}
		}

		// wait for messages from any of the incoming message queues
		switch ( n = select ( maxfd+1, &prfd, 0, 0, NULL))
		{
		case -1:
			{
				int error_val = errno;
				ALOG(WARN,RTMQ_mID, "ERROR: asif mq select failed - %d, %s\n",
					error_val, strerror(error_val));
			}
			break;
		case 0:
			ALOG(WARN,RTMQ_mID,"select timed out\n");
			break;
		default:
			ALOG(DIAG4,RTMQ_mID,"%d descriptors ready in select\n",n);

			// loop through all message queues used in asif_mgr
			// look for ones with read data ready
			for(i=0;i<NUM_AMMQ_VALS;i++)
			{
				if(FD_ISSET(mq[i].rt_mq->rt_mqueue_descr, &prfd))
				{
					ssize_t rcv_buf_size;
					char * rcv_buf;
					// get size used for this mq's messages
					if(mq[i].rt_mq->get_msg_size(&rcv_buf_size)==FAILURE)
					{
						AERR(NE_OPFAILED,ERROR,RTMQ_mID,
							"ERROR: failed to get msg size from rt_mqueue\n");
							abort();
					}
					// and allocate the buffer
					if((rcv_buf = (char *)malloc(rcv_buf_size))==NULL)
					{
						AERR(NE_MALLOC_FAILED,ERROR,MALLOC_mID,
								"ERROR: failed to malloc for mq buf\n");
						abort();
					}

					ALOG(DIAG4,RTMQ_mID,"%s has read data pending\n",
								rt_mq_names[mq[i].which]);
					// get the message
					rcv_size = mq[i].rt_mq->receive_from_mqueue(rcv_buf);
					ALOG(DIAG3,RTMQ_mID,
					"Received %d bytes from receive_from_mqueue\n",rcv_size);
					// go do everything this message requires
					process_mq_msg(rcv_buf, rcv_size);
					// ok, done with message, free buffer used for it
					free(rcv_buf);
				}
			}
			break;
		}
	}
	return(NULL);
};

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	ALOG(WARN,TEMP_mID,"ASIF RECEIVED SIGUSR1 %d, restart asif_mgr\n",signo); 
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	ALOG(WARN,TEMP_mID,"ASIF RECEIVED SIGUSR2 %d, stop asif_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
//	MAIN
int
main()
{
	int i;
	thread_entry_struct main_thread_entry;
	pthread_attr_t attr;
#if 0
	struct _clockperiod clkper;
	struct _clockperiod oldclkper;
#endif

	char * sender_name = "RTC1";
	fprintf(stderr, "starting asif_mgr\n"); 

#ifdef LOG_FILE
	if((logfile = fopen(LOG_FILE,"w+"))==NULL)
	{
		int error_val = errno;
		printf("ERROR: opening log file %s, use stderr, error %d, %s\n",
				LOG_FILE,error_val,strerror(error_val));
		logfile = stderr;
	}
#endif
	rt_set_logfile("AS",logfile);
	
	still_trying = 1;
	stop_requested = 0;

#if 0
	// set tick length in nanoseconds
	clkper.nsec = 500330;
	clkper.fract = 0;
	if(ClockPeriod(CLOCK_REALTIME,&clkper,&oldclkper,0)==-1)
	{
		int error_val = errno;
		AERR(error_val,WARN,CLOCK_mID,"ClockPeriod failed with error %d, %s\n",
						error_val,strerror(error_val));
	}
#endif

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

	for(i=0;i<16;i++)
	{
		this_sender[i].wchars.wchar_hi=0;
		this_sender[i].wchars.wchar_lo=sender_name[i];
	}
	// creating threads in detached state allows them to end without us helping
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	// clear finished (shutdown flags)
	bzero(&finished,sizeof(finished));
	// set up simple ack message
	bzero(&simple_ack_msg.sender,sizeof(RTC_HOSTNAME)); // RTC_RTC_BASE_ID + WHICH_RTC;
	simple_ack_msg.id = 0;	// must be filled in each time sent
	simple_ack_msg.type = RTC_ACKNOWLEDGEMENT_RESPONSE;
	simple_ack_msg.size = sizeof(embeddedHeader);

	while(still_trying)
	{

		//
		// instantiate and open all of the message queues to and from other procs
		//
		for(i=0;i<NUM_AMMQ_VALS;i++) 
		{
			if((mq[i].rt_mq = 
				new rt_mqueue(mq[i].which,mq[i].rw,mq[i].blk))==NULL)
			{
				fprintf(stderr,
					"ERROR: Failed to open message queue for %s in asif_mgr\n",
								rt_mq_names[mq[i].which]);
			}
			else
			{
				fprintf(stderr,"opened %s\n",rt_mq_names[mq[i].which]);
			}
		}
		// initialize logging facility
		embeddedInitLogging('A',&mq[AMMQ_TO_MSG]);

		// start thread to listen for automation system to connect to
		void * p_sock_mgr = (void *)&sock_mgr;
	
		main_thread_entry.this_ptr = p_sock_mgr;
		main_thread_entry.p_vargs = NULL;
	
		pthread_create(NULL, &attr, listen_for_entry_func, 
													(void *)&main_thread_entry);
	
		// start thread to wait for messages from other processes
		pthread_create(NULL, &attr, proc_msg_hndlr, NULL);

		sleep(1);

		// send hanshake msg to dev and sched
		mq_hand_shake_msg handshake;
		handshake.header = hand_shake_msg;
		handshake.header.msg_sender_id = mq_user_asif;
		handshake.header.msg_receiver_id = mq_user_dev;
		mq[AMMQ_TO_DEV].rt_mq->send_to_mqueue((char*)&handshake,
			sizeof(mq_hand_shake_msg));
		handshake.header.msg_receiver_id = mq_user_sched;
		mq[AMMQ_TO_SCHED].rt_mq->send_to_mqueue((char*)&handshake,
			sizeof(mq_hand_shake_msg));

		// restart gate
		stop_requested = 0;
		int loop = 0;
		while(!stop_requested)
		{
			ALOG(DIAG3,ASIF_TICK_mID,"Tick - %d\n",loop++);
			sleep(1);
		}
		ALOG(SHOW,TEMP_mID,"Got SIGUSR1 or SIGUSR2\n");
	
		// --------------------------- body -----------------------------

		// close all open connections
		
		// close all of the message queues used
		for(i=0;i<NUM_AMMQ_VALS;i++) 
		{
			delete mq[i].rt_mq;
		}
		fprintf(stderr,"Deleted all the mqueues\n");
	}
	fprintf(stderr,"Must have been SIGUSR2\n");

#ifdef LOG_FILE
	if(logfile!=stderr)
		fclose(logfile);
#endif
	return(0);
}
