#include <Client.h>
#include <thread>
#include <iostream>
#include <cctype>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

Client::Client(const std::string& server_ip, int server_port) :
    server_ip(server_ip), server_port(server_port), sockfd(-1) {}

void Client::start() {
    std::thread input_thread(&Client::inputThread, this);
    std::thread process_thread(&Client::processThread, this);
    input_thread.join();
    process_thread.join();
}

bool Client::isAlphaString(const std::string& str) {
    for (char ch : str) {
        if (!std::isalpha(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return !str.empty();
}

bool Client::isValidInput(const std::string& str) {
    return str.size() <= 64 && isAlphaString(str);
}

void Client::countCharacters(const std::string& str, std::unordered_map<char, int>& charCount) {
    for (char c : str) {
        charCount[c]++;
    }
}

void Client::inputThread(){
    while (1){
        std::string input;
        std::cout << "Type a string:" << std::endl;
        std::getline(std::cin, input);

        if (!isValidInput(input)) {
            std::cout << "The string must contain only letters(max 64)\n";
            continue;
        }

        std::unordered_map<char, int> charCount;
        countCharacters(input, charCount);

        std::ostringstream oss;
        for (const auto& pair : charCount) {
            oss << pair.first << ":" << pair.second << " ";
        }

        {
            std::lock_guard<std::mutex> lock(buffer_mutex);
            buffer = oss.str();
            data_ready = true;
        }
        cv.notify_one();
    }
}

void Client::processThread() {
    while (1){
        std::unique_lock<std::mutex> lock(buffer_mutex);
        cv.wait(lock, [this]() { return data_ready; });

        std::string data_to_send = buffer;
        buffer.clear();
        data_ready = false;
        
        lock.unlock();

        std::cout << "Sending data: " << data_to_send << std::endl;
        sendToServer(data_to_send);
    }
}

void Client::sendToServer(const std::string& data) {
    if (sockfd == -1) {
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1) {
            std::cerr << "Failed to create socket\n";
            return;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);
        inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

        if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
            std::cerr << "Failed to connect to server\n";
            close(sockfd);
            sockfd = -1;
            return;
        }
    }

    send(sockfd, data.c_str(), data.length(), 0);
}