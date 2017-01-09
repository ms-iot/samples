:: Run the following script from “Deployment and Imaging Tools Environment” as Admin.
SET PATH=%KITSROOT%\tools\bin\i386;%PATH%
SET PATH=%KITSROOT%\bin\i386;%PATH%
SET AKROOT=%KITSROOT%
SET WPDKCONTENTROOT=%KITSROOT%
SET PKG_CONFIG_XML=%KITSROOT%Tools\bin\i386\pkggen.cfg.xml
SET BSP_VERSION=10.0.0.0
SET BSP_ARCH=arm
SET OUTPUT_DIRECTORY=.

:: The following variables ensure the package is appropriately signed
SET SIGN_OEM=1
SET SIGN_WITH_TIMESTAMP=0

echo Creating Driver Package
"%KITSROOT%Tools\bin\i386\pkggen.exe" HidDriver.%BSP_ARCH%.pkg.xml /config:"%PKG_CONFIG_XML%" /output:"%OUTPUT_DIRECTORY%" /version:%BSP_VERSION% /build:fre /cpu:%BSP_ARCH% /nohives /variables:"_RELEASEDIR=%OUTPUT_DIRECTORY\;"

