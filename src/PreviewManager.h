#ifndef RSTPAD_PREVIEWMANAGER_H
#define RSTPAD_PREVIEWMANAGER_H

#include "App.h"
#include <QObject>
#include <QTimer>
#include <QString>
#include "UI/WebPage.h"
#include "UI/PreviewView.h"
#include "UI/Code/CodeEdit.h"

namespace RstPad {

    enum class AutoscrollMode;

    class PreviewManager : public QObject
    {
        Q_OBJECT

        public:
            explicit PreviewManager(PreviewView *preview, CodeEdit *codeEditor);
            ~PreviewManager();
            void refresh();
            void autoscroll();
            void setAutoscrollMode(AutoscrollMode mode);
            void setAutoscrollWaitDelay(int msec);
            void printToPdf(const QString &filePath);

        private:
            PreviewView *preview;
            CodeEdit *codeEditor;
            AutoscrollMode autoscrollMode;
            qreal lastScrollPosition = 0.0;
            int lastAutoscrollLine = 0;
            qreal lastAutoscrollPosition = 0;
            QTimer *unfreezeTimer = nullptr;
            int autoscrollWaitDelay = 500;
            void autoscrollToLine(int line);
            void autoscrollToPosition(qreal position);
            void autoscrollFinished(bool ok);
            void resetAutoscrollState();

        private slots:
            void loadStarted();
            void loadFinished(bool ok);
            void unfreezeTimerTriggered();
    };

}

#endif // RSTPAD_PREVIEWMANAGER_H
