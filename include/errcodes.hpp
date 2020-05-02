#ifndef _ERRCODES_H_
#define _ERRCODES_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	errcodes
//
//	Description:	
//
//	FileName:	errcodes.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/24/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#ifndef _SYS_ERRNO_H_
#include <errno.h>
#endif

#define NE_FIRST_CODE NE_NOERR

#define SIG2PERR(sig) ((sig) + NE_SIGHUP - 1)

#define ELAST 56


/* leave plenty of room below the start of our codes for the system codes */
enum NE_ERRORS
{
    NE_NOERR = ELAST + 1,	/* no error */
    NE_BADARG,			/* argument has a bad value */
    NE_ARGMISSING,		/* expecting arg is missing */
    NE_NOMEM,			/* unable to allocate memory */
    NE_OPFAILED,		/* failure without specific error status */
    NE_DATATOOBIG,		/* more data than expected */
    NE_BUFFTOOSMALL,		/* given buffer is too small */
    NE_BADINDEX,		/* array index out of bounds */
    NE_DATAMISSING,		/* expected missing */
    NE_ARGUNEXP,		/* unexpected argument */
    NE_ABORTING,		/* process is shutting down */
    NE_EOF,                     /* got EOF */
    NE_CHECKSUM,                /* checksum error */
    NE_IOBREAK,                 /* break condition in I/O stream */
    NE_NOTIMPLEMENTED,          /* function is not implemented yet */
    NE_BADPARM, 				/* invalid parameter */
    NE_BADCONFIG,               /* invalid configuration */
    NE_CMDOVERRUN,              /* Command overrun, queue exceeded */
    NE_REBOOT_FAILED,           /* watch dog expired without reboot */
    NE_PWR_FAIL,                /* power failure detected */
    NE_SIGHUP,                  /* exited on SIGHUP */
    NE_SIGINT,                  /* exited on SIGINT */
    NE_SIGQUIT,                 /* exited on SIGQUIT */
    NE_SIGILL,                  /* exited on SIGILL */
    NE_SIGTRAP,                 /* exited on SIGTRAP */
    NE_SIGABRT,                 /* exited on SIGABRT */
    NE_SIGEMT,                  /* exited on SIGEMT */
    NE_SIGFPE,                  /* exited on SIGFPE */
    NE_SIGKILL,                 /* exited on SIGKILL */
    NE_SIGBUS,                  /* exited on SIGBUS */
    NE_SIGSEGV,                 /* exited on SIGSEGV */
    NE_SIGSYS,                  /* exited on SIGSYS */
    NE_SIGPIPE,                 /* exited on SIGPIPE */
    NE_SIGALRM,                 /* exited on SIGALRM */
    NE_SIGTERM,                 /* exited on SIGTERM */
    NE_SIGURG,                  /* exited on SIGURG */
    NE_SIGSTOP,                 /* exited on SIGSTOP */
    NE_SIGTSTP,                 /* exited on SIGTSTP */
    NE_SIGCONT,                 /* exited on SIGCONT */
    NE_SIGCHLD,                 /* exited on SIGCHLD */
    NE_SIGTTIN,                 /* exited on SIGTTIN */
    NE_SIGTTOU,                 /* exited on SIGTTOU */
    NE_SIGIO,                   /* exited on SIGIO */
    NE_SIGXCPU,                 /* exited on SIGXCPU */
    NE_SIGXFSZ,                 /* exited on SIGXFSZ */
    NE_SIGVTALRM,               /* exited on SIGVTALRM */
    NE_SIGPROF,                 /* exited on SIGPROF */
    NE_SIGWINCH,                /* exited on SIGWINCH */
    NE_SIGINFO,                 /* exited on SIGINFO */
    NE_SIGUSR1,                 /* exited on SIGUSR1 */
    NE_SIGUSR2,                 /* exited on SIGUSR2 */
    NE_INIT_TIMEOUT,            /* child timed out on init */
    NE_INIT_FAILED,             /* init failed */
    NE_SHUTD_TIMEOUT,           /* child shutdown timed out */
    NE_MACH_INOP,               /* machine is inoperable */
    NE_HDWR_MISSING,            /* required hardware is missing */
    NE_RD_MSG_TO,               /* read message timed out */
    NE_SFTW_INCONSITANT,        /* software environment is inconsistent */
    NE_HDWR_INCONSITANT,        /* hardware environment is inconsistent */
	NE_CONN_ERR,				/* problem with socket connection */
	NE_MALLOC_FAILED,			/* a call to malloc returned NULL */
    NE_INVALID_ERRCODE,         /* invalid error code */

    /* insert new codes before this line */
    NE_LAST_CODE,
};

#endif // _ERRCODES_H_
