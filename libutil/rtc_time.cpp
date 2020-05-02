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
//	FileName:	rtc_time.cpp
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
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <time.h>
#include "rt_errors.hpp"
#include "rtc_time.hpp"

const unsigned int UEYEAR = 1970; // unix epoch year (00:00:00.000 Jan 1, 1970)

RTC_NTIMECODE ntc;
char printtime[RTC_NTIMECODE_LENGTH+1];
struct timespec tspec;
const unsigned int mdays[] = 	{ 0,31,28,31,30,31,30,31,31,30,31,30,31 };
const unsigned int mtotaldays[] = { 0,31,59,90,120,151,181,
									212,243,273,304,334,365 };
const unsigned int ltotaldays[] = { 0,31,60,91,121,152,182,
									213,244,274,305,335,366 };

const char * mnames[] = { "    ","Jan ","Feb ","Mar ","Apr ","May ","Jun ",
						  "Jul ","Aug ","Sep ","Oct ","Nov ","Dec " };
unsigned long ulmnames[13];
// static int device_count = 0;

rtc_time
rtc_time_now()
{
	rtc_time ret_time(NULL);
	struct timespec tspec;

	clock_gettime(CLOCK_REALTIME, &tspec);
	ret_time.rtc_ms = (_Uint64t)tspec.tv_sec * 1000LL 
					 + (_Uint64t)tspec.tv_nsec/1000000LL;
	return ret_time;
};

void
add_secs_to_ntc(RTC_NTIMECODE * p_ntc,int secs)
{
	int hours,minutes,seconds;
	sscanf((char *)p_ntc,"%2d%2d%2d",&hours,&minutes,&seconds);
	seconds += secs;
	if(seconds > 59)
	{
		minutes += seconds/60;
		seconds %= 60;
		if(minutes > 59)
		{
			hours += minutes/60;
			minutes %= 60;
			if(hours > 23)
			{
				hours %= 24;
			}
		}
	}
	snprintf((char *)p_ntc,6,"%02d%02d%02d",hours,minutes,seconds);
};

//=============================================================================|
// rtc_time member functions
//
rtc_time::rtc_time()
{
};

rtc_time::rtc_time( const _Uint64t * const p_rtc_ms)
{
	if(p_rtc_ms == NULL)
	{
		struct timespec tspec;

		clock_gettime(CLOCK_REALTIME, &tspec);
		rtc_ms = (_Uint64t)tspec.tv_sec * 1000LL 
					 + (_Uint64t)tspec.tv_nsec/1000000LL;
	}
	else
	{
		rtc_ms = *p_rtc_ms;
	}
};

// for assignment from timecode as returned by tc card
_Uint64t rtc_time::operator = (tc_notice & rhs)
{
	int hours;
	int minutes;
	int seconds;
	int frames;
	int msecs;
	int day_secs;
	
	struct timespec tspec;
	clock_gettime(CLOCK_REALTIME, &tspec);
	day_secs = tspec.tv_sec - (tspec.tv_sec % (24*60*60));

	// get hours, minutes, seconds, and frames from rhs
	hours = rhs.tens_hours*10 + rhs.ones_hours;
	minutes = rhs.tens_mins*10 + rhs.ones_mins;
	seconds = rhs.tens_secs*10 + rhs.ones_secs;
	frames = rhs.tens_frames*10 + rhs.ones_frames;
	msecs = (_Uint32t)(frames * 33.33333333);

	rtc_ms = (_Uint64t)((( hours + TIMEZONE)*60 
					+ minutes)*60 + seconds + day_secs)*1000 + msecs;

	return(this->rtc_ms);
};



int
rtc_time::set_time( const _Uint64t * const p_rtc_ms)
{
	int error_val = SUCCESS;
	if(p_rtc_ms == NULL)
		*this = rtc_time_now();
	else
	{
		rtc_ms = *p_rtc_ms;
	}
	return error_val;
};

int
rtc_time::up_to(const rtc_time & test_time)
{
	return (int)(test_time.rtc_ms <= this->rtc_ms);
};
//-----------------------------------------------------------------------------|
int 
rtc_time::set_from_RTC_NTIMECODE(const RTC_NTIMECODE & ntime)
{
	int error_code = SUCCESS;
	int hours;
	int minutes;
	int seconds;
	int frames;
	int day_secs;
	int i;
	struct timespec tspec;
	
	char prntime[RTC_NTIMECODE_LENGTH+1];

	snprintf(prntime,RTC_NTIMECODE_LENGTH,"%s",(char*)&ntime[0]);
	prntime[RTC_NTIMECODE_LENGTH]='\0';
	// fprintf(stderr,"set_from_RTC_NTIMECODE called with %s\n",prntime);

	clock_gettime(CLOCK_REALTIME, &tspec);
	day_secs = tspec.tv_sec - (tspec.tv_sec % (24*60*60));

	// check for legal chars (all digits)
	for(i=0;i<RTC_NTIMECODE_LENGTH;i++)
		if((ntime[i] < '0')||(ntime[i] > '9'))
				error_code = FAILURE;

	sscanf(ntime,"%2d%2d%2d%2d",&hours,&minutes,&seconds,&frames);

	// check for legal values
	if( 	((hours >= 0)&&(hours <= 23))
		&&	((minutes >= 0)&&(minutes <= 59))
		&&	((seconds >= 0)&&(seconds <= 59))
		&&	((frames >= 0)&&(frames <= 29))
		&&	(error_code == SUCCESS))
	{
		rtc_ms = (_Uint64t)(((hours + TIMEZONE)*60 + minutes)*60 
						+ seconds + day_secs)*1000 
				+ (int)((float)frames*33.3667);

		// fprintf(stderr,"formed rtc_ms of %lld\n",rtc_ms);
		rtc_time tmptime;
		tmptime = rtc_time_now();
		// fprintf(stderr,"sys rtc_ms when processed %lld\n",tmptime.rtc_ms);
	}
	else
	{
		error_code = FAILURE;
		// fprintf(logfile,"ERROR: bad input RTC_NTIMECODE of %s\n",ntime);
		// for(i=0;i<RTC_NTIMECODE_LENGTH;i++)
			// fprintf(logfile,"%02X:%02x ",wntime[i].wchar_hi,wntime[i].wchar_lo);
			// fprintf(logfile,"%02x ",ntime[i]);
		// fprintf(logfile,"\n");
	}
	return error_code;
};
//-----------------------------------------------------------------------------|

