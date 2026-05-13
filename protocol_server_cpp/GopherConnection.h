#ifndef GOPHERCONNECTION_H
#define GOPHERCONNECTION_H

#include <string>

const int MAX_SELECTOR_SIZE = 1024;

class GopherConnection {
public:

    GopherConnection(int socket, int server_port);
    ~GopherConnection();

    void handle_request();

private:
    int socket_fd;
    int port; 
    
    std::string read_selector();
    void send_file(const std::string& path);
    void send_directory(const std::string& path);
    void send_error(const std::string& message);
    std::string selector_to_path(const std::string& selector);
};

#endif