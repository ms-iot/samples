var uwp = require('uwp');
uwp.projectNamespace('Windows');

var BUTTON_PIN = 5;
var LED_PIN = 6;

var ledPin;
var buttonPin;
var ledPinValue = Windows.Devices.Gpio.GpioPinValue.high;

(function () { 
    initGPIO();
})();

function initGPIO(){
    var gpio = Windows.Devices.Gpio.GpioController.getDefault();
    
    // Log an error if there is no GPIO controller
    if (gpio == null) {
        console.log('There is no GPIO controller on this device.');
        return;
    }
    
    var buttonPinResult = gpio.tryOpenPin(BUTTON_PIN, Windows.Devices.Gpio.GpioSharingMode.exclusive);
    var ledPinResult = gpio.tryOpenPin(LED_PIN, Windows.Devices.Gpio.GpioSharingMode.exclusive);
    
    // Log an error if the GPIO pins can not be opened
    // GpioOpenStatus enumeration - https://msdn.microsoft.com/en-us/library/windows/apps/windows.devices.gpio.gpioopenstatus.aspx
    if (!buttonPinResult.succeeded) {
        console.log('Could not open GPIO pin ' + BUTTON_PIN + '. Result returned was: ' + buttonPinResult.openStatus);
        return;
    }
    else if (!ledPinResult.succeeded) {
        console.log('Could not open GPIO pin ' + LED_PIN + '. Result returned was: ' + ledPinResult.openStatus);
        return;
    }
    
    buttonPin = buttonPinResult.pin;
    ledPin = ledPinResult.pin;
    
    // Initialize LED to the OFF state by first writing a HIGH value
    // We write HIGH because the LED is wired in a active LOW configuration
    ledPin.write(Windows.Devices.Gpio.GpioPinValue.high);
    ledPin.setDriveMode(Windows.Devices.Gpio.GpioPinDriveMode.output);
    
    // Check if inputpull - up resistors are supported
    if (buttonPin.isDriveModeSupported(Windows.Devices.Gpio.GpioPinDriveMode.inputPullUp))
        buttonPin.setDriveMode(Windows.Devices.Gpio.GpioPinDriveMode.inputPullUp);
    else
        buttonPin.setDriveMode(Windows.Devices.Gpio.GpioPinDriveMode.input);
    
    // Set a debounce timeout to filter out switch bounce noise from a button press
    buttonPin.debounceTimeout = 50;
    
    // Register for the ValueChanged event so our buttonPin_ValueChanged 
    // function is called when the button is pressed
    buttonPin.onvaluechanged = buttonPin_ValueChanged;

    console.log('GPIO pins initialized correctly.');
}

function buttonPin_ValueChanged(sender, e){
    if (e.edge == Windos.Device.Gpio.GpioPinEdge.FallingEdge) {
        ledPinValue = (ledPinValue == Windows.Devices.Gpio.GpioPinValue.low) ? Windows.Devices.Gpio.GpioPinValue.high : Windows.Devices.Gpio.GpioPinValue.low;
        ledPin.write(ledPinValue);
        console.log('Button Pressed');
    }
    else {
        console.log('Button Released');
    }
}