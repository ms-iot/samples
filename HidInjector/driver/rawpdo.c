/*--

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.


Module Name:

    RawPdo.c

Abstract: This module have the code enumerate a raw PDO for every device
          the filter attaches to so that it can provide a direct
          sideband communication with the usermode application.
		            
Environment:

    Kernel mode only.

--*/

#include "HidInjectorKd.h"
#include "Ntstrsafe.h"

ULONG InstanceNo = 0;

#define MAX_ID_LEN 128

NTSTATUS
HIDINJECTOR_CreateRawPdo(
    WDFDEVICE       Device
    )
/*++

Routine Description:

    This routine creates and initialize a PDO.

Arguments:

Return Value:

    NT Status code.

--*/
{   
    NTSTATUS                    status;
    PWDFDEVICE_INIT             pDeviceInit = NULL;
    PRAWPDO_DEVICE_CONTEXT           pdoData = NULL;
    WDFDEVICE                   hChild = NULL;
    WDF_OBJECT_ATTRIBUTES       pdoAttributes;
    WDF_DEVICE_PNP_CAPABILITIES pnpCaps;
    WDF_IO_QUEUE_CONFIG         ioQueueConfig;
    WDFQUEUE                    queue;
    WDF_DEVICE_STATE            deviceState;
	PHID_DEVICE_CONTEXT             devExt;
	WDF_PNPPOWER_EVENT_CALLBACKS  pnpPowerCallbacks;
	DECLARE_CONST_UNICODE_STRING(deviceId,HIDINJECTOR_DEVICE_ID );
    DECLARE_CONST_UNICODE_STRING(deviceLocation,L"HID Injector\0" );
	DECLARE_CONST_UNICODE_STRING(SDDL_MY_PERMISSIONS, L"D:P(A;; GA;;; SY)(A;; GA;;; BA)(A;; GA;;; WD)");
    DECLARE_UNICODE_STRING_SIZE(buffer, MAX_ID_LEN);

    KdPrint(("Entered HIDINJECTOR_CreateRawPdo\n"));

    //
    // Allocate a WDFDEVICE_INIT structure and set the properties
    // so that we can create a device object for the child.
    //
    pDeviceInit = WdfPdoInitAllocate(Device);

    if (pDeviceInit == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

	//
	// Register for power callbacks so we can register the symbolic link
	//
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
	pnpPowerCallbacks.EvtDeviceSelfManagedIoInit = RAWPDO_EvtDeviceSelfManagedIoInit;
	WdfDeviceInitSetPnpPowerEventCallbacks(
		pDeviceInit,
		&pnpPowerCallbacks
		);

    //
    // Mark the device RAW so that the child device can be started
    // and accessed without requiring a function driver. Since we are
    // creating a RAW PDO, we must provide a class guid.
    //
    status = WdfPdoInitAssignRawDevice(pDeviceInit, &GUID_DEVCLASS_HIDINJECTOR);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Since keyboard is secure device, we must protect ourselves from random
    // users sending ioctls and creating trouble.
    //
    status = WdfDeviceInitAssignSDDLString(pDeviceInit,
                                           &SDDL_MY_PERMISSIONS);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Assign DeviceID - This will be reported to IRP_MN_QUERY_ID/BusQueryDeviceID
    //
    status = WdfPdoInitAssignDeviceID(pDeviceInit, &deviceId);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // We could be enumerating more than one children if the filter attaches
    // to multiple instances of keyboard, so we must provide a
    // BusQueryInstanceID. If we don't, system will throw CA bugcheck.
    //
    status =  RtlUnicodeStringPrintf(&buffer, L"%02d", InstanceNo);
	if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    status = WdfPdoInitAssignInstanceID(pDeviceInit, &buffer);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Provide a description about the device. This text is usually read from
    // the device. In the case of USB device, this text comes from the string
    // descriptor. This text is displayed momentarily by the PnP manager while
    // it's looking for a matching INF. If it finds one, it uses the Device
    // Description from the INF file to display in the device manager.
    // Since our device is raw device and we don't provide any hardware ID
    // to match with an INF, this text will be displayed in the device manager.
    //
    status = RtlUnicodeStringPrintf(&buffer,L"HID_Injector_%02d", InstanceNo );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }
	InstanceNo++;

    //
    // You can call WdfPdoInitAddDeviceText multiple times, adding device
    // text for multiple locales. When the system displays the text, it
    // chooses the text that matches the current locale, if available.
    // Otherwise it will use the string for the default locale.
    // The driver can specify the driver's default locale by calling
    // WdfPdoInitSetDefaultLocale.
    //
    status = WdfPdoInitAddDeviceText(pDeviceInit,
                                        &buffer,
                                        &deviceLocation,
                                        0x409
                                        );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    WdfPdoInitSetDefaultLocale(pDeviceInit, 0x409);
    
    //
    // Initialize the attributes to specify the size of PDO device extension.
    // All the state information private to the PDO will be tracked here.
    //
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&pdoAttributes, RAWPDO_DEVICE_CONTEXT);

    //
    // Set up our queue to allow forwarding of requests to the parent
    // This is done so that the cached Keyboard Attributes can be retrieved
    //
    WdfPdoInitAllowForwardingRequestToParent(pDeviceInit);

    status = WdfDeviceCreate(&pDeviceInit, &pdoAttributes, &hChild);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    //
    // Get the device context.
    //
    pdoData = GetRawPdoDeviceContext(hChild);

    pdoData->InstanceNo = InstanceNo;

    //
    // Get the parent queue we will be forwarding to
    //
    devExt = GetHidDeviceContext(Device);
    pdoData->ParentQueue = devExt->RawPdoQueue;

    //
    // Configure the default queue associated with the control device object
    // to be Serial so that request passed to EvtIoDeviceControl are serialized.
    // A default queue gets all the requests that are not
    // configure-fowarded using WdfDeviceConfigureRequestDispatching.
    //

    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioQueueConfig,
                                    WdfIoQueueDispatchSequential);

	ioQueueConfig.EvtIoWrite = HIDINJECTOR_EvtIoWriteForRawPdo;


    status = WdfIoQueueCreate(hChild,
                                        &ioQueueConfig,
                                        WDF_NO_OBJECT_ATTRIBUTES,
                                        &queue // pointer to default queue
                                        );
    if (!NT_SUCCESS(status)) {
        KdPrint( ("WdfIoQueueCreate failed 0x%x\n", status));
        goto Cleanup;
    }

    //
    // Set some properties for the child device.
    //
    WDF_DEVICE_PNP_CAPABILITIES_INIT(&pnpCaps);

    pnpCaps.Removable         = WdfTrue;
    pnpCaps.SurpriseRemovalOK = WdfTrue;
    pnpCaps.NoDisplayInUI     = WdfTrue;

    pnpCaps.Address  = InstanceNo;
    pnpCaps.UINumber = InstanceNo;

    WdfDeviceSetPnpCapabilities(hChild, &pnpCaps);

    //
    // TODO: In addition to setting NoDisplayInUI in DeviceCaps, we
    // have to do the following to hide the device. Following call
    // tells the framework to report the device state in
    // IRP_MN_QUERY_DEVICE_STATE request.
    //
    WDF_DEVICE_STATE_INIT(&deviceState);
    deviceState.DontDisplayInUI = WdfTrue;
    WdfDeviceSetDeviceState(hChild, &deviceState);

    //
    // Tell the Framework that this device will need an interface so that
    // application can find our device and talk to it.
    //
    status = WdfDeviceCreateDeviceInterface(
                 hChild,
                 &GUID_DEVINTERFACE_HIDINJECTOR,
                 NULL
             );

    if (!NT_SUCCESS (status)) {
        KdPrint( ("WdfDeviceCreateDeviceInterface failed 0x%x\n", status));
        goto Cleanup;
    }

    //
    // Add this device to the FDO's collection of children.
    // After the child device is added to the static collection successfully,
    // driver must call WdfPdoMarkMissing to get the device deleted. It
    // shouldn't delete the child device directly by calling WdfObjectDelete.
    //
    status = WdfFdoAddStaticChild(Device, hChild);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }
	devExt->RawPdo = hChild;

    //
    // pDeviceInit will be freed by WDF.
    //
    return STATUS_SUCCESS;

Cleanup:

    KdPrint(("HIDINJECTOR_CreateRawPdo failed %x\n", status));

    //
    // Call WdfDeviceInitFree if you encounter an error while initializing
    // a new framework device object. If you call WdfDeviceInitFree,
    // do not call WdfDeviceCreate.
    //
    if (pDeviceInit != NULL) {
        WdfDeviceInitFree(pDeviceInit);
    }

    if(hChild) {
        WdfObjectDelete(hChild);
    }

    return status;
}

NTSTATUS RAWPDO_EvtDeviceSelfManagedIoInit(
	_In_ WDFDEVICE Device
	)
{
	NTSTATUS status = STATUS_SUCCESS;

	//
	// Create a symbolic link so that user mode code to call into this device
	//
#define DOS_DEVICE_NAME  L"\\DosDevices\\HidInjector"
	DECLARE_CONST_UNICODE_STRING(dosDeviceName, DOS_DEVICE_NAME);

	status = WdfDeviceCreateSymbolicLink(
		Device,
		&dosDeviceName
		);
	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfDeviceCreateSymbolicLink failed\n"));
		return status;
	}

	return status;
}

VOID HIDINJECTOR_EvtIoWriteForRawPdo(
	_In_ WDFQUEUE   Queue,
	_In_ WDFREQUEST Request,
	_In_ size_t     Length
	)
{
	NTSTATUS status = STATUS_SUCCESS;
	PRAWPDO_DEVICE_CONTEXT pdoData;
	// WDF_REQUEST_FORWARD_OPTIONS forwardOptions;

	UNREFERENCED_PARAMETER(Length);
        
	WDFDEVICE parent = WdfIoQueueGetDevice(Queue);
	pdoData = GetRawPdoDeviceContext(parent);

	//
    // forward the request to manual queue
    //
    status = WdfRequestForwardToIoQueue(
                            Request,
                            pdoData->ParentQueue);
	if (!NT_SUCCESS(status)) {
		KdPrint(("WdfRequestForwardToIoQueue failed with 0x%x\n", status));
		WdfRequestComplete(Request, status);
	}

}