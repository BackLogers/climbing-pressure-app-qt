QT += quick bluetooth serialport widgets network

SOURCES += \
        applogic.cpp \
        bleinputdevice.cpp \
        deviceinfo.cpp \
        inputdevice.cpp \
        main.cpp \
        serialinputdevice.cpp

resources.files = main.qml Graph.qml
resources.prefix = /$${TARGET}
RESOURCES += resources \
    resources.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH =

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    applogic.h \
    bleinputdevice.h \
    deviceinfo.h \
    inputdevice.h \
    serialinputdevice.h

DISTFILES += \
    Graph.qml
