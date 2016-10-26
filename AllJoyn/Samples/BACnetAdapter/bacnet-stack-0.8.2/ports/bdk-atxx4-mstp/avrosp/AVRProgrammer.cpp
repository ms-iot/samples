/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRProgrammer.cpp
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
 * Description       : An abstract class containing a framework for a generic
 *                     programmer for AVR parts. Reading and writing Flash, EEPROM
 *                     lock bits and all fuse bits and reading OSCCAL and reading
 *                     signature bytes are supported.
 *
 *
 ****************************************************************************/
#include "AVRProgrammer.hpp"


/* Constructor */
AVRProgrammer::AVRProgrammer( CommChannel * _comm ) :
	pagesize( -1 )
{
	if( _comm == NULL )
		throw new ErrorMsg( "NULL pointer provided for communication channel!" );

	comm = _comm;
}


/* Destructor */
AVRProgrammer::~AVRProgrammer()
{
	/* No code here */
}


string AVRProgrammer::readProgrammerID( CommChannel * _comm )
{
	string id( "1234567" ); // Reserve 7 characters.

	if( _comm == NULL )
		throw new ErrorMsg( "NULL pointer provided for communication channel!" );

	/* Synchonize with programmer */
	for( int i = 0; i < 10; i++ )
		_comm->sendByte( 27 ); // Send ESC

	/* Send 'S' command to programmer */
	_comm->sendByte( 'S' );
	_comm->flushTX();

	/* Read 7 characters */
	for( long i = 0; i < id.size(); i++ )
	{
		id[i] = _comm->getByte();
	}

	return id;
}

/* end of file */

