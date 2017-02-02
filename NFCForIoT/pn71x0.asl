
DefinitionBlock ("ACPITABL.dat", "SSDT", 1, "MSFT", "NFCTEST", 1)
{
    Scope (\_SB)
    {
        // Device (GPI0)
        // {
            // Name (_HID, "BCM2845")
            // Name (_CID, "BCMGPIO")
            // Name (_UID, 0x0)
            // Method (_STA)
            // {
                // Return(0xf)
            // }
            // Method (_CRS, 0x0, NotSerialized) {
                // Name (RBUF, ResourceTemplate () {
                    // MEMORY32FIXED(ReadWrite, 0x3F200000, 0xB4, )
                    // Interrupt(ResourceConsumer, Level, ActiveHigh, Shared) { 0x51 }
                    // Interrupt(ResourceConsumer, Level, ActiveHigh, Shared) { 0x53 }
                // })
                // Return(RBUF)
            // }
        // }
       
        // Device (I2C1)
        // {
            // Name (_HID, "BCM2841")
            // Name (_CID, "BCMI2C")
            // Name (_UID, 0x1)
            // Method (_STA)
            // {
                // Return(0xf)
            // }
            // Method (_CRS, 0x0, NotSerialized)
            // {
                // Name (RBUF, ResourceTemplate()
                // {
                    // Memory32Fixed(ReadWrite, 0x3F804000, 0x20)
                    // Interrupt(ResourceConsumer, Level, ActiveHigh, Shared) {0x55}
                    // MsftFunctionConfig(Exclusive, PullUp, 0x4, "\\_SB.GPI0", 0, ResourceConsumer, ) {2, 3}
                // })
                // Return(RBUF)
            // }
        // }
        
        Scope(GPI0)
        {
            OperationRegion(NFPO, GeneralPurposeIO, Zero, One)
        }
        
        Device(NFCD)
        {
            Name(_HID, "PN71X0")
            Name(_CID, "ACPI\PN71X0")
            Name(_CRS, Buffer(0x41)
            {
	0x8e, 0x19, 0x00, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x01, 0x06, 0x00,
	0x80, 0x1a, 0x06, 0x00, 0x28, 0x00, 0x5c, 0x5f, 0x53, 0x42, 0x2e, 0x49,
	0x32, 0x43, 0x31, 0x00, 0x8c, 0x20, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x00, 0x00, 0x19, 0x00, 0x23,
	0x00, 0x00, 0x00, 0x17, 0x00, 0x5c, 0x5f, 0x53, 0x42, 0x2e, 0x47, 0x50,
	0x49, 0x30, 0x00, 0x79, 0x00
            })
            Name(NFCP, Buffer(0x25)
            {
	0x8c, 0x20, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x17, 0x00, 0x00, 0x19, 0x00, 0x23, 0x00, 0x00, 0x00, 0x18,
	0x00, 0x5c, 0x5f, 0x53, 0x42, 0x2e, 0x47, 0x50, 0x49, 0x30, 0x00, 0x79,
	0x00
            })
            Field(\_SB_.GPI0.NFPO, ByteAcc, NoLock, Preserve)
            {
                Connection(\_SB_.NFCD.NFCP),
                MGPE, 1
            }
            Method(POON, 0x0, NotSerialized)
            {
                Store(One, MGPE)
            }
            Method(POOF, 0x0, NotSerialized)
            {
                Store(Zero, MGPE)
            }
            Method(_DSM, 0x4, NotSerialized)
            {
                Store("Method NFC _DSM begin", Debug)
                If(LEqual(Arg0, Buffer(0x10)
                {
	0xc4, 0xf6, 0xe7, 0xa2, 0x38, 0x96, 0x85, 0x44, 0x9f, 0x12, 0x6b, 0x4e,
	0x20, 0xb6, 0x0d, 0x63
                }))
                {
                    If(LEqual(Arg2, Zero))
                    {
                        Store("Method NFC _DSM QUERY", Debug)
                        If(LEqual(Arg1, One))
                        {
                            \_SB_.NFCD.POOF()
                            Sleep(0x14)
                            Return(Buffer(One)
                            {
	0x0f
                            })
                        }
                    }
                    If(LEqual(Arg2, 0x2))
                    {
                        Store("Method NFC _DSM SETPOWERMODE", Debug)
                        If(LEqual(Arg3, One))
                        {
                            \_SB_.NFCD.POON()
                            Sleep(0x14)
                        }
                        If(LEqual(Arg3, Zero))
                        {
                            \_SB_.NFCD.POOF()
                            Sleep(0x14)
                        }
                    }
                    If(LEqual(Arg2, 0x3))
                    {
                        Store("Method NFC _DSM EEPROM Config", Debug)
                        Return(Buffer(0x13)
                        {
	0x9c, 0x1f, 0x38, 0x19, 0xa8, 0xb9, 0x4b, 0xab, 0xa1, 0xba, 0xd0, 0x20,
	0x76, 0x88, 0x2a, 0xe0, 0x03, 0x01, 0x08
                        })
                    }
                }
            }
        }
    }
}