#include "request/send_request.h"
#include "socket/socket.h"
#include <iostream>
#include <string>
#include <unistd.h>

using std::string;


int main() {
    string hostname {"example.com"};
    int port {80};
    
    std::cout << "Connecting to " << hostname << ":" << port << std::endl;
    
    int sockfd = createConnection(hostname, port);
    if (sockfd == -1) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully! Socket descriptor: " << sockfd << std::endl;
    
    // Format an HTTP request - GET the homepage
    string request = formatHttpRequest(hostname, "/");
    
    // Print the request for debugging
    std::cout << "\nSending HTTP request:\n" << request << std::endl;
    
    // Send the HTTP request
    if (!sendHttpRequest(sockfd, request)) {
        std::cerr << "Failed to send HTTP request" << std::endl;
        close(sockfd);
        return 1;
    }
    
    std::cout << "Request sent successfully!" << std::endl;
    
    // We'll add response handling next
    
    // Close the socket
    close(sockfd);
    
    return 0;
}
