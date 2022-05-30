#include "include/conn_dialog.h"
#include "ui_conn_dialog.h"

Conn_Dialog::Conn_Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Conn_Dialog)
{
    ui->setupUi(this);
    QObject::connect(ui->confirm, &QPushButton::clicked, this, &Conn_Dialog::trigger_confirm_sig);
    QObject::connect(ui->cancel, &QPushButton::clicked, this, &Conn_Dialog::trigger_cancel_sig);
}

Conn_Dialog::~Conn_Dialog()
{
    delete ui;
}

void Conn_Dialog::trigger_confirm_sig()
{
    QString tmp = ui->lineEdit->text();
    ui->lineEdit->setText("");
    //std::cout << tmp.toStdString() << std::endl;
    emit confirm_clicked(tmp);
}

void Conn_Dialog::trigger_cancel_sig()
{
    emit cancel_clicked();
}
