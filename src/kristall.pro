QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network multimedia multimediawidgets


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

LIBS += -lcrypto
QMAKE_CFLAGS += -Wno-unused-parameter -Werror=return-type
QMAKE_CXXFLAGS += -Wno-unused-parameter -Werror=return-type

# Enable C++17
QMAKE_CXXFLAGS += -std=c++17
CONFIG += c++17

win32-msvc {
    QMAKE_CFLAGS -= -Wno-unused-parameter
    QMAKE_CXXFLAGS -= -Wno-unused-parameter

    QMAKE_CFLAGS += /permissive-
    QMAKE_CXXFLAGS += /permissive-
    LIBS -= -lcrypto
    LIBS += "C:\Program Files\OpenSSL\lib\libcrypto.lib"
    INCLUDEPATH += "C:\Program Files\OpenSSL\include"
}

android: include(/home/felix/projects/android-hass/android-sdk/android_openssl/openssl.pri)

# android {
#   INCLUDEPATH += /home/felix/projects/android-hass/android-sdk/android_openssl/static/include
#   LIBS += -L /home/felix/projects/android-hass/android-sdk/android_openssl/static/lib/arm/
# }

INCLUDEPATH += $$PWD/../lib/luis-l-gist/
DEPENDPATH += $$PWD/../lib/luis-l-gist/

SOURCES += \
    ../lib/luis-l-gist/interactiveview.cpp \
    abouthandler.cpp \
    browsertab.cpp \
    certificatehelper.cpp \
    certificatemanagementdialog.cpp \
    certificateselectiondialog.cpp \
    cryptoidentity.cpp \
    documentoutlinemodel.cpp \
    documentstyle.cpp \
    elidelabel.cpp \
    favouritecollection.cpp \
    filehandler.cpp \
    fingerclient.cpp \
    geminiclient.cpp \
    geminirenderer.cpp \
    gopherclient.cpp \
    gophermaprenderer.cpp \
    identitycollection.cpp \
    ioutil.cpp \
    main.cpp \
    mainwindow.cpp \
    mediaplayer.cpp \
    mimeparser.cpp \
    newidentitiydialog.cpp \
    plaintextrenderer.cpp \
    protocolhandler.cpp \
    protocolsetup.cpp \
    searchbar.cpp \
    settingsdialog.cpp \
    ssltrust.cpp \
    ssltrusteditor.cpp \
    tabbrowsinghistory.cpp \
    trustedhost.cpp \
    trustedhostcollection.cpp \
    webclient.cpp

HEADERS += \
    ../lib/luis-l-gist/interactiveview.hpp \
    abouthandler.hpp \
    browsertab.hpp \
    certificatehelper.hpp \
    certificatemanagementdialog.hpp \
    certificateselectiondialog.hpp \
    cryptoidentity.hpp \
    documentoutlinemodel.hpp \
    documentstyle.hpp \
    elidelabel.hpp \
    favouritecollection.hpp \
    filehandler.hpp \
    fingerclient.hpp \
    geminiclient.hpp \
    geminirenderer.hpp \
    gopherclient.hpp \
    gophermaprenderer.hpp \
    identitycollection.hpp \
    ioutil.hpp \
    kristall.hpp \
    mainwindow.hpp \
    mediaplayer.hpp \
    mimeparser.hpp \
    newidentitiydialog.hpp \
    plaintextrenderer.hpp \
    protocolhandler.hpp \
    protocolsetup.hpp \
    searchbar.hpp \
    settingsdialog.hpp \
    ssltrust.hpp \
    ssltrusteditor.hpp \
    tabbrowsinghistory.hpp \
    trustedhost.hpp \
    trustedhostcollection.hpp \
    webclient.hpp

FORMS += \
    browsertab.ui \
    certificatemanagementdialog.ui \
    certificateselectiondialog.ui \
    mainwindow.ui \
    mediaplayer.ui \
    newidentitiydialog.ui \
    settingsdialog.ui \
    ssltrusteditor.ui

TRANSLATIONS += \
    kristall_en_US.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  ../lib/BreezeStyleSheets/breeze.qrc \
  builtins.qrc \
  icons.qrc

DISTFILES += \
  gophertypes.txt
