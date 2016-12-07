all: list.o large_int.o
	gcc -W -Wall main/main.c out/large_int.o out/list.o
	mv a.out out

large_int.o:
	gcc -c -W -Wall main/large_int.c
	mv large_int.o out

list.o:
	gcc -c -W -Wall main/list.c
	mv list.o out

.PHONY: test
test: list.o large_int.o
	gcc -W -Wall -o test_list test/test_list.c out/list.o
	mv test_list out
	# gcc -W -Wall -o test_large_int test/test_large_int.c out/large_int.o out/list.o

.PHONY: clean
clean:
	rm out/*
