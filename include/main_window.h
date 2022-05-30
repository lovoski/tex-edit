#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QDesktopWidget>
#include <QApplication>
#include <QCloseEvent>
#include <QMainWindow>
#include <QMenuBar>
#include <QThread>
#include <QAction>
#include <QMenu>

#include <iostream>

#include "include/socket_thread.h"
#include "include/conn_dialog.h"
#include "include/alert_dialog.h"

#include "3rdparty/unix/include/qt5/poppler-qt5.h"
#include "3rdparty/unix/include/Qsci/qsciscintilla.h"

#define WD_WIDHT 600
#define WD_HEIGHT 500

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connet_slots();
    void closeEvent(QCloseEvent *event);

signals:
    //void give_alert_msg(QString &msg);
    void window_closing();
public slots:
    void on_open();
    void on_save();
    void on_close();
    void on_connect();
    void on_disconnect();
    void m_on_compile_show();
    void m_on_compile_only();
    void on_help();
    void on_about();

    void processing_bef_window_closed();

    //void emit_alert_msg();

    // process information from the connection dialog
    void process_dialog_input(QString &msg);
    void process_cancel_input();
    // process alert dialog
    void close_alert_dialog();

private:

    bool sub_thread_running = 0;
    bool conn_stat = 0;

    SignalCollection *sig_col;
    SocketThread *s_thread;
    QThread *m_thread;

    QMenuBar *m_mB;
    QMenu *m_m1;
    QAction *open;
    QAction *save;
    QAction *close;

    QMenu *m_m2;
    QAction *connect;
    QAction *disconnect;

    QMenu *m_m3;
    QAction *comp_s;
    QAction *comp_o;

    QMenu *m_m4;
    QAction *help;
    QAction *about;

    QsciScintilla *m_editor;
    Conn_Dialog *dialog;
    //Alert_Dialog *a_dialog;
};
#endif // MAIN_WINDOW_H
