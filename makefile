all: webserv my_histogram.cgi

my_histogram.cgi: my_histogram.c
	gcc -o my_histogram.cgi my_histogram.c -Wall

webserv: webserv.c
	gcc -o webserv webserv.c -Wall

clean:
	rm webserv
	rm my_histogram.cgi
