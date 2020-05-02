//----------------------------------------------------------------------
//  File:  NResLock.hpp
//  The embeddedPrivateResource class inplements a mutex which is used to
//  synchronize access to critical code sections and data structures. 
//
//  The embeddedSharedResouce allows access accross processes.
//
//  Then embeddedResourceLock class is intended to be declared within a 
//  class and used to protect access to one or more member functions.
//
//  Copyright 2020 embeddedKeith
//  -*^*-
//---------------------------------------------------------------------

#ifndef __NResLock__
#define __NResLock__

#include "osinc.hpp"
#include "nstring.hpp"

#include <pthread.h>

class _MSExport NPrivateResource
{
//  friend std::ostream& operator << ( std::ostream& os, const NPrivateResource& rhs );

public:
    _QnxExport NPrivateResource();
    _QnxExport virtual ~NPrivateResource();
    _QnxExport virtual Boolean lock(unsigned long timeout_ms = INFINITE);
    _QnxExport virtual void unlock(void);

protected:
    friend class NResourceLock;

    pthread_mutex_t mutex;
    pthread_mutexattr_t att;
    Boolean bMutexGood;

private:
    NPrivateResource(const NPrivateResource& privateResource);
    NPrivateResource& operator= (const NPrivateResource& privateResource);

};


class _MSExport NSharedResource
{
    friend class NResourceLock;
public:
    _QnxExport NSharedResource( const NString& keyname );
    _QnxExport virtual ~NSharedResource();
    _QnxExport virtual NString GetKeyName() const;
    _QnxExport virtual bool Lock( unsigned long timeout_ms = INFINITE );
    _QnxExport virtual void Unlock( void );

private:
    NSharedResource(const NSharedResource& sharedResource);
    NSharedResource& operator= (const NSharedResource& sharedResource);

protected:
    NString keyname;
    pthread_mutex_t mutex;
    bool currently_locked;
	
};

class _MSExport NResourceLock
{
public:
    _QnxExport NResourceLock(NPrivateResource& resource, 
                    unsigned long timeout_ms = INFINITE);
    _QnxExport virtual ~NResourceLock();

private:
    NResourceLock(const NResourceLock& resourceLock);
    NResourceLock& operator= (const NResourceLock& resourceLock);

protected:
    NPrivateResource* resource;
};

#endif
