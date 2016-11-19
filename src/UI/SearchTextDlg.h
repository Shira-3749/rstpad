#ifndef RSTPAD_SEARCHTEXTDLG_H
#define RSTPAD_SEARCHTEXTDLG_H

#include <QDialog>

namespace RstPad {

namespace Ui {
class SearchTextDlg;
}

class SearchTextDlg : public QDialog
{
    Q_OBJECT

public:
    explicit SearchTextDlg(QWidget *parent = 0);
    ~SearchTextDlg();

private:
    Ui::SearchTextDlg *ui;
};


} // namespace RstPad
#endif // RSTPAD_SEARCHTEXTDLG_H
