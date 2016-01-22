BACnet Stack - SourceForge.net
Build for MPLAB IDE

These are some settings that are important when building 
the BACnet Stack using MPLAB IDE and MCC18 Compiler,

1. Add the files to the project that you need:
abort.c, apdu.c, bacapp.c, bacdcode.c, bacerror.c,
bacstr.c, crc.c, datetime.c, dcc.c, iam.c, 
npdu.c, rd.c, reject.c, rp.c, whois.c, wp.c

From ports/picxx: isr.c, main.c, rs485.c, mstp.c, dlmstp.c

From demo/object/: device.c or dev_tiny.c
objects as needed: ai.c, ao.c, etc.

From demo/handler/:  txbuf.c, h_dcc.c, h_rd.c, h_rp.c or h_rp_tiny.c
Additional handlers as needed: h_wp.c

2. Project->Options->Project

General Tab: Include Path:
C:\code\bacnet-stack\;C:\code\bacnet-stack\demo\handler\;C:\code\bacnet-stack\demo\object\;C:\code\bacnet-stack\ports\pic18f6720\

MPLAB C18 Tab: Memory Model:
Code: Large Code Model
Data: Large Data Model
Stack: Multi-bank Model

MPLAB C18 Tab: General: Macro Definitions:
PRINT_ENABLED=0
BACDL_MSTP=1
TSM_ENABLED=0
BIG_ENDIAN=0

3. The linker script must reserve some extra stack space.

//DATABANK   NAME=gpr12      START=0xC00          END=0xCFF
//DATABANK   NAME=gpr13      START=0xD00          END=0xDFF
DATABANK   NAME=stackreg   START=0xC00          END=0xDFF          PROTECTED

//STACK SIZE=0x100 RAM=gpr13
STACK SIZE=0x200 RAM=stackreg

