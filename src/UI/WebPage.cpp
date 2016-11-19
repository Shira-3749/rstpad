#include "WebPage.h"
#include <QDesktopServices>

namespace RstPad {

    bool WebPage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
    {
        if (!isMainFrame) {
            return false;
        }

        if (NavigationType::NavigationTypeTyped != type && NavigationType::NavigationTypeReload != type) {
            QDesktopServices::openUrl(url);

            return false;
        }

        return true;
    }

}
