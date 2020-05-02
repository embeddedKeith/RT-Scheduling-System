#ifndef _RTC_TIME_H_
#define _RTC_TIME_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	rtc_time
//
//	Description:	
//
//	FileName:	rtc_time.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/04/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#include <sys/types.h>
#include <time.h>
#include "asif_protocol.hpp"

#define TIMEZONE EAST5

enum
TimeZone
{
	GMT = 0,
	AT1,
	AT2,
	AT3,
	AT4,
	EAST5,
	CENT6,
	MOUNT7,
	PAC8,
	PAC9,
	PAC10,
	PAC11,
	PAC13,
	PAC14,
	PAC15,
	PAC16,
	UNK17,
	UNK18,
	ASIA19,
	ASIA20,
	ASIA21,
	EUR22,
	EUR23,
	NUM_TIMEZONES
};

//	_Uint16t year,month,day,days,hours,minutes,seconds,msecs;
//-----------------------------------------------------------------------------|
struct
tc_notice
{
	unsigned int	tens_hours:4;
	unsigned int	ones_hours:4;
	unsigned int	tens_mins:4;
	unsigned int	ones_mins:4;
	unsigned int	tens_secs:4;
	unsigned int	ones_secs:4;
	unsigned int	tens_frames:4;
	unsigned int	ones_frames:4;
};

void add_secs_to_ntc(RTC_NTIMECODE * p_ntc, int secs);

//-----------------------------------------------------------------------------|
// Class definition for rtc_time
//

class rtc_time
{
public:
	_Uint64t		rtc_ms;
	// bool			operator < (const rtc_time& rhs)
	bool			operator < (const rtc_time rhs) const
	{
		return(this->rtc_ms < rhs.rtc_ms);
	}
	_Uint64t operator = (_Uint64t rhs)
	{
		return (this->rtc_ms = rhs);
	}
	_Uint64t operator = (tc_notice & rhs);
	
	int set_from_RTC_NTIMECODE(const RTC_NTIMECODE &);

	rtc_time(); 
	rtc_time( const _Uint64t * p_rtc_ms); 
	int
	set_time( const _Uint64t * p_rtc_ms);
	int
	up_to( const rtc_time & test_time);

};

rtc_time 	rtc_time_now();

#endif	// _RTC_TIME_H_
