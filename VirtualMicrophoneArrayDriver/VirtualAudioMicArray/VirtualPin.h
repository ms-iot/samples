//
// Copyright (C) Microsoft Corporation. All rights reserved.
//
#pragma once

#define MS_TO_100NS(x)  ((x) * 10LL * 1000)

class VirtualPin;
class CMiniportWaveRTStream;

class InputPin
{
public:

    KsPin* m_pKsPin;
    UNICODE_STRING m_DeviceName;
    BOOLEAN m_bPresent;
    ULONG m_LastReadPosition;


    ~InputPin()
    {   
        if (m_pKsPin != NULL)
        {
            delete m_pKsPin;
        }
    }

    InputPin(_In_ VirtualPin* virtualPin, _In_ PUNICODE_STRING name)
    {
        RtAudioPin* rtAudioPin = NULL;
        UsbAudioPin* usbPin = NULL;
        KsPin* pin = NULL;
        WCHAR* buffer = NULL;
        
        const WCHAR usbDeviceString[] = L"\\??\\USB#";

        m_bPresent = FALSE;
        m_LastReadPosition = 0;

        buffer = (WCHAR*) ExAllocatePoolWithTag(PagedPool, name->MaximumLength, 'mNpV');
        

        if (buffer != NULL)
        {
            RtlCopyMemory(buffer, name->Buffer, name->MaximumLength);
            RtlInitUnicodeString(&m_DeviceName, buffer);
            
            if (RtlCompareMemory(usbDeviceString, name->Buffer, sizeof(usbDeviceString) - sizeof(WCHAR)) == sizeof(usbDeviceString) - sizeof(WCHAR))
            {
                usbPin = new UsbAudioPin(virtualPin);
                pin = reinterpret_cast<KsPin*>(usbPin);
            }
            else
            {
                rtAudioPin = new RtAudioPin(virtualPin);
                pin = reinterpret_cast<KsPin*>(rtAudioPin);
            }

            m_pKsPin = pin;
        }
    }

    PBYTE GetReadPosition() { return m_pKsPin->GetDmaBuffer() + m_LastReadPosition; }
    void UpdateReadPosition(ULONG bytes) 
    { 
        m_LastReadPosition += bytes; 
        ASSERT(m_LastReadPosition <= m_pKsPin->GetBufferSize());
        if (m_LastReadPosition == m_pKsPin->GetBufferSize())
        {
            m_LastReadPosition = 0;
        }
    }
    void ResetReadPosition() { m_LastReadPosition = 0; }

    ULONG GetCurrentPadding()
    {
        return m_pKsPin->GetPadding(m_LastReadPosition);
    }

    void* operator new(size_t t, ULONG tag = 'nPiV')
    {
        return ExAllocatePoolWithTag(NonPagedPoolNx, t, tag);
    }
    void operator delete(_In_ void* Goner) { ExFreePool(Goner); }
};

class VirtualPin
{
protected:
    ULONG m_cInputPins;
    ULONG m_Rate;
    ULONG m_NumChannels;
    ULONG m_ValidBitsPerSample;
    ULONG m_BytesPerSample;
    KSPIN_LOCK m_Lock;
    CMiniportWaveRTStream* m_pWaveRtStream;
    InputPin** m_rgInputPins;
    KTIMER     m_Timer;
    KDPC       m_Dpc;

    NTSTATUS CreateInputPin(_In_ PUNICODE_STRING deviceInterface);
    void InterleaveSamples(ULONG frameCount);
    static KDEFERRED_ROUTINE TimerDpcRoutine;
    void DoDpcWork();

public:
    VirtualPin();
    ~VirtualPin();

    NTSTATUS Initialize(__nullnullterminated WCHAR* inputPinNamesString, ULONG rate, ULONG numChannels, ULONG validBitsPerSample);
    void ProcessSamples();

    NTSTATUS HandleInputPinArrival(_In_ PUNICODE_STRING deviceInterface);
    NTSTATUS HandleInputPinRemoval(_In_ PUNICODE_STRING deviceInterface);

    NTSTATUS RegisterStream(_In_ CMiniportWaveRTStream* Stream);
    NTSTATUS UnregisterStream(_In_ CMiniportWaveRTStream* Stream);

    NTSTATUS StartDma();
    NTSTATUS PauseDma();
    NTSTATUS StopDma();
};
