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
using namespace Windows::Devices::I2c;


/**
* Controls debug output for this module
*/
#ifndef NDEBUG
extern uint8_t dbgGPIO;
#endif

IInspectable* AddRefAsInspectable(Object^ o)
{
    IInspectable* obj = reinterpret_cast<IInspectable*>(o);
    obj->AddRef();
    return obj;
}

Object^ AsObjectRef(void *o)
{
    IInspectable* pO = reinterpret_cast<IInspectable *>(o);
    return reinterpret_cast<Object ^>(pO);
}

Object^ ReleaseAsObjectRef(void *o)
{
    IInspectable* pO = reinterpret_cast<IInspectable *>(o);
    Object^ obj = reinterpret_cast<Object ^>(pO);
    pO->Release();
    return obj;
}

AJ_Status AJS_TargetIO_I2cOpen(uint8_t sda, uint8_t scl, uint32_t clock, uint8_t mode, uint8_t ownAddress, void** ctx)
{
    String^ i2cAqs = nullptr;
    const AJS_IO_Info* info = AJS_TargetIO_GetInfo(sda);
    if (info == nullptr) {
        AJ_ErrPrintf(("AJS_TargetIO_I2cOpen(): invalid sda pin (%d)\n", sda));
        return AJ_ERR_INVALID;
    }
    std::string schematicId(info->schematicId);
    if (schematicId.length() > 0) {
        i2cAqs = I2cDevice::GetDeviceSelector(ref new String(std::wstring(schematicId.begin(), schematicId.end()).c_str()));
    } else {
        i2cAqs = I2cDevice::GetDeviceSelector();
    }

    auto task = create_task(DeviceInformation::FindAllAsync(i2cAqs));
    auto c1 = task.then([clock, ownAddress, ctx](DeviceInformationCollection^ deviceInfos)
    {
        String^ deviceId = deviceInfos->GetAt(0)->Id;
        I2cConnectionSettings^ settings = ref new I2cConnectionSettings(ownAddress);
        if (clock >= 400000) {
            settings->BusSpeed = I2cBusSpeed::FastMode;
        } else {
            settings->BusSpeed = I2cBusSpeed::StandardMode;
        }
        settings->SharingMode = I2cSharingMode::Shared;

        auto task2 = create_task(I2cDevice::FromIdAsync(deviceId, settings));
        auto c2 = task2.then([ctx](I2cDevice^ i2cDevice) {
            *ctx = AddRefAsInspectable(i2cDevice);
        });

        try {
            task2.wait();
            c2.wait();
        } catch (...) {
            AJ_ErrPrintf(("AJS_TargetIO_I2cOpen(): invalid device settings:\n"));
            AJ_ErrPrintf((" address=%d\n", ownAddress));
            AJ_ErrPrintf((" BusSpeed=%d\n", settings->BusSpeed));
            throw;
        }
    });

    try {
        task.wait();
        c1.wait();
    } catch (...) {
        AJ_ErrPrintf(("AJS_TargetIO_I2cOpen(): invalid id (%s)\n", info->schematicId));
        return AJ_ERR_DRIVER;
    }

    return AJ_OK;
}

AJ_Status AJS_TargetIO_I2cClose(void* ctx)
{
    I2cDevice^ i2cDevice = reinterpret_cast<I2cDevice^>(ReleaseAsObjectRef(ctx));
    i2cDevice = nullptr;
    return AJ_OK;
}

AJ_Status AJS_TargetIO_I2cTransfer(void* ctx, uint8_t addr, uint8_t* txBuf, uint8_t txLen, uint8_t* rxBuf, uint8_t rxLen, uint8_t* rxBytes)
{
    I2cDevice^ i2cDevice = reinterpret_cast<I2cDevice^>(AsObjectRef(ctx));
    Array<uint8_t>^ writeBuffer = nullptr;
    WriteOnlyArray<uint8_t, 1U>^ readBuffer = nullptr;

    if (txLen > 0) {
        writeBuffer = ArrayReference<uint8_t>(txBuf, txLen);
    }
    if (rxLen > 0) {
        readBuffer = ArrayReference<uint8_t, 1U>(rxBuf, rxLen);
    }

    try {
        if (txLen > 0) {
            if (rxLen > 0) {
                i2cDevice->WriteRead(writeBuffer, readBuffer);
            } else {
                i2cDevice->Write(writeBuffer);
            }
        } else if (rxLen > 0) {
            i2cDevice->Read(readBuffer);
        }
    } catch (...) {
        AJ_ErrPrintf(("AJS_TargetIO_I2cTransfer(): i2C read/write failed\n"));
        return AJ_ERR_FAILURE;
    }

    if (rxLen > 0 && rxBytes != NULL) {
        *rxBytes = readBuffer->Length;
    }

    return AJ_OK;
}