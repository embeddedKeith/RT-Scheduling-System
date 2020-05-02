//----------------------------------------------------------------------------
// A more pleasant form of string class
//
// Copyright 2020 embeddedKeith
// -*^*-
//----------------------------------------------------------------------------

#ifndef __NSTRING__
#define __NSTRING__

#include "osinc.hpp"
#include <string>
#include <iostream>
using namespace std;

class _MSExport NString
{
	friend std::ostream& operator << ( std::ostream& os, const NString& rhs );
 
public:
    NString();
    virtual ~NString();
    NString( const char* pszSrc );
    NString( char letter );
    NString( const char* pszSrc, int length);
    NString( const unsigned char* pszSrc, int length);
    NString( const NString& str);
    NString(ULONG ulVal);
    NString(LONG lVal);
    NString(int iVal);
    NString(UINT iVal);
    NString(SHORT sVal);
    NString(USHORT usVal);
    NString(UINT64 Val);
    NString(INT64 Val);

	NString operator >> ( const char* text ) const;
	NString operator >> ( size_t size );
	NString operator >> ( NString& rhs );

    //  These functions are used to convert numerical  and
    //  enumerated values into strings/from strings

    static NString toString(unsigned long value);
    static NString toString(long value);
    static NString toString(int value);
    static NString toString(unsigned int value);
    static NString toString(short value);
    static NString toString(unsigned short value);
    static NString toString(UINT64 value );
    static NString toHexString(long value );
    static NString toHexString(UINT64 value);
    UINT64 asHexINT64(void) const;
    unsigned long asHex(void) const;
    NString& x2d();
    NString& d2x();

    NString& upperCase();
    NString& lowerCase();
    NString& strip();
    int length() const;
    unsigned int asUnsigned() const;
    int asInt() const;
    UINT64 asUnsignedINT64() const;
    INT64 asINT64() const;
    unsigned int indexOf(const NString& aString, unsigned int startPos = 1) const;
    unsigned int indexOf(const char *pString, unsigned int startPos = 1) const;
    unsigned int indexOf(char aChar, unsigned int startPos = 1) const;
    Boolean isDigits() const;
    NString left(unsigned int index) const;
    NString right(unsigned int index) const;

    NString& operator= (const NString& inString);
    NString& operator= (const char* inString);

    operator const char* () const;
    operator const PVOID () const;

    NString getToken(const char *cpMatch, NString& strOut) const;
    NString getToken(int iLen, NString& outString) const;
    NString getToken(const NString& strMatch, NString& strOut) const;
		
    virtual NString mid(int nFirst, int nCount)const;
    virtual NString mid(int nFirst)const;

    friend Boolean operator== (const NString& tca, const NString& tcb);
    friend Boolean operator!= (const NString& tca, const NString& tcb);
    friend Boolean operator>= (const NString& tca, const NString& tcb);
    friend Boolean operator<= (const NString& tca, const NString& tcb);
    friend Boolean operator< (const NString& tca, const NString& tcb);
    friend Boolean operator> (const NString& tca, const NString& tcb);

    friend Boolean operator== (const NString& tca, const char* tcb);
    friend Boolean operator!= (const NString& tca, const char* tcb);
    friend Boolean operator>= (const NString& tca, const char* tcb);
    friend Boolean operator<= (const NString& tca, const char* tcb);
    friend Boolean operator< (const NString& tca, const char* tcb);
    friend Boolean operator> (const NString& tca, const char* tcb);

    void operator+= (const NString& tcA);
    void operator+= (char ch);
    void operator+= (const char* cp);

    friend NString operator+ (const NString& tcA, const NString& tcB);
    friend NString operator+ (const NString& tcA, char ch);
    friend NString operator+ (const NString& tcA, const char* cp);


//    friend NString operator>> (NString& inString, NString& outString);
//    friend NString operator>> (const char *cp, NString& outString);
//    friend NString operator>> (int iLen, NString& outString);

protected:

	//  Heres the scoop... the parsing functions (operator >> ) create 
	//  strings that have separator marks in them.  Subsequent assigments 
	//  of these strings cause the separator to be interpreted and removed.  
	//  The start separator will have the value npos if not assigned,  
	//  If the start_separator value !- npos, the length will save the 
	//  length of the mark.

	void save();   // Removes the "kept" portion of separated string (if any)


    string strData;
	size_t start_separator;	
	size_t length_separator;
};

#endif

