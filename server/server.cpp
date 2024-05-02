#include "BBS.cpp"
#include "User.cpp"
#include <iostream>
#include <string>
#include <list>

#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>


using namespace std;

const static string filenameMSG = "messages.txt";
const static string filenameUSR = "users.txt";
const static int MAX_CONNECTIONS = 10;

class Client {
public:
    int socket;
    thread t; // thread for handling the client

    Client(int socket, thread t) : socket(socket), t(move(t)) {}
};

class Server
{
private:
    const static int port = 12345; //,pubKey, privKey (?)
    const string ip = "127.0.0.1";
    int serverSocket;
    BBS board = BBS(filenameMSG);
    list<Client> users;

public:
    void handleLogin();
    void handleLogout();
    void handleRegistration();
    void handleGetMessages();
    void handleAddMessages();
    void handleListMessages();

    Server()
    {
        sockaddr_in serverAddress{};
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            cerr << "Cannot open socket\n";
            exit(1);
        }

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0) {
            if (errno == EAFNOSUPPORT) {
                cerr << "Invalid address family\n";
            } else {
                cerr << "Invalid IP address or other error:\n";
            }
            exit(1);
}

        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
            cerr << "Cannot bind socket\n";
            exit(1);
        }
    }

    void start() {
        listen(serverSocket, MAX_CONNECTIONS);

        while (true) {
            sockaddr_in clientAddress{};
            socklen_t clientLen = sizeof(clientAddress);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);
            if (clientSocket < 0) {
                cerr << "Cannot accept connection\n";
                continue;
            }

            users.push_back(Client(clientSocket, thread([clientSocket]() {
                // TODO: Implement the client handling logic
                close(clientSocket);
            })));
        }
    }
};

int main() {
    Server server = Server ();
    server.start();
}