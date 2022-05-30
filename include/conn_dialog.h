#ifndef CONN_DIALOG_H
#define CONN_DIALOG_H

#include <QDialog>
#include <QString>

#include <iostream>

namespace Ui {
class Conn_Dialog;
}

class Conn_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Conn_Dialog(QWidget *parent = nullptr);
    ~Conn_Dialog();
signals:
    void confirm_clicked(QString &msg);
    void cancel_clicked();

public slots:
    void trigger_confirm_sig();
    void trigger_cancel_sig();

private:
    Ui::Conn_Dialog *ui;

    QString host_port;
};

#endif // CONN_DIALOG_H
