extern "C" {
    #include "csapp.h"
}
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <fstream>

using namespace std;

void convert_to_protocol_format(char protocol_buf[], unsigned int secret_key, unsigned int type, string filename);
unsigned int parse_error(char buf[]);
unsigned int parse_num_bytes(char buf[]);

int main(int argc, char* argv[]){
  int clientfd, port;
  char *host, buf[MAXLINE] = {0};
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

  clientfd = Open_clientfd(host, port);
  Rio_readinitb(&rio, clientfd);

  Rio_writen(clientfd, protocol_buf, MAXLINE);
  Rio_readnb(&rio, buf, MAXLINE);
  // Fputs(buf, stdout);

  unsigned int error_check = parse_error(buf);
  cout << "Error = " << error_check << endl;

  unsigned int num_bytes = parse_num_bytes(buf);
  cout << "Num bytes = " << num_bytes << endl;

  char file_buf[num_bytes];
  for (int i = 0; i < num_bytes; i++){
    file_buf[i] = buf[i+8];
  }

  ofstream out_file;
  out_file.open("testing.cpp");
  out_file.write(file_buf, num_bytes);
  out_file.close();

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

unsigned int parse_error(char buf[]){
  unsigned int error = ((unsigned int)buf[3] << 24) | ((unsigned int)buf[2] << 16) | ((unsigned int)buf[1] << 8) | (unsigned int)buf[0];
  return error;
}

unsigned int parse_num_bytes(char buf[]){
  unsigned char byte_1 = buf[7];
  unsigned char byte_2 = buf[6];
  unsigned char byte_3 = buf[5];
  unsigned char byte_4 = buf[4];

  unsigned int a = (unsigned int)byte_1 << 24;
  unsigned int b = (unsigned int)byte_2 << 16;
  unsigned int c = (unsigned int)byte_3 << 8;
  unsigned int d = (unsigned int)byte_4;

  unsigned int num_bytes = a | b | c | d;

  return num_bytes;
}
