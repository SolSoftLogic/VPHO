# -------------------------------------------------
# Project created by QtCreator 2010-03-18T20:16:35
# -------------------------------------------------
QT += xml \
    network \
    multimedia
TRANSLATIONS = IP-Pad.ts 
TARGET = vPhonet
TEMPLATE = app
CONFIG += qaxcontainer \
    uitools
LIBS += -L \
    vPhonetCore/debug \
    -lvPhonetCore
SOURCES += ./BlinkingPushButton.cpp \
    ./CallSessionThread.cpp \
    ./ConnectionStatusIndicator.cpp \
    ./DialPad.cpp \
    ./forms/AddContactForm.cpp \
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
    ./HostDependent/Windows/WebBrowser.cpp \
    ./keyboard.cpp \
    ./keyboardbutton.cpp \
    ./main.cpp \
    ./mainwindow.cpp \
    ./Model.cpp \
    ./SignalIntensityIndicator.cpp \
    ./StateMachine.cpp \
    ./Utils.cpp \
    VpStackLogDecorator.cpp \
    VpStackDecorator.cpp \
    VpStackAolDecorator.cpp \
    VpStackDbgLogDecorator.cpp \
    VpStackSoundDecorator.cpp \
    VpStackAnsweringMachineDecorator.cpp \
    forms/MmsForm.cpp \
    VpStackContactDecorator.cpp \
    VpStackChooserDecorator.cpp \
    MemLeakDetect.cpp \
    FileChooser.cpp \
    CallHandlerWidget.cpp \
    forms/VideoConfForm.cpp \
    forms/SlidesForm.cpp \
    forms/DirectoryForm.cpp \
    forms/CaptureForm.cpp \
    forms/PortForwardForm.cpp
HEADERS += ./BlinkingPushButton.h \
    ./CallSessionThread.h \
    ./Commands.h \
    ./ConnectionStatusIndicator.h \
    ./DialPad.h \
    ./forms/AddContactForm.h \
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
    ./HostDependent/WebBrowser.h \
    ./HostDependent/Windows/webaxwidget.h \
    ./keyboard.h \
    ./keyboardbutton.h \
    ./mainwindow.h \
    ./Model.h \
    ./SignalIntensityIndicator.h \
    ./StateMachine.h \
    ./StateMachineImpl.h \
    ./Utils.h \
    ./WidgetFactory.h \
    AddressBook.h \
    VpStackLogDecorator.h \
    LogEntry.h \
    VpStackDecorator.h \
    VpStackAolDecorator.h \
    VpStackDbgLogDecorator.h \
    VpStackSoundDecorator.h \
    ./core/VpSttackInterface.h \
    VpStackAnsweringMachineDecorator.h \
    forms/MmsForm.h \
    core/vpstack/VpStackInterface.h \
    forms/VideoConfForm.h \
    forms/SlidesForm.h \
    forms/DirectoryForm.h \
    forms/CaptureForm.h \
    forms/PortForwardForm.h
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
    ./qss/forms/smsForm.ui \
    ./qss/forms/mmsForm.ui \
    ./qss/forms/message.ui \
    ./qss/forms/contacts.ui \
    ./qss/forms/progress.ui \
    ./qss/forms/ftp.ui \
    ./qss/forms/settings/network.ui \
    ./qss/forms/settings/call.ui \
    ./qss/forms/settings/audio.ui \
    ./qss/forms/settings/video.ui \
    ./qss/forms/videoConfCall.ui \
    qss/forms/dial.ui \
    qss/forms/videoConf.ui \
    qss/forms/sms.ui \
    qss/forms/slides.ui \
    qss/forms/mms.ui \
    qss/forms/directory.ui \
    qss/forms/capture.ui \
    qss/forms/voiceCall.ui \
    qss-Voip2Car/forms/videoConf.ui \
    qss-Voip2Car/forms/videoCall.ui \
    qss-Voip2Car/forms/sms.ui \
    qss-Voip2Car/forms/slides.ui \
    qss-Voip2Car/forms/settings.ui \
    qss-Voip2Car/forms/progress.ui \
    qss-Voip2Car/forms/mms.ui \
    qss-Voip2Car/forms/message.ui \
    qss-Voip2Car/forms/main.ui \
    qss-Voip2Car/forms/login.ui \
    qss-Voip2Car/forms/log.ui \
    qss-Voip2Car/forms/info.ui \
    qss-Voip2Car/forms/ftp.ui \
    qss-Voip2Car/forms/directory.ui \
    qss-Voip2Car/forms/dial.ui \
    qss-Voip2Car/forms/contacts.ui \
    qss-Voip2Car/forms/chat.ui \
    qss-Voip2Car/forms/capture.ui \
    qss-Voip2Car/forms/browser.ui \
    qss-Voip2Car/forms/add.ui \
    qss-Voip2Car/forms/voiceCall.ui \
    qss-Voip2Car/forms/settings/network.ui \
    qss-Voip2Car/forms/settings/call.ui \
    qss-Voip2Car/forms/settings/audio.ui \
    qss-Voip2Car/forms/settings/video.ui \
    qss-IP-Pad/forms/videoConf.ui \
    qss-IP-Pad/forms/videoCall.ui \
    qss-IP-Pad/forms/sms.ui \
    qss-IP-Pad/forms/slides.ui \
    qss-IP-Pad/forms/settings.ui \
    qss-IP-Pad/forms/progress.ui \
    qss-IP-Pad/forms/mms.ui \
    qss-IP-Pad/forms/message.ui \
    qss-IP-Pad/forms/main.ui \
    qss-IP-Pad/forms/login.ui \
    qss-IP-Pad/forms/log.ui \
    qss-IP-Pad/forms/info.ui \
    qss-IP-Pad/forms/ftp.ui \
    qss-IP-Pad/forms/directory.ui \
    qss-IP-Pad/forms/dial.ui \
    qss-IP-Pad/forms/contacts.ui \
    qss-IP-Pad/forms/chat.ui \
    qss-IP-Pad/forms/capture.ui \
    qss-IP-Pad/forms/browser.ui \
    qss-IP-Pad/forms/add.ui \
    qss-IP-Pad/forms/voiceCall.ui \
    qss-IP-Pad/forms/settings/network.ui \
    qss-IP-Pad/forms/settings/call.ui \
    qss-IP-Pad/forms/settings/audio.ui \
    qss-IP-Pad/forms/settings/video.ui \
    qss-Voip2Car/forms/portForward.ui
RESOURCES += IP-Pad.qrc
