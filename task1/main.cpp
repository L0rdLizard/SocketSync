#include "Client.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>\n";
        return 1;
    }

    std::string server_ip = argv[1];
    int server_port;

    try {
        server_port = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Invalid port number: " << argv[2] << "\n";
        return 1;
    }

    Client client(server_ip, server_port);
    client.start();

    return 0;
}
