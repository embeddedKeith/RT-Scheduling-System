#ifndef _TIME_MGR_
#define _TIME_MGR_
//-----------------------------------------------------------------------------|
// time_mgr.h
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

// list of mqueues used in time_mgr, these names are a local nomenclature
// (local to the time_mgr process)
//
enum
en_time_mqueues
{
	TMMQ_TO_MSG = 0,
	TMMQ_TO_DEV,
	TMMQ_FROM_DEV,
	TMMQ_TO_SCHED,
	TMMQ_FROM_SCHED,
	TMMQ_TO_STAT,
	TMMQ_FROM_STAT,
	NUM_TMMQ_VALS
};

// an array of this struct, indexed by en_time_mqueues above, is used
// to keep track of just the mqueues used by time_mgr
//
struct time_mqueue
{
	en_which_mqueue which;
	en_read_write	rw;
	rt_mqueue * 	rt_mq;
};



#endif // _TIME_MGR_
