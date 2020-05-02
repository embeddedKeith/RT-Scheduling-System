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
//	FileName:	nov_log.cpp
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/24/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdarg.h>
#include <strings.h>
#include <time.h>
#include <assert.h>
#include "rt_errors.hpp"
#include "rt_mqueue.hpp"
#include "w2char.hpp"
#include "config.hpp"
#include "errcodes.hpp"
#include "errstrings.hpp"
#include "circular.hpp"
#include "tset.hpp"
#include "asif_mgr.hpp"
#include "device.hpp"
#include "log_mgr.hpp"
#include "msg_mgr.hpp"
#include "proc_mgr.hpp"
#include "sched_mgr.hpp"
#include "tc_drvr.hpp"
#include "nov_log.hpp"


#define LOGBUFSIZE 8192
// static char buf [LOGBUFSIZE];
// static char mbuf [LOGBUFSIZE];

mod_mqueue log_mqueue;
char MLET = 'R';

extern char stderr_str[];
extern char mod_chars[];

int circFD = 1;  /* by default, log to stdout */

char* mID_name[NUM_APP_mIDs] =
{
	"ALL_mID",
	"MISC_mID",
	"LOG_CONTROL_mID",
	"TC_NOTICE_mID",
	"SHOW_mID",
	"NPORT_READ_mID",
	"NPORT_WRITE_mID",
	"DEVICE_mID",
	"DEV_CMD_mID",
	"DEV_CONF_mID",
	"DEV_HOLDER_mID",
	"PROC_PKT_mID",
	"THREAD_mID",
	"EVENT_mID",
	"LOCK_mID",
	"HAND_SHAKE_mID",
	"SOCK_MSG_mID",
	"SOCKET_mID",
	"MALLOC_mID",
	"RTMQ_mID",
	"RTMQ_IN_HEX_mID",
	"RTMQ_OUT_HEX_mID",
	"RT_MQUEUE_OPEN_mID",
	"ROUTER_CMD_mID",
	"ROUTER_DEV_mID",
	"ROUTER_LOGIC_mID",
	"RECORD_DEVICE_mID",
	"SERIAL_RECORD_DEVICE_mID",
	"SONY_RECORDER_mID",
	"VDCP_CNTL_THRD_mID",
	"VDCP_STATUS_mID",
	"VDCP_STATE_FUNC_mID",
	"VDCP_STATE_CHANGE_mID",
	"VDCP_TIMECODE_mID",
	"ASIF_MQ_MSG_IN_mID",
	"ASIF_MQ_MSG_OUT_mID",
	"MS_SOCK_MSG_IN_mID",
	"MS_SOCK_MSG_OUT_mID",
	"MS_SOCK_IN_HEX_mID",
	"MS_SOCK_OUT_HEX_mID",
	"DEV_MSG_IN_mID",
	"DEV_MSG_OUT_mID",
	"SCHED_MSG_IN_mID",
	"SCHED_MSG_OUT_mID",
	"SHOW_SONY_TC_mID",
	"TEMP_mID",
	"ASIF_TICK_mID",
	"DEV_TICK_mID",
	"SCHED_TICK_mID",
	"STAT_SUBSCR_mID"
};

static char* err_string (int err)
{
    char *result;

/* 
 * Instead of asserting, check the err code.
 * If it's not valid change it to NE_INVALID_ERRCODE
 */
	if((( err <= 0 ) || (err > NE_LAST_CODE )) ||
    ((err > ELAST && err < NE_FIRST_CODE)))

	{
		err = NE_INVALID_ERRCODE;
	}
/*    assert (err >= 0 && err < NE_LAST_CODE);
 *    assert (!(err > ELAST && err < NE_FIRST_CODE));
 */
        
    if (err == 0)
    result = "";

    else if (err <= ELAST)
    result = strerror (err);

    else
    result = errStrings [err - NE_FIRST_CODE];

    return result;
}

//static int  shmid;      /* shared memory ID */
//static int  semid;      /* shared semaphore ID */

/*
 * Semaphore operation buffers for acquiring
 * and releasing the message buffer semaphore.
 */
#if 0
static struct sembuf sem_acquire = { /*member*/0, /*op*/-1, /*flags*/0 };
static struct sembuf sem_acquire_nowait = { /*member*/0, /*op*/-1,
											/*flags*/IPC_NOWAIT };
static struct sembuf sem_release = { /*member*/0, /*op*/+1, /*flags*/0 };
#endif

//static MSGBUF  *mbp;

/* Dump the contents of the circular buffer to stdout
 * starting at the read pointer going up through the
 * write pointer.  Handle a possible wrap around.
 * Modify the read pointer to reflect that the contents
 * have been read and dumped.
 */

//static int shmActive = 0;

int
circDump (void)
{
    int err;
#if 0
    long len;
    static char lostmsg[] = "\nDATA LOST\n";
    static char lockmsg[] = "\nCONTESTED LOCK (logging): (not a problem)\n";
    

    assert(shmActive ==1);

        /* get lock on shared memory */
    // lockResource(&mbp->msg_lock, &mbp->msg_busycnt);  

    len = mbp->msg_bufx - mbp->msg_bufr;

    if(mbp->msg_busycnt) {
        if( write(circFD, lockmsg, strlen(lockmsg)) != 
                            strlen(lockmsg)) {
            fprintf(stderr, 
                    "circDump: write(circFD, lockmsg FAILED: %s\n",
                    strerror (errno));
            err = errno;
            goto ERROR_EXIT;
        }
    }
    if(mbp->msg_flags & MSG_DATA_LOST) {
        mbp->msg_flags &= ~MSG_DATA_LOST;
        if( write(circFD, lostmsg, strlen(lostmsg)) != 
                            strlen(lostmsg)) {
            fprintf(stderr, 
                    "circDump: write(circFD, lostmsg FAILED %s\n",
                    strerror (errno));
            err = errno;
            goto ERROR_EXIT;
        }
    }

    /* Three possibilities: */

    if (len == 0) {
        ; /* printf("circDump: empty buffer\n");*/
    }

    else if (len > 0) {  /* simple: one chunk of contiguous data */
        if(write(circFD, &mbp->msg_bufc[mbp->msg_bufr],len)!=len) {
            fprintf(stderr, 
                    "circDump: (write(msg_bufc[mbp->msg_bufr] FAILED %s\n",
                    strerror (errno));
            err = errno;
            goto ERROR_EXIT;
        }
        mbp->msg_bufr += len;
        if (mbp->msg_bufr >= mbp->msg_size)
            mbp->msg_bufr = 0;
    }

    else /* len < 0 */ {  /* Two pieces of data */
        /* earlier part first: */
        len = mbp->msg_size - mbp->msg_bufr;
        if(write(circFD, &mbp->msg_bufc[mbp->msg_bufr],len)!=len) {
            fprintf(stderr, 
                    "circDump: write(msg_bufc[mbp->msg_bufr] FAILED2 %s\n",
                    strerror (errno));
            err = errno;
            goto ERROR_EXIT;
        }

        /* now later: */
        mbp->msg_bufr = 0;
        len = mbp->msg_bufx - mbp->msg_bufr;
        if(write(circFD, &mbp->msg_bufc[mbp->msg_bufr],len)!=len) {
            fprintf(stderr, 
                    "circDump:write(msg_bufc[mbp->msg_bufr] FAILED3 %s\n",
                    strerror (errno));
            err = errno;
            goto ERROR_EXIT;
        }
        mbp->msg_bufr += len;
    }

    // freeResource(&mbp->msg_lock, &mbp->msg_busycnt);

    err = 0;

  ERROR_EXIT:
#endif
    return err;
}

/* 
 * Copy the characters referenced by 'p' onto the circular Q.
 * The number of characters is 'n'.
 * Handle wrap around and modify the write pointer upon completion.
 * If buffer is full, advance the read pointer, clipping data, and
 * note that fact.
 */
#define MAXMESLEN 256
void
circWrite(const char *p, int n) {
#if 0
	int copy1_len;
	int copy2_len;
	int distance_to_full;

    if(shmActive !=1){
        char tmpbuf[MAXMESLEN];
        if(n>=MAXMESLEN) n=MAXMESLEN;
        memset(tmpbuf, 0, MAXMESLEN);
        strncpy(tmpbuf,p, n-1);
        fprintf(stderr, "circWrite: Shared memory is not currently active\n");
        fprintf(stderr, "--%s\n", tmpbuf);
        fprintf(stderr, "-- Process ID=%d\n", (int)getpid());
		return;
    }

    assert (shmActive == 1);
    if(n >= mbp->msg_size)
	{
		fprintf(stderr,"=========== n is %d, msg_size is %d",n,mbp->msg_size);
		assert(0);
	};
    assert (mbp->msg_bufx < mbp->msg_size);
    assert (mbp->msg_bufr < mbp->msg_size);

        /* get lock on shared memory */
    // lockResource(&mbp->msg_lock, &mbp->msg_busycnt);  

#if 1
	copy1_len = n;
	copy2_len = (mbp->msg_bufx + n) - mbp->msg_size;

	/*
	 * If this action will overwrite data that hasn't
	 * been committed, keep the most recent data.
	 */
	if (mbp->msg_bufr > mbp->msg_bufx)
		distance_to_full = mbp->msg_bufr - mbp->msg_bufx;
	else
		distance_to_full = mbp->msg_size - mbp->msg_bufx + mbp->msg_bufr;

	if (distance_to_full <= n ) {
            mbp->msg_flags = MSG_DATA_LOST;
		mbp->msg_bufr = copy2_len > 0 ? copy2_len : copy1_len;
    }
	if (copy2_len > 0)
		copy1_len = n - copy2_len;
	bcopy(p, &mbp->msg_bufc[mbp->msg_bufx], copy1_len);
	p += copy1_len;
	mbp->msg_bufx += copy1_len;

	if (copy2_len > 0)
		bcopy(p, &mbp->msg_bufc, copy2_len);

	if (copy2_len >= 0)
		mbp->msg_bufx = copy2_len;
#else
{
	int i;
	for(i=0; i<n; i++){
        mbp->msg_bufc[mbp->msg_bufx++] = *p++;
        if (mbp->msg_bufx >= mbp->msg_size)
            mbp->msg_bufx = 0;      /* wrap to start of buffer */

        /* If the buffer is full, keep the most recent data. */
        if (mbp->msg_bufr == mbp->msg_bufx) {
            mbp->msg_flags = MSG_DATA_LOST;
            if (++mbp->msg_bufr >= mbp->msg_size)
                mbp->msg_bufr = 0;
        }
    }
}

#endif

//     freeResource(&mbp->msg_lock, &mbp->msg_busycnt);  
#endif
}


/* Log a message. Time stamp it. For now we format the whole mess and
   send it to stderr. Soon we will just forward it on to a log process
   for filtering & routing.
 */
void embeddedLog (int err,
               int pri,
               int module,
               int modId,
               char *file,
               int line,
               char *fmt,
               ...)
{
    va_list ap;
    struct tm *t;
    char *errStr;
    struct timespec timeStamp;
    int n;
	int filepos;

	static bool line_continue[256];
	static char continue_mod = 'x';
	static bool print_info = true;
	static bool first_time = true;

	if(first_time)
	{
		for(int ift=0;ift<256;ift++)
			line_continue[ift]=false;
		first_time = false;
	}

	// fprintf(stderr,"===========arrived in embeddedLog==========\n");

	char* buf;

	if((buf = (char*)malloc(LOGBUFSIZE))==NULL)
		abort();

    n = clock_gettime(CLOCK_REALTIME,&timeStamp);
    assert (n != -1);
    va_start (ap, fmt);
    vsnprintf (buf, LOGBUFSIZE, fmt, ap);
    va_end (ap);

	char* mbuf;

	if((mbuf = (char*)malloc(LOGBUFSIZE))==NULL)
		abort();

    /* everything below this point can be handed off to the log process */
    /* for now only output the current time to msec accuracy, not the date */
    t = localtime (&timeStamp.tv_sec);
    errStr = err_string (err);

	for(filepos=strlen(file);((file[filepos]!='/')&&(filepos>=0));--filepos)
			;
	// base info printing this time on continue vars set last time
	print_info = (line_continue[module]==true && continue_mod==module) 
			? false : true;

	// now set continue vars for next time
	char endstr[4];
	// if last char of buf is '\' or '\n' then don't do line feed,
	snprintf(endstr,3,"%s", 
		((buf[strlen(buf)-1]=='\\')||(buf[strlen(buf)-1]=='\n'))?"":"\n");
	// if '\' then don't show it
	if(buf[strlen(buf)-1]=='\\')
	{
		line_continue[(int)module] = true;
		continue_mod = module;
		buf[strlen(buf)-1]='\0';
	}
	else
	{
		line_continue[(int)module] = false;
		continue_mod = 'x';
	}
	if(!print_info)
	{
		n = snprintf (mbuf,LOGBUFSIZE,"%s%s",buf,endstr);
	}
	else
	{
    	n = snprintf (mbuf,LOGBUFSIZE,
                  "%02d:%02d:%02d.%04ld %c%3d:%1d %-16.16s %4d: %s%s%s%s",
                  t->tm_hour,
                  t->tm_min,
                  t->tm_sec,
                  timeStamp.tv_nsec / 100000,
                  module,
                  modId,
                  pri,
                  &(file[filepos+1]),
                  line,
				  buf,
                  errStr ? ": " : "",
                  errStr ? errStr : "", 
				  endstr);
	}

	free(buf);

	if(strlen(mbuf)>(LOGBUFSIZE-32))
	{
		mbuf[(LOGBUFSIZE-33)]='\n';
		mbuf[(LOGBUFSIZE-32)]='\0';
	}

	mq_send_rtc_sys_log_msg * p_rtc_sys_log_msg;
	size_t msg_len = strlen(mbuf);
	p_rtc_sys_log_msg = (mq_send_rtc_sys_log_msg *)malloc(
		sizeof(mq_send_rtc_sys_log_msg)+msg_len);
	p_rtc_sys_log_msg->header = send_rtc_sys_log_msg;
	p_rtc_sys_log_msg->module = module;
	p_rtc_sys_log_msg->mod_id = modId;
	p_rtc_sys_log_msg->priority = pri;
	p_rtc_sys_log_msg->buf_len = msg_len;
	p_rtc_sys_log_msg->header.msg_size = sizeof(mq_send_rtc_sys_log_msg) + msg_len;
	snprintf((char*)&(p_rtc_sys_log_msg->buf[0]),msg_len,"%s\n",mbuf);
	free(mbuf);
	if(log_mqueue.rt_mq==NULL)
	{
		fprintf(stderr,"NOMQ: %s",(char*)&p_rtc_sys_log_msg->buf[0]);
	}
	else
	{
		log_mqueue.rt_mq->send_to_mqueue((char *)p_rtc_sys_log_msg,
						(size_t)(p_rtc_sys_log_msg->header.msg_size));
	}
	free(p_rtc_sys_log_msg);

    // circWrite(mbuf,n);
    /*write(1,mbuf,n);*/
}


// char  tTdvect[TVLEN];
char gProcID = ' ';

#define MUTEX_AVAIL		0x0
#define MUTEX_BUSY		0x1
#define MUTEX_WAITING	0x2

int
CreateSharedResources (int bufSize)
{
    int err;
#if 0
    int size;
    int init_level;
    union semun semarg;

    init_level = 0;

    /*
     * Create the shared memory segment, if required,
     * then attach it.
     */
    
    size = bufSize > 0 ? bufSize : MSG_BSIZE;

    if ( (shmid = shmget(SHMKEY, size, PERMS | IPC_CREAT)) < 0){
        perror("CreateSharedResources: can't get shared memory");
        err = errno;
        goto ERROR_EXIT;
    }

    init_level++;

    if ( (mbp = (MSGBUF *) shmat(shmid, (char *) 0, 0)) == (MSGBUF *) -1){
        perror("CreateSharedResources: can't attach shared memory");
        err = errno;
        goto ERROR_EXIT;
    }

    init_level++;

    mbp->msg_magic = MSG_MAGIC;
    mbp->msg_size  = size - sizeof (*mbp);
    mbp->msg_bufx  = 0;
    mbp->msg_bufr  = 0;
    mbp->msg_flags = 0;
    mbp->msg_lock  = MUTEX_AVAIL;
    mbp->msg_busycnt=0;

    mbp->bsc_lock    = MUTEX_AVAIL;
    mbp->bsc_busycnt = 0;

    /*
    ** Create our semaphore.
    */
    if ( (semid = semget(SEMKEY, /*nsems*/1, PERMS | IPC_CREAT)) == -1) {
	perror("CreateSharedResources: can't create shared semaphore");
	err = errno;
	goto ERROR_EXIT;
    }

    init_level++;

    /*
    ** Set the semaphore to indicate that there
    ** is a single resource to be manaaged.
    */
    semarg.val = 1;
    if (semctl(semid, /*sem number in set*/0, SETVAL, semarg) != 0) {
	perror("CreateSharedResources: can't initialize shared semaphore");
	err = errno;
	goto ERROR_EXIT;
    }

    init_level++;

    /*
    ** Check that the number of entries in errStrings
    ** is equal to the number of entries in the errCode enum
    ** String is terminated with a NULL entry. (It also begins
    ** with a NULL entry so skip the first one) 
    */
    size = 1 ;      
    while( errStrings[size++] != NULL )
	;

    if( --size != (NE_LAST_CODE - NE_NOERR)) {
	fprintf(stderr, "CreateSharedResources: Someone forgot to add error "
		"string(s) for their error code(s)\n" );
	err = NE_ARGMISSING;
	goto ERROR_EXIT;
    }

    init_level++;

    err = 0;

  ERROR_EXIT:

    switch (init_level) {
    default:
    case 4:
    case 3:
	if (semctl(semid, /*sem number in set*/0, IPC_RMID, semarg) == -1)
		perror("CreateSharedResources: can't remove shared semaphore");
    case 2:
	if (shmdt(mbp) == -1) 
		perror("CreateSharedResources: can't detach shared memory");
    case 1:
	if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) == -1)
		perror("CreateSharedResources: can't remove shared memory");
	break;
    case 5:
    case 0:
	/* Success or nothing to clean up */
	break;
    }
#endif
    return err;
}

#if 0
int
embeddedInitLogging (char procID)
{
    int err;
    int i;

    gProcID = procID;
    
    // for(i=0; i<TVLEN; i++)
    // tTdvect[i] = 5;
    
    if ( (shmid = shmget(SHMKEY, sizeof(MSGBUF), 0)) < 0){
        perror("embeddedInitLogging: client can't get shared memory");
        err = errno;
        goto ERROR_EXIT;
    }

    if ( (mbp = (MSGBUF *) shmat(shmid, (char *) 0, 0)) == (MSGBUF *) -1){
        perror("embeddedInitLogging client: can't attach shared memory");
        err = errno;
        goto ERROR_EXIT;
    }

    if ( (semid = semget(SEMKEY, /*nsems*/0, /*flags*/0)) == -1) {
	perror("embeddedInitLogging: client can't get shared semaphore");
	err = errno;
	goto ERROR_EXIT;
    }

    shmActive = 1;

    err = 0;

  ERROR_EXIT:
    return err;
}
#else
int
embeddedInitLogging(char mLetter, mod_mqueue * p_mq)
{
	if(p_mq!=NULL)
	{
		log_mqueue = *p_mq;
		MLET = mLetter;
		fprintf(stderr,"Logging initialized with MLET %c\n",MLET);
		return SUCCESS;
	}
	return FAILURE;

};
#endif


/******************************************
debug_option : '-d' [mod_name] pri ':' range
mod_name     : 'A' | 'B' | 'F' | 'M' | 'R'
pri          : uchar
range        : uchar '-' uchar | uchar '-'

Examples:
          -dR3:25-33  -dM4:0-
Note, mod_name not implemented below yet.
******************************************/

#if 0
int chgPri(char *s)
{
    unsigned int first, last, pri;
        register unsigned int i;

    /* handle the optional mod_name */
    /* don't do anything with it - higher levels worry */
    if(*s == 'A' || *s == 'B' || *s == 'F' || *s == 'M' || *s == 'R' || 
		*s == 'E')
        s++;

    /* get priority */
    i = 0;
    while (isdigit(*s))
        i = i * 10 + (*s++ - '0');
    pri = i;
    if( pri > 255)
        return -1;
    if (*s++ != ':')
        return -1;

    /* find first flag to set */
    i = 0;
    while (isdigit(*s))
        i = i * 10 + (*s++ - '0');
    first = last = i;
    if(first>=TVLEN)  /* make sure in range */
        return -1;

    /* find last flag to set */
    if (*s++ == '-')
    {
        last = TVLEN-1;  /* assume open ended range */
        i = 0;
        while (isdigit(*s))
            i = i * 10 + (*s++ - '0');
        if(i>0 && i<TVLEN)
            last = i; /* closed range */
    }

    FLOG (5, 150, "chgPri first=%d lst=%d", first,last);
    /* set the flags */
    while (first <= last)
        tTdvect[first++] = pri;
    return 0;
}
#endif


#if 0
/* The same thing in a machine friendly format */
int chgPriNumeric (int pri, int first, int last) 
{
    int ret;
    if  (0 <= pri && pri <= CHAR_MAX &&
    	0 <= first && first < sizeof(tTdvect)/sizeof(tTdvect[0]) &&
    	0 <= last && last < sizeof(tTdvect)/sizeof(tTdvect[0])) {
	while (first <= last)
	    tTdvect[first++] = (char) pri;
	ret = 0;
    } else {
	ret = -1;
    }
    return (ret);
}
#endif

void embeddedStopLogging (void)
{
#if 0
    /* 
     * Detach the shared memory segment and close the semaphores.
     */
    if (shmdt(mbp) < 0){
        perror("embeddedStopLogging: can't detach shared memory");
    }

    shmActive = 0;
#endif
}

void RmSharedResources (void)
{
#if 0
    union semun semarg;

    if (shmctl(shmid, IPC_RMID, (struct shmid_ds *) 0) < 0)
	perror("RmSharedResources: : can't remove shared memory");

    if (semctl(semid, /*sem number in set*/0, IPC_RMID, semarg) == -1)
	perror("RmSharedResources: : can't remove shared semaphore");

    shmActive = 0;
#endif
}



void PrintStdMIDMap (void)
{
#if 0
    fprintf (stderr, "mID\tdescription\n");
    fprintf (stderr, "%d\tfatal error logging\n", mID_FATAL);
    fprintf (stderr, "%d\twarnings logging\n", mID_WARN);
    fprintf (stderr,
             "%d\trcs version string logging\n",
             mID_RSCID);
    fprintf (stderr,
             "%d\tlog commands sent to BSM\n",
             mID_SEND_BSM_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to BSC\n",
             mID_SEND_BSC_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to MCI\n",
             mID_SEND_MCI_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to AVIO\n",
             mID_SEND_AVIO_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to RAID\n",
             mID_SEND_RAID_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from BSM\n",
             mID_RECV_BSM_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from BSC\n",
             mID_RECV_BSC_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from MCI\n",
             mID_RECV_MCI_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from AVIO\n",
             mID_RECV_AVIO_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from RAID\n",
             mID_RECV_RAID_MSGS);
    fprintf (stderr,
             "%d\tlog commands line args\n",
             mID_CMMDLN_ARGS);
    fprintf (stderr,
             "%d\tlog child process status\n",
             mID_CHLD_STATUS);
    fprintf (stderr,
             "%d\tlog commands sent to UI\n",
             mID_SEND_UI_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from UI\n",
             mID_RECV_UI_MSGS);
    fprintf (stderr,
             "%d\tlog reads to BSC status area\n",
             mID_RD_BSTAT_AREA);
    fprintf (stderr,
             "%d\tlog writes to BSC status area\n",
             mID_WR_BSTAT_AREA);
    fprintf (stderr,
             "%d\tlog begin/end of file desc. handler in DispatchMsgEvent\n",
             mID_DME_FD_HANDLER);
    fprintf (stderr,
             "%d\tlog begin/end of signal handler in DispatchMsgEvent\n",
             mID_DME_SIG_HANDLE);
    fprintf (stderr,
             "%d\tlog SMPTE errors\n",
             mID_SMPTE);
    fprintf (stderr,
             "%d\tlog parity errors\n",
             mID_PARITY_ERR);
    fprintf (stderr,
             "%d\tlog commands sent to MS\n",
             mID_SEND_MS_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from MS\n",
             mID_RECV_MS_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to TT\n",
             mID_SEND_TT_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from TT\n",
             mID_RECV_TT_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to ERAM\n",
             mID_SEND_ERAM_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from ERAM\n",
             mID_RECV_ERAM_MSGS);
    fprintf (stderr,
             "%d\tlog commands sent to DMA\n",
             mID_SEND_DMA_MSGS);
    fprintf (stderr,
             "%d\tlog commands received from DMA\n",
             mID_RECV_DMA_MSGS);
    fprintf (stderr,
             "%d\tlog actions in the buffer allocator\n",
             mID_BUFALLOCATOR);
#endif
}


/* 
 * lockResource ensures single process access to our shared memory
 * log buffer.  A straight forward implementation would use a single
 * semaphore to marshall this resource.  Unfortunately, semaphores
 * require a system call/context switch into the kernel even when
 * the semaphore count is positive.  For our application, this
 * overhead can drastically impact the performance of the system
 * with only a moderate amount of logging enabled.
 *
 * The implementation used below optimizes for the common case of
 * no contention for the log buffer.  In this scenario, no system
 * calls are performed, and the lock is acquired with only a few
 * instructions of overhead.  When contention does occur, the algorithm
 * requires several more system calls than the straight forward
 * semaphore approach.
 *
 * In a piece of shared memory shared amongst all processes is an int
 * of lock state.  This lock state can have the value of 'available',
 * 'busy', or 'busy with others waiting'.  Lock state transitions are
 * performed using an atomic 'test and set' facility, tset.  We also
 * allocate a semphore initialized to '1' (one resource available) that
 * is used to queue up any waiters for the lock.
 *
 * Successfull acquisition of the lock occurs when the lock state
 * transitions from 'available' to 'busy' status.  In the case of
 * contention, the semaphore resource is decremented to 0 by the
 * first waiter and all waiters block on the semaphore.  When the
 * process that currently has the lock releases it, a check is made
 * to see if there are any waiters.  If so, the semaphore is signaled
 * in addition to changing the lock state back to 'avaiable'.  One
 * process at a time will wake up to receive the semaphore resource,
 * release that resource (it was only used to suspend the process
 * momentarily) and attempt to acquire the lock in the normal fashion.
 * It is important to note that this use of the semaphore causes a
 * 'trampling heard' of waiters to wakeup and attempt to fetch the
 * lock again, but due to our inability to block context switches
 * between operations, there is no other choice.
 */
#if 0
int
lockResource(int *cell, int *busycnt) 
{
	for(;;)
	{
		int retval;

		retval = tset(cell, /*want*/MUTEX_BUSY, /*currently*/MUTEX_AVAIL);
		if (retval == MUTEX_AVAIL)
			break;
		if (retval == MUTEX_BUSY) {
			/*
			 * Decrement the semaphore and verify
			 * that we are the only client that 
			 * has done so.
			 */
			while ((retval = semop(semid, &sem_acquire_nowait, 1)) != 0
					&& errno == EINTR)
				;
			if (retval != 0) {
				if (errno == EAGAIN) {
					/*
					 * Another client beat us to it,
					 * so just go ahead and wait
					 * on the semaphore.
					 *
					 * * RACE condition *
					 * The semaphore may have already been released
					 * due to a context switch between the previous
					 * semop and the one we're about to do.  If so,
					 * we'll immediately recieve the semaphore,
					 * release it and try the lock again.
					 */
					while ((retval = semop(semid, &sem_acquire, 1)) != 0
						&& errno == EINTR)
							;

					if (retval != 0)
						return (errno);

					/*
					 * When we wakeup, release the dummy resource
					 * and try again to acquire the lock.
					 */
					while ((retval = semop(semid, &sem_release, 1)) != 0
						&& errno == EINTR)
							;

					if (retval != 0)
						abort();

					continue;
				} else {
					return (errno);
				}
			}

			/*
			 * Tell the current holder to inc the
			 * semaphore.
			 */
			retval = tset(cell, /*want*/MUTEX_BUSY|MUTEX_WAITING,
						  /*currently*/MUTEX_BUSY);

			if (retval != MUTEX_BUSY)
			{
				/*
				 * The holder of the lock released it before we
				 * could indicate our intention of waiting for it.
				 * Release the semaphore and try to acquire the
				 * the lock again.
				 */
				while ((retval = semop(semid, &sem_release, 1)) != 0
					&& errno == EINTR)
						;
				if (retval != 0)
					abort();
				continue;
    }

			/*
			 * Block waiting for the semaphore to be signaled by
			 * the current lock holder.
			 */
			while ((retval = semop(semid, &sem_acquire, 1)) != 0
				&& errno == EINTR)
					;
			if (retval != 0)
				return (errno);

			/*
			 * When we wakeup, release the semaphore
			 * and try again to acquire the lock.
			 */
			while ((retval = semop(semid, &sem_release, 1)) != 0
				&& errno == EINTR)
					;
			if (retval != 0)
				abort();
		}
		else
		{
			/*
			 * Someone else set the waiting flag,
			 * so get in line.
			 */
			while ((retval = semop(semid, &sem_acquire, 1)) != 0
				&& errno == EINTR)
					;
			if (retval != 0)
				return (errno);

			/*
			 * When we wakeup, release the dummy resource
			 * and try again to acquire the lock.
			 */
			while ((retval = semop(semid, &sem_release, 1)) != 0
				&& errno == EINTR)
					;
			if (retval != 0)
				abort();
		}
	}
	return (0);
}

void
freeResource(int *cell, int *busycnt) 
{
	int retval;

	retval = tset(cell, /*want*/MUTEX_AVAIL, /*currently*/MUTEX_BUSY);
	if (retval == (MUTEX_BUSY|MUTEX_WAITING)) {
		/*
		 * There are processes waiting on the semaphore.
		 * Release the lock and signal for them to attempt
		 * to acquire it again.
		 */
		retval = tset(cell, /*want*/MUTEX_AVAIL,
					  /*currently*/MUTEX_BUSY|MUTEX_WAITING);

		if (retval != (MUTEX_BUSY|MUTEX_WAITING))
			abort();

		/*
		 * * Race Condition *
		 * Another process that was not previously waiting may
		 * sneak in here and acquire the lock.  Since the waiters
		 * all wake up and release the semaphore back to a state of
		 * '1', they will simply reset the MUTEX_WAITING flag and
		 * queue up again.
		 */
		while ((retval = semop(semid, &sem_release, 1)) != 0
			&& errno == EINTR)
				;
		if (retval != 0)
			abort();
	}
}
#endif
