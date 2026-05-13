//****************************************************************************
//                   Gemini Protocol Connection Handler
//****************************************************************************
 
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "GeminiConnection.h"
#include "common.h"
 
GeminiConnection::GeminiConnection(int socket, bool use_tls)
    : socket_fd(socket), use_tls(use_tls), ctx(nullptr), ssl(nullptr) {
    
    // NIVEL 6: Inicialización de TLS
    if (use_tls) {
        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        ctx = SSL_CTX_new(TLS_server_method());
        if (ctx) {
            SSL_CTX_use_certificate_file(ctx, "cert.pem", SSL_FILETYPE_PEM);
            SSL_CTX_use_PrivateKey_file(ctx, "key.pem", SSL_FILETYPE_PEM);
            ssl = SSL_new(ctx);
            SSL_set_fd(ssl, socket_fd);
            SSL_accept(ssl);
        }
    }
}
 
GeminiConnection::~GeminiConnection() {
    if (use_tls && ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
    }
    if (use_tls && ctx) {
        SSL_CTX_free(ctx);
    }
    close(socket_fd);
}

// Helpers para manejar datos con o sin TLS de forma invisible
int GeminiConnection::my_recv(char* buf, int len) {
    if (use_tls && ssl) return SSL_read(ssl, buf, len);
    return recv(socket_fd, buf, len, 0);
}

int GeminiConnection::my_send(const char* buf, int len) {
    if (use_tls && ssl) return SSL_write(ssl, buf, len);
    return send(socket_fd, buf, len, 0);
}
 
std::string GeminiConnection::read_url() {
    std::string url;
    char c;
 
    while (url.size() <= MAX_URL_SIZE + 2) {
        if (my_recv(&c, 1) <= 0) break;
        if (c == '\r') {
            my_recv(&c, 1); // Consume \n
            break;
        }
        if (c == '\n') break;
        url += c;
    }
    return url;
}
 
std::string GeminiConnection::parse_url_path(const std::string& url) {
    const std::string scheme = "gemini://";
    if (url.substr(0, scheme.size()) != scheme) return "/";
    
    size_t host_start = scheme.size();
    size_t path_start = url.find('/', host_start);
    
    if (path_start == std::string::npos) return "/";
    return url.substr(path_start);
}
 
void GeminiConnection::send_header(int status, const std::string& meta) {
    std::ostringstream header;
    header << status << " " << meta << "\r\n";
    std::string header_str = header.str();
    my_send(header_str.c_str(), header_str.length());
}
 
void GeminiConnection::send_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        send_header(GeminiStatus::NOT_FOUND, "File not found");
        return;
    }
 
    std::string mime = get_mime_type(path);
    send_header(GeminiStatus::SUCCESS, mime);
 
    char buf[4096];
    while (file.read(buf, sizeof(buf)) || file.gcount() > 0) {
        std::streamsize n = file.gcount();
        ssize_t sent = 0;
        while (sent < n) {
            ssize_t r = my_send(buf + sent, n - sent);
            if (r <= 0) return;
            sent += r;
        }
    }
    file.close();
}
 
void GeminiConnection::send_directory(const std::string& path) {
    DIR *dir = opendir(path.c_str());
    if (!dir) {
        send_header(GeminiStatus::NOT_FOUND, "Directory not found");
        return;
    }
 
    send_header(GeminiStatus::SUCCESS, "text/gemini");
 
    // Cabecera exacta que pide el autograder
    const char *title = "# Directory Listing\r\n";
    my_send(title, strlen(title));
 
    std::string url_base = path.substr(1); // quitamos el '.' inicial
    if (url_base.empty() || url_base == "/") {
        url_base = "";
    } else if (url_base.back() == '/') {
        url_base.pop_back();
    }
 
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
 
        std::string full_path = path + "/" + name;
        bool is_dir = is_directory(full_path);
 
        std::string link_url = url_base + "/" + name;
        std::string display = name;
        
        if (is_dir) {
            link_url += "/";
            display += "/";
        }
 
        // Formato estricto de Gemtext
        std::string line = "=> " + link_url + " " + display + "\r\n";
        my_send(line.c_str(), line.length());
    }
    closedir(dir);
}
 
void GeminiConnection::handle_request() {
    std::string url = read_url();
 
    if (url.length() > MAX_URL_SIZE) {
        send_header(GeminiStatus::BAD_REQUEST, "URL too long");
        return;
    }
 
    std::string path = parse_url_path(url);
 
    if (!is_safe_path(path)) {
        send_header(GeminiStatus::BAD_REQUEST, "Invalid path");
        return;
    }
 
    std::string fs_path = "." + path;
 
    if (is_directory(fs_path)) {
        send_directory(fs_path);
    } else if (is_regular_file(fs_path)) {
        send_file(fs_path);
    } else {
        send_header(GeminiStatus::NOT_FOUND, "Not found");
    }
}