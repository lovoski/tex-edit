#include "include/main_window.h"
#include "include/Qsci/qscilexercpp.h"
#include "include/Qsci/qscilexerpython.h"

extern "C"
{
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>

#define MBUF_SIZE   1024
#define SEP_SYB     1

void MainWindow::send_msg_to_server(QString msg)
{
    if (this->connected) {
        printf("mainwindow send\n");
        int len = msg.length();
        if (len > MBUF_SIZE) {
            printf("too long string\n");
        } else {
            char send_buf[MBUF_SIZE];
            memset(send_buf, 0, MBUF_SIZE);
            std::string tmp = msg.toStdString();
            for (int i = 0;i < len;++i) {
                send_buf[i] = tmp[i];
            }
            int n = ::write(this->sockfd, send_buf, MBUF_SIZE);
            if (n == -1) printf("mainwindow send error\n");
        }
        // ::write(this->sockfd, );
    } else {
        printf("mainwindow connected false\n");
    }
}

void MainWindow::close_connection()
{
    if (this->connected) {
        std::cout << "close mainwindow sockfd, s_thread connected set false" << std::endl;
        ::close(sockfd); // close the connection
        this->connected = false;
        this->s_thread->connected = false;
    }
}

/* hadling the connecting event, only connect to server */
void MainWindow::connect_to_server() // embed C code in c++ needs to specify :: namespace for C function
{
    if (!this->connected) {
        int sockfd = 0;
        struct sockaddr_in serv_addr;
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            printf("\n Error : Could not create socket \n");
            return;
        }
        this->sockfd = sockfd; // specify the conncetion in the class

        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(this->port);
        if (inet_pton(AF_INET, this->host.toStdString().c_str(), &serv_addr.sin_addr) <= 0) {
            printf("\n inet_pton error occured\n");
            return;
        }
        if (::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("\n Error : Connect Failed \n");
            return;
        }
        char tmp_send[1024];
        tmp_send[0] = 'm';
        ::write(this->sockfd, tmp_send, 1024); // first contact
        this->connected = 1; // only by this far can the conection be secured to be valid
        
        /* tell the sub-thread what the file descipter is, setup dead loop */
        this->s_thread->sockfd = this->sockfd;
        this->s_thread->connected = true;
        this->sig_col->emit_start_recv_signal_alone(); // tell the recv thread start working
    }
}
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->m_mB = new QMenuBar(this);
    this->m_editor = new QsciScintilla(this);
    this->m_thread = new QThread;
    this->sig_col = new SignalCollection;
    this->s_thread = new SocketThread;
    this->dialog = new Conn_Dialog;
    //this->a_dialog = new Alert_Dialog;
    this->s_thread->moveToThread(this->m_thread);

    setup_editor();
    bind_shortcut();

    this->resize(600, 500);
    this->move(QApplication::desktop()->width() * 0.5 - WD_WIDHT / 2
               , QApplication::desktop()->height() * 0.5 - WD_HEIGHT / 2);
    this->setWindowTitle(QString("tex-edit"));

    m_m1 = m_mB->addMenu(tr("File"));
    m_m2 = m_mB->addMenu(tr("Conn"));
    m_m3 = m_mB->addMenu(tr("Comp"));
    m_m4 = m_mB->addMenu(tr("Help"));

    open = new QAction(tr("open"), this);
    save = new QAction(tr("save"), this);
    close = new QAction(tr("close"), this);
    connect = new QAction(tr("connect"), this);
    disconnect = new QAction(tr("disconnect"), this);
    comp_s = new QAction(tr("compile_show"), this);
    comp_o = new QAction(tr("compile_only"), this);
    help = new QAction(tr("help"), this);
    about = new QAction(tr("about"), this);

    connet_slots();

    m_m1->addAction(open);
    m_m1->addAction(save);
    m_m1->addAction(close);
    m_m2->addAction(connect);
    m_m2->addAction(disconnect);
    m_m3->addAction(comp_s);
    m_m3->addAction(comp_o);
    m_m4->addAction(help);
    m_m4->addAction(about);

    this->setMenuBar(m_mB);
    this->setCentralWidget(m_editor);
}

MainWindow::~MainWindow()
{
    //delete a_dialog;
    delete dialog;
    delete sig_col;
    delete s_thread;
    delete m_thread;
    delete m_mB;
    delete m_m1;
    delete m_m2;
    delete m_m3;
    delete m_m4;
    delete m_editor;
    delete open;
    delete save;
    delete close;
    delete connect;
    delete disconnect;
    delete help;
    delete about;
    delete comp_s;
    delete comp_o;
}

void MainWindow::setup_editor()
{
    //QsciLexerCPP *textLexer = new QsciLexerCPP;
    //m_editor->setLexer(textLexer);
    m_editor->setCaretForegroundColor(QColor(250,0,0));
    //m_editor->setCaretWidth(5);
    QFont dest_font;
    dest_font.setPointSize(13);
    m_editor->setFont(dest_font);
    m_editor->setCaretLineVisible(true);
    m_editor->setCaretLineBackgroundColor(QColor(200,200,200));
    m_editor->setMarginType(0, QsciScintilla::NumberMargin);
    m_editor->setMarginWidth(0, 30);

    QObject::connect(m_editor, &QsciScintilla::cursorPositionChanged, this, &MainWindow::cursor_position_process);
}

void MainWindow::connet_slots()
{
    QObject::connect(sig_col, &SignalCollection::start_sock_thread, this, &MainWindow::connect_to_server);
    // QObject::connect(sig_col, &SignalCollection::start_sock_thread, s_thread, &SocketThread::start_listening_thread);
    // the formal signal is sent only when the connection is ensured to be valid
    QObject::connect(sig_col, &SignalCollection::start_recv_thread_alone, s_thread, &SocketThread::start_listening_thread);
    QObject::connect(sig_col, &SignalCollection::finish_sock_thread, this, &MainWindow::close_connection);

    QObject::connect(open, &QAction::triggered, this, &MainWindow::on_open);
    QObject::connect(save, &QAction::triggered, this, &MainWindow::on_save);
    QObject::connect(close, &QAction::triggered, this, &MainWindow::on_close);
    QObject::connect(close, &QAction::triggered, this, &QMainWindow::close);
    QObject::connect(connect, &QAction::triggered, this, &MainWindow::on_connect);
    QObject::connect(disconnect, &QAction::triggered, this, &MainWindow::on_disconnect);
    QObject::connect(comp_s, &QAction::triggered, this, &MainWindow::m_on_compile_show);
    QObject::connect(comp_o, &QAction::triggered, this, &MainWindow::m_on_compile_only);
    QObject::connect(help, &QAction::triggered, this, &MainWindow::on_help);
    QObject::connect(about, &QAction::triggered, this, &MainWindow::on_about);

    QObject::connect(dialog, &Conn_Dialog::confirm_clicked, this, &MainWindow::process_dialog_input);
    QObject::connect(dialog, &Conn_Dialog::cancel_clicked, this, &MainWindow::process_cancel_input);
    QObject::connect(this, &MainWindow::window_closing, this, &MainWindow::processing_bef_window_closed);

    // send message to server when action is detected
    QObject::connect(this, &MainWindow::send_msg_to_server_request, this, &MainWindow::send_msg_to_server);
    //QObject::connect(a_dialog, &Alert_Dialog::alert_confirm_clicked, this, &MainWindow::close_alert_dialog);
}

void MainWindow::bind_shortcut()
{
    QShortcut *sc1 = new QShortcut(QKeySequence("Ctrl+S"), this);
    sc1->setContext(Qt::ApplicationShortcut);
    QObject::connect(sc1, &QShortcut::activated, this, &MainWindow::on_save);
    QShortcut *sc2 = new QShortcut(QKeySequence("Ctrl+Q"), this);
    sc1->setContext(Qt::ApplicationShortcut);
    QObject::connect(sc2, &QShortcut::activated, this, &MainWindow::processing_bef_window_closed);
    QObject::connect(sc2, &QShortcut::activated, this, &MainWindow::on_close);
    QObject::connect(sc2, &QShortcut::activated, this, &QMainWindow::close);
}

void MainWindow::on_open()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("open file"),
                                                   "./", tr("All files (*.*)"));
    if (!file_name.isNull()) {
        this->selected_file = file_name;
        QFile opened_file(file_name);
        if (!opened_file.open(QFile::ReadOnly|QFile::Text)) {
            QMessageBox::warning(this,tr("Error") , tr("open file failed"));
        } else {
            QTextStream in(&opened_file);
            m_editor->setText(QString(in.readAll()));
        }
        opened_file.close();
    }
    //std::cout << open_file.fileName().toStdString() << std::endl;
}
void MainWindow::on_close()
{
    this->selected_file = "";
}
void MainWindow::on_save()
{
    if (this->selected_file == "") {
        // not an opened file
        QString file_name = QFileDialog::getSaveFileName(this, tr("save file"), "./");
        if (!file_name.isNull()) {
            this->selected_file = file_name;
            QFile save_file(file_name);
            if (!save_file.open(QFile::ReadWrite|QFile::Text)) {
                QMessageBox::warning(this,tr("Error") , tr("save file failed"));
            } else {
                QTextStream out(&save_file);
                QString output_text = m_editor->text();
                out << output_text;
                out.flush();
            }
            save_file.close();
        }
    } else {
        // an opened file
        QFile save_file(this->selected_file);
        if (!save_file.open(QFile::ReadWrite|QFile::Text)) {
            QMessageBox::warning(this,tr("Error") , tr("save file failed"));
        } else {
            QTextStream out(&save_file);
            QString output_text = m_editor->text();
            out << output_text;
            out.flush();
        }
        save_file.close();
    }
}
void MainWindow::on_connect()
{
    this->dialog->show();
}
void MainWindow::on_disconnect()
{
    if (this->connected) { // judge the existence of connection
        QMessageBox msgBox;
        msgBox.setText(tr("disconnnected from server"));
        msgBox.exec();
        std::cout << "disconnect from server" << std::endl;
        this->close_connection();
        this->s_thread->connected = false; // kill the dead loop
        this->m_thread->quit();
        this->m_thread->wait();
        this->connected = false;
        this->sub_thread_running = false;
    }
}
void MainWindow::m_on_compile_show(){}
void MainWindow::m_on_compile_only(){}
void MainWindow::on_help(){}
void MainWindow::on_about(){}

/* process info from the connection dialog */
void MainWindow::process_dialog_input(QString &msg)
{
    this->dialog->hide();
    std::cout << "processing dialog input" << std::endl;
    if (msg == "") {
        std::cout << "empty string" << std::endl;
    } else {
        std::cout << msg.toStdString() << std::endl;
        QStringList sL = msg.split(":");
        this->set_connection_info(sL[0], "client", sL[1].toInt()); // set connect info of MainWindow
        // starting the sub-thread for socket connection
        if (sub_thread_running) {
            std::cout << "sub thread already running" << std::endl;
        } else {
            std::cout << "starting sub thread" << std::endl;
            this->m_thread->start();
            // tell the sub-thread to start connecting
            this->sig_col->emit_start_sig();
            this->sub_thread_running = 1;
            this->connected = 1;
        }
    }
}

void MainWindow::set_connection_info(QString host, QString name_id, unsigned int port)
{
    this->host = host;
    this->name_id = name_id;
    this->port = port;
}

void MainWindow::process_cancel_input()
{
    this->dialog->hide();
}

void MainWindow::close_alert_dialog()
{
    //this->a_dialog->hide();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    emit window_closing();
    event->accept();
}

void MainWindow::processing_bef_window_closed()
{
    if (sub_thread_running) {
        this->close_connection();
        this->connected = 0;
        this->s_thread->connected = false; // stop the process using break in dead loop
        this->m_thread->quit();
        this->m_thread->wait();
        sub_thread_running = 0;
    }
}

QString tell_diff(QString &bef_line, QString &aft_line)
{
    int len = bef_line.length();
    std::string bef = bef_line.toStdString();
    std::string aft = aft_line.toStdString();
    char val[200];
    int index = 0, a = 0;
    for (int b = 0;b < len;b++) {
        if (bef[b] == aft[a]) {
            a++;
        } else {
            val[index] = bef[b];
            index++;
        }
    }
    val[index] = 0;
    return QString(val);
}

void MainWindow::send_message_to_server_main_thread(QString &msg, char type)
{
    QString tmp = msg;
    QString concate;
    char head[5] = {'m', 1, 0, 1};
    QString sep('\1');
    QString len = QString::number(tmp.length(), 10);

    if (type == 'a') { // add string
        head[2] = 'a';
        concate = QString(head) + len + sep + tmp;
    } else if (type == 'd') { // delete string
        head[2] = 'd';
        concate = QString(head) + len + sep + tmp;
    } else if (type == 'c') { // compile whole text
        head[2] = 'c';
        std::cout << "compile" << std::endl;
    } else if (type == 'o') { // open destinated text
        head[2] = 'o';
        std::cout << "open" << std::endl;
    } else {
        std::cout << "others" << std::endl;
    }
    // printf("length of concate msg: %d\n", concate.length());
    // std::cout << concate.toStdString() << std::endl;
    emit send_msg_to_server_request(concate); // emit signal with modifiyed msg
}

void MainWindow::cursor_position_process(int l, int r)
{
    QString aft_text = m_editor->text();
    if (bef_text == aft_text) { // only change the position of the cursor
        //std::cout << "only move cursor, selected text:" << m_editor->selectedText().toStdString() << std::endl;
    } else {
        int bef = m_editor->positionFromLineIndex(cursor_line, cursor_index);
        int aft = m_editor->positionFromLineIndex(l, r);
        std::cout << "  bef:" << bef << "  aft:" << aft << std::endl;
        if (bef < aft) { // add char
            QString text = m_editor->text(bef, aft); // the added string
            send_message_to_server_main_thread(text, 'a');
            //std::cout << "  add:" << text.toStdString() << std::endl;
        } else { // delete char
            QString text = tell_diff(bef_text, aft_text);
            send_message_to_server_main_thread(text, 'd');
            //std::cout << "  mdel:" << text.toStdString() << std::endl;
        }
    }
    this->cursor_line = l;
    this->cursor_index = r;
    this->bef_text = this->m_editor->text();
    return; // change the cursor position and return
}
