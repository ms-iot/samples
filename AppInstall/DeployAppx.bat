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
:: set defaultappx=defappxname (Name only. no need for .appx extension)
:: set defaultappxid=defaultappxid  (You can find your app's id by checking your app's Package.appxmanifest)
:: set dependencylist=depappx1name depappx2name (Name only. No need for .appx extension. You can delimit mutliple dependency appxs with a space.)
:: ---------------------------------------------------------------------
set defaultappx=MainAppx_1.0.0.0_x86
set defaultappxid=7a11c6a0-d9a8-40cd-80cf-91d6a409edae_pft3qchy8afw0
set dependencylist=Microsoft.VCLibs.x86.14.00 Microsoft.NET.Native.Runtime.1.1 Microsoft.NET.Native.Framework.1.2
::set defaultappx=MainAppx_1.0.2.0_x86

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

ping -n 4 localhost > nul:

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
