// Copyright (c) Microsoft. All rights reserved.
#pragma once

void setup();
void printLine(int row, char text[]);

namespace ArduinoLcdLibrary
{
	public ref class LcdController sealed
	{
	public:
		LcdController();
		void PrintLine(int lineNumber, Platform::String^ text);
	};

}