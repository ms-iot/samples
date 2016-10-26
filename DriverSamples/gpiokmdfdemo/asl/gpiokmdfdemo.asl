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

DefinitionBlock ("ACPITABL.dat", "SSDT", 1, "MSFT", "gpiodemo", 1)
{
    Scope (\_SB)
    {
        //
        // Test peripheral device node for MinnowBoardMax
        //
        Device(GPOT)
        {
            Name(_HID, "GPOT0001")
            Name(_CID, "GPOT0001")
            Name(_UID, 1)
            Name (RBUF, ResourceTemplate ()
            {
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 0 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 1 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 2 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 62 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 63 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 65 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 64 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 94 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 95 }
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 54 }
            })
            Method(_CRS, 0x0, NotSerialized)
            {
                Return(RBUF)
            }
        }
    }
}