// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nerrlog.hpp"
#include "nascfile.hpp"

Boolean _Export NErrorLog::bInit = false;
NErrorLog* NErrorLog::clpThis = 0;

//----------------------------------------------------------------------------
//  This function is used to create an instance of the error log with a 
//    specified prefix and extension.  The prefix and extension can be
//    used to determine the location and name of the file.

_Export NErrorLog::NErrorLog( const NString& strHdr, const NString& strExt) :
    strFileHdr(strHdr), strFileExt(strExt)
{
    if(bInit == false)
    {
        clpThis = this;
        bInit = true;
    }

    Evt.reset();
    // pcs = AfxBeginThread((AFX_THREADPROC)NErrorLog::startProcessLoop, this);
    return;
}

_Export NErrorLog::~NErrorLog()
{
    ErrorLogData em(ErrorLogData::CloseOp, 0, NULL);
    if(clpThis == this)
    {
        bInit = false;
        clpThis = NULL;
    }

    storeMsg(em);
    threadDoneEvent.wait();
//  IThread::current().waitFor(pcsThread);

    return;
}

void _Export NErrorLog::storeMsg(ErrorLogData &em)
{
    NResourceLock lock(QueueLock);
    ErrorQueue.addAsLast(em);
    Evt.signal();

    return;
}

Boolean _Export NErrorLog::getMsg(ErrorLogData &em)
{
    NResourceLock lock(QueueLock);
    Boolean bRet = false;
    ErrorQueue.dequeue(em);
    bRet = true;

    return(bRet);
}

void _Export NErrorLog::log(DWORD Error, const char* pszCharData)
{
    ErrorLogData em(ErrorLogData::LogOp, Error, pszCharData);
    storeMsg(em);
    return;
}

void _Export NErrorLog::checkPoint()
{
    ErrorLogData em(ErrorLogData::ChkPntOp, 0, NULL);
    storeMsg(em);
    return;
}

void _Export NErrorLog::dump(NString strDumpFile)
{
    ErrorLogData em(ErrorLogData::DumpOp, 0, strDumpFile);
    storeMsg(em);
    return;
}

void _Export NErrorLog::startProcessLoop(NErrorLog *clpPtr)
{
    clpPtr->threadDoneEvent.reset();
    clpPtr->processLoop();
    clpPtr->threadDoneEvent.signal();
}

//-------------------------------------------------------------------------
//  This is the main thread loop of the error log.  It creates a file with the 
//    expected name and continues to add messages to it.  It waits for a shutdown
//    message at termination

void _Export NErrorLog::processLoop()
{
    NTime tCur;
    NString strCurDate = "";
    Evt.reset();
    NAsciiFile *clpFile = NULL;
    ErrorLogData em(ErrorLogData::LogOp, 0, NULL);
    Boolean bFileDirty = false;
    Boolean bContinue = true;
    NString strLog;

        while(bContinue == true)
        {
            Evt.wait(5000);
            Evt.reset();
                tCur.getCurrentTime();
                if(strCurDate != tCur.dateAsString())
                {
                    if(clpFile != NULL) delete clpFile;
                    //TODO clpFile = new NAsciiFile(LPCTSTR(GetFileName()), true, false);
                    strCurDate = tCur.dateAsString();
                }

                while(getMsg(em) == true)
                {
                    switch(em.Operation)
                    {
                        case ErrorLogData::LogOp:
                            strLog = em.clTime.dateAsString() + NString(" ") +
                                     em.clTime.timeAsString() + NString(" ");
                            if(em.dwError == 0)
                            {
                                strLog += "Message - ";
                            } else {
                                strLog += "Error ";
                                strLog += NString(em.dwError);
                                strLog += " - ";
                            }
                            if(em.cpCharData != NULL)
                                strLog += NString(em.cpCharData);
                            clpFile->writeLine(strLog);
                            bFileDirty = true;

                            break;

                        case ErrorLogData::ChkPntOp:
                            clpFile->checkPoint();
                            bFileDirty = false;

                            break;

                        case ErrorLogData::DumpOp:
                            clpFile->checkPoint();
                            bFileDirty = false;
                            if(em.cpCharData != NULL)
                                clpFile->Copy(em.cpCharData);

                            break;

                        case ErrorLogData::CloseOp:
                            bContinue = false;
                            break;

                        default:
                            break;
                    }
                }
                if(bFileDirty == true)
                {
                    clpFile->checkPoint();
                    bFileDirty = false;
                }
        }
        if(clpFile != NULL)
        {
             delete clpFile;
             clpFile = NULL;
        }
    return;
}


//-----------------------------------------------------------------------------
//  GetFileName:  This function creates a filename for the error log in the 
//    form:   XXXXX. YY.MM.DD, where XXXX is the name of the application that
//    is currently running.

NString _Export NErrorLog::GetFileName()
{
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;
    CHAR caBuf[12];
    NTime clCurTime;
    clCurTime.getCurrentTime().getTime(iHour, iMinute, iSecond, iMSec,
                                     iYear, iMonth, iDay);

    sprintf(caBuf, "%2.2d%2.2d%2.2d.",
            (iYear % 100), iMonth, iDay);
    NString strFile = strFileHdr + NString(caBuf) + strFileExt;
    return(strFile);
}

Boolean _Export NErrorLog::isInit()
{
    return(bInit);
}

NErrorLog& _Export NErrorLog::getClass()
{
    return(*clpThis);
}


_Export NErrorLog::ErrorLogData::ErrorLogData
(
	ErrOp Op, 
	DWORD dwErr, 
	const char* cpData
) : Operation(Op),
    dwError(dwErr),
	cpCharData(NULL)
{
    INT iLen;

    clTime.getCurrentTime();
    if(!cpData || !(iLen = strlen(cpData)))
       return;

    cpCharData = new CHAR[iLen + 1];
    strcpy(cpCharData, cpData);
    return;
}

_Export NErrorLog::ErrorLogData::ErrorLogData(const ErrorLogData &em) :
		Operation(em.Operation), 
		clTime(em.clTime),
		dwError(em.dwError), 
		cpCharData(NULL)
{
    INT iLen = 0;

    if(em.cpCharData && (iLen = strlen(em.cpCharData)))
    {
        cpCharData = new CHAR[iLen + 1];
        strcpy(cpCharData, em.cpCharData);
    }
    return;
}


NErrorLog::ErrorLogData & _Export NErrorLog::ErrorLogData::operator= (const ErrorLogData &em)
{
    INT iLen;

    if(cpCharData)
    {
        delete [] cpCharData;
        cpCharData = NULL;
    }
    Operation = em.Operation;
    dwError = em.dwError;
    clTime = em.clTime;
    if(em.cpCharData && (iLen = strlen(em.cpCharData)))
    {
        cpCharData = new CHAR[iLen + 1];

        strcpy(cpCharData, em.cpCharData);
    }
    return(*this);
}

_Export NErrorLog::ErrorLogData::~ErrorLogData()
{
    if(cpCharData)
        delete [] cpCharData;

    return;
}
