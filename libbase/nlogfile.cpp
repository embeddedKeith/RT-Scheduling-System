// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nlogfile.hpp"

_Export NLogFile::LogData::LogData(LogitOp Op, Boolean bSend, UINT Unit, UINT uiLen,
                                   const unsigned char* ucpDat) :
    Operation(Op), uiLength(sizeof(LogHeader) + (uiLen * 2)), ucpData(NULL)
{
    UINT i;
    PUCHAR ucpPnt;

    if(!ucpDat || !uiLen)
       return;

    ucpData = new UCHAR[uiLength];
    LogHeader *stpHdr = (LogHeader *)ucpData;
    memset(stpHdr->ucaSignature, '\016', sizeof(stpHdr->ucaSignature));
    stpHdr->uiLength = uiLength;
    stpHdr->clTime.getCurrentTime();
    stpHdr->uiSend = (bSend) ?  0xFFFFFFFF : 0;
    stpHdr->uiUnit = Unit;

    for(i = 0, ucpPnt = ucpData + sizeof(LogHeader); i < uiLen; i += 1, ucpPnt += 2)
    {
        *ucpPnt = *ucpDat++;
        *(ucpPnt + 1) = 0;
    }
    return;
}

_Export NLogFile::LogData::LogData(const LogData &em) :
  Operation(em.Operation), uiLength(em.uiLength), ucpData(NULL)
{
    if(em.ucpData && em.uiLength)
    {
        ucpData = new UCHAR[em.uiLength];
        memmove(ucpData, em.ucpData, em.uiLength);
    }
    return;
}


NLogFile::LogData & _Export NLogFile::LogData::operator= (const NLogFile::LogData &em)
{
    if(ucpData)
    {
        delete [] ucpData;
        ucpData = NULL;
    }
    Operation = em.Operation;
    uiLength = em.uiLength;
    if(em.ucpData && em.uiLength)
    {
        ucpData = new UCHAR[em.uiLength];
        memmove(ucpData, em.ucpData, uiLength);
    }
    return(*this);
}

_Export NLogFile::LogData::~LogData()
{
    if(ucpData)
        delete [] ucpData;

    return;
}


_Export NLogFile::NLogFile(const char* pszFile, UINT uiSize) :
    	pszFileName(NULL), 
		pcs(NULL),
		uiFileSize(uiSize)
{
	// TODO pszFileName = pszFile;
    Evt.reset();
    // TODO pcs = AfxBeginThread((AFX_THREADPROC)NLogFile::startProcessLoop, this);
    return;
}

_Export NLogFile::~NLogFile()
{
    LogData em(LogData::CloseitOp, false, 0, 0, NULL);
    storeMsg(em);
    Evt.signal();
    // TODO threadDoneEvent.wait();

    return;
}

void _Export NLogFile::storeMsg(LogData &em)
{
    NResourceLock lock(QueueLock);
    ErrorQueue.addAsLast(em);
    Evt.signal();

    return;
}

Boolean _Export NLogFile::getMsg(LogData &em)
{
    NResourceLock lock(QueueLock);
  //  TRY
 //   {
        return( ErrorQueue.dequeue(em) );
  //  }
   // CATCH(CUserException, e)
   // {
  //      return(false);
  //  }
   // END_CATCH

  //  return(true);
}

void _Export NLogFile::logSent(UINT Unit, UINT uiLen, const unsigned char* ucpDat)
{
    LogData em(LogData::LogDatOp, true, Unit, uiLen, ucpDat);
    storeMsg(em);
    Evt.signal();
    return;
}

void _Export NLogFile::logRcv(UINT Unit, UINT uiLen, const unsigned char* ucpDat)
{
    LogData em(LogData::LogDatOp, false, Unit, uiLen, ucpDat);
    storeMsg(em);
    Evt.signal();
    return;
}

void _Export NLogFile::checkPoint()
{
    LogData em(LogData::ChkPntitOp, false, 0, 0, NULL);
    storeMsg(em);
    Evt.signal();
    return;
}

void _Export NLogFile::dump(const char* pszDumpFile)
{
    LogData em(LogData::DumpitOp, false, 0, strlen(pszDumpFile), (PUCHAR)pszDumpFile);
    storeMsg(em);
    Evt.signal();
    return;
}

void _Export NLogFile::startProcessLoop(NLogFile* clpPnt)
{
    clpPnt->threadDoneEvent.reset();
    clpPnt->processLoop();
    clpPnt->threadDoneEvent.signal();
}

void _Export NLogFile::processLoop()
{
    LogData em(LogData::LogDatOp, false, 0, 0, NULL);
    Boolean bContinue = true;
    UINT uiBufLen = (50 * 1024);
    PUCHAR ucpBuf = new UCHAR[uiBufLen];
    UINT uiBufPos = 0;
    PUCHAR ucpBufPnt = ucpBuf;
    INT64 qwPos = 0;
    UINT uiLengthToWrite;
    INT iWriteLen, iMoveLen, iLenWritten;
    PUCHAR ucpGetPnt;


    NFile LogFile(pszFileName, true, true);

    while(bContinue)
    { 
		if(getMsg(em) == FALSE)
        { 
			Evt.wait(1000); 
			Evt.reset(); 
			continue; 
		}

       switch(em.Operation)
       {
       case LogData::LogDatOp:
            uiLengthToWrite = em.uiLength;
            ucpGetPnt = em.ucpData;
            while((uiBufPos + uiLengthToWrite) > uiBufLen)
            {
                iMoveLen = (uiBufLen - uiBufPos);
                memmove(ucpBufPnt, ucpGetPnt, iMoveLen);
                ucpGetPnt += iMoveLen;
                uiBufPos += iMoveLen;
                uiLengthToWrite -= iMoveLen;
                iWriteLen = uiBufPos;

                LogFile.Position(qwPos);
                LogFile.Write(ucpBuf, iWriteLen, iLenWritten);
                qwPos += iLenWritten;
                if(qwPos > uiFileSize)
                qwPos = 0;

                uiBufPos = 0;
                ucpBufPnt = ucpBuf;
            }

            if(uiLengthToWrite)
            {
                iMoveLen = uiLengthToWrite;
                memmove(ucpBufPnt, ucpGetPnt, iMoveLen);
                ucpBufPnt += iMoveLen;
                uiBufPos += iMoveLen;
            }
            break;

        case LogData::ChkPntitOp:
            iWriteLen = uiBufPos;

            LogFile.Position(qwPos);
            LogFile.Write(ucpBuf, iWriteLen, iLenWritten);
            LogFile.checkPoint();
 
            break;

        case LogData::DumpitOp:
            if((iWriteLen = uiBufPos) != 0)
            {
                LogFile.Position(qwPos);
                LogFile.Write(ucpBuf, iWriteLen, iLenWritten);
                LogFile.checkPoint();
                LogFile.Copy((PSZ)em.ucpData);
            }
 
            break;

        case LogData::CloseitOp:
            if((iWriteLen = uiBufPos) != 0)
            {
                LogFile.Position(qwPos);
                LogFile.Write(ucpBuf, iWriteLen, iLenWritten);
                LogFile.checkPoint();
            }
            bContinue = false;
            break;
        }
        continue;
    }
    if(ucpBuf != NULL) delete [] ucpBuf;
    ucpBuf = NULL;
    return;
}
