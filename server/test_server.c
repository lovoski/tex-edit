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

static MYSQL *sql;

void *recv_client_task(void *args);
void start_tex_processs(char *content, char *name);
void operate_open(char *msg, int len);
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
    printf("newly connected client fd=%d, index=%d\n", fd, index);
    while (1) {
        rc = read(fd, recv_buf, sizeof(recv_buf)); // blocking io
        if (rc <= 0) { // error or client closed
            if (rc == 0) {
                printf("closed socket\n");
                client_fd[index] = 0; // remove client from list
                client_count--;
                close(fd);
                break;
            } // error
        } else {
            if ((recv_buf[0] == 'm') && (recv_buf[1] == SEP_SYB)) { // is the message head
                if (recv_buf[2] == 'm' || recv_buf[2] == 'a' || recv_buf[2] == 'd') { // add char or delete char
                    char cont_len[100];
                    char send_buf[MBUF_SIZE];
                    memcpy(send_buf, recv_buf, MBUF_SIZE);
                    // printf("0:recv_buf=%s\n", recv_buf);
                    for (int i = 4;i < MBUF_SIZE;++i) {
                        if (recv_buf[i] != SEP_SYB) {
                            cont_len[i - 4] = recv_buf[i];
                        }
                        cont_len[i - 3] = 0;
                    }
                    // printf("1:recv_buf=%s\n", recv_buf);
                    int cont_len_num = atoi(cont_len);
                    printf("content length :%d\n", cont_len_num);
                    if (cont_len_num < 1000) {
                        int c_count = 1;
                        // printf("2:recv_buf=%s\n", recv_buf);
                        for (int i = 0;i < M_CLIENT;++i) {
                            if (c_count == client_count) break;
                            if (i == index) continue;
                            if (client_fd[i]) {
                                int t_s = write(client_fd[i], send_buf, sizeof(send_buf));
                                printf("client fd=%d, client index=%d, send_buf=%s\n", client_fd[i], i, send_buf);
                                if (t_s == -1) printf("error writting to other clients\n");
                                c_count++;
                            }
                        }
                    } else printf("remain to be solved"); // if the message is longer than 1024 bytes
                } else if (recv_buf[2] == 'c') { // compile file
                    // operate_compile();
                } else if (recv_buf[2] == 'o') { // open file
                    operate_open(recv_buf, MBUF_SIZE);
                } else if (recv_buf[2] == 'q') { // force quit
                    printf("quit as client socket required\n");
                    close(fd);
                    break;
                } else {
                    printf("1:undefined message format\n");
                }
            } else {
                printf("2:undefined message format\n");
                client_fd[index] = 0; // remove client from list
                client_count--;
                close(fd);
                break;
            }
            
        }
    }
    return NULL;
}

void operate_open(char *msg, int len)
{
    int sep_c = 0;
    char file_name[100];
    memset(file_name, 0, sizeof(file_name));
    for (int i = 0;i < len;++i) {

    }
}

void start_tex_processs(char *content, char *name)
{
    return;
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL); // make sure I know what I'm doing

    //sql = initialize_mysql("localhost", "root", "123456", "tex_store");

    int listenfd = 0, connfd;
    struct sockaddr_in serv_addr;

    listenfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER);
    bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    for (int i = 0;i < M_CLIENT;++i) { // join all the threads
        pthread_join(threads[i], NULL);
    }

    listen(listenfd, 50);
    int loop_count = 0;
    while (1) { // start the thread forever
        // printf("wait for connection\n");
        connfd = accept(listenfd, (struct sockaddr *)NULL, NULL);
        if (connfd == -1) { // error
            sleep(2);
            printf("wait=%d, errno=%d\n", loop_count, errno);
            loop_count++;
        } else {
            printf("after connection\n");
            client_count++;
            int f_non_zero = 0;
            while (client_fd[f_non_zero]) f_non_zero++; // find the first zero-free position
            client_fd[f_non_zero] = connfd; // start task regardless of type
            fd_s fd_s_t = {connfd, f_non_zero};
            pthread_create(&threads[f_non_zero], NULL, recv_client_task, (void *)(&fd_s_t));
        }
        
    }
    close(listenfd);
    return 0;
}