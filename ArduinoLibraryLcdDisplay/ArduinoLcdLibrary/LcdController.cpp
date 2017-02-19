// Copyright (c) Microsoft. All rights reserved.
#include "LcdController.h"


namespace ArduinoLcdLibrary
{
	LcdController::LcdController()
	{
		setup();
	}

	void LcdController::PrintLine(int lineNumber, Platform::String^ text)
	{
		char chars[512];
		const wchar_t* wide_chars = text->Data();
		wcstombs(chars, wide_chars, 512);
		printLine(lineNumber, chars);
	}
}