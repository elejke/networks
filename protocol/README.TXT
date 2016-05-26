
This directory includes the example program that shows
you how to control Async Server's ports with
'ASPP' mode - Async Server Proprietory Protocol.

To test the program, you should first set the Async Server
testing port to 'ASPP' mode. Otherwise, the
example program may not function as expected. Please check
the Async Server User's Manual for configuration instructions.

Be aware that you need to have TCP/IP development 
tools/libraries installed on the system you are using.

The example program is developed under SCO v3.2.4.
In your environment, the include files and network library
names might be different. Tailor the
as.mak or as.c files to suit your environment requirements.
For example, SCO must link to a TCP/IP library
called libnls.a whereas Solaris must link to libnsl.a,
and Venix must link to libnsl_s.a.

=================================
ASPP Library function description
=================================

1. sio_init()
	Initial data table using of ASPP functions.
	This function must be called once before calling any other sio_open,
	sio_close.., etc.
	Arg:	none
	Return: == 0	:	O.K.
		  -1	:	init fail.

2. sio_open(unsigned long server_ip, int port)
	open Async Server serial port.
	Args:	unsigned long	ipaddr -  IP address of Async Server
					  e.g. 0x0102A8C0 -> 192.168.2.1
		int		portno -  serial port number of Async Server
					  number range from 1 to 16.
	Return: >= 0	:	file handle ID (open success).
		  -1	:	open fail.

3. sio_close(int fd)
	close Async Server serial port
	Arg:	int	fd	-	file handle ID return from sio_open()
	Return: 0

4. sio_ioctl(int fd, int baud, int mode)
	Set Async port baud rate and character mode.
	Args:	int	fd	-	file handle ID return from sio_open()
		int	baud	-	0: 300	    1: 600	2: 1200
					3: 2400     4: 4800	5: 7200
					6: 9600     7: 19200	8: 38400
					9: 57600    10: 115200	11: 230400
					12: 460800  13: 921600	14: 150
					15: 134     16: 110	17: 75
					18: 50
		int	mode	- (bit 0,1)	0x00	-	5 data bits
						0x01	-	6 data bits
						0x10	-	7 data bits
						0x11	-	8 data bits
				  (bit 2)	0x00	-	1 stop bit
						0x04	-   1.5/2 stop bits
				  (bit 3,4,5)	0x00	-	none parity
						0x08	-	odd parity
						0x18	-	even parity
						0x28	-	mark parity
						0x38	-	space parity
	Return:    0	:	O.K.
		  -1	:	error

5. sio_baud(int fd, long baud)
	Set Async Port baud rate.
	Args:	int	fd	-	file handle ID return from sio_open()
		long	baud	-	desired baud rate
	Return:    0	:	O.K.
		  -1	:	error

6. sio_flowctrl(int fd, int mode)
	Set Async Port flow control.
	Args:	int	fd	-	file handle ID return from sio_open()
		int	mode	-	bit 0:	CTS flow control
					bit 1:	RTS flow control
					bit 2:	Tx XON/XOFF flow control
					bit 3:	Rx XON/XOFF flow control
					(0 = OFF,  1 = ON)
	Return:    0	:	O.K.
		  -1	:	error

7. sio_lctrl(int fd, int mode)
	Set Async Port line control.
	Args:	int	fd	-	file handle ID return from sio_open()
		int	mode	-	bit 0:	DTR output state
					bit 1:	RTS output state
					(0 = OFF,  1 = ON)
	Return:    0	:	O.K.
		  -1	:	error

8. sio_lstatus(int fd)
	Get Async Port line status.
	Args:	int	fd	-	file handle ID return from sio_open()
	Return:  >=0	:	line status:	bit 0: DSR, bit 1: CTS,
						bit 2: DCD
						(1 - ON, 0 - OFF)
		  -1	:	error

9. sio_flush(int fd, int func)
	Flush input/output buffer data.
	Args:	int	fd	-	file handle ID return from sio_open()
		int	func	-	0 :	flush input buffer
					1 :	flush output buffer
					2 :	flush input/output buffer
	Return: >= 0	:	O.K.
		  -1	:	error

10. sio_write(int fd, char *buf, int len)
	write data to Async Server serial port.
	Args:	int	fd	-	file handle ID return from sio_open()
		char *	buf	-	write out data buffer pointer
		int	len	-	write out data length
	Return: >= 0	:	write out data length
		  -1	:	write error

11. sio_read(int fd, char *buf, int len)
	read data from Async Server serial port.
	Args:	int	fd	-	file handle ID return from sio_open()
		char *	buf	-	read data buffer pointer
		int	len	-	read data buffer length
	Return: >= 0	:	read data length
		  -1	:	read error

12. sio_break(int fd, int time)
	Send out a break signal.
	Args:	int	fd	-	file handle ID return from sio_open()
		int	time	-	break time in mini-second
	Return:    0	:	O.K.
		  -1	:	error

13. sio_oqueue(int fd)
	Get output buffer data queue size.
	Args:	int	fd	-	file handle ID return from sio_open()
	Return: >= 0	:	output buffer data queue size
		  -1	:	error

14. sio_iqueue(int fd)
	Get input buffer data queue size.
	Args:	int	fd	-	file handle ID return from sio_open()
	Return: >= 0	:	input buffer data queue size
		  -1	:	error