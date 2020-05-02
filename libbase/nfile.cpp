// NFile.cpp                                         
//   This file contains the implementation of                  
//   classes/functions declared in lfile.hpp.                  
//
// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nfile.hpp"
#include "nexception.hpp"
// #include "NResource.hpp"
#include <iostream>

const TCHAR PATH_DELIMITER = _T('\\');
const TCHAR ALT_PATH_DELIMITER = _T('/');
const TCHAR EXT_DELIMITER = _T('.');
const TCHAR DRIVE_DELIMITER = _T(':');
const TCHAR NULL_STRING[2] = { 0, 0 };


_MSExport NFile::NFile(const char* FileName, 
				Boolean bWrite, Boolean bSupersede) :
		hFile(), 
		pszFileName(),
		bWriteEnable(bWrite)
{
	INT iLen;

	if(!FileName || !(iLen = strlen(FileName)))
	{
		// TODO CFileException::ThrowOsError(0, FileName);
	    return;
	}

	pszFileName = FileName;
#if 0 // TODO
	DWORD dwAccess = (bWriteEnable) ? (GENERIC_READ | GENERIC_WRITE) :
	                                  (GENERIC_READ);
	DWORD dwOpen = (bSupersede) ? CREATE_ALWAYS : OPEN_ALWAYS;
	hFile = CreateFile(pszFileName, dwAccess, FILE_SHARE_READ, NULL,
	                   dwOpen,
	                   (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS),
	                   NULL);

	if(hFile == (INVALID_HANDLE_VALUE))
	{
	  
		NString reason;
		reason.Format( "Unable to open file: %s, reason:%s\n",
			pszFileName, NString::GetLastErrorText() );
		DWORD rc = GetLastError();
		CFileException::ThrowOsError(rc, FileName);

	    return;
	}
#endif
	return;
}


_MSExport NFile::~NFile()
{

	bWriteEnable = false;
#if 0 // TODO
	if(CloseHandle(hFile) == false)
	{
	    hFile = INVALID_HANDLE_VALUE;
	    DWORD rc = GetLastError();
	    CFileException::ThrowOsError(rc, pszFileName);
	    return;
	}
	hFile = INVALID_HANDLE_VALUE;

#endif
	return;
}

NString NFile::GetPathname( void ) const
{
	    return( pszFileName );
}
	
Boolean _MSExport NFile::checkPoint()
{
	NResourceLock lck(fileLock);                                            
	return(checkPointNoLock());
}
Boolean _MSExport NFile::checkPointNoLock()
{
	if(bWriteEnable == false)
	    return(false);


//	if(FlushFileBuffers(hFile) == false)
//	{
	 //   DWORD rc = GetLastError();
	 //   CFileException::ThrowOsError(rc, pszFileName);
	 //   return(false);
//	}
	return(true);
}

Boolean _MSExport NFile::Read(PVOID pvBuffer, INT iBufLen, INT &iLenRead)
{
	NResourceLock lck(fileLock);
	return(ReadNoLock(pvBuffer, iBufLen, iLenRead));
}
Boolean NFile::ReadNoLock(PVOID pvBuffer, INT iBufLen, INT &iLenRead)
{
	iLenRead = 0;


	DWORD iRead;

	// if(ReadFile(hFile, pvBuffer, iBufLen, &iRead, NULL) == false)
	// {
	     // DWORD rc = GetLastError();
	     // CFileException::ThrowOsError(rc, pszFileName);
	     // return(false);
	// }
	iLenRead = iRead;
	return(true);

}

	
Boolean _MSExport NFile::Write(const void* pvBuffer, INT iLen, INT &iLenWritten)
{
	NResourceLock lck(fileLock);
	return(WriteNoLock(pvBuffer, iLen, iLenWritten));
}
Boolean _MSExport NFile::WriteNoLock(const void* pvBuffer, INT iLen, INT &iLenWritten)
{
	iLenWritten = 0;

	// if(bWriteEnable == false)
	// {
	    // CFileException::ThrowOsError(0, pszFileName);
	    // return(false);
	// }

	DWORD iWrite;

	// if(WriteFile(hFile, pvBuffer, iLen, &iWrite, NULL) == false)
	// {
	     // DWORD rc = GetLastError();
	     // CFileException::ThrowOsError(rc, pszFileName);
	     // return(false);
	// }
	iLenWritten = iWrite;
	return(true);
}

INT64 _MSExport NFile::Position(INT64 qwPos)
{
	NResourceLock lck(fileLock);
	return(PositionNoLock(qwPos));
}

INT64 _MSExport NFile::PositionNoLock(INT64 qwPos)
{
	LONG iHighLen;
	ULONG uiLowLen, uiRetLowLen;
	INT64 qwSize;

	iHighLen = LONG (qwPos >> 32);
	uiLowLen = ULONG (qwPos & (0xffffffff));
	// uiRetLowLen = SetFilePointer(hFile, uiLowLen, &iHighLen, FILE_BEGIN);
	qwSize = iHighLen;
	qwSize <<= 32;
	qwSize += uiRetLowLen;
	return(qwSize);
}

INT64 _MSExport NFile::SetToEnd()
{
	NResourceLock lck(fileLock);
	return(SetToEndNoLock());
}
INT64 _MSExport NFile::SetToEndNoLock()
{
	LONG iHighLen = 0;
	ULONG uiRetLowLen;
	INT64 qwSize;

	// uiRetLowLen = SetFilePointer(hFile, 0, &iHighLen, FILE_END);
	qwSize = iHighLen;
	qwSize <<= 32;
	qwSize += uiRetLowLen;
	return(qwSize);
}

INT64 _MSExport NFile::GetPosition()
{
	NResourceLock lck(fileLock);
	return(GetPositionNoLock());
}
INT64 _MSExport NFile::GetPositionNoLock()
{
	LONG iHighLen = 0;
	ULONG uiRetLowLen;
	INT64 qwSize;

	// uiRetLowLen = SetFilePointer(hFile, 0, &iHighLen, FILE_CURRENT);
	qwSize = iHighLen;
	qwSize <<= 32;
	qwSize += uiRetLowLen;
	return(qwSize);
}

Boolean _MSExport NFile::Copy(PSZ pszName)
{
	NResourceLock lck(fileLock);

	INT64 hFileSize, hFilePnt, hFilePosition;
	INT iSizeRead;
	INT iSizeWrite;
	INT iSizeDone;
	UINT uiBufSize = (1024 * 1024);
	PUCHAR ucpBuf = new UCHAR[uiBufSize];

	NFile Dst(pszName, true, true);

	this->checkPointNoLock();
	hFilePosition = this->GetPositionNoLock();
	hFileSize = this->SetToEndNoLock();
	this->PositionNoLock(0);

	for(hFilePnt = 0; hFilePnt < hFileSize; hFilePnt += iSizeDone)
	{
	    if((hFileSize - hFilePnt) > uiBufSize)
		iSizeRead = uiBufSize;
	    else
		iSizeRead = int ((hFileSize - hFilePnt));

	    this->ReadNoLock(ucpBuf, iSizeRead, iSizeDone);
	    iSizeWrite = iSizeDone;
	    Dst.Write(ucpBuf, iSizeWrite, iSizeDone);
	}
	this->PositionNoLock(hFilePosition);
	delete [] ucpBuf;
	return(true);
}

FILETIME _MSExport NFile::getWriteTime()
{
    FILETIME ftWrite;

    // TODO GetFileTime(hFile, NULL, NULL, &ftWrite);
    return(ftWrite);
}


//-----------------------------------------------------------------
// Some static functions to determine whether a file exists and if it
//   is write protected (read-only)

bool NFile::Exists( const NString& pathname )
{
	// TODO unsigned long result = GetFileAttributes( pathname );
	// return ( result != -1 );
	return TRUE;
}

bool NFile::IsReadOnly ( const NString& pathname )
{
    // TODO unsigned long result = GetFileAttributes( pathname );
    // return (result & FILE_ATTRIBUTE_READONLY);
		return FALSE;
}


#if 0
//-----------------------------------------------------------------------
// returns the path to the system temporary directory.
//  1.	The path specified by the TMP environment variable. 
//	2.	The path specified by the TEMP environment variable, if TMP is not defined. 
//	3.	The current directory, if both TMP and TEMP are not defined.

NString NFile::GetTempDirectory ( void )
{
    TCHAR dir_name[_MAX_PATH];
    if (!GetTempPath (sizeof(dir_name), dir_name))
    {
		NString reason;
		reason.Format( "Unable to get pathname of temp directory, reason: %s",
			"FileUtil::getTempDirectory",  __LINE__, __FILE__  );
    }

	if ( !Exists( dir_name ) )
	{
		CreateDirectory( dir_name );
	}

    return (dir_name );
}

//-----------------------------------------------------------------------
//  The CreateUniqueFilename() function returns a filename from the designated
//  directory which is in the form prefixXXX.ext, Where XXX is a numerical
//  value which is incremented until a unique value is found.   It
//  creates the file and closes it to ensure that it exists.  If the 
//  directory does not exist, or the user does not have permission, it
//  throws an exception.

NString NFile::CreateUniqueFilename
( 
	const NString& directory,   //"" indicates use system temp
	const NString& prefix, 
	const NString& extension,
	long  required_uniquer_digits
)
{   

    // setup the directory.
    NString temp_directory = directory;
    if (temp_directory.GetLength() == 0 )  // user wants me to use the system TEMP
    {
        temp_directory = GetTempDirectory();
    }

	// If the directory does not end in \, add the backslash
	if ( temp_directory[ temp_directory.GetLength() - 1 ] != PATH_DELIMITER )
	{
		temp_directory += PATH_DELIMITER;
	}

	// If the directory does not exist, throw a logic exception
	if ( !Exists( temp_directory ) )
	{
		NString reason;
		reason.Format( "Directory %s does not exist", temp_directory );
		throw NLogicException( reason, "NFile::CreateUniqueFilename", __LINE__, __FILE__ );
	}

	
	// Create the pathname string which is used to format potential pathname and
	//   build the initial pathname.

	NString pathname_format;
	NString test_pathname;
    long counter = 1;   

	test_pathname.GetBuffer(_MAX_PATH);   /// Make buffer long enough for longest path
	if ( 0 == required_uniquer_digits )         // Leading 0 digits not required
	{
		test_pathname.Format( _T("%s%s.%s"), (LPCSTR)temp_directory, (LPCSTR)prefix,	(LPCSTR)extension  );
	}
    else  // a minimum number of digits required
	{
		pathname_format.Format( _T("%%s%%s%%0%dd.%%s"), required_uniquer_digits  );
		test_pathname.Format( (LPCSTR)pathname_format, (LPCSTR)temp_directory, (LPCSTR)prefix, 
				counter, (LPCSTR)extension );
	}

	while ( 1 )
	{
	   unsigned long result = GetFileAttributes( test_pathname );
	   if ( -1 == result && ERROR_FILE_NOT_FOUND == GetLastError() )
	   {	
			try
			{
				HANDLE handle = CreateFile( test_pathname, GENERIC_WRITE, 0, 0,
			  				 CREATE_NEW, FILE_ATTRIBUTE_NORMAL, 0 );
				if ( handle != INVALID_HANDLE_VALUE )
				{
					CloseHandle( handle );
					return( test_pathname );
				}
				NString reason;
				reason.Format( "Unable to create file: %s, reason:%s",
					test_pathname, NString::GetLastErrorText() );
				throw NRuntimeException( reason, "NFile::CreateUniqueFilename", __LINE__, __FILE__ );
			}
			catch (...)
			{
				NString reason;
				reason.Format( "Unable to create file: %s, reason:%s",
					test_pathname, NString::GetLastErrorText() );
				throw NRuntimeException( reason, "NFile::CreateUniqueFilename", __LINE__, __FILE__ );
			}
	   }
	   else
	   {
		    counter++;
			test_pathname.Format( pathname_format, (LPCSTR)temp_directory, (LPCSTR)prefix, 
				counter, (LPCSTR)extension );
	   }
	}
}

//----------------------------------------------------------------------
// MoveFile -- a move file that works on NT & 95/98

bool NFile::MoveFile
( 
	const NString &old_pathname,
    const NString &new_pathname,
    bool  replace_existing // = true
)
{
    BOOL result;

    if (   1 ) // BJR Registry::isWindowsNT() )
    {
        DWORD flags = MOVEFILE_COPY_ALLOWED;
        if (replace_existing)
            flags |=  MOVEFILE_REPLACE_EXISTING;

        result = MoveFileEx (old_pathname, new_pathname, flags);
    }
    else  // Win 95/98
    {
        BOOL fail_if_exists = (replace_existing ? FALSE : TRUE);

        result = CopyFile ( old_pathname, new_pathname, fail_if_exists );
        if (TRUE == result)
        {
            result = DeleteFile (old_pathname);
        }
    }
    return (result ? true : false);
}

//------------------------------------------------------------------
//  This function copies a file name into a new directory.  The filename is
//    modified (if a version already exists).

NString NFile::MoveFileUniquely 
( 
	const NString& old_pathname,   // (input) The full pathname of a file
    const NString& new_directory   // (input) Target directory
)
{
	NString prefix = GetRootnameFromPath( old_pathname );
	NString extension = GetExtensionFromPath( old_pathname );
	long  required_uniquer_digits = 0;

	if ( !Exists( new_directory ) )
	{
		CreateDirectory( new_directory );
	}
	
	NString new_pathname = CreateUniqueFilename( new_directory, 
		prefix, extension, required_uniquer_digits );

	bool replace_existing = true;
	if ( !MoveFile( old_pathname, new_pathname, replace_existing ) )
	{
		NString reason;
		reason.Format( "Unable to copy file %s to %S, reason: %s",
			old_pathname, new_pathname,NString::GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::MoveFileUniquely", __LINE__, __FILE__ );
	}
	return( new_pathname );
}

NString NFile::CopyFileUniquely 
( 
	const NString &old_pathname,
	const NString &new_directory
)
{
	NString prefix = GetRootnameFromPath( old_pathname );
	NString extension = GetExtensionFromPath( old_pathname );
	long  required_uniquer_digits = 0;

	if ( !Exists( new_directory ) )
	{
		CreateDirectory( new_directory );
	}
	
	NString new_pathname = CreateUniqueFilename( new_directory, 
		prefix, extension, required_uniquer_digits );

	bool replace_existing = true;
	CopyFile( old_pathname, new_pathname, replace_existing );
	return( new_pathname );
}

//--------------------------------------------------------------------
//  This function creates a new directory within the specified directory
//  which is guaranteed to be unique and new.  If the directory does not exist
//  it will throw a LogicException

NString NFile::CreateUniqueDirectory
( 
	const NString& directory,   // "" to place in System TEMP dir
	const NString& prefix,
	long  required_uniquer_digits
)
{
	NString temp_directory = directory;
	

    if (temp_directory.GetLength() == 0 )  // user wants me to use the system TEMP
    {
        temp_directory = NFile::GetTempDirectory();
    }

	// If the directory does not end in \, add the backslash

	if ( temp_directory[ temp_directory.GetLength() - 1 ] != PATH_DELIMITER )
	{
		temp_directory += PATH_DELIMITER;
	}

	if ( !Exists( temp_directory ) )
	{
		NString reason;
		reason.Format ( "Directory %s does not exist", temp_directory );
		throw NLogicException( reason, "NFile::CreateUniqueDirectory", __LINE__, __FILE__ );
	}

	NString pathname_format("%s%s%d");
	NString test_directory;     
	long counter = 1;   
	test_directory.GetBuffer( _MAX_PATH );
	if ( 0 == required_uniquer_digits )         // Leading 0 digits not required
	{
		test_directory.Format( _T("%s%s"), (LPCSTR)temp_directory, (LPCSTR)prefix );
	}
    else  // a minimum number of digits required
	{
		pathname_format.Format( _T("%%s%%s%%.%dd") ,required_uniquer_digits );
		test_directory.Format( pathname_format, (LPCSTR)temp_directory, (LPCSTR)prefix, counter );
	}

	while ( 1 )
	{
	   unsigned long result = GetFileAttributes( test_directory );
	   if ( -1 == result && ERROR_FILE_NOT_FOUND == GetLastError() )
	   {	
		   if ( TRUE == ::CreateDirectory( test_directory,  0 ) )
			{
				return( test_directory );
			}
			NString reason;
			reason.Format( "Unable to create directory %s, reason:%s", 
					test_directory, NString::GetLastErrorText() );
			throw NRuntimeException( reason, "NFile::CreateUniqueDirectory", __LINE__, __FILE__ );
				
	   }
	   else
	   {
		    counter++;
			test_directory.Format( pathname_format, (LPCSTR)temp_directory, (LPCSTR)prefix, counter );
	   }
	}
}

//---------------------------------------------------------------------
//  These functions tell the number of elements within a path, or extract the 
//   field element at a particular location.  Element 0 is the root directory.
//   successfive elements identify subsequent sub-directories.  To determine the
//   File naming conventins
//   NT and Win 95 Both file systems use the backslash (\) character to separate directory 
//      names and the filename when forming a path. 
//      
//      a.  Use any character in the current code page for a name, but do not use a path separator, 
//          a character in the range 0 through 31, or any character explicitly disallowed by the file system.
//      	A name can contain characters in the extended character set (128-255).     
//      b.  Use the backslash (\), the forward slash (/), or both to separate components in a path. No other 
//          character is acceptable as a path separator.       
//      c.  Use a period (.) as a directory component in a path to represent the current directory. 
//      d.  Two consecutive periods (..) as a directory component in a path to represent the parent of
//          the current directory. 
//      e.  Use a period (.) to separate the base filename from the extension in a directory name or filename. 
//      f.  Do not use the following characters in directory names or filenames, because they are reserved 
//			for Windows: < > : " / \ | 
//      g.  Do not use device names, such as aux, con, and prn, as filenames or directory names. 
//      h   The maximum length for a path, including a trailing backslash, is given by MAX_PATH. 
//      i.   The wide (Unicode) versions of the CreateDirectory, FindFirstFile, GetFileAttributes, and 
//          SetFileAttributes functions permit paths that exceed the MAX_PATH length if the path has the 
//      	"\\?\" or "\\?\UNC\" prefix. These prefixes direct the functions to turn off path parsing. Use 
//      	the "\\?\" prefix with paths for local storage devices and the "\\?\UNC\" prefix with paths 
//      	having the Universal Naming Convention format.
//
//      j.  Do not assume case sensitivity. Consider names such as OSCAR, Oscar, and oscar to be the same. 

long NFile::GetElementCountInPathname( const NString& pathname )
{
	long element_count = 1;
	NString working_path;   
	NString separators =  _T( "/\\");

	// First remove the directory separator.  This is either a:  or \\hostname

	long directory_sep = pathname.Find( _T(':' ) ) ;
	if ( -1 == directory_sep )   // No letter based directory
	{
		directory_sep = pathname.Find( _T( "\\\\") ) ;
		if ( 0 == directory_sep)  // Starts with 
		{
			working_path = pathname.Right( pathname.GetLength() - 2);
			directory_sep = working_path.FindOneOf( separators ) ;
			if ( -1 != directory_sep)   // Now skip host name
			{
				working_path = working_path.Right( working_path.GetLength() - directory_sep - 1 );
			}
		}
		else  // Not a UNC pathname or a letter-based directory
		{
			working_path = pathname;
		}
	}
	else
	{
		working_path = pathname.Right( pathname.GetLength() - directory_sep - 1 );
	}

	// Remove any trailing \ or back slashes

	while( 1 ) 
	{
		long last_char = working_path.GetLength();
		if ( last_char > 0 )
		{
			if ( working_path[last_char-1] == '\\' || working_path[last_char-1] == '/' )
			{
				working_path = working_path.Left( working_path.GetLength() - 1 );
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

	while( working_path.GetLength() > 0  )
	{
		long separator_location = working_path.FindOneOf( separators );
		if (  0 == separator_location )
		{
			working_path = working_path.Right( working_path.GetLength() - 1 );
		}
		else if ( 0 < separator_location  )
		{
			long path_length = working_path.GetLength();
			if ( separator_location < path_length - 1 )
			{
				element_count++;
			}
			working_path = working_path.Right( path_length - separator_location - 1);
		}
		else
		{
			break;
		}
	}
	return( element_count );
}
 
//--------------------------------------------------------------------------------
// To find the i_th element, look for the start of directory part

static NString GetRemainingElementInPathname( const NString& pathname, long index )
{
	CString element;					// This is the part we are constructing
	CString remaining_path = pathname;  // This is leftover part of pathname
	CString separators =  _T( "/\\");
	

	long path_sep = remaining_path.FindOneOf( separators ) ;
	if ( -1 == path_sep )					// Name only - no separators
	{
		element += remaining_path;
	}
	else if ( 0 == path_sep )
	{
		element += remaining_path.Left( path_sep + 1);
		remaining_path = remaining_path.Right( remaining_path.GetLength() - path_sep - 1);
		path_sep = remaining_path.FindOneOf( separators );
		if ( -1 == path_sep )       // \x - no other separators separators
		{
			element += remaining_path;
		}
		else						     // \x\ ...
		{
			element += remaining_path.Left( path_sep );
			remaining_path = remaining_path.Right( remaining_path.GetLength() - path_sep - 1);
			
		}
	}
	else                               // Starts with "bb\" 
	{
		element += remaining_path.Left( path_sep );
		remaining_path = remaining_path.Right( remaining_path.GetLength() - path_sep - 1 );
	}


	while( remaining_path.GetLength() > 0  )
	{
		if ( remaining_path[0] == '\\' || remaining_path[0] == '/' )
		{
			remaining_path = remaining_path.Right(  remaining_path.GetLength() -  1 );
		}
		else
		{
			break;
		}
	}

	if ( index == 0 )
	{
		return( element );
	}
	else
	{
		return( GetRemainingElementInPathname( remaining_path, index - 1 ) );
	}
}

//--------------------------------------------------------------------------------
// To find the i_th element, look for the start of directory part

NString NFile::GetElementInPathname( const NString& pathname, long index )
{
	CString element;					// This is the part we are constructing
	CString remaining_path = pathname;  // This is leftover part of pathname
	CString separators =  _T( "/\\");
	bool is_unc = false;
	long directory_sep = remaining_path.Find( _T(':' ) ) ;
	if ( -1 == directory_sep )
	{
		directory_sep = remaining_path.Find( _T( "\\\\") ) ;
		if ( 0 == directory_sep)  // UNC pathname 
		{
			is_unc = true;
			element = "\\\\";
			remaining_path = remaining_path.Right( pathname.GetLength() - 2);
			directory_sep = remaining_path.FindOneOf( separators ) ;
			if ( -1 != directory_sep)   // Now add host name
			{
				element += remaining_path.Left( directory_sep + 1);
				remaining_path = remaining_path.Right( remaining_path.GetLength() - directory_sep - 1);
			}
		}
	}
	else // Not a UNC or a: format pathname
	{
		element = remaining_path.Left( directory_sep + 1 );
		remaining_path = remaining_path.Right( remaining_path.GetLength() - directory_sep - 1);
	}

	long path_sep = remaining_path.FindOneOf( separators ) ;
	if ( -1 == path_sep )					// Name only - no separators
	{
		element += remaining_path;
	}
	else if ( 0 == path_sep )
	{
		element += remaining_path.Left( path_sep + 1);
		remaining_path = remaining_path.Right( remaining_path.GetLength() - path_sep - 1);
		path_sep = remaining_path.FindOneOf( separators );
		if ( -1 == path_sep )       // \x - no other separators separators
		{
			element += remaining_path;
		}
		else						     // \x\ ...
		{
			element += remaining_path.Left( path_sep );
			remaining_path = remaining_path.Right( remaining_path.GetLength() - path_sep - 1);
			
		}
	}
	else                               // Starts with "bb\" 
	{
		element += remaining_path.Left( path_sep );
		remaining_path = remaining_path.Right( remaining_path.GetLength() - path_sep - 1 );
	}


	while( remaining_path.GetLength() > 0  )
	{
		if ( remaining_path[0] == '\\' || remaining_path[0] == '/' )
		{
			remaining_path = remaining_path.Right(  remaining_path.GetLength() -  1 );
		}
		else
		{
			break;
		}
	}

	if ( index == 0 )
	{
		if ( is_unc )
		{
			element += "\\";
		}
		return( element );
	}
	else
	{
		return( GetRemainingElementInPathname( remaining_path, index - 1 ) );
	}
}

//---------------------------------------------------------------------
//  This function attempts to create the designated path by creating each
//    element of the directory tree.  It returns true if the directory exists 
//    at the end of the process and false otherwise.
//
//  The approach is to find the root directory by extracting everything to the 
//    left of the first \ or /.  If it does not exist, try and create it.  If if
//    does exist, make sure it is a directory.  Then move to the next element of 
//    the pathname.

bool NFile::CreateDirectory( const NString& directory )
{
	long elements = GetElementCountInPathname( directory );
	NString root_dir = GetElementInPathname( directory, 0 );
	if ( !Exists( root_dir  ) )
	{
		if ( TRUE != ::CreateDirectory( root_dir, 0 ) )
		{
			printf ("Unable to create root directory:%s, reson:%s\n",
				root_dir, NString::GetLastErrorText() );
		}
	}

	if ( Exists( root_dir ) )
	{
		for ( long element_index = 1; element_index < elements; element_index++ )
		{
			root_dir += PATH_DELIMITER;
			root_dir += GetElementInPathname( directory, element_index );
			if ( !Exists( root_dir  ) )
			{
				::CreateDirectory( root_dir, 0 );
			}
			if ( !Exists( root_dir ) )
			{
				return( false );
			}
		}
	}
    else
    {
        return( false );
    }

	return( true );
}

//-----------------------------------------------------------
//  This function empties the contents of a directory, without
//   removing the directory.

void NFile::EmptyDirectory( const NString& directory )
{
	if ( directory.GetLength() > 0 )
	{
		WIN32_FIND_DATA file_data; 	// pointer to returned information 
    
		CString search_file = directory + _T("\\*.*" );
		CString current_dir = _T("." );
		CString current_parent = _T(".." );

		HANDLE directory_entry = FindFirstFile( search_file, &file_data );
		if ( directory_entry != INVALID_HANDLE_VALUE )
		{
			if ( current_dir.CompareNoCase( file_data.cFileName ) != 0 
				 && current_parent.CompareNoCase( file_data.cFileName ) != 0)
			{
				CString pathname = directory + _T("\\" ) + file_data.cFileName;
				if ( file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					RemoveDirectory( pathname );
				}
				else
				{
					DeleteFile ( pathname );
				}
			}

			while( TRUE == FindNextFile( directory_entry, &file_data ) )
			{	
				if ( current_dir.CompareNoCase( file_data.cFileName ) != 0 
					 && current_parent.CompareNoCase( file_data.cFileName ) != 0 )
				{
					NString pathname = directory + _T("\\" ) + file_data.cFileName;
					if ( file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					{
						RemoveDirectory( pathname );
					}
					else
					{
						SetFileAttributes( pathname, FILE_ATTRIBUTE_NORMAL );
						DeleteFile ( pathname );
					}
				}
			}	

			FindClose( directory_entry );
		}
	}
}

//---------------------------------------------------------------------
//  This function performs a recursive decent on a directroy tree, deleting
//    all the files in the tree.  It performs this task by enumerating the 
//    entries in the directory.  If the entry is a sub-directory, it removes
//    it recursively.  When all the element have been removed from the 
//    directory, the directory itself is deleted.

void NFile::RemoveDirectoryRecursively( const NString& directory )
{
	if ( directory.GetLength() > 0 )
	{
		WIN32_FIND_DATA file_data; 	// pointer to returned information 
    
		CString search_file = directory + _T("\\*.*" );
		CString current_dir = _T("." );
		CString current_parent = _T(".." );

		HANDLE directory_entry = FindFirstFile( search_file, &file_data );
		if ( directory_entry != INVALID_HANDLE_VALUE )
		{
			if ( current_dir.CompareNoCase( file_data.cFileName ) != 0 
				 && current_parent.CompareNoCase( file_data.cFileName ) != 0)
			{
				CString pathname = directory + _T("\\" ) + file_data.cFileName;
				if ( file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				{
					RemoveDirectory( pathname );
				}
				else
				{
					DeleteFile ( pathname );
				}
			}

			while( TRUE == FindNextFile( directory_entry, &file_data ) )
			{	
				if ( current_dir.CompareNoCase( file_data.cFileName ) != 0 
					 && current_parent.CompareNoCase( file_data.cFileName ) != 0 )
				{
					CString pathname = directory + _T("\\" ) + file_data.cFileName;
					if ( file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
					{
						RemoveDirectory( pathname );
					}
					else
					{
						SetFileAttributes( pathname, FILE_ATTRIBUTE_NORMAL );
						DeleteFile ( pathname );
					}
				}
			}	

			FindClose( directory_entry );
		}
		RemoveDirectory( directory );
	}
}


//---------------------------------------------------------------------
//  The GetRootnameFromPath() function returns the name portion of a full pathname.
//  If the pathname looks like c:\dir1\dir2\filename.ext, the name will be
//   "filename".
//
//  The GetFullnameFromPath() function returns "filename.txt".
//
//  These function work by finding (and skipping) the last occurence of the path 
//   separator(\) in the pathname.  If no file separator is found, they start from
//   the beginning of the string.  The full name takes the remainder of the string
//   and the name function stops after the last extension delimitor (.)

NString NFile::GetRootnameFromPath( const NString &pathname )
{
    int begin = pathname.ReverseFind(PATH_DELIMITER);
	int alt_begin = pathname.ReverseFind(ALT_PATH_DELIMITER);
	if ( -1 == begin && -1 == alt_begin )
	{
		begin = 0;
	}
	else
	{
		begin = max( begin, alt_begin ) + 1;
	}

    int end = pathname.ReverseFind(EXT_DELIMITER);
    if (end == -1)
	{
        end = pathname.GetLength();
	}

    NString name = pathname.Mid( begin, end-begin );
	return name;
}

NString NFile::GetFullnameFromPath( const NString &pathname )
{
	NString name;
    int begin = pathname.ReverseFind(PATH_DELIMITER);
	int alt_begin = pathname.ReverseFind(ALT_PATH_DELIMITER);
	if ( -1 == begin && -1 == alt_begin )
	{
		begin = 0;
	}
	else
	{
		begin = max( begin, alt_begin ) + 1;
	}

	name = pathname.Right( pathname.GetLength() - begin  );
	
	return name;
}

NString NFile::GetDriveLetterFromPath( const NString& pathname )
{
	TCHAR pathname_buffer[MAX_PATH];
	TCHAR* filename = 0;
	unsigned long result = GetFullPathName( pathname, MAX_PATH,	pathname_buffer, &filename );
	if ( result > 0  )
	{
	}
	else
	{
		NString reason;
		reason.Format( "Unable to get full pathname of file:%s, reason:%s",
			pathname, NString::GetLastErrorText() );
	}
	pathname_buffer[2] = 0;
	return( pathname_buffer );
}


NString NFile::GetExtensionFromPath( const NString& pathname )
{
	NString extension;
	
	long separator = pathname.ReverseFind( EXT_DELIMITER );
	if ( separator > 0  )
	{
		extension= pathname.Right( pathname.GetLength() - separator - 1);
	}
	
	return( extension );
}

NString NFile::GetDirectoryFromPath( const NString& pathname )
{
	NString directory( NULL_STRING, 0 );  
	long separator = pathname.ReverseFind( PATH_DELIMITER );
	if ( separator >= 0  )
	{
		directory = pathname.Left( separator );
	}
	else 
	{
		separator = pathname.ReverseFind( ALT_PATH_DELIMITER );
		if ( separator >= 0  )
		{
			directory = pathname.Left( separator );
		}
	}
	return( directory );
}

//---------------------------------------------------------------------
//  The getSize() functions returns the size of the designated file in
//   bytes.  It can be accessed by name or by handle.  The string
//   versions return the size in a text stirng

long NFile::GetSize( const NString& pathname )
{
	HANDLE file = CreateFile( pathname, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	if ( INVALID_HANDLE_VALUE == file )
	{
		NString reason;
		reason.Format("Unable to open file %s, reason:%s",
			pathname, NString::GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetSize", __LINE__, __FILE__ );
	}
	
	long file_size = ::GetFileSize( file, 0 );

	CloseHandle( file );

	return( file_size );
}

//-----------------------------------------------------------------------
//  This function finds the oldest file in the specified directory and returns
//    a handle to it.  The file will be opened for exclusive read access.  It
//    returns true if one exists and false if no files exist.  It performs this
//    function by reading all the files and returning a handle to the oldest.
//  
//  It ignores directories, hidden files, and the designated ignore file.  It also
//    makes sure that the file can be opened for exclusive access.

bool NFile::FindOldestFile
( 
	const NString& directory, 
	NString& pathname,
	const NString& ignore_file
)
{
	WIN32_FIND_DATA file_data; 	// pointer to returned information 
    FILETIME oldest_time;
	bool found_file = false;
	CString search_file = directory + _T("\\*.*" );
	HANDLE directory_entry = FindFirstFile( search_file, &file_data );
	if ( directory_entry != INVALID_HANDLE_VALUE )
	{
		if ( !(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
			 !(file_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) &&
			 ignore_file.CompareNoCase( file_data.cFileName) != 0 )
		{
			pathname = directory + _T("\\" ) + file_data.cFileName;
			oldest_time = file_data.ftLastWriteTime;
			HANDLE file = CreateFile( pathname, GENERIC_READ, 0, 0,
				OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
			if ( INVALID_HANDLE_VALUE != file )
			{
				CloseHandle( file );
				found_file = true;
			}
		}

		bool more_files = true;
		while( more_files )
		{
			if ( TRUE == FindNextFile( directory_entry, &file_data ) )
			{
				if ( !(file_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) && 
					 !(file_data.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)  &&
					ignore_file.CompareNoCase( file_data.cFileName) != 0  )
				{
					if ( !found_file || CompareFileTime( &file_data.ftLastWriteTime, &oldest_time ) < 0 )
					{
						pathname = directory + _T("\\" ) + file_data.cFileName;
						oldest_time = file_data.ftLastWriteTime;
						HANDLE file = CreateFile( pathname, GENERIC_READ, 0, 0,
							OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
						if ( INVALID_HANDLE_VALUE != file )
						{
							CloseHandle( file );
							found_file = true;
						}
					}
				}
			}
			else
			{
				more_files = false;
			}
		}	
		FindClose( directory_entry );
	}
	
	return( found_file );
}
#endif
	
//---------------------------------------------------------------------
//  These getDateTimeCreated() and getDateTimeModified() functions get 
//   the current create and modified file times from the file itself.  
//   The function taking a HANDLE argument implements the data extraction 
//   and formatting operation.  All other funcions are interfaces to it.

SYSTEMTIME NFile::GetDateTimeModified( HANDLE file  )
{
	SYSTEMTIME temp;
#if 0 // TODO
	if ( INVALID_HANDLE_VALUE == file )
	{
		NString reason = GetIntlString( RTC_BASE_ATTEMPT_TO_ACCESS_UNOPEN_FILE );
		throw NLogicException( reason, "NFile::GetDateTimeModified", __LINE__, __FILE__ );
	}
	if ( INVALID_HANDLE_VALUE == file )
	{
		NString reason = GetIntlString( RTC_BASE_ATTEMPT_TO_ACCESS_UNOPEN_FILE );
		throw NLogicException( reason, "NFile::GetDateTimeModified", __LINE__, __FILE__ );
	}

	FILETIME created_time;
	FILETIME last_access_time;
	FILETIME last_write_time;

	if ( TRUE != GetFileTime( file, &created_time, &last_access_time, &last_write_time ) )
	{
		 NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
			"GetFileTime()", GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeModified",__LINE__, __FILE__ );
	}


	FILETIME local_time;

	if ( TRUE != FileTimeToLocalFileTime( &last_write_time, &local_time ) )
	{
	    NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
			_T("FileTimeToLocalFileTime()" ), GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeModified",__LINE__, __FILE__ );
	}	
 
	if ( TRUE != FileTimeToSystemTime( &local_time, &temp ) )
	{
		NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
			_T("FileTimeToLocalFileTime()" ), GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeModified",__LINE__, __FILE__ );
	}
#endif
	return( temp );
}

SYSTEMTIME NFile::GetDateTimeCreated( HANDLE file )
{
	SYSTEMTIME temp;
#if 0 // TODO
	if ( INVALID_HANDLE_VALUE == file )
	{
		NString reason = GetIntlString( UTIL_ATTEMPT_TO_ACCESS_UNOPEN_FILE );
		throw NRuntimeException( reason, "NFile::GetDateTimeCreated", __LINE__, __FILE__ );
	}
	FILETIME created_time;
	FILETIME last_access_time;
	FILETIME last_write_time;

	if ( TRUE != GetFileTime( file, &created_time, &last_access_time, &last_write_time ) )
	{
		NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
			"GetFileTime()", GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeCreated",__LINE__, __FILE__ );
	
	}

	if ( 0 == created_time.dwLowDateTime && 0 == created_time.dwHighDateTime ) // Fat16
	{
		created_time = last_write_time;
	}

	FILETIME local_time;

	if ( TRUE != FileTimeToLocalFileTime( &created_time, &local_time ) )
	{
		NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED , "FileTimeToLocalFileTime()",
			GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeCreated",__LINE__, __FILE__ );
	}	
 
	if ( TRUE != FileTimeToSystemTime( &local_time, &temp ) )
	{
		NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED , "FileTimeToSystemTime()",
			GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeCreated",__LINE__, __FILE__ );

	}
#endif
	return( temp );
}



SYSTEMTIME NFile::GetDateTimeCreated(  const NString& pathname )
{
#if 0 // TODO
	HANDLE file = CreateFile( pathname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	if ( INVALID_HANDLE_VALUE == file )
	{
		NString reason = GetIntlString( RTC_BASE_UNABLE_TO_OPEN_FILE,
			pathname, GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeCreated", __LINE__, __FILE__ );
	}
#endif

	// TODO SYSTEMTIME temp = GetDateTimeCreated( file );
	SYSTEMTIME temp = 0;
	// CloseHandle( file );
	return( temp );
}

SYSTEMTIME NFile::GetDateTimeModified( const NString& pathname )
{
#if 0 // TODO
	HANDLE file = CreateFile( pathname, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
	if ( INVALID_HANDLE_VALUE == file )
	{
		NString reason = GetIntlString( RTC_BASE_UNABLE_TO_OPEN_FILE,
			pathname, GetLastErrorText() );
		throw NRuntimeException( reason, "NFile::GetDateTimeCreated", __LINE__, __FILE__ );

	}
#endif
	// TODO SYSTEMTIME temp = GetDateTimeModified( file );
	SYSTEMTIME temp = 0;

	// CloseHandle( file );

	return( temp );
}

#if 0
//----------------------------------------------------------------------------
//  This function is used to determine if two files have approximaetly the same
//    time of creation ( < 3 sec difference).

bool NFile::HasSameDateTimeModified( const NString &pathname_a, const NString &pathname_b)
{
	SYSTEMTIME time_a = GetDateTimeModified( pathname_a );
	SYSTEMTIME time_b = GetDateTimeModified( pathname_b );
	CTime ctime_a( time_a );
	CTime ctime_b( time_b );

	CTimeSpan time_diff = ctime_a > ctime_b ? ctime_a - ctime_b : ctime_b - ctime_a;

	long diff_seconds = time_diff.GetTotalSeconds () ;
	return ( diff_seconds < 3 );
}
       

//-------------------------------------------------------------------
//NOTE : The time is in Local Time Zone & will be converted to GMT
void NFile::SetDateTimeCreated  ( const SYSTEMTIME &created_time, 
                                     const NString    &pathname )
{
    HANDLE file_handle = CreateFile( pathname, GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if ( INVALID_HANDLE_VALUE == file_handle )
    {
//	   CString reason = GetIntlString( RTC_BASE_UNABLE_TO_OPEN_FILE,
//		    pathname, GetLastErrorText() );
//	    throw NRuntimeException( reason, "NFile::SetDateTimeCreated",  __LINE__, __FILE__ );
    }

    // convert to FileTime format.
    FILETIME local_file_time;
    if ( !SystemTimeToFileTime (&created_time, &local_file_time) )
    {
//		CString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
//			_T("SystemTimeToFileTime()" ), GetLastErrorText() );
 //       CString error_text;
//		error_text.GetBuffer( _MAX_PATH );
 //       error_text.Format ( _T("%s\nFile: %s"), (LPCSTR)reason, (LPCSTR)pathname );
  //      throw NRuntimeException (error_text, "NFile::SetDateTimeCreated", __LINE__, __FILE__);
    }
         
    // convert FileTime to GMT.
    FILETIME gmt_file_time;
    LocalFileTimeToFileTime(&local_file_time, &gmt_file_time);

                                            // create,      last accessed, last write
    BOOL result = SetFileTime( file_handle, &gmt_file_time, NULL,          NULL);
    if ( !result )
    {
//		CString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
//			_T("SetFileTime()" ), GetLastErrorText() );
 //       CString error_text;
//		error_text.GetBuffer( _MAX_PATH );
 //       error_text.Format ( _T("%s\nFile: %s"), (LPCSTR)reason, (LPCSTR)pathname );
  //      throw NRuntimeException (error_text, "NFile::SetDateTimeCreated", __LINE__, __FILE__);
    }
    CloseHandle( file_handle );
}

void NFile::SetDateTimeModified ( const SYSTEMTIME &modified_time, 
                                     const NString    &pathname )
{
    HANDLE file_handle = CreateFile( pathname, GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if ( INVALID_HANDLE_VALUE == file_handle )
    {
//	    CString reason = GetIntlString( RTC_BASE_UNABLE_TO_OPEN_FILE,
//		    pathname, GetLastErrorText() );
//	    throw NRuntimeException( reason, "NFile::SetDateTimeCreated", __LINE__, __FILE__ );
    }

    // convert to FileTime format.
    FILETIME local_file_time;
    if ( !SystemTimeToFileTime (&modified_time, &local_file_time) )
    {
//		CString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
//			_T("SystemTimeToFileTime()" ), GetLastErrorText() );
 //       CString error_text;
//		error_text.GetBuffer( _MAX_PATH );
 //       error_text.Format ( _T("%s\nFile: %s"), (LPCSTR)reason, (LPCSTR)pathname );
  //      throw NRuntimeException (error_text, "NFile::SetDateTimeCreated", __LINE__, __FILE__);
    }
         
    // convert FileTime to GMT.
    FILETIME gmt_file_time;
    LocalFileTimeToFileTime(&local_file_time, &gmt_file_time);

                                         // create, last accessed, last write
    BOOL result = SetFileTime( file_handle, NULL,    NULL,         &gmt_file_time);
    if ( !result )
    {
//		CString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
//			_T("SetFileTime()" ), GetLastErrorText() );
 //       CString error_text;
//		error_text.GetBuffer( _MAX_PATH );
 //       error_text.Format ( _T("%s\nFile: %s"), (LPCSTR)reason, (LPCSTR)pathname );
  //      throw NRuntimeException (error_text, "NFile::SetDateTimeCreated", __LINE__, __FILE__);
    }
    CloseHandle( file_handle );
}

//-------------------------------------------------------------------
void NFile::ResetCreationTime ( const NString &pathname )
{
    HANDLE file_handle = CreateFile( pathname, GENERIC_WRITE,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0 );
    if ( INVALID_HANDLE_VALUE == file_handle )
    {
//	    CString reason = GetIntlString( RTC_BASE_UNABLE_TO_OPEN_FILE,
//		    pathname, GetLastErrorText() );
//	    throw NRuntimeException( reason, "NFile::ResetCreationTime",  __LINE__, __FILE__ );
    }

	FILETIME created_time;
	FILETIME last_access_time;
	FILETIME last_write_time;

	if ( TRUE != GetFileTime( file_handle, &created_time, &last_access_time, &last_write_time ) )
	{
 //       NString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
//			_T("GetFileTime()" ), GetLastErrorText() );
 //       CString error_text;
//		error_text.GetBuffer( _MAX_PATH );
 //       error_text.Format ( _T("%s\nFile: %s"), (LPCSTR)reason, (LPCSTR)pathname );
  //      throw NRuntimeException (error_text, "NFile::ResetCreationTime",  __LINE__, __FILE__);
	}

    BOOL result = SetFileTime( file_handle, &last_write_time, &last_write_time, &last_write_time);
    if ( !result )
    {
//		CString reason = GetIntlString( RTC_BASE_FUNCTION_FAILED,
//			_T("SetFileTime()" ), GetLastErrorText() );
 //       CString error_text;
//		error_text.GetBuffer( _MAX_PATH );
 //       error_text.Format ( _T("%s\nFile: %s"), (LPCSTR)reason, (LPCSTR)pathname );
  //      throw NRuntimeException (error_text, "NFile::ResetCreationTime", __LINE__, __FILE__);
    }
    CloseHandle( file_handle );
}

//-------------------------------------------------------------------
// compareTimes
//      -1	First file time is less than second file time.
//       0	First file time is equal to second file time.
//      +1	First file time is greater than second file time.
long NFile::CompareTimes ( const SYSTEMTIME &time_1, const SYSTEMTIME &time_2 )
{
    CTime ctime_1 (time_1);
    CTime ctime_2 (time_2);

    if (ctime_1 < ctime_2)
        return -1;
    else if (ctime_1 == ctime_2)
        return 0;
    else
        return 1;
}

//-------------------------------------------------------------------
bool NFile::IsModified ( const NString &pathname, CTimeSpan delta )
{
    SYSTEMTIME created  = GetDateTimeCreated (pathname);
    SYSTEMTIME modified = GetDateTimeModified(pathname);

    CTime ct_created  (created);
    CTime ct_modified (modified);

    if ( ct_modified > (ct_created + delta) )
        return true;
    else
        return false;
}

//----------------------------------------------------------------------------
//  This function updates the designated (target) file if the file with the 
//   same name in the designated directory has a newer version.  Also copy if if
//   the target file does not exist

void NFile::UpdateOutdatedFile( const NString& target_pathname, const NString& source_dir )
{
    if ( Exists(source_dir ) )  // If source directory does not exist... can't do anything
    {
        NString filename = GetFullnameFromPath( target_pathname );
        NString source_pathname;
        source_pathname.Format( "%s/%s", (LPCSTR)source_dir, (LPCSTR)filename );  
        if ( source_pathname.CompareNoCase(target_pathname ) != 0  )  // Ignore if the same directory
        {
            if ( Exists( source_pathname ) )   // If source file doesn't exist, can't do anything
            {
                if ( !Exists( target_pathname ) )   // Target doesnt exist, but source does... copy
                {
                    CopyFile( source_pathname, target_pathname, TRUE );
                }
                else  // Source and target exist, copy only if source has been modified more recently
                {
                    SYSTEMTIME source_file_time = NFile::GetDateTimeModified(source_pathname );
                    SYSTEMTIME target_file_time = NFile::GetDateTimeModified(target_pathname );
            
                    if (  NFile::CompareTimes( source_file_time, target_file_time ) > 0  )
                    {
                        std::cerr <<"Detected a change in the file" << std::endl;
         
                        ::CopyFile( source_pathname , target_pathname, FALSE );
                        NFile::SetDateTimeModified( source_file_time, target_pathname );
                    }
                }
            }
        }
    }
}
#endif
