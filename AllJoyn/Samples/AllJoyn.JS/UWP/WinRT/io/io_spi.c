/**
 * @file
 */
/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

#define AJ_MODULE GPIO

#include <Windows.h>
#include <ppltasks.h>

#include "ajs.h"
#include "ajs_io.h"

using namespace concurrency;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Platform;

// The namespace is availalbe on Windows IoT Extensions Kit reference
using namespace Windows::Devices::Spi;


extern IInspectable* AddRefAsInspectable(Object^ o);
extern Object^ AsObjectRef(void *o);
extern Object^ ReleaseAsObjectRef(void *o);

/**
* Controls debug output for this module
*/
#ifndef NDEBUG
extern uint8_t dbgGPIO;
#endif

static int GetSpiChipSelectionId(uint16_t pin)
{
    extern uint8_t csTextLen;
    const AJS_IO_Info* info = AJS_TargetIO_GetInfo(pin);
    if (info != NULL) {
        if (info->datasheetId != NULL && strlen(info->datasheetId) > csTextLen) {
            char* endPtr;
            /*
            * Get Spi CS ID from datasheetId (i.e. "CS0" -> 0)
            */
            return strtol(info->datasheetId+ csTextLen, &endPtr, 10);
        }
    }
    return -1;
}

AJ_Status AJS_TargetIO_SpiOpen(uint8_t mosi, uint8_t miso, uint8_t cs, uint8_t clk, uint32_t clock,
    uint8_t master, uint8_t cpol, uint8_t cpha, uint8_t data, void** spiCtx)
{
    AJ_InfoPrintf(("AJS_TargetIO_SpiOpen(%d,%d,%d,%d,%d,%d,%d,%d)\n", mosi, miso, cs, clk, clock, cpol, cpha, data));

    AJ_Status status = AJ_OK;
    String^ spiAqs = nullptr;
    const AJS_IO_Info* info = AJS_TargetIO_GetInfo(mosi);
    if (info == nullptr) {
        AJ_ErrPrintf(("AJS_TargetIO_SpiOpen(): invalid mosi pin (%d)\n", mosi));
        return AJ_ERR_INVALID;
    }
    std::string schematicId(info->schematicId);

    if (schematicId.length() > 0) {
        spiAqs = SpiDevice::GetDeviceSelector(ref new String(std::wstring(schematicId.begin(), schematicId.end()).c_str()));
    } else {
        spiAqs = SpiDevice::GetDeviceSelector();
    }

    auto task = create_task(DeviceInformation::FindAllAsync(spiAqs));
    auto c1 = task.then([cs, clock, cpol, cpha, data, spiCtx, &status](DeviceInformationCollection^ deviceInfos) {
        String^ deviceId = deviceInfos->GetAt(0)->Id;
        int chipSelectionLine = GetSpiChipSelectionId(cs);
        if (chipSelectionLine < 0) {
            status = AJ_ERR_INVALID;
        } else {
            SpiConnectionSettings^ settings = ref new SpiConnectionSettings(chipSelectionLine);
            settings->ClockFrequency = clock;
            if (data > 0) {
                settings->DataBitLength = data;
            }
            settings->Mode = static_cast<SpiMode>((cpol << 1) | cpha);

            auto task2 = create_task(SpiDevice::FromIdAsync(deviceId, settings));
            auto c2 = task2.then([spiCtx](SpiDevice^ spiDevice) {
                *spiCtx = AddRefAsInspectable(spiDevice);
            });

            try {
                task2.wait();
                c2.wait();
            } catch (...) {
                AJ_ErrPrintf(("AJS_TargetIO_SpiOpen(): invalid device settings:\n"));
                AJ_ErrPrintf((" chipSelectionLine=%d\n", chipSelectionLine));
                AJ_ErrPrintf((" ClockFrequency=%d\n", settings->ClockFrequency));
                AJ_ErrPrintf((" DataBitLength=%d\n", settings->DataBitLength));
                AJ_ErrPrintf((" Mode=%d\n", settings->Mode));
                throw;
            }
        }
    });

    try {
        task.wait();
        c1.wait();
    } catch (...) {
        AJ_ErrPrintf(("AJS_TargetIO_SpiOpen(): invalid id (%s)\n", info->schematicId));
        status = AJ_ERR_DRIVER;
    }

    return status;
}

AJ_Status AJS_TargetIO_SpiClose(void* spiCtx)
{
    AJ_InfoPrintf(("AJS_TargetIO_SpiClose()\n"));
    SpiDevice^ spiDevice = reinterpret_cast<SpiDevice^>(ReleaseAsObjectRef(spiCtx));
    spiDevice = nullptr;
    return AJ_OK;
}

AJ_Status AJS_TargetIO_SpiRead(void* ctx, uint32_t length, uint8_t* buffer)
{
    AJ_InfoPrintf(("AJS_TargetIO_SpiRead(%d)\n", length));
    SpiDevice^ spiDevice = reinterpret_cast<SpiDevice^>(AsObjectRef(ctx));
    Array<unsigned char>^ readBuffer = ArrayReference<unsigned char>(buffer, length);
    spiDevice->Read(readBuffer);
    return AJ_OK;
}

void AJS_TargetIO_SpiWrite(void* ctx, uint8_t* data, uint32_t length)
{
    AJ_InfoPrintf(("AJS_TargetIO_SpiWrite(%d)\n", length));
    SpiDevice^ spiDevice = reinterpret_cast<SpiDevice^>(AsObjectRef(ctx));
    Array<unsigned char>^ writeBuffer = ArrayReference<unsigned char>(data, length);
    spiDevice->Write(writeBuffer);
}
