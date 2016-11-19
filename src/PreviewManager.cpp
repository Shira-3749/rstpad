#include "PreviewManager.h"
#include <QString>
#include <QUrl>
#include <QtMath>
#include <QVariant>

namespace RstPad {

    PreviewManager::PreviewManager(PreviewView *preview, CodeEdit *codeEditor)
    {
        this->preview = preview;
        this->codeEditor = codeEditor;
        this->unfreezeTimer = new QTimer(this);
        this->unfreezeTimer->setSingleShot(true);

        connect(this->preview, &PreviewView::loadStarted, this, &PreviewManager::loadStarted);
        connect(this->preview, &PreviewView::loadFinished, this, &PreviewManager::loadFinished);
        connect(this->unfreezeTimer, &QTimer::timeout, this, &PreviewManager::unfreezeTimerTriggered);
    }

    PreviewManager::~PreviewManager()
    {
    }

    void PreviewManager::refresh()
    {
        QUrl url("app://local/current-rst-output");

        lastScrollPosition = preview->page()->scrollPosition().y();
        preview->freeze();
        preview->load(url);
    }

    void PreviewManager::setAutoscrollMode(AutoscrollMode mode)
    {
        autoscrollMode = mode;
        autoscroll();
    }

    void PreviewManager::setAutoscrollWaitDelay(int msec)
    {
        autoscrollWaitDelay = msec;
    }

    void PreviewManager::printToPdf(const QString &filePath)
    {
        preview->page()->printToPdf(filePath);
    }

    void PreviewManager::autoscroll()
    {
        if (preview->url().isEmpty()) {
            return;
        }

        switch (autoscrollMode) {
            case AutoscrollMode::FirstLine:
                autoscrollToLine(1 + codeEditor->firstVisibleBlock().blockNumber());
                break;
            case AutoscrollMode::CurrentLine:
                autoscrollToLine(1 + codeEditor->textCursor().blockNumber());
                break;
            case AutoscrollMode::Disabled:
                autoscrollToPosition(lastScrollPosition);
                break;
        }
    }

    void PreviewManager::autoscrollToLine(int line)
    {
        if (lastAutoscrollLine != line) {
            lastAutoscrollLine = line;

            preview->page()->runJavaScript(
                QString::asprintf("typeof RstPadApi !== 'undefined' ? (RstPadApi.scrollToLine(%d), true) : false", line),
                [this](const QVariant &v) { this->autoscrollFinished(v.toBool()); }
            );
        }
    }

    void PreviewManager::autoscrollToPosition(qreal position)
    {
        if (qFabs(lastAutoscrollPosition - position) > 1) {
            lastAutoscrollPosition = position;

            preview->page()->runJavaScript(
                QString::asprintf("typeof RstPadApi !== 'undefined' ? (RstPadApi.scrollToPosition(%f), true) : false", lastScrollPosition),
                [this](const QVariant &v) { this->autoscrollFinished(v.toBool()); }
            );
        }
    }

    void PreviewManager::autoscrollFinished(bool ok)
    {
        if (!ok) {
            resetAutoscrollState();
        }

        unfreezeTimer->start(autoscrollWaitDelay);
    }

    void PreviewManager::resetAutoscrollState()
    {
        lastAutoscrollLine = 0;
        lastAutoscrollPosition = 0.0;
        unfreezeTimer->stop();
    }

    void PreviewManager::loadStarted()
    {
        resetAutoscrollState();
    }

    void PreviewManager::loadFinished(bool ok)
    {
        if (ok) {
            autoscroll();
        } else {
            preview->unfreeze();
        }
    }

    void PreviewManager::unfreezeTimerTriggered()
    {
        preview->unfreeze();
    }

}
