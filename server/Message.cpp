#include <iostream>
using namespace std;

// Structure to represent a message
class Message {
private:
    int identifier; // Unique identifier for the message of 6 elements
    string title, author, body;
    
public:
    Message(int identifier, string title, string author, string body) {
        this->identifier = identifier;
        this->title = title;
        this->author = author;
        this->body = body;
    }

    int getIdentifier() {
        return identifier;
    }

    string getTitle() {
        return title;
    }

    string getAuthor() {
        return author;
    }

    string getBody() {
        return body;
    }

    friend ostream& operator<<(ostream& os, const Message& message) {
        os << "Message ID: " << message.identifier << endl;
        os << "Title: " << message.title << endl;
        os << "Author: " << message.author << endl;
        os << "Body: " << message.body << endl;
        return os;
    }

    friend istream& operator>>(istream& is, Message& message) {
        cout << "Enter title: ";
        is >> message.title;
        cout << "Enter author: ";
        is >> message.author;
        cout << "Enter body: ";
        is >> message.body;
        return is;
    }

    // Function to serialize the message
    string serialize() {
        return to_string(identifier) + " |! " + title + " |! " + author + " |! " + body;
    }

    // Function to deserialize the message
    Message static deserialize(string data) {
        int pos = data.find(" |! ");
        int identifier = stoi(data.substr(0, pos));
        data = data.substr(pos + 1);
        pos = data.find(" |! ");
        string title = data.substr(0, pos);
        data = data.substr(pos + 1);
        pos = data.find(" |! ");
        string author = data.substr(0, pos);
        string body = data.substr(pos + 1);
        return Message(identifier, title, author, body);
    }
};
