/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRInSystemProg.cpp
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
 * Description       : A class providing an interface to the AVR ISP described
 *                     in Application Note AVR910. This class is derived from AVRPRogrammer.
 *
 * 
 ****************************************************************************/
#include "AVRInSystemProg.hpp"

#include <sstream>
#include <iomanip>

#define MEM_PROGRESS_GRANULARITY 256 // For use with progress indicator.


/* Constructor */
AVRInSystemProg::AVRInSystemProg( CommChannel * _comm ) :
	AVRProgrammer::AVRProgrammer( _comm )
{
	/* No code here */
}


/* Destructor */
AVRInSystemProg::~AVRInSystemProg()
{
	/* No code here */
}


bool AVRInSystemProg::enterProgrammingMode()
{
	/* Must select a device from the AVRISP device code table first */
	comm->sendByte( 'T' );
	comm->sendByte( 0x64 ); // Select ATmega163, any device in the table will do.
	comm->flushTX();

	/* Should return CR */
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Entering programming mode failed! "
				"Programmer did not return CR after 'T'-command." );

	/* Send command 'P' */
	comm->sendByte( 'P' );
	comm->flushTX();

	/* Should return CR */
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Entering programming mode failed! "
				"Programmer did not return CR after 'P'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::leaveProgrammingMode()
{
	/* Send command 'L' */
	comm->sendByte( 'L' );
	comm->flushTX();

	/* Should return CR */
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Leaving programming mode failed! "
				"Programmer did not return CR after 'L'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::chipErase()
{
	/* Send command 'e' */
	comm->sendByte( 'e' );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Chip erase failed! "
				"Programmer did not return CR after 'e'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::readOSCCAL( long pos, long * value )
{
	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0x38 );
	comm->sendByte( 0x00 );
	comm->sendByte( pos );
	comm->sendByte( 0x00 ); // Dummy.
	comm->flushTX();

	*value = comm->getByte();

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "OSCCAL value readout failed! "
				"Programmer did not return CR after '.'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::readSignature( long * sig0, long * sig1, long * sig2 )
{
	/* Send command 's' */
	comm->sendByte( 's' );
	comm->flushTX();

	/* Get actual signature */
	*sig2 = comm->getByte();
	*sig1 = comm->getByte();
	*sig0 = comm->getByte();
}


bool AVRInSystemProg::checkSignature( long sig0, long sig1, long sig2 )
{
	long sig[3];

	/* Get signature */
	readSignature( sig, sig+1, sig+2 );

	/* Compare signature */
	if( sig[0] != sig0 || sig[1] != sig1 || sig[2] != sig2 )
	{
		ostringstream msg;
		msg << "Signature does not match selected device! ";
		msg << "Actual signature: (" << hex
			<< "0x" << setw(2) << sig[0] << " "
			<< "0x" << setw(2) << sig[1] << " "
			<< "0x" << setw(2) << sig[2] << ") "
			<< "Signature from XML-file: (" << hex
			<< "0x" << setw(2) << sig0 << " "
			<< "0x" << setw(2) << sig1 << " "
			<< "0x" << setw(2) << sig2 << ").";

		throw new ErrorMsg( msg.str() );
	}

	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeFlashByte( long address, long value )
{
	if( address >= 0x20000 )
		throw new ErrorMsg( "Flash addresses above 128k are currently not supported!" );

	setAddress( address >> 1 ); // Flash operations use word addresses.

	/* Move data if at odd address */
	if( address & 0x01 ) // Odd address?
		value = (value << 8) | 0x00ff; // Move to high byte of one flash word.
	else
		value |= 0xff00; // Ensure no-write for high byte.

	/* Send low and high byte */
	writeFlashLowByte( value & 0xff );
	writeFlashHighByte( value >> 8 );

	/* Issue page write if required */
	if( pagesize != -1 )
	{
		setAddress( address >> 1 ); // The address could be autoincremented.
		writeFlashPage();
	}

	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeEEPROMByte( long address, long value )
{
	if( address >= 0x10000 )
		throw new ErrorMsg( "EEPROM addresses above 64k are currently not supported!" );

	setAddress( address );

	/* Send data */
	comm->sendByte( 'D' );
	comm->sendByte( value );
	comm->flushTX();

	/* Should return CR */
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing byte to EEPROM failed! "
				"Programmer did not return CR after 'D'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeFlash( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	/* Check that pagesize is set */
	if( pagesize == -1 )
		throw new ErrorMsg( "Programmer pagesize is not set!" );

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Check autoincrement support */
	comm->sendByte( 'a' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
		autoincrement = true;
	else
		autoincrement = false;

	/* Set initial address */
	setAddress( start >> 1 ); // Flash operations use word addresses.

	/* Need to write one odd byte first? */
	address = start;
	if( address & 1 )
	{
		/* Use only high byte */
		writeFlashLowByte( 0xff ); // No-write in low byte.
		writeFlashHighByte( data->getData( address ) );

		address++;

		/* Need to write page? */
		if( pagesize != -1 )
		{
			if( !(address % pagesize) ) // Just passed page limit?
			{
				setAddress( (address-1) >> 1 ); // Set to an address inside the page.
				writeFlashPage();
				setAddress( address >> 1 );
			}
		}
	}

	/* Write words */
	do
	{
		/* Need to set address again? */
		if( !autoincrement )
			setAddress( address >> 1 );

		/* Write words */
		writeFlashLowByte( data->getData( address ) );
		writeFlashHighByte( data->getData( address+1 ) );

		address += 2;

		if( (address % MEM_PROGRESS_GRANULARITY) == 0 )
			Util.progress( "#" ); // Advance progress indicator.

		/* Need to write page? */
		if( pagesize != -1 )
		{
			if( (address % pagesize) == 0 ) // Just passed a page boundary?
			{
				setAddress( (address-2) >> 1 ); // Set to an address inside the page.
				writeFlashPage();
				setAddress( address >> 1 );
			}
		}
	} while( address < end );

	/* Need to write one even byte before finished? */
	if( address == end )
	{
		/* Use only low byte */
		writeFlashLowByte( data->getData( address ) );
		writeFlashHighByte( 0xff ); // No-write in high byte.
	}

	/* Need to write page? */
	if( pagesize != -1 )
	{
		if( address == end || // One extra byte written...
				(end+1)%pagesize != 0 ) // ...or end is not on page boundary.
		{    
			setAddress( (address-2) >> 1 ); // Set to an address inside the page.
			writeFlashPage();
		}
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRInSystemProg::readFlash( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	if( pagesize == -1 )
		throw new ErrorMsg( "Programmer pagesize is not set!" );

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Check autoincrement support */
	comm->sendByte( 'a' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
		autoincrement = true;
	else
		autoincrement = false;

	/* Set initial address */
	setAddress( start >> 1 ); // Flash operations use word addresses.

	/* Need to read one odd byte first? */
	address = start;
	if( address & 1 )
	{
		/* Read both, but use only high byte */
		comm->sendByte( 'R' );
		comm->flushTX();

		data->setData( address, comm->getByte() ); // High byte.
		comm->getByte(); // Dont use low byte.

		address++;
	}

	/* Get words */
	do
	{
		/* Need to set address again? */
		if( !autoincrement )
			setAddress( address >> 1 );

		/* Get words */
		comm->sendByte( 'R' );
		comm->flushTX();

		data->setData( address+1, comm->getByte() ); // High byte.
		data->setData( address, comm->getByte() ); // Low byte.

		address += 2;

		if( (address % MEM_PROGRESS_GRANULARITY) == 0 )
			Util.progress( "#" ); // Advance progress indicator.

	} while( address < end );

	/* Need to read one even byte before finished? */
	if( address == end )
	{
		/* Read both, but use only low byte */
		comm->sendByte( 'R' );
		comm->flushTX();

		comm->getByte(); // Dont use high byte.
		data->setData( address-1, comm->getByte() ); // Low byte.
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeEEPROM( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Check autoincrement support */
	comm->sendByte( 'a' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
		autoincrement = true;
	else
		autoincrement = false;

	/* Set initial address */
	setAddress( start );

	/* Send data */
	address = start;
	do
	{
		/* Need to set address again? */
		if( !autoincrement )
			setAddress( address );

		/* Send byte */
		comm->sendByte( 'D' );
		comm->sendByte( data->getData( address ) );
		comm->flushTX();

		if( comm->getByte() != '\r' )
			throw new ErrorMsg( "Writing byte to EEPROM failed! "
					"Programmer did not return CR after 'D'-command." );

		if( (address % MEM_PROGRESS_GRANULARITY) == 0 )
			Util.progress( "#" ); // Advance progress indicator.

		address++;
	} while( address <= end );

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRInSystemProg::readEEPROM( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Check autoincrement support */
	comm->sendByte( 'a' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
		autoincrement = true;
	else
		autoincrement = false;

	/* Set initial address */
	setAddress( start );

	/* Send data */
	address = start;
	do
	{
		/* Need to set address again? */
		if( !autoincrement )
			setAddress( address );

		/* Get byte */
		comm->sendByte( 'd' );
		comm->flushTX();

		data->setData( address, comm->getByte() );		

		if( (address % MEM_PROGRESS_GRANULARITY) == 0 )
			Util.progress( "#" ); // Advance progress indicator.

		address++;
	} while( address <= end );

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeLockBits( long bits )
{
	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0xac );
	comm->sendByte( 0xe0 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( bits );

	comm->flushTX();
	comm->getByte(); // Ignore return code from SPI communication.

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Writing lock bits failed! "
				"Programmer did not return CR after '.'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::readLockBits( long * bits )
{
	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0x58 );
	comm->sendByte( 0x00 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( 0x00 ); // Dummy.
	comm->flushTX();

	*bits = comm->getByte();

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Lock byte readout failed! "
				"Programmer did not return CR after '.'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeFuseBits( long bits )
{
	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0xac );
	comm->sendByte( 0xa0 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( bits & 0xff );
	comm->flushTX();

	comm->getByte(); // Ignore return code from SPI communication.

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Low fuse byte programming failed! "
				"Programmer did not return CR after '.'-command." );

	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0xac );
	comm->sendByte( 0xa8 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( bits >> 8 );
	comm->flushTX();

	comm->getByte(); // Ignore return code from SPI communication.

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "High fuse byte programming failed! "
				"Programmer did not return CR after '.'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::readFuseBits( long * bits )
{
	long low, high;

	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0x50 );
	comm->sendByte( 0x00 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( 0x00 ); // Dummy.
	comm->flushTX();

	low = comm->getByte();

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Low fuse byte readout failed! "
				"Programmer did not return CR after '.'-command." );

	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0x58 );
	comm->sendByte( 0x08 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( 0x00 ); // Dummy.
	comm->flushTX();

	high = comm->getByte();

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Low fuse byte readout failed! "
				"Programmer did not return CR adter '.'-command." );

	/* Put low and high together */
	*bits = (high << 8) | low;

	return true; // Indicate supported command.
}


bool AVRInSystemProg::writeExtendedFuseBits( long bits )
{
	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0xac );
	comm->sendByte( 0xa4 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( bits );
	comm->flushTX();

	comm->getByte(); // Ignore return code from SPI communication.

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Extended fuse byte programming failed! "
				"Programmer did not return CR after '.'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::readExtendedFuseBits( long * bits )
{
	/* Use AVRISP's 4-byte universal command */
	comm->sendByte( '.' );
	comm->sendByte( 0x50 );
	comm->sendByte( 0x08 );
	comm->sendByte( 0x00 ); // Dummy.
	comm->sendByte( 0x00 ); // Dummy.
	comm->flushTX();

	*bits = comm->getByte();

	if( comm->getByte() != '\r' ) // Check return code from command.
		throw new ErrorMsg( "Extended fuse byte readout failed! "
				"Programmer did not return CR after '.'-command." );

	return true; // Indicate supported command.
}


bool AVRInSystemProg::programmerSoftwareVersion( long * major, long * minor )
{
	/* Send command 'V' to get software version */
	comm->sendByte( 'V' );
	comm->flushTX();

	/* Get data */
	*major = comm->getByte();
	*minor = comm->getByte();

	return true; // Indicate supported command.
}


bool AVRInSystemProg::programmerHardwareVersion( long * major, long * minor )
{
	/* Send command 'v' to get hardware version */
	comm->sendByte( 'v' );
	comm->flushTX();

	/* Get data */
	*major = comm->getByte();
	*minor = comm->getByte();

	return true; // Indicate supported command.
}


void AVRInSystemProg::setAddress( long address )
{
	/* Set current address */
	comm->sendByte( 'A' );
	comm->sendByte( address >> 8 ); // High byte of address.
	comm->sendByte( address & 0xff ); // Low byte.
	comm->flushTX();

	/* Should return CR */	
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Setting address for programming operations failed! "
				"Programmer did not return CR after 'A'-command." );
}


void AVRInSystemProg::writeFlashLowByte( long value )
{
	comm->sendByte( 'c' );
	comm->sendByte( value );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing Flash low byte failed! "
				"Programmer did not return CR after 'c'-command." );
}


void AVRInSystemProg::writeFlashHighByte( long value )
{
	comm->sendByte( 'C' );
	comm->sendByte( value );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing Flash high byte failed! "
				"Programmer did not return CR after 'C'-command." );
}


void AVRInSystemProg::writeFlashPage()
{
	comm->sendByte( 'm' );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing Flash page failed! "
				"Programmer did not return CR after 'm'-command." );
}


/* end of file */

