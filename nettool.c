#include "nettool.h"
#include "net.h"

// 获取本机mac
int get_local_mac(const char *eth_inf, char *mac)
{
    struct ifreq ifr;
    int sd;

    bzero(&ifr, sizeof(struct ifreq));
    if( (sd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
      printf("get %s mac address socket creat error\n", eth_inf);
      return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, sizeof(ifr.ifr_name) - 1);

    if(ioctl(sd, SIOCGIFHWADDR, &ifr) < 0)
    {
      printf("get %s mac address error\n", eth_inf);
      close(sd);
      return -1;
    }

    snprintf(mac, MAC_SIZE, "%02x:%02x:%02x:%02x:%02x:%02x",
      (unsigned char)ifr.ifr_hwaddr.sa_data[0],
      (unsigned char)ifr.ifr_hwaddr.sa_data[1],
      (unsigned char)ifr.ifr_hwaddr.sa_data[2],
      (unsigned char)ifr.ifr_hwaddr.sa_data[3],
      (unsigned char)ifr.ifr_hwaddr.sa_data[4],
      (unsigned char)ifr.ifr_hwaddr.sa_data[5]);

    close(sd);

    return 0;
}


// 获取本机ip
int get_local_ip(const char *eth_inf, char *ip)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
     printf("socket error: %s\n", strerror(errno));
     return -1;
    }

    strncpy(ifr.ifr_name, eth_inf, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
     printf("ioctl error: %s\n", strerror(errno));
     close(sd);
     return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(ip, IP_SIZE, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}

void ip_mapping_location(const char *eth_inf, char *location)
{
    char ip[IP_SIZE];
    //const char *test_eth = "ens33";
    get_local_ip(eth_inf, ip);
    //printf("local %s ip: %s\n", test_eth, ip);

    char *token=strtok(ip,".");

    while(token!=NULL){
        memcpy(location , token, 3);
        //printf("%s\t",token);
        token=strtok(NULL,".");
    }
}

void get_local_mask(const char *ifname, char *netmask_addr)
{
    int sock_netmask;
    //char netmask_addr[16];
    struct ifreq ifr_mask;
    struct sockaddr_in *net_mask;

    sock_netmask = socket( AF_INET, SOCK_STREAM, 0 );
    if( sock_netmask == -1)
    {
        perror("create socket failture...GetLocalNetMask/n");
        return ;
    }


    memset(&ifr_mask, 0, sizeof(ifr_mask));
    strncpy(ifr_mask.ifr_name, ifname, sizeof(ifr_mask.ifr_name )-1);

    if( (ioctl( sock_netmask, SIOCGIFNETMASK, &ifr_mask ) ) < 0 )
    {
        printf("mac ioctl error/n");
        return ;
    }

    net_mask = ( struct sockaddr_in * )&( ifr_mask.ifr_netmask );
    strcpy( netmask_addr, inet_ntoa( net_mask -> sin_addr ) );

    printf("local netmask:%s\n",netmask_addr);

    close( sock_netmask );
}

void get_gateWay(char *gateway)
{
    FILE *fp;
    char buf[512];
    char cmd[128];
    char *tmp;

    strcpy(cmd, "ip route");
    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        perror("popen error");
        return ;
    }
    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        tmp =buf;
        while(*tmp && isspace(*tmp))
            ++ tmp;
        if(strncmp(tmp, "default", strlen("default")) == 0)
            break;
    }
    sscanf(buf, "%*s%*s%s", gateway);
    printf("default gateway:%s\n", gateway);
    pclose(fp);
}
