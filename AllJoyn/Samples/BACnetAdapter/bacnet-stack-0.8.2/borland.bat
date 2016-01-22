@echo off
echo Build for Borland 5.5 tools
set BORLAND_DIR=c:\borland\bcc55
%BORLAND_DIR%\bin\make -f makefile.b32 clean
%BORLAND_DIR%\bin\make -f makefile.b32 all


