@echo off
setlocal enabledelayedexpansion
if "%1"=="/?" goto Help
if "%1"=="-?" goto Help


if /I "%1"=="remove" (
    reg delete "hklm\system\CurrentControlSet\Services\IotCoreService" /f
    reg add "hklm\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost" /v LocalSystemNetworkRestricted /t REG_MULTI_SZ /d " hidserv\0Netman\0UpdateManagerSvc\0DevQueryBroker\0AudioEndpointBuilder\0NcbService\0WUDFSvc\0wlansvc\0SmsRouter\0NgcSvc\0DsSvc\0DeviceAssociationService\0WwanSvc\0SensorService\0sysmain" /f
    exit /b 0
)


reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v DisplayName /t reg_sz /d "IotCoreService" /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v ErrorControl /t reg_dword /d 1 /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v ImagePath /t REG_EXPAND_SZ  /d "%SystemRoot%\system32\svchost.exe -k LocalSystemNetworkRestricted" /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v Description /t reg_sz /d "My service based on console application template" /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v ObjectName /t reg_sz /d "LocalSystem" /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v Start /t reg_dword /d 2 /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService" /v Type /t reg_dword /d 0x20 /f
reg add "hklm\system\CurrentControlSet\Services\IotCoreService\Parameters" /v ServiceDll /t REG_EXPAND_SZ /d "%SystemRoot%\System32\IotCoreService.dll" /f

reg add "hklm\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Svchost" /v LocalSystemNetworkRestricted /t REG_MULTI_SZ /d " hidserv\0Netman\0UpdateManagerSvc\0DevQueryBroker\0AudioEndpointBuilder\0NcbService\0WUDFSvc\0wlansvc\0SmsRouter\0NgcSvc\0DsSvc\0DeviceAssociationService\0WwanSvc\0SensorService\0sysmain\0IotCoreService" /f



goto :EOF


:Help

echo.
echo Usage:
echo ConfigureIotCoreService.bat [remove]
echo. 
echo     remove - [opt] if specified, the service configuration will be removed
echo. 
