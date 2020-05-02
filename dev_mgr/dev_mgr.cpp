#ifdef KRD
#define LOG_FILE "/tmp/krd_dev_mgr.log"
#else
#define LOG_FILE "/tmp/dev_mgr.log"
#endif
//-----------------------------------------------------------------------------|
//
// dev_mgr.cpp
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <assert.h>
#include "nov_log.hpp"
#include "embeddedSerialRecordDeviceLog.hpp"
#include "embeddedRecordDeviceTable.hpp"
#include "rt_mqueue.hpp"
#include "rt_errors.hpp"
#include "w2char.hpp"
#include "asif_protocol.hpp"
#include "rtc_config.hpp"
#include "dev_holder.hpp"
#include "dev_mgr.hpp"

FILE* logfile = stderr;

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[4] = "DE";

size_t event_type_data_size[NumEventTypes] = 	{ 
	//	X0	X1	X2	X3	X4	X5	X6	X7	X8	X9
		0,	0,	0,	0,	0,	4,	4,	0,	0,	0,	// 0X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 1X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 2X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 3X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 4X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 5X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 6X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 7X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0,	// 8X
		0,	0,	0,	0,	0,	0,	0,	0,	0,	0};	// 9X

extern char* mq_user_name[num_mq_user_vals];

// pointer to one and only device holder, created in main
dev_holder * p_holder;

#pragma pack(1)

static int stop_requested = 0;
static int still_trying = 1;

// this is a set of flags for controlling shutdown of threads or process
struct
{
	bool asif_socks;
	bool sched_mqs;
	bool stat_mqs;
	bool dev_mqs;
	bool process;
} finished;


// this array keeps track of just the mqueues dev_mgr uses out of
// all the mqueues in the system
//
mod_mqueue mq[NUM_DMMQ_VALS] =
{
	// which-mqueue         read/write		block/non-block	rt_mqueue pointer
	{ WMQ_ALL_TO_MSG,		rw_write_only,	blk_blocking,	NULL},
	{ WMQ_DEV_TO_ASIF, 		rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_ASIF_TO_DEV, 		rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_DEV_TO_SCHED, 	rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_SCHED_TO_DEV, 	rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_DEV_TO_STAT, 		rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_STAT_TO_DEV, 		rw_read_only, 	blk_blocking,	NULL}
};

// table to allow looking up outgoing mqueue index by incoming user id
const en_dev_mqueues dmq_from_sender_id[num_mq_user_vals] =
{
	NUM_DMMQ_VALS,
	DMMQ_TO_ASIF,
	NUM_DMMQ_VALS,
	NUM_DMMQ_VALS,
	NUM_DMMQ_VALS,
	NUM_DMMQ_VALS,
	DMMQ_TO_SCHED,
	DMMQ_TO_STAT,
	NUM_DMMQ_VALS,
	NUM_DMMQ_VALS
};

//-----------------------------------------------------------------------------|
// build_dev_config_reply 
//
mq_msg_header * build_dev_config_reply(const mq_msg_header * const p_header)
{
	// int phlc = 0;
	mq_all_dev_reply_msg * p_reply = NULL;
	size_t size_of_msg;
	unsigned long num_devices;

	num_devices = p_holder->get_device_count();
	DLOG(DIAG5,TEMP_mID,
		"Number of devices in build_dev_config_reply is %ld\n",
					num_devices);
	size_of_msg = sizeof(mq_msg_header) + sizeof(unsigned long)
						+ sizeof(Device) * num_devices;

	p_reply = (mq_all_dev_reply_msg *)malloc(size_of_msg);
	if(p_reply==NULL)
	{
		DLOG(DIAG5,TEMP_mID,
			"ERROR: couldn't malloc %d bytes in build_dev_config_reply\n",
						size_of_msg);
		fclose(logfile);
		abort();
	}
	bzero(p_reply, size_of_msg);

	
	p_reply->header.msg_type = reply_with_all_devices_status;
	p_reply->header.msg_size = size_of_msg;
	p_reply->header.msg_sender_id = mq_user_dev;
	p_reply->header.msg_receiver_id = mq_user_asif;
	p_reply->header.msg_id = mq_get_next_msg_id();
	p_reply->header.cookie = p_header->cookie;
	p_reply->device_count = num_devices;
	p_reply->response.record_size = sizeof(Device) * num_devices;

	p_holder->fill_all_device_response(
	 			(void*)&p_reply->response.device[0],(size_t)size_of_msg);
	
	return(&p_reply->header);
};


//-----------------------------------------------------------------------------|
// process_mq_msg
//
void
process_mq_msg(char * buf, size_t size)
{
	assert(size >= sizeof(mq_msg_header));
	mq_msg_header * p_header = (mq_msg_header *)&buf[0];
	mq_msg_header * p_reply_header = NULL;

	assert(p_header);

	switch(p_header->msg_type)
	{
	// hand_shake is universal to all processes
	case hand_shake:
		fprintf(logfile,"Received handshake from %s\n",
			mq_user_name[p_header->msg_sender_id]);
		DLOG(DIAG5,HAND_SHAKE_mID,
			"Received mq_msg hand_shake\n");
		break;
	// message types sent from asif_mgr
	case request_system_status:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_system_status\n");
		break;
	case request_all_devices_status:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_all_devices_status\n");

		ssize_t size_sent;


		p_reply_header = build_dev_config_reply(p_header);


		DLOG(DIAG2,DEV_MSG_OUT_mID,"Sending all device status to asif\n");
		if((size_sent = mq[dmq_from_sender_id[
				p_header->msg_sender_id]].rt_mq->send_to_mqueue(
				(char *)p_reply_header,(size_t)p_reply_header->msg_size))==-1)
		{
			DLOG(DIAG5,DEV_MSG_OUT_mID,
				"ERROR: send_to_mqueue returned -1 in process_mq_msg\n");
			DLOG(CONVFP,TEMP_mID,"ERROR: send_to_mqueue returned -1 in process_mq_msg\n");
		}
		else
			DLOG(CONVFP,TEMP_mID,"sent %d bytes to asif mqueue\n",size_sent);

		free(p_reply_header);
		
		break;
	case request_one_device_status:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_one_device_status\n");
		break;
	case send_port_device_assignment:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_port_device_assignment\n");
		break;
	case send_add_device:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_add_device\n");
		DLOG(CONVFP,TEMP_mID,"Received mq_msg send_add_device\n");
	{
		mq_send_add_device_msg * p_msg;
		p_msg = (mq_send_add_device_msg *)p_header;
		// add new device if all command data is valid
		DLOG(SHOW,DEVICE_mID,"Calling install_dev for add dev cmd\n");
		p_holder->install_dev(p_msg->request);
	}
		break;
	case send_delete_device:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_delete_device\n");
	{
		mq_send_delete_device_msg * p_msg;
		p_msg = (mq_send_delete_device_msg *)p_header;
		// delete device if found
		p_holder->remove_dev(p_msg->request);
	}
		break;
	case request_timecode:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_timecode\n");
		break;
	case request_all_event_brief:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_all_event_brief\n");
		break;
	case request_one_event:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_one_event\n");
		break;
	case send_add_event:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_add_event\n");
		break;
	case send_drop_event:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_drop_event\n");
		break;
	case send_device_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_device_config\n");
		break;
	case send_time_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_time_config\n");
		break;
	case send_system_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_system_config\n");
		break;
	case send_port_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_port_config\n");
		break;
	case send_log_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_log_config\n");
		break;
	case request_port_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_port_config\n");
		break;
	case request_port_device_assignment:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_port_device_assignment\n");
		break;

	// message types sent from dev_mgr
	case reply_with_all_devices_status:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_all_devices_status\n");
		break;
	
	// message types sent from log_mgr
	case reply_with_log_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_log_config\n");
		break;

	// message types sent from msg_mgr

	// message types sent from proc_mgr
	case request_process_to_shutdown:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_process_to_shutdown\n");
		break;
	case request_process_to_start_operations:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_process_to_start_operations\n");
		break;
	case request_process_to_suspend_operations:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_process_to_suspend_operations\n");
		break;
	case request_process_to_reinit:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_process_to_reinit\n");
		break;

	// message types sent from sched_mgr
	case reply_with_all_event_structs:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_all_event_structs\n");
		break;
	case reply_with_one_event_struct:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_one_event_struct\n");
		break;
	case reply_with_next_event_time:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_next_event_time\n");
		break;
	case reply_with_latest_event_time:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_latest_event_time\n");
		break;
	case reply_with_number_of_events:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_number_of_events\n");
		break;
	case reply_with_all_one_device_event_structs:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_all_one_device_event_structs\n");
		break;
	case reply_with_total_event_capacity:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_total_event_capacity\n");
		break;
	case request_all_device_status:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_all_device_status\n");
		break;
	case request_current_timecode:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg request_current_timecode\n");
		break;
	case send_device_control_data:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg send_device_control_data\n");
		// DLOG(CONVFP,TEMP_mID,"Received mq_msg send_device_control_data\n");
		{
			DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
			device* p_device = NULL;
			mq_send_device_control_data_msg * p_dcd_msg =
				(mq_send_device_control_data_msg *)p_header;
		
			DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
			// get the device pointer for device_id in request
			p_holder->get_device_for_id(
				(const w2char_t *)(&p_dcd_msg->request.device_id),
				&p_device);

			DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
			if(p_device)
			{
				DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
				// response will be set null for now, so
				// declare it here. TODO
				device_response * p_dev_resp;
				char hexstr[1024];
				DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
				hex_byte_dump_to_string(hexstr,1024,
					(char*)(&p_dcd_msg->request),
						sizeof(embeddedAddScheduledEventRequest),16);
				DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
				DLOG(DIAG3,RECORD_DEVICE_mID,"dump of device control: %s\n",hexstr);
				// send command to device 
				p_device->send_command(p_dcd_msg->request.event_type,
						(void*)(&p_dcd_msg->request.dev_data),
						event_type_data_size[p_dcd_msg->request.event_type],
						&p_dev_resp);
				DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
				if(p_dev_resp) // TODO
					DLOG(CONVFP,TEMP_mID,
					"response is not null, do something with it now!\n");
				DLOG(DIAG5,DEV_MSG_IN_mID,"mq_msg send_device_control_data line %d\n",__LINE__);
			}
			else
			{
				char devstr[32];
				w2char_string_to_char(
						(w2char_t*)p_dcd_msg->request.device_id,devstr,
						RTC_DEVICE_ID_LENGTH);
				DLOG(CONVFP,TEMP_mID,
					"ERROR: device for device_id %s in cmd not installed\n",
						devstr);
			}
		}	
		break;
	
	// message types sent from stat_mgr
	case reply_with_system_status:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_system_status\n");
		break;
	case reply_with_system_config:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_system_config\n");
		break;
	
	// message types sent from time_mgr
	case announce_vi_interrupt_with_tc_val:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg announce_vi_interrupt_with_tc_val\n");
		break;
	case reply_with_timecode:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"Received mq_msg reply_with_timecode\n");
		break;
	case reply_with_time_config:
		break;
	case send_subscription:
		DLOG(DIAG3,DEV_MSG_IN_mID,"Received mq_msg send_subscription\n");
		DLOG(DIAG3,STAT_SUBSCR_mID,"Received mq_msg send_subscription\n");
		{
			mq_send_subscription_msg * p_subscr_msg = 
					(mq_send_subscription_msg *)p_header;
			// make call to add or remove this subcriber for the device indicated
			if(p_holder->handle_subscription_request(
						(unsigned long)(p_subscr_msg->header.cookie), 
						p_subscr_msg->request) != SUCCESS)
			{
				DLOG(WARN,STAT_SUBSCR_mID,
					"FAILED to add or remove subscriber for device status\n");
				// abort(); // TODO add reason arg later
			}
		}
		break;
	default:
		DLOG(DIAG5,DEV_MSG_IN_mID,
			"ERROR: unknown message type %d received\n",p_header->msg_type);
		break;
	}
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
		for(i=0;i<NUM_DMMQ_VALS;i++)
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
				DLOG(DIAG5,TEMP_mID, "ERROR: dev mq select failed - %d, %s\n",
						error_val, strerror(error_val));
			}
			break;
		case 0:
			DLOG(DIAG5,TEMP_mID,
				"<d>select timed out\n");
			break;
		default:
			// DLOG(DIAG5,TEMP_mID,"<d>%d descriptors ready in select\n",n);

			// loop through all message queues used in asif_mgr
			// look for ones with read data ready
			for(i=0;i<NUM_DMMQ_VALS;i++)
			{
				if(FD_ISSET(mq[i].rt_mq->rt_mqueue_descr, &prfd))
				{
					ssize_t rcv_buf_size;
					char * rcv_buf;
					// get size used for this mq's messages
					if(mq[i].rt_mq->get_msg_size(&rcv_buf_size)==FAILURE)
					{
						DLOG(DIAG5,TEMP_mID,
							"ERROR: failed to get msg size from rt_mqueue\n");
							abort();
					}
					// and allocate the buffer
					if((rcv_buf = (char *)malloc(rcv_buf_size))==NULL)
					{
						DLOG(DIAG5,TEMP_mID,
							"ERROR: failed to malloc for mq buf\n");
						abort();
					}

					DLOG(DIAG5,TEMP_mID,
						"%s has read data pending\n",
								rt_mq_names[mq[i].which]);
					// get rt_mqueue message
					rcv_size = mq[i].rt_mq->receive_from_mqueue(rcv_buf);
					if(rcv_size==0)
					{
						DLOG(CONVFP,TEMP_mID,"zero size packet!\n");
						abort();
					}
					if(rcv_buf==NULL)
					{
						DLOG(CONVFP,TEMP_mID,"null rcv_buf!\n");
						abort();
					}
					DLOG(DIAG5,TEMP_mID,
						"Received %d bytes from %s\n",rcv_size,
									rt_mq_names[i]);
					DLOG(CONVFP,TEMP_mID,"i = %d, Received %d bytes from %s\n",
									i, rcv_size, rt_mq_names[i]);
					// do whatever is needed for this message
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
	DLOG(DIAG5,TEMP_mID,
		"DEV RECEIVED SIGUSR1 %d, restart dev_mgr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	DLOG(DIAG5,TEMP_mID,
		"DEV RECEIVED SIGUSR2 %d, stop dev_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
// MAIN
//

int
main()
{
	int i;
	pthread_attr_t attr;
	// creating threads in detached state allows them to end without us helping
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

	fprintf(stderr,"Starting dev_mgr\n");

#ifdef LOG_FILE
	if((logfile = fopen(LOG_FILE,"w+"))==NULL)
	{
		int error_val = errno;
		fprintf(stderr,"opening log file %s, use stderr, error %d, %s\n",
				LOG_FILE,error_val,strerror(error_val));
		logfile = stderr;
	}
#endif
	rt_set_logfile("DE",logfile);

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
		// instantiate and open all of the message queues 
		// to and from other procs
		//
		for(i=0;i<NUM_DMMQ_VALS;i++) 
		{
			if((mq[i].rt_mq = 
					new rt_mqueue(mq[i].which,mq[i].rw,mq[i].blk))==NULL)
			{
				fprintf(stderr,
					"ERROR: Failed to open message queue for %s in dev_mgr\n",
								rt_mq_names[mq[i].which]);
			}
		}
		embeddedInitLogging('D',&mq[DMMQ_TO_MSG]);

		// allocate table for record devices
		embeddedRecordDeviceTable::Allocate();
		// allocate log for recorder
		embeddedSerialRecordDeviceLog::Allocate();
	
		// start thread to wait for messages from other processes
		pthread_create(NULL, &attr, proc_msg_hndlr, NULL);

		// --------------------------- body -----------------------------
	
		// start dev_holder which will start a thread for each installed
		// device to manage that device and communications to and from it
	#ifdef KRD
		p_holder = new dev_holder("/tmp/krd_dev_info.dat");
	#else
		p_holder = new dev_holder("/tmp/dev_info.dat");
	#endif
	
		// send hanshake msg to asif and sched
		mq_hand_shake_msg handshake;
		handshake.header = hand_shake_msg;
		handshake.header.msg_id = mq_get_next_msg_id();
		handshake.header.msg_sender_id = mq_user_dev;
		handshake.header.msg_receiver_id = mq_user_asif;
		mq[DMMQ_TO_ASIF].rt_mq->send_to_mqueue((char*)&handshake,
			sizeof(mq_hand_shake_msg));
		handshake.header.msg_receiver_id = mq_user_sched;
		mq[DMMQ_TO_SCHED].rt_mq->send_to_mqueue((char*)&handshake,
			sizeof(mq_hand_shake_msg));

		int loop = 0;
		while(!stop_requested)
		{
			sleep(1);
			DLOG(DIAG3,DEV_TICK_mID, "Tick - %d\n",loop++);
			p_holder->publish_all_devices_status_reports();
		}
	
		sleep(1);

		// deallocate table for record devices
		embeddedRecordDeviceTable::Deallocate();


		// shut down dev_holder which will terminate all devices and their
		// device manager threads
		delete p_holder;
	
	
		// close all of the message queues used
		for(i=0;i<NUM_DMMQ_VALS;i++) 
		{
			delete mq[i].rt_mq;
		}
	}
#ifdef LOG_FILE
	if(logfile!=stderr)
		fclose(logfile);
#endif
	return(0);
}
