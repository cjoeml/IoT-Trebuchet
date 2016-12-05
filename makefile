all: webserv

webserv: webserv.c
	gcc -o webserv webserv.c

clean:
	rm webserv