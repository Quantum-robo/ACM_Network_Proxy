# C++ HTTP/HTTPS Forward Proxy Server

## What this is
A forward proxy server implemented in C++ using POSIX sockets.
It supports HTTP request forwarding, HTTPS CONNECT tunneling,
domain/IP filtering, logging, and graceful shutdown.

## How it works
- Listens on a TCP port for proxy clients.
- Each client is handled in a separate thread.
- HTTP requests are parsed, filtered, forwarded, and responses are relayed.
- HTTPS uses CONNECT to establish a TCP tunnel (TLS is end-to-end).
- Blocking rules and logging are configured via files.

## Build
make

## Run
./proxy

## Test

### HTTP forwarding
curl -x localhost:8888 http://httpbin.org/ip

### HTTPS tunneling
curl -x localhost:8888 https://httpbin.org/ip

### Domain blocking
Add a domain to "config/blocked_domains.txt" and run:
example: add example.com in 'config blocked_domains.txt',then do
curl -x localhost:8888 http://example.com -v

### Concurrent clients
tests/test_concurrent.sh

## Logs
Logs are written to "logs/proxy.log".

## Shutdown
Press Ctrl+C to stop the proxy gracefully.
(demo video in docs)