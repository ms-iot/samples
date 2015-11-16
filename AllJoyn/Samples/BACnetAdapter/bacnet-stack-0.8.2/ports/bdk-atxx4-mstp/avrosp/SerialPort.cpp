/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : SerialPort.cpp
 * Compiler          : Dev-C++ 4.9.8.0 - http://bloodshed.net/dev/
 * Revision          : $Revision: 1163 $
 * Date              : $Date: 2006-08-02 15:38:16 +0200 (on, 02 aug 2006) $
 * Updated by        : $Author: ohlia $
 *
 * Support mail      : avr@atmel.com
 *
 * Target platform   : Win32
 *
 * AppNote           : AVR911 - AVR Open-source Programmer
 *
 * Description       : A class providing serial communication through the PC COM port.
 *                     This class is derived from the CommChannel abstract class.
 *
 * 
 ****************************************************************************/
#include "SerialPort.hpp"


/* Constructor */
SerialPort::SerialPort( long _portNumber, long _timeout )
{
	if( _timeout < 0 )
		throw new ErrorMsg( "Negative COM-port timeout not allowed!" );

	if( _portNumber < 1 || _portNumber > 99 )
		throw new ErrorMsg( "Only COM1 to COM99 is supported!" );

	/* Initialize internal parameters */
	portNumber = _portNumber;
	timeout = _timeout;
	channelOpen = false;
}


/* Destructor */
SerialPort::~SerialPort()
{
	closeChannel();
}


/* Open the communication channel */
void SerialPort::openChannel()
{
    /* CreateFile expects a constant char, or char from the heap */
	static char comName[64] = "COM1";
    
	COMMTIMEOUTS comTimeouts;

	/* Check if channel already open */
	if( channelOpen )
		throw new ErrorMsg( "Channel already open! Cannot open port twice." );

	/* Generate COM filename and attempt open */
	if (portNumber < 10) {
		comName[3] = '0' + portNumber;
	} else if (portNumber < 100) {
		/* For COM ports greater than 9 you have to use a special syntax
		for CreateFile. The syntax also works for COM ports 1-9. */
		/* http://support.microsoft.com/kb/115831 */
		sprintf(comName, "\\\\.\\COM%ld", portNumber);
	}
	serialHandle = CreateFile( comName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	/* Print error and return if failed opening port */
	if( serialHandle == INVALID_HANDLE_VALUE )
		throw new ErrorMsg( "Error opening COM port!" );

	channelOpen = true;

	/* Store old COM port settings */
	if( !GetCommTimeouts( serialHandle, &oldComTimeouts ) )
		throw new ErrorMsg( "Error reading COM port settings!" );

	/* Get another copy of the COM settings, and change them */
	if( !GetCommTimeouts( serialHandle, &comTimeouts ) )
		throw new ErrorMsg( "Error reading COM port settings!" );

	comTimeouts.ReadIntervalTimeout = MAXDWORD;
	comTimeouts.ReadTotalTimeoutConstant = 0;
	comTimeouts.ReadTotalTimeoutMultiplier = 0;

	/* Apply new settings */
	if( !SetCommTimeouts( serialHandle, &comTimeouts ) )
		throw new ErrorMsg( "Error changing COM port settings!" );
}


/* Close the communication channel */
void SerialPort::closeChannel()
{
	if( !channelOpen )
		return;

	/* Restore old COM parameters */
	if( !SetCommTimeouts( serialHandle, &oldComTimeouts ) )
		throw new ErrorMsg( "Error changing COM port settings!" );

	/* Release port */
	if( serialHandle != INVALID_HANDLE_VALUE )
		if( !CloseHandle( serialHandle ) )
			throw new ErrorMsg( "Error closing COM port!" );

	channelOpen = false;
}



/* Transmit a single byte */
void SerialPort::sendByte( long data )
{
	DWORD written;

	/* Check if channel is open */
	if( !channelOpen )
		throw new ErrorMsg( "Channel not open! Cannot send to unopened channel." );

	/* Attempt writing */
	if( !WriteFile( serialHandle, &data, 1, &written, NULL ) )
		throw new ErrorMsg( "Error writing byte to COM port!" );
}


/* Receive a single byte */
long SerialPort::getByte()
{
	time_t startTime;
	startTime = time( NULL ); // Read current time in seconds
	DWORD readnum;
	unsigned char data;

	/* Check if channel is open */
	if( !channelOpen )
		throw new ErrorMsg( "Channel not open! Cannot read from unopened channel." );

	/* Attempt receiving byte until timeout limit exceeded */
	do
	{
		/* Read byte from port */
		if( !ReadFile( serialHandle, &data, 1, &readnum, NULL ) )
		{
			throw new ErrorMsg( "Error reading byte from COM port!" );
		}

		if( readnum == 1 )
			return ((long) data) & 0xff;

	} while( time(NULL) - startTime < timeout );

	/* Timeout */
	throw new ErrorMsg( "Timeout during COM-port read operation!" );
}


/* Flush the transmit buffer */
void SerialPort::flushTX()
{
	/* Check if channel is open */
	if( !channelOpen )
		throw new ErrorMsg( "Channel not open! Cannot flush an unopened channel." );

	/* Purge data from write buffer */
	if( !PurgeComm( serialHandle, PURGE_TXCLEAR ) )
		throw new ErrorMsg( "Error flushing COM port TX buffer!" );
}


/* Flush the receive buffer */
void SerialPort::flushRX()
{
	/* Check if channel is open */
	if( !channelOpen )
		throw new ErrorMsg( "Channel not open! Cannot flush an unopened channel." );

	/* Purge data from write buffer */
	if( !PurgeComm( serialHandle, PURGE_RXCLEAR ) )
		throw new ErrorMsg( "Error flushing COM port RX buffer!" );
}


/* Transmit multiple bytes */
void SerialPort::sendMultiple( unsigned char * data, long bufsize )
{
	DWORD written;

	/* Check if channel is open */
	if( !channelOpen )
		throw new ErrorMsg( "Channel not open! Cannot write to unopened channel." );

	/* Attempt writing */
	if( !WriteFile( serialHandle, data, bufsize, &written, NULL ) )
		throw new ErrorMsg( "Error writing multiple bytes to COM port!" );
}

/* end of file */

