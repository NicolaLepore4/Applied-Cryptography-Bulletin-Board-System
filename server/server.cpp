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

#include "BBS.cpp"
#include "./crypto/key_exchange.cpp"
#include "../common/protocol.h"

using namespace std;

const static string filenameMSG = "database/BBS.txt";
const static string filenameUSR = "database/users.txt";
const static string OTP_FILE = "../client/otp.txt";
const static int MAX_CONNECTIONS = 10;

class Server
{
private:
    // Chiave di crittografia (32 byte)
    unsigned char *key = (unsigned char *)"UnaChiaveSegretaMoltoLunga12345";
    // Vettore di inizializzazione (16 byte)
    unsigned char *iv = (unsigned char *)"UnVettoreInizia";
    const static int port = 12345; //,pubKey, privKey (?)
    const string ip = "127.0.0.1";
    int serverSocket;
    BBS board = BBS(filenameMSG, key, iv);
    unordered_map<int, int> clientMsgNumMap;

    list<Client> users = list<Client>();
    string public_key, private_key;

    bool findUserOnFile(const char *, const char *); // check if user exists in file and if password is correct
    pair<string, string> extractNoteDetails(const string &note);
    Message findMsgOnFile(char *);

public:
    void handle(int);
    bool handleLogin(int, string);
    void handleLogout(Server, int, string);
    bool handleRegistration(int, string);
    bool handleRegistrationChallenge(int, string);
    void handleGetMessages(int, string);
    void handleAddMessages(int, string);
    void handleListMessages(int, int, int, string);
    bool checkUsernameOnFile(const char *);

    Server();
    void start();

    void sendMsg(int clientSocket, const char *msg, string client_secret)
    {
        if (client_secret == "")
        {
            int n = send(clientSocket, msg, strlen(msg), 0);
            if (n < 0)
            {
                perror("write");
                close(clientSocket);
                throw runtime_error(ERROR_MSG_SENT);
            }
        }
        else
        {
            int msg_num = 0;
            auto it = clientMsgNumMap.find(clientSocket);
            if (it != clientMsgNumMap.end()) {
                msg_num = it->second;
            } else {
                clientMsgNumMap[clientSocket] = 0;
                msg_num = 0;
            }
            if (msg_num == -1)
            {
                close(clientSocket);
                throw runtime_error(ERROR_MSG_SENT);
            }
            string masg_with_num = msg;
            masg_with_num = masg_with_num + "$$$" + to_string(msg_num);
            string masg_with_hash = masg_with_num + "@@@" + computeSHA3_512Hash(masg_with_num);
            auto enc_msg = encryptString(masg_with_hash, reinterpret_cast<const unsigned char *>(client_secret.c_str()));
            // auto enc_msg = encryptString(msg, reinterpret_cast<const unsigned char*>("UnaChiaveSegretaMoltoLunga12345"));
            auto size = 4096;
            send(clientSocket, (to_string(enc_msg.second)).c_str(), size, 0);
            char response[4096] = "";
            recv(clientSocket, response, size, 0);
            int n = send(clientSocket, (enc_msg.first).c_str(), enc_msg.first.size(), 0);
            if (n < 0)
            {
                perror("write");
                close(clientSocket);
                throw runtime_error(ERROR_MSG_SENT);
            }
            it->second++;
        }
    }

    void recvMsg(int clientSocket, char *msg, string client_secret)
    {
        int size = 4096;
        int n = recv(clientSocket, msg, size, 0);
        if (n < 0)
        {
            perror("read");
            close(clientSocket);
            throw runtime_error(ERROR_MSG_RCV);
        }
        else if (n == 0)
        {
            close(clientSocket);
            throw runtime_error(ERROR_CLT_DISCONNECTED);
        }

        if (client_secret != "")
        {
            size_t len = atoi(msg);
            send(clientSocket, "ricevuto", size, 0);

            recv(clientSocket, msg, size, 0);

            auto dec_msg = decryptString(msg, reinterpret_cast<const unsigned char *>(client_secret.c_str()), len);

            string dec_msg_hash = dec_msg.substr(dec_msg.find("@@@") + 3);

            int msg_num;
            auto it = clientMsgNumMap.find(clientSocket);
            if (it != clientMsgNumMap.end()) {
                msg_num = it->second;
            } else {
                //clientMsgNumMap[clientSocket] = 0;
                msg_num = 0;
            }                                                   // Convert msg to std::string
            int num_msg = stoi(dec_msg.substr(dec_msg.find("$$$")+3,dec_msg.find("@@@"))); // Use std::string functions
            if (msg_num != num_msg)
            {
                cout << "Wrong msg number from : " << clientSocket << endl;
                close(clientSocket);
            }
            dec_msg = dec_msg.substr(0, dec_msg.find("@@@"));
            if (dec_msg_hash != computeSHA3_512Hash(dec_msg))
            {
                cout << "Wrong hash from : " << clientSocket << endl;
                close(clientSocket);
            }
            dec_msg = dec_msg.substr(0, dec_msg.find("$$$"));
            strcpy(msg, dec_msg.c_str());
            it->second++;
            cout << msg << endl;
        }
    }
};

Server::Server()
{
    try
    {
        sockaddr_in serverAddress{};
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0)
            throw runtime_error(ERROR_SOCKET);

        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0)
            if (errno == EAFNOSUPPORT)
                throw runtime_error(ERROR_ADDRESS);
            else
                throw runtime_error(ERROR_ADDRESS_INVALID);

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
    clientMsgNumMap[clientSocket] = 0;
    try
    {
        if (!generateKeyPair(public_key, private_key))
            throw runtime_error(ERROR_KEYPAIR);

        sendMsg(clientSocket, public_key.c_str(), "");
        char recv_pub_key_client[4096] = "";
        recvMsg(clientSocket, recv_pub_key_client, "");

        string client_secret = generateSharedSecret(recv_pub_key_client, private_key);
        if (client_secret == "")
            throw runtime_error("Error generating shared secret");
        string msg_c = "Connection established 234567890";

        //auto enc_msg = encryptString(msg_c, reinterpret_cast<const unsigned char *>(client_secret.c_str()));
        sendMsg(clientSocket, msg_c.c_str(), client_secret);

        char response[4096] = "";
        recvMsg(clientSocket, response, client_secret);
        sendMsg(clientSocket, (to_string(msg_c.size())).c_str(), client_secret);

        char challenge[4096] = "";
        char challenge_len[4096] = "";
        recvMsg(clientSocket, challenge_len, client_secret);
        sendMsg(clientSocket, "ricevuto_challenge_len", client_secret);
        recvMsg(clientSocket, challenge, client_secret);

        bool isLogged = false;
        char command[4096] = "";
        while (true)
        {
            memset(command, 0, sizeof(command));
            recvMsg(clientSocket, command, client_secret);
            if (strcmp(command, CMD_LOGIN.c_str()) == 0)
                isLogged = handleLogin(clientSocket, client_secret);
            else if (strcmp(command, CMD_REGISTRATION.c_str()) == 0)
                isLogged = handleRegistration(clientSocket, client_secret);
            else if (strncmp(command, CMD_LIST.c_str(), 4) == 0 && isLogged)
            {
                // prendi i due numeri di messaggi da visualizzare dopo la parola list in un buffer di 1024 che rappresentano l'inizio e la fine.
                int start, end;
                sscanf(command + 4, "%d %d", &start, &end);
                handleListMessages(clientSocket, start, end, client_secret);
            }
            else if (strcmp(command, CMD_ADD.c_str()) == 0 && isLogged)
                handleAddMessages(clientSocket, client_secret);
            else if (strcmp(command, CMD_GET.c_str()) == 0 && isLogged)
                handleGetMessages(clientSocket, client_secret);
            else if (strcmp(command, CMD_LOGOUT.c_str()) == 0 && isLogged)
                handleLogout(*this, clientSocket, client_secret);
            else
                sendMsg(clientSocket, INVALID_COMMAND.c_str(), client_secret);
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

bool Server::handleLogin(int clientSocket, string client_secret)
{
    // assume that already read login command
    sendMsg(clientSocket, CMD_LOGIN_RCV.c_str(), client_secret);
    char username[4096] = "";
    recvMsg(clientSocket, username, client_secret);
    sendMsg(clientSocket, CMD_USERNAME_RCV.c_str(), client_secret);

    char password[4096] = "";
    recvMsg(clientSocket, password, client_secret);
    sendMsg(clientSocket, CMD_PASSWORD_RCV.c_str(), client_secret);

    bool authenticated = findUserOnFile(username, password);
    char msg[4096] = "";
    recvMsg(clientSocket, msg, client_secret);
    if (strcmp(msg, OK.c_str()) != 0)
        throw runtime_error(ERROR_MSG_RCV);
    if (authenticated)
    {
        sendMsg(clientSocket, CMD_LOGIN_OK.c_str(), client_secret);
        users.push_back(Client(clientSocket, username, password));
    }
    else
        sendMsg(clientSocket, CMD_LOGIN_KO.c_str(), client_secret);
    return authenticated;
}

void Server::handleLogout(Server s, int clientSocket, string client_secret)
{
    // Remove the client from the list of users
    auto isMatchingClient = [clientSocket](Client &c)
    {
        return c.getClientSocket() == clientSocket;
    };
    s.users.remove_if(isMatchingClient);
    // Send a confirmation message to the client
    sendMsg(clientSocket, CMD_LOGOUT_OK.c_str(), client_secret);
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
            cout << "Waiting for a connection...\n";
            int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientLen);
            // 2.3. Error handling for failed connection acceptance
            if (clientSocket < 0)
            {
                cerr << "Cannot accept connection\n";
                continue;
            }
            else
                cout << "Connection accepted from " << inet_ntoa(clientAddress.sin_addr) << ":" << ntohs(clientAddress.sin_port) << endl;

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
        // thread t1(&Server::handle, this, clientSocket);
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

void Server::handleGetMessages(int clientSocket, string client_secret)
{
    sendMsg(clientSocket, CMD_GET_RCV.c_str(), client_secret);
    char message[4096] = "";
    recvMsg(clientSocket, message, client_secret);
    // expecting message identifier to be "get"
    Message msg = findMsgOnFile(message);
    if (strcmp(msg.getTitle().c_str(), "") == 0)
    {
        sendMsg(clientSocket, ERROR.c_str(), client_secret);
        return;
    }
    else
        sendMsg(clientSocket, OK.c_str(), client_secret);

    recvMsg(clientSocket, message, client_secret);

    string msgSerialized = msg.serialize();
    sendMsg(clientSocket, msgSerialized.c_str(), client_secret);
    recvMsg(clientSocket, message, client_secret);
}

pair<string, string> Server::extractNoteDetails(const string &note)
{
    size_t titleStart = note.find(NOTA_TITLE);
    size_t titleEnd = note.find("\n");
    string title = note.substr(titleStart + NOTA_TITLE.length(), titleEnd - titleStart - NOTA_TITLE.length());

    size_t bodyStart = note.find(NOTA_BODY, titleEnd);
    size_t bodyEnd = note.find("\n", bodyStart);
    string body = note.substr(bodyStart + NOTA_BODY.length(), bodyEnd - bodyStart - NOTA_BODY.length());
    return {title, body};
}
void Server::handleAddMessages(int clientSocket, string client_secret)
{
    char message[4096] = "";
    sendMsg(clientSocket, CMD_ADD_RCV.c_str(), client_secret);
    recvMsg(clientSocket, message, client_secret);
    try
    { // estrae i dettagli della nota
        string title, body;
        tie(title, body) = extractNoteDetails(message);
        // restituisce l'username da cercare nella lista degli utenti che combacia con il clientSocket
        string usr = "";
        for (auto client : users)
            if (client.getClientSocket() == clientSocket)
            {
                usr = client.getUser().getUsername();
                break;
            }
        // aggiunge la nota alla bacheca
        board.Add(title, usr, body);
        sendMsg(clientSocket, CMD_NOTA_RCV.c_str(), client_secret);
        recvMsg(clientSocket, message, client_secret);
        if (strcmp(message, STOP.c_str()) != 0)
            throw new exception();
    }
    catch (exception e)
    {
        sendMsg(clientSocket, ERROR.c_str(), client_secret);
        return;
    }
}

void Server::handleListMessages(int clientSocket, int start, int end, string client_secret)
{
    try
    {
        char message[4096] = "";
        sendMsg(clientSocket, CMD_LIST_RCV.c_str(), client_secret);
        recvMsg(clientSocket, message, client_secret);
        if (strcmp(message, OK.c_str()) != 0)
            throw new exception();
        else
        {
            if (board.size() <= end)
                end = board.retrieveLastId();
            // restituisce gli ultimi numMsg all'interno di BBS
            set<Message> messages = board.List(start, end);
            char n_elements[4096] = "";
            for (auto msg : messages)
            {
                // Serialize the message and send it
                string serializedMsg = msg.serialize_for_list() + "\n";
                sendMsg(clientSocket, serializedMsg.c_str(), client_secret);
                recvMsg(clientSocket, message, client_secret);
                if (strcmp(message, OK.c_str()) == 0)
                    continue;
                else
                    throw new exception();
            }

            // Send "stop" as the last message
            sendMsg(clientSocket, STOP.c_str(), client_secret);
            recvMsg(clientSocket, message, client_secret);
            if (strcmp(message, STOP.c_str()) == 0)
                cout << LIST_SENT_SUCCESS << endl;
            else
                throw runtime_error("LISTAAA");
        }
    }
    catch (exception e)
    {
        sendMsg(clientSocket, ERROR.c_str(), client_secret);
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

bool Server::handleRegistrationChallenge(int clientSocket, string client_secret)
{
    string otp = generateOTP();

    // Invio del codice OTP al client tramite inserimento in un file temporaneo condiviso
    ofstream otpFile = ofstream(OTP_FILE);
    otpFile << otp << "\n";
    otpFile.close();

    sendMsg(clientSocket, OTP_SENT.c_str(), client_secret);

    // ricezione del codice OTP inserito dal client
    char message[4096] = "";
    recvMsg(clientSocket, message, client_secret);
    string otp_received = string(message);

    // Verifica del codice OTP
    return strcmp(otp.c_str(), otp_received.c_str()) == 0;
}

bool Server::handleRegistration(int clientSocket, string client_secret)
{
    char message[4096] = "";
    sendMsg(clientSocket, CMD_REGISTRATION_RCV.c_str(), client_secret);
    string username = "", password = "", email = "";
    recvMsg(clientSocket, message, client_secret);
    // trasforma message in stringa
    username = string(message);
    sendMsg(clientSocket, CMD_USERNAME_RCV.c_str(), client_secret);

    recvMsg(clientSocket, message, client_secret);
    email = string(message);
    sendMsg(clientSocket, CMD_EMAIL_RCV.c_str(), client_secret);
    while (true)
    {
        recvMsg(clientSocket, message, client_secret);
        password = string(message);
        if (password.size() < 10)
        {
            sendMsg(clientSocket, CMD_PASSWORD_SHORT.c_str(), client_secret);
            continue;
        }
        else
        {
            sendMsg(clientSocket, CMD_PASSWORD_RCV.c_str(), client_secret);
            break;
        }
    }
    recvMsg(clientSocket, message, client_secret);
    bool res = handleRegistrationChallenge(clientSocket, client_secret);
    if (!res)
    {
        sendMsg(clientSocket, REGISTRATION_KO.c_str(), client_secret);
        // recvMsg(clientSocket, message);
        return false;
    }

    User u = User(username, password, email);
    if (checkUsernameOnFile(username.c_str()))
    {
        sendMsg(clientSocket,USERNAME_TAKEN.c_str(), client_secret);
        return false;
    }
    ofstream file = ofstream(filenameUSR, ios::app);
    file << u.serialize() << "\n";
    file.close();
    if (findUserOnFile(username.c_str(), password.c_str()))
    {
        sendMsg(clientSocket, REGISTRATION_OK.c_str(), client_secret);
        users.push_back(Client(clientSocket, username, password));
        return true;
    }
    else
    {
        sendMsg(clientSocket, REGISTRATION_KO.c_str(), client_secret);
        return false;
    }
}

int main()
{
    printf("Server started\n");
    Server server = Server();
    server.start();
}