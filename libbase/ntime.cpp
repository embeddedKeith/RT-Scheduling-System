//-------------------------------------------------------
// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------

#include "ntime.hpp"

const INT _Export NTime::iMsPerMinute = 60 * 1000;
const INT _Export NTime::iMsPerHour = 60 * iMsPerMinute;
const INT _Export NTime::iMsPerDay = 24 * iMsPerHour;
const INT _Export NTime::iDaysPerYear = 365;
const INT _Export NTime::iDaysPer4Years = (iDaysPerYear * 4) + 1;
const INT _Export NTime::iDaysPer100Years = (iDaysPer4Years * 25) - 1;
const INT _Export NTime::iDaysPer400Years = (iDaysPer100Years * 4) + 1;
const INT _Export NTime::iaMonthDays[] = {0, 31, 59, 90, 120, 151, 181, 212,
                                          243, 273, 304, 334, 365};
const INT _Export NTime::iaLeapMonthDays[] = {0, 31, 60, 91, 121, 152, 182, 213,
                                              244, 274, 305, 335, 366};

INT _Export NTime::iDefaultFramesPerSecond = 30;
Boolean _Export NTime::bDefaultDropFrame = true;


_Export NTime::NTime(const NTimeData& dat)
{
    hDateTime = dat.getMSec();
    return;
}

_Export NTime::NTime()
{
    hDateTime = 0;
    return;
}

_Export NTime::NTime(const NTime &lt)
{
    hDateTime = lt.hDateTime;
    return;
}

_Export NTime::NTime(INT iHour, INT iMinute, INT iSecond)
{

    hDateTime = ((NTime::iMsPerHour * iHour) +
                 (NTime::iMsPerMinute * iMinute) +
                 (1000 * iSecond));
    return;
}

_Export NTime::NTime(INT iHour, INT iMinute, INT iSecond, INT iMSec,
                     INT iYear, INT iMonth, INT iDay)

{
    setTime(iHour, iMinute, iSecond, iMSec,
            iYear, iMonth, iDay);
    return;
}


_Export NTime::NTime(const NString& strDate, const NString& strTime) :
    hDateTime(0)
{
    UINT uiYear, uiMonth, uiDay, uiHour, uiMinute, uiSecond;

    if(checkAndParseDate(strDate, &uiYear, &uiMonth, &uiDay) == false)
        uiYear = uiMonth = uiDay = 0;
    if(checkAndParseTime(strTime, &uiHour, &uiMinute, &uiSecond) == false)
        uiHour = uiMinute = uiSecond = 0;

    setTime(uiHour, uiMinute, uiSecond, 0, uiYear, uiMonth, uiDay);
    return;
}


_Export NTime::NTime(const charDate& stDate,
                     const charTime& stTime)
{
    INT iHour, iMinute, iSecond, iFrame, iMSec;
    INT iYear, iMonth, iDay;

    iYear = getNumber(stDate.caYear, sizeof(stDate.caYear));
    iMonth = getNumber(stDate.caMonth, sizeof(stDate.caMonth));
    iDay = getNumber(stDate.caDay, sizeof(stDate.caDay));

    iHour = getNumber(stTime.caHours, sizeof(stTime.caHours));
    iMinute = getNumber(stTime.caMinutes, sizeof(stTime.caMinutes));
    iSecond = getNumber(stTime.caSeconds, sizeof(stTime.caSeconds));
    iFrame = getNumber(stTime.caFrames, sizeof(stTime.caFrames));
    iMSec = ((iFrame * 1000) + (iDefaultFramesPerSecond / 2)) / iDefaultFramesPerSecond;

    setTime(iHour, iMinute, iSecond, iMSec,
            iYear, iMonth, iDay);
    return;
}

_Export NTime::~NTime()
{
    return;
}

NTimeData& _Export NTime::getDateTimeInfo(NTimeData& dat) const
{
    dat.setMSec(hDateTime);
    return(dat);
}

void _Export NTime::setDefaultFramesPerSecond(INT iFrames)
{
    if(iFrames != 25 && iFrames != 30)
        return;

    NTime::iDefaultFramesPerSecond = iFrames;
    return;
}

void _Export NTime::setDefaultDropFrame(Boolean bDropFrame)
{
    NTime::bDefaultDropFrame = bDropFrame;
    return;
}

INT _Export NTime::getDefaultFramesPerSecond()
{
    return(NTime::iDefaultFramesPerSecond);
}

Boolean _Export NTime::getDefaultDropFrame()
{
    return(NTime::bDefaultDropFrame);
}

UINT _Export NTime::asSeconds() const
{
     return(hDateTime / 1000);
}

void _Export NTime::setTime(INT iHour, INT iMinute, INT iSecond, INT iMSec,
                            INT iYear, INT iMonth, INT iDay)
{
    INT64 hTime = daysFromDate(iYear, iMonth, iDay);

    hTime *= NTime::iMsPerDay;
    hTime += ((NTime::iMsPerHour * iHour) +
              (NTime::iMsPerMinute * iMinute) +
              (1000 * iSecond) + iMSec);
    hDateTime = hTime;
    return;
}

void _Export NTime::getTime(INT &iHour, INT &iMinute, INT &iSecond, INT &iMSec,
                            INT &iYear, INT &iMonth, INT &iDay) const
{
    INT64 hDays = hDateTime / NTime::iMsPerDay;
    INT iTime = hDateTime % NTime::iMsPerDay;

    dateFromDays(hDays, iYear, iMonth, iDay);
    iHour = iTime / NTime::iMsPerHour;
    iMinute = (iTime %= NTime::iMsPerHour) / NTime::iMsPerMinute;
    iSecond = (iTime %= NTime::iMsPerMinute) / 1000;
    iMSec = (iTime % 1000);
    return;
}

NString _Export NTime::getMMDDYYYY() const
{
    CHAR caBuf[24];

    INT iHours, iMinutes, iSeconds, iMSec, iYear, iMonth, iDay;

    getTime(iHours, iMinutes, iSeconds, iMSec, iYear, iMonth, iDay);

    sprintf(caBuf, "%02d/%02d/%04d", iMonth, iDay, iYear);

    return(caBuf);
}

void _Export NTime::getCharDateAndTime(charDate& stDate, charTime& stTime) const
{
    CHAR caBuf[12];
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);

    sprintf(caBuf, "%04d%02d%02d", iYear, iMonth, iDay);
    memmove(&stDate, caBuf, sizeof(stDate));
    sprintf(caBuf, "%02d%02d%02d%02d", iHour, iMinute, iSecond, 0);
    memmove(&stTime, caBuf, sizeof(stTime));
    return;
}

void _Export NTime::getVSDDateAndTime(ULONG& ulDate, ULONG& ulTime) const
{
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);
    ulDate = (((iYear * 100) + iMonth) * 100) + iDay;
    ulTime = (((iHour * 100) + iMinute) * 100) + iSecond;
    return;
}

void _Export NTime::getMediaDateAndTime(LONG& lDate, SHORT& sTime) const
{
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);
    iYear %= 100;
    lDate = (((iYear * 100) + iMonth) * 100) + iDay;
    sTime = (iHour * 100) + iMinute;
    return;
}

void _Export NTime::getMediaDate(LONG& lDate) const
{
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);
    iYear %= 100;
    lDate = (((iYear * 100) + iMonth) * 100) + iDay;
    return;
}

time_t _Export NTime::asTimet() const
{
    tm st;
    INT iMSec;
    INT iYear;

    if(hDateTime == 0)
        return(0);

    getTime(st.tm_hour, st.tm_min, st.tm_sec, iMSec,
            iYear, st.tm_mon, st.tm_mday);
    st.tm_year = iYear - 1900;
    st.tm_mon -= 1;
    st.tm_wday = 0;
    st.tm_yday = 0;
    st.tm_isdst = 0;
    return(mktime(&st));
}

NTime& _Export NTime::fromTimet(time_t ttime)
{
    tm st;

    if(ttime == 0)
    {
        hDateTime = 0;
        return(*this);
    }

    st = *localtime(&ttime);
    setTime(st.tm_hour, st.tm_min, st.tm_sec, 0,
            st.tm_year + 1900, st.tm_mon + 1, st.tm_mday);
    return(*this);
}

NString _Export NTime::dateAsString(Boolean b2D, Boolean bZb) const
{

    CHAR caBuf[20];
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    if(bZb == true && hDateTime == 0)
        return("");

    getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);
    if(b2D == true)
        sprintf(caBuf, "%02d/%02d/%02d", iMonth, iDay, iYear % 100);
    else
        sprintf(caBuf, "%02d/%02d/%04d", iMonth, iDay, iYear);
    return(caBuf);
}

NString _Export NTime::timeAsString(Boolean bZb, Boolean bMSec) const
{

    CHAR caBuf[20];
    INT iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay;

    if(bZb == true && hDateTime == 0)
        return("");
    getTime(iHour, iMinute, iSecond, iMSec, iYear, iMonth, iDay);
    if(bMSec)
        sprintf(caBuf, "%02d:%02d:%02d.%03d", iHour, iMinute, iSecond, iMSec);
    else        
        sprintf(caBuf, "%02d:%02d:%02d", iHour, iMinute, iSecond);
    return(caBuf);
}

NString _Export NTime::dateAndTimeAsString(Boolean b2d, Boolean bZb) const
{
    if(bZb == true && hDateTime == 0)
       return("");

    return(dateAsString(b2d, bZb) + NString(" ") + timeAsString(bZb));
}


NTime& _Export NTime::currentTime()
{
    timespec tsc;
    tm stSysTime;
    clock_gettime(CLOCK_REALTIME, &tsc);
    localtime_r(&tsc.tv_sec, &stSysTime);

    setTime(stSysTime.tm_hour, stSysTime.tm_min, stSysTime.tm_sec,
            tsc.tv_nsec / 1000000, stSysTime.tm_year + 1900,
            stSysTime.tm_mon + 1, stSysTime.tm_mday);

    return(*this);
}

NTime _Export NTime::getCurrentTime()
{
    NTime tmp;
    tmp.currentTime();
    return(tmp);
}

NTime::dayOfWeek _Export NTime::getDayOfWeek() const
{
    INT64 hDays = (hDateTime / iMsPerDay) + 1;
    INT iDay = hDays % 7;
    return(dayOfWeek(iDay));
}

UINT _Export NTime::getDays() const
{
    INT64 hDays = hDateTime / NTime::iMsPerDay;
    UINT uiDays = hDays;
    return(uiDays);
}

INT64 _Export NTime::daysFromDate(INT iYear, INT iMonth, INT iDay)
{
    INT64 hDays = 0;
    Boolean bLeapYear = false;

    if(iYear < 100)
        iYear += (iYear < 90) ? 2000 : 1900;

    if((iYear % 4) == 0 && ((iYear % 100) != 0 || (iYear % 400) == 0))
        bLeapYear = true;

    iYear -= 1;

    hDays = (iYear / 400) * NTime::iDaysPer400Years;
    iYear %= 400;
    hDays += (iYear / 100) * NTime::iDaysPer100Years;
    iYear %= 100;
    hDays += (iYear / 4) * NTime::iDaysPer4Years;
    iYear %= 4;
    hDays += iYear * NTime::iDaysPerYear;

    if(bLeapYear)
        hDays += NTime::iaLeapMonthDays[iMonth - 1];
    else
        hDays += NTime::iaMonthDays[iMonth - 1];

    hDays += iDay - 1;

    return(hDays);
}


void _Export NTime::dateFromDays(INT64 hDays, INT &iYear, INT &iMonth, INT &iDay)
{
    Boolean bLeapYear = false;

    iYear = (hDays / NTime::iDaysPer400Years) * 400;
    hDays %= NTime::iDaysPer400Years;
    if(hDays < (4 * NTime::iDaysPer100Years))
    {
        iYear += (hDays / NTime::iDaysPer100Years) * 100;
        hDays %= NTime::iDaysPer100Years;
    } else {
        iYear += 300;
        hDays -= (3 * NTime::iDaysPer100Years);
    }
    
    iYear += (hDays / NTime::iDaysPer4Years) * 4;
    hDays %= NTime::iDaysPer4Years;

    if(hDays < 4 * NTime::iDaysPerYear)
    {
        iYear += (hDays / NTime::iDaysPerYear);
        hDays %= NTime::iDaysPerYear;
    } else {
        iYear += 3;
        hDays -= (3 * NTime::iDaysPerYear);
    }
    iYear += 1;
    if((iYear % 4) == 0 && ((iYear % 100) != 0 || (iYear % 400) == 0))
        bLeapYear = true;

    iMonth = hDays / 30;
    if(bLeapYear)
    {
       if(NTime::iaLeapMonthDays[iMonth] > hDays)
        iMonth -= 1;
       else if(NTime::iaLeapMonthDays[iMonth + 1] <= hDays)
        iMonth += 1;
       iDay = hDays - NTime::iaLeapMonthDays[iMonth];
    } else {
       if(NTime::iaMonthDays[iMonth] > hDays)
        iMonth -= 1;
       else if(NTime::iaMonthDays[iMonth + 1] <= hDays)
        iMonth += 1;
       iDay = hDays - NTime::iaMonthDays[iMonth];
    }
    iDay += 1;
    iMonth += 1;
    return;
}

NTime & _Export NTime::addDays(INT iDays)
{
    INT64 hAdd = iDays;
    hAdd *= iMsPerDay;
    hDateTime += hAdd;
    return(*this);
}

NTime & _Export NTime::addMSec(INT iMSec)
{
    hDateTime += iMSec;
    return(*this);
}


void _Export NTime::operator+= (const NTime &tcA)
{
    hDateTime += tcA.hDateTime;
    return;
}

void _Export NTime::operator-= (const NTime &tcA)
{
    if(tcA.hDateTime > hDateTime)
        hDateTime = 0;
    else
        hDateTime -= tcA.hDateTime;
    return;
}

void _Export NTime::operator*= (INT iMul)
{
    hDateTime *= iMul;
    if(hDateTime < 0)
        hDateTime = 0;
}

void _Export NTime::operator/= (INT iDiv)
{
    hDateTime /= iDiv;
    return;
}


void _Export NTime::operator+= (const NTimeCode& tcA)
{
    NTimeCode tcTmp(tcA);
    hDateTime += tcTmp.getMSec();
    return;
}

void _Export NTime::operator-= (const NTimeCode& tcA)
{
    NTimeCode tcTmp(tcA);
    hDateTime -= tcTmp.getMSec();
    if(hDateTime < 0)
        hDateTime = 0;
    return;
}

NTime _Export operator+ (const NTime& tcA, const NTime& tcB)
{
    NTime tTmp(tcA);
    tTmp += tcB;
    return(tTmp);
}

NTime _Export operator+ (const NTime& tcA, const NTimeCode& tcB)
{
    NTime tTmp(tcA);
    tTmp += tcB;
    return(tTmp);
}

NTime _Export operator- (const NTime& tcA, const NTime& tcB)
{
    NTime tTmp(tcA);
    tTmp -= tcB;
    return(tTmp);
}

NTime _Export operator- (const NTime& tcA, const NTimeCode& tcB)
{
    NTime tTmp(tcA);
    tTmp -= tcB;
    return(tTmp);
}

NTime _Export operator* (const NTime& tcA, INT iA)
{
    NTime tTmp(tcA);
    tTmp *= iA;
    return(tTmp);
}

NTime _Export operator/ (const NTime& tcA, INT iA)
{
    NTime tTmp(tcA);
    tTmp /= iA;
    return(tTmp);
}

Boolean _Export operator== (const NTime& tca, const NTime &tcb)
{
   return((tca.hDateTime == tcb.hDateTime));
}

Boolean _Export operator!= (const NTime& tca, const NTime& tcb)
{
   return((tca.hDateTime != tcb.hDateTime));
}

Boolean _Export operator>= (const NTime& tca, const NTime& tcb)
{
   return((tca.hDateTime >= tcb.hDateTime));
}

Boolean _Export operator<= (const NTime& tca, const NTime& tcb)
{
   return((tca.hDateTime <= tcb.hDateTime));
}

Boolean _Export operator< (const NTime& tca, const NTime& tcb)
{
   return((tca.hDateTime < tcb.hDateTime));
}

Boolean _Export operator> (const NTime& tca, const NTime& tcb)
{
   return((tca.hDateTime > tcb.hDateTime));
}

Boolean _Export NTime::checkAndParseDate(const NString& Date, UINT* uipYear,
                                         UINT* uipMonth, UINT* uipDay)
{
    NString strDate(Date);

    if(strDate.strip().length() == 0)
    {
        if(uipYear != NULL) *uipYear = 0;
        if(uipMonth != NULL) *uipMonth = 0;
        if(uipDay != NULL) *uipDay = 0;
            return(true);
    }
    NString strMonth, strDay, strYear;
    UINT uiMonth, uiDay, uiYear;

    if(strDate.indexOf("/") != 0)
    {
        strDate >> strMonth >> "/" >> strDay >> "/" >>strYear;
    } else if(strDate.length() == 6 || strDate.length() == 8) {
        strDate >> strMonth >> 2 >> strDay >> 2 >> strYear;
    } else if(strDate.length() == 5 || strDate.length() == 7) {
        strDate >> strMonth >> 1 >> strDay >> 2 >> strYear;
    } else return(false);

    if(strMonth.isDigits() == false ||
       strDay.isDigits() == false ||
       strYear.isDigits() == false)
        return(false);

    uiMonth = strMonth.asUnsigned();
    uiDay = strDay.asUnsigned();
    uiYear = strYear.asUnsigned();
    if(uiYear < 90) uiYear += 2000;
    if(uiYear < 100) uiYear += 1900;

    if(uiMonth > 12 || uiMonth == 0) return(false);
    if(uiDay > 31 || uiDay == 0) return(false);
    if(uiYear < 1900) return(false);
    if(uiDay > 30 && (uiMonth == 4 || uiMonth == 6 ||
                      uiMonth == 9 || uiMonth == 11)) return(false);
    if(uiMonth == 2)
    {
        if(uiDay > 29) return(false);
        if((uiDay > 28) && ((uiYear % 4) != 0) ||
           ((uiYear % 100) == 0 && (uiYear % 400) != 0)) return(false);
    }

    if(uipYear != NULL) *uipYear = uiYear;
    if(uipMonth != NULL) *uipMonth = uiMonth;
    if(uipDay != NULL) *uipDay = uiDay;
    return(true);
}

Boolean _Export NTime::checkAndParseTime(const NString& Time, UINT* uipHour,
                                     UINT* uipMinute, UINT* uipSecond)
{
    NString strTime(Time);
    if(strTime.strip().length() == 0)
    {
        if(uipHour != NULL) *uipHour = 0;
        if(uipMinute != NULL) *uipMinute = 0;
        if(uipSecond != NULL) *uipSecond = 0;
            return(true);
    }
    UINT uiHour, uiMinute, uiSecond;
    NString strHour, strMinute;
    NString strSecond("00");

    if(strTime.indexOf(":") != 0)
    {
        if(strTime.length() == 4 || strTime.length() == 5)
        {
            strTime >> strHour >> ":" >> strMinute;
        } else {
            strTime >> strHour >> ":" >> strMinute >> ":" >> strSecond;
        }
    } else if(strTime.length() == 3) {
       strTime >> strHour >> 1 >> strMinute;
    } else if(strTime.length() == 4) {
       strTime >> strHour >> 2 >> strMinute;
    } else if(strTime.length() == 5) {
       strTime >> strHour >> 1 >> strMinute >> 2 >> strSecond;
    } else if(strTime.length() == 6) {
       strTime >> strHour >> 2 >> strMinute >> 2 >> strSecond;
    } else return(false);

    if(strHour.isDigits() == false ||
       strMinute.isDigits() == false ||
       strSecond.isDigits() == false) return(false);

    uiHour = strHour.asUnsigned();
    uiMinute = strMinute.asUnsigned();
    uiSecond = strSecond.asUnsigned();

    if(uiHour > 23) return(false);
    if(uiMinute > 59) return(false);
    if(uiSecond > 59) return(false);

    if(uipHour != NULL) *uipHour = uiHour;
    if(uipMinute != NULL) *uipMinute = uiMinute;
    if(uipSecond != NULL) *uipSecond = uiSecond;
    return(true);
}

_Export NTimeData::NTimeData()
{
    i64MSec = 0;
    return;
}

_Export NTimeData::NTimeData(INT64 MSec)
{
    i64MSec = MSec;
    return;
}

_Export NTimeData::~NTimeData()
{
    return;
}

INT64 _Export NTimeData::getMSec() const
{
    return(i64MSec);
}

void _Export NTimeData::setMSec(INT64 MSec)
{
    i64MSec = MSec;
    return;
}
