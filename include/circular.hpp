#ifndef _CIRCULAR_H_
#define _CIRCULAR_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	circular
//
//	Description:	
//
//	FileName:	circular.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/24/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#define MSG_BSIZE       4096*8 
#define MSG_DATA_LOST	1		/* in case of buffer overflow */

typedef struct  msgbuf {
#define MSG_MAGIC       0x063061
    int             rtc_lock;
    int             rtc_busycnt;
    int             msg_lock;   /* implement mutex with intel CMPXCHG */
    unsigned int    msg_busycnt;/* number of contested locks */
    unsigned int    msg_magic;
    unsigned int    msg_size;
    unsigned int    msg_flags;
    unsigned int    msg_bufx;   /* write pointer */
    unsigned int    msg_bufr;   /* read pointer */
    char            msg_bufc[0]; /* buffer */
}MSGBUF;


/* XXX - This interface should be completely internal to clientLog.c,
 *	 so why bother exporting it in a .h files for other people
 *	 to see?
 */
#define	SHMKEY	((key_t) 7890) /* base value for shmem key */
#define	SEMKEY	((key_t) 7990) /* base value for semaphore key */
#define	PERMS	0666

extern int lockResource(int *cell, int *busycnt);
extern void freeResource(int *cell, int *busycnt);
#endif // _CIRCULAR_H_
