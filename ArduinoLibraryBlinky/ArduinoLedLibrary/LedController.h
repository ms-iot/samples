// Copyright (c) Microsoft. All rights reserved.
#pragma once

void setup();
void ledOn();
void ledOff();

namespace ArduinoLedLibrary
{
	public ref class LedController sealed
	{
	public:
		LedController();
		void LedOn();
		void LedOff();
	};

}