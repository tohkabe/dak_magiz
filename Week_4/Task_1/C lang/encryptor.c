#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_BLOCK_SIZE 16

typedef struct AESCipher {
    EVP_CIPHER_CTX *ctx;
} AESCipher;

void AESCipher_init(AESCipher *cipher, const unsigned char *key) {
    cipher->ctx = EVP_CIPHER_CTX_new();
    if (cipher->ctx == NULL) {
        printf("Error creating EVP_CIPHER_CTX\n");
        return;
    }

    if (EVP_EncryptInit_ex(cipher->ctx, EVP_aes_128_cbc(), NULL, key, NULL) != 1) {
        printf("Error initializing encryption context\n");
    }
}

void AESCipher_encrypt(AESCipher *cipher, unsigned char *data, size_t data_len, unsigned char *encrypted_data) {
    unsigned char iv[AES_BLOCK_SIZE];
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        return;
    }

    int len = 0;
    if (EVP_EncryptInit_ex(cipher->ctx, NULL, NULL, NULL, iv) != 1) {
        return;
    }

    if (EVP_EncryptUpdate(cipher->ctx, encrypted_data + AES_BLOCK_SIZE, &len, data, data_len) != 1) {
        return;
    }

    int encrypted_len = len;
    if (EVP_EncryptFinal_ex(cipher->ctx, encrypted_data + AES_BLOCK_SIZE + len, &len) != 1) {
        return;
    }
    encrypted_len += len;

    memcpy(encrypted_data, iv, AES_BLOCK_SIZE);  // Store the IV at the start
}

void AESCipher_decrypt(AESCipher *cipher, unsigned char *enc_data, size_t enc_data_len, unsigned char *decrypted_data) {
    unsigned char iv[AES_BLOCK_SIZE];
    memcpy(iv, enc_data, AES_BLOCK_SIZE);  // Extract the IV

    int len = 0;
    if (EVP_DecryptInit_ex(cipher->ctx, NULL, NULL, NULL, iv) != 1) {
        return;
    }

    if (EVP_DecryptUpdate(cipher->ctx, decrypted_data, &len, enc_data + AES_BLOCK_SIZE, enc_data_len - AES_BLOCK_SIZE) != 1) {
        return;
    }

    int decrypted_len = len;
    if (EVP_DecryptFinal_ex(cipher->ctx, decrypted_data + len, &len) != 1) {
        return;
    }
    decrypted_len += len;
}

void AESCipher_free(AESCipher *cipher) {
    EVP_CIPHER_CTX_free(cipher->ctx);
}
