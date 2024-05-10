#include <iostream>
#include <fstream>
#include <string>
#include "Message.cpp"
#include <unordered_map>
#include <set>
using namespace std;

vector<std::vector<unsigned char>> readBytesFromFile(const std::string& filename) {
    // Apri il file
    std::ifstream file(filename);

    // Controlla se il file è stato aperto correttamente
    if (!file) {
        throw std::runtime_error("Could not open file " + filename);
    }

    // Leggi dal token /start al token /end
    std::vector<std::vector<unsigned char>> result;
    std::vector<unsigned char> buffer;
    std::string line;

    // insert in buffer the lines between /start and /end and not the tokens
    while (std::getline(file, line)) {
        if (line == "/start")
            buffer.clear();
        else if (line == "/end")
            result.push_back(buffer);
        else
            buffer.insert(buffer.end(), line.begin(), line.end());
    }
    return result;
}

// Class representing the Bulletin Board System
class BBS
{
private:
    // create hash map to store messages by id
    unordered_map<int, Message> messages;
    string filenameMSG = "";
    string key, iv;

public:
    // Constructor to load messages from file
    BBS(string filenameMSG, string key, string iv);

    // Method to list the latest n available messages in the BBS
    set<Message> List(int start, int end);

    // Method to download a message specified by message identifier
    Message Get(int mid);

    // Method to add a message to the BBS
    void Add(string title, string author, string body);

    // method to return the size of the BBS
    int size();

    int retrieveLastId();
};

BBS::BBS(string filenameMSG, string key, string iv)
{
    this->key = key;
    this->iv = iv;
    // funzione che legge da file i messaggi
    this->filenameMSG = filenameMSG;
    if (filenameMSG == "")
    {
        // TODO: throw exception or print error
        return;
    }
    vector<std::vector<unsigned char>> dati = readBytesFromFile(filenameMSG);

    // each vector in dati has the following structure:
    // /start
    // <message>
    // /end

    for (auto& v : dati){
        Message m = Message::deserialize(v, key, iv);
        messages.insert({m.getIdentifier(), m});
    }

    cout << "BBS loaded from file " << messages.size() << " messages" << endl;
}
// Method to list the latest n available messages in the BBS
set<Message> BBS::List(int start, int end)
{
    cout << "Selected " <<" selectedMessages.size()" << " messages" << endl;
    set<Message> selectedMessages;
    for (auto it = messages.begin(); it != messages.end(); ++it)
    {
        // Check if the message ID is within the range [start, end]
        if (it->second.getIdentifier() >= start && it->second.getIdentifier() <= end)
        {
            selectedMessages.insert(it->second);
        }
    }
    
    return selectedMessages;
}

// Method to download a message specified by message identifier
Message BBS::Get(int mid)
{
    if (messages.find(mid) != messages.end())
        return messages[mid];
    return Message(-1, "", "", "");
}

int BBS::retrieveLastId()
{
    Message lastMessage;
    int max = 0;
    for (const auto& pair : messages) 
    {
        if (max < pair.first)
                max = pair.first;
    }
    return max;
}
    
int BBS::size()
{
    return messages.size();
}

// Method to add a message to the BBS
void BBS::Add(string title, string author, string body)
{
    int id = retrieveLastId() + 1;
    Message m = Message(id, title, author, body);
    messages.insert({id, m});
    ofstream file(filenameMSG, ios::binary | ios::app);
    vector<unsigned char> enc_vec = encrypt_AES(m.serialize(), reinterpret_cast<const unsigned char *>(key.c_str()), reinterpret_cast<const unsigned char *>(iv.c_str()));
    cout<< string(enc_vec.begin(), enc_vec.end()) << endl;
    file.write("/start\n", strlen("/start\n"));
    file.write(reinterpret_cast<const char *>(enc_vec.data()), enc_vec.size());
    file.write("\n/end\n", strlen("\n/end\n"));

    file.close();
    // funzione che scrive su file il messaggio
}