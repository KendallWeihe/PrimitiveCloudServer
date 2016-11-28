#include "csapp.h"
#include <iostream>

using namespace std;

int read_from_client(int);

int main(int argc, char* argv[]){

  // TODO:
    // check for correct arguments
    // Open_listenfd()
    // while(1)
    //   Accept()
    //   Gethostbyaddr()
    //   Rio_readinitb()
    //   Rio_readnb()
    //   process data:
    //     determine type of request
    //     if put:
    //       save to file
    //       Rio_writen() --> response?
    //     else if get:
    //       open file
    //       read file
    //       Rio_writen() --> file unfo
    //     else if del:
    //       execve("rm file.data")
    //       Rio_writen() --> response?
    //     else if list:
    //       execve("ls")
    //       capture output
    //       Rio_writen() --> output from "ls"
    //     else:
    //       Rio_writen() --> command not recognized
    //   Close()

    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;
    if (argc != 2) {
    	fprintf(stderr, "usage: %s <port>\n", argv[0]);
    	exit(0);
    }
    port = atoi(argv[1]);

    listenfd = Open_listenfd(port);
    while (1){
      clientlen = sizeof(clientaddr);
      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

      /* Determine the domain name and IP address of the client */
      hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
             sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      haddrp = inet_ntoa(clientaddr.sin_addr);
      printf("server connected to %s (%s)\n", hp->h_name, haddrp);

      read_from_client(connfd);

      Close(connfd);
    }

    return 0;

}

int read_from_client(int connfd){

  size_t n;
  char buf[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, connfd);
  while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { //line:netp:echo:eof
    printf("server received %d bytes\n", (int)n);
    Rio_writen(connfd, buf, n);
  }

  return 0;

}
