#ifdef KRD
#define LOG_FILE "/tmp/krd_tc_drvr.log"
#else
#define LOG_FILE "/tmp/tc_drvr.log"
#endif
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	tc_drvr
//
//	Description:	Interface to Adrienne PCI-VLTC RDR card.  For now polls
// 					every 3 ms (good mix of expediency and efficiency) to
//					see when tc changed bit toggles.  Will make it use the
//					interrupt later.  Sends message with timecode value and
//					actual elapsed milliseconds to sched_mgr so it can run
//					events who's time has come, and send status message to
//					connected Master Schedulers once per second.
//
//	FileName:	tc_drvr.cpp
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/11/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <hw/inout.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include "nov_log.hpp"
#include "rt_mqueue.hpp"
#include "w2char.hpp"
#include "tc_drvr.hpp"

FILE* logfile = stderr;

static int stop_requested = 0;
static int still_trying = 1;

static int count=0;

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
char stderr_str[MAX_STDLOG_STR_LENGTH];
char mod_chars[4] = "DE";

// table to allow looking up outgoing mqueue index by incoming user id
const en_tcdrvr_mqueues smq_from_sender_id[num_mq_user_vals] =
{
	TCDMQ_TO_MSG,
	TCDMQ_TO_SCHED,
	TCDMQ_TO_STAT,
	NUM_TCDMQ_VALS
};

// this array keeps track of just the mqueues sched_mgr uses out of
// all the mqueues in the system
//
mod_mqueue mq[NUM_TCDMQ_VALS] =
{
	// which-mqueue         read/write		blk/non-blk		rt_mqueue pointer
	{ WMQ_ALL_TO_MSG, 		rw_write_only, 	blk_non_blocking, NULL},
	{ WMQ_TCD_TO_SCHED, 	rw_write_only, 	blk_non_blocking, NULL},
	{ WMQ_TCD_TO_STAT,		rw_write_only,	blk_non_blocking, NULL}
};


//-----------------------------------------------------------------------------|
// bcd_tc member functions
//
bcd_tc::bcd_tc(int hours = 0, int mins = 0, int secs = 0, int frames = 0)
{
	tens_hours = hours/10;
	ones_hours = hours%10;
	tens_mins = mins/10;
	ones_mins = mins%10;
	tens_secs = secs/10;
	ones_secs = secs%10;
	tens_frames = frames/10;
	ones_frames = frames%10;
};

bcd_tc::~bcd_tc()
{
};

//-----------------------------------------------------------------------------|
// hw_8bit_reg member functions
//
hw_8bit_reg::hw_8bit_reg()
{
};

hw_8bit_reg::hw_8bit_reg(uint32_t addr_offset, rw_mode rwm, char* reg_str)
{
	reg_offset = addr_offset;
	rw_access = rwm;
	strncpy(reg_name,reg_str,REG_NAME_LENGTH);
};

int
hw_8bit_reg::reg_init()
{
	if((reg_port = mmap_device_io(1,(uint64_t)(TC_BASE_ADDR + reg_offset)))
					==(uintptr_t)MAP_FAILED)
	{
		int error_val = errno;
		fprintf(logfile,
				"ERROR: mmap_device_io, len %d, addr %lld with errno %d, %s\n",
						1,(uint64_t)(TC_BASE_ADDR + reg_offset),
						error_val, strerror(error_val));
		abort();
	}
	else
	{
		//fprintf(logfile,"initialized %s, reg_port is 0x%0x\n",reg_name,
						//(unsigned int)reg_port);
	}
	// if((rw_access == RO) || (rw_access == RW))
	// 	reg_rval = in8(reg_port);

	return 1;
};

uint8_t
hw_8bit_reg::reg_read()
{
	return reg_rval;
};

uint8_t
hw_8bit_reg::reg_immed_read()
{
	reg_rval = in8(reg_port);
	return reg_rval;
};

void
hw_8bit_reg::reg_write(const uint8_t & val)
{
	reg_wval = val;
};

void
hw_8bit_reg::reg_immed_write(const uint8_t & val)
{
	reg_wval = val;
	out8(reg_port, val);
};

void
hw_8bit_reg::reg_fprint(FILE* pfile)
{
	fprintf(pfile,"0x%02x: 0x%02x %s\n",(unsigned int)reg_offset, 
					(unsigned int)reg_rval, reg_name);
};


//-----------------------------------------------------------------------------|
// tc_drvr member functions
//

tc_drvr::tc_drvr()
{
	int itc;

	//fprintf(logfile,"NUM_TC_CARD_REGS is %d\n",NUM_TC_CARD_REGS);


	// create and initialize all of the timecode card registers
	for(itc=0;itc<NUM_TC_CARD_REGS;itc++)
	{
		reg[itc] = new hw_8bit_reg(reg_specs[itc].offset, reg_specs[itc].rw,
						reg_specs[itc].name);
		reg[itc]->reg_init();
		// fprintf(logfile,"%s\n",reg_specs[itc].name);
	}

};


tc_drvr::~tc_drvr()
{

};

//-----------------------------------------------------------------------------|
//-----------------------------------------------------------------------------|
void restart_handler( int signo )
{
	fprintf(logfile,"TCD RECEIVED USR1 SIGNAL %d, restart tc_drvr\n",signo);
	fprintf(stderr,"TCD RECEIVED USR1 SIGNAL %d, restart tc_drvr\n",signo);
	stop_requested = 1;
	still_trying = 1;
	return;
};

void stop_handler( int signo )
{
	fprintf(logfile,"TCD RECEIVED USR2 SIGNAL %d, stop tc_drvr\n",signo);
	fprintf(stderr,"TCD RECEIVED USR2 SIGNAL %d, stop tc_drvr\n",signo);
	stop_requested = 1;
	still_trying = 0;
	return;
};

//-----------------------------------------------------------------------------|
int
main()
{

	tc_drvr tcc;
 	static int first = 1;
	struct timespec ts;
	int regn;
	uint8_t rval;
	int loop = 0;
	int bkup_spcs = 0;
	int chgd_flds = 0;
	uint8_t prev;
	uint8_t tcval[4];
	uint8_t prev_tcval[4];
	hw_8bit_reg * rdy;
	rdy = tcc.reg[sel_rdr_stat_bits];

#ifdef LOG_FILE
	if((logfile = fopen(LOG_FILE,"w+"))==NULL)
	{
		int error_val = errno;
		printf("ERROR: opening log file %s, use stderr, error %d, %s\n",
				LOG_FILE,error_val,strerror(error_val));
		logfile = stderr;
	}
#endif

	rt_set_logfile("TC",logfile);

	struct sigaction restart_act;
	struct sigaction stop_act;
	sigset_t restart_set;
	sigset_t stop_set;

	sigemptyset( &restart_set );
	sigaddset( &restart_set, SIGUSR1 );
	restart_act.sa_flags = 0;
	restart_act.sa_mask = restart_set;
	restart_act.sa_handler = &restart_handler;
	sigaction( SIGUSR1, &restart_act, NULL);
	
	sigemptyset( &stop_set );
	sigaddset( &stop_set, SIGUSR2 );
	stop_act.sa_flags = 0;
	stop_act.sa_mask = stop_set;
	stop_act.sa_handler = &stop_handler;
	sigaction( SIGUSR2, &stop_act, NULL);

	still_trying = 1;
	stop_requested = 0;

	while(still_trying)
	{
	
		// instantiate and open all of the message queues 
		// to and from other procs
		//
		int i;
		for(i=0;i<NUM_TCDMQ_VALS;i++) 
		{
			if((mq[i].rt_mq = 
					new rt_mqueue(mq[i].which,mq[i].rw,mq[i].blk))==NULL)
			{
				fprintf(stderr,
					"ERROR: Failed to open message queue for %s in tc_drvr\n",
								rt_mq_names[mq[i].which]);
			}
			else
			{
				fprintf(stderr,"opened mqueue %d for %s\n",
						mq[i].rt_mq->rt_mqueue_descr,
						rt_mq_names[mq[i].which]);
			}
		}
		embeddedInitLogging('X',&mq[TCDMQ_TO_MSG]);
		
		ThreadCtl(_NTO_TCTL_IO, 0);
		// const int num_tc_regs = 4;
		// reg_nums tc_regs[num_tc_regs] = {	tc_tbits_frames,	tc_tbits_secs,
											// tc_tbits_mins,		tc_tbits_hours };
	
		// reset tc_card coprocessor software
		tcc.reg[host2brd_mbox]->reg_immed_write(0x02);
		sleep(1);
		// change operations to SMPTE mode and Idle mode
		tcc.reg[host2brd_mbox]->reg_immed_write(0x03);
		// enable ltc reader
		tcc.reg[host2brd_mbox]->reg_immed_write(0x21);
		// take special bits out of timecode to make it easy to read for debugging
		tcc.reg[ltc_rdr_cntl]->reg_immed_write(0x00);
		while(!stop_requested)
		{
			ts.tv_sec = 0;
			ts.tv_nsec = 12000000;
			loop = 0;
			while(((rval = rdy->reg_immed_read())&0x80)==prev)
			{
				loop++;
				nanosleep(&ts,NULL);
				if(ts.tv_nsec > 1000000)
					ts.tv_nsec /= 2;
	
			}
			prev = rval & 0x80;
	
			for(regn=0;regn<NUM_TC_CARD_REGS;regn++)
			{
				rval = tcc.reg[regn]->reg_immed_read();
				if(	(regn >= tc_tbits_frames) && (regn <= tc_tbits_hours))
				{
					prev_tcval[regn-tc_tbits_frames] = tcval[regn-tc_tbits_frames];
					tcval[regn-tc_tbits_frames]=rval;
				}
					// tcc.reg[regn]->reg_fprint(stderr);
			}
			if(prev_tcval[3]!=tcval[3])
			{
				bkup_spcs = 17;
				chgd_flds = 4;
				// fprintf(stderr,"hours are different, 0x%0x vs 0x%0x\n",
								// prev_tcval[3],tcval[3]);
			}
			else if(prev_tcval[2]!=tcval[2])
			{
				bkup_spcs = 9;
				chgd_flds = 3;
				// fprintf(stderr,"minutes are different, 0x%0x vs 0x%0x\n",
								// prev_tcval[2],tcval[2]);
			}
			else if(prev_tcval[1]!=tcval[1])
			{
				bkup_spcs = 6;
				chgd_flds = 2;
				// fprintf(stderr,"seconds are different, 0x%0x vs 0x%0x\n",
								// prev_tcval[1],tcval[1]);
			}
			else
			{
				bkup_spcs = 3;
				chgd_flds = 1;
				// fprintf(stderr,"frames are different, 0x%0x vs 0x%0x\n",
								// prev_tcval[0],tcval[0]);
			}
			
			if(first)
			{
				bkup_spcs = 17;
				chgd_flds = 4;
				first = 0;
			}
			for(int j=0;j<bkup_spcs;j++)
				fprintf(stderr,"%c",0x08);
			if(bkup_spcs==17)
			{
				fprintf(stderr,"LTC: ");
			}
			for(int j=chgd_flds;j>=1;j--)
			{
				fprintf(stderr,"%01d%01d%c",tcval[j-1]>>4,tcval[j-1]&0x0f,
								(j==1)?' ':':');
			}
			// fprintf(stderr,"\n%d\n",bkup_spcs);
			tcc.reg[host2brd_mbox]->reg_immed_write(0x00);


			// send message to sched_mgr with new timecode
			if((count%3)==0)
			{
			mq_tc_frame_notice_msg * p_msg;
			
			ssize_t msg_size = sizeof(mq_tc_frame_notice_msg);
			ssize_t size_sent;
	
			if((p_msg = (mq_tc_frame_notice_msg *)malloc(msg_size))==NULL)
			{
				fprintf(logfile,
					"ERROR: malloc failed for mq_msg for add_device message\n");
				abort();
			}
			// build up content of tc_frame_notice message
			p_msg->header = tc_frame_notice_msg;
			p_msg->header.msg_id = mq_get_next_msg_id();

			p_msg->timecode.tens_hours = tcval[3]>>4;
			p_msg->timecode.ones_hours = tcval[3] & 0x0f;
			p_msg->timecode.tens_mins = tcval[2]>>4;
			p_msg->timecode.ones_mins = tcval[2] & 0x0f;
			p_msg->timecode.tens_secs = tcval[1]>>4;
			p_msg->timecode.ones_secs = tcval[1] & 0x0f;
			p_msg->timecode.tens_frames = tcval[0]>>4;
			p_msg->timecode.ones_frames = tcval[0] & 0x0f;

#if 0
			fprintf(stderr,"Sending tc_notice %1d%1d:%1d%1d:%1d%1d:%1d%1d\n",
						p_msg->timecode.tens_hours,
						p_msg->timecode.ones_hours,
						p_msg->timecode.tens_mins,
						p_msg->timecode.ones_mins,
						p_msg->timecode.tens_secs,
						p_msg->timecode.ones_secs,
						p_msg->timecode.tens_frames,
						p_msg->timecode.ones_frames);
#endif

			if((size_sent = mq[TCDMQ_TO_SCHED].rt_mq->send_to_mqueue(
				(char *)p_msg, msg_size)) !=sizeof(mq_msg_header))
			{
				//fprintf(logfile,"ERROR: size sent %d instead of %d in "\
				//				"tc_drvr\n",size_sent,msg_size);
			}
			free(p_msg);
			p_msg = NULL;
			}
			count++;
		}
		// close all of the message queues used
		for(i=0;i<NUM_TCDMQ_VALS;i++) 
		{
			delete mq[i].rt_mq;
		}
	}
	return 1;
};

#if 0
	{
		// save a copy of input timecode
	    origTCf = tcPosFromTC(&InputTCval);

	    // smooth over LTC  glitches 
	    tcIncTimeCode(&LastLTC);
	    if((tcPosFromTC(&InputTCval)
		- (newCount ? (float)tcGetDSTcorrection() : 0 ))
	       != tcPosFromTC(&LastLTC)
	       && newCount < 5)
	    {
			BLOG(SMOOTH_LVL,TCSTAT_mID,
		     	"Bad  InputTCval: %02d:%02d:%02d:%02d.%01d%s",
		     	InputTCval.seg.hours,InputTCval.seg.mins,
		     	InputTCval.seg.secs,InputTCval.seg.frames,
		     	InputTCval.seg.field?1:2,InputTCval.seg.df?"DF":"ND");
			LastLTCbad = TRUE;
			InputTCval = LastLTC;
			InputMFTCval = InputTCval;
			InputMFTCval.seg.field = 1;
			newCount++;
			BLOG(SMOOTH_LVL,TCSTAT_mID,
		     	"SMOOTHING LTC from %f to %f",origTCf,
		     	tcPosFromTC(&LastLTC));
	    }
	    else
	    {
			static float prevNewLTCval = 0;
	
			newCount = 0;
			DSTchange = FALSE;
			if(tcPosFromTC(&InputTCval)
		   	!= tcPosFromTC(&LastLTC)
		   	&& tcPosFromTC(&InputTCval) != prevNewLTCval)
			{
		    	BLOG(DIAG2,_mID,"Changing LTC Continuum by %f from"
			 	" %02d:%02d:%02d:%02d.%1d to %02d:%02d:%02d:%02d.%1d",
		     	tcPosFromTC(&InputTCval) - tcPosFromTC(&LastLTC),
			 	InputTCval.seg.hours,InputTCval.seg.mins,
			 	InputTCval.seg.secs,InputTCval.seg.frames,
			 	InputTCval.seg.field,
			 	LastLTC.seg.hours,LastLTC.seg.mins,
			 	LastLTC.seg.secs,LastLTC.seg.frames,
			 	LastLTC.seg.field);
		    	prevNewLTCval = tcPosFromTC(&InputTCval);
			}
	    }
	    if(LastLTCbad)
	    {
			BLOG(SMOOTH_LVL,TCSTAT_mID,
		     	"Good InputTCval: %02d:%02d:%02d:%02d.%01d%s",
		     	InputTCval.seg.hours,InputTCval.seg.mins,
		     	InputTCval.seg.secs,InputTCval.seg.frames,
		     	InputTCval.seg.field?1:2,InputTCval.seg.df?"DF":"ND");
			LastLTCbad = FALSE;
	    }

	    tcPosToTC(IsVS625(bscGetVidStdNo()),pTA->SysConfig.DropFrame,
		      tcPosFromTC(&InputTCval) - (float)tcGetDSTcorrection(),
		      &DSTcorrectedTC);
	    
	    LastLTC = InputTCval;

	    if(newCount != 0 && DSTchange)
	    {
			// undo smoothing if DST change happened
			tcPosToTC(IsVS625(bscGetVidStdNo()),pTA->SysConfig.DropFrame,
			  	origTCf,&InputTCval);
			InputMFTCval = InputTCval;
			InputMFTCval.seg.field = 1;
			LastLTC = InputTCval;
			BLOG(SMOOTH_LVL,_mID,
		     	"Undo Smoothing, back to %02d:%02d:%02d:%02d or %f",
		     	InputTCval.seg.hours,InputTCval.seg.mins,
		     	InputTCval.seg.secs,InputTCval.seg.frames,
		     	tcPosFromTC(&InputTCval));
	    }
	}
		
#endif
