//-----------------------------------------------------------------------------
//
//	Security.cpp
//
//	Common Security/Encryption Routines
//
//	Copyright (c) 2015 Justin Hammond <justin@dynam.ac>
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
#include "Notification.h"
#include "Driver.h"

using namespace OpenZWave;


//-----------------------------------------------------------------------------
// <Notification::GetAsString>
// Return a string representation of OZW
//-----------------------------------------------------------------------------

string Notification::GetAsString() {
	string str;
	switch (m_type) {
		case Type_ValueAdded:
			str = "ValueAdded";
			break;
		case Type_ValueRemoved:
			str = "ValueRemoved";
			break;
		case Type_ValueChanged:
			str = "ValueChanged";
			break;
		case Type_ValueRefreshed:
			str = "ValueRefreshed";
			break;
		case Type_Group:
			str = "Group";
			break;
		case Type_NodeNew:
			str = "NodeNew";
			break;
		case Type_NodeAdded:
			str = "NodeAdded";
			break;
		case Type_NodeRemoved:
			str = "NodeRemoved";
			break;
		case Type_NodeProtocolInfo:
			str = "NodeProtocolInfo";
			break;
		case Type_NodeNaming:
			str = "NodeNaming";
			break;
		case Type_NodeEvent:
			str = "NodeEvent";
			break;
		case Type_PollingDisabled:
			str = "PollingDisabled";
			break;
		case Type_PollingEnabled:
			str = "PollingEnabled";
			break;
		case Type_SceneEvent:
			str = "SceneEvent";
			break;
		case Type_CreateButton:
			str = "CreateButton";
			break;
		case Type_DeleteButton:
			str = "DeleteButton";
			break;
		case Type_ButtonOn:
			str = "ButtonOn";
			break;
		case Type_ButtonOff:
			str = "ButtonOff";
			break;
		case Type_DriverReady:
			str = "DriverReady";
			break;
		case Type_DriverFailed:
			str = "DriverFailed";
			break;
		case Type_DriverReset:
			str = "DriverReset";
			break;
		case Type_EssentialNodeQueriesComplete:
			str = "EssentialNodeQueriesComplete";
			break;
		case Type_NodeQueriesComplete:
			str = "NodeQueriesComplete";
			break;
		case Type_AwakeNodesQueried:
			str = "AwakeNodesQueried";
			break;
		case Type_AllNodesQueriedSomeDead:
			str = "AllNodesQueriedSomeDead";
			break;
		case Type_AllNodesQueried:
			str = "AllNodesQueried";
			break;
		case Type_Notification:
			switch (m_byte) {
				case Code_MsgComplete:
					str = "Notification - MsgComplete";
					break;
				case Code_Timeout:
					str = "Notification - TimeOut";
					break;
				case Code_NoOperation:
					str = "Notification - NoOperation";
					break;
				case Code_Awake:
					str = "Notification - Node Awake";
					break;
				case Code_Sleep:
					str = "Notification - Node Asleep";
					break;
				case Code_Dead:
					str = "Notification - Node Dead";
					break;
				case Code_Alive:
					str = "Notification - Node Alive";
					break;
			}
			break;
		case Type_DriverRemoved:
			str = "DriverRemoved";
			break;
		case Type_ControllerCommand:
			switch (m_event) {
				case Driver::ControllerState_Normal:
					str = "ControlerCommand - Normal";
					break;
				case Driver::ControllerState_Starting:
					str = "ControllerComand - Starting";
					break;
				case Driver::ControllerState_Cancel:
					str = "ControllerCommand - Canceled";
					break;
				case Driver::ControllerState_Error:
					switch (m_byte) {
						case Driver::ControllerError_None:
							str = "ControllerCommand - Error - None";
							break;
						case Driver::ControllerError_ButtonNotFound:
							str = "ControllerCommand - Error - ButtonNotFound";
							break;
						case Driver::ControllerError_NodeNotFound:
							str = "ControllerCommand - Error - NodeNotFound";
							break;
						case Driver::ControllerError_NotBridge:
							str = "ControllerCommand - Error - NotBridge";
							break;
						case Driver::ControllerError_NotSUC:
							str = "ControllerCommand - Error - NotSUC";
							break;
						case Driver::ControllerError_NotSecondary:
							str = "ControllerCommand - Error - NotSecondary";
							break;
						case Driver::ControllerError_NotPrimary:
							str = "ControllerCommand - Error - NotPrimary";
							break;
						case Driver::ControllerError_IsPrimary:
							str = "ControllerCommand - Error - IsPrimary";
							break;
						case Driver::ControllerError_NotFound:
							str = "ControllerCommand - Error - NotFound";
							break;
						case Driver::ControllerError_Busy:
							str = "ControllerCommand - Error - Busy";
							break;
						case Driver::ControllerError_Failed:
							str = "ControllerCommand - Error - Failed";
							break;
						case Driver::ControllerError_Disabled:
							str = "ControllerCommand - Error - Disabled";
							break;
						case Driver::ControllerError_Overflow:
							str = "ControllerCommand - Error - OverFlow";
							break;
					}
					break;
				case Driver::ControllerState_Waiting:
					str = "ControllerCommand - Waiting";
					break;
				case Driver::ControllerState_Sleeping:
					str = "ControllerCommand - Sleeping";
					break;
				case Driver::ControllerState_InProgress:
					str = "ControllerCommand - InProgress";
					break;
				case Driver::ControllerState_Completed:
					str = "ControllerCommand - Completed";
					break;
				case Driver::ControllerState_Failed:
					str = "ControllerCommand - Failed";
					break;
				case Driver::ControllerState_NodeOK:
					str = "ControllerCommand - NodeOK";
					break;
				case Driver::ControllerState_NodeFailed:
					str = "ControllerCommand - NodeFailed";
					break;
			}
			break;
	}
	return str;

}



