//
// Copyright (C) Microsoft Corporation. All rights reserved.
//

#include "VirtualAudioMicArray.h"
#include "minwavertstream.h"
#if defined(_M_ARM)
#include <arm_neon.h>
#endif

VirtualPin::VirtualPin() :
    m_cInputPins(0),
    m_Rate(0),
    m_NumChannels(0),
    m_ValidBitsPerSample(0),
    m_BytesPerSample(0),
    m_rgInputPins(NULL)
{
    KeInitializeSpinLock(&m_Lock);
    KeInitializeDpc(&m_Dpc, &TimerDpcRoutine, this);
    KeInitializeTimer(&m_Timer);
}


NTSTATUS VirtualPin::Initialize
(
    __nullnullterminated WCHAR* inputPinNamesString, 
    ULONG rate, 
    ULONG numChannels, 
    ULONG validBitsPerSample
)
{
    WCHAR* currentChar = inputPinNamesString;
    WCHAR* startChar = inputPinNamesString;
    NTSTATUS status = STATUS_SUCCESS;
    UNICODE_STRING unicodeString;
    BOOLEAN done = FALSE;
    
    m_Rate = rate;
    m_NumChannels = numChannels;
    m_ValidBitsPerSample = validBitsPerSample;
    m_BytesPerSample = validBitsPerSample == 24 ? 4 : validBitsPerSample / 8;

    m_rgInputPins = (InputPin**) ExAllocatePoolWithTag(NonPagedPoolNx, numChannels * sizeof(InputPin*), 'nPnI');

    if (m_rgInputPins == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    //
    // Parse the multi SZ input string and create InputPins for each device listed. 
    //
    while (done != TRUE)
    {
        if (*currentChar == L'\0')
        {
            RtlInitUnicodeString(&unicodeString, startChar);
            status = CreateInputPin(&unicodeString);
            ASSERT(NT_SUCCESS(status));
            if (!NT_SUCCESS(status))
            {
                break;
            }

            startChar = currentChar + 1;

            if (*startChar == L'\0')
            {
                done = TRUE;
                break;
            }
        }
        
        currentChar++;
    }

    ASSERT(m_cInputPins == numChannels);

    return status;
}



VirtualPin::~VirtualPin()
{
    ULONG i;
    KIRQL oldIrql;
    ULONG cPins = m_cInputPins;
    InputPin** pins = NULL;

    if (KeCancelTimer(&m_Timer) == TRUE)
    {
        KeFlushQueuedDpcs();
    }

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    pins = m_rgInputPins;
    m_cInputPins = 0;
    m_rgInputPins = NULL;

    KeReleaseSpinLock(&m_Lock, oldIrql);

    for (i=0; i<cPins; i++)
    {
        delete pins[i];
        pins[i] = NULL;
    }
    ExFreePool(pins);
}

NTSTATUS VirtualPin::HandleInputPinArrival(_In_ PUNICODE_STRING deviceInterface)
{
    KIRQL oldIrql;
    BOOLEAN found = FALSE;
    ULONG i;
    UNICODE_STRING pinString, devInterfaceString;

    RtlInitUnicodeString(&devInterfaceString, deviceInterface->Buffer);

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    for (i = 0; i<m_cInputPins; i++)
    {
        pinString = m_rgInputPins[i]->m_DeviceName;
        KeReleaseSpinLock(&m_Lock, oldIrql);

        if (RtlCompareUnicodeString(&pinString, &devInterfaceString, TRUE) == 0)
        {
            found = TRUE;
            m_rgInputPins[i]->m_bPresent = TRUE;
        }
        
        KeAcquireSpinLock(&m_Lock, &oldIrql);
    }
    KeReleaseSpinLock(&m_Lock, oldIrql);

    return STATUS_SUCCESS;
}

NTSTATUS VirtualPin::HandleInputPinRemoval(_In_ PUNICODE_STRING deviceInterface)
{
    KIRQL oldIrql;
    BOOLEAN found = FALSE;
    ULONG i;
    InputPin* pin = NULL;
    UNICODE_STRING pinString, devInterfaceString;

    RtlInitUnicodeString(&devInterfaceString, deviceInterface->Buffer);

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    for (i = 0; i<m_cInputPins; i++)
    {
        pin = m_rgInputPins[i];
        pinString = pin->m_DeviceName;
        KeReleaseSpinLock(&m_Lock, oldIrql);

        if (RtlCompareUnicodeString(&pinString, &devInterfaceString, TRUE) == 0)
        {
            found = TRUE;
            pin->m_bPresent = FALSE;
        }
        KeAcquireSpinLock(&m_Lock, &oldIrql);
    }
    KeReleaseSpinLock(&m_Lock, oldIrql);

    if (found == TRUE)
    {
        pin->m_pKsPin->ClosePin();
    }

    return STATUS_SUCCESS;
}

NTSTATUS VirtualPin::CreateInputPin(_In_ PUNICODE_STRING deviceInterface)
{
    KIRQL oldIrql;

    InputPin* pin = new InputPin(this, deviceInterface);

	if (pin == NULL)
	{
		return STATUS_NO_MEMORY;
	}

    KeAcquireSpinLock(&m_Lock, &oldIrql);
    
    m_rgInputPins[m_cInputPins++] = pin;
    
    KeReleaseSpinLock(&m_Lock, oldIrql);

    return STATUS_SUCCESS;
}

#if 0

// The following is an example of an interleave function optimized for the neon instruction set and a two channel array.
// It has roughly 2x better performance than the default version.
void VirtualPin::InterleaveSamplesForTwoChannel16BitMicArray(ULONG frameCount)
{
	ULONG i,j, originalFrameCount = frameCount;
	KSAUDIO_POSITION position;
	BYTE* pDestination;
	int16x4_t input0, input1;
	int16x4x2_t result;
	ULONG macroFramesToProcess = frameCount >> 2;

	ASSERT(m_ValidBitsPerSample == 16);
	ASSERT(m_NumChannels == 2);

	m_pWaveRtStream->GetPosition(&position);
	pDestination = m_pWaveRtStream->GetDmaBuffer() + position.WriteOffset;
	for (i = 0; i < macroFramesToProcess; i++)
	{
		input0 = vld1_s16((SHORT*)m_rgInputPins[0]->GetReadPosition());
		input1 = vld1_s16((SHORT*)m_rgInputPins[1]->GetReadPosition());
		result = vzip_s16(input0, input1);
		vst1_s16((SHORT*) pDestination, result.val[0]);
		pDestination += sizeof(result.val[0]);
		vst1_s16((SHORT*)pDestination, result.val[1]);
		pDestination += sizeof(result.val[1]);

		m_rgInputPins[0]->UpdateReadPosition(4*m_BytesPerSample);
		m_rgInputPins[1]->UpdateReadPosition(4*m_BytesPerSample);
		
	}

	frameCount -= (macroFramesToProcess << 2);
	
	for (i = 0; i < frameCount; i++)
	{
		for (j = 0; j < m_cInputPins; j++)
		{
			RtlCopyMemory(pDestination, m_rgInputPins[j]->GetReadPosition(), m_BytesPerSample);
			m_rgInputPins[j]->UpdateReadPosition(m_BytesPerSample);
			pDestination += m_BytesPerSample;
		}
	}
	
	m_pWaveRtStream->UpdateVirtualPositionRegisters(originalFrameCount);
}

#else

void VirtualPin::InterleaveSamples(ULONG frameCount)
{
    ULONG i,j;
    KSAUDIO_POSITION position;
    BYTE* pDestination;

    m_pWaveRtStream->GetPosition(&position);
    pDestination = m_pWaveRtStream->GetDmaBuffer() + position.WriteOffset;

    for (i = 0; i < frameCount; i++)
    {
        for (j = 0; j < m_cInputPins; j++)
        {
            RtlCopyMemory(pDestination, m_rgInputPins[j]->GetReadPosition(), m_BytesPerSample);
            m_rgInputPins[j]->UpdateReadPosition(m_BytesPerSample);
            pDestination += m_BytesPerSample;
        }
    }
    m_pWaveRtStream->UpdateVirtualPositionRegisters(frameCount);
}

#endif

void VirtualPin::ProcessSamples()
{
    ULONG framesToProcess = 0;
    KIRQL oldIrql;
    ULONG i;

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    do 
    {
        if (m_pWaveRtStream != NULL)
        {
            framesToProcess = m_pWaveRtStream->GetPaddingToEndOfBuffer();
            for (i = 0; i < m_cInputPins; i++)
            {
                framesToProcess = min(framesToProcess, m_rgInputPins[i]->GetCurrentPadding());
            }

            if (framesToProcess != 0)
            {
                InterleaveSamples(framesToProcess);
            }
        }
    } while (framesToProcess != 0);

    KeReleaseSpinLock(&m_Lock, oldIrql);
}

NTSTATUS
VirtualPin::RegisterStream
(
    _In_        CMiniportWaveRTStream* stream
)
{
    KIRQL oldIrql;
    ULONG i;
    ULONG inputPinSize = stream->GetDmaBufferSize() >> 1;  
    NTSTATUS status = STATUS_SUCCESS;
    InputPin* pin;

    ASSERT(m_pWaveRtStream == NULL);

    KeAcquireSpinLock(&m_Lock, &oldIrql);
    
    m_pWaveRtStream = stream;

    for (i = 0; NT_SUCCESS(status) && i < m_cInputPins; i++)
    {
        pin = m_rgInputPins[i];

        KeReleaseSpinLock(&m_Lock, oldIrql);

        status = pin->m_pKsPin->OpenPin(&pin->m_DeviceName, m_Rate, 1, m_ValidBitsPerSample);

        ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status))
        {
            status = pin->m_pKsPin->InitRtBuffer(inputPinSize);
            ASSERT(NT_SUCCESS(status));
        }

        KeAcquireSpinLock(&m_Lock, &oldIrql);
    }

    KeReleaseSpinLock(&m_Lock, oldIrql);

    return STATUS_SUCCESS;
}

NTSTATUS
VirtualPin::UnregisterStream
(
    _In_        CMiniportWaveRTStream* stream
)
{
    ULONG i;
    InputPin* pin = NULL;
    KIRQL oldIrql;
    NTSTATUS status = STATUS_SUCCESS;
    UNREFERENCED_PARAMETER(stream);

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    ASSERT(m_pWaveRtStream == stream);

    m_pWaveRtStream = NULL;

    for (i = 0; NT_SUCCESS(status) && i < m_cInputPins; i++)
    {
        pin = m_rgInputPins[i];

        KeReleaseSpinLock(&m_Lock, oldIrql);

        status = pin->m_pKsPin->ClosePin();

        ASSERT(NT_SUCCESS(status));

        KeAcquireSpinLock(&m_Lock, &oldIrql);
    }

    KeReleaseSpinLock(&m_Lock, oldIrql);

    return STATUS_SUCCESS;
}

NTSTATUS VirtualPin::StartDma()
{
    InputPin* pin = NULL;
    ULONG i;
    KIRQL oldIrql;
    NTSTATUS status = STATUS_SUCCESS;
    LARGE_INTEGER dueTime;

    dueTime.QuadPart = -1 * MS_TO_100NS(5);

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    for (i=0; NT_SUCCESS(status) && i<m_cInputPins; i++)
    {
        pin = m_rgInputPins[i];

        KeReleaseSpinLock(&m_Lock, oldIrql);

        status = pin->m_pKsPin->SetState(TRUE);

        KeAcquireSpinLock(&m_Lock, &oldIrql);
    }

    KeSetTimerEx(&m_Timer, dueTime, 5 /* ms */, &m_Dpc);

    KeReleaseSpinLock(&m_Lock, oldIrql);

    return status;
}

NTSTATUS VirtualPin::PauseDma()
{
    InputPin* pin = NULL;
    ULONG i;
    KIRQL oldIrql;
    NTSTATUS status = STATUS_SUCCESS;

    KeAcquireSpinLock(&m_Lock, &oldIrql);

    for (i = 0; NT_SUCCESS(status) && i<m_cInputPins; i++)
    {
        pin = m_rgInputPins[i];

        KeReleaseSpinLock(&m_Lock, oldIrql);

        status = pin->m_pKsPin->SetState(FALSE);

        KeAcquireSpinLock(&m_Lock, &oldIrql);
    }

    KeCancelTimer(&m_Timer);

    KeReleaseSpinLock(&m_Lock, oldIrql);

    return status;
}

NTSTATUS VirtualPin::StopDma()
{
    return PauseDma();
}

VOID VirtualPin::TimerDpcRoutine
(
    _In_     struct _KDPC *Dpc,
    _In_opt_ PVOID        DeferredContext,
    _In_opt_ PVOID        SystemArgument1,
    _In_opt_ PVOID        SystemArgument2
)
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

    VirtualPin* me = (VirtualPin*) DeferredContext;
    me->DoDpcWork();
}

void VirtualPin::DoDpcWork()
{
    ProcessSamples();
}
