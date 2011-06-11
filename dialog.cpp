/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "application.h"
#include "dialog.h"

#include <MGConfItem>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QDesktopWidget>
#include <QDebug>
#include <QGLFormat>
#include <QGLWidget>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <unistd.h>

#include "atoms.h"

Dialog::Dialog(QWidget * parent) : QDeclarativeView(parent)
{
    Application *app = static_cast<Application *>(qApp);

    int screenWidth = qApp->desktop()->rect().width();
    int screenHeight = qApp->desktop()->rect().height();
    setSceneRect(0, 0, screenWidth, screenHeight);

    rootContext()->setContextProperty("screenWidth", screenWidth);
    rootContext()->setContextProperty("screenHeight", screenHeight);
    rootContext()->setContextProperty("qApp", app);
    rootContext()->setContextProperty("mainWindow", this);
    rootContext()->setContextProperty("theme_name", MGConfItem("/meego/ux/theme").value().toString());
    foreach (QString key, app->themeConfig->allKeys())
    {
        if (key.contains("Size") || key.contains("Padding") ||
            key.contains("Width") ||key.contains("Height") ||
            key.contains("Margin") || key.contains("Thickness"))
        {
            rootContext()->setContextProperty("theme_" + key, app->themeConfig->value(key).toInt());
        }
        else if (key.contains("Opacity"))
        {
            rootContext()->setContextProperty("theme_" + key, app->themeConfig->value(key).toDouble());
        }
        else
        {
            rootContext()->setContextProperty("theme_" + key, app->themeConfig->value(key));
        }
    }

    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QGLFormat format = QGLFormat::defaultFormat();
    format.setSampleBuffers(false);
    format.setAlpha(true);
    setViewport(new QGLWidget(format));

    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    viewport()->setAttribute(Qt::WA_OpaquePaintEvent);
    viewport()->setAttribute(Qt::WA_NoSystemBackground);
    viewport()->setAttribute(Qt::WA_TranslucentBackground);
    
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);

    setSkipAnimation();
    inhibitScreenSaver();

    qDebug() << "XXX Dialog::Dialog" << this;
}

Dialog::~Dialog()
{
    qDebug() << "XXX ~Dialog::Dialog" << this;
}

bool Dialog::event (QEvent * event)
{
    if (event->type() == QEvent::Show)
    {
        Atom stackingAtom = getAtom(ATOM_MEEGO_STACKING_LAYER);
        long layer = 2;
        XChangeProperty(QX11Info::display(), internalWinId(), stackingAtom, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&layer, 1);

        excludeFromTaskBar();
    }
    return QDeclarativeView::event(event);
}

void Dialog::setSkipAnimation()
{
    Atom miniAtom = getAtom(ATOM_MEEGOTOUCH_SKIP_ANIMATIONS);
    long min = 1;
    XChangeProperty(QX11Info::display(), internalWinId(), miniAtom, XA_CARDINAL, 32, PropModeReplace, (unsigned char*)&min, 1);
}

void Dialog::excludeFromTaskBar()
{
    // Tell the window to not to be shown in the switcher
    Atom skipTaskbarAtom = getAtom(ATOM_NET_WM_STATE_SKIP_TASKBAR);
    changeNetWmState(true, skipTaskbarAtom);

    // Also set the _NET_WM_STATE window property to ensure Home doesn't try to
    // manage this window in case the window manager fails to set the property in time
    Atom netWmStateAtom = getAtom(ATOM_NET_WM_STATE);
    QVector<Atom> atoms;
    atoms.append(skipTaskbarAtom);
    XChangeProperty(QX11Info::display(), internalWinId(), netWmStateAtom, XA_ATOM, 32, PropModeReplace, (unsigned char *)atoms.data(), atoms.count());
}

void Dialog::changeNetWmState(bool set, Atom one, Atom two)
{
    XEvent e;
    e.xclient.type = ClientMessage;
    Display *display = QX11Info::display();
    Atom netWmStateAtom = getAtom(ATOM_NET_WM_STATE);
    e.xclient.message_type = netWmStateAtom;
    e.xclient.display = display;
    e.xclient.window = internalWinId();
    e.xclient.format = 32;
    e.xclient.data.l[0] = set ? 1 : 0;
    e.xclient.data.l[1] = one;
    e.xclient.data.l[2] = two;
    e.xclient.data.l[3] = 0;
    e.xclient.data.l[4] = 0;
    XSendEvent(display, RootWindow(display, x11Info().screen()), FALSE, (SubstructureNotifyMask | SubstructureRedirectMask), &e);
    XSync(display, FALSE);
}

void Dialog::inhibitScreenSaver()
{
    bool inhibit = true;

    Atom inhibitAtom = getAtom(ATOM_MEEGO_INHIBIT_SCREENSAVER);
    XChangeProperty(QX11Info::display(), winId(), inhibitAtom, XA_CARDINAL, 32,
                    PropModeReplace, (unsigned char*)&inhibit, 1);
}
