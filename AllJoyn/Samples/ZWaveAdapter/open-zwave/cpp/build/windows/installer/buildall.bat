msbuild ..\..\..\examples\windows\MinOZW\vs2010\MinOZW.sln /t:Rebuild /p:Configuration="Debug"
msbuild ..\..\..\examples\windows\MinOZW\vs2010\MinOZW.sln /t:Rebuild /p:Configuration="DebugDLL"
msbuild ..\..\..\examples\windows\MinOZW\vs2010\MinOZW.sln /t:Rebuild /p:Configuration="Release"
msbuild ..\..\..\examples\windows\MinOZW\vs2010\MinOZW.sln /t:Rebuild /p:Configuration="ReleaseDLL"
copy ..\..\..\..\dotnet\examples/OZWForm\build\vs2010\*.sln ..\..\..\..\dotnet\examples\OZWForm\src\
copy ..\..\..\..\dotnet\examples/OZWForm\build\vs2010\*.csproj ..\..\..\..\dotnet\examples\OZWForm\src\
msbuild ..\..\..\..\dotnet\examples\OZWForm\src\OZWForm.sln /t:Rebuild /p:Configuration="Debug"
msbuild ..\..\..\..\dotnet\examples\OZWForm\src\OZWForm.sln /t:Rebuild /p:Configuration="Release"
