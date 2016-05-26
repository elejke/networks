example:	example.o
	cc example.o -lsocket -lnls -o example
example.o:	example.c
	cc -c -I/usr/include example.c
