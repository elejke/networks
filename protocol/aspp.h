
/********************************************************/
/* File name:	aspp.h					*/
/* Version:	2.0					*/
/********************************************************/

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<netdb.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<time.h>
#include	<sys/time.h>

#define 	DFD_TABLE_SIZE	128	/* Max. async ports to be control */

#define 	DCMD_IOCTL	16	/* IOCTL command	       */
#define 	DCMD_FLOWCTRL	17	/* Flow control command        */
#define 	DCMD_LINECTRL	18	/* Line status control command */
#define 	DCMD_LSTATUS	19	/* Line status read command    */
#define 	DCMD_FLUSH	20	/* Buffer flush command        */
#define 	DCMD_IQUEUE	21	/* Input queue command	       */
#define 	DCMD_OQUEUE	22	/* Output queue command        */
#define 	DCMD_BAUDRATE	23	/* Set port's baud rate command */
#define 	DCMD_BREAK	35	/* Send break in mini-seconds  */

/* parameters for D_COMMAND_IOCTL ioctl command */
#define 	D_IOCTL_B300	0	/* IOCTL : baud rate = 300 bps	 */
#define 	D_IOCTL_B600	1	/* IOCTL : baud rate = 600 bps	 */
#define 	D_IOCTL_B1200	2	/* IOCTL : baud rate = 1200 bps  */
#define 	D_IOCTL_B2400	3	/* IOCTL : baud rate = 2400 bps  */
#define 	D_IOCTL_B4800	4	/* IOCTL : baud rate = 4800 bps  */
#define 	D_IOCTL_B7200	5	/* IOCTL : baud rate = 7200 bps  */
#define 	D_IOCTL_B9600	6	/* IOCTL : baud rate = 9600 bps  */
#define 	D_IOCTL_B19200	7	/* IOCTL : baud rate = 19200 bps */
#define 	D_IOCTL_B38400	8	/* IOCTL : baud rate = 38400 bps */
#define 	D_IOCTL_B57600	9	/* IOCTL : baud rate = 57600 bps */
#define 	D_IOCTL_B115200 10	/* IOCTL : baud rate = 115200 bps */
#define 	D_IOCTL_B230400 11	/* IOCTL : baud rate = 230400 bps */
#define 	D_IOCTL_B460800 12	/* IOCTL : baud rate = 460800 bps */
#define 	D_IOCTL_B921600 13	/* IOCTL : baud rate = 921600 bps */
#define 	D_IOCTL_150	14	/* IOCTL : baud rate = 150 bps */
#define 	D_IOCTL_134	15	/* IOCTL : baud rate = 134.5 bps */
#define 	D_IOCTL_110	16	/* IOCTL : baud rate = 110 bps */
#define 	D_IOCTL_75	17	/* IOCTL : baud rate = 75 bps */
#define 	D_IOCTL_50	18	/* IOCTL : baud rate = 50 bps */

#define 	D_IOCTL_BIT8	3	/* IOCTL : 8 data bits */
#define 	D_IOCTL_BIT7	2	/* IOCTL : 7 data bits */
#define 	D_IOCTL_BIT6	1	/* IOCTL : 6 data bits */
#define 	D_IOCTL_BIT5	0	/* IOCTL : 5 data bits */

#define 	D_IOCTL_STOP1	0	/* IOCTL : 1 stop bit  */
#define 	D_IOCTL_STOP2	4	/* IOCTL : 2 stop bits */

#define 	D_IOCTL_EVEN	8	/* IOCTL : even parity */
#define 	D_IOCTL_ODD	16	/* IOCTL : odd parity  */
#define 	D_IOCTL_NONE	0	/* IOCTL : none parity */

/*
 *	Global functions:
 */
int	sio_init();
int	sio_open(unsigned long server_ip, int port);
int	sio_close(int fd);
int	sio_ioctl(int fd, int baud, int mode);
int	sio_baud(int fd, long baud);
int	sio_flowctrl(int fd, int mode);
int	sio_lctrl(int fd, int mode);
int	sio_lstatus(int fd);
int	sio_flush(int fd, int func);
int	sio_write(int fd, char *buf, int len);
int	sio_read(int fd, char *buf, int len);
int	sio_break(int fd, int time);
int	sio_oqueue(int fd);
int	sio_iqueue(int fd);

/*
 *	Local static functions:
 */
static int	SioOpen(unsigned long ipaddr, int tcpport);
static int	SioWaitAck(int fd, char cmd);
static int	SioCmdportFd(int fd);
static int	SioAddFdtbl(int fd, int cmdfd);
static int	SioDelFdtbl(int fd);

struct aspp_fds {
	int	fd_data;
	int	fd_cmd;
};

static struct aspp_fds	asfds[DFD_TABLE_SIZE];

/******************************************************************************
 *	Initial data table using of ASPP functions			      *
 *	This function must be called once before calling any other sio_open,  *
 *	sio_close.., etc.						      *
 *									      *
 *	Arg:	none							      *
 ******************************************************************************/
int	sio_init()
{
	int	i;

	for ( i=0; i<DFD_TABLE_SIZE; i++ ) {
		asfds[i].fd_data = -1;
		asfds[i].fd_cmd = -1;
	}
	return(0);
}

/******************************************************************************
 *	open Async Server serial port					      *
 *									      *
 *	Args:	unsigned long	ipaddr -  IP address of Async Server	      *
 *					  e.g. 0x0102A8C0 -> 192.168.2.1      *
 *		int		portno -  serial port number of Async Server  *
 *					  number range from 1 to 16.	      *
 *	Return: >= 0	:	file handle ID (open success).		      *
 *		  -1	:	open fail.				      *
 ******************************************************************************/
int	sio_open(unsigned long ipaddr, int portno)
{
	int	fd, fd2;

	if ( portno < 1 || portno > 16 )
		return(-1);
	portno--;
	if ( (fd = SioOpen(ipaddr, portno + 950)) < 0 )
		return(-1);
	if ( (fd2 = SioOpen(ipaddr, portno + 966)) < 0 ) {
		close(fd);
		return(-1);
	}
	if ( SioAddFdtbl(fd, fd2) != fd2 ) {
		close(fd2);
		close(fd);
		return(-1);
	}
	return(fd);
}

/******************************************************************************
 *	close Async Server serial port					      *
 *									      *
 *	Arg:	int	fd	-	file handle ID return from sio_open() *
 ******************************************************************************/
int	sio_close(int fd)
{
	close(SioCmdportFd(fd));
	close(fd);
	SioDelFdtbl(fd);
	return(0);
}

/******************************************************************************
 *	Set Async port baud rate and character mode			      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		int	baud	-	0: 300	    1: 600	2: 1200       *
 *					3: 2400     4: 4800	5: 7200       *
 *					6: 9600     7: 19200	8: 38400      *
 *					9: 57600    10: 115200	11: 230400    *
 *					12: 460800  13: 921600	14: 150       *
 *					15: 134     16: 110	17: 75	      *
 *					18: 50				      *
 *		int	mode	- (bit 0,1)	0x00	-	5 data bits   *
 *						0x01	-	6 data bits   *
 *						0x10	-	7 data bits   *
 *						0x11	-	8 data bits   *
 *				  (bit 2)	0x00	-	1 stop bit    *
 *						0x04	-   1.5/2 stop bits   *
 *				  (bit 3,4,5)	0x00	-	none parity   *
 *						0x08	-	odd parity    *
 *						0x18	-	even parity   *
 *						0x28	-	mark parity   *
 *						0x38	-	space parity  *
 *									      *
 *	Return:    0	:	O.K.					      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_ioctl(int fd, int baud, int mode)
{
	unsigned char	buf[4];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_IOCTL;
	buf[1] = 2;
	buf[2] = (unsigned char)baud;
	buf[3] = (unsigned char)mode;
	if ( write(fd, buf, 4) != 4 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Set Async Port baud rate					      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		long	baud	-	desired baud rate		      *
 *									      *
 *	Return:    0	:	O.K.					      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_baud(int fd, long baud)
{
	unsigned char	buf[8];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_BAUDRATE;
	buf[1] = 4;
	*(long *)(&buf[2]) = baud;
	if ( write(fd, buf, 6) != 6 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Set Async Port flow control					      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		int	mode	-	bit 0:	CTS flow control	      *
 *					bit 1:	RTS flow control	      *
 *					bit 2:	Tx XON/XOFF flow control      *
 *					bit 3:	Rx XON/XOFF flow control      *
 *					(0 = OFF,  1 = ON)		      *
 *									      *
 *	Return:    0	:	O.K.					      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_flowctrl(int fd, int mode)
{
	unsigned char	buf[8];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_BAUDRATE;
	buf[1] = 4;
	if ( mode & 1 )
		buf[2] = 1;
	else
		buf[2] = 0;
	if ( mode & 2 )
		buf[3] = 1;
	else
		buf[3] = 0;
	if ( mode & 4 )
		buf[4] = 1;
	else
		buf[4] = 0;
	if ( mode & 8 )
		buf[5] = 1;
	else
		buf[5] = 0;
	if ( write(fd, buf, 6) != 6 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Set Async Port line control					      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		int	mode	-	bit 0:	DTR output state	      *
 *					bit 1:	RTS output state	      *
 *					(0 = OFF,  1 = ON)		      *
 *									      *
 *	Return:    0	:	O.K.					      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_lctrl(int fd, int mode)
{
	unsigned char	buf[8];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_LINECTRL;
	buf[1] = 2;
	if ( mode & 1 )
		buf[2] = 1;
	else
		buf[2] = 0;
	if ( mode & 2 )
		buf[3] = 1;
	else
		buf[3] = 0;
	if ( write(fd, buf, 4) != 4 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Get Async Port line status					      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *									      *
 *	Return:  >=0	:	line status:	bit 0: DSR, bit 1: CTS,       *
 *						bit 2: DCD		      *
 *						(1 - ON, 0 - OFF)	      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_lstatus(int fd)
{
	unsigned char	buf[4];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_LSTATUS;
	buf[1] = 0;
	if ( write(fd, buf, 2) != 2 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Flush input/output buffer data					      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		int	func	-	0 :	flush input buffer	      *
 *					1 :	flush output buffer	      *
 *					2 :	flush input/output buffer     *
 *									      *
 *	Return: >= 0	:	O.K.					      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_flush(int fd, int func)
{
	unsigned char	buf[4];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_FLUSH;
	buf[1] = 1;
	buf[2] = func;
	if ( write(fd, buf, 3) != 3 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	write data to Async Server serial port				      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		char *	buf	-	write out data buffer pointer	      *
 *		int	len	-	write out data length		      *
 *									      *
 *	Return: >= 0	:	write out data length			      *
 *		  -1	:	write error				      *
 ******************************************************************************/
int	sio_write(int fd, char *buf, int len)
{
	return( write(fd, buf, len) );
}

/******************************************************************************
 *	read data from Async Server serial port 			      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		char *	buf	-	read data buffer pointer	      *
 *		int	len	-	read data buffer length 	      *
 *									      *
 *	Return: >= 0	:	read data length			      *
 *		  -1	:	read error				      *
 ******************************************************************************/
int	sio_read(int fd, char *buf, int len)
{
	return( read(fd, buf, len) );
}

/******************************************************************************
 *	Send out a break signal 					      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *		int	time	-	break time in mini-second	      *
 *									      *
 *	Return:    0	:	O.K.					      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_break(int fd, int time)
{
	unsigned char	buf[8];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_BREAK;
	buf[1] = 2;
	*(unsigned short *)&buf[2] = ntohs(time);
	if ( write(fd, buf, 4) != 4 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Get output buffer data queue size				      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *									      *
 *	Return: >= 0	:	output buffer data queue size		      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_oqueue(int fd)
{
	unsigned char	buf[4];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_OQUEUE;
	buf[1] = 0;
	if ( write(fd, buf, 2) != 2 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	Get input buffer data queue size				      *
 *									      *
 *	Args:	int	fd	-	file handle ID return from sio_open() *
 *									      *
 *	Return: >= 0	:	input buffer data queue size		      *
 *		  -1	:	error					      *
 ******************************************************************************/
int	sio_iqueue(int fd)
{
	unsigned char	buf[4];

	if ( (fd = SioCmdportFd(fd)) == -1 )
		return(-1);
	buf[0] = DCMD_IQUEUE;
	buf[1] = 0;
	if ( write(fd, buf, 2) != 2 )
		return(-1);
	return( SioWaitAck(fd, buf[0]) );
}

/******************************************************************************
 *	ASPP protocol control sub-functions				      *
 ******************************************************************************/
static int SioOpen(unsigned long ipaddr, int tcpport)
{
	struct sockaddr_in	des;
	int			fd, len;

	if ( (fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		return(-1);
	memset(&des, 0, sizeof(des));
	des.sin_family = AF_INET;
	des.sin_addr.s_addr = ipaddr;
	des.sin_port = htons(tcpport);
	len = sizeof(struct sockaddr_in);
	if ( connect(fd, (struct sockaddr *)&des, len) < 0 ) {
		close(fd);
		return(-1);
	}
	return(fd);
}

static int SioWaitAck(int fd, char cmd)
{
	struct timeval	authtime;
	fd_set		readfds;
	char		tmp[20];
	int		len;
	short		n;

	memset(&authtime, 0, sizeof(authtime));
	authtime.tv_usec = 0L;
	authtime.tv_sec = 10;
	FD_ZERO(&readfds);
	FD_SET(fd, &readfds);
	if ( select(fd + 1, &readfds, NULL, NULL, &authtime) < 0 )
		return(-1);
	if ( FD_ISSET(fd, &readfds) == 0 )
		return(-1);
	len = read(fd, tmp, 20);
	if ( cmd == DCMD_IQUEUE || cmd == DCMD_OQUEUE ) {
		if ( len < 4 )
			return(-1);
		if ( tmp[0] != cmd || tmp[1] != 2 )
			return(-1);
		n = *(short *)(&tmp[2]);
		return( (int)n );
	} else if ( cmd == DCMD_LSTATUS ) {
		if ( len < 5 )
			return(-1);
		if ( tmp[0] != cmd || tmp[1] != 3 )
			return(-1);
		n = 0;
		if ( tmp[2] == 1 )
			n |= 1;
		if ( tmp[3] == 1 )
			n |= 2;
		if ( tmp[4] == 1 )
			n |= 4;
		return( (int)n );
	} else {
		if ( len < 3 )
			return(-1);
		if ( tmp[0] != cmd || tmp[1] != 'O' || tmp[2] != 'K' )
			return(-1);
	}
	return(0);
}

static int SioCmdportFd(int fd)
{
	int	i;

	for ( i=0; i<DFD_TABLE_SIZE; i++ )
		if ( asfds[i].fd_data == fd )
			return(asfds[i].fd_cmd);
	return(-1);
}

static int SioAddFdtbl(int fd, int cmdfd)
{
	int	i;

	for ( i=0; i<DFD_TABLE_SIZE; i++ ) {
		if ( asfds[i].fd_data == -1 ) {
			asfds[i].fd_data = fd;
			asfds[i].fd_cmd = cmdfd;
			return(cmdfd);
		}
	}
	return(-1);
}

static int SioDelFdtbl(int fd)
{
	int	i;

	for ( i=0; i<DFD_TABLE_SIZE; i++ ) {
		if ( asfds[i].fd_data == fd ) {
			asfds[i].fd_data = -1;
			asfds[i].fd_cmd = -1;
			return(0);
		}
	}
	return(-1);
}
