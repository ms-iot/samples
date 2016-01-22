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

#include "ajs.h"
#include "ajs_io.h"

using namespace Windows::Devices::Enumeration;
using namespace Windows::Foundation;
using namespace Platform;

// The namespace is availalbe on Windows IoT Extensions Kit reference
using namespace Windows::Devices::Gpio;


/**
 * Controls debug output for this module
 */
#ifndef NDEBUG
extern "C" {
    uint8_t dbgGPIO;
}
#endif

extern void AJ_Net_Interrupt();

#define MAX_TRIGGERS 32

/*
* Struct for holding the state of a GPIO pin
*/
typedef struct {
    GpioPin^ gpioPin;
    AJS_IO_PinTriggerCondition condition;
} GPIO;

static GPIO gpio[MAX_TRIGGERS] = { {nullptr, AJS_IO_PIN_TRIGGER_DISABLE} };


/*
 * Bit mask of allocated triggers (max 32)
 */
static uint32_t trigSet;

#define BIT_IS_SET(i, b)  ((i) & (1 << (b)))
#define BIT_SET(i, b)     ((i) |= (1 << (b)))
#define BIT_CLR(i, b)     ((i) &= ~(1 << (b)))

static int GetGpioPin(uint16_t pin)
{
    extern uint8_t gpioTextLen;
    const AJS_IO_Info* info = AJS_TargetIO_GetInfo(pin);
    if (info != NULL) {
        if (info->schematicId != NULL && strlen(info->schematicId) > gpioTextLen) {
            char* endPtr;
            /*
             * "GPIO1" -> 1
             */
            return strtol(info->schematicId+gpioTextLen, &endPtr, 10);
        }
    }
    return -1;
}

static void AJS_TargetIO_Pin_ValueChanged(GpioPin ^gpioPin, GpioPinValueChangedEventArgs ^args)
{
    uint16_t pin = gpioPin->PinNumber;
    if (pin >= MAX_TRIGGERS) {
        AJ_ErrPrintf(("AJS_TargetIO_Pin_ValueChanged(): invalid pin (%d)\n", pin));
        return;
    }

    bool setTrigger = false;
    if (args->Edge == GpioPinEdge::FallingEdge) {
        if (gpio[pin].condition & AJS_IO_PIN_TRIGGER_ON_FALL) {
            setTrigger = true;
        }
    } else if (args->Edge == GpioPinEdge::RisingEdge) {
        if (gpio[pin].condition & AJS_IO_PIN_TRIGGER_ON_RISE) {
            setTrigger = true;
        }
    }

    if (setTrigger) {
        AJ_InfoPrintf(("AJS_TargetIO_Pin_ValueChanged, set trigger(%d, %02x)\n", pin, args->Edge));
        BIT_SET(trigSet, pin);
        AJ_Net_Interrupt();
    }
}

AJ_Status AJS_TargetIO_PinOpen(uint16_t pin, AJS_IO_PinConfig config, void** pinCtx)
{
    AJ_InfoPrintf(("AJS_TargetIO_PinOpen(%d, %02x)\n", pin, config));

    GpioController^ controller = GpioController::GetDefault();
    if (controller == nullptr) {
        return AJ_ERR_DRIVER;
    }

    int gpioPinNumber = GetGpioPin(pin);
    if (gpioPinNumber < 0 || gpioPinNumber >= ArraySize(gpio)) {
        AJ_ErrPrintf(("AJS_TargetIO_PinOpen(): Invalid pin(%d), %d\n", pin, gpioPinNumber));
        return AJ_ERR_INVALID;
    }

    try {
        gpio[gpioPinNumber].gpioPin = controller->OpenPin(gpioPinNumber);
    } catch (...) {
        AJ_ErrPrintf(("AJS_TargetIO_PinOpen(): OpenPin(%d) failed\n", pin));
        return AJ_ERR_RESOURCES;
    }

    GpioPinDriveMode driveMode;
    switch (config) {
    case AJS_IO_PIN_INPUT:
        driveMode = GpioPinDriveMode::Input;
        break;
    case AJS_IO_PIN_OUTPUT:
        driveMode = GpioPinDriveMode::Output;
        break;
    case AJS_IO_PIN_OPEN_DRAIN:
        driveMode = GpioPinDriveMode::OutputOpenDrain;
        break;
    case AJS_IO_PIN_PULL_UP:
        driveMode = GpioPinDriveMode::InputPullUp;
        break;
    case AJS_IO_PIN_PULL_DOWN:
        driveMode = GpioPinDriveMode::InputPullDown;
        break;
    default:
        AJ_ErrPrintf(("AJS_TargetIO_PinOpen(): Invalid config %02x\n", config));
        break;
    }

    try {
        gpio[gpioPinNumber].gpioPin->SetDriveMode(driveMode);
    } catch (...) {
        AJ_ErrPrintf(("AJS_TargetIO_PinOpen(): SetDriveMode(%02x) failed\n", driveMode));
    }

    gpio[gpioPinNumber].gpioPin->ValueChanged += ref new TypedEventHandler<GpioPin ^, GpioPinValueChangedEventArgs ^>(&AJS_TargetIO_Pin_ValueChanged);

    *pinCtx = (void*)gpioPinNumber;
    return AJ_OK;
}

AJ_Status AJS_TargetIO_PinClose(void* pinCtx)
{
    int gpioPinNumber = (int)pinCtx;
    if (gpioPinNumber >= ArraySize(gpio)) {
        AJ_ErrPrintf(("AJS_TargetIO_PinClose(): invalid pin (%d)\n", gpioPinNumber));
        return AJ_ERR_INVALID;
    }

    AJ_InfoPrintf(("AJS_TargetIO_PinClose(%d)\n", gpioPinNumber));
    gpio[gpioPinNumber].gpioPin = nullptr; // GpioPin::Close();
    return AJ_OK;
}

void AJS_TargetIO_PinToggle(void* pinCtx)
{
    uint32_t value = (AJS_TargetIO_PinGet(pinCtx) == 0) ? 1 : 0;
    AJS_TargetIO_PinSet(pinCtx, value);
}

void AJS_TargetIO_PinSet(void* pinCtx, uint32_t val)
{
    int gpioPinNumber = (int)pinCtx;
    if (gpioPinNumber >= ArraySize(gpio)) {
        AJ_ErrPrintf(("AJS_TargetIO_PinSet(): invalid pin (%d)\n", gpioPinNumber));
        return;
    }

    gpio[gpioPinNumber].gpioPin->Write(val == 0 ? GpioPinValue::Low : GpioPinValue::High);

    AJ_InfoPrintf(("AJS_TargetIO_PinSet(%d, %d)\n", gpioPinNumber, val));
}

uint32_t AJS_TargetIO_PinGet(void* pinCtx)
{
    int gpioPinNumber = (int)pinCtx;
    if (gpioPinNumber >= ArraySize(gpio)) {
        AJ_ErrPrintf(("AJS_TargetIO_PinGet(): invalid pin (%d)\n", gpioPinNumber));
        return 0;
    }

    GpioPinValue val = gpio[gpioPinNumber].gpioPin->Read();

    AJ_InfoPrintf(("AJS_TargetIO_PinGet(%d, %d)\n", gpioPinNumber, val));
    return (uint32_t)(val == GpioPinValue::Low) ? 0 : 1;
}

int32_t AJS_TargetIO_PinTrigId(AJS_IO_PinTriggerCondition* condition)
{
    if (trigSet == 0) {
        return AJS_IO_PIN_NO_TRIGGER;
    } else {
        /*
         * This is static so triggers are returned round-robin to ensure fairness
         */
        static uint32_t id = 0;
        while (!BIT_IS_SET(trigSet, id % MAX_TRIGGERS)) {
            ++id;
        }
        /*
         * Set level to 0 to indicate the trigger was on falling edge or 1 for a rising edge.
         */
        *condition = gpio[id % MAX_TRIGGERS].condition;
        BIT_CLR(trigSet, id % MAX_TRIGGERS);
        return (id % MAX_TRIGGERS);
    }
}

AJ_Status AJS_TargetIO_PinEnableTrigger(void* pinCtx, int pinFunction, AJS_IO_PinTriggerCondition condition, int32_t* trigId, uint8_t debounce)
{
    TimeSpan timespan;
    int gpioPinNumber = (int)pinCtx;
    if (gpioPinNumber >= ArraySize(gpio)) {
        AJ_ErrPrintf(("AJS_TargetIO_PinEnableTrigger(): invalid pin (%d)\n", gpioPinNumber));
        return AJ_ERR_INVALID;
    }
    if ((pinFunction & AJS_IO_FUNCTION_DIGITAL_IN) == 0) {
        AJ_ErrPrintf(("AJS_TargetIO_PinEnableTrigger(): invalid pinFunction (%d)\n", pinFunction));
        return AJ_ERR_INVALID;
    }

    if ((condition != AJS_IO_PIN_TRIGGER_ON_RISE) && (condition != AJS_IO_PIN_TRIGGER_ON_FALL)) {
        /*
         * Disable triggers for this pin
         */

        if (gpio[gpioPinNumber].condition != AJS_IO_PIN_TRIGGER_DISABLE) {
            BIT_CLR(trigSet, gpioPinNumber);
        } else {
            *trigId = AJS_IO_PIN_NO_TRIGGER;
            return AJ_OK;
        }
    }
    /*
     * Convert debounce time from millisecond to 100-nanosecond
     */
    timespan.Duration = 10 * debounce;

    gpio[gpioPinNumber].gpioPin->DebounceTimeout = timespan;
    gpio[gpioPinNumber].condition = condition;
    *trigId = gpioPinNumber;

    AJ_InfoPrintf(("AJS_TargetIO_PinEnableTrigger pinId %d\n", gpioPinNumber));
    return AJ_OK;
}

AJ_Status AJS_TargetIO_PinDisableTrigger(void* pinCtx, int pinFunction, AJS_IO_PinTriggerCondition condition, int32_t* trigId)
{
    TimeSpan timespan;
    int gpioPinNumber = (int)pinCtx;
    if (gpioPinNumber >= ArraySize(gpio)) {
        AJ_ErrPrintf(("AJS_TargetIO_PinDisableTrigger(): invalid pin (%d)\n", gpioPinNumber));
        return AJ_ERR_INVALID;
    }
    if ((pinFunction & AJS_IO_FUNCTION_DIGITAL_IN) == 0) {
        AJ_ErrPrintf(("AJS_TargetIO_PinDisableTrigger(): invalid pinFunction (%d)\n", pinFunction));
        return AJ_ERR_INVALID;
    }

    if (gpio[gpioPinNumber].condition != AJS_IO_PIN_TRIGGER_DISABLE) {
        BIT_CLR(trigSet, gpioPinNumber);
    }
    else {
        *trigId = AJS_IO_PIN_NO_TRIGGER;
        return AJ_OK;
    }
    *trigId = gpioPinNumber;

    AJ_InfoPrintf(("AJS_TargetIO_PinEnableTrigger pinId %d\n", gpioPinNumber));
    return AJ_OK;
}

AJ_Status AJS_TargetIO_PinPWM(void* pinCtx, double dutyCycle, uint32_t freq)
{
    return AJ_ERR_UNEXPECTED;
}

AJ_Status AJS_TargetIO_System(const char* cmd, char* output, uint16_t length)
{
    return AJ_ERR_DISALLOWED;
}
