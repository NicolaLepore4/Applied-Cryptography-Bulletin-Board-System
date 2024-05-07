#include "BBS.cpp"
#include "Client.cpp"
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <cstring>

#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

const static string filenameMSG = "database/BBS.txt";
const static string filenameUSR = "database/users.txt";
const static int MAX_CONNECTIONS = 10;

class Server
{
private:
    const static int port = 12345; //,pubKey, privKey (?)
    const string ip = "127.0.0.1";
    int serverSocket;
    BBS board = BBS(filenameMSG);
    list<Client> users = list<Client>();
    bool findUserOnFile(const char *, const char *); // check if user exists in file and if password is correct
    pair<string, string> extractNoteDetails(const std::string &note);
    Message findMsgOnFile(char *);

public:
    void handle(int);
    bool handleLogin(int clientSocket);
    void handleLogout(Server s, int clientSocket);
    void handleRegistration(int clientSocket);
    void handleGetMessages(int clientSocket);
    void handleAddMessages(int clientSocket);
    void handleListMessages(int clientSocket, int numMsg);

    Server();
    void start();

    void sendMsg(int clientSocket, const char *msg)
    {
        int size = 1024;
        send(clientSocket, msg, size, 0);
        cout << "Sending message: " << msg << " with size: " << size << endl;
    }

    void recvMsg(int clientSocket, char *msg)
    {
        int size = 1024;
        read(clientSocket, msg, size);
        cout << "Received message: " << msg << endl;
    }
};

Server::Server()
{
    try
    {
        sockaddr_in serverAddress{};
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0)
        {
            throw runtime_error("Cannot open socket");
        }

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0)
        {
            if (errno == EAFNOSUPPORT)
            {
                throw runtime_error("Address family not supported");
            }
            else
            {
                throw runtime_error("Invalid IP address or other error:\n");
            }
        }

        bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
    }
    catch (exception e)
    {
        cout << "Errore: " << e.what() << endl;
        exit(-1);
    }
}

void Server::handle(int clientSocket)
{
    bool isLogged = false;
    char command[1024] = "";
    while (true)
    {
        recvMsg(clientSocket, command);
        cout << "Received command: " << command << "\n";
        if (strcmp(command, "login") == 0)
        {
            cout << "login in esecuzione" << command << "\n";
            isLogged = handleLogin(clientSocket);
        }
        else if (strcmp(command, "registration") == 0)
        {
            handleRegistration(clientSocket);
        }
        else if (strncmp(command, "list", 4) == 0 && isLogged)
        {
            // prendi il numero di messaggi da visualizzare dopo la parola list in un buffer di 1024
            int numMessages = atoi(command + 4);
            handleListMessages(clientSocket, numMessages);
        }
        else if (strcmp(command, "add") == 0 && isLogged)
        {
            handleAddMessages(clientSocket);
        }
        else if (strcmp(command, "get") == 0 && isLogged)
        {
            handleGetMessages(clientSocket);
        }
        else if (strcmp(command, "logout") == 0 && isLogged)
        {
            handleLogout(*this, clientSocket);
        }
        else
        {
            sendMsg(clientSocket, "error_wrong_command");
        }
    }
}
bool Server::findUserOnFile(const char *username, const char *password)
{
    ifstream file = ifstream(filenameUSR);
    string line;
    while (getline(file, line))
    {
        User u = User::deserialize(line);
        if (strcmp(u.getUsername().c_str(), username) == 0 && strcmp(u.getPassword().c_str(), password) == 0)
            return true;
    }
    return false;
}

bool Server::handleLogin(int clientSocket)
{
    // assume that already read login command
    sendMsg(clientSocket, "ricevuto_login");
    char username[1024] = "";
    recvMsg(clientSocket, username);
    sendMsg(clientSocket, "ricevuto_username");

    char password[1024] = "";
    recvMsg(clientSocket, password);
    sendMsg(clientSocket, "ricevuto_password");

    bool authenticated = findUserOnFile(username, password);
    if (authenticated)
    {
        cout << "login granted\n";
        sendMsg(clientSocket, "granted_login");
        users.push_back(Client(clientSocket, username, password));
    }
    else
    {
        cout << "login denied\n";
        sendMsg(clientSocket, "denied_login");
    }
    return authenticated;
}

void Server::handleLogout(Server s, int clientSocket)
{
    cout << "handleLogout\n";
    // Remove the client from the list of users
    auto isMatchingClient = [clientSocket](Client &c)
    {
        cout << "clientSocket: " << clientSocket << "\n";
        return c.getClientSocket() == clientSocket;
    };
    s.users.remove_if(isMatchingClient);
    // Send a confirmation message to the client
    sendMsg(clientSocket, "ricevuto_logout");
}

void Server::start()
{
    try
    {
        // 1. Listen for incoming connections
        listen(serverSocket, MAX_CONNECTIONS);
        // 2. Infinite loop to handle connections
        while (true)
        {
            // 2.1. Initialize client address structure
            sockaddr_in clientAddress{};
            socklen_t clientLen = sizeof(clientAddress);
            // 2.2. Accept a new client connection
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);
            // 2.3. Error handling for failed connection acceptance
            if (clientSocket < 0)
            {
                cerr << "Cannot accept connection\n";
                continue;
            }
            // 2.4. Successful connection established
            printf("A client connected\n");
            // 2.5. Handle the client connection (likely in a separate thread or function)
            std::thread t1(&Server::handle, this, clientSocket);
            // handle(clientSocket);
            t1.detach();
        }
    }
    catch (exception e)
    {
        cout << "Errore: " << e.what() << endl;
        exit(-1);
    }
}
Message Server::findMsgOnFile(char *identifier)
{
    int id = atoi(identifier);
    return board.Get(id);
}

void Server::handleGetMessages(int clientSocket)
{
    sendMsg(clientSocket, "ricevuto_get");
    char message[1024] = "";
    recvMsg(clientSocket, message);
    // expecting message identifier to be "get"
    cout << "message: " << message << "\n";
    Message msg = findMsgOnFile(message);
    cout << "msg: " << msg.serialize() << "\n";
    if (strcmp(msg.getTitle().c_str(), "") == 0)
    {
        sendMsg(clientSocket, "error");
        return;
    }
    else
        sendMsg(clientSocket, "ok");

    recvMsg(clientSocket, message);

    std::string msgSerialized = msg.serialize();
    sendMsg(clientSocket, msgSerialized.c_str());
    cout << "messaggio inviato" << endl;
    recvMsg(clientSocket, message);
}

pair<string, string> Server::extractNoteDetails(const std::string &note)
{
    size_t titleStart = note.find("title: ");
    size_t titleEnd = note.find("\n");
    std::string title = note.substr(titleStart + 7, titleEnd - titleStart - 7);

    size_t bodyStart = note.find("body: ", titleEnd);
    size_t bodyEnd = note.find("\n", bodyStart);
    std::string body = note.substr(bodyStart + 6, bodyEnd - bodyStart - 6);
    return {title, body};
}
void Server::handleAddMessages(int clientSocket)
{
    char message[1024] = "";
    sendMsg(clientSocket, "ricevuto_add");
    recvMsg(clientSocket, message);
    try
    { // estrae i dettagli della nota
        string title, body;
        std::tie(title, body) = extractNoteDetails(message);
        // restituisce l'username da cercare nella lista degli utenti che combacia con il clientSocket
        string usr = "";
        for (auto client : users)
        {
            if (client.getClientSocket() == clientSocket)
            {
                usr = client.getUser().getUsername();
                break;
            }
        }
        // aggiunge la nota alla bacheca
        board.Add(title, usr, body);
        sendMsg(clientSocket, "ricevuto_nota");
        recvMsg(clientSocket, message);
        if (strcmp(message, "fine") == 0)
            return;
        else
            throw new exception();
    }
    catch (exception e)
    {
        sendMsg(clientSocket, "error");
        return;
    }
}

void Server::handleListMessages(int clientSocket, int numMsg)
{
    try
    {
        char message[1024] = "";
        sendMsg(clientSocket, "ricevuto_list");
        recvMsg(clientSocket, message);
        if (strcmp(message, "ok") != 0)
        {
            throw new exception();
        }
        else
        {
            if (board.size() <= numMsg)
            {
                numMsg = board.size();
            }
            // restituisce gli ultimi numMsg all'interno di BBS
            set<Message> messages = board.List(numMsg);
            char n_elements[1024] = "";
            for (auto msg : messages)
            {
                // aggiungi al buffer n_elements il messaggio serializzato
                strcat(n_elements, msg.serialize().c_str());
                strcat(n_elements, "\n");
            }
            sendMsg(clientSocket, n_elements);
            recvMsg(clientSocket, message);
            if (strcmp(message, "fine") == 0)
            {
                cout << "lista inviata con successo" << endl;
                return;
            }
            else
                throw new exception();
        }
    }
    catch (exception e)
    {
        sendMsg(clientSocket, "error");
        return;
    }
}
void Server::handleRegistration(int clientSocket)
{
    char message[1024] = "";
    sendMsg(clientSocket, "ricevuto_registration");
    string username = "", password = "", email = "";
    recvMsg(clientSocket, message);
    // trasforma message in stringa
    username = std::string(message);
    cout << "username: " << username << "\n";
    sendMsg(clientSocket, "ricevuto_username");

    recvMsg(clientSocket, message);
    email = std::string(message);
    sendMsg(clientSocket, "ricevuto_email");

    recvMsg(clientSocket, message);
    password = std::string(message);
    sendMsg(clientSocket, "ricevuto_password");

    recvMsg(clientSocket, message);
    User u = User(username, password, email);
    ofstream file = ofstream(filenameUSR, ios::app);
    file << u.serialize() << "\n";
    file.close();
    if (findUserOnFile(username.c_str(), password.c_str()))
        sendMsg(clientSocket, "granted_registration");
    else
        sendMsg(clientSocket, "denied_registration");
}

int main()
{
    printf("Server started\n");
    Server server = Server();
    server.start();
}