//  The data members are:
//		FILE* port;
//		NString strPortName;
//		Baud eBaud;
//		DataBits eBits;
//		Parity eParity;
//		StopBits eStop;
//		DWORD iLastError;
//		bool bPortOpen;
//		NPrivateResource portLock;
// Copyright:  2020 embeddedKeith
// -*^*-
//-------------------------------------------------------
#include "nport.hpp"
#include "nexception.hpp"
#include <iostream>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/dcmd_chr.h> 
#include <unistd.h>
#include <devctl.h>
#include <termios.h>
#include "nov_log.hpp"

std::ostream& operator << ( std::ostream& os, const  NPort& rhs )
{
	os << "Port:" << rhs.strPortName << std::endl;
	return( os );
}

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
		fprintf(stderr,"c_ispeed %ld, c_ospeed %ld\n",looktios.c_ispeed,
						looktios.c_ospeed);

}
	
NPort::NPort
(
	NString strPort, 
	Baud enBaud,
	DataBits enBits,
	Parity enParity,
	StopBits enStop
) :
	portLock(),
	fd(-1), 
//	strPortName(strPort),
	eBaud(enBaud), 
	eBits(enBits),
	eParity(enParity),
	eStop(enStop),
	iLastError(0),
	bPortOpen(false)
{
	strPortName = strPort;

	if(strPort.length() == 0)
	{
	    throw NLogicException( "Zero length port name specified", "NPort:: NPort", __LINE__, __FILE__ );
	}
}

bool NPort::Open()
{
	NResourceLock lck(portLock);
	NString strOpenName = NString("/dev/") + strPortName;
    std::cerr <<"The port is:" << strOpenName << std::endl;

	if( fd != -1 )
	{
	    CloseNoLock();
	}

	fd = open(strOpenName, O_RDWR );
	if( fd == -1 )
	{
		int error_val = errno;
		char buffer[200];
		sprintf( buffer, "Unable to open port:%s, reason:%s", (const char*)strPortName, strerror(error_val) );
		fprintf(stderr,"%s",buffer);
	    throw NResourceException( buffer, "NPort::Open",__LINE__, __FILE__ );
	}
	else
		fprintf(stderr,"fd is %d\n",fd);




// TEMP TEST
	struct termios portattr;
	tcgetattr(fd,&portattr);
	char hexbuf[1024];
	hex_byte_dump_to_string(hexbuf,1024,(char*)(&portattr), sizeof(termios),16);
		fprintf(stderr,"dump of termios: %s\n",hexbuf);





	bPortOpen = true;
		fprintf(stderr,"set parity\n");
	SetParity( eParity );
		fprintf(stderr,"set number of bits\n");
	SetBits( eBits );
		fprintf(stderr,"set stop bits\n");
	SetStop( eStop );
		fprintf(stderr,"set RTS mode\n");
	SetRTS( false  );
		fprintf(stderr,"set DTR mode\n");
	SetDTR( false );

		fprintf(stderr,"set baud rate\n");
	SetBaud(  eBaud );

//	try
//	{
//	    setReadTimeoutNoLock(0);
//	} 
//	catch( NException& ) 
//	{
 //       closeNoLock();
//	    throw;
//	}
	fprintf(stderr,"done with nport::Open\n");

	return(true);
}

NPort::~NPort()
{
	CloseNoLock();
}

bool NPort::Close()
{
	NResourceLock lck(portLock);
	return( CloseNoLock() );
}

bool NPort::CloseNoLock()
{
	if( fd != -1)
	{
		if( close(fd) == -1)
	    {
			fd = -1;
			bPortOpen = false;
			throw NResourceException( strerror(errno), "NPort::CloseNoLock",__LINE__, __FILE__ );
	    }
	}

	fd = -1;
	bPortOpen = false;
	return(true);	
}


//------------------------------------------------------------------------------
//  This function sets the baud rate to the enumerated value

bool NPort::SetBaud( Baud enBaud ) 
{
	NResourceLock lck(portLock);

	eBaud = enBaud;
	if(bPortOpen == true && fd != -1 )
	{
		termios termios_p;
		speed_t speed = enBaud;
		if( 0 == tcgetattr(fd, &termios_p))
		{
			cfsetispeed( &termios_p, speed );  // input speed
			cfsetospeed( &termios_p, speed );  // output speed
			if(tcsetattr( fd, TCSADRAIN, &termios_p)==-1)
			{
				int error_val = errno;
				fprintf(stderr,"tcsetattr failed with %d, %s\n",
								error_val,strerror(error_val));
				abort();
			}
		}
		else
		{
			int error_val = errno;
			fprintf(stderr,"tcgetattr failed with %d, %s\n",
							error_val,strerror(error_val));
			abort();
		}
	}
	return(true);
}


//--------------------------------------------------------------
//  This function sets the parity to one of the value:
//  NoParity = 0, OddParity, EvenParity,MarkParity, or SpaceParity.
//  First get the current settings, then clear or set the appropriate
//   parity bits.

bool NPort::SetParity(Parity enParity)
{
	NResourceLock lck(portLock);
	fprintf(stderr,"lock set in setParity\n");
	eParity = enParity;
	if(bPortOpen == true && fd != -1 )
	{
		fprintf(stderr,"bPortOpen == true and fd != -1 in setParity\n");
		termios termios_p;
		if ( 0 == tcgetattr( fd, &termios_p ) )
		{
			fprintf(stderr,"tcgetattr successfull in setParity\n");
			fprintf(stderr,"enParity is 0x%0x\n",enParity);
			switch ( enParity )
			{
				default:
				case NoParity:
				{
					termios_p.c_cflag  &= ~PARENB; // Clear parity enable
					break;
				}
				case OddParity:
				{
					fprintf(stderr,"setting odd parity\n");
					termios_p.c_cflag |= PARENB;  // enable parity
					termios_p.c_cflag |= PARODD;  // select odd parity
					break;
				}
				case EvenParity:
				{
					termios_p.c_cflag |= PARENB;  
					termios_p.c_cflag &= ~PARODD; 
					break;
				}
				case MarkParity:
				{
					termios_p.c_lflag |= IEXTEN;
					termios_p.c_cflag |= PARSTK | PARODD;  
					break;
				}
				case SpaceParity:
				{
					termios_p.c_lflag |= IEXTEN;
					termios_p.c_cflag |= PARSTK;   
					termios_p.c_cflag &= ~PARODD;
					break;
				}
			}
			fprintf(stderr,"out of switch statement\n");

			int result = tcsetattr( fd, TCSADRAIN, &termios_p );
			if ( result == -1 )
			{
				fprintf(stderr,"tcsetattr failed in setParity\n");
				throw NResourceException( strerror(errno), 
								"NPort::SetParity",__LINE__, __FILE__ );
			}
			else
				fprintf(stderr,"tcsetattr seems to have finished ok\n");
		}
		else  // Unable to read attributes
		{
			fprintf(stderr,"failed to read attributes in setParity\n");
			throw NResourceException( strerror(errno), 
							"NPort::SetParity",__LINE__, __FILE__ );
		}
// TEMP TEST
	struct termios portattr;
	tcgetattr(fd,&portattr);
	char hexbuf[1024];
	hex_byte_dump_to_string(hexbuf,1024,(char*)(&portattr), sizeof(termios),16);
		fprintf(stderr,"dump of termios after setting parity: %s\n",hexbuf);
	}
	fprintf(stderr,"returning from setParity\n");
	return(true);
}

bool NPort::SetStop(StopBits enStop)
{
	NResourceLock lck(portLock);
	eStop = enStop;
	if(bPortOpen == true && fd != -1)
	{
		termios termios_p;
		if ( 0 == tcgetattr( fd, &termios_p ) )
		{
			switch ( eStop )
			{
				default:
				case OnePlusStop:
				case TwoStop:
				{
					termios_p.c_cflag |= CSTOPB ;  // two stop bits 
					break;
				}
				case OneStop:
				{
					termios_p.c_cflag &= ~CSTOPB;  // one stop bit
					break;
				}
			
			}

			int result = tcsetattr( fd, TCSADRAIN, &termios_p );
			if ( result == -1 )
			{
				throw NResourceException( strerror(errno), "NPort::SetStop",__LINE__, __FILE__ );
			}
		}
		else  // Unable to read attributes
		{
			throw NResourceException( strerror(errno), "NPort::SetStop",__LINE__, __FILE__ );
		}
	}
	return(true);
}

bool NPort::SetBits(DataBits enBits)
{
	NResourceLock lck(portLock);

	eBits = enBits;
	if(bPortOpen == true && fd != -1 )
	{
		termios termios_p;
		if ( 0 == tcgetattr( fd, &termios_p ) )
		{
			switch ( eBits )
			{
				default:
				case Data4Bits:
				{
					termios_p.c_cflag &= ~(CS5 | CS6 | CS7 | CS8 ) ;
					break;
				}
				case Data5Bits:
				{
					termios_p.c_cflag |= CS5;  
					break;
				}
				case Data6Bits:
				{
					termios_p.c_cflag |= CS6; 
					break;
				}
				case Data7Bits:
				{
					termios_p.c_cflag |= CS7;  
					break;
				}
				case Data8Bits:
				{
					termios_p.c_cflag |= CS8;  
					break;
				}
			}

			int result = tcsetattr( fd, TCSADRAIN, &termios_p );
			if ( result == -1 )
			{
				throw NResourceException( strerror(errno), "NPort::SetBits",__LINE__, __FILE__ );
			}
		}
		else  // Unable to read attributes
		{
			throw NResourceException( strerror(errno), "NPort::SetBits",__LINE__, __FILE__ );
		}
	}
	return(true);
}

bool NPort::SetRTS(bool RTSON)
{
	NResourceLock lck(portLock);

	if(bPortOpen == false || fd == -1 )
	{
	    return(false);
	}
	
	int data = RTSON ? ( _CTL_RTS_CHG | _CTL_RTS ) : ( _CTL_RTS_CHG | 0 );

    if (devctl (fd, DCMD_CHR_SERCTL, &data, sizeof(data), NULL))
    {
	    throw NResourceException( strerror(errno), "NPort::SetRTS",__LINE__, __FILE__ );
	}
	return(true);
}


bool NPort::SetDTR(bool DTRON)
{
	NResourceLock lck(portLock);

	if(bPortOpen == false || fd == -1 )
	{
	    return(false);
	}
	int data = DTRON ? ( _CTL_DTR_CHG | _CTL_RTS ) : ( _CTL_DTR_CHG | 0 );

    if (devctl (fd, DCMD_CHR_SERCTL, &data, sizeof(data), NULL))
    {
	    throw NResourceException( strerror(errno), "NPort::SetDTR",__LINE__, __FILE__ );
	}
	return(true);
}

	
bool NPort::Read(PVOID pvBuffer, INT iBufLen, INT &iLenRead, bool bLock)
{
    NResourceLock lck(portLock);
    iLenRead = 0;



    if( fd == -1 || bPortOpen == false)
    {
			fprintf(stderr,"port was not open and fd legit\n");
        throw NResourceException( strerror(errno), "NPort::Read",__LINE__, __FILE__ );
	}

    if(pvBuffer == NULL || iBufLen < 1)
    {
			fprintf(stderr,"pbBuffer was NULL or length was < 1\n");
        throw NResourceException( strerror(errno), "NPort::Read",__LINE__, __FILE__ );
    }

    if( bLock)
    {
		// fprintf(stderr,"do a blocking read\n");
        NResourceLock lck(portLock);
		// fprintf(stderr,"locked the resource now do actual read\n");
		// int dLenRead = ::readcond( fd, pvBuffer, iBufLen, iBufLen, 11,11 );



		//struct termios portattr;
		//tcgetattr(fd,&portattr);
		//char hexbuf[1024];
		//hex_byte_dump_to_string(hexbuf,1024,(char*)(&portattr),
						//sizeof(termios),16);
		//fprintf(stderr,"dump of termios: %s\n",hexbuf);





		int dLenRead = read( fd, pvBuffer, iBufLen);
		if ( dLenRead != -1 )
        {
            iLenRead = dLenRead;
			char hexbuf[1024];
			hex_byte_dump_to_string(hexbuf,1024,(char*)pvBuffer,iLenRead,16);
			DLOG(DIAG4,NPORT_READ_mID,"block-read %d bytes from port\n%s\n",iLenRead, hexbuf);
            return(true);
        }
		else
			fprintf(stderr,"got -1 from read\n");
    } 
    else 
    {
		fprintf(stderr,"do a non-blocking read\n");
        int dLenRead = read( fd, pvBuffer, iBufLen );
		if ( dLenRead != -1 )
        {
            iLenRead = dLenRead;
			char hexbuf[1024];
			hex_byte_dump_to_string(hexbuf,1024,(char*)pvBuffer,iLenRead,16);
			DLOG(DIAG4,NPORT_READ_mID,"read %d bytes from port\n%s\n",iLenRead, hexbuf);
            return(true);
        }
    }

   	throw NResourceException( strerror(errno), "NPort::Read",__LINE__, __FILE__ );
    return(false);
}

bool NPort::Write( PVOID pvBuffer, INT iLen, INT &iLenWritten,
                             bool bLock)
{
    iLenWritten = 0;

    if( fd == -1 || bPortOpen == false)
    {
		// fprintf(stderr,"NPort::Write - line %d in %s",__LINE__, __FILE__ );
        throw NResourceException( strerror(errno), "NPort::Write",__LINE__, __FILE__ );
    }

    if(pvBuffer == NULL || iLen < 1)
    {
		// fprintf(stderr,"NPort::Write - line %d in %s",__LINE__, __FILE__ );
       throw NResourceException( strerror(errno), "NPort::Write",__LINE__, __FILE__ );
    }
#if 0
	// TEMP
	char* xpvBuffer;
	xpvBuffer = (char*)pvBuffer;
	xpvBuffer[0]='A';
	xpvBuffer[1]='B';
	xpvBuffer[2]='C';
#endif
    if(bLock)
    {
		// fprintf(stderr,"NPort::Write - line %d in %s",__LINE__, __FILE__ );
        NResourceLock lck(portLock);
		ssize_t dLenWritten = write( fd, pvBuffer, iLen);
		if ( dLenWritten >= 0 )
        {
            iLenWritten = dLenWritten;
			char hexbuf[1024];
			hex_byte_dump_to_string(hexbuf,1024,(char*)pvBuffer,iLenWritten,31);
			ULOG(DIAG4,NPORT_WRITE_mID,"block-wrote %d bytes to port\n%s\n",
							iLenWritten, hexbuf);
			// QLOG(DIAG4,NPORT_WRITE_mID,"block-wrote %d bytes to port\n%s\n",
			// 				iLenWritten, hexbuf);
            return(true); 
        }
    } 
	else 
	{
		// fprintf(stderr,"NPort::Write - line %d in %s",__LINE__, __FILE__ );
        ssize_t dLenWritten = write( fd, pvBuffer, iLen);
		if ( dLenWritten >= 0 )
        {
            iLenWritten = dLenWritten;
			char hexbuf[1024];
			hex_byte_dump_to_string(hexbuf,1024,(char*)pvBuffer,iLenWritten,31);
			ULOG(DIAG4,NPORT_WRITE_mID,"wrote %d bytes to port\n%s\n",
							iLenWritten, hexbuf);
            return(true);
        }
    }
	// fprintf(stderr,"NPort::Write - line %d in %se",__LINE__, __FILE__ );
    throw NResourceException( strerror(errno), "NPort::Write",__LINE__, __FILE__ );

}


bool NPort::SetReadTimeout(DWORD dwMsRead)
{
	NResourceLock lck(portLock);
	return( SetReadTimeoutNoLock(dwMsRead));
}

bool NPort::SetReadTimeoutNoLock(DWORD dwMsRead)
{
//	COMMTIMEOUTS stTimeOut;

	if( fd == -1 || bPortOpen == false)
	{
	    return(false);
	}
/*
	if(GetCommTimeouts(port, &stTimeOut) == false)
	{
	    throw NResourceException( strerror(errno), "NPort::SetReadTimeoutNoLock",__LINE__, __FILE__ );
	}

	stTimeOut.ReadIntervalTimeout = 0;
	if(dwMsRead == 0)
	{
	    stTimeOut.ReadIntervalTimeout = MAXDWORD;
	    stTimeOut.ReadTotalTimeoutMultiplier = 0;
	    stTimeOut.ReadTotalTimeoutConstant = 0;
	} 
	else if(dwMsRead == MAXDWORD)
	{
        stTimeOut.ReadIntervalTimeout = 0;
	    stTimeOut.ReadTotalTimeoutMultiplier = 0;
	    stTimeOut.ReadTotalTimeoutConstant = 0;
	} 
	else 
	{
        stTimeOut.ReadIntervalTimeout = 0;
	    stTimeOut.ReadTotalTimeoutMultiplier = 0;
	    stTimeOut.ReadTotalTimeoutConstant = dwMsRead;
	}

	if(SetCommTimeouts(port, &stTimeOut) == false)
	{
		throw NResourceException( strerror(errno), "NPort::SetReadTimeoutNoLock",__LINE__, __FILE__ );
	}
	*/
	return(true);
}

bool NPort::SetWriteTimeout(DWORD dwMsWrite)
{
	NResourceLock lck(portLock);
//	COMMTIMEOUTS stTimeOut;

	if( fd == -1 || bPortOpen == false)
	{
	    return(false);
	}
/*
	if(GetCommTimeouts(port, &stTimeOut) == false)
	{
	   	throw NResourceException( strerror(errno), "NPort::SetWriteTimeout",__LINE__, __FILE__ );
	}

	stTimeOut.WriteTotalTimeoutMultiplier = 0;
	stTimeOut.WriteTotalTimeoutConstant = dwMsWrite;

	if(SetCommTimeouts(port, &stTimeOut) == false)
	{
		throw NResourceException( strerror(errno), "NPort::SetWriteTimeout",__LINE__, __FILE__ );
	}
*/
	return(true);
}

DWORD NPort::GetReadBufferCount()
{
	NResourceLock lck(portLock);
	if( fd == -1 || bPortOpen == false )
	{
		throw NResourceException( "Port not open", "NPort::GetReadBufferCount",__LINE__, __FILE__ );
	}
	return(  tcischars( fd ) );
}

bool NPort::PurgeRead()
{
    unsigned char ucaBuf[1024];
	NResourceLock lck(portLock);
	INT iReadLen, iLenRead;

    while((iReadLen = GetReadBufferCount()) != 0)
    {
        if(iReadLen > (INT)sizeof(ucaBuf))
		{
           iReadLen = sizeof(ucaBuf);
		}
       Read(ucaBuf, iReadLen, iLenRead);
    }

	return(true);
}

NPort::DataBits NPort::GetDataBits() const
{
    return(eBits);
}

/*
//---------------------------------------------------------
//  The presumption is that a name must consist of 
//   text and/or numbers with no embedded white space

bool NPort::IsValidName( const NString& portname )
{
	for ( long index = 0; index < portname.GetLength();index++ )
	{
		char letter = portname[index]; 
		if ( !isdigit( letter )  && !isalpha( letter ))
		{
			return( false );
		}
	}
	return( true );
}
*/

//-------------------------------------------------------------------
// THis function sends the break command for 17 characters... so the 
//  time for 17 characters (in ms) is
//
//  (1000 ms / s ) / (BAUD  bit/s ) * 17 characters / 10 bits/char = 
//               
void NPort::SignalBreak()
{
	if( fd != -1 && bPortOpen == true )
	{
		int duration_ms = 1000 / eBaud * 17 * 10;
	    tcsendbreak(fd, duration_ms );  // minimum of 300 ms even if duration is 0
	}
}
