#ifndef LOADCONTROL_H
#define LOADCONTROL_H

#include <pthread.h> //线程头文件

#define PLATFORM "arm335x"

typedef enum {
    null,
    reboot,
    update,
    changeip,
    call,
    back,
    killapp
} CMD;

typedef struct {
    char head[32];
    char devname;
    char devtype;
    char devstatus;
    char filesize[4];
    char platform[16];
    CMD  cmd;
    char version[32];
    char ip[16];
    char mac[18];
    char mask[16];
    char gateway[16];
    char broadcast[16];

} MSG;

void my_thread(void *arg);                         //线程函数
void my_thread_find_process_IsRun(void *arg);    //线程函数
int init_network_param(int *fd, struct sockaddr_in *sin);

int init_send_param(int *fd, struct sockaddr_in *sin, const char *ip, const int port);
int init_recv_param(int *fd, const char *ip, const int port);

void init_msg(MSG *msg);

void data_handle(pthread_t *tid, int send_fd, struct sockaddr_in sin, char *ipv4_addr, MSG recv_msg, MSG send_msg, int *filesize);
void handle_null();
void handle_reboot(int send_fd, struct sockaddr_in sin, MSG send_msg);
int handle_update(pthread_t *tid, char *ipv4_addr, MSG recv_msg, int send_fd, struct sockaddr_in sin, MSG send_msg, int *filesize);
void handle_call(int send_fd, struct sockaddr_in sin, MSG send_msg);
void handle_killapp(int send_fd, struct sockaddr_in sin, MSG send_msg);

void handle_changeip(int send_fd, struct sockaddr_in sin, MSG send_msg,  MSG recv_msg);
int xx_process_IsRun(const char *proc);

#endif // LOADCONTROL_H
