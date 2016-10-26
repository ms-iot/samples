
#include <windows.h>
#include "winuser.h"
#include "HidInject.h"

UCHAR VKeyToKeyboardUsage(UCHAR vk)
{
	/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
	if (vk >= '1' && vk <= '9')
	{
		return vk - '1' + 0x1e;
	}
	else if (vk == '0')
	{
		return 0x27;
	}
	/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */
	else if (vk >= 'A' && vk <= 'Z')
	{
		return vk - 'A' + 0x4;
	}
	else switch (vk)
	{
	case VK_LBUTTON:
	case VK_RBUTTON:
	case VK_CANCEL:
	case VK_MBUTTON:
		return USAGE_NONE;

	case VK_BACK:
		return 0x2a;
	case VK_TAB:
		return 0x2b;

	case VK_CLEAR:
		return USAGE_TODO;
	case VK_RETURN:
		return 0x28;

	case VK_SHIFT:
		return 0xe1;
	case VK_CONTROL:
		return 0xe0;
	case VK_MENU:
		return USAGE_TODO;
	case VK_PAUSE:
		return USAGE_TODO;
	case VK_CAPITAL:
		return 0x82;

	case VK_ESCAPE:
		return 0x29;

	case VK_SPACE:
		return 0x44;
	case VK_PRIOR:
		return 0x4b;
	case VK_NEXT:
		return 0x4e;
	case VK_END:
		return 0x4d;
	case VK_HOME:
		return 0x4a;
	case VK_LEFT:
		return 0x50;
	case VK_UP:
		return 0x52;
	case VK_RIGHT:
		return 0x4f;
	case VK_DOWN:
		return 0x51;
	case VK_SELECT:
		return USAGE_TODO;
	case VK_PRINT:
		return USAGE_TODO;
	case VK_EXECUTE:
		return USAGE_TODO;
	case VK_SNAPSHOT:
		return USAGE_TODO;
	case VK_INSERT:
		return USAGE_TODO;
	case VK_DELETE:
		return USAGE_TODO;
	case VK_HELP:
		return USAGE_TODO;

	case VK_LWIN:
		return 0xe3;
	case VK_RWIN:
		0xe7;
	case VK_APPS:
		return USAGE_TODO;

	case VK_NUMPAD0:
		return 0x62;
	case VK_NUMPAD1:
	case VK_NUMPAD2:
	case VK_NUMPAD3:
	case VK_NUMPAD4:
	case VK_NUMPAD5:
	case VK_NUMPAD6:
	case VK_NUMPAD7:
	case VK_NUMPAD8:
	case VK_NUMPAD9:
		return vk - VK_NUMPAD1 + 0x59;
	case VK_MULTIPLY:
		return USAGE_TODO;
	case VK_ADD:
		return USAGE_TODO;
	case VK_SEPARATOR:
		return USAGE_TODO;
	case VK_SUBTRACT:
		return USAGE_TODO;
	case VK_DECIMAL:
		return USAGE_TODO;
	case VK_DIVIDE:
		return USAGE_TODO;

	case VK_F1:
	case VK_F2:
	case VK_F3:
	case VK_F4:
	case VK_F5:
	case VK_F6:
	case VK_F7:
	case VK_F8:
	case VK_F9:
	case VK_F10:
	case VK_F11:
	case VK_F12:
		return vk - VK_F1 + 0x3a;

	case VK_F13:
	case VK_F14:
	case VK_F15:
	case VK_F16:
	case VK_F17:
	case VK_F18:
	case VK_F19:
	case VK_F20:
	case VK_F21:
	case VK_F22:
	case VK_F23:
	case VK_F24:
		return vk - VK_F13 + 0x68;

	case VK_NUMLOCK:
		return USAGE_TODO;
	case VK_SCROLL:
		return USAGE_TODO;

	case VK_LSHIFT:
		return 0xe1;
	case VK_RSHIFT:
		return 0xe5;
	case VK_LCONTROL:
		return 0xe0;
	case VK_RCONTROL:
		return 0xe4;
	case VK_LMENU:
		return USAGE_TODO;
	case VK_RMENU:
		return USAGE_TODO;

	case VK_OEM_1:
		return USAGE_TODO;
	case VK_OEM_5:
		return USAGE_TODO;
	case VK_OEM_PLUS:
		return USAGE_TODO;
	case VK_OEM_COMMA:
		return USAGE_TODO;
	case VK_OEM_MINUS:
		return USAGE_TODO;
	case VK_OEM_PERIOD:
		return USAGE_TODO;
	case VK_OEM_7:
		return USAGE_TODO;

	case VK_PROCESSKEY:
		return USAGE_TODO;

	case VK_ATTN:
		return USAGE_TODO;
	case VK_CRSEL:
		return USAGE_TODO;
	case VK_EXSEL:
		return USAGE_TODO;
	case VK_EREOF:
		return USAGE_TODO;
	case VK_PLAY:
		return USAGE_TODO;
	case VK_ZOOM:
		return USAGE_TODO;
	case VK_NONAME:
		return USAGE_TODO;
	case VK_PA1:
		return USAGE_TODO;
	case VK_OEM_CLEAR:
		return USAGE_TODO;
	default:
		return USAGE_NONE;
	}
}

UCHAR ScanCodeToKeyboardUsage(UCHAR code)
{
	switch (code)
	{
	case 42:	// shift
		return 0xe1;
	case 75:	// left
		return 0x50;
	case 72:	// up
		return 0x52;
	case 77:	// right
		return 0x4f;
	case 80:	// down
		return 0x51;
	default:
		return USAGE_NONE;
	}
}

UCHAR UnicodeToKeyboardUsage(WCHAR wch)
{
	if (wch > 0xff)
		return USAGE_NONE;
	CHAR ch = (CHAR)(wch & 0xff);

	if (ch >= 'A' && ch <= 'Z')
	{
		return ch - 'A' + 4;
	}
	else if (ch >= 'a' && ch <= 'z')
	{
		return ch - 'a' + 4;
	}
	else if (ch >= '1' && ch <= '9')
	{
		return ch - '1' + 0x1e;
	}
	else if (ch == '0')
	{
		return 0x27;
	}
	else switch (ch)
	{
	case '`':
	case '~':
		return 0x35;
	case '!':
		return 0x1e;
	case '@':
		return 0x1f;
	case '#':
		return 0x20;
	case '$':
		return 0x21;
	case '%':
		return 0x22;
	case '^':
		return 0x23;
	case '&':
		return 0x24;
	case '*':
		return 0x25;
	case '(':
		return 0x26;
	case ')':
		return 0x27;
	case '-':
	case '_':
		return 0x2d;
	case '=':
	case '+':
		return 0x2e;
	case '[':
	case '{':
		return 0x2f;
	case ']':
	case '}':
		return 0x30;
	case '\\':
	case '|':
		return 0x31;
	case ';':
	case ':':
		return 0x33;
	case '\'':
	case '\"':
		return 0x34;
	case ',':
	case '<':
		return 0x36;
	case '.':
	case '>':
		return 0x37;
	case '/':
	case '?':
		return 0x38;
	}
	return USAGE_NONE;
}


BOOL SetKeybaordUsage(HIDINJECTOR_INPUT_REPORT *Rep, UCHAR Usage)
{
	if (Rep->ReportId == KEYBOARD_REPORT_ID)
	{
		if (Usage >= 0xe0 && Usage <= 0xe7)
		{
			UCHAR mask = 1 << (Usage - 0xe0);
			Rep->Report.KeyReport.Modifiers |= mask;
		}
		else
		{
			if (Rep->Report.KeyReport.Key1 == 0)
			{
				Rep->Report.KeyReport.Key1 = Usage;
			}
			else if (Rep->Report.KeyReport.Key2 == 0)
			{
				Rep->Report.KeyReport.Key2 = Usage;
			}
			else if (Rep->Report.KeyReport.Key3 == 0)
			{
				Rep->Report.KeyReport.Key2 = Usage;
			}
			else if (Rep->Report.KeyReport.Key4 == 0)
			{
				Rep->Report.KeyReport.Key2 = Usage;
			}
			else
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;
}

BOOL ClearKeyboardUsage(HIDINJECTOR_INPUT_REPORT *Rep, UCHAR Usage)
{
	if (Rep->ReportId == KEYBOARD_REPORT_ID)
	{
		if (Usage >= 0xe0 && Usage <= 0xe7)
		{
			UCHAR mask = 1 << (Usage - 0xe0);
			Rep->Report.KeyReport.Modifiers &= ~mask;
		}
		else
		{
			if (Rep->Report.KeyReport.Key4 == Usage)
			{
				Rep->Report.KeyReport.Key4 = 0;
			}
			if (Rep->Report.KeyReport.Key3 == Usage)
			{
				Rep->Report.KeyReport.Key3 = Rep->Report.KeyReport.Key4;
				Rep->Report.KeyReport.Key4 = 0;
			}
			if (Rep->Report.KeyReport.Key2 == Usage)
			{
				Rep->Report.KeyReport.Key2 = Rep->Report.KeyReport.Key3;
				Rep->Report.KeyReport.Key3 = Rep->Report.KeyReport.Key4;
				Rep->Report.KeyReport.Key4 = 0;
			}
			if (Rep->Report.KeyReport.Key1 == Usage)
			{
				Rep->Report.KeyReport.Key1 = Rep->Report.KeyReport.Key2;
				Rep->Report.KeyReport.Key2 = Rep->Report.KeyReport.Key3;
				Rep->Report.KeyReport.Key3 = Rep->Report.KeyReport.Key4;
				Rep->Report.KeyReport.Key4 = 0;
			}
			else
			{
				return FALSE;
			}
		}
	}
	else
	{
		return FALSE;
	}
	return TRUE;

}