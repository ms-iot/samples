@echo off 

::
:: Ensure we execute batch script from the folder that contains the batch script
::
pushd %~dp0
SETLOCAL

call AppxConfig.cmd
if not defined launchapp ( set launchapp=1 )
set installtype=%2

set /p fullappxfamilyname=<.\logs\appxfamname.txt
set AppxID=%fullappxfamilyname:~19,-1%
echo Appx Family Name: %AppxID%

set /P CurrentAppxID=<.\logs\fullpackagename.txt

if not %installtype% EQU "Add" ( 
	::
	:: Only switch to TempAppx is Appx is not currently in foreground.
	::
	for /F "skip=2 tokens=3" %%r in ('reg query "HKLM\Software\Microsoft\Windows NT\currentversion\winlogon\iotshellextension" /v appid') do (
	   if /i "%%r" EQU "%AppxID%!App" (
		call :SWITCH_TO_TEMP
		ping -n 4 localhost > nul:
	   )
	)

	if "%installtype%" EQU "forceinstall" (
		set installtype=Add
		mindeployappx /remove /PackageFullName:"%CurrentAppxID%" > .\logs\%AppxName%_removeresult.txt
		ping -n 4 localhost > nul:
	)
)
::
:: Add all dependency appx
::
for %%d in (%dependencylist%) do (
    mindeployappx /add /PackagePath:%~dp0%%d.appx > .\logs\%%d_result.txt
)


::
:: Install Appx
::

mindeployappx /%installtype% /PackagePath:%~dp0%AppxName%.appx > %temp%\%AppxName%_result.txt

echo READY > %temp%\%~n1_deploy_done.txt
REM Trigger IoTStartup
if %launchapp% == 1 (
	iotstartup.exe add headed %AppxID%
)
goto :CLEANUP

:SWITCH_TO_TEMP
REM Switch to IOTUAPOOBE 
iotstartup.exe list headed > .\logs\allinstalledappx.txt
findstr /l "IoTUAPOOBE" .\logs\allinstalledappx.txt
if %errorlevel%==0 (
    iotstartup.exe add headed IoTUAPOOBE_cw5n1h2txyewy!App
) else (
	echo error : cannot find IOTUAPOOBE app 
)
exit /b
:: -------------------------------------------------------------------------------
::
:: CLEANUP
::
:: ------------------------------------------------------------------------------- 
:CLEANUP
echo Cleaning Up
echo Exiting.
popd
ENDLOCAL
exit /b
