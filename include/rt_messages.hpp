#ifndef _RT_PROC_MSGS_
#define _RT_PROC_MSGS_
//-----------------------------------------------------------------------------|
// rt_proc_msgs.h
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/19/01
//
// last mod: 8/19/01
//-----------------------------------------------------------------------------|

enum
en_proc_msg_type
{
		MT_FRAME_TIC_TO_SCHED,
		MT_ASIF_CMD_TO_SCHED,
		MT_SCHED_REPLY_TO_ASIF,
		MT_SCHED_CMD_TO_DEV,
		MT_DEV_REPLY_TO_SCHED,
		MT_DEV_UPDATE_TO_STAT,
		MT_STAT_CMD_TO_DEV,
		MT_SCHED_UPDATE_TO_STAT,
		MT_STAT_CMD_TO_SCHED,
		NUM_MT_VALS
};


class
rt_proc_msg
{
private:
	en_proc_msg_type 		mtype;
	unsigned int			mdata_size;
	char *					data_buf;
public:
};

// proc_msg struct for add event proc_msg
struct
{
	owner_id_t owner_id;
	event_comp_t comps;
};
	
	
#endif // _RT_PROC_MSGS_
