all: list.o large_int.o
	gcc -W -Wall main/main.c out/large_int.o out/list.o
	mv a.out out

out/large_int.o: main/large_int.c
	gcc -c -W -Wall main/large_int.c
	mv large_int.o out

out/list.o: main/list.c
	gcc -c -W -Wall main/list.c
	mv list.o out

.PHONY: test
test: test/test_list.c out/list.o out/large_int.o
	cp main/*.h test
	gcc -W -Wall -o test_list test/test_list.c out/list.o
	mv test_list out
	# gcc -W -Wall -o test_large_int test/test_large_int.c out/large_int.o out/list.o
	rm test/*.h

.PHONY: clean
clean:
	rm a.out *.o
