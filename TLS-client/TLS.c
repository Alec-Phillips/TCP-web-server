#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <string.h>
#include <openssl/bio.h>


/*
    Course of action:
    - Step 1: Initalize and make a function such that it's thread safe, and will create a private, public key for each client that connects
    - Step 2: Send the certificate from the server to the client/browser?
        - Note: Client may send its own certificate to the browser, that we must verify.
    - Step 3: Client/Browser authenticates this (automatically)?
    - Step 4: Figure out if there's anything that needs to be done, or if the handshake is resolved?
*/

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    SSL_library_init();
    SSL_load_error_strings();
#endif

char* passphrase = "open salami";

int main() {

    EVP_PKEY *key = EVP_PKEY_new(); //Initalize a new key struct where keys will be stored
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL); //Allocates the RSA public key algorithm to ctx using *key
    EVP_PKEY_keygen_init(ctx);  

    // Sets the cipher suite: using a string, cipher suite is the set of algorithms used for encryption!
    // SSL_CTX_set_cipher_list(ctx, "");
    EVP_CIPHER* pcipher = NULL;

    
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 4096); // Set the ctx's public key algorithm to use 4096 bits
    EVP_PKEY_keygen(ctx, &key); // Initalize the public key algorithm into the key struct 

    // Idea: Create a file, write the private key to this file.
    // For testing purposes, will use a static file name, but if possible, change to a higher 
    // Refer to https://stackoverflow.com/questions/22743415/what-are-the-differences-between-pem-cer-and-der for file endings
    // Pem = readable, der = unreadable but smaller
    FILE *key_file;
    key_file = fopen("private_key_file.pem", "r");
    fclose(key_file); 
    key_file = fopen("private_key_file.pem", "w"); 
    PEM_write_PrivateKey(key_file, key, NULL, (unsigned char *) passphrase, (int) strlen(passphrase), NULL, NULL); // Write the private key to key_file.txt
    // i2d_PrivateKey_fp(key_file, key); 
    fclose(key_file);
    
    key_file = fopen("public_key_file.pem", "r");
    fclose(key_file); 
    // Write the public key to key_file.txt
    key_file = fopen("public_key_file.pem", "w"); 
    PEM_write_PUBKEY(key_file, key);
    // i2d_PUBKEY_fp(key_file, key); 
    fclose(key_file);



    // Creating certificate
    X509* x509; // represents a x509 certificate in memory
    x509 = X509_new();
    X509_set_pubkey(x509, PEM_read_PUBKEY(key_file, key, passphrase, NULL));

    // Set properties of certificate
    X509_NAME* name;
    name = X509_get_subject_name(x509);

    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*) "CA", -1, -1, 0); //Country code
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*) "Doofenshmirtz Evil Inc.", -1, -1, 0); //Company name?
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*) "localhost", -1, -1, 0); // Address
    
    X509_set_issuer_name(x509, name);
    X509_sign(x509, key, EVP_sha1());

    // Write certificate to pem file (readable)
    FILE* fp = fopen("X509_certificiate.pem", "wb");
    PEM_write_X509(fp, x509);
    fclose(fp);





    EVP_PKEY_free(key);
    EVP_CIPHER_CTX_free(ctx);
    X509_free(x509);



}

SSL_CTX* get_context() {
    SSL_CTX* ctx = SSL_CTX_new(TLS_method());
    // SSL_CTX_set_min_proto_version(ctx.get(), TLS1_2_VERSION);
    if(SSL_CTX_use_certificate_file(ctx, "../TLS-client/server-certificate.pem", SSL_FILETYPE_PEM) <= 0) {
        printf("Couldn't load certificate.");
        exit(EXIT_FAILURE);
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, "server-private-key.pem", SSL_FILETYPE_PEM) <= 0) {
        printf("Couldn't get private key.");
        exit(EXIT_FAILURE);
    }
    return ctx;
    // SSL_CTX_use_certificate
}

BIO* init_bio(SSL_CTX* ctx, int client) {
    return BIO_new_ssl(ctx, client);
}
