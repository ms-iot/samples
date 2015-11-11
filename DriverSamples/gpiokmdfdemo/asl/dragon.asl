//
//    Copyright(c) Microsoft Open Technologies, Inc. All rights reserved.
//
//    The MIT License(MIT)
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//    of this software and associated documentation files(the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//    to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
//    copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions :
//
//    The above copyright notice and this permission notice shall be included in
//    all copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
//    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//    THE SOFTWARE.
//

DefinitionBlock ("ACPITABL.dat", "SSDT", 1, "MSFT", "GPOT", 1)
{
    Scope (\_SB)
    {
        //
        // Test peripheral device node for the Dragon board
        //
        Device(GPOT)
        {
            Name(_HID, "GPOT0001")
            Name(_CID, "GPOT0001")
            Name(_UID, 1)
            Name(_CRS, ResourceTemplate()
            {
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0024 } // Index: 0  - Pin #23 - GPIO-A - GPIO_36
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x000C } // Index: 1  - Pin #24 - GPIO-B - GPIO_12
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x000D } // Index: 2  - Pin #25 - GPIO-C - GPIO_13
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0045 } // Index: 3  - Pin #26 - GPIO-D - GPIO_69
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0073 } // Index: 4  - Pin #27 - GPIO-E - GPIO_115
              //GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.PM01", 0x00, ResourceConsumer, ,){ 0x0518 } // Index: X  - Pin #   - GPIO-F - MPP_4
              //GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0018 } // Index: X  - Pin #   - GPIO-G - GPIO_24
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0019 } // Index: 5  - Pin #30 - GPIO-H - GPIO_25
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0023 } // Index: 6  - Pin #31 - GPIO-I - GPIO_35
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0022 } // Index: 7  - Pin #32 - GPIO-J - GPIO_34
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x001C } // Index: 8  - Pin #33 - GPIO-K - GPIO_28
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0021 } // Index: 9  - Pin #34 - GPIO-L - GPIO_33
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0015 } // Index: 10 - LED #1             GPIO_21
                GpioIo (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.GIO0", 0x00, ResourceConsumer, ,){ 0x0078 } // Index: 11 - LED #2             GPIO_120
                GpioIO (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.PM01", 0x00, ResourceConsumer, ,){ 0x0600 } // Index: 12 - LED #3             GPIO_1
                GpioIO (Shared, PullNone, 0x0000, 0x0000, IoRestrictionNoneAndPreserve, "\\_SB.PM01", 0x00, ResourceConsumer, ,){ 0x0608 } // Index: 13 - LED #4             GPIO_2
            })
        }
    }
}