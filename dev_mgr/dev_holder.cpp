//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	dev_holder
//
//	Description:	This class "holds" all of the devices installed on this
//					RTC.  It creates or registers devices on command from the
//					Master Scheduler that took the user input for device
//					installation.
//					On Startup, all known devices are instantiated and a
//					device configuration message is prepared for sending to
//					any MS queries received.
//					Each device instantiated starts its own thread for
//					managing that device.
//					This is a very rough copy of the holder concept used
//					for the orion sattelite control record devices.
//
//	FileName:	dev_holder.cpp
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
#include "nov_log.hpp"
#include "rt_errors.hpp"
#include "embeddedRecordDevice.hpp"
#include "embeddedSerialRecordDevice.hpp"
#include "embeddedSonyRecorder.hpp"
#include "embeddedVDCPRecorder.hpp"
#include "embeddedBtsRouter.hpp"
#include "device.hpp"
#include "dev_holder.hpp"

extern FILE* logfile;

extern mod_mqueue mq[];
extern char stderr_str[MAX_STDLOG_STR_LENGTH];
extern const char mod_chars[4];



//-----------------------------------------------------------------------------|
#define MAX_SER_NAME_LENGTH 16
static
device *
NewCorrectRecorderDevice(const embeddedRTControllerAddDeviceRequest& dev_add_cmd)
{
	device * p_device = NULL;
	switch(dev_add_cmd.vtr.type)
	{
    case NetVTR: 
		break;
    case SonyVTR:
		{
			char did_str[RTC_DEVICE_ID_LENGTH];
			w2char_string_to_char((w2char_t*)(dev_add_cmd.device_id),
							did_str,RTC_DEVICE_ID_LENGTH);
			DLOG(WARN,DEV_HOLDER_mID,"get new embeddedSonyRecorder object %s\n",
							did_str);
			char sername[MAX_SER_NAME_LENGTH];
			sername[snprintf(sername,MAX_SER_NAME_LENGTH,"ser%1ld",
							dev_add_cmd.port_number)]=0;

			DLOG(WARN,MISC_mID,"sername is %s\n",sername);
			p_device = new embeddedSonyRecorder(dev_add_cmd,did_str,1,sername);
			// start doing status checks via port
			p_device->SetActive(true);
		}
		break;
    case TekVServ:
			char did_str[RTC_DEVICE_ID_LENGTH];
			w2char_string_to_char((w2char_t*)(dev_add_cmd.device_id),
							did_str,RTC_DEVICE_ID_LENGTH);
			DLOG(SHOW,DEVICE_mID,"get new embeddedVDCPRecorder object\n");
			char sername[MAX_SER_NAME_LENGTH];
			sername[snprintf(sername,MAX_SER_NAME_LENGTH,"ser%1ld",
							dev_add_cmd.port_number)]=0;
			p_device = new embeddedVDCPRecorder(dev_add_cmd,did_str,1,sername,
							dev_add_cmd.video_server.record_channel,
							dev_add_cmd.video_server.play_channel,
							dev_add_cmd.video_server.unit_number,
							false,false);
			// start doing status checks via port
			p_device->SetActive(true);
		break;
    case AmpexVTR:
		break;
	default:
		break;
	}
	if(p_device == NULL)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"No device assigned at line %d, p_device null, abort\n", __LINE__);
		abort();
	}
	return p_device;
};

//-----------------------------------------------------------------------------|
static
device *
NewCorrectVideoServerDevice(const embeddedRTControllerAddDeviceRequest& dev_add_cmd)
{

	device * p_device = NULL;

	DLOG(DIAG3,DEV_HOLDER_mID,"ready to switch on video_server.type %d\n",
					dev_add_cmd.video_server.type);
	switch(dev_add_cmd.video_server.type)
	{
    case HPVServ:
		break;
    case TekVServ:
			char did_str[RTC_DEVICE_ID_LENGTH];
			w2char_string_to_char((w2char_t*)(dev_add_cmd.device_id),
							did_str,RTC_DEVICE_ID_LENGTH);
			DLOG(SHOW,DEVICE_mID,"get new embeddedVDCPRecorder object\n");
			char sername[MAX_SER_NAME_LENGTH];

			sername[snprintf(sername,MAX_SER_NAME_LENGTH,"ser%1ld",
							dev_add_cmd.port_number)]=0;

			DLOG(DIAG3,DEV_HOLDER_mID,"calling constructor for embeddedVDCPRecorder\n");
			p_device = new embeddedVDCPRecorder(dev_add_cmd,did_str,1,sername,
							dev_add_cmd.video_server.record_channel,
							dev_add_cmd.video_server.play_channel,
							dev_add_cmd.video_server.unit_number,
							false,false);
			// start doing status checks via port
			// p_device->SetActive(true);
		break;
    case DmyVS:
		break;
	case UnknownServ:
	default:
		break;
	}
	if(p_device == NULL)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"No device assigned at line %d, p_device null, abort\n", __LINE__);
		abort();
	}
	return p_device;
};

//-----------------------------------------------------------------------------|
static
device *
NewCorrectVideoRouterDevice(const embeddedRTControllerAddDeviceRequest& dev_add_cmd)
{
	device * p_device = NULL;
	embeddedRouterData * p_routerData;
	switch(dev_add_cmd.video_router.type)
	{
	case BTS_ROUTER:
		char did_str[RTC_DEVICE_ID_LENGTH];
		w2char_string_to_char((w2char_t*)(dev_add_cmd.device_id),
						did_str,RTC_DEVICE_ID_LENGTH);
		DLOG(SHOW,DEVICE_mID,"get new embeddedRouter object\n");
		char sername[MAX_SER_NAME_LENGTH];

		sername[snprintf(sername,MAX_SER_NAME_LENGTH,"ser%1ld",
						dev_add_cmd.port_number)]=0;

		DLOG(DIAG3,DEV_HOLDER_mID,"calling constructor for embeddedVDCPRecorder\n");

		// build up embeddedRouterData to pass to embeddedBtsRouter constructor
	 	p_routerData = new embeddedRouterData();

	// member data of embeddedRouterData
/*
	   CHAR caRouterName[RouterNameSize];   // Router Name ( 64 bytes)
	   RouterDeviceType enType;				// Router Type;
	   CHAR caPortName[PORT_NAME_LENGTH+1];	// Port Name
	   BOOLEAN local_port;                  // FLag indicating that port is local port
	   CHAR remote_host[HOSTNAME_LENGTH + 1]; // Remote hostname length
	   ULONG ulAddress;						// Router Address
	   ULONG ulInputs;						// Number of Inputs
	   ULONG ulOutputs;						// Number of Outputs
	   LvlMap stMap;						// Level Map  (84 byte)
	   CHAR caDummy[171];					// reserve some space
struct VideoRouter
{
	unsigned long type;       		// model
	unsigned long protocol;   		// comm protocol
	unsigned long address;			//
	unsigned long number_inputs; 	// 
	unsigned long number_outputs; 	//
	unsigned long number_levels; 	// 21 is max value
	unsigned long level_map[MAX_ROUTER_LEVELS]; 	// logical to physical map
	unsigned long level_array[MAX_ROUTER_LEVELS]; 	// physical to input to output
};
struct embeddedRTControllerAddDeviceRequest
{             
    DeviceType		type ;              // device type (struct selector)         
    RTC_DEVICE_ID	device_id; 			// device id         
    w2char_t access;                   // S - Serial port, N network port, 
    w2char_t mode;                     // M - master, B backup control mode         
    w2char_t port_mode;               // 2 - Rs232 , 4 -Rs422        
    w2char_t padding;
    RTC_INET_ADDR inet_addr;          // 000.000.000.000        
    unsigned long port_number;        // 1 - N         
    unsigned long baud_rate;          // values:          
    unsigned long parity;             // values:         
    unsigned long stop_bits;          // values:         
    unsigned char alternate_addr[16]; // slave/master inet_addr or nulls       
    union     
    {         
        struct VideoRecorder	vtr;           // Serial        
        struct VideoServer      video_server; 
        struct VideoRouter 		video_router;

    };
};
*/
	//    CHAR caRouterName[RouterNameSize];   // Router Name ( 64 bytes)

	 	p_device = new embeddedBtsRouter(dev_add_cmd,*p_routerData,
						(long unsigned int)1,
						(long unsigned int)1,
						(NPort::Baud)(dev_add_cmd.baud_rate));
		break;
	default:
		break;
	}
	if(p_device == NULL)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"No device assigned at line %d, p_device null, abort\n", __LINE__);
		abort();
	}
	return p_device;
};

//-----------------------------------------------------------------------------|
static
device *
NewCorrectVideoKeyerDevice(const embeddedRTControllerAddDeviceRequest& dev_add_cmd)
{
	device * p_device = NULL;
	//switch(dev_add_cmd.video_keyer.type)
	//{
	//default:
		//break;
	//}
	if(p_device == NULL)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"No device assigned at line %d, p_device null, abort\n", __LINE__);
		abort();
	}
	return p_device;
};

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
// dev_holder member functions
//

dev_holder::dev_holder(const char* const dev_info_filename):
	when_started((_Uint64t *)NULL)
{
	bzero(&device_info[0],sizeof(dev_info)*MAX_DEVICES);
	num_installed = 0;
	bzero(&index_by_port_num[0],sizeof(unsigned short)*MAX_DEVICES);
	
	strcpy(info_filename,dev_info_filename);

	if(dev_get_hldr_info(dev_info_filename)==-1)
	{
		DLOG(WARN,DEV_HOLDER_mID,"ERROR: failed to get device info from file %s\n",
			dev_info_filename);
	}

};

//-----------------------------------------------------------------------------|
dev_holder::~dev_holder()
{
	// save the hldr info in filename currently assigned
	if(dev_save_hldr_info(info_filename)!=-1)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"Failed to write dev_holder info to file %s\n", info_filename);
	}
	// delete all devices



};

//-----------------------------------------------------------------------------|
int
dev_holder::install_dev(const DeviceType &	type,
						const owner_id_t & 	owner,
						device ** 	p_dev_installed,
						embeddedRTControllerAddDeviceRequest dev_add_cmd)
{
	int error_val = SUCCESS;

	if(num_installed >= MAX_DEVICES)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"ERROR: max number of devices already installed, %d\n",
						MAX_DEVICES);
		error_val = FAILURE;
		goto ERR_EXIT;
	}

	// create device and fill in config data
	device_info[num_installed].p_device = new device(dev_add_cmd);
	device_info[num_installed].installer = owner;
	device_info[num_installed].when_installed = rtc_time_now();
	device_info[num_installed].when_started = device_info[num_installed].when_installed;
	device_info[num_installed].add_cmd = dev_add_cmd;

	// put entry in table for looking dev_info index by port number
	index_by_port_num[dev_add_cmd.port_number] = num_installed;

	// give caller a pointer to device to use when removing
	*p_dev_installed = device_info[num_installed].p_device;
	num_installed++;
	DLOG(SHOW,DEVICE_mID,"Just completed install_dev and count is %d\n",
					num_installed);
ERR_EXIT:
	return error_val;
};

//-----------------------------------------------------------------------------|
int
dev_holder::install_dev(embeddedRTControllerAddDeviceRequest & dev_add_cmd)
{
	int error_val = SUCCESS;
	int found = 0;
	int cmpval;

	if(num_installed >= MAX_DEVICES)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"ERROR: max number of devices already installed, %d\n",
						MAX_DEVICES);
		error_val = FAILURE;
		goto ERR_EXIT;
	}
	// check that no decice already exists for this device_id
	for(int i=0;i<num_installed;i++)
	{
		// find matching device if there
		if((cmpval = memcmp(&device_info[i].add_cmd.device_id,
				&dev_add_cmd.device_id,RTC_DEVICE_ID_LENGTH))==0)
		{
			DLOG(WARN,DEV_HOLDER_mID,
				"ERROR: trying to add device that exists on port %ld\n",
						device_info[i].add_cmd.port_number);
			found = 1;
			break;
		}
		DLOG(SHOW,DEVICE_mID,"OK, didn't find device already installed\n");
	}
	if(found==0)
	{
		// create correct kind of device and fill in config data
		DLOG(DIAG2,DEV_HOLDER_mID,"create new device of type %d\n",
						dev_add_cmd.type);
		switch(dev_add_cmd.type)
		{
		case VTR_DEVICE:
			DLOG(SHOW,DEVICE_mID,"Calling NewCorrectRecorderDevice\n");
			device_info[num_installed].p_device 
					= NewCorrectRecorderDevice(dev_add_cmd);
			break;
		case VIDEO_SERVER_DEVICE:
			device_info[num_installed].p_device 
					= NewCorrectVideoServerDevice(dev_add_cmd);
			break;
		case KEY_GENERATOR_DEVICE:
			device_info[num_installed].p_device 
					= NewCorrectVideoKeyerDevice(dev_add_cmd);
			break;
		case AUDIO_CART_DEVICE:
		case CG_DEVICE:
		case ROUTER_DEVICE:
			device_info[num_installed].p_device 
					= NewCorrectVideoRouterDevice(dev_add_cmd);
			break;
		case UNKNOWN_DEVICE:
		default:
			break;
		}
		device_info[num_installed].when_installed = rtc_time_now();
		device_info[num_installed].when_started 
				= device_info[num_installed].when_installed;
		device_info[num_installed].add_cmd = dev_add_cmd;

		DLOG(DIAG4,DEV_HOLDER_mID,
			"dev %d add block:\n",num_installed);
		hex_byte_dump_to_file(logfile,(char*)&dev_add_cmd,
						sizeof(embeddedRTControllerAddDeviceRequest));
	
		// put entry in table for looking up dev_info index by port number
		index_by_port_num[dev_add_cmd.port_number] = num_installed;

		device_info[num_installed].p_device->set_device_id(
				dev_add_cmd.device_id);
	
		num_installed++;
		char did_str[32];
		w2char_string_to_char(dev_add_cmd.device_id,did_str,24);
		// DLOG(SHOW,DEV_HOLDER_mID,
			DLOG(CONVFP,TEMP_mID,"%d devices in holder after add of %s\n",
					num_installed, did_str);
		// write all devices to dev_holder info file
		dev_save_hldr_info(info_filename);
	}
ERR_EXIT:
	DLOG(DIAG5,DEV_HOLDER_mID,
		"%s",found?"Found device\n":"Didn't find device\n");
	return error_val;
};

//-----------------------------------------------------------------------------|
int
dev_holder::remove_dev(	const device *	p_dev_to_rem,
						const owner_id_t &	owner)
{
	int error_val = SUCCESS;
	for(int i=0;i<num_installed;i++)
	{
		// find matching device and move everything down
		if(device_info[i].p_device == p_dev_to_rem)
		{
			// remove port number index lookup
			index_by_port_num[device_info[i].add_cmd.port_number] = -1;
			// slide down remaining entries
			memmove(&device_info[i],&device_info[i+1],\
							sizeof(dev_info)*(num_installed-i-1));
			--num_installed;
			DLOG(DIAG2,DEV_HOLDER_mID,
				"%d devices in holder after delete\n", num_installed);
		}
	}
	return error_val;
};

//-----------------------------------------------------------------------------|
int
dev_holder::remove_dev(embeddedRTControllerDeleteDeviceRequest & dev_del_cmd)
{
	int error_val = SUCCESS;
	int i;
	char d_id[RTC_DEVICE_ID_LENGTH];
	if(num_installed > MAX_DEVICES)
	{
		// DLOG(CONVFP,TEMP_mID,"ERROR: num_devices is %d, exceeds max of %d\n",
		// 		MAX_DEVICES,num_installed);
		abort();
	}
	w2char_string_to_char((w2char_t *)&dev_del_cmd.device_id,d_id,
					RTC_DEVICE_ID_LENGTH);
	d_id[RTC_DEVICE_ID_LENGTH] = 0;
	//DLOG(CONVFP,TEMP_mID,"looking for device %s to delete\n",d_id);
	for(i=0;i<num_installed;i++)
	{
		// find matching device and move everything down
		if(memcmp(&device_info[i].add_cmd.device_id,
				&dev_del_cmd.device_id,RTC_DEVICE_ID_LENGTH)==0)
		{
			delete device_info[i].p_device;
			// remove port number index lookup
			index_by_port_num[device_info[i].add_cmd.port_number] = -1;
			// slide down remaining entries
			memmove(&device_info[i],&device_info[i+1],\
							sizeof(dev_info)*(num_installed-i-1));
			--num_installed;
			DLOG(DIAG2,DEV_HOLDER_mID,
				"%d devices in holder after delete\n", num_installed);
		}
		else
		{
			DLOG(WARN,DEV_HOLDER_mID,
				"Couldn't find device %s to delete\n",d_id);
		}
	}
	dev_save_hldr_info(info_filename);
	return error_val;
};

//-----------------------------------------------------------------------------|
int
dev_holder::get_device_for_id(const w2char_t* device_id, device** p_device)
{
	*p_device = NULL;
	if(num_installed == 0)
		return(FAILURE);
	for(int idi=0;idi<num_installed;idi++)
	{
		if(device_info[idi].p_device->get_device_id() == NULL)
		{
			DLOG(CONVFP,TEMP_mID,"WARNING, installed device has NULL device_id\n");
			abort();
		}
		if(!compare_w2char_strings(device_info[idi].p_device->get_device_id(), 
								(w2char_t*)device_id,RTC_DEVICE_ID_LENGTH));
		{
			char did_str[32];
			w2char_string_to_char((w2char_t*)device_id,did_str,
							RTC_DEVICE_ID_LENGTH);
			DLOG(CONVFP,TEMP_mID,"found device_id %s installed\n",did_str);
			*p_device = device_info[idi].p_device;
			break;
		}
	}
	return(p_device ? SUCCESS : FAILURE);
};

//-----------------------------------------------------------------------------|
int
dev_holder::reset_dev(	const device & 		dev_to_reset)
{
	int error_val = SUCCESS;

	return error_val;
};

//-----------------------------------------------------------------------------|
int
dev_holder::dev_save_hldr_info(const char * const	dev_hldr_filename)
{
	FILE* dhfile;
	int error_val = SUCCESS;
	size_t nWritten;

	if((dhfile=fopen((const char*)dev_hldr_filename,"w"))==NULL)
	{
		error_val = errno;
		DLOG(FATAL,DEV_HOLDER_mID,
			"ERROR: failed to open %s in dev_save_hldr_info, %d: %s\n",
						dev_hldr_filename,error_val,strerror(error_val));
		abort();
	}
	if(fwrite(&this->num_installed,sizeof(this->num_installed),1,dhfile)!=1)
	{
		DLOG(FATAL,DEV_HOLDER_mID,
			"ERROR: failed to write number of devices to info file\n");
		abort();
	}
	if((nWritten = fwrite(&device_info[0],sizeof(dev_info),
			this->num_installed,dhfile)) != this->num_installed)
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"ERROR: wrote %d hldr info records instead of %d\n",
				nWritten, this->num_installed);
		abort();	// for now, later stumble on -- TODO
	}
	else
	{
		DLOG(WARN,DEV_HOLDER_mID,
			"Wrote %d bytes to %s\n",
						nWritten*sizeof(dev_info),dev_hldr_filename);
	}
	fflush(dhfile);
	return error_val;	
};

//-----------------------------------------------------------------------------|
int
dev_holder::dev_get_hldr_info(const char * const 	dev_hldr_filename)
{
	FILE* dhfile;
	int error_val = SUCCESS;

	if((dhfile=fopen((const char*)dev_hldr_filename,"r"))==NULL)
	{
		int error_val = errno;
		DLOG(SHOW,DEV_HOLDER_mID,
			"ERROR: failed to open %s in dev_get_hldr_info, %d: %s\n",
						dev_hldr_filename,error_val,strerror(error_val));
		DLOG(SHOW,DEV_HOLDER_mID,
			"Proceeding with no devices at startup.\n");
	}
	else
	{
		unsigned short num_in_file;
		// read number of devices in file
		if(fread(&num_in_file,sizeof(unsigned short),1,dhfile)!=1)
		{
			DLOG(SHOW,DEV_HOLDER_mID,"ERROR: filed to read number of "
					"devices from beginning of dev_holder info file\n");
		}
		else
		{
			if(num_in_file > MAX_DEVICES)
			{
				DLOG(SHOW,DEV_HOLDER_mID,
					"ERROR: hldr info file contains %d records, "
								"max is %d\n",num_in_file,MAX_DEVICES);
				num_in_file = MAX_DEVICES;
			}
			else
			{
				DLOG(SHOW,DEV_HOLDER_mID,
								"Loading %d dev_info blocks from file\n");
				size_t num_read;
				dev_info * p_restored_device_info;
				if((p_restored_device_info=(dev_info *)malloc(
					sizeof(dev_info)*num_in_file))==NULL)
				{
					DLOG(FATAL,MALLOC_mID,"ERROR: failed to malloc %d bytes\n",
					sizeof(dev_info)*num_in_file);
					abort();
				}
				// read all of the device records
				if((num_read = fread(p_restored_device_info,sizeof(dev_info),
					num_in_file,dhfile)) != num_in_file)
				{
					DLOG(SHOW,DEV_HOLDER_mID,
					"ERROR: failed to read some devices from dev_holder "
					"info file\n");
				}
				for(unsigned short idev=0;idev<num_read;idev++)
				{
					// this makes new devices from the original add commands
					// but keeps the original add time

					DLOG(SHOW,DEV_HOLDER_mID,
						"Loading dev_info block %d\n",idev);
					// create the device and its threads
					dev_holder::install_dev(p_restored_device_info[idev].add_cmd);
					// restore original when_installed time
					device_info[num_installed-1].when_installed =
							p_restored_device_info[idev].when_installed;
				}
				free(p_restored_device_info);
				p_restored_device_info = NULL;
			}

		}
	}
	return error_val;
};
//-----------------------------------------------------------------------------|

int dev_holder::get_device_count()
{
	return num_installed;
};
//-----------------------------------------------------------------------------|
int dev_holder::handle_subscription_request(unsigned long appl_entity,
				const embeddedDevStatSubscriptionRequest & request)	
{
	int error_code = SUCCESS;
	bool found = false;
	// find device that matches device_id in request and make call
	// to subscribe or unsubscribe
	for(int idd = 0; idd < num_installed; idd++)
	{
		char str1[64];
		char str2[64];

		w2char_string_to_char(device_info[idd].p_device->get_device_id(),
			str1,RTC_DEVICE_ID_LENGTH);
		w2char_string_to_char((w2char_t*)request.device_id,
			str2,RTC_DEVICE_ID_LENGTH);

		DLOG(DIAG3,STAT_SUBSCR_mID,"is this device the one? compare %s & %s\n",
			str1,str2);
		if(!compare_w2char_strings(device_info[idd].p_device->get_device_id(),
				(w2char_t*)request.device_id,RTC_DEVICE_ID_LENGTH))
		{
			found = true;
			DLOG(DIAG3,STAT_SUBSCR_mID,"this one matches\n");
			// this one matches, either subscribe or unsubscribe
			if(request.subscribe)
			{
				DLOG(DIAG3,STAT_SUBSCR_mID,"add subscriber at line %d\n",
								__LINE__);
				if(device_info[idd].p_device->add_subscriber(
							appl_entity)==FAILURE)
				{
					char did_str[RTC_DEVICE_ID_LENGTH];
					w2char_string_to_char((w2char_t*)request.device_id,did_str,
						RTC_DEVICE_ID_LENGTH);
					DLOG(WARN,STAT_SUBSCR_mID,"Failed to add subscriber %ld for %s\n",
						appl_entity, did_str);
					error_code = FAILURE;
				}
				else
					DLOG(DIAG3,STAT_SUBSCR_mID,
							"got success from add_subscriber\n");
			}
			else
			{
				if(device_info[idd].p_device->remove_subscriber(
							appl_entity)==FAILURE)
				{
					char did_str[RTC_DEVICE_ID_LENGTH];
					w2char_string_to_char((w2char_t*)request.device_id,did_str,
						RTC_DEVICE_ID_LENGTH);
					DLOG(WARN,STAT_SUBSCR_mID,"Failed to remove subscriber %ld for %s\n",
						appl_entity, did_str);
					error_code = FAILURE;
				}
				else
					DLOG(DIAG3,STAT_SUBSCR_mID,
							"got success from remove_subscriber\n");
			}
		}
	}
	if(found==false)
		error_code = FAILURE;
	return error_code;
}

//-----------------------------------------------------------------------------|
int dev_holder::publish_all_devices_status_reports()
{
	int error_code = SUCCESS;
	int iid;
	// go through list of installed devices and get status for each
	// that has any subscribers
	DLOG(DIAG4,STAT_SUBSCR_mID,"Publish status reports for %d devices\n",
					num_installed);
	for(iid=0;iid<num_installed;iid++)
	{
		if(device_info[iid].p_device->publish_device_status_reports() 
						!= SUCCESS)
		{
			char did_str[RTC_DEVICE_ID_LENGTH];
			w2char_string_to_char(
						(w2char_t*)device_info[iid].p_device->get_device_id(),
							did_str, RTC_DEVICE_ID_LENGTH);
			error_code = FAILURE;
			DLOG(CONVFP,TEMP_mID,
				"Failed to publish subscriber device status report for %s\n",
				did_str);
				
		}
	}
	return error_code;
};


//-----------------------------------------------------------------------------|

void
dev_holder::fill_all_device_response(void * buf, size_t len)
{
	for(int i=0;i<num_installed;i++)
	{
		int index;
		char * dest;
		
		index = i*sizeof(Device);
		dest = (char*)buf;

		if((index + sizeof(Device))>len)
			abort();

		memmove(&dest[index], &device_info[i].add_cmd, sizeof(Device));
		DLOG(DIAG2,DEV_HOLDER_mID,
			"dev %d status block:\n",i);
		hex_byte_dump_to_file(logfile,&dest[index], sizeof(Device));

	}

};

