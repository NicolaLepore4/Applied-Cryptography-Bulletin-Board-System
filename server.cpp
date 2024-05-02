#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include "BBS.cpp"

class ClientHandler {
private:
    int clientSocket;
    BBS& bbs;

public:
    ClientHandler(int clientSocket, BBS& bbs) : clientSocket(clientSocket), bbs(bbs) {}

    void handle() {
        // TODO: Implement the client handling logic
    }
};

class Server {
private:
    int serverSocket;
    BBS bbs;
    std::mutex mtx;

public:
    Server() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        // TODO: Error checking

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(8080);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
        // TODO: Error checking
    }

    void start() {
        listen(serverSocket, 5);

        while (true) {
            sockaddr_in clientAddress{};
            socklen_t clientLen = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
            // TODO: Error checking

            std::thread([this, clientSocket]() {
                ClientHandler handler(clientSocket, this->bbs);
                handler.handle();
            }).detach();
        }
    }
};

int main() {
    Server server;
    server.start();
}