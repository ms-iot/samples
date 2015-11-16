/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : ErrorMsg.hpp
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
 * Description       : A class providing a container for general error messages. This
 *                     class can be thrown as an exception.
 *
 * 
 ****************************************************************************/
#ifndef ERRORMSG_HPP
#define ERRORMSG_HPP

using namespace std;

#include <stdlib.h>
#include <iostream>
#include <string>


class ErrorMsg
{
	protected:
		string message; // Contains the error message.

	public:
		// Constructors taking the string as parameter.
		ErrorMsg( const string & _message );

		// Destructor
		~ErrorMsg();

		// Function returning the error msg.
		virtual const string & What();
};

#endif

