#ifndef RECEIVE_REQUEST_H
#define RECEIVE_REQUEST_H
#include <iostream>
#include <string>
#include <cstring>      // For memset
#include <sys/socket.h> // For socket functions
#include <netdb.h>      // For getaddrinfo
#include <unistd.h>     // For close
#include <sstream>      // For stringstream
#include <map>          // For headers storage

struct HttpResponse;
std::string receiveHttpResponse(int sockfd);
HttpResponse parseHttpResponse(const std::string& rawResponse);
void printHttpResponse(const HttpResponse& response); 
#endif // !RECEIVE_REQUEST_H


