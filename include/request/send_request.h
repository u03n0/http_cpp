#ifndef SEND_REQUEST_H
#define SEND_REQUEST_H
#include <string>

std::string formatHttpRequest(const std::string& hostname, const std::string& path, 
                             const std::string& method = "GET");


bool sendHttpRequest(int sockfd, const std::string& request);

#endif // REQUEST_H
