// Copyright (c) Microsoft. All rights reserved.

var http = require('http');

var uwp = require("uwp");
uwp.projectNamespace("Windows");

var gpioController = Windows.Devices.Gpio.GpioController.getDefault();
var pin = gpioController.openPin(5);
var currentValue = Windows.Devices.Gpio.GpioPinValue.high;
pin.write(currentValue);
pin.setDriveMode(Windows.Devices.Gpio.GpioPinDriveMode.output)


http.createServer(function (req, res) {
    if (currentValue == Windows.Devices.Gpio.GpioPinValue.high) {
        currentValue = Windows.Devices.Gpio.GpioPinValue.low;
    } else {
        currentValue = Windows.Devices.Gpio.GpioPinValue.high;
    }
    pin.write(currentValue);
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    res.end('LED value: ' + currentValue + '\n');
}).listen(1337);

uwp.close();