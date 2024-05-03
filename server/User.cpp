#include <iostream>
#include <string>
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

    User(const User &u)
    {
        this->username = u.username;
        this->password = u.password;
        this->mail = u.mail;
        this->salt = u.salt;
    }

    string getUsername(){
        return username;
    }

    string getPassword(){
        return password;
    }

    static User deserialize(string data)
    {
        string delimiter = " ";
        size_t pos = 0;
        string token;
        string arr[4];
        int i = 0;
        while ((pos = data.find(delimiter)) != string::npos)
        {
            token = data.substr(0, pos);
            arr[i] = token;
            data.erase(0, pos + delimiter.length());
            i++;
        }
        arr[i] = data;
        User u(arr[0], arr[1], arr[2], arr[3]);
        return u;
    }
};