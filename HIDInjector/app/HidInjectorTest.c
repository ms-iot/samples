/*++

Copyright (c) Microsoft Corporation.  All rights reserved.

    THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
    KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR
    PURPOSE.

Module Name:

    testvhid.c

Environment:

    user mode only

Author:

--*/

#include <windows.h>
#include "common.h"
#include "HidDevice.h"
#include "SendInput.h"


BOOLEAN
SendTestInput(
	HANDLE File
	);

//
// Implementation
//
INT __cdecl
main(
    _In_ ULONG argc,
    _In_reads_(argc) PCHAR argv[]
    )
{
    HANDLE file = INVALID_HANDLE_VALUE;
    BOOLEAN found = FALSE;
    BOOLEAN bSuccess = FALSE;


    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);

    srand( (unsigned)time( NULL ) );

	found = OpenHidInjectorDevice();
    if (found) {
        printf("...sending control request to our device\n");

		bSuccess = SendTestInput(g_hFile);
		if (bSuccess == FALSE) {
			goto cleanup;
		}

    }
    else {
        printf("Failure: Could not find our HID device \n");
    }

cleanup:

    if (found && bSuccess == FALSE) {
        printf("****** Failure: one or more commands to device failed *******\n");
    }

    if (file != INVALID_HANDLE_VALUE) {
        CloseHandle(file);
    }

    return (bSuccess ? 0 : 1);
}
BOOLEAN SendReport(
	HANDLE File,
	void *Data,
	DWORD  Size
	)
{
	DWORD BytesWritten = 0;

	return WriteFile(
		File,
		Data,
		Size,
		&BytesWritten,
		NULL
		);
}

void SendMousePosition(
	HANDLE File,
	WORD X,
	WORD Y,
	UCHAR Buttons
	)
{
	InjectMouseMove(X * 2, Y * 2, Buttons);
}

void SendRawKey(
	HANDLE File,
	BYTE b
)
{
	HIDINJECTOR_INPUT_REPORT KeyDown = { 0 };
	HIDINJECTOR_INPUT_REPORT KeyUp = { 0 };

	KeyDown.ReportId = KEYBOARD_REPORT_ID;
	KeyDown.Report.KeyReport.Key1 = b;

	KeyUp.ReportId = KEYBOARD_REPORT_ID;

	SendReport(File, &KeyDown, sizeof(KeyDown));
	// Sleep(500);	// May not be necessary
	SendReport(File, &KeyUp, sizeof(KeyUp));

}

void SendKey(
	HANDLE File,
	BYTE Modifiers,
	char Key
	)
{
	HIDINJECTOR_INPUT_REPORT KeyDown = { 0 };
	HIDINJECTOR_INPUT_REPORT KeyUp = { 0 };

	KeyDown.ReportId = KEYBOARD_REPORT_ID;
	KeyDown.Report.KeyReport.Modifiers = Modifiers;
	KeyDown.Report.KeyReport.Key1 = Key - 'a' + 4;

	KeyUp.ReportId = KEYBOARD_REPORT_ID;

	SendReport(File, &KeyDown, sizeof(KeyDown));
	// Sleep(100);	// May not be necessary
	SendReport(File, &KeyUp, sizeof(KeyUp));
}

BOOL SendTouch(HANDLE File,
    UINT32 count,
    CONST POINTER_TOUCH_INFO * contacts
	)
{
    for (UINT32 i = 0; i < count; i++)
    {
		HIDINJECTOR_INPUT_REPORT TouchState = { 0 };

        TouchState.ReportId = TOUCH_REPORT_ID;
        TouchState.Report.TouchReport.ContactCount = i == 0 ? count : 0; // Only the first report contains the contact count for the frame.
        TouchState.Report.TouchReport.ContactIndentifier = contacts[i].pointerInfo.pointerId;

        //the value is expected to be normalized and clamped to[0, 65535]
        TouchState.Report.TouchReport.AbsoluteX = (USHORT)max(0, min(contacts[i].pointerInfo.ptPixelLocation.x * GetSystemMetrics(SM_CXSCREEN), 0x7FFF));
        TouchState.Report.TouchReport.AbsoluteY = (USHORT)max(0, min(contacts[i].pointerInfo.ptPixelLocation.y * GetSystemMetrics(SM_CYSCREEN), 0x7FFF));

        if (contacts[i].pointerInfo.pointerFlags & POINTER_FLAG_INCONTACT)
        {
            TouchState.Report.TouchReport.Flags |= TOUCH_TIP_SWITCH; // Finger touching the screen
        }
        else if (contacts[i].pointerInfo.pointerFlags & POINTER_FLAG_INRANGE)
        {
            TouchState.Report.TouchReport.Flags |= TOUCH_IN_RANGE;   // Finger hovering over screen
        }

        if (!SendReport(File, &TouchState, sizeof(TouchState)))
        {
            return FALSE;
        }
    }

    return TRUE;
}

BOOL SendSingleTouch(HANDLE file,
	int index,
	USHORT x,
	USHORT y,
	POINTER_FLAGS flags)
{
	POINTER_TOUCH_INFO touchInfo = {0};

    touchInfo.pointerInfo.pointerId = 0;
    touchInfo.pointerInfo.ptPixelLocation.x = x;
    touchInfo.pointerInfo.ptPixelLocation.y = y;
    touchInfo.pointerInfo.pointerFlags = flags;


    return SendTouch(file, 1, &touchInfo);
}

BOOLEAN
SendTestInput(
	HANDLE File
	)
{
    // Uncomment out the block to see how to inject various inputs

    // The following tests show how to inject a single touch event.
    SendSingleTouch(g_hFile, 0, 320, 200, POINTER_FLAG_INCONTACT);
    SendSingleTouch(g_hFile, 0, 320, 200, 0);

    // The following tests show how to inject various keys
	InjectUnicode('a');
	InjectScanKeyDown(42);
	InjectUnicode('A');
	InjectScanKeyUp(42);
    
    /*
    // Send multiple return events.

    for (int i = 0; i < 5; i++)
    {
    InjectKeyDown(VK_RETURN);
    InjectKeyUp(VK_RETURN);
    }
    */
    
    /*
    // Send Raw keyboard events.

	SendRawKey(File, 0x28);
	SendRawKey(File, 0x28);
	SendRawKey(File, 0x28);
	SendRawKey(File, 0x28);
    */

    /*
    Send mouse delta events
	SendMouseDelta(File, 25, 25);
	SendMouseDelta(File, 25, 25);
	SendMouseDelta(File, 25, 25);
	SendMouseDelta(File, 25, 25);
	SendMouseDelta(File, -25, -25);
	SendMouseDelta(File, -25, -25);
	SendMouseDelta(File, -25, -25);
	SendMouseDelta(File, -25, -25);
	*/

    /*
    Send specific mouse position events.
    SendMousePosition(g_hFile, 3200, 800, MOUSEEVENTF_LEFTDOWN);
    SendMousePosition(g_hFile, 3200, 800, MOUSEEVENTF_LEFTUP);
    SendMousePosition(g_hFile, 800, 800, MOUSEEVENTF_LEFTDOWN);
    SendMousePosition(g_hFile, 800, 800, MOUSEEVENTF_LEFTUP);

    SendMousePosition(g_hFile, 3200, 800, MOUSEEVENTF_LEFTDOWN);
    SendMousePosition(g_hFile, 3200, 800, MOUSEEVENTF_LEFTUP);
    SendMousePosition(g_hFile, 800, 800, MOUSEEVENTF_LEFTDOWN);
    SendMousePosition(g_hFile, 800, 800, MOUSEEVENTF_LEFTUP);

    SendMousePosition(g_hFile, 0, 0, 0);
    SendMousePosition(g_hFile, 0x3fff, 0x3fff, 0);

    */
    return TRUE;
}
 
