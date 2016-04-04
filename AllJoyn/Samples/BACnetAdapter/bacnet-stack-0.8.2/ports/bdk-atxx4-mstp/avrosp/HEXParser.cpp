/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : HEXParser.cpp
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
 * Description       : A simple Intel HEX file format reader/writer.
 *
 * 
 ****************************************************************************/
#include "HEXParser.hpp"


/* Internal struct for managing HEX records */
struct HEXRecord // Intel HEX file record
{
	unsigned char length; // Record length in number of data bytes.
	unsigned long offset; // Offset address.
	unsigned char type; // Record type.
	unsigned char * data; // Optional data bytes.
};


void HEXFile::writeRecord( ofstream & f, HEXRecord * recp )
{
	unsigned char checksum;
	long recordPos; // Position inside record data field

	/* Calculate checksum */
	checksum = recp->length;
	checksum += (unsigned char) ((recp->offset >> 8) & 0xff);
	checksum += (unsigned char) (recp->offset & 0xff);
	checksum += recp->type;

	/* Write record header */
	f.fill('0');
	f << ":" << hex
		<< setw(2) << (long) recp->length
		<< setw(4) << (long) recp->offset
		<< setw(2) << (long) recp->type;

	/* Write data bytes */
	for( recordPos = 0; recordPos < recp->length; recordPos++ )
	{
		checksum += recp->data[ recordPos ]; // Further checksum calculation
		f << hex << setw(2) << (long) recp->data[ recordPos ];
	}

	/* Write checksum */
	checksum = 0 - checksum; // Final checksum preparation
	f << setw(2) << (long) checksum << endl;

	/* Check for errors */
	if( !f.good() )
		throw new ErrorMsg( "Error writing HEX record to file!" );
}


void HEXFile::parseRecord( const string & hexLine, HEXRecord * recp )
{
	unsigned char checksum;
	long recordPos; // Position inside record data fields.

	if( hexLine.size() < 11 ) // At least 11 characters.
		throw new ErrorMsg( "Wrong HEX file format, missing fields! "
				"Line from file was: (" + hexLine + ")." );

	/* Check format for line */
	if( hexLine[0] != ':' ) // Always start with colon.
		throw new ErrorMsg( "Wrong HEX file format, does not start with colon! "
				"Line from file was: (" + hexLine + ")." );

	/* Parse length, offset and type */
	recp->length = Util.convertHex( hexLine.substr( 1, 2 ) );
	recp->offset = Util.convertHex( hexLine.substr( 3, 4 ) );
	recp->type = Util.convertHex( hexLine.substr( 7, 2 ) );

	/* We now know how long the record should be */
	if( hexLine.size() < (11+recp->length*2) )
		throw new ErrorMsg( "Wrong HEX file format, missing fields! "
				"Line from file was: (" + hexLine + ")." );

	/* Process checksum */
	checksum = recp->length;
	checksum += (unsigned char) ((recp->offset >> 8) & 0xff);
	checksum += (unsigned char) (recp->offset & 0xff);
	checksum += recp->type;

	/* Parse data fields */
	if( recp->length )
	{
		recp->data = new unsigned char[ recp->length ];

		/* Read data from record */
		for( recordPos = 0; recordPos < recp->length; recordPos++ )
		{
			recp->data[ recordPos ] = Util.convertHex( hexLine.substr( 9 + recordPos*2, 2 ) );
			checksum += recp->data[ recordPos ];
		}
	}

	/* Correct checksum? */
	checksum += Util.convertHex( hexLine.substr( 9 + recp->length*2, 2 ) );
	if( checksum != 0 )
	{
		throw new ErrorMsg( "Wrong checksum for HEX record! "
				"Line from file was: (" + hexLine + ")." );
	}
}



/* Constructor */
HEXFile::HEXFile( long buffersize, long value )
{
	if( buffersize <= 0 )
		throw new ErrorMsg( "Cannot have zero-size HEX buffer!" );

	data = new unsigned char[ buffersize ];

	if( !data )
		throw new ErrorMsg( "Memory allocation failed for HEX-line-buffer!" );

	size = buffersize;

	clearAll( value );
}


/* Destructor */
HEXFile::~HEXFile()
{
	if( data ) delete data;
}


void HEXFile::readFile( const string & _filename )
{
	ifstream f;
	string hexLine; // Contains one line of the HEX file.
	HEXRecord rec; // Temp record.

	long baseAddress; // Base address for extended addressing modes.
	long dataPos; // Data position in record.

	/* Attempt to open file */
	f.open( _filename.c_str(), ios::in );
	if( !f )
		throw new ErrorMsg( "Error opening HEX file for input!" );

	/* Prepare */
	baseAddress = 0;
	start = size;
	end = 0;

	/* Parse records */
	f >> hexLine; // Read one line.
	while( !f.eof() )
	{
		Util.progress( "#" ); // Advance progress indicator.

		/* Process record according to type */
		parseRecord( hexLine, &rec );

		switch( rec.type )
		{
			case 0x00 : // Data record ?
				/* Copy data */
				if( baseAddress + rec.offset + rec.length > size )
					throw new ErrorMsg( "HEX file defines data outside buffer limits! "
							"Make sure file does not contain data outside device "
							"memory limits. "
							"Line from file was: (" + hexLine + ")." );

				for( dataPos = 0; dataPos < rec.length; dataPos++ )
					data[ baseAddress + rec.offset + dataPos ] = rec.data[ dataPos ];

				/* Update byte usage */
				if( baseAddress + rec.offset < start )
					start = baseAddress + rec.offset;

				if( baseAddress + rec.offset + rec.length - 1 > end )
					end = baseAddress + rec.offset + rec.length - 1;

				break;


			case 0x02 : // Extended segment address record ?
				baseAddress = (rec.data[0] << 8) | rec.data[1];
				baseAddress <<= 4;
				break;

			case 0x03 : // Start segment address record ?
				break; // Ignore it, since we have no influence on execution start address.

			case 0x04 : // Extended linear address record ?
				baseAddress = (rec.data[0] << 8) | rec.data[1];
				baseAddress <<= 16;
				break;

			case 0x05 : // Start linear address record ?
				break; // Ignore it, since we have no influence on exectuion start address.

			case 0x01 : // End of file record ?
				f.close();
				Util.progress( "\r\n" ); // Finish progress indicator.
				return;

			default:
				throw new ErrorMsg( "Unsupported HEX record format! "
						"Line from file was: (" + hexLine + ")." );
		}

		f >> hexLine; // Read next line.
	}


	/* We should not end up here */
	throw new ErrorMsg( "Premature end of file encountered! Make sure file "
			"contains an EOF-record." );
}


void HEXFile::writeFile( const string & _filename )
{
	ofstream f;
	HEXRecord rec; // Temp record.

	long baseAddress; // Absolute data position.
	long offset; // Offset from base address.
	long dataPos; // Position inside data record.

	enum
	{ 
		_first,
		_writing,
		_passed64k
	} status; // Write status, see usage below.

	/* Attempt to create file */
	f.open( _filename.c_str(), ios::out );
	if( !f )
		throw new ErrorMsg( "Error opening HEX file for output!" );

	/* Prepare */	
	status = _first;
	rec.data = new unsigned char[ 16 ]; // Use only 16 byte records.

	baseAddress = start & ~0xffff; // 64K aligned address.
	offset = start & 0xffff; // Offset from the aligned address.
	dataPos = 0;

	/* Write first base address record to HEX file */
	rec.length = 2;
	rec.offset = 0;
	rec.type = 0x02;
	rec.data[1] = 0x00;
	rec.data[0] = baseAddress >> 12; // Give 4k page index.
	writeRecord( f, &rec ); // Write the HEX record to file.


	/* Write all bytes in used range */
	do
	{
		/* Put data into record */
		rec.data[ dataPos ] = data[ baseAddress + offset + dataPos ];
		dataPos++;

		/* Check if we need to write out the current data record */
		if( offset + dataPos >= 0x10000 || // Reached 64k boundary?
				dataPos >= 16 || // Data record full?
				baseAddress + offset + dataPos > end ) // End of used range reached?
		{
			/* Write current data record */
			rec.length = dataPos;
			rec.offset = offset;
			rec.type = 0x00; // Data record.

			Util.progress( "#" ); // Advance progress indicator.	
			writeRecord( f, &rec );

			offset += dataPos;
			dataPos = 0;
		}    

		/* Check if we have passed a 64k boundary */
		if( offset + dataPos >= 0x10000 )
		{
			/* Update address pointers */
			offset -= 0x10000;
			baseAddress += 0x10000;

			/* Write new base address record to HEX file */
			rec.length = 2;
			rec.offset = 0;
			rec.type = 0x02;
			rec.data[0] = baseAddress >> 12; // Give 4k page index.
			rec.data[1] = 0x00;

			writeRecord( f, &rec ); // Write the HEX record to file.
		}
	} while( baseAddress + offset + dataPos <= end );


	/* Write EOF record */
	rec.length = 0;
	rec.offset = 0;
	rec.type = 0x01;

	writeRecord( f, &rec );

	f.close();
	Util.progress( "\r\n" ); // Finish progress indicator.
}


void HEXFile::setUsedRange( long _start, long _end )
{
	if( _start < 0 || _end >= size || _start > _end )
		throw new ErrorMsg( "Invalid range! Start must be 0 or larger, end must be "
				"inside allowed memory range." );

	start = _start;
	end = _end;
}


void HEXFile::clearAll( long value )
{
	for( long i = 0; i < size; i++ )
		data[i] = (unsigned char) (value & 0xff);
}


long HEXFile::getData( long address )
{
	if( address < 0 || address >= size )
		throw new ErrorMsg( "Address outside legal range!" );

	return data[ address ];
}


void HEXFile::setData( long address, long value )
{
	if( address < 0 || address >= size )
		throw new ErrorMsg( "Address outside legal range!" );

	data[ address ] = (unsigned char) (value & 0xff);
}


/* end of file */

