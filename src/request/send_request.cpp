#include <iostream>
#include <string>
#include <cstring>      // For memset
#include <sys/socket.h> // For socket functions
#include <netdb.h>      // For getaddrinfo
#include <unistd.h>     // For close
#include <sstream>      // For stringstream


// New function to format an HTTP request
std::string formatHttpRequest(const std::string& hostname, const std::string& path, 
                             const std::string& method = "GET") {
    std::stringstream request;
    
    // Request line: METHOD PATH HTTP/1.1
    request << method << " " << path << " HTTP/1.1\r\n";
    
    // Headers
    request << "Host: " << hostname << "\r\n";
    request << "User-Agent: SimpleHTTPClient/1.0\r\n";
    request << "Accept: */*\r\n";
    request << "Connection: close\r\n";  // Tell server to close connection after response
    
    // Empty line to indicate end of headers
    request << "\r\n";
    
    return request.str();
}

// New function to send an HTTP request
bool sendHttpRequest(int sockfd, const std::string& request) {
    // Send the request to the server
    int total = 0;
    int bytesleft = request.length();
    int n;
    
    // Keep sending until all bytes are sent
    while(total < request.length()) {
        n = send(sockfd, request.c_str() + total, bytesleft, 0);
        if (n == -1) { 
            std::cerr << "Error sending request: " << strerror(errno) << std::endl;
            return false; 
        }
        total += n;
        bytesleft -= n;
    }
    
    return true;
}
