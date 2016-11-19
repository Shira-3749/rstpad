#ifndef RSTPAD_SEARCHREPLACEHANDLER_H
#define RSTPAD_SEARCHREPLACEHANDLER_H

#include <QObject>
#include <QWidget>
#include <QPlainTextEdit>
#include <QTextDocument>
#include "SearchReplaceDlg.h"
#include "SearchJob.h"

namespace RstPad {

    class SearchReplaceHandler : public QObject
    {
        Q_OBJECT

        public:
            SearchReplaceHandler(QWidget *parent, QPlainTextEdit *edit);
            void activate();
            void redoLastSearch(bool backwards = false);
            void setWrapAround(bool wrapAround);

        public slots:
            void handleJob(const SearchJob &job);

        signals:
            void wrappedAround();
            void matchFailed();

        private:
            QPlainTextEdit *edit;
            SearchReplaceDlg *dlg;
            SearchJob lastSearchJob = SearchJob();
            bool wrapAround = false;
            bool tryWrapAround(const SearchJob &job, const QRegExp &regexp,  QTextDocument::FindFlags options);
            bool handleReplace(const SearchJob &job, const QRegExp &regexp,  QTextDocument::FindFlags options);
            int handleReplaceAll(const SearchJob &job, const QRegExp &regexp,  QTextDocument::FindFlags options);
            int countOccurences(const QRegExp &regexp,  QTextDocument::FindFlags options);
            void setLastSearchJob(const SearchJob &job);
            void showInfoDialog(const QString &text);
    };

}

#endif // RSTPAD_SEARCHREPLACEHANDLER_H
