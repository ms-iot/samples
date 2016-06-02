# Windows 10 IoT GoPiGoXbox Sample

This sample enable you to control your GoPiGo with your Xbox controller.  It demonstrates how to read inputs from the Xbox controller, translate these inputs as commands ('move forward',  ' move backward', etc) and issue these commands to the GoPiGo robot.


### Project Description

This sample contains two projects: XboxControllerClient and GoPiGoXboxWebService.  The XboxControllerClient is a foreground application written in C#.  It is responsible for reading the Xbox controller's input and send it to GoPiGoXboxWebService.  GoPiGoXboxWebService is a background application written in python.  It exposes a WebAPI which takes the controller's input and issues commands to the GoPiGo robot.

Since XboxControllerClient is an universal application, it can be run locally on the Raspberry Pi or remotely on a PC.  This is especially useful in the case of a wired Xbox controller -- you can connect the Xbox control to the PC and still control the Raspberry Pi / GoPiGo Robot via wifi.  See configuration 2 - Running remotely.

### Requirements

1. Raspberry Pi 2 or Raspberry Pi 3
2. Dexter Industries GoPiGo Kit (http://www.dexterindustries.com/GoPiGo/)
3. Xbox 360 controller for PC (wired or wireless)

### Setup Instructions
1. Set up your Raspberry Pi 2 or 3 running [Windows 10 IoT](https://developer.microsoft.com/en-us/windows/iot/getstarted)
2. Assemble your GoPiGo kit following Dexter Industries [instructions](http://www.dexterindustries.com/GoPiGo/getting-started-with-your-gopigo-raspberry-pi-robot-kit-2/1-assemble-the-gopigo-2/assemble-gopigo-raspberry-pi-robot/1-assemble-the-gopigo2/)
3. Setup your PC based on the instructions [here](https://developer.microsoft.com/en-us/windows/iot/win10/samples/python)
4. Power on your Raspberry Pi along with the GoPiGo Robot.  
5. Deploy and run the GoPiGoXboxWebService application to the Raspberry Pi.


#### Configuration 1 - Running locally

1. Connect the Xbox controller to the Raspberry Pi.
2. Deploy and run the XboxControllerClient.
3. Use the left thumb stick or the d-pad to control the GoPiGo Robot.

#### Configuration 2 - Running remotely

1. Connect the Xbox controller to your PC
2. Connect your Raspberry Pi to your wireless network.
3. On line 23 of `XboxControllerClient\MainPage.xaml.cs`, change the `localhost` to the IP of your Raspberry Pi.
3. Deploy and run XboxControllerClient on your PC
4. Use the left thumb stick or the d-pad to control the GoPiGo Robot.
