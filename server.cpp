extern "C" {
    #include "csapp.h"
}
#include <iostream>
#include <string>
#include <netinet/in.h>
#include <fstream>
#include <vector>

using namespace std;

// function declarations
unsigned int convert_header_to_host_order(char client_message[]);
void get(char buf[], int connfd);
void put(char buf[], int connfd);
void parse_filename(char buf[], char filename[]);
void parse_filedata(char buf[], char filename[]);
int search(vector<string>, string);

vector<string> file_names;
vector<string> file_data;
int rio_error_check = 0;

/*
  function: main()
  inputs:
    port number
    secret key
  purpose:
    accept TCP connection from server
    read data from client connection
    parse data based on MyCloud Protocol
    respond to client request
*/
int main(int argc, char* argv[]){

    // socket data
    int listenfd, connfd, port;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    struct hostent *hp;
    char *haddrp;

    // check for argument count
    if (argc != 3) {
    	fprintf(stderr, "usage: %s <port> <key>\n", argv[0]);
    	exit(0);
    }

    // user defined port
    port = atoi(argv[1]);

    // open connection on user defined port
    listenfd = Open_listenfd(port);

    // loop until user enters Ctrl-C to quit
    while (1){

      clientlen = sizeof(clientaddr);
      // accept connection with client
      connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

      /* Determine the domain name and IP address of the client */
      hp = Gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
             sizeof(clientaddr.sin_addr.s_addr), AF_INET);
      haddrp = inet_ntoa(clientaddr.sin_addr);
      printf("server connected to %s (%s)\n", hp->h_name, haddrp);

      // data required to start reading from client
      char buf[MAXLINE] = {0};
      rio_t rio;
      size_t n;
      Rio_readinitb(&rio, connfd);

      // read from client connection
      n = Rio_readnb(&rio, buf, MAXLINE);

      if (n < 0){
        cout << "Error in reading from client" << endl;
        rio_error_check = -1;
      }

      // assign secret key to local variable
      char secret_key_char_array[4];
      for (int i = 0; i < 4; i++){
        secret_key_char_array[i] = buf[i];
      }

      // assign type to local variable
      char type_char_array[4];
      for (int i = 0; i < 4; i++){
        type_char_array[i] = buf[4+i];
      }

      // call function to convert header to little endian order
      unsigned int key = convert_header_to_host_order(secret_key_char_array);
      cout << "Secret Key = " << key << endl;

      // check if client key matches the server secret key
      if (key != (unsigned int)stoi(argv[2])){
        cout << "Invalid key" << endl; // case where the keys do not match
      }
      else {
        // convert the type to little endian order
        unsigned int type = convert_header_to_host_order(type_char_array);
        cout << "Request Type = " << type << endl;

        // switch statement to call function based on client specified type
        switch(type){
          case 0: get(buf, connfd); break;
          case 1: put(buf, connfd); break;
          case 2: break;
          case 3: break;
        }
      }

      cout << "--------------------------" << endl;

      Close(connfd);
    }

    return 0;

}

/*
  function convert_header_to_host_order()
  inputs:
    char array of 4 bytes
  purpose:
    convert char array of 4 bytes from network to host order
*/
unsigned int convert_header_to_host_order(char client_message[]){
  // local variables
  unsigned int byte1, byte2, byte3, byte4;
  byte1 = (unsigned char)client_message[3] << 24; // bit level operations
  byte2 = (unsigned char)client_message[2] << 16;
  byte3 = (unsigned char)client_message[1] << 8;
  byte4 = (unsigned char)client_message[0];
  // cout << byte1 << ",  " << byte2 << ",  " << byte3 << ",  " << byte4 << endl;

  unsigned int header = byte1 | byte2 | byte3 | byte4;
  return header;
}

/*
  function parse_filename()
  inputs:
    buffer of data from client
    empty char array to store filename
  purpose:
    extract filename from buffer read from client
*/
void parse_filename(char buf[], char filename[]){
  // note the filename begins at byte number 9 (i = 8)
  for (int i = 8; i < 88; i++){
    if (buf[i] == '\0'){ // case where the end of the file is found
      break;
    }
    filename[i-8] = buf[i];
  }
  cout << "Filename = " << filename << endl;
}

/*
  function parse_filedata()
  inputs:
    buffer of data from client
    empty char array to store file data
  purpose:
    extract file data from buffer read from client
*/
void parse_filedata(char buf[], char filedata[]){
  // the length of the file starts at 88 and ends at 91
  unsigned char byte_1 = buf[91];
  unsigned char byte_2 = buf[90];
  unsigned char byte_3 = buf[89];
  unsigned char byte_4 = buf[88];

  unsigned int a = (unsigned int)byte_1 << 24; // bit level operations
  unsigned int b = (unsigned int)byte_2 << 16;
  unsigned int c = (unsigned int)byte_3 << 8;
  unsigned int d = (unsigned int)byte_4;

  unsigned int file_length = a | b | c | d;

  // note the filedata begins at byte number 93 (i = 92)
  for (unsigned int i = 92; i < 92 + file_length; i++){
    if (buf[i] == '\0'){ // case where the end of the file is found
      break;
    }
    filedata[i-92] = buf[i];
  }
}

/*
  function get()
  inputs:
    buffer of data from client
    client connection file descriptor
  purpose:
    read a file
    send file data back to client
*/
void get(char buf[], int connfd){

  // get filename from buffer
  char filename[80] = {0};
  parse_filename(buf, filename);

  bool file_exists = false;
  string file;
  int error = 0;
  for (unsigned int i = 0; i < file_names.size(); i++){
    if (file_names[i] == filename){
      file = file_data[i];
      file_exists = true;
    }
  }

  if (!file_exists){
    cout << "File not found\n";
    error = -1;
  }

  // read the data from the file
  char return_buf[MAXLINE] = {0};
  unsigned int index;
  for (index = 8; index < file.length()+8; index++){
    return_buf[index] = file[index-8];
  }

  // error bytes
  if (error < 0 || rio_error_check < 0){
    return_buf[0] = -1;
    rio_error_check = 0;
    cout << "Operation Status = error\n";
  }
  else{
    return_buf[0] = 0;
    cout << "Operation Status = success\n";
  }
  return_buf[1] = 0;
  return_buf[2] = 0;
  return_buf[3] = 0;

  // bytes 4-8 are the file size (number of bytes)
  return_buf[4] = index & 0xff;
  return_buf[5] = (index >> 8) & 0xff;
  return_buf[6] = (index >> 16) & 0xff;
  return_buf[7] = (index >> 24) & 0xff;

  // UNCOMMENT TO CHECK FILE SIZE
  // cout << "Index = " << index << endl;
  // cout << "Byte 1 = " << (unsigned int)return_buf[4] << endl;
  // cout << "Byte 2 = " << (unsigned int)return_buf[5] << endl;
  // cout << "Byte 3 = " << (unsigned int)return_buf[6] << endl;
  // cout << "Byte 4 = " << (unsigned int)return_buf[7] << endl;

  Rio_writen(connfd, return_buf, MAXLINE);

}

void put(char buf[], int connfd){

  // get filename from buffer
  char filename[80] = {0};
  parse_filename(buf, filename);
  string fname(filename); // convert to string

  char data[MAXLINE] = {0};
  parse_filedata(buf, data);
  string fdata(data);

  int index = search( file_names, fname ); // check if the file already exists
  if( index != -1 ) { // If it does, update it.
    file_data[index] = fdata;
  }
  else { // If not, add the new variable and value.
    file_names.push_back(fname);
    file_data.push_back(fdata);
  }

  // read the data from the file
  char return_buf[MAXLINE] = {0};

  // error bytes
  if (rio_error_check < 0){
    return_buf[0] = -1;
    rio_error_check = 0;
    cout << "Operation Status = error\n";
  }
  else{
    return_buf[0] = 0;
    cout << "Operation Status = success\n";
  }
  return_buf[1] = 0;
  return_buf[2] = 0;
  return_buf[3] = 0;

  // write to the client
  Rio_writen(connfd, return_buf, MAXLINE);

}



int search(vector<string> vec, string toFind) {
	for (unsigned int i = 0; i < vec.size(); i++) {
		if (vec[i] == toFind) {
			return i; // Return the position if it's found.
		}
	}
	return -1; // If not found, return -1.
}
