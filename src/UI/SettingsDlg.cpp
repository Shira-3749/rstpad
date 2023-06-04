#include "SettingsDlg.h"
#include "ui_SettingsDlg.h"
#include "../App.h"
#include <Qt>
#include <QLineEdit>
#include <QTextOption>
#include <QCheckBox>
#include <QComboBox>

namespace RstPad {

    SettingsDlg::SettingsDlg(QWidget *parent) :
        QDialog(parent),
        ui(new Ui::SettingsDlg)
    {
        ui->setupUi(this);

        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        ui->Orientation->addItem(tr("Horizontal"), static_cast<int>(EditorOrientation::Horizontal));
        ui->Orientation->addItem(tr("Vertical"), static_cast<int>(EditorOrientation::Vertical));

        ui->AutoscrollMode->addItem(tr("First visible line"), static_cast<int>(AutoscrollMode::FirstLine));
        ui->AutoscrollMode->addItem(tr("Current line (cursor)"), static_cast<int>(AutoscrollMode::CurrentLine));
        ui->AutoscrollMode->addItem(tr("Disabled"), static_cast<int>(AutoscrollMode::Disabled));

        ui->WordWrapMode->addItem(tr("No wrap"), static_cast<int>(QTextOption::NoWrap));
        ui->WordWrapMode->addItem(tr("Word wrap"), static_cast<int>(QTextOption::WordWrap));
        ui->WordWrapMode->addItem(tr("Wrap anywhere"), static_cast<int>(QTextOption::WrapAnywhere));
        ui->WordWrapMode->addItem(tr("Wrap anywhere but prefer word boundary"), static_cast<int>(QTextOption::WrapAtWordBoundaryOrAnywhere));

        ui->Headings->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
        ui->Headings->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    }

    SettingsDlg::~SettingsDlg()
    {
        delete ui;
    }

    void SettingsDlg::showEvent(QShowEvent * e)
    {
        QDialog::showEvent(e);

        updateWidgets(APP->config()->all());
    }

    void SettingsDlg::updateWidgets(const QVariantMap &config)
    {
        ui->Orientation->setCurrentIndex(ui->Orientation->findData(config["orientation"]));
        ui->AutoscrollMode->setCurrentIndex(ui->AutoscrollMode->findData(config["autoscrollMode"]));
        ui->AutoscrollWaitDelay->setValue(config["autoscrollWaitDelay"].toInt());
        ui->RstRendererDelay->setValue(config["rstRendererDelay"].toInt());
        ui->PreviewZoomFactor->setValue(config["previewZoomFactor"].toDouble());
        ui->WordWrapMode->setCurrentIndex(ui->WordWrapMode->findData(config["wordWrapMode"]));
        ui->FontSize->setValue(config["fontSize"].toInt());
        ui->SearchWrapAround->setChecked(config["searchWrapAround"].toBool());
        ui->IndentSize->setValue(config["indentSize"].toInt());
        ui->EnsureSingleEmptyLineAtEndOnSave->setChecked(config["ensureSingleEmptyLineAtEndOnSave"].toBool());
        ui->TrimTrailingWhitespaceOnSave->setChecked(config["trimTrailingWhitespaceOnSave"].toBool());
        ui->HrSymbol->setText(config["hrSymbol"].toString());
        ui->HrWidth->setValue(config["hrWidth"].toInt());

        for (int i = 1; i <= 6; ++i) {
            auto row = i - 1;

            auto symbolWidget = new QLineEdit();
            symbolWidget->setMaxLength(1);
            symbolWidget->setText(config.value(QString("headingSymbol%1").arg(i)).toString());

            auto overlineWidget = new QCheckBox();
            overlineWidget->setChecked(config.value(QString("headingOverline%1").arg(i)).toBool());

            ui->Headings->setCellWidget(row, 0, symbolWidget);
            ui->Headings->setCellWidget(row, 1, overlineWidget);
        }
    }

    void SettingsDlg::updateConfig(Config &config)
    {
        QVariantMap values;

        values.insert("orientation", ui->Orientation->currentData());
        values.insert("autoscrollMode", ui->AutoscrollMode->currentData());
        values.insert("autoscrollWaitDelay", ui->AutoscrollWaitDelay->value());
        values.insert("rstRendererDelay", ui->RstRendererDelay->value());
        values.insert("previewZoomFactor", ui->PreviewZoomFactor->value());
        values.insert("wordWrapMode", ui->WordWrapMode->currentData());
        values.insert("fontSize", ui->FontSize->value());
        values.insert("searchWrapAround", ui->SearchWrapAround->isChecked());
        values.insert("indentSize", ui->IndentSize->value());
        values.insert("ensureSingleEmptyLineAtEndOnSave", ui->EnsureSingleEmptyLineAtEndOnSave->isChecked());
        values.insert("trimTrailingWhitespaceOnSave", ui->TrimTrailingWhitespaceOnSave->isChecked());
        values.insert("hrSymbol", ui->HrSymbol->text());
        values.insert("hrWidth", ui->HrWidth->text());

        for (int i = 1; i <= 6; ++i) {
            auto row = i - 1;
            values.insert(QString("headingSymbol%1").arg(i), static_cast<QLineEdit*>(ui->Headings->cellWidget(row, 0))->text());
            values.insert(QString("headingOverline%1").arg(i), static_cast<QCheckBox*>(ui->Headings->cellWidget(row, 1))->isChecked());
        }

        config.set(values);
    }

}

void RstPad::SettingsDlg::on_ButtonBox_clicked(QAbstractButton *button)
{
    auto buttonGroup = qobject_cast<QDialogButtonBox*>(sender());

    switch (buttonGroup->standardButton(button)) {
        case QDialogButtonBox::RestoreDefaults:
            updateWidgets(APP->config()->defaultValues());
            break;

        case QDialogButtonBox::Save:
            updateConfig(*APP->config());
            break;

        default:
            break;
    }
}
