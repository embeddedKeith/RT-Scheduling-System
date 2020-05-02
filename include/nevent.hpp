#ifndef _NEvent_
#define _NEvent_
/***************************************************************
* FILE NAME: nevent.hpp                                        *
*                                                              *
* DESCRIPTION:                                                 *
*   This file contains the declaration(s) of the class(es):    *
*     NEvent - QNX event semaphore class.                      *
* Copyright (c) 2020 - embeddedKeith*
* Warning:  This file contains Propritary data                 *
*           and Trade Secrets and may not be used without      *
*           permission.
***************************************************************/

#include "osinc.hpp"
#include "nstring.hpp"
#include <semaphore.h>

/*-------------------------- NEvent ----------------------------
| Objects of this class represent "NEvents" that application   |
| threads can post and wait on.  They correspond to operating  |
| system event semaphores.                                     |
|                                                              |
| NEvents can be in either of two states:                      |
|   signalled - The signal has been set and subsequent wait    |
|               requests will not block.                       |
|   reset     - The signal has not been set; subsequent wait   |
|               requests will block until the signal is posted.|
|                                                              |
| NEvents have 2 other attributes that are fixed at the time   |
| they are constructed:                                        |
|   name          - Arbitrary identifier for the signal; named |
|                   signals can be shared between processes.   |
|   initial state - The signal can initially be signalled or   |
|                   reset; the default is reset.               |
--------------------------------------------------------------*/

class NEvent
{
public:

    _QnxExport NEvent (Boolean signaled = false, const char *name = NULL );
    _QnxExport NEvent (const char *name, Boolean signaled = false );
    _QnxExport virtual ~NEvent ( );

    _QnxExport virtual NEvent& reset();
    _QnxExport virtual NEvent& signal();
    _QnxExport virtual NEvent& wait(ULONG ultimeout = INFINITE);

protected:
    void initalize(const char*, Boolean);
    sem_t* sem;
    sem_t fastSem;
    Boolean bSemGood;
    NString strName;

private:
    NEvent(const NEvent& );
    NEvent& operator= (const NEvent& );
};

#endif
