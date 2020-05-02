#ifdef KRD
#define LOG_FILE "/tmp/krd_dev_mgr.log"
#else
#define LOG_FILE "/tmp/dev_mgr.log"
#endif
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, Keith DeWald, All rights reserved.
//-----------------------------------------------------------------------------|
//
//	Module:	checkit
//
//	Description:	
//
//	FileName:	checkit.cpp
//
//	Originator:	Keith DeWald
//-----------------------------------------------------------------------------|
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/dcmd_chr.h>
#include <unistd.h>
#include <devctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <strings.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <assert.h>
#include "../include/osinc.hpp"
#include "nstring.hpp"
#include "nov_log.hpp"
#include "w2char.hpp"
#include "embeddedSerialRecordDeviceLog.hpp"
#include "embeddedRecordDeviceTable.hpp"
#include "embeddedSonyRecorder.hpp"
#include "nport.hpp"

char mod_chars[4];
//-----------------------------------------------------------------------------|
// MAIN
//

int
main()
{
	embeddedRecordDeviceTable::Allocate();
	embeddedSerialRecordDeviceLog::Allocate();

	NString a("ser1");
	NPort n(a);
	NString b("ser2");
	NPort m(b);
	const int NUM = 26;
	char rbuf[NUM];
	char wbuf[NUM];

	n.open();
	m.open();

	for(int i=0;i<1000'i++)
	{
		loop = i % NUM
		if(n.write((void*)(&rbuf[loop]),1,rlen,true)==true)
			fprintf(stderr,"%d: rlen %d, read %c from port-n\n",
							loop, rlen, rbuf[loop]);
		else
			fprintf(stderr,"read failed with rlen %d\n",rlen);
		rbuf[loop]=0;
		if(n.read((void*)(&rbuf[loop]),1,rlen,true)==true)
			fprintf(stderr,"%d: rlen %d, read %c from port-n\n",
							loop, rlen, rbuf[loop]);
		else
			fprintf(stderr,"read failed with rlen %d\n",rlen);
		sleep(1);
	}
	embeddedSonyRecorder sony(a,1,a);
	sony.SetActive(true);
	sleep(60);
	NString b(a);
	DLOG(SHOW,TEMP_mID,"a is %s, b is %s\n",(const char *)a,(const char *)b);
	n.close();
	m.close();

	return(0);
}
