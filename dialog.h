/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef DIALOG_H
#define DIALOG_H

#include <QDeclarativeView>
#include <QUrl>
#include <QWidget>
#include <X11/X.h>

class Dialog : public QDeclarativeView
{
    Q_OBJECT
    Q_PROPERTY(int winId READ winId NOTIFY winIdChanged)
    Q_PROPERTY(int actualOrientation READ actualOrientation WRITE setActualOrientation)
    Q_PROPERTY(bool inhibitScreenSaver READ dummyInhibitScreenSaver WRITE dummySetInhibitScreenSaver)
public:
    explicit Dialog(QWidget * parent = 0);
    ~Dialog();

    int winId() const {
        return internalWinId();
    }

    // dummy implementation to make component library happy
    int actualOrientation() {
        return m_actualOrientation;
    }
    void setActualOrientation(int orientation) {
        m_actualOrientation = orientation;
    }

    // Just make the components library happy... the reality is we
    // always make each of these windows inhibit the screensaver
    int dummyInhibitScreenSaver() {
        return true;
    }
    void dummySetInhibitScreenSaver(bool) {}

signals:
    void winIdChanged();

    // Not really used, but needed to make the component library happy
    void vkbHeight(int x, int y, int width, int height);

protected:
    bool event(QEvent * event);

private:
    void excludeFromTaskBar();
    void changeNetWmState(bool set, Atom one, Atom two = 0);
    void setSkipAnimation();
    void inhibitScreenSaver();

    int m_actualOrientation;
};
#endif // DIALOG_H
