cmake_minimum_required(VERSION 3.13)

if(NOT KRISTALL_VERSION)
    execute_process(COMMAND git describe --tags --abbrev=0
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE KRISTALL_CMAKE_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT KRISTALL_CMAKE_VERSION)
        set(KRISTALL_CMAKE_VERSION "0.0.0")
    endif()

    execute_process(COMMAND git describe --tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE KRISTALL_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT KRISTALL_VERSION)
        set(KRISTALL_VERSION "development")
    endif()
endif()

# CMake requires the following format for versions:
# <major>[.<minor>[.<patch>[.<tweak>]]]]
string(REPLACE "V" "" KRISTALL_CMAKE_VERSION ${KRISTALL_CMAKE_VERSION})
project(kristall VERSION ${KRISTALL_CMAKE_VERSION} LANGUAGES CXX)

set(QT_VERSION_MAJOR 5)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# QXmpp requires C++17.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS
    Multimedia MultimediaWidgets Network Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Multimedia MultimediaWidgets Network Widgets REQUIRED)

set(PROJECT_SOURCES
    src/browsertab.cpp
    src/certificatehelper.cpp
    src/cryptoidentity.cpp
    src/dialogs/certificateiodialog.cpp
    src/dialogs/certificatemanagementdialog.cpp
    src/dialogs/certificateselectiondialog.cpp
    src/dialogs/newidentitiydialog.cpp
    src/dialogs/settingsdialog.cpp
    src/documentoutlinemodel.cpp
    src/documentstyle.cpp
    src/favouritecollection.cpp
    src/identitycollection.cpp
    src/ioutil.cpp
    src/localization.cpp
    src/main.cpp
    src/mainwindow.cpp
    src/renderers/htmlrenderer.cpp
    src/renderers/markdownrenderer.cpp
    src/renderers/renderhelpers.cpp
    src/renderers/textstyleinstance.cpp
    src/widgets/browsertabbar.cpp
    src/widgets/browsertabwidget.cpp
    src/widgets/kristalltextbrowser.cpp
    src/widgets/mediaplayer.cpp
    src/mimeparser.cpp
    src/protocolhandler.cpp
    src/protocols/abouthandler.cpp
    src/protocols/filehandler.cpp
    src/protocols/fingerclient.cpp
    src/protocols/geminiclient.cpp
    src/protocols/gopherclient.cpp
    src/protocols/webclient.cpp
    src/protocolsetup.cpp
    src/renderers/geminirenderer.cpp
    src/renderers/gophermaprenderer.cpp
    src/renderers/plaintextrenderer.cpp
    src/ssltrust.cpp
    src/tabbrowsinghistory.cpp
    src/trustedhost.cpp
    src/trustedhostcollection.cpp
    src/widgets/elidelabel.cpp
    src/widgets/searchbar.cpp
    src/widgets/ssltrusteditor.cpp
    src/widgets/favouritepopup.cpp
    src/widgets/favouritebutton.cpp
    src/widgets/querydialog.ui
    src/widgets/querydialog.cpp
    src/cachehandler.cpp
    src/widgets/searchbox.cpp
    src/browsertab.ui
    src/dialogs/certificateiodialog.ui
    src/dialogs/certificatemanagementdialog.ui
    src/dialogs/certificateselectiondialog.ui
    src/dialogs/newidentitiydialog.ui
    src/dialogs/settingsdialog.ui
    src/mainwindow.ui
    src/widgets/mediaplayer.ui
    src/widgets/ssltrusteditor.ui
    src/fonts.qrc
    lib/BreezeStyleSheets/breeze.qrc
    src/builtins.qrc
    src/icons.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(${PROJECT_NAME}
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
        )
    endif()
endif()

message(STATUS "Building Kristall ${KRISTALL_VERSION}")

target_compile_definitions(${PROJECT_NAME} PUBLIC
    KRISTALL_VERSION=${KRISTALL_VERSION})

add_subdirectory(lib/luis-l-gist)
target_link_libraries(${PROJECT_NAME} PRIVATE luis-l-gist)

find_package(PkgConfig REQUIRED)
pkg_check_modules(cmark IMPORTED_TARGET cmark)

if(cmark_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::cmark)
else()
    message(STATUS "Using in-tree cmark")
    add_subdirectory(lib/cmark)
    target_link_libraries(${PROJECT_NAME} PRIVATE cmark)
endif()

pkg_check_modules(gumbo-parser IMPORTED_TARGET gumbo)

if(gumbo_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE PkgConfig::gumbo-parser)
else()
    message(STATUS "Using in-tree gumbo-parser")
    add_subdirectory(lib/gumbo-parser)
    target_link_libraries(${PROJECT_NAME} PRIVATE gumbo-parser)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets)

target_include_directories(${PROJECT_NAME} PRIVATE src)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(${PROJECT_NAME})
endif()

find_package(OpenSSL 1.1 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)

if(NOT UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE iconv)
endif()
