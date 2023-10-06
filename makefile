make:hw05.c
	gcc -g hw05.c -o BTree
test1:BTree
	./BTree 3 22 16 41 11 18 28 58 1 8 12 17 19 23 31 52 59 61
test2:BTree
	./BTree 2 51 29 73 105 15 31
clean:
	rm BTree