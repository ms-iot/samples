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

// {F990ABA9-C9C0-4CF8-A4A2-5B06D97F6871}
DEFINE_GUID(GUID_DEVCLASS_HIDINJECTOR,
    0xf990aba9, 0xc9c0, 0x4cf8, 0xa4, 0xa2, 0x5b, 0x6, 0xd9, 0x7f, 0x68, 0x71);

// {59819B74-F102-469A-9009-3CAF35FD4686}
DEFINE_GUID(GUID_DEVINTERFACE_HIDINJECTOR,
    0x59819b74, 0xf102, 0x469a, 0x90, 0x9, 0x3c, 0xaf, 0x35, 0xfd, 0x46, 0x86);

// {FFD216E4-A560-4676-8BD5-4F26A5BFF214}
DEFINE_GUID(GUID_BUS_HIDINJECTOR,
    0xffd216e4, 0xa560, 0x4676, 0x8b, 0xd5, 0x4f, 0x26, 0xa5, 0xbf, 0xf2, 0x14);


#define  HIDINJECTOR_DEVICE_ID L"{FFD216E4-A560-4676-8BD5-4F26A5BFF214}\\HidInjectorSample\0"

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

