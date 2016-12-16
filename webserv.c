#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/wait.h>

//const char *basic_response = "HTTP-Version: HTTP/1.1 200 OK\nContent-Length: 80\nContent-Type: text/html\n\n<html><head><title>It works</title></head><body><p>kinda works</p></body></html>";

const char *response_template = "HTTP/1.1 %s\r\nContent-Length: %i\r\nContent-Type: %s\r\n\n%s";
const char *header_template = "HTTP/1.1 %s\r\nContent-Length: %i\r\nContent-Type: %s\r\n\n";

const char *request_template = "%s %s HTTP/%s\n%s";
const char *html_template = "<html><head><title>webserv</title></head><body><p>%s</p></body></html>";

const char *content_plain = "text/plain";
const char *content_html = "text/html";
const char *content_gif = "image/gif";
const char *content_jpeg = "image/jpeg";

const char *HTTP_OK = "200 OK";
const char *HTTP_NOTFOUND = "404 Not Found";
const char *HTTP_NOTIMPLEMENTED = "501 Not Implemented";

#define JPEG 1
#define GIF 2

//when a child process exits, handle it
void SIGCHLD_handler(int sig)
{
    waitpid(-1, NULL, WNOHANG);
}

char *format_response(const char* status, const char *content_type, void *content, int content_length)
{
  char *response_output = malloc(8000);
  sprintf(response_output, response_template, status, content_length, content_type, content);
  return response_output;
}

char *format_header(const char* status, const char *content_type, int content_length)
{
  char *response_output = malloc(8000);
  sprintf(response_output, header_template, status, content_length, content_type);
  return response_output;
}

char *basic_html_response(const char *status, const char *input_string)
{

  char html_output[8000];
  sprintf(html_output, html_template, input_string);
  char *response_output = format_response(status, content_html, html_output, strlen(html_output));
  //printf("\n%s\nlen: %zu\n",response_output, strlen(html_output));
  return response_output;
}

void handle_cgi(const char *cmd, int new_sd)
{
    //run the command with popen
    FILE *fd = popen(cmd, "r");

    //read output of the command into a buffer
    char *buffer = malloc(8000);
    fread(buffer, 1, 8000, fd);

    //format the response headers and ls output
    char *response = format_response(HTTP_OK, content_plain, buffer, strlen(buffer));
    write(new_sd, response, strlen(response));
    free(buffer);
}

void handle_html(const char *path, int new_sd, size_t size)
{
    //run the command with popen
    FILE *fd = fopen(path, "r");

    //read output of the command into a buffer
    char *buffer = malloc(size);
    fread(buffer, 1, size, fd);

    //format the response headers and ls output
    char *response = format_response(HTTP_OK, content_html, buffer, size);
    write(new_sd, response, strlen(response));
    free(buffer);
}


void handle_img(const char *path, int new_sd, size_t size, const char *content_type)
{
    //run the command with popen
    int fd = open(path, O_RDONLY);

    //format the response headersi
    char *response = format_header(HTTP_OK, content_type, size);
    write(new_sd, response, strlen(response));


    //if (sendfile(new_sd, fd, NULL, size) < size)
    //{
    //    exit(0);
    //}
    char* buffer = malloc(size);
    read(fd, buffer, size);
    write(new_sd, buffer, size);
    free(buffer);

    close(new_sd);
    exit(0);

    //fprintf(stderr, "response is %s", response);
}

int ends_in_cgi(const char *path)
{
    if (strstr(&path[strlen(path) - 4], ".cgi") != NULL)
        return 1;
    return 0;
}

int ends_in_html(const char *path)
{
    if (strstr(&path[strlen(path) - 5], ".html") != NULL)
        return 1;
    return 0;
}


//
int img_checker(const char *path)
{
  // Check to see whether it is an image or not
  if (strstr(&path[strlen(path)-4], ".jpg") != NULL)
  {
    return JPEG;
          
  }
  else if (strstr(&path[strlen(path)-5], ".jpeg") != NULL)
  {
    return JPEG;
  }
  else if (strstr(&path[strlen(path)-4], ".gif") != NULL)
  {
    return GIF;
  }
  else
    return 0;
}
//currently mostly copied from the given tcp source files
int main (int argc, const char *argv[])
{
  int sd, new_sd;
  struct sockaddr_in name, cli_name;
  int sock_opt_val = 1;
  socklen_t cli_len;

  struct sigaction sigchld_action;

  sigchld_action.sa_handler = SIGCHLD_handler;
  sigemptyset(&sigchld_action.sa_mask);
  sigchld_action.sa_flags = SA_RESTART;

  if (argv[1] == NULL)
  {
    printf("%s\n", "webserv requires a port number, correct usage:\n./webserv port-number");
    exit(1);
  }

  int port = atoi(argv[1]);
  if (port < 5000 || port > 65536)
  {
    printf("%s\n", "provided port number is not in expected range, must be in range 5000-65536");
    exit(1);
  }

  //should dynamically allocate this or better manage reading multiple times
  char data[8000]; 
  
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) < 0) // Create the socket here and set to sd (socket descriptor)
  {
    perror("(servConn): socket() error");
    exit (-1);
  }

  if (setsockopt (sd, SOL_SOCKET, SO_REUSEADDR, (char *) &sock_opt_val,
    sizeof(sock_opt_val)) < 0) // setsockopt is similar to getopt in terms of creating flags
  {
    perror ("(servConn): Failed to set SO_REUSEADDR on INET socket");
    exit (-1);
  }

  name.sin_family = AF_INET; // socket is in family AF_INET (IPv4 protocol)
  name.sin_port = htons (port); // htons = host to network short -> set port
  name.sin_addr.s_addr = htonl(INADDR_ANY); // htonl = host to network long -> set address

  if (bind (sd, (struct sockaddr *)&name, sizeof(name)) < 0) // bind the socket so it may listen and accept
  {
    perror ("(servConn): bind() error");
    exit (-1);
  }

  listen (sd, 5);

  while (true)
  {
    cli_len = sizeof (cli_name);

    // I think that this copies sockaddr name and its contents to make new_sd
    new_sd = accept (sd, (struct sockaddr *) &cli_name, &cli_len); 
    printf ("Assigning new socket descriptor:  %d\n", new_sd);

    if (new_sd < 0) 
    {
      perror ("(servConn): accept() error");
      exit (-1);
    }

    if (fork () == 0) 
    {   
      close (sd);

      struct stat statbuf;
      char *cwd = malloc(PATH_MAX);
      getcwd(cwd, PATH_MAX);
      char request[8000];
      char version[8000];
      char method[100];
      char *fullpath = malloc(PATH_MAX);
      int img_type;
      char req2[8000];

      read (new_sd, &data, 8000); 

      sscanf(data, request_template, method, request, version);


      // CHECK HERE FOR SEPARATING ARGS WITH QUESTION MARKS 
      // seperates potential (?) args from request
      char *line = strdup(request);
      if (strstr(request, "?") != NULL)    
      {
        char *req = strtok(line, "?");
        char *req_args = strtok(NULL, "?");
        strcpy(request, req);
        strcpy(req2, req_args);
      }
      printf("req2 is %s\n", req2);

      sprintf(fullpath, "%s%s", cwd, request);
      printf("request is %s\n", request);
      fprintf(stderr, "%s\n", fullpath);

      if (strcmp(method, "GET") != 0)
      {
          fprintf(stderr, "not a get");

          char *response = basic_html_response(HTTP_NOTIMPLEMENTED, "Error - Method not supported");
          write(new_sd, response, strlen(response));
          close(new_sd);
          exit(0);
      }


      if (stat(fullpath, &statbuf) < 0)
      {
        //errno breaks this for some reason
        //if (errno == ENOENT)
        //{

            char *response = basic_html_response(HTTP_NOTFOUND, "404 resource not found");
            //fprintf(stderr, "response is:\n%s\nwith a length of %i\n", response, strlen(response));
            if (write(new_sd, response, strlen(response)) < 0)
            {
              perror("Write to socket failed");
            }

        //}
        fprintf(stderr, "failed stat for %s", request);
        //perror("\nfdslkahfdlsj");

        //if (write (new_sd, basic_response, strlen(basic_response)) < 0)
        //      perror("error writing response to socket");

        exit(0);
      }
      else if (S_ISDIR(statbuf.st_mode))
      {
        //create a string for hte ls command
        char *cmd = malloc(PATH_MAX + 6);
        sprintf(cmd, "ls -l %s", fullpath);

        //run the command with popen
        FILE *fd = popen(cmd, "r");

        //read output of the command into a buffer
        char *buffer = malloc(8000);
        fread(buffer, 1, 8000, fd);

        //format the response headers and ls output
        char *response = format_response(HTTP_OK, content_plain, buffer, strlen(buffer));
        write(new_sd, response, strlen(response));
        free(buffer);
      }
      else if (ends_in_cgi(fullpath))
      {
        handle_cgi(fullpath, new_sd);
      }
      else if (ends_in_html(fullpath))
      {
        handle_html(fullpath, new_sd, statbuf.st_size);
      }
      else if ((img_type = img_checker(fullpath)) != 0)
      {
        const char *content_type;
        switch (img_type){
            case JPEG:
                content_type = content_jpeg;
                break;
            case GIF:
                content_type = content_gif;
                break;
            default:
                fprintf(stderr, "%s\n", "something went wrong");
                abort();
        }

        handle_img(fullpath, new_sd, statbuf.st_size, content_type);

      }

      exit (0);

    }
    close(new_sd);
  }
}