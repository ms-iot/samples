// Copyright (c) Microsoft. All rights reserved.

// The Arduino Wiring application template is documented at 
// http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

void setup()
{
    // put your setup code here, to run once:

    pinMode(GPIO5, OUTPUT);
}


void ledOn() 
{
	digitalWrite(GPIO5, HIGH);
}

void ledOff()
{
	digitalWrite(GPIO5, LOW);
}