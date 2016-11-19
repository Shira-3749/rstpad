#include "SyntaxHelpWindow.h"
#include "ui_SyntaxHelpWindow.h"
#include "WebPage.h"
#include <Qt>
#include <QShortcut>

namespace RstPad {

    SyntaxHelpWindow::SyntaxHelpWindow(QWidget *parent) :
        QMainWindow(parent),
        ui(new Ui::SyntaxHelpWindow)
    {
        ui->setupUi(this);

        // setup web engine view
        auto webPage = new WebPage(ui->WebEngineView);
        ui->WebEngineView->setPage(webPage);

        // register CTRL-F shortuc
        auto findShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_F), this);
        findShortcut->setContext(Qt::WindowShortcut);

        // load initial page
        on_QuickrefButton_clicked();

        // events
        connect(findShortcut, &QShortcut::activated, this, &SyntaxHelpWindow::findShortcut);
    }

    SyntaxHelpWindow::~SyntaxHelpWindow()
    {
        delete ui;
    }

    void SyntaxHelpWindow::closeEvent(QCloseEvent *event)
    {
        Q_UNUSED(event);

        emit closed();
    }

    void SyntaxHelpWindow::keyPressEvent(QKeyEvent *event)
    {
        if (event->key() == Qt::Key_Escape) {
            if (focusWidget() == ui->Search) {
                ui->Search->clearFocus();
            } else {
                close();
            }
        } else {
            QMainWindow::keyPressEvent(event);
        }
    }

    void SyntaxHelpWindow::doSearch()
    {
        ui->WebEngineView->page()->findText(ui->Search->text());
    }

}

void RstPad::SyntaxHelpWindow::findShortcut()
{
    ui->Search->setFocus();
    ui->Search->selectAll();
}

void RstPad::SyntaxHelpWindow::on_QuickrefButton_clicked()
{
    ui->WebEngineView->load(QUrl("qrc:///syntax-help/quickref.html"));
}

void RstPad::SyntaxHelpWindow::on_DirectivesButton_clicked()
{
    ui->WebEngineView->load(QUrl("qrc:///syntax-help/directives.html"));
}

void RstPad::SyntaxHelpWindow::on_Search_returnPressed()
{
    doSearch();
}

void RstPad::SyntaxHelpWindow::on_SearchButton_clicked()
{
    doSearch();
}

void RstPad::SyntaxHelpWindow::on_Search_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)

    doSearch();
}
