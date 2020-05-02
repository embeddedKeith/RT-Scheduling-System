#ifndef _MSG_MGR_
#define _MSG_MGR_
//-----------------------------------------------------------------------------|
// msg_mgr.h
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/14/01
//
// last mod: 8/29/01
//-----------------------------------------------------------------------------|
#include <sys/timeb.h>
#include <process.h>
#include "rt_mqueue.hpp"

// list of mqueues used in msg_mgr, these names are a local nomenclature
// (local to the msg_mgr process)
//
enum
en_msg_mqueues
{
	MMMQ_TO_MSG = 0,
	NUM_MMMQ_VALS
};

// an array of this struct, indexed by en_msg_mqueues above, is used
// to keep track of just the mqueues used by msg_mgr
//

struct
msg_mqueue
{
	en_which_mqueue which;
	en_read_write	rw;
	en_blk			blk;
	rt_mqueue * 	rt_mq;
};

#endif // _MSG_MGR_
