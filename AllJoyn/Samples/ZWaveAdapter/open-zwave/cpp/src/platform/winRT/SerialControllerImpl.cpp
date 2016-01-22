//-----------------------------------------------------------------------------
//
//	SerialControllerImpl.cpp
//
//	WinRT Implementation of the cross-platform serial port
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
#include "SerialControllerImpl.h"

#include "platform/Log.h"
#include <winstring.h>
#include <ppltasks.h>

using namespace OpenZWave;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;
using namespace Concurrency;
using namespace Platform;

//-----------------------------------------------------------------------------
// <SerialControllerImpl::SerialControllerImpl>
// Constructor
//-----------------------------------------------------------------------------
SerialControllerImpl::SerialControllerImpl(	SerialController* _owner)
	: m_owner( _owner )
{
}

//-----------------------------------------------------------------------------
// <SerialControllerImpl::~SerialControllerImpl>
// Destructor
//-----------------------------------------------------------------------------
SerialControllerImpl::~SerialControllerImpl()
{
	Close();
}

//-----------------------------------------------------------------------------
// <SerialControllerImpl::Close>
// Close the serial port
//-----------------------------------------------------------------------------
void SerialControllerImpl::Close()
{
	// cancel read task
	m_readTaskCancelationTokenSource.cancel();
}

//-----------------------------------------------------------------------------
// <SerialControllerImpl::Open>
// Open the serial port
//-----------------------------------------------------------------------------
bool SerialControllerImpl::Open()
{
	Log::Write(LogLevel_Info, "Trying to open serial port %s", m_owner->m_serialControllerName.c_str());

	try
	{
        auto selector = SerialDevice::GetDeviceSelector();

		return create_task(DeviceInformation::FindAllAsync(selector))
			.then([this](DeviceInformationCollection ^ devices) -> IAsyncOperation<SerialDevice ^> ^
		{
			wstring ourId(m_owner->m_serialControllerName.begin(), m_owner->m_serialControllerName.end());
			for (auto iterator = devices->First(); iterator->HasCurrent; iterator->MoveNext())
			{
				wstring currentId = iterator->Current->Id->Data();
				if (currentId.find(ourId) != wstring::npos)
				{
					return SerialDevice::FromIdAsync(iterator->Current->Id);
				}
			}
			return create_async([]() -> SerialDevice ^ { return nullptr; });

		}).then([this](SerialDevice ^ device) -> bool
		{
			if (device == nullptr)
			{
				return false;
			}
			m_serialDevice = device;

			m_serialDevice->BaudRate = m_owner->m_baud;
			m_serialDevice->DataBits = 8;
			switch (m_owner->m_stopBits)
			{
			case SerialController::StopBits::StopBits_One:
			{
				m_serialDevice->StopBits = SerialStopBitCount::One;
				break;
			}
			case SerialController::StopBits::StopBits_OneAndAHalf:
			{
				m_serialDevice->StopBits = SerialStopBitCount::OnePointFive;
				break;
			}
			case SerialController::StopBits::StopBits_Two:
			{
				m_serialDevice->StopBits = SerialStopBitCount::Two;
				break;
			}
			}

			switch (m_owner->m_parity)
			{
			case SerialController::Parity::Parity_Even:
			{
				m_serialDevice->Parity = SerialParity::Even;
				break;
			}
			case SerialController::Parity::Parity_Mark:
			{
				m_serialDevice->Parity = SerialParity::Mark;
				break;
			}
			case SerialController::Parity::Parity_None:
			{
				m_serialDevice->Parity = SerialParity::None;
				break;
			}
			case SerialController::Parity::Parity_Odd:
			{
				m_serialDevice->Parity = SerialParity::Odd;
				break;
			}
			case SerialController::Parity::Parity_Space:
			{
				m_serialDevice->Parity = SerialParity::Space;
				break;
			}
			}

			Windows::Foundation::TimeSpan timespan;
			timespan.Duration = 1;
			m_serialDevice->ReadTimeout = timespan;

			StartReadTask();

			return true;
		}).get();
	}
	catch (...)
	{
		return false;
	}
}

//-----------------------------------------------------------------------------
// <SerialControllerImpl::StartReadTask>
// Start a background task which reads available data and passes it along to SerialController
//-----------------------------------------------------------------------------
void SerialControllerImpl::StartReadTask()
{
	// Read serial data on background task
	cancellation_token token = m_readTaskCancelationTokenSource.get_token();

	create_task([token, this]()
	{
		uint32 readBufferLength = 512;
		Buffer ^ buffer = ref new Buffer(readBufferLength);

		for (;;)
		{
			try
			{
				create_task(m_serialDevice->InputStream->ReadAsync(buffer, readBufferLength, InputStreamOptions::None))
					.then([&, this](IBuffer ^ outBuffer)
				{
					auto reader = DataReader::FromBuffer(outBuffer);
					auto bytesRead = reader->UnconsumedBufferLength;

					std::vector<uint8> byteVector(bytesRead);
					if (!byteVector.empty())
					{
						reader->ReadBytes(::Platform::ArrayReference<uint8>(byteVector.data(), bytesRead));
						m_owner->Put(byteVector.data(), bytesRead);
					}
				}).wait();
			}
			catch (Platform::Exception^ ex)
			{
				if (ex->HResult == HRESULT_FROM_WIN32(ERROR_OPERATION_ABORTED))
				{
					m_owner->Close();
				}
			}

			if (token.is_canceled())
			{
				cancel_current_task();
			}
		}
	}, token);
}

//-----------------------------------------------------------------------------
// <SerialControllerImpl::Write>
// Send data to the serial port
//-----------------------------------------------------------------------------
uint32 SerialControllerImpl::Write
(
	uint8* _buffer,
	uint32 _length
)
{
	uint32 retVal = 0;

	if (m_serialDevice == nullptr)
	{
		//Error
		Log::Write(LogLevel_Error, "ERROR: Serial port must be opened before writing\n");
		return 0;
	}

	DataWriter ^ writer = ref new DataWriter();
	writer->WriteBytes(ref new Platform::Array<uint8>(_buffer, _length));
	try
	{
		auto writeTask = create_task(m_serialDevice->OutputStream->WriteAsync(writer->DetachBuffer()));

		// since the consumer of this function expects this to be synchronous, just wait here.
		retVal = writeTask.get();
	}
	catch (Platform::Exception^ )
	{
		//ignore - return 0
		retVal = 0;
	}

	return retVal;
}
