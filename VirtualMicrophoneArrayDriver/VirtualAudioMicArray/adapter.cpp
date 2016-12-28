/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    adapter.cpp

Abstract:

    Setup and miniport installation.  No resources are used by sysvad.
    This sample is to demonstrate how to develop a full featured audio miniport driver.

--*/

#pragma warning (disable : 4127)

//
// All the GUIDS for all the miniports end up in this object.
//
#define PUT_GUIDS_HERE

#include "VirtualAudioMicArray.h"

#include "simple.h"
#include "minipairs.h"

typedef void (*fnPcDriverUnload) (PDRIVER_OBJECT);
fnPcDriverUnload gPCDriverUnloadRoutine = NULL;
extern "C" DRIVER_UNLOAD DriverUnload;
PDRIVER_OBJECT gDriverObject = NULL;

//-----------------------------------------------------------------------------
// Referenced forward.
//-----------------------------------------------------------------------------

DRIVER_ADD_DEVICE AddDevice;

NTSTATUS
StartDevice
( 
    _In_  PDEVICE_OBJECT,      
    _In_  PIRP,                
    _In_  PRESOURCELIST        
); 

_Dispatch_type_(IRP_MJ_PNP)
DRIVER_DISPATCH PnpHandler;


//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

//=============================================================================
#pragma code_seg("PAGE")
extern "C"
void DriverUnload 
(
    _In_ PDRIVER_OBJECT DriverObject
)
/*++

Routine Description:

  Our driver unload routine. This just frees the WDF driver object.

Arguments:

  DriverObject - pointer to the driver object

Environment:

    PASSIVE_LEVEL

--*/
{
    PAGED_CODE(); 

    DPF(D_TERSE, ("[DriverUnload]"));

    if (DriverObject == NULL)
    {
        goto Done;
    }
    
    //
    // Invoke first the port unload.
    //
    if (gPCDriverUnloadRoutine != NULL)
    {
        gPCDriverUnloadRoutine(DriverObject);
    }

    //
    // Unload WDF driver object. 
    //
    if (WdfGetDriver() != NULL)
    {
        WdfDriverMiniportUnload(WdfGetDriver());
    }

Done:
    return;
}


#pragma code_seg("INIT")
extern "C" DRIVER_INITIALIZE DriverEntry;
extern "C" NTSTATUS
DriverEntry
( 
    _In_  PDRIVER_OBJECT          DriverObject,
    _In_  PUNICODE_STRING         RegistryPathName
)
{
/*++

Routine Description:

  Installable driver initialization entry point.
  This entry point is called directly by the I/O system.

  All audio adapter drivers can use this code without change.

Arguments:

  DriverObject - pointer to the driver object

  RegistryPath - pointer to a unicode string representing the path,
                   to driver-specific key in the registry.

Return Value:

  STATUS_SUCCESS if successful,
  STATUS_UNSUCCESSFUL otherwise.

--*/
    NTSTATUS                    ntStatus;
    WDF_DRIVER_CONFIG           config;

    DPF(D_TERSE, ("[DriverEntry]"));
    
    gDriverObject = DriverObject;


    WDF_DRIVER_CONFIG_INIT(&config, WDF_NO_EVENT_CALLBACK);
    //
    // Set WdfDriverInitNoDispatchOverride flag to tell the framework
    // not to provide dispatch routines for the driver. In other words,
    // the framework must not intercept IRPs that the I/O manager has
    // directed to the driver. In this case, they will be handled by Audio
    // port driver.
    //
    config.DriverInitFlags |= WdfDriverInitNoDispatchOverride;
    config.DriverPoolTag    = MINADAPTER_POOLTAG;

    ntStatus = WdfDriverCreate(DriverObject,
                               RegistryPathName,
                               WDF_NO_OBJECT_ATTRIBUTES,
                               &config,
                               WDF_NO_HANDLE);
    IF_FAILED_ACTION_JUMP(
        ntStatus,
        DPF(D_ERROR, ("WdfDriverCreate failed, 0x%x", ntStatus)),
        Done);

    //
    // Tell the class driver to initialize the driver.
    //
    ntStatus =  PcInitializeAdapterDriver(DriverObject,
                                          RegistryPathName,
                                          (PDRIVER_ADD_DEVICE)AddDevice);
    IF_FAILED_ACTION_JUMP(
        ntStatus,
        DPF(D_ERROR, ("PcInitializeAdapterDriver failed, 0x%x", ntStatus)),
        Done);

    //
    // To intercept stop/remove/surprise-remove.
    //
    DriverObject->MajorFunction[IRP_MJ_PNP] = PnpHandler;

    //
    // Hook the port class unload function
    //
    gPCDriverUnloadRoutine = DriverObject->DriverUnload;
    DriverObject->DriverUnload = DriverUnload;

    //
    // All done.
    //
    ntStatus = STATUS_SUCCESS;
    
Done:

    if (!NT_SUCCESS(ntStatus))
    {
        if (WdfGetDriver() != NULL)
        {
            WdfDriverMiniportUnload(WdfGetDriver());
        }
    }
    
    return ntStatus;
} // DriverEntry

#pragma code_seg()
// disable prefast warning 28152 because 
// DO_DEVICE_INITIALIZING is cleared in PcAddAdapterDevice
#pragma warning(disable:28152)
#pragma code_seg("PAGE")
//=============================================================================
NTSTATUS AddDevice
( 
    _In_  PDRIVER_OBJECT    DriverObject,
    _In_  PDEVICE_OBJECT    PhysicalDeviceObject 
)
/*++

Routine Description:

  The Plug & Play subsystem is handing us a brand new PDO, for which we
  (by means of INF registration) have been asked to provide a driver.

  We need to determine if we need to be in the driver stack for the device.
  Create a function device object to attach to the stack
  Initialize that device object
  Return status success.

  All audio adapter drivers can use this code without change.

Arguments:

  DriverObject - pointer to a driver object

  PhysicalDeviceObject -  pointer to a device object created by the
                            underlying bus driver.

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    NTSTATUS        ntStatus;
    ULONG           maxObjects;

    DPF(D_TERSE, ("[AddDevice]"));

    maxObjects = g_MaxMiniports;
    


    // Tell the class driver to add the device.
    //
    ntStatus = 
        PcAddAdapterDevice
        ( 
            DriverObject,
            PhysicalDeviceObject,
            PCPFNSTARTDEVICE(StartDevice),
            maxObjects,
            0
        );


    return ntStatus;
} // AddDevice

#pragma code_seg()
NTSTATUS
_IRQL_requires_max_(DISPATCH_LEVEL)
PowerControlCallback
(
    _In_        LPCGUID PowerControlCode,
    _In_opt_    PVOID   InBuffer,
    _In_        SIZE_T  InBufferSize,
    _Out_writes_bytes_to_(OutBufferSize, *BytesReturned) PVOID OutBuffer,
    _In_        SIZE_T  OutBufferSize,
    _Out_opt_   PSIZE_T BytesReturned,
    _In_opt_    PVOID   Context
)
{
    UNREFERENCED_PARAMETER(PowerControlCode);
    UNREFERENCED_PARAMETER(BytesReturned);
    UNREFERENCED_PARAMETER(InBuffer);
    UNREFERENCED_PARAMETER(OutBuffer);
    UNREFERENCED_PARAMETER(OutBufferSize);
    UNREFERENCED_PARAMETER(InBufferSize);
    UNREFERENCED_PARAMETER(Context);
    
    return STATUS_NOT_IMPLEMENTED;
}


#pragma code_seg("PAGE")
NTSTATUS 
InstallEndpointCaptureFilters(
    _In_ PDEVICE_OBJECT     _pDeviceObject, 
    _In_ PIRP               _pIrp, 
    _In_ PADAPTERCOMMON     _pAdapterCommon, 
    _In_ PENDPOINT_MINIPAIR _pAeMiniports
    )
{
    NTSTATUS    ntStatus    = STATUS_SUCCESS;
    
    PAGED_CODE();

    UNREFERENCED_PARAMETER(_pDeviceObject);

    ntStatus = _pAdapterCommon->InstallEndpointFilters(
        _pIrp,
        _pAeMiniports,
        NULL,
        NULL,
        NULL);
        
    return ntStatus;
}

#pragma code_seg("PAGE")
NTSTATUS 
InstallAllCaptureFilters(
    _In_ PDEVICE_OBJECT _pDeviceObject, 
    _In_ PIRP           _pIrp, 
    _In_ PADAPTERCOMMON _pAdapterCommon
    )
{
    NTSTATUS            ntStatus;
    PENDPOINT_MINIPAIR* ppAeMiniports     = g_CaptureEndpoints;
    
    PAGED_CODE();

    for(ULONG i = 0; i < g_cCaptureEndpoints; ++i, ++ppAeMiniports)
    {
        ntStatus = InstallEndpointCaptureFilters(_pDeviceObject, _pIrp, _pAdapterCommon, *ppAeMiniports);
        IF_FAILED_JUMP(ntStatus, Exit);
    }
    
    ntStatus = STATUS_SUCCESS;

Exit:
    return ntStatus;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
StartDevice
( 
    _In_  PDEVICE_OBJECT          DeviceObject,     
    _In_  PIRP                    Irp,              
    _In_  PRESOURCELIST           ResourceList      
)  
{
/*++

Routine Description:

  This function is called by the operating system when the device is 
  started.
  It is responsible for starting the miniports.  This code is specific to    
  the adapter because it calls out miniports for functions that are specific 
  to the adapter.                                                            

Arguments:

  DeviceObject - pointer to the driver object

  Irp - pointer to the irp 

  ResourceList - pointer to the resource list assigned by PnP manager

Return Value:

  NT status code.

--*/
    UNREFERENCED_PARAMETER(ResourceList);

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Irp);
    ASSERT(ResourceList);

    NTSTATUS                    ntStatus        = STATUS_SUCCESS;

    PADAPTERCOMMON              pAdapterCommon  = NULL;
    PUNKNOWN                    pUnknownCommon  = NULL;
    PortClassDeviceContext*     pExtension      = static_cast<PortClassDeviceContext*>(DeviceObject->DeviceExtension);

    DPF_ENTER(("[StartDevice]"));

    //
    // create a new adapter common object
    //
    ntStatus = NewAdapterCommon( 
                                &pUnknownCommon,
                                IID_IAdapterCommon,
                                NULL,
                                NonPagedPoolNx 
                                );
    IF_FAILED_JUMP(ntStatus, Exit);

    ntStatus = pUnknownCommon->QueryInterface( IID_IAdapterCommon,(PVOID *) &pAdapterCommon);
    IF_FAILED_JUMP(ntStatus, Exit);

    ntStatus = pAdapterCommon->Init(DeviceObject);
    IF_FAILED_JUMP(ntStatus, Exit);

    //
    // register with PortCls for power-management services
    ntStatus = PcRegisterAdapterPowerManagement( PUNKNOWN(pAdapterCommon), DeviceObject);
    IF_FAILED_JUMP(ntStatus, Exit);

    //
    // Install wave+topology filters for capture devices
    //
    ntStatus = InstallAllCaptureFilters(DeviceObject, Irp, pAdapterCommon);
    IF_FAILED_JUMP(ntStatus, Exit);

Exit:

    //
    // Stash the adapter common object in the device extension so
    // we can access it for cleanup on stop/removal.
    //
    if (pAdapterCommon)
    {
        ASSERT(pExtension != NULL);
        pExtension->m_pCommon = pAdapterCommon;
    }

    //
    // Release the adapter IUnknown interface.
    //
    SAFE_RELEASE(pUnknownCommon);
    
    return ntStatus;
} // StartDevice

//=============================================================================
#pragma code_seg()
NTSTATUS 
PnpHandler
(
    _In_ DEVICE_OBJECT *_DeviceObject, 
    _Inout_ IRP *_Irp
)
/*++

Routine Description:

  Handles PnP IRPs                                                           

Arguments:

  _DeviceObject - Functional Device object pointer.

  _Irp - The Irp being passed

Return Value:

  NT status code.

--*/
{
    NTSTATUS                ntStatus = STATUS_UNSUCCESSFUL;
    IO_STACK_LOCATION      *stack;
    PortClassDeviceContext *ext;

    ASSERT(_DeviceObject);
    ASSERT(_Irp);

    //
    // Check for the REMOVE_DEVICE irp.  If we're being unloaded, 
    // uninstantiate our devices and release the adapter common
    // object.
    //
    stack = IoGetCurrentIrpStackLocation(_Irp);


    if ((IRP_MN_REMOVE_DEVICE == stack->MinorFunction) ||
        (IRP_MN_SURPRISE_REMOVAL == stack->MinorFunction) ||
        (IRP_MN_STOP_DEVICE == stack->MinorFunction))
    {
        ext = static_cast<PortClassDeviceContext*>(_DeviceObject->DeviceExtension);

        if (ext->m_pCommon != NULL)
        {
            ext->m_pCommon->Cleanup();
            
            ext->m_pCommon->Release();
            ext->m_pCommon = NULL;
        }
    }
    
    ntStatus = PcDispatchIrp(_DeviceObject, _Irp);

    return ntStatus;
}

#pragma code_seg()

