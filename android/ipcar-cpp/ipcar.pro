# -------------------------------------------------
# Project created by QtCreator 2010-03-18T20:16:35
# -------------------------------------------------
QT += xml \
    sql

#QT += xml \
#    network \
#    sql
    
DEFINES += PROJECT_VOIP2CAR
DEFINES += ANDROID_VERSION
DEFINES += HOME_PATH_SDCARD
DEFINES += ANDROID_VIDEO
DEFINES += SIMPLEAUDIO


TRANSLATIONS = voip2car.ts 
TARGET = ipcar
TEMPLATE = app
CONFIG += qaxcontainer \
    uitools
LIBS +=    -L \
    vPhonetCore/debug \
    ../linuxlibs/vpstacklib.a \
    ../linuxlibs/gsmlib.a  \
    ../linuxlibs/libavc.a 
    

OBJECTS_DIR = Build
MOC_DIR = Build
UI_DIR = Build
RCC_DIR = Build
    
    
SOURCES += ./QSound.cpp    ./BlinkingPushButton.cpp \
    ./CallSessionThread.cpp \
    ./ConnectionStatusIndicator.cpp \
    ./CallHandlerWidget.cpp \
    ./DialPad.cpp \
    ./FileChooser.cpp \
    ./VpStackChooserDecorator.cpp \
    ./VpStackContactDecorator.cpp \
    ./forms/AddContactForm.cpp \
    ./forms/CaptureForm.cpp \
    ./forms/SlidesForm.cpp \
    ./forms/VideoConfForm.cpp \
    ./forms/DirectoryForm.cpp \
    ./forms/CallHistoryForm.cpp \
    ./forms/ChatForm.cpp \
    ./forms/ConnectingForm.cpp \
    ./forms/ContactListForm.cpp \
    ./forms/FileTransferForm.cpp \
    ./forms/InfoForm.cpp \
    ./forms/LoginWindow.cpp \
    ./forms/MessageForm.cpp \
    ./forms/NavigationForm.cpp \
    ./forms/Settings/AudioForm.cpp \
    ./forms/Settings/CallManagementForm.cpp \
    ./forms/Settings/NetworkForm.cpp \
    ./forms/Settings/SettingsForm.cpp \
    ./forms/Settings/VideoForm.cpp \
    ./forms/SmsForm.cpp \
    ./forms/VideoCallForm.cpp \
    ./forms/VoiceCallForm.cpp \
    ./forms/PortForwardForm.cpp \
    ./HostDependent/Linux/WebBrowser.cpp \
    ./keyboard.cpp \
    ./keyboardbutton.cpp \
    ./main.cpp \
    ./mainwindow.cpp \
    ./SignalIntensityIndicator.cpp \
    ./Utils.cpp \
    VpStackLogDecorator.cpp \
    VpStackDecorator.cpp \
    VpStackAolDecorator.cpp \
    VpStackDbgLogDecorator.cpp \
    VpStackSoundDecorator.cpp \
    VpStackAnsweringMachineDecorator.cpp \
    forms/MmsForm.cpp videowidget.cpp widgethread.cpp jinistub.c
HEADERS += videowidget.h widgethread.h ./BlinkingPushButton.h QSound.h\
    ./CallHandlerWidget.h \
    ./CallSessionThread.h \
    ./Commands.h \
    ./ConnectionStatusIndicator.h \
    ./DialPad.h \
    ./FileChooser.h \
    ./VpStackChooserDecorator.h \
    ./VpStackContactDecorator.h \
    ./forms/AddContactForm.h \
    ./forms/CaptureForm.h \
    ./forms/SlidesForm.h \
    ./forms/VideoConfForm.h \
    ./forms/DirectoryForm.h \
    ./forms/CallHistoryForm.h \
    ./forms/ChatForm.h \
    ./forms/ConnectingForm.h \
    ./forms/ContactListForm.h \
    ./forms/FileTransferForm.h \
    ./forms/InfoForm.h \
    ./forms/LoginWindow.h \
    ./forms/MessageForm.h \
    ./forms/NavigationForm.h \
    ./forms/Settings/AudioForm.h \
    ./forms/Settings/CallManagementForm.h \
    ./forms/Settings/NetworkForm.h \
    ./forms/Settings/SettingsForm.h \
    ./forms/Settings/VideoForm.h \
    ./forms/SmsForm.h \
    ./forms/VideoCallForm.h \
    ./forms/VoiceCallForm.h \
    ./forms/PortForwardForm.h \
    ./HostDependent/WebBrowser.h \
    ./HostDependent/Linux/webaxwidget.h \
    ./Keyboard.h \
    ./keyboardbutton.h \
    ./mainwindow.h \
    ./SignalIntensityIndicator.h \
    ./Utils.h \
    ./WidgetFactory.h \
    AddressBook.h \
    VpStackLogDecorator.h \
    LogEntry.h \
    VpStackDecorator.h \
    VpStackAolDecorator.h \
    VpStackDbgLogDecorator.h \
    VpStackSoundDecorator.h \
    VpStackAnsweringMachineDecorator.h \
    forms/MmsForm.h 
    
OTHER_FILES += qsdefault.qss \
    RESOURCES \
    += \
    Keyboard.qrc \
    TODO.txt
FORMS += \
	./qss/forms/add.ui \
    ./qss/forms/browser.ui \
    ./qss/forms/chat.ui \
    ./qss/forms/info.ui \
    ./qss/forms/log.ui \
    ./qss/forms/login.ui \
    ./qss/forms/main.ui \
    ./qss/forms/settings.ui \
    ./qss/forms/videoCall.ui \
    ./qss/forms/message.ui \
    ./qss/forms/contacts.ui \
    ./qss/forms/progress.ui \
    ./qss/forms/ftp.ui \
    ./qss/forms/settings/network.ui \
    ./qss/forms/settings/call.ui \
    ./qss/forms/settings/audio.ui \
    ./qss/forms/settings/video.ui \
    qss/forms/dial.ui
RESOURCES += Voip2Car.qrc
