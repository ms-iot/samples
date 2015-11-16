This port was done with the Atmel ATmega168 using two tools:
1. The WinAVR compiler avr-gcc (GCC) 4.1.2 (WinAVR 20070525)
and tools from <http://winavr.sourceforge.net/>, hints and 
sample code from <http://www.avrfreaks.net/> and 
<http://savannah.gnu.org/projects/avr-libc/>.
"avr-binutils, avr-gcc, and avr-libc form the heart of the 
Free Software toolchain for the Atmel AVR microcontrollers."
2. AVR Studio from Atmel <http://atmel.com/>

Alternatively, the project also builds using IAR Embedded Workbench AVR.

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

The makefile allows you to build just the dlmstp or a simple
server. dlmstp is the datalink layer for MS/TP over RS-485.

I used the makefile from the command line on Windows:
C:\code\bacnet-stack\ports\atmega168> make clean all

CStack check for GCC is included in the device object as property 512.  
The compile shows 648 bytes of RAM used, and the ATmega168 has 1024 bytes
of RAM, leaving 376 for the CStack.  Property 512 index 0 returns 376 from
a ReadProperty request.  My understanding is that the remaining unallocated
RAM is used for the CStack.  Keep this in mind when developing.  
After some ReadProperty and WriteProperty requests, the CStack shows 
159 CStack bytes free, meaning that 216 bytes of CStack are used.  
Note that the value 0xC5 (197) was used to paint the CStack. 

I also used the bacnet.aps project file in AVR Studio to 
make the project and simulate it, but have not kept it updated (FIXME).

Compiler settings for IAR Embedded Workbench (FIXME: makefile?):
General Options
---------------
Target
 Processor configuration: --cpu=m168. ATmega168
 Memory Model: Small
 System configuration: Configure system using dialogs (not in .XCL file)
Output
 Executable
 Output Directories: Debug\Exe, Debug\Obj, Debug\List
Library Configuration
 Library: CLIB
Library Options
 Printf formatter: Small
 Scanf formatter: Medium
Heap Configuration
 CLIB heap size: 0x10
System
 CSTACK: 0x200
 RSTACK: 32
 Initialize unused interrupt vectors with RETI instructions (enabled)
 Enable bit defnitions in I/O-Include files. (enabled)
MISRA C
 not enabled

C/C++ Compiler
--------------
Language
 Language: C
 Require prototypes (not enabled)
 Allow IAR extensions
 Plain 'char' is Signed
 Enable multibyte support (not enabled)
Code
 Memory utilization:
  Place aggregate initializers in flash memory (enabled)
  Force generation of all global and static variables (not enabled)
 Register utilization:
  Number of registers to lock for global variables: 0
  Use ICCA90 1.x calling convention (not enabled)
Optimizations
 Size: High (Maximum optimization)
 Number of cross-call passes: Unlimited
 Always do cross call optimization (not enabled)
Output
 Module type: Override default (not enabled)
 Object module name (not enabled)
 Generate debug information (enabled)
 No error messages in output files (not enabled)
List
 Output list file (not enabled)
 Output assembler file (enabled)
Preprocessor
 Ignore standard include paths (not enabled)
 Include paths: 
  $PROJ_DIR$
  $PROJ_DIR$\..\..\include
 Preinclude file: (none)
 Defined symbols: 
  BACDL_MSTP
  MAX_APDU=50
  BIG_ENDIAN=0
  MAX_TSM_TRANSACTIONS=0
  BACAPP_REAL
  BACAPP_UNSIGNED
  BACAPP_ENUMERATED
  BACAPP_CHARACTER_STRING
  BACAPP_OBJECT_ID
  WRITE_PROPERTY
Diagnostics 
 (not enabled)
MISRA C
 (not enabled)
Extra Options
 Use command line options (not enabled)

Note:  The BACnet Stack at Sourceforge source code has to be built 
with lots of different compilers. The IAR compiler has particularly 
strong (pedantic) source checking and generates several warnings when 
compiling the source code.  Unfortunately not all warnings can be 
fixed by modifying the source code. Some warnings have therefore been 
disabled in the project file.
  Compiler Diagnostics:
  (Pe550) I initilize all local variables as a best practice.
  Linker Diagnostics:
  (w31) The supplied standard libraries expect char parameters to 
  be unsigned (in functions such as strncpy(), etc.). It may 
  be possible to recompile the libraries with signed plain char's.

The BACnet Capabilities include WhoIs, I-Am, ReadProperty, and 
WriteProperty support.  The BACnet objects include a Device object,
10 Binary Value objects, and 10 Analog Value objects.  An LED is 
controlled by Binary Value object instance 0.  All required object
properties can be retrieved using ReadProperty.  The Present_Value
property of the Analog Value and Binary Value objects can be 
written using WriteProperty.  The Object_Identifier, Object_Name, 
Max_Info_Frames, Max_Master, and baud rate (property 9600) of the 
Device object can be written using WriteProperty.

With full optimization, the statistics on the demo are:

IAR Atmel AVR C/C++ Compiler V5.10A/W32
12 732 bytes of CODE memory (+ 36 range fill )
955 bytes of DATA memory (+ 24 absolute ) (includes CStack=0×200)

avr-gcc (GCC) 4.2.2 (WinAVR 20071221rc1)
Program:   15790 bytes (96.4% Full)
Data:        414 bytes (40.4% Full) (does not include CStack=0×262)

Hopefully you find this code useful!

Steve Karg <skarg@users.sourceforge.net>
