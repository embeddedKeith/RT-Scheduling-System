/***************************************************************
* FILE NAME: NLogFile.hpp                                      *
*                                                              *
* DESCRIPTION:                                                 *
*   This file contains the declaration(s) of the class(es):    *
*     NLogFile - Logging File class.                           *
*                                                              *
***************************************************************///
//  Copyright 2020 embeddedKeith
// -*^*-
//---------------------------------------------------------------------
#ifndef __NLogFile__
#define __NLogFile__



#include "osinc.hpp"
#include "nreslock.hpp"
#include "ntime.hpp"
#include "nevent.hpp"
#include "embeddedSeqList.hpp"
#include "nfile.hpp"

class _MSExport NLogFile
{

public:
//     #pragma pack(1)
    struct LogHeader
    {
        UCHAR ucaSignature[4];
        UINT  uiLength;
        NTime clTime;
        UINT  uiSend;
        UINT  uiUnit;
    };
    // #pragma pack(pop)

private:
    class LogData
	{
    public:
        enum LogitOp {LogDatOp, ChkPntitOp, DumpitOp, CloseitOp};
        LogitOp  Operation;
        UINT   uiLength;
        PUCHAR ucpData;

        LogData(LogitOp Op, Boolean bSent, UINT Unit, UINT uiLen,
                const unsigned char* ucpDat);
        LogData(const LogData &em);
        LogData &operator= (const LogData & );
        virtual ~LogData();
	};

    typedef embeddedSeqList <LogData> LogQueue;


public:
    NLogFile(const char* pszFile = "TMPLOG.DAT", UINT uiSize = (5120 * 1024));
    virtual ~NLogFile();
    void logSent(UINT Unit, UINT Len, const unsigned char* ucpDat);
    void logRcv(UINT Unit, UINT Len, const unsigned char* ucpDat);
    void checkPoint();
    void dump(const char* pszDumpFile);

private:
    LogQueue ErrorQueue;
    NPrivateResource QueueLock;
    NEvent Evt, threadDoneEvent;
    PCHAR pszFileName;
    pthread_t *pcs;
    UINT uiFileSize;

    static void startProcessLoop(NLogFile* clpPnt);
    void processLoop();
    void storeMsg(LogData &em);
    Boolean getMsg(LogData &em);
};
typedef NLogFile* PNLogFile;

#endif
