# Microsoft Developer Studio Project File - Name="bacnet" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=bacnet - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "bacnet.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "bacnet.mak" CFG="bacnet - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "bacnet - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "bacnet - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "bacnet - Win32 Release"

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
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\..\.." /I ".." /I "..\..\..\demo\object\\" /I "..\..\..\demo\handler\\" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "BACAPP_ALL" /D PRINT_ENABLED=1 /D BIG_ENDIAN=0 /D "BACDL_BIP" /D USE_INADDR=1 /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "bacnet - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\..\..\include\\" /I ".." /I "..\..\..\demo\object\\" /I "..\..\..\demo\handler\\" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "BACAPP_ALL" /D PRINT_ENABLED=1 /D BIG_ENDIAN=0 /D "BACDL_BIP" /D USE_INADDR=1 /FR /FD /GZ /c
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

# Name "bacnet - Win32 Release"
# Name "bacnet - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\src\abort.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\address.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\ai.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\ao.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\apdu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\arf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\av.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacaddr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacapp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacdcode.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacdevobjpropref.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacerror.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bacfile.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacint.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacreal.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bacstr.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bactext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bi.c
# End Source File
# Begin Source File

SOURCE="..\bip-init.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bip.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bo.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bv.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\bvlc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\cov.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\crc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\datetime.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\dcc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\debug.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\device.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\dlenv.c
# End Source File
# Begin Source File

SOURCE=..\dlmstp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_arf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_arf_a.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_cov.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_iam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_npdu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_rp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_rp_a.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_rpm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_whois.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\h_wp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\iam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\indtext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\lc.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\lc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\lsp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\lsp.h
# End Source File
# Begin Source File

SOURCE=..\main.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\memcopy.c
# End Source File
# Begin Source File

SOURCE="..\..\..\demo\object\ms-input.c"
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\mso.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\mstp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\mstptext.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\noserv.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\npdu.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\reject.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\ringbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\rp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\rpm.c
# End Source File
# Begin Source File

SOURCE=..\rs485.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\s_iam.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\s_rp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\s_whois.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\s_wp.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\trendlog.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\tsm.c
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\handler\txbuf.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\version.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\whois.c
# End Source File
# Begin Source File

SOURCE=..\..\..\src\wp.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\abort.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\address.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\ai.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\ao.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\apdu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\arcnet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bacapp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bacdcode.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bacdef.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bacenum.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bacerror.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bacfile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bacstr.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bactext.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bigend.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bip.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bits.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bo.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\bv.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bvlc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\bytes.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\config.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\crc.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\datalink.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\datetime.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\debug.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\device.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ethernet.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\handlers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\iam.h
# End Source File
# Begin Source File

SOURCE=..\..\..\demo\object\mso.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\mstp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\mstptext.h
# End Source File
# Begin Source File

SOURCE=..\net.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\npdu.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\reject.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\ringbuf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\rp.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\rs485.h
# End Source File
# Begin Source File

SOURCE=..\stdbool.h
# End Source File
# Begin Source File

SOURCE=..\stdint.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\tsm.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\version.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\whois.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\wp.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
