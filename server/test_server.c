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
#include <sys/stat.h>

#include "threadpool.h"
#include "str_utils.h"
#include "access_sql.h"

#include <pthread.h>

#define PORT_NUMBER 5001
#define MBUF_SIZE 4096
#define M_CLIENT 200
#define SEP_SYB 1     

typedef struct fd_s {int fd, index;} fd_s; // store only the fd

// store the clients in an array
static int client_fd[M_CLIENT];
static int client_editable_file[M_CLIENT];
static int client_count = 0;
// store all the threads
static pthread_t threads[M_CLIENT];

static MYSQL *sql;

void *recv_client_task(void *args);
void start_tex_processs(char *content, char *name);
void operate_open(char *msg, int len, int fd, int index);
void operate_compile(char *msg, int len, int fd, int index);
void operate_create_remote(char *msg, int len, int fd, int index);

void index_tex_file(MYSQL *sql)
{
    if (sql == NULL) {
        printf("sql pointer is NULL while indexing tex_file\n");
        return;
    }
}

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
                    printf("recv_buf=%s\n", recv_buf);
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
                    if (cont_len_num < M_CLIENT) {
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
                    operate_compile(recv_buf, MBUF_SIZE, fd, index);
                } else if (recv_buf[2] == 'o') { // open file
                    operate_open(recv_buf, MBUF_SIZE, fd, index);
                } else if (recv_buf[2] == 'r') { // create remote file
                    operate_create_remote(recv_buf, MBUF_SIZE, fd, index);
                } else if (recv_buf[2] == 'q') { // force quit
                    printf("quit as client socket required\n");
                    close(fd);
                    break;
                } else {
                    printf("1:undefined message format, msg=%s\n", recv_buf);
                }
            } else {
                printf("2:undefined message format, msg=%s\n", recv_buf);
                client_fd[index] = 0; // remove client from list
                client_count--;
                close(fd);
                break;
            }
        }
    }
    return NULL;
}

void operate_compile(char *msg, int len, int fd, int index)
{
    int a = rand();
    char *file_name = (char *)malloc(20);
    char *prefix = (char *)malloc(40);
    memset(file_name, 0, 20);
    memset(prefix, 0, 40);
    prefix[0]='.';prefix[1]='/';prefix[2]='s';prefix[3]='t';prefix[4]='o';prefix[5]='r';prefix[6]='e';prefix[7]='/';
    file_name = itostr(a, file_name);
    file_name = strconcat(file_name, ".tex");
    prefix = strconcat(prefix, file_name);
    printf("%s\n", prefix);
    printf("start compile process\n");
    FILE *out;
    out = fopen(prefix, "w");
    fputs(msg+4, out);
    fclose(out); // writes the file to local space
    char pre1[100] = "xelatex -output-directory=./store -interaction=nonstopmode ";
    char *c1 = strcat(pre1, prefix);

    struct stat stat_buf;
    int ret;
    ret = stat(prefix, &stat_buf);
    if(ret != 0) {
        printf("file doesn't exist\n");
    } else {
        int size = stat_buf.st_size;
        FILE *in;
        in = fopen(prefix, "rb");
        char send_buf[size];
        memset(send_buf, 0, size);
        fgets(send_buf, size, in);
        printf("reading file\n");
        int times = size/MBUF_SIZE + 1;
        for (int i = 0;i < times;++i) {
            send(fd, send_buf+times*MBUF_SIZE, MBUF_SIZE, 0);
        }
        printf("end sending\n");
    }
    system(c1);
    printf("compile finished\n");
    char pre2[100] = "rm -rf ";
    char *c2 = strcat(pre2, prefix);
    printf("removing generated file\n");
    system(c2); // remove generated file
    free(file_name);
    free(prefix);
}

/* query the data base for file named file_name
msg comes in hte form of {m,1,o,1}+file_name */
void operate_open(char *msg, int len, int fd, int index)
{
    int sep_c = 0;
    char file_name[100]; // maximum length to be 100
    memset(file_name, 0, sizeof(file_name));
    for (int i = 0;i < len;++i) {
        if (msg[i] == '\1') {sep_c++;i++;}
        if (sep_c == 2) {
            for (int j = i;j < len;++j) {
                file_name[j-i] = msg[j];
            }
            break;
        }
    }
    printf("open file request, file_name=%s\n", file_name);
    if (sql == NULL) {
        printf("sql pointer is NULL\n");
        return;
    }
    tex_file *file = (tex_file *)malloc(sizeof(tex_file));
    char *content = (char *)malloc(len);
    file->content = content;
    query_texfile_from_database(sql, file_name, file);
    if (file->len < 0) { // error or non-existence
        printf("error or file doesn't exist\n");
        return;
    } else { // success, send the file content back to client
        client_editable_file[index] = file->index; // set the client editing destinated file
        char send_buf[file->len + 4];
        memcpy(send_buf + 4, file->content, file->len);
        send_buf[0] = '\1';send_buf[1] = '\2';send_buf[2] = '\1';send_buf[3] = 's';send_buf[4]=0; // suceedd in opening file
        write(fd, send_buf, MBUF_SIZE);
        printf("send found file to client, data=%s\n", send_buf);
        free(send_buf);
    }
    free(file->content);
    free(file);
}

void operate_create_remote(char *msg, int len, int fd, int index)
{
    char file_name[100];
    int m_index = 0;
    memset(file_name, 0, sizeof(file_name));
    for (int i = 4;i < len;++i) {
        if (!msg[i]) break;
        file_name[m_index] = msg[i];
        m_index++;
    }
    printf("create remote file named: %s\n", file_name);
    if (sql == NULL) {
        printf("sql pointer is NULL\n");
        return;
    }
    insert_texfile_into_mysql(sql, file_name, ""); // create new file in database
    tex_file *file = (tex_file *)malloc(sizeof(tex_file));
    file = query_texfile_from_database(sql, file_name, file);
    client_editable_file[index] = file->index; // update client editing file to be created file index
    char send_head[MBUF_SIZE];
    memset(send_head, 0, sizeof(send_head));
    send_head[0] = '\1';send_head[1] = '\1';send_head[2] = '\1';send_head[3] = 's'; // create new file succeed
    write(fd, send_head, sizeof(send_head));
    free(file);
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL); // make sure I know what I'm doing

    sql = initialize_mysql("localhost", "root", "123456", "tex_store");

    index_tex_file(sql);

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
    free_wrapped_mysql(sql); // free the memory allocated, though never reachable
    return 0;
}