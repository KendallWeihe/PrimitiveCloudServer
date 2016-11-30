extern "C" {
    #include "csapp.h"
}
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <fstream>
#include <vector>

using namespace std;

unsigned int parse_header(char client_message[]);
void get(char buf[], int connfd);
void parse_filename(char buf[], char filename[]);

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
    if (argc != 3) {
    	fprintf(stderr, "usage: %s <port> <key>\n", argv[0]);
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

      n = Rio_readnb(&rio, buf, MAXLINE);

      char secret_key_char_array[4];
      for (int i = 0; i < 4; i++){
        secret_key_char_array[i] = buf[i];
      }

      char type_char_array[4];
      for (int i = 0; i < 4; i++){
        type_char_array[i] = buf[4+i];
      }

      unsigned int key = parse_header(secret_key_char_array);
      cout << "Secret Key = " << key << endl;

      if (key != stoi(argv[2])){
        cout << "Invalid key" << endl;
      }
      else {
        unsigned int type = parse_header(type_char_array);
        cout << "Request Type = " << type << endl;

        switch(type){
          case 0: get(buf, connfd); break;
          case 1: break;
          case 2: break;
          case 3: break;
        }
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

void parse_filename(char buf[], char filename[]){
  for (int i = 8; i < 88; i++){
    if (buf[i] == '\0'){
      break;
    }
    filename[i-8] = buf[i];
  }
  cout << "Filename = " << filename << endl;
}

void get(char buf[], int connfd){

  char filename[80] = {0};
  parse_filename(buf, filename);

  ifstream in_file;
  in_file.open(filename);

  char c, return_buf[MAXLINE] = {0};
  unsigned int index = 8;
  while (in_file.get(c)){
    return_buf[index] = c;
    index += 1;
  }

  // change the below lines if there is an error
  return_buf[0] = 0;
  return_buf[1] = 0;
  return_buf[2] = 0;
  return_buf[3] = 0;

  return_buf[4] = index & 0xff;
  return_buf[5] = (index >> 8) & 0xff;
  return_buf[6] = (index >> 16) & 0xff;
  return_buf[7] = (index >> 24) & 0xff;

  // cout << "Index = " << index << endl;
  // cout << "Byte 1 = " << (unsigned int)return_buf[4] << endl;
  // cout << "Byte 2 = " << (unsigned int)return_buf[5] << endl;
  // cout << "Byte 3 = " << (unsigned int)return_buf[6] << endl;
  // cout << "Byte 4 = " << (unsigned int)return_buf[7] << endl;

  in_file.close();
  Rio_writen(connfd, return_buf, MAXLINE);

}
