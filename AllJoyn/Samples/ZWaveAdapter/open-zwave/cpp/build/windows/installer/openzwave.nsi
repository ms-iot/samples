installDir "$PROGRAMFILES\OpenZWave"
Name "OpenZWave"
outFile "setup.exe"

Page license
Page components
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles


LicenseData ..\..\..\..\license\lgpl.txt


Section "Core Files"
SectionIn RO
setOutPath "$INSTDIR"
File ..\..\..\..\license\lgpl.txt
File ..\..\..\examples\windows\MinOZW\vs2010\ReleaseDLL\OpenZWave.dll
setOutPath "$INSTDIR\docs\"
File ..\..\..\..\docs\default.htm
setOutPath "$INSTDIR\docs\images+css\"
File ..\..\..\..\docs\images+css\*
setOutPath "$INSTDIR\docs\general\"
File ..\..\..\..\docs\general\*
createShortCut "$SMPROGRAMS\OpenZWave\Getting Started.lnk" "$INSTDIR\docs\default.htm" 
SectionEnd

Section /o "Development Files"
setOutPath "$INSTDIR\development\DebugDLL"
File ..\..\..\examples\windows\MinOZW\vs2010\DebugDLL\OpenZWaved.dll
File ..\..\..\examples\windows\MinOZW\vs2010\DebugDLL\OpenZWaved.lib
setOutPath "$INSTDIR\development\Debug"
File ..\..\..\..\dotnet\examples\OZWForm\src\Debug\OpenZWave.lib
setOutPath "$INSTDIR\development\ReleaseDLL"
File ..\..\..\examples\windows\MinOZW\vs2010\ReleaseDLL\OpenZWave.dll
File ..\..\..\examples\windows\MinOZW\vs2010\ReleaseDLL\OpenZWave.lib
setOutPath "$INSTDIR\development\Release"
File ..\..\..\..\dotnet\examples\OZWForm\src\Release\OpenZWave.lib
setOutPath "$INSTDIR\development\include\openzwave\"
File ..\..\..\src\*.h
setOutPath "$INSTDIR\development\include\openzwave\command_classes\"
file ..\..\..\src\command_classes\*.h
setOutPath "$INSTDIR\development\include\openzwave\value_classes\"
file ..\..\..\src\value_classes\*.h
setOutPath "$INSTDIR\development\include\openzwave\platform\"
file ..\..\..\src\platform\*.h
setOutPath "$INSTDIR\development\include\openzwave\platform\windows\"
file ..\..\..\src\platform\windows\*.h
SectionEnd

Section ".Net Component"
SectionIn RO
setOutPath "$INSTDIR\dotnet\"
File ..\..\..\..\dotnet\examples\OZWForm\src\Debug\OpenZWaveDotNetd.dll
File ..\..\..\..\dotnet\examples\OZWForm\src\Debug\OpenZWaveDotNetd.lib
File ..\..\..\..\dotnet\examples\OZWForm\src\Release\OpenZWaveDotNet.dll
File ..\..\..\..\dotnet\examples\OZWForm\src\Release\OpenZWaveDotNet.lib
SectionEnd

Section "Examples"
setOutPath "$INSTDIR\dotnet\"
File ..\..\..\examples\windows\MinOZW\vs2010\ReleaseDLL\MinOZW.exe
File ..\..\..\..\dotnet\examples\OZWForm\src\bin\x86\Release\OZWForm.exe
createShortCut "$SMPROGRAMS\OpenZWave\OpenZWave Form.lnk" "$INSTDIR\OZWForm.exe"
SectionEnd

