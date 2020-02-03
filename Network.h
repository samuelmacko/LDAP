/*
 * ISA
 * 
 * Samuel Macko xmacko10
 * xmacko10@stud.fit.vutbr.cz
 */

#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <vector>
#include <stdio.h>
#include <cstring>

const int BUFFER_SIZE = 512;

using namespace std;

class Network{
    public:
        static int create_socket(int port);
        static string read_message(int socket);
        static void send_message(int socket, vector<uint8_t> message);
};


#endif /* NETWORK_H */

