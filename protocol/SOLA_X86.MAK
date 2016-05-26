example:	example.o
	cc example.o -lsocket -lnsl -o example
example.o:	example.c
	cc -c -I/usr/include example.c
