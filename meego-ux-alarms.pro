VERSION = 0.1.3

QT += declarative opengl network dbus
CONFIG += mobility link_pkgconfig
MOBILITY += sensors multimedia
PKGCONFIG += \
    gconf-2.0 \
    mlite \
    libresourceqt1 \
    libealarm \
    libecal-1.2

TARGET = meego-ux-alarms
TEMPLATE = app
SOURCES += main.cpp \
    application.cpp \
    dialog.cpp \
    atoms.cpp \
    alarmcontrol.cpp
HEADERS += \
    application.h \
    dialog.h \
    atoms.h \
    alarmcontrol.h \
    alarmrequest.h

OBJECTS_DIR = .obj
MOC_DIR = .moc

target.files += $$TARGET
target.path += $$INSTALL_ROOT/usr/libexec/

share.files += *.qml
share.path += $$INSTALL_ROOT/usr/share/$$TARGET

desktop.files += meego-ux-alarms.desktop
desktop.path += $$INSTALL_ROOT/etc/xdg/autostart

INSTALLS += target share desktop

OTHER_FILES += \
    alarmclock.qml \
    eventreminder.qml \
    incomingcall.qml \
    taskreminder.qml \
    shutdownverification.qml

TRANSLATIONS += $${SOURCES} $${HEADERS} $${OTHER_FILES}
PROJECT_NAME = meego-ux-alarms

dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION} &&
dist.commands += git clone . $${PROJECT_NAME}-$${VERSION} &&
dist.commands += rm -fR $${PROJECT_NAME}-$${VERSION}/.git &&
dist.commands += mkdir -p $${PROJECT_NAME}-$${VERSION}/ts &&
dist.commands += lupdate $${TRANSLATIONS} -ts $${PROJECT_NAME}-$${VERSION}/ts/$${PROJECT_NAME}.ts &&
dist.commands += tar jcpvf $${PROJECT_NAME}-$${VERSION}.tar.bz2 $${PROJECT_NAME}-$${VERSION}
QMAKE_EXTRA_TARGETS += dist
