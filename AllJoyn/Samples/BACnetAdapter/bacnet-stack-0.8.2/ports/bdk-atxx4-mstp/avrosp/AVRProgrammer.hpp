/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRProgrammer.hpp
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
#ifndef AVRPROGRAMMER_HPP
#define AVRPROGRAMMER_HPP

using namespace std;

#include "ErrorMsg.hpp"
#include "HEXParser.hpp"
#include "CommChannel.hpp"

class AVRProgrammer
{
	protected:
		long pagesize; // Flash page size.
		CommChannel * comm;

	public:
		/* Constructor */
		AVRProgrammer( CommChannel * _comm );

		/* Destructor */
		~AVRProgrammer();

		/* Static member */
		static string readProgrammerID( CommChannel * _comm ); // Reads 7-character ID.

		/* Methods */
		void setPagesize( long _pagesize ) { pagesize = _pagesize; }

		virtual bool enterProgrammingMode() = 0;
		virtual bool leaveProgrammingMode() = 0;

		virtual bool chipErase() = 0;

		virtual bool readOSCCAL( long pos, long * value ) = 0;
		virtual bool readSignature( long * sig0, long * sig1, long * sig2 ) = 0;
		virtual bool checkSignature( long sig0, long sig1, long sig2 ) = 0;

		virtual bool writeFlashByte( long address, long value ) = 0;
		virtual bool writeEEPROMByte( long address, long value ) = 0;

		virtual bool writeFlash( HEXFile * data ) = 0;
		virtual bool readFlash( HEXFile * data ) = 0;

		virtual bool writeEEPROM( HEXFile * data ) = 0;
		virtual bool readEEPROM( HEXFile * data ) = 0;

		virtual bool writeLockBits( long bits ) = 0;
		virtual bool readLockBits( long * bits ) = 0;

		virtual bool writeFuseBits( long bits ) = 0;
		virtual bool readFuseBits( long * bits ) = 0;
		virtual bool writeExtendedFuseBits( long bits ) = 0;
		virtual bool readExtendedFuseBits( long * bits ) = 0;

		virtual bool programmerSoftwareVersion( long * major, long * minor ) = 0;
		virtual bool programmerHardwareVersion( long * major, long * minor ) = 0;
};


#endif

