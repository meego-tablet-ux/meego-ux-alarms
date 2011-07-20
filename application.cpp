/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QFile>
#include <QLibraryInfo>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <MGConfItem>
#include <mremoteaction.h>

#include "application.h"
#include "dialog.h"
#include "alarmcontrol.h"

Application::Application(int & argc, char ** argv) :
    QApplication(argc, argv),
    m_orientation(1),
    m_orientationSensorAvailable(false),
    m_dialog(NULL),
    m_primaryPlayer(NULL),
    m_secondaryPlayer(NULL),
    m_currentRequest(NULL),
    m_activeIncomingCall(false),
    m_soundsToPlay(0),
    m_hasAcquiredResources(false)
{
    setApplicationName("meego-ux-alarms");

    setQuitOnLastWindowClosed(false);

    if (QSensor::sensorsForType("QOrientationSensor").length() > 0)
    {
        m_orientationSensorAvailable = true;
        connect(&m_orientationSensor, SIGNAL(readingChanged()), SLOT(updateOrientation()));
    }

    QString theme = MGConfItem("/meego/ux/theme").value().toString();
    QString themeFile = QString("/usr/share/themes/") + theme + "/theme.ini";
    if(!QFile::exists(themeFile))
    {
        // fallback
        themeFile = QString("/usr/share/themes/1024-600-10/theme.ini");
    }
    themeConfig = new QSettings(themeFile, QSettings::NativeFormat, this);

    setFont(QFont(themeConfig->value("fontFamily").toString(), themeConfig->value("fontPixelSizeMedium").toInt()));

    m_translator.load("qt_meego-ux-alarms.qm", QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    installTranslator(&m_translator);

    m_secondaryRingerItem = new MGConfItem("/meego/chat/short-videocall-sound", this);
    connect(m_secondaryRingerItem, SIGNAL(valueChanged()), SLOT(updateSecondaryRinger()));
    updateSecondaryRinger();

    m_incomingCallPathItem = new MGConfItem("/meego/ux/IncomingCallPath", this);
    connect(m_incomingCallPathItem, SIGNAL(valueChanged()), SLOT(updateIncomingCallPath()));
    updateIncomingCallPath();

    m_alarmClockPathItem = new MGConfItem("/meego/ux/AlarmClockPath", this);
    connect(m_alarmClockPathItem, SIGNAL(valueChanged()), SLOT(updateAlarmClockPath()));
    updateAlarmClockPath();

    m_taskReminderPathItem = new MGConfItem("/meego/ux/TaskReminderPath", this);
    m_taskReminderSoundPathItem = new MGConfItem("/meego/ux/TaskReminderSoundPath", this);
    connect(m_taskReminderPathItem, SIGNAL(valueChanged()), SLOT(updateTaskReminderPath()));
    connect(m_taskReminderSoundPathItem, SIGNAL(valueChanged()), SLOT(updateTaskReminderPath()));
    updateTaskReminderPath();

    m_eventReminderPathItem = new MGConfItem("/meego/ux/EventReminderPath", this);
    m_eventReminderSoundPathItem = new MGConfItem("/meego/ux/EventReminderSoundPath", this);
    connect(m_eventReminderPathItem, SIGNAL(valueChanged()), SLOT(updateEventReminderPath()));    
    connect(m_eventReminderSoundPathItem, SIGNAL(valueChanged()), SLOT(updateEventReminderPath()));
    updateEventReminderPath();

    new AlarmControl(this);
    QDBusConnection::sessionBus().registerService("org.meego.alarms");
    QDBusConnection::sessionBus().registerObject("/incomingCall", this);

    m_alarmResourceSet = new ResourcePolicy::ResourceSet("alarm", this);
    m_alarmResourceSet->setAlwaysReply();

    m_alarmAudioResource = new ResourcePolicy::AudioResource("alarm");
    m_alarmAudioResource->setProcessID(QCoreApplication::applicationPid());
    m_alarmAudioResource->setStreamTag("media.name", "*");
    
    m_alarmResourceSet->addResourceObject(m_alarmAudioResource);

    g_type_init();

    m_notifyItem = alarm_notify_new();

    g_signal_connect (m_notifyItem, "alarm", G_CALLBACK (getAlarm_cb), this);

    connect (this, SIGNAL (newAlarmRequest(QString, QString, QString, QString, QString, QUrl, int, QString, bool, ECalComponent *)),
             this, SLOT (handleNewAlarmRequest(QString, QString, QString, QString, QString, QUrl, int, QString, bool, ECalComponent *) ));

    connect (m_alarmResourceSet, SIGNAL (resourcesGranted(const QList<ResourcePolicy::ResourceType>&)), this,
             SLOT(audioAcquiredHandler()));

    connect (m_alarmResourceSet, SIGNAL (lostResources()), this,
             SLOT (audioLostHandler()));

    connect (m_alarmResourceSet, SIGNAL (resourcesReleased()), this,
             SLOT (audioReleasedHandler()));

    connect(m_alarmResourceSet,
            SIGNAL(resourcesDenied()), this,
            SLOT(audioDeniedHandler()));

}

Application::~Application()
{
}

void Application::updateOrientation()
{
    switch (m_orientationSensor.reading()->orientation())
    {
    case QOrientationReading::LeftUp:
        m_orientation = 2;
        break;
    case QOrientationReading::TopDown:
        m_orientation = 3;
        break;
    case QOrientationReading::RightUp:
        m_orientation = 0;
        break;
    case QOrientationReading::TopUp:
        m_orientation = 1;
        break;
    default:
        return;
    }

    emit orientationChanged();
}

int Application::foregroundWindow()
{
    // By definition our window is the foreground window
    if (m_dialog)
    {
        return m_dialog->winId();
    }

    return 0;
}

void Application::incomingCall(QString summary,
                               QString body,
                               QString acceptAction,
                               QString rejectAction,
                               QString sound,
                               QString imageURI)
{
    if (m_currentRequest)
    {
        if (m_currentRequest->getType() == AlarmRequest::IncomingCall)
        {
            // This shouldn't happen since we would expect a dropCall
            // before getting another request, so log about it and assume
            // this new request is the currect request
            qCritical("Recieved two incoming calls on top of each other!");
        }
        else
        {
            // incoming calls always take priority
            m_requestQueue.push_front(m_currentRequest);
        }
        m_currentRequest = NULL;
        cleanupDialog(false);
    }

    m_currentRequest = new AlarmRequest(summary, body, acceptAction,
                                        rejectAction, imageURI, QUrl::fromLocalFile(sound),
                                        AlarmRequest::IncomingCall, e_cal_component_gen_uid(), false);
    showCurrentRequest();

    // If any other types of notification are waiting in the queue,
    // and if the platform has a secondary ringer configured, then
    // play the secondary ringer at the same time
    if (!m_requestQueue.isEmpty())
    {
        if (!m_secondaryRinger.isNull() &&
           (!m_secondaryPlayer ||
            m_secondaryPlayer->state() != QMediaPlayer::PlayingState))
        {
            m_secondaryPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
            connect(m_secondaryPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
            m_secondaryPlayer->setMedia(QUrl::fromLocalFile(m_secondaryRinger));
            playSound();
        }
    }
}

void Application::updateSecondaryRinger()
{
    if (m_secondaryRingerItem->value().isValid())
    {
        m_secondaryRinger = m_secondaryRingerItem->value().toString();
    }
}

void Application::dropCall()
{
    if (!m_currentRequest ||
        m_currentRequest->getType() != AlarmRequest::IncomingCall)
        return;
    cleanupDialog();
}

void Application::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia)
    {
        QMediaPlayer *p = static_cast<QMediaPlayer *>(sender());
        p->play();
    }
}

void Application::updateIncomingCallPath()
{
    if (!m_incomingCallPathItem->value().isValid())
    {
        m_incomingCallPath = "/usr/share/meego-ux-alarms/incomingcall.qml";
    }
    else
    {
        m_incomingCallPath = m_incomingCallPathItem->value().toString();
    }
}

void Application::updateAlarmClockPath()
{
    if (!m_alarmClockPathItem->value().isValid())
    {
        m_alarmClockPath = "/usr/share/meego-ux-alarms/alarmclock.qml";
    }
    else
    {
        m_alarmClockPath = m_alarmClockPathItem->value().toString();
    }
}

void Application::updateEventReminderPath()
{
    if (!m_eventReminderPathItem->value().isValid())
    {
        m_eventReminderPath = "/usr/share/meego-ux-alarms/eventreminder.qml";
    }
    else
    {
        m_eventReminderPath = m_eventReminderPathItem->value().toString();
    }

    if (!m_eventReminderSoundPathItem->value().isValid())
    {
        m_eventReminderSoundPath = "/usr/share/sounds/meego/stereo/alert-1.wav";
    }
    else
    {
        m_eventReminderSoundPath = m_eventReminderSoundPathItem->value().toString();
    }

}

void Application::updateTaskReminderPath()
{
    if (!m_taskReminderPathItem->value().isValid())
    {
        m_taskReminderPath = "/usr/share/meego-ux-alarms/taskreminder.qml";
    }
    else
    {
        m_taskReminderPath = m_taskReminderPathItem->value().toString();
    }

    if (!m_taskReminderSoundPathItem->value().isValid())
    {
        m_taskReminderSoundPath = "/usr/share/sounds/meego/stereo/alert-1.wav";
    }
    else
    {
        m_taskReminderSoundPath = m_taskReminderSoundPathItem->value().toString();
    }
}

void Application::triggerAction(QString action)
{
    if (!action.isEmpty())
    {
        MRemoteAction callAction(action);
        callAction.trigger();
    }
}

void Application::cancelAlarm()
{
    if (!m_currentRequest || m_currentRequest->getData() == NULL)
        return;

    e_cal_component_remove_all_alarms(m_currentRequest->getData());

    // TODO: Use data from m_currentRequest to make a request
    //       via libealarm to cancel/stop the alarm

    cleanupDialog();
}

void Application::cleanupDialog(bool loadNextRequest)
{

    if (m_dialog)
    {
        // For some reason, at least on 964 and 945 graphics, repeatedly
        // opening and closing a gl window will segfault deep down in egl
        // code.  A work around is to not completely free the window till
        // we instantiate a new window, so in this call we just close the
        // dialog, and only delete this instance once we have a request
        // for a new dialog.
        m_dialog->close();
    }

    if (m_primaryPlayer)
    {
        m_primaryPlayer->stop();
        delete m_primaryPlayer;
        m_primaryPlayer = NULL;
    }
    if (m_secondaryPlayer)
    {
        m_secondaryPlayer->stop();
        delete m_secondaryPlayer;
        m_secondaryPlayer = NULL;
    }

    if (m_currentRequest != NULL && (!m_currentRequest->getSound().isEmpty() || m_requestQueue.isEmpty()))
    {
        m_soundsToPlay--;
        //No more sounds to play, release the resource
        if (m_soundsToPlay <= 0 || m_requestQueue.isEmpty())
        {
            m_alarmResourceSet->release();
            m_soundsToPlay = 0;
        }
    }

    delete m_currentRequest;
    m_currentRequest = NULL;

    if (loadNextRequest && !m_requestQueue.isEmpty())
        enqueue(m_requestQueue.takeFirst());
}

void Application::handleNewAlarmRequest(QString summary, QString body, QString acceptAction, QString rejectAction, QString imageUri, QUrl sound, int type, QString uid, bool snooze, ECalComponent *data)
{        
    bool eventNotInQueue = true;
    GList *alarmList = e_cal_component_get_alarm_uids(data);

    if (g_list_nth_data (alarmList, 0) != NULL)
    {
        if (m_currentRequest && (m_currentRequest->getUid() == uid))
            eventNotInQueue = false;

        for (int i=0; i < m_requestQueue.size(); i++)
        {
            if (m_requestQueue.at(i)->getUid() == uid)
                eventNotInQueue = false;
        }

        if (eventNotInQueue)
        {
            //Tasks and Events don't let the user pick the sound, assign the default
            if (type == AlarmRequest::EventReminder)
                sound = QUrl::fromLocalFile(m_eventReminderSoundPath);
            else if (type == AlarmRequest::TaskReminder)
                sound = QUrl::fromLocalFile(m_taskReminderSoundPath);

            AlarmRequest *incomingRequest = new AlarmRequest(summary, body, acceptAction, rejectAction, imageUri, sound, type, uid, snooze, data);
            enqueue(incomingRequest);
        }
    }
}

//Callback function for getting alarm info from libealarm
void Application::getAlarm_cb(AlarmNotify *notify, ECalComponent *data, Application* appInstance)
{
    ECalComponentAlarmRepeat alarmRepeat;

    const gchar* eventUid;
    e_cal_component_get_uid(data, &eventUid);

    ECalComponentText summaryTxt;
    e_cal_component_get_summary (data, &summaryTxt);

    QString summary = summaryTxt.value;
    QString body = summaryTxt.value;
    QString acceptAction;
    QString rejectAction;
    QString imageUri;
    QUrl sound;
    bool snooze = false;

    int type = e_cal_component_get_vtype (data);

    GList *alarmList = e_cal_component_get_alarm_uids(data);  

    if (g_list_nth_data (alarmList, 0) != NULL)
    {
        const gchar* alarmItem = reinterpret_cast<const gchar*>(g_list_nth_data(alarmList,0));

        ECalComponentAlarm *alarm = e_cal_component_get_alarm(data, alarmItem);
        ECalComponentAlarmAction alarmAction;

        e_cal_component_alarm_get_action(alarm, &alarmAction);

        e_cal_component_alarm_get_repeat(alarm, &alarmRepeat);

        if (alarmRepeat.duration.weeks > 0 || alarmRepeat.duration.days >0 || alarmRepeat.duration.hours > 0 || alarmRepeat.duration.minutes > 0)
            snooze = true;

        icalattach *alarmAttachment;
        e_cal_component_alarm_get_attach(alarm, &alarmAttachment);

        if (alarmAction == E_CAL_COMPONENT_ALARM_AUDIO)
        {
            sound = icalattach_get_url(alarmAttachment);

            //If sound file not found use default
            if (!sound.isValid())
            {
                sound = QUrl::fromLocalFile("/usr/share/sounds/meego/stereo/alert-1.wav");
            }

            type = AlarmRequest::AlarmClock;
        }
    }

    cal_obj_uid_list_free(alarmList);
    appInstance->newAlarmRequest(summary, body, acceptAction, rejectAction, imageUri, sound, type, eventUid, snooze, data);
}

void Application::enqueue(AlarmRequest *request)
{
    if (!request->getSound().isEmpty())
        m_soundsToPlay++;

    if (m_currentRequest)
    {
        m_requestQueue << request;
        return;
    }

    // No active notification so show this one
    m_currentRequest = request;
    showCurrentRequest();
}

void Application::showCurrentRequest()
{
    if (m_dialog)
    {
        m_dialog->deleteLater();
    }

    m_dialog = new Dialog();
    connect(m_dialog->engine(), SIGNAL(quit()), SLOT(cleanupDialog()));
    m_dialog->rootContext()->setContextProperty("currentRequest", m_currentRequest);
    switch(m_currentRequest->getType())
    {
    case AlarmRequest::TaskReminder:
        m_dialog->setSource(QUrl::fromLocalFile(m_taskReminderPath));
        break;
    case AlarmRequest::EventReminder:
        m_dialog->setSource(QUrl::fromLocalFile(m_eventReminderPath));
        break;
    case AlarmRequest::AlarmClock:
        m_dialog->setSource(QUrl::fromLocalFile(m_alarmClockPath));
        break;
    case AlarmRequest::IncomingCall:
        m_dialog->setSource(QUrl::fromLocalFile(m_incomingCallPath));
        break;
    }

    if (!m_currentRequest->getSound().isEmpty())
    {
        m_primaryPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
        connect(m_primaryPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
        m_primaryPlayer->setMedia(m_currentRequest->getSound());
        playSound();
    }

    m_dialog->show();
}

void Application::playSound()
{
    if (m_hasAcquiredResources != true)
        m_alarmResourceSet->acquire();
    else
        audioAcquiredHandler();
}

void Application::audioAcquiredHandler()
{
    m_hasAcquiredResources = true;

    if (m_primaryPlayer && m_primaryPlayer->state() != QMediaPlayer::PlayingState)
        m_primaryPlayer->play();

    if (m_secondaryPlayer && m_secondaryPlayer->state() != QMediaPlayer::PlayingState)
        m_secondaryPlayer->play();
}

void Application::audioLostHandler()
{
    m_hasAcquiredResources = false;

    if (m_primaryPlayer && m_primaryPlayer->state() == QMediaPlayer::PlayingState)
    {
        m_primaryPlayer->pause();
    }
    if (m_secondaryPlayer && m_secondaryPlayer->state() == QMediaPlayer::PlayingState)
    {
        m_secondaryPlayer->pause();
    }
}

void Application::audioReleasedHandler()
{
    m_hasAcquiredResources = false;
}

void Application::audioDeniedHandler()
{
    m_hasAcquiredResources = false;
}

void Application::launchDesktopByName(QString name, QString cmd, QString cdata)
{
    QString service = "com.lockstatus";
    QString object = "/query";
    QString interface = "com.lockstatus.query";
    QDBusInterface launcher(service, object, interface);
    if (launcher.isValid())
    {
        launcher.asyncCall(QLatin1String("launchDesktopByName"), name, cmd, cdata);
    }
}
