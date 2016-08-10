@echo off

::
:: Ensure we execute batch script from the folder that contains the batch script
::
pushd %~dp0
SETLOCAL

if not exist %systemdrive%\windows\system32\mindeployappx.exe ( 
	echo mindeployappx.exe not found. exiting. 
	exit /b 1
)

call AppxConfig.cmd

echo Appx Name :%AppxName%

if not defined forceinstall (set forceinstall=0)
if not exist .\logs ( mkdir logs )

::
:: Get Appx Family Name
::
mindeployappx /FetchPackageFamilyName /packagepath:"%~dp0%AppxName%.appx" > .\logs\appxfamname.txt
set /p fullappxfamilyname=<.\logs\appxfamname.txt
set AppxID=%fullappxfamilyname:~19,-1%
echo Appx Family Name: %AppxID%

REM Get AppxVer
for /f "tokens=2 delims=_" %%A in ("%AppxName%") do ( set "AppxVer=%%A" )
echo Appx Ver : %AppxVer%
REM Get AppxGuid
for /f "tokens=1 delims=_" %%A in ("%AppxID%") do ( set "AppxGuid=%%A" )
echo Appx Guid : %AppxGuid%

REM Get the list of installed packages
mindeployappx /GetPackages > .\logs\installed_packages.txt
REM Check if the AppxGuid is already installed, if so get full package name

findstr /l "%AppxGuid%" .\logs\installed_packages.txt > .\logs\fullpackagename.txt
if %errorlevel%==0 (
	REM Appx is installed.
	set /P CurrentAppxID=<.\logs\fullpackagename.txt
)
if defined CurrentAppxID (
	REM Get CurrentAppxVer
	for /f "tokens=2 delims=_" %%A in ("%CurrentAppxID%") do ( set "CurrentAppxVer=%%A" )
)
if defined CurrentAppxVer (
	echo Installed Appx Ver : %CurrentAppxVer%
	REM If same version, then do nothing. 

	if /i "%AppxVer%" EQU "%CurrentAppxVer%" ( 
		echo Same version already installed
		goto :CLEANUP
	)
	REM If higher version already installed,  	
	if /i "%AppxVer%" LSS "%CurrentAppxVer%" (
		if %forceinstall%==1 (
			REM downgrade requested, proceed with uninstall and then continue with install
			echo Performing force install
			set installtype=forceinstall
			goto :SUB_DEPLOYAPPX
		) else (
			echo Higher version already installed
			goto :CLEANUP
		)
	)
	if /i "%AppxVer%" GTR "%CurrentAppxVer%" (
		set installtype=Update
	)
) else (
	:: No current version so install
	set installtype=Add
	echo No version found. Installing %AppxName%.
)

:: -------------------------------------------------------------------------------
::
:: SUB_DEPLOYAPPX 
::
:: Installs certificate and deploys specified appx package.  Exits on failure 
::
:: ------------------------------------------------------------------------------- 
:SUB_DEPLOYAPPX

REM Add AllowAllTrustedApps Reg Key
REM
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
:: Install Certificates
::
for %%i in (%certslist%) do (
    echo Installing %%i Certificate
    certmgr.exe -add .\%%i.cer -r localMachine -s root > .logs\%%i_cer_result.txt
    if %errorlevel% == 0 (
        echo Successfuly installed %%i Certificate.
    ) else ( 
        echo Failed to install %%i Certificate.
        echo ErrorCode: %errorlevel%
        goto CLEANUP
    )
)

::
:: Create Scheduled Task to Deploy Appx
::
set taskname=DeployAppxTask
set defaultaccTemp=%systemdrive%\data\users\defaultaccount\appdata\local\temp
echo Creating Scheduled Task "%taskname%" for Appx Installation.
del /Q %defaultaccTemp%\%taskname%_deploy_done.txt 2> nul:
del /Q %defaultaccTemp%\%AppxName%_result.txt 2> nul:
schtasks /create /f /tn "%taskname%" /ru DefaultAccount /sc ONSTART /tr "%~dp0deploytask.cmd %taskname% %installtype% > %~dp0logs\deployappxlog.txt"
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
if NOT EXIST %defaultaccTemp%\%taskname%_deploy_done.txt (
    set /A ITER=ITER+1
    if "%ITER%" == "50" (
        echo Deployment of %taskname% Task Timedout
        goto CLEANUP
    )
    ping -n 4 localhost > nul:
    goto CHECK_FOR_FILE
)
endlocal
findstr /B /L "ReturnCode:[0x0]" %defaultaccTemp%\%AppxName%_result.txt
if %errorlevel% == 0 (
    echo Successfuly Deployed %AppxName%.appx
) else (
    echo Failed to Deploy %AppxName%.appx
    echo ErrorCode: %errorlevel%
    goto CLEANUP
)

echo.

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
