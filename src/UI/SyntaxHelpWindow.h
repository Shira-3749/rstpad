#ifndef RSTPAD_SYNTAXHELPWINDOW_H
#define RSTPAD_SYNTAXHELPWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QKeyEvent>

namespace RstPad {

    namespace Ui {
        class SyntaxHelpWindow;
    }

    class SyntaxHelpWindow : public QMainWindow
    {
        Q_OBJECT

        public:
            explicit SyntaxHelpWindow(QWidget *parent = 0);
            ~SyntaxHelpWindow();

        signals:
            void closed();

        protected:
            void closeEvent(QCloseEvent *event);
            void keyPressEvent(QKeyEvent *event);

        private slots:
            void findShortcut();
            void on_QuickrefButton_clicked();
            void on_DirectivesButton_clicked();
            void on_Search_returnPressed();
            void on_SearchButton_clicked();
            void on_Search_textChanged(const QString &arg1);

        private:
            Ui::SyntaxHelpWindow *ui;
            void doSearch();
    };

}
#endif // RSTPAD_SYNTAXHELPWINDOW_H
