#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sstream>
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

// Function to send email
bool send_email(const string &from,
                const string &to,
                const string &subject,
                const string &body,
                const string &smtp_server,
                int port)
{

    // Initialize OpenSSL
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    // Create SSL context
    SSL_CTX *ctx = SSL_CTX_new(SSLv23_client_method());
    if (ctx == nullptr)
    {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // Create TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        SSL_CTX_free(ctx);
        return false;
    }

    // Connect to SMTP server
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = inet_addr(smtp_server.c_str());
    serveraddr.sin_port = htons(port);
    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("connection failed");
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Create SSL object
    SSL *ssl = SSL_new(ctx);
    if (ssl == nullptr)
    {
        ERR_print_errors_fp(stderr);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Associate SSL with socket
    SSL_set_fd(ssl, sockfd);

    // Initiate SSL connection
    if (SSL_connect(ssl) != 1)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Send SMTP commands (EHLO, AUTH, MAIL FROM, RCPT TO, DATA, etc.)
    // ... (implementation required based on SMTP protocol)
    // Send EHLO command
    string ehlo_command = "EHLO localhost\r\n";
    if (SSL_write(ssl, ehlo_command.c_str(), ehlo_command.length()) < 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }
    char server_response[1024];
    int bytes_received = SSL_read(ssl, server_response, sizeof(server_response));
    if (bytes_received <= 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Send MAIL FROM command
    string mail_from_command = "MAIL FROM: <" + from + ">\r\n";
    if (SSL_write(ssl, mail_from_command.c_str(), mail_from_command.length()) < 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }
    bytes_received = SSL_read(ssl, server_response, sizeof(server_response));
    if (bytes_received <= 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Send RCPT TO command for each recipient
    string rcpt_to_command = "RCPT TO: <" + to + ">\r\n";
    if (SSL_write(ssl, rcpt_to_command.c_str(), rcpt_to_command.length()) < 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }
    bytes_received = SSL_read(ssl, server_response, sizeof(server_response));
    if (bytes_received <= 0)
    {
        if (bytes_received <= 0)
        {
            ERR_print_errors_fp(stderr);
            SSL_free(ssl);
            close(sockfd);
            SSL_CTX_free(ctx);
            return false;
        }
    }

    // Send DATA command
    string data_command = "DATA\r\n";
    if (SSL_write(ssl, data_command.c_str(), data_command.length()) < 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }
    bytes_received = SSL_read(ssl, server_response, sizeof(server_response));
    if (bytes_received <= 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }
    // ... (implementation required)
    // Send email content (subject, body)
    string email_content = "Subject: " + subject + "\r\n\r\n" + body + "\r\n.";
    if (SSL_write(ssl, email_content.c_str(), email_content.length()) < 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Read server response after sending content
    bytes_received = SSL_read(ssl, server_response, sizeof(server_response));
    if (bytes_received <= 0)
    {
        ERR_print_errors_fp(stderr);
        SSL_free(ssl);
        close(sockfd);
        SSL_CTX_free(ctx);
        return false;
    }

    // Close connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);

    // Cleanup OpenSSL
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();

    return true;
}

int main()
{
    // Example usage
    string from = "nicolalepore0@gmail.com";
    string to = "nicolalepore0@gmail.com";
    string subject = "Test Email";
    string body = "This is a test email sent from C++ with OpenSSL.";
    string smtp_server = "smtp.gmail.com";
    int port = 587; // or 587 for STARTTLS

    if (send_email(from, to, subject, body, smtp_server, port))
    {
        cout << "Email sent successfully!" << endl;
    }
    else
    {
        cerr << "Error sending email." << endl;
    }

    return 0;
}
