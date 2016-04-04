

	Building OZWForm under VS2008 and VS2010

Unlike the base OpenZWave C++ project, Visual Studio appears to be much
more "particular" about where the project files are stored.  Originally,
we had two copies of the OZWForm source files...one for VS2008 and a
second for VS2010.  But they've been difficult to keep fully in sync as
changes are made.

So the files in this directory provide a rough (but hopefully effective)
solution:  
1)	a single source code directory with all of the common files,
2)	two separate "build" directories with .sln and .csproj files (one 
	for VS2010 and one for VS2008), and
3)	some batch files for copying the appropriate .sln and .csproj
	files into the source code directory.

Hopefully the batch files will work; if they don't you should be able to 
simply copy the .sln and .csproj files from the build subdirectory that
matches your Visual Studio version into the .\src directory.

If too many people have difficulty with this (or if there are other issues
with VS2010 that disallow this approach), we may have to go back to two
separate (but essentially identical) source code directories.

Let's hope not!