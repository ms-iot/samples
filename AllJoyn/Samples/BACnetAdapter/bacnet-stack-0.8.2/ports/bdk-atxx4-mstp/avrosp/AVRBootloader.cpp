/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRBootloader.cpp
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
 * Description       : A class providing an interface to the AVR bootloader
 *                     described in Application Note AVR109.
 *                     This class is derived from AVRPRogrammer.
 *
 *
 ****************************************************************************/
#include "AVRBootloader.hpp"

#include <sstream>
#include <iomanip>

#define MEM_PROGRESS_GRANULARITY 256 // For use with progress indicator.


/* Constructor */
AVRBootloader::AVRBootloader( CommChannel * _comm ) :
	AVRProgrammer::AVRProgrammer( _comm )
{
	/* No code here */
}


/* Destructor */
AVRBootloader::~AVRBootloader()
{
	/* No code here */
}


bool AVRBootloader::enterProgrammingMode()
{
	return true; // Always OK for bootloader.
}


bool AVRBootloader::leaveProgrammingMode()
{
	return true; // Always OK for bootloader.
}


bool AVRBootloader::chipErase()
{
	/* Send command 'e' */
	comm->sendByte( 'e' );
	comm->flushTX();

	/* Should return CR */
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Chip erase failed! "
				"Programmer did not return CR after 'e'-command." );

	return true; // Indicate supported command.
}


bool AVRBootloader::readOSCCAL( long pos, long * value )
{
	return false; // Indicate unsupported command.
}


bool AVRBootloader::readSignature( long * sig0, long * sig1, long * sig2 )
{
	/* Send command 's' */
	comm->sendByte( 's' );
	comm->flushTX();

	/* Get actual signature */
	*sig2 = comm->getByte();
	*sig1 = comm->getByte();
	*sig0 = comm->getByte();
}


bool AVRBootloader::checkSignature( long sig0, long sig1, long sig2 )
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


bool AVRBootloader::writeFlashByte( long address, long value )
{
	setAddress( address >> 1 ); // Flash operations use word addresses.

	/* Move data if at odd address */
	if( address & 0x01 ) // Odd address?
		value = (value << 8) | 0x00ff; // Move to high byte of one flash word.
	else
		value |= 0xff00; // Ensure no-write for high byte.

	/* Send low and high byte */
	writeFlashLowByte( value & 0xff );
	writeFlashHighByte( value >> 8 );

	/* Issue page write */
	setAddress( address >> 1 ); // The address could be autoincremented.
	writeFlashPage();

	return true; // Indicate supported command.
}


bool AVRBootloader::writeEEPROMByte( long address, long value )
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


bool AVRBootloader::writeFlash( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	/* Check that pagesize is set */
	if( pagesize == -1 )
		throw new ErrorMsg( "Programmer pagesize is not set!" );

	/* Check block write support */
	comm->sendByte( 'b' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
	{
		Util.log( "Using block mode...\r\n" );
		return writeFlashBlock( data ); // Finished writing.
	}

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
		if( (address % pagesize) == 0 ||
				address > end ) // Just passed page limit or no more bytes to write?
		{
			setAddress( (address-2) >> 1 ); // Set to an address inside the page.
			writeFlashPage();
			setAddress( address >> 1 );
		}
	}

	/* Write words */
	while( (end-address+1) >= 2 ) // More words left?
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
		if( (address % pagesize) == 0 ||
				address > end ) // Just passed a page limit or no more bytes to write?
		{
			setAddress( (address-2) >> 1 ); // Set to an address inside the page.
			writeFlashPage();
			setAddress( address >> 1 );
		}
	}

	/* Need to write one even byte before finished? */
	if( address == end )
	{
		/* Use only low byte */
		writeFlashLowByte( data->getData( address ) );
		writeFlashHighByte( 0xff ); // No-write in high byte.
		address+=2;

		/* Write page */
		setAddress( (address-2) >> 1 ); // Set to an address inside the page.
		writeFlashPage();
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRBootloader::writeFlashBlock( HEXFile * data )
{
	long start, end; // Data address range.
	long blocksize; // Bootloader block size.
	long bytecount;
	long address;

	/* Get block size, assuming command 'b' just issued and 'Y' has been read */
	blocksize = (comm->getByte() << 8) | comm->getByte();

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Need to write one odd byte first? */
	address = start;
	if( address & 1 )
	{
		setAddress( address >> 1 ); // Flash operations use word addresses.

		/* Use only high byte */
		writeFlashLowByte( 0xff ); // No-write in low byte.
		writeFlashHighByte( data->getData( address ) );
		address++;

		/* Need to write page? */
		if( (address % pagesize) == 0 ||
				address > end ) // Just passed page limit or no more bytes to write?
		{
			setAddress( (address-2) >> 1 ); // Set to an address inside the page.
			writeFlashPage();
			setAddress( address >> 1 );
		}
	}

	/* Need to write from middle to end of block first? */
	if( (address % blocksize) > 0 ) // In the middle of a block?
	{
		bytecount = blocksize - (address % blocksize); // Bytes left in block.

		if( (address+bytecount-1) > end ) // Is that past the write range?
		{
			bytecount = end-address+1; // Bytes left in write range.
			bytecount &= ~0x01; // Adjust to word count.
		}

		if( bytecount > 0 )
		{
			setAddress( address >> 1 ); // Flash operations use word addresses.

			/* Start Flash block write */
			comm->sendByte( 'B' );
			comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
			comm->sendByte( bytecount & 0xff );
			comm->sendByte( 'F' ); // Flash memory.

			while( bytecount > 0 )
			{
				comm->sendByte( data->getData( address ) );
				address++;
				bytecount--;
			}

			if( comm->getByte() != '\r' )
				throw new ErrorMsg( "Writing Flash block failed! "
						"Programmer did not return CR after 'BxxF'-command." );

			Util.progress( "#" ); // Advance progress indicator.
		}
	}

	/* More complete blocks to write? */
	while( (end-address+1) >= blocksize )
	{
		bytecount = blocksize;

		setAddress( address >> 1 ); // Flash operations use word addresses.

		/* Start Flash block write */
		comm->sendByte( 'B' );
		comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
		comm->sendByte( bytecount & 0xff );
		comm->sendByte( 'F' ); // Flash memory.

		while( bytecount > 0 )
		{
			comm->sendByte( data->getData( address ) );
			address++;
			bytecount--;
		}

		if( comm->getByte() != '\r' )
			throw new ErrorMsg( "Writing Flash block failed! "
					"Programmer did not return CR after 'BxxF'-command." );

		Util.progress( "#" ); // Advance progress indicator.
	}

	/* Any bytes left in last block */
	if( (end-address+1) >= 1 )
	{
		bytecount = (end-address+1); // Get bytes left to write.
		if( bytecount & 1 )
			bytecount++; // Align to next word boundary.

		setAddress( address >> 1 ); // Flash operations use word addresses.

		/* Start Flash block write */
		comm->sendByte( 'B' );
		comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
		comm->sendByte( bytecount & 0xff );
		comm->sendByte( 'F' ); // Flash memory.

		while( bytecount > 0 )
		{
			if( address > end )
				comm->sendByte( 0xff ); // Don't write outside write range.
			else
				comm->sendByte( data->getData( address ) );

			address++;
			bytecount--;
		}

		if( comm->getByte() != '\r' )
			throw new ErrorMsg( "Writing Flash block failed! "
					"Programmer did not return CR after 'BxxF'-command." );

		Util.progress( "#" ); // Advance progress indicator.
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRBootloader::readFlash( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	if( pagesize == -1 )
		throw new ErrorMsg( "Programmer pagesize is not set!" );

	/* Check block read support */
	comm->sendByte( 'b' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
	{
		Util.log( "Using block mode...\r\n" );
		return readFlashBlock( data ); // Finished writing.
	}

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
	while( (end-address+1) >= 2 )
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

	};

	/* Need to read one even byte before finished? */
	if( address == end )
	{
		/* Read both, but use only low byte */
		comm->sendByte( 'R' );
		comm->flushTX();

		comm->getByte(); // Dont use high byte.
		data->setData( address, comm->getByte() ); // Low byte.
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRBootloader::readFlashBlock( HEXFile * data )
{
	long start, end; // Data address range.
	long blocksize; // Bootloader block size.
	long bytecount;
	long address;

	/* Get block size, assuming command 'b' just issued and 'Y' has been read */
	blocksize = (comm->getByte() << 8) | comm->getByte();

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Need to read one odd byte first? */
	address = start;
	if( address & 1 )
	{
		setAddress( address >> 1 ); // Flash operations use word addresses.

		/* Use only high word */
		comm->sendByte( 'R' );
		comm->flushTX();

		data->setData( address, comm->getByte() ); // High byte.
		comm->getByte(); // Low byte.
		address++;
	}

	/* Need to read from middle to end of block first? */
	if( (address % blocksize) > 0 ) // In the middle of a block?
	{
		bytecount = blocksize - (address % blocksize); // Bytes left in block.

		if( (address+bytecount-1) > end ) // Is that past the read range?
		{
			bytecount = end-address+1; // Bytes left in read range.
			bytecount &= ~0x01; // Adjust to word count.
		}

		if( bytecount > 0 )
		{
			setAddress( address >> 1 ); // Flash operations use word addresses.

			/* Start Flash block read */
			comm->sendByte( 'g' );
			comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
			comm->sendByte( bytecount & 0xff );
			comm->sendByte( 'F' ); // Flash memory.

			while( bytecount > 0 )
			{
				data->setData( address, comm->getByte() );
				address++;
				bytecount--;
			}

			Util.progress( "#" ); // Advance progress indicator.
		}
	}

	/* More complete blocks to read? */
	while( (end-address+1) >= blocksize )
	{
		bytecount = blocksize;

		setAddress( address >> 1 ); // Flash operations use word addresses.

		/* Start Flash block read */
		comm->sendByte( 'g' );
		comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
		comm->sendByte( bytecount & 0xff );
		comm->sendByte( 'F' ); // Flash memory.

		while( bytecount > 0 )
		{
			data->setData( address, comm->getByte() );
			address++;
			bytecount--;
		}

		Util.progress( "#" ); // Advance progress indicator.
	}

	/* Any bytes left in last block */
	if( (end-address+1) >= 1 )
	{
		bytecount = (end-address+1); // Get bytes left to read.
		if( bytecount & 1 )
			bytecount++; // Align to next word boundary.

		setAddress( address >> 1 ); // Flash operations use word addresses.

		/* Start Flash block read */
		comm->sendByte( 'g' );
		comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
		comm->sendByte( bytecount & 0xff );
		comm->sendByte( 'F' ); // Flash memory.

		while( bytecount > 0 )
		{
			if( address > end )
				comm->getByte(); // Don't read outside write range.
			else
				data->setData( address, comm->getByte() );

			address++;
			bytecount--;
		}

		Util.progress( "#" ); // Advance progress indicator.
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRBootloader::writeEEPROM( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	/* Check block write support */
	comm->sendByte( 'b' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
	{
		Util.log( "Using block mode...\r\n" );
		return writeEEPROMBlock( data ); // Finished writing.
	}

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


bool AVRBootloader::writeEEPROMBlock( HEXFile * data )
{
	long start, end; // Data address range.
	long blocksize; // Bootloader block size.
	long bytecount;
	long address;

	/* Get block size, assuming command 'b' just issued and 'Y' has been read */
	blocksize = (comm->getByte() << 8) | comm->getByte();

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Send data */
	address = start;
	while( address <= end ) // More bytes to write?
	{
		bytecount = blocksize; // Try a full block.

		if( (address+bytecount-1) > end ) // Is that past the write range?
		{
			bytecount = end-address+1; // Bytes left in write range.
		}

		setAddress( address );

		/* Start EEPROM block write */
		comm->sendByte( 'B' );
		comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
		comm->sendByte( bytecount & 0xff );
		comm->sendByte( 'E' ); // EEPROM memory.

		while( bytecount > 0 )
		{
			comm->sendByte( data->getData( address ) );
			comm->flushTX();

			address++;
			bytecount--;
		}

		if( comm->getByte() != '\r' )
			throw new ErrorMsg( "Writing EEPROM block failed! "
					"Programmer did not return CR after 'BxxE'-command." );

		Util.progress( "#" ); // Advance progress indicator.
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRBootloader::readEEPROM( HEXFile * data )
{
	long start, end; // Data address range.
	bool autoincrement; // Bootloader supports address autoincrement?
	long address;

	/* Check block write support */
	comm->sendByte( 'b' );
	comm->flushTX();

	if( comm->getByte() == 'Y' )
	{
		Util.log( "Using block mode...\r\n" );
		return readEEPROMBlock( data ); // Finished writing.
	}

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

	/* Read data */
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


bool AVRBootloader::readEEPROMBlock( HEXFile * data )
{
	long start, end; // Data address range.
	long blocksize; // Bootloader block size.
	long bytecount;
	long address;

	/* Get block size, assuming command 'b' just issued and 'Y' has been read */
	blocksize = (comm->getByte() << 8) | comm->getByte();

	/* Get range from HEX file */
	start = data->getRangeStart();
	end = data->getRangeEnd();

	/* Read data */
	address = start;
	while( address <= end ) // More bytes to read?
	{
		bytecount = blocksize; // Try a full block.

		if( (address+bytecount-1) > end ) // Is that past the read range?
		{
			bytecount = end-address+1; // Bytes left in read range.
		}

		setAddress( address );

		/* Start EEPROM block read */
		comm->sendByte( 'g' );
		comm->sendByte( (bytecount>>8) & 0xff ); // Size, MSB first.
		comm->sendByte( bytecount & 0xff );
		comm->sendByte( 'E' ); // EEPROM memory.

		while( bytecount > 0 )
		{
			data->setData( address, comm->getByte() );
			address++;
			bytecount--;
		}

		Util.progress( "#" ); // Advance progress indicator.
	}

	Util.progress( "\r\n" ); // Finish progress indicator.
	return true; // Indicate supported command.
}


bool AVRBootloader::writeLockBits( long bits )
{
	/* Send command 'l' */
	comm->sendByte( 'l' );
	comm->sendByte( bits & 0xff );
	comm->flushTX();

	/* Should return CR */
	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing lock bits failed! "
				"Programmer did not return CR after 'l'-command." );

	return true; // Indicate supported command.
}


bool AVRBootloader::readLockBits( long * bits )
{
	/* Send command 'r' */
	comm->sendByte( 'r' );
	comm->flushTX();

	/* Get data */
	*bits = comm->getByte();

	return true; // Indicate supported command.
}


bool AVRBootloader::writeFuseBits( long bits )
{
	return false; // Indicate unsupported command.
}


bool AVRBootloader::readFuseBits( long * bits )
{
	long lowfuse, highfuse;

	/* Send command 'N' */
	comm->sendByte( 'N' );
	comm->flushTX();

	/* Get high fuse bits */
	highfuse = comm->getByte();

	/* Send command 'F' */
	comm->sendByte( 'F' );
	comm->flushTX();

	/* Get low fuse bits */
	lowfuse = comm->getByte();

	*bits = (highfuse << 8) | lowfuse;

	return true; // Indicate supported command.
}


bool AVRBootloader::writeExtendedFuseBits( long bits )
{
	return false; // Indicate unsupported command.
}


bool AVRBootloader::readExtendedFuseBits( long * bits )
{
	/* Send command 'Q' */
	comm->sendByte( 'Q' );
	comm->flushTX();

	/* Get data */
	*bits = comm->getByte();

	return true; // Indicate supported command.
}


bool AVRBootloader::programmerSoftwareVersion( long * major, long * minor )
{
	/* Send command 'V' to get software version */
	comm->sendByte( 'V' );
	comm->flushTX();

	/* Get data */
	*major = comm->getByte();
	*minor = comm->getByte();

	return true; // Indicate supported command.
}


bool AVRBootloader::programmerHardwareVersion( long * major, long * minor )
{
	return false; // Indicate unsupported command.
}


void AVRBootloader::setAddress( long address )
{
	/* Set current address */
	if( address < 0x10000 ) {
		comm->sendByte( 'A' );
		comm->sendByte( (address >> 8) & 0xff );
		comm->sendByte( address & 0xff );
		comm->flushTX();
	} else {
		comm->sendByte( 'H' );
		comm->sendByte( (address >> 16) & 0xff );
		comm->sendByte( (address >> 8) & 0xff );
		comm->sendByte( address & 0xff );
		comm->flushTX();
	}

	/* Should return CR */
	if( comm->getByte() != '\r' ) {
		throw new ErrorMsg( "Setting address for programming operations failed! "
				"Programmer did not return CR after 'A'-command." );
	}
}


void AVRBootloader::writeFlashLowByte( long value )
{
	comm->sendByte( 'c' );
	comm->sendByte( value );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing Flash low byte failed! "
				"Programmer did not return CR after 'c'-command." );
}


void AVRBootloader::writeFlashHighByte( long value )
{
	comm->sendByte( 'C' );
	comm->sendByte( value );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing Flash high byte failed! "
				"Programmer did not return CR after 'C'-command." );
}


void AVRBootloader::writeFlashPage()
{
	comm->sendByte( 'm' );
	comm->flushTX();

	if( comm->getByte() != '\r' )
		throw new ErrorMsg( "Writing Flash page failed! "
				"Programmer did not return CR after 'm'-command." );
}


/* end of file */

