//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2º de grado de Ingeniería Informática
//                       
//                   Gopher Protocol Connection Handler
//                   RFC 1436
// 
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

GopherConnection::GopherConnection(int socket) : socket_fd(socket) {
}

GopherConnection::~GopherConnection() {
    close(socket_fd);
}

// TODO: Students must implement this function
// Read the selector string from the client
// The selector is terminated by CR+LF (\r\n)
// Maximum selector size is MAX_SELECTOR_SIZE
std::string GopherConnection::read_selector() {
    // TODO: Read from socket_fd until \r\n is found
    // TODO: Return the selector string (without \r\n)
    // TODO: Handle errors and buffer overflow
    
    return "";  // Replace with actual implementation
}

// TODO: Students must implement this function
// Convert a Gopher selector to a filesystem path
// For example: "/" -> ".", "/file.txt" -> "./file.txt"
// Must ensure the path is safe (no directory traversal)
std::string GopherConnection::selector_to_path(const std::string& selector) {
    // TODO: Validate selector using is_safe_path()
    // TODO: Convert selector to filesystem path
    // TODO: Handle empty selector (should map to current directory)
    
    return "";  // Replace with actual implementation
}

// TODO: Students must implement this function
// Send a file to the client
// The file content is sent as-is, followed by closing the connection
void GopherConnection::send_file(const std::string& path) {
    // TODO: Open the file
    // TODO: Read file in chunks and send via socket
    // TODO: Handle errors (file not found, read errors, etc.)
    // TODO: Close file when done
}

// TODO: Students must implement this function
// Send a directory listing in Gopher menu format
// Format: <type><display_string><TAB><selector><TAB><host><TAB><port><CR><LF>
// Example: 0README.txt<TAB>/README.txt<TAB>localhost<TAB>7070<CR><LF>
void GopherConnection::send_directory(const std::string& path) {
    // TODO: Open directory using opendir()
    // TODO: Read directory entries using readdir()
    // TODO: For each entry:
    //       - Determine Gopher type using get_gopher_type()
    //       - Format menu line: type + name + TAB + selector + TAB + host + TAB + port + CRLF
    //       - Send line to client
    // TODO: Send terminating line: ".<CR><LF>"
    // TODO: Close directory
}

// Send an error message in Gopher format
// Error type is '3'
void GopherConnection::send_error(const std::string& message) {
    std::string error_line = "3" + message + "\terror\terror\t0\r\n";
    send(socket_fd, error_line.c_str(), error_line.length(), 0);
}

// Main request handler
void GopherConnection::handle_request() {
    // Read the selector from the client
    std::string selector = read_selector();
    
    // Validate the selector
    if (!is_safe_path(selector)) {
        send_error("Invalid selector");
        return;
    }
    
    // Convert selector to filesystem path
    std::string path = selector_to_path(selector);
    
    // Check if path exists and determine type
    if (is_directory(path)) {
        send_directory(path);
    } else if (is_regular_file(path)) {
        send_file(path);
    } else {
        send_error("File not found");
    }
}
