//  Name:  NException.h
//  Description:   Interface specification for NException classes
//     This file contains the class definitions for the internationalized
//     exception classes which are used within the embedded product line.
//		NException
//			NFormatException
//			NRuntimeException
//			NLogicException
//			NResourceException
//
//  Copyright 2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#ifndef NException_H
#define NException_H

#include "osinc.hpp"
#include "nstring.hpp"

class  NException 
{
	public:
		NException( const NException& copy );

	   	NException
		( 
			const NString&	reason,		// (input) Preconstructed error string
			const NString&	function,	// (input) Name of function
			long  line,					// (input) Use __LINE__
			const char*		file,		// (input) Use __FILE__
			const NString&	supporting_info
		);
#if 0
		NException                 
		( 
			long id,					// (input) Resource string id
			const NString&	function,	// (input) Name of function
			long line,					// (input) Use __LINE__	
			const char*	file,			// (input) Use __FILE__		
			const NString& sub1 = NString(),
			const NString& sub2 = NString(),
			const NString& sub3 = NString(),
			const NString& supporting_info = NString()
		);
#endif
	   	NException& operator = ( const NException& rhs );

		virtual ~NException( void );

		virtual NString GetReason( void ) const;
	
		virtual NString GetFunction( void ) const;

		virtual NString GetLocation( void ) const;

		virtual NString GetSupportingInfo( void ) const;

	private:
		NException( void );			// Disallowed

		NString reason;					// User displayable information
		NString supporting_info;		// Optional support info
		NString file;					// File containing source
		NString function;				// Function containing source
		long  line;						// Line exception instantiated
};


class  NFormatException : public NException 
{
	public:
		NFormatException
		( 
			const NString&	reason,		// (input) Preconstructed error string
			const NString&	function,	// (input) Name of function
			long  line,					// (input) __LINE__
			const char*		file,		// (input) __FILE__
			const NString&	supporting_info = NString()
		);
#if 0
		NFormatException
		( 
			long id,					// (input) Resource string id
			const NString&	function,	// (input) Name of function
			long line,					// (input) Use __LINE__	
			const char*	file,			// (input) Use __FILE__		
			const NString& sub1 = NString(),
			const NString& sub2 = NString(),
			const NString& sub3 = NString(),
			const NString& supporting_info = NString()
		);
#endif
		NFormatException( const NFormatException& copy );

		NFormatException& operator = ( const NFormatException& rhs );

		virtual ~NFormatException( void );
	private:
		NFormatException( void );			// Disallowed
};

class  NRuntimeException : public NException 
{
	public:
		NRuntimeException
		( 
			const NString&	reason,		// (input) Preconstructed error string
			const NString&	function,	// (input) Name of function
			long  line,					// (input) __LINE__
			const char*		file,		// (input) __FILE__
			const NString&	supporting_info = NString()
		);
#if 0
		NRuntimeException
		( 
			long id,					// (input) Resource string id
			const NString&	function,	// (input) Name of function
			long line,					// (input) Use __LINE__	
			const char*	file,			// (input) Use __FILE__		
			const NString& sub1 = NString(),
			const NString& sub2 = NString(),
			const NString& sub3 = NString(),
			const NString& supporting_info = NString()
		);
#endif
		NRuntimeException( const NRuntimeException& copy );
		NRuntimeException& operator = ( const NRuntimeException& rhs );
		virtual ~NRuntimeException( void );
	private:
		NRuntimeException( void );			// Disallowed
};


class  NLogicException : public NException 
{
	public:
		NLogicException
		( 
			const NString&	reason,		// (input) Preconstructed error string
			const NString&	function,	// (input) Name of function
			long  line,					// (input) __LINE__
			const char*		file,		// (input) __FILE__
			const NString&	supporting_info = NString()
		);
#if 0
		NLogicException
		( 
			long id,					// (input) Resource string id
			const NString&	function,	// (input) Name of function
			long line,					// (input) Use __LINE__	
			const char*	file,			// (input) Use __FILE__		
			const NString& sub1 = NString(),
			const NString& sub2 = NString(),
			const NString& sub3 = NString(),
			const NString& supporting_info = NString()
		);
#endif
		NLogicException( const NLogicException& copy );
		NLogicException& operator = ( const NLogicException& rhs );
		virtual ~NLogicException( void );
	private:
		NLogicException( void );			// Disallowed
};

class  NResourceException : public NException 
{
	public:
		NResourceException
		( 
			const NString&	reason,		// (input) Preconstructed error string
			const NString&	function,	// (input) Name of function
			long  line,					// (input) __LINE__
			const char*		file,		// (input) __FILE__
			const NString&	supporting_info = NString()
		);
#if 0
		NResourceException
		( 
			long id,					// (input) Resource string id
			const NString&	function,	// (input) Name of function
			long line,					// (input) Use __LINE__	
			const char*	file,			// (input) Use __FILE__		
			const NString& sub1 = NString(),
			const NString& sub2 = NString(),
			const NString& sub3 = NString(),
			const NString& supporting_info = NString()
		);
#endif
		NResourceException( const NResourceException& copy );
		NResourceException& operator = ( const NResourceException& rhs );
		virtual ~NResourceException( void );
	private:
		NResourceException( void );			// Disallowed
};

#endif

