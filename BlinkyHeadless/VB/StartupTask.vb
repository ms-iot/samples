' Copyright (c) Microsoft. All rights reserved.

Imports System
Imports System.Collections.Generic
Imports System.Linq
Imports System.Text
Imports System.Net.Http
Imports Windows.ApplicationModel.Background
Imports Windows.Devices.Gpio

' The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

Public NotInheritable Class StartupTask
	Implements IBackgroundTask

    Dim pin As GpioPin
    Dim deferral As BackgroundTaskDeferral
    Dim timer As Windows.System.Threading.ThreadPoolTimer
    Dim lightOn As Boolean

    Public Sub Run(taskInstance As IBackgroundTaskInstance) Implements IBackgroundTask.Run
        deferral = taskInstance.GetDeferral()
        pin = GpioController.GetDefault().OpenPin(5)
        pin.SetDriveMode(GpioPinDriveMode.Output)
        lightOn = False
        Windows.System.Threading.ThreadPoolTimer.CreatePeriodicTimer(AddressOf Tick, TimeSpan.FromMilliseconds(500))

    End Sub

    Public Sub Tick(timer As Windows.System.Threading.ThreadPoolTimer)
        If (lightOn) Then
            pin.Write(GpioPinValue.High)
            lightOn = False
        Else
            pin.Write(GpioPinValue.Low)
            lightOn = True
        End If
    End Sub

End Class
