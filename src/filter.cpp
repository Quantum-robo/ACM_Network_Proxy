#include "filter.h"

#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <netdb.h>
#include <arpa/inet.h>
#include <vector>
#include <mutex>

using namespace std;

static unordered_set<string> blocked_domains;
static unordered_set<string> blocked_ips;
static unordered_map<string, vector<string>> dns_cache;
static bool loaded = false;
static string blocklist_file;

// Helpers
static string normalize(const string& s) {
    string out = s;
    out.erase(0, out.find_first_not_of(" \t\r\n"));
    out.erase(out.find_last_not_of(" \t\r\n") + 1);
    transform(out.begin(), out.end(), out.begin(), ::tolower);
    return out;
}

static vector<string> resolve_host_ips(const string& host) {
    vector<string> ips;
    addrinfo hints{}, *res;

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0) {
        return ips;
    }

    for (addrinfo* p = res; p; p = p->ai_next) {
        sockaddr_in* addr = (sockaddr_in*)p->ai_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
        ips.emplace_back(ip);
    }

    freeaddrinfo(res);
    return ips;
}

// Public API
void init_filter(const string& path) {
    blocklist_file = path;    // store blocklist path
}

bool is_blocked(const HttpRequest& req) {
    if (!loaded) {
        // lazy-load blocklist on first request
        ifstream file(blocklist_file);
        string line;

        while (getline(file, line)) {
            line = normalize(line);
            if (line.empty()) continue;

            if (isdigit(line[0])) {
                blocked_ips.insert(line);
            } else {
                blocked_domains.insert(line);
            }
        }
        loaded = true;
    }

    string host = normalize(req.host);

    if (blocked_domains.count(host)) {
        return true; // DNS-based IP blocking
    }

    const vector<string>& ips =
        dns_cache.count(host)
            ? dns_cache[host]
            : dns_cache[host] = resolve_host_ips(host);

    for (const string& ip : ips) {
        if (blocked_ips.count(ip)) {
            return true;
        }
    }

    return false;
}
