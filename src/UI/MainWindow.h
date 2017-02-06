#ifndef RSTPAD_MAINWINDOW_H
#define RSTPAD_MAINWINDOW_H

#include "../Config.h"
#include "SyntaxHelpWindow.h"
#include "PreviewView.h"
#include "Code/CodeEdit.h"
#include "SearchReplaceHandler.h"
#include <QMainWindow>
#include <QCloseEvent>
#include <QResizeEvent>
#include <QString>
#include <QLabel>
#include <QByteArray>
#include <QAction>

namespace RstPad {

    enum class EditorOrientation;

    namespace Ui {
        class MainWindow;
    }

    class MainWindow : public QMainWindow
    {
        Q_OBJECT

        public:
            explicit MainWindow(QWidget *parent = 0);
            ~MainWindow();
            PreviewView *preview();
            CodeEdit *codeEditor();
            void resizeEvent(QResizeEvent *resizeEvent);
            void setEditorOrientation(EditorOrientation orientation);
            void showStatus(const QString &message, int timeout = 3000);
            void showSyntaxHelp();

        public slots:
            void applyConfig(Config &config);
            void openRecentFile(QAction *action);
            void fileChanged();
            void fileSaveRequested(const QString &path, QByteArray &data);
            void updateFileUi();
            void inputChanged();
            void inputScrolled();
            void inputCursorChanged();
            void syntaxHelpWindowClosed();
            void searchWrappedAround();
            void searchFailed();

        protected:
            void closeEvent(QCloseEvent *event);

        private:
            Ui::MainWindow *ui;
            QLabel *statusBarLabel;
            SearchReplaceHandler *searchReplaceHandler;
            SyntaxHelpWindow *syntaxHelpWindow = nullptr;
            int lastGotoLine = 1;
            bool restoredState = false;

        private slots:
            void on_ActionSettings_triggered();
            void on_ActionExit_triggered();
            void on_ActionAbout_triggered();
            void on_ActionAbout_Qt_triggered();
            void on_ActionNew_triggered();
            void on_ActionOpen_triggered();
            void on_ActionSave_triggered();
            void on_ActionSave_as_triggered();
            void on_ActionSyntaxHelp_triggered();
            void on_ActionPDFExport_triggered();
            void on_ActionBold_triggered();
            void on_ActionItalic_triggered();
            void on_ActionInlineCode_triggered();
            void on_ActionUL_triggered();
            void on_ActionOL_triggered();
            void on_ActionImage_triggered();
            void on_ActionLink_triggered();
            void on_ActionCodeBlock_triggered();
            void on_ActionPygmentsCodeBlock_triggered();
            void on_ActionHeading1_triggered();
            void on_ActionHeading2_triggered();
            void on_ActionHeading3_triggered();
            void on_ActionHeading4_triggered();
            void on_ActionHeading5_triggered();
            void on_ActionHeading6_triggered();
            void on_ActionHorizontalRule_triggered();
            void on_ActionSearchReplace_triggered();
            void on_ActionUndo_triggered();
            void on_ActionRedo_triggered();
            void on_ActionCut_triggered();
            void on_ActionCopy_triggered();
            void on_ActionPaste_triggered();
            void on_ActionFindNext_triggered();
            void on_ActionFindPrev_triggered();
            void on_ActionGotoLine_triggered();
            void on_ActionIndent_triggered();
            void on_ActionUnindent_triggered();
    };

}
#endif // RSTPAD_MAINWINDOW_H
