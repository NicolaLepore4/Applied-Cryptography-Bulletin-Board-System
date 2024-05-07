#include <iostream>

using namespace std;

class Exception {
public:
    Exception(const char* message) : message(message) {
        print();
    }

    friend ostream& operator<<(ostream& os, const Exception& e) {
        os << e.message;
        return os;
    }

private:
    const char* message;
    void print() const {
        cout << message << endl;
    }
};