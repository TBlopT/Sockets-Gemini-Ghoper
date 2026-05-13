//****************************************************************************
//                   Gopher Protocol Connection Handler
//****************************************************************************

#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "GopherConnection.h"
#include <sys/socket.h>
#include "common.h"


GopherConnection::GopherConnection(int socket, int server_port) : socket_fd(socket), port(server_port) {
}

GopherConnection::~GopherConnection() {
    close(socket_fd);
}

std::string GopherConnection::read_selector() {
    std::string selector;
    char c;
    int bytes;
 
    while (selector.size() < MAX_SELECTOR_SIZE) {
        bytes = recv(socket_fd, &c, 1, 0);
        if (bytes <= 0) break;
        if (c == '\r') {
            recv(socket_fd, &c, 1, 0); // Consume \n
            break;
        }
        if (c == '\n') break;
        selector += c;
    }
    return selector;
}

std::string GopherConnection::selector_to_path(const std::string& selector) {
    if (selector.empty() || selector == "/") return ".";
    return "." + selector;
}

void GopherConnection::send_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        send_error("Could not open file");
        return;
    }
 
    char buf[4096];
    while (file.read(buf, sizeof(buf)) || file.gcount() > 0) {
        std::streamsize n = file.gcount();
        ssize_t sent = 0;
        while (sent < n) {
            ssize_t r = send(socket_fd, buf + sent, n - sent, 0);
            if (r <= 0) return;
            sent += r;
        }
    }
    file.close();
}

void GopherConnection::send_directory(const std::string& path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        send_error("Could not open directory");
        return;
    }
 
    std::string selector_base = (path == "." ? "" : path.substr(1));
    std::string host = "localhost";
    std::string port_str = std::to_string(port);
 
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
 
        std::string full_path = path + "/" + name;
        bool is_dir = is_directory(full_path);
        
        char type = get_gopher_type(name, is_dir);
        std::string selector = selector_base + "/" + name;
        std::string display = name; // Sin barra final en Gopher!
 
        std::string line = std::string(1, type) + display + "\t" + selector + "\t" + host + "\t" + port_str + "\r\n";
        send(socket_fd, line.c_str(), line.length(), 0);
    }
    closedir(dir);
    
    const char *term = ".\r\n";
    send(socket_fd, term, strlen(term), 0);
}

void GopherConnection::send_error(const std::string& message) {
    std::string error_line = "3" + message + "\terror\tlocalhost\t0\r\n";
    send(socket_fd, error_line.c_str(), error_line.length(), 0);
}

void GopherConnection::handle_request() {
    std::string selector = read_selector();

    if (!selector.empty() && selector[0] != '/') {
        selector = "/" + selector;
    }
    
    if (!is_safe_path(selector)) {
        send_error("Invalid selector");
        return;
    }
    
    std::string path = selector_to_path(selector);
    
    if (is_directory(path)) {
        send_directory(path);
    } else if (is_regular_file(path)) {
        send_file(path);
    } else {
        send_error("File not found");
    }
}