/***************************************************************
* FILE NAME: nevent.cpp                                        *
*                                                              *
* DESCRIPTION:                                                 *
*   This file contains the implementation of the class):       *
*     NEvent - QNX event semaphore class.                      *
*                                                              *
* Copyright (c) 2020 - embeddedKeith*
* Warning:  This file contains Propritary data                 *
*           and Trade Secrets and may not be used without      *
*           permission.                                        *
***************************************************************/

#include "nevent.hpp"
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

_Export NEvent::NEvent(Boolean signaled = false, const char *name) :
    bSemGood(false)
{
    initalize(name, signaled);
    return;
}

_Export NEvent::NEvent(const char *name, Boolean signaled) :
    bSemGood(false)
{
    initalize(name, signaled);
    return;
}

_Export NEvent::~NEvent()
{
    if(bSemGood)
    {
        if(strName.length() == 0)
            sem_destroy(sem);
        else
            sem_close(sem);
        bSemGood = false;
        strName = "";
    }
    return;
}

void _Export NEvent::initalize(const char* name, Boolean signaled)
{
    int iRet;
    if(name != NULL)
        strName = name;
      
    if(strName.strip().length() == 0)
    {
        strName = "";
        sem = &fastSem;
        iRet = sem_init(sem, 0, (signaled) ? 1 : 0);
        if(iRet != 0) return;
        bSemGood = true;
		return;
    }

    if(strName[1] != '/')
    {
        NString strTmp('/');
        strTmp += strName;
        strName = strTmp;
    }

    sem = sem_open(strName, O_CREAT, S_IRWXG, (signaled) ? 1 : 0);
    if((INT)sem == -1) return;
    bSemGood = true;
    return;
}

NEvent& _Export NEvent::reset()
{
    return(*this);
}

NEvent& _Export NEvent::signal()
{
    if(bSemGood == true)
        sem_post(sem);
    return(*this);
}

NEvent& _Export NEvent::wait(unsigned long timout)
{
    if(bSemGood == false) return(*this);

    if(timout == 0 || timout == INFINITE)
    {
        sem_wait(sem);
        return(*this);
    }
    timespec stCurTime;
    clock_gettime(CLOCK_REALTIME, &stCurTime);
    time_t sec = timout / 1000;
    long nsec = (timout % 1000) * 1000000;
    stCurTime.tv_sec += sec;
    stCurTime.tv_nsec += nsec;
	// fprintf(stderr,"==================================%ld, %ld\n",(long)(stCurTime.tv_nsec/1000),timout);
    sem_timedwait(sem, &stCurTime);
	// fprintf(stderr,"++++++++++++++++++++++++++++++++++\n");
    return(*this);
}

    

