#include <iostream>
#include <openssl/evp.h>
#include <string>
#include <vector>
#include <string.h>

using namespace std;

// Funzione per crittografare una stringa con AES
vector<unsigned char> encrypt_AES(const string &plaintext, const unsigned char *key, const unsigned char *iv)
{

    // Buffer per la stringa in ingresso
    unsigned char in_buf[plaintext.size() + 1];
    strcpy((char *)in_buf, plaintext.c_str());

    // Buffer per la stringa crittografata ho tolto (+ EVP_CIPHER_block_size())
    unsigned char out_buf[plaintext.size() * 2 + 16];
    int out_len;

    // Contesto di crittografia AES
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        throw runtime_error("Errore: Allocazione contesto crittografia fallita.");
    }

    // Inizializzazione del contesto per la crittografia AES-CBC
    if (EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, 1) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Errore: Inizializzazione contesto crittografia fallita.");
    }

    // Crittografia della stringa
    if (EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, plaintext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Errore: Aggiornamento crittografia fallito.");
    }

    // Finalizzazione della crittografia e scrittura dei dati rimanenti
    int out_len_final;
    if (EVP_CipherFinal_ex(ctx, out_buf + out_len, &out_len_final) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Errore: Finalizzazione crittografia fallita.");
    }

    // Liberazione del contesto di crittografia
    EVP_CIPHER_CTX_free(ctx);

    // Restituisce la stringa crittografata
    vector<unsigned char> encrypted_data(out_len + out_len_final);
    copy(out_buf, out_buf + out_len + out_len_final, encrypted_data.begin());
    return encrypted_data;
}

// Funzione per decrittografare una stringa con AES
vector<unsigned char> decrypt_AES(const vector<unsigned char> &ciphertext, const unsigned char *key, const unsigned char *iv)
{
    // Buffer per la stringa crittografata
    unsigned char in_buf[ciphertext.size()];
    copy(ciphertext.begin(), ciphertext.end(), in_buf);

    // Buffer per la stringa decrittografata (dimensione massima) ho tolto (+ EVP_CIPHER_block_size())
    unsigned char out_buf[ciphertext.size() + 16];
    int out_len;

    // Contesto di crittografia AES
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (ctx == NULL)
    {
        throw runtime_error("Errore: Allocazione contesto crittografia fallita.");
    }

    // Inizializzazione del contesto per la decrittografia AES-CBC
    if (EVP_CipherInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv, 0) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Errore: Inizializzazione contesto decrittografia fallita.");
    }

    // Decrittografia della stringa
    if (EVP_CipherUpdate(ctx, out_buf, &out_len, in_buf, ciphertext.size()) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Errore: Aggiornamento decrittografia fallito.");
    }

    // Finalizzazione della decrittografia e recupero dei dati rimanenti
    int out_len_final;
    if (EVP_CipherFinal_ex(ctx, out_buf + out_len, &out_len_final) != 1)
    {
        EVP_CIPHER_CTX_free(ctx);
        throw runtime_error("Errore: Finalizzazione decrittografia fallita.");
    }

    // Liberazione del contesto di crittografia
    EVP_CIPHER_CTX_free(ctx);

    // Restituisce la stringa decrittografata
    vector<unsigned char> decrypted_data(out_len + out_len_final);
    copy(out_buf, out_buf + out_len + out_len_final, decrypted_data.begin());
    return decrypted_data;
}

int main()
{

    // Chiave di crittografia (32 byte)
    unsigned char key[] = "UnaChiaveSegretaMoltoLunga1234567890";

    // Vettore di inizializzazione (16 byte)
    unsigned char iv[] = "UnVettoreIniziale";

    // Stringa da crittografare
    string plaintext = "Nel mezzo del cammin di nostra vita mi ritrovai per una selva oscura,ché la diritta via era smarrita. Ahi quanto a dir qual era è cosa dura, esta selva selvaggia e aspra e forte, che nel pensier rinova la paura ! Tant'è amara che poco è più morte; ma per trattar del ben ch 'i' vi trovai, dirò de l 'altre cose ch' i ' v' ho scorte. Io non so ben ridir com 'i' v'intrai, tant'era pien di sonno a quel punto che la verace via abbandonai.";

    // Crittografa la stringa
    vector<unsigned char>encrypted_data = encrypt_AES(plaintext, key, iv);

    string str(encrypted_data.begin(), encrypted_data.end());
    // Converte la stringa crittografata in formato esadecimale

    cout << "Stringa crittografata: " << str << endl;

    // Decrypt the ciphertext
    vector<unsigned char> decrypted_data = decrypt_AES(encrypted_data, key, iv);

    // Print in hexadecimal format (useful for debugging)
    string str2(decrypted_data.begin(), decrypted_data.end());
    cout << "Decrypted string (hexadecimal): " << str2 << endl;

    return 0;
}
