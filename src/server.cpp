#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fstream>
#include <sstream>
#include <signal.h>
#include <atomic>


#include "filter.h"
#include "logger.h"
#include "server.h"
#include "client_handler.h"

using namespace std;

static int g_server_fd = -1;
static std::atomic<bool> g_shutdown(false);

static ProxyConfig load_config(const std::string& path) {
    ProxyConfig cfg;
    std::ifstream file(path);
    std::string line;

    if (!file.is_open()) {
        return cfg; // defaults
    }

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string key, eq, value;

        if (!(iss >> key >> eq >> value)) continue;
        if (eq != "=") continue;

        if (key == "port") {
            cfg.port = std::stoi(value);
        } else if (key == "log_file") {
            cfg.log_file = value;
        } else if (key == "blocklist") {
            cfg.blocklist = value;
        }
    }

    return cfg;
}

static void handle_sigint(int) {
    g_shutdown.store(true);

    if (g_server_fd != -1) {
        close(g_server_fd);
        g_server_fd = -1;
    }

    log_event("PROXY_SHUTDOWN_SIGNAL");
}

void start_server(int /*unused*/) {
    signal(SIGINT, handle_sigint);// enable graceful shutdown via CTRL+c
 
    ProxyConfig config = load_config("proxy.conf");
    init_logger(config.log_file);
    init_filter(config.blocklist);// load filtering policy
    int port = config.port;

    g_server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int server_fd = g_server_fd;


    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    bind(server_fd, (sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 10);  // start listening for clients

    cout << "Proxy listening on port " << port << endl;

    while (!g_shutdown.load()) {
    int client_fd = accept(server_fd, NULL, NULL);
        // accept incoming client connection

    if (client_fd < 0) {
        if (g_shutdown.load()) {
            break;  // shutdown requested
        }
        continue;   // transient error
    }

    cout << "Client connected" << endl;

    pthread_t tid;
    int* pclient = new int(client_fd);

    pthread_create(&tid, nullptr, handle_client, pclient);
    pthread_detach(tid);
}
log_event("PROXY_SHUTDOWN_COMPLETE");

}
