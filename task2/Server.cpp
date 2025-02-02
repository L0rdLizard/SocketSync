#include "Server.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>

Server::Server(int port) : port_(port), server_socket_(-1), client_socket_(-1) {}

void Server::start() {
    while (true) {
        if (server_socket_ == -1) {
            setupServer();
        }

        acceptClientConnection();

        processClientData();

        close(client_socket_);
        client_socket_ = -1;
    }
}

void Server::setupServer() {
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        perror("Ошибка при создании сокета");
        exit(1);
    }

    server_addr_.sin_family = AF_INET;
    server_addr_.sin_addr.s_addr = INADDR_ANY;
    server_addr_.sin_port = htons(port_);

    if (bind(server_socket_, (struct sockaddr*)&server_addr_, sizeof(server_addr_)) < 0) {
        perror("Ошибка при привязке сокета");
        exit(1);
    }

    if (listen(server_socket_, 5) < 0) {
        perror("Ошибка при прослушивании");
        exit(1);
    }

    std::cout << "Сервер запущен, ожидаем подключения...\n";
}

void Server::acceptClientConnection() {
    client_socket_ = accept(server_socket_, nullptr, nullptr);
    if (client_socket_ < 0) {
        perror("Ошибка при принятии соединения");
        return;
    }
    std::cout << "Клиент подключен\n";
}

void Server::processClientData() {
    std::string data = receiveDataFromClient();

    if (!data.empty()) {
        std::unordered_map<char, int> char_count;
        std::istringstream iss(data);
        char ch;
        int count;
        char colon;

        while (iss >> ch >> colon >> count) {
            if (colon == ':') {
                char_count[ch] = count;
            }
        }

        printSortedCharacterCount(char_count);
    }
}


std::string Server::receiveDataFromClient() {
    char buffer[1024] = {0};
    ssize_t bytes_read = read(client_socket_, buffer, sizeof(buffer));
    if (bytes_read <= 0) {
        std::cerr << "Ошибка при получении данных или клиент отключен.\n";
        return "";
    }

    return std::string(buffer, bytes_read);
}


void Server::printSortedCharacterCount(const std::unordered_map<char, int>& char_count) {
    std::vector<std::pair<char, int>> sorted_char_count(char_count.begin(), char_count.end());
    std::sort(sorted_char_count.begin(), sorted_char_count.end(),
                [](const std::pair<char, int>& a, const std::pair<char, int>& b) {
                    return a.second < b.second;
                });

    std::cout << "Результаты подсчёта символов:\n";
    for (const auto& entry : sorted_char_count) {
        std::cout << entry.first << ": " << entry.second << "\n";
    }
}
