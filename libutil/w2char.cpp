//-----------------------------------------------------------------------------|
// Copyright (C) 2020, All rights reserved.
// embeddedKeith -- Confidential and Proprietary.
// -*^*-
//-----------------------------------------------------------------------------|
//
//	Module:	w2char
//
//	Description:	
//
//	FileName:	w2char.cpp
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       09/14/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <time.h>
#include "nov_log.hpp"
#include "w2char.hpp"

// this is the buffer for stderr messages handled by
// msg_mgr through WMQ_ALL_TO_MSG mqueue
extern char stderr_str[MAX_STDLOG_STR_LENGTH];
extern const char mod_chars[4];

void
hex_byte_dump_to_string(char * buf, size_t len, 
						char * data, size_t bytes, size_t indent)
{
	char blanks[36];
	int i;
	int bufx = 0;
	if(indent>32)
		indent=32;
	blanks[0]='\n';
	for(i=1;i<=(int)indent;i++)
			blanks[i]=' ';
	blanks[indent+1]=0;
	for(i=0;i<(int)bytes;i++)
	{
		bufx += snprintf(&buf[bufx],(int)len - (int)bufx, "%s%02X ",
					((i%16)?"":blanks), (_Uint8t)data[i]);
	}
	bufx += snprintf(&buf[bufx],3,"\n");
	buf[bufx]=0;
};

void
hex_byte_dump_to_file(FILE * errorfile, char * data, size_t size)
{
	char prbuf[5122];
	if(size > 1024)
			size = 1024;
	hex_byte_dump_to_string(prbuf,5122,data,size,31);
	if(errorfile==stderr)
	{
		fprintf(stderr,"%s",prbuf);
	}
	else
	{
		fprintf(errorfile,"%s",prbuf);
	}
};
	

// convert char string into w2char string (our struct for 2 byte wide char)
void char_string_to_w2char(w2char_t * wide_string, char * char_string, 
				size_t length)
{
	unsigned int i;
	for(i=0;char_string[i] && (i<length);i++)
	{
		wide_string[i].wchars.wchar_hi = 0;
		wide_string[i].wchars.wchar_lo = char_string[i];
	}
	if(i>=length)
		i=length-1;
	wide_string[i].wchars.wchar_hi = wide_string[i].wchars.wchar_lo = 0;
};

// convert w2char string into char string (our struct for 2 byte wide char)
void w2char_string_to_char(w2char_t * wide_string, char * char_string, 
				size_t length)
{
	unsigned int i;
	for(i=0;wide_string[i].wchars.wchar_lo && (i<length);i++)
	{
		char_string[i] = wide_string[i].wchars.wchar_lo;
	}
	if(i>=length)
		i=length-1;
	char_string[i] = 0;
};

int compare_w2char_strings(w2char_t * wcstr1, w2char_t * wcstr2, size_t length)
{
	unsigned int i = 0;;
	while((i<length) && wcstr1[i].wchars.wchar_lo && wcstr2[i].wchars.wchar_lo)
	{
		if(wcstr1[i].wchars.wchar_lo < wcstr2[i].wchars.wchar_lo)
			return(-1);
		else if(wcstr1[i].wchars.wchar_lo > wcstr2[i].wchars.wchar_lo)
			return(1);
		if(wcstr1[i].wchars.wchar_lo == 0 && wcstr2[i].wchars.wchar_lo == 0)
			break;
		i++;
	}
	return(0);
};
			
