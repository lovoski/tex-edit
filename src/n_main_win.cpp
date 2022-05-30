#include "include/n_main_win.h"
#include "ui_n_main_win.h"

n_main_win::n_main_win(QWidget *parent) : QMainWindow(parent), ui(new Ui::n_main_win)
{
    ui->setupUi(this);
    editor = new QsciScintilla(this);
    this->setCentralWidget(editor);
}

n_main_win::~n_main_win()
{
    delete ui;
    delete editor;
}
