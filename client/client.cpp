#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

#include "../server/Message.cpp"

const int port = 12345;
const std::string ip = "127.0.0.1";

class ClientHandler
{
private:
    int clientSocket;
    bool isLogged = false;
    std::string username;

    void sendMsg(int clientSocket, const char *msg)
    {
        int size = 1024;
        send(clientSocket, msg, size, 0);
    }

    void recvMsg(int clientSocket, char *msg)
    {
        int size = 1024;
        read(clientSocket, msg, size);
        cout<<"__________________________________"<<endl<<"Received message: "<<msg<<endl<<"__________________________________"<<endl;
    }

public:
    bool handleRegistration();
    bool handleLogin(int clientSocket);
    void handleAdd(int clientSocket);
    void handleList(int clientSocket);
    void handleGet(int clientSocket);
    void handleLogout();
    void handle();
    ClientHandler();
};

ClientHandler::ClientHandler()
{
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        std::cerr << "Cannot open socket\n";
        exit(1);
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0)
    {
        if (errno == EAFNOSUPPORT)
        {
            std::cerr << "Invalid address family\n";
        }
        else
        {
            std::cerr << "Invalid IP address or other error:\n";
        }
        exit(1);
    }

    if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        std::cerr << "Cannot connect to server\n";
        exit(1);
    }
    std::cerr << "connect to server\n";
    clientSocket = serverSocket;
    handle();
}

void ClientHandler::handle()
{
    std::string command;
    while (true)
    {
        // differenzia se l'utente è loggato o meno per eseguire le operazioni
        // nel caso in cui l'utente è guest, può solo fare login o registrazione
        // altrimenti può fare l'add, la list e la get
        if (!isLogged)
        {
            std::cout << "Enter command: ";
            std::getline(std::cin, command);
            if (command == "login")
            {
                isLogged = handleLogin(clientSocket);
                if (isLogged)
                    std::cout << "Login successful\n";
                else
                    std::cout << "Login failed\n";
            }
            else if (command == "registration")
            {
                isLogged = handleRegistration();
                if (isLogged)
                    std::cout << "Registration successful\n";
                else
                    std::cout << "Registration failed\n";
            }
            else if (command == "add" || command == "list" || command == "get" || command == "logout")
                std::cout << "You need to login first\n";
            else
                std::cout << "You entered an invalid command\n";
        }
        else
        {
            std::cout << "Enter command: ";
            std::getline(std::cin, command);
            if (command == "add")
                handleAdd(clientSocket);
            else if (command == "list")
                handleList(clientSocket);
            else if (command == "get")
                handleGet(clientSocket);
            else if (command == "logout")
                handleLogout();
            else if (command == "login")
                std::cout << "You are already logged in\n";
            else if (command == "registration")
                std::cout << "You are already logged in\n";
            else
                std::cout << "You entered an invalid command\n";
        }
    }
}

bool ClientHandler::handleRegistration()
{
    std::string message = "registration";
    sendMsg(clientSocket, message.c_str());
    char response[1024] = "";
    recvMsg(clientSocket, response);
    if (strcmp(response, "ricevuto_registration") != 0)
        return false;

    std::string username;
    std::string email;
    std::string password;

    std::cout << "Enter username: ";
    std::getline(std::cin, username);
    sendMsg(clientSocket, username.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, "ricevuto_username") != 0)
        return false;

    std::cout << "Enter email: ";
    std::getline(std::cin, email);
    sendMsg(clientSocket, email.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, "ricevuto_email") != 0)
        return false;

    std::cout << "Enter password: ";
    std::getline(std::cin, password);
    sendMsg(clientSocket, password.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, "ricevuto_password") != 0)
        return false;

    sendMsg(clientSocket, "ok");
    recvMsg(clientSocket, (response));

    // TODO: inserire challenge email registration
    return (strcmp(response, "granted_registration") == 0);
}

void ClientHandler::handleList(int clientSocket)
{
    std::string number;
    int n;
    bool validInput = false;
    while (!validInput)
    {
        std::cout << "How many elements: ";
        std::getline(std::cin, number);
        try
        {
            n = std::stoi(number);
            validInput = true;
        }
        catch (const std::exception &e)
        {
            std::cout << "Invalid input. Please enter an integer.\n";
        }
    }
    std::string msg = "list " + std::to_string(n);
    sendMsg(clientSocket, msg.c_str());

    char message[1024] = "";
    recvMsg(clientSocket, message);
    // il server conferma la ricezione del comando
    if (strcmp(message, "ricevuto_list") != 0)
    {
        std::cout << "ERROR IN NOTA\n";
        return;
    }
    else
    {
        std::cout << "comando ricevuto\n";
        sendMsg(clientSocket, "ok");
        recvMsg(clientSocket, (message));
        // trasforma la stringa nei messaggi e stampali
        std::cout << message << std::endl;
        ///
        sendMsg(clientSocket, "fine");
    }
}

void ClientHandler::handleLogout()
{
    sendMsg(clientSocket, "logout");

    // check if server response with a "ricevuto_logout" message otherwise return
    char message[1024] = "";
    recvMsg(clientSocket, (message));
    if (strcmp(message, "ricevuto_logout") != 0)
    {
        std::cout << "ERROR IN LOGOUT\n";
        return;
    }
    else
    {
        std::cout << "Logout successful\n";
    }

    isLogged = false;
}

bool ClientHandler::handleLogin(int clientSocket)
{
    // send login to server
    // check if server response with a "ricevuto_login" message otherwise return false
    // if so send variable username
    // check if server response with a "ricevuto_username" message otherwise return false
    // if so send variable password
    // check if server response with a "ricevuto_password" message otherwise return false
    // if so then return authenticated = true
    // else return authenticated = false
    std::cerr << "handleLogin\n";
    sendMsg(clientSocket, "login");
    char response[1024] = "";
    recvMsg(clientSocket, (response));
    if (strcmp(response, "ricevuto_login") != 0)
        return false;
    std::string username;
    std::string password;
    std::cout << "Enter username: ";
    std::getline(std::cin, username);

    sendMsg(clientSocket, username.c_str());

    recvMsg(clientSocket, response);
    if (strcmp(response, "ricevuto_username") != 0)
        return false;

    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    sendMsg(clientSocket, password.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, "ricevuto_password") != 0)
        return false;

    recvMsg(clientSocket, response);
    return strcmp(response, "granted_login") == 0;
}

void ClientHandler::handleGet(int clientSocket)
{
    // send get to server due to perform the get operation
    sendMsg(clientSocket, "get");
    char message[1024] = "";
    // check if server response with a "ricevuto_get" message otherwise return
    recvMsg(clientSocket, message);
    if (strcmp(message, "ricevuto_get") != 0)
    {
        std::cout << "ERROR IN GET\n";
        return;
    }

    int msgId = 0;
    std::cout << "Insert the identifier of the message to search: ";
    std::cin >> msgId;
    // send the message id to the server
    sendMsg(clientSocket, to_string(msgId).c_str());
    // check if server response with a "ok" message otherwise return
    recvMsg(clientSocket, message);
    if (strcmp(message, "error") == 0)
    {
        std::cout << "Message not found!" << std::endl;
        return;
    }

    sendMsg(clientSocket, "ok");

    // return the message from the server
    recvMsg(clientSocket, message);
    std::cout << "message: " << message << std::endl;
    Message msg = Message(message);
    if (msg.getTitle() == "" && msg.getBody() == "")
    {
        std::cout << "Message empty!" << std::endl;
        return;
    }
    std::cout << msg << std::endl;
    sendMsg(clientSocket, "fine");
}
void ClientHandler::handleAdd(int clientSocket)
{
    sendMsg(clientSocket, "add");
    char message[1024] = "";
    recvMsg(clientSocket, message);
    // il server conferma la ricezione del comando
    if (strcmp(message, "ricevuto_add") == 0)
    {
        // crea la nota da inviare con titolo e body
        std::string note;
        std::string temp;
        std::cout << "Create note: ";
        std::cout << "Insert the title: ";
        std::getline(std::cin, temp);
        note += "title: " + temp + "\n";
        std::cout << "Insert the text: ";
        std::getline(std::cin, temp);
        note += "body: " + temp + "\n";
        //
        // invio la nota
        sendMsg(clientSocket, note.c_str());
        // ricevo la conferma della nota
        recvMsg(clientSocket, message);
        if (strcmp(message, "ricevuto_nota") != 0)
        {
            std::cout << "ERROR IN NOTA\n";
            return;
        }
        else if (strcmp(message, "ricevuto_nota") == 0)
        {
            std::cout << "NOTA SALVATA CON SUCCESSO\n";
            sendMsg(clientSocket, "fine");
            return;
        }
    }
    else
    { // se il comando non è stato accettato dal server
        std::cout << "ERROR IN COMMAND\n";
        return;
    }
}

int main()
{
    ClientHandler clientHandler;
    return 0;
}

// g++ /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client.cpp -o /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client && /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client
