#include "SearchTextDlg.h"
#include "ui_SearchTextDlg.h"

namespace RstPad {

SearchTextDlg::SearchTextDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SearchTextDlg)
{
    ui->setupUi(this);
}

SearchTextDlg::~SearchTextDlg()
{
    delete ui;
}

} // namespace RstPad
