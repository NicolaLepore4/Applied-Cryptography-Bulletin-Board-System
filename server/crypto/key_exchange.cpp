#include <iostream>
#include <openssl/evp.h>
#include <openssl/dh.h>
#include <cmath>
using namespace std;

class Key_Exchange{
    private:
        BIGNUM *p, *g, *a, *b, *A, *B, *s;
    public:
        void dh_key_server();
        void dh_key_client();
        unsigned char* getP();
        unsigned char* getG();
        unsigned char* getPublicKey();
        void setClientPublicKey(unsigned char* pub_key_str);
        unsigned char* getSecret();
        DH *create_dh_key();
        
};

DH *Key_Exchange::create_dh_key() {
    DH *dh = DH_new();
    if (!dh) {
        std::cerr << "Error creating DH key" << std::endl;
        return nullptr;
    }
    DH_generate_parameters_ex(dh, 128, DH_GENERATOR_2, nullptr);
    DH_generate_key(dh);
    DH_get0_pqg(dh, &p, nullptr, &g);
    return dh;
}

unsigned char* Key_Exchange::getP(){
    if (p == nullptr) {
        DH_get0_pqg(dh, &p, nullptr, nullptr);
    }
    char p_str[1024];
    BN_bn2bin(p, (unsigned char*)p_str);
    return p_str;
}

unsigned char* Key_Exchange::getG(){
    if (g == nullptr) {
        DH_get0_pqg(dh, nullptr, nullptr, &g);
    }
    char g_str[1024];
    BN_bn2bin(g, (unsigned char*)g_str);
    return g_str;
}

unsigned char* Key_Exchange::getPublicKey(){
    if (pub_key == nullptr) {
        DH_get0_key(dh, &pub_key, nullptr);
    }
    char pub_key_str[1024];
    BN_bn2bin(pub_key, (unsigned char*)pub_key_str);
    return pub_key_str;

}

void Key_Exchange::setClientPublicKey(unsigned char* pub_key_str){
    BIGNUM *recv_pub_key = BN_bin2bn((unsigned char*)pub_key_str, 1024, nullptr);
    unsigned char secret[DH_size(dh)];
    DH_compute_key(secret, recv_pub_key, dh);
    return secret;
}

int main(){
    Key_Exchange key_exchange;
    
    auto p = Key_Exchange.getP();
    auto g = Key_Exchange.getG();
    auto publicKey = Key_Exchange.getPublicKey();
    send(clientSocket, &p, sizeof(p), 0);
    send(clientSocket, &g, sizeof(g), 0);
    send(clientSocket, &publicKey, sizeof(publicKey), 0);

    char recv_pub_key_client[1024] = "";
    recv(clientSocket, recv_pub_key_client, sizeof(recv_pub_key_client), 0);
    auto client_secret = Key_Exchange.setClientPublicKey(recv_pub_key_client);

    cout << "Client secret: " << client_secret << endl;
    string msg_c = "Connection established";

    for (int i = 0; i < msg_c.size(); i++)
        msg_c[i] = msg_c[i] ^ client_secret;

    cout<<"Encrypted message: "<<msg_c<<endl;

}