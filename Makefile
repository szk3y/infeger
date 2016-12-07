all: list.o large_int.o
	gcc -W -Wall main/main.c out/large_int.o out/list.o
	mv a.out out

large_int.o:
	gcc -c -W -Wall main/large_int.c
	mv large_int.o out

list.o:
	gcc -c -W -Wall main/list.c
	mv list.o out

.PHONY: clean
clean:
	rm out/*
