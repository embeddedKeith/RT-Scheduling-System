#ifndef _ASIF_PROTOCOL_H_
#define _ASIF_PROTOCOL_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	asif_protocol
//
//	Description:	
//
//  This file contains constants and structure definitions to match
//  the RTC Master scheduler protocol.
//
//  For MasterScheduler, the constant WINDOWS may be presumed to be defined.
//  otherwise, presume that it is being compiler from GNU on QNX.
//
//  The intent is for this header file to be accessible via C or C++.
//
//	FileName:	asif_protocol.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       08/15/2020     Copied from Brian's windows c++ header file
//
//
//-----------------------------------------------------------------------------|
#include "gendef.hpp"
#include "w2char.hpp"


typedef w2char_t *  owner_id_t;

// temp for RTC_TIMECODE


// some special definitions for fixed length character strings 
// which are defined in the protocol  document.
#define RTC_NTIME_LENGTH 17
typedef char     RTC_NTIME[RTC_NTIME_LENGTH];			// YYYYMMDDHHMMSSmmm
#define RTC_NTIMECODE_LENGTH 8
typedef char     RTC_NTIMECODE[RTC_NTIMECODE_LENGTH];  	// HHMMSSFF
#define RTC_INET_ADDR_LENGTH 16
typedef char     RTC_INET_ADDR[RTC_INET_ADDR_LENGTH]; 	// AAA.BBB.CCC.DDD
#define RTC_HOSTNAME_LENGTH 16
typedef w2char_t RTC_HOSTNAME[RTC_HOSTNAME_LENGTH]; 	// Unicode hostname
#define RTC_DEVICE_ID_LENGTH 24
typedef w2char_t RTC_DEVICE_ID[RTC_DEVICE_ID_LENGTH];
     
#define RTC_RT_CONTROLLER_STATUS_REQUEST  1024    
#define RTC_RT_CONTROLLER_STATUS_RESPONSE 1025
#define RTC_FILE_TRANSFER_TO_RT_REQUEST   1032
#define RTC_FILE_TRANSFER_TO_RT_RESPONSE  1033
#define RTC_ACKNOWLEDGEMENT_RESPONSE      1065
#define RTC_SHUTDOWN_REQUEST              1064
#define RTC_RESTART_REQUEST               1066
#define RTC_RT_CONTROLLER_CONFIG_REQUEST  1080
#define RTC_RT_CONTROLLER_CONFIG_RESPONSE 1081
#define RTC_RT_CONTROLLER_ADD_REQUEST     1100
#define RTC_RT_CONTROLLER_DELETE_REQUEST  1124
#define RTC_ADD_SCHEDULED_EVENT_REQUEST   1200
#define RTC_DROP_SCHEDULED_EVENT_REQUEST  1201
#define RTC_GET_SCHEDULED_EVENT_REQUEST	  1202
// #define RTC_EXEC_DEV_COMMAND_REQUEST		1210
// #define RTC_EXEC_DEV_COMMAND_RESPONSE  	1211
#define RTC_STATUS_RESPONSE               1280
#define RTC_DEVICE_STATUS_SUBSCRIPTION 	  1290
#define RTC_LOG_CONTROL_REQUEST			  1300


struct embeddedHeader
{
    RTC_HOSTNAME sender;   	// Hostname of the sending computer (Mater 
							// scheduler or R/T system
	unsigned long appl_entity; 	// application entity id, unique reqardless of
								// which computer or connection it is to/from
    unsigned long id;      	// Unique id associated with a message from a 
							// given host.
    unsigned long type;    	// Type of message being sent
    unsigned long size;    	// Number of bytes in body of message
};
//.............................................................................
// RTC_RT_CONTROLLER_STATUS_REQUEST  1024    
// RTC_RT_CONTROLLER_STATUS_RESPONSE 1025
struct embeddedRTControllerResponse
{     
    unsigned long id;                  	//  A unique id determined by the 
										// R/T controller.  This will be 
										// based on MAC address or some 
										// similar mechanism.     
    w2char_t time_code_status;         	// Time code status.  A tri-stated 
										// value: 
                                       	//       N means no time code card,
                                       	//       Y means a time code card,
                                       	//       F means an inoperative time 
										// 			code card
    unsigned long network_card_count;  	// Number of installed Ethernet cards.
    w2char_t hardware_desc[80];        	// Hardware identification.
    w2char_t software_desc[80];        	// Software identification
    unsigned long  major;              	// Software major version number.
    unsigned long  minor;              	// Software minor version number.
};

struct embeddedRTControllerResponseMsg
{
	embeddedHeader				header;
	embeddedRTControllerResponse response;
};
      
//.............................................................................
// RTC_FILE_TRANSFER_TO_RT_REQUEST   1032
struct embeddedFileTransferRequest
{
    w2char_t name [128];                // Logical name of the file.  May 
										// include '/' directory delimiter,
                                       	// but will not be presumed to be 
										// case-sensitive or contain 
                                       	// embedded blanks. 
    unsigned char data[1];             	// Data... message length Message 
										// size - 256.
};

struct embeddedFileTransferRequestMsg
{
	embeddedHeader				header;
	embeddedFileTransferRequest  request;
};
//.............................................................................
// RTC_FILE_TRANSFER_TO_RT_RESPONSE  1033
struct embeddedFileTransferResponse
{
     w2char_t success;    	// Y indicates data received, N indicates failure      
     w2char_t reason[256]; 	// Reason while transfer failed. Only used if 
	 						// transfer failed.   
     w2char_t resend;      	// Y means resend, N means do not attempt.  Only 
	 						// used if transfer failed.
};

struct embeddedFileTransferResponseMsg
{
	embeddedHeader				header;
	embeddedFileTransferResponse request;
};
//.............................................................................
struct VideoRecorder     // Serial        
{   
	unsigned long type;      // Sony, etc.  
	w2char_t	  record_only; // Y or N
	w2char_t	  play_only;   // Y or N
};   
struct VideoServer       
{
	unsigned long type;         // HP, Tek, 
	unsigned long unit_number;             
	unsigned long record_channel;             
	unsigned long play_channel;             
	unsigned long control_unit;                
	unsigned long record_only;	// true or false 
	unsigned long play_only;	// true or false
};  
#define MAX_ROUTER_LEVELS 32
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
struct Device  // This data is an array of variant records
{

	// The first common block of data is the serial or inet data and whether 
	// this device is the primary or backup controller.

	DeviceType		type ;         	// device type (struct selector)         
	RTC_DEVICE_ID 	device_id;     	// device id         
	w2char_t access;             	// S - Serial port, N network port, 
	w2char_t mode;               	// M - master, B backup access mode   
	w2char_t port_mode;          	// 2 - Rs232 , 4 -Rs422         
	w2char_t padding;
	RTC_INET_ADDR inet_addr;     	// Internet address         
	unsigned long port_number;   	// 1 - N         
	unsigned long baud_rate;     	// values:          
	unsigned long parity;        	// values:         
	unsigned long stop_bits;     	// values:         
	RTC_INET_ADDR alternate_addr;	// Internet address of backup (if master), 
									// master (if backup)
	union   
	{         
		struct VideoRecorder		vtr;   // Serial        
		struct VideoServer       	video_server;  
		struct VideoRouter 			video_router;
#if 0
		/*
		....
		struct  KeyGenerator         
		{         
		} 
		.
		struct  AudioCart         
		{         
		}
		......
		*/
#endif
	};
};
//.............................................................................
// RTC_RT_CONTROLLER_CONFIG_REQUEST  1080
// Simple Message, inherent from header content, no struct needed

//.............................................................................
// RTC_RT_CONTROLLER_CONFIG_RESPONSE 1081
struct embeddedRTConfigurationResponse
{   
    unsigned long 	record_size;   // Size in bytes of records found in data 
    struct Device	device[1];
};

struct embeddedRTConfigurationResponseMsg
{
	embeddedHeader					header;
	embeddedRTConfigurationResponse	response;
};


//.............................................................................
// RTC_RT_CONTROLLER_ADD_REQUEST     1100
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

        /*
       ....
        struct  AudioCard         
        {         
        }  

        struct KeyGenerator
        {         
        } 
        ...
        */
    };
};

struct embeddedRTControllerAddDeviceRequestMsg
{             
	embeddedHeader						header;
	embeddedRTControllerAddDeviceRequest	request;
};

//.............................................................................
// RTC_RT_CONTROLLER_DELETE_REQUEST  1124
struct embeddedRTControllerDeleteDeviceRequest
{    
    RTC_DEVICE_ID	device_id;                      // device id
};

struct embeddedRTControllerDeleteDeviceRequestMsg
{             
	embeddedHeader							header;
	embeddedRTControllerDeleteDeviceRequest	request;
};

//.............................................................................
struct StartRecordEvent         
{     
	w2char_t clip[128];        // clip id
};  

struct StandbyEvent
{
	long mode;
};


struct PlayEvent         
{     
	w2char_t clip[128];        // clip id
}; 
        
struct JogEvent         
{     
	long number_frames;        // +/- indicates forward/reverse # frames
}; 

struct ShuttleEvent         
{     
	long speed;                // +/- indicates speed forward/reverse
}; 

struct RouterEvent
{
	unsigned long input;
	unsigned long output;
};

struct StartKeyGeneratorEvent
{    
	unsigned long command;       // Command 
	unsigned char data[1];       // Variable length data
};

struct StartCGDataEvent
{    
	unsigned long command;       // Command 
	unsigned char data[1];       // Variable length data
};


struct AudioEvent         
{  
	unsigned long command;       // Command 
	unsigned char data[1];       // Variable length data
};

struct MixerEvent         
{  
	unsigned long output;       // Command 
	unsigned long level;        // Command 
};

union device_data
{      
	StartRecordEvent		record;  

	StandbyEvent			standby;

	PlayEvent				play; 
        
	JogEvent         		jog; 

	ShuttleEvent			shuttle; 

	RouterEvent				router;

	StartKeyGeneratorEvent  start_key;

	StartCGDataEvent		start_cg;

	AudioEvent				start_audio;

	MixerEvent				set_level;
};
//.............................................................................
// RTC_ADD_SCHEDULED_EVENT_REQUEST   1200

#define EXEC_IMMEDIATE	1
#define EXEC_SCHEDULED	2
struct embeddedAddScheduledEventRequest
{    
    unsigned long   event_id;         // Global event identifier - unique    
    RTC_DEVICE_ID	device_id;    // device id to be employed for event    
    unsigned long   event_type;       // struct selector    
                                      //  1:  START_RECORD
                                      //  2:  START_PLAY
                                      //  3:  STOP  -  no extra data
                                      //  4:  PAUSE  -  no extra data
                                      //  5:  JOG
                                      //  6:  SHUTTLE
                                      //  7:  STANDBY 
                                      //  8:  EJECT -  no extra data
									  //  9:  SETUP
                                      //  20. START_KEY
                                      //  30: START_CG
                                      //  40: START_AUDIO
                                      //  50: SET ROUTER
                                      //  60: SET MIXER
    unsigned long   immediate;        // 1 do it now, 2 do it at event time
    RTC_NTIMECODE   event_time;       // HHMMSSFF  (FF is frames)
    device_data 	dev_data;
};

#define NumEventTypes 100


struct embeddedAddScheduledEventRequestMsg
{
	embeddedHeader					header;
	embeddedAddScheduledEventRequest	request;
};

//.............................................................................
// RTC_DROP_SCHEDULED_EVENT_REQUEST  1201
struct embeddedDropScheduledEventRequest
{       
    unsigned long event_id;         // Global event identifier - unique                   
};

struct embeddedDropScheduledEventRequestMsg
{
	embeddedHeader						header;
	embeddedDropScheduledEventRequest	request;
};

//.............................................................................
// RTC_LOG_CONTROL_REQUEST  1201
struct embeddedLogControlRequest
{       
	char							module;
	char							priority;
	unsigned short					mod_id;
};

struct embeddedLogControlRequestMsg
{
	embeddedHeader						header;
	embeddedLogControlRequest			request;
};

//.............................................................................
// RTC_STATUS_RESPONSE               1280

struct
rtc_recorder_device_status
{
    RTC_NTIMECODE time_reported;       // Timecode of report
    RTC_NTIMECODE time_code;           // Timecode position from device
    RTC_NTIMECODE time_other;          // Tape time position from tape VTR
    char 		 tape_status;         // 1 Tape in, 2 Tape out tape device only
	char		 speed;				  // +/- 10
	char		 dev_ready;			  // head_up = 1, head_down = 2
	char		 remote;		  	  // 0 local, 1 remote
	char		 dev_talking;		  // 0 device not talking, 1 ok
	char		 transport_mode;	  // 1 jog, 2 shuttle, 3 var, 0 NA
    unsigned long error_code;         // For error only
    RTC_DEVICE_ID	device_id; // Device id with problem
    w2char_t error_text[80] ;         // Text of problem (error only)
} __attribute__ ((packed));

struct
rtc_router_device_status
{
	char		dev_ready;
};

struct
rtc_keyer_device_status
{
	char		dev_ready;
};

typedef union
{
	rtc_recorder_device_status	recorder_dev_status;
	rtc_router_device_status	router_dev_status;
	rtc_keyer_device_status		keyer_dev_status;
} rtc_device_status;

struct embeddedScheduledStatusResponse
{		
	unsigned long event_id;		 	// Original event id
	unsigned long condition;		// Type of status 
									// 1:	Added to schedule
									// 2:	Started 
									// 3:	In progress 
									// 4:	Completed	
									// 5:	Subscription report, no event
									// 6:	Subscribed device not installed
									// Error encountered
	rtc_device_status 	dev_status;	// status in form client needs
};

struct embeddedScheduledStatusResponseMsg
{    
	embeddedHeader						header;
	embeddedScheduledStatusResponse		response;
};
//.............................................................................
// RTC_DEVICE_STATUS_SUBSCRIPTION	1290
struct embeddedDevStatSubscriptionRequest
{       
	RTC_DEVICE_ID					device_id;
	bool							subscribe; 	// false is unsubscribe
											 	// true is subscribe
};

struct embeddedDevStatSubscriptionRequestMsg
{
	embeddedHeader						header;
	embeddedDevStatSubscriptionRequest 	request;
};

//.............................................................................

#endif	// _ASIF_PROTOCOL_H_

