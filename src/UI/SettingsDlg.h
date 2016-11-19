#ifndef RSTPAD_SETTINGSDLG_H
#define RSTPAD_SETTINGSDLG_H

#include "../App.h"
#include "../Config.h"
#include <QDialog>
#include <QShowEvent>
#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QVariantMap>

namespace RstPad {

    namespace Ui {
        class SettingsDlg;
    }

    class SettingsDlg : public QDialog
    {
        Q_OBJECT

        public:
            explicit SettingsDlg(QWidget *parent = 0);
            ~SettingsDlg();

        private slots:
            void on_ButtonBox_clicked(QAbstractButton *button);

        protected:
            void showEvent(QShowEvent* e);

        private:
            Ui::SettingsDlg *ui;
            void updateWidgets(const QVariantMap &config);
            void updateConfig(Config &config);
    };

}
#endif // RSTPAD_SETTINGS_H
