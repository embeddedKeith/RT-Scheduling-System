#ifndef _SCHED_MGR_
#define _SCHED_MGR_
//-----------------------------------------------------------------------------|
// sched_mgr.h
//
//
// developed by: Keith DeWald
//
// started: 8/12/01
//
// last mod: 8/28/01
//-----------------------------------------------------------------------------|
#include <sys/timeb.h>
#include <stl.h>
#include <functional>
#include <map>
#include "asif_protocol.hpp"
#include "rt_bool.hpp"
#include "rtc_time.hpp"
#include "w2char.hpp"
#include "rt_mqueue.hpp"

// different rules for removing an event from the schedule after execution
enum
en_removal
{
	RM_REMOVE_WHEN_EXEC=0,
	RM_REMOVE_AFTER_EXEC_CONFIRM,
	RM_REMOVE_AT_REM_TIME,
	NUM_RM_VALS
};

// 
typedef int device_t;
typedef device_t rtc_device_t;

// bit fields to be filled in later, or this may become a struct
typedef _Uint32t event_status_t;
	

// list of mqueues used in sched_mgr, these names are a local nomenclature
// (local to the sched_mgr process)
//
enum
en_sched_mqueues
{
	SMMQ_TO_MSG = 0,
	SMMQ_TO_ASIF,
	SMMQ_FROM_ASIF,
	SMMQ_TO_DEV,
	SMMQ_FROM_DEV,
	SMMQ_TO_STAT,
	SMMQ_FROM_STAT,
	SMMQ_TO_TC,
	SMMQ_FROM_TC,
	SMMQ_FROM_TCD,
	NUM_SMMQ_VALS
};

//-----------------------------------------------------------------------------|
// Class definition for ms_event_id
//
// combination of ms_id wchar string and event_id.  because event_id is
// only unique across each ms, it has to be considered in combination with
// ms_id to get unique id for any event.
//

class
ms_event_id
{
public:
	unsigned long 	event_id;			// ui's id for event
	RTC_HOSTNAME	owner_id;			// same as used by ui

	bool			operator < (const ms_event_id rhs) const
	{
		bool result;
		// check event_id's first, if they differ, ownner_id's don't matter
		if(this->event_id < rhs.event_id)
		{
			//fprintf(stderr,"ID %ld < ID %ld in op <\n",this->event_id,
			//				rhs.event_id);
			result = TRUE;
		}
		else if(this->event_id > rhs.event_id)
		{
			//fprintf(stderr,"ID %ld > ID %ld in op <\n",
			//				this->event_id, rhs.event_id);
			result = FALSE;
		}
		else
		{
			//fprintf(stderr,"ID %ld == ID %ld in op <, check owner_id\n",
							//this->event_id, rhs.event_id);
			// event_id's are equal, check owner_id's
				char prname1[RTC_HOSTNAME_LENGTH+1];
				char prname2[RTC_HOSTNAME_LENGTH+1];
				w2char_string_to_char((w2char_t *)this->owner_id,
								prname1,RTC_HOSTNAME_LENGTH);
				w2char_string_to_char((w2char_t *)rhs.owner_id,
								prname2,RTC_HOSTNAME_LENGTH);
				prname1[RTC_HOSTNAME_LENGTH]='\0';
				prname2[RTC_HOSTNAME_LENGTH]='\0';
			if(compare_w2char_strings((w2char_t *)this->owner_id, 
				(w2char_t *)rhs.owner_id, (size_t)RTC_HOSTNAME_LENGTH)==-1)
			{
				//fprintf(stderr,"%s < %s\n",prname1,prname2);
				result = TRUE;
			}
			else 
			{
				//fprintf(stderr,"%s >= %s\n",prname1,prname2);
				result = FALSE;
			}
		}
		return(result);
	}
};

//-----------------------------------------------------------------------------|
// Class definition for event
//
const unsigned short default_removal_delay_ms = 100;

// these are the parts of an event that outsiders need to pass in or receive
struct
event_comp_t
{
	ms_event_id		event_id;			// unique event_id with ms_id included
	// unsigned long	event_type;			// play, stop, record, etc.
	rtc_time		exec_time;			// before device latency, 0 for immed
	en_removal		removal_mode;		// remove by time or after execution
	RTC_DEVICE_ID	device_id;			// for which controlled device
	unsigned long	attributes;			// flags for various details
	embeddedAddScheduledEventRequest	request;
	// char *			event_data;			// actual control data for device 
	// unsigned long	data_size;			// size of event_data
	void *			owner_cookie;		// whatever the owner wanted saved
};

class
event
{
public:
	event_comp_t	components;			// parts of event

	rtc_time		time_event_added;	// when added by ui or internal
	rtc_time		remove_time;		// when to remove from schedule

					event(const event_comp_t&);
	int				get_event_status(const event_status_t&);
	int				run_event();
};

#endif // _SCHED_MGR_
//-----------------------------------------------------------------------------|
// Class definition for schedule
//

class
schedule
{
public:
	multimap< rtc_time, event*, less<rtc_time> >::iterator event_i;
	multimap< rtc_time, event*, less<rtc_time> > events;

	multimap< ms_event_id, event*, less<ms_event_id> >::iterator 
			  events_by_id_i;
	multimap< ms_event_id, event*, less<ms_event_id> > events_by_id;

	int				event_count;		// number of events in schedule
	rtc_time 		exec_time_of_next;	// updated on add/remove event
	rtc_time 		exec_time_of_latest;// updated on add/remove event
	schedule();
	int	add_event(const event_comp_t &);
	int remove_event(const ms_event_id &);
	event * find_event(const ms_event_id &);
	event * find_event(const rtc_time &);
	int schedule_exec(const rtc_time &);
};


