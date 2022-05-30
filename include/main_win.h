#ifndef MAIN_WIN_H
#define MAIN_WIN_H

#include <QWidget>
#include <QMainWindow>
#include <QCloseEvent>

#include "3rdparty/unix/include/Qsci/qsciscintilla.h"
#include "3rdparty/unix/include/Qsci/qsciapis.h"

#include "include/socket_thread.h"

#include "3rdparty/unix/include/qt5/poppler-qt5.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QMainWindow
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    void closeEvent(QCloseEvent* event);

    void setup_editor_part();

signals:
    void overloaded_closing_sig();

private slots:
    void print_msg_from_btnclick();

    void start_sub_thread();
    void quit_sub_thread();

    void cleanup_bef_closing();

private:
    bool sub_thread_running = 0;

    QsciScintilla *editor;

    Ui::Widget *ui;

    QFrame *frame;

    SignalCollection *sig_col;
    SocketThread *sock_thr;
    QThread *thr;
};
#endif // MAIN_WIN_H
