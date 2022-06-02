#include "include/socket_thread.h"
#include <iostream>
#include <string>
#include <QString>

SocketThread::SocketThread(QObject *parent) : QObject(parent) {}
SocketThread::~SocketThread() {}
SignalCollection::SignalCollection(QObject *parent) : QObject(parent) {}
SignalCollection::~SignalCollection() {}

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

/* the function is only called when the connection is ensured to be valid */
void SocketThread::start_listening_thread()
{
    std::cout << "start listen from thread" << std::endl;
    char recv_buf[MBUF_SIZE];
    memset(recv_buf, 0, sizeof(recv_buf));
    int rc = 0;
    while (this->connected) { // wait and listen message from server
        // the sockfd is nonblocking
        rc = ::recv(this->sockfd, recv_buf, sizeof(recv_buf), 0);
        if (rc <= 0) {
            std::cout << "socket closed or error" << std::endl;
            break; // when the connection is dead
        } else {
            std::cout << recv_buf << std::endl;
        }
    }
    std::cout << "kill dead loop, finished receiving info" << std::endl;
}
}

void SignalCollection::emit_start_sig()
{
    emit start_sock_thread();
}

void SignalCollection::emit_finish_sig()
{
    emit finish_sock_thread();
}

void SignalCollection::emit_start_recv_signal_alone()
{
    emit start_recv_thread_alone();
}
