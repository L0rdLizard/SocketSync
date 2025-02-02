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
    std::cout << "Type a string:" << std::endl;
    while (1){
        std::string input;
        
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
    while (true) {
        std::unique_lock<std::mutex> lock(buffer_mutex);
        cv.wait(lock, [this]() { return data_ready; });

        std::string data_to_send = buffer;
        buffer.clear();
        data_ready = false;
        
        lock.unlock();

        std::cout << "Sending data: " << data_to_send << std::endl;
        sendToServer(data_to_send);

        std::cout << "Type a string:\n";
    }
}


bool Client::sendToServer(const std::string& data) {
    while (true) {
        if (sockfd == -1) {
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                std::cerr << "Failed to create socket\n";
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            sockaddr_in server_addr{};
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(server_port);
            inet_pton(AF_INET, server_ip.c_str(), &server_addr.sin_addr);

            if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
                std::cerr << "Failed to connect to server. Retrying in 2 seconds...\n";
                close(sockfd);
                sockfd = -1;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }

            // std::cout << "Connected to server!\n";
        }

        ssize_t sent_bytes = send(sockfd, data.c_str(), data.length(), 0);
        if (sent_bytes == -1) {
            std::cerr << "Failed to send data. Closing socket and retrying...\n";
            close(sockfd);
            sockfd = -1;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        char buffer[1];  
        ssize_t recv_bytes = recv(sockfd, buffer, sizeof(buffer), MSG_PEEK);
        if (recv_bytes == 0) {
            // std::cerr << "Server closed connection. Reconnecting...\n";
            close(sockfd);
            sockfd = -1;
        }

        std::cout << std::endl;

        return true;
    }
}

Client::~Client() {
    if (sockfd != -1) close(sockfd);
    std::cout << "Client stopped.\n";
}
