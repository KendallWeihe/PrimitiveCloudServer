extern "C" {
    #include "csapp.h"
}
#include <iostream>
#include <string>
#include <netinet/in.h>

using namespace std;

unsigned int parse_header(char client_message[]);
void get(rio_t rio);

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

      size_t n;
      char buf[MAXLINE] = {0};
      rio_t rio;
      Rio_readinitb(&rio, connfd);

      n = Rio_readnb(&rio, buf, 4);
      unsigned int secret_key = parse_header(buf);
      cout << "Secret key = " << secret_key << endl;
      cout << "BUF 1 = " << buf << endl;

      rio.rio_cnt = 4;
      n = Rio_readnb(&rio, buf, 4);
      unsigned int type = parse_header(buf);
      cout << "Type = " << type << endl;
      cout << "BUF 2 = " << buf << endl;

      switch(type){
        case 0: get(rio); break;
        case 1: break;
        case 2: break;
        case 3: break;
      }

      Close(connfd);
    }

    return 0;

}

unsigned int parse_header(char client_message[]){
  unsigned int byte1, byte2, byte3, byte4;
  byte1 = (unsigned char)client_message[3] << 24;
  byte2 = (unsigned char)client_message[2] << 16;
  byte3 = (unsigned char)client_message[1] << 8;
  byte4 = (unsigned char)client_message[0];
  // cout << byte1 << ",  " << byte2 << ",  " << byte3 << ",  " << byte4 << endl;

  unsigned int header = byte1 | byte2 | byte3 | byte4;
  return header;
}

void get(rio_t rio){
  char buf[80];
  size_t n;
  n = Rio_readnb(&rio, buf, 80);
  char filename[80];
  int count = 0;
  for (int i = 0; i < 80; i++){
    if (buf[i] == '\0'){
      break;
    }
    cout << "Buf = " << buf[i] << endl;
    filename[i] = buf[i];
    count += 1;
  }
  cout << count;
  cout << "Filename = " << filename << endl;
  // TODO
    // next open the file and write it back
}
