#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../include/osinc.hpp"
#include "../include/nport.hpp"


NString portname = "ser1";
NPort *testport;
#define NUM 3
char tbuf[]="abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ##########";
char mod_chars[4];

int main()
{
	assert(NUM>=1 && NUM<=26);
	testport = new NPort(portname);
	testport->open();

	int lenWritten;

	for(int i = 0; i < 4; i++)
	{
#if 0
		tbuf[0] = 0;
		tbuf[1] = 0x11;
		tbuf[2] = 0x11;

		if(testport->write(tbuf,(int)NUM,lenWritten,1))
#else
		if(testport->write((char*)(&tbuf[i%26]),(int)NUM,lenWritten,1))
#endif
		{
			if(lenWritten != NUM)
				fprintf(stderr,"Wrote %d bytes, not %d",lenWritten,NUM);
			char showbuf[26];
			snprintf(showbuf,NUM,"%s",(char*)(&tbuf[i%26]));
		}
		sleep(1);
	}

	return 1;
}



