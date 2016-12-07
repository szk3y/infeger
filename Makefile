all: list.o large_int.o
	gcc -W -Wall main.c large_int.o list.o

large_int.o:
	gcc -c -W -Wall large_int.c

list.o:
	gcc -c -W -Wall list.c
