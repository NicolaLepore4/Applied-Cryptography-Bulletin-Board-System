#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

class ClientHandler {
private:
    int clientSocket;

public:
    ClientHandler(int clientSocket) : clientSocket(clientSocket) {}

    void handle() {
        // TODO: Implement the client handling logic
    }
};

int main() {
    const int port = 12345;
    const std::string ip = "127.0.0.1";
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Cannot open socket\n";
        exit(1);
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
        if (errno == EAFNOSUPPORT) {
            std::cerr << "Invalid address family\n";
        } else {
            std::cerr << "Invalid IP address or other error:\n";
        }
        exit(1);
    }

    if (connect(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Cannot connect to server\n";
        exit(1);
    }
    std::cerr << "connect to server\n";
    // TODO: Implement the client logic

    // g++ /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client.cpp -o /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client && /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client
}
