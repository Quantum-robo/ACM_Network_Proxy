#ifndef PROXY_FORWARDER_H
#define PROXY_FORWARDER_H

#include <string>
#include "http_parser.h"

// Connects to destination server and relays response back to client
void forward_request(
    int client_fd,
    const HttpRequest& req,
    const std::string& raw_request
);

#endif
