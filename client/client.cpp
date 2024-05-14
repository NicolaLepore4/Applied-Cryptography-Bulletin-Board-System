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
#include "../common/protocol.cpp"

using namespace std;

const int port = 12345;
const string ip = "127.0.0.1";

class ClientHandler
{
private:
    int clientSocket;
    bool isLogged = false;
    string username;
    string public_key, private_key, server_secret = "";

    void sendMsg(int clientSocket, const char *msg)
    {
        int size = 4096;
        if (server_secret == "")
        {
            int n = send(clientSocket, msg, size, 0);
            if (n < 0)
                throw runtime_error(ERROR_MSG_SENT);
            return;
        }
        auto msg_encrypt = encryptString(msg, reinterpret_cast<const unsigned char *>(server_secret.c_str()));

        int msg_encrypt_first_len = msg_encrypt.second;
        send(clientSocket, to_string(msg_encrypt_first_len).c_str(), size, 0);
        char response[4096] = "";
        recv(clientSocket, response, size, 0);

        int n = send(clientSocket, msg_encrypt.first.c_str(), msg_encrypt.first.size(), 0);

        if (n < 0)
            throw runtime_error(ERROR_MSG_SENT);
    }

    void recvMsg(int clientSocket, char *msg, bool decrypt = true)
    {
        int size = 4096;
        recv(clientSocket, msg, size, 0);
        if (decrypt)
        {
            size_t server_secret_len = atoi(msg);
            send(clientSocket, "ricevuto", size, 0);
            recv(clientSocket, msg, size, 0);
            string msg_decrypt = decryptString(msg, reinterpret_cast<const unsigned char *>(server_secret.c_str()), server_secret_len);
            strcpy(msg, msg_decrypt.c_str());
        }
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

        string challenge_decrypt = decryptString(challenge, reinterpret_cast<const unsigned char *>(server_secret.c_str()), atoi(challenge_len));

        // send the challenge to the server

        auto challenge_encrypt = encryptString(challenge_decrypt + " client!1234567", reinterpret_cast<const unsigned char *>(server_secret.c_str()));

        sendMsg(serverSocket, to_string(challenge_encrypt.second).c_str());
        char response[4096] = "";
        recvMsg(serverSocket, response);
        sendMsg(serverSocket, (challenge_encrypt.first).c_str());

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
        if (!isLogged)
        {
            cout << PROMPT_COMMAND;
            getline(cin, command);
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
            cout << PROMPT_COMMAND;
            getline(cin, command);
            if (command == CMD_ADD)
                handleAdd(clientSocket);
            else if (command == CMD_LIST)
                handleList(clientSocket);
            else if (command == CMD_GET)
                handleGet(clientSocket);
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
    cin >> otp;

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
    if (strcmp(message, CMD_LOGOUT_RCV.c_str()) != 0)
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

    recvMsg(clientSocket, response);
    return strcmp(response, CMD_LOGIN_OK.c_str()) == 0;
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
