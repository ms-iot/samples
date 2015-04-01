//
// To compile this file to ACPITABL.dat, run makeacpi.cmd from a razzle prompt.
// Copy ACPITABL.dat to %windir%\system32, turn on testsigning, and reboot.
//

DefinitionBlock ("ACPITABL.dat", "SSDT", 1, "MSFT", "GPOT", 1)
{

    Scope (\_SB)
    {
        //
        // RHProxy Device Node to enable WinRT API
        //
        Device(GPOT)
        {
            Name(_HID, "GPOT0001")
            Name(_CID, "GPOT0001")
            Name(_UID, 1)

            Name(_CRS, ResourceTemplate()
            {
                // Index 0
                SPISerialBus(              // SCKL - GPIO 11 - Pin 23
                                           // MOSI - GPIO 10 - Pin 19
                                           // MISO - GPIO 9  - Pin 21
                                           // CE0  - GPIO 8  - Pin 24
                    0,                     // Device selection (CE0)
                    PolarityLow,           // Device selection polarity
                    FourWireMode,          // wiremode
                    8,                     // databit len
                    ControllerInitiated,   // slave mode
                    4000000,               // connection speed
                    ClockPolarityLow,      // clock polarity
                    ClockPhaseFirst,       // clock phase
                    "\\_SB.SPI0",          // ResourceSource: SPI bus controller name
                    0,                     // ResourceSourceIndex
                                           // Resource usage
                                           // DescriptorName: creates name for offset of resource descriptor
                    )                      // Vendor Data

                // Index 1
                SPISerialBus(              // SCKL - GPIO 11 - Pin 23
                                           // MOSI - GPIO 10 - Pin 19
                                           // MISO - GPIO 9  - Pin 21
                                           // CE1  - GPIO 7  - Pin 26
                    1,                     // Device selection (CE1)
                    PolarityLow,           // Device selection polarity
                    FourWireMode,          // wiremode
                    8,                     // databit len
                    ControllerInitiated,   // slave mode
                    4000000,               // connection speed
                    ClockPolarityLow,      // clock polarity
                    ClockPhaseFirst,       // clock phase
                    "\\_SB.SPI0",          // ResourceSource: SPI bus controller name
                    0,                     // ResourceSourceIndex
                                           // Resource usage
                                           // DescriptorName: creates name for offset of resource descriptor
                    )                      // Vendor Data

                // Index 2
                SPISerialBus(              // SCKL - GPIO 21 - Pin 40
                                           // MOSI - GPIO 20 - Pin 38
                                           // MISO - GPIO 19 - Pin 35
                                           // CE1  - GPIO 17 - Pin 11
                    1,                     // Device selection (CE1)
                    PolarityLow,           // Device selection polarity
                    FourWireMode,          // wiremode
                    8,                     // databit len
                    ControllerInitiated,   // slave mode
                    1000000,               // connection speed
                    ClockPolarityLow,      // clock polarity
                    ClockPhaseFirst,       // clock phase
                    "\\_SB.SPI1",          // ResourceSource: SPI bus controller name
                    0,                     // ResourceSourceIndex
                                           // Resource usage
                                           // DescriptorName: creates name for offset of resource descriptor
                    )                      // Vendor Data


                // Index 3
                I2CSerialBus(              // Pin 3 (GPIO2, SDA1), 5 (GPIO3, SCL1)
                    0xFFFF,                // SlaveAddress: placeholder
                    ,                      // SlaveMode: default to ControllerInitiated
                    0,                     // ConnectionSpeed: placeholder
                    ,                      // Addressing Mode: default to 7 bit
                    "\\_SB.I2C1",          // ResourceSource: I2C bus controller name
                    ,
                    ,
                    ,                      // Descriptor Name: creates name for offset of resource descriptor
                    )                      // VendorData

                // Index 4 - GPIO 0
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 0 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 0 }
                // Index 6 - GPIO 1
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 1 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 1 }
                // Index 8 - GPIO 5
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNoneAndPreserve, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 5 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 5 }
                // Index 10 - GPIO 6
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 6 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 6 }
                // Index 12 - GPIO 12
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 12 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 12 }
                // Index 14 - GPIO 13
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 13 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 13 }
                // Index 16 - GPIO 16
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 16 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 16 }
                // Index 18 - GPIO 18
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 18 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 18 }
                // Index 20 - GPIO 22
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 22 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 22 }
                // Index 22 - GPIO 23
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 23 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 23 }
                // Index 24 - GPIO 24
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 24 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 24 }
                // Index 26 - GPIO 25
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 25 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 25 }
                // Index 28 - GPIO 26
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 26 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 26 }
                // Index 30 - GPIO 27
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 27 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 27 }
                // Index 32 - GPIO 35
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 35 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 35 }
                // Index 34 - GPIO 47
                GpioIO(Shared, PullDefault, 0, 0, IoRestrictionNone, "\\_SB.GPI0", 0, ResourceConsumer, , ) { 47 }
                GpioInt(Edge, ActiveBoth, Shared, PullDefault, 0, "\\_SB.GPI0",)                            { 47 }
            })

            Name(_DSD, Package()
            {
                ToUUID("daffd814-6eba-4d8c-8a91-bc9bbf4aa301"),
                Package()
                {
                    // Reference http://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md
                    // SPI 0
                    Package(2) { "bus-SPI-SPI0", Package() { 0, 1 }},                       // Index 0 & 1
                    Package(2) { "SPI0-MinClockInHz", 7629 },                               // 7629 Hz
                    Package(2) { "SPI0-MaxClockInHz", 125000000 },                          // 125 MHz
                    Package(2) { "SPI0-SupportedDataBitLengths", Package() { 8 }},          // Data Bit Length
                    // SPI 1
                    Package(2) { "bus-SPI-SPI1", Package() { 2 }},                          // Index 2
                    Package(2) { "SPI1-MinClockInHz", 30518 },                              // 30518 Hz
                    Package(2) { "SPI1-MaxClockInHz", 125000000 },                          // 125 MHz
                    Package(2) { "SPI1-SupportedDataBitLengths", Package() { 8 }},          // Data Bit Length
                    // I2C1
                    Package(2) { "bus-I2C-I2C1", Package() { 3 }},
                    // GPIO0 - Index 4
                    Package(2) { "bus-GPIO-GPIO0", Package() { 4 }},
                    Package(2) { "GPIO0-PinNumber", Package() { 0 }},
                    // GPIO1 - Index 6
                    Package(2) { "bus-GPIO-GPIO1", Package() { 6 }},
                    Package(2) { "GPIO1-PinNumber", Package() { 1 }},
                    // GPIO5 - Index 8
                    Package(2) { "bus-GPIO-GPIO5", Package() { 8 }},
                    Package(2) { "GPIO5-PinNumber", Package() { 5 }},
                    // GPIO6 - Index 10
                    Package(2) { "bus-GPIO-GPIO6", Package() { 10 }},
                    Package(2) { "GPIO6-PinNumber", Package() { 6 }},
                    // GPIO12 - Index 12
                    Package(2) { "bus-GPIO-GPIO12", Package() { 12 }},
                    Package(2) { "GPIO12-PinNumber", Package() { 12 }},
                    // GPIO13 - Index 14
                    Package(2) { "bus-GPIO-GPIO13", Package() { 14 }},
                    Package(2) { "GPIO13-PinNumber", Package() { 13 }},
                    // GPIO16 - Index 16
                    Package(2) { "bus-GPIO-GPIO16", Package() { 16 }},
                    Package(2) { "GPIO16-PinNumber", Package() { 16 }},
                    // GPIO18 - Index 18
                    Package(2) { "bus-GPIO-GPIO18", Package() { 18 }},
                    Package(2) { "GPIO18-PinNumber", Package() { 18 }},
                    // GPIO22 - Index 20
                    Package(2) { "bus-GPIO-GPIO22", Package() { 20 }},
                    Package(2) { "GPIO22-PinNumber", Package() { 22 }},
                    // GPIO23 - Index 22
                    Package(2) { "bus-GPIO-GPIO23", Package() { 22 }},
                    Package(2) { "GPIO23-PinNumber", Package() { 23 }},
                    // GPIO24 - Index 24
                    Package(2) { "bus-GPIO-GPIO24", Package() { 24 }},
                    Package(2) { "GPIO24-PinNumber", Package() { 24 }},
                    // GPIO25 - Index 26
                    Package(2) { "bus-GPIO-GPIO25", Package() { 26 }},
                    Package(2) { "GPIO25-PinNumber", Package() { 25 }},
                    // GPIO26 - Index 28
                    Package(2) { "bus-GPIO-GPIO26", Package() { 28 }},
                    Package(2) { "GPIO26-PinNumber", Package() { 26 }},
                    // GPIO27 - Index 30
                    Package(2) { "bus-GPIO-GPIO27", Package() { 30 }},
                    Package(2) { "GPIO27-PinNumber", Package() { 27 }},
                    // GPIO35 - Index 32
                    Package(2) { "bus-GPIO-GPIO35", Package() { 32 }},
                    Package(2) { "GPIO35-PinNumber", Package() { 35 }},
                    // GPIO47 - Index 34
                    Package(2) { "bus-GPIO-GPIO47", Package() { 34 }},
                    Package(2) { "GPIO47-PinNumber", Package() { 47 }},
                }
            })
        }
    }
}