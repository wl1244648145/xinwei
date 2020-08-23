# Microsoft Developer Studio Project File - Name="tVoice" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=tVoice - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "tVoice.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tVoice.mak" CFG="tVoice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tVoice - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "tVoice - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/prj_mcwill/prj_bts/layer3/tVoice", JJABAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "tVoice - Win32 Release"

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
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "tVoice - Win32 Debug"

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
# ADD CPP /nologo /MTd /W3 /Gm /GR /GX /ZI /Od /I "H" /I "./../../../prj_common/" /I "./../../../prj_common/apframework/h" /I "./../../../prj_common/apframework/src" /I "./../messages/voicemsgs/H" /I "./../common" /I "./../messages/H" /I "./../OAM" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "__WIN32_SIM__" /D "M_TGT_L3" /D "NDEBUG" /D "__UNITTEST__" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib ws2_32.lib FWKLIBwdBTS.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "tVoice - Win32 Release"
# Name "tVoice - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\messages\voicemsgs\SRC\CallSignalMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\main.cpp
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\SRC\OtherMsg.cpp
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\SRC\timeoutVoiceMsg.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\tVoice.cpp
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\SRC\VAC_session_interface.cpp
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\SRC\VAC_Voice_Data.cpp
# End Source File
# Begin Source File

SOURCE=.\SRC\voiceFSM.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\messages\voicemsgs\H\CallSignalMsg.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\cpe_msgs_struct.h
# End Source File
# Begin Source File

SOURCE=..\COMMON\L3voiceMsgID.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\OtherMsg.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\timeoutVoiceMsg.h
# End Source File
# Begin Source File

SOURCE=.\H\tVoice.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\VAC_session_interface.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\VAC_Voice_Data.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\VDR_Voice_Data.h
# End Source File
# Begin Source File

SOURCE=..\messages\voicemsgs\H\voice_msgs_struct.h
# End Source File
# Begin Source File

SOURCE=.\H\voiceCommon.h
# End Source File
# Begin Source File

SOURCE=.\H\voiceFSM.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project