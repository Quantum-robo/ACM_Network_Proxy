#include <iostream>
#include <atomic>
#include <unistd.h>
#include <cstring>
#include <pthread.h>
#include <sys/socket.h>
#include <fstream>
#include <unordered_set>
#include <algorithm>
#include <netdb.h>
#include <arpa/inet.h>
#include <unordered_map>
#include <vector>

#include "filter.h"
#include "logger.h"
#include "client_handler.h"
#include "http_parser.h"
#include "proxy_forwarder.h"

using namespace std;

static std::atomic<unsigned long> g_client_id{0};

static bool parse_connect_target(const std::string& target,
                                 std::string& host,
                                 int& port);

static int connect_to_host(const std::string& host, int port);
static void tunnel_relay(int client_fd, int remote_fd);

void* handle_client(void* arg) {
    unsigned long cid = ++g_client_id; //assigns a client id
    log_event("------------------------------------------------------------");
    //session seperator 

    auto log = [&](const std::string& msg) {
    log_event("[CID " + std::to_string(cid) + "] " + msg);
    };

    log("CLIENT_CONNECTED");//If new client connection has been accepted;

    int client_fd = *(int*)arg;
    delete (int*)arg;

    char buffer[4096]; //temp buffer to recieve HTTP req data from client 
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    // Receive raw bytes from client socket (blocking call)

   if (n <= 0) {  // recv() failed or client closed connection
    log("RECV_ERROR");
    close(client_fd);
    log("CLIENT_DISCONNECTED");
    return nullptr;
}


    buffer[n] = '\0';
    string raw_request(buffer); // preserve raw bytes for forwarding

    HttpRequest req = parse_http_request(raw_request);
    bool is_connect = (req.method == "CONNECT"); // HTTPS tunneling?

    if (is_connect) {
    log("CONNECT_REQUEST_DETECTED " + req.path);

    std::string connect_host;
    int connect_port;

    if (!parse_connect_target(req.path, connect_host, connect_port)) {
        log("CONNECT_PARSE_ERROR " + req.path);
        close(client_fd);
        log("CLIENT_DISCONNECTED");
        return nullptr;
    }

    log("CONNECT_TARGET " + connect_host + ":" +
              std::to_string(connect_port));

   
    int remote_fd = connect_to_host(connect_host, connect_port);
     // open TCP connection to target server

    if (remote_fd < 0) {
        log("CONNECT_FAILED " + connect_host + ":" +
                  std::to_string(connect_port));

        const char* bad_gateway =
            "HTTP/1.1 502 Bad Gateway\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n"
            "\r\n";

        send(client_fd, bad_gateway, strlen(bad_gateway), 0);
        close(client_fd);
        log("CLIENT_DISCONNECTED");
        return nullptr;
    }

    log("CONNECT_ESTABLISHED " + connect_host + ":" +
              std::to_string(connect_port));

    const char* established =
        "HTTP/1.1 200 Connection Established\r\n"
        "\r\n";

   send(client_fd, established, strlen(established), 0);

// Start bidirectional tunnel
log("TUNNEL_START " + connect_host + ":" +
          std::to_string(connect_port));

tunnel_relay(client_fd, remote_fd);

close(remote_fd);
close(client_fd);

log("TUNNEL_CLOSED " + connect_host + ":" +
          std::to_string(connect_port));
log("CLIENT_DISCONNECTED");
return nullptr;

}



if (is_blocked(req)) {
    log("BLOCK " + req.host);

    const char* forbidden =
        "HTTP/1.1 403 Forbidden\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n"
        "\r\n";

    send(client_fd, forbidden, strlen(forbidden), 0);
    close(client_fd);
    log("CLIENT_DISCONNECTED");
    return nullptr;
}

log("FORWARD " + req.host + ":" + std::to_string(req.port));


forward_request(client_fd, req, raw_request);


    close(client_fd);
    log("CLIENT_DISCONNECTED");
    return nullptr;
}
static bool parse_connect_target(const std::string& target,
                                 std::string& host,
                                 int& port) { // split host:port
    size_t colon = target.find(':');
    if (colon == std::string::npos) {
        return false;
    }

    host = target.substr(0, colon);

    try {
        port = std::stoi(target.substr(colon + 1));
    } catch (...) {
        return false;
    }

    return true;
}
static int connect_to_host(const std::string& host, int port) {
    // DNS resolution + TCP connect
    addrinfo hints{}, *res, *p;
    int sockfd = -1;

    hints.ai_family = AF_INET;       // IPv4 for now
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), std::to_string(port).c_str(),
                    &hints, &res) != 0) {
        return -1;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd < 0) continue;

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;  // success
        }

        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);
    return sockfd;  // -1 on failure
}
static void tunnel_relay(int client_fd, int remote_fd) {
    // bidirectional relay using select()
    fd_set readfds;
    char buffer[8192];

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
        FD_SET(remote_fd, &readfds);

        int maxfd = std::max(client_fd, remote_fd) + 1;

        int ready = select(maxfd, &readfds, nullptr, nullptr, nullptr);
        if (ready <= 0) {
            break;
        }

        // Client → Server
        if (FD_ISSET(client_fd, &readfds)) {
            int n = recv(client_fd, buffer, sizeof(buffer), 0);
            if (n <= 0) break;
            send(remote_fd, buffer, n, 0);
        }

        // Server → Client
        if (FD_ISSET(remote_fd, &readfds)) {
            int n = recv(remote_fd, buffer, sizeof(buffer), 0);
            if (n <= 0) break;
            send(client_fd, buffer, n, 0);
        }
    }
}
