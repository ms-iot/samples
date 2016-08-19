// Copyright (c) Microsoft. All rights reserved.


var http = require('http');

var uwp = require("uwp");
uwp.projectNamespace("Windows");

var gpioController = Windows.Devices.Gpio.GpioController.getDefault();
var pin = gpioController.openPin(5);
var currentValue = Windows.Devices.Gpio.GpioPinValue.high;
pin.write(currentValue);
pin.setDriveMode(Windows.Devices.Gpio.GpioPinDriveMode.output);
setTimeout(flipLed, 500);


function flipLed(){
    if (currentValue == Windows.Devices.Gpio.GpioPinValue.high) {
        currentValue = Windows.Devices.Gpio.GpioPinValue.low;
    } else {
        currentValue = Windows.Devices.Gpio.GpioPinValue.high;
    }
    pin.write(currentValue);
    setTimeout(flipLed, 500);
}