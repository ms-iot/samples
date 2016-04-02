# Windows 10 IoT Home App Sample

This sample demonstrates how one can develop a home app for Windows 10 IoT Core. 
The sample consist of two sets of app showing two different approaches:

## IoTHomeApp

Demonstrates how one can use LaunchUriAsync to launch other apps registered for a 'protocol'.
The solution folder contains OemApp1 and OemApp2 projects that supports protocol 'oemapp1' and 'oemapp2'.
The app also shows how to resolve ambiguity, in case of multiple apps registered for a protocol, by providing the App Id of the target application in the Launcher options.

OemApp1 amd OemApp2 must be built and deployed before one can use IotHomeApp to launch them.

## IoTStartApp

IotStartApp illustrates how one can use PackageManager APIs to enumerate and launch apps installed on the system, just like a Start screen.
In order to use the PackageManager APIs, it uses a restricted capability. This may make it difficult to pass the Store certification process.

However, one can always sideload the app onto the device.