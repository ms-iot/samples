#Build the SpeechTranslator Project

###Component Lists:

- 2 Raspberry Pi 2 boards
- 2 Raspberry Pi 2 Power Supplies 
- 2 [Microsoft LifeChat-3000 Headset](https://www.microsoft.com/hardware/en-us/p/lifechat-lx-3000/JUG-00013) 
- 2 Mice 
- 2 Ethernet cables 
- 2 micro-SD cards 
- 2 HDMI monitors 
- 2 HDMI cables 
- 1 Router 
- 1 micro-SD card reader


### Setup your hardware
- Follow the [instruction](http://ms-iot.github.io/content/en-US/win10/SetupRPI.htm) to flush your micro-SD cards
- Repeat the above to setup the other device
- Once the two devices both boots up, you should see the device name and device ip on the screen; Be sure to connect two devices to the router, then the router will be connected to Internet.
- SSH/Telnet/powershell into the device, 
  type: `setcomputername speechtransrpi2` to rename one device, and type `setcomputername speechtransrpi1` to rename the other device;

	Note: you can choose the other device name as well, but in constantParam.cs file(you will see this file once you download the sample and open the solution as below steps show), you need to match them up.
	You will see the details later.

- Restart the device: shutdown /r /t 0

### Setup your sample:


1. Download the sample from [here](https://github.com/ms-iot/samples/archive/develop.zip) to your local PC
2. Open the solution file in visual studio
3. Open the constantParam.cs file, you need to specify your own azure clientid and clientsecret

	Note: You need an account to access the MS translation service in azure;
	Follow this [link](http://www.microsoft.com/en-us/translator/getstarted.aspx) to get one, 
	Once you get it, replace the clientid and clientsecret with yours.

4. Build the solutions 
5. Deploy to devices;

	If you are going to deploy to the device `speechtransrpi1`, In ConstantParam.cs, line 3, make sure it reads as ````#define RPI1````
	Then right click on the project, under property/debug, put the ip address of `speechtransrpi1`.
	
	If you are going to deploy to the device `speechtransrpi2`, In ConstantParam.cs, line 3, make sure it reads as ````#define RPI2````
	Then right click on the project, under property/debug, put the ip address of `speechtransrpi2`.
	
	Basically, the demo works like a phone line. You need to specify the ServerHostName which is where you are going to send data to. That is what 	`Define` is doing here.
	
6. Once deployment is done, open the webb url which should be something like this: http://yourdeivceipaddress:8080, under App/Installed APP, choose the `speechtranslator` app, 
	then click start;
	Do this for the other deivce as well.
	
	Now you are ready to use the speech translator!

	

