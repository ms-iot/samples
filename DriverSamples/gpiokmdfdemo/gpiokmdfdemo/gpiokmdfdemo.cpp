// Copyright (c) Microsoft. All rights reserved.


// Module Name:
//
//     driver.cpp
//
// Abstract:
//
//     GPIO KMDF Demo Driver. 
//     Carries out operations on behalf of the usermode command shell program.
//

#include <ntddk.h>
#include <wdf.h>
#define RESHUB_USE_HELPER_ROUTINES
#include <reshub.h>
#include <gpio.h>

#include "gpiokmdfdemo.h"

typedef struct _DEVICE_CONTEXT
{
	ULONG           ResourceCount;
	LARGE_INTEGER   ConnectionIds [GPOT_MAX_RESOURCES];
	WDFIOTARGET     IoTargets     [GPOT_MAX_RESOURCES];
	GPOT_OPEN_STATE OpenState     [GPOT_MAX_RESOURCES];

} DEVICE_CONTEXT, *PDEVICE_CONTEXT;


WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, DeviceGetContext)

extern "C"
{
	DRIVER_INITIALIZE DriverEntry;
	EVT_WDF_DRIVER_UNLOAD OnDriverUnload;
	EVT_WDF_DRIVER_DEVICE_ADD OnDeviceAdd;
	EVT_WDF_DEVICE_PREPARE_HARDWARE OnPrepareHardware;
	_IRQL_requires_max_(PASSIVE_LEVEL) EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL OnDeviceIoControl;
	EVT_WDF_FILE_CLEANUP OnFileCleanup;
	EVT_WDF_DEVICE_PREPARE_HARDWARE OnPrepareHardware;
}


#pragma code_seg(push, "INIT")

NTSTATUS
DriverEntry(_In_ PDRIVER_OBJECT  DriverObject, 
			_In_ PUNICODE_STRING RegistryPath)
{
	NTSTATUS          status;
	WDF_DRIVER_CONFIG config;

	WDF_DRIVER_CONFIG_INIT(&config, OnDeviceAdd);

	status = WdfDriverCreate(DriverObject,
							 RegistryPath,
							 WDF_NO_OBJECT_ATTRIBUTES,
							 &config,
							 WDF_NO_HANDLE);

	return status;
}

#pragma code_seg(pop) // INIT

#pragma code_seg(push, "PAGE")

NTSTATUS
OnDeviceAdd(_In_    WDFDRIVER       Driver,
			_Inout_ PWDFDEVICE_INIT DeviceInit)
{
	NTSTATUS                     status;
	WDF_PNPPOWER_EVENT_CALLBACKS callbacks;
	WDF_FILEOBJECT_CONFIG        fileConfig;
	WDF_OBJECT_ATTRIBUTES        deviceAttributes;
	PDEVICE_CONTEXT              deviceContext;
	WDFDEVICE                    device;
	WDF_IO_QUEUE_CONFIG          ioqConfig;
	WDF_OBJECT_ATTRIBUTES        ioqAttributes;
	WDFQUEUE                     queue;
	
	DECLARE_CONST_UNICODE_STRING(symbolicLink, L"\\DosDevices\\" GPOT_DEVICE_NAME);

	UNREFERENCED_PARAMETER(Driver);

	PAGED_CODE();

	WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, DEVICE_CONTEXT);

	deviceAttributes.SynchronizationScope = WdfSynchronizationScopeDevice;
	deviceAttributes.ExecutionLevel       = WdfExecutionLevelPassive;

	//
	// Set up PnP callbacks
	//
	WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&callbacks);
	callbacks.EvtDevicePrepareHardware = OnPrepareHardware;

	//
	// Register the PnP callbacks with the framework.
	//
	WdfDeviceInitSetPnpPowerEventCallbacks(DeviceInit, &callbacks);

	//
	// Initialize file object handling
	//
	WDF_FILEOBJECT_CONFIG_INIT(&fileConfig,
							   WDF_NO_EVENT_CALLBACK,      // OnFileCreate,
							   WDF_NO_EVENT_CALLBACK,      // OnFileClose
							   OnFileCleanup);

	WdfDeviceInitSetFileObjectConfig(DeviceInit, &fileConfig, WDF_NO_OBJECT_ATTRIBUTES);

	status = WdfDeviceCreate(&DeviceInit, &deviceAttributes, &device);

	if (!NT_SUCCESS(status))
	{
		goto end;
	}

	//
	// Initialize the device context
	//
	deviceContext = DeviceGetContext(device);
	RtlZeroMemory(deviceContext, sizeof(*deviceContext));

	//
	// Set up an queue to handle DeviceIoControl requests
	//
	WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&ioqConfig, WdfIoQueueDispatchSequential);

	ioqConfig.EvtIoDeviceControl = OnDeviceIoControl;

	WDF_OBJECT_ATTRIBUTES_INIT(&ioqAttributes);
	ioqAttributes.ExecutionLevel = WdfExecutionLevelPassive;

	status = WdfIoQueueCreate(device,
							  &ioqConfig,
							  &ioqAttributes,
							  &queue);

	if (!NT_SUCCESS(status))
	{
		goto end;
	}

	status = WdfDeviceCreateSymbolicLink(device, &symbolicLink);

end:

	return status;
}

static
NTSTATUS
OpenConnection(_In_ WDFDEVICE FxDevice,
			   _In_ ULONG Index,
			   _In_ bool Output)
{
	NTSTATUS                  status;
	PDEVICE_CONTEXT           context;
	WDF_OBJECT_ATTRIBUTES     attributes;
	WDF_IO_TARGET_OPEN_PARAMS openParams;

	DECLARE_UNICODE_STRING_SIZE(path, RESOURCE_HUB_PATH_SIZE);

	PAGED_CODE();

	context = DeviceGetContext(FxDevice);

	WDF_OBJECT_ATTRIBUTES_INIT(&attributes);
	attributes.ParentObject = FxDevice;

	status = RESOURCE_HUB_CREATE_PATH_FROM_ID(&path,
											  context->ConnectionIds[Index].LowPart,
											  context->ConnectionIds[Index].HighPart);

	if (!NT_SUCCESS(status))
	{
		goto end;
	}

	WDF_IO_TARGET_OPEN_PARAMS_INIT_OPEN_BY_NAME(&openParams,
												&path,
												Output ? GENERIC_WRITE : GENERIC_READ);

	if (context->IoTargets[Index] == WDF_NO_HANDLE)
	{
		status = WdfIoTargetCreate(FxDevice,
								   &attributes,
								   &context->IoTargets[Index]);

		if (!NT_SUCCESS(status))
		{
			goto end;
		}
	}
	else
	{
		WdfIoTargetClose(context->IoTargets[Index]);
	}

	status = WdfIoTargetOpen(context->IoTargets[Index], &openParams);
	if (NT_SUCCESS(status))
	{
		context->OpenState[Index] = Output ? OpenStateOutput : OpenStateInput;
	}
	else
	{
		context->OpenState[Index] = OpenStateNotOpened;
	}

end:
	return status;
}

static
NTSTATUS
GetIndexFromRequest(_In_ PDEVICE_CONTEXT Context, 
					_In_ WDFREQUEST FxRequest,
					_Out_ ULONG *Index)
{
	NTSTATUS status;
	ULONG    *buffer;

	PAGED_CODE();

	status = WdfRequestRetrieveInputBuffer(FxRequest,
										   sizeof(*buffer),
										   reinterpret_cast<PVOID *>(&buffer),
										   NULL);

	if (!NT_SUCCESS(status))
	{
		goto end;
	}

	if (*buffer >= Context->ResourceCount)
	{
		status = STATUS_INVALID_PARAMETER;
		goto end;
	}

	*Index = *buffer;

end:
	return status;
}

static
NTSTATUS
WritePin(_In_ PDEVICE_CONTEXT Context,
		 _In_ ULONG Index,
		 _In_ bool High)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDF_MEMORY_DESCRIPTOR  memDescriptor;
	UCHAR data = High ? 1 : 0;

	PAGED_CODE();

	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(
		&memDescriptor,
		(PVOID)&data,
		sizeof(data));

	status = WdfIoTargetSendIoctlSynchronously(
		Context->IoTargets[Index],
		NULL,
		IOCTL_GPIO_WRITE_PINS,
		&memDescriptor,
		nullptr,
		nullptr,
		nullptr);

	return status;
}

/*
static
NTSTATUS
ReadPin(_In_ PDEVICE_CONTEXT Context,
		_In_ ULONG Index,
		_Out_ BYTE *Data)
{
	NTSTATUS               status = STATUS_SUCCESS;
	WDF_MEMORY_DESCRIPTOR  memDescriptor;
	BYTE                   data = 0;

	PAGED_CODE();

	WDF_MEMORY_DESCRIPTOR_INIT_BUFFER(&memDescriptor,
									  (PVOID)&data,
									  sizeof(data));

	status = WdfIoTargetSendIoctlSynchronously(Context->IoTargets[Index],
											   NULL,
											   IOCTL_GPIO_READ_PINS,
											   nullptr,
											   &memDescriptor,
											   nullptr,
											   nullptr);

	if (NT_SUCCESS(status))
	{
		*Data = data;
	}

	return status;
}
*/

_Use_decl_annotations_
VOID
OnDeviceIoControl(WDFQUEUE   FxQueue,
				  WDFREQUEST FxRequest,
				  size_t     OutputBufferLength,
				  size_t     InputBufferLength,
				  ULONG      IoControlCode)
{
	NTSTATUS        status;
	ULONG           index;
	ULONG_PTR       lengthReturned = 0;
	PDEVICE_CONTEXT context;
	WDFDEVICE       fxDevice;

	UNREFERENCED_PARAMETER(OutputBufferLength);
	UNREFERENCED_PARAMETER(InputBufferLength);

	PAGED_CODE();

	fxDevice = WdfIoQueueGetDevice(FxQueue);
	context  = DeviceGetContext(fxDevice);

	switch (IoControlCode)
	{
	//case IOCTL_GPOT_OPEN_INPUT:
	case IOCTL_GPOT_OPEN_OUTPUT:

		status = GetIndexFromRequest(context, FxRequest, &index);
		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		status = OpenConnection(fxDevice,
								index,
								IoControlCode == IOCTL_GPOT_OPEN_OUTPUT);
		break;

	case IOCTL_GPOT_CLOSE:

		status = GetIndexFromRequest(context, FxRequest, &index);
		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		if (context->IoTargets[index] != WDF_NO_HANDLE)
		{
			WdfIoTargetClose(context->IoTargets[index]);
			context->OpenState[index] = OpenStateNotOpened;
		}

		break;

	case IOCTL_GPOT_WRITE_LOW:
	case IOCTL_GPOT_WRITE_HIGH:

		status = GetIndexFromRequest(context, FxRequest, &index);
		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		status = WritePin(context,
						  index,
						  IoControlCode == IOCTL_GPOT_WRITE_HIGH);

		break;

	/*	
	case IOCTL_GPOT_READ:
	{
		BYTE *outputBuffer;

		status = GetIndexFromRequest(context, FxRequest, &index);
		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		status = WdfRequestRetrieveOutputBuffer(FxRequest,
												sizeof(*outputBuffer),
												reinterpret_cast<PVOID *>(&outputBuffer),
												nullptr);

		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		status = ReadPin(context, index, outputBuffer);
		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		lengthReturned = sizeof(*outputBuffer);

		break;
	}

	case IOCTL_GPOT_GET_RESOURCE_COUNT:
	{
		ULONG *outputBuffer;

		status = WdfRequestRetrieveOutputBuffer(FxRequest,
												sizeof(*outputBuffer),
												reinterpret_cast<PVOID *>(&outputBuffer),
												nullptr);

		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		*outputBuffer  = context->ResourceCount;
		lengthReturned = sizeof(*outputBuffer);

		break;
	}

	case IOCTL_GPOT_GET_STATUS:
	{
		// expecting an array with ResourceCount elements
		GPOT_STATUS_ENTRY *outputBuffer;

		const ULONG bufferSize = context->ResourceCount * sizeof(*outputBuffer);

		status = WdfRequestRetrieveOutputBuffer(FxRequest,
												bufferSize,
												reinterpret_cast<PVOID *>(&outputBuffer),
												nullptr);

		if (!NT_SUCCESS(status))
		{
			goto end;
		}

		for (ULONG i = 0; i < context->ResourceCount; ++i)
		{
			outputBuffer[i].OpenState = context->OpenState[i];
			outputBuffer[i].ConnectionId = context->ConnectionIds[i];
		}

		lengthReturned = bufferSize;

		break;
	}
	*/
	default:
		// unrecognized IOCTL
		status = STATUS_NOT_SUPPORTED;
		break;
	}

end:

	WdfRequestCompleteWithInformation(FxRequest, status, lengthReturned);
}

_Use_decl_annotations_
NTSTATUS
OnPrepareHardware(WDFDEVICE     FxDevice,
				  WDFCMRESLIST  FxResourcesRaw,
				  WDFCMRESLIST  FxResourcesTranslated)
{
	NTSTATUS		status;
	PDEVICE_CONTEXT context;
	ULONG			resourceCount;

	UNREFERENCED_PARAMETER(FxResourcesRaw);

	PAGED_CODE();

	context = DeviceGetContext(FxDevice);
	resourceCount = WdfCmResourceListGetCount(FxResourcesTranslated);

	//
	// Save each of the memory resources and map them into kernel
	// virtual memory space
	//
	for (ULONG i = 0; i < resourceCount; ++i)
	{
		PCM_PARTIAL_RESOURCE_DESCRIPTOR res = WdfCmResourceListGetDescriptor(FxResourcesTranslated, i);
		switch (res->Type)
		{

		case CmResourceTypeConnection:

			//
			//  Check against expected connection type
			//
			if (context->ResourceCount < ARRAYSIZE(context->ConnectionIds))
			{

				if ((res->u.Connection.Class == CM_RESOURCE_CONNECTION_CLASS_GPIO) &&
					(res->u.Connection.Type  == CM_RESOURCE_CONNECTION_TYPE_GPIO_IO))
				{

					context->ConnectionIds[context->ResourceCount].LowPart = res->u.Connection.IdLowPart;
					context->ConnectionIds[context->ResourceCount].HighPart = res->u.Connection.IdHighPart;

					++context->ResourceCount;
				}
			}

			break;

		default:

			// Ignore all other resource types.
			break;
		}
	}

	status = STATUS_SUCCESS;

	return status;
}

_Use_decl_annotations_
VOID
OnFileCleanup(WDFFILEOBJECT FxFile)
{
	WDFDEVICE       fxDevice;
	PDEVICE_CONTEXT context;

	PAGED_CODE();

	fxDevice = WdfFileObjectGetDevice(FxFile);
	context  = DeviceGetContext(fxDevice);

	// close all of the IO targets
	for (ULONG i = 0; i < ARRAYSIZE(context->IoTargets); ++i)
	{
		if (context->IoTargets[i] != WDF_NO_HANDLE)
		{
			WdfIoTargetClose(context->IoTargets[i]);
			WdfObjectDelete(context->IoTargets[i]);
			context->IoTargets[i] = WDF_NO_HANDLE;
			context->OpenState[i] = GPOT_OPEN_STATE::OpenStateNotOpened;
		}
	}
}

#pragma code_seg(pop) // PAGE
