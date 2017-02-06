#ifndef EDITMANIPULATOR_H
#define EDITMANIPULATOR_H

#include <QString>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QtMath>

namespace RstPad {

    class EditManipulator
    {
        public:
            EditManipulator(QPlainTextEdit *edit);
            void indent();
            void indent(int numChars);
            void unindent();
            void unindent(int numChars);
            void indentNewLine();
            void jumpToStartOfLine(bool anchored = false);
            void setIndentSize(int size);
            void formatSelection(const QString &format, const QString &defaultText = QString("text"));
            void formatSelectionAsLink(const QString &defaultUrl = QString("http://example.com/"));
            void formatSelectionAsCodeBlock(const QString &pygmentsLexer = QString());
            void formatSelectionAsHeading(const QString &symbol, bool overline);
            void formatSelectionAsHorizontalRule(const QString &symbol, int width = 4);
            bool gotoLine(int line);
            void trimTrailingWhitespace();
            void ensureSingleBlankLineAtEnd();

        private:
            int indentSize = 4;
            QPlainTextEdit *edit;
            int getLineIndentation(const QString &text);
            void removeText(int from, int to);
            void insertText(int at, const QString &text);
            void insertAndSelectText(const QString &text, int selectionOffsetLeft = 0, int selectionOffsetRight = 0);
            QString indentText(const QString &text);
            void ensureEmptyLines(bool before, bool after);
            QVector<QTextBlock> getSelectedBlocks(int from, int to);
    };

}

#endif // EDITMANIPULATOR_H
