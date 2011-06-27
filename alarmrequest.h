/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef ALARMREQUEST_H
#define ALARMREQUEST_H

#include <QObject>

class AlarmRequest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString summary READ getSummary);
    Q_PROPERTY(QString body READ getBody);
    Q_PROPERTY(QString acceptAction READ getAcceptAction);
    Q_PROPERTY(QString rejectAction READ getRejectAction);
    Q_PROPERTY(QString imageUri READ getImageUri);
    Q_PROPERTY(QString sound READ getSound);
    Q_PROPERTY(int type READ getType);
    Q_PROPERTY(QString uid READ getUid);
public:
    explicit AlarmRequest(const QString summary,
                          const QString body,
                          const QString acceptAction,
                          const QString rejectAction,
                          const QString imageUri,
                          const QString sound,
                          int alarmType,
                          const QString uid,
                          QObject *parent = 0) :
        QObject(parent),
        m_summary(summary),
        m_body(body),
        m_acceptAction(acceptAction),
        m_rejectAction(rejectAction),
        m_imageUri(imageUri),
        m_sound(sound),
        m_type(alarmType),
        m_uid(uid) {}

    enum AlarmType {
        AlarmClock = 0,
        EventReminder = 1,
        TaskReminder = 2,
        IncomingCall = 3
    };

    QString getSummary() const {
        return m_summary;
    }
    QString getBody() const {
        return m_body;
    }
    QString getAcceptAction() const {
        return m_acceptAction;
    }
    QString getRejectAction() const {
        return m_rejectAction;
    }
    QString getSound() const {
        return m_sound;
    }
    QString getImageUri() const {
        return m_imageUri;
    }
    int getType() {
        return m_type;
    }
    QString getUid() const {
        return m_uid;
    }

private:
    QString m_summary;
    QString m_body;
    QString m_acceptAction;
    QString m_rejectAction;
    QString m_imageUri;
    QString m_sound;
    int m_type;
    QString m_uid;
};

#endif // ALARMREQUEST_H
