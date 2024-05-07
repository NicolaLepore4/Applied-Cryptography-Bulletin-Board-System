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

using namespace std;

const int port = 12345;
const string ip = "127.0.0.1";

class ClientHandler
{
private:
    int clientSocket;
    bool isLogged = false;
    string username;

    void sendMsg(int clientSocket, const char *msg)
    {
        int size = 1024;
        send(clientSocket, msg, size, 0);
    }

    void recvMsg(int clientSocket, char *msg)
    {
        int size = 1024;
        read(clientSocket, msg, size);
        cout << "__________________________________" << endl
             << "Received message: " << msg << endl
             << "__________________________________" << endl;
    }

public:
    bool handleRegistration();
    bool handleLogin(int clientSocket);
    void handleAdd(int clientSocket);
    void handleList(int clientSocket);
    void handleGet(int clientSocket);
    void handleLogout();
    void handle();
    bool handleExit(int clientSocket);
    bool handleQuit(int clientSocket, bool isLogged);
    ClientHandler();
};

ClientHandler::ClientHandler()
{
    try
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0)
        {
            throw runtime_error("Cannot create socket");
        }

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0)
        {
            if (errno == EAFNOSUPPORT)
                throw runtime_error("Address family not supported");
            else
                throw runtime_error("Invalid address");
        }

        if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
            throw runtime_error("Cannot connect to server");
        cerr << "connected to server\n";
        clientSocket = serverSocket;
        handle();
    }
    catch (exception e)
    {
        cerr << e.what() << endl;
        exit(-1);
    }
}

bool ClientHandler::handleExit(int clientSocket)
{
    cout << "Closing  connection with server...." << endl;
    exit(0);
}

bool ClientHandler::handleQuit(int clientSocket, bool isLogged)
{
    if (isLogged)
        handleLogout();
    return handleExit(clientSocket);
}

void ClientHandler::handle()
{
    bool isExited = false;
    while (!isExited)
    {
        // differenzia se l'utente è loggato o meno per eseguire le operazioni
        // nel caso in cui l'utente è guest, può solo fare login o registrazione
        // altrimenti può fare l'add, la list e la get
        string command = "";
        if (!isLogged)
        {
            cout << "Enter command: ";
            getline(cin, command);
            if (command == "login")
            {
                isLogged = handleLogin(clientSocket);
                if (isLogged)
                    cout << "Login successful\n";
                else
                    cout << "Login failed\n";
            }
            else if (command == "registration")
            {
                isLogged = handleRegistration();
                if (isLogged)
                    cout << "Registration successful\n";
                else
                    cout << "Registration failed\n";
            }
            else if (command == "add" || command == "list" || command == "get" || command == "logout")
                cout << "You need to login first\n";
                else if (command == "exit" || command == "quit")
                    isExited = handleQuit(clientSocket, isLogged);
                else if (command == "closing")
                    isExited = handleExit(clientSocket);

            else
                cout << "You entered an invalid command while not logged!\n";
        }
        else
        {
            cout << "Enter command: ";
            getline(cin, command);
            if (command == "add")
                handleAdd(clientSocket);
            else if (command == "list")
                handleList(clientSocket);
            else if (command == "get")
                handleGet(clientSocket);
            else if (command == "logout")
                handleLogout();
            else if (command == "login")
                cout << "You are already logged in\n";
            else if (command == "registration")
                cout << "You are already logged in\n";
                else if (command == "exit" || command == "quit")
                    isExited = handleQuit(clientSocket, isLogged);
                else if (command == "closing")
                    isExited = handleExit(clientSocket);

            else
                cout << "You entered an invalid command\n";
        }
    }
}

bool ClientHandler::handleRegistration()
{
    string message = "registration";
    sendMsg(clientSocket, message.c_str());
    char response[1024] = "";
    recvMsg(clientSocket, response);
    if (strcmp(response, "ricevuto_registration") != 0)
        return false;

    string username;
    string email;
    string password;

    cout << "Enter username: ";
    getline(cin, username);
    sendMsg(clientSocket, username.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, "ricevuto_username") != 0)
        return false;

    cout << "Enter email: ";
    getline(cin, email);
    sendMsg(clientSocket, email.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, "ricevuto_email") != 0)
        return false;

    cout << "Enter password: ";
    getline(cin, password);
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
    string number;
    int n;
    bool validInput = false;
    while (!validInput)
    {
        cout << "How many elements: ";
        getline(cin, number);
        try
        {
            n = stoi(number);
            validInput = true;
        }
        catch (const exception &e)
        {
            cout << "Invalid input. Please enter an integer.\n";
        }
    }
    string msg = "list " + to_string(n);
    sendMsg(clientSocket, msg.c_str());

    char message[1024] = "";
    recvMsg(clientSocket, message);
    // il server conferma la ricezione del comando
    if (strcmp(message, "ricevuto_list") != 0)
    {
        cout << "ERROR IN NOTA\n";
        return;
    }
    else
    {
        cout << "comando ricevuto\n";
        sendMsg(clientSocket, "ok");
        recvMsg(clientSocket, (message));
        // trasforma la stringa nei messaggi e stampali
        cout << message << endl;
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
        cout << "ERROR IN LOGOUT\n";
        return;
    }
    else
        cout << "Logout successful\n";

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
    cerr << "handleLogin\n";
    sendMsg(clientSocket, "login");
    char response[1024] = "";
    recvMsg(clientSocket, (response));
    if (strcmp(response, "ricevuto_login") != 0)
        return false;
    string username;
    string password;
    cout << "Enter username: ";
    getline(cin, username);

    sendMsg(clientSocket, username.c_str());

    recvMsg(clientSocket, response);
    if (strcmp(response, "ricevuto_username") != 0)
        return false;

    cout << "Enter password: ";
    getline(cin, password);

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
        cout << "ERROR IN GET\n";
        return;
    }

    string msgId = "";
    cout << "Insert the identifier of the message to search: ";
    getline(cin, msgId);
    // send the message id to the server
    sendMsg(clientSocket, msgId.c_str());
    // check if server response with a "ok" message otherwise return
    recvMsg(clientSocket, message);
    if (strcmp(message, "error") == 0)
    {
        cout << "Message not found!" << endl;
        return;
    }

    sendMsg(clientSocket, "ok");

    // return the message from the server
    recvMsg(clientSocket, message);
    Message msg = Message(message);
    sendMsg(clientSocket, "fine");
    if (msg.getTitle() == "" && msg.getBody() == "")
    {
        cout << "Message empty!" << endl;
        return;
    }
    cout << msg;
    return;
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
        string note;
        string temp;
        cout << "Create note: ";
        cout << "Insert the title: ";
        getline(cin, temp);
        note += "title: " + temp + "\n";
        cout << "Insert the text: ";
        getline(cin, temp);
        note += "body: " + temp + "\n";
        //
        // invio la nota
        sendMsg(clientSocket, note.c_str());
        // ricevo la conferma della nota
        recvMsg(clientSocket, message);
        if (strcmp(message, "ricevuto_nota") != 0)
        {
            cout << "ERROR IN NOTA\n";
            return;
        }
        else if (strcmp(message, "ricevuto_nota") == 0)
        {
            cout << "NOTA SALVATA CON SUCCESSO\n";
            sendMsg(clientSocket, "fine");
            return;
        }
    }
    else// se il comando non è stato accettato dal server
        cout << "ERROR IN COMMAND\n";
}

int main()
{
    ClientHandler clientHandler;
    return 0;
}

// g++ /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client.cpp -o /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client && /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client
