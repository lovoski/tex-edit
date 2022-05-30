#ifndef ALERT_DIALOG_H
#define ALERT_DIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class Alert_Dialog;
}

class Alert_Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Alert_Dialog(QWidget *parent = nullptr);
    ~Alert_Dialog();
signals:
    void alert_confirm_clicked();
public slots:
    void trigger_alert_confirm_sig();
    void show_alert_info(QString &);
private:
    Ui::Alert_Dialog *ui;
};

#endif // ALERT_DIALOG_H
