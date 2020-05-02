#ifndef _LOG_MGR_
#define _LOG_MGR_
//-----------------------------------------------------------------------------|
// log_mgr.h
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

// an array of this struct, indexed by en_log_mqueues above, is used
// to keep track of just the mqueues used by log_mgr
//

struct
log_mqueue
{
	en_which_mqueue which;
	en_read_write	rw;
	rt_mqueue * 	rt_mq;
};

// list of mqueues used in asif_mgr, these names are a local nomenclature
// (local to the asif_mgr process)
//
enum
en_log_mqueues
{
	LMMQ_TO_MSG = 0,
	NUM_LMMQ_VALS
};


// an array of this struct is kept, one for each defined xxx_mID in nov_log.h
// nov_log.cpp uses this to tell what priority of logging to show, showing
// all messages with priority <= level in this struct.
//
struct
log_ctl_node
{
	char		module;
	char		level;
};
	
#endif // _LOG_MGR_
