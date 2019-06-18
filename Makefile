all: rise
rise: rise.c
	gcc -g -Wall rise.c -o rise
clean:
	rm -f rise *~
