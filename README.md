# Webserv

Webserv is an HTTP/1.1 web server built from scratch, inspired by nginx architecture. This project demonstrates core network programming concepts, including socket management, HTTP/1.1 protocol implementation, and concurrent request handling using standard multiplexing.

## Authors
Webserv is developed as a team project with [Matthieu Manuellan](https://github.com/JVlatt) and [Nadia Galleze](https://github.com/Nadzg), aimed at understanding network programming fundamentals and web server architecture.

## Key Features

### HTTP Method Support

- Complete HTTP method implementation (GET, POST, DELETE, PUT, HEAD, PATCH)
- RESTful API endpoint handling
- JSON response formatting
- Proper HTTP status code responses

### Session & Cookie Management

- Cookie-based session tracking
- Session token generation and validation
- Session timeout handling
- Client state persistence

### CGI Script Processing

- PHP script execution support
- Python CGI implementation
- Shell/Bash script processing
- Environment variable management for CGI

### Core Server Features

- Static file serving
- Multiple concurrent connections via select()
- Custom error page handling
- Request timeout management

### I/O & Connection Handling

- Non-blocking socket operations
- select() based multiplexing
- Concurrent client request processing
- Efficient memory management

### Core Server Capabilities

- Multi-domain virtual hosting
- Flexible routing configuration
- Static asset optimization
- Real-time error handling
- Custom middleware support

### Configuration Options

- server: Virtual server block definition
- listen: Network binding and port configuration
- root: Document directory specification
- location: URL pattern matching and handling
- cgi_pass: Script interpreter routing
- session_timeout: Session lifecycle management
- max_connections: Concurrent client limits

## Usage Examples

### Server initialization
```
bash./webserv config/webserv.conf
```
### POST with JSON payload
```
curl -X POST -H "Content-Type: application/json" \
     -d '{"name":"John","email":"john@example.com"}' \
     http://localhost:8080/api/users
```
### PUT request for updates
```
curl -X PUT -H "Content-Type: application/json" \
     -d '{"name":"Jane Doe"}' \
     http://localhost:8080/api/users/1
```
### DELETE operation
```
curl -X DELETE http://localhost:8080/api/users/1
CGI script execution
bash# PHP processing
curl http://localhost:8080/scripts/process.php?action=login
```
### Bash system monitoring
```
curl http://localhost:8080/shell/system-status.sh
```
## Implementation Notes
This web server implementation follows C++98 standards with focus on clean code architecture and reliable performance. The project emphasizes practical understanding of HTTP/1.1 protocol, socket programming with select(), and proper resource management. All network operations are handled through non-blocking I/O to support multiple simultaneous client connections efficiently.
