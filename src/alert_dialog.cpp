#include "include/alert_dialog.h"
#include "ui_alert_dialog.h"

Alert_Dialog::Alert_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Alert_Dialog)
{
    ui->setupUi(this);
    QObject::connect(ui->alert_confirm, &QPushButton::clicked, this, &Alert_Dialog::trigger_alert_confirm_sig);
}

Alert_Dialog::~Alert_Dialog()
{
    delete ui;
}

void Alert_Dialog::trigger_alert_confirm_sig()
{
    emit alert_confirm_clicked();
}

void Alert_Dialog::show_alert_info(QString &msg)
{
    ui->alert_info->setText(msg);
}
