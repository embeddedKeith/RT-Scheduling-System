//  Name:  NException.cpp
//  Description:   Implementation for NException classes
//     This file contains the class definitions for the internationalized
//     exception classes which are used within the embedded product line.
//		NException
//			NFormatException
//			NRuntimeException
//			NLogicException
//			NResourceException
//
//  The data members are:
//		NString reason;					// User displayable information
//		NString supporting_info;		// Optional support info
//		NString file;					// File containing source
//		NString function;				// Function containing source
//		long  line;						// Line exception instantiated
//  Copyright 2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nexception.hpp"   // Exception class interface specifications

NException::NException
( 
	const NString&	reason_arg,		// (input) Preconstructed error string
	const NString&	function_arg,	// (input) Name of function
	long  line_arg,					// (input) Use __LINE__
	const char*		file_arg,		// (input) Use __FILE__
	const NString&	supporting_info_arg
)	: 
	reason( reason_arg ),
	supporting_info( supporting_info_arg ),
	file( file_arg ),
	function( function_arg),
	line( line_arg )
	
{
 
}



NException::NException( const NException& copy ) :
	reason( copy.reason ),
	supporting_info( copy.supporting_info),
	file( copy.file ),
	function( copy.function),
	line( copy.line )
{
	// No code at this time
}

NException& NException::operator = ( const NException& rhs )
{
	if ( this != &rhs )
	{
		reason = rhs.reason;
		function = rhs.function;
		supporting_info = rhs.supporting_info;
		line = rhs.line;
		file = rhs.file;
	}
	return( *this );
}

NException::~NException( void )
{
	// No code at this time
}

//-----------------------------------------------------------
//  The whatReason() function returns the text of the error which can be
//   presented to the user.  The whatLocation() function identifies the
//   point in the code at which the error was detectd.  


NString NException::GetReason( void ) const
{
	return( reason );
}

NString NException::GetFunction( void ) const
{

	return( function );
}

NString NException::GetSupportingInfo( void ) const
{
	return( supporting_info );
}

NString NException::GetLocation( void ) const
{
	char buffer[200];
	sprintf( buffer, "File:  %s Location:%ld", (const char*)file, line );
	return( buffer );
}

NFormatException::NFormatException
(
	const NString&	reason_arg, 
	const NString&	function_arg, 
	long  line_arg,					
	const char*	file_arg,
	const NString& supporting_info_arg
) 	: 
	NException( reason_arg, 	function_arg, line_arg, file_arg, supporting_info_arg )
{
	// No code at this time
}
#if 0
NFormatException::NFormatException
( 
	long id, 
	const NString&	function_arg, 
	long line_arg,					
	const char*	file_arg,				
	const NString& sub1,
	const NString& sub2,
	const NString& sub3,
	const NString& supporting_info_arg
)	: 	NException( id, function_arg, line_arg, file_arg, sub1, sub2, sub3, supporting_info_arg ) 
{
	// No code at this time	
}
#endif
NFormatException::NFormatException( const NFormatException& copy ) 
	: NException( copy ) 
{
	// No code at this time
}

NFormatException& NFormatException::operator = ( const NFormatException& rhs  )
{
	if ( this != &rhs )
	{
		NException::operator = ( rhs );
	}
	return( *this );
}

NFormatException::~NFormatException( void )
{
	// No code at this time
}

NRuntimeException::NRuntimeException
( 
	const NString&	reason_arg, 
	const NString&	function_arg, 
	long  line_arg,					
	const char*	file_arg,
	const NString& supporting_info_arg			
) 	: 
	NException( reason_arg, function_arg, line_arg, file_arg, supporting_info_arg )
{
	// No code at this time
}
#if 0
NRuntimeException::NRuntimeException
( 
	long id, 
	const NString&	function_arg, 
	long line_arg,					
	const char*	file_arg,				
	const NString& sub1,
	const NString& sub2,
	const NString& sub3,
	const NString& supporting_info_arg
)	: 	NException( id, function_arg, line_arg, file_arg, sub1, sub2, sub3, supporting_info_arg ) 
{
	// No code at this time	
}
#endif
NRuntimeException::NRuntimeException( const NRuntimeException& copy )
	: NException( copy ) 
{
	// No code at this time
}

NRuntimeException& NRuntimeException::operator = ( const NRuntimeException& rhs  )
{
	if ( this != &rhs )
	{
		NException::operator = ( rhs );
	}
	return( *this );
}

NRuntimeException::~NRuntimeException( void )
{
	// No code at this time
}

NLogicException::NLogicException
( 
	const NString&	reason_arg, 
	const NString&	function_arg, 
	long  line_arg,					
	const char*	file_arg,
	const NString& supporting_info_arg			
) 	: 
	NException( reason_arg, function_arg, line_arg, file_arg, supporting_info_arg )
{
	// No code at this time
}
#if 0
NLogicException::NLogicException
( 
	long id,  
	const NString&	function_arg,
	long line_arg,					
	const char*	file_arg,				
	const NString& sub1,
	const NString& sub2,
	const NString& sub3,
	const NString& supporting_info_arg
)	: 	NException( id, function_arg, line_arg, file_arg, sub1, sub2, sub3, supporting_info_arg ) 
{
	// No code at this time	
}
#endif
NLogicException::NLogicException( const NLogicException& copy )
	: NException( copy ) 
{
	// No code at this time
}

NLogicException& NLogicException::operator = ( const NLogicException& rhs  )
{
	if ( this != &rhs )
	{
		NException::operator = ( rhs );
	}
	return( *this );
}

NLogicException::~NLogicException( void )
{
	// No code at this time
}

NResourceException::NResourceException
( 
	const NString&	reason_arg, 
	const NString&	function_arg, 
	long  line_arg,					
	const char*	file_arg,
	const NString& supporting_info_arg			
) 	: 
	NException( reason_arg, function_arg, line_arg, file_arg, supporting_info_arg )

	{
	// No code at this time
}
#if 0
NResourceException::NResourceException
( 
	long id,  
	const NString&	function_arg,
	long line_arg,					
	const char*	file_arg,				
	const NString& sub1,
	const NString& sub2,
	const NString& sub3,
	const NString& supporting_info_arg
)	: 	NException( id, function_arg, line_arg, file_arg, sub1, sub2, sub3, supporting_info_arg ) 
{
	// No code at this time	
}
#endif
NResourceException::NResourceException( const NResourceException& copy )
	: NException( copy ) 
{
	// No code at this time
}

NResourceException& NResourceException::operator = ( const NResourceException& rhs  )
{
	if ( this != &rhs )
	{
		NException::operator = ( rhs );
	}
	return( *this );
}

NResourceException::~NResourceException( void )
{
	// No code at this time
}
