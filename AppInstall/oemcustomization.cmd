REM Script to install the appx on boot.
if exist C:\AppInstall\AppInstall.cmd (
    call C:\AppInstall\AppInstall.cmd > %temp%\AppInstallLog.txt
    if %errorlevel%== 0 (
        cd \
        rmdir /S /Q C:\AppInstall
    )
)

