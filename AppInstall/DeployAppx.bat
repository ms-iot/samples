@echo off

::
:: Ensure we execute batch script from the folder that contains the batch script
::
pushd %~dp0
SETLOCAL

:: ---------------------------------------------------------------------
:: Variable setup
::
:: Example:
:: set defaultappx=defappxname (name only. no need for .appx extension)
:: set defaultappxid=defaultappxid
:: set dependencylist=depappx1name depappx2name (Name only. No need for .appx extension. You can delimit mutliple dependency appxs with a space.)
:: ---------------------------------------------------------------------
set defaultappx=MainAppx_1.0.0.0_Win32_Debug
set defaultappxid=bfa3eb48-79d5-4245-9bfe-6d2ffeef846d_q8jky9dv1tcdg!App
set dependencylist=Microsoft.VCLibs.x86.Debug.14.00
::set defaultappx=MainAppx_1.0.2.0_Win32_Debug

::
:: Add all dependency appx
::
for %%d in (%dependencylist%) do (
    mindeployappx /add /PackagePath:%~dp0%%d.appx > %temp%\%%d_result.txt
)

::
:: Only switch to TempAppx is Appx is not currently in foreground.
::
for /F "skip=2 tokens=3" %%r in ('reg query "HKLM\Software\Microsoft\Windows NT\currentversion\winlogon\iotshellextension" /v appid') do (
   if /i %%r==%defaultappxid% (
    goto :SWITCH_TO_TEMP
   )
)

goto :INSTALLAPPX

:SWITCH_TO_TEMP

::
:: Switch to IOTUAPOOBE or DefaultApp
::
iotstartup.exe list headed > %temp%\allinstalledappx.txt
findstr /m "IoTUAPOOBE" %temp%\allinstalledappx.txt
if %errorlevel%==0 (
    iotstartup.exe add headed IoTUAPOOBE_cw5n1h2txyewy!App
)
findstr /m "DefaultApp" %temp%\allinstalledappx.txt
if %errorlevel%==0 (
    iotstartup.exe add headed DefaultApp_cw5n1h2txyewy!App
)

goto :INSTALLAPPX

::
:: Install defaultappx
::
:INSTALLAPPX
mindeployappx /%~n2 /PackagePath:%~dp0%defaultappx%.appx > %temp%\%defaultappx%_result.txt

echo READY > %temp%\%~n1_deploy_done.txt
REM Trigger IoTStartup
iotstartup.exe add headed %defaultappxid%
goto :CLEANUP

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
