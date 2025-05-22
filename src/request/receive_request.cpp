#include <iostream>
#include <string>
#include <cstring>      // For memset
#include <sys/socket.h> // For socket functions
#include <netdb.h>      // For getaddrinfo
#include <unistd.h>     // For close
#include <sstream>      // For stringstream
#include <map>          // For headers storage
                        //
                        //
using std::string;



struct HttpResponse {
    std::string httpVersion;
    int statusCode;
    std::string reasonPhrase;
    std::map<std::string, std::string> headers;
    std::string body;
};

// New function to receive the complete HTTP response
std::string receiveHttpResponse(int sockfd) {
    std::string response;
    char buffer[4096];
    int bytesReceived;
    
    // Keep receiving until the server closes the connection
    while ((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';  // Null-terminate the buffer
        response += buffer;
    }
    
    if (bytesReceived == -1) {
        std::cerr << "Error receiving response: " << strerror(errno) << std::endl;
        return "";
    }
    
    return response;
}

// New function to parse the HTTP response
HttpResponse parseHttpResponse(const std::string& rawResponse) {
    HttpResponse response;
    
    // Find the position where headers end (first occurrence of \r\n\r\n)
    size_t headerEndPos = rawResponse.find("\r\n\r\n");
    if (headerEndPos == std::string::npos) {
        std::cerr << "Invalid HTTP response format" << std::endl;
        return response;
    }
    
    // Split headers and body
    std::string headerSection = rawResponse.substr(0, headerEndPos);
    response.body = rawResponse.substr(headerEndPos + 4); // +4 to skip \r\n\r\n
    
    // Parse headers line by line
    std::istringstream headerStream(headerSection);
    std::string line;
    bool isFirstLine = true;
    
    while (std::getline(headerStream, line)) {
        // Remove \r if present (since getline only removes \n)
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (isFirstLine) {
            // Parse status line: HTTP/1.1 200 OK
            std::istringstream statusStream(line);
            statusStream >> response.httpVersion >> response.statusCode;
            
            // Get the reason phrase (everything after status code)
            std::string word;
            response.reasonPhrase = "";
            while (statusStream >> word) {
                if (!response.reasonPhrase.empty()) {
                    response.reasonPhrase += " ";
                }
                response.reasonPhrase += word;
            }
            
            isFirstLine = false;
        } else {
            // Parse header: Key: Value
            size_t colonPos = line.find(':');
            if (colonPos != std::string::npos) {
                std::string key = line.substr(0, colonPos);
                std::string value = line.substr(colonPos + 1);
                
                // Trim leading/trailing whitespace from value
                size_t start = value.find_first_not_of(" \t");
                size_t end = value.find_last_not_of(" \t");
                if (start != std::string::npos) {
                    value = value.substr(start, end - start + 1);
                } else {
                    value = "";
                }
                
                response.headers[key] = value;
            }
        }
    }
    
    return response;
}

// New function to print the parsed response
void printHttpResponse(const HttpResponse& response) {
    std::cout << "\n=== HTTP Response ===" << std::endl;
    std::cout << "Status: " << response.httpVersion << " " 
              << response.statusCode << " " << response.reasonPhrase << std::endl;
    
    std::cout << "\nHeaders:" << std::endl;
    for (const auto& header : response.headers) {
        std::cout << "  " << header.first << ": " << header.second << std::endl;
    }
    
    std::cout << "\nBody length: " << response.body.length() << " bytes" << std::endl;
    
    // Print first 500 characters of body for preview
    if (!response.body.empty()) {
        std::cout << "\nBody preview:" << std::endl;
        std::string preview = response.body.substr(0, 500);
        std::cout << preview;
        if (response.body.length() > 500) {
            std::cout << "... (truncated)";
        }
        std::cout << std::endl;
    }
}

