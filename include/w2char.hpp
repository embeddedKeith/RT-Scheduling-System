#ifndef _W2CHAR_H_
#define _W2CHAR_H_
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
//	FileName:	w2char.h
//
//	Originator:	Keith DeWald
//
//  Initials  Date           Description
//  --------  ---------      -------------------------------------------------
//  KRD       09/14/2020     Created file and began implementation
//
//
//-----------------------------------------------------------------------------|

#include <stdio.h>

void hex_byte_dump_to_file(FILE * errorfile, char * data, size_t len);

void hex_byte_dump_to_string(char * buf, size_t len, char * data, 
				size_t bytes, size_t indent);
struct
w2chars
{
	_Uint8t wchar_lo;
	_Uint8t wchar_hi;
};

struct
w2char_t
{
	w2chars			wchars;
};

void char_string_to_w2char(w2char_t * wide_string, char * char_string,
			size_t length);
void w2char_string_to_char(w2char_t * wide_string, char * char_string,
			size_t length);
int compare_w2char_strings(w2char_t * str1, w2char_t * str2, size_t length);
	

#endif	// _W2CHAR_H_
