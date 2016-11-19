#include "CodeEdit.h"
#include "EditManipulator.h"

namespace RstPad {

    CodeEdit::CodeEdit(QWidget *parent) : QPlainTextEdit(parent)
    {
        setWordWrapMode(QTextOption::NoWrap);

        m_manipulator = new EditManipulator(this);
    }

    CodeEdit::~CodeEdit()
    {
        delete m_manipulator;
    }

    EditManipulator *CodeEdit::manipulator()
    {
        return m_manipulator;
    }

    void CodeEdit::keyPressEvent(QKeyEvent *e)
    {
        switch (e->key()) {
            case Qt::Key_Tab:
                m_manipulator->indent();
                break;

            case Qt::Key_Backtab:
                m_manipulator->unindent();
                break;

            case Qt::Key_Enter:
            case Qt::Key_Return: {
                auto cursor = textCursor();
                cursor.beginEditBlock();
                QPlainTextEdit::keyPressEvent(e);
                m_manipulator->indentNewLine();
                cursor.endEditBlock();
            } break;

            case Qt::Key_Home:
                m_manipulator->jumpToStartOfLine((e->modifiers() & Qt::ShiftModifier) != 0);
                break;

            default:
                QPlainTextEdit::keyPressEvent(e);
                break;
        }

        ensureCursorVisible();
    }

    QTextBlock CodeEdit::firstVisibleBlock() const
    {
        return QPlainTextEdit::firstVisibleBlock();
    }
}
