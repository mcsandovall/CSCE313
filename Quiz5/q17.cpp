#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

/**
 * @author Mario Sandoval 
 * @class CSCE 313 
 * */

using namespace std;

bool validate(char* ip, char* filter){
    bool includes = false;
    for(int i ; i < strlen(ip) - strlen(filter) ;++i){
        if(ip[i] == filter[0]){
            for(int j = 0; j < strlen(filter);++j){
                includes = (ip[i+j] == filter[j]);
                if(!includes){break;}
            }
            if(includes){return true;}
        }
    }
    return false;
}

int main(int argc, char* args[]){
    /*
    Program takes two arguments, the host name and the search filter
    */
    char* host_name = args[1];
    char* filter = args[2];
    /* Get the IP from the given host name */
    int status;
    struct addrinfo hints, *servinfo, *ptr = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo(host_name,"http",&hints,&servinfo)) != 0){
        cerr  << "getaddrinfo: " << gai_strerror(status) << endl;
        return EXIT_FAILURE;
    }

    for(ptr=servinfo; ptr != nullptr; ptr = ptr->ai_next){
        void* addr;
        string ipver;

        // get the pointer to the address itself
        if(ptr->ai_family == AF_INET){ // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *) ptr->ai_addr;
            addr = &(ipv4->sin_addr);
            ipver = "IPv4";
        }else{ // IPv6
            struct sockaddr_in6 * ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
            addr = &(ipv6->sin6_addr);
            ipver = "IPv6";
        }

        // convert the IP to a string and print it
        char ipstr[ptr->ai_addrlen];
        inet_ntop(ptr->ai_family, addr, ipstr,sizeof(ipstr));
        if(validate(ipstr,filter)){ // validates the input from the filter
            // prints the ip if it passes the filter
            printf(" %s: %s\n",ipver.c_str(),ipstr);
        }
    }
    return 0;
}