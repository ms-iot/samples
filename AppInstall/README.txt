--------------------------
 APPX INSTALLATION ON IOT
--------------------------

To install your appx on an IoT device please do the following:

1. Edit variable setup for AppInstall.bat.
	- Set defaultappx = your appx's file name (Name only. No need for .appx extension)
	- Set certslist = your appx's certificate's name. (Name only. No need for .cer extension. You can delimit mutliple certificates with a space.)

2. Edit variable setup for DeployApp.bat
	- Set defaultappx = your appx's file name (Name only. No need for .appx extension)
	- Set defaultappxid = your appx's Id (You can find your app's id by checking your app's Package.appxmanifest)
	- Set dependencylist = your appx's dependency names. (Name only. No need for .cer extension. You can delimit mutliple certificates with a space.)

3. Place your files in the followind directories:
	- c:\windows\appinstall: Your Appx, Dependency Appx(s), Temp appx (optional), Certificate(s), AppInstall.bat , DeployApp.bat 
	- c:\windows\system32: OemCustomization.cmd
		
    You can do this by either:    
    - Wrapping the binaries in an OEM Package and include it when you create the image with ICD/Imggen.
    - Manually copy the files over to disk.
    
4. Restart the device and your appx will be automatically installed on boot.