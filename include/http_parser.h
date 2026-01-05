#ifndef HTTP_PARSER_H
#define HTTP_PARSER_H

#include <string>

struct HttpRequest {
    std::string method;
    std::string host;
    int port;
    std::string path;
};

HttpRequest parse_http_request(const std::string& request);

#endif
