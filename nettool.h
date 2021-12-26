#ifndef NETTOOL_H
#define NETTOOL_H

#define MAC_SIZE    18
#define IP_SIZE     16
#define IF_NAME "eth0"  //ens33

#if 1
    #define mydebug(format, ...)  printf("[%s:%d] "format"", __FILE__, __LINE__, ##__VA_ARGS__) ;fflush(stdout);
#else
    #define mydebug(format, ...) printf(format, ##__VA_ARGS__) ;
#endif

typedef unsigned char u8;

// function declare
int get_local_mac(const char *eth_inf, char *mac);  // 获取本机mac
int get_local_ip(const char *eth_inf, char *ip);    // 获取本机ip
void ip_mapping_location(const char *eth_inf, char *location);
void get_local_mask(const char *ifname, char *netmask_addr);
void get_gateWay(char *gateway);

#endif // NETTOOL_H
