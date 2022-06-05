#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H

#include <QObject>
#include <QThread>
#include <string>
#include <QBitArray>

class SignalCollection : public QObject
{
    Q_OBJECT
public:
    SignalCollection(QObject *parent = nullptr);
    ~SignalCollection();
signals:
    void start_sock_thread();
    void finish_sock_thread();
    void start_recv_thread_alone();
    
public slots:
    void emit_start_sig();
    void emit_finish_sig();
    void emit_start_recv_signal_alone();
    
};

class SocketThread : public QObject
{
    Q_OBJECT
public:
    SocketThread(QObject *parent = nullptr);
    ~SocketThread();
    // setting up the information needed for connecting to server

public:
    int sockfd = 0;
    bool connected = 0;

signals:
    void recv_add_string_msg(QString str);
    void recv_delete_string_msg(QString str);
    void recv_modified_string_msg(QString str);
    void recv_create_file_msg(QString str);
    void recv_open_file_msg(QString str);
    void recv_compiled_file_msg(QByteArray arr);

public slots:
    // setting up basic behavior of a socket

    /* the socket thread only handle the receiveing procedure */
    void start_listening_thread();
};

#endif // SOCKET_THREAD_H
