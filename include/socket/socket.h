#ifndef SOCKET_H
#define SOCKET_H
#include <string>

using std::string;

/**
 Creates a connection between the server and client via a hostname and port
 @params: 
*/
int createConnection(const string& hostname, int port);

#endif // SOCKET_H
