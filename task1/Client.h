#include <string>
#include <mutex>
#include <condition_variable>
#include <unordered_map>

class Client {
public:
    Client(const std::string& server_ip, int server_port);
    void start();
    ~Client();

private:
    int sockfd;
    std::string server_ip;
    int server_port;

    std::string buffer;
    std::mutex buffer_mutex;
    std::condition_variable cv;
    bool data_ready = false;

    void inputThread();
    void processThread();
    bool isAlphaString(const std::string& str);
    bool isValidInput(const std::string& str);
    void countCharacters(const std::string& str, std::unordered_map<char, int>& char_count);
    bool sendToServer(const std::string& data);
};