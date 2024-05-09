#include <iostream>
#include <string>
#include "crypto/aes.cpp"
using namespace std;


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
    static Message deserialize(vector<unsigned char> data,string key, string iv);

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
        int identifier = stoi(str.substr(0, str.find(",")));
        str = str.substr(str.find(",") + 1);
        string title = str.substr(0, str.find(","));
        str = str.substr(str.find(",") + 1);
        string author = str.substr(0, str.find(","));
        str = str.substr(str.find(",") + 1);
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
        return to_string(identifier) + "," + title + "," + author + "," + body;
    }
    string Message::serialize_for_list() {
        return to_string(identifier) + "," + title + "," + author;
    }

    Message Message::deserialize(vector<unsigned char> data,string key, string iv) {
        
        vector<unsigned char> vec(data.begin(), data.end());
        vector<unsigned char> data_dec = decrypt_AES(vec,  reinterpret_cast<const unsigned char*>(key.c_str()), reinterpret_cast<const unsigned char*>(iv.c_str()));
        string data2;
        data2 = string(data_dec.begin(), data_dec.end());
        int identifier = stoi(data2.substr(0, data2.find(",")));
        data2 = data2.substr(data2.find(",") + 1);
        string title = data2.substr(0, data2.find(","));
        data2 = data2.substr(data2.find(",") + 1);
        string author = data2.substr(0, data2.find(","));
        data2 = data2.substr(data2.find(",") + 1);
        string body = data2;
        return Message(identifier, title, author, body);
    }

    // Overloading the < operator for sorting
    bool Message::operator<(const Message& message) const {
        return identifier < message.identifier;
    }