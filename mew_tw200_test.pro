QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chgbox/chgbox.cpp \
    data_process/worker_thread.cpp \
    main.cpp \
    mainwindow.cpp \
    qextserial/qextserialport.cpp \
    qextserial/qextserialport_win.cpp \
    utils/utils.cpp

HEADERS += \
    chgbox/chgbox.h \
    data_process/worker_thread.h \
    mainwindow.h \
    mew_type_define.h \
    qextserial/qextserialport.h \
    qextserial/qextserialport_global.h \
    utils/utils.h

FORMS += \
    mainwindow.ui

RC_ICONS = images/maven_icon.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    image.qrc
