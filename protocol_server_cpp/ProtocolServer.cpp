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
 
std::pair<int, int> define_socket_TCP(int port) {
    // 1. Create the socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        return std::pair<int, int>(-1, -1);
    }
 
    // Allow reuse of local addresses (avoids "address already in use" on restart)
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
 
    // 2. Bind to the given port (0 = let the OS pick one)
    struct sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);
 
    if (bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
        std::cerr << "bind() failed: " << strerror(errno) << std::endl;
        close(sock);
        return std::pair<int, int>(-1, -1);
    }
 
    // 3. Start listening
    if (listen(sock, SOMAXCONN) < 0) {
        std::cerr << "listen() failed: " << strerror(errno) << std::endl;
        close(sock);
        return std::pair<int, int>(-1, -1);
    }
 
    // 4. If port was 0, retrieve the port assigned by the OS
    if (port == 0) {
        struct sockaddr_in assigned{};
        socklen_t len = sizeof(assigned);
        if (getsockname(sock, (struct sockaddr *) &assigned, &len) == 0) {
            port = ntohs(assigned.sin_port);
        }
    }
 
    return std::pair<int, int>(sock, port);
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