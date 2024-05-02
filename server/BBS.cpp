#include <iostream>
#include <fstream>
#include <string>

#include "Message.cpp"
#include <unordered_map>
#include <set>
using namespace std;

// Class representing the Bulletin Board System
class BBS
{
private:
    // create hash map to store messages by id
    unordered_map<int, Message> messages;
    string filenameMSG = "";

public:
    // Constructor to load messages from file
    BBS(string filenameMSG);

    // Method to list the latest n available messages in the BBS
    set<Message> List(int n);

    // Method to download a message specified by message identifier
    Message Get(int mid);

    // Method to add a message to the BBS
    void Add(string title, string author, string body);
    
};

BBS::BBS(string filenameMSG)
    {
        // funzione che legge da file i messaggi
        this->filenameMSG = filenameMSG;
        if (filenameMSG == "")
        {
            // TODO: throw exception or print error
            return;
        }
        ifstream file = ifstream(filenameMSG);
        string line;
        while (getline(file, line))
        {
            Message m = Message::deserialize(line);
            messages.insert({m.getIdentifier(), m});
        }
        file.close();

        cout << "BBS loaded from file " << messages.size() << " messages" << endl;
    }
    // Method to list the latest n available messages in the BBS
    set<Message> BBS::List(int n)
    {
        // return set<Message>(messages.begin(), messages.end());
        // TODO: implementare un metodo per restituire gli ultimi n messaggi
        set<Message> result = set<Message>();
        for (auto msg : messages)
        {
            result.insert(msg.second);
            if (result.size() >= n)
                break;
        }

        return result;
    }

    // Method to download a message specified by message identifier
    Message BBS::Get(int mid)
    {
        if (messages.find(mid) != messages.end())
            return messages[mid];
        return Message(-1, "", "", "");
    }

    // Method to add a message to the BBS
    void BBS::Add(string title, string author, string body)
    {
        int id = messages.size() + 1;
        Message m = Message(id, title, author, body);
        messages.insert({id, m});
        ofstream file = ofstream(filenameMSG, ios::app);
        file << m.serialize() << endl;
        file.close();
        // funzione che scrive su file il messaggio
    }