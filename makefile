all: webserv

webserv: webserv.c
	gcc -o webserv webserv.c -Wall

clean:
	rm webserv
