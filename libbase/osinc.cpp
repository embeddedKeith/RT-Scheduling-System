//-------------------------------------------------------
// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------

#include "osinc.hpp"

int _Export getNumber(const char* cp, int len)
{
    int iVal = 0;
    Boolean bMinus = false;
    Boolean bStart = false;

    for(int i = 0; i < len; i += 1)
    {
        char ch = *(cp + i);
        if(ch == ' ' && bStart == false) continue;
        if(ch == '-' && bStart == false)
        {
            bMinus = true;
            bStart = true;
            continue;
        }
        if(!isdigit(ch)) break;
        iVal *= 10;
        iVal += ch - '0';
    }
    if(bMinus) iVal = -iVal;
    return(iVal);
}

INT _Export getBcdDigit(UCHAR ucBcdDigit)
{
    return(((ucBcdDigit >> 4) * 10) + (ucBcdDigit & 0x0f));
}

void _Export setBcdDigit(UCHAR& ucBcdDigit, INT iVal)
{
    ucBcdDigit = (((iVal % 100) / 10) << 4) + (iVal % 10);
    return;
}

