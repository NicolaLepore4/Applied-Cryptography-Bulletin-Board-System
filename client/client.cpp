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

#include "../server/crypto/key_exchange.cpp"

using namespace std;

const int port = 12345;
const string ip = "127.0.0.1";

class ClientHandler
{
private:
    int clientSocket;
    bool isLogged = false;
    string username;
    string public_key, private_key, server_secret;

    void sendMsg(int clientSocket, const char *msg)
    {
        int size = 2048;
        send(clientSocket, msg, size, 0);
    }

    void recvMsg(int clientSocket, char *msg)
    {
        int size = 2048;
        read(clientSocket, msg, size);
        // cout << "__________________________________" << endl
        //      << "Received message: " << msg << endl
        //      << "__________________________________" << endl;
    }

public:
    bool handleRegistration();
    bool handleRegistrationChallenge(int);
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

        if (!generateKeyPair(public_key, private_key))
            throw runtime_error("Cannot generate keys");

        // receive the public key from the server
        char server_public_key[2048] = "";
        recvMsg(serverSocket, server_public_key);

        server_secret = generateSharedSecret(server_public_key, private_key);

        // send the public key to the server
        sendMsg(serverSocket, public_key.c_str());

        // receive challenge from server
        char challenge[2048] = "";
        recvMsg(serverSocket, challenge);

        sendMsg(serverSocket, "challenge_received");
        char challenge_len[2048] = "";
        recvMsg(serverSocket, challenge_len);

        string challenge_decrypt = decryptString(challenge, reinterpret_cast<const unsigned char *>(server_secret.c_str()), atoi(challenge_len));

        // send the challenge to the server

        auto challenge_encrypt = encryptString(challenge_decrypt + " client!1234567", reinterpret_cast<const unsigned char *>(server_secret.c_str()), size(server_secret));
        sendMsg(serverSocket, to_string(challenge_encrypt.second).c_str());
        char response[2048] = "";
        recvMsg(serverSocket, response);
        sendMsg(serverSocket, (challenge_encrypt.first).c_str());

        cerr << "connected to server\n";
        clientSocket = serverSocket;
        handle();
    }
    catch (const exception &e)
    {
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

string printCommands(int logged)
{
    string separator = "############################################\n";
    string commands = separator+"Commands:\n";
    if (!logged)
    {
        commands += "login: login to the system\n";
        commands += "registration: register to the system\n";
    }
    else
    {
        commands += "add: add a new message on the board\n";
        commands += "list: list the messages on the board\n";
        commands += "get: get a message from the board\n";
        commands += "logout: logout from the system\n";
    }
    commands += "exit: close the connection\n";
    return commands+separator;
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
        cout << printCommands(isLogged);
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
    close(clientSocket);
}

bool ClientHandler::handleRegistrationChallenge(int socket)
{

    char otp_challenge[2048] = "";
    recvMsg(socket, otp_challenge);

    if (strcmp(otp_challenge, "otp_sent") != 0)
        return false;
    cout<<"OTP Challenge received on file \"otp.txt\" in client folder\n";

    string otp = "";
    cout<<"Enter OTP: ";
    cin>>otp;

    cout << "Sending back OTP Challenge\n";
    sendMsg(socket, otp.c_str());

    return true;
}

bool ClientHandler::handleRegistration()
{
    string message = "registration";
    sendMsg(clientSocket, message.c_str());
    char response[2048] = "";
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
    while(true){
        cout << "Enter password: ";
        getline(cin, password);
        sendMsg(clientSocket, password.c_str());
        recvMsg(clientSocket, (response));

        if (strcmp(response, "ricevuto_password") != 0)
            continue;
        else break;
    }
    sendMsg(clientSocket, "ok");
    (handleRegistrationChallenge(clientSocket));
    recvMsg(clientSocket, (response));

    return (strcmp(response, "granted_registration") == 0);
}

void ClientHandler::handleList(int clientSocket)
{
    string number;
    int start;
    int end;
    bool validInput = false;
    while (!validInput)
    {
        cout << "Element from: ";
        getline(cin, number);
        try
        {
            start = stoi(number);
            validInput = true;
        }
        catch (const exception &e)
        {
            cout << "Invalid input. Please enter an integer.\n";
            validInput = false;
        }
        cout << "to: ";
        getline(cin, number);
        try
        {
            end = stoi(number);
            validInput = true;
        }
        catch (const exception &e)
        {
            cout << "Invalid input. Please enter an integer.\n";
            validInput = false;
        }
    }
    string msg = "list " + to_string(start) + " " + to_string(end);
    sendMsg(clientSocket, msg.c_str());

    char message[2048] = "";
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
        while (true)
        {
            memset(message, 0, sizeof(message));
            recvMsg(clientSocket, message);
            if (strcmp(message, "stop") == 0)
            {
                break;
            }
            else
            {
                cout << message << endl;
                sendMsg(clientSocket, "ok");
            }
        }
        sendMsg(clientSocket, "fine");
    }
}

void ClientHandler::handleLogout()
{
    sendMsg(clientSocket, "logout");

    // check if server response with a "ricevuto_logout" message otherwise return
    char message[2048] = "";
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
    char response[2048] = "";
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
    char message[2048] = "";
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
    char message[2048] = "";
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
    else // se il comando non è stato accettato dal server
        cout << "ERROR IN COMMAND\n";
}

int main()
{
    ClientHandler clientHandler;
    return 0;
}

// g++ /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client.cpp -o /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client && /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client
