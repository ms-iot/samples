Windows 10 IoT Core sample code
===============

[Documentation for this sample](https://developer.microsoft.com/en-us/windows/iot/samples/arduino-wiring/arduinowiringcomponents) 

This sample shows some of the power of being able to create WinRT libraries in an Arduino Wiring project and use those libraries from other UWP apps. In this case the Arduino Wiring app exposes basic APIs to control a 2-line LCD display. Arduino Wiring has built in libraries for these displays and so it is only a few lines of code, where as there is no existing UWP library for these and it would normally be hundreds of lines of code to duplicate in C# or your other language of choice. 

By building a small WinRT library in this Arduino Wiring project a C# app is able to reference it and easily get access to the hardware. The code in the C# app is actually slightly smaller than in our C# headless blinky sample. 

For full instructions on how to wire up the LCD display, see this sample: https://developer.microsoft.com/en-us/windows/iot/samples/arduino-wiring/lcdtextdisplay


## How to download:

Unfortunately, GitHub does not support downloading individual code. 

Navigate to [ms-iot/samples](https://github.com/ms-iot/samples) and select **Clone or download** to download the whole repository.


## Additional resources
* [Windows 10 IoT Core home page](https://developer.microsoft.com/en-us/windows/iot/)
* [Documentation for all samples](https://developer.microsoft.com/en-us/windows/iot/samples)

This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact opencode@microsoft.com with any additional questions or comments.



For general information and instructions for creating Arduino Wiring projects, see this page: https://developer.microsoft.com/en-us/windows/iot/win10/arduinowiringprojectguide