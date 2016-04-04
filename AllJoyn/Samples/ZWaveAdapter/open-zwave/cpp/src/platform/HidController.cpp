//-----------------------------------------------------------------------------
//
//	HidController.h
//
//	Cross-platform HID port handler
//
//	Copyright (c) 2010 Jason Frazier <frazierjason@gmail.com>
//
//	SOFTWARE NOTICE AND LICENSE
//
//	This file is part of OpenZWave.
//
//	OpenZWave is free software: you can redistribute it and/or modify
//	it under the terms of the GNU Lesser General Public License as published
//	by the Free Software Foundation, either version 3 of the License,
//	or (at your option) any later version.
//
//	OpenZWave is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU Lesser General Public License for more details.
//
//	You should have received a copy of the GNU Lesser General Public License
//	along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#include "Msg.h"
#include "platform/Thread.h"
#include "platform/Event.h"
#include "platform/Log.h"
#include "platform/TimeStamp.h"
#include "platform/HidController.h"
#include "hidapi.h"


#define CHECK_HIDAPI_RESULT(RESULT, ERRORLABEL) if (RESULT < 0) goto ERRORLABEL
#define PACKET_BUFFER_LENGTH 256

// are these specific to Wayne-Dalton?
#define FEATURE_REPORT_LENGTH 0x40
#define INPUT_REPORT_LENGTH 0x5
#define OUTPUT_REPORT_LENGTH 0x0

using namespace OpenZWave;

//-----------------------------------------------------------------------------
//	<HidController::HidController>
//	Constructor
//-----------------------------------------------------------------------------
HidController::HidController
(
):
	m_hHidController( NULL ),
	m_thread( NULL ),
	m_vendorId( 0x1b5f ),	// Wayne Dalton
	m_productId( 0x01 ),	// ControlThink ThinkStick
	m_serialNumber( "" ),
	m_hidControllerName( "" ),
	m_bOpen( false )
{
}

//-----------------------------------------------------------------------------
//	<HidController::~HidController>
//	Destructor
//-----------------------------------------------------------------------------
HidController::~HidController
(
)
{
}

//-----------------------------------------------------------------------------
//  <HidController::SetVendorId>
//  Set the USB vendor ID search value.  The HID port must be closed for the setting to be accepted.
//-----------------------------------------------------------------------------
bool HidController::SetVendorId
(
	uint32 const _vendorId
)
{
	if( m_bOpen )
	{
		return false;
	}

	m_vendorId = _vendorId;
	return true;
}

//-----------------------------------------------------------------------------
//  <HidController::SetProductId>
//  Set the USB product ID search value.  The HID port must be closed for the setting to be accepted.
//-----------------------------------------------------------------------------
bool HidController::SetProductId
(
	uint32 const _productId
)
{
	if( m_bOpen )
	{
		return false;
	}

	m_productId = _productId;
	return true;
}

//-----------------------------------------------------------------------------
//  <HidController::SetSerialNumber>
//  Set the USB serial number search value.  The HID port must be closed for the setting to be accepted.
//-----------------------------------------------------------------------------
bool HidController::SetSerialNumber
(
	string const& _serialNumber
)
{
	if( m_bOpen )
	{
		return false;
	}

	m_serialNumber = _serialNumber;
	return true;
}

//-----------------------------------------------------------------------------
//	<HidController::Open>
//	Open and configure a HID port
//-----------------------------------------------------------------------------
bool HidController::Open
(
	string const& _hidControllerName
)
{
	if( m_bOpen )
	{
		return false;
	}

	m_hidControllerName = _hidControllerName;

	// Try to init the serial port
	if( !Init( 1 ) )
	{
		// Failed.  We bail to allow the app a chance to take over, rather than retry
		// automatically.  Automatic retries only occur after a successful init.
		return false;
	}

	m_thread = new Thread( "HidController" );

	// Start the read thread
	m_thread->Start( ThreadEntryPoint, this );
	return true;
}

//-----------------------------------------------------------------------------
//	<HidController::Close>
//	Close a HID port
//-----------------------------------------------------------------------------
bool HidController::Close
(
)
{
	if( m_thread )
	{
		m_thread->Stop();
		m_thread->Release();
		m_thread = NULL;
	}

	hid_close( m_hHidController );
	m_hHidController = NULL;

	m_bOpen = false;
	hid_exit();
	return true;
}

//-----------------------------------------------------------------------------
// <HidController::ThreadEntryPoint>
// Entry point of the thread for receiving data from the HID port
//-----------------------------------------------------------------------------
void HidController::ThreadEntryPoint
(
	Event* _exitEvent,
	void* _context
)
{
	HidController* hc = (HidController*)_context;
	if( hc )
	{
		hc->ThreadProc( _exitEvent );
	}
}

//-----------------------------------------------------------------------------
// <HidController::ThreadProc>
// Handle receiving data
//-----------------------------------------------------------------------------
void HidController::ThreadProc
(
	Event* _exitEvent
)
{  
	uint32 attempts = 0;
	while( true )
	{
		// Init must have been called successfully during Open, so we
		// don't do it again until the end of the loop
		if( m_hHidController )
		{
			// Enter read loop.  Call will only return if
			// an exit is requested or an error occurs
			Read();

			// Reset the attempts, so we get a rapid retry for temporary errors
			attempts = 0;
		}

		if( attempts < 25 )		
		{
			// Retry every 5 seconds for the first two minutes...
			if( Wait::Single( _exitEvent, 5000 ) >= 0 )
			{
				// Exit signalled.
				break;
			}
		}
		else
		{
			// ...retry every 30 seconds after that
			if( Wait::Single( _exitEvent, 30000 ) >= 0 )
			{
				// Exit signalled.
				break;
			}
		}

		Init( ++attempts );
	}
}

//-----------------------------------------------------------------------------
// <HidController::Init>
// Open the HID port 
//-----------------------------------------------------------------------------
bool HidController::Init
(
	uint32 const _attempts
)
{
	// HIDAPI result
	int hidApiResult;
	const uint8 dataOutEnableZwave[3] = { 0x02, 0x01, 0x04 };

	hid_init();
	Log::Write( LogLevel_Info, "    Open HID port %s", m_hidControllerName.c_str() );
	m_hHidController = hid_open(m_vendorId, m_productId, NULL);
	if (!m_hHidController)
	{   
	        Log::Write( LogLevel_Error, "Cannot find specified HID port with VID:%04hx and PID:0x%04hx.", m_vendorId, m_productId );

		// Enumerate connected HIDs for debugging purposes
		// Note: most OS intentionally hide keyboard/mouse devices from HID access
		struct hid_device_info *devices, *currentDevice;
		devices = hid_enumerate(0x0, 0x0);
		currentDevice = devices;

		Log::Write( LogLevel_Error, "Enumerating connected HIDs:" );
		while (currentDevice)
		{
			Log::Write( LogLevel_Error, "\tVID:%04hx\tPID:0x%04hx\tSN:%ls\tMfg:%ls\tProd:%ls\tPath:%s",
				    currentDevice->vendor_id,
				    currentDevice->product_id,
				    currentDevice->serial_number,
				    currentDevice->manufacturer_string,
				    currentDevice->product_string,
				    currentDevice->path);
			currentDevice = currentDevice->next;
		}
		hid_free_enumeration(devices);

		goto HidOpenFailure;
	}

	wchar_t hidInfoString[255];
	hidInfoString[0] = 0x0000;
	Log::Write( LogLevel_Info, "    Found HID ZWave controller:");
	Log::Write( LogLevel_Info, "      Vendor ID:    0x%04hx", m_vendorId);
	Log::Write( LogLevel_Info, "      Product ID:   0x%04hx", m_productId);

	hidApiResult = hid_get_manufacturer_string(m_hHidController, hidInfoString, 255);
	if (hidApiResult < 0)
	{
	        Log::Write( LogLevel_Info, "      Manufacturer: <<ERROR READING: 0x%04hx>>", hidApiResult );
	}
	else
	{
	        Log::Write( LogLevel_Info, "      Manufacturer: %ls", hidInfoString );
	}

	hidApiResult = hid_get_product_string(m_hHidController, hidInfoString, 255);
	if (hidApiResult < 0)
	{
	        Log::Write( LogLevel_Info, "      Product name: <<ERROR READING: 0x%04hx>>", hidApiResult );
	}
	else
	{
	        Log::Write( LogLevel_Info, "      Product name: %ls", hidInfoString );
	}

	hidApiResult = hid_get_serial_number_string(m_hHidController, hidInfoString, 255);
	if (hidApiResult < 0)
	{
	        Log::Write( LogLevel_Warning, "Serial #:     <<ERROR READING: 0x%04hx>>", hidApiResult );
	}
	else
	{
	        size_t serialLength = wcslen(hidInfoString);
		char* serialHex = new char[serialLength + 1];
		memset(serialHex, 0, serialLength + 1);
		for (size_t i = 0; i < serialLength; ++i)
		{
			snprintf(&serialHex[i], serialLength - i + 1, "%hx", (unsigned short)(hidInfoString[i] & 0x0f));
		}
		Log::Write( LogLevel_Info, "      Serial #:     %ls   --> %s", hidInfoString, serialHex );
		delete [] serialHex;
	}
	Log::Write( LogLevel_Info, "" );

	// Turn on ZWave data via a short series of feature reports
	uint8 dataIn[FEATURE_REPORT_LENGTH];

	// Get Report ID 2
	// Not sure what the result is for, we don't use it so far
	hidApiResult = GetFeatureReport(FEATURE_REPORT_LENGTH, 0x02, dataIn );
	CHECK_HIDAPI_RESULT(hidApiResult, HidOpenFailure);
    
	// Send Report ID 2 - 1 byte "0x04"
	// Enables ZWave packet reports on ID 4 (tx) and ID 5 (rx)
	hidApiResult = SendFeatureReport(0x3, dataOutEnableZwave);
	CHECK_HIDAPI_RESULT(hidApiResult, HidOpenFailure);

	// Get Report ID 2
	// Not sure what the result is for, we don't use it so far
	hidApiResult = GetFeatureReport(FEATURE_REPORT_LENGTH, 0x02, dataIn );
	CHECK_HIDAPI_RESULT(hidApiResult, HidOpenFailure);

	// Ensure that reads for input reports are blocked.
	// Input report data is polled in Wait() to check if there are feature
	// reports waiting to be retrieved that contain ZWave rx packets.
	hidApiResult = hid_set_nonblocking(m_hHidController, 0);
	CHECK_HIDAPI_RESULT(hidApiResult, HidOpenFailure);

	// Open successful
	m_bOpen = true;
	return true;

HidOpenFailure:
	Log::Write( LogLevel_Error, "Failed to open HID port %s", m_hidControllerName.c_str() );
	const wchar_t* errString = hid_error(m_hHidController);
	Log::Write( LogLevel_Error, "HIDAPI ERROR STRING (if any): %ls", errString);
	hid_close(m_hHidController);
	m_hHidController = NULL;
	return false;
}

//-----------------------------------------------------------------------------
// <HidController::Read>
// Read data from the HID port
//-----------------------------------------------------------------------------
void HidController::Read
(
)
{
	uint8 buffer[FEATURE_REPORT_LENGTH];
	int bytesRead = 0;
	uint8 inputReport[INPUT_REPORT_LENGTH];
	TimeStamp readTimer;

 	while( true )
	{
		// Rx feature report buffer should contain
		// [0]      - 0x05 (rx feature report ID)
		// [1]      - length of rx data (or 0x00 and no further bytes if no rx data waiting)
		// [2]...   - rx data
	  	// We poll this waiting for data.
		bytesRead = GetFeatureReport(FEATURE_REPORT_LENGTH, 0x5, buffer);
		CHECK_HIDAPI_RESULT(bytesRead, HidPortError);
		if( bytesRead >= 2 )
		{

			if( buffer[1] > 0 )
			{
				string tmp = "";
				for (int i = 0; i < buffer[1]; i++)
				{
					char bstr[16];
					snprintf( bstr, sizeof(bstr), "0x%.2x ", buffer[2+i] );
					tmp += bstr;
				}
				Log::Write( LogLevel_Detail, "hid report read=%d ID=%d len=%d %s", bytesRead, buffer[0], buffer[1], tmp.c_str() );
			}

			if( buffer[1] > 0 )
			{
				Put( &buffer[2], buffer[1] );
			}
		}
		if( readTimer.TimeRemaining() <= 0 )
		{
			// Hang a hid_read to acknowledge receipt. Seems the response is conveying
			// transaction status.
			// Wayne-Dalton input report data is structured as follows (best guess):
			// [0] 0x03      - input report ID
			// [1] 0x01      - ??? never changes
			// [2] 0xNN      - if 0x01, no feature reports waiting
			//                 if 0x02, feature report ID 0x05 is waiting to be retrieved
			// [3,4] 0xNNNN  - Number of ZWave messages?

			int hidApiResult = hid_read( m_hHidController, inputReport, INPUT_REPORT_LENGTH );
			//if( hidApiResult != 0 )
			//{
			//	string tmp = "";
			//	for( int i = 0; i < INPUT_REPORT_LENGTH; i++ )
			//	{
			//		char bstr[16];
			//		snprintf(bstr, sizeof(bstr), "%02x ", inputReport[i] );
			//		tmp += bstr;
			//	}
			//	Log::Write( LogLevel_Detail, "hid read %d %s", hidApiResult, tmp.c_str() );
			//}

			if( hidApiResult == -1 )
			{
				const wchar_t* errString = hid_error(m_hHidController);
				Log::Write( LogLevel_Warning, "Error: HID port returned error reading input bytes: 0x%08hx, HIDAPI error string: %ls", hidApiResult, errString );
			}
			readTimer.SetTime( 100 );
		}
		m_thread->Sleep(10);
	}

HidPortError:
	Log::Write( LogLevel_Warning, "Error: HID port returned error reading rest of packet: 0x%08hx, HIDAPI error string:", bytesRead );
	Log::Write( LogLevel_Warning, "%ls", hid_error(m_hHidController));
}

//-----------------------------------------------------------------------------
// <HidController::Write>
// Send data to the HID port
//-----------------------------------------------------------------------------
uint32 HidController::Write
(
	uint8* _buffer,
	uint32 _length
)
{
	if( !m_bOpen )
	{
		//Error
		Log::Write( LogLevel_Warning, "Error: HID port must be opened before writing" );
		return 0;
	}

	if ( FEATURE_REPORT_LENGTH - 2 < _length)
	{
		//Error
		Log::Write( LogLevel_Info, "Error: Write buffer length %d exceeded feature report data capacity %d", _length, FEATURE_REPORT_LENGTH - 2 );
		return 0;
	}

	// transmit buffer should be arranged:
	// byte 0 - 0x04 (tx feature report)
	// byte 1 - length of tx data
	// byte 2....  - tx data
	uint8 hidBuffer[FEATURE_REPORT_LENGTH];
	memset(hidBuffer, 0, FEATURE_REPORT_LENGTH);
	hidBuffer[0] = 0x4;
	hidBuffer[1] = (uint8)_length;
	memcpy(&hidBuffer[2], _buffer, _length);

	Log::Write( LogLevel_Debug, "      HidController::Write (sent to controller)" );
	LogData(_buffer, _length, "      Write: ");

	int bytesSent = SendFeatureReport(FEATURE_REPORT_LENGTH, hidBuffer);
	if (bytesSent < 2)
	{
		//Error
		const wchar_t* errString = hid_error(m_hHidController);
		Log::Write( LogLevel_Warning, "Error: HID port returned error sending bytes: 0x%08hx, HIDAPI error string: %ls", bytesSent, errString );
		return 0;
	}

	return (uint32)bytesSent - 2;
}

//-----------------------------------------------------------------------------
//	<HidController::GetFeatureReport>
//	Read bytes from the specified HID feature report
//-----------------------------------------------------------------------------
int HidController::GetFeatureReport
(
	uint32 _length,
	uint8  _reportId,
	uint8* _buffer
)
{
	int result;
	_buffer[0] = _reportId;
	result = hid_get_feature_report(m_hHidController, _buffer, _length);
	if (result < 0)
	{
	        Log::Write( LogLevel_Info, "Error: HID GetFeatureReport on ID 0x%hx returned (0x%.8x)", _reportId, result );
	}
	return result;
}

//-----------------------------------------------------------------------------
//	<HidController::SendFeatureReport>
//	Write bytes to the specified HID feature report
//-----------------------------------------------------------------------------
int HidController::SendFeatureReport
(
	uint32 _length,
	const uint8* _data
)
{
	int result;

	result = hid_send_feature_report(m_hHidController, _data, _length);
	if (result < 0)
	{
		Log::Write( LogLevel_Info, "Error: HID SendFeatureReport on ID 0x%hx returned (0x%.8x)", _data[0], result );
	}
	return result;
}

