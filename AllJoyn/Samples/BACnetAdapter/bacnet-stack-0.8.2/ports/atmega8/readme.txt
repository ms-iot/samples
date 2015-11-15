This port was originally done with the Atmel ATmega168 
I used the following tools:
1. The WinAVR compiler avr-gcc (GCC) 4.1.2 (WinAVR 20070525)
and tools from <http://winavr.sourceforge.net/>, hints and 
sample code from <http://www.avrfreaks.net/> and 
<http://savannah.gnu.org/projects/avr-libc/>.
"avr-binutils, avr-gcc, and avr-libc form the heart of the 
Free Software toolchain for the Atmel AVR microcontrollers."
2. AVR Studio 4 from Atmel <http://atmel.com/>

The hardware is expected to utilize the signals as defined
in the spreadsheet hardware.ods (OpenOffice.org calc).
Attach a DS75176 RS-485 transceiver (or similar) to the USART.
DS75176 ATmega168
------  ---------
 RO       RXD
 /RE      --choice of I/O
 DE       --choice of I/O
 DI       TXD
 GND      GND
 DO       --to RS-485 wire
 DO       --to RS-485 wire
 +5V      From 5V Regulator

The makefile allows you to build a simple server. 
dlmstp is the datalink layer for MS/TP over RS-485.
This project uses an MS/TP Slave Node.

I used the makefile from the command line on Windows:
C:\code\bacnet-stack\ports\atmega168> make clean all

The BACnet Capabilities include ReadProperty support.  
The BACnet objects include only a Device object.  
All required object properties can be retrieved using ReadProperty.  

With full optimization, the statistics on the demo are:

avr-gcc (GCC) 4.2.2 (WinAVR 20071221rc1)
Device: atmega168
Program:    8734 bytes (53.3% Full)
Data:        254 bytes (24.8% Full) (does not include CStack)

Hopefully you find this code useful!

Steve Karg <skarg@users.sourceforge.net>
