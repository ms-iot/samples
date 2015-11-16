/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : Utility.hpp
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
 * Description       : A class holding misc. utility methods used in AVROSP.
 *
 * 
 ****************************************************************************/
#ifndef UTILITY_HPP
#define UTILITY_HPP

using namespace std;

#ifndef NOREGISTRY
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <vector>
#include <string>
#include "ErrorMsg.hpp"

class Utility
{
	protected:
		bool noLog;
		bool noProgress;

	public:
		/* Constructor */
		Utility();

		/* Destructor */
		~Utility();

		/* Methods */
		void muteLog() { noLog = true; }
		void muteProgress() { noProgress = true; }

		void log( const string & txt );
		void progress( const string & txt );

		long convertHex( const string & txt );
		string convertLong( long num, long radix = 10 );

		void parsePath( vector<string> & list );
		bool fileExists( string filename );
		void saveString( string txt, string filename );

#ifndef NOREGISTRY	
		string getRegistryValue( const string & path, const string & value );
#endif
};

extern Utility Util;

#endif

