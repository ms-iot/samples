/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRBootloader.hpp
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
#ifndef AVRBOOTLOADER_HPP
#define AVRBOOTLOADER_HPP

using namespace std;

#include "AVRProgrammer.hpp"
#include "Utility.hpp"

class AVRBootloader : public AVRProgrammer
{
	protected:
		virtual void setAddress( long address );
		virtual void writeFlashLowByte( long value ); // Alwyas low byte first...
		virtual void writeFlashHighByte( long value ); // ...then high byte.
		virtual void writeFlashPage();

		virtual bool writeFlashBlock( HEXFile * data );
		virtual bool readFlashBlock( HEXFile * data );
		virtual bool writeEEPROMBlock( HEXFile * data );
		virtual bool readEEPROMBlock( HEXFile * data );

	public:
		/* Constructor */
		AVRBootloader( CommChannel * _comm );

		/* Destructor */
		~AVRBootloader();

		/* Methods */
		virtual bool enterProgrammingMode();
		virtual bool leaveProgrammingMode();

		virtual bool chipErase();

		virtual bool readOSCCAL( long pos, long * value );
		virtual bool readSignature( long * sig0, long * sig1, long * sig2 );
		virtual bool checkSignature( long sig0, long sig1, long sig2 );

		virtual bool writeFlashByte( long address, long value );
		virtual bool writeEEPROMByte( long address, long value );

		virtual bool writeFlash( HEXFile * data );
		virtual bool readFlash( HEXFile * data );

		virtual bool writeEEPROM( HEXFile * data );
		virtual bool readEEPROM( HEXFile * data );

		virtual bool writeLockBits( long bits );
		virtual bool readLockBits( long * bits );

		virtual bool writeFuseBits( long bits );
		virtual bool readFuseBits( long * bits );
		virtual bool writeExtendedFuseBits( long bits );
		virtual bool readExtendedFuseBits( long * bits );

		virtual bool programmerSoftwareVersion( long * major, long * minor );
		virtual bool programmerHardwareVersion( long * major, long * minor );
};


#endif

