#include "http_parser.h"
#include <sstream>

using namespace std;

HttpRequest parse_http_request(const string& request) {
    HttpRequest req;
    req.port = 80;

    istringstream iss(request);
    string line;

    // -------- Parse request line --------
    getline(iss, line);
    istringstream rl(line);

    rl >> req.method;
    string url;
    rl >> url;

    if (url.find("http://") == 0) {
        url = url.substr(7);
        size_t pos = url.find('/');
        if (pos != string::npos) {
            req.host = url.substr(0, pos);
            req.path = url.substr(pos);
        } else {
            req.host = url;
            req.path = "/";
        }
    } else {
        req.path = url;
    }

    // -------- Parse headers --------
    while (getline(iss, line)) {
        if (line == "\r" || line.empty())
            break;

        if (line.find("Host:") == 0) {
            string host = line.substr(5);

            // trim leading spaces
            while (!host.empty() && host[0] == ' ')
                host.erase(host.begin());

            // trim trailing '\r'
            if (!host.empty() && host.back() == '\r')
                host.pop_back();

            size_t colon = host.find(':');
            if (colon != string::npos) {
                req.host = host.substr(0, colon);
                req.port = stoi(host.substr(colon + 1));
            } else {
                req.host = host;
            }
        }
    }

    return req;
}
