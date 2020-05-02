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
#include <termios.h>
#include "../include/osinc.hpp"


int
main(int argc, char** argv)
{
	int ser_fd;
	char portname[32];
	int portnum;
	int baud;

	if(argc > 1)
		sscanf(argv[1],"%d",&portnum);
	else
		return 1;

	if(argc > 2)
		sscanf(argv[2],"%d",&baud);
	else
		baud = 38400;

	fprintf(stderr,"portbaud using /dev/ser%1d at %d baud\n",
					portnum,baud);

	fprintf(stderr,"running portbaud for port %d\n",portnum);
	portname[snprintf(portname,30,"/dev/ser%1d",portnum)]=0;
	fprintf(stderr,"opening %s\n",portname);

	if((ser_fd=open(portname,O_RDWR))==-1)
	{
		int error_val = errno;
		fprintf(stderr,"open failed for %s with %d, %s\n",portname,
				error_val,strerror(error_val));
	}

	// get terminal control attributes
	struct termios tios;
	if(tcgetattr(ser_fd,&tios)==-1)
	{
		int error_val = errno;
		fprintf(stderr,"tcgetattr failed with %d, %s\n",
				error_val, strerror(error_val));
	}
	else
	{
		// show some of the attributes
		fprintf(stderr,"in baud was %d and out baud was %d\n",
				(int)cfgetispeed(&tios),(int)cfgetospeed(&tios));
	}
	close(ser_fd);

	return(0);
}
