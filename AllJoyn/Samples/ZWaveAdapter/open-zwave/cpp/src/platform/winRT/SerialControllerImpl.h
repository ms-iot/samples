//-----------------------------------------------------------------------------
//
//	SerialControllerImpl.h
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

#ifndef _SerialControllerImpl_H
#define _SerialControllerImpl_H

#include <Windows.h>
#include <ppltasks.h>

#include "Defs.h"
#include "platform/SerialController.h"

namespace OpenZWave
{
	class SerialControllerImpl
	{
	private:
		friend class SerialController;

		SerialControllerImpl( SerialController* _owner );
		~SerialControllerImpl();

		bool Open();
		void Close();

		uint32 Write( uint8* _buffer, uint32 _length );
		void StartReadTask();

		Windows::Devices::SerialCommunication::SerialDevice ^ m_serialDevice;
		Concurrency::cancellation_token_source   m_readTaskCancelationTokenSource;

		SerialController*			m_owner;
	};

} // namespace OpenZWave

#endif //_SerialControllerImpl_H

