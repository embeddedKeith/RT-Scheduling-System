#ifndef _DEVICE_H_
#define _DEVICE_H_
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
//	FileName:	device.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/04/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include "osinc.hpp"
#include "rt_mqueue.hpp"
#include "nstring.hpp"
#include "w2char.hpp"
#include "dev_port.hpp"

const unsigned short	MAX_SUBSCRIBERS = 16;

#if 0 // TODO use DeviceType in gendef.hpp instead
enum
device_type
{
	dt_unknown = 0,
	dt_recorder,
	dt_player,
	dt_router,
	dt_switcher,
	dt_keyer,
	dt_audiocart,
	dt_simple_gpo,
	dt_custom,
	num_dt_vals
};
#endif

enum
device_status
{
	ds_unknown = 0,
	ds_offline,
	ds_ready,
	ds_standby,
	ds_recovering,
	ds_dead,
	num_ds_vals
};

enum port_types
{
	PT_UNKNOWN_PORT_TYPE = 0,
	PT_SERIAL_PORT,
	PT_LAN_PORT,
	PT_LAN_SERIAL_HUB_PORT,
	NUM_PT_VALS
};

struct
device_response
{
	unsigned int		type;
	unsigned long		flags;
	unsigned long		data_size;
	char 				data;
};

//-----------------------------------------------------------------------------|
// device class definition
//
class device 
{
	public:
		device();
		device(	const embeddedRTControllerAddDeviceRequest & add_cmd);
		virtual ~device();

		int add_subscriber(const unsigned long & appl_entity);
		int remove_subscriber(const unsigned long & appl_entity);
		int send_command(unsigned long cmd_type,void * cmd_buf,size_t cmd_size, 
				device_response ** p_dev_resp);
		int publish_device_status_reports();
		w2char_t* get_device_id();
		void set_device_id(const RTC_DEVICE_ID& device_id_val);

	protected:
		DeviceType		type;
		device_status	status;
		RTC_DEVICE_ID	device_id;
		unsigned long	subscribers[MAX_SUBSCRIBERS];
		unsigned int	num_subscribers;
};
	
// list of mqueues used in dev_mgr, these names are a local nomenclature
// (local to the dev_mgr process)
//
enum
en_dev_mqueues
{
	DMMQ_TO_MSG = 0,
	DMMQ_TO_ASIF,
	DMMQ_FROM_ASIF,
	DMMQ_TO_SCHED,
	DMMQ_FROM_SCHED,
	DMMQ_TO_STAT,
	DMMQ_FROM_STAT,
	NUM_DMMQ_VALS
};

//-----------------------------------------------------------------------------|

#endif	// _DEVICE_H_
