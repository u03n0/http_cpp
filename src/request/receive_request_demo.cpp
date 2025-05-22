#include <iostream>
#include <string>
#include <cstring>      // For memset
#include <sys/socket.h> // For socket functions
#include <netdb.h>      // For getaddrinfo
#include <unistd.h>     // For close
#include <sstream>      // For stringstream
#include <map>          // For headers storage
#include "request/receive_request.h"
#include "request/send_request.h"
#include "socket/socket.h"
// Structure to hold parsed HTTP response
struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string reasonPhrase;
    std::map<std::string, std::string> headers;
    std::string body;
};


int main() {
    std::string hostname = "example.com";
    int port = 80;
    
    std::cout << "Connecting to " << hostname << ":" << port << std::endl;
    
    int sockfd = createConnection(hostname, port);
    if (sockfd == -1) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully!" << std::endl;
    
    // Format and send HTTP request
    std::string request = formatHttpRequest(hostname, "/");
    std::cout << "\nSending HTTP request..." << std::endl;
    
    if (!sendHttpRequest(sockfd, request)) {
        std::cerr << "Failed to send HTTP request" << std::endl;
        close(sockfd);
        return 1;
    }
    
    std::cout << "Request sent! Waiting for response..." << std::endl;
    
    // Receive the HTTP response
    std::string rawResponse = receiveHttpResponse(sockfd);
    if (rawResponse.empty()) {
        std::cerr << "Failed to receive response" << std::endl;
        close(sockfd);
        return 1;
    }
    
    // Parse the response
    HttpResponse response = parseHttpResponse(rawResponse);
    
    // Print the parsed response
    printHttpResponse(response);
    
    // Close the socket
    close(sockfd);
    
    return 0;
}
