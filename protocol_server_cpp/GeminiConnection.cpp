//****************************************************************************
//                         REDES Y SISTEMAS DISTRIBUIDOS
//                      
//                     2º de grado de Ingeniería Informática
//                       
//                   Gemini Protocol Connection Handler
// 
//****************************************************************************

#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include "GeminiConnection.h"
#include <sys/socket.h>
#include "common.h"

GeminiConnection::GeminiConnection(int socket, bool use_tls) 
    : socket_fd(socket), use_tls(use_tls), tls_context(nullptr) {
    
    // TODO (OPTIONAL - ADVANCED): Initialize TLS context if use_tls is true
    // This requires OpenSSL library
    // Students can implement this as an advanced feature
}

GeminiConnection::~GeminiConnection() {
    // TODO (OPTIONAL - ADVANCED): Clean up TLS context if it was initialized
    
    close(socket_fd);
}

// TODO: Students must implement this function
// Read the URL from the client
// Format: <URL><CR><LF>
// Maximum URL size is MAX_URL_SIZE (1024 bytes)
// Returns the URL string (without CR+LF)
std::string GeminiConnection::read_url() {
    // TODO: Read from socket_fd until \r\n is found
    // TODO: Validate URL length (must be <= 1024 bytes)
    // TODO: Return the URL string (without \r\n)
    // TODO: Handle errors and buffer overflow
    
    return "";  // Replace with actual implementation
}

// TODO: Students must implement this function
// Parse a Gemini URL and extract the path component
// Format: gemini://hostname[:port]/path
// Example: "gemini://localhost/index.gmi" -> "/index.gmi"
// Example: "gemini://example.com:1965/docs/file.txt" -> "/docs/file.txt"
std::string GeminiConnection::parse_url_path(const std::string& url) {
    // TODO: Check if URL starts with "gemini://"
    // TODO: Find the first "/" after the hostname
    // TODO: Extract and return the path
    // TODO: If no path, return "/"
    // TODO: Validate the path using is_safe_path()
    
    return "";  // Replace with actual implementation
}

// Send a Gemini response header
// Format: <STATUS><SPACE><META><CR><LF>
// Example: "20 text/gemini\r\n"
void GeminiConnection::send_header(int status, const std::string& meta) {
    std::ostringstream header;
    header << status << " " << meta << "\r\n";
    std::string header_str = header.str();
    
    // TODO (OPTIONAL): If using TLS, send via TLS
    // For now, send via regular socket
    send(socket_fd, header_str.c_str(), header_str.length(), 0);
}

// TODO: Students must implement this function
// Send a file to the client with appropriate Gemini header
void GeminiConnection::send_file(const std::string& path) {
    // TODO: Open the file
    // TODO: Get MIME type using get_mime_type()
    // TODO: Send header: "20 <mime_type>\r\n"
    // TODO: Read file in chunks and send via socket
    // TODO: Handle errors (file not found, read errors, etc.)
    // TODO: Close file when done
}

// TODO: Students must implement this function
// Send a directory listing in gemtext format
// Gemtext format for links: "=> <URL> <DISPLAY_TEXT>"
// Example: "=> /docs/file.txt File.txt"
void GeminiConnection::send_directory(const std::string& path) {
    // TODO: Send header: "20 text/gemini\r\n"
    // TODO: Send title: "# Directory listing\n\n"
    // TODO: Open directory using opendir()
    // TODO: Read directory entries using readdir()
    // TODO: For each entry:
    //       - Skip "." and ".."
    //       - Format as gemtext link: "=> /<path>/<name> <name>\n"
    //       - Add "/" suffix for directories
    //       - Send line to client
    // TODO: Close directory
}

// Convert filesystem path to URL path
std::string GeminiConnection::path_to_url(const std::string& path) {
    // Simple implementation: just ensure it starts with /
    if (path.empty() || path[0] != '/') {
        return "/" + path;
    }
    return path;
}

// Main request handler
void GeminiConnection::handle_request() {
    // Read the URL from the client
    std::string url = read_url();
    
    // Check URL length (Gemini spec: max 1024 bytes)
    if (url.length() > MAX_URL_SIZE) {
        send_header(GeminiStatus::BAD_REQUEST, "URL too long");
        return;
    }
    
    // Parse the URL to get the path
    std::string path = parse_url_path(url);
    
    // Validate the path
    if (!is_safe_path(path)) {
        send_header(GeminiStatus::BAD_REQUEST, "Invalid path");
        return;
    }
    
    // Convert to filesystem path (prepend current directory)
    std::string fs_path = "." + path;
    
    // Check if path exists and determine type
    if (is_directory(fs_path)) {
        send_directory(fs_path);
    } else if (is_regular_file(fs_path)) {
        send_file(fs_path);
    } else {
        send_header(GeminiStatus::NOT_FOUND, "Not found");
    }
}
