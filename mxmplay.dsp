# Microsoft Developer Studio Project File - Name="mxmplay" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=mxmplay - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "mxmplay.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "mxmplay.mak" CFG="mxmplay - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "mxmplay - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "mxmplay - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "mxmplay - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GX- /O1 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x414 /d "NDEBUG"
# ADD RSC /l 0x414 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 gdi32.lib dsound.lib user32.lib /nologo /subsystem:windows /machine:I386
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=upx --best Release/mxmplay.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "mxmplay - Win32 Debug"

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
# ADD CPP /nologo /W3 /Gm /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x414 /d "_DEBUG"
# ADD RSC /l 0x414 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 dsound.lib user32.lib gdi32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "mxmplay - Win32 Release"
# Name "mxmplay - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\3d.c
# End Source File
# Begin Source File

SOURCE=.\DSIO.ASM

!IF  "$(CFG)" == "mxmplay - Win32 Release"

# Begin Custom Build
InputPath=.\DSIO.ASM

"dsio.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw dsio.asm -o dsio.obj -f win32

# End Custom Build

!ELSEIF  "$(CFG)" == "mxmplay - Win32 Debug"

# Begin Custom Build
InputPath=.\DSIO.ASM

"dsio.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw dsio.asm -o dsio.obj -f win32

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\leepra.c
# End Source File
# Begin Source File

SOURCE=.\MAIN.C
# End Source File
# Begin Source File

SOURCE=.\MXMFILE.ASM

!IF  "$(CFG)" == "mxmplay - Win32 Release"

# Begin Custom Build
InputPath=.\MXMFILE.ASM

"mxmfile.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw mxmfile.asm -o mxmfile.obj -f win32

# End Custom Build

!ELSEIF  "$(CFG)" == "mxmplay - Win32 Debug"

# Begin Custom Build
InputPath=.\MXMFILE.ASM

"mxmfile.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw mxmfile.asm -o mxmfile.obj -f win32

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\MXMPLAY.ASM

!IF  "$(CFG)" == "mxmplay - Win32 Release"

# Begin Custom Build
InputPath=.\MXMPLAY.ASM

"mxmplay.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw mxmplay.asm -o mxmplay.obj -f win32

# End Custom Build

!ELSEIF  "$(CFG)" == "mxmplay - Win32 Debug"

# Begin Custom Build
InputPath=.\MXMPLAY.ASM

"mxmplay.obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw mxmplay.asm -o mxmplay.obj -f win32

# End Custom Build

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\3d.h
# End Source File
# Begin Source File

SOURCE=.\MXMPLAY.H
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
