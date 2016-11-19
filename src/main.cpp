#include <QApplication>
#include <QtGlobal>
#include "App.h"

int main(int argc, char *argv[])
{
    #if defined(QT_DEBUG) && defined(DEBUG_WEBENGINE_REMOTE_PORT)
        if (DEBUG_WEBENGINE_REMOTE_PORT > 0 && !qEnvironmentVariableIsSet("QTWEBENGINE_REMOTE_DEBUGGING")) {
            qputenv("QTWEBENGINE_REMOTE_DEBUGGING", QString::number(DEBUG_WEBENGINE_REMOTE_PORT).toLocal8Bit());
        }
    #endif

    RstPad::App app(argc, argv);

    return app.exec();
}
