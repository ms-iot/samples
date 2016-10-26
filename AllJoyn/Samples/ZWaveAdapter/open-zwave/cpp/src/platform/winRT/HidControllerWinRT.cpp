//-----------------------------------------------------------------------------
//
//	HidControllerWinRT.cpp
//
//	WinRT implementation of a HidController
//
//	Copyright (c) 2015 Microsoft Corporation
//	All rights reserved.
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

#include "Defs.h"
#include "HidControllerWinRT.h"
#include "platform/Log.h"

#include <ppltasks.h>
#include <winstring.h>

using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::HumanInterfaceDevice;
using namespace Windows::Devices::Usb;
using namespace Windows::Foundation;
using namespace Windows::Storage::Streams;
using namespace Platform;
using namespace Concurrency;
using namespace OpenZWave;

#define AQS_FORMAT L"System.Devices.InterfaceClassGuid:=\"{4D1E55B2-F16F-11CF-88CB-001111000030}\" AND System.Devices.InterfaceEnabled:=System.StructuredQueryType.Boolean#True AND System.DeviceInterface.Hid.VendorId:=%04d AND System.DeviceInterface.Hid.ProductId:=%04d"
#define AQS_LENGTH 300

//-----------------------------------------------------------------------------
//	<HidControllerImpl::HidControllerImpl>
//	Constructor
//-----------------------------------------------------------------------------
HidController::HidController() :
	m_vendorId(0x1b5f),	// Wayne Dalton
	m_productId(0x01),	// ControlThink ThinkStick
	m_serialNumber(""),
	m_hidControllerName(""),
	m_bOpen(false)
{
}

//-----------------------------------------------------------------------------
//	<HidControllerImpl::~HidControllerImpl>
//	Destructor
//-----------------------------------------------------------------------------
HidController::~HidController()
{
	if (m_device != nullptr)
	{
		m_device->InputReportReceived -= m_inputReportEventToken;
	}
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
	if (m_bOpen)
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
bool HidController::SetProductId (uint32 const _productId)
{
	if (m_bOpen)
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
bool HidController::SetSerialNumber (string const& _serialNumber)
{
	if (m_bOpen)
	{
		return false;
	}

	m_serialNumber = _serialNumber;
	return true;
}

//-----------------------------------------------------------------------------
//	<HidControllerImpl::Open>
//	Open and configure a HID port
//-----------------------------------------------------------------------------
bool HidController::Open(string const& _hidControllerName)
{
	if (m_bOpen)
	{
		return false;
	}

	m_hidControllerName = _hidControllerName;

	bool success = false;
	try
	{
		create_task(Init()).then([&success, this](bool initResult)
		{
			success = initResult;
			if (success && m_device != nullptr)
			{
				m_inputReportEventToken = m_device->InputReportReceived += ref new TypedEventHandler<HidDevice^, HidInputReportReceivedEventArgs^>
					([this](HidDevice ^sender, HidInputReportReceivedEventArgs ^args)
				{
					auto reader = DataReader::FromBuffer(args->Report->Data);
					uint32 bufferSize = reader->UnconsumedBufferLength;
					std::vector<uint8> data(bufferSize);

					if (!data.empty())
					{
						reader->ReadBytes(::Platform::ArrayReference<uint8>(&data[0], bufferSize));
						Put(&data[0], bufferSize);
					}
				});
			}

		}).wait();
	}
	catch (Platform::Exception^ ex)
	{
	}

	return success;
}

//-----------------------------------------------------------------------------
//	<HidControllerImpl::Close>
//	Close a HID port
//-----------------------------------------------------------------------------
bool HidController::Close()
{
	if (m_device != nullptr)
	{
		m_device->InputReportReceived -= m_inputReportEventToken;
		delete m_device;
		m_device = nullptr;
	}
	return true;
}

//-----------------------------------------------------------------------------
// <HidControllerImpl::Init>
// Open the HID port
//-----------------------------------------------------------------------------
task<bool> HidController::Init()
{
	// Yields the same as API above w/o the usage page and usage Id filters
	wchar_t buffer[AQS_LENGTH];
	swprintf_s(buffer, AQS_FORMAT, m_vendorId, m_productId);
	auto selector = ref new String(buffer);

	return create_task(Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(selector))
		.then([this](DeviceInformationCollection ^ devices) -> String ^
	{
		String ^deviceId = L"";
		for (auto iterator = devices->First(); iterator->HasCurrent; iterator->MoveNext())
		{
			// Not sure how to differentiate when there are multiple things returned.
			// Just return first matching ID for now
			deviceId = iterator->Current->Id;
			break;
		}
		return deviceId;

	}).then([this](String ^deviceId) -> IAsyncOperation<HidDevice ^> ^
	{
		return HidDevice::FromIdAsync(deviceId, Windows::Storage::FileAccessMode::Read);

	}).then([this](task<HidDevice ^> deviceTask) -> bool
	{
		try
		{
			m_device = deviceTask.get();

			if (m_device == nullptr)
			{
				return false;
			}

			// Send Report ID 2 - 1 byte "0x04"
			// Enables ZWave packet reports on ID 4 (tx) and ID 5 (rx)
			uint8 data = 0x04;
			SendFeatureReport(&data, 1, 2);

			return true;
		}
		catch (Platform::Exception^ ex)
		{
			return false;
		}
	});
}

//-----------------------------------------------------------------------------
// <HidControllerImpl::Write>
// Send data to the HID port
//-----------------------------------------------------------------------------
uint32 HidController::Write
(
	uint8* _buffer,
	uint32 _length
)
{
	// report Id 0x04 is tx feature report
	return SendFeatureReport(_buffer, _length, 0x04);
}

//-----------------------------------------------------------------------------
// <HidController::SendFeatureReport>
// Send a feature report with the specified data and report ID
//-----------------------------------------------------------------------------
uint32 HidController::SendFeatureReport
(
	uint8* _buffer,
	uint32 _length,
	unsigned short reportId
)
{
	auto featureReport = m_device->CreateFeatureReport();
	auto dataWriter = ref new DataWriter();

	auto array = ref new Array<uint8>(_buffer, _length);
	dataWriter->WriteBytes(ArrayReference<uint8>(_buffer, _length));

	featureReport->Data = dataWriter->DetachBuffer();

	uint32 bytesWritten = 0;
	try
	{
		bytesWritten = create_task(m_device->SendFeatureReportAsync(featureReport)).get();
	}
	catch (Platform::Exception^)
	{
	}
	return bytesWritten;
}
