all:
	gcc -Wall -o my-unix-shell my-unix-shell.c

clean:
	rm -f my-unix-shell output.txt
