This sample shows how to easily build an "AppService" that can communicate with
 other UWP applications.

Blinky Service:
    It is UWP app service to switch on/off LED by communicating with Gpio library. 
On request received, it parses the PinNumber and SetLeadState values from the request object .
If SetLedState key value equals to SetLedStateOff, then it switch off the LED connected to requested PinNumber
If SetLedState key value equals to SetLedStateOn, then it switch on the LED connected to requested PinNumber

Blinky Client:
    It is UWP background task to request app service (i.e. blinky service) 
indefininitely for blinking LED.

To run this sample first run the server and then the client. However, as long as the 
server is deployed to machine, running the client will cause it to automatically activate. 
Assuming that Gpio Pin 5 is available on the device.
