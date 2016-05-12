/*++

Copyright (C) Microsoft Corporation, All Rights Reserved

Module Name:

    hidinjectorkd.h

Abstract:

    This module contains the type definitions for the driver

Environment:

    Windows Driver Framework (WDF)

--*/

#ifdef _KERNEL_MODE
#include <ntddk.h>
#else
#include <windows.h>
#endif

#include <wdf.h>
#include <hidport.h>
#include <vhf.h>
#include <initguid.h>
#include "common.h"

typedef UCHAR HID_REPORT_DESCRIPTOR, *PHID_REPORT_DESCRIPTOR;

DRIVER_INITIALIZE                   HIDINJECTOR_DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD           HIDINJECTOR_EvtDeviceAdd;
EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT HIDINJECTOR_EvtDeviceSelfManagedIoInit;
EVT_WDF_IO_QUEUE_IO_WRITE			HIDINJECTOR_EvtIoWriteForRawPdo;
EVT_WDF_IO_QUEUE_IO_WRITE			HIDINJECTOR_EvtIoWriteFromRawPdo;

typedef struct _HID_MAX_COUNT_REPORT
{
	UCHAR ReportID;
	UCHAR MaxCount;
} HIDINJECTOR_MAX_COUNT_REPORT, *PHIDINJECTOR_MAX_COUNT_REPORT;

typedef struct _HID_DEVICE_CONTEXT
{
    WDFDEVICE               Device;
    WDFQUEUE                DefaultQueue;
    WDFQUEUE                ManualQueue;
    HID_DESCRIPTOR          HidDescriptor;
    PHID_REPORT_DESCRIPTOR  ReportDescriptor;
	WDFDEVICE				RawPdo;
	WDFQUEUE				RawPdoQueue;	// Queue for handling requests that come from the rawPdo
	VHFHANDLE               VhfHandle;
} HID_DEVICE_CONTEXT, *PHID_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(HID_DEVICE_CONTEXT, GetHidDeviceContext);

typedef struct _QUEUE_CONTEXT
{
    WDFQUEUE                Queue;
    PHID_DEVICE_CONTEXT     DeviceContext;
} QUEUE_CONTEXT, *PQUEUE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(QUEUE_CONTEXT, GetQueueContext);


// Raw PDO context.  
typedef struct _RAWPDO_DEVICE_CONTEXT
{

	// TODO; is this used
	ULONG InstanceNo;

	//
	// Queue of the parent device we will forward requests to
	//
	WDFQUEUE ParentQueue;

} RAWPDO_DEVICE_CONTEXT, *PRAWPDO_DEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(RAWPDO_DEVICE_CONTEXT, GetRawPdoDeviceContext)

// {7390C4D1-774F-466A-A343-AF91CD702C45}
DEFINE_GUID(GUID_DEVCLASS_HIDINJECTOR,
	0x7390c4d1, 0x774f, 0x466a, 0xa3, 0x43, 0xaf, 0x91, 0xcd, 0x70, 0x2c, 0x45);

// {A8E7AF29-4F2F-43E0-AA80-086C12135547}
DEFINE_GUID(GUID_DEVINTERFACE_HIDINJECTOR,
	0xa8e7af29, 0x4f2f, 0x43e0, 0xaa, 0x80, 0x8, 0x6c, 0x12, 0x13, 0x55, 0x47);

// {97976E57-A740-4B31-A929-6B21B435BCC4}
DEFINE_GUID(GUID_BUS_HIDINJECTOR ,
	0x97976e57, 0xa740, 0x4b31, 0xa9, 0x29, 0x6b, 0x21, 0xb4, 0x35, 0xbc, 0xc4);


#define  HIDINJECTOR_DEVICE_ID L"{97976E57-A740-4B31-A929-6B21B435BCC4}\\HidInjector\0"

// TODO: make sure there's no stale prototype in here

NTSTATUS
RawQueueCreate(
	_In_ WDFDEVICE										Device,
	_Out_ WDFQUEUE										*Queue
	);

NTSTATUS
HIDINJECTOR_CreateRawPdo(
	_In_  WDFDEVICE         Device
	);

EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT     HIDINJECTOR_EvtDeviceSelfManagedIoInit;
EVT_WDF_DEVICE_SELF_MANAGED_IO_INIT     RAWPDO_EvtDeviceSelfManagedIoInit;
EVT_WDF_DEVICE_SELF_MANAGED_IO_CLEANUP  HIDINJECTOR_EvtDeviceSelfManagedIoCleanup;

_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
HIDINJECTOR_VhfInitialize(
	WDFDEVICE WdfDevice
	);

VOID
HIDINJECTOR_VhfSubmitReadReport(
	_In_ PHID_DEVICE_CONTEXT DeviceContext,
	_In_ PUCHAR Report,
	_In_ size_t ReportSize
	);

NTSTATUS
HIDINJECTOR_GetFeatureReport(
	_In_ PHID_DEVICE_CONTEXT DeviceContext,
	_In_ PHID_XFER_PACKET    HidTransferPacket
	);

VOID
HIDINJECTOR_VhfAsyncOperationGetFeature(
	_In_     PVOID              VhfClientContext,
	_In_     VHFOPERATIONHANDLE VhfOperationHandle,
	_In_opt_ PVOID              VhfOperationContext,
	_In_     PHID_XFER_PACKET   HidTransferPacket
	);

