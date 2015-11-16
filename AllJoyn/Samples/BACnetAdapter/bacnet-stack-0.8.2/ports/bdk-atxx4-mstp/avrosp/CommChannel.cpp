/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : CommChannel.cpp
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
 * Description       : An abstract class for general byte-by-byte communication.
 *                     Serialport, USB, TCP/IP or similar implementations can be derived
 *                     from this class to create a technology-independent
 *                     communication interface.
 *
 *                     This abstract class does not provide any constructor as it is
 *                     too specific for this generalized class. Derived classes should
 *                     implement their own constructors for specific communication devices.
 *
 * 
 ****************************************************************************/
#include "CommChannel.hpp"


/* Destructor */
CommChannel::~CommChannel()
{
	/* no code here */
}

/* end of file */

