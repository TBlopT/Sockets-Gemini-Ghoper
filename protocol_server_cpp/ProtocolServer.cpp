//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2º de grado de Ingeniería Informática
//                       
//                   Base class for protocol servers
// 
//****************************************************************************

#include <cerrno>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include "common.h"
#include "ProtocolServer.h"

// TODO: Students must implement this function
// This function should create a TCP socket, bind it to the specified port,
// and start listening for connections.
// If port is 0, the OS will assign a random available port.
// The function should return the socket descriptor.
std::pair<int, int> define_socket_TCP(int port) {
    // TODO: Create socket using socket()
    // TODO: Bind socket to port using bind()
    // TODO: Start listening using listen()
    // TODO: If port was 0, retrieve the assigned port using getsockname()
    
    return std::pair<int, int>(-1, -1);  // Replace with actual socket descriptor and port number
}

ProtocolServer::ProtocolServer(int port) : port(port), msock(-1), should_stop(false) {
}

ProtocolServer::~ProtocolServer() {
    stop();
}

void ProtocolServer::stop() {
    should_stop = true;
    if (msock >= 0) {
        shutdown(msock, SHUT_RDWR);
        close(msock);
        msock = -1;
    }
}
