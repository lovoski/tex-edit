QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    src/conn_dialog.cpp \
    src/main.cpp \
    src/main_window.cpp \
    src/openfile_dialog.cpp \
    src/socket_thread.cpp \
    src/image_dialog.cpp

HEADERS += \
    3rdparty/unix/include/* \
    3rdparty/unix/include/Qsci/* \
    3rdparty/unix/include/qt5/* \
    include/main_window.h \
    include/socket_thread.h \
    include/conn_dialog.h \
    include/openfile_dialog.h \
    include/image_dialog.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/unix/release/ -lpoppler
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/unix/debug/ -lpoppler
else:unix: LIBS += -L$$PWD/3rdparty/unix/ -lpoppler

INCLUDEPATH += $$PWD/3rdparty/unix
DEPENDPATH += $$PWD/3rdparty/unix

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/unix/release/ -lpoppler-qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/unix/debug/ -lpoppler-qt5
else:unix: LIBS += -L$$PWD/3rdparty/unix/ -lpoppler-qt5

INCLUDEPATH += $$PWD/3rdparty/unix
DEPENDPATH += $$PWD/3rdparty/unix

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/unix/release/ -lqscintilla2_qt5
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/unix/debug/ -lqscintilla2_qt5
else:unix: LIBS += -L$$PWD/3rdparty/unix/ -lqscintilla2_qt5

INCLUDEPATH += $$PWD/3rdparty/unix
DEPENDPATH += $$PWD/3rdparty/unix

FORMS += \
    ui/openfile_dialog.ui \
    ui/conn_dialog.ui \
    ui/image_dialog.ui

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/unix/release/ -lpoppler-glib
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/unix/debug/ -lpoppler-glib
else:unix: LIBS += -L$$PWD/3rdparty/unix/ -lpoppler-glib

INCLUDEPATH += $$PWD/3rdparty/unix
DEPENDPATH += $$PWD/3rdparty/unix

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/3rdparty/unix/release/ -lpoppler-cpp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/3rdparty/unix/debug/ -lpoppler-cpp
else:unix: LIBS += -L$$PWD/3rdparty/unix/ -lpoppler-cpp

INCLUDEPATH += $$PWD/3rdparty/unix
DEPENDPATH += $$PWD/3rdparty/unix

RESOURCES += \
    resource/store.qrc
