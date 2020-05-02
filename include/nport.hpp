// FILE NAME: NPort.h                                         
//                                                              
// DESCRIPTION:                                                 
//   This file contains the declaration(s) of the class(es):    
//   NPort - Portable Comm Port class.                        
//      Objects of this class represent "NPorts" that 
//      applications can use to access Communications Ports. 
//   All of the instances of Port share a single static semaphore to
//     prevent access to critical sections.  The semaphore is
//     created when the DLL is loaded.                                                                  *
//
//  Copyright 2020 embeddedKeith
// -*^*-
//---------------------------------------------------------------------
#ifndef NPort_H
#define NPort_H

#include "osinc.hpp"
#include "nstring.hpp"
#include "nreslock.hpp"
#include <stdio.h>

class NPort 
{
	friend std::ostream& operator << ( std::ostream& os, const NPort& rhs );

	public:
		enum Parity {NoParity = 0, OddParity, EvenParity,MarkParity,
					 SpaceParity};
		enum StopBits {OneStop = 0, OnePlusStop, TwoStop};
		enum DataBits {Data4Bits = 4, Data5Bits, Data6Bits, Data7Bits, Data8Bits};
		enum Baud {Baud110 = 110, Baud300 = 300, Baud600 = 600,
				   Baud1200 = 1200, Baud2400 = 2400, Baud4800 = 4800,
				   Baud9600 = 9600, Baud14400 = 14440, Baud19200 = 19200,
				   Baud38400 = 38400, Baud56000 = 56000, Baud57600 = 57600,
				   Baud115200 = 115200};
		static bool IsValidName( const NString& portname );

	public:
		NPort
		( 
			const NString strPort, 
			Baud enBaud = Baud38400, 
			DataBits enBits = Data8Bits,
			Parity enParity= OddParity,
			StopBits enStop = OneStop
		);
		virtual ~NPort();
		void SignalBreak();
		bool Open();
		bool Close();
		bool Read(void*pvBuffer, INT iBufLen, INT &iLenRead,
					 bool bLock = true);
		bool Write(void* pvBuffer, INT iLen, INT &iLenWritteni,
					  bool bLock = true);

		bool SetBaud(Baud enBuad);
		bool SetParity(Parity enParity);
		bool SetStop(StopBits enStop);
		bool SetBits(DataBits enDataBits);
		bool SetRTS(bool RTSON);
		bool SetDTR(bool DTRON);
		bool SetReadTimeout(DWORD dwMsRead);
		bool SetWriteTimeout(DWORD dwMsWrite);
		DWORD GetReadBufferCount();
		bool PurgeRead();
		DataBits GetDataBits() const;

	protected:
		NPrivateResource portLock;
		int fd;
		NString strPortName;
		Baud eBaud;
		DataBits eBits;
		Parity eParity;
		StopBits eStop;
		DWORD iLastError;
		bool bPortOpen;
	
		bool CloseNoLock();
		bool SetReadTimeoutNoLock(DWORD dwMsRead);

	private:
		NPort( const NPort& copy );
		NPort& operator = ( const NPort& rhs );
		

};
typedef NPort* PNPort;

#endif

