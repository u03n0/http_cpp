#ifndef PROCESSING_H
#define PROCESSING_H

#include <string>
#include <map>
#include <vector>

/**
 * Structure to hold parsed HTTP response data
 */
struct HttpResponse {
    std::string httpVersion;    // e.g., "HTTP/1.1"
    int statusCode;            // e.g., 200, 404, 500
    std::string reasonPhrase;  // e.g., "OK", "Not Found"
    std::map<std::string, std::string> headers;
    std::string body;
    bool isSuccess;
    std::string errorMessage;
    
    // Constructor
    HttpResponse() : statusCode(0), isSuccess(false) {}
};

/**
 * Simple HTTP client for making HTTP requests
 * Supports GET requests, response parsing, and error handling
 */
class SimpleHttpClient {
private:
    int maxRedirects;
    
    // Private helper methods
    bool parseStatusLine(const std::string& line, HttpResponse& response);
    void parseHeaderLine(const std::string& line, HttpResponse& response);
    bool isChunkedEncoding(const std::string& headerSection);
    std::string decodeChunkedBody(const std::string& chunkedBody);
    
    // Response processing helpers
    void handleSuccessResponse(const HttpResponse& response);
    void handleRedirectResponse(const HttpResponse& response);
    void handleClientErrorResponse(const HttpResponse& response);
    void handleServerErrorResponse(const HttpResponse& response);
    void displayImportantHeaders(const HttpResponse& response);
    void displayBodyInfo(const HttpResponse& response);

public:
    /**
     * Constructor
     * @param maxRedirects Maximum number of redirects to follow (default: 5)
     */
    explicit SimpleHttpClient(int maxRedirects = 5);
    
    /**
     * Destructor
     */
    ~SimpleHttpClient() = default;
    
    // Copy constructor and assignment operator
    SimpleHttpClient(const SimpleHttpClient&) = default;
    SimpleHttpClient& operator=(const SimpleHttpClient&) = default;
    
    /**
     * Create a TCP connection to the specified hostname and port
     * @param hostname The hostname to connect to
     * @param port The port number to connect to
     * @return Socket file descriptor on success, -1 on failure
     */
    int createConnection(const std::string& hostname, int port);
    
    /**
     * Format an HTTP request string
     * @param hostname The hostname for the Host header
     * @param path The path to request
     * @param method The HTTP method (default: "GET")
     * @return Formatted HTTP request string
     */
    std::string formatHttpRequest(const std::string& hostname, 
                                 const std::string& path, 
                                 const std::string& method = "GET");
    
    /**
     * Send an HTTP request through the socket
     * @param sockfd The socket file descriptor
     * @param request The HTTP request string to send
     * @return true on success, false on failure
     */
    bool sendHttpRequest(int sockfd, const std::string& request);
    
    /**
     * Receive HTTP response from the socket
     * @param sockfd The socket file descriptor
     * @return Raw HTTP response string
     */
    std::string receiveHttpResponse(int sockfd);
    
    /**
     * Parse raw HTTP response into structured data
     * @param rawResponse The raw HTTP response string
     * @return Parsed HttpResponse structure
     */
    HttpResponse parseHttpResponse(const std::string& rawResponse);
    
    /**
     * Make a complete HTTP request and return the response
     * @param hostname The hostname to connect to
     * @param path The path to request
     * @param port The port number (default: 80)
     * @param method The HTTP method (default: "GET")
     * @return HttpResponse structure with the result
     */
    HttpResponse makeHttpRequest(const std::string& hostname, 
                                const std::string& path, 
                                int port = 80, 
                                const std::string& method = "GET");
    
    /**
     * Process and display information about an HTTP response
     * @param response The HttpResponse to process
     */
    void processResponse(const HttpResponse& response);
    
    /**
     * Set the maximum number of redirects to follow
     * @param maxRedirects Maximum redirect count
     */
    void setMaxRedirects(int maxRedirects);
    
    /**
     * Get the current maximum redirect setting
     * @return Current maximum redirect count
     */
    int getMaxRedirects() const;
    
    /**
     * Make a simple GET request (convenience method)
     * @param url Full URL in format "hostname/path" or "hostname:port/path"
     * @return HttpResponse structure with the result
     */
    HttpResponse get(const std::string& url);
    
    /**
     * Check if a status code indicates success (2xx range)
     * @param statusCode HTTP status code
     * @return true if status code is in 2xx range
     */
    static bool isSuccessStatusCode(int statusCode);
    
    /**
     * Check if a status code indicates a redirect (3xx range)
     * @param statusCode HTTP status code
     * @return true if status code is in 3xx range
     */
    static bool isRedirectStatusCode(int statusCode);
    
    /**
     * Check if a status code indicates a client error (4xx range)
     * @param statusCode HTTP status code
     * @return true if status code is in 4xx range
     */
    static bool isClientErrorStatusCode(int statusCode);
    
    /**
     * Check if a status code indicates a server error (5xx range)
     * @param statusCode HTTP status code
     * @return true if status code is in 5xx range
     */
    static bool isServerErrorStatusCode(int statusCode);
};

#endif // SIMPLE_HTTP_CLIENT_H
