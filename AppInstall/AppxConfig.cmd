:: ---------------------------------------------------------------------
:: Variable setup
::
:: Example:
:: set AppxName=defappxname (Name only. No need for .appx extension)
:: set certslist=cert1name cert2name (Name only. No need for .cer extension. You can delimit mutliple certificates with a space.)
:: set defaultappxid=defaultappxid  (You can find your app's id by checking your app's Package.appxmanifest)
:: set dependencylist=depappx1name depappx2name (Name only. No need for .appx extension. You can delimit mutliple dependency appxs with a space.)
:: ---------------------------------------------------------------------
set AppxName=MainAppx_1.0.0.0_arm
set certslist=MainAppx_1.0.0.0_arm
set dependencylist=Microsoft.VCLibs.ARM.14.00 Microsoft.NET.Native.Runtime.1.1 Microsoft.NET.Native.Framework.1.2
set forceinstall=0
set launchapp=1

