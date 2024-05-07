#include <iostream>
#include <string>
#include <openssl/evp.h> //for all other OpenSSL function calls
#include <random>
#include <openssl/crypto.h>

using namespace std;

#define SHA3_512_DIGEST_LENGTH 512


// Funzione per generare un valore casuale (salt)
int generateSalt()
{
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 1000000); // Intervallo per il salt
  return dis(gen);
}

// Funzione per effettuare l'hash di una stringa con salting

std::string hashWithSalt(const std::string &password, int salt)
{
  // Convertire la password e il salt in array di byte
  unsigned char passwordBytes[password.size()];
  std::copy(password.begin(), password.end(), passwordBytes);

  unsigned char saltBytes[sizeof(salt)];
  std::copy((unsigned char *)&salt, (unsigned char *)&salt + sizeof(salt), saltBytes);

  // Creare un digest SHA3
  unsigned char digest[SHA3_512_DIGEST_LENGTH]; // Modifica la dimensione del digest
  EVP_MD_CTX *mdctx;
  const EVP_MD *md;
  md = EVP_sha3_512();
  mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, passwordBytes, password.size());
  EVP_DigestFinal_ex(mdctx, digest, NULL);
  EVP_MD_CTX_free(mdctx);

  // Concatenare digest e salt
  unsigned char saltedDigest[SHA3_512_DIGEST_LENGTH + sizeof(salt)];
  std::copy(digest, digest + SHA3_512_DIGEST_LENGTH, saltedDigest);
  std::copy(saltBytes, saltBytes + sizeof(salt), saltedDigest + SHA3_512_DIGEST_LENGTH);

  // Calcolare l'hash SHA3 del digest concatenato
  unsigned char finalDigest[SHA3_512_DIGEST_LENGTH]; // Modifica la dimensione del digest
  mdctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, saltedDigest, SHA3_512_DIGEST_LENGTH + sizeof(salt));
  EVP_DigestFinal_ex(mdctx, finalDigest, NULL);
  EVP_MD_CTX_free(mdctx);

  // Convertire l'hash finale in stringa esadecimale
  std::string hexHash;
  for (int i = 0; i < SHA3_512_DIGEST_LENGTH; ++i)
  {
    char buf[3];
    sprintf(buf, "%02x", finalDigest[i]);
    hexHash += buf;
  }

  return hexHash;
}

int main()
{
  string password = "password";
  int salt = generateSalt();
  string hashedPassword = hashWithSalt(password, 0);
  cout << "Hashed password: " << hashedPassword << "\n";
  return 1;
}