#include "proxy_forwarder.h"

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>

using namespace std;

void forward_request(
    int client_fd,
    const HttpRequest& req,
    const string& raw_request
) {
    // DNS lookup
    addrinfo hints{}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    string port_str = to_string(req.port);

if (getaddrinfo(req.host.c_str(),
                port_str.c_str(),
                &hints,
                &res) != 0) 
 {
        cerr << "DNS lookup failed for " << req.host << endl;
        return;
    }

    // Create socket
    int remote_fd = socket(res->ai_family,
                           res->ai_socktype,
                           res->ai_protocol);
    if (remote_fd < 0) {
        freeaddrinfo(res);
        return;
    }

    // Connect to destination server
    if (connect(remote_fd, res->ai_addr, res->ai_addrlen) < 0) {
        perror("connect");
        close(remote_fd);
        freeaddrinfo(res);
        return;
    }

    freeaddrinfo(res);

    // Forward request
    send(remote_fd, raw_request.c_str(), raw_request.size(), 0);

    // Relay response
    char buffer[8192];
    int n;
    while ((n = recv(remote_fd, buffer, sizeof(buffer), 0)) > 0) {
        send(client_fd, buffer, n, 0);
    }

    close(remote_fd);
}
