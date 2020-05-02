#ifndef __NTimeCode__
#define __NTimeCode__

#include "osinc.hpp"
#include "ntime.hpp"
#include "nstring.hpp"

class NTime;

struct VDCPBcdTime {
    UCHAR bcdFrames;
    UCHAR bcdSeconds;
    UCHAR bcdMinutes;
    UCHAR bcdHours;
} __attribute__ ((packed));

struct SONYTCODE
{
    UCHAR bcdFrames;
    UCHAR bcdSeconds;
    UCHAR bcdMinutes;
    UCHAR bcdHours;
} __attribute__ ((packed));

struct TCInfo
{
    UINT Frames;
    UINT FPS;
    Boolean DropFrame;
} __attribute__ ((packed));

class NTimeCode
{
public:
    NTimeCode();
    NTimeCode(INT iHours, INT iMinutes = 0,
              INT iSeconds = 0, INT iFrames = 0, INT iFPS = 0);
    NTimeCode(const charTime& stTime);
    NTimeCode(const NTimeCode &tcStart);
    NTimeCode(const VDCPBcdTime& bcdTime);
    NTimeCode(const SONYTCODE& sonyTime);
    NTimeCode(const TCInfo& tci);
    virtual ~NTimeCode();

    NTime timeFromTc();
    static NTimeCode tcFromTime(const NTime& tTime);
    static NTimeCode tcFromFrames(ULONG ulFrames);
    static NTimeCode tcFromFields(ULONG ulFields);
    NTimeCode& addFrames(INT iFrames);
    NTimeCode& addTime(const NTime &tTime);

    Boolean setFramesPerSecond(INT iFrames);
    INT getFramesPerSecond() const;
    Boolean setDropFrame(Boolean bDrop);
    Boolean getDropFrame() const;
    UINT getMSec() const;
    void getVDCPBcd(VDCPBcdTime& bcdTime) const;
    void getSONYTCODE(SONYTCODE& sonyTime) const;
    TCInfo& getTCInfo(TCInfo& tci) const;
    NString getTimeCodeAsString(Boolean bFrames = true) const;
    void getCharTime(charTime& stTime) const;
    void getTCAsInts(INT& iHour, INT& iMinute, INT& iSecond, INT& iFrame) const;
    Boolean setFromString(const NString& strVal);
    ULONG getFields() const;
    ULONG getFrames() const;

    void operator= (const NTimeCode &tcStart);

    void operator+= (const NTimeCode &tcA);
    void operator-= (const NTimeCode &tcA);
    void operator*= (INT iA);
    void operator/= (INT iA);

    friend NTimeCode operator+ (const NTimeCode &tcA, const NTimeCode &tcB);
    friend NTimeCode operator- (const NTimeCode &tcA);
    friend NTimeCode operator- (const NTimeCode &tcA, const NTimeCode &tcB);
    friend NTimeCode operator* (const NTimeCode &tcA, INT iA);
    friend NTimeCode operator/ (const NTimeCode &tcA, INT iA);

    friend Boolean operator== (const NTimeCode &tcA, const NTimeCode &tcB);
    friend Boolean operator!= (const NTimeCode &tcA, const NTimeCode &tcb);
    friend Boolean operator<= (const NTimeCode &tcA, const NTimeCode &tcb);
    friend Boolean operator>= (const NTimeCode &tcA, const NTimeCode &tcb);
    friend Boolean operator< (const NTimeCode &tcA, const NTimeCode &tcb);
    friend Boolean operator> (const NTimeCode &tcA, const NTimeCode &tcb);

protected:
    UINT uiFramesPerSecond;
    UINT uiFramesPerDay;
    Boolean bDropFrame;
    UINT uiFrames;

    UINT Convert(const NTimeCode &tcA);
};

#endif
