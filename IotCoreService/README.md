Windows 10 IoT Core Service Sample
==============
This sample creates a simple shared host service for Windows 10 IoT Core.

###Usage
1. Download this sample from [here](https://github.com/ms-iot/samples/archive/develop.zip).
2. Open IotCoreService.sln, select the appropriate platform and build the project.  The path containing IotCoreService.dll will be displayed in your Visual Studio Output view.  It will look something like this: IotCoreService.vcxproj -> D:\samples-develop\IotCoreService\ARM\Debug\IotCoreService.dll.
3. Copy IotCoreService.dll to your device.  You can connect and make modifications to your device when it is running by opening Windows Explorer and entering \\<IP address or device name>\C$\Windows\System32.
4. To configure your service, copy ConfigureIotCoreService.bat to your device, connect to your device using SSH or Powershell, and run ConfigureIotCoreService.bat.  This will update the registry to enable IotCoreService on boot.
5. Restart your device.
6. Query your service by running `sc query IotCoreService`.  Given the configuration we've set up, you should see output like this:

```
    SERVICE_NAME: iotcoreservice
            TYPE               : 20  WIN32_SHARE_PROCESS
            STATE              : 2  START_PENDING
                                    (NOT_STOPPABLE, NOT_PAUSABLE, IGNORES_SHUTDOWN)
            WIN32_EXIT_CODE    : 0  (0x0)
            SERVICE_EXIT_CODE  : 0  (0x0)
            CHECKPOINT         : 0x0
            WAIT_HINT          : 0x7d0
```

7. At this point, ServiceMain has been called.  There is no functionality presently in ServiceMain, but you can consult the [MSDN documentation](https://msdn.microsoft.com/en-us/library/windows/desktop/ms687414(v=vs.85).aspx) to add functionality as desired!
8. To stop your service, run `ConfigureIotCoreService.bat remove` to remove the service configuration and restart your device.
