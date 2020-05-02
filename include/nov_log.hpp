#ifndef _NOV_LOG_H_
#define _NOV_LOG_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	nov_log
//
//	Description:	
//
//	FileName:	nov_log.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/24/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#include <sys/cdefs.h>
#include "rt_mqueue.hpp"

#ifndef _CONFIG_H_
#include "config.hpp"
#endif

#ifndef _ERRCODES_H_
#include "errcodes.hpp"
#endif

#define MAX_STDLOG_STR_LENGTH 512

#define FATAL 1
#define WARN  2
#define DIAG2 2
#define DIAG3 3
#define DIAG4 4
#define DIAG5 5
#define ERROR FATAL
#define SHOW WARN
#define CONVFP 1


const char MODLETTERS[7] = {'A','D','L','M','P','S','T'};

#if 0

#define QLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'A', mId, fmt, ## args)

#define ALOG(pri, mId, fmt, args...) 
#define DLOG(pri, mId, fmt, args...)
#define LLOG(pri, mId, fmt, args...)
#define MLOG(pri, mId, fmt, args...)
#define PLOG(pri, mId, fmt, args...)
#define RLOG(pri, mId, fmt, args...)
#define SLOG(pri, mId, fmt, args...)
#define TLOG(pri, mId, fmt, args...)
#define ULOG(pri, mId, fmt, args...)
#define WLOG(pri, mId, fmt, args...)

#define AERR(e,pri,mId,fmt, args...)
#define DERR(e,pri,mId,fmt, args...)
#define LERR(e,pri,mId,fmt, args...)
#define MERR(e,pri,mId,fmt, args...)
#define PERR(e,pri,mId,fmt, args...)
#define RERR(e,pri,mId,fmt, args...)
#define SERR(e,pri,mId,fmt, args...)
#define TERR(e,pri,mId,fmt, args...)

#else

#define ALOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'A', mId, fmt, ## args)
#define DLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'D', mId, fmt, ## args)
#define LLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'L', mId, fmt, ## args)
#define MLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'M', mId, fmt, ## args)
#define PLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'P', mId, fmt, ## args)
#define RLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, MLET,mId, fmt, ## args)
#define SLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'S', mId, fmt, ## args)
#define TLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'T', mId, fmt, ## args)
#define ULOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, 'U', mId, fmt, ## args)
#define WLOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, MLET,mId, fmt, ## args)

#define AERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'A', mId, fmt, ## args)
#define DERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'D', mId, fmt, ## args)
#define LERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'L', mId, fmt, ## args)
#define MERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'M', mId, fmt, ## args)
#define PERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'P', mId, fmt, ## args)
#define RERR(e,pri,mId,fmt, args...) NLOG (e,        pri, MLET,mId, fmt, ## args)
#define SERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'S', mId, fmt, ## args)
#define TERR(e,pri,mId,fmt, args...) NLOG (e,        pri, 'T', mId, fmt, ## args)
#endif

#define LIB_LOG(pri, mId, fmt, args...) NLOG (NE_NOERR, pri, gProcID, mId, fmt, ## args)
#define LIB_ERR(e,pri,mId,fmt, args...) NLOG (e,        pri, gProcID, mId, fmt, ## args)

// #define TVLEN 202
// extern char tTdvect[TVLEN];
extern char gProcID;
extern int circFD;

/* reserved module id defs */
// #define mID_NUM_RESERVED   24
// #define mID_FIRST_ID       TVLEN - mID_NUM_RESERVED

// #define mID_FATAL          mID_FIRST_ID 
// #define mID_WARN           mID_FIRST_ID + 1

enum mIDs
{
	ALL_mID = 0,
	MISC_mID,
	LOG_CONTROL_mID,
	TC_NOTICE_mID,
	SHOW_mID,
	NPORT_READ_mID,
	NPORT_WRITE_mID,
	DEVICE_mID,
	DEV_CMD_mID,
	DEV_CONF_mID,
	DEV_HOLDER_mID,
	PROC_PKT_mID,
	THREAD_mID,
	EVENT_mID,
	LOCK_mID,
	HAND_SHAKE_mID,
	SOCK_MSG_mID,
	SOCKET_mID,
	MALLOC_mID,
	RTMQ_mID,
	RTMQ_IN_HEX_mID,
	RTMQ_OUT_HEX_mID,
	RT_MQUEUE_OPEN_mID,
	ROUTER_CMD_mID,
	ROUTER_DEV_mID,
	ROUTER_LOGIC_mID,
	RECORD_DEVICE_mID,
	SERIAL_RECORD_DEVICE_mID,
	SONY_RECORDER_mID,
	VDCP_CNTL_THRD_mID,
	VDCP_STATUS_mID,
	VDCP_STATE_FUNC_mID,
	VDCP_STATE_CHANGE_mID,
	VDCP_TIMECODE_mID,
	ASIF_MQ_MSG_IN_mID,
	ASIF_MQ_MSG_OUT_mID,
	MS_SOCK_MSG_IN_mID,
	MS_SOCK_MSG_OUT_mID,
	MS_SOCK_IN_HEX_mID,
	MS_SOCK_OUT_HEX_mID,
	DEV_MSG_IN_mID,
	DEV_MSG_OUT_mID,
	SCHED_MSG_IN_mID,
	SCHED_MSG_OUT_mID,
	SHOW_SONY_TC_mID,
	TEMP_mID,
	ASIF_TICK_mID,
	DEV_TICK_mID,
	SCHED_TICK_mID,
	STAT_SUBSCR_mID,
	NUM_APP_mIDs
};

/* When we ship code, set to to about 3 */
#ifdef PRODUCTION
#define COMPILE_PRI_LEVEL DIAG3
#else
#define COMPILE_PRI_LEVEL 255
#endif

#define NLOG(err, pri, m, mId, fmt, args...)		\
NLOG ## pri (err, pri, m, mId, fmt, ## args)

/* for now we support priority 0 - 7 */
#if (0 <= COMPILE_PRI_LEVEL)
#define NLOG0(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG0(err, pri, m, mId, fmt, args...)
#endif


#if 1 <= COMPILE_PRI_LEVEL
#define NLOG1(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG1(err, pri, m, mId, fmt, args...)
#endif


#if 2 <= COMPILE_PRI_LEVEL
#define NLOG2(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG2(err, pri, m, mId, fmt, args...)
#endif


#if 3 <= COMPILE_PRI_LEVEL
#define NLOG3(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG3(err, pri, m, mId, fmt, args...)
#endif


#if 4 <= COMPILE_PRI_LEVEL
#define NLOG4(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG4(err, pri, m, mId, fmt, args...)
#endif


#if 5 <= COMPILE_PRI_LEVEL
#define NLOG5(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG5(err, pri, m, mId, fmt, args...)
#endif


#if 6 <= COMPILE_PRI_LEVEL
#define NLOG6(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG6(err, pri, m, mId, fmt, args...)
#endif


#if 7 <= COMPILE_PRI_LEVEL
#define NLOG7(err, pri, m, mId, fmt, args...) \
     NOVUS_LOG (err, pri, m, mId, fmt, ## args)
#else
#define NLOG7(err, pri, m, mId, fmt, args...)
#endif

//	fprintf(stderr,"=========%d %d %d %d\n",mId,TVLEN,tTdvect[mId],pri); \

#define NOVUS_LOG(err, pri, m, mId, fmt, args...)		\
{ \
	embeddedLog (err, pri, m, mId, __FILE__, __LINE__, fmt , ## args); \
}

// {								\
    // if (mId>=0 && mId<TVLEN /* && tTdvect[mId] >= pri */)		\
  //   {								\
  //   }								\
// }


// __BEGIN_DECLS
void embeddedLog (int err,
               int pri,
               int mod,
               int mId,
               char *file,
               int line,
               char *fmt,
               ...);
int  embeddedInitLogging (char mLetter, mod_mqueue * p_mq);
void embeddedStopLogging (void);

int chgPri (char *);
int chgPriNumeric (int pri, int first, int last);
void PrintStdMIDMap (void);
// __END_DECLS

#endif /* _NOVUSLOG_H_ */
