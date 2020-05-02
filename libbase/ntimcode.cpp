//-------------------------------------------------------
// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------

#include "ntimcode.hpp"

_Export NTimeCode::NTimeCode()
{
    uiFramesPerSecond = NTime::getDefaultFramesPerSecond();
    uiFramesPerDay = uiFramesPerSecond * (24 * 60 * 60);
    bDropFrame = NTime::getDefaultDropFrame();
    uiFrames = 0;
    return;
}

_Export NTimeCode::~NTimeCode()
{
    return;
}

_Export NTimeCode::NTimeCode(const charTime& stTime)
{
    INT iHours = getNumber(stTime.caHours, sizeof(stTime.caHours));
    INT iMinutes = getNumber(stTime.caMinutes, sizeof(stTime.caMinutes));
    INT iSeconds = getNumber(stTime.caSeconds, sizeof(stTime.caSeconds));
    INT iFrames = getNumber(stTime.caFrames, sizeof(stTime.caFrames));

    uiFramesPerSecond = NTime::getDefaultFramesPerSecond();
    uiFramesPerDay = uiFramesPerSecond * (24 * 60 * 60);
    uiFrames = ((((iHours % 24) * 60) + (iMinutes % 60)) * 60) +
               (iSeconds % 60);
    uiFrames *= uiFramesPerSecond;
    uiFrames += (iFrames % uiFramesPerSecond);
    bDropFrame = NTime::getDefaultDropFrame();
    return;
}

_Export NTimeCode::NTimeCode(INT iHours, INT iMinutes, INT iSeconds,
                             INT iFrames, INT iFPS)
{
    if(!iFPS || (iFPS != 30 && iFPS != 25))
        uiFramesPerSecond = NTime::getDefaultFramesPerSecond();
    else
        uiFramesPerSecond = iFPS;

    uiFramesPerDay = uiFramesPerSecond * (24 * 60 * 60);
    uiFrames = ((((iHours % 24) * 60) + (iMinutes % 60)) * 60) +
               (iSeconds % 60);
    uiFrames *= uiFramesPerSecond;
    uiFrames += (iFrames % uiFramesPerSecond);
    bDropFrame = NTime::getDefaultDropFrame();
    return;
}

_Export NTimeCode::NTimeCode(const NTimeCode& tcA)
{
    uiFramesPerSecond = tcA.uiFramesPerSecond;
    uiFrames = tcA.uiFrames;
    uiFramesPerDay = tcA.uiFramesPerDay;
    bDropFrame = tcA.bDropFrame;
    return;
}

_Export NTimeCode::NTimeCode(const TCInfo& tci)
{
    uiFramesPerSecond = tci.FPS;
    if(uiFramesPerSecond != 25 && uiFramesPerSecond != 30)
        uiFramesPerSecond = NTime::getDefaultFramesPerSecond();
    uiFramesPerDay = uiFramesPerSecond * (24 * 60 * 60);
    uiFrames = tci.Frames % uiFramesPerDay;
    bDropFrame = tci.DropFrame;
}

_Export NTimeCode::NTimeCode(const VDCPBcdTime& bcdTime)
{
    UINT uiF = getBcdDigit(bcdTime.bcdFrames);
    UINT uiS = getBcdDigit(bcdTime.bcdSeconds);
    UINT uiM = getBcdDigit(bcdTime.bcdMinutes);
    UINT uiH = getBcdDigit(bcdTime.bcdHours);

    uiFramesPerSecond = NTime::getDefaultFramesPerSecond();
    uiFramesPerDay = uiFramesPerSecond * (24 * 60 * 60);
    uiFrames = ((((uiH % 24) * 60) + (uiM % 60)) * 60) +
               (uiS % 60);
    uiFrames *= uiFramesPerSecond;
    uiFrames += (uiF % uiFramesPerSecond);
    bDropFrame = NTime::getDefaultDropFrame();
    return;
}

_Export NTimeCode::NTimeCode(const SONYTCODE& bcdTime)
{

    UINT uiF = getBcdDigit(bcdTime.bcdFrames & 0x3f);
    UINT uiS = getBcdDigit(bcdTime.bcdSeconds & 0x7f);
    UINT uiM = getBcdDigit(bcdTime.bcdMinutes & 0x7f);
    UINT uiH = getBcdDigit(bcdTime.bcdHours & 0x3f);

    uiFramesPerSecond = NTime::getDefaultFramesPerSecond();
    uiFramesPerDay = uiFramesPerSecond * (24 * 60 * 60);
    uiFrames = ((((uiH % 24) * 60) + (uiM % 60)) * 60) +
               (uiS % 60);
    uiFrames *= uiFramesPerSecond;
    uiFrames += (uiF % uiFramesPerSecond);
    bDropFrame = (bcdTime.bcdFrames & 0x40) ? true : false;
    return;
}

void _Export NTimeCode::getTCAsInts(INT& iHour, INT& iMinute,
                                    INT& iSecond, INT& iFrame) const
{
    iFrame = uiFrames;
    iHour = iFrame / (uiFramesPerSecond * 60 * 60);
    iFrame %= (uiFramesPerSecond * 60 * 60);
    iMinute = iFrame / (uiFramesPerSecond * 60);
    iFrame %= (uiFramesPerSecond * 60);
    iSecond = iFrame / uiFramesPerSecond;
    iFrame %= uiFramesPerSecond;

    return;
}

TCInfo& _Export NTimeCode::getTCInfo(TCInfo& tci) const
{
    tci.Frames = uiFrames;
    tci.FPS = uiFramesPerSecond;
    tci.DropFrame = bDropFrame;
    return(tci);
}

void _Export NTimeCode::getVDCPBcd(VDCPBcdTime& bcdTime) const
{
    INT iHour, iMinute, iSecond, iFrame;

    iFrame = uiFrames;
    iHour = iFrame / (uiFramesPerSecond * 60 * 60);
    iFrame %= (uiFramesPerSecond * 60 * 60);
    iMinute = iFrame / (uiFramesPerSecond * 60);
    iFrame %= (uiFramesPerSecond * 60);
    iSecond = iFrame / uiFramesPerSecond;
    iFrame %= uiFramesPerSecond;

    setBcdDigit(bcdTime.bcdFrames, iFrame);
    setBcdDigit(bcdTime.bcdSeconds, iSecond);
    setBcdDigit(bcdTime.bcdMinutes, iMinute);
    setBcdDigit(bcdTime.bcdHours, iHour);

    return;
}

void _Export NTimeCode::getSONYTCODE(SONYTCODE& bcdTime) const
{
    INT iHour, iMinute, iSecond, iFrame;

    iFrame = uiFrames;
    iHour = iFrame / (uiFramesPerSecond * 60 * 60);
    iFrame %= (uiFramesPerSecond * 60 * 60);
    iMinute = iFrame / (uiFramesPerSecond * 60);
    iFrame %= (uiFramesPerSecond * 60);
    iSecond = iFrame / uiFramesPerSecond;
    iFrame %= uiFramesPerSecond;

    if(bDropFrame)
    {
        if(iMinute % 10 == 0 && iSecond == 0)
           if(iFrame < 2) iFrame = 2;
    }

    setBcdDigit(bcdTime.bcdFrames, iFrame);
    setBcdDigit(bcdTime.bcdSeconds, iSecond);
    setBcdDigit(bcdTime.bcdMinutes, iMinute);
    setBcdDigit(bcdTime.bcdHours, iHour);
    if(bDropFrame)
        bcdTime.bcdFrames |= 0x40;

    return;
}

Boolean _Export NTimeCode::setFromString(const NString& Val)
{
    NString strVal(Val);
    NString strHour, strMinute, strSecond, strFrame;
    UINT uiHour, uiMinute, uiSecond, uiFrame;

    strVal.strip();
    if(strVal.indexOf(":"))
    {
        if(strVal.length() == 4 || strVal.length() == 5)
        {
            strVal >> strHour >> ":" >> 1 >> strMinute;
            strSecond = "00";
            strFrame = "00";
        } else if(strVal.length() == 7 || strVal.length() == 8) {
            strVal >> strHour >> ":" >> 1 >> strMinute >> ":" >> 1 >> strSecond;
            strFrame = "00";
        } else if(strVal.length() == 10 || strVal.length() == 11) {
            strVal >> strHour >> ":" >> 1 >> strMinute >> ":" >> 1 >> strSecond >> ":" >> 1 >> strFrame;
        } else return(false);
    } else if(strVal.isDigits() == false) {
        return(false);
    } else if(strVal.length() == 3) {
        strVal >> strHour >> 1 >> strMinute;
        strSecond = "00";
        strFrame = "00";
    } else if(strVal.length() == 4) {
        strVal >> strHour >> 2 >> strMinute;
        strSecond = "00";
        strFrame = "00";
    } else if(strVal.length() == 5) {
        strVal >> strHour >> 1 >> strMinute >> 2 >> strSecond;
        strFrame = "00";
    } else if(strVal.length() == 6) {
        strVal >> strHour >> 2 >> strMinute >> 2 >> strSecond;
        strFrame = "00";
    } else if(strVal.length() == 7) {
        strVal >> strHour >> 1 >> strMinute >> 2 >> strSecond >> 2 >> strFrame;
    } else if(strVal.length() == 8) {
        strVal >> strHour >> 2 >> strMinute >> 2 >> strSecond >> 2 >> strFrame;
    } else return(false);

    if(strHour.isDigits() == false ||
       strMinute.isDigits() == false ||
       strSecond.isDigits() == false ||
       strFrame.isDigits() == false) return(false);

    uiHour = strHour.asUnsigned();
    uiMinute = strMinute.asUnsigned();
    uiSecond = strSecond.asUnsigned();
    uiFrame = strFrame.asUnsigned();

    if(uiHour > 23 || uiMinute > 59 || uiSecond > 59 || uiFrame > uiFramesPerSecond)
        return(false);


    uiFrames = ((((uiHour % 24) * 60) + (uiMinute % 60)) * 60) +
               (uiSecond % 60);
    uiFrames *= uiFramesPerSecond;
    uiFrames += (uiFrame % uiFramesPerSecond);
    return(true);
}

NString _Export NTimeCode::getTimeCodeAsString(Boolean bFrames) const
{
    INT iHour, iMinute, iSecond, iFrame;
    CHAR caBuf[20];

    iFrame = uiFrames;
    iHour = iFrame / (uiFramesPerSecond * 60 * 60);
    iFrame %= (uiFramesPerSecond * 60 * 60);
    iMinute = iFrame / (uiFramesPerSecond * 60);
    iFrame %= (uiFramesPerSecond * 60);
    iSecond = iFrame / uiFramesPerSecond;
    iFrame %= uiFramesPerSecond;

    if(bFrames == true)
    {
        sprintf(caBuf, "%02d:%02d:%02d:%02d", iHour, iMinute,
                iSecond, iFrame);
    } else {
        sprintf(caBuf, "%02d:%02d:%02d", iHour, iMinute, iSecond);
    }
    return(caBuf);
}

void _Export NTimeCode::getCharTime(charTime& stTime) const
{
    INT iHour, iMinute, iSecond, iFrame;
    CHAR caBuf[20];

    iFrame = uiFrames;
    iHour = iFrame / (uiFramesPerSecond * 60 * 60);
    iFrame %= (uiFramesPerSecond * 60 * 60);
    iMinute = iFrame / (uiFramesPerSecond * 60);
    iFrame %= (uiFramesPerSecond * 60);
    iSecond = iFrame / uiFramesPerSecond;
    iFrame %= uiFramesPerSecond;

    sprintf(caBuf, "%02d%02d%02d%02d", iHour, iMinute,
            iSecond, iFrame);
    memmove(&stTime, caBuf, sizeof(stTime));
    return;
}

ULONG _Export NTimeCode::getFrames() const
{
    return(uiFrames);
}

ULONG _Export NTimeCode::getFields() const
{
    return(uiFrames * 2);
}

NTimeCode _Export NTimeCode::tcFromFrames(ULONG frames)
{
    NTimeCode tcTmp;
    tcTmp.uiFrames = frames;
    return(tcTmp);
}

NTimeCode _Export NTimeCode::tcFromFields(ULONG fields)
{
    NTimeCode tcTmp;
    tcTmp.uiFrames = fields / 2;
    return(tcTmp);
}

void _Export NTimeCode::operator= (const NTimeCode &tcA)
{
    uiFramesPerSecond = tcA.uiFramesPerSecond;
    uiFrames = tcA.uiFrames;
    bDropFrame = tcA.bDropFrame;
    uiFramesPerDay = tcA.uiFramesPerDay;
    return;
}

Boolean _Export NTimeCode::setFramesPerSecond(INT iFrames)
{
    if(iFrames == 30 || iFrames == 25)
    {
        uiFramesPerSecond = iFrames;
        uiFramesPerDay = (24 * 60 * 60) * uiFramesPerSecond;
        return(true);
    }
    return(false);
}

INT _Export NTimeCode::getFramesPerSecond() const
{
    return(uiFramesPerSecond);
}

Boolean _Export NTimeCode::setDropFrame(Boolean Drop)
{
    if(uiFramesPerSecond == 25 && Drop == true)
        return(false);

    bDropFrame = Drop;
    return(true);
}

Boolean _Export NTimeCode::getDropFrame() const
{
    return(bDropFrame);
}

void _Export NTimeCode::operator+= (const NTimeCode &tcA)
{
    uiFrames += Convert(tcA);
}

UINT _Export NTimeCode::Convert(const NTimeCode &tcA)
{
    return(((tcA.uiFrames * uiFramesPerSecond) +
                 (tcA.uiFramesPerSecond / 2)) / tcA.uiFramesPerSecond);
}

UINT _Export NTimeCode::getMSec() const
{
    return(((uiFrames * 1000) + (uiFramesPerSecond / 2)) /
             uiFramesPerSecond);
}

void _Export NTimeCode::operator-= (const NTimeCode &tcA)
{
    UINT uiTmpFrames = Convert(tcA);

    if(uiTmpFrames > uiFrames)
        uiFrames += uiFramesPerDay;
    uiFrames -= uiTmpFrames;
    return;
}

void _Export NTimeCode::operator*= (INT iMul)
{
    UINT64 hFrames = uiFrames;

    if(iMul < 0)
    {
        hFrames *= (-iMul);
        uiFrames = hFrames % uiFramesPerDay;
        uiFrames = uiFramesPerDay - uiFrames;
    } else {
        hFrames *= iMul;
        uiFrames = hFrames % uiFramesPerDay;
    }
    return;
}

void _Export NTimeCode::operator/= (INT iDiv)
{
    if(iDiv == 0)
        return;

    if(iDiv > 0)
    {
        uiFrames /= iDiv;
        return;
    }

    uiFrames /= (-iDiv);
    uiFrames = uiFramesPerDay - uiFrames;
}

NTimeCode _Export operator+ (const NTimeCode &tcA, const NTimeCode &tcB)
{
    NTimeCode tcTmp(tcA);

    tcTmp += tcB;
    return(tcTmp);
}

NTimeCode _Export operator- (const NTimeCode &tcA)
{
    NTimeCode tcTmp(tcA);

    tcTmp.uiFrames = tcTmp.uiFramesPerDay - tcTmp.uiFrames;
    return(tcTmp);
}

NTimeCode _Export operator- (const NTimeCode &tcA, const NTimeCode &tcB)
{
    NTimeCode tcTmp(tcA);

    tcTmp -= tcB;
    return(tcTmp);
}

NTimeCode _Export operator* (const NTimeCode &tcA, INT iMul)
{
    NTimeCode tcTmp(tcA);

    tcTmp *= iMul;
    return(tcTmp);
}

NTimeCode _Export operator/ (const NTimeCode &tcA, INT iDiv)
{
    NTimeCode tcTmp(tcA);

    tcTmp /= iDiv;
    return(tcTmp);
}

Boolean _Export operator== (const NTimeCode &tcA, const NTimeCode &tcB)
{
    if(((tcA.uiFrames * tcB.uiFramesPerSecond) / tcA.uiFramesPerSecond) ==
       tcB.uiFrames)
        return(true);
    return(false);
}

Boolean _Export operator!= (const NTimeCode &tcA, const NTimeCode &tcB)
{
    if(((tcA.uiFrames * tcB.uiFramesPerSecond) / tcA.uiFramesPerSecond) !=
       tcB.uiFrames)
        return(true);
    return(false);
}

Boolean _Export operator<= (const NTimeCode &tcA, const NTimeCode &tcB)
{
    if(((tcA.uiFrames * tcB.uiFramesPerSecond) / tcA.uiFramesPerSecond) <=
       tcB.uiFrames)
        return(true);
    return(false);
}

Boolean _Export operator>= (const NTimeCode &tcA, const NTimeCode &tcB)
{
    if(((tcA.uiFrames * tcB.uiFramesPerSecond) / tcA.uiFramesPerSecond) >=
       tcB.uiFrames)
        return(true);
    return(false);
}

Boolean _Export operator< (const NTimeCode &tcA, const NTimeCode &tcB)
{
    if(((tcA.uiFrames * tcB.uiFramesPerSecond) / tcA.uiFramesPerSecond) <
       tcB.uiFrames)
        return(true);
    return(false);
}

Boolean _Export operator> (const NTimeCode &tcA, const NTimeCode &tcB)
{
    if(((tcA.uiFrames * tcB.uiFramesPerSecond) / tcA.uiFramesPerSecond) >
       tcB.uiFrames)
        return(true);
    return(false);
}

NTimeCode _Export NTimeCode::tcFromTime(const NTime &tTime)
{
    NTime tTmp = tTime;
    NTimeCode tcTmp;
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    tTmp.getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);
    tcTmp.uiFrames = (((iHour * 60) + iMinute) * 60) + iSecond;
    tcTmp.uiFrames *= tcTmp.uiFramesPerSecond;
    tcTmp.uiFrames += ((iMSec *tcTmp.uiFramesPerSecond) + 500) / 1000;
    return(tcTmp);
}

NTime _Export NTimeCode::timeFromTc()
{
    INT iHour, iMinute, iSecond, iMSec, iFrame;

    iFrame = uiFrames;
    iHour = iFrame / (uiFramesPerSecond * 60 * 60);
    iFrame %= (uiFramesPerSecond * 60 * 60);
    iMinute = iFrame / (uiFramesPerSecond * 60);
    iFrame %= (uiFramesPerSecond * 60);
    iSecond = iFrame / uiFramesPerSecond;
    iFrame %= uiFramesPerSecond;
    iMSec = ((iFrame * 1000) + (uiFramesPerSecond / 2)) / uiFramesPerSecond;

    NTime tTmp(iHour, iMinute, iSecond, iMSec, 0, 0, 0);
    return(tTmp);
}

NTimeCode & _Export NTimeCode::addTime(const NTime &tA)
{
    NTimeCode tcTmp(*this);

    *this += tcTmp.tcFromTime(tA);
    return(*this);
}

NTimeCode & _Export NTimeCode::addFrames(INT iFrames)
{
    if(iFrames >= 0)
    {
        uiFrames += iFrames;
        uiFrames %= uiFramesPerDay;
    } else {
        iFrames = -iFrames;
        iFrames %= uiFramesPerDay;
        if((unsigned)iFrames > uiFrames)
            uiFrames += uiFramesPerDay;
        uiFrames -= iFrames;
    }
    return(*this);
}
