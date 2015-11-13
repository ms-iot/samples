--------------------------
 APPX INSTALLATION ON IOT
--------------------------

1. Edit variable setup for AppInstall.bat.
	- Set defaultappx = your appx's file name
	- Set certslist = your appx's certificate's name. You can add multiple certificates, separate by a space.

2. Edit variable setup for DeployApp.bat
	- Set defaultappx = your appx's file name
	- Set defaultappxid = your appx's Id
	- Set dependencylist = your appx's dependency names. You can add multiple dependency names, separate by a space.
	- Set tempappx = your temp appx's file name. This is optional. If you would like to provide your own temp appx that runs in the foreground during appx installation/update you can provide it here.
	- Set tempappxid = your temp appx's file name. If you would like to provide your own temp appx that runs in the foreground during appx installation/update you can provide it here.

3. Bin Place
	- Your Appx, Dependency Appx, Temp appx and Certs to c:\windows\appinstall
	- AppInstall.bat and DeployApp.bat to c:\windows\appinstall
	- OemCustomization.cmd to c:\windows\system32

4. Restart device and appx will be automatically installed on boot.