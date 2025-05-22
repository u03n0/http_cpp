#include "processing/processing.h"
#include <iostream>
#include <vector>

int main() {
    SimpleHttpClient client;
    
    // Example 1: Simple GET request
    std::cout << "=== Example 1: Simple GET Request ===" << std::endl;
    HttpResponse response = client.get("example.com/");
    client.processResponse(response);
    
    std::cout << "\n\nPress Enter to continue...";
    std::cin.get();
    
    // Example 2: Using the detailed method
    std::cout << "\n=== Example 2: Detailed Method Call ===" << std::endl;
    response = client.makeHttpRequest("httpbin.org", "/json", 80, "GET");
    client.processResponse(response);
    
    std::cout << "\n\nPress Enter to continue...";
    std::cin.get();
    
    // Example 3: Testing different status codes
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"httpbin.org", "/status/200"},      // Success
        {"httpbin.org", "/status/404"},      // Not Found
        {"httpbin.org", "/status/500"},      // Server Error
        {"httpbin.org", "/redirect/1"}       // Redirect
    };
    
    for (const auto& testCase : testCases) {
        std::cout << "\n=== Testing: " << testCase.first << testCase.second << " ===" << std::endl;
        
        response = client.makeHttpRequest(testCase.first, testCase.second);
        client.processResponse(response);
        
        // Check status using utility methods
        if (SimpleHttpClient::isSuccessStatusCode(response.statusCode)) {
            std::cout << "✓ Success status detected" << std::endl;
        } else if (SimpleHttpClient::isClientErrorStatusCode(response.statusCode)) {
            std::cout << "⚠ Client error detected" << std::endl;
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
    }
    
    return 0;
}
