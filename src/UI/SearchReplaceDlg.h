#ifndef RSTPAD_SEARCHREPLACEDLG_H
#define RSTPAD_SEARCHREPLACEDLG_H

#include <QDialog>
#include <QString>
#include "SearchJob.h"

namespace RstPad {

    namespace Ui {
        class SearchReplaceDlg;
    }

    class SearchReplaceDlg : public QDialog
    {
        Q_OBJECT

        public:
            explicit SearchReplaceDlg(QWidget *parent = 0);
            ~SearchReplaceDlg();

        signals:
            void job(const SearchJob &search);

        public slots:
            void show();

        private slots:
            void on_Find_clicked();
            void on_Replace_clicked();
            void on_ReplaceAll_clicked();
            void on_Count_clicked();
            void on_Cancel_clicked();

    private:
                Ui::SearchReplaceDlg *ui;
                SearchJob buildJob(SearchJob::Mode mode);
    };

}

#endif // RSTPAD_SEARCHREPLACEDLG_H
