#include "RstRenderer.h"

#include <QVariant>
#include <QVariantMap>

namespace RstPad {

    RstRenderer::RstRenderer(PythonBridge *pythonBridge)
    {
        this->pythonBridge = pythonBridge;
        this->timer = new QTimer(this);
        this->timer->setSingleShot(true);

        connect(this->timer, &QTimer::timeout, this, &RstRenderer::timerTriggered);
    }

    RstRenderer::~RstRenderer()
    {
        if (thread) {
            thread->waitForFinished();
            delete thread;
        }
    }

    void RstRenderer::schedule(const QString &input)
    {
        if (delay > 0) {
            // render after a set delay
            pendingInput = input;
            timer->start(delay);
        } else {
            // try rendering right now
            if (!render(input)) {
                pendingInput = input;
            }
        }
    }

    void RstRenderer::reschedule()
    {
        if (delay > 0) {
            timer->start(delay);
        } else {
            if (render(pendingInput)) {
                clearPendingInput();
            }
        }
    }

    void RstRenderer::clearPendingInput()
    {
        pendingInput = QString();
    }

    void RstRenderer::setDelay(int msec)
    {
        delay = msec;
    }

    bool RstRenderer::render(const QString &input)
    {
        if (nullptr == thread) {
            thread = new RstRendererThread();

            connect(thread, &RstRendererThread::finished, this, &RstRenderer::threadFinished);
            connect(thread, &RstRendererThread::canceled, this, &RstRenderer::threadFinished);

            QFuture<QByteArray> future = QtConcurrent::run([this, input]() {
                QVariantMap settingsOverrides;
                settingsOverrides.insert("output_encoding", "unicode");

                QVariantList args;
                args.append(input); // source
                args.append(QVariant()); // source_path
                args.append(QVariant()); // destination_path
                args.append(QVariant()); // reader
                args.append("standalone"); // reader_name
                args.append(QVariant()); // parser
                args.append("restructuredtext"); // parser_name
                args.append(QVariant()); // writer
                args.append("html5"); // writer_name
                args.append(QVariant()); // settings
                args.append(QVariant()); // settings_spec
                args.append(settingsOverrides); // settings_overrides
                //args.append(QVariant()); // config_section
                //args.append(QVariant()); // enable_exit_status

                auto result = pythonBridge->callModuleFunction("docutils.core", "publish_string", args);

                if (!result.isValid() && pythonBridge->hasException()) {
                    return renderPythonException(pythonBridge->currentException()).toUtf8();
                }

                return result.toByteArray();
            });

            thread->setFuture(future);

            // started
            return true;
        }

        // busy
        return false;
    }

    QString RstRenderer::renderPythonException(PythonException *e)
    {
        QString document(R"document(<!doctype html>
<meta charset="utf-8">
<title>Renderer exception</title>
<div class="rst-exception">
)document");

        document.append(QString("<pre class=\"rst-exception-message\">%1: %2</pre>\n").arg(e->type().toHtmlEscaped(), e->message().toHtmlEscaped()));
        document.append(QString("<pre class=\"rst-exception-trace\">%1</pre>\n").arg(e->trace().toHtmlEscaped()));

        document.append("</div>\n");

        return document;
    }

    void RstRenderer::timerTriggered()
    {
        if (render(pendingInput)) {
            clearPendingInput();
        }
    }

    void RstRenderer::threadFinished()
    {
        // fetch data and emit event
        auto future = static_cast<RstRendererThread*>(sender());

        if (future->isFinished()) {
            emit ready(future->result());
        }

        // cleanup
        delete thread;
        thread = nullptr;

        // reschedule if there's pending input
        if (!pendingInput.isNull()) {
            reschedule();
        }
    }

}
