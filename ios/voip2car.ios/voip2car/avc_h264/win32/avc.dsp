# Microsoft Developer Studio Project File - Name="avc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=avc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "avc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "avc.mak" CFG="avc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "avc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "avc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "avc - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\common\include" /I "..\enc\include" /I "..\dec\include" /I "..\colorconvert\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "avc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /I "..\common\include" /I "..\enc\include" /I "..\dec\include" /I "..\colorconvert\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "avc - Win32 Release"
# Name "avc - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\codecs\avc.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\avc_bitstream.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\avcdec_api.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\avcenc_api.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\bitstream_io.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\block.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccrgb12toyuv420.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccrgb16toyuv420.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccrgb24torgb16.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccrgb24toyuv420.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccyuv420semiplnrtoyuv420plnr.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccyuv420toyuv420semi.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccyuv420toyuv422.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\ccyuv422toyuv420.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\cczoomrotation12.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\cczoomrotation16.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\cczoomrotation24.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\cczoomrotation32.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\cczoomrotationbase.cpp
# End Source File
# Begin Source File

SOURCE=..\colorconvert\src\cpvvideoblend.cpp
# End Source File
# Begin Source File

SOURCE=..\common\src\deblock.cpp
# End Source File
# Begin Source File

SOURCE=..\common\src\dpb.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\findhalfpel.cpp
# End Source File
# Begin Source File

SOURCE=..\common\src\fmo.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\header.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\headerd.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\init.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\intra_est.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\itrans.cpp
# End Source File
# Begin Source File

SOURCE=..\common\src\mb_access.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\motion_comp.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\motion_est.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\pred_inter.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\pred_intra.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\pvavcdecoder.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\pvavcdecoder_factory.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\pvavcencoder.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\pvavcencoder_factory.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\rate_control.cpp
# End Source File
# Begin Source File

SOURCE=..\common\src\reflist.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\residual.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\residuald.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\sad.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\sad_halfpel.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\slice.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\sliced.cpp
# End Source File
# Begin Source File

SOURCE=..\dec\src\vlc.cpp
# End Source File
# Begin Source File

SOURCE=..\enc\src\vlc_encode.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# End Target
# End Project
