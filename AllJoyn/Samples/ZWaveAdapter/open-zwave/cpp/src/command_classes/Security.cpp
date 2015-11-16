//-----------------------------------------------------------------------------
//
//	Security.cpp
//
//	Implementation of the Z-Wave COMMAND_CLASS_Security
//
//	Copyright (c) 2011 Mal Lansell <openzwave@lansell.org>
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

#include <ctime>


#include "command_classes/CommandClasses.h"
#include "command_classes/Security.h"
#include "Defs.h"
#include "Msg.h"
#include "Node.h"
#include "Driver.h"
#include "platform/Log.h"
#include "Utils.h"

#include "value_classes/ValueBool.h"


using namespace OpenZWave;



Security::Security
(
		uint32 const _homeId,
		uint8 const _nodeId
):
CommandClass( _homeId, _nodeId ),
m_schemeagreed(false),
m_secured(false)

{
	/* We don't want the Driver to route "Security" messages back to us for Encryption,
	 * so disable SecureSupport for the Security Command Class
	 * (This stops this Command Class getting Marked as as IsSecured() if its listed
	 * in the SecurityCmd_SupportedReport from the device - Which some devices do)
	 */
	ClearSecureSupport();

}

Security::~Security
(
)
{
}

//-----------------------------------------------------------------------------
// <Version::ReadXML>
// Read configuration.
//-----------------------------------------------------------------------------
void Security::ReadXML
(
		TiXmlElement const* _ccElement
)
{
	CommandClass::ReadXML( _ccElement );
}

//-----------------------------------------------------------------------------
// <Version::WriteXML>
// Save changed configuration
//-----------------------------------------------------------------------------
void Security::WriteXML
(
		TiXmlElement* _ccElement
)
{
	CommandClass::WriteXML( _ccElement );
}



bool Security::Init
(
)
{
	Msg* msg = new Msg( "SecurityCmd_SupportedGet", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
	msg->Append( GetNodeId() );
	msg->Append( 2 );
	msg->Append( GetCommandClassId() );
	msg->Append( SecurityCmd_SupportedGet );
	msg->Append( GetDriver()->GetTransmitOptions() );
	msg->setEncrypted();
	GetDriver()->SendMsg( msg, Driver::MsgQueue_Security);
	return true;
}
//-----------------------------------------------------------------------------
// <Security::RequestState>
// Request current state from the device
//-----------------------------------------------------------------------------
bool Security::ExchangeNetworkKeys
(
)
{
	if (GetNodeUnsafe()->IsAddingNode()) {
		Msg * msg = new Msg ("SecurityCmd_SchemeGet", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
		msg->Append( GetNodeId() );
		msg->Append( 3 );
		msg->Append( GetCommandClassId() );
		msg->Append( SecurityCmd_SchemeGet );
		msg->Append( 0 );
		msg->Append( GetDriver()->GetTransmitOptions() );
		/* SchemeGet is unencrypted */
		GetDriver()->SendMsg(msg, Driver::MsgQueue_Security);
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// <Security::RequestState>
// Request current state from the device
//-----------------------------------------------------------------------------
bool Security::RequestState
(
		uint32 const _requestFlags,
		uint8 const _instance,
		Driver::MsgQueue const _queue
)
{
#if 0
	if( _requestFlags & RequestFlag_Static )
	{

	}
	return false;
#endif
	return true;
}

//-----------------------------------------------------------------------------
// <Security::RequestValue>
// Request current state from the device
//-----------------------------------------------------------------------------
bool Security::RequestValue
(
		uint32 const _requestFlags,
		uint8 const _index,
		uint8 const _instance,
		Driver::MsgQueue const _queue
)
{
	Log::Write(LogLevel_Info, GetNodeId(), "Got a RequestValue Call");
	return true;
}


bool Security::HandleSupportedReport
(
		uint8 const* _data,
		uint32 const _length
)
{

#ifdef DEBUG
	PrintHex("Security Classes", _data, _length);
#endif
	GetNodeUnsafe()->SetSecuredClasses(_data, _length);
	return true;
}

//-----------------------------------------------------------------------------
// <Security::HandleMsg>
// Handle a message from the Z-Wave network
//-----------------------------------------------------------------------------
bool Security::HandleMsg
(
		uint8 const* _data,
		uint32 const _length,
		uint32 const _instance	// = 1
)
{
	switch( (SecurityCmd)_data[0] )
	{
		case SecurityCmd_SupportedReport:
		{
			/* this is a list of CommandClasses that should be Encrypted.
			 * and it might contain new command classes that were not present in the NodeInfoFrame
			 * so we have to run through, mark existing Command Classes as SetSecured (so SendMsg in the Driver
			 * class will route the unecrypted messages to our SendMsg) and for New Command
			 * Classes, create them, and of course, also do a SetSecured on them.
			 *
			 * This means we must do a SecurityCmd_SupportedGet request ASAP so we dont have
			 * Command Classes created after the Discovery Phase is completed!
			 */
			Log::Write(LogLevel_Info, GetNodeId(), "Received SecurityCmd_SupportedReport from node %d", GetNodeId() );
			m_secured = true;
			if( ValueBool* value = static_cast<ValueBool*>( GetValue( _instance, 0 ) ) )
			{
				value->OnValueRefreshed( m_secured );
				value->Release();
			}
			HandleSupportedReport(&_data[2], _length-2);
			break;
		}
		case SecurityCmd_SchemeReport:
		{
			Log::Write(LogLevel_Info, GetNodeId(), "Received SecurityCmd_SchemeReport from node %d: %d", GetNodeId(), _data[1]);
			uint8 schemes = _data[1];
			if (m_schemeagreed == true) {
				Log::Write(LogLevel_Warning, GetNodeId(), "   Already Received a SecurityCmd_SchemeReport from the node. Ignoring");
				break;
			}
			if( schemes == SecurityScheme_Zero )
			{
				/* We're good to go.  We now should send our NetworkKey to the device if this is the first
				 * time we have seen it
				 */
				Log::Write(LogLevel_Info, GetNodeId(), "    Security scheme agreed." );
				/* create the NetworkKey Packet. EncryptMessage will encrypt it for us (And request the NONCE) */
				Msg * msg = new Msg ("SecurityCmd_NetworkKeySet", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
				msg->Append( GetNodeId() );
				msg->Append( 18 );
				msg->Append( GetCommandClassId() );
				msg->Append( SecurityCmd_NetworkKeySet );
				for (int i = 0; i < 16; i++)
					msg->Append(GetDriver()->GetNetworkKey()[i]);
				msg->Append( GetDriver()->GetTransmitOptions() );
				msg->setEncrypted();
				GetDriver()->SendMsg( msg, Driver::MsgQueue_Security);
				m_schemeagreed = true;
			}
			else
			{
				/* No common security scheme.  The device should continue as an unsecured node.
				 * but Some Command Classes might not be present...
				 */
				Log::Write(LogLevel_Warning,  GetNodeId(), "    No common security scheme.  The device will continue as an unsecured node." );
			}
			break;
		}
		case SecurityCmd_NetworkKeySet:
		{
			/* we shouldn't get a NetworkKeySet from a node if we are the controller
			 * as we send it out to the Devices
			 */
			Log::Write(LogLevel_Info,  GetNodeId(), "Received SecurityCmd_NetworkKeySet from node %d", GetNodeId() );
			break;
		}
		case SecurityCmd_NetworkKeyVerify:
		{
			/* if we can decrypt this packet, then we are assured that our NetworkKeySet is successfull
			 * and thus should set the Flag referenced in SecurityCmd_SchemeReport
			 */
			Log::Write(LogLevel_Info,  GetNodeId(), "Received SecurityCmd_NetworkKeyVerify from node %d", GetNodeId() );
			/* now as for our SupportedGet */
			Msg* msg = new Msg( "SecurityCmd_SupportedGet", GetNodeId(), REQUEST, FUNC_ID_ZW_SEND_DATA, true, true, FUNC_ID_APPLICATION_COMMAND_HANDLER, GetCommandClassId() );
			msg->Append( GetNodeId() );
			msg->Append( 2 );
			msg->Append( GetCommandClassId() );
			msg->Append( SecurityCmd_SupportedGet );
			msg->Append( GetDriver()->GetTransmitOptions() );
			msg->setEncrypted();
			GetDriver()->SendMsg( msg, Driver::MsgQueue_Security);

			break;
		}
		case SecurityCmd_SchemeInherit:
		{
			/* only used in a Controller Replication Type enviroment.
			 *
			 */
			Log::Write(LogLevel_Info,  GetNodeId(), "Received SecurityCmd_SchemeInherit from node %d", GetNodeId() );
			break;
		}
		/* the rest of these should be handled by the Driver Code (in Driver::ProcessMsg) */
		case SecurityCmd_NonceGet:
		case SecurityCmd_NonceReport:
		case SecurityCmd_MessageEncap:
		case SecurityCmd_MessageEncapNonceGet:
		{
			Log::Write(LogLevel_Warning, GetNodeId(), "Recieved a Security Message that should have been handled in the Driver");
			break;
		}
		default:
		{
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// <Security::CreateVars>
// Create the values managed by this command class
//-----------------------------------------------------------------------------
void Security::CreateVars
(
		uint8 const _instance
)
{
	if( Node* node = GetNodeUnsafe() )
	{
		node->CreateValueBool( ValueID::ValueGenre_System, GetCommandClassId(), _instance, 0, "Secured", "", true, false, false, 0 );
	}
}

