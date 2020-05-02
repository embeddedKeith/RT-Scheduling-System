//  This file contains generic definitions for sizes and types
//   used within the broadcast automation software.
//  All sizes are presumed to be "real" characters.  Character
//  buffers must be one character larger.
//
// Copyright 2020 embeddedKeith
// -*^*-
//-------------------------------------------------------------------------

#ifndef _GENERALDEF_
#define _GENERALDEF_

#define test_virtual virtual

#define NOVUS_SCHEMA "NOVUSSCHEMA"

#define MAXMEDIASEGMENT 256

#define NAMED_DEVICE_LENGTH 24   // Number character in device name
#define HOSTNAME_LENGTH 16       // Number of characters in host name

#define OPERATOR_NAME_LENGTH 80
#define OPERATOR_ID_LENGTH 8

#define CONTROLLER_NAME_LENGTH 80
#define SCHEDULE_NAME_LENGTH 16

#define HOUSESIZE 32             // Maximum size of house id
#define TITLESIZE 128
#define DESCRIPTIONSIZE 256
#define GROUPSIZE 64
#define OPRSIZE 16
#define SOURCESIZE 64
#define LOGOIDSIZE 64

#define PORT_NAME_LENGTH 16
#define DATASET_NAME_LENGTH 24

#define PLAYCOMMENTSIZE 256

#define	VIDEO_DEVICE_NAME_LENGTH 33
#define AUDIO_DEVICE_NAME_LENGTH  33
#define DESCRIPTION_LENGTH  65
#define HOUSE_NUMBER_LENGTH  33
#define TITLE_LENGTH  65
#define AGENCY_CODE_LENGTH  64
#define DESCREPANCY_LINE_COUNT  5
#define DESCREPANCY_LENGTH_LENGTH  81
#define SYSTEM_DESCREPANCY_LENGTH  81
#define CG_PAGE_LENGTH  81
#define CG_DATA_LENGTH 81
#define AUDIO_DATA_LENGTH  33
#define AUDIO_DEVICE_KEY_LENGTH  33
#define	KEY_VIDEO 65
#define	ERROR_MESSAGE_LENGTH 257
#define	COMMENT_DATA_LENGTH 60
#define	NOLA_LENGTH 16
#define	KEY_STATUS_LENGTH 8
#define	MYERS_EPISODE_LENGTH 8

#define	NOLA_CODE_LENGTH 32
#define	EPISODE_LENGTH 32

#define USERDATACLIP   64
#define USERDATATITLE  24
#define USERDATADESC   64
#define USERDATAGROUP  64
#define USERDATAAGENCY 64

#define NODENAMESIZE 15
#define MAXSERVER 10
#define	MAX_EXTENSIONS 50    // Maximum number of child events per events
#define	KEY_DATA_LENGTH 500

typedef char RTC_TIME[16];   // YYYYMMDDHHMMSSss  (ss = milliseconds) 

// DO NOT CHANGE ORDER OR VALUE

enum MediaType 
{
	VTape = 1, 
	VServer, 
	VArchive
};
   
enum DeviceType
{
	UNKNOWN_DEVICE = 0,
	VTR_DEVICE,					// Video tape recorder
	VIDEO_SERVER_DEVICE,        // Video server
	KEY_GENERATOR_DEVICE,		// Key Generator
	AUDIO_CART_DEVICE,			// Audio Cartridge Device
	CG_DEVICE,					// CG Generator
	ROUTER_DEVICE,              // Router
	NUMBER_OF_DEVICE_TYPES
};

enum TimeType {CTNDF = 1, CTDF, TCNDF, TCDF};
    
enum BStandard {NTSC = 1, PAL, SECAM, ATSC};
    
enum MaterialType {UNKNOWN = 0, PGM, CM, PSA, PROMO, ANNC, UDW, ID};
	
enum ResolutionType {NTSC525 = 1, PAL625, i1120Lines, i1080Lines, i780Lines, i480Lines};

enum ExtensionType {Event = 0, KeyExtension, VOCAExtension, CGExtension, EVENTerror, EVENTdevice, EVENTcomment};

enum ScheduleEventType {UnknownEvent = 0, PGMEvent, CMEvent, IDEvent, PSAEvent, UNDEvent};

enum AnncType {Pgm = 1, Cm, Net, News};

enum EventMode
{ 
	CUED_EVENT = 1, 
	DURATION_EVENT,
	ABSOLUTE_EVENT,
};

enum ProgSource{progLIVE = 1, progRECORDED, progNETWORK, progOTHER, progUNKNOWN};

enum EventRunStatus{ EVENTDidNotAir = 1, EVENTAired, EVENTAdded, EVENTDeleted, EVENTAiredTechDiff, EVENTDurDiscrepency, 
	 EVENTHouseChanged, EVENTNoInfoStatus, EVENTUnused};

enum FCCType{FCCAdjacency =1, FCCCommAnnc, FCCColorBars, FCCEBSTest, FCCFiller, FCCStatId,
	 FCCPledgeBrk, FCCPromo, FCCProgSponsor, FCCPsa, FCCSupImposed, FCCVo, FCCUnderWriter, FCCProgSeg};

enum EventTransition {TranCut = 1, TranMix, TranFade, TranUp, TranCutFade,
                              TranFadeCut, TranWipe};
enum EventRanCode {NotRun = 0, Ran, RanDeadRoll};

enum ErrorSeverity 
{  
	UNKNOWN_SEVERITY = 0,
	FATAL_SEVERITY, 
	WARNING_SEVERITY, 
	INFORMATION_SEVERITY,
	NUMBER_SEVERITY_VALS
};
 
enum ScheduleError 
{ 
	UNKNOWN_SCHEDULE_ERROR = 0,
	NO_VIDEO_SOURCE_IDENTIFIED_SCHEDULE_ERROR,
	VIDEO_SOURCE_NOT_AVAILABLE,
	VIDEO_MEDIA_NOT_LOADED_SCHEDULE_ERROR,
	EVENT_DOES_NOT_MATCH_CRITERIA,
	ROUTER_NOT_AVAILABLE,
	SEGMENT_NUMBER_BAD,
	VIDEO_SOURCE_NOT_POSITIONED,
	NUMBER_SCHEDULE_ERROR_VALS
};

enum ScheduleMessageType
{
    INSERT_SCHD_EVENT = 3, 
    REPLACE_SCHD_EVENT,
    DELETE_SCHD_EVENT, 
    ERASE_MASTER_SCHEDULE,
	REFRESH_VIEW,
};


//---------------------------------------------------------------------------
//  EventExecutionStatus is used to identify the state of an event in the 
//   master schedule internal control state chart.  Each device implements
//   a control mechanism in terms of state transitions.  These are the 
//   generic state representations.  There are really only 8 "healthy" 
//   conditions

enum EventExecutionStatus
{
    UNKNOWN_EXECUTION_STATUS = 0,    // Unused value
    NO_SOURCE_IDENTIFIED,            // Schedule does not identify physical source device
    SOURCE_DEVICE_NOT_AVAILABLE,     // Unable to access the data source device
    WAITING_FOR_SOURCE_LOAD,         // Source device does not show data loaded
    WAITING_FOR_SOURCE_POSITION,     // Source device loaded, not positioned for play
    WAITING_FOR_PREROLL,             // Source device position, ready for pre-roll
    WAITING_TO_AIR,                  // Source device playing, output not switched
    ON_AIR,                          // Source device playing to output destination
    FINISHED_EXECUTION,              // Source device finished
	NUMBER_EXECUTION_STATUS_VALS
};


enum CueListReportField
{
	CUE_LIST_FIRST_FIELD = 1,
	CUE_LIST_HOUSE_HUMBER = 1,
	CUE_LIST_MEDIA_TYPE,
	CUE_LIST_PROGRAM_TITLE,
	CUE_LIST_PROGRAM_DESCRIPTION,
	CUE_LIST_PROGRAM_SOURCE,
	CUE_LIST_PROGRAM_AGENCY,
	CUE_LIST_PROGRAM_ARCHIVE_GROUP,
	CUE_LIST_PROGRAM_NOLA_CODE,
	CUE_LIST_PROGRAM_ARCHIVE_RESOLUTION,
	CUE_LIST_PROGRAM_CREATION_TIME,
	CUE_LIST_PROGRAM_MODIFIED_TIME,
	CUE_LIST_PROGRAM_KILL_DATE,
	CUE_LIST_PROGRAM_REVIEW_DATE,
	CUE_LIST_PROGRAM_OBSERVE_KILL_DATE,
	CUE_LIST_PROGRAM_MATERIAL_TYPE,
	CUE_LIST_PROGRAM_TIME_TYPE,
	CUE_LIST_PROGRAM_DUB_OPERATOR,
	CUE_LIST_PROGRAM_ENTRY_OPERATOR,
	CUE_LIST_PROGRAM_MARK_OPERATOR,
	CUE_LIST_LAST_FIELD = CUE_LIST_PROGRAM_MARK_OPERATOR
};
  	
enum DubListReportField
{
	DUB_LIST_FIRST_FIELD = 1,
	DUB_LIST_HOUSE_NUMBER = 1,
	DUB_LIST_PROGRAM_TITLE,
    DUB_LIST_PROGRAM_DESCRIPTION,
    DUB_LIST_PROGRAM_EPISODE,
    DUB_LIST_PROGRAM_CUT_NUMBER,
    DUB_LIST_PROGRAM_NOLA_CODE,
    DUB_LIST_PROGRAM_FILE_TAPE_MATERIAL,
	DUB_LIST_PROGRAM_KILL_DATE,
	DUB_LIST_PROGRAM_REVIEW_DATE,
	DUB_LIST_PROGRAM_OBSERVE_KILL_DATE,
    DUB_LIST_PROGRAM_PLAY_DATE,
    DUB_LIST_PROGRAM_ARCHIVE_GROUP,
	DUB_LIST_PROGRAM_LENGTH,
    DUB_LIST_PROGRAM_STEREO,
    DUB_LIST_PROGRAM_SECONDARY_AUDIO,
    DUB_LIST_PROGRAM_CLOSED_CAPTION,
    DUB_LIST_PROGRAM_DESCRIPTIVE_VIDEO_SERVICE,
	DUB_LIST_PROGRAM_VOCA_START,
	DUB_LIST_PROGRAM_VOCA_LENGTH,
	DUB_LIST_LAST_FIELD = DUB_LIST_PROGRAM_VOCA_LENGTH
};

enum FormatValidation
{ 
	FORMAT_VALID,
	FORMAT_OUT_OF_RANGE,
	FORMAT_UNRECOGNIZABLE 
};

// Future video server / recorders are Leith, SeaChange. Initial versions are
//  just the HP(Pinnacle) and GVG (Tektronix) devices

enum RecorderType 
{
	UnknownServ = 0,
    HPVServ,
    TekVServ,
    NetVTR, 
    SonyVTR,
    AmpexVTR,
    DmyVS,
    NUMBER_OF_RECORDER_TYPES
};

enum KeyGeneratorType
{
	UnknownKGT = 0,
    KGTDev1,
	KGTDev2,
	KGTDev3,
	KGTDev4,
	KGTDevDummy,
    NUMBER_OF_KEY_GEN_TYPES
};

enum AudioCartType
{
	UnknownACT = 0,
	ACTDev1,
	ACTDev2,
	ACTDev3,
	ACTDev4,
	ACTDevDummy,
    NUMBER_OF_AUDIO_CART_TYPES
};
 
enum CGDeviceType
{
	UnknownCGD = 0,
	CGDDev1,
	CGDDev2,
	CGDDev3,
	CGDDev4,
	CGDDevDummy,
    NUMBER_OF_CG_DEVICE_TYPES
};

enum RouterDeviceType
{
	UNKNOWN_ROUTER = 0,
	NET_ROUTER,	      
	UTAH_ROUTER,	    
	HORZ_ROUTER,        
    BTS_ROUTER,	 
	SIERRA_ROUTER,	      
	GVG10X_ROUTER,	     
	GVG7000_ROUTER,	  
	DUMMY_ROUTER,
    NUMBER_OF_ROUTER_DEVICE_TYPES
};

enum BarCodeScannerType
{
	UNKNOWN_BARCODE_SCANNER = 0,
    SYMBOL_2100_BARCODE_SCANNER,
    NUBMER_OF_BARCODE_SCANNER_DEVICE_TYPES
};

enum EventStatusCode
{
	UNKNOWN_EVENT_STATUS_CODE = 0,
	ADDED_TO_SCHEDULE_STATUS,
	STARTED_STATUS,
	IN_PROGRESS_STATUS,
	COMPLETED_STATUS,
	ERROR_STATUS,
    NUMBER_OF_STATUS_CODES
};
  
enum DeviceEventType
{
	DEVICE_START_RECORD = 5,
    DEVICE_START_PLAY,
    DEVICE_STOP , 
    DEVICE_PAUSE, 
    DEVICE_JOG,
    DEVICE_SHUTTLE,
    DEVICE_STANDBY ,
    DEVICE_EJECT,
    DEVICE_START_KEY,
    DEVICE_START_CG,
    DEVICE_START_AUDIO,
    DEVICE_SET_ROUTER,
    DEVICE_SET_MIXER
};
#endif
