/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QSettings>
#include <QTranslator>
#include <QMediaPlayer>
#include <QOrientationSensor>

#include "alarmrequest.h"

QTM_USE_NAMESPACE

class Dialog;
class Desktop;
class MGConfItem;

class Application : public QApplication
{
    Q_OBJECT
    Q_PROPERTY(int orientation READ getOrientation NOTIFY orientationChanged);
    Q_PROPERTY(bool orientationLocked READ getOrientationLocked WRITE setOrientationLocked);
    Q_PROPERTY(int foregroundWindow READ foregroundWindow NOTIFY foregroundWindowChanged);
public:
    explicit Application(int & argc, char ** argv);
    ~Application();

    int getOrientation() {
        return m_orientation;
    }

    // dummy implementation to make the components library happy
    bool getOrientationLocked() { return false; }
    void setOrientationLocked(bool) {}

    QSettings *themeConfig;

    int foregroundWindow();

    Q_INVOKABLE void incomingCall(QString summary,
                                  QString body,
                                  QString acceptAction,
                                  QString rejectAction,
                                  QString sound,
                                  QString imageURI);

signals:
    void orientationChanged();
    void foregroundWindowChanged();

public slots:
    void cleanupDialog(bool loadNextRequest = true);
    void dropCall();
    void triggerAction(QString action);
    void cancelAlarm();

private slots:
    void updateOrientation();
    void updateSecondaryRinger();
    void updateIncomingCallPath();
    void mediaStatusChanged(QMediaPlayer::MediaStatus);
    void enqueue(AlarmRequest *);

private:
    void showCurrentRequest();

    int m_orientation;
    QOrientationSensor m_orientationSensor;
    bool m_orientationSensorAvailable;
    QTranslator m_translator;

    QString m_primaryRinger;
    QString m_secondaryRinger;
    MGConfItem *m_primaryRingerItem;
    MGConfItem *m_secondaryRingerItem;

    QString m_incomingCallPath;
    MGConfItem *m_incomingCallPathItem;

    Dialog *m_dialog;

    QMediaPlayer *m_primaryPlayer;
    QMediaPlayer *m_secondaryPlayer;

    AlarmRequest *m_currentRequest;
    QList<AlarmRequest *> m_requestQueue;

    bool m_activeIncomingCall;
};

#endif // APPLICATION_H
