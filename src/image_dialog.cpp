#include "include/image_dialog.h"
#include "ui_image_dialog.h"

image_dialog::image_dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::image_dialog)
{
    ui->setupUi(this);
    ui->scrollArea->setAlignment(Qt::AlignCenter);
}

image_dialog::~image_dialog()
{
    delete ui;
}

void image_dialog::add_image(QImage &img)
{
    QLabel label(ui->scrollAreaWidgetContents);
    label.setScaledContents(true);
    label.setAlignment(Qt::AlignCenter);
    label.setPixmap(QPixmap::fromImage(img));
    ui->scrollArea->setWidget(&label);
}

void image_dialog::clear_image()
{

}
