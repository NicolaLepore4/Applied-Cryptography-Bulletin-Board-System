# Applied-Cryptography-Bulletin-Board-System
Foundations of Cybersecurity/Applied Cryptography (a.a.2023-24) project

Protocollo Crittografico per il Sistema BBS che utilizza C/C++ e OpenSSL (escluso TLS):

1. Autenticazione Utente:

Protocollo: Utilizzare una combinazione di Funzione di derivazione della chiave basata su password (PBKDF2) con un algoritmo di hash sicuro (SHA-256) e lo scambio di chiavi Diffie-Hellman (DH).
Processo:
L'utente inserisce nome utente e password.
Il server recupera un salt associato al nome utente (memorizzato in modo sicuro sul server).
Client e server eseguono uno scambio di chiavi DH per stabilire una chiave segreta condivisa.
La password dell'utente, insieme al salt recuperato, viene utilizzata con PBKDF2 per derivare una chiave lato client.
Il client crittografa un nonce casuale con la chiave derivata e lo invia al server insieme al nome utente.
Il server utilizza il salt e il nome utente memorizzati per derivare la propria chiave con PBKDF2.
Il server decrittografa il messaggio ricevuto utilizzando la chiave derivata e verifica il nonce.
La decrittazione e la verifica riuscite indicano una password valida.

2. Comunicazione Sicura:

Protocollo: Utilizzare la Crittografia Simmetrica a Chiave (AES-GCM) con la chiave segreta condivisa stabilita tramite lo scambio di chiavi DH.
Processo:
Sia il client che il server utilizzano la chiave segreta condivisa per la crittografia e la decrittazione AES-GCM dei messaggi.
GCM fornisce riservatezza, integrità e autenticità per la comunicazione.

3. Segretezza Perfetta Avanzata (PFS):

Lo scambio di chiavi DH garantisce PFS. Anche se la chiave privata del server viene compromessa, le chiavi di sessione passate non possono essere derivate.