
/************************************************************************/
/*	MOXA-CN2000  'ASPP' Port Control Program Example                */
/*	Program:							*/
/*	   example.c 							*/
/*	Purpose:							*/
/*	   This program will continuously send '1234567890'             */
/*	   string out from CN2000  RS-232 port, whose port Mode 	*/
/*	   should be set as 'ASPP', and read back any                   */
/*	   incoming data until program interrupted.			*/
/*	Setting:							*/
/*	   The target port will be configured as:			*/
/*		no parity,						*/
/*		8 data bits,						*/
/*		1 stop bit,						*/
/*		with software(XON/XOFF) flow control,			*/
/*		but no hardware(RTS/CTS).				*/
/*		1st port and 9600bps is default, however you may change */
/*		it in command line.					*/
/*	   The target port Mode should be set as 'ASPP'                 */
/*		via [Configure] [OP_mode] from Async Server		*/
/*		CONSOLE utility.					*/
/*	Syntax: 							*/
/*	   # ./aspp AsyncServerName [port(1) [Baud(9600)]]		*/
/*									*/
/*	   For example: 						*/
/*	      # ./aspp cn2000 1 19200					*/
/*	   The program will send '1234567890' to port 1 on Async        */
/*	   Server at 19200 bps baud and read back any data on it.	*/
/*									*/
/*	Environment:							*/
/*	   This program is originally developed under SCO UNIX. 	*/
/*	   Your environment may be different, if so, the include file	*/
/*	   name and other variable may need to be modified to suit	*/
/*	   your environment.						*/
/************************************************************************/

#include	<stdio.h>
#include	<fcntl.h>
#include	<sys/errno.h>

#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<arpa/telnet.h>
#include    <arpa/inet.h>

#include	"aspp.h"

int main(int argc, char * argv[])
{
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
/*
	printf("CN2000 IP addr = 0x%X(%s) \n", des.sin_addr.s_addr,
    	inet_ntoa(des.sin_addr.s_addr));
*/
	
	if ( argc > 2 )
		port = atoi(argv[2]);

	if ( argc > 3 ) {
		baud = atoi(argv[3]);
		switch( baud ){
		case 300:	baud = D_IOCTL_B300;	break;
		case 600:	baud = D_IOCTL_B600;	break;
		case 1200:	baud = D_IOCTL_B1200;	break;
		case 2400:	baud = D_IOCTL_B2400;	break;
		case 4800:	baud = D_IOCTL_B4800;	break;
		case 9600:	baud = D_IOCTL_B9600;	break;
		case 19200:	baud = D_IOCTL_B19200;	break;
		case 38400:	baud = D_IOCTL_B38400;	break;
		case 57600:	baud = D_IOCTL_B57600;	break;
		case 115200:	baud = D_IOCTL_B115200; break;
		case 230400:	baud = D_IOCTL_B230400; break;
		case 460800:	baud = D_IOCTL_B460800; break;
		default:	printf("invalide baud rate %d \n", baud);
				exit(0);
		}
		printf("\nport %d : %s,n,8,1,no RTS/CTS flow control.\n\n",
			port, argv[3]);
	} else {
		baud = D_IOCTL_B9600;
		printf("\nport %d : 9600,n,8,1,no RTS/CTS flow control.\n\n",
			port);
	}
	printf("Press any key to continue...\n");
	i = getchar();

	sio_init();

	if ( (fd = sio_open(des.sin_addr.s_addr, port)) < 0 ) {
		printf("\nPort %d open fail.\n", port);
		printf("\nRe-examine the port Mode from cn2000 CONSOLE.\n");
		exit(0);
	}
	printf("Port %2d opened OK.\n", port);

	if ( sio_ioctl(fd,baud,D_IOCTL_BIT8+D_IOCTL_STOP1+D_IOCTL_NONE) < 0 ) {
		printf("Port ioctl error.\n");
		sio_close(fd);
		exit(0);
	}
	printf("Port ioctl OK. \n");

	if ( sio_flowctrl(fd, 0) < 0 ) {
		printf("Port flow control error.\n");
		sio_close(fd);
		exit(0);
	}
	printf("Port flow control OK.\n");

	if ( sio_flush(fd, 2) < 0 ) {
		printf("Port flush error.\n");
		sio_close(fd);
		exit(0);
	}
	printf("Port flush OK.\n");

	i = sio_lstatus(fd);
	if ( i < 0 ) {
		printf("Port lstatus error!\n");
		sio_close(fd);
		exit(0);
	}
	printf("\n");
	if ( i & 1 )
		printf("DSR ON ");
	if ( i & 2 )
		printf("CTS ON ");
	if ( i & 4 )
		printf("DCD ON ");
	if ( (i & 7) == 0 )
		printf("LINE OFF ");
	printf("\n");

	if ( sio_write(fd, (char *) "1234567890", 10) != 10 ) {
		printf("write data [1234567890] fail!\n");
		sio_close(fd);
		exit(0);
	}
	printf("Waiting data [1234567890] from port %d .....\n", port);

	i = sio_read(fd, (char *) buf, 100);
	if ( i > 0 ) {
		buf[i] = 0;
		printf("Read: [%s]\n", buf);
	} else
		printf("read return %d !!!\n", i);
	sio_close(fd);
}
