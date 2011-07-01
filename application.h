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
#include <policy/resource-set.h>

#include <QtCore/QtCore>
#include <QtCore/QObject>

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
    void newAlarmRequest(QString summary, QString body, QString acceptAction, QString rejectAction, QString imageUri, QUrl sound, int type, QString uid, bool, ECalComponent *);

public slots:
    void cleanupDialog(bool loadNextRequest = true);
    void dropCall();
    void triggerAction(QString action);
    void cancelAlarm();
    void launchDesktopByName(QString name, QString cmd = QString(), QString cdata = QString());

private slots:
    void updateOrientation();
    void playSound();
    void updateSecondaryRinger();
    void updateIncomingCallPath();
    void mediaStatusChanged(QMediaPlayer::MediaStatus);
    void enqueue(AlarmRequest *);
    void audioAcquiredHandler();
    void audioLostHandler();
    void audioReleasedHandler();
    void audioDeniedHandler();
    void handleNewAlarmRequest(QString summary, QString body, QString acceptAction, QString rejectAction, QString imageUri, QUrl sound, int type, QString uid, bool, ECalComponent * data);

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

    ResourcePolicy::ResourceSet *m_alarmResourceSet;
    ResourcePolicy::AudioResource *m_alarmAudioResource;
    int m_soundsToPlay;

    bool m_hasAcquiredResources;

    static void getAlarm_cb(AlarmNotify *notify, ECalComponent *data, Application* appInstance);

    AlarmNotify *m_notifyItem;
};

#endif // APPLICATION_H
