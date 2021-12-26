#include "net.h"
#include <sys/stat.h>
#include "nettool.h"
#include "loadcontrol.h"

int main (void)
{
    MSG send_msg, recv_msg;
    int send_fd;
    int recv_fd;
    struct sockaddr_in sin;
    init_recv_param(&recv_fd, "235.10.10.3", 5004);
    init_send_param(&send_fd, &sin, "235.10.10.3", 5005);
    init_msg(&send_msg);

    pthread_t tid;
    pthread_t tid2;

    int filesize;

    pthread_create(&tid2, NULL, (void *)my_thread_find_process_IsRun, (void *)&send_msg); //创建线程

    char buf[BUFSIZ];
    char ipv4_addr[16];   
    struct sockaddr_in cin;
    socklen_t addrlen = sizeof (cin);
    mydebug ("\nmulticast demo started!\n");

    while (1) {
        bzero (buf, BUFSIZ);
        if (recvfrom (recv_fd, buf, BUFSIZ - 1, 0, (struct sockaddr *) &cin, &addrlen) < 0) {
            perror ("recvfrom");
            continue;
        }


        if (!inet_ntop (AF_INET, (void *) &cin.sin_addr, ipv4_addr, sizeof (cin))) {
            perror ("inet_ntop");
            exit (1);
        }

        //mydebug("Recived from(%s:%d), data:%s\n", ipv4_addr, ntohs (cin.sin_port), buf);

        memset(&recv_msg, 0, sizeof (recv_msg));
        memcpy(&recv_msg, buf, sizeof (recv_msg));
        data_handle(&tid, send_fd, sin, ipv4_addr, recv_msg, send_msg, &filesize);
    }
    close (send_fd);
    close (recv_fd);
    return 0;
} 

void my_thread(void *arg) //线程函数
{
    int res;
    int filesize = *(int *)arg;
    struct stat st;//定义结构体变量，保存所获取的文件属性
    while (1){
        res = stat("/bin/release.zip", &st);
        //res = stat("/home/forlinx/release.zip", &st);
        mydebug("this is thread %d  %d\n", filesize, st.st_size);

        if(res == -1)//获取文件属性失败，errno设置为合适的值
        {
            perror("stat fail");
            return;
        }

        if (filesize == st.st_size)
        {
            mydebug("上传成功");
            system("tar -xvf release.zip");
            system("chmod 744 /bin/audio");
            system("rm release.zip");
            system("reboot");
        }
        sleep(1);
    }

}

void my_thread_find_process_IsRun(void *arg) //线程函数
{

    MSG *msg = (MSG *)arg;

    while (1){
        if (msg->devstatus == reboot || msg->devstatus == update || msg->devstatus == changeip)
            continue;
        if (xx_process_IsRun("audio"))
        {
            msg->devstatus = back;
        }
        else
        {
            msg->devstatus = null;
        }
        sleep(1);
    }
}

void handle_null()
{

}

void handle_reboot(int send_fd, struct sockaddr_in sin, MSG send_msg)
{
    send_msg.devstatus = reboot;
    sendto (send_fd, &send_msg, sizeof (send_msg), 0, (struct sockaddr *) &sin, sizeof (sin));
    system("killall rcS");
    system("reboot");
}

int handle_update(pthread_t *tid, char *ipv4_addr, MSG recv_msg, int send_fd, struct sockaddr_in sin, MSG send_msg, int *filesize)
{
    char cmd[64];

    int ret = 0;
    send_msg.devstatus = update;
    sendto (send_fd, &send_msg, sizeof (send_msg), 0, (struct sockaddr *) &sin, sizeof (sin));
    system("killall audio");
    system("killall rcS");
    system("rm -rf /bin/audio");
    system("rm -rf /bin/3ss.pcm");
    sprintf(cmd, "tftp -g -r release.zip %s", ipv4_addr);
    system(cmd);
    *filesize = (u8)recv_msg.filesize[0] << 24 | \
                (u8)recv_msg.filesize[1] << 16 | \
                (u8)recv_msg.filesize[2] << 8 | \
                (u8)recv_msg.filesize[3];

    ret = pthread_create(tid, NULL, (void *)my_thread, (void *)filesize); //创建线程
    if(ret)
    {
        printf("create error\n");
        return 1;
    }
}

void handle_changeip(int send_fd, struct sockaddr_in sin, MSG send_msg, MSG recv_msg)
{
    char temvar[128]={0};
    send_msg.devstatus = changeip;
    sendto (send_fd, &send_msg, sizeof (send_msg), 0, (struct sockaddr *) &sin, sizeof (sin));
    if (!strncasecmp (send_msg.platform, "arm335x", strlen("arm335x")))
    {
        sprintf(temvar, "sed -i '/address/c address %s' /etc/network/interfaces", recv_msg.ip);
        system(temvar);mydebug("%s\n", temvar);
        sprintf(temvar, "sed -i '/netmask/c netmask %s' /etc/network/interfaces", recv_msg.mask);
        system(temvar);mydebug("%s\n", temvar);
        sprintf(temvar, "sed -i '/gateway/c gateway %s' /etc/network/interfaces", recv_msg.gateway);
        system(temvar);mydebug("%s\n", temvar);
        sprintf(temvar, "sed -i '/broadcast/c broadcast %s' /etc/network/interfaces", recv_msg.broadcast);
        system(temvar);
        mydebug("%s\n", temvar);
    }
    else
    {
        sprintf(temvar, "sed -i '/ifconfig/c ifconfig eth0 %s' /etc/init.d/rcS", recv_msg.ip);
        system(temvar);mydebug("%s\n", temvar);
        sprintf(temvar, "sed -i '/route/c route add default gw %s' /etc/init.d/rcS", recv_msg.gateway);
        system(temvar);mydebug("%s\n", temvar);
    }

    system("killall rcS");
    system("reboot");
}

void handle_call(int send_fd, struct sockaddr_in sin, MSG send_msg)
{
    send_msg.cmd = back;
    //mydebug("%d\n", sizeof(send_msg));
    sendto (send_fd, &send_msg, sizeof (send_msg), 0, (struct sockaddr *) &sin, sizeof (sin));
}

void handle_back()
{

}


int init_send_param(int *fd, struct sockaddr_in *sin, const char *ip, const int port)
{
    /* 1. 创建socket fd */
    if ((*fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {	//UDP编程
        perror ("socket");
        exit (1);
    }

    /*2.1 填充struct sockaddr_in结构体变量 */
    bzero (sin, sizeof (*sin));

    sin->sin_family = AF_INET;
    sin->sin_port = htons (port);	//网络字节序的端口号
#if 0
    sin->sin_addr.s_addr = inet_addr (ip);
#else
    if (inet_pton (AF_INET, ip, (void *) &sin->sin_addr) != 1) {
        perror ("inet_pton");
        exit (1);
    }
#endif

}

int init_recv_param(int *fd, const char *ip, const int port)
{
    struct sockaddr_in sin;
    /* 1. 创建socket fd */
    if ((*fd = socket (AF_INET, SOCK_DGRAM, 0)) < 0) {	//udp程序
        perror ("socket");
        exit (1);
    }

    /* 2. 允许绑定地址快速重用 */
    int b_reuse = 1;
    setsockopt (*fd, SOL_SOCKET, SO_REUSEADDR, &b_reuse, sizeof (int));

    /*加入多播组*/
    struct ip_mreq mreq;
    bzero(&mreq, sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    setsockopt(*fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,&mreq, sizeof(mreq));

    /*2. 绑定 */
    /*2.1 填充struct sockaddr_in结构体变量 */
    bzero (&sin, sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons (port);	//网络字节序的端口号

    /* 让服务器程序能绑定在任意的IP上 */
#if 1
    sin.sin_addr.s_addr = htonl (INADDR_ANY);
#else
    if (inet_pton (AF_INET, SERV_IP_ADDR, (void *) &sin.sin_addr) != 1) {
        perror ("inet_pton");
        exit (1);
    }
#endif
    /*2.2 绑定 */
    if (bind (*fd, (struct sockaddr *) &sin, sizeof (sin)) < 0) {
        perror ("bind");
        exit (1);
    }

}

void init_msg(MSG *msg)
{
    memset(msg->head, 0, sizeof (msg->head));
    memcpy(msg->head, "loadcontrol", strlen("loadcontrol"));

    msg->devname = 5;
    msg->devtype = 0;
    msg->devstatus = null;

    memset(msg->filesize, 0, sizeof (msg->filesize));

    memset(msg->platform, 0, sizeof (msg->platform));
    memcpy(msg->platform, PLATFORM, strlen(PLATFORM));

    msg->cmd = null;

    memset(msg->version, 0, sizeof (msg->version));
    memcpy(msg->version, "v20211212/00.01", strlen("v20211212/00.01"));

    memset(msg->ip, 0, sizeof (msg->ip));
    get_local_ip(IF_NAME, msg->ip);

    memset(msg->mac, 0, sizeof (msg->mac));
    get_local_mac(IF_NAME, msg->mac);

    memset(msg->mask, 0, sizeof (msg->mask));
    get_local_mask(IF_NAME, msg->mask);

    memset(msg->gateway, 0, sizeof (msg->gateway));
    get_gateWay(msg->gateway);

}

void data_handle(pthread_t *tid, int send_fd, struct sockaddr_in sin, char *ipv4_addr, MSG recv_msg, MSG send_msg, int *filesize)
{
    if ((!strncasecmp (recv_msg.head, HEAD_STR, strlen(HEAD_STR)) &&
        !strncasecmp (recv_msg.mac, send_msg.mac, strlen(send_msg.mac))) ||
        (recv_msg.cmd == call))
    {
        switch(recv_msg.cmd){
            case null:
                handle_null();
                break;
            case reboot:
                handle_reboot(send_fd, sin, send_msg);
                break;
            case update:
                handle_update(tid, ipv4_addr, recv_msg, send_fd, sin, send_msg, filesize);
                break;
            case changeip:
                handle_changeip(send_fd, sin, send_msg, recv_msg);
                break;
            case call:
                handle_call(send_fd, sin, send_msg);
                break;
            case back:
                break;
            case killapp:
                handle_killapp(send_fd, sin, send_msg);
                break;
        }
    }
}

int xx_process_IsRun(const char *proc)
{
    FILE* fp = NULL;
    int count = 0;
    char buf[100];
    char command[150];

    sprintf(command, "ps | grep -w %s | wc -l", proc);

    if((fp = popen(command,"r")) == NULL)
    {
        printf("popen err\r\n");
        return 0;
    }
    if((fgets(buf,100,fp))!= NULL)
    {
        count = atoi(buf);
    }

    pclose(fp);

    fp=NULL;

    if ((count - 2) == 0) {
        return 0;
    } else {
        return 1;
    }
}

void handle_killapp(int send_fd, struct sockaddr_in sin, MSG send_msg)
{
    send_msg.devstatus = killapp;
    sendto (send_fd, &send_msg, sizeof (send_msg), 0, (struct sockaddr *) &sin, sizeof (sin));
    system("killall start.sh");
    system("killall audio");
}
