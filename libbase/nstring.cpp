//-------------------------------------------------------
// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nstring.hpp"
#include "w2char.hpp"
#include "nov_log.hpp"
#include <algorithm>
//--------------------------------------------------------
//  This function creates a temporary which contains the portion of
//    the string up to the text being matched.
	
NString NString::operator >> ( const char* text ) const
{
	NString temp;
	if ( temp.start_separator == temp.strData.npos )   // String never separated 
	{
		temp.start_separator = strData.find( text );
		if ( strData.npos != temp.start_separator ) // Found start of sequence
		{
			temp.length_separator = strlen( text );
			temp.strData = strData; 
		}
	}
	return( temp );

}

//-----------------------------------------------------------------
//  Mark the string size characters into the sequence.

NString NString::operator >> ( size_t size )
{
	NString temp;
	save();			// strip any existing marks;
	temp.strData = strData;
	temp.start_separator =  size < strData.length() ? size : strData.length() ;
	temp.length_separator = 0;
	return( temp );
}

//------------------------------------------------------------------------------
//  This function remove the separator marker from the string and loses the 
//     saved portion.
	
void NString::save()
{
	if ( start_separator != strData.npos )
	{
		strData.assign ( strData, start_separator + length_separator, 
			strData.length() - start_separator - length_separator );
		start_separator = strData.npos;
	}
}

//----------------------------------------------------------
//  This function simply assigns "saved" (everything before the separator" portion
//    of lhs to the rhs.  Then removes the separator from the lhs

NString NString::operator >> ( NString& rhs )
{
	if ( start_separator != strData.npos )
	{
		rhs.strData.assign( strData, 0, start_separator );
		save();   // loose the separator sequence
	}
	return( *this );
}

_QnxExport std::ostream& operator << ( std::ostream& os, const NString& rhs )
{
	if ( rhs.start_separator != rhs.strData.npos )
	{
		string temp( rhs.strData, rhs.start_separator + rhs.length_separator,
			 rhs.strData.length() - rhs.start_separator - rhs.length_separator );
		os << temp;
	}
	else
	{
		os << rhs.strData;
	}
	return( os );
}

_QnxExport NString::NString() :
    strData(),
	start_separator( strData.npos ),
	length_separator( 0 )
{
	return;
}

_QnxExport NString::~NString() 
{

}

_QnxExport NString::NString(const char* strSrc) :
    strData(strSrc),
	start_separator( strData.npos )
{
    return;
}
	
_QnxExport NString::NString(char letter) :
    strData(1, letter),
	start_separator( strData.npos )
{
	return;
}

_QnxExport NString::NString(const char* pszSrc, int length) :
    strData(pszSrc) ,
	start_separator( strData.npos )
{
    strData = strData.substr(0, length);
    return;
}

_QnxExport NString::NString(const unsigned char* Src, int length) :
    strData((const char*)Src),
	start_separator( strData.npos ) 
{
    strData = strData.substr(0, length);    
    return;
}

_QnxExport NString::NString(const NString& str) :
    strData(str.strData),
	start_separator( str.start_separator ),
	length_separator( str.length_separator )
{

}

_QnxExport NString::NString(ULONG ulVal) : 	start_separator( strData.npos )
{
    if(ulVal == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    for(; ulVal; ulVal /= 10)
    {
        char ch = (ulVal % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(LONG lVal): 	start_separator( strData.npos )
{
    if(lVal == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }
    Boolean bMinus = false;

    if(lVal < 0)
    {
        bMinus = true;
        lVal = - lVal;
    }

    for(; lVal; lVal /= 10)
    {
        char ch = (lVal % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    if(bMinus)
    {
        string strTmp(1, '-');
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(int Val): 	start_separator( strData.npos )
{
    if(Val == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    Boolean bMinus = false;

    if(Val < 0)
    {
        bMinus = true;
        Val = -Val;
    }

    for(; Val; Val /= 10)
    {
        char ch = (Val % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    if(bMinus)
    {
        string strTmp(1, '-');
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(UINT Val): 	start_separator( strData.npos )
{
    if(Val == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    for(; Val; Val /= 10)
    {
        char ch = (Val % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(SHORT Val): 	start_separator( strData.npos )
{
    if(Val == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    Boolean bMinus = false;

    if(Val < 0)
    {
        bMinus = true;
        Val = -Val;
    }

    for(; Val; Val /= 10)
    {
        char ch = (Val % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    if(bMinus)
    {
        string strTmp(1, '-');
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(USHORT Val): 	start_separator( strData.npos )
{
    if(Val == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    for(; Val; Val /= 10)
    {
        char ch = (Val % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(UINT64 Val) : start_separator( strData.npos )
{
    if(Val == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    for(; Val; Val /= 10)
    {
        char ch = (Val % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    return;
}

_QnxExport NString::NString(INT64 Val) : 	start_separator( strData.npos )
{
    if(Val == 0)
    {
       string strTmp(1, '0');
       strData = strTmp;
       return;
    }

    Boolean bMinus = false;

    if(Val < 0)
    {
        bMinus = true;
        Val = -Val;
    }

    for(; Val; Val /= 10)
    {
        char ch = (Val % 10) + '0';
        string strTmp(1, ch);
        strData.insert(0, strTmp);
    }
    if(bMinus)
    {
        string strTmp(1, '-');
        strData.insert(0, strTmp);
    }
    return;
}

//  These functions are used to convert numerical  and
//  enumerated values into strings/from strings

_QnxExport NString NString::toString(unsigned long value)
{
    return(value);
}

_QnxExport NString NString::toString(long value)
{
    return(value);
}

_QnxExport NString NString::toString(int value)
{
    return(value);
}

_QnxExport NString NString::toString(unsigned int value)
{
    return(value);
}

NString _QnxExport NString::toString(short value)
{
    return(value);
}

NString _QnxExport NString::toString(unsigned short value)
{
    return(value);
}

NString _QnxExport NString::toString(UINT64 value )
{
    return(value);
}

NString _QnxExport NString::toHexString( long value )
{
    NString tmp;

    if(value == 0)
    {
        string schar(1, '0');
        tmp.strData.insert(0, schar);
        return(tmp);
    }

    for(; value; value /= 16)
    {
        char ch = (value % 16) + '0';
        if(ch > '9') ch += ('A' - '9');
        string schar(1, ch);
        tmp.strData.insert(0, schar);
    }
    return(tmp);
}

NString _QnxExport NString::toHexString(UINT64 value )
{
    NString tmp;

    if(value == 0)
    {
        string schar(1, '0');
        tmp.strData.insert(0, schar);
        return(tmp);
    }

    for(; value; value /= 16)
    {
        char ch = (value % 16) + '0';
        if(ch > '9') ch += ('A' - '9');
        string schar(1, ch);
        tmp.strData.insert(0, schar);
    }
    return(tmp);
}


UINT64 _QnxExport NString::asHexINT64(void) const
{
    UINT64 tmp = 0;
    Boolean bStart = false;
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
    {
        char ch = strData[i];
        if(ch == ' ' && bStart == false)
            continue;
        ch = toupper(ch);
        if(!isxdigit(ch)) break;
        tmp *= 16;
        tmp += ch - ((ch > '9') ? ('A' - 10) : '0');
    }
    return(tmp);
}

unsigned long _QnxExport NString::asHex(void) const
{
    unsigned long tmp = 0;
    Boolean bStart = false;
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
    {
        char ch = strData[i];
        if(ch == ' ' && bStart == false)
            continue;
        ch = toupper(ch);
        if(!isxdigit(ch)) break;
        bStart = true;
        tmp *= 16;
        tmp += ch - ((ch > '9') ? ('A' - 10) : '0');
    }
    return(tmp);
}

_QnxExport NString& NString::x2d()
{
    UINT64 tmp = asHexINT64();
    *this = NString(tmp);
    return(*this);
}

_QnxExport NString& NString::d2x()
{
    UINT64 tmp = asUnsignedINT64();
    *this = toHexString(tmp);
    return(*this);
}

_QnxExport NString& NString::upperCase()
{
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
        strData[i] = toupper(strData[i]);
    return(*this);
}

_QnxExport NString& NString::lowerCase()
{
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
        strData[i] = tolower(strData[i]);
    return(*this);
}

_QnxExport NString&  NString::strip()
{
    string::size_type iFirstChar = 0;
    string::size_type iLastChar = 0;
    string::size_type iLen = strData.length();

    for(iFirstChar = 0; iFirstChar < iLen; iFirstChar += 1)
        if(!isspace(strData[iFirstChar])) break;
    if(iFirstChar >= iLen)
    {
        strData.erase();
        return(*this);
    }

    for(iLastChar = iLen - 1; iLastChar > iFirstChar; iLastChar -= 1)
        if(!isspace(strData[iLastChar])) break;

    strData = strData.substr(iFirstChar, iLastChar - iFirstChar + 1);
    return(*this);
}

_QnxExport int NString ::length() const
{
    string::size_type iLen = strData.length();
    return(iLen);
}

_QnxExport unsigned int NString::asUnsigned() const
{
    Boolean bStart = false;
    unsigned int tmp = 0;
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
    {
        char ch;
        if(isspace(ch = strData[i]) && bStart == false)
            continue;

        bStart = true;
        if(!isdigit(ch)) break;
        tmp *= 10;
        tmp += ch - '0';
    }
    return(tmp);
}

_QnxExport int NString::asInt() const
{
    Boolean bStart = false;
    Boolean bMinus = false;
    int tmp = 0;
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
    {
        char ch;
        if(isspace(ch = strData[i]) && bStart == false)
            continue;
        if(ch == '-')
        {
            if(bStart == true) break;
            bStart = true;
            bMinus = true;
            continue;
        }

        bStart = true;
        if(!isdigit(ch)) break;
        tmp *= 10;
        tmp += ch - '0';
    }
    if(bMinus) tmp = -tmp;
    return(tmp);
}
       
 _QnxExport UINT64 NString::asUnsignedINT64() const
{
    Boolean bStart = false;
    UINT64 tmp = 0;
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
    {
        char ch;
        if(isspace(ch = strData[i]) && bStart == false)
            continue;

        bStart = true;
        if(!isdigit(ch)) break;
        tmp *= 10;
        tmp += ch - '0';
    }
    return(tmp);
}

_QnxExport INT64  NString::asINT64() const
{
    Boolean bStart = false;
    Boolean bMinus = false;
    INT64 tmp = 0;
    string::size_type iLen = strData.length();
    for(string::size_type i = 0; i < iLen; i += 1)
    {
        char ch;
        if(isspace(ch = strData[i]) && bStart == false)
            continue;
        if(ch == '-')
        {
            if(bStart == true) break;
            bStart = true;
            bMinus = true;
            continue;
        }

        bStart = true;
        if(!isdigit(ch)) break;
        tmp *= 10;
        tmp += ch - '0';
    }
    if(bMinus) tmp = -tmp;
    return(tmp);
}
       
_QnxExport unsigned int NString::indexOf(const NString& aString, unsigned int startPos) const
{
    if(startPos == 0) startPos = 1;
    string::size_type iPos = strData.find(aString.strData, (startPos - 1));
    if(iPos == string::npos) return(0);
    return(iPos + 1);
}

_QnxExport unsigned int NString::indexOf(const char *pString, unsigned int startPos) const
{
    if(startPos == 0) startPos = 1;
    string tmp(pString);
    string::size_type iPos = strData.find(tmp, (startPos - 1));
    if(iPos == string::npos) return(0);
    return(iPos + 1);
}

_QnxExport unsigned int NString::indexOf(char aChar, unsigned int startPos) const
{
    if(startPos == 0) startPos = 1;
    string::size_type iPos = strData.find(aChar, startPos);
    if(iPos == string::npos) return(0);
    return(iPos + 1);
}

_QnxExport Boolean NString::isDigits() const
{
    string::size_type iPos = strData.length();
    for(string::size_type i = 0; i < iPos; i += 1)
        if(!isdigit(strData[i]))
            return(false);
    return(true);
}

_QnxExport NString NString::left(unsigned int index) const
{
    NString tmp;
    if(index <= 1) return(tmp);
    tmp.strData = strData.substr(0, index - 1);
    return(tmp);
}

_QnxExport NString NString::right(unsigned int index) const
{
    if(index == 0) index = 1;
    NString tmp;
    tmp.strData = strData.substr(index - 1);
    return(tmp);
}


//----------------------------------------------------------------
// When assigning a string, the "saved" portion (if any) is lost and
//   the left-over portion is kept.

_QnxExport NString& NString::operator = (const NString& rhs)
{
	if ( this != &rhs )
	{
		strData = rhs.strData;
		start_separator = rhs.start_separator;
		length_separator = rhs.length_separator;
		save();
	}
	// char hexbuf[1024];
	// hex_byte_dump_to_string(hexbuf,1024,(char *)strData.c_str(),
	// 				strData.length(),31);
	// DLOG(SHOW,TEMP_mID,"strData dump:\n%s\n",hexbuf);
	// hex_byte_dump_to_string(hexbuf,1024,(char *)rhs.strData.c_str(),
	// 				rhs.strData.length(),31);
	// DLOG(SHOW,TEMP_mID,"rhs.strData dump:\n%s\n",hexbuf);
    return(*this);
}

_QnxExport NString& NString::operator= (const char* inString)
{
    strData = inString;
    return(*this);
}

_QnxExport NString::operator const char* () const
{
    return(strData.c_str());
}

_QnxExport NString::operator const PVOID () const
{
    return((PVOID)strData.c_str());
}

_QnxExport NString NString::getToken(const char *cpMatch, NString& strOut) const
{
    NString rem;
    NString tmp(cpMatch);
    string::size_type pos = strData.find(tmp.strData, 0);
    if(pos == string::npos)
    {
        strOut.strData.erase();
        rem.strData = strData;
        return(rem);
    }
    strOut = this->left(pos + 1);
    rem.strData = strData.substr(pos + strOut.strData.length());
    return(rem);
}

_QnxExport NString NString::getToken(int iLen, NString& outString) const
{
    NString rem;
    if(iLen <= 0)
    {
        outString.strData.erase();
        rem.strData = strData;
        return(rem);
    }
    outString.strData = strData.substr(0, iLen);
    rem.strData = strData.substr(iLen);
    return(rem);
}

_QnxExport NString NString::getToken(const NString& strMatch, NString& strOut) const
{
    NString rem;
    string::size_type pos = strData.find(strMatch.strData, 0);
    if(pos == string::npos)
    {
        strOut.strData.erase();
        rem.strData = strData;
        return(rem);
    }
    strOut = this->left(pos + 1);
    rem.strData = strData.substr(pos + strOut.strData.length());
    return(rem);
}

_QnxExport NString NString::mid(int nFirst, int nCount) const
{
    NString tmp;
    if(nFirst <= 0) nFirst = 1;
    if(nCount <= 0) return(tmp);
    tmp.strData = strData.substr((nFirst - 1), nCount);
    return(tmp);
}

_QnxExport NString  NString::mid(int nFirst) const
{
    NString tmp;
    if(nFirst <= 0) nFirst = 1;
    tmp.strData = strData.substr(nFirst);
    return(tmp);
}

_QnxExport Boolean  operator== (const NString& tca, const NString& tcb)
{
    return(tca.strData == tcb.strData);
}

_QnxExport Boolean  operator!= (const NString& tca, const NString& tcb)
{
    return(tca.strData != tcb.strData);
}

_QnxExport Boolean  operator>= (const NString& tca, const NString& tcb)
{
    return(tca.strData >= tcb.strData);
}

_QnxExport Boolean  operator<= (const NString& tca, const NString& tcb)
{
    return(tca.strData <= tcb.strData);
}

_QnxExport Boolean  operator< (const NString& tca, const NString& tcb)
{
    return(tca.strData < tcb.strData);
}

_QnxExport Boolean  operator> (const NString& tca, const NString& tcb)
{
    return(tca.strData > tcb.strData);
}


_QnxExport Boolean operator== (const NString& tca, const char* tcb)
{
    return(tca.strData == tcb);
}

_QnxExport Boolean operator!= (const NString& tca, const char* tcb)
{
    return(tca.strData != tcb);
}

_QnxExport Boolean operator>= (const NString& tca, const char* tcb)
{
    return(tca.strData >= tcb);
}

_QnxExport Boolean operator<= (const NString& tca, const char* tcb)
{
    return(tca.strData <= tcb);
}

_QnxExport Boolean  operator< (const NString& tca, const char* tcb)
{
    return(tca.strData < tcb);
}

_QnxExport Boolean  operator> (const NString& tca, const char* tcb)
{
    return(tca.strData > tcb);
}

_QnxExport void NString::operator+= (const NString& tcA)
{
    strData += tcA.strData;
    return;
}

_QnxExport void  NString::operator+= (char ch)
{
    strData += ch;
    return;
}

_QnxExport void  NString::operator+= (const char* cp)
{
    strData += cp;
    return;
}

_QnxExport NString  operator+ (const NString& tcA, const NString& tcB)
{
    NString tmp(tcA);
    tmp.strData += tcB.strData;
    return(tmp);
}

_QnxExport NString  operator+ (const NString& tcA, char ch)
{
    NString tmp(tcA);
    tmp.strData += ch;
    return(tmp);
}

_QnxExport NString  operator+ (const NString& tcA, const char* cp)
{
    NString tmp(tcA);
    string tmp1(cp);
    tmp.strData += tmp1;
    return(tmp);
}
