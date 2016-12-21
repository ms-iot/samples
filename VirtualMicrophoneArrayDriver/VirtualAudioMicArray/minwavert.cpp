/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    minwavert.cpp

Abstract:

    Implementation of wavert miniport.

--*/

#pragma warning (disable : 4127)

#include "VirtualAudioMicArray.h"
#include <limits.h>

#include "simple.h"
#include "minwavert.h"
#include "minwavertstream.h"


//=============================================================================
// CMiniportWaveRT
//=============================================================================

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CreateMiniportWaveRTVirtualMicArray
( 
    _Out_           PUNKNOWN                              * Unknown,
    _In_            REFCLSID,
    _In_opt_        PUNKNOWN                                UnknownOuter,
    _When_((PoolType & NonPagedPoolMustSucceed) != 0,
       __drv_reportError("Must succeed pool allocations are forbidden. "
             "Allocation failures cause a system crash"))
    _In_            POOL_TYPE                               PoolType,
    _In_            PUNKNOWN                                UnknownAdapter,
    _In_opt_        PVOID                                   DeviceContext,
    _In_            PENDPOINT_MINIPAIR                      MiniportPair
)
/*++

Routine Description:

  Create the wavert miniport.

Arguments:

  Unknown - 

  RefClsId -

  UnknownOuter -

  PoolType -

  UnkownAdapter -

  DeviceContext -

  MiniportPair -

Return Value:

  NT status code.

--*/
{
    UNREFERENCED_PARAMETER(UnknownOuter);

    PAGED_CODE();

    ASSERT(Unknown);
    ASSERT(MiniportPair);

    CMiniportWaveRT *obj = new (PoolType, MINWAVERT_POOLTAG) CMiniportWaveRT
                                                             (
                                                                UnknownAdapter,
                                                                MiniportPair,
                                                                DeviceContext
                                                             );
    if (NULL == obj)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    
    obj->AddRef();
    *Unknown = reinterpret_cast<IUnknown*>(obj);

    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
CMiniportWaveRT::~CMiniportWaveRT
( 
    void 
)
/*++

Routine Description:

  Destructor for wavert miniport

Arguments:

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveRT::~CMiniportWaveRT]"));

    if (m_plVolumeLevel)
    {
        ExFreePoolWithTag( m_plVolumeLevel, MINWAVERT_POOLTAG );
        m_plVolumeLevel = NULL;
    }

    if (m_pPortEvents)
    {
        m_pPortEvents->Release();
        m_pPortEvents = NULL;
    }

} // ~CMiniportWaveRT

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CMiniportWaveRT::DataRangeIntersection
( 
    _In_        ULONG                       PinId,
    _In_        PKSDATARANGE                ClientDataRange,
    _In_        PKSDATARANGE                MyDataRange,
    _In_        ULONG                       OutputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ResultantFormatLength)
                PVOID                       ResultantFormat,
    _Out_       PULONG                      ResultantFormatLength 
)
/*++

Routine Description:

  The DataRangeIntersection function determines the highest quality 
  intersection of two data ranges.

  This sample just validates the # of channels and lets the class handler
  do the rest.

Arguments:

  PinId -           Pin for which data intersection is being determined. 

  ClientDataRange - Pointer to KSDATARANGE structure which contains the data 
                    range submitted by client in the data range intersection 
                    property request. 

  MyDataRange -         Pin's data range to be compared with client's data 
                        range. In this case we actually ignore our own data 
                        range, because we know that we only support one range.

  OutputBufferLength -  Size of the buffer pointed to by the resultant format 
                        parameter. 

  ResultantFormat -     Pointer to value where the resultant format should be 
                        returned. 

  ResultantFormatLength -   Actual length of the resultant format placed in 
                            ResultantFormat. This should be less than or equal 
                            to OutputBufferLength. 

  Return Value:

    NT status code.

  Remarks:

    This sample driver's custom data intersection handler handles all the
    audio endpoints defined in this driver. Some endpoints support mono formats
    while others do not. The handler is written such that it requires an exact
    match in MaximumChannels. This simplifies the handler but requires the pin
    data ranges to include a separate data range for mono formats if the pin
    supports mono formats.

--*/
{
    ULONG                   requiredSize;

    UNREFERENCED_PARAMETER(PinId);
    UNREFERENCED_PARAMETER(ResultantFormat);

    PAGED_CODE();

    if (!IsEqualGUIDAligned(ClientDataRange->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
    {
        return STATUS_NOT_IMPLEMENTED;
    }

    requiredSize = sizeof (KSDATAFORMAT_WAVEFORMATEX);

    //
    // Validate return buffer size, if the request is only for the
    // size of the resultant structure, return it now before
    // returning other types of errors.
    //
    if (!OutputBufferLength) 
    {
        *ResultantFormatLength = requiredSize;
        return STATUS_BUFFER_OVERFLOW;
    } 
    else if (OutputBufferLength < requiredSize) 
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Verify channel count is supported. This routine assumes a separate data
    // range for each supported channel count.
    if (((PKSDATARANGE_AUDIO)MyDataRange)->MaximumChannels != ((PKSDATARANGE_AUDIO)ClientDataRange)->MaximumChannels)
    {
        return STATUS_NO_MATCH;
    }
    
    //
    // Ok, let the class handler do the rest.
    //
    return STATUS_NOT_IMPLEMENTED;
} // DataRangeIntersection

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CMiniportWaveRT::GetDescription
( 
    _Out_ PPCFILTER_DESCRIPTOR * OutFilterDescriptor 
)
/*++

Routine Description:

  The GetDescription function gets a pointer to a filter description. 
  It provides a location to deposit a pointer in miniport's description 
  structure.

Arguments:

  OutFilterDescriptor - Pointer to the filter description. 

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    *OutFilterDescriptor = &m_FilterDesc;

    return STATUS_SUCCESS;
} // GetDescription

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CMiniportWaveRT::Init
( 
    _In_  PUNKNOWN                UnknownAdapter_,
    _In_  PRESOURCELIST           ResourceList_,
    _In_  PPORTWAVERT             Port_ 
)
/*++

Routine Description:

  The Init function initializes the miniport. Callers of this function 
  should run at IRQL PASSIVE_LEVEL

Arguments:

  UnknownAdapter - A pointer to the Iuknown interface of the adapter object. 

  ResourceList - Pointer to the resource list to be supplied to the miniport 
                 during initialization. The port driver is free to examine the 
                 contents of the ResourceList. The port driver will not be 
                 modify the ResourceList contents. 

  Port - Pointer to the topology port object that is linked with this miniport. 

Return Value:

  NT status code.

--*/
{
    UNREFERENCED_PARAMETER(UnknownAdapter_);
    UNREFERENCED_PARAMETER(ResourceList_);
    UNREFERENCED_PARAMETER(Port_);
    PAGED_CODE();

    ASSERT(UnknownAdapter_);
    ASSERT(Port_);

    DPF_ENTER(("[CMiniportWaveRT::Init]"));

    NTSTATUS ntStatus = STATUS_SUCCESS;

    //
    // Init class data members
    //
    m_CaptureStream                 = NULL;
    m_plVolumeLevel                 = NULL;
    
  
    // 
    // For KS event support.
    //
    if (!NT_SUCCESS(Port_->QueryInterface(IID_IPortEvents, (PVOID *)&m_pPortEvents)))
    {
        m_pPortEvents = NULL;
    }

    return ntStatus;
} // Init

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CMiniportWaveRT::NewStream
( 
    _Out_ PMINIPORTWAVERTSTREAM * OutStream,
    _In_  PPORTWAVERTSTREAM       OuterUnknown,
    _In_  ULONG                   Pin,
    _In_  BOOLEAN                 Capture,
    _In_  PKSDATAFORMAT           DataFormat
)
/*++

Routine Description:

  The NewStream function creates a new instance of a logical stream 
  associated with a specified physical channel. Callers of NewStream should 
  run at IRQL PASSIVE_LEVEL.

Arguments:

  OutStream -

  OuterUnknown -

  Pin - 

  Capture - 

  DataFormat -

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    ASSERT(OutStream);
    ASSERT(DataFormat);

    DPF_ENTER(("[CMiniportWaveRT::NewStream]"));

    NTSTATUS                    ntStatus = STATUS_SUCCESS;
    PCMiniportWaveRTStream      stream = NULL;
    GUID                        signalProcessingMode = AUDIO_SIGNALPROCESSINGMODE_DEFAULT;
    
    *OutStream = NULL;

     //
    // If the data format attributes were specified, extract them.
    //
    if ( DataFormat->Flags & KSDATAFORMAT_ATTRIBUTES )
    {
        // The attributes are aligned (QWORD alignment) after the data format
        PKSMULTIPLE_ITEM attributes = (PKSMULTIPLE_ITEM) (((PBYTE)DataFormat) + ((DataFormat->FormatSize + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT));
        ntStatus = GetAttributesFromAttributeList(attributes, attributes->Size, &signalProcessingMode);
    }

    // Check if we have enough streams.
    //
    if (m_CaptureStream != NULL)
    {
        return STATUS_RESOURCE_IN_USE;
    }

    // Determine if the format is valid.
    //
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = IsFormatSupported(Pin, Capture, DataFormat);
    }

    // Instantiate a stream. Stream must be in
    // NonPagedPool(Nx) because of file saving.
    //
    if (NT_SUCCESS(ntStatus))
    {
        stream = new (NonPagedPoolNx, MINWAVERT_POOLTAG) 
            CMiniportWaveRTStream(NULL);

        if (stream)
        {
            stream->AddRef();

            ntStatus = 
                stream->Init
                ( 
                    this,
                    OuterUnknown,
                    Pin,
                    Capture,
                    DataFormat,
                    signalProcessingMode
                );
        }
        else
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    if (NT_SUCCESS(ntStatus))
    {
        m_CaptureStream = stream;
        *OutStream = PMINIPORTWAVERTSTREAM(stream);
        (*OutStream)->AddRef();
        

        // The stream has references now for the caller.  The caller expects these
        // references to be there.
    }

    // This is our private reference to the stream.  The caller has
    // its own, so we can release in any case.
    //
    if (stream)
    {
        stream->Release();
    }
    
    return ntStatus;
} // NewStream

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS)
CMiniportWaveRT::NonDelegatingQueryInterface
( 
    _In_ REFIID  Interface,
    _COM_Outptr_ PVOID * Object 
)
/*++

Routine Description:

  QueryInterface

Arguments:

  Interface - GUID

  Object - interface pointer to be returned.

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    ASSERT(Object);

    if (IsEqualGUIDAligned(Interface, IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMINIPORTWAVERT(this)));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniport))
    {
        *Object = PVOID(PMINIPORT(this));
    }
    else if (IsEqualGUIDAligned(Interface, IID_IMiniportWaveRT))
    {
        *Object = PVOID(PMINIPORTWAVERT(this));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        // We reference the interface for the caller.

        PUNKNOWN(*Object)->AddRef();
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_PARAMETER;
} // NonDelegatingQueryInterface

//=============================================================================
#pragma code_seg("PAGE")
STDMETHODIMP_(NTSTATUS) CMiniportWaveRT::GetDeviceDescription(_Out_ PDEVICE_DESCRIPTION DmaDeviceDescription)
{
    PAGED_CODE ();

    ASSERT (DmaDeviceDescription);

    DPF_ENTER(("[CMiniportWaveRT::GetDeviceDescription]"));

    RtlZeroMemory (DmaDeviceDescription, sizeof (DEVICE_DESCRIPTION));

    //
    // Init device description. This sample is using the same info for all m_DeviceType(s).
    // 
    
    DmaDeviceDescription->Master = TRUE;
    DmaDeviceDescription->ScatterGather = TRUE;
    DmaDeviceDescription->Dma32BitAddresses = TRUE;
    DmaDeviceDescription->InterfaceType = PCIBus;
    DmaDeviceDescription->MaximumLength = 0xFFFFFFFF;

    return STATUS_SUCCESS;
}



//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::StreamCreated
(
    _In_ ULONG                  _Pin,
    _In_ PCMiniportWaveRTStream _Stream
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(_Pin);
    UNREFERENCED_PARAMETER(_Stream);

    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::StreamClosed
(
    _In_ ULONG                  _Pin,
    _In_ PCMiniportWaveRTStream _Stream
)
{
    PAGED_CODE();
    UNREFERENCED_PARAMETER(_Pin);
    UNREFERENCED_PARAMETER(_Stream);
    
    m_CaptureStream = NULL;
    
    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::GetAttributesFromAttributeList
(
    _In_ const KSMULTIPLE_ITEM *_pAttributes,
    _In_ size_t _Size,
    _Out_ GUID* _pSignalProcessingMode
)
/*++

Routine Description:

  Processes attributes list and return known attributes.

Arguments:

  _pAttributes - pointer to KSMULTIPLE_ITEM at head of an attributes list.

  _Size - count of bytes in the buffer pointed to by _pAttributes. The routine
    verifies sufficient buffer size while processing the attributes.

  _pSignalProcessingMode - returns the signal processing mode extracted from
    the attribute list, or AUDIO_SIGNALPROCESSINGMODE_DEFAULT if the attribute
    is not present in the list.

Return Value:

  NT status code.

Remarks

    This function is currently written for a single supported attribute
    (KSATTRIBUTEID_AUDIOSIGNALPROCESSING_MODE). As additional attributes are defined in the future,
    this function should be rewritten to be data driven through tables, etc.

--*/
{
    PAGED_CODE();
    
    DPF_ENTER(("[CMiniportWaveRT::GetAttributesFromAttributeList]"));

    size_t cbRemaining = _Size;

    *_pSignalProcessingMode = AUDIO_SIGNALPROCESSINGMODE_DEFAULT;

    if (cbRemaining < sizeof(KSMULTIPLE_ITEM))
    {
        return STATUS_INVALID_PARAMETER;
    }
    cbRemaining -= sizeof(KSMULTIPLE_ITEM);

    //
    // Extract attributes.
    //
    PKSATTRIBUTE attributeHeader = (PKSATTRIBUTE)(_pAttributes + 1);

    for (ULONG i = 0; i < _pAttributes->Count; i++)
    {
        if (cbRemaining < sizeof(KSATTRIBUTE))
        {
            return STATUS_INVALID_PARAMETER;
        }

        if (attributeHeader->Attribute == KSATTRIBUTEID_AUDIOSIGNALPROCESSING_MODE)
        {
            KSATTRIBUTE_AUDIOSIGNALPROCESSING_MODE* signalProcessingModeAttribute;

            if (cbRemaining < sizeof(KSATTRIBUTE_AUDIOSIGNALPROCESSING_MODE))
            {
                return STATUS_INVALID_PARAMETER;
            }

            if (attributeHeader->Size != sizeof(KSATTRIBUTE_AUDIOSIGNALPROCESSING_MODE))
            {
                return STATUS_INVALID_PARAMETER;
            }

            signalProcessingModeAttribute = (KSATTRIBUTE_AUDIOSIGNALPROCESSING_MODE*)attributeHeader;

            // Return mode to caller.
            *_pSignalProcessingMode = signalProcessingModeAttribute->SignalProcessingMode;
        }
        else
        {
            return STATUS_NOT_SUPPORTED;
        }

        // Adjust pointer and buffer size to next attribute (QWORD aligned)
        ULONG cbAttribute = ((attributeHeader->Size + FILE_QUAD_ALIGNMENT) & ~FILE_QUAD_ALIGNMENT);

        attributeHeader = (PKSATTRIBUTE) (((PBYTE)attributeHeader) + cbAttribute);
        cbRemaining -= cbAttribute;
    }

    return STATUS_SUCCESS;
}

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::IsFormatSupported
(
    _In_ ULONG          _ulPin,
    _In_ BOOLEAN        _bCapture,
    _In_ PKSDATAFORMAT  _pDataFormat
)
{
    PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveRT::IsFormatSupported]"));

    NTSTATUS                            ntStatus = STATUS_NO_MATCH;
    PKSDATAFORMAT_WAVEFORMATEXTENSIBLE  pPinFormats = NULL;
    ULONG                               cPinFormats = 0;

    UNREFERENCED_PARAMETER(_bCapture);

    if (_ulPin >= m_pMiniportPair->WaveDescriptor->PinCount)
    {
        return STATUS_INVALID_PARAMETER;
    }

    cPinFormats = GetPinSupportedDeviceFormats(_ulPin, &pPinFormats);

    for (UINT iFormat = 0; iFormat < cPinFormats; iFormat++)
    {
        PKSDATAFORMAT_WAVEFORMATEXTENSIBLE pFormat = &pPinFormats[iFormat];
        // KSDATAFORMAT VALIDATION
        if (!IsEqualGUIDAligned(pFormat->DataFormat.MajorFormat, _pDataFormat->MajorFormat)) { continue; }
        if (!IsEqualGUIDAligned(pFormat->DataFormat.SubFormat, _pDataFormat->SubFormat)) { continue; }
        if (!IsEqualGUIDAligned(pFormat->DataFormat.Specifier, _pDataFormat->Specifier)) { continue; }
        if (pFormat->DataFormat.FormatSize < sizeof(KSDATAFORMAT_WAVEFORMATEX)) { continue; }

        // WAVEFORMATEX VALIDATION
        PWAVEFORMATEX pWaveFormat = reinterpret_cast<PWAVEFORMATEX>(_pDataFormat + 1);
        
        if (pWaveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        {
            if (pWaveFormat->wFormatTag != EXTRACT_WAVEFORMATEX_ID(&(pFormat->WaveFormatExt.SubFormat))) { continue; }
        }
        if (pWaveFormat->nChannels  != pFormat->WaveFormatExt.Format.nChannels) { continue; }
        if (pWaveFormat->nSamplesPerSec != pFormat->WaveFormatExt.Format.nSamplesPerSec) { continue; }
        if (pWaveFormat->nBlockAlign != pFormat->WaveFormatExt.Format.nBlockAlign) { continue; }
        if (pWaveFormat->wBitsPerSample != pFormat->WaveFormatExt.Format.wBitsPerSample) { continue; }

        if (pWaveFormat->wFormatTag != WAVE_FORMAT_EXTENSIBLE)
        {
            ntStatus = STATUS_SUCCESS;
            break;
        }

        // WAVEFORMATEXTENSIBLE VALIDATION
        if (pWaveFormat->cbSize < sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)) { continue; }

        PWAVEFORMATEXTENSIBLE pWaveFormatExt = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pWaveFormat);
        if (pWaveFormatExt->Samples.wValidBitsPerSample != pFormat->WaveFormatExt.Samples.wValidBitsPerSample) { continue; }
        if (pWaveFormatExt->dwChannelMask != pFormat->WaveFormatExt.dwChannelMask) { continue; }
        if (!IsEqualGUIDAligned(pWaveFormatExt->SubFormat, pFormat->WaveFormatExt.SubFormat)) { continue; }

        ntStatus = STATUS_SUCCESS;
        break;
    }

    return ntStatus;
}    

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::PropertyHandlerProposedFormat
(
    _In_ PPCPROPERTY_REQUEST      PropertyRequest
)
{
    PKSP_PIN                kspPin                  = NULL;
    PKSDATAFORMAT           pKsFormat               = NULL;
    ULONG                   cbMinSize               = 0;
    NTSTATUS                ntStatus                = STATUS_INVALID_PARAMETER;

    PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveRT::PropertyHandlerProposedFormat]"));
    
    // All properties handled by this handler require at least a KSP_PIN descriptor.

    // Verify instance data stores at least KSP_PIN fields beyond KSPPROPERTY.
    if (PropertyRequest->InstanceSize < (sizeof(KSP_PIN) - RTL_SIZEOF_THROUGH_FIELD(KSP_PIN, Property)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Extract property descriptor from property request instance data
    kspPin = CONTAINING_RECORD(PropertyRequest->Instance, KSP_PIN, PinId);

    //
    // This method is valid only on streaming pins.
    //
    if (IsSystemCapturePin(kspPin->PinId))
    {
        ntStatus = STATUS_SUCCESS;
    }
    else if (IsBridgePin(kspPin->PinId))
    {
        ntStatus = STATUS_NOT_SUPPORTED;
    }
    else 
    {
        ntStatus = STATUS_INVALID_PARAMETER;
    }

    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    cbMinSize = sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE);
   
    // Handle KSPROPERTY_TYPE_BASICSUPPORT query
    if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        ULONG flags = PropertyRequest->PropertyItem->Flags;
        
        return PropertyHandler_BasicSupport(PropertyRequest, flags, VT_ILLEGAL);
    }

    // Verify value size
    if (PropertyRequest->ValueSize == 0)
    {
        PropertyRequest->ValueSize = cbMinSize;
        return STATUS_BUFFER_OVERFLOW;
    }
    if (PropertyRequest->ValueSize < cbMinSize)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Only SET is supported for this property
    if ((PropertyRequest->Verb & KSPROPERTY_TYPE_SET) == 0)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    pKsFormat = (PKSDATAFORMAT)PropertyRequest->Value;
    ntStatus = IsFormatSupported(kspPin->PinId, 
                                 (BOOLEAN)IsSystemCapturePin(kspPin->PinId),
                                 pKsFormat);
    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    return ntStatus;
} // PropertyHandlerProposedFormat

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::PropertyHandlerProposedFormat2
(
    _In_ PPCPROPERTY_REQUEST      PropertyRequest
)
{
    PKSP_PIN                kspPin                  = NULL;
    ULONG                   cbMinSize               = 0;
    NTSTATUS                ntStatus                = STATUS_INVALID_PARAMETER;
    ULONG                   numModes                = 0;
    MODE_AND_DEFAULT_FORMAT *modeInfo               = NULL;
    MODE_AND_DEFAULT_FORMAT *modeTemp               = NULL;
    PKSMULTIPLE_ITEM        pKsItemsHeader          = NULL;
    PKSMULTIPLE_ITEM        pKsItemsHeaderOut       = NULL;
    size_t                  cbItemsList             = 0;
    GUID                    signalProcessingMode    = {0};
    BOOLEAN                 bFound                  = FALSE;
    ULONG                   i;

    PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveRT::PropertyHandlerProposedFormat2]"));
    
    // All properties handled by this handler require at least a KSP_PIN descriptor.

    // Verify instance data stores at least KSP_PIN fields beyond KSPPROPERTY.
    if (PropertyRequest->InstanceSize < (sizeof(KSP_PIN) - RTL_SIZEOF_THROUGH_FIELD(KSP_PIN, Property)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Extract property descriptor from property request instance data
    kspPin = CONTAINING_RECORD(PropertyRequest->Instance, KSP_PIN, PinId);

    if (kspPin->PinId >= m_pMiniportPair->WaveDescriptor->PinCount)
    {
        return STATUS_INVALID_PARAMETER;
    }

    //
    // This property is supported only on some streaming pins.
    //
    numModes = GetPinSupportedDeviceModes(kspPin->PinId, &modeInfo);

    ASSERT((modeInfo != NULL && numModes > 0) || (modeInfo == NULL && numModes == 0));

    if (modeInfo == NULL)
    {
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Even for pins that support modes, the pin might not support proposed formats
    //
    bFound = FALSE;
    for (i=0, modeTemp=modeInfo; i<numModes; ++i, ++modeTemp)
    {
        if (modeTemp->DefaultFormat != NULL)
        {
            bFound = TRUE;
            break;
        }
    }

    if (!bFound)
    {
        return STATUS_NOT_SUPPORTED;
    }

    //
    // The property is generally supported on this pin. Handle basic support request.
    //
    if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        return PropertyHandler_BasicSupport(PropertyRequest, PropertyRequest->PropertyItem->Flags, VT_ILLEGAL);
    }

    //
    // Get the mode if specified.
    //
    pKsItemsHeader = (PKSMULTIPLE_ITEM)(kspPin + 1);
    cbItemsList = (((PBYTE)PropertyRequest->Instance) + PropertyRequest->InstanceSize) - (PBYTE)pKsItemsHeader;

    ntStatus = GetAttributesFromAttributeList(pKsItemsHeader, cbItemsList, &signalProcessingMode);
    if (!NT_SUCCESS(ntStatus))
    {
        return ntStatus;
    }

    //
    // Get the info associated with this mode.
    //
    bFound = FALSE;
    for (i=0; i<numModes; ++i, ++modeInfo)
    {
        if (modeInfo->Mode == signalProcessingMode)
        {
            bFound = TRUE;
            break;
        }
    }

    // Either the mode isn't supported, or the driver doesn't support a
    // proprosed format for this specific mode.
    if (!bFound || modeInfo->DefaultFormat == NULL)
    {
        return STATUS_NOT_SUPPORTED;
    }

    //
    // Compute output data buffer.
    //
    cbMinSize = modeInfo->DefaultFormat->FormatSize;
    cbMinSize = (cbMinSize + 7) & ~7;

    pKsItemsHeaderOut = (PKSMULTIPLE_ITEM)((PBYTE)PropertyRequest->Value + cbMinSize);

    if (cbItemsList > MAXULONG)
    {
        return STATUS_INVALID_PARAMETER;
    }

    // Total # of bytes.
    ntStatus = RtlULongAdd(cbMinSize, (ULONG)cbItemsList, &cbMinSize);
    if (!NT_SUCCESS(ntStatus))
    {
        return STATUS_INVALID_PARAMETER;
    }
        
    // Property not supported.
    if (cbMinSize == 0)
    {
        return STATUS_NOT_SUPPORTED;
    }

    // Verify value size
    if (PropertyRequest->ValueSize == 0)
    {
        PropertyRequest->ValueSize = cbMinSize;
        return STATUS_BUFFER_OVERFLOW;
    }
    if (PropertyRequest->ValueSize < cbMinSize)
    {
        return STATUS_BUFFER_TOO_SMALL;
    }

    // Only GET is supported for this property
    if ((PropertyRequest->Verb & KSPROPERTY_TYPE_GET) == 0)
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    // Copy the proposed default format.
    RtlCopyMemory(PropertyRequest->Value, modeInfo->DefaultFormat, modeInfo->DefaultFormat->FormatSize);

    // Copy back the attribute list.
    ASSERT(cbItemsList > 0);
    ((KSDATAFORMAT*)PropertyRequest->Value)->Flags = KSDATAFORMAT_ATTRIBUTES;
    RtlCopyMemory(pKsItemsHeaderOut, pKsItemsHeader, cbItemsList);
    
    PropertyRequest->ValueSize = cbMinSize;

    return STATUS_SUCCESS;
} // PropertyHandlerProposedFormat




//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportWaveRT::PropertyHandlerEffectListRequest
(
    _In_ PPCPROPERTY_REQUEST      PropertyRequest
)
{
    GUID StreamEffectList[] =
    {
        AUDIO_EFFECT_TYPE_LOUDNESS_EQUALIZER,
        AUDIO_EFFECT_TYPE_VIRTUAL_SURROUND
    };

    PAGED_CODE();

    DPF_ENTER(("[CMiniportWaveRT::PropertyHandlerEffectListRequest]"));

    // This specific APO->driver communication example is mainly added to show to this communication is done.
    // It skips the pin id validation and returns pin specific answers to the caller, which a real miniport 
    // audio driver probably needs to tate care of.

    // Handle KSPROPERTY_TYPE_BASICSUPPORT query
    if (PropertyRequest->Verb & KSPROPERTY_TYPE_BASICSUPPORT)
    {
        return PropertyHandler_BasicSupport(PropertyRequest, PropertyRequest->PropertyItem->Flags, VT_ILLEGAL);
    }

    // Verify instance data stores at least KSP_PIN fields beyond KSPPROPERTY.
    if (PropertyRequest->InstanceSize < (sizeof(KSP_PIN) - RTL_SIZEOF_THROUGH_FIELD(KSP_PIN, Property)))
    {
        return STATUS_INVALID_PARAMETER;
    }

    if (PropertyRequest->Verb & KSPROPERTY_TYPE_GET)
    {
        PKSMULTIPLE_ITEM ksMultipleItem;
        ULONG ulEffectsCount = ARRAYSIZE(StreamEffectList);
        ULONG cbMinSize;
        LPGUID pEffectGuids = NULL;

        // Compute min value size requirements    
        cbMinSize = sizeof(KSMULTIPLE_ITEM) + ulEffectsCount * sizeof(GUID);

        // Verify value size
        if (PropertyRequest->ValueSize == 0)
        {
            PropertyRequest->ValueSize = cbMinSize;
            return STATUS_BUFFER_OVERFLOW;
        }
        if (PropertyRequest->ValueSize < cbMinSize)
        {
            return STATUS_BUFFER_TOO_SMALL;
        }
        // Value is a KSMULTIPLE_ITEM followed by list of GUIDs.
        ksMultipleItem = (PKSMULTIPLE_ITEM)PropertyRequest->Value;
        pEffectGuids = (LPGUID)(ksMultipleItem + 1);

        // Copy effect guid 
        RtlCopyMemory(pEffectGuids, StreamEffectList, ulEffectsCount * sizeof(GUID));

        // Miniport filled in the list of GUIDs. Fill in the KSMULTIPLE_ITEM header.
        ksMultipleItem->Size = sizeof(KSMULTIPLE_ITEM) + ulEffectsCount * sizeof(GUID);
        ksMultipleItem->Count = ulEffectsCount;

        PropertyRequest->ValueSize = ksMultipleItem->Size;
        return STATUS_SUCCESS;
    }

    return STATUS_INVALID_DEVICE_REQUEST;

} // PropertyHandlerEffectListRequest
//=============================================================================
#pragma code_seg("PAGE")

//=============================================================================


#pragma code_seg("PAGE")
NTSTATUS
PropertyHandler_WaveFilter
( 
    _In_ PPCPROPERTY_REQUEST      PropertyRequest 
)
/*++

Routine Description:

  Redirects general property request to miniport object

Arguments:

  PropertyRequest - 

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    NTSTATUS            ntStatus = STATUS_INVALID_DEVICE_REQUEST;
    CMiniportWaveRT*    pWaveHelper = reinterpret_cast<CMiniportWaveRT*>(PropertyRequest->MajorTarget);

    if (pWaveHelper == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    pWaveHelper->AddRef();

    if (IsEqualGUIDAligned(*PropertyRequest->PropertyItem->Set, KSPROPSETID_Pin))
    {
        switch (PropertyRequest->PropertyItem->Id)
        {
            case KSPROPERTY_PIN_PROPOSEDATAFORMAT:
                ntStatus = pWaveHelper->PropertyHandlerProposedFormat(PropertyRequest);
                break;
            
            case KSPROPERTY_PIN_PROPOSEDATAFORMAT2:
                ntStatus = pWaveHelper->PropertyHandlerProposedFormat2(PropertyRequest);
                break;

            default:
                DPF(D_TERSE, ("[PropertyHandler_WaveFilter: Invalid Device Request]"));
        }
    }
    
    

    pWaveHelper->Release();

    return ntStatus;
} // PropertyHandler_WaveFilter
//=============================================================================

