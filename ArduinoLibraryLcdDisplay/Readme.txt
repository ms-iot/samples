This sample shows some of the power of being able to create WinRT libraries in an Arduino Wiring project and use those libraries from other UWP apps. In this case the Arduino Wiring app exposes basic APIs to control a 2-line LCD display. Arduino Wiring has built in libraries for these displays and so it is only a few lines of code, where as there is no existing UWP library for these and it would normally be hundreds of lines of code to duplicate in C# or your other language of choice. 

By building a small WinRT library in this Arduino Wiring project a C# app is able to reference it and easily get access to the hardware. The code in the C# app is actually slightly smaller than in our C# headless blinky sample. 

For full instructions on how to wire up the LCD display, see this sample: https://developer.microsoft.com/en-us/windows/iot/win10/samples/arduino-wiring/lcdscreen

For general information and instructions for creating Arduino Wiring projects, see this page: https://developer.microsoft.com/en-us/windows/iot/win10/arduinowiringprojectguide