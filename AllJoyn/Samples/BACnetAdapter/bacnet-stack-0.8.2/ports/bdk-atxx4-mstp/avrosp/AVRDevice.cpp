/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRDevice.cpp
 * Compiler          : Dev-C++ 4.9.8.0 - http://bloodshed.net/dev/
 * Revision          : $Revision: 4017 $
 * Date              : $Date: 2008-06-02 14:26:03 +0200 (ma, 02 jun 2008) $
 * Updated by        : $Author: khole $
 *
 * Support mail      : avr@atmel.com
 *
 * Target platform   : Win32
 *
 * AppNote           : AVR911 - AVR Open-source Programmer
 *
 * Description       : A class containing information of device memory sizes etc.
 *                     It also provides funcitons for reading these parameters from
 *                     the PartDescriptionFiles supplied with AVR Studio 4.
 *
 *
 ****************************************************************************/
#include "AVRDevice.hpp"


/* Constructor */
AVRDevice::AVRDevice( const string & _deviceName ) :
	deviceName( _deviceName )
{
	flashSize =
		eepromSize = 0;
	hasFuseBits = false;
	hasExtendedFuseBits = false;
	signature0 =
		signature1 =
		signature2 = 0;
	pagesize = -1;
}


/* Destructor */
AVRDevice::~AVRDevice()
{
	/* no code here */
}

/* Read parameters from AVR Studio XML files */	
void AVRDevice::readParametersFromAVRStudio( vector<string> & searchpath )
{
	string path;
	string signature;
	string cache;

#ifndef NOREGISTRY
	/* Locate the directory containing the XML files from the Windows registry database */
	try
	{
		path = Util.getRegistryValue( "SOFTWARE\\Atmel\\AVRTools\\", "AVRToolsPath" );
		path += "\\PartDescriptionFiles";
		searchpath.push_back( path );
	} catch( ErrorMsg * e )
	{
		delete e;
	}
#endif

	/* Search for file */
	path.erase();
	int i;
	for( i = 0; i < searchpath.size(); i++ )
	{
		path = searchpath[i] + "\\" + deviceName + ".xml";
		if( Util.fileExists( path ) )
			break;
	}

	if( i == searchpath.size() )
		throw new ErrorMsg( "Device XML file not found in search path!" );

	/* Parse the file for required info */
	Util.log( "Parsing '" + path + "'...\r\n" );
	XMLFile f( path ); // Load XML info

	flashSize = atoi( f.getValue( "AVRPART\\MEMORY\\PROG_FLASH" ).c_str() );
	eepromSize = atoi( f.getValue( "AVRPART\\MEMORY\\EEPROM" ).c_str() );

	cache += "<AVRPART><MEMORY><PROG_FLASH>";
	cache += f.getValue( "AVRPART\\MEMORY\\PROG_FLASH" );
	cache += "</PROG_FLASH><EEPROM>";
	cache += f.getValue( "AVRPART\\MEMORY\\EEPROM" );
	cache += "</EEPROM>";

	if( f.exists( "AVRPART\\MEMORY\\BOOT_CONFIG" ) )
	{
		pagesize = atoi( f.getValue( "AVRPART\\MEMORY\\BOOT_CONFIG\\PAGESIZE" ).c_str() );
		pagesize <<= 1; // We want pagesize in bytes.

		cache += "<BOOT_CONFIG><PAGESIZE>";
		cache += f.getValue( "AVRPART\\MEMORY\\BOOT_CONFIG\\PAGESIZE" );
		cache += "</PAGESIZE></BOOT_CONFIG>";
	}

	cache += "</MEMORY>";

	if( f.exists( "AVRPART\\FUSE" ) )
	{
		hasFuseBits = true;

		cache += "<FUSE>";

		if( f.exists( "AVRPART\\FUSE\\EXTENDED" ) )
		{
			hasExtendedFuseBits = true;
			cache += "<EXTENDED></EXTENDED>";
		}

		cache += "</FUSE>";
	}

	signature = f.getValue( "AVRPART\\ADMIN\\SIGNATURE\\ADDR000" );
	signature.erase( 0, 1 ); // Remove the $ character.
	signature0 = Util.convertHex( signature );

	signature = f.getValue( "AVRPART\\ADMIN\\SIGNATURE\\ADDR001" );
	signature.erase( 0, 1 ); // Remove the $ character.
	signature1 = Util.convertHex( signature );

	signature = f.getValue( "AVRPART\\ADMIN\\SIGNATURE\\ADDR002" );
	signature.erase( 0, 1 ); // Remove the $ character.
	signature2 = Util.convertHex( signature );

	cache += "<ADMIN><SIGNATURE><ADDR000>";
	cache += f.getValue( "AVRPART\\ADMIN\\SIGNATURE\\ADDR000" );
	cache += "</ADDR000><ADDR001>";
	cache += f.getValue( "AVRPART\\ADMIN\\SIGNATURE\\ADDR001" );
	cache += "</ADDR001><ADDR002>";
	cache += f.getValue( "AVRPART\\ADMIN\\SIGNATURE\\ADDR002" );
	cache += "</ADDR002></SIGNATURE></ADMIN></AVRPART>\r\n";

	/* Save cached file to application directory */
	Util.log( "Saving cached XML parameters...\r\n" );
	Util.saveString( cache, searchpath[1] + "\\" + deviceName + ".xml" );
}


void AVRDevice::getSignature( long * sig0, long * sig1, long * sig2 )
{
	if( sig0 == NULL || sig1 == NULL || sig2 == NULL )
		throw new ErrorMsg( "Cannot copy signature bytes to NULL-pointers!" );

	*sig0 = signature0;
	*sig1 = signature1;
	*sig2 = signature2;
}


/* end of file */
