#ifndef RSTPAD_PYGMENTLEXERSDLG_H
#define RSTPAD_PYGMENTLEXERSDLG_H

#include <QDialog>
#include <QCompleter>
#include <QString>
#include <QStringList>

namespace RstPad {

    namespace Ui {
        class PygmentLexersDlg;
    }

    class PygmentLexersDlg : public QDialog
    {
        Q_OBJECT

        public:
            explicit PygmentLexersDlg(QWidget *parent = 0);
            ~PygmentLexersDlg();
            QString lexer();

        private:
            Ui::PygmentLexersDlg *ui;
            QCompleter *completer;
            QStringList pygmentsLexers();
    };

}
#endif // RSTPAD_PYGMENTLEXERSDLG_H
