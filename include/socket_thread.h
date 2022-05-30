#ifndef SOCKET_THREAD_H
#define SOCKET_THREAD_H

#include <QObject>
#include <QThread>
#include <string>

class SignalCollection : public QObject
{
    Q_OBJECT
public:
    SignalCollection(QObject *parent = nullptr);
    ~SignalCollection();
signals:
    void start_sock_thread();
    void finish_sock_thread();
public slots:
    void emit_start_sig();
    void emit_finish_sig();
};

class SocketThread : public QObject
{
    Q_OBJECT
public:
    SocketThread(QObject *parent = nullptr);
    ~SocketThread();
    // setting up the information needed for connecting to server
    void set_connection_info(QString host, QString name_id, unsigned int port);

private:
    QString host;
    QString name_id;
    unsigned int port;

    int sockfd = 0;
    bool connected = 0;

public slots:
    // setting up basic behavior of a socket
    void connect_to_server();
    void close_connection();
};

#endif // SOCKET_THREAD_H
