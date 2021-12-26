#ifndef __MAKEU_NET_H__
#define __MAKEU_NET_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>			/* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>			/* superset of previous */


#include <netdb.h>  
#include <net/if.h>  
#include <arpa/inet.h>  
#include <sys/ioctl.h>  

#define SERV_PORT 5004
#define MULTICAST_IP "235.10.10.3"

#define QUIT_STR "quit"
#define HEAD_STR "loadcontrol"

#endif
