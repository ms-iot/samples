/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    minwavert.h

Abstract:

    Definition of wavert miniport class.

--*/

#ifndef _SYSVAD_MINWAVERT_H_
#define _SYSVAD_MINWAVERT_H_


//=============================================================================
// Referenced Forward
//=============================================================================
class CMiniportWaveRTStream;
typedef CMiniportWaveRTStream *PCMiniportWaveRTStream;

//=============================================================================
// Classes
//=============================================================================

///////////////////////////////////////////////////////////////////////////////
// CMiniportWaveRT
//   
class CMiniportWaveRT : 
    public IMiniportWaveRT,
    public CUnknown
{
private:
    // weak ref of running streams.
    PCMiniportWaveRTStream              m_CaptureStream;
    
    PLONG                               m_plVolumeLevel;
 
    PCFILTER_DESCRIPTOR                 m_FilterDesc;
    PIN_DEVICE_FORMATS_AND_MODES *      m_DeviceFormatsAndModes;
    ULONG                               m_DeviceFormatsAndModesCount; 
    USHORT                              m_DeviceMaxChannels;

    union {
        PVOID                           m_DeviceContext;
    };

protected:
    PADAPTERCOMMON                      m_pAdapterCommon;
    ULONG                               m_DeviceFlags;
    eDeviceType                         m_DeviceType;
    PPORTEVENTS                         m_pPortEvents;
    PENDPOINT_MINIPAIR                  m_pMiniportPair;

public:
    
   
    NTSTATUS StreamCreated
    (
        _In_ ULONG                  _Pin,
        _In_ PCMiniportWaveRTStream _Stream
    );
    
    NTSTATUS StreamClosed
    (
        _In_ ULONG _Pin,
        _In_ PCMiniportWaveRTStream _Stream
    );
    
    NTSTATUS IsFormatSupported
    ( 
        _In_ ULONG          _ulPin, 
        _In_ BOOLEAN        _bCapture,
        _In_ PKSDATAFORMAT  _pDataFormat
    );

    static NTSTATUS GetAttributesFromAttributeList
    (
        _In_ const KSMULTIPLE_ITEM *_pAttributes,
        _In_ size_t _Size,
        _Out_ GUID* _pSignalProcessingMode
    );

public:
    DECLARE_STD_UNKNOWN();

#pragma code_seg("PAGE")
    CMiniportWaveRT(
        _In_            PUNKNOWN                                UnknownAdapter,
        _In_            PENDPOINT_MINIPAIR                      MiniportPair,
        _In_opt_        PVOID                                   DeviceContext
        )
        :CUnknown(0),
        m_DeviceType(MiniportPair->DeviceType),
        m_DeviceContext(DeviceContext), 
        m_DeviceMaxChannels(MiniportPair->DeviceMaxChannels),
        m_DeviceFormatsAndModes(MiniportPair->PinDeviceFormatsAndModes),
        m_DeviceFormatsAndModesCount(MiniportPair->PinDeviceFormatsAndModesCount),
        m_DeviceFlags(MiniportPair->DeviceFlags),
        m_pMiniportPair(MiniportPair)
    {
        PAGED_CODE();

        m_pAdapterCommon = (PADAPTERCOMMON)UnknownAdapter; // weak ref.
        
        if (MiniportPair->WaveDescriptor)
        {
            RtlCopyMemory(&m_FilterDesc, MiniportPair->WaveDescriptor, sizeof(m_FilterDesc));
        }
        
    }

#pragma code_seg()

    ~CMiniportWaveRT();

    IMP_IMiniportWaveRT;
    
    
    // Friends
    friend class        CMiniportWaveRTStream;
    friend class        CMiniportTopologySimple;
    
    friend NTSTATUS PropertyHandler_WaveFilter
    (   
        _In_ PPCPROPERTY_REQUEST      PropertyRequest 
    );   

public:


    NTSTATUS PropertyHandlerEffectListRequest
    (
        _In_ PPCPROPERTY_REQUEST PropertyRequest
    );    

    NTSTATUS PropertyHandlerProposedFormat
    (
        _In_ PPCPROPERTY_REQUEST PropertyRequest
    );

    NTSTATUS PropertyHandlerProposedFormat2
    (
        _In_ PPCPROPERTY_REQUEST PropertyRequest
    );

    PADAPTERCOMMON GetAdapterCommObj() 
    {
        return m_pAdapterCommon; 
    };
#pragma code_seg()



    //---------------------------------------------------------------------------------------------------------
    // volume
    //---------------------------------------------------------------------------------------------------------
    NTSTATUS GetVolumeChannelCount
    (
        _Out_  UINT32 * pulChannelCount
    );
    
    NTSTATUS GetVolumeSteppings
    (
        _Out_writes_bytes_(_ui32DataSize)  PKSPROPERTY_STEPPING_LONG _pKsPropStepLong, 
        _In_  UINT32    _ui32DataSize
    );
    
    NTSTATUS GetChannelVolume
    (
        _In_  UINT32    _uiChannel, 
        _Out_  LONG *   _pVolume
    );
    
    NTSTATUS SetChannelVolume
    (
        _In_  UINT32    _uiChannel, 
        _In_  LONG      _Volume
    );

private:
#pragma code_seg("PAGE")
    //---------------------------------------------------------------------------
    // GetPinSupportedDeviceFormats 
    //
    //  Return supported formats for a given pin.
    //
    //  Return value
    //      The number of KSDATAFORMAT_WAVEFORMATEXTENSIBLE items.
    //
    //  Remarks
    //      Supported formats index array follows same order as filter's pin
    //      descriptor list.
    //
    _Post_satisfies_(return > 0)
    ULONG GetPinSupportedDeviceFormats(_In_ ULONG PinId, _Outptr_opt_result_buffer_(return) KSDATAFORMAT_WAVEFORMATEXTENSIBLE **ppFormats)
    {
        PAGED_CODE();

        ASSERT(m_DeviceFormatsAndModesCount > PinId);
        ASSERT(m_DeviceFormatsAndModes[PinId].WaveFormats != NULL);
        ASSERT(m_DeviceFormatsAndModes[PinId].WaveFormatsCount > 0);

        if (ppFormats != NULL)
        {
            *ppFormats = m_DeviceFormatsAndModes[PinId].WaveFormats;
        }
        
        return m_DeviceFormatsAndModes[PinId].WaveFormatsCount;
    }

    
    //---------------------------------------------------------------------------
    // GetPinSupportedDeviceModes 
    //
    //  Return mode information for a given pin.
    //
    //  Return value
    //      The number of MODE_AND_DEFAULT_FORMAT items or 0 if none.
    //
    //  Remarks
    //      Supported formats index array follows same order as filter's pin
    //      descriptor list.
    //
    _Success_(return != 0)
    ULONG GetPinSupportedDeviceModes(_In_ ULONG PinId, _Outptr_opt_result_buffer_(return) _On_failure_(_Deref_post_null_) MODE_AND_DEFAULT_FORMAT **ppModes)
    {
        PMODE_AND_DEFAULT_FORMAT modes;
        ULONG numModes;

        PAGED_CODE();

        ASSERT(m_DeviceFormatsAndModesCount > PinId);
        ASSERT((m_DeviceFormatsAndModes[PinId].ModeAndDefaultFormatCount == 0) == (m_DeviceFormatsAndModes[PinId].ModeAndDefaultFormat == NULL));

        modes = m_DeviceFormatsAndModes[PinId].ModeAndDefaultFormat;
        numModes = m_DeviceFormatsAndModes[PinId].ModeAndDefaultFormatCount;


        if (ppModes != NULL)
        {
            if (numModes > 0)
            {
                *ppModes = modes;
            }
            else
            {
                // ensure that the returned pointer is NULL
                // in the event of failure (SAL annotation above
                // indicates that it must be NULL, and OACR sees a possibility
                // that it might not be).
                *ppModes = NULL;
            }
        }

        return numModes;
    }
#pragma code_seg()

protected:
#pragma code_seg("PAGE")
    BOOL IsRenderDevice()
    {
        PAGED_CODE();
        return FALSE;
    }

    BOOL IsOffloadSupported()
    {
        PAGED_CODE();
        return FALSE;
    }

    BOOL IsSystemCapturePin(ULONG nPinId)
    {
        PINTYPE pinType = m_DeviceFormatsAndModes[nPinId].PinType;
        PAGED_CODE();
        return (pinType == SystemCapturePin);
    }


    BOOL IsBridgePin(ULONG nPinId)
    {
        PAGED_CODE();
        return (m_DeviceFormatsAndModes[nPinId].PinType == BridgePin);
    }


#pragma code_seg()


};
typedef CMiniportWaveRT *PCMiniportWaveRT;

#endif // _SYSVAD_MINWAVERT_H_

