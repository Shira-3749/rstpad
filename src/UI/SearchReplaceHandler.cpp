#include "SearchReplaceHandler.h"
#include <Qt>
#include <QtGlobal>
#include <QRegExp>
#include <QTextCursor>
#include <QTextBlock>
#include <QMessageBox>

namespace RstPad {

    SearchReplaceHandler::SearchReplaceHandler(QWidget *parent, QPlainTextEdit *edit) : QObject(parent)
    {
        this->edit = edit;
        this->dlg = new SearchReplaceDlg(parent);

        connect(this->dlg, &SearchReplaceDlg::job, this, &SearchReplaceHandler::handleJob);
    }

    void SearchReplaceHandler::activate()
    {
        this->dlg->show();
    }

    void SearchReplaceHandler::setWrapAround(bool wrapAround)
    {
        this->wrapAround = wrapAround;
    }

    void SearchReplaceHandler::redoLastSearch(bool backwards)
    {
        if (!lastSearchJob.text.isEmpty()) {
            lastSearchJob.backwards = backwards;

            handleJob(lastSearchJob);
        }
    }

    void SearchReplaceHandler::handleJob(const SearchJob &job)
    {
        // prepare regexp and options
        QRegExp regexp(
            job.text,
            job.caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive,
            job.wildcards ? QRegExp::Wildcard : QRegExp::FixedString
        );

        QTextDocument::FindFlags options;

        if (job.wholeWords) {
            options |= QTextDocument::FindWholeWords;
        }
        if (job.backwards) {
            options |= QTextDocument::FindBackward;
        }

        switch (job.mode) {
            // replace all
            case SearchJob::ReplaceAll: {
                auto numReplacements = handleReplaceAll(job, regexp, options);
                showInfoDialog(numReplacements > 0 ? tr("Replaced %1 occurences").arg(numReplacements) : tr("No matches"));
            } break;

            // count
            case SearchJob::Count:
                showInfoDialog(tr("Found %1 occurences").arg(countOccurences(regexp, options)));
                break;

            default:
                // replace or find
                if (job.mode == SearchJob::Replace && edit->textCursor().hasSelection()) {
                    if (!handleReplace(job, regexp, options)) {
                        emit matchFailed();
                    }
                } else {
                    if (!edit->find(regexp, options) && wrapAround) {
                        if (tryWrapAround(job, regexp, options)) {
                            emit wrappedAround();
                        } else {
                            emit matchFailed();
                        }
                    }
                }
                break;
        }

        // remember this search job
        setLastSearchJob(job);
    }

    bool SearchReplaceHandler::tryWrapAround(const SearchJob &job, const QRegExp &regexp, QTextDocument::FindFlags options)
    {
        // remember current cursor
        auto cursor = edit->textCursor();

        // prepare a wrap around cursor
        QTextCursor wrapCursor(edit->document());

        if (job.backwards) {
            auto lastBlock = edit->document()->lastBlock();
            wrapCursor.setPosition(lastBlock.position() + lastBlock.length() - 1);
        } else {
            wrapCursor.setPosition(0);
        }

        // try to search again from the wrap around cursor
        edit->setTextCursor(wrapCursor);

        if (edit->find(regexp, options)) {
            // success
            return true;
        } else {
            // failure - restore original cursor
            edit->setTextCursor(cursor);

            return false;
        }
    }

    bool SearchReplaceHandler::handleReplace(const SearchJob &job, const QRegExp &regexp,  QTextDocument::FindFlags options)
    {
        // remember current cursor
        auto cursor = edit->textCursor();

        // prepare a match cursor
        QTextCursor matchCursor(edit->document());
        matchCursor.setPosition(qMin(cursor.position(), cursor.anchor()));

        // try to match again from the match cursor (to ensure an actual match is selected)
        edit->setTextCursor(matchCursor);

        if (edit->find(regexp, options)) {
            // compare cursors
            matchCursor = edit->textCursor();

            if (matchCursor == cursor) {
                // success
                matchCursor.insertText(job.replacement);

                return true;
            }
        }

        // failed - restore original cursor
        edit->setTextCursor(cursor);

        return false;
    }

    int SearchReplaceHandler::handleReplaceAll(const SearchJob &job, const QRegExp &regexp, QTextDocument::FindFlags options)
    {
        // remember current cursor
        auto cursor = edit->textCursor();

        // prepare a replace all cursor
        QTextCursor replaceAllCursor(edit->document());
        replaceAllCursor.setPosition(0);

        // replace all occurences
        edit->setTextCursor(replaceAllCursor);

        int numReplacements = 0;
        cursor.beginEditBlock();

        while (edit->find(regexp, options)) {
            edit->textCursor().insertText(job.replacement);
            ++numReplacements;
        }

        cursor.endEditBlock();

        // restore original cursor if there were no replacements
        if (numReplacements == 0) {
            edit->setTextCursor(cursor);
        }

        return numReplacements;
    }

    int SearchReplaceHandler::countOccurences(const QRegExp &regexp, QTextDocument::FindFlags options)
    {
        // remember current cursor
        auto cursor = edit->textCursor();

        // prepare a match all cursor
        QTextCursor matchAllCursor(edit->document());
        matchAllCursor.setPosition(0);

        // find all occurences
        edit->setTextCursor(matchAllCursor);

        int numOccurences = 0;

        while (edit->find(regexp, options)) {
            ++numOccurences;
        }

        // restore original cursor
        edit->setTextCursor(cursor);

        return numOccurences;
    }

    void SearchReplaceHandler::setLastSearchJob(const SearchJob &job)
    {
        lastSearchJob = job;
        lastSearchJob.mode = SearchJob::Find;
        lastSearchJob.replacement = QString();
    }

    void SearchReplaceHandler::showInfoDialog(const QString &text)
    {
        QMessageBox infoDlg(dlg);

        infoDlg.setWindowTitle(tr("Search / replace"));
        infoDlg.setText(text);
        infoDlg.setIcon(QMessageBox::Information);

        infoDlg.exec();
    }

}
