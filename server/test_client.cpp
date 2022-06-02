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

#define PORT_NUMBER 5001
#define MBUF_SIZE   1024
#define HOST_IP     "127.0.0.1"
#define SEP_SYB     222

int main(int argc, char *argv[])
{
    int sockfd = 0;
    struct sockaddr_in serv_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Error : Could not create socket \n");
        return 1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_NUMBER);

    // connect to destinated server
    if (inet_pton(AF_INET, HOST_IP, &serv_addr.sin_addr) <= 0)
    {
        printf("\n inet_pton error occured\n");
        return 1;
    }
    // initialize finished

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\n Error : Connect Failed \n");
        return 1;
    }
    // printf("Connected to server\n");
    char send_buf[1024];
    // memset(send_buf, 0, 1024);
    send_buf[0] = 'm';
    send_buf[1] = rand() % 40 + 20;
    send_buf[2] = 0;
    int n = write(sockfd, send_buf, sizeof(send_buf));
    n = write(sockfd, send_buf, sizeof(send_buf));
    // printf("%s\n", send_buf);
    getchar();
    close(sockfd);
    // getchar();

    return 0;
}