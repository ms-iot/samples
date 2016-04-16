## Building the solution


### The Models Solution Folder

The Models solution folder must be built before the MyLivingRoom project will
build. This is because the project is setup to reference the Models as WINMD
files, instead of using project references. This allows you to Clean/Rebuild
the MyLivingRoom project, without it first cleaning/rebuilding all of the
dependent C++/CX projects.


## Generated Code


### The Models projects

The C++/CX projects in the Models solution folder were generated from the AllJoyn® Introspect XML
files under the Models/AllJoynIntrospect solution folder. Note, however, that some of the generated
code was "tweaked" to handle certain device identification scenarios. The tools used to generate
the projects were AllJoynCodeGenerator.exe (a command line utiliity installed with the UWP SDK) and
the AllJoyn® Studio Visual Studio extension, available in the Visual Studio Gallery at:
https://visualstudiogallery.msdn.microsoft.com/064e58a7-fb56-464b-bed5-f85914c89286?SRC=VSIDE


### Resource (RESW) file in the MyLivingRoom project

The Resources/en-US/String.resw file has its Custom Tool property set to InternalReswFileCodeGenerator.
This custom tool can be installed from ResW File Code Generator (http://reswcodegen.codeplex.com).
It can also be found on the Visual Studio Gallery
(https://visualstudiogallery.msdn.microsoft.com/3ab88efd-1afb-4ff5-9faf-8825a282596a).
If you add or remove strings to the RESW file (or add new languages), you can use this tool to
autmatically generate a C# class that makes it easier to load the strings in your code. Because
it generates a class with the string loading code, you don't have to refer to the string resources
by string key. This makes the code less error prone, and reveals any string keyh change errors at
compile time vs. at run time.


