#ifndef _RT_ERRORS_
#define _RT_ERRORS_
//-----------------------------------------------------------------------------|
// rt_errors.h
//
// Copyright(c) 2020, embeddedKeith
//
// developed by: Keith DeWald
//
// started: 8/6/01
//
// last mod: 8/7/01
//-----------------------------------------------------------------------------|

enum
function_success_t
{
	SUCCESS = 0,
	FAILURE
};

enum
en_rt_error_type
{
	ERT_OK = 0,
	ERT_CONSTR_FAILED,
	NUM_ERT_VALS
};

enum
en_ertmq_error
{
	ERTMQ_OK = 0,
	ERTMQ_BAD_MSGSIZE,
	ERTMQ_SENDERR,
	ERTMQ_RCVERR,
	NUM_ERTMQ_VALS
};


#endif // _RT_ERRORS_
