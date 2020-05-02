#ifndef _TC_DRVR_H_
#define _TC_DRVR_H_
//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	tc_drvr
//
//	Description:	
//
//	FileName:	tc_drvr.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       10/11/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include "rtc_time.hpp"

#pragma pack (1)
 
enum
en_tcdrvr_mqueues
{
	TCDMQ_TO_MSG = 0,
	TCDMQ_TO_SCHED,
	TCDMQ_TO_STAT,
	NUM_TCDMQ_VALS
};

// an array of this struct, indexed by en_sched_mqueues above, is used
// to keep track of just the mqueues used by sched_mgr
//
struct
tcdrvr_mqueue
{
	en_which_mqueue which;
	en_read_write	rw;
	en_blk			blk;
	rt_mqueue * rt_mq;
};

const uint8_t REG_NAME_LENGTH = 32;

enum rw_mode
{
	rw_unknown=0,
	read_only,
	write_only,
	read_or_write
};
const rw_mode RO = read_only;
const rw_mode RW = read_or_write;

struct
reg_spec
{
	uint8_t 	offset;
	rw_mode		rw;
	char* 		name;
};

enum
reg_nums
{
	aec_vend_lo=0,
	aec_vend_hi,
	pci_brd_num_lo,
	pci_brd_num_hi,
	sw_ver_letter,
	sw_ver_number,
	brd_capbl_code,
	brd_op_mode_info,
	pend_intr_bmap,
	brd2host_mbox,
	tc_tbits_frames,
	tc_tbits_secs,
	tc_tbits_mins,
	tc_tbits_hours,
	tc_ubits_frames,
	tc_ubits_secs,
	tc_ubits_mins,
	tc_ubits_hours,
	sel_rdr_embd_bits,
	sel_rdr_stat_bits,
	tc_cmpr_stat_bits,
	brd2host_mbox_0,
	brd2host_mbox_1,
	brd2host_mbox_2,
	brd2host_mbox_3,
	host2brd_mbox_0,
	host2brd_mbox_1,
	host2brd_mbox_2,
	host2brd_mbox_3,
	ltc_rdr_cntl,
	comparator_cntl,
	intr_cntl_bmap,
	host2brd_mbox,
	cmpr_tbits_frames,
	cmpr_tbits_secs,
	cmpr_tbits_mins,
	cmpr_tbits_hours,
	special_brd_status,
	NUM_TC_CARD_REGS
};
	
const reg_spec reg_specs[NUM_TC_CARD_REGS] =
{
	// timecode bits are packed BCD
	{ 0x00, RO, "AEC vendor code low byte"},
	{ 0x01, RO, "AEC vendor code high byte"},
	{ 0x02, RO, "PCI-TC board number low byte"},
	{ 0x03, RO, "PCI-TC board number high byte"},
	{ 0x06, RO, "Software revision A-Z letter"},
	{ 0x07, RO, "Software revision 0-9 number"},
	{ 0x08, RO, "Board Capabilities Code"},
	{ 0x0d, RO, "Board operating mode info"},
	{ 0x0e, RO, "Pending interrupts bit map"},
	{ 0x0f, RO, "Board-to-host mailbox port"},
	{ 0x10, RO, "Timecode time bits frames"},
	{ 0x11, RO, "Timecode time bits seconds"},
	{ 0x12, RO, "Timecode time bits minutes"},
	{ 0x13, RO, "Timecode time bits hours"},
	{ 0x14, RO, "Timecode user bits frames"},
	{ 0x15, RO, "Timecode user bits minutes"},
	{ 0x16, RO, "Timecode user bits seconds"},
	{ 0x17, RO, "Timecode user bits hours"},
	{ 0x18, RO, "Selected reader embedded bits"},
	{ 0x19, RO, "Selected reader status bits"},
	{ 0x1b, RO, "TC comparator status bits"},
	{ 0x1c, RO, "Board-to-host mailbox byte0"},
	{ 0x1d, RO, "Board-to-host mailbox byte1"},
	{ 0x1e, RO, "Board-to-host mailbox byte2"},
	{ 0x1f, RO, "Board-to-host mailbox byte3"},
	{ 0x20, RW, "Host-to-board mailbox byte0"},
	{ 0x21, RW, "Host-to-board mailbox byte1"},
	{ 0x22, RW, "Host-to-board mailbox byte2"},
	{ 0x23, RW, "Host-to-board mailbox byte3"},
	{ 0x2c, RW, "LTC reader control"},
	{ 0x2d, RW, "Comparator control"},
	{ 0x2e, RW, "Interrupt control bit map"},
	{ 0x2f, RW, "Host-to-board mailbox port"},
	{ 0x30, RW, "Comparator time bits frames"},
	{ 0x31, RW, "Comparator time bits secods"},
	{ 0x32, RW, "Comparator time bits minutes"},
	{ 0x33, RW, "Comparator time bits hours"},
	{ 0xfe, RW, "Special board status register"}
};

const uint64_t TC_BASE_ADDR = 0xd000;

class bcd_tc
{
	public:
		unsigned int	tens_hours:4;
		unsigned int	ones_hours:4;
		unsigned int	tens_mins:4;
		unsigned int	ones_mins:4;
		unsigned int	tens_secs:4;
		unsigned int	ones_secs:4;
		unsigned int	tens_frames:4;
		unsigned int	ones_frames:4;

	bcd_tc(int hours = 0, int mins = 0, int secs = 0, int frames = 0);
	~bcd_tc();
};

class hw_8bit_reg
{
	public:
		hw_8bit_reg();
		hw_8bit_reg(uint32_t addr_offset, rw_mode rwm, char* reg_str);
		~hw_8bit_reg();

		uint8_t reg_read();
		uint8_t reg_immed_read();
		void	reg_write(const uint8_t & val);
		void	reg_immed_write(const uint8_t & val);
		int		reg_init();
		void	reg_fprint(FILE* pfile);

	private:
		uint64_t	reg_offset;
		uintptr_t	reg_port;
		uint8_t		reg_rval;
		uint8_t		reg_wval;
		uint8_t		default_val;
		char		reg_name[REG_NAME_LENGTH];
		rw_mode		rw_access;
};

//-----------------------------------------------------------------------------|
// tc_drvr class definition
//
class tc_drvr
{
	public:
		tc_drvr();
		~tc_drvr();
		int enable_tc_intr();
		int disable_tc_intr();
		int get_timecode(bcd_tc * p_tc);
		int get_timecode(int * hours, int * mins, int * secs, int * frames);
		hw_8bit_reg	*	reg[NUM_TC_CARD_REGS];

	protected:
	private:
		bcd_tc			tc_val;
		rtc_time		time_of_last_intr;
		
};

// #pragma pack (pop)

#endif
