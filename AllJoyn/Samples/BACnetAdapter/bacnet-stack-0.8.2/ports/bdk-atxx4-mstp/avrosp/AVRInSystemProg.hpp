/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : AVRInSystemProg.hpp
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
#ifndef AVRINSYSTEMPROG_HPP
#define AVRINSYSTEMPROG_HPP

using namespace std;

#include "AVRProgrammer.hpp"
#include "Utility.hpp"

class AVRInSystemProg : public AVRProgrammer
{
	protected:
		void setAddress( long address );
		void writeFlashLowByte( long value ); // Alwyas low byte first...
		void writeFlashHighByte( long value ); // ...then high byte.
		void writeFlashPage();

	public:
		/* Constructor */
		AVRInSystemProg( CommChannel * _comm );

		/* Destructor */
		~AVRInSystemProg();

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

