#Build the SpeechTranslator Project

###Component Lists:

- 2 Rpi2 boards
- 2 Rpi2 Power Supplies 
- 2 [Headsets](https://www.microsoft.com/hardware/en-us/p/lifechat-lx-3000/JUG-00013) 
- 2 Mouses 
- 2 Ethernet cables 
- 2 SD cards 
- 2 HDMI monitors 
- 2 HDMI cable  
- 1 Router 
- 1 SD card reader


### Setup your hardware
- Follow the [instruction](http://ms-iot.github.io/content/en-US/win10/SetupRPI.htm) to flush your sd cards
- Insert the sd card and USB headsets into devices; Also Connect HDMI monitors and Ethernet cable to the devices; You can connect both Ethernet cable to the router, then the router will be connected to the Internet
- Once all connected, power the devices on
- You should see the device name and device ip on the screen
- Telnet/powershell into the device, 
  type: `setcomputername speechtransrpi2` to rename one device, and type `setcomputername speechtransrpi1` to rename the other device;

	Note: you can choose the other device name as well, but in constantParam.cs file(you will see this file once you download the sample and open the solution as below steps show), you need to match them up.
	You will see the details later.

- Restart the device: shutdown /r /t 0

### Setup your sample:


1. Download the sample from [here](https://github.com/ms-iot/samples/speechtranslator) to your local PC
2. Open the solution file in visual studio
3. Open the constantParam.cs file, you need to specify your own clientid and clientsecret

	Note: this is an azure account which you need to access the MS translation service in azure;
	Follow this [link](http://www.microsoft.com/en-us/translator/getstarted.aspx) to get a account, 
	And	once you get the account, replace the clientid and clientsecret with yours.

4. Build the solutions and deploy to devices;

	If you are going to deploy to the device `speechtransrpi1`, open the constantParam.cs file, make sure the serviceHostName is equal to `speechtransrpi2`;
	Then right click on the project, under property/debug, put the ip address of `speechtransrpi1`.
	

	If you are going to deploy to the device `speechtransrpi2`, open the constantParam.cs file, make sure the serviceHostName is equal to `speechtransrpi1`;
	Then right click on the project, under property/debug, put the ip address of `speechtransrpi2`.
	
	Basically, the demo works like a phone line. You need to specify the ServerHostName which is where you are going to send data to.
	
5. Once deployment is done, open the webb url which should be something like this: http://yourdeivceipaddress:8080, under App/Installed APP, choose the `speechtranslator` app, 
	then click start;
	Do this for the other deivce as well.
	
	Now you are done with repro the demo. and Feel free to try out the demo to see how it works

	

