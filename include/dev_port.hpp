#ifndef _DEV_PORT_H_
#define _DEV_PORT_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	dev_port
//
//	Description:	
//
//	FileName:	dev_port.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/04/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#include <pthread.h>

enum Parity 
{
	NoParity = 0, 
	OddParity, 
	EvenParity,
	MarkParity,
	SpaceParity,
	NumParityVals
};
enum StopBits 
{
	OneStop = 0, 
	OnePlusStop, 
	TwoStop,
	NumStopBitsVals
};
enum DataBits 
{
	Data4Bits = 4, 
	Data5Bits, 
	Data6Bits, 
	Data7Bits, 
	Data8Bits,
	NumDataBitsVals
};
enum Baud 
{
	Baud110 = 110, 
	Baud300 = 300, 
	Baud600 = 600,
	Baud1200 = 1200, 
	Baud2400 = 2400, 
	Baud4800 = 4800,
	Baud9600 = 9600, 
	Baud14400 = 14440, 
	Baud19200 = 19200,
	Baud38400 = 38400, 
	Baud56000 = 56000, 
	Baud57600 = 57600,
	Baud115200 = 115200,
	NumBaudVals = 13
};

enum Protocol
{
	Unknown = 0,
	BVW75,
	DVW500,
	VDCP,
	SMPTE,
	NumProtocolVals
};

enum Mode
{
	RS232 = 0,
	RS422,
	NumModeVals
};

struct port_config
{
	Mode			mode;
	Baud			baud;
	Parity 			parity;
	StopBits		stop_bits;
	DataBits		data_bits;
	Protocol		protocol;
};


//-----------------------------------------------------------------------------|
// dev_port class definition
//
class dev_port
{
	protected:
		pthread_mutex_t	port_lock;
		port_config		config;
		unsigned short	port_number;

	public:
		dev_port(const port_config & config, unsigned short port_num);
		~dev_port();

		int
		write_to_port(char* buf, size_t len);
		size_t
		read_from_port(char* buf, size_t len);

};
#endif	// _DEV_PORT_H_
