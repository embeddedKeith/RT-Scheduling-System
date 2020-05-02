//
// Copyright 2020 embeddedKeith
// -*^*-
//-------------------------------------------------------------------------
#ifndef NAsciiFile_H
#define NAsciiFile_H

#include "nfile.hpp"

#define NAsciiFileBUFFER 4096

class _MSExport NAsciiFile : public NFile
{
protected:
    PCHAR cpBuf;
    ULONG ulBufPnt;
    ULONG ulBufUsed;
    ULONG ulBufSize;
    UINT64 uhReadPnt;
    UINT64 uhFileSize;

    Boolean fillBuffer();

public:
    NAsciiFile(const char* FileName,
               Boolean bWrite = false,
               Boolean bSupersede = false);
    virtual ~NAsciiFile();

    ULONG getLine(PCHAR cpLine, ULONG ulLineLen);
    Boolean writeLine(const char* pzLine);
};

#endif

