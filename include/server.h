#ifndef SERVER_H
#define SERVER_H

#include <string>


// Starts the proxy server and listens on the given port
void start_server(int port);
struct ProxyConfig {
    int port = 8888;
    std::string log_file = "logs/proxy.log";
    std::string blocklist = "config/blocked_domains.txt";
};


#endif
