# Proxy Architecture

## Threading Model
- Main thread accepts connections
- Each client handled in its own pthread
- No shared mutable request state

## HTTP Handling
- Parse request
- Apply filter
- Forward request
- Stream response

## HTTPS Handling
- CONNECT method
- TCP tunnel
- select()-based relay
- TLS untouched (end-to-end)

## Shutdown
- SIGINT handler
- Stop accept loop
- Existing threads finish
