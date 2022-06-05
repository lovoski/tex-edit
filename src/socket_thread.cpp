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

#define MBUF_SIZE 4096
#define SEP_SYB     1

/* the function is only called when the connection is ensured to be valid */
void SocketThread::start_listening_thread()
{
    std::cout << "start listen from thread" << std::endl;
    char recv_buf[MBUF_SIZE];
    memset(recv_buf, 0, sizeof(recv_buf));
    ::setbuf(stdout, NULL);
    int rc = 0, wait_count = 0;

    QByteArray file;
    int send_time_counts = 0;

    while (this->connected) { // wait and listen message from server
        // the sockfd is nonblocking
        rc = ::read(this->sockfd, recv_buf, sizeof(recv_buf));
        if (rc <= 0) {
            if (rc == 0) {
                printf("break loop due to killed connection\n");
                break; // this means the connection is not available
            }
            ::usleep(500000); // sleep for 0.5s
            //printf("wait=%d, rc=%d, errno=%d\n",wait_count, rc, errno);
            wait_count++;
            // break; // when the connection is dead // this is non-blocking soccket, no need for break
        } else { // process the received message
            // std::cout << "recv msg: " << recv_buf << std::endl;
            if (recv_buf[0] == 'm' && recv_buf[1] == 1 && recv_buf[3] == 1) { // modify string
                QString tmp(recv_buf);
                emit recv_modified_string_msg(tmp);
            } else if (recv_buf[0] == '\1' && recv_buf[1] == '\1' && recv_buf[2] == '\1') { // created file
                QString tmp(recv_buf);
                emit recv_create_file_msg(tmp);
            } else if (recv_buf[0] == '\1' && recv_buf[1] == '\2' && recv_buf[2] == '\1') { // open file
                QString tmp(recv_buf);
                emit recv_open_file_msg(tmp);
            } else if (recv_buf[0]==3&&recv_buf[1]==3&&recv_buf[2]==3) {
                if (recv_buf[3]==3) {
                    std::cout << "in file" << std::endl;
                    file.append(recv_buf+4, MBUF_SIZE-4);
                    send_time_counts++;
                } else if(recv_buf[3]==4) { // end of file
                    send_time_counts++;
                    int lnz = MBUF_SIZE-1;
                    while (!recv_buf[lnz]) lnz--;
                    file.append(recv_buf+4, lnz-3);
                    emit recv_compiled_file_msg(file);
                    std::cout << "receive file finished, totally " << send_time_counts << " times" << std::endl; 
                    send_time_counts=0;
                    file.clear();
                }
            } else {
                printf("recv command undefined\n, recv_buf=%s\n", recv_buf);
            }
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
