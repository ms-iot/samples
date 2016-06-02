/*++

Copyright (C) Microsoft Corporation, All Rights Reserved.

Module Name:

    HidInjectorKd.cpp

Abstract:

    This module contains the implementation of the driver

Environment:

    Windows Driver Framework (WDF)

--*/

#include "HidInjectorKd.h"

//
// This is the default report descriptor for the virtual Hid device returned
// by the mini driver in response to IOCTL_HID_GET_REPORT_DESCRIPTOR.
//
HID_REPORT_DESCRIPTOR       G_DefaultReportDescriptor[] = {
	// Keyboard(STD101)
	0x05,   0x01,       // Usage Page(Generic Desktop),
	0x09,   0x06,       // Usage(Keyboard),
	0xA1,   0x01,       // Collection(Application),
	0x85,   0x01,       //  Report Id(1)
	0x05,   0x07,       //  usage page key codes
	0x19,   0xe0,       //  usage min left control
	0x29,   0xe7,       //  usage max keyboard right gui
	0x15,   0x00,       //  Logical Minimum(0),
	0x25,   0x01,       //  Logical Maximum(1),
	0x75,   0x01,       //  report size 1
	0x95,   0x08,       //  report count 8
	0x81,   0x02,       //  input(Variable)
	0x19,   0x00,       //  usage min 0
	0x29,   0x91,       //  usage max 91
	0x15,   0x00,       //  Logical Minimum(0),
	0x26,   0xff, 0x00, //  logical max 0xff
	0x75,   0x08,       //  report size 8
	0x95,   0x04,       //  report count 4
	0x81,   0x00,       //  Input(Data, Array),
	0x95,   0x05,       //  REPORT_COUNT (2)
	0x75,   0x08,       //  REPORT_SIZE (8)
	0x81,   0x03,       //  INPUT (Cnst,Var,Abs)
	0xC0,				// End Collection,

	// Mouse
	0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
	0x09, 0x02,                    // USAGE (Mouse)
	0xa1, 0x01,                    // COLLECTION (Application)
	0x85, 0x02,                    //  Report Id(2)
	0x09, 0x01,                    //   USAGE (Pointer)
	0xa1, 0x00,                    //   COLLECTION (Physical)
	0x05, 0x09,                    //     USAGE_PAGE (Button)
	0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
	0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
	0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
	0x95, 0x03,                    //     REPORT_COUNT (3)
	0x75, 0x01,                    //     REPORT_SIZE (1)
	0x81, 0x02,                    //     INPUT (Data,Var,Abs)
	0x95, 0x01,                    //     REPORT_COUNT (1)
	0x75, 0x05,                    //     REPORT_SIZE (5)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                    //     USAGE (X)
	0x09, 0x31,                    //     USAGE (Y)
	0x16, 0x00, 0x00,              //     Logical Minimum(0)
	0x26, 0xFF, 0x7F,              //     Logical Maximum(32767)
	0x36, 0x00, 0x00,              //     Physical Minimum(0)
	0x46, 0xFF, 0x7F,              //     Physical Maximum(32767)
	0x66, 0x00, 0x00,              //     Unit(None)
	0x75, 0x10,                    //     Report Size(16)
	0x95, 0x02,                    //     Report Count(2)
	0x81, 0x62,                    //     Input(Data, Variable, Absolute, No Preferred, Null State)
	0x95, 0x05,                    //     REPORT_COUNT (2)
	0x75, 0x08,                    //     REPORT_SIZE (8)
	0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
	0xc0,                          //   END_COLLECTION
    0xc0,                          // END_COLLECTION
                                   
    // Touch
	0x05, 0x0d,                     // USAGE_PAGE (Digitizers)
	0x09, 0x04,                     // USAGE (Touch Screen)
	0xa1, 0x01,                     // COLLECTION (Application)
	0x85, 0x03,                     //  Report Id(3)
	0x09, 0x22,                     //   USAGE (Finger)
	0xa1, 0x02,                     //   COLLECTION (Logical)
	0x09, 0x42,                     //     USAGE (Tip Switch)
	0x09, 0x32,                     //     USAGE (In Range)
	0x15, 0x00,                     //     LOGICAL_MINIMUM (0)
	0x25, 0x01,                     //     LOGICAL_MAXIMUM (1)
	0x75, 0x01,                     //     REPORT_SIZE (1)
	0x95, 0x02,                     //     REPORT_COUNT (2)
	0x81, 0x02,                     //     INPUT (Data,Var,Abs)
	0x95, 0x06,                     //     REPORT_COUNT (6)
	0x81, 0x01,                     //     INPUT (Cnst,Ary,Abs)
	0x09, 0x51,                     //     USAGE (Contact Identifier)
	0x75, 0x08,                     //     REPORT_SIZE (8)
	0x95, 0x01,                     //     REPORT_COUNT (1)
	0x81, 0x02,                     //     INPUT (Data,Var,Abs)
	0x05, 0x01,                     //     USAGE_PAGE (Generic Desktop)
	0x09, 0x30,                     //     USAGE (X)
	0x09, 0x31,                     //     USAGE (Y)
	0x16, 0x00, 0x00,               //     Logical Minimum(0)
	0x26, 0xFF, 0x7F,               //     Logical Maximum(32767)
	0x36, 0x00, 0x00,               //     Physical Minimum(0)
	0x46, 0xFF, 0x7F,               //     Physical Maximum(32767)
	0x66, 0x00, 0x00,               //     Unit(None)
	0x75, 0x10,                     //     REPORT_SIZE(16)
	0x95, 0x02,                     //     REPORT_COUNT(2)
	0x81, 0x62,                     //     Input(Data, Variable, Absolute, No Preferred, Null State)
	0xc0,                           //   END_COLLECTION
	0x05, 0x0d,                     //   USAGE_PAGE (Digitizers)
	0x09, 0x54,                     //   USAGE (Contact count)
	0x25, 0x7f,                     //   LOGICAL_MAXIMUM (127) 
	0x95, 0x01,                     //   REPORT_COUNT (1)
	0x75, 0x08,                     //   REPORT_SIZE (8)    
	0x81, 0x02,                     //   INPUT (Data,Var,Abs)
	0x85, MAX_COUNT_REPORT_ID,      //   REPORT_ID (Feature)              
	0x09, 0x55,                     //   USAGE(Contact Count Maximum)
	0x25, 0x0a,                     //   LOGICAL_MAXIMUM (10) 
	0x95, 0x01,                     //   REPORT_COUNT (1)
	0x75, 0x08,                     //   REPORT_SIZE (8)
	0xb1, 0x02,                     //   FEATURE (Data,Var,Abs)
	0xc0,                           // END_COLLECTION
};

#define MAX_HID_REPORT_SIZE sizeof(HIDINJECTOR_INPUT_REPORT)

//
// This is the default HID descriptor returned by the mini driver
// in response to IOCTL_HID_GET_DEVICE_DESCRIPTOR. The size
// of report descriptor is currently the size of G_DefaultReportDescriptor.
//

HID_DESCRIPTOR              G_DefaultHidDescriptor = {
    0x09,   // length of HID descriptor
    0x21,   // descriptor type == HID  0x21
    0x0100, // hid spec release
    0x00,   // country code == Not Specified
    0x01,   // number of HID class descriptors
    {                                       //DescriptorList[0]
        0x22,                               //report descriptor type 0x22
        sizeof(G_DefaultReportDescriptor)   //total length of report descriptor
    }
};


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HIDINJECTOR_EvtDeviceAdd)
#pragma alloc_text(PAGE, HIDINJECTOR_EvtDeviceSelfManagedIoInit)
#pragma alloc_text(PAGE, HIDINJECTOR_EvtDeviceSelfManagedIoCleanup)
#pragma alloc_text(PAGE, HIDINJECTOR_VhfInitialize)
#pragma alloc_text(PAGE, HIDINJECTOR_VhfSubmitReadReport)
#pragma alloc_text(PAGE, HIDINJECTOR_GetFeatureReport)
#pragma alloc_text(PAGE, HIDINJECTOR_VhfAsyncOperationGetFeature)

#endif // ALLOC_PRAGMA

NTSTATUS
DriverEntry(
    _In_  PDRIVER_OBJECT    DriverObject,
    _In_  PUNICODE_STRING   RegistryPath
    )
/*++

Routine Description:
    DriverEntry initializes the driver and is the first routine called by the
    system after the driver is loaded. DriverEntry specifies the other entry
    points in the function driver, such as EvtDevice and DriverUnload.

Parameters Description:

    DriverObject - represents the instance of the function driver that is loaded
    into memory. DriverEntry must initialize members of DriverObject before it
    returns to the caller. DriverObject is allocated by the system before the
    driver is loaded, and it is released by the system after the system unloads
    the function driver from memory.

    RegistryPath - represents the driver specific path in the Registry.
    The function driver can use the path to store driver related data between
    reboots. The path does not store hardware instance specific data.

Return Value:

    STATUS_SUCCESS, or another status value for which NT_SUCCESS(status) equals
                    TRUE if successful,

    STATUS_UNSUCCESSFUL, or another status for which NT_SUCCESS(status) equals
                    FALSE otherwise.

--*/
{
    WDF_DRIVER_CONFIG       config;
    NTSTATUS                status;

    KdPrint(("DriverEntry for HIDINJECTOR\n"));

    WDF_DRIVER_CONFIG_INIT(&config, HIDINJECTOR_EvtDeviceAdd);

    status = WdfDriverCreate(DriverObject,
                            RegistryPath,
                            WDF_NO_OBJECT_ATTRIBUTES,
                            &config,
                            WDF_NO_HANDLE);
    if (!NT_SUCCESS(status)) {
        KdPrint(("Error: WdfDriverCreate failed 0x%x\n", status));
        return status;
    }

    return status;
}

NTSTATUS
HIDINJECTOR_EvtDeviceAdd(
    _In_  WDFDRIVER         Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
/*++
Routine Description:C

    HIDINJECTOR_EvtDeviceAdd is called by the framework in response to AddDevice
    call from the PnP manager. We create and initialize a device object to
    represent a new instance of the device.

Arguments:

    Driver - Handle to a framework driver object created in DriverEntry

    DeviceInit - Pointer to a framework-allocated WDFDEVICE_INIT structure.

Return Value:

    NTSTATUS

--*/
{
    NTSTATUS                status;
    WDF_OBJECT_ATTRIBUTES   deviceAttributes;
    WDFDEVICE               device;
    PHID_DEVICE_CONTEXT         deviceContext;
    UNREFERENCED_PARAMETER  (Driver);
	WDF_PNPPOWER_EVENT_CALLBACKS    wdfPnpPowerCallbacks;


	PAGED_CODE();

    KdPrint(("Enter HIDINJECTOR_EvtDeviceAdd\n"));

    //
    // Mark ourselves as a filter, which also relinquishes power policy ownership
    //
    WdfFdoInitSetFilter(DeviceInit);

	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&wdfPnpPowerCallbacks);
	wdfPnpPowerCallbacks.EvtDeviceSelfManagedIoInit = HIDINJECTOR_EvtDeviceSelfManagedIoInit;
	wdfPnpPowerCallbacks.EvtDeviceSelfManagedIoCleanup = HIDINJECTOR_EvtDeviceSelfManagedIoCleanup;
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &wdfPnpPowerCallbacks);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
                            &deviceAttributes,
                            HID_DEVICE_CONTEXT);

    status = WdfDeviceCreate(&DeviceInit,
                            &deviceAttributes,
                            &device);
    if (!NT_SUCCESS(status)) {
        KdPrint(("Error: WdfDeviceCreate failed 0x%x\n", status));
        return status;
    }

    deviceContext = GetHidDeviceContext(device);
    deviceContext->Device       = device;

	//
	// Use default "HID Descriptor" (hardcoded).
	//
	deviceContext->HidDescriptor = G_DefaultHidDescriptor;
	deviceContext->ReportDescriptor = G_DefaultReportDescriptor;

	//
	// Initialize VHF.  This will talk to HIDCLASS for us.
	//
	status = HIDINJECTOR_VhfInitialize(device);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to init VHF\n"));
		return status;
	}

	//
	// Create a new queue to handle IOCTLs that will be forwarded to us from
	// the rawPDO. 
	//
	status = RawQueueCreate(
		device,
		&deviceContext->RawPdoQueue);
	if (!NT_SUCCESS(status)) {
		KdPrint(("QueueCreate failed 0x%x\n", status));
		return status;
	}

	if (NT_SUCCESS(status)) {
		status = HIDINJECTOR_CreateRawPdo(device);
		if (!NT_SUCCESS(status)) {
			KdPrint(("Failed to create Raw Pdo and/or queue\n"));
			return status;
		}
	}

	return status;
}

NTSTATUS
RawQueueCreate(
	_In_ WDFDEVICE										Device,
	_Out_ WDFQUEUE										*Queue
	)
{
	NTSTATUS                status;
	WDF_IO_QUEUE_CONFIG     queueConfig;
	WDF_OBJECT_ATTRIBUTES   queueAttributes;
	WDFQUEUE                queue;
	PQUEUE_CONTEXT          queueContext;

	WDF_IO_QUEUE_CONFIG_INIT(
		&queueConfig,
		WdfIoQueueDispatchParallel);

	queueConfig.EvtIoWrite = HIDINJECTOR_EvtIoWriteFromRawPdo;

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(
		&queueAttributes,
		QUEUE_CONTEXT);

	status = WdfIoQueueCreate(
		Device,
		&queueConfig,
		&queueAttributes,
		&queue);

	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfIoQueueCreate failed 0x%x\n", status));
		return status;
	}

	queueContext = GetQueueContext(queue);
	queueContext->Queue = queue;
	queueContext->DeviceContext = GetHidDeviceContext(Device);

	*Queue = queue;
	return status;
}


VOID HIDINJECTOR_EvtIoWriteFromRawPdo(
	_In_ WDFQUEUE   Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t     Length
	)
{
	PQUEUE_CONTEXT queueContext;
	WDFMEMORY memory;
	void *pvoid;
	size_t length;
	NTSTATUS status;

	queueContext = GetQueueContext(Queue);

	if (Length < sizeof(HIDINJECTOR_INPUT_REPORT)) {
		KdPrint(("WriteReport: invalid input buffer. size %d, expect %d\n",
			Length, sizeof(HIDINJECTOR_INPUT_REPORT)));
		WdfRequestComplete(Request, STATUS_INVALID_BUFFER_SIZE);
		return;
	}

	status = WdfRequestRetrieveInputMemory(Request, &memory);
	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfRequestRetrieveInputMemory failed %x\n",
			status));
		WdfRequestComplete(Request, status);
		return;
	}
	pvoid = WdfMemoryGetBuffer(memory, &length);
	if (pvoid == NULL) {
		WdfRequestComplete(Request, STATUS_INVALID_BUFFER_SIZE);
		return;
	}


	// 
	// Complete the input IRP if we have one
	//
	HIDINJECTOR_VhfSubmitReadReport(queueContext->DeviceContext,
		pvoid,
		Length);

	//
	// set status and information
	//
	WdfRequestCompleteWithInformation(Request, STATUS_SUCCESS, Length);
}

NTSTATUS
HIDINJECTOR_EvtDeviceSelfManagedIoInit(
	WDFDEVICE WdfDevice
	)
{
	PHID_DEVICE_CONTEXT	deviceContext;
	NTSTATUS        status;

	PAGED_CODE();

	deviceContext = GetHidDeviceContext(WdfDevice);

	status = VhfStart(deviceContext->VhfHandle);

	if (!NT_SUCCESS(status)) {
		// TODO; seach all KDPrint Calls to make sure parameters match format specifiers
		KdPrint(("VhfStart failed %d\n", status));
	}

	return status;
}

VOID
HIDINJECTOR_EvtDeviceSelfManagedIoCleanup(
	WDFDEVICE WdfDevice
	)
{
	PHID_DEVICE_CONTEXT	deviceContext;

	PAGED_CODE();

	deviceContext = GetHidDeviceContext(WdfDevice);

	VhfDelete(deviceContext->VhfHandle, TRUE);

	return;
}

NTSTATUS
HIDINJECTOR_VhfInitialize(
	WDFDEVICE WdfDevice
	)
{
	PHID_DEVICE_CONTEXT	deviceContext;
	NTSTATUS			status;
	VHF_CONFIG			vhfConfig;

	PAGED_CODE();

	deviceContext = GetHidDeviceContext(WdfDevice);

	VHF_CONFIG_INIT(&vhfConfig,
		WdfDeviceWdmGetDeviceObject(WdfDevice),
		deviceContext->HidDescriptor.DescriptorList[0].wReportLength,
		deviceContext->ReportDescriptor);

	vhfConfig.VhfClientContext = deviceContext;
	vhfConfig.OperationContextSize = 11;
	vhfConfig.EvtVhfAsyncOperationGetFeature = HIDINJECTOR_VhfAsyncOperationGetFeature;

	status = VhfCreate(&vhfConfig, &deviceContext->VhfHandle);

	if (!NT_SUCCESS(status)) {
		KdPrint(("VhfCreate failed %d", status));
		goto _exit;
	}

_exit:
	return status;
}

VOID
HIDINJECTOR_VhfSubmitReadReport(
	_In_ PHID_DEVICE_CONTEXT DeviceContext,
	_In_ PUCHAR Report,
	_In_ size_t ReportSize
	)
{
	NTSTATUS        status;
	HID_XFER_PACKET transferPacket;

	transferPacket.reportId = *Report;
	transferPacket.reportBufferLen = ReportSize;
	transferPacket.reportBuffer = Report;

	status = VhfReadReportSubmit(DeviceContext->VhfHandle, &transferPacket);

	if (!NT_SUCCESS(status)) {
		KdPrint(("VhfReadReportSubmit failed %d", status));
	}
}

NTSTATUS
HIDINJECTOR_GetFeatureReport(
		_In_ PHID_DEVICE_CONTEXT DeviceContext,
		_In_ PHID_XFER_PACKET    HidTransferPacket
		)
/*++
Routine Description:C

    HIDINJECTOR_GetFeatureReport handles a get feature report request

Arguments:

	DeviceContext - 	The device context.

	HidTransferPacket -	A pointer to a HID_XFER_PACKET structure. Contains information about a HID 
				Report and is used by the HID source driver and the HID class/mini driver 
				pair for I/O requests to get or set a report.
    
Return Value:

    VOID

--*/
{
	NTSTATUS        status= STATUS_SUCCESS;;

	UNREFERENCED_PARAMETER(DeviceContext);

	switch (HidTransferPacket->reportId)
	{
		case MAX_COUNT_REPORT_ID:
			{
				PHIDINJECTOR_MAX_COUNT_REPORT maxCountReport = (PHIDINJECTOR_MAX_COUNT_REPORT)HidTransferPacket->reportBuffer;

				if(HidTransferPacket->reportBufferLen < sizeof(HIDINJECTOR_MAX_COUNT_REPORT))
				{
					status = STATUS_BUFFER_TOO_SMALL;
					KdPrint(("HIDINJECTOR_GetFeatureReport Error: reportBufferLen too small"));
					goto GetFeatureReportExit;
				};
				maxCountReport->ReportID = MAX_COUNT_REPORT_ID;
				maxCountReport->MaxCount = TOUCH_MAX_FINGER;
				break;
			};

		default:
			{
				KdPrint(("HIDINJECTOR_GetFeatureReport Error: parameter %Xh is invalid",(*(PUCHAR)HidTransferPacket->reportBuffer)));
				status = STATUS_INVALID_PARAMETER;
				goto GetFeatureReportExit;
			};
	}

GetFeatureReportExit:
	return status; 
}

void HIDINJECTOR_VhfAsyncOperationGetFeature(
  _In_     PVOID              VhfClientContext,
  _In_     VHFOPERATIONHANDLE VhfOperationHandle,
  _In_opt_ PVOID              VhfOperationContext,
  _In_     PHID_XFER_PACKET   HidTransferPacket
)
/*++
Routine Description:C

    HIDINJECTOR_VhfAsyncOperationGetFeature is used to submit a report to virtual hid framework

Arguments:

	VhfClientContext - 	An opaque pointer to a HID source driver-defined buffer that the driver 
				passed in the VHF_CONFIG structure supplied to VhfCreate to create the 
				virtual HID device.

	VhfOperationHandle - 	An opaque handle that uniquely identifies this asynchronous operation.

	VhfOperationContext - 	Pointer to a buffer that can be used by the HID source driver for servicing 
				the operation. Size of the buffer is specified by the HID source driver 
				in the VHF_CONFIG structure supplied to VhfCreate.

	HidTransferPacket -	A pointer to a HID_XFER_PACKET structure. Contains information about a HID 
				Report and is used by the HID source driver and the HID class/mini driver 
				pair for I/O requests to get or set a report.t
    
Return Value:

    VOID

--*/
{
	NTSTATUS        status;
	PHID_DEVICE_CONTEXT context;
	context = (PHID_DEVICE_CONTEXT)VhfClientContext;

	UNREFERENCED_PARAMETER(VhfOperationContext);

	status = HIDINJECTOR_GetFeatureReport(context, HidTransferPacket);

	VhfAsyncOperationComplete(VhfOperationHandle, status);

	return;
}

