#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#include "threadpool.h"
#include "str_utils.h"
#include "access_sql.h"

#include <pthread.h>

#define PORT_NUMBER 5001
#define MBUF_SIZE 1024
#define M_CLIENT 200
#define SEP_SYB 1     

typedef struct fd_s {int fd, index;} fd_s; // store only the fd

// store the clients in an array
static int client_fd[M_CLIENT];
static int client_count = 0;
// store all the threads
static pthread_t threads[M_CLIENT];

void *recv_client_task(void *args);
void start_tex_processs(char *content, char *name);
void operate_open(/* char *content, char *name */);
void operate_compile();

/* receive message from client and send the message to 
all clients ,and store the operate into the database */
void *recv_client_task(void *args)
{
    setbuf(stdout, NULL);
    printf("thread started\n");
    int rc = 0;
    int fd = ((fd_s *)args)->fd;
    int index = ((fd_s *)args)->index;
    char recv_buf[MBUF_SIZE];
    memset(recv_buf, 0, MBUF_SIZE);
    while (1) {
        rc = read(fd, recv_buf, sizeof(recv_buf)); // blocking io
        printf("looping rc=%d\n", rc);
        if (rc <= 0) { // error or client closed
            printf("close socket due to error or shutdown client");
            client_fd[index] = 0; // remove client from list
            client_count--;
            close(fd);
            break;
        } else {
            printf("%s\n", recv_buf);
            if ((recv_buf[0] == 'm') && (recv_buf[1] == SEP_SYB)) { // is the message head
                if (recv_buf[2] == 'a' || recv_buf[2] == 'd') { // add char or delete char
                    char cont_len[100];
                    for (int i = 4;i < MBUF_SIZE;++i) {
                        if (recv_buf[i] != SEP_SYB) cont_len[i - 4] = recv_buf[i];
                        cont_len[i - 3] = 0;
                    }
                    int cont_len_num = atoi(cont_len);
                    if (cont_len_num < 1000) {
                        int c_count = 1;
                        for (int i = 0;i < M_CLIENT;++i) {
                            if (c_count == client_count) break;
                            if (i == index) continue;
                            if (client_fd[i]) {
                                write(client_fd[i], recv_buf, sizeof(recv_buf));
                                c_count++;
                            }
                        }
                    } else printf("remain to be solved"); // if the message is longer than 1024 bytes
                } else if (recv_buf[2] == 'c') { // compile file
                    // operate_compile();
                } else if (recv_buf[2] == 'o') { // open file
                    // operate_open();
                } else if (recv_buf[2] == 'q') { // force quit
                    printf("quit as client socket required\n");
                    close(fd);
                    break;
                } else {
                    printf("undefined message format");
                }
            }
            
        }
    }
    return NULL;
}

void start_tex_processs(char *content, char *name)
{
    return;
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL); // make sure I know what I'm doing
    int listenfd = 0, connfd;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER);

    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    for (int i = 0;i < M_CLIENT;++i) { // join all the threads
        pthread_join(threads[i], NULL);
    }

    listen(listenfd, 10);
    while (1) { // start the thread forever
        printf("wait for connection\n");
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        printf("after connection\n");
        client_count++;
        // process the first recv
        char recv_buf[MBUF_SIZE];
        memset(recv_buf, 0, MBUF_SIZE);
        int n = recv(connfd, recv_buf, MBUF_SIZE, 0);
        if (n == -1) {
            printf("fail to read\n");
        } else {
            printf("%s\n", recv_buf);
            int f_non_zero = 0;
            while (client_fd[f_non_zero]) f_non_zero++; // find the first zero-free position
            if (recv_buf[0] == 'm') { // client
                client_fd[f_non_zero] = connfd;
                fd_s fd_s_t = {connfd, f_non_zero};
                pthread_create(&threads[f_non_zero], NULL, recv_client_task, (void *)(&fd_s_t));
            } else {
                printf("not a valid client connection\n");
            }
        }
    }
    close(listenfd);
    return 0;
}