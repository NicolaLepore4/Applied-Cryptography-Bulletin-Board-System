/**
 * @file client.cpp
 * @brief This file contains the implementation of the ClientHandler class, which handles the client-side operations for a bulletin board system.
 *
 * The ClientHandler class establishes a connection with the server, handles user registration, login, adding messages to the board, listing messages, getting a message, and logging out.
 * It uses socket programming to communicate with the server and implements cryptographic functions for secure communication.
 *
 * The main function in this file creates an instance of the ClientHandler class and starts the client-side operations.
 */
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
#include <algorithm>

#include "../common/Message.cpp"
#include "../common/crypto/key_exchange.cpp"
#include "../common/protocol.h"

using namespace std;

const int port = 12345;
const string ip = "127.0.0.1";

class ClientHandler
{
    /**
     * @class ClientHandler
     * @brief Handles the client-side functionality for the bulletin board system.
     */
private:
    int clientSocket;                                   /**< The client socket for communication with the server. */
    bool isLogged = false;                              /**< Flag indicating if the client is logged in. */
    string username;                                    /**< The username of the client. */
    string public_key, private_key, server_secret = ""; /**< Cryptographic keys and server secret. */
    int msg_num = 0;                                    /**< The message ID for the client. */

    /**
     * @brief Sends a message to the server.
     * @param clientSocket The client socket for communication.
     * @param msg The message to be sent.
     * @throws runtime_error if the message fails to send.
     */
    void sendMsg(int clientSocket, const char *msg);
    /**
     * @brief Receives a message from the server.
     * @param clientSocket The client socket for communication.
     * @param msg The buffer to store the received message.
     * @param decrypt Flag indicating if the received message needs to be decrypted.
     */
    void recvMsg(int clientSocket, char *msg, bool decrypt = true);
    /**
     * @brief Handles the registration process for a new user.
     * @return true if the registration is successful, false otherwise.
     */
    bool handleRegistration();

    /**
     * @brief Handles the registration challenge for a new user.
     * @param clientSocket The client socket for communication.
     * @return true if the challenge is successful, false otherwise.
     */
    bool handleRegistrationChallenge(int clientSocket);

    /**
     * @brief Handles the login process for an existing user.
     * @param clientSocket The client socket for communication.
     * @return true if the login is successful, false otherwise.
     */
    bool handleLogin(int clientSocket);

    /**
     * @brief Handles the addition of a new message to the bulletin board.
     * @param clientSocket The client socket for communication.
     */
    void handleAdd(int clientSocket);

    /**
     * @brief Handles the listing of all messages on the bulletin board.
     * @param clientSocket The client socket for communication.
     */
    void handleList(int clientSocket);

    void getLocal();
    void saveMessage(Message &msg);

    /**
     * @brief Handles the retrieval of a specific message from the bulletin board.
     * @param clientSocket The client socket for communication.
     */
    void handleGet(int clientSocket);

    /**
     * @brief Handles the logout process for the client.
     */
    void handleLogout();

    /**
     * @brief Handles the main functionality of the client.
     */
    void handle();

    /**
     * @brief Handles the exit process for the client.
     * @param clientSocket The client socket for communication.
     * @return true if the exit is successful, false otherwise.
     */
    bool handleExit(int clientSocket);

    /**
     * @brief Handles the quit process for the client.
     * @param clientSocket The client socket for communication.
     * @param isLogged Flag indicating if the client is logged in.
     * @return true if the quit is successful, false otherwise.
     */
    bool handleQuit(int clientSocket, bool isLogged);

public:
    /**
     * @brief Default constructor for the ClientHandler class.
     */
    ClientHandler();
};

void ClientHandler::sendMsg(int clientSocket, const char *msg)
{
    int size = 4096;
    if (server_secret == "")
    {
        int n = send(clientSocket, msg, size, 0);
        if (n < 0)
            throw runtime_error(ERROR_MSG_SENT);
        return;
    }
    string masg_with_num = msg;
    masg_with_num = masg_with_num + "$$$" + to_string(msg_num);
    string masg_with_hash = masg_with_num + "@@@" + computeSHA3_512Hash(masg_with_num);
    auto msg_encrypt = encryptString(masg_with_hash, reinterpret_cast<const unsigned char *>(server_secret.c_str()));

    int msg_encrypt_first_len = msg_encrypt.second;
    send(clientSocket, to_string(msg_encrypt_first_len).c_str(), size, 0);
    char response[4096] = "";
    recv(clientSocket, response, size, 0);

    int n = send(clientSocket, msg_encrypt.first.c_str(), msg_encrypt.first.size(), 0);
    msg_num++;
    if (n < 0)
        throw runtime_error(ERROR_MSG_SENT);
}

void ClientHandler::recvMsg(int clientSocket, char *msg, bool decrypt)
{
    int size = 4096;
    recv(clientSocket, msg, size, 0);
    if (decrypt)
    {
        size_t server_secret_len = atoi(msg);
        send(clientSocket, "ricevuto", size, 0);
        recv(clientSocket, msg, size, 0);
        string msg_decrypt = decryptString(msg, reinterpret_cast<const unsigned char *>(server_secret.c_str()), server_secret_len);

        string dec_msg_hash = msg_decrypt.substr(msg_decrypt.find("@@@") + 3);

        int num_msg = stoi(msg_decrypt.substr(msg_decrypt.find("$$$") + 3, msg_decrypt.find("@@@"))); // Use std::string functions
        if (msg_num != num_msg)
        {
            cout << "Wrong msg number from : " << clientSocket << endl;
            handleExit(clientSocket);
            exit(-1);
        }
        msg_decrypt = msg_decrypt.substr(0, msg_decrypt.find("@@@"));
        if (dec_msg_hash != computeSHA3_512Hash(msg_decrypt))
        {
            cout << "Wrong hash in msg_num : " << msg_num << endl;
            close(clientSocket);
            exit(-1);
        }
        msg_decrypt = msg_decrypt.substr(0, msg_decrypt.find("$$$"));
        // cout << "Received: " << msg_decrypt << endl;
        strcpy(msg, msg_decrypt.c_str());
        msg_num++; // Convert msg to std::string
    }
}
ClientHandler::ClientHandler()
{
    try
    {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0)
            throw runtime_error(ERROR_SOCKET);

        sockaddr_in serverAddress{};
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        if (inet_pton(AF_INET, ip.c_str(), &serverAddress.sin_addr) <= 0)
            if (errno == EAFNOSUPPORT)
                throw runtime_error(ERROR_ADDRESS);
            else
                throw runtime_error(ERROR_ADDRESS_INVALID);

        if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
            throw runtime_error(SERVER_UNAVAILABLE);

        if (!generateKeyPair(public_key, private_key))
            throw runtime_error(ERROR_KEYPAIR);

        // receive the public key from the server
        char server_public_key[4096] = "";
        recvMsg(serverSocket, server_public_key, false);

        string server_pubKey_check = "";
        loadFromFile(server_pubKey_check);
        if (!strcmp(server_public_key, server_pubKey_check.c_str()) == 0)
            throw runtime_error(ERROR_MSG_RCV);

        // send the public key to the server
        sendMsg(serverSocket, public_key.c_str());

        server_secret = generateSharedSecret(server_public_key, private_key);

        // receive challenge from server
        char challenge[4096] = "";
        recvMsg(serverSocket, challenge);
        cout << challenge << endl;

        sendMsg(serverSocket, CHALLENGE_RCV.c_str());
        char challenge_len[4096] = "";
        recvMsg(serverSocket, challenge_len);

        // string challenge_decrypt = decryptString(challenge, reinterpret_cast<const unsigned char *>(server_secret.c_str()), atoi(challenge_len));

        // send the challenge to the server

        // auto challenge_encrypt = encryptString(challenge_decrypt + " client!1234567", reinterpret_cast<const unsigned char *>(server_secret.c_str()));
        string challenge_encrypt = challenge + string(" client!1234567");
        sendMsg(serverSocket, to_string(challenge_encrypt.size()).c_str());
        char response[4096] = "";
        recvMsg(serverSocket, response);
        sendMsg(serverSocket, (challenge_encrypt).c_str());

        cerr << SERVER_CONNECTED << endl;
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
    cout << CLOSING_SOCKET_CLT << endl;
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
    string commands = separator + "Commands:\n";
    if (!logged)
    {
        commands += CMD_LOGIN + ": login to the system\n";
        commands += CMD_REGISTRATION + ": register to the system\n";
    }
    else
    {
        commands += CMD_ADD + ": add a new message on the board\n";
        commands += CMD_LIST + ": list the messages on the board\n";
        commands += CMD_GET + ": get a message from the board\n";
        commands += CMD_LOGOUT + ": logout from the system\n";
    }
    commands += CMD_GETLOCAL + ": get a local message\n";
    commands += CMD_EXIT + ": close the connection\n";
    return commands + separator;
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
        cout << PROMPT_COMMAND;
        getline(cin, command);
        if (!isLogged)
        {
            if (command == CMD_LOGIN)
            {
                isLogged = handleLogin(clientSocket);
                if (isLogged)
                    cout << LOGIN_OK << endl;
                else
                    cout << LOGIN_FAIL << endl;
            }
            else if (command == CMD_REGISTRATION)
            {
                isLogged = handleRegistration();
                if (isLogged)
                    cout << REGISTRATION_OK_FULL << endl;
                else
                    cout << REGISTRATION_FAIL << endl;
            }
            else if (command == CMD_GETLOCAL)
                getLocal();
            else if (command == CMD_ADD || command == CMD_LIST || command == CMD_GET || command == CMD_LOGOUT)
                cout << LOGIN_REQUIRED << endl;
            else if (command == CMD_EXIT || command == CMD_QUIT)
                isExited = handleQuit(clientSocket, isLogged);
            else if (command == CMD_CLOSING)
                isExited = handleExit(clientSocket);
            else
                cout << INVALID_COMMAND << endl;
        }
        else
        {
            if (command == CMD_ADD)
                handleAdd(clientSocket);
            else if (command == CMD_LIST)
                handleList(clientSocket);
            else if (command == CMD_GET)
                handleGet(clientSocket);
            else if (command == CMD_GETLOCAL)
                getLocal();
            else if (command == CMD_LOGOUT)
                handleLogout();
            else if (command == CMD_LOGIN || command == CMD_REGISTRATION)
                cout << ALREADY_LOGGED_IN << endl;
            else if (command == CMD_EXIT || command == CMD_QUIT)
                isExited = handleQuit(clientSocket, isLogged);
            else if (command == CMD_CLOSING)
                isExited = handleExit(clientSocket);
            else
                cout << INVALID_COMMAND << endl;
        }
    }
    close(clientSocket);
}

bool ClientHandler::handleRegistrationChallenge(int socket)
{

    char otp_challenge[4096] = "";
    recvMsg(socket, otp_challenge);

    if (strcmp(otp_challenge, OTP_SENT.c_str()) != 0)
        return false;
    cout << OTP_GENERATED_PROMPT << endl;

    string otp = "";
    cout << OTP_PROMPT_ENTER;
    getline(cin, otp);
    ;

    cout << OTP_PROMPT_BACK << endl;
    sendMsg(socket, otp.c_str());

    return true;
}

bool ClientHandler::handleRegistration()
{
    string message = CMD_REGISTRATION;
    sendMsg(clientSocket, message.c_str());
    char response[4096] = "";
    recvMsg(clientSocket, response);
    if (strcmp(response, CMD_REGISTRATION_RCV.c_str()) != 0)
        return false;

    string username, email, password;

    cout << PROMPT_USERNAME;
    getline(cin, username);
    sendMsg(clientSocket, username.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, CMD_USERNAME_RCV.c_str()) != 0)
        return false;

    cout << PROMPT_EMAIL;
    getline(cin, email);
    sendMsg(clientSocket, email.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, CMD_EMAIL_RCV.c_str()) != 0)
        return false;
    while (true)
    {
        cout << PROMPT_PASSWORD;
        getline(cin, password);
        sendMsg(clientSocket, password.c_str());
        recvMsg(clientSocket, (response));

        if (strcmp(response, CMD_PASSWORD_RCV.c_str()) == 0)
            break;
    }
    sendMsg(clientSocket, OK.c_str());
    handleRegistrationChallenge(clientSocket);
    recvMsg(clientSocket, (response));

    return (strcmp(response, REGISTRATION_OK.c_str()) == 0);
}

void ClientHandler::handleList(int clientSocket)
{
    string number;
    int start;
    int end;
    bool validInput = false;
    while (!validInput)
    {
        cout << PROMPT_LIST_FROM;
        getline(cin, number);
        try
        {
            start = stoi(number);
            validInput = true;
        }
        catch (const exception &e)
        {
            cout << INTEGER_REQUIRED << endl;
            validInput = false;
        }
        cout << PROMPT_LIST_TO;
        getline(cin, number);
        try
        {
            end = stoi(number);
            validInput = true;
        }
        catch (const exception &e)
        {
            cout << INTEGER_REQUIRED << endl;
            validInput = false;
        }
    }
    string msg = CMD_LIST + " " + to_string(start) + " " + to_string(end);
    sendMsg(clientSocket, msg.c_str());

    char message[4096] = "";
    recvMsg(clientSocket, message);
    // il server conferma la ricezione del comando
    if (strcmp(message, CMD_LIST_RCV.c_str()) != 0)
    {
        cout << INVALID_NOTA << endl;
        return;
    }
    else
    {
        cout << CMD_RCV << endl;
        sendMsg(clientSocket, OK.c_str());
        while (true)
        {
            memset(message, 0, sizeof(message));
            recvMsg(clientSocket, message);
            if (strcmp(message, STOP.c_str()) == 0)
                break;
            else
            {
                cout << message << endl;
                sendMsg(clientSocket, OK.c_str());
            }
        }
        sendMsg(clientSocket, STOP.c_str());
    }
}

void ClientHandler::handleLogout()
{
    sendMsg(clientSocket, CMD_LOGOUT.c_str());

    // check if server response with a "ricevuto_logout" message otherwise return
    char message[4096] = "";
    recvMsg(clientSocket, (message));
    if (strcmp(message, CMD_LOGOUT_OK.c_str()) != 0)
    {
        cout << CMD_LOGOUT_ERROR << endl;
        return;
    }
    else
        cout << CMD_LOGOUT_OK << endl;

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
    sendMsg(clientSocket, CMD_LOGIN.c_str());
    char response[4096] = "";
    recvMsg(clientSocket, (response));
    if (strcmp(response, CMD_LOGIN_RCV.c_str()) != 0)
        return false;
    string username;
    string password;
    cout << PROMPT_USERNAME;
    getline(cin, username);

    sendMsg(clientSocket, username.c_str());

    recvMsg(clientSocket, response);
    if (strcmp(response, CMD_USERNAME_RCV.c_str()) != 0)
        return false;

    cout << PROMPT_PASSWORD;
    getline(cin, password);

    sendMsg(clientSocket, password.c_str());
    recvMsg(clientSocket, (response));

    if (strcmp(response, CMD_PASSWORD_RCV.c_str()) != 0)
        return false;

    sendMsg(clientSocket, OK.c_str());
    recvMsg(clientSocket, response);
    return strcmp(response, CMD_LOGIN_OK.c_str()) == 0;
}

void ClientHandler::getLocal()
{
    // prendi in input il nome del file
    string filename;
    cout << "Enter the name of the file: ";
    getline(cin, filename);
    // dal nome del file prendi il valore numerico tra & e .txt
    if (filename.find("&") == string::npos || filename.find(".txt") == string::npos)
    {
        cout << "Invalid file name" << endl;
        return;
    }
    int len = stoi(filename.substr(filename.find("&") + 1, filename.find(".txt")));
    // prendi in input la password
    string password;
    cout << "Enter the password: ";
    getline(cin, password);
    // unsigned char *key = (unsigned char *)computeSHA3_512Hash(password).substr(0, 31).c_str();
    // password = "";
    // prendi dal file il primo rigo e mettilo come lunghezza
    auto str_key = computeSHA3_512Hash(password).substr(0, 32);
    unsigned char key[32];
    memcpy(key, str_key.c_str(), 32);
    // unsigned char *key = (unsigned char *)("UnaChiaveSegretaMoltoLunga12345");
    unsigned char iv[32] = "";
    unsigned char cipher_file[4096];

    ifstream rfile(filename);
    if (!rfile.is_open() && ".txt" != filename.substr(filename.find_last_of(".")))
    {
        cout << "Error opening file" << endl;
        return;
    }
    rfile.read(reinterpret_cast<char *>(cipher_file), len);
    rfile.close();

    unsigned char plaintext[len];
    int dec_size = decrypt(cipher_file, len, key, plaintext, iv);
    vector<unsigned char> data_dec(plaintext, plaintext + dec_size);
    string data2;
    data2 = string(data_dec.begin(), data_dec.end());
    int identifier = stoi(data2.substr(0, data2.find(delimiter)));
    data2 = data2.substr(data2.find(delimiter) + delimiter.size());
    string title = data2.substr(0, data2.find(delimiter));
    data2 = data2.substr(data2.find(delimiter) + delimiter.size());
    string author = data2.substr(0, data2.find(delimiter));
    data2 = data2.substr(data2.find(delimiter) + delimiter.size());
    string body = data2;
    Message msg = Message(identifier, title, author, body);
    cout << msg << endl;
}

void ClientHandler::saveMessage(Message &msg)
{
    // save the message to a file and encrypt with aes nella cartella client
    // prendi la password in input
    string password;
    bool validPassword = false;
    while (!validPassword)
    {
        cout << "Enter the password to encrypt the message lenght at least 10 characters, with at least one uppercase letter and one number: ";
        getline(cin, password);
        // Check password length
        if (password.length() < 10)
        {
            cout << "Password must be at least 10 characters long." << endl;
            continue;
        }
        // Check for uppercase letter
        bool hasUppercase = false;
        for (char c : password)
        {
            if (isupper(c))
            {
                hasUppercase = true;
                break;
            }
        }
        if (!hasUppercase)
        {
            cout << "Password must contain at least one uppercase letter." << endl;
            continue;
        } // Check for numbers
        bool hasNumber = false;
        for (char c : password)
        {
            if (isdigit(c))
            {
                hasNumber = true;
                break;
            }
        }
        if (!hasNumber)
        {
            cout << "Password must contain at least one number." << endl;
            continue;
        }
        validPassword = true;
    } // fai lo sha della password e prendi i primi 31 caratteri e converti la key in unsigned char*

    auto str_key = computeSHA3_512Hash(password).substr(0, 32);
    unsigned char key[32];
    memcpy(key, str_key.c_str(), 32);

    // elimina password
    password = "";
    unsigned char cipher[4096];
    unsigned char iv[32] = "";
    int cipher_len = encrypt(msg.serialize(), msg.serialize().length(), key, cipher, iv);
    string filename = "msg_" + msg.getTitle() + "&" + to_string(cipher_len) + ".txt";
    replace(filename.begin(), filename.end(), ' ', '_');
    ofstream file(filename);
    file.flush();
    // scrivi il cipher nel file in hex
    file.write(reinterpret_cast<char *>(cipher), cipher_len);
    file.close();
}

void ClientHandler::handleGet(int clientSocket)
{
    // send get to server due to perform the get operation
    sendMsg(clientSocket, CMD_GET.c_str());
    char message[4096] = "";
    // check if server response with a "ricevuto_get" message otherwise return
    recvMsg(clientSocket, message);
    if (strcmp(message, CMD_GET_RCV.c_str()) != 0)
    {
        cout << INVALID_GET << endl;
        return;
    }

    string msgId = "";
    cout << CMD_GET_SEARCH_PROMPT;
    getline(cin, msgId);
    // send the message id to the server
    sendMsg(clientSocket, msgId.c_str());
    // check if server response with a "ok" message otherwise return
    recvMsg(clientSocket, message);
    if (strcmp(message, ERROR.c_str()) == 0)
    {
        cout << ERROR_MSG_NOT_FOUND << endl;
        return;
    }

    sendMsg(clientSocket, OK.c_str());

    // return the message from the server
    recvMsg(clientSocket, message);
    Message msg = Message(message);
    saveMessage(msg);
    sendMsg(clientSocket, END.c_str());
    if (msg.getTitle() == "" && msg.getBody() == "")

        cout << ERROR_MSG_EMPTY << endl;
    else
        cout << msg;
}
void ClientHandler::handleAdd(int clientSocket)
{
    sendMsg(clientSocket, CMD_ADD.c_str());
    char message[4096] = "";
    recvMsg(clientSocket, message);
    // il server conferma la ricezione del comando
    if (strcmp(message, CMD_ADD_RCV.c_str()) == 0)
    {
        // crea la nota da inviare con titolo e body
        string note;
        string temp;
        cout << NOTA_NEW_PROMPT << endl;
        cout << NOTA_TITLE_PROMPT;
        getline(cin, temp);
        note += NOTA_TITLE + temp + "\n";
        cout << NOTA_BODY_PROMPT;
        getline(cin, temp);
        note += NOTA_BODY + temp;

        // invio la nota
        sendMsg(clientSocket, note.c_str());
        // ricevo la conferma della nota
        recvMsg(clientSocket, message);
        if (strcmp(message, CMD_NOTA_RCV.c_str()) != 0)
            cout << INVALID_NOTA << endl;
        else if (strcmp(message, CMD_NOTA_RCV.c_str()) == 0)
        {
            cout << CMD_NOTA_SAVED << endl;
            sendMsg(clientSocket, STOP.c_str());
        }
    }
    else // se il comando non è stato accettato dal server
        cout << ERROR << endl;
}

int main()
{
    ClientHandler clientHandler;
    return 0;
}

// g++ /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client.cpp -o /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client && /home/bho/Applied-Cryptography-Bulletin-Board-System/client/client
