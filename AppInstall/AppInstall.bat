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
:: set defaultappx=defappxname (Name only. No need for .appx extension)
:: set certslist=cert1name cert2name (Name only. No need for .cer extension. You can delimit mutliple certificates with a space.)
:: ---------------------------------------------------------------------
set defaultappx=MainAppx_1.0.0.0_x86
set certslist=MainAppx_1.0.0.0_x86
::set defaultappx=MainAppx_1.0.2.0_x86
::set certslist=MainAppx_1.0.2.0_x86

::
:: Get Appx Family Name
::
mindeployappx /FetchPackageFamilyName /packagepath:%~dp0%defaultappx%.appx > %temp%\appxfamname.txt
set /p fullappxfamilyname=<%temp%\appxfamname.txt
set appxfamilyname=%fullappxfamilyname:~19,-1%
echo Appx Family Name: %appxfamilyname%

::
:: Get Appx Version
::
mindeployappx /FetchPackageVersion /packagepath:%~dp0%defaultappx%.appx > %temp%\appxversion.txt
set /p appxversion=<%temp%\appxversion.txt
echo Appx Version: %appxversion%

::
:: Get Installed Appx Version
::
mindeployappx /FetchPackageVersion /packagefamilyname:%appxfamilyname% > %temp%\installedversion.txt
set /p installedversion=<%temp%\installedversion.txt
echo Installed Appx Version: %installedversion%


::
:: If Appx with same Family Name isn't already installed, install it
::
set installtype=Add
If NOT "%installedversion%"=="%installedversion:ReturnCode=%" (
    echo No appx with Family Name %appxfamilyname% is currently installed. Appx can be installed.
    goto SUB_DEPLOYAPPX
) 

::
:: If Appx with same Family Name is already installed, compare Appx version and Installed Appx version
:: Install if Appx version > Installed Appx version
::
echo Comparing Appx Version and Installed Appx Version.
set installtype=Update
call :COMPARE_VERSIONS %appxversion% %installedversion%
if %errorlevel% == 1 (
    echo Appx version greater than Installed Appx Version. Appx can be installed.
    goto SUB_DEPLOYAPPX
)

::
:: If Appx version =< Installed Appx version, exit
echo Appx version is less than or equal to the Installed Appx Version. Appx cannot be installed.
echo Exiting.
goto CLEANUP


:: -------------------------------------------------------------------------------
::
:: SUB_DEPLOYAPPX 
::
:: Installs certificate and deploys specified appx package.  Exits on failure 
::
:: ------------------------------------------------------------------------------- 
:SUB_DEPLOYAPPX

::
:: Add AllowAllTrustedApps Reg Key
::
echo Adding AllowAllTrustedApps Reg Key.
reg add "HKLM\Software\Policies\Microsoft\Windows\Appx" /v AllowAllTrustedApps /t REG_DWORD /d 1 /f
if %errorlevel% == 0 (
    echo Successfuly added AllowAllTrustedApps to registry.
) else (
    echo Failed to add AllowAllTrustedApps to registry: %errorlevel%
    echo ErrorCode: %errorlevel%
    goto CLEANUP
)

::
:: Instal Certificates
::
(for %%i in (%certslist%) do (
    echo Installing %%i Certificate
    certmgr.exe -add .\%%i.cer -r localMachine -s root > %temp%\%%i_cer_result.txt
    if %errorlevel% == 0 (
        echo Successfuly installed %%i Certificate.
    ) else ( 
        echo Failed to install %%i Certificate.
        echo ErrorCode: %errorlevel%
        goto CLEANUP
    )))

::
:: Create Scheduled Task to Deploy Appx
::
set taskname=DeployAppxTask
echo Creating Scheduled Task "%taskname%" for Appx Installation.
del /Q %systemdrive%\data\users\defaultaccount\appdata\local\temp\%taskname%_deploy_done.txt 2> nul:
del /Q %systemdrive%\data\users\defaultaccount\appdata\local\temp\%defaultappx%_result.txt 2> nul:
schtasks /create /f /tn "%taskname%" /ru DefaultAccount /sc ONSTART /tr "%~dp0deployappx.bat %taskname% %installtype%"
if %errorlevel% == 0 (
    echo Successfuly Created Scheduled Task "%taskname%".
) else (
    echo Failed to Create Scheduled Task "%taskname%"
    echo ErrorCode: %errorlevel%
    goto CLEANUP
)

::
:: Run Schedued Task
::
echo Running Scheduled Task "%taskname%"
schtasks /run /tn %taskname%
if %errorlevel% == 0 (
    echo Successfuly Started Scheduled Task "%taskname%".
) else (
    echo Failed to Start Scheduled Task "%taskname%"
    echo ErrorCode: %errorlevel%
    goto CLEANUP
)

::
:: Check if Sched Task Ran
::
setlocal
set ITER=0
:CHECK_FOR_FILE
if NOT EXIST %systemdrive%\data\users\defaultaccount\appdata\local\temp\%taskname%_deploy_done.txt (
    set /A ITER=ITER+1
    if "%ITER%" == "50" (
        echo Deployment of %taskname% Task Timedout
        goto CLEANUP
    )
    ping -n 4 localhost > nul:
    goto CHECK_FOR_FILE
)
endlocal
findstr /B /L "ReturnCode:[0x0]" %systemdrive%\data\users\defaultaccount\appdata\local\temp\%defaultappx%_result.txt
if %errorlevel% == 0 (
    echo Successfuly Deployed %defaultappx%.
) else (
    echo Failed to Deploy %defaultappx%
    echo ErrorCode: %errorlevel%
    goto CLEANUP
)

echo.
goto CLEANUP

setlocal
:: -------------------------------------------------------------------------------
::
:: COMPARE_VERSIONS 
::
:: Compares two version numbers and returns the result in the ERRORLEVEL
::
:: Returns 1 if version1 > version2
::         0 if version1 = version2
::        -1 if version1 < version2
::
:: The nodes must be delimited by . or , or -
::
:: Nodes are normally strictly numeric, without a 0 prefix. A letter suffix
:: is treated as a separate node
::
:: ------------------------------------------------------------------------------- 
:COMPARE_VERSIONS  version1  version2
setlocal enableDelayedExpansion
set "v1=%~1"
set "v2=%~2"
call :DIVIDE_LETTERS v1
call :DIVIDE_LETTERS v2
:loop
call :PARSE_NODE "%v1%" n1 v1
call :PARSE_NODE "%v2%" n2 v2
if %n1% gtr %n2% exit /b 1
if %n1% lss %n2% exit /b -1
if not defined v1 if not defined v2 exit /b 0
if not defined v1 exit /b -1
if not defined v2 exit /b 1
goto :loop


:PARSE_NODE  version  nodeVar  remainderVar
for /f "tokens=1* delims=.,-" %%A in ("%~1") do (
  set "%~2=%%A"
  set "%~3=%%B"
)
exit /b


:DIVIDE_LETTERS  versionVar
for %%C in (a b c d e f g h i j k l m n o p q r s t u v w x y z) do set "%~1=!%~1:%%C=.%%C!"
exit /b


:: -------------------------------------------------------------------------------
::
:: CLEANUP
::
:: ------------------------------------------------------------------------------- 
:CLEANUP
echo Cleaning Up.
schtasks /delete /f /tn %taskname% 2> nul:
echo Exiting.
popd
ENDLOCAL
exit /b
