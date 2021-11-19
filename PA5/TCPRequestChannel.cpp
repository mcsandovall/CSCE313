#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include "TCPRequestChannel.h"
using namespace std;

TCPRequestChannel::TCPRequestChannel(const string host_name, const string port_no){
   if(host_name.empty()){ // set up channel for server side
      int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
      struct addrinfo hints, *serv;
      struct sockaddr_storage their_addr; // connector's address information
      socklen_t sin_size;
      char s[INET6_ADDRSTRLEN];
      int rv;

      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE; // use my IP

      if ((rv = getaddrinfo(NULL, port_no.c_str(), &hints, &serv)) != 0) {
         cerr  << "getaddrinfo: " << gai_strerror(rv) << endl;
         exit(EXIT_FAILURE);
      }
      if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
         perror("server: socket");
         exit(EXIT_FAILURE);
      }
      if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
         close(sockfd);
         perror("server: bind");
         exit(EXIT_FAILURE);
      }
      freeaddrinfo(serv); // all done with this structure

      if (listen(sockfd, 20) == -1) {
         perror("listen");
         exit(1);
      }
   } // set up client for server side

   struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int status;
	//getaddrinfo("www.example.com", "3490", &hints, &res);
	if ((status = getaddrinfo (host_name.c_str(), port_no.c_str(), &hints, &res)) != 0) {
      cerr << "getaddrinfo: " << gai_strerror(status) << endl;
      exit(EXIT_FAILURE);
    }

	// make a socket:
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (sockfd < 0){
		perror ("Cannot create socket");	
		exit(EXIT_FAILURE);
	}

	// // connect!
	// if (connect(sockfd, res->ai_addr, res->ai_addrlen)<0){
	// 	perror ("Cannot Connect");
	// 	exit(EXIT_FAILURE);
	// }
	// cout << "Connected " << endl;

   freeaddrinfo (res);
}

TCPRequestChannel::TCPRequestChannel (int client_socket){
   // create a channel out of a newly accepted client socket
   struct sockaddr_storage their_addr;
   socklen_t sin_size =  sizeof(their_addr);

   sockfd = accept (client_socket, (struct sockaddr *)&their_addr, &sin_size);
   if (sockfd== -1) {
      perror("Denied Access");
   }
   cout << "Access Granted" << endl;
   return;
}

TCPRequestChannel::~TCPRequestChannel(){
   close(sockfd);
}

int TCPRequestChannel::cread (void* msgbuf, int buflen){
   return read (sockfd, msgbuf, buflen); 
}

int TCPRequestChannel::cwrite(void* msgbuf, int msglen){
   return write (sockfd, msgbuf, msglen);
}

int TCPRequestChannel::getfd(){
   return sockfd;
}