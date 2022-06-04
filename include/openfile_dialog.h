#ifndef OPENFILE_DIALOG_H
#define OPENFILE_DIALOG_H

#include <QObject>
#include <QDialog>
#include <QString>
#include <iostream>
#include <QBitArray>
#include <QPushButton>
#include <QRadioButton>

namespace Ui {
class openfile_dialog;
}

class openfile_dialog : public QDialog
{
    Q_OBJECT

signals:
    void confirm_btn_clicked_signal(QString &file_name);
    void cancel_btn_clicked_signal();
    void create_file_signal(bool checked);
public slots:
    void emit_confirm_btn_clicked_signal();
    void emit_cancel_btn_clicked_signal();

public:
    explicit openfile_dialog(QWidget *parent = nullptr);
    ~openfile_dialog();

private:
    Ui::openfile_dialog *ui;

    QString file_name;
};

#endif // OPENFILE_DIALOG_H
