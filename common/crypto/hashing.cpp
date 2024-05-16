#ifndef HASHING_CPP
#define HASHING_CPP

#include <iostream>
#include <string>
#include <openssl/evp.h> //for all other OpenSSL function calls
#include <random>
#include <openssl/sha.h>

using namespace std;

#define SHA3_512_DIGEST_LENGTH 512

/**
 * Generates a random salt value.
 *
 * @return The generated salt value.
 */
int generateSalt()
{
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dis(0, 1000000); // Intervallo per il salt
  return dis(gen);
}

/**
 * Computes the SHA3-512 hash of the given input string.
 *
 * @param input The input string to be hashed.
 * @return The SHA3-512 hash of the input string as a hexadecimal string.
 */
string computeSHA3_512Hash(const string &input)
{
  // creation of the context used to calculate the hash
  EVP_MD_CTX *mdctx = EVP_MD_CTX_new();

  const EVP_MD *md = EVP_sha3_512();

  unsigned char hash[SHA512_DIGEST_LENGTH];
  unsigned int hash_len;

  EVP_DigestInit_ex(mdctx, md, NULL);
  EVP_DigestUpdate(mdctx, input.c_str(), input.length());
  EVP_DigestFinal_ex(mdctx, hash, &hash_len);

  EVP_MD_CTX_free(mdctx);

  // Convert hash to hex string
  string hashStr;
  char hex[3];
  for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i)
  {
    // convert the byte into a hex string
    snprintf(hex, sizeof(hex), "%02x", hash[i]);
    // append the hexadecimal into the result string
    hashStr.append(hex);
  }

  return hashStr;
}

/**
 * Computes the SHA3-512 hash of the given input concatenated with the salt.
 *
 * @param input The input string to be hashed.
 * @param salt The salt string to be concatenated with the input
 * @return The SHA3-512 hash of the input concatenated with the salt.
 */
string computeSHA3_512Hash(const string &input, const string &salt)
{
  string data = input + salt;
  return computeSHA3_512Hash(data);
}

/**
 * Computes the SHA3-512 hash of the given input concatenated with the salt
 * @param input The input string to be hashed.
 * @param salt The salt as integer value to be transformed into a string and then to be concatenanted with the input
 * @return The SHA3-512 hash of the input concatenated with the salt.
*/
string computeSHA3_512Hash(const string &input, const int &salt)
{
  string data = input + to_string(salt);
  return computeSHA3_512Hash(data);
}

#endif