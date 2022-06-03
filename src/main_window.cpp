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
        // set socket to be non-blocking
        if ((sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) < 0) {
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
        int connect_times = 0;
        int rc = 0;
        while ((rc = ::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
            connect_times++;
            if (connect_times > 20) break;
        }
        if (rc < 0) {
            printf("failed after 20 attempts\n");
            this->m_thread->quit();
            this->m_thread->wait();
        } else { // rc=0 stands for success
            printf("connected to server, sockfd=%d\n", this->sockfd);
            this->connected = 1; // only by this far can the conection be secured to be valid

            /* tell the sub-thread what the file descipter is, setup dead loop */
            this->s_thread->sockfd = this->sockfd;
            this->s_thread->connected = true;
            this->sig_col->emit_start_recv_signal_alone(); // tell the recv thread start working
        }
    }
}
}
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    this->setFocusPolicy(Qt::StrongFocus);
    QWidget::setFocusPolicy(Qt::StrongFocus);
    this->main_tab = new QTabWidget(this);
    this->m_mB = new QMenuBar(this);
    this->m_editor = new QsciScintilla(this);
    this->m_thread = new QThread;
    this->sig_col = new SignalCollection;
    this->s_thread = new SocketThread;
    this->dialog = new Conn_Dialog;
    this->o_dialog = new openfile_dialog;
    //this->a_dialog = new Alert_Dialog;
    this->s_thread->moveToThread(this->m_thread);

    setup_editor(this->m_editor);
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
    o_remote = new QAction(tr("open remote"), this);
    add_tab = new QAction(tr("new tab"), this);
    save = new QAction(tr("save"), this);
    close = new QAction(tr("close"), this);
    connect = new QAction(tr("connect"), this);
    disconnect = new QAction(tr("disconnect"), this);
    comp_s = new QAction(tr("compile_show"), this);
    comp_o = new QAction(tr("compile_only"), this);
    help = new QAction(tr("help"), this);
    about = new QAction(tr("about"), this);

    connet_slots();

    m_m1->addAction(add_tab);
    m_m1->addAction(open);
    m_m1->addAction(o_remote);
    m_m1->addAction(save);
    m_m1->addAction(close);
    m_m2->addAction(connect);
    m_m2->addAction(disconnect);
    m_m3->addAction(comp_s);
    m_m3->addAction(comp_o);
    m_m4->addAction(help);
    m_m4->addAction(about);

    this->setMenuBar(m_mB);
    this->setCentralWidget(main_tab);
    main_tab->addTab(m_editor, "editor");

    //this->setCentralWidget(m_editor);
}

MainWindow::~MainWindow()
{
    //delete a_dialog;
    delete add_tab;
    delete main_tab;
    delete dialog;
    delete sig_col;
    delete s_thread;
    delete m_thread;
    delete o_dialog;
    delete m_mB;
    delete m_m1;
    delete m_m2;
    delete m_m3;
    delete m_m4;
    delete m_editor;
    delete open;
    delete o_remote;
    delete save;
    delete close;
    delete connect;
    delete disconnect;
    delete help;
    delete about;
    delete comp_s;
    delete comp_o;
}

void MainWindow::setup_editor(QsciScintilla *editor)
{
    //QsciLexerCPP *textLexer = new QsciLexerCPP;
    //m_editor->setLexer(textLexer);
    editor->setCaretForegroundColor(QColor(250,0,0));
    //m_editor->setCaretWidth(5);
    QFont dest_font;
    dest_font.setPointSize(13);
    editor->setFont(dest_font);
    editor->setCaretLineVisible(true);
    editor->setCaretLineBackgroundColor(QColor(200,200,200));
    editor->setMarginType(0, QsciScintilla::NumberMargin);
    editor->setMarginWidth(0, 30);

    // this needs to be abandoned due to severe problem
    // QObject::connect(m_editor, &QsciScintilla::cursorPositionChanged, this, &MainWindow::cursor_position_process);
    QObject::connect(this, &MainWindow::key_released_signal, this, &MainWindow::process_text_changed);
}

void MainWindow::connet_slots()
{
    QObject::connect(sig_col, &SignalCollection::start_sock_thread, this, &MainWindow::connect_to_server);
    // QObject::connect(sig_col, &SignalCollection::start_sock_thread, s_thread, &SocketThread::start_listening_thread);
    // the formal signal is sent only when the connection is ensured to be valid
    QObject::connect(sig_col, &SignalCollection::start_recv_thread_alone, s_thread, &SocketThread::start_listening_thread);
    QObject::connect(sig_col, &SignalCollection::finish_sock_thread, this, &MainWindow::close_connection);

    QObject::connect(open, &QAction::triggered, this, &MainWindow::on_open);
    QObject::connect(add_tab, &QAction::triggered, this, &MainWindow::on_add_new_tab);
    QObject::connect(o_remote, &QAction::triggered, this, &MainWindow::on_open_remote_local);
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

    QObject::connect(o_dialog, &openfile_dialog::confirm_btn_clicked_signal, this, &MainWindow::process_openfile_dialog_confirm);
    QObject::connect(o_dialog, &openfile_dialog::cancel_btn_clicked_signal, this, &MainWindow::process_openfile_dialog_cancel);

    // tell the mainwindow class to process the received message from other clients
    //QObject::connect(s_thread, &SocketThread::recv_add_string_msg, this, &MainWindow::process_recv_add_string_msg);
    //QObject::connect(s_thread, &SocketThread::recv_delete_string_msg, this, &MainWindow::process_recv_delete_string_msg);
    QObject::connect(s_thread, &SocketThread::recv_modified_string_msg, this, &MainWindow::process_recv_modified_string_msg);
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

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    // std::cout << "key pressed" << std::endl;
    emit key_pressed_signal();
    QWidget::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    // std::cout << "key released" << std::endl;
    emit key_released_signal();
    QWidget::keyReleaseEvent(event);
}

void MainWindow::process_text_changed()
{
    QString currrent_text = m_editor->text();
    if (currrent_text != bef_text) {
        QStringList cur_line_seg = currrent_text.split('\n');
        QStringList bef_line_seg = bef_text.split('\n');
        QString changed_lines = "", concate;
        int a_ln = cur_line_seg.length();
        int b_ln = bef_line_seg.length();
        if (a_ln == b_ln) { // modify, many lines can be changed, but the stays the same
            char head[4] = {'m', '\1', 'm', '\1'};
            int start = 0, end = a_ln-1;
            while (cur_line_seg[start] == bef_line_seg[start])
                start++; // index of the first changed line
            while (cur_line_seg[end] == bef_line_seg[end])
                end--; // index of the last changed line
            for (int i = start;i <= end;++i) {
                changed_lines = changed_lines.append(cur_line_seg[i]).append('\n');
            } // even the lines are '\n' themselves, '\n' is still needed
            QString len = QString::number(changed_lines.length()) + QString('\1');
            QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
            concate = QString(head) + pos_signature + len + changed_lines;
        } else if (a_ln > b_ln) { // add, find the added new and changed line
            char head[4] = {'m', '\1', 'a', '\1'};
            int start = 0, end = 0;
            while (start < b_ln-1 && (cur_line_seg[start] == bef_line_seg[start]))
                start++; // index of the first changed line
            while ((b_ln-1-end) > 0 && (cur_line_seg[a_ln-1-end] == bef_line_seg[b_ln-1-end]))
                end++; // index of the last changed line
            /* rule-1: stop when next is not available */
            int m_end = a_ln-1-end;
            if (start < m_end) { // replace
                for (int i = 0;i <= end;++i) {
                    changed_lines.append(cur_line_seg[i]).append('\n');
                }
                QString len = QString::number(changed_lines.length()) + QString('\1');
                QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
                concate = QString(head) + pos_signature + len + changed_lines;
            } else if (start == m_end) { // insert
                changed_lines = changed_lines.append(cur_line_seg[start]).append('\n');
                QString len = QString::number(changed_lines.length()) + QString('\1');
                QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
                concate = QString(head) + pos_signature + len + changed_lines;
            } else if (start > m_end) { // complex, no need for changed_lines
                QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
                concate = QString(head) + pos_signature + changed_lines;
            }
        } else if (a_ln < b_ln) { //delete, find the deleted and changed line
            char head[4] = {'m', '\1', 'd', '\1'};
            int start = 0, end = 0;
            while (start < a_ln-1 && (cur_line_seg[start] == bef_line_seg[start]))
                start++;
            while ((a_ln-1-end) > 0 && (cur_line_seg[a_ln-1-end] == bef_line_seg[b_ln-1-end]))
                end++;
            int m_end = b_ln-1-end;
            if (start > m_end) { // complex, no need for changed_lines
                QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
                concate = QString(head) + pos_signature + changed_lines;
            } else if(start == m_end) { // remove one line indexed start|end
                changed_lines = changed_lines.append(cur_line_seg[start]).append('\n');
                QString len = QString::number(changed_lines.length()) + QString('\1');
                QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
                concate = QString(head) + pos_signature + len + changed_lines;
            } else if (start < m_end) { // remove from start ot end, insert the line at start
                QString len = QString::number(changed_lines.length()) + QString('\1');
                QString pos_signature = QString::number(start) + QString('\1') + QString::number(end) + QString('\1');
                concate = QString(head) + pos_signature + len + changed_lines;
            } // keep in mind that end and start starts at 0, so the real value should -2
        }
        emit send_msg_to_server_request(concate); // emit signal with modifiyed msg
        bef_text = m_editor->text(); // update before text
    } // if no change happened
    
}

void MainWindow::process_recv_modified_string_msg(QString msg)
{
    std::cout << msg.toStdString() << std::endl; // the complex method
    QStringList list = msg.split('\1');
    int start = list[2].toInt();
    int end = list[3].toInt();

    std::cout << "start=" << start << ", end=" << end << std::endl;

    if (list[1] == QString('m')) {
        std::cout << "mod" << std::endl;
    } else if (list[1] == QString('a')) {
        std::cout << "add" << std::endl;
    } else if (list[1] == QString('d')) {
        std::cout << "del" << std::endl;
    }

    /* m_editor->setText(changed);
    bef_text = m_editor->text(); */
}

void __sep_length(const char *recv_buf, const int len)
{
    char num[50], sep_c = 0, num_i = 0;
    memset(num, 0, sizeof(num));
    for (int i = 0;i < len;++i) {
        if (recv_buf[i] == SEP_SYB) sep_c++;
        if (sep_c == 2) {
            num[num_i] = recv_buf[i];
            num_i++;
        } else if (sep_c > 2) break;
    } // the num[] is the length of whole information
}

void MainWindow::process_recv_add_string_msg(QString str)
{
    QStringList list = str.split('\1');
    QString content = list[3];
    int o_l = list[4].toInt(), o_i = list[5].toInt();
    // std::cout << "content:" << content.toStdString() << std::endl;
    // std::cout << o_l << " " << o_i << " " << a_l << " " << a_i << std::endl;
    m_editor->insertAt(content, o_l, o_i);
    this->bef_text = m_editor->text(); // reset the text before to make sure nothing went wrong
}

void MainWindow::process_recv_delete_string_msg(QString str)
{
    QStringList list = str.split('\1');
    int o_l = list[4].toInt(), o_i = list[5].toInt();
    int a_l = list[6].toInt(), a_i = list[7].toInt();
    int start = m_editor->positionFromLineIndex(o_l, o_i);
    int end = m_editor->positionFromLineIndex(a_l, a_i);
    QString cur_text = m_editor->text();
    QString seg1 = cur_text.mid(0, start -1);
    QString seg2 = cur_text.mid(end + 1, cur_text.length() - end - 1);
    m_editor->setText(seg1 + seg2);
    this->bef_text = m_editor->text(); // same as add string
}

void MainWindow::process_openfile_dialog_confirm(QString &file_name)
{
    QString tmp_file_name = file_name;
    if (tmp_file_name != "" && !tmp_file_name.isNull()) {
        // send_message_to_server_main_thread();
    }
}

void MainWindow::process_openfile_dialog_cancel()
{
    this->o_dialog->hide();
}

void MainWindow::on_add_new_tab()
{
    QsciScintilla *tmp = new QsciScintilla;
    main_tab->addTab(tmp, "new");
}

void MainWindow::on_open_remote_local()
{
    if (!this->connected) {
        QMessageBox::warning(this,tr("Error") , tr("connect to server first"));
    } else {
        // send_message_to_server_main_thread();
    }
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
        QMessageBox::information(this, tr("information"), tr("disconnecting from server"));
        printf("on disconnected, s_thread->connected=%d\n", s_thread->connected);
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

/* the two functions below should be forbiden */

void MainWindow::send_message_to_server_main_thread(QString &msg, char type)
{
    /* when msg is command for add string or delete string
    the msg contains the position of the original and latter position of cursor */
    QString tmp = msg;
    QString concate;
    char head[5] = {'m', 1, 0, 1};
    QString sep('\1');
    QString len = QString::number(tmp.length(), 10); // length of the real information

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
        /* construct the position of current and before cursor position */
        QString tail_b = QString('\1') + QString::number(cursor_line, 10) + QString('\1') + QString::number(cursor_index, 10);
        QString tail_a = QString('\1') + QString::number(l, 10) + QString('\1') + QString::number(r, 10);
        QString tail = tail_b + tail_a;
        // std::cout << "  bef:" << bef << "  aft:" << aft << std::endl;
        if (bef < aft) { // add char
            QString text = m_editor->text(bef, aft); // the added string
            //m_editor->insertAt("a", 1, 1);
            text = text + tail;
            send_message_to_server_main_thread(text, 'a');
            //std::cout << "  add:" << text.toStdString() << std::endl;
        } else { // delete char
            QString text = tell_diff(bef_text, aft_text);
            text = text + tail;
            send_message_to_server_main_thread(text, 'd');
            //std::cout << "  mdel:" << text.toStdString() << std::endl;
        }
    }
    this->cursor_line = l;
    this->cursor_index = r;
    this->bef_text = this->m_editor->text();
    return; // change the cursor position and return
}
