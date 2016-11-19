# Config
QT += core gui webenginewidgets widgets
CONFIG += c++11

TARGET = RSTPad
QMAKE_EXTRA_TARGETS += first
TEMPLATE = app

# Defines
VERSION = 0.1.0
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Build
DESTDIR = $$OUT_PWD/app

DISTFILES += \
    rstpad.rc

RC_FILE = rstpad.rc
ICON = resources/icons/main.icns

# Sources
SOURCES += src/main.cpp\
    src/UI/MainWindow.cpp \
    src/UI/Code/CodeEdit.cpp \
    src/UI/Code/EditManipulator.cpp \
    src/UI/SettingsDlg.cpp \
    src/App.cpp \
    src/Config.cpp \
    src/PythonBridge.cpp \
    src/FileManager.cpp \
    src/RstRenderer.cpp \
    src/PythonException.cpp \
    src/PreviewManager.cpp \
    src/AppSchemeHandler.cpp \
    src/UI/PreviewView.cpp \
    src/UI/WebPage.cpp \
    src/UI/SearchReplaceDlg.cpp \
    src/UI/PygmentLexersDlg.cpp \
    src/UI/SearchReplaceHandler.cpp \
    src/UI/SearchJob.cpp \
    src/UI/SyntaxHelpWindow.cpp

HEADERS  += \
    src/UI/MainWindow.h \
    src/UI/Code/CodeEdit.h \
    src/UI/Code/EditManipulator.h \
    src/UI/SettingsDlg.h \
    src/App.h \
    src/Config.h \
    src/PythonBridge.h \
    src/FileManager.h \
    src/RstRenderer.h \
    src/PythonException.h \
    src/PreviewManager.h \
    src/AppSchemeHandler.h \
    src/UI/PreviewView.h \
    src/UI/WebPage.h \
    src/UI/SearchReplaceDlg.h \
    src/UI/PygmentLexersDlg.h \
    src/UI/SearchReplaceHandler.h \
    src/UI/SearchJob.h \
    src/UI/SyntaxHelpWindow.h

FORMS += src/UI/MainWindow.ui \
    src/UI/SettingsDlg.ui \
    src/UI/SearchReplaceDlg.ui \
    src/UI/PygmentLexersDlg.ui \
    src/UI/SyntaxHelpWindow.ui

RESOURCES += \
    resources/resources.qrc

# Local config
include(rstpad.local.pro)

# Web engine
debug {
    DEFINES += DEBUG_WEBENGINE_REMOTE_PORT=$$DEBUG_WEBENGINE_REMOTE_PORT
}

# Python
INCLUDEPATH += $$PYTHON_INCLUDE_PATH
LIBS += $$PYTHON_LINK

# Deploy Python libraries
PYTHON_SYS_TARGET_PATH = $${DESTDIR}/libs

win32|win64 {
    PYTHON_SYS_TARGET_PATH = $${DESTDIR}/libs
    deploy_python_libs.commands = "if NOT EXIST \"$${PYTHON_SYS_TARGET_PATH}\" ( xcopy /s /q /y /i \"$${PYTHON_SYS_PATH}\" \"$${PYTHON_SYS_TARGET_PATH}\" )"
    first.depends += deploy_python_libs
    QMAKE_EXTRA_TARGETS += deploy_python_libs
} else:osx {
    deploy_python_libs.files = $${PYTHON_SYS_PATH}
    deploy_python_libs.path = Contents/MacOS

    QMAKE_BUNDLE_DATA += deploy_python_libs
} else {
    warning("Not deplying Python libraries - platform not supported")
}
