/*++

Copyright (c) Microsoft Corporation All Rights Reserved

Module Name:

    micarraywavtable.h

Abstract:

    Declaration of wave miniport tables for the mic array.

--*/

#ifndef _SYSVAD_MICARRAYWAVTABLE_H_
#define _SYSVAD_MICARRAYWAVTABLE_H_

//
// Mic array range.
//

#define MICARRAY_DEVICE_MAX_CHANNELS            2       // Max channels overall
#define MICARRAY_MIN_BITS_PER_SAMPLE_PCM        16      // Min Bits Per Sample
#define MICARRAY_MAX_BITS_PER_SAMPLE_PCM        16      // Max Bits Per Sample
#define MICARRAY_RAW_SAMPLE_RATE                44100   // Raw sample rate
#define MICARRAY_MIN_SAMPLE_RATE                44100    // Min Sample Rate
#define MICARRAY_MAX_SAMPLE_RATE                44100   // Max Sample Rate

//=============================================================================
static 
KSDATAFORMAT_WAVEFORMATEXTENSIBLE MicArrayPinSupportedDeviceFormats[] =
{
    // 0 - Note the ENDPOINT_MINIPAIR structures for the mic arrays use this first element as the proposed DEFAULT format
    // 44.1 KHz 16-bit mono
    {
        {
            sizeof(KSDATAFORMAT_WAVEFORMATEXTENSIBLE),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        {
            {
                WAVE_FORMAT_EXTENSIBLE,
                2,
                44100,
                88200*2,
                4,
                16,
                sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX)
            },
            16,
            KSAUDIO_SPEAKER_STEREO,
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM)
        }
    },
};

//
// Supported modes (only on streaming pins).
//
static
MODE_AND_DEFAULT_FORMAT MicArrayPinSupportedDeviceModes[] =
{
    {
        STATIC_AUDIO_SIGNALPROCESSINGMODE_RAW,
        &MicArrayPinSupportedDeviceFormats[SIZEOF_ARRAY(MicArrayPinSupportedDeviceFormats)-1].DataFormat
    },
};


//
// The entries here must follow the same order as the filter's pin
// descriptor array.
//
static 
PIN_DEVICE_FORMATS_AND_MODES MicArrayPinDeviceFormatsAndModes[] = 
{
    {
        BridgePin,
        NULL,
        0,
        NULL,
        0
    },
    {
        SystemCapturePin,
        MicArrayPinSupportedDeviceFormats,
        SIZEOF_ARRAY(MicArrayPinSupportedDeviceFormats),
        MicArrayPinSupportedDeviceModes,
        SIZEOF_ARRAY(MicArrayPinSupportedDeviceModes)
    },
};

//=============================================================================
static
KSDATARANGE MicArrayPinDataRangesBridge[] =
{
    {
        sizeof(KSDATARANGE),
        0,
        0,
        0,
        STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
        STATICGUIDOF(KSDATAFORMAT_SUBTYPE_ANALOG),
        STATICGUIDOF(KSDATAFORMAT_SPECIFIER_NONE)
    }
};

static
PKSDATARANGE MicArrayPinDataRangePointersBridge[] =
{
    &MicArrayPinDataRangesBridge[0]
};

//=============================================================================
static
KSDATARANGE_AUDIO MicArrayPinDataRangesStream[] =
{
    { // 0
        {
            sizeof(KSDATARANGE_AUDIO),
            KSDATARANGE_ATTRIBUTES,         // An attributes list follows this data range
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_PCM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        MICARRAY_DEVICE_MAX_CHANNELS,           
        MICARRAY_MIN_BITS_PER_SAMPLE_PCM,
        MICARRAY_MAX_BITS_PER_SAMPLE_PCM,    
        MICARRAY_MIN_SAMPLE_RATE,            
        MICARRAY_MAX_SAMPLE_RATE             
    },
};

static
PKSDATARANGE MicArrayPinDataRangePointersStream[] =
{
    PKSDATARANGE(&MicArrayPinDataRangesStream[0]),
    PKSDATARANGE(&PinDataRangeAttributeList)
};

//=============================================================================
static
PCPIN_DESCRIPTOR MicArrayWaveMiniportPins[] =
{
    // Wave In Bridge Pin (Capture - From Topology) KSPIN_WAVE_BRIDGE
    {
        0,
        0,
        0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(MicArrayPinDataRangePointersBridge),
            MicArrayPinDataRangePointersBridge,
            KSPIN_DATAFLOW_IN,
            KSPIN_COMMUNICATION_NONE,
            &KSCATEGORY_AUDIO,
            NULL,
            0
        }
    },
    // Wave In Streaming Pin (Capture) KSPIN_WAVE_HOST
    {
        MAX_INPUT_SYSTEM_STREAMS,
        MAX_INPUT_SYSTEM_STREAMS,
        0,
        NULL,
        {
            0,
            NULL,
            0,
            NULL,
            SIZEOF_ARRAY(MicArrayPinDataRangePointersStream),
            MicArrayPinDataRangePointersStream,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_SINK,
            &KSCATEGORY_AUDIO,
            &KSAUDFNAME_RECORDING_CONTROL,  
            0
        }
    },
};

//=============================================================================
static
PCNODE_DESCRIPTOR MicArrayWaveMiniportNodes[] =
{
    // KSNODE_WAVE_ADC
    {
        0,                      // Flags
        NULL,                   // AutomationTable
        &KSNODETYPE_ADC,        // Type
        NULL                    // Name
    }
};

//=============================================================================
static
PCCONNECTION_DESCRIPTOR MicArrayWaveMiniportConnections[] =
{
    { PCFILTER_NODE,        KSPIN_WAVE_BRIDGE,      KSNODE_WAVE_ADC,     1 },    
    { KSNODE_WAVE_ADC,      0,                      PCFILTER_NODE,       KSPIN_WAVEIN_HOST },
};

//=============================================================================
DECLARE_CLASSPROPERTYHANDLER(CMiniportWaveRT, Get_SoundDetectorSupportedPatterns);
DECLARE_CLASSPROPERTYHANDLER(CMiniportWaveRT, Set_SoundDetectorPatterns);
DECLARE_CLASSPROPERTYHANDLER(CMiniportWaveRT, Get_SoundDetectorArmed);
DECLARE_CLASSPROPERTYHANDLER(CMiniportWaveRT, Set_SoundDetectorArmed);
DECLARE_CLASSPROPERTYHANDLER(CMiniportWaveRT, Get_SoundDetectorMatchResult);

static
SYSVADPROPERTY_ITEM PropertiesMicArrayWaveFilter[] =
{
    {
        {
            &KSPROPSETID_General,
            KSPROPERTY_GENERAL_COMPONENTID,
            KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
            PropertyHandler_WaveFilter,
        },
        0,
        0,
        NULL,
        NULL,
        NULL
    },
    {
        {
            &KSPROPSETID_Pin,
            KSPROPERTY_PIN_PROPOSEDATAFORMAT,
            KSPROPERTY_TYPE_SET | KSPROPERTY_TYPE_BASICSUPPORT,
            PropertyHandler_WaveFilter,
        },
        0,
        0,
        NULL,
        NULL,
        NULL
    },
    {
        {
            &KSPROPSETID_Pin,
            KSPROPERTY_PIN_PROPOSEDATAFORMAT2,
            KSPROPERTY_TYPE_GET | KSPROPERTY_TYPE_BASICSUPPORT,
            PropertyHandler_WaveFilter,
        },
        0,
        0,
        NULL,
        NULL,
        NULL
    },

};


//=============================================================================
static
PCFILTER_DESCRIPTOR MicArrayWaveMiniportFilterDescriptor =
{
    0,                                              // Version
    NULL,                                           // AutomationTable
    sizeof(PCPIN_DESCRIPTOR),                       // PinSize
    SIZEOF_ARRAY(MicArrayWaveMiniportPins),         // PinCount
    MicArrayWaveMiniportPins,                       // Pins
    sizeof(PCNODE_DESCRIPTOR),                      // NodeSize
    SIZEOF_ARRAY(MicArrayWaveMiniportNodes),        // NodeCount
    MicArrayWaveMiniportNodes,                      // Nodes
    SIZEOF_ARRAY(MicArrayWaveMiniportConnections),  // ConnectionCount
    MicArrayWaveMiniportConnections,                // Connections
    0,                                              // CategoryCount
    NULL                                            // Categories  - use defaults (audio, render, capture)
};

#endif // _SYSVAD_MICARRAYWAVTABLE_H_
