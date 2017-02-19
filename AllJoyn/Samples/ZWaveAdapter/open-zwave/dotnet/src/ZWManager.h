//-----------------------------------------------------------------------------
//
//      ZWManager.h
//
//      Cli/C++ wrapper for the C++ OpenZWave Manager class
//
//      Copyright (c) 2010 Amer Harb <harb_amer@hotmail.com>
//
//      SOFTWARE NOTICE AND LICENSE
//
//      This file is part of OpenZWave.
//
//      OpenZWave is free software: you can redistribute it and/or modify
//      it under the terms of the GNU Lesser General Public License as published
//      by the Free Software Foundation, either version 3 of the License,
//      or (at your option) any later version.
//
//      OpenZWave is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU Lesser General Public License for more details.
//
//      You should have received a copy of the GNU Lesser General Public License
//      along with OpenZWave.  If not, see <http://www.gnu.org/licenses/>.
//
//-----------------------------------------------------------------------------

#pragma once

#include <Windows.h>
#include <stdio.h>
#include <msclr/auto_gcroot.h>
#include <msclr/lock.h>

#include "ZWValueID.h"
#include "ZWNotification.h"

#include "Manager.h"
#include "ValueID.h"
#include "Notification.h"
#include "Driver.h"
#include "Log.h"

using namespace System;
using namespace System::Threading;
using namespace System::Collections::Generic;
using namespace Runtime::InteropServices;
using namespace OpenZWave;


namespace OpenZWaveDotNet
{
	// Delegate for handling notification callbacks
	public delegate void ManagedNotificationsHandler(ZWNotification^ notification);

	[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
	private delegate void OnNotificationFromUnmanagedDelegate(Notification* _notification, void* _context);

	// Logging levels
	public enum class ZWLogLevel
	{
		None		= LogLevel_None,
		Always		= LogLevel_Always,
		Fatal		= LogLevel_Fatal,
		Error		= LogLevel_Error,
		Warning		= LogLevel_Warning,
		Alert		= LogLevel_Alert,
		Info		= LogLevel_Info,
		Detail		= LogLevel_Detail,
		Debug		= LogLevel_Debug,
		StreamDetail= LogLevel_StreamDetail,
		Internal	= LogLevel_Internal
	};

	// Delegate for handling controller command callbacks
	public enum class ZWControllerState
	{
		Normal		= Driver::ControllerState_Normal,								/**< No command in progress. */
		Starting	= Driver::ControllerState_Starting,								/**< The command is starting. */
		Cancel		= Driver::ControllerState_Cancel,								/**< The command was cancelled. */
		Error		= Driver::ControllerState_Error,								/**< Command invocation had error(s) and was aborted */
		Waiting		= Driver::ControllerState_Waiting,								/**< Controller is waiting for a user action. */
		Sleeping	= Driver::ControllerState_Sleeping,								/**< Controller command is on a sleep queue wait for device. */
		InProgress	= Driver::ControllerState_InProgress,							/**< The controller is communicating with the other device to carry out the command. */
		Completed	= Driver::ControllerState_Completed,							/**< The command has completed successfully. */
		Failed		= Driver::ControllerState_Failed,								/**< The command has failed. */
		NodeOK		= Driver::ControllerState_NodeOK,								/**< Used with the HasNodeFailed, RemoveFailedNode and ReplaceFailedNode commands to indicate that the controller thinks the node is OK. */
		NodeFailed	= Driver::ControllerState_NodeFailed							/**< Used only with HasNodeFailed to indicate that the controller thinks the node has failed. */
	};

	// Controller interface types
	public enum class ZWControllerInterface
	{
		Unknown		= Driver::ControllerInterface_Unknown,
		Serial		= Driver::ControllerInterface_Serial,
		Hid			= Driver::ControllerInterface_Hid
	};

	public enum class ZWControllerCommand
	{
		None						= Driver::ControllerCommand_None,						/**< No command. */
		AddDevice					= Driver::ControllerCommand_AddDevice,					/**< Add a new device (but not a controller) to the Z-Wave network. */
		CreateNewPrimary			= Driver::ControllerCommand_CreateNewPrimary,			/**< Add a new controller to the Z-Wave network.  The new controller will be the primary, and the current primary will become a secondary controller. */
		ReceiveConfiguration		= Driver::ControllerCommand_ReceiveConfiguration,		/**< Receive Z-Wave network configuration information from another controller. */
		RemoveDevice				= Driver::ControllerCommand_RemoveDevice,				/**< Remove a new device (but not a controller) from the Z-Wave network. */
		RemoveFailedNode			= Driver::ControllerCommand_RemoveFailedNode,			/**< Move a node to the controller's failed nodes list. This command will only work if the node cannot respond. */
		HasNodeFailed				= Driver::ControllerCommand_HasNodeFailed,				/**< Check whether a node is in the controller's failed nodes list. */
		ReplaceFailedNode			= Driver::ControllerCommand_ReplaceFailedNode,			/**< Replace a non-responding device with another. */
		TransferPrimaryRole			= Driver::ControllerCommand_TransferPrimaryRole,		/**< Make a different controller the primary. */
		RequestNetworkUpdate		= Driver::ControllerCommand_RequestNetworkUpdate,		/**< Request network information from the SUC/SIS. */
		RequestNodeNeighborUpdate	= Driver::ControllerCommand_RequestNodeNeighborUpdate,	/**< Get a node to rebuild its neighbour list.  This method also does ControllerCommand_RequestNodeNeighbors */
		AssignReturnRoute			= Driver::ControllerCommand_AssignReturnRoute,			/**< Assign a network return route to a device. */
		DeleteAllReturnRoutes		= Driver::ControllerCommand_DeleteAllReturnRoutes,		/**< Delete all network return routes from a device. */
		SendNodeInformation			= Driver::ControllerCommand_SendNodeInformation,		/**< Send a node information frame */
		ReplicationSend				= Driver::ControllerCommand_ReplicationSend,			/**< Send information from primary to secondary */
		CreateButton				= Driver::ControllerCommand_CreateButton,				/**< Create an id that tracks handheld button presses */
		DeleteButton				= Driver::ControllerCommand_DeleteButton				/**< Delete id that tracks handheld button presses */
	};

	public delegate void ManagedControllerStateChangedHandler( ZWControllerState _state);

	[UnmanagedFunctionPointer(CallingConvention::Cdecl)]
	private delegate void OnControllerStateChangedFromUnmanagedDelegate(Driver::ControllerState _state, void* _context);

	public ref class ZWManager
	{
	//-----------------------------------------------------------------------------
	// Events
	//-----------------------------------------------------------------------------
	private:
		ManagedNotificationsHandler^ m_notificationEvent;
		event ManagedNotificationsHandler^ ZWOnNotification
		{
			void add( ManagedNotificationsHandler ^ d )
			{ 
				m_notificationEvent += d;
			} 
			
			void remove(ManagedNotificationsHandler ^ d)
			{ 
				m_notificationEvent -= d;
			} 
			
			void raise(ZWNotification^ notification)
			{ 
				ManagedNotificationsHandler^ tmp = m_notificationEvent; 
				if (tmp)
				{ 
					tmp->Invoke( notification );
				} 
			} 
		}

	public:
		property ManagedNotificationsHandler^ OnNotification
		{
			ManagedNotificationsHandler^ get()
			{
				return m_notificationEvent;
			}
			void set( ManagedNotificationsHandler^ value )
			{
				m_notificationEvent = value;
			}
		}

	private:
		ManagedControllerStateChangedHandler^ m_controllerStateChangedEvent;
		event ManagedControllerStateChangedHandler^ ZWOnControllerStateChanged
		{
			void add( ManagedControllerStateChangedHandler ^ d )
			{ 
				m_controllerStateChangedEvent += d;
			} 
			
			void remove(ManagedControllerStateChangedHandler ^ d)
			{ 
				m_controllerStateChangedEvent -= d;
			} 
			
			void raise(ZWControllerState state)
			{ 
				ManagedControllerStateChangedHandler^ tmp = m_controllerStateChangedEvent; 
				if (tmp)
				{ 
					tmp->Invoke( state );
				} 
			} 
		} 

		ManagedControllerStateChangedHandler^ m_onControllerStateChanged;

	public:
		property ManagedControllerStateChangedHandler^ OnControllerStateChanged
		{
			ManagedControllerStateChangedHandler^ get()
			{
				return m_controllerStateChangedEvent;
			}
			void set( ManagedControllerStateChangedHandler^ value )
			{
				m_controllerStateChangedEvent = value;
			}
		}

	//-----------------------------------------------------------------------------
	// Construction
	//-----------------------------------------------------------------------------
	/** \name Construction
	 *  For creating and destroying the Manager singleton.
	 */
	/*@{*/
	public:
   		/**
		 * \brief Creates the Manager singleton object.
		 *
		 * The Manager provides the public interface to OpenZWave, exposing all the functionality required to add Z-Wave support to an application.
		 * There can be only one Manager in an OpenZWave application.  Once the Manager has been created, call AddWatcher to install a notification
		 * callback handler, and then call the AddDriver method for each attached PC Z-Wave controller in turn.
		 * \param _configPath a string containing the path to the OpenZWave library config folder, which contains XML descriptions of Z-Wave manufacturers and products.
		 * \param _userPath a string containing the path to the application's user data folder where the OpenZWave should store the Z-Wave network configuration and state.
		 * \return a pointer to the newly created Manager object.
		 * \see Destroy, AddWatcher, AddDriver
		 */
		void Create();

		/**
		 * \brief Deletes the Manager and cleans up any associated objects.  
		 *
		 * \see Create, Get
		 */
		void Destroy(){ Manager::Get()->Destroy(); }

		/**
		 * \brief Get the Version Number of OZW as a string
		 * \return a String representing the version number as MAJOR.MINOR.REVISION
		 */
		String^ GetVersionAsString() { return gcnew String(Manager::Get()->getVersionAsString().c_str()); }

		/**
		 * \brief Get the Version Number as the Version Struct (Only Major/Minor returned)
		 * \return the version struct representing the version
		 */
		//ozwversion GetVersion();


		/**
		 * \brief Sets the library logging state.
		 * \param bState True to enable logging; false to disable logging.
		 * \see GetLoggingState
		 */
		void SetLoggingState(bool bState){ Log::SetLoggingState(bState); }

		/**
		 * \brief Gets the current library logging state.
		 * \return True if logging is enabled; false otherwise
		 * \see SetLoggingState
		 */
		bool GetLoggingState() { return Log::GetLoggingState(); }

		/**
		 * \brief Sets the current library log file name to a new name
		 */
		void SetLogFileName( String^ _filename ) { Log::SetLogFileName((const char*)(Marshal::StringToHGlobalAnsi(_filename)).ToPointer()); }

		/**
		 * \brief Sends current driver statistics to the log file
		 */
		void LogDriverStatistics(uint32 homeId ) { Manager::Get()->LogDriverStatistics(homeId); }
	/*@}*/					   

	//-----------------------------------------------------------------------------
	// Configuration
	//-----------------------------------------------------------------------------
	/** \name Configuration
	 *  For saving the Z-Wave network configuration so that the entire network does not need to be 
	 *  polled every time the application starts.
	 */
	/*@{*/
	public:
		/**
		 * \brief Saves the configuration of a PC Controller's Z-Wave network to the application's user data folder.
		 *
		 * This method does not normally need to be called, since OpenZWave will save the state automatically
		 * during the shutdown process.  It is provided here only as an aid to development.
		 * The configuration of each PC Controller's Z-Wave network is stored in a separate file.  The filename 
		 * consists of the 8 digit hexadecimal version of the controller's Home ID, prefixed with the string 'zwcfg_'.
		 * This convention allows OpenZWave to find the correct configuration file for a controller, even if it is
		 * attached to a different serial port.
		 * \param homeId The Home ID of the Z-Wave controller to save.
		 */
		void WriteConfig(uint32 homeId){ Manager::Get()->WriteConfig(homeId); }
	/*@}*/					   

	//-----------------------------------------------------------------------------
	//	Drivers
	//-----------------------------------------------------------------------------
	/** \name Drivers
	 *  Methods for adding and removing drivers and obtaining basic controller information.
	 */
	/*@{*/
	public:
		/**
		 * \brief Creates a new driver for a Z-Wave controller.
		 *
		 * This method creates a Driver object for handling communications with a single Z-Wave controller.  In the background, the  
		 * driver first tries to read configuration data saved during a previous run.  It then queries the controller directly for any
		 * missing information, and a refresh of the list of nodes that it controls.  Once this information
		 * has been received, a DriverReady notification callback is sent, containing the Home ID of the controller.  This Home ID is
		 * required by most of the OpenZWave Manager class methods.
		 * \param serialPortName The string used to open the serial port, for example "\\.\COM3".
		 8 \param interfaceType Specifies whether this is a serial or HID interface (default is serial).
		 * \return True if a new driver was created, false if a driver for the controller already exists.
		 * \see Create, Get, RemoveDriver
		 */
		bool AddDriver( String^ serialPortName ){ return Manager::Get()->AddDriver((const char*)(Marshal::StringToHGlobalAnsi(serialPortName)).ToPointer()); }
		bool AddDriver( String^ serialPortName, ZWControllerInterface interfaceType ){ return Manager::Get()->AddDriver((const char*)(Marshal::StringToHGlobalAnsi(serialPortName)).ToPointer(), (Driver::ControllerInterface) interfaceType); }

		/**
		 * \brief Removes the driver for a Z-Wave controller, and closes the serial port.
		 *
		 * Drivers do not need to be explicitly removed before calling Destroy - this is handled automatically.
		 * @paaram serialPortName The same string as was passed in the original call to AddDriver.
		 * \returns True if the driver was removed, false if it could not be found.
		 * \see Destroy, AddDriver
		 */
		bool RemoveDriver( String^ serialPortName ){ return Manager::Get()->RemoveDriver((const char*)(Marshal::StringToHGlobalAnsi(serialPortName)).ToPointer()); }

		/**
		 * \brief Get the node ID of the Z-Wave controller.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return the node ID of the Z-Wave controller.
		 */
		uint8 GetControllerNodeId( uint32 homeId ){ return Manager::Get()->GetControllerNodeId(homeId); }

		/**
		 * \brief Get the node ID of the Static Update Controller.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return the node ID of the Z-Wave controller.
		 */
		uint8 GetSUCNodeId( uint32 homeId ) { return Manager::Get()->GetSUCNodeId(homeId); }


		/**
		 * \brief Query if the controller is a primary controller.
		 *
		 * The primary controller is the main device used to configure and control a Z-Wave network.
		 * There can only be one primary controller - all other controllers are secondary controllers.
		 * <p> 
		 * The only difference between a primary and secondary controller is that the primary is the
		 * only one that can be used to add or remove other devices.  For this reason, it is usually
		 * better for the promary controller to be portable, since most devices must be added when
		 * installed in their final location.
		 * <p>
		 * Calls to BeginControllerCommand will fail if the controller is not the primary.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return true if it is a primary controller, false if not.
		 */
		bool IsPrimaryController( uint32 homeId ){ return Manager::Get()->IsPrimaryController(homeId); }

		/**
		 * \brief Query if the controller is a static update controller.
		 *
		 * A Static Update Controller (SUC) is a controller that must never be moved in normal operation
		 * and which can be used by other nodes to receive information about network changes.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return true if it is a static update controller, false if not.
		 */
		bool IsStaticUpdateController( uint32 homeId ){ return Manager::Get()->IsStaticUpdateController(homeId); }

		/**
		 * \brief Query if the controller is using the bridge controller library.
		 *
		 * A bridge controller is able to create virtual nodes that can be associated
		 * with other controllers to enable events to be passed on.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return true if it is a bridge controller, false if not.
		 */
		bool IsBridgeController( uint32 const homeId ){ return Manager::Get()->IsBridgeController(homeId); }

		/**
		 * \brief Get the version of the Z-Wave API library used by a controller.
		 *
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return a string containing the library version. For example, "Z-Wave 2.48".
		 */
		String^ GetLibraryVersion( uint32 const homeId ){ return gcnew String(Manager::Get()->GetLibraryVersion(homeId).c_str()); }

		/**
		 * \brief Get a string containing the Z-Wave API library type used by a controller.
		 *
		 * The possible library types are:
		 * - Static Controller
		 * - Controller
		 * - Enhanced Slave
		 * - Slave            
	     * - Installer
	     * - Routing Slave
	     * - Bridge Controller
		 * - Device Under Test
		 * The controller should never return a slave library type.
		 * For a more efficient test of whether a controller is a Bridge Controller, use
		 * the IsBridgeController method.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return a string containing the library type.
		 * \see GetLibraryVersion, IsBridgeController
		 */
		String^ GetLibraryTypeName( uint32 const homeId ){ return gcnew String(Manager::Get()->GetLibraryTypeName(homeId).c_str()); }

		/**
		 * \brief Get count of messages in the outgoing send queue.
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return a integer message count
		 */
		int32 GetSendQueueCount( uint32 const homeId ){ return Manager::Get()->GetSendQueueCount( homeId ); }

		/**
		 * \brief Obtain controller interface type
		 * \param homeId The Home ID of the Z-Wave controller.
		 */
		ZWControllerInterface GetControllerInterfaceType( uint32 homeId ) { return (ZWControllerInterface)Manager::Get()->GetControllerInterfaceType(homeId); }

		/**
		 * \brief Obtain controller interface path
		 * \param homeId The Home ID of the Z-Wave controller.
		 */
		String^ GetControllerPath( uint32 homeId ) { return gcnew String(Manager::Get()->GetControllerPath(homeId).c_str()); }

	/*@}*/					   

	//-----------------------------------------------------------------------------
	//	Polling Z-Wave devices
	//-----------------------------------------------------------------------------
	/** \name Polling Z-Wave devices
	 *  Methods for controlling the polling of Z-Wave devices.  Modern devices will not
	 *  require polling.  Some old devices need to be polled as the only way to detect
	 *  status changes.
	 */
	/*@{*/
	public:
		/**
		 * \brief Get the time period between polls of a node's state.
		 */
		int32 GetPollInterval() { return Manager::Get()->GetPollInterval(); }

		/**
		 * \brief Set the time period between polls of a node's state.
		 *
		 * Due to patent concerns, some devices do not report state changes automatically to the controller.
		 * These devices need to have their state polled at regular intervals.  The length of the interval
		 * is the same for all devices.  To even out the Z-Wave network traffic generated by polling, OpenZWave
		 * divides the polling interval by the number of devices that have polling enabled, and polls each
		 * in turn.  It is recommended that if possible, the interval should not be set shorter than the
		 * number of polled devices in seconds (so that the network does not have to cope with more than one
		 * poll per second).
		 *
		 * Note that the polling interval cannot be set on a per-node basis.  Every node that is polled is
		 * polled at the specified interval.
		 * \param milliseconds The length of the polling interval in milliseconds.
		 */
		void SetPollInterval( int32 milliseconds, bool bIntervalBetweenPolls ){ Manager::Get()->SetPollInterval(milliseconds, bIntervalBetweenPolls); }

		/**
		 * \brief Enable the polling of a device's state.
		 *
		 * \param valueId The ID of the value to start polling.
		 * \return True if polling was enabled.
		 */
		bool EnablePoll( ZWValueID^ valueId ){ return Manager::Get()->EnablePoll(valueId->CreateUnmanagedValueID()); }

		/**
		 * \brief Enable the polling of a device's state.
		 *
		 * \param valueId The ID of the value to start polling.
		 * \param intensity, number of polling for one polling interval.
		 * \return True if polling was enabled.
		 */
		bool EnablePoll( ZWValueID^ valueId, uint8 intensity ){ return Manager::Get()->EnablePoll(valueId->CreateUnmanagedValueID(), intensity); }

		/**
		 * \brief Disable the polling of a device's state.
		 *
		 * \param valueId The ID of the value to stop polling.
		 * \return True if polling was disabled.
		 */
		bool DisablePoll( ZWValueID^ valueId ){ return Manager::Get()->DisablePoll(valueId->CreateUnmanagedValueID()); }

		/**
		 * \brief Determine the polling of a device's state.
		 * \param valueId The ID of the value to check polling.
		 * \return True if polling is active.
		 */
		bool IsPolled( ZWValueID^ valueId ) { return Manager::Get()->isPolled(valueId->CreateUnmanagedValueID()); }

		/**
		 * \brief Set the frequency of polling (0=none, 1=every time through the list, 2-every other time, etc)
		 * \param valueId The ID of the value whose intensity should be set
		 * \param intensity The intensity to set
		 */
		void SetPollIntensity( ZWValueID^ valueId, uint8 intensity ) { Manager::Get()->SetPollIntensity(valueId->CreateUnmanagedValueID(), intensity); }

		/**
		 * \brief Get the polling intensity of a device's state.
		 * \param valueId The ID of the value to check polling.
		 * \return Intensity, number of polling for one polling interval.
		 * \throws OZWException with Type OZWException::OZWEXCEPTION_INVALID_VALUEID if the ValueID is invalid
		 * \throws OZWException with Type OZWException::OZWEXCEPTION_INVALID_HOMEID if the Driver cannot be found
		 */
		uint8 GetPollIntensity( ZWValueID^ valueId ) { return Manager::Get()->GetPollIntensity( valueId->CreateUnmanagedValueID()); }

	/*@}*/

	//-----------------------------------------------------------------------------
	//	Node information
	//-----------------------------------------------------------------------------
	/** \name Node information
	 *  Methods for accessing information on indivdual nodes.
	 */
	/*@{*/
	public:
		/**
		 * \brief Trigger the fetching of fixed data about a node.
		 *
		 * Causes the node's data to be obtained from the Z-Wave network in the same way as if it had just been added.
		 * This method would normally be called automatically by OpenZWave, but if you know that a node has been
		 * changed, calling this method will force a refresh of the data held by the library.  This can be especially 
		 * useful for devices that were asleep when the application was first run.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the request was sent successfully.
		 */
		bool RefreshNodeInfo( uint32 homeId, uint8 nodeId ){ return Manager::Get()->RefreshNodeInfo(homeId,nodeId); }
 		
		/**
		 * \brief Trigger the fetching of session and dynamic value data for a node.
		 *
		 * Causes the node's values to be requested from the Z-Wave network.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the request was sent successfully.
		 */
		void RequestNodeState( uint32 homeId, uint8 nodeId ){ Manager::Get()->RequestNodeState(homeId,nodeId); }

		/**
		 * \brief Trigger the fetching of just the dynamic value data for a node.
		 * Causes the node's values to be requested from the Z-Wave network. This is the
		 * same as the query state starting from the dynamic state.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the request was sent successfully.
		 */
		bool RequestNodeDynamic( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->RequestNodeDynamic(homeId, nodeId); }

		/**
		 * \brief Get whether the node is a listening device that does not go to sleep.
		 *
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if it is a listening node.
		 */
		bool IsNodeListeningDevice( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->IsNodeListeningDevice(homeId,nodeId); }

		/**
		 * \brief Get whether the node is a frequent listening device that goes to sleep but
		 * can be woken up by a beam. Useful to determine node and controller consistency.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if it is a frequent listening node.
		 */
		bool IsNodeFrequentListeningDevice( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->IsNodeFrequentListeningDevice(homeId, nodeId); }

		/**
		 * \brief Get whether the node is a beam capable device.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if it is a frequent listening node.
		 */
		bool IsNodeBeamingDevice( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->IsNodeBeamingDevice(homeId, nodeId); }

		/**
		 * \brief Get whether the node is a routing device that passes messages to other nodes.
		 *
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the node is a routing device
		 */
		bool IsNodeRoutingDevice( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->IsNodeRoutingDevice(homeId,nodeId); }

		/**
		 * \brief Get the security attribute for a node. True if node supports security features.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return true if security features implemented.
		 */
		bool IsNodeSecurityDevice( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->IsNodeSecurityDevice(homeId, nodeId); }
		
		/**
		 * \brief Get the maximum baud rate of a node's communications
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return the baud rate in bits per second.
		 */
		uint32 GetNodeMaxBaudRate( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->GetNodeMaxBaudRate(homeId, nodeId); }

		/**
		 * \brief Get the version number of a node
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return the node's version number
		 */
		uint8 GetNodeVersion( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->GetNodeVersion(homeId, nodeId); }

		/**
		 * \brief Get the security byte for a node.  Bit meanings are still to be determined.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return the node's security byte
		 */
		uint8 GetNodeSecurity( uint32 const homeId, uint8 const nodeId ){ return Manager::Get()->GetNodeSecurity(homeId, nodeId); }
		
		/**
		 * \brief Get a node's "basic" type.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return The basic type.
		 */
		uint8 GetNodeBasic( uint32 homeId, uint8 nodeId ){ return Manager::Get()->GetNodeBasic(homeId, nodeId); }

		/**
		 * \brief Get a node's "generic" type.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return The generic type.
		 */
		uint8 GetNodeGeneric( uint32 homeId, uint8 nodeId ){ return Manager::Get()->GetNodeGeneric(homeId,nodeId); }

		/**
		 * \brief Get a node's "specific" type.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return The specific type.
		 */
		uint8 GetNodeSpecific( uint32 homeId, uint8 nodeId ){ return Manager::Get()->GetNodeSpecific(homeId,nodeId); }

		/**
		 * \brief Get a human-readable label describing the node.
		 *
		 * The label is taken from the Z-Wave specific, generic or basic type, depending on which of those values are specified by the node.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the label text.
		 */
		String^ GetNodeType( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeType(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the bitmap of this node's neighbors
		 *
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \param o_associations An array of 29 uint8s to hold the neighbor bitmap
		 */
		uint32 GetNodeNeighbors( uint32 const homeId, uint8 const nodeId, [Out] array<Byte>^ %o_associations );

		/**
		 * \brief Get the manufacturer name of a device.
		 *
		 * The manufacturer name would normally be handled by the Manufacturer Specific commmand class,
		 * taking the manufacturer ID reported by the device and using it to look up the name from the
		 * manufacturer_specific.xml file in the OpenZWave config folder.
		 * However, there are some devices that do not support the command class, so to enable the user
		 * to manually set the name, it is stored with the node data and accessed via this method rather
		 * than being reported via a command class Value object.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's manufacturer name.
		 * \see SetNodeManufacturerName, GetNodeProductName, SetNodeProductName
		 */
		String^ GetNodeManufacturerName( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeManufacturerName(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the product name of a device.
		 *
		 * The product name would normally be handled by the Manufacturer Specific commmand class,
		 * taking the product Type and ID reported by the device and using it to look up the name from the
		 * manufacturer_specific.xml file in the OpenZWave config folder.
		 * However, there are some devices that do not support the command class, so to enable the user
		 * to manually set the name, it is stored with the node data and accessed via this method rather
		 * than being reported via a command class Value object.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's product name.
		 * \see SetNodeProductName, GetNodeManufacturerName, SetNodeManufacturerName
		 */
		String^ GetNodeProductName( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeProductName(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the name of a node.
		 *
		 * The node name is a user-editable label for the node that would normally be handled by the
		 * Node Naming commmand class, but many devices do not support it.  So that a node can always
		 * be named, OpenZWave stores it with the node data, and provides access through this method
		 * and SetNodeName, rather than reporting it via a command class Value object.
		 * The maximum length of a node name is 16 characters.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's name.
		 * \see SetNodeName, GetNodeLocation, SetNodeLocation
		 */
		String^ GetNodeName( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeName(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the location of a node.
		 *
		 * The node location is a user-editable string that would normally be handled by the Node Naming
		 * commmand class, but many devices do not support it.  So that a node can always report its
		 * location, OpenZWave stores it with the node data, and provides access through this method
		 * and SetNodeLocation, rather than reporting it via a command class Value object.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's location.
		 * \see SetNodeLocation, GetNodeName, SetNodeName
		 */
		String^ GetNodeLocation( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeLocation(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the manufacturer ID of a device.
		 *
		 * The manufacturer ID is a four digit hex code and would normally be handled by the Manufacturer
		 * Specific commmand class, but not all devices support it.  Although the value reported by this
		 * method will be an empty string if the command class is not supported and cannot be set by the 
		 * user, the manufacturer ID is still stored with the node data (rather than being reported via a
		 * command class Value object) to retain a consistent approach with the other manufacturer specific data.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's manufacturer ID, or an empty string if the manufactuer
		 * specific command class is not supported by the device.
		 * \see GetNodeProductType, GetNodeProductId, GetNodeManufacturerName, GetNodeProductName
		 */
		String^ GetNodeManufacturerId( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeManufacturerId(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the product type of a device.
		 *
		 * The product type is a four digit hex code and would normally be handled by the Manufacturer Specific
		 * commmand class, but not all devices support it.  Although the value reported by this method will
		 * be an empty string if the command class is not supported and cannot be set by the user, the product
		 * type is still stored with the node data (rather than being reported via a command class Value object)
		 * to retain a consistent approach with the other manufacturer specific data.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's product type, or an empty string if the manufactuer
		 * specific command class is not supported by the device.
		 * \see GetNodeManufacturerId, GetNodeProductId, GetNodeManufacturerName, GetNodeProductName
		 */
		String^ GetNodeProductType( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeProductType(homeId,nodeId).c_str()); }

		/**
		 * \brief Get the product ID of a device.
		 *
		 * The product ID is a four digit hex code and would normally be handled by the Manufacturer Specific
		 * commmand class, but not all devices support it.  Although the value reported by this method will
		 * be an empty string if the command class is not supported and cannot be set by the user, the product
		 * ID is still stored with the node data (rather than being reported via a command class Value object)
		 * to retain a consistent approach with the other manufacturer specific data.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return A string containing the node's product ID, or an empty string if the manufactuer
		 * specific command class is not supported by the device.
		 * \see GetNodeManufacturerId, GetNodeProductType, GetNodeManufacturerName, GetNodeProductName
		 */
		String^ GetNodeProductId( uint32 homeId, uint8 nodeId ){ return gcnew String(Manager::Get()->GetNodeProductId( homeId, nodeId ).c_str()); }

		/**
		 * \brief Set the manufacturer name of a device.
		 *
		 * The manufacturer name would normally be handled by the Manufacturer Specific commmand class,
		 * taking the manufacturer ID reported by the device and using it to look up the name from the
		 * manufacturer_specific.xml file in the OpenZWave config folder.
		 * However, there are some devices that do not support the command class, so to enable the user
		 * to manually set the name, it is stored with the node data and accessed via this method rather
		 * than being reported via a command class Value object.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \param manufacturerName	A string containing the node's manufacturer name.
		 * \see GetNodeManufacturerName, GetNodeProductName, SetNodeProductName
		 */
		void SetNodeManufacturerName( uint32 homeId, uint8 nodeId, String^ manufacturerName ){ Manager::Get()->SetNodeManufacturerName( homeId, nodeId, (const char*)(Marshal::StringToHGlobalAnsi(manufacturerName)).ToPointer()); }
		
		/**
		 * \brief Set the product name of a device.
		 *
		 * The product name would normally be handled by the Manufacturer Specific commmand class,
		 * taking the product Type and ID reported by the device and using it to look up the name from the
		 * manufacturer_specific.xml file in the OpenZWave config folder.
		 * However, there are some devices that do not support the command class, so to enable the user
		 * to manually set the name, it is stored with the node data and accessed via this method rather
		 * than being reported via a command class Value object.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \param productName A string containing the node's product name.
		 * \see GetNodeProductName, GetNodeManufacturerName, SetNodeManufacturerName
		 */
		void SetNodeProductName( uint32 homeId, uint8 nodeId, String^ productName ){ Manager::Get()->SetNodeProductName( homeId, nodeId, (const char*)(Marshal::StringToHGlobalAnsi(productName)).ToPointer()); }

		/**
		 * \brief Set the name of a node.
		 *
		 * The node name is a user-editable label for the node that would normally be handled by the
		 * Node Naming commmand class, but many devices do not support it.  So that a node can always
		 * be named, OpenZWave stores it with the node data, and provides access through this method
		 * and GetNodeName, rather than reporting it via a command class Value object.
		 * If the device does support the Node Naming command class, the new name will be sent to the node.
		 * The maximum length of a node name is 16 characters.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \param nodeName A string containing the node's name.
		 * \see GetNodeName, GetNodeLocation, SetNodeLocation
		 */
		void SetNodeName( uint32 homeId, uint8 nodeId, String^ nodeName ){ Manager::Get()->SetNodeName( homeId, nodeId, (const char*)(Marshal::StringToHGlobalAnsi(nodeName)).ToPointer()); }

		/**
		 * \brief Set the location of a node.
		 *
		 * The node location is a user-editable string that would normally be handled by the Node Naming
		 * commmand class, but many devices do not support it.  So that a node can always report its
		 * location, OpenZWave stores it with the node data, and provides access through this method
		 * and GetNodeLocation, rather than reporting it via a command class Value object.
		 * If the device does support the Node Naming command class, the new location will be sent to the node.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \param location A string containing the node's location.
		 * \see GetNodeLocation, GetNodeName, SetNodeName
		 */
		void SetNodeLocation( uint32 homeId, uint8 nodeId, String^ location ){ Manager::Get()->SetNodeLocation( homeId, nodeId, (const char*)(Marshal::StringToHGlobalAnsi(location)).ToPointer()); }
	
		/**
		 * \brief Turns a node on.
		 *
		 * This is a helper method to simplify basic control of a node.  It is the equivalent of
		 * changing the level reported by the node's Basic command class to 255, and will generate a 
		 * ValueChanged notification from that class.  This command will turn on the device at its
		 * last known level, if supported by the device, otherwise it will turn	it on at 100%.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to be changed.
		 * \see SetNodeOff, SetNodeLevel
		 */
		void SetNodeOn( uint32 homeId, uint8 nodeId ){ Manager::Get()->SetNodeOn( homeId, nodeId ); }

		/**
		 * \brief Turns a node off.
		 *
		 * This is a helper method to simplify basic control of a node.  It is the equivalent of
		 * changing the level reported by the node's Basic command class to zero, and will generate
		 * a ValueChanged notification from that class.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to be changed.
		 * \see SetNodeOn, SetNodeLevel
		 */
		void SetNodeOff( uint32 homeId, uint8 nodeId ){ Manager::Get()->SetNodeOff( homeId, nodeId ); }

		/**
		 * \brief Sets the basic level of a node.
		 *
		 * This is a helper method to simplify basic control of a node.  It is the equivalent of
		 * changing the value reported by the node's Basic command class and will generate a 
		 * ValueChanged notification from that class.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to be changed.
		 * \param level The level to set the node.  Valid values are 0-99 and 255.  Zero is off and
		 * 99 is fully on.  255 will turn on the device at its last known level (if supported).
		 * \see SetNodeOn, SetNodeOff
		 */
		void SetNodeLevel( uint32 homeId, uint8 nodeId, uint8 level ){ Manager::Get()->SetNodeLevel( homeId, nodeId, level ); }
		
		/**
		 * \brief Get whether the node information has been received
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the node information has been received yet
		 */
		bool IsNodeInfoReceived( uint32 homeId, uint8 nodeId ) { return Manager::Get()->IsNodeInfoReceived( homeId, nodeId ); }

		/**
		 * \brief Get whether the node has the defined class available or not
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \param commandClassId Id of the class to test for
		 * \return True if the node does have the class instantiated, will return name & version
		 */
		bool GetNodeClassInformation(uint32 homeId, uint8 nodeId, uint8 commandClassId, [Out] String^ %className, [Out] System::Byte %classVersion);

		/**
		 * \brief Get whether the node is awake or asleep
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the node is awake
		 */
		bool IsNodeAwake( uint32 homeId, uint8 nodeId ) { return Manager::Get()->IsNodeAwake( homeId, nodeId); }

		/**
		 * \brief Get whether the node is working or has failed
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return True if the node has failed and is no longer part of the network
		 */
		bool IsNodeFailed( uint32 homeId, uint8 nodeId ) { return Manager::Get()->IsNodeFailed( homeId, nodeId); }

		/**
		 * \brief Get whether the node's query stage as a string
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to query.
		 * \return name of current query stage as a string.
		 */
		String^ GetNodeQueryStage( uint32 homeId, uint8 nodeId ) { return gcnew String(Manager::Get()->GetNodeQueryStage( homeId, nodeId).c_str()); }

	/*@}*/

	//-----------------------------------------------------------------------------
	// Values
	//-----------------------------------------------------------------------------
	/** \name Values
	 *  Methods for accessing device values.  All the methods require a ValueID, which will have been provided
	 *  in the ValueAdded Notification callback when the the value was first discovered by OpenZWave.
	 */
	/*@{*/
	public:

		/**
		 * \brief Gets the user-friendly label for the value.
		 *
		 * \param id The unique identifier of the value.
		 * \return The value label.
		 * \see ValueID
		 */
		String^ GetValueLabel( ZWValueID^ id ){ return gcnew String(Manager::Get()->GetValueLabel(id->CreateUnmanagedValueID()).c_str()); }

		/**
		 * \brief Sets the user-friendly label for the value.
		 * \param id The unique identifier of the value.
		 * \param value The new value of the label.
		 * \throws OZWException with Type OZWException::OZWEXCEPTION_INVALID_VALUEID if the ValueID is invalid
		 * \throws OZWException with Type OZWException::OZWEXCEPTION_INVALID_HOMEID if the Driver cannot be found
		 * \see ValueID
		 */
		void SetValueLabel( ZWValueID^ id, String^ value ) { Manager::Get()->SetValueLabel( id->CreateUnmanagedValueID(), (const char*)(Marshal::StringToHGlobalAnsi(value)).ToPointer()); }

		/**
		 * \brief Gets the units that the value is measured in.
		 *
		 * \param id The unique identifier of the value.
		 * \return The value units.
		 * \see ValueID
		 */
		String^ GetValueUnits( ZWValueID^ id ){ return gcnew String(Manager::Get()->GetValueUnits(id->CreateUnmanagedValueID()).c_str()); }
		
		/**
		 * \brief Gets a help string describing the value's purpose and usage.
		 *
		 * \param id The unique identifier of the value.
		 * \return The value help text.
		 * \see ValueID
		 */
		String^ GetValueHelp( ZWValueID^ id ){ return gcnew String(Manager::Get()->GetValueHelp(id->CreateUnmanagedValueID()).c_str()); }
		
		/**
		 * \brief Test whether the value is read-only.
		 *
		 * \param id The unique identifier of the value.
		 * \return true if the value cannot be changed by the user.	
		 * \see ValueID
		 */
		bool IsValueReadOnly( ZWValueID^ id ){ return Manager::Get()->IsValueReadOnly(id->CreateUnmanagedValueID()); }

		/**
		 * \brief Test whether the value has been set.
		 *
		 * \param id The unique identifier of the value.
		 * \return true if the value has actually been set by a status message from the device, rather than simply being the default.	
		 * \see ValueID
		 */
		bool IsValueSet( ZWValueID^ id ){ return Manager::Get()->IsValueSet(id->CreateUnmanagedValueID()); }

		/**
		 * \brief Test whether the value is currently being polled.
		 *
		 * \param id The unique identifier of the value.
		 * \return true if the value is being polled, false otherwise.	
		 * \see ValueID
		 */
		bool IsValuePolled( ZWValueID^ id ){ return Manager::Get()->IsValuePolled(id->CreateUnmanagedValueID()); }

		/**
		 * \brief Gets a value as a bool.
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value a Boolean that will be filled with the value.
		 * \return true if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_Bool. The type can be tested with a call to ZWValueID::GetType.
		 * \see ValueID::GetType, GetValueAsByte, GetValueAsDecimal, GetValueAsInt, GetValueAsShort, GetValueAsString, GetValueListSelection, GetValueListItems 
		 */
		bool GetValueAsBool( ZWValueID^ id, [Out] System::Boolean %o_value );

		/**
		 * \brief Gets a value as an 8-bit unsigned integer.
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value a Byte that will be filled with the value.
		 * \return true if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_Byte. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsDecimal, GetValueAsInt, GetValueAsShort, GetValueAsString, GetValueListSelection, GetValueListItems 
		 */
		bool GetValueAsByte( ZWValueID^ id, [Out] System::Byte %o_value );

		/**
		 * \brief Gets a value as a decimal.
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value a Decimal that will be filled with the value.
		 * \return true if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_Decimal. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsInt, GetValueAsShort, GetValueAsString, GetValueListSelection, GetValueListItems 
		 */
		bool GetValueAsDecimal( ZWValueID^ id, [Out] System::Decimal %o_value );

		/**
		 * \brief Gets a value as a 32-bit signed integer.
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value an Int32 that will be filled with the value.
		 * \return true if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_Int. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsDecimal, GetValueAsShort, GetValueAsString, GetValueListSelection, GetValueListItems 
		 */
		bool GetValueAsInt( ZWValueID^ id, [Out] System::Int32 %o_value );

		/**
		 * \brief Gets a value as a 16-bit signed integer.
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value an Int16 that will be filled with the value.
		 * \return true if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_Short. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsDecimal, GetValueAsInt, GetValueAsString, GetValueListSelection, GetValueListItems 
		 */
		bool GetValueAsShort( ZWValueID^ id, [Out] System::Int16 %o_value );
		
		/**
		 * \brief Gets a value as a string.
		 *
		 * Creates a string representation of a value, regardless of type.
		 * \param _id The unique identifier of the value.
		 * \param o_value a String that will be filled with the value.
		 * \return true if the value was obtained.</returns>
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsDecimal, GetValueAsInt, GetValueAsShort, GetValueListSelection, GetValueListItems 
		 */
		bool GetValueAsString( ZWValueID^ id, [Out] String^ %o_value );
		
		/**
		 * \brief Gets the selected item from a list value (as a string).
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value A String that will be filled with the selected item.
		 * \return True if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_List. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsDecimal, GetValueAsInt, GetValueAsShort, GetValueAsString, GetValueListItems 
		 */
		bool GetValueListSelection( ZWValueID^ id, [Out] String^ %o_value );

		/**
		 * \brief Gets the selected item from a list value (as an integer).
		 *
		 * \param _id The unique identifier of the value.
		 * \param o_value An integer that will be filled with the selected item.
		 * \return True if the value was obtained.  Returns false if the value is not a ZWValueID::ValueType_List. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsDecimal, GetValueAsInt, GetValueAsShort, GetValueAsString, GetValueListItems 
		 */
		bool GetValueListSelection( ZWValueID^ id, [Out] System::Int32 %o_value );

		/**
		 * \brief Gets the list of items from a list value.
		 *
		 * \param id The unique identifier of the value.
		 * \param o_value List that will be filled with list items.
		 * \return true if the list items were obtained.  Returns false if the value is not a ZWValueID::ValueType_List. The type can be tested with a call to ZWValueID::GetType
		 * \see ValueID::GetType, GetValueAsBool, GetValueAsByte, GetValueAsDecimal, GetValueAsInt, GetValueAsShort, GetValueAsString, GetValueListSelection 
		 */
		bool GetValueListItems( ZWValueID^ id, [Out] array<String^>^ %o_value );

		/**
		 * \brief Sets the state of a bool.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the bool value.
		 * \param value The new value of the bool.
		 * \return true if the value was set.  Returns false if the value is not a ZWValueID::ValueType_Bool. The type can be tested with a call to ZWValueID::GetType
		 */
		bool SetValue( ZWValueID^ id, bool value ){ return Manager::Get()->SetValue(id->CreateUnmanagedValueID(), value); }

		/**
		 * \brief Sets the value of a byte.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the byte value.
		 * \param value The new value of the byte.
		 * \return true if the value was set.  Returns false if the value is not a ZWValueID::ValueType_Byte. The type can be tested with a call to ZWValueID::GetType
		 */
		bool SetValue( ZWValueID^ id, uint8 value ){ return Manager::Get()->SetValue(id->CreateUnmanagedValueID(), value); }

		/**
		 * \brief Sets the value of a decimal.
		 *
		 * It is usually better to handle decimal values using strings rather than floats, to avoid floating point accuracy issues.
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the decimal value.
		 * \param value The new value of the decimal.
		 * \return true if the value was set.  Returns false if the value is not a ZWValueID::ValueType_Decimal. The type can be tested with a call to ZWValueID::GetType
		 */
		bool SetValue( ZWValueID^ id, float value ){ return Manager::Get()->SetValue(id->CreateUnmanagedValueID(), value); }
		
		/**
		 * \brief Sets the value of a 32-bit signed integer.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the integer value.
		 * \param value The new value of the integer.
		 * \return true if the value was set.  Returns false if the value is not a ZWValueID::ValueType_Int. The type can be tested with a call to ZWValueID::GetType
		 */
		bool SetValue( ZWValueID^ id, int32 value ){ return Manager::Get()->SetValue(id->CreateUnmanagedValueID(), value); }

		/**
		 * \brief Sets the value of a 16-bit signed integer.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the integer value.
		 * \param value The new value of the integer.
		 * \return true if the value was set.  Returns false if the value is not a ZWValueID::ValueType_Short. The type can be tested with a call to ZWValueID::GetType
		 */
		bool SetValue( ZWValueID^ id, int16 value ){ return Manager::Get()->SetValue(id->CreateUnmanagedValueID(), value); }

		/**
		 * \brief Sets the value from a string, regardless of type.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the integer value.
		 * \param value The new value of the string.
		 * \return true if the value was set.  Returns false if the value could not be parsed into the correct type for the value.</returns>
		 */
		bool SetValue( ZWValueID^ id, String^ value ){ return Manager::Get()->SetValue(id->CreateUnmanagedValueID(), string((const char*)((Marshal::StringToHGlobalAnsi(value)).ToPointer())) ); }

		/**
		 * \brief Sets the selected item in a list.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the value
		 * held by the node is updated directly.  This will be reverted by a future status message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param id The unique identifier of the list value.
		 * \param value A string matching the new selected item in the list.
		 * \return true if the value was set.  Returns false if the selection is not in the list, or if the value is not a ZWValueID::ValueType_List.
		 * The type can be tested with a call to ZWValueID::GetType
		 */
		bool SetValueListSelection( ZWValueID^ id, String^ selectedItem ){ return Manager::Get()->SetValueListSelection(id->CreateUnmanagedValueID(), (const char*)(Marshal::StringToHGlobalAnsi(selectedItem)).ToPointer()); }
	
		/**
		 * \brief Refreshes the specified value from the Z-Wave network.
		 * A call to this function causes the library to send a message to the network to retrieve the current value
		 * of the specified ValueID (just like a poll, except only one-time, not recurring).
		 * \param _id The unique identifier of the value to be refreshed.
		 * \return true if the driver and node were found; false otherwise
		 */
		bool RefreshValue( ZWValueID^ id ){ return Manager::Get()->RefreshValue(id->CreateUnmanagedValueID()); }

		/**
		 * \brief Sets a flag indicating whether value changes noted upon a refresh should be verified.  If so, the
		 * library will immediately refresh the value a second time whenever a change is observed.  This helps to filter
		 * out spurious data reported occasionally by some devices.
		 * \param _id The unique identifier of the value whose changes should or should not be verified.
		 * \param _verify if true, verify changes; if false, don't verify changes.
		 */
		void SetChangeVerified( ZWValueID^ id, bool verify ){ Manager::Get()->SetChangeVerified(id->CreateUnmanagedValueID(), verify); }

		/**
		 * \brief Starts an activity in a device.
		 *
		 * Since buttons are write-only values that do not report a state, no notification callbacks are sent.
		 * \param id The unique identifier of the integer value.
		 * \return true if the activity was started.  Returns false if the value is not a ZWValueID::ValueType_Button. The type can be tested with a call to ZWValueID::GetType
		 */
		bool PressButton( ZWValueID^ id ){ return Manager::Get()->PressButton(id->CreateUnmanagedValueID()); }

		/**
		 * \brief Stops an activity in a device.
		 *
		 * Since buttons are write-only values that do not report a state, no notification callbacks are sent.
		 * \param id The unique identifier of the integer value.
		 * \return true if the activity was stopped.  Returns false if the value is not a ZWValueID::ValueType_Button. The type can be tested with a call to ZWValueID::GetType
		 */
		bool ReleaseButton( ZWValueID^ id ){ return Manager::Get()->ReleaseButton(id->CreateUnmanagedValueID()); }
		
	/*@}*/

	//-----------------------------------------------------------------------------
	// Climate Control Schedules
	//-----------------------------------------------------------------------------
	/** \name Climate Control Schedules
	 *  Methods for accessing schedule values.  All the methods require a ValueID, which will have been provided
	 *  in the ValueAdded Notification callback when the the value was first discovered by OpenZWave.
	 *  <p>The ValueType_Schedule is a specialized Value used to simplify access to the switch point schedule
	 *  information held by a setback thermostat that supports the Climate Control Schedule command class.
	 *  Each schedule contains up to nine switch points for a single day, consisting of a time in
	 *  hours and minutes (24 hour clock) and a setback in tenths of a degree Celsius.  The setback value can
	 *  range from -128 (-12.8C) to 120 (12.0C).  There are two special setback values - 121 is used to set
	 *  Frost Protection mode, and 122 is used to set Energy Saving mode.
	 *  <p>The switch point methods only modify OpenZWave's copy of the schedule information.  Once all changes
	 *  have been made, they are sent to the device by calling SetSchedule.
	 */
	/*@{*/

		/**
		 * \brief Get the number of switch points defined in a schedule.
		 * \param _id The unique identifier of the schedule value.
		 * \return the number of switch points defined in this schedule.  Returns zero if the value is not a ValueID::ValueType_Schedule. The type can be tested with a call to ValueID::GetType.
		 */
		uint8 GetNumSwitchPoints( ZWValueID^ id ){ return Manager::Get()->GetNumSwitchPoints( id->CreateUnmanagedValueID() ); }

		/**
		 * \brief Set a switch point in the schedule.
		 * Inserts a new switch point into the schedule, unless a switch point already exists at the specified
		 * time in which case that switch point is updated with the new setback value instead.
		 * A maximum of nine switch points can be set in the schedule.
		 * \param id The unique identifier of the schedule value.
		 * \param hours The hours part of the time when the switch point will trigger.  The time is set using
		 * the 24-hour clock, so this value must be between 0 and 23.
		 * \param minutes The minutes part of the time when the switch point will trigger.  This value must be
		 * between 0 and 59.
		 * \param setback The setback in tenths of a degree Celsius.  The setback value can range from -128 (-12.8C)
		 * to 120 (12.0C).  There are two special setback values - 121 is used to set Frost Protection mode, and
		 * 122 is used to set Energy Saving mode.
		 * \return true if successful.  Returns false if the value is not a ValueID::ValueType_Schedule. The type can be tested with a call to ValueID::GetType.
		 * \see GetNumSwitchPoints, RemoveSwitchPoint, ClearSwitchPoints
		 */
		bool SetSwitchPoint( ZWValueID^ id, uint8 hours, uint8 minutes, int8 setback ){ return Manager::Get()->SetSwitchPoint( id->CreateUnmanagedValueID(), hours, minutes, setback ); }

		/**
		 * \brief Remove a switch point from the schedule.
		 * Removes the switch point at the specified time from the schedule.
		 * \param id The unique identifier of the schedule value.
		 * \param hours The hours part of the time when the switch point will trigger.  The time is set using
		 * the 24-hour clock, so this value must be between 0 and 23.
		 * \param minutes The minutes part of the time when the switch point will trigger.  This value must be
		 * between 0 and 59.
		 * \return true if successful.  Returns false if the value is not a ValueID::ValueType_Schedule or if there 
		 * is not switch point with the specified time values. The type can be tested with a call to ValueID::GetType.
		 * \see GetNumSwitchPoints, SetSwitchPoint, ClearSwitchPoints
		 */
		bool RemoveSwitchPoint( ZWValueID^ id, uint8 hours, uint8 minutes ){ return Manager::Get()->RemoveSwitchPoint( id->CreateUnmanagedValueID(), hours, minutes ); }

		/**
		 * \brief Clears all switch points from the schedule.
		 * \param id The unique identifier of the schedule value.
		 * \see GetNumSwitchPoints, SetSwitchPoint, RemoveSwitchPoint
		 */
		void ClearSwitchPoints( ZWValueID^ id ){ Manager::Get()->ClearSwitchPoints( id->CreateUnmanagedValueID() ); }
		
		/**
		 * \brief Gets switch point data from the schedule.
		 * Retrieves the time and setback values from a switch point in the schedule.
		 * \param _id The unique identifier of the schedule value.
		 * \param _idx The index of the switch point, between zero and one less than the value
		 * returned by GetNumSwitchPoints.
		 * \param o_hours a pointer to a uint8 that will be filled with the hours part of the switch point data.
		 * \param o_minutes a pointer to a uint8 that will be filled with the minutes part of the switch point data.
		 * \param o_setback a pointer to an int8 that will be filled with the setback value.  This can range from -128
		 * (-12.8C)to 120 (12.0C).  There are two special setback values - 121 is used to set Frost Protection mode, and
		 * 122 is used to set Energy Saving mode.
		 * \return true if successful.  Returns false if the value is not a ValueID::ValueType_Schedule. The type can be tested with a call to ValueID::GetType.
		 * \see GetNumSwitchPoints
		 */
		bool GetSwitchPoint( ZWValueID^ id, uint8 idx, [Out] System::Byte %o_value, [Out] System::Byte %o_minutes, [Out] System::SByte %o_setback );
		
	/*@}*/

	//-----------------------------------------------------------------------------
	// SwitchAll
	//-----------------------------------------------------------------------------
	/** \name SwitchAll
	 *  Methods for switching all devices on or off together.  The devices must support
	 *	the SwitchAll command class.  The command is first broadcast to all nodes, and
	 *	then followed up with individual commands to each node (because broadcasts are
	 *	not routed, the message might not otherwise reach all the nodes).
	 */
	/*@{*/

		/**
		 * \brief Switch all devices on.
		 * All devices that support the SwitchAll command class will be turned on.
		 */
		void SwitchAllOn( uint32 homeId ){ Manager::Get()->SwitchAllOn( homeId ); }

		/**
		 * \brief Switch all devices off.
		 * All devices that support the SwitchAll command class will be turned off.
		 */
		void SwitchAllOff( uint32 homeId ){ Manager::Get()->SwitchAllOff( homeId ); }

	/*@}*/

	//-----------------------------------------------------------------------------
	// Configuration Parameters
	//-----------------------------------------------------------------------------
	/** \name Configuration Parameters
	 *  Methods for accessing device configuration parameters.
	 *  Configuration parameters are values that are managed by the Configuration command class.
	 *	The values are device-specific and are not reported by the devices. Information on parameters
	 *  is provided only in the device user manual.
	 *  <p>An ongoing task for the OpenZWave project is to create XML files describing the available
	 *  parameters for every Z-Wave.  See the config folder in the project source code for examples.
	 */
	/*@{*/
	public:		
		/**
		 * \brief Set the value of a configurable parameter in a device.
		 *
		 * Some devices have various parameters that can be configured to control the device behaviour.
		 * These are not reported by the device over the Z-Wave network, but can usually be found in
		 * the device's user manual.
		 * This method returns immediately, without waiting for confirmation from the device that the
		 * change has been made.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to configure.
		 * \param _param The index of the parameter.
		 * \param value The value to which the parameter should be set.
		 * \return true if the a message setting the value was sent to the device.
		 * \see RequestConfigParam
		 */
		bool SetConfigParam( uint32 homeId, uint8 nodeId, uint8 param, int32 value ){ return Manager::Get()->SetConfigParam( homeId, nodeId, param, value ); }

		/**
		 * \brief Request the value of a configurable parameter from a device.
		 *
		 * Some devices have various parameters that can be configured to control the device behaviour.
		 * These are not reported by the device over the Z-Wave network, but can usually be found in
		 * the device's user manual.
		 * This method requests the value of a parameter from the device, and then returns immediately, 
		 * without waiting for a response.  If the parameter index is valid for this device, and the 
		 * device is awake, the value will eventually be reported via a ValueChanged notification callback.
		 * The ValueID reported in the callback will have an index set the same as _param and a command class
		 * set to the same value as returned by a call to Configuration::StaticGetCommandClassId. 
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to configure.
		 * \param _param The index of the parameter.
		 * \see SetConfigParam, ValueID, Notification
		 */
		void RequestConfigParam( uint32 homeId, uint8 nodeId, uint8 param ){ Manager::Get()->RequestConfigParam( homeId, nodeId, param ); }

		/**
		 * \brief Request the values of all known configurable parameters from a device.
		 *
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node to configure.
		 * \see SetConfigParam, RequestConfigParam, ValueID, Notification
		 */
		void RequestAllConfigParams( uint32 homeId, uint8 nodeId ){ Manager::Get()->RequestAllConfigParams( homeId, nodeId ); }
	/*@}*/

	//-----------------------------------------------------------------------------
	// Groups (wrappers for the Node methods)
	//-----------------------------------------------------------------------------
	/** \name Groups
	 *  Methods for accessing device association groups.
	 */
	/*@{*/
	public:		
		/**
		 * \brief Gets the number of association groups reported by this node.
		 *
		 * In Z-Wave, groups are numbered starting from one.  For example, if a call to GetNumGroups returns 4, the _groupIdx 
		 * value to use in calls to GetAssociations, AddAssociation and RemoveAssociation will be a number between 1 and 4.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node whose groups we are interested in.
		 * \return The number of groups.
		 * \see GetAssociations, AddAssociation, RemoveAssociation
		 */
		uint8 GetNumGroups( uint32 homeId, uint8 nodeId ){ return Manager::Get()->GetNumGroups( homeId, nodeId ); }

		/**
		 * \brief Gets the associations for a group.
		 *
		 * Makes a copy of the list of associated nodes in the group, and returns it in an array of uint8's.
		 * The caller is responsible for freeing the array memory with a call to delete [].
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node whose associations we are interested in.
		 * \param _groupIdx One-based index of the group (because Z-Wave product manuals use one-based group numbering).
		 * \param o_associations If the number of associations returned is greater than zero, o_associations will be set to point to an array containing the IDs of the associated nodes.
		 * \return The number of nodes in the associations array.  If zero, the array will point to NULL, and does not need to be deleted.
		 * \see GetNumGroups, AddAssociation, RemoveAssociation
		 */
		uint32 GetAssociations( uint32 const homeId, uint8 const nodeId, uint8 const groupIdx, [Out] array<Byte>^ %o_associations );

		/**
		 * \brief Gets the maximum number of associations for a group.
		 *
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node whose associations we are interested in.
		 * \param _groupIdx one-based index of the group (because Z-Wave product manuals use one-based group numbering).
		 * \return The maximum number of nodes that can be associated into the group.
		 * \see GetNumGroups, AddAssociation, RemoveAssociation
		 */
		uint8 GetMaxAssociations( uint32 const homeId, uint8 const nodeId, uint8 const groupIdx ){ return Manager::Get()->GetMaxAssociations( homeId, nodeId, groupIdx ); }

		/**
		 * \brief Adds a node to an association group.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the association data
		 * held in this class is updated directly.  This will be reverted by a future Association message from the device
		 * if the Z-Wave message actually failed to get through.  Notification callbacks will be sent in both cases.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node whose associations are to be changed.
		 * \param _groupIdx One-based index of the group (because Z-Wave product manuals use one-based group numbering).
		 * \param _targetNodeId Identifier for the node that will be added to the association group.
		 * \see GetNumGroups, GetAssociations, RemoveAssociation
		 */
		void AddAssociation( uint32 homeId, uint8 nodeId, uint8 groupIdx, uint8 targetNodeId ){ return Manager::Get()->AddAssociation( homeId, nodeId, groupIdx, targetNodeId ); }

		/**
		 * \brief Removes a node from an association group.
		 *
		 * Due to the possibility of a device being asleep, the command is assumed to suceeed, and the association data
		 * held in this class is updated directly.  This will be reverted by a future Association message from the device
		 * if the Z-Wave message actually failed to get through.   Notification callbacks will be sent in both cases.
		 * \param homeId The Home ID of the Z-Wave controller that manages the node.
		 * \param nodeId The ID of the node whose associations are to be changed.
		 * \param _groupIdx One-based index of the group (because Z-Wave product manuals use one-based group numbering).
		 * \param _targetNodeId Identifier for the node that will be removed from the association group.
		 * \see GetNumGroups, GetAssociations, AddAssociation
		 */
		void RemoveAssociation( uint32 homeId, uint8 nodeId, uint8 groupIdx, uint8 targetNodeId ){ return Manager::Get()->RemoveAssociation( homeId, nodeId, groupIdx, targetNodeId ); }
	/*@}*/

	//-----------------------------------------------------------------------------
	// Controller commands
	//-----------------------------------------------------------------------------
	/** \name Controller Commands
	 *  Commands for Z-Wave network management using the PC Controller.
	 */
	/*@{*/
	public:	
		/**
		 * \brief Hard Reset a PC Z-Wave Controller.
		 *
		 * Resets a controller and erases its network configuration settings.  The controller becomes a primary controller ready to add devices to a new network.
		 * \param homeId The Home ID of the Z-Wave controller to be reset.
		 * \see SoftReset
		 */
		void ResetController( uint32 homeId ){ Manager::Get()->ResetController( homeId ); }

		/**
		 * \brief Soft Reset a PC Z-Wave Controller.
		 *
		 * Resets a controller without erasing its network configuration settings.
		 * \param homeId The Home ID of the Z-Wave controller to be reset.
		 * \see SoftReset
		 */
		void SoftReset( uint32 homeId ){ Manager::Get()->SoftReset( homeId ); }

		/**
		 * \brief Start a controller command process.
		 *
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \param command The command to be sent to the controller.
		 * \param highPower used only with the AddDevice, AddController, RemoveDevice and RemoveController commands. 
		 * Usually when adding or removing devices, the controller operates at low power so that the controller must
		 * be physically close to the device for security reasons.  If _highPower is true, the controller will 
		 * operate at normal power levels instead.  Defaults to false.
		 * \param nodeId used only with the ReplaceFailedNode command, to specify the node that is going to be replaced.
		 * \return true if the command was accepted and has started.
		 * \see CancelControllerCommand, HasNodeFailed, RemoveFailedNode, Driver::ControllerCommand, Driver::pfnControllerCallback_t, 
		 * to notify the user of progress or to request actions on the user's part.  Defaults to NULL.
		 * <p> Commands
		 * - ZWControllerCommand.AddController - Add a new secondary controller to the Z-Wave network.
		 * - ZWControllerCommand.AddDevice - Add a new device (but not a controller) to the Z-Wave network.
		 * - ZWControllerCommand.CreateNewPrimary (Not yet implemented)
		 * - ZWControllerCommand.ReceiveConfiguration -   
		 * - ZWControllerCommand.RemoveController - remove a controller from the Z-Wave network.
		 * - ZWControllerCommand.RemoveDevice - remove a device (but not a controller) from the Z-Wave network.
 		 * - ZWControllerCommand.RemoveFailedNode - move a node to the controller's list of failed nodes.  The node must actually
		 * have failed or have been disabled since the command will fail if it responds.  A node must be in the controller's failed nodes list
		 * for ControllerCommand_ReplaceFailedNode to work.
		 * - ZWControllerCommand.HasNodeFailed - Check whether a node is in the controller's failed nodes list.
		 * - ZWControllerCommand.ReplaceFailedNode - replace a failed device with another. If the node is not in 
		 * the controller's failed nodes list, or the node responds, this command will fail.
		 * - ZWControllerCommand.TransferPrimaryRole (Not yet implemented) - Add a new controller to the network and
		 * make it the primary.  The existing primary will become a secondary controller.  
		 * - ZWControllerCommand.RequestNetworkUpdate - Update the controller with network information from the SUC/SIS.
		 * - ZWControllerCommand.RequestNodeNeighborUpdate - Get a node to rebuild its neighbour list.  This method also does ControllerCommand_RequestNodeNeighbors afterwards.
		 * - ZWControllerCommand.AssignReturnRoute - Assign network routes to a device.
		 * - ZWControllerCommand.DeleteReturnRoute - Delete network routes from a device.
		 * <p>These processes are asynchronous, and at various stages OpenZWave will trigger a callback
		 * to notify the user of progress or to request actions on the user's part.
		 * <p> Controller States
		 * - ZWControllerState.Waiting, the controller is waiting for a user action.  A notice should be displayed 
		 * to the user at this point, telling them what to do next.
		 * For the add, remove, replace and transfer primary role commands, the user needs to be told to press the 
		 * inclusion button on the device that  is going to be added or removed.  For ControllerCommand_ReceiveConfiguration, 
		 * they must set their other controller to send its data, and for ControllerCommand_CreateNewPrimary, set the other
		 * controller to learn new data.
		 * - ZWControllerState.InProgress - the controller is in the process of adding or removing the chosen node.
		 * - ZWControllerState.Complete - the controller has finished adding or removing the node, and the command is complete.
		 * - ZWControllerState.Failed - will be sent if the command fails for any reason.
		 * <p>To register for these notifications, create an event handler with the same signature as
		 * the ManagedControllerStateChangedHandler delegate.  Just before calling the BeginControllerCommand
		 * method, subscribe to the OnControllerStateChanged event.  Once the command has completed, remember
		 * to unsubscribe from the event.
		 * /code
		 * private UInt32 m_homeId;
		 * private ZWManager m_manager;
		 * private ManagedControllerStateChangedHandler m_myEventHandler = new ManagedControllerStateChangedHandler( MyControllerStateChangedHandler );
		 *
		 * public void MyAddControllerMethod()
		 * {
		 *     m_manager.OnControllerStateChanged += m_myEventHandler;
		 *     m_manager.BeginControllerCommand( m_homeId, ZWControllerCommand::AddController, false );		
		 * }
		 *
		 * public void MyControllerStateChangedHandler( ZWControllerState state )
		 * {
		 *     // Handle the controller state notifications here.
		 *     bool complete = false;
		 *     switch( state )
		 *     {
		 *         case ZWControllerState::Waiting:
		 *         {
	     *             // Display a message to tell the user to press the include button on the controller
		 *             break;
		 *         }
		 *         case ZWControllerState::InProgress:
		 *         {
		 *             // Tell the user that the controller has been found and the adding process is in progress.
		 *             break;
		 *         }
		 *         case ZWControllerState::Completed:
		 *         {
		 *             // Tell the user that the controller has been successfully added.
		 *             // The command is now complete
		 *             complete = true;
		 *             break;
		 *         }
		 *         case ZWControllerState::Failed:
		 *         {
		 *             // Tell the user that the controller addition process has failed.
		 *             // The command is now complete
		 *             complete = true;
		 *             break;
		 *         }
		 *         case ZWControllerState::NodeOK:
		 *         {
		 *             // Tell the user that the node referenced by one of the Failed commands is actually working.
		 *             // The command is now complete
		 *             complete = true;
		 *             break;
		 *         }
		 *         case ZWControllerState::Failed:
		 *         {
		 *             // Tell the user that the node referenced in the HasNodeFailed command has failed.
		 *             // The command is now complete
		 *             complete = true;
		 *             break;
		 *         }
		 *     }
		 *
		 *     if( complete )
		 *     {
		 *         // Remove the event handler
		 *         m_manager.OnControllerStateChanged -= m_myEventHandler;
		 *     }
		 * }
		 * /endcode
		 */
		bool BeginControllerCommand( uint32 homeId, ZWControllerCommand command, bool highPower, uint8 nodeId );
			
		/**
		 * \brief Cancels any in-progress command running on a controller.
		 *
		 * \param homeId The Home ID of the Z-Wave controller.
		 * \return true if a command was running and was cancelled.
		 * \see BeginControllerCommand 
		 */
		bool CancelControllerCommand(uint32 homeId){ return Manager::Get()->CancelControllerCommand(homeId); }		
		
	/*@}*/

	//-----------------------------------------------------------------------------
	// Network commands
	//-----------------------------------------------------------------------------
	/** \name Network Commands
	 *  Commands for Z-Wave network for testing, routing and other internal
	 *  operations.
	 */
	/*@{*/
	public:	
		/**
		 * \brief Test network node.
		 * Sends a series of messages to a network node for testing network reliability.
		 * \param homeId The Home ID of the Z-Wave controller to be reset.
		 * \param count This is the number of test messages to send.
		 * \see TestNetwork
		 */
		void TestNetworkNode( uint32 const homeId, uint8 const nodeId, uint32 const count ){ Manager::Get()->TestNetworkNode(homeId, nodeId, count);}

		/**
		 * \brief Test network.
		 * Sends a series of messages to every node on the network for testing network reliability.
		 * \param homeId The Home ID of the Z-Wave controller to be reset.
		 * \param count This is the number of test messages to send.
		 * \see TestNetwork
		 */
		void TestNetwork( uint32 const homeId, uint32 const count ){ Manager::Get()->TestNetwork(homeId, count);}

 		/**
		 * \brief Heal network node by requesting the node rediscover their neighbors.
		 * Sends a ControllerCommand_RequestNodeNeighborUpdate to the node.
		 * \param homeId The Home ID of the Z-Wave network to be healed.
		 * \param nodeId The node to heal.
		 * \param doRR Whether to perform return routes initialization.
		 */
		void HealNetworkNode( uint32 const homeId, uint8 const nodeId, bool doRR ){ Manager::Get()->HealNetworkNode(homeId, nodeId, doRR);}

 		/**
		 * \brief Heal network by requesting node's rediscover their neighbors.
		 * Sends a ControllerCommand_RequestNodeNeighborUpdate to every node.
		 * Can take a while on larger networks.
		 * \param homeId The Home ID of the Z-Wave network to be healed.
		 * \param doRR Whether to perform return routes initialization.
		 */
		void HealNetwork( uint32 const homeId, bool doRR ){ Manager::Get()->HealNetwork(homeId, doRR);}

		/**
		* \brief Start the Inclusion Process to add a Node to the Network.
		* The Status of the Node Inclusion is communicated via Notifications. Specifically, you should
		* monitor ControllerCommand Notifications.
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The Home ID of the Z-Wave network where the device should be added.
		* \param doSecurity Whether to initialize the Network Key on the device if it supports the Security CC
		* \return if the Command was sent succcesfully to the Controller
		*/
		bool AddNode(uint32 const homeId, bool doSecurity){	return Manager::Get()->AddNode(homeId, doSecurity);	}

		/**
		* \brief Remove a Device from the Z-Wave Network
		* The Status of the Node Removal is communicated via Notifications. Specifically, you should
		* monitor ControllerCommand Notifications.
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network where you want to remove the device
		* \return if the Command was send succesfully to the Controller
		*/
		bool RemoveNode(uint32 const homeId){return Manager::Get()->RemoveNode(homeId);}

		/**
		* \brief Remove a Failed Device from the Z-Wave Network
		* This Command will remove a failed node from the network. The Node should be on the Controllers Failed
		* Node List, otherwise this command will fail. You can use the HasNodeFailed function below to test if the Controller
		* believes the Node has Failed.
		* The Status of the Node Removal is communicated via Notifications. Specifically, you should
		* monitor ControllerCommand Notifications.
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network where you want to remove the device
		* \param nodeId The NodeID of the Failed Node.
		* \return if the Command was send succesfully to the Controller
		*/
		bool RemoveFailedNode(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->RemoveFailedNode(homeId, nodeId); }

		/**
		* \brief Check if the Controller Believes a Node has Failed.
		* This is different from the IsNodeFailed call in that we test the Controllers Failed Node List, whereas the IsNodeFailed is testing
		* our list of Failed Nodes, which might be different.
		* The Results will be communicated via Notifications. Specifically, you should monitor the ControllerCommand notifications
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network where you want to test the device
		* \param nodeId The NodeID of the Failed Node.
		* \return if the RemoveDevice Command was send succesfully to the Controller
		*/
		bool HasNodeFailed(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->HasNodeFailed(homeId, nodeId); }

		/**
		* \brief Ask a Node to update its update its Return Route to the Controller
		* This command will ask a Node to update its Return Route to the Controller
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network where you want to update the device
		* \param nodeId The NodeID of the Node.
		* \return if the Command was send succesfully to the Controller
		*/
		bool AssignReturnRoute(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->AssignReturnRoute(homeId, nodeId); }

		/**
		* \brief Ask a Node to update its Neighbor Tables
		* This command will ask a Node to update its Neighbor Tables.
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network where you want to update the device
		* \param nodeId The NodeID of the Node.
		* \return if the Command was send succesfully to the Controller
		*/
		bool RequestNodeNeighborUpdate(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->RequestNodeNeighborUpdate(homeId, nodeId); }

		/**
		* \brief Ask a Node to delete all Return Route.
		* This command will ask a Node to delete all its return routes, and will rediscover when needed.
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network where you want to update the device
		* \param nodeId The NodeID of the Node.
		* \return if the Command was send succesfully to the Controller
		*/
		bool DeleteAllReturnRoutes(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->DeleteAllReturnRoutes(homeId, nodeId); }

		/**
		* \brief Send a NIF frame from the Controller to a Node.
		* This command send a NIF frame from the Controller to a Node
		*
		* Results of the AddNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \param nodeId The NodeID of the Node to recieve the NIF
		* \return if the sendNIF Command was send succesfully to the Controller
		*/
		bool SendNodeInformation(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->SendNodeInformation(homeId, nodeId); }

		/**
		* \brief Create a new primary controller when old primary fails. Requires SUC.
		* This command Creates a new Primary Controller when the Old Primary has Failed. Requires a SUC on the network to function
		*
		* Results of the CreateNewPrimary Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \return if the CreateNewPrimary Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool CreateNewPrimary(uint32 const homeId){ return Manager::Get()->CreateNewPrimary(homeId); }

		/**
		* \brief Receive network configuration information from primary controller. Requires secondary.
		* This command prepares the controller to recieve Network Configuration from a Secondary Controller.
		*
		* Results of the ReceiveConfiguration Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \return if the ReceiveConfiguration Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool ReceiveConfiguration(uint32 const homeId){ return Manager::Get()->ReceiveConfiguration(homeId); }

		/**
		* \brief Replace a failed device with another.
		* If the node is not in the controller's failed nodes list, or the node responds, this command will fail.
		* You can check if a Node is in the Controllers Failed node list by using the HasNodeFailed method
		*
		* Results of the ReplaceFailedNode Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \param nodeId the ID of the Failed Node
		* \return if the ReplaceFailedNode Command was send succesfully to the Controller
		* \sa HasNodeFailed
		* \sa CancelControllerCommand
		*/
		bool ReplaceFailedNode(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->ReplaceFailedNode(homeId, nodeId); }

		/**
		* \brief Add a new controller to the network and make it the primary.
		* The existing primary will become a secondary controller.
		*
		* Results of the TransferPrimaryRole Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \return if the TransferPrimaryRole Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool TransferPrimaryRole(uint32 const homeId){ return Manager::Get()->TransferPrimaryRole(homeId); }

		/**
		* \brief Update the controller with network information from the SUC/SIS.
		*
		* Results of the RequestNetworkUpdate Command will be send as a Notification with the Notification type asc
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \param nodeId the ID of the Node
		* \return if the RequestNetworkUpdate Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool RequestNetworkUpdate(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->RequestNetworkUpdate(homeId, nodeId); }

		/**
		* \brief Send information from primary to secondary
		*
		* Results of the ReplicationSend Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \param nodeId the ID of the Node
		* \return if the ReplicationSend Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool ReplicationSend(uint32 const homeId, uint8 const nodeId){ return Manager::Get()->ReplicationSend(homeId, nodeId); }

		/**
		* \brief Create a handheld button id.
		*
		* Only intended for Bridge Firmware Controllers.
		*
		* Results of the CreateButton Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \param nodeId the ID of the Virtual Node
		* \param buttonId the ID of the Button to create
		* \return if the CreateButton Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool CreateButton(uint32 const homeId, uint8 const nodeId, uint8 const buttonid) { return Manager::Get()->CreateButton(homeId, nodeId, buttonid); }
		

		/**
		* \brief Dekete a handheld button id.
		*
		* Only intended for Bridge Firmware Controllers.
		*
		* Results of the DeleteButton Command will be send as a Notification with the Notification type as
		* Notification::Type_ControllerCommand
		*
		* \param homeId The HomeID of the Z-Wave network
		* \param nodeId the ID of the Virtual Node
		* \param buttonId the ID of the Button to delete
		* \return if the DeleteButton Command was send succesfully to the Controller
		* \sa CancelControllerCommand
		*/
		bool DeleteButton(uint32 const homeId, uint8 const nodeId, uint8 const buttonid){ return Manager::Get()->DeleteButton(homeId, nodeId, buttonid); }

	//-----------------------------------------------------------------------------
	// Scene commands
	//-----------------------------------------------------------------------------
	/** \name Scene Commands
	 *  Commands for Z-Wave scene interface.
	 */
	/*@{*/
	public:	
		/**
		 * \brief Gets the number of scenes that have been defined.
		 * \return The number of scenes.
		 * \see GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		uint8 GetNumScenes(){ return Manager::Get()->GetNumScenes(); }

		/**
		 * \brief Gets a list of all the SceneIds.
		 * \param sceneIds returns an array of bytes containing the ids if the existing scenes.
		 * \return The number of scenes.
		 * \see GetNumScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		uint8 GetAllScenes( [Out] array<Byte>^ sceneIds );

		/**
		 * \brief Create a new Scene passing in Scene ID
		 * \return uint8 Scene ID used to reference the scene. 0 is failure result.
		 * \see GetNumScenes, GetAllScenes, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene

		 */
		uint8 CreateScene(){ return Manager::Get()->CreateScene(); }

		/**
		 * \brief Remove an existing Scene.
		 * \param sceneId is an integer representing the unique Scene ID to be removed.
		 * \return true if scene was removed.
		 * \see GetNumScenes, GetAllScenes, CreateScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool RemoveScene( uint8 sceneId ){ return Manager::Get()->RemoveScene( sceneId ); }

		/**
		 * \brief Add a bool Value ID to an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the bool value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValue( uint8 sceneId, ZWValueID^ valueId, bool value ){ return Manager::Get()->AddSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Add a byte Value ID to an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the byte value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValue( uint8 sceneId, ZWValueID^ valueId, uint8 value ){ return Manager::Get()->AddSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Add a decimal Value ID to an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the float value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValue( uint8 sceneId, ZWValueID^ valueId, float const value ){ return Manager::Get()->AddSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Add a 32-bit signed integer Value ID to an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the int32 value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValue( uint8 sceneId, ZWValueID^ valueId, int32 const value ){ return Manager::Get()->AddSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Add a 16-bit signed integer Value ID to an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the int16 value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValue( uint8 sceneId, ZWValueID^ valueId, int16 const value ){ return Manager::Get()->AddSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Add a string Value ID to an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the string value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValue( uint8 sceneId, ZWValueID^ valueId, String^ value ){ return Manager::Get()->AddSceneValue( sceneId, valueId->CreateUnmanagedValueID(), string((const char*)((Marshal::StringToHGlobalAnsi(value)).ToPointer())) ); }

		/**
		 * \brief Add the selected item list Value ID to an existing scene (as a string).
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the string value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValueListSelection( uint8 sceneId, ZWValueID^ valueId, String^ value ){ return Manager::Get()->AddSceneValueListSelection( sceneId, valueId->CreateUnmanagedValueID(), string((const char*)((Marshal::StringToHGlobalAnsi(value)).ToPointer())) ); }

		/**
		 * \brief Add the selected item list Value ID to an existing scene (as a integer).
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the integer value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool AddSceneValueListSelection( uint8 sceneId, ZWValueID^ valueId, int32 value ){ return Manager::Get()->AddSceneValueListSelection( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Remove the Value ID from an existing scene.
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be removed.
		 * \return true if Value ID was removed.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool RemoveSceneValue( uint8 sceneId, ZWValueID^ valueId ){ return Manager::Get()->RemoveSceneValue( sceneId, valueId->CreateUnmanagedValueID() ); }

		/**
		 * \brief Retrieves the scene's list of values.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param o_value an array of ValueIDs.
		 * \return The number of nodes in the o_value array. If zero, the array will point to NULL and does not need to be deleted.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		int SceneGetValues( uint8 sceneId, [Out] array<ZWValueID ^>^ %o_values );

		/**
		 * \brief Retrieves a scene's value as a bool.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param boolean that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueAsBool( uint8 sceneId, ZWValueID^ valueId, [Out] System::Boolean %o_value );

		/**
		 * \brief Retrieves a scene's value as an 8-bit unsigned integer.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value Byte that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueAsByte( uint8 sceneId, ZWValueID^ valueId, [Out] System::Byte %o_value );

		/**
		 * \brief Retrieves a scene's value as a decimal.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value decimal that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueAsDecimal( uint8 sceneId, ZWValueID^ valueId, [Out] System::Decimal %o_value );

		/**
		 * \brief Retrieves a scene's value as a 32-bit signed integer.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value Int32 that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueAsInt( uint8 sceneId, ZWValueID^ valueId, [Out] System::Int32 %o_value );

		/**
		 * \brief Retrieves a scene's value as a 16-bit signed integer.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value Int16 that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueAsShort( uint8 sceneId, ZWValueID^ valueId, [Out] System::Int16 %o_value );

		/**
		 * \brief Retrieves a scene's value as a string.
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value Pointer to a string that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueAsString( uint8 sceneId, ZWValueID^ valueId, [Out] String^ %o_value );

		/**
		 * \brief Retrieves a scene's value as a list (as a string).
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value Pointer to a string that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueListSelection( uint8 sceneId, ZWValueID^ valueId, [Out] String^ %o_value );

		/**
		 * \brief Retrieves a scene's value as a list (as a integer).
		 * \param sceneId The Scene ID of the scene to retrieve the value from.
		 * \param valueId The Value ID of the value to retrieve.
		 * \param o_value Pointer to a integer that will be filled with the returned value.
		 * \return true if the value was obtained.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SceneGetValueListSelection( uint8 sceneId, ZWValueID^ valueId, System::Int32 %o_value );

		/**
		 * \brief Set a bool Value ID to an existing scene's ValueID
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the bool value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValue( uint8 sceneId, ZWValueID^ valueId, bool value ){ return Manager::Get()->SetSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Set a byte Value ID to an existing scene's ValueID
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the byte value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValue( uint8 sceneId, ZWValueID^ valueId, uint8 value ){ return Manager::Get()->SetSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Set a decimal Value ID to an existing scene's ValueID
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the float value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValue( uint8 sceneId, ZWValueID^ valueId, float value ){ return Manager::Get()->SetSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Set a 32-bit signed integer Value ID to an existing scene's ValueID
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the int32 value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValue( uint8 sceneId, ZWValueID^ valueId, int32 value ){ return Manager::Get()->SetSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Set a 16-bit integer Value ID to an existing scene's ValueID
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the int16 value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValue( uint8 sceneId, ZWValueID^ valueId, int16 value ){ return Manager::Get()->SetSceneValue( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Set a string Value ID to an existing scene's ValueID
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the string value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValue( uint8 sceneId, ZWValueID^ valueId, String^ value ){ return Manager::Get()->SetSceneValue( sceneId, valueId->CreateUnmanagedValueID(), string((const char*)((Marshal::StringToHGlobalAnsi(value)).ToPointer())) ); }

		/**
		 * \brief Set the list selected item Value ID to an existing scene's ValueID (as a string).
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the string value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValueListSelection( uint8 sceneId, ZWValueID^ valueId, String^ value ){ return Manager::Get()->SetSceneValueListSelection( sceneId, valueId->CreateUnmanagedValueID(), string((const char*)((Marshal::StringToHGlobalAnsi(value)).ToPointer())) ); }

		/**
		 * \brief Set the list selected item Value ID to an existing scene's ValueID (as a integer).
		 * \param sceneId is an integer representing the unique Scene ID.
		 * \param valueId is the Value ID to be added.
		 * \param value is the integer value to be saved.
		 * \return true if Value ID was added.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists, ActivateScene
		 */
		bool SetSceneValueListSelection( uint8 sceneId, ZWValueID^ valueId, int32 value ){ return Manager::Get()->SetSceneValueListSelection( sceneId, valueId->CreateUnmanagedValueID(), value ); }

		/**
		 * \brief Returns a label for the particular scene.
		 * \param sceneId The Scene ID
		 * \return The label string.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, SetSceneLabel, SceneExists, ActivateScene
		 */
		String^ GetSceneLabel( uint8 sceneId ){ return gcnew String(Manager::Get()->GetSceneLabel( sceneId ).c_str()); }

		/**
		 * \brief Sets a label for the particular scene.
		 * \param sceneId The Scene ID
		 * \param value The new value of the label.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SceneExists, ActivateScene
		 */
		void SetSceneLabel( uint8 sceneId, String^ value ){ return Manager::Get()->SetSceneLabel( sceneId, (const char*)(Marshal::StringToHGlobalAnsi(value)).ToPointer() ); }

		/**
		 * \brief Check if a Scene ID is defined.
		 * \param sceneId The Scene ID.
		 * \return true if Scene ID exists.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, ActivateScene
		 */
		bool SceneExists( uint8 sceneId ){ return Manager::Get()->SceneExists( sceneId ); }

		/**
		 * \brief Activate given scene to perform all its actions.
		 * \param sceneId The Scene ID.
		 * \return true if it is successful.
		 * \see GetNumScenes, GetAllScenes, CreateScene, RemoveScene, AddSceneValue, RemoveSceneValue, SceneGetValues, SceneGetValueAsBool, SceneGetValueAsByte, SceneGetValueAsFloat, SceneGetValueAsInt, SceneGetValueAsShort, SceneGetValueAsString, SetSceneValue, GetSceneLabel, SetSceneLabel, SceneExists
		 */
		bool ActivateScene( uint8 sceneId ){ return Manager::Get()->ActivateScene( sceneId ); }

	/*@}*/

	public:
		ZWManager(){}

	private:

		void  OnNotificationFromUnmanaged(Notification* _notification,void* _context);					// Forward notification to managed delegates hooked via Event addhandler 
		void  OnControllerStateChangedFromUnmanaged(Driver::ControllerState _state,void* _context);		// Forward controller state change to managed delegates hooked via Event addhandler 

		GCHandle										m_gchNotification;
		OnNotificationFromUnmanagedDelegate^			m_onNotification;

		GCHandle										m_gchControllerState;
		OnControllerStateChangedFromUnmanagedDelegate^	m_onStateChanged;
	};
}
