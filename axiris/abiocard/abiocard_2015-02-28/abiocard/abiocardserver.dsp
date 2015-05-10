# Microsoft Developer Studio Project File - Name="abiocardserver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=abiocardserver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "abiocardserver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "abiocardserver.mak" CFG="abiocardserver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "abiocardserver - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "abiocardserver - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "abiocardserver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "./../build/msvc/release/abiocardserver"
# PROP Intermediate_Dir "./../build/msvc/release/abiocardserver"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /Zi /O1 /I "./../common" /I "./../common/win32" /I "./../axicat" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib Ws2_32.lib /nologo /subsystem:windows /debug /machine:I386 /out:"abiocardserver.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "abiocardserver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "./../build/msvc/debug/abiocardserver"
# PROP Intermediate_Dir "./../build/msvc/debug/abiocardserver"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Zi /Od /I "./../common" /I "./../common/win32" /I "./../axicat" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib Ws2_32.lib /nologo /subsystem:windows /incremental:no /debug /machine:I386 /out:"abiocardserver_debug.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "abiocardserver - Win32 Release"
# Name "abiocardserver - Win32 Debug"
# Begin Source File

SOURCE=.\abiocard.c
# End Source File
# Begin Source File

SOURCE=.\abiocard.h
# End Source File
# Begin Source File

SOURCE=.\abiocard_data.h
# End Source File
# Begin Source File

SOURCE=.\abiocardserver.c
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat.h
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al.c
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al.h
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al_conf.h
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al_gpio.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al_spi.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al_twi.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=..\axicat\axicat_al_uart.c
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\axicat_i2cbus.c
# End Source File
# Begin Source File

SOURCE=.\axicat_i2cbus.h
# End Source File
# Begin Source File

SOURCE=..\common\win32\console.c
# End Source File
# Begin Source File

SOURCE=..\common\win32\console.h
# End Source File
# Begin Source File

SOURCE=..\common\win32\ft245.c
# End Source File
# Begin Source File

SOURCE=..\common\win32\ft245.h
# End Source File
# Begin Source File

SOURCE=..\common\ft245_common.h
# End Source File
# Begin Source File

SOURCE=..\common\ldllist.c
# End Source File
# Begin Source File

SOURCE=..\common\ldllist.h
# End Source File
# Begin Source File

SOURCE=..\common\osrtl.h
# End Source File
# Begin Source File

SOURCE=..\common\win32\osrtl_time.c
# End Source File
# Begin Source File

SOURCE=..\common\serial.h
# End Source File
# End Target
# End Project
