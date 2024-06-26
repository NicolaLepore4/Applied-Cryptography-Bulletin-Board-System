#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <string>
using namespace std;

// ERROR CODES
const string ERROR_SOCKET = "Error! Cannot create socket";
const string ERROR_ADDRESS = "Error! Address family not supported";
const string ERROR_ADDRESS_INVALID = "Error! Invalid Address";
const string SERVER_UNAVAILABLE = "Error! Cannot connect to server";
const string ERROR_KEYPAIR = "Error! Cannot generate key pair";
const string ERROR_MSG_SENT = "Error! Cannot send message";
const string ERROR_MSG_RCV = "Error! Cannot receive message";
const string ERROR_CLT_DISCONNECTED = "Error! Cannot send message";

// SERVER SIDE MESSAGES
const string CHALLENGE_RCV = "Received challenge";

// CLIENT SIDE MESSAGES
const string SERVER_CONNECTED = "Connected to server";
const string CLOSING_SOCKET_CLT = "Closing connection with server...";
const string LOGIN_OK = "Login successful";
const string LOGIN_FAIL = "Login failed";
const string REGISTRATION_OK_FULL = "Registration successful";
const string REGISTRATION_FAIL = "Registration failed";
const string LOGIN_REQUIRED = "You must login first";
const string ALREADY_LOGGED_IN = "You are already logged in";
const string INVALID_COMMAND = "You have entered an invalid command";
const string ERROR_MSG_NOT_FOUND = "Error! Message not found";
const string ERROR_MSG_EMPTY = "Message is empty";

const string INTEGER_REQUIRED = "Invalid input. Please enter an integer.";
const string INVALID_NOTA = "Error! Invalid NOTA";
const string INVALID_GET = "Error! Invalid GET";
const string CMD_RCV = "Command received";
const string CMD_LOGOUT_ERROR = "Error! Cannot logout";
const string ERROR = "error";
// COMMANDS 
const string CMD_LOGIN = "login";
const string CMD_LOGOUT = "logout";
const string CMD_LIST = "list";
const string CMD_REGISTRATION = "registration";
const string CMD_ADD = "add";
const string CMD_GET = "get";
const string CMD_EXIT = "exit";
const string CMD_QUIT = "quit";
const string CMD_CLOSING = "closing";

const string CMD_REGISTRATION_RCV = "ricevuto_registration";
const string CMD_USERNAME_RCV = "ricevuto_username";
const string USERNAME_TAKEN = "username_already_taken";
const string CMD_EMAIL_RCV = "ricevuto_email";
const string CMD_PASSWORD_RCV = "ricevuto_password";
const string CMD_PASSWORD_SHORT = "password_too_short";
const string OK = "ok";
const string STOP = "stop";
const string END = "fine";
const string REGISTRATION_OK = "granted_registration";
const string REGISTRATION_KO = "denied_registration";
const string CMD_LIST_RCV = "ricevuto_list";
const string CMD_LOGOUT_RCV = "ricevuto_logout";
const string CMD_LOGOUT_OK = "logout successful";
const string CMD_LOGIN_RCV = "ricevuto_login";
const string CMD_LOGIN_OK = "granted_login";
const string CMD_LOGIN_KO = "denied_login";
const string CMD_GET_RCV = "ricevuto_get";
const string CMD_ADD_RCV = "ricevuto_add";
const string CMD_NOTA_RCV = "ricevuto_nota";
const string CMD_NOTA_SAVED = "Nota saved";
const string CMD_GETLOCAL = "get_local";

const string CMD_GET_SEARCH_PROMPT = "Insert the identifier of the message to search: ";

const string NOTA_TITLE = "Title: ";
const string NOTA_BODY = "Body: ";
const string NOTA_AUTHOR = "Author: ";

const string NOTA_NEW_PROMPT = "Enter new nota: ";
const string NOTA_TITLE_PROMPT = "Enter title: ";
const string NOTA_BODY_PROMPT = "Enter body: ";
const string OTP_SENT = "otp_sent";
const string OTP_GENERATED_PROMPT = "OTP Challenge received on file \"otp.txt\" in client folder";
const string OTP_PROMPT_ENTER = "Enter OTP: ";
const string OTP_PROMPT_BACK = "Send back OTP Challenge";
const string LIST_SENT_SUCCESS = "List sent successfully";

const string PROMPT_USERNAME = "Enter username: ";
const string PROMPT_PASSWORD = "Enter password: ";
const string PROMPT_EMAIL = "Enter email: ";
const string PROMPT_COMMAND = "Enter command: ";
const string PROMPT_LIST_FROM = "Elements from: ";
const string PROMPT_LIST_TO = "to: ";

#endif // PROTOCOL_H