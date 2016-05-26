example:	example.o
	cc example.o -lsocket -lnet -lnsl_s -o example
example.o:	example.c
	cc -c -I/usr/netinclude example.c
