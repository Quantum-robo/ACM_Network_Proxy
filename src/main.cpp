#include <iostream>
#include "server.h"

using namespace std;

//Entry point of the proxy application.
int main() {
    int port = 8888;  // legacy parameter; (actual port is loaded from proxy.conf)
    cout << "Starting proxy server on port " << port << endl;
    start_server(port); //starts the proxy server
    return 0;
}
