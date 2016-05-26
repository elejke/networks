example:	example.o
	cc example.o -o example
example.o:	example.c
	cc -c example.c
