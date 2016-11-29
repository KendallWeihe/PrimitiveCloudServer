extern "C" {
    #include "csapp.h"
}
#include <iostream>
#include <string>
#include <netinet/in.h>

using namespace std;

void convert_to_protocol_format(char protocol_buf[], unsigned int secret_key, unsigned int type, string filename);

int main(int argc, char* argv[]){
  int clientfd, port;
  char *host, buf[MAXLINE];
  rio_t rio;

  if (argc != 5) {
    fprintf(stderr, "usage: %s <host> <port> <key> <filename>\n", argv[0]);
    exit(0);
  }
  host = argv[1];
  port = atoi(argv[2]);

  unsigned int secret_key = atoi(argv[3]);
  unsigned int type = 0;
  string filename = argv[4];
  char protocol_buf[80] = {0};
  convert_to_protocol_format(protocol_buf, secret_key, type, filename);

  for (int i = 0; i < 87; i++){
    cout << "protocol_buf = " << protocol_buf[i] << endl;
  }

  clientfd = Open_clientfd(host, port);
  Rio_readinitb(&rio, clientfd);

  Rio_writen(clientfd, protocol_buf, strlen(buf));

  // Rio_readnb(&rio, buf, MAXLINE);
  // Fputs(buf, stdout);
  //
  // cout << buf << endl;
  Close(clientfd); //line:netp:echoclient:close
  exit(0);
}

void convert_to_protocol_format(char protocol_buf[], unsigned int secret_key, unsigned int type, string filename){
  protocol_buf[0] = htonl(secret_key) >> 24;
  protocol_buf[1] = (htonl(secret_key) >> 16) & 0xff;
  protocol_buf[2] = (htonl(secret_key) >> 8) & 0xff;
  protocol_buf[3] = htonl(secret_key) & 0xff;
  protocol_buf[4] = htonl(type) >> 24;
  protocol_buf[5] = (htonl(type) >> 16) & 0xff;
  protocol_buf[6] = (htonl(type) >> 8) & 0xff;
  protocol_buf[7] = htonl(type) & 0xff;

  for (int i = 8; i < filename.length()+8; i++){
    protocol_buf[i] = filename[i-8];
  }

}
