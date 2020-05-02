#include <sys/types.h>
#include <sys/socket.h>
#include <sys/dcmd_chr.h>
#include <sys/dcmd_chr.h>
#include <termios.h>
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
#include "../include/w2char.hpp"
#include "../include/osinc.hpp"


char mod_chars[4];

char * // simple, non-threadsafe, not for porting anywhere else
prcstr(char c)
{
	static char s[32];
	int ix = 0;

	if((c>='a' && c>='z') || (c>='A' && c<='Z') || (c>='0' && c<='9'))
		s[ix = snprintf(s,5,"%c - ",c)]=0;
	s[ix + snprintf(&s[ix],6,"0x%02x",(_Uint8t)c)]=0;
	return s;
};

void showAttrs(int fd)
{
		struct termios looktios;
		if(tcgetattr(fd,&looktios)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"tcgetattr failed with %d, %s\n",error_val,
							strerror(error_val));
			abort();
		}
		fprintf(stderr,"c_iflag 0x%0lx\nc_oflag 0x%0lx\nc_cflag 0x%0lx\n"
						"c_lflag 0x%0lx\n",looktios.c_iflag, looktios.c_oflag,
						looktios.c_cflag, looktios.c_lflag);
		// for(int itios=0;itios<NCCS;itios++)
		// 	fprintf(stderr,"cc_t[%d] 0x%0x\n",itios,looktios.cc_t[itios]);
		fprintf(stderr,"c_ispeed %ld, c_ospeed %ld\n",looktios.c_ispeed,
						looktios.c_ospeed);

}
	
int
main(int argc, char** argv)
{
	int loop;
	const int NUM = 26;

	char rbuf[NUM];
	INT rlen = 0;
	int ser_fd;
	char portname[32];
	int portnum;
	int baud;
	speed_t ispeed, ospeed;

	if(argc > 1)
		sscanf(argv[1],"%d",&portnum);
	else
		return 1;

	if(argc > 2)
		sscanf(argv[2],"%d",&baud);
	else
		baud = 38400;

	fprintf(stderr,"portin using /dev/ser%1d at %d baud\n",
					portnum,baud);

	fprintf(stderr,"running portin for port %d\n",portnum);
	portname[snprintf(portname,30,"/dev/ser%1d",portnum)]=0;
	fprintf(stderr,"opening %s\n",portname);

	if((ser_fd=open(portname,O_RDWR))==-1)
	{
		int error_val = errno;
		fprintf(stderr,"open failed for %s with %d, %s\n",portname,
				error_val,strerror(error_val));
	}



	// turn off RTS
	int data = _CTL_RTS_CHG;
	if(devctl(ser_fd,DCMD_CHR_SERCTL, &data,sizeof(data),NULL))
	{
		fprintf(stderr,"failed to turn off RTS\n");
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
		// set baud rates from arg or default
		ispeed = ospeed = (speed_t)baud;
		if(cfsetispeed(&tios,ispeed)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"cfsetispeed failed with %d, %s\n",
					error_val, strerror(error_val));
		}
		if(cfsetospeed(&tios,ospeed)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"cfsetospeed failed with %d, %s\n",
					error_val, strerror(error_val));
		}
		// set parity to odd
		tios.c_cflag |= PARENB;  // enable parity
		tios.c_cflag |= PARODD;  // select odd parity

		// put changed attributes into effect
		if(tcsetattr(ser_fd,TCSAFLUSH,&tios)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"tcsetattr failed with %d, %s\n",
					error_val, strerror(error_val));
		}
	}
		struct termios looktios;
		if(tcgetattr(ser_fd,&looktios)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"tcgetattr failed with %d, %s\n",error_val,
							strerror(error_val));
			abort();
		}
		fprintf(stderr,"c_iflag 0x%0lx\nc_oflag 0x%0lx\nc_cflag 0x%0lx\n"
						"c_lflag 0x%0lx\n",looktios.c_iflag, looktios.c_oflag,
						looktios.c_cflag, looktios.c_lflag);
		// for(int itios=0;itios<NCCS;itios++)
		// 	fprintf(stderr,"cc_t[%d] 0x%0x\n",itios,looktios.cc_t[itios]);
		fprintf(stderr,"c_ispeed %ld, c_ospeed %ld\n",looktios.c_ispeed,
						looktios.c_ospeed);

	showAttrs(ser_fd);

	for(int i = 0; i < 1000; i++)
	{
		loop = i % NUM;
		rbuf[loop]=0;
		if((rlen = read(ser_fd,&rbuf[loop],1))==1)
			fprintf(stderr,"%d: rlen %d, read %s from %s\n",
				i, rlen, prcstr(rbuf[loop]),portname);
		else
			fprintf(stderr,"read failed with rlen %d\n",rlen);

		//sleep(1);
	}
	close(ser_fd);
	return(0);
}
