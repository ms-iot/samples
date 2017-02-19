/**************************************************************************
*
* Copyright (C) 2005 Steve Karg <skarg@users.sourceforge.net>
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
*********************************************************************/

#include <rttarget.h>
#include <rtkernel.h>
#include <windows.h>

extern void RTEmuInit(
    void);

#ifdef _MSC_VER
#define VOIDEXPORT _declspec(dllexport) void __cdecl
#else
#define VOIDEXPORT void __export __cdecl
#endif

/* DISK SYSTEM */
#ifdef DOC      /* include DiskOnChip driver */
#include <rtfiles.h>
#define RTF_MAX_FILES 16        /* support for more open files (default is 8) */
#define RTF_BUFFERS_IN_BSS      /* we do not need file I/O before the run-time */
#include <rtfdata.c>    /* system is initialized */

  /*#define READ_HEAD_BUFFER_SIZE 2048+4 */

  /*static BYTE ReadAheadBuffer[READ_HEAD_BUFFER_SIZE]; */

static RTFDrvFLPYData FLPYDriveAData = { 0 };
static RTFDrvDOCData DOCDriveData = { 0 };
static RTFDrvIDEData IDEDriveData = { 0 };

RTFDevice RTFDeviceList[] = {
    /* type,number,flags,driver,driverdata */
    {RTF_DEVICE_FLOPPY, 0, 0, &RTFDrvFloppy, &FLPYDriveAData},
    {RTF_DEVICE_FDISK, 0, 0, &RTFDrvDOC, &DOCDriveData},
    {RTF_DEVICE_FDISK, 0, 0, &RTFDrvIDE, &IDEDriveData},
    {0, 0, 0, NULL, NULL}
};
#endif
/* END OF DISK SYSTEM */

/* RTTarget only defines 64 Win32 handles, which are not enough for BACstac */
/* the following code is from the RTTarget manual, page 106 */
#define MAXHANDLES 1024
#define MAXOBJECTS 1024
#define MAXTYPES   32

RTW32Handle RTHandleTable[MAXHANDLES] = { {0} };

int RTHandleCount = MAXHANDLES;

RTW32Object RTObjectTable[MAXOBJECTS] = { {0} };

int RTObjectCount = MAXOBJECTS;

RTW32Types RTTypeTable[MAXTYPES] = { {0} };

int RTTypeCount = MAXTYPES;

#if 0
/* We can embed some files in the RTB file, like a binary
   file used for configuring a remote device, using
   'Locate File filename HighMem' in the config (.CFG) file.
   However, the default setup for RTFiles and RTTarget
   doesn't include the RAM files, so we need to specify
   that here, as well as the LPT, console, and FAT.
   From RTFiles-32 manual, ch. 7, "Using RTFiles-32 with
   RTTarget-32" */
RTFileSystem Console = { RT_FS_CONSOLE, 0, 0, &RTConsoleFileSystem };

RTFileSystem LPTFiles = { RT_FS_LPT_DEVICE, 0, 0, &RTLPTFileSystem };

/* logical drive Z: can be used to access the RAM drive */
RTFileSystem RAMFiles = { RT_FS_FILE, 1 << ('Z' - 'A'), 0, &RTRAMFileSystem };

/* logical drive A: through D: are reserved for FAT */
RTFileSystem FATFiles =
    { RT_FS_FILE | RT_FS_IS_DEFAULT, 0x0F, 0x03, &RTFilesFileSystem };

RTFileSystem *RTFileSystemList[] = {
    &Console,
    &LPTFiles,
    &RAMFiles,
    &FATFiles,
    NULL,
};
#endif

/*-----------------------------------*/
VOIDEXPORT Init(
    void)
{
    (void) RTSetFlags(RT_MM_VIRTUAL, 1);        /* this is the better method */
    (void) RTCMOSExtendHeap();  /* get as much memory as we can */
    RTCMOSSetSystemTime();      /* get the right date and time */
    RTEmuInit();        /* set up floating point emulation */
    /* pizza - RTHaltCPL3 appears to cause problems with file handling */
    /*RTIdleHandler = (void RTTAPI *)RTHaltCPL3;  // low power when idle */
    /* not needed with pre-emptive */
    /*RTKTimeSlice(2);               // allow same priority task switch */
    RTKConfig.Flags |= RF_PREEMPTIVE;   /* preemptive multitasking */
    RTKConfig.Flags |= RF_WIN32MUTEX_MUTEX;     /* Win32 mutexes are RTK32 mutexes */
    RTKConfig.Flags |= RF_FPCONTEXT;    /* saves floating point context for tasks */
    RTKConfig.HookedIRQs |= 1 << 1;     /* hook the keyboard IRQ  */
    RTKConfig.DefaultTaskStackSize = 1024 * 8;  /* for Win32 task stacks req = 0 */
}
