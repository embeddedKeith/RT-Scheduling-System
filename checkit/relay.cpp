#include <sys/types.h>
#include <sys/socket.h>
#include <sys/dcmd_chr.h>
#include <time.h>
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
#include "../include/osinc.hpp"


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
	const int NUM = 26;
	int loop;
	char buf[NUM];
	INT wlen = 0;
	INT rlen = 0;
	int ser_fda;
	int ser_fdb;
	char portnamea[32];
	char portnameb[32];
	int portnuma;
	int portnumb;
	int baud;
	speed_t ispeed, ospeed;
	int data = _CTL_RTS_CHG;
	struct termios tios;
	char blanks[80];
	int offset = 0;
	for(int j=0;j<80;j++)
		blanks[j]=' ';

	if(argc > 1)
		sscanf(argv[1],"%d",&portnuma);
	else
		return 1;
	if(argc > 2)
		sscanf(argv[2],"%d",&portnumb);
	else
		return 1;

	if(argc > 3)
		sscanf(argv[3],"%d",&baud);
	else
		baud = 38400;

	if(argc > 4)
	{
		sscanf(argv[4],"%d",&offset);
		blanks[(offset<80)?offset:0]=0;
	}
	else
		blanks[0]=0;

	fprintf(stderr,"relay from /dev/ser%1d to /dev/ser%1d at %d baud\n",
					portnuma,portnumb,baud);

	portnamea[snprintf(portnamea,30,"/dev/ser%1d",portnuma)]=0;
	fprintf(stderr,"opening %s\n",portnamea);
	portnameb[snprintf(portnameb,30,"/dev/ser%1d",portnumb)]=0;
	fprintf(stderr,"opening %s\n",portnameb);

	if((ser_fda=open(portnamea,O_RDWR))==-1)
	{
		int error_val = errno;
		fprintf(stderr,"open failed for %s with %d, %s\n",portnamea,
				error_val,strerror(error_val));
	}
	if((ser_fdb=open(portnameb,O_RDWR))==-1)
	{
		int error_val = errno;
		fprintf(stderr,"open failed for %s with %d, %s\n",portnameb,
				error_val,strerror(error_val));
	}

	// turn off RTS
	if(devctl(ser_fda,DCMD_CHR_SERCTL, &data,sizeof(data),NULL))
	{
		fprintf(stderr,"failed to turn off RTS\n");
	}
	// get terminal control attributes
	if(tcgetattr(ser_fda,&tios)==-1)
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
		if(tcsetattr(ser_fda,TCSAFLUSH,&tios)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"tcsetattr failed with %d, %s\n",
					error_val, strerror(error_val));
		}
	}
	// turn off RTS
	if(devctl(ser_fdb,DCMD_CHR_SERCTL, &data,sizeof(data),NULL))
	{
		fprintf(stderr,"failed to turn off RTS\n");
	}
	// get terminal control attributes
	if(tcgetattr(ser_fdb,&tios)==-1)
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
		if(tcsetattr(ser_fdb,TCSAFLUSH,&tios)==-1)
		{
			int error_val = errno;
			fprintf(stderr,"tcsetattr failed with %d, %s\n",
					error_val, strerror(error_val));
		}
	}

	showAttrs(ser_fda);
	showAttrs(ser_fdb);

	int i = 0;
	
	while(1)
	{
		i++;
		loop = i % NUM;
		buf[loop] = 0;
		if((rlen = read(ser_fda,&buf[loop],1))==1)
		{
			struct timespec ts;
			clock_gettime(CLOCK_REALTIME,&ts);
			fprintf(stderr,"%d.%d: %s%s\n",(int)(ts.tv_sec % 60),
					(int)(ts.tv_nsec/1000),
				blanks,prcstr(buf[loop]));
		}
		else
			fprintf(stderr,"read failed with rlen %d\n",rlen);

		if((wlen = write(ser_fdb,&buf[loop],1))==1)
		{
			//fprintf(stderr,"                wrote %s to %s\n",
				//portnameb,prcstr(buf[loop]));
		}
		else
			fprintf(stderr,"write failed with wlen %d\n",wlen);
	}
	close(ser_fda);
	close(ser_fdb);

	return(0);
}
