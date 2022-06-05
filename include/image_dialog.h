#ifndef IMAGE_DIALOG_H
#define IMAGE_DIALOG_H

#include <QDialog>
#include <QLabel>

namespace Ui {
class image_dialog;
}

class image_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit image_dialog(QWidget *parent = nullptr);
    ~image_dialog();

    void add_image(QImage &img);
    void clear_image();

private:
    Ui::image_dialog *ui;
};

#endif // IMAGE_DIALOG_H
