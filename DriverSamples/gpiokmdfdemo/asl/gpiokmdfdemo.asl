//
// Compile with:
//   asl.exe gpiokmdfdemo.asl
//   - OR -
//   iasl.exe gpiokmdfdemo.asl
//
// Copy ACPITABL.dat to %windir%\system32, turn on testsigning, and reboot.
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
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 0 }       // JP1:21
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 1 }       // JP1:23
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 2 }       // JP1:25
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO2", 0, ResourceConsumer, , ) { 0, 1, 2 }
                
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 62 }      // JP1:14
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 63 }      // JP1:16
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 65 }      // JP1:18
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 64 }      // JP1:20
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 94 }      // JP1:22
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 95 }      // JP1:24
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPO0", 0, ResourceConsumer, , ) { 54 }      // JP1:26
                
                // Interrupt pin: falling edge
                GpioInt(Edge, ActiveLow, Exclusive, PullUp, 0, "\\_SB.GPO2", , , ,) { 0 }
            })
            Method(_CRS, 0x0, NotSerialized)
            {
                Return(RBUF)
            }
        }
    }
}