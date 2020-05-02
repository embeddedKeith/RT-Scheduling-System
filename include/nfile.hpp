//  Filename: NFile.hpp 
//                                       
//  This file contains the declaration(s) of the class(es):   
//     NFile - Portable File class.                            
//       
//  Copyright 2020 embeddedKeith                              
// -*^*-
//-------------------------------------------------------------------

#ifndef NFile_H
#define NFile_H

#include "osinc.hpp"
#include "nreslock.hpp"
#include "nstring.hpp"
#include "ntime.hpp"

class _MSExport NFile
{	
	
	public:

		NFile(const char* FileName, Boolean bWrite = false, Boolean bSupersede = false);
		virtual ~NFile();
        NString GetPathname( void ) const;
		Boolean checkPoint();
		Boolean Read(void* pvBuffer, INT iBufLen, INT &iLenRead);
		Boolean Write(const void* pvBuffer, INT iLen, INT &iLenWritten);
		Boolean Copy(PSZ pszName);
		INT64 Position(INT64 qwPos);
		INT64 SetToEnd();
		INT64 GetPosition();
		FILETIME getWriteTime();

		static long GetSize( const NString& pathname );  // size in bytes
		static bool Exists( const NString& pathname );   // Determines whether file exists
		static bool IsReadOnly ( const NString& pathname );


		static SYSTEMTIME GetDateTimeCreated( const NString& pathname );
		static SYSTEMTIME GetDateTimeCreated( fd_t file );
		static SYSTEMTIME GetDateTimeModified( const NString& pathname );
		static SYSTEMTIME GetDateTimeModified( fd_t file );

        // static long CompareTimes ( const SYSTEMTIME &time_1, const SYSTEMTIME &time_2 );

	private:
		fd_t hFile;
		NString pszFileName;
		Boolean bWriteEnable;
		NPrivateResource fileLock;
		
		NFile(const NFile &);
		NFile & operator= (const NFile &);
		Boolean checkPointNoLock();
		Boolean ReadNoLock(void* pvBuffer, INT iBufLen, INT &iLenRead);
		Boolean WriteNoLock(const void* pvBuffer, INT iLen, INT &iLenWritten);
		INT64 PositionNoLock(INT64 qwPos);
		INT64 SetToEndNoLock();
		INT64 GetPositionNoLock();
};

#endif

