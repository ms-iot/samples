This port was done with a AT91SAM7S-EK which contained a
AT91SAM7S64 processor.  The compiler was the GNU ARM compiler
and tools from Yagarto project.  

The hardware was modified by severing the I-PA5 (RXD0), 
I-PA6 (TXD0), I-PA7 (RTS0) pads and rerouting those 
signals to a DS75176 RS-485 transceiver. 
PIN SIGNAL AT91SAM7S
--- ------ ---------
 1   RO      RXD0
 2   /RE     RTS
 3   DE      RTS
 4   DI      TXD0
 5   GND     GND
 6   DO      n/c
 7   DO      n/c
 8   +5V     From EXT_VCC via 5V Regulator

The makefile allows you to build just the dlmstp or a simple
server, both for programming into the flash memory of the processor.
The dlmstp is the datalink layer for MS/TP over RS-485.

I used the makefile from the command line on Windows, and 
then used the SAM-BA to send the resulting .bin file to the 
board using a J-Link.  To debug the code from flash, run the 
J-Link GDB Server and then:
> arm-elf-gdb bacnet.elf

I got the crt.s, at91sam7s256.ld, blinker.c, init.c, isr.c, and
timer.c from James P Lynch.  I created the rs485.c based on the 
initialization sequence from serial.c by Keil Electronik.  I 
got the at91sam7s256.h file from Atmel via the Keil website.
I started with the makefile from James P Lynch, but it didn't work
for me.  I then used some ideas from FreeRTOS makefile, and 
created my own makefile from scratch.

Hopefully you find it useful!

Steve Karg
