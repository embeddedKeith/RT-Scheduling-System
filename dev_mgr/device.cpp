//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	device
//
//	Description:	Individual devices controlled by a RTC system.  Can be
//					a VTR, VideoServer, Router, Keyer, AudioCart, or who
//					knows what else.  Flexibility, inheritance, virtual methods
//					and lots of comments.
//
//	FileName:	device.cpp
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
#include "embeddedRecordDevice.hpp"
#include "nov_log.hpp"
#include "rt_errors.hpp"
#include "asif_protocol.hpp"
#include "device.hpp"

extern FILE* logfile;

//-----------------------------------------------------------------------------|

//#define MSG_MQ DMMQ_TO_MSG

extern mod_mqueue mq[];
extern char stderr_str[MAX_STDLOG_STR_LENGTH];
extern const char mod_chars[4];


//-----------------------------------------------------------------------------|
// device member functions
//
device::device()
{
	bzero((void*)(&subscribers[0]),sizeof(unsigned long)*MAX_SUBSCRIBERS);
	num_subscribers = 0;
	DLOG(DIAG2,DEVICE_mID,"new device\n"); 
};

device::device( const embeddedRTControllerAddDeviceRequest & add_cmd)
{
	bzero((void*)(&subscribers[0]),sizeof(unsigned long)*MAX_SUBSCRIBERS);
	num_subscribers = 0;
	type = add_cmd.type;
	DLOG(DIAG2,DEVICE_mID,"new device has type %d\n",type); 
};

device::~device()
{
	DLOG(DIAG2,DEVICE_mID,"delete device\n"); 
};

int device::add_subscriber(const unsigned long & appl_entity)
{
	int error_code = SUCCESS;
	bool found = false;

	// check to make sure it isn't already a subscriber
	for(unsigned int iis = 0; iis < num_subscribers; iis++)
		if(subscribers[iis] == appl_entity)
		{
			found = true;
			DLOG(CONVFP,TEMP_mID,"Had a subscription for %ld\n",appl_entity);
		}
	
	if(!found && (num_subscribers < MAX_SUBSCRIBERS))
	{
		subscribers[num_subscribers++] = appl_entity;
		DLOG(CONVFP,TEMP_mID,"Added number %d  subscription for %ld\n",
						num_subscribers,appl_entity);
	}
	else if(!found)
		error_code = FAILURE;

	return error_code;
};

int device::remove_subscriber(const unsigned long & appl_entity)
{
	if(num_subscribers)
	{
		for(int iis = 0; iis < (int)num_subscribers; iis++)
		{
			if(subscribers[iis] == appl_entity)
			{
				char did_str[32];
				w2char_string_to_char((w2char_t*)device_id,did_str,
							RTC_DEVICE_ID_LENGTH);
				DLOG(CONVFP,TEMP_mID,"removing subscription for %s status by %ld\n",
						did_str,appl_entity);
				subscribers[iis] = 0;
				if(iis < (int)(num_subscribers - 1))
				{
					//move down valid entries above one removed
					for(int iit = iis; iit < (int)(num_subscribers - 1); iit++)
						subscribers[iit] = subscribers[iit + 1];
				}
				num_subscribers--;
				DLOG(CONVFP,TEMP_mID,"reduced num_subscripers for %ld to %d\n",
								appl_entity, num_subscribers);
			}
		}
	}
	return FAILURE;
};

w2char_t* device::get_device_id()
{
	return (w2char_t*)(&device_id);
};

void device::set_device_id(const RTC_DEVICE_ID& device_id_val)
{
	for(int i=0;i<RTC_DEVICE_ID_LENGTH;i++)
		device_id[i] = device_id_val[i];
	// memmove((void*)device_id,(void*)device_id_val,
	//				sizeof(w2char_t)*RTC_DEVICE_ID_LENGTH);
};


int device::publish_device_status_reports()
{
	int error_code = SUCCESS;
	if(num_subscribers)
		DLOG(DIAG3,DEVICE_mID,
			"send status reports to %d subscribers\n",num_subscribers);
	for(int ids = 0; ids < (int)num_subscribers; ids++)
	{
		// DLOG(CONVFP,TEMP_mID,"type is %d\n",type);
		switch(type)
		{
		case VTR_DEVICE:
		case VIDEO_SERVER_DEVICE:
		{
			embeddedRecordDevice* this_dev = dynamic_cast <embeddedRecordDevice*> (this);

			DLOG(DIAG3,DEVICE_mID,"generate VTR status report\n");
			if(this_dev->generate_device_status_report(subscribers[ids])
					!= SUCCESS)
			{
				error_code = FAILURE;
			}
		}
			break;
		case KEY_GENERATOR_DEVICE:
			break;
		case AUDIO_CART_DEVICE:
		case CG_DEVICE:
		case ROUTER_DEVICE:
			break;
		case UNKNOWN_DEVICE:
		default:
			break;
		}
	}
	return error_code;
};

int device::send_command(unsigned long cmd_type,void * cmd_buf,size_t cmd_size, 
				device_response ** p_dev_resp)
{
	int error_code = SUCCESS;
	embeddedRecordDevice* this_dev = dynamic_cast < embeddedRecordDevice* > (this);

	// sleep(1);
	// DLOG(CONVFP,TEMP_mID,"send_simple_command called with cmd_type %ld\n",cmd_type);
	// sleep(1);

	if(this_dev)
	{
		// QLOG(FATAL,TEMP_mID,"calling function for command\n");
		// call function for command
		switch(cmd_type)
		{
		case 1: //  1:  START_RECORD
			DLOG(DIAG3,DEV_CMD_mID,"****************** RECORD ******************\n");
			this_dev->StartRecord();
			break;
		case 2: //  2:  START_PLAY
			DLOG(DIAG3,DEV_CMD_mID,"****************** PLAY ******************\n");
			this_dev->PlayStart(false);
			break;
		case 3: //  3:  STOP  -  no extra data
			DLOG(DIAG3,DEV_CMD_mID,"****************** STOP ******************\n");
			this_dev->Stop(false);
			break;
		case 4: //  4:  PAUSE  -  no extra data
			DLOG(DIAG3,DEV_CMD_mID,"****************** PAUSE ******************\n");
			break;
		case 5: //  5:  JOG
			DLOG(DIAG3,DEV_CMD_mID,"****************** JOG ******************\n");
			{
				long frames = *(long*)(cmd_buf);
				DLOG(DIAG3,DEV_CMD_mID,"Jog frames is %ld\n",frames);
				this_dev->Jog(frames, false);
			}
			break;
		case 6: //  6:  SHUTTLE
			DLOG(DIAG3,DEV_CMD_mID,"****************** SHUTTLE ******************\n");
			{
				long speed = *(long*)(cmd_buf);
				DLOG(DIAG4,DEV_CMD_mID,"Shuttle speed is %ld\n",speed);
				this_dev->Shuttle(speed,false);
			}
			break;
		case 7: //  7:  STANDBY -  0 ready, 1 standby
			DLOG(DIAG3,DEV_CMD_mID,"****************** STANDBY ******************\n");
			{
				long mode = *(long*)(cmd_buf);
				this_dev->Standby(mode,false);
			}
			break;
		case 8: //  8:  EJECT -  no extra data
			break;
		case 9: //  9:  SETUP to PLAY
			DLOG(DIAG3,DEV_CMD_mID,"****************** SETUP to PLAY ******************\n");
			{
				char clipname[128];
				w2char_string_to_char((w2char_t*)cmd_buf,clipname,128);
				// NTimecode timecode = 0;
				DLOG(DIAG4,DEV_CMD_mID,"clipname is %s\n",clipname);
				this_dev->PlaySetup(clipname,0,false);
			}
			break;
		case 20: //  20. START_KEY
			break;
		case 30: //  30: START_CG
			break;
		case 40: //  40: START_AUDIO
			break;
		case 50: //  50: SET ROUTER
			break;
		case 60: //  60: SET MIXER
			break;
			default:
			break;
		}
	}
	else
		DLOG(WARN,DEV_CMD_mID,"ERROR, this_dev is NULL\n");

	*p_dev_resp = NULL;
	return error_code;
};


