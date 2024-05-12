#include <iostream>
#include <string>
#include "crypto/aes.cpp"
using namespace std;

const string delimiter = "/,";
// Structure to represent a message
class Message {
private:
    int identifier = 0; // Unique identifier for the message of 6 elements
    string title = " ", author = " ", body = " ";
    
public:
    Message() = default;
    Message(Message const &message);
    Message(const int& identifier, const string& title, const string& author, const string& body);
    Message(char*);
    int getIdentifier();
    string getTitle();
    string getAuthor();
    string getBody();
    friend ostream& operator<<(ostream& os, const Message& message);
    friend istream& operator>>(istream& is, Message& message);
    string serialize();
    string serialize_for_list();
    static Message deserialize(vector<unsigned char> data,unsigned char* key, unsigned char* iv);

    // create operator < and > for sorting
    bool operator<(const Message&) const;
    
};

    // Copy constructor
    Message::Message(Message const &message) {
        this->identifier = message.identifier;
        this->title = message.title;
        this->author = message.author;
        this->body = message.body;
    }
    // Constructor
    Message::Message(const int& identifier, const string& title, const string& author, const string& body) {
        this->identifier = identifier;
        this->title = title;
        this->author = author;
        this->body = body;
    }

    // Constructor to deserialize data
    Message::Message(char* data) {
        string str(data);
        int identifier = stoi(str.substr(0, str.find(delimiter)));
        str = str.substr(str.find(delimiter) + 2);
        string title = str.substr(0, str.find(delimiter));
        str = str.substr(str.find(delimiter) + 2);
        string author = str.substr(0, str.find(delimiter));
        str = str.substr(str.find(delimiter) + 2);
        string body = str;
        this->identifier = identifier;
        this->title = title;
        this->author = author;
        this->body = body;
    }

    // Getters
    int Message::getIdentifier() {
        return identifier;
    }

    string Message::getTitle() {
        return title;
    }

    string Message::getAuthor() {
        return author;
    }

    string Message::getBody() {
        return body;
    }

    // Overloading the << and >> operators
    ostream& operator<<(ostream& os, const Message& message) {
        os << "Identifier: " << message.identifier << endl;
        os << "Title: " << message.title << endl;
        os << "Author: " << message.author << endl;
        os << "Body: " << message.body << endl;
        return os;
    }

    istream& operator>>(istream& is, Message& message) {
        cout << "Enter the title: ";
        is >> message.title;
        cout << "Enter the author: ";
        is >> message.author;
        cout << "Enter the body: ";
        is >> message.body;
        return is;
    }

    // Serialize and deserialize functions
    string Message::serialize() {
        return to_string(identifier) + delimiter + title + delimiter + author + delimiter + body;
    }
    string Message::serialize_for_list() {
        return to_string(identifier) + ";;" + title + ";;" + author;
    }

    Message Message::deserialize(vector<unsigned char> data,unsigned char* key, unsigned char* iv) {
        unsigned char* cipher = reinterpret_cast<unsigned char*>(data.data());
        unsigned char plaintext[2048];
        int dec_size = decrypt(cipher, data.size(), key, plaintext, iv);
        cout << "Decrypted size: " << dec_size << endl;
        cout << "Decrypted text: " << plaintext << endl;
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
        return Message(identifier, title, author, body);
    }

    // Overloading the < operator for sorting
    bool Message::operator<(const Message& message) const {
        return identifier < message.identifier;
    }