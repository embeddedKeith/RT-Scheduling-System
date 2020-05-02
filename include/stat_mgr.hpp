#ifndef _STAT_MGR_
#define _STAT_MGR_
//-----------------------------------------------------------------------------|
// stat_mgr.h
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

// list of mqueues used in stat_mgr, these names are a local nomenclature
// (local to the stat_mgr process)
//
enum
en_stat_mqueues
{
	STMQ_TO_ASIF = 0,
	STMQ_FROM_ASIF,
	STMQ_TO_DEV, 
	STMQ_FROM_DEV,
	STMQ_TO_SCHED,
	STMQ_FROM_SCHED,
	NUM_STMQ_VALS
};

// an array of this struct, indexed by en_stat_mqueues above, is used
// to keep track of just the mqueues used by stat_mgr
//
struct
stat_mqueue
{
	en_which_mqueue which;
	en_read_write	rw;
	rt_mqueue * 	rt_mq;
};


#endif // _STAT_MGR_
