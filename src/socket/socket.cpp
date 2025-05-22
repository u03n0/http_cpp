#include <iostream>
#include <string>
#include <cstring>      // For memset
#include <sys/socket.h> // For socket functions
#include <netdb.h>      // For getaddrinfo
#include <unistd.h>     // For close

using std::string;

int createConnection(const string& hostname, int port) {
    // Step 1: Set up the address info structure
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;     // Use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM; // TCP stream sockets or SOCK_DGRAM for Datagram
    
    // Convert port to string for getaddrinfo
    string portStr = std::to_string(port);
    
    // Get address information
    if ((rv = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }
    
    int sockfd;
    // Step 2: Loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        // Step 2.1: Create a socket
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "socket: " << strerror(errno) << std::endl;
            continue;
        }
        
        // Step 2.2: Connect to the server
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::cerr << "connect: " << strerror(errno) << std::endl;
            continue;
        }
        
        break; // If we get here, we made a successful connection
    }
    
    // Check if connection succeeded
    if (p == NULL) {
        std::cerr << "Failed to connect" << std::endl;
        return -1;
    }
    
    // Step 3: Free the linked list
    freeaddrinfo(servinfo);
    
    return sockfd; // Return the socket file descriptor
}
