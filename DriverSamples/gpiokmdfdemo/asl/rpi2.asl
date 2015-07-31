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
        // Test peripheral device node for RPi2
        //
        Device(GPOT)
        {
            Name(_HID, "GPOT0001")
            Name(_CID, "GPOT0001")
            Name(_UID, 1)
            Name(_CRS, ResourceTemplate()
            {
                // Index 4 - GPIO 0
                //GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 0 }
				
                // Index 6 - GPIO 1
                //GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 1 }
				
				// Index ? - GPIO 4
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 4 }
				
                // Index 8 - GPIO 5
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 5 }
				
                // Index 10 - GPIO 6
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 6 }
				
                // Index 12 - GPIO 12
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 12 }
				
                // Index 14 - GPIO 13
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 13 }
				
                // Index 16 - GPIO 16
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 16 }
				
                // Index 18 - GPIO 18
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 18 }
				
                // Index 20 - GPIO 22
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 22 }
				
                // Index 22 - GPIO 23
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 23 }
				
                // Index 24 - GPIO 24
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 24 }
				
                // Index 26 - GPIO 25
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 25 }
				
                // Index 28 - GPIO 26
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 26 }
				
                // Index 30 - GPIO 27
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 27 }
				
                // Index 32 - GPIO 35
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 35 }
				
                // Index 34 - GPIO 47
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 47 }
            })
        }
    }
}
