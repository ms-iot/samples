//-----------------------------------------------------------------------------
//
//	CentralScene.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_CENTRAL_SCENE
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
#include "command_classes/CentralScene.h"
#include "Defs.h"
#include "Msg.h"
#include "Node.h"
#include "Driver.h"
#include "platform/Log.h"
#include "value_classes/ValueInt.h"

#include "tinyxml.h"

using namespace OpenZWave;

enum CentralSceneCmd
{
	CentralSceneCmd_Capability_Get    = 0x01,
	CentralSceneCmd_Capability_Report = 0x02,
	CentralSceneCmd_Set				  = 0x03
};


enum CentralScene_ValueID_Index
{
	CentralScene_Count		= 0x00
};

//-----------------------------------------------------------------------------
// <CentralScene::CentralScene>
// Constructor
//-----------------------------------------------------------------------------
CentralScene::CentralScene
(
		uint32 const _homeId,
		uint8 const _nodeId
):
CommandClass( _homeId, _nodeId ),
m_scenecount(0)
{
	Log::Write(LogLevel_Info, GetNodeId(), "CentralScene - Created %d", HasStaticRequest( StaticRequest_Values ));
}


//-----------------------------------------------------------------------------
// <CentralScene::RequestState>
// Request current state from the device
//-----------------------------------------------------------------------------
bool CentralScene::RequestState
(
		uint32 const _requestFlags,
		uint8 const _instance,
		Driver::MsgQueue const _queue
)
{
	Log::Write(LogLevel_Info, GetNodeId(), "CentralScene RequestState: %d", _requestFlags);
	bool requests = false;
	if( ( _requestFlags & RequestFlag_AfterMark ))
	{
			requests = RequestValue( _requestFlags, CentralSceneCmd_Capability_Get, _instance, _queue );
	} else {
		Log::Write(LogLevel_Info, GetNodeId(), "CentralScene: Not a StaticRequest");
	}
	return requests;
}

//-----------------------------------------------------------------------------
// <CentralScene::RequestValue>
// Request current value from the device
//-----------------------------------------------------------------------------
bool CentralScene::RequestValue
(
		uint32 const _requestFlags,
		uint8 const _what,
		uint8 const _instance,
		Driver::MsgQueue const _queue
)
{
	if (_what == CentralSceneCmd_Capability_Get) {
		Msg* msg = new Msg( "CentralSceneCmd_Capability_Get", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
		msg->SetInstance( this, _instance );
		msg->Append( GetNodeId() );
		msg->Append( 2 );
		msg->Append( GetCommandClassId() );
		msg->Append( _what );
		msg->Append( GetDriver()->GetTransmitOptions() );
		GetDriver()->SendMsg( msg, _queue );
	}
	return true;
}
//-----------------------------------------------------------------------------
// <CentralScene::ReadXML>
// Class specific configuration
//-----------------------------------------------------------------------------
void CentralScene::ReadXML
(
		TiXmlElement const* _ccElement
)
{
	int32 intVal;

	CommandClass::ReadXML( _ccElement );
	if( TIXML_SUCCESS == _ccElement->QueryIntAttribute( "scenecount", &intVal ) )
	{
		m_scenecount = intVal;
	}
}

//-----------------------------------------------------------------------------
// <CentralScene::WriteXML>
// Class specific configuration
//-----------------------------------------------------------------------------
void CentralScene::WriteXML
(
		TiXmlElement* _ccElement
)
{
	char str[32];

	CommandClass::WriteXML( _ccElement );
	snprintf( str, sizeof(str), "%d", m_scenecount );
	_ccElement->SetAttribute( "scenecount", str);
}


//-----------------------------------------------------------------------------
// <CentralScene::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
bool CentralScene::HandleMsg
(
		uint8 const* _data,
		uint32 const _length,
		uint32 const _instance	// = 1
)
{
	if( CentralSceneCmd_Set == (CentralSceneCmd)_data[0] )
	{
		// Central Scene Set received so send notification
		int32 when;
		if( _data[2] == 0 )
			when = 0;
		else if( _data[2] <= 0x7F )
			when = _data[2];
		else if( _data[2] <= 0xFE )
			when = 60 * _data[2];
		else
			when = 0;
		Log::Write( LogLevel_Info, GetNodeId(), "Received Central Scene set from node %d: scene id=%d in %s seconds. Sending event notification.", GetNodeId(), _data[3], when);

		if( ValueInt* value = static_cast<ValueInt*>( GetValue( _instance, _data[3] ) ) )
		{
			value->OnValueRefreshed( when );
			value->Release();
		} else {
			Log::Write( LogLevel_Warning, GetNodeId(), "No ValueID created for Scene %d", _data[3]);
			return false;
		}
		return true;
	} else if (CentralSceneCmd_Capability_Report == (CentralSceneCmd)_data[0]) {
		/* Create a Number of ValueID's based on the m_scenecount variable
		 * We prefer what the Config File specifies rather than what is returned by
		 * the Device...
		 */
		int scenecount = _data[1];
		if (m_scenecount != 0)
			m_scenecount = scenecount;

		if ( ValueInt* value = static_cast<ValueInt*>( GetValue( _instance, CentralScene_Count)))
		{
			value->OnValueRefreshed(m_scenecount);
			value->Release();
		} else {
			Log::Write( LogLevel_Warning, GetNodeId(), "Can't find ValueID for SceneCount");
		}

		if( Node* node = GetNodeUnsafe() )
		{
				char lbl[64];
				for (int i = 1; i <= m_scenecount; i++) {
					snprintf(lbl, 64, "Scene %d", i);
					node->CreateValueInt(ValueID::ValueGenre_User, GetCommandClassId(), _instance, i, lbl, "", true, false, 0, 0 );
				}

		} else {
			Log::Write(LogLevel_Info, GetNodeId(), "CentralScene: Can't find Node!");
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// <CentralScene::CreateVars>
// Create the values managed by this command class
//-----------------------------------------------------------------------------
void CentralScene::CreateVars
(
		uint8 const _instance
)
{
	if( Node* node = GetNodeUnsafe() )
	{
		node->CreateValueInt( ValueID::ValueGenre_System, GetCommandClassId(), _instance, CentralScene_Count, "Scene Count", "", true, false, 0, 0 );
	}
}


