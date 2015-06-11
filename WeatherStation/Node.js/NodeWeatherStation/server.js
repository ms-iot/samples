// Copyright (c) Microsoft. All rights reserved.


var http = require('http');


//Import WinRT into Node.JS
var uwp = require("uwp");
uwp.projectNamespace("Windows");

var i2cDevice;


//Find the device: same code in other project, except in JS instead of C#
var aqs = Windows.Devices.I2c.I2cDevice.getDeviceSelector("I2C1");

Windows.Devices.Enumeration.DeviceInformation.findAllAsync(aqs, null).done(function (dis) {
    Windows.Devices.I2c.I2cDevice.fromIdAsync(dis[0].id, new Windows.Devices.I2c.I2cConnectionSettings(0x40)).done(function (device) {
        i2cDevice = device;
    });
});


http.createServer(function (req, res) {
    res.writeHead(200, { 'Content-Type': 'text/plain' });
    var output = "";
    var humidity = 0;
    
    //Read the humidity from the sensor
    var command = new Array(1);
    command[0] = 0xE5;
    var data = new Array(2);
    i2cDevice.writeRead(command, data);
    var rawhumidityReading = data[0] << 8 | data[1];
    var ratio = rawhumidityReading / 65536.0;
    humidity = -6 + (125 * ratio);
    
    //Read the temperature from the sensor
    var tempCommand = new Array(1);
    command[0] = 0xE3;
    var tempData = new Array(2);
    i2cDevice.writeRead(command, data);
    var rawTempReading = data[0] << 8 | data[1];
    var tempRatio = rawTempReading / 65536.0;
    var temperature = (-46.85 + (175.72 * tempRatio)) * 9 / 5 + 32;

    output = "Humidity: " + humidity + ", Temperature: " + temperature;
    res.end(output);
}).listen(1337);