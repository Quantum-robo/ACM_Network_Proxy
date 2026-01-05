# Proxy Server Design & Workflow
This document describes how control flows through the proxy server
from program startup to request handling and shutdown.

## 1. Program Startup
Execution begins in main.cpp.

main()
-> start_server()
- main.cpp only prints a startup message.
- All runtime control is delegated to server.cpp.

## 2. Server Initialization (server.cpp)

start_server() performs process-level initialization:

a. Install SIGINT handler for graceful shutdown.
b. Load configuration from proxy.conf.
c. Initialize:
   - Logger (logger.cpp)
   - Filter (filter.cpp)
d. Create, bind, and listen on the server socket.

After initialization, the server enters the accept loop.

## 3. Accept Loop & Concurrency (server.cpp)

while (server running):
accept(client)
create thread → handle_client()

- Each incoming client connection is accepted.
- A detached pthread is created for the client.
- The server thread never processes requests itself.

Concurrency model: thread-per-connection.

## 4. Client Handling (client_handler.cpp)

Each client thread executes handle_client().

###Request handling flow:

handle_client()
--recv() client request
--parse_http_request()
--if CONNECT:
    -parse CONNECT target
    -apply filter
    -connect to destination
    -tunnel_relay()
-- else (HTTP):
--apply filter
--forward_request()

- The raw request is read from the client socket.
- The request is parsed to extract method, host, port, and path.

## 5. Filtering (filter.cpp)

- is_blocked() is called before forwarding or tunneling.
- Blocklist is loaded lazily from blocked_domains.txt.
- Hostnames are normalized.
- DNS resolution is used for IP-based blocking.
- If blocked, a 403 Forbidden response is returned.

## 6. HTTP Forwarding (proxy_forwarder.cpp)

For non-CONNECT requests:

forward_request()
   --DNS resolution
   --connect() to destination server
   --send() raw request
   --recv()/send() response stream

- Responses are streamed, not buffered.
- Connection is closed after request completion.

## 7. HTTPS CONNECT Tunneling (client_handler.cpp)

For CONNECT requests:

CONNECT host:port
  -- connect_to_host()
  --send 200 Connection Established
  --tunnel_relay()

- A TCP tunnel is established.
- select() relays bytes bidirectionally.
- TLS traffic is not inspected or modified.

## 8. Logging (logger.cpp)markdown

- All major events are logged:
  - Client connect/disconnect
  - Forwarded requests
  - Blocked requests
  - CONNECT tunnel lifecycle
  - Server shutdown
- Logging is thread-safe using a mutex.

## 9. Shutdown (server.cpp)

- SIGINT (Ctrl+C) sets shutdown flag.
- Listening socket is closed to interrupt accept().
- Server exits accept loop.
- Existing client threads finish naturally.
- Shutdown is logged before exit.
