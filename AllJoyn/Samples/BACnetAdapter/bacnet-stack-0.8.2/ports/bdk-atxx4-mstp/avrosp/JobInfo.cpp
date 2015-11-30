/*****************************************************************************
 *
 * Atmel Corporation
 *
 * File              : JobInfo.cpp
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
 * Description       : A class holding information on what the AVR Open-Source Programmer
 *                     should do. The information is derived from the command-line.
 *
 *
 ****************************************************************************/
#include "JobInfo.hpp"

#include <conio.h>

#define VERSIONSTRING "$Revision: 1163 $" // For use in later text output.

#define TIMEOUT 5
#define TIMEOUTSTRING "5"


JobInfo::JobInfo()
{
	/* Initialize variables */
	showHelp = false;
	silentMode = false;
	noProgressIndicator = false;
	readSignature = false;
	chipErase = false;
	getHWrevision = false;
	getSWrevision = false;
	programFlash = false;
	programEEPROM = false;
	readFlash = false;
	readEEPROM = false;
	verifyFlash = false;
	verifyEEPROM = false;
	readLockBits = false;
	readFuseBits = false;
	readOSCCAL = false;

	deviceName.erase();
	inputFileFlash.erase();
	inputFileEEPROM.erase();
	outputFileFlash.erase();
	outputFileEEPROM.erase();

	OSCCAL_Parameter = -1;
	OSCCAL_FlashAddress = -1;
	OSCCAL_EEPROMAddress = -1;

	programLockBits = -1;
	verifyLockBits = -1;	

	programFuseBits = -1;
	programExtendedFuseBits = -1;
	verifyFuseBits = -1;
	verifyExtendedFuseBits = -1;

	memoryFillPattern = -1;

	flashStartAddress = -1;
	flashEndAddress = -1;

	eepromStartAddress = -1;
	eepromEndAddress = -1;

	comPort = -1;
}



void JobInfo::parseCommandline( int argc, char *argv[] )
{
	char * param; // Temp string ptr for holding current parsed parameter.
	int comma; // Temp position for comma separator in address ranges.

	/* Get application directory */
	string ownpath = argv[0];
	int slash_pos = ownpath.find_last_of( "\\/" ); // Search for last og / or \.
	if( slash_pos == string::npos ) // Not found?
	{
		ownpath.assign( "." ); // The current directory is the AVROSP EXE path also.
	} else
	{
		ownpath.erase( slash_pos ); // Remove from and including the last slash separator.
	}

	searchpath.push_back( "." ); // Add current directory also.
	searchpath.push_back( ownpath ); // Save for later.

	if( argc <= 1 )
	{
		showHelp = true;
		return;
	}

	/* Iterate through cmdline parameters */
	for( int i = 1; i < argc; i++ )
	{
		param = argv[i];

		/* Allow parameters to start with '-' */
		if( param[0] != '-' )
			throw new ErrorMsg( "All parameters must start with '-'!" );

		if( strlen( param ) <= 1 )
			throw new ErrorMsg( "Parameters cannot be just the minus without any characters!" );

		/* Now for the parsing... */
		switch( param[1] )
		{
			case 'a' : // Address range specified.
				if( strlen( param ) <= 2 )
					throw new ErrorMsg( "Cannot use -a without memory type!" );

				if( strlen( param ) <= 5 )
					throw new ErrorMsg( "Cannot use -a without start and end address!" );

				/* Find comma position and set it to '0' to help hex conversion */
				comma = 2;
				while( (param[comma] != 0) && (param[comma] != ',') )
					comma++;

				if( comma == strlen( param ) ) // No comma found?
					throw new ErrorMsg( "No comma separator found in -a parameter!" );

				param[comma] = 0; // It is now two separate strings for hex conversion.

				/* Convert limits */
				switch( param[2] )
				{
					case 'f' : // Flash address range.
						try
						{
							flashStartAddress = convertHex( param + 3 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Number format error in start limit for -af parameter!" );
						}

						try
						{
							flashEndAddress = convertHex( param + comma + 1 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Number format error in end limit for -af parameter!" );
						}

						if( flashEndAddress < flashStartAddress )
							throw new ErrorMsg( "Cannot have Flash end limit less than start limit!" );

						break;

					case 'e' : // EEPROM address range.
						try
						{
							eepromStartAddress = convertHex( param + 3 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Number format error in start limit for -ae parameter!" );
						}

						try
						{
							eepromEndAddress = convertHex( param + comma + 1 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Number format error in end limit for -ae parameter!" );
						}

						if( eepromEndAddress < eepromStartAddress )
							throw new ErrorMsg( "Cannot have EEPROM end limit less than start limit!" );

						break;

					default:
						throw new ErrorMsg( "Unknown choice for -a, use -af or -ae!" );

				}

				break;


			case 'b' : // Get revision.
				if( strlen( param ) != 3 )
					throw new ErrorMsg( "Specify SW og HW revision, not just -b!" );

				switch( param[2] )
				{
					case 'h' : // Hardware revision.
						getHWrevision = true;
						break;

					case 's' : // Software revision.
						getSWrevision = true;
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -b, use -bs or -bh!" );
				}

				break;


			case 'c' : // Specify COM port.
				if (( strlen( param ) < 6 ) || (strlen( param ) > 7) ||
                    (param[2] != 'C' || param[3] != 'O' || param[4] != 'M' ) ||
				    (param[5] < '1' || param[5] > '9')) {
					throw new ErrorMsg( "COM port parameter syntax is -cCOM1 to -cCOM99" );
                }
				comPort = param[5] - '0'; // Convert COM port digit to number.
				if (param[6] != 0) {
                    comPort = (comPort * 10) + param[6] - '0';
                } 
				break;


			case 'd' : // Device name specified.
				if( strlen( param ) <= 2 )
					throw new ErrorMsg( "Cannot use -d without the device name!" );

				/* Copy device name string to class variable */
				deviceName.assign( param + 2 );
				break;


			case 'e' : // Chip erase before programming.
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -e needs no extra arguments!" );

				chipErase = true;
				break;


			case 'E' : // Set extended fuse bits.
				if( strlen( param ) != 4 )
					throw new ErrorMsg( "Use two hex digits for the -E parameter!" );

				try
				{
					programExtendedFuseBits = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -E parameter!" );
				}

				break;


			case 'f' : // Set fuse bits.
				if( strlen( param ) != 6 )
					throw new ErrorMsg( "Use four hex digits for the -f parameter!" );

				try
				{
					programFuseBits = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -f parameter!" );
				}

				break;


			case 'F' : // Verify fuse bits.
				if( strlen( param ) != 6 )
					throw new ErrorMsg( "Use four hex digits for the -F parameter!" );

				try
				{
					verifyFuseBits = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -F parameter!" );
				}

				break;


			case 'g' : // Silent operation.
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -g needs no extra arguments!" );

				silentMode = true;
				break;


			case 'G' : // Verify extended fuse bits.
				if( strlen( param ) != 4 )
					throw new ErrorMsg( "Use two hex digits for the -G parameter!" );

				try
				{
					verifyExtendedFuseBits = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -G parameter!" );
				}

				break;


			case 'h' : // Help screen.
			case '?' : // Help screen.
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -h and -? needs no extra arguments!" );

				showHelp = true;
				break;


			case 'i' : // Input file specified.
				if( strlen( param ) <= 2 )
					throw new ErrorMsg( "Cannot use -i without memory type!" );

				if( strlen( param ) <= 3 )
					throw new ErrorMsg( "Cannot use -i without file name!" );

				/* Copy file name string to correct class variable */
				switch( param[2] )
				{
					case 'f' : // Flash input file.
						inputFileFlash.assign( param + 3 );
						break;

					case 'e' : // EEPROM input file.
						inputFileEEPROM.assign( param + 3 );
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -i, use -if or -ie!" );
				}

				break;


			case 'l' : // Set lock bits.
				if( strlen( param ) != 4 )
					throw new ErrorMsg( "Use two hex digits for the -l parameter!" );

				try
				{
					programLockBits = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -l parameter!" );
				}

				break;


			case 'L' : // Verify lock bits.
				if( strlen( param ) != 4 )
					throw new ErrorMsg( "Use two hex digits for the -l parameter!" );

				try
				{
					verifyLockBits = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -L parameter!" );
				}

				break;


			case 'o' : // Output file specified.
				if( strlen( param ) <= 2 )
					throw new ErrorMsg( "Cannot use -o without memory type!" );

				if( strlen( param ) <= 3 )
					throw new ErrorMsg( "Cannot use -o without file name!" );

				/* Copy file name string to correct class variable */
				switch( param[2] )
				{
					case 'f' : // Flash output file.
						outputFileFlash.assign( param + 3 );
						break;

					case 'e' : // EEPROM output file.
						outputFileEEPROM.assign( param + 3 );
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -o, use -of or -oe!" );
				}

				break;


			case 'O' : // Read OSCCAL byte.
				switch( strlen( param ) )
				{
					case 2 : // No value specified, use first OSCCAL byte.
						readOSCCAL = true;
						OSCCAL_Parameter = 0; // First OSCCAL byte.
						break;

					case 3 : // Byte index specified.
					case 4 :
						readOSCCAL = true;
						try
						{
							OSCCAL_Parameter = convertHex( param + 2 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Hex number format error for -O parameter!" );
						}

						break;


					case 5 : // Direct value specified.
						if( param[2] != '#' )
							throw new ErrorMsg( "Use one or two hex digits for -O and two for -O#!" );

						readOSCCAL = false;
						try
						{
							OSCCAL_Parameter = convertHex( param + 3 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Hex number format error for -O# parameter!" );
						}

						break;

					default:
						throw new ErrorMsg( "Invalid use of -O or -O# parameter!" );
				}

				break;


			case 'p' : // Program data.
				if( strlen( param ) != 3 )
					throw new ErrorMsg( "Specify memory type, not just -p!" );

				switch( param[2] )
				{
					case 'f' : // Program Flash memory.
						programFlash = true;
						break;

					case 'e' : // Program EEPROM memory.
						programEEPROM = true;
						break;

					case 'b' : // Both.
						programFlash = true;
						programEEPROM = true;
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -p, use -pf, -pe or -pb!" );
				}

				break;


			case 'q' : // Read all fuse bits.
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -q needs no extra arguments!" );

				readFuseBits = true;
				break;


			case 'r' : // Read data.
				if( strlen( param ) != 3 )
					throw new ErrorMsg( "Specify memory type, not just -r!" );

				switch( param[2] )
				{
					case 'f' : // Read Flash memory.
						readFlash = true;
						break;

					case 'e' : // Read EEPROM memory.
						readEEPROM = true;
						break;

					case 'b' : // Both.
						readFlash = true;
						readEEPROM = true;
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -r, use -rf, -re or -rb!" );
				}

				break;


			case 's' : // Read signature byte.
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -s needs no extra arguments!" );

				readSignature = true;
				break;


			case 'S' : // Write OSCCAL byte to memory.
				if( strlen( param ) <= 2 )
					throw new ErrorMsg( "Cannot use -S without memory type!" );

				if( strlen( param ) <= 3 )
					throw new ErrorMsg( "Cannot use -S without byte address!" );

				switch( param[2] )
				{
					case 'f' : // Write to Flash address.
						try
						{
							OSCCAL_FlashAddress = convertHex( param + 3 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Cannot convert hex number for -Sf parameter!" );
						}
						break;

					case 'e' : // Write to EEPROM address.
						try
						{
							OSCCAL_EEPROMAddress = convertHex( param + 3 );
						}
						catch( ErrorMsg * e )
						{
							delete e;
							throw new ErrorMsg( "Cannot convert hex number for -Se parameter!" );
						}
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -S, use -Sf or -Se!" );
				}

				break;


			case 'v' : // Verify data.
				if( strlen( param ) != 3 )
					throw new ErrorMsg( "Specify memory type, not just -v!" );

				switch( param[2] )
				{
					case 'f' : // Verify Flash memory.
						verifyFlash = true;
						break;

					case 'e' : // Verify EEPROM memory.
						verifyEEPROM = true;
						break;

					case 'b' : // Both.
						verifyFlash = true;
						verifyEEPROM = true;
						break;

					default:
						throw new ErrorMsg( "Unknown choice for -v, use -vf, -ve or -vb!" );
				}

				break;


			case 'x' : // Fill unspecified memory.
				if( strlen( param ) != 4 )
					throw new ErrorMsg( "Use two hex digits for the -x parameter!" );

				try
				{
					memoryFillPattern = convertHex( param + 2 );
				}
				catch( ErrorMsg * e )
				{
					delete e;
					throw new ErrorMsg( "Hex number format error for -x parameter!" );
				}

				break;


			case 'y' : // Read lock bits.
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -y needs no extra arguments!" );

				readLockBits = true;
				break;


			case 'z' : // No progress indicator?
				if( strlen( param ) != 2 )
					throw new ErrorMsg( "Parameter -z needs no extra arguments!" );

				noProgressIndicator = true;
				break;


			default:
				throw new ErrorMsg( "Unknow parameter!" );
		}
	}
}


void JobInfo::help()
{
	cout
		<< "Command Line Switches:" << endl
		<< "        [-d device name] [-if infile] [-ie infile] [-of outfile] [-oe outfile]" << endl
		<< "        [-s] [-O index] [-O#value] [-Sf addr] [-Se addr] [-e] [-p f|e|b]" << endl
		<< "        [-r f|e|b] [-v f|e|b] [-l value] [-L value] [-y] [-f value] [-E value]" << endl
		<< "        [-F value] [-G value] [-q] [-x value] [-af start,stop] [-ae start,stop]" << endl
		<< "        [-c port] [-b h|s] [-g] [-z] [-h|?]" << endl
		<< endl
		<< "Parameters:" << endl
		<< "d       Device name. Must be applied when programming the device." << endl
		<< "if      Name of FLASH input file. Required for programming or verification" << endl
		<< "        of the FLASH memory. The file format is Intel Extended HEX." << endl
		<< "ie      Name of EEPROM input file. Required for programming or verification" << endl
		<< "        of the EEPROM memory. The file format is Intel Extended HEX." << endl
		<< "of      Name of FLASH output file. Required for readout of the FLASH memory." << endl
		<< "        The file format is Intel Extended HEX." << endl
		<< "oe      Name of EEPROM output file. Required for readout of the EEPROM" << endl
		<< "        memory. The file format is Intel Extended HEX." << endl
		<< "s       Read signature bytes." << endl;
	getch();
	cout
		<< "O       Read oscillator calibration byte. 'index' is optional." << endl
		<< "O#      User-defined oscillator calibration value." << endl
		<< "Sf      Write oscillator cal. byte to FLASH memory. 'addr' is byte address." << endl
		<< "Se      Write oscillator cal. byte to EEPROM memory. 'addr' is byte address." << endl
		<< "e       Erase device. If applied with another programming parameter, the" << endl
		<< "        device will be erased before any other programming takes place." << endl
		<< "p       Program device; FLASH (f), EEPROM (e) or both (b). Corresponding" << endl
		<< "        input files are required." << endl
		<< "r       Read out device; FLASH (f), EEPROM (e) or both (b). Corresponding" << endl
		<< "        output files are required" << endl
		<< "v       Verify device; FLASH (f), EEPROM (e) or both (b). Can be used with" << endl
		<< "        -p or alone. Corresponding input files are required." << endl
		<< "l       Set lock byte. 'value' is an 8-bit hex. value." << endl
		<< "L       Verify lock byte. 'value' is an 8-bit hex. value to verify against." << endl
		<< "y       Read back lock byte." << endl
		<< "f       Set fuse bytes. 'value' is a 16-bit hex. value describing the" << endl
		<< "        settings for the upper and lower fuse bytes." << endl
		<< "E       Set extended fuse byte. 'value' is an 8-bit hex. value describing the" << endl
		<< "        extend fuse settings." << endl
		<< "F       Verify fuse bytes. 'value' is a 16-bit hex. value to verify against." << endl;
	getch();
	cout
		<< "G       Verify extended fuse byte. 'value' is an 8-bit hex. value to" << endl
		<< "        verify against." << endl
		<< "q       Read back fuse bytes." << endl
		<< "x       Fill unspecified locations with a value (00-ff). The default is" << endl
		<< "        to not program locations not specified in the input files." << endl
		<< "af      FLASH address range. Specifies the address range of operations. The" << endl
		<< "        default is the entire FLASH. Byte addresses in hex." << endl
		<< "ae      EEPROM address range. Specifies the address range of operations." << endl
		<< "        The default is the entire EEPROM. Byte addresses in hex." << endl
		<< "c       Select communication port; 'COM1' to 'COM99'. If this parameter is" << endl
		<< "        omitted the program will scan the COM ports for a programmer." << endl
		<< "b       Get revisions; hardware revision (h) and software revision (s)." << endl
		<< "g       Silent operation." << endl
		<< "z       No progress indicator. E.g. if piping to a file for log purposes, use" << endl
		<< "        this option to avoid the characters used for the indicator." << endl
		<< "h|?     Help information (overrides all other settings)." << endl
		<< endl
		<< "Example:" << endl
		<< "        AVROSP -dATmega128 -ifmyapp.hex -pf" << endl;
}


long JobInfo::convertHex( char * txt )
{
	string t( txt );
	return Util.convertHex( t );
}


void JobInfo::doJob()
{
	long scanCOM;
	SerialPort * com;
	AVRProgrammer * prog;
	AVRDevice * avr;
	string programmerID;
	long sig0, sig1, sig2; // Signature bytes.

	/* Set correct silent and progress indicator status */
	if( silentMode )
	{
		Util.muteLog();
		Util.muteProgress(); // Silent also includes progress indicator.
	}

	if( noProgressIndicator )
		Util.muteProgress();

	/* Application header text */
	Util.log( "AVR Open-source Programmer " VERSIONSTRING " (C) 2004 Atmel Corp.\n\r\n\r" );

	/* Show help screen? */
	if( showHelp )
	{
		help();
		return;
	}

	Util.log( "Serial port timeout set to " TIMEOUTSTRING " sec.\r\n" );

	/* Need to scan for COM port? */
	if( comPort == -1 )
	{
		Util.log( "Scanning COM ports for supported programmer...\n\r" );

		for( scanCOM = 1; scanCOM <= 99; scanCOM++ )
		{
			Util.progress( "COM" + Util.convertLong( scanCOM ) + "...\r\n" );

			try
			{
				/* Try to communicate */
				com = NULL;
				com = new SerialPort( scanCOM, TIMEOUT );
				com->openChannel();
				programmerID = AVRProgrammer::readProgrammerID( com );

				/* Contact! Check ID... Add custom handler signatures here */
				if( programmerID == "AVRBOOT" || programmerID == "AVR ISP" )
				{
					break;
				}

				delete com;
				Util.progress( programmerID + " found - not supported!\r\n" );
			}
			catch( ErrorMsg * e )
			{
				/* No contact on COM port, skip to next */
				if( com != NULL ) delete com;
				delete e;
			}
		}

		/* Exit if no supported programmers found */
		if( scanCOM > 99 )
		{
			Util.log( "No supported programmers found!\r\n" );
			return;
		}

		comPort = scanCOM;

	} else // ... COM port is specified
	{
		/* Try to communicate, errors will propagate to caller */
		com = new SerialPort( comPort, TIMEOUT );
		com->openChannel();
		programmerID = AVRProgrammer::readProgrammerID( com );

		/* Contact! Check ID */
		if( programmerID != "AVRBOOT" && programmerID != "AVR ISP" )
			throw new ErrorMsg( "Programmer not supported!" );
	}

	Util.log( "Found " + programmerID + " on COM" + Util.convertLong( comPort ) + "!\r\n" );

	/* Create programmer interface object, add custom handlers here */
	if( programmerID == "AVRBOOT" )
	{
		prog = new AVRBootloader( com );
	}

	if( programmerID == "AVR ISP" )
	{
		prog = new AVRInSystemProg( com );
	}

	Util.log( "Entering programming mode...\r\n" );
	prog->enterProgrammingMode(); // Ignore return code.

	/* Do device independent operations */
	doDeviceIndependent( prog );

	/* Finished if no device name is specified */
	if( deviceName.size() == 0 )
	{
		Util.log( "Device name not specified!\r\n" );
		return;
	}

	/* Parse XML part description file */
	avr = new AVRDevice( deviceName );
	Util.log( "Parsing XML file for device parameters...\r\n" );
	Util.parsePath( searchpath );
	avr->readParametersFromAVRStudio( searchpath );

	/* Verify that the device signature matches the signature from the XML file */
	avr->getSignature( &sig0, &sig1, &sig2 );
	if( prog->checkSignature( sig0, sig1, sig2 ) )
		Util.log( "Signature matches device!\r\n" );

	/* Do device dependent operations */
	doDeviceDependent( prog, avr );

	/* Clean up */
	Util.log( "Leaving programming mode...\r\n" );
	prog->leaveProgrammingMode(); // Ignore return code.

	delete avr;
	delete prog;
	delete com;
}


void JobInfo::doDeviceIndependent( AVRProgrammer * prog )
{
	long sig0, sig1, sig2; // Signature bytes.
	long minor, major; // Minor and major programmer revision.

	/* Read signature? */
	if( readSignature )
	{
		Util.log( "Reading signature bytes: " );
		if( !prog->readSignature( &sig0, &sig1, &sig2 ) )
			throw new ErrorMsg( "Signature readout is not supported by this programmer!" );

		/* No pass through Util, since user wants the info */
		cout.fill( '0' );
		cout << hex
			<< "0x" << setw(2) << sig0 << " "
			<< "0x" << setw(2) << sig1 << " "
			<< "0x" << setw(2) << sig2 << endl;
	}

	/* Get software version? */
	if( getSWrevision )
	{
		Util.log( "Reading programmer software revision: " );
		if( !prog->programmerSoftwareVersion( &major, &minor ) )
			throw new ErrorMsg( "Software revision readout is not supported by this programmer!" );

		/* No pass through Util, since user wants the info */
		cout << (char) (major & 0xff) << "." << (char) (minor & 0xff) << endl;
	}

	/* Get software version? */
	if( getHWrevision )
	{
		Util.log( "Reading programmer hardware revision: " );
		if( !prog->programmerHardwareVersion( &major, &minor ) )
			throw new ErrorMsg( "Hardware revision readout is not supported by this programmer!" );

		/* No pass through Util, since user wants the info */
		cout << (char) (major & 0xff) << "." << (char) (minor & 0xff) << endl;
	}
}


void JobInfo::doDeviceDependent( AVRProgrammer * prog, AVRDevice * avr )
{
	HEXFile * hex;
	HEXFile * hex_v; // Used for verifying memory contents.
	long pos; // Used when comparing data.
	long bits; // Used for lock and fuse bits.

	/* Set programmer pagesize */
	prog->setPagesize( avr->getPageSize() );

	/* Check if specified address limits are within device range */
	if( flashEndAddress != -1 )
	{
		if( flashEndAddress >= avr->getFlashSize() )
			throw new ErrorMsg( "Specified Flash address range is outside device address space!" );
	} else
	{
		flashStartAddress = 0;
		flashEndAddress = avr->getFlashSize() - 1;
	}

	if( eepromEndAddress != -1 )
	{
		if( eepromEndAddress >= avr->getEEPROMSize() )
			throw new ErrorMsg( "Specified EEPROM address range is outside device address space!" );
	} else
	{
		eepromStartAddress = 0;
		eepromEndAddress = avr->getEEPROMSize() - 1;
	}


	/* Read out Flash contents? */
	if( readFlash )
	{
		/* Check that filename has been specified */
		if( outputFileFlash.size() == 0 )
			throw new ErrorMsg( "Cannot read Flash without output file specified!" );

		/* Prepare the file */
		hex = new HEXFile( avr->getFlashSize() );
		hex->setUsedRange( flashStartAddress, flashEndAddress );

		/* Read data and save file */
		Util.log( "Reading Flash contents...\r\n" );
		if( !prog->readFlash( hex ) )
			throw new ErrorMsg( "Flash readout is not supported by this programmer!" );
		Util.log( "Writing HEX output file...\r\n" );
		hex->writeFile( outputFileFlash );

		delete hex;
	}


	/* Read out EEPROM contents? */
	if( readEEPROM )
	{
		/* Check that filename has been specified */
		if( outputFileEEPROM.size() == 0 )
			throw new ErrorMsg( "Cannot read EEPROM without output file specified!" );

		/* Prepare the file */
		hex = new HEXFile( avr->getEEPROMSize() );
		hex->setUsedRange( eepromStartAddress, eepromEndAddress );

		/* Read data and save file */
		Util.log( "Reading EEPROM contents...\r\n" );
		if( !prog->readEEPROM( hex ) )
			throw new ErrorMsg( "EEPROM readout is not supported by this programmer!" );
		Util.log( "Writing HEX output file...\r\n" );
		hex->writeFile( outputFileEEPROM );

		delete hex;
	}


	/* Read lock bits? */
	if( readLockBits )
	{
		Util.log( "Reading lock bits...\r\n" );
		if( !prog->readLockBits( &bits ) )
			throw new ErrorMsg( "Lock bit readout is not supported by this programmer!" );
		cout << "0x" << std::hex << setw(2) << bits << endl;
	}


	/* Read fuse bits (both ordinary and extended)? */
	if( readFuseBits )
	{
		if( !avr->getFuseStatus() )
			throw new ErrorMsg( "Selected device has no fuse bits!" );

		Util.log( "Reading fuse bits...\r\n" );
		if( !prog->readFuseBits( &bits ) )
			throw new ErrorMsg( "Fuse bit readout is not supported by this programmer!" );
		cout << "0x" << std::hex << setw(4) << bits << endl;

		if( avr->getXFuseStatus() )
		{
			if( !prog->readExtendedFuseBits( &bits ) )
				throw new ErrorMsg( "Extended fuse bit readout is not supported by this programmer!" );
			cout << "0x" << std::hex << setw(2) << bits << endl;
		}
	}


	/* Erase chip before programming anything? */
	if( chipErase )
	{
		Util.log( "Erasing chip contents...\r\n" );
		if( !prog->chipErase() )
			throw new ErrorMsg( "Chip erase is not supported by this programmer!" );
	}


	/* Prepare input hex file for flash */
	if( programFlash || verifyFlash )
	{
		/* Check that filename has been specified */
		if( inputFileFlash.size() == 0 )
			throw new ErrorMsg( "Cannot program or verify Flash without input file specified!" );

		/* Prepare the file */
		hex = new HEXFile( avr->getFlashSize() );

		/* Fill if wanted */
		if( memoryFillPattern != -1 )
			hex->clearAll( memoryFillPattern );

		/* Read file */
		Util.log( "Reading HEX input file for flash operations...\r\n" );
		hex->readFile( inputFileFlash );

		/* Check limits */
		if( hex->getRangeStart() > flashEndAddress ||
				hex->getRangeEnd() < flashStartAddress )
			throw new ErrorMsg( "HEX file defines data outside specified range!" );

		if( memoryFillPattern == -1 )
		{
			if( hex->getRangeStart() > flashStartAddress )
				flashStartAddress = hex->getRangeStart();

			if( hex->getRangeEnd() < flashEndAddress )
				flashEndAddress = hex->getRangeEnd();
		}

		hex->setUsedRange( flashStartAddress, flashEndAddress );
	}


	/* Program new Flash contents? */
	if( programFlash )
	{
		/* Program data */
		Util.log( "Programming Flash contents...\r\n" );
		if( !prog->writeFlash( hex ) )
			throw new ErrorMsg( "Flash programming is not supported by this programmer!" );
	}


	/* Verify Flash contents? */
	if( verifyFlash )
	{
		/* Prepare HEX file for comparision */
		hex_v = new HEXFile( avr->getFlashSize() );

		/* Compare to Flash */
		Util.log( "Reading Flash contents...\r\n" );
		hex_v->setUsedRange( hex->getRangeStart(), hex->getRangeEnd() );
		if( !prog->readFlash( hex_v ) )
			throw new ErrorMsg( "Flash readout is not supported by this programmer!" );

		/* Compare data */
		Util.log( "Comparing Flash data...\r\n" );

		for( pos = hex->getRangeStart(); pos <= hex->getRangeEnd(); pos++ )
		{
			if( hex->getData( pos ) != hex_v->getData( pos ) )
			{
				cout << "Unequal at address 0x" << std::hex << pos << "!" << endl;
				break;
			}
		}

		if( pos > hex->getRangeEnd() ) // All equal?
		{
			cout << "Equal!" << endl;
		}

		delete hex_v;
	}

	if( programFlash || verifyFlash )
		delete hex;


	/* Prepare input hex file for EEPROM */
	if( programEEPROM || verifyEEPROM )
	{
		/* Check that filename has been specified */
		if( inputFileEEPROM.size() == 0 )
			throw new ErrorMsg( "Cannot program or verify EEPROM without input file specified!" );

		/* Prepare the file */
		hex = new HEXFile( avr->getEEPROMSize() );

		/* Fill if wanted */
		if( memoryFillPattern != -1 )
			hex->clearAll( memoryFillPattern );

		/* Read file and program contents */
		Util.log( "Reading HEX input file for EEPROM operations...\r\n" );
		hex->readFile( inputFileEEPROM );

		/* Check limits */
		if( hex->getRangeStart() > eepromEndAddress ||
				hex->getRangeEnd() < eepromStartAddress )
			throw new ErrorMsg( "HEX file defines data outside specified range!" );

		if( memoryFillPattern == -1 )
		{
			if( hex->getRangeStart() > eepromStartAddress )
				eepromStartAddress = hex->getRangeStart();

			if( hex->getRangeEnd() < eepromEndAddress )
				eepromEndAddress = hex->getRangeEnd();
		}

		hex->setUsedRange( eepromStartAddress, eepromEndAddress );
	}


	/* Program new EEPROM contents? */
	if( programEEPROM )
	{
		/* Program data */
		Util.log( "Programming EEPROM contents...\r\n" );
		if( !prog->writeEEPROM( hex ) )
			throw new ErrorMsg( "EEPROM programming is not supported by this programmer!" );
	}

	/* Verify EEPROM contents? */
	if( verifyEEPROM )
	{
		/* Prepare HEX file for comparision */
		hex_v = new HEXFile( avr->getEEPROMSize() );

		/* Compare to EEPROM */
		Util.log( "Reading EEPROM contents...\r\n" );
		hex_v->setUsedRange( hex->getRangeStart(), hex->getRangeEnd() );
		if( !prog->readEEPROM( hex_v ) )
			throw new ErrorMsg( "EEPROM readout is not supported by this programmer!" );

		/* Compare data */
		Util.log( "Comparing EEPROM data...\r\n" );
		for( pos = hex->getRangeStart(); pos <= hex->getRangeEnd(); pos++ )
		{
			if( hex->getData( pos ) != hex_v->getData( pos ) )
			{
				cout << "Unequal at address 0x" << std::hex << pos << "!" << endl;
				break;
			}
		}

		if( pos > hex->getRangeEnd() ) // All equal?
		{
			cout << "Equal!" << endl;
		}

		delete hex_v;
	}

	if( programEEPROM || verifyEEPROM )
		delete hex;


	/* Program lock bits */
	if( programLockBits != -1 )
	{
		Util.log( "Programming lock bits...\r\n" );
		if( !prog->writeLockBits( programLockBits ) )
			throw new ErrorMsg( "Lock bit programming is not supported by this programmer!" );
	}


	/* Program fuse bits */
	if( programFuseBits != -1 )
	{
		if( !avr->getFuseStatus() )
			throw new ErrorMsg( "Selected device has no fuse bits!" );

		Util.log( "Programming fuse bits...\r\n" );
		if( !prog->writeFuseBits( programFuseBits ) )
			throw new ErrorMsg( "Fuse bit programming is not supported by this programmer!" );
	}


	/* Program extended fuse bits */
	if( programExtendedFuseBits != -1 )
	{
		if( !avr->getXFuseStatus() )
			throw new ErrorMsg( "Selected device has no extended fuse bits!" );

		Util.log( "Programming extended fuse bits...\r\n" );
		if( !prog->writeExtendedFuseBits( programExtendedFuseBits ) )
			throw new ErrorMsg( "Extended fuse bit programming is not supported by this programmer!" );
	}


	/* Verify lock bits */
	if( verifyLockBits != -1 )
	{
		Util.log( "Verifying lock bits...\r\n" );
		if( !prog->readLockBits( &bits ) )
			throw new ErrorMsg( "Lock bit readout is not supported by this programmer!" );
		if( bits == verifyLockBits )
			cout << "Equal!" << endl;
		else
			cout << "Unequal!" << endl;
	}


	/* Verify fuse bits */
	if( verifyFuseBits != -1 )
	{
		if( !avr->getFuseStatus() )
			throw new ErrorMsg( "Selected device has no fuse bits!" );

		Util.log( "Verifying fuse bits...\r\n" );
		if( !prog->readFuseBits( &bits ) )
			throw new ErrorMsg( "Fuse bit readout is not supported by this programmer!" );
		if( bits == verifyFuseBits )
			cout << "Equal!" << endl;
		else
			cout << "Unequal!" << endl;
	}


	/* Verify extended fuse bits */
	if( verifyExtendedFuseBits != -1 )
	{
		if( !avr->getXFuseStatus() )
			throw new ErrorMsg( "Selected device has no extended fuse bits!" );

		Util.log( "Verifying extended fuse bits...\r\n" );
		if( !prog->readExtendedFuseBits( &bits ) )
			throw new ErrorMsg( "Extended fuse bit readout is not supported by this programmer!" );
		if( bits == verifyExtendedFuseBits )
			cout << "Equal!" << endl;
		else
			cout << "Unequal!" << endl;
	}


	/* Read osccal value? */
	if( OSCCAL_Parameter != -1 )
	{
		/* Output to log if read from device */
		if( readOSCCAL )
		{
			Util.log( "Reading OSCCAL from device...\r\n" );
			pos = OSCCAL_Parameter;
			if( !prog->readOSCCAL( pos, &OSCCAL_Parameter ) )
				throw new ErrorMsg( "OSCCAL parameter readout is not supported by this programmer!" );
			cout << "0x" << std::hex << setw(2) << OSCCAL_Parameter << endl;
		}
	}


	/* Write OSCCAL to Flash? */
	if( OSCCAL_FlashAddress != -1 )
	{
		if( OSCCAL_Parameter == -1 )
			throw new ErrorMsg( "OSCCAL value not specified!" );

		Util.log( "Programming OSCCAL value to Flash...\r\n" );
		if( !prog->writeFlashByte( OSCCAL_FlashAddress, OSCCAL_Parameter ) )
			throw new ErrorMsg( "Flash programming is not supported by this programmer!" );
	}


	/* Write OSCCAL to EEPROM? */
	if( OSCCAL_EEPROMAddress != -1 )
	{
		if( OSCCAL_Parameter == -1 )
			throw new ErrorMsg( "OSCCAL value not specified!" );

		Util.log( "Programming OSCCAL value to EEPROM...\r\n" );
		if( !prog->writeEEPROMByte( OSCCAL_EEPROMAddress, OSCCAL_Parameter ) )
			throw new ErrorMsg( "EEPROM programming is not supported by this programmer!" );
	}

}


/* end of file */

