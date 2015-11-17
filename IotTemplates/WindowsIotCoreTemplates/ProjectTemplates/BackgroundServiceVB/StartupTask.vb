Imports System
Imports System.Collections.Generic
Imports System.Linq
Imports System.Text
Imports System.Net.Http
Imports Windows.ApplicationModel.Background

' The Background Application template is documented at http://go.microsoft.com/fwlink/?LinkID=533884&clcid=0x409

Public NotInheritable Class StartupTask
    Implements IBackgroundTask

    Public Sub Run(taskInstance As IBackgroundTaskInstance) Implements IBackgroundTask.Run

        ' 
        ' TODO: Insert code to perform background work
        '
        ' If you start any asynchronous methods here, prevent the task
        ' from closing prematurely by using BackgroundTaskDeferral as
        ' described in http://aka.ms/backgroundtaskdeferral
        '

    End Sub
End Class
