// Copyright (c) Microsoft. All rights reserved.
// The Arduino Wiring application template is documented at 
// http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

#include <LiquidCrystal.h>
LiquidCrystal *lcd;

void setup()
{
	//Configure the display with the pins used and specify a 16 column by 2 row display
	lcd = new LiquidCrystal(GPIO20, GPIO16, GPIO2, GPIO3, GPIO4, GPIO17);
	lcd->begin(16, 2);

	lcd->print("LCD Initialized");
}



void printLine(int row, char text[])
{
	lcd->setCursor(0, row);
	lcd->print(text);

}