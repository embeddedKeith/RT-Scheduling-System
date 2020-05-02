#ifndef _ERRSTRINGS_H_
#define _ERRSTRINGS_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	errstrings
//
//	Description:	
//
//	FileName:	errstrings.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/24/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|


static char *errStrings [] =
{
    NULL,			/* NE_NOERR       */
    "arg has bad value",	/* NE_BADARG      */
    "missing arg",		/* NE_ARGMISSING  */
    "no memory",		/* NE_NOMEM       */
    "operation failed",		/* NE_OPFAILED    */
    "more data than expected",	/* NE_DATATOOBIG  */
    "supplied buffer is too small", /* NE_BUFFTOOSMALL */
    "array index out of bounds", /* NE_BADINDEX    */
    "expected data is missing",	/* NE_DATAMISSING */
    "unexpected argument",	/* NE_ARGUNEXP */
    "process is exiting",	/* NE_ABORTING */
    "EOF encountered",          /* NE_EOF */
    "checksum error",           /* NE_CHECKSUM */
    "break condition in stream", /* NE_IOBREAK */
    "function is not implemented yet", /* NE_NOTIMPLEMENTED */
    "invalid paramter",                /* NE_BADPARM */
    "invalid configuration",           /* NE_BADCONFIG */
    "Command overrun, queue exceeded", /* NE_CMDOVERRUN */
    "watch dog expired without reboot", /* NE_REBOOT_FAILED */
    "power failure detected",   /* NE_PWR_FAIL */
    "exited on SIGHUP",         /* NE_SIGHUP    */
    "exited on SIGINT",         /* NE_SIGINT    */
    "exited on SIGQUIT",        /* NE_SIGQUIT   */
    "exited on SIGILL",         /* NE_SIGILL    */
    "exited on SIGTRAP",        /* NE_SIGTRAP   */
    "exited on SIGABRT",        /* NE_SIGABRT   */
    "exited on SIGEMT",         /* NE_SIGEMT    */
    "exited on SIGFPE",         /* NE_SIGFPE    */
    "exited on SIGKILL",        /* NE_SIGKILL   */
    "exited on SIGBUS",         /* NE_SIGBUS    */
    "exited on SIGSEGV",        /* NE_SIGSEGV   */
    "exited on SIGSYS",         /* NE_SIGSYS    */
    "exited on SIGPIPE",        /* NE_SIGPIPE   */
    "exited on SIGALRM",        /* NE_SIGALRM   */
    "exited on SIGTERM",        /* NE_SIGTERM   */
    "exited on SIGURG",         /* NE_SIGURG    */
    "exited on SIGSTOP",        /* NE_SIGSTOP   */
    "exited on SIGTSTP",        /* NE_SIGTSTP   */
    "exited on SIGCONT",        /* NE_SIGCONT   */
    "exited on SIGCHLD",        /* NE_SIGCHLD   */
    "exited on SIGTTIN",        /* NE_SIGTTIN   */
    "exited on SIGTTOU",        /* NE_SIGTTOU   */
    "exited on SIGIO",          /* NE_SIGIO     */
    "exited on SIGXCPU",        /* NE_SIGXCPU   */
    "exited on SIGXFSZ",        /* NE_SIGXFSZ   */
    "exited on SIGVTALRM",      /* NE_SIGVTALRM */
    "exited on SIGPROF",        /* NE_SIGPROF   */
    "exited on SIGWINCH",       /* NE_SIGWINCH  */
    "exited on SIGINFO",        /* NE_SIGINFO   */
    "exited on SIGUSR1",        /* NE_SIGUSR1   */
    "exited on SIGUSR2",        /* NE_SIGUSR2   */
    "child timed out on init",  /* NE_INIT_TIMEOUT */
    "init failed",              /* NE_INIT_FAILED */
    "child shutdown timed out",  /* NE_SHUTD_TIMEOUT */
    "machine is inoperable",    /* NE_MACH_INOP */
    "required hardware is missing", /* NE_HDWR_MISSING */
    "read message timed out",   /* NE_RD_MSG_TO */
    "software environment is inconsistent", /* NE_SFTW_INCONSITANT */
    "hardware environment is inconsistent", /* NE_HDWR_INCONSITANT */
	"invalid error code",       /* NE_INVALID_ERRCODE */

    /*
    ** Terminate with NULL string so size of array can be
    ** checked with errcodes enum at initialization.
    */
    NULL                        /* table terminator */
};

#endif // _ERRSTRINGS_H_
