# “Hello, blinky!”

We’ll create a simple LED blinking app and connect a LED to your Windows 10 IoT Core device.

This is a headed sample. To better understand what headed mode is and how to configure your device to be headed, follow the instructions [here](/en-us/windows/iot/Docs/HeadlessMode).

Also, be aware that the GPIO APIs are only available on Windows 10 IoT Core, so this sample cannot run on your desktop.

## Load the project in Visual Studio

* * *

## Connect the LED to your Windows IoT device

* * *

You’ll need a few components:

*   a LED (any color you like)

*   a 220 ? resistor for the Raspberry Pi 2, Raspberry Pi 3 and the MinnowBoard Max or a 330 ? resistor for the DragonBoard

*   a breadboard and a couple of connector wires

![Electrical Components](https://az835927.vo.msecnd.net/sites/iot/Resources/images/Blinky/components.png)

### For Raspberry Pi 2 or 3 (RPi2 or RPi3)

1.  Connect the shorter leg of the LED to GPIO 5 (pin 29 on the expansion header) on the RPi2 or RPi3.
2.  Connect the longer leg of the LED to the resistor.
3.  Connect the other end of the resistor to one of the 3.3V pins on the RPi2 or RPi3.
4.  Note that the polarity of the LED is important. (This configuration is commonly known as Active Low)

And here is the pinout of the RPi2 and RPi3:

![](https://az835927.vo.msecnd.net/sites/iot/Resources/images/PinMappings/RP2_Pinout.png)

Here is an example of what your breadboard might look like with the circuit assembled:

![Image made with Fritzing(http://fritzing.org/)](https://az835927.vo.msecnd.net/sites/iot/Resources/images/Blinky/breadboard_assembled_rpi2_kit.jpg)

### For MinnowBoard Max (MBM)

We will connect the one end of the LED to GPIO 5 (pin 18 on the JP1 expansion header) on the MBM, the other end to the resistor, and the resistor to the 3.3 volt power supply from the MBM. Note that the polarity of the LED is important. Make sure the shorter leg (-) is connected to GPIO 5 and the longer leg (+) to the resistor or it wont light up.

And here is the JP1 connector on the MBM:

![](https://az835927.vo.msecnd.net/sites/iot/Resources/images/PinMappings/MBM_Pinout.png)

Here is an example of what your breadboard might look like with the circuit assembled:

![Image made with Fritzing(http://fritzing.org/)](https://az835927.vo.msecnd.net/sites/iot/Resources/images/Blinky/breadboard_assembled.png)

### For DragonBoard 410c (DB)

For reference, the functionality of the low-speed expansion connector is outlined in the following diagram

![](https://az835927.vo.msecnd.net/sites/iot/Resources/images/PinMappings/DB_Pinout.png)

Perform the following steps to create the circuit:

1.  Connect the shorter leg of the LED to GPIO 12 (pin 24 on the expansion header) on the DB.
2.  Connect the longer leg of the LED to the resistor.
    *   Note that the polarity of the LED is important (this configuration is commonly known as Active Low).
3.  Connect the other end of the resistor to 1.8V (pin 35 on the expansion header).

Here is an illustration of what your breadboard might look like with the circuit assembled:

![Image made with Fritzing(http://fritzing.org/)](https://az835927.vo.msecnd.net/sites/iot/Resources/images/Blinky/breadboard_assembled_db_kit.png)

Finally, the LED_PIN variable of _MainPage.xaml.cs_ file of the sample code will need the following modification:

	private const int LED_PIN = 12;

## Deploy your app

* * *

1.  With the application open in Visual Studio, set the architecture in the toolbar dropdown. If you’re building for MinnowBoard Max, select `x86`. If you’re building for Raspberry Pi 2 or 3 or the DragonBoard, select `ARM`.

2.  Next, in the Visual Studio toolbar, click on the `Local Machine` dropdown and select `Remote Machine`

    ![RemoteMachine Target](https://az835927.vo.msecnd.net/sites/iot/Resources/images/AppDeployment/cs-remote-machine-debugging.png)

3.  At this point, Visual Studio will present the **Remote Connections** dialog. If you previously used [PowerShell](/en-us/windows/iot/Docs/PowerShell) to set a unique name for your device, you can enter it here (in this example, we’re using **my-device**). Otherwise, use the IP address of your Windows IoT Core device. After entering the device name/IP select `Universal` for Windows Authentication, then click **Select**.

    ![Remote Machine Debugging](https://az835927.vo.msecnd.net/sites/iot/Resources/images/AppDeployment/cs-remote-connections.PNG)

4.  You can verify or modify these values by navigating to the project properties (select **Properties** in the Solution Explorer) and choosing the `Debug` tab on the left:

    ![Project Properties Debug Tab](https://az835927.vo.msecnd.net/sites/iot/Resources/images/AppDeployment/cs-debug-project-properties.PNG)

When everything is set up, you should be able to press F5 from Visual Studio. If there are any missing packages that you did not install during setup, Visual Studio may prompt you to acquire those now. The Blinky app will deploy and start on the Windows IoT device, and you should see the LED blink in sync with the simulation on the screen.

![](https://az835927.vo.msecnd.net/sites/iot/Resources/images/Blinky/blinky-screenshot.png)

Congratulations! You controlled one of the GPIO pins on your Windows IoT device.

## Let’s look at the code

* * *

The code for this sample is pretty simple. We use a timer, and each time the ‘Tick’ event is called, we flip the state of the LED.

### Timer code

Here is how you set up the timer in C#:


	public MainPage()
	{
		// ...

		timer = new DispatcherTimer();
		timer.Interval = TimeSpan.FromMilliseconds(500);
		timer.Tick += Timer_Tick;
		InitGPIO();
		if (pin != null)
		{
			timer.Start();
		}

		// ...
	}

	private void Timer_Tick(object sender, object e)
	{
		if (pinValue == GpioPinValue.High)
		{
			pinValue = GpioPinValue.Low;
			pin.Write(pinValue);
			LED.Fill = redBrush;
		}
		else
		{
			pinValue = GpioPinValue.High;
			pin.Write(pinValue);
			LED.Fill = grayBrush;
		}
	}

### Initialize the GPIO pin

To drive the GPIO pin, first we need to initialize it. Here is the C# code (notice how we leverage the new WinRT classes in the Windows.Devices.Gpio namespace):


	using Windows.Devices.Gpio;

	private void InitGPIO()
	{
		var gpio = GpioController.GetDefault();

		// Show an error if there is no GPIO controller
		if (gpio == null)
		{
			pin = null;
			GpioStatus.Text = "There is no GPIO controller on this device.";
			return;
		}

		pin = gpio.OpenPin(LED_PIN);
		pinValue = GpioPinValue.High;
		pin.Write(pinValue);
		pin.SetDriveMode(GpioPinDriveMode.Output);

		GpioStatus.Text = "GPIO pin initialized correctly.";

	}

Let’s break this down a little:

*   First, we use `GpioController.GetDefault()` to get the GPIO controller.

*   If the device does not have a GPIO controller, this function will return `null`.

*   Then we attempt to open the pin by calling `GpioController.OpenPin()` with the `LED_PIN` value.

*   Once we have the `pin`, we set it to be off (High) by default using the `GpioPin.Write()` function.

*   We also set the `pin` to run in output mode using the `GpioPin.SetDriveMode()` function.

### Modify the state of the GPIO pin

Once we have access to the `GpioOutputPin` instance, it’s trivial to change the state of the pin to turn the LED on or off.

To turn the LED on, simply write the value `GpioPinValue.Low` to the pin:

	pin.Write(GpioPinValue.Low);

and of course, write `GpioPinValue.High` to turn the LED off:

	pin.Write(GpioPinValue.High);


Remember that we connected the other end of the LED to the 3.3 Volts power supply, so we need to drive the pin to low to have current flow into the LED.

## Additional resources
* [Windows 10 IoT Core home page](https://developer.microsoft.com/en-us/windows/iot/)
* [Documentation for all samples](https://developer.microsoft.com/en-us/windows/iot/samples)

This project has adopted the Microsoft Open Source Code of Conduct. For more information see the Code of Conduct FAQ or contact <opencode@microsoft.com> with any additional questions or comments.
