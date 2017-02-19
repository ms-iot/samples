# Microsoft Developer Studio Project File - Name="bacwi" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=bacwi - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bacwi.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bacwi.mak" CFG="bacwi - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bacwi - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "bacwi - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bacwi - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "..\..\include" /I "..\..\win32" /D "NDEBUG" /D "BACDL_BIP" /D TSM_ENABLED=0 /D BACDL_BIP=1 /D USE_INADDR=1 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D PRINT_ENABLED=1 /D BIG_ENDIAN=0 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "bacwi - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\ports\win32" /I "..\..\include" /I "..\..\win32" /D "_DEBUG" /D TSM_ENABLED=1 /D USE_INADDR=0 /D BACDL_BIP=1 /D USE_INADDR=1 /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D PRINT_ENABLED=1 /D BIG_ENDIAN=0 /FR /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "bacwi - Win32 Release"
# Name "bacwi - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\src\abort.c
# End Source File
# Begin Source File

SOURCE=..\..\src\address.c
# End Source File
# Begin Source File

SOURCE=..\object\ai.c
# End Source File
# Begin Source File

SOURCE=..\object\ao.c
# End Source File
# Begin Source File

SOURCE=..\..\src\apdu.c
# End Source File
# Begin Source File

SOURCE=..\..\src\arf.c
# End Source File
# Begin Source File

SOURCE=..\object\av.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacaddr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacapp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacdcode.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacerror.c
# End Source File
# Begin Source File

SOURCE=..\object\bacfile.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacint.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacreal.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bacstr.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bactext.c
# End Source File
# Begin Source File

SOURCE=..\object\bi.c
# End Source File
# Begin Source File

SOURCE="..\..\ports\win32\bip-init.c"
# End Source File
# Begin Source File

SOURCE=..\..\src\bip.c
# End Source File
# Begin Source File

SOURCE=..\object\bo.c
# End Source File
# Begin Source File

SOURCE=..\object\bv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\bvlc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\cov.c
# End Source File
# Begin Source File

SOURCE=..\..\src\crc.c
# End Source File
# Begin Source File

SOURCE=..\..\src\datetime.c
# End Source File
# Begin Source File

SOURCE=..\..\src\dcc.c
# End Source File
# Begin Source File

SOURCE=..\object\device.c
# End Source File
# Begin Source File

SOURCE=..\..\src\filename.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_arf.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_arf_a.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_cov.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_iam.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_rp.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_rp_a.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_whois.c
# End Source File
# Begin Source File

SOURCE=..\handler\h_wp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\iam.c
# End Source File
# Begin Source File

SOURCE=..\..\src\indtext.c
# End Source File
# Begin Source File

SOURCE=..\object\lc.c
# End Source File
# Begin Source File

SOURCE=..\object\lc.h
# End Source File
# Begin Source File

SOURCE=..\object\lsp.c
# End Source File
# Begin Source File

SOURCE=..\object\lsp.h
# End Source File
# Begin Source File

SOURCE=main.c
# End Source File
# Begin Source File

SOURCE=..\object\mso.c
# End Source File
# Begin Source File

SOURCE=..\handler\noserv.c
# End Source File
# Begin Source File

SOURCE=..\..\src\npdu.c
# End Source File
# Begin Source File

SOURCE=..\..\src\reject.c
# End Source File
# Begin Source File

SOURCE=..\..\src\ringbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\rp.c
# End Source File
# Begin Source File

SOURCE=..\handler\s_rp.c
# End Source File
# Begin Source File

SOURCE=..\handler\s_whois.c
# End Source File
# Begin Source File

SOURCE=..\handler\s_wp.c
# End Source File
# Begin Source File

SOURCE=..\..\src\tsm.c
# End Source File
# Begin Source File

SOURCE=..\handler\txbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\src\version.c
# End Source File
# Begin Source File

SOURCE=..\..\src\whois.c
# End Source File
# Begin Source File

SOURCE=..\..\src\wp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\include\abort.h
# End Source File
# Begin Source File

SOURCE=..\..\include\address.h
# End Source File
# Begin Source File

SOURCE=..\object\ai.h
# End Source File
# Begin Source File

SOURCE=..\object\ao.h
# End Source File
# Begin Source File

SOURCE=..\..\include\apdu.h
# End Source File
# Begin Source File

SOURCE=..\..\include\arcnet.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bacapp.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bacdcode.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bacdef.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bacenum.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bacerror.h
# End Source File
# Begin Source File

SOURCE=..\object\bacfile.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bacstr.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bactext.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bigend.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bip.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bits.h
# End Source File
# Begin Source File

SOURCE=..\object\bo.h
# End Source File
# Begin Source File

SOURCE=..\object\bv.h
# End Source File
# Begin Source File

SOURCE=..\..\include\bytes.h
# End Source File
# Begin Source File

SOURCE=..\handler\client.h
# End Source File
# Begin Source File

SOURCE=..\..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\include\crc.h
# End Source File
# Begin Source File

SOURCE=..\..\include\datalink.h
# End Source File
# Begin Source File

SOURCE=..\..\include\datetime.h
# End Source File
# Begin Source File

SOURCE=..\object\device.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ethernet.h
# End Source File
# Begin Source File

SOURCE=..\handler\handlers.h
# End Source File
# Begin Source File

SOURCE=..\..\include\iam.h
# End Source File
# Begin Source File

SOURCE=..\object\mso.h
# End Source File
# Begin Source File

SOURCE=..\..\include\mstp.h
# End Source File
# Begin Source File

SOURCE=..\..\include\npdu.h
# End Source File
# Begin Source File

SOURCE=..\..\include\reject.h
# End Source File
# Begin Source File

SOURCE=..\..\include\ringbuf.h
# End Source File
# Begin Source File

SOURCE=..\..\include\rp.h
# End Source File
# Begin Source File

SOURCE=..\..\include\rs485.h
# End Source File
# Begin Source File

SOURCE=..\stdbool.h
# End Source File
# Begin Source File

SOURCE=..\stdint.h
# End Source File
# Begin Source File

SOURCE=..\..\include\tsm.h
# End Source File
# Begin Source File

SOURCE=..\..\include\whois.h
# End Source File
# Begin Source File

SOURCE=..\..\include\wp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
