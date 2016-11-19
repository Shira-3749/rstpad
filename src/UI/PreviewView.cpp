#include "PreviewView.h"
#include <QPainter>
#include <QColor>

namespace RstPad {

    PreviewView::~PreviewView()
    {
        delete capture;
    }

    void PreviewView::freeze()
    {
        if (!frozen && hostView) {
            // create / recreate capture pixmap if dimensions have changed
            if (!capture || (capture->width() != hostView->width() || capture->height() != hostView->height())) {
                delete capture;
                capture = new QPixmap(hostView->width(), hostView->height());
            }

            // render the host view into the capture pixmap
            hostView->render(capture);

            // freeze
            frozen = true;
        }
    }

    void PreviewView::unfreeze(bool update)
    {
        if (frozen) {
            frozen = false;

            if (update) {
                hostView->update();
            }
        }
    }

    bool PreviewView::event(QEvent *event)
    {
        // capture the host view when it's added as a child
        if (event->type() == QEvent::ChildAdded) {
            auto childEvent = static_cast<QChildEvent*>(event);
            auto openGLWidget = qobject_cast<QOpenGLWidget*>(childEvent->child());

            if (openGLWidget) {
                openGLWidget->installEventFilter(this);
                hostView = openGLWidget;
            }
        }

        return QWebEngineView::event(event);
    }

    bool PreviewView::eventFilter(QObject *watched, QEvent *event)
    {
        if (event->type() == QEvent::Paint) {
            auto widget = dynamic_cast<QWidget*>(watched);

            if (frozen) {
                watched->event(event);

                QPainter painter(widget);
                painter.drawPixmap(0, 0, *capture);

                //painter.setPen(QColor("red"));
                //painter.drawLine(0, 0, widget->width() - 1, widget->height() - 1);
                //painter.drawLine(widget->width() - 1, 0, 0, widget->height() - 1);

                return true;
            }
        }

        return false;
    }

}
