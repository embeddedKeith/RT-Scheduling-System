#ifdef KRD
#define LOG_FILE "/tmp/krd_sched_mgr.log"
#else
#define LOG_FILE "/tmp/sched_mgr.log"
#endif
//-----------------------------------------------------------------------------|
//
// sched_mgr.cpp
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/12/01
//
// last mod: 8/28/01
//-----------------------------------------------------------------------------|
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <strings.h>
#include <assert.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/elftypes.h>
#include "nov_log.hpp"
#include "w2char.hpp"
#include "rt_mqueue.hpp"
#include "rt_errors.hpp"
#include "sched_mgr.hpp"

FILE* logfile = stderr;

static int stop_requested = 0;
static int still_trying = 1;

static int running_schedule = 0;

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[3] = "SC";

extern char* mq_user_name[num_mq_user_vals];

//=============================================================================|

static schedule rt_schedule;		// one instance of schedule, only one

// this array keeps track of just the mqueues sched_mgr uses out of
// all the mqueues in the system
//
mod_mqueue mq[NUM_SMMQ_VALS] =
{
	// which-mqueue         read/write		block/non-block	rt_mqueue pointer
	{ WMQ_ALL_TO_MSG,		rw_write_only,	blk_blocking,	NULL},
	{ WMQ_SCHED_TO_ASIF, 	rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_ASIF_TO_SCHED, 	rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_SCHED_TO_DEV, 	rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_DEV_TO_SCHED, 	rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_SCHED_TO_STAT, 	rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_STAT_TO_SCHED, 	rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_SCHED_TO_TC, 		rw_write_only, 	blk_blocking,	NULL},
	{ WMQ_TC_TO_SCHED, 		rw_read_only, 	blk_blocking,	NULL},
	{ WMQ_TCD_TO_SCHED,		rw_read_only,	blk_non_blocking,	NULL}
};

// table to allow looking up outgoing mqueue index by incoming user id
const en_sched_mqueues smq_from_sender_id[num_mq_user_vals] =
{
	NUM_SMMQ_VALS,
	SMMQ_TO_ASIF,
	SMMQ_TO_DEV,
	NUM_SMMQ_VALS,
	NUM_SMMQ_VALS,
	NUM_SMMQ_VALS,
	NUM_SMMQ_VALS,
	SMMQ_TO_STAT,
	SMMQ_TO_TC,
	NUM_SMMQ_VALS
};

//=============================================================================|
// TEMPORARY STUBS FOR FUNCTIONS THAT WILL EXIST ELSWHERE
//
int time_str(rtc_time et, char* buf, size_t size)
{
	int error_code = SUCCESS;
	int last = 0;
	if((last=snprintf(buf,size,"%lld",et.rtc_ms))<0)
	{
		int error_val = errno;
		SLOG(DIAG5,TEMP_mID,
			"ERROR: snprintf had neg return val %d in time_str\n",last);
		SLOG(DIAG5,TEMP_mID,
			"errno was %d, %s\n",error_val,strerror(error_val));
		error_code = FAILURE;
	}
	else if(last<(int)size)
		buf[last]='\0';
	else
		error_code = FAILURE;
	return error_code;
};

//=============================================================================|
// methods
//
mq_msg_header * make_mq_msg_and_fill_header(en_mq_user_id receiver_id, 
				size_t size_of_msg)
{
	mq_msg_header * p_reply = (mq_msg_header *)malloc(size_of_msg);
	if(p_reply==NULL)
	{
		SLOG(DIAG5,TEMP_mID,
			"ERROR: couldn't malloc %d bytes in build_dev_config_reply\n",
						size_of_msg);
	}
	bzero(p_reply, size_of_msg);
	
	// fill in msg_type in calling routine
	p_reply->msg_size = size_of_msg;
	p_reply->msg_sender_id = mq_user_sched;
	p_reply->msg_receiver_id = receiver_id;
	p_reply->msg_id = mq_get_next_msg_id();

	return(p_reply);
};

//-----------------------------------------------------------------------------|
mq_msg_header * build_add_event_reply(mq_msg_header * p_header)
{

	mq_msg_header * p_reply;
	size_t size_of_msg;

	size_of_msg = sizeof(mq_msg_header); // + data size

	p_reply = (mq_msg_header *)make_mq_msg_and_fill_header(
					mq_user_asif, size_of_msg);
	p_reply->cookie = p_header->cookie;

	p_reply->msg_type = reply_with_all_devices_status;

	return(p_reply);
};
//-----------------------------------------------------------------------------|
mq_msg_header * build_delete_event_reply(mq_msg_header * p_header) 
{
	mq_msg_header * p_reply;
	size_t size_of_msg;

	size_of_msg = sizeof(mq_msg_header); // + data size

	p_reply = (mq_msg_header *)make_mq_msg_and_fill_header(
					mq_user_asif, size_of_msg);
	p_reply->cookie = p_header->cookie;

	p_reply->msg_type = reply_with_all_devices_status;

	return(p_reply);
};
#if 0
//-----------------------------------------------------------------------------|
char * build_one_event_reply(mq_msg_header * p_header)
{
};
//-----------------------------------------------------------------------------|
char * build_all_event_brief_reply(mq_msg_header * p_header)
{
};
#endif

//-----------------------------------------------------------------------------|
// sm_proc_rcv_msg
//
void
sm_proc_rcv_msg(char * buf, size_t size)
{
	assert(size >= sizeof(mq_msg_header));
	mq_msg_header * p_header = (mq_msg_header *)&buf[0];

	switch(p_header->msg_type)
	{
	// hand_shake is universal to all processes
	case hand_shake:
		fprintf(logfile,"Received handshake from %s\n",
			mq_user_name[p_header->msg_sender_id]);
		SLOG(DIAG2,HAND_SHAKE_mID,"Received hand_shake\n");
		break;
	// message types sent from asif_mgr
	case request_system_status:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_system_status\n");
		break;
	case request_all_devices_status:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_all_devices_status\n");
		break;
	case request_one_device_status:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_one_device_status\n");
		break;
	case send_port_device_assignment:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_port_device_assignment\n");
		break;
	case request_timecode:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_timecode\n");
		break;
	case request_all_event_brief:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_all_event_brief\n");
#if 0
		p_reply_header = build_all_event_brief_reply(p_header);
		
		if((size_sent = mq[dmq_from_sender_id[
				p_header->msg_sender_id]].rt_mq->send_to_mqueue(
				(char *)p_reply_header,(size_t)p_reply_header->msg_size))==-1)
		{
			SLOG(DIAG5,TEMP_mID,
				"ERROR: send_to_mqueue returned -1 in sm_proc_rcv_msg\n");
		}
		free(p_reply_header);
#endif
		
		break;
	case request_one_event:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_one_event\n");
#if 0
		p_reply_header = build_one_event_reply(p_header);
		
		if((size_sent = mq[dmq_from_sender_id[
				p_header->msg_sender_id]].rt_mq->send_to_mqueue(
				(char *)p_reply_header,(size_t)p_reply_header->msg_size))==-1)
		{
			SLOG(DIAG5,TEMP_mID,
				"ERROR: send_to_mqueue returned -1 in sm_proc_rcv_msg\n");
		}
		free(p_reply_header);
#endif		
		break;
	case send_add_event:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_add_event\n");
		// SLOG(CONVFP,TEMP_mID,"Received send_add_event\n");
	{
#if 1
		// char printtime[RTC_NTIME_LENGTH+1];
		struct timespec ts;
		_Uint64t chkms;

		clock_gettime(CLOCK_REALTIME,&ts);
		chkms = (_Uint64t)ts.tv_sec*(_Uint64t)1000 
				+ (_Uint64t)ts.tv_nsec/(_Uint64t)1000000
				- (_Uint64t)21600;
		SLOG(SHOW,TEMP_mID, "chkms is %lld\n",chkms);
		char timestr[27];
		ctime_r(&ts.tv_sec,timestr);
		SLOG(SHOW,TEMP_mID,"%s",timestr);
#endif
		// access message with correct struct
		mq_send_add_event_msg * p_add_msg;
		p_add_msg = (mq_send_add_event_msg*)p_header;

		// SLOG(CONVFP,TEMP_mID,"send_add_event in sched_mgr, event_type is %ld\n",
				// p_add_msg->request.event_type);

		// add event to schedule or execute it now if immediate
		event_comp_t * p_comps;

		p_comps = new event_comp_t;
		
		p_comps->event_id.event_id = p_add_msg->request.event_id;
		SLOG(DIAG5,TEMP_mID,
			"event_id is %ld\n",p_comps->event_id.event_id);
		// SLOG(CONVFP,TEMP_mID,"event_id is %ld\n",p_comps->event_id.event_id);
		
		p_comps->event_id.owner_id = p_add_msg->owner_id;
		p_comps->owner_cookie = p_header->cookie;
#if 0
		char owner_id[RTC_HOSTNAME_LENGTH + 1];
		w2char_string_to_char((w2char_t *)&p_comps->event_id.owner_id[0],
						owner_id,RTC_HOSTNAME_LENGTH);
		owner_id[RTC_HOSTNAME_LENGTH] = 0;
		SLOG(DIAG5,TEMP_mID, "owner_id is %s\n",owner_id);
#endif
		// get exec time from input char string
		p_comps->exec_time.set_from_RTC_NTIMECODE(
						p_add_msg->request.event_time);
#if 0
		memcpy((void *)printtime,
				(const void *)&p_add_msg->request.event_time[0],
				RTC_NTIME_LENGTH);
		printtime[RTC_NTIME_LENGTH]=0;
		SLOG(DIAG5,TEMP_mID,
			"RTC_NTIME received for add scheduled event was %s\n",printtime);
#endif

		p_comps->removal_mode = (en_removal)0; // TODO
		// p_comps->device_id = p_add_msg->request.device_id;
		p_comps->attributes = 0;
		p_comps->request = p_add_msg->request;

		// p_comps->event_data = (char *)&p_add_msg->request.dev_data;

		// p_comps->data_size = sizeof(embeddedAddScheduledEventRequest);
		// p_comps->event_type = p_add_msg->request.event_type;
	
		// is event to be done immediately, or scheduled for later?
		if(p_add_msg->request.immediate == EXEC_IMMEDIATE)
		{
			// SLOG(CONVFP,TEMP_mID,"immediate event received\n");
			// create event outside of schedule for immediate event
			event* p_event = new event(*p_comps);
			// execute the event now, actually sends a message immediately
			// to dev_mgr so that the device in question can execute the 
			// device data contained in the event.
			p_event->run_event();
		//=========================
			delete p_event;
		}
		else
		{
			// put this event in the schedule so it will be executed at
			// its appointed exec time.
			rt_schedule.add_event(*p_comps);
		}
		// not needed, it has been copied into the event component struct
		// or run immediately
		delete p_comps;
#if 0
		p_reply_header = build_add_event_reply(p_header);
		
		if((size_sent = mq[dmq_from_sender_id[
				p_header->msg_sender_id]].rt_mq->send_to_mqueue(
				(char *)p_reply_header,(size_t)p_reply_header->msg_size))==-1)
		{
			SLOG(DIAG5,TEMP_mID,
				"ERROR: send_to_mqueue returned -1 in sm_proc_rcv_msg\n");
		}
		free(p_reply_header);
#endif		
	}
		break;
	case send_drop_event:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_drop_event\n");
	{
#if 0
		p_reply_header = build_delete_event_reply(p_header);
		
		if((size_sent = mq[smq_from_sender_id[
				p_header->msg_sender_id]].rt_mq->send_to_mqueue(
				(char *)p_reply_header,(size_t)p_reply_header->msg_size))==-1)
		{
			SLOG(DIAG5,TEMP_mID,
				"ERROR: send_to_mqueue returned -1 in sm_proc_rcv_msg\n");
		}
		free(p_reply_header);
#endif
		mq_send_drop_event_msg * p_drop_request;
			p_drop_request = (mq_send_drop_event_msg*)p_header;
			
		ms_event_id event_id;

		event_id.event_id = p_drop_request->request.event_id;
		event_id.owner_id = p_drop_request->owner_id;

#if 1
		char owner_id[RTC_HOSTNAME_LENGTH + 1];
		w2char_string_to_char((w2char_t *)&event_id.owner_id[0],
						owner_id,RTC_HOSTNAME_LENGTH);
		owner_id[RTC_HOSTNAME_LENGTH] = 0;
#endif
		SLOG(DIAG5,TEMP_mID,
			"calling remove event for event_id %ld and owner_id %s\n",
					event_id.event_id,owner_id);

		rt_schedule.remove_event((const ms_event_id &)event_id);
	}
		break;
#if 0
	case send_device_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_device_config\n");
		break;
	case send_time_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_time_config\n");
		break;
	case send_system_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_system_config\n");
		break;
	case send_port_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_port_config\n");
		break;
	case send_log_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_log_config\n");
		break;
	case request_port_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_port_config\n");
		break;
	case request_port_device_assignment:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_port_device_assignment\n");
		break;

	// message types sent from dev_mgr
	case reply_with_all_devices_status:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_all_devices_status\n");
		break;
	
	// message types sent from log_mgr
	case reply_with_log_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_log_config\n");
		break;

	// message types sent from msg_mgr

	// message types sent from proc_mgr
	case request_process_to_shutdown:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_process_to_shutdown\n");
		break;
	case request_process_to_start_operations:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_process_to_start_operations\n");
		break;
	case request_process_to_suspend_operations:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_process_to_suspend_operations\n");
		break;
	case request_process_to_reinit:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_process_to_reinit\n");
		break;

	// message types sent from sched_mgr
	case reply_with_all_event_structs:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_all_event_structs\n");
		break;
	case reply_with_one_event_struct:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_one_event_struct\n");
		break;
	case reply_with_next_event_time:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_next_event_time\n");
		break;
	case reply_with_latest_event_time:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_latest_event_time\n");
		break;
	case reply_with_number_of_events:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_number_of_events\n");
		break;
	case reply_with_all_one_device_event_structs:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_all_one_device_event_structs\n");
		break;
	case reply_with_total_event_capacity:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_total_event_capacity\n");
		break;
	case request_all_device_status:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_all_device_status\n");
		break;
	case request_current_timecode:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received request_current_timecode\n");
		break;
	case send_device_control_message:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received send_device_control_message\n");
		break;
	
	// message types sent from stat_mgr
	case reply_with_system_status:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_system_status\n");
		break;
	case reply_with_system_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_system_config\n");
		break;
	
	// message types sent from time_mgr
	case announce_vi_interrupt_with_tc_val:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received announce_vi_interrupt_with_tc_val\n");
		break;
	case reply_with_timecode:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_timecode\n");
		break;
	case reply_with_time_config:
		SLOG(DIAG2,SCHED_MSG_IN_mID,"Received reply_with_time_config\n");
		break;
#endif
	// message types sent from tc_drvr
	case tc_frame_notice:
		SLOG(DIAG2,TC_NOTICE_mID,"Received tc_frame_notice\n");
		// SLOG(CONVFP,TEMP_mID,"Received tc_frame_notice\n");
	{
		// get timecode from message and convert it
		mq_tc_frame_notice_msg * p_tcnot = (mq_tc_frame_notice_msg *)p_header;
		rtc_time tc_notice_time;
#if 1
		SLOG(DIAG5,TC_NOTICE_mID,
			"Received tc_notice %1d%1d:%1d%1d:%1d%1d:%1d%1d\n",
			p_tcnot->timecode.tens_hours,
			p_tcnot->timecode.ones_hours,
			p_tcnot->timecode.tens_mins,
			p_tcnot->timecode.ones_mins,
			p_tcnot->timecode.tens_secs,
			p_tcnot->timecode.ones_secs,
			p_tcnot->timecode.tens_frames,
			p_tcnot->timecode.ones_frames);
#endif
		// conversion in operator;
		tc_notice_time = p_tcnot->timecode;
		SLOG(DIAG4,TC_NOTICE_mID, 
			"tc_notice_time nsecs is %lld\n",tc_notice_time.rtc_ms);
		// SLOG(CONVFP,TEMP_mID,"tc_notice_time nsecs is %lld\n",tc_notice_time.rtc_ms);

		// run any events in schedule with time earlier than tc_notice_time
		rt_schedule.schedule_exec(tc_notice_time);
	}
		break;
	default:
		SLOG(DIAG5,TEMP_mID,
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
	int i;
	int n;
	size_t rcv_size;

	// variables for setting up select()
	fd_set readfds;
	
	while(1)
	{
		// prepare variables used as args to select()
		FD_ZERO(&readfds);
		int maxfd = 0;
		for(i=0;i<NUM_SMMQ_VALS;i++)
		{
			if((mq[i].rw == rw_read_only) || (mq[i].rw == rw_read_write))
			{
				FD_SET(mq[i].rt_mq->rt_mqueue_descr, &readfds);
				if(mq[i].rt_mq->rt_mqueue_descr > maxfd)
					maxfd = mq[i].rt_mq->rt_mqueue_descr;
			}
		}

		// wait for messages from any of the incoming message queues
		switch ( n = select ( maxfd+1, &readfds, 0, 0, NULL))
		{
		case -1:
			{
				int error_val = errno;
			SLOG(DIAG5,TEMP_mID, "ERROR: sched mq select failed - %d, %s\n",
							error_val, strerror(error_val));
			}
			break;
		case 0:
			SLOG(DIAG5,TEMP_mID, "select timed out\n");
			break;
		default:
			//SLOG(DIAG5,TEMP_mID, "%d descriptors ready in select\n",n);

			// loop through all message queues used in sched_mgr
			// look first for ones with read data ready
			for(i=0;i<NUM_SMMQ_VALS;i++)
			{
				if(FD_ISSET(mq[i].rt_mq->rt_mqueue_descr, &readfds))
				{
					ssize_t rcv_buf_size;
					char * rcv_buf;
					// get size used for this mq's messages
					if(mq[i].rt_mq->get_msg_size(&rcv_buf_size)==FAILURE)
					{
						SLOG(DIAG5,TEMP_mID,
						  "ERROR: failed to get msg size from rt_mqueue\n");
							abort();
					}
					// and allocate the buffer
					if((rcv_buf = (char *)malloc(rcv_buf_size))==NULL)
					{
						SLOG(DIAG5,TEMP_mID,
							"ERROR: failed to malloc for mq buf\n");
						abort();
					}

					SLOG(DIAG5,TEMP_mID,
						"%s has read data pending\n",
								rt_mq_names[mq[i].which]);
					// get message from rt_mqueue
					rcv_size = mq[i].rt_mq->receive_from_mqueue(rcv_buf);
					// do whatever is indicated in message
					sm_proc_rcv_msg(rcv_buf, rcv_size);
					// done with buffer
					free(rcv_buf);
				}
			}
			break;
		}
	}
	return(NULL);
};
//=============================================================================|
// schedule methods
//

schedule::schedule()
{
	event_count = 0;
	exec_time_of_next.rtc_ms = 0;
	exec_time_of_latest.rtc_ms = 0;
};

int	schedule::add_event(const event_comp_t & comps)
{
	const event_comp_t &test_comp = comps;
	int error_code = SUCCESS;
	event *event2add = new event(test_comp);
	SLOG(DIAG5,TEMP_mID, "new event has address %p\n",event2add);
	char owner_string[RTC_HOSTNAME_LENGTH+1];
	bzero(owner_string,RTC_HOSTNAME_LENGTH+1);

	w2char_string_to_char((w2char_t *)comps.event_id.owner_id,
					owner_string,
					(size_t)RTC_HOSTNAME_LENGTH);

	if(event2add==NULL)
	{
		SLOG(SHOW,TEMP_mID,
			"ERROR: null return from new event in schedule::add_event\n");
		error_code = FAILURE;
		goto ERROR_EXIT;
	}

	if(comps.exec_time.rtc_ms == 0)
	{
		SLOG(WARN,EVENT_mID,"Received immediate command\n");
	}
	else
	{

		SLOG(SHOW,TEMP_mID, "Adding event %ld, owner %s, exec_time %lld\n",
				comps.event_id.event_id, owner_string, comps.exec_time.rtc_ms);
	
	
		events.insert(pair<rtc_time,event*>(comps.exec_time, event2add));
	
		events_by_id.insert(pair<ms_event_id,event*>(comps.event_id, 
								event2add));
 		
		if(comps.exec_time.rtc_ms > exec_time_of_latest.rtc_ms)
				exec_time_of_latest = comps.exec_time;
	
		event_count++;
		SLOG(SHOW,TEMP_mID, "Schedule now has %d events after add\n",event_count);
		
#if 1
		// build event_scheduled message and send to asif_mgr
		ssize_t size_sent;
		size_t msg_size;
		mq_send_event_device_status_msg * p_msg;
		msg_size = sizeof(mq_send_event_device_status_msg);
		if((p_msg = (mq_send_event_device_status_msg *)malloc(msg_size))==NULL)
		{
			SLOG(DIAG5,TEMP_mID,
				"ERROR: malloc failed for send_event_device_status msg\n");
			abort();
		}
		p_msg->header = send_event_device_status_msg;
		p_msg->header.msg_id = mq_get_next_msg_id();
		p_msg->header.cookie = comps.owner_cookie;
		p_msg->response.event_id = comps.event_id.event_id;
		p_msg->response.condition = 1; // Scheduled
		snprintf((char*)&p_msg->response.dev_status.recorder_dev_status.time_reported,RTC_NTIME_LENGTH,
						"20200101123456789");
		snprintf((char*)&p_msg->response.dev_status.recorder_dev_status.time_code,RTC_NTIMECODE_LENGTH,
						"12345600");
		snprintf((char*)&p_msg->response.dev_status.recorder_dev_status.time_other,RTC_NTIMECODE_LENGTH,
						"12345600");
		p_msg->response.dev_status.recorder_dev_status.tape_status = 0;
		p_msg->response.dev_status.recorder_dev_status.error_code = 0;
		p_msg->response.dev_status.recorder_dev_status.device_id = event2add->components.device_id;
	
		{
			char idstr[32];
			w2char_string_to_char(&event2add->components.device_id[0],
							idstr,24);
			char hexstr[5124];
			hex_byte_dump_to_string(hexstr,5122,
				(char *)(&event2add->components.device_id[0]),48,31);
			SLOG(SHOW,SOCK_MSG_mID,"Sending event with device id %s\n%s\n",
				idstr,hexstr);
		}
		bzero(&p_msg->response.dev_status.recorder_dev_status.error_text,80);
	
		SLOG(DIAG2,SCHED_MSG_OUT_mID,"Sending event scheduled msg to asif\n");
		if(((size_t)size_sent = mq[SMMQ_TO_ASIF].rt_mq->send_to_mqueue(
				(char *)p_msg, msg_size)) != msg_size)
		{
			if(size_sent==-1)
			{
				int error_val = errno;
				SLOG(DIAG5,TEMP_mID, "send_to_mqueue failed with %d, %s\n",
							error_val, strerror(error_val));
			}
			else
			{
				SLOG(DIAG5,TEMP_mID, "ERROR: size sent %d instead of %d in "
								"process_complete_ms_msg\n",size_sent,msg_size);
			}
		}
		free(p_msg);
		p_msg = NULL;
#endif
	}
	

ERROR_EXIT:
	return error_code;
}; 

int schedule::remove_event(const ms_event_id & event_id)
{
	int error_code = SUCCESS;
	multimap< rtc_time, event*, less<rtc_time> >::iterator e_it;

	multimap< ms_event_id, event*, less<ms_event_id> >::iterator e_bid;

	rtc_time exec_time;
	event* p_event;

	SLOG(SHOW,TEMP_mID,
		"remove event %ld\n",event_id.event_id);

	// remove from multimap by event_id
	if((e_bid = events_by_id.find(event_id))!=events_by_id.end())
	{
		p_event = (*e_bid).second;
		exec_time = p_event->components.exec_time;
		events_by_id.erase(e_bid);
	}
	else
		error_code = FAILURE;
	
	// remove from multimap by exec_time
	if((e_it = events.find(exec_time))!=events.end())
	{
		p_event = (*e_it).second;
		events.erase(e_it);
	}
	else
		error_code = FAILURE;

	// if event was removed from both multimaps ok, delete it
	if(error_code == SUCCESS)
	{
		--event_count;
		SLOG(SHOW,TEMP_mID,
			"Schedule now has %d events after droping event %ld\n",
				event_count, p_event->components.event_id.event_id);
		delete p_event;
	}

	return error_code;
};

event * schedule::find_event(const ms_event_id & ms_event)
{
	event* p_event;
	multimap< ms_event_id, event*, less<ms_event_id> >::iterator e_bid;

	e_bid = events_by_id.find(ms_event);
	p_event = (*e_bid).second;
	return p_event;
};

event * schedule::find_event(const rtc_time & etime)
{
	event* p_event;
	multimap< rtc_time, event*, less<rtc_time> >::iterator e_it;

	e_it = events.find(etime);
	p_event = (*e_it).second;
	return p_event;
};

//=============================================================================|
// event methods
//

event::event(const event_comp_t &comps)
{
	_Uint16t TIME_STR_BUF_SIZE = 128;
	char buf[TIME_STR_BUF_SIZE];

	components = comps;

	time_str(components.exec_time,buf,TIME_STR_BUF_SIZE);
	remove_time.rtc_ms = components.exec_time.rtc_ms 
						 + default_removal_delay_ms;
	SLOG(DIAG5,TEMP_mID, "New event with exec_time of %s\n",buf);
};
	
int event::get_event_status(const event_status_t & status)
{
	int error_code = SUCCESS;

	return error_code;
};

int event::run_event()
{
	int result = SUCCESS;

	SLOG(DIAG4,EVENT_mID,"run_event called for event\n");
	// SLOG(CONVFP,TEMP_mID,"run_event called for event with type %ld\n",
					// components.event_type);

	// send event to it's device for execution
	mq_send_device_control_data_msg * p_dcd_msg;
	size_t msg_size = sizeof(mq_send_device_control_data_msg);
	ssize_t size_sent = 0;
	// get memory for message
	if((p_dcd_msg = (mq_send_device_control_data_msg *)malloc(msg_size))==NULL)
	{
		int error_val = errno;
		SLOG(DIAG5,TEMP_mID, "ERROR: failed to malloc %d bytes "
						"for mq_send_device_control_data_msg - %d, %s\n",
						msg_size, error_val, strerror(error_val));
		abort();
	}
	// copy canned header for this type message
	p_dcd_msg->header = send_device_control_data_msg;

	p_dcd_msg->header.msg_id = mq_get_next_msg_id();
	p_dcd_msg->request = components.request;

	// send to dev_mgr
	if((size_sent = mq[SMMQ_TO_DEV].rt_mq->send_to_mqueue(
		(char *)p_dcd_msg, msg_size)) != (ssize_t)msg_size)
	{
		if(size_sent==-1)
		{
			int error_val = errno;
			SLOG(DIAG5,TEMP_mID, "send_to_mqueue failed with %d, %s\n",
						error_val, strerror(error_val));
		}
		else
		{
			SLOG(DIAG5,TEMP_mID, "ERROR: size sent %d instead of %d in "
							"process_complete_ms_msg\n",size_sent,msg_size);
			result = FAILURE;
		}
	}
	else
		SLOG(CONVFP,TEMP_mID,"sent %d bytes to SMMQ_TO_DEV\n",size_sent);
	// done with message, clean up
	free(p_dcd_msg);
	p_dcd_msg = NULL;

	// assume event was executed successfully by dev_mgr
#if 1
	// build event_completed message and send to asif_mgr
	mq_send_event_device_status_msg * p_msg;
	msg_size = sizeof(mq_send_event_device_status_msg);
	if((p_msg = (mq_send_event_device_status_msg *)malloc(msg_size))==NULL)
	{
		SLOG(DIAG5,TEMP_mID,
			"ERROR: malloc failed for send_event_device_status msg\n");
		abort();
	}
	p_msg->header = send_event_device_status_msg;
	p_msg->header.msg_id = mq_get_next_msg_id();
	p_msg->header.cookie = components.owner_cookie;
	p_msg->response.event_id = components.event_id.event_id;
	p_msg->response.condition = 4; // event completed
	snprintf((char*)&p_msg->response.dev_status.recorder_dev_status.time_reported,RTC_NTIME_LENGTH,
					"20200101123456789");
	snprintf((char*)&p_msg->response.dev_status.recorder_dev_status.time_code,RTC_NTIMECODE_LENGTH,
					"12345600");
	snprintf((char*)&p_msg->response.dev_status.recorder_dev_status.time_other,RTC_NTIMECODE_LENGTH,
					"12345600");
	p_msg->response.dev_status.recorder_dev_status.tape_status = 0;
	p_msg->response.dev_status.recorder_dev_status.error_code = 0;
	p_msg->response.dev_status.recorder_dev_status.device_id = components.device_id;
	bzero(&p_msg->response.dev_status.recorder_dev_status.error_text,80);

	SLOG(SHOW,SCHED_MSG_OUT_mID,"Sending event completed msg to asif\n");
	if(((size_t)size_sent = mq[SMMQ_TO_ASIF].rt_mq->send_to_mqueue(
			(char *)p_msg, msg_size)) != msg_size)
	{
		if(size_sent==-1)
		{
			int error_val = errno;
			SLOG(DIAG5,TEMP_mID, "send_to_mqueue failed with %d, %s\n",
						error_val, strerror(error_val));
		}
		else
		{
			SLOG(DIAG5,TEMP_mID, "ERROR: size sent %d instead of %d in "
							"process_complete_ms_msg\n",size_sent,msg_size);
			result = FAILURE;
		}

	}
	free(p_msg);
	p_msg = NULL;
#endif

	SLOG(SHOW,TEMP_mID, "Sent event to device, now remove it\n");

	// now delete the event since it has been run
	rt_schedule.remove_event(components.event_id);

	return result;
};

//=============================================================================|
// schedule_exec -- waits for notifications from timer interrupt and runs
// 					any events who's exec_time is <= current time
//  System Implementation Note:
// 	The timer interrupt is from the Adrienne LTC Timecode reader card
//  and causes the interrupt handler to send a message to this thread.
//  The interrupt should come once per frame of video, or roughly 30 HZ
//  for NTSC and 25 HZ for PAL.
//
//  If finer granularity is needed later, a timer can be started for
//  sub-frame intervals.  If exec times are in milliseconds, they can be
//  executed at the first interval time past their exec time.
//
// NOTE: waiting is done by main select, msg handler for tc_frame_notice
// 		 calls this function.

int schedule::schedule_exec(const rtc_time & run_time)
{
	int error_code = SUCCESS;
	event * p_event;
	multimap< rtc_time, event*, less<rtc_time> >::iterator e_it;
	
	SLOG(DIAG3,EVENT_mID,
		"Looking for event with event_time <= %lld\n", run_time.rtc_ms);

	SLOG(DIAG5,EVENT_mID,"There are %d events in schedule\n",event_count);
	// run all events up to run_time
	for(e_it = events.begin();e_it != events.end(); ++e_it)
	{

		p_event = (*e_it).second;
		SLOG(DIAG3,EVENT_mID, "Looking at event at time %lld\n",
			p_event->components.exec_time.rtc_ms);
		if(p_event->components.exec_time.rtc_ms <= run_time.rtc_ms)
		{
			SLOG(DIAG3,EVENT_mID,
				"Event %ld is at %lld, less than %lld (now), run it\n",
					p_event->components.event_id.event_id,
					p_event->components.exec_time.rtc_ms,run_time.rtc_ms);
			if(p_event->run_event()!=SUCCESS)
			{
				fprintf(logfile,"ERROR: failed to run event\n");
				abort(); // TODO
			}
		}
		else
		{
			SLOG(DIAG2,EVENT_mID,
				" Event is at %lld, not less than %lld (now)\n",
				p_event->components.exec_time.rtc_ms,run_time.rtc_ms);
			// these are in order, so don't check any more after
			// the first one past run_time
			break;
		}
	}
	//SLOG(DIAG5,TEMP_mID,"=================================================\n");
	return error_code;
};

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	SLOG(DIAG5,TEMP_mID,
		"SCHED RECEIVED SIGUSR1 %d, restart sched_mgr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	SLOG(DIAG5,TEMP_mID, "SCHED RECEIVED SIGUSR2 %d, stop sched_mgr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//=============================================================================|
// 
//=============================================================================|
//
// This is the main thread of the schedule manager.  It starts a thread
// to wait for timer interrupts and execute events that have reached their
// exec time.  
// After starting that thread, it drops into a wait for commands loop 
// where it waits for commands from the automation system interface process.  
// Commands received will be either deferred or immediate in nature.  
// Deferred commands cause an event to be added to the schedule. Immediate 
// commands cause some immediate action such as returning a status response 
// or sending an immediate command to a device.

int main()
{
	int i;

	fprintf(stderr,"Starting sched_mgr\n");

#ifdef LOG_FILE
	if((logfile = fopen(LOG_FILE,"w+"))==NULL)
	{
		int error_val = errno;
		printf("ERROR: opening log file %s, use stderr, error %d, %s\n",
				LOG_FILE,error_val,strerror(error_val));
		logfile = stderr;
	}
#endif
	rt_set_logfile("SC",logfile);

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
		stop_requested = 0;

		//
		// instantiate and open all of the message queues to and from other procs
		//
		for(i=0;i<NUM_SMMQ_VALS;i++) 
		{
			if((mq[i].rt_mq = 
					new rt_mqueue(mq[i].which,mq[i].rw,mq[i].blk))==NULL)
			{
				fprintf(stderr,
					"ERROR: Failed to open message queue for %s in sched_mgr\n",
								rt_mq_names[mq[i].which]);
				abort();
			}
			else
			{
				fprintf(stderr, "opened mqueue %d for %s\n",
						mq[i].rt_mq->rt_mqueue_descr,
						rt_mq_names[mq[i].which]);
			}
		}
		embeddedInitLogging('S',&mq[SMMQ_TO_MSG]);

		running_schedule = 0;

		sleep(1);

		// send hanshake msg to asif and dev
		mq_hand_shake_msg handshake;
		handshake.header = hand_shake_msg;
		handshake.header.msg_sender_id = mq_user_sched;
		handshake.header.msg_receiver_id = mq_user_asif;
		mq[SMMQ_TO_ASIF].rt_mq->send_to_mqueue((char*)&handshake,
			sizeof(mq_hand_shake_msg));
		handshake.header.msg_receiver_id = mq_user_dev;
		mq[SMMQ_TO_DEV].rt_mq->send_to_mqueue((char*)&handshake,
			sizeof(mq_hand_shake_msg));

		// start thread to wait for frame time tick messages and run due events
		pthread_create(NULL, NULL, proc_msg_hndlr, NULL);

		int loop = 0;
		while(!stop_requested)
		{
			SLOG(DIAG3,SCHED_TICK_mID,"Tick - %d\n",loop++);
			sleep(1);
		}
		
		// tell schedule_exec thread to close down
		
	
		// close all of the message queues used
		for(i=0;i<NUM_SMMQ_VALS;i++) 
		{
			// don't shutdown mqueue from timecode card, it won't work
			// when restarted if stopped (figure out why later TODO)
			if(mq[i].which!=WMQ_TCD_TO_SCHED)
			{
				fprintf(stderr,
						"closing mqueue %s\n",rt_mq_names[mq[i].which]);
				delete mq[i].rt_mq;
			}
		}
	}

	// report shut-down condition

#ifdef LOG_FILE
	if(logfile!=stderr)
		fclose(logfile);
#endif

	// end
};

