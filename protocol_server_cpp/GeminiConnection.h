#ifndef GEMINICONNECTION_H
#define GEMINICONNECTION_H

#include <string>
#include <openssl/ssl.h>
#include <openssl/err.h>

const int MAX_URL_SIZE = 1024;

namespace GeminiStatus {
    const int INPUT = 10;
    const int SUCCESS = 20;
    const int REDIRECT_TEMPORARY = 30;
    const int REDIRECT_PERMANENT = 31;
    const int TEMPORARY_FAILURE = 40;
    const int SERVER_UNAVAILABLE = 41;
    const int CGI_ERROR = 42;
    const int PROXY_ERROR = 43;
    const int SLOW_DOWN = 44;
    const int PERMANENT_FAILURE = 50;
    const int NOT_FOUND = 51;
    const int GONE = 52;
    const int PROXY_REQUEST_REFUSED = 53;
    const int BAD_REQUEST = 59;
    const int CLIENT_CERT_REQUIRED = 60;
    const int CERT_NOT_AUTHORIZED = 61;
    const int CERT_NOT_VALID = 62;
}

class GeminiConnection {
public:
    GeminiConnection(int socket, bool use_tls = false);
    ~GeminiConnection();

    void handle_request();

private:
    int socket_fd;
    bool use_tls;
    
    SSL_CTX *ctx;
    SSL *ssl;
    

    int my_recv(char* buf, int len);
    int my_send(const char* buf, int len);
    
    std::string read_url();
    std::string parse_url_path(const std::string& url);
    void send_header(int status, const std::string& meta);
    void send_file(const std::string& path);
    void send_directory(const std::string& path);
};

#endif