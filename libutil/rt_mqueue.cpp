//-----------------------------------------------------------------------------|
//
// rt_mqueue.cpp
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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>
#include <strings.h>
#include <pthread.h>
#include <semaphore.h>
#include <limits.h>
#include <sys/mman.h>
#include "nov_log.hpp"
#include "rt_bool.hpp"
#include "rt_errors.hpp"
#include "rt_mqueue.hpp"

FILE* rt_logfile = stderr;

extern char mod_chars[4];
extern char MLET;

void
rt_set_logfile(char* mstr, FILE* logfile)
{
	rt_logfile = logfile;
	strncpy(mod_chars,mstr,2);
	mod_chars[2] = '\0';
};

//-----------------------------------------------------------------------------|
char* mq_user_name[num_mq_user_vals] =
{
	"mq_user_all",
	"mq_user_asif",
	"mq_user_dev",
	"mq_user_log",
	"mq_user_msg",
	"mq_user_proc",
	"mq_user_sched",
	"mq_user_stat",
	"mq_user_time",
	"mq_user_tcd"
};
//-----------------------------------------------------------------------------|
const char* mq_msg_type_name[num_mq_msg_types] =
{
	"hand_shake",
	"send_log_control",
	"send_rtc_sys_log",
	"send_stdlog",
	"request_system_status",
	"request_all_devices_status",
	"request_one_device_status",
	"send_port_device_assignment",
	"send_add_device",
	"send_delete_device",
	"request_timecode",
	"request_all_event_brief",
	"request_one_event",
	"send_add_event",
	"send_drop_event",
	"send_device_config",
	"send_time_config",
	"send_system_config",
	"send_port_config",
	"send_log_config",
	"request_port_config",
	"request_port_device_assignment",
	"reply_with_all_devices_status",
	"reply_with_log_config",
	"request_process_to_shutdown",
	"request_process_to_start_operations",
	"request_process_to_suspend_operations",
	"request_process_to_reinit",
	"reply_with_all_event_structs",
	"reply_with_one_event_struct",
	"reply_with_next_event_time",
	"reply_with_latest_event_time",
	"reply_with_number_of_events",
	"reply_with_all_one_device_event_structs",
	"reply_with_total_event_capacity",
	"reply_to_send_add_event",
	"reply_to_send_drop_event",
	"request_all_device_status",
	"request_current_timecode",
	"send_device_control_data",
	"send_event_device_status",
	"reply_with_system_status",
	"reply_with_system_config",
	"announce_vi_interrupt_with_tc_val",
	"reply_with_timecode",
	"reply_with_time_config",
	"tc_frame_notice"
};
//-----------------------------------------------------------------------------|

// stuff to malloc before and after real mallocs to detect heap problems

#ifdef CRAPPING
static char * craps[crapnum];
static int crapx = 0;
#endif
void crap()
{
#ifdef CRAPPING
	craps[crapx++]=(char *)malloc(crapsize);
	fprintf(stderr,"craps[%d] = %p\n",crapx-1,craps[crapx-1]); 
#endif
};
int crapchk()
{
#ifdef CRAPPING
	if(crapx>0)
			return (craps[0]==0)?1:0;
	else
			return 0;
#else
	return 0;
#endif
};

// #pragma pack(push)
#pragma pack(1)

// one array which holds all of the mqueues we use
static mqd_t rt_mqds[NUM_WMQ_VALS];

//-----------------------------------------------------------------------------|
// mq_get_next_msg_id -- thread-safe function to get next message id for a 
// mqueue message so that all messages can have consecutive message ids across 
// all mqueue messages
// Uses a counter in shared memory and a mutex to lock during read & inc cycle
//
static pthread_mutex_t  mq_msg_id_lock;
static int mq_msg_id_fd;
static unsigned int * mq_msg_id_addr = NULL;

void init_mq_msg_id_and_lock()
{
	int result = 0;
	// create lock, delete old one if existing and make new
	pthread_mutex_unlock(&mq_msg_id_lock);	// no check for error, probably
	pthread_mutex_destroy(&mq_msg_id_lock); // doesn't exist, OK either way
	if((result = pthread_mutex_init(&mq_msg_id_lock,NULL))!=EOK)
	{
		fprintf(stderr,"ERROR: pthread_mutex_init returned %d, not EOK\n", result);
		abort();
	}
	// incase not cleaned up from last time, remove previous
	//shm_unlink("/mq_msg_id");
	
	// create and initialize shared memory used for running msg id value
	if((mq_msg_id_fd = shm_open("/mq_msg_id",O_RDWR | O_CREAT, 0777))==-1)
	{
		int error_val = errno;
		fprintf(stderr,"ERROR: shm_open failed for mq_msg_id, errno %d, %s\n",
						error_val,strerror(error_val));
		abort();
	}
	// set the size of the shared memory mq_msg_id
	if(ftruncate(mq_msg_id_fd,sizeof(mq_msg_id_addr))==-1)
	{
		fprintf(stderr,"ERROR: failed to set size of shared mem for mq_msg_id\n");
		abort();
	}
	// map shared memory for mq_msg_id
	mq_msg_id_addr = (unsigned int *)mmap(0,sizeof(mq_msg_id_addr),
				PROT_READ | PROT_WRITE, MAP_SHARED, mq_msg_id_fd, 0);
	if(mq_msg_id_addr == MAP_FAILED)
	{
		fprintf(stderr,"ERROR: failed to map memory for shared mem mq_msg_id\n");
		abort();
	}
	fprintf(stderr,"Got memory at %p for shared mem mq_msg_id\n",mq_msg_id_addr);
	// set value to zero
	*mq_msg_id_addr = 0;
};

void destroy_mq_msg_id_and_lock()
{
	if(pthread_mutex_lock(&mq_msg_id_lock)!=EOK)
	{
		RLOG(DIAG5,TEMP_mID,
			"ERROR: pthread_mutex_unlock failed in mq_get_next_msg_id\n");
		abort();
	}

	close(mq_msg_id_fd);
	shm_unlink("/mq_msg_id");

	if(pthread_mutex_unlock(&mq_msg_id_lock)!=EOK)
	{
		RLOG(DIAG5,TEMP_mID,
			"ERROR: pthread_mutex_unlock failed in mq_get_next_msg_id\n");
		abort();
	}

	pthread_mutex_destroy(&mq_msg_id_lock);
};
	
// function to lock, get value, increment value, and unlock
_Uint32t mq_get_next_msg_id()
{
	if(pthread_mutex_lock(&mq_msg_id_lock)!=EOK)
	{
		RLOG(DIAG5,TEMP_mID,
			"ERROR: pthread_mutex_unlock failed in mq_get_next_msg_id\n");
		abort();
	}

	*mq_msg_id_addr += 1;
	// fprintf(stderr,"incremented mq_msg_id to %d\n",*mq_msg_id_addr);
	RLOG(DIAG5,TEMP_mID,"incremented mq_msg_id to %d\n",*mq_msg_id_addr);

	if(pthread_mutex_unlock(&mq_msg_id_lock)!=EOK)
	{
		RLOG(DIAG5,TEMP_mID,
			"ERROR: pthread_mutex_unlock failed in mq_get_next_msg_id\n");
		abort();
	}
	return (_Uint32t)*mq_msg_id_addr;
};

//-----------------------------------------------------------------------------|
// Methods for rt_mqueue class
//
// -------- CONSTRUCTOR --------
rt_mqueue::rt_mqueue(en_which_mqueue which, en_read_write rw, en_blk blk):
	rt_mqueue_descr(0)
{
	int o_flags = 0;
	int mode = 0;

	rt_mq_attr = rt_mq_init_attr[which];
	which_mq = which;

	if(mq_msg_id_addr==NULL)
	{
		_Uint32t waste_one;
		init_mq_msg_id_and_lock();
		waste_one = mq_get_next_msg_id();
		if(waste_one != 1)
			RLOG(WARN,TEMP_mID,
			"After initialization of mq_msg_id it is %d, not 1\n",waste_one);
	}
	// RLOG;
	// fprintf(rt_logfile,"mq_msg_id_sem at %p\n",p_mq_msg_id_sem);
#if 0
	// set flags and mode for this queue user as read, write, or read-write
	switch(rw)
	{
	case rw_read_only:
		o_flags = O_RDONLY | O_CREAT | ((blk==blk_non_blocking)?O_NONBLOCK:0);;
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		break;
	case rw_write_only:
		o_flags = O_WRONLY | O_CREAT | ((blk==blk_non_blocking)?O_NONBLOCK:0);;
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
		break;
	case rw_read_write:
#endif
		o_flags = O_RDWR | O_CREAT | ((blk==blk_non_blocking)?O_NONBLOCK:0);
		mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
#if 0
		break;
	default:
		RLOG(DIAG5,TEMP_mID,
			"ERROR: bad value for en_read_write in open_message_queue %d",rw);
		assert(0);
		break;
	}
#endif
	
	if((rt_mqds[which]=mq_open(rt_mq_names[which],o_flags, mode,
								&rt_mq_init_attr[which]))==-1)
	{
		 int error_val = errno;
		// RLOG(DIAG5,TEMP_mID,
			// "ERROR: Couldn't create mqueue %s, error %d, %s\n",
				// rt_mq_names[which],error_val,strerror(error_val));
		fprintf(rt_logfile,"ERROR: Couldn't create mqueue %s, error %d, %s\n",
				rt_mq_names[which],error_val,strerror(error_val));
	}
	else
	{
		// RLOG(DIAG5,TEMP_mID,
			// "Created mqueue %s\n",rt_mq_names[which]);

		rt_mqueue_descr = rt_mqds[which];
	}
};
// -------- DESTRUCTOR --------
rt_mqueue::~rt_mqueue()
{
	RLOG(DIAG5,TEMP_mID,
		"Closing mqueue %d\n",rt_mqueue_descr);
/*
	if(mq_unlink(rt_mq_names[which_mq])==-1)
	{
		RLOG(DIAG5,TEMP_mID,"ERROR: couldn't unlink mqueue for mqueue %s\n",
						rt_mq_names[which_mq]);
	}
*/
	if(mq_close(rt_mqueue_descr)==-1)
	{
		RLOG(DIAG5,TEMP_mID,
			"ERROR: couldn't close mqueue for mqd %d, mqueue %s\n",
						rt_mqueue_descr,rt_mq_names[which_mq]);
	}
}
// -------- send_to_mqueue --------
ssize_t rt_mqueue::send_to_mqueue(char * buf, size_t size)
{
	ssize_t size_sent = 0;
	if((long)size > rt_mq_init_attr[which_mq].mq_msgsize)
	{
		fprintf(stderr,
			"ERROR: send_to_mqueue got size %d, greater than max %d\n",
			(_Uint32t)size, (_Uint32t)rt_mq_init_attr[which_mq].mq_msgsize);
		abort();			
	}

	mq_attr look_mq_attr;
	mq_getattr(rt_mqueue_descr,&look_mq_attr);
	if(look_mq_attr.mq_curmsgs >= look_mq_attr.mq_maxmsg)
	{
		// fprintf(stderr,
			// "ERROR: send_to_mqueue mqueue has max of %ld messages already.\n",
				// look_mq_attr.mq_maxmsg);
		return(0);
	}

	// last arg is priority, jam to zero for now
	if((size_sent = mq_send(rt_mqueue_descr,buf,size,0))==-1)
	{
 		int error_val = errno;
		fprintf(stderr,
			"ERROR: Call to mq_send failed with error %d, %s\n",error_val,
				strerror(error_val));
		size_sent = -1;
	}
	else	// mq_send doesn't return actual size sent, if successful
	{
		size_sent = (ssize_t)size;	// assume size sent was size asked
		if(which_mq != WMQ_ALL_TO_MSG)
		{
			char hexstr[8192];
			hex_byte_dump_to_string(hexstr,8192,buf,size_sent,16);
			RLOG(DIAG3,RTMQ_OUT_HEX_mID,"dump of message sent to %s: %s\n",
					get_rt_mq_name(), hexstr);
		}
	}
	return(size_sent);
};
		
// -------- receive_from_mqueue --------
ssize_t		rt_mqueue::receive_from_mqueue(char * buf)
{
	unsigned int prio;
	ssize_t size_recvd = 0;
	size_t size = (size_t)rt_mq_init_attr[which_mq].mq_msgsize;

	bzero(buf,size);

	// char * p_name = get_rt_mq_name();
	// RLOG(CONVFP,TEMP_mID,"call to receive from  %s\n",p_name);

	if((size_recvd = mq_receive(rt_mqueue_descr,buf,size,&prio))==-1)
	{
		int error_val = errno;
		fprintf(stderr,"ERROR: Call to mq_receive failed with error %d, %s\n",
						 error_val, strerror(error_val));
	}
	if(which_mq != WMQ_ALL_TO_MSG)
	{
		char hexstr[8192];
		hex_byte_dump_to_string(hexstr,8192,buf,size_recvd,16);
		RLOG(DIAG3,RTMQ_IN_HEX_mID,"dump of message received from %s: %s\n",
					get_rt_mq_name(), hexstr);
	}
	return size_recvd;
};

//--------- get_msg_size ---------
int		rt_mqueue::get_msg_size(ssize_t * p_size)
{
	int error_val = SUCCESS;
	*p_size = (ssize_t)rt_mq_init_attr[which_mq].mq_msgsize;
	return error_val;
};

//--------- get_rt_mq_name ---------
char * rt_mqueue::get_rt_mq_name()
{
	return (char *)&rt_mq_names[which_mq][0];
};


//-----------------------------------------------------------------------------|
// -------- MAIN --------
#undef DO_MQUEUE_TESTS 
#ifdef DO_MQUEUE_TESTS 
int
main(int argc, char** argv)
{

	// test message queues 

	return(1);
}
// #pragma pack(pop)
#endif

