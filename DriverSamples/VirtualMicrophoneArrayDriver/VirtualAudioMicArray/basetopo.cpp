/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    basetopo.cpp

Abstract:

    Implementation of topology miniport. This the base class for 
    all SYSVAD samples

--*/

//4127: conditional expression is constant
#pragma warning (disable : 4127)

#include "VirtualAudioMicArray.h"
#include "basetopo.h"

//=============================================================================
#pragma code_seg("PAGE")
CMiniportTopologySYSVAD::CMiniportTopologySYSVAD
(
    _In_        PCFILTER_DESCRIPTOR    *FilterDesc,
    _In_        USHORT                  DeviceMaxChannels
)
/*++

Routine Description:

  Topology miniport constructor

Arguments:

  FilterDesc - 

  DeviceMaxChannels - 

Return Value:

  void

--*/
{
    PAGED_CODE();

    DPF_ENTER(("[%s]",__FUNCTION__));

    m_AdapterCommon     = NULL;

    ASSERT(FilterDesc != NULL);
    m_FilterDescriptor  = FilterDesc;
    m_PortEvents        = NULL;
    
    ASSERT(DeviceMaxChannels > 0);
    m_DeviceMaxChannels = DeviceMaxChannels;
} // CMiniportTopologySYSVAD

CMiniportTopologySYSVAD::~CMiniportTopologySYSVAD
(
    void
)
/*++

Routine Description:

  Topology miniport destructor

Arguments:

Return Value:

  void

--*/
{
    PAGED_CODE();

    DPF_ENTER(("[%s]",__FUNCTION__));

    SAFE_RELEASE(m_AdapterCommon);
    SAFE_RELEASE(m_PortEvents);
} // ~CMiniportTopologySYSVAD

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportTopologySYSVAD::DataRangeIntersection
( 
    _In_  ULONG                 PinId,
    _In_  PKSDATARANGE          ClientDataRange,
    _In_  PKSDATARANGE          MyDataRange,
    _In_  ULONG                 OutputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ResultantFormatLength)
          PVOID                 ResultantFormat     OPTIONAL,
    _Out_ PULONG                ResultantFormatLength 
)
/*++

Routine Description:

  The DataRangeIntersection function determines the highest 
  quality intersection of two data ranges. Topology miniport does nothing.

Arguments:

  PinId - Pin for which data intersection is being determined. 

  ClientDataRange - Pointer to KSDATARANGE structure which contains the data range 
                    submitted by client in the data range intersection property 
                    request

  MyDataRange - Pin's data range to be compared with client's data range

  OutputBufferLength - Size of the buffer pointed to by the resultant format 
                       parameter

  ResultantFormat - Pointer to value where the resultant format should be 
                    returned

  ResultantFormatLength - Actual length of the resultant format that is placed 
                          at ResultantFormat. This should be less than or equal 
                          to OutputBufferLength

Return Value:

  NT status code.

--*/
{
    UNREFERENCED_PARAMETER(PinId);
    UNREFERENCED_PARAMETER(ClientDataRange);
    UNREFERENCED_PARAMETER(MyDataRange);
    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(ResultantFormat);
    UNREFERENCED_PARAMETER(ResultantFormatLength);

    PAGED_CODE();

    DPF_ENTER(("[%s]",__FUNCTION__));

    return (STATUS_NOT_IMPLEMENTED);
} // DataRangeIntersection

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportTopologySYSVAD::GetDescription
( 
    _Out_ PPCFILTER_DESCRIPTOR *  OutFilterDescriptor 
)
/*++

Routine Description:

  The GetDescription function gets a pointer to a filter description. 
  It provides a location to deposit a pointer in miniport's description 
  structure. This is the placeholder for the FromNode or ToNode fields in 
  connections which describe connections to the filter's pins

Arguments:

  OutFilterDescriptor - Pointer to the filter description. 

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    ASSERT(OutFilterDescriptor);

    DPF_ENTER(("[%s]",__FUNCTION__));

    *OutFilterDescriptor = m_FilterDescriptor;

    return (STATUS_SUCCESS);
} // GetDescription

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS
CMiniportTopologySYSVAD::Init
( 
    _In_  PUNKNOWN          UnknownAdapter_,
    _In_  PPORTTOPOLOGY     Port_ 
)
/*++

Routine Description:

  Initializes the topology miniport.

Arguments:

  UnknownAdapter -

  Port_ - Pointer to topology port

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();
    
    ASSERT(UnknownAdapter_);
    ASSERT(Port_);

    DPF_ENTER(("[CMiniportTopologySYSVAD::Init]"));

    NTSTATUS    ntStatus;

    ntStatus = 
        UnknownAdapter_->QueryInterface( 
            IID_IAdapterCommon,
            (PVOID *) &m_AdapterCommon);
    
    if (NT_SUCCESS(ntStatus))
    {
        //
        // Get the port event interface.
        //
        ntStatus = Port_->QueryInterface(
            IID_IPortEvents, 
            (PVOID *)&m_PortEvents);
    }

    if (!NT_SUCCESS(ntStatus))
    {
        // clean up AdapterCommon
        SAFE_RELEASE(m_AdapterCommon);
        SAFE_RELEASE(m_PortEvents);
    }

    return ntStatus;
} // Init

//=============================================================================
#pragma code_seg("PAGE")
NTSTATUS                            
CMiniportTopologySYSVAD::PropertyHandlerGeneric
(
    _In_  PPCPROPERTY_REQUEST     PropertyRequest
)
/*++

Routine Description:

  Handles all properties for this miniport.

Arguments:

  PropertyRequest - property request structure

Return Value:

  NT status code.

--*/
{
    PAGED_CODE();

    NTSTATUS                    ntStatus = STATUS_INVALID_DEVICE_REQUEST;

    switch (PropertyRequest->PropertyItem->Id)
    {

        case KSPROPERTY_AUDIO_CPU_RESOURCES:
            ntStatus = PropertyHandler_CpuResources(PropertyRequest);
            break;

        default:
            DPF(D_TERSE, ("[PropertyHandlerGeneric: Invalid Device Request]"));
    }

    return ntStatus;
} // PropertyHandlerGeneric

//=============================================================================
#pragma code_seg("PAGE")
VOID
CMiniportTopologySYSVAD::AddEventToEventList
(
    _In_  PKSEVENT_ENTRY    EventEntry 
)
/*++

Routine Description:

  The AddEventToEventList method adds an event to the port driver's event list

Arguments:

  EventEntry - 

--*/
{
    PAGED_CODE();
    DPF_ENTER(("[CMiniportTopology::AddEventToEventList]"));

    ASSERT(m_PortEvents != NULL);

    m_PortEvents->AddEventToEventList(EventEntry);
}

//=============================================================================
#pragma code_seg()
VOID
CMiniportTopologySYSVAD::GenerateEventList
(
    _In_opt_    GUID   *Set,
    _In_        ULONG   EventId,
    _In_        BOOL    PinEvent,
    _In_        ULONG   PinId,
    _In_        BOOL    NodeEvent,
    _In_        ULONG   NodeId
)
/*++

Routine Description:

  The GenerateEventList method notifies clients through the port driver's list 
  of event entries that a particular event has occurred.

Arguments:

  Set -

  EventId - 

  PinEvent -

  PinId -

  NodeEvent -

  NodeId -

--*/
{
    DPF_ENTER(("[CMiniportTopologySYSVAD::GenerateEventList]"));

    ASSERT(m_PortEvents != NULL);

    m_PortEvents->GenerateEventList(
        Set,
        EventId,
        PinEvent,
        PinId,
        NodeEvent,
        NodeId);
}
 
#pragma code_seg()

