#ifndef RSTPAD_PREVIEWVIEW_H
#define RSTPAD_PREVIEWVIEW_H

#include <QWebEngineView>
#include <QPaintEvent>
#include <QEvent>
#include <QOpenGLWidget>
#include <QPixmap>

namespace RstPad {

    class PreviewView : public QWebEngineView
    {
        using QWebEngineView::QWebEngineView;

        public:
            void freeze();
            void unfreeze(bool update = true);
            ~PreviewView();

        protected:
            bool event(QEvent *event);
            bool eventFilter(QObject *watched, QEvent *event);

        private:
            bool frozen = false;
            QOpenGLWidget *hostView;
            QPixmap *capture = nullptr;
    };

}

#endif // RSTPAD_PREVIEWVIEW_H
