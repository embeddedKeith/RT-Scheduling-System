// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nascfile.hpp"

NAsciiFile::NAsciiFile(const char* FileName, Boolean bWrite, Boolean bSupersede) :
    NFile(FileName, bWrite, bSupersede),
    cpBuf(new CHAR [NAsciiFileBUFFER]),
    ulBufPnt(0),
    ulBufUsed(0),
    ulBufSize(NAsciiFileBUFFER),
    uhReadPnt(0)
{
    uhFileSize = SetToEnd();
    return;
}

NAsciiFile::~NAsciiFile()
{
    delete [] cpBuf;

#ifdef __DEBUG_ALLOC_
    _heap_check();
#endif

    return;
}

ULONG NAsciiFile::getLine(PCHAR cpLine, ULONG ulLineLen)
{
    ULONG ulLinePnt = 0;
    CHAR ch;

    memset(cpLine, 0, ulLineLen);

    while(ulLinePnt < ulLineLen)
    {
        if(ulBufPnt >= ulBufUsed)
            if(fillBuffer() == false)
                return(ulLinePnt);

        ch = *(cpBuf + ulBufPnt++);
        if(ch == '\r' || ch == '\n')
        {
            if(ulLinePnt != 0)
                return(ulLinePnt);
        } else {
            *(cpLine + ulLinePnt++) = ch;
        }
    }

    return(ulLinePnt);
}

Boolean NAsciiFile::fillBuffer()
{
    INT iLenRead;

    Position(uhReadPnt);
    Read(cpBuf, ulBufSize, iLenRead);

    ulBufUsed = iLenRead;
    ulBufPnt = 0;
    uhReadPnt += iLenRead;
    if(iLenRead == 0)
        return(false);
    return(true);
}

Boolean NAsciiFile::writeLine(const char* cpLine)
{
    INT iLen, iLenWritten, iLenW;

    if(cpLine != NULL && (iLen = strlen(cpLine)) != 0)
    {
        SetToEnd();
        Write((const void*)cpLine, iLen, iLenWritten);
        Write("\r\n", 2, iLenW);
        if(iLen == iLenWritten) return(true);
    }
    return(false);
}
