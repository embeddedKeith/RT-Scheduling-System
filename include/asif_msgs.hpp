#ifndef _ASIF_MSGS_H_
#define _ASIF_MSGS_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	asif_msgs
//
//	Description:	
//
//	FileName:	asif_msgs.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       09/05/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include "asif_prototcol.hpp"
#include "rt_mqueue.hpp"

typedef
enum
{
	hand_shake = 0,
	request_system_status,
	request_all_devices_status,
	request_one_device_status,
	request_timecode,
	request_all_event_brief,
	request_one_event,
	num_asif_msg_types
} asif_msg_types;

typedef
struct
{
	mq_msg_header						header;
} request_all_devices_status_msg;

#endif	// _ASIF_MSGS_H_

