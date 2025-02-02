#include "Server.h"
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    int port = 12345;

    if (argc == 2) {
        try {
            port = std::stoi(argv[1]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid port number: " << argv[1] << "\n";
            return 1;
        }
    }

    Server server(port);
    server.start();

    return 0;
}
