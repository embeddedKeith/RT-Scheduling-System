#ifndef _DEV_HOLDER_H_
#define _DEV_HOLDER_H_
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
//	FileName:	dev_holder.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/04/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#include "asif_protocol.hpp"
// #include "sched_mgr.hpp"
#include "device.hpp"

const unsigned short	MAX_DEVICES = 32;
const unsigned short	INFO_FILENAME_LENGTH = 16;

struct dev_info
{
	device*								p_device;
	port_types							port_type;
	unsigned short						port_num;
	owner_id_t							installer;
	rtc_time							when_installed;
	rtc_time							when_started;
	unsigned long						subscribers[MAX_SUBSCRIBERS];
	unsigned int						num_subscribers;
	embeddedRTControllerAddDeviceRequest		add_cmd;
};

//-----------------------------------------------------------------------------|
// dev_holder class definition
//
class dev_holder 
{
	public:
		dev_holder(		const char* const 		dev_info_filename);
		~dev_holder();

		int
		install_dev(	const DeviceType &	type,
						const owner_id_t & 	owner,
						device ** 	p_dev_installed,
						embeddedRTControllerAddDeviceRequest dev_add_cmd);
		int
		install_dev(	embeddedRTControllerAddDeviceRequest & dev_add_cmd);
		int
		remove_dev(		const device *	p_dev_to_rem,
						const owner_id_t &	owner);
		int
		remove_dev(		embeddedRTControllerDeleteDeviceRequest & dev_delete_cmd);
		int
		reset_dev(		const device & 			dev_to_reset);

		int
		get_device_for_id(const w2char_t*	p_device_id, device** p_device);

		int
		get_device_count();

		void
		fill_all_device_response(void * buf, size_t len);

		int handle_subscription_request( unsigned long appl_entity,
						const embeddedDevStatSubscriptionRequest & request);

		int dev_holder::publish_all_devices_status_reports();

	protected:

		dev_info		device_info[MAX_DEVICES];
		int  			index_by_port_num[MAX_DEVICES];
		unsigned short	num_installed;
		char			info_filename[INFO_FILENAME_LENGTH];
		rtc_time		when_started;

	private:
		int
		dev_save_hldr_info(	const char * const	dev_info_filename);
		int
		dev_get_hldr_info(	const char * const	dev_info_filename);

};



#endif	// _DEV_HOLDER_H_
