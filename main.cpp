#include <iostream>
#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/errno.h>

#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<arpa/telnet.h>
#include    <arpa/inet.h>

#include	"protocol/aspp.h"

using namespace std;

int main() {
    int			fd, port, i, baud;
    unsigned long		ipaddr;
    struct hostent *	name;
    struct sockaddr_in	des;
    unsigned char		buf[100];

    port = 1;	/* target port = 1st port on cn2000  */
    if ( argc == 1 ) {
        printf("\nNo Async Server name specified.\n");
        printf("\nSyntax: # ./aspp ServerName [port_no [Speed]]\n");
        printf("For example: # ./aspp CN2000 1 9600\n\n");
        exit(0);
    }
    memset(&des, 0, sizeof(des));
    printf("Hook up a terminal to the first port to test.\n");
    name = gethostbyname(argv[1]);
    if ( !name ) {
        printf("\n%s must appear in '/etc/hosts'.\n",argv[1]);
        printf("Please add CN2000 IP addr and name into /etc/hosts.\n");
        exit(0);
    }
    strncpy((char *)&des.sin_addr, name->h_addr, name->h_length);
    printf("CN2000 IP addr = 0x%X(%s) \n", des.sin_addr.s_addr, inet_ntoa(des.sin_addr.s_addr) );

    return 0;
}