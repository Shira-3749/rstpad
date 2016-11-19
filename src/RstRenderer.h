#ifndef RSTPAD_RSTRENDERER_H
#define RSTPAD_RSTRENDERER_H

#include <QObject>
#include <QString>
#include <QtConcurrent/QtConcurrent>
#include <QTimer>
#include "PythonBridge.h"
#include "PythonException.h"

namespace RstPad {

    typedef QFutureWatcher<QByteArray> RstRendererThread;

    class RstRenderer : public QObject
    {
        Q_OBJECT

        public:
            explicit RstRenderer(PythonBridge *pythonBridge);
            ~RstRenderer();
            void schedule(const QString &input);
            void setDelay(int msec);

        signals:
            void ready(const QByteArray &result);

        private:
            PythonBridge *pythonBridge;
            RstRendererThread *thread = nullptr;
            QTimer *timer = nullptr;
            int delay = 0;
            QString pendingInput = QString();
            bool render(const QString &input);
            QString renderPythonException(PythonException *e);
            void reschedule();
            void clearPendingInput();

        private slots:
            void timerTriggered();
            void threadFinished();
    };

}

#endif // RSTPAD_RSTRENDERER_H
