#include "include/main_window.h"
#include "include/Qsci/qscilexercpp.h"
#include "include/Qsci/qscilexerpython.h"

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
    this->s_thread->set_connection_info("", "client", 0);
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
    QObject::connect(sig_col, &SignalCollection::start_sock_thread, s_thread, &SocketThread::connect_to_server);
    QObject::connect(sig_col, &SignalCollection::finish_sock_thread, s_thread, &SocketThread::close_connection);

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
    if (conn_stat) { // judge the existence of connection
        QMessageBox msgBox;
        msgBox.setText(tr("disconnnected from server"));
        msgBox.exec();
        std::cout << "disconnect from server" << std::endl;
        this->s_thread->close_connection();
        this->m_thread->quit();
        this->m_thread->wait();
        this->conn_stat = 0;
        this->sub_thread_running = 0;
    }
}
void MainWindow::m_on_compile_show(){}
void MainWindow::m_on_compile_only(){}
void MainWindow::on_help(){}
void MainWindow::on_about(){}
void MainWindow::process_dialog_input(QString &msg)
{
    this->dialog->hide();
    std::cout << "processing dialog input" << std::endl;
    if (msg == "") {
        std::cout << "empty string" << std::endl;
    } else {
        std::cout << msg.toStdString() << std::endl;
        QStringList sL = msg.split(":");
        this->s_thread->set_connection_info(sL[0], "client", sL[1].toInt());
        // starting the sub-thread for socket connection
        if (sub_thread_running) {
            std::cout << "sub thread already running" << std::endl;
        } else {
            std::cout << "starting sub thread" << std::endl;
            this->m_thread->start();
            // tell the s_thread to start connecting
            this->sig_col->emit_start_sig();
            this->sub_thread_running = 1;
            this->conn_stat = 1;
        }
    }
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
        this->s_thread->close_connection();
        this->conn_stat = 0;
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
            //std::cout << "  add:" << text.toStdString() << std::endl;
        } else { // delete char
            QString text = tell_diff(bef_text, aft_text);
            //std::cout << "  mdel:" << text.toStdString() << std::endl;
        }
    }
    this->cursor_line = l;
    this->cursor_index = r;
    this->bef_text = this->m_editor->text();
    return; // change the cursor position and return
}
