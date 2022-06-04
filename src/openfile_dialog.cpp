#include "include/openfile_dialog.h"
#include "ui_openfile_dialog.h"

openfile_dialog::openfile_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::openfile_dialog)
{
    ui->setupUi(this);
    QObject::connect(ui->confirm, &QPushButton::clicked, this, &openfile_dialog::emit_confirm_btn_clicked_signal);
    QObject::connect(ui->cancel, &QPushButton::clicked, this, &openfile_dialog::emit_confirm_btn_clicked_signal);
    QObject::connect(ui->radioButton, &QRadioButton::toggled, [=](bool checked){
        emit create_file_signal(checked);
    });
}

openfile_dialog::~openfile_dialog()
{
    delete ui;
}

void openfile_dialog::emit_confirm_btn_clicked_signal()
{
    QString file_name = ui->lineEdit->text();
    ui->lineEdit->setText("");
    emit confirm_btn_clicked_signal(file_name);
}
void openfile_dialog::emit_cancel_btn_clicked_signal()
{
    emit cancel_btn_clicked_signal();
}
