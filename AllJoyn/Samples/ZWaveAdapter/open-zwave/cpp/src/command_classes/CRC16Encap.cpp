//-----------------------------------------------------------------------------
//
//	CRC16Encap.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_CRC_16_ENCAP
//
//	Copyright (c) 2012 Greg Satz <satz@iranger.com>
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

#include "command_classes/CommandClasses.h"
#include "command_classes/CRC16Encap.h"
#include "Defs.h"
#include "Msg.h"
#include "Node.h"
#include "Driver.h"
#include "platform/Log.h"

using namespace OpenZWave;

//
// CRC-CCITT (0x1D0F)
//
uint16 crc16(uint8 const * data_p, uint32 const _length){
    uint8 x;
    uint16 crc = 0xF6AF; // 0x1D0F with first byte 0x56;
	uint32 length = _length;

    while (length--){
        x = crc >> 8 ^ *data_p++;
        x ^= x>>4;
        crc = (crc << 8) ^ ((uint16)(x << 12)) ^ ((uint16)(x <<5)) ^ ((uint16)x);
    }
    return crc;
}

enum CRC16EncapCmd
{
	CRC16EncapCmd_Encap = 0x01
};


//-----------------------------------------------------------------------------
// <CRC16Encap::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
bool CRC16Encap::HandleMsg
(
	uint8 const* _data,
	uint32 const _length,
	uint32 const _instance	// = 1
)
{
	if( CRC16EncapCmd_Encap == (CRC16EncapCmd)_data[0] )
	{
		Log::Write( LogLevel_Info, GetNodeId(), "Received CRC16-command from node %d", GetNodeId());

		uint16 crcM = (_data[_length - 3] << 8) + _data[_length - 2] ; // crc as reported in msg
		uint16 crcC = crc16(&_data[0], _length - 3 );				   // crc calculated

		if ( crcM != crcC )
		{
			Log::Write( LogLevel_Info, GetNodeId(), "CRC check failed, message contains 0x%.4x but should be 0x%.4x", crcM, crcC);
			return false;
		}

		if( Node const* node = GetNodeUnsafe() )
		{
			uint8 commandClassId = _data[1];

			if( CommandClass* pCommandClass = node->GetCommandClass( commandClassId ) )
			{
				pCommandClass->HandleMsg( &_data[2], _length - 4 );
			}
		}

		return true;
	}
	return false;
}
