#include <iostream>
#include "socket/socket.h"
#include <string>
#include <unistd.h>


using std::string;

int main() {
    string hostname = "example.com";
    int port = 80;
    
    std::cout << "Connecting to " << hostname << ":" << port << std::endl;
    
    int sockfd = createConnection(hostname, port);
    if (sockfd == -1) {
        std::cerr << "Connection failed" << std::endl;
        return 1;
    }
    
    std::cout << "Connected successfully! Socket descriptor: " << sockfd << std::endl;
    
    // Don't forget to close the socket when done
    close(sockfd);
  return 0;
}
