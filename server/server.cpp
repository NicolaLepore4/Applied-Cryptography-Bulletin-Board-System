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
    bool findUserOnFile(char*, char*); // check if user exists in file and if password is correct
    pair<string,string> extractNoteDetails(const std::string& note);

public:
    void handle(int);
    bool handleLogin(int clientSocket);
    void handleLogout(Server s, int clientSocket);
    void handleRegistration(int clientSocket);
    void handleGetMessages(int clientSocket);
    void handleAddMessages(int clientSocket);
    void handleListMessages(int clientSocket);

    Server();
    void start();
};

Server::Server()
{
    sockaddr_in serverAddress{};
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0)
    {
        cerr << "Cannot open socket\n";
        exit(1);
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0)
    {
        if (errno == EAFNOSUPPORT)
        {
            cerr << "Invalid address family\n";
        }
        else
        {
            cerr << "Invalid IP address or other error:\n";
        }
        exit(1);
    }

    bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress));
}

void Server::handle(int clientSocket)
{   
    bool isLogged = false;
    char command[128] = "";
    while(true)
    {
        read(clientSocket, &command, sizeof(command));
        cout<<"Received command: "<<command<<"\n";
        if (strcmp (command, "login") == 0){
            cout<<"login in esecuzione"<<command<<"\n";
            isLogged = handleLogin(clientSocket);
        }else if (strcmp (command,"registration") == 0){
            handleRegistration(clientSocket);
        }else if (strcmp (command, "list") == 0 && isLogged){
            handleListMessages(clientSocket);
        }else if (strcmp (command,"add") == 0 && isLogged){
            handleAddMessages(clientSocket);
        }else if (strcmp (command,"get") == 0 && isLogged){
            handleGetMessages(clientSocket);
        }else if (strcmp (command,"logout") == 0 && isLogged){
            handleLogout(*this, clientSocket);
        }else{
            send(clientSocket, "error_wrong_command", sizeof("error_wrong_command"), 0);
        }
    }
}
bool Server::findUserOnFile(char *username, char *password)
{
    ifstream file = ifstream(filenameUSR);
    string line;
    while (getline(file, line))
    {
        cout<<"line: "<<line<<"\n";
        User u = User::deserialize(line);
        cout<<"username: "<<u.getUsername()<<"\n";
        cout<<"password: "<<u.getPassword()<<"\n";
        if (strcmp(u.getUsername().c_str(), username) == 0 && strcmp(u.getPassword().c_str(), password) == 0)
            return true;
    }
    return false;
}

bool Server::handleLogin(int clientSocket)
{
    // assume that already read login command
    send(clientSocket, "ricevuto_login", sizeof("ricevuto_login"), 0);
    char username[128] = "";
    read(clientSocket, &username, sizeof(username));
    send(clientSocket, "ricevuto_username", sizeof("ricevuto_username"), 0);

    char password[128] = "";
    read(clientSocket, &password, sizeof(password));
    send(clientSocket, "ricevuto_password", sizeof("ricevuto_password"), 0);

    bool authenticated = findUserOnFile(username, password);
    if (authenticated){
        cout<<"login granted\n";
        send(clientSocket, "granted_login", sizeof("granted_login"), 0);
        users.push_back(Client(clientSocket, username, password));
    }else{
        cout<<"login danied\n";
        send(clientSocket, "denied_login", sizeof("denied_login"), 0);
    }
    return authenticated;
}

void Server::handleLogout(Server s, int clientSocket)
{
    // s.users.remove_if([clientSocket](Client c)
    //                   { return c.getClientSocket() == clientSocket; });
    close(clientSocket);
}

void Server::start()
{
    listen(serverSocket, MAX_CONNECTIONS);

    while (true)
    {
        sockaddr_in clientAddress{};
        socklen_t clientLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);

        if (clientSocket < 0)
        {
            cerr << "Cannot accept connection\n";
            continue;
        }
        printf("A client connected\n");
        handle(clientSocket);
        handleLogout(*this, clientSocket);
    }
}

void Server::handleRegistration(int clientSocket)
{
}

void Server::handleGetMessages(int clientSocket)
{
}
pair<string,string> Server::extractNoteDetails(const std::string& note) {
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
    char message[128] = "";
    send(clientSocket, "ricevuto_add", sizeof("ricevuto_add"), 0);
    read(clientSocket, &message, sizeof(message));
    try
    {   //estrae i dettagli della nota
        string title, body;
        std::tie(title, body) = extractNoteDetails(message);
        //restituisce l'username da cercare nella lista degli utenti che combacia con il clientSocket
        string usr = "";
        for (auto client : users)
        {
            if (client.getClientSocket() == clientSocket)
            {
                usr = client.getUser().getUsername();
                cout<<"username:AAAAAAAAAAAAAAAAAAAAAAAAAAAA "<<usr<<"\n";
                break;
            }
        }
        //aggiunge la nota alla bacheca
        board.Add(title , usr, body);
        send(clientSocket, "ricevuto_nota", sizeof("ricevuto_nota"), 0);
        read(clientSocket, &message, sizeof(message));
        if (strcmp (message,"fine") == 0)
        {
            return;
        }
        else
        {
            send(clientSocket, "error", sizeof("error"), 0);
            return;
        }
    }
    catch (exception e)
    {
        send(clientSocket, "error", sizeof("error"), 0);
        return;
    }
}

void Server::handleListMessages(int clientSocket)
{
}

int main()
{
    printf("Server started\n");
    Server server = Server();
    server.start();
}