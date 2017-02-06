#include "../App.h"
#include "Code/EditManipulator.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "WebPage.h"
#include "SettingsDlg.h"
#include "PygmentLexersDlg.h"
#include <Qt>
#include <QTextOption>
#include <QFontDatabase>
#include <QMessageBox>
#include <QInputDialog>
#include <QBoxLayout>
#include <QScrollBar>
#include <QMenu>
#include <QFileInfo>

namespace RstPad {

    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::MainWindow)
    {
        ui->setupUi(this);

        // setup open recent menu
        auto fileMenu = ui->MenuBar->findChild<QMenu*>("MenuFile");
        auto openRecentMenu = new QMenu(tr("Open recent"), fileMenu);
        openRecentMenu->setObjectName("MenuOpenRecent");
        fileMenu->insertMenu(fileMenu->actions().at(2), openRecentMenu);

        // setup status bar
        statusBarLabel = new QLabel();
        ui->StatusBar->setContentsMargins(10, 0, 10, 10);
        ui->StatusBar->addWidget(statusBarLabel);

        // setup preview
        auto previewPage = new WebPage(this->ui->Preview);
        ui->Preview->setPage(previewPage);

        // search replace handler
        searchReplaceHandler = new SearchReplaceHandler(this, ui->CodeEditor);

        // events
        connect(APP->config(), &Config::updated, this, &MainWindow::applyConfig);
        connect(openRecentMenu, &QMenu::triggered, this, &MainWindow::openRecentFile);
        connect(ui->CodeEditor, &CodeEdit::textChanged, this, &MainWindow::inputChanged);
        connect(ui->CodeEditor->verticalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::inputScrolled);
        connect(ui->CodeEditor, &CodeEdit::cursorPositionChanged, this, &MainWindow::inputCursorChanged);
        connect(searchReplaceHandler, &SearchReplaceHandler::wrappedAround, this, &MainWindow::searchWrappedAround);
        connect(searchReplaceHandler, &SearchReplaceHandler::matchFailed, this, &MainWindow::searchFailed);
    }

    MainWindow::~MainWindow()
    {
        delete ui;
        delete syntaxHelpWindow;
    }

    PreviewView *MainWindow::preview()
    {
        return ui->Preview;
    }

    CodeEdit *MainWindow::codeEditor()
    {
        return ui->CodeEditor;
    }

    void MainWindow::closeEvent(QCloseEvent *closeEvent)
    {
        if (APP->fileManager()->ensureChangesNotLost()) {
            // close syntax help window when main window is closed
            if (syntaxHelpWindow) {
                syntaxHelpWindow->close();
            }

            // close accepted
            closeEvent->accept();
        } else {
            // close cancelled
            closeEvent->ignore();
        }
    }

    void MainWindow::resizeEvent(QResizeEvent *resizeEvent)
    {
        Q_UNUSED(resizeEvent)

        APP->previewManager()->autoscroll();
    }

    void MainWindow::setEditorOrientation(EditorOrientation orientation)
    {
        ui->layout->setDirection(orientation == EditorOrientation::Horizontal ? QBoxLayout::LeftToRight : QBoxLayout::TopToBottom);
    }

    void MainWindow::showStatus(const QString &message, int timeout)
    {
        ui->StatusBar->showMessage(message, timeout);
    }

    void MainWindow::showSyntaxHelp()
    {
        if (!syntaxHelpWindow) {
            syntaxHelpWindow = new SyntaxHelpWindow();

            connect(syntaxHelpWindow, &SyntaxHelpWindow::closed, this, &MainWindow::syntaxHelpWindowClosed);

            syntaxHelpWindow->show();
        } else {
            APP->setActiveWindow(syntaxHelpWindow);
        }
    }

    void MainWindow::applyConfig(Config &config)
    {
        // restore state and geometry (once)
        if (!restoredState) {
            auto mainWindowState = config.get("mainWindowState");
            auto mainWindowGeometry = config.get("mainWindowGeometry");

            if (!mainWindowState.isNull()) {
                restoreState(QByteArray::fromBase64(mainWindowState.toByteArray()));
            }
            if (!mainWindowGeometry.isNull()) {
                restoreGeometry(QByteArray::fromBase64(mainWindowGeometry.toByteArray()));
            }

            restoredState = true;
        }

        // preview zoom factor
        ui->Preview->setZoomFactor(config.get("previewZoomFactor").toDouble());

        // word wrap mode
        auto wordWrapMode = static_cast<QTextOption::WrapMode>(config.get("wordWrapMode").toInt());
        ui->CodeEditor->setLineWrapMode(wordWrapMode == QTextOption::NoWrap ? CodeEdit::NoWrap : CodeEdit::WidgetWidth);
        ui->CodeEditor->setWordWrapMode(wordWrapMode);

        // font fize
        auto codeEditorFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
        codeEditorFont.setPointSize(config.get("fontSize").toInt());
        ui->CodeEditor->setFont(codeEditorFont);

        // indent size
        ui->CodeEditor->manipulator()->setIndentSize(config.get("indentSize").toInt());

        // search wrap around
        searchReplaceHandler->setWrapAround(config.get("searchWrapAround").toBool());

        // recent files
        QStringList recentFiles = config.get("recentFiles").toStringList();

        auto openRecentMenu = ui->MenuBar->findChild<QMenu*>("MenuOpenRecent");
        openRecentMenu->clear();

        if (!recentFiles.isEmpty()) {
            foreach (const QString &filePath, recentFiles) {
                QFileInfo fileInfo(filePath);
                auto fileAction = new QAction(QString("%1 (%2)").arg(fileInfo.fileName(), fileInfo.path()), openRecentMenu);
                fileAction->setData(filePath);
                openRecentMenu->addAction(fileAction);
            }
        } else {
            auto noRecentFilesAction = new QAction(tr("No recent files"), openRecentMenu);
            noRecentFilesAction->setDisabled(true);
            openRecentMenu->addAction(noRecentFilesAction);
        }
    }

    void MainWindow::openRecentFile(QAction *action)
    {
        APP->fileManager()->openFile(action->data().toString());
    }

    void MainWindow::fileChanged()
    {
        // file changed
        // update the code editor
        if (APP->fileManager()->isFileOpen()) {
            // existing file
            auto filePath = APP->fileManager()->file().absoluteFilePath();

            ui->CodeEditor->setPlainText(QString::fromUtf8(APP->fileManager()->readFile()));

            // reset modification state - setPlainText() makes the document modified
            ui->CodeEditor->document()->setModified(false);
            APP->fileManager()->makeClean();

            // update recent file list
            auto recentFiles = APP->config()->get("recentFiles").toStringList();
            if (!recentFiles.contains(filePath)) {
                while (recentFiles.length() >= 10) {
                    recentFiles.removeLast();
                }
                recentFiles.prepend(filePath);
                APP->config()->set("recentFiles", recentFiles);
            }
        } else {
            // new file
            ui->CodeEditor->clear();
        }

        lastGotoLine = 1;
    }

    void MainWindow::fileSaveRequested(const QString &path, QByteArray &data)
    {
        Q_UNUSED(path)

        // pre-save processing
        bool ensureSingleEmptyLineAtEnd = APP->config()->get("ensureSingleEmptyLineAtEndOnSave").toBool();
        bool trimTrailingWhitespace = APP->config()->get("trimTrailingWhitespaceOnSave").toBool();

        if (ensureSingleEmptyLineAtEnd || trimTrailingWhitespace) {
            ui->CodeEditor->textCursor().beginEditBlock();

            if (ensureSingleEmptyLineAtEnd) {
                ui->CodeEditor->manipulator()->ensureSingleBlankLineAtEnd();
            }
            if (trimTrailingWhitespace) {
                ui->CodeEditor->manipulator()->trimTrailingWhitespace();
            }

            ui->CodeEditor->textCursor().endEditBlock();
        }

        // use the code editor contents
        showStatus(tr("Saving"), 500);
        data = ui->CodeEditor->toPlainText().toUtf8();
    }

    void MainWindow::updateFileUi()
    {
        bool dirty = APP->fileManager()->isDirty();

        // update window title
        QString title;
        title.append(APP->fileManager()->presentableFileName());
        if (dirty) {
            title.append("*");
        }
        title.append(" - RSTPad");
        setWindowTitle(title);

        // update status bar
        statusBarLabel->setText(APP->fileManager()->isFileOpen() ? APP->fileManager()->file().filePath() : tr("New file"));

        // update save button
        ui->ActionSave->setEnabled(dirty);
    }

    void MainWindow::inputChanged()
    {
        auto text = ui->CodeEditor->toPlainText();

        if (ui->CodeEditor->document()->isModified()) {
            APP->fileManager()->makeDirty();
        } else {
            APP->fileManager()->makeClean();
        }

        APP->rstRenderer()->schedule(text);
    }

    void MainWindow::inputScrolled()
    {
        APP->previewManager()->autoscroll();
    }

    void MainWindow::inputCursorChanged()
    {
        APP->previewManager()->autoscroll();
    }

    void MainWindow::syntaxHelpWindowClosed()
    {
        if (syntaxHelpWindow) {
            syntaxHelpWindow->deleteLater();
            syntaxHelpWindow = nullptr;
        }
    }

    void MainWindow::searchWrappedAround()
    {
        APP->beep();
    }

    void MainWindow::searchFailed()
    {
        APP->beep();
    }

}

void RstPad::MainWindow::on_ActionSettings_triggered()
{
    SettingsDlg dlg(this);

    dlg.show();
    dlg.exec();
}

void RstPad::MainWindow::on_ActionExit_triggered()
{
    close();
}

void RstPad::MainWindow::on_ActionAbout_triggered()
{
    auto text = R"about(RSTPad v%1
(built %2 %3)

Author: ShiraNai7 (shira.cz)
License: MIT

Credits:

QT %4
Docutils (docutils.sourceforge.net)
Pygments (pygments.org)
Python 2.7 (python.org)
FamFamFam (famfamfam.com))about";

    QMessageBox::about(this, tr("About RSTPad"), QString(text).arg(APP_VERSION, __DATE__, __TIME__, QT_VERSION_STR));
}

void RstPad::MainWindow::on_ActionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this, tr("About Qt"));
}

void RstPad::MainWindow::on_ActionNew_triggered()
{
    APP->fileManager()->newFile();
}

void RstPad::MainWindow::on_ActionOpen_triggered()
{
    APP->fileManager()->openFile();
}

void RstPad::MainWindow::on_ActionSave_triggered()
{
    APP->fileManager()->saveFile();
}

void RstPad::MainWindow::on_ActionSave_as_triggered()
{
    APP->fileManager()->saveFileAs();
}

void RstPad::MainWindow::on_ActionSyntaxHelp_triggered()
{
    showSyntaxHelp();
}

void RstPad::MainWindow::on_ActionPDFExport_triggered()
{
    APP->exportToPdf();
}

void RstPad::MainWindow::on_ActionBold_triggered()
{
    ui->CodeEditor->manipulator()->formatSelection("**%1**");
}

void RstPad::MainWindow::on_ActionItalic_triggered()
{
    ui->CodeEditor->manipulator()->formatSelection("*%1*");
}

void RstPad::MainWindow::on_ActionInlineCode_triggered()
{
    ui->CodeEditor->manipulator()->formatSelection("``%1``");
}

void RstPad::MainWindow::on_ActionUL_triggered()
{
    ui->CodeEditor->manipulator()->formatSelection("- %1", "item");
}

void RstPad::MainWindow::on_ActionOL_triggered()
{
    ui->CodeEditor->manipulator()->formatSelection("#. %1", "item");
}

void RstPad::MainWindow::on_ActionImage_triggered()
{
    ui->CodeEditor->manipulator()->formatSelection(".. image:: %1", "path");
}

void RstPad::MainWindow::on_ActionLink_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsLink();
}

void RstPad::MainWindow::on_ActionCodeBlock_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsCodeBlock();
}

void RstPad::MainWindow::on_ActionPygmentsCodeBlock_triggered()
{
    PygmentLexersDlg dlg(this);

    if (dlg.exec() == PygmentLexersDlg::Accepted) {
        ui->CodeEditor->manipulator()->formatSelectionAsCodeBlock(dlg.lexer());
    }
}

void RstPad::MainWindow::on_ActionHeading1_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHeading(
        APP->config()->get("headingSymbol1").toString(),
        APP->config()->get("headingOverline1").toBool()
    );
}

void RstPad::MainWindow::on_ActionHeading2_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHeading(
        APP->config()->get("headingSymbol2").toString(),
        APP->config()->get("headingOverline2").toBool()
    );
}

void RstPad::MainWindow::on_ActionHeading3_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHeading(
        APP->config()->get("headingSymbol3").toString(),
        APP->config()->get("headingOverline3").toBool()
    );
}

void RstPad::MainWindow::on_ActionHeading4_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHeading(
        APP->config()->get("headingSymbol4").toString(),
        APP->config()->get("headingOverline4").toBool()
    );
}

void RstPad::MainWindow::on_ActionHeading5_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHeading(
        APP->config()->get("headingSymbol5").toString(),
        APP->config()->get("headingOverline5").toBool()
    );
}

void RstPad::MainWindow::on_ActionHeading6_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHeading(
        APP->config()->get("headingSymbol6").toString(),
        APP->config()->get("headingOverline6").toBool()
    );
}

void RstPad::MainWindow::on_ActionHorizontalRule_triggered()
{
    ui->CodeEditor->manipulator()->formatSelectionAsHorizontalRule(
        APP->config()->get("hrSymbol").toString(),
        APP->config()->get("hrWidth").toInt()
    );
}

void RstPad::MainWindow::on_ActionSearchReplace_triggered()
{
    searchReplaceHandler->activate();
}

void RstPad::MainWindow::on_ActionUndo_triggered()
{
    ui->CodeEditor->undo();
}

void RstPad::MainWindow::on_ActionRedo_triggered()
{
    ui->CodeEditor->redo();
}

void RstPad::MainWindow::on_ActionCut_triggered()
{
    ui->CodeEditor->cut();
}

void RstPad::MainWindow::on_ActionCopy_triggered()
{
    ui->CodeEditor->copy();
}

void RstPad::MainWindow::on_ActionPaste_triggered()
{
    ui->CodeEditor->paste();
}

void RstPad::MainWindow::on_ActionFindNext_triggered()
{
    searchReplaceHandler->redoLastSearch();
}

void RstPad::MainWindow::on_ActionFindPrev_triggered()
{
    searchReplaceHandler->redoLastSearch(true);
}

void RstPad::MainWindow::on_ActionGotoLine_triggered()
{
    bool ok = false;

    int line = QInputDialog::getInt(
        this, tr("Go to line"),
        tr("Line number"),
        lastGotoLine,
        1,
        ui->CodeEditor->document()->lastBlock().blockNumber() + 1,
        1,
        &ok
    );

    if (ok) {
        lastGotoLine = line;

        if (ui->CodeEditor->manipulator()->gotoLine(line)) {
            ui->CodeEditor->setFocus();
        } else {
            APP->beep();
        }
    }
}

void RstPad::MainWindow::on_ActionIndent_triggered()
{
    if (APP->keyboardModifiers() & Qt::ShiftModifier) {
        ui->CodeEditor->manipulator()->indent(1);
    } else {
        ui->CodeEditor->manipulator()->indent();
    }
}

void RstPad::MainWindow::on_ActionUnindent_triggered()
{
    if (APP->keyboardModifiers() & Qt::ShiftModifier) {
        ui->CodeEditor->manipulator()->unindent(1);
    } else {
        ui->CodeEditor->manipulator()->unindent();
    }
}
