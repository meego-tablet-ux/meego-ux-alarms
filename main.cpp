/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "application.h"

#include <QStringList>
#include <QPluginLoader>

#include "atoms.h"

int main(int argc, char *argv[])
{
    // we never, ever want to be saddled with 'native' graphicssystem, as it is
    // amazingly slow. set us to 'raster'. this won't impact GL mode, as we are
    // explicitly using a QGLWidget viewport in those cases.
    QApplication::setGraphicsSystem("raster");

    Application app(argc, argv);

    foreach (QString path, QCoreApplication::libraryPaths())
    {
        QPluginLoader loader(path + "/libmultipointtouchplugin.so");
        loader.load();
        if (loader.isLoaded())
        {
            loader.instance();
            break;
        }
    }

    initAtoms();

    return app.exec();
}
