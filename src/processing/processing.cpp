#include "processing/processing.h"
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <sstream>
#include <algorithm>

// Constructor
SimpleHttpClient::SimpleHttpClient(int maxRedirects) : maxRedirects(maxRedirects) {}

// Public methods
int SimpleHttpClient::createConnection(const std::string& hostname, int port) {
    struct addrinfo hints, *servinfo, *p;
    int rv;
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portStr = std::to_string(port);
    
    if ((rv = getaddrinfo(hostname.c_str(), portStr.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
        return -1;
    }
    
    int sockfd;
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        freeaddrinfo(servinfo);
        return -1;
    }
    
    freeaddrinfo(servinfo);
    return sockfd;
}

std::string SimpleHttpClient::formatHttpRequest(const std::string& hostname, 
                                               const std::string& path, 
                                               const std::string& method) {
    std::stringstream request;
    request << method << " " << path << " HTTP/1.1\r\n";
    request << "Host: " << hostname << "\r\n";
    request << "User-Agent: SimpleHTTPClient/1.0\r\n";
    request << "Accept: */*\r\n";
    request << "Connection: close\r\n";
    request << "\r\n";
    
    return request.str();
}

bool SimpleHttpClient::sendHttpRequest(int sockfd, const std::string& request) {
    int total = 0;
    int bytesleft = request.length();
    int n;
    
    while(total < request.length()) {
        n = send(sockfd, request.c_str() + total, bytesleft, 0);
        if (n == -1) { 
            return false; 
        }
        total += n;
        bytesleft -= n;
    }
    
    return true;
}

std::string SimpleHttpClient::receiveHttpResponse(int sockfd) {
    std::string response;
    char buffer[4096];
    int bytesReceived;
    
    while ((bytesReceived = recv(sockfd, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        response += buffer;
    }
    
    return response;
}

HttpResponse SimpleHttpClient::parseHttpResponse(const std::string& rawResponse) {
    HttpResponse response;
    response.isSuccess = false;
    
    if (rawResponse.empty()) {
        response.errorMessage = "Empty response received";
        return response;
    }
    
    size_t headerEndPos = rawResponse.find("\r\n\r\n");
    if (headerEndPos == std::string::npos) {
        response.errorMessage = "Invalid HTTP response format - no header separator found";
        return response;
    }
    
    std::string headerSection = rawResponse.substr(0, headerEndPos);
    response.body = rawResponse.substr(headerEndPos + 4);
    
    // Handle chunked encoding if present
    if (isChunkedEncoding(headerSection)) {
        response.body = decodeChunkedBody(response.body);
    }
    
    std::istringstream headerStream(headerSection);
    std::string line;
    bool isFirstLine = true;
    
    while (std::getline(headerStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        if (isFirstLine) {
            if (!parseStatusLine(line, response)) {
                return response;
            }
            isFirstLine = false;
        } else {
            parseHeaderLine(line, response);
        }
    }
    
    response.isSuccess = true;
    return response;
}

HttpResponse SimpleHttpClient::makeHttpRequest(const std::string& hostname, 
                                              const std::string& path, 
                                              int port, 
                                              const std::string& method) {
    HttpResponse response;
    response.isSuccess = false;
    
    // Create connection
    int sockfd = createConnection(hostname, port);
    if (sockfd == -1) {
        response.errorMessage = "Failed to establish connection to " + hostname;
        return response;
    }
    
    // Format and send request
    std::string request = formatHttpRequest(hostname, path, method);
    if (!sendHttpRequest(sockfd, request)) {
        response.errorMessage = "Failed to send HTTP request";
        close(sockfd);
        return response;
    }
    
    // Receive and parse response
    std::string rawResponse = receiveHttpResponse(sockfd);
    close(sockfd);
    
    response = parseHttpResponse(rawResponse);
    return response;
}

void SimpleHttpClient::processResponse(const HttpResponse& response) {
    if (!response.isSuccess) {
        std::cout << "Error: " << response.errorMessage << std::endl;
        return;
    }
    
    std::cout << "\n=== HTTP Response Processing ===" << std::endl;
    std::cout << "Status: " << response.statusCode << " " << response.reasonPhrase << std::endl;
    
    // Handle different status code ranges
    if (isSuccessStatusCode(response.statusCode)) {
        handleSuccessResponse(response);
    } else if (isRedirectStatusCode(response.statusCode)) {
        handleRedirectResponse(response);
    } else if (isClientErrorStatusCode(response.statusCode)) {
        handleClientErrorResponse(response);
    } else if (isServerErrorStatusCode(response.statusCode)) {
        handleServerErrorResponse(response);
    } else {
        std::cout << "Unknown status code range" << std::endl;
    }
    
    displayImportantHeaders(response);
    displayBodyInfo(response);
}

void SimpleHttpClient::setMaxRedirects(int maxRedirects) {
    this->maxRedirects = maxRedirects;
}

int SimpleHttpClient::getMaxRedirects() const {
    return maxRedirects;
}

HttpResponse SimpleHttpClient::get(const std::string& url) {
    // Parse URL to extract hostname, port, and path
    std::string hostname, path;
    int port = 80;
    
    // Simple URL parsing (assumes format: hostname[:port]/path or hostname/path)
    size_t slashPos = url.find('/');
    if (slashPos == std::string::npos) {
        hostname = url;
        path = "/";
    } else {
        hostname = url.substr(0, slashPos);
        path = url.substr(slashPos);
    }
    
    // Check for port in hostname
    size_t colonPos = hostname.find(':');
    if (colonPos != std::string::npos) {
        port = std::stoi(hostname.substr(colonPos + 1));
        hostname = hostname.substr(0, colonPos);
    }
    
    return makeHttpRequest(hostname, path, port, "GET");
}

// Static utility methods
bool SimpleHttpClient::isSuccessStatusCode(int statusCode) {
    return statusCode >= 200 && statusCode < 300;
}

bool SimpleHttpClient::isRedirectStatusCode(int statusCode) {
    return statusCode >= 300 && statusCode < 400;
}

bool SimpleHttpClient::isClientErrorStatusCode(int statusCode) {
    return statusCode >= 400 && statusCode < 500;
}

bool SimpleHttpClient::isServerErrorStatusCode(int statusCode) {
    return statusCode >= 500;
}

// Private helper methods
bool SimpleHttpClient::parseStatusLine(const std::string& line, HttpResponse& response) {
    std::istringstream statusStream(line);
    if (!(statusStream >> response.httpVersion >> response.statusCode)) {
        response.errorMessage = "Invalid status line format";
        return false;
    }
    
    std::string word;
    response.reasonPhrase = "";
    while (statusStream >> word) {
        if (!response.reasonPhrase.empty()) {
            response.reasonPhrase += " ";
        }
        response.reasonPhrase += word;
    }
    
    return true;
}

void SimpleHttpClient::parseHeaderLine(const std::string& line, HttpResponse& response) {
    size_t colonPos = line.find(':');
    if (colonPos != std::string::npos) {
        std::string key = line.substr(0, colonPos);
        std::string value = line.substr(colonPos + 1);
        
        // Trim whitespace
        size_t start = value.find_first_not_of(" \t");
        size_t end = value.find_last_not_of(" \t");
        if (start != std::string::npos) {
            value = value.substr(start, end - start + 1);
        } else {
            value = "";
        }
        
        // Convert header key to lowercase for easier lookup
        std::transform(key.begin(), key.end(), key.begin(), ::tolower);
        response.headers[key] = value;
    }
}

bool SimpleHttpClient::isChunkedEncoding(const std::string& headerSection) {
    std::string lowerHeaders = headerSection;
    std::transform(lowerHeaders.begin(), lowerHeaders.end(), lowerHeaders.begin(), ::tolower);
    return lowerHeaders.find("transfer-encoding: chunked") != std::string::npos;
}

std::string SimpleHttpClient::decodeChunkedBody(const std::string& chunkedBody) {
    std::string decodedBody;
    std::istringstream stream(chunkedBody);
    std::string line;
    
    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        
        size_t chunkSize;
        std::stringstream ss;
        ss << std::hex << line;
        ss >> chunkSize;
        
        if (chunkSize == 0) {
            break;
        }
        
        std::string chunkData;
        for (size_t i = 0; i < chunkSize && std::getline(stream, line); ++i) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            chunkData += line;
            if (i < chunkSize - 1) chunkData += "\n";
        }
        
        decodedBody += chunkData;
    }
    
    return decodedBody;
}

void SimpleHttpClient::handleSuccessResponse(const HttpResponse& response) {
    std::cout << "✓ Success! Request completed successfully." << std::endl;
    
    auto contentType = response.headers.find("content-type");
    if (contentType != response.headers.end()) {
        std::cout << "Content Type: " << contentType->second << std::endl;
    }
}

void SimpleHttpClient::handleRedirectResponse(const HttpResponse& response) {
    std::cout << "↻ Redirect response." << std::endl;
    
    auto location = response.headers.find("location");
    if (location != response.headers.end()) {
        std::cout << "Redirect location: " << location->second << std::endl;
        std::cout << "Note: This client doesn't automatically follow redirects." << std::endl;
    }
}

void SimpleHttpClient::handleClientErrorResponse(const HttpResponse& response) {
    std::cout << "✗ Client Error." << std::endl;
    
    switch (response.statusCode) {
        case 400:
            std::cout << "Bad Request - The server couldn't understand the request." << std::endl;
            break;
        case 401:
            std::cout << "Unauthorized - Authentication required." << std::endl;
            break;
        case 403:
            std::cout << "Forbidden - Access denied." << std::endl;
            break;
        case 404:
            std::cout << "Not Found - The requested resource doesn't exist." << std::endl;
            break;
        default:
            std::cout << "Client error occurred." << std::endl;
    }
}

void SimpleHttpClient::handleServerErrorResponse(const HttpResponse& response) {
    std::cout << "⚠ Server Error." << std::endl;
    
    switch (response.statusCode) {
        case 500:
            std::cout << "Internal Server Error - Something went wrong on the server." << std::endl;
            break;
        case 502:
            std::cout << "Bad Gateway - Invalid response from upstream server." << std::endl;
            break;
        case 503:
            std::cout << "Service Unavailable - Server temporarily unavailable." << std::endl;
            break;
        default:
            std::cout << "Server error occurred." << std::endl;
    }
}

void SimpleHttpClient::displayImportantHeaders(const HttpResponse& response) {
    std::cout << "\nImportant Headers:" << std::endl;
    
    std::vector<std::string> importantHeaders = {
        "content-length", "content-type", "server", "date", 
        "cache-control", "set-cookie", "location"
    };
    
    for (const std::string& headerName : importantHeaders) {
        auto it = response.headers.find(headerName);
        if (it != response.headers.end()) {
            std::cout << "  " << headerName << ": " << it->second << std::endl;
        }
    }
}

void SimpleHttpClient::displayBodyInfo(const HttpResponse& response) {
    std::cout << "\nResponse Body:" << std::endl;
    std::cout << "Length: " << response.body.length() << " bytes" << std::endl;
    
    if (!response.body.empty()) {
        auto contentType = response.headers.find("content-type");
        bool isText = true;
        
        if (contentType != response.headers.end()) {
            std::string type = contentType->second;
            isText = (type.find("text/") == 0 || 
                     type.find("application/json") != std::string::npos ||
                     type.find("application/xml") != std::string::npos);
        }
        
        if (isText) {
            std::cout << "\nContent preview:" << std::endl;
            std::string preview = response.body.substr(0, 300);
            std::cout << preview;
            if (response.body.length() > 300) {
                std::cout << "\n... (content truncated)";
            }
            std::cout << std::endl;
        } else {
            std::cout << "Binary content (not displayed)" << std::endl;
        }
    }
}
