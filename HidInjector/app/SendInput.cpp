#include <Windows.h>
#include "HidInject.h"
#include "SendInput.h"
#include"HidDevice.h"

HIDINJECTOR_INPUT_REPORT KeyboardState = { 0 };
HIDINJECTOR_INPUT_REPORT MouseState = { 0 };

BOOL SendHidReport(HIDINJECTOR_INPUT_REPORT *Rep)
{
	if (g_hFile != NULL)
	{
		DWORD BytesWritten = 0;

		return WriteFile(
			g_hFile,
			Rep,
			sizeof(*Rep),
			&BytesWritten,
			NULL
			);
	}
	return FALSE;
}

UCHAR GetUsage(LPINPUT Input)
{
	if (Input->ki.dwFlags & KEYEVENTF_EXTENDEDKEY)
	{
		// Can't handle these types of events
		return USAGE_NONE;
	}
	if (Input->ki.dwFlags & KEYEVENTF_UNICODE)
	{
		return UnicodeToKeyboardUsage(Input->ki.wScan);
	}
	else if (Input->ki.dwFlags & KEYEVENTF_SCANCODE)
	{
		return ScanCodeToKeyboardUsage(Input->ki.wScan);
	}
	else
	{
		return VKeyToKeyboardUsage(Input->ki.wVk);
	}

}

BOOL InjectKeyboardSingle(LPINPUT Input)
{
	BOOL ret = FALSE;
	KeyboardState.ReportId = KEYBOARD_REPORT_ID;	// TODO: this needs a better location
	UCHAR usage = GetUsage(Input);

	if (usage != USAGE_NONE)
	{
		BOOL sendDown = FALSE;
		BOOL sendUp = FALSE;

		// For unicode events, we get a single call for down and up.  
		// For vkey and scancode, we get individual down and up calls.
		if (Input->ki.dwFlags & KEYEVENTF_UNICODE)
		{
			sendDown = TRUE;
			sendUp = TRUE;
		}
		else if (Input->ki.dwFlags & KEYEVENTF_KEYUP)
		{
			sendUp = TRUE;
		}
		else
		{
			sendDown = TRUE;
		}

		if (sendDown)
		{
			if (SetKeybaordUsage(&KeyboardState, usage) &&
				SendHidReport(&KeyboardState))
			{
				ret = TRUE;
			}
		}

		if (sendUp)
		{
			if (ClearKeyboardUsage(&KeyboardState, usage) &&
				SendHidReport(&KeyboardState))
			{
				ret = TRUE;
			}
		}
	}

	return ret;
}

BOOL InjectMouseSingle(LPINPUT Input)
{
	BOOL ret = FALSE;
	MouseState.ReportId = MOUSE_REPORT_ID;	// TODO: this needs a better location

	// Can't do relative.  
	if (Input->mi.dwFlags & MOUSEEVENTF_ABSOLUTE == 0)
	{
		return FALSE;
	}

	if (Input->mi.dwFlags & MOUSEEVENTF_LEFTDOWN)
	{
		MouseState.Report.MouseReport.Buttons |= MOUSE_BUTTON_1;
	}
	if (Input->mi.dwFlags & MOUSEEVENTF_LEFTUP)
	{
		MouseState.Report.MouseReport.Buttons &= ~MOUSE_BUTTON_1;
	}
	if (Input->mi.dwFlags & MOUSEEVENTF_RIGHTDOWN)
	{
		MouseState.Report.MouseReport.Buttons |= MOUSE_BUTTON_2;
	}
	if (Input->mi.dwFlags & MOUSEEVENTF_RIGHTUP)
	{
		MouseState.Report.MouseReport.Buttons &= ~MOUSE_BUTTON_2;
	}

	// API accepts 0-65535, but driver expects 0-32767.  
	MouseState.Report.MouseReport.AbsoluteX = Input->mi.dx >> 1;
	MouseState.Report.MouseReport.AbsoluteY = Input->mi.dy >> 1;

	return SendHidReport(&MouseState);
}

BOOL InjectSendInputSingle(LPINPUT Input)
{

	if (Input->type == INPUT_KEYBOARD)
	{
		return InjectKeyboardSingle(Input);
	}
	else if (Input->type == INPUT_MOUSE)
	{
		return InjectMouseSingle(Input);
	}
	else
	{
		return FALSE;
	}
}


UINT InjectSendInput(
	_In_ UINT    nInputs,
	_In_ LPINPUT pInputs,
	_In_ int     cbSize
	)
{
	UINT c = 0;
	for (UINT i = 0; i < nInputs; i++)
	{
		if (InjectSendInputSingle(&pInputs[i]))
		{
			c++;
		}
	}
	return 0;
}

void InjectKeyDown(UCHAR vk)
{
	INPUT inp = { 0 };
	inp.type = INPUT_KEYBOARD;
	inp.ki.wVk = vk;
	InjectSendInput(1, &inp, sizeof(inp));
}

void InjectKeyUp(UCHAR vk)
{
	INPUT inp = { 0 };
	inp.type = INPUT_KEYBOARD;
	inp.ki.dwFlags = KEYEVENTF_KEYUP;
	inp.ki.wVk = vk;
	InjectSendInput(1, &inp, sizeof(inp));

}

void InjectScanKeyDown(
	WORD scanCode
	)
{
	INPUT inp = { 0 };
	inp.type = INPUT_KEYBOARD;
	inp.ki.dwFlags = KEYEVENTF_SCANCODE;
	inp.ki.wScan = scanCode;
	InjectSendInput(1, &inp, sizeof(inp));
}

void InjectUnicode(
	WORD wch
	)
{
	INPUT inp = { 0 };
	inp.type = INPUT_KEYBOARD;
	inp.ki.dwFlags = KEYEVENTF_UNICODE;
	inp.ki.wScan = wch;
	InjectSendInput(1, &inp, sizeof(inp));
}

void InjectScanKeyUp(
	WORD scanCode
	)
{
	INPUT inp = { 0 };
	inp.type = INPUT_KEYBOARD;
	inp.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	inp.ki.wScan = scanCode;
	InjectSendInput(1, &inp, sizeof(inp));
}

void InjectMouseMove(
	WORD X,
	WORD Y,
	UINT Buttons
	)
{
	INPUT inp = { 0 };
	inp.type = INPUT_MOUSE;
	inp.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | Buttons;
	inp.mi.dx = X;
	inp.mi.dy = Y;

	InjectSendInput(1, &inp, sizeof(inp));
}

