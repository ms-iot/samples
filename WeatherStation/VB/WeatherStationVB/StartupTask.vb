' Copyright (c) Microsoft. All rights reserved.

Imports System
Imports System.Collections.Generic
Imports System.Linq
Imports System.Text
Imports System.Net.Http
Imports Windows.ApplicationModel.Background
Imports Windows.Devices.Enumeration
Imports Windows.Devices.I2C
Imports Windows.System.Threading
' The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

Public NotInheritable Class StartupTask
	Implements IBackgroundTask
    Dim deferral As BackgroundTaskDeferral
    Dim sensor As I2cDevice
    Dim timer As ThreadPoolTimer
    Public Async Sub Run(taskInstance As IBackgroundTaskInstance) Implements IBackgroundTask.Run
        deferral = taskInstance.GetDeferral()
        Dim controller As I2cController
        controller = Await I2cController.GetDefaultAsync()
        sensor = controller.GetDevice(New I2cConnectionSettings(&H40))
        timer = ThreadPoolTimer.CreatePeriodicTimer(AddressOf Timer_Tick, TimeSpan.FromSeconds(2))

    End Sub

    Public Sub Timer_Tick(timer As ThreadPoolTimer)
        Dim tempCommand(0 To 0) As Byte
        tempCommand(0) = &HE3
        Dim tempData(0 To 1) As Byte
        sensor.WriteRead(tempCommand, tempData)
        Dim byte0 As Int32 = tempData(0)
        Dim byte1 As Int32 = tempData(1)
        Dim rawReading As Short = byte0 << 8 Or byte1
        Dim tempRatio As Double = rawReading / 65536.0
        Dim temperature As Double = (-46.85 + (175.72 * tempRatio)) * 9.0 / 5.0 + 32
        System.Diagnostics.Debug.WriteLine("Temp: " + temperature.ToString())

        Dim humidityCommand(0 To 0) As Byte
        humidityCommand(0) = &HE5
        Dim humidityData(0 To 1) As Byte
        sensor.WriteRead(humidityCommand, humidityData)
        byte0 = humidityData(0)
        byte1 = humidityData(1)
        rawReading = byte0 << 8 Or byte1
        Dim humidityRatio As Double = rawReading / 65536.0
        Dim humidity As Double = -6 + (125 * humidityRatio)
        System.Diagnostics.Debug.WriteLine("Humidity: " + humidity.ToString())

    End Sub
End Class
