#include  <stdio.h>
#include <openssl/evp.h>
#include <openssl/conf.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iomanip>

using namespace std;

int encrypt(string text, int text_len, unsigned char* key, unsigned char* cipher, unsigned char* iv){
    int cipher_len = 0;
    int len = 0;
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if(!ctx){
        perror("EVP_CIPHER_CTX_new failed");
        exit(-1);
    }
    if(!EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
        perror("EVP_EncryptInit_ex failed");
        exit(-1);
    }
    if(!EVP_EncryptUpdate(ctx, cipher, &len, (unsigned char*) text.c_str(), text_len)){
        perror("EVP_EncryptUpdate failed");
        exit(-1);
    }
    cipher_len += len;
    if(!EVP_EncryptFinal_ex(ctx, cipher + len, &len)){
        perror("EVP_EncryptFinal_ex failed");
        exit(-1);
    }
    cipher_len += len;
    EVP_CIPHER_CTX_free(ctx);

    return cipher_len;
}

int decrypt(unsigned char* cipher, int cipher_len, unsigned char* key, unsigned char* text, unsigned char* iv){
    int text_len = 0;
    int len = 0;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

    if(!ctx){
        perror("EVP_CIPHER_CTX_new failed");
        exit(-1);
    }
    if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv)){
        perror("EVP_DecryptInit_ex failed");
        exit(-1);
    }
    if(!EVP_DecryptUpdate(ctx, text, &len, cipher, cipher_len)){
        perror("EVP_DecryptUpdate failed");
        exit(-1);
    }  
    text_len += len;
    if(!EVP_DecryptFinal_ex(ctx, text+len,&len)){
        perror("EVP_DecryptFinal_ex failed");
        exit(-1);
    }
    text_len += len;
    EVP_CIPHER_CTX_free(ctx);
    return text_len;
}