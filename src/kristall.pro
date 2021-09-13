QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets network multimedia multimediawidgets


# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += KRISTALL_VERSION="\"$(shell cd $$PWD; git describe --tags)\""

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

!win32: LIBS += -lcrypto

# We need iconv on non-linux platforms
!linux: LIBS += -liconv

# Initialize build flags from environment variables.
QMAKE_CFLAGS   *= $$(CFLAGS)
QMAKE_CXXFLAGS *= $$(CXXFLAGS)
QMAKE_CPPFLAGS *= $$(CPPFLAGS)
QMAKE_LFLAGS   *= $$(LDFLAGS)

QMAKE_CFLAGS += -Wno-unused-parameter -Werror=return-type
QMAKE_CXXFLAGS += -Wno-unused-parameter -Werror=return-type

# Enable C++17
QMAKE_CXXFLAGS += -std=c++17
CONFIG += c++17

# avoid x86 OOM errors
CONFIG += resources_big

win32-msvc {
    # message("Use windows/msvc build")
    QMAKE_CFLAGS -= -Wno-unused-parameter
    QMAKE_CXXFLAGS -= -Wno-unused-parameter

    QMAKE_CFLAGS += /permissive-
    QMAKE_CXXFLAGS += /permissive-
    LIBS -= -lcrypto
    LIBS += "C:\Program Files\OpenSSL\lib\libcrypto.lib"
    INCLUDEPATH += "C:\Program Files\OpenSSL\include"
}

win32-g++ {
    # message("Use windows/mingw build")
    INCLUDEPATH += $$quote("C:\Qt\Tools\OpenSSL\Win_x64\include")
    LIBS += -L$$quote(C:\Qt\Tools\OpenSSL\Win_x64\lib)
    LIBS += -llibcrypto
}

macx {
    # Homebrew include paths
    contains(QMAKE_HOST.arch, arm.*):{
        INCLUDEPATH += /opt/homebrew/opt/qt5/include
        LIBS += -L/opt/homebrew/opt/qt5/lib
    
        INCLUDEPATH += /opt/homebrew/opt/openssl/include
        LIBS += -L/opt/homebrew/opt/openssl/lib
    } else {
        INCLUDEPATH += /usr/local/opt/qt/include
        LIBS += -L/usr/local/opt/qt/lib

        INCLUDEPATH += /usr/local/opt/openssl/include
        LIBS += -L/usr/local/opt/openssl/lib
    }

    ICON = icons/AppIcon.icns
}

android: include(/home/felix/projects/android-hass/android-sdk/android_openssl/openssl.pri)

# android {
#   INCLUDEPATH += /home/felix/projects/android-hass/android-sdk/android_openssl/static/include
#   LIBS += -L /home/felix/projects/android-hass/android-sdk/android_openssl/static/lib/arm/
# }


external-cmark {
    CONFIG += link_pkgconfig
    PKGCONFIG += libcmark
} else {
    include($$PWD/../lib/cmark/cmark.pri)
}


external-gumbo-parser {
    CONFIG += link_pkgconfig
    PKGCONFIG += gumbo
} else {
    include($$PWD/../lib/gumbo-parser/gumbo-parser.pri)
}

INCLUDEPATH += $$PWD/../lib/luis-l-gist/
DEPENDPATH += $$PWD/../lib/luis-l-gist/

SOURCES += \
    ../lib/luis-l-gist/interactiveview.cpp \
    browsertab.cpp \
    certificatehelper.cpp \
    cryptoidentity.cpp \
    dialogs/certificateiodialog.cpp \
    dialogs/certificatemanagementdialog.cpp \
    dialogs/certificateselectiondialog.cpp \
    dialogs/newidentitiydialog.cpp \
    dialogs/settingsdialog.cpp \
    documentoutlinemodel.cpp \
    documentstyle.cpp \
    favouritecollection.cpp \
    identitycollection.cpp \
    ioutil.cpp \
    localization.cpp \
    main.cpp \
    mainwindow.cpp \
    renderers/htmlrenderer.cpp \
    renderers/markdownrenderer.cpp \
    renderers/renderhelpers.cpp \
    renderers/textstyleinstance.cpp \
    widgets/browsertabbar.cpp \
    widgets/browsertabwidget.cpp \
    widgets/kristalltextbrowser.cpp \
    widgets/mediaplayer.cpp \
    mimeparser.cpp \
    protocolhandler.cpp \
    protocols/abouthandler.cpp \
    protocols/filehandler.cpp \
    protocols/fingerclient.cpp \
    protocols/geminiclient.cpp \
    protocols/gopherclient.cpp \
    protocols/webclient.cpp \
    protocolsetup.cpp \
    renderers/geminirenderer.cpp \
    renderers/gophermaprenderer.cpp \
    renderers/plaintextrenderer.cpp \
    ssltrust.cpp \
    tabbrowsinghistory.cpp \
    trustedhost.cpp \
    trustedhostcollection.cpp \
    widgets/elidelabel.cpp \
    widgets/searchbar.cpp \
    widgets/ssltrusteditor.cpp \
    widgets/favouritepopup.cpp \
    widgets/favouritebutton.cpp \
    cachehandler.cpp \
    widgets/searchbox.cpp

HEADERS += \
    ../lib/luis-l-gist/interactiveview.hpp \
    browsertab.hpp \
    certificatehelper.hpp \
    cryptoidentity.hpp \
    dialogs/certificateiodialog.hpp \
    dialogs/certificatemanagementdialog.hpp \
    dialogs/certificateselectiondialog.hpp \
    dialogs/newidentitiydialog.hpp \
    dialogs/settingsdialog.hpp \
    documentoutlinemodel.hpp \
    documentstyle.hpp \
    favouritecollection.hpp \
    identitycollection.hpp \
    ioutil.hpp \
    kristall.hpp \
    localization.hpp \
    mainwindow.hpp \
    renderers/htmlrenderer.hpp \
    renderers/markdownrenderer.hpp \
    renderers/textstyleinstance.hpp \
    widgets/browsertabbar.hpp \
    widgets/browsertabwidget.hpp \
    widgets/kristalltextbrowser.hpp \
    widgets/mediaplayer.hpp \
    mimeparser.hpp \
    protocolhandler.hpp \
    protocols/abouthandler.hpp \
    protocols/filehandler.hpp \
    protocols/fingerclient.hpp \
    protocols/geminiclient.hpp \
    protocols/gopherclient.hpp \
    protocols/webclient.hpp \
    protocolsetup.hpp \
    renderers/geminirenderer.hpp \
    renderers/gophermaprenderer.hpp \
    renderers/plaintextrenderer.hpp \
    ssltrust.hpp \
    tabbrowsinghistory.hpp \
    trustedhost.hpp \
    trustedhostcollection.hpp \
    widgets/elidelabel.hpp \
    widgets/searchbar.hpp \
    widgets/ssltrusteditor.hpp \
    widgets/favouritepopup.hpp \
    widgets/favouritebutton.hpp \
    cachehandler.hpp \
    widgets/searchbox.hpp

FORMS += \
  browsertab.ui \
  dialogs/certificateiodialog.ui \
  dialogs/certificatemanagementdialog.ui \
  dialogs/certificateselectiondialog.ui \
  dialogs/newidentitiydialog.ui \
  dialogs/settingsdialog.ui \
  mainwindow.ui \
  widgets/mediaplayer.ui \
  widgets/ssltrusteditor.ui

CONFIG += lrelease embed_translations

TRANSLATIONS += \
    ../translations/kristall_en_US.ts \
    ../translations/kristall_ru.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
  ./fonts.qrc \
  ../lib/BreezeStyleSheets/breeze.qrc \
  builtins.qrc \
  icons.qrc

# Embed icon resource on Windows platforms
win32:RC_ICONS += ./icons/kristall.ico

DISTFILES += \
  gophertypes.txt
