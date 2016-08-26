@echo off

::
:: Ensure we execute batch script from the folder that contains the batch script
::
pushd %~dp0
SETLOCAL

if exist %systemdrive%\windows\system32\mindeployappx.exe (
	echo Mindeployappx.exe found. Using older install script
	if exist AppInstall_TH.cmd (call AppInstall_TH.cmd )
	exit /b %errorlevel%
)

REM New Install Mechanism
if not exist %systemdrive%\windows\system32\deployappx.exe ( 
	echo Error: deployappx.exe not found. exiting. 
	exit /b 1
)

call AppxConfig.cmd

echo Appx Name :%AppxName%

if not defined forceinstall ( set forceinstall=0 )
if not defined launchapp ( set launchapp=1 )
if not exist .\logs ( mkdir logs ) else ( del /Q .\logs\*.* )

REM
REM Add AllowAllTrustedApps Reg Key
REM
echo Adding AllowAllTrustedApps Reg Key.
reg add "HKLM\Software\Policies\Microsoft\Windows\Appx" /v AllowAllTrustedApps /t REG_DWORD /d 1 /f >nul
call :SUB_CHECKERROR "Failed to add AllowAllTrustedApps"

REM
REM Add all dependency appx
REM
echo Installing dependency appx packages
for %%d in (%dependencylist%) do (
	echo Installing %%d.appx
    deployappx.exe install .\%%d.appx >> .\logs\dependency_result.txt
)

REM
REM Install the Main Appx
REM
if %forceinstall% == 1 (
    set INSTALL_PARAMS=install force %AppxName%.appx
) else (
    set INSTALL_PARAMS=install %AppxName%.appx
)
echo Installing %AppxName%.appx with %INSTALL_PARAMS%
deployappx.exe %INSTALL_PARAMS% > %temp%\%AppxName%_result.txt
if "%ERRORLEVEL%"=="0" (
    call :LAUNCH_APP
) else (
    echo. Error in installing %AppxName%.appx. 
    echo. Result:%ERRORLEVEL%
)

goto :CLEANUP

:LAUNCH_APP
deployappx.exe getpackageid %AppxName%.appx > .\logs\packageid.txt
for /f "tokens=2,5 delims=:_" %%A in (.\logs\packageid.txt) do (
    set AppxID=%%A_%%B
)
set AppxID=%AppxID: =%
echo Launching %AppxID% 
REM Trigger IoTStartup
iotstartup.exe add headed %AppxID%
exit /b

:SUB_CHECKERROR
if "%ERRORLEVEL%"=="0" exit/b
echo.
echo Error %1
echo Result=%ERRORLEVEL%
echo.
exit /b %ERRORLEVEL%

:CLEANUP
popd
ENDLOCAL
exit /b

