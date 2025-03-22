# Secure Bulletin Board System

This repository contains the implementation of a **Secure Bulletin Board System (BBS)** developed in C++ as part of the *Foundations of Cybersecurity* course at the University of Pisa (Academic Year 2023/2024). The project was developed collaboratively by **Nicola Lepore** and **Francesco Copelli**.

---

## Overview

The Secure Bulletin Board System is a centralized application that allows registered users to:
- **Register** for an account
- **Log in** securely
- **Post messages** on the bulletin board
- **Retrieve and list** existing messages

Security is the primary focus of the project. The system is designed to ensure confidentiality, integrity, and authentication for all client–server communications through a robust cryptographic protocol.

---

## Protocol Design and Security Features

The protocol implemented in the system is designed with the following key aspects:

### 1. **Key Exchange Protocol**
- **Diffie-Hellman Key Exchange:**  
  When a client connects to the server, both parties generate a public-private key pair using OpenSSL's EVP_Keygen functions.  
  - **Server to Client:** The server sends its public key in plaintext.
  - **Client Processing:** Upon receiving the server's public key, the client calculates the shared secret using its private key.
  - **Client to Server:** The client then sends its public key to the server.
  - **Server Processing:** The server calculates the shared secret with the client’s public key.
  
  This mutual key exchange results in a **shared secret** used to encrypt subsequent communications.

### 2. **Secure Message Exchange**
- **Encryption:**  
  After the shared secret is established, all messages are encrypted using **AES-256 CBC mode**.  
  - The encryption uses a securely generated Initialization Vector (IV) via `RAND_bytes`, ensuring that the same IV is not reused with the same key.
  
- **Message Formatting and Integrity:**  
  Each message exchanged between the client and the server is composed with the following fields:
  - **Message Content:** The actual payload, which may be in plaintext (for initial key exchange) or ciphertext (for secured messages).
  - **Delimiters (`$$$` and `@@@`):** Used to separate parts of the message, particularly to isolate the counter and hash.
  - **Counter:** An incremental counter starting from 0, used to prevent replay attacks within a single session.
  - **Hash:** A cryptographic hash computed over the message content, delimiters, and counter, ensuring the integrity of the message.
  
  The inclusion of the counter and hash fields provides a measure of protection against replay attacks and tampering.

### 3. **Challenge-Response Mechanism**
- **Authentication during Registration and Login:**  
  When a client attempts to register or log in, a challenge-response mechanism is employed:
  - **Server Challenge:** After receiving the client’s public key, the server encrypts a challenge message using the shared secret and sends it to the client.
  - **Client Response:** The client decrypts the challenge, sends an acknowledgment, and issues its own encrypted challenge.
  - **Server Verification:** The server decrypts and verifies the client’s challenge to confirm authenticity.
  
  This two-way challenge-response process ensures that both parties are in possession of the correct shared secret, bolstering mutual authentication.

---

## Application Architecture

The project is structured into three main components:

- **Client:**  
  Implements the user interface (CLI) and handles communication with the server. The client supports registration, login, posting, listing, and retrieving messages.

- **Common:**  
  Contains shared modules for both the client and server, including:
  - Cryptographic functions for AES-256 CBC encryption/decryption.
  - Hashing functions (using SHA3-512) to ensure message integrity.
  - Key exchange and message formatting utilities.

- **Server:**  
  Manages client connections and handles secure communication. The server processes registration and login requests, manages the bulletin board (storing, listing, and retrieving messages), and ensures all data exchanged is protected via the established secure protocol.

---

## Getting Started

### Prerequisites

- **C++ Compiler:**  
  Ensure you have a C++ compiler (e.g., GCC, Clang) installed.
  
- **OpenSSL Library:**  
  The project requires OpenSSL for cryptographic operations.
  
- **Make:**  
  A GNU Make utility to build the client and server applications.

### Compilation and Execution

1. **Compile the Server:**
   ```bash
   cd server
   make clean && make server
   ./server
   ```
   
2. **Compile the Client:**
   ```bash
   cd client
   make clean && make client
   ./client
   ```
---

## Usage

After successful compilation:
- **Registration:** Users can register by providing a username, password, and email. A one-time password (OTP) is used during registration for additional verification.
- **Login:** Users log in using their credentials. The secure key exchange and challenge-response mechanism ensure the authenticity of the session.
- **Posting and Retrieving Messages:** Authenticated users can post messages to the bulletin board and retrieve messages by specifying a message ID or listing recent messages.

---

## License

This project is released under the [MIT License](LICENSE).

---

## Acknowledgements

- **University of Pisa** for providing the academic framework and support.
- The instructors of the *Foundations of Cybersecurity* course.
- **Nicola Lepore** and **Francesco Copelli** for their joint effort in developing this secure Bulletin Board System.
