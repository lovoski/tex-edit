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

void SocketThread::connect_to_server()
{
    if (!this->connected) {
        std::cout << "socket connecting to server\n\thost:"
                  << this->host.toStdString()
                  << "\n\tport:"
                  << this->port
                  << std::endl;

        int sockfd = 0, n = 0;
        char recvBuff[1024];
        struct sockaddr_in serv_addr;
        memset(recvBuff, '0', sizeof(recvBuff));

        /* a socket is created through call to socket() function */
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Error : Could not create socket \n");
            return;
        }

        // specify the conncetion in the class
        this->sockfd = sockfd;

        memset(&serv_addr, '0', sizeof(serv_addr));

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(this->port);

        if (inet_pton(AF_INET, this->host.toStdString().c_str(), &serv_addr.sin_addr) <= 0)
        {
            printf("\n inet_pton error occured\n");
            return;
        }

        /* Information like IP address of the remote host and its port is
         * bundled up in a structure and a call to function connect() is made
         * which tries to connect this socket with the socket (IP address and port)
         * of the remote host
         */
        if (::connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\n Error : Connect Failed \n");
            return;
        }

        /* Once the sockets are connected, the server sends the data (date+time)
         * on clients socket through clients socket descriptor and client can read it
         * through normal read call on the its socket descriptor.
         */
        while ((n = read(sockfd, recvBuff, sizeof(recvBuff) - 1)) > 0)
        {
            recvBuff[n] = 0;
            if (fputs(recvBuff, stdout) == EOF)
            {
                printf("\n Error : Fputs error\n");
            }
        }

        if (n < 0)
        {
            printf("\n Read error \n");
        }

        this->connected = 1;
    }
}

void SocketThread::close_connection()
{
    if (this->connected) {
        std::cout << "socket disconnect from server" << std::endl;
        close(sockfd); // close the connection
        this->connected = 0;
    }
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

void SocketThread::set_connection_info(QString host, QString name_id, unsigned int port)
{
    this->host = host;
    this->name_id = name_id;
    this->port = port;
}
