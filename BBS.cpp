#include <iostream>
#include <deque>
#include <string>

// Structure to represent a message
struct Message {
    int identifier;
    std::string title;
    std::string author;
    std::string body;
};

// Class representing the Bulletin Board System
class BBS {
private:
    std::deque<Message> messages;
    int nextId = 0;

public:
    // Method to list the latest n available messages in the BBS
    void List(int n) {
        for (int i = std::max(0, static_cast<int>(messages.size()) - n); i < messages.size(); ++i) {
            std::cout << "Title: " << messages[i].title << "\n"
                      << "Author: " << messages[i].author << "\n"
                      << "Body: " << messages[i].body << "\n\n";
        }
    }

    // Method to download a message specified by message identifier
    void Get(int mid) {
        for (const auto& message : messages) {
            if (message.identifier == mid) {
                std::cout << "Title: " << message.title << "\n"
                          << "Author: " << message.author << "\n"
                          << "Body: " << message.body << "\n\n";
                return;
            }
        }
        std::cout << "Message with id " << mid << " not found.\n";
    }

    // Method to add a message to the BBS
    void Add(std::string title, std::string author, std::string body) {
        messages.emplace_back(std::move(title), std::move(author), std::move(body), nextId++);
    }
};

int main() {
    // TODO: Implement the main function to test the BBS class

    return 0;
}