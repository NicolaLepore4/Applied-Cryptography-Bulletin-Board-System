#include <thread>
#include <iostream>
#include <string>
#include "User.cpp"
using namespace std;

class Client
{
private:
    int clientSocket;
    User user = User(" ", " ", " ", " ");

public:
    Client(int clientSocket, string username, string password)
    {
        this->clientSocket = clientSocket;
        this->user = User(username, password, " ", " ");
    }

    int getClientSocket()
    {
        return clientSocket;
    }

    User getUser()
    {
        return user;
    }
};