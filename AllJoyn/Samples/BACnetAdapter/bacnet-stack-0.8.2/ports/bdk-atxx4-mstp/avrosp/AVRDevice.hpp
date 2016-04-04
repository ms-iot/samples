/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRDevice.hpp
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
 * Description       : A class containing information of device memory sizes etc.
 *                     It also provides funcitons for reading these parameters from
 *                     the PartDescriptionFiles supplied with AVR Studio 4.
 *
 * 
 ****************************************************************************/
#ifndef AVRDEVICE_HPP
#define AVRDEVICE_HPP

using namespace std;


#include <string>
#include <vector>
#include "Utility.hpp"
#include "XMLParser.hpp"
#include "ErrorMsg.hpp"

class AVRDevice
{
	protected:
		string deviceName; // The name of the device, eg. ATmega128.

		long flashSize; // Size of Flash memory in bytes.
		long eepromSize; // Size of EEPROM memory in bytes.
		bool hasFuseBits; // Does this device have fuse bits at all?
		bool hasExtendedFuseBits; // Does this device have extended fuses?
		long signature0;
		long signature1;
		long signature2; // The three signature bytes, read from XML PartDescriptionFiles.
		long pagesize; // Flash page size.

	public:
		/* Constructor */
		AVRDevice( const string & _deviceName );

		/* Destructor */
		~AVRDevice();

		/* Methods */
		void readParametersFromAVRStudio( vector<string> & searchpath );

		long getFlashSize() { return flashSize; }
		long getEEPROMSize() { return eepromSize; }
		long getPageSize() { return pagesize; }
		bool getFuseStatus() { return hasFuseBits; }
		bool getXFuseStatus() { return hasExtendedFuseBits; }

		void getSignature( long * sig0, long * sig1, long * sig2 );
};

#endif

