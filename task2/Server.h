#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <unordered_map>
#include <netinet/in.h>

class Server {
public:
    explicit Server(int port);
    void start();

private:
    int port_;
    int server_socket_;
    int client_socket_;
    struct sockaddr_in server_addr_;

    void setupServer();
    void acceptClientConnection();
    void processClientData();
    std::string receiveDataFromClient();
    void printSortedCharacterCount(const std::unordered_map<char, int>& char_count);
};

#endif