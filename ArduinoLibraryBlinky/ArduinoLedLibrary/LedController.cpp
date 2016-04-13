// Copyright (c) Microsoft. All rights reserved.
#include "LedController.h"

namespace ArduinoLedLibrary
{

	LedController::LedController()
	{
		setup();
	}

	void LedController::LedOn()
	{
		ledOn();
	}


	void LedController::LedOff()
	{
		ledOff();
	}

}