#ifndef RSTPAD_WEBPAGE_H
#define RSTPAD_WEBPAGE_H

#include <QWebEnginePage>
#include <QUrl>

namespace RstPad {

    class WebPage : public QWebEnginePage
    {
        using QWebEnginePage::QWebEnginePage;

        public:
            bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame);
    };

}

#endif // RSTPAD_WEBPAGE_H
