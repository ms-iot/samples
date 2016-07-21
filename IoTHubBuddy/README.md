IoTHub Buddy
==============

## Device to Device communication via IoTHub

This sample consists of two projects, one that runs on the Raspberry Pi and one that can run on any other Windows device. A blog post outlining the implementation of this project will be coming soon.
Note: You must have an Azure subscription and a device associated with an IoTHub instance. You can set up your Azure account [here](http://portal.azure.com). 

## IoTHub Buddy Client
A UWP app that runs on an IoT device. This app is responsible for sending messages to the IoTHub associated with the device. In this app, the message data consists of latitude/longitude coordinates and a timestamp. Users have an option of sending 1, 10, or 100 messages at a time. The app uses the Microsoft.Azure.Devices.Client library to send messages to Azure.

## IotHub Buddy
A UWP app that runs on any Windows device. This app is responsible for receiving messages from the IoT device. This app will receive the message, parse the data, and then display the IoT device's location on a map. 
Known Bug: Right now, authentication has a lot of bugs. Authentication is necessary to get access to a user's IoTHub/devices tied to the IoTHub. If your azure subscription is not tied to a work/school account, please follow the instructions in MainPage.xaml.cs to circumvent this bug. 
This app uses the Microsoft.Azure.DeviceClient library to gain access to the IoTHub data and AmqpNetLite to receive messages. 