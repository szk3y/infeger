all: main.c list.o large_int.o
	gcc -W -Wall main.c large_int.o list.o

large_int.o: large_int.c
	gcc -c -W -Wall large_int.c

list.o: list.c
	gcc -c -W -Wall list.c

.PHONY: test
test: test_list.c list.o large_int.o
	gcc -W -Wall -o test_list.out test_list.c list.o

.PHONY: clean
clean:
	rm *.out *.o
