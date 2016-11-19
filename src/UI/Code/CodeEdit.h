#ifndef CODEEDIT_H
#define CODEEDIT_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QTextBlock>
#include <QKeyEvent>
#include "EditManipulator.h"

namespace RstPad {

    class CodeEdit : public QPlainTextEdit
    {
        Q_OBJECT

        public:
            explicit CodeEdit(QWidget *parent = 0);
            virtual ~CodeEdit();
            QTextBlock firstVisibleBlock() const;
            EditManipulator *manipulator();

        private:
            EditManipulator *m_manipulator;
            void keyPressEvent(QKeyEvent *e);
    };

}

#endif // CODEEDIT_H
