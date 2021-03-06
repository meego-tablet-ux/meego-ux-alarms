/*
 * This file was generated by qdbusxml2cpp version 0.7
 * Command line was: qdbusxml2cpp -c AlarmControl interfaces/alarmcontrol.xml -a alarmcontrol
 *
 * qdbusxml2cpp is Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef ALARMCONTROL_H_1307775551
#define ALARMCONTROL_H_1307775551

#include <QObject>
#include <QDBusAbstractAdaptor>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.meego.alarms
 */
class AlarmControl: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.meego.alarms")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.meego.alarms\">\n"
"    <method name=\"incomingCall\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"summary\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"body\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"acceptAction\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"rejectAction\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"sound\"/>\n"
"      <arg direction=\"in\" type=\"s\" name=\"imageURI\"/>\n"
"    </method>\n"
"    <method name=\"dropCall\"/>\n"
"  </interface>\n"
        "")
public:
    AlarmControl(QObject *parent);
    virtual ~AlarmControl();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    void dropCall();
    void incomingCall(const QString &summary, const QString &body, const QString &acceptAction, const QString &rejectAction, const QString &sound, const QString &imageURI);
Q_SIGNALS: // SIGNALS
};

#endif
