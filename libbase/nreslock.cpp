//----------------------------------------------------------------------
//  File:  NResLock.cpp
//  The NPrivateResource class inplements a mutex which is used to
//  synchronize access to critical code sections and data structures. 
//
//  The NSharedResouce allows access accross processes.
//
//  Then NResourceLock class is intended to be declared within a 
//  class and used to protect access to one or more member functions.
//
//  Copyright 2020 embeddedKeith
//  -*^*-
//---------------------------------------------------------------------

#include "nreslock.hpp"
#include <time.h>

NPrivateResource::NPrivateResource()
{
    bMutexGood = false;
    pthread_mutexattr_init(&att);
    pthread_mutexattr_setrecursive(&att, PTHREAD_RECURSIVE_ENABLE);

    int iRet = pthread_mutex_init(&mutex, &att);
    if(iRet == EOK)
        bMutexGood =true;
    return;
}

_Export NPrivateResource::~NPrivateResource()
{
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&att);
    bMutexGood = false;
    return;
}

Boolean _Export NPrivateResource::lock( unsigned long timeout_ms = INFINITE )
{
    if(bMutexGood == false) return(false);

    int iRet;
    if(timeout_ms == INFINITE)
    {
        iRet = pthread_mutex_lock(&mutex);
        if(iRet != EOK) return(false);
        return(true);
    }

    timespec stCurTime;
    clock_gettime(CLOCK_REALTIME, &stCurTime);
    time_t sec = timeout_ms / 1000;
    long nsec = (timeout_ms % 1000) * 1000000;
    stCurTime.tv_sec += sec;
    stCurTime.tv_nsec += nsec;
    iRet = pthread_mutex_timedlock(&mutex, &stCurTime);
    if(iRet != 0) return(false);
    return(true);
}

void _Export NPrivateResource::unlock(void)
{
    if(bMutexGood == false) return;
    pthread_mutex_unlock(&mutex);
    return;
}

_Export NResourceLock::NResourceLock(NPrivateResource& res, 
                                     unsigned long timeout_ms) :
    resource(&res)
{
    resource->lock(timeout_ms);
    return;
}

_Export NResourceLock::~NResourceLock()
{
    resource->unlock();
    return;
}

