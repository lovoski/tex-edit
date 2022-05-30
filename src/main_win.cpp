#include "include/main_win.h"
#include "ui_widget.h"
#include <iostream>

Widget::Widget(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Widget)
{
    // setting up ui component
    ui->setupUi(this);

    setup_editor_part();

    // initialize all the pointers
    sock_thr = new SocketThread;
    sig_col = new SignalCollection;
    thr = new QThread;
    // setting up connection information

    std::string tmp_host = "127.0.0.1";

    sock_thr->set_connection_info(tmp_host.c_str(), "some_client", 5000);
    sock_thr->moveToThread(thr); //make sure the socket thread object execute inside sub_thread

    std::cout << "[client]:start setting up slots" << std::endl;

    // QObject::connect(ui->btn_1, &QPushButton::clicked, this, &Widget::print_msg_from_btnclick);
    QObject::connect(sig_col, &SignalCollection::start_sock_thread, sock_thr, &SocketThread::connect_to_server);
    // connect to server when pushing the button con
    QObject::connect(ui->con, &QPushButton::clicked, sig_col, &SignalCollection::emit_start_sig);
    QObject::connect(ui->con, &QPushButton::clicked, this, &Widget::start_sub_thread);
    // disconnect when pushing the button dis_con
    QObject::connect(ui->dis_con, &QPushButton::clicked, sock_thr, &SocketThread::close_connection);
    QObject::connect(ui->dis_con, &QPushButton::clicked, this, &Widget::quit_sub_thread);
    // when the window is closed, turn off the connection if there is any
    QObject::connect(this, &Widget::overloaded_closing_sig, this, &Widget::cleanup_bef_closing);

    std::cout << "[client]:setting up slots finished" << std::endl;
}

Widget::~Widget()
{
    delete ui;
    delete sig_col;
    delete thr;
    delete sock_thr;
}

void Widget::closeEvent(QCloseEvent *event)
{
    emit overloaded_closing_sig();
    event->accept();
}

void Widget::print_msg_from_btnclick()
{
    std::cout << "btn clicked!!" << std::endl;
}

void Widget::start_sub_thread()
{
    if (!this->sub_thread_running) {
        std::cout << "[client]:start sub-thread for socket" << std::endl;
        this->thr->start(); //start the sub_thread when specified signal is recieved
        this->sub_thread_running = 1;
    }
}

void Widget::cleanup_bef_closing()
{
    std::cout << "[client]:shutting down process" << std::endl;
    quit_sub_thread();
}

void Widget::quit_sub_thread()
{
    if (this->sub_thread_running) {
        std::cout << "[client]:closing sub-thread" << std::endl;
        this->thr->quit();
        this->thr->wait();
        this->sub_thread_running = 0;
    }
}

void Widget::setup_editor_part()
{
    this->editor = new QsciScintilla(this);
    editor->setGeometry(QRect(0,0,300,200));
}
