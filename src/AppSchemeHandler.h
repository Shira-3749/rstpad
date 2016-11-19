#ifndef RSTPAD_APPSCHEMEHANDLER_H
#define RSTPAD_APPSCHEMEHANDLER_H

#include "App.h"
#include <QWebEngineUrlSchemeHandler>
#include <QWebEngineUrlRequestJob>

namespace RstPad {

    class AppSchemeHandler : public QWebEngineUrlSchemeHandler
    {
        public:
            AppSchemeHandler();
            void requestStarted(QWebEngineUrlRequestJob* request);

        private:
            void handleCurrentRstOutputRequest(QWebEngineUrlRequestJob* request);
            void handleLocalFileRequest(QWebEngineUrlRequestJob* request);
    };

}

#endif // RSTPAD_APPSCHEMEHANDLER_H
