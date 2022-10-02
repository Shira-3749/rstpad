#include "SearchReplaceDlg.h"
#include "ui_SearchReplaceDlg.h"
#include <Qt>

namespace RstPad {

    SearchReplaceDlg::SearchReplaceDlg(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::SearchReplaceDlg)
    {
        ui->setupUi(this);

        setWindowFlags(
            (windowFlags() & ~Qt::WindowContextHelpButtonHint)
            | Qt::WindowStaysOnTopHint
        );

        connect(ui->Text, &QLineEdit::returnPressed, this, &SearchReplaceDlg::on_Find_clicked);
        connect(ui->Replacement, &QLineEdit::returnPressed, this, &SearchReplaceDlg::on_Replace_clicked);
    }

    SearchReplaceDlg::~SearchReplaceDlg()
    {
        delete ui;
    }

    void SearchReplaceDlg::show()
    {
        QWidget::show();

        ui->Text->setFocus();
    }

    SearchJob SearchReplaceDlg::buildJob(SearchJob::Mode mode)
    {
        SearchJob job;

        job.mode = mode;
        job.text = ui->Text->text();
        job.caseSensitive = ui->CaseSensitive->isChecked();
        job.wholeWords = ui->WholeWords->isChecked();
        job.wildcards = ui->Wildcards->isChecked();
        job.backwards = ui->DirectionBackward->isChecked();

        if (mode == SearchJob::Replace || mode == SearchJob::ReplaceAll) {
            job.replacement = ui->Replacement->text();
        }

        return job;
    }

}

void RstPad::SearchReplaceDlg::on_Find_clicked()
{
    emit job(buildJob(SearchJob::Find));
}

void RstPad::SearchReplaceDlg::on_Replace_clicked()
{
    emit job(buildJob(SearchJob::Replace));
}

void RstPad::SearchReplaceDlg::on_ReplaceAll_clicked()
{
    emit job(buildJob(SearchJob::ReplaceAll));
}

void RstPad::SearchReplaceDlg::on_Count_clicked()
{
    emit job(buildJob(SearchJob::Count));
}

void RstPad::SearchReplaceDlg::on_Cancel_clicked()
{
    close();
}
