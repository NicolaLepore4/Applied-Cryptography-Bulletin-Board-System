#include <iostream>
using namespace std;

class User
{
private:
    string username, mail, password, salt;

public:
    User(string name, string password, string mail, string salt)
    {
        this->username = name;
        this->password = password;
        this->mail = mail;
        this->salt = salt;
    }

    string getUsername()
    {
        return username;
    }

    string getPassword()
    {
        return password;
    }

    string getMail()
    {
        return mail;
    }

    string getSalt()
    {
        return salt;
    }
};