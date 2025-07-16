# Webserv

A fully-featured HTTP/1.1 web server built from scratch in C++98, inspired by nginx architecture. This project demonstrates advanced network programming concepts including socket management, HTTP/1.1 protocol implementation, and high-performance concurrent request handling using non-blocking I/O and multiplexing.

## Authors
Webserv is developed as a collaborative team project with [Matthieu Manuellan](https://github.com/JVlatt) and [Nadia Galleze](https://github.com/Nadzg), showcasing deep understanding of network programming fundamentals, web server architecture, and full-stack web development.

## Key Features

### Complete HTTP/1.1 Implementation

- Full HTTP Method Support: GET, POST, DELETE, PUT, HEAD, PATCH
- RESTful API Architecture: Clean endpoint design with proper routing
- Data Transfer: Chunked encoding for large payloads
- JSON Response Formatting: Structured data exchange
- Comprehensive Status Codes: Proper HTTP status code responses
- Concurrent Connections: Up to 1024 simultaneous clients
- Content-Type Handling: Support for various MIME types
- Testing: Thoroughly tested with various clients (curl, netcat, browsers)

### Session & Authentication System

- Cookie-Based Session Management: Secure session tracking
- User Registration & Authentication: Username, email, password system
- Session Token Generation: Using secure tokens
- Session Timeout Handling: Isolated session management
- Persistent Client State: Reliable state management across requests

### Multi-CGI Script Processing

- Separate CGI environments
- PHP Script Execution: Full PHP CGI support
- Python CGI Implementation: Python script processing
- Shell/Bash Script Processing: System script execution
- Environment Variable Management: Proper CGI environment setup
- Multiple Script Support: Concurrent CGI processing

### Core Server Features

- Static file serving
- Multiple concurrent connections via select()
- Custom error page handling
- Request timeout management

### High-Performance Server Architecture

- Non-Blocking I/O: Fully asynchronous operations
- select() Multiplexing: Efficient concurrent connection handling
- Chunked Data Transfer: CHUNK_SIZE configurable for optimal performance
- FD_SETSIZE Support: Handle up to 1024 concurrent clients
- Concurrent client request processing
- Memory Leak Free: Efficient memory management
- Zero-Copy Operations: Optimized data handling without unnecessary buffering
  
### User Management & File Handling

- User Registration System: Complete signup/login workflow
- Profile Photo Management: Upload, replace, delete profile pictures
- User Information Updates: Edit profile functionality
- File Upload Handling: File processing with size limits
- Image Processing: Profile photo handling and validation

### Interactive Web Applications

- Calculator Application: Built-in mathematical calculator
- Timezone Calculator: Time zone conversion utility
- User Interface: Clean, responsive web interface

### Multi-Domain Virtual Hosting

- Virtual Server Blocks: Multiple website hosting
- Independent document roots
- Static Asset Optimization: Efficient static file serving
- Custom Error Pages: Real-time error handling
- Middleware Support: Extensible request processing

### Core Server Configuration Options

- server: Virtual server block definition
- listen: Network binding and port configuration
- root: Document directory specification
- location: URL pattern matching and handling (here localhost for debugging purposes)
- cgi_pass: Script interpreter routing
- session_timeout: Session lifecycle management
- methods: Allowed HTTP methods per location
- index: Default directory index files

## Usage Examples

### Server initialization
```
make
./webserv <config_file>
# access in browser
http://localhost:<port>
```
The project includes four pre-configured test scenarios to demonstrate various server capabilities. You can also create your own configuration files using familiar nginx-style syntax and format.

### Development & Debugging
The server includes terminal logging output for development purposes (some in comments):

- Request logging and processing details
- Connection management information
- CGI execution status
- Error tracking and debugging info

## Implementation Notes
This implementation focuses on educational purposes and demonstrates professional-grade web server development using C++98 standards. The project emphasizes practical understanding of HTTP/1.1 protocol, advanced socket programming, and efficient resource management in a concurrent environment.
