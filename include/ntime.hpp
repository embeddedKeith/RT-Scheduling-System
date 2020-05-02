
#ifndef __NTime__
#define __NTime__

#include "osinc.hpp"
#include "nstring.hpp"
#include <time.h>

struct charDate 
{
    CHAR caYear[4];
    CHAR caMonth[2];
    CHAR caDay[2];
} __attribute__ ((packed));

struct charTime {
    CHAR caHours[2];
    CHAR caMinutes[2];
    CHAR caSeconds[2];
    CHAR caFrames[2];
} __attribute__ ((packed));

class NTimeData
{
protected:

    INT64 i64MSec;

public:
    NTimeData();
    NTimeData(INT64 MSec);
    ~NTimeData();
    INT64 getMSec() const;
    void setMSec(INT64 MSec);
};

#include "ntimcode.hpp"


// TIME is a 64 bit integer that is actually the milliseconds since
// Jan 1, 0001

class NTimeCode;

class NTime
{
public:
        enum dayOfWeek {enSunday = 0, enMonday, enTuesday, enWednesday,
                        enThursday, enFriday, enSaturday};
	NTime();
	NTime(const NTime& lt);
	NTime(INT iHour, INT iMinute, INT iSecond);
	NTime(INT iHour, INT iMinute, INT iSecond, INT iMSec,
	      INT iYear, INT iMonth, INT iDay);
        NTime(const charDate& stDate, const charTime& stTime);
	NTime(const NString& strDate, const NString& strTime);
	NTime(const NTimeData& dat);
	virtual ~NTime();

	void getTime(INT &iHour, INT &iMinute, INT &iSecond, INT &iMSec,
                     INT &iYear, INT &iMonth, INT &iDay) const;
	NTime& currentTime();
	static NTime getCurrentTime();
	NTime& fromTimet(time_t);
	UINT getDays() const;
	dayOfWeek getDayOfWeek() const;
	NTime &addDays(INT iDays);
	NTime &addMSec(INT iMSec);
	static void setDefaultFramesPerSecond(INT iFrames);
	static void setDefaultDropFrame(Boolean bDropFrame);
	static INT getDefaultFramesPerSecond();
	static Boolean getDefaultDropFrame();

	time_t asTimet() const;

	void getCharDateAndTime(charDate& stDate, charTime& stTime) const;
        void getVSDDateAndTime(ULONG& ulDate, ULONG& ulTime) const;
        void getMediaDateAndTime(LONG& lDate, SHORT& sTime) const;
        void getMediaDate(LONG& lDate) const;
        NTimeData& getDateTimeInfo(NTimeData& dat) const;

        UINT asSeconds() const;
	NString getMMDDYYYY() const;
	NString dateAsString(Boolean b2Digit = false, Boolean bZeroBlank = false) const; // return MM/DD/YYYY
	NString timeAsString(Boolean bZeroBlank = false, Boolean bMSec = false) const; // return HH:MM:SS
	NString dateAndTimeAsString(Boolean b2Digit = false, Boolean bZeroBlank = false) const;
        static Boolean checkAndParseDate(const NString& strDate, UINT* uipYear = NULL,
                                         UINT* uipMonth = NULL,
                                         UINT* uipDay = NULL);
	static Boolean checkAndParseTime(const NString& strTime, UINT* uipHour = NULL,
	                                 UINT* uipMinute = NULL,
	                                 UINT* uipSecond = NULL);

	void operator+= (const NTime &tcA);
	void operator-= (const NTime &tcA);
	void operator+= (const NTimeCode &tcA);
	void operator-= (const NTimeCode &tcA);
	void operator*= (INT iA);
	void operator/= (INT iA);

	friend NTime operator+ (const NTime &tcA, const NTime &tcB);
	friend NTime operator+ (const NTime &tcA, const NTimeCode &tcB);
	friend NTime operator- (const NTime &tcA, const NTime &tcB);
	friend NTime operator- (const NTime &tcA, const NTimeCode &tcB);
	friend NTime operator* (const NTime &tcA, INT iA);
	friend NTime operator/ (const NTime &tcA, INT iA);

        friend Boolean operator== (const NTime& tca, const NTime &tcb);
        friend Boolean operator!= (const NTime& tca, const NTime &tcb);
        friend Boolean operator>= (const NTime& tca, const NTime &tcb);
        friend Boolean operator<= (const NTime& tca, const NTime &tcb);
        friend Boolean operator< (const NTime& tca, const NTime &tcb);
        friend Boolean operator> (const NTime& tca, const NTime &tcb);

protected:
	INT64 hDateTime;
	static const INT iMsPerMinute;
	static const INT iMsPerHour;
	static const INT iMsPerDay;
	static const INT iDaysPerYear;
	static const INT iDaysPer4Years;
	static const INT iDaysPer100Years;
	static const INT iDaysPer400Years;
	static const INT iaMonthDays[];
	static const INT iaLeapMonthDays[];
	static INT iDefaultFramesPerSecond;
	static Boolean bDefaultDropFrame;

	void setTime(INT iHour, INT iMinute, INT iSecond, INT iMSec,
	             INT iYear, INT iMonth, INT iDay);
	void setCurrentTime();
	static INT64 daysFromDate(INT iYear, INT iMonth, INT iDay);
	static void dateFromDays(INT64 hDays, INT &iYear, INT &IMonth, INT &IDay);
};

#endif
