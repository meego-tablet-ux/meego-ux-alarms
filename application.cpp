/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QDebug>
#include <QDBusConnection>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QFile>
#include <QLibraryInfo>
#include <QSettings>
#include <QTimer>
#include <QUrl>
#include <MGConfItem>

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
    m_activeIncomingCall(false)
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

    new AlarmControl(this);
    QDBusConnection::sessionBus().registerService("org.meego.alarms");
    QDBusConnection::sessionBus().registerObject("/incomingCall", this);
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
    qDebug() << "XXX incomingCall" << m_dialog;

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
                                        rejectAction, imageURI, sound,
                                        AlarmRequest::IncomingCall);
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
            m_secondaryPlayer->play();
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
    qDebug() << "XXX dropCall" << m_dialog;
    if (!m_currentRequest ||
            m_currentRequest->getType() != AlarmRequest::IncomingCall)
        return;
    cleanupDialog();
}

void Application::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "mediaStatuschanged" << status;
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

void Application::triggerAction(QString action)
{
    if (!action.isEmpty())
    {
        // call it
    }
}

void Application::cancelAlarm()
{
    if (!m_currentRequest ||
            m_currentRequest->getType() != AlarmRequest::AlarmClock)
        return;

    // TODO: Use data from m_currentRequest to make a request
    //       via libealarm to cancel/stop the alarm

    cleanupDialog();
}

void Application::cleanupDialog(bool loadNextRequest)
{
    qDebug() << "XXX cleanupDialog" << m_dialog;
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

    delete m_currentRequest;
    m_currentRequest = NULL;

    if (loadNextRequest && !m_requestQueue.isEmpty())
        enqueue(m_requestQueue.takeFirst());
}

void Application::enqueue(AlarmRequest *request)
{
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
        m_dialog->setSource(QUrl::fromLocalFile("/usr/share/meego-ux-alarms/taskreminder.qml"));
        break;
    case AlarmRequest::EventReminder:
        m_dialog->setSource(QUrl::fromLocalFile("/usr/share/meego-ux-alarms/eventreminder.qml"));
        break;
    case AlarmRequest::AlarmClock:
        m_dialog->setSource(QUrl::fromLocalFile("/usr/share/meego-ux-alarms/alarmclock.qml"));
        break;
    case AlarmRequest::IncomingCall:
        m_dialog->setSource(QUrl::fromLocalFile("/usr/share/meego-ux-alarms/incomingcall.qml"));
        break;
    }
    m_dialog->show();

    if (!m_currentRequest->getSound().isEmpty())
    {
        m_primaryPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
        connect(m_primaryPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), SLOT(mediaStatusChanged(QMediaPlayer::MediaStatus)));
        m_primaryPlayer->setMedia(QUrl::fromLocalFile(m_currentRequest->getSound()));
        m_primaryPlayer->play();
    }
}
