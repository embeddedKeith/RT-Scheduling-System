#ifndef _RT_MQUEUE_
#define _RT_MQUEUE_
//-----------------------------------------------------------------------------|
// rt_mqueue.h
//
// Provides centralized point for access to POSIX mqueue functions
// so that any changes to use of this facility will be able to
// all change in one place.
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/17/01
//
// last mod: 8/27/01
//-----------------------------------------------------------------------------|
#include <sys/timeb.h>
#include <process.h>
#include <mqueue.h>
#include "rtc_time.hpp"
#include "asif_protocol.hpp"


// stuff to malloc before and after real mallocs to detect heap problems
#define CRAPPING
#ifdef CRAPPING
#define crapsize 1024
#define crapnum 100
void crap();
int crapchk();
#endif
//-----------------------------------------------------------------------------|
// miscellaneous support functions not specific to any message queue
_Uint32t mq_get_next_msg_id();
void destroy_mq_msg_id_lock_and_sem();

void rt_set_logfile(char*,FILE*);

//-----------------------------------------------------------------------------|
// mq_user_id used in header for msg_sender_id and msg_receiver_id;
//

enum 
en_mq_user_id
{
	mq_user_all = 0,
	mq_user_asif,
	mq_user_dev,
	mq_user_log,
	mq_user_msg,
	mq_user_proc,
	mq_user_sched,
	mq_user_stat,
	mq_user_time,
	mq_user_tcd,
	num_mq_user_vals
};

//-----------------------------------------------------------------------------|
enum
en_read_write
{
		rw_no_access = 0,
		rw_read_only, 
		rw_write_only,
		rw_read_write
};

enum
en_blk
{
		blk_blocking=0,
		blk_non_blocking
};

//-----------------------------------------------------------------------------|
// these are all of the proc-to-proc message queues
enum
en_which_mqueue
{
	WMQ_ALL_TO_MSG = 0,
	WMQ_ASIF_TO_DEV,
	WMQ_ASIF_TO_SCHED,
	WMQ_ASIF_TO_STAT,
	WMQ_DEV_TO_ASIF,
	WMQ_DEV_TO_SCHED,
	WMQ_DEV_TO_STAT,
	WMQ_SCHED_TO_ASIF,
	WMQ_SCHED_TO_DEV,
	WMQ_SCHED_TO_STAT,
	WMQ_SCHED_TO_TC,
	WMQ_STAT_TO_ASIF,
	WMQ_STAT_TO_DEV,
	WMQ_STAT_TO_SCHED,
	WMQ_STAT_TO_TC,
	WMQ_TC_TO_SCHED,
	WMQ_TC_TO_STAT,
	WMQ_TCD_TO_SCHED,
	WMQ_TCD_TO_STAT,
	WMQ_TCD_TO_TIME,
	NUM_WMQ_VALS     // = 20
};

//-----------------------------------------------------------------------------|
// separate message queues for each unique sender-receiver, directional
//

#ifdef KRD
const char * const rt_mq_names[NUM_WMQ_VALS] =
{
	"/krd_all_to_msg",			//  0
	"/krd_asif_to_dev",			//	1
	"/krd_asif_to_sched",		//	2
	"/krd_asif_to_stat",		//	3
	"/krd_dev_to_asif",			//	4
	"/krd_dev_to_sched",		//	5
	"/krd_dev_to_stat",			//	6
	"/krd_sched_to_asif",		//	7
	"/krd_sched_to_dev",		//	8
	"/krd_sched_to_stat",		//	9
	"/krd_sched_to_tc",			//	10
	"/krd_stat_to_asif",		//	11
	"/krd_stat_to_dev",			//	12
	"/krd_stat_to_sched",		//	13
	"/krd_stat_to_tc",			//	14
	"/krd_tc_to_sched",			//	15
	"/krd_tc_to_stat",			//	16
	"/krd_tcd_to_sched",		//  17
	"/krd_tcd_to_stat",			//  18
	"/krd_tcd_to_time"			//  19
};
#else
const char * const rt_mq_names[NUM_WMQ_VALS] =
{
	"/all_to_msg",			//  0
	"/asif_to_dev",			//	1
	"/asif_to_sched",		//	2
	"/asif_to_stat",		//	3
	"/dev_to_asif",			//	4
	"/dev_to_sched",		//	5
	"/dev_to_stat",			//	6
	"/sched_to_asif",		//	7
	"/sched_to_dev",		//	8
	"/sched_to_stat",		//	9
	"/sched_to_tc",			//	10
	"/stat_to_asif",		//	11
	"/stat_to_dev",			//	12
	"/stat_to_sched",		//	13
	"/stat_to_tc",			//	14
	"/tc_to_sched",			//	15
	"/tc_to_stat",			//	16
	"/tcd_to_sched",		//  17
	"/tcd_to_stat",			//  18
	"/tcd_to_time"			//  19
};
#endif
//-----------------------------------------------------------------------------|
#define NB O_NONBLOCK
static const struct mq_attr rt_mq_init_attr[NUM_WMQ_VALS] =
{
	// fields are:
	//mq_maxmsg
	//|      mq_msgsize
	//|      |      mq_flags
	//|      |      |   mq_curmsgs
	//|      |      |   |   mq_sendwait
	//|      |      |   |   |   mq_recvwait
	//|      |      |   |   |   |		(only set maxmsg and msgsize, rest 0)
	//V      V      V   V   V   V
	{32,	9000,	NB,	0,	0,	0},  //		WMQ_ALL_TO_MSG		0
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_ASIF_TO_DEV  	1
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_ASIF_TO_SCHED	2
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_ASIF_TO_STAT	3
	{4,		8192,	0,	0,	0,	0},	 // 	WMQ_DEV_TO_ASIF		4
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_DEV_TO_SCHED	5
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_DEV_TO_STAT		6
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_SCHED_TO_ASIF	7
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_SCHED_TO_DEV	8
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_SCHED_TO_STAT	9
	{16,	1024,	0,	0,	0,	0},	 // 	WMQ_SCHED_TO_TC		10
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_STAT_TO_ASIF	11
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_STAT_TO_DEV		12
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_STAT_TO_SCHED	13
	{4,		256,	0,	0,	0,	0},	 //		WMQ_STAT_TO_TC		14
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_TC_TO_SCHED		15
	{4,		256,	0,	0,	0,	0},	 // 	WMQ_TC_TO_STAT		16
	{16,	256,	NB,	0,	0,	0},	 //		WMQ_TCD_TO_SCHED	17
	{16,	256,	NB,	0,	0,	0},	 //		WMQ_TCD_TO_STAT		18
	{16,	256,	NB,	0,	0,	0}	 //		WMQ_TCD_TO_TIME		18
};
//-----------------------------------------------------------------------------|
// message types used by all processes in message queues
//

enum 
en_mq_msg_type
{
	// hand_shake is universal to all processes
	hand_shake = 0,
	send_log_control,				// set log level shown per mod_id
	send_rtc_sys_log,					// messages from nov_log to msg_mgr
	send_stdlog,
	// message types sent from asif_mgr
	request_system_status,			// also sent by sched_mgr
	request_all_devices_status,
	request_one_device_status,		// also sent by sched_mgr
	send_port_device_assignment,
	send_add_device,
	send_delete_device,
	send_subscription,
	request_timecode,
	request_all_event_brief,
	request_one_event,
	send_add_event,
	send_drop_event,
	send_device_config,
	send_time_config,
	send_system_config,
	send_port_config,
	send_log_config,
	request_port_config,
	request_port_device_assignment,

	// message types sent from dev_mgr
	reply_with_all_devices_status,
	
	// message types sent from log_mgr
	reply_with_log_config,

	// message types sent from msg_mgr

	// message types sent from proc_mgr
	request_process_to_shutdown,
	request_process_to_start_operations,
	request_process_to_suspend_operations,
	request_process_to_reinit,

	// message types sent from sched_mgr
	reply_with_all_event_structs,
	reply_with_one_event_struct,
	reply_with_next_event_time,
	reply_with_latest_event_time,
	reply_with_number_of_events,
	reply_with_all_one_device_event_structs,
	reply_with_total_event_capacity,
	reply_to_send_add_event,
	reply_to_send_drop_event,
	request_all_device_status,		// also sent by stat_mgr
	request_current_timecode,		// also sent by stat_mgr
	send_device_control_data,		// also sent by asif_mgr
	send_event_device_status,
	
	// message types sent from stat_mgr
	reply_with_system_status,
	reply_with_system_config,
	
	// message types sent from time_mgr
	announce_vi_interrupt_with_tc_val,
	reply_with_timecode,
	reply_with_time_config,

	// message types sent from tc_drvr
	tc_frame_notice,
	num_mq_msg_types
};

//-----------------------------------------------------------------------------|
// message header used for all mqueueu messages in realtime controller
//
struct
mq_msg_header
{
	_Uint32t				msg_size;
	en_mq_msg_type 			msg_type;
	en_mq_user_id			msg_sender_id;
	en_mq_user_id			msg_receiver_id;
	_Uint32t				msg_id;
	void * 					cookie;			// receiver sends back in reply

};

//-----------------------------------------------------------------------------|
enum en_handshake_type
{
	RTMQ_HS_REQ = 0,
	RTMQ_HS_REPLY,
};
struct mq_hand_shake_msg
{
	mq_msg_header				header;
	en_handshake_type			req_or_reply; // 0 for request, 1 for reply
};

const mq_msg_header hand_shake_msg =
{
	sizeof(mq_hand_shake_msg),		// msg_size 
	hand_shake,						// msg_type
	mq_user_all,					// msg_sender_id
	mq_user_all,					// msg_receiver_id
	0,								// msg_id
	NULL 							// cookie
};
	
//-----------------------------------------------------------------------------|
const mq_msg_header req_all_dev_status_msg =
{
	sizeof(mq_msg_header),			// msg_size 
	request_all_devices_status,		// msg_type
	mq_user_asif,					// msg_sender_id
	mq_user_dev,					// msg_receiver_id
	0,								// msg_id
	NULL 							// cookie
};
	
//-----------------------------------------------------------------------------|
struct mq_all_dev_reply_msg
{
	mq_msg_header				header;
	unsigned long				device_count;
	embeddedRTConfigurationResponse	response;
};

const mq_msg_header reply_with_all_dev_status_msg =
{
	sizeof(mq_msg_header),			// msg_size (will be adjusted where used)
	reply_with_all_devices_status,	// msg_type
	mq_user_dev,					// msg_sender_id
	mq_user_asif,					// msg_receiver_id
	0,								// msg_id
	NULL 							// cookie
};
	
//-----------------------------------------------------------------------------|

struct mq_send_add_device_msg
{
	mq_msg_header					header;
	RTC_HOSTNAME					owner_id;
	embeddedRTControllerAddDeviceRequest	request; 
};

const mq_msg_header send_add_device_msg =
{
	sizeof(mq_send_add_device_msg),			// msg_size
	send_add_device,						// msg_type
	mq_user_asif,							// msg_sender_id
	mq_user_dev,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};

//-----------------------------------------------------------------------------|

struct mq_send_delete_device_msg
{
	mq_msg_header					header;
	RTC_HOSTNAME					owner_id;
	embeddedRTControllerDeleteDeviceRequest	request; 
};

const mq_msg_header send_delete_device_msg =
{
	sizeof(mq_send_delete_device_msg),			// msg_size
	send_delete_device,						// msg_type
	mq_user_asif,							// msg_sender_id
	mq_user_dev,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};

//-----------------------------------------------------------------------------|

struct mq_send_add_event_msg
{
	mq_msg_header					header;
	RTC_HOSTNAME					owner_id;
	embeddedAddScheduledEventRequest 	request; 
};

const mq_msg_header send_add_event_msg =
{
	sizeof(mq_send_add_event_msg),			// msg_size
	send_add_event,							// msg_type
	mq_user_asif,							// msg_sender_id
	mq_user_sched,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};

//-----------------------------------------------------------------------------|

struct mq_send_drop_event_msg
{
	mq_msg_header					header;
	RTC_HOSTNAME					owner_id;
	embeddedDropScheduledEventRequest 	request; 
};

const mq_msg_header send_drop_event_msg =
{
	sizeof(mq_send_drop_event_msg),			// msg_size
	send_drop_event,						// msg_type
	mq_user_asif,							// msg_sender_id
	mq_user_sched,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};

//-----------------------------------------------------------------------------|
const mq_msg_header reply_to_add_event_msg =
{
	sizeof(mq_msg_header),			// msg_size
	send_add_event,					// msg_type
	mq_user_sched,					// msg_sender_id
	mq_user_asif,					// msg_receiver_id
	0,								// msg_id
	NULL 							// cookie
};

//-----------------------------------------------------------------------------|
struct mq_tc_frame_notice_msg
{
	mq_msg_header					header;
	tc_notice	 					timecode; 
};

const mq_msg_header tc_frame_notice_msg =
{
	sizeof(mq_send_drop_event_msg),			// msg_size
	tc_frame_notice,						// msg_type
	mq_user_tcd,							// msg_sender_id
	mq_user_sched,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};
	
//-----------------------------------------------------------------------------|
struct mq_send_device_control_data_msg
{
	mq_msg_header					header;
	embeddedAddScheduledEventRequest		request;
};

const mq_msg_header send_device_control_data_msg =
{
	sizeof(mq_send_device_control_data_msg),			// msg_size
	send_device_control_data,							// msg_type
	mq_user_sched,							// msg_sender_id
	mq_user_dev,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};
	
//-----------------------------------------------------------------------------|
struct mq_send_stdlog_msg
{
	mq_msg_header					header;
	size_t							buf_len;
	char							buf[1]; 
};

const mq_msg_header send_stdlog_msg =
{
	sizeof(mq_send_stdlog_msg),				// msg_size
	send_stdlog,							// msg_type
	mq_user_all,							// msg_sender_id
	mq_user_msg,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};
	
//-----------------------------------------------------------------------------|
struct mq_send_rtc_sys_log_msg
{
	mq_msg_header					header;
	char							module;
	unsigned char					priority;
	unsigned short					mod_id;
	size_t							buf_len;
	char							buf[1]; 
};

const mq_msg_header send_rtc_sys_log_msg =
{
	sizeof(mq_send_rtc_sys_log_msg),			// msg_size
	send_rtc_sys_log,							// msg_type
	mq_user_all,							// msg_sender_id
	mq_user_msg,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};
	
//-----------------------------------------------------------------------------|
struct mq_send_log_control_msg
{
	mq_msg_header					header;
	embeddedLogControlRequest			request;
};

const mq_msg_header send_log_control_msg =
{
	sizeof(mq_send_log_control_msg),		// msg_size
	send_log_control,						// msg_type
	mq_user_asif,							// msg_sender_id
	mq_user_log,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};
	
//-----------------------------------------------------------------------------|
struct mq_send_event_device_status_msg
{
	mq_msg_header					header;
	embeddedScheduledStatusResponse		response;
};

const mq_msg_header send_event_device_status_msg =
{
	sizeof(mq_send_event_device_status_msg),		// msg_size
	send_event_device_status,						// msg_type
	mq_user_sched,							// msg_sender_id
	mq_user_asif,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};

//-----------------------------------------------------------------------------|
struct mq_send_subscription_msg
{
	mq_msg_header					header;
	embeddedDevStatSubscriptionRequest	request;
};

const mq_msg_header send_subscription_msg =
{
	sizeof(mq_send_subscription_msg),		// msg_size
	send_subscription,						// msg_type
	mq_user_asif,							// msg_sender_id
	mq_user_dev,							// msg_receiver_id
	0,										// msg_id
	NULL 									// cookie
};
	
//-----------------------------------------------------------------------------|
// Class definition for rt_mqueue
//
// Separate instance exists for each client using a specific systme mqueue.
// Which mqueue is specified to instantiate, after which access is implicit by
// queue descriptor stored in private class member upon successful 
// instanciation.


class
rt_mqueue
{
private:
//
// index arrays by en_which_rt_mqueue 
//

en_which_mqueue which_mq; 			// saved when mqueue is opened
									// used for future indexing
// working attributes, same struct as rt_mq_init_attr above
struct mq_attr rt_mq_attr;



public:
		rt_mqueue(en_which_mqueue which, en_read_write rw, en_blk blk);
 		~rt_mqueue();

ssize_t	send_to_mqueue(char * buf, size_t size);
ssize_t	receive_from_mqueue(char* buf);
int		get_msg_size(ssize_t * p_size);
char *  get_rt_mq_name();

// 		flush_message_queue(en_which_queue which);



mqd_t rt_mqueue_descr;
};

//-----------------------------------------------------------------------------|
// an array of this struct, indexed by en_dev_mqueues above, is used
// to keep track of just the mqueues used by dev_mgr
//
struct mod_mqueue
{
	en_which_mqueue which;
	en_read_write	rw;
	en_blk			blk;
	rt_mqueue * 	rt_mq;
};


#endif // _RT_MQUEUE_
