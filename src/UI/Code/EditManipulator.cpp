#include "EditManipulator.h"
#include <QUrl>
#include <QString>
#include <QTextCursor>
#include <QRegExp>
#include <QtGlobal>
#include <QChar>

namespace RstPad {

    EditManipulator::EditManipulator(QPlainTextEdit *edit)
    {
        this->edit = edit;
    }

    void EditManipulator::indent()
    {
        indent(indentSize);
    }

    void EditManipulator::indent(int numChars)
    {
        auto cursor = edit->textCursor();

        if (cursor.hasSelection()) {
            // indent selection (one or more lines)
            auto blocks = getSelectedBlocks(cursor.selectionStart(), cursor.selectionEnd());
            auto indentation = QString(" ").repeated(numChars);

            cursor.beginEditBlock();

            for (int i = 0; i < blocks.length(); ++i) {
                insertText(blocks[i].position(), indentation);
            }

            cursor.endEditBlock();

        } else {
            // indent from cursor position
            int pos = cursor.positionInBlock();
            int numSpaces = qCeil((float) pos / (float) numChars) * numChars - pos;

            cursor.insertText(QString(" ").repeated(0 == numSpaces ? numChars : numSpaces));
        }
    }

    void EditManipulator::unindent()
    {
        unindent(indentSize);
    }

    void EditManipulator::unindent(int numChars)
    {
        auto cursor = edit->textCursor();

        if (cursor.hasSelection()) {
            // unindent selection (one or more lines)
            auto blocks = getSelectedBlocks(cursor.selectionStart(), cursor.selectionEnd());

            cursor.beginEditBlock();

            for (int i = 0; i < blocks.length(); ++i) {
                auto text = blocks[i].text();
                auto numCharsToRemove = numChars;

                // stop on first non-space character
                for (int j = 0; j < numChars; ++j) {
                    if (text[j] != QLatin1Char(' ')) {
                        numCharsToRemove = j;
                        break;
                    }
                }

                // remove the spaces
                if (numCharsToRemove > 0) {
                    removeText(blocks[i].position(), blocks[i].position() + numCharsToRemove);
                }
            }

            cursor.endEditBlock();
        } else {
            // unindent from cursor position
            int pos = cursor.positionInBlock();
            int numBackspaces = pos - pos / indentSize * indentSize;

            if (0 == numBackspaces && pos > 0) {
                numBackspaces = indentSize;
            }

            if (numBackspaces > 0) {
                auto text = cursor.block().text();

                // stop on first non-space character
                for (int i = 1; i <= numBackspaces; ++i) {
                    if (text[pos - i] != QLatin1Char(' ')) {
                        numBackspaces = i - 1;
                        break;
                    }
                }

                // remove the spaces
                if (numBackspaces > 0) {
                    removeText(
                        cursor.block().position() + pos - numBackspaces,
                        cursor.block().position() + pos
                    );
                }
            }
        }
    }

    void EditManipulator::indentNewLine()
    {
        auto cursor = edit->textCursor();
        auto currentBlockNumber = cursor.blockNumber();

        if (currentBlockNumber > 0) {
            auto currentIndentSize = getLineIndentation(edit->document()->findBlockByNumber(currentBlockNumber - 1).text());

            if (currentIndentSize > 0) {
                cursor.insertText(QString(" ").repeated(currentIndentSize));
            }
        }
    }

    void EditManipulator::jumpToStartOfLine(bool anchored)
    {
        auto cursor = edit->textCursor();

        auto indentation = getLineIndentation(cursor.block().text());
        auto moveMode = anchored ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

        if (cursor.positionInBlock() == indentation) {
            cursor.setPosition(cursor.block().position(), moveMode);
        } else {
            cursor.setPosition(cursor.block().position() + indentation, moveMode);
        }

        edit->setTextCursor(cursor);
    }

    void EditManipulator::setIndentSize(int size)
    {
        indentSize = size;
    }

    void EditManipulator::formatSelection(const QString &format, const QString &defaultText)
    {
        auto selectedText = edit->textCursor().selectedText();
        int selectionOffsetLeft = 0, selectionOffsetRight = 0;

        // try to limit selection to the inserted text
        auto placeholderOffset = format.indexOf("%1");

        if (placeholderOffset >= 0) {
            selectionOffsetLeft = placeholderOffset;
            selectionOffsetRight = format.length() - placeholderOffset - 2;
        }

        // insert and select
        insertAndSelectText(format.arg(!selectedText.isEmpty() ? selectedText : defaultText), selectionOffsetLeft, selectionOffsetRight);
    }

    void EditManipulator::formatSelectionAsLink(const QString &defaultUrl)
    {
        auto selectedText = edit->textCursor().selectedText();

        if (selectedText.isEmpty()) {
            // no selection - insert generic text and edit URL
            formatSelection("`link <%1>`_", defaultUrl);
        } else {
            // there is selection
            QUrl url(selectedText);

            if (url.isValid() && !url.scheme().isEmpty()) {
                // edit text
                insertAndSelectText(QString("`link <%1>`_").arg(selectedText), 1, 5 + selectedText.length());
            } else {
                // edit URL
                insertAndSelectText(QString("`%1 <%2>`_").arg(selectedText, defaultUrl), 3 + selectedText.length(), 3);
            }
        }
    }

    void EditManipulator::formatSelectionAsCodeBlock(const QString &pygmentsLexer)
    {
        auto cursor = edit->textCursor();
        auto selectedText = cursor.selectedText();

        cursor.beginEditBlock();

        ensureEmptyLines(true, true);

        if (!pygmentsLexer.isEmpty()) {
            cursor.insertText(QString(".. code:: %1\n\n").arg(pygmentsLexer));
        } else {
            cursor.insertText("::\n\n");
        }

        cursor.insertText(indentText(!selectedText.isEmpty() ? selectedText : "code"));

        cursor.endEditBlock();
    }

    void EditManipulator::formatSelectionAsHeading(const QString &symbol, bool overline)
    {
        auto cursor = edit->textCursor();

        QString text;

        if (cursor.hasSelection()) {
            text = cursor.selectedText();
        } else {
            text = QString("Heading");
        }

        cursor.beginEditBlock();

        ensureEmptyLines(true, true);

        QString toInsert;
        toInsert.reserve(text.length() * (overline ? 3 : 2) + (overline ? 3 : 2));

        QString line;
        line.reserve(text.length() + 1);
        line.append(symbol.repeated(text.length()));

        if (overline) {
            toInsert.append(line);
            toInsert.append("\n");
        }
        toInsert.append(text);
        toInsert.append("\n");
        toInsert.append(line);

        insertAndSelectText(toInsert, overline ? line.length() + 1 : 0, line.length() + 1);

        cursor.endEditBlock();

    }

    void EditManipulator::formatSelectionAsHorizontalRule(const QString &symbol, int width)
    {
        auto cursor = edit->textCursor();

        cursor.beginEditBlock();

        ensureEmptyLines(true, true);
        cursor.insertText(symbol.repeated(width));

        cursor.endEditBlock();
    }

    bool EditManipulator::gotoLine(int line)
    {
        if (line >= 1 && line <= edit->document()->lastBlock().blockNumber() + 1) {
            auto cursor = edit->textCursor();
            cursor.setPosition(edit->document()->findBlockByNumber(line - 1).position());
            edit->setTextCursor(cursor);

            return true;
        } else {
            return false;
        }
    }

    void EditManipulator::trimTrailingWhitespace()
    {
        auto document = edit->document();
        auto cursor = edit->textCursor();

        // scan all lines
        cursor.beginEditBlock();

        QTextCursor wsRemovalCursor(document);

        for (int i = 0; i < document->blockCount(); ++i) {
            auto block = document->findBlockByNumber(i);
            auto text = block.text();
            int numTrailingSpaces = 0;

            for (int j = text.length() - 1; j >= 0; --j) {
                if (text[j] == QLatin1Char(' ')) {
                    ++numTrailingSpaces;
                } else {
                    break;
                }
            }

            if (numTrailingSpaces > 0) {
                wsRemovalCursor.setPosition(block.position() + text.length() - numTrailingSpaces);
                wsRemovalCursor.setPosition(block.position() + text.length(), QTextCursor::KeepAnchor);
                wsRemovalCursor.removeSelectedText();
            }
        }

        cursor.endEditBlock();

        // restore original cursor
        edit->setTextCursor(cursor);
    }

    void EditManipulator::ensureSingleBlankLineAtEnd()
    {
        auto document = edit->document();
        auto cursor = edit->textCursor();

        // scan lines from the end
        QRegExp emptyLineRegExp("^\\s*$");

        int numEmptyLinesAtEnd = 0;

        for (int i = document->blockCount() - 1; i >= 0; --i) {
            auto block = document->findBlockByNumber(i);
            auto text = block.text();

            if (text.contains(emptyLineRegExp)) {
                ++numEmptyLinesAtEnd;
            } else {
                break;
            }
        }

        // remove extra lines or insert a new one
        cursor.beginEditBlock();

        if (numEmptyLinesAtEnd >= 1) {
            for (int i = numEmptyLinesAtEnd; i > 0; --i) {
                QTextCursor removalCursor(document->lastBlock());
                removalCursor.select(QTextCursor::LineUnderCursor);
                removalCursor.removeSelectedText();

                if (i > 1) {
                    removalCursor.deletePreviousChar();
                }
            }
        } else {
            QTextCursor insertionCursor(document);
            insertionCursor.setPosition(document->lastBlock().position() + document->lastBlock().length() - 1);
            insertionCursor.insertText("\n");
        }

        cursor.endEditBlock();

        // restore original cursor
        edit->setTextCursor(cursor);
    }

    int EditManipulator::getLineIndentation(const QString &text)
    {
        int indentation = 0;

        for (int i = 0; i < text.length(); ++i) {
            if (text[i] == QLatin1Char(' ')) {
                ++indentation;
            } else {
                break;
            }
        }

        return indentation;
    }

    void EditManipulator::removeText(int from, int to)
    {
        QTextCursor removalCursor(edit->document());

        removalCursor.setPosition(from);
        removalCursor.setPosition(to, QTextCursor::KeepAnchor);
        removalCursor.removeSelectedText();
    }

    void EditManipulator::insertText(int at, const QString &text)
    {
        QTextCursor insertionCursor(edit->document());

        insertionCursor.setPosition(at);
        insertionCursor.insertText(text);
    }

    void EditManipulator::insertAndSelectText(const QString &text, int selectionOffsetLeft, int selectionOffsetRight)
    {
        auto cursor = edit->textCursor();

        // store current position
        auto start = qMin(cursor.position(), cursor.anchor());

        // insert text
        cursor.insertText(text);

        // store position after insertion
        auto anchor = cursor.position();

        // make selection
        QTextCursor selectionCursor(edit->document());
        selectionCursor.setPosition(anchor - selectionOffsetRight);
        selectionCursor.setPosition(start + selectionOffsetLeft, QTextCursor::KeepAnchor);
        edit->setTextCursor(selectionCursor);
    }

    QString EditManipulator::indentText(const QString &text)
    {
        auto indentation = QString(" ").repeated(indentSize);
        auto lines = text.split(QChar::ParagraphSeparator);

        QString output;
        output.reserve(text.length() + lines.length() * indentSize);

        for (int i = 0; i < lines.length(); ++i) {
            if (i > 0) {
                output.append(QChar::ParagraphSeparator);
            }
            output.append(indentation);
            output.append(lines[i]);
        }

        return output;
    }

    void EditManipulator::ensureEmptyLines(bool before, bool after)
    {
        auto cursor = edit->textCursor();
        auto document = edit->document();
        QRegExp emptyLineRegExp("^\\s*$");

        // ensure empty line before selection
        if (before) {
            auto start = qMin(cursor.position(), cursor.anchor());
            auto startBlock = document->findBlock(start);

            if (startBlock.blockNumber() > 0) {
                auto previousBlock = document->findBlockByNumber(startBlock.blockNumber() - 1);

                if (!previousBlock.text().contains(emptyLineRegExp)) {
                    QTextCursor insertionCursor(edit->document());
                    insertionCursor.setPosition(startBlock.position());
                    insertionCursor.insertText("\n");
                }
            }
        }

        // ensure empty line after selection
        if (after) {
            auto end = qMax(cursor.position(), cursor.anchor());
            auto endBlock = document->findBlock(end);
            auto nextBlock = document->findBlockByNumber(endBlock.blockNumber() + 1);

            if (nextBlock.isValid() && !nextBlock.text().contains(emptyLineRegExp)) {
                QTextCursor insertionCursor(edit->document());
                insertionCursor.setPosition(nextBlock.position());
                insertionCursor.insertText("\n");
            }
        }
    }

    QVector<QTextBlock> EditManipulator::getSelectedBlocks(int from, int to)
    {
        auto document = edit->document();

        auto firstBlockNumber = document->findBlock(from).blockNumber();
        auto lastBlockNumber = document->findBlock(to).blockNumber();

        QVector<QTextBlock> blocks;

        for (int i = firstBlockNumber; i <= lastBlockNumber; ++i) {
            blocks.append(document->findBlockByNumber(i));
        }

        return blocks;
    }

}
