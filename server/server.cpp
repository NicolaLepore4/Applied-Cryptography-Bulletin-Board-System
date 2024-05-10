#include "BBS.cpp"
#include "Client.cpp"
#include <iostream>
#include <string>
#include <sstream>
#include <list>
#include <cstring>
#include <vector>

#include <thread>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "./crypto/key_exchange.cpp"
using namespace std;

const static string filenameMSG = "database/BBS.txt";
const static string filenameUSR = "database/users.txt";
const static int MAX_CONNECTIONS = 10;

class Server
{
private:
    // Chiave di crittografia (32 byte)
    string key = "UnaChiaveSegretaMoltoLunga123456";
    // Vettore di inizializzazione (16 byte)
    string iv = "UnVettoreInizial";
    const static int port = 12345; //,pubKey, privKey (?)
    const string ip = "127.0.0.1";
    int serverSocket;
    BBS board = BBS(filenameMSG, key, iv);
    list<Client> users = list<Client>();
    string public_key, private_key;

    bool findUserOnFile(const char *, const char *); // check if user exists in file and if password is correct
    pair<string, string> extractNoteDetails(const std::string &note);
    Message findMsgOnFile(char *);

public:
    void handle(int);
    bool handleLogin(int clientSocket);
    void handleLogout(Server s, int clientSocket);
    bool handleRegistration(int clientSocket);
    bool handleRegistrationChallenge(int clientSocket);
    void handleGetMessages(int clientSocket);
    void handleAddMessages(int clientSocket);
    void handleListMessages(int clientSocket, int start, int end);
    bool checkUsernameOnFile(const char *username);

    Server();
    void start();

    void sendMsg(int clientSocket, const char *msg)
    {
        int size = 2048;
        if (send(clientSocket, msg, size, 0) == -1)
        {
            perror("send");
            close(clientSocket);
            throw runtime_error("Client closed connection");
        };
        cout << "Sending message: " << msg << " with size: " << size << endl;
    }

    void recvMsg(int clientSocket, char *msg)
    {
        int size = 2048;
        int n = read(clientSocket, msg, size);
        if (n <= 0)
        {
            // Read error or client closed connection, so close our end and return
            perror("read");
            close(clientSocket);
            throw runtime_error("Client closed connection");
        }
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
    try
    {
        if (!generateKeyPair(public_key, private_key))
            throw runtime_error("Error generating keys");

        sendMsg(clientSocket, public_key.c_str());
        char recv_pub_key_client[2048] = "";
        recvMsg(clientSocket, recv_pub_key_client);

        string client_secret = generateSharedSecret(recv_pub_key_client, private_key);
        string msg_c = "Connection established 234567890";

        auto enc_msg = encryptString(msg_c, reinterpret_cast<const unsigned char *>(client_secret.c_str()), size(client_secret));

        sendMsg(clientSocket, (enc_msg.first).c_str());

        char response[2048] = "";
        recvMsg(clientSocket, response);
        sendMsg(clientSocket, (to_string(enc_msg.second)).c_str());

        char challenge[2048] = "";
        char challenge_len[2048] = "";
        recvMsg(clientSocket, challenge_len);
        sendMsg(clientSocket, "ricevuto_challenge_len");
        recvMsg(clientSocket, challenge);

        bool isLogged = false;
        char command[2048] = "";
        while (true)
        {
            memset(command, 0, sizeof(command));
            recvMsg(clientSocket, command);
            if (strcmp(command, "login") == 0)
            {
                cout << "login in esecuzione" << command << "\n";
                isLogged = handleLogin(clientSocket);
            }
            else if (strcmp(command, "registration") == 0)
            {
                isLogged = handleRegistration(clientSocket);
            }
            else if (strncmp(command, "list", 4) == 0 && isLogged)
            {
                cout << "list in esecuzione" << command << "\n";
                // prendi i due numeri di messaggi da visualizzare dopo la parola list in un buffer di 1024 che rappresentano l'inizio e la fine.
                int start, end;
                sscanf(command + 4, "%d %d", &start, &end);
                handleListMessages(clientSocket, start, end);
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
    catch (runtime_error e)
    {
        close(clientSocket);
        return;
    }
}
bool Server::findUserOnFile(const char *username, const char *password)
{
    ifstream file = ifstream(filenameUSR);
    string line;
    while (getline(file, line))
    {
        User u = User::deserialize(line);
        if (u.getUsername() == username && u.getPassword() == computeSHA3_512Hash(string(password), u.getSalt()))
            return true;
    }
    return false;
}

bool Server::checkUsernameOnFile(const char *username)
{
    ifstream file = ifstream(filenameUSR);
    string line;
    while (getline(file, line))
    {
        User u = User::deserialize(line);
        if (u.getUsername() == username)
            return true;
    }
    return false;
}

bool Server::handleLogin(int clientSocket)
{
    // assume that already read login command
    sendMsg(clientSocket, "ricevuto_login");
    char username[2048] = "";
    recvMsg(clientSocket, username);
    sendMsg(clientSocket, "ricevuto_username");

    char password[2048] = "";
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
        vector<thread> threads;
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
            threads.push_back(thread(&Server::handle, this, clientSocket));
            // OR (if you need to check for thread creation errors)
            if (!threads.back().joinable())
            {
                // Handle thread creation error (e.g., insufficient resources)
                cerr << "Error creating thread\n";
                continue;
            }
        }
        // std::thread t1(&Server::handle, this, clientSocket);
        //  handle(clientSocket);
        // t1.detach();
    }
    catch (runtime_error e)
    {
        cout << "Errore: " << e.what() << endl;
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
    char message[2048] = "";
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
    char message[2048] = "";
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

void Server::handleListMessages(int clientSocket, int start, int end)
{
    try
    {
        char message[2048] = "";
        sendMsg(clientSocket, "ricevuto_list");
        recvMsg(clientSocket, message);
        if (strcmp(message, "ok") != 0)
        {
            throw new exception();
        }
        else
        {
            if (board.size() <= end)
            {
                end = board.retrieveLastId();
            }
            // restituisce gli ultimi numMsg all'interno di BBS
            set<Message> messages = board.List(start, end);
            char n_elements[2048] = "";
            for (auto msg : messages)
            {
                // Serialize the message and send it
                string serializedMsg = msg.serialize_for_list() + "\n";
                sendMsg(clientSocket, serializedMsg.c_str());
                recvMsg(clientSocket, message);
                if (strcmp(message, "ok") == 0)
                {
                    continue;
                }
                else
                {
                    throw new exception();
                }
            }

            // Send "stop" as the last message
            sendMsg(clientSocket, "stop");
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

// Funzione per generare un codice OTP di 6 numeri
string generateOTP()
{
    const int OTP_LENGTH = 6;
    const string digits = "0123456789";
    string otp;

    // Inizializzazione del generatore di numeri casuali
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, digits.size() - 1);

    // Generazione del codice OTP
    for (int i = 0; i < OTP_LENGTH; ++i)
        otp += digits[dis(gen)];
    return otp;
}

bool Server::handleRegistrationChallenge(int clientSocket)
{
    string otp = generateOTP();

    // Invio del codice OTP al client tramite send o tramite inserimento in un file temporaneo condiviso
    sendMsg(clientSocket, otp.c_str());

    // ricezione del codice OTP inserito dal client
    char message[2048] = "";
    recvMsg(clientSocket, message);
    string otp_received = std::string(message);

    // Verifica del codice OTP
    return strcmp(otp.c_str(), otp_received.c_str()) == 0;
}

bool Server::handleRegistration(int clientSocket)
{
    char message[2048] = "";
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
    bool res = handleRegistrationChallenge(clientSocket);
    cout<<"Handle registration challenge: "<<res<<"\n";
    if (!res)
    {
        sendMsg(clientSocket, "denied_registration");
        // recvMsg(clientSocket, message);
        return false;
    }
    cout << "username: " << username << " password: " << password << " email: " << email << "\n";
    User u = User(username, password, email);
    if(checkUsernameOnFile(username.c_str()))
    {
        sendMsg(clientSocket, "username_already_exists");
        return false;
    }
    ofstream file = ofstream(filenameUSR, ios::app);
    file << u.serialize() << "\n";
    file.close();
    if (findUserOnFile(username.c_str(), password.c_str()))
    {
        sendMsg(clientSocket, "granted_registration");
        return true;
    }
    else
    {
        sendMsg(clientSocket, "denied_registration");
        return false;
    }
}

int main()
{
    printf("Server started\n");
    Server server = Server();
    server.start();
}